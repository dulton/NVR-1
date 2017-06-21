// file description
#include "RemotePlay.h"
#include "Mod_syscomplex.h"
#include "remotetimemultiplay.h"
#include <sys/time.h>
#include "diskmanage.h"

//** macro

#define MAX_REMOTE_MULTI_PLAY_CHNS MAX_REMOTE_PLAYER_NUM//(CHN_NUM_MAIN * 2)

//** typedef 

//** local var
BOOL send_is_ok;
static u32  sect_start_pos[RMTPLAY_PROG_SEG_MAX];
static PRemotePlayCB pPlayCB = NULL;
static u8		nInitVideMediaType;
static u8		nInitAudioMediaType;
static u8		nInitVideFrameRate;
static u8		nInitAudioFrameRate;
static u8		nInitAudioMode;
static u8		nChnMax;

//** global var
extern u16 SendMediaFrameByTcp(SOCKHANDLE sock_fd, PFRAMEHDR pFrmHdr, u8 bVideo );
extern u16 SendMediaFrameByTcpExt(SOCKHANDLE sock_fd, PFRAMEHDR pFrmHdr, u8 b_mutil, u8 bVideo);

//** local functions
static void *remoteTimePlayFxn(void *arg);

static s32 remotePlay_InitVFrameHeader( 
		FRAMEHDR* pFrameHead, 
		u16 nFrameWidth, 
		u16 nFrameHeight
);

static s32 remotePlay_InitAFrameHeader( 
		custommp4_t * file,
		FRAMEHDR* pFrameHead
);

//** global functions
extern u16 GetNewIdNum();

u32 getTimeStamp()
{
	struct timeval tv;
	
	gettimeofday(&tv, NULL);
	//return tv.tv_sec*HZ + (tv.tv_usec*HZ)/1000000;
	return tv.tv_sec*1000 + (tv.tv_usec)/1000;
	
	//return TickGet();
}

/*
 play mode by time or file
*/
typedef enum
{
	EM_PlayMode_Time,
	EM_PlayMode_File,
	
} EM_PlayMode;

/*
 remote play control block structure
 
 // datasour - filename, seekpos,  byTime/byFile, 
 // datadest - socket
 // status control - seekpos, playdirection, playspeed, bPause, bStop, connBreak

*/

#define RMT_PLAY_STREAM_MAX	8

typedef struct							//04-30
{
	ifly_play_handle_t hid;
	ifly_remote_playback_t remoteplay;
	CPHandle cph;
	u8   byFlag;//0:按文件播放;1:按时间回放
	u8   byUse;
	u8   byStop;
	u8   byPause;
	u8   byMute;
	u8   byStatus;
	u8   bySkip;
	re_em_play_rate rate;
	u32  prePos;
	u32  curPos;
	u32  totalTime;
	u32  refTime;
	u32  seekPos;
	SOCKHANDLE playback_sockfd;
} ifly_remote_player_t;

// 按时间回放搜索内部结构
// 在原结构基础上扩展至支持32路搜索
typedef struct
{
	u32		channel_mask;				//通道 按位组合
	u16		type_mask;					//类型 按位组合
	u32		start_time;					//开始时间
	u32		end_time;					//终止时间
	u16		startID;					//返回的第一条记录,从1开始
	u16		max_return;					//每次返回的最大记录数
	u8		reserved[4];				//预留
}PACK_NO_PADDING ifly_recsearch_param_ex_t;

static ifly_remote_player_t remote_player[MAX_REMOTE_PLAYER_NUM];

u8 remote_multi_play_status[MAX_REMOTE_MULTI_PLAY_CHNS] = {0};
#define remote_multi_player remote_player
ifly_recsearch_param_ex_t remote_multi_recsearch_param[MAX_REMOTE_MULTI_PLAY_CHNS];
recplayfile_t multi_playchn[MAX_REMOTE_MULTI_PLAY_CHNS];
custommp4_t *multi_play_file[MAX_REMOTE_MULTI_PLAY_CHNS];
u32 mutil_play_index[MAX_REMOTE_MULTI_PLAY_CHNS];
FRAMEHDR multi_play_hdr_v[MAX_REMOTE_MULTI_PLAY_CHNS];
FRAMEHDR multi_play_hdr_a[MAX_REMOTE_MULTI_PLAY_CHNS];
u8 multi_frame_ready[MAX_REMOTE_MULTI_PLAY_CHNS];

//csp modify 20121116//这里需要验证//CHIP_HISI3531是否起作用?
#ifdef CHIP_HISI3531
u8 multi_play_buf[MAX_REMOTE_MULTI_PLAY_CHNS][512 << 10];
#else
//csp modify 20140302
//u8 multi_play_buf[MAX_REMOTE_MULTI_PLAY_CHNS][256 << 10];
//csp modify 20140406
u8 multi_play_buf[MAX_REMOTE_MULTI_PLAY_CHNS][512 << 10];
#endif

u8 seekpos_flag[MAX_REMOTE_MULTI_PLAY_CHNS];

static void *remotePlayFxn(void *arg);

static int remotePlay_ProgressNotify( u32 envent, u8 nIdx )
{
	void* pVoid = NULL;
	int   nLen  = 0;
	
	ifly_progress_t 		play_progress;
	ifly_remote_player_t* 	pCtrl = &remote_player[nIdx];
	
	int linkid = pCtrl->hid.id;
	u8 b_send = 0;
	
	//printf("!!!!!!!!!!!!1\n");
	if(envent==CTRL_NOTIFY_PLAYEND)
	{
		pVoid = &pCtrl->hid;
		nLen = sizeof(pCtrl->hid);
		b_send = 1;
	}
	else if(pCtrl->byStatus)//是否上传进度
	{
		if (pCtrl->prePos != pCtrl->curPos)
		{
			play_progress.currPos = htonl(pCtrl->curPos);
			play_progress.totallen = htonl(pCtrl->totalTime);
			play_progress.id = htonl(linkid);
			
			pVoid = &play_progress;
			nLen = sizeof(play_progress);

			pCtrl->prePos = pCtrl->curPos;
			b_send = 1;
		}
		//printf("!!!!!!!!!!!!4\n");
	}
	
	u16 ret;
	
	//printf("!!!!!!!!!!!!2\n");
	if (b_send)
	{
		ret = CPPost(pCtrl->cph,envent,pVoid,nLen);
		if(ret)
		{
			printf("CPPost error %d\n", ret);
		}
	}
	//printf("!!!!!!!!!!!!3\n");
	
	return 0;
}

// 0 step forward, 1 continue, 2 break;
// 
static EM_REMOTEPLAY_CHECKDO remotePlay_CheckStatus(u8 nIdx)
{
	// assert(nIdx)
	s32 rtn = EM_RMTPLAY_CHK_STEPFWD;
	
	ifly_remote_player_t* pCtrl = &remote_player[nIdx];
#if 0	
	if(pCtrl->byConLost)//||pCtrl->bStop)
	{
		rtn = EM_RMTPLAY_STATE_CONNLOST;
	}
	else 
#endif
	if(pCtrl->byStop)
	{
		rtn  = EM_RMTPLAY_CHK_STOP;
	}
	else if(pCtrl->byPause)
	{
		if(pCtrl->rate != PLAYRATE_SINGLE)
		{
			usleep(20000);
			rtn  = EM_RMTPLAY_CHK_CONTINUE;
		}
	}
	
	return rtn;
}

// init seekpos segment parts
// 
static void remotePlay_InitSeekSeg( u8 nIdx, int nFileTotalTime )
{
	int i;
	ifly_remote_player_t* pCtrl = &remote_player[nIdx];
	
	pCtrl->seekPos 		= 0;
	pCtrl->curPos 		= 0;
	pCtrl->totalTime 	= nFileTotalTime / 1000;//按文件回放以秒为单位
	
	// split progress for jumping later
	// 
	for(i=0; i<RMTPLAY_PROG_SEG_MAX; i++)
	{
		sect_start_pos[i] = nFileTotalTime * i / RMTPLAY_PROG_SEG_MAX;
	}
}

