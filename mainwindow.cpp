#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "weatherutil.h"
#include "weathermodel.h"
#include "weatherproxymodel.h"
#include <QFileDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

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
    util->loadFromDirectory(dialog.directory().absolutePath());
    updateWeatherData();
}

void MainWindow::on_actionClear_triggered()
{
    qDebug() << "clear";
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

