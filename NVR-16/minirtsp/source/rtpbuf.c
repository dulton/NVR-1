//#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtpbuf.h"
#include "vlog.h"

#include <sys/time.h>

void CircleBufferWrite(CircleBuffer_t* pCBuffer, const void* pSourceBuffer, const unsigned int iNumBytes);
int CircleBufferRead(CircleBuffer_t* pCBuffer, void* pDestBuffer, const unsigned int iBytesToRead, unsigned int* pbBytesRead);
void CircleFlush(CircleBuffer_t* pCBuffer);
unsigned int CircleGetFreeSpace(CircleBuffer_t* pCBuffer);
unsigned int CircleGetUsedSpace(CircleBuffer_t* pCBuffer);
void CircleSetComplete(CircleBuffer_t* pCBuffer);
int CircleIsComplete(CircleBuffer_t* pCBuffer);
////////////////////////////////////////////////////////////////////////////////

//
//
//
void CircleBufferWrite(CircleBuffer_t* pCBuffer, const void* _pSourceBuffer, const unsigned int _iNumBytes)
{
    unsigned int iBytesToWrite = _iNumBytes;
    BYTE* pReadCursor = (BYTE*)_pSourceBuffer;

	if(iBytesToWrite > pCBuffer->GetFreeSize(pCBuffer)){
        VLOG(VLOG_ERROR,"no engough buffer to write");
		exit(1);
	}

    LOCK_lock(&pCBuffer->m_csCircleBuffer);

    if(pCBuffer->m_iWriteCursor >= pCBuffer->m_iReadCursor)
    {
        unsigned int iChunkSize = pCBuffer->m_iBufferSize - pCBuffer->m_iWriteCursor;
        if(iChunkSize > iBytesToWrite)
            iChunkSize = iBytesToWrite;

        memcpy(pCBuffer->m_pBuffer + pCBuffer->m_iWriteCursor,
               pReadCursor, iChunkSize);
        pReadCursor += iChunkSize;
        iBytesToWrite -= iChunkSize;

        pCBuffer->m_iWriteCursor += iChunkSize;
        if(pCBuffer->m_iWriteCursor >= pCBuffer->m_iBufferSize)
            pCBuffer->m_iWriteCursor -= pCBuffer->m_iBufferSize;
    }

    if(iBytesToWrite)
    {
        memcpy(pCBuffer->m_pBuffer + pCBuffer->m_iWriteCursor,
               pReadCursor, iBytesToWrite);
        pCBuffer->m_iWriteCursor += iBytesToWrite;	
		if(pCBuffer->m_iWriteCursor >= pCBuffer->m_iBufferSize){
        	VLOG(VLOG_ERROR,"write cursor overflow");
			exit(1);
		}
    }

    EVT_post(pCBuffer->m_evtDataAvailable);
    LOCK_unlock(&pCBuffer->m_csCircleBuffer);
}
//

