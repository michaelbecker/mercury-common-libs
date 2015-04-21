#include "SystemEvent.h"
#include "TaWinAssert.h"
#include "Version.h"



void
SystemEvent::Subscribe(SystemEventObserver *Observer)
{
    pthread_mutex_lock(&Lock);

    Observers.push_front(Observer);

    pthread_mutex_unlock(&Lock);
}



void
SystemEvent::Unsubscribe(SystemEventObserver *Observer)
{
    pthread_mutex_lock(&Lock);

    Observers.remove(Observer);

    pthread_mutex_unlock(&Lock);
}



void
SystemEvent::Subscribe(SystemEventHandler_t StaticHandler, void *UserData)
{
    StaticSystemEventObservers_t s(StaticHandler, UserData);

    pthread_mutex_lock(&Lock);

    StaticObservers.push_front(s);

    pthread_mutex_unlock(&Lock);
}



void
SystemEvent::Unsubscribe(SystemEventHandler_t StaticHandler)
{
    //------------------------------------------------------
    list<StaticSystemEventObservers_t>::iterator i;
    //------------------------------------------------------

    pthread_mutex_lock(&Lock);

    for (i = StaticObservers.begin() ; i != StaticObservers.end() ; ){

        StaticSystemEventObservers_t Element = *i;

        if (Element.first == StaticHandler){
            i = StaticObservers.erase(i);
        }
        else{
            i++;
        }
    }

    pthread_mutex_unlock(&Lock);
}



void
SystemEvent::RaiseEvent()
{
    //------------------------------------------------------
    list<SystemEventObserver *>::iterator i;
    list<StaticSystemEventObservers_t>::iterator j;
    //------------------------------------------------------

    pthread_mutex_lock(&Lock);

    for (i = Observers.begin(); i != Observers.end(); i++){
        SystemEventObserver *Element = *i;
        Element->EventHandler(this);
    }

    for (j = StaticObservers.begin(); j != StaticObservers.end(); j++){
        StaticSystemEventObservers_t Element = *j;
        Element.first(Element.second, this);
    }

    pthread_mutex_unlock(&Lock);
}



SystemEvent::SystemEvent(int id)
:   Id(id)
{
    //----------------------------
    pthread_mutexattr_t attr;
    int rc;
    static bool LogVersion = true;
    //----------------------------

    rc = pthread_mutexattr_init(&attr);
    ASSERT(rc == 0);

    rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ASSERT(rc == 0);
    
    rc = pthread_mutex_init(&Lock, &attr);
    ASSERT(rc == 0);
    
    rc = pthread_mutexattr_destroy(&attr);
    ASSERT(rc == 0);

    //
    //  Not thread safe if you try realllly hard, but in that case we 
    //  just get 2 messages, not a huge deal.
    //
    if (LogVersion){
        LogVersion = false;
        LOG_LIBRARY_VERSION();
    }
}



SystemEvent::~SystemEvent()
{
    pthread_mutex_lock(&Lock);

    Observers.clear();
    StaticObservers.clear();
    
    pthread_mutex_unlock(&Lock);

    pthread_mutex_destroy(&Lock);
}



int 
SystemEvent::GetSystemEventId()
{
    return Id;
}



