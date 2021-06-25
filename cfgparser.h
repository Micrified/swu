#ifndef CFGPARSER_H
#define CFGPARSER_H

#include "element.h"
#include "attributes.h"

namespace SWU {

class Parser
{
public:
    Parser();

    static bool acceptConfiguration(std::shared_ptr<Element> element);
    static bool acceptFile (std::shared_ptr<Element> element);
    static bool acceptDirectory (std::shared_ptr<Element> element);
    static bool acceptBackup (std::shared_ptr<Element> element);
    static bool acceptValidate (std::shared_ptr<Element> element);
    static bool acceptResourceURI(std::shared_ptr<Element> element);
    static bool acceptOperations(std::shared_ptr<Element> element);
    static bool acceptCopy(std::shared_ptr<Element> element);
    static bool acceptFrom(std::shared_ptr<Element> element);
    static bool acceptTo(std::shared_ptr<Element> element);
    static bool acceptRemove(std::shared_ptr<Element> element);
};

}

#endif // CFGPARSER_H
