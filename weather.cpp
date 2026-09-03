#include "weather.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>

WeatherData getData(const QString &url)
{
    WeatherData data;

    QNetworkAccessManager manager;
    QNetworkRequest request{QUrl(url)};
    QNetworkReply *reply = manager.get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError)
    {
        data.success = false;
        data.errorMessage = reply->errorString();
        reply->deleteLater();
        return data;
    }

    QByteArray raw = reply->readAll();
    data = parseWeatherData(raw);
    reply->deleteLater();
    return data;
}

WeatherData parseWeatherData(const QByteArray &raw)
{
    WeatherData data;
    QJsonDocument doc = QJsonDocument::fromJson(raw);
    QJsonObject obj = doc.object();

    QJsonObject cityObj = obj["city"].toObject();
    data.locationName = cityObj["name"].toString();
    data.sunrise = QDateTime::fromSecsSinceEpoch(cityObj["sunrise"].toInt());
    data.sunset = QDateTime::fromSecsSinceEpoch(cityObj["sunset"].toInt());

    QJsonArray list = obj["list"].toArray();
    for (const QJsonValue &v : list)
    {
        QJsonObject block = v.toObject();
        QJsonObject main = block["main"].toObject();
        QJsonObject wind = block["wind"].toObject();
        QJsonArray weatherArr = block["weather"].toArray();
        QJsonObject weatherObj = weatherArr.isEmpty() ? QJsonObject() : weatherArr[0].toObject();

        ForecastEntry entry;
        entry.time = QDateTime::fromSecsSinceEpoch(block["dt"].toInt());
        entry.temp = main["temp"].toDouble();
        entry.feelsLike = main["feels_like"].toDouble();
        entry.humidity = main["humidity"].toInt();
        entry.windSpeed = wind["speed"].toDouble();
        entry.pop = block["pop"].toDouble(); // 0.0–1.0
        entry.condition = weatherObj["main"].toString();
        entry.description = weatherObj["description"].toString();

        // rain volume is nested and only present if it rained in that block
        if (block.contains("rain"))
        {
            entry.rainVolume = block["rain"].toObject()["3h"].toDouble();
        }
        else
        {
            entry.rainVolume = 0.0;
        }

        data.forecast.append(entry);
    }

    data.success = true;
    return data;
}

