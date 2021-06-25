#include "token.h"

using namespace SWU;

Token::Token(Transition token, const std::weak_ptr<Token> parent):
    d_ready(false),
    d_token(token),
    d_parent(parent)
{

}

void Token::setValue (QString value)
{
    d_value = value;
    d_ready = true;
}

Transition Token::token ()
{
    return d_token;
}

off_t Token::setAttributes (const QXmlAttributes &attributes)
{
    Attributes a = Attributes::get_instance();
    attribute_key_t k = ATTRIBUTE_KEY_ENUM_MAX;
    off_t i = 0;

    // Register/validate attributes
    for (; i < attributes.count(); ++i) {
        const QString qName = attributes.qName(i);
        const QString value = attributes.value(qName);
        if (ATTRIBUTE_KEY_ENUM_MAX == (k = a.key(qName))) {
            break;
        }
        d_attributes[k] = value;
    }

    return i;
}

QMap<SWU::attribute_key_t, QString>& Token::attributes ()
{
    return d_attributes;
}

QString Token::value ()
{
    return d_value;
}

const std::weak_ptr<Token> Token::parent ()
{
    return d_parent;
}

bool Token::ready ()
{
    return d_ready;
}
