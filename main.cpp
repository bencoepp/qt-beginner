#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

bool initializeDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("weather.db");

    if (!db.open()) {
        qDebug() << "Error: Unable to open database!" << db.lastError().text();
        return false;
    } else {
        qDebug() << "Database connected successfully.";
    }

    QSqlQuery query;
    QString createTable = R"(
        CREATE TABLE IF NOT EXISTS weather (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            date TEXT NOT NULL,
            averageTemperature REAL,
            minimumTemperature REAL,
            maximunTemperature REAL,
            precipitation REAL,
            snow INTEGER,
            windDirection INTEGER,
            windSpeed REAL,
            windPeakGust REAL,
            airPressure REAL,
            sunshineDuration INTEGER
        )
    )";

    if (!query.exec(createTable)) {
        qDebug() << "Error creating table:" << query.lastError().text();
    } else {
        qDebug() << "Table 'weather' created successfully.";
    }

    query.clear();
    db.close();

    return true;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!initializeDatabase()) {
        return -1;
    }

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "qt-beginner_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
