#pragma once

#include "calendar.h"
#include "todoList.h"
#include "hourlySchedule.h"
#include "reminder.h"

#include <QString>
#include <QStringList>
#include <QMap>

struct prod
{
    Calendar calendar;
    TodoList todoList;
    HourlySchedules hourly;
    Reminders reminders;
};

struct userInfo
{
    QString username;
    QString password; // stored as hash
    QString email;
    QString homeCity;
    QString work;
    prod productivity;
};

class PasswordHash
{
public:
    static QString hash(const QString &password);
    static bool verify(const QString &password, const QString &hash);
};

class Database
{
public:
    explicit Database(const QString &dbPath);
    ~Database();

    bool isOpen() const;

    bool createUser(userInfo user) const;
    userInfo getAllInfo(QString uName);

    bool saveTodoList(QStringList todo, userInfo u);
    bool saveHourlySchedules(QMap<int, QString> hourly, userInfo u);
    bool saveCalendar(QByteArray cal, userInfo u);
    bool saveReminders(userInfo u);
    bool updateUserProfile(const userInfo &user);

private:
    bool createTables();
    bool m_ready = false;
};