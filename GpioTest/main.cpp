#include <stdio.h>
#include <stdlib.h>
#include "LogLib.h"
#include "TaWinAssert.h"
#include "TaGpio.h"


void GPIO_IN_EXCHANGER_FAULT_InputChangeCallback(int ioValue, void *UserData)
{
    ASSERT(UserData == (void *)GPIO_IN_EXCHANGER_FAULT);
    printf("GPIO_IN_EXCHANGER_FAULT: %d\n", ioValue);
}

void GPIO_IN_FAULT_1_InputChangeCallback(int ioValue, void *UserData)
{
    ASSERT(UserData == (void *)GPIO_IN_FAULT_1);
    printf("GPIO_IN_FAULT_1: %d\n", ioValue);
}

void GPIO_IN_FAULT_2_InputChangeCallback(int ioValue, void *UserData)
{
    ASSERT(UserData == (void *)GPIO_IN_FAULT_2);
    printf("GPIO_IN_FAULT_2: %d\n", ioValue);
}

void GPIO_IN_HEATER_FAULT_InputChangeCallback(int ioValue, void *UserData)
{
    ASSERT(UserData == (void *)GPIO_IN_HEATER_FAULT);
    printf("GPIO_IN_HEATER_FAULT: %d\n", ioValue);
}

void GPIO_IN_SYNC_InputChangeCallback(int ioValue, void *UserData)
{
    ASSERT(UserData == (void *)GPIO_IN_SYNC);
    printf("GPIO_IN_SYNC: %d\n", ioValue);
}

void GPIO_IN_NO_CAN_POWER_InputChangeCallback(int ioValue, void *UserData)
{
    ASSERT(UserData == (void *)GPIO_IN_NO_CAN_POWER);
    printf("GPIO_IN_NO_CAN_POWER: %d\n", ioValue);
}

void GPIO_IN_UART1_DTR_InputChangeCallback(int ioValue, void *UserData)
{
    ASSERT(UserData == (void *)GPIO_IN_UART1_DTR);
    printf("GPIO_IN_UART1_DTR: %d\n", ioValue);
}



int Menu(void)
{
	//----------------------------
    int Selection;
    int rc;
	//----------------------------

	printf("=======================================\n");
	printf("GPIO Tester \n");
	printf("Built on: " __DATE__ " " __TIME__ "\n");
	printf("=======================================\n");
    printf("!) Print Config\n\n");
	printf("x) EXCHANGER_OFF (OFF)\n");
	printf("X) EXCHANGER_OFF (ON)\n\n");
	printf("h) HEATER_OFF    (OFF)\n");
	printf("H) HEATER_OFF    (ON)\n\n");
	printf("e) EVENT         (OFF)\n");
	printf("E) EVENT         (ON)\n\n");
	printf("g) GAS           (OFF)\n");
	printf("G) GAS           (ON)\n\n");
	printf("r) RED_LED       (OFF)\n");
	printf("R) RED_LED       (ON)\n\n");

	printf("q) Quit\n");
	printf("=> ");

REPEAT:    
	Selection = getchar();
    if ((Selection == '\n') || (Selection == '\r'))
        goto REPEAT;

	printf("\n");
	sleep(1);

	switch(Selection){

        case '!':
            TaGpioPrintConfig();
            break;

		case 'x':
            rc = TaGpioWrite(GPIO_OUT_EXCHANGER_OFF, 0);
            ASSERT(rc == 0);
            break;

		case 'X':
            rc = TaGpioWrite(GPIO_OUT_EXCHANGER_OFF, 1);
            ASSERT(rc == 0);
            break;


		case 'h':
            rc = TaGpioWrite(GPIO_OUT_HEATER_OFF, 0);
            ASSERT(rc == 0);
            break;

		case 'H':
            rc = TaGpioWrite(GPIO_OUT_HEATER_OFF, 1);
            ASSERT(rc == 0);
            break;


		case 'e':
            rc = TaGpioWrite(GPIO_OUT_EVENT, 0);
            ASSERT(rc == 0);
            break;

		case 'E':
            rc = TaGpioWrite(GPIO_OUT_EVENT, 1);
            ASSERT(rc == 0);
            break;


		case 'g':
            rc = TaGpioWrite(GPIO_OUT_GAS, 0);
            ASSERT(rc == 0);
            break;

		case 'G':
            rc = TaGpioWrite(GPIO_OUT_GAS, 1);
            ASSERT(rc == 0);
            break;


		case 'r':
            rc = TaGpioWrite(GPIO_OUT_RED_LED, 0);
            ASSERT(rc == 0);
            break;

		case 'R':
            rc = TaGpioWrite(GPIO_OUT_RED_LED, 1);
            ASSERT(rc == 0);
            break;


        case 'q':
		case 'Q':
            return 0;

		default:
			break;
    }

    return 1;
}


int main (void)
{
    int rc;

    InitLog();

    printf("Running GPIO Test\n");


    rc = TaGpioInit("gpio-bd.conf", 1000);
    ASSERT(rc == 0);


    rc = TaGpioRegisterInputChangeCallback( GPIO_IN_EXCHANGER_FAULT, 
            GPIO_IN_EXCHANGER_FAULT_InputChangeCallback, 
            (void *)GPIO_IN_EXCHANGER_FAULT,
            0);
    ASSERT(rc == 0);


    rc = TaGpioRegisterInputChangeCallback( GPIO_IN_FAULT_1, 
            GPIO_IN_FAULT_1_InputChangeCallback, 
            (void *)GPIO_IN_FAULT_1,
            0);
    ASSERT(rc == 0);


    rc = TaGpioRegisterInputChangeCallback( GPIO_IN_FAULT_2, 
            GPIO_IN_FAULT_2_InputChangeCallback, 
            (void *)GPIO_IN_FAULT_2,
            0);
    ASSERT(rc == 0);


    rc = TaGpioRegisterInputChangeCallback( GPIO_IN_HEATER_FAULT, 
            GPIO_IN_HEATER_FAULT_InputChangeCallback, 
            (void *)GPIO_IN_HEATER_FAULT,
            0);
    ASSERT(rc == 0);


    rc = TaGpioRegisterInputChangeCallback( GPIO_IN_SYNC, 
            GPIO_IN_SYNC_InputChangeCallback, 
            (void *)GPIO_IN_SYNC,
            0);
    ASSERT(rc == 0);


    rc = TaGpioRegisterInputChangeCallback( GPIO_IN_NO_CAN_POWER, 
            GPIO_IN_NO_CAN_POWER_InputChangeCallback, 
            (void *)GPIO_IN_NO_CAN_POWER,
            0);
    ASSERT(rc == 0);


    rc = TaGpioRegisterInputChangeCallback( GPIO_IN_UART1_DTR, 
            GPIO_IN_UART1_DTR_InputChangeCallback, 
            (void *)GPIO_IN_UART1_DTR,
            0);
    ASSERT(rc == 0);


    int Continue = 1;
	//
	//	Run the menu app.
	//
    while (Continue){
		Continue = Menu();
    }


    return 0;
}



