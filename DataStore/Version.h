#ifndef VERSION_H__
#define VERSION_H__

#include "LogLib.h"

/**
 *  Macro to add the version to the log.
 *
 *  This can all be in-line, since this is required to be a 
 *  manual step.
 */
#define LOG_LIBRARY_VERSION()  LogMessage("DataStore 1.0.0.0 - Built on " __DATE__)

#endif

