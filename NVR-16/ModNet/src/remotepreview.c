// file description
#include "remotepreview.h"
#include "loopbuf.h"
//#include "record.h"
#include "NetCommon.h"
//#include "mediastream.h"
#include "CtrlProtocol.h" 
#include <sys/types.h>
#include <linux/unistd.h>
#include "lib_audio.h"
#include <sys/syscall.h>
#include <unistd.h>
#include "utils.h"

//** macro

#define MAX_FRAME_SIZE				(s32)256*1024//zlb20100802
#define TL_AUDIO_MAX_BUF			(4*480*2)

#define NET_SEND_FRAME_SIZE_MAX		((256 << 10) + sizeof(IFLY_MediaFRAMEHDR_t))

//csp modify 20121120
//#define NET_SEND_FRAME_SIZE_MAX2	((1024 << 10) + sizeof(IFLY_MediaFRAMEHDR_t))
#define NET_SEND_FRAME_SIZE_MAX2	((NET_SEND_FRAME_SIZE_MAX)*2)

//#define PRINT_SND_TIME

//** global var
extern SNetCommCtrl sNetCommCtrl;

//** static functions
s32 mb_GetStatus( u8 nCh );
s32 mb_SendStream(FRAMEHDR* pFrame, u8 nCh);
static u16 SendMediaFrameByTcpFast(u8 *pFrameBuf, SOCKHANDLE sock_fd, PFRAMEHDR pFrmHdr, u8 bVideo) ;

//** global funciton


extern 
THREADHANDLE IFly_CreateThread(LINUXFUNC pvTaskEntry, char* szName, u8 byPriority, u32 dwStacksize, u32 dwParam, u16 wFlag, u32 *pdwTaskID);


//** typedef 

/*
 remote preview control block structure
*/
typedef struct _sRemotePreviewCtrl
{
	u8				bInit;
	u8				bVoipActive;		// 0 noactive, 1 active
	u32				yFactor;			// 
	u16				nStreamTcpNum;
	u16				nVidTcpNum;
	u16				nAudTcpNum;
	u8				nVoipMode;
	u8				nAudioMdType;
	u8				nVideoMediaType;
	PSNCSSCtrl			psVidTcpCtrl;	//[STREAM_TCP_CHANNELS];
	PSNCSSCtrl			psAudTcpCtrl;	//[CHN_NUM_MAIN];
	SVIOPCtrl			sVoipCtrlIns;
	PStreamRequestCB	pRmtPrvwCB;
	PRequestKeyFrameCB 	pReqKeyCB;
	PNetCommVOIPOut 	pVOIPCB;
	PNetCommVOIPIn 		pVOIPCBIn;
	u32 nMaxFrameSize;
} SRmtPrvwCtrl, *PSRmtPrvwCtrl;


//** local var

static SRmtPrvwCtrl sRmtPrvwIns;


//** global var

//** local functions
static 
void* mb_StreamThread( void* param );
static 
s32 remoteVoip_Init(SVIOPCtrl* pCtrl);

static 
void *RemoteVoipRecvThread(void *arg);
//static void* RemoteVoipSendThread(void* param);

s32 remoteVoip_GetPlayState();
static void* RemotePreviewThread(void* param);

//csp modify 20130423
static s32 remotePreview_InitLinkNode( 
		SNCSSCtrl1Link*	pLnkCtrl,
		SOCKHANDLE		sock,
		u8				bSndQcif,
		u32				nLinkId,
		u32				MonitorInfo
);

static s32 remotePreview_ClearLinkNode( 
		SNCSSCtrl1Link* pLnkCtrl
);

#if 0
static int get_thread_policy( pthread_attr_t* attr )
{
	int policy;
	//int rs = pthread_attr_getschedpolicy( attr, &policy );
	switch ( policy )
	{
		case SCHED_FIFO:
		printf("policy = SCHED_FIFO \n");
		break;

		case SCHED_RR:
		printf("policy = SCHED_RR \n");
		break;

		case SCHED_OTHER:
		printf("policy = SCHED_OTHER \n");
		break;

		default:
		printf("policy = UNKNOWN \n");
		break;
	}

	return policy;
}
#endif

s32 remotePreview_Init(
			u32 yFactor,
			u16 nStreamTcpNum,
			u16 nVidTcpNum,
			u16 nAudTcpNum,
			u8  nVoipModeIns,
			u8  nAudioMediaType,
			u8  nVideoMediaType,
			PStreamRequestCB pFxn,
			PRequestKeyFrameCB pReqKeyCB,
			PNetCommVOIPOut pVOIP,
			PNetCommVOIPIn pVOIPIn
)
{
	s32 rtn = 0;
	u8	i, j;
	
	pthread_t	mbThxd;
	
	memset(&sRmtPrvwIns, 0, sizeof(sRmtPrvwIns));
	
	SRmtPrvwCtrl* pCtrl = &sRmtPrvwIns;
	
	SNetCommCtrl *pNCCtrl = &sNetCommCtrl;
	
	pCtrl->pRmtPrvwCB = pFxn;
	if(!pFxn)
	{
		printf("callback null!!\n");
		rtn = -1;
		
		goto END;
	}
	pCtrl->pReqKeyCB	= pReqKeyCB;
	pCtrl->pVOIPCB		= pVOIP;
	pCtrl->pVOIPCBIn	= pVOIPIn;	
	
	// allocate memory for ctrl blocks
	pCtrl->nStreamTcpNum = nStreamTcpNum;
	pCtrl->nAudTcpNum = nAudTcpNum;
	pCtrl->nVidTcpNum = nVidTcpNum;
	pCtrl->nVoipMode  = nVoipModeIns;
	pCtrl->nVideoMediaType = nVideoMediaType;
	pCtrl->nAudioMdType = nAudioMediaType;
	
	pCtrl->psAudTcpCtrl = NULL;
	pCtrl->psAudTcpCtrl = malloc(nAudTcpNum*sizeof(SNCSSCtrl));
	pCtrl->psVidTcpCtrl = NULL;
	pCtrl->psVidTcpCtrl = malloc(nVidTcpNum*sizeof(SNCSSCtrl));
	
	if(!pCtrl->psAudTcpCtrl||!pCtrl->psVidTcpCtrl)
	{
		rtn = -1;
		
		NETCOMM_DEBUG_STR("psAudTcpCtrl NULL!", -1);
		
		goto END;
	}
	
	printf("nAudTcpNum %d nVidTcpNum %d nStreamTcpNum %d\n", nAudTcpNum, nVidTcpNum, nStreamTcpNum);
	
	for(i=0; i<nAudTcpNum; i++)
	{
		//csp modify 20130423
		pCtrl->psAudTcpCtrl[i].bLost = 0;
		pthread_mutex_init(&pCtrl->psAudTcpCtrl[i].LinkCtrlLock, NULL);
		
		for(j=0; j<EACH_STREAM_TCP_LINKS; j++)
		{
			pCtrl->psAudTcpCtrl[i].sLinkCtrl[j].sockfd = INVALID_SOCKET;
			pthread_mutex_init(&pCtrl->psAudTcpCtrl[i].sLinkCtrl[j].sockLock, NULL);
		}
	}
	
	for(i=0; i<nVidTcpNum; i++)
	{
		//csp modify 20130423
		pCtrl->psVidTcpCtrl[i].bLost = 0;
		pthread_mutex_init(&pCtrl->psVidTcpCtrl[i].LinkCtrlLock, NULL);
		
		for(j=0; j<EACH_STREAM_TCP_LINKS; j++)
		{
			pCtrl->psVidTcpCtrl[i].sLinkCtrl[j].sockfd = INVALID_SOCKET;
			pthread_mutex_init(&pCtrl->psVidTcpCtrl[i].sLinkCtrl[j].sockLock, NULL);
		}
	}
	
	pCtrl->yFactor = yFactor;
	
	//sleep(3);
	//printf("before RemotePreviewThread\n");
	
	//create preview stream thread
#if 1//set priority
	#ifdef CHIP_HISI3531
	//csp modify 20121016//子码流应该单独一个线程
	//堆栈不够会发生严重错误
	IFly_CreateThread(RemotePreviewThread, NULL, PRI_MEDIASND, (2<<20), (u32)&pNCCtrl->netsndMsgQ, 0, NULL);
	#else
	//csp modify 20121120
	//IFly_CreateThread(RemotePreviewThread, NULL, PRI_MEDIASND, STKSIZE_MEDIASND, (u32)&pNCCtrl->netsndAudioMsgQ, 0, NULL);//csp modify
	//csp modify 20140302
	////IFly_CreateThread(RemotePreviewThread, NULL, PRI_MEDIASND, STKSIZE_MEDIASND+64*1024, (u32)&pNCCtrl->netsndMsgQ, 0, NULL);
	IFly_CreateThread(RemotePreviewThread, NULL, PRI_MEDIASND, (1<<20), (u32)&pNCCtrl->netsndMsgQ, 0, NULL);
	#endif
#else
	rtn = pthread_create(&previewThxd,
							 NULL,
							 RemotePreviewThread,
							 NULL);
	if(0 != rtn)
	{
		//pthread_exit(&previewThxd);
		
		rtn = -1;//fail to create preview thread
		
		NETCOMM_DEBUG_STR("create RemotePreviewThread failed!", -1);
		goto END;
	}
	printf("RemotePreviewThread id : %d\n", previewThxd);
#endif
	
	//sleep(3);
	//printf("before mb_StreamThread\n");
	
	//create mobile stream thread
	rtn = pthread_create(&mbThxd,
							 NULL,
							 mb_StreamThread,
							 NULL);
	if(0 != rtn)
	{
		//pthread_exit(&mbThxd);
		
		rtn = -1;//fail to create preview thread
		
		NETCOMM_DEBUG_STR("create mb_StreamThread failed!", -1);
		goto END;
	}
	
	//sleep(3);
	//printf("before remoteVoip_Init\n");
	
	//voip init
	if( 0!= remoteVoip_Init(&pCtrl->sVoipCtrlIns) )
	{
		rtn = -1;
		
		NETCOMM_DEBUG_STR("create remoteVoip_Init failed!", -1);
		
		goto END;
	}
	
	//sleep(3);
	//printf("after remoteVoip_Init\n");
	
END:
	if(rtn)
	{
		if(pCtrl->psAudTcpCtrl) free(pCtrl->psAudTcpCtrl);
		if(pCtrl->psVidTcpCtrl) free(pCtrl->psVidTcpCtrl);
		NETCOMM_DEBUG_STR("Init failed!", -1);
	}
	else
	{
		pCtrl->bInit = 1;
		NETCOMM_DEBUG_STR("Init success!", 0);
	}
	
	return rtn;
}

