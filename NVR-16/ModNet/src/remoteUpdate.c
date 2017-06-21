//** include
//	
#include "ctrlprotocol.h"
#include "remoteUpdate.h"
#include "diskmanage.h"
#include "Mod_syscomplex.h"
#include <sys/reboot.h>
#include "lib_common.h"

#define FILE_DIR_MAX	12

#define REMOTE_UPDATE_BUFF_LEN	(9*1024*1024)

#define UPDATE_BLOCK	4096

typedef struct
{
	ifly_remote_upload_t 	remoteupload;
	CPHandle 				cph;
	int 					serversock;
	u32 					id;
} ifly_remote_updater_t;

// remote format control
typedef struct
{
	CPHandle 				cph;
	int 					sock;
	u32 					idx;
} SRemoteFormatCtrl;

//static PRemotePlayCB pDownCB = NULL;

static ifly_remote_updater_t 	remote_updater;
static SRemoteFormatCtrl		sRFormatCtrl;

static BOOL byUpdateFlag = 0;
static BOOL bFormatFlag = 0;
static disk_manager* pDiskMgr;

static void *remoteUpdateFxn(void *arg);

extern BOOL send_is_ok;

extern SNetCommCtrl sNetCommCtrl;

PFNSysResetCB  pStopRecCB;//cw_9508S

static s32 remoteFormat_ErrAck(SRemoteFormatCtrl *pCtrl, s32 err);
static s32 remoteFormat_InitLinkNode(SRemoteFormatCtrl*	pLnkCtrl, SOCKHANDLE sock);
static s32 remoteFormat_ClearLinkNode(SRemoteFormatCtrl* pLnkCtrl);
static void *remoteFormatFxn(void *arg);

s32 remoteUpdate_Init( void* pDMgr )
{
	memset(&remote_updater, 0, sizeof(remote_updater));
	
	pDiskMgr = (disk_manager*)pDMgr;
	
	byUpdateFlag = 0;
	
	pStopRecCB=GetRecStopCB();//cw_9508S
	
	return 0;
}

static s32 remoteUpdate_CheckCurStatus()
{
	// record status callback needed here
	//
	pStopRecCB();//cw_9508S
	return 0;
}


static s32 remoteUpdate_InitLinkNode(
		ifly_remote_updater_t*	pLnkCtrl,
		SOCKHANDLE				sock
)
{
	s32 rtn = 0;
	struct timeval tv;
	
	NETCOMM_ASSERT(pLnkCtrl);
	
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));
	
	pLnkCtrl->serversock = sock;
	
	return rtn;
}

static s32 remoteUpdate_ClearLinkNode( ifly_remote_updater_t* pLnkCtrl  )
{
	s32 rtn = 0;
	
	if(!pLnkCtrl)
	{
		rtn = -1;
		goto CLR_ERR;
	}

	if(pLnkCtrl->serversock!=INVALID_SOCKET)
	{
		close(pLnkCtrl->serversock);
		pLnkCtrl->serversock = INVALID_SOCKET;
	}

	byUpdateFlag = 0;
	
CLR_ERR:
	if(rtn)
	{
		NETCOMM_DEBUG_STR("clear lnk node err!!!", rtn);
	}
	
	NETCOMM_DEBUG_STR("Exit remoteUpdate_ClearLinkNode!!!", rtn);
	
	return rtn;
}

s32 remoteUpdate_ErrAck(ifly_remote_updater_t *pUpdater, s32 err)
{
	ifly_TCP_Pos backCode;
	
	memset(&backCode,0,sizeof(backCode));
	
	backCode.errcode = htons((u16)err);//err;//csp modify
	
	send(pUpdater->serversock,&backCode,sizeof(backCode),MSG_NOSIGNAL);
	
	return 0;
}

