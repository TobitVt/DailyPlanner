#pragma once

#include "hourlySchedule.h"
#include "calendar.h"
#include "todoList.h"

#include <QString>
#include <QStringList>
#include <QMap>

struct prod {
    Calendar calendar;              
    TodoList todoList;             
    HourlySchedules HourlySchedules;
};

struct userInfo{
    QString username;
    QString password;
    QString homeCity;
    QString work;
    prod productivity;
};

class Database {
public:
    explicit Database(const QString& dbPath);
    ~Database();

    bool isOpen() const;

    bool createUser(userInfo user) const;
    userInfo getAllInfo(QString uName);

    bool saveTodoList(QStringList todo, userInfo u);
    bool saveHourlySchedules(QMap<int, QString> hourly, userInfo u);
    bool saveCalendar(QByteArray cal, userInfo u);



private:
    bool createTables();
};