#include "biz.h"
#include "biz_manager.h"
#include "biz_snap.h"
#include "sg_platform.h"


#include <sys/time.h>
#include "biz_syscomplex.h"
#include <sys/syscall.h>

#include <unistd.h>
#include <signal.h>

#define DIRECT_PARA_GETSET
#define PTZMOVESPEED_MAX	10

SBizManager g_sBizManager;
static u8 rec_statue[32] = {0};
static int tmp_rate = 0;
static int play_type = 1;

void BizSetPlayType(int type)
{
	play_type = type;
}

void SetCurPlayRate(int rate)
{
	tmp_rate = rate;
}

int GetCurPlayRate()
{
	if(0 == play_type)
	{
		//csp modify 20130429
		//tmp_rate += 100;
		int rate = tmp_rate+100;
		return rate;
	}
	
	return tmp_rate;
}

#if 0//csp modify
void term_exit(int signo)
{
	time_t cur;
	cur = time(NULL);
	//printf("term_exit:system time:%s\n",  ctime(&cur));	
	printf("!!!!!!recv signal(%d),SIGBUS=%d,SIGPIPE=%d,Biz,%s\n",signo,SIGBUS,SIGPIPE,ctime(&cur));
	if(signo != 17)//子进程结束
	{
		//sleep(10);
		printf("process quit!!!\n");
		exit(-1);
	}
}
#endif

void BizSetNoAuthFuncCB(CHECKNOAUTH func)
{
	SetNoAuthFuncCB(func);
}

u8 CheckAllHDDFormated()
{
	return BizCheckAllHDDFormated();
}

void CheckAllHDD(u8 *bDiskIsExist, u8 *bDiskIsErr, u8 *bDiskIsUnFormat)
{
	return BizCheckAllHDD(bDiskIsExist, bDiskIsErr, bDiskIsUnFormat);
}


s32 BizInit(SBizInitPara* psBizPara)
{
    SBizManager* psBizManager = &g_sBizManager;
    static u8 nInitFlag = 0;
    int ret = -1;//csp modify
	
#if 0//csp modify
	//ignore BROKE_PIPE signal
	signal(SIGPIPE,SIG_IGN);//忽略SIGPIPE信号
	
	//注册信号处理函数
	int i;
	for(i=1;i<32;i++)
	{
		if(i == SIGBUS) continue;
		if(i == SIGPIPE) continue;
		if(signal(i,term_exit) == SIG_ERR)
		{
			printf("Register signal(%d) handler failed\n",i);
		}
	}
#endif
	
    if(0 == nInitFlag)
	{
   		ret = DoBizInit(psBizManager, psBizPara);
		if(BIZ_SUCCESS == ret)
		{
		    //开启业务层外部命令调度线程
		    ret = pthread_create(&psBizManager->thrdDispatch, NULL, BizDispatchFxn, psBizManager);
		    if(ret != 0)
		    {
		        printf("pthread create failed\n");
				nInitFlag = 0;
		        DoBizDeinit(psBizManager);
		        return BIZ_ERR_INIT_DISPATCH_OPEN;
		    }
			
		    //开启业务层内部命令调度线程
		    ret = pthread_create(&psBizManager->thrdDispatch, NULL, BizDispatchInnerFxn, psBizManager);
		    if(ret != 0)
		    {
		        printf("pthread create failed\n");
				nInitFlag = 0;
		        DoBizDeinit(psBizManager);
		        return BIZ_ERR_INIT_DISPATCH_OPEN;
		    }
			
			nInitFlag = 1; //置初始化状态为1
		}
	}
	else if(1 == nInitFlag)
	{
		ret = DoBizInit(psBizManager, psBizPara);
		if (BIZ_SUCCESS == ret)
		{
		    nInitFlag = 2; //置初始化状态为1
		}
	}
	else if(nInitFlag >= 2)
    {
        return BIZ_ERR_INIT_TOOMUCH;
    }
	
    printf("BizInit ret=%d, nInitFlag[%d]\n", ret, nInitFlag);
    
    return ret;
}

s32 BizDeinit(void)
{
    SBizManager* psBizManager = &g_sBizManager;
    
    return DoBizDeinit(psBizManager);
}

//手动开始预览
s32 BizStartPreview(SBizPreviewPara* psBizPreviewPara)
{
#ifndef DIRECT_PARA_GETSET
	SBizMsgHeader sBizMsgHeader;
	SBizManager* psBizManager = &g_sBizManager;
#endif
	
	if(NULL == psBizPreviewPara)
	{
		return BIZ_ERR_PREVIEW_NULLPARA;
	}
	
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_START_PREVIEW;
	sBizMsgHeader.pData = (void *)psBizPreviewPara;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	//printf("yg PreviewStart\n");
	nRet = PreviewStart(psBizPreviewPara);
#endif
	if(0 == nRet)
	{
		//csp modify
		//if(psBizPreviewPara->emBizPreviewMode == EM_BIZPREVIEW_1SPLIT)
		//{
		//	BizSetZoomMode(1);
		//}
		//else
		//{
		//	BizSetZoomMode(0);
		//}
		
		return BIZ_SUCCESS;
	}
	
    return BIZ_ERR_PREVIEW_FAILEDCMD;
}

//csp modify
//校验序列号
s32 BizVerifySN()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	BizGetPara(&bizTar, &bizSysPara);
	
	FILE *fp = fopen("/tmp2/panel.ini","r");
	if(fp == NULL)
	{
		return 0;
	}
	
	fseek(fp,0,SEEK_END);
	long len = ftell(fp);
	fseek(fp,0,SEEK_SET);
	
	if(len == 0)
	{
		fclose(fp);
		return 0;
	}
	
	char line[512];
	memset(line,0,sizeof(line));
	while(fgets(line, sizeof(line), fp))
	{
		char *p = strstr(line,"\r\n");
		if(p)
		{
			*p = '\0';
		}
		else
		{
			p = strstr(line,"\n");
			if(p)
			{
				*p = '\0';
			}
		}
		printf("sn=%s,line=%s\n",bizSysPara.sn,line);
		if(strcmp(line,bizSysPara.sn) == 0)
		{
			printf("BizVerifySN success\n");
			fclose(fp);
			return 0;
		}
	}
	
	fclose(fp);
	return -1;
}

