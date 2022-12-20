/*
** Purpose: Define configurations for the SMS application.
**
** Notes:
**   1. These macros can only be built with the application and can't
**      have a platform scope because the same file name is used for
**      all applications following the object-based application design.
**
** License:
**   Template written by David McComas and licensed under the GNU
**   Lesser General Public License (LGPL).
**
** References:
**   1. OpenSatKit Object-based Application Developers Guide.
**   2. cFS Application Developer's Guide.
**
*/

#ifndef _app_cfg_
#define _app_cfg_

/*
** Includes
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfe.h"
#include "sms.h"
#include "osk_app_fw.h"

#include "../platform_inc/structs.h"
#include "../platform_inc/sms_platform_cfg.h"

#include "../mission_inc/sms_mission_msgids.h"

/******************************************************************************
** SMS Application Macros
*/
#define  APPNAME                        "SMS"

#define  SMS_MAJOR_VERSION      0
#define  SMS_MINOR_VERSION      1
#define  SMS_REVISION           0
#define  SMS_MISSION_REV        0

/******************************************************************************
** Command Macros
**
*/

// #define SAVE_BUFFER_CMD_FC (CMDMGR_APP_START_FC + 0)

/******************************************************************************
** Event Macros
** 
** Define the base event message IDs used by each object/component used by the
** application. There are no automated checks to ensure an ID range is not
** exceeded so it is the developer's responsibility to verify the ranges. 
*/

#define SMS_BASE_EID (APP_FW_APP_BASE_EID +  0)

/******************************************************************************
** Example Table Configurations
**
*/

// #define EXTBL_MAX_ENTRY_ID 32

#endif /* _app_cfg_ */