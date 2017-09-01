#include <assert.h>
#include <semaphore.h>
#include <stdio.h>

#include "utils.h"
#include "lib_misc.h"
#include "lib_vdec.h"
#include "lib_preview.h"
#include "lib_venc.h"
#include "lib_audio.h"
#include "custommp4.h"
#include "diskmanage.h"
#include "partitionindex.h"
#include "mod_playback.h"

#define MAX_SEARCH_NUM	4000

#define JUMP_TIME		(1*1200)//(5*1000)//csp modify 20130415
#define PLAY_TIME		(5*1000)//(5*1000)//csp modify 20130415

typedef enum
{
	EM_CTL_STATE_IDLE=0,
	EM_CTL_STATE_PAUSE,
	EM_CTL_STATE_RUN,
}EmCtlState;

typedef enum
{
	EM_CHN_STATE_IDLE=0,
	EM_CHN_STATE_WAIT,
	EM_CHN_STATE_PLAY,
}EmChnState;

typedef enum
{
	EM_CHN_NULL=0,
	EM_CHN_STOP,
	EM_CHN_PLAY,
	EM_CHN_PAUSE,
	EM_CHN_RESUME,
	EM_CHN_STEP,
	EM_CHN_SEEK,
	EM_CHN_UPDATEPTS,
	EM_CHN_BACKWARD,
	EM_CHN_FORWARD,
}EmPBChnCmd;

typedef union
{
	SPBSearchPara sSearchParam;
	SPBRecfileInfo sFileInfo;
	u32 nSeekTime;
	EmPlayRate emPlayRate;
	u8 nMute;
}UnCtlCxt;

typedef union
{
	u64 nRefPts;
	u32 nSeekTime;
}UnChnCxt;

typedef struct 
{
	u32 emCmd;
	UnCtlCxt unCxt;
}SCtlMSG;

typedef struct 
{
	u32 emCmd;
	UnChnCxt unCxt;
}SChnMSG;

typedef struct
{
	s16 nForward;
	s16 nMediaFrameReady;
	s32 nChnNo;
	s32 nPlayNo;
	s32 nRealFileNums;
	u8  nFlagZoom;		//单通道放大标志,
						//2:多通道回放模式,
						//0:单通道模式且该通道没有被ZOOM, 
						//1:单通道模式且该通道被ZOOM
	u8 nMute;
	u32 nFrameStepTime; //每帧间的间隔
	u8 nIsPlaying;		//该通道是否正在回放(有数据正在解码)
	u8 nFlagDoSeek;		//刚刚做了seek操作,需要读个关键帧
	SPBRecfileInfo aRecFileInfos[MAX_SEARCH_NUM];
	custommp4_t* pFile;
	s32 nCurIndex;
	s64 nCurFrameStamp;
	EmPlayRate emRate;
	EmChnState emCurChnState;
	EmPBChnCmd emPbChnCmd;
	UnChnCxt unPbChnCxt;
	s64 nPbChnAck;
	void* pManagerParent;
	sem_t semPbChnFinish;
	sem_t semSendChnFinish;
	
	//csp modify
	u32  nVWidth;
	u32  nVHeight;
}SPlaybackChnInfo;

//#define DEC_USE_ONE_SAME_BUF

typedef struct SPlayBackManger
{
	s16  nForward;
	u32  nTotalTime;
	u64  nRefPTS;
	u32  nTimeInterval;
	u32  nSrchStartTime;
	u32  nSrchEndTime;
	u32  nRecStartTime;
	u32  nRecEndTime;
	
	//csp modify
	//u32  nVWidth;
	//u32  nVHeight;
	
	u32  nAllSrchFileNum;
	u32  nRealPlayNum;
	u32  nMaxChnNum;
	u64  nPlayingChnsMask;		//暂时只支持到64路产品,实际有文件正在播放的通道
	//u8   nZoomChn;				//单通道放大的chn
	//u8* pValidPlayChn;			//实际有效地回放通道
	//u8   nFlagUpdateValid;		//更新上个参数的标志
	u32  nStepBaseTime;			//帧放时每次前进时间，默认40 ms
	u8   nSeekPreSectFlag;
	u8   nIsVOIP;
	//u64  nRefPtsAfterSeek;		//调用Seek后，需要更新nRefPTS,否则会有卡顿一下的现象
	EmPlayRate emRate;
	EmCtlState emCurCtlState;
	EmPBCtlCmd emPbCtlCmd;
	UnCtlCxt unPbCtlCxt;
	u32 nPbCtlAck;
	SPlaybackChnInfo* aPlaybackChnInfos;
	#ifdef DEC_USE_ONE_SAME_BUF
	s8* pDecBuf;
	sem_t semDecBuf;
	#endif
	u32 nMaxFrameSize;
	u64 nChnMaskOfD1;
	sem_t semPbCtlFinish;
	//sem_t semSendCtlFinish;
	pthread_mutex_t lock;//csp modify
}SPlayBackManager;

disk_manager* gp_sHddManager = NULL;
PBPROGFUNC pbProgCB = NULL;
PBREGFUNCCB pbStopElecZoom = NULL;

//SPlayBackManager g_sPlayBackManager;

//#define TLDEBUG
#ifdef TLDEBUG
#define DEBUG(fmt, args...) do { printf(fmt, ## args); } while(0)
#else
#define DEBUG(fmt, args...)
#endif

s32 SendCtlMsg(SPlayBackManager* pSPbMgr,SCtlMSG* pMsg,u32* ack)
{
	if(!pSPbMgr || !pMsg)
	{
		return -1;
	}
	//printf("%s cmd: %d, cxt: %d\n", __func__, pMsg->emCmd, pMsg->unCxt);
	
	pthread_mutex_lock(&pSPbMgr->lock);//csp modify
	
	//csp modify:先修改参数再修改命令
	pSPbMgr->unPbCtlCxt = pMsg->unCxt;
	pSPbMgr->emPbCtlCmd = pMsg->emCmd;
	
	//sem_post(&pSPbMgr->semSendCtlFinish);
	sem_wait(&pSPbMgr->semPbCtlFinish);
	
	if(ack != NULL)//csp modify
	{
		*ack = pSPbMgr->nPbCtlAck;
	}
	
	pthread_mutex_unlock(&pSPbMgr->lock);//csp modify
	
	return 0;
}

s32 GetCtlMsg(SPlayBackManager* pSPbMgr,SCtlMSG* pMsg)
{
	if(!pSPbMgr || !pMsg)
	{
		return -1;
	}
	
	//sem_wait(&pSPbMgr->semSendCtlFinish);
	
	pMsg->emCmd = pSPbMgr->emPbCtlCmd;
	pMsg->unCxt = pSPbMgr->unPbCtlCxt;
	
	return 0;
}

s32 AckCtlMsg(SPlayBackManager* pSPbMgr)
{
	if(!pSPbMgr)
	{
		return -1;
	}
	
	pSPbMgr->emPbCtlCmd = EM_CTL_NULL;
	
	sem_post(&pSPbMgr->semPbCtlFinish);
	
	return 0;
}

s32 SendChnMsg(SPlaybackChnInfo* pSPbchn,SChnMSG* pMsg,u64* ack)
{
	if(!pSPbchn || !pMsg)
	{
		return -1;
	}
	
	//csp modify:先修改参数再修改命令
	pSPbchn->unPbChnCxt = pMsg->unCxt;
	pSPbchn->emPbChnCmd = pMsg->emCmd;
	
	sem_post(&pSPbchn->semSendChnFinish);
	
	sem_wait(&pSPbchn->semPbChnFinish);
	
	*ack = pSPbchn->nPbChnAck;
	
	return 0;
}

s32 GetChnMsg(SPlaybackChnInfo* pSPbchn,SChnMSG* pMsg, u8 nWaitType)
{
	if(!pSPbchn || !pMsg)
	{
		return -1;
	}
	
	s32 nRet = 0;
	
	if(nWaitType)
		nRet = sem_wait(&pSPbchn->semSendChnFinish);//csp modify
	else
		nRet = sem_trywait(&pSPbchn->semSendChnFinish);
	
	pMsg->emCmd = pSPbchn->emPbChnCmd;
	pMsg->unCxt = pSPbchn->unPbChnCxt;
	
	return nRet;
}

s32 AckChnMsg(SPlaybackChnInfo* pSPbchn)
{
	pSPbchn->emPbChnCmd = EM_CHN_NULL;
	
	sem_post(&pSPbchn->semPbChnFinish);
	
	return 0;
}

BOOL IsIframe(u8 nVideoType)
{
	if(0 == nVideoType)//P
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

static u64 GetTimeStamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	
#if 1//csp modify 20130408
	//return (u64)(((((u64)(tv.tv_sec))*1000000) + (u64)tv.tv_usec)/1000);
	return ((u64)tv.tv_sec*1000 + (u64)tv.tv_usec/1000);
#else
	//return tv.tv_sec*1000 + (tv.tv_usec)/1000;
	return (u64)(((((u64)(tv.tv_sec))*1000000) + (u64)tv.tv_usec)/1000);
#endif
}

s32 InsertFileInfo(SPlayBackManager* pSPbMgr,SPBRecfileInfo* pFileInfo)
{
	if((!pSPbMgr) || (!pFileInfo))
	{
		return -1;
	}
	
	u8 i = 0;
	for(i = 0; i < pSPbMgr->nMaxChnNum; i++)
	{
		pSPbMgr->aPlaybackChnInfos[i].nRealFileNums = 0;
		pSPbMgr->aPlaybackChnInfos[i].nFlagZoom = 0;
		pSPbMgr->aPlaybackChnInfos[i].nPlayNo = -1;
	}
	
	pSPbMgr->nSrchStartTime = pFileInfo->nStartTime;
	pSPbMgr->nSrchEndTime = pFileInfo->nEndTime;
	pSPbMgr->nAllSrchFileNum = 1;
	pSPbMgr->aPlaybackChnInfos[pFileInfo->nChn].nRealFileNums = 1;
	pSPbMgr->aPlaybackChnInfos[pFileInfo->nChn].nCurIndex = 0;//csp modify
	memcpy(pSPbMgr->aPlaybackChnInfos[pFileInfo->nChn].aRecFileInfos, pFileInfo, sizeof(SPBRecfileInfo));
	pSPbMgr->aPlaybackChnInfos[pFileInfo->nChn].nChnNo = pFileInfo->nChn;
	pSPbMgr->aPlaybackChnInfos[pFileInfo->nChn].nPlayNo = 0;
	pSPbMgr->aPlaybackChnInfos[pFileInfo->nChn].nFlagZoom = 1;
	pSPbMgr->nRealPlayNum = 1;
	pSPbMgr->nRecStartTime = pFileInfo->nStartTime;
	pSPbMgr->nRecEndTime = pFileInfo->nEndTime;
	
	//printf("chn : %d, realfilenum: %d\n", pFileInfo->nChn, pSPbMgr->aPlaybackChnInfos[pFileInfo->nChn].nRealFileNums);
	
	return 0;
}

s32 SearchRecFiles(SPlayBackManager* pSPbMgr,SPBSearchPara* pSearchParam)
{
	if(!pSPbMgr)
	{
		return -1;
	}
	
	u16 nPlayIndex = 0;
	u16 nRealPlayIndex = 0;
	//u32 nRealFilenums = 0;//csp modify
	u32 i = 0;
	
	search_param_t sSearch;
	memset(&sSearch,0,sizeof(search_param_t));
	
	pSPbMgr->nSrchStartTime = pSearchParam->nStartTime;
	pSPbMgr->nSrchEndTime = pSearchParam->nEndTime;
	
	DEBUG("SearchRecFiles:searchStartTime = %d\n",pSPbMgr->nSrchStartTime);
	DEBUG("SearchRecFiles:searchEndTime = %d\n",pSPbMgr->nSrchEndTime);
	
	pSPbMgr->nAllSrchFileNum = 0;
	
#if 1//yzw modify 缩短搜索时间
	sSearch.channel_no = pSearchParam->nMaskChn;
	sSearch.play_no = nPlayIndex;
	sSearch.start_time = pSearchParam->nStartTime;
	sSearch.end_time = pSearchParam->nEndTime;
	sSearch.type = pSearchParam->nMaskType;
	
	SPBRecfileInfo nRecFileInfos[MAX_SEARCH_NUM];
	memset(nRecFileInfos, 0, sizeof(SPBRecfileInfo)*MAX_SEARCH_NUM);
	//printf("SearchRecFiles:sizeof(SPBRecfileInfo)=%d,sizeof(recfileinfo_t)=%d\n",sizeof(SPBRecfileInfo),sizeof(recfileinfo_t));
	
	int nFileNum = search_all_rec_file(gp_sHddManager,
										&sSearch,
										(recfileinfo_t*)nRecFileInfos,
										MAX_SEARCH_NUM);
	//printf("all file num: %d\n", nFileNum);
	
	if(nFileNum > MAX_SEARCH_NUM) 
	{
		pSPbMgr->nAllSrchFileNum = 0;
	}
	else if(nFileNum < 0)
	{
		pSPbMgr->nAllSrchFileNum = MAX_SEARCH_NUM;
	}
	else
	{
		pSPbMgr->nAllSrchFileNum = nFileNum;
	}
	
	for(i=0; i<pSPbMgr->nMaxChnNum; i++)
	{
		pSPbMgr->aPlaybackChnInfos[i].nRealFileNums = 0;
		pSPbMgr->aPlaybackChnInfos[i].nFlagZoom = 2;
		pSPbMgr->aPlaybackChnInfos[i].nPlayNo = -1;
		
		if((1<<i)&(pSearchParam->nMaskChn))
		{
			pSPbMgr->aPlaybackChnInfos[i].nChnNo = i;
			pSPbMgr->aPlaybackChnInfos[i].nPlayNo = nPlayIndex;
			nPlayIndex++;
		}
	}
	for(i = 0; i < pSPbMgr->nAllSrchFileNum; i++)
	{
		//printf("nRecFileInfos[%d].nChn = %d, nMaxChnNum = %d\n", i, nRecFileInfos[i].nChn, pSPbMgr->nMaxChnNum);
		if(nRecFileInfos[i].nChn <= pSPbMgr->nMaxChnNum)
		{
			memcpy(&pSPbMgr->aPlaybackChnInfos[nRecFileInfos[i].nChn-1].aRecFileInfos[pSPbMgr->aPlaybackChnInfos[nRecFileInfos[i].nChn-1].nRealFileNums], &nRecFileInfos[i], sizeof(SPBRecfileInfo));
			pSPbMgr->aPlaybackChnInfos[nRecFileInfos[i].nChn-1].nRealFileNums++;
		}
	}
	
	for(i=0; i<pSPbMgr->nMaxChnNum; i++)
	{
		//printf("chn[%d] file num: %d\n", i, pSPbMgr->aPlaybackChnInfos[i].nRealFileNums);
		if(pSPbMgr->aPlaybackChnInfos[i].nRealFileNums != 0)
		{
			nRealPlayIndex++;
			DEBUG("SearchRecFiles:% chn has %d files\n",i,pSPbMgr->aPlaybackChnInfos[i].nRealFileNums);
		}
	}
#else
	for(i=0; i<pSPbMgr->nMaxChnNum; i++)
	{
		pSPbMgr->aPlaybackChnInfos[i].nRealFileNums = 0;
		pSPbMgr->aPlaybackChnInfos[i].nFlagZoom = 2;
		pSPbMgr->aPlaybackChnInfos[i].nPlayNo = -1;
					
		if((1<<i)&(pSearchParam->nMaskChn))
		{
			sSearch.channel_no = 0;
			sSearch.channel_no |= (1<<(i));
			sSearch.play_no = nPlayIndex;
			sSearch.start_time = pSearchParam->nStartTime;
			sSearch.end_time = pSearchParam->nEndTime;
			sSearch.type = pSearchParam->nMaskType;
			
			pSPbMgr->aPlaybackChnInfos[i].nRealFileNums 
				= search_all_rec_file(gp_sHddManager,
										&sSearch,
										(recfileinfo_t*)pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos,
										MAX_SEARCH_NUM);
			
			if(pSPbMgr->aPlaybackChnInfos[i].nRealFileNums > MAX_SEARCH_NUM) 
			{
				pSPbMgr->aPlaybackChnInfos[i].nRealFileNums = 0;
			}
			else if(pSPbMgr->aPlaybackChnInfos[i].nRealFileNums < 0)
			{
				pSPbMgr->aPlaybackChnInfos[i].nRealFileNums = MAX_SEARCH_NUM;
			}

			pSPbMgr->nAllSrchFileNum 
				+= pSPbMgr->aPlaybackChnInfos[i].nRealFileNums;

			pSPbMgr->aPlaybackChnInfos[i].nChnNo = i;
			pSPbMgr->aPlaybackChnInfos[i].nPlayNo = nPlayIndex;
			
			//nSearchIndex++;
			if(pSPbMgr->aPlaybackChnInfos[i].nRealFileNums != 0)
			{
				nRealPlayIndex++;
				DEBUG("SearchRecFiles:% chn has %d files \n",i,pSPbMgr->aPlaybackChnInfos[i].nRealFileNums);
			}

			nPlayIndex++;
		}
	}
#endif
	
	pSPbMgr->nRealPlayNum = nPlayIndex;
	
	if(nRealPlayIndex == 0)
	{
		printf("no playback files\n");
		return -1;
	}
	
	//csp modify
	for(i=0; i<pSPbMgr->nMaxChnNum; i++)
	{
		pSPbMgr->aPlaybackChnInfos[i].nCurIndex = pSPbMgr->aPlaybackChnInfos[i].nRealFileNums - 1;
	}
	
	#if 1
	pSPbMgr->nRecStartTime = 0xffffffff;
	pSPbMgr->nRecEndTime = 0;
	u8 flag_start = 0;
	u8 flag_end = 0;
	for(i=0; i<pSPbMgr->nMaxChnNum; i++)
	{
		if(pSPbMgr->aPlaybackChnInfos[i].nRealFileNums != 0)
		{
			u32 nLastFileIndex = pSPbMgr->aPlaybackChnInfos[i].nRealFileNums-1;
			
			//printf("chn[%d], pSearchParam->nStartTime = %ld, pSearchParam->nEndTime = %ld\n", i, pSearchParam->nStartTime, pSearchParam->nEndTime);
			//printf("pSPbMgr->nRecStartTime = %lu, nStartTime = %ld\n", pSPbMgr->nRecStartTime, pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[nLastFileIndex].nStartTime);
			
			if(!flag_start)
			{
				if(pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[nLastFileIndex].nStartTime > pSearchParam->nStartTime)
				{
					if(pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[nLastFileIndex].nStartTime < pSPbMgr->nRecStartTime)
					{
						pSPbMgr->nRecStartTime = pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[nLastFileIndex].nStartTime;
					}
				}
				else
				{
					pSPbMgr->nRecStartTime = pSearchParam->nStartTime;
					flag_start = 1;
				}
			}
			
			if(!flag_end)
			{
				if(pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[0].nEndTime < pSearchParam->nEndTime)
				{
					if(pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[0].nEndTime > pSPbMgr->nRecEndTime)
					{
						pSPbMgr->nRecEndTime = pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[0].nEndTime;
					}
				}
				else
				{
					pSPbMgr->nRecEndTime = pSearchParam->nEndTime;
					flag_end= 1;
				}
			}
		}
	}
	#else
	for(i=0; i<pSPbMgr->nMaxChnNum; i++)
	{
		if(pSPbMgr->aPlaybackChnInfos[i].nRealFileNums != 0)
		{
			u32 nLastFileIndex = pSPbMgr->aPlaybackChnInfos[i].nRealFileNums-1;
			pSPbMgr->nRecStartTime 
				= pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[nLastFileIndex].nStartTime;
			pSPbMgr->nRecEndTime 
				= pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[0].nEndTime;
			
			break;
		}
	}
	
	for(i=0; i<pSPbMgr->nMaxChnNum; i++)
	{
		if(pSPbMgr->aPlaybackChnInfos[i].nRealFileNums != 0)
		{
			u32 nLastFileIndex = pSPbMgr->aPlaybackChnInfos[i].nRealFileNums-1;
			pSPbMgr->nRecStartTime 
				= MIN(pSPbMgr->nRecStartTime,pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[nLastFileIndex].nStartTime);
			pSPbMgr->nRecEndTime	
				= MAX(pSPbMgr->nRecEndTime,pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[0].nEndTime);	
		}
	}
	
	pSPbMgr->nRecStartTime 
		= MAX(pSPbMgr->nRecStartTime,pSearchParam->nStartTime);
	pSPbMgr->nRecEndTime 
		= MIN(pSPbMgr->nRecEndTime,pSearchParam->nEndTime);
	#endif
	
	return 0;
}

