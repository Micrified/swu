#ifndef CFGSTATEMACHINE_H
#define CFGSTATEMACHINE_H

#include <QObject>
#include <QMap>

namespace SWU {

// Number of machine states
static const size_t N_Machine_States = 18;
static const signed int StartState = 0;
static const signed int FinalState = 16;
static const signed int FaultState = 17;

// State machine statuses (stati?)
enum Status {
    STATUS_READY = 0,
    STATUS_FAULT,
    STATUS_COMPLETE
};

// State machine tokens
enum Token {
    T_CONFIGURATION_OPEN = 0,
    T_CONFIGURATION_CLOSE,
    T_RESOURCE_URI_OPEN,
    T_RESOURCE_URI_CLOSE,
    T_VALIDATE_OPEN,
    T_VALIDATE_CLOSE,
    T_FILE_OPEN,
    T_FILE_CLOSE,
    T_DIRECTORY_OPEN,
    T_DIRECTORY_CLOSE,
    T_BACKUP_OPEN,
    T_BACKUP_CLOSE,
    T_OPERATION_OPEN,
    T_OPERATION_CLOSE,
    T_COPY_OPEN,
    T_COPY_CLOSE,
    T_FROM_OPEN,
    T_FROM_CLOSE,
    T_TO_OPEN,
    T_TO_CLOSE,
    T_REMOVE_OPEN,
    T_REMOVE_CLOSE,

    /* Size */
    T_ENUM_MAX
};


class Machine : public QObject
{
    Q_OBJECT

private:

    // Internal state
    unsigned short d_state = 0;

    // Map: Lexemes -> Tokens
    QMap<QString, Token> d_token_hash_map;

    // Setup state machine
    void init_states ();

public:
    explicit Machine(QObject *parent = nullptr);

    // Input: token
    Status input (Token t);

    // Input: lexeme
    Status input (QString lex, Token *t_ptr = nullptr);

    // Get: machine status
    Status status ();

    // Reset the machine to its initial state
    void reset ();
signals:

};


}

#endif // CFGSTATEMACHINE_H
