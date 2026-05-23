#include "overlay_window.h"
#include "companion_state.h"

#include <QGuiApplication>
#include <QQmlContext>
#include <QScreen>
#include <QSurfaceFormat>

OverlayWindow::OverlayWindow(CompanionState* companionState, QQuickView* parent)
    : QQuickView(parent), companionStateValue(companionState) {

    // Ensure the QQuickView's surface has an alpha buffer so transparent
    // background actually composites with the desktop behind it.
    QSurfaceFormat transparentSurfaceFormat = format();
    transparentSurfaceFormat.setAlphaBufferSize(8);
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

    // QML module 'Clicky' is registered via qt_add_qml_module in CMakeLists.
    // Qt6 places module files under qrc:/qt/qml/Clicky/<filename>.
    loadFromModule("Clicky", "OverlayContent");
}

void OverlayWindow::showFullScreenOnPrimaryDisplay() {
    const QScreen* primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen) {
        setGeometry(primaryScreen->geometry());
    }
    applyClickThroughAndAlwaysAboveAttributes();
    show();
}

void OverlayWindow::applyClickThroughAndAlwaysAboveAttributes() {
    // Qt::WindowTransparentForInput is the cross-platform click-through flag;
    // re-asserted here in case window flags were modified after show().
    setFlag(Qt::WindowTransparentForInput, true);
    setFlag(Qt::WindowStaysOnTopHint, true);
}