//cdp modify 20130408
#define PLAY_OPT

//#ifdef PLAY_OPT
#if 1
void GetFileStepTime_fast(custommp4_t* pFile, u32* nFrameStepTime)
{
	if(nFrameStepTime == NULL)
	{
		return;
	}
	
	*nFrameStepTime = 40;
	
	if(pFile == NULL)
	{
		return;
	}
	
	int total_time = custommp4_total_time(pFile);
	int total_frames = custommp4_video_length(pFile);
	if(total_time == 0 || total_frames == 0)
	{
		return;
	}
	
	u32 tmp_basetime = total_time / total_frames;
	tmp_basetime = (tmp_basetime < 33) ? 33 : tmp_basetime;
	
	if(tmp_basetime > 40)
	{
		tmp_basetime = tmp_basetime / 40 * 40;
	}
	
	*nFrameStepTime = tmp_basetime;
	
	//printf("@@@@@@@@@@@@@@@@FrameStepTime = %u\n", *nFrameStepTime);
}
#else
//这样计算帧间隔时间性能开销太大
void GetFileStepTime(u8* pFileName, u32 nOpenOffset, u32* nFrameStepTime)
{
	if(nFrameStepTime == NULL)
	{
		return;
	}
	
	*nFrameStepTime = 40;//csp modify
	
	if(pFileName == NULL)
	{
		return;
	}
	
	custommp4_t* pFile = NULL;
	pFile = custommp4_open(pFileName, O_R, nOpenOffset);
	if(pFile == NULL)
	{
		return;
	}
	
	int x = 0;
	u64 first_tmp_PTS = 0;
	u64 last_tmp_PTS = 0;
	u64* pPTS = &first_tmp_PTS;
	u64 tmp_PTS = 0;
	u8 nMediaType = 0;
	u8 nKey = 0;
	u32 nStampTime = 0;
	
	u32 nBufSize = 256<<10;
#ifdef CHIP_HISI3531
	//csp modify 20121120
	//nBufSize = (256<<10)*4;
	nBufSize = 512<<10;
#endif
	
	static u8* pDecBuf = NULL;
	if(NULL == pDecBuf)
	{
		pDecBuf = calloc(nBufSize, 1);
		if(NULL == pDecBuf)
		{
			return;
		}
	}
	
	for(x = 0; x < 25; x++)
	{
		if(x > 0)
		{
			pPTS = &tmp_PTS;
		}
		
		if(0 >= custommp4_read_one_media_frame(pFile, pDecBuf, nBufSize,&nStampTime,&nKey,pPTS,&nMediaType))
		{
			break;
		}
		
		if(nMediaType)
		{
			x--;
			continue;
		}
		
		if(x > 0)
		{
			last_tmp_PTS = tmp_PTS;
		}
	}
	
	//free(pDecBuf);
	//pDecBuf = NULL;
	
	custommp4_close(pFile);
	pFile = NULL;
	
	if((x == 0) || (x == 1))
	{
		*nFrameStepTime = 40;
	}
	else
	{
		u64 tmp_basetime = (last_tmp_PTS - first_tmp_PTS)/(x-1)/1000;
		tmp_basetime = (tmp_basetime < 33) ? 33 : tmp_basetime;
		*nFrameStepTime = tmp_basetime;
	}
	
	//printf("first_tmp_PTS = %llu\n",first_tmp_PTS);
	printf("*******nFrameStepTime = %lu\n", *nFrameStepTime);
}
#endif

s32 SeekRealRecStartTime(SPlayBackManager* pSPbMgr)
{
	if(!pSPbMgr)
	{
		return -1;
	}
	
	pSPbMgr->nPlayingChnsMask = 0;
	
	//u32 nPlayNum = g_sPlayBackManager.nRealPlayNum;
	char aFileName[32] = {0};
	u32 nOpenOffset = 0;
	
	u32 i = 0;
	//printf("nMaxChnNum = %d\n", pSPbMgr->nMaxChnNum);
	for(i=0; i<pSPbMgr->nMaxChnNum; i++)
	{
		//printf("&&&haha%d, %d\n", i, pSPbMgr->aPlaybackChnInfos[i].nRealFileNums);
		if(pSPbMgr->aPlaybackChnInfos[i].nRealFileNums != 0)
		{
			pSPbMgr->nPlayingChnsMask |= (1 << i);//csp modify
			
			u32 nRealFileNums = pSPbMgr->aPlaybackChnInfos[i].nRealFileNums;
			
			get_rec_file_name((recfileinfo_t*)&pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[nRealFileNums-1], 
								aFileName, 
								&nOpenOffset);
			
			if(pSPbMgr->aPlaybackChnInfos[i].pFile != NULL)
			{
				custommp4_close(pSPbMgr->aPlaybackChnInfos[i].pFile);
				pSPbMgr->aPlaybackChnInfos[i].pFile = NULL;
			}
			//printf("***play file : %s\n", aFileName);
			
			pSPbMgr->aPlaybackChnInfos[i].nCurIndex = nRealFileNums - 1;
			
			#ifndef PLAY_OPT
			GetFileStepTime(aFileName, nOpenOffset, &pSPbMgr->aPlaybackChnInfos[i].nFrameStepTime);
			#endif
			
			pSPbMgr->aPlaybackChnInfos[i].pFile = custommp4_open(aFileName, O_R, nOpenOffset);
			if(pSPbMgr->aPlaybackChnInfos[i].pFile == NULL)
			{
				printf("custommp4_open error!!!!\n");
				continue;
				//goto out;
			}
			
			#ifdef PLAY_OPT
			GetFileStepTime_fast(pSPbMgr->aPlaybackChnInfos[i].pFile, &pSPbMgr->aPlaybackChnInfos[i].nFrameStepTime);
			//printf("@@@@@@@@@@@@@@@@nChn = %d FrameStepTime = %u\n", i, pSPbMgr->aPlaybackChnInfos[i].nFrameStepTime);
			#endif
			
			//csp modify
			pSPbMgr->aPlaybackChnInfos[i].nVWidth = custommp4_video_width(pSPbMgr->aPlaybackChnInfos[i].pFile);
			pSPbMgr->aPlaybackChnInfos[i].nVHeight = custommp4_video_height(pSPbMgr->aPlaybackChnInfos[i].pFile);
			
			//printf("pSPbMgr->nRecStartTime = %ld, nStartTime = %ld\n", pSPbMgr->nRecStartTime, pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[nRealFileNums-1].nStartTime);
			if(pSPbMgr->nRecStartTime > pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[nRealFileNums-1].nStartTime)
			{
				//printf("***before custommp4_seek_to_sys_time offset:%d, file->fpot.end_time = %d\n", pSPbMgr->aPlaybackChnInfos[i].pFile->current_video_offset, pSPbMgr->aPlaybackChnInfos[i].pFile->fpot.end_time);
				//csp modify
				//if(custommp4_seek_to_sys_time(pSPbMgr->aPlaybackChnInfos[i].pFile,pSPbMgr->nRecStartTime) < 0)
				if(custommp4_seek_to_sys_time(pSPbMgr->aPlaybackChnInfos[i].pFile,pSPbMgr->nRecStartTime) <= 0)
				{
					printf("custommp4_seek_to_sys_time error\n");
					custommp4_close(pSPbMgr->aPlaybackChnInfos[i].pFile);
					pSPbMgr->aPlaybackChnInfos[i].pFile = NULL;
					continue;
				}
				//printf("***after custommp4_seek_to_sys_time offset:%d\n", pSPbMgr->aPlaybackChnInfos[i].pFile->current_video_offset);
			}
			
			//printf("file format: %d\n", pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[nRealFileNums-1].nFormat);
			
			//pSPbMgr->nPlayingChnsMask |= (1 << i);//应该放在前面???//csp modify
		}
		
		pSPbMgr->aPlaybackChnInfos[i].nMediaFrameReady = 0;
	}
	
	return 0;//csp modify
}

s32 SetVdecSpeed(SPlayBackManager* pSPbMgr)
{
	if(!pSPbMgr)
	{
		return -1;
	}
	
	//csp:应该根据P/N制设置不同的值
	u32 nSpeedBase = 25;//P脰脝:25    N脰脝:30
	tl_vdec_speed_base(nSpeedBase);
	
	EmPlayRate emRate = pSPbMgr->emRate;
	if(emRate == EM_PLAYRATE_1)
	{
		tl_vdec_speed(VDEC_SPEED_NORMAL);
	}
	else if(emRate == EM_PLAYRATE_2)
	{
		//printf("VDEC_SPEED_UP_2X = %d\n",VDEC_SPEED_UP_2X);
		tl_vdec_speed(VDEC_SPEED_UP_2X);
	}
	else if(emRate == EM_PLAYRATE_4)
	{
		//printf("SetVdecSpeed: emRate = %d\n",emRate);
		tl_vdec_speed(VDEC_SPEED_UP_4X);
	}
	//csp modify 20130429
	//else if(emRate == EM_PLAYRATE_MAX)
	else if(emRate == EM_PLAYRATE_8)
	{
		//printf("SetVdecSpeed: emRate = %d\n",emRate);
		tl_vdec_speed(VDEC_SPEED_UP_8X);
	}
	//csp modify 20130429
	else if(emRate == EM_PLAYRATE_16)
	{
		printf("SetVdecSpeed: emRate = %d\n",emRate);
		tl_vdec_speed(VDEC_SPEED_NORMAL);
	}
	//csp modify 20130429
	else if(emRate == EM_PLAYRATE_32)
	{
		printf("SetVdecSpeed: emRate = %d\n",emRate);
		//tl_vdec_speed(VDEC_SPEED_UP_2X);
		tl_vdec_speed(VDEC_SPEED_NORMAL);
	}
	//csp modify 20130429
	else if(emRate == EM_PLAYRATE_64)
	{
		printf("SetVdecSpeed: emRate = %d\n",emRate);
		tl_vdec_speed(VDEC_SPEED_UP_2X);
		//tl_vdec_speed(VDEC_SPEED_NORMAL);
	}
	else if(emRate == EM_PLAYRATE_1_2)
	{
		tl_vdec_speed(VDEC_SPEED_DOWN_2X);
	}
	else if(emRate == EM_PLAYRATE_1_4)
	{
		tl_vdec_speed(VDEC_SPEED_DOWN_4X);
	}
	else if(emRate == EM_PLAYRATE_MIN)
	{
		tl_vdec_speed(VDEC_SPEED_DOWN_8X);
	}
	else
	{
		tl_vdec_speed(VDEC_SPEED_NORMAL);
	}
	
	int i;
	for(i = 0; i < pSPbMgr->nMaxChnNum; i++)
	{
		pSPbMgr->aPlaybackChnInfos[i].emRate = emRate;
	}
	
	return 0;
}

