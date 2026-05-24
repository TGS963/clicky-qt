#include "companion_state.h"

#include "task_item.h"
#include "task_list_model.h"

#include <QColor>
#include <QPointer>
#include <QRandomGenerator>

namespace {

constexpr qreal TASK_TICK_INTERVAL_SECONDS = 1.0 / 60.0;
constexpr int TASK_TICK_INTERVAL_MS = 16;

// Tasks only need elapsedSeconds ticked while their QML satellite is still
// inside the birth animation window (see OverlayContent.qml
// `satelliteBirthDurationSeconds = 0.32`). After that, elapsedSeconds is
// clamped by the QML side and additional updates just churn bindings.
// Slightly larger than the QML constant to absorb scheduling jitter.
constexpr qreal BIRTH_ANIMATION_TICK_CAP_SECONDS = 0.5;

// Delay between a task reaching a terminal status and being physically
// removed from the model. Gives the QML side time to fade the satellite +
// the row out before the delegate is destroyed.
constexpr int TERMINAL_FADEOUT_BEFORE_REMOVAL_MS = 420;

constexpr int PRIMARY_FLASH_HOLD_DURATION_MS = 280;

constexpr int FULL_ROTATION_DEGREES = 360;

// Avoid generating task colors in the blue band so satellites are visually
// distinguishable from the primary blue companion dot.
constexpr int HUE_BLUE_BAND_MIN_DEGREES = 195;
constexpr int HUE_BLUE_BAND_MAX_DEGREES = 265;
constexpr int TASK_COLOR_SATURATION = 200;
constexpr int TASK_COLOR_VALUE = 240;

QColor pickRandomNonBlueTaskColor() {
    int hueDegrees = 0;
    do {
        hueDegrees = QRandomGenerator::global()->bounded(FULL_ROTATION_DEGREES);
    } while (hueDegrees > HUE_BLUE_BAND_MIN_DEGREES && hueDegrees < HUE_BLUE_BAND_MAX_DEGREES);
    return QColor::fromHsv(hueDegrees, TASK_COLOR_SATURATION, TASK_COLOR_VALUE);
}

}  // namespace

CompanionState::CompanionState(QObject* parent)
    : QObject(parent),
      activeTasksModelInstance(new TaskListModel(this)) {
    taskAdvanceTimer.setInterval(TASK_TICK_INTERVAL_MS);
    connect(&taskAdvanceTimer, &QTimer::timeout, this, &CompanionState::advanceAndPruneTasks);
    // Timer is (re)started by registerNewTask; it stops itself once every
    // active task has aged past the birth-animation window.

    primaryFlashReleaseTimer.setSingleShot(true);
    primaryFlashReleaseTimer.setInterval(PRIMARY_FLASH_HOLD_DURATION_MS);
    connect(&primaryFlashReleaseTimer, &QTimer::timeout, this, &CompanionState::endPrimaryFlash);
}

void CompanionState::setVoiceState(VoiceState newVoiceState) {
    if (voiceStateValue == newVoiceState) {
        return;
    }
    voiceStateValue = newVoiceState;
    emit voiceStateChanged();
}

void CompanionState::setOverlayMode(OverlayMode newOverlayMode) {
    if (overlayModeValue == newOverlayMode) {
        return;
    }
    overlayModeValue = newOverlayMode;
    emit overlayModeChanged();
}

void CompanionState::toggleOverlayMode() {
    setOverlayMode(overlayModeValue == FollowingCursor ? Docked : FollowingCursor);
}

void CompanionState::togglePushToTalkListening() {
    if (voiceStateValue == Listening) {
        spawnTaskUsingPendingColor();
        setVoiceState(Idle);
        schedulePrimaryFlashRelease();
    } else {
        pendingTaskColorValue = pickRandomNonBlueTaskColor();
        beginPrimaryFlashHeld(pendingTaskColorValue);
        setVoiceState(Listening);
    }
}

void CompanionState::spawnSubprocessTask(const QString& title,
                                         const QString& program,
                                         const QStringList& arguments) {
    const QColor taskColor = pickRandomNonBlueTaskColor();
    auto* newTask = registerNewTask(taskColor, title,
                                    QStringLiteral("Starting %1…").arg(program));
    newTask->attachProcess(program, arguments);
}

void CompanionState::setCursorScreenPosition(const QPointF& newCursorScreenPosition) {
    if (cursorScreenPositionValue == newCursorScreenPosition) {
        return;
    }
    cursorScreenPositionValue = newCursorScreenPosition;
    emit cursorScreenPositionChanged();
}

void CompanionState::openTaskMenu() {
    if (interactionModeValue == MenuOpen) {
        return;
    }
    setTaskMenuAnchorPosition(cursorScreenPositionValue);
    setInteractionMode(MenuOpen);
}

