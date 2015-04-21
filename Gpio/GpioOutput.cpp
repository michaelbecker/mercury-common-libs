#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "GpioOutput.h"


bool 
GpioOutput::SetDirection()
{
    //-----------------------------------
    int fd;
    int rc;
    const char *LowString = "low\n";
    const char *HighString = "high\n";
    int Expected;
    //-----------------------------------

    fd = open(DirectionPath, O_WRONLY);
    if (fd < 0){
        LogMessage( "Failed %s SetDirection open(%s) - pin %d", 
                    Name, DirectionPath, PinNumber);
        return false;
    }

    //
    //  Try to make this glitch free.
    //
    if (DefaultValue){
        Expected = 6;
        rc = write(fd, HighString, Expected);
    }
    else{
        Expected = 5;
        rc = write(fd, LowString, Expected);
    }

    close(fd);

    if (rc != Expected){
        if (DefaultValue){
            LogMessage( "Failed %s SetDirection write(%s, %s) - pin %d", 
                        Name, DirectionPath, HighString, PinNumber);
        }
        else{
            LogMessage( "Failed %s SetDirection write(%s, %s) - pin %d", 
                        Name, DirectionPath, LowString, PinNumber);
        }
        return false;
    }
    else{
        return true;
    }
}



bool 
GpioOutput::Write(int value)
{
    //-----------------------------------
    int fd;
    int rc;
    char Buffer[10];
    int length;
    //-----------------------------------

    fd = open(ValuePath, O_WRONLY);
    if (fd < 0){
        LogMessage( "Failed %s Write open(%s) - pin %d", 
                    Name, ValuePath, PinNumber);
        return false;
    }

    if (value){
        value = 1;
    }

    length = sprintf(Buffer, "%d\n", value);

    rc = write(fd, Buffer, length);

    close(fd);

    if (rc != length){
        LogMessage( "Failed %s Write read(%s) = %d - pin %d", 
                    Name, ValuePath, rc, PinNumber);
        return false;
    }
    else{
        return true;
    }
}



static char *
GetDefaultOutputDirectionValue(int DefaultValue)
{
    if (DefaultValue){
        return "high";
    }
    else{
        return "low";
    }
}



void 
GpioOutput::Init(char *pin, char *activeLow, char *name, char *defaultValue)
{
    DefaultValue = atoi(defaultValue);

    //
    //  The base class sets up the Sense and Direction, calling our 
    //  virtual SetDirection() method.  Make sure any data we need 
    //  for this is setup prior to initializing the base.
    //
    Gpio::Init(pin, activeLow, name);
}


void 
GpioOutput::ToString()
{
    printf( "%s\n\tPin: %d\n\tActiveLow: %d\n"
            "\tValue: %s\n"
            "\tDefault: %s\n"
            "\tDirection: %s\n"
            "\tActiveLow: %s\n",
            Name,
            PinNumber,
            ActiveLow,
            ValuePath,
            GetDefaultOutputDirectionValue(DefaultValue),
            DirectionPath,
            ActiveLowPath);
}



void 
GpioOutput::LogString()
{
    LogMessage( "%s\n\tPin: %d\n\tActiveLow: %d\n"
            "\tValue: %s\n"
            "\tDefault: %s\n"
            "\tDirection: %s\n"
            "\tActiveLow: %s\n",
            Name,
            PinNumber,
            ActiveLow,
            ValuePath,
            GetDefaultOutputDirectionValue(DefaultValue),
            DirectionPath,
            ActiveLowPath);
}