s32 SpeedUp(SPlayBackManager* pSPbMgr)
{
	if(!pSPbMgr)
	{
		return -1;
	}
	
	if(pSPbMgr->emRate == EM_PLAYRATE_MIN)
	{
		pSPbMgr->emRate = EM_PLAYRATE_1_4;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_1_4)
	{
		pSPbMgr->emRate = EM_PLAYRATE_1_2;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_1_2)
	{
		pSPbMgr->emRate = EM_PLAYRATE_1;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_1)
	{
		//printf("EM_PLAYRATE_2\n");
		pSPbMgr->emRate = EM_PLAYRATE_2;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_2)
	{
		//printf("EM_PLAYRATE_4\n");
		pSPbMgr->emRate = EM_PLAYRATE_4;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_4)
	{
		//csp modify 20130429
		//pSPbMgr->emRate = EM_PLAYRATE_MAX;
		pSPbMgr->emRate = EM_PLAYRATE_8;
	}
	//csp modify 20130429
	//else if(pSPbMgr->emRate == EM_PLAYRATE_MAX)
	//{
	//	//nothing to do
	//}
	//csp modify 20130429
	else if(pSPbMgr->emRate == EM_PLAYRATE_8)
	{
		pSPbMgr->emRate = EM_PLAYRATE_16;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_16)
	{
		pSPbMgr->emRate = EM_PLAYRATE_32;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_32)
	{
		pSPbMgr->emRate = EM_PLAYRATE_64;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_64)
	{
		//nothing to do
	}
	
	SetVdecSpeed(pSPbMgr);
	
	return 0;
}

s32 SpeedDown(SPlayBackManager* pSPbMgr)
{
	if(!pSPbMgr)
	{
		return -1;
	}
	
	//csp modify 20130429
	if(pSPbMgr->emRate == EM_PLAYRATE_64)
	{
		pSPbMgr->emRate = EM_PLAYRATE_32;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_32)
	{
		pSPbMgr->emRate = EM_PLAYRATE_16;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_16)
	{
		pSPbMgr->emRate = EM_PLAYRATE_8;
	}
	//csp modify 20130429
	//if(pSPbMgr->emRate == EM_PLAYRATE_MAX)
	else if(pSPbMgr->emRate == EM_PLAYRATE_8)
	{
		pSPbMgr->emRate = EM_PLAYRATE_4;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_4)
	{
		pSPbMgr->emRate = EM_PLAYRATE_2;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_2)
	{
		pSPbMgr->emRate = EM_PLAYRATE_1;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_1)
	{
		pSPbMgr->emRate = EM_PLAYRATE_1_2;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_1_2)
	{
		pSPbMgr->emRate = EM_PLAYRATE_1_4;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_1_4)
	{
		pSPbMgr->emRate = EM_PLAYRATE_MIN;
	}
	else if(pSPbMgr->emRate == EM_PLAYRATE_MIN)
	{
		//nothing to do
	}

	SetVdecSpeed(pSPbMgr);

	return 0;
}

s32 SetSpeed(SPlayBackManager* pSPbMgr,EmPlayRate emRate)
{
	if(!pSPbMgr)
	{
		return -1;
	}
	
	pSPbMgr->emRate = emRate;
	
	SetVdecSpeed(pSPbMgr);
	
	return 0;//csp modify
}

s32 ResetSpeedToNormal(SPlayBackManager* pSPbMgr)
{
	if(!pSPbMgr)
	{
		return -1;
	}
	
	//if(pSPbMgr->emRate != EM_PLAYRATE_1)
	{
		pSPbMgr->emRate = EM_PLAYRATE_1;
		tl_vdec_speed(VDEC_SPEED_NORMAL);
		
		int i;
		for(i = 0; i < pSPbMgr->nMaxChnNum; i++)
		{
			pSPbMgr->aPlaybackChnInfos[i].emRate = pSPbMgr->emRate;
		}
	}
	
	return 0;
}

