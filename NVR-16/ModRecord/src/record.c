//#include "mod_common.h"
#include <string.h>
#include "custommp4.h"
#include "diskmanage.h"
#include "partitionindex.h"
#include "hddcmd.h"
#include "mod_record.h"
#include "record.h"
#include "circlebuf.h"
#include "fs_yg.h"//yaogang hdd
#include "Mod_syscomplex.h"
#include "mod_config.h"
#include "ipcamera.h"


extern u32 getTimeStamp();


static u8 g_ChnNum = 0;
static u8 g_nMsgQNum = 0;
static u8 g_recordinit = 0;
static u8 g_recorddeinit = 1;
static u8 *g_RecordStatus = NULL;
static u8 *g_RecordStatusPast = NULL;
static u8 *g_RecordStatusDelay = NULL;
static u8 *g_ChnPaused = NULL;
static SCircleBufInfo *g_RecBufInfo = NULL;


static SCircleBufInfo *g_MsgQueueInfo = NULL;
static SModRecSchPara *g_RecSchPara = NULL;
static SModRecChnTrigger *g_RecChnTrigger = NULL;
static u32 *g_RecordPreTime = NULL;//报警预录时间
static u8 *g_IsPreRecording = NULL;
static u32 *g_RecordDelayTime = NULL; //报警延录时间
static u8 *g_SendMsgReadBuf = NULL;
static u8 *g_RecSchOnceDone = NULL;
static u8 *g_IsWritingFile = NULL;

static disk_manager* g_HddManager = NULL;

FPMODRECSTATUS RecordRefreshStatus = NULL;
FPMODRECEXCEPTION RecordException = NULL;
static time_t *g_LastTimeGetFrame = NULL;
static sem_t *nSemForBuffer = NULL;
static sem_t *g_semMsgQueue = NULL;

//yaogang modify20150115
static SCircleBufInfo g_SnapMsgQueueInfo;
static SCircleBufInfo g_RecSnapBufInfo;
//static sem_t SemForSnapBuffer;	//互斥
//static sem_t SemForSnapCount;//计数，Buffer中Snap的数量。
static pthread_mutex_t LockSnapBuf;
static pthread_cond_t CondSnapBuf;	//Buf为空，则线程RecordSnapFxn 阻塞
static volatile u32 BufSnapCnt;		//Buf中图片个数

static u8 g_DvrHasHdd = 0;
static u8 g_HddIsFullCover = 1;
static u8 g_IsModWorking = 0;
static u32 *g_RecChnBufLength = NULL;
static u8 *g_RecordChnReleased = NULL;
static u8 g_RecordHDDFormated = 1;
static u8* g_pTmpPara = NULL;

//NVR used
static u8** g_RecordCacheAddr = NULL;
static u32* g_RecordCacheLength = NULL;
static u32* g_RecordCacheUsed = NULL;

//static u8 g_RecordTemp = 0;
//static u8 g_Temp = 0;

//extern int tl_power_atx_check();

//yg modify
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static u8 g_diskuse[16] = {0};//per chn 1:sda 2:sdb
static u32 g_rec_disk_err_bitmask = 0;//1<<phinfo->disk_logic_idx;

//yaogang modify 20141118
//以硬盘逻辑编号按位排列
//disk logic index	 8765 4321
//eg disk_status = 0b0000 0100 表示逻辑SATA口3上的硬盘读写出错
typedef struct 
{
	int init_status;
	int disk_status;
	pthread_mutex_t lock;
}check_disk_io_err;
static check_disk_io_err check_disk_err;


//预录数据结构
//每通道一个环形管理
typedef struct 
{
	u8 in;	//指向下一个的写入位置，当前空间数据是无效的
	u8 out;	//指向最早写入的图片
	u8 total;
	sem_t sem;
} TypePreRecSnapChnCtl; 

static TypePreRecSnapChnCtl *p_PreRecSnapChnCtl = NULL;
static FILE *pf_PreRecSnap[2] = {NULL, NULL};	//126/127两个文件容器
#define PreRecSnapNumPerChn (100)
#define SnapSize (128 << 10)


//report to modsys_UpdateDiskInfoFxn() --> update Gui
//如果通道0写文件rec/b1/fly00000.ifv，pdiskuse[0] = 'b' -'a'+1=2;
void Record_get_disk_use_info(u8 pdiskuse[16], u32 *prec_disk_err_bitmask)
{
	pthread_mutex_lock(&g_mutex);
	memcpy(pdiskuse, g_diskuse, sizeof(g_diskuse));
	*prec_disk_err_bitmask = g_rec_disk_err_bitmask;//sda bit0, sdb bit1
	pthread_mutex_unlock(&g_mutex);
}

static u8 g_HotPlugFlag = 0;

s32 ModRecordSetHotPlugFlag(u8 flag)
{
	SSnapFxnMsg msg;
	s32 ret;
		
	g_HotPlugFlag = flag;
	printf("%s :%d\n", __func__, flag);

	memset(&msg, 0, sizeof(SSnapFxnMsg));
	msg.cause = 1;
	msg.type = flag ? TypeRecSnapStop : TypeRecSnapWillStart;

	printf("%s RecordSnapWriteMsgQueue type: %d\n", __func__, msg.type);
	ret = RecordSnapWriteMsgQueue(&msg);
	if (ret)
	{
		printf("%s RecordSnapWriteMsgQueue type: %d failed\n", __func__, msg.type);
	}
	
	return 0;
}

s32 RecordCreateBuf(u8 chn, u32 length)
{
	if(chn >= g_ChnNum || length == 0)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	g_RecBufInfo[chn].nBufId = chn;
	g_RecBufInfo[chn].nLength = length;
	
	s32 ret = EM_REC_SUCCESS;
	ret = CreateCircleBuf(&g_RecBufInfo[chn]);
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		//return EM_REC_RECORDCREATEBUF;
	}
	
	return ret;
}

s32 RecordDestroyBuf(u8 chn)
{
	if(chn >= g_ChnNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	s32 ret = EM_REC_SUCCESS;
	ret = DestroyCircleBuf(&g_RecBufInfo[chn]);
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		//return EM_REC_RECORDDESTROYBUF;
	}
	
	return ret;
}

s32 RecordWriteFrameToBuf(SModRecRecordHeader* header)
{
	//csp modify
	if(g_DvrHasHdd == 0)
	{
		return EM_REC_SUCCESS;
	}
	
	//csp modify
	if(g_IsModWorking == 0)
	{
		return EM_REC_SUCCESS;
	}
	
	//static u8 flag = 0;
	//struct timeval tv;
	static u8* uCountOfNoNeed = NULL;
	static u8* pFlagChnBufFull = NULL;
	static u8* pFlagChnReset = NULL;
	static time_t *pLastTimeResetBuf = NULL;
	if(uCountOfNoNeed == NULL)
	{
		uCountOfNoNeed = (u8*)calloc(g_ChnNum, sizeof(u8));
		memset(uCountOfNoNeed,0,g_ChnNum*sizeof(u8));//csp modify
	}
	if(pFlagChnBufFull == NULL)
	{
		pFlagChnBufFull = (u8*)calloc(g_ChnNum, sizeof(u8));
		memset(pFlagChnBufFull,0,g_ChnNum*sizeof(u8));//csp modify
	}
	if(pFlagChnReset == NULL)
	{
		pFlagChnReset = (u8*)calloc(g_ChnNum, sizeof(u8));
		memset(pFlagChnReset,0,g_ChnNum*sizeof(u8));//csp modify
	}
	if(pLastTimeResetBuf == NULL)
	{
		pLastTimeResetBuf = (time_t*)calloc(g_ChnNum, sizeof(time_t));
		memset(pLastTimeResetBuf,0,g_ChnNum*sizeof(time_t));//csp modify
	}
	
	s32 ret = EM_REC_SUCCESS;
	
	if(NULL == header)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	if(0 == header->nMediaType)//H.264
	{
		g_LastTimeGetFrame[header->nChn] = header->nTimeStamp / 1000;
	}
	
	//csp modify
	//if(((0 == (g_RecordStatus[header->nChn] | g_RecordStatusDelay[header->nChn])) && (0 == g_RecordPreTime[header->nChn])) || (1 == g_ChnPaused[header->nChn]))
	if(((0 == (g_RecordStatus[header->nChn] | g_RecordStatusDelay[header->nChn])) && (0 == g_RecordPreTime[header->nChn] || 0 == g_IsPreRecording[header->nChn])) || (1 == g_ChnPaused[header->nChn]))
	{
		if(uCountOfNoNeed[header->nChn]++ > 200)
		{
			uCountOfNoNeed[header->nChn] = 0;
			fprintf(stderr, "No need to send data! chn = [%d]; function: %s\n", header->nChn, __FUNCTION__);
		}
		return EM_REC_SUCCESS;
	}
	
#if 0
	while(g_IsWritingFile[header->nChn])
	{
		usleep(30 * 1000);
		flag = 1;
	}
	if(flag)
	{
		ret = RecordResetBuf(header->nChn);
		if(EM_REC_SUCCESS != ret)
		{
			fprintf(stderr, "RecordResetBuf[%d] error!!!\n", header->nChn);
			return ret;
		}
		else
		{
			flag = 0;		
		}
	}
#else
	#if 1//csp modify
	if(g_IsWritingFile[header->nChn])
	{
		return EM_REC_SUCCESS;
	}
	#endif
	#if 0//csp modify
	if((0 == g_IsPreRecording[header->nChn]) && (g_RecordPreTime[header->nChn]) && (0 == (g_RecordStatus[header->nChn] | g_RecordStatusDelay[header->nChn])))
	{
		return EM_REC_SUCCESS;
	}
	#endif
#endif
	
	if(0 == g_IsPreRecording[header->nChn])
	{
		if(g_SendMsgReadBuf[header->nChn] == 0)
		{
			#if 0
			//unsigned char cur_atx_flag = tl_power_atx_check();
			//if(cur_atx_flag == 0 || g_HotPlugFlag)//电池供电
			if(g_HotPlugFlag)
			{
				pFlagChnBufFull[header->nChn] = 0;
				pFlagChnReset[header->nChn] = 0;
				pLastTimeResetBuf[header->nChn] = time(NULL);
				RecordResetBuf(header->nChn);
				return EM_REC_SUCCESS;
			}
			#endif
			
			//u8 rate = RecordGetBufUseRate(header->nChn);
			//if(header->nChn == 7) printf("chn[%d],Frame Len:%d, bufUseRate[%d]\n",header->nChn,header->nDataLength,rate);
			//NVR used
			//if((rate > 50) || ((header->nDataLength + sizeof(SModRecRecordHeader)) > (g_RecBufInfo[header->nChn].nLength - g_RecBufInfo[header->nChn].nLenUsed)))
			if(((header->nDataLength + sizeof(SModRecRecordHeader)) > (g_RecBufInfo[header->nChn].nLength - g_RecBufInfo[header->nChn].nLenUsed)))
			//if((rate > 50) || ((header->nDataLength + sizeof(SModRecRecordHeader)) > (g_RecBufInfo[header->nChn].nLength - g_RecBufInfo[header->nChn].nLenUsed)))
			{
				SModRecMsgHeader sMsg;
				memset(&sMsg, 0, sizeof(sMsg));
				sMsg.nChn = header->nChn;
				sMsg.nMsg = EM_REC_READ_BUF;
				sMsg.data = NULL;
				g_SendMsgReadBuf[header->nChn] = 1;//csp modify
				if(EM_REC_SUCCESS == RecordWriteMsgQueue(header->nChn / 8 + 1, &sMsg))
				{
					//struct timeval tv;
					//gettimeofday(&tv, NULL);
					//printf("$$$$$$$$$$$$$$$$$$$ chn[%d] send readMsg, bufUseRate[%d], time:%llu\n", header->nChn, rate, (u64)tv.tv_sec*1000000+tv.tv_usec);
					//g_SendMsgReadBuf[header->nChn] = 1;//csp modify//在这里置标志位有bug
				}
				else
				{
					g_SendMsgReadBuf[header->nChn] = 0;
					fprintf(stderr, "Write Msg error! %s\n", __FUNCTION__);
				}
			}
		}
		
		#if 0//NVR used
		sem_wait(&nSemForBuffer[header->nChn]);
		
		if((header->nDataLength + sizeof(SModRecRecordHeader)) > (g_RecBufInfo[header->nChn].nLength - g_RecBufInfo[header->nChn].nLenUsed))
		{
			sem_post(&nSemForBuffer[header->nChn]);
			
			printf("@@@$$$ chn[%d] recording, drop one frame\n", header->nChn);
			
			if(pFlagChnBufFull[header->nChn]++ > 20)
			{
				pFlagChnBufFull[header->nChn] = 0;
				RecordResetBuf(header->nChn);
				printf("chn[%d] RecordResetBuf OK! times[%d]\n", header->nChn, pFlagChnReset[header->nChn]);
				if(time(NULL) - pLastTimeResetBuf[header->nChn] > 600 || time(NULL) < pLastTimeResetBuf[header->nChn])
				{
					pFlagChnReset[header->nChn] = 0;
					pLastTimeResetBuf[header->nChn] = time(NULL);
				}
				pFlagChnReset[header->nChn]++;
			}
			
			return EM_REC_FRAMETOOBIG;
		}
		
		//sem_post(&nSemForBuffer[header->nChn]);
		
		pFlagChnBufFull[header->nChn] = 0;
		
		//sem_wait(&nSemForBuffer[header->nChn]);
		#else
		u8 flag_full = 0;
		while((header->nDataLength + sizeof(SModRecRecordHeader)) > (g_RecBufInfo[header->nChn].nLength - g_RecBufInfo[header->nChn].nLenUsed))
		{
			#if 0
			//unsigned char cur_atx_flag = tl_power_atx_check();
			//if(cur_atx_flag == 0 || g_HotPlugFlag)//电池供电
			if( g_HotPlugFlag)
			{
				flag_full = 0;
				pFlagChnBufFull[header->nChn] = 0;
				pFlagChnReset[header->nChn] = 0;
				pLastTimeResetBuf[header->nChn] = time(NULL);
				RecordResetBuf(header->nChn);
				return EM_REC_SUCCESS;
			}
			#endif
			
			#if 1
			usleep(750);
			#else
			tv.tv_sec = 0;
			tv.tv_usec = 500;
			select(0, NULL, NULL, NULL, &tv);
			#endif
			
			//printf("chn[%d],Frame Len:%d, bufUseRate[%d] &&\n", header->nChn,header->nDataLength,RecordGetBufUseRate(header->nChn));
			
			//NVR used
			if(flag_full++ == 4)
			//if(flag_full++ == 5)
			{
				printf("@@@$$$ chn[%d] ignore one frame\n", header->nChn);
				if(pFlagChnBufFull[header->nChn]++ > 5)
				{
					pFlagChnBufFull[header->nChn] = 0;
					RecordResetBuf(header->nChn);
					printf("chn[%d] RecordResetBuf OK! times[%d]\n", header->nChn, pFlagChnReset[header->nChn]);
					#if 1//csp modify
					if(time(NULL) - pLastTimeResetBuf[header->nChn] > 600 || time(NULL) < pLastTimeResetBuf[header->nChn])
					{
						pFlagChnReset[header->nChn] = 0;
						pLastTimeResetBuf[header->nChn] = time(NULL);
					}
					pFlagChnReset[header->nChn]++;
					#else
					if(time(NULL) - pLastTimeResetBuf[header->nChn] > 600)
					{
						pFlagChnReset[header->nChn] = 0;
						pLastTimeResetBuf[header->nChn] = time(NULL);
					}
					if(0)//(pFlagChnReset[header->nChn]++ == 20)
					{
						u8 tmp = 0;
						for(tmp = 0; tmp < g_ChnNum; tmp++)
						{
							if((tmp != header->nChn) && (pFlagChnReset[tmp] > 30))
							{
								break;
							}
						}
						printf("tmp = %d\n",tmp);
						if(tmp >= g_ChnNum)
						{
							printf("here stop record\n");
							RecordException(0);//硬盘异常
							ModRecordStopAll();
							usleep(1000);
							for(tmp = 0; tmp < g_ChnNum; tmp++)
							{
								ModRecordResume(tmp);
							}
						}
					}
					else if(0)//(pFlagChnReset[header->nChn]++ == 30)
					{
						/*SModRecMsgHeader sMsg;
						memset(&sMsg, 0, sizeof(sMsg));
						sMsg.data = NULL;
						sMsg.nMsg = EM_REC_FORCE_END;
						u8 chn = 0;
						for(chn = 0; chn < g_ChnNum; chn++)
						{
							sMsg.nChn = chn;
							if(EM_REC_SUCCESS != RecordWriteMsgQueue(chn / 8 + 1, &sMsg))
							{
								fprintf(stderr, "RecordWriteMsgQueue error!\n");
							}
						}
						usleep(3000*1000);*/
						RecordException(0);//硬盘异常
						RecordException(3);//重启主机
					}
					#endif
				}
				
				return EM_REC_FRAMETOOBIG;
			}
		}
		
		pFlagChnBufFull[header->nChn] = 0;
		
		sem_wait(&nSemForBuffer[header->nChn]);
		#endif
		
		ret = WriteDataToBuf(&g_RecBufInfo[header->nChn], (u8 *)header, sizeof(SModRecRecordHeader));
		if(EM_REC_SUCCESS != ret)
		{
			fprintf(stderr, "function: %s\n", __FUNCTION__);
			sem_post(&nSemForBuffer[header->nChn]);
			return ret;
		}
		ret = WriteDataToBuf(&g_RecBufInfo[header->nChn], header->nDate, header->nDataLength);
		if(EM_REC_SUCCESS != ret)
		{
			fprintf(stderr, "function: %s\n", __FUNCTION__);
			ResumeCircleBufToPast(&g_RecBufInfo[header->nChn], 1, 0);
			sem_post(&nSemForBuffer[header->nChn]);
			return ret;
		}
		
		sem_post(&nSemForBuffer[header->nChn]);
		
		#if 1
		//NVR used
		if(g_SendMsgReadBuf[header->nChn] == 0)
		{
			u8 rate = RecordGetBufUseRate(header->nChn);
			if(rate > 30) //> 50% ,启动保存//yaogang modify 20150313
			{
				SModRecMsgHeader sMsg;
				memset(&sMsg, 0, sizeof(sMsg));
				sMsg.nChn = header->nChn;
				sMsg.nMsg = EM_REC_READ_BUF;
				sMsg.data = NULL;
				g_SendMsgReadBuf[header->nChn] = 1;//csp modify
				if(EM_REC_SUCCESS == RecordWriteMsgQueue(header->nChn / 8 + 1, &sMsg))
				{
					//struct timeval tv;
					//gettimeofday(&tv, NULL);
					//printf("$$$$$$$$$$$$$$$$$$$ chn[%d] send readMsg, bufUseRate[%d], time:%llu\n", header->nChn, rate, (u64)tv.tv_sec*1000000+tv.tv_usec);
					//g_SendMsgReadBuf[header->nChn] = 1;//csp modify//在这里置标志位有bug
				}
				else
				{
					g_SendMsgReadBuf[header->nChn] = 0;
					fprintf(stderr, "Write Msg error! %s\n", __FUNCTION__);
				}
			}
		}
		#endif
	}
	else// if (1 == g_IsPreRecording[header->nChn])
	{
		//printf("***buf[%d] length:%d, header->nTimeStamp:%llu\n",header->nChn,g_RecBufInfo[header->nChn].nLenUsed, header->nTimeStamp);
		//printf("buf[%d] use rate: %d , frameLength:%u\n",header->nChn, RecordGetBufUseRate(header->nChn), header->nDataLength);
		
		ret = RecordUpdateBufReadOffset(header);
		if(EM_REC_SUCCESS != ret)
		{
			fprintf(stderr, "function: %s\n", __FUNCTION__);
			return ret;
		}
		
		//printf("buf[%d] use rate: %d\n",header->nChn, RecordGetBufUseRate(header->nChn));
	}
	
	return ret;
}

//yaogang modify 20170218 in shanghai  add MaxFrameSize
s32 RecordReadFrameFromBuf(u8 chn, u8* frame_data, s32 MaxFrameSize)
{
	SModRecRecordHeader header;
	memset(&header, 0, sizeof(header));
	
	if((chn >= g_ChnNum) || (NULL == frame_data))
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}

	if (chn >= g_ChnNum)
	{
		printf("error %s: chn(%d) >= g_ChnNum(%d)!\n", __func__, chn, g_ChnNum);
		return EM_REC_INVALID_PARA;
	}

	if (NULL == frame_data)
	{
		printf("error %s: NULL == frame_data!\n", __func__);
		return EM_REC_INVALID_PARA;
	}

	if (MaxFrameSize <= 0)
	{
		printf("error %s: MaxFrameSize[%d] <= 0!\n", __func__, MaxFrameSize);
		return EM_REC_INVALID_PARA;
	}
	
	sem_wait(&nSemForBuffer[chn]);
	
	s32 ret = EM_REC_SUCCESS;
	ret = ReadDataFromBuf(&g_RecBufInfo[chn], (u8 *)&header, sizeof(SModRecRecordHeader));
	if(EM_REC_SUCCESS != ret)
	{
		printf("error %s: Read RecordHeader failed!\n", __func__);
		sem_post(&nSemForBuffer[chn]);
		return ret;
	}
	
	if (header.nDataLength > (MaxFrameSize - sizeof(SModRecRecordHeader)))//超出给定的空间
	{
		if (header.nDataLength < (1<<20)) //小于1M，丢弃该帧，跳过
		{
			printf("warning %s: wanna[%u] > MaxFrameSize-header[%d], ignore it!\n", __func__, header.nDataLength, MaxFrameSize - sizeof(SModRecRecordHeader));
			SkipCircleBuf(&g_RecBufInfo[chn], 0, 1, header.nDataLength);
			ret = EM_REC_FRAMETOOBIG;
		}
		else //大于1M 认为缓存区出错
		{
			printf("error %s: wanna[%u] too larger\n", __func__, header.nDataLength);
			ret = EM_REC_BUF_NO_DATA;//为缓冲区出错暂定的错误值 yaogang modify 20170328
		}

		sem_post(&nSemForBuffer[chn]);
		return ret;
	}

	
	memcpy(frame_data, &header, sizeof(SModRecRecordHeader));

	#if 0
	//printf("this frame len : %u\n",header.nDataLength);
	if (header.nChn == 0 && header.nFrameType)
	{
		printf("%s chn%d Iframe len: %u\n", __func__, header.nChn, header.nDataLength);
	}
	#endif
	
	ret = ReadDataFromBuf(&g_RecBufInfo[chn], frame_data + sizeof(SModRecRecordHeader), header.nDataLength);
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s [2]\n", __FUNCTION__);
		ResumeCircleBufToPast(&g_RecBufInfo[chn], 0, 1);
		sem_post(&nSemForBuffer[chn]);
		return ret;
	}
	
	sem_post(&nSemForBuffer[chn]);
	
	return ret;
}