s32 remotePreview_Request(ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock, u32 nId)
{
	u8	j;
	s32 rtn = 0;
	u8 bSubStream = 0;
	
	PSNCSSCtrl pStmSndCtrl = NULL;//pVidSndCtrl, pAudSndCtrl;
	
	//printf("%s ch %d sock %d\n", __FUNCTION__, pReq->Monitor_t.chn, sock);
	
	SRmtPrvwCtrl* pCtrl = &sRmtPrvwIns;
	
	if(!pReq)
	{
		NETCOMM_DEBUG_STR("serious error, request null!", -1);
		return CTRL_FAILED_PARAM;//exit(1);//csp modify
	}
	
	u8 chn = pReq->Monitor_t.chn;
	if(chn >= pCtrl->nStreamTcpNum)
	{
		rtn = CTRL_FAILED_CHNERROR;
		return rtn;
	}
	
	pStmSndCtrl = &pCtrl->psVidTcpCtrl[chn];
	
	//NETCOMM_DEBUG_STR("req chn ok!", chn);
	//NETCOMM_DEBUG_STR("req type ok!", pReq->Monitor_t.type);
	
	//notify up-layer to send stream
	STcpStreamReq sReq;
	sReq.nChn = pReq->Monitor_t.chn;
	
	//csp modify 20130423
	u32 MonitorInfo = 0;
	
	switch(pReq->Monitor_t.type)
	{
		case 0:
		{
			bSubStream = 0;
			
			//csp modify 20130423
			SNetThirdStreamProperty property;
			NetComm_GetThirdStreamProperty(&property);
			if(property.support && pReq->reserved[6] == 0x5A)
			{
				if(pReq->reserved[0] > property.wnd_num_threshold)
				{
				#if 1
					bSubStream = 1;
					
					sReq.nType = EM_PREQ_SUB;
					MonitorInfo = 0;
				#else
					sReq.nType = EM_PREQ_THIRD;
					MonitorInfo = pReq->reserved[0];//pReq->reserved[0] | (pReq->reserved[1] << 8);
				#endif
				}
				else
				{
					sReq.nType = EM_PREQ_MAIN;
				}
			}
			else
			{
				sReq.nType = EM_PREQ_MAIN;
			}
		} break;
		case 1:
		{
			pStmSndCtrl = &pCtrl->psAudTcpCtrl[chn];
			sReq.nType = EM_PREQ_AUDIO;
		} break;
		case 2:
		{
			#if 1//csp modify 20140318
			bSubStream = 1;
			sReq.nType = EM_PREQ_SUB;
			
			struct sockaddr_in peeraddr;	
			socklen_t len = sizeof(peeraddr);
			if(getpeername(sock, (struct sockaddr*)&peeraddr, &len) == 0)
			{
				if(peeraddr.sin_addr.s_addr == inet_addr("127.0.0.1"))
				{
					sReq.nType = EM_PREQ_THIRD;
				}
			}
			#else
			bSubStream = 1;
			sReq.nType = EM_PREQ_SUB;
			#endif
		} break;
		default:
		{
			printf("not supported preview type req!!!\n");
			return CTRL_FAILED_PARAM;
		} break;
	}
	
	//printf("pStmSndCtrl %p\n", pStmSndCtrl);
	
	SNCSSCtrl1Link*	pLnkCtrl = NULL;
	for(j = 0; j < EACH_STREAM_TCP_LINKS; j++)
	{
		pLnkCtrl = &pStmSndCtrl->sLinkCtrl[j];
		
		//printf("@@@ ch %d idx %d sub %d ::sock::%d id %d\n", chn, j, bSubStream, pLnkCtrl->sockfd, nId);
		
		if(INVALID_SOCKET == pLnkCtrl->sockfd)
		{
			break;
		}
		else
		{
			//NETCOMM_DEBUG_STR("used socket slot err!", j);
		}
	}
	
	if(j >= EACH_STREAM_TCP_LINKS)
	{
		rtn = CTRL_FAILED_LINKLIMIT;
	}
	else
	{
		//printf("### ch %d idx %d sub %d sock %d id %d\n", chn, j, bSubStream, pLnkCtrl->sockfd, nId);
		
		netComm_Ack(0, sock, nId);
		
		//csp modify 20130423
		if(0 == remotePreview_InitLinkNode(pLnkCtrl, sock, bSubStream, nId, MonitorInfo))
		{
			sReq.nAct = EM_START;
			(*pCtrl->pRmtPrvwCB)(&sReq);
		}
	}
	
	return rtn;
}

u16 SendMediaFrameByTcpFast(u8 *pFrameBuf, SOCKHANDLE sock_fd, PFRAMEHDR pFrmHdr, u8 bVideo) 
{
	IFLY_MediaFRAMEHDR_t tMediaHDR;
	int 	i_size;
	int 	ret;
	
	u8 *FrameBuf = NULL;
	
	if(pFrameBuf)
	{
		FrameBuf = pFrameBuf;
	}
	else
	{
		printf("error frame buff!\n");
		return 4;
	}
	
	tMediaHDR.m_wVideoWidth		= htonl(pFrmHdr->m_tVideoParam.m_wVideoWidth);
	tMediaHDR.m_wVideoHeight 	= htonl(pFrmHdr->m_tVideoParam.m_wVideoHeight);
	tMediaHDR.m_byMediaType 	= (pFrmHdr->m_byMediaType);
	
	tMediaHDR.m_byFrameRate 	= (pFrmHdr->m_byFrameRate);
	tMediaHDR.m_dwFrameID 		= htonl(pFrmHdr->m_dwFrameID);
	tMediaHDR.m_bKeyFrame 		= (pFrmHdr->m_tVideoParam.m_bKeyFrame);
	tMediaHDR.m_dwTimeStamp 	= htonl(pFrmHdr->m_dwTimeStamp);
	int tmp = pFrmHdr->m_dwDataSize;
	tMediaHDR.m_dwDataSize 		= htonl(pFrmHdr->m_dwDataSize);
	
	i_size = 0;
	memcpy(FrameBuf, &tMediaHDR, sizeof(tMediaHDR));
	i_size += sizeof(tMediaHDR);
	//printf("len:%dKB\n", tmp>>10);
	memcpy(FrameBuf + i_size, pFrmHdr->m_pData, tmp);
	//printf("len:0x%x\n", tmp);
	i_size += tmp;
	if(sock_fd != INVALID_SOCKET)
	{
		u32 remain = i_size;
		u32 sendlen = 0;
		int sendprelen = 0;
		
		//unsigned int t1 = getTimeStamp();
		
		//#define MTU_LIMIT	1460
		while(remain > 0)
		{
			sendprelen = remain;
			
			/*
			if(remain > MTU_LIMIT)
			{
				sendprelen = MTU_LIMIT;
			}
			else
			{
				sendprelen = remain;
			}
			*/
			
			ret = send(sock_fd, FrameBuf + sendlen, sendprelen, 0);
			if(ret < 0)
			{
				printf("tcp socket send failed sockfd=%d,ret = %d errno = %d, err msg = %s\n", sock_fd,ret, errno, strerror(errno));
				return 2;
			}
			else if(ret == 0)
			{
				printf("socket send ret = 0\n");
				return 3;
			}
			else if(ret != sendprelen)
			{
				//printf("want send=%d,real send=%d\n",sendprelen,ret);
			}
			sendlen += ret;
			remain -= ret;
		}
		
		//fsync(sock_fd);
		
		//unsigned int t2 = getTimeStamp();
		//if(ret > 0)
		//{
		//	printf("here,send %d bytes span %dms\n",ret,t2-t1);
		//}
	}
	else
	{
		//printf("sock_fd == INVALID_SOCKET\n");
		return 1;
	}
	
	return 0;
}