s32 Seek(SPlaybackChnInfo* psPlaybackChnInfo, u32 nSeekTime)
{
	if(!psPlaybackChnInfo)
	{
		return -1;
	}
	
	int ret = -1;
	s8 aFileName[64] = {0};
	u32 nOpenOffset = 0;
	s32 nCurIndex = psPlaybackChnInfo->nRealFileNums - 1;
	
#if 0//csp modify
	u8 nMediaType = 0;
	u8 nKey = 0;
	u32 nStampTime = 0;
	
	u32 nBufSize = 256<<10;
	#ifdef CHIP_HISI3531
	nBufSize = (256<<10)*4;
	#endif
	static u8* pDecBuf = NULL;
	if(NULL == pDecBuf)
	{
		pDecBuf = calloc(nBufSize, 1);
		if(NULL == pDecBuf)
		{
			return;
		}
	}
#endif
	
	while(nCurIndex >= 0)
	{
		if(psPlaybackChnInfo->aRecFileInfos[nCurIndex].nStartTime <= nSeekTime
			&& nSeekTime <= psPlaybackChnInfo->aRecFileInfos[nCurIndex].nEndTime)
		{
			get_rec_file_name((recfileinfo_t*)&psPlaybackChnInfo->aRecFileInfos[nCurIndex], 
								aFileName, 
								&nOpenOffset);
			
			if(psPlaybackChnInfo->pFile != NULL)
			{
				custommp4_close(psPlaybackChnInfo->pFile);
				psPlaybackChnInfo->pFile = NULL;
			}
			
			#ifndef PLAY_OPT
			GetFileStepTime(aFileName, nOpenOffset, &psPlaybackChnInfo->nFrameStepTime);
			#endif
			
			psPlaybackChnInfo->pFile = custommp4_open(aFileName, O_R, nOpenOffset);
			if(psPlaybackChnInfo->pFile == NULL)
			{
				printf("custommp4_open error\n");
				nCurIndex--;//nCurIndex = -1;
				continue;
			}
			
			#ifdef PLAY_OPT
			GetFileStepTime_fast(psPlaybackChnInfo->pFile, &psPlaybackChnInfo->nFrameStepTime);
			#endif
			
			//csp modify
			psPlaybackChnInfo->nVWidth = custommp4_video_width(psPlaybackChnInfo->pFile);
			psPlaybackChnInfo->nVHeight = custommp4_video_height(psPlaybackChnInfo->pFile);
			
			//如果nSeekTime到文件结束的这段时间没有关键帧，则偏移会转到文件开头，故作以下纠正
			//printf("nEndTime = %lu, seektime = %lu, file.start = %d,file.end=%d\n", psPlaybackChnInfo->aRecFileInfos[nCurIndex].nEndTime, nSeekTime,psPlaybackChnInfo->pFile->fpot.start_time,psPlaybackChnInfo->pFile->fpot.end_time);
			SEEK_NO_KEY_FRAME:
			if((ret = custommp4_seek_to_sys_time(psPlaybackChnInfo->pFile,nSeekTime)) < 0)
			{
				printf("custommp4_seek_to_sys_time error\n");
				custommp4_close(psPlaybackChnInfo->pFile);
				psPlaybackChnInfo->pFile = NULL;
				nCurIndex--;//nCurIndex = -1;
				continue;
			}
			if(ret == 0)//到文件尾都没有关键帧
			{
				if(((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nRealPlayNum == 1)
				{
					nSeekTime--;
					if(nSeekTime < psPlaybackChnInfo->aRecFileInfos[nCurIndex].nStartTime)
					{
						//csp modify
						custommp4_close(psPlaybackChnInfo->pFile);
						psPlaybackChnInfo->pFile = NULL;
						
						nCurIndex--;
						continue;
					}
					((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nRefPTS -= 1000;
					goto SEEK_NO_KEY_FRAME;
				}
				else
				{
					//csp modify
					custommp4_close(psPlaybackChnInfo->pFile);
					psPlaybackChnInfo->pFile = NULL;
					
					nCurIndex--;
					continue;
				}
			}
			if(ret > 0)
			{
				#if 0//csp modify
				if(((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nRealPlayNum == 1)
				{
					u8 tmp = 2;
					u64 pts = 0;
					while(tmp--)
					{
						if(0 >= custommp4_read_one_media_frame(psPlaybackChnInfo->pFile, pDecBuf, nBufSize, &nStampTime, &nKey, &pts, &nMediaType))
						{
							break;
						}
						if(!nMediaType)
						{
							//printf("nRefPts = %llu, ", ((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nRefPTS);
							((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nRefPTS = pts/1000;
							//printf("%llu\n", ((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nRefPTS);
							custommp4_seek_to_sys_time(psPlaybackChnInfo->pFile,nSeekTime);
							break;
						}
					}
				}
				#endif
			}
			
			//psPlaybackChnInfo->nMediaFrameReady = 0;
			
			psPlaybackChnInfo->nFlagDoSeek = 1;
			
			break;
		}
		else if(nSeekTime <= psPlaybackChnInfo->aRecFileInfos[nCurIndex].nStartTime)
		{
			get_rec_file_name((recfileinfo_t*)&psPlaybackChnInfo->aRecFileInfos[nCurIndex], 
								aFileName, 
								&nOpenOffset);
			
			if(psPlaybackChnInfo->pFile != NULL)
			{
				custommp4_close(psPlaybackChnInfo->pFile);
				psPlaybackChnInfo->pFile = NULL;
			}
			
			#ifndef PLAY_OPT
			GetFileStepTime(aFileName, nOpenOffset, &psPlaybackChnInfo->nFrameStepTime);
			#endif
			
			psPlaybackChnInfo->pFile = custommp4_open(aFileName, O_R, nOpenOffset);
			if(psPlaybackChnInfo->pFile == NULL)
			{
				printf("custommp4_open error seek\n");
				nCurIndex--;//nCurIndex = -1;
				continue;
			}
			
			#ifdef PLAY_OPT
			GetFileStepTime_fast(psPlaybackChnInfo->pFile, &psPlaybackChnInfo->nFrameStepTime);
			#endif
			
			//csp modify
			psPlaybackChnInfo->nVWidth = custommp4_video_width(psPlaybackChnInfo->pFile);
			psPlaybackChnInfo->nVHeight = custommp4_video_height(psPlaybackChnInfo->pFile);
			
			//psPlaybackChnInfo->nMediaFrameReady = 0;
			
			psPlaybackChnInfo->nIsPlaying = 0;
			
			break;
		}	
		
		nCurIndex--;
	}
	
	/*if(nCurIndex >= 0)
	{
		g_sPlayBackManager.nRefTime = (u64)nSeekTime*1000;
	}*/
	
	psPlaybackChnInfo->nCurIndex = nCurIndex;
	
	psPlaybackChnInfo->nMediaFrameReady = 0;
	
	if(nCurIndex == -1)
	{
		if(psPlaybackChnInfo->pFile != NULL)
		{
			custommp4_close(psPlaybackChnInfo->pFile);
			psPlaybackChnInfo->pFile = NULL;
		}
		psPlaybackChnInfo->nIsPlaying = 0;
	}
	
	return 0;
}

s32 CheckPreSectTime(SPlayBackManager* pSPbMgr, u32 nSeekTime)
{
	if(NULL == pSPbMgr)
	{
		return -1;
	}
	
	//csp modify
	//u8 aFileName[64] = {0};
	//u32 nOpenOffset = 0;
	
	u8 i = 0;
	s32 flag = -1;
	for(i = 0; i < pSPbMgr->nMaxChnNum; i++)
	{
		if(pSPbMgr->aPlaybackChnInfos[i].nRealFileNums)
		{
			s32 nCurIndex = pSPbMgr->aPlaybackChnInfos[i].nRealFileNums-1;
			while(nCurIndex >= 0)
			{
				if(pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[nCurIndex].nStartTime <= nSeekTime
					&& nSeekTime <= pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[nCurIndex].nEndTime)
				{
					flag = 0;
					break;
				}
				else if(nSeekTime <= pSPbMgr->aPlaybackChnInfos[i].aRecFileInfos[nCurIndex].nStartTime)
				{
					break;
				}
				nCurIndex--;
			}
			
			if(0 == flag)
			{
				break;
			}
		}
	}
	
	return flag;
}

#define SECT_NUMS	n_sect_nums

s32 SeekPreSect(SPlayBackManager* pSPbMgr)
{
	if(!pSPbMgr)
	{
		return -1;
	}
	
	u32 nTotalTime = pSPbMgr->nTotalTime;
	u8 n_sect_nums = nTotalTime > 10 ? 10 : nTotalTime;
	
	u32 nRefPts = pSPbMgr->nRefPTS/1000 - pSPbMgr->nRecStartTime;
	s32 nPreSectID = (nRefPts/(nTotalTime/SECT_NUMS)-1);
	u32 nPreSectTime = 0;
	
	DEBUG("nTotalTime = %d\n",nTotalTime);
	
	while(1)
	{
		if(nPreSectID < 0)
		{
			printf("no pre sect\n");
			nPreSectTime = pSPbMgr->nRecStartTime;
			break;
		}
		else
		{
			nPreSectTime = nPreSectID*(nTotalTime/SECT_NUMS) + pSPbMgr->nRecStartTime;
		}
		if(0 == CheckPreSectTime(pSPbMgr, nPreSectTime))
		{
			break;
		}
		nPreSectID--;
	}
	
	return nPreSectTime;
}

s32 SeekNextSect(SPlayBackManager* pSPbMgr)
{
	if(!pSPbMgr)
	{
		return -1;
	}
	
	u32 nTotalTime = pSPbMgr->nTotalTime;
	u8 n_sect_nums = nTotalTime > 10 ? 10 : nTotalTime;
	
	u32 nRefPts = pSPbMgr->nRefPTS / 1000 - pSPbMgr->nRecStartTime;
	u32 nNextSectID = (nRefPts / (nTotalTime/SECT_NUMS) + 1);
	
	DEBUG("nTotalTime = %d \n",nTotalTime);
	DEBUG("pSPbMgr->nRecStartTime = %d \n",pSPbMgr->nRecStartTime);
	DEBUG("pSPbMgr->nRecEndTime = %d \n",pSPbMgr->nRecEndTime);
	
	u32 nNextSectTime = 0;
	if(nNextSectID >= SECT_NUMS)
	{
		printf("time over,stop playback\n");
		nNextSectTime = pSPbMgr->nRecEndTime + 1;
		DEBUG("nNextSectTime = %d\n",nNextSectTime);
	}
	else
	{
		nNextSectTime = nNextSectID*(nTotalTime/SECT_NUMS) + pSPbMgr->nRecStartTime;
		DEBUG("nNextSectTime = %d\n",nNextSectTime);
	}
	
	//Seek(pSPbMgr,nNextSectTime);
	
	return nNextSectTime;
}

s32 StopPlayback(SPlayBackManager* pSPbManager, s8 flush)
{
	if(!pSPbManager)
	{
		return -1;
	}
	
	if(pbStopElecZoom)
	{
		//printf("**************pbStopElecZoom\n");
		pbStopElecZoom();//关闭电子放大
		//printf("**************pbStopElecZoom OK\n");
	}
	
	SChnMSG sChnMsg;
	sChnMsg.emCmd = EM_CHN_STOP;
	
	u64 nAck = 0;
	u32 i = 0;
	for(i=0; i<pSPbManager->nMaxChnNum; i++)
	{
		if(pSPbManager->aPlaybackChnInfos[i].nRealFileNums != 0)
		{
			//printf("chn %d EM_CHN_STOP\n",i);
			SendChnMsg(&pSPbManager->aPlaybackChnInfos[i],&sChnMsg,&nAck);
		}
	}
	
	for(i=0; i<pSPbManager->nRealPlayNum; i++)
	{
		if(flush)//csp modify 20130408
		{
			//printf("**************tl_vdec_flush\n");
			tl_vdec_flush(i);
			//printf("**************tl_vdec_flush OK\n");
		}
	}
	
	//printf("**************tl_vdec_close\n");
	tl_vdec_close();//yzw
	//printf("**************tl_vdec_close OK\n");
	
	if(pbProgCB)
	{
		SPBCallbackInfo cbInfo;
		cbInfo.bStop = 1;
		cbInfo.nCurTime = pSPbManager->nRefPTS;
		cbInfo.nTotalTime = pSPbManager->nTotalTime;
		cbInfo.nProg = 100;
		//printf("**************pbProgCB\n");
		pbProgCB(&cbInfo);
		//printf("**************pbProgCB OK\n");
	}
	
	//printf("**************StopPlayback OK\n");
	
	return 0;
}

void* PBCtlFunc(void* arg)
{
	if(!arg)
	{
		return 0;
	}
	
	//printf("$$$$$$$$$$$$$$$$$$PBCtlFunc id:%d\n",getpid());
	
	//u32 cnt = 0;//csp modify
	
	SPlayBackManager* pSPbManager = (SPlayBackManager*)arg;
	SCtlMSG sCtlMsg;
	EmPBCtlCmd emPbCtlCmd;
	UnCtlCxt unPbCtlCxt;
	SChnMSG sChnMsg;
	
	//u32* ack = malloc(sizeof(u32)*g_sPlayBackManager.nMaxChnNum);
	
	u64 nRefSystime = 0;
	u64 nArriveTime = 0;
	
	s32 nRate = 1;
	
	u32 nBaseInterval = 40;
	s32 nInterval = 40;
	
	//u8 nForword = 1;
	
	//csp modify 20130509
	//EmCtlState emCurCtlState = EM_CTL_STATE_IDLE;
	pSPbManager->emCurCtlState = EM_CTL_STATE_IDLE;
	
	while(1)
	{
		GetCtlMsg(pSPbManager,&sCtlMsg);
		emPbCtlCmd = sCtlMsg.emCmd;
		unPbCtlCxt = sCtlMsg.unCxt;
		
		//csp modify
		if(emPbCtlCmd == EM_CTL_NULL)
		{
			goto PlayRunning;
		}
		
		//printf("%s cur state: %d, emPbCtlCmd = %d\n",__func__, pSPbManager->emCurCtlState, emPbCtlCmd);
		if(emPbCtlCmd == EM_CTL_MUTE)
		{
			int i;
			//printf("^^^^^^^%s,%d,Mute=%d\n",__func__,__LINE__,unPbCtlCxt.nMute);
			for(i = 0; i < pSPbManager->nMaxChnNum; i++)
			{
				pSPbManager->aPlaybackChnInfos[i].nMute = unPbCtlCxt.nMute;
			}
		}
		else
		{
			switch(pSPbManager->emCurCtlState)
			{
				case EM_CTL_STATE_IDLE:
				{
					if((emPbCtlCmd == EM_CTL_PBTIME) || (emPbCtlCmd == EM_CTL_PBFILE))
					{
						if(emPbCtlCmd == EM_CTL_PBTIME)
						{
							SPBSearchPara sSearchParam = unPbCtlCxt.sSearchParam;
							SearchRecFiles(pSPbManager,&sSearchParam);
						}
						else if(emPbCtlCmd == EM_CTL_PBFILE)
						{
							SPBRecfileInfo sFileInfo = unPbCtlCxt.sFileInfo;
							sFileInfo.nChn -= 1; //结构体中的nChn>=1,而非>=0
							InsertFileInfo(pSPbManager,&sFileInfo);
						}
						else
						{
							//error
						}
						
						//printf(">>>>>>>search ok, time:%d\n", time(0));
						
						u32 nSearchFileNum = pSPbManager->nAllSrchFileNum;
						
						pSPbManager->nStepBaseTime = 2000;//需要设置个无效值
						
						u32 i = 0;
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							pSPbManager->aPlaybackChnInfos[i].nFrameStepTime = 40;
							pSPbManager->aPlaybackChnInfos[i].nFlagDoSeek = 0;
							
							//csp modify
							pSPbManager->aPlaybackChnInfos[i].nIsPlaying = 0;
							pSPbManager->aPlaybackChnInfos[i].nMediaFrameReady = 0;
						}
						
						#if 1//csp modify
						pSPbManager->nForward = 1;
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							pSPbManager->aPlaybackChnInfos[i].nForward = 1;
						}
						#endif
						
						pSPbManager->nPlayingChnsMask = 0;//csp modify
						
						//csp modify
						//SeekRealRecStartTime(pSPbManager);
						//printf(">>>>>>>seek ok, time:%d\n", time(0));
						
						pSPbManager->nTotalTime = pSPbManager->nRecEndTime - pSPbManager->nRecStartTime;
						
						if((0 == nSearchFileNum) || (0 == pSPbManager->nTotalTime))
						{
							pSPbManager->emCurCtlState = EM_CTL_STATE_IDLE;
							
							AckCtlMsg(pSPbManager);
							
							//yzw add
							if(pbProgCB)
							{
								SPBCallbackInfo cbInfo;
								cbInfo.bStop = 1;
								cbInfo.nCurTime = pSPbManager->nRefPTS;
								cbInfo.nTotalTime = pSPbManager->nTotalTime;
								cbInfo.nProg = 100;
								pbProgCB(&cbInfo);
							}
							//end
							
							continue;
						}
						
						SeekRealRecStartTime(pSPbManager);//csp modify
						
						#if 0//csp modify
						pSPbManager->nForward = 1;
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							pSPbManager->aPlaybackChnInfos[i].nForward = 1;
						}
						#endif
						
						pSPbManager->emRate = EM_PLAYRATE_1;
						for(i = 0; i < pSPbManager->nMaxChnNum; i++)
						{
							pSPbManager->aPlaybackChnInfos[i].emRate = pSPbManager->emRate;
						}
						
						//g_sPlayBackManager.nRefTime = (u64)g_sPlayBackManager.nRecStartTime*1000;
						
						u32 nRealPlayNum = pSPbManager->nRealPlayNum;
						if(nRealPlayNum == 1)
						{
							tl_vdec_open(1);
						}
						else if(nRealPlayNum <= 4)
						{
							tl_vdec_open(4);
						}
						else if(nRealPlayNum <= 9)
						{
							tl_vdec_open(9);
						}
						else if(nRealPlayNum <= 16)
						{
							tl_vdec_open(16);
						}
						else if(nRealPlayNum <= 24)
						{
							tl_vdec_open(24);
						}
						else
						{
							printf("%d playChns is too large\n", nRealPlayNum);
							continue;
						}
						//printf(">>>>>>>open dec ok, TotalTime:%u\n", (int)pSPbManager->nTotalTime);//csp modify 20150110
						
						//csp:应该根据P/N制设置不同的值
						tl_vdec_speed_base(25);
						ResetSpeedToNormal(pSPbManager);
						
						nRefSystime = 0;//yzw add
						nInterval = 40;//yzw add
						nBaseInterval = 40;
						
						pSPbManager->nRefPTS = (u64)pSPbManager->nRecStartTime*1000;//csp modify
						
						sChnMsg.emCmd = EM_CHN_PLAY;
						u64 nAck = 0;
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							if(pSPbManager->aPlaybackChnInfos[i].nRealFileNums != 0)
							{
								SendChnMsg(&pSPbManager->aPlaybackChnInfos[i],&sChnMsg,&nAck);
							}
						}
						
						#if 0//csp modify
						pSPbManager->nRefPTS = (u64)pSPbManager->nRecStartTime*1000;
						//pSPbManager->nRefPTS += 800; //做800ms的偏移,可以消除卡顿一下
						//printf("pSPbManager->nRefPTS = %lld\n",pSPbManager->nRefPTS);
						#endif
						
						pSPbManager->emCurCtlState = EM_CTL_STATE_RUN;
					}
				}
				break;
				case EM_CTL_STATE_PAUSE:
				{
					if(emPbCtlCmd == EM_CTL_RESUME)
					{
						nRefSystime = 0;
						
						pSPbManager->emCurCtlState = EM_CTL_STATE_RUN;
					}
					else if(emPbCtlCmd == EM_CTL_STOP)
					{
						StopPlayback(pSPbManager, 0);
						
						pSPbManager->emCurCtlState = EM_CTL_STATE_IDLE;
					}
					else if(emPbCtlCmd == EM_CTL_STEP)
					{
						ResetSpeedToNormal(pSPbManager);
						
						nInterval = nBaseInterval;//40;//yzw add//csp modify 20130429
						
						pSPbManager->nForward = 1;
						
						u32 i = 0;
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							pSPbManager->aPlaybackChnInfos[i].nForward = 1;
						}
						
						//csp modify 20130509
						#if defined(CHIP_HISI3531)
						pSPbManager->nStepBaseTime = 2000;//需要设置个无效值
						#endif
						
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							if((pSPbManager->aPlaybackChnInfos[i].nRealFileNums != 0) && (pSPbManager->aPlaybackChnInfos[i].pFile != NULL))
							{
								if(pSPbManager->nStepBaseTime > pSPbManager->aPlaybackChnInfos[i].nFrameStepTime)
								{
								#if defined(CHIP_HISI3531)//csp modify 20130509
									if(pSPbManager->aPlaybackChnInfos[i].nFlagZoom != 0)
									{
										pSPbManager->nStepBaseTime = pSPbManager->aPlaybackChnInfos[i].nFrameStepTime;
									}
								#else
									pSPbManager->nStepBaseTime = pSPbManager->aPlaybackChnInfos[i].nFrameStepTime;
								#endif
								}
								//printf("chn[%d], nFrameStepTime = %lu\n", i, pSPbManager->aPlaybackChnInfos[i].nFrameStepTime);
							}
						}
						//printf("pSPbManager->nStepBaseTime = %lu\n", pSPbManager->nStepBaseTime);
						
						int nProg = 100*(pSPbManager->nRefPTS/1000-pSPbManager->nRecStartTime)/pSPbManager->nTotalTime;
						
						if(pbProgCB)
						{
							SPBCallbackInfo cbInfo;
							cbInfo.bStop = 0;
							cbInfo.nCurTime = pSPbManager->nRefPTS;
							cbInfo.nTotalTime = pSPbManager->nTotalTime;
							cbInfo.nProg = nProg;
							pbProgCB(&cbInfo);
						}
						
						if(pSPbManager->nForward == 1)
						{
							pSPbManager->nRefPTS += pSPbManager->nStepBaseTime;
						}
						else
						{
							pSPbManager->nRefPTS -= pSPbManager->nStepBaseTime;
						}
						
						sChnMsg.emCmd = EM_CHN_UPDATEPTS;
						sChnMsg.unCxt.nRefPts = pSPbManager->nRefPTS;
						
						u64 nAck = 0;
						u64 nNewPts = (pSPbManager->nForward==1) ? ~0 : 0;
						
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							if(pSPbManager->aPlaybackChnInfos[i].nRealFileNums != 0)
							{
								SendChnMsg(&pSPbManager->aPlaybackChnInfos[i],&sChnMsg,&nAck);
								
								if((pSPbManager->nForward==1 && nAck<nNewPts) || (pSPbManager->nForward==-1 && nAck>nNewPts))
								{
									nNewPts = nAck;
								}
							}
						}
						
						if(pSPbManager->nForward == 1)
						{
							if(nNewPts == ~0)
							{
								//printf("all chn stop, ctl stop\n");
								StopPlayback(pSPbManager, 1);
								
								pSPbManager->emCurCtlState = EM_CTL_STATE_IDLE;
							}
							else
							{
								if((s64)(nNewPts-pSPbManager->nRefPTS) > JUMP_TIME)
								{
									//Seek(pSPbManager,(u32)(nNewPts/1000));
									pSPbManager->nRefPTS = nNewPts;
								}
							}
						}
						else
						{
							if(nNewPts == 0)
							{
								//printf("all chn stop, ctl stop\n");
								StopPlayback(pSPbManager, 1);
								
								pSPbManager->emCurCtlState = EM_CTL_STATE_IDLE;
							}
							else
							{
								if((s64)(pSPbManager->nRefPTS-nNewPts) > JUMP_TIME)
								{
									//Seek(pSPbManager,(u32)(nNewPts/1000));
									pSPbManager->nRefPTS = nNewPts;
								}
							}
						}
					}
					else if(emPbCtlCmd == EM_CTL_SPEED_UP)
					{
						SpeedUp(pSPbManager);
						
						nRate = pSPbManager->emRate;
						nInterval = nBaseInterval;
						if(nRate < 0)
						{
							nInterval *= -nRate;
						}
						else if(nRate > 8)//csp modify 20130429
						{
							s32 v = nRate / 2;
							v = v ? v : 1;
							nInterval /= v;
						}
						else if(nRate > 0)//csp modify
						{
							nInterval /= nRate;
						}
						
						//csp modify 20130429
						if(nInterval == 0)
						{
							nInterval = 1;
						}
						
						pSPbManager->emCurCtlState = EM_CTL_STATE_RUN;
					}
					else if(emPbCtlCmd == EM_CTL_SPEED_DOWN)
					{
						SpeedDown(pSPbManager);
						
						nRate = pSPbManager->emRate;
						nInterval = nBaseInterval;
						if(nRate < 0)
						{
							nInterval *= -nRate;
						}
						else if(nRate > 8)//csp modify 20130429
						{
							s32 v = nRate / 2;
							v = v ? v : 1;
							nInterval /= v;
						}
						else if(nRate > 0)//csp modify
						{
							nInterval /= nRate;
						}
						
						//csp modify 20130429
						if(nInterval == 0)
						{
							nInterval = 1;
						}
						
						pSPbManager->emCurCtlState = EM_CTL_STATE_RUN;
					}
					else if(emPbCtlCmd == EM_CTL_SET_SPEED)
					{
						EmPlayRate emPlayRate = unPbCtlCxt.emPlayRate;
						
						SetSpeed(pSPbManager,emPlayRate);
						
						nRate = pSPbManager->emRate;
						nInterval = nBaseInterval;
						if(nRate < 0)
						{
							nInterval *= -nRate;
						}
						else if(nRate > 8)//csp modify 20130429
						{
							s32 v = nRate / 2;
							v = v ? v : 1;
							nInterval /= v;
						}
						else if(nRate > 0)//csp modify
						{
							nInterval /= nRate;
						}
						
						//csp modify 20130429
						if(nInterval == 0)
						{
							nInterval = 1;
						}
						
						pSPbManager->emCurCtlState = EM_CTL_STATE_RUN;
					}
					else if(emPbCtlCmd == EM_CTL_SEEK)
					{
						u32 nSeekTime = unPbCtlCxt.nSeekTime;
						pSPbManager->nRefPTS = (u64)nSeekTime*1000;
						//pSPbManager->nRefPTS += 1200; //做1200ms的偏移,可以消除卡顿一下
						
						//Seek(pSPbManager,nSeekTime);
						sChnMsg.emCmd = EM_CHN_SEEK;
						sChnMsg.unCxt.nSeekTime = nSeekTime;
						
						u64 nAck = 0;
						s32 i;
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							if(pSPbManager->aPlaybackChnInfos[i].nRealFileNums != 0)
							{
								SendChnMsg(&pSPbManager->aPlaybackChnInfos[i],&sChnMsg,&nAck);
							}
						}
						
						pSPbManager->emCurCtlState = EM_CTL_STATE_RUN;
					}
					else if(emPbCtlCmd == EM_CTL_NXT_SECT)
					{
						u32 nNextTime = SeekNextSect(pSPbManager);
						pSPbManager->nRefPTS = (u64)nNextTime*1000;
						//pSPbManager->nRefPTS += 1200; //做1200ms的偏移,可以消除卡顿一下
						
						sChnMsg.emCmd = EM_CHN_SEEK;
						sChnMsg.unCxt.nSeekTime = nNextTime;
						
						u64 nAck = 0;
						s32 i;
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							if(pSPbManager->aPlaybackChnInfos[i].nRealFileNums != 0)
							{
								SendChnMsg(&pSPbManager->aPlaybackChnInfos[i],&sChnMsg,&nAck);
							}
						}
						
						pSPbManager->emCurCtlState = EM_CTL_STATE_RUN;
					}
					else if(emPbCtlCmd == EM_CTL_PRE_SECT)
					{
						u32 nPreTime = SeekPreSect(pSPbManager);
						pSPbManager->nRefPTS = (u64)nPreTime*1000;
						//pSPbManager->nRefPTS += 1200; //做1200ms的偏移,可以消除卡顿一下
						
						sChnMsg.emCmd = EM_CHN_SEEK;
						sChnMsg.unCxt.nSeekTime = nPreTime;
						
						u64 nAck = 0;
						s32 i;
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							if(pSPbManager->aPlaybackChnInfos[i].nRealFileNums != 0)
							{
								SendChnMsg(&pSPbManager->aPlaybackChnInfos[i],&sChnMsg,&nAck);
							}
						}
						
						pSPbManager->emCurCtlState = EM_CTL_STATE_RUN;
					}
				}
				break;
				case EM_CTL_STATE_RUN:
				{
					if(emPbCtlCmd == EM_CTL_STOP)
					{
						StopPlayback(pSPbManager, 0);
						
						pSPbManager->emCurCtlState = EM_CTL_STATE_IDLE;
					}
					else if(emPbCtlCmd == EM_CTL_PAUSE)
					{
						pSPbManager->emCurCtlState = EM_CTL_STATE_PAUSE;
					}
					else if(emPbCtlCmd == EM_CTL_SPEED_UP)
					{
						SpeedUp(pSPbManager);
						
						nRate = pSPbManager->emRate;
						nInterval = nBaseInterval;
						if(nRate < 0)
						{
							nInterval *= -nRate;
						}
						else if(nRate > 8)//csp modify 20130429
						{
							s32 v = nRate / 2;
							v = v ? v : 1;
							nInterval /= v;
						}
						else if(nRate > 0)//csp modify
						{
							nInterval /= nRate;
						}
						
						//csp modify 20130429
						if(nInterval == 0)
						{
							nInterval = 1;
						}
					}
					else if(emPbCtlCmd == EM_CTL_SPEED_DOWN)
					{
						SpeedDown(pSPbManager);
						
						nRate = pSPbManager->emRate;
						nInterval = nBaseInterval;
						if(nRate < 0)
						{
							nInterval *= -nRate;
						}
						else if(nRate > 8)//csp modify 20130429
						{
							s32 v = nRate / 2;
							v = v ? v : 1;
							nInterval /= v;
						}
						else if(nRate > 0)//csp modify
						{
							nInterval /= nRate;
						}
						
						//csp modify 20130429
						if(nInterval == 0)
						{
							nInterval = 1;
						}
					}
					else if(emPbCtlCmd == EM_CTL_SET_SPEED)
					{
						EmPlayRate emPlayRate = unPbCtlCxt.emPlayRate;
						
						SetSpeed(pSPbManager,emPlayRate);
						
						nRate = pSPbManager->emRate;
						nInterval = nBaseInterval;
						if(nRate < 0)
						{
							nInterval *= -nRate;
						}
						else if(nRate > 8)//csp modify 20130429
						{
							s32 v = nRate / 2;
							v = v ? v : 1;
							nInterval /= v;
						}
						else if(nRate > 0)//csp modify
						{
							nInterval /= nRate;
						}
						
						//csp modify 20130429
						if(nInterval == 0)
						{
							nInterval = 1;
						}
					}
					else if(emPbCtlCmd == EM_CTL_STEP)
					{
						ResetSpeedToNormal(pSPbManager);
						
						pSPbManager->emCurCtlState = EM_CTL_STATE_PAUSE;
					}
					else if(emPbCtlCmd == EM_CTL_BACKWARD)
					{
						u32 i = 0;
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							if(pSPbManager->aPlaybackChnInfos[i].nRealFileNums != 0)
							{
								sChnMsg.emCmd = EM_CHN_BACKWARD;
								u64 nAck = 0;
								SendChnMsg(&pSPbManager->aPlaybackChnInfos[i],&sChnMsg,&nAck);
							}
						}
						
						pSPbManager->nForward = -1;
						
						nRefSystime = 0;
					}
					else if(emPbCtlCmd == EM_CTL_FORWARD)
					{
						u32 i = 0;
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							if(pSPbManager->aPlaybackChnInfos[i].nRealFileNums != 0)
							{
								sChnMsg.emCmd = EM_CHN_FORWARD;
								u64 nAck = 0;
								SendChnMsg(&pSPbManager->aPlaybackChnInfos[i],&sChnMsg,&nAck);
							}
						}
						
						pSPbManager->nForward = 1;
					}
					else if(emPbCtlCmd == EM_CTL_SEEK)
					{
						u32 nSeekTime = unPbCtlCxt.nSeekTime;
						pSPbManager->nRefPTS = (u64)nSeekTime*1000;
						//pSPbManager->nRefPTS += 1200; //做1200ms的偏移,可以消除卡顿一下
						
						//Seek(pSPbManager,nSeekTime);
						
						sChnMsg.emCmd = EM_CHN_SEEK;
						sChnMsg.unCxt.nSeekTime = nSeekTime;
						u64 nAck = 0;
						s32 i;
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							if(pSPbManager->aPlaybackChnInfos[i].nRealFileNums != 0)
							{
								SendChnMsg(&pSPbManager->aPlaybackChnInfos[i],&sChnMsg,&nAck);
							}
						}
						
						pSPbManager->emCurCtlState = EM_CTL_STATE_RUN;
					}
					else if(emPbCtlCmd == EM_CTL_NXT_SECT)
					{
						u32 nNextTime = SeekNextSect(pSPbManager);
						pSPbManager->nRefPTS = (u64)nNextTime*1000;
						//pSPbManager->nRefPTS += 1200; //做1200ms的偏移,可以消除卡顿一下
						
						sChnMsg.emCmd = EM_CHN_SEEK;
						sChnMsg.unCxt.nSeekTime = nNextTime;
						
						u64 nAck = 0;
						s32 i;
						
						DEBUG("before SendChnMsg(EM_CTL_NXT_SECT)\n");
						
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							if(pSPbManager->aPlaybackChnInfos[i].nRealFileNums != 0)
							{
								SendChnMsg(&pSPbManager->aPlaybackChnInfos[i],&sChnMsg,&nAck);
							}
						}
						
						DEBUG("after SendChnMsg(EM_CTL_NXT_SECT)\n");
						
						pSPbManager->emCurCtlState = EM_CTL_STATE_RUN;
					}
					else if(emPbCtlCmd == EM_CTL_PRE_SECT)
					{
						u32 nPreTime = SeekPreSect(pSPbManager);
						pSPbManager->nRefPTS = (u64)nPreTime*1000;
						//pSPbManager->nRefPTS += 1200; //做1200ms的偏移,可以消除卡顿一下
						
						sChnMsg.emCmd = EM_CHN_SEEK;
						sChnMsg.unCxt.nSeekTime = nPreTime;
						
						u64 nAck = 0;
						s32 i;
						
						for(i=0; i<pSPbManager->nMaxChnNum; i++)
						{
							if(pSPbManager->aPlaybackChnInfos[i].nRealFileNums != 0)
							{
								SendChnMsg(&pSPbManager->aPlaybackChnInfos[i],&sChnMsg,&nAck);
							}
						}
						
						pSPbManager->emCurCtlState = EM_CTL_STATE_RUN;
					}
				}
				break;
				default:
				{
					
				}
				break;
			}
		}
		
		if(emPbCtlCmd != EM_CTL_NULL)
		{
			AckCtlMsg(pSPbManager);
			
			//csp modify
			usleep(0);
			continue;
		}
		
