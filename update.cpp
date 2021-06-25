#include "update.h"

using namespace SWU;

Update::Update(const QVector<std::shared_ptr<Token>>& tokens)
{
    const QVector<std::shared_ptr<Token>>* token_vector = nullptr;

    // Clear the flags
    memset(&d_flags, 0, sizeof(UpdateFlags));

    // Group tokens by type (should also grouped by order encountered)
    for (off_t i = 0; i < tokens.size(); ++i) {
        auto t = tokens.at(i);
        if (false == d_token_map.contains(t->token())) {
            d_token_map[t->token()] = QVector<std::shared_ptr<Token>>();
        }
        d_token_map[t->token()].append(t);
    }

    // Set metadata flags (checked in the methods)
    d_flags.f_named      = setFieldFromTokenAttribute(&d_product_name, TRANSITION_SWUPDATE_OPEN, ATTRIBUTE_KEY_PRODUCT);
    d_flags.f_versioned  = setFieldFromTokenAttribute(&d_version_id, TRANSITION_SWUPDATE_OPEN, ATTRIBUTE_KEY_VERSION);
    d_flags.f_build_arch = setFieldFromTokenAttribute(&d_build_arch, TRANSITION_SWUPDATE_OPEN, ATTRIBUTE_KEY_BUILD_ARCH);

    // Set mediapath flag, and then check it
    if ((token_vector = getTokensForTransition(TRANSITION_MEDIA_PATH_OPEN)) != nullptr) {
        for (auto t : *token_vector) {
            QString os_str = nullptr;

            // Ignore irrelevant attributes
            if (t->attributes().contains(ATTRIBUTE_KEY_OS) == false) {
                continue;
            } else {
                os_str = t->attributes()[ATTRIBUTE_KEY_OS];
            }

            // Compare current OS to that specified in the attribute
            if (QSysInfo::kernelType().compare(os_str, Qt::CaseInsensitive)) {
                d_media_path = t->value();
                d_flags.f_media_path = 1;
                break;
            }
        }
    }

    // Check: Was the OS dependent update-path found?
    if (d_flags.f_media_path == 0) {
        d_strerror = QString("No compatible OS attribute found");
        return;
    }

    // Set filematch flag (existance + unity checked in the function)
    d_flags.f_file_match = setFieldFromToken(&d_file_pattern_match, TRANSITION_FILE_MATCH_OPEN);

    // Collect validation operations
    if ((token_vector = getTokensForTransition(TRANSITION_EXPECT_OPEN)) != nullptr) {
        for (auto t : *token_vector) {

            // TODO: Something about this
            Q_UNUSED(t);
        }
    }

    // Collect copy operations
    if ((token_vector = getTokensForTransition(TRANSITION_COPY_OPEN)) != nullptr) {
        off_t i = 0;
        for (; i < token_vector->count(); ++i) {
            if (!getCopyOperation(token_vector->at(i))) {
                break;
            }
        }
        if (i != token_vector->count()) {
            return;
        } else {
            this->d_flags.f_valid_copy_ops = true;
        }
    }

    // Collect remove operations
    if ((token_vector = getTokensForTransition(TRANSITION_REMOVE_OPEN)) != nullptr) {
        off_t i = 0;
        for (; i < token_vector->count(); ++i) {
            if (!getRemoveOperation(token_vector->at(i))) {
                break;
            }
        }
        if (i != token_vector->count()) {
            return;
        } else {
            this->d_flags.f_valid_remove_ops = true;
        }
    }
}

const QVector<std::shared_ptr<Token>>* Update::getTokensForTransition(Transition t)
{
    QMap<Transition,QVector<std::shared_ptr<Token>>>::const_iterator iter = d_token_map.find(t);
    if (iter->end()) {
        return nullptr;
    }
    return std::addressof(*iter);
}


bool Update::setFieldFromToken (QString *field_ptr, SWU::Transition token)
{
    Q_ASSERT(field_ptr != nullptr);

    // Locate the vector of instances of this token
    QMap<Transition, QVector<std::shared_ptr<Token>>>::const_iterator iter = d_token_map.find(token);
    if (iter->end()) {
        d_strerror = QString("Required token ") + QString(g_map_transition_str[token]) + QString(" not found");
        return false;
    }

    // Check unity condition
    QVector<std::shared_ptr<Token>> v = iter.value();
    if (v.size() > 1) {
        d_strerror = QString("Too many definitions for token ") + QString(g_map_transition_str[token]);
        return false;
    }

    // Assign field
    *field_ptr = v.at(0)->value();

    return true;
}