void CompanionState::closeTaskMenu() {
    setInteractionMode(Passive);
}

void CompanionState::forceCloseTask(TaskItem* task) {
    if (!task || !activeTasksModelInstance->contains(task)) {
        return;
    }
    // requestKill terminates the subprocess if any and flips status to
    // Cancelled. Tasks without a process just get the status flip.
    task->requestKill();
}

void CompanionState::setInteractionMode(InteractionMode newInteractionMode) {
    if (interactionModeValue == newInteractionMode) {
        return;
    }
    interactionModeValue = newInteractionMode;
    emit interactionModeChanged();
}

void CompanionState::setTaskMenuAnchorPosition(const QPointF& anchorPosition) {
    if (taskMenuAnchorPositionValue == anchorPosition) {
        return;
    }
    taskMenuAnchorPositionValue = anchorPosition;
    emit taskMenuAnchorPositionChanged();
}

void CompanionState::spawnTaskUsingPendingColor() {
    const QColor taskColor = pendingTaskColorValue.isValid()
        ? pendingTaskColorValue
        : pickRandomNonBlueTaskColor();
    const QString demoTitle = QStringLiteral("Demo Task %1").arg(nextTaskOrdinalNumber++);

    // Until real AI work is wired in, the PTT-release path spawns a
    // shell-driven demo subprocess that emits `PROGRESS: 0.N` lines and
    // interleaved status text so the row's progress bar + description fill
    // in from real stdout rather than a fake timer.
    const QString demoCommand = QStringLiteral(
        "for i in 1 2 3 4 5 6 7 8 9 10; do "
        "  echo \"PROGRESS: 0.$i\"; "
        "  echo \"working on step $i\"; "
        "  sleep 1; "
        "done; "
        "echo \"PROGRESS: 1.0\"; "
        "echo \"done\"");

    auto* newTask = registerNewTask(taskColor, demoTitle, QStringLiteral("Starting…"));
    newTask->attachProcess(QStringLiteral("bash"),
                           QStringList{QStringLiteral("-c"), demoCommand});
}

TaskItem* CompanionState::registerNewTask(const QColor& taskColor,
                                          const QString& title,
                                          const QString& description) {
    auto* newTask = new TaskItem(taskColor, title, description, this);

    QPointer<TaskItem> taskGuard(newTask);
    connect(newTask, &TaskItem::statusChanged, this, [this, taskGuard]() {
        if (!taskGuard || !taskGuard->isTerminal()) {
            return;
        }
        QTimer::singleShot(TERMINAL_FADEOUT_BEFORE_REMOVAL_MS, this, [this, taskGuard]() {
            if (!taskGuard || !activeTasksModelInstance->contains(taskGuard)) {
                return;
            }
            removeTaskAndDelete(taskGuard);
        });
    });

    activeTasksModelInstance->appendTask(newTask);
    if (!taskAdvanceTimer.isActive()) {
        taskAdvanceTimer.start();
    }
    return newTask;
}

void CompanionState::beginPrimaryFlashHeld(const QColor& flashColor) {
    primaryFlashReleaseTimer.stop();
    const bool wasActive = primaryFlashActiveValue;
    primaryFlashColorValue = flashColor;
    primaryFlashActiveValue = true;
    emit primaryFlashColorChanged();
    if (!wasActive) {
        emit primaryFlashActiveChanged();
    }
}

void CompanionState::schedulePrimaryFlashRelease() {
    primaryFlashReleaseTimer.start();
}

void CompanionState::endPrimaryFlash() {
    if (!primaryFlashActiveValue) {
        return;
    }
    primaryFlashActiveValue = false;
    emit primaryFlashActiveChanged();
}

void CompanionState::advanceAndPruneTasks() {
    const int taskCount = activeTasksModelInstance->taskCount();
    int tasksStillNeedingTicks = 0;
    // Advance only tasks whose elapsedSeconds is still inside the birth
    // window. Progress is driven by the attached process's stdout, not
    // by this timer.
    for (int taskIndex = 0; taskIndex < taskCount; ++taskIndex) {
        TaskItem* task = activeTasksModelInstance->taskAt(taskIndex);
        if (task->elapsedSeconds() < BIRTH_ANIMATION_TICK_CAP_SECONDS) {
            task->advance(TASK_TICK_INTERVAL_SECONDS);
            ++tasksStillNeedingTicks;
        }
    }
    if (tasksStillNeedingTicks == 0) {
        taskAdvanceTimer.stop();
    }
}

void CompanionState::removeTaskAndDelete(TaskItem* task) {
    activeTasksModelInstance->removeTask(task);
    task->deleteLater();
}
