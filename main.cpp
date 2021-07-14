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

/*!
 * \brief Configures an Updater instance using a configuration file
 * \param config_filename Path to the configuration file to read
 * \param delegate Delegate that will be given to the Updater instance
 * \return
 */
std::shared_ptr<SWU::Updater> getUpdater (const char *config_filename, SWU::UpdateDelegate &delegate)
{
    FILE *file_handle;
    QFile file;
    QXmlInputSource inputSource;
    QXmlSimpleReader reader;
    ConfigXMLHandler handler;

    // Open file
    if (nullptr == (file_handle = fopen(config_filename, "r"))) {
        qCritical() << "FILE: Cannot open: " << QString(config_filename) << ": " << QString(strerror(errno)) ;
        return nullptr;
    }

    // Convert to QFile
    if (false == file.open(file_handle, QIODevice::ReadOnly)) {
        qCritical() << "QFile: Cannot open: " << QString(config_filename) ;
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
        qCritical() << "XML Parse: Failed" ;
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
        qCritical() << "SWU Parse: Failed for reason: " << QString(parser.fault()) ;
        return nullptr;
    }

    // Create the updater
    std::shared_ptr<SWU::Updater> updater = std::make_shared<SWU::Updater>(SWU::Updater(parser, delegate));

    // Return updater
    return updater;
}


/* Enable: systemd service */
static int systemd_service_stop (QString service)
{
    return QProcess::execute("systemctl", QStringList() << "stop" << service);
}

static bool systemd_service_start (QString service)
{
    return QProcess::execute("systemctl", QStringList() << "start" << service);
}

/*!
 * \brief Implements the UpdateDelegate interface
 *
 * This class implements the methods necessary to correctly configure and execute
 * an update. This includes locating (possibly mounting) and passing the update
 * resource URI to the updater, perform pre and post update actions, and optionally
 * taking action on backup, update, and validate operations.
 */
class MyUpdateDelegate : public SWU::UpdateDelegate {
private:
    MainWindow* d_window; /**< Pointer to the window to be updated */
    off_t d_steps, d_total_steps; /**< Update progress is tracked using steps */
    QString d_product_id; /**< An example field used to hold a generated product ID string */
public:
    MyUpdateDelegate(MainWindow *w):
        d_window(w)
    {}

    /*!
     * \brief Increments the step counter
     * \return Normalised progress value as an integer on a scale of 0 to 100
     */
    int advanceStep()
    {
        if (d_steps < d_total_steps) {
            d_steps++;
        }
        float real = (float)d_steps / (float)d_total_steps * 100.0;
        int natural = (int)ceil(real);
        return natural;
    }


    /*!
     * \brief All pre-update tasks go here
     * \param updater A reference to the updater object
     * \return STATUS_OK if the updater should continue, else error status.
     */
    SWU::UpdateStatus on_init (SWU::Updater &updater)
    {
        // Set: product label (shown onscreen)
        d_window->getUI()->productLabel->setText(updater.product());

        // Set: product ID (custom ID generated using product and platform)
        QString product_id = QString("%1 %2").arg(updater.product(), updater.platform());
        d_product_id = product_id.replace(" ", "_").toLower();

        // Set: progress bar value
        d_steps = 0;
        d_total_steps = updater.operationCount() +
                        STOP_SERVICE_STEP +
                        START_SERVICE_STEP;

        // Set: pre UI
        d_window->getUI()->statusLabel->setText("Stopping services");
        QThread::msleep(500);

        // Stop: Running instances of target to be updated
        if (systemd_service_stop("backend.service")) {
            qInfo() << "Notice: Failed to stop backend, continuing anyways";
        }
        if (systemd_service_stop("frontend.service")) {
            qInfo() << "Notice: Failed to stop frontend, continuing anyways";
        }

        // Set: post UI
        d_window->getUI()->progressBar->setValue(advanceStep());

        return SWU::STATUS_OK;
    }

    /*!
     * \brief The path to the update resource (typically a mounted FS) is assigned here
     * \param resourceManager The singleton instance to place the resource path in
     * \param resource_uris The resource URIs that were specified in the config file
     * \return STATUS_OK if the updater should continue, else error status
     */
    SWU::UpdateStatus on_configure_resource_manager (
            SWU::ResourceManager &resourceManager,
            QVector<QString> &resource_uris)
    {
        QString search_term, resource_path = nullptr;

        // Set: pre UI
        d_window->getUI()->statusLabel->setText("Locating update resource");
        QThread::msleep(500);

        // Peruse connected media
        for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {

            // Update: UI
            d_window->getUI()->statusLabel->setText(QString("Scanning %1").arg(storage.rootPath()));
            QThread::msleep(250);

            // Check if the storage name is found in the list of expected resource URIs
            for (auto uri : resource_uris) {
                if (storage.rootPath().contains(uri, Qt::CaseInsensitive)) {
                    resource_path = storage.rootPath();
                    break;
                }
            }

            // If no path was set, continue to check the next storage path
            if (resource_path == nullptr) {
                continue;
            }

            // Set the search term (wildcard to account for possible date or version suffixes)
            search_term = d_product_id + "*";

            // Collect matching files at the given resource path that match the search term, sorted
            // lexicographically by name
            const QStringList &directoryList = QDir(resource_path).entryList(QStringList() << d_product_id,
                (QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot), QDir::Name);

            // Exit if no entry found
            if (directoryList.length() == 0) {
                resource_path = nullptr;
                break;
            }

            // Otherwise set the resource path to the firest match
            resource_path = QDir(resource_path).filePath(directoryList.at(0));
        }

        // Set: post UI
        d_window->getUI()->progressBar->setValue(advanceStep());

        // If resource path found, then assign to resource manager.
        if (resource_path != nullptr) {
            resourceManager.setResourcePath(SWU::RESOURCE_KEY_REMOTE, resource_path);
            return SWU::STATUS_OK;
        } else {
            return SWU::STATUS_RESOURCE_NOT_FOUND;
        }
    }

