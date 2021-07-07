#include "element.h"

using namespace SWU;

// Array-designation map: Token to lexeme
extern QString g_token_lexeme_map[];

Element::Element(Token token, const std::weak_ptr<Element> parent):
    d_ready(false),
    d_token(token),
    d_parent(parent)
{

}

void Element::setValue (QString value)
{
    d_value = value;
    d_ready = true;
}

Token Element::token ()
{
    return d_token;
}

QString Element::setAttributes (const QXmlAttributes &attributes)
{
    for (off_t i = 0; i < attributes.count(); ++i) {
        const QString qName = attributes.qName(i);
        const QString value = attributes.value(qName);
        if (d_attributes.contains(qName)) {
            return qName;
        } else {
            d_attributes[qName] = value;
        }
    }

    return nullptr;
}

QMap<QString, QString>& Element::attributes ()
{
    return d_attributes;
}

QString Element::value ()
{
    return d_value;
}

const std::weak_ptr<Element> Element::parent ()
{
    return d_parent;
}

bool Element::ready ()
{
    return d_ready;
}

QString Element::description ()
{
    QString att, t = g_token_lexeme_map[d_token];
    QMap<QString,QString>::iterator iter;
    for (iter = d_attributes.begin(); iter != d_attributes.end(); ++iter) {
        att += " [" + iter.key() + ":" + iter.value() + "]";
    }
    return t + att + " { " + d_value + " } " + t;
}
