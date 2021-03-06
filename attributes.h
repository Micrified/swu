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
    ATTRIBUTE_KEY_PATH = 0,
    ATTRIBUTE_KEY_ROOT,
    ATTRIBUTE_KEY_PRODUCT,
    ATTRIBUTE_KEY_PLATFORM,

    /* Size */
    ATTRIBUTE_KEY_ENUM_MAX
};

/* Enumeration of all possible attribute values */
enum attribute_value_t {
    ATTRIBUTE_VALUE_REMOTE = 0,
    ATTRIBUTE_VALUE_TARGET,

    /* Size */
    ATTRIBUTE_VALUE_ENUM_MAX
};

/* Attribute key-value pair */
struct attribute_kv_pair {
    attribute_key_t key;
    attribute_value_t val;
    QString lexeme;
};

/* Attribute key-value pair, with a pointer lexeme */
struct attribute_kp_pair {
    attribute_key_t key;
    QString *lexeme_p;
};

/* Symbolic constant: unset/uninitialized attribute type */
#define ATTRIBUTE_UNSET          attribute_kv_pair{ATTRIBUTE_KEY_ENUM_MAX, \
                                     ATTRIBUTE_VALUE_ENUM_MAX, \
                                     nullptr \
                                 }
/* Symbolic constant: returns true if a given attribute is unset */
#define ATTRIBUTE_IS_UNSET(a)    ((a)->key == ATTRIBUTE_KEY_ENUM_MAX ? true : false)


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
