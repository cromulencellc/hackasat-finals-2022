/* 
** Purpose: Define a SPACEFLAG application.
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

#ifndef SPACEFLAG_H
#define SPACEFLAG_H

#include "app_cfg.h"

extern SPACEFLAG_Class spaceflag_Object;
extern SPACEFLAG_HkPkt spaceflag_HkPkt;
extern SPACEFLAG_Token_Pkt spaceflag_Token_Pkt;

/* Convience Macros */
#define  CMDMGR_OBJ (&(spaceflag_Object.CmdMgr))
#define  TBLMGR_OBJ (&(spaceflag_Object.TblMgr))

extern volatile char spaceflag_flag[32];
extern int32_t (*validPtrs)[10];

int32_t initializeApp_SPACEFLAG(void);
int32_t appMainLoop_SPACEFLAG(void);

size_t strnlen(const char *s, size_t maxlen);

boolean SPACEFLAG_NoOpCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);
boolean SPACEFLAG_ResetAppCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);
boolean SPACEFLAG_String(void* ObjDataPtr, const CFE_SB_MsgPtr_t MsgPtr);
boolean SPACEFLAG_Send_Token(void* ObjDataPtr, const CFE_SB_MsgPtr_t MsgPtr);


#endif