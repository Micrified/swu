#include "resource_manager.h"

using namespace SWU;

/*
*******************************************************************************
*                             Singleton instance                              *
*******************************************************************************
*/


ResourceManager& ResourceManager::get_instance()
{
   static ResourceManager r;
   return r;
}


/*
*******************************************************************************
*                              Class definition                               *
*******************************************************************************
*/


ResourceManager::ResourceManager()
{

   /* Init hashmaps */
   d_resource_map[RESOURCE_KEY_ROOT] = QDir::rootPath();
}

QString ResourceManager::getResourcePath(resource_root_key_t key)
{
   QString retval = nullptr;
   if (d_resource_map.contains(key)) {
       retval = d_resource_map[key];
   }
   return retval;
}

void ResourceManager::setResourcePath(resource_root_key_t key, QString path)
{
   d_resource_map[key] = path;
}
