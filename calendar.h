#pragma once

#include <QString>
#include <QVector>
#include <QDateTime>
#include <QByteArray>

struct CalendarEvent {
    QString summary;
    QString description;
    QDateTime start;
    QDateTime end;
    bool allDay = false;
};

class Calendar {
public:
    Calendar();

    // Import events from a raw .ics file's contents
    bool importFromIcsText(const QString& icsText);
    bool importFromFile(const QString& filePath);

    // Manual event management.
    void addEvent(const CalendarEvent& event);
    void removeEvent(int index);
    void clear();

    const QVector<CalendarEvent>& events() const;
    QVector<CalendarEvent> eventsOnDate(const QDate& date) const;
    QVector<CalendarEvent> upcomingEvents(int maxCount = 5) const;

    // Serialization for storage in the Database class 
    QByteArray toJson() const;
    static Calendar fromJson(const QByteArray& data);

private:
    QVector<CalendarEvent> m_events;

    static CalendarEvent parseVEventBlock(const QVector<QString>& lines);
};