#include "fsoperation.h"

using namespace SWU;


/*
 *******************************************************************************
 *                            Forward declarations                             *
 *******************************************************************************
*/

static OperationResult remove_file (const QString filename);
static OperationResult remove_directory (const QString dirname);
static OperationResult copy_directory (const QString dirname,
                                       const QString directory,
                                       bool force);
static OperationResult copy_file (const QString filename,
                                  const QString directory,
                                  bool force);


/*
 *******************************************************************************
 *                         Member function definitions                         *
 *******************************************************************************
*/


FSOperation::FSOperation(){}
FSOperation::~FSOperation() = default;
OperationResult FSOperation::execute() { return RESULT_OK; }
OperationResult FSOperation::undo() { return RESULT_OK; }
OperationResult FSOperation::invert() { return RESULT_OK; }
QString FSOperation::errstr() { return nullptr; }
QString FSOperation::label() { return nullptr; }

RemoveOperation::RemoveOperation(Resource resource):
    d_resource(resource)
{}

OperationResult RemoveOperation::execute()
{
    OperationResult retval = RESULT_OK;
    ResourceManager resourceManager = ResourceManager::get_instance();
    Resource r = d_resource;

    // Build full resource path
    QString root = resourceManager.getResourcePath(r.rootKey());
    QString path = QDir(root).filePath(r.path());

    qDebug() << QString("REMOVE %2 [from = \"%1\"]\n").arg(path, (r.resourceType() == RESOURCE_TYPE_FILE ? "File" : "Dir"));

    // TODO: Copy to staging location

    // TODO: Remove from specified location

//    switch (r.resourceType()) {
//    case RESOURCE_TYPE_FILE:
//        retval = remove_file(path);
//        break;
//    case RESOURCE_TYPE_DIRECTORY:
//        retval = remove_directory(path);
//        break;
//    default:
//        retval = RESULT_BAD_RESOURCE;
//    }

    return retval;
}

OperationResult RemoveOperation::undo ()
{
    OperationResult retval = RESULT_OK;
    ResourceManager resourceManager = ResourceManager::get_instance();
    Resource r = d_resource;

    // Build full resource path
    QString root = resourceManager.getResourcePath(r.rootKey());
    QString path = QDir(root).filePath(r.path());

    qDebug() << QString("REMOVE UNDO %2 [from = \"%1\"]\n").arg(path, (r.resourceType() == RESOURCE_TYPE_FILE ? "File" : "Dir"));

    // TODO: Copy from staging location back to original

    return retval;
}

OperationResult RemoveOperation::invert ()
{
    return RESULT_OK;
}

QString RemoveOperation::errstr()
{
    // TODO: Unimplemented
    return nullptr;
}

QString RemoveOperation::label()
{
    return QString("Remove %1").arg(d_resource.path());
}

CopyOperation::CopyOperation(Resource from, Resource to):
    d_from_resource(from),
    d_to_resource(to)
{}

OperationResult CopyOperation::execute()
{
    OperationResult retval = RESULT_OK;
    ResourceManager resourceManager = ResourceManager::get_instance();
    Resource from = d_from_resource, to = d_to_resource;

    // Build full resource paths
    QString from_root = resourceManager.getResourcePath(from.rootKey());
    QString to_root = resourceManager.getResourcePath(to.rootKey());
    QString from_path = QDir(from_root).filePath(from.path());
    QString to_path = QDir(to_root).filePath(to.path());

    qDebug() << QString("COPY %3 [from = \"%1\", to = \"%2\"]\n").arg(from_path, to_path,
                 (from.resourceType() == RESOURCE_TYPE_FILE ? "File" : "Dir"));

//    switch (from.resourceType()) {
//    case RESOURCE_TYPE_FILE:
//        retval = copy_file(from_path, to_path, true);
//        break;
//    case RESOURCE_TYPE_DIRECTORY:
//        retval = copy_directory(from_path, to_path, true);
//        break;
//    default:
//        retval = RESULT_BAD_RESOURCE;
//    }

    return retval;
}

OperationResult CopyOperation::undo()
{
    OperationResult retval = RESULT_OK;
    ResourceManager resourceManager = ResourceManager::get_instance();
    Resource to_remove = d_to_resource;

    // A copy operation is undone by removing the copy at the destination location
    QString to_remove_root = resourceManager.getResourcePath(to_remove.rootKey());
    QString to_remove_path = QDir(to_remove_root).filePath(to_remove.path());

    qDebug() << QString("UNDO COPY %2 [remove = \"%1\"]\n").arg(to_remove_path,
                 (to_remove.resourceType() == RESOURCE_TYPE_FILE ? "File" : "Dir"));

    // TODO: Implement undo

    return retval;
}