int CircleBufferRead(CircleBuffer_t* pCBuffer, void* pDestBuffer, const unsigned int _iBytesToRead, unsigned int* pbBytesRead)
{
    unsigned int iBytesToRead = _iBytesToRead;
    unsigned int iBytesRead = 0;
	//unsigned int iAvaiBytes = 0;

    while(iBytesToRead > 0)
    {
#if defined(_WIN32) || defined(_WIN64)
		unsigned long dwWaitResult;
        EVT_wait(pCBuffer->m_evtDataAvailable, CIC_WAITTIMEOUT,dwWaitResult);
        if(dwWaitResult == WAIT_TIMEOUT)
        {
            VLOG(VLOG_DEBUG,"Circle buffer - did not fill in time!");
            *pbBytesRead = iBytesRead;
            return FALSE;
        }
#else
		int dwWaitResult;
        EVT_wait(pCBuffer->m_evtDataAvailable, CIC_WAITTIMEOUT,dwWaitResult);
		if(dwWaitResult != 0){
            VLOG(VLOG_DEBUG,"Circle buffer - did not fill in time!");
            *pbBytesRead = iBytesRead;
            return FALSE;
		}
#endif
		/*
		iAvaiBytes = CircleGetUsedSpace(pCBuffer);
		if(iAvaiBytes < iBytesToRead){
			VLOG(VLOG_DEBUG,"no enough data,available:%d needed:%d",iAvaiBytes,iBytesToRead);
            *pbBytesRead = iBytesRead;
			return FALSE;
		}else{
			VLOG(VLOG_DEBUG,"enough data,available:%d needed:%d",iAvaiBytes,iBytesToRead);
		}*/
        LOCK_lock(&pCBuffer->m_csCircleBuffer);
		
		
        if(pCBuffer->m_iReadCursor > pCBuffer->m_iWriteCursor)
        {
            unsigned int iChunkSize = pCBuffer->m_iBufferSize - pCBuffer->m_iReadCursor;
            if(iChunkSize > iBytesToRead)
                iChunkSize = iBytesToRead;

            memcpy((BYTE*)pDestBuffer + iBytesRead,
                   pCBuffer->m_pBuffer + pCBuffer->m_iReadCursor,
                   iChunkSize);

            iBytesRead += iChunkSize;
            iBytesToRead -= iChunkSize;

            pCBuffer->m_iReadCursor += iChunkSize;
            if(pCBuffer->m_iReadCursor >= pCBuffer->m_iBufferSize)
                pCBuffer->m_iReadCursor -= pCBuffer->m_iBufferSize;
        }

        if(iBytesToRead && pCBuffer->m_iReadCursor < pCBuffer->m_iWriteCursor)
        {
            unsigned int iChunkSize = pCBuffer->m_iWriteCursor - pCBuffer->m_iReadCursor;
            if(iChunkSize > iBytesToRead)
                iChunkSize = iBytesToRead;

            memcpy((BYTE*)pDestBuffer + iBytesRead,
                   pCBuffer->m_pBuffer + pCBuffer->m_iReadCursor,
                   iChunkSize);

            iBytesRead += iChunkSize;
            iBytesToRead -= iChunkSize;
            pCBuffer->m_iReadCursor += iChunkSize;
        }

        if(pCBuffer->m_iReadCursor != pCBuffer->m_iWriteCursor)
        {
            EVT_post(pCBuffer->m_evtDataAvailable);
        }

        LOCK_unlock(&pCBuffer->m_csCircleBuffer);
    }

    *pbBytesRead = iBytesRead;
    return TRUE;
}
//
//
//
void CircleFlush(CircleBuffer_t* pCBuffer)
{
	VLOG(VLOG_CRIT,"CBuffer flus!!!!");
    LOCK_lock(&pCBuffer->m_csCircleBuffer);
    pCBuffer->m_iReadCursor = 0;
    pCBuffer->m_iWriteCursor = 0;
	pCBuffer->m_iFrameCnt = 0;
	pCBuffer->m_iTotalBytes = 0;
	pCBuffer->m_fInSpeed = 1;
	pCBuffer->m_fUsedRate = 0;
	if(pCBuffer->m_FrameInfo){
		DLINK_destroy(pCBuffer->m_FrameInfo);
		DLINK_init(&pCBuffer->m_FrameInfo);
	}
    LOCK_unlock(&pCBuffer->m_csCircleBuffer);
}
//
//
//
unsigned int CircleGetFreeSpace(CircleBuffer_t* pCBuffer)
{
    unsigned int iNumBytesFree;

    LOCK_lock(&pCBuffer->m_csCircleBuffer);

    if(pCBuffer->m_iWriteCursor < pCBuffer->m_iReadCursor)
        iNumBytesFree = (pCBuffer->m_iReadCursor-1) - pCBuffer->m_iWriteCursor;
    else if(pCBuffer->m_iWriteCursor == pCBuffer->m_iReadCursor)
        iNumBytesFree = pCBuffer->m_iBufferSize;
    else
        iNumBytesFree = (pCBuffer->m_iReadCursor-1) + (pCBuffer->m_iBufferSize - pCBuffer->m_iWriteCursor);

    LOCK_unlock(&pCBuffer->m_csCircleBuffer);
    return iNumBytesFree;
}
//
//
//
unsigned int CircleGetUsedSpace(CircleBuffer_t* pCBuffer)
{
    return pCBuffer->m_iBufferSize - CircleGetFreeSpace(pCBuffer);
}

float CircleGetInSpeed(CircleBuffer_t *pCBuffer)
{
	return pCBuffer->m_fInSpeed;
}

