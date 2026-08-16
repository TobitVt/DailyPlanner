#pragma once
#include <QString>


struct verseData
{
    bool success = false;
    QString passage_id;
    int day;
    QString errorMessage;
};


// QString getpassageID();
void getDailyVerse();