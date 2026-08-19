 #pragma once
 #include "database.h"
 
 class Calendar
{
private:
    QByteArray calendarData;

public:
    void addEvent();
    void removeEvent();
    void importCalendar();

    void save(Database& db);
};