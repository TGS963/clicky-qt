#pragma once

#include <QColor>
#include <QList>
#include <QObject>
#include <QPointF>
#include <QQmlListProperty>
#include <QTimer>

class TaskItem;

// Central observable state for the overlay. Exposed to QML through
// Q_PROPERTYs so the overlay scene can bind directly to voice/overlay state,
// cursor position, the list of active tasks, and the transient primary flash
// color that drives the listening-color choreography.
class CompanionState : public QObject {
    Q_OBJECT

    // --- Voice + overlay mode ---
    Q_PROPERTY(VoiceState voiceState READ voiceState NOTIFY voiceStateChanged)
    Q_PROPERTY(OverlayMode overlayMode READ overlayMode NOTIFY overlayModeChanged)

    // --- Cursor tracking ---
    Q_PROPERTY(QPointF cursorScreenPosition READ cursorScreenPosition
               NOTIFY cursorScreenPositionChanged)

    // --- Task list (satellite stars) ---
    Q_PROPERTY(QQmlListProperty<TaskItem> activeTasks READ activeTasksListProperty
               NOTIFY activeTasksChanged)

    // --- Primary-dot color flash (synced with task lifecycle) ---
    Q_PROPERTY(QColor primaryFlashColor READ primaryFlashColor NOTIFY primaryFlashColorChanged)
    Q_PROPERTY(bool primaryFlashActive READ primaryFlashActive NOTIFY primaryFlashColorChanged)

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

    explicit CompanionState(QObject* parent = nullptr);

    VoiceState voiceState() const { return voiceStateValue; }
    OverlayMode overlayMode() const { return overlayModeValue; }
    QPointF cursorScreenPosition() const { return cursorScreenPositionValue; }
    QColor primaryFlashColor() const { return primaryFlashColorValue; }
    bool primaryFlashActive() const { return primaryFlashActiveValue; }

public slots:
    void setVoiceState(VoiceState newVoiceState);
    void setOverlayMode(OverlayMode newOverlayMode);
    void toggleOverlayMode();
    void togglePushToTalkListening();
    void setCursorScreenPosition(const QPointF& newCursorScreenPosition);

signals:
    void voiceStateChanged();
    void overlayModeChanged();
    void cursorScreenPositionChanged();
    void activeTasksChanged();
    void primaryFlashColorChanged();

private:
    // --- State ---
    VoiceState voiceStateValue = Idle;
    OverlayMode overlayModeValue = FollowingCursor;
    QPointF cursorScreenPositionValue;

    // --- Task tracking ---
    QList<TaskItem*> activeTasksList;
    QTimer taskAdvanceTimer;
    int nextOrbitAngleDegrees = 0;

    // --- Primary-dot color flash ---
    QColor primaryFlashColorValue;
    bool primaryFlashActiveValue = false;
    QTimer primaryFlashReleaseTimer;
    QColor pendingTaskColorValue;

    // --- Internal helpers ---
    void spawnTaskUsingPendingColor();
    void advanceAndPruneTasks();
    void beginPrimaryFlashHeld(const QColor& flashColor);
    void schedulePrimaryFlashRelease();
    void endPrimaryFlash();

    // --- QQmlListProperty plumbing for `activeTasks` ---
    QQmlListProperty<TaskItem> activeTasksListProperty();
    static qsizetype activeTasksCount(QQmlListProperty<TaskItem>* listProperty);
    static TaskItem* activeTasksAt(QQmlListProperty<TaskItem>* listProperty, qsizetype index);
};
