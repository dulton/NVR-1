#include "loopbuf.h"
#include "netcommon.h"

#ifdef MSGQ_OPT
BOOL initMsgQ(ifly_msgQ_t *mq, u32 msgnum, u32 totallen, u32 unit_size)
{
	int i;

	if(mq && msgnum && totallen)
	{
		mq->rpos    = 0;
		mq->wpos    = 0;
		mq->unitNum = (totallen + unit_size - 1) / unit_size;
		mq->unitLen = unit_size;
		mq->buf = (u8 *)malloc(unit_size * mq->unitNum);
		if(mq->buf == NULL)
		{
			printf("initMsgQ:malloc buf failed\n");
			return FALSE;
		}

		mq->buf_start = malloc(sizeof(u32) * msgnum);
		if (NULL == mq->buf_start) {
			free(mq->buf);
			printf("initMsgQ:malloc buf_start failed\n");
			return FALSE;
		}

		mq->buf_num = malloc(sizeof(u32) * msgnum);
		if (NULL == mq->buf_num) {
			free(mq->buf);
			free(mq->buf_start);
			printf("initMsgQ:malloc buf_num failed\n");
			return FALSE;
		}

		mq->buf_jump = malloc(sizeof(u32) * msgnum);
		if (NULL == mq->buf_jump) {
			free(mq->buf);
			free(mq->buf_num);
			free(mq->buf_start);
			printf("initMsgQ:malloc buf_jump failed\n");
			return FALSE;
		}

		#ifdef MSGQ_H264_OVERLAY
		mq->gop_id= malloc(sizeof(u32) * msgnum);
		if (NULL == mq->gop_id) {
			free(mq->buf);
			free(mq->buf_num);
			free(mq->buf_start);
			free(mq->buf_jump);
			printf("initMsgQ:malloc gop_id failed\n");
			return FALSE;
		}
		mq->last_gop = 0;
		#endif

		memset(mq->buf_start , 0, sizeof(u32) * msgnum);
		memset(mq->buf_num , 0, sizeof(u32) * msgnum);
		memset(mq->buf_jump , 0, sizeof(u32) * msgnum);
		#ifdef MSGQ_H264_OVERLAY
		memset(mq->gop_id , 0, sizeof(u32) * msgnum);
		#endif

		mq->msgnum = msgnum;
		mq->free_pos = 0;
		mq->free_num = mq->unitNum;
		mq->used_pos = 0;
		#ifdef MSGQ_STAT
                mq->last_stamp = 0;
		mq->new_stamp = 0;
		#endif
		
		pthread_mutex_init(&mq->lock,NULL);
		
		sem_init(&mq->rsem,0,msgnum);
		sem_init(&mq->wsem,0,msgnum);
		for(i=0;i<msgnum;i++)
		{
			sem_wait(&mq->rsem);
		}
		return TRUE;
	}
	printf("initMsgQ:param error\n");
	return FALSE;
}


u8 getMsgQUsedPercent(ifly_msgQ_t *mq)
{
     // if((mq->unitNum - mq->free_num)>1000)
          //  printf("******%d***********%d \n",(mq->unitNum - mq->free_num),mq->unitNum);

        u32 percent;
	 pthread_mutex_lock(&mq->lock);
       if(0 ==(mq->unitNum - mq->free_num))
	  percent = 0;	
      else if(0 == mq->free_num)
	  percent = 100;
      else
	  percent = (mq->unitNum - mq->free_num)*100 /mq->unitNum;
	pthread_mutex_unlock(&mq->lock); 
	return percent;
}

u32 getMsgQBufSpan(ifly_msgQ_t *mq)
{
        #ifdef  STAT_DEBUG
         printf("spanstamp:%u \n",mq->new_stamp - mq->last_stamp);
	#endif
         pthread_mutex_lock(&mq->lock);
       u32 span = mq->new_stamp - mq->last_stamp;
	    pthread_mutex_unlock(&mq->lock);
	  return span;
}