s32 RecordSkipOneFrameFromBuf(u8 chn)
{
	if(chn >= g_ChnNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	sem_wait(&nSemForBuffer[chn]);
	
	s32 ret = EM_REC_SUCCESS;
	u8 frame_data[128] = {0};//u8 frame_data[50] = {0};//csp modify
	ret = ReadDataFromBuf(&g_RecBufInfo[chn], frame_data, sizeof(SModRecRecordHeader));
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s   [1]\n", __FUNCTION__);
		sem_post(&nSemForBuffer[chn]);
		return ret;
	}
	
	SModRecRecordHeader header;
	memset(&header, 0, sizeof(header));
	memcpy(&header, frame_data, sizeof(header));
	
	ret = SkipCircleBuf(&g_RecBufInfo[chn], 0, 1, header.nDataLength);
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s   [2]\n", __FUNCTION__);
		ResumeCircleBufToPast(&g_RecBufInfo[chn], 0, 1);
		sem_post(&nSemForBuffer[chn]);
		return ret;
	}
	
	sem_post(&nSemForBuffer[chn]);
	
	return ret;
}

u8 RecordGetBufUseRate(u8 chn)
{
	if(chn >= g_ChnNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return (u8)EM_REC_INVALID_PARA;
	}
	
	u8 rate = 0;
	rate = g_RecBufInfo[chn].nLenUsed * 100 / g_RecBufInfo[chn].nLength;
	//printf("use rate: %d%c\n", rate, '%');
	return rate;
}

s32 RecordResetBuf(u8 chn)
{
	if(chn >= g_ChnNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	sem_wait(&nSemForBuffer[chn]);
	
	s32 ret = ResetCircleBuf(&g_RecBufInfo[chn]);
	
	sem_post(&nSemForBuffer[chn]);
	
	return ret;
}
s32 RecordResetSnapBuf()
{	
	s32 ret = 0;
	
	pthread_mutex_lock(&LockSnapBuf);
	
	ret = ResetCircleBuf(&g_RecSnapBufInfo);
	BufSnapCnt = 0;

	pthread_mutex_unlock(&LockSnapBuf);
	
	return ret;
}


s32 RecordUpdateBufReadOffset(SModRecRecordHeader* header)
{
	SCircleBufInfo bufinfo;
	
	s32 ret = EM_REC_SUCCESS;
	
	SModRecRecordHeader head;
	memset(&head, 0, sizeof(head));
	
	#if 0//csp modify
	static u8* frame_data = NULL;
	if(NULL == frame_data)
	{
		frame_data = calloc(256<<10, sizeof(u8));
		if(NULL == frame_data)
		{
			return EM_REC_MALLOC_ERR;
		}
	}
	#endif
	
	//覆盖最老的帧,打开文件时保证从关键帧开始录制
	while((RecordGetBufUseRate(header->nChn) > 95)
		 || (header->nDataLength + sizeof(SModRecRecordHeader)) > 
		    (g_RecBufInfo[header->nChn].nLength - g_RecBufInfo[header->nChn].nLenUsed))
	{
		//printf("**\n");
		ret = RecordSkipOneFrameFromBuf(header->nChn);
		if(EM_REC_SUCCESS != ret)
		{
			RecordResetBuf(header->nChn);
			break;
		}
	}
	
	//printf("buf[%d] length:%d, header->nTimeStamp:%llu\n",header->nChn,g_RecBufInfo[header->nChn].nLenUsed, header->nTimeStamp);
	
	sem_wait(&nSemForBuffer[header->nChn]);
	
	ret = WriteDataToBuf(&g_RecBufInfo[header->nChn], (u8 *)header, sizeof(SModRecRecordHeader));
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s  [1]\n", __FUNCTION__);
		sem_post(&nSemForBuffer[header->nChn]);
		return ret;
	}
	ret = WriteDataToBuf(&g_RecBufInfo[header->nChn], header->nDate, header->nDataLength);
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s [2]\n", __FUNCTION__);
		ResumeCircleBufToPast(&g_RecBufInfo[header->nChn], 1, 0);
		sem_post(&nSemForBuffer[header->nChn]);
		return ret;
	}
	
	//printf("buf[%d] length:%d, \n",header->nChn,g_RecBufInfo[header->nChn].nLenUsed);
	
	while((0 == header->nMediaType) && g_RecBufInfo[header->nChn].nLenUsed)//(1)
	{
		memcpy(&bufinfo, &g_RecBufInfo[header->nChn], sizeof(SCircleBufInfo));
		
		u8 frame_data[128] = {0};//u8 frame_data[50] = {0};//csp modify
		
		ret = ReadDataFromBuf(&g_RecBufInfo[header->nChn], frame_data, sizeof(SModRecRecordHeader));
		if(EM_REC_SUCCESS != ret)
		{
			fprintf(stderr, "function: %s [3]\n", __FUNCTION__);
			sem_post(&nSemForBuffer[header->nChn]);
			return ret;
		}
		memcpy(&head, frame_data, sizeof(head));
		
		//printf("head.nTimeStamp:%llu, header->nTimeStamp:%llu\n", head.nTimeStamp, header->nTimeStamp);
		if((0 == head.nMediaType)
			&& (head.nTimeStamp + 1000 * g_RecordPreTime[header->nChn] > header->nTimeStamp))
		{
			memcpy(&g_RecBufInfo[header->nChn], &bufinfo, sizeof(SCircleBufInfo));
			break;
		}
		else
		{
			ret = SkipCircleBuf(&g_RecBufInfo[header->nChn], 0, 1, head.nDataLength);
			if(EM_REC_SUCCESS != ret)
			{
				fprintf(stderr, "function: %s [4]\n", __FUNCTION__);
				ResumeCircleBufToPast(&g_RecBufInfo[header->nChn], 0, 1);
				sem_post(&nSemForBuffer[header->nChn]);
				return ret;
			}
		}
	}
	
	sem_post(&nSemForBuffer[header->nChn]);
	
	return ret;
}

s32 RecordCreateMsgQueue(u8 msgid)
{
	//NVR used
	if(msgid > 1)
	{
		msgid = 1;
	}
	
	if(msgid > g_nMsgQNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	g_MsgQueueInfo[msgid].nBufId = (-1 - msgid);
	
	//NVR used
	//g_MsgQueueInfo[msgid].nLength = 1024 * sizeof(SModRecMsgHeader);
	g_MsgQueueInfo[msgid].nLength = 4096 * sizeof(SModRecMsgHeader);
	
	s32 ret = EM_REC_SUCCESS;
	ret = CreateCircleBuf(&g_MsgQueueInfo[msgid]);
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}
	
	return ret;
}

s32 RecordDestroyMsgQueue(u8 msgid)
{
	//NVR used
	if(msgid > 1)
	{
		msgid = 1;
	}
	
	if(msgid > g_nMsgQNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	s32 ret = EM_REC_SUCCESS;
	ret = DestroyCircleBuf(&g_MsgQueueInfo[msgid]);
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}
	
	return ret;	
}

s32 RecordWriteMsgQueue(u8 msgid, SModRecMsgHeader* header)
{
	//NVR used
	if(msgid > 1)
	{
		msgid = 1;
	}
	
	if(msgid > g_nMsgQNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	s32 ret = EM_REC_SUCCESS;
	ret = WriteDataToBuf(&g_MsgQueueInfo[msgid], (u8 *)header, sizeof(SModRecMsgHeader));
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}
	
	if(msgid)
	{
		//int value = -1;
		//sem_getvalue(&g_semMsgQueue[msgid-1], &value);
		//if(0 == value)
			sem_post(&g_semMsgQueue[msgid-1]);
		//printf("value = %d\n",value);
		if(header->nMsg == EM_REC_FORCE_END)
		{
			int value = -1;
			sem_getvalue(&g_semMsgQueue[msgid-1], &value);
			printf("RecordWriteMsgQueue: sem value = %d\n",value);
		}
	}
	
	return ret;
}

s32 RecordReadMsgQueue(u8 msgid, SModRecMsgHeader* header)
{
	//NVR used
	if(msgid > 1)
	{
		msgid = 1;
	}
	
	if(msgid > g_nMsgQNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	s32 ret = EM_REC_SUCCESS;
	ret = ReadDataFromBuf(&g_MsgQueueInfo[msgid], (u8 *)header, sizeof(SModRecMsgHeader));
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return ret;
	}
	
	return ret;
}

