#include "weather.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>

WeatherData getWeatherForecast(const QString &url)
{
    WeatherData data;

    QNetworkAccessManager manager;
    QNetworkRequest request{QUrl(url)};
    QNetworkReply *reply = manager.get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        data.success = false;
        data.errorMessage = reply->errorString();
        reply->deleteLater();
        return data;
    }

    QByteArray raw = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(raw);
    QJsonObject obj = doc.object();

    QJsonObject cityObj = obj["city"].toObject();
    data.locationName = cityObj["name"].toString();
    data.sunrise = QDateTime::fromSecsSinceEpoch(cityObj["sunrise"].toInt());
    data.sunset  = QDateTime::fromSecsSinceEpoch(cityObj["sunset"].toInt());

    QJsonArray list = obj["list"].toArray();
    for (const QJsonValue &v : list) {
        QJsonObject block = v.toObject();
        QJsonObject main = block["main"].toObject();
        QJsonObject wind = block["wind"].toObject();
        QJsonArray weatherArr = block["weather"].toArray();
        QJsonObject weatherObj = weatherArr.isEmpty() ? QJsonObject() : weatherArr[0].toObject();

        ForecastEntry entry;
        entry.time        = QDateTime::fromSecsSinceEpoch(block["dt"].toInt());
        entry.temp        = main["temp"].toDouble();
        entry.feelsLike   = main["feels_like"].toDouble();
        entry.humidity    = main["humidity"].toInt();
        entry.windSpeed   = wind["speed"].toDouble();
        entry.pop         = block["pop"].toDouble(); // 0.0–1.0
        entry.condition   = weatherObj["main"].toString();
        entry.description = weatherObj["description"].toString();

        // rain volume is nested and only present if it rained in that block
        if (block.contains("rain")) {
            entry.rainVolume = block["rain"].toObject()["3h"].toDouble();
        } else {
            entry.rainVolume = 0.0;
        }

        data.forecast.append(entry);
    }

    data.success = true;
    return data;
}