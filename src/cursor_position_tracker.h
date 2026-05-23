#pragma once

#include <QObject>
#include <QTimer>

class CompanionState;

// Periodically samples the global cursor position via QCursor::pos() and
// pushes it into CompanionState. The QML overlay binds to the resulting
// cursorScreenPosition property to render the agent dot near the cursor.
class CursorPositionTracker : public QObject {
    Q_OBJECT
public:
    explicit CursorPositionTracker(CompanionState* companionState, QObject* parent = nullptr);

    void start();
    void stop();

private slots:
    void pollCursorPosition();

private:
    CompanionState* companionStateValue = nullptr;
    QTimer pollingTimer;
};