static int remotePlay_PlayDelay(u8 nIdx, int playrate, int start_time, int playPos, int seekpos_flag  )
{
	int delaytime, arrive_time;
	ifly_remote_player_t* pCtrl = &remote_player[nIdx];
	
	delaytime = 0;
	if(playrate > 0)
	{
		//NETCOMM_DEBUG_STR("playrate > 0\n",0);
		arrive_time = pCtrl->refTime+(start_time-playPos)/playrate;
		delaytime = arrive_time-getTimeStamp();
		
		//csp modify 20131001
		//解决远程回放音频卡顿问题
		if(playrate == 1)
		{
			delaytime = delaytime - 8;
			if(delaytime < 0)
			{
				delaytime = 0;
			}
			//printf("delay:%d\n",delaytime);
		}
	}
	else if(playrate == 0)
	{
		//NETCOMM_DEBUG_STR("playrate == 0\n",0);
		//帧进
		arrive_time = pCtrl->refTime+start_time-playPos;
		delaytime = arrive_time-getTimeStamp();
		pCtrl->rate = 1;
	}
	else if(playrate == -1)
	{
		//NETCOMM_DEBUG_STR("playrate == -1\n",0);
		//回退--只播放关键帧
		delaytime = 40;
		arrive_time = getTimeStamp() + delaytime;
	}
	else
	{
		//NETCOMM_DEBUG_STR("playrate == other\n",0);
		arrive_time = pCtrl->refTime + (start_time - playPos) * (-1) * playrate;
		delaytime = arrive_time - getTimeStamp();
	}
	
	/*********************/
	//07-10-26
	if(delaytime > 0 && seekpos_flag)
	{
		//delaytime = 0;
		delaytime = 40;
		pCtrl->refTime=0;				
	}
	else
	{
		if(delaytime > 10000)
		{
			delaytime = 40;
			pCtrl->refTime = 0;
		}
	}
	
	if(delaytime > 0)
	{
		struct timeval val;
		val.tv_sec  = delaytime/1000;
		val.tv_usec = (delaytime%1000)*1000;
		select(0,NULL,NULL,NULL,&val);
		//printf("delaytime=%d\n", delaytime);
	}

	return 0;
}

// return seekpos
// 
static int remotePlay_Seek2Pos( 
	void *pFile,
	SRmtPlayFileOp* psOp,
	u8 			nIdx, 
	int 		playPos, 
	int* 		seekpos_flag 
)
{
	// assert(nIdx)
	int i;
	u32 seekpos = 0;
	
	ifly_remote_player_t* pCtrl = &remote_player[nIdx];

	if(pCtrl->bySkip==1||pCtrl->bySkip==2)
	{
		if(pCtrl->bySkip == 1)
		{
			for(i=0;i<RMTPLAY_PROG_SEG_MAX;i++)
			{
				if(playPos<sect_start_pos[i])
				{
					*seekpos_flag = 1;
					break;
				}
			}
			
			seekpos = i>1?sect_start_pos[i-2]:0;
		}
		else
		{
			for(i=RMTPLAY_PROG_SEG_MAX-1;i>=0;i--)
			{
				if(playPos>=sect_start_pos[i])
				{
					*seekpos_flag = 1;
					break;
				}
			}
			
			if(i<9)
			{
				seekpos = sect_start_pos[i+1];
			}
			else
			{
				*seekpos_flag = 0;
				return -1;
			}
		}
		
		pCtrl->bySkip = 0;
		pCtrl->refTime = 0;	
		
		psOp->eOp = EM_RMTPLAY_SEEK;
		psOp->OpenSeek.nOffset = seekpos;
		
		(*pPlayCB)(pFile, psOp);	
		if(psOp->nOpRslt<0)
			return -1;
	}

	if(pCtrl->seekPos)
	{
		seekpos = pCtrl->seekPos;	
		pCtrl->seekPos = 0;
		*seekpos_flag = 1;
		
		psOp->eOp = EM_RMTPLAY_SEEK;
		psOp->OpenSeek.nOffset = seekpos;
		
		(*pPlayCB)(pFile, psOp);
	}

	if(	*seekpos_flag )
		NETCOMM_DEBUG_STR("seek active!!", 0);
		
	return 0;
}


static s32 remotePlay_ClearLinkNode( 
		ifly_remote_player_t*	pLnkCtrl
);

s32 remotePlay_Init( 
	PRemotePlayCB	 pRpCB,
	u8 nVideMediaType,
	u8 nAudioMediaType,
	u8 nVideFrameRate,
	u8 nAudioFrameRate,
	u8 nAudioMode,
	u8 nChMax
)
{
	s32 rtn = 0;
	
	memset( &remote_player[0], 0, sizeof(ifly_remote_player_t)*MAX_REMOTE_PLAYER_NUM);
	
	pPlayCB = pRpCB;
	
	nInitVideMediaType 	= nVideMediaType;
	nInitAudioMediaType = nAudioMediaType;
	nInitVideFrameRate 	= nVideFrameRate;
	nInitAudioFrameRate = nAudioFrameRate;
	nInitAudioMode 		= nAudioMode;
	
	nChnMax = nChMax;
	
	return rtn;
}

static s32 remotePlay_InitLinkNode(
		ifly_remote_player_t*	pLnkCtrl,
		SOCKHANDLE				sock,
		u32	nAckId
)
{
	s32 rtn = 0;
	struct timeval tv;
	
	NETCOMM_ASSERT(pLnkCtrl);

	tv.tv_sec = 10;
	tv.tv_usec = 0;
	rtn = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));
	if(rtn)
	{
		NETCOMM_DEBUG_STR("setsockopt err !", rtn);
	}
	
	pLnkCtrl->playback_sockfd = sock;
	pLnkCtrl->byUse = 1;
	pLnkCtrl->byStop = 0;
	pLnkCtrl->byPause = 0;
	pLnkCtrl->byMute = 0;
	pLnkCtrl->bySkip = 0;
	pLnkCtrl->rate = PLAYRATE_1;
	pLnkCtrl->hid.id = nAckId;
	
	//printf("pLnkCtrl->hid.id = GetNewIdNum(); id %d\n", pLnkCtrl->hid.id);
	
	return rtn;
}

extern s32 netComm_Ack( u32 err, int sock, u32 nAckId );
					
s32 remotePlay_Request(ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock, u32 nAckId)
{
	u8	j;
	s32 rtn = 0;
	ifly_remote_player_t* pLnkCtrl;
	pthread_t 		remotePlayThxd;
	
	NETCOMM_DEBUG_STR("Enter remotePlay_Request !", 0);
	
	if(!pReq)
	{
		NETCOMM_DEBUG_STR("serious error, request null!", -1);
		printf("serious error, request null!\n");
		
		exit(1);
	}
	
	for (j = 0; j < MAX_REMOTE_PLAYER_NUM; j++)
	{
		pLnkCtrl = &remote_player[j];
		//printf("remote_player[j].byUse=%d\n",remote_player[j].byUse);
		if (0 == pLnkCtrl->byUse )
		{
			remotePlay_InitLinkNode(pLnkCtrl,sock,nAckId);
			
			break;
		}
	}
	
	if (MAX_REMOTE_PLAYER_NUM <= j)
	{		
		rtn = CTRL_FAILED_LINKLIMIT;
		
		goto END;
	}
	
	strcpy(pLnkCtrl->remoteplay.filename, pReq->FilePlayBack_t.filename);
	//printf("app file name:%s\n",pLnkCtrl->remoteplay.filename);
	
	pLnkCtrl->remoteplay.offset = (pReq->FilePlayBack_t.offset);
	
	//printf("pLnkCtrl->remoteplay.offset %d\n", pLnkCtrl->remoteplay.offset);
	
	/*
	sigset_t signal_mask;
	sigemptyset (&signal_mask);
	sigaddset (&signal_mask, SIGPIPE);
	int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) {
		printf("block sigpipe error\n");
	}
	*/
	send_is_ok = FALSE;
	
	// create preview stream thread
	rtn = pthread_create(&remotePlayThxd,
						 NULL,
						 remotePlayFxn,
						 (void*)(u32)j);
	if(0 != rtn)
	{
		//pthread_exit(&remotePlayThxd);
		
		rtn = CTRL_FAILED_RESOURCE; // fail to create preview thread
		
		remotePlay_ClearLinkNode(pLnkCtrl);
		
		goto END;
	}
	
	netComm_Ack(0, sock, nAckId);
	
	send_is_ok = TRUE;
	
END:
	if(rtn)
	{
		NETCOMM_DEBUG_STR("remotePlay_Request, err ", rtn);
	}
	
	return rtn;
}

