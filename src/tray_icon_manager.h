#pragma once

#include <QObject>
#include <QSystemTrayIcon>

class CompanionState;

// Creates and manages the system tray icon + context menu. The icon is
// painted in-process via QPainter so no PNG asset is required. On window
// managers without a native StatusNotifier/AppIndicator host (e.g. plain
// dwm) the icon silently fails to appear; the rest of the app is
// unaffected.
class TrayIconManager : public QObject {
    Q_OBJECT
public:
    explicit TrayIconManager(CompanionState* companionState, QObject* parent = nullptr);

    void show();

private:
    CompanionState* companionStateValue = nullptr;
    QSystemTrayIcon* trayIcon = nullptr;
};
