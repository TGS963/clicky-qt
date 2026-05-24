#include "task_list_model.h"

#include "task_item.h"

TaskListModel::TaskListModel(QObject* parent)
    : QAbstractListModel(parent) {}

int TaskListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return tasksList.size();
}

QVariant TaskListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= tasksList.size()) {
        return {};
    }
    if (role == TaskObjectRole) {
        return QVariant::fromValue(static_cast<QObject*>(tasksList.at(index.row())));
    }
    return {};
}

QHash<int, QByteArray> TaskListModel::roleNames() const {
    static const QHash<int, QByteArray> roles{
        {TaskObjectRole, QByteArrayLiteral("task")}};
    return roles;
}

TaskItem* TaskListModel::taskAt(int index) const {
    if (index < 0 || index >= tasksList.size()) {
        return nullptr;
    }
    return tasksList.at(index);
}

bool TaskListModel::contains(TaskItem* task) const {
    return tasksList.contains(task);
}

void TaskListModel::appendTask(TaskItem* task) {
    if (!task || tasksList.contains(task)) {
        return;
    }
    beginInsertRows(QModelIndex(), tasksList.size(), tasksList.size());
    tasksList.append(task);
    endInsertRows();
}

void TaskListModel::removeTask(TaskItem* task) {
    const int rowIndex = tasksList.indexOf(task);
    if (rowIndex < 0) {
        return;
    }
    beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
    tasksList.removeAt(rowIndex);
    endRemoveRows();
}
