#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>

MainWindow::MainWindow(UpdateThread &thread, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      d_thread(thread)
{
    ui->setupUi(this);

    // Connect signal/slot
    connect(&d_thread, &UpdateThread::setUI, this, &MainWindow::setUI);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setUI(const QString productLabel,
                       const QString statusLabel,
                       int progressValue)
{
    ui->productLabel->setText(productLabel);
    ui->statusLabel->setText(statusLabel);
    if (progressValue < 0) {
        ui->progressBar->setVisible(false);
    } else {
        ui->progressBar->setVisible(true);
        ui->progressBar->setValue(progressValue);
    }
}


