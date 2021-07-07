#include "cfgparser.h"
using namespace SWU;

// Array-designation map: Token to lexeme
extern QString g_token_lexeme_map[];

// Forward declarations
static QString dropRootPrefix (const QString s);
static QString dropNameAndRootPrefix (const QString s);

std::shared_ptr<CFGElement> next(QVector<std::shared_ptr<CFGElement>> &elements)
{
    std::shared_ptr<CFGElement> e = nullptr;
    if (elements.count() > 0) {
        e = elements.first();
        elements.pop_front();
    }
    return e;
}

bool Parser::hasAttributeKeys (std::shared_ptr<CFGElement> element,
                             QVector<attribute_kp_pair> key_pointer_pairs)
{
    off_t matched = 0;
    Attributes a = Attributes::get_instance();

    for (auto keyval : key_pointer_pairs) {

        // Extract kv-pair
        attribute_kv_pair *kvpair = element->attribute_index(keyval.key);

        // If the attribute index is not valid - move on
        if (ATTRIBUTE_IS_UNSET(kvpair)) {
            break;
        }

        // Else copy the value to the given pointer
        (*keyval.lexeme_p) = QString(kvpair->lexeme);

        // Increment the counter
        matched++;
    }

    return matched == key_pointer_pairs.length();
}

off_t Parser::attributeValueInSet (QString raw_attribute, QVector<attribute_value_t> values) {
    Attributes a = Attributes::get_instance();
    attribute_value_t value = a.value(raw_attribute);
    for (off_t i = 0; i < values.length(); ++i) {
        if (values[i] == value) {
            return i;
        }
    }
    return -1;
}

Parser::Parser(const QVector<std::shared_ptr<CFGElement>>& elements)
{
    // Clone the elements
    QVector<std::shared_ptr<CFGElement>> elements_stack_copy(elements);

    // Accept configuration
    d_status = acceptConfiguration(elements_stack_copy);
}

ParseStatus Parser::status()
{
    return d_status;
}

ParseStatus Parser::acceptConfiguration(QVector<std::shared_ptr<CFGElement>>& elements)
{
    ParseStatus retval = PARSE_OK;
    std::shared_ptr<CFGElement> config = nullptr;

    // First stack element is the configuration
    if ((config = next(elements)) == nullptr) {
        return PARSE_INVALID_ELEMENT;
    } else {
        d_parse_stack.push_back(config->token());
    }

    // Extract expected attributes
    QVector<attribute_kp_pair> req_atts = QVector<attribute_kp_pair>({
        {ATTRIBUTE_KEY_PLATFORM, &d_platform},
        {ATTRIBUTE_KEY_PRODUCT,  &d_product}
    });
    if (!hasAttributeKeys(config, req_atts)) {
        return PARSE_INVALID_ATTRIBUTE_KEY;
    }

    // While there remain more elements on the stack
    while (elements.length() > 0) {
        std::shared_ptr<CFGElement> element = elements.front();
        d_parse_stack.push_back(element->token());

        switch (element->token()) {
        case T_RESOURCE_URI_OPEN:
            retval = acceptResourceURI(elements);
            break;
        case T_VALIDATE_OPEN:
            retval = acceptValidate(elements);
            break;
        case T_BACKUP_OPEN:
            retval = acceptBackup(elements);
            break;
        case T_OPERATION_OPEN:
            retval = acceptOperations(elements);
            break;

        default:
            retval = PARSE_INVALID_ELEMENT;
        }

        if (retval != PARSE_OK) {
            break;
        } else {
            d_parse_stack.pop_back();
        }
    }

    // Pop only on non-error
    if (retval == PARSE_OK) {
        d_parse_stack.pop_back();
    }

    return retval;
}

ParseStatus Parser::acceptFile (QVector<std::shared_ptr<CFGElement>>& elements,
                              QString *value_p)
{
    ParseStatus retval = PARSE_OK;
    Q_ASSERT(value_p != nullptr);

    // Pop the lead element off the stack
    std::shared_ptr<CFGElement> file = next(elements);

    // Copy value to pointer
    (*value_p) = QString(file->value());

    return retval;
}

ParseStatus Parser::acceptDirectory (QVector<std::shared_ptr<CFGElement>>& elements,
                                   QString *value_p)
{
    ParseStatus retval = PARSE_OK;
    Q_ASSERT(value_p != nullptr);

    // Pop the lead element off the stack
    std::shared_ptr<CFGElement> directory = next(elements);

    // Copy value to pointer
    (*value_p) = QString(directory->value());

    return retval;
}

