#pragma once

#include <QDateTime>
#include <QString>
#include <QVector>

struct Reminder {
    QString text;
    QDateTime due;
    bool completed = false;
};

class Reminders {
public:
    void add(const Reminder& reminder);
    void remove(int index);
    void setCompleted(int index, bool completed);
    const QVector<Reminder>& all() const;
    QVector<Reminder> upcoming(int maxCount = 5) const;
    QString toJson() const;
    static Reminders fromJson(const QString& json);

private:
    QVector<Reminder> m_reminders;
};
