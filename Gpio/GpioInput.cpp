#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "GpioInput.h"


/****************************************************************************
 *
 *                      GpioInputCallbackData Class
 *
 ****************************************************************************
 */
GpioInputCallbackData::GpioInputCallbackData(   InputChangeCallback callback,
                                                void *userData,
                                                unsigned int flags)
    : Callback(callback), 
      UserData(userData),
      Flags(flags),
      IsDirty(true),
      IoValue(0)
{}


void 
GpioInputCallbackData::Execute(int ioValue)
{
    if (IsDirty){
        IoValue = ioValue;
        IsDirty = false;
        Callback(ioValue, UserData);
    }
    else if (IoValue != ioValue){
        IoValue = ioValue;
        Callback(ioValue, UserData);
    }
}



/****************************************************************************
 *
 *                          GpioInput Class
 *
 ****************************************************************************
 */
bool
GpioInput::Read()
{
    int fd;
    int bytes_read;
    char Buffer[3] = {0};

    fd = open(ValuePath, O_RDONLY);
    if (fd < 0){
        LogMessage( "Failed %s Read open(%s) - pin %d", 
                    Name, ValuePath, PinNumber);
        return false;
    }

    bytes_read = read(fd, Buffer, sizeof(Buffer));

    close(fd);

    if (bytes_read <= 0){
        LogMessage( "Failed %s Read read(%s) = %d - pin %d", 
                    Name, ValuePath, bytes_read, PinNumber);
        return false;
    }

    IoValue = atoi(Buffer);

    return true;
}



bool 
GpioInput::SetDirection()
{
    //------------------------------
    int fd;
    int rc;
    const char *InString = "in\n";
    //------------------------------

    fd = open(DirectionPath, O_WRONLY);
    if (fd < 0){
        LogMessage( "Failed %s SetDirection open(%s) - pin %d", 
                    Name, DirectionPath, PinNumber);
        return false;
    }

    rc = write(fd, InString, 4);

    close(fd);

    if (rc != 4){
        LogMessage( "Failed %s SetDirection write(%s, %s) - pin %d", 
                    Name, DirectionPath, InString, PinNumber);
        return false;
    }
    else{
        return true;
    }
}



GpioInput::GpioInput() 
    : Gpio(),
      CallbackCount(0)
{}



void 
GpioInput::AddCallback(GpioInputCallbackData *CallbackData)
{
    CallbackList.push_front(CallbackData);
    CallbackCount++;
}



void 
GpioInput::Execute()
{
    if (CallbackCount){

        bool Success = Read();

        if (!Success){
            IoValue = -1;
        }

        for (list<GpioInputCallbackData *>::iterator it = CallbackList.begin();
                it != CallbackList.end();
                ++it){
            
            GpioInputCallbackData *Callback = *it;
            Callback->Execute(IoValue);
        }
    }
}



void 
GpioInput::ToString()
{
    printf("%s\n\tPin: %d\n\tActiveLow: %d\n"
            "\tValue: %s\n"
            "\tDirection: %s\n"
            "\tActiveLow: %s\n",
            Name,
            PinNumber,
            ActiveLow,
            ValuePath,
            DirectionPath,
            ActiveLowPath);
}



void 
GpioInput::LogString()
{
    LogMessage("%s\n\tPin: %d\n\tActiveLow: %d\n"
            "\tValue: %s\n"
            "\tDirection: %s\n"
            "\tActiveLow: %s\n",
            Name,
            PinNumber,
            ActiveLow,
            ValuePath,
            DirectionPath,
            ActiveLowPath);
}