s32 remoteUpdate_Request(ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock, u32 nAckId)
{
	s32 rtn = 0;
	ifly_remote_updater_t* pLnkCtrl;
	pthread_t remoteUpdateThxd;
	
	NETCOMM_DEBUG_STR("Enter remoteUpdate_Request !", 0);
	
	if(!pReq)
	{
		NETCOMM_DEBUG_STR("serious error, request null!", -1);
		exit(1);
	}
	
	pLnkCtrl = &remote_updater;
	
	if(byUpdateFlag)
	{
		rtn = CTRL_FAILED_BUSY;
		goto END;
	}
	else
	{
		remoteUpdate_InitLinkNode(pLnkCtrl, sock);
		byUpdateFlag = 1;
	}
	
	pLnkCtrl->remoteupload.size 	= (pReq->Update_t.size);
	pLnkCtrl->remoteupload.reserved	= (pReq->Update_t.updateType);
	
	if(pLnkCtrl->remoteupload.reserved==0)
	{
		if(0!=remoteUpdate_CheckCurStatus())
		{
			rtn = CTRL_FAILED_CONFLICT;
			goto END;
		}
	}
	
	if(pLnkCtrl->remoteupload.size==0)
	{
		rtn = CTRL_FAILED_PARAM;
		goto END;
	}
	
	send_is_ok = FALSE;
	
	netComm_Ack(0, sock, nAckId);
	printf("remoteUpdate_Request:netComm_Ack:(%d,%d)\n",sock,nAckId);
	
	rtn = pthread_create(&remoteUpdateThxd,
						 NULL,
						 remoteUpdateFxn,
						 NULL);
	if(0!=rtn)
	{
		//pthread_exit(&remoteUpdateThxd);
		rtn = CTRL_FAILED_RESOURCE;//fail to create preview thread
		goto END;
	}
	
	send_is_ok = TRUE;
	
END:
	if(rtn)
	{
		remoteUpdate_ClearLinkNode(pLnkCtrl);
		NETCOMM_DEBUG_STR("remoteUpdate_Request, err ", rtn);
	}
	
	return rtn;
}

static int 			nUpdateSock = -1;
static int 			nFormatSock = -1;
static u8			oldpos = 0;
static ifly_TCP_Pos backCode;

#if 1
int loopsendprog(SOCKET s, char * buf, unsigned int sndsize)
{
	int remian = sndsize;
	int sendlen = 0;
	int ret = 0;
	while(remian > 0)
	{
		ret = send(s,buf+sendlen,remian,0);
		if(ret <= 0)
		{
			printf("loopsendprog failed,ret=%d,errcode:(%d,%s)\n", ret, errno, strerror(errno));
			return ret;
		}
		sendlen += ret;
		remian -= ret;
	}
	return sndsize;
}

// 目前 格式化和升级进度通知都用此接口
void remoteUpdate_ProgNotify(u8 pos)
{
	u16 err = 0;
	int sock = -1;
	if(nUpdateSock>=0)
	{
		sock = nUpdateSock;
		err = 0xffff;
	}
	else if(nFormatSock>=0)
	{
		sock = nFormatSock;
		err = 0;
	}
	else
	{
		return;
	}
	
	backCode.errcode 	= htons(err);
	backCode.pos 		= pos;
	
	if(backCode.pos != oldpos)
	{
		loopsendprog(sock, (char*)&backCode, sizeof(backCode));
		
		oldpos = pos;
		
		printf("sock %d remote update notfiy progress %d\n", sock, oldpos);
	}
	//send(nUpdateSock,&backCode,sizeof(backCode),0);
}
#else
void remoteUpdate_ProgNotify(u8 pos)
{
	backCode.errcode 	= htons(0xffff);
	backCode.pos 		= pos;
}
#endif

static void remoteFormat_UpdateState(u8 nIdx, u8 bErr)
{
	SNetStateNotify sNotify;
	SNetCommCtrl* 	pNCCIns = &sNetCommCtrl;
	
	memset(&sNotify, 0, sizeof(sNotify));
	
	sNotify.eAct 		= EM_NET_STATE_FORMAT_START;
	
	sNotify.sRslt.state = nIdx;
	sNotify.sRslt.nErr 	= bErr;
	pNCCIns->sCommonCfg.pNotify((void*)&sNotify);
}

