#include "fsw/src/spaceflag.h"
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
SPACEFLAG_Class spaceflag_Object;
SPACEFLAG_HkPkt  spaceflag_HkPkt;
SPACEFLAG_Token_Pkt  spaceflag_Token_Pkt;


volatile char spaceflag_flag[32] = {0};
int32_t (*validPtrs)[10];

/* Convience Macros */
#define  CMDMGR_OBJ (&(spaceflag_Object.CmdMgr))
#define  TBLMGR_OBJ  (&(spaceflag_Object.TblMgr))

void appStart_SPACEFLAG(void)
{
    int32 status = 0;
    
    OS_printf("%s APP Starting...\n", APPNAME);
    /* Register this application with the CFE */
    if (CFE_ES_RegisterApp() != CFE_SUCCESS)
    {
        OS_printf("%s App Registration Failure\n", APPNAME);  
        exit(EXIT_FAILURE);
    }
    int psize = sysconf(_SC_PAGE_SIZE);
    printf("Page size: %d\n", psize);
    if (psize == -1)
    if (mprotect(validPtrs, psize, PROT_READ) == -1)
    {
        OS_printf("%s sysconf Failure(%d)\n", APPNAME, errno);
        exit(EXIT_FAILURE);
    }
    validPtrs = memalign(psize, psize);
    if (validPtrs == NULL)
    {
        OS_printf("%s memalign Failure(%d)\n", APPNAME, errno);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 10; i++)
        (*validPtrs)[i] = NULL;
    (*validPtrs)[0] = (void *)SPACEFLAG_Send_Token;
    // add NOOP
    // add reset counter, other safe/tlm functions
    if (mprotect(validPtrs, psize, PROT_READ) == -1)
    {
        OS_printf("%s mprotect Failure(%d)\n", APPNAME, errno);
        exit(EXIT_FAILURE);
    }
    /* Register with the event service system */
    if (CFE_EVS_Register(NULL,0,0) != CFE_SUCCESS)
    {
        OS_printf("%s App Event Service Registration Failure\n", APPNAME);
        exit(EXIT_FAILURE);
    }
    
    /* Initialize app with pipe/subscriptions/cmdmgr/functions/intmessage */
    status = initializeApp_SPACEFLAG();
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

    void * fp = (void *)SPACEFLAG_Send_Token;
    OS_printf("Pointer Leak: %p and %p\n", fp, (*validPtrs)[0]);

    appMainLoop_SPACEFLAG();

}