s32 RecordDealRecMsg(SModRecMsgHeader* sMsg)
{
	if((NULL == sMsg) || (sMsg->nChn >= g_ChnNum))
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	u8 smgtype = ((sMsg->nMsg == EM_REC_TIMER) ? 0 : (1 + sMsg->nMsg));
	//printf("%s sMsg->nMsg: %d, smgtype: %u\n", __func__, sMsg->nMsg, smgtype);
	
	u8 i = 0;
	u8 j = 0;
	u8 k = 0;
	
	time_t nowtime = time(NULL);
	
	//csp modify 20131213
	nowtime += ModRecordGetTimeZoneOffset();
	
	//csp modify
	//struct tm *now = localtime(&nowtime);
	struct tm result;
	struct tm *now = &result;
	localtime_r(&nowtime, now);
	
	u32 sec = 0;
	u8 day = 0;
	
	SModRecSchPara RecSchPara;
	
	static u8 flag_num_trigger = 0;
	static u64 *chn_is_triggerring_me[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
	static u64 *chn_wanna_trigger_me[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
	
	if(flag_num_trigger == 0)
	{
		for(i = 0; i < 6; i++)
		{
			//csp modify
			if(chn_is_triggerring_me[i] != NULL)
			{
				free(chn_is_triggerring_me[i]);
				chn_is_triggerring_me[i] = NULL;
			}
			
			chn_is_triggerring_me[i] = (u64 *)malloc(g_ChnNum * sizeof(u64));
			if(chn_is_triggerring_me[i] == NULL)
			{
				fprintf(stderr, "function: %s\n", __FUNCTION__);
				return EM_REC_MALLOC_ERR;
			}
			memset(chn_is_triggerring_me[i], 0, g_ChnNum * sizeof(u64));	
		}
		
		for(i = 0; i < 6; i++)
		{
			//csp modify
			if(chn_wanna_trigger_me[i] != NULL)
			{
				free(chn_wanna_trigger_me[i]);
				chn_wanna_trigger_me[i] = NULL;
			}
			
			chn_wanna_trigger_me[i] = (u64 *)malloc(g_ChnNum * sizeof(u64));
			if(chn_wanna_trigger_me[i] == NULL)
			{
				fprintf(stderr, "function: %s\n", __FUNCTION__);
				return EM_REC_MALLOC_ERR;
			}
			memset(chn_wanna_trigger_me[i], 0, g_ChnNum * sizeof(u64));	
		}
		
		flag_num_trigger = 1;
	}
	
	for(i = 0; i < g_ChnNum; i++)
	{
		for(j = EM_REC_SCH_REC_TIMER; j <= EM_REC_SCH_REC_ALARM; j++)
		{
			if((now->tm_hour == 0) && (now->tm_min == 0) && (now->tm_sec >= 0) && (now->tm_sec <= 1))
			{
				g_RecSchOnceDone[i + j * g_ChnNum] = 1;
			}
		}
	}
	
	if((sMsg->nMsg == EM_REC_V_MOTION) || (sMsg->nMsg == EM_REC_V_ALARM))//报警触发录像暂时只支持这两类
	{
		memcpy(&RecSchPara, &g_RecSchPara[sMsg->nChn + smgtype * g_ChnNum], sizeof(RecSchPara));
		//printf("%s RecSchPara.nTimeType: %d\n", __func__, RecSchPara.nTimeType);
		switch(RecSchPara.nTimeType)
		{
			case EM_REC_SCH_ONCE_DAY:
				if(1 == g_RecSchOnceDone[sMsg->nChn + smgtype * g_ChnNum])
				{
					return EM_REC_SUCCESS;
				}
				break;
				
			case EM_REC_SCH_MONTH_DAY:
				if(0 == ((1 << (now->tm_mday - 1)) & RecSchPara.nValue))
				{
					return EM_REC_SUCCESS;
				}
				day = now->tm_mday - 1;
				break;
				
			case EM_REC_SCH_WEEK_DAY:
				if(0 == ((1 << now->tm_wday) & RecSchPara.nValue))
				{
					return EM_REC_SUCCESS;
				}
				day = now->tm_wday;
				break;
				
			case EM_REC_SCH_EVERY_DAY:
				day = 0;
				break;
				
			default:
				fprintf(stderr, "Invalid type of REC_SCH !\n");
				return EM_REC_SUCCESS;
		}
		
		sec = now->tm_hour * 3600 + now->tm_min * 60 + now->tm_sec;
		
		if(sMsg->nKey == 1)
		{
			for(j = 0; j < MAX_TIME_SEGMENTS; j++)
			{
				if((sec >= RecSchPara.nSchTime[day][j].nStartTime) 
					&& (sec <= RecSchPara.nSchTime[day][j].nStopTime))
				{
					g_RecordStatus[sMsg->nChn] |= (1 << (1 + smgtype));
					chn_is_triggerring_me[smgtype-1][sMsg->nChn] |= (1 << sMsg->nChn);
					//printf("%s g_RecordStatus: 0x%x\n", __func__, g_RecordStatus[sMsg->nChn]);
					break;
				}
			}
			
			if(j >= MAX_TIME_SEGMENTS)
			{
				chn_wanna_trigger_me[smgtype-1][sMsg->nChn] |= (1 << sMsg->nChn);
			}
		}
		else if(sMsg->nKey == 0)
		{
			chn_is_triggerring_me[smgtype-1][sMsg->nChn] &= ~(1 << sMsg->nChn);
			
			if(0 == chn_is_triggerring_me[smgtype-1][sMsg->nChn])
			{
				g_RecordStatus[sMsg->nChn] &= ~(1 << (1 + smgtype));
			}
			
			chn_wanna_trigger_me[smgtype-1][sMsg->nChn] &= ~(1 << sMsg->nChn);
		}
	}
	else if(sMsg->nMsg == EM_REC_TIMER)
	{
		for(i = 0; i < g_ChnNum; i++)
		{
			memcpy(&RecSchPara, &g_RecSchPara[i], sizeof(RecSchPara));
			
			switch(RecSchPara.nTimeType)
			{
				case EM_REC_SCH_ONCE_DAY:
					if(1 == g_RecSchOnceDone[i])
					{
						continue;
					}
					break;
					
				case EM_REC_SCH_MONTH_DAY:
					if(0 == ((1 << (now->tm_mday - 1)) & RecSchPara.nValue))
					{
						continue;
					}
					day = now->tm_mday - 1;
					break;
					
				case EM_REC_SCH_WEEK_DAY:
					if(0 == ((1 << now->tm_wday) & RecSchPara.nValue))
					{
						continue;
					}
					day = now->tm_wday;
					break;
					
				case EM_REC_SCH_EVERY_DAY:
					day = 0;
					break;
					
				default:
					fprintf(stderr, "Invalid type of REC_SCH !\n");
					continue;
			}
			
			sec = now->tm_hour * 3600 + now->tm_min * 60 + now->tm_sec;
			
			for(j = 0; j < MAX_TIME_SEGMENTS; j++)
			{
				if((sec >= RecSchPara.nSchTime[day][j].nStartTime) 
					&& (sec < RecSchPara.nSchTime[day][j].nStopTime)) //不能用 '<=' ,否则在0点0分0秒时会启动定时录像
				{
					g_RecordStatus[i] |= (1 << 1);
					break;
				}
			}
			
			if(j >= MAX_TIME_SEGMENTS)
			{
				g_RecordStatus[i] &= ~(1 << 1);
			}
		}
		
		for(i = 0; i < g_ChnNum; i++)
		{
			for(k = EM_REC_V_MOTION + 1; k <= EM_REC_V_ALARM + 1; k++)
			{
				memcpy(&RecSchPara, &g_RecSchPara[i + k * g_ChnNum], sizeof(RecSchPara));
				
				switch(RecSchPara.nTimeType)
				{
					case EM_REC_SCH_ONCE_DAY:
						if(1 == g_RecSchOnceDone[i + k * g_ChnNum])
						{
							continue;
						}
						break;
						
					case EM_REC_SCH_MONTH_DAY:
						if(0 == ((1 << (now->tm_mday - 1)) & RecSchPara.nValue))
						{
							continue;
						}
						day = now->tm_mday - 1;
						break;
						
					case EM_REC_SCH_WEEK_DAY:
						if(0 == ((1 << now->tm_wday) & RecSchPara.nValue))
						{
							continue;
						}
						day = now->tm_wday;
						break;
						
					case EM_REC_SCH_EVERY_DAY:
						day = 0;
						break;
						
					default:
						fprintf(stderr, "Invalid type of REC_SCH !\n");
						continue;
				}
				
				sec = now->tm_hour * 3600 + now->tm_min * 60 + now->tm_sec;
				
				for(j = 0; j < MAX_TIME_SEGMENTS; j++)
				{
					if((sec >= RecSchPara.nSchTime[day][j].nStartTime) 
						&& (sec <= RecSchPara.nSchTime[day][j].nStopTime)) 
					{
						if(chn_wanna_trigger_me[k-1][i] != 0)
						{
							g_RecordStatus[i] |= (1 << (1 + k));
							chn_is_triggerring_me[k-1][i] |= chn_wanna_trigger_me[k-1][i];
							chn_wanna_trigger_me[k-1][i] = 0;
						}
						break;
					}
				}
				if(j >= MAX_TIME_SEGMENTS)
				{
					g_RecordStatus[i] &= ~(1 << (1 + k));
					if(chn_is_triggerring_me[k-1][i] != 0)//if((chn_wanna_trigger_me[k-1][i] == 0) && (chn_is_triggerring_me[k-1][i] != 0))//csp modify
					{
						chn_wanna_trigger_me[k-1][i] |= chn_is_triggerring_me[k-1][i];
						chn_is_triggerring_me[k-1][i] = 0;
					}
				}
			}
		}
	}		
	
	return EM_REC_SUCCESS;
}

// ret 1: 只要有格式化过的硬盘就返回1
// ret 0: 所有硬盘都未格式化则返回0
void RecordGetDiskFormated(u8* pFormated)
{
	static u8 nFormated = 0;
	
	*pFormated = 1;
	
	if(nFormated)
	{
		return;
	}
	
	u8 i = 0;
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		if((g_HddManager->hinfo[i].is_disk_exist) 
			&& (g_HddManager->hinfo[i].storage_type != 'u'))
		{
			get_disk_info(g_HddManager, i);
			if(g_HddManager->hinfo[i].disk_total)
			{
				break;
			}
		}
	}
	
	if(i >= MAX_HDD_NUM)
	{
		*pFormated = 0;
	}
	else
	{
		nFormated = 1;
	}
	
	return;
}

void RecordStatusManagerFxn()
{
	printf("$$$$$$$$$$$$$$$$$$RecordStatusManagerFxn id:%d\n",getpid());
	
	SModRecMsgHeader sMsg;
	memset(&sMsg, 0, sizeof(sMsg));
	
	u8 flag_pause[g_ChnNum];//zlb20111117  去掉部分malloc
	memset(flag_pause, 0, g_ChnNum);
	
	u8 flag_sendmsg_pause[g_ChnNum];
	memset(flag_sendmsg_pause, 0, g_ChnNum);
	
	u8 CurRecStatus[g_ChnNum];
	memset(CurRecStatus, 0, g_ChnNum);
	
	time_t nLastFrameTime[g_ChnNum];
	memset(nLastFrameTime, 0, sizeof(time_t)*g_ChnNum);
	
	//time_t nThisFrameTime = 0;//NVR using
	
	u16 flag_timesNoFrame[g_ChnNum];
	memset(flag_timesNoFrame, 0, g_ChnNum*2);
	
	//csp modify
	u8 last_IsModWorking = 0;
	u8 TmpRecStatus[g_ChnNum];
	memset(TmpRecStatus, 0, g_ChnNum);
	
	u8 i = 0;
	
	while(1)
	{
		//csp modify
		if(g_recorddeinit == 1)
		{
			break;
		}
		
		if(0 == g_IsModWorking)
		{
			//csp modify
			if(last_IsModWorking)
			{
				last_IsModWorking = 0;
				
				int chn = 0;
				for(chn = 0; chn < g_ChnNum; chn++)
				{
					SModRecMsgHeader sMsg;
					memset(&sMsg, 0, sizeof(sMsg));
					sMsg.nChn = chn;
					sMsg.data = NULL;
					sMsg.nMsg = EM_REC_FLUSH;
					if(EM_REC_SUCCESS != RecordWriteMsgQueue(chn / 8 + 1, &sMsg))
					{
						fprintf(stderr, "RecordWriteMsgQueue error!\n");
						//return;
					}
				}
				
				if(RecordRefreshStatus != NULL)
				{
					RecordRefreshStatus((void *)TmpRecStatus);
				}
			}
			
			usleep(10 * 1000);
			continue;
		}
		
		last_IsModWorking = 1;//csp modify
		
		//usleep(10 * 1000);//csp modify
		
		if(g_MsgQueueInfo[0].nLenUsed > 0)
		{
			if(EM_REC_SUCCESS != RecordReadMsgQueue(0, &sMsg))
			{
				fprintf(stderr, "RecordReadMsgQueue[0] error!!!\n");
				//continue;//csp modify
				usleep(10 * 1000);//csp modify
			}
			else
			{
				//printf("**********RecordReadMsgQueue[0] msg:%d, key:%d\n", sMsg.nMsg, sMsg.nKey);
				switch(sMsg.nMsg)
				{
					case EM_REC_SETSCHPARA:
						{
							SModRecSchPara* tmp = (SModRecSchPara*)(sMsg.data);
							memcpy(&g_RecSchPara[sMsg.nChn + g_ChnNum * tmp->nRecType], tmp, sizeof(SModRecSchPara));
							if(EM_REC_SCH_ONCE_DAY == g_RecSchPara[sMsg.nChn + g_ChnNum * tmp->nRecType].nTimeType)
							{
								g_RecSchOnceDone[sMsg.nChn + g_ChnNum * tmp->nRecType] = 0;
							}
							//free(sMsg.data); //zlb20111117 bugfix
						}
						break;
						
					case EM_REC_SETPRETIME:
						
						memcpy(&g_RecordPreTime[sMsg.nChn], sMsg.data, sizeof(u32));
						//printf("%s EM_REC_SETPRETIME %d\n", __func__, g_RecordPreTime[sMsg.nChn]);
						//free(sMsg.data); //zlb20111117 bugfix
						break;
						
					case EM_REC_SETDELAYTIME:
						memcpy(&g_RecordDelayTime[sMsg.nChn], sMsg.data, sizeof(u32));
						//free(sMsg.data);//zlb20111117 bugfix
						break;
						
					case EM_REC_SETRECCHN_MOTION:
					case EM_REC_SETRECCHN_LOST:
					case EM_REC_SETRECCHN_BIND:
					case EM_REC_SETRECCHN_ALARM:
					case EM_REC_SETRECCHN_NETLINK_LOST:
					case EM_REC_SETRECCHN_ABNORMAL:
						memcpy(&g_RecChnTrigger[sMsg.nChn].nMotion + 8 * (sMsg.nMsg - EM_REC_SETRECCHN_MOTION), sMsg.data, sizeof(u64));
						//free(sMsg.data);//zlb20111117 bugfix
						//break;//why???
						break;//csp modify
						
					case EM_REC_V_MOTION:
					case EM_REC_V_LOST:
					case EM_REC_V_BIND:
					case EM_REC_V_ALARM:
					case EM_REC_NETLINK_LOST:
					case EM_REC_ABNORMAL:
						if(EM_REC_SUCCESS != RecordDealRecMsg(&sMsg))
						{
							fprintf(stderr,"RecordDealRecMsg error !\n");
						}
						break;
						
					case EM_REC_V_MANUAL:
						if(sMsg.nKey == 1)//手动录像开始
						{
							g_RecordStatus[sMsg.nChn] |= 1;
							//printf("modrec start rec!\n");
						}
						else if(sMsg.nKey == 0)//停止
						{
							g_RecordStatus[sMsg.nChn] &= ~1;
						}
						break;
						
					case EM_REC_PAUSEREC:
						printf("@@@ chn%d pause rec @@@\n",sMsg.nChn);
						flag_pause[sMsg.nChn] = 1;
						break;
						
					case EM_REC_RESUMEREC:
						flag_pause[sMsg.nChn] = 0;						
						break;
						
					case EM_REC_STOPCHN:
						flag_pause[sMsg.nChn] = 1;
						break;
						
					default:
						fprintf(stderr,"Invalid Message[%d] received by RecordReadMsgQueue[0]!\n", sMsg.nMsg);
						break;
				}
				
				if(sMsg.data)//zlb20111117 bugfix
				{
					free(sMsg.data);
					sMsg.data = NULL;
				}
				
				usleep(1);//csp modify
			}
		}
		else
		{
			usleep(10 * 1000);//csp modify
		}

		//查看当前时间是否在定时录像时间内，决定是否录像
		sMsg.nMsg = EM_REC_TIMER;
		if(EM_REC_SUCCESS != RecordDealRecMsg(&sMsg))
		{
			fprintf(stderr,"RecordDealRecMsg EM_REC_TIMER error !\n");
		}
		
		RecordGetDiskFormated(&g_RecordHDDFormated);
		
		for(i = 0; i < g_ChnNum; i++)
		{
			if(((flag_pause[i] == 1) || (0 == g_RecordHDDFormated)) && (flag_sendmsg_pause[i] == 0))
			{
				if((0 != g_RecordStatus[i]) || (0 != g_RecordStatusDelay[i]))
				{
					SModRecMsgHeader sMsg;
					memset(&sMsg, 0, sizeof(sMsg));
					sMsg.nChn = i;
					sMsg.data = NULL;
					sMsg.nMsg = EM_REC_FORCE_END;
					if(EM_REC_SUCCESS != RecordWriteMsgQueue(i / 8 + 1, &sMsg))
					{
						fprintf(stderr, "RecordWriteMsgQueue error!\n");
					}
					
					printf("$$$ chn%d send pause rec msg $$$\n",sMsg.nChn);
				}
				else
				{
					printf("$$$ chn%d donot need send pause rec msg $$$\n",sMsg.nChn);
				}
				
				g_ChnPaused[i] = 1;
				flag_sendmsg_pause[i] = 1;
			}
			else if(((flag_pause[i] == 0) && g_RecordHDDFormated) && (flag_sendmsg_pause[i] == 1))
			{
				SModRecMsgHeader sMsg;
				memset(&sMsg, 0, sizeof(sMsg));
				sMsg.nChn = i;
				sMsg.data = NULL;
				sMsg.nMsg = EM_REC_RESUME_REC;
				
				if(EM_REC_SUCCESS != RecordWriteMsgQueue(i / 8 + 1, &sMsg))
				{
					fprintf(stderr, "RecordWriteMsgQueue error!\n");
				}
				
				printf("$$$ chn%d send resume rec msg $$$\n",sMsg.nChn);
				
				g_ChnPaused[i] = 0;
				flag_sendmsg_pause[i] = 0;
			}
			
			RecordUpdateRecStatus(i);
		}
		
		for(i = 0; i < g_ChnNum; i++)
		{
			CurRecStatus[i] = (g_RecordStatus[i] | g_RecordStatusDelay[i]);
			if(g_RecordPreTime[i] > 0)//报警预录时间
			{
				CurRecStatus[i] |= 0x80;
			}
			if((flag_pause[i] == 1) || (0 == g_RecordHDDFormated))
			{
				CurRecStatus[i] = 0;
			}
		}
		
		if(RecordRefreshStatus != NULL)
		{
			//printf("Record status starting callback .......\n");
			RecordRefreshStatus((void *)CurRecStatus);
		}
		
		#if 0//NVR using
		for(i = 0; i < g_ChnNum; i++)
		{
			if(CurRecStatus[i])
			{
				//printf("chn[%d], g_LastTimeGetFrame = %d, nLastFrameTime = %d, flag_timesNoFrame = %d\n",i,g_LastTimeGetFrame[i],nLastFrameTime[i],flag_timesNoFrame[i]);
				nThisFrameTime = g_LastTimeGetFrame[i];
				if(nLastFrameTime[i] == nThisFrameTime)
				{
					flag_timesNoFrame[i]++;
					if(flag_timesNoFrame[i] == 1500)
					{
						if(RecordException != NULL)
						{
							RecordException(1);//编码异常
						}
						ModRecordPause(i);
						usleep(1);//csp modify
						ModRecordResume(i);
						printf("chn[%d] encode abnormal,restart......\n", i);
					}
					else if(flag_timesNoFrame[i] == 2500)
					{
						SModRecMsgHeader sMsg;
						memset(&sMsg, 0, sizeof(sMsg));
						sMsg.data = NULL;
						sMsg.nMsg = EM_REC_FORCE_END;
						u8 chn = 0;
						for(chn = 0; chn < g_ChnNum; chn++)
						{
							sMsg.nChn = chn;
							if(EM_REC_SUCCESS != RecordWriteMsgQueue(chn / 8 + 1, &sMsg))
							{
								fprintf(stderr, "RecordWriteMsgQueue error!\n");
							}
						}
						usleep(3000*1000);
						if(RecordException != NULL)
						{
							RecordException(3);//编码异常,重启主机
						}
						printf("chn[%d] encode abnormal,exit......!\n", i);
					}
				}
				else
				{
					nLastFrameTime[i] = nThisFrameTime;
					flag_timesNoFrame[i] = 0;
				}
			}
			else
			{
				//if((flag_pause[i] == 0) && g_RecordHDDFormated)
				{
					flag_timesNoFrame[i] = 0;
				}
			}
		}
		#endif
		
		//csp modify
		//if(g_recorddeinit == 1)
		//{
		//	break;
		//}
		
		//usleep(10 * 1000);//csp modify
	}
}

#define RECORD_UPDATE_FILE_INFO_NOT_FREQUENT //更新录像文件类型不实时，实时更新容易产生零碎文件

s32 RecordUpdateRecStatus(u8 chn)
{
	if(chn >= g_ChnNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	static time_t *start_delay = NULL;
	static u8 *pre_start = NULL;
	static u8 flag_init = 0;
	if(flag_init == 0)
	{
		start_delay = (time_t *)malloc(g_ChnNum * sizeof(time_t));
		if(start_delay == NULL)
		{
			fprintf(stderr, "function: %s\n", __FUNCTION__);
			return EM_REC_MALLOC_ERR;
		}
		memset(start_delay, 0, g_ChnNum * sizeof(time_t));
		
		pre_start = malloc(g_ChnNum);
		if(pre_start == NULL)
		{
			fprintf(stderr, "function: %s\n", __FUNCTION__);
			return EM_REC_MALLOC_ERR;
		}
		memset(pre_start, 0, g_ChnNum);
		
		flag_init = 1;
	}
	
	u8 flag_rectype_changed = 0;
	
	SModRecMsgHeader sMsg;
	memset(&sMsg, 0, sizeof(sMsg));
	sMsg.nChn = chn;
	sMsg.data = NULL;
	
	if(g_RecordStatusPast[chn] != g_RecordStatus[chn])
	{
		if(0 == g_RecordStatusPast[chn])
		{
			if(0 == g_RecordStatusDelay[chn])
			{
				//csp modify
				//if((g_RecordPreTime[chn] > 0) && (g_RecordStatus[chn] > 0x03))
				if((g_IsPreRecording[chn] > 0) && (g_RecordStatus[chn] > 0x03))
				{
					sMsg.nMsg = EM_REC_PRE_RUN;
					g_IsPreRecording[chn] = 0;
					pre_start[chn] = 0;
				}
				else
				{
					sMsg.nMsg = EM_REC_IDLE_RUN;
					g_IsPreRecording[chn] = 0;
					pre_start[chn] = 0;
				}
			}
			else
			{
				//g_RecordStatusDelay[chn] = 0;//csp modify
				
				//sMsg.nMsg = EM_REC_DELAY_RUN;
				
				//csp modify
				//if(0 == ((~g_RecordStatusDelay[chn]) & g_RecordStatus[chn]))
				if(g_RecordStatusDelay[chn] == g_RecordStatus[chn])
				{
					sMsg.nMsg = EM_REC_DELAY_RUN;
				}
				else
				{
					sMsg.nMsg = EM_REC_DELAY_RUN;
					flag_rectype_changed = 1;
				}
				
				g_RecordStatusDelay[chn] = 0;//csp modify
			}
			
			if(EM_REC_SUCCESS != RecordWriteMsgQueue(chn / 8 + 1, &sMsg))
			{
				fprintf(stderr, "RecordWriteMsgQueue error!\n");
				//return;
			}
			
			#ifndef RECORD_UPDATE_FILE_INFO_NOT_FREQUENT
			if(flag_rectype_changed)
			{
				sMsg.nMsg = EM_REC_TYPE_CHANGE_END;
				if(EM_REC_SUCCESS != RecordWriteMsgQueue(chn / 8 + 1, &sMsg))
				{
					fprintf(stderr, "RecordWriteMsgQueue error!\n");
					//return;
				}
			}
			#endif
			
			sMsg.nMsg = EM_REC_UPDATEFILEINFO;
			sMsg.data = (void *)(unsigned int)g_RecordStatus[chn];//sMsg.data = (void *)g_RecordStatus;//csp modify
			if(EM_REC_SUCCESS != RecordWriteMsgQueue(chn / 8 + 1, &sMsg))
			{
				fprintf(stderr, "RecordWriteMsgQueue error!\n");
				//return;
			}
		}
		else if(0 == g_RecordStatus[chn])
		{
			if((g_RecordStatusPast[chn] <= 0x03))
			{
				sMsg.nMsg = EM_REC_RUN_IDLE;
			}
			else
			{
				if(g_RecordDelayTime[chn] > 0)
				{
					g_RecordStatusDelay[chn] = g_RecordStatusPast[chn];
					start_delay[chn] = time(NULL);
					
					sMsg.nMsg = EM_REC_RUN_DELAY;
				}
				else
				{
					sMsg.nMsg = EM_REC_RUN_IDLE;
				}
			}
			if(EM_REC_SUCCESS != RecordWriteMsgQueue(chn / 8 + 1, &sMsg))
			{
				fprintf(stderr, "RecordWriteMsgQueue error!\n");
				//return;
			}
		}
		else
		{
			#ifndef RECORD_UPDATE_FILE_INFO_NOT_FREQUENT
			sMsg.nMsg = EM_REC_TYPE_CHANGE_END;
			if(EM_REC_SUCCESS != RecordWriteMsgQueue(chn / 8 + 1, &sMsg))
			{
				fprintf(stderr, "RecordWriteMsgQueue error!\n");
				//return;
			}
			#endif
			
			sMsg.nMsg = EM_REC_UPDATEFILEINFO;
			sMsg.data = (void *)(unsigned int)g_RecordStatus[chn];//sMsg.data = (void *)g_RecordStatus;//csp modify
			if(EM_REC_SUCCESS != RecordWriteMsgQueue(chn / 8 + 1, &sMsg))
			{
				fprintf(stderr, "RecordWriteMsgQueue error!\n");
				//return;
			}
		}
	}
	
	sMsg.data = NULL;//csp modify 20121018
	
	if((0 == g_RecordStatusPast[chn]) && (0 == g_RecordStatus[chn]) && (0 == g_RecordStatusDelay[chn]))
	{
		if((pre_start[chn] == 0) && (g_RecordPreTime[chn] > 0))
		{
			if((0 == g_IsWritingFile[chn]) && (0 == g_RecBufInfo[chn].nLenUsed))
			{
				pre_start[chn] = 1;
				g_IsPreRecording[chn] = 1;
				
				/*sMsg.nMsg = EM_REC_IDLE_PRE;
				
				if(EM_REC_SUCCESS != RecordWriteMsgQueue(chn / 8 + 1, &sMsg))
				{
					fprintf(stderr, "RecordWriteMsgQueue error!\n");
					//return;
				}*/
			}
		}
		else if((pre_start[chn] != 0) && (g_RecordPreTime[chn] == 0))
		{
			pre_start[chn] = 0;
			g_IsPreRecording[chn] = 0;
			
			sMsg.nMsg = EM_REC_PRE_IDLE;
			
			if(EM_REC_SUCCESS != RecordWriteMsgQueue(chn / 8 + 1, &sMsg))
			{
				fprintf(stderr, "RecordWriteMsgQueue error!\n");
				//return;
			}
		}		
	}
	
	sMsg.data = NULL;//csp modify 20121018
	
	if((0 == g_RecordStatusPast[chn]) && (0 == g_RecordStatus[chn]) && (0 != g_RecordStatusDelay[chn]))
	{
		if(((start_delay[chn] + g_RecordDelayTime[chn]) < time(NULL)) || (time(NULL) < start_delay[chn]))
		{
			sMsg.nMsg = EM_REC_DELAY_IDLE;
			
			if(EM_REC_SUCCESS != RecordWriteMsgQueue(chn / 8 + 1, &sMsg))
			{
				fprintf(stderr, "RecordWriteMsgQueue error!\n");
				//return;
			}
			else
			{
				g_RecordStatusDelay[chn] = 0;
			}
		}
	}
	
	g_RecordStatusPast[chn] = g_RecordStatus[chn];
	
	return EM_REC_SUCCESS;
}

//yaogang modify for bad disk
void report_disk_err(const char *diskname)
{
	HddInfo *phinfo = NULL;
	int i;
	
	for(i=0; i<MAX_HDD_NUM; i++)
	{
		phinfo = &g_HddManager->hinfo[i];
		if(phinfo->is_disk_exist)
		{
			if (strncmp(phinfo->disk_name, diskname, strlen(diskname)) == 0)
			{
				phinfo->is_bad_disk = 1;

				//report to modsys_UpdateDiskInfoFxn() --> update Gui
				pthread_mutex_lock(&g_mutex);//yg modify
				g_rec_disk_err_bitmask |= 1<<(phinfo->disk_logic_idx-1);
				pthread_mutex_unlock(&g_mutex);//yg modify

				printf("bad disk logic index: %d\n", phinfo->disk_logic_idx);
			}
		}
	}
}

s32 RecordOpenNewFile(custommp4_t** file, partition_index** p_ptn_index, char* recfilename, u32* offset, u8 chn)
{
	u32 open_offset = 0;
	
	if(file == NULL || p_ptn_index == NULL)
	{
		fprintf(stderr, "error: param!\n");
		return EM_REC_INVALID_PARA;
	}
	
	if(NULL != *file)
	{
		fprintf(stderr, "error: can not open a new file!\n");
		return EM_REC_CANNOTOPENNEWFILE;
	}
	else
	{
		//printf("func:%s,line:%d\n",__FUNCTION__,__LINE__);
		u8 disk_system_idx = 0;
		*p_ptn_index = get_rec_path(g_HddManager, recfilename, &open_offset, chn, &disk_system_idx);
		if(*p_ptn_index == NULL)
		{
			fprintf(stderr, "error: get_rec_path!\n");
			return EM_REC_ERRGETRECPATH;
		}
		else
		{  
			*file = custommp4_open(recfilename,O_W_APPEND,open_offset);
			if(*file == NULL)
			{
				fprintf(stderr, "custommp4_open failed\n");
				return EM_REC_ERROPENNEWFILE;
			}
			else
			{
				pthread_mutex_lock(&g_mutex);
				g_diskuse[chn] = disk_system_idx;//sda--1 sdb--2
				pthread_mutex_unlock(&g_mutex);
				//printf("%s chn%d, disk_system_idx: %d\n", __func__, chn, disk_system_idx);
				*offset = open_offset;
				return EM_REC_SUCCESS;
			}
		}
	}
}

s32 RecordGetFrameSize(EMRECENCRESOL emResolution, u16* width, u16* height)
{
	if((NULL == width) || (NULL == height))
	{
		fprintf(stderr, "error: param!\n");
		return EM_REC_INVALID_PARA;
	}
	
	if(emResolution == EM_REC_ENC_D1)
	{
		*width = 704;
		*height = 576;
	}
	else if(emResolution == EM_REC_ENC_HALF_D1)
	{
		*width = 704;
		*height = 576 >> 1;
	}
	else if(emResolution == EM_REC_ENC_CIF)
	{
		*width = 704 >> 1;
		*height = 576 >> 1;
	}
	else if(emResolution == EM_REC_ENC_QCIF)
	{
		*width = 704 >> 2;
		*height = 576 >> 2;
	}
	else if(emResolution == EM_REC_ENC_N_D1)
	{
		*width = 704;
		*height = 480;
	}
	else if(emResolution == EM_REC_ENC_N_HALF_D1)
	{
		*width = 704;
		*height = 480 >> 1;
	}
	else if(emResolution == EM_REC_ENC_N_CIF)
	{
		*width = 704 >> 1;
		*height = 480 >> 1;
	}
	else if(emResolution == EM_REC_ENC_N_QCIF)
	{
		*width = 704 >> 2;
		*height = 480 >> 2;
	}
	else if(emResolution == EM_REC_ENC_720P)
	{
		*width = 1280;
		*height = 720;
	}
	else if(emResolution == EM_REC_ENC_1080P)
	{
		*width = 1920;
		*height = 1080;
	}
	else if(emResolution == EM_REC_ENC_960H)
	{
		*width = 960;
		*height = 576;
	}
	else if(emResolution == EM_REC_ENC_N_960H)//csp modify
	{
		*width = 960;
		*height = 480;//576;
	}
	else
	{
		fprintf(stderr, "error: Resolution param!\n");
		return EM_REC_INVALID_PARA;
	}
	
	return EM_REC_SUCCESS;
}

//所有硬盘都没有剩余空间 返回1
void RecordGetDiskFull(u8* pFull)
{
	*pFull = 0;
	if(0 == g_HddIsFullCover)
	{
		u8 i = 0;
		for(i = 0; i < MAX_HDD_NUM; i++)
		{
			if((g_HddManager->hinfo[i].is_disk_exist) 
				&& (g_HddManager->hinfo[i].storage_type != 'u'))
			{
				get_disk_info(g_HddManager, i);
				//printf("hdd[%d], total[%d]/free[%d]\n", i, g_HddManager->hinfo[i].total, g_HddManager->hinfo[i].free);
				if(g_HddManager->hinfo[i].disk_free)
				{
					break;
				}
			}
		}
		
		if(i >= MAX_HDD_NUM)
		{
			*pFull = 1;
		}
	}
	//printf("hdd, free[%d], free[%d]\n", g_HddManager->hinfo[0].free, g_HddManager->hinfo[1].free);
}

int check_disk_ptn(char *prefix)
{
/*
	unsigned char fat[48] = 
	{
		0xf8, 0xff, 0xff, 0x0f, 
		0xff, 0xff, 0xff, 0x0f, 
		0x03, 0x00, 0x00, 0x00, 
		0x04, 0x00, 0x00, 0x00, 
		0x05, 0x00, 0x00, 0x00, 
		0x06, 0x00, 0x00, 0x00, 
		0x07, 0x00, 0x00, 0x00, 
		0xff, 0xff, 0xff, 0x0f, 
		0xff, 0xff, 0xff, 0x0f, 
		0x0a, 0x00, 0x00, 0x00, 
		0x0b, 0x00, 0x00, 0x00, 
		0x0c, 0x00, 0x00, 0x00
	};
	int fd = open("/dev/sda4", O_RDONLY);
	if(fd == -1)
	{
		printf("check_disk_ptn: open %s failed\n", "/dev/sda4");
		return -1;
	}
	lseek(fd, 32*512, SEEK_SET);
	unsigned char real[48] = {0};
	int ret = read(fd, real, sizeof(real));
	close(fd);
	if(ret != sizeof(real))
	{
		printf("check_disk_ptn: read %s failed\n", "/dev/sda4");
		return -1;
	}
	if(memcmp(real, fat, sizeof(fat)) != 0)
	{
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@check_disk_ptn(%s): check %s failed\n", prefix, "/dev/sda4");
		fflush(stdout);
		sleep(15);
		exit(-1);
		return -1;
	}
*/
	return 0;
}
//yaogang modify 20141118
//通过文件名来得到设备名(挂载点-- 设备名)
//file name: rec/a1/fly00000.ifv
//size: diskname bufsize
//diskname : /dev/sdx
int get_disk_from_filename(const char * filename, char *diskname, int size)
{
	FILE * file = NULL;
	char pre_filename[10];//前缀,只需要rec/a1
	char buf[1024];
	
	memset(diskname, 0, size);

	if (size < 9)
	{
		printf("get_disk_from_filename: size too small\n");
		return -1;
	}

	if (strstr(filename, "rec/") == NULL)
	{
		printf("get_disk_from_filename: filename is invalid\n");
		return -1;
	}
	strncpy(pre_filename, filename, 6);
	pre_filename[6] = '\0';
	//printf("get_disk_from_filename: pre_filename : %s\n", pre_filename);
	
	//read /proc/mounts  & find it
	file = fopen("/proc/mounts", "r");
	if (NULL == file)
	{
		printf("get_disk_from_filename: fopen failed\n");
		return -1;
	}
	
	while(1)
	{
		if (fgets(buf, 1024, file) == NULL)
		{
			printf("get_disk_from_filename: fgets: failed or file end\n");
			break;
		}
		if (strstr(buf, pre_filename))
		{
			strncpy(diskname, buf, 8);
			diskname[8] = '\0';
			break;
		}
	}

	fclose(file);
	
	if (diskname[0] == 0)
	{
		printf("get_disk_from_filename: mount point: %s not found devname\n", filename);
		return -1;
	}
	
	//printf("get_disk_from_filename: mount point: %s found diskname: %s\n", filename, diskname);

	return 0;
}

//yaogang modify for bad disk
#if 0
static void set_disk_err_bit(int bitnum)
{
	if (check_disk_err.init_status)
	{
		pthread_mutex_lock(&check_disk_err.lock);
		
		check_disk_err.disk_status |= 1<<bitnum;
		
		pthread_mutex_unlock(&check_disk_err.lock);
	}
}


int get_disk_err_record()
{
	int ret = 0;

	if (check_disk_err.init_status)
	{
		pthread_mutex_lock(&check_disk_err.lock);
		
		ret = check_disk_err.disk_status;
		check_disk_err.disk_status = 0;
		
		pthread_mutex_unlock(&check_disk_err.lock);
	}
	
	return ret;
}
//#else

u32 get_disk_err_record()
{
	u32 bitmask = 0;
	
	pthread_mutex_lock(&g_mutex);//yg modify
	bitmask = g_rec_disk_err_bitmask;
	pthread_mutex_unlock(&g_mutex);//yg modify

	return bitmask;
}


#endif

extern SDevInfo* modsys_GetDiskInfo( u8 nIdx );

//yaogang modify 20170218 in shanghai
#define MallocFrameBuf
#define MaxFrameSize (512<<10)

void RecordMainFxn(void* ID)
{
	if(NULL == ID)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return;
	}
	
	printf("$$$$$$$$$$$$$$$$$$RecordMainFxn id:%d\n",getpid());
	
	u8 nMsgQID = *((u8*)ID) + 1;
	printf("pthread %s start OK! MsgQID[%d]\n", __FUNCTION__, nMsgQID);
	
	SModRecMsgHeader sMsg;
	memset(&sMsg, 0, sizeof(sMsg));
	
	SModRecRecordHeader header;
	memset(&header, 0, sizeof(header));

	int FrameSize = 0;
	
//yaogang modify 20170218 in shanghai	
#ifdef MallocFrameBuf	
	u8 *frame_data = NULL;
	FrameSize = MaxFrameSize;

	frame_data = (u8 *)malloc(FrameSize);
	if (NULL == frame_data)
	{
		printf("error %s frame_data malloc failed\n", __func__);
		return;
	}
	printf("%s malloc frame buf size: %d\n", __func__, FrameSize);
	
#else
	//csp modify 20121016//这里有bug
	#if defined(CHIP_HISI3531)// || defined(CHIP_HISI3521)
	u8 frame_data[512 << 10];
	#else
	//u8 frame_data[256 << 10];
	u8 frame_data[512 << 10];//NVR used
	#endif

	FrameSize = 512<<10;

	printf("%s stack frame buf size: %d\n", __func__, sizeof(frame_data));
	//memset(frame_data, 0, sizeof(frame_data));
#endif
	
	//s32 i = 0, num = 0;
	s32 i = 0;
	u8 type = 0;
	
	u8 flag_force_writefile[g_ChnNum];//zlb20111117 去掉malloc
	memset(flag_force_writefile, 0, g_ChnNum);
	
	u8 flag_file_opened[g_ChnNum];//zlb20111117 去掉malloc
	memset(flag_file_opened, 0, g_ChnNum);
	
	//u8* flag_ReceiveRightFrame = (u8*)calloc(1, g_ChnNum);
	
	u8 flag_set_file_start_time[g_ChnNum];//zlb20111117 去掉malloc
	memset(flag_set_file_start_time, 0, g_ChnNum);
	
	//u8* flag_set_file_end_time = (u8*)calloc(1, g_ChnNum);
	
	#ifdef RECORD_UPDATE_FILE_INFO_NOT_FREQUENT
	u8 rec_file_type[g_ChnNum];//zlb20111117 去掉malloc
	memset(rec_file_type, 0, g_ChnNum);
	u8 rec_file_type_total[g_ChnNum];//zlb20111117 去掉malloc
	memset(rec_file_type_total, 0, g_ChnNum);
	#endif
	
	custommp4_t *file[g_ChnNum];//zlb20111117 去掉malloc
	partition_index *p_ptn_index[g_ChnNum];//zlb20111117 去掉malloc
	u32 open_offset = 0;
	char recfilename[40];
	u64 max_pts[g_ChnNum];//zlb20111117 去掉malloc
	u64 startTimeStamp[g_ChnNum];//zlb20111117 去掉malloc
	s32 real_write_bytes = 0;
	u8 update = 0;
	s32 ret = 0;

	//yaogang modify fileflush time test
	//u32 time_pre, time_cur, time_interval;
	//time_pre = time_cur = time_interval = 0;
	
	
	for(i = 0; i < g_ChnNum; i++)
	{
		file[i] = NULL;
		p_ptn_index[i] = NULL;
		max_pts[i] = 0;
		startTimeStamp[i] = 0;
	}
	
	segment_use_info s_u_info[g_ChnNum];//zlb20111117 去掉malloc
	memset(s_u_info, 0, g_ChnNum * sizeof(segment_use_info));
	
	#if 0//NVR used
	//csp modify 20121016//这里有bug
	//char rec_w_buf[512 << 10];
	//u8 rec_w_buf[512 << 10];//zlb20111117 去掉malloc
	u8 rec_w_buf[1 << 20];//NVR used
	u32 rec_w_buf_len = 0;
	#endif
	
	u8 bufrate = 0;
	
	u32 limit_max = SIZE_OF_FILE_VECTOR - SIZE_OF_RESERVED;
	
	u32 nReservedLength = 0;
	u8 nDiskFull = 0;
	u8 nStopRecordByDiskFull = 0;
	u8 nExceptionHappened = 0;
	//yaogang modify 20141118
	u8 ndisk_open_write_err = 0;
	u8 b_not_found_valid_disk = 0;
	//u8 b_stop_rec = 0;
	char err_filename[32];
	
	#if 0
	u16 nExceptionTimes = 0;
	time_t nLastExceptionTime = 0;
	#endif
	
	//u8 openfile = 0;
	//g_RecordTemp = 0;
	
	while(1)
	{
		//csp modify
		if(g_recorddeinit == 1)
		{
			break;
		}
		
		//printf("func:%s, line:%d\n",__FUNCTION__,__LINE__);
		//printf("g_recorddeinit = %d  nStopRecordByDiskFull = %d\n",g_recorddeinit,nStopRecordByDiskFull);
		
		if(!nStopRecordByDiskFull)
		{
			//int value = -1;
			//sem_getvalue(&g_semMsgQueue[nMsgQID-1], &value);
			//fprintf(stderr, "RecordMainFxn: sem_wait [%d,%d], nLenUsed = %d...................................\n", nMsgQID-1, value, g_MsgQueueInfo[nMsgQID].nLenUsed);
			
			sem_wait(&g_semMsgQueue[nMsgQID - 1]);
			
			//value = -1;
			//sem_getvalue(&g_semMsgQueue[nMsgQID-1], &value);
			//fprintf(stderr, "RecordMainFxn: sem_wait [%d,%d] OK, nLenUsed = %d!\n", nMsgQID-1, value, g_MsgQueueInfo[nMsgQID].nLenUsed);
		}
		else//csp modify
		{
			usleep(10 * 1000);
			//printf("###func:%s, line:%d###, nLenUsed:%d\n",__FUNCTION__,__LINE__,g_MsgQueueInfo[nMsgQID].nLenUsed);
		}
		//printf("func:%s, line:%d\n",__FUNCTION__,__LINE__);
		
		if(g_MsgQueueInfo[nMsgQID].nLenUsed > 0)
		{
			if(EM_REC_SUCCESS != RecordReadMsgQueue(nMsgQID, &sMsg))
			{
				fprintf(stderr, "RecordReadMsgQueue[%d] error!!!\n", nMsgQID);
				continue;
			}
			else if(flag_force_writefile[sMsg.nChn])
			{
				if(EM_REC_RESUME_REC == sMsg.nMsg)
				{
					flag_force_writefile[sMsg.nChn] = 0;
				}
				else if(EM_REC_UPDATEFILEINFO == sMsg.nMsg)
				{
					type = (u8)(unsigned int)sMsg.data;//memcpy(&type, (u8 *)sMsg.data + sMsg.nChn, sizeof(type));//csp modify
					type = ((type >> 1) | ((type & 1) << 3));
					#ifdef RECORD_UPDATE_FILE_INFO_NOT_FREQUENT
					rec_file_type[sMsg.nChn] = type;
					rec_file_type_total[sMsg.nChn] |= type;//csp modify
					#else
					s_u_info[sMsg.nChn].type = type;
					#endif
				}
				else
				{
					//
				}
			}
			else
			{
				bufrate = 0;
				
				switch(sMsg.nMsg)
				{
					case EM_REC_RESUME_REC://csp modify//do nothing
						break;
						
					case EM_REC_IDLE_RUN:
					case EM_REC_PRE_RUN:
						//RecordOpenNewFile(file[sMsg.nChn], p_ptn_index[sMsg.nChn], recfilename, sMsg.nChn);
						//end
						break;
						
					case EM_REC_DELAY_RUN:
					case EM_REC_RUN_DELAY:
						//no need to deal
						break;
						
					case EM_REC_UPDATEFILEINFO:
						type = (u8)(unsigned int)sMsg.data;//memcpy(&type, (u8 *)sMsg.data + sMsg.nChn, sizeof(type));//csp modify
						type = ((type >> 1) | ((type & 1) << 3));
						#ifdef RECORD_UPDATE_FILE_INFO_NOT_FREQUENT
						rec_file_type[sMsg.nChn] = type;
						rec_file_type_total[sMsg.nChn] |= type;
						#else
						s_u_info[sMsg.nChn].type = type;
						#endif
						break;
						
					case EM_REC_PRE_IDLE:
						//usleep(20 * 1000);
						if(EM_REC_SUCCESS != RecordResetBuf(sMsg.nChn))
						{
							fprintf(stderr, "RecordResetBuf[%d] error!!!\n", sMsg.nChn);
						}
						break;
						
					case EM_REC_READ_BUF:
						//printf("chn[%d], need read buf\n",sMsg.nChn);
						//NVR used
						//bufrate = 1;
						bufrate = 5;//yaogang modify 20150313
						
					case EM_REC_FORCE_END:
						if(bufrate == 0)
						{
							printf("chn%d stop record - EM_REC_FORCE_END\n",sMsg.nChn);
							flag_force_writefile[sMsg.nChn] = 1;
						}
						
					case EM_REC_FLUSH://csp modify
					case EM_REC_TYPE_CHANGE_END:
					case EM_REC_RUN_IDLE:
					case EM_REC_DELAY_IDLE:
						if(bufrate == 0)
						{
							g_IsWritingFile[sMsg.nChn] = 1;
						}
						//memset(rec_w_buf, 0, sizeof(rec_w_buf));
						//memset(rec_w_buf, 0, 512<<10);//csp modify
						//rec_w_buf_len = 0;//NVR used
						nExceptionHappened = 0;
						//yaogang modify 20141118
						ndisk_open_write_err = 0;
						memset(err_filename, 0, sizeof(err_filename));
						//struct timeval tv;
						//gettimeofday(&tv, NULL);
						//printf("chn[%d]:nMsg:%d, bufrate:%d, time:%llu\n", sMsg.nChn, sMsg.nMsg, RecordGetBufUseRate(sMsg.nChn), (u64)tv.tv_sec*1000000+tv.tv_usec);

						//yaogang modify 20141118 for test disk WR err
						//static int wr_err = 0;
						//static int open_err = 0;
						
						
						if(sMsg.nMsg == EM_REC_FORCE_END)
						{
							printf("chn%d record force end - step 1$$$\n",sMsg.nChn);
						}
						
						while(1)
						{
							#if 0
							//printf("**********************************************************************\n");
							if(g_RecordTemp == 0)//xdc
							{
								num = 0;
								for(i=0;i<8;i++)
								{
									if(g_HddManager->hinfo[i].is_disk_exist == 1)
									{
										//printf("i = %d\n",i);
										num++;
										if(num == 1)
										{
											g_RecordTemp = 1;
											g_Temp = i;
										}
									}
								}
							}
							//printf("g_Temp = %d  g_RecordTemp=%d\n",g_Temp,g_RecordTemp);
							num = 0;
							if(g_RecordTemp == 1)//xdc
							{
								for(i=0;i<8;i++)
								{
									if(g_HddManager->hinfo[i].is_disk_exist == 1)
									{
										//printf("g_Temp = %d i = %d\n",g_Temp,i);
										num++;
										if(num == 1)
										{
											if(g_Temp != i)
											{
												g_RecordTemp = 0;
												openfile = 9;
											}
										}
									}
								}
								if(num == 0)
								{
									g_RecordTemp = 0;
									openfile = 9;
								}
							}
							if(openfile == 9)
							{
								RecordResetBuf(sMsg.nChn);
								nExceptionHappened = 1;
								break;
							}
							#endif
							
							if(sMsg.nMsg == EM_REC_FORCE_END)
							{
								//printf("chn%d record force end - step 2$$$\n",sMsg.nChn);
							}
							
							#if 1
							//unsigned char cur_atx_flag = tl_power_atx_check();
							//if(cur_atx_flag == 0 || g_HotPlugFlag)//电池供电
							if(g_HotPlugFlag)
							{
								RecordResetBuf(sMsg.nChn);
								break;
							}
							#endif
							
							if(sMsg.nMsg == EM_REC_FORCE_END)
							{
								//printf("chn%d record force end - step 3$$$\n",sMsg.nChn);
							}
							
							if(RecordGetBufUseRate(sMsg.nChn) <= bufrate)
							{
								if(bufrate > 0)
								{
									break;
								}
								else
								{
									if(g_RecBufInfo[sMsg.nChn].nLenUsed == 0)
									{
										break;
									}
								}
							}
							
							if(sMsg.nMsg == EM_REC_FORCE_END)
							{
								//printf("chn%d record force end - step 4$$$\n",sMsg.nChn);
							}

							//yaogang modify 20170218 in shanghai  add MaxFrameSize
							ret = RecordReadFrameFromBuf(sMsg.nChn, frame_data, FrameSize);
							if(EM_REC_SUCCESS != ret)
							{
								printf("RecordReadFrameFromBuf[%d] error code = %d\n", sMsg.nChn, ret);
								
								if (EM_REC_BUF_NO_DATA == ret) //大于1M 认为缓存区出错
								{
									RecordResetBuf(sMsg.nChn);//重置缓冲区
								}
								
								break;
							}
							memcpy(&header, frame_data, sizeof(header));
							
							if(sMsg.nMsg == EM_REC_FORCE_END)
							{
								//printf("chn%d record force end - step 5$$$\n",sMsg.nChn);
							}
							
							//yzw add
							//if((0 == flag_ReceiveRightFrame[sMsg.nChn]) && (0 == header.nFrameType) && (0 == header.nMediaType))
							//{
							//	continue;
							//}
							//end
							//if(0 == sMsg.nChn)
							//{
							//	printf("length:%d, stamp:%llu\n", header.nDataLength,header.nTimeStamp);
							//}
							//printf("frame length: %d\n", header.nDataLength);
							
							if(file[sMsg.nChn] != NULL)
							{
								//printf("yg file name: %s\n", file[sMsg.nChn]->file_name);
								if((0 == header.nMediaType) && (header.nFrameType))
								{
									nReservedLength = 0;// 1 << 20; //1MBytes
								}
								else
								{
									////csp modify 20141211//可能少计算了3000个视频帧头和4000个音频帧头的长度
									//nReservedLength = SIZE_OF_RESERVED;//0;
									nReservedLength = SIZE_OF_RESERVED / 2;//SIZE_OF_RESERVED;//0;
								}
								//NVR used
								//文件大小超过限制，关闭该文件
								//if(custommp4_end_position(file[sMsg.nChn]) /*- s_u_info[sMsg.nChn].start_position*/ + header.nDataLength + rec_w_buf_len > limit_max + nReservedLength)
								if(custommp4_end_position(file[sMsg.nChn])  + header.nDataLength + g_RecordCacheUsed[sMsg.nChn] > limit_max + nReservedLength)
								{
									//NVR used
									//if(rec_w_buf_len > 0)
									if(g_RecordCacheUsed[sMsg.nChn] > 0)
									{
										//printf("file:%s, line:%d\n", __FILE__, __LINE__);
										//if(0 > custommp4_write_data(file[sMsg.nChn], rec_w_buf, rec_w_buf_len))
										if(0 > custommp4_write_data(file[sMsg.nChn], g_RecordCacheAddr[sMsg.nChn], g_RecordCacheUsed[sMsg.nChn]))
										{
											nExceptionHappened = 1;
											//yaogang modify 20141118
											if (!ndisk_open_write_err)
											{
												ndisk_open_write_err = 1;
												//printf("RecordMainFxn: custommp4_write_data 1 failed: filename: %s\n", file[sMsg.nChn]->file_name);
												strcpy(err_filename, file[sMsg.nChn]->file_name);
												printf("Error: chn[%d], custommp4_write_data 1 failed, filename: %s\n", sMsg.nChn, err_filename);
											}
										}
										//printf("file:%s, line:%d\n", __FILE__, __LINE__);
										//NVR used
										//rec_w_buf_len = 0;
										g_RecordCacheUsed[sMsg.nChn] = 0;
										//memset(rec_w_buf, 0, sizeof(rec_w_buf));
										//memset(rec_w_buf, 0, 512<<10);//csp modify
									}
									s_u_info[sMsg.nChn].end_time = max_pts[sMsg.nChn] / 1000000;
									s_u_info[sMsg.nChn].end_position = custommp4_end_position(file[sMsg.nChn]);	
									#ifdef RECORD_UPDATE_FILE_INFO_NOT_FREQUENT
									s_u_info[sMsg.nChn].type = rec_file_type_total[sMsg.nChn];
									rec_file_type_total[sMsg.nChn] = rec_file_type[sMsg.nChn];
									#endif
									
									u8 error_flag = file[sMsg.nChn]->error_flag;
									
									custommp4_set_file_end_time(file[sMsg.nChn], (u32)(header.nTimeStamp/1000));
									int rtn = custommp4_close(file[sMsg.nChn]);
									
									file[sMsg.nChn] = NULL;
									
									check_disk_ptn("-close");
									
									pthread_mutex_lock(&g_mutex);//yg modify
									g_diskuse[sMsg.nChn] = 0;//rec/a1/fly00000.ifv
									pthread_mutex_unlock(&g_mutex);
									
									if(error_flag == 0 && rtn == 1)
									{
										if(0 == update_chn_cur_segment(p_ptn_index[sMsg.nChn], sMsg.nChn, &s_u_info[sMsg.nChn], 1))
										{
											printf("update_chn_cur_segment chn[%d] failed!*******\n", sMsg.nChn);	
										}
										
										check_disk_ptn("-update2");
									}
									
									max_pts[sMsg.nChn] = 0;//csp modify
									flag_file_opened[sMsg.nChn] = 0;
									flag_set_file_start_time[sMsg.nChn] = 0;
									//flag_ReceiveRightFrame[sMsg.nChn] = 0;//yzw add
								}

								//yaogang modify for bad disk
								if(ndisk_open_write_err)
								{
									break;
								}
							}
							
							if(sMsg.nMsg == EM_REC_FORCE_END)
							{
								//printf("chn%d record force end - step 6$$$\n",sMsg.nChn);
							}
							
							if(0 == header.nMediaType)
							{
								if(header.nPts > max_pts[sMsg.nChn])
								{
									max_pts[sMsg.nChn] = header.nPts;
								}
							}
							
							if(0 == flag_file_opened[sMsg.nChn])
							{
								if((0 == header.nFrameType) || (0 != header.nMediaType))
								//if((0 == header.nFrameType) && (0 == header.nMediaType))
								{
									continue;
								}
								
								//unsigned char cur_atx_flag = tl_power_atx_check();
								//if(cur_atx_flag == 0)
								//{
								//	continue;
								//}
								
								//if(0 == header.nMediaType)
								//{
								//	flag_ReceiveRightFrame[sMsg.nChn] = 1;
								//}
								
								memset(recfilename, 0, sizeof(recfilename));
								//yaogang file open error
								ret = RecordOpenNewFile(&file[sMsg.nChn], &p_ptn_index[sMsg.nChn], recfilename, &open_offset, sMsg.nChn);
								/*if (!open_err)
								{
									open_err = 1;
									ret = EM_REC_ERROPENNEWFILE;
									file[sMsg.nChn] = NULL;
								}*/
								if(EM_REC_SUCCESS != ret)
								{
									fprintf(stderr, "chn[%d], RecordOpenNewFile error!!!\n", sMsg.nChn);
									RecordResetBuf(sMsg.nChn);
									//ModRecordPause(sMsg.nChn);
									nExceptionHappened = 1;
									
									//yaogang modify 20141118
									if (EM_REC_ERRGETRECPATH == ret)//找不到文件容器，没有有效的磁盘
									{
										b_not_found_valid_disk = 1;
									}
									else if (EM_REC_ERROPENNEWFILE == ret)
									{
										if (!ndisk_open_write_err)
										{
											ndisk_open_write_err = 1;
											strcpy(err_filename, recfilename);

											printf("Error: chn[%d], RecordOpenNewFile failed, filename: %s\n", sMsg.nChn, err_filename);
										}
									}
									
									break;
								}

								b_not_found_valid_disk = 0;
								
								//if(sMsg.nChn == 3)
								{
									//fprintf(stdout, "***chn[%d], format:%d, file:%s, open_offset:%u, fd: %d***\n", 
									//	sMsg.nChn, header.emResolution, recfilename, open_offset, fileno(file[sMsg.nChn]->stream));
								}
								
								max_pts[sMsg.nChn] = header.nPts;//csp modify
								
								startTimeStamp[sMsg.nChn] = header.nTimeStamp;
								
								s_u_info[sMsg.nChn].video_compressor = 3;//h.264
								s_u_info[sMsg.nChn].audio_compressor = 3;//adpcm
								s_u_info[sMsg.nChn].start_time = max_pts[sMsg.nChn] / 1000000;
								s_u_info[sMsg.nChn].end_time = s_u_info[sMsg.nChn].start_time;
								//新文件在文件容器中的开始位置
								s_u_info[sMsg.nChn].start_position = open_offset;
								s_u_info[sMsg.nChn].end_position = s_u_info[sMsg.nChn].start_position;
								s_u_info[sMsg.nChn].image_format = header.emResolution;
								s_u_info[sMsg.nChn].stream_flag = header.nStreamType;
								#ifdef RECORD_UPDATE_FILE_INFO_NOT_FREQUENT
								s_u_info[sMsg.nChn].type = rec_file_type_total[sMsg.nChn];
								#endif
								
								u16 tmp_w = 0;
								u16 tmp_h = 0;
								if(header.width == 0 || header.height == 0)
								{
									RecordGetFrameSize(header.emResolution, &tmp_w, &tmp_h);
								}
								else
								{
									tmp_w = header.width;
									tmp_h = header.height;
								}
								
								//printf("header.nBitRate=%u\n", header.nBitRate);
								//yaogang modify for bad disk
								//custommp4_set_video(file[sMsg.nChn], 1000, tmp_w, tmp_h, (header.emResolution < EM_REC_ENC_N_D1)?25:30, (header.nBitRate) << 10, str2uint("H264"), 0x18);
								if (custommp4_set_video(file[sMsg.nChn], 1000, tmp_w, tmp_h, (header.emResolution < EM_REC_ENC_N_D1)?25:30, (header.nBitRate) << 10, str2uint("H264"), 0x18) != 1)
								{
									if (!ndisk_open_write_err)
									{
										ndisk_open_write_err = 1;
										strcpy(err_filename, file[sMsg.nChn]->file_name);

										printf("Error: chn[%d], custommp4_set_video failed, filename: %s\n", sMsg.nChn, err_filename);
									}

									break;
								}
								
								if(s_u_info[sMsg.nChn].stream_flag > 0)
								{
									//yaogang modify for bad disk
									//custommp4_set_audio(file[sMsg.nChn], 1000, 1, 16, 8000, str2uint("GRAW"), 642, 40);
									if (custommp4_set_audio(file[sMsg.nChn], 1000, 1, 16, 8000, str2uint("GRAW"), 642, 40) != 1)
									{
										if (!ndisk_open_write_err)
										{
											ndisk_open_write_err = 1;
											strcpy(err_filename, file[sMsg.nChn]->file_name);

											printf("Error: chn[%d], custommp4_set_audio failed, filename: %s\n", sMsg.nChn, err_filename);
										}

										break;
									}
								}
								
								flag_file_opened[sMsg.nChn] = 1;
								flag_set_file_start_time[sMsg.nChn] = 0;//csp modify
								
								check_disk_ptn("-open");
							}
							
							if(sMsg.nMsg == EM_REC_FORCE_END)
							{
								//printf("chn%d record force end - step 7$$$\n",sMsg.nChn);
							}
							
							if(header.nMediaType == 0)
							{
								//写入3000视频帧后，更新文件头信息
								//real_write_bytes = custommp4_write_video_frame_with_buf(file[sMsg.nChn], frame_data + sizeof(header), header.nDataLength, (u32)(header.nTimeStamp - startTimeStamp[sMsg.nChn]), header.nFrameType, header.nPts, &update, (char *)rec_w_buf, &rec_w_buf_len, sizeof(rec_w_buf));
								real_write_bytes = custommp4_write_video_frame_with_buf(file[sMsg.nChn], frame_data + sizeof(header), header.nDataLength, (u32)(header.nTimeStamp - startTimeStamp[sMsg.nChn]), header.nFrameType, header.nPts, &update, (char *)g_RecordCacheAddr[sMsg.nChn], &g_RecordCacheUsed[sMsg.nChn], g_RecordCacheLength[sMsg.nChn]);
								//printf("***chn[%d], filename:%s, filePos:%ld, real_write_bytes:%d***\n", sMsg.nChn, file[sMsg.nChn]->file_name, file[sMsg.nChn]->file_position, real_write_bytes);
								
								if(flag_set_file_start_time[sMsg.nChn] == 0)
								{
									if(0 == custommp4_set_file_start_time(file[sMsg.nChn], (u32)(header.nTimeStamp/1000)))
									{
										flag_set_file_start_time[sMsg.nChn] = 1;
									}
								}
								
								check_disk_ptn("-video");
							}
							else
							{
								if(header.nTimeStamp <= startTimeStamp[sMsg.nChn])
								{
									continue; //丢弃文件头上早于文件开始时间的音频帧，否则造成音频异常
								}
								
								#if 1//csp modify
								if(file[sMsg.nChn]->fpot.audio_tracks == 0)
								{
									real_write_bytes = header.nDataLength;
								}
								else
								{
									//NVR used
									//real_write_bytes = custommp4_write_audio_frame_with_buf(file[sMsg.nChn], frame_data + sizeof(header), header.nDataLength, (u32)(header.nTimeStamp - startTimeStamp[sMsg.nChn]), &update, (char *)rec_w_buf, &rec_w_buf_len, sizeof(rec_w_buf));
									real_write_bytes = custommp4_write_audio_frame_with_buf(file[sMsg.nChn], frame_data + sizeof(header), header.nDataLength, (u32)(header.nTimeStamp - startTimeStamp[sMsg.nChn]), &update, (char *)g_RecordCacheAddr[sMsg.nChn], &g_RecordCacheUsed[sMsg.nChn], g_RecordCacheLength[sMsg.nChn]);
									//printf("**\n");
								}
								#else
								real_write_bytes = custommp4_write_audio_frame_with_buf(file[sMsg.nChn], frame_data + sizeof(header), header.nDataLength, (u32)(header.nTimeStamp - startTimeStamp[sMsg.nChn]), &update, (char *)rec_w_buf, &rec_w_buf_len, sizeof(rec_w_buf));
								//printf("**\n");
								#endif
								
								check_disk_ptn("-audio");
							}
							
							if(sMsg.nMsg == EM_REC_FORCE_END)
							{
								//printf("chn%d record force end - step 8$$$\n",sMsg.nChn);
							}
							
							//HDDL
							/*
							wr_err++;
							if (wr_err == 10)
							{
								real_write_bytes = header.nDataLength - 1;
							}
							*/
							#if 0 //for test bad disk
							static int cnt = 0;
							if ((sMsg.nChn == 0) && (header.nMediaType == 0) && (++cnt == 200))
							{
								real_write_bytes = header.nDataLength - 1;								
							}
							#endif
							
							if(real_write_bytes != header.nDataLength)
							{
								nExceptionHappened = 1;
								RecordResetBuf(sMsg.nChn);
								
								//yaogang modify 20141114
								if (header.nMediaType == 0)
								{
									fprintf(stderr, "Warning: chn%d custommp4_write_video_frame wrong!real_write_bytes: %d, nDataLength: %d\n", 
										sMsg.nChn, real_write_bytes, header.nDataLength);
								}
								else
								{
									fprintf(stderr, "Warning: chn%d custommp4_write_audio_frame wrong!real_write_bytes: %d, nDataLength: %d\n", 
										sMsg.nChn, real_write_bytes, header.nDataLength);
								}

								//yaogang modify 20141118
								if (!ndisk_open_write_err)
								{
									ndisk_open_write_err = 1;
									//printf("RecordMainFxn: custommp4_write frame failed: filename: %s\n", file[sMsg.nChn]->file_name);
									strcpy(err_filename, file[sMsg.nChn]->file_name);
									printf("Error: chn[%d], custommp4_write frame failed, filename: %s\n", sMsg.nChn, err_filename);
								}
								
								//g_RecordCacheUsed[sMsg.nChn] = 0;
								
								//custommp4_close(file[sMsg.nChn]);
								//file[sMsg.nChn] = NULL;
								
								//max_pts[sMsg.nChn] = 0;
								//flag_file_opened[sMsg.nChn] = 0;
								//s_u_info[sMsg.nChn].type = 0;
								//flag_set_file_start_time[sMsg.nChn] = 0;
								////rec_file_type[sMsg.nChn] = 0;
								//rec_file_type_total[sMsg.nChn] = rec_file_type[sMsg.nChn];
								
								break;
							}
							
							if(sMsg.nMsg == EM_REC_FORCE_END)
							{
								//printf("chn%d record force end - step 9$$$\n",sMsg.nChn);
							}
							
							//正常情况下，文件写入3000视频帧后update=1
							//在此更新索引文件中的信息
							if(update)
							{
								//time_pre = getTimeStamp();
								
								fileflush(file[sMsg.nChn]->stream);
								s_u_info[sMsg.nChn].end_time = max_pts[sMsg.nChn] / 1000000;
								s_u_info[sMsg.nChn].end_position = custommp4_end_position(file[sMsg.nChn]);
								#ifdef RECORD_UPDATE_FILE_INFO_NOT_FREQUENT
								s_u_info[sMsg.nChn].type = rec_file_type_total[sMsg.nChn];
								#endif
								update_chn_cur_segment(p_ptn_index[sMsg.nChn], sMsg.nChn, &s_u_info[sMsg.nChn], 0);
								
								check_disk_ptn("-update1");

								//time_cur = getTimeStamp();
								#if 0
								if (time_cur > time_pre)
								{
									if (time_interval < (time_cur - time_pre))
									{
										time_interval = time_cur - time_pre;
									}
								}
								else
								{
									printf("file flush time overflow");
								}

								if (sMsg.nChn == 0)
								{
									printf("file flush interval: %u\n", time_interval);
								}
								#endif
							}
							
							if(sMsg.nMsg == EM_REC_FORCE_END)
							{
								//printf("chn%d record force end - step 10$$$\n",sMsg.nChn);
							}
						}	
						
						#if 0//NVR used
						if(file[sMsg.nChn] != NULL)
						{
							if(rec_w_buf_len > 0)
							{
								//printf("file:%s, line:%d\n", __FILE__, __LINE__);
								if(0 > custommp4_write_data(file[sMsg.nChn], rec_w_buf, rec_w_buf_len))
								{
									nExceptionHappened = 1;
								}
								//printf("file:%s, line:%d\n", __FILE__, __LINE__);
								rec_w_buf_len = 0;
							}
						}
						#endif
						
						#if 0
						if(9 == openfile)
						{
							printf("ModRecordStopAll ModRecordStopAll\n");
							ModRecordStopAll();
							break;
						}
						#endif
						
						if(sMsg.nMsg == EM_REC_FORCE_END)
						{
							//printf("chn%d record force end - step 11$$$\n",sMsg.nChn);
						}
						
						if(bufrate > 0)
						{
							g_SendMsgReadBuf[sMsg.nChn] = 0;
						}
						else
						{
							if(file[sMsg.nChn] != NULL)
							{
								#if 1//NVR used
								if(g_RecordCacheUsed[sMsg.nChn] > 0)
								{
									//printf("file:%s, line:%d\n", __FILE__, __LINE__);
									if(0 > custommp4_write_data(file[sMsg.nChn], g_RecordCacheAddr[sMsg.nChn], g_RecordCacheUsed[sMsg.nChn]))
									{
										nExceptionHappened = 1;
										//yaogang modify 20141118
										if (!ndisk_open_write_err)
										{
											ndisk_open_write_err = 1;
											//printf("RecordMainFxn: custommp4_write_data 2 failed: filename: %s\n", file[sMsg.nChn]->file_name);
											strcpy(err_filename, file[sMsg.nChn]->file_name);

											printf("Error: chn[%d], custommp4_write_data 2 failed, filename: %s\n", sMsg.nChn, err_filename);
										}
									}
									//printf("file:%s, line:%d\n", __FILE__, __LINE__);
									g_RecordCacheUsed[sMsg.nChn] = 0;
								}
								#endif
								
								printf("^^^chn%d stop,close file^^^\n",sMsg.nChn);
								
								s_u_info[sMsg.nChn].end_time = max_pts[sMsg.nChn] / 1000000;
								s_u_info[sMsg.nChn].end_position = custommp4_end_position(file[sMsg.nChn]);
								#ifdef RECORD_UPDATE_FILE_INFO_NOT_FREQUENT
								s_u_info[sMsg.nChn].type = rec_file_type_total[sMsg.nChn];
								
								//csp modify
								//if(sMsg.nMsg != EM_REC_FORCE_END)
								if(sMsg.nMsg != EM_REC_FORCE_END && sMsg.nMsg != EM_REC_FLUSH)
								{
									rec_file_type[sMsg.nChn] = 0;
								}
								
								rec_file_type_total[sMsg.nChn] = rec_file_type[sMsg.nChn];
								#endif
								//fileflush(file[sMsg.nChn]->stream);
								//fsync(fileno(file[sMsg.nChn]->stream));
								
								u8 error_flag = file[sMsg.nChn]->error_flag;
								
								custommp4_set_file_end_time(file[sMsg.nChn], (u32)(header.nTimeStamp/1000));
								int rtn = custommp4_close(file[sMsg.nChn]);
								file[sMsg.nChn] = NULL;
								
								pthread_mutex_lock(&g_mutex);//yg modify
								g_diskuse[sMsg.nChn] = 0;//rec/a1/fly00000.ifv
								pthread_mutex_unlock(&g_mutex);
								
								if(error_flag == 0 && rtn == 1)
								{
									if(0 == update_chn_cur_segment(p_ptn_index[sMsg.nChn], sMsg.nChn, &s_u_info[sMsg.nChn], 1))
									{
										printf("update_chn_cur_segment chn[%d] failed!*******\n", sMsg.nChn);
									}
								}
								
								s_u_info[sMsg.nChn].type = 0;
								//flag_file_opened[sMsg.nChn] = 0;//csp modify
								flag_set_file_start_time[sMsg.nChn] = 0;
								//printf("update record file start time:%s", ctime((time_t *)&s_u_info[sMsg.nChn].start_time));
								//printf("update record file end time:%s", ctime((time_t *)&s_u_info[sMsg.nChn].end_time));
							}
							else
							{
								printf("^^^chn%d already stopped^^^\n",sMsg.nChn);
							}
							
							max_pts[sMsg.nChn] = 0;
							flag_file_opened[sMsg.nChn] = 0;
							//flag_ReceiveRightFrame[sMsg.nChn] = 0;//yzw add
							
							if(EM_REC_SUCCESS != RecordResetBuf(sMsg.nChn))
							{
								fprintf(stderr, "RecordResetBuf[%d] error!!!\n", sMsg.nChn);
							}
						}
						
						if(sMsg.nMsg == EM_REC_FORCE_END)
						{
							//printf("chn%d record force end - step 12$$$\n",sMsg.nChn);
						}
						
						g_IsWritingFile[sMsg.nChn] = 0;
						
						#if 0//MR HDD
						if(nExceptionHappened)
						{
							if(time(NULL) - nLastExceptionTime > 600)
							{
								nExceptionTimes = 0;
							}
							nLastExceptionTime = time(NULL);
							nExceptionTimes++;
							if(nExceptionTimes % 100 == 0)
							{
								RecordException(2);//录像文件异常
								ModRecordStopAll();
								u8 tmp = 0;
								for(tmp = 0; tmp < g_ChnNum; tmp++)
								{
									ModRecordResume(tmp);
								}
							}
							else if(nExceptionTimes >= 175)
							{
								RecordException(3);//重启主机
							}
						}
						#endif
						//yaogang modify for disk I/O error
						char diskname[10];
						//int j;
						//SDevInfo* pDevInfo = NULL;

						if(ndisk_open_write_err)
						{
							//error process
							for(i = 0; i < g_ChnNum; i++)
							{
								if (flag_file_opened[i])
								{
									//在此已确定硬盘有坏道，
									//置位后所有文件写操作将直接返回
									file[i]->write_error_flog = 1;
									
									custommp4_close(file[i]);
									file[i] = NULL;
								}

								pthread_mutex_lock(&g_mutex);//yg modify
								g_diskuse[i] = 0;//rec/a1/fly00000.ifv
								pthread_mutex_unlock(&g_mutex);

								max_pts[i] = 0;//csp modify
								flag_file_opened[i] = 0;
								flag_set_file_start_time[i] = 0;
							}
							
							//alarm
							if (err_filename[0] != 0)
							{
								//printf("RecordMainFxn: err_filename: %s\n", err_filename);
								if (get_disk_from_filename(err_filename, diskname, 10) == 0)
								{
									//get disk logic index from diskname
									printf("bad disk dev: %s\n", diskname);

									#if 1
									
									report_disk_err(diskname);
									
									#else
									for(i=0; i<MAX_HDD_NUM; i++)
									{
										pDevInfo = modsys_GetDiskInfo(i);
										if(pDevInfo)
										{
											if (strncmp(pDevInfo->strDevPath, diskname, strlen(diskname)) == 0)
											{
												//printf("RecordMainFxn: err disk logic index: %d\n", pDevInfo->disk_logic_idx);
												set_disk_err_bit(pDevInfo->disk_logic_idx -1);
												printf("bad disk logic idx: %d, SN: %s\n",
													pDevInfo->disk_logic_idx, pDevInfo->disk_sn);
											}

											//report to modsys_UpdateDiskInfoFxn() --> update Gui
											pthread_mutex_lock(&g_mutex);//yg modify
											g_rec_disk_err_bitmask |= 1<<pDevInfo->disk_system_idx;
											pthread_mutex_unlock(&g_mutex);//yg modify
										}
									}
									#endif
								}
							}
							
							ndisk_open_write_err = 0;
							memset(err_filename, 0, sizeof (err_filename));
						}

						if (b_not_found_valid_disk)// && !b_stop_rec)
						{
							//b_stop_rec = 1;
							b_not_found_valid_disk = 0;
							ModRecordStopAll();

							printf("not found valid disk, record stop !!\n");
						}
						
						break;
						
					default:
						fprintf(stderr,"Invalid Message[%d] received by RecordReadMsgQueue[1]!\n", sMsg.nMsg);//Invalid Message[223]//why???
						break;
				}
			}
			
			//printf("*****************chn[%d], msg:%d, msgQID[%d], bufUseRate:%d\n", sMsg.nChn, sMsg.nMsg, nMsgQID, RecordGetBufUseRate(sMsg.nChn));
		}
		
		//printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");		
		
		//csp modify
		//if(g_recorddeinit == 1)
		//{
		//	break;
		//}
		
		RecordGetDiskFull(&nDiskFull);
		if(nDiskFull)
		{
			if(0 == nStopRecordByDiskFull)
			{
				ModRecordStopAll();
				nStopRecordByDiskFull = 1;
				printf("Hard Disk Full! Auto stop record!\n");
			}
			u8 i = 0;
			for(i = 0; i < g_ChnNum; i++)
			{
				if(!g_ChnPaused[i])
				{
					ModRecordPause(i);
				}
			}
		}
		else
		{
			if(nStopRecordByDiskFull)
			{
				u8 i = 0;
				for(i = 0; i < g_ChnNum; i++)
				{
					ModRecordResume(i);
				}
				nStopRecordByDiskFull = 0;
			}
		}
		
		#if 0//csp modify
		if(!nStopRecordByDiskFull)
		{
			continue;
		}
		else
		{
			usleep(100 * 1000);
		}
		#endif
	}

#ifdef MallocFrameBuf

	free(frame_data);

#endif
	//free(flag_force_writefile);
	//free(flag_file_opened);
	//free(file);
	//free(p_ptn_index);
	//free(max_pts);
	//free(startTimeStamp);
	//free(s_u_info);
	//free(rec_w_buf);
	
	printf("pthread RecordMainFxn shutting down ... ...\n");
}

static s32 RecordReadSnapFromBuf(SModRecSnapHeader *pheader, u8* frame_data, u32 data_size)
{
	s32 ret = EM_REC_SUCCESS;
	
	if (NULL == frame_data || NULL == pheader)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	pthread_mutex_lock(&LockSnapBuf);	
	
	ret = ReadDataFromBuf(&g_RecSnapBufInfo, (u8 *)pheader, sizeof(SModRecSnapHeader));
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s ReadDataFromBuf 1\n", __FUNCTION__);
		pthread_mutex_unlock(&LockSnapBuf);
		return ret;
	}
	//图片大小出错
	if (pheader->data_size > data_size)
	{
		printf("%s error snap size %u > data_size %u, reset snap buf\n", __func__, pheader->data_size, data_size);
		
		pthread_mutex_unlock(&LockSnapBuf);
		RecordResetSnapBuf();
		
		return ret;
	}
	
	ret = ReadDataFromBuf(&g_RecSnapBufInfo, (u8 *)frame_data, pheader->data_size);
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s ReadDataFromBuf 2\n", __FUNCTION__);
		ResumeCircleBufToPast(&g_RecSnapBufInfo, 0, 1);
		
		pthread_mutex_unlock(&LockSnapBuf);
		return ret;
	}
	
	BufSnapCnt--;
	pthread_mutex_unlock(&LockSnapBuf);
	
	return ret;
}

