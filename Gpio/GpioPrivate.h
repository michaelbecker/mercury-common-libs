#ifndef GPIO_PRIVATE_H__
#define GPIO_PRIVATE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#include <list>

#include "TaGpio.h"
#include "TaWinAssert.h"
#include "GpioInput.h"
#include "GpioOutput.h"



extern GpioInput GpioInputs[NUM_GPIO_INPUTS];
extern GpioOutput GpioOutputs[NUM_GPIO_OUTPUTS];


int 
ParseConfigFile(char *ConfigFile);

void
PrintGpioConfig(void);



#endif

