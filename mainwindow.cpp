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

void MainWindow::on_pushButton_clicked()
{
    util->loadFromDirectory("C:\\Users\\Administrator\\Desktop\\Share\\qt-beginner\\qt-beginner\\data");
    qDebug() << util->entries().length();
}

