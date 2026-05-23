#pragma once

#include <QColor>
#include <QObject>

// One satellite task that orbits the cursor for a fixed lifetime, exposing
// just enough state for QML to position and fade-out the satellite dot.
// Tasks are owned by CompanionState; QML accesses them via QQmlListProperty
// and never instantiates a TaskItem directly.
class TaskItem : public QObject {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color CONSTANT)
    Q_PROPERTY(qreal orbitAngleBaseDegrees READ orbitAngleBaseDegrees CONSTANT)
    Q_PROPERTY(qreal elapsedSeconds READ elapsedSeconds NOTIFY tickedAhead)
    Q_PROPERTY(qreal totalLifetimeSeconds READ totalLifetimeSeconds CONSTANT)

public:
    explicit TaskItem(QColor color,
                      qreal orbitAngleBaseDegrees,
                      qreal totalLifetimeSeconds,
                      QObject* parent = nullptr);

    QColor color() const { return colorValue; }
    qreal orbitAngleBaseDegrees() const { return orbitAngleBaseDegreesValue; }
    qreal elapsedSeconds() const { return elapsedSecondsValue; }
    qreal totalLifetimeSeconds() const { return totalLifetimeSecondsValue; }

    bool isExpired() const { return elapsedSecondsValue >= totalLifetimeSecondsValue; }

    void advance(qreal deltaSeconds);

signals:
    void tickedAhead();

private:
    QColor colorValue;
    qreal orbitAngleBaseDegreesValue;
    qreal totalLifetimeSecondsValue;
    qreal elapsedSecondsValue = 0.0;
};