int readMsgQ(ifly_msgQ_t *mq,u8 *pBuf,u32 readLen)
{
	int realLen;
	if(mq && pBuf && readLen)
	{
		sem_wait(&mq->rsem);

		u32 start = mq->buf_start[mq->rpos] ;
		u32 jump = mq->buf_jump[mq->rpos];
		u32 buf_oft = 0;
		if (0 == jump) buf_oft = start  * mq->unitLen;
		
		//pthread_mutex_lock(&mq->lock);
		memcpy(&realLen, mq->buf + buf_oft,  sizeof(u32));
		realLen = min(realLen, readLen);
		memcpy(pBuf, mq->buf+buf_oft+sizeof(u32),realLen);

		pthread_mutex_lock(&mq->lock);
		mq->used_pos += jump + mq->buf_num[mq->rpos];
		mq->used_pos %= mq->unitNum;
		mq->free_num += jump + mq->buf_num[mq->rpos];
		pthread_mutex_unlock(&mq->lock);
		
		mq->rpos = (mq->rpos + 1) % mq->msgnum;

		//pthread_mutex_unlock(&mq->lock);

		sem_post(&mq->wsem);
		
		return realLen;
	}
	printf("readMsgQ:param error\n");
	return -1;
}

int writeMsgQ(ifly_msgQ_t *mq,u8 *pBuf,u32 writeLen)
{
	if(mq && pBuf && writeLen <= mq->unitLen)
	{	
		sem_wait(&mq->wsem);
		pthread_mutex_lock(&mq->lock);

		u32 free_num = mq->free_num;
		u32 jump = 0;
		u32 buf_num = (writeLen + sizeof(u32) + mq->unitLen - 1) / mq->unitLen;

		//if (mq->free_pos >= mq->used_pos)
		//	free_num = mq->unitNum - mq->free_pos + mq->used_pos;
		//else
		//	free_num = mq->used_pos - mq->free_pos;

		if (mq->free_pos + buf_num > mq->unitNum) jump = mq->unitNum - mq->free_pos;

		if (jump + buf_num > free_num) {
			printf("writeMsgQ:failed\n");
			pthread_mutex_unlock(&mq->lock);
			sem_post(&mq->wsem);
			return -1;
		}

		mq->buf_start[mq->wpos] = mq->free_pos;
		mq->buf_num[mq->wpos] = buf_num;
		mq->buf_jump[mq->wpos] = jump;
		u32 buf_oft = 0;
		if (0 == jump) buf_oft = mq->free_pos  * mq->unitLen;
		
		memcpy(mq->buf+buf_oft, &writeLen, sizeof(u32));
		memcpy(mq->buf+buf_oft+sizeof(u32),pBuf,writeLen);

		mq->free_pos += buf_num + jump;
		mq->free_pos %= mq->unitNum;
		mq->free_num -= (buf_num + jump);
		
		mq->wpos = (mq->wpos + 1) % mq->msgnum;
		
		pthread_mutex_unlock(&mq->lock);

		sem_post(&mq->rsem);
		
		return writeLen;
	}
	printf("writeMsgQ:param error,mq=%x,pBuf=%x,writeLen=%d,mq->unitLen=%d\n",(int)mq,(int)pBuf,writeLen,mq->unitLen);
	return -1;
}

BOOL destroyMsgQ(ifly_msgQ_t *mq)
{
	if(mq)
	{
		pthread_mutex_destroy(&mq->lock);
		sem_destroy(&mq->rsem);
		sem_destroy(&mq->wsem);	
		if(mq->buf != NULL)
		{
			free(mq->buf);
			mq->buf = NULL;
		}

		if(mq->buf_start != NULL)
		{
			free(mq->buf_start);
			mq->buf_start = NULL;
		}

		if(mq->buf_num != NULL)
		{
			free(mq->buf_num);
			mq->buf_num = NULL;
		}

		if(mq->buf_jump != NULL)
		{
			free(mq->buf_jump);
			mq->buf_jump = NULL;
		}

		return TRUE;
	}
	printf("destroyMsgQ:param error\n");
	return FALSE;
}

