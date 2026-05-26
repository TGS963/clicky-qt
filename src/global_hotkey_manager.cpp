#include "global_hotkey_manager.h"

#include "companion_state.h"
#include "screenshot_capture_controller.h"

#include <QHotkey>
#include <QKeySequence>

namespace {
// System-wide shortcuts. Kept in this single translation unit so changing them
// requires touching exactly one place.
constexpr auto PUSH_TO_TALK_TOGGLE_SHORTCUT = "Ctrl+Alt+Space";
constexpr auto DOCK_TOGGLE_SHORTCUT = "Ctrl+Alt+D";
constexpr auto SCREENSHOT_CAPTURE_SHORTCUT = "Ctrl+Alt+S";
}  // namespace

GlobalHotkeyManager::GlobalHotkeyManager(
    CompanionState* companionState,
    ScreenshotCaptureController* screenshotCaptureController,
    QObject* parent)
    : QObject(parent),
      companionStateValue(companionState),
      screenshotCaptureControllerValue(screenshotCaptureController) {

    pushToTalkToggleHotkey = new QHotkey(
        QKeySequence(QLatin1String(PUSH_TO_TALK_TOGGLE_SHORTCUT)), true, this);
    connect(pushToTalkToggleHotkey, &QHotkey::activated, this, [this]() {
        companionStateValue->togglePushToTalkListening();
    });

    dockToggleHotkey = new QHotkey(
        QKeySequence(QLatin1String(DOCK_TOGGLE_SHORTCUT)), true, this);
    connect(dockToggleHotkey, &QHotkey::activated, this, [this]() {
        companionStateValue->toggleOverlayMode();
    });

    screenshotCaptureHotkey = new QHotkey(
        QKeySequence(QLatin1String(SCREENSHOT_CAPTURE_SHORTCUT)), true, this);
    connect(screenshotCaptureHotkey, &QHotkey::activated, this, [this]() {
        screenshotCaptureControllerValue->captureCursorMarkedScreenshotAndSpawnTask();
    });
}

