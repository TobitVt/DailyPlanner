#pragma once
#include "database.h"

class HourlySchedule
{
private:
    QMap<int, QString> tasks;

public:
    void addTask(int hour, QString task);
    void removeTask(int hour);
    void editTask(int hour, QString task);

    void save(Database& db);
};