#include "operation.h"
using namespace SWU;

/*
 *******************************************************************************
 *                            Forward declarations                             *
 *******************************************************************************
*/


static OperationResultType remove_file (const QString filename);
static OperationResultType remove_directory (const QString dirname);
static OperationResultType copy_directory (const QString dirname,
                                           const QString directory,
                                           bool force);
static OperationResultType copy_file (const QString filename,
                                      const QString directory,
                                      bool force);


/*
 *******************************************************************************
 *                         Member function definitions                         *
 *******************************************************************************
*/


Operation::Operation():
    d_resource_type(RESOURCE_ENUM_MAX),
    d_operation_type(OPERATION_ENUM_MAX)
{

}

void Operation::setExpectOperation(const QString from,
                                   ResourceType resourceType)
{
    d_from = from;
    d_resource_type = resourceType;
    d_operation_type = OPERATION_EXPECT;
}

void Operation::setCopyOperation (const QString from,
                                  const QString to,
                                  bool force,
                                  ResourceType resourceType)
{
    d_from  = from;
    d_to    = to;
    d_force = force;
    d_resource_type = resourceType;
    d_operation_type = OPERATION_COPY;
}

void Operation::setMoveOperation (const QString from,
                                  const QString to,
                                  bool force,
                                  ResourceType resourceType)
{
    d_from  = from;
    d_to    = to;
    d_force = force;
    d_resource_type = resourceType;
    d_operation_type = OPERATION_MOVE;
}

void Operation::setRemoveOperation (const QString from,
                                    ResourceType resourceType)
{
    d_from = from;
    d_resource_type = resourceType;
    d_operation_type = OPERATION_REMOVE;
}

OperationResultType Operation::execute ()
{
    OperationResultType result = RESULT_OK;
    switch (d_operation_type) {
    case OPERATION_EXPECT:
        if (RESOURCE_FILE == d_resource_type) {
            // TODO: Check if file exists, optionally run hash
            qInfo() << "Vroom: File " << d_from << " is checked!" << Qt::endl;
        } else {
            // TODO: Check if directory exists,
            qInfo() << "Vroom: Directory " << d_from << " is checked!" << Qt::endl;
        }
        break;
    case OPERATION_COPY:
        if (RESOURCE_FILE == d_resource_type) {
            result = copy_file(d_from, d_to, d_force);
        } else {
            result = copy_directory(d_from, d_to, d_force);
        }
        break;
    case OPERATION_MOVE:
//        if (RESOURCE_FILE == d_resource_type) {
//        } else {

//        }
        break;
    case OPERATION_REMOVE:
        if (RESOURCE_FILE == d_resource_type) {
            result = remove_file(d_from);
        } else {
            result = remove_directory(d_from);
        }
        break;
    default:
        break;
    }

    return result;
}

QString Operation::errstr ()
{
    return d_errstr;
}


/*
 *******************************************************************************
 *                            Function definitions                             *
 *******************************************************************************
*/


static OperationResultType copy_file (const QString filename,
                                      const QString directory,
                                      bool force)
{
    OperationResultType result = RESULT_OK;

    qDebug() << "copy_file(" << filename << ","
             << directory << ") [force = " << force << "]" << Qt::endl;

    // Check if source file exists
    const QFileInfo source(filename);
    if (false == source.exists()) {
        qDebug() << " --- Source file does not exist!" << Qt::endl;
        return RESULT_BAD_SOURCE;
    }

    // Check if the destination directory exists
    const QDir destination(directory);
    if (false == destination.exists() && false == force) {
        qDebug() << " --- Destination directory does not exist!" << Qt::endl;
        return RESULT_BAD_DESTINATION;
    }

    // Create the directory if needed
    if (false == destination.exists() && false == destination.mkpath(directory)) {
        qDebug() << " --- Unable to create destination directory" << Qt::endl;
        return RESULT_BAD_DESTINATION;
    }

    // Remove any existing file (if force is specified)
    QFileInfo copy = destination.absoluteFilePath(source.fileName());
    qDebug() << "[!] Checking if a copy exists already at: " << copy.filePath() << Qt::endl;
    if (copy.exists() && false == force) {
        return RESULT_BAD_DESTINATION;
    } else {
        qDebug() << " --- Removed already existing file at: " << copy.filePath() << Qt::endl;
        remove_file(copy.absoluteFilePath());
    }

    // Copy the file
    if (false == QFile::copy(filename, QDir(directory).filePath(source.fileName()))) {
        qDebug() << " --- Bad result on QFile::copy(" << filename << "," << directory << ")" << Qt::endl;
        return RESULT_BAD_DESTINATION;
    }

    return result;
}

