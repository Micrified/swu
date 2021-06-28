#include "cfgparser.h"
using namespace SWU;

std::shared_ptr<Element> next(QVector<std::shared_ptr<Element>> &elements)
{
    std::shared_ptr<Element> e = nullptr;
    if (elements.count() > 0) {
        e = elements.first();
        elements.pop_front();
    }
    return e;
}

bool requireExactAttributes (std::shared_ptr<Element> element,
                             QVector<attribute_kp_pair> key_pointer_pairs)
{
    for (auto )
}

Parser::Parser(const QVector<std::shared_ptr<SWU::Element>>& elements)
{
    // Clone the elements
    QVector<std::shared_ptr<Element>> elements_stack_copy(elements);

    // Accept configuration
    CFGResult result = acceptConfiguration(elements_stack_copy);
    d_status = result.status;
}

CFGResult Parser::acceptConfiguration(QVector<std::shared_ptr<SWU::Element>>& elements)
{
    CFGResult r = DEFAULT_CFG_RESULT;
    std::shared_ptr<Element> config = nullptr;

    // First stack element is the configuration
    if ((config = next(elements)) == nullptr) {
        return r;
    }

    // Extract expected attributes

    std::shared_ptr<Element> configuration = elements.first();
    elements.removeFirst();
    elements.

    return r;
}
CFGResult Parser::acceptFile (QVector<std::shared_ptr<SWU::Element>>& elements)
{
    CFGResult r = DEFAULT_CFG_RESULT;

    return r;
}
CFGResult Parser::acceptDirectory (QVector<std::shared_ptr<SWU::Element>>& elements)
{
    CFGResult r = DEFAULT_CFG_RESULT;
    return r;
}
CFGResult Parser::acceptBackup (QVector<std::shared_ptr<SWU::Element>>& elements)
{
    CFGResult r = DEFAULT_CFG_RESULT;
    return r;
}
CFGResult Parser::acceptValidate (QVector<std::shared_ptr<SWU::Element>>& elements)
{
    CFGResult r = DEFAULT_CFG_RESULT;
    return r;
}
CFGResult Parser::acceptResourceURI(QVector<std::shared_ptr<SWU::Element>>& elements)
{
    CFGResult r = DEFAULT_CFG_RESULT;
    return r;
}
CFGResult Parser::acceptOperations(QVector<std::shared_ptr<SWU::Element>>& elements)
{
    CFGResult r = DEFAULT_CFG_RESULT;
    return r;
}
CFGResult Parser::acceptCopy(QVector<std::shared_ptr<SWU::Element>>& elements)
{
    CFGResult r = DEFAULT_CFG_RESULT;
    return r;
}
CFGResult Parser::acceptFrom(QVector<std::shared_ptr<SWU::Element>>& elements)
{
    CFGResult r = DEFAULT_CFG_RESULT;
    return r;
}
CFGResult Parser::acceptTo(QVector<std::shared_ptr<SWU::Element>>& elements)
{
    CFGResult r = DEFAULT_CFG_RESULT;
    return r;
}
CFGResult Parser::acceptRemove(QVector<std::shared_ptr<SWU::Element>>& elements)
{
    CFGResult r = DEFAULT_CFG_RESULT;
    return r;
}