OperationResult CopyOperation::invert()
{
    OperationResult retval = RESULT_OK;
    ResourceManager resourceManager = ResourceManager::get_instance();
    Resource from = d_to_resource, to = d_from_resource;

    // Build full resource paths
    QString from_root = resourceManager.getResourcePath(from.rootKey());
    QString to_root = resourceManager.getResourcePath(to.rootKey());
    QString from_path = QDir(from_root).filePath(from.path());
    QString to_path = QDir(to_root).filePath(to.path());

    qDebug() << QString("INVERT COPY %3 [from = \"%1\", to = \"%2\"]\n").arg(from_path, to_path,
                 (from.resourceType() == RESOURCE_TYPE_FILE ? "File" : "Dir"));

//    switch (from.resourceType()) {
//    case RESOURCE_TYPE_FILE:
//        retval = copy_file(from_path, to_path, true);
//        break;
//    case RESOURCE_TYPE_DIRECTORY:
//        retval = copy_directory(from_path, to_path, true);
//        break;
//    default:
//        retval = RESULT_BAD_RESOURCE;
//    }

    return retval;
}

QString CopyOperation::errstr()
{
    // TODO: Unimplemented
    return nullptr;
}

QString CopyOperation::label()
{
    return QString("Copy %1 to %2").arg(d_from_resource.path(), d_to_resource.path());
}

ExpectOperation::ExpectOperation(Resource resource):
    d_resource(resource)
{}

OperationResult ExpectOperation::execute()
{
    OperationResult retval = RESULT_OK;
    ResourceManager resourceManager = ResourceManager::get_instance();
    Resource from = d_resource;

    // Build full resource paths
    QString root = resourceManager.getResourcePath(from.rootKey());
    QString path = QDir(root).filePath(from.path());

    qDebug() << QString("VALIDATE %2 [from = \"%1\"]\n").arg(path, (from.resourceType() == RESOURCE_TYPE_FILE ? "File" : "Dir"));

    // TODO: Unimplemented

    return retval;
}

OperationResult ExpectOperation::undo()
{
    return RESULT_OK;
}

OperationResult ExpectOperation::invert()
{
    return RESULT_OK;
}

QString ExpectOperation::errstr()
{
    // TODO: Unimplemented
    return nullptr;
}

QString ExpectOperation::label()
{
    return QString("Validate %1").arg(d_resource.path());
}

/*
 *******************************************************************************
 *                            Function definitions                             *
 *******************************************************************************
*/


static OperationResult copy_file (const QString filename,
                                      const QString directory,
                                      bool force)
{
    OperationResult result = RESULT_OK;

    qDebug() << "copy_file(" << filename << ","
             << directory << ") [force = " << force << "]" << Qt::endl;

    // Check if source file exists
    const QFileInfo source(filename);
    if (false == source.exists()) {
        qDebug() << " --- Source file does not exist!" << Qt::endl;
        return RESULT_BAD_RESOURCE;
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

static OperationResult copy_directory (const QString dirname, const QString directory, bool force)
{
    qDebug() << "copy_directory(" << dirname << "," << directory << ") [force = " << force << "]" << Qt::endl;

    // Check if source directory exists
    const QDir source(dirname);
    if (false == source.exists()) {
        qDebug() << " --- Source directory does not exist!" << Qt::endl;
        return RESULT_BAD_RESOURCE;
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
        OperationResult result;
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

static OperationResult remove_file (const QString filename)
{
    QFileInfo file(filename);

    // Assert whether file exists
    if (false == file.exists()) {
        return RESULT_BAD_RESOURCE;
    }

    // Remove file
    if (false == QFile::remove(filename)) {
        return RESULT_BAD_RESOURCE;
    }

    return RESULT_OK;
}

static OperationResult remove_directory (const QString dirname)
{
    QDir directory(dirname);

    // Assert whether directory exists
    if (false == directory.exists()) {
        return RESULT_BAD_RESOURCE;
    }

    // Remove directory
    if (false == directory.removeRecursively()) {
        return RESULT_BAD_RESOURCE;
    }

    return RESULT_OK;
}


