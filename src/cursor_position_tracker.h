#pragma once

#include <QObject>
#include <QTimer>

class CompanionState;

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