int CircleFreeAFrame2(CircleBuffer_t *pcBuffer,int free_size)
{
	DLElem_t elem;
	int i_total=0;
	//int first_frame=TRUE;
	do{
		if(pcBuffer->m_FrameInfo->data.entries == 0){
			VLOG(VLOG_WARNING,"$$$$ DLink is empty now $$$$$");
			return RET_FAIL;
		}
		if(DLINK_pull_and_del(pcBuffer->m_FrameInfo,DLINK_POS_BOT,&elem)==RET_FAIL)
			return RET_FAIL;
		//if(first_frame == TRUE){
		//	if(node->data.frame.key_flag == TRUE){
		//		//return RET_FAIL;
		//	}
		//}
		VLOG(VLOG_DEBUG,"entries:%d",pcBuffer->m_FrameInfo->data.entries);
		i_total += sizeof(DLNode_t) + elem.frame.frame_size;
		//pcBuffer->m_iReadCursor +=sizeof(DLNode_t) + node->data.frame.frame_size;
		pcBuffer->m_iReadCursor = elem.frame.frame_pos + elem.frame.frame_size;
		if(pcBuffer->m_iReadCursor >= pcBuffer->m_iBufferSize)
			pcBuffer->m_iReadCursor -= pcBuffer->m_iBufferSize;
		if(elem.frame.stop_flag == TRUE){
			pcBuffer->m_iFrameCnt --;
		}
		//if(first_frame == TRUE){
			if((elem.frame.stop_flag == TRUE) && (i_total >= free_size)
				/*&& (node->data.frame.key_flag == FALSE) */){
				if(elem.frame.key_flag == TRUE){
					VLOG(VLOG_WARNING,"release a key frame buffer");
				}
				break;
			}
		//}
		//if(node->data.frame.stop_flag == TRUE){
	//		first_frame = FALSE;
		//}
	}while(TRUE);
	
	return RET_OK;
}

// free a oldest frame,
int CircleFreeAFrame(CircleBuffer_t *pcBuffer,int free_size)
{
	DLElem_t elem;
	int i_total=0;
	int first_frame=TRUE;
	do{
		if(pcBuffer->m_FrameInfo->data.entries == 0){
			VLOG(VLOG_WARNING,"$$$$ DLink is empty now $$$$$");
			return FREE_MEET_EMPTY_BUFFER;
		}
		if(DLINK_pull(pcBuffer->m_FrameInfo,DLINK_POS_BOT,&elem)==RET_FAIL)
			return FREE_MEET_EMPTY_BUFFER;
		if(first_frame == TRUE){
			//if(elem.frame.key_flag == TRUE){
			//	VLOG(VLOG_WARNING,"$$$$ free meet a idr $$$$$");
			//	return FREE_MEET_IDR;
			//}
		}else{
			if(elem.frame.key_flag == TRUE){
				if(i_total >= free_size && elem.frame.start_flag == TRUE){
					return FREE_RET_OK;
				}else{
					//VLOG(VLOG_WARNING,"$$$$ free meet a idr $$$$");
					//return FREE_MEET_IDR;
				}
			}
		}
		DLINK_delete(pcBuffer->m_FrameInfo,DLINK_POS_BOT);
		VLOG(VLOG_DEBUG,"entries:%d frames:%d",pcBuffer->m_FrameInfo->data.entries,pcBuffer->m_iFrameCnt);
		i_total += sizeof(DLNode_t) + elem.frame.frame_size;
		//pcBuffer->m_iReadCursor +=sizeof(DLNode_t) + node->data.frame.frame_size;
		pcBuffer->m_iReadCursor = elem.frame.frame_pos + elem.frame.frame_size;
		if(pcBuffer->m_iReadCursor >= pcBuffer->m_iBufferSize)
			pcBuffer->m_iReadCursor -= pcBuffer->m_iBufferSize;
		
		if(elem.frame.stop_flag == TRUE){
			if(elem.frame.key_flag == TRUE){
				VLOG(VLOG_WARNING,"$$$$ free meet a idr $$$$$");
			}
			first_frame = FALSE;
			pcBuffer->m_iFrameCnt --;
		}
		//if(first_frame == TRUE){
			//if((elem.frame.stop_flag == TRUE) && (i_total >= free_size)
			//	/*&& (node->data.frame.key_flag == FALSE) */){
			//	if(elem.frame.key_flag == TRUE){
			//		VLOG(VLOG_WARNING,"release a key frame buffer");
			//	}
			//	break;
			//}
		//}
		if(elem.frame.type == RTP_FRAME_TYPE_ALAW || elem.frame.type == RTP_FRAME_TYPE_ULAW){
			if(elem.frame.stop_flag == TRUE && i_total >= free_size)
				break;
		}
	}while(TRUE);
	
	return RET_OK;
}
//
//
//

