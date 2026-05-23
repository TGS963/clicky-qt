#include "tray_icon_manager.h"
#include "companion_state.h"

#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QPixmap>

static QIcon makeBlueDotIcon() {
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QRadialGradient gradient(QPointF(13, 13), 12);
    gradient.setColorAt(0.0, QColor("#c8dcff"));
    gradient.setColorAt(0.55, QColor("#5aa9ff"));
    gradient.setColorAt(1.0, QColor("#1f6fd6"));
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(16, 16), 12, 12);
    return QIcon(pixmap);
}

TrayIconManager::TrayIconManager(CompanionState* companionState, QObject* parent)
    : QObject(parent), companionStateValue(companionState) {

    trayIcon = new QSystemTrayIcon(makeBlueDotIcon(), this);
    trayIcon->setToolTip(QStringLiteral("Clicky"));

    QMenu* trayContextMenu = new QMenu();
    QAction* toggleDockAction = trayContextMenu->addAction(QStringLiteral("Toggle dock"));
    QAction* togglePushToTalkAction = trayContextMenu->addAction(QStringLiteral("Toggle listening"));
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