PlayRunning:
		if(pSPbManager->emCurCtlState == EM_CTL_STATE_RUN)
		{
			//printf("nRefPTS=%llu,nForward=%d,nBaseInterval=%d,nRate=%d\n",pSPbManager->nRefPTS,pSPbManager->nForward,nBaseInterval,nRate);//csp modify 20150110
			
			#if 1//csp modify
			if(pSPbManager->nForward == -1)//倒放
			{
				//???//what are you doing
			}
			#endif
			
			if(nRefSystime == 0)
			{
				nRefSystime = GetTimeStamp();
				nArriveTime = nRefSystime;
				//printf("reset refSysTime *****\n");
			}
			
			nArriveTime += (u32)nInterval;
			
			u64 nCurTime = GetTimeStamp();
			
			//printf("nArriveTime=%llu, nCurTime=%llu\n", nArriveTime, nCurTime);
			
			if(nCurTime < nArriveTime)
			{
				//if(nArriveTime > nCurTime + JUMP_TIME)//csp modify
				if(nArriveTime > nCurTime + PLAY_TIME)//csp modify 20130415
				{
					printf("warning:play error??????\n");
					nRefSystime = 0;
					usleep(0);
					continue;
				}
				else
				{
					struct timeval sVal;
					sVal.tv_sec  = (nArriveTime - nCurTime)/1000;
					sVal.tv_usec = ((nArriveTime - nCurTime)%1000)*1000;
					select(0,NULL,NULL,NULL,&sVal);
				}
			}
			//printf("***nArriveTime=%llu, nCurTime=%llu\n", nArriveTime, GetTimeStamp());
			
			int nProg = 100*(pSPbManager->nRefPTS/1000-pSPbManager->nRecStartTime)/pSPbManager->nTotalTime;
			
			if(pbProgCB)
			{
				SPBCallbackInfo cbInfo;
				cbInfo.bStop = 0;
				cbInfo.nCurTime = pSPbManager->nRefPTS;
				cbInfo.nTotalTime = pSPbManager->nTotalTime;
				cbInfo.nProg = nProg;
				pbProgCB(&cbInfo);
			}
			
			if(pSPbManager->nForward == 1)
			{
				pSPbManager->nRefPTS += nBaseInterval;
			}
			else
			{
				pSPbManager->nRefPTS -= nBaseInterval;
				
				pSPbManager->nRefPTS -= 160;//csp modify//让倒放快一点
			}
			
			sChnMsg.emCmd = EM_CHN_UPDATEPTS;
			sChnMsg.unCxt.nRefPts = pSPbManager->nRefPTS;
			
			u64 nNewPts = (pSPbManager->nForward == 1) ? ~0 : 0;
			
			u64 nAck = 0;
			u32 i = 0;
			for(i=0; i<pSPbManager->nMaxChnNum; i++)
			{
				if(pSPbManager->aPlaybackChnInfos[i].nRealFileNums != 0)
				{
					//printf("line %d, chn[%d]\n", __LINE__, i);
					SendChnMsg(&pSPbManager->aPlaybackChnInfos[i],&sChnMsg,&nAck);
					//printf("line %d, chn[%d]\n", __LINE__, i);
					
					if((pSPbManager->nForward == 1 && nAck < nNewPts) || (pSPbManager->nForward == -1 && nAck > nNewPts))
					{
						nNewPts = nAck;
					}
					//printf("nNewPts=%llu, nAck=%llu, pSPbManager->nRefPTS=%llu\n",nNewPts, nAck, pSPbManager->nRefPTS);
				}
			}
			
			if(pSPbManager->nForward == 1)
			{
				if(nNewPts == ~0)
				{
					//printf("all files playback over-1!\n");
					StopPlayback(pSPbManager, 1);
					
					pSPbManager->emCurCtlState = EM_CTL_STATE_IDLE;
				}
				else
				{
					if((s64)(nNewPts-pSPbManager->nRefPTS) > JUMP_TIME)
					{
						//Seek(pSPbManager,(u32)(nNewPts/1000));
						pSPbManager->nRefPTS = nNewPts;
					}
				}
			}
			else
			{
				if(nNewPts == 0)
				{
					//printf("all files playback over-2!\n");
					StopPlayback(pSPbManager, 1);
					
					pSPbManager->emCurCtlState = EM_CTL_STATE_IDLE;
				}
				else
				{
					if((s64)(pSPbManager->nRefPTS-nNewPts) > JUMP_TIME)
					{
						//Seek(pSPbManager,(u32)(nNewPts/1000));
						pSPbManager->nRefPTS = nNewPts;
					}
					
					//printf("newpts=%lld\n",nNewPts);
				}
			}
		}
		else
		{
			usleep(40*1000);//usleep(100*1000);//csp modify
			//printf("usleep\n");
		}
	}
}

int CheckH264(u8 *pFrameData, int len)
{
	if (pFrameData && (len > 4))
	{
		if ((0 == pFrameData[0]) && (0 == pFrameData[1]) && (0 == pFrameData[2]) && (1 == pFrameData[3]))
		{
			return 1;
		}
	}
	return 0;
}