//手动停止预览
s32 BizStopPreview(void)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	SBizMsgHeader sBizMsgHeader;
	SBizManager* psBizManager = &g_sBizManager;
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_STOP_PREVIEW;
	sBizMsgHeader.pData = NULL;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = PreviewStop();
#endif
	if (0 == nRet)
    {
        return BIZ_SUCCESS;
    }
	
    return BIZ_ERR_PREVIEW_FAILEDCMD;
}

//单通道预览
s32 BizChnPreview(u8 nChn)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
    SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_CHN_PREVIEW;
	sBizMsgHeader.pData = (void *)(u32)nChn;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = PreviewZoomChn(nChn);
#endif
	if (0 == nRet)
    {
    	//csp modify
    	//BizSetZoomMode(1);
		
        return BIZ_SUCCESS;
    }

    return BIZ_ERR_PREVIEW_FAILEDCMD;
}

s32 BizElecZoomPreview(int flag, u8 nChn, SBizPreviewElecZoom* stCapRect)
{
	int nRet = PreviewElecZoomChn(flag, nChn,stCapRect);
	if (0 == nRet)
    {
        return BIZ_SUCCESS;
    }
	
    return BIZ_ERR_PREVIEW_FAILEDCMD;
}
//图片预览
s32 BizPreviewSnap(int flag, SBizPreviewElecZoom* stMenuRect)
{
	int nRet = PreviewSnap(flag, stMenuRect);
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
    return BIZ_ERR_PREVIEW_FAILEDCMD;
}

s32 BizPbElecZoomPreview(int flag, u8 nChn, SBizPreviewElecZoom* stCapRect)
{
	int nRet = PreviewPbElecZoomChn(flag, nChn,stCapRect);
	if (0 == nRet)
    {
        return BIZ_SUCCESS;
    }
	
    return BIZ_ERR_PREVIEW_FAILEDCMD;
}

s32 BizPlayBackRegistFunCB(u8 type, BIZPBREGFUNCCB func)
{
	return PlayBackRegistFunCB(type, func);
}

//单通道预览或者预览停止后恢复多画面
s32 BizResumePreview(u8 nForce)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_RESUME_PREVIEW;
	sBizMsgHeader.pData = NULL;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = PreviewResume(nForce);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
    return BIZ_ERR_PREVIEW_FAILEDCMD;
}

//预览下一页/手动轮巡
s32 BizNextPreview(void)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;
	
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_NEXT_PREVIEW;
	sBizMsgHeader.pData = NULL;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	//printf("BizNextPreview - PreviewNext\n");
	nRet = PreviewNext();
#endif
	if(0 == nRet)
    {
        return BIZ_SUCCESS;
    }
	
    return BIZ_ERR_PREVIEW_FAILEDCMD;
}

//预览上一页
s32 BizLastPreview(void)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType= EM_BIZMSG_LAST_PREVIEW;
	sBizMsgHeader.pData = NULL;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = PreviewLast();
#endif
	if (0 == nRet)
    {
        return BIZ_SUCCESS;
    }

    return BIZ_ERR_PREVIEW_FAILEDCMD;
}

//预览下一模式
s32 BizNextPreviewMod(void)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType= EM_BIZMSG_NEXT_PREVIEW_MOD;
	sBizMsgHeader.pData = NULL;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = PreviewNextMod();
#endif
	if (0 == nRet)
    {
        return BIZ_SUCCESS;
    }

    return BIZ_ERR_PREVIEW_FAILEDCMD;
}

s32 BizPreviewZoomChn(u8 chn)
{
	return PreviewZoomChn(chn);
}

//预览上一模式
s32 BizLastPreviewMod(void)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType= EM_BIZMSG_LAST_PREVIEW_MOD;
	sBizMsgHeader.pData = NULL;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = PreviewLastMod();
#endif
	if (0 == nRet)
    {
        return BIZ_SUCCESS;
    }

    return BIZ_ERR_PREVIEW_FAILEDCMD;
}
//手动开启/停止静音
s32 BizPreviewMute(u8 nEnable)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;
	
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_PREVIEW_MUTE;
	sBizMsgHeader.pData = (void *)(u32)nEnable;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = PreviewMute(nEnable);
#endif
	if (0 == nRet)
    {
        return BIZ_SUCCESS;
    }
	
    return BIZ_ERR_PREVIEW_FAILEDCMD;
}

//设置音频预览通道
s32 BizPreviewAudioOutChn(u8 nChn)//0xff表示自动
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_PREVIEW_AUDIOCHN;
	sBizMsgHeader.pData = (void *)(u32)nChn;
	if (0 == WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader))
	{
		return BIZ_SUCCESS;
	}
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = PreviewSetAudioOut(nChn);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
    return BIZ_ERR_PREVIEW_FAILEDCMD;
}

//控制音频预览音量
s32 BizPreviewVolume(u8 nVolume)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_PREVIEW_VOLUME;
	sBizMsgHeader.pData = (void *)(u32)nVolume;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = PreviewVolume(nVolume);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
	return BIZ_ERR_PREVIEW_FAILEDCMD;
}

//直接设置预览图像参数而不保存配置
s32 BizPreviewSetImage(u8 nChn, SBizPreviewImagePara* psImagePara)
{
	return PreviewSetImage(nChn, (SPreviewImagePara*)psImagePara);
}

s32 BizPlayBackSetImage(SBizVoImagePara* psVoImagePara)
{
	return PlayBackSetImage((SVoImagePara*)psVoImagePara);
}

//获取预览图像参数
s32 BizPreviewGetImage(u8 nChn, SBizPreviewImagePara* psImagePara)
{
	return PreviewGetImage(nChn, (SPreviewImagePara*)psImagePara);
}