void *remotePlayFxn(void *arg)
{
	printf("$$$$$$$$$$$$$$$$$$remotePlayFxn id:%d\n",getpid());
	
	NETCOMM_DEBUG_STR("Enter remotePlayFxn ...", 0);
	
	// check if network tx is ok
	while(FALSE == send_is_ok)
	{
		usleep(RMTPLAY_TX_ERR_DELAY);
	}
	
	// set to confirm release all the resource after thread exit
	pthread_detach(pthread_self());
	
	printf("start remoteplay file......\n");
	
	ifly_remote_player_t *pRemotePlayer = NULL;
	ifly_remote_playback_t *rp = NULL;
	
	void* pFileHandle = NULL;
	
	// ignore authorization...
	// begin to deal with the request
	// 
	s32 rtn;
	u16 wRet;
	u16 nPlayerIdx;
	
	// parse the request parameter 
	// it's player idx now
	// 
	nPlayerIdx = (u32)arg;
	
	if(MAX_REMOTE_PLAYER_NUM<=nPlayerIdx)
	{
		NETCOMM_DEBUG_STR("Player idx too big ...", -1);
		
		goto ERROR;
	}
	
	pRemotePlayer = &remote_player[nPlayerIdx];
	rp = &pRemotePlayer->remoteplay;
	
	FRAMEHDR tVideoFRAMEHDR;
	FRAMEHDR tAudioFRAMEHDR;
	
	//csp modify
	//tVideoFRAMEHDR.m_tVideoParam.m_wVideoWidth  = vwidth;
	//tVideoFRAMEHDR.m_tVideoParam.m_wVideoHeight = vheight;
	tVideoFRAMEHDR.m_dwPreBufSize = 0;
	tVideoFRAMEHDR.m_byMediaType  = 98;//MEDIA_TYPE_H264;
	tVideoFRAMEHDR.m_byFrameRate  = 25;
	
	//csp modify
	tAudioFRAMEHDR.m_byAudioMode  = 16;
	tAudioFRAMEHDR.m_dwPreBufSize = 0;
	tAudioFRAMEHDR.m_byFrameRate  = 25;
	#ifdef USE_AUDIO_PCMU
	tAudioFRAMEHDR.m_byMediaType  = 0;//MEDIA_TYPE_PCMU
	#else
	tAudioFRAMEHDR.m_byMediaType  = 21;//MEDIA_TYPE_ADPCM
	#endif
	
	SRmtPlayFileOp sRmtPlayFileIns;
	
	memset(&sRmtPlayFileIns, 0, sizeof(sRmtPlayFileIns));	
	sRmtPlayFileIns.eOpSrc = EM_FILEOP_REPLAY;
	sRmtPlayFileIns.eOp = EM_RMTPLAY_OPEN;
	
	strcpy(sRmtPlayFileIns.OpenSeek.szFilePath, rp->filename);
	sRmtPlayFileIns.OpenSeek.nOffset = rp->offset;	
	(*pPlayCB)(NULL, &sRmtPlayFileIns);
	if(sRmtPlayFileIns.OpenSeek.pFile == NULL)
	{
		remotePlay_ClearLinkNode(pRemotePlayer);		
		return 0;
	}
	
	pFileHandle = sRmtPlayFileIns.OpenSeek.pFile;
	
	NETCOMM_DEBUG_STR("custommp4_open ok",0);
	//printf("custommp4_open ok......\n");
	
	//csp modify 20121116//这里需要验证//CHIP_HISI3531是否起作用?
	#ifdef CHIP_HISI3531
	u8 media_buf[512 << 10];
	#else
	//csp modify 20140302
	//u8 media_buf[256 << 10];
	u8 media_buf[512 << 10];
	#endif
	
	u8 media_type,key;
	u64 pts;//wrchen 081226
	u32 start_time;
	int realsize = 1;
	int playrate = 1;
	
	sRmtPlayFileIns.eOp = EM_RMTPLAY_GetMediaFormat;
	(*pPlayCB)(pFileHandle, &sRmtPlayFileIns);
	
	//printf("EM_RMTPLAY_GetMediaFormat......\n");
	
	remotePlay_InitVFrameHeader(
		&tVideoFRAMEHDR,
		sRmtPlayFileIns.GetFormat.nWidth,
		sRmtPlayFileIns.GetFormat.nHeight
	);
	
	//printf("EM_RMTPLAY_GetTimeLength......\n");
	
	sRmtPlayFileIns.eOp = EM_RMTPLAY_GetTimeLength;
	(*pPlayCB)(pFileHandle, &sRmtPlayFileIns);
	
	/*
	FRAMEHDR* pHdr = &tVideoFRAMEHDR;
	printf("tMediaHDR.m_wVideoWidth %d\n", pHdr->m_tVideoParam.m_wVideoWidth);
	printf("tMediaHDR.m_wVideoHeight %d\n", pHdr->m_tVideoParam.m_wVideoHeight);
	printf("tMediaHDR.m_byMediaType %d\n", pHdr->m_byMediaType);
	printf("tMediaHDR.m_byFrameRate %d\n", pHdr->m_byFrameRate);
	printf("tMediaHDR.m_dwFrameID %d\n", pHdr->m_dwFrameID);
	printf("tMediaHDR.m_bKeyFrame %d\n", pHdr->m_tVideoParam.m_bKeyFrame);
	printf("tMediaHDR.m_dwTimeStamp %d\n", pHdr->m_dwTimeStamp);
	printf("tMediaHDR.m_dwDataSize %d\n", pHdr->m_dwDataSize);
	*/
	
	//getchar();
	
	// Split to little parts
	//
	remotePlay_InitSeekSeg(nPlayerIdx, sRmtPlayFileIns.GetTimeLen.nTotalTime);
	
	int playPos = 0;
	int seekpos_flag = 0;
	
	pRemotePlayer->refTime = 0;
	
	printf("remoteplay loop......\n");
	
	// start read & send loop
	do
	{
		//printf("remoteplay loop-1\n");
		//fflush(stdout);
		
		// check bStop, connBreak quit if TRUE, 
		// or pause then next loop
		// 
		rtn = remotePlay_CheckStatus(nPlayerIdx);
		if(rtn==EM_RMTPLAY_CHK_STOP)
		{
			NETCOMM_DEBUG_STR("EM_RMTPLAY_CHK_STOP", 0);
			
			break;
		}
		else if(rtn==EM_RMTPLAY_CHK_CONTINUE)
		{
			NETCOMM_DEBUG_STR("EM_RMTPLAY_CHK_CONTINUE", 0);
			
			continue;
		}
		
		//printf("remoteplay loop-2\n");
		//fflush(stdout);
		
		// check direction to play
		// check skip, seekpos, move pos of file if TRUE
		//
		// seek to belonged segment
		#if 1
		if( remotePlay_Seek2Pos( 
			pFileHandle, 
			&sRmtPlayFileIns, 
			nPlayerIdx, 
			playPos, 
			&seekpos_flag ) > 0 
		)
		{
			NETCOMM_DEBUG_STR("remotePlay_Seek2Pos err", -1);
			
			break;
		}
		#endif
		
		//printf("remoteplay loop-3\n");
		//fflush(stdout);
		
		// getframe & init reftime
		// 
		
		sRmtPlayFileIns.eOp = EM_RMTPLAY_GETFRAME;
		sRmtPlayFileIns.GetFrame.pFrameBuf = media_buf;
		sRmtPlayFileIns.GetFrame.nBufSize = sizeof(media_buf);
		
		(*pPlayCB)(pFileHandle, &sRmtPlayFileIns);
		if(sRmtPlayFileIns.nOpRslt<0)
			break;
		
		//printf("remoteplay loop-4\n");
		//fflush(stdout);
		
		start_time 	= sRmtPlayFileIns.GetFrame.nStartTime;
		key 		= sRmtPlayFileIns.GetFrame.bKey;
		pts 		= sRmtPlayFileIns.GetFrame.nPts;
		media_type 	= sRmtPlayFileIns.GetFrame.nMediaType;		
		realsize 	= sRmtPlayFileIns.GetFrame.nFrameSize;
		if(realsize <= 0)
		{
			break;
		}
		
		if(pRemotePlayer->refTime == 0)
		{
			playPos = start_time;
			pRemotePlayer->refTime = getTimeStamp();
		}
		
		playrate = pRemotePlayer->rate;
		
		//printf("remoteplay loop-5\n");
		//fflush(stdout);
		
		// send frame by media type
		// 
		if(media_type)
		{
			//printf("a-1\n");
			//fflush(stdout);
			
			if(pRemotePlayer->playback_sockfd != INVALID_SOCKET && !pRemotePlayer->byMute && playrate == 1)
			{
				tAudioFRAMEHDR.m_pData = media_buf;
				tAudioFRAMEHDR.m_dwDataSize = realsize;
				tAudioFRAMEHDR.m_dwTimeStamp = getTimeStamp();
				wRet = SendMediaFrameByTcp((SOCKHANDLE)pRemotePlayer->playback_sockfd, &tAudioFRAMEHDR, 0);
				//printf("send one audio frame size=%d ret=%d\n",tAudioFRAMEHDR.m_dwDataSize,wRet);
				//fflush(stdout);
				if(wRet)
				{
					pRemotePlayer->byStop = 1;
					break;
				}
			}
			
			//printf("a-2\n");
			//fflush(stdout);
		}
		else
		{
			//printf("v-1\n");
			//fflush(stdout);
			
			// delay before send video frame
			//
			remotePlay_PlayDelay(nPlayerIdx,playrate,start_time,playPos,seekpos_flag);
			
			//printf("v-2\n");
			//fflush(stdout);
			
			// fill head & send 
			pRemotePlayer->curPos = start_time/1000;
			playPos = start_time;
			pRemotePlayer->refTime = getTimeStamp();
			
			if(!seekpos_flag || key)
			{				
				tVideoFRAMEHDR.m_pData = media_buf;
				tVideoFRAMEHDR.m_dwDataSize = realsize;
				//u64 nTmpStamp = (pts/1000);
				
				//tVideoFRAMEHDR.m_dwTimeStamp 	= nTmpStamp&0xFFFFFFFF;//(pts/1000);//getTimeStamp();
				tVideoFRAMEHDR.m_dwTimeStamp = getTimeStamp();
				
				//tVideoFRAMEHDR.m_byMediaType 	= media_type;
				tVideoFRAMEHDR.m_tVideoParam.m_bKeyFrame = key ? 3 : 0;
				
				//printf("v-3\n");
				//fflush(stdout);
				
				tVideoFRAMEHDR.m_dwFrameID++;
				wRet = SendMediaFrameByTcp(pRemotePlayer->playback_sockfd, &tVideoFRAMEHDR, 1);
				
				//printf("v-4\n");
				//fflush(stdout);
				
				if(wRet)
				{
					pRemotePlayer->byStop = 1;
					break;
				}
				
				seekpos_flag = 0;
			}
			
			//printf("v-5\n");
			//fflush(stdout);
			
			remotePlay_ProgressNotify(CTRL_NOTIFY_PLAYPROGRESS, nPlayerIdx);
			
			//printf("v-6\n");
			//fflush(stdout);
		}
		
		// change delay as playspeed change or bPause
	} while(realsize > 0);
	
	// endloop
	
	// notify play end
	// release sources
ERROR:
	
	NETCOMM_DEBUG_STR("remotePlayFxn end 0", 0);
	
	//发送放像结束通知
	//remotePlay_ProgressNotify(CTRL_NOTIFY_PLAYEND, nPlayerIdx);

	NETCOMM_DEBUG_STR("remotePlayFxn end 1", 0);
	
	// file close
	if(NULL!=pFileHandle)
	{
		sRmtPlayFileIns.eOp = EM_RMTPLAY_CLOSE;
		(*pPlayCB)(pFileHandle, &sRmtPlayFileIns);
		
		pFileHandle = NULL;
	}
	NETCOMM_DEBUG_STR("remotePlayFxn end 2", 0);
	
	// clear replay link
	remotePlay_ClearLinkNode(pRemotePlayer);
	
	NETCOMM_DEBUG_STR("Exit remotePlayFxn !!!", 0);
	
	return 0;
}

