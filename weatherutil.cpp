#include "weatherutil.h"
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QFuture>
#include <QMutex>
#include <QSqlRecord>
#include <QtConcurrent/QtConcurrent>

WeatherUtil::WeatherUtil(QObject *parent)
    : QObject{parent}
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("weather.db");

    if (!db.open()) {
        qDebug() << "Error: Unable to connect to database!" << db.lastError().text();
    }
}

void processCsvFile(const QString &filePath, const QString &dbPath, QMutex *mutex)
{
    QString connectionName = QUuid::createUuid().toString();
    QSqlDatabase threadDb = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    threadDb.setDatabaseName(dbPath);

    if (!threadDb.open()) {
        qWarning() << "Thread DB open failed:" << threadDb.lastError().text();
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << filePath;
        return;
    }

    QTextStream in(&file);
    bool firstLine = true;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (firstLine) {
            firstLine = false;
            continue;
        }

        try {
            Weather element;
            element.parse(line);

            QSqlQuery query(threadDb);

            // Lock while checking and inserting
            QMutexLocker locker(mutex);
            query.prepare("SELECT COUNT(*) FROM weather WHERE date = :date");
            query.bindValue(":date", element.getDate().toString(Qt::ISODate));
            if (query.exec() && query.next() && query.value(0).toInt() == 0) {
                query.prepare(R"(
                    INSERT INTO weather (
                        date, averageTemperature, minimumTemperature, maximunTemperature,
                        precipitation, snow, windDirection, windSpeed, windPeakGust,
                        airPressure, sunshineDuration
                    ) VALUES (
                        :date, :averageTemperature, :minimumTemperature, :maximunTemperature,
                        :precipitation, :snow, :windDirection, :windSpeed, :windPeakGust,
                        :airPressure, :sunshineDuration
                    )
                )");

                query.bindValue(":date", element.getDate().toString(Qt::ISODate));
                query.bindValue(":averageTemperature", element.getAverageTemperature());
                query.bindValue(":minimumTemperature", element.getMinimumTemperature());
                query.bindValue(":maximunTemperature", element.getMaximunTemperature());
                query.bindValue(":precipitation", element.getPrecipitation());
                query.bindValue(":snow", element.getSnow());
                query.bindValue(":windDirection", element.getWindDirection());
                query.bindValue(":windSpeed", element.getWindSpeed());
                query.bindValue(":windPeakGust", element.getWindPeakGust());
                query.bindValue(":airPressure", element.getAirPressure());
                query.bindValue(":sunshineDuration", element.getSunshineDuration());

                if (!query.exec()) {
                    qWarning() << "Insert failed:" << query.lastError().text();
                }
            }
        } catch (...) {
            qWarning() << "Error parsing line in file:" << filePath;
        }
    }

    file.close();
    threadDb.close();
    QSqlDatabase::removeDatabase(connectionName);
}

bool WeatherUtil::loadFromDirectory(const QString &directoryPath)
{
    QDir dir(directoryPath);
    if (!dir.exists()) {
        qWarning() << "Directory does not exist:" << directoryPath;
        return false;
    }

    const QStringList csvFiles = dir.entryList(QStringList() << "*.csv", QDir::Files);
    if (csvFiles.isEmpty()) {
        qWarning() << "No CSV files found in:" << directoryPath;
        return false;
    }

    QMutex *mutex = new QMutex();
    QList<QFuture<void>> futures;

    for (const QString &fileName : csvFiles) {
        QString fullPath = dir.filePath(fileName);
        QFuture<void> future = QtConcurrent::run(processCsvFile, fullPath, db.databaseName(), mutex);
        futures.append(future);
    }

    for (auto &future : futures) {
        future.waitForFinished();
    }

    delete mutex;
    return true;
}

QVector<Weather> WeatherUtil::select(const QString &selectQuery)
{
    QVector<Weather> weatherList;

    QSqlQuery query;
    if (!query.exec(selectQuery)) {
        qDebug() << "Error executing select query:" << query.lastError().text();
        return weatherList;
    }

    while (query.next()) {
        Weather w;
        w.parse(query);
        weatherList.push_back(w);
    }

    return weatherList;
}

QVector<QMap<QString, QVariant>> WeatherUtil::selectAsMap(const QString &selectQuery)
{
    QVector<QMap<QString, QVariant>> resultList;

    QSqlQuery query;
    if (!query.exec(selectQuery)) {
        qDebug() << "Error executing select query:" << query.lastError().text();
        return resultList;
    }

    while (query.next()) {
        QMap<QString, QVariant> rowMap;
        for (int i = 0; i < query.record().count(); ++i) {
            QString columnName = query.record().fieldName(i);
            QVariant value = query.value(i);
            rowMap.insert(columnName, value);
        }
        resultList.append(rowMap);
    }

    return resultList;
}

double WeatherUtil::highestTemp()
{
    QVector<Weather> m_entries = select("SELECT * FROM weather");
    if (m_entries.isEmpty())
        return 0.0f;

    float highest = m_entries.first().getMaximunTemperature();

    for (const Weather &weather : std::as_const(m_entries)) {
        if (weather.getMaximunTemperature() > highest) {
            highest = weather.getMaximunTemperature();
        }
    }
    return highest;
}

