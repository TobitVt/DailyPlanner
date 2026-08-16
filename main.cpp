
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

//set up database
//daily verse
//set up calender class
// set up daily hourly schedule
// set up reminders

int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);

    Database plannerDB("planner.db"); 

    getWeatherSummary("Pretoria");
    getForecast("Pretoria");

    // QString verseID = getpassageID();

    getDailyVerse();



    return app.exec();
}  

    // todoList myTodoList("todoList");

    // myTodoList.createTodoList();
 
    // while (true)
    // {
    //     std::vector<std::string>& todo_vect = myTodoList.getTodoList();

    //     std::cout << std::endl << "please select the tasks you have completed(1 - " << todo_vect.size() <<"): ";
    //     myTodoList.printList();

    //     int done;
    //     std::cin >> done;

    //     bool found = false;

    //     for (size_t i = 0; i < todo_vect.size(); i++)
    //     {
    //         int taskNumber = stoi(todo_vect[i]);

    //         if (done == taskNumber)
    //         {
    //             todo_vect.erase(todo_vect.begin() + i);
    //             myTodoList.updateList();
    //             found = true;
    //             break;
    //         }
    //     }

    //     if (!found)
    //     {
    //         std::cout << "\nInvalid task number.\n";
    //     }


    //     if (myTodoList.isEmpty()) {break;}

    // }


