#ifndef CFGPARSER_H
#define CFGPARSER_H

#include <QtDebug>
#include <QDir>
#include "attributes.h"
#include "cfgelement.h"

// Array-designation map: Token to lexeme
extern QString g_token_lexeme_map[];

namespace SWU {

enum CFGRootType {
    ROOT_TYPE_REMOTE,
    ROOT_TYPE_TARGET,

    /* Size */
    ROOT_TYPE_ENUM_MAX
};

enum ParseStatus {
    PARSE_OK,
    PARSE_INVALID_ELEMENT,
    PARSE_INVALID_ATTRIBUTE_KEY,
    PARSE_INVALID_ATTRIBUTE_VALUE,

    /* Size */
    PARSE_ENUM_MAX
};

struct Copy {
    CFGRootType from_root, to_root;
    QString from_path, to_path;
};

struct Remove {
    CFGRootType root;
    QString path;
};

#define ERR_CFG_RESULT {PARSE_ERR, CFGData{}}
#define DEFAULT_CFG_RESULT CFGResult {PARSE_ENUM_MAX, CFGData{}}

class Parser
{
private:

    // Status of the parser
    ParseStatus d_status;
    QString d_product, d_platform;
    QVector<QString> d_resource_uris;
    QString d_backup_path;
    QVector<QString> d_validate_file_paths, d_validate_directory_paths;
    QVector<QString> d_backup_file_paths, d_backup_directory_paths;
    QVector<Copy> d_copy_operations;
    QVector<Remove> d_remove_operations;

    ParseStatus acceptConfiguration(QVector<std::shared_ptr<SWU::CFGElement>>& elements);
    ParseStatus acceptFile (QVector<std::shared_ptr<SWU::CFGElement>>& elements,
                          QString *value_p);
    ParseStatus acceptDirectory (QVector<std::shared_ptr<SWU::CFGElement>>& elements,
                               QString *value_p);
    ParseStatus acceptBackup (QVector<std::shared_ptr<SWU::CFGElement>>& elements);
    ParseStatus acceptValidate (QVector<std::shared_ptr<SWU::CFGElement>>& elements);
    ParseStatus acceptResourceURI(QVector<std::shared_ptr<SWU::CFGElement>>& elements);
    ParseStatus acceptOperations(QVector<std::shared_ptr<SWU::CFGElement>>& elements);
    ParseStatus acceptCopy(QVector<std::shared_ptr<SWU::CFGElement>>& elements);
    ParseStatus acceptFrom(QVector<std::shared_ptr<SWU::CFGElement>>& elements,
                         QString *root_p, QString *value_p);
    ParseStatus acceptTo(QVector<std::shared_ptr<SWU::CFGElement>>& elements,
                         QString *root_p, QString *value_p);
    ParseStatus acceptRemove(QVector<std::shared_ptr<SWU::CFGElement>>& elements);
public:
    Parser(const QVector<std::shared_ptr<SWU::CFGElement>>& elements);
    ParseStatus status();
};

}

#endif // CFGPARSER_H
