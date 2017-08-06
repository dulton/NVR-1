#include "GUI/Pages/BizData.h"
#include "biz.h"
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "GUI/Pages/PageStartFrameWork.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/PagePlayBackFrameWork.h"
#include "GUI/Pages/PageStartFrameWork.h"
#include "GUI/Pages/PageChnSelect.h"
#include "GUI/Pages/PageAudioSelect.h"
#include "GUI/Pages/PageColorSetup.h"
#include "GUI/Pages/PagePtzCtl.h"
#include "GUI/Pages/PageDesktop.h"
#include "GUI/Pages/PageDataBackup.h"
#include "GUI/GDI/Region.h"
#include "GUI/Pages/PageUpdate.h"
#include "GUI/Pages/PageDiskMgr.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageSensorAlarm.h"
#include "GUI/Pages/PageIPCameraExtSensorAlarm.h"
#include "GUI/Pages/PageMDCfg.h"
#include "GUI/Pages/PageVideoLoss.h"
#include "GUI/Pages/PagePtzPreset.h"
#include "GUI/Pages/PagePtzTrack.h"
#include "GUI/Pages/PageCruiseLine.h"
#include "GUI/Pages/PagePtzConfigFrameWork.h"
#include "GUI/Pages/PageLiveConfigFrameWork.h" 
#include "Devices/DevFrontboard.h"
#include "Devices/DevMouse.h"
#include "GUI/Pages/PageNetworkConfig.h"
#include "GUI/Pages/PageBackupInfo.h"
#include "GUI/Pages/PageInfoBox.h"
#include "GUI/Pages/PageUpdate.h"
#include "GUI/Pages/PageRecordFrameWork.h"
#include "GUI/Pages/PageShenGuangConfig.h"
#include "GUI/Pages/PageBasicConfigFrameWork.h"


#include "GUI/Pages/PageAlarmList.h"
#include "sg_platform.h"


int GetMaxChnNum()
{
	return GetVideoMainNum();
}

int GetMaxSensorChnNum()
{
	return GetSenSorNum();
}

//#ifdef CHIP_HISI351X_2X
//#define longse
//#endif

/*
#include "System/Signals.h"
#include "APIs/Frontboard.h"
#include "MultiTask/Thread.h"
#include "MultiTask/Timer.h"
#include "System/Object.h"
*/

#if 1//csp modify 20130509
#define PIC_STATE_OFFSET_X 10//250 //状态小图标距离通道左侧的偏移量，参考标准:720x576
#define PIC_STATE_OFFSET_Y 524//20//525//550 //状态小图标距离通道顶部的偏移量，参考标准:720x576
#else
#define PIC_STATE_OFFSET_X 10//250 //状态小图标距离通道左侧的偏移量，参考标准:720x576
#define PIC_STATE_OFFSET_Y 530//20//525//550 //状态小图标距离通道顶部的偏移量，参考标准:720x576
#endif

#define CHNNAME_HEIGHT 20

static int bRecord[32] = {0};
static int bMD[32] = {0};
static int bSensorAlarm[32] = {0};
static int bIPCExtAlarm[32] = {0};
static int bIPCCoverAlarm[32] = {0};



int bizData_GetSch(SBizSchedule* pSchPara, uint* pTgtSch);

#define MAX_HDD_NUM	8

u8 GetVideoMainNum(void)
{
	s32         ret;
	u8 			nChMax = 4;
	SBizDvrInfo sInf;
	
	ret = BizGetDvrInfo(&sInf);
	if(ret == 0)
	{
		nChMax = sInf.nVidMainNum;
	}
	
	return nChMax;
}

s32 FxnKeycodeShut()
{
	char buf[2]={0};
	
	buf[0] = 0x04;
	//buf[1] = 0x00;
	//yaogang modify 20141130
	buf[1] = 0x10;//正在关机，请等待完成

	printf("%s : system shutdown is in progress, please wait!!!\n", __FUNCTION__);
	int fret = FrontboardWrite(buf,2);
	if(fret <= 0)
	{
		printf("FxnKeycodeShut error-1\n");
		return FALSE;
	}
	
	/*
	const char* szInfo;
	const char* szType;
	
	UDM ret;
	
	szInfo = GetParsedString("&CfgPtn.ShutDownNote");
	szType = GetParsedString("&CfgPtn.WARNING");
	ret = MessageBox(szInfo, szType , MB_OKCANCEL|MB_ICONWARNING|MB_DEFBUTTON2);
	if(ret == UDM_OK)
	*/
	
	if(0 != BizSysComplexExit(EM_BIZSYSEXIT_POWEROFF))
	{
		return FALSE;
	}

	printf("%s : NVR system has been turned off!!!\n", __FUNCTION__);
	buf[0] = 0x04;
	//buf[1] = 0x01;
	//yaogang modify 20141130
	buf[1] = 0x11;//关机完成，可以关闭ATX电源了
	
	if(FrontboardWrite(buf,2) <= 0)
	{
		printf("FxnKeycodeShut error-2\n");
		return FALSE;
	}
	
	return TRUE;
}

void RestartSystem()
{
	BizSysComplexExit(EM_BIZSYSEXIT_RESTART);
}

static u8 g_LedStatus = 0;//cw_led
static pthread_mutex_t LED_MUTEX = PTHREAD_MUTEX_INITIALIZER;

#ifdef longse
//zhao1
#define LED_REC 	1
#define LED_ALARM 	2

/*args:
	which: control which led
	ctl: 1->unlight, 0->light
*/
extern "C" int tl_led_ctl(int which, int ctl);
#endif

void RecLedCtrl(int flag,int type)
{
	pthread_mutex_lock(&LED_MUTEX);
#ifdef longse
	if(type == REC_LED_CTRL)//REC_LED_CTRL
	{
		tl_led_ctl(LED_REC,!flag);
	}
	else//ALARM_LED_CTRL
	{
		tl_led_ctl(LED_ALARM,!flag);
	}
#else
	char buf[2]={0};
	buf[0] = 0x20;
	
	u8 is_hi3520d_chip = 0;
	#ifdef HI3520D
	is_hi3520d_chip = 1;
	#endif
	
	char tmpchar[20] = {0};
	GetProductNumber(tmpchar);
	//printf("RecLedCtrl GetProductNumber:%s\n",tmpchar);
	if((0 == strcasecmp(tmpchar, "R9508S"))
		||(0 == strcasecmp(tmpchar, "R9504S"))
		||(0 == strcasecmp(tmpchar, "R9516S"))
		||(0 == strcasecmp(tmpchar, "R1008"))
		||(0 == strcasecmp(tmpchar, "R1004"))
		||(0 == strcasecmp(tmpchar, "R1004W"))
		||is_hi3520d_chip)
	{
		if(type == REC_LED_CTRL)
		{
			if(flag == 0)
				g_LedStatus = 0x01;
			else
				g_LedStatus = 0x81;
		}
		else//ALARM_LED_CTRL
		{
			if(flag == 0)
				g_LedStatus = 0x03;
			else
				g_LedStatus = 0x83;
		}
		//printf("hehe1,g_LedStatus=0x%x\n",g_LedStatus);
	}
	else
	{
		if(flag == 0)
			g_LedStatus &= (~(1 << type));
		else
			g_LedStatus |= (1 << type);
		//printf("hehe2,g_LedStatus=0x%x\n",g_LedStatus);
	}
	buf[1] = g_LedStatus;
	FrontboardWrite(buf,2);
#endif
	pthread_mutex_unlock(&LED_MUTEX);
}

void SetZoomMode(u8 flag)//cw_zoom
{
	BizSetZoomMode(flag);
}

static u8 g_playbackstatus = 0;//cw_remote
u8 GetPlayBakStatus()
{
	return g_playbackstatus;
}
void SetPlayBakStatus(int flag)
{
	g_playbackstatus = flag;
}

static u8 nFormat = 3;
void BizSetPlayBackFileFormat(u8 flag)
{
	//printf("SetPlayBackFileFormat flag=%d\n",flag);
	nFormat = flag;
}
s32 BizGetPlayBackFileFormat()
{
	//printf("GetPlayBackFileFormat flag=%d\n",nFormat);
	return nFormat;
}

static EMBIZPREVIEWMODE g_curPreviewMode;//cw_preview
EMBIZPREVIEWMODE GetCurPreviewMode_CW()
{
	return g_curPreviewMode;
}
void SetCurPreviewMode_CW(EMBIZPREVIEWMODE mode)
{
	g_curPreviewMode = mode;
}

void SetDwellStartFlag(u8 flag)
{
	BizSetDwellStartFlag(flag);
}

u8 GetDwellStartFlag()
{
	return BizGetDwellStartFlag();
}

u8 GetSenSorNum()
{
	s32         ret;
	SBizDvrInfo sInf;

	ret = BizGetDvrInfo(&sInf);
	if(ret == 0)
	{
		return sInf.nSensorNum;
	}
}

u8 GetAlarmOutNum()
{
	s32         ret;
	SBizDvrInfo sInf;

	ret = BizGetDvrInfo(&sInf);
	if(ret == 0)
	{
		return sInf.nAlarmoutNum;
	}
}

u8 GetAudioNum(void)
{
	s32         ret;
	SBizDvrInfo sInf;

	ret = BizGetDvrInfo(&sInf);
	
	return sInf.nAudNum;
}

u8 GetAlarmNum(void)
{
	return 1;
}

void GetProductNumber(s8* pProduct)
{
	if(pProduct)
	{
		s32 ret;
		SBizDvrInfo sInf;
		ret = BizGetDvrInfo(&sInf);
		if(ret == 0)
		{
			strcpy(pProduct, sInf.sproductnumber);
		}
	}
}

#if 0//csp modify 20121222
int Pthread_Create(pthread_t* pid, void* attr, void* func, void* para)
{
	return 0;//pthread_create(pid, NULL, (void*)func, NULL);
}
#endif

void ResetRecState(int value)
{
	//memset(bRecord,0,sizeof(bRecord));
	for(int i=0; i<GetVideoMainNum(); i++)
	{
		bRecord[i] = value;
	}
}

s32 SwitchPreviewEx(EMBIZPREVIEWMODE emMode, u8 nModePara)
{
	s32 rtn = 0;
	
	if (0xff == nModePara)
	{
		//printf("SwitchPreviewEx - BizNextPreview\n");
		rtn = BizNextPreview();
	}
	else
	{
		SBizPreviewPara sBPP;
		
		sBPP.emBizPreviewMode = emMode;
		sBPP.nModePara = nModePara;
		
		rtn = BizStartPreview(&sBPP);
	}

	return rtn;
}

void SwitchPreview(EMBIZPREVIEWMODE emMode, u8 nModePara)
{
	if(0xff == nModePara)
	{
		//printf("SwitchPreview - BizNextPreview\n");
		BizNextPreview();
	}
	else
	{
		SBizPreviewPara sBPP;
		
		sBPP.emBizPreviewMode = emMode;
		sBPP.nModePara = nModePara;
		
		BizStartPreview(&sBPP);
	}
}

void PreviewToNextPic(void)
{
	//printf("PreviewToNextPic - BizNextPreview\n");
	BizNextPreview();
}

void PreviewToLastPic(void)
{
	BizLastPreview();
}

void PreviewToNextMod(void)
{
	BizNextPreviewMod();
}

void PreviewToLastMod(void)
{
	BizLastPreviewMod();
}

s32 PreviewElecZoom(int flag, s32 nChn, SBizPreviewElecZoom* stCapRect)
{
	BizElecZoomPreview(flag, nChn, stCapRect);
}

s32 PreviewSnapSet(int flag, SBizPreviewElecZoom* stMenuRect)
{
	BizPreviewSnap(flag, stMenuRect);
}


static unsigned char g_PbElecZoom_flag = 0;//csp modify 20130509

s32 PreviewPbElecZoom(int flag, s32 nChn, SBizPreviewElecZoom* stCapRect)
{
	BizPbElecZoomPreview(flag, nChn, stCapRect);
	
#if defined(CHIP_HISI3531)//csp modify 20130509
	CPage** page = GetPage();
	CPagePlayBackFrameWork* pPagePlayBack = (CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK];
	if(pPagePlayBack->GetPbPauseStatue())
	{
		//回放结束时的停止电子放大,如果此时进行回放控制会造成死循环
		if(g_PbElecZoom_flag == (!flag))
		{
			BizPlaybackControl(EM_BIZCTL_STEP, 0xfd);
		}
		else
		{
			BizPlaybackControl(EM_BIZCTL_STEP, 0xfd);
		}
	}
#endif
	
	g_PbElecZoom_flag = !flag;//csp modify 20130509
}

void ControlPatrol(u8 nEnable)
{
	if(nEnable)
	{
		BizStartPatrol();
	}
	else
	{
		BizStopPatrol();
	}
}

void ControlAudioOutChn(u8 nChn)
{
	BizPreviewAudioOutChn(nChn);
}

void ControlMute(u8 nEnable)
{
	BizPreviewMute(nEnable);
}

void ControlVolume(u8 nVolume)
{
	BizPreviewVolume(nVolume);
}

s32 PreviewSetImageColor(u8 nChn, SBizPreviewImagePara* psImagePara)
{
	return BizPreviewSetImage(nChn, psImagePara);
}

s32 PlayBackSetImageColor(SBizVoImagePara* psVoImagePara)
{
	return BizPlayBackSetImage(psVoImagePara);
}
	
static CPage** g_pPages = NULL;//用于存放回放需要用到的页面数组

//zlb20111117 去掉多余代码
//static char** strChn = NULL;
//static int* chnNameEnabel = NULL;

void SetPage(EM_PAGE emPage, CPage* pPage)
{
	//PUBPRT("Here");
	if(NULL == g_pPages)
	{
		g_pPages = (CPage**)malloc(sizeof(u32) * EM_PAGE_NUM);
		
		memset(g_pPages, 0, sizeof(u32) * EM_PAGE_NUM);
	}
	//PUBPRT("Here");
	if(emPage < EM_PAGE_NUM)
	{
		g_pPages[emPage] = pPage;
	}
	//PUBPRT("Here");
}

CPage** GetPage()
{
	return g_pPages;
}

void BoardGetReUsedKey(u8 key, u8* pKeyNew)
{
	FrontBoardGetReUsedKey(key, pKeyNew);
}

static u32 g_nUserId = 0;//csp modify

static char g_userName[15] = {0};//csp modify
static u8 g_IsLogined = 0;

u32 GetGUILoginedUserID()
{
	return g_nUserId;
}

void SetGUILoginedUserID(u32 ID)
{
	g_nUserId = ID;
}

char* GetGUILoginedUserName()
{
	return g_userName;
}

void SetGUILoginedUserName(char* name)
{
	memset(g_userName, 0, sizeof(g_userName));
	strcpy(g_userName, name);
	BizSetUserNameLog(name);//cw_log
}

u8 IsGUIUserLogined()
{
	return g_IsLogined;
}

void SetGUIUserIsLogined(u8 logined)
{
	g_IsLogined = logined;
}

//zlb20111117 去掉多余代码
/*
void InitStrChn()
{
	int maxChn = GetVideoMainNum();
	
	strChn = (char**)malloc(maxChn*sizeof(char*));
	for(int i=0; i<maxChn; i++)
	{
		strChn[i] = (char*)malloc(16);
	}

}

void InitChnEnable()
{
	int maxChn = GetVideoMainNum();

	chnNameEnabel = (int*)malloc(maxChn*sizeof(int));
}

void SetChnString(int chn,char* str)
{
	strcpy(strChn[chn],str);
}

void SetChnNameEnable(int chn,int enable)
{
	chnNameEnabel[chn] = enable;

}
*/
static SBizPreviewData  g_pBizPreviewData;

EMBIZPREVIEWMODE bizData_GetPreviewMode()
{
	return g_pBizPreviewData.emBizPreviewMode;
}
//yaogang modify 20141202
void bizData_GetPreviewPara(EMBIZPREVIEWMODE * PreviewMode, u8* ModePara)
{
	*PreviewMode = g_pBizPreviewData.emBizPreviewMode;
	*ModePara = g_pBizPreviewData.nModePara;
}

int EventPreviewFreshDeal(SBizPreviewData* pBizPreviewData)
{
	//printf("yg EventPreviewFreshDeal 1\n");//csp modify 20150110
	if(!pBizPreviewData)
	{
		printf("EventPreviewFreshDeal err-1\n");
		return -1;
	}
	
	if((!g_pPages) || (!g_pPages[EM_PAGE_DESKTOP]) || (!g_pPages[EM_PAGE_START]) || (!g_pPages[EM_PAGE_LIVECFG]))
	{
		printf("EventPreviewFreshDeal err-2\n");
		return -1;
	}
	
	memcpy(&g_pBizPreviewData, pBizPreviewData, sizeof(SBizPreviewData));
	
	CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
	CPageStartFrameWork* pPageStart = (CPageStartFrameWork *)g_pPages[EM_PAGE_START];
	CPageLiveConfigFrameWork* pPageLive = (CPageLiveConfigFrameWork *)g_pPages[EM_PAGE_LIVECFG];
	
	u8 nStartChn = 0;
	u8 nSplitNum = 1;
	u8 nMaxChn;
	u8 nXScale;
	u8 nYScale;
	int i;
	static int nScreanWidth = 720;
	static int nScreanHeight = 576;
	int nChnWidth;
	int nOffSet25 = 0;
	int nChnHeight;
	int x2, y2; //osd
	static int x3;
	char strChnName[16];
	int nOsdWidth = 0;
	int nOsdHeight = CHNNAME_HEIGHT;//TEXT_HEIGHT;
	
	//csp modify 20130501
	int large = 0;
	int small = 0;
	int index = 0;
	//add by liu
	int nStreamWidth = 0;
	static int preMaxNum = 0; //上一次的最大值
	static CRect tableRT[16];
	
	static char flag = 1;
	if(flag)
	{
		GetVgaResolution(&nScreanWidth, &nScreanHeight);
		flag = 0;
	}
	
	nMaxChn = GetVideoMainNum();
	
	nStartChn = pBizPreviewData->nModePara;
	
	switch(pBizPreviewData->emBizPreviewMode)
	{
		case EM_BIZPREVIEW_1SPLIT:
			nSplitNum = 1;
			nXScale = 1;
			nYScale = 1;
			break;
		case EM_BIZPREVIEW_4SPLITS:
			nSplitNum = 4;
			nXScale = 2;
			nYScale = 2;
			break;
		case EM_BIZPREVIEW_9SPLITS:
			nSplitNum = 9;
			nXScale = 3;
			nYScale = 3;
			break;
		case EM_BIZPREVIEW_13SPLITS:
			/*nSplitNum = 13;
			nXScale = 2;
			nYScale = 2;
			break;*/
		case EM_BIZPREVIEW_6SPLITS:
		case EM_BIZPREVIEW_10SPLITS:
		case EM_BIZPREVIEW_16SPLITS:
			nSplitNum = 16;
			nXScale = 4;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_24SPLITS:
			nSplitNum = 24;
			nXScale = 6;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_25SPLITS:
			nOffSet25 = 8;
			nSplitNum = 25;
			nXScale = 5;
			nYScale = 5;
			break;
		case EM_BIZPREVIEW_36SPLITS:
			nSplitNum = 36;
			nXScale = nYScale = 6;
			break;
		//csp modify 20130501
		case EM_BIZPREVIEW_PIP:
			nSplitNum = nMaxChn;
			nXScale = 2;
			nYScale = 2;
			large = (nStartChn >> 4) & 0xf;
			small = (nStartChn >> 0) & 0xf;
			//nStartChn = min(large,small);
			//return 0;
			break;
		case EM_BIZPREVIEW_CLOSE:
			printf("%s EM_BIZPREVIEW_CLOSE\n", __func__);
			return 0;
		default:
			printf("EventPreviewFreshDeal err-3, PreviewMode=%d\n", pBizPreviewData->emBizPreviewMode);
			return -1;
	}
	
	pPageStart->SetCurPreview(nStartChn);
	pPageStart->SetCurPreviewMode(pBizPreviewData->emBizPreviewMode);
	pPageLive->SetCurPreviewMode(pBizPreviewData->emBizPreviewMode);
	
	//SetCurPreviewMode_CW(pBizPreviewData->emBizPreviewMode);
	
	if(nOffSet25 > 0)
	{
		SBizSystemPara sysPara;
		BizGetSysInfo(&sysPara);
		
		if(EM_BIZ_PAL == sysPara.nVideoStandard)
		{
			//nChnHeight = nScreanHeight * (576 - nOffSet25 * 2) / nYScale / 576;
			nOffSet25 = nScreanHeight * nOffSet25 / 576;
			nChnHeight = (nScreanHeight - nOffSet25 * 2) / nYScale;
		}
		else
		{
			nOffSet25 = 0;
			nChnHeight = nScreanHeight / nYScale;
		}
	}
	else
	{
		nChnHeight = nScreanHeight / nYScale; //通道高度
	}
	
	nChnWidth = nScreanWidth / nXScale;//通道宽度
	
	for (i = 0; i < nMaxChn; i++)
	{
		pDeskTop->ShowChnName(i, FALSE);
	}
	//yaogang modify 20141109
	//printf("yg EventPreviewFreshDeal ShowChnKbps FALSE\n");
	for (i = 0; i < nMaxChn; i++)
	{
		pDeskTop->ShowChnKbps(i, FALSE);
		pDeskTop->ShowChnElseState(i, EM_STATE_SNAP, 0, 0, EM_CHNSTATE_HIDE);

		pDeskTop->ShowSplitLine(i, FALSE);
	}
	
	for (i = 0; i < nXScale - 1; i++) //nXScale:x方向通道个数
	{
		if((pBizPreviewData->emBizPreviewMode == EM_BIZPREVIEW_10SPLITS) && (nXScale == 4) && (i != 1))
		{
			pDeskTop->SetSplitLineRect(i, nChnWidth * (i + 1), nScreanHeight/2, nScreanHeight/2, EM_SPLIT_V);
		}
		else if((pBizPreviewData->emBizPreviewMode == EM_BIZPREVIEW_6SPLITS) && (nXScale == 4) && (i != 1))
		{
			pDeskTop->SetSplitLineRect(i, 0, 0, 0, EM_SPLIT_V);
		}
		else if(pBizPreviewData->emBizPreviewMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			pDeskTop->SetSplitLineRect(i, nChnWidth * (i + 1), nScreanHeight/2, nScreanHeight/2, EM_SPLIT_V);
		}
		else
		{
			//画竖线
			pDeskTop->SetSplitLineRect(i, nChnWidth * (i + 1), 0,  nScreanHeight, EM_SPLIT_V);
		}
		pDeskTop->ShowSplitLine(i, TRUE);
	}
	
	for (i = 0; i < nYScale - 1; i++)
	{
		if((pBizPreviewData->emBizPreviewMode == EM_BIZPREVIEW_10SPLITS) && (nYScale == 4) && (i == 0))
		{
			pDeskTop->SetSplitLineRect(i + nXScale - 1, 0, 0, 0, EM_SPLIT_H);
		}
		else if((pBizPreviewData->emBizPreviewMode == EM_BIZPREVIEW_6SPLITS) && (nYScale == 4) && (i == 0))
		{
			pDeskTop->SetSplitLineRect(i + nXScale - 1, 0, 0, 0, EM_SPLIT_H);
		}
		else if(pBizPreviewData->emBizPreviewMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			pDeskTop->SetSplitLineRect(i + nXScale - 1, nScreanWidth/2, nOffSet25 + nChnHeight * (i + 1),  nScreanWidth/2, EM_SPLIT_H);
		}
		else
		{
			//画横线
			pDeskTop->SetSplitLineRect(i + nXScale - 1, 0, nOffSet25 + nChnHeight * (i + 1),  nScreanWidth, EM_SPLIT_H);
		}
		pDeskTop->ShowSplitLine(i + nXScale - 1, TRUE);
	}
	
	if(pBizPreviewData->emBizPreviewMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
	{
		nStartChn = 0;//min(large,small);
	}
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	BizGetPara(&bizTar, &bizPreCfg);
	//yaogang modify 20140918
	//printf("yg EventPreviewFreshDeal nShowTime: %d\n", bizPreCfg.nShowTime);
	
	pDeskTop->ClearAllChnState();
	
	nMaxChn = (nMaxChn < (nStartChn + nSplitNum)) ? nMaxChn : (nStartChn + nSplitNum);

	//add by liu
	//printf("yg EventPreviewFreshDeal \n");
	//printf("yg ShowDeskStreamInfo chns: %d\n", nMaxChn);
	if(preMaxNum != nMaxChn)//清屏
	{
		for(i = 0; i < preMaxNum; i++)
		//for(i = nStartChn; i < preMaxNum; i++)
		{
			//yaogang modify 20141107
			//pDeskTop->ShowChnElseState( i, EM_STATE_LINK, x3-20*(3-EM_STATE_LINK), y2, EM_CHNSTATE_HIDE );
			//pDeskTop->ShowChnElseState( i, EM_STATE_SOUND, x3-20*(3-EM_STATE_SOUND), y2, EM_CHNSTATE_HIDE );
			//pDeskTop->ShowChnElseState( i, EM_STATE_MKPHONE, x3-20*(3-EM_STATE_MKPHONE), y2, EM_CHNSTATE_HIDE );
			pDeskTop->SetChnKbps(i, tableRT[i]," ");						
			//pDeskTop->ShowChnKbps(i, bizPreCfg.nShowChnKbps);
		}
		preMaxNum = nMaxChn;
	}
	
	for(i = nStartChn; i < nMaxChn; i++)
	{
		if(pBizPreviewData->emBizPreviewMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(i == nStartChn)
			{
				i += 4;
			}
			else if(i == nStartChn+1)
			{
				i += 5;
			}
			else
			{
				i += 6;
			}
		}
		else if(pBizPreviewData->emBizPreviewMode == EM_BIZPREVIEW_6SPLITS)
		{
			if(i == nStartChn)
			{
				i += 4;
			}
			else if(i == nStartChn+1)
			{
				i += 5;
			}
			else
			{
				i += (i+4);
			}
		}
		else if(pBizPreviewData->emBizPreviewMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			if(i == large)
			{
				index = i;
				i = 2;
			}
			else if(i == small)
			{
				index = i;
				i = 3;
			}
			else
			{
				continue;
			}
		}

		//x2是文本的左边界
		x2 = PIC_STATE_OFFSET_X + (( i - nStartChn) % nXScale) * nChnWidth;
		//y2是文本的上边界
		y2 = (nChnHeight - (576-PIC_STATE_OFFSET_Y)) + (( i - nStartChn) / nXScale) * nChnHeight;
		
		if(nOffSet25)
		{
			y2 += nOffSet25;
			
			if (i >= 20 && i <= 24 && 0 == nStartChn) 
			{
				y2 += nOffSet25;
			}
			else if (i >= 27 && i <= 31 && nStartChn > 0)
			{
				y2 += nOffSet25;
			}
		}
		
		if(nScreanWidth == 720)
		{
			if(0 == (i-nStartChn)%nXScale)//第一列通道
			{
				//x2 = nChnWidth - PIC_STATE_OFFSET_X - (nOsdWidth>80?nOsdWidth:80);
				x2 += 26;
			}
			
			if((( i - nStartChn) / nXScale + 1) == nYScale)//最后一行通道
			{
				y2 = (( i - nStartChn) / nXScale) * nChnHeight + (576-PIC_STATE_OFFSET_Y-CHNNAME_HEIGHT-16 -2) + nOffSet25;
			}
			
			if(1 == nXScale)
			{
				x2 = 100;
				y2 = nChnHeight-100;
			}
		}
		
		if(pBizPreviewData->emBizPreviewMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(i == nStartChn+4)
			{
				i -= 4;
			}
			else if(i == nStartChn+1+5)
			{
				i -= 5;
			}
			else
			{
				i -= 6;
			}
		}
		else if(pBizPreviewData->emBizPreviewMode == EM_BIZPREVIEW_6SPLITS)
		{
			i = (i-4)/2;
		}
		else if(pBizPreviewData->emBizPreviewMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			i = index;
		}
		
		char chName[32] = {0};
		GetChnName(i, chName, sizeof(chName));
		//nOsdWidth是文本的宽度
		nOsdWidth = strlen(chName) ? ((strlen(chName)+3)*TEXT_WIDTH/3) : 0;
		
		for(int a = 0; a < strlen(chName); a++)
		{
			if(chName[a] >= 128)//汉字
			{
				a+=2;
				nOsdWidth-=8;
			}
		}
		
		int bShowChName = GetDisplayNameCheck(i);
		
		if(bShowChName)
		{
			pDeskTop->SetChnName(i, CRect(x2, y2, x2 + nOsdWidth, y2 + nOsdHeight), strlen(chName)?chName:" ");
			pDeskTop->ShowChnName(i, TRUE);
		}

		/*add by liu*/
		char chKbps[10];
		int StreamKbps = Get_StreamKbps(i)+Get_StreamKbps(i+16);
		sprintf(chKbps,"%dKbps",StreamKbps);
		
		x3 = (( i - nStartChn) % nXScale + 1) * nChnWidth;
		nStreamWidth = strlen(chKbps) ? (strlen(chKbps)*TEXT_WIDTH/3) : 0;
		
		if(bShowChName)
		{	
			if(0 == (i+1-nStartChn)%nXScale) //最后一列通道
			{
				tableRT[i].left = x3-23-nStreamWidth;
				tableRT[i].top = y2;
				tableRT[i].right = x3-23;
				tableRT[i].bottom = y2 + nOsdHeight;
			}
			else
			{
				tableRT[i].left = x3-20-nStreamWidth;
				tableRT[i].top = y2;
				tableRT[i].right = x3-20;
				tableRT[i].bottom = y2 + nOsdHeight;
			}
			//printf("yg set & show Chn%d Kbps\n", i);
			pDeskTop->SetChnKbps(i, tableRT[i], chKbps);		
			pDeskTop->ShowChnKbps(i, bizPreCfg.nShowChnKbps);
		}
		
		y2 += CHNNAME_HEIGHT + 2;

		if (IPC_GetLinkStatus(i) == 0)
		{
			//抓图图标处理
			if (pDeskTop->ChnElseStateIsShow(i, EM_STATE_SNAP))
			{
				pDeskTop->ShowChnElseState(i, EM_STATE_SNAP, 0, 0, EM_CHNSTATE_HIDE);
			}
		}
		else
		{
			pDeskTop->ShowChnElseState( i, EM_STATE_SNAP, x3-20*(3-EM_STATE_SNAP), y2, EM_CHNSTATE_1);
		}
/*
		//联接状态
		if(StreamKbps != 0)
			pDeskTop->ShowChnElseState( i, EM_STATE_LINK, x3-20*(3-EM_STATE_LINK), y2, EM_CHNSTATE_1 );
		else
			pDeskTop->ShowChnElseState( i, EM_STATE_LINK, x3-20*(3-EM_STATE_LINK), y2, EM_CHNSTATE_2 );
*/			
		/*
		//喇叭状态
		if(bizPreCfg.nMute == 1) //静音
			pDeskTop->ShowChnElseState( i, EM_STATE_SOUND, x3-20*(3-EM_STATE_SOUND), y2, EM_CHNSTATE_2 );
		else //非静音
		{
			if(bizPreCfg.nVolume > 0)
				pDeskTop->ShowChnElseState( i, EM_STATE_SOUND, x3-20*(3-EM_STATE_SOUND), y2, EM_CHNSTATE_1 );
			else
				pDeskTop->ShowChnElseState( i, EM_STATE_SOUND, x3-20*(3-EM_STATE_SOUND), y2, EM_CHNSTATE_2 );
		}
		*/
		//麦克风状态
		//pDeskTop->ShowChnElseState( i, EM_STATE_MKPHONE, x3-20*(3-EM_STATE_MKPHONE), y2, EM_CHNSTATE_1 );
			
		//现场配置->设置"显示录像状态"时bizPreCfg.nShowRecState = 1
		//printf("bizPreCfg.nShowRecState = %d\n",bizPreCfg.nShowRecState);
		
		if(bizPreCfg.nShowRecState)
		{
			if(bRecord[i] == 1) //是否在录像
			{
				pDeskTop->ShowChnState( i, EM_STATE_RECORD, x2, y2, EM_CHNSTATE_1 );
				if(GetCurRecStatus(i) & 1)//是否手工录像
				{
					pDeskTop->ShowChnState( i, EM_STATE_MANUAL_REC, x2, y2, EM_CHNSTATE_1 );
				}
				else
				{
					pDeskTop->ShowChnState( i, EM_STATE_MANUAL_REC, x2, y2, EM_CHNSTATE_HIDE );
				}
				if(GetCurRecStatus(i) & 2)//是否定时录像
				{
					pDeskTop->ShowChnState( i, EM_STATE_TIMER, x2, y2, EM_CHNSTATE_1 );
				}
				else
				{
					pDeskTop->ShowChnState( i, EM_STATE_TIMER, x2, y2, EM_CHNSTATE_HIDE );
				}
			}
			else
			{
				pDeskTop->ShowChnState( i, EM_STATE_RECORD, x2, y2, EM_CHNSTATE_HIDE);
				pDeskTop->ShowChnState( i, EM_STATE_MANUAL_REC, x2, y2, EM_CHNSTATE_HIDE);
				pDeskTop->ShowChnState( i, EM_STATE_TIMER, x2, y2, EM_CHNSTATE_HIDE);
			}
		}
		else //不显示录像状态
		{
			if(bRecord[i] == 1)//是否在录像
			{
				pDeskTop->ShowChnState( i, EM_STATE_RECORD, x2, y2, EM_CHNSTATE_1 );
			}
			else
			{
				pDeskTop->ShowChnState( i, EM_STATE_RECORD, x2, y2, EM_CHNSTATE_HIDE);
			}
		}
		
		if(bMD[i] == 1)//是否移动侦探
		{
			pDeskTop->ShowChnState( i, EM_STATE_MD, x2, y2, EM_CHNSTATE_1 );
		}
		else
		{
			pDeskTop->ShowChnState( i, EM_STATE_MD, x2, y2, EM_CHNSTATE_HIDE );
		}

		if(bSensorAlarm[i] == 1) //是否报警录像
		{
			pDeskTop->ShowChnState( i, EM_STATE_ALARM, x2, y2, EM_CHNSTATE_1 );
		}
		else
		{
			pDeskTop->ShowChnState( i, EM_STATE_ALARM, x2, y2, EM_CHNSTATE_HIDE );
		}
	}
	return 0;
}


void CPageDesktop::ShowRegionALl(int hua)
{
	return;
	static int nScreanWidth = 720;
	static int nScreanHeight = 576;

	u8 nStartChn = 0;
	static u8 nSplitNum = 0;
	u8 nMaxChn;
	u8 nXScale;
	u8 nYScale;
	int i;
	static int LastNum;
	
	LastNum = nSplitNum;
	nSplitNum = hua;

	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	BizGetPara(&bizTar, &bizPreCfg);
	
	
	GetVgaResolution(&nScreanWidth, &nScreanHeight);
	
	nMaxChn = GetVideoMainNum();
	
	nStartChn = g_pBizPreviewData.nModePara;
#if 0
	switch(g_pBizPreviewData.emBizPreviewMode)
	{
		case EM_BIZPREVIEW_1SPLIT:
			nSplitNum = 1;
			nXScale = 1;
			nYScale = 1;
			break;
		case EM_BIZPREVIEW_4SPLITS:
			nSplitNum = 4;
			nXScale = 2;
			nYScale = 2;
			break;
		case EM_BIZPREVIEW_9SPLITS:
			nSplitNum = 9;
			nXScale = 3;
			nYScale = 3;
			break;
		case EM_BIZPREVIEW_16SPLITS:
			nSplitNum = 16;
			nXScale = 4;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_CLOSE:
			
			break;
		default:
			printf("EventPreviewFreshDeal err-3, PreviewMode=%d\n", g_pBizPreviewData.emBizPreviewMode);
			
			break;
	}
#endif
	switch(nSplitNum) {
		case 1:
			nXScale = 1;
			nYScale = 1;
			break;
		case 4:
			nXScale = 2;
			nYScale = 2;
			break;
		case 9:
			nXScale = 3;
			nYScale = 3;
			break;
		case 16:
			nXScale = 4;
			nYScale = 4;
			break;
		default:
			printf("fuck\n");
			break;
	}

	int nnChnHeight,nnChnWidth;

	printf("screen %d %d \n",nScreanWidth,nScreanHeight);
	
	
	nnChnHeight = nScreanHeight / nYScale; //通道高度
	nnChnWidth = nScreanWidth / nXScale; //通道宽度

	printf("small screen %d %d\n",nnChnWidth,nnChnHeight);

	nMaxChn = (nMaxChn < (nStartChn + nSplitNum)) ? nMaxChn : (nStartChn + nSplitNum);
	printf("this is %d to %d  vo\n",LastNum,nSplitNum);

	//先把上次的显示去掉
	for(i = 0; i < LastNum; i++) {

		//DeleteRegion(i);
		//ShowRegion(i,TRUE);
		//ShowRegion(i, FALSE);
	}

	CRect RegionCRect;
	
	for(i = 0; i < nSplitNum; i++)
	{
		RegionCRect.left = (nChnRegion[i].x)/nXScale + nnChnWidth*(i%nXScale);
		RegionCRect.top = (nChnRegion[i].y)/nYScale + nnChnHeight*(i/nYScale);
		RegionCRect.right = (nChnRegion[i].x2)/nXScale + nnChnWidth*(i%nXScale);
		RegionCRect.bottom = (nChnRegion[i].y2)/nYScale + nnChnHeight*(i/nYScale);
		printf("%d %d %d %d\n",RegionCRect.left,RegionCRect.top,RegionCRect.right,RegionCRect.bottom);
		SetRegion(i, &RegionCRect,"");
		//ShowRegion(i, TRUE);
	}

}



int CPageDesktop::ShowDeskStreamInfo(uint param)
{
	u8 nStartChn = 0;
	u8 nSplitNum = 1;
	u8 nMainChn;
	u8 nMaxChn;
	u8 nXScale;
	u8 nYScale;
	int i;
	static int nScreanWidth = 720;
	static int nScreanHeight = 576;
	int nChnWidth;
	int nOffSet25 = 0;
	int nChnHeight;
	int x2, y2; //osd
	int x3;
	char strChnName[16];
	int nOsdWidth = 0;
	int nOsdHeight = CHNNAME_HEIGHT;//TEXT_HEIGHT;
	
	int nStreamWidth = 0;
	//int bShowChName = 0;
	//int bShowChnKbps = 0;
	static char flag = 1;

	static int preMaxChn = 0;
	static CRect tableRT[16];
	static int preStreamKbps[16];

//yaogang modify 20130913
	//printf("ShowDeskStreamInfo\n");
	

	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	BizGetPara(&bizTar, &bizPreCfg);
	//yaogang modify 20140918
	//printf("yg ShowDeskStreamInfo nShowTime: %d, nShowChnKbps: %d\n", bizPreCfg.nShowTime, bizPreCfg.nShowChnKbps);
	
	if(flag)
	{
		GetVgaResolution(&nScreanWidth, &nScreanHeight);
		flag = 0;
	}
	
	nMainChn = nMaxChn = GetVideoMainNum();
	
	nStartChn = g_pBizPreviewData.nModePara;

	switch(g_pBizPreviewData.emBizPreviewMode)
	{
		case EM_BIZPREVIEW_1SPLIT:
			nSplitNum = 1;
			nXScale = 1;
			nYScale = 1;
			break;
		case EM_BIZPREVIEW_4SPLITS:
			nSplitNum = 4;
			nXScale = 2;
			nYScale = 2;
			break;
		case EM_BIZPREVIEW_9SPLITS:
			nSplitNum = 9;
			nXScale = 3;
			nYScale = 3;
			break;
		case EM_BIZPREVIEW_16SPLITS:
			nSplitNum = 16;
			nXScale = 4;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_CLOSE:
			return 0;
		default:
			printf("EventPreviewFreshDeal err-3, PreviewMode=%d\n", g_pBizPreviewData.emBizPreviewMode);
			return -1;
	}

	nChnHeight = nScreanHeight / nYScale; //通道高度
	nChnWidth = nScreanWidth / nXScale; //通道宽度

	nMaxChn = (nMaxChn < (nStartChn + nSplitNum)) ? nMaxChn : (nStartChn + nSplitNum);

	//处理抓图图标,不预览的通道--隐藏图标
	for(i = 0; i < nStartChn; i++)
	{
		if (ChnElseStateIsShow(i, EM_STATE_SNAP))
		{
			ShowChnElseState(i, EM_STATE_SNAP, 0, 0, EM_CHNSTATE_HIDE);
		}
	}
	for(i = nMaxChn; i < nMainChn; i++)
	{
		if (ChnElseStateIsShow(i, EM_STATE_SNAP))
		{
			ShowChnElseState(i, EM_STATE_SNAP, 0, 0, EM_CHNSTATE_HIDE);
		}
	}
#if 0
	if(preMaxChn != nMaxChn) //码流信息清屏
	{
		for(i = 0; i < preMaxChn; i++)
		{
			SetChnKbps(i, tableRT[i]," ");						
			//ShowChnKbps(i, bizPreCfg.nShowChnKbps);
		}
			
		preMaxChn = nMaxChn;
	}
#endif
	//CRect RegionCRect;
	//printf("yg ShowDeskStreamInfo chns: %d\n", nMaxChn);


//yaogang modify 20141119 码流显示残像，在双击第16通道时
	//for(i = 0; i < nMaxChn; i++)
	for(i = nStartChn; i < nMaxChn; i++)
	{
		
		x3 = (( i - nStartChn) % nXScale + 1) * nChnWidth;
		//y2是文本的上边界
		y2 = (nChnHeight - (576-PIC_STATE_OFFSET_Y)) + (( i - nStartChn) / nXScale) * nChnHeight;

		char chName[32] = {0};
		char Kbpsbuffer[10] = {0};
		int StreamKbps;

		//yaogang modify 判断通道断开
		if (IPC_GetLinkStatus(i) == 0)
		{
			StreamKbps = 0;

			//抓图图标处理
			if (ChnElseStateIsShow(i, EM_STATE_SNAP))
			{
				ShowChnElseState(i, EM_STATE_SNAP, 0, 0, EM_CHNSTATE_HIDE);
			}
		}
		else
		{
			if (m_FlagPlaying)//电子放大时隐藏
			{
				if (ChnElseStateIsShow(i, EM_STATE_SNAP))
				{
					ShowChnElseState(i, EM_STATE_SNAP, 0, 0, EM_CHNSTATE_HIDE);
				}
			}
			else
			{
				ShowChnElseState( i, EM_STATE_SNAP, x3-20*(3-EM_STATE_SNAP), y2+CHNNAME_HEIGHT + 2, EM_CHNSTATE_1);
			}
			StreamKbps = Get_StreamKbps(i)+Get_StreamKbps(i+16);
		}
		
		sprintf(Kbpsbuffer," %dKbps",StreamKbps);	
		nStreamWidth = strlen(Kbpsbuffer) ? (strlen(Kbpsbuffer)*TEXT_WIDTH/3) : 0;

		//yaogang modify 20140913
		//GetChnName(i, chName, sizeof(chName));		
		//bShowChName = GetDisplayNameCheck(i);
		//bShowChnKbps = GetChnKbpsDisplayCheck();
		//yaogang modify 20140913
		//if (i==0)
			//printf("ShowDeskStreamInfo bShowChName: %d\n", bShowChName);
			
		//if(bShowChName)
		{
			if(0 == (i+1-nStartChn)%nXScale) //最后一列通道
			{
				tableRT[i].left = x3-23-nStreamWidth;
				tableRT[i].top = y2;
				tableRT[i].right = x3-23;
				tableRT[i].bottom = y2 + nOsdHeight;
			}
			else
			{
				tableRT[i].left = x3-20-nStreamWidth;
				tableRT[i].top = y2;
				tableRT[i].right = x3-20;
				tableRT[i].bottom = y2 + nOsdHeight;
			}
			//yaogang modify 20140918
			//SetChnKbps(i, tableRT[i]," ");						
			//ShowChnKbps(i, bShowChnKbps);
			//SetChnKbps(i, tableRT[i], strlen(Kbpsbuffer)?Kbpsbuffer:" ");
			
			if (preStreamKbps[i] != StreamKbps)
			{
				SetChnKbps(i, tableRT[i], strlen(Kbpsbuffer)?Kbpsbuffer:" ");
				preStreamKbps[i] = StreamKbps;
			}
			//if (i == 0)
				//printf("%s m_FlagPlaying: %d\n", __func__, m_FlagPlaying);
			
			ShowChnKbps(i, bizPreCfg.nShowChnKbps && !m_FlagPlaying);
			
			
		}
		//yaogang modify 20141107
		//ShowChnElseState( i, EM_STATE_LINK, x3-20*(3-EM_STATE_LINK), y2+CHNNAME_HEIGHT + 2, EM_CHNSTATE_HIDE);//不能少
		//ShowChnElseState( i, EM_STATE_SOUND, x3-20*(3-EM_STATE_SOUND), y2+CHNNAME_HEIGHT + 2, EM_CHNSTATE_HIDE);//不能少
		//ShowChnElseState( i, EM_STATE_SOUND, x3-20*(3-EM_STATE_SOUND), y2+CHNNAME_HEIGHT + 2, EM_CHNSTATE_HIDE);//不能少
/*
		//码流信息
		if(StreamKbps != 0)
			ShowChnElseState( i, EM_STATE_LINK, x3-20*(3-EM_STATE_LINK), y2+CHNNAME_HEIGHT + 2, EM_CHNSTATE_1 );
		else
			ShowChnElseState( i, EM_STATE_LINK, x3-20*(3-EM_STATE_LINK), y2+CHNNAME_HEIGHT + 2, EM_CHNSTATE_2 );
*/
		/*
		//喇叭状态
		if(bizPreCfg.nMute == 1) //静音
			{
				ShowChnElseState( i, EM_STATE_SOUND, x3-20*(3-EM_STATE_SOUND), y2+CHNNAME_HEIGHT + 2, EM_CHNSTATE_2 );
			}
		else //非静音
		{
			if(bizPreCfg.nVolume > 0)			
				ShowChnElseState( i, EM_STATE_SOUND, x3-20*(3-EM_STATE_SOUND), y2+CHNNAME_HEIGHT + 2, EM_CHNSTATE_1 );
			else
				ShowChnElseState( i, EM_STATE_SOUND, x3-20*(3-EM_STATE_SOUND), y2+CHNNAME_HEIGHT + 2, EM_CHNSTATE_2 );
		}
		*/
		
	}

}


int EventShowTimeFresh()
{
	if((!g_pPages) || (!g_pPages[EM_PAGE_DESKTOP]))
	{
		return -1;
	}
	
	CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
	
	int bShowTime = GetTimeDisplayCheck();
	
	pDeskTop->ShowTimeTitle(0, bShowTime);
	
	return 0;
}

int EventLiveFreshDeal(int ch)
{
	if((!g_pPages) || (!g_pPages[EM_PAGE_DESKTOP]) || (!g_pPages[EM_PAGE_START]) || (!g_pPages[EM_PAGE_LIVECFG]))
	{
		return -1;
	}
	
	//printf("EventLiveFreshDeal start...\n");
	
	CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
	CPageStartFrameWork* pPageStart = (CPageStartFrameWork *)g_pPages[EM_PAGE_START];
	CPageLiveConfigFrameWork* pPageLive = (CPageLiveConfigFrameWork *)g_pPages[EM_PAGE_LIVECFG];
	
	u8 nStartChn = 0;
	u8 nSplitNum = 1;
	u8 nMaxChn;
	u8 nXScale;
	u8 nYScale;
	static int nScreanWidth = 720;
	static int nScreanHeight = 576;
	int nChnWidth;
	int nChnHeight;
	int nOffSet25 = 0;
	int x2, y2; //osd
	char strChnName[16];
	int nOsdWidth = 0; //文本的宽度
	int nOsdHeight = CHNNAME_HEIGHT;//TEXT_HEIGHT;文本的高度
	
	//csp modify 20130501
	int large = 0;
	int small = 0;
	int index = 0;
	
	static char flag = 1;
	if(flag)
	{
		GetVgaResolution(&nScreanWidth, &nScreanHeight);
		flag = 0;
	}
	
	nMaxChn = GetVideoMainNum();
	
	nStartChn = g_pBizPreviewData.nModePara;
	
	switch(g_pBizPreviewData.emBizPreviewMode)//显示模式
	{
		case EM_BIZPREVIEW_1SPLIT:
			nSplitNum = 1;
			nXScale = 1;
			nYScale = 1;
			break;
		case EM_BIZPREVIEW_4SPLITS://2行2列
			nSplitNum = 4;
			nXScale = 2;
			nYScale = 2;
			break;
		case EM_BIZPREVIEW_9SPLITS://3行3列
			nSplitNum = 9;
			nXScale = 3;
			nYScale = 3;
			break;
		case EM_BIZPREVIEW_13SPLITS:
			/*nSplitNum = 13;
			nXScale = 2;
			nYScale = 2;
			break;*/
		case EM_BIZPREVIEW_6SPLITS:
		case EM_BIZPREVIEW_10SPLITS:
		case EM_BIZPREVIEW_16SPLITS://4行4列
			nSplitNum = 16;
			nXScale = 4;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_24SPLITS:
			nSplitNum = 24;
			nXScale = 6;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_25SPLITS:
			nOffSet25 = 8;
			nSplitNum = 25;
			nXScale = 5;
			nYScale = 5;
			break;
		case EM_BIZPREVIEW_36SPLITS:
			nSplitNum = 36;
			nXScale = nYScale = 6;
			break;
		//csp modify 20130501
		case EM_BIZPREVIEW_PIP:
			nSplitNum = nMaxChn;
			nXScale = 2;
			nYScale = 2;
			large = (nStartChn >> 4) & 0xf;
			small = (nStartChn >> 0) & 0xf;
			//nStartChn = min(large,small);
			//return 0;
			break;
		default:
			printf("EventLiveFreshDeal error:PreviewMode=%d\n",g_pBizPreviewData.emBizPreviewMode);
			return -1;
	}
	//printf("nStartChn = %d, emBizPreviewMode = %d, nOffSet25 = %d\n", nStartChn, g_pBizPreviewData.emBizPreviewMode, nOffSet25);
	
	pPageStart->SetCurPreview(nStartChn);
	pPageStart->SetCurPreviewMode(g_pBizPreviewData.emBizPreviewMode);
	pPageLive->SetCurPreviewMode(g_pBizPreviewData.emBizPreviewMode);
	
	if(nOffSet25 > 0)
	{
		SBizSystemPara sysPara;
		BizGetSysInfo(&sysPara);
		
		if(EM_BIZ_PAL == sysPara.nVideoStandard)
		{
			//nChnHeight = nScreanHeight * (576 - nOffSet25 * 2) / nYScale / 576;
			nOffSet25 = nScreanHeight * nOffSet25 / 576;
			nChnHeight = (nScreanHeight - nOffSet25 * 2) / nYScale;			
		}
		else
		{
			nOffSet25 = 0;
			nChnHeight = nScreanHeight / nYScale;
		}
	}
	else
	{
		nChnHeight = nScreanHeight / nYScale;//一个通道的高度
	}
	
	nChnWidth = nScreanWidth / nXScale;//一个通道的宽度
	
	if(g_pBizPreviewData.emBizPreviewMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
	{
		nStartChn = 0;//min(large,small);
	}
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	
	SBizCfgPreview bizPreCfg;
	BizGetPara(&bizTar, &bizPreCfg);
	
	nMaxChn = (nMaxChn < (nStartChn + nSplitNum)) ? nMaxChn : (nStartChn + nSplitNum);
	
	if(nStartChn <= ch && ch < nMaxChn)
	{
		pDeskTop->ShowChnName(ch, FALSE);
		
		if(g_pBizPreviewData.emBizPreviewMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(0 == ch)
			{
				ch = 4;
			}
			else if(1 == ch)
			{
				ch = 6;
			}
			else
			{
				ch += 6;
			}
		}
		else if(g_pBizPreviewData.emBizPreviewMode == EM_BIZPREVIEW_6SPLITS)
		{
			if(0 == ch)
			{
				ch = 4;
			}
			else if(1 == ch)
			{
				ch = 6;
			}
			else
			{
				ch += (ch+4);
			}
		}
		else if(g_pBizPreviewData.emBizPreviewMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			if(ch == large)
			{
				index = ch;
				ch = 2;
			}
			else if(ch == small)
			{
				index = ch;
				ch = 3;
			}
			else
			{
				return 0;
			}
		}
		//nXScale:x方向的通道个数
		//nChnWidth:一个通道的宽度
		//nChnHeight:一个通道的高度
		//x2是文本的左边界
		x2 = PIC_STATE_OFFSET_X + (( ch - nStartChn) % nXScale) * nChnWidth;
		//y2是文本的上边界
		y2 = (nChnHeight - (576-PIC_STATE_OFFSET_Y)) + (( ch - nStartChn) / nXScale) * nChnHeight;
		
		if(nOffSet25)
		{
			y2 += nOffSet25;
			
			if (ch >= 20 && ch <= 24 && 0 == nStartChn) 
			{
				y2 += nOffSet25;
			}
			else if (ch >= 27 && ch <= 31 && nStartChn > 0)
			{
				y2 += nOffSet25;
			}
		}
		//printf("chn[%d], y2=%d\n", ch, y2);
		
		if(nScreanWidth == 720)
		{
			if(0 == (ch-nStartChn)%nXScale)//x方向的第一个
			{
				//x2 = nChnWidth - PIC_STATE_OFFSET_X - (nOsdWidth>80?nOsdWidth:80);
				x2 += 26;
			}
			if((( ch - nStartChn) / nXScale + 1) == nYScale)//最后一行的通道   
			{
				y2 = (( ch - nStartChn) / nXScale) * nChnHeight + (576-PIC_STATE_OFFSET_Y-CHNNAME_HEIGHT-16-2) + nOffSet25;
			}
			if(1 == nXScale)//只显示一个通道
			{
				x2 = 100;
				y2 = nChnHeight-100;
			}
		}
		
		if(g_pBizPreviewData.emBizPreviewMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(4 == ch)
			{
				ch = 0;
			}
			else if(6 == ch)
			{
				ch = 1;
			}
			else
			{
				ch -= 6;
			}
		}
		else if(g_pBizPreviewData.emBizPreviewMode == EM_BIZPREVIEW_6SPLITS)
		{
			ch = (ch-4)/2;
		}
		else if(g_pBizPreviewData.emBizPreviewMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			ch = index;
		}
		
		char chName[32] = {0};
		GetChnName(ch, chName, sizeof(chName));
		//文本宽度设置
		nOsdWidth = strlen(chName) ? ((strlen(chName)+1)*TEXT_CHNWIDTH/3) : 0;
		
		for(int a = 0; a < strlen(chName); a++)
		{
			if(chName[a] >= 128)//汉字
			{
				a+=2;
				nOsdWidth-=8;
			}
		}
		
		int bShowChName = GetDisplayNameCheck(ch);
		
		//printf("EventLiveFreshDeal chn=%d bShowChName=%d chName=%s\n",ch,bShowChName,chName);
		
		if(bShowChName)
		{
			//显示"CH"
			pDeskTop->SetChnName(ch, CRect(x2, y2, x2 + nOsdWidth, y2 + nOsdHeight), strlen(chName)?chName:" ");
			pDeskTop->ShowChnName(ch, TRUE);
		}
		
		y2+=nOsdHeight+2;
		//add by liu 
		//printf("bizPreCfg.nShowRecState: %d\n",bizPreCfg.nShowRecState);
		
		if(bizPreCfg.nShowRecState)
		{
			if(bRecord[ch] == 1)
			{
				pDeskTop->ShowChnState( ch, EM_STATE_RECORD, x2, y2, EM_CHNSTATE_1 );
				if(GetCurRecStatus(ch) & 1)
				{
					pDeskTop->ShowChnState( ch, EM_STATE_MANUAL_REC, x2, y2, EM_CHNSTATE_1 );
				}
				else
				{
					pDeskTop->ShowChnState( ch, EM_STATE_MANUAL_REC, x2, y2, EM_CHNSTATE_HIDE );
				}
				if(GetCurRecStatus(ch) & 2)
				{
					pDeskTop->ShowChnState( ch, EM_STATE_TIMER, x2, y2, EM_CHNSTATE_1 );
				}
				else
				{
					pDeskTop->ShowChnState( ch, EM_STATE_TIMER, x2, y2, EM_CHNSTATE_HIDE );
				}
			}
			else
			{
				pDeskTop->ShowChnState( ch, EM_STATE_RECORD, x2, y2, EM_CHNSTATE_HIDE);
				pDeskTop->ShowChnState( ch, EM_STATE_MANUAL_REC, x2, y2, EM_CHNSTATE_HIDE);
				pDeskTop->ShowChnState( ch, EM_STATE_TIMER, x2, y2, EM_CHNSTATE_HIDE);
			}
		}
		else
		{	
			if(bRecord[ch] == 1)
			{
				pDeskTop->ShowChnState( ch, EM_STATE_RECORD, x2, y2, EM_CHNSTATE_1 );
			}
			else
			{
				pDeskTop->ShowChnState( ch, EM_STATE_RECORD, x2, y2, EM_CHNSTATE_HIDE);
			}
			
			pDeskTop->ShowChnState( ch, EM_STATE_MANUAL_REC, x2, y2, EM_CHNSTATE_HIDE);
			pDeskTop->ShowChnState( ch, EM_STATE_TIMER, x2, y2, EM_CHNSTATE_HIDE);
		}
		
		if(bMD[ch] == 1)
		{
			pDeskTop->ShowChnState( ch, EM_STATE_MD, x2, y2, EM_CHNSTATE_1 );
		}
		else
		{
			pDeskTop->ShowChnState( ch, EM_STATE_MD, x2, y2, EM_CHNSTATE_HIDE );
		}	

		if(bSensorAlarm[ch] == 1)
		{
			pDeskTop->ShowChnState( ch, EM_STATE_ALARM, x2, y2, EM_CHNSTATE_1 );
		}
		else
		{
			pDeskTop->ShowChnState( ch, EM_STATE_ALARM, x2, y2, EM_CHNSTATE_HIDE );
		}
	}
	
	return 0;
}

static u8* g_nCurRecStatus = NULL;
u8 GetCurRecStatus(u8 nChn)
{	
	if(g_nCurRecStatus == NULL)
	{
		g_nCurRecStatus = (u8*)calloc(1, GetVideoMainNum());
		memset(g_nCurRecStatus,0,1*GetVideoMainNum());//csp modify
	}
	
	return g_nCurRecStatus[nChn];
}

u8 SetCurRecStatus(u8 nChn)
{	
	BizSetCurRecStatus(nChn,g_nCurRecStatus[nChn]);
}

int EventRecordDeal(SBizRecStatus* recStatus)
{
	if(g_nCurRecStatus == NULL)
	{
		g_nCurRecStatus = (u8*)calloc(1, GetVideoMainNum());
		memset(g_nCurRecStatus,0,1*GetVideoMainNum());//csp modify
	}
	
	if(!recStatus)
	{
		return -1;
	}
	
	int rec_status_led = 0;//cw_led
	
	int rett = 0;
	
	if((0x80 == recStatus->nCurStatus) && (0 == GetRecordEnable(recStatus->nChn)))
	{
		BizRecordPause(recStatus->nChn);
		rett = 617;
	}
	
	recStatus->nCurStatus &= 0x7f;//第8位是预录的标志，需要屏蔽掉
	
	g_nCurRecStatus[recStatus->nChn] = recStatus->nCurStatus;
	
	SBizParaTarget bizTar;
	
	if(recStatus->nCurStatus != 0)
	{
		bRecord[recStatus->nChn] = 1;
	}
	else
	{
		bRecord[recStatus->nChn] = 0;
	}
	
	//yzw add
	if((0 != recStatus->nCurStatus)
		&& (0 == GetRecordEnable(recStatus->nChn)))
		//&& (0 == GetAudioEnable(recStatus->nChn)))
	{
		g_nCurRecStatus[recStatus->nChn] = 0;
		bRecord[recStatus->nChn] = 0;
		
		BizRecordPause(recStatus->nChn);
		
		for(int i=0;i<GetVideoMainNum();i++)//cw_led
		{
			rec_status_led |= GetCurRecStatus(i);
		}
		RecLedCtrl(rec_status_led,REC_LED_CTRL);
		
		return 617;
	}
	else if((0 != recStatus->nCurStatus)
		&& (0 == GetRecordEnable(recStatus->nChn))
		&& (0 != GetAudioEnable(recStatus->nChn)))
	{
		rett = 618;
	}
	else if((0 != recStatus->nCurStatus)
		&& (0 != GetRecordEnable(recStatus->nChn))
		&& (0 == GetAudioEnable(recStatus->nChn)))
	{
		rett = 619;
	}
	//end
	
	for(int i=0;i<GetVideoMainNum();i++)//cw_led
	{
		rec_status_led |= GetCurRecStatus(i);
	}
	
	RecLedCtrl(rec_status_led,REC_LED_CTRL);
	
    SetCurRecStatus(recStatus->nChn);
	
	u8 nSplitNum = 1;
	u8 nMaxChn;
	u8 nXScale;
	u8 nYScale;
	static int nScreanWidth = 720,  nScreanHeight = 576;
	int nChnWidth;
	int nChnHeight;
	int x2, y2;
	int nOffSet25 = 0;
	
	//csp modify 20130501
	int large = 0;
	int small = 0;
	int index = 0;
	
	if((!g_pPages) || (!g_pPages[EM_PAGE_DESKTOP]) || (!g_pPages[EM_PAGE_START]))
	{
		return -1;
	}
	
	static char flag = 1;
	if(flag)
	{
		GetVgaResolution(&nScreanWidth, &nScreanHeight);
		flag = 0;
	}
	
	CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
	CPageStartFrameWork* pPageStart = (CPageStartFrameWork *)g_pPages[EM_PAGE_START];
	
	int nStartChn = pPageStart->GetCurPreview();
	EMBIZPREVIEWMODE emPreMode = pPageStart->GetCurPreviewMode();
	
	if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
	{
		nStartChn = g_pBizPreviewData.nModePara;
	}
	
	nMaxChn = GetVideoMainNum();
	
	switch(emPreMode)
	{
		case EM_BIZPREVIEW_1SPLIT:
			nSplitNum = 1;
			nXScale = 1;
			nYScale = 1;
			break;
		case EM_BIZPREVIEW_4SPLITS:
			nSplitNum = 4;
			nXScale = 2;
			nYScale = 2;
			break;
		case EM_BIZPREVIEW_9SPLITS:
			nSplitNum = 9;
			nXScale = 3;
			nYScale = 3;
			break;
		case EM_BIZPREVIEW_13SPLITS:
			/*nSplitNum = 13;
			nXScale = 2;
			nYScale = 2;
			break;*/
		case EM_BIZPREVIEW_6SPLITS:
		case EM_BIZPREVIEW_10SPLITS:
		case EM_BIZPREVIEW_16SPLITS:
			nSplitNum = 16;
			nXScale = 4;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_24SPLITS://9624
			nSplitNum = 24;
			nXScale = 6;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_25SPLITS://9624
			nSplitNum = 25;
			nXScale = 5;
			nYScale = 5;
			nOffSet25 = 8;
			break;
		case EM_BIZPREVIEW_36SPLITS:
			nSplitNum = 36;
			nXScale = nYScale = 6;
			break;
		//csp modify 20130501
		case EM_BIZPREVIEW_PIP:
			nSplitNum = nMaxChn;
			nXScale = 2;
			nYScale = 2;
			large = (nStartChn >> 4) & 0xf;
			small = (nStartChn >> 0) & 0xf;
			//nStartChn = min(large,small);
			//return 0;
			break;
		default:
			return -1;
	}
	
	nMaxChn = (nMaxChn < (nStartChn + nSplitNum)) ? nMaxChn : (nStartChn + nSplitNum);
	
	nChnWidth = nScreanWidth / nXScale;
	
	if(nOffSet25 > 0)
	{
		SBizSystemPara sysPara;
		BizGetSysInfo(&sysPara);
		
		if(EM_BIZ_PAL == sysPara.nVideoStandard)
		{
			//nChnHeight = nScreanHeight * (576 - nOffSet25 * 2) / nYScale / 576;
			nOffSet25 = nScreanHeight * nOffSet25 / 576;
			nChnHeight = (nScreanHeight - nOffSet25 * 2) / nYScale;			
		}
		else
		{
			nOffSet25 = 0;
			nChnHeight = nScreanHeight / nYScale;
		}
	}
	else
	{
		nChnHeight = nScreanHeight / nYScale;
	}
	
	if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
	{
		nStartChn = 0;//min(large,small);
	}
	
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	
	int ret = BizGetPara(&bizTar, &bizPreCfg);
	if(ret!=0)
	{
		return -1;
	}
	
	char chName[32] = {0};
	//int nOsdWidth = 0;
	
	for(int i = nStartChn; i < nMaxChn; i++)
	{
		#if 0
		GetChnName(i, chName, sizeof(chName));
		nOsdWidth = strlen(chName) ? ((strlen(chName)+1)*TEXT_CHNWIDTH/3) : 0;
		for(int a= 0; a < strlen(chName); a++)
		{
			if(chName[a] >= 128)
			{
				a+=2;
				nOsdWidth-=8;
			}
		}
		#endif
		
		if(emPreMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(i == nStartChn)
			{
				i += 4;
			}
			else if(i == nStartChn+1)
			{
				i += 5;
			}
			else
			{
				i += 6;
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_6SPLITS)
		{
			if(i == nStartChn)
			{
				i += 4;
			}
			else if(i == nStartChn+1)
			{
				i += 5;
			}
			else
			{
				i += (i+4);
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			if(i == large)
			{
				index = i;
				i = 2;
			}
			else if(i == small)
			{
				index = i;
				i = 3;
			}
			else
			{
				continue;
			}
		}
		
		x2 = PIC_STATE_OFFSET_X +  (( i - nStartChn) % nXScale) * nChnWidth;
		y2 = (nChnHeight-(576-PIC_STATE_OFFSET_Y)+CHNNAME_HEIGHT+2) + (( i - nStartChn) / nXScale) * nChnHeight;
		
		if(nOffSet25)
		{
			y2 += nOffSet25;
			
			if (i >= 20 && i <= 24 && 0 == nStartChn) 
			{
				y2 += nOffSet25;
			}
			else if (i >= 27 && i <= 31 && nStartChn > 0)
			{
				y2 += nOffSet25;
			}
		}
		
		if(nScreanWidth == 720)
		{
			if(0 == (i-nStartChn)%nXScale)
			{
				x2 += 26;
			}
			
			if((( i - nStartChn) / nXScale + 1) == nYScale)
			{
				y2 = (( i - nStartChn) / nXScale) * nChnHeight + (576-PIC_STATE_OFFSET_Y-CHNNAME_HEIGHT-16-2+CHNNAME_HEIGHT+2)+nOffSet25;
			}
			
			if(1 == nXScale)
			{
				x2 = 100;
				y2 = nChnHeight-100+CHNNAME_HEIGHT+2;
			}
		}				
			
		if(emPreMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(i == nStartChn+4)
			{
				i -= 4;
			}
			else if(i == nStartChn+1+5)
			{
				i -= 5;
			}
			else
			{
				i -= 6;
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_6SPLITS)
		{
			i = (i-4)/2;
		}
		else if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			i = index;
		}
		
		if(i == recStatus->nChn)
		{
			if(bizPreCfg.nShowRecState)
			{
				//printf("curRecStatus:0x%02x\n", recStatus->nCurStatus);
				if(recStatus->nCurStatus)
				{
					pDeskTop->ShowChnState( recStatus->nChn, EM_STATE_RECORD, x2, y2, EM_CHNSTATE_1 );
					if(recStatus->nCurStatus & 1)
					{
						pDeskTop->ShowChnState( recStatus->nChn, EM_STATE_MANUAL_REC, x2, y2, EM_CHNSTATE_1 );
					}
					else
					{
						pDeskTop->ShowChnState( recStatus->nChn, EM_STATE_MANUAL_REC, x2, y2, EM_CHNSTATE_HIDE );						
					}
					
					if(recStatus->nCurStatus & 2)
					{
						pDeskTop->ShowChnState( recStatus->nChn, EM_STATE_TIMER, x2, y2, EM_CHNSTATE_1 );						
					}
					else
					{
						pDeskTop->ShowChnState( recStatus->nChn, EM_STATE_TIMER, x2, y2, EM_CHNSTATE_HIDE );						
					}
				}
				else
				{
					pDeskTop->ShowChnState( recStatus->nChn, EM_STATE_RECORD, x2, y2, EM_CHNSTATE_HIDE);
					pDeskTop->ShowChnState( recStatus->nChn, EM_STATE_MANUAL_REC, x2, y2, EM_CHNSTATE_HIDE );
					pDeskTop->ShowChnState( recStatus->nChn, EM_STATE_TIMER, x2, y2, EM_CHNSTATE_HIDE );
				}
			}
			else
			{
				if(recStatus->nCurStatus)
				{
					pDeskTop->ShowChnState( recStatus->nChn, EM_STATE_RECORD, x2, y2, EM_CHNSTATE_1 );
				}
				else
				{
					pDeskTop->ShowChnState( recStatus->nChn, EM_STATE_RECORD, x2, y2, EM_CHNSTATE_HIDE);
				}
				
				pDeskTop->ShowChnState( recStatus->nChn, EM_STATE_MANUAL_REC, x2, y2, EM_CHNSTATE_HIDE );
				pDeskTop->ShowChnState( recStatus->nChn, EM_STATE_TIMER, x2, y2, EM_CHNSTATE_HIDE );
			}
		}
	}
	
	return rett;
}

int EventMDDeal(SBizAlarmStatus* mdStatus)
{
	if(!mdStatus)
	{
		return -1;
	}
	
	if(mdStatus->nCurStatus != 0)
	{
		bMD[mdStatus->nChn] = 1;
	}
	else
	{
		bMD[mdStatus->nChn] = 0;
	}
	
	#ifdef longse//csp modify
	u8 led_ctrl = 0;
	int num = GetMaxChnNum();
	for(int i=0;i<num;i++)
	{
		if(bMD[i])
		{
			led_ctrl = 1;
		}
	}
	RecLedCtrl(led_ctrl,ALARM_LED_CTRL);
	//printf("Alarm led:%s\n",led_ctrl?"ON":"OFF");
	#endif
	
	u8 nSplitNum = 1;
	u8 nMaxChn;
	u8 nXScale;
	u8 nYScale;
	static int nScreanWidth = 720,  nScreanHeight = 576;
	int nChnWidth, nChnHeight;
	int x2, y2;
	int nOffSet25 = 0;
	
	//csp modify 20130501
	int large = 0;
	int small = 0;
	int index = 0;
	
	if((!g_pPages) || (!g_pPages[EM_PAGE_DESKTOP]) || (!g_pPages[EM_PAGE_START]))
	{
		return -1;
	}
	
	static char flag = 1;
	if(flag)
	{
		GetVgaResolution(&nScreanWidth, &nScreanHeight);
		flag = 0;
	}
	
	CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
	CPageStartFrameWork* pPageStart = (CPageStartFrameWork *)g_pPages[EM_PAGE_START];
	
	int nStartChn = pPageStart->GetCurPreview();
	EMBIZPREVIEWMODE emPreMode = pPageStart->GetCurPreviewMode();
	
	if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
	{
		nStartChn = g_pBizPreviewData.nModePara;
	}
	
	switch(emPreMode)
	{
		case EM_BIZPREVIEW_1SPLIT:
			nSplitNum = 1;
			nXScale = 1;
			nYScale = 1;
			break;
		case EM_BIZPREVIEW_4SPLITS:
			nSplitNum = 4;
			nXScale = 2;
			nYScale = 2;
			break;
		case EM_BIZPREVIEW_9SPLITS:
			nSplitNum = 9;
			nXScale = 3;
			nYScale = 3;
			break;
		case EM_BIZPREVIEW_13SPLITS:
			/*nSplitNum = 13;
			nXScale = 2;
			nYScale = 2;
			break;*/
		case EM_BIZPREVIEW_6SPLITS:
		case EM_BIZPREVIEW_10SPLITS:
		case EM_BIZPREVIEW_16SPLITS:
			nSplitNum = 16;
			nXScale = 4;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_24SPLITS://9624
			nSplitNum = 24;
			nXScale = 6;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_25SPLITS://9624
			nSplitNum = 25;
			nXScale = 5;
			nYScale = 5;
			nOffSet25 = 8;
			break;
		case EM_BIZPREVIEW_36SPLITS:
			nSplitNum = 36;
			nXScale = nYScale = 6;
			break;
		//csp modify 20130501
		case EM_BIZPREVIEW_PIP:
			nSplitNum = nMaxChn;
			nXScale = 2;
			nYScale = 2;
			large = (nStartChn >> 4) & 0xf;
			small = (nStartChn >> 0) & 0xf;
			//nStartChn = min(large,small);
			//return 0;
			break;
		default:
			return -1;
	}
	
	nMaxChn = GetVideoMainNum();
	nMaxChn = (nMaxChn < (nStartChn + nSplitNum)) ? nMaxChn : (nStartChn + nSplitNum);
	
	nChnWidth = nScreanWidth / nXScale;
	if(nOffSet25 > 0)
	{
		SBizSystemPara sysPara;
		BizGetSysInfo(&sysPara);
		
		if(EM_BIZ_PAL == sysPara.nVideoStandard)
		{
			//nChnHeight = nScreanHeight * (576 - nOffSet25 * 2) / nYScale / 576;
			nOffSet25 = nScreanHeight * nOffSet25 / 576;	
			nChnHeight = (nScreanHeight - nOffSet25 * 2) / nYScale;		
		}
		else
		{
			nOffSet25 = 0;
			nChnHeight = nScreanHeight / nYScale;
		}
	}
	else
	{
		nChnHeight = nScreanHeight / nYScale;
	}
	
	if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
	{
		nStartChn = 0;//min(large,small);
	}
	
	if(mdStatus->nCurStatus != 0)
	{
		bMD[mdStatus->nChn] = 1;
	}
	else
	{
		bMD[mdStatus->nChn] = 0;
	}
	//printf("nStartChn = %d nMaxChn = %d\n",nStartChn,nMaxChn);
	
	char chName[32] = {0};
	//int nOsdWidth = 0;
	
	for(int i = nStartChn; i < nMaxChn; i++)
	{
		#if 0
		GetChnName(i, chName, sizeof(chName));
		nOsdWidth = strlen(chName) ? ((strlen(chName)+1)*TEXT_CHNWIDTH/3) : 0;
		
		for(int a= 0; a < strlen(chName); a++)
		{
			if(chName[a] >= 128)
			{
				a+=2;
				nOsdWidth-=8;
			}
		}
		#endif
		
		if(emPreMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(i == nStartChn)
			{
				i += 4;
			}
			else if(i == nStartChn+1)
			{
				i += 5;
			}
			else
			{
				i += 6;
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_6SPLITS)
		{
			if(i == nStartChn)
			{
				i += 4;
			}
			else if(i == nStartChn+1)
			{
				i += 5;
			}
			else
			{
				i += (i+4);
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			if(i == large)
			{
				index = i;
				i = 2;
			}
			else if(i == small)
			{
				index = i;
				i = 3;
			}
			else
			{
				continue;
			}
		}
		
		x2 = PIC_STATE_OFFSET_X +  (( i - nStartChn) % nXScale) * nChnWidth;
		y2 = (nChnHeight-(576-PIC_STATE_OFFSET_Y)+CHNNAME_HEIGHT+2) + (( i - nStartChn) / nXScale) * nChnHeight;
		
		if(nOffSet25)
		{
			y2 += nOffSet25;
			
			if (i >= 20 && i <= 24 && 0 == nStartChn) 
			{
				y2 += nOffSet25;
			}
			else if (i >= 27 && i <= 31 && nStartChn > 0)
			{
				y2 += nOffSet25;
			}
		}
		
		if(nScreanWidth == 720)
		{
			if(0 == (i-nStartChn)%nXScale)
			{
				x2 += 26;
			}
			
			if((( i - nStartChn) / nXScale + 1) == nYScale)
			{
				y2 = (( i - nStartChn) / nXScale) * nChnHeight + (576-PIC_STATE_OFFSET_Y-CHNNAME_HEIGHT-16-2+CHNNAME_HEIGHT+2)+nOffSet25;
			}
			
			if(1 == nXScale)
			{
				x2 = 100;
				y2 = nChnHeight-100+CHNNAME_HEIGHT+2;
			}
		}				
		
		if(emPreMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(i == nStartChn+4)
			{
				i -= 4;
			}
			else if(i == nStartChn+1+5)
			{
				i -= 5;
			}
			else
			{
				i -= 6;
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_6SPLITS)
		{
			i = (i-4)/2;
		}
		else if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			i = index;
		}
		
		if(i == mdStatus->nChn)
		{
			if(mdStatus->nCurStatus != 0)
			{
				pDeskTop->ShowChnState(i, EM_STATE_MD, x2, y2, EM_CHNSTATE_1);
			}
			else
			{
				pDeskTop->ShowChnState(i, EM_STATE_MD , x2, y2, EM_CHNSTATE_HIDE);
			}
		}
	}

	if(mdStatus->nCurStatus != 0)
	{
		BizGUiWriteLogWithChn(mdStatus->nChn+1, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_START_MD);
	}
	else
	{
		BizGUiWriteLogWithChn(mdStatus->nChn+1, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_STOP_MD);
	}

	
	return 0;
}

CDevMouse* g_devMouse = NULL;
CDevFrontboard* g_devFrontBoard = NULL;

void SetDevMouse(CDevMouse* mouse)
{
	g_devMouse = mouse;
}

CDevMouse* GetDevMouse()
{
	return g_devMouse;
}

void SetDevFrontBoard(CDevFrontboard* board)
{
	g_devFrontBoard = board;
}

#if 0//csp modify 20121222
//void (*RunMouseThread)(void* para)
void* RunMouseThread(void* para)
{
	g_devMouse->RunThread();
}

void* RunFrontBoardThread(void* para)
{
	g_devFrontBoard->RunThread();
}

void RunThread(u8 id)
{
	pthread_t pid;
	if(0 == id)
	{
		pthread_create(&pid, NULL, RunMouseThread, NULL);
	}
	else if(1 == id)
	{
		pthread_create(&pid, NULL, RunFrontBoardThread, NULL);
	}
}
#endif
int EventIPCCoverAlarmDeal(SBizAlarmStatus * sBizAlaStatus)
{
	//printf("********EventSensorAlarmDeal\n");
	
	int sensor_status_led = 0;//cw_led
	
	if(!sBizAlaStatus)
	{
		return -1;
	}

	if(sBizAlaStatus->nCurStatus != 0)
	{
		bIPCCoverAlarm[sBizAlaStatus->nChn] = 1;
	}
	else
	{
		bIPCCoverAlarm[sBizAlaStatus->nChn] = 0;
	}
	//printf("*******bSensorAlarm[%d] = %d\n",sBizAlaStatus->nChn,bSensorAlarm[sBizAlaStatus->nChn]);
	
	u8 nSplitNum = 1, nMaxChn, nXScale, nYScale;
	static int nScreanWidth = 720,	nScreanHeight = 576;
	int nChnWidth, nChnHeight;
	int x2, y2;
	int nOffSet25 = 0;
	
	//csp modify 20130501
	int large = 0;
	int small = 0;
	int index = 0;
	
	if((!g_pPages) || (!g_pPages[EM_PAGE_DESKTOP]) || (!g_pPages[EM_PAGE_START]))
	{
		return -2;
	}
	
	static char flag = 1;
	if(flag)
	{
		GetVgaResolution(&nScreanWidth, &nScreanHeight);
		flag = 0;
	}
	
	CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
	CPageStartFrameWork* pPageStart = (CPageStartFrameWork *)g_pPages[EM_PAGE_START];
	
	int nStartChn = pPageStart->GetCurPreview();
	EMBIZPREVIEWMODE emPreMode = pPageStart->GetCurPreviewMode();
	
	if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
	{
		nStartChn = g_pBizPreviewData.nModePara;
	}
	
	switch(emPreMode)
	{
		case EM_BIZPREVIEW_1SPLIT:
			nSplitNum = 1;
			nXScale = 1;
			nYScale = 1;
			break;
		case EM_BIZPREVIEW_4SPLITS:
			nSplitNum = 4;
			nXScale = 2;
			nYScale = 2;
			break;
		case EM_BIZPREVIEW_9SPLITS:
			nSplitNum = 9;
			nXScale = 3;
			nYScale = 3;
			break;
		case EM_BIZPREVIEW_13SPLITS:
			/*nSplitNum = 13;
			nXScale = 2;
			nYScale = 2;
			break;*/
		case EM_BIZPREVIEW_6SPLITS:
		case EM_BIZPREVIEW_10SPLITS:
		case EM_BIZPREVIEW_16SPLITS:
			nSplitNum = 16;
			nXScale = 4;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_24SPLITS://9624
			nSplitNum = 24;
			nXScale = 6;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_25SPLITS://9624
			nSplitNum = 25;
			nXScale = 5;
			nYScale = 5;
			nOffSet25 = 8;
			break;
		case EM_BIZPREVIEW_36SPLITS:
			nSplitNum = 36;
			nXScale = nYScale = 6;
			break;
		//csp modify 20130501
		case EM_BIZPREVIEW_PIP:
			nSplitNum = nMaxChn;
			nXScale = 2;
			nYScale = 2;
			large = (nStartChn >> 4) & 0xf;
			small = (nStartChn >> 0) & 0xf;
			//nStartChn = min(large,small);
			//return 0;
			break;
		default:
			return -3;
	}
	
	nMaxChn = GetVideoMainNum();
	nMaxChn = (nMaxChn < (nStartChn + nSplitNum)) ? nMaxChn : (nStartChn + nSplitNum);
	
	nChnWidth = nScreanWidth / nXScale;
	if(nOffSet25 > 0)
	{
		SBizSystemPara sysPara;
		BizGetSysInfo(&sysPara);
		
		if(EM_BIZ_PAL == sysPara.nVideoStandard)
		{
			//nChnHeight = nScreanHeight * (576 - nOffSet25 * 2) / nYScale / 576;
			nOffSet25 = nScreanHeight * nOffSet25 / 576;	
			nChnHeight = (nScreanHeight - nOffSet25 * 2) / nYScale; 	
		}
		else
		{
			nOffSet25 = 0;
			nChnHeight = nScreanHeight / nYScale;
		}
	}
	else
	{
		nChnHeight = nScreanHeight / nYScale;
	}
	
	if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
	{
		nStartChn = 0;//min(large,small);
	}


	
	if(sBizAlaStatus->nCurStatus != 0)
	{
		bIPCCoverAlarm[sBizAlaStatus->nChn] = 1;
	}
	else
	{
		bIPCCoverAlarm[sBizAlaStatus->nChn] = 0;
	}
/*
	for(int i=0;i<GetVideoMainNum();i++)//cw_led
	{
		sensor_status_led |= bSensorAlarm[i];
	}
	RecLedCtrl(sensor_status_led,ALARM_LED_CTRL);
*/	
	char chName[32] = {0};
	//int nOsdWidth = 0;
	
	for(int i = nStartChn; i < nMaxChn; i++)
	{
	#if 0
		GetChnName(i, chName, sizeof(chName));
		nOsdWidth = strlen(chName) ? ((strlen(chName)+1)*TEXT_CHNWIDTH/3) : 0;
		for(int a= 0; a < strlen(chName); a++)
		{
			if(chName[a] >= 128)
			{
				a+=2;
				nOsdWidth-=8;
			}
		}
	#endif
		
		if(emPreMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(i == nStartChn)
			{
				i += 4;
			}
			else if(i == nStartChn+1)
			{
				i += 5;
			}
			else
			{
				i += 6;
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_6SPLITS)
		{
			if(i == nStartChn)
			{
				i += 4;
			}
			else if(i == nStartChn+1)
			{
				i += 5;
			}
			else
			{
				i += (i+4);
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			if(i == large)
			{
				index = i;
				i = 2;
			}
			else if(i == small)
			{
				index = i;
				i = 3;
			}
			else
			{
				continue;
			}
		}
		
		x2 = PIC_STATE_OFFSET_X +  (( i - nStartChn) % nXScale) * nChnWidth;
		y2 = (nChnHeight-(576-PIC_STATE_OFFSET_Y)+CHNNAME_HEIGHT+2) + (( i - nStartChn) / nXScale) * nChnHeight;
		if(nOffSet25)
		{
			y2 += nOffSet25;
			
			if (i >= 20 && i <= 24 && 0 == nStartChn) 
			{
				y2 += nOffSet25;
			}
			else if (i >= 27 && i <= 31 && nStartChn > 0)
			{
				y2 += nOffSet25;
			}
		}
		
		if(nScreanWidth == 720)
		{
			if(0 == (i-nStartChn)%nXScale)
			{
				//x2 = nChnWidth - PIC_STATE_OFFSET_X - (nOsdWidth>80?nOsdWidth:80);
				x2 += 26;
			}
			if((( i - nStartChn) / nXScale + 1) == nYScale)
			{
				y2 = (( i - nStartChn) / nXScale) * nChnHeight + (576-PIC_STATE_OFFSET_Y-CHNNAME_HEIGHT-16-2+CHNNAME_HEIGHT+2)+nOffSet25;
			}
			if(1 == nXScale)
			{
				x2 = 100;
				y2 = nChnHeight-100+CHNNAME_HEIGHT+2;
			}
		}
		
		if(emPreMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(i == nStartChn+4)
			{
				i -= 4;
			}
			else if(i == nStartChn+1+5)
			{
				i -= 5;
			}
			else
			{
				i -= 6;
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_6SPLITS)
		{
			i = (i-4)/2;
		}
		else if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			i = index;
		}
		
		if(i == sBizAlaStatus->nChn)
		{
			if(sBizAlaStatus->nCurStatus != 0)
			{
				pDeskTop->ShowChnState(i, EM_STATE_ALARM, x2, y2, EM_CHNSTATE_1);
			}
			else
			{
				pDeskTop->ShowChnState(i, EM_STATE_ALARM , x2, y2, EM_CHNSTATE_HIDE);
			}
		}
	}

	if(sBizAlaStatus->nCurStatus != 0)
	{
		BizGUiWriteLogWithChn(sBizAlaStatus->nChn+1, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_IPCCOVER);
	}
	else
	{
		BizGUiWriteLogWithChn(sBizAlaStatus->nChn+1, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_IPCCOVER_RESUME);
	}
			
	return 0;
}
int Event485ExtSensorAlarmDeal(SBizAlarmStatus* sBizAlaStatus)
{
	if(sBizAlaStatus->nCurStatus != 0)
	{
		BizGUiWriteLogWithChn(sBizAlaStatus->nChn+1, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_START_485EXT);
	}
	else
	{
		BizGUiWriteLogWithChn(sBizAlaStatus->nChn+1, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_STOP_485EXT);
	}
}


int EventIPCExtSensorAlarmDeal(SBizAlarmStatus* sBizAlaStatus)
{
	//printf("********EventSensorAlarmDeal\n");
	
	int sensor_status_led = 0;//cw_led
	
	if(!sBizAlaStatus)
	{
		return -1;
	}

	if(sBizAlaStatus->nCurStatus != 0)
	{
		bIPCExtAlarm[sBizAlaStatus->nChn] = 1;
	}
	else
	{
		bIPCExtAlarm[sBizAlaStatus->nChn] = 0;
	}
	//printf("*******bSensorAlarm[%d] = %d\n",sBizAlaStatus->nChn,bSensorAlarm[sBizAlaStatus->nChn]);
	
	u8 nSplitNum = 1, nMaxChn, nXScale, nYScale;
	static int nScreanWidth = 720,	nScreanHeight = 576;
	int nChnWidth, nChnHeight;
	int x2, y2;
	int nOffSet25 = 0;
	
	//csp modify 20130501
	int large = 0;
	int small = 0;
	int index = 0;
	
	if((!g_pPages) || (!g_pPages[EM_PAGE_DESKTOP]) || (!g_pPages[EM_PAGE_START]))
	{
		return -2;
	}
	
	static char flag = 1;
	if(flag)
	{
		GetVgaResolution(&nScreanWidth, &nScreanHeight);
		flag = 0;
	}
	
	CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
	CPageStartFrameWork* pPageStart = (CPageStartFrameWork *)g_pPages[EM_PAGE_START];
	
	int nStartChn = pPageStart->GetCurPreview();
	EMBIZPREVIEWMODE emPreMode = pPageStart->GetCurPreviewMode();
	
	if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
	{
		nStartChn = g_pBizPreviewData.nModePara;
	}
	
	switch(emPreMode)
	{
		case EM_BIZPREVIEW_1SPLIT:
			nSplitNum = 1;
			nXScale = 1;
			nYScale = 1;
			break;
		case EM_BIZPREVIEW_4SPLITS:
			nSplitNum = 4;
			nXScale = 2;
			nYScale = 2;
			break;
		case EM_BIZPREVIEW_9SPLITS:
			nSplitNum = 9;
			nXScale = 3;
			nYScale = 3;
			break;
		case EM_BIZPREVIEW_13SPLITS:
			/*nSplitNum = 13;
			nXScale = 2;
			nYScale = 2;
			break;*/
		case EM_BIZPREVIEW_6SPLITS:
		case EM_BIZPREVIEW_10SPLITS:
		case EM_BIZPREVIEW_16SPLITS:
			nSplitNum = 16;
			nXScale = 4;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_24SPLITS://9624
			nSplitNum = 24;
			nXScale = 6;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_25SPLITS://9624
			nSplitNum = 25;
			nXScale = 5;
			nYScale = 5;
			nOffSet25 = 8;
			break;
		case EM_BIZPREVIEW_36SPLITS:
			nSplitNum = 36;
			nXScale = nYScale = 6;
			break;
		//csp modify 20130501
		case EM_BIZPREVIEW_PIP:
			nSplitNum = nMaxChn;
			nXScale = 2;
			nYScale = 2;
			large = (nStartChn >> 4) & 0xf;
			small = (nStartChn >> 0) & 0xf;
			//nStartChn = min(large,small);
			//return 0;
			break;
		default:
			return -3;
	}
	
	nMaxChn = GetVideoMainNum();
	nMaxChn = (nMaxChn < (nStartChn + nSplitNum)) ? nMaxChn : (nStartChn + nSplitNum);
	
	nChnWidth = nScreanWidth / nXScale;
	if(nOffSet25 > 0)
	{
		SBizSystemPara sysPara;
		BizGetSysInfo(&sysPara);
		
		if(EM_BIZ_PAL == sysPara.nVideoStandard)
		{
			//nChnHeight = nScreanHeight * (576 - nOffSet25 * 2) / nYScale / 576;
			nOffSet25 = nScreanHeight * nOffSet25 / 576;	
			nChnHeight = (nScreanHeight - nOffSet25 * 2) / nYScale; 	
		}
		else
		{
			nOffSet25 = 0;
			nChnHeight = nScreanHeight / nYScale;
		}
	}
	else
	{
		nChnHeight = nScreanHeight / nYScale;
	}
	
	if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
	{
		nStartChn = 0;//min(large,small);
	}
	
	if(sBizAlaStatus->nCurStatus != 0)
	{
		bIPCExtAlarm[sBizAlaStatus->nChn] = 1;
	}
	else
	{
		bIPCExtAlarm[sBizAlaStatus->nChn] = 0;
	}
/*
	for(int i=0;i<GetVideoMainNum();i++)//cw_led
	{
		sensor_status_led |= bSensorAlarm[i];
	}
	RecLedCtrl(sensor_status_led,ALARM_LED_CTRL);
*/
	char chName[32] = {0};
	//int nOsdWidth = 0;
	
	for(int i = nStartChn; i < nMaxChn; i++)
	{
	#if 0
		GetChnName(i, chName, sizeof(chName));
		nOsdWidth = strlen(chName) ? ((strlen(chName)+1)*TEXT_CHNWIDTH/3) : 0;
		for(int a= 0; a < strlen(chName); a++)
		{
			if(chName[a] >= 128)
			{
				a+=2;
				nOsdWidth-=8;
			}
		}
	#endif
		
		if(emPreMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(i == nStartChn)
			{
				i += 4;
			}
			else if(i == nStartChn+1)
			{
				i += 5;
			}
			else
			{
				i += 6;
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_6SPLITS)
		{
			if(i == nStartChn)
			{
				i += 4;
			}
			else if(i == nStartChn+1)
			{
				i += 5;
			}
			else
			{
				i += (i+4);
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			if(i == large)
			{
				index = i;
				i = 2;
			}
			else if(i == small)
			{
				index = i;
				i = 3;
			}
			else
			{
				continue;
			}
		}
		
		x2 = PIC_STATE_OFFSET_X +  (( i - nStartChn) % nXScale) * nChnWidth;
		y2 = (nChnHeight-(576-PIC_STATE_OFFSET_Y)+CHNNAME_HEIGHT+2) + (( i - nStartChn) / nXScale) * nChnHeight;
		if(nOffSet25)
		{
			y2 += nOffSet25;
			
			if (i >= 20 && i <= 24 && 0 == nStartChn) 
			{
				y2 += nOffSet25;
			}
			else if (i >= 27 && i <= 31 && nStartChn > 0)
			{
				y2 += nOffSet25;
			}
		}
		
		if(nScreanWidth == 720)
		{
			if(0 == (i-nStartChn)%nXScale)
			{
				//x2 = nChnWidth - PIC_STATE_OFFSET_X - (nOsdWidth>80?nOsdWidth:80);
				x2 += 26;
			}
			if((( i - nStartChn) / nXScale + 1) == nYScale)
			{
				y2 = (( i - nStartChn) / nXScale) * nChnHeight + (576-PIC_STATE_OFFSET_Y-CHNNAME_HEIGHT-16-2+CHNNAME_HEIGHT+2)+nOffSet25;
			}
			if(1 == nXScale)
			{
				x2 = 100;
				y2 = nChnHeight-100+CHNNAME_HEIGHT+2;
			}
		}
		
		if(emPreMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(i == nStartChn+4)
			{
				i -= 4;
			}
			else if(i == nStartChn+1+5)
			{
				i -= 5;
			}
			else
			{
				i -= 6;
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_6SPLITS)
		{
			i = (i-4)/2;
		}
		else if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			i = index;
		}
		
		if(i == sBizAlaStatus->nChn)
		{
			if(sBizAlaStatus->nCurStatus != 0)
			{
				pDeskTop->ShowChnState(i, EM_STATE_ALARM, x2, y2, EM_CHNSTATE_1);
			}
			else
			{
				pDeskTop->ShowChnState(i, EM_STATE_ALARM , x2, y2, EM_CHNSTATE_HIDE);
			}
		}
	}
	if(sBizAlaStatus->nCurStatus != 0)
	{
		BizGUiWriteLogWithChn(sBizAlaStatus->nChn+1, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_START_IPCEXT);
	}
	else
	{
		BizGUiWriteLogWithChn(sBizAlaStatus->nChn+1, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_STOP_IPCEXT);
	}
	
	return 0;
}	

int EventVLossAlarmDeal(SBizAlarmStatus* sBizAlaStatus)
{
	//printf("********EventSensorAlarmDeal\n");
	
	int sensor_status_led = 0;//cw_led
	
	if(!sBizAlaStatus)
	{
		return -1;
	}

	if(sBizAlaStatus->nCurStatus != 0)
	{
		bSensorAlarm[sBizAlaStatus->nChn] = 1;
	}
	else
	{
		bSensorAlarm[sBizAlaStatus->nChn] = 0;
	}
	//printf("*******bSensorAlarm[%d] = %d\n",sBizAlaStatus->nChn,bSensorAlarm[sBizAlaStatus->nChn]);
	
	u8 nSplitNum = 1, nMaxChn, nXScale, nYScale;
	static int nScreanWidth = 720,	nScreanHeight = 576;
	int nChnWidth, nChnHeight;
	int x2, y2;
	int nOffSet25 = 0;
	
	//csp modify 20130501
	int large = 0;
	int small = 0;
	int index = 0;
	
	if((!g_pPages) || (!g_pPages[EM_PAGE_DESKTOP]) || (!g_pPages[EM_PAGE_START]))
	{
		return -2;
	}
	
	static char flag = 1;
	if(flag)
	{
		GetVgaResolution(&nScreanWidth, &nScreanHeight);
		flag = 0;
	}
	
	CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
	CPageStartFrameWork* pPageStart = (CPageStartFrameWork *)g_pPages[EM_PAGE_START];
	
	int nStartChn = pPageStart->GetCurPreview();
	EMBIZPREVIEWMODE emPreMode = pPageStart->GetCurPreviewMode();
	
	if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
	{
		nStartChn = g_pBizPreviewData.nModePara;
	}
	
	switch(emPreMode)
	{
		case EM_BIZPREVIEW_1SPLIT:
			nSplitNum = 1;
			nXScale = 1;
			nYScale = 1;
			break;
		case EM_BIZPREVIEW_4SPLITS:
			nSplitNum = 4;
			nXScale = 2;
			nYScale = 2;
			break;
		case EM_BIZPREVIEW_9SPLITS:
			nSplitNum = 9;
			nXScale = 3;
			nYScale = 3;
			break;
		case EM_BIZPREVIEW_13SPLITS:
			/*nSplitNum = 13;
			nXScale = 2;
			nYScale = 2;
			break;*/
		case EM_BIZPREVIEW_6SPLITS:
		case EM_BIZPREVIEW_10SPLITS:
		case EM_BIZPREVIEW_16SPLITS:
			nSplitNum = 16;
			nXScale = 4;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_24SPLITS://9624
			nSplitNum = 24;
			nXScale = 6;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_25SPLITS://9624
			nSplitNum = 25;
			nXScale = 5;
			nYScale = 5;
			nOffSet25 = 8;
			break;
		case EM_BIZPREVIEW_36SPLITS:
			nSplitNum = 36;
			nXScale = nYScale = 6;
			break;
		//csp modify 20130501
		case EM_BIZPREVIEW_PIP:
			nSplitNum = nMaxChn;
			nXScale = 2;
			nYScale = 2;
			large = (nStartChn >> 4) & 0xf;
			small = (nStartChn >> 0) & 0xf;
			//nStartChn = min(large,small);
			//return 0;
			break;
		default:
			return -3;
	}
	
	nMaxChn = GetVideoMainNum();
	nMaxChn = (nMaxChn < (nStartChn + nSplitNum)) ? nMaxChn : (nStartChn + nSplitNum);
	
	nChnWidth = nScreanWidth / nXScale;
	if(nOffSet25 > 0)
	{
		SBizSystemPara sysPara;
		BizGetSysInfo(&sysPara);
		
		if(EM_BIZ_PAL == sysPara.nVideoStandard)
		{
			//nChnHeight = nScreanHeight * (576 - nOffSet25 * 2) / nYScale / 576;
			nOffSet25 = nScreanHeight * nOffSet25 / 576;	
			nChnHeight = (nScreanHeight - nOffSet25 * 2) / nYScale; 	
		}
		else
		{
			nOffSet25 = 0;
			nChnHeight = nScreanHeight / nYScale;
		}
	}
	else
	{
		nChnHeight = nScreanHeight / nYScale;
	}
	
	if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
	{
		nStartChn = 0;//min(large,small);
	}
	
	if(sBizAlaStatus->nCurStatus != 0)
	{
		bSensorAlarm[sBizAlaStatus->nChn] = 1;
	}
	else
	{
		bSensorAlarm[sBizAlaStatus->nChn] = 0;
	}

	for(int i=0;i<GetVideoMainNum();i++)//cw_led
	{
		sensor_status_led |= bSensorAlarm[i];
	}
	RecLedCtrl(sensor_status_led,ALARM_LED_CTRL);
	
	char chName[32] = {0};
	//int nOsdWidth = 0;
	
	for(int i = nStartChn; i < nMaxChn; i++)
	{
	#if 0
		GetChnName(i, chName, sizeof(chName));
		nOsdWidth = strlen(chName) ? ((strlen(chName)+1)*TEXT_CHNWIDTH/3) : 0;
		for(int a= 0; a < strlen(chName); a++)
		{
			if(chName[a] >= 128)
			{
				a+=2;
				nOsdWidth-=8;
			}
		}
	#endif
		
		if(emPreMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(i == nStartChn)
			{
				i += 4;
			}
			else if(i == nStartChn+1)
			{
				i += 5;
			}
			else
			{
				i += 6;
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_6SPLITS)
		{
			if(i == nStartChn)
			{
				i += 4;
			}
			else if(i == nStartChn+1)
			{
				i += 5;
			}
			else
			{
				i += (i+4);
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			if(i == large)
			{
				index = i;
				i = 2;
			}
			else if(i == small)
			{
				index = i;
				i = 3;
			}
			else
			{
				continue;
			}
		}
		
		x2 = PIC_STATE_OFFSET_X +  (( i - nStartChn) % nXScale) * nChnWidth;
		y2 = (nChnHeight-(576-PIC_STATE_OFFSET_Y)+CHNNAME_HEIGHT+2) + (( i - nStartChn) / nXScale) * nChnHeight;
		if(nOffSet25)
		{
			y2 += nOffSet25;
			
			if (i >= 20 && i <= 24 && 0 == nStartChn) 
			{
				y2 += nOffSet25;
			}
			else if (i >= 27 && i <= 31 && nStartChn > 0)
			{
				y2 += nOffSet25;
			}
		}
		
		if(nScreanWidth == 720)
		{
			if(0 == (i-nStartChn)%nXScale)
			{
				//x2 = nChnWidth - PIC_STATE_OFFSET_X - (nOsdWidth>80?nOsdWidth:80);
				x2 += 26;
			}
			if((( i - nStartChn) / nXScale + 1) == nYScale)
			{
				y2 = (( i - nStartChn) / nXScale) * nChnHeight + (576-PIC_STATE_OFFSET_Y-CHNNAME_HEIGHT-16-2+CHNNAME_HEIGHT+2)+nOffSet25;
			}
			if(1 == nXScale)
			{
				x2 = 100;
				y2 = nChnHeight-100+CHNNAME_HEIGHT+2;
			}
		}
		
		if(emPreMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(i == nStartChn+4)
			{
				i -= 4;
			}
			else if(i == nStartChn+1+5)
			{
				i -= 5;
			}
			else
			{
				i -= 6;
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_6SPLITS)
		{
			i = (i-4)/2;
		}
		else if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			i = index;
		}
		
		if(i == sBizAlaStatus->nChn)
		{
			if(sBizAlaStatus->nCurStatus != 0)
			{
				pDeskTop->ShowChnState(i, EM_STATE_ALARM, x2, y2, EM_CHNSTATE_1);
			}
			else
			{
				pDeskTop->ShowChnState(i, EM_STATE_ALARM , x2, y2, EM_CHNSTATE_HIDE);
			}
		}
	}
	if(sBizAlaStatus->nCurStatus != 0)
	{
		BizGUiWriteLogWithChn(sBizAlaStatus->nChn+1, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_VIDEO_LOST);
	}
	else
	{
		BizGUiWriteLogWithChn(sBizAlaStatus->nChn+1, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_VIDEO_RESUME);
	}
			
	return 0;
}


int EventSensorAlarmDeal(SBizAlarmStatus* sBizAlaStatus)
{
	//printf("********EventSensorAlarmDeal\n");
	
	int sensor_status_led = 0;//cw_led
	
	if(!sBizAlaStatus)
	{
		return -1;
	}

	if(sBizAlaStatus->nCurStatus != 0)
	{
		bSensorAlarm[sBizAlaStatus->nChn] = 1;
	}
	else
	{
		bSensorAlarm[sBizAlaStatus->nChn] = 0;
	}
	//printf("*******bSensorAlarm[%d] = %d\n",sBizAlaStatus->nChn,bSensorAlarm[sBizAlaStatus->nChn]);
	
	u8 nSplitNum = 1, nMaxChn, nXScale, nYScale;
	static int nScreanWidth = 720,  nScreanHeight = 576;
	int nChnWidth, nChnHeight;
	int x2, y2;
	int nOffSet25 = 0;
	
	//csp modify 20130501
	int large = 0;
	int small = 0;
	int index = 0;
	
	if((!g_pPages) || (!g_pPages[EM_PAGE_DESKTOP]) || (!g_pPages[EM_PAGE_START]))
	{
		return -2;
	}
	
	static char flag = 1;
	if(flag)
	{
		GetVgaResolution(&nScreanWidth, &nScreanHeight);
		flag = 0;
	}
	
	CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
	CPageStartFrameWork* pPageStart = (CPageStartFrameWork *)g_pPages[EM_PAGE_START];
	
	int nStartChn = pPageStart->GetCurPreview();
	EMBIZPREVIEWMODE emPreMode = pPageStart->GetCurPreviewMode();
	
	if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
	{
		nStartChn = g_pBizPreviewData.nModePara;
	}
	
	switch(emPreMode)
	{
		case EM_BIZPREVIEW_1SPLIT:
			nSplitNum = 1;
			nXScale = 1;
			nYScale = 1;
			break;
		case EM_BIZPREVIEW_4SPLITS:
			nSplitNum = 4;
			nXScale = 2;
			nYScale = 2;
			break;
		case EM_BIZPREVIEW_9SPLITS:
			nSplitNum = 9;
			nXScale = 3;
			nYScale = 3;
			break;
		case EM_BIZPREVIEW_13SPLITS:
			/*nSplitNum = 13;
			nXScale = 2;
			nYScale = 2;
			break;*/
		case EM_BIZPREVIEW_6SPLITS:
		case EM_BIZPREVIEW_10SPLITS:
		case EM_BIZPREVIEW_16SPLITS:
			nSplitNum = 16;
			nXScale = 4;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_24SPLITS://9624
			nSplitNum = 24;
			nXScale = 6;
			nYScale = 4;
			break;
		case EM_BIZPREVIEW_25SPLITS://9624
			nSplitNum = 25;
			nXScale = 5;
			nYScale = 5;
			nOffSet25 = 8;
			break;
		case EM_BIZPREVIEW_36SPLITS:
			nSplitNum = 36;
			nXScale = nYScale = 6;
			break;
		//csp modify 20130501
		case EM_BIZPREVIEW_PIP:
			nSplitNum = nMaxChn;
			nXScale = 2;
			nYScale = 2;
			large = (nStartChn >> 4) & 0xf;
			small = (nStartChn >> 0) & 0xf;
			//nStartChn = min(large,small);
			//return 0;
			break;
		default:
			return -3;
	}
	
	nMaxChn = GetVideoMainNum();
	nMaxChn = (nMaxChn < (nStartChn + nSplitNum)) ? nMaxChn : (nStartChn + nSplitNum);
	
	nChnWidth = nScreanWidth / nXScale;
	if(nOffSet25 > 0)
	{
		SBizSystemPara sysPara;
		BizGetSysInfo(&sysPara);
		
		if(EM_BIZ_PAL == sysPara.nVideoStandard)
		{
			//nChnHeight = nScreanHeight * (576 - nOffSet25 * 2) / nYScale / 576;
			nOffSet25 = nScreanHeight * nOffSet25 / 576;	
			nChnHeight = (nScreanHeight - nOffSet25 * 2) / nYScale;		
		}
		else
		{
			nOffSet25 = 0;
			nChnHeight = nScreanHeight / nYScale;
		}
	}
	else
	{
		nChnHeight = nScreanHeight / nYScale;
	}
	
	if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
	{
		nStartChn = 0;//min(large,small);
	}
	
	if(sBizAlaStatus->nCurStatus != 0)
	{
		bSensorAlarm[sBizAlaStatus->nChn] = 1;
	}
	else
	{
		bSensorAlarm[sBizAlaStatus->nChn] = 0;
	}

	for(int i=0;i<GetVideoMainNum();i++)//cw_led
	{
		sensor_status_led |= bSensorAlarm[i];
	}
	RecLedCtrl(sensor_status_led,ALARM_LED_CTRL);
	
	char chName[32] = {0};
	//int nOsdWidth = 0;
	
	for(int i = nStartChn; i < nMaxChn; i++)
	{
		#if 0
		GetChnName(i, chName, sizeof(chName));
		nOsdWidth = strlen(chName) ? ((strlen(chName)+1)*TEXT_CHNWIDTH/3) : 0;
		for(int a= 0; a < strlen(chName); a++)
		{
			if(chName[a] >= 128)
			{
				a+=2;
				nOsdWidth-=8;
			}
		}
		#endif
		
		if(emPreMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(i == nStartChn)
			{
				i += 4;
			}
			else if(i == nStartChn+1)
			{
				i += 5;
			}
			else
			{
				i += 6;
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_6SPLITS)
		{
			if(i == nStartChn)
			{
				i += 4;
			}
			else if(i == nStartChn+1)
			{
				i += 5;
			}
			else
			{
				i += (i+4);
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			if(i == large)
			{
				index = i;
				i = 2;
			}
			else if(i == small)
			{
				index = i;
				i = 3;
			}
			else
			{
				continue;
			}
		}
		
		x2 = PIC_STATE_OFFSET_X +  (( i - nStartChn) % nXScale) * nChnWidth;
		y2 = (nChnHeight-(576-PIC_STATE_OFFSET_Y)+CHNNAME_HEIGHT+2) + (( i - nStartChn) / nXScale) * nChnHeight;
		if(nOffSet25)
		{
			y2 += nOffSet25;
			
			if (i >= 20 && i <= 24 && 0 == nStartChn) 
			{
				y2 += nOffSet25;
			}
			else if (i >= 27 && i <= 31 && nStartChn > 0)
			{
				y2 += nOffSet25;
			}
		}
		
		if(nScreanWidth == 720)
		{
			if(0 == (i-nStartChn)%nXScale)
			{
				//x2 = nChnWidth - PIC_STATE_OFFSET_X - (nOsdWidth>80?nOsdWidth:80);
				x2 += 26;
			}
			if((( i - nStartChn) / nXScale + 1) == nYScale)
			{
				y2 = (( i - nStartChn) / nXScale) * nChnHeight + (576-PIC_STATE_OFFSET_Y-CHNNAME_HEIGHT-16-2+CHNNAME_HEIGHT+2)+nOffSet25;
			}
			if(1 == nXScale)
			{
				x2 = 100;
				y2 = nChnHeight-100+CHNNAME_HEIGHT+2;
			}
		}
		
		if(emPreMode == EM_BIZPREVIEW_10SPLITS)
		{
			if(i == nStartChn+4)
			{
				i -= 4;
			}
			else if(i == nStartChn+1+5)
			{
				i -= 5;
			}
			else
			{
				i -= 6;
			}
		}
		else if(emPreMode == EM_BIZPREVIEW_6SPLITS)
		{
			i = (i-4)/2;
		}
		else if(emPreMode == EM_BIZPREVIEW_PIP)//csp modify 20130501
		{
			i = index;
		}
		
		if(i == sBizAlaStatus->nChn)
		{
			if(sBizAlaStatus->nCurStatus != 0)
			{
				pDeskTop->ShowChnState(i, EM_STATE_ALARM, x2, y2, EM_CHNSTATE_1);
			}
			else
			{
				pDeskTop->ShowChnState(i, EM_STATE_ALARM , x2, y2, EM_CHNSTATE_HIDE);
			}
		}
	}
	if(sBizAlaStatus->nCurStatus != 0)
	{
		BizGUiWriteLogWithChn(sBizAlaStatus->nChn+1, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_START_LOCAL_ALARM);
	}
	else
	{
		BizGUiWriteLogWithChn(sBizAlaStatus->nChn+1, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_STOP_LOCAL_ALARM);
	}
			
	return 0;
}	

/* create cdc */
static CDC* pCdc = NULL;//debug temp
static CLocales* pLocales = NULL;

void EventTimeTickDeal(s8* pstrDataTime)
{
	//return;
	//CDevFrontboard* fontboard = CDevFrontboard::instance();
	//CDevMouse* mouseAA = CDevMouse::instance();
	
	#if 0
	CDevMouse* mouse = g_devMouse;
	
	if(mouse)
	{
		mouse->SendSignal(XM_TIMETICK, 0, FB_FLAG_NONE);
	}
	#endif
	
	if(pstrDataTime)
	{
		if(NULL == g_pPages || NULL == g_pPages[EM_PAGE_DESKTOP])
		{
			return;
		}
		
		CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
		
		pDeskTop->SetTimeTitleString(0, pstrDataTime);
	}
}

void bizData_SendMail(SBizEventPara* pPara)
{
	//printf("## bizData_SendMail %d\n", pPara->emType);
	
	s32 ret;
	u8 	i = 0;
	char szSubject[256];
	char szBody[512];
	char szUserName[32] = {0};
	char szToUser[32] = {0};
	char szTime[64] = {0};
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmDispatch		sAlarmCfgIns;
	memset(&sAlarmCfgIns, 0, sizeof(sAlarmCfgIns));
	
	switch(pPara->emType)
	{
		case EM_BIZ_EVENT_ALARM_VLOSS:
		{	
			sParaTgtIns.emBizParaType 	= EM_BIZ_DISPATCH_VLOST;
		} break;
		case EM_BIZ_EVENT_ALARM_VBLIND:
		{		
			sParaTgtIns.emBizParaType 	= EM_BIZ_DISPATCH_VBLIND;
		} break;
		case EM_BIZ_EVENT_ALARM_VMOTION:
		{
			sParaTgtIns.emBizParaType 	= EM_BIZ_DISPATCH_VMOTION;
		} break;
		case EM_BIZ_EVENT_ALARM_SENSOR:
		{
			sParaTgtIns.emBizParaType 	= EM_BIZ_DISPATCH_SENSOR;
		} break;
		case EM_BIZ_EVENT_ALARM_IPCEXT:
		{
			sParaTgtIns.emBizParaType 	= EM_BIZ_DISPATCH_IPCEXTSENSOR;
		} break;
		case EM_BIZ_EVENT_ALARM_485EXT:
		{
			sParaTgtIns.emBizParaType 	= EM_BIZ_DISPATCH_485EXTSENSOR;
		} break;
		case EM_BIZ_EVENT_ALARM_IPCCOVER:
		{
			sParaTgtIns.emBizParaType 	= EM_BIZ_DISPATCH_IPCCOVER;
		} break;
		case EM_BIZ_EVENT_ALARM_DISK_LOST:
		case EM_BIZ_EVENT_ALARM_DISK_ERR:
		case EM_BIZ_EVENT_ALARM_DISK_NONE:			
		{
			sParaTgtIns.emBizParaType 	= EM_BIZ_DISPATCH_HDD;
		} break;
		default:
			return;
	}
	
	sParaTgtIns.nChn = pPara->sBizAlaStatus.nChn;
	ret = BizGetPara(&sParaTgtIns, &sAlarmCfgIns);
	if(0 != ret)
	{
		return;
	}
	
	if(!sAlarmCfgIns.nFlagEmail)
	{
		return;
	}
	
	SBizNetPara	sCfgIns;
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	
	sParaTgtIns.emBizParaType = EM_BIZ_NETPARA;
	ret = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(ret == 0)
	{
		SBizNetAdvancePara* pNetAdvPara = &sCfgIns.sAdancePara;
		
		if(strlen(pNetAdvPara->szSendMailBox) <= 5)
		{
			return;		
		}
		
		//struct tm* ptm;
		//ptm = gmtime(&pPara->sBizAlaStatus.nTime);
		
		//csp modify 20131213
		int nTimeZone = GetTimeZone();
		time_t nAlarmTime = pPara->sBizAlaStatus.nTime + GetTimeZoneOffset(nTimeZone);
		
		struct tm tmAlarm;
		struct tm* ptm = &tmAlarm;
		localtime_r(&nAlarmTime, ptm);
		
		sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d",
			ptm->tm_year+1900,// = y - 1900;
			ptm->tm_mon+1,// = m - 1;
			ptm->tm_mday,// = d;
			ptm->tm_hour,// = h;
			ptm->tm_min,// = min;
			ptm->tm_sec// = s;
		);
		
		u8 nChn = pPara->sBizAlaStatus.nChn + 1;
		
		switch(pPara->emType)
		{
			case EM_BIZ_EVENT_ALARM_VLOSS:
			{
				//csp modify
				if(pPara->sBizAlaStatus.nCurStatus)
				{
					sprintf(szSubject, "%s:Alarm:Channel %d video loss!", szTime, nChn);
					sprintf(szBody, "Attention please, channel %d video loss detected at %s!", nChn, szTime);
				}
				else
				{
					sprintf(szSubject, "%s:Alarm:Channel %d video resume!", szTime, nChn);
					sprintf(szBody, "Attention please, channel %d video resume detected at %s!", nChn, szTime);
				}
			} break;
			case EM_BIZ_EVENT_ALARM_VBLIND:
			{
				//csp modify
				if(pPara->sBizAlaStatus.nCurStatus)
				{
					sprintf(szSubject, "%s:Alarm:Channel %d video blocked!", szTime, nChn);
					sprintf(szBody, "Attention please, channel %d blocked detected at %s!", nChn, szTime);
				}
				else
				{
					sprintf(szSubject, "%s:Alarm:Channel %d video blocked resume!", szTime, nChn);
					sprintf(szBody, "Attention please, channel %d blocked resume at %s!", nChn, szTime);
				}
			} break;
			case EM_BIZ_EVENT_ALARM_VMOTION:
			{
				//csp modify
				if(pPara->sBizAlaStatus.nCurStatus)
				{
					sprintf(szSubject, "%s:Alarm:Channel %d motion detected!", szTime, nChn);
					sprintf(szBody, "Attention please, channel %d motion detected at %s!", nChn, szTime);
				}
				else
				{
					sprintf(szSubject, "%s:Alarm:Channel %d motion resume!", szTime, nChn);
					sprintf(szBody, "Attention please, channel %d motion resume at %s!", nChn, szTime);
				}
			} break;
			case EM_BIZ_EVENT_ALARM_SENSOR:
			{
				//csp modify
				if(pPara->sBizAlaStatus.nCurStatus)
				{
					sprintf(szSubject, "%s:Alarm input from line %d!", szTime, nChn);
					sprintf(szBody, "Attention please, pin %d alarm detected at %s!", nChn, szTime);
				}
				else
				{
					sprintf(szSubject, "%s:Alarm input resume from line %d!", szTime, nChn);
					sprintf(szBody, "Attention please, pin %d resume detected at %s!", nChn, szTime);
				}
			} break;
			case EM_BIZ_EVENT_ALARM_485EXT:
			{
				//csp modify
				if(pPara->sBizAlaStatus.nCurStatus)
				{
					sprintf(szSubject, "%s:Alarm input from 485ext %d!", szTime, nChn);
					sprintf(szBody, "Attention please, 485Ext %d alarm detected at %s!", nChn, szTime);
				}
				else
				{
					sprintf(szSubject, "%s:Alarm input resume from 485ext %d!", szTime, nChn);
					sprintf(szBody, "Attention please, 485Ext %d resume detected at %s!", nChn, szTime);
				}
			} break;
			case EM_BIZ_EVENT_ALARM_IPCEXT:
			{
				//csp modify
				if(pPara->sBizAlaStatus.nCurStatus)
				{
					sprintf(szSubject, "%s:Alarm input from IPC %d!", szTime, nChn);
					sprintf(szBody, "Attention please, IPC %d Ext alarm detected at %s!", nChn, szTime);
				}
				else
				{
					sprintf(szSubject, "%s:Alarm input resume from IPC %d!", szTime, nChn);
					sprintf(szBody, "Attention please, IPC %d Ext resume detected at %s!", nChn, szTime);
				}
			} break;
			
			case EM_BIZ_EVENT_ALARM_IPCCOVER:
			{
				//csp modify
				if(pPara->sBizAlaStatus.nCurStatus)
				{
					sprintf(szSubject, "%s:Alarm:Channel %d video blocked!", szTime, nChn);
					sprintf(szBody, "Attention please, channel %d blocked detected at %s!", nChn, szTime);
				}
				else
				{
					sprintf(szSubject, "%s:Alarm:Channel %d video blocked resume!", szTime, nChn);
					sprintf(szBody, "Attention please, channel %d blocked resume at %s!", nChn, szTime);
				}
			} break;
			case EM_BIZ_EVENT_ALARM_DISK_LOST:
			{
				//硬盘丢失实际上是没有恢复的
				if(pPara->sBizAlaStatus.nCurStatus)
				{
					sprintf(szSubject, "%s:Alarm:SATA%d disk lost!", szTime, nChn);
					sprintf(szBody, "Attention please, SATA%d disk lost detected at %s!", nChn, szTime);
				}
				//else
				//{
				//	sprintf(szSubject, "%s:Alarm:Channel %d video blocked resume!", szTime, nChn);
				//	sprintf(szBody, "Attention please, channel %d blocked resume at %s!", nChn, szTime);
				//}
			} break;
			case EM_BIZ_EVENT_ALARM_DISK_ERR:
			{
				//硬盘丢失实际上是没有恢复的
				if(pPara->sBizAlaStatus.nCurStatus)
				{
					sprintf(szSubject, "%s:Alarm:SATA%d disk W/R ERR!", szTime, nChn);
					sprintf(szBody, "Attention please, SATA%d disk W/R ERR detected at %s!", nChn, szTime);
				}
				//else
				//{
				//	sprintf(szSubject, "%s:Alarm:Channel %d video blocked resume!", szTime, nChn);
				//	sprintf(szBody, "Attention please, channel %d blocked resume at %s!", nChn, szTime);
				//}
			} break;
			case EM_BIZ_EVENT_ALARM_DISK_NONE:
			{
				//硬盘丢失实际上是没有恢复的
				if(pPara->sBizAlaStatus.nCurStatus)
				{
					sprintf(szSubject, "%s:Alarm:POWER ON no disk!", szTime);
					sprintf(szBody, "Attention please, POWER ON no disk detected at %s!", szTime);
				}
				//else
				//{
				//	sprintf(szSubject, "%s:Alarm:Channel %d video blocked resume!", szTime, nChn);
				//	sprintf(szBody, "Attention please, channel %d blocked resume at %s!", nChn, szTime);
				//}
			} break;
			default:
				return;
		}
		
		SBizMailContent sMailIns;
		memset(&sMailIns, 0, sizeof(SBizMailContent));
		
		memset(szUserName,0,sizeof(szUserName));//csp modify
		
		i = 0;//csp modify
		while(pNetAdvPara->szSendMailBox[i] != '@' && pNetAdvPara->szSendMailBox[i] != '\0')//while(pNetAdvPara->szSendMailBox[i] != '@')//csp modify
		{
			szUserName[i] = pNetAdvPara->szSendMailBox[i];
			i++;
		}
		
		sMailIns.pSrcUsr 	= szUserName;
		sMailIns.pUserName 	= szUserName;
		sMailIns.bAttach  	= 0;
		sMailIns.pSubject 	= szSubject;
		sMailIns.pPasswd 	= pNetAdvPara->szSMailPasswd;
		sMailIns.pSmtp 		= pNetAdvPara->szSmtpServer;
		sMailIns.pSrcBox 	= pNetAdvPara->szSendMailBox;
		sMailIns.pBody 		= szBody;
		
		char *pRcvBox[3];
		pRcvBox[0] = pNetAdvPara->szReceiveMailBox;
		pRcvBox[1] = pNetAdvPara->szReceiveMailBox2;
		pRcvBox[2] = pNetAdvPara->szReceiveMailBox3;
		
		for(int j=0; j<3; j++)
		{
			memset(szToUser, 0, sizeof(szToUser));
			
			i = 0;
			while(pRcvBox[j][i] != '@' && pRcvBox[j][i] != '\0')//while(pRcvBox[j][i] != '@')//csp modify
			{
				szToUser[i] = pRcvBox[j][i];
				i++;
			}
			
			sMailIns.pToUsr 	= szToUser;
			sMailIns.pToBox		= pRcvBox[j];
			
			BizNetSendMail(&sMailIns);
		}
	}
}

void BizNetEventCB(SBizNetStateNotify* pNotify)
{
	printf("BizNetEventCB deal !\n");
}

static sem_t semForDC;
void GetSemForDC(sem_t** sem)
{
	*sem = &semForDC;
}

int CheckGpageExit()//cw_page
{
	for(int i = 0;i<EM_PAGE_NUM;i++)
	{
		if(g_pPages)
		{
			if(!g_pPages[i])
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	return 1;
}

void StopElecZoom(void)
{
	if(!g_pPages || !g_pPages[EM_PAGE_PLAYBACK] || !g_pPages[EM_PAGE_DESKTOP])
	{
		return;
	}
	
	if(((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->GetZoomStatue())
	{
		int ZoomChn = ((CPageDesktop*)g_pPages[EM_PAGE_DESKTOP])->GetElecZoomChn();
		SBizPreviewElecZoom stCapRect;
		stCapRect.s32X = 128;
		stCapRect.s32Y = 128;
		stCapRect.u32Height = 128;
		stCapRect.u32Width = 128;
		
		//printf("StopElecZoom by func start\n");
		
		//csp modify 20130509
		//回放结束时的停止电子放大,如果此时进行回放控制会造成死循环
		//PreviewPbElecZoom(1, ZoomChn, &stCapRect);
		BizPbElecZoomPreview(1, ZoomChn, &stCapRect);
		
		g_PbElecZoom_flag = 0;//csp modify 20130509
		
		//printf("StopElecZoom by func CB\n");
	}
}

extern "C" int BizReloadHdd(SBizStoreDevManage *psStorge);


int BizEventCB(SBizEventPara* pSBizEventPara)
{
	if(pSBizEventPara==NULL)
	{
		printf("BizEventCB got null parameter!!!\n");
		exit(1);
	}
	
	static u64 count = 0;
	static u32 lastTotal = 0;
	static s32 lastProg = 0;
	static time_t lastCurTime = 0;
	static u32 nStartTime = 0;
	static u8 nFitstUseDC = 1;
	
	int ret = 0;
	
	//printf("pSBizEventPara->emType = %d\n",pSBizEventPara->emType);
	
	switch(pSBizEventPara->emType)
	{
		case EM_BIZ_EVENT_PREVIEW_REFRESH:
		{
			if(0 != EventPreviewFreshDeal(&pSBizEventPara->sPreviewData))
			{
				printf("EventPreviewFreshDeal failed!!!\n");
			}
			break;
		}
		case EM_BIZ_EVENT_TIMETICK:
			EventTimeTickDeal(pSBizEventPara->pstrDateTime);
			break;
		case EM_BIZ_EVENT_RECORD:
			return EventRecordDeal(&pSBizEventPara->sBizRecStatus);
			break;
		case EM_BIZ_EVENT_ALARM_VLOSS:
			//yaogang modify 20141020
			if(pSBizEventPara->sBizAlaStatus.nCurStatus)//视频丢失添加信息显示
			{
				//printf("%s yg EM_BIZ_EVENT_ALARM_VLOSS\n", __func__);
				AlarmListAdd(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_VEDIO_LOST, 1);
				//slave = BIZ_LOG_SLAVE_VIDEO_LOST;
				//BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_VIDEO_LOST);
				//只在报警发生时上报
				
				SSG_MSG_TYPE msg;
				msg.type = EM_DVR_VLOST;
				msg.chn = pSBizEventPara->sBizAlaStatus.nChn+1;
				//msg.alm_info.type = EM_SG_ALARM_VLOSS;
				//msg.alm_info.status = 1;
				strcpy(msg.note, GetParsedString("&CfgPtn.VideoLoss"));
				upload_sg(&msg);
				
			}
			else
			{
				AlarmListDelete(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_VEDIO_LOST, 0);
				//报警解除EM_DVR_ALARM_RESUME AlarmRemmove
				SSG_MSG_TYPE msg;
				msg.type = EM_DVR_ALARM_RESUME;
				msg.chn = pSBizEventPara->sBizAlaStatus.nChn+1;
				strcpy(msg.note, GetParsedString("&CfgPtn.AlarmRemmove"));
				upload_sg(&msg);
				//slave = BIZ_LOG_SLAVE_VIDEO_RESUME;
				//BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_VIDEO_RESUME);
			}
			EventVLossAlarmDeal(&pSBizEventPara->sBizAlaStatus);//通道报警图标
			break;
		case EM_BIZ_EVENT_ALARM_VBLIND:
			bizData_SendMail(pSBizEventPara);	
			break;
		case EM_BIZ_EVENT_ALARM_VMOTION:
			//printf("VMOTION\n");
			//yaogang modify 20141020
			if(pSBizEventPara->sBizAlaStatus.nCurStatus)//移动侦测添加信息显示
			{
				//printf("%s yg EM_BIZ_EVENT_ALARM_VMOTION\n", __func__);
				AlarmListAdd(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_MOTION_DETECTION, 1);
				
				SSG_MSG_TYPE msg;
				msg.type = EM_DVR_VMOTION;
				msg.chn = pSBizEventPara->sBizAlaStatus.nChn+1;
				//msg.alm_info.type = EM_SG_ALARM_VMOTION;
				//msg.alm_info.status = 1;
				strcpy(msg.note, GetParsedString("&CfgPtn.Motion"));
				upload_sg(&msg);
			}
			else
			{
				AlarmListDelete(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_MOTION_DETECTION, 0);
				
			}
			
			bizData_SendMail(pSBizEventPara);
			EventMDDeal(&pSBizEventPara->sBizAlaStatus);
			break;
		case EM_BIZ_EVENT_ALARM_SENSOR:
			
			if(pSBizEventPara->sBizAlaStatus.nCurStatus)//移动侦测添加信息显示
			{
				//printf("%s yg EM_BIZ_EVENT_ALARM_SENSOR\n", __func__);
				AlarmListAdd(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_ALARM_INPUT, 1);
				/*
				SSG_MSG_TYPE msg;
				msg.type = EM_DVR_ALARM_EXT;
				msg.chn = pSBizEventPara->sBizAlaStatus.nChn+1;
				//msg.alm_info.type = EM_SG_ALARM_SENSOR;
				//msg.alm_info.status = 1;
				strcpy(msg.note, GetParsedString("&CfgPtn.LocalAlarm"));
				upload_sg(&msg);
				*/

				//printf("%s alarm sersor %d, trigger\n", __func__, pSBizEventPara->sBizAlaStatus.nChn);
				BizSnapAlarmStatusChange(pSBizEventPara->sBizAlaStatus.nChn, 0, 1);
			}
			else
			{
				AlarmListDelete(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_ALARM_INPUT, 0);
				//printf("%s alarm sersor %d, over\n", __func__, pSBizEventPara->sBizAlaStatus.nChn);
				//报警解除EM_DVR_ALARM_RESUME AlarmRemmove
				SSG_MSG_TYPE msg;
				msg.type = EM_DVR_ALARM_RESUME;
				msg.chn = pSBizEventPara->sBizAlaStatus.nChn+1;
				strcpy(msg.note, GetParsedString("&CfgPtn.AlarmRemmove"));
				upload_sg(&msg);
				BizSnapAlarmStatusChange(pSBizEventPara->sBizAlaStatus.nChn, 0, 0);
			}
			
			bizData_SendMail(pSBizEventPara);
			//桌面报警图标
			ret = EventSensorAlarmDeal(&pSBizEventPara->sBizAlaStatus);
			//printf("*********EventSensorAlarmDeal ret = %d\n", ret);
			break;
		case EM_BIZ_EVENT_ALARM_485EXT:
			//yaogang modify 20141020
			if(pSBizEventPara->sBizAlaStatus.nCurStatus)//移动侦测添加信息显示
			{
				//printf("%s yg EM_BIZ_EVENT_ALARM_485EXT\n", __func__);
				AlarmListAdd(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_ALARM_485EXT, 1);
				/*
				SSG_MSG_TYPE msg;
				msg.type = EM_DVR_ALARM_EXT;
				msg.chn = pSBizEventPara->sBizAlaStatus.nChn;
				//msg.alm_info.type = EM_SG_ALARM_485EXT;
				//msg.alm_info.status = 1;
				strcpy(msg.note, GetParsedString("&CfgPtn.485ExtAlarm"));
				upload_sg(&msg);
				*/
				
			}
			else
			{
			/*
				//报警解除EM_DVR_ALARM_RESUME AlarmRemmove
				SSG_MSG_TYPE msg;
				msg.type = EM_DVR_ALARM_RESUME;
				msg.chn = pSBizEventPara->sBizAlaStatus.nChn;
				strcpy(msg.note, GetParsedString("&CfgPtn.AlarmRemmove"));
				upload_sg(&msg);
				*/
				AlarmListDelete(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_ALARM_485EXT, 0);
			}
			
			bizData_SendMail(pSBizEventPara);
			ret = Event485ExtSensorAlarmDeal(&pSBizEventPara->sBizAlaStatus);
			//printf("*********EventSensorAlarmDeal ret = %d\n", ret);
			break;
		case EM_BIZ_EVENT_ALARM_IPCEXT:
			//yaogang modify 20141020
			if(pSBizEventPara->sBizAlaStatus.nCurStatus)//移动侦测添加信息显示
			{
				//printf("%s yg EM_BIZ_EVENT_ALARM_IPCEXT\n", __func__);
				AlarmListAdd(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_ALARM_IPCEXT, 1);
				/*
				SSG_MSG_TYPE msg;
				msg.type = EM_DVR_ALARM_EXT;
				msg.chn = pSBizEventPara->sBizAlaStatus.nChn;
				//msg.alm_info.type = EM_SG_ALARM_IPCEXT;
				//msg.alm_info.status = 1;
				strcpy(msg.note, GetParsedString("&CfgPtn.IPCExtAlarm"));
				upload_sg(&msg);
				*/
				
			}
			else
			{
			/*
			//报警解除EM_DVR_ALARM_RESUME AlarmRemmove
				SSG_MSG_TYPE msg;
				msg.type = EM_DVR_ALARM_RESUME;
				msg.chn = pSBizEventPara->sBizAlaStatus.nChn;
				strcpy(msg.note, GetParsedString("&CfgPtn.AlarmRemmove"));
				upload_sg(&msg);
				*/
				AlarmListDelete(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_ALARM_IPCEXT, 0);
			}
			
			bizData_SendMail(pSBizEventPara);
			ret = EventIPCExtSensorAlarmDeal(&pSBizEventPara->sBizAlaStatus);
			//printf("*********EventSensorAlarmDeal ret = %d\n", ret);
			break;
		case EM_BIZ_EVENT_ALARM_DISK_LOST:
			//yaogang modify 20141020
			if(pSBizEventPara->sBizAlaStatus.nCurStatus)//添加信息显示
			{
				//printf("%s yg EM_BIZ_EVENT_ALARM_DISK_LOST\n", __func__);
				AlarmListAdd(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_DISK_LOST, 1);
				//BizGUiWriteLogWithChn(pSBizEventPara->sBizAlaStatus.nChn, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_DISK_LOST);
				//yaogang modify 20141128
				/*
				在这里是无法写日志的，原因如下:
				s32 ModWriteLog(SLogInfo* psLogInfo)//向下会进入该函数
				{
					......
					if(g_formating || g_HotPlugFlag)	
					{
						PRINT("formatting or without_atx or hotplug\n");
						return -1;
					}
					......
				}
				*/
				
				SSG_MSG_TYPE msg;
				msg.type = EM_DVR_HDD_ERR;
				msg.chn = pSBizEventPara->sBizAlaStatus.nChn+1;
				//msg.alm_info.type = EM_SG_ALARM_DISK_LOST;
				//msg.alm_info.status = 1;
				strcpy(msg.note, GetParsedString("&CfgPtn.DiskLost"));
				upload_sg(&msg);
				
			}
			else
			{
				AlarmListDelete(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_DISK_LOST, 0);
			}
			
			bizData_SendMail(pSBizEventPara);
			
			break;
		case EM_BIZ_EVENT_ALARM_DISK_ERR:
			//yaogang modify 20141020
			if(pSBizEventPara->sBizAlaStatus.nCurStatus)//添加信息显示
			{
				//printf("%s yg EM_BIZ_EVENT_ALARM_DISK_ERR\n", __func__);
				AlarmListAdd(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_DISK_WRERR, 1);
				BizGUiWriteLogWithChn(pSBizEventPara->sBizAlaStatus.nChn, BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_DISK_WRERR);
				
				SSG_MSG_TYPE msg;
				msg.type = EM_DVR_HDD_ERR;
				msg.chn = pSBizEventPara->sBizAlaStatus.nChn+1;
				//msg.alm_info.type = EM_SG_ALARM_DISK_WRERR;
				//msg.alm_info.status = 1;
				strcpy(msg.note, GetParsedString("&CfgPtn.DiskErr"));
				upload_sg(&msg);
				
			}
			else
			{
				AlarmListDelete(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_DISK_WRERR, 0);
			}
			
			bizData_SendMail(pSBizEventPara);
			//ret = EventIPCExtSensorAlarmDeal(&pSBizEventPara->sBizAlaStatus);
			//printf("*********EventSensorAlarmDeal ret = %d\n", ret);
			break;
		case EM_BIZ_EVENT_ALARM_DISK_NONE:
			//yaogang modify 20141020
			if(pSBizEventPara->sBizAlaStatus.nCurStatus)//添加信息显示
			{
				//printf("%s yg EM_BIZ_EVENT_ALARM_DISK_NONE\n", __func__);
				AlarmListAdd(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_DISK_NONE, 1);
				//BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_DISK_NONE);
				
				SSG_MSG_TYPE msg;
				msg.type = EM_DVR_HDD_ERR;
				msg.chn = pSBizEventPara->sBizAlaStatus.nChn+1;
				//msg.alm_info.type = EM_SG_ALARM_DISK_NONE;
				//msg.alm_info.status = 1;
				strcpy(msg.note, GetParsedString("&CfgPtn.DiskNone"));
				upload_sg(&msg);
				
			}
			else
			{
				AlarmListDelete(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_DISK_NONE, 0);
			}
			
			bizData_SendMail(pSBizEventPara);
			//ret = EventIPCExtSensorAlarmDeal(&pSBizEventPara->sBizAlaStatus);
			//printf("*********EventSensorAlarmDeal ret = %d\n", ret);
			break;
		case EM_BIZ_EVENT_ALARM_IPCCOVER:
			//yaogang modify 20141020
			if(pSBizEventPara->sBizAlaStatus.nCurStatus)//移动侦测添加信息显示
			{
				//printf("%s yg EM_BIZ_EVENT_ALARM_IPCCOVER\n", __func__);
				AlarmListAdd(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_ALARM_IPCCOVER, 1);
				
				//SSG_MSG_TYPE msg;
				//msg.type = EM_DVR_ALARM_EXT;
				//msg.chn = pSBizEventPara->sBizAlaStatus.nChn;
				//msg.alm_info.type = EM_SG_ALARM_IPCCOVER;
				//msg.alm_info.status = 1;
				//strcpy(msg.note, GetParsedString("&CfgPtn.IPCCoverAlarm"));
				//upload_sg(&msg);
				
			}
			else
			{
				AlarmListDelete(pSBizEventPara->sBizAlaStatus.nChn, EM_ALRM_ALARM_IPCCOVER, 0);
			}
			
			bizData_SendMail(pSBizEventPara);
			ret = EventIPCCoverAlarmDeal(&pSBizEventPara->sBizAlaStatus);
			//printf("*********EventSensorAlarmDeal ret = %d\n", ret);
			break;
		case EM_BIZ_EVENT_ENCODE_GETRASTER:
			//break;
			{
				u8* pBuffer = pSBizEventPara->sBizRaster.pBuffer;
				int w = pSBizEventPara->sBizRaster.w;
				int h = pSBizEventPara->sBizRaster.h;
				s8* str = pSBizEventPara->sBizRaster.strOsd;
				int len = strlen(str);
				//static u32 tmp = 0;
				
				if(nFitstUseDC)
				{
					sem_wait(&semForDC);
				}
				
				nFitstUseDC = 0;
				
				while(NULL == pCdc) usleep(10 * 1000);
				
				//#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
				//#else
				pCdc->SetBkMode(BM_CROCHET);
				//#endif
				
				#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
				pCdc->SetFont(VD_RGB(255,255,255), FS_NORMAL);
				//pCdc->SetFont(VD_RGB(255,255,255), FS_OUTLINE);
				#else
				//pCdc->SetBkColor(VD_RGB(255,255,0));
				pCdc->SetFont(VD_RGB(255,255,255), FS_BOLD);
				#endif
				
				/*
				if (('m' == *(str + strlen(str) - 1)) || ('M' == *(str + strlen(str) - 1)))
				{
					w += 24;
				}
				*/
				
				pCdc->StringToBitmap(pBuffer, w, h, str, len);
			}
			break;
		//case EM_BIZ_EVENT_FORMAT_INIT:
		//{
		//
		//}
		//break;
		case EM_BIZ_EVENT_SHOWTIME_REFRESH:
			{
				//printf("fresh time\n");
				EventShowTimeFresh();
			}
			break;
		case EM_BIZ_EVENT_LIVE_REFRESH:
			{
				int ch = pSBizEventPara->sBizRecStatus.nChn;
				EventLiveFreshDeal(ch);
			}
			break;
		case EM_BIZ_EVENT_POWEROFF_MANUAL:
		{
			CRect infoRt = CRect(0, 0, 480, 300/*370*/);
			char tmpchar[20] = {0};
			GetProductNumber(tmpchar);
			if((0 == strcasecmp(tmpchar, "R9632S"))
				||(0 == strcasecmp(tmpchar, "R9624T"))
				||(0 == strcasecmp(tmpchar, "R9624SL"))
				||(0 == strcasecmp(tmpchar, "R9616S"))
				||(0 == strcasecmp(tmpchar, "R9608S")))
			{
				CPageInfoBox infoBox2(&infoRt/*&recordRt*/, "&CfgPtn.Info"/*"&titles.mainmenu"*/,"&CfgPtn.Shutdownthesystem", NULL, NULL);
				infoBox2.Open();
			}
			else
			{
				/*
				const char* szInfo = GetParsedString("&CfgPtn.Nowyoucanshutdownsafely");
				const char* szType = GetParsedString("&CfgPtn.WARNING");
				UDM ret= MessageBox(szInfo, szType , MB_NULL|MB_ICONINFORMATION|MB_DEFBUTTON2,TRUE);
				*/
				CPageInfoBox infoBox(&infoRt/*&recordRt*/, "&CfgPtn.Info"/*"&titles.mainmenu"*/,"&CfgPtn.Nowyoucanshutdownsafely", NULL, NULL);
				infoBox.Open();
			}
		} break;
		default:
		break;
	}
	
	//以下命令需直接判断页面是否创建完毕
	if(!CheckGpageExit())//cw_page
	{
		return 0;
	}
			
	switch(pSBizEventPara->emType)
	{
		case EM_BIZ_EVENT_NET_CHANGEPARA_RESET:
		{
			//g_pPages[EM_PAGE_DESKTOP]->Show(TRUE);
			
			//说明:	infoBox除了这个作用域就会被析构,可能会造成问题
			CRect infoRt = CRect(0, 0, 480, 300);
			CPageInfoBox infoBox(&infoRt/*&recordRt*/, "&CfgPtn.Info"/*"&titles.mainmenu"*/,"&CfgPtn.ChangeParaReset", NULL, NULL);
			infoBox.Open();
		} break;
		case EM_BIZ_EVENT_GETDMINFO:
		{
			if(g_pPages &&  g_pPages[EM_PAGE_DISKMGR] && g_pPages[EM_PAGE_BACKUP])
			{
				((CPageDiskMgr*)g_pPages[EM_PAGE_DISKMGR])->SetInfoProg(
					pSBizEventPara->sDmInfo.nType,
					pSBizEventPara->sDmInfo.szMountPoint,
					pSBizEventPara->sDmInfo.nTotal,
					pSBizEventPara->sDmInfo.nFree
				);
				if(pSBizEventPara->sDmInfo.nType) // usb
				{
					((CPageBackupInfo*)g_pPages[EM_PAGE_BACKUP])->SetUdskInfo(
						pSBizEventPara->sDmInfo.nTotal,
						pSBizEventPara->sDmInfo.nFree
					);
				}
			}
		} break;
		case EM_BIZ_EVENT_SATARELOAD:
		{
			BizReloadHdd(&pSBizEventPara->sStoreMgr);
		} break;
		case EM_BIZ_EVENT_DISKCHANGED:
		{
			if(g_pPages && g_pPages[EM_PAGE_DISKMGR])
			{
				((CPageDiskMgr*)g_pPages[EM_PAGE_DISKMGR])->NotifyDiskChanged(&pSBizEventPara->sStoreMgr);
			}
		} break;
		case EM_BIZ_EVENT_FORMAT_RUN:
			{
				if(g_pPages)
				{
					if(pSBizEventPara->emBizResult != 0)
					{
						MessageBox("&CfgPtn.FormatFail", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
						((CPageDiskMgr*)g_pPages[EM_PAGE_DISKMGR])->SetFormatProg(0);						
					}
					else
					{
						s32 nProg = pSBizEventPara->sBizProgress.nProgress;
						((CPageDiskMgr*)g_pPages[EM_PAGE_DISKMGR])->SetFormatProg(nProg);
					}
				}
			}
			break;
		case EM_BIZ_EVENT_FORMAT_DONE:
			{
				/*((CPageDiskMgr*)g_pPages[EM_PAGE_DISKMGR])->SetStateInfo("Format done!");
				usleep(1000*1000);
				((CPageDiskMgr*)g_pPages[EM_PAGE_DISKMGR])->SetStateInfo("");*/
			}
			break;
		case EM_BIZ_EVENT_BACKUP_RUN:
			{
				if(g_pPages)
				{
					s32 nProg = pSBizEventPara->sBizProgress.nProgress;
					((CPageBackupInfo*)g_pPages[EM_PAGE_BACKUP])->SetBakProg(nProg);
				}
			}
			break;
		case EM_BIZ_EVENT_BACKUP_DONE:
			{
				SetSystemLockStatus(0);//cw_backup
				if(g_pPages)
				{
					if(pSBizEventPara->emBizResult == EM_BIZBACKUP_SUCCESS)
					{
						((CPageBackupInfo*)g_pPages[EM_PAGE_BACKUP])->SetBakProg(100);
					}
					else
					{
						if(pSBizEventPara->emBizResult == EM_BIZBACKUP_FAILED_NOSPACE)
						{
							MessageBox("&CfgPtn.BackupNoSpace", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
						}
						else
							((CPageBackupInfo*)g_pPages[EM_PAGE_BACKUP])->SetBakProg(-1);
					}
				}
			}
			break;
		case EM_BIZ_EVENT_UPGRADE_RUN:
			{
				//printf("%s EM_BIZ_EVENT_UPGRADE_RUN g_pPages: %d\n", __func__, g_pPages);
				if(g_pPages)
				{
					s32 nProg = pSBizEventPara->sBizProgress.nProgress;
					((CPageUpdate*)g_pPages[EM_PAGE_UPDATE])->SetUpdateProg(nProg);
				}
			}
			break;
		case EM_BIZ_EVENT_UPGRADE_DONE:
			{
				//printf("%s EM_BIZ_EVENT_UPGRADE_DONE g_pPages: %d, emBizResult: %d\n", \
				//	__func__, g_pPages, pSBizEventPara->emBizResult);
				if(g_pPages)
				{
					if(pSBizEventPara->emBizResult==EM_BIZUPGRADE_SUCCESS)
						((CPageUpdate*)g_pPages[EM_PAGE_UPDATE])->SetUpdateProg(100);
					else
						((CPageUpdate*)g_pPages[EM_PAGE_UPDATE])->SetUpdateProg(0);
				}
			}
			break;
		case EM_BIZ_EVENT_PLAYBACK_RUN:
			{
				if(g_pPages)
				{
					s32 nProg = pSBizEventPara->sBizProgress.nProgress;
					time_t nCurTime = (time_t)(pSBizEventPara->sBizProgress.lCurSize/1000);
					u32 nTotal = pSBizEventPara->sBizProgress.lTotalSize;
					
					if(0 == count)
					{
						((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->SetPbTotalTime(nTotal);
						((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->SetPbStartTime(nCurTime);
						nStartTime = nCurTime;
						count++;
					}
					
					u32 nNow = nCurTime - nStartTime;
					
					if(nProg != lastProg)
					{
						((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->SetPbProg(nProg);
						lastProg = nProg;
					}
					
					u8 flag = 0;
					if(nCurTime != lastCurTime)
					{
						struct tm tagTime;
						localtime_r(&nCurTime, &tagTime);
						
						char szTime[32] = {0};
						sprintf(szTime,"%04d/%02d/%02d %02d:%02d:%02d",
											     tagTime.tm_year+1900,tagTime.tm_mon+1, tagTime.tm_mday,
												 tagTime.tm_hour, tagTime.tm_min,tagTime.tm_sec);
						
						((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->SetPbCurTime(szTime);
		 				lastCurTime = nCurTime;
						flag = 1;
					}
					
					if((nTotal != lastTotal) || flag)
					{
						char szTotal[50] = {0};
						sprintf(szTotal,"%02d:%02d:%02d/%02d:%02d:%02d",nNow/3600,(nNow%3600)/60,(nNow%3600)%60,nTotal/3600,(nTotal%3600)/60,(nTotal%3600)%60);
						((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->SetPbTotalTime(szTotal);
						lastTotal = nTotal;
					}
				}
			}
			break;
		case EM_BIZ_EVENT_PLAYBACK_DONE:
			{
				if(g_pPages)
				{
					printf("EM_BIZ_EVENT_PLAYBACK_DONE...\n");
					
					//((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->m_Mutex.Enter();//csp modify 20121118
					
					((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->SetPBFinished(1);//csp modify 20121118
					
					count = 0;
					
					if(((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->GetZoomStatue())
					{
						//printf("EM_BIZ_EVENT_PLAYBACK_DONE-1.1\n");
						((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->UpdateData(UDM_EMPTY);
						((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->SetZoomStatue(0);
						//printf("EM_BIZ_EVENT_PLAYBACK_DONE-1.2\n");
					}
					else
					{
						//printf("EM_BIZ_EVENT_PLAYBACK_DONE-2.1\n");
						if(((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->IsOpened())
						{
							//printf("EM_BIZ_EVENT_PLAYBACK_DONE-2.11\n");
							((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->Close();
							//printf("EM_BIZ_EVENT_PLAYBACK_DONE-2.12\n");
						}
						else
						{
							//printf("EM_BIZ_EVENT_PLAYBACK_DONE-2.21\n");
							((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->UpdateData(UDM_EMPTY);
							((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->SetZoomStatue(0);
							//printf("EM_BIZ_EVENT_PLAYBACK_DONE-2.22\n");
						}
						//printf("EM_BIZ_EVENT_PLAYBACK_DONE-2.2\n");
					}
					
					//((CPagePlayBackFrameWork*)g_pPages[EM_PAGE_PLAYBACK])->m_Mutex.Leave();//csp modify 20121118
				}
			}
			break;
		case EM_BIZ_EVENT_NET_STATE_DHCP:
		{
			if(g_pPages)
			{
				SBizNetPara sPIns;
				if(0 == BizNetReadCfg(&sPIns))
				{
					((CPageNetworkConfig*)g_pPages[EM_PAGE_NETCFG])->SetDynNetPara(
						sPIns.HostIP, sPIns.Submask, sPIns.GateWayIP, sPIns.DNSIP
					);
					
					printf("bizData_NetStateNotify EM_NET_STATE_DHCP ok!\n");
				}
				
				((CPageNetworkConfig*)g_pPages[EM_PAGE_NETCFG])->SetNetStatus(
					0,
					pSBizEventPara->emType==EM_BIZ_EVENT_NET_STATE_DHCP?0:1,
					pSBizEventPara->emBizResult?1:0);
			}
		} break;
		case EM_BIZ_EVENT_NET_STATE_DHCP_STOP:
		{
			((CPageNetworkConfig*)g_pPages[EM_PAGE_NETCFG])->SetNetStatus(
				0,
				pSBizEventPara->emType==EM_BIZ_EVENT_NET_STATE_DHCP?0:1,
				pSBizEventPara->emBizResult?1:0);
		} break;
		case EM_BIZ_EVENT_NET_STATE_PPPOE: // 1
		case EM_BIZ_EVENT_NET_STATE_PPPOE_STOP: // 1
		{
			if(g_pPages)
			{
				((CPageNetworkConfig*)g_pPages[EM_PAGE_NETCFG])->SetNetStatus(
					1, 
					pSBizEventPara->emType==EM_BIZ_EVENT_NET_STATE_PPPOE?0:1,
					pSBizEventPara->emBizResult?1:0
				);
			}
		} break;
		case EM_BIZ_EVENT_NET_STATE_DDNS:  // 3
		case EM_BIZ_EVENT_NET_STATE_DDNS_STOP:  // 3
		{
			if(g_pPages)
			{
				((CPageNetworkConfig*)g_pPages[EM_PAGE_NETCFG])->SetNetStatus(
				3, 
				pSBizEventPara->emType==EM_BIZ_EVENT_NET_STATE_DDNS?0:1,
				pSBizEventPara->emBizResult?1:0);
			}
		} break;
		case EM_BIZ_EVENT_NET_STATE_MAIL:  // 2
		{
			if(g_pPages)
			{
				((CPageNetworkConfig*)g_pPages[EM_PAGE_NETCFG])->SetNetStatus(2, 0, pSBizEventPara->emBizResult?1:0);
			}
		} break;
		case EM_BIZ_EVENT_NET_STATE_CONN: // 4
		{
			;
		} break;
		case EM_BIZ_EVENT_NET_STATE_UPDATEMAINBOARDSTART:
		case EM_BIZ_EVENT_NET_STATE_UPDATEPANNELSTART:
		case EM_BIZ_EVENT_NET_STATE_UPDATESTARTLOGOSTART:
		case EM_BIZ_EVENT_NET_STATE_UPDATEAPPLOGOSTART:
		{
			//printf("%s emType: %d, 1\n", \
			//	__func__, pSBizEventPara->emType);
			if(GetPlayBakStatus()==1)
			{
				//printf("%s emType: %d, 2\n", \
				//	__func__, pSBizEventPara->emType);
				CPage** page = GetPage();
				((CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK])->StopPb();
			}
			if(0 == pSBizEventPara->emBizResult)
			{
				//printf("%s emType: %d, 3\n", \
				//	__func__, pSBizEventPara->emType);
				((CPageUpdate*)g_pPages[EM_PAGE_UPDATE])->CloseAll();
			}
			//printf("%s emType: %d, 4\n", \
			//	__func__, pSBizEventPara->emType);
			((CPageUpdate*)g_pPages[EM_PAGE_UPDATE])->Open();
			((CPageUpdate*)g_pPages[EM_PAGE_UPDATE])->RemoteUpdateStart((u32)pSBizEventPara->emType, (u32)pSBizEventPara->emBizResult);
		} break;
		case EM_BIZ_EVENT_NET_STATE_FORMATSTART:
		{
			if(0 == pSBizEventPara->sBizFormat.emBizResult)
			{			
				((CPageDiskMgr*)g_pPages[EM_PAGE_DISKMGR])->CloseAll();
			}
			
			((CPageDiskMgr*)g_pPages[EM_PAGE_DISKMGR])->Open();
			((CPageDiskMgr*)g_pPages[EM_PAGE_DISKMGR])->RemoteFormatStart(pSBizEventPara->sBizFormat.nDiskIdx, (int)pSBizEventPara->sBizFormat.emBizResult);			
		} break;
		case EM_BIZ_EVENT_NET_STATE_SGUPLOAD:
		{
			//printf("%s emBizResult: %d\n", __func__, (u32)pSBizEventPara->emBizResult);
			u32 tmp = (u32)pSBizEventPara->emBizResult;
			u8 chn, ErrVal;
			chn = (tmp >> 8) & 0xff;
			ErrVal = tmp & 0xff;
				
			((CPageShenGuangConfig*)g_pPages[EM_PAGE_SG_PLATFORM])->Open();
			((CPageShenGuangConfig*)g_pPages[EM_PAGE_SG_PLATFORM])->RecvNotify(chn, ErrVal);
			//((CPageUpdate*)g_pPages[EM_PAGE_SG_PLATFORM])->RemoteUpdateStart((u32)pSBizEventPara->emType, (u32)pSBizEventPara->emBizResult);
		}break;
		default:
		break;
	}
	
	return 0;
}

static u8 g_nSystemLocked = 0;

extern sem_t MouseSem;//cw_lock

void SetSystemLockStatus(u8 flag)
{
	g_nSystemLocked = flag ? 1 : 0;
	
	#if 1
	if(flag==1)
	{
		if(-1 == sem_wait(&MouseSem))
		{
			printf("locked error\n");
		}
	}
	else if(flag==0)
	{
		int value = -1;
		sem_getvalue(&MouseSem, &value);
		if(value == 0)
		{
			if(-1==sem_post(&MouseSem))
			{
				printf("unlocked error\n");
			}
		}		
	}
	#endif
}

void GetSystemLockStatus(u8* flag)
{
	if(flag)
	{
		*flag = g_nSystemLocked;
	}
}


#define nMaxListLen 20
#define nMaxStrLen 64

s8* g_strList[nMaxListLen];
s8 strList[nMaxListLen][nMaxStrLen] = {{0}};

void InitStrList()
{
	BIZ_DATA_DBG("InitStrList  start\n");
	int i = 0;
	for (i = 0; i < nMaxListLen; i++)
	{
		g_strList[i] = strList[i];
		//printf("pstrList[%d]=%p\n", i, g_strList[i]);
	}

	BIZ_DATA_DBG("InitStrList  finish\n");
}

void GetVideoFormatList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		//获得 配置 
		u8 real = 0;
		BIZ_DATA_DBG("GetVideoFormatList1\n");
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_VIDEOSTANDARD, g_strList, (u8*)&real, 20, 64);
		BIZ_DATA_DBG("GetVideoFormatList2\n");
		*nRealNum = real;
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetVideoFormatList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nVideoFormat[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}	
	}
}


int nMaxUser[4] = 
{
	1,5,10,20
};

const char* szMaxUser[4] = 
{
	"1","5","10","20"
};

void GetMaxUserList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	if (psValueList)
	{

		//获得 配置
		
		*nRealNum = 4;

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			psValueList[i].nValue = nMaxUser[i];
			strcpy(psValueList[i].strDisplay,szMaxUser[i]);
		}
	}
}

void GetOutputList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_OUTPUT, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;

		if(ret!=0)
		{
			BIZ_DATA_DBG("GetOutputList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nOutput[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}
	}
}
//yaogang modify 20141217 for sg platform
void GetSGSysNameList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_RepairName, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;

		if(ret!=0)
		{
			BIZ_DATA_DBG("GetOutputList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nOutput[i];
			strcpy(psValueList[i].strDisplay, g_strList[i]);
		}
	}
}
void GetSGSysTypeList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_RepairType, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;

		if(ret!=0)
		{
			BIZ_DATA_DBG("GetOutputList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nOutput[i];
			strcpy(psValueList[i].strDisplay, g_strList[i]);
		}
	}
}
void GetSGMaintainTypeList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_Maintain, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;

		if(ret!=0)
		{
			BIZ_DATA_DBG("GetOutputList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nOutput[i];
			strcpy(psValueList[i].strDisplay, g_strList[i]);
		}
	}
}
void GetSGTestTypeList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_Test, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;

		if(ret!=0)
		{
			BIZ_DATA_DBG("GetOutputList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nOutput[i];
			strcpy(psValueList[i].strDisplay, g_strList[i]);
		}
	}
}

//yaogang modify 20141025
void GetDWellSwitchPictureList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_SWITCHPICTURE, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;

		if(ret!=0)
		{
			BIZ_DATA_DBG("GetOutputList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nOutput[i];
			strcpy(psValueList[i].strDisplay, g_strList[i]);
		}
	}
}


void GetLanguageList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if(psValueList)
	{
		//csp modify 20121224//大于20种语言怎么办???
		u8 real = 0;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_LANGSUP, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;
		
		printf("GetLanguageList num : %d\n",*nRealNum);
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetLanguageList\n");
		}
		
		if(*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}
		
		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nLanguage[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
			//printf("GetLanguageList name : %s\n",g_strList[i]);
		}
	}
}

int nScreenSave[5] = 
{
	-1,30,60,180,300
};

const char* szScreenSave[5] = 
{
	"&CfgPtn.Never","30","60","3 X 60","5 X 60"
};

void GetScreenSaverList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	if(psValueList)
	{
		//获得 配置 
		*nRealNum = 5;
		
		if(*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}
		
		for(int i=0; i<*nRealNum; i++)
		{
			psValueList[i].nValue = nScreenSave[i];
			strcpy(psValueList[i].strDisplay,szScreenSave[i]);
		}
	}
}

//csp modify
int nLockTime[8] = 
{
	0,5,10,30,60,300,600,1800
};

const char* szLockTime[8] = 
{
	"&CfgPtn.NeverLock","&CfgPtn.5s","&CfgPtn.10s","&CfgPtn.30s","&CfgPtn.1m","&CfgPtn.5m","&CfgPtn.10m","&CfgPtn.30m"
};

void GetLockTimeList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	if(psValueList)
	{
		//获得 配置 
		*nRealNum = 8;
		
		if(*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}
		
		for(int i=0; i<*nRealNum; i++)
		{
			psValueList[i].nValue = nLockTime[i];
			strcpy(psValueList[i].strDisplay,szLockTime[i]);
		}
	}
}

int nDWellTime[5] = 
{
	5,10,20,30,60
};

const char* szDWellTime[5] = 
{
	"&CfgPtn.5s","&CfgPtn.10s","&CfgPtn.20s","&CfgPtn.30s","&CfgPtn.60s"
};

void GetDWellSwitchTimeList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	if(psValueList)
	{
		//获得 配置 
		*nRealNum = 5;
		
		if(*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}
		
		for(int i=0; i<*nRealNum; i++)
		{
			psValueList[i].nValue = nDWellTime[i];
			strcpy(psValueList[i].strDisplay,szDWellTime[i]);
		}
	}
}

//返回comboBox  index
int GetVideoFormat()
{
	//return 1;

	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetVideoFormat\n");
	}
	
	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_VIDEOSTANDARD, 
							bizSysPara.nVideoStandard, 
							&index);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetVideoFormat\n");
	}
	
	return index;
}

//返回实际制式
int GetVideoFormatReal()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetVideoFormat\n");
		return EM_BIZ_PAL;
	}
	
	return (int)bizSysPara.nVideoStandard;
}

int GetMaxUser()
{
	return 2;
}

int GetOutput()
{
	//return 0;
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetOutput\n");
	}
	
	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_OUTPUT, 
							bizSysPara.nOutput, 
							&index);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetOutput\n");
	}
	
	return index;
}

void GetVgaResolution(int* w, int* h)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	
	if(0 == BizGetPara(&bizTar, &bizSysPara))
	{
		switch(bizSysPara.nOutput)
		{
			case EM_BIZ_OUTPUT_VGA_1024X768:
				*w = 1024;
				*h = 768;
				break;
			case EM_BIZ_OUTPUT_VGA_1280X1024:
				*w = 1280;
				*h = 1024;
				break;
			case EM_BIZ_OUTPUT_VGA_800X600:
				*w = 800;
				*h = 600;
				break;
			case EM_BIZ_OUTPUT_HDMI_1920X1080:
				*w = 1920;
				*h = 1080;
				break;
			case EM_BIZ_OUTPUT_HDMI_1280X720:
				*w = 1280;
				*h = 720;
				break;
			case EM_BIZ_OUTPUT_CVBS:
				//csp modify
				*w = 800;
				*h = 600;
				//*w = 720;
				//*h = (EM_BIZ_PAL == bizSysPara.nVideoStandard) ? 576 : 480;
				break;
			default:
				*w = 800;
				*h = 600;
				break;
		}
	}
}

int GetLanguage()
{
	//return 1;
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetLanguage\n");
		return 0;
	}
/*
	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_LANGSUP, 
							bizSysPara.nLangId, 
							&index);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetLanguage\n");
	}
	
	return index;
*/
	return bizSysPara.nLangId;
}

int GetLanguageIndex()
{
	//return 1;
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetLanguage\n");
		return 0;
	}
	
	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_LANGSUP, 
							bizSysPara.nLangId, 
							&index);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetLanguage\n");
	}
	
	return index;
}

//csp modify
int GetLockTimeIndex()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetLockTimeIndex\n");
		return 0;
	}
	
	int i;
	for(i=0;i<8;i++)
	{
		if(bizSysPara.nLockTime == nLockTime[i])
		{
			return i;
		}
	}
	
	return 0;
}

int SetLockTimeIndex(int index)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("SetLockTimeIndex\n");
		return 0;
	}
	
	if(index >= 8 || index < 0)
	{
		index = 0;
	}
	
	bizSysPara.nLockTime = nLockTime[index];
	
	ret = BizSetPara(&bizTar, &bizSysPara);
	
	return ret;
}

int GetDWellSwitchPictureIndex()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PATROLPARA;
	SBizCfgPatrol bizPara;
	int ret = BizGetPara(&bizTar, &bizPara);
	if(ret!=0)
	{
		return 0;
	}
	
    //return 1;
	
    switch(bizPara.nPatrolMode)// 1 4 9 16轮巡画面
    {
        case 1:
        {
            ret = 0;
        } break;
        case 4:
        {
            ret = 1;
        } break;
        case 9:
        {
            ret = 2;
        } break;
	case 16:
        {
            ret = 3;
        } break;
        default:
        {
            ret = 0;
        } break;
    }
	
    return ret;
}
int SetDWellSwitchPictureIndex(int index)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PATROLPARA;
	SBizCfgPatrol bizPara;
	int ret = BizGetPara(&bizTar, &bizPara);
	if(ret!=0)
	{
		return 0;
	}
	
    //return 1;
	
    switch(index)
    {
        case 0:
        {
            bizPara.nPatrolMode = 1;
        } break;
        case 1:
        {
            bizPara.nPatrolMode = 4;
        } break;
        case 2:
        {
            bizPara.nPatrolMode = 9;
        } break;
	case 3:
        {
            bizPara.nPatrolMode = 16;
        } break;
        default:
        {
            bizPara.nPatrolMode = 1;
        } break;
    }

    ret = BizSetPara(&bizTar, &bizPara);

    return ret;
}


int GetDWellSwitchTimeIndex()
{
    SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PATROLPARA;
	SBizCfgPatrol bizPara;
	int ret = BizGetPara(&bizTar, &bizPara);
	if(ret!=0)
	{
		return 0;
	}
	
    //return 1;
	
    switch(bizPara.nInterval)
    {
        case 5:
        {
            ret = 0;
        } break;
        case 10:
        {
            ret = 1;
        } break;
        case 20:
        {
            ret = 2;
        } break;
        case 30:
        {
            ret = 3;
        } break;
        case 60:
        {
            ret = 4;
        } break;
        default:
        {
            ret = 0;
        } break;
    }
	
    return ret;
}

int SetDWellSwitchTimeIndex(int index)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PATROLPARA;
	SBizCfgPatrol bizPara;
	int ret = BizGetPara(&bizTar, &bizPara);
	if(ret!=0)
	{
		return 0;
	}
	
    //return 1;
	
    switch(index)
    {
        case 0:
        {
            bizPara.nInterval = 5;
        } break;
        case 1:
        {
            bizPara.nInterval = 10;
        } break;
        case 2:
        {
            bizPara.nInterval = 20;
        } break;
        case 3:
        {
            bizPara.nInterval = 30;
        } break;
        case 4:
        {
            bizPara.nInterval = 60;
        } break;
        default:
        {
            bizPara.nInterval = 5;
        } break;
    }

    ret = BizSetPara(&bizTar, &bizPara);

    return ret;
}

int GetScreenSaver()
{
	return 0;
}

void GetDevId(char* pID, int nMaxLen)
{
	//strncpy(pID,"0",nMaxLen);
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetDevId\n");
	}
	
	u32 devId = bizSysPara.nDevId;
	char pdevid[nMaxLen]; //zlb20111116 原变量是malloc方式
	sprintf(pdevid, "%d", devId);
	strncpy(pID,pdevid,nMaxLen);
}

static char devID[32] = {0};
void GetDevId0(char* pID, int nMaxLen)
{
	strncpy(pID,devID,nMaxLen);
}

//获取软件版本号型号
s32 GetDvrVersionModel(u32 flag, char *sDvr_info,  char *sDvr_Model)
{
	BizConfigGetDvrInfo(0,sDvr_info, sDvr_Model);
}

void GetDevName(char* pName, int nMaxLen)
{
	//strncpy(pName,"EDVR",nMaxLen);
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetDevName\n");
	}
	
	strncpy(pName,bizSysPara.strDevName,nMaxLen);
}

int GetAuthCheck()
{
	//return 1;
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetAuthCheck\n");
	}
	
	return bizSysPara.nAuthCheck;
}
	
int GetTimeDisplayCheck()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	int ret = BizGetPara(&bizTar, &bizPreCfg);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeDisplayCheck\n");
	}
	
	return bizPreCfg.nShowTime;
}
//yaogang modify 20140918
int GetChnKbpsDisplayCheck()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	int ret = BizGetPara(&bizTar, &bizPreCfg);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetChnKbpsDisplayCheck\n");
	}
	
	return bizPreCfg.nShowChnKbps;
}


int GetShowGuide()
{
	//return 1;
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetGuideCheck\n");
	}
	
	return bizSysPara.nShowGuide;
}

//返回comboBox  index
int GetVideoFormatDefault()
{
	//return 1;

	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetVideoFormat\n");
	}

	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_VIDEOSTANDARD, 
							bizSysPara.nVideoStandard, 
							&index);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetVideoFormat\n");
	}
	
	return index;
}


int GetMaxUserDefault()
{
	return 2;

}

int GetOutputDefault()
{
	//return 0;
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetOutput\n");
	}

	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_OUTPUT, 
							bizSysPara.nOutput, 
							&index);

	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetOutput\n");
	}

	return index;

}

int GetLanguageDefault()
{
	//return 1;

	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);

	if(ret!=0)
	{
		printf("GetLanguage DefaultPara Error!\n");
	}

	u8 index = 0;
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_LANGSUP, 
							bizSysPara.nLangId, 
							&index);

	if(ret!=0)
	{
		printf("GetLanguage DefaultIndex Error!\n");
	}

	return index;

}

int GetScreenSaverDefault()
{
	return 0;
}

int GetShowGuideDefault()
{
    //return 1;
    SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetGuideCheck\n");
	}

	return bizSysPara.nShowGuide;
}

//csp modify
int GetLockTimeIndexDefault()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetLockTimeIndexDefault\n");
		return 0;
	}
	
	int i;
	for(i=0;i<8;i++)
	{
		if(bizSysPara.nLockTime == nLockTime[i])
		{
			return i;
		}
	}
	
	return 0;
}
int GetDWellSwitchPictureDefault()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PATROLPARA;
	SBizCfgPatrol bizPara;
	int ret = BizGetDefaultPara(&bizTar, &bizPara);
	if(ret!=0)
	{
		return 0;
	}
	
    //return 1;
	
     switch(bizPara.nPatrolMode)// 1 4 9 16轮巡画面
    {
        case 1:
        {
            ret = 0;
        } break;
        case 4:
        {
            ret = 1;
        } break;
        case 9:
        {
            ret = 2;
        } break;
	case 16:
        {
            ret = 3;
        } break;
        default:
        {
            ret = 0;
        } break;
    }

    return ret;
}

int GetDWellSwitchTimeDefault()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PATROLPARA;
	SBizCfgPatrol bizPara;
	int ret = BizGetDefaultPara(&bizTar, &bizPara);
	if(ret!=0)
	{
		return 0;
	}
	
    //return 1;
	
    switch(bizPara.nInterval)
    {
        case 5:
        {
            ret = 0;
        } break;
        case 10:
        {
            ret = 1;
        } break;
        case 20:
        {
            ret = 2;
        } break;
        case 30:
        {
            ret = 3;
        } break;
        case 60:
        {
            ret = 4;
        } break;
        default:
        {
            ret = 0;
        } break;
    }

    return ret;
}

void GetDevIdDefault(char* pID, int nMaxLen)
{
	//strncpy(pID,"0",nMaxLen);
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetDevId\n");
	}

	u32 devId = bizSysPara.nDevId;
	char* pdevid = (char*)malloc(sizeof(char)*nMaxLen);
	sprintf(pdevid,"%d",devId);
	strncpy(pID,pdevid,nMaxLen);

	free(pdevid);
}


int GetDevNameDefault(char* pName, int nMaxLen)
{
	//strncpy(pName,"EDVR",nMaxLen);
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetDevName\n");
	}

	strncpy(pName,bizSysPara.strDevName,nMaxLen);

	return ret;
}

int GetAuthCheckDefault()
{
	//return 1;
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetAuthCheck\n");
	}

	return bizSysPara.nAuthCheck;
}
	
int GetTimeDisplayCheckDefault()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	int ret = BizGetDefaultPara(&bizTar, &bizPreCfg);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeDisplayCheck\n");
	}
	
	return bizPreCfg.nShowTime;
}
//yaogang modify 20140918
int GetChnKbpsDisplayCheckDefault()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	int ret = BizGetDefaultPara(&bizTar, &bizPreCfg);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetChnKbpsDisplayCheckDefault\n");
	}
	
	return bizPreCfg.nShowChnKbps;
}


void GetMainTainTypeList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	int i = 0;
	*nRealNum = 3;
	const char* statictext[3] = 
	{
		"&CfgPtn.DAYS",
		"&CfgPtn.WEEKLY",
		"&CfgPtn.MONTHLY",
	};
	
	for (i = 0; i < 3; i++ )
	{
		strcpy(psValueList[i].strDisplay,statictext[i]);
	}
}

void GetWeekList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	int i = 0;
	*nRealNum = 7;
	
	const char* statictext[7] = 
	{
		"&CfgPtn.Sunday",
		"&CfgPtn.Monday",
		"&CfgPtn.Tuesday",
		"&CfgPtn.Wednesday",
		"&CfgPtn.Thursday",
		"&CfgPtn.Friday",
		"&CfgPtn.Saturday",
	};

	for (i = 0; i < 7; i++ )
	{
		strcpy(psValueList[i].strDisplay,statictext[i]);
	}
}

void GetDateFormatList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	*nRealNum = 3;
	strcpy(psValueList[0].strDisplay,"&CfgPtn.YYYYMMDD");
    strcpy(psValueList[1].strDisplay,"&CfgPtn.MMDDYYYY");
    strcpy(psValueList[2].strDisplay,"&CfgPtn.DDMMYYYY");
}

void GetTimeFormatList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
#if 0
	InitStrList();
	
	if(psValueList)
	{
		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_TIMESTANDARD, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;

		if(ret!=0)
		{
			BIZ_DATA_DBG("GetTimeFormatList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nTimeFormat[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}
	}
#else
	*nRealNum = 2;
	strcpy(psValueList[0].strDisplay,"&CfgPtn.24h");
    strcpy(psValueList[1].strDisplay,"&CfgPtn.12h");
#endif
}

#if 0
int nTimeZone[9] = 
{
	-4,-3,-2,-1,0,1,2,3,4
};

char* szTimeZone[1] = 
{
	"GMT+8:00",
};
#else
int nTimeZone[25] = 
{
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24
};//QYJ

const char* szTimeZone[] = 
{
	"GMT-12:00",
	"GMT-11:00",
	"GMT-10:00",
	"GMT-09:00",
	"GMT-08:00",
	"GMT-07:00",
	"GMT-06:00",
	"GMT-05:00",
	"GMT-04:30",
	"GMT-04:00",
	"GMT-03:30",
	"GMT-03:00",
	"GMT-02:00",
	"GMT-01:00",
	"GMT",//"GMT+00:00",
	"GMT+01:00",
	"GMT+02:00",
	"GMT+03:00",
	"GMT+03:30",
	"GMT+04:00",
	"GMT+04:30",
	"GMT+05:00",
	"GMT+05:30",
	"GMT+05:45",
	"GMT+06:00",
	"GMT+06:30",
	"GMT+07:00",
	"GMT+08:00",
	"GMT+09:00",
	"GMT+09:30",
	"GMT+10:00",
	"GMT+11:00",
	"GMT+12:00",
	"GMT+13:00",
};//QYJ
#endif

void GetTimeZoneList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	printf("get time zone list\n");//QYJ
	
	if(psValueList)
	{
		//QYJ
		//for nvr ntp and sync ipcamera time
		//*nRealNum = 1;
		*nRealNum = sizeof(szTimeZone)/sizeof(szTimeZone[0]);
		
		//BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_TIMESTANDARD, g_strList, nRealNum, 20, 64);
		
		if(*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}
		
		for(int i=0; i<*nRealNum; i++)
		{
			psValueList[i].nValue = nTimeZone[i];
			strcpy(psValueList[i].strDisplay,szTimeZone[i]);
		}
	}
}

//返回comboBox index
int GetDateFormat()
{
	//return 0;
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetDateFormat\n");
	}
	
	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_DATEFORMAT, 
							bizSysPara.nDateFormat, 
							&index);

	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetDateFormat\n");
	}
	
	return index;
}

int GetTimeFormat()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeFormat\n");
	}

	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_TIMESTANDARD, 
							bizSysPara.nTimeStandard, 
							&index);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeFormat\n");
	}
	
	return index;
}

int GetDateFormatDefault()
{
	//return 0;
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetDateFormat\n");
	}
	
	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_DATEFORMAT, 
							bizSysPara.nDateFormat, 
							&index);

	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetDateFormat\n");
	}
	
	return index;
}

int GetTimeFormatDefault()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeFormat\n");
	}
	
	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_TIMESTANDARD, 
							bizSysPara.nTimeStandard, 
							&index);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeFormat\n");
	}
	
	return index;
}

int GetTimeSyncToIPC()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeSyncToIPC\n");
	}
	
	return bizSysPara.nSyncTimeToIPC;
}

int GetTimeSyncToIPCDefault()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeSyncToIPCDefault\n");
	}
	
	return bizSysPara.nSyncTimeToIPC;
}

int GetTimeZone()
{
#if 1//QYJ
	//return 0;
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeZone\n");
	}
	
	return bizSysPara.nTimeZone;
#else
	return 27;
#endif
}

static int TimeZoneOffset[] = 
{
	-12*3600,
	-11*3600,
	-10*3600,
	-9*3600,
	-8*3600,
	-7*3600,
	-6*3600,
	-5*3600,
	-4*3600-1800,
	-4*3600,
	-3*3600-1800,
	-3*3600,
	-2*3600,
	-1*3600,
	0,
	1*3600,
	2*3600,
	3*3600,
	3*3600+1800,
	4*3600,
	4*3600+1800,
	5*3600,
	5*3600+1800,
	5*3600+2700,
	6*3600,
	6*3600+1800,
	7*3600,
	8*3600,
	9*3600,
	9*3600+1800,
	10*3600,
	11*3600,
	12*3600,
	13*3600,
};

int GetTimeZoneOffset(int index)
{
	if(index < 0 || index >= sizeof(TimeZoneOffset)/sizeof(TimeZoneOffset[0]))
	{
		return 0;
	}
	
	return TimeZoneOffset[index];
}

int GetTimeZoneDefault()
{
#if 1//QYJ
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeZoneDefault\n");
	}
	
	return bizSysPara.nTimeZone;
#else
	return 27;
#endif
}

int SetMacAddress(char *pBuf)
{
	return BizSetHWAddr(pBuf);
}

int GetNetworkSyscCheck()
{
#if 1//QYJ
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetNetworkSyscCheck\n");
	}
	
	return bizSysPara.nSyscCheck;
#else
	return 0;
#endif
}

void GetNtpServer(char* pServer, int nMaxLen)
{
#if 1//QYJ
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetNtpServer\n");
	}
	
	strncpy(pServer,bizSysPara.strNtpServerAdress,nMaxLen);
#else
	strncpy(pServer,"time.windows.com",nMaxLen);
#endif
}

//QYJ
int GetNetworkSyscCheckDefault()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetNetworkSyscCheckDefault\n");
	}
	
	return bizSysPara.nSyscCheck;
}

void GetNtpServerDefault(char* pser, int nMaxLen)
{
#if 1//QYJ
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetNtpServerDefault\n");
	}
	
	strncpy(pser,bizSysPara.strNtpServerAdress,nMaxLen);
#else
	strncpy(pser,"time.windows.com",nMaxLen);
#endif
}

void GetSysDate(SYSTEM_TIME *p)
{
	SBizDateTime sBizDt;
	BizSysComplexDTGet(&sBizDt);
	
	p->year = sBizDt.nYear;
	p->month = sBizDt.nMonth;
	p->day = sBizDt.nDay;
}

void GetSysTime(SYSTEM_TIME *p)
{
	SBizDateTime sBizDt;
	BizSysComplexDTGet(&sBizDt);
	
	p->hour = sBizDt.nHour;
	p->minute = sBizDt.nMinute;
	p->second = sBizDt.nSecond;
}

void GetSysDate_TZ(SYSTEM_TIME *p)
{
	SBizDateTime sBizDt;
	BizSysComplexDTGet_TZ(&sBizDt);
	
	p->year = sBizDt.nYear;
	p->month = sBizDt.nMonth;
	p->day = sBizDt.nDay;
}

void GetSysTime_TZ(SYSTEM_TIME *p)
{
	SBizDateTime sBizDt;
	BizSysComplexDTGet_TZ(&sBizDt);
	
	p->hour = sBizDt.nHour;
	p->minute = sBizDt.nMinute;
	p->second = sBizDt.nSecond;
}

void GetPtzAddr(int nCh, char* addr, int nMaxLen)
{
	//snprintf(pName,nMaxLen,"CAMERA0%d",nCh+1);
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PTZPARA;
	bizTar.nChn = nCh;
	SBizPtzPara bizptzpara;
	int ret = BizGetPara(&bizTar, &bizptzpara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetPtzAddr\n");
	}
	
	u8 CamAddr = bizptzpara.nCamAddr;
	char pcamaddr[nMaxLen]; //zlb20111116 原变量是malloc方式
	sprintf(pcamaddr, "%d", CamAddr);
	
	//BIZ_DO_DBG("GetDevId %d\n", devId);
	
	strncpy(addr,pcamaddr,nMaxLen);
}

void GetChnName(int nCh, char* pName, int nMaxLen)
{
	//snprintf(pName,nMaxLen,"CAMERA0%d",nCh+1);
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_STROSD;
	bizTar.nChn = nCh;
	SBizCfgStrOsd bizStrOsd;
	int ret = BizGetPara(&bizTar, &bizStrOsd);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetChnName\n");
	}
	
	strncpy(pName,bizStrOsd.strChnName,nMaxLen);
}

int GetDisplayNameCheck(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_STROSD;
	bizTar.nChn = nCh;
	SBizCfgStrOsd bizStrOsd;
	int ret = BizGetPara(&bizTar, &bizStrOsd);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetDisplayNameCheck\n");
	}
	
	return bizStrOsd.nShowChnName;
}

int GetLiveAllCheck()
{
	return 0;
}

int GetLiveRecStateCheck()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	int ret = BizGetPara(&bizTar, &bizPreCfg);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetLiveRecStateCheck\n");
	}
	
	return bizPreCfg.nShowRecState;
}

int GetAudioOutInfo(u8* pAudioOutChn, u8* pVolume, u8* pMute)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	int ret = BizGetPara(&bizTar, &bizPreCfg);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetLiveRecStateCheck\n");
		return -1;
	}

	*pAudioOutChn = bizPreCfg.nAudioOutChn;
	*pVolume = bizPreCfg.nVolume;
	*pMute = bizPreCfg.nMute;
	return 0;
}

int SetAudioOutInfo(u8 nAudioOutChn, u8 nVolume, u8 nMute)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	int ret = BizGetPara(&bizTar, &bizPreCfg);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetLiveRecStateCheck\n");
		return -1;
	}
	
	bizPreCfg.nAudioOutChn = nAudioOutChn;
	bizPreCfg.nVolume = nVolume;
	bizPreCfg.nMute = nMute;
	
	BizSetPara(&bizTar, &bizPreCfg);
	
	return 0;
}

s32 SetIsPatrolPara(SBizCfgPatrol *psPara, u8 nId)
{
	return BizSetIsPatrolPara(psPara, nId);
}

s32 GetIsPatrolPara(u32 bDefault,SBizCfgPatrol *psPara, u8 nId)
{
	return BizGetIsPatrolPara(bDefault,psPara, nId);
}

int GetChnNameDefault(int nCh, char* pName, int nMaxLen)
{
	//snprintf(pName,nMaxLen,"CAMERA0%d",nCh+1);

	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_STROSD;
	bizTar.nChn = nCh;
	SBizCfgStrOsd bizStrOsd;
	int ret = BizGetDefaultPara(&bizTar, &bizStrOsd);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetChnName\n");
	}
	

	strncpy(pName,bizStrOsd.strChnName,nMaxLen);

	return ret;
}

int GetDisplayNameCheckDefault(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_STROSD;
	bizTar.nChn = nCh;
	SBizCfgStrOsd bizStrOsd;
	int ret = BizGetDefaultPara(&bizTar, &bizStrOsd);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetDisplayNameCheck\n");
	}
	
	return bizStrOsd.nShowChnName;
}

int GetLiveRecStateCheckDefault()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	int ret = BizGetDefaultPara(&bizTar, &bizPreCfg);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetLiveRecStateCheck\n");
	}
	
	return bizPreCfg.nShowRecState;
}



void GetSplitList(SValue* psValueList, int* nRealNum, int nMaxNum)
{	
	InitStrList();
		

	if (psValueList)
	{

		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_SITEMAINOUT, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = 1;
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetSplitList\n");
		}
	

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nSplitMode[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}
	}

}

int GetRectOsd(int nChn,int maxNum,int* realNum,SRect* osdRect)
{
	//BIZ_DATA_DBG("11111111GetRectOsd");
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECTOSd;
	bizTar.nChn = nChn;
	SBizCfgRectOsd bizRectOsd;

	bizRectOsd.psRectOsd = osdRect; //(SRect*)malloc(sizeof(SRect)*10);
	if(!bizRectOsd.psRectOsd)
	{
		printf("osdRect error\n");
		return -1;
	}

	bizRectOsd.nMaxNum = maxNum;
	int ret = BizGetPara(&bizTar, &bizRectOsd);
	
	//BIZ_DATA_DBG("22222GetRectOsd");
			
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetRectOsd error\n");
	}
	
	//printf("line:%d max:%d, real:%d  33333\n",__LINE__,bizRectOsd.nMaxNum,bizRectOsd.nRealNum);
#if 0
	if( bizRectOsd.nMaxNum >=4 || bizRectOsd.nRealNum >=4)
	{
		*maxNum = 4;
		*realNum = 0;
	}
	else
	{
		*maxNum = bizRectOsd.nMaxNum;
		*realNum = 0;
	}
#else
	//*maxNum = bizRectOsd.nMaxNum;
	*realNum = bizRectOsd.nRealNum;

#endif
	
	//printf("line:%d max:%d, real:%d  4444\n",__LINE__,*maxNum,*realNum);

	/*for(int i=0; i<*realNum; i++)
	//for(int i=0; i<4; i++)
	{
		osdRect[i] = bizRectOsd.psRectOsd[i];
	}*/

	//printf("line:%d \n",__LINE__);
}

int SetRectOsd(int nChn,int realNum,SRect* osdRect)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECTOSd;
	bizTar.nChn = nChn;
	SBizCfgRectOsd bizRectOsd;
	
	bizRectOsd.nShowRectOsd = 1;
	bizRectOsd.nRealNum = realNum;
	bizRectOsd.psRectOsd = osdRect;
	
	int ret = BizSetPara(&bizTar, &bizRectOsd);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetRectOsd\n");
	}
	
	return ret;
}


int nChn[5] = 
{
	0,1,2,3
};

const char* szChn[5] = 
{
	"1",
	"2",
	"3",
	"4",
	"&CfgPtn.All",
};

#if 0
void GetChnList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	
	if (psValueList)
	{

		//获得 配置 
		*nRealNum = 4;

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nChn[i];
			strcpy(psValueList[i].strDisplay,szChn[i]);
		}
	}
}
#else
void GetChnList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	if(psValueList)
	{
		//获得 配置 
		*nRealNum = GetMaxChnNum();
		
		if(*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}
		
		int i = 0;
		for(i=0; i<GetMaxChnNum(); i++)
		{
			//psValueList[i].nValue = nChn[i];
			char szCH[8] = {0};
			sprintf(szCH,"%d",i+1);
			strcpy(psValueList[i].strDisplay,szCH);
		}
		
		//strcpy(psValueList[i].strDisplay,"&CfgPtn.All");
	}
}
#endif

void GetCopyChnList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	
	if (psValueList)
	{

		//获得 配置 
		*nRealNum = GetMaxChnNum()+1;

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		int i = 0;
		for(i=0; i<GetMaxChnNum(); i++)
		{
			//psValueList[i].nValue = nChn[i];
			char szCH[8] = {0};
			sprintf(szCH,"%d",i+1);
			strcpy(psValueList[i].strDisplay,szCH);
		}
		
		strcpy(psValueList[i].strDisplay,"&CfgPtn.All");
	}
}

void GetTimerSchedule(int nChn, uint* pMask, int nNum)
{
	SBizSchedule sCfgIns;
	SBizParaTarget sParaTgtIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	
	//clear all of the weekday
	if(pMask)
	{
		memset(pMask, 0, 7*sizeof(uint));
	}
	
	sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECTIMING;
	sParaTgtIns.nChn = nChn;
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		int i, j, k, l, h0, h1;
		for(i=0; i<nNum; i++)
		{
			SBizSchTime* pSch = sCfgIns.sSchTime[i];
			for(j=0; j<CUR_SCH_SEG; j++)
			{
				if(pSch[j].nStartTime<pSch[j].nStopTime)
				{
					h0 = pSch[j].nStartTime/3600;
					h1 = (pSch[j].nStopTime+3600-1)/3600;
					
					for(k=h0; k<=h1; k++)
					{
						pMask[i] |= (0x1<<k);
					}
				}
			}
		}
		
		return;
	}
}

void GetMotionAlarmSchedule(int nChn, uint* pMask, int nNum)
{
	SBizSchedule sCfgIns;
	SBizParaTarget sParaTgtIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	
	//clear all of the weekday
	if(pMask)
	{
		memset(pMask, 0, 7*sizeof(uint));
	}
	
	sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECMOTION;
	sParaTgtIns.nChn = nChn;
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		int i, j, k, l, h0, h1;
		for(i=0; i<nNum; i++)
		{
			SBizSchTime* pSch = sCfgIns.sSchTime[i];
			for(j=0; j<CUR_SCH_SEG; j++)
			{
				if(pSch[j].nStartTime<pSch[j].nStopTime)
				{
					h0 = pSch[j].nStartTime/3600;
					h1 = (pSch[j].nStopTime+3600-1)/3600;
					
					for(k=h0; k<=h1; k++)
					{
						pMask[i] |= (0x1<<k);
					}
				}
			}
		}
		
		return;
	}
}

void GetSensorAlarmSchedule(int nChn, uint* pMask, int nNum)
{
	SBizSchedule sCfgIns;
	SBizParaTarget sParaTgtIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	
	//clear all of the weekday
	if(pMask)
	{
		memset(pMask, 0, 7*sizeof(uint));
	}
	
	sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECSENSOR;
	sParaTgtIns.nChn = nChn;
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		int i, j, k, l, h0, h1;
		for(i=0; i<nNum; i++)
		{
			SBizSchTime* pSch = sCfgIns.sSchTime[i];
			for(j=0; j<CUR_SCH_SEG; j++)
			{
				if(pSch[j].nStartTime<pSch[j].nStopTime)
				{
					h0 = pSch[j].nStartTime/3600;
					h1 = (pSch[j].nStopTime+3600-1)/3600;
					
					for(k=h0; k<=h1; k++)
					{
						pMask[i] |= (0x1<<k);
					}
				}
			}
		}
		
		return;
	}
	
	printf("3333333333333333333\n");
}

int GetMainSplit(){}
int GetSpotSplit(){}
int GetMainDwellTime(){}
int GetSpotDwellTime(){}


void GetDwellTimeList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_INTERVAL, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;

		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetDwellTimeList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nDwellTime[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}
	}

}

int GetDwellTime()
{
	return 0;

}

int GetRecordEnable(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	bizTar.nChn = nCh;
	SBizRecPara bizRecPara;
	int ret = BizGetPara (&bizTar, &bizRecPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetRecordEnable\n");
	}
	
	return bizRecPara.nEnable;

	//return 1;
}

int GetAudioEnable(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	bizTar.nChn = nCh;
	SBizRecPara bizRecPara;
	int ret = BizGetPara (&bizTar, &bizRecPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetAudioEnable\n");
	}
	
	return bizRecPara.nStreamType;
}

int GetRecordEnableDefault(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	bizTar.nChn = nCh;
	SBizRecPara bizRecPara;
	int ret = BizGetDefaultPara (&bizTar, &bizRecPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetRecordEnable\n");
	}
	
	return bizRecPara.nEnable;
	
	return 1;
}

int GetAudioEnableDefault(int nCh)
{

	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	bizTar.nChn = nCh;
	SBizRecPara bizRecPara;
	int ret = BizGetDefaultPara (&bizTar, &bizRecPara);

	if(ret != 0)
	{
		BIZ_DATA_DBG("GetAudioEnable\n");
		return ret;
	}
	
	return bizRecPara.nStreamType;

}


int GetEnableAllCheck()
{
	return 0;
}

void GetMobResoList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	//printf("line: %d \n",__LINE__);
	
	InitStrList();
	
	if (psValueList)
	{
		//printf("line: %d \n",__LINE__);
		//获得 配置 
		u8 real=0;
		//int ret = 1;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_VMOBRESOL, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;

		//printf("real = %d line: %d \n",real,__LINE__);
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetResoList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}
		
		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nReso[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}
	}
}


void GetSubResoList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	//printf("line: %d \n",__LINE__);
	
	InitStrList();
	
	if (psValueList)
	{
		//printf("line: %d \n",__LINE__);
		//获得 配置 
		u8 real=0;
		//int ret = 1;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_VSUBRESOL, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;

		//printf("real = %d line: %d \n",real,__LINE__);
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetResoList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}
		
		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nReso[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}
	}
}

void GetVideoSrcTypeList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		u8 real=0;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_VIDEOSRCTYPE, (s8**)g_strList, (u8*)&real, 10, 20);
		*nRealNum = real;
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetVideoSrcTypeList\n");
		}
		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}		
		for(int i=0; i<*nRealNum; i++)
		{
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}
	}
}

int SetVideoSrcIndex(uchar* pIndex)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	memset(&bizPreCfg , 0, sizeof(bizPreCfg));
	int ret = 0;
	ret = BizGetPara(&bizTar, &bizPreCfg);
	s32 val = 0;
	for(int i = 0; i<GetMaxChnNum(); i++)
	{
		ret |= BizConfigGetParaListValue(
					EM_BIZ_CFG_PARALIST_VIDEOSRCTYPE,
					pIndex[i],
					&val
			);
		bizPreCfg.nVideoSrcType[i] = val;
	}
	ret |= BizSetPara(&bizTar, &bizPreCfg);
	return ret;
}

void GetVideoSrcIndex(uchar* pIndex)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	memset(&bizPreCfg , 0, sizeof(bizPreCfg));
	BizGetPara(&bizTar, &bizPreCfg);
	for(int i = 0; i<GetMaxChnNum(); i++)
	{
		pIndex[i] = bizPreCfg.nVideoSrcType[i];
		BizConfigGetParaListIndex(
					EM_BIZ_CFG_PARALIST_VIDEOSRCTYPE,
					(s32)bizPreCfg.nVideoSrcType[i],
					&pIndex[i]
			);
	}
}

void GetVideoSrcIndexDefault(uchar* pIndex)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PREVIEWPARA;
	SBizCfgPreview bizPreCfg;
	memset(&bizPreCfg , 0, sizeof(bizPreCfg));
	BizGetDefaultPara(&bizTar, &bizPreCfg);
	for(int i = 0; i<GetMaxChnNum(); i++)
	{
		pIndex[i] = bizPreCfg.nVideoSrcType[i];
		BizConfigGetParaListIndex(
					EM_BIZ_CFG_PARALIST_VIDEOSRCTYPE,
					(s32)bizPreCfg.nVideoSrcType[i],
					&pIndex[i]
			);
	}
}

int GetEPortValue(int sel)//cw_ddns
{
	int value = 0;
	BizConfigGetParaListValue(EM_BIZ_CFG_PARALIST_DDNSDOMAIN,sel,&value);
	return value;
}

void GetResoListH_NUM(int* nRealNum)//cw_9508S
{
	BizConfigGetParaListValue(EM_BIZ_CFG_PARALIST_VMAINRESOLH_NUM, 0, nRealNum);
	int temp = *nRealNum;
	SetBizResolHNum(temp);
}

void GetResoList2(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	//printf("line: %d \n",__LINE__);
	
	InitStrList();
	
	if (psValueList)
	{
		//printf("line: %d \n",__LINE__);
		//获得 配置 
		u8 real=0;
		//int ret = 1;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_VMAINRESOL2, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;

		//printf("real = %d line: %d \n",real,__LINE__);
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetResoList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}
		
		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nReso[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}

		BIZ_DATA_DBG("$$$");
	
	}
}

void GetResoList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	//printf("line: %d \n",__LINE__);
	
	InitStrList();
	
	if (psValueList)
	{
		//printf("line: %d \n",__LINE__);
		//获得 配置 
		u8 real=0;
		//int ret = 1;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_VMAINRESOL, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;
		
		//printf("real = %d line: %d \n",real,__LINE__);
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetResoList\n");
		}
		
		if(*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}
		
		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nReso[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}
		
		BIZ_DATA_DBG("$$$");
	}
}

void GetFrameRateList(int nResol, SValue* psValueList, int* nRealNum, int nMaxNum) //nResol为combox选卡值
{
	InitStrList();
	
	if(psValueList)
	{
		//获得 配置
		//*nRealNum = 7;
		u8 val = nResol;
		s32 nValue = EM_BIZENC_RESOLUTION_CIF;
		if(1)
		{	
			if( 0==BizConfigGetParaListValue(
					EM_BIZ_CFG_PARALIST_VMAINRESOL, 
					val, 
					&nValue )
			)
			{
			}
		}
		
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
		SBizSystemPara bizSysPara;
		int ret = BizGetPara(&bizTar, &bizSysPara);	
		int format = EM_BIZ_PAL;
		if(ret==0)
		{
			format = bizSysPara.nVideoStandard;
		}
	
		if(nValue == EM_BIZENC_RESOLUTION_D1)//d1
		{
			u8 real;
			EMBIZCFGPARALIST para = EM_BIZ_CFG_PARALIST_FPSPALD1;
			if(format == EM_BIZ_PAL)
			{
				para = EM_BIZ_CFG_PARALIST_FPSPALD1;
			}
			else if(format == EM_BIZ_NTSC)
			{
				para = EM_BIZ_CFG_PARALIST_FPSNTSCD1;
			}
			int ret = BizConfigGetParaStr(para, (s8**)g_strList, (u8*)&real, 20, 64);
			*nRealNum = real;

			if(ret!=0)
			{
				BIZ_DATA_DBG("GetFrameRateList\n");
			}

			
			if (*nRealNum > nMaxNum)
			{
				*nRealNum = nMaxNum;
			}

			for(int i=0; i<*nRealNum; i++)
			{
				//psValueList[i].nValue = nFps[i];
				strcpy(psValueList[i].strDisplay,g_strList[i]);
			}
		}
		else if(nValue == EM_BIZENC_RESOLUTION_CIF) //cif
		{
			u8 real;
			EMBIZCFGPARALIST para = EM_BIZ_CFG_PARALIST_FPSPALCIF;
			if(format == EM_BIZ_PAL)
			{
				para = EM_BIZ_CFG_PARALIST_FPSPALCIF;
			}
			else if(format == EM_BIZ_NTSC)
			{
				para = EM_BIZ_CFG_PARALIST_FPSNTSCCIF;
			}
			int ret = BizConfigGetParaStr(para, (s8**)g_strList, (u8*)&real, 20, 64);
			*nRealNum = real;

			if(ret!=0)
			{
				BIZ_DATA_DBG("GetFrameRateList\n");
			}


			
			if (*nRealNum > nMaxNum)
			{
				*nRealNum = nMaxNum;
			}
			
			for(int i=0; i<*nRealNum; i++)
			{
				//psValueList[i].nValue = nFps[i];
				strcpy(psValueList[i].strDisplay,g_strList[i]);
			}
	
		}
		else
		{
			u8 real;
			EMBIZCFGPARALIST para = EM_BIZ_CFG_PARALIST_FPSPAL;
			if(format == EM_BIZ_PAL)
			{
				para = EM_BIZ_CFG_PARALIST_FPSPAL;
			}
			else if(format == EM_BIZ_NTSC)
			{
				para = EM_BIZ_CFG_PARALIST_FPSNTSC;
			}
			int ret = BizConfigGetParaStr(para, (s8**)g_strList, (u8*)&real, 20, 64);
			*nRealNum = real;

			if(ret!=0)
			{
				BIZ_DATA_DBG("GetFrameRateList\n");
			}


			
			if (*nRealNum > nMaxNum)
			{
				*nRealNum = nMaxNum;
			}
			
			for(int i=0; i<*nRealNum; i++)
			{
				//psValueList[i].nValue = nFps[i];
				strcpy(psValueList[i].strDisplay,g_strList[i]);
			}
	
		}
	}
}


void GetEncodeList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_BITTYPE, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetEncodeList\n");
		}
		//*nRealNum = 2;
		
		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}
		
		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nEncode[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}
	}
}

void GetQualityList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_PICLEVEL, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetQualityList\n");
		}
		//*nRealNum = 7;

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nQuality[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}
	}
}

void GetBitrateList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if(psValueList)
	{
		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_BITRATE, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetBitrateList\n");
		}
		
		if(*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}
		
		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nBitrate[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}
	}
}

//返回combo index
int GetReso(int nCh)
{
	//printf("GetReso\n");
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_ENCMAINPARA;
	bizTar.nChn = nCh;
	SBizEncodePara bizEncPara;
	int ret = BizGetPara(&bizTar, &bizEncPara);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetReso\n");
	}
	
	u8 index = 0;
	
	//printf("bizEncPara.nVideoResolution = %d\n",bizEncPara.nVideoResolution);
	//int start = GetTimeTick();
	int Hnum = 0;//cw_9508S
	EMBIZCFGPARALIST emParaType;
	GetResoListH_NUM(&Hnum);
	if(nCh < Hnum)
		emParaType = EM_BIZ_CFG_PARALIST_VMAINRESOL;
	else
		emParaType = EM_BIZ_CFG_PARALIST_VMAINRESOL2;
	ret = BizConfigGetParaListIndex(
							emParaType, 
							bizEncPara.nVideoResolution, 
							&index);
	
	//int end = GetTimeTick();
	//printf("end - start = %d\n",end - start);
	//printf("index = %d\n",index);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetReso\n");
	}
	
	return index;
	//return 1;
}

//csp modify 20130504
int VerifyReso(int nCh, int idx)
{
	u8 index = idx;
	
	int Hnum = 0;
	GetResoListH_NUM(&Hnum);
	
	EMBIZCFGPARALIST emParaType;
	if(nCh < Hnum)
	{
		emParaType = EM_BIZ_CFG_PARALIST_VMAINRESOL;
	}
	else
	{
		emParaType = EM_BIZ_CFG_PARALIST_VMAINRESOL2;
	}
	
	s32 value = -1;
	s32 ret = BizConfigGetParaListValue(
							emParaType, 
							index, 
							&value);
	
	if(ret!=0)
	{
		//printf("CH%d VerifyReso failed\n",nCh);
		return -1;
	}
	
	return 0;
}

int GetFramerate(int nResol, int nCh)
{
	u8 val = nResol;
	s32 nValue = EM_BIZENC_RESOLUTION_CIF;
	BizConfigGetParaListValue(EM_BIZ_CFG_PARALIST_VMAINRESOL, val,&nValue );
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);	
	int format = EM_BIZ_PAL;
	if(ret==0)
	{
		format = bizSysPara.nVideoStandard;
	}
	
	bizTar.emBizParaType = EM_BIZ_ENCMAINPARA;
	bizTar.nChn = nCh;
	SBizEncodePara bizEncPara;
	ret = BizGetPara(&bizTar, &bizEncPara);
	if(ret!=0)
	{
		return ret;
	}
	
	u8 index = 0;		

	if(nValue == EM_BIZENC_RESOLUTION_D1)//d1
	{
		EMBIZCFGPARALIST para = EM_BIZ_CFG_PARALIST_FPSPALD1;
		if(format == EM_BIZ_PAL)
		{
			para = EM_BIZ_CFG_PARALIST_FPSPALD1;
		}
		else if(format == EM_BIZ_NTSC)
		{
			para = EM_BIZ_CFG_PARALIST_FPSNTSCD1;
		}

		ret = BizConfigGetParaListIndex(
								para, 
								bizEncPara.nFrameRate, 
								&index);
	}
	else if(nValue == EM_BIZENC_RESOLUTION_CIF)//cif
	{
		EMBIZCFGPARALIST para = EM_BIZ_CFG_PARALIST_FPSPALCIF;
		if(format == EM_BIZ_PAL)
		{
			para = EM_BIZ_CFG_PARALIST_FPSPALCIF;
		}
		else if(format == EM_BIZ_NTSC)
		{
			para = EM_BIZ_CFG_PARALIST_FPSNTSCCIF;
		}

		ret = BizConfigGetParaListIndex(
								para, 
								bizEncPara.nFrameRate, 
								&index);
	}
	else
	{
		EMBIZCFGPARALIST para = EM_BIZ_CFG_PARALIST_FPSPAL;
		if(format == EM_BIZ_PAL)
		{
			para = EM_BIZ_CFG_PARALIST_FPSPAL;
		}
		else if(format == EM_BIZ_NTSC)
		{
			para = EM_BIZ_CFG_PARALIST_FPSNTSC;
		}

		ret = BizConfigGetParaListIndex(
								para, 
								bizEncPara.nFrameRate, 
								&index);
	}
	
	return index;
}

#if 0//csp modify
u8 bizData_IsCbr( char *pItemStr )
{
	int ret = 0;
	
	if(pItemStr)
	{
		if(strcmp(pItemStr, "CBR")==0)
		{
			ret = 1;
		}
		else
			ret = 0;
	}
	
	//printf("bizData_IsCbr pItemStr:%s ret:%d\n",pItemStr,ret);
	
	return ret;
}
#endif

int GetEncode(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_ENCMAINPARA;
	bizTar.nChn = nCh;
	
	SBizEncodePara bizEncPara;
	int ret = BizGetPara(&bizTar, &bizEncPara);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetEncode\n");
	}
	
	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_BITTYPE, 
							bizEncPara.nBitRateType, 
							&index);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetEncode\n");
	}
	
	return index;
}

int GetQuality(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_ENCMAINPARA;
	bizTar.nChn = nCh;
	SBizEncodePara bizEncPara;
	int ret = BizGetPara(&bizTar, &bizEncPara);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetQuality\n");
	}

	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_PICLEVEL, 
							bizEncPara.nPicLevel, 
							&index);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetQuality\n");
	}

	return index;

}

int GetBitrate(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_ENCMAINPARA;
	bizTar.nChn = nCh;
	SBizEncodePara bizEncPara;
	int ret = BizGetPara(&bizTar, &bizEncPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetBitrate\n");
	}

	u8 index = 0;

	//printf("ch = %d , nBitRate = %d \n",nCh,bizEncPara.nBitRate);
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_BITRATE, 
							bizEncPara.nBitRate, 
							&index);

	//printf("ch = %d , index = %d \n",nCh,index);

	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetBitrate\n");
	}

	return index;
}

int GetResoDefault(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_ENCMAINPARA;
	bizTar.nChn = nCh;
	SBizEncodePara bizEncPara;
	int ret = BizGetDefaultPara(&bizTar, &bizEncPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetResoDefault\n");
	}

	u8 index = 0;
	
	int Hnum = 0;//cw_9508S
	EMBIZCFGPARALIST emParaType;
	GetResoListH_NUM(&Hnum);
	if(nCh < Hnum)
		emParaType = EM_BIZ_CFG_PARALIST_VMAINRESOL;
	else
		emParaType = EM_BIZ_CFG_PARALIST_VMAINRESOL2;
	
	ret = BizConfigGetParaListIndex(
							emParaType, 
							bizEncPara.nVideoResolution, 
							&index);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetResoDefault\n");
	}

	return index;
	//return 1;
}

int GetFramerateDefault(int nResol, int nCh)
{
	//printf("GetFramerate %d chn \n",nCh);
	
	if(nResol == 2)//d1
	{
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_ENCMAINPARA;
		bizTar.nChn = nCh;
		SBizEncodePara bizEncPara;

		int ret = BizGetDefaultPara(&bizTar, &bizEncPara);

		if(ret!=0)
		{
			BIZ_DATA_DBG("GetFramerate\n");
		}

		u8 index = 0;
		
		ret = BizConfigGetParaListIndex(
								EM_BIZ_CFG_PARALIST_FPSPALD1, 
								bizEncPara.nFrameRate, 
								&index);

		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetFramerate\n");
		}

		return index;
	}
	else  //qcif , cif
	{
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_ENCMAINPARA;
		bizTar.nChn = nCh;
		SBizEncodePara bizEncPara;
		int ret = BizGetDefaultPara(&bizTar, &bizEncPara);

		if(ret!=0)
		{
			BIZ_DATA_DBG("GetFramerate\n");
		}

		u8 index = 0;
		
		ret = BizConfigGetParaListIndex(
								EM_BIZ_CFG_PARALIST_FPSPAL, 
								bizEncPara.nFrameRate, 
								&index);

		if(ret!=0)
		{
			BIZ_DATA_DBG("GetFramerate\n");
		}

		return index;
	
	}

}

int GetEncodeDefault(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_ENCMAINPARA;
	bizTar.nChn = nCh;
	SBizEncodePara bizEncPara;
	int ret = BizGetDefaultPara(&bizTar, &bizEncPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetEncode\n");
	}

	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_BITTYPE, 
							bizEncPara.nBitRateType, 
							&index);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetEncode\n");
	}

	return index;

}

int GetQualityDefault(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_ENCMAINPARA;
	bizTar.nChn = nCh;
	SBizEncodePara bizEncPara;
	int ret = BizGetDefaultPara(&bizTar, &bizEncPara);
	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetQuality\n");
	}

	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_PICLEVEL, 
							bizEncPara.nPicLevel, 
							&index);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetQuality\n");
	}

	return index;

}

int GetBitrateDefault(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_ENCMAINPARA;
	bizTar.nChn = nCh;
	SBizEncodePara bizEncPara;
	int ret = BizGetDefaultPara(&bizTar, &bizEncPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetBitrate\n");
		return ret;
	}

	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_BITRATE, 
							bizEncPara.nBitRate, 
							&index);

	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetBitrate\n");
		return ret;
	}

	return index;
}

char* GetCodeStreamLeft()
{	
	return (char *)"Left: 4 (CIF).";
}

int GetCodeStreamAllCheck()
{

	return 0;
}

void GetPreAlarmTimeList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{

		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_PRETIME, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;

		if(ret!=0)
		{
			BIZ_DATA_DBG("GetPreAlarmTimeList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nAlarmTime[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}
	}
}

void GetDelayAlarmTimeList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{

		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_RECDELAY, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetDelayAlarmTimeList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nAlarmTime[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}
	}
}



void GetExpirationList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	if (psValueList)
	{

		//获得 配置 
		u8 real;
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_RECEXPIRE, (s8**)g_strList, (u8*)&real, 20, 64);
		*nRealNum = real;
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetExpirationList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nExpiration[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}
	}
}


//返回comboBox index
int GetTimeBeforeAlarm(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	bizTar.nChn = nCh;
	SBizRecPara bizRecPara;
	int start = 0, end = 0;

	//start = GetTimeTick();
	int ret = BizGetPara(&bizTar, &bizRecPara);
	//end = GetTimeTick();
	//printf("BizGetPara expire = %d \n",end - start);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeBeforeAlarm\n");
	}

	u8 index = 0;

	//start = GetTimeTick();
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_PRETIME, 
							bizRecPara.nPreTime, 
							&index);
	//end = GetTimeTick();
	//printf("BizConfigGetParaListIndex expire = %d \n",end - start);

	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeBeforeAlarm\n");
	}

	return index;

}

int GetTimeAfterAlarm(int nCh)
{

	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	bizTar.nChn = nCh;
	SBizRecPara bizRecPara;
	int ret = BizGetPara(&bizTar, &bizRecPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeAfterAlarm\n");
	}

	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_RECDELAY, 
							bizRecPara.nRecDelay, 
							&index);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeAfterAlarm\n");
	}

	return index;
}

int GetExpiration(int nCh)
{

	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	bizTar.nChn = nCh;
	SBizRecPara bizRecPara;
	int ret = BizGetPara(&bizTar, &bizRecPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetExpiration\n");
	}

	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_RECEXPIRE, 
							bizRecPara.nRecExpire, 
							&index);

	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetExpiration\n");
	}

	return index;
}



int GetTimeBeforeAlarmDefault(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	bizTar.nChn = nCh;
	SBizRecPara bizRecPara;
	int ret = BizGetDefaultPara(&bizTar, &bizRecPara);

	printf("GetTimeBeforeAlarm, %d\n",bizRecPara.nPreTime);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeBeforeAlarm\n");
		return ret;
	}

	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_PRETIME, 
							bizRecPara.nPreTime, 
							&index);

	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeBeforeAlarm\n");
		return ret;
	}

	return index;

}

int GetTimeAfterAlarmDefault(int nCh)
{

	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	bizTar.nChn = nCh;
	SBizRecPara bizRecPara;
	int ret = BizGetDefaultPara(&bizTar, &bizRecPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeAfterAlarm\n");
	}

	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_RECDELAY, 
							bizRecPara.nRecDelay, 
							&index);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeAfterAlarm\n");
	}

	return index;
}

int GetExpirationDefault(int nCh)
{

	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	bizTar.nChn = nCh;
	SBizRecPara bizRecPara;
	int ret = BizGetDefaultPara(&bizTar, &bizRecPara);

	if(ret!=0)
	{
		BIZ_DATA_DBG("GetExpiration\n");
	}

	u8 index = 0;
	
	ret = BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_RECEXPIRE, 
							bizRecPara.nRecExpire, 
							&index);

	
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetExpiration\n");
	}

	return index;
}

int GetRecordTimeAllCheck()
{
	return 0;
}

int GetChnNameCheck(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_STROSD;
	bizTar.nChn = nCh;
	SBizCfgStrOsd bizStrOsd;
	int ret = BizGetPara(&bizTar, &bizStrOsd);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetChnNameCheck\n");
	}
	
	return bizStrOsd.nEncShowChnName;
}

int GetTimeStampCheck(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_STROSD;
	bizTar.nChn = nCh;
	SBizCfgStrOsd bizStrOsd;
	int ret = BizGetPara(&bizTar, &bizStrOsd);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeStampCheck\n");
	}
	
	return bizStrOsd.nEncShowTime;
}

int GetChnNameCheckDefault(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_STROSD;
	bizTar.nChn = nCh;
	SBizCfgStrOsd bizStrOsd;
	int ret = BizGetDefaultPara(&bizTar, &bizStrOsd);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetChnNameCheck\n");
		return ret;
	}
	
	return bizStrOsd.nEncShowChnName;
}

int GetTimeStampCheckDefault(int nCh)
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_STROSD;
	bizTar.nChn = nCh;
	SBizCfgStrOsd bizStrOsd;
	int ret = BizGetDefaultPara(&bizTar, &bizStrOsd);
	if(ret!=0)
	{
		BIZ_DATA_DBG("GetTimeStampCheck\n");
	}
	
	return bizStrOsd.nEncShowTime;
}

int GetOsdAllCheck()
{
	return 0;
}

int GetCyclingRecCheck()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(ret != 0)
	{
		BIZ_DATA_DBG("GetCyclingRecCheck error\n");
	}
	
	return bizSysPara.nCyclingRecord;
}

int GetCyclingRecCheckDefault()
{
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetDefaultPara(&bizTar, &bizSysPara);
	if(ret != 0)
	{
		BIZ_DATA_DBG("GetCyclingRecCheck error\n");
		return ret;
	}
	
	return bizSysPara.nCyclingRecord;
}


void InitPtzAdvance(SBizPtzPara* ptzPara)
{
	if(!ptzPara)
	{
		return;
	}

	for(int i=0; i<10; i++)
	{
		ptzPara->sAdvancedPara.sTourPath[i].nPathNo = i+1;
		SBizTourPath* pPath = &ptzPara->sAdvancedPara.sTourPath[i];

		for(int j=i*10; j<i*10+5; j++)
		{
			pPath->sTourPoint[j].nPresetPos = j;
		}

	}


}

void GetCruiseLineList(int id,SValue* psValueList, int* nRealNum, int nMaxNum)
{
	/*
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PTZPARA;
	bizTar.nChn = id;
	SBizPtzPara bizPtzPara;
	int ret = BizGetPara(&bizTar, &bizPtzPara);
	if(ret != 0)
	{
		BIZ_DATA_DBG("GetCruiseLineList error\n");
	}*/

	SBizPtzPara bizPtzPara;
	bizPtzPara.nEnable = 1;
	InitPtzAdvance(&bizPtzPara);

	int real = 0;

	if(bizPtzPara.nEnable)
	{
		SBizPtzAdvancedPara* ptzAdvance = &bizPtzPara.sAdvancedPara;

		for(int i=0; i<128; i++)
		{
			if(ptzAdvance->sTourPath[i].nPathNo > 0)
			{
				char line[16] = {0};
				sprintf(line,"line%d",ptzAdvance->sTourPath[i].nPathNo);
				strcpy(psValueList[real++].strDisplay,line);

				if(real >= nMaxNum)
				{
					break;
				}
			}
		}

	}

	*nRealNum = real;

	

}




void GetPresetList(int id, int pathNO,SValue* psValueList, int* nRealNum, int nMaxNum)
{
	/*
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_PTZPARA;
	bizTar.nChn = id;
	SBizPtzPara bizPtzPara;
	int ret = BizGetPara(&bizTar, &bizPtzPara);
	if(ret != 0)
	{
		BIZ_DATA_DBG("GetCruiseLineList error\n");
	}
	*/

	SBizPtzPara bizPtzPara;
	bizPtzPara.nEnable = 1;
	InitPtzAdvance(&bizPtzPara);

	int real = 0;

	if(bizPtzPara.nEnable)
	{
		SBizPtzAdvancedPara* ptzAdvance = &bizPtzPara.sAdvancedPara;
		int i = 0;
		for(i=0; i<128; i++)
		{
			if(ptzAdvance->sTourPath[i].nPathNo == pathNO)
			{
				break;
			}
		}

		int real = 0;
		if(i != 128) // find
		{
			for(int j=0; j<128; j++)
			{
				if(ptzAdvance->sTourPath[i].sTourPoint[j].nPresetPos > 0)
				{
					char preset[24] = {0};
					sprintf(preset,"preset%d",ptzAdvance->sTourPath[i].sTourPoint[j].nPresetPos);
					strcpy(psValueList[real++].strDisplay,preset);
				}

				if(real >= nMaxNum)
				{
					break;

				}
			}

			*nRealNum = real;
			

		}

	}


}




// Base
/*
char* szSensorAlarmBaseType[] = 
{
	"Normal Open","Normal Close"
};
*/
/*
void GetSensorAlarmTypeList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	int i;
	s8* pTmpList[CBX_MAX];
	u8 nReal = 0;
	
	if(nMaxNum>CBX_MAX)
	{
		nMaxNum = CBX_MAX;
	}
	
	for(i=0; i<nMaxNum; i++)
	{
		pTmpList[i] = psValueList[i].strDisplay;
	}
	
	if( 0==BizConfigGetParaStr(
				EM_BIZ_CFG_PARALIST_SENSORALARMTYPE, 
				pTmpList, 
				(u8*)&nReal, 
				nMaxNum, 
				EDIT_MAX 
		)
	)
	{
		*nRealNum = nReal;
		BIZ_DATA_DBG("GetSensorAlarmBaseTypeList OK!\n");
	}
	else
	{
		BIZ_DATA_DBG("GetSensorAlarmBaseTypeList failed!\n");
	}
	
	for(i = 0; i < nReal; i++)
	{
		BizConfigGetParaListValue(
			EM_BIZ_CFG_PARALIST_SENSORALARMTYPE, 
			i, 
			&psValueList[i].nValue
		);
	}
}
*/

void GetSensorAlarmBaseTypeList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	int i;
	s8* pTmpList[CBX_MAX];
	u8 nReal;
	
	if(nMaxNum>CBX_MAX)
	{
		nMaxNum = CBX_MAX;
	}
	
	for(i=0; i<nMaxNum; i++)
	{
		pTmpList[i] = psValueList[i].strDisplay;
	}
	// 
	if( 0==BizConfigGetParaStr(
				EM_BIZ_CFG_PARALIST_SENSORTYPE, 
				pTmpList, 
				(u8*)&nReal, 
				nMaxNum, 
				EDIT_MAX 
		) 
	)
	{
		*nRealNum = nReal;
		
		BIZ_DATA_DBG("GetSensorAlarmBaseTypeList OK!\n");
	}
	else
	{
		BIZ_DATA_DBG("GetSensorAlarmBaseTypeList failed!\n");
	}
}

int GetSensorAlarmBaseEnableCheckCur(int ch) // 0-all, >0-ch
{
	int rtn = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmSensorPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
	sParaTgtIns.nChn = ch;
	rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		return sCfgIns.nEnable;
	}
	else
	{
		BIZ_DATA_DBG("GetSensorAlarmBaseEnableCheckCur err\n");
		return -1;
	}
}

int GetSensorAlarmBaseEnableCheckCurDefault(int ch) // 0-all, >0-ch
{
	int rtn = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmSensorPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
	sParaTgtIns.nChn = ch;
	rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		return sCfgIns.nEnable;
	}
	else
	{
		BIZ_DATA_DBG("GetSensorAlarmBaseEnableCheckCur err\n");
		return -1;
	}
}

int GetSensorAlarmBaseTypeCur(int ch)
{
	int rtn = 0;
	u8 nIdx = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmSensorPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
	sParaTgtIns.nChn = ch;
	rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{ 
		if( 0==BizConfigGetParaListIndex(
					EM_BIZ_CFG_PARALIST_SENSORTYPE,
					sCfgIns.nType,
					&nIdx
			) 
		)
		{
			BIZ_DATA_DBG("EM_BIZ_CFG_PARALIST_SENSORTYPE!\n");
		}
		else
		{
			BIZ_DATA_DBG("EM_BIZ_CFG_PARALIST_SENSORTYPE failed!\n");
		}
		
		return nIdx;
	}

	return 0;
}

int GetSensorAlarmBaseTypeCurDefault(int ch)
{
	int rtn = 0;
	u8 nIdx = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmSensorPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
	sParaTgtIns.nChn = ch;
	rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{ 
		if( 0==BizConfigGetParaListIndex(
					EM_BIZ_CFG_PARALIST_SENSORTYPE,
					sCfgIns.nType,
					&nIdx
			) 
		)
		{
			BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
		}
		else
		{
			BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
		}
		
		return nIdx;
	}

	return 0;
}

int GetSensorAlarmBaseNameCurDefault(int ch, char* psValue)
{
	int rtn = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmSensorPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
	sParaTgtIns.nChn = ch;
	rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		if(psValue)
		{
			//BIZ_DT_DBG("GetSensorAlarmBaseNameCur name %s\n", sCfgIns.name);
			strcpy(psValue, sCfgIns.name);
		}
		return rtn;
	}
	
	//BIZ_DATA_DBG("GetSensorAlarmBaseNameCur err\n");
	return rtn;
}

void GetSensorAlarmBaseNameCur(int ch, char* psValue)
{
	int rtn = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmSensorPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
	sParaTgtIns.nChn = ch;
	rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		if(psValue)
		{
			//BIZ_DT_DBG("GetSensorAlarmBaseNameCur name %s\n", sCfgIns.name);
			strcpy(psValue, sCfgIns.name);
		}
		return;
	}
	
	//BIZ_DATA_DBG("GetSensorAlarmBaseNameCur err\n");
	return;
}

int GetSensorAlarmBaseChnAllCheck()
{
	return 1;
}


// === AlarmOut ===
// alarm out
int GetAlarmOutAllChnCheckCur()
{
	return 0;
}

int GetAlarmOutNameCurDefault(int ch, char* szCur) // ch not used now
{
	int rtn = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmOutPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_ALARMOUTPARA;
	sParaTgtIns.nChn = ch;
	rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		if(szCur)
			strcpy(szCur, sCfgIns.name);

		return 0;
	}
	
	BIZ_DATA_DBG("GetAlarmOutNameCur err\n");
	return -1;
}

int GetAlarmOutNameCur(int ch, char* szCur) // ch not used now
{
	int rtn = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmOutPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_ALARMOUTPARA;
	sParaTgtIns.nChn = ch;
	rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		if(szCur)
			strcpy(szCur, sCfgIns.name);

		return 0;
	}
	
	BIZ_DATA_DBG("GetAlarmOutNameCur err\n");
	return -1;
}

int GetAlarmOutDelayTimeList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	int i;
	s8* pTmpList[CBX_MAX];
	
	u8 nReal;
	
	if(nMaxNum>CBX_MAX)
	{
		nMaxNum = CBX_MAX;
	}
	
	for(i=0; i<nMaxNum; i++)
	{
		pTmpList[i] = psValueList[i].strDisplay;
	}
	// 
	if( 0==BizConfigGetParaStr(
				EM_BIZ_CFG_PARALIST_DELAY, 
				pTmpList, 
				&nReal, 
				nMaxNum, 
				EDIT_MAX 
		) 
	)
	{
		*nRealNum = nReal;
		
		BIZ_DATA_DBG("GetAlarmOutDelayTimeList!\n");
	}
	else
	{
		BIZ_DATA_DBG("GetAlarmOutDelayTimeList failed!\n");
	}

	return 0;
}

int GetAlarmOutDelayTimeCurDefault(int ch) // 0 all, 1 chn
{	
	int rtn = 0;
	u8 nIdx = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmOutPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_ALARMOUTPARA;
	sParaTgtIns.nChn = ch;
	rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{ 
		if( 0==BizConfigGetParaListIndex(
					EM_BIZ_CFG_PARALIST_DELAY,
					sCfgIns.nDelay,
					&nIdx
			) 
		)
		{
			BIZ_DATA_DBG("GetAlarmOutDelayTimeCur!\n");
		}
		else
		{
			BIZ_DATA_DBG("GetAlarmOutDelayTimeCur failed!\n");
		}
		
		return nIdx;
	}

	return 0;
	
}

int GetAlarmOutDelayTimeCur(int ch) // 0 all, 1 chn
{	
	int rtn = 0;
	u8 nIdx = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmOutPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_ALARMOUTPARA;
	sParaTgtIns.nChn = ch;
	rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{ 
		if( 0==BizConfigGetParaListIndex(
					EM_BIZ_CFG_PARALIST_DELAY,
					sCfgIns.nDelay,
					&nIdx
			) 
		)
		{
			BIZ_DATA_DBG("GetAlarmOutDelayTimeCur!\n");
		}
		else
		{
			BIZ_DATA_DBG("GetAlarmOutDelayTimeCur failed!\n");
		}
		
		return nIdx;
	}

	return 0;
	
}

// sch
int GetAlarmOutSCHChnList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
#if 1
	int i;

	u8 nAlaNum = GetAlarmNum();

	for(i=0; i<nAlaNum; i++)
	{
		sprintf( psValueList[i].strDisplay, "%d", i+1 );
	}
	
	*nRealNum = nAlaNum;

	if(1)
	{
		BIZ_DATA_DBG("GetAlarmOutSCHChnList!\n");
	}
	else
	{
		BIZ_DATA_DBG("GetAlarmOutSCHChnList failed!\n");
	}
		
#endif
	return -1;
}

int GetAlarmOutSCHCurDefault(int ch, uint sch[]) // 7days
{
	int rtn = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizSchedule sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	// clear all of the weekday
	if(sch)
	{
		memset(sch, 0, 7*sizeof(uint));
	}
	
	sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_ALARMOUT;
	sParaTgtIns.nChn = ch;
	rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
	
	if(0==rtn)
	{
		bizData_GetSch(&sCfgIns, sch);
		
		return 0;
	}
	
	BIZ_DATA_DBG("GetAlarmOutSCHCur err\n");
	
	return -1;
}

int GetAlarmOutSCHCur(int ch, uint sch[]) // 7days
{
	int rtn = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizSchedule sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	// clear all of the weekday
	if(sch)
	{
		memset(sch, 0, 7*sizeof(uint));
	}
	
	sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_ALARMOUT;
	sParaTgtIns.nChn = ch;
	rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	
	if(0==rtn)
	{
		bizData_GetSch(&sCfgIns, sch);
		
		return 0;
	}
	
	BIZ_DATA_DBG("GetAlarmOutSCHCur err\n");
	
	return -1;
}

int GetAlarmOutSCHApplyToList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	if(psValueList&&nRealNum)
	{
		//csp modify:报警输出这里是不是有bug
		const char *szStrIns[] = {
			"1",/*"2","3","4","&CfgPtn.All"*/
		};
		
		*nRealNum = 1;
		
		if(*nRealNum>nMaxNum) *nRealNum = nMaxNum;
		
		for(int i=0; i<*nRealNum; i++)
		{
			strcpy(psValueList[i].strDisplay,szStrIns[i]);
		}
	}	
}

int GetAlarmOutSCHApplyToCur()
{
	return 0;
}

// soundalarm
int GetAlarmOutSoundAlarmDelayTimeList(SValue* psValueList, int* nRealNum, int nMaxNum)
{
	int i;
	s8* pTmpList[CBX_MAX];
	u8 nReal = 0;
	
	if(nMaxNum>CBX_MAX)
	{
		nMaxNum = CBX_MAX;
	}
	
	for(i=0; i<nMaxNum; i++)
	{
		pTmpList[i] = psValueList[i].strDisplay;
	}
	// 
	if( 0==BizConfigGetParaStr(
				EM_BIZ_CFG_PARALIST_DELAY, 
				pTmpList, 
				(u8*)&nReal, 
				nMaxNum, 
				EDIT_MAX 
		) 
	)
	{
		*nRealNum = nReal;
		BIZ_DATA_DBG("GetAlarmOutSoundAlarmDelayTimeList!\n");
	}
	else
	{
		*nRealNum = 0;
		BIZ_DATA_DBG("GetAlarmOutSoundAlarmDelayTimeList failed!\n");
	}
}

int GetAlarmOutSoundAlarmDelayTimeCurDefault()
{
	int rtn = 0;
	u8 nIdx = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmBuzzPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_ALARMBUZZPARA;
	sParaTgtIns.nChn = 0;
	rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{ 
		if( 0==BizConfigGetParaListIndex(
					EM_BIZ_CFG_PARALIST_DELAY,
					sCfgIns.nDelay,
					&nIdx
			) 
		)
		{
			BIZ_DATA_DBG("GetAlarmOutSoundAlarmDelayTimeCur!\n");
			printf("GetAlarmOutSoundAlarmDelayTimeCur idx %d, value %d\n", nIdx, sCfgIns.nDelay);
		}
		else
		{
			BIZ_DATA_DBG("GetAlarmOutSoundAlarmDelayTimeCur failed!\n");
		}
		
		return nIdx;
	}
	
	
	BIZ_DATA_DBG("GetAlarmOutSoundAlarmDelayTimeCur err\n");
	
	return -1;
}

int GetAlarmOutSoundAlarmDelayTimeCur()
{
	int rtn = 0;
	u8 nIdx = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmBuzzPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_ALARMBUZZPARA;
	sParaTgtIns.nChn = 0;
	rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{ 
		if( 0==BizConfigGetParaListIndex(
					EM_BIZ_CFG_PARALIST_DELAY,
					sCfgIns.nDelay,
					&nIdx
			) 
		)
		{
			BIZ_DATA_DBG("GetAlarmOutSoundAlarmDelayTimeCur!\n");
			printf("GetAlarmOutSoundAlarmDelayTimeCur idx %d, value %d\n", nIdx, sCfgIns.nDelay);
		}
		else
		{
			BIZ_DATA_DBG("GetAlarmOutSoundAlarmDelayTimeCur failed!\n");
		}
		
		return nIdx;
	}
	
	
	BIZ_DATA_DBG("GetAlarmOutSoundAlarmDelayTimeCur err\n");
	
	return -1;
}

int GetAlarmOutSoundAlarmCheckDefault()
{
	int rtn = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmBuzzPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_ALARMBUZZPARA;
	sParaTgtIns.nChn = 0;
	rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		BIZ_DO_DBG("GetAlarmOutSoundAlarmCheck %d\n", sCfgIns.nEnable);
		return sCfgIns.nEnable;
	}
	
	BIZ_DATA_DBG("GetAlarmOutSoundAlarmCheck err\n");
	
	return -1;
}

int GetAlarmOutSoundAlarmCheck()
{
	int rtn = 0;
	SBizParaTarget sParaTgtIns;
	
	SBizAlarmBuzzPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_ALARMBUZZPARA;
	sParaTgtIns.nChn = 0;
	rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0==rtn)
	{
		BIZ_DO_DBG("GetAlarmOutSoundAlarmCheck %d\n", sCfgIns.nEnable);
		return sCfgIns.nEnable;
	}
	
	BIZ_DATA_DBG("GetAlarmOutSoundAlarmCheck err\n");
	
	return -1;
}


/*
	nItemID - GUI item only label
	sGsr    - struct parameter to get item value
*/

int GetScreenStrSelCHKORIDX(u32 nItemID, u8 chn )
{
	u8	nIdx = 0;
	int rtn = 0;
	SBizParaTarget sParaTgtIns;

	if(nItemID>=GSR_DEFAULT_OFFSET)
	{
		switch(nItemID)
		{	
		#if 0
			case GSR_CONFIG_BASIC_SYSTEM_DEVICENAME       + GSR_DEFAULT_OFFSET : 
				{;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+0
			case GSR_CONFIG_BASIC_SYSTEM_DEVICEID       + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_BASIC_SYSTEM_VIDEOFORMAT      + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_BASIC_SYSTEM_AUTHCHECK      + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_BASIC_SYSTEM_TIMEDISPLAY      + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_BASIC_SYSTEM_MAXNETUSERS      + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_BASIC_SYSTEM_OUTPUT         + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_BASIC_SYSTEM_LANGUAGE       + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_BASIC_SYSTEM_SCRENNSAVER      + GSR_DEFAULT_OFFSET : 
			
			
			//date&time..
			
			case GSR_CONFIG_BASIC_DATETIME_DATEFORMAT         + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_BASIC_DATETIME_TIMEFORMAT         + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_BASIC_DATETIME_TIMEZONE         + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_BASIC_DATETIME_NETSYN         + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_BASIC_DATETIME_NTPSERVER          + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_BASIC_DATETIME_SYSDATE          + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_BASIC_DATETIME_SYSTIME          + GSR_DEFAULT_OFFSET : 
			
			
			//-------------------------------live
			
			//case GSR_CONFIG_LIVE_LIVE             + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_LIVE_ALLCHN             + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_LIVE_CHNNAME            + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_LIVE_DISPLAY            + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_LIVE_LIVEREC            + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_LIVE_COLORSET             + GSR_DEFAULT_OFFSET : 
			
			//case GSR_CONFIG_LIVE_MAINMONITOR            + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_LIVE_MAINMONITOR_SPLITMODE      + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_LIVE_MAINMONITOR_CHN      + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_LIVE_MAINMONITOR_DWELLTIME      + GSR_DEFAULT_OFFSET : 
			
			//case GSR_CONFIG_LIVE_SPOT             + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_LIVE_SPOT_SPLITMODE             + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_LIVE_SPOT_CHN             + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_LIVE_SPOT_DWELLTIME             + GSR_DEFAULT_OFFSET : 
			
			//case GSR_CONFIG_LIVE_MASK             + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_LIVE_MASK_AREA            + GSR_DEFAULT_OFFSET : 
			
			
			
			//-------------------------------record
			
			
			case GSR_CONFIG_RECORD_ENABLE_ALLCHN        + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_RECORD_ENABLE_RECORD        + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_RECORD_ENABLE_AUDIO         + GSR_DEFAULT_OFFSET : 
			
			
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ALLCHN         + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_RESOLUTION         + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_FRAMERATE        + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ENCODE         + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_QUALITY        + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_MAXRATE      + GSR_DEFAULT_OFFSET : 
			
			
			case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_ALLCHN       + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEBEFALARM   + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEAFTALARM       + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_EXPIRATION       + GSR_DEFAULT_OFFSET : 
			
			
			case GSR_CONFIG_RECORD_ENABLE_OSD_ALLCHN        + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_RECORD_ENABLE_OSD_CHNNAME         + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_RECORD_ENABLE_OSD_TIMESTAMP         + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_RECORD_ENABLE_OSD_POSITION        + GSR_DEFAULT_OFFSET : 
			
			
			case GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_REC        + GSR_DEFAULT_OFFSET : 
			
			
			
			
			
			//-------------------------------schedule
			
			//case GSR_CONFIG_SCHEDULE_TIMEREC  + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_SCHEDULE_TIMEREC_CHN  + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_SCHEDULE_TIMEREC_SCH  + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_SCHEDULE_TIMEREC_COPYTO   + GSR_DEFAULT_OFFSET : 
			
			
			//case GSR_CONFIG_SCHEDULE_MOTIONALARMREC   + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_CHN   + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH   + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_COPYTO  + GSR_DEFAULT_OFFSET : 
			
			//case GSR_CONFIG_SCHEDULE_SENSORALARMREC   + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_SCHEDULE_SENSORALARMREC_CHN   + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_SCHEDULE_SENSORALARMREC_SCH   + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_SCHEDULE_SENSORALARMREC_COPYTO  + GSR_DEFAULT_OFFSET : 
			
			
			//-------------------------------alarm
			
			//case GSR_CONFIG_ALARM_SENSOR    + GSR_DEFAULT_OFFSET : 
			
			case GSR_CONFIG_ALARM_SENSOR_BASE_ALLCHN    + GSR_DEFAULT_OFFSET : 
			{
				SBizPreviewImagePara sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_IMAGEPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nHue;
				}
				else
				{
					BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA err\n");
				}
			} 
			break;  // GSR_CONFIG_ALARM_SENSOR_BASE+0
			case GSR_CONFIG_ALARM_SENSOR_BASE_ENABLE    + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmSensorPara sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nEnable;
				}
				else
				{
					BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA err\n");
				}
			} 
			break;   // GSR_CONFIG_ALARM_SENSOR_BASE+1
			case GSR_CONFIG_ALARM_SENSOR_BASE_TYPE    + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmSensorPara sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.emType;
				}
				else
				{
					BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA err\n");
				}
			} 
			break;    // GSR_CONFIG_ALARM_SENSOR_BASE+2
			case GSR_CONFIG_ALARM_SENSOR_BASE_NAME    + GSR_DEFAULT_OFFSET : 
			
			
			#endif
			
			//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL    + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALLCHN     + GSR_DEFAULT_OFFSET : 
			{
				return 0;
			} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+0
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME    + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmSensorPara sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					if( 0==BizConfigGetParaListIndex(
								EM_BIZ_CFG_PARALIST_DELAY,
								sCfgIns.nDelay,
								&nIdx
						) 
					)
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME!\n");
						return nIdx;
					}
					else
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME failed!\n");
					}
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME err\n");
				}
			} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+1
			
			//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+ GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nFlagBuzz;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND err\n");
				}
			} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+0
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO  + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nZoomChn;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO err\n");
				}
			} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+1
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					uint nTigChn = 0;
					
					for(int i=0; i<32; i++)
					{
						nTigChn |= (0x1<<sCfgIns.nAlarmOut[i]);
					}
					
					return nTigChn;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO err\n");
				}
			} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+2
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL  + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{				
					return sCfgIns.nFlagEmail;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL err\n");
				}
			} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+3
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP  + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					uint nTigChn = 0;
					
					for(int i=0; i<32; i++)
					{
						nTigChn |= (0x1<<sCfgIns.nSnapChn[i]);
					}
					
					return nTigChn;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP err\n");
				}
			} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+4 // bitmap
			
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER  + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					uint nTigChn = 0;
					
					for(int i=0; i<32; i++)
					{
						nTigChn |= (0x1<<sCfgIns.nRecordChn[i]);
					}
					
					return nTigChn;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER err\n");
				}
			} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP+1
			
			//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE  + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{ 
					if( 0==BizConfigGetParaListIndex(
								EM_BIZ_CFG_PARALIST_PTZLINKTYPE,
								sCfgIns.sAlarmPtz[chn].nALaPtzType,
								&nIdx
						) 
					)
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
					}
					else
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
					}
					
					return nIdx;
				}
			}
			
			break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+0
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX  + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{ 
				#if 1 // debug by lanston
					nIdx = sCfgIns.sAlarmPtz[chn].nId;
				#else
					if( 0==BizConfigGetParaListIndex(
								EM_BIZ_CFG_PARALIST_DELAY,
								sCfgIns.sAlarmPtz[chn].nId,
								&nIdx
						) 
					)
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
					}
					else
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
					}
				#endif
					
					return nIdx;
				}
			} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1

			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE  + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmVMotionPara sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nEnable;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE err\n");
				}
			} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+0
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmVMotionPara sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{ 
					if( 0==BizConfigGetParaListIndex(
								EM_BIZ_CFG_PARALIST_DELAY,
								sCfgIns.nDelay,
								&nIdx
						) 
					)
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
					}
					else
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
						return -1;
					}
					
					return nIdx;
				}
				else
				{
					return -1;
				}
			} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+1
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALLCHN  + GSR_DEFAULT_OFFSET : 
			
			//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL + GSR_DEFAULT_OFFSET : 
			
			
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND  + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nFlagBuzz;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND err\n");
				}
			} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+0
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nZoomChn;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND err\n");
				}
			} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+1
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_TIRGGER  + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					uint nTigChn = 0;
					
					for(int i=0; i<32; i++)
					{
						nTigChn |= (0x1<<sCfgIns.nAlarmOut[i]);
					}
					
					return nTigChn;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND err\n");
				}
			} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+2
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nFlagEmail;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL err\n");
				}
			} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+3
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					uint nTigChn = 0;
					
					for(int i=0; i<32; i++)
					{
						nTigChn |= (0x1<<sCfgIns.nSnapChn[i]);
					}
					
					return nTigChn;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL err\n");
				}
			} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+4 // bitmap
			
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_TRIGGER + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					uint nTigChn = 0;
					
					for(int i=0; i<32; i++)
					{
						nTigChn |= (0x1<<sCfgIns.nRecordChn[i]);
					}
					
					return nTigChn;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL err\n");
				}
			} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x10
			
			//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_TYPE  + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{ 
					if( 0==BizConfigGetParaListIndex(
								EM_BIZ_CFG_PARALIST_PTZLINKTYPE,
								sCfgIns.sAlarmPtz[chn].nALaPtzType,
								&nIdx
						) 
					)
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
					}
					else
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
					}
					
					return nIdx;
				}
			}break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+0
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_INDEX + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{ 
				#if 1 // debug by lanston
					nIdx = sCfgIns.sAlarmPtz[chn].nId;
				#else
					if( 0==BizConfigGetParaListIndex(
								EM_BIZ_CFG_PARALIST_DELAY,
								sCfgIns.sAlarmPtz[chn].nId,
								&nIdx
						) 
					)
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
					}
					else
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
					}
				#endif
					
					return nIdx;
				}
			} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+1
			
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET  + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmVMotionPara sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					uint nTigChn = 0;
					
					for(int i=0; i<32; i++)
					{
						nTigChn |= (0x1<<sCfgIns.nBlockStatus[i]);
					}
					
					return nTigChn;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET err\n");
				}
			} break; // GSR_CONFIG_ALARM_MD+0x28
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET_SENSITY+ GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmVMotionPara sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nSensitivity;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET_SENSITY err\n");
				}
			} break; // GSR_CONFIG_ALARM_MD+0x28
			
			
			//case GSR_CONFIG_ALARM_MD_SCH      + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_ALARM_MD_SCH_SCH_CHN  + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_ALARM_MD_SCH_SCH_SCH  + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_ALARM_MD_SCH_SCH_COPYTO   + GSR_DEFAULT_OFFSET : 
			
			
			//case GSR_CONFIG_ALARM_VIDEOLOST     + GSR_DEFAULT_OFFSET : 
			
			//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL + GSR_DEFAULT_OFFSET : 
			
			
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND  + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nFlagBuzz;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND err\n");
				}
			} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+0
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nZoomChn;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO err\n");
				}
			} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+1
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER  + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					uint nTigChn = 0;
					
					for(int i=0; i<32; i++)
					{
						nTigChn |= (0x1<<sCfgIns.nAlarmOut[i]);
					}
					
					return nTigChn;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER err\n");
				}
			} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+2
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nFlagEmail;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL err\n");
				}
			} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+3
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					uint nTigChn = 0;
					
					for(int i=0; i<32; i++)
					{
						nTigChn |= (0x1<<sCfgIns.nSnapChn[i]);
					}
					
					return nTigChn;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP err\n");
				}
			} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+4 // bitmap
			
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					uint nTigChn = 0;
					
					for(int i=0; i<32; i++)
					{
						nTigChn |= (0x1<<sCfgIns.nRecordChn[i]);
					}
					
					return nTigChn;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER err\n");
				}
			} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x10
			
			//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_TYPE  + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{ 
				#if 0 // debug by lanston
					nIdx = sCfgIns.sAlarmPtz[chn].nId;
				#else
					if( 0==BizConfigGetParaListIndex(
								EM_BIZ_CFG_PARALIST_PTZLINKTYPE,
								sCfgIns.sAlarmPtz[chn].nALaPtzType,
								&nIdx
						) 
					)
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
					}
					else
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
					}
				#endif
					
					return nIdx;
				}
			} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_INDEX + GSR_DEFAULT_OFFSET : 
			{
				SBizAlarmDispatch sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{ 
				#if 1 // debug by lanston
					nIdx = sCfgIns.sAlarmPtz[chn].nId;
				#else
					if( 0==BizConfigGetParaListIndex(
								EM_BIZ_CFG_PARALIST_PTZLINKTYPE,
								sCfgIns.sAlarmPtz[chn].emALaPtzType,
								&nIdx
						) 
					)
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
					}
					else
					{
						BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
					}
				#endif
					
					return nIdx;
				}
			} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+1
			
			
			//case GSR_CONFIG_ALARM_OTHER           + GSR_DEFAULT_OFFSET : 
			
			case GSR_CONFIG_ALARM_OTHER_ALARMTYPE     + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_ALARM_OTHER_SOUNDALARM    + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_ALARM_OTHER_MAIL        + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_ALARM_OTHER_TRIGGERALARM    + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_ALARM_OTHER_DISKALARM     + GSR_DEFAULT_OFFSET : 
			case GSR_CONFIG_ALARM_OTHER_ALLCHN      + GSR_DEFAULT_OFFSET : 
			
			
			case GSR_CONFIG_ALARM_ALARMOUT    + GSR_DEFAULT_OFFSET : 
			
			
			case GSR_CONFIG_ALARM_EXIT    + GSR_DEFAULT_OFFSET : 
			
			
			//-------------------------------network
			
			case GSR_CONFIG_NETWORK_BASE    + GSR_DEFAULT_OFFSET : 
			
			
			//-------------------------------user
			
			case GSR_CONFIG_USER_BASE       + GSR_DEFAULT_OFFSET : 
			
			
			//-------------------------------ptz
			
			case GSR_PTZCONFIG_SERIAL_ENABLE    + GSR_DEFAULT_OFFSET : 
			{
				SBizPtzPara sCfgIns;
				
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nEnable;
				}

				return rtn;
				BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_ENABLE err\n");
				
			} break; // GSR_PTZCONFIG_BASE+1
			case GSR_PTZCONFIG_SERIAL_ADDRESS     + GSR_DEFAULT_OFFSET : 
			{
				//return 0;
				{
					SBizPtzPara sCfgIns;
					
					memset(&sCfgIns, 0, sizeof(sCfgIns));
					sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
					sParaTgtIns.nChn = chn;
					rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);				
					if(0==rtn)
					{ 
						#if 1 // debug temp
						return sCfgIns.nCamAddr;
						#else
						if( 0==BizConfigGetParaListIndex(
									EM_BIZ_CFG_PARALIST_BAUDRATE,
									sCfgIns.nCamAddr,
									&nIdx
							) 
						)
						{
							BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_ADDRESS!\n");
						}
						else
						{
							BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_ADDRESS failed!\n");
						}
						return nIdx;
						#endif
					}
				}
				
				BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_ADDRESS err\n");
				
			} break; // GSR_PTZCONFIG_BASE+2
			case GSR_PTZCONFIG_SERIAL_BPS     + GSR_DEFAULT_OFFSET : 
			{
				//return 0;
				
				SBizPtzPara sCfgIns;
				
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);		
				if(0==rtn)
				{ 
					if( 0==BizConfigGetParaListIndex(
								EM_BIZ_CFG_PARALIST_BAUDRATE,
								sCfgIns.nBaudRate,
								&nIdx
						) 
					)
					{
						BIZ_DT_DBG("GSR_PTZCONFIG_SERIAL_BPS val %d idx %d\n", sCfgIns.nBaudRate, nIdx);
						BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_BPS!\n");
					}
					else
					{
						BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_BPS failed!\n");
					}
					
					return nIdx;
				}
				
				BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_BPS err\n");
				
			} break; // GSR_PTZCONFIG_BASE+3
			
			case GSR_PTZCONFIG_SERIAL_PROTOCOL    + GSR_DEFAULT_OFFSET : 
			{
				//return 0;
				
				SBizPtzPara sCfgIns;
				
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);		
				if(0==rtn)
				{ 
					if( 0==BizConfigGetParaListIndex(
								EM_BIZ_CFG_PARALIST_PTZPROTOCOL,
								sCfgIns.nProtocol,
								&nIdx
						) 
					)
					{
						BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_PROTOCOL!\n");
					}
					else
					{
						BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_PROTOCOL failed!\n");
					}
					
					return nIdx;
				}
				
				BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_BPS err\n");
				
			} break; // GSR_PTZCONFIG_BASE+4
			case GSR_PTZCONFIG_SERIAL_ALLCHECK    + GSR_DEFAULT_OFFSET : 
			
			//+ GSR_DEFAULT_OFFSET : 
			
			//+ GSR_DEFAULT_OFFSET : 
			
			case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE    + GSR_DEFAULT_OFFSET : 
			{
				SBizPtzPara sCfgIns;
				
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{	
					if(chn<128)
						return sCfgIns.sAdvancedPara.nIsPresetSet[chn];
				}
				
				BIZ_DATA_DBG("GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE err\n");
				
			} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+1
			case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ID    + GSR_DEFAULT_OFFSET : 
			{
				SBizPtzPara sCfgIns;
				
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{	
					if(chn<128)
						return sCfgIns.sAdvancedPara.sTourPath[chn].nPathNo;
				}
				
				BIZ_DATA_DBG("GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ID err\n");
				
			} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+2
			
			//+ GSR_DEFAULT_OFFSET : 
			
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_ID  + GSR_DEFAULT_OFFSET : 
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_NAME  + GSR_DEFAULT_OFFSET : 
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_PRESET+ GSR_DEFAULT_OFFSET : 
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_RATE  + GSR_DEFAULT_OFFSET : 
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_TIME  + GSR_DEFAULT_OFFSET : 
			
			
			//+ GSR_DEFAULT_OFFSET : 
			
			
			
			//-------------------------------advance
			//case GSR_CONFIG_ADVANCE     + GSR_DEFAULT_OFFSET : 
			
			
			case GSR_ADVANCECONFIG_IMPORTEXPORT   + GSR_DEFAULT_OFFSET : 
			case GSR_ADVANCECONFIG_IMPORTEXPORT_REFRESH   + GSR_DEFAULT_OFFSET : 
			
			
			// === search ===
			
			// === backup ===
			
			// === info ===
			
			//-------------------------------system
			
			case GSR_INFO_SYSTEM_DEVICENAME   + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_SYSTEM_DEVICENUM  + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_SYSTEM_HARDWAREVERSION  + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_SYSTEM_SCMVERSION   + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_SYSTEM_KERNELVERSION  + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_SYSTEM_ROMVERSION   + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_SYSTEM_RELEASEDATE  + GSR_DEFAULT_OFFSET : 
			
			
			case GSR_INFO_DISK_NO   + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_DISK_TYPE   + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_DISK_CAPACITY   + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_DISK_FREE   + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_DISK_STATE  + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_DISK_ATTRIBUTE  + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_DISK_SOURCE   + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_DISK_RSV1   + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_DISK_RSV2   + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_DISK_IMAGE  + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_DISK_TOTAL  + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_DISK_USED   + GSR_DEFAULT_OFFSET : 
			
			
			//-------------------------------event
			
			
			//-------------------------------syslog
			
			
			//------------------------------- netinfo
			
			
			//------------------------------- online
			
			case GSR_INFO_ONLINE_USERNAME + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_ONLINE_IP     + GSR_DEFAULT_OFFSET : 
			case GSR_INFO_ONLINE_STATE  + GSR_DEFAULT_OFFSET : 
			
			// exit
			
			
			
			// === alarm ===
			
			
			case GSR_MANUALALARM_ALLCHECK + GSR_DEFAULT_OFFSET : 
			case GSR_MANUALALARM_ALARMNAME+ GSR_DEFAULT_OFFSET : 
			case GSR_MANUALALARM_IP     + GSR_DEFAULT_OFFSET : 
			case GSR_MANUALALARM_ALARMPROCESS + GSR_DEFAULT_OFFSET : 
			
			// === disk ===
			
			
			
			// === resource ===
			
			
			
			// === logoff ===
			
			
			
			// === shutdown ===
			
			
			// === desktop ===
			
			case GSR_DESKTOP_SYSTIME  + GSR_DEFAULT_OFFSET : 
			case GSR_DESKTOP_CHNNAME  + GSR_DEFAULT_OFFSET : 
			
			//case GSR_DESKTOP_VIDEO_ATTRIBUTE  + GSR_DEFAULT_OFFSET : 
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN  + GSR_DEFAULT_OFFSET : 
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS   + GSR_DEFAULT_OFFSET : 
			{
				SBizPreviewImagePara sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_IMAGEPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS ok\n");
					return sCfgIns.nBrightness;
				}
				else
				{
					BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS err\n");
				}
			} 
			break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+1
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA   + GSR_DEFAULT_OFFSET : 
			{
				SBizPreviewImagePara sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_IMAGEPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nHue;
				}
				else
				{
					BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA err\n");
				}
			} 
			break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+2
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION   + GSR_DEFAULT_OFFSET : 
			{
				SBizPreviewImagePara sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_IMAGEPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nSaturation;
				}
				else
				{
					BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION err\n");
				}
			} 
			break;  // GSR_DESKTOP_VIDEO_ATTRIBUTE+3
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_CONTRAST   + GSR_DEFAULT_OFFSET : 
			{
				SBizPreviewImagePara sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_IMAGEPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					return sCfgIns.nContrast;
				}
				else
				{
					BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION err\n");
				}
			} 
			break;   // GSR_DESKTOP_VIDEO_ATTRIBUTE+4
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS   + GSR_DEFAULT_OFFSET :
				{
					SBizPreviewImagePara sCfgIns;
					memset(&sCfgIns, 0, sizeof(sCfgIns));
					sParaTgtIns.emBizParaType = EM_BIZ_VO_IMAGEPARA;
					sParaTgtIns.nChn = chn;
					rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
					if(0==rtn)
					{
						BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_VO_ATTRIBUTE_BRIGHTNESS ok\n");
						return sCfgIns.nBrightness;
					}
					else
					{
						BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_VO_ATTRIBUTE_BRIGHTNESS err\n");
					}
				}
				break;// GSR_DESKTOP_VIDEO_ATTRIBUTE+6
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_CHROMA   + GSR_DEFAULT_OFFSET :
				{
					SBizPreviewImagePara sCfgIns;
					memset(&sCfgIns, 0, sizeof(sCfgIns));
					sParaTgtIns.emBizParaType = EM_BIZ_VO_IMAGEPARA;
					sParaTgtIns.nChn = chn;
					rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
					if(0==rtn)
					{
						return sCfgIns.nHue;
					}
					else
					{
						BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_CHROMA err\n");
					}
				}
				break;// GSR_DESKTOP_VIDEO_ATTRIBUTE+7
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_SATURATION   + GSR_DEFAULT_OFFSET :
				{
					SBizPreviewImagePara sCfgIns;
					memset(&sCfgIns, 0, sizeof(sCfgIns));
					sParaTgtIns.emBizParaType = EM_BIZ_VO_IMAGEPARA;
					sParaTgtIns.nChn = chn;
					rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
					if(0==rtn)
					{
						return sCfgIns.nSaturation;
					}
					else
					{
						BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_SATURATION err\n");
					}
				}
				break;// GSR_DESKTOP_VIDEO_ATTRIBUTE+8
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_CONTRAST   + GSR_DEFAULT_OFFSET :
				{
					SBizPreviewImagePara sCfgIns;
					memset(&sCfgIns, 0, sizeof(sCfgIns));
					sParaTgtIns.emBizParaType = EM_BIZ_VO_IMAGEPARA;
					sParaTgtIns.nChn = chn;
					rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
					if(0==rtn)
					{
						return sCfgIns.nContrast;
					}
					else
					{
						BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_CONTRAST err\n");
					}
				}
				break;// GSR_DESKTOP_VIDEO_ATTRIBUTE+9
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_DEFAULT  + GSR_DEFAULT_OFFSET : 
				break;
			case GSR_CONFIG_NETWORK_ADVANCE_DDNSUPINTVL  + GSR_DEFAULT_OFFSET : 
			{
				SBizNetPara sCfgIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_NETPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					if( 0==BizConfigGetParaListIndex(
								EM_BIZ_CFG_PARALIST_DDNSIPUPDATEITVL,
								sCfgIns.UpdateIntvl,
								&nIdx
						) 
					)
					{
						BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_PROTOCOL!\n");
					}
					else
					{
						BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_PROTOCOL failed!\n");
					}
					
					return nIdx;
				}	
			} break;
		}
		
		return 0;
	}
	
	switch(nItemID)
	{	
	#if 0
		case GSR_CONFIG_BASIC_SYSTEM_DEVICENAME       : 
			{;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+0
		case GSR_CONFIG_BASIC_SYSTEM_DEVICEID       : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+1
		case GSR_CONFIG_BASIC_SYSTEM_VIDEOFORMAT      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+2
		case GSR_CONFIG_BASIC_SYSTEM_AUTHCHECK      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+3
		case GSR_CONFIG_BASIC_SYSTEM_TIMEDISPLAY      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+4
		case GSR_CONFIG_BASIC_SYSTEM_MAXNETUSERS      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+5
		case GSR_CONFIG_BASIC_SYSTEM_OUTPUT         : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+6
		case GSR_CONFIG_BASIC_SYSTEM_LANGUAGE       : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+7
		case GSR_CONFIG_BASIC_SYSTEM_SCRENNSAVER      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+8
		
		
		//date&time..
		
		case GSR_CONFIG_BASIC_DATETIME_DATEFORMAT         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+0
		case GSR_CONFIG_BASIC_DATETIME_TIMEFORMAT         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+1
		case GSR_CONFIG_BASIC_DATETIME_TIMEZONE         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+2
		case GSR_CONFIG_BASIC_DATETIME_NETSYN         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+3
		case GSR_CONFIG_BASIC_DATETIME_NTPSERVER          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+4
		case GSR_CONFIG_BASIC_DATETIME_SYSDATE          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+5
		case GSR_CONFIG_BASIC_DATETIME_SYSTIME          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+6
		
		
		//-------------------------------live
		
		//case GSR_CONFIG_LIVE_LIVE             : {;} break; // GSR_CONFIG_LIVE_BASE+0
		case GSR_CONFIG_LIVE_ALLCHN             : {;} break; // GSR_CONFIG_LIVE_BASE+1
		case GSR_CONFIG_LIVE_CHNNAME            : {;} break; // GSR_CONFIG_LIVE_BASE+2
		case GSR_CONFIG_LIVE_DISPLAY            : {;} break; // GSR_CONFIG_LIVE_BASE+3
		case GSR_CONFIG_LIVE_LIVEREC            : {;} break; // GSR_CONFIG_LIVE_BASE+4
		case GSR_CONFIG_LIVE_COLORSET             : {;} break; // GSR_CONFIG_LIVE_BASE+5
		
		//case GSR_CONFIG_LIVE_MAINMONITOR            : {;} break; // GSR_CONFIG_LIVE_BASE+0x40
		case GSR_CONFIG_LIVE_MAINMONITOR_SPLITMODE      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+0
		case GSR_CONFIG_LIVE_MAINMONITOR_CHN      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+1
		case GSR_CONFIG_LIVE_MAINMONITOR_DWELLTIME      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+2
		
		//case GSR_CONFIG_LIVE_SPOT             : {;} break; // GSR_CONFIG_LIVE_BASE+0x80
		case GSR_CONFIG_LIVE_SPOT_SPLITMODE             : {;} break; // GSR_CONFIG_LIVE_SPOT+0
		case GSR_CONFIG_LIVE_SPOT_CHN             : {;} break; // GSR_CONFIG_LIVE_SPOT+1
		case GSR_CONFIG_LIVE_SPOT_DWELLTIME             : {;} break; // GSR_CONFIG_LIVE_SPOT+2
		
		//case GSR_CONFIG_LIVE_MASK             : {;} break; // GSR_CONFIG_LIVE_BASE+0xc0
		case GSR_CONFIG_LIVE_MASK_AREA            : {;} break; // GSR_CONFIG_LIVE_MASK+0
		
		
		
		//-------------------------------record
		
		
		case GSR_CONFIG_RECORD_ENABLE_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_RECORD        : {;} break; // GSR_CONFIG_RECORD_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_AUDIO         : {;} break; // GSR_CONFIG_RECORD_BASE+2
		
		
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ALLCHN         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_RESOLUTION         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_FRAMERATE        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ENCODE         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+3
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_QUALITY        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+4
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_MAXRATE      : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+5
		
		
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_ALLCHN       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEBEFALARM   : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEAFTALARM       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_EXPIRATION       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+3
		
		
		case GSR_CONFIG_RECORD_ENABLE_OSD_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_OSD_CHNNAME         : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_OSD_TIMESTAMP         : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_OSD_POSITION        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+3
		
		
		case GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_REC        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_BASE+0
		
		
		
		
		
		//-------------------------------schedule
		
		//case GSR_CONFIG_SCHEDULE_TIMEREC  : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0
		case GSR_CONFIG_SCHEDULE_TIMEREC_CHN  : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+0
		case GSR_CONFIG_SCHEDULE_TIMEREC_SCH  : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+1
		case GSR_CONFIG_SCHEDULE_TIMEREC_COPYTO   : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+2
		
		
		//case GSR_CONFIG_SCHEDULE_MOTIONALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x20
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+0
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH   : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+1
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+2
		
		//case GSR_CONFIG_SCHEDULE_SENSORALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x30
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+0
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_SCH   : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+1
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+2
		
		
		//-------------------------------alarm
		
		//case GSR_CONFIG_ALARM_SENSOR    : {;} break; // GSR_CONFIG_ALARM_BASE+0
		
		case GSR_CONFIG_ALARM_SENSOR_BASE_ALLCHN    :
		{
			SBizPreviewImagePara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_IMAGEPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nHue;
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA err\n");
			}
		} 
		break;  // GSR_CONFIG_ALARM_SENSOR_BASE+0
		case GSR_CONFIG_ALARM_SENSOR_BASE_ENABLE    :
		{
			SBizAlarmSensorPara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nEnable;
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA err\n");
			}
		} 
		break;   // GSR_CONFIG_ALARM_SENSOR_BASE+1
		case GSR_CONFIG_ALARM_SENSOR_BASE_TYPE    :
		{
			SBizAlarmSensorPara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.emType;
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA err\n");
			}
		} 
		break;    // GSR_CONFIG_ALARM_SENSOR_BASE+2
		case GSR_CONFIG_ALARM_SENSOR_BASE_NAME    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+3
		
		
		#endif
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x20
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALLCHN     : 
		{
			return 0;
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME    : 
		{
			SBizAlarmSensorPara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_DELAY,
							sCfgIns.nDelay,
							&nIdx
					) 
				)
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME!\n");
					return nIdx;
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME failed!\n");
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE: {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND :  
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nFlagBuzz;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO  : 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nZoomChn;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER : 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				uint nTigChn = 0;
				
				for(int i=0; i<32; i++)
				{
					nTigChn |= (0x1<<sCfgIns.nAlarmOut[i]);
				}
				
				return nTigChn;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL  :  
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{				
				return sCfgIns.nFlagEmail;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP  :
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				uint nTigChn = 0;
				
				for(int i=0; i<32; i++)
				{
					nTigChn |= (0x1<<sCfgIns.nSnapChn[i]);
				}
				
				return nTigChn;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER  : 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				uint nTigChn = 0;
				
				for(int i=0; i<32; i++)
				{
					nTigChn |= (0x1<<sCfgIns.nRecordChn[i]);
				}
				
				return nTigChn;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE : 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{ 
				if( 0==BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_PTZLINKTYPE,
							sCfgIns.sAlarmPtz[chn].nALaPtzType,
							&nIdx
					) 
				)
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
				}
				
				return nIdx;
			}
		}
		
		break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX  :		 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{ 
			#if 1 // debug by lanston
				nIdx = sCfgIns.sAlarmPtz[chn].nId;
			#else
				if( 0==BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_DELAY,
							sCfgIns.sAlarmPtz[chn].nId,
							&nIdx
					) 
				)
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
				}
			#endif
				
				return nIdx;
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1

		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE  :  
		{
			SBizAlarmVMotionPara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nEnable;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME : 
		{
			SBizAlarmVMotionPara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{ 
				if( 0==BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_DELAY,
							sCfgIns.nDelay,
							&nIdx
					) 
				)
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
				}
				
				return nIdx;
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALLCHN  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+2
		
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+3
		
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND  : 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nFlagBuzz;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO :
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nZoomChn;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_TIRGGER  : 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				uint nTigChn = 0;
				
				for(int i=0; i<32; i++)
				{
					nTigChn |= (0x1<<sCfgIns.nAlarmOut[i]);
				}
				
				return nTigChn;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL :  
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nFlagEmail;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP : 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				uint nTigChn = 0;
				
				for(int i=0; i<32; i++)
				{
					nTigChn |= (0x1<<sCfgIns.nSnapChn[i]);
				}
				
				return nTigChn;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_TRIGGER :  
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				uint nTigChn = 0;
				
				for(int i=0; i<32; i++)
				{
					nTigChn |= (0x1<<sCfgIns.nRecordChn[i]);
				}
				
				return nTigChn;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_TYPE  : 		 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{ 
				if( 0==BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_PTZLINKTYPE,
							sCfgIns.sAlarmPtz[chn].nALaPtzType,
							&nIdx
					) 
				)
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
				}
				
				return nIdx;
			}
		}break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_INDEX :		 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{ 
			#if 1 // debug by lanston
				nIdx = sCfgIns.sAlarmPtz[chn].nId;
			#else
				if( 0==BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_DELAY,
							sCfgIns.sAlarmPtz[chn].nId,
							&nIdx
					) 
				)
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
				}
			#endif
				
				return nIdx;
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+1
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET  :  
		{
			SBizAlarmVMotionPara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				uint nTigChn = 0;
				
				for(int i=0; i<32; i++)
				{
					nTigChn |= (0x1<<sCfgIns.nBlockStatus[i]);
				}
				
				return nTigChn;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD+0x28
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET_SENSITY:  
		{
			SBizAlarmVMotionPara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nSensitivity;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET_SENSITY err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD+0x28
		
		
		//case GSR_CONFIG_ALARM_MD_SCH      : {;} break; // GSR_CONFIG_ALARM_MD+0x30
		case GSR_CONFIG_ALARM_MD_SCH_SCH_CHN  : {;} break; // GSR_CONFIG_ALARM_MD_SCH+0
		case GSR_CONFIG_ALARM_MD_SCH_SCH_SCH  : {;} break; // GSR_CONFIG_ALARM_MD_SCH+1
		case GSR_CONFIG_ALARM_MD_SCH_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_MD_SCH+2
		
		
		//case GSR_CONFIG_ALARM_VIDEOLOST     : {;} break; // GSR_CONFIG_ALARM_BASE+0x80
		
		//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST+0
		
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND  : 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nFlagBuzz;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND err\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO :  
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nZoomChn;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO err\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER  :  
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				uint nTigChn = 0;
				
				for(int i=0; i<32; i++)
				{
					nTigChn |= (0x1<<sCfgIns.nAlarmOut[i]);
				}
				
				return nTigChn;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER err\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL : 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nFlagEmail;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL err\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP :
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				uint nTigChn = 0;
				
				for(int i=0; i<32; i++)
				{
					nTigChn |= (0x1<<sCfgIns.nSnapChn[i]);
				}
				
				return nTigChn;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP err\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER : 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				uint nTigChn = 0;
				
				for(int i=0; i<32; i++)
				{
					nTigChn |= (0x1<<sCfgIns.nRecordChn[i]);
				}
				
				return nTigChn;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER err\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_TYPE  : 	 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{ 
			#if 0 // debug by lanston
				nIdx = sCfgIns.sAlarmPtz[chn].nId;
			#else
				if( 0==BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_PTZLINKTYPE,
							sCfgIns.sAlarmPtz[chn].nALaPtzType,
							&nIdx
					) 
				)
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
				}
			#endif
				
				return nIdx;
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_INDEX :	 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{ 
			#if 1 // debug by lanston
				nIdx = sCfgIns.sAlarmPtz[chn].nId;
			#else
				if( 0==BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_PTZLINKTYPE,
							sCfgIns.sAlarmPtz[chn].emALaPtzType,
							&nIdx
					) 
				)
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
				}
				else
				{
					BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
				}
			#endif
				
				return nIdx;
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+1
		
		
		//case GSR_CONFIG_ALARM_OTHER           : {;} break; // GSR_CONFIG_ALARM_BASE+0xA0
		
		case GSR_CONFIG_ALARM_OTHER_ALARMTYPE     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+1
		case GSR_CONFIG_ALARM_OTHER_SOUNDALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+2
		case GSR_CONFIG_ALARM_OTHER_MAIL        : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+3
		case GSR_CONFIG_ALARM_OTHER_TRIGGERALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+4
		case GSR_CONFIG_ALARM_OTHER_DISKALARM     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+5
		case GSR_CONFIG_ALARM_OTHER_ALLCHN      : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+6
		
		
		case GSR_CONFIG_ALARM_ALARMOUT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xB0
		
		
		case GSR_CONFIG_ALARM_EXIT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xC0
		
		
		//-------------------------------network
		
		case GSR_CONFIG_NETWORK_BASE    : {;} break; // GSR_CONFIG_BASE+0x500
		
		
		//-------------------------------user
		
		case GSR_CONFIG_USER_BASE       : {;} break; // GSR_CONFIG_BASE+0x600
		
		
		//-------------------------------ptz
		
		case GSR_PTZCONFIG_SERIAL_ENABLE    : 
		{
			SBizPtzPara sCfgIns;
			
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nEnable;
			}
			
			BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_ENABLE err\n");
			
		} break; // GSR_PTZCONFIG_BASE+1
		case GSR_PTZCONFIG_SERIAL_ADDRESS     : 
		{
			//return 0;
			{
				SBizPtzPara sCfgIns;
				
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetPara(&sParaTgtIns, &sCfgIns);				
				if(0==rtn)
				{ 
					#if 1 // debug temp, now no address list in configmodule
					return sCfgIns.nCamAddr;
					#else
					if( 0==BizConfigGetParaListIndex(
								EM_BIZ_CFG_PARALIST_BAUDRATE,
								sCfgIns.nCamAddr,
								&nIdx
						) 
					)
					{
						BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_ADDRESS!\n");
					}
					else
					{
						BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_ADDRESS failed!\n");
					}
					return nIdx;
					#endif
				}
			}
			
			BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_ADDRESS err\n");
			
		} break; // GSR_PTZCONFIG_BASE+2
		case GSR_PTZCONFIG_SERIAL_BPS     :  
		{
			//return 0;
			
			SBizPtzPara sCfgIns;
			
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);		
			if(0==rtn)
			{ 
				if( 0==BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_BAUDRATE,
							sCfgIns.nBaudRate,
							&nIdx
					) 
				)
				{
					BIZ_DT_DBG("GSR_PTZCONFIG_SERIAL_BPS val %d idx %d\n", sCfgIns.nBaudRate, nIdx);
					BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_BPS!\n");
				}
				else
				{
					BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_BPS failed!\n");
				}
				
				return nIdx;
			}
			
			BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_BPS err\n");
			
		} break; // GSR_PTZCONFIG_BASE+3
		
		case GSR_PTZCONFIG_SERIAL_PROTOCOL    :
		{
			//return 0;
			
			SBizPtzPara sCfgIns;
			
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);		
			if(0==rtn)
			{ 
				if( 0==BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_PTZPROTOCOL,
							sCfgIns.nProtocol,
							&nIdx
					) 
				)
				{
					BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_PROTOCOL!\n");
				}
				else
				{
					BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_PROTOCOL failed!\n");
				}
				
				return nIdx;
			}
			
			BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_BPS err\n");
			
		} break; // GSR_PTZCONFIG_BASE+4
		case GSR_PTZCONFIG_SERIAL_ALLCHECK    : {;} break; // GSR_PTZCONFIG_BASE+5
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_BASE
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE    :   
		{
			SBizPtzPara sCfgIns;
			
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{	
				if(chn < 128)
					return sCfgIns.sAdvancedPara.nIsPresetSet[chn];
			}
			
			BIZ_DATA_DBG("GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE err\n");
			
		} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ID    :    
		{
			SBizPtzPara sCfgIns;
			
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{	
				if(chn < 128)
					return sCfgIns.sAdvancedPara.sTourPath[chn].nPathNo;
			}
			
			BIZ_DATA_DBG("GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ID err\n");
			
		} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+2
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_ID  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_NAME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+2
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_PRESET: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+3
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_RATE  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+4
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_TIME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+5
		
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_TRACK_BASE
		
		
		
		//-------------------------------advance
		//case GSR_CONFIG_ADVANCE     : {;} break; // GSR_CONFIG_BASE+0x800
		
		
		case GSR_ADVANCECONFIG_IMPORTEXPORT   : {;} break; // GSR_ADVANCECONFIG_BASE+0
		case GSR_ADVANCECONFIG_IMPORTEXPORT_REFRESH   : {;} break; // GSR_ADVANCECONFIG_IMPORTEXPORT+1
		
		
		// === search ===
		
		// === backup ===
		
		// === info ===
		
		//-------------------------------system
		
		case GSR_INFO_SYSTEM_DEVICENAME   : {;} break; // GSR_INFO_SYSTEM_BASE+1
		case GSR_INFO_SYSTEM_DEVICENUM  : {;} break; // GSR_INFO_SYSTEM_BASE+2
		case GSR_INFO_SYSTEM_HARDWAREVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+3
		case GSR_INFO_SYSTEM_SCMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+4
		case GSR_INFO_SYSTEM_KERNELVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+5
		case GSR_INFO_SYSTEM_ROMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+6
		case GSR_INFO_SYSTEM_RELEASEDATE  : {;} break; // GSR_INFO_SYSTEM_BASE+7
		
		
		case GSR_INFO_DISK_NO   : {;} break; // GSR_INFO_DISK_BASE+1
		case GSR_INFO_DISK_TYPE   : {;} break; // GSR_INFO_DISK_BASE+2
		case GSR_INFO_DISK_CAPACITY   : {;} break; // GSR_INFO_DISK_BASE+3
		case GSR_INFO_DISK_FREE   : {;} break; // GSR_INFO_DISK_BASE+4
		case GSR_INFO_DISK_STATE  : {;} break; // GSR_INFO_DISK_BASE+5
		case GSR_INFO_DISK_ATTRIBUTE  : {;} break; // GSR_INFO_DISK_BASE+6
		case GSR_INFO_DISK_SOURCE   : {;} break; // GSR_INFO_DISK_BASE+7
		case GSR_INFO_DISK_RSV1   : {;} break; // GSR_INFO_DISK_BASE+8
		case GSR_INFO_DISK_RSV2   : {;} break; // GSR_INFO_DISK_BASE+9
		case GSR_INFO_DISK_IMAGE  : {;} break; // GSR_INFO_DISK_BASE+10
		case GSR_INFO_DISK_TOTAL  : {;} break; // GSR_INFO_DISK_BASE+11
		case GSR_INFO_DISK_USED   : {;} break; // GSR_INFO_DISK_BASE+12
		
		
		//-------------------------------event
		
		
		//-------------------------------syslog
		
		
		//------------------------------- netinfo
		
		
		//------------------------------- online
		
		case GSR_INFO_ONLINE_USERNAME : {;} break; // GSR_INFO_ONLINE_BASE+1
		case GSR_INFO_ONLINE_IP     : {;} break; // GSR_INFO_ONLINE_BASE+2
		case GSR_INFO_ONLINE_STATE  : {;} break; // GSR_INFO_ONLINE_BASE+3
		
		// exit
		
		
		
		// === alarm ===
		
		
		case GSR_MANUALALARM_ALLCHECK : {;} break; // GSR_MANUALALARM_BASE+1
		case GSR_MANUALALARM_ALARMNAME: {;} break; // GSR_MANUALALARM_BASE+2
		case GSR_MANUALALARM_IP     : {;} break; // GSR_MANUALALARM_BASE+3
		case GSR_MANUALALARM_ALARMPROCESS : {;} break; // GSR_MANUALALARM_BASE+4
		
		// === disk ===
		
		
		
		// === resource ===
		
		
		
		// === logoff ===
		
		
		
		// === shutdown ===
		
		
		// === desktop ===
		
		case GSR_DESKTOP_SYSTIME  : {;} break; // GSR_DESKTOP_BASE+1
		case GSR_DESKTOP_CHNNAME  : {;} break; // GSR_DESKTOP_BASE+2
		
		//case GSR_DESKTOP_VIDEO_ATTRIBUTE  : {;} break; // GSR_DESKTOP_BASE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+0
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS   :
		{
			SBizPreviewImagePara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_IMAGEPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS ok\n");
				return sCfgIns.nBrightness;
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS err\n");
			}
		} 
		break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+1
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA   :
		{
			SBizPreviewImagePara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_IMAGEPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nHue;
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA err\n");
			}
		} 
		break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+2
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION   :
		{
			SBizPreviewImagePara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_IMAGEPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nSaturation;
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION err\n");
			}
		} 
		break;  // GSR_DESKTOP_VIDEO_ATTRIBUTE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CONTRAST   :
		{
			SBizPreviewImagePara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_IMAGEPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nContrast;
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION err\n");
			}
		} 
		break;   // GSR_DESKTOP_VIDEO_ATTRIBUTE+4
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_DEFAULT  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+5			
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS   :
		{
			SBizPreviewImagePara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_VO_IMAGEPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS ok\n");
				return sCfgIns.nBrightness;
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS err\n");
			}
		} 
		break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+6
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_CHROMA   :
		{
			SBizPreviewImagePara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_VO_IMAGEPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nHue;
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_CHROMA err\n");
			}
		} 
		break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+7
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_SATURATION   :
		{
			SBizPreviewImagePara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_VO_IMAGEPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nSaturation;
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_SATURATION err\n");
			}
		} 
		break;  // GSR_DESKTOP_VIDEO_ATTRIBUTE+8
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_CONTRAST   :
		{
			SBizPreviewImagePara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_VO_IMAGEPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return sCfgIns.nContrast;
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_CONTRAST err\n");
			}
		} 
		break;   // GSR_DESKTOP_VIDEO_ATTRIBUTE+9
		case GSR_CONFIG_NETWORK_ADVANCE_DDNSUPINTVL  : 
		{
			SBizNetPara sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_NETPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0 == rtn)
			{
				if( 0==BizConfigGetParaListIndex(
							EM_BIZ_CFG_PARALIST_DDNSIPUPDATEITVL,
							sCfgIns.UpdateIntvl,
							&nIdx
					)
				)
				{
					BIZ_DATA_DBG("EM_BIZ_CFG_PARALIST_DDNSIPUPDATEITVL!\n");
				}
				else
				{
					BIZ_DATA_DBG("EM_BIZ_CFG_PARALIST_DDNSIPUPDATEITVL failed!\n");
				}
				
				return nIdx;
			}	
		} break;
	}

	return 0;
}

int GetScreenStrSelCOMBLIST(u32 nItemID, SCombList* pList)
{
	int i, rtn = 0;
	SBizParaTarget sParaTgtIns;
	s8*	pTmpList[CBX_MAX];
	u8 nChMax = GetVideoMainNum();

	BIZ_DT_DBG("GetScreenStrSelCOMBLIST ID %d\n", nItemID);
	
	if(NULL==pList)
	{
		BIZ_DATA_DBG("GetScreenStrSelCOMBLIST null list!!!\n");
	}
	
	if(NULL==pList->psValueList)
	{
		BIZ_DATA_DBG("GetScreenStrSelCOMBLIST null psValueList!!!\n");
	}
	
	if(pList->max>=CBX_MAX)
	{
		pList->max = CBX_MAX;
	}
	for(i=0; i<CBX_MAX; i++)
	{
		pTmpList[i] = pList->psValueList[i];
	}
		
	switch(nItemID)
	{		
		//-------------------------------record
		case GSR_CONFIG_RECORD_SNAP_CHN_CHOICE:
		{
			// 
			for(i=0; i<nChMax; i++)
			{
				sprintf( pList->psValueList[i], "%d", i+1 );
			}
			sprintf( pList->psValueList[i], "&CfgPtn.All");
			
			pList->nRealNum = nChMax+1;
			printf("%s: GSR_CONFIG_RECORD_SNAP_CHN_CHOICE\n", __FUNCTION__);
		} break;
		//Resolution=分辨率
		case GSR_CONFIG_RECORD_SNAP_RES:
		{
			if( 0==BizConfigGetParaStr(
						EM_BIZ_CFG_PARALIST_RECORD_SNAP_RES, 
						pTmpList, 
						&pList->nRealNum, 
						pList->max, 
						EDIT_MAX 
				) 
			)
			{
				//printf("%s: GSR_CONFIG_RECORD_SNAP_RES success\n", __FUNCTION__);
			}
			else
			{
				printf("%s: GSR_CONFIG_RECORD_SNAP_RES failed\n", __FUNCTION__);
			}
		} break;
		//Quality=图片质量
		case GSR_CONFIG_RECORD_SNAP_QUALITY:
		{
			if( 0==BizConfigGetParaStr(
						EM_BIZ_CFG_PARALIST_RECORD_SNAP_QUALITY, 
						pTmpList, 
						&pList->nRealNum, 
						pList->max, 
						EDIT_MAX 
				) 
			)
			{
				//printf("%s: GSR_CONFIG_RECORD_SNAP_QUALITY success\n", __FUNCTION__);
			}
			else
			{
				printf("%s: GSR_CONFIG_RECORD_SNAP_QUALITY failed\n", __FUNCTION__);
			}
		} break;
		//Interval=抓图时间间隔
		case GSR_CONFIG_RECORD_SNAP_INTERVAL:
		{
			if( 0==BizConfigGetParaStr(
						EM_BIZ_CFG_PARALIST_RECORD_SNAP_INTERVAL, 
						pTmpList, 
						&pList->nRealNum, 
						pList->max, 
						EDIT_MAX 
				) 
			)
			{
				//printf("%s: GSR_CONFIG_RECORD_SNAP_INTERVAL success\n", __FUNCTION__);
			}
			else
			{
				printf("%s: GSR_CONFIG_RECORD_SNAP_INTERVAL failed\n", __FUNCTION__);
			}
		} break;
		//-------------------------------alarm
		
		//case GSR_CONFIG_ALARM_SENSOR    : {;} break; // GSR_CONFIG_ALARM_BASE+0
		
		
		case GSR_CONFIG_ALARM_SENSOR_BASE_ALLCHN    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+0
		case GSR_CONFIG_ALARM_SENSOR_BASE_ENABLE    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+1
		case GSR_CONFIG_ALARM_SENSOR_BASE_TYPE    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+2
		case GSR_CONFIG_ALARM_SENSOR_BASE_NAME    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+3
		
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x20
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALLCHN     : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME    : 
		{
			// 
			if( 0==BizConfigGetParaStr(
						EM_BIZ_CFG_PARALIST_DELAY, 
						pTmpList, 
						&pList->nRealNum, 
						pList->max, 
						EDIT_MAX 
				) 
			)
			{
				//printf("get id %d real num %d\n", nItemID, pList->nRealNum);
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME!\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE: {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO  : 
		{
			// 
			for(i=0; i<nChMax; i++)
			{
				sprintf( pList->psValueList[i], "%d", i+1 );
			}
			pList->nRealNum = nChMax;

			if(1)
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO!\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE : 
		{
			// 
			if( 0==BizConfigGetParaStr(
						EM_BIZ_CFG_PARALIST_PTZLINKTYPE, 
						pTmpList, 
						&pList->nRealNum, 
						pList->max, 
						EDIT_MAX 
				) 
			)
			{
				//printf("get id %d real num %d\n", nItemID, pList->nRealNum);
				//pList->nRealNum = 2;
				BIZ_DATA_DBG("getEM_BIZ_CFG_PARALIST_PTZLINKTYPEok!\n");
			}
			else
			{
				//pList->nRealNum = 0;
				BIZ_DATA_DBG("getEM_BIZ_CFG_PARALIST_PTZLINKTYPE failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX  :
		{
			
			// 
			if(pList->max>LINKAGE_INDEX)
			{
				pList->nRealNum = LINKAGE_INDEX;
			}
			else
			{
				pList->nRealNum = pList->max;
			}
			for(i=0; i<LINKAGE_INDEX; i++)
			{
				sprintf( pList->psValueList[i], "%d", i+1 );
			}

			if(1)
			{
				//printf("get id %d real num %d\n", nItemID, pList->nRealNum);
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX!\n");
			}
			else
			{
				BIZ_DATA_DBG("getEM_BIZ_CFG_PARALIST_PTZLINKTYPE failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		
		//yaogang modify 20141010
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE: {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME+1
		case GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO  : 
		{
			// 
			for(i=0; i<nChMax; i++)
			{
				sprintf( pList->psValueList[i], "%d", i+1 );
			}
			pList->nRealNum = nChMax;

			if(1)
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO!\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_TRIGGER  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		case GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE : 
		{
			// 
			if( 0==BizConfigGetParaStr(
						EM_BIZ_CFG_PARALIST_PTZLINKTYPE, 
						pTmpList, 
						&pList->nRealNum, 
						pList->max, 
						EDIT_MAX 
				) 
			)
			{
				//printf("get id %d real num %d\n", nItemID, pList->nRealNum);
				//pList->nRealNum = 2;
				BIZ_DATA_DBG("getEM_BIZ_CFG_PARALIST_PTZLINKTYPEok!\n");
			}
			else
			{
				//pList->nRealNum = 0;
				BIZ_DATA_DBG("getEM_BIZ_CFG_PARALIST_PTZLINKTYPE failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+0
		case GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX  :
		{
			
			// 
			if(pList->max>LINKAGE_INDEX)
			{
				pList->nRealNum = LINKAGE_INDEX;
			}
			else
			{
				pList->nRealNum = pList->max;
			}
			for(i=0; i<LINKAGE_INDEX; i++)
			{
				sprintf( pList->psValueList[i], "%d", i+1 );
			}

			if(1)
			{
				//printf("get id %d real num %d\n", nItemID, pList->nRealNum);
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX!\n");
			}
			else
			{
				BIZ_DATA_DBG("getEM_BIZ_CFG_PARALIST_PTZLINKTYPE failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		
		//yaogang modify 20141010 end
		
		//case GSR_CONFIG_ALARM_SENSOR_SCH    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x40
		case GSR_CONFIG_ALARM_SENSOR_SCH_CHN  : 
		{
			nChMax = GetMaxSensorChnNum();
			for(i=0; i<nChMax; i++)
			{
				sprintf( pList->psValueList[i], "%d", i+1 );
			}
			pList->nRealNum = nChMax;

			if(1)
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_SCH_CHN!\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_SCH_CHN failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_SCH+0
		case GSR_CONFIG_ALARM_SENSOR_SCH_SCH  : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+1
		case GSR_CONFIG_ALARM_IPCEXTSENSOR_SCH_SCH : {;} break; 
		case GSR_CONFIG_ALARM_SENSOR_SCH_COPYTO   :
		{
			nChMax = GetMaxSensorChnNum();
			for(i=0; i<nChMax; i++)
			{
				sprintf( pList->psValueList[i], "%d", i+1 );
			}
			sprintf( pList->psValueList[i], "&CfgPtn.All");
			
			pList->nRealNum = nChMax+1;

			if(1)
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_SCH_COPYTO!\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_SCH_COPYTO failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_SCH+2
		
		///
		
		//case GSR_CONFIG_ALARM_MD            : {;} break; // GSR_CONFIG_ALARM_BASE+0x40
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT     : {;} break; // GSR_CONFIG_ALARM_MD+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME :
		{
			// 
			if( 0==BizConfigGetParaStr(
						EM_BIZ_CFG_PARALIST_DELAY, 
						pTmpList, 
						&pList->nRealNum, 
						pList->max, 
						EDIT_MAX 
				) 
			)
			{
				//printf("get id %d real num %d\n", nItemID, pList->nRealNum);
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME!\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALLCHN  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+2
		
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+3
		
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO : 
		{
			// 
			for(i=0; i<nChMax; i++)
			{
				sprintf( pList->psValueList[i], "%d", i+1 );
			}
			pList->nRealNum = nChMax;

			if(1)
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO!\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_TYPE  :
		{
			// 
			if( 0==BizConfigGetParaStr(
						EM_BIZ_CFG_PARALIST_PTZLINKTYPE, 
						pTmpList, 
						&pList->nRealNum, 
						pList->max, 
						EDIT_MAX 
				) 
			)
			{
				//printf("!!!!!!!!!!!!!!!!get id %d real num %d\n", nItemID, pList->nRealNum);
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_TYPE!\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_TYPE failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_INDEX :
		{
			// 
			if(pList->max>LINKAGE_INDEX)
			{
				pList->nRealNum = LINKAGE_INDEX;
			}
			else
			{
				pList->nRealNum = pList->max;
			}
			for(i=0; i<LINKAGE_INDEX; i++)
			{
				sprintf( pList->psValueList[i], "%d", i+1 );
			}

			if(1)
			{
				//printf("get id %d real num %d\n", nItemID, pList->nRealNum);
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX!\n");
			}
			else
			{
				BIZ_DATA_DBG("getEM_BIZ_CFG_PARALIST_PTZLINKTYPE failed!\n");
			}
		}  break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+1
		case GSR_CONFIG_ALARM_MD_SCH_SCH_CHN  : 
		case GSR_CONFIG_ALARM_IPCEXTSENSOR_SCH_CHN:
		{
			// 
			for(i=0; i<nChMax; i++)
			{
				sprintf( pList->psValueList[i], "%d", i+1 );
			}
			
			pList->nRealNum = nChMax;

			if(1)
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_SCH_SCH_CHN!\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_SCH_SCH_CHN failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_SCH+0
		case GSR_CONFIG_ALARM_MD_SCH_SCH_SCH  : {;} break; // GSR_CONFIG_ALARM_MD_SCH+1
		case GSR_CONFIG_ALARM_MD_SCH_SCH_COPYTO   :
		case GSR_CONFIG_ALARM_IPCEXTSENSOR_SCH_COPYTO:
		{
			// 
			for(i=0; i<nChMax; i++)
			{
				sprintf( pList->psValueList[i], "%d", i+1 );
			}
			sprintf( pList->psValueList[i], "&CfgPtn.All");
			
			pList->nRealNum = nChMax+1;

			if(1)
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_SCH_SCH_COPYTO!\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_SCH_SCH_COPYTO failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_SCH+2
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO : 
		{
			// 
			for(i=0; i<nChMax; i++)
			{
				sprintf( pList->psValueList[i], "%d", i+1 );
			}
			pList->nRealNum = nChMax;

			if(1)
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO!\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO failed!\n");
			}
		}break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_TYPE  :
		{
			// 
			if( 0==BizConfigGetParaStr(
						EM_BIZ_CFG_PARALIST_PTZLINKTYPE, 
						pTmpList, 
						&pList->nRealNum, 
						pList->max, 
						EDIT_MAX 
				) 
			)
			{
				//printf("get id %d real num %d\n", nItemID, pList->nRealNum);
				BIZ_DATA_DBG("getEM_BIZ_CFG_PARALIST_PTZLINKTYPEok!\n");
			}
			else
			{
				BIZ_DATA_DBG("getEM_BIZ_CFG_PARALIST_PTZLINKTYPE failed!\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_INDEX :
		{
			// 
			if(pList->max>LINKAGE_INDEX)
			{
				pList->nRealNum = LINKAGE_INDEX;
			}
			else
			{
				pList->nRealNum = pList->max;
			}
			for(i=0; i<LINKAGE_INDEX; i++)
			{
				sprintf( pList->psValueList[i], "%d", i+1 );
			}

			if(1)
			{
				//printf("get id %d real num %d\n", nItemID, pList->nRealNum);
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX!\n");
			}
			else
			{
				BIZ_DATA_DBG("getEM_BIZ_CFG_PARALIST_PTZLINKTYPE failed!\n");
			}
		}  break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+1
		
		
		//case GSR_CONFIG_ALARM_OTHER           : {;} break; // GSR_CONFIG_ALARM_BASE+0xA0
		
		case GSR_CONFIG_ALARM_OTHER_ALARMTYPE     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+1
		case GSR_CONFIG_ALARM_OTHER_SOUNDALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+2
		case GSR_CONFIG_ALARM_OTHER_MAIL        : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+3
		case GSR_CONFIG_ALARM_OTHER_TRIGGERALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+4
		case GSR_CONFIG_ALARM_OTHER_DISKALARM     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+5
		case GSR_CONFIG_ALARM_OTHER_ALLCHN      : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+6
		
		
		case GSR_CONFIG_ALARM_ALARMOUT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xB0
		
		
		case GSR_CONFIG_ALARM_EXIT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xC0
		
		
		//-------------------------------network
		
		case GSR_CONFIG_NETWORK_BASE    : {;} break; // GSR_CONFIG_BASE+0x500
		
		
		//-------------------------------user
		
		case GSR_CONFIG_USER_BASE       : {;} break; // GSR_CONFIG_BASE+0x600
		
		
		//-------------------------------ptz
		
		
		
		case GSR_PTZCONFIG_SERIAL_ENABLE    : {;} break; // GSR_PTZCONFIG_BASE+1
		case GSR_PTZCONFIG_SERIAL_ADDRESS     : 
		{
			//pList->nRealNum = 2;
			//strcpy(pTmpList[0],"Cam01");
			//strcpy(pTmpList[1],"Cam02");
			
			char s[10]={0};
			pList->nRealNum = 32;
            for (int i = 0; i < 32; i++)
            {
                sprintf(pTmpList[i], "%d", i);
            }

			return 0;
		} break; // GSR_PTZCONFIG_BASE+2
		case GSR_PTZCONFIG_SERIAL_BPS     :
		{			
			// 
			BIZ_DATA_DBG("getEM_BIZ_CFG_PARALIST_PTZLINKTYPE enter!\n");
			if( 0==BizConfigGetParaStr(
						EM_BIZ_CFG_PARALIST_BAUDRATE, 
						pTmpList, 
						&pList->nRealNum, 
						pList->max, 
						EDIT_MAX 
				) 
			)
			{
				//printf("get id %d real num %d\n", nItemID, pList->nRealNum);
				
				BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_BPS!\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_BPS failed!\n");
			}
		} break; // GSR_PTZCONFIG_BASE+3
		case GSR_PTZCONFIG_SERIAL_PROTOCOL    : 
		{
			// 
			#if 1
			if( 0==BizConfigGetParaStr(
						EM_BIZ_CFG_PARALIST_PTZPROTOCOL,
						pTmpList, 
						&pList->nRealNum, 
						pList->max, 
						EDIT_MAX 
				) 
			)
			{
				BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_PROTOCOL ok!\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_PROTOCOL failed!\n");
			}
			#else
			pList->nRealNum = 2;
			pTmpList[0] = "PELCOD";
			pTmpList[1] = "PELCOC";
			#endif
		} break; // GSR_PTZCONFIG_BASE+4
		case GSR_PTZCONFIG_SERIAL_ALLCHECK    : {;} break; // GSR_PTZCONFIG_BASE+5
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_BASE
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ID    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+2
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_ID  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_NAME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+2
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_PRESET: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+3
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_RATE  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+4
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_TIME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+5
		
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_TRACK_BASE
		
		
		
		//-------------------------------advance
		//case GSR_CONFIG_ADVANCE     : {;} break; // GSR_CONFIG_BASE+0x800
		
		
		case GSR_ADVANCECONFIG_IMPORTEXPORT   : {;} break; // GSR_ADVANCECONFIG_BASE+0
		case GSR_ADVANCECONFIG_IMPORTEXPORT_REFRESH   : {;} break; // GSR_ADVANCECONFIG_IMPORTEXPORT+1
		
		
		// === search ===
		
		// === backup ===
		
		// === info ===
		
		//-------------------------------system
		
		case GSR_INFO_SYSTEM_DEVICENAME   : {;} break; // GSR_INFO_SYSTEM_BASE+1
		case GSR_INFO_SYSTEM_DEVICENUM  : {;} break; // GSR_INFO_SYSTEM_BASE+2
		case GSR_INFO_SYSTEM_HARDWAREVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+3
		case GSR_INFO_SYSTEM_SCMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+4
		case GSR_INFO_SYSTEM_KERNELVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+5
		case GSR_INFO_SYSTEM_ROMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+6
		case GSR_INFO_SYSTEM_RELEASEDATE  : {;} break; // GSR_INFO_SYSTEM_BASE+7
		
		
		case GSR_INFO_DISK_NO   : {;} break; // GSR_INFO_DISK_BASE+1
		case GSR_INFO_DISK_TYPE   : {;} break; // GSR_INFO_DISK_BASE+2
		case GSR_INFO_DISK_CAPACITY   : {;} break; // GSR_INFO_DISK_BASE+3
		case GSR_INFO_DISK_FREE   : {;} break; // GSR_INFO_DISK_BASE+4
		case GSR_INFO_DISK_STATE  : {;} break; // GSR_INFO_DISK_BASE+5
		case GSR_INFO_DISK_ATTRIBUTE  : {;} break; // GSR_INFO_DISK_BASE+6
		case GSR_INFO_DISK_SOURCE   : {;} break; // GSR_INFO_DISK_BASE+7
		case GSR_INFO_DISK_RSV1   : {;} break; // GSR_INFO_DISK_BASE+8
		case GSR_INFO_DISK_RSV2   : {;} break; // GSR_INFO_DISK_BASE+9
		case GSR_INFO_DISK_IMAGE  : {;} break; // GSR_INFO_DISK_BASE+10
		case GSR_INFO_DISK_TOTAL  : {;} break; // GSR_INFO_DISK_BASE+11
		case GSR_INFO_DISK_USED   : {;} break; // GSR_INFO_DISK_BASE+12
		
		
		//-------------------------------event
		
		
		//-------------------------------syslog
		
		
		//------------------------------- netinfo
		
		
		//------------------------------- online
		
		case GSR_INFO_ONLINE_USERNAME : {;} break; // GSR_INFO_ONLINE_BASE+1
		case GSR_INFO_ONLINE_IP     : {;} break; // GSR_INFO_ONLINE_BASE+2
		case GSR_INFO_ONLINE_STATE  : {;} break; // GSR_INFO_ONLINE_BASE+3
		
		// exit
		
		
		
		// === alarm ===
		
		
		case GSR_MANUALALARM_ALLCHECK : {;} break; // GSR_MANUALALARM_BASE+1
		case GSR_MANUALALARM_ALARMNAME: {;} break; // GSR_MANUALALARM_BASE+2
		case GSR_MANUALALARM_IP     : {;} break; // GSR_MANUALALARM_BASE+3
		case GSR_MANUALALARM_ALARMPROCESS : {;} break; // GSR_MANUALALARM_BASE+4
		
		// === disk ===
		
		
		
		// === resource ===
		
		
		
		// === logoff ===
		
		
		
		// === shutdown ===
		
		
		// === desktop ===
		
		case GSR_DESKTOP_SYSTIME  : {;} break; // GSR_DESKTOP_BASE+1
		case GSR_DESKTOP_CHNNAME  : {;} break; // GSR_DESKTOP_BASE+2
		
		//case GSR_DESKTOP_VIDEO_ATTRIBUTE  : {;} break; // GSR_DESKTOP_BASE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN  : 
		{
			// 
			for(i=0; i<nChMax; i++)
			{
				sprintf( pList->psValueList[i], "%d", i+1 );
			}
			sprintf( pList->psValueList[i], "&CfgPtn.All");//cw_test
			
			pList->nRealNum = nChMax+1;

			if(1)
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN!\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN failed!\n");
			}
		}
		
		break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+0
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+1
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+2
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CONTRAST   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+4
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_DEFAULT  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+5
		case GSR_CONFIG_NETWORK_ADVANCE_DDNSDOMAIN: 
		{
			if( 0==BizConfigGetParaStr(
						EM_BIZ_CFG_PARALIST_DDNSDOMAIN, 
						pTmpList, 
						&pList->nRealNum, 
						pList->max, 
						EDIT_MAX 
				) 
			)
			{
				BIZ_DATA_DBG("EM_BIZ_CFG_PARALIST_DDNSDOMAIN ok!\n");
			}
			else
			{
				BIZ_DATA_DBG("EM_BIZ_CFG_PARALIST_DDNSDOMAIN failed!\n");
			}			
		} break;
		case GSR_CONFIG_NETWORK_ADVANCE_DDNSUPINTVL: 
		{
			if( 0==BizConfigGetParaStr(
						EM_BIZ_CFG_PARALIST_DDNSIPUPDATEITVL,
						pTmpList, 
						&pList->nRealNum, 
						pList->max, 
						EDIT_MAX 
				) 
			)
			{
				BIZ_DATA_DBG("EM_BIZ_CFG_PARALIST_DDNSIPUPDATEITVL ok!\n");
			}
			else
			{
				BIZ_DATA_DBG("EM_BIZ_CFG_PARALIST_DDNSIPUPDATEITVL failed!\n");
			}			
		} break;
	}
	
	return 0;
}

int GetScreenStrSelSCH(u32 nItemID, u8 chn, uint* sch)
{
	int i, j;
	int rtn = 0;
	SBizParaTarget sParaTgtIns;
	
	// clear all of the weekday
	if(sch)
	{
		memset(sch, 0, 7*sizeof(uint));
	}
	
	if(nItemID>=GSR_DEFAULT_OFFSET)
	{
		switch(nItemID)
		{	
			case GSR_CONFIG_BASIC_SYSTEM_DEVICENAME       + GSR_DEFAULT_OFFSET :
			{;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+0
			case GSR_CONFIG_BASIC_SYSTEM_DEVICEID       : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+1
			case GSR_CONFIG_BASIC_SYSTEM_VIDEOFORMAT      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+2
			case GSR_CONFIG_BASIC_SYSTEM_AUTHCHECK      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+3
			case GSR_CONFIG_BASIC_SYSTEM_TIMEDISPLAY      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+4
			case GSR_CONFIG_BASIC_SYSTEM_MAXNETUSERS      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+5
			case GSR_CONFIG_BASIC_SYSTEM_OUTPUT         : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+6
			case GSR_CONFIG_BASIC_SYSTEM_LANGUAGE       : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+7
			case GSR_CONFIG_BASIC_SYSTEM_SCRENNSAVER      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+8
			
			
			//date&time..
			
			case GSR_CONFIG_BASIC_DATETIME_DATEFORMAT         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+0
			case GSR_CONFIG_BASIC_DATETIME_TIMEFORMAT         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+1
			case GSR_CONFIG_BASIC_DATETIME_TIMEZONE         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+2
			case GSR_CONFIG_BASIC_DATETIME_NETSYN         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+3
			case GSR_CONFIG_BASIC_DATETIME_NTPSERVER          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+4
			case GSR_CONFIG_BASIC_DATETIME_SYSDATE          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+5
			case GSR_CONFIG_BASIC_DATETIME_SYSTIME          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+6
			case GSR_CONFIG_BASIC_DATETIME_SYNCTIMETOIPC    : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+7
			
			//-------------------------------live
			
			//case GSR_CONFIG_LIVE_LIVE             : {;} break; // GSR_CONFIG_LIVE_BASE+0
			case GSR_CONFIG_LIVE_ALLCHN             : {;} break; // GSR_CONFIG_LIVE_BASE+1
			case GSR_CONFIG_LIVE_CHNNAME            : {;} break; // GSR_CONFIG_LIVE_BASE+2
			case GSR_CONFIG_LIVE_DISPLAY            : {;} break; // GSR_CONFIG_LIVE_BASE+3
			case GSR_CONFIG_LIVE_LIVEREC            : {;} break; // GSR_CONFIG_LIVE_BASE+4
			case GSR_CONFIG_LIVE_COLORSET             : {;} break; // GSR_CONFIG_LIVE_BASE+5
			
			//case GSR_CONFIG_LIVE_MAINMONITOR            : {;} break; // GSR_CONFIG_LIVE_BASE+0x40
			case GSR_CONFIG_LIVE_MAINMONITOR_SPLITMODE      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+0
			case GSR_CONFIG_LIVE_MAINMONITOR_CHN      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+1
			case GSR_CONFIG_LIVE_MAINMONITOR_DWELLTIME      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+2
			
			//case GSR_CONFIG_LIVE_SPOT             : {;} break; // GSR_CONFIG_LIVE_BASE+0x80
			case GSR_CONFIG_LIVE_SPOT_SPLITMODE             : {;} break; // GSR_CONFIG_LIVE_SPOT+0
			case GSR_CONFIG_LIVE_SPOT_CHN             : {;} break; // GSR_CONFIG_LIVE_SPOT+1
			case GSR_CONFIG_LIVE_SPOT_DWELLTIME             : {;} break; // GSR_CONFIG_LIVE_SPOT+2
			
			//case GSR_CONFIG_LIVE_MASK             : {;} break; // GSR_CONFIG_LIVE_BASE+0xc0
			case GSR_CONFIG_LIVE_MASK_AREA            : {;} break; // GSR_CONFIG_LIVE_MASK+0
			
			
			
			//-------------------------------record
			
			
			case GSR_CONFIG_RECORD_ENABLE_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_BASE+0
			case GSR_CONFIG_RECORD_ENABLE_RECORD        : {;} break; // GSR_CONFIG_RECORD_BASE+1
			case GSR_CONFIG_RECORD_ENABLE_AUDIO         : {;} break; // GSR_CONFIG_RECORD_BASE+2
			
			
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ALLCHN         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+0
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_RESOLUTION         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+1
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_FRAMERATE        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+2
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ENCODE         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+3
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_QUALITY        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+4
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_MAXRATE      : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+5
			
			
			case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_ALLCHN       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+0
			case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEBEFALARM   : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+1
			case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEAFTALARM       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+2
			case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_EXPIRATION       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+3
			
			
			case GSR_CONFIG_RECORD_ENABLE_OSD_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+0
			case GSR_CONFIG_RECORD_ENABLE_OSD_CHNNAME         : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+1
			case GSR_CONFIG_RECORD_ENABLE_OSD_TIMESTAMP         : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+2
			case GSR_CONFIG_RECORD_ENABLE_OSD_POSITION        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+3
			
			
			case GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_REC        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_BASE+0
			
			
			
			
			
			//-------------------------------schedule
			
			//case GSR_CONFIG_SCHEDULE_TIMEREC  : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0
			case GSR_CONFIG_SCHEDULE_TIMEREC_CHN  : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+0
			case GSR_CONFIG_SCHEDULE_TIMEREC_SCH  + GSR_DEFAULT_OFFSET :
			{
				SBizSchedule sCfgIns;
				SBizParaTarget sParaTgtIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				// clear all of the weekday
				if(sch)
				{
					memset(sch, 0, 7*sizeof(uint));
				}
				
				sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECTIMING;
				sParaTgtIns.nChn = chn;
				int rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					bizData_GetSch(&sCfgIns, sch);
					
					return 0;
				}
	
			} break; // GSR_CONFIG_SCHEDULE_TIMEREC+1
			case GSR_CONFIG_SCHEDULE_TIMEREC_COPYTO   : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+2
			
			
			//case GSR_CONFIG_SCHEDULE_MOTIONALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x20
			case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+0
			case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH   + GSR_DEFAULT_OFFSET :
			{
				SBizSchedule sCfgIns;
				SBizParaTarget sParaTgtIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				// clear all of the weekday
				if(sch)
				{
					memset(sch, 0, 7*sizeof(uint));
				}
				
				
				sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECMOTION;
				sParaTgtIns.nChn = chn;
				int rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					bizData_GetSch(&sCfgIns, sch);
					
					return 0;
				}
	
			} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+1
			case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+2
			
			//case GSR_CONFIG_SCHEDULE_SENSORALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x30
			case GSR_CONFIG_SCHEDULE_SENSORALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+0
			case GSR_CONFIG_SCHEDULE_SENSORALARMREC_SCH   + GSR_DEFAULT_OFFSET :
			{
				SBizSchedule sCfgIns;
				SBizParaTarget sParaTgtIns;
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				// clear all of the weekday
				if(sch)
				{
					memset(sch, 0, 7*sizeof(uint));
				}
				
				
				sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECSENSOR;
				sParaTgtIns.nChn = chn;
				int rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					bizData_GetSch(&sCfgIns, sch);
					return 0;
				}
			} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+1
			case GSR_CONFIG_SCHEDULE_SENSORALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+2
			
			
			//-------------------------------alarm
			
			//case GSR_CONFIG_ALARM_SENSOR    : {;} break; // GSR_CONFIG_ALARM_BASE+0
			
			case GSR_CONFIG_ALARM_SENSOR_BASE_ALLCHN    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+0
			case GSR_CONFIG_ALARM_SENSOR_BASE_ENABLE    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+1
			case GSR_CONFIG_ALARM_SENSOR_BASE_TYPE    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+2
			case GSR_CONFIG_ALARM_SENSOR_BASE_NAME    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+3
			
			
			//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x20
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALLCHN     : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+0
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME    : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+1
			
			//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE: {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME+1
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+0
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+1
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+2
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+3
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+4 // bitmap
			
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP+1
			
			//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+0
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
			
			
			
			//case GSR_CONFIG_ALARM_SENSOR_SCH    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x40
			case GSR_CONFIG_ALARM_SENSOR_SCH_CHN  : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+0
			case GSR_CONFIG_ALARM_SENSOR_SCH_SCH  + GSR_DEFAULT_OFFSET :
			{
				SBizSchedule sCfgIns;
				
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_SENSOR;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);//BizGetPara(&sParaTgtIns, &sCfgIns);BizGetDefaultPara
				if(0==rtn)
				{
					BIZ_DO_DBG("sCfgIns.time0 start %x end %x\n", 
						sCfgIns.sSchTime[0][0].nStartTime,
						sCfgIns.sSchTime[0][0].nStopTime
					);
					bizData_GetSch(&sCfgIns, sch);
					
					return 0;
				}
				
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_SCH_SCH err\n");
				
			} break; // GSR_CONFIG_ALARM_SENSOR_SCH+1
			case GSR_CONFIG_ALARM_IPCEXTSENSOR_SCH_SCH  + GSR_DEFAULT_OFFSET :
			{
				SBizSchedule sCfgIns;
				
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_IPCEXTSENSOR;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);//BizGetPara(&sParaTgtIns, &sCfgIns);BizGetDefaultPara
				if(0==rtn)
				{
					BIZ_DO_DBG("sCfgIns.time0 start %x end %x\n", 
						sCfgIns.sSchTime[0][0].nStartTime,
						sCfgIns.sSchTime[0][0].nStopTime
					);
					bizData_GetSch(&sCfgIns, sch);
					
					return 0;
				}
				
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_SCH_SCH err\n");
				
			} break; // GSR_CONFIG_ALARM_SENSOR_SCH+1
			case GSR_CONFIG_ALARM_SENSOR_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+2
			
			///
			
			//case GSR_CONFIG_ALARM_MD            : {;} break; // GSR_CONFIG_ALARM_BASE+0x40
			//case GSR_CONFIG_ALARM_MD_MOTIONDETECT     : {;} break; // GSR_CONFIG_ALARM_MD+0
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+0
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+1
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALLCHN  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+2
			
			//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+3
			
			
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+0
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+1
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+2
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+3
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+4 // bitmap
			
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x10
			
			//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x20
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_TYPE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+0
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_INDEX : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+1
			
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET  : {;} break; // GSR_CONFIG_ALARM_MD+0x28
			
			
			//case GSR_CONFIG_ALARM_MD_SCH      : {;} break; // GSR_CONFIG_ALARM_MD+0x30
			case GSR_CONFIG_ALARM_MD_SCH_SCH_CHN  : {;} break; // GSR_CONFIG_ALARM_MD_SCH+0
			case GSR_CONFIG_ALARM_MD_SCH_SCH_SCH  + GSR_DEFAULT_OFFSET :
			{
				SBizSchedule sCfgIns;
				
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_VMOTION;
				sParaTgtIns.nChn = chn;
				rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					BIZ_DO_DBG("sCfgIns.time0 start %x end %x\n", 
						sCfgIns.sSchTime[0][0].nStartTime,
						sCfgIns.sSchTime[0][0].nStopTime
					);
					bizData_GetSch(&sCfgIns, sch);
					
					return 0;
				}
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_SCH_SCH err\n");
				
			} break; // GSR_CONFIG_ALARM_MD_SCH+1
			case GSR_CONFIG_ALARM_MD_SCH_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_MD_SCH+2
			
			
			//case GSR_CONFIG_ALARM_VIDEOLOST     : {;} break; // GSR_CONFIG_ALARM_BASE+0x80
			
			//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST+0
			
			
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+0
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+1
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+2
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+3
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+4 // bitmap
			
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x10
			
			//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x20
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_TYPE  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_INDEX : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+1
			
			
			//case GSR_CONFIG_ALARM_OTHER           : {;} break; // GSR_CONFIG_ALARM_BASE+0xA0
			
			case GSR_CONFIG_ALARM_OTHER_ALARMTYPE     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+1
			case GSR_CONFIG_ALARM_OTHER_SOUNDALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+2
			case GSR_CONFIG_ALARM_OTHER_MAIL        : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+3
			case GSR_CONFIG_ALARM_OTHER_TRIGGERALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+4
			case GSR_CONFIG_ALARM_OTHER_DISKALARM     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+5
			case GSR_CONFIG_ALARM_OTHER_ALLCHN      : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+6
			
			
			case GSR_CONFIG_ALARM_ALARMOUT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xB0
			
			
			case GSR_CONFIG_ALARM_EXIT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xC0
			
			
			//-------------------------------network
			
			case GSR_CONFIG_NETWORK_BASE    : {;} break; // GSR_CONFIG_BASE+0x500
			
			
			//-------------------------------user
			
			case GSR_CONFIG_USER_BASE       : {;} break; // GSR_CONFIG_BASE+0x600
			
			
			//-------------------------------ptz
			case GSR_PTZCONFIG_SERIAL_PROTOCOL    :   
			{
				
			} break; // GSR_PTZCONFIG_BASE+4
			case GSR_PTZCONFIG_SERIAL_ALLCHECK    : {;} break; // GSR_PTZCONFIG_BASE+5
			
			//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_BASE
			
			//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE
			
			case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+1
			case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ID    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+2
			
			//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE
			
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_ID  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+1
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_NAME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+2
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_PRESET: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+3
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_RATE  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+4
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_TIME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+5
			
			
			//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_TRACK_BASE
			
			
			
			//-------------------------------advance
			//case GSR_CONFIG_ADVANCE     : {;} break; // GSR_CONFIG_BASE+0x800
			
			
			case GSR_ADVANCECONFIG_IMPORTEXPORT   : {;} break; // GSR_ADVANCECONFIG_BASE+0
			case GSR_ADVANCECONFIG_IMPORTEXPORT_REFRESH   : {;} break; // GSR_ADVANCECONFIG_IMPORTEXPORT+1
			
			
			// === search ===
			
			// === backup ===
			
			// === info ===
			
			//-------------------------------system
			
			case GSR_INFO_SYSTEM_DEVICENAME   : {;} break; // GSR_INFO_SYSTEM_BASE+1
			case GSR_INFO_SYSTEM_DEVICENUM  : {;} break; // GSR_INFO_SYSTEM_BASE+2
			case GSR_INFO_SYSTEM_HARDWAREVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+3
			case GSR_INFO_SYSTEM_SCMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+4
			case GSR_INFO_SYSTEM_KERNELVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+5
			case GSR_INFO_SYSTEM_ROMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+6
			case GSR_INFO_SYSTEM_RELEASEDATE  : {;} break; // GSR_INFO_SYSTEM_BASE+7
			
			
			case GSR_INFO_DISK_NO   : {;} break; // GSR_INFO_DISK_BASE+1
			case GSR_INFO_DISK_TYPE   : {;} break; // GSR_INFO_DISK_BASE+2
			case GSR_INFO_DISK_CAPACITY   : {;} break; // GSR_INFO_DISK_BASE+3
			case GSR_INFO_DISK_FREE   : {;} break; // GSR_INFO_DISK_BASE+4
			case GSR_INFO_DISK_STATE  : {;} break; // GSR_INFO_DISK_BASE+5
			case GSR_INFO_DISK_ATTRIBUTE  : {;} break; // GSR_INFO_DISK_BASE+6
			case GSR_INFO_DISK_SOURCE   : {;} break; // GSR_INFO_DISK_BASE+7
			case GSR_INFO_DISK_RSV1   : {;} break; // GSR_INFO_DISK_BASE+8
			case GSR_INFO_DISK_RSV2   : {;} break; // GSR_INFO_DISK_BASE+9
			case GSR_INFO_DISK_IMAGE  : {;} break; // GSR_INFO_DISK_BASE+10
			case GSR_INFO_DISK_TOTAL  : {;} break; // GSR_INFO_DISK_BASE+11
			case GSR_INFO_DISK_USED   : {;} break; // GSR_INFO_DISK_BASE+12
			
			
			//-------------------------------event
			
			
			//-------------------------------syslog
			
			
			//------------------------------- netinfo
			
			
			//------------------------------- online
			
			case GSR_INFO_ONLINE_USERNAME : {;} break; // GSR_INFO_ONLINE_BASE+1
			case GSR_INFO_ONLINE_IP     : {;} break; // GSR_INFO_ONLINE_BASE+2
			case GSR_INFO_ONLINE_STATE  : {;} break; // GSR_INFO_ONLINE_BASE+3
			
			// exit
			
			
			
			// === alarm ===
			
			
			case GSR_MANUALALARM_ALLCHECK : {;} break; // GSR_MANUALALARM_BASE+1
			case GSR_MANUALALARM_ALARMNAME: {;} break; // GSR_MANUALALARM_BASE+2
			case GSR_MANUALALARM_IP     : {;} break; // GSR_MANUALALARM_BASE+3
			case GSR_MANUALALARM_ALARMPROCESS : {;} break; // GSR_MANUALALARM_BASE+4
			
			// === disk ===
			
			
			
			// === resource ===
			
			
			
			// === logoff ===
			
			
			
			// === shutdown ===
			
			
			// === desktop ===
			
			case GSR_DESKTOP_SYSTIME  : {;} break; // GSR_DESKTOP_BASE+1
			case GSR_DESKTOP_CHNNAME  : {;} break; // GSR_DESKTOP_BASE+2
			
			//case GSR_DESKTOP_VIDEO_ATTRIBUTE  : {;} break; // GSR_DESKTOP_BASE+3
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+0
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+1
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+2
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+3
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_CONTRAST   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+4
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_DEFAULT  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+5
			
	
		}

		return 0;
	}

	switch(nItemID)
	{	
		case GSR_CONFIG_BASIC_SYSTEM_DEVICENAME       : 
			{;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+0
		case GSR_CONFIG_BASIC_SYSTEM_DEVICEID       : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+1
		case GSR_CONFIG_BASIC_SYSTEM_VIDEOFORMAT      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+2
		case GSR_CONFIG_BASIC_SYSTEM_AUTHCHECK      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+3
		case GSR_CONFIG_BASIC_SYSTEM_TIMEDISPLAY      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+4
		case GSR_CONFIG_BASIC_SYSTEM_MAXNETUSERS      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+5
		case GSR_CONFIG_BASIC_SYSTEM_OUTPUT         : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+6
		case GSR_CONFIG_BASIC_SYSTEM_LANGUAGE       : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+7
		case GSR_CONFIG_BASIC_SYSTEM_SCRENNSAVER      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+8
		
		
		//date&time..
		
		case GSR_CONFIG_BASIC_DATETIME_DATEFORMAT         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+0
		case GSR_CONFIG_BASIC_DATETIME_TIMEFORMAT         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+1
		case GSR_CONFIG_BASIC_DATETIME_TIMEZONE         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+2
		case GSR_CONFIG_BASIC_DATETIME_NETSYN         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+3
		case GSR_CONFIG_BASIC_DATETIME_NTPSERVER          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+4
		case GSR_CONFIG_BASIC_DATETIME_SYSDATE          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+5
		case GSR_CONFIG_BASIC_DATETIME_SYSTIME          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+6
		case GSR_CONFIG_BASIC_DATETIME_SYNCTIMETOIPC    : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+7
		
		
		//-------------------------------live
		
		//case GSR_CONFIG_LIVE_LIVE             : {;} break; // GSR_CONFIG_LIVE_BASE+0
		case GSR_CONFIG_LIVE_ALLCHN             : {;} break; // GSR_CONFIG_LIVE_BASE+1
		case GSR_CONFIG_LIVE_CHNNAME            : {;} break; // GSR_CONFIG_LIVE_BASE+2
		case GSR_CONFIG_LIVE_DISPLAY            : {;} break; // GSR_CONFIG_LIVE_BASE+3
		case GSR_CONFIG_LIVE_LIVEREC            : {;} break; // GSR_CONFIG_LIVE_BASE+4
		case GSR_CONFIG_LIVE_COLORSET             : {;} break; // GSR_CONFIG_LIVE_BASE+5
		
		//case GSR_CONFIG_LIVE_MAINMONITOR            : {;} break; // GSR_CONFIG_LIVE_BASE+0x40
		case GSR_CONFIG_LIVE_MAINMONITOR_SPLITMODE      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+0
		case GSR_CONFIG_LIVE_MAINMONITOR_CHN      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+1
		case GSR_CONFIG_LIVE_MAINMONITOR_DWELLTIME      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+2
		
		//case GSR_CONFIG_LIVE_SPOT             : {;} break; // GSR_CONFIG_LIVE_BASE+0x80
		case GSR_CONFIG_LIVE_SPOT_SPLITMODE             : {;} break; // GSR_CONFIG_LIVE_SPOT+0
		case GSR_CONFIG_LIVE_SPOT_CHN             : {;} break; // GSR_CONFIG_LIVE_SPOT+1
		case GSR_CONFIG_LIVE_SPOT_DWELLTIME             : {;} break; // GSR_CONFIG_LIVE_SPOT+2
		
		//case GSR_CONFIG_LIVE_MASK             : {;} break; // GSR_CONFIG_LIVE_BASE+0xc0
		case GSR_CONFIG_LIVE_MASK_AREA            : {;} break; // GSR_CONFIG_LIVE_MASK+0
		
		
		
		//-------------------------------record
		
		
		case GSR_CONFIG_RECORD_ENABLE_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_RECORD        : {;} break; // GSR_CONFIG_RECORD_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_AUDIO         : {;} break; // GSR_CONFIG_RECORD_BASE+2
		
		
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ALLCHN         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_RESOLUTION         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_FRAMERATE        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ENCODE         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+3
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_QUALITY        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+4
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_MAXRATE      : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+5
		
		
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_ALLCHN       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEBEFALARM   : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEAFTALARM       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_EXPIRATION       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+3
		
		
		case GSR_CONFIG_RECORD_ENABLE_OSD_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_OSD_CHNNAME         : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_OSD_TIMESTAMP         : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_OSD_POSITION        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+3
		
		
		case GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_REC        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_BASE+0
		
		
		
		
		
		//-------------------------------schedule
		
		//case GSR_CONFIG_SCHEDULE_TIMEREC  : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0
		case GSR_CONFIG_SCHEDULE_TIMEREC_CHN  : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+0
		case GSR_CONFIG_SCHEDULE_TIMEREC_SCH  : 
		{

			//printf("GSR_CONFIG_SCHEDULE_TIMEREC_SCH 2\n");
			SBizSchedule sCfgIns;
			SBizParaTarget sParaTgtIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			// clear all of the weekday
			if(sch)
			{
				memset(sch, 0, 7*sizeof(uint));
				//printf("GSR_CONFIG_SCHEDULE_TIMEREC_SCH 1\n");
			}
			
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECTIMING;
			sParaTgtIns.nChn = chn;
			int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				//printf("GSR_CONFIG_SCHEDULE_TIMEREC_SCH 31\n");
				bizData_GetSch(&sCfgIns, sch);
				
				return 0;
			}
			printf("GSR_CONFIG_SCHEDULE_TIMEREC_SCH 41\n");

		} break; // GSR_CONFIG_SCHEDULE_TIMEREC+1
		case GSR_CONFIG_SCHEDULE_TIMEREC_COPYTO   : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+2
		
		
		//case GSR_CONFIG_SCHEDULE_MOTIONALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x20
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+0
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH   : 
		{
			SBizSchedule sCfgIns;
			SBizParaTarget sParaTgtIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			// clear all of the weekday
			if(sch)
			{
				memset(sch, 0, 7*sizeof(uint));
			}
			
			
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECMOTION;
			sParaTgtIns.nChn = chn;
			int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				bizData_GetSch(&sCfgIns, sch);
				
				return 0;
			}

		} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+1
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+2
		
		//case GSR_CONFIG_SCHEDULE_SENSORALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x30
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+0
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_SCH   : 
		{
			SBizSchedule sCfgIns;
			SBizParaTarget sParaTgtIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			// clear all of the weekday
			if(sch)
			{
				memset(sch, 0, 7*sizeof(uint));
			}
			
			
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECSENSOR;
			sParaTgtIns.nChn = chn;
			int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				bizData_GetSch(&sCfgIns, sch);
				return 0;
			}
		} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+1
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+2
		
		
		//-------------------------------alarm
		
		//case GSR_CONFIG_ALARM_SENSOR    : {;} break; // GSR_CONFIG_ALARM_BASE+0
		
		case GSR_CONFIG_ALARM_SENSOR_BASE_ALLCHN    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+0
		case GSR_CONFIG_ALARM_SENSOR_BASE_ENABLE    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+1
		case GSR_CONFIG_ALARM_SENSOR_BASE_TYPE    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+2
		case GSR_CONFIG_ALARM_SENSOR_BASE_NAME    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+3
		
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x20
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALLCHN     : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME    : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE: {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		
		
		
		//case GSR_CONFIG_ALARM_SENSOR_SCH    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x40
		case GSR_CONFIG_ALARM_SENSOR_SCH_CHN  : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+0
		case GSR_CONFIG_ALARM_SENSOR_SCH_SCH  : 
		{
			SBizSchedule sCfgIns;
			
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_SENSOR;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				BIZ_DO_DBG("sCfgIns.time0 start %x end %x\n", 
					sCfgIns.sSchTime[0][0].nStartTime,
					sCfgIns.sSchTime[0][0].nStopTime
				);
				bizData_GetSch(&sCfgIns, sch);
				
				return 0;
			}
			
			BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_SCH_SCH err\n");
			
		} break; // GSR_CONFIG_ALARM_SENSOR_SCH+1
		case GSR_CONFIG_ALARM_IPCEXTSENSOR_SCH_SCH  : 
		{
			SBizSchedule sCfgIns;
			
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_IPCEXTSENSOR;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				BIZ_DO_DBG("sCfgIns.time0 start %x end %x\n", 
					sCfgIns.sSchTime[0][0].nStartTime,
					sCfgIns.sSchTime[0][0].nStopTime
				);
				bizData_GetSch(&sCfgIns, sch);
				
				return 0;
			}
			
			BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_SCH_SCH err\n");
			
		} break;
		case GSR_CONFIG_ALARM_SENSOR_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+2
		
		///
		
		//case GSR_CONFIG_ALARM_MD            : {;} break; // GSR_CONFIG_ALARM_BASE+0x40
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT     : {;} break; // GSR_CONFIG_ALARM_MD+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALLCHN  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+2
		
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+3
		
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_TYPE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_INDEX : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+1
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET  : {;} break; // GSR_CONFIG_ALARM_MD+0x28
		
		
		//case GSR_CONFIG_ALARM_MD_SCH      : {;} break; // GSR_CONFIG_ALARM_MD+0x30
		case GSR_CONFIG_ALARM_MD_SCH_SCH_CHN  : {;} break; // GSR_CONFIG_ALARM_MD_SCH+0
		case GSR_CONFIG_ALARM_MD_SCH_SCH_SCH  : 
		{
			SBizSchedule sCfgIns;
			
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_VMOTION;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				BIZ_DO_DBG("sCfgIns.time0 start %x end %x\n", 
					sCfgIns.sSchTime[0][0].nStartTime,
					sCfgIns.sSchTime[0][0].nStopTime
				);
				bizData_GetSch(&sCfgIns, sch);
				
				return 0;
			}
			BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_SCH_SCH err\n");
			
		} break; // GSR_CONFIG_ALARM_MD_SCH+1
		case GSR_CONFIG_ALARM_MD_SCH_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_MD_SCH+2
		
		
		//case GSR_CONFIG_ALARM_VIDEOLOST     : {;} break; // GSR_CONFIG_ALARM_BASE+0x80
		
		//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST+0
		
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_TYPE  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_INDEX : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+1
		
		
		//case GSR_CONFIG_ALARM_OTHER           : {;} break; // GSR_CONFIG_ALARM_BASE+0xA0
		
		case GSR_CONFIG_ALARM_OTHER_ALARMTYPE     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+1
		case GSR_CONFIG_ALARM_OTHER_SOUNDALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+2
		case GSR_CONFIG_ALARM_OTHER_MAIL        : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+3
		case GSR_CONFIG_ALARM_OTHER_TRIGGERALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+4
		case GSR_CONFIG_ALARM_OTHER_DISKALARM     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+5
		case GSR_CONFIG_ALARM_OTHER_ALLCHN      : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+6
		
		
		case GSR_CONFIG_ALARM_ALARMOUT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xB0
		
		
		case GSR_CONFIG_ALARM_EXIT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xC0
		
		
		//-------------------------------network
		
		case GSR_CONFIG_NETWORK_BASE    : {;} break; // GSR_CONFIG_BASE+0x500
		
		
		//-------------------------------user
		
		case GSR_CONFIG_USER_BASE       : {;} break; // GSR_CONFIG_BASE+0x600
		
		
		//-------------------------------ptz
		case GSR_PTZCONFIG_SERIAL_PROTOCOL    :   
		{
			
		} break; // GSR_PTZCONFIG_BASE+4
		case GSR_PTZCONFIG_SERIAL_ALLCHECK    : {;} break; // GSR_PTZCONFIG_BASE+5
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_BASE
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ID    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+2
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_ID  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_NAME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+2
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_PRESET: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+3
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_RATE  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+4
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_TIME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+5
		
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_TRACK_BASE
		
		
		
		//-------------------------------advance
		//case GSR_CONFIG_ADVANCE     : {;} break; // GSR_CONFIG_BASE+0x800
		
		
		case GSR_ADVANCECONFIG_IMPORTEXPORT   : {;} break; // GSR_ADVANCECONFIG_BASE+0
		case GSR_ADVANCECONFIG_IMPORTEXPORT_REFRESH   : {;} break; // GSR_ADVANCECONFIG_IMPORTEXPORT+1
		
		
		// === search ===
		
		// === backup ===
		
		// === info ===
		
		//-------------------------------system
		
		case GSR_INFO_SYSTEM_DEVICENAME   : {;} break; // GSR_INFO_SYSTEM_BASE+1
		case GSR_INFO_SYSTEM_DEVICENUM  : {;} break; // GSR_INFO_SYSTEM_BASE+2
		case GSR_INFO_SYSTEM_HARDWAREVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+3
		case GSR_INFO_SYSTEM_SCMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+4
		case GSR_INFO_SYSTEM_KERNELVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+5
		case GSR_INFO_SYSTEM_ROMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+6
		case GSR_INFO_SYSTEM_RELEASEDATE  : {;} break; // GSR_INFO_SYSTEM_BASE+7
		
		
		case GSR_INFO_DISK_NO   : {;} break; // GSR_INFO_DISK_BASE+1
		case GSR_INFO_DISK_TYPE   : {;} break; // GSR_INFO_DISK_BASE+2
		case GSR_INFO_DISK_CAPACITY   : {;} break; // GSR_INFO_DISK_BASE+3
		case GSR_INFO_DISK_FREE   : {;} break; // GSR_INFO_DISK_BASE+4
		case GSR_INFO_DISK_STATE  : {;} break; // GSR_INFO_DISK_BASE+5
		case GSR_INFO_DISK_ATTRIBUTE  : {;} break; // GSR_INFO_DISK_BASE+6
		case GSR_INFO_DISK_SOURCE   : {;} break; // GSR_INFO_DISK_BASE+7
		case GSR_INFO_DISK_RSV1   : {;} break; // GSR_INFO_DISK_BASE+8
		case GSR_INFO_DISK_RSV2   : {;} break; // GSR_INFO_DISK_BASE+9
		case GSR_INFO_DISK_IMAGE  : {;} break; // GSR_INFO_DISK_BASE+10
		case GSR_INFO_DISK_TOTAL  : {;} break; // GSR_INFO_DISK_BASE+11
		case GSR_INFO_DISK_USED   : {;} break; // GSR_INFO_DISK_BASE+12
		
		
		//-------------------------------event
		
		
		//-------------------------------syslog
		
		
		//------------------------------- netinfo
		
		
		//------------------------------- online
		
		case GSR_INFO_ONLINE_USERNAME : {;} break; // GSR_INFO_ONLINE_BASE+1
		case GSR_INFO_ONLINE_IP     : {;} break; // GSR_INFO_ONLINE_BASE+2
		case GSR_INFO_ONLINE_STATE  : {;} break; // GSR_INFO_ONLINE_BASE+3
		
		// exit
		
		
		
		// === alarm ===
		
		
		case GSR_MANUALALARM_ALLCHECK : {;} break; // GSR_MANUALALARM_BASE+1
		case GSR_MANUALALARM_ALARMNAME: {;} break; // GSR_MANUALALARM_BASE+2
		case GSR_MANUALALARM_IP     : {;} break; // GSR_MANUALALARM_BASE+3
		case GSR_MANUALALARM_ALARMPROCESS : {;} break; // GSR_MANUALALARM_BASE+4
		
		// === disk ===
		
		
		
		// === resource ===
		
		
		
		// === logoff ===
		
		
		
		// === shutdown ===
		
		
		// === desktop ===
		
		case GSR_DESKTOP_SYSTIME  : {;} break; // GSR_DESKTOP_BASE+1
		case GSR_DESKTOP_CHNNAME  : {;} break; // GSR_DESKTOP_BASE+2
		
		//case GSR_DESKTOP_VIDEO_ATTRIBUTE  : {;} break; // GSR_DESKTOP_BASE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+0
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+1
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+2
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CONTRAST   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+4
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_DEFAULT  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+5
		

	}
	
	return 0;
}

int GetScreenStrSelEDIT(u32 nItemID, u8 chn, char* szValue  )
{
	int rtn = 0;
	SBizParaTarget sParaTgtIns;

	if(nItemID>=GSR_DEFAULT_OFFSET)
	{
		switch(nItemID)
		{	
			case GSR_CONFIG_BASIC_SYSTEM_DEVICENAME       + GSR_DEFAULT_OFFSET :
				{;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+0
			case GSR_CONFIG_BASIC_SYSTEM_DEVICEID       : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+1
			case GSR_CONFIG_BASIC_SYSTEM_VIDEOFORMAT      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+2
			case GSR_CONFIG_BASIC_SYSTEM_AUTHCHECK      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+3
			case GSR_CONFIG_BASIC_SYSTEM_TIMEDISPLAY      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+4
			case GSR_CONFIG_BASIC_SYSTEM_MAXNETUSERS      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+5
			case GSR_CONFIG_BASIC_SYSTEM_OUTPUT         : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+6
			case GSR_CONFIG_BASIC_SYSTEM_LANGUAGE       : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+7
			case GSR_CONFIG_BASIC_SYSTEM_SCRENNSAVER      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+8
			
			
			//date&time..
			
			case GSR_CONFIG_BASIC_DATETIME_DATEFORMAT         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+0
			case GSR_CONFIG_BASIC_DATETIME_TIMEFORMAT         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+1
			case GSR_CONFIG_BASIC_DATETIME_TIMEZONE         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+2
			case GSR_CONFIG_BASIC_DATETIME_NETSYN         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+3
			case GSR_CONFIG_BASIC_DATETIME_NTPSERVER          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+4
			case GSR_CONFIG_BASIC_DATETIME_SYSDATE          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+5
			case GSR_CONFIG_BASIC_DATETIME_SYSTIME          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+6
			case GSR_CONFIG_BASIC_DATETIME_SYNCTIMETOIPC    : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+7
			
			//-------------------------------live
			
			//case GSR_CONFIG_LIVE_LIVE             : {;} break; // GSR_CONFIG_LIVE_BASE+0
			case GSR_CONFIG_LIVE_ALLCHN             : {;} break; // GSR_CONFIG_LIVE_BASE+1
			case GSR_CONFIG_LIVE_CHNNAME            : {;} break; // GSR_CONFIG_LIVE_BASE+2
			case GSR_CONFIG_LIVE_DISPLAY            : {;} break; // GSR_CONFIG_LIVE_BASE+3
			case GSR_CONFIG_LIVE_LIVEREC            : {;} break; // GSR_CONFIG_LIVE_BASE+4
			case GSR_CONFIG_LIVE_COLORSET             : {;} break; // GSR_CONFIG_LIVE_BASE+5
			
			//case GSR_CONFIG_LIVE_MAINMONITOR            : {;} break; // GSR_CONFIG_LIVE_BASE+0x40
			case GSR_CONFIG_LIVE_MAINMONITOR_SPLITMODE      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+0
			case GSR_CONFIG_LIVE_MAINMONITOR_CHN      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+1
			case GSR_CONFIG_LIVE_MAINMONITOR_DWELLTIME      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+2
			
			//case GSR_CONFIG_LIVE_SPOT             : {;} break; // GSR_CONFIG_LIVE_BASE+0x80
			case GSR_CONFIG_LIVE_SPOT_SPLITMODE             : {;} break; // GSR_CONFIG_LIVE_SPOT+0
			case GSR_CONFIG_LIVE_SPOT_CHN             : {;} break; // GSR_CONFIG_LIVE_SPOT+1
			case GSR_CONFIG_LIVE_SPOT_DWELLTIME             : {;} break; // GSR_CONFIG_LIVE_SPOT+2
			
			//case GSR_CONFIG_LIVE_MASK             : {;} break; // GSR_CONFIG_LIVE_BASE+0xc0
			case GSR_CONFIG_LIVE_MASK_AREA            : {;} break; // GSR_CONFIG_LIVE_MASK+0
			
			
			
			//-------------------------------record
			
			
			case GSR_CONFIG_RECORD_ENABLE_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_BASE+0
			case GSR_CONFIG_RECORD_ENABLE_RECORD        : {;} break; // GSR_CONFIG_RECORD_BASE+1
			case GSR_CONFIG_RECORD_ENABLE_AUDIO         : {;} break; // GSR_CONFIG_RECORD_BASE+2
			
			
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ALLCHN         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+0
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_RESOLUTION         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+1
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_FRAMERATE        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+2
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ENCODE         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+3
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_QUALITY        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+4
			case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_MAXRATE      : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+5
			
			
			case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_ALLCHN       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+0
			case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEBEFALARM   : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+1
			case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEAFTALARM       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+2
			case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_EXPIRATION       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+3
			
			
			case GSR_CONFIG_RECORD_ENABLE_OSD_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+0
			case GSR_CONFIG_RECORD_ENABLE_OSD_CHNNAME         : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+1
			case GSR_CONFIG_RECORD_ENABLE_OSD_TIMESTAMP         : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+2
			case GSR_CONFIG_RECORD_ENABLE_OSD_POSITION        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+3
			
			
			case GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_REC        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_BASE+0
			
			
			
			
			
			//-------------------------------schedule
			
			//case GSR_CONFIG_SCHEDULE_TIMEREC  : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0
			case GSR_CONFIG_SCHEDULE_TIMEREC_CHN  : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+0
			case GSR_CONFIG_SCHEDULE_TIMEREC_SCH  : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+1
			case GSR_CONFIG_SCHEDULE_TIMEREC_COPYTO   : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+2
			
			
			//case GSR_CONFIG_SCHEDULE_MOTIONALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x20
			case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+0
			case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH   : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+1
			case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+2
			
			//case GSR_CONFIG_SCHEDULE_SENSORALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x30
			case GSR_CONFIG_SCHEDULE_SENSORALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+0
			case GSR_CONFIG_SCHEDULE_SENSORALARMREC_SCH   : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+1
			case GSR_CONFIG_SCHEDULE_SENSORALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+2
			
			
			//-------------------------------alarm
			
			//case GSR_CONFIG_ALARM_SENSOR    : {;} break; // GSR_CONFIG_ALARM_BASE+0
			
			case GSR_CONFIG_ALARM_SENSOR_BASE_ALLCHN    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+0
			case GSR_CONFIG_ALARM_SENSOR_BASE_ENABLE    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+1
			case GSR_CONFIG_ALARM_SENSOR_BASE_TYPE    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+2
			case GSR_CONFIG_ALARM_SENSOR_BASE_NAME    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+3
			
			
			//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x20
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALLCHN     : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+0
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME    : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+1
			
			//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE: {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME+1
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+0
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+1
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+2
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+3
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+4 // bitmap
			
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP+1
			
			//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+0
			case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
			
			
			
			//case GSR_CONFIG_ALARM_SENSOR_SCH    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x40
			case GSR_CONFIG_ALARM_SENSOR_SCH_CHN  : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+0
			case GSR_CONFIG_ALARM_SENSOR_SCH_SCH  : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+1
			case GSR_CONFIG_ALARM_SENSOR_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+2
			
			///
			
			//case GSR_CONFIG_ALARM_MD            : {;} break; // GSR_CONFIG_ALARM_BASE+0x40
			//case GSR_CONFIG_ALARM_MD_MOTIONDETECT     : {;} break; // GSR_CONFIG_ALARM_MD+0
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+0
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+1
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALLCHN  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+2
			
			//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+3
			
			
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+0
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+1
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+2
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+3
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+4 // bitmap
			
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x10
			
			//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x20
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_TYPE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+0
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_INDEX : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+1
			
			case GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET  : {;} break; // GSR_CONFIG_ALARM_MD+0x28
			
			
			//case GSR_CONFIG_ALARM_MD_SCH      : {;} break; // GSR_CONFIG_ALARM_MD+0x30
			case GSR_CONFIG_ALARM_MD_SCH_SCH_CHN  : {;} break; // GSR_CONFIG_ALARM_MD_SCH+0
			case GSR_CONFIG_ALARM_MD_SCH_SCH_SCH  : {;} break; // GSR_CONFIG_ALARM_MD_SCH+1
			case GSR_CONFIG_ALARM_MD_SCH_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_MD_SCH+2
			
			
			//case GSR_CONFIG_ALARM_VIDEOLOST     : {;} break; // GSR_CONFIG_ALARM_BASE+0x80
			
			//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST+0
			
			
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+0
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+1
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+2
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+3
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+4 // bitmap
			
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x10
			
			//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x20
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_TYPE  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0
			case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_INDEX : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+1
			
			
			//case GSR_CONFIG_ALARM_OTHER           : {;} break; // GSR_CONFIG_ALARM_BASE+0xA0
			
			case GSR_CONFIG_ALARM_OTHER_ALARMTYPE     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+1
			case GSR_CONFIG_ALARM_OTHER_SOUNDALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+2
			case GSR_CONFIG_ALARM_OTHER_MAIL        : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+3
			case GSR_CONFIG_ALARM_OTHER_TRIGGERALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+4
			case GSR_CONFIG_ALARM_OTHER_DISKALARM     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+5
			case GSR_CONFIG_ALARM_OTHER_ALLCHN      : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+6
			
			
			case GSR_CONFIG_ALARM_ALARMOUT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xB0
			
			
			case GSR_CONFIG_ALARM_EXIT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xC0
			
			
			//-------------------------------network
			
			case GSR_CONFIG_NETWORK_BASE    : {;} break; // GSR_CONFIG_BASE+0x500
			
			
			//-------------------------------user
			
			case GSR_CONFIG_USER_BASE       : {;} break; // GSR_CONFIG_BASE+0x600
			
			
			//-------------------------------ptz
			
			
			
			case GSR_PTZCONFIG_SERIAL_ENABLE    : {;} break; // GSR_PTZCONFIG_BASE+1
			case GSR_PTZCONFIG_SERIAL_ADDRESS     : {;} break; // GSR_PTZCONFIG_BASE+2
			case GSR_PTZCONFIG_SERIAL_BPS     : {;} break; // GSR_PTZCONFIG_BASE+3
			case GSR_PTZCONFIG_SERIAL_PROTOCOL    + GSR_DEFAULT_OFFSET :
			{
			#if 0
				SBizPtzPara sCfgIns;
				
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
				sParaTgtIns.nChn = chn;
				rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
				if(0==rtn)
				{
					if(szValue)
						strcpy(szValue, (char*)sCfgIns.strProtocolName);
					
					return 0;
				}
				BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_PROTOCOL err\n");;
			#endif
			} break; // GSR_PTZCONFIG_BASE+4
			case GSR_PTZCONFIG_SERIAL_ALLCHECK    : {;} break; // GSR_PTZCONFIG_BASE+5
			
			//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_BASE
			
			//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE
			
			case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+1
			case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ID    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+2
			
			//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE
			
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_ID  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+1
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_NAME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+2
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_PRESET: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+3
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_RATE  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+4
			case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_TIME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+5
			
			
			//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_TRACK_BASE
			
			
			
			//-------------------------------advance
			//case GSR_CONFIG_ADVANCE     : {;} break; // GSR_CONFIG_BASE+0x800
			
			
			case GSR_ADVANCECONFIG_IMPORTEXPORT   : {;} break; // GSR_ADVANCECONFIG_BASE+0
			case GSR_ADVANCECONFIG_IMPORTEXPORT_REFRESH   : {;} break; // GSR_ADVANCECONFIG_IMPORTEXPORT+1
			
			
			// === search ===
			
			// === backup ===
			
			// === info ===
			
			//-------------------------------system
			
			case GSR_INFO_SYSTEM_DEVICENAME   : {;} break; // GSR_INFO_SYSTEM_BASE+1
			case GSR_INFO_SYSTEM_DEVICENUM  : {;} break; // GSR_INFO_SYSTEM_BASE+2
			case GSR_INFO_SYSTEM_HARDWAREVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+3
			case GSR_INFO_SYSTEM_SCMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+4
			case GSR_INFO_SYSTEM_KERNELVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+5
			case GSR_INFO_SYSTEM_ROMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+6
			case GSR_INFO_SYSTEM_RELEASEDATE  : {;} break; // GSR_INFO_SYSTEM_BASE+7
			
			
			case GSR_INFO_DISK_NO   : {;} break; // GSR_INFO_DISK_BASE+1
			case GSR_INFO_DISK_TYPE   : {;} break; // GSR_INFO_DISK_BASE+2
			case GSR_INFO_DISK_CAPACITY   : {;} break; // GSR_INFO_DISK_BASE+3
			case GSR_INFO_DISK_FREE   : {;} break; // GSR_INFO_DISK_BASE+4
			case GSR_INFO_DISK_STATE  : {;} break; // GSR_INFO_DISK_BASE+5
			case GSR_INFO_DISK_ATTRIBUTE  : {;} break; // GSR_INFO_DISK_BASE+6
			case GSR_INFO_DISK_SOURCE   : {;} break; // GSR_INFO_DISK_BASE+7
			case GSR_INFO_DISK_RSV1   : {;} break; // GSR_INFO_DISK_BASE+8
			case GSR_INFO_DISK_RSV2   : {;} break; // GSR_INFO_DISK_BASE+9
			case GSR_INFO_DISK_IMAGE  : {;} break; // GSR_INFO_DISK_BASE+10
			case GSR_INFO_DISK_TOTAL  : {;} break; // GSR_INFO_DISK_BASE+11
			case GSR_INFO_DISK_USED   : {;} break; // GSR_INFO_DISK_BASE+12
			
			
			//-------------------------------event
			
			
			//-------------------------------syslog
			
			
			//------------------------------- netinfo
			
			
			//------------------------------- online
			
			case GSR_INFO_ONLINE_USERNAME : {;} break; // GSR_INFO_ONLINE_BASE+1
			case GSR_INFO_ONLINE_IP     : {;} break; // GSR_INFO_ONLINE_BASE+2
			case GSR_INFO_ONLINE_STATE  : {;} break; // GSR_INFO_ONLINE_BASE+3
			
			// exit
			
			
			
			// === alarm ===
			
			
			case GSR_MANUALALARM_ALLCHECK : {;} break; // GSR_MANUALALARM_BASE+1
			case GSR_MANUALALARM_ALARMNAME: {;} break; // GSR_MANUALALARM_BASE+2
			case GSR_MANUALALARM_IP     : {;} break; // GSR_MANUALALARM_BASE+3
			case GSR_MANUALALARM_ALARMPROCESS : {;} break; // GSR_MANUALALARM_BASE+4
			
			// === disk ===
			
			
			
			// === resource ===
			
			
			
			// === logoff ===
			
			
			
			// === shutdown ===
			
			
			// === desktop ===
			
			case GSR_DESKTOP_SYSTIME  : {;} break; // GSR_DESKTOP_BASE+1
			case GSR_DESKTOP_CHNNAME  : {;} break; // GSR_DESKTOP_BASE+2
			
			//case GSR_DESKTOP_VIDEO_ATTRIBUTE  : {;} break; // GSR_DESKTOP_BASE+3
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+0
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+1
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+2
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+3
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_CONTRAST   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+4
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_DEFAULT  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+5
			

		}

		return 0;
	}

	switch(nItemID)
	{	
		case GSR_CONFIG_BASIC_SYSTEM_DEVICENAME       : 
			{;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+0
		case GSR_CONFIG_BASIC_SYSTEM_DEVICEID       : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+1
		case GSR_CONFIG_BASIC_SYSTEM_VIDEOFORMAT      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+2
		case GSR_CONFIG_BASIC_SYSTEM_AUTHCHECK      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+3
		case GSR_CONFIG_BASIC_SYSTEM_TIMEDISPLAY      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+4
		case GSR_CONFIG_BASIC_SYSTEM_MAXNETUSERS      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+5
		case GSR_CONFIG_BASIC_SYSTEM_OUTPUT         : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+6
		case GSR_CONFIG_BASIC_SYSTEM_LANGUAGE       : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+7
		case GSR_CONFIG_BASIC_SYSTEM_SCRENNSAVER      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+8
		
		
		//date&time..
		
		case GSR_CONFIG_BASIC_DATETIME_DATEFORMAT         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+0
		case GSR_CONFIG_BASIC_DATETIME_TIMEFORMAT         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+1
		case GSR_CONFIG_BASIC_DATETIME_TIMEZONE         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+2
		case GSR_CONFIG_BASIC_DATETIME_NETSYN         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+3
		case GSR_CONFIG_BASIC_DATETIME_NTPSERVER          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+4
		case GSR_CONFIG_BASIC_DATETIME_SYSDATE          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+5
		case GSR_CONFIG_BASIC_DATETIME_SYSTIME          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+6
		case GSR_CONFIG_BASIC_DATETIME_SYNCTIMETOIPC    : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+7
		
		
		//-------------------------------live
		
		//case GSR_CONFIG_LIVE_LIVE             : {;} break; // GSR_CONFIG_LIVE_BASE+0
		case GSR_CONFIG_LIVE_ALLCHN             : {;} break; // GSR_CONFIG_LIVE_BASE+1
		case GSR_CONFIG_LIVE_CHNNAME            : {;} break; // GSR_CONFIG_LIVE_BASE+2
		case GSR_CONFIG_LIVE_DISPLAY            : {;} break; // GSR_CONFIG_LIVE_BASE+3
		case GSR_CONFIG_LIVE_LIVEREC            : {;} break; // GSR_CONFIG_LIVE_BASE+4
		case GSR_CONFIG_LIVE_COLORSET             : {;} break; // GSR_CONFIG_LIVE_BASE+5
		
		//case GSR_CONFIG_LIVE_MAINMONITOR            : {;} break; // GSR_CONFIG_LIVE_BASE+0x40
		case GSR_CONFIG_LIVE_MAINMONITOR_SPLITMODE      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+0
		case GSR_CONFIG_LIVE_MAINMONITOR_CHN      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+1
		case GSR_CONFIG_LIVE_MAINMONITOR_DWELLTIME      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+2
		
		//case GSR_CONFIG_LIVE_SPOT             : {;} break; // GSR_CONFIG_LIVE_BASE+0x80
		case GSR_CONFIG_LIVE_SPOT_SPLITMODE             : {;} break; // GSR_CONFIG_LIVE_SPOT+0
		case GSR_CONFIG_LIVE_SPOT_CHN             : {;} break; // GSR_CONFIG_LIVE_SPOT+1
		case GSR_CONFIG_LIVE_SPOT_DWELLTIME             : {;} break; // GSR_CONFIG_LIVE_SPOT+2
		
		//case GSR_CONFIG_LIVE_MASK             : {;} break; // GSR_CONFIG_LIVE_BASE+0xc0
		case GSR_CONFIG_LIVE_MASK_AREA            : {;} break; // GSR_CONFIG_LIVE_MASK+0
		
		
		
		//-------------------------------record
		
		
		case GSR_CONFIG_RECORD_ENABLE_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_RECORD        : {;} break; // GSR_CONFIG_RECORD_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_AUDIO         : {;} break; // GSR_CONFIG_RECORD_BASE+2
		
		
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ALLCHN         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_RESOLUTION         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_FRAMERATE        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ENCODE         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+3
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_QUALITY        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+4
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_MAXRATE      : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+5
		
		
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_ALLCHN       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEBEFALARM   : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEAFTALARM       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_EXPIRATION       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+3
		
		
		case GSR_CONFIG_RECORD_ENABLE_OSD_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_OSD_CHNNAME         : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_OSD_TIMESTAMP         : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_OSD_POSITION        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+3
		
		
		case GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_REC        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_BASE+0
		
		
		
		
		
		//-------------------------------schedule
		
		//case GSR_CONFIG_SCHEDULE_TIMEREC  : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0
		case GSR_CONFIG_SCHEDULE_TIMEREC_CHN  : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+0
		case GSR_CONFIG_SCHEDULE_TIMEREC_SCH  : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+1
		case GSR_CONFIG_SCHEDULE_TIMEREC_COPYTO   : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+2
		
		
		//case GSR_CONFIG_SCHEDULE_MOTIONALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x20
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+0
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH   : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+1
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+2
		
		//case GSR_CONFIG_SCHEDULE_SENSORALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x30
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+0
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_SCH   : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+1
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+2
		
		
		//-------------------------------alarm
		
		//case GSR_CONFIG_ALARM_SENSOR    : {;} break; // GSR_CONFIG_ALARM_BASE+0
		
		case GSR_CONFIG_ALARM_SENSOR_BASE_ALLCHN    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+0
		case GSR_CONFIG_ALARM_SENSOR_BASE_ENABLE    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+1
		case GSR_CONFIG_ALARM_SENSOR_BASE_TYPE    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+2
		case GSR_CONFIG_ALARM_SENSOR_BASE_NAME    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+3
		
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x20
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALLCHN     : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME    : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE: {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		
		
		
		//case GSR_CONFIG_ALARM_SENSOR_SCH    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x40
		case GSR_CONFIG_ALARM_SENSOR_SCH_CHN  : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+0
		case GSR_CONFIG_ALARM_SENSOR_SCH_SCH  : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+1
		case GSR_CONFIG_ALARM_SENSOR_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+2
		
		///
		
		//case GSR_CONFIG_ALARM_MD            : {;} break; // GSR_CONFIG_ALARM_BASE+0x40
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT     : {;} break; // GSR_CONFIG_ALARM_MD+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALLCHN  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+2
		
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+3
		
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_TYPE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_INDEX : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+1
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET  : {;} break; // GSR_CONFIG_ALARM_MD+0x28
		
		
		//case GSR_CONFIG_ALARM_MD_SCH      : {;} break; // GSR_CONFIG_ALARM_MD+0x30
		case GSR_CONFIG_ALARM_MD_SCH_SCH_CHN  : {;} break; // GSR_CONFIG_ALARM_MD_SCH+0
		case GSR_CONFIG_ALARM_MD_SCH_SCH_SCH  : {;} break; // GSR_CONFIG_ALARM_MD_SCH+1
		case GSR_CONFIG_ALARM_MD_SCH_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_MD_SCH+2
		
		
		//case GSR_CONFIG_ALARM_VIDEOLOST     : {;} break; // GSR_CONFIG_ALARM_BASE+0x80
		
		//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST+0
		
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_TYPE  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_INDEX : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+1
		
		
		//case GSR_CONFIG_ALARM_OTHER           : {;} break; // GSR_CONFIG_ALARM_BASE+0xA0
		
		case GSR_CONFIG_ALARM_OTHER_ALARMTYPE     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+1
		case GSR_CONFIG_ALARM_OTHER_SOUNDALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+2
		case GSR_CONFIG_ALARM_OTHER_MAIL        : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+3
		case GSR_CONFIG_ALARM_OTHER_TRIGGERALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+4
		case GSR_CONFIG_ALARM_OTHER_DISKALARM     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+5
		case GSR_CONFIG_ALARM_OTHER_ALLCHN      : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+6
		
		
		case GSR_CONFIG_ALARM_ALARMOUT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xB0
		
		
		case GSR_CONFIG_ALARM_EXIT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xC0
		
		
		//-------------------------------network
		
		case GSR_CONFIG_NETWORK_BASE    : {;} break; // GSR_CONFIG_BASE+0x500
		
		
		//-------------------------------user
		
		case GSR_CONFIG_USER_BASE       : {;} break; // GSR_CONFIG_BASE+0x600
		
		
		//-------------------------------ptz
		
		
		
		case GSR_PTZCONFIG_SERIAL_ENABLE    : {;} break; // GSR_PTZCONFIG_BASE+1
		case GSR_PTZCONFIG_SERIAL_ADDRESS     : {;} break; // GSR_PTZCONFIG_BASE+2
		case GSR_PTZCONFIG_SERIAL_BPS     : {;} break; // GSR_PTZCONFIG_BASE+3
		case GSR_PTZCONFIG_SERIAL_PROTOCOL    : 
		{
		#if 0
			SBizPtzPara sCfgIns;
			
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				if(szValue)
					strcpy(szValue, (char*)sCfgIns.strProtocolName);
				
				return 0;
			}
			BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_PROTOCOL err\n");;
		#endif
		} break; // GSR_PTZCONFIG_BASE+4
		case GSR_PTZCONFIG_SERIAL_ALLCHECK    : {;} break; // GSR_PTZCONFIG_BASE+5
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_BASE
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ID    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+2
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_ID  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_NAME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+2
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_PRESET: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+3
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_RATE  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+4
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_TIME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+5
		
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_TRACK_BASE
		
		
		
		//-------------------------------advance
		//case GSR_CONFIG_ADVANCE     : {;} break; // GSR_CONFIG_BASE+0x800
		
		
		case GSR_ADVANCECONFIG_IMPORTEXPORT   : {;} break; // GSR_ADVANCECONFIG_BASE+0
		case GSR_ADVANCECONFIG_IMPORTEXPORT_REFRESH   : {;} break; // GSR_ADVANCECONFIG_IMPORTEXPORT+1
		
		
		// === search ===
		
		// === backup ===
		
		// === info ===
		
		//-------------------------------system
		
		case GSR_INFO_SYSTEM_DEVICENAME   : {;} break; // GSR_INFO_SYSTEM_BASE+1
		case GSR_INFO_SYSTEM_DEVICENUM  : {;} break; // GSR_INFO_SYSTEM_BASE+2
		case GSR_INFO_SYSTEM_HARDWAREVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+3
		case GSR_INFO_SYSTEM_SCMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+4
		case GSR_INFO_SYSTEM_KERNELVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+5
		case GSR_INFO_SYSTEM_ROMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+6
		case GSR_INFO_SYSTEM_RELEASEDATE  : {;} break; // GSR_INFO_SYSTEM_BASE+7
		
		
		case GSR_INFO_DISK_NO   : {;} break; // GSR_INFO_DISK_BASE+1
		case GSR_INFO_DISK_TYPE   : {;} break; // GSR_INFO_DISK_BASE+2
		case GSR_INFO_DISK_CAPACITY   : {;} break; // GSR_INFO_DISK_BASE+3
		case GSR_INFO_DISK_FREE   : {;} break; // GSR_INFO_DISK_BASE+4
		case GSR_INFO_DISK_STATE  : {;} break; // GSR_INFO_DISK_BASE+5
		case GSR_INFO_DISK_ATTRIBUTE  : {;} break; // GSR_INFO_DISK_BASE+6
		case GSR_INFO_DISK_SOURCE   : {;} break; // GSR_INFO_DISK_BASE+7
		case GSR_INFO_DISK_RSV1   : {;} break; // GSR_INFO_DISK_BASE+8
		case GSR_INFO_DISK_RSV2   : {;} break; // GSR_INFO_DISK_BASE+9
		case GSR_INFO_DISK_IMAGE  : {;} break; // GSR_INFO_DISK_BASE+10
		case GSR_INFO_DISK_TOTAL  : {;} break; // GSR_INFO_DISK_BASE+11
		case GSR_INFO_DISK_USED   : {;} break; // GSR_INFO_DISK_BASE+12
		
		
		//-------------------------------event
		
		
		//-------------------------------syslog
		
		
		//------------------------------- netinfo
		
		
		//------------------------------- online
		
		case GSR_INFO_ONLINE_USERNAME : {;} break; // GSR_INFO_ONLINE_BASE+1
		case GSR_INFO_ONLINE_IP     : {;} break; // GSR_INFO_ONLINE_BASE+2
		case GSR_INFO_ONLINE_STATE  : {;} break; // GSR_INFO_ONLINE_BASE+3
		
		// exit
		
		
		
		// === alarm ===
		
		
		case GSR_MANUALALARM_ALLCHECK : {;} break; // GSR_MANUALALARM_BASE+1
		case GSR_MANUALALARM_ALARMNAME: {;} break; // GSR_MANUALALARM_BASE+2
		case GSR_MANUALALARM_IP     : {;} break; // GSR_MANUALALARM_BASE+3
		case GSR_MANUALALARM_ALARMPROCESS : {;} break; // GSR_MANUALALARM_BASE+4
		
		// === disk ===
		
		
		
		// === resource ===
		
		
		
		// === logoff ===
		
		
		
		// === shutdown ===
		
		
		// === desktop ===
		
		case GSR_DESKTOP_SYSTIME  : {;} break; // GSR_DESKTOP_BASE+1
		case GSR_DESKTOP_CHNNAME  : {;} break; // GSR_DESKTOP_BASE+2
		
		//case GSR_DESKTOP_VIDEO_ATTRIBUTE  : {;} break; // GSR_DESKTOP_BASE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+0
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+1
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+2
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CONTRAST   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+4
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_DEFAULT  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+5
		

	}
}

int GetScreenStrSelOSD(u32 nItemID, u8 chn, char* szValue  )
{
	int rtn = 0;
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));
	
	if(!szValue)
		return -1;

	if(nItemID>=GSR_DEFAULT_OFFSET)
	{
		switch(nItemID)
		{	
			// === desktop ===
			
			case GSR_DESKTOP_SYSTIME  + GSR_DEFAULT_OFFSET :
			{
				BizSysComplexDTGetString(szValue);
			} 
			break; // GSR_DESKTOP_BASE+1
			case GSR_DESKTOP_CHNNAME  + GSR_DEFAULT_OFFSET :
			{
				SBizCfgStrOsd sStrOds;
				memset(&sStrOds, 0, sizeof(sStrOds));
				sParaTgtIns.emBizParaType = EM_BIZ_STROSD;
				sParaTgtIns.nChn = chn;
				rtn = BizGetPara(&sParaTgtIns, &sStrOds);
				if(0==rtn)
				{
					if(szValue)
					{
						strcpy(szValue, (char*)sStrOds.strChnName);
					}
				}
				else
				{
					BIZ_DATA_DBG("GSR_DESKTOP_CHNNAME err\n");
				}
			} 
			break; // GSR_DESKTOP_BASE+2
			
			//case GSR_DESKTOP_VIDEO_ATTRIBUTE  : {;} break; // GSR_DESKTOP_BASE+3
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN  + GSR_DEFAULT_OFFSET :
			{
				;
			} 
			break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+0
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS   :
			break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+1
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+2
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+3
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_CONTRAST   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+4
			case GSR_DESKTOP_VIDEO_ATTRIBUTE_DEFAULT  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+5
			

		}
		return 0;
	}


	switch(nItemID)
	{	
		// === desktop ===
		
		case GSR_DESKTOP_SYSTIME  : 
		{
			BizSysComplexDTGetString(szValue);
		} 
		break; // GSR_DESKTOP_BASE+1
		case GSR_DESKTOP_CHNNAME  : 
		{
			SBizCfgStrOsd sStrOds;
			memset(&sStrOds, 0, sizeof(sStrOds));
			sParaTgtIns.emBizParaType = EM_BIZ_STROSD;
			sParaTgtIns.nChn = chn;
			rtn = BizGetPara(&sParaTgtIns, &sStrOds);
			if(0==rtn)
			{
				if(szValue)
				{
					strcpy(szValue, (char*)sStrOds.strChnName);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_CHNNAME err\n");
			}
		} 
		break; // GSR_DESKTOP_BASE+2
		
		//case GSR_DESKTOP_VIDEO_ATTRIBUTE  : {;} break; // GSR_DESKTOP_BASE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN  : 
		{
			;
		} 
		break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+0
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS   :
		break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+1
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+2
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CONTRAST   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+4
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_DEFAULT  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+5
		

	}
}

int GetScreenStrSelSZLIST( u32 nItemID, u8 nCh, SCombList* pList )
{
	int i, rtn = 0;
	SBizParaTarget sParaTgtIns;
	int  nListLen;

	char** pszList = NULL;
	s8*	pTmpList[CBX_MAX];

	if(NULL==pList)
	{
		BIZ_DATA_DBG("GetScreenStrSelSZLIST null list!!!\n");
		return -1;
	}
	
	if(NULL==pList->psValueList)
	{
		BIZ_DATA_DBG("GetScreenStrSelSZLIST null psValueList!!!\n");
		return -1;
	}
	
	if(pList->max>=CBX_MAX)
	{
		pList->max = CBX_MAX;
	}

	pList->nRealNum = 0;
	
	for(i=0; i<CBX_MAX; i++)
	{
		pTmpList[i] = pList->psValueList[i];
	}
	
	switch(nItemID)
	{
		case GSR_CONFIG_BASIC_SYSTEM_DEVICENAME       : 
		{
			char *szList[] = {
				"TL_9508",
				"PAL",
				"YES",
				"2011-06-05",
				"256",
				"1",
				"English",
				"TL-Scr1"
			};
			pszList = szList;
			nListLen = sizeof(szList)/sizeof(szList[0]);
		} 
		break; // GSR_CONFIG_BASIC_SYSTEM_BASE+0
		case GSR_CONFIG_BASIC_SYSTEM_DEVICEID       : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+1
		case GSR_CONFIG_BASIC_SYSTEM_VIDEOFORMAT      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+2
		case GSR_CONFIG_BASIC_SYSTEM_AUTHCHECK      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+3
		case GSR_CONFIG_BASIC_SYSTEM_TIMEDISPLAY      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+4
		case GSR_CONFIG_BASIC_SYSTEM_MAXNETUSERS      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+5
		case GSR_CONFIG_BASIC_SYSTEM_OUTPUT         : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+6
		case GSR_CONFIG_BASIC_SYSTEM_LANGUAGE       : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+7
		case GSR_CONFIG_BASIC_SYSTEM_SCRENNSAVER      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+8
		
		
		//date&time..
		
		case GSR_CONFIG_BASIC_DATETIME_DATEFORMAT         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+0
		case GSR_CONFIG_BASIC_DATETIME_TIMEFORMAT         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+1
		case GSR_CONFIG_BASIC_DATETIME_TIMEZONE         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+2
		case GSR_CONFIG_BASIC_DATETIME_NETSYN         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+3
		case GSR_CONFIG_BASIC_DATETIME_NTPSERVER          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+4
		case GSR_CONFIG_BASIC_DATETIME_SYSDATE          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+5
		case GSR_CONFIG_BASIC_DATETIME_SYSTIME          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+6
		case GSR_CONFIG_BASIC_DATETIME_SYNCTIMETOIPC    : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+7
		
		
		//-------------------------------live
		
		//case GSR_CONFIG_LIVE_LIVE             : {;} break; // GSR_CONFIG_LIVE_BASE+0
		case GSR_CONFIG_LIVE_ALLCHN             : {;} break; // GSR_CONFIG_LIVE_BASE+1
		case GSR_CONFIG_LIVE_CHNNAME            : {;} break; // GSR_CONFIG_LIVE_BASE+2
		case GSR_CONFIG_LIVE_DISPLAY            : {;} break; // GSR_CONFIG_LIVE_BASE+3
		case GSR_CONFIG_LIVE_LIVEREC            : {;} break; // GSR_CONFIG_LIVE_BASE+4
		case GSR_CONFIG_LIVE_COLORSET             : {;} break; // GSR_CONFIG_LIVE_BASE+5
		
		//case GSR_CONFIG_LIVE_MAINMONITOR            : {;} break; // GSR_CONFIG_LIVE_BASE+0x40
		case GSR_CONFIG_LIVE_MAINMONITOR_SPLITMODE      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+0
		case GSR_CONFIG_LIVE_MAINMONITOR_CHN      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+1
		case GSR_CONFIG_LIVE_MAINMONITOR_DWELLTIME      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+2
		
		//case GSR_CONFIG_LIVE_SPOT             : {;} break; // GSR_CONFIG_LIVE_BASE+0x80
		case GSR_CONFIG_LIVE_SPOT_SPLITMODE             : {;} break; // GSR_CONFIG_LIVE_SPOT+0
		case GSR_CONFIG_LIVE_SPOT_CHN             : {;} break; // GSR_CONFIG_LIVE_SPOT+1
		case GSR_CONFIG_LIVE_SPOT_DWELLTIME             : {;} break; // GSR_CONFIG_LIVE_SPOT+2
		
		//case GSR_CONFIG_LIVE_MASK             : {;} break; // GSR_CONFIG_LIVE_BASE+0xc0
		case GSR_CONFIG_LIVE_MASK_AREA            : {;} break; // GSR_CONFIG_LIVE_MASK+0
		
		
		
		//-------------------------------record
		
		
		case GSR_CONFIG_RECORD_ENABLE_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_RECORD        : {;} break; // GSR_CONFIG_RECORD_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_AUDIO         : {;} break; // GSR_CONFIG_RECORD_BASE+2
		
		
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ALLCHN         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_RESOLUTION         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_FRAMERATE        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ENCODE         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+3
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_QUALITY        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+4
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_MAXRATE      : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+5
		
		
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_ALLCHN       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEBEFALARM   : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEAFTALARM       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_EXPIRATION       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+3
		
		
		case GSR_CONFIG_RECORD_ENABLE_OSD_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_OSD_CHNNAME         : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_OSD_TIMESTAMP         : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_OSD_POSITION        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+3
		
		
		case GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_REC        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_BASE+0
		
		
		
		
		
		//-------------------------------schedule
		
		//case GSR_CONFIG_SCHEDULE_TIMEREC  : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0
		case GSR_CONFIG_SCHEDULE_TIMEREC_CHN  : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+0
		case GSR_CONFIG_SCHEDULE_TIMEREC_SCH  : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+1
		case GSR_CONFIG_SCHEDULE_TIMEREC_COPYTO   : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+2
		
		
		//case GSR_CONFIG_SCHEDULE_MOTIONALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x20
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+0
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH   : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+1
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+2
		
		//case GSR_CONFIG_SCHEDULE_SENSORALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x30
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+0
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_SCH   : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+1
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+2
		
		
		//-------------------------------alarm
		
		//case GSR_CONFIG_ALARM_SENSOR    : {;} break; // GSR_CONFIG_ALARM_BASE+0
		
		case GSR_CONFIG_ALARM_SENSOR_BASE_ALLCHN    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+0
		case GSR_CONFIG_ALARM_SENSOR_BASE_ENABLE    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+1
		case GSR_CONFIG_ALARM_SENSOR_BASE_TYPE    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+2
		case GSR_CONFIG_ALARM_SENSOR_BASE_NAME    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+3
		
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x20
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALLCHN     : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME    : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE: {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		
		
		
		//case GSR_CONFIG_ALARM_SENSOR_SCH    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x40
		case GSR_CONFIG_ALARM_SENSOR_SCH_CHN  : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+0
		case GSR_CONFIG_ALARM_SENSOR_SCH_SCH  : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+1
		case GSR_CONFIG_ALARM_SENSOR_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+2
		
		///
		
		//case GSR_CONFIG_ALARM_MD            : {;} break; // GSR_CONFIG_ALARM_BASE+0x40
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT     : {;} break; // GSR_CONFIG_ALARM_MD+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALLCHN  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+2
		
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+3
		
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_TYPE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_INDEX : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+1
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET  : {;} break; // GSR_CONFIG_ALARM_MD+0x28
		
		
		//case GSR_CONFIG_ALARM_MD_SCH      : {;} break; // GSR_CONFIG_ALARM_MD+0x30
		case GSR_CONFIG_ALARM_MD_SCH_SCH_CHN  : {;} break; // GSR_CONFIG_ALARM_MD_SCH+0
		case GSR_CONFIG_ALARM_MD_SCH_SCH_SCH  : {;} break; // GSR_CONFIG_ALARM_MD_SCH+1
		case GSR_CONFIG_ALARM_MD_SCH_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_MD_SCH+2
		
		
		//case GSR_CONFIG_ALARM_VIDEOLOST     : {;} break; // GSR_CONFIG_ALARM_BASE+0x80
		
		//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST+0
		
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_TYPE  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_INDEX : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+1
		
		
		//case GSR_CONFIG_ALARM_OTHER           : {;} break; // GSR_CONFIG_ALARM_BASE+0xA0
		
		case GSR_CONFIG_ALARM_OTHER_ALARMTYPE     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+1
		case GSR_CONFIG_ALARM_OTHER_SOUNDALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+2
		case GSR_CONFIG_ALARM_OTHER_MAIL        : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+3
		case GSR_CONFIG_ALARM_OTHER_TRIGGERALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+4
		case GSR_CONFIG_ALARM_OTHER_DISKALARM     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+5
		case GSR_CONFIG_ALARM_OTHER_ALLCHN      : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+6
		
		
		case GSR_CONFIG_ALARM_ALARMOUT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xB0
		
		
		case GSR_CONFIG_ALARM_EXIT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xC0
		
		
		//-------------------------------network
		
		case GSR_CONFIG_NETWORK_BASE    : 
		{
		} break; // GSR_CONFIG_BASE+0x500
		
		
		//-------------------------------user
		
		case GSR_CONFIG_USER_BASE       : {;} break; // GSR_CONFIG_BASE+0x600
		
		
		//-------------------------------ptz
		
		
		
		case GSR_PTZCONFIG_SERIAL_ENABLE    : {;} break; // GSR_PTZCONFIG_BASE+1
		case GSR_PTZCONFIG_SERIAL_ADDRESS     : {;} break; // GSR_PTZCONFIG_BASE+2
		case GSR_PTZCONFIG_SERIAL_BPS     : {;} break; // GSR_PTZCONFIG_BASE+3
		case GSR_PTZCONFIG_SERIAL_PROTOCOL    : {;} break; // GSR_PTZCONFIG_BASE+4
		case GSR_PTZCONFIG_SERIAL_ALLCHECK    : {;} break; // GSR_PTZCONFIG_BASE+5
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_BASE
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ID    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+2
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_ID  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_NAME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+2
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_PRESET: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+3
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_RATE  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+4
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_TIME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+5
		
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_TRACK_BASE
		
		
		
		//-------------------------------advance
		//case GSR_CONFIG_ADVANCE     : {;} break; // GSR_CONFIG_BASE+0x800
		
		
		case GSR_ADVANCECONFIG_IMPORTEXPORT   : {;} break; // GSR_ADVANCECONFIG_BASE+0
		case GSR_ADVANCECONFIG_IMPORTEXPORT_REFRESH   : {;} break; // GSR_ADVANCECONFIG_IMPORTEXPORT+1
		
		
		// === search ===
		
		// === backup ===
		
		// === info ===
		
		//-------------------------------system
		case GSR_INFO_SYSTEM_BASE :	
		{
			char version[64];
			char model[64];
			char time_arr[20];
			
			GetDvrVersionModel(0,version,model);
			
			sprintf(time_arr, "%s", __DATE__);
			
			#if 1
			int nSzIdx = 0;
			SBizParaTarget bizTar;
			bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
			SBizSystemPara bizSysPara;
			int ret = BizGetPara(&bizTar, &bizSysPara);
			if(ret==0)
			{
                char s[50];
                int nRealLen = strlen(bizSysPara.strDevName);
                if(nRealLen>31) nRealLen = 31;
				strncpy(pTmpList[nSzIdx], bizSysPara.strDevName, nRealLen); nSzIdx++;
				sprintf(pTmpList[nSzIdx], "%d", bizSysPara.nDevId); nSzIdx++;
				strcpy(pTmpList[nSzIdx], model); nSzIdx++;
				
				//csp modify
				//strcpy(pTmpList[nSzIdx], "2.0"); nSzIdx++;
				//strcpy(pTmpList[nSzIdx], "2.0"); nSzIdx++;
				strcpy(pTmpList[nSzIdx], "1.0"); nSzIdx++;
				strcpy(pTmpList[nSzIdx], "1.0"); nSzIdx++;
				
				#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
				strcpy(pTmpList[nSzIdx], "3.0.8"); nSzIdx++;
				#else
				strcpy(pTmpList[nSzIdx], "2.6.24"); nSzIdx++;
				#endif
				
				#ifdef RELEASE_VERSION
                strcpy(pTmpList[nSzIdx], version); nSzIdx++;
				sprintf(s, "%s", time_arr);
                #else//鉴于debug版本不带加密，统一作此修改
                sprintf(pTmpList[nSzIdx], "%s %s", version,"TL demo version"); nSzIdx++;
				sprintf(s, "%s %s", time_arr,GetParsedString("&CfgPtn.TLDemo"));
                #endif
                strcpy(pTmpList[nSzIdx], s/*time_arr*/); nSzIdx++;
				pszList = pTmpList;
				nListLen = nSzIdx;
			}
			else
				pszList = NULL;
			#else
			char *szList[] = {
				"TL-DVR",
				"123456",
				"V1.0",
				"V2.0.5.1",
				"2.6.4",
				version,
				time_arr
			};
			pszList = szList;
			nListLen = sizeof(szList)/sizeof(szList[0]);
			#endif
		}	
		break;
		case GSR_INFO_SYSTEM_DEVICENAME   : {;} break; // GSR_INFO_SYSTEM_BASE+1
		case GSR_INFO_SYSTEM_DEVICENUM  : {;} break; // GSR_INFO_SYSTEM_BASE+2
		case GSR_INFO_SYSTEM_HARDWAREVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+3
		case GSR_INFO_SYSTEM_SCMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+4
		case GSR_INFO_SYSTEM_KERNELVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+5
		case GSR_INFO_SYSTEM_ROMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+6
		case GSR_INFO_SYSTEM_RELEASEDATE  : {;} break; // GSR_INFO_SYSTEM_BASE+7
		
		case GSR_DISK_BASE:
		{		
			u8 nMaxDiskNum = 64;
			SBizStoreDevManage sStoreDevMgr;
			SBizDevInfo sBizDevList[nMaxDiskNum]; //zlb20111117 原变量是malloc方式

			sStoreDevMgr.nDiskNum = 0;
			sStoreDevMgr.psDevList = sBizDevList;

			if(sStoreDevMgr.psDevList)
			{
				memset(sStoreDevMgr.psDevList, 0, sizeof(SBizDevInfo)*nMaxDiskNum);
				printf("BizSysComplexDMGetInfo\n");
				BizSysComplexDMGetInfo(&sStoreDevMgr, nMaxDiskNum);

				printf("BizSysComplexDMGetInfo finish\n");
				
				u8 nDiskNum = sStoreDevMgr.nDiskNum;

				if(nCh < nMaxDiskNum)
				{
					BIZ_DO_DBG("GetScreenStrSelSZLIST nCh %d\n",nCh);
					
					SBizDevInfo* pList = &sStoreDevMgr.psDevList[nCh];
					
					char outinfo[32] = {0};
					char outinfo1[32] = {0};
					char tmp[8] = {0};

					u8 nSzIdx = 0;
					
					sprintf(pTmpList[nSzIdx], "%d", nCh); 		nSzIdx++;
					strcpy( pTmpList[nSzIdx], (pList->nType==0?"HDD":"USB") ); 	nSzIdx++;
					sprintf(outinfo, "%ld", pList->nTotal/MEGA);
					sprintf(tmp, ".%02ldG", (pList->nTotal%MEGA)/TENKB);
					strcat(outinfo,tmp);	
					strcpy(pTmpList[nSzIdx], outinfo); nSzIdx++;
					
					sprintf(outinfo1, "%ld", pList->nFree/MEGA);
					sprintf(tmp, ".%02ldG", (pList->nFree%MEGA)/TENKB);
					strcat(outinfo1,tmp);	
					strcpy(pTmpList[nSzIdx], outinfo1); nSzIdx++;
					
					strcpy( pTmpList[nSzIdx], 
						(pList[i].nStatus==2?"ERR":((pList->nStatus==0)?"OK":"RAW")) ); 	nSzIdx++;
					strcpy(pTmpList[nSzIdx], "R/W"); nSzIdx++;
					strcpy(pTmpList[nSzIdx], pList->strDevPath); nSzIdx++;
					strcpy(pTmpList[nSzIdx], ""); nSzIdx++;
					strcpy(pTmpList[nSzIdx], ""); nSzIdx++;
					strcpy(pTmpList[nSzIdx], ""); nSzIdx++;
					strcpy(pTmpList[nSzIdx], outinfo); nSzIdx++;
					strcpy(pTmpList[nSzIdx], outinfo1); nSzIdx++;

					pszList = (char**)pTmpList;
					nListLen = 12;					
				}
				else
				{
					pszList = NULL;
				}
			}
		}	break;
		case GSR_INFO_DISK_NO   : {;} break; // GSR_INFO_DISK_BASE+1
		case GSR_INFO_DISK_TYPE   : {;} break; // GSR_INFO_DISK_BASE+2
		case GSR_INFO_DISK_CAPACITY   : {;} break; // GSR_INFO_DISK_BASE+3
		case GSR_INFO_DISK_FREE   : {;} break; // GSR_INFO_DISK_BASE+4
		case GSR_INFO_DISK_STATE  : {;} break; // GSR_INFO_DISK_BASE+5
		case GSR_INFO_DISK_ATTRIBUTE  : {;} break; // GSR_INFO_DISK_BASE+6
		case GSR_INFO_DISK_SOURCE   : {;} break; // GSR_INFO_DISK_BASE+7
		case GSR_INFO_DISK_RSV1   : {;} break; // GSR_INFO_DISK_BASE+8
		case GSR_INFO_DISK_RSV2   : {;} break; // GSR_INFO_DISK_BASE+9
		case GSR_INFO_DISK_IMAGE  : {;} break; // GSR_INFO_DISK_BASE+10
		case GSR_INFO_DISK_TOTAL  : {;} break; // GSR_INFO_DISK_BASE+11
		case GSR_INFO_DISK_USED   : {;} break; // GSR_INFO_DISK_BASE+12
		
		
		//-------------------------------event
		
		
		//-------------------------------syslog
		
		
		//------------------------------- netinfo
		case GSR_INFO_NETINFO_BASE:
		{
			SBizNetPara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			sParaTgtIns.emBizParaType = EM_BIZ_NETPARA;
			
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				u8 nSzIdx = 0;
				
				sprintf(pTmpList[nSzIdx], "%d", sPIns.HttpPort); 	nSzIdx++;
				sprintf(pTmpList[nSzIdx], "%d", sPIns.TCPPort); 	nSzIdx++;
                sprintf(pTmpList[nSzIdx], "%d", sPIns.MobilePort); 	nSzIdx++; //add by Lirl on Nov/03/2011
				
				// read net para
				SBizNetPara sPara;
				
				if( 0 == BizNetReadCfgEx( &sPara ) )
				{
					sprintf(pTmpList[nSzIdx], "%s", bizData_inet_ntoa(sPara.HostIP)); 	nSzIdx++;
					sprintf(pTmpList[nSzIdx], "%s", bizData_inet_ntoa(sPara.Submask)); 	nSzIdx++;
					sprintf(pTmpList[nSzIdx], "%s", bizData_inet_ntoa(sPara.GateWayIP)); 	nSzIdx++;
					sprintf(pTmpList[nSzIdx], "%s", bizData_inet_ntoa(sPara.DNSIP)); 	nSzIdx++;
                    //去掉备用DNS显示,添加手机端口显示
					//sprintf(pTmpList[nSzIdx], "%s", bizData_inet_ntoa(sPara.DNSIPAlt)); 	nSzIdx++;
				}

				SBizNetConnStatus sStatus;

				if( 0 == BizNetReqConnStatus(&sStatus) )
				{
					switch(sStatus.eWay)
					{
						case BIZ_ECW_DHCP:
						{
							sprintf(pTmpList[nSzIdx], "%s", GetParsedString("&CfgPtn.DHCPC"));
						} break;
						case BIZ_ECW_STATIC:
						{
							sprintf(pTmpList[nSzIdx], "%s", GetParsedString("&CfgPtn.STATICC"));
						} break;
						case BIZ_ECW_PPPOE:
						{
							sprintf(pTmpList[nSzIdx], "%s", GetParsedString("&CfgPtn.PPPOE"));
						} break;
					}
					
					nSzIdx++;

					switch(sStatus.eState)
					{
						case BIZ_ECS_ONLINE:
						{
							sprintf(pTmpList[nSzIdx], "%s", GetParsedString("&CfgPtn.Online"));
						} break;
						case BIZ_ECS_OFFLINE:
						{
							sprintf(pTmpList[nSzIdx], "%s", GetParsedString("&CfgPtn.Offline"));
						} break;
					}
					
					nSzIdx++;
				}
				else
					nSzIdx += 2;
				
				strcpy(pTmpList[nSzIdx], (char*)sPIns.DDNSDomain); 	nSzIdx++;
				strcpy(pTmpList[nSzIdx], (char*)sPIns.MAC); 	nSzIdx++;
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_CHNNAME err\n");
			}
			
			pszList = (char**)pTmpList;
			nListLen = 11;	
			
			#if 0
			char *szList[] = {
				"8670",
				"192.169.1.1",
				"192.168.1.63",
				"255.255.255.0",
				"192.168.1.1",
				"202.111.111.111",
				"202.111.111.112",
				"TCP",
				"ON",
				"TL-DDNS",
				"FF:EE:DD:CC:BB:AA"
			};		
					
			pszList = szList;
			nListLen = sizeof(szList)/sizeof(szList[0]);
			#endif
		}
		break;
		case GSR_CONFIG_NETWORK_IP:
		{
			;
		}
		break;
		
		//------------------------------- online
		
		case GSR_INFO_ONLINE_USERNAME : {;} break; // GSR_INFO_ONLINE_BASE+1
		case GSR_INFO_ONLINE_IP     : {;} break; // GSR_INFO_ONLINE_BASE+2
		case GSR_INFO_ONLINE_STATE  : {;} break; // GSR_INFO_ONLINE_BASE+3
		
		// exit
		
		
		
		// === alarm ===
		
		
		case GSR_MANUALALARM_ALLCHECK : {;} break; // GSR_MANUALALARM_BASE+1
		case GSR_MANUALALARM_ALARMNAME: {;} break; // GSR_MANUALALARM_BASE+2
		case GSR_MANUALALARM_IP     : {;} break; // GSR_MANUALALARM_BASE+3
		case GSR_MANUALALARM_ALARMPROCESS : {;} break; // GSR_MANUALALARM_BASE+4
		
		// === disk ===
		
		
		
		// === resource ===
		
		
		
		// === logoff ===
		
		
		
		// === shutdown ===
		
		
		// === desktop ===
		
		case GSR_DESKTOP_SYSTIME  : 
		{;} break; // GSR_DESKTOP_BASE+1
		case GSR_DESKTOP_CHNNAME  : {;} break; // GSR_DESKTOP_BASE+2
		
		//case GSR_DESKTOP_VIDEO_ATTRIBUTE  : {;} break; // GSR_DESKTOP_BASE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+0
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+1
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+2
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CONTRAST   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+4
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_DEFAULT  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+5
	}
	
	if(!pszList)
	{
		return -1;
	}
	
	//printf("GetScreenStrSelSZLIST!!! max %d, listlen %d\n", pList->max, nListLen);
	
	if(nListLen>pList->max)
		pList->nRealNum = pList->max;
	else
		pList->nRealNum = nListLen;
	
	for(int i=0; i<pList->nRealNum; i++)
	{
		strcpy(pList->psValueList[i], pszList[i]);
	}
	
	return 0;
}

int GetScreenStrSel( u32 nItemID, SGetScr* psGsr )
{	
	int rtn = 0;
	
	if(psGsr)
	{
		EM_GSR type = psGsr->type;
		u8	chn = psGsr->chn;
		
		switch(type)
		{
			case EM_GSR_OSD:
			{
				GetScreenStrSelOSD(nItemID, chn, psGsr->szValue);
			}
			break;
			case EM_GSR_CHKORIDX:
			{				
				psGsr->value = GetScreenStrSelCHKORIDX(nItemID, chn);
			}
			break;
			case EM_GSR_COMBLIST:
			{
				GetScreenStrSelCOMBLIST(nItemID, &psGsr->sCombList);
			}
			break;
			case EM_GSR_SCH:
			{
				GetScreenStrSelSCH(nItemID, chn, &psGsr->sSCH.sch[0] );
			}
			break;
			case EM_GSR_EDIT:
			{
				GetScreenStrSelEDIT(nItemID, chn, psGsr->szValue);
			}
			break;
			case EM_GSR_SZLIST:
			{
				rtn = GetScreenStrSelSZLIST(nItemID, chn, &psGsr->sCombList);
			}
			break;
			default:
				break;
		}
	}

	return rtn;
}

int GetScrStrSelDo(
	u32 	nItemID, 
	EM_GSR 	type, 
	u8 		ch,
	u32 	max,
	s8* psValueList[],
	SGetScr* pGsr
)
{
	//sGsr.type = EM_GSR_CHKORIDX;
	pGsr->type = type;
	//sGsr.type = EM_GSR_SCH;
	pGsr->chn = ch;
	//sGsr.chn = ALL_SEL;
	pGsr->sCombList.max = max;
	
	//if(psValueList)
	//	pGsr->sCombList.psValueList = psValueList;
	
	return GetScreenStrSel( nItemID, pGsr );
}

void debug_trace_err(char* str)
{
	//return;
	
	//csp modify 20130102
	static int i = 0;
	printf("myapp DEBUG %s %d \n", str, i++);
}

u32 GetLocalTimeForBackup()
{
	return (u32)time(NULL);
}

u32 MakeTimeForBackup(int y, int m, int d, int h, int min, int s)
{
	struct tm tm_time;	
	
	memset(&tm_time,0,sizeof(tm_time));
	
	tm_time.tm_year = y - 1900;
	tm_time.tm_mon = m - 1;
	tm_time.tm_mday = d;
	tm_time.tm_hour = h;
	tm_time.tm_min = min;
	tm_time.tm_sec = s;
	
	printf("%d %d %d %d %d %d\n", 
		tm_time.tm_year,
		tm_time.tm_mon,
		tm_time.tm_mday,
		tm_time.tm_hour,
		tm_time.tm_min,
		tm_time.tm_sec);
	
	//csp modify 20131213
	//return mktime(&tm_time);
	u32 t = mktime(&tm_time);
	
	//csp modify 20131213
	int nTimeZone = GetTimeZone();
	t -= GetTimeZoneOffset(nTimeZone);
	return t;
}

void GetTimeForBackup( u32 t, char* szTime )//时间和日期
{
	// invalid until 2038-XXX
	//time_t* tmpT = (time_t*)&t;
	
	time_t tmpT = t;
	
	//csp modify 20131213
	int nTimeZone = GetTimeZone();
	tmpT += GetTimeZoneOffset(nTimeZone);
	
	#if 0
	struct tm* ptm = gmtime(&tmpT);
	#else
	struct tm tmIns;
	struct tm* ptm = &tmIns;
	gmtime_r(&tmpT, &tmIns);
	#endif
	
	char tmp[20] = {0};
	
	if(szTime)
	{
		sprintf(tmp, "%04d-%02d-%02d %02d:%02d:%02d",
			ptm->tm_year+1900,// = y - 1900;
			ptm->tm_mon+1,// = m - 1;
			ptm->tm_mday,// = d;
			ptm->tm_hour,// = h;
			ptm->tm_min,// = min;
			ptm->tm_sec// = s;
		);
		printf("%d,%d,%04d-%02d-%02d %02d:%02d:%02d, %s\n",t,tmpT,ptm->tm_year,ptm->tm_mon,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,tmp);
		strcpy(szTime, tmp);
	}
}

void GetTimeForBackup2( u32 t, char* szTime )//只有时间
{
	time_t tmpT = t;
	
	//csp modify 20131213
	int nTimeZone = GetTimeZone();
	tmpT += GetTimeZoneOffset(nTimeZone);
	
	#if 0
	struct tm* ptm = gmtime(&tmpT);
	#else
	struct tm tmIns;
	struct tm* ptm = &tmIns;
	gmtime_r(&tmpT, &tmIns);
	#endif
	
	char tmp[20] = {0};
	
	if(szTime)
	{
		sprintf(tmp, "%02d:%02d:%02d",
			ptm->tm_hour,// = h;
			ptm->tm_min,// = min;
			ptm->tm_sec// = s;
		);
		//printf("snap file time,%04d-%02d-%02d %02d:%02d:%02d, %s\n",ptm->tm_year,ptm->tm_mon,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,tmp);
		strcpy(szTime, tmp);
	}
}

int GetScrStrInitComboxSelDo(
	void* pCtrl,
	u32 	nItemID, 
	EM_GSR 	type, 
	u8 		ch
)
{
	CCheckBox* pCheckBox;
	CComboBox* pCombBox;
	uint nRealNum = 0;
	
	u32 max = CBX_MAX;
	s8**	psValueList;
	SGetScr sGsr;
	memset(&sGsr, 0, sizeof(sGsr));
	SGetScr* pGsr = &sGsr;
	//pGsr->sCombList.psValueList = psValueList;

	//psValueList = (s8**)pGsr->sCombList.psValueList;
	switch(type)
	{
		case EM_GSR_COMBLIST:
		{
			//BIZ_DATA_DBG("EM_GSR_COMBLIST 1");
			pCombBox = (CComboBox*)pCtrl;
			
			GetScrStrSelDo(
				nItemID, 
				EM_GSR_COMBLIST, 
				0,
				max,
				NULL,
				pGsr
			);
			nRealNum = pGsr->sCombList.nRealNum;
			
			for(int j=0;j<nRealNum;j++)
			{
				pCombBox->AddString(pGsr->sCombList.psValueList[j]);
			}
			pCombBox->SetCurSel(0);
			//BIZ_DATA_DBG("EM_GSR_COMBLIST 2");
			
			#if 0
			GetScrStrSelDo(
				nItemID, 
				EM_GSR_CHKORIDX, 
				ch,
				0,
				NULL,
				pGsr
			);
			
			pCombBox->SetCurSel(pGsr->value);	
			#endif
			
			//BIZ_DATA_DBG("EM_GSR_COMBLIST 3");		
		}
		break;
		default:
			break;	
	}
}

int GetScrStrSetCurSelDo(
	void* pCtrl,
	u32 	nItemID, 
	EM_GSR_CTRL 	type, 
	u8 		ch
)
{
	CCheckBox* pCheckBox;
	CComboBox* pCombBox;
	CButton* pButton;
	CStatic** pStatic;
	CSliderCtrl* pSlider;
	CGridCtrl** pGrid;
	CEdit* pEdit;
	CIPAddressCtrl** pIPCtrl;
	uint nRealNum;
	
	//u8**	psValueList;
	SGetScr sGsr;
	memset(&sGsr, 0, sizeof(sGsr));
	SGetScr* pGsr = &sGsr;
	
	int ret = 0;
	switch(type)
	{
		case EM_GSR_CTRL_SZLIST:
		{
			if(!GetScrStrSelDo(
				nItemID, 
				EM_GSR_SZLIST, 
				ch,
				CBX_MAX,
				NULL,
				pGsr ) )
			{
				nRealNum = pGsr->sCombList.nRealNum;
				for(int i=0; i<nRealNum; i++)
				{
					pStatic = (CStatic**)pCtrl;
					if(pStatic)
					{
						if(pStatic[i])
							pStatic[i]->SetText(pGsr->sCombList.psValueList[i]);
					}
				}
				
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		break;
		case EM_GSR_CTRL_SZIPLIST:
		{
			return GetScrStrSelDo(
				nItemID, 
				EM_GSR_SZLIST, 
				0,
				ch,
				NULL,
				pGsr
			);
			nRealNum = pGsr->sCombList.nRealNum;
			IPADDR sIpAdd;
			memset(&sIpAdd,0,sizeof(sIpAdd));
			
			for(int i=0; i<nRealNum; i++)
			{
				pIPCtrl = (CIPAddressCtrl**)pCtrl;
				if(pIPCtrl)
				{
					if(pIPCtrl[i])
					{
						// get ip part from ip str such as "192.168.1.128"
						// 
						int k=0;int n=0;
						char szIpp[4]={0};
						for( int j=0; j<strlen(pGsr->sCombList.psValueList[i]); j++ )
						{
							if(pGsr->sCombList.psValueList[i][j]=='.'||
								pGsr->sCombList.psValueList[i][j]==' '||
								pGsr->sCombList.psValueList[i][j]==0 ||
								pGsr->sCombList.psValueList[i][j]=='\0' )
							{
								szIpp[k]	= '\0';
								sIpAdd.c[n]	= 0;
								for( int m=0;m<k;m++)
								{
									sIpAdd.c[n] = 10*sIpAdd.c[n]+szIpp[m]-'0';
								}
								if(n>=4)
									break;
								k=0; n++;
							}
							else
							{
								szIpp[k++]=pGsr->sCombList.psValueList[i][j];
							}
						}
						pIPCtrl[i]->SetIPAddress(&sIpAdd);
					}			
				}
			}
		}
		break;
		case EM_GSR_CTRL_EDIT:
		{
			pEdit = (CEdit*)pCtrl;
			char szStr[EDIT_MAX];

			pGsr->szValue = szStr;
			ret =  GetScrStrSelDo(
				nItemID, 
				EM_GSR_EDIT, 
				ch,
				0,
				NULL,
				pGsr
			);
			
			pEdit->SetText(pGsr->szValue);			
		}
		break;	
		case EM_GSR_CTRL_BTN:
		{
			pButton = (CButton*)pCtrl;

			ret =  GetScrStrSelDo(
				nItemID, 
				EM_GSR_CHKORIDX, 
				ch,
				0,
				NULL,
				pGsr
			);
			
			pButton->Enable(pGsr->value);			
		}
		break;
		case EM_GSR_CTRL_COMB:
		{
			pCombBox = (CComboBox*)pCtrl;

			ret =  GetScrStrSelDo(
				nItemID, 
				EM_GSR_CHKORIDX, 
				ch,
				0,
				NULL,
				pGsr
			);
			
			pCombBox->SetCurSel(pGsr->value);
			
		}
		break;
		case EM_GSR_CTRL_CHK:
		{
			pCheckBox = (CCheckBox*)pCtrl;

			ret = GetScrStrSelDo(
				nItemID, 
				EM_GSR_CHKORIDX, 
				ch,
				0,
				NULL,
				pGsr
			);
			
			pCheckBox->SetValue(pGsr->value);
		}
		break;
		case EM_GSR_CTRL_SLIDE:
		{	
#if 1		
			pSlider = (CSliderCtrl*)pCtrl;

			ret = GetScrStrSelDo(
				nItemID, 
				EM_GSR_CHKORIDX, 
				ch,
				0,
				NULL,
				pGsr
			);
			
			if(pGsr->value>255)
				pGsr->value = 255;
			
			pSlider->SetPos(pGsr->value);
#endif
		}
		break;
		case EM_GSR_CTRL_SCH:
		{			
			pGrid = (CGridCtrl**)pCtrl;

			ret = GetScrStrSelDo(
				nItemID, 
				EM_GSR_SCH, 
				ch,
				0,
				NULL,
				pGsr
			);

			//printf("EM_GSR_CTRL_SCH \n");
			
			for(int i=0; i<7; i++)
			{
			//printf("EM_GSR_CTRL_SCH2 %d\n", i);
				pGrid[i]->SetMask(&pGsr->sSCH.sch[i]);
			}
			//printf("EM_GSR_CTRL_SCH1 \n");
		}
		break;
		default:
			return -1;
			//break;	
	}

	return ret;
	
}

/*
	save to config
*/

int SaveScrStr(u32 nItemID, u8 ch, char* pStr)
{
	int rtn = 0;//int rtn;//csp modify
	s32 nValue;
	
	SBizParaTarget sParaTgtIns;
	
	/*
	if(pStr)
		BIZ_DT_DBG("ID:%d CH:%d VAL:%s\n", nItemID, ch, pStr);
	else
		BIZ_DT_DBG("ID:%d CH:%d VAL NULL!!!\n", nItemID, ch);
	*/
	
	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));
	
	switch(nItemID)
	{
		case GSR_CONFIG_ALARM_ALARMOUT_NAME:
		{
			SBizAlarmOutPara bizSysPara;
			
			sParaTgtIns.emBizParaType = EM_BIZ_ALARMOUTPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			
			if(0==rtn)
			{
				strcpy(bizSysPara.name, pStr);
				
				rtn = BizSetPara(&sParaTgtIns, &bizSysPara);
				
				printf("line:%d rtn:%d \n",__LINE__ ,rtn);
				
				return rtn;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_SYSTEM_DEVICENAME set err\n");
				
				return -1;
			}
		}
		break;
		case GSR_CONFIG_BASIC_SYSTEM_DEVICENAME       : 
		{
			SBizSystemPara bizSysPara;
			
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			//sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			
			if(0==rtn)
			{
				strcpy(bizSysPara.strDevName, pStr);
				//strcpy(devID, pStr);
				rtn = BizSetPara(&sParaTgtIns, &bizSysPara);
				//printf("line:%d rtn:%d \n",__LINE__ ,rtn);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_SYSTEM_DEVICENAME set err\n");
			}
		} 
		break;
		break; // GSR_CONFIG_BASIC_SYSTEM_BASE+0
		case GSR_CONFIG_BASIC_SYSTEM_DEVICEID       :
		{
			SBizSystemPara bizSysPara;
			
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			//sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{
				bizSysPara.nDevId = strtol(pStr, 0, 10);//cw_panel
				sprintf(devID, "%d", bizSysPara.nDevId);
				#if 0
				char buf[2]= {0};//cw_panel
				buf[0] = (1 << 7);
				buf[1] = bizSysPara.nDevId;
				FrontboardWrite(buf ,2);
				printf("write board buf = (%02x %02x)\n",buf[0],buf[1]);
				BIZ_DO_DBG("GSR_CONFIG_BASIC_SYSTEM_DEVICEID %d\n", bizSysPara.nDevId);
				#else
				FrontBoardSetDvrId((u32)bizSysPara.nDevId);
				#endif
				BizSetPara(&sParaTgtIns, &bizSysPara);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_SYSTEM_DEVICEID set err\n");
			}
		}
		break;
		case GSR_CONFIG_BASIC_SYSTEM_VIDEOFORMAT      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+2
		case GSR_CONFIG_BASIC_SYSTEM_AUTHCHECK      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+3
		case GSR_CONFIG_BASIC_SYSTEM_TIMEDISPLAY      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+4
		case GSR_CONFIG_BASIC_SYSTEM_MAXNETUSERS      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+5
		case GSR_CONFIG_BASIC_SYSTEM_OUTPUT         : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+6
		case GSR_CONFIG_BASIC_SYSTEM_LANGUAGE       : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+7
		case GSR_CONFIG_BASIC_SYSTEM_SCRENNSAVER      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+8
		
		//date&time..
		
		case GSR_CONFIG_BASIC_DATETIME_DATEFORMAT         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+0
		case GSR_CONFIG_BASIC_DATETIME_TIMEFORMAT         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+1
		case GSR_CONFIG_BASIC_DATETIME_TIMEZONE         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+2
		case GSR_CONFIG_BASIC_DATETIME_NETSYN         : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+3
		case GSR_CONFIG_BASIC_DATETIME_NTPSERVER          : //QYJ
		{
			SBizSystemPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			sParaTgtIns.nChn = 0;
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{
				printf("strNtpServerAdress = %s\n",pStr);
				strcpy(bizSysPara.strNtpServerAdress, pStr);
				rtn = BizSetPara(&sParaTgtIns, &bizSysPara);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_SYSTEM_DEVICENAME set err\n");
			}
		} break; // GSR_CONFIG_BASIC_DATETIME_BASE+4
		case GSR_CONFIG_BASIC_DATETIME_SYSDATE          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+5
		case GSR_CONFIG_BASIC_DATETIME_SYSTIME          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+6
		case GSR_CONFIG_BASIC_DATETIME_SYNCTIMETOIPC    : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+7
		
		
		//-------------------------------live
		
		//case GSR_CONFIG_LIVE_LIVE             : {;} break; // GSR_CONFIG_LIVE_BASE+0
		case GSR_CONFIG_LIVE_ALLCHN             : {;} break; // GSR_CONFIG_LIVE_BASE+1
		case GSR_CONFIG_LIVE_CHNNAME            : 
		{
			//printf("yg SaveScrStr GSR_CONFIG_LIVE_CHNNAME chn%d\n", ch);
			SBizCfgStrOsd bizStrOsd;
			sParaTgtIns.emBizParaType = EM_BIZ_STROSD;
			sParaTgtIns.nChn = ch;
			//printf("%s get osd\n", __func__);
			rtn = BizGetPara(&sParaTgtIns, &bizStrOsd);
			if(0==rtn)
			{
				//printf("%s GSR_CONFIG_LIVE_CHNNAME & ALLCHN chn %d, osd name get: %s, set: %s\n",
				//	__func__, ch, bizStrOsd.strChnName, pStr);
				
				strcpy(bizStrOsd.strChnName, pStr);
				
				BizSetPara(&sParaTgtIns, &bizStrOsd);
			}
			else
			{
				printf("GSR_CONFIG_LIVE_CHNNAME set err\n");
				
				//BIZ_DATA_DBG("GSR_CONFIG_LIVE_CHNNAME set err\n");
			}
		} break; // GSR_CONFIG_LIVE_BASE+2
		case GSR_CONFIG_LIVE_DISPLAY            : {;} break; // GSR_CONFIG_LIVE_BASE+3
		case GSR_CONFIG_LIVE_LIVEREC            : {;} break; // GSR_CONFIG_LIVE_BASE+4
		case GSR_CONFIG_LIVE_COLORSET             : {;} break; // GSR_CONFIG_LIVE_BASE+5
		
		//case GSR_CONFIG_LIVE_MAINMONITOR            : {;} break; // GSR_CONFIG_LIVE_BASE+0x40
		case GSR_CONFIG_LIVE_MAINMONITOR_SPLITMODE      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+0
		case GSR_CONFIG_LIVE_MAINMONITOR_CHN      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+1
		case GSR_CONFIG_LIVE_MAINMONITOR_DWELLTIME      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+2
		
		//case GSR_CONFIG_LIVE_SPOT             : {;} break; // GSR_CONFIG_LIVE_BASE+0x80
		case GSR_CONFIG_LIVE_SPOT_SPLITMODE             : {;} break; // GSR_CONFIG_LIVE_SPOT+0
		case GSR_CONFIG_LIVE_SPOT_CHN             : {;} break; // GSR_CONFIG_LIVE_SPOT+1
		case GSR_CONFIG_LIVE_SPOT_DWELLTIME             : {;} break; // GSR_CONFIG_LIVE_SPOT+2
		
		//case GSR_CONFIG_LIVE_MASK             : {;} break; // GSR_CONFIG_LIVE_BASE+0xc0
		case GSR_CONFIG_LIVE_MASK_AREA            : {;} break; // GSR_CONFIG_LIVE_MASK+0
		
		
		
		//-------------------------------record
		
		
		case GSR_CONFIG_RECORD_ENABLE_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_RECORD        : {;} break; // GSR_CONFIG_RECORD_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_AUDIO         : {;} break; // GSR_CONFIG_RECORD_BASE+2
		
		
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ALLCHN         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_RESOLUTION         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_FRAMERATE        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ENCODE         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+3
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_QUALITY        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+4
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_MAXRATE      : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+5
		
		
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_ALLCHN       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEBEFALARM   : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEAFTALARM       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_EXPIRATION       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+3
		
		
		case GSR_CONFIG_RECORD_ENABLE_OSD_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_OSD_CHNNAME         : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_OSD_TIMESTAMP         : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_OSD_POSITION        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+3
		
		
		case GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_REC        : {;} break; // GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_BASE+0
		
		
		
		
		
		//-------------------------------schedule
		
		//case GSR_CONFIG_SCHEDULE_TIMEREC  : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0
		case GSR_CONFIG_SCHEDULE_TIMEREC_CHN  : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+0
		case GSR_CONFIG_SCHEDULE_TIMEREC_SCH  : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+1
		case GSR_CONFIG_SCHEDULE_TIMEREC_COPYTO   : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+2
		
		
		//case GSR_CONFIG_SCHEDULE_MOTIONALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x20
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+0
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH   : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+1
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+2
		
		//case GSR_CONFIG_SCHEDULE_SENSORALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x30
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+0
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_SCH   : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+1
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+2
		
		
		//-------------------------------alarm
		
		//case GSR_CONFIG_ALARM_SENSOR    : {;} break; // GSR_CONFIG_ALARM_BASE+0
		
		case GSR_CONFIG_ALARM_SENSOR_BASE_ALLCHN    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+0
		case GSR_CONFIG_ALARM_SENSOR_BASE_ENABLE    : 
		{
		} break; // GSR_CONFIG_ALARM_SENSOR_BASE+1
		case GSR_CONFIG_ALARM_SENSOR_BASE_TYPE    : {;}break; // GSR_CONFIG_ALARM_SENSOR_BASE+2
		case GSR_CONFIG_ALARM_SENSOR_BASE_NAME    : 
		{
			SBizAlarmSensorPara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				strcpy(sPIns.name, pStr);
				BIZ_DT_DBG("GSR_CONFIG_ALARM_SENSOR_BASE_NAME set %s\n", sPIns.name);
				
				return BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_BASE_NAME set err\n");
				return -1;
			}
		}
		break; // GSR_CONFIG_ALARM_SENSOR_BASE+3		
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x20
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALLCHN     : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME    : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE: {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		
		
		
		//case GSR_CONFIG_ALARM_SENSOR_SCH    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x40
		case GSR_CONFIG_ALARM_SENSOR_SCH_CHN  : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+0
		case GSR_CONFIG_ALARM_SENSOR_SCH_SCH  : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+1
		case GSR_CONFIG_ALARM_SENSOR_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+2
		
		///
		
		//case GSR_CONFIG_ALARM_MD            : {;} break; // GSR_CONFIG_ALARM_BASE+0x40
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT     : {;} break; // GSR_CONFIG_ALARM_MD+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALLCHN  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+2
		
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+3
		
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_TYPE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_INDEX : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+1
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET  : {;} break; // GSR_CONFIG_ALARM_MD+0x28
		
		
		//case GSR_CONFIG_ALARM_MD_SCH      : {;} break; // GSR_CONFIG_ALARM_MD+0x30
		case GSR_CONFIG_ALARM_MD_SCH_SCH_CHN  : {;} break; // GSR_CONFIG_ALARM_MD_SCH+0
		case GSR_CONFIG_ALARM_MD_SCH_SCH_SCH  : {;} break; // GSR_CONFIG_ALARM_MD_SCH+1
		case GSR_CONFIG_ALARM_MD_SCH_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_MD_SCH+2
		
		
		//case GSR_CONFIG_ALARM_VIDEOLOST     : {;} break; // GSR_CONFIG_ALARM_BASE+0x80
		
		//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST+0
		
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_TYPE  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_INDEX : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+1
		
		
		//case GSR_CONFIG_ALARM_OTHER           : {;} break; // GSR_CONFIG_ALARM_BASE+0xA0
		
		case GSR_CONFIG_ALARM_OTHER_ALARMTYPE     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+1
		case GSR_CONFIG_ALARM_OTHER_SOUNDALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+2
		case GSR_CONFIG_ALARM_OTHER_MAIL        : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+3
		case GSR_CONFIG_ALARM_OTHER_TRIGGERALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+4
		case GSR_CONFIG_ALARM_OTHER_DISKALARM     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+5
		case GSR_CONFIG_ALARM_OTHER_ALLCHN      : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+6
		
		
		case GSR_CONFIG_ALARM_ALARMOUT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xB0
		
		
		case GSR_CONFIG_ALARM_EXIT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xC0
		
		
		//-------------------------------network
		
		case GSR_CONFIG_NETWORK_BASE    : 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = 0;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return 0;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME err\n");
			}
		} break; // GSR_CONFIG_BASE+0x500
		
		
		//-------------------------------user
		
		case GSR_CONFIG_USER_BASE       : {;} break; // GSR_CONFIG_BASE+0x600
		
		
		//-------------------------------ptz
		
		
		
		case GSR_PTZCONFIG_SERIAL_ENABLE    : {;} break; // GSR_PTZCONFIG_BASE+1

		case GSR_PTZCONFIG_SERIAL_ADDRESS     :		
		{
			SBizPtzPara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nCamAddr = strtol(pStr, 0, 10);
				//printf("sPIns.nCamAddr   =   %d  \n",sPIns.nCamAddr);
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_ENABLE set err\n");
			}
		} break; // GSR_PTZCONFIG_BASE+2

		case GSR_PTZCONFIG_SERIAL_BPS     : {;} break; // GSR_PTZCONFIG_BASE+3
		case GSR_PTZCONFIG_SERIAL_PROTOCOL    : {;} break; // GSR_PTZCONFIG_BASE+4
		case GSR_PTZCONFIG_SERIAL_ALLCHECK    : {;} break; // GSR_PTZCONFIG_BASE+5
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_BASE
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ID    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+2
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_ID  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_NAME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+2
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_PRESET: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+3
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_RATE  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+4
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_TIME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+5
		
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_TRACK_BASE
		
		
		
		//-------------------------------advance
		//case GSR_CONFIG_ADVANCE     : {;} break; // GSR_CONFIG_BASE+0x800
		
		
		case GSR_ADVANCECONFIG_IMPORTEXPORT   : {;} break; // GSR_ADVANCECONFIG_BASE+0
		case GSR_ADVANCECONFIG_IMPORTEXPORT_REFRESH   : {;} break; // GSR_ADVANCECONFIG_IMPORTEXPORT+1
		
		
		// === search ===
		
		// === backup ===
		
		// === info ===
		
		//-------------------------------system
		
		case GSR_INFO_SYSTEM_DEVICENAME   : {;} break; // GSR_INFO_SYSTEM_BASE+1
		case GSR_INFO_SYSTEM_DEVICENUM  : {;} break; // GSR_INFO_SYSTEM_BASE+2
		case GSR_INFO_SYSTEM_HARDWAREVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+3
		case GSR_INFO_SYSTEM_SCMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+4
		case GSR_INFO_SYSTEM_KERNELVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+5
		case GSR_INFO_SYSTEM_ROMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+6
		case GSR_INFO_SYSTEM_RELEASEDATE  : {;} break; // GSR_INFO_SYSTEM_BASE+7
		
		
		case GSR_INFO_DISK_NO   : {;} break; // GSR_INFO_DISK_BASE+1
		case GSR_INFO_DISK_TYPE   : {;} break; // GSR_INFO_DISK_BASE+2
		case GSR_INFO_DISK_CAPACITY   : {;} break; // GSR_INFO_DISK_BASE+3
		case GSR_INFO_DISK_FREE   : {;} break; // GSR_INFO_DISK_BASE+4
		case GSR_INFO_DISK_STATE  : {;} break; // GSR_INFO_DISK_BASE+5
		case GSR_INFO_DISK_ATTRIBUTE  : {;} break; // GSR_INFO_DISK_BASE+6
		case GSR_INFO_DISK_SOURCE   : {;} break; // GSR_INFO_DISK_BASE+7
		case GSR_INFO_DISK_RSV1   : {;} break; // GSR_INFO_DISK_BASE+8
		case GSR_INFO_DISK_RSV2   : {;} break; // GSR_INFO_DISK_BASE+9
		case GSR_INFO_DISK_IMAGE  : {;} break; // GSR_INFO_DISK_BASE+10
		case GSR_INFO_DISK_TOTAL  : {;} break; // GSR_INFO_DISK_BASE+11
		case GSR_INFO_DISK_USED   : {;} break; // GSR_INFO_DISK_BASE+12
		
		
		//-------------------------------event
		
		
		//-------------------------------syslog
		
		
		//------------------------------- netinfo
		case GSR_INFO_NETINFO_BASE:
		{;}	break;		
		
		//------------------------------- online
		
		case GSR_INFO_ONLINE_USERNAME : {;} break; // GSR_INFO_ONLINE_BASE+1
		case GSR_INFO_ONLINE_IP     : {;} break; // GSR_INFO_ONLINE_BASE+2
		case GSR_INFO_ONLINE_STATE  : {;} break; // GSR_INFO_ONLINE_BASE+3
		
		// exit
		
		
		
		// === alarm ===
		
		
		case GSR_MANUALALARM_ALLCHECK : {;} break; // GSR_MANUALALARM_BASE+1
		case GSR_MANUALALARM_ALARMNAME: {;} break; // GSR_MANUALALARM_BASE+2
		case GSR_MANUALALARM_IP     : {;} break; // GSR_MANUALALARM_BASE+3
		case GSR_MANUALALARM_ALARMPROCESS : {;} break; // GSR_MANUALALARM_BASE+4
		
		// === disk ===
		
		
		
		// === resource ===
		
		
		
		// === logoff ===
		
		
		
		// === shutdown ===
		
		
		// === desktop ===
		
		case GSR_DESKTOP_SYSTIME  : 
		{;} break; // GSR_DESKTOP_BASE+1
		case GSR_DESKTOP_CHNNAME  : {;} break; // GSR_DESKTOP_BASE+2
		
		//case GSR_DESKTOP_VIDEO_ATTRIBUTE  : {;} break; // GSR_DESKTOP_BASE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+0
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+1
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+2
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CONTRAST   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+4
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_DEFAULT  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+5
	}
	
	return rtn;//csp modify
}

int SaveScrInt(u32 nItemID, u8 ch, int val)
{
	int rtn = 0;
	s32 nValue;
	
	SBizParaTarget sParaTgtIns;
	
	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));
	
	BIZ_DT_DBG("ID:%d CH:%d VAL:%d\n", nItemID, ch, val);
	
	switch(nItemID)
	{
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET_SENSITY:
		{
			SBizAlarmVMotionPara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
			sParaTgtIns.nChn = ch;
			
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nSensitivity = val;
				
				BizSetPara(&sParaTgtIns, &sPIns);
			}
		} break;		
		case GSR_CONFIG_ALARM_ALARMOUT_DELAYTIME:
		{
			SBizAlarmOutPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_ALARMOUTPARA;
			
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_DELAY, 
						val, 
						&nValue )
				)
				{
					BIZ_DT_DBG("GSR_CONFIG_ALARM_ALARMOUT_DELAYTIME set %d %d\n", val, nValue);
					
					bizSysPara.nDelay = nValue;
					
					BizSetPara(&sParaTgtIns, &bizSysPara);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_ALARMOUT_DELAYTIME set err\n");
			}
		}break;
		case GSR_CONFIG_ALARM_ALARMOUT_SOUNDALARM_ALARM:
		{
			SBizAlarmBuzzPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_ALARMBUZZPARA;
			
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{
				bizSysPara.nEnable = val;
				
				return BizSetPara(&sParaTgtIns, &bizSysPara);

				//printf("BizConfigGetParaListValue index %d value %d\n", nValue, val);
				
				//BIZ_DATA_DBG("GSR_CONFIG_ALARM_ALARMOUT_SOUNDALARM_ALARM set ok\n");
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_ALARMOUT_SOUNDALARM_ALARM set err\n");
				return -1;
			}
		}break;
		case GSR_CONFIG_ALARM_ALARMOUT_SOUNDALARM_DELAY:
		{
			SBizAlarmBuzzPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_ALARMBUZZPARA;
			
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_DELAY, 
						val, 
						&nValue )
				)
				{
					bizSysPara.nDelay = nValue;
					
					BizSetPara(&sParaTgtIns, &bizSysPara);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_ALARMOUT_SOUNDALARM_DELAY set err\n");
			}
		}break;
		
		case GSR_CONFIG_BASIC_SYSTEM_DEVICENAME       : 
		{;
		} 
		break; // GSR_CONFIG_BASIC_SYSTEM_BASE+0
		case GSR_CONFIG_BASIC_SYSTEM_DEVICEID       : 
		break;		// GSR_CONFIG_BASIC_SYSTEM_BASE+1
		case GSR_CONFIG_BASIC_SYSTEM_VIDEOFORMAT      : 
		{
			SBizSystemPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_VIDEOSTANDARD, 
						val, 
						&nValue )
				)
				{
					bizSysPara.nVideoStandard = nValue;
					
					return BizSetPara(&sParaTgtIns, &bizSysPara);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_SYSTEM_VIDEOFORMAT set err\n");
			}



		} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+2
		case GSR_CONFIG_BASIC_SYSTEM_AUTHCHECK      : 
		{
			
			SBizSystemPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{		
				bizSysPara.nAuthCheck = val;
					
				return BizSetPara(&sParaTgtIns, &bizSysPara);			
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_SYSTEM_AUTHCHECK set err\n");
			}
		} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+3
		case GSR_CONFIG_BASIC_SYSTEM_TIMEDISPLAY      : 
		{
			SBizCfgPreview bizPreCfg;
			sParaTgtIns.emBizParaType = EM_BIZ_PREVIEWPARA;
			
			rtn = BizGetPara(&sParaTgtIns, &bizPreCfg);
			if(0==rtn)
			{	
				bizPreCfg.nShowTime = val;
				
				return BizSetPara(&sParaTgtIns, &bizPreCfg);			
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_SYSTEM_TIMEDISPLAY set err\n");
			}


		} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+4
		case GSR_CONFIG_BASIC_SYSTEM_MAXNETUSERS    : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+5
		case GSR_CONFIG_BASIC_SYSTEM_OUTPUT         : 
		{
			SBizSystemPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{
				if(0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_OUTPUT, 
						val, 
						&nValue)
				)
				{
					bizSysPara.nOutput = nValue;
					printf("SaveScrInt:nOutput=%d\n",nValue);
					BizSetPara(&sParaTgtIns, &bizSysPara);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_SYSTEM_OUTPUT set err\n");
			}
		} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+6
		case GSR_CONFIG_BASIC_SYSTEM_LANGUAGE       : 
		{
			SBizSystemPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_LANGSUP, 
						val, 
						&nValue )
				)
				{
					bizSysPara.nLangId = nValue;

					printf("GSR_CONFIG_BASIC_SYSTEM_LANGUAGE %d \n", nValue);
					
					BizSetPara(&sParaTgtIns, &bizSysPara);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_SYSTEM_LANGUAGE set err\n");
			}
		} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+7
		case GSR_CONFIG_BASIC_SYSTEM_SHOWGUIDE: 
		{
			SBizSystemPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{		
				bizSysPara.nShowGuide= val;
					
				return BizSetPara(&sParaTgtIns, &bizSysPara);			
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_SYSTEM_SHOWGUIDE set err\n");
			}
		} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+8
		case GSR_CONFIG_BASIC_SYSTEM_SCRENNSAVER      : {;} break; // GSR_CONFIG_BASIC_SYSTEM_BASE+9
		case GSR_CONFIG_BASIC_SYSTEM_SHOWCHNKBPS:
		{
			SBizCfgPreview bizPreCfg;
			sParaTgtIns.emBizParaType = EM_BIZ_PREVIEWPARA;
			
			rtn = BizGetPara(&sParaTgtIns, &bizPreCfg);
			if(0==rtn)
			{	
				//printf("yg SaveScrInt nShowChnKbps read: %d, write: %d\n", bizPreCfg.nShowChnKbps, val);
				bizPreCfg.nShowChnKbps = val;
				
				return BizSetPara(&sParaTgtIns, &bizPreCfg);			
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_SYSTEM_SHOWCHNKBPS set err\n");
			}
		} break;
		
		//date&time..
		
		case GSR_CONFIG_BASIC_DATETIME_DATEFORMAT         : 
		{
			
			SBizSystemPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_DATEFORMAT, 
						val, 
						&nValue )
				)
				{
					bizSysPara.nDateFormat = nValue;
					
					return BizSetPara(&sParaTgtIns, &bizSysPara);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_DATETIME_DATEFORMAT set err\n");
			}



		} break; // GSR_CONFIG_BASIC_DATETIME_BASE+0
		case GSR_CONFIG_BASIC_DATETIME_TIMEFORMAT         : 
		{
			SBizSystemPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_TIMESTANDARD, 
						val, 
						&nValue )
				)
				{
					bizSysPara.nTimeStandard = nValue;
					
					BizSetPara(&sParaTgtIns, &bizSysPara);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_DATETIME_TIMEFORMAT set err\n");
			}


		} break; // GSR_CONFIG_BASIC_DATETIME_BASE+1
		case GSR_CONFIG_BASIC_DATETIME_TIMEZONE         : //QYJ
		{
			SBizSystemPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			sParaTgtIns.nChn = 0;
			printf("GSR_CONFIG_BASIC_DATETIME_TIMEZONE val = %d\n",val);
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{
				//printf("rtn==0\n");
				bizSysPara.nTimeZone = val;
				return BizSetPara(&sParaTgtIns, &bizSysPara);
			}
			else
			{
				printf("GSR_CONFIG_BASIC_DATETIME_TIMEZONE set err\n");
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_DATETIME_TIMEZONE set err\n");
			}
		} break; // GSR_CONFIG_BASIC_DATETIME_BASE+2
		case GSR_CONFIG_BASIC_DATETIME_NETSYN         : //QYJ
		{
			SBizSystemPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			sParaTgtIns.nChn = 0;
			printf("GSR_CONFIG_BASIC_DATETIME_NETSYN val = %d\n",val);
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{
				//printf("rtn==0\n");
				bizSysPara.nSyscCheck = val;
				return BizSetPara(&sParaTgtIns, &bizSysPara);
			}
			else
			{
				printf("GSR_CONFIG_BASIC_DATETIME_NETSYN set err\n");
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_DATETIME_NETSYN set err\n");
			}
		} break; // GSR_CONFIG_BASIC_DATETIME_BASE+3
		case GSR_CONFIG_BASIC_DATETIME_NTPSERVER          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+4
		case GSR_CONFIG_BASIC_DATETIME_SYSDATE          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+5
		case GSR_CONFIG_BASIC_DATETIME_SYSTIME          : {;} break; // GSR_CONFIG_BASIC_DATETIME_BASE+6
		case GSR_CONFIG_BASIC_DATETIME_SYNCTIMETOIPC    : 
		{
			SBizSystemPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			sParaTgtIns.nChn = 0;
			printf("GSR_CONFIG_BASIC_DATETIME_SYNCTIMETOIPC val = %d\n",val);
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{
				//printf("rtn==0\n");
				bizSysPara.nSyncTimeToIPC = val;
				return BizSetPara(&sParaTgtIns, &bizSysPara);
			}
			else
			{
				printf("GSR_CONFIG_BASIC_DATETIME_SYNCTIMETOIPC set err\n");
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_DATETIME_SYNCTIMETOIPC set err\n");
			}
		}break; // GSR_CONFIG_BASIC_DATETIME_BASE+7
		
		
		//-------------------------------live
		
		//case GSR_CONFIG_LIVE_LIVE             : {;} break; // GSR_CONFIG_LIVE_BASE+0
		case GSR_CONFIG_LIVE_ALLCHN             : {;} break; // GSR_CONFIG_LIVE_BASE+1
		case GSR_CONFIG_LIVE_CHNNAME            : {;} break; // GSR_CONFIG_LIVE_BASE+2
		case GSR_CONFIG_LIVE_DISPLAY            : 
		{
			SBizCfgStrOsd bizStrOsd;
			sParaTgtIns.emBizParaType = EM_BIZ_STROSD;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizStrOsd);
			if(0==rtn)
			{
				bizStrOsd.nShowChnName = val;
				
				BizSetPara(&sParaTgtIns, &bizStrOsd);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_LIVE_DISPLAY set err\n");
			}
		} break; // GSR_CONFIG_LIVE_BASE+3
		case GSR_CONFIG_LIVE_LIVEREC            : 
		{
			SBizCfgPreview bizPreCfg;
			sParaTgtIns.emBizParaType = EM_BIZ_PREVIEWPARA;
			//sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizPreCfg);
			if(0==rtn)
			{
					//printf("GSR_CONFIG_LIVE_LIVEREC 2222\n");
					//printf("get EM_BIZ_PREVIEWPARA nMode: %d\n", bizPreCfg.nMode);
					//bizPreCfg.nMode = 16;
					bizPreCfg.nShowRecState = val;
					
					int ret =  BizSetPara(&sParaTgtIns, &bizPreCfg);
					
					//printf("GSR_CONFIG_LIVE_LIVEREC 3333\n");
					
					return ret;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_LIVE_LIVEREC set err\n");
			}
			
		} break; // GSR_CONFIG_LIVE_BASE+4
		case GSR_CONFIG_LIVE_COLORSET             : {;} break; // GSR_CONFIG_LIVE_BASE+5
		
		//case GSR_CONFIG_LIVE_MAINMONITOR            : {;} break; // GSR_CONFIG_LIVE_BASE+0x40
		case GSR_CONFIG_LIVE_MAINMONITOR_SPLITMODE      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+0
		case GSR_CONFIG_LIVE_MAINMONITOR_CHN      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+1
		case GSR_CONFIG_LIVE_MAINMONITOR_DWELLTIME      : {;} break; // GSR_CONFIG_LIVE_MAINMONITOR+2
		
		//case GSR_CONFIG_LIVE_SPOT             : {;} break; // GSR_CONFIG_LIVE_BASE+0x80
		case GSR_CONFIG_LIVE_SPOT_SPLITMODE             : {;} break; // GSR_CONFIG_LIVE_SPOT+0
		case GSR_CONFIG_LIVE_SPOT_CHN             : {;} break; // GSR_CONFIG_LIVE_SPOT+1
		case GSR_CONFIG_LIVE_SPOT_DWELLTIME             : {;} break; // GSR_CONFIG_LIVE_SPOT+2
		
		//case GSR_CONFIG_LIVE_MASK             : {;} break; // GSR_CONFIG_LIVE_BASE+0xc0
		case GSR_CONFIG_LIVE_MASK_AREA            : {;} break; // GSR_CONFIG_LIVE_MASK+0
		
		
		
		//-------------------------------record
		
		
		case GSR_CONFIG_RECORD_ENABLE_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_RECORD        : 
		{

			SBizRecPara bizRecPara;
			sParaTgtIns.emBizParaType = EM_BIZ_RECPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizRecPara);
			//printf("chn[%d], enable:%d, new:%d\n", ch, bizRecPara.nEnable, val);
			
			if(0==rtn)
			{
					//getchar();
					bizRecPara.nEnable = val;
					
					return BizSetPara(&sParaTgtIns, &bizRecPara);
			
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_RECORD_ENABLE_RECORD set err\n");

				return rtn;
			}


		} break; // GSR_CONFIG_RECORD_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_AUDIO         : 
		{
			SBizRecPara bizRecPara;
			sParaTgtIns.emBizParaType = EM_BIZ_RECPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizRecPara);
			if(0==rtn)
			{
		
					bizRecPara.nStreamType = val;
					
					BizSetPara(&sParaTgtIns, &bizRecPara);
			
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_RECORD_ENABLE_AUDIO set err\n");
			}




		} break; // GSR_CONFIG_RECORD_BASE+2
		
		
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ALLCHN         : {;} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_RESOLUTION         : 
		{
			SBizEncodePara bizEncPara;
			sParaTgtIns.emBizParaType = EM_BIZ_ENCMAINPARA;
			sParaTgtIns.nChn = ch;
			int start;
			int end;
			rtn = BizGetPara(&sParaTgtIns, &bizEncPara);
			if(0==rtn)
			{
				int Hnum = 0;//cw_9508S
				EMBIZCFGPARALIST emParaType;
				GetResoListH_NUM(&Hnum);
				if(ch < Hnum)
					emParaType = EM_BIZ_CFG_PARALIST_VMAINRESOL;
				else
					emParaType = EM_BIZ_CFG_PARALIST_VMAINRESOL2;
				
				if( 0==BizConfigGetParaListValue(
						emParaType, 
						val, 
						&nValue )
				)
				{
					bizEncPara.nVideoResolution = (EMBIZENCRESOLUTION)nValue;
					
					//start = GetTimeTick();
					int ret = BizSetPara(&sParaTgtIns, &bizEncPara);
					//end = GetTimeTick();
					//printf("end - start = %d %d\n",end - start, bizEncPara.nVideoResolution);
					
					//printf("chn%d nVideoResolution=%d\n",ch,bizEncPara.nVideoResolution);
					
					return ret;
				}
				else
				{
					printf("BizConfigGetParaListValue error\n");
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_BASIC_DATETIME_TIMEFORMAT set err\n");
				return rtn;
			}
		} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_FRAMERATE        : 
		{
			SBizEncodePara bizEncPara;
			sParaTgtIns.emBizParaType = EM_BIZ_ENCMAINPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizEncPara);
			if(0==rtn)
			{
				SBizParaTarget bizTar;
				bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
				SBizSystemPara bizSysPara;
				int ret = BizGetPara(&bizTar, &bizSysPara);	
				int format = EM_BIZ_PAL;
				if(ret==0)
				{
					format = bizSysPara.nVideoStandard;
				}			
				EMBIZCFGPARALIST para = EM_BIZ_CFG_PARALIST_FPSPAL;
				if(format == EM_BIZ_PAL)
				{
					if(bizEncPara.nVideoResolution == EM_BIZENC_RESOLUTION_D1)
					{
						para = EM_BIZ_CFG_PARALIST_FPSPALD1;
					}
					else if(bizEncPara.nVideoResolution == EM_BIZENC_RESOLUTION_CIF)
					{
						para = EM_BIZ_CFG_PARALIST_FPSPALCIF;
					}
				}
				else if(format == EM_BIZ_NTSC)
				{
					para = EM_BIZ_CFG_PARALIST_FPSNTSC;
					if(bizEncPara.nVideoResolution == EM_BIZENC_RESOLUTION_D1)
					{
						para = EM_BIZ_CFG_PARALIST_FPSNTSCD1;
					}
					else if(bizEncPara.nVideoResolution == EM_BIZENC_RESOLUTION_CIF)
					{
						para = EM_BIZ_CFG_PARALIST_FPSNTSCCIF;
					}
				}
			
				if( 0==BizConfigGetParaListValue(
						para, 
						val, 
						&nValue )
				)
				{
					bizEncPara.nFrameRate = nValue;
                    //add by Lirl on Nov/23/2011
                    if(0 == bizEncPara.nFrameRate)
                    {
                        if(EM_BIZ_PAL==format)
                        {
                            bizEncPara.nFrameRate = 25;
                        }
                        else if(EM_BIZ_NTSC==format)
                        {
                            bizEncPara.nFrameRate = 30;
                        }
                    }
                    //end
					
					//printf("ch = %d , nFrameRate = %d \n",ch,nValue);
					
					sParaTgtIns.emBizParaType = EM_BIZ_ENCMAINPARA;
					BizSetPara(&sParaTgtIns, &bizEncPara);
				}

			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_RECORD_ENABLE_CODESTREAM_FRAMERATE set err\n");
			}
		} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ENCODE         : 
		{
			SBizEncodePara bizEncPara;
			sParaTgtIns.emBizParaType = EM_BIZ_ENCMAINPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizEncPara);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_BITTYPE, 
						val, 
						&nValue )
				)
				{
					//printf("ch = %d , nBitRateType = %d \n",ch,nValue);
					bizEncPara.nBitRateType = (EMBIZENCBITRATETYPE)nValue;
					
					BizSetPara(&sParaTgtIns, &bizEncPara);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ENCODE set err\n");
			}
		} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+3
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_QUALITY        : 
		{
			SBizEncodePara bizEncPara;
			sParaTgtIns.emBizParaType = EM_BIZ_ENCMAINPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizEncPara);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_PICLEVEL, 
						val, 
						&nValue )
				)
				{
					bizEncPara.nPicLevel = (EMBIZENCPICLEVEL)nValue;
					//printf("##ch = %d , nPicLevel = %d \n",ch, bizEncPara.nPicLevel);
					
					BizSetPara(&sParaTgtIns, &bizEncPara);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_RECORD_ENABLE_CODESTREAM_QUALITY set err\n");
			}
		} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+4
		case GSR_CONFIG_RECORD_ENABLE_CODESTREAM_MAXRATE      : 
		{
			SBizEncodePara bizEncPara;
			sParaTgtIns.emBizParaType = EM_BIZ_ENCMAINPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizEncPara);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_BITRATE, 
						val, 
						&nValue )
				)
				{
					//printf("ch = %d , nBitRate = %d \n",ch,nValue);
					bizEncPara.nBitRate = nValue;
					
					BizSetPara(&sParaTgtIns, &bizEncPara);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_RECORD_ENABLE_CODESTREAM_MAXRATE set err\n");
			}




		} break; // GSR_CONFIG_RECORD_ENABLE_CODESTREAM_BASE+5
		
		
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_ALLCHN       : {;} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEBEFALARM   : 
		{
			
			SBizRecPara bizRecPara;
			sParaTgtIns.emBizParaType = EM_BIZ_RECPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizRecPara);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_PRETIME, 
						val, 
						&nValue )
				)
				{
					bizRecPara.nPreTime = nValue;
					
					return BizSetPara(&sParaTgtIns, &bizRecPara);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEBEFALARM set err\n");
				return rtn;
			}



		} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEAFTALARM       : 
		{
			
			SBizRecPara bizRecPara;
			sParaTgtIns.emBizParaType = EM_BIZ_RECPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizRecPara);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_RECDELAY, 
						val, 
						&nValue )
				)
				{
					bizRecPara.nRecDelay = nValue;
					
					BizSetPara(&sParaTgtIns, &bizRecPara);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEAFTALARM set err\n");
			}




		} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_RECORDTIME_EXPIRATION       : 
		{

			SBizRecPara bizRecPara;
			sParaTgtIns.emBizParaType = EM_BIZ_RECPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizRecPara);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_RECEXPIRE, 
						val, 
						&nValue )
				)
				{
					bizRecPara.nRecExpire = nValue;
					
					BizSetPara(&sParaTgtIns, &bizRecPara);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_RECORD_ENABLE_RECORDTIME_EXPIRATION set err\n");
			}



		} break; // GSR_CONFIG_RECORD_ENABLE_RECORDTIME_BASE+3
		
		
		case GSR_CONFIG_RECORD_ENABLE_OSD_ALLCHN        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+0
		case GSR_CONFIG_RECORD_ENABLE_OSD_CHNNAME         : 
		{
			printf("yg SaveScrInt chn:%d\n", ch);
			SBizCfgStrOsd bizStrOsd;
			sParaTgtIns.emBizParaType = EM_BIZ_STROSD;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizStrOsd);
			if(0==rtn)
			{
		
					bizStrOsd.nEncShowChnName = val;
					
					return BizSetPara(&sParaTgtIns, &bizStrOsd);
			
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_RECORD_ENABLE_OSD_CHNNAME set err\n");
				return rtn;
			}



		} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+1
		case GSR_CONFIG_RECORD_ENABLE_OSD_TIMESTAMP         : 
		{
			SBizCfgStrOsd bizStrOsd;
			sParaTgtIns.emBizParaType = EM_BIZ_STROSD;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizStrOsd);
			if(0==rtn)
			{
		
					bizStrOsd.nEncShowTime = val;
					
					BizSetPara(&sParaTgtIns, &bizStrOsd);
			
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_RECORD_ENABLE_OSD_TIMESTAMP set err\n");
			}




		} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+2
		case GSR_CONFIG_RECORD_ENABLE_OSD_POSITION        : {;} break; // GSR_CONFIG_RECORD_ENABLE_OSD_BASE+3
		
		
		case GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_REC        : 
		{
			
			SBizSystemPara bizSysPara;
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			//sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
			if(0==rtn)
			{
		
					bizSysPara.nCyclingRecord = val;
					
					return BizSetPara(&sParaTgtIns, &bizSysPara);
			
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_REC set err\n");
				return rtn;
			}




		} break; // GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_BASE+0
		
		
		
		
		
		//-------------------------------schedule
		
		//case GSR_CONFIG_SCHEDULE_TIMEREC  : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0
		case GSR_CONFIG_SCHEDULE_TIMEREC_CHN  : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+0
		case GSR_CONFIG_SCHEDULE_TIMEREC_SCH  : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+1
		case GSR_CONFIG_SCHEDULE_TIMEREC_COPYTO   : {;} break; // GSR_CONFIG_SCHEDULE_TIMEREC+2
		
		
		//case GSR_CONFIG_SCHEDULE_MOTIONALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x20
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+0
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH   : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+1
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+2
		
		//case GSR_CONFIG_SCHEDULE_SENSORALARMREC   : {;} break; // GSR_CONFIG_SCHEDULE_BASE+0x30
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_CHN   : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+0
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_SCH   : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+1
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_COPYTO  : {;} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+2
		
		
		//-------------------------------alarm
		
		//case GSR_CONFIG_ALARM_SENSOR    : {;} break; // GSR_CONFIG_ALARM_BASE+0
		
		case GSR_CONFIG_ALARM_SENSOR_BASE_ALLCHN    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+0
		case GSR_CONFIG_ALARM_SENSOR_BASE_ENABLE    : 
		{
			SBizAlarmSensorPara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nEnable = val;
				return BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_BASE_ENABLE set err\n");
				return -1;
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_BASE+1
		case GSR_CONFIG_ALARM_SENSOR_BASE_TYPE    : 
		{
			SBizAlarmSensorPara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_SENSORTYPE, 
						val, 
						&nValue )
				)
					
				{
					sPIns.nType = nValue;
					
					BizSetPara(&sParaTgtIns, &sPIns);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_BASE_TYPE set err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_BASE+2
		case GSR_CONFIG_ALARM_SENSOR_BASE_NAME    : {;} break; // GSR_CONFIG_ALARM_SENSOR_BASE+3
		
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL    : {;} break; // GSR_CONFIG_ALARM_SENSOR+0x20
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALLCHN     : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME    :
		{
			SBizAlarmSensorPara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_DELAY, 
						val, 
						&nValue 
					))
				{
					sPIns.nDelay = nValue;
					
					BizSetPara(&sParaTgtIns, &sPIns);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME set err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE: {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND :
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nFlagBuzz = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND set err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO  :
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nZoomChn = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND set err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER :
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nAlarmOut[ch] = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND set err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL  : 
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nFlagEmail = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL set err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP  : 
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nSnapChn[ch] = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP set err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER  : 
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nRecordChn[ch] = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND set err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP+1
		
		//case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE  : {;} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE : 
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_PTZLINKTYPE, 
						val, 
						&nValue 
					))
				{
					sPIns.sAlarmPtz[ch].nALaPtzType = nValue;
					sPIns.sAlarmPtz[ch].nChn = ch;
					
					BizSetPara(&sParaTgtIns, &sPIns);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE set err\n");
			}
		}  break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+0
		case GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX  :  
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_PTZLINKTYPE, 
						val, 
						&nValue 
					))
				{	
					sPIns.sAlarmPtz[ch].nId = nValue;
					sPIns.sAlarmPtz[ch].nChn = ch;
					
					BizSetPara(&sParaTgtIns, &sPIns);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE set err\n");
			}
		} break; // GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE  : 
		{
			SBizAlarmVMotionPara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nEnable = val;
				return BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE set err\n");
				return -1;
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+0
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND  : 
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nFlagBuzz = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND set err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO :
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nZoomChn = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO set err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_TIRGGER  :
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nAlarmOut[ch] = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO set err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL : 
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nFlagEmail = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL set err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP :
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nSnapChn[ch] = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP set err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_TRIGGER :
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nRecordChn[ch] = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP set err\n");
			}
		} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x10

		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND  :
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nFlagBuzz = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND set err\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO :
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nZoomChn= val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND set err\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER  :
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nAlarmOut[ch] = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND set err\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL : 
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nFlagEmail = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND set err\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP : 
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nSnapChn[ch] = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND set err\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER : 
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nRecordChn[ch] = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND set err\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_TYPE  : 
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_PTZLINKTYPE, 
						val, 
						&nValue 
					))
				{
					sPIns.sAlarmPtz[ch].nALaPtzType = (EMBIZALARMPTZTYPE)nValue;
					sPIns.sAlarmPtz[ch].nChn = ch;
					
					BizSetPara(&sParaTgtIns, &sPIns);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE set err\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_INDEX : 
		{
			SBizAlarmDispatch sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_PTZLINKTYPE, 
						val, 
						&nValue 
					))
				{	
					sPIns.sAlarmPtz[ch].nId = nValue;
					sPIns.sAlarmPtz[ch].nChn = ch;
					
					BizSetPara(&sParaTgtIns, &sPIns);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE set err\n");
			}
		} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+1
		
		
		//case GSR_CONFIG_ALARM_OTHER           : {;} break; // GSR_CONFIG_ALARM_BASE+0xA0
		
		case GSR_CONFIG_ALARM_OTHER_ALARMTYPE     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+1
		case GSR_CONFIG_ALARM_OTHER_SOUNDALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+2
		case GSR_CONFIG_ALARM_OTHER_MAIL        : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+3
		case GSR_CONFIG_ALARM_OTHER_TRIGGERALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+4
		case GSR_CONFIG_ALARM_OTHER_DISKALARM     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+5
		case GSR_CONFIG_ALARM_OTHER_ALLCHN      : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+6
		
		
		case GSR_CONFIG_ALARM_ALARMOUT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xB0
		
		
		case GSR_CONFIG_ALARM_EXIT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xC0
		
		
		//-------------------------------network
		
		case GSR_CONFIG_NETWORK_BASE    : 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = 0;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return 0;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME err\n");
			}
		} break; // GSR_CONFIG_BASE+0x500
		
		
		//-------------------------------user
		
		case GSR_CONFIG_USER_BASE       : {;} break; // GSR_CONFIG_BASE+0x600
		
		
		//-------------------------------ptz
		
		
		
		case GSR_PTZCONFIG_SERIAL_ENABLE    :		
		{
			SBizPtzPara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nEnable = val;
				return BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_ENABLE set err\n");
				return -1;
			}
		} break; // GSR_PTZCONFIG_BASE+1
		case GSR_PTZCONFIG_SERIAL_ADDRESS     :		
		{
			SBizPtzPara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nCamAddr = val;

				//printf("sPIns.nCamAddr   =   %d  \n",sPIns.nCamAddr);
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_PTZCONFIG_SERIAL_ENABLE set err\n");
			}
		} break; // GSR_PTZCONFIG_BASE+2
		case GSR_PTZCONFIG_SERIAL_BPS     :  
		{
			SBizPtzPara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_BAUDRATE, 
						val, 
						&nValue 
					))
				{	
					sPIns.nBaudRate = nValue;
					
					BizSetPara(&sParaTgtIns, &sPIns);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE set err\n");
			}
		} break; // GSR_PTZCONFIG_BASE+3
		case GSR_PTZCONFIG_SERIAL_PROTOCOL:
		
		{
			SBizPtzPara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_PTZPROTOCOL, 
						val, 
						&nValue 
					))
				{	
					sPIns.nProtocol = nValue;
					//printf("sPIns.nProtocol = %d\n",sPIns.nProtocol);
					BizSetPara(&sParaTgtIns, &sPIns);
				}
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE set err\n");
			}
		}
		//{;} break; // GSR_PTZCONFIG_BASE+4
		case GSR_PTZCONFIG_SERIAL_ALLCHECK    : {;} break; // GSR_PTZCONFIG_BASE+5
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_BASE
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE    :  
		{
			SBizPtzPara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.sAdvancedPara.nIsPresetSet[ch] = 1;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE set err\n");
			}
		} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ID    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+2
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_ID  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_NAME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+2
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_PRESET: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+3
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_RATE  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+4
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_TIME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+5
		
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_TRACK_BASE
		
		
		
		//-------------------------------advance
		//case GSR_CONFIG_ADVANCE     : {;} break; // GSR_CONFIG_BASE+0x800
		
		
		case GSR_ADVANCECONFIG_IMPORTEXPORT   : {;} break; // GSR_ADVANCECONFIG_BASE+0
		case GSR_ADVANCECONFIG_IMPORTEXPORT_REFRESH   : {;} break; // GSR_ADVANCECONFIG_IMPORTEXPORT+1
		
		
		// === search ===
		
		// === backup ===
		
		// === info ===
		
		//-------------------------------system
		
		case GSR_INFO_SYSTEM_DEVICENAME   : {;} break; // GSR_INFO_SYSTEM_BASE+1
		case GSR_INFO_SYSTEM_DEVICENUM  : {;} break; // GSR_INFO_SYSTEM_BASE+2
		case GSR_INFO_SYSTEM_HARDWAREVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+3
		case GSR_INFO_SYSTEM_SCMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+4
		case GSR_INFO_SYSTEM_KERNELVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+5
		case GSR_INFO_SYSTEM_ROMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+6
		case GSR_INFO_SYSTEM_RELEASEDATE  : {;} break; // GSR_INFO_SYSTEM_BASE+7
		
		
		case GSR_INFO_DISK_NO   : {;} break; // GSR_INFO_DISK_BASE+1
		case GSR_INFO_DISK_TYPE   : {;} break; // GSR_INFO_DISK_BASE+2
		case GSR_INFO_DISK_CAPACITY   : {;} break; // GSR_INFO_DISK_BASE+3
		case GSR_INFO_DISK_FREE   : {;} break; // GSR_INFO_DISK_BASE+4
		case GSR_INFO_DISK_STATE  : {;} break; // GSR_INFO_DISK_BASE+5
		case GSR_INFO_DISK_ATTRIBUTE  : {;} break; // GSR_INFO_DISK_BASE+6
		case GSR_INFO_DISK_SOURCE   : {;} break; // GSR_INFO_DISK_BASE+7
		case GSR_INFO_DISK_RSV1   : {;} break; // GSR_INFO_DISK_BASE+8
		case GSR_INFO_DISK_RSV2   : {;} break; // GSR_INFO_DISK_BASE+9
		case GSR_INFO_DISK_IMAGE  : {;} break; // GSR_INFO_DISK_BASE+10
		case GSR_INFO_DISK_TOTAL  : {;} break; // GSR_INFO_DISK_BASE+11
		case GSR_INFO_DISK_USED   : {;} break; // GSR_INFO_DISK_BASE+12
		
		
		//-------------------------------event
		
		
		//-------------------------------syslog
		
		
		//------------------------------- netinfo
		case GSR_INFO_NETINFO_BASE:
		{;
		}
		break;		
		
		//------------------------------- online
		
		case GSR_INFO_ONLINE_USERNAME : {;} break; // GSR_INFO_ONLINE_BASE+1
		case GSR_INFO_ONLINE_IP     : {;} break; // GSR_INFO_ONLINE_BASE+2
		case GSR_INFO_ONLINE_STATE  : {;} break; // GSR_INFO_ONLINE_BASE+3
		
		// exit
		
		
		
		// === alarm ===
		
		
		case GSR_MANUALALARM_ALLCHECK : {;} break; // GSR_MANUALALARM_BASE+1
		case GSR_MANUALALARM_ALARMNAME: {;} break; // GSR_MANUALALARM_BASE+2
		case GSR_MANUALALARM_IP     : {;} break; // GSR_MANUALALARM_BASE+3
		case GSR_MANUALALARM_ALARMPROCESS : {;} break; // GSR_MANUALALARM_BASE+4
		
		// === disk ===
		
		
		
		// === resource ===
		
		
		
		// === logoff ===
		
		
		
		// === shutdown ===
		
		
		// === desktop ===
		
		case GSR_DESKTOP_SYSTIME  : 
		{;} break; // GSR_DESKTOP_BASE+1
		case GSR_DESKTOP_CHNNAME  : {;} break; // GSR_DESKTOP_BASE+2
		
		//case GSR_DESKTOP_VIDEO_ATTRIBUTE  : {;} break; // GSR_DESKTOP_BASE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+0
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS   :  
		{
			//return 0;
			
			SBizPreviewImagePara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_IMAGEPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nBrightness = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS set err\n");
			}
		} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+1
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA   :   
		{
			//return 0;
			SBizPreviewImagePara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_IMAGEPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nHue = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS set err\n");
			}
		} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+2
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION   :   
		{
			//return 0;
			SBizPreviewImagePara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_IMAGEPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nSaturation = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS set err\n");
			}
		} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CONTRAST   :   
		{
			//return 0;
			SBizPreviewImagePara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_IMAGEPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nContrast = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS set err\n");
			}
		} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+4
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_DEFAULT  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+5
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS   :  
		{
			//return 0;
			
			SBizPreviewImagePara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			sParaTgtIns.emBizParaType = EM_BIZ_VO_IMAGEPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nBrightness = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_BRIGHTNESS set err\n");
			}
		} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+6
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_CHROMA   :   
		{
			//return 0;
			SBizPreviewImagePara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_VO_IMAGEPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nHue = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS set err\n");
			}
		} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+7
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_SATURATION   :   
		{
			//return 0;
			SBizPreviewImagePara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_VO_IMAGEPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nSaturation = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS set err\n");
			}
		} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+8
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_VO_CONTRAST   :   
		{
			//return 0;
			SBizPreviewImagePara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_VO_IMAGEPARA;
			sParaTgtIns.nChn = ch;
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				sPIns.nContrast = val;
				BizSetPara(&sParaTgtIns, &sPIns);
			}
			else
			{
				BIZ_DATA_DBG("GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS set err\n");
			}
		} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+9

		default:
			break;
	}
	
	return rtn;
}

int bizData_CopyOneDaySch( 
	uint nSch,
	SBizSchTime pSTIns[] )
{
	int i, j;
	//SBizSchTime* pSTIns;
	u8 bRecStart = 0;
	u8 nSeg = 0;
	
	//for(i=0; i<7; i++)
	{
		//pSTIns = pSTVecIns[i];
		
		for(j=0; j<24; j++)
		{
			// check 24 hours of one day
			// if j hour marked, set segment starttime when not set
			// if j not marked, skip and set last segment stoptime
			if(nSch&(0x1<<j))
			{
				// if not set starttime, set
				if(!bRecStart)
				{
					bRecStart = 1;
					pSTIns[nSeg].nStartTime = j*3600;
				}
				//else // if set , skip
				{
					if(j==23)
					{
						pSTIns[nSeg].nStopTime = (j+1)*3600-1;
					}
				}
			}
			else
			{
				// if last hour marked, set last hour endtime, clear setstarttime flag
				if(bRecStart && (nSch&(0x1<<(j-1))))
				{
					bRecStart = 0;
					pSTIns[nSeg].nStopTime = j*3600-1;
					
					nSeg++;
				}
				else
				// if last hour not marked, skip
				{
					;
				}
			}
		}
	}
	
	//for(i=0;i<1;i++)
		//BIZ_DT_DBG("start  %d end  %d\n",pSTIns[i].nStartTime, pSTIns[i].nStopTime);
	
	return 0;
}

int SaveScrSch(u32 nItemID, u8 ch, uint* pSch)
{
	int rtn, i, j;
	s32 nValue;
	
	SBizParaTarget sParaTgtIns;
	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));
	
	rtn = 0;//csp modify
	
	/*
	if(pSch)
		BIZ_DT_DBG("ID:%d CH:%d VAL:%ud %ud %ud %ud %ud %ud %ud\n", 
			nItemID, ch, pSch[0],pSch[1],pSch[2],pSch[3],pSch[4],pSch[5],pSch[6] );
	else
		BIZ_DT_DBG("ID:%d CH:%d VAL NULL!!!\n", nItemID, ch);
	*/
	
	// clear head 8 bits
	for(i=0;i<7;i++)
	{
		pSch[i] &= 0xffffffff;
		
		//printf("chn %d   pSch[%d] = %d \n",ch,i,pSch[i]);
	}
	
	switch(nItemID)
	{
		case GSR_CONFIG_ALARM_ALARMOUT_SCH_SCH:
		{		
			SBizSchedule sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_ALARMOUT;
			sParaTgtIns.nChn = ch;
			
			SBizSchTime* pSTIns;
			
			for(i=0; i<7; i++)
			{
				pSTIns = sPIns.sSchTime[i];
				bizData_CopyOneDaySch(pSch[i], pSTIns);
			}
			
			return BizSetPara(&sParaTgtIns, &sPIns);
			
		} break;
		//-------------------------------schedule
		case GSR_CONFIG_SCHEDULE_TIMEREC_SCH  :
		{		
			SBizSchedule sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECTIMING;
			sParaTgtIns.nChn = ch;
			
			SBizSchTime* pSTIns;
			
			for(i=0; i<7; i++)
			{
				pSTIns = sPIns.sSchTime[i];
				bizData_CopyOneDaySch(pSch[i], pSTIns);
			}
			
			return BizSetPara(&sParaTgtIns, &sPIns);
			
		} break; // GSR_CONFIG_SCHEDULE_TIMEREC+1
		case GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH   : 
		{		
			SBizSchedule sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECMOTION;
			sParaTgtIns.nChn = ch;
			
			SBizSchTime* pSTIns;

			
			for(i=0; i<7; i++)
			{
				pSTIns = sPIns.sSchTime[i];
				bizData_CopyOneDaySch(pSch[i], pSTIns);
			}

			
			//printf("GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH 1111\n");
			//printf("00 psch[0] = %d\n",pSch[0]);
			//printf("00  start %d, end %d \n",sPIns.sSchTime[0][0].nStartTime,sPIns.sSchTime[0][0].nStopTime);
			
			return BizSetPara(&sParaTgtIns, &sPIns);

			//printf("GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH 2222\n");
			
		} break; // GSR_CONFIG_SCHEDULE_MOTIONALARMREC+1
		case GSR_CONFIG_SCHEDULE_SENSORALARMREC_SCH   : 
		{		
			SBizSchedule sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_RECSENSOR;
			sParaTgtIns.nChn = ch;
			
			SBizSchTime* pSTIns;
			
			for(i=0; i<7; i++)
			{
				pSTIns = sPIns.sSchTime[i];
				bizData_CopyOneDaySch(pSch[i], pSTIns);
			}
			
			return BizSetPara(&sParaTgtIns, &sPIns);
			
		} break; // GSR_CONFIG_SCHEDULE_SENSORALARMREC+1
		case GSR_CONFIG_ALARM_SENSOR_SCH_SCH  : 
		{		
			SBizSchedule sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_SENSOR;
			sParaTgtIns.nChn = ch;
			
			SBizSchTime* pSTIns;
			for(i=0; i<7; i++)
			{
				pSTIns = sPIns.sSchTime[i];
				bizData_CopyOneDaySch(pSch[i], pSTIns);
			}
			
			return BizSetPara(&sParaTgtIns, &sPIns);
			
		} break; // GSR_CONFIG_ALARM_SENSOR_SCH+1
		case GSR_CONFIG_ALARM_IPCEXTSENSOR_SCH_SCH  : 
		{		
			SBizSchedule sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_IPCEXTSENSOR;
			sParaTgtIns.nChn = ch;
			
			SBizSchTime* pSTIns;
			for(i=0; i<7; i++)
			{
				pSTIns = sPIns.sSchTime[i];
				bizData_CopyOneDaySch(pSch[i], pSTIns);
				if (i == 0)
					printf("yg bizData_CopyOneDaySch sch[0]: 0x%x\n", pSch[0]);
			}
			
			return BizSetPara(&sParaTgtIns, &sPIns);
			
		} break; 
		case GSR_CONFIG_ALARM_SENSOR_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_SENSOR_SCH+2
		
		///
		
		//case GSR_CONFIG_ALARM_MD            : {;} break; // GSR_CONFIG_ALARM_BASE+0x40
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT     : {;} break; // GSR_CONFIG_ALARM_MD+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ENABLE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALLCHN  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+2
		
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT+3
		
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_TYPE  : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+0
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_INDEX : {;} break; // GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE+1
		
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET  : 
		{		
			SBizAlarmVMotionPara sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
			sParaTgtIns.nChn = ch;
			
			rtn = BizGetPara(&sParaTgtIns, &sPIns);
			if(0==rtn)
			{
				if(pSch)
					memcpy((void*)sPIns.nBlockStatus,pSch, 8*64); 
				
				BizSetPara(&sParaTgtIns, &sPIns);
			}
				
		} break; // GSR_CONFIG_ALARM_MD+0x28
		case GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET_SENSITY: {;} break;
		
		//case GSR_CONFIG_ALARM_MD_SCH      : {;} break; // GSR_CONFIG_ALARM_MD+0x30
		case GSR_CONFIG_ALARM_MD_SCH_SCH_CHN  : {;} break; // GSR_CONFIG_ALARM_MD_SCH+0
		case GSR_CONFIG_ALARM_MD_SCH_SCH_SCH  :
		{		
			SBizSchedule sPIns;
			memset(&sPIns, 0, sizeof(sPIns));
			
			sParaTgtIns.emBizParaType = EM_BIZ_SCHEDULE_VMOTION;
			sParaTgtIns.nChn = ch;
			
			SBizSchTime* pSTIns;
			for(i=0; i<7; i++)
			{
				pSTIns = sPIns.sSchTime[i];
				bizData_CopyOneDaySch(pSch[i], pSTIns);
			}
			
			return BizSetPara(&sParaTgtIns, &sPIns);
			
		} break; // GSR_CONFIG_ALARM_MD_SCH+1
		case GSR_CONFIG_ALARM_MD_SCH_SCH_COPYTO   : {;} break; // GSR_CONFIG_ALARM_MD_SCH+2
		
		
		//case GSR_CONFIG_ALARM_VIDEOLOST     : {;} break; // GSR_CONFIG_ALARM_BASE+0x80
		
		//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST+0
		
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+1
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+2
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+3
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE+4 // bitmap
		
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x10
		
		//case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0x20
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_TYPE  : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+0
		case GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_INDEX : {;} break; // GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL+1
		
		
		//case GSR_CONFIG_ALARM_OTHER           : {;} break; // GSR_CONFIG_ALARM_BASE+0xA0
		
		case GSR_CONFIG_ALARM_OTHER_ALARMTYPE     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+1
		case GSR_CONFIG_ALARM_OTHER_SOUNDALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+2
		case GSR_CONFIG_ALARM_OTHER_MAIL        : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+3
		case GSR_CONFIG_ALARM_OTHER_TRIGGERALARM    : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+4
		case GSR_CONFIG_ALARM_OTHER_DISKALARM     : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+5
		case GSR_CONFIG_ALARM_OTHER_ALLCHN      : {;} break; // GSR_CONFIG_ALARM_OTHER_BASE+6
		
		
		case GSR_CONFIG_ALARM_ALARMOUT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xB0
		
		
		case GSR_CONFIG_ALARM_EXIT    : {;} break; // GSR_CONFIG_ALARM_BASE+0xC0
		
		
		//-------------------------------network
		
		case GSR_CONFIG_NETWORK_BASE    : 
		{
			SBizAlarmDispatch sCfgIns;
			memset(&sCfgIns, 0, sizeof(sCfgIns));
			sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
			sParaTgtIns.nChn = 0;
			rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
			if(0==rtn)
			{
				return 0;
			}
			else
			{
				BIZ_DATA_DBG("GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_DELAYTIME err\n");
			}
		} break; // GSR_CONFIG_BASE+0x500
		
		
		//-------------------------------user
		
		case GSR_CONFIG_USER_BASE       : {;} break; // GSR_CONFIG_BASE+0x600
		
		
		//-------------------------------ptz
		
		
		
		case GSR_PTZCONFIG_SERIAL_ENABLE    : {;} break; // GSR_PTZCONFIG_BASE+1
		case GSR_PTZCONFIG_SERIAL_ADDRESS     : {;} break; // GSR_PTZCONFIG_BASE+2
		case GSR_PTZCONFIG_SERIAL_BPS     : {;} break; // GSR_PTZCONFIG_BASE+3
		case GSR_PTZCONFIG_SERIAL_PROTOCOL    : {;} break; // GSR_PTZCONFIG_BASE+4
		case GSR_PTZCONFIG_SERIAL_ALLCHECK    : {;} break; // GSR_PTZCONFIG_BASE+5
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_BASE
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ENABLE    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_PRESET_ID    : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_PRESET_BASE+2
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE
		
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_ID  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+1
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_NAME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+2
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_PRESET: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+3
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_RATE  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+4
		case GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_CRUISEPOINT_TIME  : {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_CRUISELINE_BASE+5
		
		
		//: {;} break; // GSR_PTZCONFIG_ADVANCECONFIG_TRACK_BASE
		
		
		
		//-------------------------------advance
		//case GSR_CONFIG_ADVANCE     : {;} break; // GSR_CONFIG_BASE+0x800
		
		
		case GSR_ADVANCECONFIG_IMPORTEXPORT   : {;} break; // GSR_ADVANCECONFIG_BASE+0
		case GSR_ADVANCECONFIG_IMPORTEXPORT_REFRESH   : {;} break; // GSR_ADVANCECONFIG_IMPORTEXPORT+1
		
		
		// === search ===
		
		// === backup ===
		
		// === info ===
		
		//-------------------------------system
		
		case GSR_INFO_SYSTEM_DEVICENAME   : {;} break; // GSR_INFO_SYSTEM_BASE+1
		case GSR_INFO_SYSTEM_DEVICENUM  : {;} break; // GSR_INFO_SYSTEM_BASE+2
		case GSR_INFO_SYSTEM_HARDWAREVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+3
		case GSR_INFO_SYSTEM_SCMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+4
		case GSR_INFO_SYSTEM_KERNELVERSION  : {;} break; // GSR_INFO_SYSTEM_BASE+5
		case GSR_INFO_SYSTEM_ROMVERSION   : {;} break; // GSR_INFO_SYSTEM_BASE+6
		case GSR_INFO_SYSTEM_RELEASEDATE  : {;} break; // GSR_INFO_SYSTEM_BASE+7
		
		
		case GSR_INFO_DISK_NO   : {;} break; // GSR_INFO_DISK_BASE+1
		case GSR_INFO_DISK_TYPE   : {;} break; // GSR_INFO_DISK_BASE+2
		case GSR_INFO_DISK_CAPACITY   : {;} break; // GSR_INFO_DISK_BASE+3
		case GSR_INFO_DISK_FREE   : {;} break; // GSR_INFO_DISK_BASE+4
		case GSR_INFO_DISK_STATE  : {;} break; // GSR_INFO_DISK_BASE+5
		case GSR_INFO_DISK_ATTRIBUTE  : {;} break; // GSR_INFO_DISK_BASE+6
		case GSR_INFO_DISK_SOURCE   : {;} break; // GSR_INFO_DISK_BASE+7
		case GSR_INFO_DISK_RSV1   : {;} break; // GSR_INFO_DISK_BASE+8
		case GSR_INFO_DISK_RSV2   : {;} break; // GSR_INFO_DISK_BASE+9
		case GSR_INFO_DISK_IMAGE  : {;} break; // GSR_INFO_DISK_BASE+10
		case GSR_INFO_DISK_TOTAL  : {;} break; // GSR_INFO_DISK_BASE+11
		case GSR_INFO_DISK_USED   : {;} break; // GSR_INFO_DISK_BASE+12
		
		
		//-------------------------------event
		
		
		//-------------------------------syslog
		
		
		//------------------------------- netinfo
		case GSR_INFO_NETINFO_BASE:
		{;
		}
		break;		
		
		//------------------------------- online
		
		case GSR_INFO_ONLINE_USERNAME : {;} break; // GSR_INFO_ONLINE_BASE+1
		case GSR_INFO_ONLINE_IP     : {;} break; // GSR_INFO_ONLINE_BASE+2
		case GSR_INFO_ONLINE_STATE  : {;} break; // GSR_INFO_ONLINE_BASE+3
		
		// exit
		
		
		
		// === alarm ===
		
		
		case GSR_MANUALALARM_ALLCHECK : {;} break; // GSR_MANUALALARM_BASE+1
		case GSR_MANUALALARM_ALARMNAME: {;} break; // GSR_MANUALALARM_BASE+2
		case GSR_MANUALALARM_IP     : {;} break; // GSR_MANUALALARM_BASE+3
		case GSR_MANUALALARM_ALARMPROCESS : {;} break; // GSR_MANUALALARM_BASE+4
		
		// === disk ===
		
		
		
		// === resource ===
		
		
		
		// === logoff ===
		
		
		
		// === shutdown ===
		
		
		// === desktop ===
		
		case GSR_DESKTOP_SYSTIME  : 
		{;} break; // GSR_DESKTOP_BASE+1
		case GSR_DESKTOP_CHNNAME  : {;} break; // GSR_DESKTOP_BASE+2
		
		//case GSR_DESKTOP_VIDEO_ATTRIBUTE  : {;} break; // GSR_DESKTOP_BASE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHN  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+0
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_BRIGHTNESS   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+1
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CHROMA   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+2
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_SATURATION   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+3
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_CONTRAST   : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+4
		case GSR_DESKTOP_VIDEO_ATTRIBUTE_DEFAULT  : {;} break; // GSR_DESKTOP_VIDEO_ATTRIBUTE+5
	}
	
	return rtn;//csp modify
}
/*
	Save Parameters to config module
	 
*/
int SaveScrStr2Cfg(
	void* 			pVoid,
	u32 			nItemID, 
	EM_GSR_CTRL 	type, 
	u8 				ch
)
{
	CCheckBox* 	pCheckBox;
	CComboBox* 	pCombBox;
	CButton* 	pButton;
	CStatic* 	pStatic;
	CSliderCtrl* pSlider;
	CGridCtrl** 	pGrid;
	CEdit* 		pEdit;
	CIPAddressCtrl* pIPCtrl;

	if(NULL==pVoid)
	{
		BIZ_DATA_DBG("SaveScrStr2Cfg NULL!!!\n");

		return -1;
	}

	int 	val;
	char 	szStr[EDIT_MAX] = {0};
	u8*	  	pSch;
	IPADDR	sIpIns;
	
	int ret = 0;//int ret;//csp modify
	
	switch(type)
	{
		case EM_GSR_CTRL_CHK:
		{
			pCheckBox = (CCheckBox*)pVoid;
			
			val = pCheckBox->GetValue();
			//printf("yg pCheckBox->GetValue: %d\n", val);
			ret = SaveScrInt(nItemID, ch, val);
		}
		break;
		case EM_GSR_CTRL_COMB:
		{
			pCombBox = (CComboBox*)pVoid;
			
			val = pCombBox->GetCurSel();
			//printf("ch = %d, nItemID = %d , index = %d\n",ch,nItemID,val);
			ret = SaveScrInt(nItemID, ch, val);
		}
		break;
		case EM_GSR_CTRL_EDIT:
		{
			pEdit = (CEdit*)pVoid;
			
			pEdit->GetText(szStr, sizeof(szStr));
			//printf("%s chn%d edit: %s\n",__func__, ch, szStr);//QYJ
			
			ret = SaveScrStr(nItemID, ch, szStr);
		}
		break;
		case EM_GSR_CTRL_SLIDE:
		{
			pSlider = (CSliderCtrl*)pVoid;

			val = pSlider->GetPos();
			
			ret = SaveScrInt(nItemID, ch, val);
		}
		break;
		case EM_GSR_CTRL_SZIPLIST:
		{
			pIPCtrl = (CIPAddressCtrl*)pVoid;

			pIPCtrl->GetIPAddress(&sIpIns);
			
			ret = SaveScrInt(nItemID, ch, sIpIns.l);
		}
		break;
		case EM_GSR_CTRL_SCH:
		{
			int i;
			
			pGrid = (CGridCtrl**)pVoid;
			uint sch[7];
			for(i=0; i<7; i++)
				pGrid[i]->GetMask(&sch[i]);

			ret = SaveScrSch(nItemID, ch, sch);
		}
		break;
		case EM_GSR_CTRL_SCH_VAL:
		{
			ret = SaveScrSch(nItemID, ch, (uint*)pVoid);
		}
		break;
		case EM_GSR_CTRL_RGNSET:
		{
			ret = SaveScrSch(nItemID, ch, (uint*)pVoid);
		}
		break;
	}
	
	return ret;
}

/*
 get / set region 
 
*/
int GetRgnCurDo(
	u64* 	pRgn,
	u32 	nItemID, 
	EM_GSR_CTRL 	type, 
	u8 		ch
)
{
	int rtn, i, j;
	s32 nValue;
	
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));
	
	SBizAlarmVMotionPara sPIns;
	memset(&sPIns, 0, sizeof(sPIns));
	
	sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
	sParaTgtIns.nChn = ch;

	rtn = BizGetPara(&sParaTgtIns, &sPIns);
	if(pRgn&&rtn==0)
		memcpy(pRgn, (void*)sPIns.nBlockStatus, 8*64); 

	return 0;
}

void bizData_CreateCDC()
{
	if(pCdc==NULL)
	{
		pCdc = new CDC(DC_MEMORY, 10, 10);
		if(pCdc==NULL)
		{
			printf("fail to create CDC!\n");
		}
		else
		{
			printf("success to create CDC!\n");
		}
	}
}

void bizData_SetLocales(CLocales* pLocale)
{
	pLocales = pLocale;
}

void bizData_GetLocales(CLocales** pLocale)
{
	*pLocale = pLocales;
}

void bizData_resumeRecord()
{
	SBizParaTarget bizTar;
	SBizRecPara bizRecPara;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	for(int i=0; i<GetVideoMainNum(); i++)
	{
		bizTar.nChn = i;
		if(0 != BizGetPara(&bizTar, &bizRecPara))
		{
			printf("Get rec %d para error \n",i);
			continue;
		}
		
		if(bizRecPara.bRecording)
		{
			//printf("chn%d start manual......\n",i);
			BizStartManualRec(i);
		}
	}
}

int bizData_GetSch(SBizSchedule* pSchPara, uint* pTgtSch)
{
	int i, j, k, l, h0, h1;
	
/*	i = 0;
	while(i++ <7)
	{
		printf("##start%d:%d, stop%d:%d\n", i,pSchPara->sSchTime[i-1][0].nStartTime,i,pSchPara->sSchTime[i-1][0].nStopTime);
	}
*/
	//printf("bizData_GetSch1 %p\n", pSchPara);
	
	#if 1
	for(i=0; i<7; i++)
	{
		SBizSchTime* pSch = pSchPara->sSchTime[i];
		
		//printf("bizData_GetSch2 %p pTgtSch %p\n", pSch, pTgtSch);
		
		for(j=0; j<CUR_SCH_SEG; j++)
		{
			if(pSch[j].nStartTime<pSch[j].nStopTime)
			{
				h0 = pSch[j].nStartTime/3600;
				h1 = (pSch[j].nStopTime)/3600;
				
				for(k=h0; k<=h1; k++)
				{
					pTgtSch[i] |= (0x1<<k);
				}
			}
		}
	}
	#endif
	
	return 0;
}

int bizData_GetSENSORAlarmDealDefault(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
		sParaTgtIns.nChn = nCh;
		
		int ret =  BizGetDefaultPara(&sParaTgtIns, &sDisPatch);

		//BIZ_DO_DBG("bizData_GetSENSORAlarmDeal ... \n");
		//BIZ_DO_DBG("CH %d SNAP %d %d %d %d\n", nCh, 
		//	sDisPatch.nSnapChn[0],
		//	sDisPatch.nSnapChn[1],
		//	sDisPatch.nSnapChn[2],
		//	sDisPatch.nSnapChn[3] );
		
		memcpy(pGDispatch, &sDisPatch, sizeof(SGuiAlarmDispatch));

		return ret;
	}
	
	return -1;
}
//yaogang modify 20141010
int bizData_GetIPCCoverAlarmDealDefault(u8 nCh, SGuiAlarmDispatch * pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch; 

	if(pGDispatch)
	{
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_IPCCOVER;
		sParaTgtIns.nChn = nCh;
		
		int ret =  BizGetDefaultPara(&sParaTgtIns, &sDisPatch);

		//BIZ_DO_DBG("bizData_GetSENSORAlarmDeal ... \n");
		//BIZ_DO_DBG("CH %d SNAP %d %d %d %d\n", nCh, 
		//	sDisPatch.nSnapChn[0],
		//	sDisPatch.nSnapChn[1],
		//	sDisPatch.nSnapChn[2],
		//	sDisPatch.nSnapChn[3] );
		
		memcpy(pGDispatch, &sDisPatch, sizeof(SGuiAlarmDispatch));

		return ret;
	}
	
	return -1;
}

int bizData_GetHDDAlarmDealDefault(SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_HDD;
		sParaTgtIns.nChn = 0;
		
		int ret =  BizGetDefaultPara(&sParaTgtIns, &sDisPatch);

		//BIZ_DO_DBG("bizData_GetSENSORAlarmDeal ... \n");
		//BIZ_DO_DBG("CH %d SNAP %d %d %d %d\n", nCh, 
		//	sDisPatch.nSnapChn[0],
		//	sDisPatch.nSnapChn[1],
		//	sDisPatch.nSnapChn[2],
		//	sDisPatch.nSnapChn[3] );
		
		memcpy(pGDispatch, &sDisPatch, sizeof(SGuiAlarmDispatch));

		return ret;
	}
	
	return -1;
}


int bizData_GetIPCExtSENSORAlarmDealDefault(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch; 

	if(pGDispatch)
	{
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_IPCEXTSENSOR;
		sParaTgtIns.nChn = nCh;
		
		int ret =  BizGetDefaultPara(&sParaTgtIns, &sDisPatch);

		//BIZ_DO_DBG("bizData_GetSENSORAlarmDeal ... \n");
		//BIZ_DO_DBG("CH %d SNAP %d %d %d %d\n", nCh, 
		//	sDisPatch.nSnapChn[0],
		//	sDisPatch.nSnapChn[1],
		//	sDisPatch.nSnapChn[2],
		//	sDisPatch.nSnapChn[3] );
		
		memcpy(pGDispatch, &sDisPatch, sizeof(SGuiAlarmDispatch));

		return ret;
	}
	
	return -1;
}
int bizData_Get485ExtSENSORAlarmDealDefault(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch; 

	if(pGDispatch)
	{
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_485EXTSENSOR;
		sParaTgtIns.nChn = nCh;
		
		int ret =  BizGetDefaultPara(&sParaTgtIns, &sDisPatch);

		//BIZ_DO_DBG("bizData_GetSENSORAlarmDeal ... \n");
		//BIZ_DO_DBG("CH %d SNAP %d %d %d %d\n", nCh, 
		//	sDisPatch.nSnapChn[0],
		//	sDisPatch.nSnapChn[1],
		//	sDisPatch.nSnapChn[2],
		//	sDisPatch.nSnapChn[3] );
		
		memcpy(pGDispatch, &sDisPatch, sizeof(SGuiAlarmDispatch));

		return ret;
	}
	
	return -1;
}


void bizData_GetSENSORAlarmDeal(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
		sParaTgtIns.nChn = nCh;
		
		BizGetPara(&sParaTgtIns, &sDisPatch);

		//BIZ_DO_DBG("bizData_GetSENSORAlarmDeal ... \n");
		//BIZ_DO_DBG("CH %d SNAP %d %d %d %d\n", nCh, 
		//	sDisPatch.nSnapChn[0],
		//	sDisPatch.nSnapChn[1],
		//	sDisPatch.nSnapChn[2],
		//	sDisPatch.nSnapChn[3] );

		/*
		printf("CH %d ptz %d %d %d %d %d %d %d %d\n", 
			nCh, 
			sDisPatch.sAlarmPtz[0].nALaPtzType,
			sDisPatch.sAlarmPtz[0].nId,
			sDisPatch.sAlarmPtz[1].nALaPtzType,
			sDisPatch.sAlarmPtz[1].nId,
			sDisPatch.sAlarmPtz[2].nALaPtzType,
			sDisPatch.sAlarmPtz[2].nId,
			sDisPatch.sAlarmPtz[3].nALaPtzType,
			sDisPatch.sAlarmPtz[3].nId
		);*/
		
		memcpy(pGDispatch, &sDisPatch, sizeof(SGuiAlarmDispatch));
	}
	
	return;
}
void bizData_GetIPCCoverAlarmDeal(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_IPCCOVER;
		sParaTgtIns.nChn = nCh;
		
		BizGetPara(&sParaTgtIns, &sDisPatch);

		//BIZ_DO_DBG("bizData_GetSENSORAlarmDeal ... \n");
		//BIZ_DO_DBG("CH %d SNAP %d %d %d %d\n", nCh, 
		//	sDisPatch.nSnapChn[0],
		//	sDisPatch.nSnapChn[1],
		//	sDisPatch.nSnapChn[2],
		//	sDisPatch.nSnapChn[3] );

		/*
		printf("CH %d ptz %d %d %d %d %d %d %d %d\n", 
			nCh, 
			sDisPatch.sAlarmPtz[0].nALaPtzType,
			sDisPatch.sAlarmPtz[0].nId,
			sDisPatch.sAlarmPtz[1].nALaPtzType,
			sDisPatch.sAlarmPtz[1].nId,
			sDisPatch.sAlarmPtz[2].nALaPtzType,
			sDisPatch.sAlarmPtz[2].nId,
			sDisPatch.sAlarmPtz[3].nALaPtzType,
			sDisPatch.sAlarmPtz[3].nId
		);*/
		
		memcpy(pGDispatch, &sDisPatch, sizeof(SGuiAlarmDispatch));
	}
	
	return;
}

void bizData_GetIPCExtSENSORAlarmDeal(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_IPCEXTSENSOR;
		sParaTgtIns.nChn = nCh;
		
		BizGetPara(&sParaTgtIns, &sDisPatch);

		//BIZ_DO_DBG("bizData_GetSENSORAlarmDeal ... \n");
		//BIZ_DO_DBG("CH %d SNAP %d %d %d %d\n", nCh, 
		//	sDisPatch.nSnapChn[0],
		//	sDisPatch.nSnapChn[1],
		//	sDisPatch.nSnapChn[2],
		//	sDisPatch.nSnapChn[3] );

		/*
		printf("CH %d ptz %d %d %d %d %d %d %d %d\n", 
			nCh, 
			sDisPatch.sAlarmPtz[0].nALaPtzType,
			sDisPatch.sAlarmPtz[0].nId,
			sDisPatch.sAlarmPtz[1].nALaPtzType,
			sDisPatch.sAlarmPtz[1].nId,
			sDisPatch.sAlarmPtz[2].nALaPtzType,
			sDisPatch.sAlarmPtz[2].nId,
			sDisPatch.sAlarmPtz[3].nALaPtzType,
			sDisPatch.sAlarmPtz[3].nId
		);*/
		
		memcpy(pGDispatch, &sDisPatch, sizeof(SGuiAlarmDispatch));
	}
	
	return;
}
void bizData_Get485ExtSENSORAlarmDeal(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_485EXTSENSOR;
		sParaTgtIns.nChn = nCh;
		
		BizGetPara(&sParaTgtIns, &sDisPatch);

		//BIZ_DO_DBG("bizData_GetSENSORAlarmDeal ... \n");
		//BIZ_DO_DBG("CH %d SNAP %d %d %d %d\n", nCh, 
		//	sDisPatch.nSnapChn[0],
		//	sDisPatch.nSnapChn[1],
		//	sDisPatch.nSnapChn[2],
		//	sDisPatch.nSnapChn[3] );

		/*
		printf("CH %d ptz %d %d %d %d %d %d %d %d\n", 
			nCh, 
			sDisPatch.sAlarmPtz[0].nALaPtzType,
			sDisPatch.sAlarmPtz[0].nId,
			sDisPatch.sAlarmPtz[1].nALaPtzType,
			sDisPatch.sAlarmPtz[1].nId,
			sDisPatch.sAlarmPtz[2].nALaPtzType,
			sDisPatch.sAlarmPtz[2].nId,
			sDisPatch.sAlarmPtz[3].nALaPtzType,
			sDisPatch.sAlarmPtz[3].nId
		);*/
		
		memcpy(pGDispatch, &sDisPatch, sizeof(SGuiAlarmDispatch));
	}
	
	return;
}
void bizData_GetHDDAlarmDeal(SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_HDD;
		sParaTgtIns.nChn = 0;
		
		BizGetPara(&sParaTgtIns, &sDisPatch);

		//BIZ_DO_DBG("bizData_GetSENSORAlarmDeal ... \n");
		//BIZ_DO_DBG("CH %d SNAP %d %d %d %d\n", nCh, 
		//	sDisPatch.nSnapChn[0],
		//	sDisPatch.nSnapChn[1],
		//	sDisPatch.nSnapChn[2],
		//	sDisPatch.nSnapChn[3] );

		/*
		printf("CH %d ptz %d %d %d %d %d %d %d %d\n", 
			nCh, 
			sDisPatch.sAlarmPtz[0].nALaPtzType,
			sDisPatch.sAlarmPtz[0].nId,
			sDisPatch.sAlarmPtz[1].nALaPtzType,
			sDisPatch.sAlarmPtz[1].nId,
			sDisPatch.sAlarmPtz[2].nALaPtzType,
			sDisPatch.sAlarmPtz[2].nId,
			sDisPatch.sAlarmPtz[3].nALaPtzType,
			sDisPatch.sAlarmPtz[3].nId
		);*/
		
		memcpy(pGDispatch, &sDisPatch, sizeof(SGuiAlarmDispatch));
	}
	
	return;
}



int bizData_GetVLOSTAlarmDealDefault(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
		sParaTgtIns.nChn = nCh;
		
		int ret = BizGetDefaultPara(&sParaTgtIns, &sDisPatch);
		
		/*BIZ_DO_DBG("CH %d SNAP %d %d %d %d\n", nCh, 
			sDisPatch.nSnapChn[0],
			sDisPatch.nSnapChn[1],
			sDisPatch.nSnapChn[2],
			sDisPatch.nSnapChn[3] );
		*/
		
		memcpy(pGDispatch, &sDisPatch, sizeof(SGuiAlarmDispatch));

		return ret;
	}
	
	return -1;
}

void bizData_GetVLOSTAlarmDeal(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
		sParaTgtIns.nChn = nCh;
		
		BizGetPara(&sParaTgtIns, &sDisPatch);

		/*
		BIZ_DO_DBG("CH %d SNAP %d %d %d %d\n", nCh, 
			sDisPatch.nSnapChn[0],
			sDisPatch.nSnapChn[1],
			sDisPatch.nSnapChn[2],
			sDisPatch.nSnapChn[3] );
		*/
		
		memcpy(pGDispatch, &sDisPatch, sizeof(SGuiAlarmDispatch));
	}
	
	return;
}


int bizData_GetVMOTIONAlarmDealDefault(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
		sParaTgtIns.nChn = nCh;
		
		int ret = BizGetDefaultPara(&sParaTgtIns, &sDisPatch);
		
		memcpy(pGDispatch, &sDisPatch, sizeof(SGuiAlarmDispatch));

		return ret;
	}
	
	return -1;
}

void bizData_GetVMOTIONAlarmDeal(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
		sParaTgtIns.nChn = nCh;
		
		BizGetPara(&sParaTgtIns, &sDisPatch);
		
		memcpy(pGDispatch, &sDisPatch, sizeof(SGuiAlarmDispatch));
	}
	
	return;
}

void bizData_SaveSENSORAlarmDeal(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		memcpy(&sDisPatch, pGDispatch, sizeof(SGuiAlarmDispatch));
	
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_SENSOR;
		sParaTgtIns.nChn = nCh;
		
		BizSetPara(&sParaTgtIns, &sDisPatch);
	}
	
	return;
}
//yaogang modify 20141010
void bizData_SaveIPCCoverAlarmDeal(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		memcpy(&sDisPatch, pGDispatch, sizeof(SGuiAlarmDispatch));
	
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_IPCCOVER;
		sParaTgtIns.nChn = nCh;
		
		BizSetPara(&sParaTgtIns, &sDisPatch);
	}
	
	return;
}

void bizData_SaveIPCExtSENSORAlarmDeal(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		memcpy(&sDisPatch, pGDispatch, sizeof(SGuiAlarmDispatch));
	
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_IPCEXTSENSOR;
		sParaTgtIns.nChn = nCh;
		
		BizSetPara(&sParaTgtIns, &sDisPatch);
	}
	
	return;
}
void bizData_Save485ExtSENSORAlarmDeal(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		memcpy(&sDisPatch, pGDispatch, sizeof(SGuiAlarmDispatch));
	
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_485EXTSENSOR;
		sParaTgtIns.nChn = nCh;
		
		BizSetPara(&sParaTgtIns, &sDisPatch);
	}
	
	return;
}
void	bizData_SaveHDDAlarmDeal(SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		memcpy(&sDisPatch, pGDispatch, sizeof(SGuiAlarmDispatch));
	
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_HDD;
		sParaTgtIns.nChn = 0;
		
		BizSetPara(&sParaTgtIns, &sDisPatch);
	}
	
	return;
}



void bizData_SaveVMOTIONAlarmDeal(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		memcpy(&sDisPatch, pGDispatch, sizeof(SGuiAlarmDispatch));
	
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VMOTION;
		sParaTgtIns.nChn = nCh;
		
		BizSetPara(&sParaTgtIns, &sDisPatch);
	}
	
	return;
	
}

int bizData_SaveVLOSTAlarmDeal(u8 nCh, SGuiAlarmDispatch* pGDispatch)
{
	SBizParaTarget sParaTgtIns;

	memset(&sParaTgtIns, 0, sizeof(sParaTgtIns));

	SBizAlarmDispatch sDisPatch;

	if(pGDispatch)
	{
		memcpy(&sDisPatch, pGDispatch, sizeof(SGuiAlarmDispatch));
	
		sParaTgtIns.emBizParaType = EM_BIZ_DISPATCH_VLOST;
		sParaTgtIns.nChn = nCh;
		
		return BizSetPara(&sParaTgtIns, &sDisPatch);
	}
	
	return -1;
}


int bizData_GetPtzAdvancedParaDefault(uchar nChn, SGuiPtzAdvancedPara* pGDispatch)
{
	s32 rtn;
	SBizParaTarget sParaTgtIns;
	SBizPtzPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
	sParaTgtIns.nChn = nChn;
	if(pGDispatch)
	{
		rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			memcpy(pGDispatch, &sCfgIns.sAdvancedPara, sizeof(SGuiPtzAdvancedPara));
		}		
	}
	
	return rtn;
}


int bizData_GetUartPara(uchar nChn, STabParaUart* pTabPara) 
{ 
	s32 rtn;
	SBizParaTarget sParaTgtIns;
	SBizPtzPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
	sParaTgtIns.nChn = nChn;
	if(pTabPara)
	{
		memset(pTabPara, 0, sizeof(STabParaUart));
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			u8 nIdx;

			pTabPara->bEnable = sCfgIns.nEnable;
			pTabPara->nAddress= sCfgIns.nCamAddr;
			
			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_BAUDRATE,
						sCfgIns.nBaudRate,
						&nIdx
				) 
			)
			{
				pTabPara->nBaudRate= nIdx;
			}
			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_PTZPROTOCOL,
						sCfgIns.nProtocol,
						&nIdx
				) 
			)
			{
				pTabPara->nProtocol= nIdx;
			}
			
		}
	}
	
	return rtn;
}

int bizData_GetUartParaDefault(uchar nChn, STabParaUart* pTabPara) 
{
	s32 rtn;
	SBizParaTarget sParaTgtIns;
	SBizPtzPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
	sParaTgtIns.nChn = nChn;
	if(pTabPara)
	{
		memset(pTabPara, 0, sizeof(STabParaUart));
		rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			u8 nIdx;

			pTabPara->bEnable = sCfgIns.nEnable;
			pTabPara->nAddress= sCfgIns.nCamAddr;
			
			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_BAUDRATE,
						sCfgIns.nBaudRate,
						&nIdx
				) 
			)
			{
				pTabPara->nBaudRate= nIdx;
			}
			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_PTZPROTOCOL,
						sCfgIns.nProtocol,
						&nIdx
				) 
			)
			{
				pTabPara->nProtocol= nIdx;
			}
		}
	}
	
	return rtn;
}

int bizData_SetUartPara(uchar nChn, STabParaUart* pTabPara) 
{ 
	s32 rtn;
	SBizParaTarget sParaTgtIns;
	SBizPtzPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
	sParaTgtIns.nChn = nChn;
	if(pTabPara)
	{
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			s32 nVal;

			sCfgIns.nEnable = pTabPara->bEnable;
			sCfgIns.nCamAddr = pTabPara->nAddress;
            sCfgIns.nProtocol = pTabPara->nProtocol;
            sCfgIns.nBaudRate = pTabPara->nBaudRate;
			
			if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_BAUDRATE,
						pTabPara->nBaudRate,
						&nVal
				) 
			)
			{
				sCfgIns.nBaudRate = nVal;
			}
			
			if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_PTZPROTOCOL,
						pTabPara->nProtocol,
						&nVal
				) 
			)
			{
				sCfgIns.nProtocol = nVal;
			}
			BizSetPara(&sParaTgtIns, &sCfgIns);

			//printf("##addr %d br %d pr %d\n", sCfgIns.nCamAddr, sCfgIns.nBaudRate, sCfgIns.nProtocol);
		}
	}
	
	return rtn;
}

int bizData_GetPtzAdvancedPara(uchar nChn, SGuiPtzAdvancedPara* pGDispatch)
{
	s32 rtn;
	SBizParaTarget sParaTgtIns;
	SBizPtzPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
	sParaTgtIns.nChn = nChn;
	if(pGDispatch)
	{
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			memcpy(pGDispatch, &sCfgIns.sAdvancedPara, sizeof(SGuiPtzAdvancedPara));
		}
	}
	
	return rtn;
}

int bizData_SavePtzAdvancedPara(uchar nChn, SGuiPtzAdvancedPara* pGDispatch)
{
	s32 rtn;
	SBizParaTarget sParaTgtIns;
	SBizPtzPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_PTZPARA;
	sParaTgtIns.nChn = nChn;
	if(pGDispatch)
	{
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
        if (0 == rtn)
		{
			memcpy(&sCfgIns.sAdvancedPara, pGDispatch, sizeof(SGuiPtzAdvancedPara));
			
			rtn = BizSetPara(&sParaTgtIns, &sCfgIns);
			
		}
	}
	
	return rtn;
}

// 网络配置参数存取/网络字码流编码参数存取
// 
int bizData_GetNetParaDefault(SGuiNetPara* pGNet)
{
	s32 rtn;
	SBizParaTarget sParaTgtIns;
	SBizNetPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_NETPARA;
	if(pGNet)
	{
		rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			memcpy(pGNet, &sCfgIns, sizeof(SGuiNetPara));
		}
	}
	
	return rtn;
}

void bizData_GetNetPara(SGuiNetPara* pGNet)
{
	s32 rtn;
	SBizParaTarget sParaTgtIns;
	SBizNetPara sCfgIns;
	u8 nIdx = 0;//u8 nIdx;//csp modify
	
	//printf("bizData_GetNetPara - BizNetReadCfg %d\n", __LINE__);
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_NETPARA;
	if(pGNet)
	{
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_DDNSIPUPDATEITVL,
						sCfgIns.UpdateIntvl,
						&nIdx)
			)
			{
				//printf("bizData_GetNetPara UpdateIntvl=%d nIdx=%d\n",sCfgIns.UpdateIntvl,nIdx);
				sCfgIns.UpdateIntvl = nIdx;
			}
			else
			{
				//printf("BizConfigGetParaListIndex failed,UpdateIntvl=%d nIdx=%d\n",sCfgIns.UpdateIntvl,nIdx);
			}
			
			/*
			if(sCfgIns.DhcpFlag)
			{
				SBizNetPara sPIns;

				if( 0==BizNetReadCfg(&sPIns) )
				{					
					sCfgIns.HostIP = sPIns.HostIP;                 			// IP 地址
					sCfgIns.Submask = sPIns.Submask;                		// 子网掩码
					sCfgIns.GateWayIP = sPIns.GateWayIP;              	// 网关 IP
					sCfgIns.DNSIP = sPIns.DNSIP;                  			// DNS IP
					memcpy( sCfgIns.MAC, sPIns.MAC, sizeof(sPIns.MAC) );	// MAC Address
				}
			}
			*/
			
			memcpy(pGNet, &sCfgIns, sizeof(SGuiNetPara));
		}
	}
	
	return;
}

s32  bizData_SaveNetPara(SGuiNetPara* pGNet)
{
	s32 rtn = 0;
	SBizParaTarget sParaTgtIns;
	SBizNetPara sCfgIns;
	s32 nVal = 0;//s32 nVal;//csp modify
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_NETPARA;
	if(pGNet)
	{
		memcpy(&sCfgIns, pGNet, sizeof(SGuiNetPara));
		
		if( 0==BizConfigGetParaListValue(
					EM_BIZ_CFG_PARALIST_DDNSIPUPDATEITVL,
					sCfgIns.UpdateIntvl,
					&nVal
			) 
		)
		{
			//printf("bizData_SaveNetPara UpdateIntvl=%d nVal=%d\n",sCfgIns.UpdateIntvl,nVal);
			sCfgIns.UpdateIntvl = nVal;
		}
		
		BizSetPara(&sParaTgtIns, &sCfgIns);
	}
	
	return rtn;
}

void bizData_SaveSensorAlarmPara(int ch, SSensorAlarmPara* pGAlarm)
{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns;	
		SBizAlarmSensorPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
		sParaTgtIns.nChn = ch;
		s32 nVal;
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			sCfgIns.nEnable = pGAlarm->bEnable;
			
			if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_SENSORTYPE,
						pGAlarm->nAlarmType,
						&nVal
				) 
			)			
			sCfgIns.nType = nVal;
			
			strcpy( sCfgIns.name, pGAlarm->szName );//*******
			
			if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_DELAY,
						pGAlarm->nDelayTime,
						&nVal
				) 
			)			
			sCfgIns.nDelay = nVal;

			BizSetPara(&sParaTgtIns, &sCfgIns);
			
			return;
		}
	}
}

void bizData_GetSensorAlarmDefault(int ch, SSensorAlarmPara* pGAlarm)
{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns;	
		SBizAlarmSensorPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
		sParaTgtIns.nChn = ch;
		u8 	nIdx;
		rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			pGAlarm->bEnable = sCfgIns.nEnable;
			
			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_SENSORTYPE,
						sCfgIns.nType,
						&nIdx
				) 
			)
			pGAlarm->nAlarmType = nIdx;		
				
			strcpy( pGAlarm->szName, sCfgIns.name );

			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_DELAY,
						sCfgIns.nDelay,
						&nIdx
				) 
			)
				pGAlarm->nDelayTime = nIdx;
			
			return;
		}
	}
}

void bizData_GetSensorAlarmPara(int ch, SSensorAlarmPara* pGAlarm)
{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns;	
		SBizAlarmSensorPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
		sParaTgtIns.nChn = ch;
		u8 nIdx;
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			pGAlarm->bEnable = sCfgIns.nEnable;
			
			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_SENSORTYPE,
						sCfgIns.nType,
						&nIdx
				) 
			)
				pGAlarm->nAlarmType = nIdx;
				
				
			strcpy( pGAlarm->szName, sCfgIns.name );

			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_DELAY,
						sCfgIns.nDelay,
						&nIdx
				) 
			)
				pGAlarm->nDelayTime = nIdx;
			
			return;
		}
	}	
}

//yaogang modify 20141010
//IPCCOVER
void bizData_SaveIPCCoverAlarmPara(int ch, SIPCExt_SensorAlarmPara* pGAlarm)
{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns; 
		SBizAlarmIPCExtSensorPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_IPCCOVERPARA;
		sParaTgtIns.nChn = ch;
		s32 nVal;
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			sCfgIns.nEnable = pGAlarm->bEnable;
					
			if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_DELAY,
						pGAlarm->nDelayTime,
						&nVal
				) 
			)			
			sCfgIns.nDelay = nVal;
			//if (ch == 0)
				//printf("yg Save nDelayTime index: %d\n", sCfgIns.nDelay);

			BizSetPara(&sParaTgtIns, &sCfgIns);
			
			return;
		}
	}
}

void bizData_GetIPCCoverAlarmDefault(int ch, SIPCExt_SensorAlarmPara* pGAlarm)
{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns;	
		SBizAlarmIPCExtSensorPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_IPCCOVERPARA;
		sParaTgtIns.nChn = ch;
		u8 	nIdx;
		rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			pGAlarm->bEnable = sCfgIns.nEnable;

			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_DELAY,
						sCfgIns.nDelay,
						&nIdx
				) 
			)
			pGAlarm->nDelayTime = nIdx;
			//if (ch == 0)
				//printf("yg get Default nDelayTime index: %d\n", nIdx);
			
			return;
		}
	}
}

void bizData_GetIPCCoverAlarmPara(int ch, SIPCExt_SensorAlarmPara* pGAlarm)

{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns;	
		SBizAlarmIPCExtSensorPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_IPCCOVERPARA;
		sParaTgtIns.nChn = ch;
		u8 nIdx;

		//if (ch == 0)
			//printf("yg bizData_GetIPCExtSensorAlarmPara chn0\n");
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			pGAlarm->bEnable = sCfgIns.nEnable;

			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_DELAY,
						sCfgIns.nDelay,
						&nIdx
				) 
			)
				pGAlarm->nDelayTime = nIdx;
			//if (ch == 0)
				//printf("yg get nDelayTime index: %d\n", nIdx);
			
			return;
		}
	}	
}


//IPCExtSensor
void bizData_SaveIPCExtSensorAlarmPara(int ch, SIPCExt_SensorAlarmPara* pGAlarm)
{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns; 
		SBizAlarmIPCExtSensorPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_IPCEXTSENSORPARA;
		sParaTgtIns.nChn = ch;
		s32 nVal;
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			sCfgIns.nEnable = pGAlarm->bEnable;
					
			if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_DELAY,
						pGAlarm->nDelayTime,
						&nVal
				) 
			)			
			sCfgIns.nDelay = nVal;
			if (ch == 0)
				printf("yg Save nEnable: %d, nDelayTime index: %d\n", sCfgIns.nEnable, sCfgIns.nDelay);

			BizSetPara(&sParaTgtIns, &sCfgIns);
			
			return;
		}
	}
}

void bizData_GetIPCExtSensorAlarmDefault(int ch, SIPCExt_SensorAlarmPara* pGAlarm)
{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns;	
		SBizAlarmIPCExtSensorPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_IPCEXTSENSORPARA;
		sParaTgtIns.nChn = ch;
		u8 	nIdx;
		rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			pGAlarm->bEnable = sCfgIns.nEnable;

			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_DELAY,
						sCfgIns.nDelay,
						&nIdx
				) 
			)
			pGAlarm->nDelayTime = nIdx;
			//if (ch == 0)
				//printf("yg get Default nDelayTime index: %d\n", nIdx);
			
			return;
		}
	}
}

void bizData_GetIPCExtSensorAlarmPara(int ch, SIPCExt_SensorAlarmPara* pGAlarm)

{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns;	
		SBizAlarmIPCExtSensorPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_IPCEXTSENSORPARA;
		sParaTgtIns.nChn = ch;
		u8 nIdx;

		//if (ch == 0)
			//printf("yg bizData_GetIPCExtSensorAlarmPara chn0\n");
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			pGAlarm->bEnable = sCfgIns.nEnable;

			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_DELAY,
						sCfgIns.nDelay,
						&nIdx
				) 
			)
				pGAlarm->nDelayTime = nIdx;
			//if (ch == 0)
				//printf("yg get nDelayTime index: %d\n", nIdx);
			
			return;
		}
	}	
}

//485EXT ALARM
void bizData_Save485ExtSensorAlarmPara(int ch, SIPCExt_SensorAlarmPara* pGAlarm)
{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns; 
		SBizAlarmIPCExtSensorPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_485EXTSENSORPARA;
		sParaTgtIns.nChn = ch;
		s32 nVal;
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			sCfgIns.nEnable = pGAlarm->bEnable;
					
			if( 0==BizConfigGetParaListValue(
						EM_BIZ_CFG_PARALIST_DELAY,
						pGAlarm->nDelayTime,
						&nVal
				) 
			)		
			if (ch == 0)
				printf("yg Save nDelayTime value: %d, old value:%d\n", nVal, sCfgIns.nDelay);
			
			sCfgIns.nDelay = nVal;

			BizSetPara(&sParaTgtIns, &sCfgIns);
			
			return;
		}
	}
}

void bizData_Get485ExtSensorAlarmDefault(int ch, SIPCExt_SensorAlarmPara* pGAlarm)
{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns;	
		SBizAlarmIPCExtSensorPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_485EXTSENSORPARA;
		sParaTgtIns.nChn = ch;
		u8 	nIdx;
		rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			pGAlarm->bEnable = sCfgIns.nEnable;

			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_DELAY,
						sCfgIns.nDelay,
						&nIdx
				) 
			)
			pGAlarm->nDelayTime = nIdx;
			//if (ch == 0)
				//printf("yg get Default nDelayTime index: %d\n", nIdx);
			
			return;
		}
	}
}

void bizData_Get485ExtSensorAlarmPara(int ch, SIPCExt_SensorAlarmPara* pGAlarm)
{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns;	
		SBizAlarmIPCExtSensorPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_485EXTSENSORPARA;
		sParaTgtIns.nChn = ch;
		u8 nIdx;

		//if (ch == 0)
			//printf("yg bizData_GetIPCExtSensorAlarmPara chn0\n");
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			pGAlarm->bEnable = sCfgIns.nEnable;

			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_DELAY,
						sCfgIns.nDelay,
						&nIdx
				) 
			)
				pGAlarm->nDelayTime = nIdx;
			//if (ch == 0)
				//printf("yg get nDelayTime index: %d\n", nIdx);
			
			return;
		}
	}	
}

//HDD ALARM
void bizData_SaveHDDAlarmPara(SHDDAlarmPara* pGAlarm)
{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns; 
		SBizAlarmHDDPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_HDDPARA;
		sParaTgtIns.nChn = 0;
		s32 nVal;
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			printf("yg BizGetPara enable: %d\n", sCfgIns.nEnable);
			sCfgIns.nEnable = pGAlarm->bEnable;
			printf("yg  enable: %d\n", sCfgIns.nEnable);
			
			BizSetPara(&sParaTgtIns, &sCfgIns);
			
			return;
		}
	}
}

void bizData_GetHDDAlarmDefault(SHDDAlarmPara* pGAlarm)
{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns;	
		SBizAlarmHDDPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_HDDPARA;
		sParaTgtIns.nChn = 0;
		u8 	nIdx;
		rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			pGAlarm->bEnable = sCfgIns.nEnable;
			
			return;
		}
	}
}

void bizData_GetHDDAlarmPara(SHDDAlarmPara* pGAlarm)
{
	int rtn = 0;

	if(pGAlarm)
	{
		SBizParaTarget sParaTgtIns;	
		SBizAlarmHDDPara sCfgIns;
	
		memset(&sCfgIns, 0, sizeof(sCfgIns));
		sParaTgtIns.emBizParaType = EM_BIZ_HDDPARA;
		sParaTgtIns.nChn = 0;
		u8 nIdx;

		//if (ch == 0)
			//printf("yg bizData_GetIPCExtSensorAlarmPara chn0\n");
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			pGAlarm->bEnable = sCfgIns.nEnable;
			
			return;
		}
	}	
}


//yaogang modify 20141010 end
//yaogang modify 20150402 mobile stream
int bizData_GetMobStreamParaDefault(uchar nChn, SGuiNetStreamPara* pGNet)
{
	s32 rtn;
	SBizParaTarget sParaTgtIns;
	SBizEncodePara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_ENCMOBPARA;//手机码流编码参数SBizEncodePara;
	sParaTgtIns.nChn = nChn;
	if(pGNet)
	{
		rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			memcpy(pGNet, &sCfgIns, sizeof(SGuiNetStreamPara));

			u8 nIndex = 0;
			//printf("%s 1 pGNet->nVideoResolution: %d\n", __func__, pGNet->nVideoResolution);
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_VMOBRESOL, // 手机码流通用支持的分辨率列表
				pGNet->nVideoResolution, 
				&nIndex  
			) )
			{
				pGNet->nVideoResolution	= nIndex;
			}
			//printf("%s 2 pGNet->nVideoResolution: %d\n", __func__, pGNet->nVideoResolution);
			
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			SBizSystemPara bizSysPara;
			BizGetPara(&sParaTgtIns, &bizSysPara);
			if( 0==BizConfigGetParaListIndex(
				bizSysPara.nVideoStandard == EM_BIZ_PAL ?
					EM_BIZ_CFG_PARALIST_FPSPALCIF: EM_BIZ_CFG_PARALIST_FPSNTSCCIF,
				pGNet->nFrameRate, 
				&nIndex  
			) )
			{
				pGNet->nFrameRate = nIndex;
			}
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_BITTYPE, 
				pGNet->nBitRateType, 
				&nIndex  
			) )
			{
				pGNet->nBitRateType = nIndex;
			}
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_PICLEVEL, 
				pGNet->nPicLevel, 
				&nIndex  
			) )
			{
				pGNet->nPicLevel = nIndex;
			}
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_BITRATE, 
				pGNet->nBitRate, 
				&nIndex  
			) )
			{
				pGNet->nBitRate = nIndex;
			}
		}		
	}
	
	return rtn;
}

void bizData_GetMobStreamPara(uchar nChn, SGuiNetStreamPara* pGNet)
{
	s32 rtn;
	SBizParaTarget sParaTgtIns;
	SBizEncodePara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_ENCMOBPARA;
	sParaTgtIns.nChn = nChn;
	if(pGNet)
	{
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			memcpy(pGNet, &sCfgIns, sizeof(SGuiNetStreamPara));
			
			//printf("bizData_GetNetStreamPara fr %d\n", sCfgIns.nFrameRate);
#if 1
			u8 nIndex = 0;

			//if (nChn == 0)
				//printf("%s 1 pGNet->nVideoResolution: %d\n", __func__, pGNet->nVideoResolution);
			
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_VMOBRESOL, 
				pGNet->nVideoResolution, 
				&nIndex  
			) )
			{
				pGNet->nVideoResolution	= nIndex;
			}
			
			//if (nChn == 0)
				//printf("%s 2 pGNet->nVideoResolution: %d\n", __func__, pGNet->nVideoResolution);
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			SBizSystemPara bizSysPara;
			BizGetPara(&sParaTgtIns, &bizSysPara);
			
			if( 0==BizConfigGetParaListIndex(
				bizSysPara.nVideoStandard == EM_BIZ_PAL ?
					EM_BIZ_CFG_PARALIST_FPSPALCIF: EM_BIZ_CFG_PARALIST_FPSNTSCCIF,
				pGNet->nFrameRate, 
				&nIndex  
			) )
			{
				pGNet->nFrameRate = nIndex;
			}
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_BITTYPE, 
				pGNet->nBitRateType, 
				&nIndex  
			) )
			{
				pGNet->nBitRateType = nIndex;
			}
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_PICLEVEL, 
				pGNet->nPicLevel, 
				&nIndex  
			) )
			{
				pGNet->nPicLevel = nIndex;
			}
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_BITRATE, 
				pGNet->nBitRate, 
				&nIndex  
			) )
			{
				pGNet->nBitRate = nIndex;
			}

			//printf("BizGetPara vr %d fr %d br %d\n", pGNet->nVideoResolution, pGNet->nFrameRate, pGNet->nBitRate);
#endif
		}
	}
	
	return;
}


void bizData_SaveMobStreamPara(uchar nChn, SGuiNetStreamPara* pGNet)
{	
	s32 rtn;
	SBizParaTarget sParaTgtIns;
	SBizEncodePara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_ENCMOBPARA;
	sParaTgtIns.nChn = nChn;
	if(pGNet)
	{
		if( 0 == BizGetPara(&sParaTgtIns, &sCfgIns) )
		{
			sCfgIns.nVideoResolution 	= pGNet->nVideoResolution;
			sCfgIns.nFrameRate 			= pGNet->nFrameRate;
			sCfgIns.nBitRateType 		= pGNet->nBitRateType;
			sCfgIns.nPicLevel 			= pGNet->nPicLevel;
			sCfgIns.nBitRate 			= pGNet->nBitRate;
			
			s32 nValue = 0;

			//if (nChn == 0)
				//printf("%s 1 pGNet->nVideoResolution: %d\n", __func__, pGNet->nVideoResolution);
			
			if( 0==BizConfigGetParaListValue(
				EM_BIZ_CFG_PARALIST_VMOBRESOL, 
				sCfgIns.nVideoResolution, 
				&nValue
			) )
			{
				sCfgIns.nVideoResolution	= nValue;
			}
			//if (nChn == 0)
				//printf("%s 2 pGNet->nVideoResolution: %d\n", __func__, sCfgIns.nVideoResolution);
			
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			SBizSystemPara bizSysPara;
			BizGetPara(&sParaTgtIns, &bizSysPara);
			if( 0==BizConfigGetParaListValue(
				bizSysPara.nVideoStandard == EM_BIZ_PAL ?
					EM_BIZ_CFG_PARALIST_FPSPALCIF: EM_BIZ_CFG_PARALIST_FPSNTSCCIF,
				sCfgIns.nFrameRate, 
				&nValue  
			) )
			{
				sCfgIns.nFrameRate = nValue;
			}
			
			if( 0==BizConfigGetParaListValue(
				EM_BIZ_CFG_PARALIST_BITTYPE, 
				sCfgIns.nBitRateType, 
				&nValue  
			) )
			{
				sCfgIns.nBitRateType = nValue;
			}
			if( 0==BizConfigGetParaListValue(
				EM_BIZ_CFG_PARALIST_PICLEVEL, 
				sCfgIns.nPicLevel, 
				&nValue  
			) )
			{
				sCfgIns.nPicLevel = nValue;
			}
			if( 0==BizConfigGetParaListValue(
				EM_BIZ_CFG_PARALIST_BITRATE, 
				sCfgIns.nBitRate, 
				&nValue  
			) )
			{
				sCfgIns.nBitRate = nValue;
			}
			
			sParaTgtIns.emBizParaType = EM_BIZ_ENCMOBPARA;
			BizSetPara(&sParaTgtIns, &sCfgIns);
		}
	}
	
	return;
}


//yaogang modify 20150402 mobile stream end

int bizData_GetNetStreamParaDefault(uchar nChn, SGuiNetStreamPara* pGNet)
{
	s32 rtn;
	SBizParaTarget sParaTgtIns;
	SBizEncodePara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_ENCSUBPARA;
	sParaTgtIns.nChn = nChn;
	if(pGNet)
	{
		rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			memcpy(pGNet, &sCfgIns, sizeof(SGuiNetStreamPara));

			u8 nIndex = 0;
			
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_VSUBRESOL, 
				pGNet->nVideoResolution, 
				&nIndex  
			) )
			{
				pGNet->nVideoResolution	= nIndex;
			}
			
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			SBizSystemPara bizSysPara;
			BizGetPara(&sParaTgtIns, &bizSysPara);
			if( 0==BizConfigGetParaListIndex(
				bizSysPara.nVideoStandard == EM_BIZ_PAL ?
					EM_BIZ_CFG_PARALIST_FPSPALCIF: EM_BIZ_CFG_PARALIST_FPSNTSCCIF,
				pGNet->nFrameRate, 
				&nIndex  
			) )
			{
				pGNet->nFrameRate = nIndex;
			}
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_BITTYPE, 
				pGNet->nBitRateType, 
				&nIndex  
			) )
			{
				pGNet->nBitRateType = nIndex;
			}
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_PICLEVEL, 
				pGNet->nPicLevel, 
				&nIndex  
			) )
			{
				pGNet->nPicLevel = nIndex;
			}
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_BITRATE, 
				pGNet->nBitRate, 
				&nIndex  
			) )
			{
				pGNet->nBitRate = nIndex;
			}
		}		
	}
	
	return rtn;
}



void bizData_GetNetStreamPara(uchar nChn, SGuiNetStreamPara* pGNet)
{
	s32 rtn;
	SBizParaTarget sParaTgtIns;
	SBizEncodePara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_ENCSUBPARA;
	sParaTgtIns.nChn = nChn;
	if(pGNet)
	{
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0==rtn)
		{
			memcpy(pGNet, &sCfgIns, sizeof(SGuiNetStreamPara));
			
			//printf("bizData_GetNetStreamPara fr %d\n", sCfgIns.nFrameRate);
#if 1
			u8 nIndex = 0;

			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_VSUBRESOL, 
				pGNet->nVideoResolution, 
				&nIndex  
			) )
			{
				pGNet->nVideoResolution	= nIndex;
			}
			
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			SBizSystemPara bizSysPara;
			BizGetPara(&sParaTgtIns, &bizSysPara);
			
			if( 0==BizConfigGetParaListIndex(
				bizSysPara.nVideoStandard == EM_BIZ_PAL ?
					EM_BIZ_CFG_PARALIST_FPSPALCIF: EM_BIZ_CFG_PARALIST_FPSNTSCCIF,
				pGNet->nFrameRate, 
				&nIndex  
			) )
			{
				pGNet->nFrameRate = nIndex;
			}
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_BITTYPE, 
				pGNet->nBitRateType, 
				&nIndex  
			) )
			{
				pGNet->nBitRateType = nIndex;
			}
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_PICLEVEL, 
				pGNet->nPicLevel, 
				&nIndex  
			) )
			{
				pGNet->nPicLevel = nIndex;
			}
			if( 0==BizConfigGetParaListIndex(
				EM_BIZ_CFG_PARALIST_BITRATE, 
				pGNet->nBitRate, 
				&nIndex  
			) )
			{
				pGNet->nBitRate = nIndex;
			}

			//printf("BizGetPara vr %d fr %d br %d\n", pGNet->nVideoResolution, pGNet->nFrameRate, pGNet->nBitRate);
#endif
		}
	}
	
	return;
}


void bizData_SaveNetStreamPara(uchar nChn, SGuiNetStreamPara* pGNet)
{	
	s32 rtn;
	SBizParaTarget sParaTgtIns;
	SBizEncodePara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_ENCSUBPARA;
	sParaTgtIns.nChn = nChn;
	if(pGNet)
	{
		if( 0 == BizGetPara(&sParaTgtIns, &sCfgIns) )
		{
			sCfgIns.nVideoResolution 	= pGNet->nVideoResolution;
			sCfgIns.nFrameRate 			= pGNet->nFrameRate;
			sCfgIns.nBitRateType 		= pGNet->nBitRateType;
			sCfgIns.nPicLevel 			= pGNet->nPicLevel;
			sCfgIns.nBitRate 			= pGNet->nBitRate;
			
			s32 nValue = 0;
			
			if( 0==BizConfigGetParaListValue(
				EM_BIZ_CFG_PARALIST_VSUBRESOL, 
				sCfgIns.nVideoResolution, 
				&nValue
			) )
			{
				sCfgIns.nVideoResolution	= nValue;
			}
			
			sParaTgtIns.emBizParaType = EM_BIZ_SYSTEMPARA;
			SBizSystemPara bizSysPara;
			BizGetPara(&sParaTgtIns, &bizSysPara);
			if( 0==BizConfigGetParaListValue(
				bizSysPara.nVideoStandard == EM_BIZ_PAL ?
					EM_BIZ_CFG_PARALIST_FPSPALCIF: EM_BIZ_CFG_PARALIST_FPSNTSCCIF,
				sCfgIns.nFrameRate, 
				&nValue  
			) )
			{
				sCfgIns.nFrameRate = nValue;
			}
			
			if( 0==BizConfigGetParaListValue(
				EM_BIZ_CFG_PARALIST_BITTYPE, 
				sCfgIns.nBitRateType, 
				&nValue  
			) )
			{
				sCfgIns.nBitRateType = nValue;
			}
			if( 0==BizConfigGetParaListValue(
				EM_BIZ_CFG_PARALIST_PICLEVEL, 
				sCfgIns.nPicLevel, 
				&nValue  
			) )
			{
				sCfgIns.nPicLevel = nValue;
			}
			if( 0==BizConfigGetParaListValue(
				EM_BIZ_CFG_PARALIST_BITRATE, 
				sCfgIns.nBitRate, 
				&nValue  
			) )
			{
				sCfgIns.nBitRate = nValue;
			}
			
			sParaTgtIns.emBizParaType = EM_BIZ_ENCSUBPARA;
			BizSetPara(&sParaTgtIns, &sCfgIns);
		}
	}
	
	return;
}

void bizData_SaveMDAlarmPara(int ch, SMDCfg* pGAlarm)
{
	int rtn;
	SBizParaTarget sParaTgtIns;
	SBizAlarmVMotionPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	
	sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
	sParaTgtIns.nChn = ch;
	rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0 == rtn)
	{
		s32 nVal;
		sCfgIns.nEnable = pGAlarm->bEnable;
		
		if( 0==BizConfigGetParaListValue(
					EM_BIZ_CFG_PARALIST_DELAY,
					pGAlarm->nDelay,
					&nVal
			) 
		)
		{
			sCfgIns.nDelay = nVal;
            BizSetPara(&sParaTgtIns, &sCfgIns);
		}

		//printf("##MD en %d dy %d\n", sCfgIns.nEnable, sCfgIns.nDelay);
		
		//BizSetPara(&sParaTgtIns, &sCfgIns);
	}
}

void bizData_GetMDAlarmDefault(int ch, SMDCfg* pGAlarm)
{
	int rtn;
	SBizParaTarget sParaTgtIns;
	SBizAlarmVMotionPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	
	sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
	sParaTgtIns.nChn = ch;
	rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
	if(0 == rtn)
	{
		u8 nIdx;
		pGAlarm->bEnable = sCfgIns.nEnable;
		
		if( 0==BizConfigGetParaListIndex(
					EM_BIZ_CFG_PARALIST_DELAY,
					sCfgIns.nDelay,
					&nIdx
			) 
		)
		{
			pGAlarm->nDelay = nIdx;
		}
	}
}

void bizData_GetMDAlarmPara(int ch, SMDCfg* pGAlarm)
{
	int rtn;
	SBizParaTarget sParaTgtIns;
	SBizAlarmVMotionPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	
	sParaTgtIns.emBizParaType = EM_BIZ_VMOTIONPARA;
	sParaTgtIns.nChn = ch;
	rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(0 == rtn)
	{
		u8 nIdx;
		pGAlarm->bEnable = sCfgIns.nEnable;
		
		if( 0==BizConfigGetParaListIndex(
					EM_BIZ_CFG_PARALIST_DELAY,
					sCfgIns.nDelay,
					&nIdx
			) 
		)
		{
			pGAlarm->nDelay = nIdx;
		}
		
		//printf("##MD en %d dy %d\n", sCfgIns.nEnable, sCfgIns.nDelay);
	}
}

int GetCifEncCap() 
{
	// get resolution
	// get enc cap
	
	return 16*25; 
}

int GetFrameRateVal(int nResol, int nIdx)
{
	EMBIZCFGPARALIST eType;
	s32 nVal = 0;
	
	if(nResol==2)
	{
		eType = EM_BIZ_CFG_PARALIST_FPSPALD1;
	}
	else
		eType = EM_BIZ_CFG_PARALIST_FPSPAL;
		
	if( 0==BizConfigGetParaListValue(
			eType,
			nIdx,
			&nVal
		) )
	{
		return nVal;
	}
	else
		return -1;
}

//yaogang modify 20141127
s32 BizGUiWriteLogWithChn(u8 chn, s8 master, s8 slave)
{
	SBizLogInfo info;
	memset(&info, 0, sizeof(info));
	info.nMasterType = master;
	info.nSlaveType = slave;
	info.nchn = chn;
	strcpy(info.aIp, "GUI");
	strcpy(info.aUsername, GetGUILoginedUserName());
	if (slave == BIZ_LOG_SLAVE_DISK_LOST)
	{
		printf("yg BizGUiWriteLogWithChn: BIZ_LOG_SLAVE_DISK_LOST\n");
	}
	return BizWriteLog(&info);
}


s32 BizGUiWriteLog(s8 master, s8 slave)
{
	SBizLogInfo info;
	memset(&info, 0, sizeof(info));
	info.nMasterType = master;
	info.nSlaveType = slave;
	strcpy(info.aIp, "GUI");
	strcpy(info.aUsername, GetGUILoginedUserName());
	return BizWriteLog(&info);
}

char* bizData_inet_ntoa( u32 nIP )
{
	struct in_addr addr;

	memcpy( &addr, &nIP, 4 );

	return inet_ntoa(addr);
}

static sem_t* mouseSem = NULL;

void WaitForSem()
{
	if(mouseSem == NULL)
	{
		mouseSem = (sem_t*)malloc(sizeof(sem_t));
		sem_init(mouseSem,0,1);
	}
	
	//static int count = 0;
	sem_wait(mouseSem);
}

void ReleaseSem()
{
	//static int count = 0;
	//printf("sem_post(mouseSem) #########################  %d \n",count++);
	sem_post(mouseSem);
}

static int g_open = 0;

//u64 GetTimeTick()
//{
//	struct timeval time;
//	gettimeofday(&time, NULL);
//	
//	u64 millSec = (u64)time.tv_sec*1000+time.tv_usec/1000;
//	
//	return millSec;
//}

u8 GetHardDiskNum()
{
    HDDHDR hddHdr = PublicGetHddManage();
	u8 num = 0;
	if(hddHdr)
	{
		for (int i = 0; i < MAX_HDD_NUM; ++i) 
		{
			if ((((disk_manager *)hddHdr)->hinfo[i].is_disk_exist) &&
	            (((disk_manager *)hddHdr)->hinfo[i].storage_type != 'u')) 
	        {
				num++;
			}
		}
	}
	return num;
}

//Modify by Lirl on Jan/05/2012,删除目录
void delDir(char * path)
{
    #define MAXDIR 256
    
    char dirName[MAXDIR], rootDir[MAXDIR];
    DIR* dp = NULL;
    DIR* dpin = NULL;
    char *pathName = (char*)malloc(MAXDIR);
    struct dirent* dirp;
    
    dp = opendir(path);
    if(dp == NULL)
    {
        printf("The directory is not exist!\n");
        return;
    }
    
    while((dirp = readdir(dp)) != NULL)
    {
        if(strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0)
        {
            continue;
        }
        
        strcpy(pathName, path);
        strcat(pathName, "/");
        strcat(pathName, dirp->d_name);
        
        dpin = opendir(pathName);
        if(dpin != NULL)
        {
            delDir(pathName); //递归删除子目录
        }
        else
        {
            remove(pathName); //为文件
        }
        
        strcpy(pathName, "");
        closedir(dpin);
        dpin = NULL;
    }

    rmdir(path);
    closedir(dp);
    free(pathName);
    pathName = NULL;
    dirp = NULL;
}

//Modify by Lirl on Jan/05/2012
void rmMemData()
{
	#define DELFILE "/tmp/ui/data/Data/Font.bin"
	#define DELDIR  "/tmp/ui/data/Data/temp/"
	#define DELDIR2 "/lib/modules/"
	
	//csp modify 20130729
	//测试文件是否存在
	if(0 == access(DELFILE, 0))
	{
		remove(DELFILE);
	}
	
	return;//csp modify
	
	//测试目录是否存在
	if(0 == access(DELDIR, 0))
	{
		delDir(DELDIR);
	}
	
	if(0 == access(DELDIR2, 0))
	{
		delDir(DELDIR2);
	}
}


typedef struct
{
	std::string type;
	std::string detail;
}SAlarmListItem;

typedef struct
{
	u8 current;
	u8 still_stamp;//状态持续戳，直到用户查看过报警信息
} SAlarmStatus;

class CAlarmList:public CObject
{
public:
	CAlarmList();
	~CAlarmList();
	
	void RefreshAlarmList(u8 page);
	void RefreshAllStatus();
	
public:
	SAlarmStatus m_byAlarmEvent[EM_ALRM_TYPES];
	SAlarmStatus m_byVLoss[32];
	SAlarmStatus m_byVMotion[32];
	SAlarmStatus m_bySensor[72];
	SAlarmStatus m_byIPCExtSensor[32];
	SAlarmStatus m_by485ExtSensor[32];
	SAlarmStatus m_byIPCCover[32];
	SAlarmStatus m_byDiskLost[MAX_HDD_NUM];
	SAlarmStatus m_byDiskErr[MAX_HDD_NUM];
	SAlarmStatus m_byDiskNone;
	/*
	u8 m_byAlarmEvent[EM_ALRM_TYPES];
	u8 m_byVLoss[32];
	u8 m_byVMotion[32];
	u8 m_bySensor[72];
	u8 m_byIPCExtSensor[32];
	u8 m_byIPCCover[32];
	*/
	
	std::vector<SAlarmListItem> m_vCurrItems;
	
	CMutex m_Mutex;
	BOOL m_bChanged;
	int m_nPage;
};

static CTimer g_cAlarmListTimer;
static CAlarmList g_cAlarmListObj;

static u8 g_nAlarmListPage = 0;

void AlarmListAdd(u8 chanel,ALARMLISTTYPE cmd,u8 status)
{
	if(cmd >= EM_ALRM_TYPES || cmd < 0)
	{
		return;
	}
	
	g_cAlarmListObj.m_Mutex.Enter();
	
	if(cmd == EM_ALRM_ALARM_INPUT)
	{
		if(chanel >= GetSenSorNum())
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		
		if(!g_cAlarmListObj.m_bySensor[chanel].current)
		{
			if (!g_cAlarmListObj.m_bySensor[chanel].still_stamp)
			{
				g_cAlarmListObj.m_bChanged = TRUE;
				g_cAlarmListObj.m_bySensor[chanel].still_stamp = 1;
			}
		}
		
		g_cAlarmListObj.m_bySensor[chanel].current= 1;
	}
	else if(cmd == EM_ALRM_MOTION_DETECTION)
	{
		if(chanel >= GetVideoMainNum())
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		
		if(!g_cAlarmListObj.m_byVMotion[chanel].current)
		{
			if (!g_cAlarmListObj.m_byVMotion[chanel].still_stamp)
			{
				g_cAlarmListObj.m_bChanged = TRUE;
				g_cAlarmListObj.m_byVMotion[chanel].still_stamp = 1;
			}
		}
		
		g_cAlarmListObj.m_byVMotion[chanel].current= 1;
	}
	else if(cmd == EM_ALRM_VEDIO_LOST)
	{
		if(chanel >= GetVideoMainNum())
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		
		if(!g_cAlarmListObj.m_byVLoss[chanel].current)
		{
			if (!g_cAlarmListObj.m_byVLoss[chanel].still_stamp)
			{
				g_cAlarmListObj.m_bChanged = TRUE;
				g_cAlarmListObj.m_byVLoss[chanel].still_stamp = 1;
			}
		}
		
		g_cAlarmListObj.m_byVLoss[chanel].current= 1;
	}
	else if(cmd == EM_ALRM_ALARM_IPCEXT)
	{
		printf("m_byIPCExtSensor[%d].current: %d\n", chanel, g_cAlarmListObj.m_byIPCExtSensor[chanel].current);
		if(chanel >= GetVideoMainNum())
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		
		if(!g_cAlarmListObj.m_byIPCExtSensor[chanel].current)
		{
			printf("chn%d IPCExtSensor current set, still_stamp: %d\n", chanel, g_cAlarmListObj.m_byIPCExtSensor[chanel].still_stamp);
			if (!g_cAlarmListObj.m_byIPCExtSensor[chanel].still_stamp)
			{
				g_cAlarmListObj.m_bChanged = TRUE;
				g_cAlarmListObj.m_byIPCExtSensor[chanel].still_stamp = 1;
				printf("chn%d IPCExtSensor still_stamp set\n", chanel);
			}
		}
		
		g_cAlarmListObj.m_byIPCExtSensor[chanel].current= 1;
	}
	else if(cmd == EM_ALRM_ALARM_IPCCOVER)
	{
		if(chanel >= GetVideoMainNum())
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		
		if(!g_cAlarmListObj.m_byIPCCover[chanel].current)
		{
			if (!g_cAlarmListObj.m_byIPCCover[chanel].still_stamp)
			{
				g_cAlarmListObj.m_bChanged = TRUE;
				g_cAlarmListObj.m_byIPCCover[chanel].still_stamp = 1;
			}
		}
		
		g_cAlarmListObj.m_byIPCCover[chanel].current = 1;
	}
	else if(cmd == EM_ALRM_DISK_LOST)
	{
		if(chanel >= MAX_HDD_NUM)
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		
		if(!g_cAlarmListObj.m_byDiskLost[chanel].current)
		{
			if (!g_cAlarmListObj.m_byDiskLost[chanel].still_stamp)
			{
				g_cAlarmListObj.m_bChanged = TRUE;
				g_cAlarmListObj.m_byDiskLost[chanel].still_stamp = 1;
				printf("sata%d DiskLost current set\n", chanel);
			}
		}
		
		g_cAlarmListObj.m_byDiskLost[chanel].current = 1;
	}
	else if(cmd == EM_ALRM_DISK_WRERR)
	{
		if(chanel >= MAX_HDD_NUM)
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		
		if(!g_cAlarmListObj.m_byDiskErr[chanel].current)
		{
			if (!g_cAlarmListObj.m_byDiskErr[chanel].still_stamp)
			{
				g_cAlarmListObj.m_bChanged = TRUE;
				g_cAlarmListObj.m_byDiskErr[chanel].still_stamp = 1;
				printf("sata%d DiskErr current set\n", chanel);
			}
		}
		
		g_cAlarmListObj.m_byDiskErr[chanel].current = 1;
	}
	else if(cmd == EM_ALRM_DISK_NONE)
	{
		if(!g_cAlarmListObj.m_byDiskNone.current)
		{
			if (!g_cAlarmListObj.m_byDiskNone.still_stamp)
			{
				g_cAlarmListObj.m_bChanged = TRUE;
				g_cAlarmListObj.m_byDiskNone.still_stamp = 1;
				printf("DiskNone current set\n");
			}
		}
		
		g_cAlarmListObj.m_byDiskNone.current = 1;
	}
	else if(cmd == EM_ALRM_ALARM_485EXT)
	{
		printf("m_by485ExtSensor[%d].current: %d\n", chanel, g_cAlarmListObj.m_by485ExtSensor[chanel].current);
		if(chanel >= GetVideoMainNum())
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		
		if(!g_cAlarmListObj.m_by485ExtSensor[chanel].current)
		{
			printf("chn%d 485ExtSensor current set, still_stamp: %d\n", chanel, g_cAlarmListObj.m_by485ExtSensor[chanel].still_stamp);
			if (!g_cAlarmListObj.m_by485ExtSensor[chanel].still_stamp)
			{
				g_cAlarmListObj.m_bChanged = TRUE;
				g_cAlarmListObj.m_by485ExtSensor[chanel].still_stamp = 1;
				printf("chn%d 485ExtSensor still_stamp set\n", chanel);
			}
		}
		
		g_cAlarmListObj.m_by485ExtSensor[chanel].current= 1;
	}
	else
	{
		printf("yg AlarmListAdd alarm source unknow\n");
		if(!g_cAlarmListObj.m_byAlarmEvent[cmd].current)
		{
			if (!g_cAlarmListObj.m_byAlarmEvent[chanel].still_stamp)
			{
				g_cAlarmListObj.m_bChanged = TRUE;
				g_cAlarmListObj.m_byAlarmEvent[chanel].still_stamp = 1;
			}
		}
		
		g_cAlarmListObj.m_byAlarmEvent[cmd].current = 1;
	}
	
	g_cAlarmListObj.m_Mutex.Leave();
	
	CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
	if(pDeskTop != NULL)
	{
		pDeskTop->ToggleAlarmIcon(1);
	}
}

//yaogang  modify 20141117
//视频丢失恢复后要清除报警列表中相关信息，同时
//若此时列表中没有其他报警源，则清除报警三角标
static void video_resume_proc()
{
	int i;

	for(int i = 0; i < GetSenSorNum(); i++)
	{
		if (g_cAlarmListObj.m_bySensor[i].still_stamp)
		{
			return;
		}
	}

	for(i = 0; i < GetVideoMainNum(); i++)
	{
		if(g_cAlarmListObj.m_byVMotion[i].still_stamp)
		{
			return;
		}
		if(g_cAlarmListObj.m_byIPCCover[i].still_stamp)
		{
			return;
		}
		if(g_cAlarmListObj.m_byIPCExtSensor[i].still_stamp)
		{
			return;
		}
	}

	for(i = 0; i < EM_ALRM_TYPES; i++)
	{
		if(g_cAlarmListObj.m_byAlarmEvent[i].still_stamp)
		{
			return;
		}
	}

	int byEvent = 0;
	for(i = 0; i < GetVideoMainNum(); i++)
	{
		
		if(g_cAlarmListObj.m_byVLoss[i].current)
		{
			byEvent = 1;
		}
	}

	if(!byEvent)
	{
		CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
		if(pDeskTop != NULL)
		{
			pDeskTop->ToggleAlarmIcon(0);
		}
	}
}

void AlarmListDelete(u8 chanel,ALARMLISTTYPE cmd,u8 status)
{
	if(cmd >= EM_ALRM_TYPES || cmd < 0)
	{
		return;
	}
	
	g_cAlarmListObj.m_Mutex.Enter();
	
	if(cmd == EM_ALRM_ALARM_INPUT)
	{
		if(chanel >= GetSenSorNum())
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		/*
		if(g_cAlarmListObj.m_bySensor[chanel])
		{
			g_cAlarmListObj.m_bChanged = TRUE;
		}
		*/
		g_cAlarmListObj.m_bySensor[chanel].current = 0;
	}
	else if(cmd == EM_ALRM_MOTION_DETECTION)
	{
		if(chanel >= GetVideoMainNum())
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		/*
		if(g_cAlarmListObj.m_byVMotion[chanel])
		{
			g_cAlarmListObj.m_bChanged = TRUE;
		}
		*/
		g_cAlarmListObj.m_byVMotion[chanel].current= 0;
	}
	else if(cmd == EM_ALRM_VEDIO_LOST)
	{
		if(chanel >= GetVideoMainNum())
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		/*
		if(g_cAlarmListObj.m_byVLoss[chanel])
		{
			g_cAlarmListObj.m_bChanged = TRUE;
		}
		*/
		printf("chn%d video resume\n", chanel);
		g_cAlarmListObj.m_byVLoss[chanel].current= 0;
		g_cAlarmListObj.m_byVLoss[chanel].still_stamp= 0;
		g_cAlarmListObj.m_bChanged = TRUE;
		//yaogang  modify 20141117
		//视频丢失恢复后要清除报警列表中相关信息，同时
		//若此时列表中没有其他报警源，则清除报警三角标
		video_resume_proc();
		
	}
	else if(cmd == EM_ALRM_ALARM_IPCEXT)
	{
		if(chanel >= GetVideoMainNum())
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		/*
		if(g_cAlarmListObj.m_byIPCExtSensor[chanel].current)
		{
			g_cAlarmListObj.m_bChanged = TRUE;
		}
		*/
		g_cAlarmListObj.m_byIPCExtSensor[chanel].current= 0;
		printf("chn%d IPCExtSensor current clear\n", chanel);
	}
	else if(cmd == EM_ALRM_ALARM_IPCCOVER)
	{
		if(chanel >= GetVideoMainNum())
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		/*
		if(g_cAlarmListObj.m_byIPCCover[chanel])
		{
			g_cAlarmListObj.m_bChanged = TRUE;
		}
		*/
		g_cAlarmListObj.m_byIPCCover[chanel].current = 0;
	}
	else if(cmd == EM_ALRM_DISK_LOST)
	{
		if(chanel >= GetVideoMainNum())
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		/*
		if(g_cAlarmListObj.m_byIPCExtSensor[chanel].current)
		{
			g_cAlarmListObj.m_bChanged = TRUE;
		}
		*/
		g_cAlarmListObj.m_byDiskLost[chanel].current= 0;
		printf("yg sata%d DiskLost current clear\n", chanel);
	}
	else if(cmd == EM_ALRM_DISK_WRERR)
	{
		if(chanel >= GetVideoMainNum())
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		
		g_cAlarmListObj.m_byDiskErr[chanel].current= 0;
		printf("yg sata%d DiskErr current clear\n", chanel);
	}
	else if(cmd == EM_ALRM_DISK_NONE)
	{
		
		g_cAlarmListObj.m_byDiskNone.current= 0;
		printf("yg power ON DiskNone current clear\n", chanel);
	}
	else if(cmd == EM_ALRM_ALARM_485EXT)
	{
		if(chanel >= GetVideoMainNum())
		{
			g_cAlarmListObj.m_Mutex.Leave();
			return;
		}
		/*
		if(g_cAlarmListObj.m_byIPCExtSensor[chanel].current)
		{
			g_cAlarmListObj.m_bChanged = TRUE;
		}
		*/
		g_cAlarmListObj.m_by485ExtSensor[chanel].current= 0;
		printf("chn%d 485ExtSensor current clear\n", chanel);
	}
	else
	{
		/*
		if(g_cAlarmListObj.m_byAlarmEvent[cmd])
		{
			g_cAlarmListObj.m_bChanged = TRUE;
		}
		*/
		g_cAlarmListObj.m_byAlarmEvent[cmd].current= 0;
	}
	
	g_cAlarmListObj.m_Mutex.Leave();

}

void RefreshList(u8 page)
{
	g_nAlarmListPage = page;
	g_cAlarmListObj.RefreshAlarmList(page);
}

//yaogang modify 20141106
//用户已经查看了报警信息
//窗口关闭前去除报警已经恢复的源信息
void RefreshAllStatus()
{
	g_cAlarmListObj.RefreshAllStatus();
}

void Biz_CloseGuide()
{
	((CPageBasicConfigFrameWork*)g_pPages[EM_PAGE_BASICCFG])->RecvNotifyClose();
}


void StartTimer()
{
	g_cAlarmListTimer.Start(&g_cAlarmListObj,(VD_TIMERPROC)&CAlarmList::RefreshAlarmList,3000,3000,0xff);
}

void StopTimer()
{
	g_cAlarmListTimer.Stop(TRUE);
}

static SAlarmListItem g_sAlarmInfos[EM_ALRM_TYPES] = 
{
	{"&CfgPtn.DiskFull", "&CfgPtn.DiskFull"},
	{"&CfgPtn.DiskError", "&CfgPtn.LOG_ERROR_HDD_ABNORMAL"},
	{"&CfgPtn.NetDisconnect", "&CfgPtn.PleaseCheckNetWork"},
	{"&CfgPtn.IPConflict", "&CfgPtn.PleaseCheckIP"},
	{"&CfgPtn.2G3GCheck", "&CfgPtn.PleaseCheck2GOr3GModularExist"},
	{"&CfgPtn.PSTNCheck", "&CfgPtn.PleaseCheckPSTNModularExist"},
	
	{"&CfgPtn.Sensoring", "&CfgPtn.Sensoring"},
	{"&CfgPtn.Motion", "&CfgPtn.Motion"},
	{"&CfgPtn.VideoLoss", "&CfgPtn.VideoLoss"},
};
/*
typedef enum
{
	LocalAlarm,
	IPCExtAlarm,
	IPCCoverAlarm,
	HDDAlarm,
	ExtAlarm,
	MaxAlarm,
};

const char *AlarmType[] = 
{
	"&CfgPtn.LocalAlarm", 	//本机报警
	"&CfgPtn.IPCExtAlarm",	//IPC外部报警
	"&CfgPtn.IPCCoverAlarm",	//IPC遮挡报警
	"&CfgPtn.HDDAlarm",	//硬盘报警
	"&CfgPtn.ExtAlarm",		//报警扩展
};
*/
//yaogang modify 20141106
//用户已经查看了报警信息
//窗口关闭前去除报警已经恢复的源信息
void CAlarmList::RefreshAllStatus()
{
	int i;
	bool bAlarmIconShow = FALSE;

	//printf("yg CAlarmList::RefreshAllStatus \n");
	
	m_Mutex.Enter();
	
	for(int i = 0; i < GetSenSorNum(); i++)
	{
		if (m_bySensor[i].still_stamp)// 曾经触发过
		{
			if (m_bySensor[i].current == 0)//现在已经恢复
			{
				m_bChanged = TRUE;
				m_bySensor[i].still_stamp = 0;
			}
			else
			{
				bAlarmIconShow = TRUE;
			}
		}
	}

	for(i = 0; i < GetVideoMainNum(); i++)
	{
		if(m_byVMotion[i].still_stamp)// 曾经触发过
		{
			if (m_byVMotion[i].current == 0)//现在已经恢复
			{
				m_bChanged = TRUE;
				m_byVMotion[i].still_stamp = 0;
			}
			else
			{
				bAlarmIconShow = TRUE;
			}
		}
		if(m_byVLoss[i].still_stamp)
		{
			if (m_byVLoss[i].current == 0)
			{
				m_bChanged = TRUE;
				m_byVLoss[i].still_stamp = 0;
			}
			else
			{
				bAlarmIconShow = TRUE;
			}
		}
		if(m_byIPCCover[i].still_stamp)
		{
			//printf("yg RefreshAllStatus m_byIPCCover[%d].current: %d\n", i, m_byIPCCover[i].current);
			if (m_byIPCCover[i].current == 0)
			{
				m_bChanged = TRUE;
				m_byIPCCover[i].still_stamp = 0;
			}
			else
			{
				bAlarmIconShow = TRUE;
			}
		}
		if(m_byIPCExtSensor[i].still_stamp)
		{
			if (m_byIPCExtSensor[i].current == 0)
			{
				m_bChanged = TRUE;
				m_byIPCExtSensor[i].still_stamp = 0;
				printf("chn%d IPCExtSensor still_stamp clear\n", i);
			}
			else
			{
				bAlarmIconShow = TRUE;
			}
		}
		if(m_by485ExtSensor[i].still_stamp)
		{
			if (m_by485ExtSensor[i].current == 0)
			{
				m_bChanged = TRUE;
				m_by485ExtSensor[i].still_stamp = 0;
				printf("chn%d 485ExtSensor still_stamp clear\n", i);
			}
			else
			{
				bAlarmIconShow = TRUE;
			}
		}
	}

	for(int i = 0; i < MAX_HDD_NUM; i++)
	{
		if (m_byDiskLost[i].still_stamp)// 曾经触发过
		{
			if (m_byDiskLost[i].current == 0)//现在已经恢复
			{
				m_bChanged = TRUE;
				m_byDiskLost[i].still_stamp = 0;
			}
			else
			{
				bAlarmIconShow = TRUE;
			}
		}
		if (m_byDiskErr[i].still_stamp)// 曾经触发过
		{
			if (m_byDiskErr[i].current == 0)//现在已经恢复
			{
				m_bChanged = TRUE;
				m_byDiskErr[i].still_stamp = 0;
			}
			else
			{
				bAlarmIconShow = TRUE;
			}
		}
	}
	if (m_byDiskNone.still_stamp)// 曾经触发过
	{
		if (m_byDiskNone.current == 0)//现在已经恢复
		{
			m_bChanged = TRUE;
			m_byDiskNone.still_stamp = 0;
		}
		else
		{
			bAlarmIconShow = TRUE;
		}
	}

	for(i = 0; i < EM_ALRM_TYPES; i++)
	{
		if(m_byAlarmEvent[i].still_stamp)
		{
			if (m_byAlarmEvent[i].current == 0)
			{
				m_bChanged = TRUE;
				m_byAlarmEvent[i].still_stamp = 0;
			}
			else
			{
				bAlarmIconShow = TRUE;
			}
		}
	}	

	m_Mutex.Leave();

	CPageDesktop* pDeskTop = (CPageDesktop *)g_pPages[EM_PAGE_DESKTOP];
	if(pDeskTop != NULL)
	{
		if (bAlarmIconShow)
		{
			pDeskTop->ToggleAlarmIcon(1);
		}
		else
		{
			pDeskTop->ToggleAlarmIcon(0);
		}
	}
}

void CAlarmList::RefreshAlarmList(u8 page)
{
	if(page == 0xff)
	{
		page = g_nAlarmListPage;
	}
	
	//SValue ComboAlarmType[CBX_MAX];
	//int nItemAlarmCount = 0;
	//GetSensorAlarmTypeList(ComboAlarmType, &nItemAlarmCount, CBX_MAX);
	
	m_Mutex.Enter();
	
	if(m_bChanged)
	{
		m_vCurrItems.clear();
		
		int i = 0;
		//for(i = 0; i < nItemAlarmCount; i++)
		{
			SAlarmListItem item;
			item.type = "&CfgPtn.LocalAlarm";//GetParsedString ComboAlarmType[i].strDisplay;
			item.detail = "";
			u8 sensorflag = 0;
			for(int j = 0; j < GetSenSorNum(); j++)
			{
				//SBizParaTarget sParaTgtIns;
				//SBizAlarmSensorPara sCfgIns;
				//memset(&sCfgIns, 0, sizeof(sCfgIns));
				//sParaTgtIns.emBizParaType = EM_BIZ_SENSORPARA;
				//sParaTgtIns.nChn = j;
				//BizGetPara(&sParaTgtIns, &sCfgIns);
				
				//if(m_bySensor[j] && sCfgIns.nAlarmType == ComboAlarmType[i].nValue)
				if(m_bySensor[j].still_stamp)
				{
					char achStr[4] = {0};
					if(sensorflag)
					{
						sprintf(achStr, " %d", j+1);
					}
					else
					{
						sprintf(achStr, "%d", j+1);
					}
					item.detail += achStr;
					sensorflag = 1;
				}
			}
			//printf("index:%d type:%s value:%d sensorflag:%d\n",i,ComboAlarmType[i].strDisplay,ComboAlarmType[i].nValue,sensorflag);
			if(sensorflag)
			{
				m_vCurrItems.push_back(item);
			}
		}
		
		SAlarmListItem motionitem;
		motionitem.type = "&CfgPtn.Motion";
		motionitem.detail = "";
		
		SAlarmListItem vlostitem;
		vlostitem.type = "&CfgPtn.VideoLoss";
		vlostitem.detail = "";
		
		SAlarmListItem IPCExtitem;
		IPCExtitem.type = "&CfgPtn.IPCExtAlarm";
		IPCExtitem.detail = "";

		SAlarmListItem IPCCoveritem;
		IPCCoveritem.type = "&CfgPtn.IPCCoverAlarm";
		IPCCoveritem.detail = "";

		SAlarmListItem yt485Extitem;
		yt485Extitem.type = "&CfgPtn.485ExtAlarm";
		yt485Extitem.detail = "";
		
		u8 motionflag = 0;
		u8 vlostflag = 0;
		u8 IPCExtflag = 0;
		u8 IPCCoverflag = 0;
		u8 yt485extflag = 0;
		
		for(i = 0; i < GetVideoMainNum(); i++)
		{
			if(m_byVMotion[i].still_stamp)
			{
				char achStr[4] = {0};
				if(motionflag)
				{
					sprintf(achStr, " %d", i+1);
				}
				else
				{
					sprintf(achStr, "%d", i+1);
				}
				motionitem.detail += achStr;
				motionflag = 1;
			}
			if(m_byVLoss[i].still_stamp)
			{
				char achStr[4] = {0};
				if(vlostflag)
				{
					sprintf(achStr, " %d", i+1);
				}
				else
				{
					sprintf(achStr, "%d", i+1);
				}
				vlostitem.detail += achStr;
				vlostflag = 1;
			}
			if(m_byIPCCover[i].still_stamp)
			{
				char achStr[4] = {0};
				if(IPCCoverflag)
				{
					sprintf(achStr, " %d", i+1);
				}
				else
				{
					sprintf(achStr, "%d", i+1);
				}
				IPCCoveritem.detail += achStr;
				IPCCoverflag = 1;
			}
			if(m_byIPCExtSensor[i].still_stamp)
			{
				char achStr[4] = {0};
				if(IPCExtflag)
				{
					sprintf(achStr, " %d", i+1);
				}
				else
				{
					sprintf(achStr, "%d", i+1);
				}
				IPCExtitem.detail += achStr;
				IPCExtflag = 1;
			}
			if(m_by485ExtSensor[i].still_stamp)
			{
				char achStr[4] = {0};
				if(yt485extflag)
				{
					sprintf(achStr, " %d", i+1);
				}
				else
				{
					sprintf(achStr, "%d", i+1);
				}
				yt485Extitem.detail += achStr;
				yt485extflag = 1;
			}
		}
		if(motionflag)
		{
			m_vCurrItems.push_back(motionitem);
		}
		if(vlostflag)
		{
			m_vCurrItems.push_back(vlostitem);
		}
		if(IPCExtflag)
		{
			m_vCurrItems.push_back(IPCExtitem);
		}
		if(IPCCoverflag)
		{
			m_vCurrItems.push_back(IPCCoveritem);
		}
		if(yt485extflag)
		{
			m_vCurrItems.push_back(yt485Extitem);
		}
		//yaogang modify 20141118
		SAlarmListItem disklostitem;
		disklostitem.type = "&CfgPtn.DiskLost";
		disklostitem.detail = "";
		u8 disklostflag = 0;
		SAlarmListItem diskerritem;
		diskerritem.type = "&CfgPtn.DiskErr";
		diskerritem.detail = "";
		u8 diskerrflag = 0;
		
		for(int j = 0; j < MAX_HDD_NUM; j++)
		{
			if(m_byDiskLost[j].still_stamp)
			{
				char achStr[4] = {0};
				if(disklostflag)
				{
					sprintf(achStr, " %d", j+1);
				}
				else
				{
					sprintf(achStr, "%d", j+1);
				}
				disklostitem.detail += achStr;
				disklostflag = 1;
			}
			if(m_byDiskErr[j].still_stamp)
			{
				char achStr[4] = {0};
				if(diskerrflag)
				{
					sprintf(achStr, " %d", j+1);
				}
				else
				{
					sprintf(achStr, "%d", j+1);
				}
				diskerritem.detail += achStr;
				diskerrflag = 1;
			}
		}
		//printf("index:%d type:%s value:%d sensorflag:%d\n",i,ComboAlarmType[i].strDisplay,ComboAlarmType[i].nValue,sensorflag);
		if(disklostflag)
		{
			m_vCurrItems.push_back(disklostitem);
		}
		if(diskerrflag)
		{
			m_vCurrItems.push_back(diskerritem);
		}
		
		SAlarmListItem disknoneitem;
		disknoneitem.type = "&CfgPtn.DiskNone";
		disknoneitem.detail = "";
		u8 disknoneflag = 0;
		if(m_byDiskNone.still_stamp)
		{
			disknoneflag = 1;
		}
		if(disknoneflag)
		{
			m_vCurrItems.push_back(disknoneitem);
		}
		
		for(i = 0; i < EM_ALRM_TYPES; i++)
		{
			if(m_byAlarmEvent[i].still_stamp)
			{
				m_vCurrItems.push_back(g_sAlarmInfos[i]);
			}
		}
	}
	
	if(m_bChanged || m_nPage != page)
	{
		m_bChanged = FALSE;
		m_nPage = page;
		
		m_Mutex.Leave();
		
		CPageAlarmList* pAlarmList = (CPageAlarmList *)g_pPages[EM_PAGE_ALARMLIST];
		if(pAlarmList != NULL)
		{
			pAlarmList->ClearItems(page);
			
			int start = page * 10;
			int end = (page + 1) * 10;
			
			for(int i = start; i < m_vCurrItems.size() && i < end; i++)
			{
				pAlarmList->InsertItem(i-start, m_vCurrItems[i].type.c_str(), m_vCurrItems[i].detail.c_str());
			}
		}
		
		return;
	}
	
	m_Mutex.Leave();
}

CAlarmList::CAlarmList() : m_Mutex(MUTEX_RECURSIVE)
{
	m_nPage = 0;
	m_bChanged = FALSE;
	
	memset(m_byAlarmEvent, 0, sizeof(m_byAlarmEvent));
	memset(m_byVLoss, 0, sizeof(m_byVLoss));
	memset(m_byVMotion, 0, sizeof(m_byVMotion));
	memset(m_bySensor, 0, sizeof(m_bySensor));
	memset(m_byIPCExtSensor, 0, sizeof(m_byIPCExtSensor));
	memset(m_byIPCCover, 0, sizeof(m_byIPCCover));
	
	m_vCurrItems.clear();
}

CAlarmList::~CAlarmList()
{
	
}

//yaogang modify 20141210
void GetRepairNameList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		//获得 配置 
		u8 real = 0;
		BIZ_DATA_DBG("GetRepairNameList1\n");
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_RepairName, g_strList, (u8*)&real, 20, 64);
		BIZ_DATA_DBG("GetRepairNameList2\n");
		*nRealNum = real;
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetRepairNameList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nVideoFormat[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}	
	}
}
void GetRepairTypeList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		//获得 配置 
		u8 real = 0;
		BIZ_DATA_DBG("GetRepairTypeList 1\n");
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_RepairType, g_strList, (u8*)&real, 20, 64);
		BIZ_DATA_DBG("GetRepairTypeList 2\n");
		*nRealNum = real;
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetRepairTypeList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nVideoFormat[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}	
	}
}
void GetMaintainTypeList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		//获得 配置 
		u8 real = 0;
		BIZ_DATA_DBG("GetMaintainTypeList 1\n");
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_Maintain, g_strList, (u8*)&real, 20, 64);
		BIZ_DATA_DBG("GetMaintainTypeList 2\n");
		*nRealNum = real;
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetMaintainTypeList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nVideoFormat[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}	
	}
}

void GetTestTypeList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	InitStrList();
	
	if (psValueList)
	{
		//获得 配置 
		u8 real = 0;
		BIZ_DATA_DBG("GetTestTypeList 1\n");
		int ret = BizConfigGetParaStr(EM_BIZ_CFG_PARALIST_Test, g_strList, (u8*)&real, 20, 64);
		BIZ_DATA_DBG("GetTestTypeList 2\n");
		*nRealNum = real;
		
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetTestTypeList\n");
		}

		if (*nRealNum > nMaxNum)
		{
			*nRealNum = nMaxNum;
		}

		for(int i=0; i<*nRealNum; i++)
		{
			//psValueList[i].nValue = nVideoFormat[i];
			strcpy(psValueList[i].strDisplay,g_strList[i]);
		}	
	}
}

void GetAlarmInputList( SValue* psValueList, int* nRealNum, int nMaxNum)
{
	return ;
}


//yaogang modify 20141209
//深广平台参数
int bizData_GetSGparamDefault(SGParam *para)
{
	int rtn = 1;
	SBizParaTarget sParaTgtIns;
	SBizSGParam sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SGPARA;
	
	if (para)
	{
		rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
		if(0 == rtn)
		{
			memcpy(para, &sCfgIns, sizeof(SGParam));
		}
	}
	return rtn;
}

int bizData_GetSGparam(SGParam *para)
{
	int rtn = 1;
	SBizParaTarget sParaTgtIns;
	SBizSGParam sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SGPARA;
	

	if (para)
	{
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0 == rtn)
		{
			memcpy(para, &sCfgIns, sizeof(SGParam));
		}
	}
	return rtn;
}

int bizData_SaveSGparam(SGParam *para)
{
	int rtn = 1;
	SBizParaTarget sParaTgtIns;
	SBizSGParam sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SGPARA;

	if (para)
	{
		memcpy(&sCfgIns, para, sizeof(SBizSGParam));
		rtn = BizSetPara(&sParaTgtIns, &sCfgIns);
	}
	return rtn;
}

//报警图片配置
int bizData_GetSGAlarmParamDefault(int chn, SAlarmPicCFG *para)
{
	int rtn = 1;
	SBizParaTarget sParaTgtIns;
	SBizAlarmPicCFG sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SGALARMPARA;
	sParaTgtIns.nChn = chn;
	
	if (para)
	{
		rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
		if(0 == rtn)
		{
			memcpy(para, &sCfgIns, sizeof(SBizAlarmPicCFG));
		}
	}
	return rtn;
}

int bizData_GetSGAlarmParam(int chn, SAlarmPicCFG *para)
{
	int rtn = 1;
	SBizParaTarget sParaTgtIns;
	SBizAlarmPicCFG sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SGALARMPARA;
	sParaTgtIns.nChn = chn;

	if (para)
	{
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0 == rtn)
		{
			memcpy(para, &sCfgIns, sizeof(SBizAlarmPicCFG));
		}
	}
	return rtn;
}

int bizData_SaveSGAlarmParam(int chn, SAlarmPicCFG *para)
{
	int rtn = 1;
	SBizParaTarget sParaTgtIns;
	SBizAlarmPicCFG sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SGALARMPARA;
	sParaTgtIns.nChn = chn;

	if (para)
	{
		memcpy(&sCfgIns, para, sizeof(SBizAlarmPicCFG));
		rtn = BizSetPara(&sParaTgtIns, &sCfgIns);
	}
	return rtn;
}

//日常图片配置
int bizData_GetSGDailyParamDefault(SDailyPicCFG *para)
{
	int rtn = 1;
	SBizParaTarget sParaTgtIns;
	SBizDailyPicCFG sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SGDAILYPARA;
	
	if (para)
	{
		rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
		if(0 == rtn)
		{
			memcpy(para, &sCfgIns, sizeof(SBizDailyPicCFG));
		}
	}
	return rtn;
}

int bizData_GetSGDailyParam(SDailyPicCFG *para)
{
	int rtn = 1;
	SBizParaTarget sParaTgtIns;
	SBizDailyPicCFG sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SGDAILYPARA;

	if (para)
	{
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0 == rtn)
		{
			memcpy(para, &sCfgIns, sizeof(SBizDailyPicCFG));
		}
	}
	return rtn;
}

int bizData_SaveSGDailyParam(SDailyPicCFG *para)
{
	int rtn = 1;
	SBizParaTarget sParaTgtIns;
	SBizDailyPicCFG sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SGDAILYPARA;

	if (para)
	{
		memcpy(&sCfgIns, para, sizeof(SBizDailyPicCFG));
		rtn = BizSetPara(&sParaTgtIns, &sCfgIns);
	}
	return rtn;
}


//yaogang modify 20150105
//snap param
int bizData_SetSnapChnPara(int chn, SGuiSnapChnPara *para)
{
	int rtn = 1;
	SBizParaTarget sParaTgtIns;
	SBizSnapChnPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SNAP_PARA;
	sParaTgtIns.nChn = chn;

	if (para)
	{
		memcpy(&sCfgIns, para, sizeof(SGuiSnapChnPara));
		rtn = BizSetPara(&sParaTgtIns, &sCfgIns);
		
	}
	return rtn;
}
int bizData_GetSnapChnPara(int chn, SGuiSnapChnPara *para)
{
	int rtn = 1;
	SBizParaTarget sParaTgtIns;
	SBizSnapChnPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SNAP_PARA;
	sParaTgtIns.nChn = chn;

	if (para)
	{
		rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
		if(0 == rtn)
		{
			memcpy(para, &sCfgIns, sizeof(SGuiSnapChnPara));
		}
	}
	return rtn;
}
int bizData_GetSnapChnParaDefault(int chn, SGuiSnapChnPara *para)
{
	int rtn = 1;
	SBizParaTarget sParaTgtIns;
	SBizSnapChnPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_SNAP_PARA;
	sParaTgtIns.nChn = chn;

	if (para)
	{
		rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
		if(0 == rtn)
		{
			memcpy(para, &sCfgIns, sizeof(SGuiSnapChnPara));
		}
	}
	return rtn;
}


