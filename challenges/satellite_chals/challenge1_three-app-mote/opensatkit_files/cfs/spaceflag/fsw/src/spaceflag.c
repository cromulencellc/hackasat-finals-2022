#include "spaceflag.h"
#include <openssl/sha.h>

static void _ProcessCommands(void);
static void _SendHouseKeepingPacket(void);

int32_t appMainLoop_SPACEFLAG(void)
{
    OS_printf("%s App Main Loop\n", APPNAME);
    /* Main process loop */
    uint32_t RunStatus = CFE_ES_APP_RUN;
    while (CFE_ES_RunLoop(&RunStatus))
    {
        OS_TaskDelay(SPACEFLAG_RUNLOOP_DELAY);
        _ProcessCommands();
    }

    return RunStatus;
}

/* Function: ProcessCommands */
static void _ProcessCommands(void)
{
   int32           Status;
   CFE_SB_Msg_t*   CmdMsgPtr;
   CFE_SB_MsgId_t  MsgId;
   uint16          FuncCode;

   Status = CFE_SB_RcvMsg(&CmdMsgPtr, spaceflag_Object.CmdPipe, CFE_SB_POLL);

   if (Status == CFE_SUCCESS)
   {
        MsgId = CFE_SB_GetMsgId(CmdMsgPtr);
        FuncCode = CFE_SB_GetCmdCode(CmdMsgPtr);
        switch (MsgId)
        {
            case SPACEFLAG_CMD_MID:
                OS_printf("%s App received command message(%d)(%d)\n", APPNAME, FuncCode, MsgId);
                
                CMDMGR_DispatchFunc(CMDMGR_OBJ, CmdMsgPtr);
                break;
            case SPACEFLAG_SEND_HK_MID:
                _SendHouseKeepingPacket();
                break;
            default:
                CFE_EVS_SendEvent(SPACEFLAG_CMD_INVALID_MID_ERR_EID, CFE_EVS_ERROR, "Received invalid command packet,MID = 0x%4X",MsgId);
                break;
        }
   }
} /* End _ProcessCommands() */


/******************************************************************************
** Function: _SendHousekeepingPkt
**
*/

static void _SendHouseKeepingPacket(void)
{
    /* Good design practice in case app expands to more than one table */
        // const TBLMGR_Tbl* LastTbl = TBLMGR_GetLastTblStatus(TBLMGR_OBJ);

    /* CMDMGR Data */
    spaceflag_HkPkt.ValidCmdCnt   = spaceflag_Object.CmdMgr.ValidCmdCnt;
    spaceflag_HkPkt.InvalidCmdCnt = spaceflag_Object.CmdMgr.InvalidCmdCnt;

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &spaceflag_HkPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &spaceflag_HkPkt);

} /* End _SendHouseKeepingPacket() */

/******************************************************************************
** Function: SPACEFLAG_NoOpCmd
**
** Function signature must match CMDMGR_CmdFuncPtr typedef 
*/
boolean SPACEFLAG_NoOpCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
   OS_printf("received NOOP command\n");
   CFE_EVS_SendEvent (SPACEFLAG_CMD_NOOP_INFO_EID,
                      CFE_EVS_INFORMATION,
                      "No operation command received for %s version %d.%d",
                      APPNAME, SPACEFLAG_MAJOR_VERSION, SPACEFLAG_MINOR_VERSION);

   return TRUE;
} /* End SPACEFLAG_NoOpCmd() */


/******************************************************************************
** Function: SPACEFLAG_ResetAppCmd
**
** Function signature must match CMDMGR_CmdFuncPtr typedef 
*/

boolean SPACEFLAG_ResetAppCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
   OS_printf("%s received ResetApp command\n", APPNAME);
   CMDMGR_ResetStatus(CMDMGR_OBJ);

   return TRUE;
} /* End SPACEFLAG_ResetAppCmd() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SPACEFLAG_String                                                 */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function gets a string                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
boolean SPACEFLAG_String(void* ObjDataPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
    OS_printf("%s received String command\n", APPNAME);
    const SPACEFLAG_STRING_t *Msg = (const SPACEFLAG_STRING_t*) MsgPtr;

    // save payload to the buffer
    memset(spaceflag_flag, '\0', MAX_STRLEN+1);
    CFE_PSP_MemCpy((void *)spaceflag_flag, (void *)(Msg->Payload), MAX_STRLEN);

    // OS_printf("%s stored\n", spaceflag_flag);
    // const unsigned char* hash = NULL;
    // hash = calloc(SHA256_DIGEST_LENGTH, sizeof(char));
    // SHA256(spaceflag_flag, strnlen(spaceflag_flag, MAX_STRLEN), (unsigned char*) hash);
    // OS_printf("created: %s\n", hash);
    return TRUE;
} /* End of SPACEFLAG_String */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SPACEFLAG_Send_Token                                             */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function sends a string                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
boolean SPACEFLAG_Send_Token(void* ObjDataPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
    OS_printf("%s received Send Token command, sending (%s)\n", APPNAME, spaceflag_flag);
    /* Good design practice in case app expands to more than one table */
        // const TBLMGR_Tbl* LastTbl = TBLMGR_GetLastTblStatus(TBLMGR_OBJ);

    if (strnlen(spaceflag_flag, MAX_STRLEN) != 0)
    {
        memset(spaceflag_Token_Pkt.token, '\0', MAX_TOKLEN);
        memcpy(spaceflag_Token_Pkt.token, spaceflag_flag , strnlen(spaceflag_flag, MAX_STRLEN));
    }
    else
    {
        memset(spaceflag_Token_Pkt.token, '\0', MAX_TOKLEN);
        memcpy(spaceflag_Token_Pkt.token, "missing input\0",14);
    }

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &spaceflag_Token_Pkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &spaceflag_Token_Pkt);
    return TRUE;
} /* End SPACEFLAG_Send_Token() */





size_t strnlen(const char *s, size_t maxlen) {
    size_t len = 0;
    for(char c = *s; (len < maxlen && c != '\0'); c = *++s) len++;
    return len;
}
