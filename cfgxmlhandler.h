#ifndef CFGXMLHANDLER_H
#define CFGXMLHANDLER_H

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
#include "cfgstatemachine.h"
#include "element.h"

/*
 *******************************************************************************
 *                             External variables                              *
 *******************************************************************************
*/


// Array-designation map: Token to lexeme
extern QString g_token_lexeme_map[];

/*
 *******************************************************************************
 *                         Class declaration: Handler                          *
 *******************************************************************************
*/

class ConfigXMLHandler : public QXmlDefaultHandler {
private:

    // Custom state machine for parsing the file
    SWU::Machine d_state_machine;

    // Element collection stack
    QVector<std::shared_ptr<SWU::Element>> d_element_stack;

    // Element scope stack
    QVector<std::weak_ptr<SWU::Element>> d_scope_stack;

    // Parse bool
    bool d_parsed;

protected:

public:

    /* Constructor/Destructor */
    ConfigXMLHandler ();
    ~ConfigXMLHandler () = default;

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
    const QVector<std::shared_ptr<SWU::Element>>& elementStack();
};

#endif // CFGXMLHANDLER_H