ParseStatus Parser::acceptBackup (QVector<std::shared_ptr<CFGElement>>& elements)
{
    ParseStatus retval = PARSE_OK;
    QString temp_path_value = nullptr;
    std::shared_ptr<CFGElement> backup;

    // Pop the lead element off the stack
    if ((backup = next(elements)) == nullptr) {
        return PARSE_INVALID_ELEMENT;
    } else {
        d_parse_stack.push_back(backup->token());
    }

    // Extract expected attributes
    QVector<attribute_kp_pair> req_atts = QVector<attribute_kp_pair>({
        {ATTRIBUTE_KEY_PATH, &d_backup_path}
    });
    if (!hasAttributeKeys(backup, req_atts)) {
        return PARSE_INVALID_ATTRIBUTE_KEY;
    }

    // While we encounter elements of type: {file, directory}
    bool more = true;
    while (elements.length() > 0 && more) {
        std::shared_ptr<CFGElement> element = elements.front();
        d_parse_stack.push_back(element->token());

        switch (element->token()) {
        case T_FILE_OPEN:
            retval = acceptFile(elements, &temp_path_value);
            qInfo() << "backup to: " << QDir(d_backup_path).filePath(dropNameAndRootPrefix(temp_path_value));
            d_backup_operations.push_back(std::make_shared<CopyOperation>(CopyOperation(
              Resource(QString(temp_path_value), RESOURCE_TYPE_FILE),
              Resource(QDir(d_backup_path).filePath(dropNameAndRootPrefix(temp_path_value)),RESOURCE_TYPE_FILE))
            ));
            break;
        case T_DIRECTORY_OPEN:
            retval = acceptDirectory(elements, &temp_path_value);
            qInfo() << "backup to: " << QDir(d_backup_path).filePath(dropNameAndRootPrefix(temp_path_value));
            d_backup_operations.push_back(std::make_shared<CopyOperation>(CopyOperation(
              Resource(QString(temp_path_value), RESOURCE_TYPE_DIRECTORY),
              Resource(QDir(d_backup_path).filePath(dropNameAndRootPrefix(temp_path_value)), RESOURCE_TYPE_DIRECTORY))
            ));
            break;
        default:
            more = false;
            break;
        }

        if (retval != PARSE_OK) {
            break;
        } else {
            d_parse_stack.pop_back();
        }
    }

    // Pop only on non-error
    if (retval == PARSE_OK) {
        d_parse_stack.pop_back();
    }

    return retval;
}

ParseStatus Parser::acceptValidate (QVector<std::shared_ptr<CFGElement>>& elements)
{
    ParseStatus retval = PARSE_OK;
    QString temp_path_value = nullptr;
    std::shared_ptr<CFGElement> validate;

    // Pop the lead element off the stack
    if ((validate = next(elements)) == nullptr) {
        return PARSE_INVALID_ELEMENT;
    } else {
        d_parse_stack.push_back(validate->token());
    }

    // While we encounter elements of type: {file, directory}
    bool more = true;
    while (elements.length() > 0 && more) {
        std::shared_ptr<CFGElement> element = elements.front();
        d_parse_stack.push_back(element->token());

        switch (element->token()) {
        case T_FILE_OPEN:
            retval = acceptFile(elements, &temp_path_value);
            d_validate_operations.push_back(
                std::make_shared<ExpectOperation>(ExpectOperation(Resource(QString(temp_path_value), RESOURCE_TYPE_FILE,
                                         RESOURCE_KEY_REMOTE)))
            );
            break;
        case T_DIRECTORY_OPEN:
            retval = acceptDirectory(elements, &temp_path_value);
            d_validate_operations.push_back(
                std::make_shared<ExpectOperation>(ExpectOperation(Resource(QString(temp_path_value), RESOURCE_TYPE_DIRECTORY,
                                         RESOURCE_KEY_REMOTE)))
            );
            break;
        default:
            more = false;
            break;
        }

        if (retval != PARSE_OK) {
            break;
        } else {
            d_parse_stack.pop_back();
        }

    }

    // Pop only on non-error
    if (retval == PARSE_OK) {
        d_parse_stack.pop_back();
    }

    return retval;
}

ParseStatus Parser::acceptResourceURI(QVector<std::shared_ptr<CFGElement>>& elements)
{
    ParseStatus retval = PARSE_OK;

    // Pop the lead element off the stack
    std::shared_ptr<CFGElement> resourceURI = next(elements);

    // Copy URI to ordered vector
    d_resource_uris.append(QString(resourceURI->value()));

    return retval;
}

