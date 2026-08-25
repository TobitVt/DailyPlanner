
#include <iostream>
#include <QVector>
#include <QDebug>
#include <cstdlib>
#include <QCoreApplication>
#include <QTime>
#include <QDate>
#include <cmath>
#include <QLocale>

#include "todoList.h"
#include "weather.h"
#include "database.h"
#include "verse.h"


//set up GUI
//set up log in/ sign up
//set up user auth

int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);

    Database plannerDB("planner.db"); 

    getWeatherSummary("Pretoria");
    getForecast("Pretoria");

    getDailyVerse();



    return app.exec();
}  

