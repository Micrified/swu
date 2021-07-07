#ifndef RESOURCE_H
#define RESOURCE_H

#include <QString>
#include <QDir>
#include <QMap>

namespace SWU {

/*
 *******************************************************************************
 *                              Type declarations                              *
 *******************************************************************************
*/

/* enumeration of resource keys */
enum resource_root_key_t {
    RESOURCE_KEY_ROOT = 0,
    RESOURCE_KEY_REMOTE,

    /* Size */
    RESOURCE_KEY_ENUM_MAX
};

/* enumeration of resource types */
enum resource_type_t {
    RESOURCE_TYPE_FILE,
    RESOURCE_TYPE_DIRECTORY,

    /* Size */
    RESOURCE_TYPE_ENUM_MAX
};

/*
 *******************************************************************************
 *                             Class declarations                              *
 *******************************************************************************
*/

class Resource {
private:
    resource_root_key_t d_root_key;
    resource_type_t d_resource_type;
    QString d_path;

public:
    Resource (QString path, resource_type_t resource_type,
              resource_root_key_t root_key = RESOURCE_KEY_ROOT);
    QString path();
    resource_type_t resourceType();
    resource_root_key_t rootKey();
};

}


#endif // RESOURCE_H