//手动开启轮巡
s32 BizStartPatrol(void)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_START_PATROL;
	sBizMsgHeader.pData = NULL;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = PreviewStartPatrol();
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}

	return BIZ_ERR_PREVIEW_FAILEDCMD;
}

//手动停止轮巡
s32 BizStopPatrol(void)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_STOP_PATROL;
	sBizMsgHeader.pData = NULL;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = PreviewStopPatrol();
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}


	return BIZ_ERR_PREVIEW_FAILEDCMD;
}

//s32 GetTimeTick()
//{
//	struct timeval time;
//	gettimeofday(&time, NULL);   
//	return time.tv_sec*1000+time.tv_usec/1000;
//}

//用户
s32 BizUserLogin(SBizUserLoginPara* para, u32* nUserId)
{
	return UserLogin(para,nUserId);
}

s32 BizUserLogout(u32 nUserId)
{
	return UserLogout(nUserId);
}

EMBIZUSERCHECKAUTHOR BizUserCheckAuthority(EMBIZUSEROPERATION emOperation,u32 * para,u32 nUserId)
{
	return UserCheckAuthor(emOperation,para,nUserId);
}

s32 BizUserAddUser(SBizUserUserPara* para)
{
	s32 ret = UserAddUser(para);
	if(ret != 0)
	{
		return ret;
	}
	
	return UserUpdatePara();
}

s32 BizGetMaxChnNum(void)
{
	return ConfigGetMaxChnNum();
}

s32 BizGetCurRecStatus(u8 nChn)
{
	return rec_statue[nChn];
}

void BizSetCurRecStatus(u8 nChn, u8 flag)
{
	rec_statue[nChn] = flag;
}

s32 BizUserDeleteUser(s8* pUserName)
{
	s32 ret = UserDeleteUser(pUserName);	
	if(ret != 0)
	{
		return ret;
	}
	
	return UserUpdatePara();
}

//日志
s32 BizWriteLog(SBizLogInfo* psLogInfo)
{
	return WriteLog(psLogInfo);
}

static char g_pUserNamelog[30] = {0};  //cw_log

s32 BizSetUserNameLog(char* pName)
{
	if(!pName)
		return -1;
	memset(g_pUserNamelog, 0, sizeof(g_pUserNamelog));
	strcpy(g_pUserNamelog, pName);
	return 0;
}

char* BizGetUserNameLog()
{
	return g_pUserNamelog;
}

s32 BizWriteLog_2(s8 master, s8 slave)
{
	SBizLogInfo info;
	memset(&info, 0, sizeof(info));
	info.nMasterType = master;
	info.nSlaveType = slave;
	strcpy(info.aIp, "GUI");
	strcpy(info.aUsername, g_pUserNamelog);
	return BizWriteLog(&info);
}

s32 BizSearchLog(SBizLogSearchPara* psLogSearch, SBizLogResult* psLogResult)
{
	return SearchLog(psLogSearch, psLogResult);
}

#if 0//csp modify
u64 GetTimeStamp()//cw_time
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	
	//return tv.tv_sec*1000 + (tv.tv_usec)/1000;
	return (u64)(((((u64)(tv.tv_sec))*1000000) + (u64)tv.tv_usec)/1000);
}
#endif

s32 BizExportLog(SBizLogResult* psLogResult, u8* pPath)
{
	return ExportLog(psLogResult, pPath);
}

//获得及设置参数
//(设置时如果只改变结构体部分参数先get再set)
s32 BizSetPara(SBizParaTarget* psBizParaTarget, void* pData)
{
	//return 0;
	
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;
	SBizPara sBizPara;
	
	if(NULL == psBizParaTarget || NULL == pData)
	{
		return BIZ_ERR_PARASET_NULLPARA;
	}
	
	sBizPara.sBizParaTarget = *psBizParaTarget;
	sBizPara.pData = pData;
	
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_SET_PARA;
	sBizMsgHeader.pData = &sBizPara;
	
	//int start = GetTimeTick();
	
	int ret = 0;
	//printf("yg BizSetPara chn%d\n", sBizPara.sBizParaTarget.nChn);
	ret = DoBizSetPara((SBizPara*)sBizMsgHeader.pData);
	if(0 == ret)
	{
		//int end = GetTimeTick();
		//printf("1111 end - start = %d\n",end-start);
		return BIZ_SUCCESS;
	}
	
	//int end = GetTimeTick();
	//printf("2222 end - start = %d\n",end-start);
	
	/*
	if (0 == WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader))
	{
		return BIZ_SUCCESS;
	}
	*/
	
    return BIZ_ERR_PARASET_FAILEDCMD;
}

s32 BizGetPara(SBizParaTarget* psBizParaTarget, PARAOUT void* pData)
{
	//int start = GetTimeTick();
	
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;
	SBizPara sBizPara;
	
	if(NULL == psBizParaTarget || NULL == pData)
	{
		return BIZ_ERR_PARAGET_NULLPARA;
	}
	
	sBizPara.sBizParaTarget = *psBizParaTarget;
	sBizPara.pData = pData;
	
	/*memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_GET_PARA;
	sBizMsgHeader.pData = &sBizPara;
	if (0 == WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader))
	{	
		//static int count = 0;
		//printf("psBizParaTarget type %d  !!! %d \n",psBizParaTarget->emBizParaType,count++);
		//int end = GetTimeTick();
		//printf("BizGetPara: end-start = %d\n",end-start);
		return BIZ_SUCCESS;
	}*/
	
	if(0 == DoBizGetPara(0,(SBizPara*)&sBizPara))
	{
		//int end = GetTimeTick();
		//printf("BizGetPara: end-start = %d\n",end-start);
		return BIZ_SUCCESS;
	}
	
    return BIZ_ERR_PARAGET_FAILEDCMD;
}

//返回实际制式
int BizGetVideoFormatReal()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	
	if(ret!=0)
	{
		//BIZ_DATA_DBG("GetVideoFormat\n");
        return EM_BIZ_PAL;
	}
	
	return (int)bizSysPara.nVideoStandard;
}

