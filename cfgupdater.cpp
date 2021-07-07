#include "cfgupdater.h"

using namespace SWU;


UpdateDelegate::UpdateDelegate() {};
UpdateDelegate::~UpdateDelegate() = default;
UpdateStatus UpdateDelegate::on_init (SWU::Updater &updater)
{
    Q_UNUSED(updater);
    return STATUS_OK;
}
UpdateStatus UpdateDelegate::on_configure_resource_manager (
        ResourceManager &resourceManager,
        QVector<QString> &resource_uris)
{
    Q_UNUSED(resourceManager);
    Q_UNUSED(resource_uris);
    return STATUS_OK;
}
UpdateStatus UpdateDelegate::on_pre_validate (std::shared_ptr<ExpectOperation> op)
{
    Q_UNUSED(op);
    return STATUS_OK;
}
UpdateStatus UpdateDelegate::on_pre_backup (std::shared_ptr<CopyOperation> op)
{
    Q_UNUSED(op);
    return STATUS_OK;
}
UpdateStatus UpdateDelegate::on_pre_update (std::shared_ptr<FSOperation> op)
{
    Q_UNUSED(op);
    return STATUS_OK;
}
UpdateStatus UpdateDelegate::on_exit (UpdateStatus status,
                                      std::shared_ptr<FSOperation> op,
                                      OperationResult op_result)
{
    Q_UNUSED(status);
    Q_UNUSED(op);
    Q_UNUSED(op_result);
    return STATUS_OK;
}

Updater::Updater(Parser &parser, UpdateDelegate &delegate):
    d_status(STATUS_OK),
    d_update_delegate(delegate),
    d_product(parser.product()),
    d_platform(parser.platform()),
    d_resource_uris(parser.resource_uris()),
    d_backup_path(parser.backup_path()),
    d_validate_sp(0),
    d_backup_sp(0),
    d_update_sp(0)
{
    // Set: Validate operations
    for (off_t i = 0; i < parser.validate_operations().length(); ++i) {
        d_validate_operations.push_back(parser.validate_operations().at(i));
    }

    // Set: Backup operations
    for (off_t i = 0; i < parser.backup_operations().length(); ++i) {
        d_backup_operations.push_back(parser.backup_operations().at(i));
    }

    // Set: Update operations
    for (off_t i = 0; i < parser.update_operations().length(); ++i) {
        d_update_operations.push_back(parser.update_operations().at(i));
    }
}

UpdateStatus Updater::execute()
{
    UpdateStatus retval = STATUS_OK;
    OperationResult err = RESULT_OK;
    UpdateDelegate &d = d_update_delegate;

    // Check: Platform
    if (QSysInfo::kernelType().compare(d_platform) != 0) {
        return d_update_delegate.on_exit(STATUS_BAD_PLATFORM);
    }

    // Notify: Init
    if (STATUS_OK != (retval = d_update_delegate.on_init(*this))) {
        return d_update_delegate.on_exit(retval);
    }

    // Notify: Resource manager config
    if (STATUS_OK != (retval = d_update_delegate.on_configure_resource_manager(
                          ResourceManager::get_instance(),
                          d_resource_uris)))
    {
        return d_update_delegate.on_exit(retval);
    }

    // Run through operations block
    while (d_validate_sp < d_validate_operations.length()) {
        std::shared_ptr<ExpectOperation> e =
                std::dynamic_pointer_cast<ExpectOperation>(d_validate_operations.at(d_validate_sp));

        // Precondition
        if ((retval = d.on_pre_validate(e)) != STATUS_OK) {
            return d_update_delegate.on_exit(STATUS_BAD_PRECONDITION, e);
        }

        // Execute
        if ((err = e.get()->execute()) != RESULT_OK) {
            return d_update_delegate.on_exit(STATUS_BAD_RESULT, e, err);
        }

        // Increment offeset
        d_validate_sp++;
    }

    // Run through backup block
    while (d_backup_sp < d_backup_operations.length()) {
        std::shared_ptr<CopyOperation> c =
                std::dynamic_pointer_cast<CopyOperation>(d_backup_operations.at(d_backup_sp));

        // Precondition
        if ((retval = d.on_pre_backup(c)) != STATUS_OK) {
            return d_update_delegate.on_exit(STATUS_BAD_PRECONDITION, c);
        }

        // Execute
        if ((err = c.get()->execute()) != RESULT_OK) {
            return d_update_delegate.on_exit(STATUS_BAD_RESULT, c, err);
        }

        // Increment pointer
        d_backup_sp++;
    }

    // Run through update block (could be a remove, or copy operation)
    while (d_update_sp < d_update_operations.length()) {
        std::shared_ptr<FSOperation> op =d_backup_operations.at(d_update_sp);

        // Precondition
        if ((retval = d.on_pre_update(op)) != STATUS_OK) {
            return d_update_delegate.on_exit(STATUS_BAD_PRECONDITION, op);
        }

        // Execute
        if ((err = op.get()->execute()) != RESULT_OK) {
            return d_update_delegate.on_exit(STATUS_BAD_RESULT, op, err);
        }

        // Increment pointer
        d_update_sp++;
    }

    // Run exit condition
    return d_update_delegate.on_exit(retval);
}

UpdateStatus Updater::undo ()
{
    UpdateStatus retval = STATUS_OK;

    // In order to undo an update, the following must be done:
    // 1. All operations in the update block must be undone
    // 2. All backup operations must then be 'undone'


    // We want to undo update operations
    for (off_t i = d_update_sp; i >= 0; --i) {
        std::shared_ptr<FSOperation> op = d_update_operations.at(i);
        if (op->undo() != RESULT_OK) {
            return STATUS_BAD_UNDO;
        }
    }

    // We want to invert copy operations
    for (off_t i = d_backup_sp; i >= 0; --i) {
        std::shared_ptr<FSOperation> op = d_update_operations.at(i);
        if (op->undo() != RESULT_OK) {
            return STATUS_BAD_UNDO;
        }
    }

    return retval;
}

off_t Updater::validate_sp ()
{
    return d_validate_sp;
}

off_t Updater::backup_sp ()
{
    return d_backup_sp;
}

off_t Updater::update_sp ()
{
    return d_update_sp;
}

const QVector<std::shared_ptr<SWU::FSOperation>> Updater::validate_operations ()
{
    return d_validate_operations;
}

const QVector<std::shared_ptr<SWU::FSOperation>> Updater::backup_operations ()
{
    return d_backup_operations;
}

const QVector<std::shared_ptr<SWU::FSOperation>> Updater::update_operations ()
{
    return d_update_operations;
}
