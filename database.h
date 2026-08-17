#pragma once

#include <QString>
#include <QStringList>
#include <QMap>

struct prod {
    QByteArray calendar;              
    QStringList todoList;             
    QMap<int, QString> hourlyTasks;
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
    userInfo getUser(QString uName);



private:
    bool createTables();
};