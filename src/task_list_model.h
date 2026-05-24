#pragma once

#include "task_item.h"

#include <QAbstractListModel>
#include <QList>

// Thin list model around the active TaskItem* list. Exposes one role
// (`task`) that returns the TaskItem as a QObject. Replacing the previous
// QQmlListProperty avoids full delegate rebuilds in QML Repeaters when a
// task is added or removed — incremental beginInsertRows / beginRemoveRows
// signals let existing delegates keep their state and animate properties
// like orbital angle smoothly.
class TaskListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        TaskObjectRole = Qt::UserRole + 1,
    };

    explicit TaskListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int taskCount() const { return tasksList.size(); }
    TaskItem* taskAt(int index) const;
    bool contains(TaskItem* task) const;

    void appendTask(TaskItem* task);
    void removeTask(TaskItem* task);

private:
    QList<TaskItem*> tasksList;
};