static s32 remotePlay_ClearMultiLinkNode( s32 index, ifly_remote_player_t*	pLnkCtrl )
{
	int i;
	s32 rtn = 0;
	
	if(!pLnkCtrl)
	{
		rtn = -1;
		goto CLR_ERR;
	}

	if(pLnkCtrl->playback_sockfd!=INVALID_SOCKET)
	{
		close(pLnkCtrl->playback_sockfd);
		pLnkCtrl->playback_sockfd = INVALID_SOCKET;
	}
	
	pLnkCtrl->byStop = 0;
	pLnkCtrl->byUse = 0;

	for (i = 0; i < MAX_REMOTE_MULTI_PLAY_CHNS; i++)
		if (remote_multi_play_status[i] == (index + 1)) remote_multi_play_status[i] = 0;
	
CLR_ERR:
	if(rtn)
	{
		NETCOMM_DEBUG_STR("clear lnk node err!!!", rtn);
	}
	
	NETCOMM_DEBUG_STR("Exit remotePlay_ClearLinkNode!!!", rtn);
			
	return rtn;
}

static s32 remotePlay_ClearLinkNode( ifly_remote_player_t*	pLnkCtrl )
{
	s32 rtn = 0;
	
	if(!pLnkCtrl)
	{
		rtn = -1;
		goto CLR_ERR;
	}

	if(pLnkCtrl->playback_sockfd!=INVALID_SOCKET)
	{
		close(pLnkCtrl->playback_sockfd);
		pLnkCtrl->playback_sockfd = INVALID_SOCKET;
	}
	
	pLnkCtrl->byStop = 1;
	pLnkCtrl->byUse = 0;
	
CLR_ERR:
	if(rtn)
	{
		NETCOMM_DEBUG_STR("clear lnk node err!!!", rtn);
	}
	
	NETCOMM_DEBUG_STR("Exit remotePlay_ClearLinkNode!!!", rtn);
			
	return rtn;
}


static s32 remotePlay_InitAFrameHeader( 
		custommp4_t * file,
		FRAMEHDR* pFrameHead
)
{
	u32 raw_audio = str2uint("GRAW");
	pFrameHead->m_byAudioMode  = nInitAudioMode;
	pFrameHead->m_dwPreBufSize = 0;
	pFrameHead->m_byFrameRate  = nInitAudioFrameRate;
	
	pFrameHead->m_byMediaType  = nInitAudioMediaType;	
	
	if(custommp4_audio_compressor(file) == raw_audio)
		pFrameHead->m_byMediaType  = nInitAudioMediaType;
	else
		pFrameHead->m_byMediaType  = nInitAudioMediaType+21;// temp debug MEDIA_TYPE_ADPCM;
	
	return 0;
}

static s32 remotePlay_InitVFrameHeader( 
		FRAMEHDR* pFrameHead, 
		u16 nFrameWidth, 
		u16 nFrameHeight
)
{
	//u16 nFW, nFH;
	s32 rtn = 0;

	NETCOMM_DEBUG_STR("nFrameWidth", nFrameWidth);
	NETCOMM_DEBUG_STR("nFrameHeight", nFrameHeight);
	
	pFrameHead->m_tVideoParam.m_wVideoWidth  = nFrameWidth;
	pFrameHead->m_tVideoParam.m_wVideoHeight = nFrameHeight;

	pFrameHead->m_dwPreBufSize = 0;
	pFrameHead->m_byMediaType  = nInitVideMediaType;
	pFrameHead->m_byFrameRate  = nInitVideFrameRate;
	
	NETCOMM_DEBUG_STR("Exit remotePlay_InitVFrameHeader!!!", rtn);

	return rtn;
}

