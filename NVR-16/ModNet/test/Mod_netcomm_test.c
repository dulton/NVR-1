#include "netcomm.h"
#include <string.h>
#include "custommp4.h"
//#include "mediastream.h"
//#include "mediacommon.h"
//#include "record.h"
#include "ctrlprotocol.h"
#include "testnetcomm.h"
#include "partitionindex.h"
#include "Mod_syscomplex.h"
#if 1
#include "diskmanage.h"
#include <stdio.h>
#include "mail.h"
#include <sys/time.h>


#define PAL 12
#define NTSC 10

#define AUDIO_SAMPLE_BITWIDTH		16

#define CHN_MAX			16
#define MAX_FRAME_SIZE				(s32)256*1024//zlb20100802

#define CUR_STARDMODE	PAL

typedef struct _sFrame
{
	// header
	SNetComStmHead header;
	
	// buffer
	u8* pbuff;
} SFrame, *PSframe;

typedef struct _sFrameCtrl
{
	BOOL bSend;
	u8 type; // 0 vid, 1 aud
	BOOL bKeyFrame;
} SFrameCtrl, *PSFrameCtrl;

static SFrameCtrl sFrameCtrlIns[CHN_MAX];

#define TEST_REC_FILE 	"rec/a1/fly00"



static SFrame sFrameIns;

disk_manager hddmanager;

static custommp4_t * fh[CHN_MAX] = {NULL};

void  DoCTRL_CMD_GETSYSTIME(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
);
void  DoCTRL_CMD_SETSYSTIME(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
);

int GetVidWH( int resolution, int* w, int* h);


u32 getTimeStamp()
{
	struct timeval tv;
	
	gettimeofday(&tv, NULL);
	//return tv.tv_sec*HZ + (tv.tv_usec*HZ)/1000000;
	return tv.tv_sec*1000 + (tv.tv_usec)/1000;
	
	//return TickGet();
}

static s32 ncTest_ReadFrame( 
	u8	i,
	u8 eFrmType, 
	PSframe pFrame	
);


int debug_write(char* buf, int size)
{
	static int cnr =0;
	static FILE* fh = NULL;
	//if(cnr>1000) return 0;
	if(!fh)
	{
		fh = fopen("/root/rec/debug.ifv","wb");
	}
	//else
	if(fh)
	{
	#if 0
		if(cnr++>1000)
		{
			fclose(fh);
			fh = NULL;
		}
		else
	#endif
		{
			fwrite(buf, 1, size, fh);
		}
	}
}

// 	sendmail("chinadvr2009", "chinadvr2009@sina.com", "lbzhu", "lbzhu@tl-tek.com"
// 		, "chinadvr2009", "9002dvr?_-", "smtp.sina.com", "this is a mail test", "wonderful mail"
// 		, "1.jpg", "base64");

#if 1
extern 
BOOL sendmail(char *s_name_from, char *s_mail_from, 
	char *s_name_to, char *s_mail_to, char *s_user, char *s_password, 
	char *s_mailserver, char *s_subject, 
	char *s_body, char *s_attached, char *s_encode_type);

static s32 ncTest_SendMail()
{
	#if 0
 	int ret =sendmail("tltest1", "tltest1@163.com", "spliang", "spliang@tl-tek.com"
 		, "tltest1", "tongli", "123.58.178.203", "this is a mail test", "wonderful mail"
 		, NULL, "base64");	
 	#else
 	
 	int ret =sendmail("tltest1", "tltest1@163.com", "spliang", "spliang@tl-tek.com"
 		, "tltest1", "tongli", "smtp.163.com", "this is a mail test", "wonderful mail"
 		, NULL, "base64");	
 	#endif
 		
	if(!ret) printf("send mail failed , errcode %d\n", ret);
}
#endif

