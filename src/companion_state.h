#pragma once

#include "task_list_model.h"

#include <QColor>
#include <QList>
#include <QObject>
#include <QPointF>
#include <QTimer>

class TaskItem;

// Central observable state for the overlay. Exposed to QML through
// Q_PROPERTYs so the overlay scene can bind directly to voice/overlay/
// interaction state, cursor position, the list of active tasks, and the
// transient primary flash color that drives the listening-color
// choreography.
class CompanionState : public QObject {
    Q_OBJECT

    // --- Voice + overlay mode + interaction mode ---
    Q_PROPERTY(VoiceState voiceState READ voiceState NOTIFY voiceStateChanged)
    Q_PROPERTY(OverlayMode overlayMode READ overlayMode NOTIFY overlayModeChanged)
    Q_PROPERTY(InteractionMode interactionMode READ interactionMode
               NOTIFY interactionModeChanged)

    // --- Cursor tracking ---
    Q_PROPERTY(QPointF cursorScreenPosition READ cursorScreenPosition
               NOTIFY cursorScreenPositionChanged)
    Q_PROPERTY(QPointF taskMenuAnchorPosition READ taskMenuAnchorPosition
               NOTIFY taskMenuAnchorPositionChanged)

    // --- Task list (satellite stars) ---
    // QAbstractListModel — Repeater can do incremental insert/remove so
    // existing delegates persist across changes (lets them animate).
    Q_PROPERTY(TaskListModel* activeTasksModel READ activeTasksModel CONSTANT)

    // --- Primary-dot color flash (synced with task lifecycle) ---
    Q_PROPERTY(QColor primaryFlashColor READ primaryFlashColor NOTIFY primaryFlashColorChanged)
    Q_PROPERTY(bool primaryFlashActive READ primaryFlashActive NOTIFY primaryFlashActiveChanged)

public:
    enum VoiceState {
        Idle,
        Listening,
        Processing,
        Responding,
    };
    Q_ENUM(VoiceState)

    enum OverlayMode {
        FollowingCursor,
        Docked,
    };
    Q_ENUM(OverlayMode)

    // Held-modifier task menu lifecycle:
    //   Passive  -> default; overlay is click-through, primary dot follows cursor
    //   MenuOpen -> Right Ctrl held; primary dot morphs into the task menu card
    enum InteractionMode {
        Passive,
        MenuOpen,
    };
    Q_ENUM(InteractionMode)

    explicit CompanionState(QObject* parent = nullptr);

    VoiceState voiceState() const { return voiceStateValue; }
    OverlayMode overlayMode() const { return overlayModeValue; }
    InteractionMode interactionMode() const { return interactionModeValue; }
    QPointF cursorScreenPosition() const { return cursorScreenPositionValue; }
    QPointF taskMenuAnchorPosition() const { return taskMenuAnchorPositionValue; }
    QColor primaryFlashColor() const { return primaryFlashColorValue; }
    bool primaryFlashActive() const { return primaryFlashActiveValue; }

    TaskListModel* activeTasksModel() const { return activeTasksModelInstance; }

public slots:
    void setVoiceState(VoiceState newVoiceState);
    void setOverlayMode(OverlayMode newOverlayMode);
    void toggleOverlayMode();
    void togglePushToTalkListening();
    void setCursorScreenPosition(const QPointF& newCursorScreenPosition);

    void openTaskMenu();
    void closeTaskMenu();

    // Invoked from QML when the user clicks the per-row ✕ button.
    Q_INVOKABLE void forceCloseTask(TaskItem* task);

    // Spawn a real subprocess-backed task. `progressCommand` should print
    // lines like `PROGRESS: 0.42` to stdout to drive the progress bar; any
    // other line becomes the row's description.
    Q_INVOKABLE void spawnSubprocessTask(const QString& title,
                                         const QString& program,
                                         const QStringList& arguments);

signals:
    void voiceStateChanged();
    void overlayModeChanged();
    void interactionModeChanged();
    void cursorScreenPositionChanged();
    void taskMenuAnchorPositionChanged();
    void primaryFlashColorChanged();
    void primaryFlashActiveChanged();

private:
    // --- State ---
    VoiceState voiceStateValue = Idle;
    OverlayMode overlayModeValue = FollowingCursor;
    InteractionMode interactionModeValue = Passive;
    QPointF cursorScreenPositionValue;
    QPointF taskMenuAnchorPositionValue;

    // --- Task tracking ---
    TaskListModel* activeTasksModelInstance = nullptr;
    QTimer taskAdvanceTimer;
    int nextTaskOrdinalNumber = 1;

    // --- Primary-dot color flash ---
    QColor primaryFlashColorValue;
    bool primaryFlashActiveValue = false;
    QTimer primaryFlashReleaseTimer;
    QColor pendingTaskColorValue;

    // --- Internal helpers ---
    void setInteractionMode(InteractionMode newInteractionMode);
    void setTaskMenuAnchorPosition(const QPointF& anchorPosition);
    void spawnTaskUsingPendingColor();
    TaskItem* registerNewTask(const QColor& taskColor,
                              const QString& title,
                              const QString& description);
    void advanceAndPruneTasks();
    void removeTaskAndDelete(TaskItem* task);
    void beginPrimaryFlashHeld(const QColor& flashColor);
    void schedulePrimaryFlashRelease();
    void endPrimaryFlash();

};