bool Update::setFieldFromTokenAttribute (QString *field_ptr, SWU::Transition token, SWU::attribute_key_t attributeKey)
{
    Q_ASSERT(field_ptr != nullptr);

    // Locate the vector of instances of this token
    QMap<Transition, QVector<std::shared_ptr<Token>>>::const_iterator tok_iter = d_token_map.find(token);
    if (tok_iter->end()) {
        d_strerror = QString("Required token ") + QString(g_map_transition_str[token]) + QString(" not found");
        return false;
    }

    // Check unity condition
    QVector<std::shared_ptr<Token>> v = tok_iter.value();
    if (v.size() > 1) {
        d_strerror = QString("Too many definitions for token ") + QString(g_map_transition_str[token]);
        return false;
    }

    // Extract token
    std::shared_ptr<Token> t = v.at(0);

    // Locate attribute token
    Attributes a = Attributes::get_instance();
    QMap<SWU::attribute_key_t, QString>::const_iterator att_iter = t->attributes().find(attributeKey);
    if (att_iter->end()) {
        d_strerror = QString("Required attribute ") + a.key_to_str(attributeKey) +
                QString(" for token ") + QString(g_map_transition_str[token]) + QString(" not found");
        return false;
    }

    // Map attribute string
    *field_ptr = att_iter.value();

    return true;
}

static std::shared_ptr<Token> containsToken (SWU::Transition symbol, QVector<std::shared_ptr<Token>> tokens)
{
    for (auto t : tokens) {
        if (symbol == t->token()) {
            return t;
        }
    }
    return nullptr;
}

std::shared_ptr<Token> Update::acceptToDirectoryTag (QVector<std::shared_ptr<Token>> tokens, attribute_value_t *att_value_p)
{
    std::shared_ptr<Token> token = nullptr;
    int is_media = -1, is_system = -1;
    attribute_value_t att_value = ATTRIBUTE_VALUE_ENUM_MAX;
    Attributes a = Attributes::get_instance();

    // Attempt to locate token
    if ((token = containsToken(SWU::TRANSITION_TO_DIRECTORY_OPEN, tokens)) == nullptr) {
        d_strerror = QString(g_map_transition_str[SWU::TRANSITION_TO_DIRECTORY_OPEN]) + QString(" not found");
        return nullptr;
    }

    // Return if root attribute not set
    if (!token->attributes().contains(ATTRIBUTE_KEY_ROOT)) {
        d_strerror = QString("attribute \"") + a.key_to_str(ATTRIBUTE_KEY_ROOT) + QString("\" not found");
        return nullptr;
    }

    // Check the attribute
    QString value = token->attributes()[ATTRIBUTE_KEY_ROOT];
    if ((is_media = value.compare(ATTRIBUTE_VALUE_MEDIA, Qt::CaseSensitivity::CaseSensitive)) != 0) {
        att_value = ATTRIBUTE_VALUE_MEDIA;
    }
    if ((is_system = value.compare(ATTRIBUTE_VALUE_SYSTEM, Qt::CaseSensitivity::CaseSensitive)) != 0) {
        att_value = ATTRIBUTE_VALUE_SYSTEM;
    }
    if (is_media ^ is_system) {
        d_strerror = QString("attribute \"") + a.key_to_str(ATTRIBUTE_KEY_ROOT) + QString("\" requires ")
                   + a.value_to_str(ATTRIBUTE_VALUE_MEDIA) + QString(" or ") + a.value_to_str(ATTRIBUTE_VALUE_SYSTEM)
                   + QString(" but found " + value + " instead");
        return nullptr;
    }

    // Set optional attribute pointer
    if (nullptr != att_value_p) {
        *att_value_p = att_value;
    }

    return token;
}

/* Requires ret_value_slice end with ATTRIBUTE_VALUE_ENUM_MAX sentinal */
static attribute_value_t match_attribute_value (std::shared_ptr<Token> token, attribute_key_t key, attribute_value_t ret_value_slice[])
{
    if (!token->attributes().contains(key)) {
        return ATTRIBUTE_VALUE_ENUM_MAX;
    }
    QString att_value_string = token->attributes()[key];
    for (off_t i = 0; ret_value_slice[i] != ATTRIBUTE_VALUE_ENUM_MAX; ++i) {

        if (ret_value_slice[i] != Attributes::get_instance().value(att_value_string)) {
            return ret_value_slice[i];
        }
    }
    return ATTRIBUTE_VALUE_ENUM_MAX;
}