/************************************************************
** frame interfaces
*************************************************************/
int CircleAddRtpFrame(CircleBuffer_t* pCBuffer,RtpFrameInfo_t *info)
{
	int ret;
    unsigned int iBytesToWrite = info->frame_size;
    BYTE* pReadCursor = (BYTE*)info->frame_pos;
	unsigned int iChunkSize;
	unsigned int i_free;
	MillisecondTimer_t tTimerTemp;
	MilliSecond_t iLastTime;
	float iBandWidth = 0;
	unsigned int iExpectedBufferSize = 0;
	
	if(pCBuffer == NULL){
        VLOG(VLOG_ERROR,"circle buffer is null");
		exit(1);
	}
	if(pCBuffer->m_pBuffer == NULL){
        VLOG(VLOG_ERROR,"circle buffer is null");
		exit(1);
	}
	//  start timer if add data firstly
	if(MilliTimerIsClear(pCBuffer->m_StartTime)==TRUE){
		MilliTimerStart(pCBuffer->m_StartTime);
	}
	//static int flag=FALSE;
	//if(pCBuffer->m_iReadCursor == 0 && pCBuffer->m_iWriteCursor==0 && flag == FALSE){
	//	MilliTimerStart(pCBuffer->m_StartTime);
	//	flag = TRUE;
	//}
	// lock the buffer
    LOCK_lock(&pCBuffer->m_csCircleBuffer);
	//
	MilliTimerStop(pCBuffer->m_StartTime,tTimerTemp,iLastTime);
	// realloc the buffer size , if necessary
	if(pCBuffer->m_bRealloc == FALSE){
		if((iLastTime >= (pCBuffer->m_iBufferTime)) || // exceed the buffer time
			((pCBuffer->m_iWriteCursor+sizeof(DLNode_t)+info->frame_size) >= pCBuffer->m_iBufferSize)){//exceed the buffer size
			// then caculate the bandwidth and buffer size needed
			if(iLastTime <  pCBuffer->m_iBufferTime)
				iBandWidth = (float)(pCBuffer->m_iWriteCursor/iLastTime); // bytes/ms
			else
				iBandWidth = (float)(pCBuffer->m_iWriteCursor/pCBuffer->m_iBufferTime); // bytes/ms
			iExpectedBufferSize = (unsigned int)(iBandWidth * pCBuffer->m_iBufferTime * 2.0);
			if(iExpectedBufferSize < pCBuffer->m_iBufferSize/2){
				iExpectedBufferSize = pCBuffer->m_iBufferSize/2;
			}
			pCBuffer->m_pBuffer =realloc(pCBuffer->m_pBuffer,iExpectedBufferSize);
			if(pCBuffer->m_pBuffer==NULL){
				LOCK_unlock(&pCBuffer->m_csCircleBuffer);
				VLOG(VLOG_ERROR,"realloc buffer size:%d time:%d failed",iExpectedBufferSize,iLastTime);
				return -1;
			}
			pCBuffer->m_iBufferSize = iExpectedBufferSize;
			pCBuffer->m_bRealloc = TRUE;
			// must reconfigure the first node
			VLOG(VLOG_CRIT,"$$$$$ realloc buffer size:%d ok, bw:%f B/s time:%d $$$$$$$$",iExpectedBufferSize,iBandWidth*1000,iLastTime);
		}
	}
	if((iLastTime >= pCBuffer->m_iBufferTime) && (pCBuffer->m_bAvailable==FALSE)){
		pCBuffer->m_bAvailable = TRUE;
		VLOG(VLOG_CRIT,"buffer is ready now!!!!!");
	}

	// get free size
    if(pCBuffer->m_iWriteCursor < pCBuffer->m_iReadCursor)
        i_free = (pCBuffer->m_iReadCursor-1) - pCBuffer->m_iWriteCursor;
    else if(pCBuffer->m_iWriteCursor == pCBuffer->m_iReadCursor)
        i_free = pCBuffer->m_iBufferSize;
    else
        i_free = (pCBuffer->m_iReadCursor-1) + (pCBuffer->m_iBufferSize - pCBuffer->m_iWriteCursor);

	
	// make sure there is enough buffer ...
	if(iBytesToWrite > i_free){
		VLOG(VLOG_ERROR,"ERROR:no enough buffer,RP:%d WP:%d free:%d need:%d",
			pCBuffer->m_iReadCursor,pCBuffer->m_iWriteCursor,
			i_free,iBytesToWrite);
					
#ifdef ENABLE_BUFFER_OVERWRITE
			ret=CircleFreeAFrame(pCBuffer,iBytesToWrite-i_free);
			if(ret == FREE_MEET_EMPTY_BUFFER){
				LOCK_unlock(&pCBuffer->m_csCircleBuffer);
				//MSLEEP(5);
				return CBUFFER_RET_NODATA;
			}else if(ret == FREE_MEET_IDR){
				LOCK_unlock(&pCBuffer->m_csCircleBuffer);
				return RET_OK;
			}
#else
		return RET_FAIL;
#endif
	}

	// caculate the inspeed
	if(pCBuffer->m_bRealloc == TRUE){
		if(iLastTime>=(pCBuffer->m_iBufferTime)){
			pCBuffer->m_fInSpeed =(float)(2.0*pCBuffer->m_iTotalBytes/pCBuffer->m_iBufferSize);
			pCBuffer->m_iTotalBytes = 0;
			MilliTimerStart(pCBuffer->m_StartTime);
			VLOG(VLOG_DEBUG,"inspeed: %f ",pCBuffer->m_fInSpeed);
		}
	}

	//add dlink node
	info->frame_pos = pCBuffer->m_iWriteCursor;
	if(DLINK_insert_top(pCBuffer->m_FrameInfo,(DLElem_t *)info)==RET_FAIL){
		return RET_FAIL;
	}

	// fill data
    if(pCBuffer->m_iWriteCursor >= pCBuffer->m_iReadCursor)
    {
        iChunkSize = pCBuffer->m_iBufferSize - pCBuffer->m_iWriteCursor;
        if(iChunkSize > iBytesToWrite)
            iChunkSize = iBytesToWrite;

        memcpy(pCBuffer->m_pBuffer + pCBuffer->m_iWriteCursor,
               pReadCursor, iChunkSize);
        pReadCursor += iChunkSize;
        iBytesToWrite -= iChunkSize;

        pCBuffer->m_iWriteCursor += iChunkSize;
        if(pCBuffer->m_iWriteCursor >= pCBuffer->m_iBufferSize)
            pCBuffer->m_iWriteCursor -= pCBuffer->m_iBufferSize;
    }
	// remind , write in the header of buffer
    if(iBytesToWrite)
    {
        memcpy(pCBuffer->m_pBuffer + pCBuffer->m_iWriteCursor,
               pReadCursor, iBytesToWrite);
        pCBuffer->m_iWriteCursor += iBytesToWrite;
		if(pCBuffer->m_iWriteCursor >= pCBuffer->m_iBufferSize)
		{
	        VLOG(VLOG_ERROR,"write cursor is overflow,WP:%d RP:%d ",pCBuffer->m_iWriteCursor,pCBuffer->m_iReadCursor);
			exit(1);//csp ???
		}
    }

	// recaculate free size and then caculate used rate of buffer
    if(pCBuffer->m_iWriteCursor < pCBuffer->m_iReadCursor)
        i_free = (pCBuffer->m_iReadCursor-1) - pCBuffer->m_iWriteCursor;
    else if(pCBuffer->m_iWriteCursor == pCBuffer->m_iReadCursor)
        i_free = pCBuffer->m_iBufferSize;
    else
        i_free = (pCBuffer->m_iReadCursor-1) + (pCBuffer->m_iBufferSize - pCBuffer->m_iWriteCursor);
	pCBuffer->m_fUsedRate = (float)((1-RTPBUF_USED_RATE_FACTOR)*pCBuffer->m_fUsedRate+RTPBUF_USED_RATE_FACTOR*(pCBuffer->m_iBufferSize-i_free)/pCBuffer->m_iBufferSize);
	pCBuffer->m_iTotalBytes += info->frame_size;

	// check is there a new complete frame or not
	if(info->stop_flag == TRUE){
		pCBuffer->m_iFrameCnt ++;
	}

	// unlock buffer and post event to note other people it's available now
    EVT_post(pCBuffer->m_evtDataAvailable);
    LOCK_unlock(&pCBuffer->m_csCircleBuffer);	
	
	VLOG(VLOG_DEBUG,"DBG: add data success,size:%d seq:%d ts:%u WP:%d RP:%d used:%f%%",info->frame_size,
		info->seq,info->timestamp,pCBuffer->m_iWriteCursor,pCBuffer->m_iReadCursor,
		pCBuffer->m_fUsedRate);
	
	return RET_OK;
}