s32 remoteTimePlay_Request(u32 nChMsk, ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock, u32 nAckId)
{				
	u8	j,z;
	s32 rtn = CTRL_SUCCESS;
	ifly_remote_player_t* pLnkCtrl;
	pthread_t 		remoteTimePlayThxd;
	u32 mask_chn = 0;
	int available_chns=0, play_chns = 0;
	
	NETCOMM_DEBUG_STR("Enter remotePlay_Request !", 0);
	
	if(!pReq)
	{
		NETCOMM_DEBUG_STR("serious error, request null!", -1);
		
		exit(1);
	}
	
	mask_chn = nChMsk;

	printf("mash_chn %d nChnMax %d\n", mask_chn, nChnMax);

	if(mask_chn==0)
	{
		mask_chn=0xff;
	}
	
	for (j = 0; j < MAX_REMOTE_MULTI_PLAY_CHNS; j++) 
		if (0 == remote_multi_play_status[j]) available_chns++;
	
	for (j = 0; j < nChnMax; j ++) {
		if ((mask_chn >> j) & 1) {
			play_chns++;
			if (play_chns > available_chns) {
				printf("play_chns > available_chns:%d\n", available_chns);
				mask_chn &= ~(1 << j);
			}
		}
	}
	
	if (0 == play_chns || 0 == available_chns) {
		rtn = CTRL_FAILED_LINKLIMIT;

		printf("0 == play_chns || 0 == available_chns \n");
		goto END;
	}

	for (j = 0; j < MAX_REMOTE_MULTI_PLAY_CHNS; j++)
	{
		pLnkCtrl = &remote_multi_player[j];
		//printf("remote_player[j].byUse=%d\n",remote_player[j].byUse);
		if (0 == pLnkCtrl->byUse )
		{
			int temp_chns;
			remotePlay_InitLinkNode(pLnkCtrl,sock,nAckId);
			
			temp_chns = play_chns;
			for (z = 0; z < MAX_REMOTE_MULTI_PLAY_CHNS; z++) {
				if (temp_chns == 0) break;
				if (0 == remote_multi_play_status[z]) {
					if (temp_chns > 0) {
						remote_multi_play_status[z] = j + 1;
						temp_chns--;
					}
				}
			}
			break;
		}
	}

	if (MAX_REMOTE_MULTI_PLAY_CHNS <= j)
	{
		rtn = CTRL_FAILED_LINKLIMIT; 

		remotePlay_ClearLinkNode(pLnkCtrl);
		
		printf("MAX_REMOTE_MULTI_PLAY_CHNS <= %d\n", j);
		
		goto END;
	}

	//printf("(g_snd[i].req.TimePlayBack_t.channel)=%d\n", mask_chn);
	remote_multi_recsearch_param[j].channel_mask = mask_chn;
	remote_multi_recsearch_param[j].type_mask = pReq->MultiTimePlayBack_t.type;
	remote_multi_recsearch_param[j].start_time = pReq->MultiTimePlayBack_t.start_time;
	remote_multi_recsearch_param[j].end_time = pReq->MultiTimePlayBack_t.end_time;
	
	/*
	printf("remotePlayMultiWithTimeFxn start remote time player %d:(0x%x,%d,%d,0x%x)\n",j
		,remote_multi_recsearch_param[j].channel_mask,remote_multi_recsearch_param[j].start_time
		,remote_multi_recsearch_param[j].end_time,remote_multi_recsearch_param[j].type_mask);
	*/
	send_is_ok = FALSE;
	// create multitimeplay thread
	rtn = pthread_create(&remoteTimePlayThxd,
						 NULL,
						 remoteTimePlayFxn,
						 (void*)(u32)j);
	if(0 != rtn)
	{
		//pthread_exit(&remoteTimePlayThxd);
		
		rtn = CTRL_FAILED_RESOURCE;// fail to create preview thread
		
		remotePlay_ClearLinkNode(pLnkCtrl);
		
		goto END;
	}
	
	netComm_Ack(0, sock, nAckId);
	
	send_is_ok = TRUE;
	
END:
	if(rtn)
	{
		NETCOMM_DEBUG_STR("remotePlay_Request, err ", rtn);
	}
	
	return rtn;	
}

//判断是否为关键帧
BOOL is_i_frame(u8 video_type)
{
	//printf("frame type=%d\n",video_type);
	if(3 == video_type)
	{
		return video_type;
	}
	else
	{
	#define FRAME_TYPE_I 5
		//return (FRAME_TYPE_I == video_type);
		return (FRAME_TYPE_I == video_type)?3:0;
	}

	//pw
	//return (FRAME_TYPE_I == video_type)?3:0;
}

u64 DeleteHigh3Dot(u64 v)
{
	#if 1
	int i = 0;
	int k = 0;
	u64 tmp  = v;
	for(i = 0; i < 100; i++)
	{
		if(tmp == 0)
			break;
		tmp/=10;		
	}
	if(i<=3)
		return 0;
	tmp = 1;
	for(k = 0; k< i-3; k++)
		tmp*=10;
	v = v%tmp;
	#endif
	
	return v;
}

u32 getTimeStamp_Fixed()
{
	struct timeval tv;	
	gettimeofday(&tv, NULL);
	u64 v = (u64)tv.tv_sec*1000 + (tv.tv_usec)/1000;
	//v = DeleteHigh3Dot(v);
	//printf("getTimeStamp_Fixed %lu\n", (u32)(v&0xffffffff));
	return (u32)(v&0xffffffff);
}

