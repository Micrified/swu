#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "math.h"
#include "cfgxmlhandler.h"
#include "element.h"
#include "cfgelement.h"
#include "cfgparser.h"
#include "cfgupdater.h"

#include <QApplication>
#include <QtXml>
#include <QThread>
#include <QStorageInfo>
#include <iostream>

#define STOP_SERVICE_STEP   1
#define START_SERVICE_STEP  1

std::shared_ptr<SWU::Updater> getUpdater (const char *config_filename, SWU::UpdateDelegate &delegate)
{
    FILE *file_handle;
    QFile file;
    QXmlInputSource inputSource;
    QXmlSimpleReader reader;
    ConfigXMLHandler handler;

    // Open file
    if (nullptr == (file_handle = fopen(config_filename, "r"))) {
        qCritical() << "FILE: Cannot open: " << QString(config_filename) << ": " << QString(strerror(errno)) << Qt::endl;
        return nullptr;
    }

    // Convert to QFile
    if (false == file.open(file_handle, QIODevice::ReadOnly)) {
        qCritical() << "QFile: Cannot open: " << QString(config_filename) << Qt::endl;
        return nullptr;
    }

    // Set the XML input source
    inputSource.setData(file.readAll());

    // Set the XML content handler
    reader.setContentHandler(&handler);

    // Parser input
    reader.parse(inputSource);

    // Check handler output
    if (false == handler.parsed()) {
        qCritical() << "XML Parse: Failed" << Qt::endl;
        return nullptr;
    }

    // Close the file
    file.close();

    // Convert XML elements to SWU ones
    QVector<std::shared_ptr<SWU::CFGElement>> config_elements;
    for (auto element : handler.elementStack()) {
        config_elements.push_back(std::make_shared<SWU::CFGElement>(element));
    }

    // Parse the SWU elements now
    SWU::Parser parser(config_elements);
    if (SWU::PARSE_OK != parser.status()) {
        qCritical() << "SWU Parse: Failed for reason: " << QString(parser.fault()) << Qt::endl;
        return nullptr;
    }

    // Create the updater
    std::shared_ptr<SWU::Updater> updater = std::make_shared<SWU::Updater>(SWU::Updater(parser, delegate));

    // Return updater
    return updater;
}


/* Class which implements the updater interface */
class MyUpdateDelegate : public SWU::UpdateDelegate {
private:
    MainWindow* d_window;
    off_t d_steps, d_total_steps;
    QString d_product_id;
public:
    MyUpdateDelegate(MainWindow *w):
        d_window(w)
    {}

    int advanceStep()
    {
        d_steps++;
        float real = (float)d_steps / (float)d_total_steps * 100.0;
        int natural = (int)ceil(real);
        return natural;
    }


    SWU::UpdateStatus on_init (SWU::Updater &updater)
    {
        qInfo() << "on_init()" << Qt::endl;

        // Set the product label
        d_window->getUI()->productLabel->setText(updater.product());

        // Create the product ID
        QString product_id = QString("%1 %2").arg(updater.product(), updater.platform());
        d_product_id = product_id.replace(" ", "_").toLower();

        // Set the step counter
        d_steps = 0;
        d_total_steps = updater.operationCount() +
                        STOP_SERVICE_STEP +
                        START_SERVICE_STEP;

        // Set pre UI
        d_window->getUI()->statusLabel->setText("Stopping services");
        QThread::msleep(500);

        // TODO: systemctl stop frontend.service

        // TODO: systemctl stop backend.service

        // Set post UI
        d_window->getUI()->progressBar->setValue(advanceStep());

        return SWU::STATUS_OK;
    }

    SWU::UpdateStatus on_configure_resource_manager (
            SWU::ResourceManager &resourceManager,
            QVector<QString> &resource_uris)
    {
        // Set pre UI
        d_window->getUI()->statusLabel->setText("Locating update resource");
        QThread::msleep(500);

        // Default: No path found
        QString resource_path = nullptr;

        // Locate connected media
        for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {
            qInfo() << "Storage found: " << storage.rootPath();

            // Set on UI
            d_window->getUI()->statusLabel->setText(QString("Scanning %1").arg(storage.rootPath()));
            QThread::msleep(250);

            // Compare it to each search term
            for (auto uri : resource_uris) {
                if (storage.rootPath().contains(uri, Qt::CaseInsensitive)) {
                    resource_path = storage.rootPath();
                    break;
                }
            }

            // Continue if an invalid path was encountered
            if (resource_path == nullptr) {
                continue;
            }

            // Build the search term
            QString search_term = d_product_id + "*";

            // Scan for the folder
            const QStringList &directoryList = QDir(resource_path).entryList(QStringList() << d_product_id,
                (QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot), QDir::Name);

            // Exit if no entry found
            if (directoryList.length() == 0) {
                resource_path = nullptr;
                break;
            }

            // Otherwise set the resource path
            resource_path = QDir(resource_path).filePath(directoryList.at(0));
            qInfo() << "Remote found: " << resource_path;
        }

        // Set post UI
        d_window->getUI()->progressBar->setValue(advanceStep());

        // If the resource path is set, then set it
        if (resource_path != nullptr) {
            resourceManager.setResourcePath(SWU::RESOURCE_KEY_REMOTE, resource_path);
            return SWU::STATUS_OK;
        } else {
            return SWU::STATUS_RESOURCE_NOT_FOUND;
        }
    }

