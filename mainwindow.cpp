#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Configure state-machine
    QStateMachine *machine = new QStateMachine();
    QState *start = new QState();
    QState *final = new QState();

    machine->addState(start);
    machine->addState(final);

    start->addTransition(this, SIGNAL(on_pushButton_clicked), final);

    machine->setInitialState(start);
    machine->start();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete d_machine;
}


void MainWindow::on_pushButton_clicked()
{
    qInfo() << "Pushed!\n";
    //d_machine->postEvent(new QEvent(nullptr), 0);
}