void *remoteTimePlayFxn(void *arg)
{
	pthread_detach(pthread_self());
	
	printf("$$$$$$$$$$$$$$$$$$remoteTimePlayFxn id:%d\n",getpid());
	
	while(FALSE == send_is_ok)
	{
		usleep(50*1000);
	}
	
	int i, j;
	u32 index = (u32)arg;
	ifly_remote_player_t *pRemotePlayer = &remote_multi_player[index];
	char filename[64];
	
	//pw 2010/6/25
	#if 0// defined(REC_RETENTION_DAYS)
	ifly_reserved_t sp;
	GetReservedParam(&sp);
	
	time_t currtime;
	currtime = time(NULL) - sp.flag_timeoverwrt * 24 * 3600;
	#endif
	
	//printf("wait time:%d\n",count);
	
	//pw 2010/6/25
	#if 0//defined(REC_RETENTION_DAYS)
	if(sp.flag_timeoverwrt > 0) 
	{
		localsearch.start_time= (localsearch.start_time >= currtime ? localsearch.start_time : currtime);
		
		if(localsearch.end_time <= localsearch.start_time)
		{
			localsearch.end_time = localsearch.start_time;
		}
	}
	#endif
	
	/*
	printf("\",chn=0x%x,arg=%d start:%u end:%u\n",
		remote_multi_recsearch_param[index].channel_mask, 
		index, 
		remote_multi_recsearch_param[index].start_time, 
		remote_multi_recsearch_param[index].end_time
	);
	*/
	
	u32 total_search_num = 0;
	
	i = 0;
	
	#if 1
	// search record files
	//
	SSearchPara 	sSearchParaIns;
	SSearchResult 	sSearchResultIns;
	
	sSearchParaIns.nStartTime 		= remote_multi_recsearch_param[index].start_time;
	sSearchParaIns.nEndTime 		= remote_multi_recsearch_param[index].end_time;
	sSearchParaIns.nMaskType 		= remote_multi_recsearch_param[index].type_mask;
	sSearchParaIns.nMaskChn 		= 0;//remote_recsearch_param.mask;
	
	for (j = 0; j < nChnMax; j++) {
		if (remote_multi_recsearch_param[index].channel_mask & (1 << j)) {
			while (i < MAX_REMOTE_MULTI_PLAY_CHNS)  {
				if (remote_multi_play_status[i] == (index + 1)) {
					mutil_play_index[i] = 0;
					multi_frame_ready[i] = 0;
					seekpos_flag[i] = 0;
					multi_play_file[i] = NULL;
					
					sSearchParaIns.nMaskChn = (1 << j);

					sSearchResultIns.psRecfileInfo = (SRecfileInfo*)multi_playchn[i].rec_file_info;

					if( 0==ModSysComplexDMSearch(EM_FILE_REC, &sSearchParaIns, &sSearchResultIns, MAX_SEARCH_NUM) )
					{
						multi_playchn[i].real_file_nums = sSearchResultIns.nFileNum;
						
						if (multi_playchn[i].real_file_nums < 0) multi_playchn[i].real_file_nums = MAX_SEARCH_NUM;
						if (multi_playchn[i].real_file_nums > 0) {
							remote_multi_recsearch_param[index].start_time = max(remote_multi_recsearch_param[index].start_time
								, multi_playchn[i].rec_file_info[multi_playchn[i].real_file_nums-1].nStartTime);
							remote_multi_recsearch_param[index].end_time = min(remote_multi_recsearch_param[index].end_time
								, multi_playchn[i].rec_file_info[0].nEndTime);
						}
						printf("i=%d chn=%d seach_num=%d\n",  i, j, multi_playchn[i].real_file_nums);
						total_search_num += multi_playchn[i].real_file_nums;
						i++;
						
						break;
					}
				}
				i++;
			}
		}
	}
	
	if(total_search_num == 0) {
		// printf("search failed,total_search_num=%d\n",total_search_num);

		remotePlay_ClearMultiLinkNode(index, pRemotePlayer);

		goto MULTI_END;
	}
	#endif
	
	pRemotePlayer->byStop = 0;
	pRemotePlayer->byPause = 0;
	pRemotePlayer->byMute = 0;
	//pRemotePlayer->byStatus = 0;
	pRemotePlayer->bySkip = 0;
	pRemotePlayer->rate = PLAYRATE_1;

	pRemotePlayer->seekPos = 0;
	pRemotePlayer->curPos = 0;
	
	u32 open_offset, startpos, endpos;
	int file_nums;
	
	startpos = remote_multi_recsearch_param[index].start_time;	
	endpos = remote_multi_recsearch_param[index].end_time;
	
	#if 0//yzw add
	printf("startpos:%lu, endpos:%lu\n", startpos,endpos);
	startpos = (u32)(DeleteHigh3Dot((u64)startpos)&0xffffffff);
	endpos = (u32)(DeleteHigh3Dot((u64)endpos)&0xffffffff);
	printf("startpos:%lu, endpos:%lu\n", startpos,endpos);
	#endif

	pRemotePlayer->totalTime = endpos - startpos;

	//printf("totalTime=%u total_search_num=%u startpos=%u endpos=%u\n",pRemotePlayer->totalTime, total_search_num, startpos, endpos);

	ifly_progress_t play_progress;//07-08-25
	memset(&play_progress, 0, sizeof(ifly_play_progress_t));

	play_progress.currPos = htonl(pRemotePlayer->curPos);
	play_progress.totallen = htonl(pRemotePlayer->totalTime);

	u32 playPos = 0;
	pRemotePlayer->refTime = 0;

	u32 seekpos = 0;
	static u32 offset = 0xffffffff;

	u32 sect_start_pos[10];

	for(i = 0; i < 10; i++){
		sect_start_pos[i] =(endpos - startpos) * i / 10;
		// printf("sect_start_pos[%d]=%u\n", i, sect_start_pos[i]);
	}

	while (1) {
		//printf("loop 1*\n");
		//if (conn_lost_flag) break;
		
		if (pRemotePlayer->byStop) break;
		
		if (pRemotePlayer->byPause) {
			printf("mutil pause\n");
			if (pRemotePlayer->rate != PLAYRATE_SINGLE) {
				printf("pRemotePlayer->rate != PLAYRATE_SINGLE\n");
				usleep(20000);
				continue;
			}
		}

		if (pRemotePlayer->bySkip) {
			for (i = 9; i >= 0; i--) if (pRemotePlayer->curPos >= sect_start_pos[i]) break;

			//printf("pRemotePlayer->bySkip=%d i=%d pRemotePlayer->curPos=%u\n", pRemotePlayer->bySkip, i, pRemotePlayer->curPos);
			
			if (1 == pRemotePlayer->bySkip) i--;
			else if (2 == pRemotePlayer->bySkip) i++;

			//printf("2****pRemotePlayer->bySkip=%d i=%d\n", pRemotePlayer->bySkip, i);

			if (i < 0) i = 0;
			//if (i >= 10) i = 9;

			if  (i > 0 && i < 10) {
				pRemotePlayer->seekPos = sect_start_pos[i];
			} else if (0 == i)
				pRemotePlayer->seekPos = 1;

			//printf("3****pRemotePlayer->bySkip=%d seekPos=%d\n", pRemotePlayer->bySkip, pRemotePlayer->seekPos);
			pRemotePlayer->bySkip = 0;
			pRemotePlayer->refTime = 0;
		}
		
		
		//seek pos
		if (pRemotePlayer->seekPos) {
			if (1 == pRemotePlayer->seekPos) pRemotePlayer->seekPos = 0;
			seekpos = startpos + pRemotePlayer->seekPos;
			for (i = 0; i < MAX_REMOTE_MULTI_PLAY_CHNS; i++) {
				if (remote_multi_play_status[i] == (index + 1)) {
					multi_frame_ready[i] = 0;
					j = 0;
					file_nums = multi_playchn[i].real_file_nums;
					//printf("seek pos i=%d mutil_play_index[i]=%d file_nums=%d\n", i, mutil_play_index[i], file_nums);
					while (j >= 0  && j < file_nums) {
						//printf("j =%d file_nums=%d\n", j, file_nums);
						if (seekpos >= multi_playchn[i].rec_file_info[j].nEndTime) {
							j--;
							continue;
						} else if (seekpos < multi_playchn[i].rec_file_info[j].nStartTime) {
							
							if ((j + 1 < file_nums) && seekpos >= multi_playchn[i].rec_file_info[j + 1].nEndTime)
								break;
							else
								j++;
							continue;
						} else {
							break;
						}
					}

					if (j < 0)
						j = file_nums;
					else if (j >= file_nums)
						j = file_nums -1;
					else
						j = file_nums - j -1;
					
					//printf("pRemotePlayer->seekPos=%d seekpos=%d j=%d mutil_play_index[i]=%d\n", pRemotePlayer->seekPos
					//	, seekpos, j, mutil_play_index[i]);

					if (j != mutil_play_index[i] ) {
						if (multi_play_file[i]) custommp4_close(multi_play_file[i]);
						multi_play_file[i] = NULL;
						mutil_play_index[i] = j;
					} else if (multi_play_file[i] && j < file_nums && j >= 0) {
						if (seekpos >= multi_playchn[i].rec_file_info[j].nEndTime 
							/*|| seekpos < multi_playchn[i].rec_file_info[j].start_time*/) {
							//printf("seek error");
							custommp4_close(multi_play_file[i]);
							multi_play_file[i] = NULL;
						} else {
							seekpos_flag[i] = 1;
							custommp4_seek_to_sys_time(multi_play_file[i] , seekpos);
						}
					}
				}
			}
			pRemotePlayer->seekPos = 0;
			pRemotePlayer->refTime = 0;
		}
	
		//check if playover
		for (i = 0; i < MAX_REMOTE_MULTI_PLAY_CHNS; i++) {
			if (remote_multi_play_status[i] == (index + 1))
				if (mutil_play_index[i] < multi_playchn[i].real_file_nums) break;
		}

		if (i >= MAX_REMOTE_MULTI_PLAY_CHNS) break;
		
		//open files
		for (i = 0; i < MAX_REMOTE_MULTI_PLAY_CHNS; i++) {
			file_nums = multi_playchn[i].real_file_nums;
			j = file_nums - mutil_play_index[i] - 1;
			
			//printf("mutil_play_index[%d]=%d remote_multi_play_status[%d]=%d file_nums=%d index=%d j=%d\n",i, mutil_play_index[i]
			//	, i, remote_multi_play_status[i], file_nums, index, j);
			
			if (remote_multi_play_status[i] == (index + 1) && j >= 0 && j <  file_nums && (NULL == multi_play_file[i])) {
				get_rec_file_name((recfileinfo_t*)&multi_playchn[i].rec_file_info[j], filename, &open_offset);
				multi_play_file[i] = custommp4_open(filename, O_R, open_offset);
				if (multi_play_file[i]) {
					//printf("~~~~NNNN open %s ok i=%d\n", filename, i);
					if (seekpos < startpos) seekpos = startpos;

					//printf("seekpos=%u j=%d start_time=%u end_time=%d\n", seekpos, j, multi_playchn[i].rec_file_info[j].start_time
					//	, multi_playchn[i].rec_file_info[j].end_time);
					if (seekpos > multi_playchn[i].rec_file_info[j].nStartTime) {
						if(custommp4_seek_to_sys_time(multi_play_file[i], seekpos) < 0) {
							printf("custommp4_seek_to_sys_time failed\n");
							custommp4_close(multi_play_file[i] );
							multi_play_file[i]  = NULL;
							mutil_play_index[i] = file_nums;
							multi_frame_ready[i] = 0;
							continue;
						} else {
							seekpos_flag[i] = 1;
							multi_frame_ready[i] = 0;
							//pRemotePlayer->refTime = 0;
						}
					}

					remotePlay_InitVFrameHeader(
						&multi_play_hdr_v[i], 
						custommp4_video_width(multi_play_file[i]),
						custommp4_video_height(multi_play_file[i])
					);
					remotePlay_InitAFrameHeader(multi_play_file[i], &multi_play_hdr_a[i]);
				}else {
					//printf("~~~~NNNN open %s failed\n", filename);
					mutil_play_index[i] = file_nums;
				}
				//printf("~~~~mutil_play_index[%d]=%d\n", i, mutil_play_index[i]);
			}
		}
		
		u32 start_time = 0, min_start_time = 0xffffffff;
		
		//startpos = remote_multi_recsearch_param[index].start_time;
		
		//read frames
		u8 bVideo = 0;
		
		for (i = 0; i < MAX_REMOTE_MULTI_PLAY_CHNS; i++) {
			file_nums = multi_playchn[i].real_file_nums;
			j = file_nums - mutil_play_index[i] - 1;
			if (remote_multi_play_status[i] == (index + 1) ) {
				//printf("~~~~mutil_play_index[%d]=%d multi_frame_ready[%d]=%d\n", i, mutil_play_index[i], i, multi_frame_ready[i]);
				if (j >= 0 && j <  file_nums && multi_play_file[i] && 0 == multi_frame_ready[i]) {
					u8 media_type, key;
					u64 pts;
					int realsize = custommp4_read_one_media_frame(multi_play_file[i], multi_play_buf[i]
						, sizeof(multi_play_buf[i]), &start_time, &key, &pts, &media_type);
					key = is_i_frame(key);
					if (realsize <= 0) {
						//printf("mutil custommp4_read_one_media_frame failed\n");
						custommp4_close(multi_play_file[i] );
						multi_play_file[i] = NULL;
						mutil_play_index[i]++;
						multi_frame_ready[i] = 0;
						continue;
					}
					
					if (multi_playchn[i].rec_file_info[j].nStartTime + (start_time / 1000) > endpos) {
						//printf("arrival end pos:(%u,%u)\n", multi_playchn[i].rec_file_info[j].start_time+(start_time/1000),endpos);
						custommp4_close(multi_play_file[i] );
						multi_play_file[i] = NULL;
						mutil_play_index[i]++;
						multi_frame_ready[i] = 0;
						continue;
					}
					
					if (media_type) {
						multi_play_hdr_a[i].m_pData = multi_play_buf[i];
						multi_play_hdr_a[i].m_dwPreBufSize = ((multi_playchn[i].rec_file_info[j].nChn - 1) << 24);
						multi_play_hdr_a[i].m_dwDataSize = realsize;
						multi_play_hdr_a[i].m_dwTimeStamp = start_time;
						multi_frame_ready[i] = 2;//audio ready
						//printf("~~~~read audio ok chn:%d pts:%llu start_time:%u\n", multi_playchn[i].rec_file_info[j].channel_no
						//	, pts, start_time);
						
						bVideo = 0;
					} else {
						u64 div = 1000;
						
						u32 tmp = pts/div;
						
            			//printf("---------------nPts %llu pts/1k %u startpos %u\n", pts, tmp, startpos);
            			//printf("---------------pts/1k %u\n", tmp);
            			
						if (tmp < startpos*1000) tmp = startpos*1000;
						start_time = tmp - startpos*1000;
						/*
						*/
						if (min_start_time > start_time) min_start_time = start_time;
						
						multi_play_hdr_v[i].m_pData = multi_play_buf[i];
						multi_play_hdr_v[i].m_dwDataSize = realsize;
						multi_play_hdr_v[i].m_dwPreBufSize= ((multi_playchn[i].rec_file_info[j].nChn - 1) << 24);
						multi_play_hdr_v[i].m_tVideoParam.m_bKeyFrame = key;
						multi_play_hdr_v[i].m_dwTimeStamp = start_time;
						multi_frame_ready[i] = 1;//video ready
						bVideo = 1;
						
						//printf("~~~~read video ok chn:%d pts:%llu start_time:%u file start:%u\n", multi_playchn[i].rec_file_info[j].channel_no
						//	, pts, start_time,multi_playchn[i].rec_file_info[j].start_time);
					}
					
				} else if (multi_frame_ready[i] == 1) {
					//printf("multi_frame_ready[%d]=%d min_start_time=%u m_dwTimeStamp=%u\n", i, multi_frame_ready[i]
					//	, min_start_time, multi_play_hdr_v[i].m_dwTimeStamp);
					if (min_start_time > multi_play_hdr_v[i].m_dwTimeStamp)
						min_start_time = multi_play_hdr_v[i].m_dwTimeStamp;

					bVideo = 1;
						
					//printf("2*******multi_frame_ready[%d]=%d min_start_time=%u m_dwTimeStamp=%u\n", i, multi_frame_ready[i]
					//	, min_start_time, multi_play_hdr_v[i].m_dwTimeStamp);
				} else if (multi_frame_ready[i] == 2) {
				
					bVideo = 0;
					//printf("multi_frame_ready[%d]=%d\n", i, multi_frame_ready[i]);
					//if (min_start_time > multi_play_hdr_a[i].m_dwTimeStamp)
						//min_start_time = multi_play_hdr_a[i].m_dwTimeStamp;
				}
			}
		}

		int playrate = pRemotePlayer->rate;
		u32 arrive_time;
		int delaytime;
		
		if(bVideo)
		{
			if (pRemotePlayer->refTime == 0 && min_start_time < 0xffffffff) {
				//printf("mark first frame %u\n", min_start_time);
				playPos = min_start_time;
				pRemotePlayer->refTime = getTimeStamp();
			}
		
			start_time = min_start_time;
			//printf("time play playrate %d\n", playrate);
			if (playrate > 0) {
				arrive_time = pRemotePlayer->refTime + (start_time - playPos) / playrate;
				delaytime = arrive_time-getTimeStamp();
		
				//printf("video frame ready and delaytime %d line %d arrive_time %u stamp %u start_time %u playpos %u\n", delaytime, __LINE__, arrive_time, getTimeStamp(), start_time, playPos);
			}
			else if (playrate == 0) {
				//帧进
				arrive_time = pRemotePlayer->refTime + start_time - playPos;
				delaytime = arrive_time-getTimeStamp();
				pRemotePlayer->rate = 1;
			}
			else if (playrate == -1)	{
				//回退--只播放关键帧
				delaytime = 40;
				arrive_time = getTimeStamp()+delaytime;
			} else {
				arrive_time = pRemotePlayer->refTime+(start_time-playPos)*(-1)*playrate;
				delaytime = arrive_time-getTimeStamp();
			}

			if (delaytime > 10000) {
				delaytime = 40;
				pRemotePlayer->refTime = 0;
			}
			
			//printf("video frame ready and delaytime %d\n", delaytime);
			
			if (min_start_time < 0xffffffff) {
				if (delaytime > 0) {
					//printf("~~~~delaytime=%u\n", delaytime);
					struct timeval val;
					val.tv_sec  = delaytime / 1000;
					val.tv_usec = (delaytime % 1000) * 1000;
					select(0, NULL, NULL, NULL, &val);
				}
			}
		}
		
		//send frames
		for (i = 0; i < MAX_REMOTE_MULTI_PLAY_CHNS; i++) {
			file_nums = multi_playchn[i].real_file_nums;
			j = file_nums - mutil_play_index[i] - 1;
			if (remote_multi_play_status[i] ==(index + 1) ) {
				if (multi_frame_ready[i] == 2) {
					if (pRemotePlayer->playback_sockfd != INVALID_SOCKET && !pRemotePlayer->byMute && playrate == 1) {
						multi_play_hdr_a[i].m_dwTimeStamp = getTimeStamp();
						//int wRet = SendMediaFrameByTcpExt(pRemotePlayer->playback_sockfd, &multi_play_hdr_a[i], 1, 0);
						//printf("time play audio frame %d\n", multi_play_hdr_a[i].m_dwTimeStamp);
						int wRet = SendMediaFrameByTcpExt(pRemotePlayer->playback_sockfd, &multi_play_hdr_a[i], 0, 0);
						if (wRet) pRemotePlayer->byStop = 1;
					}
					multi_frame_ready[i] = 0; 
				} else if (multi_frame_ready[i] == 1) {
					playPos = min_start_time;
					start_time = multi_play_hdr_v[i].m_dwTimeStamp;
					if (start_time == min_start_time) {
						if (start_time > playPos) playPos = start_time;
						//printf("min_start_time[%lu], start_time[%lu], playPos[%lu], startpos[%lu], seekpos[%lu]\n", min_start_time,start_time,playPos,startpos,seekpos);

						#if 0
						if(offset == 0xffffffff)
							offset = start_time/1000 - DeleteHigh3Dot(startpos);
						if (playPos / 1000 < DeleteHigh3Dot(startpos)) {
							if (seekpos > startpos)
								pRemotePlayer->curPos = seekpos - startpos;
							else
								pRemotePlayer->curPos = 0;
							offset = 0;
						} else pRemotePlayer->curPos = playPos / 1000 - DeleteHigh3Dot(startpos) - offset;
						#else
							if (playPos/1000  < 0) {
								if (seekpos > startpos)
									pRemotePlayer->curPos = seekpos - startpos;
								else
									pRemotePlayer->curPos = 0;
							} else 
								pRemotePlayer->curPos = playPos/1000;//  - startpos;
						#endif
						
						pRemotePlayer->refTime = getTimeStamp();						
						multi_frame_ready[i] = 0;

						if (!seekpos_flag[i] || multi_play_hdr_v[i].m_tVideoParam.m_bKeyFrame) {
							multi_play_hdr_v[i].m_dwTimeStamp = getTimeStamp();
							multi_play_hdr_v[i].m_dwFrameID++;
							//int wRet = SendMediaFrameByTcpExt(pRemotePlayer->playback_sockfd, &multi_play_hdr_v[i], 1, 0);
							int wRet = SendMediaFrameByTcpExt(pRemotePlayer->playback_sockfd, &multi_play_hdr_v[i], 0, 1);
							if (wRet) pRemotePlayer->byStop = 1;
							seekpos_flag[i] = 0; 
							//printf("~send video wRet=%d i=%d chn:%d\n", wRet, i, multi_playchn[i].rec_file_info[j].channel_no);
						}

						//发送放像进度通知
						//intf("datasize:%u pRemotePlayer->totalTime=%u,curPos = %u\n", multi_play_hdr_v[i].m_dwDataSize
						//RemotePlayer->totalTime,pRemotePlayer->curPos);
						if(pRemotePlayer->byStatus) {
							//07-08-25
							play_progress.currPos = htonl(pRemotePlayer->curPos);
							play_progress.totallen = htonl(pRemotePlayer->totalTime);
							play_progress.id = htonl(pRemotePlayer->hid.id);
							CPPost(pRemotePlayer->cph,CTRL_NOTIFY_PLAYPROGRESS,&play_progress,sizeof(play_progress));
						}
						//printf("byStatus[%d],currPos[%d],totallen[%d],id[%d]\n", pRemotePlayer->byStatus, pRemotePlayer->curPos, pRemotePlayer->totalTime,pRemotePlayer->hid.id);
					}
/**/
				}
			}
		}
	}
	
	offset = 0xffffffff;
	
	printf("remoteTimePlay end, out of loop.\n");
	
	if (0 == pRemotePlayer->byStop)
	{
		CPPost(pRemotePlayer->cph,CTRL_NOTIFY_PLAYEND,&pRemotePlayer->hid,sizeof(pRemotePlayer->hid));
	}
	
#ifdef NEW_REMOTE_PLAYBACK
	close(pRemotePlayer->playback_sockfd);
	pRemotePlayer->playback_sockfd = INVALID_SOCKET;
#endif
	pRemotePlayer->byStop = 0;
	pRemotePlayer->byUse = 0;

	for (i = 0; i < MAX_REMOTE_MULTI_PLAY_CHNS; i++) {
		if (remote_multi_play_status[i] ==(index + 1)) {
			if (multi_play_file[i]) custommp4_close(multi_play_file[i]);
			multi_play_file[i] = NULL;
			remote_multi_play_status[i] = 0; 
		}
	}
	
MULTI_END:
	
	return 0;
}

