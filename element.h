#ifndef ELEMENT_H
#define ELEMENT_H

#include <QXmlAttributes>
#include <QMap>
#include <memory>
#include "cfgstatemachine.h"

namespace SWU {

/*
 *******************************************************************************
 *                             Class declarations                              *
 *******************************************************************************
*/



class Element {
private:

    // If successfully parsed
    bool d_ready;

protected:

    // Parser token associated with element
    SWU::Token d_token;

    // Element in outer scope
    const std::weak_ptr<Element> d_parent;

    // Enclosed string value
    QString d_value;

    // Key-value attributes
    QMap<QString, QString> d_attributes;

public:
    Element(SWU::Token token,
           const std::weak_ptr<Element> parent = std::weak_ptr<Element>());
    ~Element() = default;

    // Set string value of element (that which is typically enclosed between tags)
    void setValue(QString value);

    // Sets key-value attribute map. Returns nullptr if okay; else first colliding key
    QString setAttributes (const QXmlAttributes &attributes);

    // Returns associated token
    SWU::Token token();

    // Returns key-value attribute map
    QMap<QString, QString> &attributes();

    // Returns enclosed string value
    QString value ();

    // Returns parent element
    const std::weak_ptr<Element> parent ();

    // Returns parse state
    bool ready();

    // Returns string descriptor
    QString description();
};

}

#endif // ELEMENT_H
