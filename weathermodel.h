#ifndef WEATHERMODEL_H
#define WEATHERMODEL_H
#include <QAbstractTableModel>
#include "weather.h"

class WeatherModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit WeatherModel(QObject *parent = nullptr);

    void setWeatherList(const QList<Weather> &list);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QList<Weather> weatherList;
};

#endif // WEATHERMODEL_H
