#ifndef CFGPARSER_H
#define CFGPARSER_H

#include <QtDebug>
#include <QDir>
#include "attributes.h"
#include "cfgelement.h"

namespace SWU {


/*\
 * Configuration root options
 * - remote: Root of the resource uri
 * - target: Root of the local system
\*/
enum CFGRootType {
    ROOT_TYPE_REMOTE,
    ROOT_TYPE_TARGET,

    /* Size */
    ROOT_TYPE_ENUM_MAX
};


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


/*\
 * Structure describing a copy
 * cp <from_root/from_path> <to_root/to_path>
\*/
struct Copy {
    CFGRootType from_root, to_root;
    QString from_path, to_path;
};


/*\
 * Structure describing a remove
 * rm <root/path>
\*/
struct Remove {
    CFGRootType root;
    QString path;
};

class Parser
{
private:

    // Status of the parser
    ParseStatus d_status;

    // Parse error stack
    QVector<QString> d_parse_stack;

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

    // Copy operations (ordered)
    QVector<Copy> d_copy_operations;

    // Remove operations (ordered)
    QVector<Remove> d_remove_operations;


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
};

}

#endif // CFGPARSER_H
