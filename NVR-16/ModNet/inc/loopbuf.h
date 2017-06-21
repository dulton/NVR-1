#ifndef _LOOPBUF_H_
#define _LOOPBUF_H_

//#include "common.h"

#include <semaphore.h>
#include "common_basetypes.h"

#define MSGQ_OPT

typedef struct
{
	u8   *buf;
	#ifdef MSGQ_OPT
	u32 *buf_start;
	u32 *buf_num;
	u32 *buf_jump;
	#ifdef MSGQ_H264_OVERLAY
	u32 *gop_id;
	u32 last_gop;
	#endif
	u32 msgnum;
	u32 free_pos;
	u32 used_pos;
	u32 free_num;
	u32 new_stamp;
	u32 last_stamp;
	#endif
	u32  unitNum;
	u32  unitLen;
	u32  rpos;
	u32  wpos;
	pthread_mutex_t lock;
	sem_t rsem;
	sem_t wsem;
}ifly_msgQ_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSGQ_OPT
BOOL initMsgQ(ifly_msgQ_t *mq, u32 msgnum, u32 totallen, u32 unit_size);
#else
BOOL initMsgQ(ifly_msgQ_t *mq,u32 unitNum,u32 unitLen);
#endif
#ifdef MSGQ_STAT

#if defined(TL_9508_8M) || defined(TL_9508_D1_8M)
#define N_SYS_CH		8
#else
#define N_SYS_CH		4
#endif

u8 getMsgQUsedPercent(ifly_msgQ_t *mq);

u8 UsedPerOldValure[N_SYS_CH*2];

#endif

int  readMsgQ(ifly_msgQ_t *mq,u8 *pBuf,u32 readLen);
int  writeMsgQ(ifly_msgQ_t *mq,u8 *pBuf,u32 writeLen);
BOOL destroyMsgQ(ifly_msgQ_t *mq);

BOOL GetMsgQReadInfo(ifly_msgQ_t *mq,u8 **ppBuf,u32 *pReadLen);
BOOL skipReadMsgQ(ifly_msgQ_t *mq);

BOOL GetMsgQWriteInfo(ifly_msgQ_t *mq,u8 **ppBuf,u32 *pWriteLen);
BOOL skipWriteMsgQ(ifly_msgQ_t *mq);

#ifdef __cplusplus
}
#endif

#endif