s32 BizGetDefaultPara(SBizParaTarget* psBizParaTarget, PARAOUT void* pData)
{
    SBizManager* psBizManager = &g_sBizManager;
    SBizMsgHeader sBizMsgHeader;
    SBizPara sBizPara;

    if (NULL == psBizParaTarget || NULL == pData)
    {
        return BIZ_ERR_PARAGET_NULLPARA;
    }
    
    sBizPara.sBizParaTarget = *psBizParaTarget;
    sBizPara.pData = pData;

    memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
    sBizMsgHeader.emMsgType = EM_BIZMSG_GET_DEFAULTPARA;
    sBizMsgHeader.pData = &sBizPara;

	if(0 == DoBizGetPara(1,(SBizPara*)sBizMsgHeader.pData))
	{
		return BIZ_SUCCESS;
	}

	/*
    if (0 == WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader))
    {
        return BIZ_SUCCESS;
    }*/

    return BIZ_ERR_PARAGET_FAILEDCMD;
}

void BizSystemRestart()
{
	sysComplex_resetcb();
}

static LEDDTRLFN LEDFUN = NULL;//cw_led

void RegLedCtrlFUN(LEDDTRLFN fun)
{
	if(fun!=NULL)
	{
		LEDFUN=fun;
	}
}

/*
  需要两个参数：类型枚举（锁定/重起/关闭等）
	函数功能:系统退出
	输入参数：
		emSysExitType:
			类型:EMBIZSYSEXITTYPE
			含义:退出类型锁定/重起/关闭系统
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
static pthread_mutex_t SHUT_MUTEX = PTHREAD_MUTEX_INITIALIZER;

s32 BizSysComplexExit(EMBIZSYSEXITTYPE emSysExitType)
{
	s32 nRet = 0;
	
	printf("BizSysComplexExit-1\n");
	fflush(stdout);
	
	pthread_mutex_lock(&SHUT_MUTEX);
	
	printf("BizSysComplexExit-2\n");
	fflush(stdout);
		
	//yzw add
	if((EM_BIZSYSEXIT_RESTART == emSysExitType) || (EM_BIZSYSEXIT_POWEROFF == emSysExitType))
	{
		nRet |= BizRecordStopAll();
		usleep(2*1000*1000);
		PreviewMute(1);//重启后声音会一直存在，包括在uboot和内核阶段
		BizAlarmWorkingEnable(0);//禁用报警模块  yzw
		BizStopPatrol();//停止轮巡
	}
	if(EM_BIZSYSEXIT_POWEROFF == emSysExitType)
	{
		BizWriteLog_2(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_POWER_OFF);//cw_log
		nRet |= ConfigSyncFileToFlash(0);
	}
	if(EM_BIZSYSEXIT_RESTART == emSysExitType)
	{
		BizWriteLog_2(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_REBOOT);//cw_log
		//nRet |= ConfigSyncFileToFlash(0);
	}
	//end
	
	printf("BizSysComplexExit-3\n");
	fflush(stdout);
	
	LEDFUN(0,0);
	LEDFUN(0,1);
	
	printf("BizSysComplexExit-4\n");
	fflush(stdout);
	
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;
	
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_SYSCOMPLEX_EXIT;
	sBizMsgHeader.pData = (void *)(u32)emSysExitType;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet |= SysComplexExit(emSysExitType);
#endif
	if(0 == nRet)
	{
		pthread_mutex_unlock(&SHUT_MUTEX);

		//设置正常退出标志
		SSG_MSG_TYPE msg;
		SBizParaTarget sParaTgtIns;
		SBizSGParam sCfgIns;
		
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_SGPARA;
		
		if (BizGetPara(&sParaTgtIns, &sCfgIns) == 0)
		{
			sCfgIns.normal_exit = 1;
			BizSetPara(&sParaTgtIns, &sCfgIns);
		}

		//上传监管平台，正常退出
		char shutdown[] = {0xE7, 0xB3, 0xBB, 0xE7, 0xBB, 0x9F, \
			0xE5, 0x85, 0xB3, 0xE6, 0x9C, 0xBA, 0};//系统关机utf8
		memset(&msg, 0, sizeof(msg));
		msg.type = EM_DVR_EXIT_NORMOAL;
		msg.chn = 0;
		strcpy(msg.note, shutdown);
		upload_sg(&msg);
		
		return BIZ_SUCCESS;
	}
	
	printf("BizSysComplexExit-5\n");
	fflush(stdout);
	
	pthread_mutex_unlock(&SHUT_MUTEX);
	
	return BIZ_ERR_PREVIEW_FAILEDCMD;
}

/*
	函数功能:系统升级
	输入参数：
		emUpdateTarget:
			类型:EMBIZUPGRADETARGET
			含义:升级目标（主板/面板）
		pcUpdateFName：
			类型:char *
			含义:升级文件路径（本地升级可以为空，其他不能为空）
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 BizSysComplexUpgrade(EMBIZUPGRADETARGET emUpdateTarget, char *pcUpdateFName)
{
	s32 nRet = 0;
	SBizUpgrade sPara;

	sPara.emUpdateTarget = emUpdateTarget;
	sPara.pcUpdateFName = pcUpdateFName;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_SYSCOMPLEX_UPGRADE;
	sBizMsgHeader.pData = &sPara;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = SysComplexUpgrade(&sPara);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}

	return BIZ_ERR_PARAGET_FAILEDCMD;
}


//函数功能:配置文件列表
s32 BizSysComplexConfigFilelist(char* szDir, SBizUpdateFileInfoList* spFileList)
{
	if(!spFileList || !spFileList->pInfo)
	{
		return BIZ_ERR_PARAGET_NULLPARA;
	}     

	s32 nRet = 0;
	SBizConfigFileList sPara;

	sPara.szDir = szDir;
	sPara.spFileList = spFileList;
#ifndef DIRECT_PARA_GETSET	
	SBizManager* psBizManager = &g_sBizManager;
    	SBizMsgHeader sBizMsgHeader;

   	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
    	sBizMsgHeader.emMsgType = EM_BIZMSG_SYSCOMPLEX_CONFIG_FILELIST;
   	sBizMsgHeader.pData = &sPara;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = SysComplexConfigFilelist(&sPara);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}

    return BIZ_ERR_PARAGET_FAILEDCMD;
}

//函数功能:usb设备挂载路径列表
s32 BizSysComplexUsbDevMountDirlist(SBizUpdateFileInfoList* spFileList)
{
	if(!spFileList || !spFileList->pInfo)
	{
		return BIZ_ERR_PARAGET_NULLPARA;
	}

	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET	
	SBizManager* psBizManager = &g_sBizManager;
    	SBizMsgHeader sBizMsgHeader;

   	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
    	sBizMsgHeader.emMsgType = EM_BIZMSG_SYSCOMPLEX_MOUNTED_USB_DIR_LIST;
    	sBizMsgHeader.pData = spFileList;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = SysComplexMountUsbDirlist(spFileList);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}

    	return BIZ_ERR_PARAGET_FAILEDCMD;
}

s32 BizSysComplexUpgradeFilelist(EMBIZUPGRADETARGET emUpdateTarget, SBizUpdateFileInfoList* spFileList)
{
	if(!spFileList || !spFileList->pInfo)
	{
		return BIZ_ERR_PARAGET_NULLPARA;
	}
	
	s32 nRet = 0;
	SBizUpgradeFileInfo sPara;

	sPara.emUpdateTarget = emUpdateTarget;
	sPara.spFileList = spFileList;
#ifndef DIRECT_PARA_GETSET		
	SBizManager* psBizManager = &g_sBizManager;
    	SBizMsgHeader sBizMsgHeader;

   	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
    	sBizMsgHeader.emMsgType = EM_BIZMSG_SYSCOMPLEX_UPGRADE_FILELIST;
    	sBizMsgHeader.pData = &sPara;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = SysComplexUpgradeFilelist(&sPara);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}

    	return BIZ_ERR_PARAGET_FAILEDCMD;


}


/*
	函数功能:文件备份
	输入参数：
		emFileType:
			类型:EMBIZFILETYPE
			含义:文件类型
		sBackTgtAtt:
			类型:PSBizBackTgtAtt
			含义:输出的文件格式及路径		
		psSearchCondition：
			类型:SSearchCondition*
			含义:搜索条件
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 BizSysComplexBackup(EMBIZFILETYPE emFileType, 
						 PSBizBackTgtAtt	psBackTgtAtt, 
						 SBizSearchCondition* psSearchCondition)
{
	s32 nRet = 0;
	SBizBackup sPara;
	
	sPara.emFileType = emFileType;
	sPara.psBackTgtAtt = psBackTgtAtt;
	sPara.psSearchCondition = psSearchCondition;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;
	
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_SYSCOMPLEX_BACKUP;
	sBizMsgHeader.pData = &sPara;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = SysComplexBackup(&sPara);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
	return BIZ_ERR_PARAGET_FAILEDCMD;
}

void BizSysComplexSetProductModel(char* updatemodel)
{
	SysComplexSetProductModel(updatemodel);
}

u8 BizSysGetInsertDiskStatue()
{
	return SysGetInsertDiskStatue();
}

/*
函数功能:中断文件备份
*/
s32 BizSysCmplexBreakBackup(int type)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_SYSCOMPLEX_BREAK_BACKUP;
	sBizMsgHeader.pData = type;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = SysComplexBreakBackup(type);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}

    	return BIZ_ERR_PREVIEW_FAILEDCMD;
}

