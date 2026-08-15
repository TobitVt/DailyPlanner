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

    // createTables();
}

Database::~Database() {
    QSqlDatabase::database().close();
}

bool Database::isOpen() const {
    return QSqlDatabase::database().isOpen();
}