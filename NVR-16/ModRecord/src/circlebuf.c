#include "common_basetypes.h"
//#include "mod_common.h"
#include "circlebuf.h"

s32 CreateCircleBuf(SCircleBufInfo* info)
{
	if((NULL == info) || (info->nLength <= 0))
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	u8 *head = (u8 *)malloc(info->nLength);
	if(NULL == head)
	{
		fprintf(stderr,"Invalid pointer returned , malloc failed !\n");
		return EM_REC_MALLOC_ERR;
	}
	
	info->nBufAddr = head;
	info->nWrOffset = 0;
	info->nRdOffset = 0;
	info->nWrOffsetPast = 0;
	info->nRdOffsetPast = 0;
	info->nLenUsed = 0;
	
	if(0 != sem_init(&info->sem, 0, 1))
	{
		fprintf(stderr,"sem_init failed ! function: %s\n", __FUNCTION__);
		
		//csp modify
		free(info->nBufAddr);
		info->nBufAddr = NULL;
		
		return EM_REC_SEMINIT_FAILED;
	}
	
	return EM_REC_SUCCESS;
}

s32 DestroyCircleBuf(SCircleBufInfo* info)
{
	if((NULL == info) || (NULL == info->nBufAddr))
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	sem_wait(&info->sem);
	
	if(info->nBufAddr)
	{
		free(info->nBufAddr);
	}
	info->nBufAddr = NULL;
	
	sem_destroy(&info->sem);
	
	return EM_REC_SUCCESS;
}

s32 ResetCircleBuf(SCircleBufInfo* info)
{
	if(NULL == info)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	sem_wait(&info->sem);
	
	info->nWrOffset = 0;
	info->nRdOffset = 0;
	info->nWrOffsetPast = 0;
	info->nRdOffsetPast = 0;
	info->nLenUsed = 0;
	
	//csp modify
	//if(0 != sem_init(&info->sem, 0, 1))
	//{
	//	fprintf(stderr,"sem_init failed ! function: %s\n", __FUNCTION__);
	//	return EM_REC_SEMINIT_FAILED;
	//}
	
	//csp modify
	sem_post(&info->sem);
	
	return EM_REC_SUCCESS;
}

s32 WriteDataToBuf(SCircleBufInfo* info, u8* data_addr, u32 length)
{
	if((NULL == info) || (NULL == data_addr) || (length <= 0))
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	sem_wait(&info->sem);
	
	if(info->nLenUsed + length > info->nLength)
	{
		fprintf(stderr, "%s: data is too long,write[%u],used[%u],total[%u]\n",__FUNCTION__,length,info->nLenUsed,info->nLength);
		sem_post(&info->sem);
		return EM_REC_BUF_NO_SPACE;
	}
	
	info->nWrOffsetPast = info->nWrOffset;
	
	if(info->nWrOffset + length >= info->nLength)
	{
		u8* des_addr = info->nBufAddr + info->nWrOffset;
		u32 len = info->nLength - info->nWrOffset;
		memcpy(des_addr, data_addr, len);
		des_addr = info->nBufAddr;
		data_addr += len;
		len = length - len;
		memcpy(des_addr, data_addr, len);
		info->nWrOffset = (info->nWrOffset + length - info->nLength);
	}
	else
	{
		u8* des_addr = info->nBufAddr + info->nWrOffset;
		memcpy(des_addr, data_addr, length);
		info->nWrOffset += length;
	}
	info->nLenUsed += length;
	
	sem_post(&info->sem);
	
	return EM_REC_SUCCESS;
}

s32 ReadDataFromBuf(SCircleBufInfo* info, u8* data_addr, u32 length)
{
	if((NULL == info) || (NULL == data_addr) || (length <= 0))
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	//fprintf(stderr, "function: %s,  all[%u], wanna[%u]\n",__FUNCTION__,info->nLenUsed,length);
	
	sem_wait(&info->sem);
	
	if(info->nLenUsed < length)
	{
		if (info->nLenUsed != 0)
			fprintf(stderr, "function: %s, The data you wanna read is too long, will not read buffer ! all[%u], wanna[%u]\n",__FUNCTION__,info->nLenUsed,length);
		
		info->nLenUsed = 0; //是否得当? 值得考虑
		info->nWrOffset = 0;
		info->nRdOffset = 0;
		info->nWrOffsetPast = 0;
		info->nRdOffsetPast = 0;
		
		sem_post(&info->sem);
		
		return EM_REC_BUF_NO_DATA;
	}
	
	info->nRdOffsetPast = info->nRdOffset;
	
	if(info->nRdOffset + length >= info->nLength)
	{
		u8* src_addr = info->nBufAddr + info->nRdOffset;
		u32 len = info->nLength - info->nRdOffset;
		memcpy(data_addr, src_addr, len);
		src_addr = info->nBufAddr;
		data_addr += len;
		len = length - len;
		memcpy(data_addr, src_addr, len);
		info->nRdOffset = (info->nRdOffset + length - info->nLength);
	}
	else
	{
		u8* src_addr = info->nBufAddr + info->nRdOffset;
		memcpy(data_addr, src_addr, length);
		info->nRdOffset += length;
	}
	info->nLenUsed -= length;
	
	sem_post(&info->sem);
	
	return EM_REC_SUCCESS;
}

