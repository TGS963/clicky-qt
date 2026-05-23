#pragma once

#include <QObject>

class QHotkey;
class CompanionState;

class GlobalHotkeyManager : public QObject {
    Q_OBJECT
public:
    explicit GlobalHotkeyManager(CompanionState* companionState, QObject* parent = nullptr);
    ~GlobalHotkeyManager() override;

private:
    CompanionState* companionStateValue = nullptr;
    QHotkey* pushToTalkToggleHotkey = nullptr;
    QHotkey* dockToggleHotkey = nullptr;
};