/*
	函数功能:获得存储设备管理信息，一个参数，设备管理信息结构体（数量及各设备信息）
	输出参数：
		psStoreDevManage:
			类型:SBizStoreDevManage
			含义:存储设备管理信息
	        nMaxDiskNum:
			类型:u8
			含义:支持返回的最大磁盘数 
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 BizSysComplexDMGetInfo(SBizStoreDevManage *psStoreDevManage, u8 nMaxDiskNum)
{
	s32 nRet = 0;
	SBizDiskManage sPara;

	sPara.psStoreDevManage = psStoreDevManage;
	sPara.nMaxDiskNum = nMaxDiskNum;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_SYSCOMPLEX_DISKMANAGE;
	sBizMsgHeader.pData = &sPara;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = SysComplexDiskManage(&sPara);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}


	return BIZ_ERR_PARAGET_FAILEDCMD;
}

/*
	函数功能:格式化
	输入参数：
		pstrDevPath:
			类型:char *
			含义:设备路径
		emFormatWay:
			类型:EMBIZFORMATWAY
			含义:格式化格式及方式
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 BizSysComplexDMFormat(char *pstrDevPath, EMBIZFORMATWAY emFormatWay)
{
	s32 nRet = 0;
	SBizDiskFormat sPara;

	sPara.pstrDevPath = pstrDevPath;
	sPara.emFormatWay = emFormatWay;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_SYSCOMPLEX_DISKFORMAT;
	sBizMsgHeader.pData = &sPara;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = SysComplexDiskFormat(&sPara);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}

	return BIZ_ERR_PARAGET_FAILEDCMD;
}

/*
	函数功能:文件搜索
	输入参数：
		emFileType:
			类型:EMBIZFILETYPE
			含义:文件类型
		psSearchPara:
			类型:SBIZSearchPara
			含义:搜索条件
		psSearchResul:
			类型:SBizSearchResult
			含义:传入用于存放搜索结果结构体
		nMaxFileNum:
			类型:u16
			含义:psRecfileList的最大个数
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 BizSysComplexDMSearch(EMBIZFILETYPE emFileType, SBizSearchPara* psSearchPara, SBizSearchResult* psSearchResult, u16 nMaxFileNum)
{
	s32 nRet = 0;
	SBizDiskSearch sPara;
	
	sPara.emFileType = emFileType;
	sPara.psSearchPara = psSearchPara;
	sPara.psSearchResult = psSearchResult;
	sPara.nMaxFileNum = nMaxFileNum;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;
	
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_SYSCOMPLEX_DISKSEARCH;
	sBizMsgHeader.pData = &sPara;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = SysComplexDiskSearch(&sPara);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
	return BIZ_ERR_PARAGET_FAILEDCMD;
}

/*
	函数功能:设置系统日期时间
	输入参数：
		psDateTime:
			类型:const SBizDateTime*
			含义:用于设置日期时间
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 BizSysComplexDTSet(const SBizDateTime* psDateTime)
{
#ifndef DIRECT_PARA_GETSET
    SBizManager* psBizManager = &g_sBizManager;
    SBizMsgHeader sBizMsgHeader;
#endif
	
	if (NULL == psDateTime)
	{
	    return BIZ_ERR_SYSCOMPLEXDTSET_NULLPARA;
	}
	
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET	
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_SYSCOMPLEX_DTSET;
	sBizMsgHeader.pData = (void *)psDateTime;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = SysComplexDTSet(psDateTime);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
	return BIZ_ERR_PARAGET_FAILEDCMD;
}

s32 BizSysComplexSetTimeZone(int nTimeZone)
{
	s32 nRet = SysComplexSetTimeZone(nTimeZone);
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
	return BIZ_ERR_PARAGET_FAILEDCMD;
}

/*
	函数功能:获得系统日期时间
	输入参数：
		psDateTime:
			类型:SBizDateTime*
			含义:用于获得日期时间
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 BizSysComplexDTGet(SBizDateTime* psDateTime)
{
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;
#endif
	
	if (NULL == psDateTime)
	{
	   	return BIZ_ERR_SYSCOMPLEXDTGET_NULLPARA;
	}
	
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET	
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_SYSCOMPLEX_DTGET;
	sBizMsgHeader.pData = (void *)psDateTime;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = SysComplexDTGet(psDateTime);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
	return BIZ_ERR_PARAGET_FAILEDCMD;
}

s32 BizSysComplexDTGet_TZ(SBizDateTime* psDateTime)
{
	s32 nRet = SysComplexDTGet_TZ(psDateTime);
	if(0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
	return BIZ_ERR_PARAGET_FAILEDCMD;
}

/*
	函数功能:获得格式化系统日期时间字符串
	输入参数：
		pstrDateTime:
			类型:char*
			含义:用于获得日期时间字符串(先按1.0默认YYYY-MM-DD HH:NN:SS)
	返回：
		类型:s32
		含义:返回成功,失败,或错误值
		取值:0成功;非0失败或错误值
*/
s32 BizSysComplexDTGetString(char* pstrDateTime)
{
#ifndef DIRECT_PARA_GETSET	
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;
#endif
	if (NULL == pstrDateTime)
	{
	    	return BIZ_ERR_SYSCOMPLEXDTGET_NULLPARA;
	}
	
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_SYSCOMPLEX_DTGETSTR;
	sBizMsgHeader.pData = (void *)pstrDateTime;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = SysComplexDTGetStr(pstrDateTime);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
	return BIZ_ERR_PARAGET_FAILEDCMD;
}

