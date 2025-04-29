#include "weatherutil.h"
#include <QDir>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>

WeatherUtil::WeatherUtil(QObject *parent)
    : QObject{parent}
{}

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

    m_entries.clear();

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
                    m_entries.push_back(std::move(element));
                }
            }catch(std::exception){
                qWarning() << "An error happend while parsing a line";
            }
        }
        file.close();
    }

    return true;
}

const QVector<Weather> &WeatherUtil::entries() const
{
    return m_entries;
}

double WeatherUtil::highestTemp()
{
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

bool WeatherUtil::checkWeatherExists(const Weather &weather)
{
    for(const Weather &element : std::as_const(m_entries)){
        if (element.getDate() == weather.getDate()) {
            return true;
        }
    }
    return false;
}
