/**
 *  @file   CommandLineOptions.cpp
 *
 *  @brief  API to read in command line arguments
 *
 *  Subversion Information
 *      - $HeadURL: svn://fortress/NG-Thermal/Embedded/trunk/linux/usbserver/svr/CommandLineOptions.cpp $
 *      - $Revision: 11633 $
 *      - $Date: 2012-09-06 17:03:44 -0400 (Thu, 06 Sep 2012) $
 *      - $Author: mbecker $
 *
 *  Copyright 2010 (c), TA Instruments
 *
 *  @see http://www.gnu.org/s/libc/manual/html_node/Getopt-Long-Options.html#Getopt-Long-Options
 *
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "CommandLineOptions.h"


//
//  Options that look like --long-name1 or --long-name2 <val>
//
static struct option LongOptions[] = 
{
    {   "help",          no_argument,       0,   '?'},
    {   "foreground",    no_argument,       0,   'f'},

    //
    //  End of table marker
    //
    {0, 0, 0, 0}
};



/**
 *  Wrapper function to get all of the command line options.
 */
void ParseCommandLineOptions(int argc, char *argv[])
{
    //-----------------------------
    int OptionIndex = 0;
    int c;
    struct option *OptionList;
    //-----------------------------

    //
    //  Keep going until we're done
    //
    while (1){

        c = getopt_long(    argc, 
                            argv, 
                            "",     // Don't bother with short options.
                            LongOptions, 
                            &OptionIndex);
        switch (c){

            //
            //  -1 means we are all done.
            //
            case -1:
                return;

            case 0:
                /* If this option set a flag, do nothing else now. */
                break;
     
            case 'f':
                RunInForeground = true;
                break;

            case '?':
                OptionList = LongOptions;
                printf("Supported options:\n");
                while (OptionList->name  != 0){
                    printf("  --%s\n", OptionList->name);
                    OptionList++;
               }
               exit (1);
     
            default:
                abort ();
        }
    }
}



