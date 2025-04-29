#ifndef WEATHERPROXYMODEL_H
#define WEATHERPROXYMODEL_H

#include <QSortFilterProxyModel>

class WeatherProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit WeatherProxyModel(QObject *parent = nullptr);

    void setFilterString(const QString &text);
    void setFilterColumn(int column);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    QString filterText;
    int filterColumnIndex;
};

#endif // WEATHERPROXYMODEL_H