void* ncTest_SendFrameThread(void* param)
{
	u32 i;
	SNetComStmHead header;
	PSFrameCtrl pFCtrl;

	PSframe	 pFrame = &sFrameIns;
	
	// read rec file & send to client
	while(1)
	{
		for(i=0;i<CHN_MAX;i++)
		{
			pFCtrl = &sFrameCtrlIns[i];
			if(pFCtrl->bSend)
			{
				if(0!=ncTest_ReadFrame(0, pFCtrl->type,pFrame))
				{
					printf("read frame error!!\n");

					getchar();

					exit(1);
				}
				usleep(1000*80);
				break;
			}
		}

		if(i!=CHN_MAX)
		{
			for(i=0;i<CHN_MAX;i++)
			{
				pFCtrl = &sFrameCtrlIns[i];
				if(pFCtrl->bSend)
				{
					SNetComStmHead* pHead = &pFrame->header;
					
					//printf("get frame and send frame...\n");
					pHead->byChnIndex = i;	

					//printf("width %d\n", pHead->nWidth);
					//printf("pHead->byChnIndex %d\n", pHead->byChnIndex);
					
					//getchar();
					
					NetCommSendPreviewFrame(&pFrame->header, pFrame->pbuff);
				}
				//printf("test read frame thread!!!\n");
			}
			//getchar();
		}
	}
}

extern u32 getTimeStamp();

s32 ncTest_ReadFrame( 
	u8	i,
	u8 eFrmType, 
	PSframe pFrame	
)
{
	u16 j;
	u32 realsize;
	char fn[64]={0};
#if 1
	if(!fh[i])
	{
		for(j=0;j<200;j++)
		{
			sprintf(fn,"%s%03d.ifv",TEST_REC_FILE,j);
			fh[i]=custommp4_open(fn, O_R, 0);
			if(fh[i])
			{
				printf("file %s opened!!!\n", fn);
				break;
			}
		}
		if(j==200)
		{
			printf("no file valid!!!\n");
			getchar();
			exit(1);
		}
	}
#endif

	u32  start_time=0;
	u8 key=0, media_type=0;
	u64 pts=0;

	realsize = custommp4_read_one_media_frame(
		fh[i], pFrame->pbuff, MAX_FRAME_SIZE, 
		&start_time, &key, &pts,&media_type);

	if(realsize <= 0){
		
		printf("read file err!!!\n");
		
		custommp4_close(fh[i]);

		getchar();
		
		exit(1);
	}

	
	pFrame->header.bSub = 0;
	pFrame->header.byMediaType = MEDIA_TYPE_H264;

	pFrame->header.byFrameType = (key?3:0);//key?1:0;
	pFrame->header.byChnIndex = i;
	pFrame->header.resolution = VIDEO_RESOLUTION_CIF;
	pFrame->header.dwlen = realsize;
	pFrame->header.timeStamp = getTimeStamp();
/*
GetVidWH(
		pFrame->header.resolution,
		&pFrame->header.nWidth,
		&pFrame->header.nHeight
	);
*/
	pFrame->header.nWidth = 352;
	pFrame->header.nHeight = 288;
	pFrame->header.nFrameRate = 25;
	pFrame->header.eFrameType = EM_AUDIO; 
	if(!media_type)
		pFrame->header.eFrameType = EM_VIDEO;
	else
		pFrame->header.nAudioMode = MEDIA_TYPE_PCMU;
	
	return 0;
}

void req_stream(STcpStreamReq* param)
{
	u8 type;
	u8 ch;
	SFrameCtrl * pCtrl;

	pCtrl = sFrameCtrlIns;
	
	if(param==NULL)
	{
		printf("null param!!\n");

		getchar();

		exit(1);
	}

	ch = param->nChn;
	if(param->nType!=EM_STOP)
	{
		pCtrl[ch].bSend = TRUE;
	}
	else
	{
		printf("stop %d chn preview !\n", ch);
		pCtrl[ch].bSend = FALSE;
	}	
#if 0
	switch(param->command)
	{	
		case 0:
		{
			u8 ch = param->Monitor_t.chn;
			
			pCtrl[ch].bSend = TRUE;
		}
		break;
		default:
			printf("not supported req\n");
			break;
	}
#endif

	return;
}

static void DealStopPreviewLogin(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
);

