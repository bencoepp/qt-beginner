#include "querymodel.h"

QueryModel::QueryModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void QueryModel::setData(const QVector<QMap<QString, QVariant>> &data)
{
    beginResetModel();
    m_data = data;

    // Extract column names from the first row
    m_columnNames.clear();
    if (!m_data.isEmpty()) {
        m_columnNames = m_data.first().keys();
    }

    endResetModel();
}

int QueryModel::rowCount(const QModelIndex & /* parent */) const
{
    return m_data.size();
}

int QueryModel::columnCount(const QModelIndex & /* parent */) const
{
    return m_columnNames.size();
}

QVariant QueryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    const QMap<QString, QVariant> &row = m_data.at(index.row());
    const QString &column = m_columnNames.at(index.column());
    return row.value(column);
}

QVariant QueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal && section < m_columnNames.size()) {
        return m_columnNames.at(section);
    } else if (orientation == Qt::Vertical) {
        return section + 1;  // Optional: show row numbers
    }

    return QVariant();
}
