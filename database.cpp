#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>


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
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS forecasts ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "city TEXT NOT NULL, "
        "temp REAL, "
        "pop REAL"
        ")"
    );

    if (!ok) {
        qDebug() << "Failed to create tables:" << query.lastError().text();
    }
    return ok;
}