#include <QApplication>
#include <QQmlApplicationEngine>

#include "companion_state.h"
#include "cursor_position_tracker.h"
#include "global_hotkey_manager.h"
#include "overlay_window.h"
#include "tray_icon_manager.h"

int main(int argc, char** argv) {
    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("Clicky"));
    application.setQuitOnLastWindowClosed(false);

    CompanionState companionState;

    // Register CompanionState enums under uri "Clicky" so QML can reference them.
    qmlRegisterUncreatableType<CompanionState>("Clicky", 1, 0, "CompanionState",
        QStringLiteral("CompanionState is provided by the application; do not instantiate from QML."));

    OverlayWindow overlayWindow(&companionState);
    overlayWindow.showFullScreenOnPrimaryDisplay();

    CursorPositionTracker cursorPositionTracker(&companionState);
    cursorPositionTracker.start();

    GlobalHotkeyManager globalHotkeyManager(&companionState);

    TrayIconManager trayIconManager(&companionState);
    trayIconManager.show();

    return application.exec();
}
