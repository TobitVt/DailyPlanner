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

const QVector<Reminder> &Reminders::all() const { return m_reminders; }

QVector<Reminder> Reminders::upcoming(int maxCount) const
{
    QVector<Reminder> result;
    for (const Reminder &reminder : m_reminders)
    {
        if (!reminder.completed && reminder.due.isValid() && reminder.due >= QDateTime::currentDateTime())
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
        reminders.add(reminder);
    }
    return reminders;
}