u16 SendMediaFrameByTcpExt(SOCKHANDLE sock_fd, PFRAMEHDR pFrmHdr, u8 b_mutil, u8 bVideo) 
{
	int 					ret = 0;
	IFLY_MediaFRAMEHDR_t 	tMediaHDR;
	SRmtPrvwCtrl* 			pCtrl = &sRmtPrvwIns;
	
	#ifdef CHIP_HISI3531
	//u8 					FrameBuf[NET_SEND_FRAME_SIZE_MAX2] = {0};
	u8 						FrameBuf[NET_SEND_FRAME_SIZE_MAX] = {0};
	#else
	u8 						FrameBuf[NET_SEND_FRAME_SIZE_MAX] = {0};
	#endif
    
	/*
	FRAMEHDR* pHdr = pFrmHdr;
	printf("tMediaHDR.m_wVideoWidth %d\n", pHdr->m_tVideoParam.m_wVideoWidth);
	printf("tMediaHDR.m_wVideoHeight %d\n", pHdr->m_tVideoParam.m_wVideoHeight);
	printf("tMediaHDR.m_byMediaType %d\n", pHdr->m_byMediaType);
	printf("tMediaHDR.m_byFrameRate %d\n", pHdr->m_byFrameRate);
	printf("tMediaHDR.m_dwFrameID %d\n", pHdr->m_dwFrameID);
	printf("tMediaHDR.m_bKeyFrame %d\n", pHdr->m_tVideoParam.m_bKeyFrame);
	printf("tMediaHDR.m_dwTimeStamp %lld\n", pHdr->m_dwTimeStamp);
	printf("tMediaHDR.m_dwDataSize %d\n", pHdr->m_dwDataSize);
	*/
	
	int i_size = 0;
	int tmp = pFrmHdr->m_dwDataSize;
	
	if(b_mutil) 
	{
		if(pFrmHdr->m_byMediaType == pCtrl->nVideoMediaType)
		{
			tMediaHDR.m_wVideoWidth = htonl((pFrmHdr->m_tVideoParam.m_wVideoWidth & 0xffffff) | (pFrmHdr->m_dwPreBufSize & 0xff000000));
		}
		else
		{
			tMediaHDR.m_wVideoWidth = htonl(pFrmHdr->m_dwPreBufSize & 0xff000000);
		}
	}
	else
	{
		tMediaHDR.m_wVideoWidth = htonl(pFrmHdr->m_tVideoParam.m_wVideoWidth);
	}
	tMediaHDR.m_wVideoHeight 	= htonl(pFrmHdr->m_tVideoParam.m_wVideoHeight);
	tMediaHDR.m_byMediaType 	= (pFrmHdr->m_byMediaType);
	tMediaHDR.m_byFrameRate 	= (pFrmHdr->m_byFrameRate);
	tMediaHDR.m_dwFrameID 		= htonl(pFrmHdr->m_dwFrameID);
	tMediaHDR.m_bKeyFrame 		= (pFrmHdr->m_tVideoParam.m_bKeyFrame);
	tMediaHDR.m_dwTimeStamp 	= htonl(pFrmHdr->m_dwTimeStamp);
	tMediaHDR.m_dwDataSize 		= htonl(pFrmHdr->m_dwDataSize);
	
	if(sizeof(tMediaHDR) + tmp > sizeof(FrameBuf))
	{
		if(sock_fd != INVALID_SOCKET)
		{
			u32 remain = sizeof(tMediaHDR);
			u32 sendlen = 0;
			while(remain > 0)
			{
				int sendprelen = remain;
				ret = send(sock_fd, (u8 *)&tMediaHDR + sendlen, sendprelen, 0);
				if(ret < 0)
				{
					printf("tcp socket send failed sockfd = %d, ret = %d, errno = %d, err msg = %s\n", sock_fd, ret, errno, strerror(errno));
					return 2;
				}
				else if(ret == 0)
				{
					printf("socket send ret = 0\n");
					return 3;
				}
				sendlen += ret;
				remain -= ret;
			}
			
			remain = tmp;
			sendlen = 0;
			while(remain > 0)
			{
				int sendprelen = remain;
				ret = send(sock_fd, (u8 *)pFrmHdr->m_pData + sendlen, sendprelen, 0);
				if(ret < 0)
				{
					printf("tcp socket send failed sockfd = %d, ret = %d, errno = %d, err msg = %s\n", sock_fd, ret, errno, strerror(errno));
					return 2;
				}
				else if(ret == 0)
				{
					printf("socket send ret = 0\n");
					return 3;
				}
				sendlen += ret;
				remain -= ret;
			}
		}
		else
		{
			//printf("sock_fd == INVALID_SOCKET\n");
			return 1;
		}
	}
	else
	{
		i_size = 0;
		
		memcpy(FrameBuf, &tMediaHDR, sizeof(tMediaHDR));
		i_size += sizeof(tMediaHDR);
		
		memcpy(FrameBuf + i_size, pFrmHdr->m_pData, tmp);
		i_size += tmp;
		
		if(sock_fd != INVALID_SOCKET)
		{
			u32 remain = i_size;
			u32 sendlen = 0;
			while(remain > 0)
			{
				int sendprelen = remain;
				
				/*
				#define MTU_LIMIT	1460
				if(remain > MTU_LIMIT)
				{
					sendprelen = MTU_LIMIT;
				}
				else
				{
					sendprelen = remain;
				}
				*/
				
				ret = send(sock_fd, FrameBuf + sendlen, sendprelen, 0);
				if(ret < 0)
				{
					printf("tcp socket send failed sockfd = %d,ret = %d, errno = %d, err msg = %s\n", sock_fd, ret, errno, strerror(errno));
					return 2;
				}
				else if(ret == 0)
				{
					printf("socket send ret = 0\n");
					return 3;
				}
				sendlen += ret;
				remain -= ret;
			}
		}
		else
		{
			//printf("sock_fd == INVALID_SOCKET\n");
			return 1;
		}
	}
	
	return 0;
}

u16 SendMediaFrameByTcp(SOCKHANDLE sock_fd, PFRAMEHDR pFrmHdr, u8 bVideo )
{
	//struct timeval start, end;
	//u64 span;
	
	//gettimeofday(&start, NULL);
	u16 err = SendMediaFrameByTcpExt(sock_fd, pFrmHdr, 0, bVideo);
	//gettimeofday(&end, NULL);
	
	//span = (u64)end.tv_sec*1000000+end.tv_usec - ((u64)start.tv_sec*1000000+start.tv_usec);
	//printf("span:%llu ******\n", span);
	
	return err;
}

static s32 remotePreview_ClearLinkNode( 
		SNCSSCtrl1Link* pLnkCtrl
)
{
	s32 rtn = 0;
	
	pthread_mutex_lock(&pLnkCtrl->sockLock);
	
	//printf("lock socklock %d\n", __LINE__);
	
	pLnkCtrl->nLinkId = 0;//csp modify 20121225
	
	if(pLnkCtrl->sockfd != INVALID_SOCKET)
	{
		printf("remotePreview close socket:%d\n",pLnkCtrl->sockfd);
		//shutdown(pLnkCtrl->sockfd, 2);//csp modify 20121225
		close(pLnkCtrl->sockfd);
		pLnkCtrl->sockfd = INVALID_SOCKET;
		//usleep(5);//csp modify 20121225
	}
	else
	{
		rtn = -1;
	}
	
	//pLnkCtrl->nLinkId = 0;//csp modify 20121225
	
	pLnkCtrl->VMonitorInfo = 0;//csp modify 20130423
	
	pthread_mutex_unlock(&pLnkCtrl->sockLock);
	
	if(rtn)
	{
		NETCOMM_DEBUG_STR("clear lnk node err!!!", rtn);
	}
	
	NETCOMM_DEBUG_STR("Exit remotePreview_ClearLinkNode!!!", rtn);
	
	return rtn;
}

//csp modify 20130423
static s32 remotePreview_InitLinkNode(
		SNCSSCtrl1Link*	pLnkCtrl,
		SOCKHANDLE		sock,
		u8				bSubStream,
		u32				nId,
		u32				MonitorInfo
)
{
	s32 rtn = 0;
	struct timeval tv;
	
	NETCOMM_ASSERT(pLnkCtrl);
	
	pthread_mutex_lock(&pLnkCtrl->sockLock);
	
	//printf("lock socklock %d\n", __LINE__);
	
	pLnkCtrl->streamtype = bSubStream?2:0;//csp modify 20140318
	
	#if 1
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	
	//csp modify 20140108
	struct sockaddr_in peeraddr;	
	socklen_t len = sizeof(peeraddr);
	if(getpeername(sock, (struct sockaddr*)&peeraddr, &len) != 0)
	{
		perror("getpeername");
	}
	else
	{
		if(peeraddr.sin_addr.s_addr == inet_addr("127.0.0.1"))
		{
			printf("remotePreview_InitLinkNode:127.0.0.1\n");
			tv.tv_sec = 30;
			tv.tv_usec = 0;

			pLnkCtrl->streamtype = 3;//csp modify 20140318
		}
		else
		{
			//printf("remotePreview_InitLinkNode:%s\n",inet_ntoa(peeraddr.sin_addr));
		}
	}
	#else
	tv.tv_sec = 0;//10;
	tv.tv_usec = 200*1000;
	#endif
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));
	
	#ifdef NETSND_OPT
	pLnkCtrl->nBps = 0xffffffff;
	pLnkCtrl->bKeyFrame = 0;
	#endif
	
	pLnkCtrl->m_dwFrameId		= 0;
	pLnkCtrl->m_dwsubFrameId	= 0;
	pLnkCtrl->bSub				= bSubStream;
	
	//csp modify 20130423
	pLnkCtrl->VMonitorInfo		= MonitorInfo;
	pLnkCtrl->m_dwThirdFrameId	= 0;
	
	pLnkCtrl->sockfd			= sock;
	pLnkCtrl->nLinkId			= nId;
	
	pthread_mutex_unlock(&pLnkCtrl->sockLock);
	
	return rtn;
}