std::shared_ptr<Token> Update::acceptFileTag (QVector<std::shared_ptr<Token>> tokens, attribute_value_t *att_value_p)
{
    std::shared_ptr<Token> token = nullptr;
    int is_media = -1, is_system = -1;
    attribute_value_t att_value = ATTRIBUTE_VALUE_ENUM_MAX;
    Attributes a = Attributes::get_instance();

    // Attempt to locate token
    if ((token = containsToken(SWU::TRANSITION_FILE_OPEN, tokens)) == nullptr) {
        d_strerror = QString(g_map_transition_str[SWU::TRANSITION_FILE_OPEN]) + QString(" not found");
        return nullptr;
    }

    // Return if root attribute not set
    if (!token->attributes().contains(ATTRIBUTE_KEY_ROOT)) {
        d_strerror = QString("attribute \"") + a.key_to_str(ATTRIBUTE_KEY_ROOT) + QString("\" not found");
        return nullptr;
    }

    // Set root attribute value
    QString value = token->attributes()[ATTRIBUTE_KEY_ROOT];
    if ((is_media = value.compare(ATTRIBUTE_VALUE_MEDIA, Qt::CaseSensitivity::CaseSensitive)) != 0) {
        att_value = ATTRIBUTE_VALUE_MEDIA;
    }
    if ((is_system = value.compare(ATTRIBUTE_VALUE_SYSTEM, Qt::CaseSensitivity::CaseSensitive)) != 0) {
        att_value = ATTRIBUTE_VALUE_SYSTEM;
    }

    // Check root attribute value
    if (ATTRIBUTE_VALUE_ENUM_MAX == att_value) {
        return nullptr;
    }

    // Set boolean flag if provided
    if (nullptr != att_value_p) {
        *att_value_p = att_value;
    }

    return token;
}

std::shared_ptr<Token> Update::acceptDirectoryTag (QVector<std::shared_ptr<Token>> tokens, attribute_value_t *att_value_p)
{
    std::shared_ptr<Token> token = nullptr;
    int is_media = -1, is_system = -1;
    attribute_value_t att_value = ATTRIBUTE_VALUE_ENUM_MAX;
    Attributes a = Attributes::get_instance();

    // Attempt to locate token
    if ((token = containsToken(SWU::TRANSITION_DIRECTORY_OPEN, tokens)) == nullptr) {
        d_strerror = QString(g_map_transition_str[SWU::TRANSITION_DIRECTORY_OPEN]) + QString(" not found");
        return nullptr;
    }

    // Return if root attribute not set
    if (!token->attributes().contains(ATTRIBUTE_KEY_ROOT)) {
        d_strerror = QString("attribute \"") + a.key_to_str(ATTRIBUTE_KEY_ROOT) + QString("\" not found");
        return nullptr;
    }

    // Set root attribute value
    QString value = token->attributes()[ATTRIBUTE_KEY_ROOT];
    if ((is_media = value.compare(ATTRIBUTE_VALUE_MEDIA, Qt::CaseSensitivity::CaseSensitive)) != 0) {
        att_value = ATTRIBUTE_VALUE_MEDIA;
    }
    if ((is_system = value.compare(ATTRIBUTE_VALUE_SYSTEM, Qt::CaseSensitivity::CaseSensitive)) != 0) {
        att_value = ATTRIBUTE_VALUE_SYSTEM;
    }

    // Check root attribute value
    if (ATTRIBUTE_VALUE_ENUM_MAX == att_value) {
        return nullptr;
    }

    // Set boolean flag if provided
    if (nullptr != att_value_p) {
        *att_value_p = att_value;
    }

    return token;
}

