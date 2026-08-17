#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

QString serializeTodoList(const QStringList& list) {
    return QJsonDocument(QJsonArray::fromStringList(list)).toJson(QJsonDocument::Compact);
}

QStringList deserializeTodoList(const QString& json) {
    QJsonArray arr = QJsonDocument::fromJson(json.toUtf8()).array();
    QStringList list;
    for (const auto& v : arr) list << v.toString();
    return list;
}

QString serializeHourlyTasks(const QMap<int, QString>& tasks) {
    QJsonObject obj;
    for (auto it = tasks.constBegin(); it != tasks.constEnd(); ++it) {
        obj[QString::number(it.key())] = it.value();
    }
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QMap<int, QString> deserializeHourlyTasks(const QString& json) {
    QJsonObject obj = QJsonDocument::fromJson(json.toUtf8()).object();
    QMap<int, QString> tasks;
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        tasks[it.key().toInt()] = it.value().toString();
    }
    return tasks;
}


Database::Database(const QString& dbPath) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Failed to open database:" << db.lastError().text();
        return;
    }

    createTables();
}

Database::~Database() {
    QSqlDatabase::database().close();
}

bool Database::isOpen() const {
    return QSqlDatabase::database().isOpen();
}

bool Database::createTables() {
    QSqlQuery query;

    bool ok1 = query.exec(
        "CREATE TABLE IF NOT EXISTS user ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "userName TEXT NOT NULL UNIQUE, "
        "password TEXT NOT NULL, "
        "homeCity TEXT NOT NULL, "
        "work TEXT"
        ")"
    );
    if (!ok1) {
        qDebug() << "Failed to create user table:" << query.lastError().text();
        return false;
    }

    bool ok2 = query.exec(
        "CREATE TABLE IF NOT EXISTS productivity ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "user_id INTEGER NOT NULL, "
        "calender BLOB, "
        "hourly_tasks TEXT, "
        "todoList TEXT, "
        "FOREIGN KEY (user_id) REFERENCES user(id)"
        ")"
    );
    if (!ok2) {
        qDebug() << "Failed to create productivity table:" << query.lastError().text();
        return false;
    }

    return true;
}

 bool Database::createUser(userInfo user) const
{
    QSqlQuery query;
    
    query.prepare("INSERT INTO user (userName, password, homeCity, work) "
                  "VALUES (:username, :password, :homeCity, :work);");
    
    query.bindValue(":username", user.username);
    query.bindValue(":password", user.password);
    query.bindValue(":homeCity", user.homeCity);
    query.bindValue(":work",     user.work);
    
    return query.exec();
}

userInfo Database::getUser(QString uName)
{
    QSqlQuery query;
    userInfo temp;
    temp.username = uName;

    query.prepare("SELECT u.homeCity, u.work, p.calender, p.hourly_tasks, p.todoList "
    "FROM user u "
    "JOIN productivity p ON u.id = p.id "
    "WHERE u.userName = :username");

    query.bindValue(":username", uName);

    if (query.exec())
    {
        if (query.next())
        {
            temp.homeCity = query.value(0).toString();
            temp.work = query.value(1).toString();
            temp.productivity.calendar = query.value(2).toByteArray();
            temp.productivity.hourlyTasks = deserializeHourlyTasks(query.value(3).toString());
            temp.productivity.todoList = deserializeTodoList(query.value(4).toString());
        }
        else
        {
            qDebug() << "No user found";
        }
    }
    else
    {
        qDebug() << "Query failed to execute:" << query.lastError().text();
    }

    return temp;



}