#include "sms.h"
//#include <openssl/sha.h>

static void _ProcessCommands(void);
static void _SendHouseKeepingPacket(void);

boolean bad_struct_type(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr);

int32_t appMainLoop_SMS(void)
{
    OS_printf("%s App Main Loop\n", APPNAME);
    /* Main process loop */
    uint32_t RunStatus = CFE_ES_APP_RUN;
    while (CFE_ES_RunLoop(&RunStatus))
    {
        OS_TaskDelay(SMS_RUNLOOP_DELAY);
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

   Status = CFE_SB_RcvMsg(&CmdMsgPtr, sms_Object.CmdPipe, CFE_SB_POLL);

   if (Status == CFE_SUCCESS)
   {
        MsgId = CFE_SB_GetMsgId(CmdMsgPtr);
        FuncCode = CFE_SB_GetCmdCode(CmdMsgPtr);

        if (MsgId == SMS_CMD_MID) {
            OS_printf("%s App received command message(%d)(%d)\n", APPNAME, FuncCode, MsgId);
            CMDMGR_DispatchFunc(CMDMGR_OBJ, CmdMsgPtr);
        }
        else if (MsgId == SMS_CMD_MID_EXTEND) {
            SMS_ExtendedCmd(CMDMGR_OBJ, CmdMsgPtr);
        }
        else if (MsgId == SMS_SEND_HK_MID) {
            _SendHouseKeepingPacket();
        }
        else {
            CFE_EVS_SendEvent(SMS_CMD_INVALID_MID_ERR_EID, CFE_EVS_ERROR, "Received invalid command packet,MID = 0x%4X",MsgId);
            SMS_CMD_MID = SMS_CMD_MID_DEF;
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
    sms_HkPkt.ValidCmdCnt   = sms_Object.CmdMgr.ValidCmdCnt;
    sms_HkPkt.InvalidCmdCnt = sms_Object.CmdMgr.InvalidCmdCnt;

    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &sms_HkPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &sms_HkPkt);

} /* End _SendHouseKeepingPacket() */

/******************************************************************************
** Function: SMS_NoOpCmd
**
** Function signature must match CMDMGR_CmdFuncPtr typedef 
*/
boolean SMS_NoOpCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
   OS_printf("received NOOP command\n");
   CFE_EVS_SendEvent (SMS_CMD_NOOP_INFO_EID,
                      CFE_EVS_INFORMATION,
                      "No operation command received for %s version %d.%d",
                      APPNAME, SMS_MAJOR_VERSION, SMS_MINOR_VERSION);

   return TRUE;
} /* End SMS_NoOpCmd() */


/******************************************************************************
** Function: SMS_ResetAppCmd
**
** Function signature must match CMDMGR_CmdFuncPtr typedef 
*/

boolean SMS_ResetAppCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
   OS_printf("%s received ResetApp command\n", APPNAME);
   CMDMGR_ResetStatus(CMDMGR_OBJ);

   return TRUE;
} /* End SMS_ResetAppCmd() */

/******************************************************************************
** Function: SMS_NormalCmd
**
** Function signature must match CMDMGR_CmdFuncPtr typedef 
*/
boolean SMS_NormalCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
    /* Do something in this function - doesn't really matter what */
    OS_printf("received new message\n");
    const SMS_DATA_t *Msg = (const SMS_DATA_t*) MsgPtr;

    if (Msg->another_integer == EXTENDMSG && strlen(Msg->Payload) > 64)
    {
        OS_printf("%d Extended message detected\n", Msg->an_integer);
        SMS_CMD_MID += 0x10;
    }
    else
    {
        strcpy(sms_HkPkt.Payload, Msg->Payload);
    }
    
    OS_printf("%d stored\n", Msg->an_integer);
    OS_printf("%d stored\n", Msg->another_integer);
    OS_printf("%s stored\n", Msg->Payload);
    return TRUE;
} /* End SMS_NormalCmd() */


/******************************************************************************
** Function: SMS_ExtendedCmd
**
** Function signature must match CMDMGR_CmdFuncPtr typedef 
*/
boolean SMS_ExtendedCmd(void* DataObjPtr, const CFE_SB_MsgPtr_t MsgPtr)
{
    /* Do something in this function - doesn't really matter what */
    OS_printf("received Extended command\n");
    const SMS_EXTEND_DATA_t *Msg = (const SMS_EXTEND_DATA_t*) MsgPtr;

    OS_printf("%d stored\n", Msg->an_integer);
    OS_printf("%d stored\n", Msg->another_integer);
    OS_printf("%s stored\n", Msg->longPayload);

    //sms_HkPkt.longPayload = Msg->longPayload;

    SMS_CMD_MID = SMS_CMD_MID_DEF;
    return TRUE;
} /* End SMS_ExtendedCmd() */
