/**
 * @file
 * OpenM++ common helper utilities.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_I_HELPER_H
#define OM_I_HELPER_H

#include "libopenm/common/omHelper.h"

using namespace std;

// time intervals for interprocess communication and model status update
//
#define OM_SEND_SLEEP_TIME  29L         /* msec, send completion test sleep interval */
#define OM_RECV_SLEEP_TIME  31L         /* msec, receive probe sleep interval */
#define OM_RUN_POLL_TIME 17L            /* msec, wait interval for modeling threads polling */
#define OM_ACTIVE_SLEEP_TIME 37L        /* msec, sleep interval if there any child process activity */
#define OM_WAIT_SLEEP_TIME 971L         /* msec, sleep interval if no progress in modeling threads */
#define OM_STATE_SAVE_TIME 1811L        /* msec, interval between run state save if any changes in run status */

// add file and line number to debug log
//
#ifdef _DEBUG
    #define OM_DBG_STR(line)        #line
    #define OM_DBG_TO_STR(s_line)   OM_DBG_STR(s_line)
    #define OM_FILE_LINE            __FILE__ ":" OM_DBG_TO_STR( __LINE__ )
#else
//    #define OM_FILE_LINE 
    #define OM_FILE_LINE            ""
#endif  // _DEBUG

//#define OM_LOG_MSG(msg)         msg OM_FILE_LINE

#endif  // OM_I_HELPER_H
