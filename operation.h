#ifndef OPERATION_H
#define OPERATION_H

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QDebug>

namespace SWU {


/*
 *******************************************************************************
 *                              Type definitions                               *
 *******************************************************************************
*/


enum RootType {
    ROOT_TYPE_REMOTE,
    ROOT_TYPE_TARGET,

    /* Size */
    ROOT_TYPE_ENUM_MAX
};

enum ResourceType {
    RESOURCE_FILE,
    RESOURCE_DIRECTORY,

    /* Size */
    RESOURCE_ENUM_MAX
};

enum OperationType {
    OPERATION_EXPECT,
    OPERATION_COPY,
    OPERATION_MOVE,
    OPERATION_REMOVE,

    /* Size */
    OPERATION_ENUM_MAX
};

enum OperationResultType {
    RESULT_OK,
    RESULT_BAD_SOURCE,
    RESULT_BAD_DESTINATION,
    RESULT_BAD_PERMISSIONS,

    /* Size */
    RESULT_ENUM_MAX
};

struct Resource {
    RootType root;
    QString path;
};


/*
 *******************************************************************************
 *                             Class declarations                              *
 *******************************************************************************
*/




class Operation
{

private:
    ResourceType d_resource_type;
    OperationType d_operation_type;
    QString d_from, d_to, d_errstr;
    bool d_force;

public:
    explicit Operation ();
    ~Operation() = default;
    void setExpectOperation(const Resource resource,
                            ResourceType resourceType);
    void setCopyOperation (const Resource from,
                           const Resource to,
                           bool force,
                           ResourceType resourceType);
    void setMoveOperation (const Resource from,
                           const Resource to,
                           bool force,
                           ResourceType resourceType);
    void setRemoveOperation (const Resource from,
                             ResourceType resourceType);
    OperationResultType execute ();
    QString errstr ();
};

}

#endif // OPERATION_H