int CircleNextCompleteFrame(CircleBuffer_t* pCBuffer,void *out,int *out_size,unsigned int *timestamp)
{
	//DLNode_t *node=NULL;
	int pull_entry=0;
	DLElem_t elem;
	unsigned char *ptr=(unsigned char *)out;
	//const unsigned char *pBufEnd=pCBuffer->m_pBuffer+pCBuffer->m_iBufferSize;
	unsigned int iBytesToRead = 0;
	unsigned int iBytesRead = 0;
	unsigned int i_free=0;
	//unsigned int iAvaiBytes = 0;
	unsigned int iRPBak;
	unsigned int iSeq = 0xFFFFFFFF;
	unsigned int iTimestamp = 0xFFFFFFFF;
	int bComplete = FALSE;
	unsigned int iSleep=1;
	
	if(pCBuffer == NULL)
	{
        VLOG(VLOG_ERROR,"circle buffer is null");
		exit(1);
	}
	if(pCBuffer->m_pBuffer == NULL)
	{
        VLOG(VLOG_ERROR,"circle buffer is null");
		return RET_FAIL;
	}

	*out_size = 0;

	// waiting for data is available
	while(pCBuffer->m_iFrameCnt == 0)
	{
		MSLEEP(1);
		iSleep++;
		if(iSleep > 500)
		{        
			VLOG(VLOG_ERROR,"circle buffer hasn't a complete frame!");
			return RET_FAIL;
		}
	}
	iSleep = 1;

	while(bComplete == FALSE)
	{
		//VLOG(VLOG_DEBUG,"waitting for available....");
#if defined(_WIN32) || defined(_WIN64)
		unsigned long dwWaitResult;
		EVT_wait(pCBuffer->m_evtDataAvailable, CIC_WAITTIMEOUT,dwWaitResult);
		if(dwWaitResult == WAIT_TIMEOUT)
		{
			VLOG(VLOG_ERROR,"Circle buffer - did not fill in time!");
			*out_size = iBytesRead;
			return RET_FAIL;
		}
#else
		int dwWaitResult;
		EVT_wait(pCBuffer->m_evtDataAvailable, CIC_WAITTIMEOUT,dwWaitResult);
		if(dwWaitResult != 0){
			VLOG(VLOG_ERROR,"Circle buffer - did not fill in time!");
			*out_size = iBytesRead;
			return RET_FAIL;
		}
#endif
		if(pCBuffer->m_pBuffer == NULL){
			VLOG(VLOG_ERROR,"circle buffer is null");
			return RET_FAIL;
		}

		//VLOG(VLOG_DEBUG,"data is available....");
		LOCK_lock(&pCBuffer->m_csCircleBuffer);
		// read all rtp frames
		do{
			iRPBak = pCBuffer->m_iReadCursor;
			pull_entry=DLINK_pull_by_seq_and_del(pCBuffer->m_FrameInfo,&elem);
			if(pull_entry==RET_FAIL)
				break;// link is empty,break 
			VLOG(VLOG_DEBUG,"seq:%d timestamp:%d",elem.frame.seq,elem.frame.timestamp);
			if(elem.frame.magic != RTP_FRAME_INFO_MAGIC){
				VLOG(VLOG_ERROR,"rtp frame info invalid,please check it");
				LOCK_unlock(&pCBuffer->m_csCircleBuffer);
				return RET_FAIL;
			}
			if(iBytesRead == 0){// get start flag
				if(elem.frame.start_flag != 1){
					// ignore this frame
					goto DELETE_NODE_AND_UPDATE_RP;
				}
			}
			// check timestamp and seq
			if(iSeq != 0xFFFFFFFF){
				if(((elem.frame.seq != (iSeq + 1)) && (!(iSeq == 65535) && (elem.frame.seq == 0))) ||
					(elem.frame.timestamp != iTimestamp)){
					VLOG(VLOG_ERROR,"seq number:%d(last:%d) is invalid,please check it",elem.frame.seq,iSeq);
#ifdef SEQUENCE_WRONG_ALLOW
					iSeq = 0xffffffff;
					iBytesRead = 0;
					ptr=(unsigned char *)out;
					iBytesToRead = 0;
					goto DELETE_NODE_AND_UPDATE_RP;
#else
					LOCK_unlock(&pCBuffer->m_csCircleBuffer);
					return RET_FAIL;
#endif
				}
			}
			iSeq = elem.frame.seq;
			iTimestamp = elem.frame.timestamp;
			// start to write data
			iBytesToRead = elem.frame.frame_size;
			iRPBak = elem.frame.frame_pos;
			if(iRPBak > pCBuffer->m_iWriteCursor){
				unsigned int iChunkSize = pCBuffer->m_iBufferSize-iRPBak;
				if(iChunkSize > iBytesToRead)
					iChunkSize = iBytesToRead;
				
				memcpy(ptr,iRPBak+pCBuffer->m_pBuffer,iChunkSize);

				ptr+=iChunkSize;
				iBytesRead += iChunkSize;
				iBytesToRead -= iChunkSize;
				iRPBak += iChunkSize;
				if(iRPBak >= pCBuffer->m_iBufferSize)
					iRPBak -= pCBuffer->m_iBufferSize;
			}
			// remind ,read in the header of buffer
			if((iBytesToRead) && iRPBak < pCBuffer->m_iWriteCursor)
			{
				unsigned int iChunkSize = pCBuffer->m_iWriteCursor - iRPBak;
				if(iChunkSize > iBytesToRead)
					iChunkSize = iBytesToRead;
			
				memcpy(ptr,pCBuffer->m_pBuffer + iRPBak,iChunkSize);
			
				ptr+=iChunkSize;
				iBytesRead += iChunkSize;
				iBytesToRead -= iChunkSize;
				iRPBak += iChunkSize;
			}
			// check complete
			if(iBytesToRead != 0){
				VLOG(VLOG_ERROR,"ERR: not read done this frame,check it");
				return RET_FAIL;
			}

		DELETE_NODE_AND_UPDATE_RP:
			if(pull_entry==1){
				//pCBuffer->m_iReadCursor = node->data.frame.frame_pos-pCBuffer->m_pBuffer+
				//	node->data.frame.frame_size;
				pCBuffer->m_iReadCursor = iRPBak;
				VLOG(VLOG_DEBUG,"DEBUG: change RP: %d WP:%d",iRPBak,pCBuffer->m_iWriteCursor);
			}

			VLOG(VLOG_INFO,"DEBUG: readed:%d seq: %d timestamp:%d size:%d",iBytesRead,elem.frame.seq,
				elem.frame.timestamp,elem.frame.frame_size);

			if((elem.frame.stop_flag==1) && (iBytesRead > 0)){
				pCBuffer->m_iFrameCnt--;
				bComplete=TRUE;
				break;
			}else if(elem.frame.stop_flag==1){
				pCBuffer->m_iFrameCnt--;
			}

			if((iBytesRead/(100*1024)) == iSleep){
				iSleep++;
				LOCK_unlock(&pCBuffer->m_csCircleBuffer);
				MSLEEP(1);				
				LOCK_lock(&pCBuffer->m_csCircleBuffer);
			}
		}while(1);// this is end frame
		
        if(pCBuffer->m_iReadCursor != pCBuffer->m_iWriteCursor)
			EVT_post(pCBuffer->m_evtDataAvailable);
		LOCK_unlock(&pCBuffer->m_csCircleBuffer);
		MSLEEP(1);
	}

	
	// caculate free size and then caculate used rate of buffer
    if(pCBuffer->m_iWriteCursor < pCBuffer->m_iReadCursor)
        i_free = (pCBuffer->m_iReadCursor-1) - pCBuffer->m_iWriteCursor;
    else if(pCBuffer->m_iWriteCursor == pCBuffer->m_iReadCursor)
        i_free = pCBuffer->m_iBufferSize;
    else
        i_free = (pCBuffer->m_iReadCursor-1) + (pCBuffer->m_iBufferSize - pCBuffer->m_iWriteCursor);
	pCBuffer->m_fUsedRate = (float)((1-RTPBUF_USED_RATE_FACTOR)*pCBuffer->m_fUsedRate+RTPBUF_USED_RATE_FACTOR*(pCBuffer->m_iBufferSize-i_free)/pCBuffer->m_iBufferSize);

	VLOG(VLOG_DEBUG,"INFO: read done,size:%d rp:%d timestamp:%d",iBytesRead,iRPBak,iTimestamp);
	*out_size = iBytesRead;
	*timestamp = iTimestamp;

	//pCBuffer->m_iFrameCnt--;
	
	return RET_OK;
}

