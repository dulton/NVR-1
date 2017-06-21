// file description
#include "CtrlProtocol.h"
#include "remoteTimeMultiPlay.h"

//** macro

//** typedef 

//** local var
BOOL send_is_ok;
static u32  sect_start_pos[RMTPLAY_PROG_SEG_MAX];
static PRemotePlayCB pPlayCB = NULL;
static 
u8		nInitVideMediaType;
u8		nInitAudioMediaType;
u8		nInitVideFrameRate;
u8		nInitAudioFrameRate;
u8		nInitAudioMode;

//** global var

//** local functions
static s32 remotePlay_InitFrameHeader( 
		FRAMEHDR* pFrameHead, 
		u16 nFrameWidth, 
		u16 nFrameHeight
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

//pw 2010/8/24 远程回放单个历史文件

/*
 play mode by time or file
*/
typedef enum
{
	EM_PlayMode_Time,
	EM_PlayMode_File,
	
} EM_PlayMode;

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
	u32  curPos;
	u32  totalTime;
	u32  refTime;
	u32  seekPos;
	SOCKHANDLE playback_sockfd;
} ifly_remote_player_t;

static ifly_remote_player_t remote_player[MAX_REMOTE_PLAYER_NUM];
static void *remotePlayFxn(void *arg);

static int remotePlay_ProgressNotify( u32 envent, u8 nIdx )
{
	ifly_progress_t play_progress;
	ifly_remote_player_t* pCtrl = &remote_player[nIdx];
	void* pVoid;
	int   nLen;
	
	int linkid = pCtrl->hid.id;

	if(envent!=CTRL_NOTIFY_PLAYEND)
	{		
		if(0 == pCtrl->byStop)
		{
			pVoid = &pCtrl->hid;
			nLen = sizeof(pCtrl->hid);
		}
	}
	else if(pCtrl->byStatus)
	{
		play_progress.currPos = htonl(pCtrl->curPos);
		play_progress.totallen = htonl(pCtrl->totalTime);
		play_progress.id = htonl(linkid);
		
		pVoid = &play_progress;
		nLen = sizeof(play_progress);
	}
	
	CPPost(pCtrl->cph,envent,pVoid,nLen);
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
	for(i=0;i<RMTPLAY_PROG_SEG_MAX;i++)
	{
		sect_start_pos[i] = nFileTotalTime * i / RMTPLAY_PROG_SEG_MAX;
	}

	return 0;
}

