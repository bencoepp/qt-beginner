
#include "weatherproxymodel.h"
#include "weathermodel.h"

WeatherProxyModel::WeatherProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent),
    filterColumnIndex(-1)
{
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void WeatherProxyModel::setFilterString(const QString &text)
{
    filterText = text;
    invalidateFilter();
}

void WeatherProxyModel::setFilterColumn(int column)
{
    filterColumnIndex = column;
    invalidateFilter();
}

bool WeatherProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (filterText.isEmpty())
        return true;

    if (filterColumnIndex < 0)
        return true;

    QModelIndex index = sourceModel()->index(sourceRow, filterColumnIndex, sourceParent);
    QString data = sourceModel()->data(index).toString();

    return data.contains(filterText, Qt::CaseInsensitive);
}

bool WeatherProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    // Try to compare as numbers first
    bool ok1, ok2;
    double d1 = leftData.toDouble(&ok1);
    double d2 = rightData.toDouble(&ok2);

    if (ok1 && ok2) {
        return d1 < d2;
    }

    // If not numbers, compare as strings
    return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
}
