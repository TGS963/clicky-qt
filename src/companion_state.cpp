#include "companion_state.h"

#include "task_item.h"

#include <QColor>
#include <QRandomGenerator>

namespace {

constexpr qreal TASK_TICK_INTERVAL_SECONDS = 1.0 / 60.0;
constexpr int TASK_TICK_INTERVAL_MS = 16;
constexpr qreal DEFAULT_TASK_LIFETIME_SECONDS = 5.0;
constexpr int PRIMARY_FLASH_HOLD_DURATION_MS = 280;

// Successive satellite tasks are placed at angles separated by the golden
// angle so a burst of rapid spawns spreads visually around the cursor instead
// of stacking on top of each other.
constexpr int ORBIT_ANGLE_GOLDEN_STEP_DEGREES = 137;
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
    : QObject(parent) {
    taskAdvanceTimer.setInterval(TASK_TICK_INTERVAL_MS);
    connect(&taskAdvanceTimer, &QTimer::timeout, this, &CompanionState::advanceAndPruneTasks);
    taskAdvanceTimer.start();

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
    // Press 1 (Idle -> Listening): pick the upcoming task's color now and hold it on
    // the primary dot for the duration of the listening session.
    // Press 2 (Listening -> Idle): spawn the satellite with that same color; primary
    // briefly keeps the color while the star emerges, then fades back to blue.
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

void CompanionState::setCursorScreenPosition(const QPointF& newCursorScreenPosition) {
    if (cursorScreenPositionValue == newCursorScreenPosition) {
        return;
    }
    cursorScreenPositionValue = newCursorScreenPosition;
    emit cursorScreenPositionChanged();
}

void CompanionState::spawnTaskUsingPendingColor() {
    const QColor taskColor = pendingTaskColorValue.isValid()
        ? pendingTaskColorValue
        : pickRandomNonBlueTaskColor();
    const qreal startingOrbitAngleDegrees = static_cast<qreal>(nextOrbitAngleDegrees);
    nextOrbitAngleDegrees = (nextOrbitAngleDegrees + ORBIT_ANGLE_GOLDEN_STEP_DEGREES)
        % FULL_ROTATION_DEGREES;

    auto* newTask = new TaskItem(taskColor, startingOrbitAngleDegrees,
                                 DEFAULT_TASK_LIFETIME_SECONDS, this);
    activeTasksList.append(newTask);
    emit activeTasksChanged();
}

void CompanionState::beginPrimaryFlashHeld(const QColor& flashColor) {
    primaryFlashReleaseTimer.stop();
    primaryFlashColorValue = flashColor;
    primaryFlashActiveValue = true;
    emit primaryFlashColorChanged();
}

void CompanionState::schedulePrimaryFlashRelease() {
    primaryFlashReleaseTimer.start();
}

void CompanionState::endPrimaryFlash() {
    primaryFlashActiveValue = false;
    emit primaryFlashColorChanged();
}

void CompanionState::advanceAndPruneTasks() {
    if (activeTasksList.isEmpty()) {
        return;
    }
    bool anyExpired = false;
    for (TaskItem* task : activeTasksList) {
        task->advance(TASK_TICK_INTERVAL_SECONDS);
        if (task->isExpired()) {
            anyExpired = true;
        }
    }
    if (!anyExpired) {
        return;
    }

    QList<TaskItem*> remainingTasks;
    remainingTasks.reserve(activeTasksList.size());
    for (TaskItem* task : activeTasksList) {
        if (task->isExpired()) {
            task->deleteLater();
        } else {
            remainingTasks.append(task);
        }
    }
    activeTasksList = remainingTasks;
    emit activeTasksChanged();
}

QQmlListProperty<TaskItem> CompanionState::activeTasksListProperty() {
    return QQmlListProperty<TaskItem>(this, nullptr,
                                      &CompanionState::activeTasksCount,
                                      &CompanionState::activeTasksAt);
}

qsizetype CompanionState::activeTasksCount(QQmlListProperty<TaskItem>* listProperty) {
    auto* state = qobject_cast<CompanionState*>(listProperty->object);
    return state ? state->activeTasksList.size() : 0;
}

TaskItem* CompanionState::activeTasksAt(QQmlListProperty<TaskItem>* listProperty,
                                        qsizetype index) {
    auto* state = qobject_cast<CompanionState*>(listProperty->object);
    if (!state || index < 0 || index >= state->activeTasksList.size()) {
        return nullptr;
    }
    return state->activeTasksList.at(index);
}
