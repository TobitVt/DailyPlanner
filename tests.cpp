#include <QtTest>

#include "calendar.h"
#include "hourlySchedule.h"
#include "reminder.h"
#include "todoList.h"
#include "weather.h"

class PlannerTests : public QObject {
    Q_OBJECT

private slots:
    void taskRoundTrip();
    void calendarImport();
    void duplicateCalendarImport();
    void hourlyScheduleRoundTrip();
    void remindersRoundTrip();
    void recurringReminderAdvance();
    void weatherParsing();
};

void PlannerTests::taskRoundTrip() {
    TodoList tasks;
    const int index = tasks.addTask("Write report", TaskPriority::High,
                                   QDateTime(QDate(2026, 9, 5), QTime(10, 30)));
    tasks.setDescription(index, "Write final report");
    tasks.setDone(index, true);

    const TodoList restored = TodoList::fromJson(tasks.toJson());
    QCOMPARE(restored.count(), 1);
    QCOMPARE(restored.tasks().first().description, QString("Write final report"));
    QCOMPARE(restored.tasks().first().priority, TaskPriority::High);
    QVERIFY(restored.tasks().first().done);
}

void PlannerTests::calendarImport() {
    Calendar calendar;
    const QString ics = "BEGIN:VCALENDAR\nBEGIN:VEVENT\n"
                        "SUMMARY:Team meeting\nDTSTART:20260905T090000\n"
                        "DTEND:20260905T100000\nEND:VEVENT\nEND:VCALENDAR\n";

    QVERIFY(calendar.importFromIcsText(ics));
    QCOMPARE(calendar.events().size(), 1);
    QCOMPARE(calendar.eventsOnDate(QDate(2026, 9, 5)).first().summary,
             QString("Team meeting"));
}

void PlannerTests::duplicateCalendarImport() {
    Calendar calendar;
    const QString ics = "BEGIN:VCALENDAR\nBEGIN:VEVENT\nUID:event-1\nSUMMARY:Planning\n"
                        "DTSTART:20260905T090000\nDTEND:20260905T100000\nEND:VEVENT\nEND:VCALENDAR\n";
    QVERIFY(calendar.importFromIcsText(ics));
    QVERIFY(!calendar.importFromIcsText(ics));
    QCOMPARE(calendar.events().size(), 1);
}

void PlannerTests::hourlyScheduleRoundTrip() {
    HourlySchedules schedule(QDate(2026, 9, 5));
    schedule.setTask(9, "Planning");
    schedule.markDone(9);

    const HourlySchedules restored = HourlySchedules::fromJson(schedule.toJson(), schedule.date());
    QVERIFY(restored.hasTask(9));
    QCOMPARE(restored.taskAt(9).description, QString("Planning"));
    QVERIFY(restored.taskAt(9).done);
}

void PlannerTests::remindersRoundTrip() {
    Reminders reminders;
    reminders.add({"Submit report", QDateTime::currentDateTime().addSecs(3600), false});

    const Reminders restored = Reminders::fromJson(reminders.toJson());
    QCOMPARE(restored.all().size(), 1);
    QCOMPARE(restored.all().first().text, QString("Submit report"));
    QCOMPARE(restored.upcoming().size(), 1);
}

void PlannerTests::recurringReminderAdvance() {
    Reminders reminders;
    Reminder reminder{"Daily check", QDateTime::currentDateTime().addSecs(-60), false, 1, {}};
    reminders.add(reminder);
    reminders.advanceDueReminders();
    QVERIFY(reminders.all().first().due > QDateTime::currentDateTime());
    QCOMPARE(reminders.all().first().recurrenceDays, 1);
}

void PlannerTests::weatherParsing() {
    const QByteArray json = R"({
        "city": {"name": "London", "sunrise": 1725520000, "sunset": 1725560000},
        "list": [{
            "dt": 1725530000,
            "main": {"temp": 18.5, "feels_like": 18.0, "humidity": 72},
            "wind": {"speed": 4.0},
            "weather": [{"main": "Rain", "description": "light rain"}],
            "pop": 0.8,
            "rain": {"3h": 1.2}
        }]
    })";

    const WeatherData weather = parseWeatherData(json);
    QVERIFY(weather.success);
    QCOMPARE(weather.locationName, QString("London"));
    QCOMPARE(weather.forecast.size(), 1);
    QCOMPARE(weather.forecast.first().condition, QString("Rain"));
    QCOMPARE(weather.forecast.first().humidity, 72);
}

QTEST_MAIN(PlannerTests)
#include "tests.moc"
