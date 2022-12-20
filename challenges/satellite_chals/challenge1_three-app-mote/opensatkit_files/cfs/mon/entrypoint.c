#include "fsw/src/mon.h"

MON_Class mon_Object;
MON_HkPkt  mon_HkPkt;
MON_Token_Pkt  mon_Token_Pkt;

char mon_string[MAX_STRLEN+1];

uint32_t SMS_CMD_MID; // = MON_CMD_MID_DEF;
int32_t (*validPtrs)[10];
/* Convience Macros */
#define  CMDMGR_OBJ (&(mon_Object.CmdMgr))
#define  TBLMGR_OBJ  (&(mon_Object.TblMgr))

void appStart_MON(void)
{
    int32 status = 0;
    
    OS_printf("%s APP Starting...\n", APPNAME);
    /* Register this application with the CFE */
    if (CFE_ES_RegisterApp() != CFE_SUCCESS)
    {
        OS_printf("%s App Registration Failure\n", APPNAME);  
        exit(EXIT_FAILURE);
    }
    
    /* Register with the event service system */
    if (CFE_EVS_Register(NULL,0,0) != CFE_SUCCESS)
    {
        OS_printf("%s App Event Service Registration Failure\n", APPNAME);
        exit(EXIT_FAILURE);
    }
    
    /* Initialize app with pipe/subscriptions/cmdmgr/functions/intmessage */
    status = initializeApp_MON();
    if (status != CFE_SUCCESS)
    {
        OS_printf("%s App Initialization Failure (%d)\n", APPNAME, status);
        exit(EXIT_FAILURE);
    }

    /*** 
     * At this point many flight apps use CFE_ES_WaitForStartupSync()
     * to synchronize their startup timing with other apps. This is
     * not needed for this simple app.
     ***/

    appMainLoop_MON();

}