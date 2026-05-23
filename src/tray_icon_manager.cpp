#include "tray_icon_manager.h"

#include "companion_state.h"

#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QPixmap>

namespace {
constexpr int TRAY_ICON_PIXEL_SIZE = 32;
constexpr qreal TRAY_ICON_HIGHLIGHT_CENTER_X = 13.0;
constexpr qreal TRAY_ICON_HIGHLIGHT_CENTER_Y = 13.0;
constexpr qreal TRAY_ICON_RADIUS_PX = 12.0;
constexpr qreal TRAY_ICON_DRAW_CENTER_PX = 16.0;

QIcon makeBlueDotIcon() {
    QPixmap pixmap(TRAY_ICON_PIXEL_SIZE, TRAY_ICON_PIXEL_SIZE);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QRadialGradient gradient(
        QPointF(TRAY_ICON_HIGHLIGHT_CENTER_X, TRAY_ICON_HIGHLIGHT_CENTER_Y),
        TRAY_ICON_RADIUS_PX);
    gradient.setColorAt(0.0, QColor("#c8dcff"));
    gradient.setColorAt(0.55, QColor("#5aa9ff"));
    gradient.setColorAt(1.0, QColor("#1f6fd6"));

    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(
        QPointF(TRAY_ICON_DRAW_CENTER_PX, TRAY_ICON_DRAW_CENTER_PX),
        TRAY_ICON_RADIUS_PX, TRAY_ICON_RADIUS_PX);
    return QIcon(pixmap);
}
}  // namespace

TrayIconManager::TrayIconManager(CompanionState* companionState, QObject* parent)
    : QObject(parent), companionStateValue(companionState) {

    trayIcon = new QSystemTrayIcon(makeBlueDotIcon(), this);
    trayIcon->setToolTip(QStringLiteral("Clicky"));

    auto* trayContextMenu = new QMenu();
    QAction* toggleDockAction = trayContextMenu->addAction(QStringLiteral("Toggle dock"));
    QAction* togglePushToTalkAction =
        trayContextMenu->addAction(QStringLiteral("Toggle listening"));
    trayContextMenu->addSeparator();
    QAction* quitAction = trayContextMenu->addAction(QStringLiteral("Quit"));

    connect(toggleDockAction, &QAction::triggered, this, [this]() {
        companionStateValue->toggleOverlayMode();
    });
    connect(togglePushToTalkAction, &QAction::triggered, this, [this]() {
        companionStateValue->togglePushToTalkListening();
    });
    connect(quitAction, &QAction::triggered, this, []() {
        QApplication::quit();
    });

    trayIcon->setContextMenu(trayContextMenu);
}

void TrayIconManager::show() {
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        trayIcon->show();
    }
}
