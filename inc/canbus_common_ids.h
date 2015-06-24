/**
 *  @file   canbus_common_ids.h
 *
 *  @brief  Public API listing out the CANbus IDs that all 
 *          nodes need to know about.
 *  
 *  Subversion Information
 *      - $HeadURL: svn://fortress/mercury/common/trunk/include/canbus_common_ids.h $
 *      - $Revision: 1541 $
 *      - $Date: 2014-07-01 15:43:03 -0400 (Tue, 01 Jul 2014) $
 *      - $Author: mbecker $
 *
 *  The required IDs are BROADCAST, PC_SERVICE_TOOL, and TEST_ID.  
 *  If more are required (which is highly unlikely), it is anticipated that 
 *  we will work up from the bottom.  
 */
#ifndef CANBUS_COMMON_IDS_H__
#define CANBUS_COMMON_IDS_H__


#define CAN_ID_CORTEX           (0)
//
//  Addresses 1 - 27 available.
//
#define CAN_ID_CBOOT            (28)
#define CAN_ID_TEST_ID          (29)
#define CAN_ID_PC_SERVICE_TOOL  (30)
#define CAN_ID_BROADCAST        (31)


#endif

