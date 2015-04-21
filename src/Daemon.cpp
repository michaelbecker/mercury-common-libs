/**
 *  @file   Daemon.cpp
 *
 *  @brief  Helper functions to turn a process into a unix daemon.
 *
 *  Subversion Information
 *      - $HeadURL: svn://fortress/NG-Thermal/Embedded/trunk/linux/usbserver/svr/Daemon.cpp $
 *      - $Revision: 11633 $
 *      - $Date: 2012-09-06 17:03:44 -0400 (Thu, 06 Sep 2012) $
 *      - $Author: mbecker $
 *
 *  Copyright 2011 (c), TA Instruments
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>
#include <fcntl.h>



/**
 *  If the call to DaemonInit() was successful, this should be called next, to 
 *  completely disassociate the process from the console.
 */
static void 
CloseAllFileHandles(void)
{
    //---------------
    int i;
    int openmax;
    //---------------


    if ( (openmax = sysconf(_SC_OPEN_MAX)) < 0){
        openmax = 256;
    }

    for (i = 0; i < openmax; i++){
        (void)close(i);
    }

    /* redirect stdin, stdout, and stderr to /dev/null */
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
}



/**
 *  @brief  Call to turn the process into a daemon process.
 *
 *  From Ch 13, Program 13.1, "Advanced Programming in the Unix Environment"
 */
int
DaemonInit(void)
{
    //------------------
    pid_t   pid;
    //------------------

    //
    //  Just get out on errors
    //
    if ((pid = fork()) < 0){
        return (-1);
    }
    //
    //  We want to be a daemon, let the parent die.
    //
    else if (pid != 0){
        exit (0);
    }
    //
    //  We are the child, keep going.
    //  
    else{
        //  
        //  Become session leader
        //
        setsid();   

        //
        //  Clear out file mode creation (do we need this?)
        //
        umask(0);

        CloseAllFileHandles();

        //chdir("/TABin/usbsvr");

        //
        //  All done!
        //
        return (0);
    }
}




