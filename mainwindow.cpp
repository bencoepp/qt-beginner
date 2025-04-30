#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "weatherutil.h"
#include "weathermodel.h"
#include "weatherproxymodel.h"
#include "querymodel.h"
#include <QFileDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QMessageBox>
#include <QDir>

WeatherUtil *util = nullptr;
WeatherModel *model = nullptr;
WeatherProxyModel *proxyModel = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    util = new WeatherUtil(this);
    model = new WeatherModel(this);
    proxyModel = new WeatherProxyModel(this);

    updateWeatherData();
    proxyModel->setSourceModel(model);
    ui->tableView->setModel(proxyModel);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionLoad_triggered()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.exec();
    ui->statusbar->showMessage("Loading...");
    connect(util, &WeatherUtil::loadingFinished, this, [=]() {
        updateWeatherData();
        ui->statusbar->showMessage("Finished", 50);
    });
    util->loadFromDirectoryAsync(dialog.directory().absolutePath());
}

void MainWindow::on_actionClear_triggered()
{
    QString dbPath = QDir::currentPath() + "/weather.db";

    if (!QFile::exists(dbPath)) {
        QMessageBox::information(this, "Clear Database", "No database file found to delete.");
        return;
    }

    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Database",
                                  "Are you sure you want to delete the weather database?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (QFile::remove(dbPath)) {
            QMessageBox::information(this, "Clear Database", "Database deleted successfully.");
        } else {
            QMessageBox::critical(this, "Clear Database", "Failed to delete the database file.");
        }
    }
    updateWeatherData();
}

void MainWindow::updateWeatherData()
{
    qDebug() << util->select("SELECT * FROM weather").size();
    model->setWeatherList(util->select("SELECT * FROM weather"));
    ui->lcd_totalElements->display(static_cast<int>(util->select("SELECT * FROM weather").size()));
    ui->lcd_highestTemp->display(util->highestTemp());
    ui->lcd_avgTemp->display(util->avgTemp());

    QChartView *chartView = util->createTemperatureChart();
    if(chartView)
        ui->chart->layout()->addWidget(chartView);
}


void MainWindow::on_pushButton_clicked()
{
    if(ui->lineEdit->text() == "" || ui->lineEdit->text() == nullptr){
        qWarning() << "No query provided";
        return;
    }

    QString text = ui->lineEdit->text();

    if(text.contains("INSERT") || text.contains("UPDATE") || text.contains("DELETE")){
        qWarning() << "Dangerous query provided";
        return;
    }

    QueryModel *modelQuery = new QueryModel(this);
    WeatherProxyModel *proxyModelQuery = new WeatherProxyModel(this);

    modelQuery->setData(util->selectAsMap(text));
    proxyModelQuery->setSourceModel(modelQuery);
    ui->queryTable->setModel(proxyModelQuery);
    ui->queryTable->setSortingEnabled(true);
    ui->queryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

