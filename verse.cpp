#include "verse.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDate>

void getDailyVerse()
{
    int dayOfYear = QDate::currentDate().dayOfYear();
    QString votdUrl = QString("https://api.youversion.com/v1/verse_of_the_days/%1").arg(dayOfYear);

    QNetworkRequest votdRequest{QUrl(votdUrl)};
    votdRequest.setRawHeader("x-yvp-app-key", qEnvironmentVariable("YOUVERSION_API_KEY").toUtf8());

    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QNetworkReply* votdReply = manager->get(votdRequest);

    QObject::connect(votdReply, &QNetworkReply::finished, [votdReply, manager]() {
        if (votdReply->error() != QNetworkReply::NoError) {
            qDebug() << "Error fetching verse of the day:" << votdReply->errorString();
            votdReply->deleteLater();
            return;
        }

        QJsonDocument votdDoc = QJsonDocument::fromJson(votdReply->readAll());
        QString passageId = votdDoc.object().value("passage_id").toString();
        votdReply->deleteLater();

        if (passageId.isEmpty()) {
            qDebug() << "No passage_id in response.";
            return;
        }


        QString passageUrl = QString("https://api.youversion.com/v1/bibles/3034/passages/%1").arg(passageId);

        QNetworkRequest passageRequest{QUrl(passageUrl)};
        passageRequest.setRawHeader("x-yvp-app-key", qEnvironmentVariable("YOUVERSION_API_KEY").toUtf8());

        QNetworkReply* passageReply = manager->get(passageRequest);

        QObject::connect(passageReply, &QNetworkReply::finished, [passageReply, manager, passageId]() {
            if (passageReply->error() != QNetworkReply::NoError) {
                qDebug() << "Error fetching passage text:" << passageReply->errorString();
            } else {
                QJsonDocument passageDoc = QJsonDocument::fromJson(passageReply->readAll());
                QJsonObject obj = passageDoc.object();

                QString content = obj.value("content").toString();
                QString reference = obj.value("reference").toString();

                qDebug().noquote() << reference << "-" << content;
            }

            passageReply->deleteLater();
            manager->deleteLater();
        });
    });
}