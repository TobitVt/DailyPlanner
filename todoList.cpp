#include "todolist.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

TodoList::TodoList() {}

int TodoList::addTask(const QString& description, TaskPriority priority, const QDateTime& dueDate) {
    Task task;
    task.description = description;
    task.priority = priority;
    task.dueDate = dueDate;
    m_tasks.push_back(task);
    return m_tasks.size() - 1;
}

void TodoList::removeTask(int index) {
    if (index >= 0 && index < m_tasks.size()) {
        m_tasks.remove(index);
    }
}

void TodoList::setDone(int index, bool done) {
    if (index >= 0 && index < m_tasks.size()) {
        m_tasks[index].done = done;
    }
}

void TodoList::setPriority(int index, TaskPriority priority) {
    if (index >= 0 && index < m_tasks.size()) {
        m_tasks[index].priority = priority;
    }
}

void TodoList::setDueDate(int index, const QDateTime& dueDate) {
    if (index >= 0 && index < m_tasks.size()) {
        m_tasks[index].dueDate = dueDate;
    }
}

const QVector<Task>& TodoList::tasks() const {
    return m_tasks;
}

QVector<Task> TodoList::incompleteTasks() const {
    QVector<Task> result;
    for (const auto& t : m_tasks) {
        if (!t.done) result.push_back(t);
    }
    return result;
}

QVector<Task> TodoList::completedTasks() const {
    QVector<Task> result;
    for (const auto& t : m_tasks) {
        if (t.done) result.push_back(t);
    }
    return result;
}

bool TodoList::isEmpty() const { return m_tasks.isEmpty(); }
int TodoList::count() const { return m_tasks.size(); }
void TodoList::clear() { m_tasks.clear(); }

QString TodoList::toJson() const {
    QJsonArray arr;
    for (const auto& task : m_tasks) {
        QJsonObject obj;
        obj["description"] = task.description;
        obj["done"] = task.done;
        obj["priority"] = static_cast<int>(task.priority);
        obj["dueDate"] = task.dueDate.isValid() ? task.dueDate.toString(Qt::ISODate) : QString();
        arr.append(obj);
    }
    return QJsonDocument(arr).toJson(QJsonDocument::Compact);
}

TodoList TodoList::fromJson(const QString& json) {
    TodoList list;
    QJsonArray arr = QJsonDocument::fromJson(json.toUtf8()).array();

    for (const auto& v : arr) {
        QJsonObject obj = v.toObject();
        Task task;
        task.description = obj.value("description").toString();
        task.done = obj.value("done").toBool();
        task.priority = static_cast<TaskPriority>(obj.value("priority").toInt());
        QString dueStr = obj.value("dueDate").toString();
        if (!dueStr.isEmpty()) {
            task.dueDate = QDateTime::fromString(dueStr, Qt::ISODate);
        }
        list.m_tasks.push_back(task);
    }
    return list;
}