void getWeatherSummary(QString Loc)
{
    QString apiKey = qEnvironmentVariable("OPENWEATHER_API_KEY");
    if (apiKey.trimmed().isEmpty())
    {
        qDebug() << "Weather unavailable: OPENWEATHER_API_KEY is not set.";
        return;
    }
    QString url = QString("https://api.openweathermap.org/data/2.5/forecast?q=%1&units=metric&appid=%2").arg(Loc, apiKey);

    WeatherData weather = getData(url);

    QTime currentTime = QTime::currentTime();
    // Format as a string ("14:32:05")
    QString timeString = currentTime.toString("hh:mm:ss");

    int currentHour = timeString.left(2).toInt();
    QString currentDay = QDate::currentDate().toString("dd/MM/yyyy");

    QString out;

    int todayCount = 0;

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
                if (std::abs(currentHour - fcHour) <= 3)
                {
                    out += QString::number(forecast[i].temp) + "°C" + "\n";
                    out += forecast[i].description + "\n";
                    out += "Currently feels like: " + QString::number(forecast[i].feelsLike) + "°C" + "\n";
                    out += QString::number(forecast[i].windSpeed) + "km/h windspeed \n";
                }

                totalTemp += forecast[i].temp;
                totalRain += forecast[i].pop;
                totalFall += forecast[i].rainVolume;
                todayCount++;
            }
        }

        double avgRain = (todayCount > 0) ? (totalRain / todayCount) : 0.0;
        double avgTemp = (todayCount > 0) ? (totalTemp / todayCount) : 0.0;
        double avgFall = (todayCount > 0) ? (totalFall / todayCount) : 0.0;

        if (avgRain > 0.5)
        {
            out += QString::number(avgRain * 100, 'f', 0) + "% chance of rain, remember the umbrella! \n";
            out += QString::number(avgFall, 'f', 1) + "mm of rain for today \n";
        }

        if (avgTemp > 25)
        {
            out += "High tempratures today, averaging at " + QString::number(avgTemp) + "°C, remember your hat and sun screen! \n";
        }

        out += "Sunset: " + weather.sunset.toString("hh:mm:ss") + "\n";

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
    if (apiKey.trimmed().isEmpty())
    {
        qDebug() << "Forecast unavailable: OPENWEATHER_API_KEY is not set.";
        return;
    }
    QString url = QString("https://api.openweathermap.org/data/2.5/forecast?q=%1&units=metric&appid=%2").arg(Loc, apiKey);

    WeatherData weather = getData(url);

    if (weather.success)
    {
        QString out;
        QVector<ForecastEntry> forecast = weather.forecast;
        QLocale locale;

        out += "\n";
        out += "3 hourly temprature: \n";
        int tc = 0;

        for (int i = 0; i < forecast.size(); i++)
        {
            out += forecast[i].time.toString("hh:mm") + " " + QString::number(forecast[i].temp) + "°C" + " ";
            tc++;
            if (tc == 8)
            {
                break;
            }
        }
        out += "\n";
        out += "3 hourly Precipation: \n";
        int rc = 0;

        for (int i = 0; i < forecast.size(); i++)
        {
            out += forecast[i].time.toString("hh:mm") + " " + QString::number(forecast[i].pop * 100, 'f', 0) + "%" + " ";
            rc++;
            if (rc == 8)
            {
                break;
            }
        }
        out += "\n";
        out += "3 hourly wind speeds: \n";
        int wc = 0;

        for (int i = 0; i < forecast.size(); i++)
        {
            out += forecast[i].time.toString("hh:mm") + " " + QString::number(forecast[i].windSpeed) + "km/h" + " ";
            wc++;
            if (wc == 8)
            {
                break;
            }
        }
        out += "\n";

        QVector<QString> usedTemp;
        out += "daily temprature: \n";
        for (int i = 0; i < forecast.size(); i++)
        {
            int dayNumber = forecast[i].time.date().dayOfWeek();
            QString fcDay = locale.dayName(dayNumber, QLocale::ShortFormat);

            if (!usedTemp.contains(fcDay))
            {
                double totalTemp = 0.0;
                int count = 0;

                for (int j = 0; j < forecast.size(); j++)
                {
                    int otherDayNumber = forecast[j].time.date().dayOfWeek();
                    QString otherDay = locale.dayName(otherDayNumber, QLocale::ShortFormat);
                    if (otherDay == fcDay)
                    {
                        totalTemp += forecast[j].temp;
                        count++;
                    }
                }
                double avgTemp = (count > 0) ? (totalTemp / count) : 0.0;
                out += fcDay + " " + QString::number(avgTemp, 'f', 0) + "°C" + " ";
                usedTemp.push_back(fcDay);
            }
        }
        out += "\n";

        QVector<QString> usedRain;
        out += "daily Precipation: \n";

        for (int i = 0; i < forecast.size(); i++)
        {
            int dayNumber = forecast[i].time.date().dayOfWeek();
            QString fcDay = locale.dayName(dayNumber, QLocale::ShortFormat);

            if (!usedRain.contains(fcDay))
            {
                double totalRain = 0.0;
                int count = 0;

                for (int j = 0; j < forecast.size(); j++)
                {
                    int otherDayNumber = forecast[j].time.date().dayOfWeek();
                    QString otherDay = locale.dayName(otherDayNumber, QLocale::ShortFormat);
                    if (otherDay == fcDay)
                    {
                        totalRain += forecast[j].pop;
                        count++;
                    }
                }
                double avgRain = (count > 0) ? (totalRain / count * 100) : 0.0;
                out += fcDay + " " + QString::number(avgRain) + "%" + " ";
                usedRain.push_back(fcDay);
            }
        }
        out += "\n";

        qDebug().noquote() << out;
    }
    else
    {
        qDebug() << "Error:" << weather.errorMessage;
    }
}