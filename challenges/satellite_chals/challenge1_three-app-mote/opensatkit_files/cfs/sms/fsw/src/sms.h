/* 
** Purpose: Define a SMS application.
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

#ifndef SMS_H
#define SMS_H

#include "app_cfg.h"

extern SMS_Class sms_Object;
extern SMS_HkPkt sms_HkPkt;
extern SMS_Token_Pkt sms_Token_Pkt;

/* Convience Macros */
#define  CMDMGR_OBJ (&(sms_Object.CmdMgr))
#define  TBLMGR_OBJ (&(sms_Object.TblMgr))

extern char sms_string[MAX_STRLEN+1];

int32_t initializeApp_SMS(void);
int32_t appMainLoop_SMS(void);


boolean SMS_NoOpCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);
boolean SMS_ResetAppCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);
boolean SMS_NormalCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);
boolean SMS_ExtendedCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr); 

#endif