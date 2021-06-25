#include "mainwindow.h"
#include "cfgxmlhandler.h"
//#include "operation.h"
#include "element.h"
//#include "update.h"
#include <QApplication>
#include <QtXml>

int parseSoftwareUpdate (const char *filename)
{
    FILE *file_handle;
    QFile file;
    QXmlInputSource inputSource;
    QXmlSimpleReader reader;
    ConfigXMLHandler handler;

    // Get file-descriptor for filename
    if (nullptr == (file_handle = fopen(filename, "r"))) {
        qCritical() << "Cannot find file " << QString(filename) << Qt::endl;
        return -1;
    }

    qInfo() << "File open 1 okay!";

    // Open for reading via QFile
    if (false == file.open(file_handle, QIODevice::ReadOnly)) {
        return -1;
    }

    qInfo() << "File open seemed okay!";

    // Dump file contents into input stream
    inputSource.setData(file.readAll());

    // Parse with custom content handler
    reader.setContentHandler(&handler);
    reader.parse(inputSource);

    QVector<std::shared_ptr<SWU::Element>> elements = handler.elementStack();
    for (auto e : elements) {
        qCritical() << e->description();
    }

    if (handler.parsed()) {
      qInfo() << "Parse succeeded!";
    }

    // Build software update from token stack
  //  std::unique_ptr<SWU::Update> swupdate(new SWU::Update(handler.tokenStack()));

    // Close the file
    file.close();
    if (0 != fclose(file_handle)) {
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int check = -1;

    // Check arguments
    if (argc <= 1) {
        qCritical() << "No descriptor XML file provided!";
        return EXIT_FAILURE;
    }

    // Parse the XML into the update class
    if (-1 == (check = parseSoftwareUpdate(argv[1]))) {
        qCritical() << "Unable to parse software update XML file provided!";
        return EXIT_FAILURE;
    }

    // Perform the copy operations


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
