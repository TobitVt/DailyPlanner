#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

QString PasswordHash::hash(const QString &password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString::fromLatin1(hash.toHex());
}

bool PasswordHash::verify(const QString &password, const QString &hash)
{
    return PasswordHash::hash(password) == hash;
}

// QString serializeTodoList(const QStringList& list) {
//     return QJsonDocument(QJsonArray::fromStringList(list)).toJson(QJsonDocument::Compact);
// }

// QStringList deserializeTodoList(const QString& json) {
//     QJsonArray arr = QJsonDocument::fromJson(json.toUtf8()).array();
//     QStringList list;
//     for (const auto& v : arr) list << v.toString();
//     return list;
// }

// QString serializeHourlySchedules(const QMap<int, QString>& tasks) {
//     QJsonObject obj;
//     for (auto it = tasks.constBegin(); it != tasks.constEnd(); ++it) {
//         obj[QString::number(it.key())] = it.value();
//     }
//     return QJsonDocument(obj).toJson(QJsonDocument::Compact);
// }

// QMap<int, QString> deserializeHourlySchedules(const QString& json) {
//     QJsonObject obj = QJsonDocument::fromJson(json.toUtf8()).object();
//     QMap<int, QString> tasks;
//     for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
//         tasks[it.key().toInt()] = it.value().toString();
//     }
//     return tasks;
// }

Database::Database(const QString &dbPath)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open())
    {
        qDebug() << "Failed to open database:" << db.lastError().text();
        return;
    }

    m_ready = createTables();
}

Database::~Database()
{
    QSqlDatabase::database().close();
}

bool Database::isOpen() const
{
    return m_ready && QSqlDatabase::database().isOpen();
}

bool Database::createTables()
{
    QSqlQuery query;

    bool ok1 = query.exec(
        "CREATE TABLE IF NOT EXISTS user ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "userName TEXT NOT NULL UNIQUE, "
        "password TEXT NOT NULL, "
        "email TEXT, "
        "homeCity TEXT NOT NULL, "
        "work TEXT"
        ")");
    if (!ok1)
    {
        qDebug() << "Failed to create user table:" << query.lastError().text();
        return false;
    }
    query.exec("ALTER TABLE user ADD COLUMN email TEXT");

    bool ok2 = query.exec(
        "CREATE TABLE IF NOT EXISTS productivity ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "user_id INTEGER NOT NULL, "
        "calendar BLOB, "
        "hourly_tasks TEXT, "
        "todoList TEXT, "
        "reminders TEXT, "
        "FOREIGN KEY (user_id) REFERENCES user(id)"
        ")");
    if (!ok2)
    {
        qDebug() << "Failed to create productivity table:" << query.lastError().text();
        return false;
    }

    // Upgrade databases created before reminders were added.
    query.exec("ALTER TABLE productivity ADD COLUMN reminders TEXT");

    return true;
}

bool Database::createUser(userInfo user) const
{
    if (!isOpen() || user.username.trimmed().isEmpty() || user.password.isEmpty() ||
        user.homeCity.trimmed().isEmpty())
    {
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction())
    {
        qDebug() << "Failed to start user creation transaction:" << db.lastError().text();
        return false;
    }

    QSqlQuery query;

    query.prepare("INSERT INTO user (userName, password, email, homeCity, work) "
                  "VALUES (:username, :password, :email, :homeCity, :work)");

    query.bindValue(":username", user.username);
    query.bindValue(":password", PasswordHash::hash(user.password));
    query.bindValue(":email", user.email);
    query.bindValue(":homeCity", user.homeCity);
    query.bindValue(":work", user.work);

    if (!query.exec())
    {
        qDebug() << "Failed to create user:"
                 << query.lastError().text();
        db.rollback();
        return false;
    }

    int userId = query.lastInsertId().toInt();

    QSqlQuery productivityQuery;

    productivityQuery.prepare("INSERT INTO productivity (user_id, calendar, hourly_tasks, todoList, reminders) "
                              "VALUES (:user_id, :calendar, :hourly_tasks, :todoList, :reminders)");

    productivityQuery.bindValue(":user_id", userId);
    productivityQuery.bindValue(":calendar", QByteArray());
    productivityQuery.bindValue(":hourly_tasks", user.productivity.hourly.toJson());
    productivityQuery.bindValue(":todoList", user.productivity.todoList.toJson());
    productivityQuery.bindValue(":reminders", user.productivity.reminders.toJson());

    if (!productivityQuery.exec())
    {
        qDebug() << "Failed to create productivity record:"
                 << productivityQuery.lastError().text();
        db.rollback();
        return false;
    }

    if (!db.commit())
    {
        qDebug() << "Failed to commit user creation transaction:" << db.lastError().text();
        db.rollback();
        return false;
    }

    return true;
}

