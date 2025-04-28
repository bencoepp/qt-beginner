#ifndef WEATHER_H
#define WEATHER_H

#include <QDateTime>

class Weather
{
public:
    explicit Weather();
    void parse(const QString &line);
    QDateTime getDate() const;
    float getAverageTemperature() const;
    float getMinimumTemperature() const;
    float getMaximunTemperature() const;
    float getPrecipitation() const;
    int getSnow() const;
    short getWindDirection() const;
    float getWindSpeed() const;
    float getWindPeakGust() const;
    float getAirPressure() const;
    int getSunshineDuration() const;
private:
    QDateTime date;
    float averageTemperature;
    float minimumTemperature;
    float maximunTemperature;
    float precipitation;
    int snow;
    short windDirection;
    float windSpeed;
    float windPeakGust;
    float airPressure;
    int sunshineDuration;

signals:
};

#endif // WEATHER_H