    /*!
     * \brief Take (optional) action on the validate operation that will occur.
     * \param op The validate operation
     * \return STATUS_OK if the updater should continue, else error status
     */
    SWU::UpdateStatus on_pre_validate (std::shared_ptr<SWU::ExpectOperation> op)
    {

        // Set: pre UI
        QString label = QString("Verifying existence of: %1").arg(op.get()->label());
        d_window->getUI()->statusLabel->setText(label);
        QThread::msleep(500);

        // Unimplemented

        // Set: post UI
        d_window->getUI()->progressBar->setValue(advanceStep());

        return SWU::STATUS_OK;
    }

    /*!
     * \brief Take (optional) action on the backup operation that will occur.
     * \param op The backup operation
     * \return STATUS_OK if the updater should continue, else error status
     */
    SWU::UpdateStatus on_pre_backup (std::shared_ptr<SWU::CopyOperation> op)
    {
        // Set: pre UI
        QString label = QString("Backing up: %1").arg(op.get()->label());
        d_window->getUI()->statusLabel->setText(label);
        QThread::msleep(500);

        // Unimplemented

        // Set: post UI
        d_window->getUI()->progressBar->setValue(advanceStep());

        return SWU::STATUS_OK;
    }

    /*!
     * \brief Take (optional) action on the update operation that will occur.
     * \param op The update operation (either a copy or remove)
     * \return STATUS_OK if the updater should continue, else error status
     */
    SWU::UpdateStatus on_pre_update (std::shared_ptr<SWU::FSOperation> op)
    {
        // Set: pre UI
        QString label = QString("Updating: %1").arg(op.get()->label());
        d_window->getUI()->statusLabel->setText(label);
        QThread::msleep(500);

        // Unimplemented

        // Set: post UI
        d_window->getUI()->progressBar->setValue(advanceStep());

        return SWU::STATUS_OK;
    }



    /*!
     * \brief Executed when the Updater is finished or interrupted
     * \param updater Reference to the updater object
     * \param status The status code of the updater
     * \param op If not STATUS_OK, then the offending operation
     * \param op_result The exit code when executing the offending operation (if executed)
     * \return STATUS_OK if the updater succeeded, else error status
     */
    SWU::UpdateStatus on_exit (SWU::Updater &updater,
                               SWU::UpdateStatus status,
                               std::shared_ptr<SWU::FSOperation> op,
                               SWU::OperationResult op_result)
    {
        Q_UNUSED(op);
        Q_UNUSED(op_result);

        // Message to leave on screen last
        QString last_message = "Update complete";

        // Whether or not to stay on screen (default no)
        bool should_stay_onscreen = false;

        // Status switch
        switch (status) {
            case SWU::STATUS_OK: {
                    // Set: UI
                    d_window->getUI()->statusLabel->setText("Restarting services");
                    QThread::msleep(500);
                }
                break;
            case SWU::STATUS_BAD_PLATFORM: {
                    // Set: UI
                    last_message = "Incompatible platform";
                    d_steps = -1;
                    QThread::msleep(500);
                }
                break;

            case SWU::STATUS_RESOURCE_NOT_FOUND: {
                    // Set: UI
                    last_message = "No update found";
                    d_steps = -1;
                    QThread::msleep(500);
                }
                break;

            default: {
                    // Set: UI
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

        // Restart services (only if update restore didn't fail)
        if (should_stay_onscreen == false) {
            if (systemd_service_start("backend.service")) {
                qCritical() << "Failed to start backend, continuing anyways";
            }
            if (systemd_service_start("frontend.service")) {
                qCritical() << "Failed to start frontend, continuing anyways";
            }
        }

        // Set post UI
        d_window->getUI()->progressBar->setValue(advanceStep());
        d_window->getUI()->statusLabel->setText(last_message);

        // Clear the progress bar and product
        d_window->getUI()->progressBar->setHidden(true);
        d_window->getUI()->productLabel->setHidden(true);

        // Run countdown to quit, if applicable
        if (should_stay_onscreen == false) {
            d_window->getUI()->statusLabel->setText(last_message);
            for (off_t i = 3; i >= 0; --i) {
                QString time_label = QString("Returning in %1").arg(i);
                d_window->getUI()->statusLabel->setText(time_label);
                QThread::msleep(1000);
            }
            QApplication::quit();
        }

        return status;
    }
};

/*!
 * \brief The work-thread executes the update procedure in the background, occasionally
 * pushing updates to the main UI thread
 */
class WorkThread : public QThread {

private:
    MyUpdateDelegate d_update_delegate; /**< Delegate class that will handle updater events */
    MainWindow *d_window_p; /**< Pointer to the main window that will be updated */
    std::shared_ptr<SWU::Updater> d_updater; /**< The updater class instance */
public:
    WorkThread(const char *config_filename, MainWindow *window_p):
        QThread(),
        d_update_delegate(window_p),
        d_window_p(window_p),
        d_updater(getUpdater(config_filename, d_update_delegate))
    {}

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

    // Run the update routine
    WorkThread updateWorkThread(argv[1], &w);
    updateWorkThread.start();
    return a.exec();
}