userInfo Database::getAllInfo(QString uName)
{
    QSqlQuery query;
    userInfo temp;

    query.prepare("SELECT u.password, u.email, u.homeCity, u.work, p.calendar, p.hourly_tasks, p.todoList, p.reminders "
                  "FROM user u "
                  "JOIN productivity p ON u.id = p.user_id "
                  "WHERE u.userName = :username");

    query.bindValue(":username", uName);

    if (query.exec())
    {
        if (query.next())
        {
            temp.username = uName;
            temp.password = query.value(0).toString();
            temp.email = query.value(1).toString();
            temp.homeCity = query.value(2).toString();
            temp.work = query.value(3).toString();
            temp.productivity.calendar = Calendar::fromJson(query.value(4).toByteArray());
            temp.productivity.hourly = HourlySchedules::fromJson(query.value(5).toString(), QDate::currentDate());
            temp.productivity.todoList = TodoList::fromJson(query.value(6).toString());
            temp.productivity.reminders = Reminders::fromJson(query.value(7).toString());
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
                  "WHERE user_id = (SELECT id FROM user WHERE userName = :username);");

    query.bindValue(":todoList", u.productivity.todoList.toJson());
    query.bindValue(":username", u.username);

    return query.exec();
}

bool Database::saveHourlySchedules(QMap<int, QString> hourly, userInfo u)
{
    QSqlQuery query;

    query.prepare("UPDATE productivity "
                  "SET hourly_tasks = :hourly_tasks "
                  "WHERE user_id = (SELECT id FROM user WHERE userName = :username);");

    query.bindValue(":hourly_tasks", u.productivity.hourly.toJson());
    query.bindValue(":username", u.username);

    return query.exec();
}

bool Database::saveCalendar(QByteArray cal, userInfo u)
{
    QSqlQuery query;

    query.prepare("UPDATE productivity "
                  "SET calendar = :calendar "
                  "WHERE user_id = (SELECT id FROM user WHERE userName = :username);");

    query.bindValue(":calendar", cal);
    query.bindValue(":username", u.username);

    return query.exec();
}

bool Database::saveReminders(userInfo u)
{
    QSqlQuery query;
    query.prepare("UPDATE productivity SET reminders = :reminders "
                  "WHERE user_id = (SELECT id FROM user WHERE userName = :username)");
    query.bindValue(":reminders", u.productivity.reminders.toJson());
    query.bindValue(":username", u.username);
    return query.exec();
}

bool Database::updateUserProfile(const userInfo &user)
{
    if (!isOpen() || user.username.trimmed().isEmpty() || user.homeCity.trimmed().isEmpty())
    {
        return false;
    }
    QSqlQuery query;
    query.prepare("UPDATE user SET email = :email, homeCity = :homeCity, work = :work "
                  "WHERE userName = :username");
    query.bindValue(":email", user.email.trimmed());
    query.bindValue(":homeCity", user.homeCity.trimmed());
    query.bindValue(":work", user.work.trimmed());
    query.bindValue(":username", user.username);
    return query.exec() && query.numRowsAffected() == 1;
}