int pre_record_snap_deinit()
{	
	if (p_PreRecSnapChnCtl != NULL)
	{
		free(p_PreRecSnapChnCtl);
		p_PreRecSnapChnCtl = NULL;
	}
	
	if (pf_PreRecSnap[0] != NULL)
	{
		fclose(pf_PreRecSnap[0]);
		pf_PreRecSnap[0] = NULL;
	}
	
	if (pf_PreRecSnap[1] != NULL)
	{
		fclose(pf_PreRecSnap[1]);
		pf_PreRecSnap[1] = NULL;
	}

	return 0;
}


int pre_record_snap_init(int MaxChnNum)
{
	int i, ret;
	partition_index *pindex = NULL;
	char filename[64];

	pindex = get_pre_rec_partition(g_HddManager);

	if (pindex == NULL)
	{
		printf("%s HDD NONE\n", __func__);

		pre_record_snap_deinit();
	
		return 1;
	}
	
	if (p_PreRecSnapChnCtl == NULL)
	{
		p_PreRecSnapChnCtl = (TypePreRecSnapChnCtl *)malloc(sizeof(TypePreRecSnapChnCtl) * MaxChnNum);
		
		if (p_PreRecSnapChnCtl == NULL)
		{
			printf("%s p_PreRecSnapChnCtl malloc failed\n", __func__);
			pre_record_snap_deinit();
			return 1;
		}
	}
	
	memset(p_PreRecSnapChnCtl, 0, sizeof(TypePreRecSnapChnCtl) * MaxChnNum);

	for (i=0; i<MaxChnNum; i++)
	{
		p_PreRecSnapChnCtl[i].total = PreRecSnapNumPerChn;
		
		ret = sem_init(&p_PreRecSnapChnCtl[i].sem, 0, 1);//互斥
		if(ret)
		{
			fprintf(stderr, "sem_init p_PreRecSnapChnCtl[%d] error!!\n", i);
			pre_record_snap_deinit();
			return ret;
		}
	}

	
	sprintf(filename,"%s/pic%05d.ifv", pindex->mountpath, 126);
	if (pf_PreRecSnap[0] == NULL)
	{
		pf_PreRecSnap[0] = fopen(filename, "rb+");
		if (pf_PreRecSnap[0] == NULL)
		{
			printf("%s fopen failed, %s\n", __func__, strerror(errno));
			pre_record_snap_deinit();
			return 1;
		}
	}
	

	sprintf(filename,"%s/pic%05d.ifv", pindex->mountpath, 127);
	if (pf_PreRecSnap[1] == NULL)
	{
		pf_PreRecSnap[1] = fopen(filename, "rb+");
		if (pf_PreRecSnap[1] == NULL)
		{
			printf("%s fopen failed, %s\n", __func__, strerror(errno));
			pre_record_snap_deinit();
			return 1;
		}
	}

	return 0;
}