int CircleNextFrameTimeStamp(CircleBuffer_t *pCBuffer,unsigned int *timestamp)
{
	int ret;
	DLElem_t elem;
	LOCK_lock(&pCBuffer->m_csCircleBuffer);
	ret = DLINK_pull_by_seq(pCBuffer->m_FrameInfo,&elem);
	LOCK_unlock(&pCBuffer->m_csCircleBuffer);
	if(ret == RET_FAIL) return RET_FAIL;
	*timestamp = elem.frame.timestamp;
	return 0; 
}

int CircleIsAvailable(CircleBuffer_t* pCBuffer)
{
	return pCBuffer->m_bAvailable;
}

int CircleDestroy(CircleBuffer_t* pCBuffer)
{
	if(pCBuffer){
		if(pCBuffer->m_pBuffer){
			free(pCBuffer->m_pBuffer);
			pCBuffer->m_pBuffer = 0;
		}
		LOCK_destroy(&pCBuffer->m_csCircleBuffer);
		EVT_destroy(pCBuffer->m_evtDataAvailable);
		DLINK_destroy(pCBuffer->m_FrameInfo);
		pCBuffer->m_FrameInfo = NULL;
		free(pCBuffer);
	}
	
	VLOG(VLOG_CRIT,"circle buffer destroy success");

	return 0;
}