BOOL GetMsgQReadInfo(ifly_msgQ_t *mq,u8 **ppBuf,u32 *pReadLen)
{
	if(mq && ppBuf && pReadLen)
	{
		#ifdef MSGQ_STAT
		if(-1 == sem_trywait(&mq->rsem))
		{
			return FALSE;
		}
		#else
		sem_wait(&mq->rsem);
		#endif
		
		u32 start = mq->buf_start[mq->rpos];
		u32 jump = mq->buf_jump[mq->rpos];
		u32 buf_oft = 0;
		if(0 == jump) buf_oft = start * mq->unitLen;
		//printf("start=%d buf_oft=%d jump=%d buf_num=%u\n", start, buf_oft, jump, mq->buf_num[mq->rpos] );
		memcpy(pReadLen,mq->buf+buf_oft,sizeof(u32));
		*ppBuf = mq->buf+buf_oft+sizeof(u32);
		//printf("after start=%d buf_oft=%d *pReadLen=%u mq->rpos=%d mq->free_num=%u\n", start, buf_oft, *pReadLen, mq->rpos, mq->free_num);
		
		return TRUE;
	}
	printf("GetMsgQReadInfo:param error\n");
	return FALSE;
}

BOOL skipReadMsgQ(ifly_msgQ_t *mq)
{
	if(mq)
	{
		u32 jump = mq->buf_jump[mq->rpos];
		
		pthread_mutex_lock(&mq->lock);
		
		mq->used_pos += jump + mq->buf_num[mq->rpos];
		mq->used_pos %= mq->unitNum;
		mq->free_num += jump + mq->buf_num[mq->rpos];
		
		mq->rpos = (mq->rpos + 1) % mq->msgnum;
		
		pthread_mutex_unlock(&mq->lock);
		
		sem_post(&mq->wsem);
		
		return TRUE;
	}
	printf("skipReadMsgQ:param error\n");
	return FALSE;
}

BOOL GetMsgQWriteInfo(ifly_msgQ_t *mq,u8 **ppBuf,u32 *pWriteLen)
{
	if(mq && ppBuf && pWriteLen)
	{
		if(-1==sem_trywait(&mq->wsem))//wrchen 090307
		{
		#ifdef MSGQ_H264_OVERLAY
			printf("MSGQ_H264_OVERLAY******************\n");
			
			pthread_mutex_lock(&mq->lock);
			int overlay_pos = -1, gop_id = -1, tmppos = mq->wpos;
			
			tmppos = (tmppos + mq->msgnum - 1) % mq->msgnum;
			
			while (tmppos != mq->rpos) {
				if (mq->gop_id[tmppos] > gop_id) overlay_pos = tmppos;
				tmppos = (tmppos + mq->msgnum - 1) % mq->msgnum;
			}

			if (overlay_pos >= 0) {
				sem_wait(&mq->rsem);
				
				u32 overlay_start = mq->buf_start[overlay_pos];
				u32 overlay_num= mq->buf_num[overlay_pos];
				u32 overlay_jump= mq->buf_jump[overlay_pos];
				u32 next_pos = (overlay_pos + 1) % mq->msgnum;

				mq->free_num += overlay_num + overlay_jump;

				while (next_pos != mq->wpos) {
					u32 next_num= mq->buf_num[next_pos];
					u32 next_jump= mq->buf_jump[next_pos];
					u8 *buf_from, *buf_to;

					if (next_jump) {
						buf_from = mq->buf;
						mq->free_num += next_jump;
					} else
						buf_from = mq->buf +  mq->buf_start[next_pos] * mq->unitLen;
					
					if (overlay_start + next_num > mq->unitNum) {
						mq->buf_jump[overlay_pos] = mq->unitNum - overlay_start;
						mq->free_num -= mq->buf_jump[overlay_pos] ;
						buf_to = mq->buf;
					} else {
						mq->buf_jump[overlay_pos] = 0;
						buf_to = mq->buf +  overlay_start * mq->unitLen;
					}
					mq->buf_num[overlay_pos] = next_num;
					mq->gop_id[overlay_pos] = mq->gop_id[next_pos];

					if (buf_from != buf_to)
						memcpy(buf_to, buf_from, next_num * mq->unitLen);

					mq->buf_start[next_pos] = (overlay_start + next_num + mq->buf_jump[overlay_pos]) % mq->msgnum;

					overlay_pos = next_pos;
					next_pos = (next_pos + 1) % mq->msgnum;
					
				}

				mq->free_pos = mq->buf_start[overlay_pos];
				mq->wpos = (mq->wpos + mq->msgnum - 1) % mq->msgnum;
			
				pthread_mutex_unlock(&mq->lock);
				
			} else{
				pthread_mutex_unlock(&mq->lock);
				return FALSE;
			}
		#else
			//printf("GetMsgQWriteInfo sem_trywait failed,msgnum=%d,unitNum=%d,unitLen=%d,bufsize=%d\n",mq->msgnum,mq->unitNum,mq->unitLen,mq->unitNum*mq->unitLen);
			return FALSE;
		#endif
		}
  		
		pthread_mutex_lock(&mq->lock);
		
		u32 free_num = mq->free_num;
		u32 jump = 0;
		u32 buf_num = (*pWriteLen + sizeof(u32) + mq->unitLen - 1) / mq->unitLen;
		
		//if (mq->free_pos >= mq->used_pos)
		//	free_num = mq->unitNum - mq->free_pos + mq->used_pos;
		//else
		//	free_num = mq->used_pos - mq->free_pos;
		
		if (mq->free_pos + buf_num > mq->unitNum) jump = mq->unitNum - mq->free_pos;
		
		if (jump + buf_num > free_num) {
			pthread_mutex_unlock(&mq->lock);
			sem_post(&mq->wsem);
			printf("writeMsgQ:failed\n");
			return FALSE;
		}
		
		mq->buf_start[mq->wpos] = mq->free_pos;
		mq->buf_num[mq->wpos] = buf_num;
		mq->buf_jump[mq->wpos] = jump;
		#ifdef MSGQ_H264_OVERLAY
		mq->gop_id[mq->wpos] = 0;
		#endif
		//printf("before mq->free_pos=%d unitNum=%d\n", mq->free_pos, mq->unitNum);
		u32 buf_oft = 0;
		if (0 == jump) buf_oft = mq->free_pos  * mq->unitLen;
		
		memcpy(mq->buf+buf_oft, pWriteLen, sizeof(u32));
		*ppBuf = mq->buf+buf_oft+sizeof(u32);
		//printf("after mq->free_pos=%d *pWriteLen=%d buf_num=%d buf_oft=%d wpos=%d\n", mq->free_pos, *pWriteLen, buf_num, buf_oft, mq->wpos);
		
		return TRUE;
	}
	printf("GetMsgQWriteInfo:param error\n");
	return FALSE;
}