#define nSECPERMIN	(60)
#define nSECPERHOUR	(60*60)
const int indextosec[] = {1, 2, 3, 4, 5, 10*nSECPERMIN, 30*nSECPERMIN, nSECPERHOUR, 12*nSECPERHOUR, 24*nSECPERHOUR};

#if 1

//保存图片的数据结构
typedef struct
{
	u8	pic_type_mask;		//抓图类型掩码，标识当前文件容器中是否保存过相应类型的图片
	u32	chn_mask;			//通道掩码，标识当前文件容器中是否保存过相应通道的图片
	u32	file_pic_nums;		//文件中保存的图片数量
	u32	next_pic_pos;			//写下一张图片时的起始位置
	char filename[64];
	partition_index* index;		//
	FILE *pf; 
} sSnapFile;

//为了避免每次写图片数据都刷新磁盘
typedef struct
{
	u16 num;
	u16 total;
	pic_use_info *pPicInfo;
}sPicInfoBuf;

s32 GetNewSnapFile(sSnapFile *pf)
{
	s32 ret;
	partition_index* index = NULL;
	file_pic_use_info finfo;
	FILE *pfile = NULL;
	char filename[64];

	memset(pf, 0, sizeof(sSnapFile));
	
	if (pf == NULL)
	{
		printf("%s param invalid\n", __func__);
		return EM_REC_INVALID_PARA;
	}

	index = get_pic_rec_partition(g_HddManager);
	if (index == NULL)
	{
		//未找到分区
		printf("%s failed, can not find partition to record picture\n", __FUNCTION__);
		//exit(0);
		return 1;
	}	

	lock_partition_index(index);
	
	ret = get_index_fileinfo(index, index->pic_header.file_cur_no, &finfo, 0);
	if (ret)
	{
		printf("%s: get_index_fileinfo failed\n", __FUNCTION__);
		unlock_partition_index(index);
		return 1;
	}	

	sprintf(filename, "%s/pic%05d.ifv", index->mountpath, index->pic_header.file_cur_no);
	printf("%s filename: %s\n", __FUNCTION__, filename);

	pfile = fopen(filename, "r+");
	if(pfile == NULL)
	{
		printf("%s fopen %s failed\n", __FUNCTION__, filename);
		unlock_partition_index(index);
		return 1;
	}

	if (finfo.status == 2)	//已经写满
	{
		printf("%s finfo.status == 2, impossible\n", __func__);
	}
	
	if (finfo.status == 0)	//初始状态，未使用
	{
		finfo.status = 1; //未写满，正在使用
		finfo.pic_type_mask = 0;
		finfo.chn_mask = 0;
		finfo.next_pic_pos = 0;
		finfo.file_pic_nums = 0;
		finfo.start_sec = 0;
		finfo.start_usec = 0;
		finfo.end_sec = 0;
		finfo.end_usec = 0;	

		ret = refresh_index_fileinfo(index, &finfo, 0);
		if (ret)
		{
			printf("%s: refresh_index_fileinfo failed\n", __FUNCTION__);
			unlock_partition_index(index);
			return 1;
		}
	}

	if (finfo.next_pic_pos != 0)
	{
		ret = fseek(pfile, finfo.next_pic_pos, SEEK_SET);
		if (ret)
		{
			printf("%s: %s fseek to %d failed, %s\n", __FUNCTION__, filename, finfo.next_pic_pos, strerror(errno));
			
			fclose(pfile);
			unlock_partition_index(index);
			
			return 1;
		}
	}
	pf->pic_type_mask = finfo.pic_type_mask;
	pf->chn_mask = finfo.chn_mask;
	pf->next_pic_pos = finfo.next_pic_pos;
	pf->file_pic_nums = finfo.file_pic_nums;
	
	strcpy(pf->filename, filename);
	pf->index = index;
	pf->pf = pfile;
	
	unlock_partition_index(index);
	return EM_REC_SUCCESS;
}

