#ifndef CFGPARSER_H
#define CFGPARSER_H

#include <QtDebug>
#include <QDir>
#include "attributes.h"
#include "cfgelement.h"
#include "operation.h"

namespace SWU {


/*\
 * Parser status
\*/
enum ParseStatus {
    PARSE_OK,
    PARSE_INVALID_ELEMENT,
    PARSE_INVALID_ATTRIBUTE_KEY,
    PARSE_INVALID_ATTRIBUTE_VALUE,

    /* Size */
    PARSE_ENUM_MAX
};


class Parser
{
private:

    // Status of the parser
    ParseStatus d_status;

    // Parse error stack
    QVector<SWU::Token> d_parse_stack;

    // Product and platform
    QString d_product, d_platform;

    // Resource URIs (ordered)
    QVector<QString> d_resource_uris;

    // Path (implicitly on target) at which to backup specified files/directories
    QString d_backup_path;

    // Paths of files and directories to validate
    QVector<QString> d_validate_file_paths, d_validate_directory_paths;

    // Paths (implicitly on target) of files and directories to backup
    QVector<QString> d_backup_file_paths, d_backup_directory_paths;

    // Operations
    QVector<Operation> d_operations;


    bool hasAttributeKeys (std::shared_ptr<CFGElement> element,
                           QVector<attribute_kp_pair> key_pointer_pairs);

    off_t attributeValueInSet (QString raw_attribute,
                               QVector<attribute_value_t> values);


    /*\
     * Returns OK if configuration could be parsed (mutates elements)
     * - elements: Ordered stack of elements
    \*/
    ParseStatus acceptConfiguration(QVector<std::shared_ptr<SWU::CFGElement>>& elements);

    /*\
     * Returns OK if a file could be parsed from the stack (mutates elements)
     * - elements: Ordered stack of elements
     * - value_p: Filepath contained within the element
    \*/
    ParseStatus acceptFile (QVector<std::shared_ptr<SWU::CFGElement>>& elements,
                          QString *value_p);

    /*\
     * Returns OK if a directory could be parsed from the stack (mutates elements)
     * - elements: Ordered stack of elements
     * - value_p: Directory path contained within the element
    \*/
    ParseStatus acceptDirectory (QVector<std::shared_ptr<SWU::CFGElement>>& elements,
                               QString *value_p);

    /*\
     * Returns OK if a backup set could be parsed from the stack (mutates elements)
     * - elements: Ordered stack of elements
    \*/
    ParseStatus acceptBackup (QVector<std::shared_ptr<SWU::CFGElement>>& elements);

    /*\
     * Returns OK if a validate section could be parsed from the stack (mutates elements)
     * - elements: Ordered stack of elements
    \*/
    ParseStatus acceptValidate (QVector<std::shared_ptr<SWU::CFGElement>>& elements);

    /*\
     * Returns OK if a resource URI could be parsed from the stack (mutates elements)
     * - elements: Ordered stack of elements
    \*/
    ParseStatus acceptResourceURI(QVector<std::shared_ptr<SWU::CFGElement>>& elements);

    /*\
     * Returns OK if a operations section could be parsed from the stack (mutates elements)
     * - elements: Ordered stack of elements
    \*/
    ParseStatus acceptOperations(QVector<std::shared_ptr<SWU::CFGElement>>& elements);

    /*\
     * Returns OK if a copy operation could be parsed from the stack (mutates elements)
     * - elements: Ordered stack of elements
    \*/
    ParseStatus acceptCopy(QVector<std::shared_ptr<SWU::CFGElement>>& elements);

    /*\
     * Returns OK if a 'from' element could be parsed from the stack (mutates elements)
     * - elements: Ordered stack of elements
     * - root_p: Pointer at which to store the value assigned to the root attribute
     * - value_p: Pointer at which to store the contents of the 'from' element
    \*/
    ParseStatus acceptFrom(QVector<std::shared_ptr<SWU::CFGElement>>& elements,
                         QString *root_p, QString *value_p);

    /*\
     * Returns OK if a 'to' element could be parsed from the stack (mutates elements)
     * - elements: Ordered stack of elements
     * - root_p: Pointer at which to store the value assigned to the root attribute
     * - value_p: Pointer at which to store the contents of the 'to' element
    \*/
    ParseStatus acceptTo(QVector<std::shared_ptr<SWU::CFGElement>>& elements,
                         QString *root_p, QString *value_p);

    /*\
     * Returns OK if a 'remove' element could be parsed from the stack (mutates elements)
     * - elements: Ordered stack of elements
    \*/
    ParseStatus acceptRemove(QVector<std::shared_ptr<SWU::CFGElement>>& elements);

public:

    /*\
     * Parses element stack into local fields of the instance
     * - elements: Constant ordered stack of elements (not mutated)
    \*/
    Parser(const QVector<std::shared_ptr<SWU::CFGElement>>& elements);

    /*\
     * Returns the parser status
    \*/
    ParseStatus status();

    /*\
     * Returns nullptr if status is OK; else description of error
    \*/
    QString fault();


    /*\
     * Returns product desciptor string
    \*/
    QString product();

    /*\
     * Returns system platform; should be in format QSysInfo::kernelType()
    \*/
    QString platform();

    /*\
     * Returns list of resource URIs
    \*/
    QVector<QString> resource_uris();

    /*\
     * Returns path to backup directory
    \*/
    QString backup_path();

    /*\
     * Returns ordered list of file paths to validate
    \*/
    QVector<QString> validate_file_paths();

    /*\
     * Returns ordered list of directory paths to validate
    \*/
    QVector<QString> validate_directory_paths();

    /*\
     * Returns ordered list of files to backup
    \*/
    QVector<QString> backup_file_paths();

    /*\
     * Returns ordered list of directories to backup
    \*/
    QVector<QString> backup_directory_paths();

    /*\
     * Returns ordered list of operations
    \*/
    QVector<Operation> operations();

};

}

#endif // CFGPARSER_H
