#pragma once

#include <QObject>
#include <QSystemTrayIcon>

class CompanionState;

class TrayIconManager : public QObject {
    Q_OBJECT
public:
    explicit TrayIconManager(CompanionState* companionState, QObject* parent = nullptr);

    void show();

private:
    CompanionState* companionStateValue = nullptr;
    QSystemTrayIcon* trayIcon = nullptr;
};