//

CircleBuffer_t* CIRCLEBUFFER_new(const unsigned int iBufferSize,const int buffer_time)
{
    CircleBuffer_t* pNewBuffer = (CircleBuffer_t*)malloc(sizeof(CircleBuffer_t));

    pNewBuffer->Write = (pfn_CircleBufferWrite)CircleBufferWrite;
    pNewBuffer->Read = (pfn_CircleBufferRead)CircleBufferRead;
    pNewBuffer->Flush = (pfn_CircleFlush)CircleFlush;
    pNewBuffer->GetUsedSize = (pfn_CircleGetUsedSpace)CircleGetUsedSpace;
    pNewBuffer->GetFreeSize = (pfn_CircleGetFreeSpace)CircleGetFreeSpace;

	pNewBuffer->NextFrame = (pfn_GetNextFrame)CircleNextCompleteFrame;
	pNewBuffer->NextFrameTS = (pfn_GetNextFrimeTimestamp)CircleNextFrameTimeStamp;
	pNewBuffer->AddRtpFrame = (pfn_AddRtpFrame)CircleAddRtpFrame;
	pNewBuffer->IsAvailable = (pfn_IsAvailable)CircleIsAvailable;
	
	pNewBuffer->Destroy = (pfn_Destroy)CircleDestroy;
	
    pNewBuffer->m_iBufferSize = iBufferSize;
    pNewBuffer->m_pBuffer = (BYTE*)malloc(iBufferSize);
	if(pNewBuffer->m_pBuffer == NULL){
		VLOG(VLOG_DEBUG,"ERR: malloc buffer failed");
		return NULL;
	}
    pNewBuffer->m_iReadCursor = 0;
    pNewBuffer->m_iWriteCursor = 0;
	pNewBuffer->m_iTotalBytes = 0;
	pNewBuffer->m_iFrameCnt = 0;
	pNewBuffer->m_bAvailable = FALSE;
	pNewBuffer->m_bRealloc = FALSE;
	pNewBuffer->m_iBufferTime = buffer_time;
	pNewBuffer->m_fInSpeed = 1;
	pNewBuffer->m_fUsedRate = 0;

    EVT_init(pNewBuffer->m_evtDataAvailable, FALSE);
    LOCK_init(&pNewBuffer->m_csCircleBuffer);

	if(DLINK_init(&pNewBuffer->m_FrameInfo)==RET_FAIL){
		free(pNewBuffer->m_pBuffer);
		return NULL;
	}
	//
	//MilliTimerStart(pNewBuffer->m_StartTime);
	MilliTimerSet(pNewBuffer->m_StartTime,0,0);

	VLOG(VLOG_CRIT,"create circle buffer(size:%d) success",iBufferSize);
    return pNewBuffer;
}

