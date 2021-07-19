#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStateMachine>

#include "updatethread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(UpdateThread &thread, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void setUI(const QString productLabel,
               const QString statusLabel,
               int progressValue);

private:
    Ui::MainWindow *ui;
    UpdateThread &d_thread;

};
#endif // MAINWINDOW_H
