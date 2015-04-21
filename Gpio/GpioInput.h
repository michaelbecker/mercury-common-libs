#ifndef GPIO_INPUT_H__
#define GPIO_INPUT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#include <list>

#include "TaGpio.h"
#include "TaWinAssert.h"

#include "Gpio.h"


using namespace std;



class GpioInputCallbackData {

    private:

        InputChangeCallback Callback;
        void *UserData;
        unsigned int Flags;
        bool IsDirty;
        int IoValue;

    public:

        GpioInputCallbackData(  InputChangeCallback callback,
                                void *userData,
                                unsigned int flags);

        void Execute(int ioValue);
};



class GpioInput : public Gpio
{

    private:

        int CallbackCount;
        list<GpioInputCallbackData *>CallbackList;
        int IoValue;
    
        bool Read();
        virtual bool SetDirection();

    public:

        GpioInput();

        void AddCallback(GpioInputCallbackData *CallbackData);
        void Execute();

        virtual void ToString();
        virtual void LogString();

};



#endif

