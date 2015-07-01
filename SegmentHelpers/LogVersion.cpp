#include "Version.h"

volatile bool bLogVersion = true;

void
LogVersion()
{
    if (bLogVersion){
        bLogVersion = false;
        LOG_LIBRARY_VERSION();
    }
}


