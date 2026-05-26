#include "cursor_marked_screenshot_capturer.h"

#include <QColor>
#include <QCursor>
#include <QDateTime>
#include <QDir>
#include <QGuiApplication>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPoint>
#include <QScreen>

namespace {

// Marker ring drawn around the cursor on the captured screenshot.
constexpr int MARKER_RING_RADIUS_PX = 48;
constexpr int MARKER_RING_THICKNESS_PX = 4;
const QColor MARKER_RING_COLOR = QColor(0, 122, 255);  // primary-dot blue

QString uniqueScreenshotFilePath() {
    const QString fileName =
        QStringLiteral("clicky-screenshot-%1.png")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-hhmmss-zzz")));
    return QDir(QDir::tempPath()).filePath(fileName);
}

}  // namespace

QString CursorMarkedScreenshotCapturer::captureScreenWithCursorMarker() {
    const QPoint cursorScreenPosition = QCursor::pos();

    QScreen* targetScreen = QGuiApplication::screenAt(cursorScreenPosition);
    if (!targetScreen) {
        targetScreen = QGuiApplication::primaryScreen();
    }
    if (!targetScreen) {
        return {};
    }

    QPixmap screenshotPixmap = targetScreen->grabWindow(0);
    if (screenshotPixmap.isNull()) {
        return {};
    }

    // grabWindow returns pixels relative to the screen's own top-left, so
    // convert the global cursor position into screen-local coordinates.
    const QPoint cursorLocalPosition =
        cursorScreenPosition - targetScreen->geometry().topLeft();

    QPainter painter(&screenshotPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(MARKER_RING_COLOR, MARKER_RING_THICKNESS_PX));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(cursorLocalPosition, MARKER_RING_RADIUS_PX, MARKER_RING_RADIUS_PX);
    painter.end();

    const QString screenshotFilePath = uniqueScreenshotFilePath();
    if (!screenshotPixmap.save(screenshotFilePath, "PNG")) {
        return {};
    }
    return screenshotFilePath;
}
