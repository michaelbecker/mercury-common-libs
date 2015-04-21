#include <stdio.h>
#include "LogLib.h"
#include "SystemEvent.h"


class DerivedSystemEvent : public SystemEvent {

    public:
        
        DerivedSystemEvent(int id) : SystemEvent(id)
        {}

        void ToString()
        {
            printf("DerivedSystemEvent(%d).ToString\n", GetSystemEventId());
        }
};



void StaticHandler1(void *UserData, SystemEvent *Event)
{
    printf("In StaticHandler1 - ");
    printf("%p %p(%d)\n", UserData, Event, Event->GetSystemEventId());
}

void StaticHandler2(void *UserData, SystemEvent *Event)
{
    printf("In StaticHandler2 - ");
    printf("%p %p(%d)\n", UserData, Event, Event->GetSystemEventId());
}

void StaticHandler3(void *UserData, SystemEvent *Event)
{
    printf("In StaticHandler3 - ");
    printf("%p %p(%d)\n", UserData, Event, Event->GetSystemEventId());
}

void StaticHandler4(void *UserData, SystemEvent *Event)
{
    printf("In StaticHandler4 - ");
    printf("%p %p(%d)\n", UserData, Event, Event->GetSystemEventId());
}

void StaticHandler5(void *UserData, SystemEvent *Event)
{
    printf("In StaticHandler5 - ");
    printf("%p %p(%d)\n", UserData, Event, Event->GetSystemEventId());
    ((DerivedSystemEvent *)Event)->ToString();
}



class MyObserver1 : public SystemEventObserver {

    public:
        virtual void EventHandler(SystemEvent *Event){
            printf("In MyObserver1.EventHandler(%d) - ", Event->GetSystemEventId());
            printf("%p\n", Event);
        };
};

class MyObserver2 : public SystemEventObserver {

    public:
        virtual void EventHandler(SystemEvent *Event){
            printf("In MyObserver2.EventHandler(%d) - ", Event->GetSystemEventId());
            printf("%p\n", Event);
        };
};




int main(int argc, char* argv[])
{
    (void)argv;
    (void)argc;

    InitLog();

    SystemEvent sysEvent1(1);
    SystemEvent sysEvent2(2);
    DerivedSystemEvent dSysEvent1(3);

    sysEvent1.Subscribe(StaticHandler1, (void *)0x11111111);
    sysEvent1.Subscribe(StaticHandler2, (void *)0x22222222);
    sysEvent1.Subscribe(StaticHandler3, (void *)0x33333333);
    sysEvent1.Subscribe(StaticHandler3, (void *)0x30303030);

    sysEvent2.Subscribe(StaticHandler3, (void *)0x33333333);
    sysEvent2.Subscribe(StaticHandler4, (void *)0x44444444);

    dSysEvent1.Subscribe(StaticHandler5, NULL);

    sysEvent1.RaiseEvent();
    sysEvent2.RaiseEvent();
    dSysEvent1.RaiseEvent();
    printf("\n");

    sysEvent1.Unsubscribe(StaticHandler3);

    sysEvent1.RaiseEvent();
    sysEvent2.RaiseEvent();
    dSysEvent1.RaiseEvent();
    printf("\n");

    sysEvent1.Unsubscribe(StaticHandler1);
    sysEvent1.Unsubscribe(StaticHandler2);
    sysEvent1.Unsubscribe(StaticHandler1);
    sysEvent1.Unsubscribe(StaticHandler3);

    sysEvent1.RaiseEvent();
    sysEvent2.RaiseEvent();
    dSysEvent1.RaiseEvent();
    printf("\n");

    MyObserver1 ob1;
    MyObserver2 ob2;

    sysEvent1.Subscribe(&ob1);
    sysEvent1.Subscribe(&ob2);

    sysEvent1.RaiseEvent();
    sysEvent2.RaiseEvent();
    dSysEvent1.RaiseEvent();
    printf("\n");

	return 0;
}