s32 ResumeCircleBufToPast(SCircleBufInfo* info, u8 nWr, u8 nRd)
{
	if(NULL == info)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	sem_wait(&info->sem);
	
	if(nWr)
	{
		u32 len = (info->nWrOffset < info->nWrOffsetPast) ? (info->nWrOffset + info->nLength - info->nWrOffsetPast) : (info->nWrOffset - info->nWrOffsetPast);
		if(info->nLenUsed < len)
		{
			printf("Warnning: [W] sth may be wrong! function: %s\n", __FUNCTION__);
			
			//csp modify
			//info->nWrOffset = (info->nWrOffset < info->nLenUsed) ? (info->nWrOffset + info->nLength - info->nLenUsed) : (info->nWrOffset - info->nLenUsed);
			//info->nWrOffsetPast = info->nWrOffset;
			//info->nLenUsed = 0;
			
			//csp modify
			info->nWrOffset = 0;
			info->nRdOffset = 0;
			info->nWrOffsetPast = 0;
			info->nRdOffsetPast = 0;
			info->nLenUsed = 0;
			sem_post(&info->sem);
			printf("ResumeCircleBufToPast Wr failed......\n");
			fflush(stdout);
			//exit(-1);
			return EM_REC_SUCCESS;
		}
		else
		{
			info->nLenUsed -= len;
			info->nWrOffset = info->nWrOffsetPast;	
		}	
	}
	if(nRd)
	{
		u32 len = (info->nRdOffset < info->nRdOffsetPast) ? (info->nRdOffset + info->nLength - info->nRdOffsetPast) : (info->nRdOffset - info->nRdOffsetPast);
		if(info->nLenUsed + len > info->nLength)
		{
			printf("Warnning: [R] sth may be wrong! function: %s\n", __FUNCTION__);
			
			//csp modify
			//info->nLenUsed = info->nLength;
			
			//csp modify
			info->nWrOffset = 0;
			info->nRdOffset = 0;
			info->nWrOffsetPast = 0;
			info->nRdOffsetPast = 0;
			info->nLenUsed = 0;
			sem_post(&info->sem);
			printf("ResumeCircleBufToPast Rd failed......\n");
			fflush(stdout);
			//exit(-1);
			return EM_REC_SUCCESS;
		}
		else
		{
			info->nLenUsed += len;
		}
		info->nRdOffset = info->nRdOffsetPast;
	}
	
	sem_post(&info->sem);
	
	return EM_REC_SUCCESS;
}

s32 SkipCircleBuf(SCircleBufInfo* info, u8 nWr, u8 nRd, u32 nSkipLen)
{
	if((NULL == info) || (nSkipLen > info->nLength))
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	sem_wait(&info->sem);
	
	if(nWr)
	{
		if(info->nLenUsed + nSkipLen > info->nLength)
		{
			fprintf(stderr, "function: %s, The len you wanna skip is too long, will not skip wr buffer ! wanna[%u]\n",__FUNCTION__,nSkipLen);
			sem_post(&info->sem);
			return EM_REC_BUF_NO_SPACE;
		}
		
		info->nWrOffsetPast = info->nWrOffset;
		info->nLenUsed += nSkipLen;
		
		//csp modify
 		//if(info->nLength < info->nWrOffset + nSkipLen)
 		if(info->nLength <= info->nWrOffset + nSkipLen)
		{
			info->nWrOffset = (info->nWrOffset + nSkipLen - info->nLength);
		}
		else
		{
			info->nWrOffset += nSkipLen;	
		}
	}
	if(nRd)
	{
		if(info->nLenUsed < nSkipLen)
		{
			fprintf(stderr, "function: %s, The len you wanna skip is too long, will not skip rd buffer ! wanna[%u]\n",__FUNCTION__,nSkipLen);
			sem_post(&info->sem);
			return EM_REC_BUF_NO_SPACE;
		}
		
		info->nRdOffsetPast = info->nRdOffset;
		info->nLenUsed -= nSkipLen;
		
		//csp modify
		//if(info->nLength < info->nRdOffset + nSkipLen)
 		if(info->nLength <= info->nRdOffset + nSkipLen)
		{
			info->nRdOffset = (info->nRdOffset + nSkipLen - info->nLength);
		}
		else
		{
			info->nRdOffset += nSkipLen;	
		}
	}

	sem_post(&info->sem);

	return EM_REC_SUCCESS;
}

