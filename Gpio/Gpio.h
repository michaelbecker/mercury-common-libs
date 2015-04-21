#ifndef GPIO_H__
#define GPIO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#include <list>

#include "TaGpio.h"
#include "TaWinAssert.h"


class Gpio {

    protected:

        int PinNumber;
        int ActiveLow;
        char *ValuePath;
        char *DirectionPath;
        char *ActiveLowPath;
        char *Name;
        bool Valid;

        Gpio();
        bool Export();
        bool SetSense();

        virtual bool SetDirection() = 0;

    public:

        bool IsValid();
        virtual void ToString() = 0;
        virtual void LogString() = 0;

        /**
         *  If something in this function fails, we log it but do not 
         *  set an error.  We wait to see if we every access this GPIO
         *  before setting something.
         */
        virtual void Init(char *pin, char *activeLow, char *name);
};


#endif 