static void remoteUpdate_UpdateState(u32 nType, u8 bErr)
{
	SNetStateNotify sNotify;
	SNetCommCtrl* 	pNCCIns = &sNetCommCtrl;
	
	memset(&sNotify, 0, sizeof(sNotify));
	switch(nType)
	{
		case UPDATE_TYPE_MAINBOARD:
		{
			sNotify.eAct 		= EM_NET_STATE_UPDATEMAINBOARDSTART;
		}
		break;
		case UPDATE_TYPE_PANEL:
		{
			sNotify.eAct 		= EM_NET_STATE_UPDATEPANNELSTART;
		}
		break;
		case UPDATE_TYPE_STARTLOGO:
		{
			sNotify.eAct 		= EM_NET_STATE_UPDATESTARTLOGOSTART;
		}
		break;
		case UPDATE_TYPE_APPLOGO:
		{
			sNotify.eAct 		= EM_NET_STATE_UPDATEAPPLOGOSTART;
		}
		break;
		default:
			return;
	}
	
	sNotify.sRslt.state = 0;
	sNotify.sRslt.nErr 	= bErr;
	pNCCIns->sCommonCfg.pNotify((void*)&sNotify);
}

#define UPDATEPRT(x)	//printf("[remoteupdate %d %d]\n", __LINE__, x)

static SRemoteOpCB sOpIns;

