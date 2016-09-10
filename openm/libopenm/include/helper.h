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

// define max size of file path
#ifdef PATH_MAX
    #define OM_PATH_MAX (FILENAME_MAX < PATH_MAX ? FILENAME_MAX : PATH_MAX)
#endif
#ifndef OM_PATH_MAX
    #ifdef _MAX_PATH
        #define OM_PATH_MAX (FILENAME_MAX < _MAX_PATH ? FILENAME_MAX : _MAX_PATH)
    #else
        #define OM_PATH_MAX (FILENAME_MAX < 4096 ? FILENAME_MAX : 4096)
    #endif 
#endif 

#define OM_STRLEN_MAX   INT_MAX         /** max string length supported */
#define OM_STR_DB_MAX   32000           /** max database string length: notes varchar (clob, text) */

#define OM_SEND_SLEEP_TIME  29L         /* msec, send completion test sleep interval */
#define OM_RECV_SLEEP_TIME  31L         /* msec, receive probe sleep interval */
#define OM_RUN_POLL_TIME 17L            /* msec, wait interval for modeling threads polling */
#define OM_ACTIVE_SLEEP_TIME 37L        /* msec, sleep interval if there any child process activity */
#define OM_WAIT_SLEEP_TIME 971L         /* msec, sleep interval if no progress in modeling threads */

// add file and line number to debug log
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
