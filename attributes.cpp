#include "attributes.h"

using namespace SWU;


/*
 *******************************************************************************
 *                         Global variable definitions                         *
 *******************************************************************************
*/


static const char *g_attr_key_str_map[ATTRIBUTE_KEY_ENUM_MAX] = {
    [ATTRIBUTE_KEY_PATH]     = "path",
    [ATTRIBUTE_KEY_ROOT]     = "root",
    [ATTRIBUTE_KEY_PRODUCT]  = "product",
    [ATTRIBUTE_KEY_PLATFORM] = "platform",
};

static const char *g_attr_val_str_map[ATTRIBUTE_VALUE_ENUM_MAX] = {
    [ATTRIBUTE_VALUE_REMOTE]    = "Remote",
    [ATTRIBUTE_VALUE_TARGET]    = "Target"
};


/*
 *******************************************************************************
 *                             Singleton instance                              *
 *******************************************************************************
*/


Attributes& Attributes::get_instance()
{
    static Attributes a;
    return a;
}


/*
 *******************************************************************************
 *                              Class definition                               *
 *******************************************************************************
*/


Attributes::Attributes()
{

    /* Init hashmaps */
    for (off_t i = 0; i < ATTRIBUTE_KEY_ENUM_MAX; ++i) {
        attribute_key_t katt = static_cast<attribute_key_t>(i);
        d_key_map[g_attr_key_str_map[i]] = katt;
    }
    for (off_t i = 0; i < ATTRIBUTE_VALUE_ENUM_MAX; ++i) {
        attribute_value_t vatt = static_cast<attribute_value_t>(i);
        d_value_map[g_attr_val_str_map[i]] = vatt;
    }
}

attribute_key_t Attributes::key(QString k)
{
    attribute_key_t retval = ATTRIBUTE_KEY_ENUM_MAX;
    if (d_key_map.contains(k)) {
        retval = d_key_map[k];
    }
    return retval;
}

attribute_value_t Attributes::value(QString v)
{
    attribute_value_t retval = ATTRIBUTE_VALUE_ENUM_MAX;
    if (d_value_map.contains(v)) {
        retval = d_value_map[v];
    }
    return retval;
}

QString Attributes::key_to_str (attribute_key_t key)
{
    if (key == ATTRIBUTE_KEY_ENUM_MAX) {
        return nullptr;
    } else {
        return QString::fromUtf8(g_attr_key_str_map[key]);
    }
}

QString Attributes::value_to_str (attribute_value_t value)
{
    if (value == ATTRIBUTE_VALUE_ENUM_MAX) {
        return nullptr;
    } else {
        return QString::fromUtf8(g_attr_val_str_map[value]);
    }
}
