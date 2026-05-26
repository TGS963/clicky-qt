#pragma once

#include "task_list_model.h"

#include <QColor>
#include <QList>
#include <QObject>
#include <QPointF>
#include <QTimer>
#include <QVariantList>

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

    // --- Screenshot thumbnail preview (flashed briefly after a grab) ---
    // file:// URL of the just-captured screenshot and whether the QML preview
    // should currently be shown. Active for ~SCREENSHOT_PREVIEW_HOLD_MS.
    Q_PROPERTY(QString screenshotPreviewSource READ screenshotPreviewSource
               NOTIFY screenshotPreviewSourceChanged)
    Q_PROPERTY(bool screenshotPreviewActive READ screenshotPreviewActive
               NOTIFY screenshotPreviewActiveChanged)

    // --- Listening waveform bar amplitudes ---
    // QVariantList of `listeningBarCount` qreals in [0, 1]. Updated at ~30 Hz
    // while voiceState == Listening, frozen otherwise. Drives the
    // MorphingWaveform bars. Currently fed by a smoothed RNG stub; swap to
    // QAudioSource later without touching QML.
    Q_PROPERTY(QVariantList listeningAmplitudes READ listeningAmplitudes
               NOTIFY listeningAmplitudesChanged)
    Q_PROPERTY(int listeningBarCount READ listeningBarCount CONSTANT)

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
    //   MenuOpen -> trigger key held (Right Ctrl on Linux/Win, Right ⌥ on macOS)
    enum InteractionMode {
        Passive,
        MenuOpen,
    };
    Q_ENUM(InteractionMode)

    // Which input mode currently owns the listening state. Both input modes
    // drive the same Listening voiceState, so this is the single source of
    // truth that keeps them mutually exclusive: a session can only be ended
    // by the key that started it, and the other mode's key is ignored while a
    // session is open.
    enum InputSession {
        NoInputSession,
        VoiceListening,
        ScreenshotListening,
    };
    Q_ENUM(InputSession)

    explicit CompanionState(QObject* parent = nullptr);

    VoiceState voiceState() const { return voiceStateValue; }
    OverlayMode overlayMode() const { return overlayModeValue; }
    InteractionMode interactionMode() const { return interactionModeValue; }
    InputSession inputSession() const { return inputSessionValue; }
    QPointF cursorScreenPosition() const { return cursorScreenPositionValue; }
    QPointF taskMenuAnchorPosition() const { return taskMenuAnchorPositionValue; }
    QString screenshotPreviewSource() const { return screenshotPreviewSourceValue; }
    bool screenshotPreviewActive() const { return screenshotPreviewActiveValue; }
    QColor primaryFlashColor() const { return primaryFlashColorValue; }
    bool primaryFlashActive() const { return primaryFlashActiveValue; }

    QVariantList listeningAmplitudes() const { return listeningAmplitudesValue; }
    int listeningBarCount() const;

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

    // Screenshot input mode is screenshot + voice context: the controller
    // grabs the image, then drives this two-step listen cycle (mirroring the
    // push-to-talk voice path, which is audio-only).
    //   begin  -> remember the screenshot, flash + enter Listening, flash the
    //             thumbnail preview. Audio capture is stubbed (same as voice).
    //   finish -> leave Listening and spawn the task for the remembered image.
    void beginScreenshotListening(const QString& screenshotFilePath);
    void finishScreenshotListeningAndSpawnTask();

signals:
    void voiceStateChanged();
    void overlayModeChanged();
    void interactionModeChanged();
    void cursorScreenPositionChanged();
    void taskMenuAnchorPositionChanged();
    void primaryFlashColorChanged();
    void primaryFlashActiveChanged();
    void screenshotPreviewSourceChanged();
    void screenshotPreviewActiveChanged();
    void listeningAmplitudesChanged();

private:
    // --- State ---
    VoiceState voiceStateValue = Idle;
    OverlayMode overlayModeValue = FollowingCursor;
    InteractionMode interactionModeValue = Passive;
    InputSession inputSessionValue = NoInputSession;
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

    // --- Screenshot input mode ---
    QString pendingScreenshotPathValue;  // image grabbed on press, spawned on release
    QString screenshotPreviewSourceValue;
    bool screenshotPreviewActiveValue = false;
    QTimer screenshotPreviewHideTimer;
    void showScreenshotPreview(const QString& screenshotFilePath);
    // Spawns a task for `screenshotFilePath`, reusing pendingTaskColorValue.
    // The PNG is deleted when the task reaches a terminal status.
    void spawnScreenshotTask(const QString& screenshotFilePath);

    // --- Listening waveform amplitudes ---
    QVariantList listeningAmplitudesValue;
    QTimer listeningAmplitudeTimer;
    void tickListeningAmplitudes();
    void resetListeningAmplitudes();

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
