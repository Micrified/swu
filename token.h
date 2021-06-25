#ifndef TOKEN_H
#define TOKEN_H

#include <QXmlAttributes>
#include <QMap>
#include "xmlstatemachine.h"
#include "attributes.h"

namespace SWU {

/*
 *******************************************************************************
 *                             Class declarations                              *
 *******************************************************************************
*/



class Token {
private:
    bool d_ready;
    SWU::Transition d_token;
    const std::weak_ptr<Token> d_parent;
    QString d_value;
    QMap<SWU::attribute_key_t, QString> d_attributes;
public:
    Token (SWU::Transition token,
           const std::weak_ptr<Token> parent = std::weak_ptr<Token>());
    ~Token() = default;
    void setValue(QString value);

    /* Returns attributes.length() on success; valid index of offending attribute key on error */
    off_t setAttributes (const QXmlAttributes &attributes);

    SWU::Transition token();
    QMap<SWU::attribute_key_t, QString> &attributes();
    QString value ();
    const std::weak_ptr<Token> parent();
    bool ready();
};

}

#endif // TOKEN_H
