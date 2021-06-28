#ifndef CFGPARSER_H
#define CFGPARSER_H

#include "element.h"
#include "attributes.h"

namespace SWU {

enum CFGRootType {
    ROOT_TYPE_REMOTE,
    ROOT_TYPE_TARGET,

    /* Size */
    ROOT_TYPE_ENUM_MAX
};

union CFGData {
    struct configuration {
        QString product, platform;
    };
    struct resource_uri {
        QString uri;
    };
    struct backup {
        QString backup_path;
    };
    struct file {
        QString file_path;
    };
    struct directory {
        QString directory_path;
    };
    struct copy {
        QString from_path, to_path;
    };
    struct remove {
        QString remove_path;
    };
};

enum ParseStatus {
    PARSE_OK,

    /* Size */
    PARSE_ENUM_MAX
};


struct CFGResult {
    ParseStatus status;
    CFGData data;
};

#define DEFAULT_CFG_RESULT CFGResult {PARSE_ENUM_MAX, CFGData{}}

class Parser
{
private:
    ParseStatus d_status;
    QString d_product, d_platform;
    QString d_resource_uri;
    QString d_backup_path;
    QVector<QString> d_backup_file_paths, d_backup_directory_paths;
    QVector<QPair<QString,QString>> d_copy_operations;
    QVector<QString> d_remove_operations;

    CFGResult acceptConfiguration(QVector<std::shared_ptr<SWU::Element>>& elements);
    CFGResult acceptFile (QVector<std::shared_ptr<SWU::Element>>& elements);
    CFGResult acceptDirectory (QVector<std::shared_ptr<SWU::Element>>& elements);
    CFGResult acceptBackup (QVector<std::shared_ptr<SWU::Element>>& elements);
    CFGResult acceptValidate (QVector<std::shared_ptr<SWU::Element>>& elements);
    CFGResult acceptResourceURI(QVector<std::shared_ptr<SWU::Element>>& elements);
    CFGResult acceptOperations(QVector<std::shared_ptr<SWU::Element>>& elements);
    CFGResult acceptCopy(QVector<std::shared_ptr<SWU::Element>>& elements);
    CFGResult acceptFrom(QVector<std::shared_ptr<SWU::Element>>& elements);
    CFGResult acceptTo(QVector<std::shared_ptr<SWU::Element>>& elements);
    CFGResult acceptRemove(QVector<std::shared_ptr<SWU::Element>>& elements);
public:
    Parser(const QVector<std::shared_ptr<SWU::Element>>& elements);
};

}

#endif // CFGPARSER_H
