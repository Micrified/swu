#ifndef XMLHANDLER_H
#define XMLHANDLER_H

/*\
 * The XMLHandler class implements the QXmlDefaultHandler interface.
 * This means it responds to events during the parsing of raw XML, but provides
 * no additional interpretation.
 *
 * The XMLHandler class therefore adds extra functions to the interface:
 *
 * 1. It validates the format of the XML (via a state machine: xmlstatemachine)
 * 2. It builds a flat token tree
 *
 * The token-tree constructed is called flat because it is a single array in
 * which each token contains a weak reference to their parent (container) token.
 *
\*/


#include <QtXml>
#include "xmlstatemachine.h"
#include "token.h"

/*
 *******************************************************************************
 *                             External variables                              *
 *******************************************************************************
*/


// Maps tokens (transitions) to a human readable (string) form
extern const char *g_map_transition_str[];

/*
 *******************************************************************************
 *                         Class declaration: Handler                          *
 *******************************************************************************
*/

class XMLHandler : public QXmlDefaultHandler {
private:

    // Custom state machine for parsing the file
    SWU::Machine d_state_machine;

    // Token collection stack
    QVector<std::shared_ptr<SWU::Token>> d_token_stack;

    // Token scope stack
    QVector<std::weak_ptr<SWU::Token>> d_scope_stack;

    // Parse bool
    bool d_parsed;

protected:

public:

    /* Constructor/Destructor */
    XMLHandler ();
    ~XMLHandler () = default;

    /* Overridden methods */
    bool startDocument () override;
    bool endDocument () override;
    bool startElement (const QString &nsURI,
                       const QString &localname,
                       const QString &name,
                       const QXmlAttributes &att) override;
    bool endElement (const QString &nsURI,
                     const QString &localname,
                     const QString &name) override;
    bool characters (const QString &characters) override;
    bool fatalError (const QXmlParseException &e) override;

    /* New methods */
    bool parsed ();
    const QVector<std::shared_ptr<SWU::Token>>& tokenStack();
};

#endif // XMLHANDLER_H
