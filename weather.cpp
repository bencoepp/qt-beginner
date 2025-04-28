#include "weather.h"

Weather::Weather()
{}

void Weather::parse(const QString &line)
{
    QStringList fields = line.split(',');
    if(fields.size() != 11)
        throw std::runtime_error("To many fields in line");

    QString format = "yyyy-MM-dd HH:mm:ss";
    date = QDateTime::fromString(fields[0], format);
    averageTemperature =  fields[1].toFloat();
    minimumTemperature = fields[2].toFloat();
    maximunTemperature = fields[3].toFloat();
    precipitation = fields[4].toFloat();
    snow = fields[5].toInt();
    windDirection = fields[6].toShort();
    windSpeed = fields[7].toFloat();
    windPeakGust = fields[8].toFloat();
    airPressure = fields[9].toFloat();
    sunshineDuration = fields[10].toInt();
}

QDateTime Weather::getDate() const
{
    return date;
}

float Weather::getAverageTemperature() const
{
    return averageTemperature;
}

float Weather::getMinimumTemperature() const
{
    return minimumTemperature;
}

float Weather::getMaximunTemperature() const
{
    return maximunTemperature;
}

float Weather::getPrecipitation() const
{
    return precipitation;
}

int Weather::getSnow() const
{
    return snow;
}

short Weather::getWindDirection() const
{
    return windDirection;
}

float Weather::getWindSpeed() const
{
    return windSpeed;
}

float Weather::getWindPeakGust() const
{
    return windPeakGust;
}

float Weather::getAirPressure() const
{
    return airPressure;
}

int Weather::getSunshineDuration() const
{
    return sunshineDuration;
}