#if 0
u16 DealPlayProgress( 
	CPHandle cph,
	u16 event,
	u8 *pbyMsgBuf,
	int msgLen,
	u8 *pbyAckBuf,
	int *pAckLen,
	void* pContext
)
{
	u32 id;
	int i;
	memcpy(&id,pbyMsgBuf,sizeof(u32));
	id = ntohl(id);
	*pAckLen = 0;
	u8 byStatus;
	memcpy(&byStatus,pbyMsgBuf+sizeof(id),sizeof(byStatus));
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		if(remote_player[i].byUse)
		{
			if(remote_player[i].hid.id == id)
			{
				printf("find remote player%d\n",i);
				break;
			}
		}
	}
	printf("sockfd=%d\n",cph->sockfd);
	if(i<MAX_REMOTE_PLAYER_NUM)
	{
		remote_player[i].byStatus = !(byStatus==0);
		remote_player[i].cph = cph;
	}
	
	return CTRL_SUCCESS;
}
#endif

s32 remotePlay_Stop(u32 nLnkId)
{
	int i;
	
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		ifly_remote_player_t* pCtrl = &remote_player[i];
		if(pCtrl->byUse)
		{
			if(pCtrl->hid.id == nLnkId)
			{
				remotePlay_ClearLinkNode(pCtrl);

				//pCtrl->byStop = 1;
				break;
			}
		}
	}

	return 0;
}

