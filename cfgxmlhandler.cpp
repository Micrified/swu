#include "cfgxmlhandler.h"

using namespace SWU;


ConfigXMLHandler::ConfigXMLHandler ():
    QXmlDefaultHandler(),
    d_parsed(false)
{

}

bool ConfigXMLHandler::startDocument ()
{
    qInfo() << "Parse started ... ";
    return true;
}

bool ConfigXMLHandler::endDocument()
{

    // Check: State-machine is in the end state
    if (d_state_machine.status() != STATUS_COMPLETE) {
        qCritical() << "Parse ended in incomplete state: " << d_state_machine.status();
        return false;
    }

    // Check: Scope stack is empty (should be always true if the state-machine is well made)
    if (d_scope_stack.size() > 0) {
        qCritical() << "Parse ended with an open scope!";
        return false;
    }

    // [TODO: Remove] Print stack
//    for (off_t i = 0; i < d_element_stack.size(); ++i) {
//        QString prefix = "";
//        std::shared_ptr<Element> element_p = d_element_stack.at(i);
//        if (!element_p->parent().expired()) {
//            std::weak_ptr<Element> parent = element_p->parent();
//            if (auto p = parent.lock()) {
//                QString parent_lexeme = g_token_lexeme_map[p->token()];
//                prefix.append(parent_lexeme + "::");
//            }
//        }
//        qCritical() << prefix << "[" << g_token_lexeme_map[element_p->token()] << "]: value="
//                << element_p->value() << ", ready=" << element_p->ready();
//    }
    qInfo() << "Ok";

    // Mark parsed
    d_parsed = true;

    return d_parsed;
}

bool ConfigXMLHandler::startElement (const QString &nsURI, const QString &localname,
                                     const QString &name, const QXmlAttributes &att)
{
    Q_UNUSED(nsURI);
    Q_UNUSED(localname);
    Q_UNUSED(att);

    Token token;

    // Feed the token to the state machine
    if (d_state_machine.input("<" + name + ">", &token) == STATUS_FAULT) {
        qCritical() << "The opening tag " << name << " is either unrecognized, or unexpected in the scope";
        return false;
    }

    // Update the object 'update' with the lexeme contents.
    // Trick: All tokens divisible by two are "closing" tags. So we don't act on them
    if (0 == (token % 2)) {

        // Obtain the current scope parent
        std::weak_ptr<Element> parent = std::weak_ptr<Element>();
        if (d_scope_stack.size() > 0) {
            parent = d_scope_stack.at(d_scope_stack.size() - 1);
        }

        // Create a new element
        std::shared_ptr<Element> element_p = std::make_shared<Element>(Element(token, parent));

        // Set attributes and check for offending token keys
        if (element_p->setAttributes(att) != nullptr) {
            QString duplicate_key = element_p->setAttributes(att);
            qCritical() << "Attribute key \"" << duplicate_key
                        << "\" duplicated in element \"" << name << "\"";
        }

        // Push to collection stack
        d_element_stack.append(element_p);

        // Push to scope stack as latest open scope
        d_scope_stack.append(element_p);
    }

    // Mark to buffer contents
    this->startCDATA();
    return true;
}

bool ConfigXMLHandler::endElement (const QString &nsURI, const QString &localname, const QString &name)
{
    Q_UNUSED(nsURI);
    Q_UNUSED(localname);

    // Apply the transition to the state machine
    if (d_state_machine.input("</"+ name + ">") == STATUS_FAULT) {
        qCritical() << "The closing tag " << name << " is recognized, but not expected here" <<
                       " (check nested level)";
        return false;
    }

    // Pop the last element off the scope stack
    d_scope_stack.pop_back();

    // Mark end of buffering zone
    this->endCDATA();
    return true;
}

bool ConfigXMLHandler::characters (const QString &characters)
{
    std::weak_ptr<Element> element;

    // Return if nothing to do.
    if (d_element_stack.size() <= 0) {
        return true;
    }

    // Obtain the latest element on the stack
    element = d_element_stack.last();
    if (auto e = element.lock()) {

        // Update the value of the token
        if (e->ready() == false) {
            e->setValue(characters);
        }

    }

    return true;
}

bool ConfigXMLHandler::fatalError (const QXmlParseException &e)
{
    qCritical() << e.lineNumber() << ":"
                << e.columnNumber() << " - "
                << e.message();
    return false;
}

bool ConfigXMLHandler::parsed ()
{
    return d_parsed;
}

const QVector<std::shared_ptr<Element>>& ConfigXMLHandler::elementStack()
{
    return d_element_stack;
}
