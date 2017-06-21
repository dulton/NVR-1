#include "ctrlprotocol.h"
#include "remoteDown.h"
#include "netcommon.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

#define MAX_REMOTE_DOWNLOADER_NUM	2
#define DOWN_BLOCK	(4096) //1460

typedef struct
{
	ifly_download_handle_t hid;
	ifly_remote_download_t remotedownload;
	u8   byUse;
	u8   byStop;
	SOCKHANDLE download_sockfd;
}ifly_remote_downloader_t;

static PRemotePlayCB pDownCB = NULL;
static ifly_remote_downloader_t remote_downloader[MAX_REMOTE_DOWNLOADER_NUM];

static void *remoteDownloadFxn(void *arg);

extern BOOL send_is_ok;

s32 remoteDownload_Init( PRemotePlayCB pCB )
{
	memset(remote_downloader, 0, 
		sizeof(ifly_remote_downloader_t)*MAX_REMOTE_DOWNLOADER_NUM);
	
	pDownCB = pCB;
	
	return 0;
}

static s32 remoteDownload_InitLinkNode(
		ifly_remote_downloader_t*	pLnkCtrl,
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

	pLnkCtrl->download_sockfd = sock;
	pLnkCtrl->byUse = 1;
	
	return rtn;
}

static s32 remoteDownload_ClearLinkNode( ifly_remote_downloader_t*	pLnkCtrl )
{
	s32 rtn = 0;
	
	if(!pLnkCtrl)
	{
		rtn = -1;
		goto CLR_ERR;
	}
	
	if(pLnkCtrl->download_sockfd!=INVALID_SOCKET)
	{
		printf("@@ close down sockfd %d\n", pLnkCtrl->download_sockfd);
		
		//shutdown(pLnkCtrl->download_sockfd, 2);//csp modify 20121225
		close(pLnkCtrl->download_sockfd);
		pLnkCtrl->download_sockfd = INVALID_SOCKET;
		usleep(5);
	}
	
	pLnkCtrl->byStop = 0;
	pLnkCtrl->byUse = 0;
	
CLR_ERR:
	if(rtn)
	{
		NETCOMM_DEBUG_STR("clear lnk node err!!!", rtn);
	}
	
	NETCOMM_DEBUG_STR("Exit remoteDownload_ClearLinkNode!!!", rtn);
			
	return rtn;
}

