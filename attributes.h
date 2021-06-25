#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H
#include <QString>
#include <QMap>

namespace SWU {

/*
 *******************************************************************************
 *                              Type declarations                              *
 *******************************************************************************
*/

/* Enumeration of all possible attribute keys */
enum attribute_key_t {
    ATTRIBUTE_KEY_PRODUCT = 0,
    ATTRIBUTE_KEY_VERSION,
    ATTRIBUTE_KEY_BUILD_ARCH,
    ATTRIBUTE_KEY_OS,
    ATTRIBUTE_KEY_TYPE,
    ATTRIBUTE_KEY_ROOT,
    ATTRIBUTE_KEY_MD5,

    ATTRIBUTE_KEY_ENUM_MAX
};

/* Enumeration of all possible attribute values */
enum attribute_value_t {
    ATTRIBUTE_VALUE_FILE = 0,
    ATTRIBUTE_VALUE_DIRECTORY,
    ATTRIBUTE_VALUE_MEDIA,
    ATTRIBUTE_VALUE_SYSTEM,

    ATTRIBUTE_VALUE_ENUM_MAX
};

/* Attribute key-value pair */
struct attribute_kv_pair {
    attribute_key_t key;
    attribute_value_t val;
};


/*
 *******************************************************************************
 *                             Class declarations                              *
 *******************************************************************************
*/

class Attributes
{
private:
    QMap<QString, attribute_key_t> d_key_map;
    QMap<QString, attribute_value_t> d_value_map;
public:
    Attributes();
    static Attributes& get_instance();
    attribute_key_t key(QString);
    attribute_value_t value(QString);
    QString key_to_str (attribute_key_t key);
    QString value_to_str (attribute_value_t value);
};

}

#endif // ATTRIBUTES_H
