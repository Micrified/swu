#include "cfgelement.h"

using namespace SWU;

CFGElement::CFGElement(std::shared_ptr<Element> &element):
    Element(element->token(), element->parent())
{
    Attributes a = Attributes::get_instance();

    // Initialize the attribute index
#pragma unroll(ATTRIBUTE_KEY_ENUM_MAX)
    for (off_t i = 0; i < ATTRIBUTE_KEY_ENUM_MAX; ++i) {
        d_attribute_index[i] = ATTRIBUTE_UNSET;
    }

    // Extract attributes
    QMap<QString,QString> attributes = element->attributes();

    // Convert to attribute built-in types
    QMap<QString,QString>::iterator i;
    for (i = attributes.begin(); i != attributes.end(); ++i) {
        attribute_key_t key_type = a.key(i.key());
        attribute_value_t value_type = a.value(i.value());

        // Check key validity
        if (ATTRIBUTE_KEY_ENUM_MAX == key_type) {
            // TODO: set error state for key here
            return;
        }

        // Construct key-value type
        attribute_kv_pair kv = {
            key_type,
            value_type,
            i.value()
        };

        // Store
        d_attribute_index[key_type] = kv;
    }
}

attribute_kv_pair *CFGElement::attribute_index(attribute_key_t key)
{
    return d_attribute_index + key;
}
