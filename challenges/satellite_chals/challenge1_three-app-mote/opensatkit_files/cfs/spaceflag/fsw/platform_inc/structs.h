#ifndef STRUCTS_H
#define STRUCTS_H

#define MAX_STRLEN      32
#define MAX_TOKLEN      32


typedef struct
{

   CMDMGR_Class  CmdMgr;
   TBLMGR_Class  TblMgr;
   uint32_t BufferSaveCnt;

   CFE_SB_PipeId_t CmdPipe;

} SPACEFLAG_Class;

typedef struct
{

   uint8    Header[CFE_SB_TLM_HDR_SIZE];

   /*
   ** CMDMGR Data
   */
   uint16   ValidCmdCnt;
   uint16   InvalidCmdCnt;
} SPACEFLAG_HkPkt;
#define SPACEFLAG_TLM_HK_PKT_LEN sizeof (SPACEFLAG_HkPkt)

typedef struct
{

   uint8    Header[CFE_SB_TLM_HDR_SIZE];
 
   /*
   ** Buffer Save Data
   */

   uint8   token[MAX_TOKLEN];
} SPACEFLAG_Token_Pkt;
#define SPACEFLAG_TLM_TKN_PKT_LEN sizeof (SPACEFLAG_Token_Pkt)

typedef struct
{
   uint8   CmdHeader[CFE_SB_CMD_HDR_SIZE];
   uint8   Payload[MAX_STRLEN];
} SPACEFLAG_STRING_t;
#define SPACEFLAG_STRING_LEN  (sizeof(SPACEFLAG_STRING_t) - CFE_SB_CMD_HDR_SIZE)

#endif