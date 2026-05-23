#include "cursor_position_tracker.h"

#include "companion_state.h"

#include <QCursor>

namespace {
// ~60 Hz cursor poll. QCursor::pos() is cheap, but emitting position changes
// drives QML bindings each frame, so the rate is bounded to one frame.
constexpr int CURSOR_POLL_INTERVAL_MS = 16;
}  // namespace

CursorPositionTracker::CursorPositionTracker(CompanionState* companionState, QObject* parent)
    : QObject(parent), companionStateValue(companionState) {
    pollingTimer.setInterval(CURSOR_POLL_INTERVAL_MS);
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