s32 FlushPicInfoBufToFile_unlock(sSnapFile *pSnapFile, sPicInfoBuf *pPicInfoBuf)
{
	s32 ret = 0;
	u16 i;
	
	if ( (pSnapFile == NULL) \
		|| (pPicInfoBuf == NULL) \
		|| (pSnapFile->index == NULL) )
	{
		printf("%s param invalid\n", __func__);
		return 1;
	}

	if (pPicInfoBuf->num)
	{
		for (i=0; i<pPicInfoBuf->num; i++)
		{
			/*
			printf("\t pic_no: %u, file_offset: %u, pic_size: %u\n", 
				pPicInfoBuf->pPicInfo[i].pic_no, pPicInfoBuf->pPicInfo[i].file_offset, pPicInfoBuf->pPicInfo[i].pic_size);
			*/
			ret = refresh_index_picinfo(pSnapFile->index, &pPicInfoBuf->pPicInfo[i], 0);
			if (ret)
			{
				printf("%s refresh_index_picinfo failure, i: %d\n", __func__, i);
				break;
			}
		}
		fileflush(pSnapFile->index->pic_idx);
		fileflush(pSnapFile->index->pic_idx_bk);
	}

	return ret;
}

s32 FlushSnapIndex(sSnapFile *pSnapFile, sPicInfoBuf *pPicInfoBuf, u8 FileFull)
{
	partition_pic_index_header * phead = NULL;
	partition_pic_index_header head;
	file_pic_use_info finfo;
	//pic_use_info pinfo;

	time_t		start_sec;
	suseconds_t	start_usec;
	time_t		end_sec;
	suseconds_t	end_usec;
	u16 num;	//pPicInfoBuf 中图片数量
	u32 verify, i;
	s32 ret;

	if ( (pSnapFile == NULL) \
		|| (pPicInfoBuf == NULL) \
		|| (pSnapFile->index == NULL) )
	{
		printf("%s param invalid\n", __func__);
		return 1;
	}
	
	phead = &pSnapFile->index->pic_header;

	//*****************************
	lock_partition_index(pSnapFile->index);
	
	// 1、刷新文件容器pf							
	fileflush(pSnapFile->pf);

	num = pPicInfoBuf->num;
	
	if (num) //如果缓冲中有图片
	{
		//printf("%s pPicInfoBuf->num: %d\n", __func__, num);
		start_sec	= pPicInfoBuf->pPicInfo[0].tv_sec;
		start_usec	= pPicInfoBuf->pPicInfo[0].tv_usec;
		end_sec	= pPicInfoBuf->pPicInfo[num-1].tv_sec;
		end_usec	= pPicInfoBuf->pPicInfo[num-1].tv_usec;
		
		// 2、更新图片索引	
		ret = FlushPicInfoBufToFile_unlock(pSnapFile, pPicInfoBuf);
		if (ret)
		{
			printf("%s: FlushPicInfoBufToFile_unlock failed 1\n", __func__);
			unlock_partition_index(pSnapFile->index);
			return 1;
		}
	}
	/*
	else
	{	//得到文件中最后一个图片的时间作为结束时间
		ret = get_index_picinfo(pSnapFile->index, pSnapFile->index->pic_header.file_cur_no, 
				pSnapFile->file_pic_nums-1, &pinfo, 0);
		if (ret)
		{
			printf("%s: FlushPicInfoBufToFile_unlock failed 1\n", __func__);
			unlock_partition_index(pSnapFile->index);
			return 1;
		}
		end_sec	= pinfo.tv_sec;
		end_usec	= pinfo.tv_usec;
	}
	*/
	
	if (num || FileFull)
	{
		// 3、更新文件索引
		ret = get_index_fileinfo(pSnapFile->index, phead->file_cur_no, &finfo, 0);
		if (ret)
		{
			printf("%s: get_index_fileinfo failed 1\n", __func__);
			unlock_partition_index(pSnapFile->index);
			return 1;
		}

		if (FileFull)
		{			
			finfo.status = 2;//已写满
		}
		else
		{
			finfo.status = 1;
		}
		
		finfo.pic_type_mask |= pSnapFile->pic_type_mask;
		finfo.chn_mask |= pSnapFile->chn_mask;
		finfo.next_pic_pos = pSnapFile->next_pic_pos;
		finfo.file_pic_nums = pSnapFile->file_pic_nums;
		
		if (num)
		{
			finfo.end_sec = end_sec;
			finfo.end_usec = end_usec;

			if (finfo.start_sec == 0)
			{
				finfo.start_sec = start_sec;
				finfo.start_usec = start_usec;
			}
		}

		#if 1		
		if (FileFull)
		{
			struct tm start,end;
			localtime_r(&finfo.start_sec, &start);
			localtime_r(&finfo.end_sec, &end);
		
			printf("%s file full, file info: \n"
					"\t pic_buf_num: %u\n"
					"\t file_pic_nums: %u\n"
					"\t next_pic_pos: %u\n"
					"\t start time: %d-%d-%d\n"
					"\t end time: %d-%d-%d\n", 
					__func__, 
					num, 
					finfo.file_pic_nums, 
					finfo.next_pic_pos,
					start.tm_hour+8, start.tm_min, start.tm_sec,
					end.tm_hour+8, end.tm_min, end.tm_sec);
		}
		#endif

		ret = refresh_index_fileinfo(pSnapFile->index, &finfo, 0);
		if (ret)
		{
			printf("%s: refresh_index_file failed 1\n", __func__);
			unlock_partition_index(pSnapFile->index);
			return 1;
		}							
	
		// 4、更新索引头
		//判断该分区内文件容器是否写完
		memcpy(&head, phead, sizeof(partition_pic_index_header));

		if (FileFull)
		{
			if (head.file_cur_no + 1 >=  head.file_nums)	//该分区的所有文件容器都写完
			{						
				head.file_cur_no = head.file_nums;//标示该分区文件容器已经用完
			}
			else
			{
				head.file_cur_no++;
			}
		}
		
		if (num)
		{
			head.partition_pic_nums += num;
			head.end_sec = end_sec;
			head.end_usec = end_usec;
			
			if (head.start_sec == 0)
			{
				head.start_sec = start_sec;
				head.start_usec = start_usec;
			}												
		}
		
		
		u8 *p = (u8 *)&head;
		verify = 0;
		for (i=0; i<sizeof(partition_pic_index_header)-4; i++)
		{
			verify += p[i];
		}
		head.verify = verify;
			
		ret = refresh_index_header(pSnapFile->index, &head, 0);						
		if (ret)
		{
			printf("%s: refresh_index_header failed 1\n", __func__);
			unlock_partition_index(pSnapFile->index);
			return 1;
		}

		memcpy(phead, &head, sizeof(partition_pic_index_header));
	}
	unlock_partition_index(pSnapFile->index);

	#if 0		//for test
						
	FILE *pf_test;
	static int cnt = 0;
	char testfilename[64];
	u8 *pbuf;//SnapSize;
	pic_use_info *pinfo;
	pbuf = (u8 *)malloc(SnapSize);
	if (pbuf == NULL)
	{
		printf("%s buf malloc failure\n", __func__);
		return 0;
	}

	if (num)
	{
		for (i=0; i<pPicInfoBuf->num; i++)
		{
			pinfo = &pPicInfoBuf->pPicInfo[i];
			ret = read_snap_file(pSnapFile->index, pbuf, pinfo->file_no, pinfo->file_offset, pinfo->pic_size);
			if (ret == 0)
			{
				sprintf(testfilename, "/mnt/snap_read%d.jpg", cnt++);
				pf_test = fopen(testfilename, "w");
				if (pf_test == NULL)
				{
					printf("%s fopen %s failure, %s\n", __func__, testfilename, strerror(errno));
					break;
				}

				if (fwrite(pbuf, pinfo->pic_size, 1, pf_test) != 1)
				{
					printf("%s write %s failure, %s\n", __func__, testfilename, strerror(errno));
					break;
				}
				fclose(pf_test);
				printf("%s: write %s over\n", __func__, testfilename);
								
			}
		}
	}
	
	#endif

	return 0;
}

//预录: 使用最后两个图片文件容器，单独管理
//刷新磁盘策略
/*
1、当前文件容器写满，更换文件容器时
2、运行状态改变且上一个状态是TypeRecSnapRun 时
3、TypeRecSnapRun 状态下每隔2S
4、PicInfoBuf 使用率大于50%
*/
void* RecordSnapFxn(void* data)
{
	s32 ret;
	//s32 secs;
	u8 *pSnapData = NULL;//NVR used
	SModRecSnapHeader header;
	u8 chn_num = (u8)data;
	
	SSnapFxnMsg msg;
	SnapFxnMsgType status, pre_status;
	sSnapFile SnapFile;

	partition_index *pindex = NULL;
	//partition_pic_index_header * phead = NULL;
	//partition_pic_index_header head;
	//file_pic_use_info finfo;
	pic_use_info pic;
	sPicInfoBuf PicInfoBuf;
	//u32 verify;
	u8 flag_hotplug = 0;
	u8 flag_format = 0;
	u8 flag_update = 0;
	u8 b_ReadSnapfromBuf; //是否需要从BUF中读图片
	u8 b_RecSnap;	//是否需要保存图片
	struct timespec tv;
	//time_t		end_sec;		//文件容器内最后一张图片时间	
	//suseconds_t	end_usec;
	u32 pre_pts, cur_pts;
	

	printf("%s running chn_num: %d**************************\n", __func__, chn_num);
	
	if ( pthread_detach(pthread_self()) ) 
	{
		printf("%s pthread_detach failed\n", __func__);
		pthread_exit((void *)(-1));
	}

	pSnapData = malloc(SnapSize);
	if (pSnapData == NULL)
	{
		printf("%s pSnapData malloc failed\n", __func__);
		pthread_exit((void *)(-1));
	}

	//为了避免每次写图片数据都刷新磁盘
	PicInfoBuf.num = 0;
	PicInfoBuf.total = chn_num * 8;
	PicInfoBuf.pPicInfo = (pic_use_info *)malloc(PicInfoBuf.total * sizeof(pic_use_info));
	if (PicInfoBuf.pPicInfo == NULL)
	{
		printf("%s PicInfoBu malloc failed\n", __func__);
		pthread_exit((void *)(-1));
	}

	pre_pts = 0;
	pre_status = status = TypeRecSnapWillStart;
	
	while(1)
	{
		//csp modify		
		if(g_recorddeinit)
		{
			printf("%s g_recorddeinit thread quit!!\n", __func__);
			break;
		}	

		b_ReadSnapfromBuf = 0;
		b_RecSnap = 0;
		
		//从BUF中取一张图片				
		clock_gettime(CLOCK_MONOTONIC, &tv);
		tv.tv_sec += 1;//超时1s
		
		pthread_mutex_lock(&LockSnapBuf);
		
		if (BufSnapCnt == 0)
		{
			//printf("%s pthread_cond_timedwait\n", __func__);
			ret = pthread_cond_timedwait(&CondSnapBuf, &LockSnapBuf, &tv);
			if (ret)
			{
				if (ETIMEDOUT == ret)
				{
					//printf("%s pthread_cond_timedwait timeout\n", __func__);					
				}
				else
				{
					printf("%s pthread_cond_timedwait failed, ret: %d\n", __func__, ret);
				}				
			}
			else
			{
				b_ReadSnapfromBuf = 1;
			}
		}
		else
		{
			b_ReadSnapfromBuf = 1;
		}
		
		pthread_mutex_unlock(&LockSnapBuf);
		
		//printf("%s b_ReadSnapfromBuf: %d, BufSnapCnt: %d, pts: %u\n", __func__, b_ReadSnapfromBuf, BufSnapCnt, getTimeStamp());
		if (b_ReadSnapfromBuf) 
		{
			if (BufSnapCnt == 0)
			{
				printf("%s after pthread_cond_timedwait---BufSnapCnt == 0, impossible\n", __func__);
			}
			else
			{
				//读一张Snap，起始是图片信息头
				ret = RecordReadSnapFromBuf(&header, pSnapData, SnapSize);//BufSnapCnt--
				if(EM_REC_SUCCESS == ret)
				{
					//printf("%s ReadSnapformBuf success, chn:%d, pic_type: 0x%x, time: %d\n", \
					//	__func__, header.chn, header.pic_type, header.tv_sec);
					b_RecSnap = 1;					
				}			
				else
				{
					printf("%s RecordReadSnapFromBuf failed! error code = %d\n",__func__, ret);					
				}
			}
		}
		
		//读消息
		ret = RecordSnapReadMsgQueue(&msg);
		if (ret == 0)
		{	
			printf("%s recv msg, cause: %d, type: %d\n", __func__, msg.cause, msg.type);
			switch (msg.type)
			{
				case TypeRecSnapWillStart:
				{
					printf("%s get msg TypeRecSnapWillStart, pre status = %d\n", __func__, status);
					status = msg.type;
					
					if (msg.cause == 0)//format
					{
						printf("%s pre flag_format: %d\n", __func__, flag_format);
						if (flag_format == 1)
						{
							flag_format = 0;
						}						
					}
					else if (msg.cause == 1)//hotplug
					{
						printf("%s pre flag_hotplug: %d\n", __func__, flag_hotplug);
						if (flag_hotplug == 1)
						{
							flag_hotplug = 0;
						}						
					}
					else if (msg.cause == 2)//update
					{
						printf("%s pre flag_update: %d\n", __func__, flag_update);
						if (flag_update == 1)
						{
							flag_update = 0;
						}						
					}
				} break;
				case TypeRecSnapStop:
				{
					printf("%s get msg TypeRecSnapStop, pre status = %d\n", __func__, status);

					if ((status != TypeRecSnapStop) && (status != TypeRecSnapIdle))
					{
						status = TypeRecSnapStop;//msg.type;
					}
					
					if (msg.cause == 0)//format
					{
						printf("%s pre flag_format: %d\n", __func__, flag_format);
						if (flag_format == 0)
						{
							flag_format = 1;
						}						
					}
					else if (msg.cause == 1)//hotplug
					{
						printf("%s pre flag_hotplug: %d\n", __func__, flag_hotplug);
						if (flag_hotplug == 0)
						{
							flag_hotplug = 1;
						}						
					}
					else if (msg.cause == 2)//update
					{
						printf("%s pre flag_update: %d\n", __func__, flag_update);
						if (flag_update == 0)
						{
							flag_update = 1;
						}						
					}
				} break;
				default:
				{
					printf("%s read msg type :%d invalid\n", __func__, msg.type);
				}
			}
		}
		
		//根据运行状态处理图片
		//printf("%s run status: %d, pts: %u, BufSnapCnt: %u\n", __func__, status, getTimeStamp(), BufSnapCnt);
		switch (status)
		{
			case TypeRecSnapWillStart:	//将要启动，其中检测各种状态
			{
				//检测格式化和硬盘热插拔
				if (flag_format)
				{
					printf("%s formatting\n", __func__);
					break;
				}

				if (flag_hotplug)
				{
					printf("%s hot plugging\n", __func__);
					break;
				}

				if (flag_update)
				{
					printf("%s flag_update\n", __func__);
					break;
				}
				//当前仅仅检测有无硬盘
				pindex = get_pre_rec_partition(g_HddManager);
				if (pindex == NULL)//无硬盘
				{
					//printf("%s HDD NONE\n", __func__);
					break;
				}
				
				printf("%s status = TypeRecSnapStart\n", __func__);
				status = TypeRecSnapStart;				
			}
			//break;//继续处理
			case TypeRecSnapStart:	//内部数据初始化
			{
				
				//预录初始化
				ret = pre_record_snap_init(chn_num);
				if (ret)
				{
					printf("%s pre_record_snap_init failed 1, chn_num: %d\n", __func__, chn_num);
					break;
				}
				
				//保存初始化
				ret = GetNewSnapFile(&SnapFile);
				if (ret)
				{
					printf("%s GetNewSnapFile failed 1\n", __func__);
					pre_record_snap_deinit();
					break;
				}

				printf("%s status = TypeRecSnapRun\n", __func__);
				pre_pts = 0;
				PicInfoBuf.num = 0;
				status = TypeRecSnapRun;
			} 
			//break;//继续处理
			case TypeRecSnapRun:	//正常运行
			{
				cur_pts = getTimeStamp();
					
				if (pre_pts == 0)
				{
					pre_pts = cur_pts;
				}
					
				if (b_RecSnap)
				{
					b_RecSnap = 0;

					#if 1
					//测试硬盘，暂时关闭预录20150527
					//预录
					if (header.pic_type & (1<<5))
					{
						ret = ModRecordPreSnapToFile(header.chn, \
													header.tv_sec, \
													pSnapData, \
													header.data_size, \
													header.width, \
													header.height);
						if(ret)
						{
							printf("%s ModRecordPreSnapToFile failed, ret: %d\n", __func__, ret);
							status = TypeRecSnapRst;
							break;
						}
					}
					#endif
					
					//保存
					header.pic_type &= ~(1<<5);//去掉预录标志

					if (header.pic_type)
					{
						//判断文件容器满，图片数量和文件长度
						if ((SnapFile.file_pic_nums + 1 >= PIC_NUMS_PER_FILE) \
							|| (SnapFile.next_pic_pos + header.data_size > SIZE_OF_FILE_VECTOR))
						{	
							printf("%s: %s full\n", __func__, SnapFile.filename);
							ret = FlushSnapIndex(&SnapFile, &PicInfoBuf, 1); //文件满刷新
							if (ret)
							{
								printf("%s: FlushSnapIndex failed 1, status: %d\n", __func__, status);
								PicInfoBuf.num = 0;
								status = TypeRecSnapRst;
								break;
							}
							PicInfoBuf.num = 0;
							printf("%s FlushSnapIndex full, pts: %u\n", __func__, getTimeStamp());							

							fclose(SnapFile.pf); //文件写满，关闭文件
							SnapFile.pf = NULL;
							//得到新的文件容器
							ret = GetNewSnapFile(&SnapFile);
							if (ret)
							{
								printf("%s: GetNewSnapFile failed 2\n", __func__);
								status = TypeRecSnapRst;
								break;
							}
						}

						//test
						#if 0
						
						FILE *pf_test;
						static int cnt = 0;
						char testfilename[64];
						sprintf(testfilename, "/mnt/snap_write%d.jpg", cnt++);
						pf_test = fopen(testfilename, "w");
						fwrite(pSnapData, header.data_size, 1, pf_test);
						fclose(pf_test);
						#endif
						//test end
						
						//写入文件
						ret = fwrite(pSnapData, header.data_size, 1, SnapFile.pf);
						if (ret != 1)
						{
							printf("%s: %s fwrite failed, %s\n", __func__, SnapFile.filename,  strerror(errno));
							status = TypeRecSnapRst;
							break;
						}
						//fileflush(SnapFile.pf);//yaogang test for 2T HDD
						#if 0
						printf("%s before fwrite file pos: %d, snap size: %d\n", __func__, ftell(SnapFile.pf) - header.data_size, header.data_size);
						#endif

						//图片信息写入缓冲
						pic.chn = header.chn;
						pic.pic_type = header.pic_type;
						pic.file_no = SnapFile.index->pic_header.file_cur_no;
						pic.width = header.width;
						pic.height = header.height;
						pic.file_offset = SnapFile.next_pic_pos;//图片在文件容器中的偏移
						pic.pic_no = SnapFile.file_pic_nums;//图片序号
						pic.pic_size = header.data_size;						
						pic.tv_sec = header.tv_sec;
						pic.tv_usec = header.tv_usec;

						PicInfoBuf.pPicInfo[PicInfoBuf.num++] = pic;
						
						//更新SnapFile
						#if 0
						printf("%s SnapFile.next_pic_pos: %d\n", __func__, SnapFile.next_pic_pos);
						#endif
						SnapFile.pic_type_mask |= header.pic_type;
						SnapFile.chn_mask |= 1<<header.chn;
						SnapFile.file_pic_nums++;
						SnapFile.next_pic_pos += header.data_size;
						
						//printf("%s write to HDD over, pts: %u\n", __func__, getTimeStamp());
					}	
				}

				if (PicInfoBuf.num * 100 /PicInfoBuf.total > 50)	//PicInfoBuf 使用率大于50% 刷新
				{
					ret = FlushSnapIndex(&SnapFile, &PicInfoBuf, 0); 
					if (ret)
					{
						printf("%s: FlushSnapIndex failed 2, status: %d\n", __func__, status);
						status = TypeRecSnapRst;
						PicInfoBuf.num = 0;
						break;
					}
					PicInfoBuf.num = 0;
					printf("%s FlushSnapIndex use 50%%, pts: %u\n", __func__, getTimeStamp());
				}

				if (pre_pts && ((pre_pts+1900) < cur_pts))	//TypeRecSnapRun 状态下每隔2S
				{
					pre_pts = cur_pts;
					
					ret = FlushSnapIndex(&SnapFile, &PicInfoBuf, 0); 
					if (ret)
					{
						printf("%s: FlushSnapIndex failed 3, status: %d\n", __func__, status);
						status = TypeRecSnapRst;
						PicInfoBuf.num = 0;
						break;
					}
					PicInfoBuf.num = 0;
					//printf("%s FlushSnapIndex 2S, pts: %u\n", __func__, getTimeStamp());
				}
			} break;

			case TypeRecSnapStop:	//启动升级或格式化时需要关闭
			{
				//flush all
				if (pre_status == TypeRecSnapRun)
				{
					//flush
					ret = FlushSnapIndex(&SnapFile, &PicInfoBuf, 0); 
					if (ret)
					{
						printf("%s: FlushSnapIndex failed, status: %d\n", __func__, status);
						//status = TypeRecSnapRst;
						//PicInfoBuf.num = 0;
						//break;
					}
					PicInfoBuf.num = 0;
				}
				
				pre_record_snap_deinit();
				
				if (SnapFile.pf != NULL)
				{
					fclose(SnapFile.pf);
				}
				memset(&SnapFile, 0, sizeof(SnapFile));
				
				status = TypeRecSnapIdle;
			} break;
			
			case TypeRecSnapIdle:	//升级或格式化进行中等待启动
			{
				//do nothing
			} break;

			case TypeRecSnapRst:
			{
				//flush all
				if (pre_status == TypeRecSnapRun)
				{
					//flush
					ret = FlushSnapIndex(&SnapFile, &PicInfoBuf, 0); 
					if (ret)
					{
						printf("%s: FlushSnapIndex failed, status: %d\n", __func__, status);
						//status = TypeRecSnapRst;
						//PicInfoBuf.num = 0;
						//break;
					}
					PicInfoBuf.num = 0;
				}
				
				pre_record_snap_deinit();
				
				if (SnapFile.pf != NULL)
				{
					fclose(SnapFile.pf);
					SnapFile.pf = NULL;
				}
				memset(&SnapFile, 0, sizeof(SnapFile));

				status = TypeRecSnapWillStart;
			} break;
			default:
			{
				printf("%s status: %d invalid\n", __func__, status);
			}
		}

		pre_status = status;
	}
	
	printf("pthread RecordSnapFxn shutting down ... ...\n");
	
	pthread_exit((void *)0);
}
#endif

