//$COMMON.CPP$                   
//   File Name:  StateMachine.cpp
//   Copyright 1997 Inventive Technologies,Inc. All Rights Reserved.
//
//   Purpose: Provides state machine transition handling.
//
//   Interfaces:
//
//   Restrictions:
/******************************************************************************/

#include "StateMachine.h"

//*****************************************************************************/
//    Operation Name: StateMachine()
//
//    Processing:   Class constructor.  
//
//    Input Parameters:
//
//    Output Parameters:
//
//    Return Values:
//
//    Pre-Conditions:
//
//    Miscellaneous:
//
//    Requirements:  
//
//*****************************************************************************/
StateMachine::StateMachine(void) 
{
    CurrentState = 0;
    LastState = 0;
    EventGenerated = eFalse;
    pEventData = NULL;
}    

//*****************************************************************************/
//    Operation Name: StateEngine()
//
//    Processing: 
//      Will continue to call the derived class's DoStateAction method while
//      events are being generated. 
//
//      The StateEngine handles both external and internal events. External 
//      events being an unsolicited event, or function call. Internal events
//      are events self generated by the state machine in response to an
//      external event. For instance, an external event comes in and forces a
//      state machine transition to state 2. While executing state 2 the 
//      state machine transitions to state 3, an internal event.
//
//      External and internal events can have event data attached to be decoded
//      by the state upon arrival. All event data structures must inherit from
//      the class EventData and not exceed the size specified by that class. 
//      After the data is used, it will be deleted automatically. 
//
//      The caller to StateEngine() is responsible for deleting the external 
//      event data. Internal event data will be automatically deleted by the
//      state engine.
//
//    Input Parameters:
//
//    Output Parameters:
//
//    Return Values:
//
//    Pre-Conditions:
//
//    Miscellaneous:
//
//    Requirements:  
//
//*****************************************************************************/
void StateMachine::StateEngine(void)
{
    EventData* pDataTemp = NULL;
    E_Bool ExternalEvent = eTrue;

    // WHILE events are being generated
    //      copy event data pointer to temp pointer so we can delete when done
    //      event used up, reset event generated flag
    //      event data used up, reset event data pointer
    //      do required state machine action sending along event data, if any
    //      IF event data was used AND event is not an external event
    //          delete the event data
    //          reset pData to NULL
    //      ELSE
    //          all new events will be internal so we need to delete any data
    while (EventGenerated) 
    {         
        pDataTemp = pEventData;
        EventGenerated = eFalse;
        pEventData = NULL;
        DoStateAction(pDataTemp);
        if (pDataTemp && !ExternalEvent)
        {
//            pDataTemp->Delete();
            pDataTemp = NULL;
        }
        else
            ExternalEvent = eFalse;
    }
}

//*****************************************************************************/
//    Operation Name: NextState()
//
//    Processing: 
//      Used to generate a state transition within the derived class.
//
//    Input Parameters:
//      newState - the new state to transition to.
//      pData - optional event data to send to the new state. This parameter
//          is expected to be an upcast from a derived class/struct of 
//          EventData.
//
//    Output Parameters:
//
//    Return Values:
//
//    Pre-Conditions:
//
//    Miscellaneous:
//
//    Requirements:  
//
//*****************************************************************************/
void StateMachine::NextState(UCHAR newState, EventData* pData) 
{
    pEventData = pData;
    EventGenerated = eTrue;
    LastState = CurrentState;
    CurrentState = newState;
}