static int remotePlay_PlayDelay(u8 nIdx, int playrate, int start_time, int playPos, int seekpos_flag  )
{
	int rtn = 0;
	int delaytime, arrive_time;
	ifly_remote_player_t* pCtrl = &remote_player[nIdx];
	
	delaytime = 0;
	if(playrate > 0)
	{
		//NETCOMM_DEBUG_STR("playrate > 0\n",0);
		arrive_time = pCtrl->refTime+(start_time-playPos)/playrate;
		delaytime = arrive_time-getTimeStamp();
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
	int rtn = 0, i;
	u32 seekpos;
	
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
	u8 nAudioMode
)
{
	s32 rtn = 0;
	
	memset( &remote_player[0], 0, sizeof(ifly_remote_player_t)*MAX_REMOTE_PLAYER_NUM);

	pPlayCB = pRpCB;

	
	nInitVideMediaType = nVideMediaType;
	nInitAudioMediaType = nAudioMediaType;
	nInitVideFrameRate = nVideFrameRate;
	nInitAudioFrameRate = nAudioFrameRate;
	nInitAudioMode = nAudioMode;

	return rtn;
}

static s32 remotePlay_InitLinkNode(
		ifly_remote_player_t*	pLnkCtrl,
		SOCKHANDLE				sock
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
	pLnkCtrl->hid.id = GetNewIdNum();
	
	return rtn;
}

s32 remotePlay_Request(ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock)
{
	u8	j;
	s32 rtn = 0;
	ifly_remote_player_t* pLnkCtrl;
	pthread_t 		remotePlayThxd;

	NETCOMM_DEBUG_STR("Enter remotePlay_Request !", 0);

	if(!pReq)
	{
		NETCOMM_DEBUG_STR("serious error, request null!", -1);
		
		exit(1);
	}
	
	for (j = 0; j < MAX_REMOTE_PLAYER_NUM; j++)
	{
		pLnkCtrl = &remote_player[j];
		//printf("remote_player[j].byUse=%d\n",remote_player[j].byUse);
		if (0 == pLnkCtrl->byUse )
		{
			remotePlay_InitLinkNode(pLnkCtrl,sock);
			
			break;
		}
	}
	
	if (MAX_REMOTE_PLAYER_NUM <= j)
	{		
		rtn = CTRL_FAILED_LINKLIMIT;

		goto END;
	}

	strcpy(pLnkCtrl->remoteplay.filename, pReq->FilePlayBack_t.filename);
	//printf("app file name:%s,%s\n",remote_player[j].remoteplay.filename,g_snd[i].req.FilePlayBack_t.filename);

	pLnkCtrl->remoteplay.offset = ntohl(pReq->FilePlayBack_t.offset);

	// create preview stream thread
	rtn = pthread_create(&remotePlayThxd,
						 NULL,
						 remotePlayFxn,
						 (u32)j);
	if(0 != rtn)
	{
		//pthread_exit(&remotePlayThxd);
		
		rtn = CTRL_FAILED_RESOURCE;//fail to create preview thread
		
		remotePlay_ClearLinkNode(pLnkCtrl);
		
		goto END;
	}
	
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
	u32 seekpos;

	NETCOMM_DEBUG_STR("Enter remotePlayFxn ...", 0);
	
	// check if network tx is ok
	while(FALSE == send_is_ok)
	{
		usleep(RMTPLAY_TX_ERR_DELAY);
	}
	
	// set to confirm release all the resource after thread exit
	pthread_detach(pthread_self());
	
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
	
	ifly_remote_player_t 	*pRemotePlayer = &remote_player[nPlayerIdx];
	ifly_remote_playback_t 	*rp = &pRemotePlayer->remoteplay;
		
	FRAMEHDR tVideoFRAMEHDR;
	FRAMEHDR tAudioFRAMEHDR;

	void* pFileHandle = NULL;
	
	SRmtPlayFileOp sRmtPlayFileIns;

	memset(&sRmtPlayFileIns, 0, sizeof(sRmtPlayFileIns));	

	sRmtPlayFileIns.eOp = EM_RMTPLAY_OPEN;
	strcpy( sRmtPlayFileIns.OpenSeek.szFilePath, rp->filename );
	sRmtPlayFileIns.OpenSeek.nOffset = rp->offset;	
	(*pPlayCB)(NULL, &sRmtPlayFileIns);
	if(sRmtPlayFileIns.OpenSeek.pFile == NULL)
	{
		remotePlay_ClearLinkNode(pRemotePlayer);		
		return 0;
	}
	pFileHandle = sRmtPlayFileIns.OpenSeek.pFile;
	
	NETCOMM_DEBUG_STR("custommp4_open ok",0);
		
	u8 media_buf[256 << 10];
	u8 media_type,key;
	u64 pts;//wrchen 081226
	u32 start_time;
	int realsize = 1;
	int playrate = 1;

	sRmtPlayFileIns.eOp = EM_RMTPLAY_GetMediaFormat;
	(*pPlayCB)(pFileHandle, &sRmtPlayFileIns);
	
	sRmtPlayFileIns.eOp = EM_RMTPLAY_GetTimeLength;
	(*pPlayCB)(pFileHandle, &sRmtPlayFileIns);
	
	remotePlay_InitFrameHeader(
		&tVideoFRAMEHDR,
		sRmtPlayFileIns.GetFormat.nWidth,
		sRmtPlayFileIns.GetFormat.nHeight
	);

	// Split to little parts 
	// 		
	remotePlay_InitSeekSeg(nPlayerIdx, sRmtPlayFileIns.GetTimeLen.nTotalTime);

	int 	playPos = 0;
	u8 		seekpos_flag = 0;
	
	pRemotePlayer->refTime = 0;	
		
	// start read & send loop
	do
	{
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
				
		// check direction to play		
		// check skip, seekpos, move pos of file if TRUE
		//
		// seek to belonged segment
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
		
		// getframe & init reftime
		// 
		
		sRmtPlayFileIns.eOp = EM_RMTPLAY_GETFRAME;
		sRmtPlayFileIns.GetFrame.pFrameBuf = media_buf;
		sRmtPlayFileIns.GetFrame.nBufSize = sizeof(media_buf);
		
		(*pPlayCB)(pFileHandle, &sRmtPlayFileIns);	
		if(sRmtPlayFileIns.nOpRslt<0)
			break;	

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

		// send frame by media type
		// 
		if(media_type)
		{
			if(pRemotePlayer->playback_sockfd != INVALID_SOCKET && 
				!pRemotePlayer->byMute && playrate == 1)
			{
				tAudioFRAMEHDR.m_pData = media_buf;
				tAudioFRAMEHDR.m_dwDataSize = realsize;
				tAudioFRAMEHDR.m_dwTimeStamp = getTimeStamp();

				wRet = SendMediaFrameByTcp((SOCKHANDLE)pRemotePlayer->playback_sockfd, &tAudioFRAMEHDR, 0);
				if (wRet)
				{
					pRemotePlayer->byStop = 1;
					break;
				}
			}
		}
		else
		{
			// delay before send video frame
			//
			remotePlay_PlayDelay(nPlayerIdx,playrate,start_time,playPos,seekpos_flag);

			// fill head & send 
			pRemotePlayer->curPos = start_time/1000;
			playPos = start_time;
			pRemotePlayer->refTime = getTimeStamp();
			
			if (!seekpos_flag || key)
			{
				tVideoFRAMEHDR.m_pData = media_buf;
				tVideoFRAMEHDR.m_dwDataSize = realsize;
				tVideoFRAMEHDR.m_tVideoParam.m_bKeyFrame = key;
				tVideoFRAMEHDR.m_dwTimeStamp = getTimeStamp();

				tVideoFRAMEHDR.m_dwFrameID++;
				wRet = SendMediaFrameByTcp(pRemotePlayer->playback_sockfd, &tVideoFRAMEHDR, 1);
				if (wRet)
				{
					pRemotePlayer->byStop = 1;
					break;
				}

				seekpos_flag = 0;
			}
			
			//remotePlay_ProgressNotify(CTRL_NOTIFY_PLAYPROGRESS, nPlayerIdx);
		}
		
		// change delay as playspeed change or bPause

	} while(realsize > 0);

	// endloop

	// notify play end
	// release sources
ERROR:
	
	//发送放像结束通知
	remotePlay_ProgressNotify(CTRL_NOTIFY_PLAYEND, nPlayerIdx);

	// file close
	if(NULL!=pFileHandle)
	{
		sRmtPlayFileIns.eOp = EM_RMTPLAY_CLOSE;
		(*pPlayCB)(pFileHandle, &sRmtPlayFileIns);
		
		pFileHandle = NULL;
	}
	
	// clear replay link
	remotePlay_ClearLinkNode(pRemotePlayer);
	
	NETCOMM_DEBUG_STR("Exit remotePlayFxn !!!", 0);
	
	return 0;
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
	
	pLnkCtrl->byStop = 0;
	pLnkCtrl->byUse = 0;
	
CLR_ERR:
	if(rtn)
	{
		NETCOMM_DEBUG_STR("clear lnk node err!!!", rtn);
	}
	
	NETCOMM_DEBUG_STR("Exit remotePlay_ClearLinkNode!!!", rtn);
			
	return rtn;
}

static s32 remotePlay_InitFrameHeader( 
		FRAMEHDR* pFrameHead, 
		u16 nFrameWidth, 
		u16 nFrameHeight
)
{
	u16 nFW, nFH;
	s32 rtn = 0;

	NETCOMM_DEBUG_STR("nFrameWidth", nFrameWidth);
	NETCOMM_DEBUG_STR("nFrameHeight", nFrameHeight);
	
	pFrameHead->m_tVideoParam.m_wVideoWidth  = nFrameWidth;
	pFrameHead->m_tVideoParam.m_wVideoHeight = nFrameHeight;

	pFrameHead->m_dwPreBufSize = 0;
	pFrameHead->m_byMediaType  = nInitVideMediaType;
	pFrameHead->m_byFrameRate  = nInitVideFrameRate;
	
	pFrameHead->m_byAudioMode  = nInitAudioMode;
	pFrameHead->m_dwPreBufSize = 0;
	pFrameHead->m_byFrameRate  = nInitAudioFrameRate;

	pFrameHead->m_byMediaType  = nInitAudioMediaType;
	
	NETCOMM_DEBUG_STR("Exit remotePlay_InitFrameHeader!!!", rtn);
			
	return rtn;
}