s32 BizGetVMainEncodePara(u32 bDefault,SBizEncodePara *psPara, u8 nId)
{
	return ConfigGetVMainEncodePara(bDefault, psPara, nId);
}

s32 BizSetVMainEncodePara(SBizEncodePara *psPara, u8 nId)
{
	return ConfigSetVMainEncodePara(psPara, nId);
}

s32 BizGetVSubEncodePara(u32 bDefault, SBizEncodePara * psPara, u8 nId)
{
	return ConfigGetVSubEncodePara(bDefault, psPara, nId);
}

s32 BizSetVSubEncodePara(SBizEncodePara * psPara, u8 nId)
{
	return ConfigSetVSubEncodePara(psPara, nId);
}

s32 BizGetIsPatrolPara(u32 bDefault,SBizCfgPatrol *psPara, u8 nId)
{
	return ConfigGetIsPatrolPara(bDefault, psPara, nId);
}

s32 BizSetIsPatrolPara(SBizCfgPatrol *psPara, u8 nId)
{
	return ConfigSetIsPatrolPara(psPara, nId);
}

s32 BizStartManualRec(u8 nChn)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;
	
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_START_MANUAL_REC;
	sBizMsgHeader.pData = (void *)(u32)nChn;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = RecordStartManual(nChn);
#endif
	if(0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
	return BIZ_ERR_RECORD_FAILEDCMD;
}

s32 BizStopManualRec(u8 nChn)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET	
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_STOP_MANUAL_REC;
	sBizMsgHeader.pData = (void *)(u32)nChn;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = RecordStopManual(nChn);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}

	return BIZ_ERR_RECORD_FAILEDCMD;
}

s32 BizRecordStop(u8 nChn)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET	
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_STOP_REC;
	sBizMsgHeader.pData = (void *)(u32)nChn;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = RecordStop(nChn);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}

	return BIZ_ERR_RECORD_FAILEDCMD;
}

s32 BizRecordStopAll(void)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET	
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_STOP_ALLREC;
	sBizMsgHeader.pData = NULL;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = RecordStopAll();
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
	return BIZ_ERR_RECORD_FAILEDCMD;
}

//yaogang modify 20150314
//0: format 1: hotplugup 2:update
s32 BizSnapRecordPause(u8 cause)
{
	BizSnapPause();//yaogang 20150715
	RecordSnapPause(cause);
}

s32 BizSnapRecordResume(u8 cause)
{
	RecordSnapResume(cause);
	BizSnapResume();//yaogang 20150715
}

s32 BizRecordPause(u8 nChn)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET	
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_PAUSE_REC;
	sBizMsgHeader.pData = (void *)(u32)nChn;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = RecordPause(nChn);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}

	return BIZ_ERR_RECORD_FAILEDCMD;
}

s32 BizRecordResume(u8 nChn)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET	
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_RESUME_REC;
	sBizMsgHeader.pData = (void *)(u32)nChn;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = RecordResume(nChn);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}

	return BIZ_ERR_RECORD_FAILEDCMD;
}

void BizRecordSetWorkingEnable(u8 nEnable)
{
	RecordSetWorkingEnable(nEnable);
}

