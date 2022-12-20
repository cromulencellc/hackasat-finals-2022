#include "sms.h"

int32_t initializeApp_SMS(void)
{
    int32 status = 0;

    /* Initialize cFE interfaces */
    /* create a pipe for this app to receive commands, and subscribe to specific command values */
    status = CFE_SB_CreatePipe(&sms_Object.CmdPipe, SMS_CMD_PIPE_DEPTH, SMS_CMD_PIPE_NAME);
    if (status != CFE_SUCCESS)
        return status;
    status = CFE_SB_Subscribe(SMS_CMD_MID, sms_Object.CmdPipe);
    if (status != CFE_SUCCESS)
        return status;
    // CFE_SB_Subscribe(MADE_UP_CONSTANT+0x10, thisObject.CmdPipe);
    status = CFE_SB_Subscribe(SMS_SEND_HK_MID, sms_Object.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SMS_INIT_INFO_EID, CFE_EVS_ERROR,
                            "Error Subscribing to HK Request,RC=0x%08X",status);
        return (status);
    }
    
    /* Initialize App Framework Components */
    CMDMGR_Constructor(CMDMGR_OBJ);

    CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_NOOP_CMD_FC,  NULL, SMS_NoOpCmd,     0);
    CMDMGR_RegisterFunc(CMDMGR_OBJ, CMDMGR_RESET_CMD_FC, NULL, SMS_ResetAppCmd, 0);
    CMDMGR_RegisterFunc(CMDMGR_OBJ, SMS_MODIFY_CMD_FC, NULL, SMS_NormalCmd, SMS_DATA_LEN);
    //CMDMGR_RegisterFunc(CMDMGR_OBJ, SMS_EXTEND_CMD_FC, NULL, SMS_ExtendedCmd, SMS_STRING_LEN);

    CFE_SB_InitMsg(&sms_HkPkt, SMS_TLM_HK_MID, SMS_TLM_HK_PKT_LEN, TRUE);

    /* Application startup event message */
    status = CFE_EVS_SendEvent(SMS_INIT_INFO_EID,
                               CFE_EVS_INFORMATION,
                               "%s Initialized. Version %d.%d.%d.%d",
                               APPNAME,
                               SMS_MAJOR_VERSION,
                               SMS_MINOR_VERSION,
                               SMS_REVISION,
                               SMS_MISSION_REV);
    if (status != CFE_SUCCESS)
        return status;

    OS_printf("%s initialization finished.\n", APPNAME);
    return CFE_SUCCESS;
} /* End of InitApp() */