/*
 mobile sub stream send thread
*/
void* mb_StreamThread( void* param )
{
	u8*		 pBuf;
	u16		 chn = 0;
	u32		 dwReadLen;
	FRAMEHDR tVideoFRAMEHDR;
	FRAMEHDR tAudioFRAMEHDR;
	
	SNetComStmHead nsheader;
	
	SNetCommCtrl *pNCCtrl = &sNetCommCtrl;
	
	// 清空数据流通道结构对应的相关标记
	// 包括是否第一次发送,	
	
	// 填充数据结构头
	
	tVideoFRAMEHDR.m_dwPreBufSize = 0;
	tAudioFRAMEHDR.m_dwPreBufSize = 0;
	
	printf("$$$$$$$$$$$$$$$$$$mb_StreamThread id:%d\n",getpid());
	
	while(1)
	{
		// 收取数据流通道队列
		if(!GetMsgQReadInfo(&pNCCtrl->netsndMbMsgQ,&pBuf,&dwReadLen))
		{
			NETCOMM_DEBUG_STR("~~~~~~read msg queue failed!!!, mb_StreamThread exit", -1);
			
			usleep(40*1000);
			break;
		}
		
		//printf("mb_StreamThread-1\n");
		
		// 处理相应队列数据
		memcpy(&nsheader,pBuf,sizeof(nsheader));
		
		chn = nsheader.byChnIndex;
		
		if(!mb_GetStatus(chn))
		{
			skipReadMsgQ(&pNCCtrl->netsndMbMsgQ);
			
			continue;
		}
		
		//printf("mb_StreamThread-2\n");
		
		//csp modify 20140525
		//if(nsheader.eFrameType==EM_VIDEO)
		if(nsheader.eFrameType==EM_THIRD)
		{
			//printf("mb_StreamThread-3\n");
			
			tVideoFRAMEHDR.m_tVideoParam.m_wVideoWidth  = nsheader.nWidth;
			tVideoFRAMEHDR.m_tVideoParam.m_wVideoHeight = nsheader.nHeight;
			
			tVideoFRAMEHDR.m_byMediaType  = nsheader.byMediaType;
			tVideoFRAMEHDR.m_byFrameRate  = nsheader.nFrameRate;
			
			tVideoFRAMEHDR.m_pData			 	= pBuf+sizeof(nsheader);
			tVideoFRAMEHDR.m_dwDataSize 		= nsheader.dwlen;
			tVideoFRAMEHDR.m_dwTimeStamp 		= nsheader.timeStamp;
			tVideoFRAMEHDR.m_tVideoParam.m_bKeyFrame = nsheader.byFrameType;//is_i_frame(nsheader.byFrameType);
			
			// mobile stream tx
			if(nsheader.bSub)
			{
				//debug_write(tVideoFRAMEHDR.m_pData, tVideoFRAMEHDR.m_dwDataSize);
				//printf("mb_SendStream ch%d size=%d w=%d h=%d timeStamp=%d\n", chn, nsheader.dwlen, nsheader.nWidth, nsheader.nHeight, nsheader.timeStamp);
				mb_SendStream(&tVideoFRAMEHDR, chn);
			}
		}
		#if 0
		else if(nsheader.eFrameType==EM_AUDIO)
		{
			tAudioFRAMEHDR.m_byMediaType  = nsheader.byMediaType;
			tAudioFRAMEHDR.m_byFrameRate  = nsheader.nFrameRate;
			tAudioFRAMEHDR.m_byAudioMode  = nsheader.nAudioMode;
			tAudioFRAMEHDR.m_dwDataSize = nsheader.dwlen;
			tAudioFRAMEHDR.m_dwTimeStamp =  nsheader.timeStamp;//getTimeStamp();
			
			tAudioFRAMEHDR.m_pData = pBuf + sizeof(nsheader);
			
			tAudioFRAMEHDR.m_dwPreBufSize = 0;
			tAudioFRAMEHDR.m_byFrameRate  = 33;
			
			b_lost_all = 1;
			
			PSNCSSCtrl pAudSndCtrl = &pRmtPrvwCtrl->psAudTcpCtrl[chn];
			
			for (j = 0; j < EACH_STREAM_TCP_LINKS; j++)
			{
				PSNCSSCtrl1Link	pLinkCtrl = &pAudSndCtrl->sLinkCtrl[j];

				if (pLinkCtrl->sockfd != INVALID_SOCKET)
				{
					pLinkCtrl->m_dwFrameId++;
					tAudioFRAMEHDR.m_dwFrameID = pLinkCtrl->m_dwFrameId;
					//printf("chn:%d,frameId:%d\n",chn+1, pLinkCtrl->m_dwFrameId);
					wRet = SendMediaFrameByTcp(pLinkCtrl->sockfd, &tAudioFRAMEHDR, 0);
					
					//debug_write(tAudioFRAMEHDR.m_pData, tAudioFRAMEHDR.m_dwDataSize);
					
					if (wRet)
					{
						if(remotePreview_ClearLinkNode(pLinkCtrl))
						{
							NETCOMM_DEBUG_STR("ClearLinkNode err!", wRet);
						}
					}
					else
					{
						b_lost_all = 0;
					}
				}
			}
		}
		#endif
		
		skipReadMsgQ(&pNCCtrl->netsndMbMsgQ);
	}
	
	return NULL;
}

