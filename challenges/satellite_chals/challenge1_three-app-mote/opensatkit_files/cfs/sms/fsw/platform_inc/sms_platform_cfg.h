/*
** Purpose: Define platform configurations for the SMS application
**
** Notes:
**   None
**
** License:
**   Template written by David McComas and licensed under the GNU
**   Lesser General Public License (LGPL).
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide.
**   2. cFS Application Developer's Guide.
**
*/

#ifndef _sms_platform_cfg_
#define _sms_platform_cfg_

/******************************************************************************
** Application Macros
*/
#define  SMS_RUNLOOP_DELAY              500  /* Delay in milliseconds */
#define  SMS_CMD_PIPE_DEPTH             10
#define  SMS_CMD_PIPE_NAME              APPNAME /* name must be 1 to 15 characters in length */


/*
** Macro Definitions
*/

#define SMS_INIT_INFO_EID               (SMS_BASE_EID + 0)
#define SMS_EXIT_ERR_EID                (SMS_BASE_EID + 1)
#define SMS_CMD_NOOP_INFO_EID           (SMS_BASE_EID + 2)
#define SMS_CMD_INVALID_MID_ERR_EID     (SMS_BASE_EID + 3)

#define SMS_MODIFY_CMD_FC               2
#define SMS_EXTEND_CMD_FC               3
#endif /* _sms_platform_cfg_ */