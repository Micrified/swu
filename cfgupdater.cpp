#include "cfgupdater.h"

using namespace SWU;

struct UpdateStatus {
    int okay : 1;

};

Updater::Updater(Parser parser):
    d_status(STATUS_OK),
    d_product(parser.product()),
    d_resource_uris(parser.resource_uris()),
    d_backup_path(parser.backup_path())

{

    // Store product name
    d_product = parser.product();

    // Verify platform
    if (QSysInfo::kernelType().compare(parser.platform()) != 0) {
        qCritical() << "Mismatching platform: Currently running "
                    << QSysInfo::kernelType() << ", but expected "
                    << parser.platform() << " instead";
        d_status = STATUS_BAD_PLATFORM;
        return;
    }

    // Obtain resource URIs
    d_resource_uris = parser.resource_uris();

    //


}
