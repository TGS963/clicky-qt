#include "global_hotkey_manager.h"
#include "companion_state.h"

#include <QHotkey>
#include <QKeySequence>

GlobalHotkeyManager::GlobalHotkeyManager(CompanionState* companionState, QObject* parent)
    : QObject(parent), companionStateValue(companionState) {

    // Push-to-talk toggle (no AI yet — just flips Listening on/off for UX testing).
    pushToTalkToggleHotkey = new QHotkey(
        QKeySequence(QStringLiteral("Ctrl+Alt+Space")), true, this);
    connect(pushToTalkToggleHotkey, &QHotkey::activated, this, [this]() {
        companionStateValue->togglePushToTalkListening();
    });

    // Dock toggle — flies agent to top-right corner or back to cursor.
    dockToggleHotkey = new QHotkey(
        QKeySequence(QStringLiteral("Ctrl+Alt+D")), true, this);
    connect(dockToggleHotkey, &QHotkey::activated, this, [this]() {
        companionStateValue->toggleOverlayMode();
    });
}

GlobalHotkeyManager::~GlobalHotkeyManager() = default;
