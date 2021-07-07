#include "mainwindow.h"
#include "cfgxmlhandler.h"
//#include "operation.h"
#include "element.h"
#include "cfgelement.h"
#include "cfgparser.h"
#include "cfgupdater.h"

#include <QApplication>
#include <QtXml>
#include <iostream>


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
        config_elements.push_back(std::make_shared<SWU::CFGElement>(SWU::CFGElement(element)));
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
public:
    SWU::UpdateStatus on_init (SWU::Updater &updater)
    {
        qInfo() << "on_init()" << Qt::endl;
        return SWU::STATUS_OK;
    }

    SWU::UpdateStatus on_configure_resource_manager (
            SWU::ResourceManager &resourceManager,
            QVector<QString> &resource_uris)
    {
        qInfo() << "on_configure_resource_manager()" << Qt::endl;
        for (auto s : resource_uris) {
            qInfo() << "\t" << s << Qt::endl;
        }
        return SWU::STATUS_OK;
    }

    SWU::UpdateStatus on_pre_validate (std::shared_ptr<SWU::ExpectOperation> op)
    {
        qInfo() << "on_pre_validate: " << op->label() << Qt::endl;
        return SWU::STATUS_OK;
    }

    SWU::UpdateStatus on_pre_backup (std::shared_ptr<SWU::CopyOperation> op)
    {
        qInfo() << "on_pre_backup: " << op->label() << Qt::endl;
        return SWU::STATUS_OK;
    }

    SWU::UpdateStatus on_pre_update (std::shared_ptr<SWU::FSOperation> op)
    {
        qInfo() << "on_pre_update: " << op->label() << Qt::endl;
        return SWU::STATUS_OK;
    }

    SWU::UpdateStatus on_exit (SWU::UpdateStatus status,
                               std::shared_ptr<SWU::FSOperation> op,
                               SWU::OperationResult op_result)
    {
        qInfo() << "on_exit()" << Qt::endl;

        // If error -> attempt backup

        // If backup fails, then return failure
        return status;
    }
};


int main(int argc, char *argv[])
{
    // Check arguments
    if (argc <= 1) {
        qCritical() << "No descriptor XML file provided!";
        return EXIT_FAILURE;
    }

    // Create my update delegate
    MyUpdateDelegate myDelegate = MyUpdateDelegate();

    // Get the updater
    std::shared_ptr<SWU::Updater> updater = getUpdater(argv[1], myDelegate);

    // Make a few operations
//    QString root("/Users/micrified/Documents/Projects/Qt/root");
//    QString archive("/Users/micrified/Desktop/update_package_v1.18");

//    QString frontend("frontend"), backend("backend"), languages("languages"), foo("foo");

//    QString frontend_path = QDir(archive).absoluteFilePath(frontend);
//    QString backend_path = QDir(archive).absoluteFilePath(backend);
//    QString languages_path = QDir(archive).absoluteFilePath(languages);
//    QString foo_installed_path = QDir(root).absoluteFilePath(foo);

//    qInfo() << "About to begin copying the archive into root ... " << Qt::endl;
//    Operations::Operation m1;
//    Operations::Operation m2;
//    Operations::Operation m3;
//    m1.setCopyOperation(frontend_path, root, true, Operations::RESOURCE_FILE);
//    m2.setCopyOperation(backend_path, root, true, Operations::RESOURCE_FILE);
//    m3.setCopyOperation(languages_path, root, true, Operations::RESOURCE_DIRECTORY);

//    qInfo() << "About to begin removing things from root ... " << Qt::endl;
//    Operations::Operation r1;
//    r1.setRemoveOperation(foo_installed_path, Operations::RESOURCE_DIRECTORY);

//    if (m1.execute() != Operations::RESULT_OK) {
//        qCritical() << "Unable to copy the file: " << frontend_path << " to directory: " << root << Qt::endl;
//    }
//    if (m2.execute() != Operations::RESULT_OK) {
//        qCritical() << "Unable to copy the file: " << backend_path << " to directory: " << root << Qt::endl;
//    }
//    if (m3.execute() != Operations::RESULT_OK) {
//        qCritical() << "Unable to copy the directory: " << languages_path << " to directory: " << root << Qt::endl;
//    }
//    if (r1.execute() != Operations::RESULT_OK) {
//        qCritical() << "Unable to remove the directory: " << foo_installed_path << Qt::endl;
//    }

    // Run the update application
//    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();
//    return a.exec();
    return EXIT_SUCCESS;
}
