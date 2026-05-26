#pragma once

#include <QObject>

class QHotkey;
class CompanionState;
class ScreenshotCaptureController;

// Registers and owns the application's system-wide hotkeys. Each hotkey
// dispatches into CompanionState or ScreenshotCaptureController. Hotkey
// strings live as private constants in global_hotkey_manager.cpp.
class GlobalHotkeyManager : public QObject {
    Q_OBJECT
public:
    GlobalHotkeyManager(CompanionState* companionState,
                        ScreenshotCaptureController* screenshotCaptureController,
                        QObject* parent = nullptr);

private:
    CompanionState* companionStateValue = nullptr;
    ScreenshotCaptureController* screenshotCaptureControllerValue = nullptr;
    QHotkey* pushToTalkToggleHotkey = nullptr;
    QHotkey* dockToggleHotkey = nullptr;
    QHotkey* screenshotCaptureHotkey = nullptr;
};
