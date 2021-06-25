#include "xmlhandler.h"

using namespace SWU;

/*
 *******************************************************************************
 *                         Static variable definitions                         *
 *******************************************************************************
*/


// Array designation map (used when initialising hashmap)
static QString transition_string_map[TRANSITION_ENUM_MAX] = {
    [TRANSITION_SWUPDATE_OPEN]         = QString("<swupdate>"),
    [TRANSITION_SWUPDATE_CLOSE]        = QString("</swupdate>"),
    [TRANSITION_MEDIA_OPEN]            = QString("<media>"),
    [TRANSITION_MEDIA_CLOSE]           = QString("</media>"),
    [TRANSITION_MEDIA_PATH_OPEN]       = QString("<media-path>"),
    [TRANSITION_MEDIA_PATH_CLOSE]      = QString("</media-path>"),
    [TRANSITION_FILE_MATCH_OPEN]       = QString("<file-match>"),
    [TRANSITION_FILE_MATCH_CLOSE]      = QString("</file-match>"),
    [TRANSITION_UPDATE_VALIDATE_OPEN]  = QString("<update-validate>"),
    [TRANSITION_UPDATE_VALIDATE_CLOSE] = QString("</update-validate>"),
    [TRANSITION_EXPECT_OPEN]           = QString("<expect>"),
    [TRANSITION_EXPECT_CLOSE]          = QString("</expect>"),
    [TRANSITION_UPDATE_OPEN]           = QString("<update>"),
    [TRANSITION_UPDATE_CLOSE]          = QString("</update>"),
    [TRANSITION_COPY_OPEN]             = QString("<copy>"),
    [TRANSITION_COPY_CLOSE]            = QString("</copy>"),
    [TRANSITION_DIRECTORY_OPEN]        = QString("<directory>"),
    [TRANSITION_DIRECTORY_CLOSE]       = QString("</directory>"),
    [TRANSITION_TO_DIRECTORY_OPEN]     = QString("<to-directory>"),
    [TRANSITION_TO_DIRECTORY_CLOSE]    = QString("</to-directory>"),
    [TRANSITION_FILE_OPEN]             = QString("<file>"),
    [TRANSITION_FILE_CLOSE]            = QString("</file>"),
    [TRANSITION_REMOVE_OPEN]           = QString("<remove>"),
    [TRANSITION_REMOVE_CLOSE]          = QString("</remove>")
};


/*
 *******************************************************************************
 *                        Class definition: XMLHandler                         *
 *******************************************************************************
*/


XMLHandler::XMLHandler ():
    QXmlDefaultHandler(),
    d_parsed(false)
{

}

bool XMLHandler::startDocument ()
{
    qInfo() << "Parse started ... ";
    return true;
}

bool XMLHandler::endDocument()
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

    // Print stack
    for (off_t i = 0; i < d_token_stack.size(); ++i) {
        QString prefix = "";
        std::shared_ptr<Token> token_p = d_token_stack.at(i);
        if (!token_p->parent().expired()) {
            std::weak_ptr<Token> parent = token_p->parent();
            if (auto p = parent.lock()) {
                const char *parent_name = g_map_transition_str[p->token()];
                prefix.append(QString(parent_name) + "::");
            }
        }
        qInfo() << prefix << "[" << g_map_transition_str[token_p->token()] << "]: value=" << token_p->value() << ", ready=" << token_p->ready();
    }
    qInfo() << "Ok";

    // Mark parsed
    d_parsed = true;

    return d_parsed;
}

bool XMLHandler::startElement (const QString &nsURI, const QString &localname, const QString &name, const QXmlAttributes &att)
{
    Q_UNUSED(nsURI);
    Q_UNUSED(localname);
    Q_UNUSED(att);

    Transition token;

    // Apply the transition to the state machine
    if (d_state_machine.input("<" + name +">", &token) == STATUS_FAULT) {
        qCritical() << "The opening tag " << name << " is either unrecognized, or unexpected in the scope";
        return false;
    }

    // Update the object 'update' with the lexeme contents.
    // Trick: All tokens divisible by two are "closing" tags. So we don't act on them
    if (0 == (token % 2)) {

        // Obtain the current scope parent
        std::weak_ptr<Token> parent = std::weak_ptr<Token>();
        if (d_scope_stack.size() > 0) {
            parent = d_scope_stack.at(d_scope_stack.size() - 1);
        }

        // Create a new token
        std::shared_ptr<Token> token_p = std::make_shared<Token>(Token(token, parent));

        // Set attributes and check for offending token keys
        off_t i;
        if (att.length() != (i = token_p->setAttributes(att))) {
            QString invalid_key = att.qName(i);
            qCritical() << "Attribute key \"" << invalid_key << "\" is invalid for tag \"" << name << "\"";
        }

        // Push to collection stack
        d_token_stack.append(token_p);

        // Push to scope stack as latest open scope
        d_scope_stack.append(token_p);
    }

    // Mark to buffer contents
    this->startCDATA();
    return true;
}

bool XMLHandler::endElement (const QString &nsURI, const QString &localname, const QString &name)
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

bool XMLHandler::characters (const QString &characters)
{
    std::weak_ptr<Token> token;

    // Return if nothing to do.
    if (d_token_stack.size() <= 0) {
        return true;
    }

    // Obtain the latest element on the stack
    token = d_token_stack.last();
    if (auto t = token.lock()) {

        // Update the value of the token
        if (t->ready() == false) {
            t->setValue(characters);
        }

    }

    return true;
}

bool XMLHandler::fatalError (const QXmlParseException &e)
{
    qCritical() << e.lineNumber() << ":"
                << e.columnNumber() << " - "
                << e.message();
    return false;
}

bool XMLHandler::parsed ()
{
    return d_parsed;
}

const QVector<std::shared_ptr<Token>>& XMLHandler::tokenStack()
{
    return d_token_stack;
}
