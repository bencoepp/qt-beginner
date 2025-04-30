#ifndef WEATHERUTIL_H
#define WEATHERUTIL_H

#include "weather.h"
#include <QObject>
#include <qmutex.h>
#include <qsqldatabase.h>
#include <QtCharts/QChartView>

class WeatherUtil : public QObject
{
    Q_OBJECT
public:
    explicit WeatherUtil(QObject *parent = nullptr);
    bool loadFromDirectory(const QString &directoryPath);
    QVector<Weather> select(const QString &selectQuery);
    QVector<QMap<QString, QVariant>> selectAsMap(const QString &selectQuery);
    double highestTemp();
    double avgTemp();
    double lowestTemp();
    QChartView* createTemperatureChart();
public slots:
    void loadFromDirectoryAsync(const QString &directoryPath);
private:
    QSqlDatabase db;
    bool insert(const Weather &weather);
    bool checkWeatherExists(const Weather &weather);
signals:
    void loadingFinished();
};

#endif // WEATHERUTIL_H
