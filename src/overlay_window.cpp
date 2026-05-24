#include "overlay_window.h"

#include "companion_state.h"

#include <QGuiApplication>
#include <QQmlContext>
#include <QScreen>
#include <QSurfaceFormat>

namespace {
// Some minimal X11 window managers (dwm, i3 in floating mode, etc.) re-stack
// floating windows above override-redirect overlays whenever focus changes.
// Periodically raising the overlay keeps it pinned on top without disrupting
// the focused application.
constexpr int WINDOW_RESTACK_INTERVAL_MS = 200;

// Surface alpha buffer depth required for transparent compositing with the
// desktop behind the overlay.
constexpr int TRANSPARENT_SURFACE_ALPHA_BITS = 8;
}  // namespace

OverlayWindow::OverlayWindow(CompanionState* companionState, QQuickView* parent)
    : QQuickView(parent), companionStateValue(companionState) {

    QSurfaceFormat transparentSurfaceFormat = format();
    transparentSurfaceFormat.setAlphaBufferSize(TRANSPARENT_SURFACE_ALPHA_BITS);
    setFormat(transparentSurfaceFormat);

    setColor(Qt::transparent);

    setFlags(Qt::FramelessWindowHint
             | Qt::WindowStaysOnTopHint
             | Qt::Tool
             | Qt::WindowTransparentForInput
             | Qt::NoDropShadowWindowHint
             | Qt::X11BypassWindowManagerHint);

    setResizeMode(QQuickView::SizeRootObjectToView);

    rootContext()->setContextProperty("companionState", companionStateValue);

    // QML module 'Clicky' is registered via qt_add_qml_module in CMakeLists.txt;
    // module files resolve under qrc:/qt/qml/Clicky/<filename>.qml at runtime.
    loadFromModule("Clicky", "OverlayContent");

    keepAboveTimer.setInterval(WINDOW_RESTACK_INTERVAL_MS);
    connect(&keepAboveTimer, &QTimer::timeout, this, &OverlayWindow::raiseAboveOtherWindows);

    // Toggle click-through whenever focus-mode interaction changes.
    connect(companionStateValue, &CompanionState::interactionModeChanged,
            this, &OverlayWindow::applyInteractionModeFlags);
}

void OverlayWindow::showFullScreenOnPrimaryDisplay() {
    if (const QScreen* primaryScreen = QGuiApplication::primaryScreen()) {
        setGeometry(primaryScreen->geometry());
    }
    applyInteractionModeFlags();
    show();
    keepAboveTimer.start();
}

void OverlayWindow::applyInteractionModeFlags() {
    // Only the click-through flag changes per interaction mode. Re-setting
    // WindowStaysOnTopHint after show() causes X11 to re-map the window,
    // producing a visible flicker — leave it set once in the constructor.
    const bool wantClickThrough =
        companionStateValue->interactionMode() == CompanionState::Passive;
    setFlag(Qt::WindowTransparentForInput, wantClickThrough);
}

void OverlayWindow::raiseAboveOtherWindows() {
    if (isVisible()) {
        raise();
    }
}