s32 BizAlarmWorkingEnable(u8 nEnable)
{
	return AlarmWorkingEnable(nEnable);
}

s32 BizStartPlayback(EMBIZPLAYTYPE emBizPlayType, void* para)
{
	s32 nRet = 0;
	SBizPbStart sBizPbStart;
	
	sBizPbStart.emBizPlayType = emBizPlayType;
	sBizPbStart.para = para;
#ifndef DIRECT_PARA_GETSET
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;
	
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_START_PLAYBACK;
	sBizMsgHeader.pData = (void *)&sBizPbStart;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = PlaybackStart(&sBizPbStart);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
	return BIZ_ERR_PLAYBACK_FAILEDCMD;
}

s32 BizPlaybackControl(EMBIZPLAYCTRLCMD emBizPlayCtrlCmd, s32 nContext)
{
    SBizManager* psBizManager = &g_sBizManager;
    SBizMsgHeader sBizMsgHeader;
    SBizPbControl sBizPbCtl;
	
    sBizPbCtl.emPBCtlcmd = emBizPlayCtrlCmd;
    sBizPbCtl.nContext = nContext;
	
#if 1
	if(EM_BIZCTL_SET_SPEED == emBizPlayCtrlCmd)
	{
		SetCurPlayRate(nContext);
	}
	
	PlayBackControl(&sBizPbCtl);
	
	return BIZ_SUCCESS;//csp modify
#else
    memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
    sBizMsgHeader.emMsgType = EM_BIZMSG_PLAYBACK_CTRL;
    sBizMsgHeader.pData = (void *)&sBizPbCtl;
    if (0 == WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader))
    {
        return BIZ_SUCCESS;
    }

    return BIZ_ERR_PLAYBACK_FAILEDCMD;
#endif
}

//yaogang modify 20150112
s32 BizSnapDisplay(SBizRecSnapInfo *pSnapInfo)
{
	return SnapDisplay(pSnapInfo);
}
s32 BizPtzCtrl(u8 nChn, EMBIZPTZCMD emCmd, s16 nId)
{
	s32 nRet = 0;
	static u32 nMax = 0;
	SBizPtzCtrl sBizPtzCtrl;
	SBizDvrInfo sInfo;

	//printf("yg BizPtzCtrl emCmd: %d\n", emCmd);

	if (0 == nMax) {
		ConfigGetDvrInfo(0, &sInfo);
		nMax = sInfo.nVidMainNum;
	}	

	sBizPtzCtrl.emCmd = emCmd;
	sBizPtzCtrl.nChn = nChn;
	sBizPtzCtrl.nId = nId;

	//add by Lirl on Nov/30/2011
	if (!(nChn>=0 && nChn<nMax)) {
		return BIZ_ERR_PTZ_FAILEDCMD;
	}

	switch(emCmd)
	{
		case EM_BIZPTZ_CMD_NULL:
			{
				return BIZ_ERR_PTZ_FAILEDCMD;
			}
			break;
		case EM_BIZPTZ_CMD_PRESET_GOTO:
		case EM_BIZPTZ_CMD_PRESET_SET:
			{
				if (!(nId>=0 && nId<=TOURPIONT_MAX)) {
					return BIZ_ERR_PTZ_FAILEDCMD;
				}
			}
			break;
		case EM_BIZPTZ_CMD_START_TOUR:
		case EM_BIZPTZ_CMD_STOP_TOUR:
			{
				if (!(nId>=0 && nId<TOURPATH_MAX)) {
					return BIZ_ERR_PTZ_FAILEDCMD;
				}
			}
			break;
		case EM_BIZPTZ_CMD_SETSPEED:
			{
				if (!(nId>0 && nId<=PTZMOVESPEED_MAX)) {
					return BIZ_ERR_PTZ_FAILEDCMD;
				}
			}
			break;
		default:
			{
				#if 0
				if (0 != nId) {
					return BIZ_ERR_PTZ_FAILEDCMD;
				}
				#endif
			}
			break;
	}
	//end
	
#ifndef DIRECT_PARA_GETSET	
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_PTZ_CTRL;
	sBizMsgHeader.pData = (void *)&sBizPtzCtrl;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = PtzCtrl(&sBizPtzCtrl);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}

	return BIZ_ERR_PTZ_FAILEDCMD;
}

s32 BizPtzCheckTouring(u8 nChn)
{
	return PtzCheckIsTouring(nChn);
}

s32 BizPlayBackZoom(s32 nKey)
{
	return PlayBackZoom(nKey);
}

s32 BizPlayBackGetRealPlayChn(u64* pChnMask)
{
	return PlayBackGetRealPlayChn(pChnMask);
}

s32 BizPlayBackGetVideoFormat(u8 nChn)
{
	return PlayBackGetVideoFormat(nChn);
}

s32 BizPlayBackSetModeVOIP(u8 nVoip)
{
	return PlayBackSetModeVOIP(nVoip);
}

s32 BizConfigBackup(char* pszFileName)
{
	s32 nRet = 0;
#ifndef DIRECT_PARA_GETSET	
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_BACKUP_CONFIG;
	sBizMsgHeader.pData = pszFileName;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = ConfigBackup(pszFileName);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}

	return BIZ_ERR_CONFIG_FAILEDCMD;
}

s32 BizConfigGetDvrInfo(u32 flag, char *sDvr_version, char *sDvr_Model)
{
	SBizDvrInfo sDvr_info;
	memset(&sDvr_info, 0, sizeof(sDvr_info));
	ConfigGetDvrInfo(0,&sDvr_info);
	
	strcpy(sDvr_version , sDvr_info.strVersion);
	strcpy(sDvr_Model , sDvr_info.strModel);
	
	return BIZ_SUCCESS;//csp modify
}

s32 BizSendBizEvent(SBizEventPara * psBizEventPara)
{
	return SendBizEvent(psBizEventPara);
}

s32 BizConfigResumeDefault(EMBIZCONFIGPARATYPE emType, s32 nId)
{
	return ConfigResumeDefault(emType, nId);
}

