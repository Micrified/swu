#include "resource.h"

using namespace SWU;



Resource::Resource (QString path, resource_type_t resource_type, resource_root_key_t root_key):
    d_path(path),
    d_resource_type(resource_type),
    d_root_key(root_key)
{

}

QString Resource::path()
{
    return d_path;
}

resource_type_t Resource::resourceType()
{
    return d_resource_type;
}

resource_root_key_t Resource::rootKey()
{
    return d_root_key;
}
