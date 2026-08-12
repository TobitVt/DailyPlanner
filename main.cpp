
#include <iostream>
#include <vector>
#include <QDebug>
#include <cstdlib>
#include <QCoreApplication>

#include "todoList.h"
#include "weather.h"

int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);

    QString apiKey = qEnvironmentVariable("OPENWEATHER_API_KEY");
    QString city = "Pretoria";
    QString url = QString("https://api.openweathermap.org/data/2.5/forecast?q=%1&units=metric&appid=%2").arg(city, apiKey);
    
    WeatherData weather = getWeatherForecast(url);

    if (weather.success) {
        qDebug() << "Location:" << weather.locationName;
        qDebug() << "Sunrise:" << weather.sunrise.toString() << "Sunset:" << weather.sunset.toString();
        for (const auto &entry : weather.forecast) {
            qDebug() << entry.time.toString() << entry.temp << "°C, feels" << entry.feelsLike
                    << entry.description << "rain%:" << entry.pop * 100 << "rainmm:" << entry.rainVolume;
        }
    } else {
        qDebug() << "Error:" << weather.errorMessage;
    }
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