ParseStatus Parser::acceptOperations(QVector<std::shared_ptr<CFGElement>>& elements)
{
    ParseStatus retval = PARSE_OK;
    std::shared_ptr<CFGElement> operations;

    // Pop the lead element off the stack
    if ((operations = next(elements)) == nullptr) {
        return PARSE_INVALID_ELEMENT;
    } else {
        d_parse_stack.push_back(operations->token());
    }

    // While we encounter elements of type: {copy, remove}
    bool more = true;
    while (elements.length() > 0 && more) {
        std::shared_ptr<CFGElement> element = elements.front();
        d_parse_stack.push_back(element->token());

        switch (element->token()) {
        case T_COPY_OPEN:
            retval = acceptCopy(elements);
            break;
        case T_REMOVE_OPEN:
            retval = acceptRemove(elements);
            break;
        default:
            more = false;
            break;
        }

        if (retval != PARSE_OK) {
            break;
        } else {
            d_parse_stack.pop_back();
        }
    }

    // Pop only on non-error
    if (retval == PARSE_OK) {
        d_parse_stack.pop_back();
    }

    return retval;
}

ParseStatus Parser::acceptCopy(QVector<std::shared_ptr<CFGElement>>& elements)
{
    ParseStatus retval = PARSE_OK;
    std::shared_ptr<CFGElement> copy;
    resource_root_key_t from_root = RESOURCE_KEY_ENUM_MAX;
    resource_root_key_t to_root = RESOURCE_KEY_ENUM_MAX;

    QString from_path = nullptr, to_path = nullptr;
    QString from_root_value = nullptr, to_root_value = nullptr;
    off_t i = -1;

    // Pop the lead element off the stack
    if ((copy = next(elements)) == nullptr) {
        return PARSE_INVALID_ELEMENT;
    }

    // Require element: from
    d_parse_stack.push_back(elements.front()->token());
    if (elements.front()->token() != T_FROM_OPEN) {
        return PARSE_INVALID_ELEMENT;
    }
    if ((retval = acceptFrom(elements, &from_root_value, &from_path)) != PARSE_OK) {
        return retval;
    }

    // Valid attribute vector
    QVector<attribute_value_t> valid_attribute_index({
        ATTRIBUTE_VALUE_REMOTE,
        ATTRIBUTE_VALUE_TARGET
    });

    // CFGRootType array with index parity
    resource_root_key_t valid_root_index[] = {RESOURCE_KEY_REMOTE, RESOURCE_KEY_ROOT};

    // Validate: from
    if ((i = attributeValueInSet(from_root_value, valid_attribute_index)) == -1) {
        return PARSE_INVALID_ATTRIBUTE_VALUE;
    } else {
        from_root = valid_root_index[i];
    }
    d_parse_stack.pop_back();

    // Require element: to
    d_parse_stack.push_back(elements.front()->token());
    if (elements.front()->token() != T_TO_OPEN) {
        return PARSE_INVALID_ELEMENT;
    }
    if ((retval = acceptTo(elements, &to_root_value, &to_path)) != PARSE_OK) {
        return retval;
    }

    // Validate: to
    if ((i = attributeValueInSet(to_root_value, valid_attribute_index)) == -1) {
        return PARSE_INVALID_ATTRIBUTE_VALUE;
    } else {
        to_root = valid_root_index[i];
    }
    d_parse_stack.pop_back();

    // Push copy operation
    d_update_operations.push_back(std::make_shared<CopyOperation>(CopyOperation(
        Resource(from_path, RESOURCE_TYPE_FILE, from_root),
        Resource(to_path, RESOURCE_TYPE_DIRECTORY, to_root))
    ));

    return retval;
}

ParseStatus Parser::acceptFrom(QVector<std::shared_ptr<CFGElement>>& elements,
                             QString *root_p,
                             QString *value_p)
{
    ParseStatus retval = PARSE_OK;
    Q_ASSERT(root_p != nullptr);
    Q_ASSERT(value_p != nullptr);

    // Pop the lead element off the stack
    std::shared_ptr<CFGElement> from = next(elements);

    // Require attributes: {root}
    QVector<attribute_kp_pair> req_atts = QVector<attribute_kp_pair>({
        {ATTRIBUTE_KEY_ROOT, root_p}
    });
    if (!hasAttributeKeys(from, req_atts)) {
        return PARSE_INVALID_ATTRIBUTE_KEY;
    }

    // Copy value to pointer
    (*value_p) = QString(from->value());

    return retval;
}