/*
 A/V remote preview thread
*/
void* RemotePreviewThread(void* param)
{
	u8*		 pBuf;
	u8		 b_lost_all;
	u16		 chn, wRet;
	u32		 dwReadLen, j;
	FRAMEHDR tVideoFRAMEHDR;
	FRAMEHDR tAudioFRAMEHDR;
	
	#ifdef CHIP_HISI3531
	u8 sFrameBuf[NET_SEND_FRAME_SIZE_MAX2] = {0};//这里明显会越过此线程的堆栈
	#else
	//csp modify 20140302
	//u8 sFrameBuf[NET_SEND_FRAME_SIZE_MAX] = {0};
	u8 sFrameBuf[NET_SEND_FRAME_SIZE_MAX2] = {0};
	#endif
	
	//usleep(10*1000*1000);
	
	SNetComStmHead nsheader;
	
	SNetCommCtrl *pNCCtrl = &sNetCommCtrl;//csp modify 20140315
	SRmtPrvwCtrl* pRmtPrvwCtrl = &sRmtPrvwIns;
	
	//check thread id
	//NETCOMM_DEBUG_STR("RemotePreviewThread id: ", syscall(__NR_gettid));
	
	// 清空数据流通道结构对应的相关标记
	// 包括是否第一次发送,	
	
	//填充数据结构头
	
	tVideoFRAMEHDR.m_dwPreBufSize = 0;
	tAudioFRAMEHDR.m_dwPreBufSize = 0;
	
	NETCOMM_DEBUG_STR("Enter remotepreview thread...",0);
	
	ifly_msgQ_t* pQue = (ifly_msgQ_t*)param;
	
	printf("$$$$$$$$$$$$$$$$$$RemotePreviewThread id:%d\n",getpid());
	
	while(1)
	{
		//printf("GetMsgQReadInfo-1\n");
		
		// 收取数据流通道队列
		if(!GetMsgQReadInfo(pQue,&pBuf,&dwReadLen))
		{
			//NETCOMM_DEBUG_STR("~~~~~~read msg queue failed!!!, RemotePreviewThread exit", -1);
			printf("~~~~~~read msg queue failed!!!, RemotePreviewThread");
			
			usleep(1);//usleep(5*1000);//csp modify
			
			continue;
		}
		
		//printf("GetMsgQReadInfo-2\n");
		
		// 处理相应队列数据
		memcpy(&nsheader, pBuf, sizeof(nsheader));
		
		chn = nsheader.byChnIndex;
		PSNCSSCtrl pVidSndCtrl = &pRmtPrvwCtrl->psVidTcpCtrl[chn];
		
		if(nsheader.eFrameType == EM_VIDEO)
		{
			tVideoFRAMEHDR.m_tVideoParam.m_wVideoWidth  = nsheader.nWidth;
			tVideoFRAMEHDR.m_tVideoParam.m_wVideoHeight = nsheader.nHeight;
			//printf("RemotePreviewThread:chn%d w=%d h=%d\n",chn,tVideoFRAMEHDR.m_tVideoParam.m_wVideoWidth,tVideoFRAMEHDR.m_tVideoParam.m_wVideoHeight);
			
			tVideoFRAMEHDR.m_byMediaType  				= nsheader.byMediaType;
			tVideoFRAMEHDR.m_byFrameRate  				= nsheader.nFrameRate;
			
			tVideoFRAMEHDR.m_pData			 			= pBuf+sizeof(nsheader);
			tVideoFRAMEHDR.m_dwDataSize 				= nsheader.dwlen;
			tVideoFRAMEHDR.m_dwTimeStamp 				= nsheader.timeStamp;
			tVideoFRAMEHDR.m_tVideoParam.m_bKeyFrame 	= nsheader.byFrameType;//is_i_frame(nsheader.byFrameType);
			
			wRet = 0;
			b_lost_all = 1;
			
			// 检查该通道所有相应连接点是否可用
			for(j = 0; j < EACH_STREAM_TCP_LINKS; j++)
			{
				PSNCSSCtrl1Link	pLinkCtrl = &pVidSndCtrl->sLinkCtrl[j];
				
				//主/子码流占用相同的连接点//这样设计不是很合理//建议主码流和子码流分开
				if ( (pLinkCtrl->sockfd != INVALID_SOCKET) && 
					 (((nsheader.bSub!=0) && pLinkCtrl->bSub) || ((nsheader.bSub==0) && !pLinkCtrl->bSub))
				   )
				{
				#if 1//csp modify 20140318
					//csp modify 20130423
					SNetThirdStreamProperty property;
					NetComm_GetThirdStreamProperty(&property);
					if(property.support)
					{
						//csp modify 20140318
						//if(!pLinkCtrl->bSub && pLinkCtrl->VMonitorInfo > property.wnd_num_threshold)
						if(pLinkCtrl->bSub && pLinkCtrl->streamtype == 3)//此连接需要的是三码流
						{
							pVidSndCtrl->bLost &= ((0 == nsheader.bSub) ? 0xfe : 0xfd);
							continue;
						}
					}
				#endif
					
				#if 1//csp modify 20140315
					//检查是否I帧和是否需要发送I帧
					u8 idx = (pLinkCtrl->bSub ? 1 : 0);
					if(pNCCtrl->pnLostFrame[idx][chn] > 0 && !tVideoFRAMEHDR.m_tVideoParam.m_bKeyFrame)
					{
						b_lost_all = 0;
						break;
					}
					
					//pNCCtrl->pnLostFrame[idx][chn] = 0;//csp modify 20140315
				#endif
					
					//通道发送视频帧计数
					if(0 == nsheader.bSub)
					{
						pLinkCtrl->m_dwFrameId++;
						tVideoFRAMEHDR.m_dwFrameID = pLinkCtrl->m_dwFrameId;
					}
					else
					{
						pLinkCtrl->m_dwsubFrameId++;
						tVideoFRAMEHDR.m_dwFrameID = pLinkCtrl->m_dwsubFrameId;
					}
    				
    				pthread_mutex_lock(&pLinkCtrl->sockLock);
					wRet = SendMediaFrameByTcpFast(sFrameBuf, (SOCKHANDLE)pLinkCtrl->sockfd, &tVideoFRAMEHDR, 1);
    				pthread_mutex_unlock(&pLinkCtrl->sockLock);
					
					//if(chn == 0) printf("v:%u\n",nsheader.timeStamp);
					
					if(wRet)
					{
						//NETCOMM_DEBUG_STR("SendMediaFrameByTcp err!", wRet);
						
						printf("chn%d sockfd=%d SendMediaFrameByTcp err!\n", chn, pLinkCtrl->sockfd);
						
						// 释放相应队列节点
						if(remotePreview_ClearLinkNode(pLinkCtrl))
						{
							NETCOMM_DEBUG_STR("ClearLinkNode err!", wRet);
						}
					}
					else
					{
						b_lost_all = 0;
						
						//csp modify 20130423
						//pVidSndCtrl->bLost = 0;
						pVidSndCtrl->bLost &= ((0 == nsheader.bSub) ? 0xfe : 0xfd);
					}
				}
			}
			
			//csp modify 20130423
			//if(b_lost_all && !pVidSndCtrl->bLost)
			if(b_lost_all && !(pVidSndCtrl->bLost & ((0 == nsheader.bSub) ? 0x1 : 0x2)))
			{
				//stop preview stream sending
				STcpStreamReq sReq;
				sReq.nChn = chn;
				if(nsheader.bSub)
				{
					sReq.nType = EM_PREQ_SUB;
				}
				else
				{
					sReq.nType = EM_PREQ_MAX;
				}
				
				sReq.nAct = EM_STOP;
				
				(*pRmtPrvwCtrl->pRmtPrvwCB)(&sReq);
				
				b_lost_all = 0;
				
				//csp modify 20130423
				//pVidSndCtrl->bLost = 1;
				pVidSndCtrl->bLost |= ((0 == nsheader.bSub) ? 0x1 : 0x2);
			}
		}
		else if(nsheader.eFrameType==EM_THIRD)//csp modify 20130423
		{
			tVideoFRAMEHDR.m_tVideoParam.m_wVideoWidth  = nsheader.nWidth;
			tVideoFRAMEHDR.m_tVideoParam.m_wVideoHeight = nsheader.nHeight;
			//printf("RemotePreviewThread:chn%d w=%d h=%d\n",chn,tVideoFRAMEHDR.m_tVideoParam.m_wVideoWidth,tVideoFRAMEHDR.m_tVideoParam.m_wVideoHeight);
			
			tVideoFRAMEHDR.m_byMediaType  				= nsheader.byMediaType;
			tVideoFRAMEHDR.m_byFrameRate  				= nsheader.nFrameRate;
			
			tVideoFRAMEHDR.m_pData			 			= pBuf+sizeof(nsheader);
			tVideoFRAMEHDR.m_dwDataSize 				= nsheader.dwlen;
			tVideoFRAMEHDR.m_dwTimeStamp 				= nsheader.timeStamp;
			tVideoFRAMEHDR.m_tVideoParam.m_bKeyFrame 	= nsheader.byFrameType;//is_i_frame(nsheader.byFrameType);
			
			wRet = 0;
			b_lost_all = 1;
			
			// 检查该通道所有相应连接点是否可用
			for(j = 0; j < EACH_STREAM_TCP_LINKS; j++)
			{
				PSNCSSCtrl1Link	pLinkCtrl = &pVidSndCtrl->sLinkCtrl[j];
				
				//主/子码流占用相同的连接点//这样设计不是很合理//建议主码流和子码流分开
				//csp modify 20140318
				//if((pLinkCtrl->sockfd != INVALID_SOCKET) && (!pLinkCtrl->bSub))
				if((pLinkCtrl->sockfd != INVALID_SOCKET) && (pLinkCtrl->bSub))
				{
					SNetThirdStreamProperty property;
					NetComm_GetThirdStreamProperty(&property);
					//csp modify 20140318
					//if(pLinkCtrl->VMonitorInfo <= property.wnd_num_threshold)
					if(pLinkCtrl->streamtype != 3)//此连接需要的不是三码流
					{
						pVidSndCtrl->bLost &= 0xfb;
						continue;
					}
					
					#if 1//csp modify 20140315
					//检查是否I帧和是否需要发送I帧
					u8 idx = 2;
					if(pNCCtrl->pnLostFrame[idx][chn] > 0 && !tVideoFRAMEHDR.m_tVideoParam.m_bKeyFrame)
					{
						b_lost_all = 0;
						break;
					}
					
					//pNCCtrl->pnLostFrame[idx][chn] = 0;//csp modify 20140315
					#endif
					
					//通道发送视频帧计数
					pLinkCtrl->m_dwThirdFrameId++;
					tVideoFRAMEHDR.m_dwFrameID = pLinkCtrl->m_dwThirdFrameId;
					
    				pthread_mutex_lock(&pLinkCtrl->sockLock);
					wRet = SendMediaFrameByTcpFast(sFrameBuf, (SOCKHANDLE)pLinkCtrl->sockfd, &tVideoFRAMEHDR, 1);
    				pthread_mutex_unlock(&pLinkCtrl->sockLock);
					
					//if(chn == 0) printf("v:%u\n",nsheader.timeStamp);
					
					//printf("chn%d send third stream wRet:%d\n",chn,wRet);
					
					if(wRet)
					{
						NETCOMM_DEBUG_STR("SendMediaFrameByTcpFast err!", wRet);
						
						// 释放相应队列节点
						if(remotePreview_ClearLinkNode(pLinkCtrl))
						{
							NETCOMM_DEBUG_STR("ClearLinkNode err!", wRet);
						}
					}
					else
					{
						b_lost_all = 0;
						
						//csp modify 20130423
						//pVidSndCtrl->bLost = 0;
						pVidSndCtrl->bLost &= 0xfb;
					}
				}
			}
			
			if(b_lost_all && !(pVidSndCtrl->bLost & 0x4))
			{
				//stop preview stream sending
				STcpStreamReq sReq;
				sReq.nChn = chn;
				sReq.nType = EM_PREQ_THIRD;
				sReq.nAct = EM_STOP;
				
				(*pRmtPrvwCtrl->pRmtPrvwCB)(&sReq);
				
				b_lost_all = 0;
				
				pVidSndCtrl->bLost |= 0x4;
			}
		}
		else if(nsheader.eFrameType==EM_AUDIO)
		{
			tAudioFRAMEHDR.m_byMediaType	= 0;//nsheader.byMediaType;//csp modify
			tAudioFRAMEHDR.m_byAudioMode	= nsheader.nAudioMode;
			tAudioFRAMEHDR.m_dwDataSize		= nsheader.dwlen;
			tAudioFRAMEHDR.m_dwTimeStamp	= nsheader.timeStamp;//getTimeStamp();
			
			tAudioFRAMEHDR.m_pData = pBuf + sizeof(nsheader);
			
			tAudioFRAMEHDR.m_dwPreBufSize = 0;
			tAudioFRAMEHDR.m_byFrameRate  = 25;//33;//debug by lanston//csp modify
			
			b_lost_all = 1;
			
			PSNCSSCtrl pAudSndCtrl = &pRmtPrvwCtrl->psAudTcpCtrl[chn];
			
			for(j = 0; j < EACH_STREAM_TCP_LINKS; j++)
			{
				PSNCSSCtrl1Link	pLinkCtrl = &pAudSndCtrl->sLinkCtrl[j];
				
				if(pLinkCtrl->sockfd != INVALID_SOCKET)
				{
					pLinkCtrl->m_dwFrameId++;
					tAudioFRAMEHDR.m_dwFrameID = pLinkCtrl->m_dwFrameId;
					
					//printf("send audio %d bytes,MediaType:%d,AudioMode:%d\n",tAudioFRAMEHDR.m_dwDataSize,tAudioFRAMEHDR.m_byMediaType,tAudioFRAMEHDR.m_byAudioMode);
					
    				pthread_mutex_lock(&pLinkCtrl->sockLock);
					wRet = SendMediaFrameByTcpFast(sFrameBuf, pLinkCtrl->sockfd, &tAudioFRAMEHDR, 0);
    				pthread_mutex_unlock(&pLinkCtrl->sockLock);
					
					//if(chn == 0) printf("a:%u\n",nsheader.timeStamp);
					
					if(wRet)
					{
						if(remotePreview_ClearLinkNode(pLinkCtrl))
						{
							NETCOMM_DEBUG_STR("ClearLinkNode err!", wRet);
						}
					}
					else
					{
						b_lost_all = 0;
						pAudSndCtrl->bLost = 0;//cw_9508S
					}
				}
			}
			
			if(b_lost_all && !pAudSndCtrl->bLost)
			{
				//stop preview stream sending
				STcpStreamReq sReq;
				sReq.nChn = chn;
				sReq.nType = EM_PREQ_AUDIO;
				sReq.nAct = EM_STOP;
				(*pRmtPrvwCtrl->pRmtPrvwCB)(&sReq);
				
				b_lost_all = 0;
				pAudSndCtrl->bLost = 1;
			}
		}
		
		skipReadMsgQ(pQue);
	}
	
	return 0;
}