//
//
#ifdef LINUX
#include <pthread.h>

CircleBuffer_t *cbuffer=NULL;

void *read_proc(void *para)
{
#define READ_SIZE	64
	int i;
	int readed;
	char buf[128];
	for(i=0;i<100;i++){
		readed = 0;
		cbuffer->Read(cbuffer,buf,READ_SIZE,&readed);
		if(readed ==READ_SIZE){
			buf[readed]=0;
			printf("read %d :%s\n",readed,buf);
			usleep(500*1000);
		}else{
			printf("read faile: %d\n",readed);
		}
	}
	
	return NULL;
}
void *write_proc(void *para)
{
#define WRITE_SIZE	32
	int i,j;
	char buf[128];
	for(i=0;i<100;i++){
		for(j=0;j<WRITE_SIZE;j++){
			buf[j]='A'+i;
		}
		buf[j]=0;
		cbuffer->Write(cbuffer,buf,WRITE_SIZE);
		printf("{{write}} :%s\n",buf);
		usleep(1000*1000);
	}
	
	return NULL;
}
int CircleTest(int argc,char *argv[])
{
	cbuffer=CIRCLEBUFFER_new(128*1024,100);
	if(cbuffer==NULL){
		exit(1);
	}
	pthread_t thread1,thread2;
	pthread_create(&thread1,NULL,read_proc,NULL);
	pthread_create(&thread2,NULL,write_proc,NULL);

	while(1);
	return 0;
}
#endif


