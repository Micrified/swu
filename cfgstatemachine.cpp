#include "cfgstatemachine.h"

using namespace SWU;


/*
 *******************************************************************************
 *                         Static variable definitions                         *
 *******************************************************************************
*/


#define Q_XML_OPEN(x)         QString("<"  x ">")
#define Q_XML_CLOSE(x)        QString("</" x ">")
#define Q_TOK_PAIR(T1,T2,L)     [T1] = Q_XML_OPEN(L), [T2] = Q_XML_CLOSE(L)

// State machine map (init to invalid states)
static signed short g_map[N_Machine_States][T_ENUM_MAX] = {{FaultState}};

// Array-designation map: Token to lexeme
QString g_token_lexeme_map[T_ENUM_MAX] = {
    Q_TOK_PAIR(T_CONFIGURATION_OPEN, T_CONFIGURATION_CLOSE, "configuration"),
    Q_TOK_PAIR(T_RESOURCE_URI_OPEN, T_RESOURCE_URI_CLOSE, "resource-uri"),
    Q_TOK_PAIR(T_VALIDATE_OPEN, T_VALIDATE_CLOSE, "validate"),
    Q_TOK_PAIR(T_FILE_OPEN, T_FILE_CLOSE, "file"),
    Q_TOK_PAIR(T_DIRECTORY_OPEN, T_DIRECTORY_CLOSE, "directory"),
    Q_TOK_PAIR(T_BACKUP_OPEN, T_BACKUP_CLOSE, "backup"),
    Q_TOK_PAIR(T_OPERATION_OPEN, T_OPERATION_CLOSE, "operations"),
    Q_TOK_PAIR(T_COPY_OPEN, T_COPY_CLOSE, "copy"),
    Q_TOK_PAIR(T_FROM_OPEN, T_FROM_CLOSE, "from"),
    Q_TOK_PAIR(T_TO_OPEN, T_TO_CLOSE, "to"),
    Q_TOK_PAIR(T_REMOVE_OPEN, T_REMOVE_CLOSE, "remove")
};


/*
 *******************************************************************************
 *                          Class definition: Machine                          *
 *******************************************************************************
*/


Machine::Machine(QObject *parent) : QObject(parent)
{
    // Initialise the states
    init_states();

    // Initialize the hashmap
    for (off_t t = 0; t < T_ENUM_MAX; ++t) {
        d_token_hash_map[g_token_lexeme_map[t]] = static_cast<Token>(t);
    }
}

void Machine::init_states ()
{
    // State 0
    g_map[0][T_CONFIGURATION_OPEN]  = 1;

    // State 1
    g_map[1][T_RESOURCE_URI_OPEN]   = 2;
    g_map[1][T_VALIDATE_OPEN]       = 3;
    g_map[1][T_BACKUP_OPEN]         = 6;
    g_map[1][T_OPERATION_OPEN]      = 9;
    g_map[1][T_CONFIGURATION_CLOSE] = FinalState;

    // State 2
    g_map[2][T_RESOURCE_URI_CLOSE]  = 1;

    // State 3
    g_map[3][T_VALIDATE_CLOSE]      = 1;
    g_map[3][T_FILE_OPEN]           = 4;
    g_map[3][T_DIRECTORY_OPEN]      = 5;

    // State 4
    g_map[4][T_FILE_CLOSE]          = 3;

    // State 5
    g_map[5][T_DIRECTORY_CLOSE]     = 3;

    // State 6
    g_map[6][T_FILE_OPEN]           = 7;
    g_map[6][T_DIRECTORY_OPEN]      = 8;
    g_map[6][T_BACKUP_CLOSE]        = 1;

    // State 7
    g_map[7][T_FILE_CLOSE]          = 6;

    // State 8
    g_map[8][T_DIRECTORY_CLOSE]     = 6;

    // State 9
    g_map[9][T_COPY_OPEN]           = 10;
    g_map[9][T_REMOVE_OPEN]         = 15;
    g_map[9][T_OPERATION_CLOSE]     = 1;

    // State 10
    g_map[10][T_FROM_OPEN]          = 11;

    // State 11
    g_map[11][T_FROM_CLOSE]         = 12;

    // State 12
    g_map[12][T_TO_OPEN]            = 13;

    // State 13
    g_map[13][T_TO_CLOSE]           = 14;

    // State 14
    g_map[14][T_COPY_CLOSE]         = 9;

    // State 15
    g_map[15][T_REMOVE_CLOSE]       = 9;
}

Status Machine::input(Token t)
{
    d_state = g_map[d_state][t];
    return status();
}

Status Machine::input(QString t, Token *t_ptr)
{
    // Lookup the token for the lexeme
    QMap<QString, Token>::const_iterator iter = d_token_hash_map.find(t);
    if (iter == d_token_hash_map.end()) {
        return STATUS_FAULT;
    }
    if (t_ptr != nullptr) {
        *t_ptr = iter.value();
    }
    return input(iter.value());
}

Status Machine::status()
{
    switch (d_state) {
    case FinalState: return STATUS_COMPLETE;
    case FaultState: return STATUS_FAULT;
    default: return STATUS_READY;
    }
}

void Machine::reset()
{
    d_state = StartState;
}