#ifdef MSGQ_H264_OVERLAY
void set_msg_gopid(ifly_msgQ_t *mq, u8  b_auto)
{
	if (mq) {
		if (b_auto)
			mq->last_gop++;
		else
			mq->last_gop = 0;
		mq->gop_id[mq->wpos] = mq->last_gop;
	}
}
#endif

BOOL skipWriteMsgQ(ifly_msgQ_t *mq)
{
	if(mq)
	{
		u32 jump = 0;
		u32 buf_num = mq->buf_num[mq->wpos];
		if (mq->free_pos + buf_num > mq->unitNum) jump = mq->unitNum - mq->free_pos;
		
		mq->free_pos += (buf_num + jump);
		mq->free_pos %= mq->unitNum;
		mq->free_num -= (buf_num + jump);
		
		mq->wpos = (mq->wpos + 1) % mq->msgnum;
		
		pthread_mutex_unlock(&mq->lock);
		
		sem_post(&mq->rsem);
		
		return TRUE;
	}
	printf("skipWriteMsgQ:param error\n");
	return FALSE;
}

#else
BOOL initMsgQ(ifly_msgQ_t *mq,u32 unitNum,u32 unitLen)
{
	int i;

	if(mq && unitNum && unitLen)
	{
		mq->rpos    = 0;
		mq->wpos    = 0;
		mq->unitNum = unitNum;
		mq->unitLen = unitLen;
		mq->buf = (u8 *)malloc(unitNum*(sizeof(u32)+unitLen));
		if(mq->buf == NULL)
		{
			printf("initMsgQ:malloc failed\n");
			return FALSE;
		}
		pthread_mutex_init(&mq->lock,NULL);
		sem_init(&mq->rsem,0,unitNum);
		sem_init(&mq->wsem,0,unitNum);
		for(i=0;i<unitNum;i++)
		{
			sem_wait(&mq->rsem);
		}
		return TRUE;
	}
	printf("initMsgQ:param error\n");
	return FALSE;
}