ParseStatus Parser::acceptTo(QVector<std::shared_ptr<CFGElement>>& elements,
                           QString *root_p,
                           QString *value_p)
{
    ParseStatus retval = PARSE_OK;
    Q_ASSERT(root_p != nullptr);
    Q_ASSERT(value_p != nullptr);

    // Pop the lead element off the stack
    std::shared_ptr<CFGElement> to = next(elements);

    // Require attributes: {root}
    QVector<attribute_kp_pair> req_atts = QVector<attribute_kp_pair>({
        {ATTRIBUTE_KEY_ROOT, root_p}
    });
    if (!hasAttributeKeys(to, req_atts)) {
        return PARSE_INVALID_ATTRIBUTE_KEY;
    }

    // Copy value to pointer
    (*value_p) = QString(to->value());

    return retval;
}

ParseStatus Parser::acceptRemove(QVector<std::shared_ptr<CFGElement>>& elements)
{
    ParseStatus retval = PARSE_OK;
    QString root_value_raw = nullptr;
    attribute_value_t root_value = ATTRIBUTE_VALUE_ENUM_MAX;
    Attributes a = Attributes::get_instance();
    resource_root_key_t root_type = RESOURCE_KEY_ENUM_MAX;

    // Pop the lead element off the stack
    std::shared_ptr<CFGElement> remove = next(elements);

    // Require attributes: {root}
    QVector<attribute_kp_pair> req_atts = QVector<attribute_kp_pair>({
        {ATTRIBUTE_KEY_ROOT, &root_value_raw}
    });
    if (!hasAttributeKeys(remove, req_atts)) {
        return PARSE_INVALID_ATTRIBUTE_KEY;
    }

    // Validate attribute: {root}
    if ((root_value = a.value(root_value_raw)) == ATTRIBUTE_VALUE_REMOTE) {
        root_type = RESOURCE_KEY_REMOTE;
    }
    if (root_value == ATTRIBUTE_VALUE_TARGET) {
        root_type = RESOURCE_KEY_ROOT;
    }
    if (root_type == RESOURCE_KEY_ENUM_MAX) {
        return PARSE_INVALID_ATTRIBUTE_VALUE;
    }

    // Create a new resource
    std::shared_ptr<Resource> resource = std::make_shared<Resource>(
        QString(remove->value()),
        RESOURCE_TYPE_FILE,
        root_type
    );

    qDebug() << "acceptRemove(): " << remove.get()->description();
    qDebug() << "acceptRemove(): " << remove->value() ;

    // Append remove operation
    d_update_operations.push_back(
        std::make_shared<RemoveOperation>(RemoveOperation(resource))
    );

    return retval;
}

QString Parser::fault()
{
    // If not in an error state, return null
    if (PARSE_OK == d_status) {
        return nullptr;
    }

    // Otherwise build the fault stack
    QString description;
    switch (d_status) {
    case PARSE_INVALID_ELEMENT:
        description = QString("Invalid element: ");
        break;
    case PARSE_INVALID_ATTRIBUTE_KEY:
        description = QString("Missing/Invalid attribute key in element: ");
        break;
    case PARSE_INVALID_ATTRIBUTE_VALUE:
        description = QString("Missing/Invalid attribute value in element: ");
        break;
    default:
        description = QString("Parse anomaly: ");
        break;
    }

    for (off_t i = d_parse_stack.length() - 1; i >= 0; --i) {
        description += g_token_lexeme_map[d_parse_stack[i]];
        if (i > 0) {
            description += " in\n\t";
        }
    }
    return description + "\n";
}

QString Parser::product()
{
    return d_product;
}

QString Parser::platform()
{
    return d_platform;
}

QVector<QString> Parser::resource_uris()
{
    return d_resource_uris;
}

QString Parser::backup_path()
{
    return d_backup_path;
}

QVector<std::shared_ptr<SWU::FSOperation>> Parser::validate_operations()
{
    return d_validate_operations;
}

QVector<std::shared_ptr<SWU::FSOperation>> Parser::backup_operations()
{
    return d_backup_operations;
}

QVector<std::shared_ptr<SWU::FSOperation>> Parser::update_operations()
{
    return d_update_operations;
}

static QString dropRootPrefix (const QString s)
{
    QString clone = QString(s);
    if (clone.at(0) == '/') {
        clone.remove(0,1);
    }
    return clone;
}

static QString dropNameAndRootPrefix (const QString s) {
    QString path = dropRootPrefix(s);

    // drop the last element
    QStringList list = path.split("/");

    // Case: One element, then just return
    if (list.length() == 1) {
        return list.value(0);
    }

    // Otherwise concat
    QString concat = "";
    for (off_t i = 0; i < list.length() - 1; ++i) {
        concat += list.at(i);
        if (i < (list.length() - 1)) {
            concat += "/";
        }
    }
    return concat;
}
