#include "companion_state.h"
#include "cursor_position_tracker.h"
#include "global_hotkey_manager.h"
#include "overlay_window.h"
#include "task_item.h"
#include "tray_icon_manager.h"

#include <QApplication>

namespace {
constexpr auto APPLICATION_NAME = "Clicky";
constexpr auto QML_MODULE_URI = "Clicky";
constexpr int QML_MODULE_MAJOR_VERSION = 1;
constexpr int QML_MODULE_MINOR_VERSION = 0;
}  // namespace

int main(int argc, char** argv) {
    QApplication application(argc, argv);
    application.setApplicationName(QLatin1String(APPLICATION_NAME));
    application.setQuitOnLastWindowClosed(false);

    CompanionState companionState;

    // Expose CompanionState's VoiceState / OverlayMode enums and TaskItem to
    // QML under the `Clicky` import so QML can reference them by name
    // (e.g. `CompanionState.Listening`) instead of magic integers.
    qmlRegisterUncreatableType<CompanionState>(
        QML_MODULE_URI, QML_MODULE_MAJOR_VERSION, QML_MODULE_MINOR_VERSION,
        "CompanionState",
        QStringLiteral("CompanionState is provided by the application."));
    qmlRegisterUncreatableType<TaskItem>(
        QML_MODULE_URI, QML_MODULE_MAJOR_VERSION, QML_MODULE_MINOR_VERSION,
        "TaskItem",
        QStringLiteral("TaskItem instances are created by the application."));

    OverlayWindow overlayWindow(&companionState);
    overlayWindow.showFullScreenOnPrimaryDisplay();

    CursorPositionTracker cursorPositionTracker(&companionState);
    cursorPositionTracker.start();

    GlobalHotkeyManager globalHotkeyManager(&companionState);

    TrayIconManager trayIconManager(&companionState);
    trayIconManager.show();

    return application.exec();
}