static void netcommtest_remoteplay(
	void* pFileHandle, 
	PSRmtPlayFileOp pParam
);

static void  DoCTRL_CMD_RECFILESEARCH(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
);

void  DoCTRL_CMD_RECFILESEARCH1(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
);


void cmd_proc_cb(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
);

extern int spla();

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

int main(void)
{
	int i;
	s32 rtn;
	SNetCommCfg sNCCfg;
	pthread_t 		frmThxd;
	
	SModSysCmplxInit sMSCInit;

	//spla();
	ncTest_SendMail();
	
	printf("send mail test finish!\n");
	
	getchar();
	
	return 0;
	
#if DEBUG_TEMP
	// debug lanston
	cmdPro_Init( DoCTRL_CMD_RECFILESEARCH1, 4);
#endif
	//init_disk_manager(disk_manager * hdd_manager);
	
	init_disk_manager(&hddmanager);

	memset(&sMSCInit, 0, sizeof(sMSCInit));
	sMSCInit.pDMHdr = &hddmanager;
	sMSCInit.pSysNotifyCB = NULL;

	// set time	
	SDateTime sDtIns;
			
	// module init
	rtn = ModSysComplexInit(&sMSCInit);

#if 0
	// set time
	memset( &sDtIns, 0, sizeof(sDtIns) );
	
	sDtIns.nYear 	= 2011;
	sDtIns.nMonth 	= 6;
	sDtIns.nDay 	= 11;
	sDtIns.nHour	= 15;
	sDtIns.nMinute 	= 42;	
	sDtIns.nSecode	= 0;
	
	printf("cmd to set datetime \n");
	
	rtn = ModSysComplexDTSet(&sDtIns);	
	if(rtn)
	{
		printf("ModSysComplexDTSet failed!\n");
		getchar();
	}
#endif

	// clear frame ctrl block
	memset(&sFrameCtrlIns, 0, sizeof(sFrameCtrlIns)*CHN_MAX);
	// allocate frame buff
	memset(&sFrameIns,0,sizeof(sFrameIns));
	sFrameIns.pbuff = (u8*)malloc(MAX_FRAME_SIZE);
	if(!sFrameIns.pbuff)
	{
		printf("no free mem!\n");

		getchar();

		exit(1);
	}
	
	// start stream get thread
	rtn = pthread_create(&frmThxd,
							 NULL,
							 ncTest_SendFrameThread,
							 NULL );
	if(0 != rtn)
	{
		//pthread_exit(&frmThxd);
		
		printf("create thread failed!!!!\n");
		
		getchar();
		
		exit(1);
	}

	// register preview cmd callback
	for(i=0; i<128; i++)
		NetCommRegCmdCB( i+CTRL_CMD_LOGIN, cmd_proc_cb );

	NetCommUnregCmdCB(CTRL_CMD_RECFILESEARCH);
	NetCommRegCmdCB(CTRL_CMD_RECFILESEARCH, DoCTRL_CMD_RECFILESEARCH1);
	NetCommUnregCmdCB(CTRL_CMD_GETSYSTIME);
	NetCommRegCmdCB(CTRL_CMD_GETSYSTIME, DoCTRL_CMD_GETSYSTIME);
	NetCommUnregCmdCB(CTRL_CMD_SETSYSTIME);
	NetCommRegCmdCB(CTRL_CMD_SETSYSTIME, DoCTRL_CMD_SETSYSTIME);
	
	sNCCfg.sParaIns.TCPPort = 8630;
	sNCCfg.nAllLinkMax = STREAM_LINK_MAXNUM;			// 码流数上限
	sNCCfg.sParaIns.TCPMaxConn = 4;
	sNCCfg.nSubStreamMax = 4;			// 子码流数上限
	sNCCfg.nSendBufNumPerChn = 3;		// 每通道发送buff数
	sNCCfg.nFrameSizeMax=MAX_FRAME_SIZE;			// 帧buff大小上限
	sNCCfg.yFactor = PAL;				// 制式


	sNCCfg.nVideoMediaType = MEDIA_TYPE_H264;
	sNCCfg.nAudioMediaType = AUDIO_SAMPLE_BITWIDTH;
	sNCCfg.nVideoFrameRate = 25;
	sNCCfg.nAudioFrameRate = 33;
	sNCCfg.nAudioMode = MEDIA_TYPE_PCMU;
	sNCCfg.pDiskMgr = &hddmanager;
	sNCCfg.pReqKeyCB = NULL;
	sNCCfg.pReqStreamCB = req_stream;
	sNCCfg.pRpCB = netcommtest_remoteplay;
	sNCCfg.pVOIPIn = NULL;
	sNCCfg.pVOIPOut = NULL;
	
	rtn = NetCommInit(&sNCCfg);
	if(rtn)
	{
		printf("netcomminit failed!!!\n");
		getchar();
	}

	printf("start test net comm module!!!!\n");

	getchar();
	
	return 0;
}


