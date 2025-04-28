#include "weatherutil.h"
#include <QDir>

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

bool WeatherUtil::checkWeatherExists(const Weather &weather)
{
    for(const Weather &element : std::as_const(m_entries)){
        if (element.getDate() == weather.getDate()) {
            return true;
        }
    }
    return false;
}