int readMsgQ(ifly_msgQ_t *mq,u8 *pBuf,u32 readLen)
{
	int realLen;
	if(mq && pBuf && readLen)
	{
		sem_wait(&mq->rsem);
		
		//pthread_mutex_lock(&mq->lock);

		memcpy(&realLen,mq->buf+mq->rpos*(sizeof(u32)+mq->unitLen),sizeof(u32));
		realLen = min(realLen,readLen);
		memcpy(pBuf,mq->buf+mq->rpos*(sizeof(u32)+mq->unitLen)+sizeof(u32),realLen);
		
		mq->rpos = (mq->rpos + 1) % mq->unitNum;

		//pthread_mutex_unlock(&mq->lock);

		sem_post(&mq->wsem);
		
		return realLen;
	}
	printf("readMsgQ:param error\n");
	return -1;
}

int writeMsgQ(ifly_msgQ_t *mq,u8 *pBuf,u32 writeLen)
{
	if(mq && pBuf && writeLen <= mq->unitLen)
	{
		sem_wait(&mq->wsem);

		pthread_mutex_lock(&mq->lock);

		memcpy(mq->buf+mq->wpos*(sizeof(u32)+mq->unitLen),&writeLen,sizeof(u32));
		memcpy(mq->buf+mq->wpos*(sizeof(u32)+mq->unitLen)+sizeof(u32),pBuf,writeLen);
		
		mq->wpos = (mq->wpos + 1) % mq->unitNum;
		
		sem_post(&mq->rsem);

		pthread_mutex_unlock(&mq->lock);
		
		return writeLen;
	}
	printf("writeMsgQ:param error,mq=%x,pBuf=%x,writeLen=%d,mq->unitLen=%d\n",(int)mq,(int)pBuf,writeLen,mq->unitLen);
	return -1;
}

BOOL destroyMsgQ(ifly_msgQ_t *mq)
{
	if(mq)
	{
		pthread_mutex_destroy(&mq->lock);
		sem_destroy(&mq->rsem);
		sem_destroy(&mq->wsem);	
		if(mq->buf != NULL)
		{
			free(mq->buf);
			mq->buf = NULL;
		}
		return TRUE;
	}
	printf("destroyMsgQ:param error\n");
	return FALSE;
}

BOOL GetMsgQReadInfo(ifly_msgQ_t *mq,u8 **ppBuf,u32 *pReadLen)
{
	if(mq && ppBuf && pReadLen)
	{
		sem_wait(&mq->rsem);

		//pthread_mutex_lock(&mq->lock);
  
		memcpy(pReadLen,mq->buf+mq->rpos*(sizeof(u32)+mq->unitLen),sizeof(u32));
		*ppBuf = mq->buf+mq->rpos*(sizeof(u32)+mq->unitLen)+sizeof(u32);
		
		return TRUE;
	}
	printf("GetMsgQReadInfo:param error\n");
	return FALSE;
}

BOOL skipReadMsgQ(ifly_msgQ_t *mq)
{
	if(mq)
	{
		mq->rpos = (mq->rpos + 1) % mq->unitNum;
		
		//pthread_mutex_unlock(&mq->lock);

		sem_post(&mq->wsem);
		
		return TRUE;
	}
	printf("skipReadMsgQ:param error\n");
	return FALSE;
}

BOOL GetMsgQWriteInfo(ifly_msgQ_t *mq,u8 **ppBuf,u32 *pWriteLen)
{
	if(mq && ppBuf && pWriteLen)
	{
#if 0
		sem_wait(&mq->wsem);
#else
		if(-1==sem_trywait(&mq->wsem))//wrchen 090307
		{
			return FALSE;
		}
#endif

		pthread_mutex_lock(&mq->lock);
		    
    	    
		int tmpWPos = mq->wpos;
		mq->wpos = (mq->wpos + 1) % mq->unitNum;

		//pthread_mutex_unlock(&mq->lock);

		memcpy(mq->buf+tmpWPos*(sizeof(u32)+mq->unitLen),pWriteLen,sizeof(u32));
		*ppBuf = mq->buf+tmpWPos*(sizeof(u32)+mq->unitLen)+sizeof(u32);
		
		return TRUE;
	}
	printf("GetMsgQWriteInfo:param error\n");
	return FALSE;
}

BOOL skipWriteMsgQ(ifly_msgQ_t *mq)
{
	if(mq)
	{
		sem_post(&mq->rsem);
		
		pthread_mutex_unlock(&mq->lock);

		return TRUE;
	}
	printf("skipWriteMsgQ:param error\n");
	return FALSE;
}

#endif

