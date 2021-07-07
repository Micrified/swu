#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <QDir>
#include <QMap>
#include "resource.h"

namespace SWU {

class ResourceManager
{
private:
    QMap<resource_root_key_t, QString> d_resource_map;
public:
    ResourceManager();
    static ResourceManager& get_instance();
    QString getResourcePath(resource_root_key_t key);
    void setResourcePath(resource_root_key_t key, QString path);
};

}

#endif // RESOURCE_MANAGER_H
