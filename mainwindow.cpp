#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


}

Ui::MainWindow *MainWindow::getUI()
{
    return ui;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    qInfo() << "Pushed!\n";
    //d_machine->postEvent(new QEvent(nullptr), 0);
}
