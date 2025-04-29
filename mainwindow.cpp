#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "weatherutil.h"
#include "weathermodel.h"
#include <QFileDialog>

WeatherUtil *util = nullptr;
WeatherModel *model = nullptr;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    util = new WeatherUtil(this);
    model = new WeatherModel(this);
    updateWeatherData();
    ui->tableView->setModel(model);
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
    qDebug() << util->entries().size();
    model->setWeatherList(util->entries());
}

