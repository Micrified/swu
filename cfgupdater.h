#ifndef CFGUPDATER_H
#define CFGUPDATER_H

#include <QVector>
#include <QSysInfo>
#include "cfgparser.h"
#include "fsoperation.h"
#include "resource.h"
#include "resource_manager.h"

namespace SWU {

// Updated status
enum UpdateStatus {
    STATUS_OK,
    STATUS_BAD_PLATFORM,
    STATUS_RESOURCE_NOT_FOUND,
    STATUS_BAD_RESULT,
    STATUS_BAD_PRECONDITION,
    STATUS_BAD_UNDO,

    /* Size */
    STATUS_ENUM_MAX
};

// Forward declaration of updater class
class Updater;


// Updater delegate
class UpdateDelegate
{
public:
    UpdateDelegate();
    virtual ~UpdateDelegate();

    virtual UpdateStatus on_init (SWU::Updater &updater) = 0;

    virtual UpdateStatus on_configure_resource_manager (
            ResourceManager &resourceManager,
            QVector<QString> &resource_uris) = 0;

    virtual UpdateStatus on_pre_validate (std::shared_ptr<ExpectOperation> op) = 0;

    virtual UpdateStatus on_pre_backup (std::shared_ptr<CopyOperation> op) = 0;

    virtual UpdateStatus on_pre_update (std::shared_ptr<FSOperation> op) = 0;

    virtual UpdateStatus on_exit (SWU::Updater &updater,
                                  UpdateStatus status,
                                  std::shared_ptr<FSOperation> op = nullptr,
                                  OperationResult op_result = RESULT_ENUM_MAX) = 0;
};

class Updater
{
private:

    // Status
    UpdateStatus d_status;

    // The update delegate
    UpdateDelegate &d_update_delegate;

    // Product name
    QString d_product;

    // Platform
    QString d_platform;

    // Resource URIs
    QVector <QString> d_resource_uris;

    // Backup path
    QString d_backup_path;

    // Operation stack pointers
    off_t d_validate_sp, d_backup_sp, d_update_sp;

    // Operation stack
    QVector<std::shared_ptr<SWU::FSOperation>> d_validate_operations;
    QVector<std::shared_ptr<SWU::FSOperation>> d_backup_operations;
    QVector<std::shared_ptr<SWU::FSOperation>> d_update_operations;

public:
    Updater(SWU::Parser &parser, SWU::UpdateDelegate &delegate);
    UpdateStatus execute ();
    UpdateStatus undo ();
    off_t validate_sp ();
    off_t backup_sp ();
    off_t update_sp ();
    const QVector<std::shared_ptr<SWU::FSOperation>> validate_operations ();
    const QVector<std::shared_ptr<SWU::FSOperation>> backup_operations ();
    const QVector<std::shared_ptr<SWU::FSOperation>> update_operations ();
    off_t operationCount();
    QString product();
    QString platform();
};

}
#endif // CFGUPDATER_H