static OperationResultType copy_directory (const QString dirname, const QString directory, bool force)
{
    qDebug() << "copy_directory(" << dirname << "," << directory << ") [force = " << force << "]" << Qt::endl;

    // Check if source directory exists
    const QDir source(dirname);
    if (false == source.exists()) {
        qDebug() << " --- Source directory does not exist!" << Qt::endl;
        return RESULT_BAD_SOURCE;
    }

    qDebug() << " A " << Qt::endl;

    // Check if the destination directory exists
    const QDir destination(directory);
    if (false == destination.exists() && false == force) {
        qDebug() << " --- Destination directory does not exist (and no force)!" << Qt::endl;
        return RESULT_BAD_DESTINATION;
    }

    qDebug() << " B " << Qt::endl;

    // Create destination directory name
    qDebug() << "Source.dirname() = " << source.dirName() << Qt::endl;
    QDir new_destination(destination.absoluteFilePath(source.dirName()));

    // Create directory if necessary
    if (false == new_destination.exists() && false == new_destination.mkpath(new_destination.path())) {
        qDebug() << " --- Unable to create destination directory at: " << new_destination.path() << Qt::endl;
        return RESULT_BAD_DESTINATION;
    } else {
        qDebug() << " --- Created: " << new_destination.path() << Qt::endl;
    }

    qDebug() << " C " << Qt::endl;

    // Copy the directory (recursive - perhaps unwise with limited stack)
    const QFlags<QDir::Filter> flags = QDir::Filter::Dirs | QDir::Filter::Files | QDir::Filter::NoSymLinks |
                                       QDir::Filter::NoDotAndDotDot | QDir::Filter::Hidden;
    QFileInfoList contents = source.entryInfoList(flags, QDir::DirsFirst);
    qDebug() << "There are " << contents.size() << " elements inside directory " << source.dirName() << Qt::endl;
    for (off_t i = 0; i < contents.size(); ++i) {
        qDebug() << "Item " << i << Qt::endl;
        QFileInfo item = contents.at(i);
        OperationResultType result;
        if (item.isDir()) {
            result = copy_directory(item.absoluteFilePath(), new_destination.path(), force);
        } else {
            result = copy_file(item.absoluteFilePath(), new_destination.path(), force);
        }
        if (RESULT_OK != result) {
            return result;
        }
    }

    qDebug() << " D " << Qt::endl;

    return RESULT_OK;
}

static OperationResultType remove_file (const QString filename)
{
    QFileInfo file(filename);

    // Assert whether file exists
    if (false == file.exists()) {
        return RESULT_BAD_SOURCE;
    }

    // Remove file
    if (false == QFile::remove(filename)) {
        return RESULT_BAD_SOURCE;
    }

    return RESULT_OK;
}

static OperationResultType remove_directory (const QString dirname)
{
    QDir directory(dirname);

    // Assert whether directory exists
    if (false == directory.exists()) {
        return RESULT_BAD_SOURCE;
    }

    // Remove directory
    if (false == directory.removeRecursively()) {
        return RESULT_BAD_SOURCE;
    }

    return RESULT_OK;
}