s32 ModRecordInit(u8 chn_num, SModRecordRecPara* para, FPMODRECSTATUS getrecstatus, RecHddInfo hddinfo, FPMODRECEXCEPTION RecExceptCB)
{
	if(g_recordinit == 1)
	{
		fprintf(stderr, "error : ModRecordInit has been done!\n");
		return EM_REC_HASBEENINIT;
	}
	
	if((NULL == para) || (NULL == getrecstatus) || (NULL == hddinfo))
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	g_HddManager = (disk_manager*)hddinfo;
	
	u8 i = 0;
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		if(1)//g_HddManager->hinfo[i].is_disk_exist
		{
			break;
		}
	}
	if(i >= MAX_HDD_NUM)
	{
		fprintf(stderr,"Warnning: No hardDisk, sth will not be done in function [%s] !\n", __FUNCTION__);
		g_DvrHasHdd = 0;
		g_recordinit = 1;
		g_recorddeinit = 0;
		fprintf(stdout, "ModRecordInit success! Partly!\n");
		return EM_REC_SUCCESS;//return EM_REC_NODISK;
	}
	
	g_DvrHasHdd = 1;
	
	g_ChnNum = chn_num;
	printf("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH g_ChnNum = %d, chn_num = %d\n",g_ChnNum,chn_num);
	
	//NVR used
	//g_nMsgQNum = (g_ChnNum >> 3) + ((g_ChnNum % 8) ? 1 : 0); //每个线程处理8个通道
	g_nMsgQNum = 1;
	
	g_RecordStatus = malloc(g_ChnNum);
	if(g_RecordStatus == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memset(g_RecordStatus, 0, g_ChnNum);
	
	g_RecordStatusPast = malloc(g_ChnNum);
	if(g_RecordStatusPast == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memset(g_RecordStatusPast, 0, g_ChnNum);
	
	g_RecordStatusDelay = malloc(g_ChnNum);
	if(g_RecordStatusDelay == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memset(g_RecordStatusDelay, 0, g_ChnNum);
	
	g_ChnPaused = malloc(g_ChnNum);
	if(g_ChnPaused == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memset(g_ChnPaused, 0, g_ChnNum);
	
	g_RecSchOnceDone = malloc(g_ChnNum * 3);
	if(g_RecSchOnceDone == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memset(g_RecSchOnceDone, 0, g_ChnNum * 3);
	
	g_IsPreRecording = malloc(g_ChnNum);
	if(g_IsPreRecording == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memset(g_IsPreRecording, 0, g_ChnNum);
	
	g_SendMsgReadBuf = malloc(g_ChnNum);
	if(g_SendMsgReadBuf == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memset(g_SendMsgReadBuf, 0, g_ChnNum);
	
	g_IsWritingFile = malloc(g_ChnNum);
	if(g_IsWritingFile == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memset(g_IsWritingFile, 0, g_ChnNum);
	
	g_RecordDelayTime = (u32 *)malloc(g_ChnNum * sizeof(u32));
	if(g_RecordDelayTime == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memset(g_RecordDelayTime, 0, g_ChnNum * sizeof(u32));
	
	g_RecordPreTime = (u32 *)malloc(g_ChnNum * sizeof(u32));
	if(g_RecordPreTime == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memset(g_RecordPreTime, 0, g_ChnNum * sizeof(u32));
	
	g_RecSchPara = (SModRecSchPara *)malloc(g_ChnNum * 3 * sizeof(SModRecSchPara));
	if(g_RecSchPara == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memset(g_RecSchPara, 0, g_ChnNum * 3 * sizeof(SModRecSchPara));
	
	g_RecChnTrigger = (SModRecChnTrigger *)malloc(g_ChnNum * sizeof(SModRecChnTrigger));
	if(g_RecChnTrigger == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memset(g_RecChnTrigger, 0, g_ChnNum * sizeof(SModRecChnTrigger));
	
	g_RecBufInfo = (SCircleBufInfo *)malloc(g_ChnNum * sizeof(SCircleBufInfo));
	if(g_RecBufInfo == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memset(g_RecBufInfo, 0, g_ChnNum * sizeof(SCircleBufInfo));
	
	g_MsgQueueInfo = (SCircleBufInfo *)malloc((1 + g_nMsgQNum) * sizeof(SCircleBufInfo));
	if(g_MsgQueueInfo == NULL)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memset(g_MsgQueueInfo, 0, (1 + g_nMsgQNum) * sizeof(SCircleBufInfo));
	
	g_RecChnBufLength = (u32*)calloc(g_ChnNum, sizeof(u32));
	g_RecordChnReleased = (u8*)calloc(g_ChnNum, sizeof(u8));
	g_LastTimeGetFrame = (time_t*)calloc(g_ChnNum, sizeof(time_t));
	
	u32 len = 0;
	//len = (((para->nBitRate << 10) >> 3) << 5);
	u8 nVirtualSeconds = 30;//录像缓冲区能存储的最大录像时长 = nVirtualSeconds/10 秒
	if(g_ChnNum >= 16)
	{
		nVirtualSeconds = 30;
	}
	if((g_ChnNum == 8) && (para[7].nBitRate <= 1024))//9508
	{
		nVirtualSeconds = 20;
	}
	//csp modify 20130429
	#if defined(CHIP_HISI3531)
	if(g_ChnNum == 4)//4路SDI
	{
		nVirtualSeconds = 40;//加大录像缓存是为了增加预录时间到10秒
	}
	#endif
	
	nVirtualSeconds = 40;//NVR used
	
	s32 ret = EM_REC_SUCCESS;
	for(i = 0; i < g_ChnNum; i++)
	{
		//csp modify 20121016//这里需要验证
		//4SDI:3M buffer
		//len = ((((para[i].nBitRate << 10) >> 3) * nVirtualSeconds) / 10);
		len = 3*1024*1024;
		g_RecChnBufLength[i] = len;
		printf("chn%d record buf len=%d,bitrate=%d,time=%ds\n",i,len,para[i].nBitRate,nVirtualSeconds);
		ret = RecordCreateBuf(i, len);
		if(EM_REC_SUCCESS != ret)
		{
			fprintf(stderr, "RecordCreateBuf : chn[%d] error!\n", i);
			return ret;
		}
		//printf("RecordCreateBuf chn[%d], len[0x%x]\n",i, len);
	}
	
	g_semMsgQueue = (sem_t *)malloc(g_nMsgQNum * sizeof(sem_t));
	for(i = 0; i < g_nMsgQNum; i++)
	{
		if(0 != (ret = sem_init(&g_semMsgQueue[i], 0, 0)))
		{
			fprintf(stderr, "sem_init error!!\n");
			return ret;
		}
		
		#if 0
		//start test
		int sval = 0;
		
		sem_post(&g_semMsgQueue[i]);
		sem_getvalue(&g_semMsgQueue[i], &sval);
		printf("sem_getvalue-step 1:[%d]\n",sval);
		
		sem_post(&g_semMsgQueue[i]);
		sem_getvalue(&g_semMsgQueue[i], &sval);
		printf("sem_getvalue-step 2:[%d]\n",sval);
		
		sem_post(&g_semMsgQueue[i]);
		sem_getvalue(&g_semMsgQueue[i], &sval);
		printf("sem_getvalue-step 3:[%d]\n",sval);
		//test end
		#endif
	}
	
	if(EM_REC_SUCCESS != (ret = RecordCreateMsgQueue(0)))
	{
		fprintf(stderr, "RecordCreateMsgQueue[0] error!!!\n");
		return ret;
	}
	
	for(i = 0; i < g_nMsgQNum; i++)
	{
		if(EM_REC_SUCCESS != (ret = RecordCreateMsgQueue(i + 1)))
		{
			fprintf(stderr, "RecordCreateMsgQueue[%d] error!!!\n", i+1);
			return ret;
		}
	}
	
#if 1//csp modify
	nSemForBuffer = (sem_t *)malloc(g_ChnNum * sizeof(sem_t));
	for(i = 0; i < g_ChnNum; i++)
	{
		if(0 != (ret = sem_init(&nSemForBuffer[i], 0, 1)))
		{
			fprintf(stderr, "sem_init error!!\n");
			return ret;
		}
	}
#endif
	
	//NVR used
	g_RecordCacheAddr = (u8 **)malloc(g_ChnNum * sizeof(u8*));
	g_RecordCacheLength = (u32 *)malloc(g_ChnNum * sizeof(u32));
	g_RecordCacheUsed = (u32 *)malloc(g_ChnNum * sizeof(u32));
	if(g_RecordCacheAddr == NULL || g_RecordCacheLength == NULL || g_RecordCacheUsed == NULL)
	{
		fprintf(stderr, "malloc failed for record cache???\n");
		return EM_REC_MALLOC_ERR;
	}
	for(i = 0; i < g_ChnNum; i++)
	{
		g_RecordCacheLength[i] = (1<<20);
		g_RecordCacheUsed[i] = 0;
		g_RecordCacheAddr[i] = (u8 *)malloc(1<<20);
		if(g_RecordCacheAddr[i] == NULL)
		{
			fprintf(stderr, "malloc failed for chn%d record cache!\n", i);
			return EM_REC_MALLOC_ERR;
		}
	}
	
	g_recorddeinit = 0;
	
	RecordRefreshStatus = getrecstatus;
	RecordException = RecExceptCB;
	
	pthread_t id;
	if((ret = (pthread_create(&id, NULL, (void*)RecordStatusManagerFxn, NULL))) != 0)
	{
		fprintf(stderr, "create pthread error!!\n");
		return ret;
	}
	
	g_pTmpPara = (u8 *)calloc(sizeof(u8), g_nMsgQNum);//发现静态变量比较安全，局部变量当参数传递在3531上出现了值错误
	for(i = 0; i < g_nMsgQNum; i++)
	{
		*(g_pTmpPara+i) = i;
		if((ret = (pthread_create(&id, NULL, (void*)RecordMainFxn, (void*)(g_pTmpPara+i)))) != 0)
		{
			fprintf(stderr, "create pthread error!!\n");
			return ret;
		}
	}
	
#if 0//csp modify
	nSemForBuffer = (sem_t *)malloc(g_ChnNum * sizeof(sem_t));
	for(i = 0; i < g_ChnNum; i++)
	{
		if(0 != (ret = sem_init(&nSemForBuffer[i], 0, 1)))
		{
			fprintf(stderr, "sem_init error!!\n");
			return ret;
		}
	}
#endif
	
//yg modify
	memset(g_diskuse, 0, sizeof(g_diskuse));
	g_rec_disk_err_bitmask = 0;
//yaogang modify 20150114 for record snap
	//为了顺序写硬盘
	//创建缓冲区
	/*
	ret = sem_init(&SemForSnapBuffer, 0, 1);//互斥
	if(ret)
	{
		fprintf(stderr, "sem_init SemForSnapBuffer error!!\n");
		return ret;
	}
	
	ret = sem_init(&SemForSnapCount, 0, 0);//计数，Buffer中Snap的数量。
	if(ret)
	{
		fprintf(stderr, "sem_init SemForSnapCount error!!\n");
		return ret;
	}	
	*/
	/*
	CLOCK_MONOTONIC：Represents monotonic time. Cannot be set. 表示单调时间，
	为系统起机时到现在的时间,不能被设置跟修改.
	*/
	pthread_condattr_t condattr;
	pthread_condattr_init(&condattr);
	pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);

	pthread_mutex_init(&LockSnapBuf, NULL);//非递归锁
	pthread_cond_init(&CondSnapBuf, &condattr);//初始化条件变量
	BufSnapCnt = 0;
	
	memset(&g_RecSnapBufInfo, 0, sizeof(SCircleBufInfo));
	g_RecSnapBufInfo.nBufId = 0;
	g_RecSnapBufInfo.nLength = SnapSize * g_ChnNum *  4;//2倍
	
	ret = EM_REC_SUCCESS;

	ret = RecordSnapCreateMsgQueue(sizeof(SSnapFxnMsg) * 32);
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "RecordSnapCreateMsgQueue error!!!\n");
		return ret;
	}
	
	ret = CreateCircleBuf(&g_RecSnapBufInfo);
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "%s snap CreateCircleBuf g_RecSnapBufInfo failed\n", __FUNCTION__);
		return ret;
	}
	printf("%s snap CreateCircleBuf g_RecSnapBufInfo success, size: %d\n", __FUNCTION__, g_RecSnapBufInfo.nLength);

	//预录初始化
	//pre_record_snap_init(g_ChnNum);
	
	//创建线程
	//测试硬盘，暂时关闭预录20150527

	//yaogang modify for bad disk
	ret = pthread_create(&id, NULL, (void*)RecordSnapFxn, (void *)g_ChnNum);
	if(ret)
	{
		fprintf(stderr, " %s create pthread RecordSnapFxn error!!\n", __func__);
		return ret;
	}
	
	//yaogang modify 20150114 for record snap  end
		
	g_recordinit = 1;

	//yaogang modify for bad disk
	#if 0
	pthread_mutex_init(&check_disk_err.lock, NULL);
	check_disk_err.disk_status = 0;
	check_disk_err.init_status = 1;
	#endif
	
	fprintf(stdout, "ModRecordInit success!\n");
	
	return ret;
}

//yaogang modify
HddInfo *get_hddinfo_by_sn(const char *pdisksn)
{
	if(g_recordinit != 1)
	{
		return NULL;
	}
	
	int i = 0;
	for(i = 0; i < MAX_HDD_NUM; i++)
	{
		if((g_HddManager->hinfo[i].is_disk_exist) 
			&& (g_HddManager->hinfo[i].storage_type == 's')
			&& (strcmp(g_HddManager->hinfo[i].disk_sn, pdisksn) == 0))
		{
			//printf("%s i: %d\n", __func__, i);
			get_disk_info(g_HddManager, i);
			//printf("find it, i: %d\n", i);
			break;
		}
	}
	if(i < MAX_HDD_NUM)
	{
		return &g_HddManager->hinfo[i];
	}
	else
	{
		return NULL;
	}
}

s32 ModRecordDeInit()
{
	if(g_recordinit == 0)
	{
		fprintf(stderr, "error : ModRecordInit not done!\n");
		return EM_REC_HASNOTINIT;
	}
	
	if(g_recorddeinit == 1)
	{
		fprintf(stderr, "error : ModRecordDeInit has been done!\n");
		return EM_REC_HASBEENDEINIT;
	}
	
	g_recorddeinit = 1;
	g_recordinit = 0;
	
	if(g_DvrHasHdd == 0)
	{
		return EM_REC_SUCCESS;
	}
	
	usleep(500 * 1000);
	printf("Now, destroy buffer ... ...  [fun:%s]\n", __FUNCTION__);
	
	s32 i = 0;
	s32 ret = EM_REC_SUCCESS;

	ret = DestroyCircleBuf(&g_RecSnapBufInfo);
	if(EM_REC_SUCCESS != ret)
	{
		printf("%s RecordDestroyBuf Snap error!\n", __func__);
		return ret;
	}
		
	for(i = 0; i < g_ChnNum; i++)
	{
		ret = RecordDestroyBuf(i);
		if(EM_REC_SUCCESS != ret)
		{
			fprintf(stderr, "RecordDestroyBuf : chn[%d] error!\n", i);
			return ret;
		}
	}
	
	for(i = 0; i < 2; i++)
	{
		ret = RecordDestroyMsgQueue(i);
		if(EM_REC_SUCCESS != ret)
		{
			fprintf(stderr, "RecordDestroyMsgQueue : id[%d] error!\n", i);
			return ret;
		}
	}

	ret = RecordSnapDestroyMsgQueue(i);
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "RecordSnapDestroyMsgQueue error!\n");
		return ret;
	}
	
	//pre_record_snap_deinit();
	
	return ret;
}

s32 ModRecordStartManual(u8 chn)
{
	//printf("mod rec start %d !!!\n", chn);
	
	if(g_recordinit == 0)
	{
		printf("error : ModRecordInit not done!\n");
		return EM_REC_HASNOTINIT;
	}
	
	if(chn >= g_ChnNum)
	{
		printf("Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	if(g_DvrHasHdd == 0)
	{
		printf("no disk, rec not start!");
		return EM_REC_SUCCESS;
	}
	
	//printf("mod rec start %d !!!\n", chn);
	
	SModRecMsgHeader sMsg;
	memset(&sMsg, 0, sizeof(sMsg));
	
	sMsg.nChn = chn;
	sMsg.nMsg = EM_REC_V_MANUAL;
	sMsg.nKey = 1;
	sMsg.data = NULL;
	
	return RecordWriteMsgQueue(0, &sMsg);
}

s32 ModRecordStopManual(u8 chn)
{
	if(g_recordinit == 0)
	{
		fprintf(stderr, "error : ModRecordInit not done!\n");
		return EM_REC_HASNOTINIT;
	}
	
	if(chn >= g_ChnNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	if(g_DvrHasHdd == 0)
	{
		return EM_REC_SUCCESS;
	}

	SModRecMsgHeader sMsg;
	memset(&sMsg, 0, sizeof(sMsg));
	
	sMsg.nChn = chn;
	sMsg.nMsg = EM_REC_V_MANUAL;
	sMsg.nKey = 0;
	sMsg.data = NULL;
	
	return RecordWriteMsgQueue(0, &sMsg);
}

s32 ModRecordStop(u8 chn)
{
	if(g_recordinit == 0)
	{
		fprintf(stderr, "error : ModRecordInit not done!\n");
		return EM_REC_HASNOTINIT;
	}
	
	if(chn >= g_ChnNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	if(g_DvrHasHdd == 0)
	{
		return EM_REC_SUCCESS;
	}
	
	SModRecMsgHeader sMsg;
	memset(&sMsg, 0, sizeof(sMsg));
	
	sMsg.nChn = chn;
	sMsg.nMsg = EM_REC_STOPCHN;
	sMsg.data = NULL;
	
	return RecordWriteMsgQueue(0, &sMsg);
}

s32 ModRecordStopAll()
{
	s32 i = 0;
	s32 ret = 0;
	for(i = 0; i < g_ChnNum; i++)
	{
		ret = ModRecordStop(i);
		if(ret != EM_REC_SUCCESS)
		{
			break;
		}
	}
	return ret;
}

//yaogang modify 20150314
s32 ModRecordSnapPause(u8 cause)
{
	SSnapFxnMsg msg;
	s32 ret = 0;

	if (g_recordinit)
	{
		memset(&msg, 0, sizeof(SSnapFxnMsg));
		msg.cause = cause;
		msg.type = TypeRecSnapStop;

		printf("%s RecordSnapWriteMsgQueue\n", __func__);
		ret = RecordSnapWriteMsgQueue(&msg);
		if (ret)
		{
			printf("%s RecordSnapWriteMsgQueue failed\n", __func__);
		}
	}
	
	return ret;
}

s32 ModRecordSnapResume(u8 cause)
{
	SSnapFxnMsg msg;
	s32 ret = 0;;

	if (g_recordinit)
	{
		memset(&msg, 0, sizeof(SSnapFxnMsg));
		msg.cause = cause;
		msg.type = TypeRecSnapWillStart;

		printf("%s RecordSnapWriteMsgQueue\n", __func__);
		ret = RecordSnapWriteMsgQueue(&msg);
		if (ret)
		{
			printf("%s RecordSnapWriteMsgQueue failed\n", __func__);
		}
	}
	
	return ret;
}

s32 ModRecordPause(u8 chn)
{
	if(g_recordinit == 0)
	{
		fprintf(stderr, "error : ModRecordInit not done!\n");
		return EM_REC_HASNOTINIT;
	}
	
	if(chn >= g_ChnNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	if(g_DvrHasHdd == 0)
	{
		return EM_REC_SUCCESS;
	}
	
	SModRecMsgHeader sMsg;
	memset(&sMsg, 0, sizeof(sMsg));
	
	sMsg.nChn = chn;
	sMsg.nMsg = EM_REC_PAUSEREC;
	sMsg.data = NULL;
	
	#if 1//csp modify
	s32 ret = RecordWriteMsgQueue(0, &sMsg);
	if(ret != EM_REC_SUCCESS)
	{
		if(sMsg.data != NULL)
		{
			free(sMsg.data);
			sMsg.data = NULL;
		}
		return ret;
	}
	return EM_REC_SUCCESS;
	#else
	return RecordWriteMsgQueue(0, &sMsg);
	#endif
}

s32 ModRecordResume(u8 chn)
{
	if(g_recordinit == 0)
	{
		fprintf(stderr, "error : ModRecordInit not done!\n");
		return EM_REC_HASNOTINIT;
	}
	
	if(chn >= g_ChnNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	if(g_DvrHasHdd == 0)
	{
		return EM_REC_SUCCESS;
	}
	
	SModRecMsgHeader sMsg;
	memset(&sMsg, 0, sizeof(sMsg));
	
	sMsg.nChn = chn;
	sMsg.nMsg = EM_REC_RESUMEREC;
	sMsg.data = NULL;
	
	#if 1//csp modify
	s32 ret = RecordWriteMsgQueue(0, &sMsg);
	if(ret != EM_REC_SUCCESS)
	{
		if(sMsg.data != NULL)
		{
			free(sMsg.data);
			sMsg.data = NULL;
		}
		return ret;
	}
	return EM_REC_SUCCESS;
	#else
	return RecordWriteMsgQueue(0, &sMsg);
	#endif
}

s32 ModRecordSetPreTime(u8 chn, u32 time)
{
	if(g_recordinit == 0)
	{
		fprintf(stderr, "error : ModRecordInit not done!\n");
		return EM_REC_HASNOTINIT;
	}
	
	if(chn >= g_ChnNum)
	{
		fprintf(stderr,"Invalid parameter for function %s, g_ChnNum = %d!\n", __FUNCTION__,g_ChnNum);
		return EM_REC_INVALID_PARA;
	}
	
	if(g_DvrHasHdd == 0)
	{
		return EM_REC_SUCCESS;
	}

	SModRecMsgHeader sMsg;
	memset(&sMsg, 0, sizeof(sMsg));
	
	sMsg.nChn = chn;
	sMsg.nMsg = EM_REC_SETPRETIME;
	sMsg.data = malloc(sizeof(u32));
	if(sMsg.data == NULL)
	{
		fprintf(stderr,"Malloc failed for function %s !\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memcpy(sMsg.data, &time, sizeof(u32));
	
	#if 1//csp modify
	s32 ret = RecordWriteMsgQueue(0, &sMsg);
	if(ret != EM_REC_SUCCESS)
	{
		if(sMsg.data != NULL)
		{
			free(sMsg.data);
			sMsg.data = NULL;
		}
		return ret;
	}
	return EM_REC_SUCCESS;
	#else
	return RecordWriteMsgQueue(0, &sMsg);
	#endif
}

s32 ModRecordSetDelayTime(u8 chn, u32 time)
{
	if(g_recordinit == 0)
	{
		fprintf(stderr, "error : ModRecordInit not done!\n");
		return EM_REC_HASNOTINIT;
	}
	
	if(chn >= g_ChnNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	if(g_DvrHasHdd == 0)
	{
		return EM_REC_SUCCESS;
	}

	SModRecMsgHeader sMsg;
	memset(&sMsg, 0, sizeof(sMsg));
	
	sMsg.nChn = chn;
	sMsg.nMsg = EM_REC_SETDELAYTIME;
	sMsg.data = malloc(sizeof(u32));
	if(sMsg.data == NULL)
	{
		fprintf(stderr,"Malloc failed for function %s !\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memcpy(sMsg.data, &time, sizeof(u32));
	
	#if 1//csp modify
	s32 ret = RecordWriteMsgQueue(0, &sMsg);
	if(ret != EM_REC_SUCCESS)
	{
		if(sMsg.data != NULL)
		{
			free(sMsg.data);
			sMsg.data = NULL;
		}
		return ret;
	}
	return EM_REC_SUCCESS;
	#else
	return RecordWriteMsgQueue(0, &sMsg);
	#endif
}

s32 ModRecordSetRecChnTriggered(u8 chn, EMRECALARMTYPE emType, u64 data)
{
	if(g_recordinit == 0)
	{
		fprintf(stderr, "error : ModRecordInit not done!\n");
		return EM_REC_HASNOTINIT;
	}
	
	if(chn >= g_ChnNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	if(g_DvrHasHdd == 0)
	{
		return EM_REC_SUCCESS;
	}
	
	SModRecMsgHeader sMsg;
	memset(&sMsg, 0, sizeof(sMsg));
	
	sMsg.nChn = chn;
	sMsg.nMsg = emType + 10;
	sMsg.data = malloc(sizeof(u64));
	if(sMsg.data == NULL)
	{
		fprintf(stderr,"Malloc failed for function %s !\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memcpy(sMsg.data, &data, sizeof(u64));
	
	#if 1//csp modify
	s32 ret = RecordWriteMsgQueue(0, &sMsg);
	if(ret != EM_REC_SUCCESS)
	{
		if(sMsg.data != NULL)
		{
			free(sMsg.data);
			sMsg.data = NULL;
		}
		return ret;
	}
	return EM_REC_SUCCESS;
	#else
	return RecordWriteMsgQueue(0, &sMsg);
	#endif
}

s32 ModRecordSetTrigger(u8 chn, EMRECALARMTYPE emType, u8 key)
{
	if(g_recordinit == 0)
	{
		fprintf(stderr, "error : ModRecordInit not done!\n");
		return EM_REC_HASNOTINIT;
	}
	
	if(chn >= g_ChnNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	if(g_DvrHasHdd == 0)
	{
		return EM_REC_SUCCESS;
	}
	
	SModRecMsgHeader sMsg;
	memset(&sMsg, 0, sizeof(sMsg));
	
	sMsg.nChn = chn;
	sMsg.nMsg = emType;
	sMsg.nKey = ((key == 0) ? 0 : 1);
	sMsg.data = NULL;
	
	#if 1//csp modify
	s32 ret = RecordWriteMsgQueue(0, &sMsg);
	if(ret != EM_REC_SUCCESS)
	{
		if(sMsg.data != NULL)
		{
			free(sMsg.data);
			sMsg.data = NULL;
		}
		return ret;
	}
	return EM_REC_SUCCESS;
	#else
	return RecordWriteMsgQueue(0, &sMsg);
	#endif
}

s32 ModRecordSetSchedulePara(u8 chn, SModRecSchPara* para)
{
	if(g_recordinit == 0)
	{
		fprintf(stderr, "error : ModRecordInit not done!\n");
		return EM_REC_HASNOTINIT;
	}
	
	if((chn >= g_ChnNum) || (NULL == para) || (chn != para->nChn))
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	if(g_DvrHasHdd == 0)
	{
		return EM_REC_SUCCESS;
	}
	
	SModRecMsgHeader sMsg;
	memset(&sMsg, 0, sizeof(sMsg));
	
	sMsg.nChn = chn;
	sMsg.nMsg = EM_REC_SETSCHPARA;
	sMsg.data = malloc(sizeof(SModRecSchPara));
	if(sMsg.data == NULL)
	{
		fprintf(stderr,"Malloc failed for function %s !\n", __FUNCTION__);
		return EM_REC_MALLOC_ERR;
	}
	memcpy(sMsg.data, para, sizeof(SModRecSchPara));
	//sMsg.data = (void *)para;
	
	#if 1//csp modify
	s32 ret = RecordWriteMsgQueue(0, &sMsg);
	if(ret != EM_REC_SUCCESS)
	{
		if(sMsg.data != NULL)
		{
			free(sMsg.data);
			sMsg.data = NULL;
		}
		return ret;
	}
	return EM_REC_SUCCESS;
	#else
	return RecordWriteMsgQueue(0, &sMsg);
	#endif
}


//帧长度+ sizeof(SModRecRecordHeader) 必须小于 (640<<10)，原因RecordMainFxn()中frame_data[640<<10]
//#define TEST_FRAME_LEN (640<<10)
#undef TEST_FRAME_LEN
s32 ModRecordWriteOneFrame(u8 chn, SModRecRecordHeader* header)
{
	s32 ret;
	//printf("ModRecordWriteOneFrame-1\n");
	
	if(g_recordinit == 0)
	{
		fprintf(stderr, "error : ModRecordInit not done!\n");
		return EM_REC_HASNOTINIT;
	}
	
	if((chn >= g_ChnNum) || (NULL == header) || (chn != header->nChn))
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}
	
	if(g_DvrHasHdd == 0)
	{
		return EM_REC_SUCCESS;
	}
	
	#if 1
	//unsigned char cur_atx_flag = tl_power_atx_check();
	//if(cur_atx_flag == 0 || g_HotPlugFlag)//电池供电
	if(g_HotPlugFlag)
	{
		RecordResetBuf(chn);
		return EM_REC_SUCCESS;
	}
	#endif
	
	//printf("ModRecordWriteOneFrame-2\n");
	
	#ifdef TEST_FRAME_LEN
	static u8 *pdata = NULL;
	if (NULL == pdata)
	{
		pdata = malloc(TEST_FRAME_LEN);
		if (NULL == pdata)
		{
			printf("%s malloc failed\n", __func__);
			return -1;
		}
	}
	//chn == 0 && Iframe
	if (header->nChn == 0 && header->nFrameType)
	{
		printf("%s chn%d Iframe modify\n", __func__, header->nChn);
		memset(pdata, 0, TEST_FRAME_LEN);
		memcpy(pdata, header->nDate, header->nDataLength);
		header->nDate = pdata;
		header->nDataLength = TEST_FRAME_LEN;
	}
	#endif
	
	ret = RecordWriteFrameToBuf(header);

	//printf("ModRecordWriteOneFrame-3\n");

	return ret;
}

void ModRecordSetDealHardDiskFull(u8 nCover)
{
	if(nCover != g_HddIsFullCover)
	{
		g_HddIsFullCover = (nCover?1:0);
		set_policy_when_disk_full(g_HddIsFullCover);
	}
}

void ModRecordSetWorkingEnable(u8 nEnable)
{
	g_IsModWorking = nEnable ? 1 : 0;
}

s32 ModRecordBufRelease(void)
{
	if(g_DvrHasHdd == 0)
	{
		return EM_REC_SUCCESS;
	}
	
	usleep(100*1000);//csp modify
	
	printf("Now, destroy buffer ... ...  [fun:%s]\n", __FUNCTION__);
	
	s32 ret = EM_REC_SUCCESS;
	ret = DestroyCircleBuf(&g_RecSnapBufInfo);
	if(EM_REC_SUCCESS != ret)
	{
		printf("%s RecordDestroyBuf Snap error!\n", __func__);
		return ret;
	}
	
	s32 i = 0;
	for(i = 0; i < g_ChnNum; i++)
	{
		#if 1//csp modify
		int j = 0;
		for(j=0;j<10;j++)
		{
			if(g_ChnPaused[i])
			{
				break;
			}
			usleep(100*1000);
		}
		if(j>=10)
		{
			continue;
		}
		#else
		if(!g_ChnPaused[i])
		{
			continue;
		}
		#endif
		ret = RecordDestroyBuf(i);
		if(EM_REC_SUCCESS != ret)
		{
			fprintf(stderr, "RecordDestroyBuf : chn[%d] error!\n", i);
			return ret;
		}
		g_RecordChnReleased[i] = 1;
	}
	return ret;
}

s32 ModRecordBufResume(void)
{
	if(g_DvrHasHdd == 0)
	{
		return EM_REC_SUCCESS;
	}
	printf("Now, resume buffer ... ...  [fun:%s]\n", __FUNCTION__);
	s32 i = 0;
	s32 ret = EM_REC_SUCCESS;
	for(i = 0; i < g_ChnNum; i++)
	{
		if(!g_RecordChnReleased[i])
		{
			continue;
		}
		ret = RecordCreateBuf(i, g_RecChnBufLength[i]);
		if(EM_REC_SUCCESS != ret)
		{
			fprintf(stderr, "RecordCreateBuf : chn[%d] error!\n", i);
			return ret;
		}
		g_RecordChnReleased[i] = 0;
	}
	return ret;
}

static int g_tz_offset = 8*3600;

s32 ModRecordSetTimeZoneOffset(int nOffset)
{
	g_tz_offset = nOffset;
	return g_tz_offset;
}

s32 ModRecordGetTimeZoneOffset()
{
	return g_tz_offset;
}

//yaogang modify 20141225
int ModRecordSnapshotToBuf(u8 chn, u8 pic_type, time_t tv_sec, suseconds_t tv_usec, void *pdata, u32 data_size, u32 width, u32 height)
{
	SModRecSnapHeader snap_header;
	s32 ret;
	//printf("\n\n%s chn: %d, pic_type: 0x%x\n", __func__, chn, pic_type);
	//csp modify
	if(g_recordinit == 0)
	{
		fprintf(stderr, "error : ModRecordInit not done!\n");
		return EM_REC_HASNOTINIT;
	}
	
	if(chn >= g_ChnNum)
	{
		fprintf(stderr,"Invalid parameter for function %s !\n", __FUNCTION__);
		return EM_REC_INVALID_PARA;
	}

	if (data_size > SnapSize)
	{
		printf("%s error data_size: %d > SnapSize\n", __func__, data_size);
		return EM_REC_INVALID_PARA;
	}
	
	if(g_DvrHasHdd == 0)
	{
		printf("%s g_DvrHasHdd == 0\n", __func__);
		return EM_REC_SUCCESS;
	}
	
	//csp modify
	if(g_IsModWorking == 0)
	{
		printf("%s g_IsModWorking == 0\n", __func__);
		return EM_REC_SUCCESS;
	}

	if(g_HotPlugFlag)
	{
		RecordResetSnapBuf();
		return EM_REC_SUCCESS;
	}

	snap_header.chn = chn;
	snap_header.pic_type = pic_type;
	snap_header.data_size = data_size;
	snap_header.width = width;
	snap_header.height = height;
	snap_header.tv_sec = tv_sec;
	snap_header.tv_usec = tv_usec;

	pthread_mutex_lock(&LockSnapBuf);
	
	ret = WriteDataToBuf(&g_RecSnapBufInfo, (u8 *)&snap_header, sizeof(SModRecSnapHeader));
	if(EM_REC_SUCCESS != ret)
	{
		printf("%s chn%d header WriteDataToBuf failed\n", __func__, chn);
		pthread_cond_broadcast(&CondSnapBuf); 
		
		pthread_mutex_unlock(&LockSnapBuf);	
		
		return ret;
	}
	
	ret = WriteDataToBuf(&g_RecSnapBufInfo, (u8 *)pdata, data_size);
	if(EM_REC_SUCCESS != ret)
	{
		printf("%s chn%d data WriteDataToBuf failed\n", __func__, chn);
		
		ResumeCircleBufToPast(&g_RecSnapBufInfo, 1, 0);
		pthread_cond_broadcast(&CondSnapBuf); 
		
		pthread_mutex_unlock(&LockSnapBuf);
		
		return ret;
	}
	
	BufSnapCnt++; //计数+1，Buffer中Snap的数量。
	pthread_cond_broadcast(&CondSnapBuf); 
	
	pthread_mutex_unlock(&LockSnapBuf);

	#if 0
	if (chn == 0)
		printf("%s chn%d, pic_type: 0x%x, data_size: %u\n", __func__, chn, pic_type, data_size);
	#endif
	
	return ret;
}

//预录
int ModRecordPreSnapFromFile(u8 chn, time_t tv_sec, void *pdata, u32 * pdata_size)
{
	FILE *pf = NULL;
	int ret;
	u8 in, out, total, i;
	prepic_use_info *pPreRecSnapInfo = NULL;
	long offset;

	if (NULL == pdata)
	{
		printf("%s pdata == NULL\n", __func__);
		return 1;
	}
	if (NULL == pdata_size)
	{
		printf("%s pdata_size == NULL\n", __func__);
		return 1;
	}
	
	if (chn < 16)
		pf = pf_PreRecSnap[0];
	else
		pf = pf_PreRecSnap[1];
	
	if (pf == NULL)
	{
		pre_record_snap_deinit();
		//预录初始化
		pre_record_snap_init(g_ChnNum);
	}
	
	if (chn < 16)
		pf = pf_PreRecSnap[0];
	else
		pf = pf_PreRecSnap[1];
	
	if (pf == NULL)
	{
		printf("%s not found hdd\n", __func__);
		return 1;
	}
	
	//printf("%s read chn%d, time: %d\n", __func__, chn, tv_sec);
	
	sem_wait(&p_PreRecSnapChnCtl[chn].sem);

	in = p_PreRecSnapChnCtl[chn].in;
	out = p_PreRecSnapChnCtl[chn].out;
	total = p_PreRecSnapChnCtl[chn].total;

	if (in == out)
	{
		printf("%s PreRecord space empty\n", __func__);
		sem_post(&p_PreRecSnapChnCtl[chn].sem);
		return 1;
	}
	
	pPreRecSnapInfo = (prepic_use_info *)pdata;//暂用

	offset = FILE_VECTOR_SIZE/16*chn;
	
	ret = fseek(pf, offset, SEEK_SET);
	if (ret)
	{
		printf("%s fseek 1 failed, %s\n", __func__, strerror(errno));
		sem_post(&p_PreRecSnapChnCtl[chn].sem);
		return 1;
	}
	
	ret = fread(pPreRecSnapInfo, sizeof(prepic_use_info), PreRecSnapNumPerChn, pf);
	if (ret != PreRecSnapNumPerChn)
	{
		printf("%s fread prepic_use_info failed, %s\n", __func__, strerror(errno));
		sem_post(&p_PreRecSnapChnCtl[chn].sem);
		return 1;
	}

	u8 end;//指向最后一张图片
	end = (in==0) ? total-1 : in-1; 

	//是否超出预录的时间范围
	if ( (tv_sec < pPreRecSnapInfo[out].tv_sec) || (tv_sec > pPreRecSnapInfo[end].tv_sec) )
	{
		//printf("%s get Snap time %d out of rang[%d - %d]\n", __func__, tv_sec, 
		//		pPreRecSnapInfo[out].tv_sec, pPreRecSnapInfo[end].tv_sec);
		sem_post(&p_PreRecSnapChnCtl[chn].sem);
		return 1;
	}

	//查找时间对应的索引
	for (i=out; i != in; i=(i+1)%total)
	{
		//printf("%s, %d, %d\n", __func__, tv_sec, pPreRecSnapInfo[i].tv_sec);
		if (pPreRecSnapInfo[i].tv_sec == tv_sec)
			break;
	}
	if (i == in)
	{
		printf("%s can not found\n", __func__);
		sem_post(&p_PreRecSnapChnCtl[chn].sem);
		return 1;
	}

	if (*pdata_size < pPreRecSnapInfo[i].pic_size)
	{
		printf("%s buffer too small\n", __func__);
		*pdata_size = 0;
		sem_post(&p_PreRecSnapChnCtl[chn].sem);
		return 1;
	}

	*pdata_size = pPreRecSnapInfo[i].pic_size;
	
	//读Snap
	offset = FILE_VECTOR_SIZE/16*chn \
			+ sizeof(prepic_use_info) * PreRecSnapNumPerChn \
			+ SnapSize * i;
	ret = fseek(pf, offset, SEEK_SET);
	if (ret)
	{
		printf("%s fseek 2 failed, %s\n", __func__, strerror(errno));
		sem_post(&p_PreRecSnapChnCtl[chn].sem);
		return 1;
	}

	ret = fread(pdata, 1, *pdata_size, pf);
	if (ret != *pdata_size)
	{
		printf("%s fread pdata failed, %s\n", __func__, strerror(errno));
		sem_post(&p_PreRecSnapChnCtl[chn].sem);
		return 1;
	}

	//printf("%s ok\n", __func__);
	/****************************************************************/
	
	sem_post(&p_PreRecSnapChnCtl[chn].sem);

	return 0;	
}

int ModRecordPreSnapToFile(u8 chn, time_t tv_sec, void *pdata, u32 data_size, u32 width, u32 height)
{
	FILE *pf = NULL;
	int ret;
	u8 in, out, total;
	prepic_use_info pic_info;

	//printf("%s chn:%d time:%d\n", __func__, chn, tv_sec);
	
	if (chn < 16)
		pf = pf_PreRecSnap[0];
	else
		pf = pf_PreRecSnap[1];
	
	if (pf == NULL)
	{
		printf("%s pf == NULL, Uninitialized? ? ?\n", __func__);
		return 1;
	}

	if (data_size >= SnapSize)
	{
		printf("%s snap size : %d too big\n", __func__, data_size);
		return 1;
	}
	
	//printf("%s write chn%d, size: %d, time: %d\n", __func__, chn, data_size, tv_sec);
	
	sem_wait(&p_PreRecSnapChnCtl[chn].sem);

	in = p_PreRecSnapChnCtl[chn].in;
	out = p_PreRecSnapChnCtl[chn].out;
	total = p_PreRecSnapChnCtl[chn].total;

	//printf("%s chn%d, in:%d, out: %d, total: %d\n", __func__, chn, in, out, total);
	
	long offset = FILE_VECTOR_SIZE/16*chn \
		+ sizeof(prepic_use_info) * PreRecSnapNumPerChn \
		+ SnapSize * in;
	ret = fseek(pf, offset, SEEK_SET);
	if (ret)
	{
		printf("%s fseek 1 failed, %s\n", __func__, strerror(errno));
		sem_post(&p_PreRecSnapChnCtl[chn].sem);
		return 1;
	}

	ret = fwrite(pdata, 1, data_size, pf);
	if (ret != data_size)
	{
		printf("%s fwrite 1 failed, %s, ret: %d\n", __func__, strerror(errno), ret);
		sem_post(&p_PreRecSnapChnCtl[chn].sem);
		return 1;
	}

	//环形缓冲满 判断
	in = (in + 1) % total;
	if (in == out)
	{
		p_PreRecSnapChnCtl[chn].out = (p_PreRecSnapChnCtl[chn].out+1)%total;
	}
	

	//更行索引头
	offset = FILE_VECTOR_SIZE/16*chn \
		+ sizeof(prepic_use_info) * p_PreRecSnapChnCtl[chn].in;
	
	ret = fseek(pf, offset, SEEK_SET);
	if (ret)
	{
		printf("%s fseek 2 failed, %s\n", __func__, strerror(errno));
		sem_post(&p_PreRecSnapChnCtl[chn].sem);
		return 1;
	}
	
	memset(&pic_info, 0, sizeof(pic_info));
	pic_info.width = width;
	pic_info.height = height;
	pic_info.pic_size = data_size;
	pic_info.tv_sec = tv_sec;
	
	ret = fwrite(&pic_info, 1, sizeof(pic_info), pf);
	if (ret != sizeof(pic_info))
	{
		printf("%s fwrite 2 failed, %s, ret: %d\n", __func__, strerror(errno), ret);
		sem_post(&p_PreRecSnapChnCtl[chn].sem);
		return 1;
	}

	p_PreRecSnapChnCtl[chn].in = in;
	fileflush(pf);
	
	sem_post(&p_PreRecSnapChnCtl[chn].sem);

	return 0;	
}

//yaogang modify 20150313
s32 RecordSnapCreateMsgQueue(u32 length)
{
	s32 ret = EM_REC_SUCCESS;
	
	g_SnapMsgQueueInfo.nBufId = 0;	
	g_SnapMsgQueueInfo.nLength = length;
		
	ret = CreateCircleBuf(&g_SnapMsgQueueInfo);
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
	}
	
	return ret;
}

s32 RecordSnapDestroyMsgQueue()
{
	s32 ret = EM_REC_SUCCESS;
	
	ret = DestroyCircleBuf(&g_SnapMsgQueueInfo);
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
	}
	
	return ret;
}

s32 RecordSnapWriteMsgQueue(SSnapFxnMsg *pmsg)
{
	s32 ret = EM_REC_SUCCESS;
	
	ret = WriteDataToBuf(&g_SnapMsgQueueInfo, (u8 *)pmsg, sizeof(SSnapFxnMsg));
	if(EM_REC_SUCCESS != ret)
	{
		fprintf(stderr, "function: %s\n", __FUNCTION__);
	}
	
	return ret;
}

s32 RecordSnapReadMsgQueue(SSnapFxnMsg *pmsg)
{
	s32 ret = EM_REC_SUCCESS;
	
	ret = ReadDataFromBuf(&g_SnapMsgQueueInfo, (u8 *)pmsg, sizeof(SSnapFxnMsg));
	if(EM_REC_SUCCESS != ret)
	{
		//fprintf(stderr, "function: %s\n", __FUNCTION__);
	}
	
	return ret;
}


