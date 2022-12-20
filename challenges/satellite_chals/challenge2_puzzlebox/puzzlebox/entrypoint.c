#include "fsw/src/puzzlebox.h"

PUZZLEBOX_Class puzzlebox_Object;
PUZZLEBOX_HkPkt  puzzlebox_HkPkt;
PUZZLEBOX_Status_Pkt puzzlebox_StatusPkt;

/* Convience Macros */
#define  CMDMGR_OBJ (&(puzzlebox_Object.CmdMgr))
#define  TBLMGR_OBJ  (&(puzzlebox_Object.TblMgr))

void appStart_PUZZLEBOX(void)
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
    status = initializeApp_PUZZLEBOX();
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

    appMainLoop_PUZZLEBOX();
}