//** global functions
s32 remotePreview_StopPreview(u32 nLinkId, u8 bVideo)
{
	int         i, j;
	PSNCSSCtrl	pStmSndCtrl = NULL;//pVidSndCtrl, pAudSndCtrl;
	
	SRmtPrvwCtrl* pCtrl = &sRmtPrvwIns;
	
	//printf("link%d stop %s remote preview begin\n",nLinkId,bVideo?"video":"audio");
	
	if(bVideo)
	{
		#if 1//csp modify
		for(i=0; i<pCtrl->nVidTcpNum; i++)
		#else
		for(i=0; i<=pCtrl->nVidTcpNum; i++)
		#endif
		{
			pStmSndCtrl = &pCtrl->psVidTcpCtrl[i];
			for(j=0;j<EACH_STREAM_TCP_LINKS;j++)
			{
				SNCSSCtrl1Link* pLnk = &pStmSndCtrl->sLinkCtrl[j];
				if(nLinkId==pLnk->nLinkId)
				{
					remotePreview_ClearLinkNode(pLnk);
					break;
				}
			}
			
			if(j!=EACH_STREAM_TCP_LINKS)
			{
				break;
			}
		}
	}
	else
	{
		#if 1//csp modify
		for(i=0; i<pCtrl->nAudTcpNum; i++)
		#else
		for(i=0; i<=pCtrl->nAudTcpNum; i++)
		#endif
		{
			pStmSndCtrl = &pCtrl->psAudTcpCtrl[i];
			for(j=0;j<EACH_STREAM_TCP_LINKS;j++)
			{
				SNCSSCtrl1Link* pLnk = &pStmSndCtrl->sLinkCtrl[j];
				if(nLinkId==pLnk->nLinkId)
				{
					remotePreview_ClearLinkNode(pLnk);
					break;
				}
			}
			
			if(j!=EACH_STREAM_TCP_LINKS)
			{
				break;
			}
		}
	}
	
	//printf("link%d stop %s remote preview end\n",nLinkId,bVideo?"video":"audio");
	
	return 0;
}

//csp modify 20130423
s32 remotePreview_SetMonitorInfo(u32 nLinkId, u32 MonitorInfo)
{
	int         i, j;
	PSNCSSCtrl	pStmSndCtrl = NULL;
	
	SRmtPrvwCtrl* pCtrl = &sRmtPrvwIns;
	
	for(i=0; i<pCtrl->nVidTcpNum; i++)
	{
		pStmSndCtrl = &pCtrl->psVidTcpCtrl[i];
		for(j=0;j<EACH_STREAM_TCP_LINKS;j++)
		{
			SNCSSCtrl1Link* pLnk = &pStmSndCtrl->sLinkCtrl[j];
			if(nLinkId == pLnk->nLinkId)
			{
				//u32 LastVMonitorInfo = pLnk->VMonitorInfo;
				//pLnk->VMonitorInfo = MonitorInfo;
				pLnk->VMonitorInfo = 0;
				
				SNetThirdStreamProperty property;
				NetComm_GetThirdStreamProperty(&property);
				//if(LastVMonitorInfo <= property.wnd_num_threshold && MonitorInfo > property.wnd_num_threshold)
				if(!pLnk->bSub && MonitorInfo > property.wnd_num_threshold)
				{
					STcpStreamReq sReq;
					sReq.nChn = i;
					#if 1
					sReq.nType = EM_PREQ_SUB;
					pLnk->bSub = 1;
					#else
					sReq.nType = EM_PREQ_THIRD;
					#endif
					sReq.nAct = EM_START;
					
					(*pCtrl->pRmtPrvwCB)(&sReq);
				}
				//else if(LastVMonitorInfo > property.wnd_num_threshold && MonitorInfo <= property.wnd_num_threshold)
				else if(pLnk->bSub && MonitorInfo <= property.wnd_num_threshold)
				{
					STcpStreamReq sReq;
					sReq.nChn = i;
					#if 1
					sReq.nType = EM_PREQ_MAIN;
					pLnk->bSub = 0;
					#else
					sReq.nType = EM_PREQ_MAIN;
					#endif
					sReq.nAct = EM_START;
					
					(*pCtrl->pRmtPrvwCB)(&sReq);
				}
				
				return i;
			}
		}
	}
	
	return -1;
}

s32 remoteVoip_Init(SVIOPCtrl* pCtrl)
{
	s32 errCode;
	pthread_t threadIns;
	
	if(pCtrl)
	{
		memset(pCtrl, 0, sizeof(SVIOPCtrl));
	}
	else
		return -1;
	
	// start read thread
	errCode = pthread_create(&threadIns,
							 NULL,
							 RemoteVoipRecvThread,
							 NULL );
	if(0 != errCode)
	{
		//pthread_exit(&threadIns);
		
		errCode = -1;// fail to create cmd thread
		
		goto END;
	}
	
	#if 0
	// start send thread
	errCode = pthread_create(&threadIns,
							 NULL,
							 RemoteVoipSendThread,
							 NULL );
	if(0 != errCode)
	{
		//pthread_exit(&threadIns);
		
		errCode = -1;//fail to create cmd thread
		
		goto END;
	}
	#endif
END:
	if(errCode!=0)
		NETCOMM_DEBUG_STR("NetComm Init err ", errCode);
	
	return errCode;
}

s32 remoteVoip_InitLink(SVIOPCtrl* pCtrl,SOCKHANDLE sock, u32 nId)
{
	s32 rtn = 0;
	
	if(pCtrl)
	{
		if(!pCtrl->bUsed)
		{
			pCtrl->sockfd = sock;
			pCtrl->nLinkId = nId;
			pCtrl->bUsed = TRUE;
			
			//tl_audio_enable(TL_AUDIO_CH_TALK_BACK);
			//tl_audio_out_sel(TL_AUDIO_CH_TALK_BACK);
		}
		else
			rtn = CTRL_FAILED_BAUDLIMIT;
	}

	return rtn;
}

s32 remoteVoip_ClearLink(SVIOPCtrl* pCtrl)
{
	if(pCtrl)
	{
		#if 1//csp modify
		pCtrl->bUsed = FALSE;
		if(pCtrl->sockfd!=INVALID_SOCKET)
		{
			close(pCtrl->sockfd);
			pCtrl->sockfd = INVALID_SOCKET;
		}
		pCtrl->nLinkId = 0;
		#else
		if(pCtrl->sockfd!=INVALID_SOCKET)
		{
			close(pCtrl->sockfd);
			pCtrl->sockfd = INVALID_SOCKET;
		}
		pCtrl->nLinkId = 0;
		pCtrl->bUsed = FALSE;
		#endif
	}

	return 0;
}

u8 RemoteVoip_GetStatus( void )
{
	return sRmtPrvwIns.bVoipActive;
}

