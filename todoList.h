#pragma once

#include <QString>
#include <QVector>
#include <QDateTime>

enum class TaskPriority
{
    Low,
    Medium,
    High
};

struct Task
{
    QString description;
    bool done = false;
    QDateTime dueDate;
    TaskPriority priority = TaskPriority::Medium;
};

class TodoList
{
public:
    TodoList();

    int addTask(const QString &description,
                TaskPriority priority = TaskPriority::Medium,
                const QDateTime &dueDate = QDateTime());

    void removeTask(int index);
    void setDone(int index, bool done = true);
    void setPriority(int index, TaskPriority priority);
    void setDescription(int index, const QString &description);
    void setDueDate(int index, const QDateTime &dueDate);

    const QVector<Task> &tasks() const;
    QVector<Task> incompleteTasks() const;
    QVector<Task> completedTasks() const;

    bool isEmpty() const;
    int count() const;

    void clear();

    // Serialization for storage in the Database class
    QString toJson() const;
    static TodoList fromJson(const QString &json);

private:
    QVector<Task> m_tasks;
};