void* PBChnFunc(void* arg)
{
	if(arg == NULL)
	{
		return 0;
	}
	
	//printf("$$$$$$$$$$$$$$$$$$PBChnFunc id:%d\n",getpid());
	
	SPlaybackChnInfo *psPlaybackChnInfo = (SPlaybackChnInfo *)arg;
	
	s32 nChn = psPlaybackChnInfo->nChnNo;
	s64 nRefPts = 0;
	
	u32 nBufSize = 0;//zlb20111117  去掉部分malloc
//#ifdef CHIP_HISI3531
#if 1//csp modify 20140406
	nBufSize = 640<<10;
	if((1 << nChn) & ((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nChnMaskOfD1)
	{
		nBufSize = 640<<10;//nBufSize = 4*256*1024;//csp modify 20121116
	}
#else
	nBufSize = 80 * 1024;
	if((1 << nChn) & ((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nChnMaskOfD1)
	{
		nBufSize = 512*1024;//256*1024;//csp modify 20140302
	}
#endif
	//4SDI:1M buffer -> 512K buffer
	printf("PBChnFunc chn%d play bufsize:%d\n",nChn,nBufSize);
	
	//csp modify 20121016//这里需要验证//CHIP_HISI3531是否起作用?
	u8 pDecBuf[640<<10];
	
	char aFileName[32] = {0};
	u32 nOpenOffset = 0;
	
	vdec_stream_s sVds;
	audio_frame_t sAudioFrame;
	
	u8 nMediaType = 0;//0 video 1 audio
	u8 nKey = 0;
	u64 nPts = 0;
	u8 nTmpMediaType = 0;
	u64 nTmpPts = 0;
	u32 nStampTime = 0;
	s32 nRealSize = 0;
	
	//u64 nFlagSeekFirstFpts = 0;//csp modify
	
	SChnMSG sChnMsg;
	EmPBChnCmd emPbChnCmd;
	UnChnCxt unPbChnCxt;
	
	EmChnState emCurChnState = EM_CHN_STATE_IDLE;
	
	while(1)
	{
		switch(emCurChnState)
		{
			case EM_CHN_STATE_IDLE:
			{
				//printf("chn %d come into EM_CHN_STATE_IDLE\n",nChn);
				GetChnMsg(psPlaybackChnInfo, &sChnMsg, 1);
				emPbChnCmd = sChnMsg.emCmd;
				unPbChnCxt = sChnMsg.unCxt;
				
				if(emPbChnCmd == EM_CHN_PLAY)
				{
					//zlb20111117  去掉部分malloc
					/*nBufSize = 80*1024;
					if((1 << nChn) & ((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nChnMaskOfD1)
					{
						nBufSize = 256*1024;
					}
					if(!(pDecBuf = malloc(nBufSize)))
					{
						printf("malloc decBuffer error\n");
						continue;
					}*/
					
					emCurChnState = EM_CHN_STATE_WAIT;
					
					nTmpMediaType = 0;
					nTmpPts = 0;
					
					psPlaybackChnInfo->nIsPlaying = 0;
					
					//csp modify
					psPlaybackChnInfo->nMediaFrameReady = 0;
				}
				
				AckChnMsg(psPlaybackChnInfo);
			}
			break;
			case EM_CHN_STATE_WAIT:
			{
				GetChnMsg(psPlaybackChnInfo, &sChnMsg, 1);
				emPbChnCmd = sChnMsg.emCmd;
				unPbChnCxt = sChnMsg.unCxt;
				
				if(emPbChnCmd == EM_CHN_UPDATEPTS)
				{
					nRefPts = unPbChnCxt.nRefPts;
					
					if(psPlaybackChnInfo->nMediaFrameReady)
					{
						psPlaybackChnInfo->nPbChnAck = psPlaybackChnInfo->nCurFrameStamp;
					}
					else
					{
						if(psPlaybackChnInfo->nForward == 1)
						{
							if(psPlaybackChnInfo->nCurIndex < 0)
							{
								psPlaybackChnInfo->nPbChnAck = ~0;
								
								emCurChnState = EM_CHN_STATE_WAIT;
								
								AckChnMsg(psPlaybackChnInfo);
								
								continue;
							}
							else
							{
								psPlaybackChnInfo->nPbChnAck = nRefPts;
								//printf("%d chn, ack %lld\n",nChn,nRefPts);
							}
						}
						else
						{
							if(psPlaybackChnInfo->nCurIndex >= psPlaybackChnInfo->nRealFileNums)
							{
								psPlaybackChnInfo->nPbChnAck = 0;
								
								emCurChnState = EM_CHN_STATE_WAIT;
								
								AckChnMsg(psPlaybackChnInfo);
								
								continue;
							}
							else
							{
								psPlaybackChnInfo->nPbChnAck = nRefPts;
							}
						}
					}	
					
					//printf("chn %d,psPlaybackChnInfo->nPbChnAck=%lld \n",nChn,psPlaybackChnInfo->nPbChnAck);
					
					emCurChnState = EM_CHN_STATE_PLAY;
					
					AckChnMsg(psPlaybackChnInfo);
				}
				else if(emPbChnCmd == EM_CHN_STOP)//else if(emPbChnCmd == EM_CTL_STOP)//csp modify 20130415
				{
					//printf("wait emPbChnCmd STOP\n");
					
					if(psPlaybackChnInfo->pFile != NULL)
					{
						custommp4_close(psPlaybackChnInfo->pFile);
						psPlaybackChnInfo->pFile = NULL;
					}
					
					//csp modify
					psPlaybackChnInfo->nIsPlaying = 0;
					psPlaybackChnInfo->nMediaFrameReady = 0;
					
					//zlb20111117  去掉部分malloc
					/*if(pDecBuf != NULL)
					{
						free(pDecBuf);
						pDecBuf = NULL;
					}*/
					
					nRefPts = 0;
					
					emCurChnState = EM_CHN_STATE_IDLE;
					
					AckChnMsg(psPlaybackChnInfo);
				}
				else if(emPbChnCmd == EM_CHN_BACKWARD)
				{
					psPlaybackChnInfo->nForward = -1;
					
					AckChnMsg(psPlaybackChnInfo);
					
					continue;
				}
				else if(emPbChnCmd == EM_CHN_FORWARD)
				{
					psPlaybackChnInfo->nForward = 1;
					
					AckChnMsg(psPlaybackChnInfo);
					
					continue;
				}
				else if(emPbChnCmd == EM_CHN_SEEK)
				{
					//printf("line:%d, time:%llu\n", __LINE__, GetTimeStamp());
					u32 nSeekTime = unPbChnCxt.nSeekTime;
					Seek(psPlaybackChnInfo, nSeekTime);
					//printf("line:%d, time:%llu\n", __LINE__, GetTimeStamp());
					AckChnMsg(psPlaybackChnInfo);
					continue;
				}
				else//csp modify
				{
					printf("warning:EM_CHN_STATE_WAIT recv cmd:%d\n",emPbChnCmd);
					AckChnMsg(psPlaybackChnInfo);
					continue;
				}
			}
			break;
			case EM_CHN_STATE_PLAY:
			{
				printf("%d EM_CHN_STATE_PLAY\n", nChn);
				if(GetChnMsg(psPlaybackChnInfo, &sChnMsg, 0) == 0)
				{
					emPbChnCmd = sChnMsg.emCmd;
					unPbChnCxt = sChnMsg.unCxt;
					
					if(emPbChnCmd == EM_CHN_STOP)
					{
						if(psPlaybackChnInfo->pFile != NULL)
						{
							custommp4_close(psPlaybackChnInfo->pFile);
							psPlaybackChnInfo->pFile = NULL;
						}
						
						//csp modify
						psPlaybackChnInfo->nIsPlaying = 0;
						psPlaybackChnInfo->nMediaFrameReady = 0;
						
						//zlb20111117  去掉部分malloc
						/*if(pDecBuf != NULL)
						{
							free(pDecBuf);
							pDecBuf = NULL;
						}*/
						
						nRefPts = 0;
						
						emCurChnState = EM_CHN_STATE_IDLE;
						
						AckChnMsg(psPlaybackChnInfo);
					}
					else if(emPbChnCmd == EM_CHN_PAUSE)
					{
						emCurChnState = EM_CHN_STATE_WAIT;
						
						AckChnMsg(psPlaybackChnInfo);
					}	
					else if(emPbChnCmd == EM_CHN_UPDATEPTS)
					{
						nRefPts = unPbChnCxt.nRefPts;
						
						psPlaybackChnInfo->nPbChnAck = nRefPts;
						//printf("line %d\n", __LINE__);
						
						AckChnMsg(psPlaybackChnInfo);
					}
					else if(emPbChnCmd == EM_CHN_BACKWARD)
					{
						printf("%s emPbChnCmd == EM_CHN_BACKWARD\n", __func__);
						psPlaybackChnInfo->nForward = -1;
						
						AckChnMsg(psPlaybackChnInfo);
						
						continue;
					}
					else if(emPbChnCmd == EM_CHN_FORWARD)
					{
						psPlaybackChnInfo->nForward = 1;
						
						AckChnMsg(psPlaybackChnInfo);
						
						continue;
					}
					else if(emPbChnCmd == EM_CHN_SEEK)
					{
						//printf("line:%d, time:%llu\n", __LINE__, GetTimeStamp());
						u32 nSeekTime = unPbChnCxt.nSeekTime;
						Seek(psPlaybackChnInfo, nSeekTime);
						//printf("line:%d, time:%llu\n", __LINE__, GetTimeStamp());
						AckChnMsg(psPlaybackChnInfo);
						continue;
					}
					else//csp modify
					{
						printf("warning:EM_CHN_STATE_PLAY recv cmd:%d\n",emPbChnCmd);
						AckChnMsg(psPlaybackChnInfo);
						continue;
					}
				}
			}
			break;
			default:
			{
				printf("warning:emCurChnState:%d\n",emCurChnState);
			}
			break;
		}
		
		//if(nChn == 0) printf("chn%d emCurChnState=%d\n",nChn,emCurChnState);
		
		if(emCurChnState == EM_CHN_STATE_PLAY)
		{
			if(psPlaybackChnInfo->nMediaFrameReady)
			{
				//csp modify
				//u32 nCurIndex = psPlaybackChnInfo->nCurIndex;
				//u64 nCurFileStartTime = (u64)psPlaybackChnInfo->aRecFileInfos[nCurIndex].nStartTime*1000;
				
				psPlaybackChnInfo->nCurFrameStamp = nPts/1000;
				
				if((psPlaybackChnInfo->nForward == 1 && psPlaybackChnInfo->nCurFrameStamp <= nRefPts)
					|| (psPlaybackChnInfo->nForward == -1 && psPlaybackChnInfo->nCurFrameStamp >= nRefPts))
				{
					//if(nChn == 0)
					//	printf("hehe1 - player%d ready, CurFrameStamp = %lld, nRefPts = %lld\n", nChn, psPlaybackChnInfo->nCurFrameStamp, nRefPts);
					
					psPlaybackChnInfo->nIsPlaying = 1;
					
					if(nMediaType)//audio
					{
						if((0 == ((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nIsVOIP) 
							&& (!psPlaybackChnInfo->nMute) 
							&& ((psPlaybackChnInfo->nFlagZoom == 1) || ((psPlaybackChnInfo->nFlagZoom == 2) && (psPlaybackChnInfo->nPlayNo == 0))))
						{
							if((psPlaybackChnInfo->emRate == EM_PLAYRATE_1) && (psPlaybackChnInfo->nForward == 1))
							{
								sAudioFrame.data = pDecBuf;
								sAudioFrame.len = nRealSize;
								if(nRealSize < 200)
								{
									sAudioFrame.is_raw_data = 0;
								}
								else
								{
									sAudioFrame.is_raw_data = 1;
								}
								tl_audio_write_data(&sAudioFrame);
								//printf("*****1 receive one audio frame!\n");
							}
						}
						
						psPlaybackChnInfo->nMediaFrameReady = 0;
						
						//continue;//csp modify 20130415
					}
					else//video//csp modify 20130415
					{
						sVds.data = pDecBuf;
						sVds.len = nRealSize;
						sVds.pts = 0;
						sVds.rsv = 0;
						
					#if 0//defined(CHIP_HISI3531)//csp modify 20130509
						if(1)
					#else
						if(psPlaybackChnInfo->nFlagZoom != 0)//csp modify//节省性能
					#endif
						{
							s32 nPlayNo = psPlaybackChnInfo->nPlayNo;
							
							//csp modify
							sVds.rsv = (psPlaybackChnInfo->nVWidth << 16) | psPlaybackChnInfo->nVHeight;
							
							//printf("1 before tl_vdec_write, len:%d, data:(%02x %02x %02x %02x %02x)\n", sVds.len, sVds.data[0], sVds.data[1], sVds.data[2], sVds.data[3], sVds.data[4]);
							int rett = 0;
							rett = tl_vdec_write(nPlayNo,&sVds);
							//printf("1 chn[%d] tl_vdec_write result:%d len:%d\n", nChn, rett, sVds.len);
							//printf("sVds.len = %u, sVds.pts = %llu, rett = %d\n", sVds.len, sVds.pts, rett);
						}
						else//csp modify
						{
							psPlaybackChnInfo->nFlagDoSeek = 1;
						}
						
						psPlaybackChnInfo->nMediaFrameReady = 0;
					}
				}
				else
				{
					emCurChnState = EM_CHN_STATE_WAIT;
					//if(nChn == 0) printf("chn%d EM_CHN_STATE_WAIT:%lld,%lld\n",nChn,psPlaybackChnInfo->nCurFrameStamp,nRefPts);
					
					continue;//csp modify 20130415
				}
			}
			
CHECK_FOLLOW_FRAMES:
			if(!psPlaybackChnInfo->nMediaFrameReady)
			{
				//csp modify 20130415
				int value = -1;
				sem_getvalue(&psPlaybackChnInfo->semSendChnFinish, &value);
				if(value > 0 && psPlaybackChnInfo->emPbChnCmd == EM_CHN_STOP)
				{
					continue;
				}
				
				//nFlagSeekFirstFpts = 0;
				
				s32 seconds = 1, i = 0;//xdc modify 0715
				
				s32 nRetry = 0;
				for(nRetry = 0; nRetry < 3; nRetry++)
				{
					if(psPlaybackChnInfo->pFile)
					{
						if(psPlaybackChnInfo->nForward == -1)
						{
							//printf("1 seek_to_prev_key_frame nRetry: %d\n", nRetry);
							if(custommp4_seek_to_prev_key_frame(psPlaybackChnInfo->pFile) == 0)
							{
								printf("chn %d,custommp4_seek_to_prev_key_frame failed\n",nChn);
								continue;
							}
							seconds = 4;//xdc modify 0715
							//printf("1 seek_to_prev_key_frame nRetry: %d, seconds: %d\n", nRetry, seconds);
						}
						else//csp modify 20130429
						{
							//if(psPlaybackChnInfo->emRate > EM_PLAYRATE_8 || psPlaybackChnInfo->nFlagDoSeek)
							if(psPlaybackChnInfo->emRate > EM_PLAYRATE_8)
							{
								if(custommp4_seek_to_next_key_frame(psPlaybackChnInfo->pFile) == 0)
								{
									//printf("chn %d,custommp4_seek_to_next_key_frame failed\n",nChn);
									continue;
								}
								else
								{
									//printf("chn %d,custommp4_seek_to_next_key_frame success\n",nChn);
								}
								seconds = 4;//xdc modify 0715
							}
						}
						
						#if 1
						//printf("seconds = %d\n",seconds);
						u8 *pbuf = pDecBuf;//xdc modify 0715
						s32 realsize = nBufSize;//xdc modify 0715
						for(i=0;i<seconds;i++)//xdc modify 0715
						{
							//if(realsize > 0)
							//{
								nRealSize = custommp4_read_one_media_frame(psPlaybackChnInfo->pFile, 
																		pbuf,//pDecBuf
																		realsize,//nBufSize
																		&nStampTime, 
																		&nKey, 
																		&nPts, 
																		&nMediaType);
							//}
							//printf("read_one_media_frame i: %d, nRealSize: %d\n", i, nRealSize);
							if(nRealSize <= 0)
							{
								realsize = nBufSize;
								break;
							}
							
							if(seconds == 4 && nMediaType)//audio
							{
								i--;
								continue;
							}
							
							pbuf = pbuf + nRealSize;
							realsize = realsize - nRealSize;
							
							if(realsize == 0)
							{
								break;
							}
							
							if(seconds == 4 && nRealSize > 128)
							{
								break;
							}
						}
						nRealSize = nBufSize - realsize;//xdc modify 0715
						if(seconds == 4)
						{
							nKey = 1;//xdc modify 0715
						}
						//printf("nRealSize = %d\n",nRealSize);
						#else
						nRealSize = custommp4_read_one_media_frame(psPlaybackChnInfo->pFile, 
																		pDecBuf, 
																		nBufSize, 
																		&nStampTime, 
																		&nKey, 
																		&nPts, 
																		&nMediaType);
						#endif						
						if(nRealSize <= 0)
						{
							//printf("custommp4_read_one_media_frame error,retry...playIndex:%d\n",psPlaybackChnInfo->nChnNo);
							nRetry = 3;//csp modify
							continue;
						}
						
						//回放正在录着的文件时会出现这种情况
						//csp:需要做这个限制吗?有可能有些没有正常结束的录像文件回放不完全
						if(psPlaybackChnInfo->aRecFileInfos[psPlaybackChnInfo->nCurIndex].nEndTime + 1 <= nPts / 1000000)
						{
							//printf("play warning 1\n");
							
							#if 0//csp modify
							nRetry = 3;
							continue;
							#endif
						}
						
						//printf("nEndTime[%ld], nPts[%llu]\n",psPlaybackChnInfo->aRecFileInfos[psPlaybackChnInfo->nCurIndex].nEndTime, nPts / 1000000);
						
						#if 1//csp modify
						//到点了结束回放
						if((psPlaybackChnInfo->nForward == 1) && (psPlaybackChnInfo->aRecFileInfos[psPlaybackChnInfo->nCurIndex].nStartTime + nStampTime / 1000 > ((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nRecEndTime))
						{
							//printf("play warning 1.1\n");
							nRetry = 3;
							continue;
						}
						//到点了结束回放
						if((psPlaybackChnInfo->nForward == -1) && nPts / 1000000 < ((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nRecStartTime)
						{
							//printf("play warning 1.2\n");
							nRetry = 3;
							continue;
						}
						#endif
						
						//csp modify
						//csp:什么情况下会出现这样的情况
						//只有音频帧才会出现这个问题，音频帧的pts为0
						//系统10分钟调整一次tlsdk的参考时间是否有可能出现这种问题???
						//用户修改RTC时间时是否会造成这个问题???
						if((nTmpMediaType == nMediaType) && (nTmpPts == nPts))//有些文件是可能有这种问题的
						//if((nTmpMediaType == nMediaType) && (nTmpPts >= nPts))//有些文件是可能有这种问题的
						{
							//printf("player%d warning 2 - [%d,%u]\n",nChn,nMediaType,(u32)nPts);
							
							#if 0//csp modify
							nRetry--;
							continue;
							#endif
						}
						
						//丢弃1秒前的帧
						#if 1//csp modify
						if(psPlaybackChnInfo->nForward == 1)
						{
							if(nRefPts > nPts/1000 + 1000)
							{
								//printf("player%d warning 2.1\n",nChn);
								psPlaybackChnInfo->nFlagDoSeek = 1;//丢了帧所以需要定位到下一个关键帧，否则会花屏
								nRetry--;
								continue;
							}
						}
						else
						{
							if(nPts/1000 > nRefPts + 1000)
							{
								//printf("play warning 2.2\n");
								//psPlaybackChnInfo->nFlagDoSeek = 1;//倒放的每一帧都是关键帧，所以不需要置位
								nRetry--;
								continue;
							}
						}
						#else
						if(nRefPts > nPts/1000 + 1000)
						{
							nRetry--;
							continue;
						}
						#endif
						
						if(nMediaType)
						{
							//continue;//不要屏蔽音频
							
							//csp modify//why???//音频帧不可能这么大
							if(nRealSize > 1024)
							{
								printf("Invalid audio frame!!!!!!! Be Covered\n");
								nRetry = 3;
								continue;
							}
						}
						else
						{
							if(!CheckH264(pDecBuf, nRealSize))
							{
								printf("Invalid frame data!!!!!!! Be Covered\n");
								nRetry = 3;
								continue;
							}
							
							if(psPlaybackChnInfo->nFlagDoSeek)//偏移到I帧,防止花屏
							{
								if(!IsIframe(nKey))
								{
									//printf("play warning 2.3\n");
									nRetry--;
									continue;
								}
								else
								{
									psPlaybackChnInfo->nFlagDoSeek = 0;
								}
							}
						}
						
						nTmpMediaType = nMediaType;
						nTmpPts = nPts;
						
						break;
					}
				}
				
				//printf("nStampTime[%ld], nPts[%llu]\n",nStampTime, nPts);
				
				if(nRetry >= 3)
				{
					if(psPlaybackChnInfo->nForward == 1)
					{
						if(psPlaybackChnInfo->nCurIndex <= 0)
						{
							//printf("psPlaybackChnInfo->nCurIndex = %d\n",psPlaybackChnInfo->nCurIndex);
							//printf("ready %d\n",psPlaybackChnInfo->nMediaFrameReady);
							//nStop = 1;
							
							#if 1//csp modify
							psPlaybackChnInfo->nCurIndex = -1;
							#else
							//if(psPlaybackChnInfo->nCurIndex == 0)
							//{
								psPlaybackChnInfo->nCurIndex--;
							//}
							#endif
							
							psPlaybackChnInfo->nMediaFrameReady = 0;
							
							if(psPlaybackChnInfo->pFile != NULL)
							{
								custommp4_close(psPlaybackChnInfo->pFile);
								psPlaybackChnInfo->pFile = NULL;
							}
							
							//zlb20111117  去掉部分malloc
							/*if(pDecBuf != NULL)
							{
								free(pDecBuf);
								pDecBuf = NULL;
							}*/
							
							psPlaybackChnInfo->nIsPlaying = 0;
							
							emCurChnState = EM_CHN_STATE_WAIT;
							
							continue;
						}
						
						u32 nCurIndex = --psPlaybackChnInfo->nCurIndex;
						get_rec_file_name((recfileinfo_t*)&psPlaybackChnInfo->aRecFileInfos[nCurIndex], 
											aFileName, 
											&nOpenOffset);
						
						if(psPlaybackChnInfo->pFile != NULL)
						{
							custommp4_close(psPlaybackChnInfo->pFile);
							psPlaybackChnInfo->pFile = NULL;
						}
						
						#ifndef PLAY_OPT
						GetFileStepTime(aFileName, nOpenOffset, &psPlaybackChnInfo->nFrameStepTime);
						#endif
						
						psPlaybackChnInfo->pFile = custommp4_open(aFileName, O_R, nOpenOffset);
						if(psPlaybackChnInfo->pFile == NULL)
						{
							printf("custommp4_open error,chn %d\n",nChn);
							continue;//也尝试5次可能比较好,出现过双击单个文件第一次未成功,第2次却成功
						}
						
						#ifdef PLAY_OPT
						GetFileStepTime_fast(psPlaybackChnInfo->pFile, &psPlaybackChnInfo->nFrameStepTime);
						//printf("@@@@@@@@@@@@@@@@nChn = %d FrameStepTime = %u\n", nChn, psPlaybackChnInfo->nFrameStepTime);
						#endif
						
						//csp modify
						psPlaybackChnInfo->nVWidth = custommp4_video_width(psPlaybackChnInfo->pFile);
						psPlaybackChnInfo->nVHeight = custommp4_video_height(psPlaybackChnInfo->pFile);
						
						//csp modify
						//此处按道理是需要seek到回放的起始时间的，因为有可能用户修改了时间，造成同一段时间有好几个录像文件
						
						psPlaybackChnInfo->nMediaFrameReady = 0;
						
						//printf("&&&&&&&&&&&&&&&&&&& nStartTime=%d, nRefPts=%lld,format = %d\n", psPlaybackChnInfo->aRecFileInfos[nCurIndex].nStartTime,nRefPts,psPlaybackChnInfo->aRecFileInfos[nCurIndex].nFormat);
						if(psPlaybackChnInfo->aRecFileInfos[nCurIndex].nStartTime >= nRefPts/1000 + 1)
						{
							psPlaybackChnInfo->nIsPlaying = 0;
						}
						
						//csp modify 20130415
						continue;
						//goto CHECK_FOLLOW_FRAMES;
					}
					else
					{
						#if 1//csp modify
						if(psPlaybackChnInfo->nCurIndex >= psPlaybackChnInfo->nRealFileNums-1)
						#else
						//if(psPlaybackChnInfo->nCurIndex == psPlaybackChnInfo->nRealFileNums-1)
						if(1)//倒放有问题，暂时不用了
						#endif
						{
							#if 1//csp modify
							psPlaybackChnInfo->nCurIndex = psPlaybackChnInfo->nRealFileNums;
							#else
							psPlaybackChnInfo->nCurIndex++;
							#endif
							
							psPlaybackChnInfo->nMediaFrameReady = 0;
							
							if(psPlaybackChnInfo->pFile != NULL)
							{
								custommp4_close(psPlaybackChnInfo->pFile);
								psPlaybackChnInfo->pFile = NULL;
							}
							
							//zlb20111117 去掉malloc
							/*if(pDecBuf != NULL)
							{
								free(pDecBuf);
								pDecBuf = NULL;
							}*/
							
							emCurChnState = EM_CHN_STATE_WAIT;
							
							psPlaybackChnInfo->nIsPlaying = 0;
							
							continue;
						}
						
						s32 nCurIndex = ++psPlaybackChnInfo->nCurIndex;
						get_rec_file_name((recfileinfo_t*)&psPlaybackChnInfo->aRecFileInfos[nCurIndex], 
											aFileName, 
											&nOpenOffset);
						
						if(psPlaybackChnInfo->pFile != NULL)
						{
							custommp4_close(psPlaybackChnInfo->pFile);
							psPlaybackChnInfo->pFile = NULL;
						}
						
						#ifndef PLAY_OPT
						GetFileStepTime(aFileName, nOpenOffset, &psPlaybackChnInfo->nFrameStepTime);
						//printf("&&&&&&&&&&&&&&&&&&& nStartTime=%d, nRefPts=%lld,format = %d\n", psPlaybackChnInfo->aRecFileInfos[nCurIndex].nStartTime,nRefPts,psPlaybackChnInfo->aRecFileInfos[nCurIndex].nFormat);
						#endif
						
						psPlaybackChnInfo->pFile = custommp4_open(aFileName, O_R, nOpenOffset);
						if(psPlaybackChnInfo->pFile == NULL)
						{
							printf("custommp4_open error,chn %d\n",nChn);
							continue;
						}
						
						#ifdef PLAY_OPT
						GetFileStepTime_fast(psPlaybackChnInfo->pFile, &psPlaybackChnInfo->nFrameStepTime);
						#endif
						
						//csp modify
						psPlaybackChnInfo->nVWidth = custommp4_video_width(psPlaybackChnInfo->pFile);
						psPlaybackChnInfo->nVHeight = custommp4_video_height(psPlaybackChnInfo->pFile);
						
						u32 nFileEndTime = psPlaybackChnInfo->aRecFileInfos[nCurIndex].nEndTime;
						
						#if 1//csp modify
						if(nFileEndTime > ((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nRecEndTime)
						{
							nFileEndTime = ((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nRecEndTime;
						}
						int rtn = 0;
						while(1)
						{
							rtn = custommp4_seek_to_sys_time(psPlaybackChnInfo->pFile,nFileEndTime);
							if(rtn < 0)//seek失败
							{
								break;
							}
							else if(rtn > 0)//seek成功
							{
								break;
							}
							else//if(rtn == 0)//没有seek到关键帧
							{
								nFileEndTime--;
								if(nFileEndTime < psPlaybackChnInfo->aRecFileInfos[nCurIndex].nStartTime)
								{
									break;
								}
							}
						}
						if(rtn <= 0)
						{
							custommp4_close(psPlaybackChnInfo->pFile);
							psPlaybackChnInfo->pFile = NULL;
							continue;
						}
						#else
						if(custommp4_seek_to_sys_time(psPlaybackChnInfo->pFile,nFileEndTime) < 0)
						{
							custommp4_close(psPlaybackChnInfo->pFile);
							psPlaybackChnInfo->pFile = NULL;
							continue;
						}
						#endif
						
						if(psPlaybackChnInfo->aRecFileInfos[nCurIndex].nEndTime <= nRefPts/1000 - 1)
						{
							psPlaybackChnInfo->nIsPlaying = 0;
						}
						
						psPlaybackChnInfo->nMediaFrameReady = 0;
						
						//csp modify 20130415
						continue;
						//goto CHECK_FOLLOW_FRAMES;
					}
				}
				
				psPlaybackChnInfo->nMediaFrameReady = 1;
				
				//if(nChn == 0) printf("data ready\n");
				
				psPlaybackChnInfo->nCurFrameStamp = nPts/1000;
				
				//s32 nCurIndex = psPlaybackChnInfo->nCurIndex;//csp modify
				
				if((psPlaybackChnInfo->nForward==1 && psPlaybackChnInfo->nCurFrameStamp<=nRefPts)
					|| (psPlaybackChnInfo->nForward==-1 && psPlaybackChnInfo->nCurFrameStamp>=nRefPts))
				{
					//if(nChn == 0)
					//	printf("hehe2 - player%d ready, CurFrameStamp = %lld, nRefPts = %lld\n", nChn, psPlaybackChnInfo->nCurFrameStamp, nRefPts);
					
					psPlaybackChnInfo->nIsPlaying = 1;
					
					if(nMediaType)//audio
					{
						if((0 == ((SPlayBackManager*)(psPlaybackChnInfo->pManagerParent))->nIsVOIP) 
							&& (!psPlaybackChnInfo->nMute) 
							&& ((psPlaybackChnInfo->nFlagZoom == 1) || ((psPlaybackChnInfo->nFlagZoom == 2) && (psPlaybackChnInfo->nPlayNo == 0))))
						{
							if((psPlaybackChnInfo->emRate == EM_PLAYRATE_1)&&(psPlaybackChnInfo->nForward == 1))
							{
								sAudioFrame.data = pDecBuf;
								sAudioFrame.len = nRealSize;
								if(nRealSize < 200)
								{
									sAudioFrame.is_raw_data = 0;
								}
								else
								{
									sAudioFrame.is_raw_data = 1;
								}
								tl_audio_write_data(&sAudioFrame);
								//printf("*****2 receive one audio frame!\n");
							}
						}
					}
					else//video
					{
						sVds.data = pDecBuf;
						sVds.len = nRealSize;
						sVds.pts = 0;
						sVds.rsv = 0;
						
					#if 0//defined(CHIP_HISI3531)//csp modify 20130509
						if(1)
					#else
						if(psPlaybackChnInfo->nFlagZoom != 0)//csp modify//节省性能
					#endif
						{
							s32 nPlayNo = psPlaybackChnInfo->nPlayNo;
							
							//csp modify
							sVds.rsv = (psPlaybackChnInfo->nVWidth << 16) | psPlaybackChnInfo->nVHeight;
							
							//printf("2 before tl_vdec_write, len:%d, line:%d\n", sVds.len, __LINE__);
							int rett = 0;
							rett = tl_vdec_write(nPlayNo,&sVds);
							//printf("2 chn[%d] tl_vdec_write result2:%d len:%d\n", nChn, rett, sVds.len);
							//printf("sVds.len = %u, sVds.pts = %llu, rett = %d\n", sVds.len, sVds.pts, rett);
						}
						else//csp modify
						{
							psPlaybackChnInfo->nFlagDoSeek = 1;
						}
						//printf("chn[%d] tl_vdec_write &&&&&\n", nChn);
					}
					
					psPlaybackChnInfo->nMediaFrameReady = 0;
					
					goto CHECK_FOLLOW_FRAMES;
				}
				else
				{
					emCurChnState = EM_CHN_STATE_WAIT;
					
					continue;//csp modify 20130415
				}
			}
		}
	}
}

PbMgrHandle ModPlayBackInit(u32 nMaxChn, void* pContent, u64 nChnMaskOfD1)
{
	if((!pContent))
	{
		return NULL;
	}
	
	printf("ModPlayBackInit:nMaxChn=%d,nChnMaskOfD1=0x%08x\n",nMaxChn,(u32)nChnMaskOfD1);
	
	gp_sHddManager = (disk_manager*)pContent;
	//memcpy(&g_sHddManager, pContent, sizeof(g_sHddManager));
	
	SPlayBackManager* pSPbManager = NULL;
	if(!(pSPbManager = malloc(sizeof(SPlayBackManager))))
	{
		printf("malloc SPlayBackManager error\n");
		return NULL;
	}
	memset(pSPbManager, 0x0, sizeof(SPlayBackManager));
	
	if(!(pSPbManager->aPlaybackChnInfos = malloc(nMaxChn*sizeof(SPlaybackChnInfo))))
	{
		printf("malloc aPlaybackChnInfos error\n");
		return NULL;
	}
	memset(pSPbManager->aPlaybackChnInfos, 0x0, nMaxChn*sizeof(SPlaybackChnInfo));//csp modify
	
	sem_init(&pSPbManager->semPbCtlFinish, 0, 0);
	//sem_init(&pSPbManager->semSendCtlFinish, 0, 0);
	
	pthread_mutex_init(&pSPbManager->lock, NULL);//csp modify
	
	u32 i = 0;
	for(i=0; i<nMaxChn; i++)
	{
		sem_init(&pSPbManager->aPlaybackChnInfos[i].semPbChnFinish, 0, 0);
		sem_init(&pSPbManager->aPlaybackChnInfos[i].semSendChnFinish, 0, 0);
	}
	
	pSPbManager->nMaxChnNum = nMaxChn;
	pSPbManager->nChnMaskOfD1 = nChnMaskOfD1;
	
	#ifdef DEC_USE_ONE_SAME_BUF
	if(!(pSPbManager->pDecBuf = calloc(256<<10, 1)))
	{
		printf("malloc pDecBuf error\n");
		return NULL;
	}
	sem_init(&pSPbManager->semDecBuf, 0, 1);
	#endif
	
	TL_CreateThread(PBCtlFunc, NULL, PRI_DECODE, (512<<10), (u32)pSPbManager, 0, NULL);
	
	for(i=0; i<nMaxChn; i++)
	{
		pSPbManager->aPlaybackChnInfos[i].nChnNo = i;
		pSPbManager->aPlaybackChnInfos[i].pManagerParent = (void*)pSPbManager;
		
		//csp modify
		//TL_CreateThread(PBChnFunc, NULL, PRI_DECODE, (512<<10), (u32)&pSPbManager->aPlaybackChnInfos[i], 0, NULL);
		pthread_t pid;
		pthread_create(&pid,NULL,PBChnFunc,(void *)&pSPbManager->aPlaybackChnInfos[i]);
	}
	
	//init_disk_manager(&g_sHddManager);
	
	return (PbMgrHandle)pSPbManager;
}

s32 ModPlayBackDeinit(PbMgrHandle hPbMgr)
{
	if(!hPbMgr)
	{
		return -1;
	}
	
	SPlayBackManager* pSPbManager = (SPlayBackManager*)hPbMgr;

	if(pSPbManager->aPlaybackChnInfos)
	{
		free(pSPbManager->aPlaybackChnInfos);
		pSPbManager->aPlaybackChnInfos = NULL;
	}

	sem_destroy(&pSPbManager->semPbCtlFinish);

	u32 nMaxChn = pSPbManager->nMaxChnNum;
	u32 i = 0;
	for(i=0; i<nMaxChn; i++)
	{
		sem_destroy(&pSPbManager->aPlaybackChnInfos[i].semPbChnFinish);
		sem_destroy(&pSPbManager->aPlaybackChnInfos[i].semSendChnFinish);
	}

	free(pSPbManager);
	pSPbManager = NULL;
	
	return 0;
}

s32 ModPlayBackByTime(PbMgrHandle hPbMgr, SPBSearchPara* pSearchParam)
{
	if(!hPbMgr || !pSearchParam)
	{
		return -1;
	}
	
	SPlayBackManager* pSPbManager = (SPlayBackManager*)hPbMgr;
	
	SCtlMSG sCtlMsg;
	sCtlMsg.emCmd = EM_CTL_PBTIME;
	sCtlMsg.unCxt.sSearchParam = *pSearchParam;
	//DEBUG("sSearchParam.nStartTime = %d\n",sCtlMsg.unCxt.sSearchParam.nStartTime);
	//DEBUG("sSearchParam.nEndTime = %d\n",sCtlMsg.unCxt.sSearchParam.nEndTime);
	u32 ack;
	SendCtlMsg(pSPbManager,&sCtlMsg,&ack);
	
	return 0;
}

s32 ModPlayBackByFile(PbMgrHandle hPbMgr,SPBRecfileInfo* pFileInfo)
{
	if(!hPbMgr || !pFileInfo)
	{
		return -1;
	}
	
	SPlayBackManager* pSPbManager = (SPlayBackManager*)hPbMgr;
	
	SCtlMSG sCtlMsg;
	sCtlMsg.emCmd = EM_CTL_PBFILE;
	memcpy(&sCtlMsg.unCxt.sFileInfo, pFileInfo, sizeof(SPBRecfileInfo));
	u32 ack;
	SendCtlMsg(pSPbManager,&sCtlMsg,&ack);
	
	return 0;
}

s32 ModPlayBackControl(PbMgrHandle hPbMgr, EmPBCtlCmd emPBCtlcmd, s32 nContext)
{
	if(!hPbMgr)
	{
		return -1;
	}
	
	SPlayBackManager* pSPbManager = (SPlayBackManager*)hPbMgr;
	
	SCtlMSG sCtlMsg;
	sCtlMsg.emCmd = emPBCtlcmd;
	
	//printf("ModPlayBackControl:emPBCtlcmd=%d start...\n",emPBCtlcmd);
	
	if(emPBCtlcmd == EM_CTL_SET_SPEED)
	{
		sCtlMsg.unCxt.emPlayRate = nContext;
		printf("ModPlayBackControl:EM_CTL_SET_SPEED=%d,emPlayRate=%d\n",emPBCtlcmd,sCtlMsg.unCxt.emPlayRate);
	}
	else if(emPBCtlcmd == EM_CTL_SEEK)
	{
		sCtlMsg.unCxt.nSeekTime = nContext;
	}
	else if(emPBCtlcmd == EM_CTL_MUTE)
	{
		sCtlMsg.unCxt.nMute = nContext;
	}
	//csp modify
	else if(emPBCtlcmd == EM_CTL_BACKWARD)
	{
		printf("ModPlayBackControl:EM_CTL_BACKWARD=%d\n",emPBCtlcmd);
	}
	else if(emPBCtlcmd == EM_CTL_FORWARD)
	{
		printf("ModPlayBackControl:EM_CTL_FORWARD=%d\n",emPBCtlcmd);
	}
	//csp modify 20130509
	#if defined(CHIP_HISI3531)
	else if(emPBCtlcmd == EM_CTL_PAUSE)
	{
		
	}
	else if(emPBCtlcmd == EM_CTL_RESUME)
	{
		
	}
	else if(emPBCtlcmd == EM_CTL_STEP)
	{
		if(nContext == 0xfd)
		{
			tl_vdec_ioctl(0, 0xf0, NULL);//CMD_VDEC_REFRESH
			return 0;
		}
	}
	else if(emPBCtlcmd == EM_CTL_STOP)
	{
		
	}
	#endif
	
	u32 ack;
	SendCtlMsg(pSPbManager,&sCtlMsg,&ack);
	
	//printf("ModPlayBackControl:emPBCtlcmd=%d over\n",emPBCtlcmd);
	
	return 0;
}

s32 ModPlayBackProgress(PBPROGFUNC pbProgFunc)
{
	if(pbProgFunc)
	{
		pbProgCB = pbProgFunc;
		
		return 0;
	}
	
	return -1;
}

s32 ModPlayBackRegistFunCB(u8 type, PBREGFUNCCB func)
{
	if(func)
	{
		if(0 == type)
		{
			pbStopElecZoom = func;
		}
	}
	
	return -1;
}

s32 ModPlayBackGetRealPlayChn(PbMgrHandle hPbMgr, u64* pChnMask)
{
	if((!hPbMgr) || (!pChnMask))
	{
		return -1;
	}
	
	SPlayBackManager* pSPbManager = (SPlayBackManager*)hPbMgr;
	
	*pChnMask = 0;
	int i = 0;
	for(i = 0; i < pSPbManager->nMaxChnNum; i++)
	{
		//printf("nIsPlaying[%d] = %d\n",i,pSPbManager->aPlaybackChnInfos[i].nIsPlaying);
		if((pSPbManager->aPlaybackChnInfos[i].nRealFileNums)
			&&(pSPbManager->aPlaybackChnInfos[i].nIsPlaying))
		{
			//*pChnMask |= (1 << i);
			*pChnMask |= (1 << pSPbManager->aPlaybackChnInfos[i].nPlayNo);
		}
		if(i == 63)
		{
			break;
		}
	}
	return 0;
}

s32 ModPlayBackGetVideoFormat(PbMgrHandle hPbMgr, u8 nChn)
{
	if(!hPbMgr)
	{
		return -1;
	}
	
	SPlayBackManager* pSPbManager = (SPlayBackManager*)hPbMgr;
	
	u8 i = 0;
	for(i = 0; i < pSPbManager->nMaxChnNum; i++)
	{
		if(pSPbManager->aPlaybackChnInfos[i].nPlayNo == nChn)
		{
			//csp modify
			int nCurIndex = pSPbManager->aPlaybackChnInfos[i].nCurIndex;
			if(nCurIndex >= 0 && nCurIndex < pSPbManager->aPlaybackChnInfos[i].nRealFileNums)
			{
				//printf("ModPlayBackGetVideoFormat nChn=%d nFormat=%d\n",nChn,pSPbManager->aPlaybackChnInfos[i].aRecFileInfos[nCurIndex].nFormat);
				return pSPbManager->aPlaybackChnInfos[i].aRecFileInfos[nCurIndex].nFormat;
			}
		}
	}
	
	return -1;
}

s32 ModPlayBackSetModeVOIP(PbMgrHandle hPbMgr, u8 nVoip)
{
	if(!hPbMgr)
	{
		return -1;
	}
	
	SPlayBackManager* pSPbManager = (SPlayBackManager*)hPbMgr;
	pSPbManager->nIsVOIP = nVoip ? 1 : 0;
	
	return 0;
}

//回放时单通道放大/恢复
#if 1
s32 ModPlayBackZoom(PbMgrHandle hPbMgr, s32 nKey)
{
	if(!hPbMgr)
	{
		return -1;
	}
	
	//printf("entry ModPlayBackZoom\n");
	
	SPlayBackManager* pSPbManager = (SPlayBackManager*)hPbMgr;
	
	if(pSPbManager->nPlayingChnsMask == 0)
	{
		return 0;
	}
	
	unsigned char zoom_changed = 0;//csp modify 20130509
	
	u32 nMaxChn = pSPbManager->nMaxChnNum;
	
	static int lastKey = 0;
	
	if(nKey < 0)//进入下一个单画面
	{
		//csp modify
		int loop_count = 0;
		
		while(1)
		{
			if(lastKey == nMaxChn)
			{
				lastKey = 0;
			}
			if((pSPbManager->nPlayingChnsMask & (1 << lastKey)) && (pSPbManager->aPlaybackChnInfos[lastKey].nIsPlaying))//cw_playback
			{
				tl_vdec_zoom(pSPbManager->aPlaybackChnInfos[lastKey].nPlayNo);
				
				zoom_changed = 1;//csp modify 20130509
				
				int i;
				for(i = 0; i < nMaxChn; i++)
				{
					pSPbManager->aPlaybackChnInfos[i].nFlagZoom = 0;
				}
				
				pSPbManager->aPlaybackChnInfos[lastKey].nFlagZoom = 1;
				
				lastKey++;
				
				break;
			}
			lastKey++;
			
			//csp modify
			//目的是为了有办法退出此循环
			loop_count++;
			if(loop_count > nMaxChn)
			{
				break;
			}
		}		
	}
	else if(nKey >= pSPbManager->nRealPlayNum)//恢复到多路回放
	{
		u32 nRealPlayNum = pSPbManager->nRealPlayNum;
		if(nRealPlayNum == 1)
		{
			//
		}
		else if(nRealPlayNum <= 4)
		{
			nRealPlayNum = 4;
		}
		else if(nRealPlayNum <= 9)
		{
			nRealPlayNum = 9;
		}
		else if(nRealPlayNum <= 16)
		{
			nRealPlayNum = 16;
		}
		else if(nRealPlayNum <= 24)
		{
			nRealPlayNum = 24;
		}
		else if(nRealPlayNum <= 32)
		{
			nRealPlayNum = 36;
		}
		else
		{
			nRealPlayNum = 16;
			printf("*****%d playChns is too large\n", nRealPlayNum);
		}
		
		//printf("ModPlayBackZoom-tl_vdec_zoom-1\n");
		tl_vdec_zoom(nRealPlayNum);
		//printf("ModPlayBackZoom-tl_vdec_zoom-2\n");
		
		zoom_changed = 1;//csp modify 20130509
		
		int i;
		for(i = 0; i < nMaxChn; i++)
		{
			pSPbManager->aPlaybackChnInfos[i].nFlagZoom = 2;
		}
		lastKey = 0;
	}
	else//放大某一个画面
	{
		int i;
		for(i = 0; i < nMaxChn; i++)
		{
			if(pSPbManager->nPlayingChnsMask & (1 << i))
			{
				if(pSPbManager->aPlaybackChnInfos[i].nPlayNo == nKey)
				{
					//printf("rz_vdec_zoom %d\n",nKey);
					tl_vdec_zoom(nKey);
					
					zoom_changed = 1;//csp modify 20130509
					
					int ii;
					for(ii = 0; ii < nMaxChn; ii++)
					{
						pSPbManager->aPlaybackChnInfos[ii].nFlagZoom = 0;
					}
					
					pSPbManager->aPlaybackChnInfos[i].nFlagZoom = 1;
					
					lastKey = i + 1;
				}
			}
		}
	}
	
	#if 0//#if defined(CHIP_HISI3531)//csp modify 20130509
	if(pSPbManager->emCurCtlState == EM_CTL_STATE_PAUSE && zoom_changed)
	{
		printf("zoom at pause,emCurCtlState=%d...\n",pSPbManager->emCurCtlState);
		
		int i;
		for(i = 0; i < 3; i++)
		{
			SCtlMSG sCtlMsg;
			sCtlMsg.emCmd = EM_CTL_STEP;
			
			u32 ack = 0;
			SendCtlMsg(pSPbManager,&sCtlMsg,&ack);
		}
		
		usleep(200*1000);
		
		tl_vdec_ioctl(0, 0xf0, NULL);//CMD_VDEC_REFRESH
	}
	#endif
	
	//printf("ModPlayBackZoom over\n");
	
	return 0;
}
#else
s32 ModPlayBackZoom(PbMgrHandle hPbMgr, s32 nKey)
{
	if(!hPbMgr)
	{
		return -1;
	}	
	SPlayBackManager* pSPbManager = (SPlayBackManager*)hPbMgr;
	u32 nMaxChn = pSPbManager->nMaxChnNum;
	static int lastKey = 0;
	
	if(nKey < 0)
	{
		tl_vdec_zoom(lastKey);
		pSPbManager->nZoomChn = lastKey;
		if(++lastKey == pSPbManager->nRealPlayNum)
		{
			lastKey = 0;
		}
	}
	else if(nKey >= pSPbManager->nRealPlayNum)
	{
		u32 nRealPlayNum = pSPbManager->nRealPlayNum;
		if(nRealPlayNum == 1)
		{
			//
		}
		else if(nRealPlayNum <= 4)
		{
			nRealPlayNum = 4;
		}
		else if(nRealPlayNum <= 9)
		{
			nRealPlayNum = 9;
		}
		else if(nRealPlayNum <= 16)
		{
			nRealPlayNum = 16;
		}
		else if(nRealPlayNum <= 24)
		{
			nRealPlayNum = 24;
		}
		else if(nRealPlayNum <= 32)
		{
			nRealPlayNum = 36;
		}
		else
		{
			nRealPlayNum = 16;
			printf("***** %d playChns is too large\n", nRealPlayNum);
		}
		tl_vdec_zoom(nRealPlayNum);
		pSPbManager->nZoomChn = 0;
		lastKey = 0;
	}
	else
	{
		tl_vdec_zoom(nKey);
		lastKey = nKey;
		pSPbManager->nZoomChn = lastKey;
		if(++lastKey == pSPbManager->nRealPlayNum)
		{
			lastKey = 0;
		}
	}
	
	return 0;
}
#endif

//yaogang modify 20150112
//s32 ModSnapDisplay(PbMgrHandle hPbMgr, SPBRecSnapInfo* pSnapInfo)
s32 ModSnapDisplay(SPBRecSnapInfo* pSnapInfo)
{
	s32 ret = 1;
	u8 buf[128*1024];
	partition_index *index = NULL;

	/*
	u32 nSize;
	u32 nOffset;
	u8 nDiskNo;
	u8 nPtnNo;
	u16 nFileNo;
	*/
	index = get_partition(gp_sHddManager, pSnapInfo->nDiskNo, pSnapInfo->nPtnNo);
	if (index)
	{
		memset(buf, 0, 128*1024);
		ret = read_snap_file(index, buf, pSnapInfo->nFileNo, pSnapInfo->nOffset, pSnapInfo->nSize);
		if (ret == 0)
		{
		#if 0 //debug
			char file_name[64];
			time_t pic_time = pSnapInfo->nStartTime;
			struct tm now;
			localtime_r(&pic_time, &now);
			
			sprintf(file_name, 
				"/mnt/chn%02d_%02d-%02d-%02d.jpg", 
				pSnapInfo->nChn, now.tm_hour+8, now.tm_min, now.tm_sec);
	
			FILE *fp = fopen(file_name, "w");
			if(fp != NULL)
			{
				fwrite(buf, pSnapInfo->nSize, 1, fp);
				fclose(fp);
			}
		#endif
			
			ret = tl_snap_vdec_write(buf, pSnapInfo->nSize);
			if (ret)
			{
				printf("%s tl_snap_vdec_write failed\n", __func__);
			}
		}
		else
		{
			printf("%s read_snap_file failed\n", __func__);
		}
	}

	return ret;
}


