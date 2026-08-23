#include "hourlySchedule.h"


HourlySchedules::HourlySchedules() : m_date(QDate::currentDate()) {}

HourlySchedules::HourlySchedules(const QDate& date) : m_date(date) {}

void HourlySchedules::setTask(int hour, const QString& description) {
    if (hour < 0 || hour > 23) return;
    m_tasks[hour] = {description, false};
}

void HourlySchedules::removeTask(int hour) {
    m_tasks.remove(hour);
}

void HourlySchedules::markDone(int hour, bool done) {
    if (m_tasks.contains(hour)) {
        m_tasks[hour].done = done;
    }
}

bool HourlySchedules::hasTask(int hour) const {
    return m_tasks.contains(hour);
}

HourlyTask HourlySchedules::taskAt(int hour) const {
    return m_tasks.value(hour, {"", false});
}

const QMap<int, HourlyTask>& HourlySchedules::allTasks() const {
    return m_tasks;
}

QDate HourlySchedules::date() const {
    return m_date;
}

void HourlySchedules::setDate(const QDate& date) {
    m_date = date;
}

void HourlySchedules::clear() {
    m_tasks.clear();
}

QString HourlySchedules::toJson() const{
    //serializeHourlyTasks(const QMap<int, QString>& tasks)
}

HourlySchedules HourlySchedules::fromJson(const QString& json, const QDate& date = QDate()){
    // deserializeHourlyTasks(const QString& json)
}




