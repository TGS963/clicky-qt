#pragma once

#include <QObject>

class QHotkey;
class CompanionState;

// Registers and owns the application's system-wide hotkeys. Each hotkey
// dispatches into CompanionState. Hotkey strings live as private constants
// in global_hotkey_manager.cpp.
class GlobalHotkeyManager : public QObject {
    Q_OBJECT
public:
    explicit GlobalHotkeyManager(CompanionState* companionState, QObject* parent = nullptr);

private:
    CompanionState* companionStateValue = nullptr;
    QHotkey* pushToTalkToggleHotkey = nullptr;
    QHotkey* dockToggleHotkey = nullptr;
};