double WeatherUtil::avgTemp()
{
    QVector<Weather> m_entries = select("SELECT * FROM weather");

    if (m_entries.isEmpty())
        return 0.0f;

    float sum = 0.0f;
    for (const Weather &weather : std::as_const(m_entries)) {
        sum += weather.getAverageTemperature();
    }
    return sum / m_entries.size();
}

double WeatherUtil::lowestTemp()
{
    QVector<Weather> m_entries = select("SELECT * FROM weather");

    if (m_entries.isEmpty())
        return 0.0f;

    float lowest = m_entries.first().getMinimumTemperature();

    for (const Weather &weather : std::as_const(m_entries)) {
        if (weather.getMinimumTemperature() < lowest) {
            lowest = weather.getMinimumTemperature();
        }
    }
    return lowest;
}

QChartView *WeatherUtil::createTemperatureChart()
{
    QVector<Weather> m_entries = select("SELECT * FROM weather");

    if (m_entries.isEmpty())
        return nullptr;

    QLineSeries *avgTempSeries = new QLineSeries();
    avgTempSeries->setName("Average Temp");

    QLineSeries *minTempSeries = new QLineSeries();
    minTempSeries->setName("Minimum Temp");

    QLineSeries *maxTempSeries = new QLineSeries();
    maxTempSeries->setName("Maximum Temp");

    for (const Weather &weather : std::as_const(m_entries)) {
        qint64 timestamp = weather.getDate().toMSecsSinceEpoch();
        avgTempSeries->append(timestamp, weather.getAverageTemperature());
        minTempSeries->append(timestamp, weather.getMinimumTemperature());
        maxTempSeries->append(timestamp, weather.getMaximunTemperature());
    }

    QChart *chart = new QChart();
    chart->addSeries(avgTempSeries);
    chart->addSeries(minTempSeries);
    chart->addSeries(maxTempSeries);
    chart->setTitle("Temperature Over Time");
    chart->legend()->setAlignment(Qt::AlignBottom);

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setFormat("yyyy-MM-dd");
    axisX->setTitleText("Date");
    chart->addAxis(axisX, Qt::AlignBottom);

    avgTempSeries->attachAxis(axisX);
    minTempSeries->attachAxis(axisX);
    maxTempSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Temperature (°C)");
    axisY->setRange(lowestTemp(), highestTemp());
    chart->addAxis(axisY, Qt::AlignLeft);

    avgTempSeries->attachAxis(axisY);
    minTempSeries->attachAxis(axisY);
    maxTempSeries->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    return chartView;
}

void WeatherUtil::loadFromDirectoryAsync(const QString &directoryPath)
{
    QtConcurrent::run([=]() {
        QDir dir(directoryPath);
        if (!dir.exists()) {
            qWarning() << "Directory does not exist:" << directoryPath;
            emit loadingFinished();  // still emit to unblock any UI loading indicator
            return;
        }

        const QStringList csvFiles = dir.entryList(QStringList() << "*.csv", QDir::Files);
        if (csvFiles.isEmpty()) {
            qWarning() << "No CSV files found in:" << directoryPath;
            emit loadingFinished();
            return;
        }

        QMutex mutex;
        QList<QFuture<void>> futures;

        for (const QString &fileName : csvFiles) {
            QString fullPath = dir.filePath(fileName);
            QFuture<void> future = QtConcurrent::run(processCsvFile, fullPath, db.databaseName(), &mutex);
            futures.append(future);
        }

        for (auto &future : futures) {
            future.waitForFinished();  // This is okay here since we're already on a background thread
        }

        emit loadingFinished();
    });
}

bool WeatherUtil::insert(const Weather &weather)
{
    QSqlQuery query;

    query.prepare(R"(
        INSERT INTO weather (
            date,
            averageTemperature,
            minimumTemperature,
            maximunTemperature,
            precipitation,
            snow,
            windDirection,
            windSpeed,
            windPeakGust,
            airPressure,
            sunshineDuration
        ) VALUES (
            :date,
            :averageTemperature,
            :minimumTemperature,
            :maximunTemperature,
            :precipitation,
            :snow,
            :windDirection,
            :windSpeed,
            :windPeakGust,
            :airPressure,
            :sunshineDuration
        )
    )");

    // Bind values
    query.bindValue(":date", weather.getDate().toString(Qt::ISODate)); // Save date as ISO format string
    query.bindValue(":averageTemperature", weather.getAverageTemperature());
    query.bindValue(":minimumTemperature", weather.getMinimumTemperature());
    query.bindValue(":maximunTemperature", weather.getMaximunTemperature());
    query.bindValue(":precipitation", weather.getPrecipitation());
    query.bindValue(":snow", weather.getSnow());
    query.bindValue(":windDirection", weather.getWindDirection());
    query.bindValue(":windSpeed", weather.getWindSpeed());
    query.bindValue(":windPeakGust", weather.getWindPeakGust());
    query.bindValue(":airPressure", weather.getAirPressure());
    query.bindValue(":sunshineDuration", weather.getSunshineDuration());

    // Execute and check success
    if (!query.exec()) {
        qDebug() << "Error inserting weather data:" << query.lastError().text();
        return false;
    }

    qDebug() << "Inserted Element";

    return true;
}

bool WeatherUtil::checkWeatherExists(const Weather &weather)
{    
    for(const Weather &element : select("SELECT * FROM weather")){
        if (element.getDate() == weather.getDate()) {
            return true;
        }
    }
    return false;
}
