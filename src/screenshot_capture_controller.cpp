#include "screenshot_capture_controller.h"

#include "companion_state.h"
#include "overlay_window.h"

#include <QString>
#include <QTimer>

namespace {
// How long to wait after hiding the overlays before grabbing, giving the
// compositor time to repaint without Clicky's dot in the frame.
constexpr int OVERLAY_HIDE_SETTLE_MS = 50;
}  // namespace

ScreenshotCaptureController::ScreenshotCaptureController(
    CompanionState* companionState,
    QList<OverlayWindow*>* overlayWindows,
    QObject* parent)
    : QObject(parent),
      companionStateValue(companionState),
      overlayWindowsValue(overlayWindows) {}

void ScreenshotCaptureController::captureCursorMarkedScreenshotAndSpawnTask() {
    const CompanionState::InputSession session = companionStateValue->inputSession();

    // Second press: finish our own session and spawn the task.
    if (session == CompanionState::ScreenshotListening) {
        companionStateValue->finishScreenshotListeningAndSpawnTask();
        return;
    }

    // Busy with the voice mode, or a grab from a previous press is still
    // settling — ignore the key.
    if (session != CompanionState::NoInputSession || captureInFlight) {
        return;
    }

    // First press: overlays must be hidden before the grab so our own dot
    // isn't captured (see class doc for the full cycle). inputSession only
    // flips to ScreenshotListening once the deferred grab completes, so
    // captureInFlight guards against a second press during the settle delay.
    captureInFlight = true;
    for (OverlayWindow* overlayWindow : *overlayWindowsValue) {
        overlayWindow->hide();
    }

    QTimer::singleShot(OVERLAY_HIDE_SETTLE_MS, this, [this]() {
        const QString screenshotFilePath = capturer.captureScreenWithCursorMarker();

        for (OverlayWindow* overlayWindow : *overlayWindowsValue) {
            overlayWindow->showOnScreen();
        }

        captureInFlight = false;
        if (screenshotFilePath.isEmpty()) {
            qWarning("ScreenshotCaptureController: screen grab failed; staying idle.");
            return;
        }
        companionStateValue->beginScreenshotListening(screenshotFilePath);
    });
}
