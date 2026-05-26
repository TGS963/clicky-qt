#include "companion_state.h"
#include "cursor_position_tracker.h"
#include "global_hotkey_manager.h"
#include "modifier_key_monitor.h"
#include "overlay_window.h"
#include "screenshot_capture_controller.h"
#include "task_item.h"
#include "task_list_model.h"
#include "tray_icon_manager.h"

#include <QApplication>
#include <QGuiApplication>
#include <QList>
#include <QScreen>

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

    qmlRegisterUncreatableType<CompanionState>(
        QML_MODULE_URI, QML_MODULE_MAJOR_VERSION, QML_MODULE_MINOR_VERSION,
        "CompanionState",
        QStringLiteral("CompanionState is provided by the application."));
    qmlRegisterUncreatableType<TaskItem>(
        QML_MODULE_URI, QML_MODULE_MAJOR_VERSION, QML_MODULE_MINOR_VERSION,
        "TaskItem",
        QStringLiteral("TaskItem instances are created by the application."));
    qmlRegisterUncreatableType<TaskListModel>(
        QML_MODULE_URI, QML_MODULE_MAJOR_VERSION, QML_MODULE_MINOR_VERSION,
        "TaskListModel",
        QStringLiteral("TaskListModel is provided by the application."));

    // One overlay window per physical screen. Each window knows its screen's
    // geometry so QML can convert global cursor coords to window-local coords
    // and hide the dot when the cursor is on a different screen.
    QList<OverlayWindow*> overlayWindows;

    auto createWindowForScreen = [&](QScreen* screen) {
        auto* win = new OverlayWindow(&companionState, screen);
        win->showOnScreen();
        overlayWindows.append(win);
    };

    for (QScreen* screen : QGuiApplication::screens()) {
        createWindowForScreen(screen);
    }

    QObject::connect(&application, &QGuiApplication::screenAdded,
                     [&](QScreen* screen) { createWindowForScreen(screen); });

    QObject::connect(&application, &QGuiApplication::screenRemoved,
                     [&](QScreen* screen) {
        for (int i = 0; i < overlayWindows.size(); ++i) {
            if (overlayWindows[i]->overlayScreen() == screen) {
                overlayWindows[i]->deleteLater();
                overlayWindows.removeAt(i);
                break;
            }
        }
    });

    CursorPositionTracker cursorPositionTracker(&companionState);
    cursorPositionTracker.start();

    ModifierKeyMonitor modifierKeyMonitor(&companionState);
    modifierKeyMonitor.start();

    ScreenshotCaptureController screenshotCaptureController(&companionState, &overlayWindows);

    GlobalHotkeyManager globalHotkeyManager(&companionState, &screenshotCaptureController);

    TrayIconManager trayIconManager(&companionState);
    trayIconManager.show();

    return application.exec();
}
