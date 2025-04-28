#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "weatherutil.h"

WeatherUtil *util = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    util = new WeatherUtil(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionLoad_triggered()
{
    qDebug() << "load";
}

void MainWindow::on_actionClear_triggered()
{
    qDebug() << "clear";
}

