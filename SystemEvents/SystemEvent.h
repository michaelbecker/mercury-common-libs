/**
 *  @file   SystemEvent.h
 *  @brief  Simple implementation generic object oriented events.
 *
 *  This file works with both C style callbacks as well as C++ 
 *  derived classes.
 */
#ifndef SYSTEM_EVENT_H__
#define SYSTEM_EVENT_H__

#include <pthread.h>
#include <list>
#include <map>

using namespace std;

#include "CommonSystemEvents.h"


//
//  Forward delcaration.
//
class SystemEvent;


/**
 *  If you want to register a static method or std C function 
 *  with a SystemEvent object, you need to match this prototype.
 */
typedef void (*SystemEventHandler_t)(void *UserData, SystemEvent *Event);


/**
 *  If you want to register a class with a SystemEvent object, 
 *  you need to implement this interface.
 */
class SystemEventObserver {
    
    public:
        virtual void EventHandler(SystemEvent *Event) = 0;
};



//
//  Convenience typedef for the SystemEvent class.
//  End users should just ignore this.
//
typedef pair<SystemEventHandler_t, void *> StaticSystemEventObservers_t ;



/**
 *  Implements a generic Subject / Observer pattern as a 
 *  stand-alone class.  This class may be subclassed, or not.
 */ 
class SystemEvent {

    public:

        /**
         *  Registers a class implementing the SystemEventObserver
         *  interface.
         */
        void Subscribe(SystemEventObserver *Observer);

        /**
         *  This removes "all" instances of the Observer class from the 
         *  registered list.
         */
        void Unsubscribe(SystemEventObserver *Observer);

        /**
         *  Registers a function of the form 
         *  void "Xxx"(void *UserData, SystemEvent *Event)
         */
        void Subscribe(SystemEventHandler_t StaticHandler, void *UserData);

        /**
         *  This removes "all" instances of the Observer class from the 
         *  registered list.
         */
        void Unsubscribe(SystemEventHandler_t StaticHandler);

        /**
         *  This triggers all of the EventHandler functions 
         *  and members.
         */
        void RaiseEvent();

        /**
         *  Ctor requires an ID, so clients can figure out who 
         *  called them if they register with multiple SystemEvents.
         */
        SystemEvent(int id);

        /**
         *  Retrieve the ID of this SystemEvent.
         */
        int GetSystemEventId();

        /**
         *  It's not anticipated that after you create a 
         *  System Event you will destroy it.
         */
        ~SystemEvent();
    
    private:

        int Id;
        list<SystemEventObserver *>Observers;
        list<StaticSystemEventObservers_t>StaticObservers;
        pthread_mutex_t Lock;
};


#endif

