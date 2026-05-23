#include "task_item.h"

TaskItem::TaskItem(QColor color,
                   qreal orbitAngleBaseDegrees,
                   qreal totalLifetimeSeconds,
                   QObject* parent)
    : QObject(parent),
      colorValue(std::move(color)),
      orbitAngleBaseDegreesValue(orbitAngleBaseDegrees),
      totalLifetimeSecondsValue(totalLifetimeSeconds) {}

void TaskItem::advance(qreal deltaSeconds) {
    elapsedSecondsValue += deltaSeconds;
    emit tickedAhead();
}