s32 remoteVoip_Stop(u32 nLinkId)
{
	printf("stop voip\n");
	fflush(stdout);
	
	SRmtPrvwCtrl* pCtrl = &sRmtPrvwIns;
	SVIOPCtrl* pVoipCtrl = &pCtrl->sVoipCtrlIns;
	
	if(pCtrl->bVoipActive) return 0;
	
	STcpStreamReq sReq;
	
	sReq.nChn  = 0;// useless now
	sReq.nAct  = EM_STOP;
	sReq.nType = EM_PREQ_VOIP;
	
	(*pCtrl->pRmtPrvwCB)(&sReq);
	
	if(nLinkId==pVoipCtrl->nLinkId)
	{
		remoteVoip_ClearLink(pVoipCtrl);
		
		//tl_audio_disable(TL_AUDIO_CH_TALK_BACK);
		//sReq.nType  = EM_PREQ_VOIP;
		//sReq.nAct	= EM_STOP;
		//(*pCtrl->pRmtPrvwCB)(&sReq);
	}
	
	pCtrl->bVoipActive = 1;
	
	printf("stop voip over\n");
	fflush(stdout);
	
	return 0;
}

s32 remoteVoip_Request(ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock, u32 nId)
{
	s32 ret = 0;
	
	SRmtPrvwCtrl* pCtrl = &sRmtPrvwIns;
	
	STcpStreamReq sReq;
	
	sReq.nChn  = 0;// useless now
	sReq.nAct  = EM_START;
	sReq.nType = EM_PREQ_VOIP;
	
	netComm_Ack(0, sock, nId);
	
	ret = remoteVoip_InitLink(&sRmtPrvwIns.sVoipCtrlIns, sock, nId);
	
	(*pCtrl->pRmtPrvwCB)(&sReq);
	
	pCtrl->bVoipActive = 0;
	
	return ret;
}

static u8 bTalkDataRecving = 0;

s32 RemoteVoipSendData( SVOIPFrame* pFrame )
{
	int      ret;
	FRAMEHDR tAudioFRAMEHDR;
	SRmtPrvwCtrl* pCtrl = &sRmtPrvwIns;
	SVIOPCtrl* pVoipCtrl = &pCtrl->sVoipCtrlIns;
	
	tAudioFRAMEHDR.m_byFrameRate = pFrame->nFrameRate;
	tAudioFRAMEHDR.m_byMediaType = pFrame->nMediaType;
	tAudioFRAMEHDR.m_byAudioMode = pFrame->nAudioMode;
	tAudioFRAMEHDR.m_pData		 = pFrame->pData;
	tAudioFRAMEHDR.m_dwDataSize  = pFrame->nSize;//sVoipStreamIns.nLen;	
	tAudioFRAMEHDR.m_dwTimeStamp = pFrame->nTStamp; //时间戳, 用于接收端
	
	/*
	printf("Local audio: ... \n");
	printf("Local audio: mode %d\n", tAudioFRAMEHDR.m_byAudioMode);
	printf("Local audio: size %d\n", tAudioFRAMEHDR.m_dwDataSize);
	printf("Local audio: stamp %d\n", tAudioFRAMEHDR.m_dwTimeStamp);
	*/
	
	ret = SendMediaFrameByTcp(pVoipCtrl->sockfd, &tAudioFRAMEHDR, 0); 
	if(ret && !bTalkDataRecving)
	{
		//remoteVoip_Stop(pVoipCtrl->nLinkId);
		//remoteVoip_ClearLink(pVoipCtrl);
	}
	
	return 0;
}

#if 0
void* RemoteVoipSendThread(void* param)
{
	int ret;
	//int i;
	FRAMEHDR tAudioFRAMEHDR;
	SRmtPrvwCtrl* pCtrl = &sRmtPrvwIns;
	SVIOPCtrl* pVoipCtrl = &pCtrl->sVoipCtrlIns;
	char audio_buf[TL_AUDIO_MAX_BUF];
	SVOIPStream  sVoipStreamIns; // in_frame;	
	
	audio_frame_t audio_frame;
	memset(&audio_frame,0,sizeof(audio_frame));
	
	tl_audio_open();
	//set_volume(0);
	
	memset(&sVoipStreamIns, 0, sizeof(sVoipStreamIns));
	
	printf("create RemoteVoipSendThread ... \n");
	
	while(1)
	{
		if (pVoipCtrl->bUsed&&pVoipCtrl->sockfd!=INVALID_SOCKET)
		{
			// read audio data
			// 
			sVoipStreamIns.nLen = TL_AUDIO_MAX_BUF;
			sVoipStreamIns.pData = audio_buf;

			#if 1
			ret = tl_audio_read_data(audio_buf,TL_AUDIO_MAX_BUF, &audio_frame,200*1000);			
			if(ret <0)
			{
				printf("enc faild errno=%d,errstr is %s\n",errno, strerror(errno));
				usleep(40*1000);
				continue;
			}
			else if(ret == 0)
			{
				printf("audio enc timeout audio frame len=%d\n",audio_frame.len);
				usleep(40*1000);
				continue;
			}
			else if(audio_frame.len <= 0||audio_frame.len >=TL_AUDIO_MAX_BUF)
			{
				printf("audio enc timeout audio frame len=%d\n",audio_frame.len);
				usleep(40*1000);
				continue;
			}
			else
			{
				//printf("audio enc success audio frame len=%d, chn:%d\n",audio_frame.len, audio_frame.channel);
			}
			#else
			(*pCtrl->pVOIPCBIn)(&sVoipStreamIns);
			#endif
			
			//
			tAudioFRAMEHDR.m_byAudioMode = pCtrl->nAudioMdType;
			tAudioFRAMEHDR.m_pData		 = audio_buf;
			tAudioFRAMEHDR.m_dwDataSize  = audio_frame.len;//sVoipStreamIns.nLen;
			
			tAudioFRAMEHDR.m_dwTimeStamp = audio_frame.time_stamp; //时间戳, 用于接收端

			printf("Local audio: ... \n");
			printf("Local audio: mode %d\n", tAudioFRAMEHDR.m_byAudioMode);
			printf("Local audio: size %d \n", tAudioFRAMEHDR.m_dwDataSize);
			printf("Local audio: stamp %d \n", tAudioFRAMEHDR.m_dwTimeStamp);
			
			ret = SendMediaFrameByTcp(pVoipCtrl->sockfd,&tAudioFRAMEHDR, 0); 
			if(ret)
			{
				remoteVoip_ClearLink(pVoipCtrl);
			}
			
		}
		else
		{
			usleep(400*1000);
		}
	}

	return NULL;
}
#endif

#define TALK_LOST_FRAME_MAX	10

