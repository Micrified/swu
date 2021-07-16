#ifndef FSOPERATION_H
#define FSOPERATION_H

#include <QString>
#include <QDebug>
#include <QThread>
#include <memory>
#include "resource.h"
#include "resource_manager.h"

namespace SWU {

enum OperationResult {
    RESULT_OK,
    RESULT_BAD_RESOURCE,
    RESULT_BAD_DESTINATION,
    RESULT_BAD_PERMISSIONS,

    /* Size */
    RESULT_ENUM_MAX
};

enum OperationLabel {
    LABEL_VALIDATE,
    LABEL_BACKUP,
    LABEL_COPY,
    LABEL_REMOVE,

    /* Size */
    LABEL_ENUM_MAX
};


/* File System operation base */
class FSOperation
{
private:
    QString d_label;
    QString d_errstr;
public:
    FSOperation();
    virtual ~FSOperation();
    virtual OperationResult execute() = 0;
    virtual OperationResult undo () = 0;
    virtual OperationResult invert () = 0;
    virtual QString errstr () = 0;
    virtual QString label ();
};

/* Remove operation */
class RemoveOperation : public FSOperation {
private:
    std::shared_ptr<Resource> d_resource;
public:
    RemoveOperation(std::shared_ptr<Resource> resource);
    OperationResult execute () override;
    OperationResult undo () override;
    OperationResult invert () override;
    QString errstr () override;
    QString label () override;

};

/* Copy operation */
class CopyOperation : public FSOperation {
private:
    Resource d_from_resource, d_to_resource;
public:
    CopyOperation(Resource from, Resource to);
    OperationResult execute () override;
    OperationResult undo () override;
    OperationResult invert () override;
    QString errstr() override;
    QString label() override;
};

/* Check operation */
class ExpectOperation : public FSOperation {
private:
    Resource d_resource;
public:
    ExpectOperation(Resource resource);
    OperationResult execute() override;
    OperationResult undo () override;
    OperationResult invert () override;
    QString errstr() override;
    QString label() override;
};


}

#endif // FSOPERATION_H
