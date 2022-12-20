#ifndef STRUCTS_H
#define STRUCTS_H

#define MAX_STRLEN      100
#define MAX_NORM_STRLEN 64
#define MAX_TOKLEN      50

typedef struct
{

   CMDMGR_Class  CmdMgr;
   TBLMGR_Class  TblMgr;
   uint32_t BufferSaveCnt;

   CFE_SB_PipeId_t CmdPipe;

} SMS_Class;

typedef struct
{

   uint8    Header[CFE_SB_TLM_HDR_SIZE];

   /*
   ** CMDMGR Data
   */
   uint16   ValidCmdCnt;
   uint16   InvalidCmdCnt;
   char     Payload[100];
   char     longPayload[200];
} SMS_HkPkt;
#define SMS_TLM_HK_PKT_LEN sizeof (SMS_HkPkt)

typedef struct
{
    uint8    Header[CFE_SB_TLM_HDR_SIZE];

    /*
    ** Buffer Save Data
    */

    uint8   token[MAX_TOKLEN];
} SMS_Token_Pkt;
#define SMS_TLM_TKN_PKT_LEN sizeof (SMS_Token_Pkt)

typedef struct
{
    uint8   CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint32  an_integer;
    uint32  another_integer;
    char    Payload[100];
} SMS_DATA_t;
#define SMS_DATA_LEN  (sizeof(SMS_DATA_t) - CFE_SB_CMD_HDR_SIZE)

typedef struct
{
    uint8   CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint32  an_integer;
    uint32  another_integer;
    char    longPayload[200];
} SMS_EXTEND_DATA_t;
#define SMS_EXT_DATA_LEN  (sizeof(SMS_EXTEND_DATA_t) - CFE_SB_CMD_HDR_SIZE)
#define EXTENDMSG       1722

#endif