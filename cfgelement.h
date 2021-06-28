#ifndef CFGELEMENT_H
#define CFGELEMENT_H

#include "element.h"
#include "attributes.h"
#include "cfgxmlhandler.h"

namespace SWU {

class CFGElement : public Element
{
private:
    attribute_kv_pair d_attribute_index[ATTRIBUTE_KEY_ENUM_MAX];

public:
    CFGElement(std::shared_ptr<Element> &element);
    attribute_kv_pair *attribute_index(attribute_key_t key);
};

}

#endif // CFGELEMENT_H
