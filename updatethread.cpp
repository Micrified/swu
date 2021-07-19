#include "updatethread.h"

UpdateThread::UpdateThread(QObject *parent):
    QThread(parent),
    d_abort(false)
{

}

UpdateThread::~UpdateThread()
{
}

void UpdateThread::initUI(const QString productLabel,
                          const QString statusLabel)
{
    d_productLabel = productLabel;
    d_statusLabel = statusLabel;
    d_progressValue = 0;
    emit setUI(d_productLabel, d_statusLabel, d_progressValue);
}

void UpdateThread::setStatus(const QString statusLabel)
{
    d_statusLabel = statusLabel;
    emit setUI(d_productLabel, d_statusLabel, d_progressValue);
}

void UpdateThread::setProduct(const QString productLabel)
{
    d_productLabel = productLabel;
    emit setUI(d_productLabel, d_statusLabel, d_progressValue);
}

void UpdateThread::setProgress(int progressValue)
{
    d_progressValue = progressValue;
    emit setUI(d_productLabel, d_statusLabel, d_progressValue);
}

void UpdateThread::updateUI(const QString statusLabel,
                            int progressValue)
{
    d_statusLabel = statusLabel;
    d_progressValue = progressValue;
    emit setUI(d_productLabel, d_statusLabel, d_progressValue);
}

void UpdateThread::run()
{}

