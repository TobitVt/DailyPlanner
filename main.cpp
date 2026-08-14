
#include <iostream>
#include <vector>
#include <QDebug>
#include <cstdlib>
#include <QCoreApplication>
#include <QTime>
#include <QDate>
#include <cmath>

#include "todoList.h"
#include "weather.h"

// finish weather and functions
//set up calender class
// set up daily verse
// set up daily hourly schedule
// set up reminders
//set up database

void getWeatherSummary(QString Loc)
{
    QString apiKey = qEnvironmentVariable("OPENWEATHER_API_KEY");
    QString city = Loc;
    QString url = QString("https://api.openweathermap.org/data/2.5/forecast?q=%1&units=metric&appid=%2").arg(city, apiKey);
    
    WeatherData weather = getWeatherForecast(url);

    QTime currentTime = QTime::currentTime();
    //Format as a string ("14:32:05")
    QString timeString = currentTime.toString("hh:mm:ss");

    int currentHour = timeString.left(2).toInt();
    QString currentDay = QDate::currentDate().toString("dd/MM/yyyy");

    QString out;

    if (weather.success) 
    {
        out += "Weather - " + weather.locationName + "\n";

        out += "Sunrise: " + weather.sunrise.toString("hh:mm:ss") + "\n";

        out += "Currently: \n";
        
        QVector<ForecastEntry> forecast = weather.forecast;
        double totalTemp = 0.0;
        double totalRain = 0.0;
        double totalFall = 0.0;

        for (int i = 0; i < forecast.size(); i++)
        {
            QDateTime fcDate = forecast[i].time;
            int fcHour = fcDate.time().hour();
            QString fcDay = fcDate.date().toString("dd/MM/yyyy");

            if (currentDay == fcDay)
            {
                if (std::abs(currentHour - fcHour) < 3)
                {
                    out += QString::number(fcHour) + "\n";
                    out += QString::number(forecast[i].temp) + "°C" + "\n";
                    out += forecast[i].description + "\n";
                    out += "Currently feels like: " + QString::number(forecast[i].feelsLike) + "°C" + "\n";
                    out += QString::number(forecast[i].windSpeed) + "km/h windspeed \n";             
                }

            totalTemp += forecast[i].temp;
            totalRain += forecast[i].pop;
            }
        }

        double avgRain = totalRain / forecast.size();
        double avgTemp = totalTemp / forecast.size();
        double avgFall = totalFall / forecast.size();

        if (avgRain > 50)
        {
            out += QString::number(avgRain) + "% chance of rain, remember the umbrella! \n";
            out += QString::number(avgFall) + "mm of rain for today \n";
        }

        if (avgTemp > 25)
        {
            out += "High tempratures today, averaging at" + QString::number(avgTemp) + "°C, remember your hat and sun screen! \n";
        }

        out += "Sun will set at " + weather.sunset.toString("hh:mm:ss") + "\n";

        qDebug().noquote() << out;
    } 
    else 
    {
        qDebug() << "Error:" << weather.errorMessage;
        
    }
}


void getForecast(QString Loc)
{
    QString apiKey = qEnvironmentVariable("OPENWEATHER_API_KEY");
    QString city = Loc;
    QString url = QString("https://api.openweathermap.org/data/2.5/forecast?q=%1&units=metric&appid=%2").arg(city, apiKey);
    
    WeatherData weather = getWeatherForecast(url);

    QTime currentTime = QTime::currentTime();
    //Format as a string ("14:32:05")
    QString timeString = currentTime.toString("hh:mm:ss");

    int currentHour = timeString.left(2).toInt();
    QString currentDay = QDate::currentDate().toString("dd/MM/yyyy");

    QString out;

    if (weather.success) 
    {
        QString out;
        QVector<ForecastEntry> forecast = weather.forecast;

        for (int i = 0; i < forecast.size(); i++)
        {
            out += "3 hourly temprature: \n";
            

        }

        for (int i = 0; i < forecast.size(); i++)
        {
            out += "3 hourly Precipation: \n";

        }

        for (int i = 0; i < forecast.size(); i++)
        {
            out += "3 hourly wind speeds: \n";

        }

        for (int i = 0; i < forecast.size(); i++)
        {
            out += "daily temprature: \n";
            

        }

        for (int i = 0; i < forecast.size(); i++)
        {
            out += "daily Precipation: \n";
            

        }

        qDebug().noquote() << out;
    } 
    else 
    {
        qDebug() << "Error:" << weather.errorMessage;
        
    }

}
int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);

    getWeatherSummary("Pretoria");
    getForecast("Pretoria");



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


