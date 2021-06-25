#ifndef XMLSTATEMACHINE_H
#define XMLSTATEMACHINE_H

#include <QObject>
#include <QMap>

namespace SWU {

// Number of machine states
static const size_t N_Machine_States = 18;
static const signed int StartState = 0;
static const signed int FinalState = 15;
static const signed int FaultState = 17;

// Enumeration of possible machine statuses
enum Status {
    STATUS_OK,
    STATUS_FAULT,
    STATUS_COMPLETE
};

// Enumeration of possible transition for the XML state machine validator
enum Transition {
    TRANSITION_SWUPDATE_OPEN = 0,
    TRANSITION_SWUPDATE_CLOSE,
    TRANSITION_MEDIA_OPEN,
    TRANSITION_MEDIA_CLOSE,
    TRANSITION_MEDIA_PATH_OPEN,
    TRANSITION_MEDIA_PATH_CLOSE,
    TRANSITION_FILE_MATCH_OPEN,
    TRANSITION_FILE_MATCH_CLOSE,
    TRANSITION_UPDATE_VALIDATE_OPEN,
    TRANSITION_UPDATE_VALIDATE_CLOSE,
    TRANSITION_EXPECT_OPEN,
    TRANSITION_EXPECT_CLOSE,
    TRANSITION_UPDATE_OPEN,
    TRANSITION_UPDATE_CLOSE,
    TRANSITION_COPY_OPEN,
    TRANSITION_COPY_CLOSE,
    TRANSITION_DIRECTORY_OPEN,
    TRANSITION_DIRECTORY_CLOSE,
    TRANSITION_TO_DIRECTORY_OPEN,
    TRANSITION_TO_DIRECTORY_CLOSE,
    TRANSITION_FILE_OPEN,
    TRANSITION_FILE_CLOSE,
    TRANSITION_REMOVE_OPEN,
    TRANSITION_REMOVE_CLOSE,

    // The number of transitions
    TRANSITION_ENUM_MAX
};

class Machine : public QObject
{
    Q_OBJECT

private:

    // Internal state
    unsigned short d_state = 0;

    // Hashmap mapping input tokens to transitions
    QMap<QString, Transition> d_token_hash_map;

    // Internal state initialisation
    void init_states ();

public:
    explicit Machine(QObject *parent = nullptr);

    // Feed a enumeration-type transition to the machine
    Status input (Transition t);

    // Feed a string-type transition to the machine
    Status input (QString t, Transition *t_ptr = nullptr);

    // Returns the current machine status
    Status status ();

    // Reset the machine to its initial state
    void reset ();
signals:

};


}

#endif // XMLSTATEMACHINE_H
