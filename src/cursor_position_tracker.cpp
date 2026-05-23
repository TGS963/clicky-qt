#include "cursor_position_tracker.h"
#include "companion_state.h"

#include <QCursor>

CursorPositionTracker::CursorPositionTracker(CompanionState* companionState, QObject* parent)
    : QObject(parent), companionStateValue(companionState) {
    pollingTimer.setInterval(16);
    connect(&pollingTimer, &QTimer::timeout, this, &CursorPositionTracker::pollCursorPosition);
}

void CursorPositionTracker::start() {
    pollingTimer.start();
}

void CursorPositionTracker::stop() {
    pollingTimer.stop();
}

void CursorPositionTracker::pollCursorPosition() {
    const QPoint currentCursorScreenPosition = QCursor::pos();
    companionStateValue->setCursorScreenPosition(QPointF(currentCursorScreenPosition));
}
