#ifndef UPDATETHREAD_H
#define UPDATETHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>


class UpdateThread : public QThread
{
    Q_OBJECT
public:
    UpdateThread(QObject *parent = nullptr);
    ~UpdateThread();
    void initUI(const QString productLabel, const QString statusLabel);
    void setStatus (const QString statusLabel);
    void updateUI(const QString statusLabel, int progressValue);
signals:
    void setUI(const QString productLabel,
               const QString statusLabel,
               int progressValue);
protected:
    virtual void run() override;
private:
    bool d_abort, d_show_progress;
    QString d_productLabel, d_statusLabel;
    int d_progressValue;
};

#endif // UPDATETHREAD_H