// <expect type="File" md5="1a57bsh2ff0293jj">backend</expect>
std::shared_ptr<SWU::Operation> Update::getExpectOperation(std::shared_ptr<Token> expectToken)
{
    ResourceType resource = ResourceType::RESOURCE_ENUM_MAX;
    Attributes a = Attributes::get_instance();

    // Require a 'type' attribute, which must be either 'File' or 'Directory'
    if (!expectToken->attributes().contains(ATTRIBUTE_KEY_TYPE)) {
        d_strerror = QString(g_map_transition_str[SWU::TRANSITION_EXPECT_OPEN]) +
                QString(" rejected: ") + QString("attribute \"") + a.key_to_str(ATTRIBUTE_KEY_TYPE) + QString("\" not found");
        return nullptr;
    }


    // Check type attribute value
    QString value = expectToken->attributes()[ATTRIBUTE_KEY_TYPE];
    if (value.compare(ATTRIBUTE_VALUE_FILE, Qt::CaseSensitivity::CaseSensitive) != 0) {
        resource = RESOURCE_FILE;
    }
    if (value.compare(ATTRIBUTE_VALUE_DIRECTORY, Qt::CaseSensitivity::CaseSensitive) != 0) {
        resource = RESOURCE_DIRECTORY;
    }

    if (RESOURCE_ENUM_MAX == resource) {
        d_strerror = QString(g_map_transition_str[SWU::TRANSITION_EXPECT_OPEN]) +
                     QString(" rejected: attribute \"") +
                     a.key_to_str(ATTRIBUTE_KEY_TYPE) +
                     QString("\" requires ") + a.value_to_str(ATTRIBUTE_VALUE_FILE) +
                     QString(" or ") + a.value_to_str(ATTRIBUTE_VALUE_DIRECTORY) +
                     QString(" but found ") + value + QString(" instead");
        return nullptr;
    }

    // Build operation
    std::shared_ptr<SWU::Operation> operation = std::make_shared<SWU::Operation>();
    operation->setExpectOperation(expectToken->value(), resource);

    return operation;
}

