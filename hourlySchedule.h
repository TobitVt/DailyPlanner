#pragma once

#include <QString>
#include <QMap>
#include <QDate>

struct HourlyTask {
    QString description;
    bool done = false;
};

class HourlySchedules {
public:
    HourlySchedules();
    explicit HourlySchedules(const QDate& date);

    void setTask(int hour, const QString& description);
    void removeTask(int hour);
    void markDone(int hour, bool done = true);
    bool hasTask(int hour) const;

    HourlyTask taskAt(int hour) const;
    const QMap<int, HourlyTask>& allTasks() const;

    QDate date() const;
    void setDate(const QDate& date);

    void clear();

    // Serialization for storage in the Database class
    // (matches the `hourly_tasks TEXT` column).
    QString toJson() const;
    static HourlySchedules fromJson(const QString& json, const QDate& date = QDate());

private:
    QDate m_date;
    QMap<int, HourlyTask> m_tasks; // key: hour, 0-23
};