//枚举必须与底层配置模块EMCONFIGPARATYPE 匹配
s32 BizConfigResume(EMBIZCONFIGPARATYPE emType, s32 nId, s8* pFilePath)
{
	s32 nRet = 0;
	SBizCfgResume sCfgResume;

	if(!pFilePath)
	{
		return BIZ_ERR_SYSCOMPLEXDTGET_NULLPARA;
	}

	sCfgResume.emType = emType;
	sCfgResume.nId = nId;
	strcpy(sCfgResume.pFilePath, pFilePath);
#ifndef DIRECT_PARA_GETSET	
	SBizManager* psBizManager = &g_sBizManager;
	SBizMsgHeader sBizMsgHeader;

	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_RESUME_CONFIG;
	sBizMsgHeader.pData = &sCfgResume;
	nRet = WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader);
#else
	nRet = ConfigResume(&sCfgResume);
#endif
	if (0 == nRet)
	{
		return BIZ_SUCCESS;
	}
	
    return BIZ_ERR_CONFIG_FAILEDCMD;
}

static int gResolHNum=0;
void SetBizResolHNum(int num)//cw_9508S
{
	gResolHNum=num;
}
int GetBizResolHNum()
{
	return gResolHNum;		
}

//获得参数字符串取值列表
s32 BizConfigGetParaStr(EMBIZCFGPARALIST emParaType, PARAOUT s8* strList[], PARAOUT u8* nRealListLen, u8 nMaxListLen, u8 nMaxStrLen)
{
    SBizManager* psBizManager = &g_sBizManager;
    SBizMsgHeader sBizMsgHeader;
    SBizConfigParaList sBizConfigParaList;
	//PUBPRT("HERE");
    memset(&sBizConfigParaList, 0, sizeof(SBizConfigParaList));
	
    sBizConfigParaList.emParaType = emParaType;
    int i;
    for (i = 0; i < nMaxListLen; i++)
    {
        sBizConfigParaList.strList[i] = strList[i];
    }
    sBizConfigParaList.pnRealListLen = nRealListLen;
    sBizConfigParaList.nMaxListLen = nMaxListLen;
    sBizConfigParaList.nMaxStrLen = nMaxStrLen;
	//PUBPRT("HERE");
    memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
    sBizMsgHeader.emMsgType = EM_BIZMSG_GETPARALIST;
    sBizMsgHeader.pData = (void *)&sBizConfigParaList;
	
	//int start = GetTimeTick();
	if(0 == ConfigGetParaStr((SBizConfigParaList *)sBizMsgHeader.pData))
	{
		//int end = GetTimeTick();
		//printf("sBizConfigParaList.strList[0]: %s\n", sBizConfigParaList.strList[0]);
		return BIZ_SUCCESS;
	}
	
	/*
	if (0 == WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader))
	{
		return BIZ_SUCCESS;
	}
	//PUBPRT("HERE");
	*/
	
    return BIZ_ERR_CONFIG_FAILEDCMD;
}

//根据参数取值获得字符串列表中的index
s32 BizConfigGetParaListIndex(EMBIZCFGPARALIST emParaType, s32 nValue, PARAOUT u8* pnIndex)
{
    SBizManager* psBizManager = &g_sBizManager;
    SBizMsgHeader sBizMsgHeader;
    SBizConfigParaIndex sBizConfigParaIndex;
	
    sBizConfigParaIndex.emParaType = emParaType;
    sBizConfigParaIndex.nValue = nValue;
    sBizConfigParaIndex.pnIndex = pnIndex;
	
	/*
	memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
	sBizMsgHeader.emMsgType = EM_BIZMSG_GETPARAINDEX;
	sBizMsgHeader.pData = (void *)&sBizConfigParaIndex;
	if (0 == WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader))
	{
		return BIZ_SUCCESS;
	}
	*/
	
	//int start = GetTimeTick();
	if(0 == ConfigGetParaListIndex((SBizConfigParaIndex *)&sBizConfigParaIndex))
	{
		//int end = GetTimeTick();
		//printf("ConfigGetParaListIndex: end-start = %d\n",end-start);
		return BIZ_SUCCESS;
	}
	
    return BIZ_ERR_CONFIG_FAILEDCMD;
}

//根据index获得实际参数取值
s32 BizConfigGetParaListValue(EMBIZCFGPARALIST emParaType, u8 nIndex, PARAOUT s32* pnValue)
{
    SBizManager* psBizManager = &g_sBizManager;
    SBizMsgHeader sBizMsgHeader;
    SBizConfigParaValue sBizConfigParaValue;

    sBizConfigParaValue.emParaType = emParaType;
    sBizConfigParaValue.nIndex = nIndex;

	//printf(" sBizConfigParaValue.nIndex  = %d \n", sBizConfigParaValue.nIndex);
    sBizConfigParaValue.pnValue = pnValue;

    memset(&sBizMsgHeader, 0, sizeof(SBizMsgHeader));
    sBizMsgHeader.emMsgType = EM_BIZMSG_GETPARAVALUE;
    sBizMsgHeader.pData = (void *)&sBizConfigParaValue;
	
	/*
	if (0 == WriteMsgCmd(psBizManager->psMsgCmdHdr, &sBizMsgHeader))
	{
		return BIZ_SUCCESS;
	}
	*/
	
	if(0 == ConfigGetParaListValue((SBizConfigParaValue *)&sBizConfigParaValue))
	{
		return BIZ_SUCCESS;
	}
	
    return BIZ_ERR_CONFIG_FAILEDCMD;
}

s32 BizConfigDefault(void)//恢复默认设置
{
	return ConfigDefault();
}

//yaogang modify 2015020
int BizRequestSnap(u8 chn, int type, const char *PoliceID, const char *pswd)
{
	//printf(stdout, "%s yg 123\n", __func__);
	return RequestSnap(chn, (EM_SNAP_TYPE) type, PoliceID, pswd);
}

int BizSnapAlarmStatusChange(u8 chn, int type, int status)
{
	//printf(stdout, "%s yg 123\n", __func__);
	return AlarmStatusChange(chn, (EM_ALARM_TYPE)type, status);
}