void *remoteUpdateFxn(void *arg)
{
	while(FALSE == send_is_ok)
	{
		usleep(50*1000);
	}
	
	SNetCommCtrl* pNCCIns = &sNetCommCtrl;
	ifly_remote_updater_t *pUpdater = &remote_updater;
	ifly_remote_upload_t *ru = &pUpdater->remoteupload;
	
	printf("软件升级开始......pUpdater->serversock=%d\n",pUpdater->serversock);
	
	oldpos = 0;
	//printf("%s 1, reserved: %d\n", __func__, ru->reserved);
	remoteUpdate_UpdateState(ru->reserved, 0);
	//printf("%s 2\n", __func__);
	nUpdateSock = pUpdater->serversock;// for progress notify
	
	int  status = 0;
	char buf[UPDATE_BLOCK];
	int  remain = ru->size;
	int recv_count = 0;
	int recv_count2 = 0;
	FILE *fp = NULL;
	//char *pbuf = NULL;
	char filepath[FILE_PATH_MAX];
	char filedir[FILE_DIR_MAX];
	int  i;
	int  ret = 0;
	
	//考虑到RAMDISK可能空间不足
	//在内存中开辟地址空间，若成功，将文件放入临时目录中
	//若不成功，则将文件放入硬盘地址空间中
	//pbuf = (char*)malloc(REMOTE_UPDATE_BUFF_LEN);	
	//if( pbuf == NULL)
	if(0)
	{
		printf("mem allocated failed\n");
		
		for(i=0;i<MAX_HDD_NUM;++i)
		{
			if(pDiskMgr->hinfo[i].is_disk_exist && pDiskMgr->hinfo[i].is_partition_exist[0])
			{
				sprintf(filedir,"rec/%c%d/",'a'+i,1);
				break;
			}
		}
		
		if(i>=MAX_HDD_NUM)
		{
			ret = CTRL_FAILED_DEVICEBUSY;			
			goto UPDATE_END;
		}
	}
	else
	{
		//free(pbuf);
		#if 1 //def VGA_RESOL_FIX
		sprintf(filedir,"/tmp2/");
		#else
		sprintf(filedir,"/tmp/");
		#endif
	}
	//printf("%s 3\n", __func__);
	switch(ru->reserved)
	{
		case UPDATE_TYPE_PANEL:
			sprintf(filepath, "%spanel.bin", filedir);
			break;
		case UPDATE_TYPE_MAINBOARD:
			sprintf(filepath, "%smainboard.bin", filedir);
			break;
		case UPDATE_TYPE_STARTLOGO:
			sprintf(filepath, "%sstartlogo.jpg", filedir);
			break;
		case UPDATE_TYPE_APPLOGO:
			sprintf(filepath, "%sapplogo.jp", filedir);
			break;
		default:
			ret = CTRL_FAILED_PARAM;			
			goto UPDATE_END;
			break;
	}
	
	UPDATEPRT(ru->reserved);
	
	//printf("filepath=%s\n", filepath);
	/**********************/
	//
	fp = fopen(filepath, "wb");
	if(fp == NULL)
	{
		printf("file null\n");
		
		ret = CTRL_FAILED_OUTOFMEMORY;
		
		goto UPDATE_END;
	}
	
	UPDATEPRT(remain);
	
	while(remain > 0)
	{
		int rcvlen = min(remain,sizeof(buf));
		
		ret = recv(pUpdater->serversock,buf,rcvlen,MSG_NOSIGNAL);
		if(ret < 0)
		{
			printf("recv failed,sock=%d,ret=%d,errcode=%d,errstr=%s,recv:%d,%d,remain=%d\n",pUpdater->serversock,ret,errno,strerror(errno),recv_count,recv_count2,remain);
			status = -1;
			break;
		}
		
		if(ret == 0)
		{
			printf("recv bytes:0!!!\n");
			status = -1;
			break;
		}
		
		if(fp != NULL)
			fwrite(buf,ret,1,fp);
		
		remain -= ret;
		recv_count += ret;
		recv_count2++;
	}
	
	fclose(fp);
	
	ret = 0;
	
	if(status == -1)
	{
		printf("update failed because recv failed\n");
		ret = CTRL_FAILED_NETRCV;
		goto UPDATE_END;
	}
	
	EMNETUPGRADETARGET	 	eTgt;
	EMNETUPGRADEWAY	 		eWay = EM_NETUPGRADE_NET;
	switch(ru->reserved)
	{
		case UPDATE_TYPE_MAINBOARD:
			eTgt = EM_NETUPGRADE_MAINBOARD;
		break;
		case UPDATE_TYPE_PANEL:
			eTgt = EM_NETUPGRADE_PANEL;
		break;
		case UPDATE_TYPE_STARTLOGO:
			eTgt = EM_NETUPGRADE_STARTLOGO;
		break;
		case UPDATE_TYPE_APPLOGO:
			eTgt = EM_NETUPGRADE_APPLOGO;
		break;
		default:
		{
			printf("invalid update type!\n");
			ret = CTRL_FAILED_NETRCV;
			goto UPDATE_END;
		}
		break;
	}
	
	ret = 0;
	///*
	
	UPDATEPRT(eTgt);
	
	sleep(1);
	
	ret = -1;
	if(pNCCIns->pRUpdate != NULL)
	{
		memset(&sOpIns, 0, sizeof(sOpIns));
		sOpIns.eOp					= EM_NETUPDATE;
		sOpIns.Update.eTgt			= eTgt;
		sOpIns.Update.eWay			= eWay;
		strcpy(sOpIns.Update.sfilepath, filepath);
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@软件升级开始@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
		ret = pNCCIns->pRUpdate(&sOpIns);
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@软件升级结束:%d@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n",ret);
	}
	
	if(ret != 0)
	{
		printf("upgrade type %d failed!\n", ru->reserved);
		ret = CTRL_FAILED_UNKNOWN;
	}
	
UPDATE_END:	
	UPDATEPRT(ret);
	
	//printf("软件升级结束\n");
	
	remoteUpdate_ErrAck(pUpdater, ret);
	
	UPDATEPRT(ret);
	sleep(1);
	
	if(ret == 0)
	{
		printf("ready to reboot...\n");
		
		// wait for 1 second
		// and then restart
		//sleep(1);
		rebootSlaveByCmd();//9616需要同时重启从片
		//system(reboot); //9624调用下行代码无效
		reboot(RB_AUTOBOOT);
	}
	else
	{
		printf("update file %s ru->reserved %d\n", filepath, ru->reserved);
		remove(filepath);
		remoteUpdate_ClearLinkNode(pUpdater);
		remoteUpdate_UpdateState(ru->reserved, 1);
	}
	
	NETCOMM_DEBUG_STR("remote update over!!", 0);
	
	return NULL;
}