s32 remotePlay_FastPlay(u32 id)
{
	int i;
	
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		ifly_remote_player_t* pCtrl = &remote_player[i];
		
		if(pCtrl->byUse)
		{
			if(pCtrl->hid.id == id)
			{
				if(pCtrl->byPause)
				{
					pCtrl->rate = PLAYRATE_2;
					pCtrl->byPause = 0;
				}
				else
				{
					if(pCtrl->rate < PLAYRATE_2)
					{
						pCtrl->rate = PLAYRATE_2;
					}
					else
					{
						pCtrl->rate *= 2;
						if(pCtrl->rate > PLAYRATE_MAX)
						{
							pCtrl->rate = PLAYRATE_1;
						}
					}
				}
			}
		}
	}
	
	return CTRL_SUCCESS;
}

s32 remotePlay_SlowPlay(u32 id)
{
	int i;
	
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		ifly_remote_player_t* pCtrl = &remote_player[i];
		
		if(pCtrl->byUse)
		{
			if(pCtrl->hid.id == id)
			{
				if(pCtrl->byPause)
				{
					pCtrl->rate = PLAYRATE_1_2;
					pCtrl->byPause = 0;
				}
				else
				{
					if(pCtrl->rate < PLAYRATE_1_2)
					{
						pCtrl->rate = PLAYRATE_1_2;
					}
					else
					{
						pCtrl->rate /= 2;
						if(pCtrl->rate < PLAYRATE_MIN)
						{
							pCtrl->rate = PLAYRATE_1;
						}
					}
				}
			}
		}
	}
	
	return CTRL_SUCCESS;
}

s32 remotePlay_SetRate(u32 id, int rate)
{
	int i;
	
	if(rate > PLAYRATE_MAX || rate < PLAYRATE_MIN)
	{
		return CTRL_FAILED_PARAM;
	}
	
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		ifly_remote_player_t* pCtrl = &remote_player[i];
		if(pCtrl->byUse)
		{
			if(pCtrl->hid.id == id)
			{
				printf("find remote player%d\n",i);
				
				pCtrl->rate = rate;
				if(pCtrl->byPause) pCtrl->byPause = 0;
				
				break;
			}
		}
	}
	
	return CTRL_SUCCESS;
}

s32 remotePlay_Pause(u32 id)
{
	int i;
	
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		ifly_remote_player_t* pCtrl = &remote_player[i];

		if(pCtrl->byUse)
		{
			if(pCtrl->hid.id == id)
			{
				printf("find remote player%d\n",i);
				
				pCtrl->byPause = !pCtrl->byPause;
		
				break;
			}
		}
	}
	
	return CTRL_SUCCESS;
}

s32 remotePlay_Resume(u32 id)
{
	int i;
	
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		ifly_remote_player_t* pCtrl = &remote_player[i];
		if(pCtrl->byUse)
		{			
			if(pCtrl->hid.id == id)
			{
				printf("find remote player%d\n",i);
				
				pCtrl->byPause = 0;
		
				break;
			}
		}
	}
	
	return CTRL_SUCCESS;
}

s32 remotePlay_Step(u32 id)
{
	int i;
	
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		ifly_remote_player_t* pCtrl = &remote_player[i];

		if(pCtrl->byUse)
		{			
			if(pCtrl->hid.id == id)
			{
				printf("find remote player%d\n",i);
				
				if(pCtrl->byPause) 
					pCtrl->rate = PLAYRATE_SINGLE;
					
				break;
			}
		}
	}
	
	return CTRL_SUCCESS;
}

s32 remotePlay_PrevNext(u32 id, u8 bPrev)
{
	int i;
	
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		ifly_remote_player_t* pCtrl = &remote_player[i];

		if(pCtrl->byUse)
		{			
			if(pCtrl->hid.id == id)
			{
				printf("find remote player%d\n",i);
				
				if(bPrev) 
					pCtrl->bySkip = 1;
				else 
					pCtrl->bySkip = 2;
					
				break;
			}
		}
	}
	
	return CTRL_SUCCESS;
}


s32 remotePlay_Seek(u32 id, u32 nSeekPos)
{
	int i;
	
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		ifly_remote_player_t* pCtrl = &remote_player[i];

		if(pCtrl->byUse)
		{			
			if(pCtrl->hid.id == id)
			{
				printf("find remote player%d\n",i);
				
				pCtrl->seekPos = nSeekPos;
					
				break;
			}
		}
	}
	
	return CTRL_SUCCESS;
}


s32 remotePlay_Mute(u32 id, u8 bMute)
{
	int i;
	
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		ifly_remote_player_t* pCtrl = &remote_player[i];

		if(pCtrl->byUse)
		{			
			if(pCtrl->hid.id == id)
			{
				printf("find remote player%d\n",i);
				
				pCtrl->byMute = !(bMute==0);
					
				break;
			}
		}
	}
	
	return CTRL_SUCCESS;
}

s32 remotePlay_Progress(u32 id, CPHandle cph, u8 bStatus)
{
	int i;
	
	for(i=0;i<MAX_REMOTE_PLAYER_NUM;++i)
	{
		ifly_remote_player_t* pCtrl = &remote_player[i];
		
		if(pCtrl->byUse)
		{
			if(pCtrl->hid.id == id)
			{
				printf("find remote player%d\n",i);
				
				pCtrl->byStatus = !(bStatus==0);
				pCtrl->cph = cph;
		
				break;
			}
		}
	}
	
	return CTRL_SUCCESS;
}

