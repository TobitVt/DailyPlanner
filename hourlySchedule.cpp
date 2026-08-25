#include "hourlySchedule.h"

#include <QJsonDocument>
#include <QJsonObject>


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
    QJsonObject obj;
    for (auto it = m_tasks.constBegin(); it != m_tasks.constEnd(); ++it) {
        QJsonObject taskObj;
        taskObj["description"] = it.value().description;
        taskObj["done"] = it.value().done;
        obj[QString::number(it.key())] = taskObj;
    }
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

HourlySchedules HourlySchedules::fromJson(const QString& json, const QDate& date = QDate()){
    HourlySchedules result(date);
    QJsonObject obj = QJsonDocument::fromJson(json.toUtf8()).object();

    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        QJsonObject taskObj = it.value().toObject();
        HourlyTask task;
        task.description = taskObj.value("description").toString();
        task.done = taskObj.value("done").toBool();
        result.m_tasks[it.key().toInt()] = task;
    }
    return result;
}