s32 remoteDownload_Request(ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock, u32 nAckId)
{		
	u8	j;
	s32 rtn = 0;
	ifly_remote_downloader_t* pLnkCtrl;
	pthread_t 		remoteDownThxd;
	
	NETCOMM_DEBUG_STR("Enter remoteDownload_Request !", 0);
	
	if(!pReq)
	{
		NETCOMM_DEBUG_STR("serious error, request null!", -1);
		
		exit(1);
	}
	
	for (j = 0; j < MAX_REMOTE_DOWNLOADER_NUM; j++)
	{
		pLnkCtrl = &remote_downloader[j];
		//printf("remote_player[j].byUse=%d\n",remote_player[j].byUse);
		if (0 == pLnkCtrl->byUse )
		{
			remoteDownload_InitLinkNode(pLnkCtrl,sock);

			printf("@@ got down sockfd %d\n", sock);
			break;
		}
	}	
	
	if (MAX_REMOTE_DOWNLOADER_NUM <= j)
	{		
		rtn = CTRL_FAILED_LINKLIMIT;

		goto END;
	}
	
	strcpy(pLnkCtrl->remotedownload.filename, pReq->FileDownLoad_t.filename);

	pLnkCtrl->remotedownload.offset = (pReq->FileDownLoad_t.offset);
	pLnkCtrl->remotedownload.size = (pReq->FileDownLoad_t.size);

	/*
	printf("start remote file downloader%d:(%s,%d,%d)\n",
		j,
		pLnkCtrl->remotedownload.filename,
		pLnkCtrl->remotedownload.offset,
		pLnkCtrl->remotedownload.size);
	*/

	int bReuseAddr = 1;
	if( setsockopt(pLnkCtrl->download_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&bReuseAddr, sizeof(bReuseAddr)) == -1 )
	{
		printf("setsockopt for remote download failed!\n");
	}
	else
		printf("setsockopt for remote download success!\n");

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
	//if(0 == (IFly_CreateThread(remoteDownloadFxn,NULL,THREAD_PRI_MID,1<<20,(u32)j,0,NULL)))
	rtn = pthread_create(&remoteDownThxd,
						 NULL,
						 remoteDownloadFxn,
						 (void*)((u32)j) );
	if(0!=rtn)
	{
		//pthread_exit(&remoteDownThxd);
		
		rtn = CTRL_FAILED_RESOURCE;// fail to create preview thread
		
		goto END;
	}

	netComm_Ack( 0, sock, nAckId );
	
	send_is_ok = TRUE;

END:
	if(rtn)
	{
		remoteDownload_ClearLinkNode(pLnkCtrl);
		NETCOMM_DEBUG_STR("remoteDownload_Request, err ", rtn);
	}
	
	return rtn;
}
void *remoteDownloadFxn(void *arg)
{
	printf("$$$$$$$$$$$$$$$$$$remoteDownloadFxn id:%d\n",getpid());
	
	while(FALSE == send_is_ok)
	{
		usleep(50*1000);
	}
	
	ifly_remote_downloader_t *pDownloader = &remote_downloader[(u32)arg];
	ifly_remote_download_t *rd = &pDownloader->remotedownload;
	
	pthread_detach(pthread_self());
	
	// open file
	// 
	#if 1
	void* pFileHandle = NULL;
	SRmtPlayFileOp sRmtPlayFileIns;
	
	memset(&sRmtPlayFileIns, 0, sizeof(sRmtPlayFileIns));
	
	sRmtPlayFileIns.eOpSrc = EM_FILEOP_DOWN;
	sRmtPlayFileIns.eOp = EM_RMTPLAY_OPEN;
	strcpy( sRmtPlayFileIns.OpenSeek.szFilePath, rd->filename );
	//sRmtPlayFileIns.OpenSeek.nOffset = rd->offset;	
	(*pDownCB)(NULL, &sRmtPlayFileIns);
	if(sRmtPlayFileIns.OpenSeek.pFile == NULL)
	{
		remoteDownload_ClearLinkNode(pDownloader);		
		return 0;
	}
	pFileHandle = sRmtPlayFileIns.OpenSeek.pFile;
	#endif
	
	NETCOMM_DEBUG_STR(rd->filename, 0);
	
	// seek postion
	//
	sRmtPlayFileIns.eOp = EM_RMTPLAY_SEEK;
	sRmtPlayFileIns.OpenSeek.nOffset = rd->offset;	
	(*pDownCB)(pFileHandle, &sRmtPlayFileIns);	
	if(sRmtPlayFileIns.nOpRslt<0)
	{
		remoteDownload_ClearLinkNode(pDownloader);
		
		goto ERROR;
	}
	
	NETCOMM_DEBUG_STR(rd->filename, rd->offset);
	
	// clear status
	//
	pDownloader->byStop = 0;
	
	char buf[DOWN_BLOCK];
	int remain = rd->size;
	int total = 0;

	// enter downloading loop
	//
	while(remain > 0)
	{
		// check status
		if(pDownloader->byStop)
		{
			printf("remoteDownloadFxn:stop\n");
			break;
		}

		// read file by block 
		//
		
		sRmtPlayFileIns.eOp = EM_RMTPLAY_GETFRAME;
		sRmtPlayFileIns.GetFrame.pFrameBuf = (u8 *)buf;
		sRmtPlayFileIns.GetFrame.nBufSize = min(remain,sizeof(buf));//sizeof(buf);
		(*pDownCB)(pFileHandle, &sRmtPlayFileIns);
		
		if(sRmtPlayFileIns.nOpRslt<0 || sRmtPlayFileIns.GetFrame.nFrameSize<0)
			break;
		
		//NETCOMM_DEBUG_STR("fread success(readsize =", sRmtPlayFileIns.GetFrame.nFrameSize);
		
		remain -= sRmtPlayFileIns.GetFrame.nFrameSize;

		// send data block
		//
		//NETCOMM_DEBUG_STR("download running......,readsize=", sRmtPlayFileIns.GetFrame.nFrameSize);
		//NETCOMM_DEBUG_STR("download running......,remain=", remain);
		//NETCOMM_DEBUG_STR("download running......,rd->size-remain=", rd->size-remain);
		
		int ret = 0;
		int writesize = 0;
		int readsize = sRmtPlayFileIns.GetFrame.nFrameSize;
		while(readsize > 0)
		{
			ret = send(pDownloader->download_sockfd,buf+writesize,readsize,MSG_NOSIGNAL);
			if(ret < 0)
			{
				printf("send failed,errcode=%d,errstr=%s\n",errno,strerror(errno));
				break;
			}
			
			writesize += ret;
			readsize -= ret;
			/*
			NETCOMM_DEBUG_STR("download running...... remain %d", readsize);
			NETCOMM_DEBUG_STR("download running...... MSG_OOB %d", MSG_OOB);
			NETCOMM_DEBUG_STR("download running...... send once", ret);
			NETCOMM_DEBUG_STR("download running...... send total", writesize);
			*/
		}

		// down progress notify
		//
		total +=writesize;
		if(ret < 0)
		{
			printf("send failed\n");
			break;
		}
		
		//usleep(50);//csp modify 20121122//Ô¶³ÌÏÂÔØÌ«ÂýÁË
	}
	
ERROR:
	usleep(2000*1000);//dong	100424
	
	// clear down handler
	///
	remoteDownload_ClearLinkNode(pDownloader);
	
	// close file
	sRmtPlayFileIns.eOp = EM_RMTPLAY_CLOSE;		
	(*pDownCB)(pFileHandle, &sRmtPlayFileIns);	
	
	// down status notify
	//
	printf("download finish\n");
	
	return 0;
}

