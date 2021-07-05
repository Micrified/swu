#ifndef CFGUPDATER_H
#define CFGUPDATER_H

#include <QVector>
#include <QSysInfo>
#include "cfgparser.h"
#include "operation.h"

namespace SWU {

// Updated status
enum UpdateStatus {
    STATUS_OK,
    STATUS_BAD_PLATFORM,
};

class Updater
{
private:

    // Status
    UpdateStatus d_status;

    // Product name
    QString d_product;

    // Resource URIs
    QVector <QString> d_resource_uris;

    // Backup path
    QString d_backup_path;

    // Operation stack
    QVector<SWU::Operation> d_operation_stack;

public:
    Updater(SWU::Parser parser);

};

}
#endif // CFGUPDATER_H
