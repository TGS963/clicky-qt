#include "overlay_window.h"

#include "companion_state.h"

#ifdef Q_OS_MACOS
#include "mac_window_config.h"
#endif

#include <QGuiApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScreen>
#include <QSurfaceFormat>

namespace {
// X11 WMs (dwm, i3) re-stack windows on focus change; periodic raise counters that.
constexpr int WINDOW_RESTACK_INTERVAL_MS = 200;
constexpr int TRANSPARENT_SURFACE_ALPHA_BITS = 8;
}  // namespace

OverlayWindow::OverlayWindow(CompanionState* companionState, QScreen* screen,
                             QQuickView* parent)
    : QQuickView(parent), companionStateValue(companionState), screenValue(screen) {

    QSurfaceFormat transparentSurfaceFormat = format();
    transparentSurfaceFormat.setAlphaBufferSize(TRANSPARENT_SURFACE_ALPHA_BITS);
    setFormat(transparentSurfaceFormat);

    setColor(Qt::transparent);

    setFlags(Qt::FramelessWindowHint
             | Qt::WindowStaysOnTopHint
             | Qt::Tool
             | Qt::WindowTransparentForInput
             | Qt::NoDropShadowWindowHint
             | Qt::X11BypassWindowManagerHint
             | Qt::WindowDoesNotAcceptFocus);

    setResizeMode(QQuickView::SizeRootObjectToView);

    rootContext()->setContextProperty("companionState", companionStateValue);

    // Pass this screen's geometry to QML so it can:
    //   1. convert global cursor coords to window-local coords
    //   2. show the dot/menu only when content belongs to this screen
    const QRect geom = screen->geometry();
    rootContext()->setContextProperty("screenRect",
                                      QRectF(geom.x(), geom.y(), geom.width(), geom.height()));

    // QML files are embedded at qrc:/Clicky/qml/*.qml; add the root so
    // 'import Clicky' inside QML resolves the module's qmldir from QRC.
    engine()->addImportPath(QStringLiteral("qrc:/"));

    setSource(QUrl(QStringLiteral("qrc:/Clicky/qml/OverlayContent.qml")));

    keepAboveTimer.setInterval(WINDOW_RESTACK_INTERVAL_MS);
    connect(&keepAboveTimer, &QTimer::timeout, this, &OverlayWindow::raiseAboveOtherWindows);

    connect(companionStateValue, &CompanionState::interactionModeChanged,
            this, &OverlayWindow::applyInteractionModeFlags);
}

void OverlayWindow::showOnScreen() {
    setGeometry(screenValue->geometry());
    show();
    // Must come after show() so the native window handle exists.
    applyInteractionModeFlags();
#ifdef Q_OS_MACOS
    configureMacOverlayWindow(winId());
#endif
    // X11-only keep-above workaround — not needed on macOS or Wayland.
    if (QGuiApplication::platformName() == QLatin1String("xcb")) {
        keepAboveTimer.start();
    }
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
