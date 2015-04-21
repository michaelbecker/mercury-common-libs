#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "Gpio.h"
#include "LogLib.h"



Gpio::Gpio()
    : PinNumber(0), 
      ActiveLow(0), 
      ValuePath(NULL), 
      DirectionPath(NULL), 
      ActiveLowPath(NULL),
      Name(NULL),
      Valid(false)
{}


bool 
Gpio::SetSense()
{
    //--------------------
    int fd;
    int rc;
    char Buffer[10];
    int length;
    //--------------------

    fd = open(ActiveLowPath, O_WRONLY);
    if (fd < 0){
        LogMessage( "Failed %s SetSense open(%s) - pin %d", 
                    Name, ActiveLowPath, PinNumber);
        return false;
    }

    length = sprintf(Buffer, "%d\n", ActiveLow);

    rc = write(fd, Buffer, length);

    close(fd);

    if (rc != length){
        LogMessage( "Failed %s SetSense write(%s, %s) - pin %d", 
                    Name, ActiveLowPath, Buffer, PinNumber);
        return false;
    }
    else{
        return true;
    }
}


bool
Gpio::Export()
{
    //------------------------
    struct stat st;
    int rc;
    int fd;
    char Buffer[10];
    int length;
    //------------------------

    rc = stat(ValuePath, &st);
    if (rc == 0){
        return true;
    }

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0){
        LogMessage( "Failed %s Export open(/sys/class/gpio/export) - pin %d", 
                    Name, PinNumber);
        return false;
    }
    
    length = sprintf(Buffer, "%d\n", PinNumber);

    rc = write(fd, Buffer, length);

    close(fd);

    if (rc != length){
        LogMessage( "Failed %s Export write(/sys/class/gpio/export, %s) - pin %d", 
                    Name, Buffer, PinNumber);
        return false;
    }
    else{
        return true;
    }
}



void 
Gpio::Init(char *pin, char *activeLow, char *name)
{
    //--------------------
    char Buffer[512];
    bool Success;
    //--------------------
    
    PinNumber = atoi(pin);
    ActiveLow = atoi(activeLow);
    ASSERT(PinNumber != 0);

    sprintf(Buffer, "/sys/class/gpio/gpio%d/value", PinNumber);
    ValuePath = strdup(Buffer);
    sprintf(Buffer, "/sys/class/gpio/gpio%d/direction", PinNumber);
    DirectionPath = strdup(Buffer);
    sprintf(Buffer, "/sys/class/gpio/gpio%d/active_low", PinNumber);
    ActiveLowPath = strdup(Buffer);

    Name = strdup(name);

    Success = Export();
    if (!Success){
        return;
    }

    Success = SetSense();
    if (!Success){
        return;
    }

    Success = SetDirection();
    if (!Success){
        return;
    }

    Valid = true;
}



bool 
Gpio::IsValid()
{
    return Valid;
}


