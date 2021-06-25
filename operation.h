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


enum ResourceType {
    RESOURCE_FILE,
    RESOURCE_DIRECTORY,

    RESOURCE_ENUM_MAX
};

enum OperationType {
    OPERATION_EXPECT,
    OPERATION_COPY,
    OPERATION_MOVE,
    OPERATION_REMOVE,

    OPERATION_ENUM_MAX
};

enum OperationResultType {
    RESULT_OK,
    RESULT_BAD_SOURCE,
    RESULT_BAD_DESTINATION,
    RESULT_BAD_PERMISSIONS
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
    void setExpectOperation(const QString from,
                            ResourceType resourceType);
    void setCopyOperation (const QString from,
                           const QString to,
                           bool force,
                           ResourceType resourceType);
    void setMoveOperation (const QString from,
                           const QString to,
                           bool force,
                           ResourceType resourceType);
    void setRemoveOperation (const QString from,
                             ResourceType resourceType);
    OperationResultType execute ();
    QString errstr ();

};

}

#endif // OPERATION_H
