#include "calendar.h"

#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

#include <algorithm>

namespace {

QString unescapeIcsText(QString value) {
    value.replace(QStringLiteral("\\n"), QStringLiteral("\n"));
    value.replace(QStringLiteral("\\N"), QStringLiteral("\n"));
    value.replace(QStringLiteral("\\,"), QStringLiteral(","));
    value.replace(QStringLiteral("\\;"), QStringLiteral(";"));
    value.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));
    return value;
}

QDateTime parseIcsDateTime(const QString& value, bool* dateOnly) {
    if (dateOnly) *dateOnly = false;

    if (value.size() == 8) {
        const QDate date = QDate::fromString(value, QStringLiteral("yyyyMMdd"));
        if (date.isValid()) {
            if (dateOnly) *dateOnly = true;
            return QDateTime(date, QTime(0, 0), Qt::LocalTime);
        }
    }

    QString dateTimeValue = value;
    const bool isUtc = dateTimeValue.endsWith(QLatin1Char('Z'));
    if (isUtc) dateTimeValue.chop(1);

    const QDate date = QDate::fromString(dateTimeValue.left(8), QStringLiteral("yyyyMMdd"));
    const QTime time = QTime::fromString(dateTimeValue.mid(9), QStringLiteral("HHmmss"));
    if (!date.isValid() || !time.isValid()) return {};

    return QDateTime(date, time, isUtc ? Qt::UTC : Qt::LocalTime);
}

}

Calendar::Calendar() {}

void Calendar::addEvent(const CalendarEvent& event) {
    m_events.push_back(event);
}

void Calendar::removeEvent(int index) {
    if (index >= 0 && index < m_events.size()) {
        m_events.remove(index);
    }
}

void Calendar::clear() {
    m_events.clear();
}

const QVector<CalendarEvent>& Calendar::events() const {
    return m_events;
}

QVector<CalendarEvent> Calendar::eventsOnDate(const QDate& date) const {
    QVector<CalendarEvent> result;
    for (const auto& e : m_events) {
        if (e.start.date() == date) result.push_back(e);
    }
    return result;
}

QVector<CalendarEvent> Calendar::upcomingEvents(int maxCount) const {
    if (maxCount <= 0) return {};

    const QDateTime now = QDateTime::currentDateTime();
    QVector<CalendarEvent> upcoming;
    for (const CalendarEvent& event : m_events) {
        if (event.start.isValid() && event.start >= now) upcoming.push_back(event);
    }

    std::sort(upcoming.begin(), upcoming.end(), [](const CalendarEvent& left,
                                                   const CalendarEvent& right) {
        return left.start < right.start;
    });
    if (upcoming.size() > maxCount) upcoming.resize(maxCount);
    return upcoming;
}

QByteArray Calendar::toJson() const {
    QJsonArray arr;
    for (const auto& e : m_events) {
        QJsonObject obj;
        obj["summary"] = e.summary;
        obj["description"] = e.description;
        obj["start"] = e.start.toString(Qt::ISODate);
        obj["end"] = e.end.toString(Qt::ISODate);
        obj["allDay"] = e.allDay;
        arr.append(obj);
    }
    return QJsonDocument(arr).toJson(QJsonDocument::Compact);
}

Calendar Calendar::fromJson(const QByteArray& data) {
    Calendar cal;
    QJsonArray arr = QJsonDocument::fromJson(data).array();
    for (const auto& v : arr) {
        QJsonObject obj = v.toObject();
        CalendarEvent e;
        e.summary = obj.value("summary").toString();
        e.description = obj.value("description").toString();
        e.start = QDateTime::fromString(obj.value("start").toString(), Qt::ISODate);
        e.end = QDateTime::fromString(obj.value("end").toString(), Qt::ISODate);
        e.allDay = obj.value("allDay").toBool();
        cal.m_events.push_back(e);
    }
    return cal;
}

bool Calendar::importFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    return importFromIcsText(content);
}

bool Calendar::importFromIcsText(const QString& icsText) {
    QVector<QString> block;
    bool imported = false;
    const QStringList rawLines = icsText.split(QLatin1Char('\n'));
    QStringList lines;
    for (QString line : rawLines) {
        if (line.endsWith(QLatin1Char('\r'))) line.chop(1);
        if (!lines.isEmpty() && (line.startsWith(QLatin1Char(' ')) ||
                                 line.startsWith(QLatin1Char('\t')))) {
            lines.last() += line.mid(1);
        } else {
            lines.append(line);
        }
    }

    for (const QString& line : lines) {
        if (line.compare(QStringLiteral("BEGIN:VEVENT"), Qt::CaseInsensitive) == 0 ||
            line.compare(QStringLiteral("BEGIN:VTODO"), Qt::CaseInsensitive) == 0) {
            block.clear();
            block.append(line);
        } else if (!block.isEmpty()) {
            block.append(line);
            if (line.compare(QStringLiteral("END:VEVENT"), Qt::CaseInsensitive) == 0 ||
                line.compare(QStringLiteral("END:VTODO"), Qt::CaseInsensitive) == 0) {
                const CalendarEvent event = parseVEventBlock(block);
                if (event.start.isValid()) {
                    addEvent(event);
                    imported = true;
                }
                block.clear();
            }
        }
    }
    return imported;
}

CalendarEvent Calendar::parseVEventBlock(const QVector<QString>& lines) {
    CalendarEvent event;
    bool startIsDateOnly = false;

    for (const QString& line : lines) {
        const int separator = line.indexOf(QLatin1Char(':'));
        if (separator < 0) continue;

        const QString property = line.left(separator).section(QLatin1Char(';'), 0, 0).toUpper();
        const QString value = line.mid(separator + 1);
        if (property == QStringLiteral("SUMMARY")) {
            event.summary = unescapeIcsText(value);
        } else if (property == QStringLiteral("DESCRIPTION")) {
            event.description = unescapeIcsText(value);
        } else if (property == QStringLiteral("DTSTART")) {
            event.start = parseIcsDateTime(value, &startIsDateOnly);
        } else if (property == QStringLiteral("DTEND")) {
            event.end = parseIcsDateTime(value, nullptr);
        } else if (property == QStringLiteral("DUE")) {
            event.start = parseIcsDateTime(value, &startIsDateOnly);
            event.end = event.start;
        }
    }

    event.allDay = startIsDateOnly;
    if (event.start.isValid() && !event.end.isValid()) {
        event.end = event.allDay ? event.start.addDays(1) : event.start;
    }
    return event;
}