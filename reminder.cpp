#include "reminder.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>

void Reminders::add(const Reminder &reminder)
{
    m_reminders.append(reminder);
}

void Reminders::remove(int index)
{
    if (index >= 0 && index < m_reminders.size())
        m_reminders.remove(index);
}

void Reminders::setCompleted(int index, bool completed)
{
    if (index >= 0 && index < m_reminders.size())
        m_reminders[index].completed = completed;
}

void Reminders::snooze(int index, int minutes)
{
    if (index >= 0 && index < m_reminders.size())
        m_reminders[index].snoozedUntil = QDateTime::currentDateTime().addSecs(minutes * 60);
}

void Reminders::advanceDueReminders()
{
    const QDateTime now = QDateTime::currentDateTime();
    for (Reminder& reminder : m_reminders) {
        if (!reminder.completed && reminder.recurrenceDays > 0 && reminder.due.isValid() && reminder.due <= now) {
            do {
                reminder.due = reminder.due.addDays(reminder.recurrenceDays);
            } while (reminder.due <= now);
            reminder.snoozedUntil = {};
        }
    }
}

const QVector<Reminder> &Reminders::all() const { return m_reminders; }

QVector<Reminder> Reminders::upcoming(int maxCount) const
{
    QVector<Reminder> result;
    for (const Reminder &reminder : m_reminders)
    {
        if (!reminder.completed && reminder.due.isValid() &&
            reminder.due >= QDateTime::currentDateTime() &&
            (!reminder.snoozedUntil.isValid() || reminder.snoozedUntil <= QDateTime::currentDateTime()))
        {
            result.append(reminder);
        }
    }
    std::sort(result.begin(), result.end(), [](const Reminder &left, const Reminder &right)
              { return left.due < right.due; });
    if (maxCount >= 0 && result.size() > maxCount)
        result.resize(maxCount);
    return result;
}

QString Reminders::toJson() const
{
    QJsonArray array;
    for (const Reminder &reminder : m_reminders)
    {
        QJsonObject object;
        object["text"] = reminder.text;
        object["due"] = reminder.due.toString(Qt::ISODate);
        object["completed"] = reminder.completed;
        object["recurrenceDays"] = reminder.recurrenceDays;
        object["snoozedUntil"] = reminder.snoozedUntil.toString(Qt::ISODate);
        array.append(object);
    }
    return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
}

Reminders Reminders::fromJson(const QString &json)
{
    Reminders reminders;
    const QJsonArray array = QJsonDocument::fromJson(json.toUtf8()).array();
    for (const QJsonValue &value : array)
    {
        const QJsonObject object = value.toObject();
        Reminder reminder;
        reminder.text = object["text"].toString();
        reminder.due = QDateTime::fromString(object["due"].toString(), Qt::ISODate);
        reminder.completed = object["completed"].toBool();
        reminder.recurrenceDays = object["recurrenceDays"].toInt();
        reminder.snoozedUntil = QDateTime::fromString(object["snoozedUntil"].toString(), Qt::ISODate);
        reminders.add(reminder);
    }
    return reminders;
}
