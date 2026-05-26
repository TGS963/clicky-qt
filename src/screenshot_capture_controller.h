#pragma once

#include "cursor_marked_screenshot_capturer.h"

#include <QList>
#include <QObject>

class CompanionState;
class OverlayWindow;

// Orchestrates the screenshot input mode (screenshot + voice context). It is
// a two-press toggle on one hotkey:
//   press 1 -> hide Clicky's own overlays (so the dot / satellites don't end
//              up in the shot), grab + mark the screen, re-show the overlays,
//              and enter the voice-context listening state.
//   press 2 -> stop listening and spawn a task for the grabbed image.
//
// The hide step is asynchronous, so this lives in its own controller rather
// than in CompanionState (which owns the listening/flash/task state). It reads
// main's overlay-window list live at capture time, so windows added/removed
// for hot-plugged screens are handled automatically.
class ScreenshotCaptureController : public QObject {
    Q_OBJECT
public:
    ScreenshotCaptureController(CompanionState* companionState,
                                QList<OverlayWindow*>* overlayWindows,
                                QObject* parent = nullptr);

public slots:
    void captureCursorMarkedScreenshotAndSpawnTask();

private:
    CompanionState* companionStateValue = nullptr;
    QList<OverlayWindow*>* overlayWindowsValue = nullptr;
    CursorMarkedScreenshotCapturer capturer;
    // Set while a first-press grab is mid-flight (overlays hidden, awaiting the
    // settle delay), blocking re-entry before inputSession flips. The two-press
    // toggle state itself lives in CompanionState::inputSession.
    bool captureInFlight = false;
};
