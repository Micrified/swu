#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "math.h"
#include "cfgxmlhandler.h"
#include "element.h"
#include "cfgelement.h"
#include "cfgparser.h"
#include "cfgupdater.h"
#include "updatethread.h"

#include <QApplication>
#include <QFileInfo>
#include <QtXml>
#include <QThread>
#include <QStorageInfo>
#include <iostream>
#include <memory>

#define STOP_SERVICE_STEP   1
#define START_SERVICE_STEP  1


/*!
 * \brief Returns pointer to QString encoded stylesheet
 * \param path Path to the stylesheet file
 * \return Stylesheet as a QString
 */
static std::unique_ptr<QString> getStyleSheet (const QString path)
{
    QFile file(path);
    QString contents;

    // Validate file
    if (!(file.exists() )) {
        qCritical() << "File exists:" << file.exists() << " readable: " << file.isReadable();
        return nullptr;
    }

    // Read file contents
    file.open(QFile::ReadOnly);
    contents = QLatin1String(file.readAll());

    // Return contents
    return std::unique_ptr<QString>(new QString(contents));
}


/*!
 * \brief Configures an Updater instance using a configuration file
 * \param config_filename Path to the configuration file to read
 * \param delegate Delegate that will be given to the Updater instance
 * \return
 */
static std::shared_ptr<SWU::Parser> getParser (const char *config_filename)
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
    std::shared_ptr<SWU::Parser> parser = std::make_shared<SWU::Parser>(config_elements);
    if (SWU::PARSE_OK != parser->status()) {
        qCritical() << "SWU Parse: Failed for reason: " << QString(parser->fault()) ;
        return nullptr;
    } else {
        return parser;
    }
}


/* Enable: systemd service */
static bool systemd_service_stop (QString service)
{
    bool retval = true;
#ifndef QT_DEBUG
    retval = QProcess::execute("systemctl", QStringList() << "stop" << service);
#else
    Q_UNUSED(service);
#endif
    return retval;
}

static bool systemd_service_start (QString service)
{
    bool retval = true;
#ifndef QT_DEBUG
    retval = QProcess::execute("systemctl", QStringList() << "start" << service);
#else
    Q_UNUSED(service);
#endif
    return retval;
}

/*!
 * \brief Implements the UpdateDelegate interface
 *
 * This class implements the methods necessary to correctly configure and execute
 * an update. This includes locating (possibly mounting) and passing the update
 * resource URI to the updater, perform pre and post update actions, and optionally
 * taking action on backup, update, and validate operations.
 */
class MyUpdaterThread : public UpdateThread, public SWU::UpdateDelegate {
private:
    std::shared_ptr<SWU::Updater> d_updater_ptr;
    off_t d_steps, d_total_steps; /**< Update progress is tracked using steps */
    QString d_product_id; /**< An example field used to hold a generated product ID string */
public:
    MyUpdaterThread(std::shared_ptr<SWU::Parser> parser, QObject *parent = nullptr):
        UpdateThread(parent),
        d_updater_ptr(nullptr)
    {

        // Init the updater
        d_updater_ptr = std::make_shared<SWU::Updater>(parser, *this);
    }

    /*!
     * \brief Increments the step counter
     * \return Normalised progress value as an integer on a scale of 0 to 100
     */
    int advanceStep()
    {
        if (d_steps < d_total_steps) {
            d_steps++;
        }
        return step();
    }

    int step ()
    {
        float real = (float)d_steps / (float)d_total_steps * 100.0;
        int natural = (int)ceil(real);
        return natural;
    }


