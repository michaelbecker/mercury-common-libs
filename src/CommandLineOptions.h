/**
 *  @file   CommandLineOptions.h
 *
 *  @brief  API to read in command line arguments
 *
 *  Subversion Information
 *      - $HeadURL: svn://fortress/NG-Thermal/Embedded/trunk/linux/usbserver/svr/CommandLineOptions.h $
 *      - $Revision: 11633 $
 *      - $Date: 2012-09-06 17:03:44 -0400 (Thu, 06 Sep 2012) $
 *      - $Author: mbecker $
 *
 *  Copyright 2010 (c), TA Instruments
 *
 *  @see http://www.gnu.org/s/libc/manual/html_node/Getopt-Long-Options.html#Getopt-Long-Options
 *
 */
#ifndef __COMMAND_LINE_OPTIONS_H__
#define __COMMAND_LINE_OPTIONS_H__

void ParseCommandLineOptions(int argc, char *argv[]);

extern bool RunInForeground;

#endif