int getYFactor()
{
	return CUR_STARDMODE;
}

int GetVidWH( int resolution, int* w, int* h)
{
	int tmp_w, tmp_h;
	
	if(VIDEO_RESOLUTION_CIF==resolution)
	{
		tmp_w = 352;
		tmp_h = ((NTSC == getYFactor())?240:288);
	}
	else if(VIDEO_RESOLUTION_HALF_D1==resolution)
	{
		tmp_w = 704;
		tmp_h = ((NTSC == getYFactor())?240:288);
	}
	else if(VIDEO_RESOLUTION_D1==resolution)
	{
		tmp_w = 704;
		tmp_h = ((NTSC == getYFactor())?480:576);
	}
	else if(VIDEO_RESOLUTION_QCIF==resolution)
	{
		tmp_w = 176;
		tmp_h = ((NTSC == getYFactor())?120:144);
	}
	else
	{
		tmp_w = 352;
		tmp_h = ((NTSC == getYFactor())?240:288);
	}

	*(int*)w = 352;
	*(int*)h = tmp_h;

	printf("tmp_w tmp_h %d %d resolution %d width %d\n", tmp_w, tmp_h, resolution, *(int*)w);

	getchar();

	return 0;
}

// 
// #define CTRL_CMD_STOPVIDEOMONITOR		CTRL_CMD_BASE+8			//停止视频预览

#if 0
void DealStopPreviewLogin(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	printf("DealStopPreviewLogin ..., deal \n" );
	
	int i = 0;
	ifly_loginpara_t login;
	
	if(NULL!=pCmd&&NULL!=pRslt)
	{
		ifly_link_online links;
		ifly_remusr_limit quanxian;
	
		memset(&links,0,sizeof(links));
		memcpy(&login,&pCmd->sReq,sizeof(ifly_loginpara_t));
		
		u32 dwIp = login.ipAddr;
		printf("name:%s,ip:%d.%d.%d.%d\n",login.username,(u8)dwIp,(u8)(dwIp>>8),(u8)(dwIp>>16),(u8)(dwIp>>24));

		
		links.link_online = 1;
		memcpy(&pRslt->sBasicInfo,&links,sizeof(links));
		pRslt->nBasicInfoLen = sizeof(links);

		strcpy( quanxian.usrname, "Admin" );

		#define REMOTE_PRIVILIGE_NUM 9
		
		for(i=0; i<REMOTE_PRIVILIGE_NUM; i++)
			quanxian.remote_privilege[i] = '1';
			
		pRslt->nExtInfoLen = links.link_online*sizeof(quanxian);	
		memcpy(&pRslt->sExtInfo,&quanxian,pRslt->nExtInfoLen);

		pRslt->nErrCode = CTRL_SUCCESS;
	}
	
	return;
}
#endif
extern BOOL is_i_frame(u8 video_type);

#if 0
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
#endif


