#ifndef QUERYMODEL_H
#define QUERYMODEL_H

#include <QAbstractTableModel>
#include <QMap>
#include <QVariant>
#include <QString>
#include <QVector>

class QueryModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit QueryModel(QObject *parent = nullptr);

    void setData(const QVector<QMap<QString, QVariant>> &data);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QVector<QMap<QString, QVariant>> m_data;
    QStringList m_columnNames;
};

#endif // QUERYMODEL_H