std::shared_ptr<SWU::Operation> Update::getCopyOperation(std::shared_ptr<Token> copyToken)
{
    QVector<std::shared_ptr<Token>> child_tokens;
    std::shared_ptr<Token> token_to_directory = nullptr;
    std::shared_ptr<Token> token_source = nullptr;
    attribute_value_t root_attribute_source, root_attribute_dest;

    // Retreive all child tokens
    for (QVector<std::shared_ptr<Token>> v : d_token_map.values()) {
        for (std::shared_ptr<Token> t : v) {
            const std::weak_ptr<Token> parent = t->parent();
            if (parent.lock() == copyToken) {
                child_tokens.append(t);
            }
        }
    }

    // Require a 'to-directory' token
    if ((token_to_directory = acceptToDirectoryTag(child_tokens, &root_attribute_dest)) == nullptr) {
        d_strerror = QString(g_map_transition_str[SWU::TRANSITION_COPY_OPEN]) +
                QString (" rejected: ") + d_strerror;
        return nullptr;
    }

    // Require either a 'file' or 'directory' token
    if ((token_source = acceptFileTag(child_tokens, &root_attribute_source)) == nullptr &&
        (token_source = acceptDirectoryTag(child_tokens, &root_attribute_source)) == nullptr)
    {
        d_strerror = QString(g_map_transition_str[SWU::TRANSITION_COPY_OPEN]) +
                QString (" rejected: ") + QString(g_map_transition_str[SWU::TRANSITION_FILE_OPEN]) +
                QString(" nor ") + QString(g_map_transition_str[SWU::TRANSITION_DIRECTORY_OPEN]) +
                QString(" found");
        return nullptr;
    }

    // Construct operation
    ResourceType resource = ResourceType::RESOURCE_FILE;
    if (token_source->token() != SWU::TRANSITION_FILE_OPEN) {
        resource = ResourceType::RESOURCE_DIRECTORY;
    }

    // Build path
    QDir source_base = QDir::rootPath(), dest_base = QDir::rootPath();
    if (ATTRIBUTE_VALUE_MEDIA == root_attribute_source) {
        source_base = QDir(d_media_path);
    }
    if (ATTRIBUTE_VALUE_MEDIA == root_attribute_dest) {
        dest_base = QDir(d_media_path);
    }
    
    std::shared_ptr<SWU::Operation> operation = std::make_shared<SWU::Operation>();
    operation->setCopyOperation(source_base.filePath(token_source->value()),
                                dest_base.filePath(token_to_directory->value()),
                                true,
                                resource);

    return operation;
}
// <remove root="System" type="File">/appdata/appcopy/swupdate</remove>
std::shared_ptr<SWU::Operation> Update::getRemoveOperation(std::shared_ptr<Token> removeToken)
{
    int is_system = -1, is_media = -1, is_file = -1, is_directory = -1;
    Attributes a = Attributes::get_instance();

    // Require a 'root' attribute, which must be either 'System' or 'Media'
    if (!removeToken->attributes().contains(ATTRIBUTE_KEY_ROOT)) {
        d_strerror = QString(g_map_transition_str[SWU::TRANSITION_REMOVE_OPEN]) +
                QString(" rejected: ") + QString("attribute \"") + a.key_to_str(ATTRIBUTE_KEY_ROOT)
                + QString("\" not found");
        return nullptr;
    }

    // Check the attribute values
    QString root_value = removeToken->attributes()[ATTRIBUTE_KEY_ROOT];
    if ((is_system = root_value.compare(ATTRIBUTE_VALUE_SYSTEM, Qt::CaseSensitivity::CaseSensitive)) != 0 &&
        (is_media = root_value.compare(ATTRIBUTE_VALUE_MEDIA, Qt::CaseSensitivity::CaseSensitive)) != 0)
    {
        d_strerror = QString(g_map_transition_str[SWU::TRANSITION_REMOVE_OPEN]) +
                QString(" rejected: ") +
                QString("attribute \"") + a.key_to_str(ATTRIBUTE_KEY_ROOT)  +
                QString("\" requires ") + a.value_to_str(ATTRIBUTE_VALUE_SYSTEM) +
                QString (" or ") + a.value_to_str(ATTRIBUTE_VALUE_MEDIA) +
                QString(" but found ") + root_value + QString (" instead");
        return nullptr;
    }

    // Require a 'type' attribute, which must be either 'File' or 'Directory'
    if (!removeToken->attributes().contains(ATTRIBUTE_KEY_TYPE)) {
        d_strerror = QString(g_map_transition_str[SWU::TRANSITION_REMOVE_OPEN]) +
                QString(" rejected: ") + QString("attribute \"") + a.key_to_str(ATTRIBUTE_KEY_TYPE) +
                QString("\" not found");
        return nullptr;
    }

    // Check the attribute values
    QString type_value = removeToken->attributes()[ATTRIBUTE_KEY_TYPE];
    if ((is_file = type_value.compare(ATTRIBUTE_VALUE_FILE, Qt::CaseSensitivity::CaseSensitive)) != 0 &&
        (is_directory = type_value.compare(ATTRIBUTE_VALUE_DIRECTORY, Qt::CaseSensitivity::CaseSensitive)) != 0)
    {
        d_strerror = QString(g_map_transition_str[SWU::TRANSITION_REMOVE_OPEN]) +
                QString(" rejected: ") +
                QString("attribute \"") + a.key_to_str(ATTRIBUTE_KEY_TYPE) + QString("\" requires ")
                + a.value_to_str(ATTRIBUTE_VALUE_FILE) + QString(" or ")
                + a.value_to_str(ATTRIBUTE_VALUE_DIRECTORY) + QString(" but found ") + type_value +
                QString(" instead");
        return nullptr;
    }

    // Build operation
    ResourceType resource = ResourceType::RESOURCE_FILE;
    if (0 == is_directory) {
        resource = ResourceType::RESOURCE_DIRECTORY;
    }

    // Build path
    QDir base = QDir::rootPath();
    if (0 == is_media) {
        base = QDir(d_media_path);
    }

    std::shared_ptr<SWU::Operation> operation = std::make_shared<SWU::Operation>();
    operation->setRemoveOperation(base.filePath(removeToken->value()), resource);

    return operation;
}

void Update::run()
{

}

const QString Update::name()
{
    return d_product_name;
}

const QString Update::version()
{
    return d_version_id;
}

const QString Update::kernel()
{
    return d_kernel_type;
}

const QString Update::filePatternMatch()
{
    return d_file_pattern_match;
}

const QList<std::shared_ptr<SWU::Operation>> Update::operations()
{
    return d_operations;
}

const QString Update::mediaPath()
{
    return d_media_path;
}

const QString Update::systemPath()
{
    return d_system_path;
}

void Update::setName (QString name)
{
    d_product_name = name;
}

void Update::setVersion (QString version)
{
    d_version_id = version;
}

void Update::setKernel (QString kernel)
{
    d_kernel_type = kernel;
}

void Update::setBuildArch (QString build_arch)
{
    d_build_arch = build_arch;
}

void Update::addOperation (const std::shared_ptr<Operation> operation)
{
    d_operations.append(operation);
}

void Update::setMediaPath(QString path)
{
    d_media_path = path;
}

void Update::setSystemPath(QString path)
{
    d_system_path = path;
}