    /*!
     * \brief All pre-update tasks go here
     * \param updater A reference to the updater object
     * \return STATUS_OK if the updater should continue, else error status.
     */
    SWU::UpdateStatus on_init (SWU::Updater &updater) override
    {
        QString productLabel;
        QString statusLabel;
        int progressValue;

        // Set: product label (shown onscreen)
        productLabel = updater.product();
        statusLabel  = "Starting updater ...";
        initUI(productLabel, statusLabel);

        // Set: product ID (custom ID generated using product and platform)
        QString product_id = QString("%1 %2").arg(updater.product(), updater.platform());
        d_product_id = product_id.replace(" ", "_").toLower();

        // Init progress tracking
        d_steps = 0;
        d_total_steps = updater.operationCount() +
                        STOP_SERVICE_STEP +
                        START_SERVICE_STEP;

        // Set: UI for stopping services
        statusLabel = "Stopping services ...";
        setStatus(statusLabel);
        QThread::msleep(500);

        // Stop: Running instances of target to be updated
        if (systemd_service_stop("backend.service")) {
            qInfo() << "Notice: Failed to stop backend, continuing anyways";
        }
        if (systemd_service_stop("frontend.service")) {
            qInfo() << "Notice: Failed to stop frontend, continuing anyways";
        }

        // Set: final UI
        progressValue = advanceStep();
        updateUI(statusLabel, progressValue);

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
            QVector<QString> &resource_uris) override
    {
        QString statusLabel;
        int progressValue;
        QString search_term, resource_path = nullptr;

        // Set: pre UI
        statusLabel = "Locating update resource ...";
        setStatus(statusLabel);
        QThread::msleep(500);

        // Peruse connected media
        foreach(const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {

            // Update: UI
            statusLabel = QString("Scanning %1 for %2 ...").arg(storage.rootPath(), d_product_id);
            setStatus(statusLabel);
            QThread::msleep(250);

            // Check if the storage name is found in the list of expected resource URIs
            foreach (const QString &uri, resource_uris) {
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
        progressValue = advanceStep();
        updateUI(statusLabel, progressValue);

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
    SWU::UpdateStatus on_pre_validate (std::shared_ptr<SWU::ExpectOperation> op, off_t index) override
    {
        QString statusLabel;
        int progressValue;

        // Set: pre UI
        statusLabel = QString("Verifying %1 ...").arg(index);
        setStatus(statusLabel);
        QThread::msleep(500);

        // Unimplemented (do whatever checks here)
        Q_UNUSED(op);

        // Set: post UI
        progressValue = advanceStep();
        updateUI(statusLabel, progressValue);

        return SWU::STATUS_OK;
    }

    /*!
     * \brief Take (optional) action on the backup operation that will occur.
     * \param op The backup operation
     * \return STATUS_OK if the updater should continue, else error status
     */
    SWU::UpdateStatus on_pre_backup (std::shared_ptr<SWU::CopyOperation> op, off_t index) override
    {
        QString statusLabel;
        int progressValue;

        // Set: pre UI
        statusLabel = QString("Backing up %1 ...").arg(index);
        setStatus(statusLabel);
        QThread::msleep(500);

        // Unimplemented (do whatever checks here)
        Q_UNUSED(op);

        // Set: post UI
        progressValue = advanceStep();
        updateUI(statusLabel, progressValue);

        return SWU::STATUS_OK;
    }

    /*!
     * \brief Take (optional) action on the update operation that will occur.
     * \param op The update operation (either a copy or remove)
     * \return STATUS_OK if the updater should continue, else error status
     */
    SWU::UpdateStatus on_pre_update (std::shared_ptr<SWU::FSOperation> op, off_t index) override
    {
        QString statusLabel;
        int progressValue;

        // Set: pre UI
        statusLabel = QString("Updating %1 ...").arg(index);
        setStatus(statusLabel);
        QThread::msleep(500);

        // Unimplemented (do whatever checks here)
        Q_UNUSED(op);

        // Set: post UI
        progressValue = advanceStep();
        updateUI(statusLabel, progressValue);

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
                               SWU::OperationResult op_result) override
    {
        Q_UNUSED(op);
        Q_UNUSED(op_result);
        QString statusLabel;
        int progressValue = step();
        bool shouldTerminate = true;
        bool shouldRecover = false;

        switch (status) {
            case SWU::STATUS_OK:
                statusLabel = "Restarting services ...";
                break;
            case SWU::STATUS_BAD_PLATFORM:
                statusLabel = "Incompatible platform!";
                progressValue = -1;
                break;

            case SWU::STATUS_RESOURCE_NOT_FOUND:
                statusLabel = "No update found!";
                progressValue = -1;
                break;

            default:
                statusLabel = "Recovering from exception ...";
                progressValue = -1;
                shouldRecover = true;
                break;
        }

        // Display status with delay
        updateUI(statusLabel, progressValue);
        QThread::msleep(1000);

        // Recover (if required)
        if (shouldRecover) {
            if (SWU::STATUS_OK != updater.undo()) {
                statusLabel = "Unable to recover. Contact support!";
                shouldTerminate = false;
            } else {
                statusLabel = "Recovered from backup successfully!";
            }

            // Display status with delay
            updateUI(statusLabel, progressValue);
            QThread::msleep(500);
        }


        // On terminate condition
        if (shouldTerminate) {

            // Restart services
            if (systemd_service_start("backend.service")) {
                qCritical() << "Failed to start backend, continuing anyways";
            }
            if (systemd_service_start("frontend.service")) {
                qCritical() << "Failed to start frontend, continuing anyways";
            }

            // Countdown to return
            for (off_t i = 3; i >= 0; --i) {
                statusLabel = QString("Returning in %1").arg(i);
                setProduct(statusLabel);
                QThread::msleep(1000);
            }

            // Return OK (quit condition)
            return SWU::STATUS_OK;
        }

        // Else stay onscreen
        return status;
    }

    void run() override {

        // If executed successfully, then quit.
        if (SWU::STATUS_OK == d_updater_ptr->execute()) {
            QApplication::quit();
        } else {
            QThread::wait(QDeadlineTimer::Forever);
        }
    }
};


int main(int argc, char *argv[])
{
    std::unique_ptr<QString> css;

    // Check arguments
    if (argc <= 1) {
        qCritical() << "No descriptor XML file provided!";
        return EXIT_FAILURE;
    }

    // Create application
    QApplication a(argc, argv);
    if (argc > 2) {
        if ((nullptr == (css = getStyleSheet(argv[2])))) {
            qWarning() << "Unable to read stylesheet: \"" << QString(argv[2])
                       << "\"";
        } else {
            a.setStyleSheet(*css);
        }
    } else {
        qInfo() << "No stylesheet provided";
    }

    // Get the parser
    std::shared_ptr<SWU::Parser> parser = getParser(argv[1]);
    if (nullptr == parser) {
        return EXIT_FAILURE;
    }

    // Create the updater thread
    MyUpdaterThread updaterThread(parser);

    // Create and show window
    MainWindow w(updaterThread);
    w.show();

    // Run the update routine
    updaterThread.start();

    return a.exec();
}
