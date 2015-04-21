/**
 *  @file   CheckUid.cpp
 *
 *  @brief  Make sure we are root before we run.
 *
 *  Subversion Information
 *      - $HeadURL: svn://fortress/NG-Thermal/Embedded/trunk/linux/common/CheckUid.cpp $
 *      - $Revision: 4876 $
 *      - $Date: 2011-05-09 14:12:17 -0400 (Mon, 09 May 2011) $
 *      - $Author: mbecker $
 *
 *  Copyright 2010 (c), TA Instruments
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


void
CheckUserId(void)
{
    uid_t EffectiveUserId = geteuid();

    if (EffectiveUserId != 0){
        printf("\n\n");
        printf("+---------------------------------------------------+\n");
        printf("| This program must be run as root using sudo or su)|\n");
        printf("|                       Aborting!                   |\n");
        printf("+---------------------------------------------------+\n");
        abort();
    }

}