void *RemoteVoipRecvThread(void *arg)
{
	IFLY_MediaFRAMEHDR_t tMediaHDR;
	SVOIPStream  		sVoipStreamIns; // in_frame;
	u32 remian 		= 0;
	u32 recvlen 	= 0;
	u64 count 		= 0;
	int ret			= 0;
	int lost_cnr	= 0;
	
	//csp modify
	//char buf[512] 	= {0};
	char buf[640] 	= {0};
	
	unsigned char frmaebuf[MAX_FRAME_SIZE] = {0};
	
	fd_set sock_fd;
	struct timeval tv;
	
	audio_frame_t in_frame;
	
	SRmtPrvwCtrl* pCtrl = &sRmtPrvwIns;
	SVIOPCtrl* pVoipCtrl = &pCtrl->sVoipCtrlIns;
	
	//printf("create RemoteVoipRecvThread ... \n");
	
	printf("$$$$$$$$$$$$$$$$$$RemoteVoipRecvThread id:%d\n",getpid());
	
#if 1
	while(1)
	{
		bTalkDataRecving = 0;
		
		if(pVoipCtrl->bUsed==FALSE || (INVALID_SOCKET==pVoipCtrl->sockfd))
		{
			//printf("voip wait ..................\n");
			//remoteVoip_ClearLink(pVoipCtrl);
			
			lost_cnr = 0;
			usleep(10*1000);//usleep(100*1000);//csp modify
			continue;
		}
		else
		{
			//printf("voip line %d\n", __LINE__);
			
			FD_ZERO(&sock_fd);
			FD_SET(pVoipCtrl->sockfd,&sock_fd);
			
			tv.tv_sec  = VOIP_IDLE_WAIT;
			tv.tv_usec = 0;
			ret = select(pVoipCtrl->sockfd+1, &sock_fd, NULL, NULL, &tv);
			if(ret <= 0)// timeout; 
			{
				// stop & close talk avoid pc talk stop exception
				if(++lost_cnr > TALK_LOST_FRAME_MAX)
				{
					printf("voip select failed:%d,stop voip\n",ret);
					remoteVoip_Stop(pVoipCtrl->nLinkId);
				}
				
				//memset(buf, 0, sizeof(buf));
				
				printf("no talk data come???\n");
				
				continue;
			}
			
			//printf("haha1\n");
			
			bTalkDataRecving = 1;
			
			lost_cnr = 0;
			
			if(FD_ISSET(pVoipCtrl->sockfd, &sock_fd))
			{
				remian = sizeof(IFLY_MediaFRAMEHDR_t);
				recvlen = 0;
				while(remian > 0)
				{
					if((!pVoipCtrl->bUsed)||(INVALID_SOCKET==pVoipCtrl->sockfd))
					{
						printf("1 voip fd=INVALID_SOCKET\n");
						goto end;
					}
					
					ret = recv(pVoipCtrl->sockfd,buf+recvlen,remian,0);
					if(ret <= 0)
					{
						///*
						// stop & close talk avoid pc talk stop exception
						//if(++lost_cnr > TALK_LOST_FRAME_MAX)//csp modify 20121225
						{
							remoteVoip_Stop(pVoipCtrl->nLinkId);
						}
						//*/
						
						printf("1 voip recv error ret=%d\n",ret);
						
						#if 1//csp modify
						usleep(1);
						goto end;
						#else
						usleep(200000);
						continue;
						#endif
					}
					recvlen += ret;
					remian -= ret;
				}
				
				lost_cnr = 0;
				
				memcpy(&tMediaHDR, buf, sizeof(tMediaHDR));
				
				recvlen = 0;
				remian = ntohl(tMediaHDR.m_dwDataSize);
				
				//if(remian != 642) printf("remian %d sock %d m_byMediaType %d\n", remian, pVoipCtrl->sockfd, tMediaHDR.m_byMediaType);
				
				remian = min(642,remian);
				//remian = 642;
				
				while(remian > 0)
				{
					if((!pVoipCtrl->bUsed)||(INVALID_SOCKET==pVoipCtrl->sockfd))
					{
						printf("2 voip fd=INVALID_SOCKET\n");
						goto end;
					}
					
					//printf("recv data 1\n");
					ret=recv(pVoipCtrl->sockfd,frmaebuf+recvlen,remian,0);
					//printf("recv data 2\n");
					if(ret <= 0)
					{
						///*
						// stop & close talk avoid pc talk stop exception
						//if(++lost_cnr > TALK_LOST_FRAME_MAX)//csp modify 20121225
						{
							remoteVoip_Stop(pVoipCtrl->nLinkId);
						}
						
						//memset(buf, 0, sizeof(buf));//csp modify 20121225
						//*/
						
						printf("2 voip recv error ret=%d\n",ret);
						
						#if 1//csp modify
						usleep(1);
						goto end;
						#else
						usleep(20000);
						continue;
						#endif
					}
					recvlen += ret;
					remian -= ret;
				}
				
				lost_cnr = 0;
				
				//printf("count=%lld,type=%d,PCMU=0\n",count,tMediaHDR.m_byMediaType);
				if(tMediaHDR.m_byMediaType == pCtrl->nVoipMode)//MEDIA_TYPE_ADPCM
				{
					if(pVoipCtrl->bUsed&&!remoteVoip_GetPlayState())
					{
						//printf("size=%d\n", ntohl(tMediaHDR.m_dwDataSize));
						memset(&sVoipStreamIns, 0, sizeof(sVoipStreamIns));
						sVoipStreamIns.pData 	= frmaebuf;
						sVoipStreamIns.nLen 	= ntohl(tMediaHDR.m_dwDataSize);
						sVoipStreamIns.bRaw 	= 1;
						
						//printf("Remote audio: ... \n");
						//printf("Remote audio: mode %d\n", pCtrl->nVoipMode);
						//printf("Remote audio: size %d\n", sVoipStreamIns.nLen);
						//printf("Remote audio: stamp %d\n", tAudioFRAMEHDR.m_dwTimeStamp);
						
						#if 1
						in_frame.data 			= frmaebuf;
						in_frame.len 			= sVoipStreamIns.nLen;
						in_frame.is_raw_data 	= 1;
						
						//debug_write(frmaebuf, in_frame.len);
						
						//printf("in_frame.len %d\n", in_frame.len);
						ret = tl_audio_write_data(&in_frame);
						//ret = in_frame.len;
						if(ret != in_frame.len)
						{
							printf("tl_audio_write_data error %d\n", ret);
						}
						#else
						(*pCtrl->pVOIPCB)(&sVoipStreamIns);
						#endif
						
						count++;
#ifdef VOIP_TEST
						//int ret = write(fd, frmaebuf, recvlen);
						//printf("fd:%d, write %d bytes\n", fd, ret);
#endif
					}					
				}
				
end:
				if(INVALID_SOCKET==pVoipCtrl->sockfd) printf("voip end\n");
				
				bTalkDataRecving = 0;
			}
		}
	}
#endif
	
	return 0;
}

s32 remoteVoip_GetPlayState()
{
	return 0;
}

// 0 - no stream, other - tx
s32 mb_GetStatus( u8 nCh )
{
	return mobile_get_status(nCh);
}

s32 mb_SendStream(FRAMEHDR* pFrame, u8 nCh)
{
	mobile_send_media_frame(pFrame, nCh);
	return 0;
}

s32 remotePreview_SendAudio( SNetComStmHead* pnsheader, u8* pBuf )
{
	u16 		wRet;
	int 		j;
	FRAMEHDR 	tAudioFRAMEHDR;
	SRmtPrvwCtrl* pRmtPrvwCtrl = &sRmtPrvwIns;
	static  u32 nFrameId = 0;
	
	tAudioFRAMEHDR.m_byMediaType  = pnsheader->byMediaType;
	tAudioFRAMEHDR.m_byAudioMode  = pnsheader->nAudioMode;
	tAudioFRAMEHDR.m_dwDataSize = pnsheader->dwlen;
	tAudioFRAMEHDR.m_dwTimeStamp =  pnsheader->timeStamp; //getTimeStamp();
	
	tAudioFRAMEHDR.m_pData = pBuf;
	tAudioFRAMEHDR.m_dwFrameID = nFrameId++;
	
	tAudioFRAMEHDR.m_dwPreBufSize = 0;
	tAudioFRAMEHDR.m_byFrameRate  = 33; // 25 debug by lanston
	
	PSNCSSCtrl pAudSndCtrl = &pRmtPrvwCtrl->psAudTcpCtrl[pnsheader->byChnIndex];
	
	for (j = 0; j < EACH_STREAM_TCP_LINKS; j++)
	{
		PSNCSSCtrl1Link	pLinkCtrl = &pAudSndCtrl->sLinkCtrl[j];

		if (pLinkCtrl->sockfd != INVALID_SOCKET)
		{
			pLinkCtrl->m_dwFrameId++;
			//printf("chn:%d,frameId:%d\n",chn+1, pLinkCtrl->m_dwFrameId);
			//printf("audio stamp %d\n", pnsheader->timeStamp);
			wRet = SendMediaFrameByTcp(pLinkCtrl->sockfd, &tAudioFRAMEHDR, 0);		
			if (wRet)
			{
				printf("ERROR: send audio link %d %d \n", j, pLinkCtrl->sockfd);
				if(remotePreview_ClearLinkNode(pLinkCtrl))
				{
					NETCOMM_DEBUG_STR("ClearLinkNode err!", wRet);
				}
			}
		}
	}
	return 0;
}

s32 remotePreview_SendVideo( SNetComStmHead* pnsheader, u8* pBuf )
{
	u16 		wRet;
	int 		j;
	FRAMEHDR 	tVideoFRAMEHDR;
	SRmtPrvwCtrl* pRmtPrvwCtrl = &sRmtPrvwIns;
	static  u32 nFrameId = 0;
	
	tVideoFRAMEHDR.m_tVideoParam.m_wVideoWidth  = pnsheader->nWidth;
	tVideoFRAMEHDR.m_tVideoParam.m_wVideoHeight = pnsheader->nHeight;
			
	tVideoFRAMEHDR.m_byMediaType  = pnsheader->byMediaType;
	tVideoFRAMEHDR.m_byAudioMode  = pnsheader->nAudioMode;
	tVideoFRAMEHDR.m_dwDataSize = pnsheader->dwlen;
	tVideoFRAMEHDR.m_dwTimeStamp =  pnsheader->timeStamp; //getTimeStamp();
	
	tVideoFRAMEHDR.m_pData = pBuf;
	tVideoFRAMEHDR.m_dwFrameID = nFrameId++;
	
	tVideoFRAMEHDR.m_dwPreBufSize = 0;
	tVideoFRAMEHDR.m_byFrameRate  = 25;//pnsheader->nFrameRate;
	
	tVideoFRAMEHDR.m_tVideoParam.m_bKeyFrame = pnsheader->byFrameType;//is_i_frame(nsheader.byFrameType);
	
	u8 chn = pnsheader->byChnIndex;
	
	PSNCSSCtrl pVidSndCtrl = &pRmtPrvwCtrl->psVidTcpCtrl[chn];
	
	for (j = 0; j < EACH_STREAM_TCP_LINKS; j++)
	{
		PSNCSSCtrl1Link	pLinkCtrl = &pVidSndCtrl->sLinkCtrl[j];

		if (pLinkCtrl->sockfd != INVALID_SOCKET)
		{
			pLinkCtrl->m_dwFrameId++;
			printf("chn:%d,frameId:%d\n",pnsheader->byChnIndex+1, tVideoFRAMEHDR.m_dwFrameID);
			//printf("audio stamp %d \n", pnsheader->timeStamp);
			wRet = SendMediaFrameByTcp(pLinkCtrl->sockfd, &tVideoFRAMEHDR, 1);		
			if (wRet)
			{
				printf("ERROR: send audio link %d %d \n", j, pLinkCtrl->sockfd);
				if(remotePreview_ClearLinkNode(pLinkCtrl))
				{
					NETCOMM_DEBUG_STR("ClearLinkNode err!", wRet);
				}
			}
		}
	}

	return 0;
}

