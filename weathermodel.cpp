#include "weathermodel.h"

WeatherModel::WeatherModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void WeatherModel::setWeatherList(const QList<Weather> &list)
{
    beginResetModel();
    weatherList = list;
    endResetModel();
}

int WeatherModel::rowCount(const QModelIndex & /*parent*/) const
{
    return weatherList.size();
}

int WeatherModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 11; // Number of fields
}

QVariant WeatherModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const Weather &weather = weatherList.at(index.row());

    switch (index.column()) {
    case 0: return weather.getDate().toString("yyyy-MM-dd HH:mm");
    case 1: return weather.getAverageTemperature();
    case 2: return weather.getMinimumTemperature();
    case 3: return weather.getMaximunTemperature();
    case 4: return weather.getPrecipitation();
    case 5: return weather.getSnow();
    case 6: return weather.getWindDirection();
    case 7: return weather.getWindSpeed();
    case 8: return weather.getWindPeakGust();
    case 9: return weather.getAirPressure();
    case 10: return weather.getSunshineDuration();
    default: return QVariant();
    }
}

QVariant WeatherModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return "Date";
        case 1: return "Avg Temp (°C)";
        case 2: return "Min Temp (°C)";
        case 3: return "Max Temp (°C)";
        case 4: return "Precipitation (mm)";
        case 5: return "Snow (cm)";
        case 6: return "Wind Direction (°)";
        case 7: return "Wind Speed (m/s)";
        case 8: return "Peak Gust (m/s)";
        case 9: return "Air Pressure (hPa)";
        case 10: return "Sunshine Duration (min)";
        default: return QVariant();
        }
    }

    return QVariant();
}
