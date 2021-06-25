#ifndef UPDATE_H
#define UPDATE_H

#include <QtGlobal>
#include <QString>
#include "token.h"
#include "operation.h"
#include "attributes.h"


// Maps tokens (transitions) to a human readable (string) form
extern const char *g_map_transition_str[];

namespace SWU {

struct UpdateFlags {
    unsigned int f_named            : 1;
    unsigned int f_versioned        : 1;
    unsigned int f_kernel           : 1;
    unsigned int f_build_arch       : 1;
    unsigned int f_file_match       : 1;
    unsigned int f_media_path       : 1;
    unsigned int f_valid_expect_ops : 1;
    unsigned int f_valid_copy_ops   : 1;
    unsigned int f_valid_remove_ops : 1;
};

class Update
{
private:

    // Fields (internal)
    UpdateFlags d_flags;
    QMap<Transition, QVector<std::shared_ptr<Token>>> d_token_map;
    QString d_strerror;

    // Fields (meta)
    QString d_product_name;
    QString d_version_id;
    QString d_kernel_type;
    QString d_build_arch;
    QString d_file_pattern_match;

    // Fields (operational)
    QString d_media_path;
    QString d_system_path;
    QList<std::shared_ptr<SWU::Operation>> d_operations;

    // Method (private)
    const QVector<std::shared_ptr<Token>>* getTokensForTransition(SWU::Transition t);
    bool setFieldFromToken (QString *field_ptr, SWU::Transition token);
    bool setFieldFromTokenAttribute (QString *field_ptr, SWU::Transition token, SWU::attribute_key_t attributeKey);

    std::shared_ptr<SWU::Operation> getExpectOperation(std::shared_ptr<Token>);
    std::shared_ptr<SWU::Operation> getCopyOperation(std::shared_ptr<Token>);
    std::shared_ptr<SWU::Operation> getRemoveOperation(std::shared_ptr<Token>);

    std::shared_ptr<Token> acceptToDirectoryTag (QVector<std::shared_ptr<Token>> tokens, SWU::attribute_value_t *att_value_p = nullptr);
    std::shared_ptr<Token> acceptFileTag (QVector<std::shared_ptr<Token>> tokens, SWU::attribute_value_t *att_value_p = nullptr);
    std::shared_ptr<Token> acceptDirectoryTag (QVector<std::shared_ptr<Token>> tokens, SWU::attribute_value_t *att_value_p = nullptr);

public:

    // Constructor/destructor
     Update(const QVector<std::shared_ptr<Token>>& tokens);
    ~Update() = default;

    // Executable
    void run();

    // Getters
    const UpdateFlags flags();
    const QString name();
    const QString version();
    const QString kernel();
    const QString filePatternMatch();
    const QList<std::shared_ptr<SWU::Operation>> operations();
    const QString mediaPath();
    const QString systemPath();

    // Setters
    void setName (QString name);
    void setVersion (QString version);
    void setKernel (QString kernel);
    void setBuildArch (QString build_arch);
    void addOperation (const std::shared_ptr<Operation> operation);
    void setMediaPath (QString path);
    void setSystemPath (QString path);

protected:
//    virtual bool install_prepend();
//    virtual bool operation_prepend(const Operations::Operation operation);
//    virtual bool operation_append(const Operations::Operation operation);
//    virtual bool install_append();
};

}

#endif // UPDATE_H
