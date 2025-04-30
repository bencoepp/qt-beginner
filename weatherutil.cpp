#include "weatherutil.h"
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QSqlRecord>

WeatherUtil::WeatherUtil(QObject *parent)
    : QObject{parent}
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("weather.db");

    if (!db.open()) {
        qDebug() << "Error: Unable to connect to database!" << db.lastError().text();
    }
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

    for (const QString &fileName : csvFiles) {
        QFile file(dir.filePath(fileName));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Cannot open file:" << fileName;
            continue;
        }

        QTextStream in(&file);

        bool firstLine = true;
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (firstLine) {
                firstLine = false;
                continue;
            }

            try{
                Weather element;
                element.parse(line);
                if(!checkWeatherExists(element)){
                    insert(element);
                }
            }catch(std::exception){
                qWarning() << "An error happend while parsing a line";
            }
        }
        file.close();
    }

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
