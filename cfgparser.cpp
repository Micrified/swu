#include "cfgparser.h"
using namespace SWU;

std::shared_ptr<CFGElement> next(QVector<std::shared_ptr<CFGElement>> &elements)
{
    std::shared_ptr<CFGElement> e = nullptr;
    if (elements.count() > 0) {
        e = elements.first();
        elements.pop_front();
    }
    return e;
}

bool requireExactAttributes (std::shared_ptr<CFGElement> element,
                             QVector<attribute_kp_pair> key_pointer_pairs)
{
    off_t matched = 0;
    for (auto keyval : key_pointer_pairs) {

        // Extract kv-pair
        attribute_kv_pair *kvpair = element->attribute_index(keyval.key);

        // If the attribute index is not valid - move on
        if (ATTRIBUTE_IS_UNSET(kvpair)) {
            continue;
        }

        // Else copy the value to the given pointer
        (*keyval.lexeme_p) = QString(kvpair->lexeme);

        // Increment the counter
        matched++;
    }

    return matched == key_pointer_pairs.length();
}

off_t attributeInSet (QString raw_attribute, QVector<attribute_value_t> values) {
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
    qCritical() << __FUNCTION__ << ": ()";
    ParseStatus retval = PARSE_OK;
    std::shared_ptr<CFGElement> config = nullptr;

    // First stack element is the configuration
    if ((config = next(elements)) == nullptr) {
        qCritical() << __FUNCTION__ << ": couldn't find config element!";
        return PARSE_INVALID_ELEMENT;
    }

    // Extract expected attributes
    QVector<attribute_kp_pair> req_atts = QVector<attribute_kp_pair>({
        {ATTRIBUTE_KEY_PLATFORM, &d_platform},
        {ATTRIBUTE_KEY_PRODUCT,  &d_product}
    });
    if (!requireExactAttributes(config, req_atts)) {
        qCritical() << __FUNCTION__ << ": invalid attributes!";
        return PARSE_INVALID_ATTRIBUTE_KEY;
    }

    // While there remain more elements on the stack
    while (elements.length() > 0 && retval == PARSE_OK) {
        std::shared_ptr<CFGElement> head = elements.front();

        switch (head->token()) {
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
    }

    return retval;
}

ParseStatus Parser::acceptFile (QVector<std::shared_ptr<CFGElement>>& elements,
                              QString *value_p)
{
    qCritical() << __FUNCTION__ << ": ()";
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
    qCritical() << __FUNCTION__ << ": ()";
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
    qCritical() << __FUNCTION__ << ": ()";
    ParseStatus retval = PARSE_OK;
    QString temp_path_value = nullptr;

    // Pop the lead element off the stack
    std::shared_ptr<CFGElement> backup = next(elements);
    Q_UNUSED(backup);

    // While we encounter elements of type: {file, directory}
    bool more = true;
    while (elements.length() > 0 && more && retval == PARSE_OK) {
        std::shared_ptr<CFGElement> element = elements.front();
        switch (element->token()) {
        case T_FILE_OPEN:
            retval = acceptFile(elements, &temp_path_value);
            d_backup_file_paths.push_back(QString(temp_path_value));
            break;
        case T_DIRECTORY_OPEN:
            retval = acceptDirectory(elements, &temp_path_value);
            d_backup_directory_paths.push_back(QString(temp_path_value));
            break;
        default:
            more = false;
            break;
        }
    }
    return retval;
}

ParseStatus Parser::acceptValidate (QVector<std::shared_ptr<CFGElement>>& elements)
{
    qCritical() << __FUNCTION__ << ": ()";
    ParseStatus retval = PARSE_OK;
    QString temp_path_value = nullptr;

    // Pop the lead element off the stack
    std::shared_ptr<CFGElement> validate = next(elements);
    Q_UNUSED(validate);

    // While we encounter elements of type: {file, directory}
    bool more = true;
    while (elements.length() > 0 && more) {
        std::shared_ptr<CFGElement> element = elements.front();
        switch (element->token()) {
        case T_FILE_OPEN:
            retval = acceptFile(elements, &temp_path_value);
            d_validate_file_paths.push_back(QString(temp_path_value));
            break;
        case T_DIRECTORY_OPEN:
            retval = acceptDirectory(elements, &temp_path_value);
            d_validate_directory_paths.push_back(QString(temp_path_value));
            break;
        default:
            more = false;
            break;
        }
    }
    return retval;
}

ParseStatus Parser::acceptResourceURI(QVector<std::shared_ptr<CFGElement>>& elements)
{
    qCritical() << __FUNCTION__ << ": ()";
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
    qCritical() << __FUNCTION__ << ": ()";
    // Pop the lead element off the stack
    std::shared_ptr<CFGElement> operations = next(elements);
    Q_UNUSED(operations);

    // While we encounter elements of type: {copy, remove}
    bool more = true;
    while (elements.length() > 0 && more && retval == PARSE_OK) {
        std::shared_ptr<CFGElement> element = elements.front();
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
    }
    return retval;
}

ParseStatus Parser::acceptCopy(QVector<std::shared_ptr<CFGElement>>& elements)
{
    qCritical() << __FUNCTION__ << ": ()";
    ParseStatus retval = PARSE_OK;
    CFGRootType from_root = ROOT_TYPE_ENUM_MAX, to_root = ROOT_TYPE_ENUM_MAX;
    QString from_path = nullptr, to_path = nullptr;
    QString from_root_value = nullptr, to_root_value = nullptr;
    off_t i = -1;

    // Pop the lead element off the stack
    std::shared_ptr<CFGElement> copy = next(elements);
    Q_UNUSED(copy);

    // Require element: from
    if (elements.front()->token() != T_FROM_OPEN) {
        return PARSE_INVALID_ELEMENT;
    }
    if ((retval = acceptFrom(elements, &from_root_value, &from_path)) != PARSE_OK) {
        qCritical() << __FUNCTION__ << ": acceptFrom() fault";
        return retval;
    }

    // Valid attribute vector
    QVector<attribute_value_t> valid_attribute_index({
        ATTRIBUTE_VALUE_REMOTE,
        ATTRIBUTE_VALUE_TARGET
    });

    // CFGRootType array with index parity
    CFGRootType valid_root_index[] = {ROOT_TYPE_REMOTE, ROOT_TYPE_TARGET};

    // Validate: from
    if ((i = attributeInSet(from_root_value, valid_attribute_index)) == -1) {
        qCritical() << __FUNCTION__ << ": invalid attribute for <from> root";
        return PARSE_INVALID_ATTRIBUTE_VALUE;
    } else {
        from_root = valid_root_index[i];
    }

    // Require element: to
    if (elements.front()->token() != T_TO_OPEN) {
        qCritical() << __FUNCTION__ << ": expected <to> but didn't find it";
        return PARSE_INVALID_ELEMENT;
    }
    if ((retval = acceptTo(elements, &to_root_value, &to_path)) != PARSE_OK) {
        qCritical() << __FUNCTION__ << ": unable to parse <to>";
        return retval;
    }

    // Validate: to
    if ((i = attributeInSet(to_root_value, valid_attribute_index)) == -1) {
        qCritical() << __FUNCTION__ << ": invalid attribute for <to> root";
        return PARSE_INVALID_ATTRIBUTE_VALUE;
    } else {
        to_root = valid_root_index[i];
    }

    // Push copy operation
    d_copy_operations.push_back({from_root, to_root, from_path, to_path});

    return retval;
}

ParseStatus Parser::acceptFrom(QVector<std::shared_ptr<CFGElement>>& elements,
                             QString *root_p,
                             QString *value_p)
{
    qCritical() << __FUNCTION__ << ": ()";
    ParseStatus retval = PARSE_OK;
    Q_ASSERT(root_p != nullptr);
    Q_ASSERT(value_p != nullptr);

    // Pop the lead element off the stack
    std::shared_ptr<CFGElement> from = next(elements);

    // Require attributes: {root}
    QVector<attribute_kp_pair> req_atts = QVector<attribute_kp_pair>({
        {ATTRIBUTE_KEY_ROOT, root_p}
    });
    if (!requireExactAttributes(from, req_atts)) {
        qCritical() << __FUNCTION__ << ": could not find required attributes";
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
    qCritical() << __FUNCTION__ << ": ()";
    ParseStatus retval = PARSE_OK;
    Q_ASSERT(root_p != nullptr);
    Q_ASSERT(value_p != nullptr);

    // Pop the lead element off the stack
    std::shared_ptr<CFGElement> to = next(elements);

    // Require attributes: {root}
    QVector<attribute_kp_pair> req_atts = QVector<attribute_kp_pair>({
        {ATTRIBUTE_KEY_ROOT, root_p}
    });
    if (!requireExactAttributes(to, req_atts)) {
        return PARSE_INVALID_ATTRIBUTE_KEY;
    }

    // Copy value to pointer
    (*value_p) = QString(to->value());

    return retval;
}

ParseStatus Parser::acceptRemove(QVector<std::shared_ptr<CFGElement>>& elements)
{
    qCritical() << __FUNCTION__ << ": ()";
    ParseStatus retval = PARSE_OK;
    QString root_value_raw = nullptr;
    attribute_value_t root_value = ATTRIBUTE_VALUE_ENUM_MAX;
    Attributes a = Attributes::get_instance();
    CFGRootType root_type = ROOT_TYPE_ENUM_MAX;

    // Pop the lead element off the stack
    std::shared_ptr<CFGElement> remove = next(elements);

    // Require attributes: {root}
    QVector<attribute_kp_pair> req_atts = QVector<attribute_kp_pair>({
        {ATTRIBUTE_KEY_ROOT, &root_value_raw}
    });
    if (!requireExactAttributes(remove, req_atts)) {
        return PARSE_INVALID_ATTRIBUTE_KEY;
    }

    // Validate attribute: {root}
    if ((root_value = a.value(root_value_raw)) == ATTRIBUTE_VALUE_REMOTE) {
        root_type = ROOT_TYPE_REMOTE;
    }
    if (root_value == ATTRIBUTE_VALUE_TARGET) {
        root_type = ROOT_TYPE_TARGET;
    }
    if (root_type == ROOT_TYPE_ENUM_MAX) {
        return PARSE_INVALID_ATTRIBUTE_VALUE;
    }

    // Append remove operation
    d_remove_operations.push_back({root_type, remove->value()});

    return retval;
}