    SWU::UpdateStatus on_pre_validate (std::shared_ptr<SWU::ExpectOperation> op)
    {

        // Set pre UI
        QString label = QString("Verifying existence of: %1").arg(op.get()->label());
        d_window->getUI()->statusLabel->setText(label);
        QThread::msleep(500);

        // Unimplemented

        // Set post UI
        d_window->getUI()->progressBar->setValue(advanceStep());

        return SWU::STATUS_OK;
    }

    SWU::UpdateStatus on_pre_backup (std::shared_ptr<SWU::CopyOperation> op)
    {
        // Set pre UI
        QString label = QString("Backing up: %1").arg(op.get()->label());
        d_window->getUI()->statusLabel->setText(label);
        QThread::msleep(500);

        // Unimplemented

        // Set post UI
        d_window->getUI()->progressBar->setValue(advanceStep());

        return SWU::STATUS_OK;
    }

    SWU::UpdateStatus on_pre_update (std::shared_ptr<SWU::FSOperation> op)
    {
        // Set pre UI
        QString label = QString("Updating: %1").arg(op.get()->label());
        d_window->getUI()->statusLabel->setText(label);
        QThread::msleep(500);

        // Unimplemented

        // Set post UI
        d_window->getUI()->progressBar->setValue(advanceStep());

        return SWU::STATUS_OK;
    }

    SWU::UpdateStatus on_exit (SWU::Updater &updater,
                               SWU::UpdateStatus status,
                               std::shared_ptr<SWU::FSOperation> op,
                               SWU::OperationResult op_result)
    {
        QString last_message = "Update complete";

        // Whether or not to stay on screen (default no)
        bool should_stay_onscreen = false;

        // Switch on the status
        switch (status) {
            case SWU::STATUS_OK: {
                    // Set UI
                    d_window->getUI()->statusLabel->setText("Restarting services");
                    QThread::msleep(500);
                }
                break;
            case SWU::STATUS_BAD_PLATFORM: {
                    // Set UI
                    last_message = "Incompatible platform";
                    d_steps = -1;
                    QThread::msleep(500);
                }
                break;

            case SWU::STATUS_RESOURCE_NOT_FOUND: {
                    // Set UI
                    last_message = "No update found";
                    d_steps = -1;
                    QThread::msleep(500);
                }
                break;

            default: {
                    // Set UI
                    d_window->getUI()->statusLabel->setText("Exception: restoring ...");
                    QThread::msleep(500);

                    // Adjust final message
                    if (updater.undo() != SWU::STATUS_OK) {
                        last_message = "Fatal exception: contact support";
                        should_stay_onscreen = true;
                    } else {
                        last_message = "Restored from backup successfully";
                    }

                    // Reset progress
                    d_steps = -1;
                }
                break;
        }

        // TODO: systemctl start backend.service

        // TODO: systemctl start frontend.service

        // Set post UI
        d_window->getUI()->progressBar->setValue(advanceStep());
        d_window->getUI()->statusLabel->setText(last_message);

        if (should_stay_onscreen == false) {
            // TODO: Kill self
        }

        return status;
    }
};

class WorkThread : public QThread {

private:
    MyUpdateDelegate d_update_delegate;
    MainWindow *d_window_p;
    std::shared_ptr<SWU::Updater> d_updater;
public:
    WorkThread(const char *config_filename, MainWindow *window_p):
        QThread(),
        d_update_delegate(window_p),
        d_window_p(window_p),
        d_updater(nullptr)
    {

        // Configure updater
        d_updater = getUpdater(config_filename, d_update_delegate);
    }
    void run() override
    {
        d_updater->execute();
    }
};

int main(int argc, char *argv[])
{
    // Check arguments
    if (argc <= 1) {
        qCritical() << "No descriptor XML file provided!";
        return EXIT_FAILURE;
    }

    // Create application window
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

//    // Create my update delegate
//    MyUpdateDelegate myDelegate = MyUpdateDelegate(&w);

//    // Get the updater
//    std::shared_ptr<SWU::Updater> updater = getUpdater(argv[1], myDelegate);

//    // Execute the updater
//    if (updater->execute() != SWU::STATUS_OK) {
//        qCritical() << "Failed to run the update routine!";
//    }

    WorkThread updateWorkThread(argv[1], &w);
    updateWorkThread.start();
    return a.exec();
}