// remote format i/f
s32 remoteFormat_Request(ifly_TCP_Stream_Req* pReq, SOCKHANDLE sock, u32 nAckId)
{
	s32 rtn = 0;
	SRemoteFormatCtrl 	*pLnkCtrl = &sRFormatCtrl;
	pthread_t 			remoteFormatThxd;

	NETCOMM_DEBUG_STR("Enter remoteFomat_Request !", 0);

	if(!pReq)
	{
		NETCOMM_DEBUG_STR("serious error, request null!", -1);
		
		exit(1);
	}
	
	if (bFormatFlag)
	{
		rtn = CTRL_FAILED_BUSY;
		goto END;
	}
	else
	{
		remoteFormat_InitLinkNode(pLnkCtrl, sock);
		bFormatFlag = 1;
	}

	pLnkCtrl->idx 	= pReq->formatHddIndex;
	
	send_is_ok = FALSE;
	
	netComm_Ack(0, sock, nAckId);
	
	rtn = pthread_create(&remoteFormatThxd,
						 NULL,
						 remoteFormatFxn,
						 NULL );
	if(0!=rtn)
	{
		//pthread_exit(&remoteFormatThxd);
		
		rtn = CTRL_FAILED_RESOURCE;//fail to create preview thread
		
		goto END;
	}
	
	send_is_ok = TRUE;
	
END:
	if(rtn)
	{	
		remoteFormat_ClearLinkNode(pLnkCtrl);
		NETCOMM_DEBUG_STR("remoteFormat_Request, err ", rtn);
	}
	
	return rtn;
}

s32 remoteFormat_ErrAck(SRemoteFormatCtrl *pCtrl, s32 err)
{
	ifly_TCP_Pos rtnCode;
	memset(&rtnCode, 0, sizeof(rtnCode));
	
	if(err)
	{
		rtnCode.errcode = htons((u16)err);//err;//csp modify
	}
	else
	{
		rtnCode.errcode = htons(0);//0;//csp modify
		rtnCode.pos = 100;
	}
	
	printf("errcode %d pos %d \n", rtnCode.errcode, rtnCode.pos);
	send(pCtrl->sock, &rtnCode, sizeof(rtnCode), MSG_NOSIGNAL);
	
	return 0;
}

static s32 remoteFormat_InitLinkNode(
		SRemoteFormatCtrl*		pLnkCtrl,
		SOCKHANDLE				sock
)
{
	s32 rtn = 0;
	struct timeval tv;
	
	NETCOMM_ASSERT(pLnkCtrl);

	tv.tv_sec = 10;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));
	pLnkCtrl->sock = sock;
	
	return rtn;
}

static s32 remoteFormat_ClearLinkNode( SRemoteFormatCtrl* pLnkCtrl  )
{
	s32 rtn = 0;
	
	if(!pLnkCtrl)
	{
		rtn = -1;
		goto CLR_ERR;
	}

	if(pLnkCtrl->sock!=INVALID_SOCKET)
	{
		close(pLnkCtrl->sock);
		pLnkCtrl->sock = INVALID_SOCKET;
	}

	bFormatFlag = 0;
	
CLR_ERR:
	if(rtn)
	{
		NETCOMM_DEBUG_STR("clear lnk node err!!!", rtn);
	}
	
	NETCOMM_DEBUG_STR("Exit remoteFormat_ClearLinkNode!!!", rtn);
			
	return rtn;
}

void *remoteFormatFxn(void *arg)
{
	int ret = 0;
	
	while(FALSE == send_is_ok)	
	{
		usleep(50*1000);
	}
	
	SRemoteFormatCtrl *pCtrl = &sRFormatCtrl;
	
	oldpos = 0;	
	nFormatSock = pCtrl->sock;
	
	printf("Start remote foramt sock %d, disk id =%d\n", pCtrl->sock, pCtrl->idx);

	//
	ret = -1;
	SNetCommCtrl* 	pNCCIns = &sNetCommCtrl;

	pStopRecCB();
	remoteFormat_UpdateState(pCtrl->idx, 0);
	usleep(500*1000);
	remoteUpdate_ProgNotify(1); // 启动远程格式化即发送1%进度，避免远程错以为设备无响应
		
	if(pNCCIns->pRFormat != NULL)
	{
		memset(&sOpIns, 0, sizeof(sOpIns));		
		sOpIns.eOp 					= EM_NETFORMAT;
		sOpIns.Format.nIdx			= pCtrl->idx;
		ret = pNCCIns->pRFormat(&sOpIns);
	}
	
	sleep(1);
	
	remoteFormat_ClearLinkNode(pCtrl);
	remoteFormat_ErrAck(pCtrl, ret);

	remoteFormat_UpdateState(pCtrl->idx, 1);
	
	NETCOMM_DEBUG_STR("remote format over!!", 0);
	
	return NULL;
}

