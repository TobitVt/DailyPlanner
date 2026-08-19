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
        "calendar BLOB, "
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
                  "VALUES (:username, :password, :homeCity, :work)");

    query.bindValue(":username", user.username);
    query.bindValue(":password", user.password);
    query.bindValue(":homeCity", user.homeCity);
    query.bindValue(":work", user.work);

    if (!query.exec())
    {
        qDebug() << "Failed to create user:"
                 << query.lastError().text();
        return false;
    }

    int userId = query.lastInsertId().toInt();

    QSqlQuery productivityQuery;

    productivityQuery.prepare("INSERT INTO productivity (user_id, calendar, hourly_tasks, todoList) "
                              "VALUES (:user_id, :calendar, :hourly_tasks, :todoList)");

    productivityQuery.bindValue(":user_id", userId);
    productivityQuery.bindValue(":calendar", QByteArray());
    productivityQuery.bindValue(":hourly_tasks", serializeHourlyTasks({}));
    productivityQuery.bindValue(":todoList", serializeTodoList({}));

    if (!productivityQuery.exec())
    {
        qDebug() << "Failed to create productivity record:"
                 << productivityQuery.lastError().text();
        return false;
    }

    return true;
}

userInfo Database::getAllInfo(QString uName)
{
    QSqlQuery query;
    userInfo temp;
    temp.username = uName;

    query.prepare("SELECT u.homeCity, u.work, p.calendar, p.hourly_tasks, p.todoList "
    "FROM user u "
    "JOIN productivity p ON u.id = p.user_id "
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

bool Database::saveTodoList(QStringList todo, userInfo u)
{
    QSqlQuery query;
    
    query.prepare("UPDATE productivity "
                "SET todoList = :todoList "
                "WHERE id = (SELECT id FROM user WHERE userName = :username);");
    
    query.bindValue(":todoList", serializeTodoList(todo));
    query.bindValue(":username", u.username);

    
    return query.exec();
}

bool Database::saveHourlyTasks(QMap<int, QString> hourly, userInfo u)
{
    QSqlQuery query;
    
    query.prepare("UPDATE productivity "
                "SET hourly_tasks = :hourly_tasks "
                "WHERE id = (SELECT id FROM user WHERE userName = :username);");
        
    query.bindValue(":hourly_tasks", serializeHourlyTasks(hourly));
    query.bindValue(":username", u.username);

    
    return query.exec();
}

bool Database::saveCalendar(QByteArray cal, userInfo u)
{
    QSqlQuery query;
    
    query.prepare("UPDATE productivity "
                "SET calendar = :calendar "
                "WHERE id = (SELECT id FROM user WHERE userName = :username);");
    
    query.bindValue(":calendar", cal);
    query.bindValue(":username", u.username);

    
    return query.exec();
}