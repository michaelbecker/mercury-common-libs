#ifndef TA_GPIO_H__
#define TA_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif


/**
 *  Abstract names for the INPUT GPIO on the carrier board.
 */
#define GPIO_IN_EXCHANGER_FAULT     0
#define GPIO_IN_FAULT_1             1
#define GPIO_IN_FAULT_2             2
#define GPIO_IN_HEATER_FAULT        3
#define GPIO_IN_SYNC                4
#define GPIO_IN_NO_CAN_POWER        5
#define GPIO_IN_UART1_DTR           6
#define NUM_GPIO_INPUTS             7

/**
 *  Abstract names for the OUTPUT GPIO on the carrier board.
 */
#define GPIO_OUT_EXCHANGER_OFF      0
#define GPIO_OUT_HEATER_OFF         1
#define GPIO_OUT_EVENT              2
#define GPIO_OUT_GAS                3
#define GPIO_OUT_RED_LED            4
#define NUM_GPIO_OUTPUTS            5


/**
 *  @param ConfigFile, for the board.
 *  @param MonitorPeriodInMs, the period we want to 
 *         pool IO for, 0 for default.
 *
 *  @returns 0 on success, -1 on error.
 */
int
TaGpioInit( char *ConfigFile,
            int MonitorPeriodInMs
            );


/**
 *  @param  ioValue, tells you whether the logic level high (1) or low (0).
 *          If you receive a (-1), this is an error condition!
 *
 *  @param  UserData, what you passed into the 
 *          TaGpioRegisterInputChangeCallback() function.
 */
typedef void (*InputChangeCallback)(int ioValue, void *UserData);


/**
 *  Pass in a GPIO_IN_Xxx value and your callback and data.
 *
 *  @returns 0 on success, -1 on error.
 */
int
TaGpioRegisterInputChangeCallback(  int GpioInput, 
                                    InputChangeCallback Callback,
                                    void *UserData,
                                    unsigned int Flags);


/**
 *  Pass in a GPIO_OUT_Xxx value and whether you want it logic level
 *  high (1) or low (0).  
 *
 *  @returns 0 on success, -1 on error.
 */
int 
TaGpioWrite(int GpioOutput, int value);



/**
 *  Debug functions
 */
void
TaGpioLogConfig(void);

void
TaGpioPrintConfig(void);


#ifdef __cplusplus
}
#endif


#endif


