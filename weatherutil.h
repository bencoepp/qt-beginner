#ifndef WEATHERUTIL_H
#define WEATHERUTIL_H

#include "weather.h"
#include <QObject>
#include <QtCharts/QChartView>

class WeatherUtil : public QObject
{
    Q_OBJECT
public:
    explicit WeatherUtil(QObject *parent = nullptr);
    bool loadFromDirectory(const QString &directoryPath);
    const QVector<Weather>& entries() const;
    double highestTemp();
    double avgTemp();
    double lowestTemp();
    QChartView* createTemperatureChart();
private:
    QVector<Weather> m_entries;
    bool checkWeatherExists(const Weather &weather);
signals:
};

#endif // WEATHERUTIL_H
