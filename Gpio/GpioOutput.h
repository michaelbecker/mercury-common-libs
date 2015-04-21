#ifndef GPIO_OUTPUT_H__
#define GPIO_OUTPUT_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#include "TaGpio.h"
#include "TaWinAssert.h"

#include "Gpio.h"


class GpioOutput : public Gpio
{
    private:
        int DefaultValue;
        virtual bool SetDirection();

    public:
        bool Write(int value);

        virtual void ToString();
        virtual void LogString();

        void Init(char *pin, char *activeLow, char *name, char *defaultValue);
};


#endif

