#include "xmlstatemachine.h"

using namespace SWU;


/*
 *******************************************************************************
 *                         Static variable definitions                         *
 *******************************************************************************
*/


// State machine map (init to invalid states)
static signed short map[N_Machine_States][TRANSITION_ENUM_MAX] = {{FaultState}};

// Array designation map (used when initialising hashmap)
static QString transition_string_map[TRANSITION_ENUM_MAX] = {
    [TRANSITION_SWUPDATE_OPEN]         = QString("<swupdate>"),
    [TRANSITION_SWUPDATE_CLOSE]        = QString("</swupdate>"),
    [TRANSITION_MEDIA_OPEN]            = QString("<media>"),
    [TRANSITION_MEDIA_CLOSE]           = QString("</media>"),
    [TRANSITION_MEDIA_PATH_OPEN]       = QString("<media-path>"),
    [TRANSITION_MEDIA_PATH_CLOSE]      = QString("</media-path>"),
    [TRANSITION_FILE_MATCH_OPEN]       = QString("<file-match>"),
    [TRANSITION_FILE_MATCH_CLOSE]      = QString("</file-match>"),
    [TRANSITION_UPDATE_VALIDATE_OPEN]  = QString("<update-validate>"),
    [TRANSITION_UPDATE_VALIDATE_CLOSE] = QString("</update-validate>"),
    [TRANSITION_EXPECT_OPEN]           = QString("<expect>"),
    [TRANSITION_EXPECT_CLOSE]          = QString("</expect>"),
    [TRANSITION_UPDATE_OPEN]           = QString("<update>"),
    [TRANSITION_UPDATE_CLOSE]          = QString("</update>"),
    [TRANSITION_COPY_OPEN]             = QString("<copy>"),
    [TRANSITION_COPY_CLOSE]            = QString("</copy>"),
    [TRANSITION_DIRECTORY_OPEN]        = QString("<directory>"),
    [TRANSITION_DIRECTORY_CLOSE]       = QString("</directory>"),
    [TRANSITION_TO_DIRECTORY_OPEN]     = QString("<to-directory>"),
    [TRANSITION_TO_DIRECTORY_CLOSE]    = QString("</to-directory>"),
    [TRANSITION_FILE_OPEN]             = QString("<file>"),
    [TRANSITION_FILE_CLOSE]            = QString("</file>"),
    [TRANSITION_REMOVE_OPEN]           = QString("<remove>"),
    [TRANSITION_REMOVE_CLOSE]          = QString("</remove>")
};

// Maps a transition to a human-readable string form
const char *g_map_transition_str[TRANSITION_ENUM_MAX] =
{
    [TRANSITION_SWUPDATE_OPEN]         = "SWUpdate",
    [TRANSITION_SWUPDATE_CLOSE]        = "SWUpdate",
    [TRANSITION_MEDIA_OPEN]            = "Media",
    [TRANSITION_MEDIA_CLOSE]           = "Media",
    [TRANSITION_MEDIA_PATH_OPEN]       = "Media-path",
    [TRANSITION_MEDIA_PATH_CLOSE]      = "Media-path",
    [TRANSITION_FILE_MATCH_OPEN]       = "File-match",
    [TRANSITION_FILE_MATCH_CLOSE]      = "File-match",
    [TRANSITION_UPDATE_VALIDATE_OPEN]  = "Update-validate",
    [TRANSITION_UPDATE_VALIDATE_CLOSE] = "Update-validate",
    [TRANSITION_EXPECT_OPEN]           = "Expect",
    [TRANSITION_EXPECT_CLOSE]          = "Expect",
    [TRANSITION_UPDATE_OPEN]           = "Update",
    [TRANSITION_UPDATE_CLOSE]          = "Update",
    [TRANSITION_COPY_OPEN]             = "Copy",
    [TRANSITION_COPY_CLOSE]            = "Copy",
    [TRANSITION_DIRECTORY_OPEN]        = "Directory",
    [TRANSITION_DIRECTORY_CLOSE]       = "Directory",
    [TRANSITION_TO_DIRECTORY_OPEN]     = "To-directory",
    [TRANSITION_TO_DIRECTORY_CLOSE]    = "To-directory",
    [TRANSITION_FILE_OPEN]             = "File",
    [TRANSITION_FILE_CLOSE]            = "File",
    [TRANSITION_REMOVE_OPEN]           = "Remove",
    [TRANSITION_REMOVE_CLOSE]          = "Remove"
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
    for (off_t t = 0; t < TRANSITION_ENUM_MAX; ++t) {
        d_token_hash_map[transition_string_map[t]] = static_cast<Transition>(t);
    }
}

void Machine::init_states ()
{
    // State 0
    map[0][TRANSITION_SWUPDATE_OPEN] = 1;

    // State 1
    map[1][TRANSITION_MEDIA_OPEN] = 2;
    map[1][TRANSITION_UPDATE_VALIDATE_OPEN] = 5;
    map[1][TRANSITION_UPDATE_OPEN] = 7;
    map[1][TRANSITION_SWUPDATE_CLOSE] = 15;

    // State 2
    map[2][TRANSITION_MEDIA_CLOSE] = 1;
    map[2][TRANSITION_MEDIA_PATH_OPEN] = 3;
    map[2][TRANSITION_FILE_MATCH_OPEN] = 4;

    // State 3
    map[3][TRANSITION_MEDIA_PATH_CLOSE] = 2;

    // State 4
    map[4][TRANSITION_FILE_MATCH_CLOSE] = 2;

    // State 5
    map[5][TRANSITION_UPDATE_VALIDATE_CLOSE] = 1;
    map[5][TRANSITION_EXPECT_OPEN] = 6;

    // State 6
    map[6][TRANSITION_EXPECT_CLOSE] = 5;

    // State 7
    map[7][TRANSITION_UPDATE_CLOSE] = 1;
    map[7][TRANSITION_COPY_OPEN] = 8;
    map[7][TRANSITION_REMOVE_OPEN] = 16;

    // State 8
    map[8][TRANSITION_DIRECTORY_OPEN] = 9;
    map[8][TRANSITION_COPY_CLOSE] = 7;
    map[8][TRANSITION_FILE_OPEN] = 12;

    // State 9
    map[9][TRANSITION_DIRECTORY_CLOSE] = 10;

    // State 10
    map[10][TRANSITION_TO_DIRECTORY_OPEN] = 11;

    // State 11
    map[11][TRANSITION_TO_DIRECTORY_CLOSE] = 8;

    // State 12
    map[12][TRANSITION_FILE_CLOSE] = 13;

    // State 13
    map[13][TRANSITION_TO_DIRECTORY_OPEN] = 14;

    // State 14
    map[14][TRANSITION_TO_DIRECTORY_CLOSE] = 8;

    // State 15 (end state - no transitions)

    // State 16
    map[16][TRANSITION_REMOVE_CLOSE] = 7;
}

Status Machine::input(Transition t)
{
    signed short next_state = d_state;
    next_state = map[d_state][t];
    d_state = next_state;
    return status();
}

Status Machine::input(QString t, Transition *t_ptr)
{
    // Lookup the token for the lexeme
    QMap<QString, Transition>::const_iterator iter = d_token_hash_map.find(t);
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
    default: return STATUS_OK;
    }
}

void Machine::reset()
{
    d_state = StartState;
}
