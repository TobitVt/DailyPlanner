#pragma once
#include <QString>
#include <QVector>
#include <QDateTime>
#include <QByteArray>

struct ForecastEntry
{
    QDateTime time;
    double temp;
    double feelsLike;
    QString condition;   // e.g. "Rain"
    QString description; // e.g. "light rain"
    int humidity;        // %
    double windSpeed;    // m/s (metric)
    double pop;          // 0.0–1.0, rain probability
    double rainVolume;   // mm, 0 if none
};

struct WeatherData
{
    QString locationName;
    QDateTime sunrise;
    QDateTime sunset;
    QVector<ForecastEntry> forecast; // 3-hour steps, ~5 days
    bool success = false;
    QString errorMessage;
};

WeatherData getData(const QString &url);
WeatherData parseWeatherData(const QByteArray &raw);
void getWeatherSummary(QString Loc);
void getForecast(QString Loc);