void netcommtest_remoteplay(
	void* pFileHandle,
	PSRmtPlayFileOp pParam
)
{
	printf("Enter netcommtest_remoteplay ... \n");
 	void* pTmpFileHandle = NULL;
	
	if(!pParam)
	{
		printf("NULL param!!!\n");

		return;
	}
	EM_RMTPLAY_OP eOp = pParam->eOp;
	EM_FILEOP_SRC eOpSrc = pParam->eOpSrc;

	switch(eOp)
	{
		case EM_RMTPLAY_OPEN:
		{
			if(eOpSrc==EM_FILEOP_REPLAY)
			{
				pTmpFileHandle = (void*)custommp4_open(
					pParam->OpenSeek.szFilePath, 
					O_R, 
					pParam->OpenSeek.nOffset
				);
			}
			else if(eOpSrc==EM_FILEOP_DOWN)
			{				
				pTmpFileHandle = fopen(
					pParam->OpenSeek.szFilePath,"rb");
			}
			
			pParam->OpenSeek.pFile = (void*)pTmpFileHandle;
		}
		break;
		case EM_RMTPLAY_SEEK:
		{
			
			if(pFileHandle)
			{
				if(eOpSrc==EM_FILEOP_REPLAY)
				{
					pParam->nOpRslt = 
						custommp4_seek_to_time_stamp((custommp4_t*)pFileHandle, pParam->OpenSeek.nOffset);
				}
				else if(eOpSrc==EM_FILEOP_DOWN)
				{				
					pParam->nOpRslt = fseek((FILE*)pFileHandle,pParam->OpenSeek.nOffset,SEEK_SET);
				}
			}
		}
		break;
		case EM_RMTPLAY_CLOSE:
		{
			if(pFileHandle)
			{
				if(eOpSrc==EM_FILEOP_REPLAY)
				{
					custommp4_close((custommp4_t*)pFileHandle);
				}
				else if(eOpSrc==EM_FILEOP_DOWN)
				{				
					fclose((FILE*)pFileHandle);
				}
			}
		}
		break;
		case EM_RMTPLAY_GETFRAME:
		{
			if(pFileHandle)
			{
				if(eOpSrc==EM_FILEOP_REPLAY)
				{
					if(pFileHandle)
						pParam->GetFrame.nFrameSize = custommp4_read_one_media_frame(
									(custommp4_t*)pFileHandle,
									pParam->GetFrame.pFrameBuf,
									pParam->GetFrame.nBufSize,
									&pParam->GetFrame.nStartTime,
									&pParam->GetFrame.bKey,
									&pParam->GetFrame.nPts,
									&pParam->GetFrame.nMediaType
								);

						// debug by lanston
						if(pParam->GetFrame.nFrameSize>0)
						{
							debug_write((char*)pParam->GetFrame.pFrameBuf, pParam->GetFrame.nFrameSize);
						}
						
						printf("frame size %d\n", pParam->GetFrame.nFrameSize);
						printf("nStartTime %d\n", pParam->GetFrame.nStartTime);
						printf("frame bKey %d\n", pParam->GetFrame.bKey);
						printf("frame nPts %d\n", pParam->GetFrame.nPts);
						printf("nMediaType %d\n", pParam->GetFrame.nMediaType);
						
						usleep(40*1000);
								
					pParam->GetFrame.bKey = is_i_frame((u8)pParam->GetFrame.bKey);
				}
				else if(eOpSrc==EM_FILEOP_DOWN)
				{
					pParam->GetFrame.nFrameSize
						= fread(pParam->GetFrame.pFrameBuf,
							1,pParam->GetFrame.nBufSize,(FILE*)pFileHandle);
				}
			}
		}
		break;
		case EM_RMTPLAY_GetTimeLength:
		{
			if(pFileHandle)
				pParam->GetTimeLen.nTotalTime = custommp4_total_time((custommp4_t*)pFileHandle);
		}
		break;
		case EM_RMTPLAY_GetMediaFormat:
		{
			if(pFileHandle)
			{
				pParam->GetFormat.nWidth = custommp4_video_width((custommp4_t*)pFileHandle);
				pParam->GetFormat.nHeight = custommp4_video_height((custommp4_t*)pFileHandle);
			}
		}
		break;
	}
	return;
}


#if 1
void  DoCTRL_CMD_RECFILESEARCH(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
return;

}

