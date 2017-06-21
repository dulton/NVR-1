#ifndef _MSGCMD_H_
#define _MSGCMD_H_

#include "common_basetypes.h"

#include <pthread.h>
#include <semaphore.h>
#include <string.h>

typedef struct
{
    sem_t wsem;
    sem_t cmdsem;
    sem_t acksem;
    u32 nCmdSize;
    void *pData;
} SMsgCmd, *SMsgCmdHdr;

SMsgCmdHdr CreateMsgCmd(u32 nCmdSize);
s32 DestroyMsgCmd(SMsgCmdHdr psMsgCmdHdr);
s32 WriteMsgCmd(SMsgCmdHdr psMsgCmdHdr, const void* pData);
s32 ReadMsgCmd(SMsgCmdHdr psMsgCmdHdr, void* pData);
s32 AckMsgCmd(SMsgCmdHdr psMsgCmdHdr);

#endif

