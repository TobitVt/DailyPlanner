#pragma once

#include <QString>

class Database {
public:
    explicit Database(const QString& dbPath);
    ~Database();

    bool isOpen() const;

// private:
//     bool createTables();
};