void  DoCTRL_CMD_GETSYSTIME(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_sysTime_t *pGetsystemtime;
	SDateTime	sDTIns;
	struct tm tm_time;
	
	pGetsystemtime = &pRslt->sBasicInfo.sysTime;

	ModSysComplexDTGet(&sDTIns);
	
	tm_time.tm_year = sDTIns.nYear - 1900;
	tm_time.tm_mon = sDTIns.nMonth - 1;
	tm_time.tm_mday = sDTIns.nDay;
	tm_time.tm_hour = sDTIns.nHour;
	tm_time.tm_min = sDTIns.nMinute;
	tm_time.tm_sec = sDTIns.nSecode;

	pGetsystemtime->systemtime = mktime(&tm_time);

	printf("get %04d-%02d-%02d\n", tm_time.tm_year, tm_time.tm_mon, tm_time.tm_mday);
			
	return;
}
	
#define CTRL_CMD_GETSYSTIME				CTRL_CMD_BASE+65		//获得系统时间
#define CTRL_CMD_SETSYSTIME				CTRL_CMD_BASE+66		//设置系统时间


void cmd_proc_cb(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	if(pCmd)
	{
		printf("set cmd !!!\n");
	}
	else
	{
		printf("get cmd !!!\n");
	}
}

void  DoCTRL_CMD_SETSYSTIME(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	ifly_sysTime_t *pGetsystemtime;
	SDateTime	sDTIns;
	struct tm *ptm_time;
	
	pGetsystemtime = &pCmd->sReq.sysTime;

	ptm_time = (struct tm *)gmtime((const time_t*)&pGetsystemtime->systemtime);

	sDTIns.nYear = 1900+ptm_time->tm_year;
	sDTIns.nMonth = ptm_time->tm_mon+1;
	sDTIns.nDay = ptm_time->tm_mday;
	sDTIns.nHour = ptm_time->tm_hour;
	sDTIns.nMinute = ptm_time->tm_min;
	sDTIns.nSecode = ptm_time->tm_sec;
	
	ModSysComplexDTSet(&sDTIns);
	
	printf("set %04d-%02d-%02d\n", sDTIns.nYear, sDTIns.nMonth, sDTIns.nDay);
			
	return;
}

void  DoCTRL_CMD_RECFILESEARCH1(
	SRemoteCmdReq* pCmd,
	SRemoteCmdRslt* pRslt
)
{
	int sum = 0, i;
	s32 rtn;
	
	//search_param_t localsearch;
	ifly_recsearch_param_t remotesearch;
	ifly_search_desc_t desc;
	ifly_recfileinfo_t info;

	printf("Enter DoCTRL_CMD_RECFILESEARCH1111 ... \n");
	
#if 1

	struct tm tm_time;
	u8 y,m,d, h,min, type, ch;
	char* pFileBuf = NULL;
	
	SSearchPara sSearchParaIns;
	SSearchResult sSearchResultIns;
	int nMaxFileNum;
	
	memcpy(&remotesearch, &pCmd->sReq.recsearch_param, sizeof(remotesearch));
	
	printf("remotesearch.startID %d\n",remotesearch.startID);
	
	//sSearchParaIns.nEndTime = time(NULL);
	
	//earchParaIns.nStartTime = 1425486931;//1325481210; // 1325481210
	//earchParaIns.nEndTime = 1465486931;
	//earchParaIns.nEndTime = 1483200000;

	sSearchParaIns.nMaskType = remotesearch.channel_mask;
	sSearchParaIns.nMaskChn = remotesearch.type_mask;
	sSearchParaIns.nStartTime = remotesearch.start_time;
	sSearchParaIns.nEndTime = remotesearch.end_time;
	//sSearchParaIns.max_return = remotesearch.max_return;		
	
	printf("mask type %d\n",sSearchParaIns.nMaskType);
	printf("nMaskChn %d\n",sSearchParaIns.nMaskChn);
	printf("nStartTime %d\n",sSearchParaIns.nStartTime);
	printf("nEndTime %d\n",sSearchParaIns.nEndTime);
	printf("max_return %d\n",remotesearch.max_return);	
	
	nMaxFileNum = remotesearch.max_return;

	//pFileBuf= malloc(sizeof(SRecfileInfo)*nMaxFileNum);
 
	sSearchResultIns.psRecfileInfo = (SRecfileInfo*)pRslt->sBasicInfo.sRecFile.info;
	
	if(!sSearchResultIns.psRecfileInfo) printf("not enough memory for found files!\n");
	
	rtn = ModSysComplexDMSearch(EM_FILE_REC, &sSearchParaIns, &sSearchResultIns, nMaxFileNum);

	if(rtn)
	{
		printf("search failed, err %d!\n", rtn);

		getchar();
	}
	else if(sSearchResultIns.nFileNum>0)
	{
		printf("found the %d files as following: \n", sSearchResultIns.nFileNum);

		for( i=0; i<sSearchResultIns.nFileNum; i++ )
			printf("file %d %d %d %d %d\n", 
					sSearchResultIns.psRecfileInfo[i].nChn,
					sSearchResultIns.psRecfileInfo[i].nDiskNo, 
					sSearchResultIns.psRecfileInfo[i].nStartTime,
					sSearchResultIns.psRecfileInfo[i].nEndTime,
					sSearchResultIns.psRecfileInfo[i].nFileNo );		
	}
	else
	{
		return;
	}

	if(remotesearch.startID > sSearchResultIns.nFileNum)
	{
		desc.sum = sSearchResultIns.nFileNum;
		desc.startID = 0;
		desc.endID = 0;  

		return;
	}
	else
	{
		desc.startID = remotesearch.startID;
		desc.sum = sSearchResultIns.nFileNum; 
		
		#if 0
		//pRslt->bUseExt = 1;	
		for(i=remotesearch.startID;i<(sSearchResultIns.nFileNum+1);++i)
		{
			if(i>=remotesearch.startID+remotesearch.max_return) break;
			sum++;
			
			printf("channel_no:%d\n", sSearchResultIns.psRecfileInfo[i-1].nChn);
			info.channel_no = (sSearchResultIns.psRecfileInfo[i-1].nChn - 1);
			info.type = (sSearchResultIns.psRecfileInfo[i-1].nType);
			info.start_time = htonl(sSearchResultIns.psRecfileInfo[i-1].nStartTime);
			info.end_time = htonl(sSearchResultIns.psRecfileInfo[i-1].nEndTime);
			info.image_format = (sSearchResultIns.psRecfileInfo[i-1].nFormat);
			info.stream_flag = (sSearchResultIns.psRecfileInfo[i-1].nStreamFlag);
			info.size =htonl( sSearchResultIns.psRecfileInfo[i-1].nSize);

			SRecfileInfo* pInfo = &sSearchResultIns.psRecfileInfo[i-1];
			 
			get_rec_file_name(&sSearchResultIns.psRecfileInfo[i-1],info.filename,&info.offset);
			
			printf("filename:%s\n", info.filename);
			printf("nOffset %d\n", info.offset);
			
			info.offset = htonl(info.offset);

			
			//memcpy((u8*)&pRslt->sExtInfo+sizeof(desc)+(i-remotesearch.startID)*sizeof(ifly_recfileinfo_t),
			//	&info,sizeof(ifly_recfileinfo_t));
			
			memcpy(pAckBuf+sizeof(desc)+(i-remotesearch.startID)*sizeof(ifly_recfileinfo_t),
					&info,sizeof(ifly_recfileinfo_t));
			
			//*pnAckLen = sizeof(desc);
		}
		desc.endID = htons(remotesearch.startID+sum-1);
		
		memcpy((u8*)pAckBuf,&desc,sizeof(desc));

		*pnAckLen = sizeof(desc)+sum*sizeof(ifly_recfileinfo_t);

		#else
		
		desc.endID = remotesearch.startID+sum-1;

		memcpy(&pRslt->sBasicInfo.sRecFile.desc, &desc, sizeof(desc));
		
		#endif
	}
	
	return;
#endif
	
}
#endif
#endif


