#include <sys/time.h>
#include "GUI/Pages/PageInputTray.h"
#include "GUI/Pages/PageLogin.h"
#include "GUI/Pages/PageStartFrameWork.h"
//#include "GUI/Pages/PageFunction.h"
//#include "GUI/Pages/PageControlPtz.h"
//#include "GUI/Pages/PageImageColor.h"
//#include "GUI/Pages/PagePlay.h"
//#include "GUI/Pages/PageBackup.h"
#include "GUI/Pages/PageDesktop.h"
//#include "GUI/Pages/PageMain.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/PagePlayBackFrameWork.h"
#include "GUI/Pages/PageAlarmList.h"
#include "text.h"


#include "GUI/GUI.h"
#include "Main.h"
#include "GUI/Pages/BizData.h"
//#include "Functions/Record.h"
//#include "Functions/Alarm.h"
//#include "Devices/DevVideo.h"
//#include "Functions/DriverManager.h"

//#define CHORTCUT_BITMAP_DIR "/usr/data/Data/menu/"  //设置菜单图片路径add langzi 2009-11-9
#define CHORTCUT_BITMAP_DIR "/tmp/ui/data/Data/temp/"

#define EXCEED_MS 350		//超时时间350ms
#define SHUT_TIME 3000  	//关机键按下时间
#define CHSTATE_WIDTH 24	// 通道状态标志的宽度

#define CHANNEL_RECT_ADJUST(video_rect)		\
		do {	\
			if(video_rect.right && video_rect.bottom)	\
			{	\
				video_rect.top += 5;	\
				video_rect.left += 2;	\
				video_rect.right += 0;	\
				video_rect.bottom++;	\
			}	\
		}while(0)

//add by chenweihong
#if defined(VIEWGATE) || defined(DIGITAL)
//监视界面背景中的需要重复的中间长条的高度
#define MAIN_FRAME_REPEAT_HEIGHT  60
#else
#define MAIN_FRAME_REPEAT_HEIGHT  42
#endif
//打开主页面的消息，因为不能在定时器里直接调用（会引起阻塞，所以以消息的形式）
#define XM_OPENMAINPAGE (XM_SYSTEM_END + 1)

//显示关机密码菜单，执行关机功能消息，防止引起阻塞
#define XM_SHUTDOWNTIMEUP	(XM_SYSTEM_END + 2)

//add by yanjun 20061117 小于此大小被认为成矩形框的大小为零
#define ZERO_RECT_SIZE_ENABLE	20

/*
	添加对网络键盘的轮训键支持
	如果开启了网络键盘轮训键的支持宏，则强制打开轮训控制支持
*/ 
#ifdef MON_NETKEY_TOUR
	#ifndef APP_TOUR_TURN
	#define APP_TOUR_TURN
	#endif
#endif

static int GetMaxChnNum()
{
	return GetVideoMainNum();
}

enum
{
	//与下面的数组一一对应
	SHORTCUT_TOOLMENU,
	SHORTCUT_TOOL1X1,
	SHORTCUT_TOOLTRIANGLE,
	SHORTCUT_TOOL2X2,
	SHORTCUT_TOOLDWELLNO,
	SHORTCUT_TOOLCOLOR,
	SHORTCUT_TOOLZOOM,
	SHORTCUT_TOOLAUDIO,	
	SHORTCUT_TOOLPTZ,
	SHORTCUT_TOOLSNAP,
	SHORTCUT_TOOLREC,
	SHORTCUT_TOOLPLAY,
	SHORTCUT_TOOLEDIT,
	SHORTCUT_TOOLTMBK,
	SHORTCUT_MAX
};

#if 0//csp modify
char* shortcut[SHORTCUT_MAX] = 
{
	"tool.menu",
	"tool.1x1",	
	"tool.triangle",
	"tool.2x2",
	"tool.dwellno",
	"tool.color",
	"tool.zoom",
	"tool.audio",
	"tool.ptz",
	"tool.snap",
	"tool.rec",
	"tool.play",
	"tool.edit",
	//"tool.edit",
};

char *shortcutBmpName[SHORTCUT_MAX][2] = 
{
	//add langzi 2009-11-9 右击菜单图标文件名
	{CHORTCUT_BITMAP_DIR"tool_menu.bmp",CHORTCUT_BITMAP_DIR"tool_menu_f.bmp"},
	{CHORTCUT_BITMAP_DIR"tool_1x1.bmp",CHORTCUT_BITMAP_DIR"tool_1x1_f.bmp"},	
	{CHORTCUT_BITMAP_DIR"tool_triangle.bmp",CHORTCUT_BITMAP_DIR"tool_triangle_f.bmp"},
	{CHORTCUT_BITMAP_DIR"tool_2x2.bmp",CHORTCUT_BITMAP_DIR"tool_2x2_f.bmp"},
	{CHORTCUT_BITMAP_DIR"tool_dwell_no.bmp",CHORTCUT_BITMAP_DIR"tool_dwell_no_f.bmp"},
	{CHORTCUT_BITMAP_DIR"tool_set_color.bmp",CHORTCUT_BITMAP_DIR"tool_set_color_f.bmp"},
	{CHORTCUT_BITMAP_DIR"tool_zoom.bmp",CHORTCUT_BITMAP_DIR"tool_zoom_f.bmp"},
	{CHORTCUT_BITMAP_DIR"tool_audio_mute.bmp",CHORTCUT_BITMAP_DIR"tool_audio_mute_f.bmp"},
	{CHORTCUT_BITMAP_DIR"tool_ptz.bmp",CHORTCUT_BITMAP_DIR"tool_ptz_f.bmp"},
	{CHORTCUT_BITMAP_DIR"tool_snap.bmp",CHORTCUT_BITMAP_DIR"tool_snap_f.bmp"},
	{CHORTCUT_BITMAP_DIR"tool_rec.bmp",CHORTCUT_BITMAP_DIR"tool_rec_f.bmp"},
	{CHORTCUT_BITMAP_DIR"tool_play.bmp",CHORTCUT_BITMAP_DIR"tool_play_f.bmp"},
	{CHORTCUT_BITMAP_DIR"tool_edit.bmp",CHORTCUT_BITMAP_DIR"tool_edit_f.bmp"},
	{CHORTCUT_BITMAP_DIR"tool_tmbk.bmp",CHORTCUT_BITMAP_DIR"tool_tmbk.bmp"},
};
#endif

//csp modify
//VD_BITMAP *pbmpIcon[SHORTCUT_MAX][2];//={NULL,LoadBitmap()};//add langzi 2009-11-9
VD_BITMAP *pbmpIcon[SHORTCUT_MAX+1][2],*alarmIcon;

static int split_h = 0;
static int split_v = 0;
static int IsOperaterror = 0;

CPageDesktop::CPageDesktop(VD_PCRECT pRect, VD_PCSTR psz, VD_BITMAP* icon, CPage * pParent) : 
	CPage(pRect, psz, pParent, pageFullScreen | pageNoExpire)
	//,m_PageMainTimer("PageDesktop")
	//,m_UpdateTimer("UpdateTime")
	//,m_ShutTimer("ShutDown")
	//,m_SwitchTimer("SwitchOutput")
	//,m_ScreenProtect(NULL)
	//,m_iTimeFmt(-1)
#ifdef APP_TOUR_TURN
	//,m_bTitleFlag(TRUE)
#endif
{
	printf(">>>>Enter CPageDesktop::CPageDesktop...Creat!\n");

	//yaogang modify 20150324
	SBizDvrInfo DvrInfo;
	nNVROrDecoder = 1; 
	//跃天: 1 nvr，2 轮巡解码器，3 切换解码器
	if (BizGetDvrInfo(&DvrInfo) == 0)
	{
		nNVROrDecoder = DvrInfo.nNVROrDecoder;
	}
	
	int i, j;
	int sub;
	
	VD_BOOL flag = FALSE;//是否有分割画面子菜单
	nPreviewPage = 0;
	IsDrawElecZoomArea = 0;
	IsElecZoomBegin = 0;
	IsLbuttonDown = 0;
	m_CurChn = 0;
	
	start_x = 0;
	start_y = 0;
	end_x = 0;
	end_y = 0;
	zoom_w = 0;
	zoom_h = 0;
	
	iszoom_x1 = 0;
	iszoom_y1 = 0;
	iszoom_x2 = 0;
	iszoom_y2 = 0;
	
	IsDwell = 0;
	
	//printf("CPageDesktop::CPageDesktop..1\n");
	
#if 1//csp modify
	for(i = 0; i < SHORTCUT_MAX+1; i++)
	{
		pbmpIcon[i][0] = NULL;
		pbmpIcon[i][1] = NULL;
	}
#else
	for(i = 0; i < SHORTCUT_MAX; i++)//add langzi 2009-11-9 导入图片
	{
		pbmpIcon[i][0] = VD_LoadBitmap(shortcutBmpName[i][0]);
		pbmpIcon[i][1] = VD_LoadBitmap(shortcutBmpName[i][1]);
		ReplaceBitmapBits(pbmpIcon[i][0], VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
		ReplaceBitmapBits(pbmpIcon[i][1], VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	}
#endif
	
#if 0
	memset(pbmpIcon[SHORTCUT_TOOLTMBK][0]->bits,0x80,pbmpIcon[SHORTCUT_TOOLTMBK][0]->height*pbmpIcon[SHORTCUT_TOOLTMBK][0]->width*2);
#endif
	
	//printf("CPageDesktop::CPageDesktop..2\n");
	
	//以下为创建桌面菜单结构
	CRect m_rect;
	//printf("CPageDesktop::CPageDesktop..3\n");
	m_rect.SetRect(0, 0, 0, 0);
	//printf("CPageDesktop::CPageDesktop..4\n");
	pMainMenu = CreateCMenu(m_rect, this, NULL, NULL, styleDeskTopNew/*styleDeskTop*/);//桌面主菜单项
	
	//cj@20110412
	CRect rtmenu = CRect(0,0,796,40);
	pMainMenu->SetRect(&rtmenu);
	pMainMenu->SetItemWidth(40);
	pMainMenu->SetItemHeight(72);
	//pMainMenu->SetXSpace(6);
	//end cj@20110412
	
	//printf("CPageDesktop::CPageDesktop..5\n");
	
	pMainMenu->SetRevPopup(FALSE);
	
	//printf("CPageDesktop::CPageDesktop..6\n");
	//pMainMenu->ApendItem(LOADSTR("pgquick.sysmenu"),(CPage*)NULL);
	//printf("CPageDesktop::CPageDesktop..6.1\n");
	
#if 0
	//add langzi 2009-11-9
	pMainMenu->ApendItem(LOADSTR(shortcut[SHORTCUT_MAINMENU]),(CTRLPROC)&CPageDesktop::OnMain, pbmpIcon[SHORTCUT_MAINMENU][0],pbmpIcon[SHORTCUT_MAINMENU][1]);
 	pMainMenu->ApendItem(LOADSTR(shortcut[SHORTCUT_QUICKREPLAY]),(CTRLPROC)&CPageDesktop::OnQuickSch, pbmpIcon[SHORTCUT_QUICKREPLAY][0],pbmpIcon[SHORTCUT_QUICKREPLAY][1]);
 	pMainMenu->ApendItem(LOADSTR(shortcut[SHORTCUT_RECORDCONTROL]),(CTRLPROC)&CPageDesktop::OnRecord, pbmpIcon[SHORTCUT_RECORDCONTROL][0],pbmpIcon[SHORTCUT_RECORDCONTROL][1]);
 	pMainMenu->ApendItem(LOADSTR(shortcut[SHORTCUT_BACKUP]),(CTRLPROC)&CPageDesktop::OnBackUp,pbmpIcon[SHORTCUT_BACKUP][0],pbmpIcon[SHORTCUT_BACKUP][1]);
 	pMainMenu->ApendItem(LOADSTR(shortcut[SHORTCUT_QUICKALMOUT]),(CTRLPROC)&CPageDesktop::OnCtrlAlmOut,pbmpIcon[SHORTCUT_QUICKALMOUT][0],pbmpIcon[SHORTCUT_QUICKALMOUT][1]);
 	pMainMenu->ApendItem(LOADSTR(shortcut[SHORTCUT_PTZCTRL]),(CTRLPROC)&CPageDesktop::OnPtzCtrl,pbmpIcon[SHORTCUT_PTZCTRL][0],pbmpIcon[SHORTCUT_PTZCTRL][1]);
 	pMainMenu->ApendItem(LOADSTR(shortcut[SHORTCUT_QUICKALMIN]),(CTRLPROC)&CPageDesktop::OnImgConf,pbmpIcon[SHORTCUT_QUICKALMIN][0],pbmpIcon[SHORTCUT_QUICKALMIN][1]);
#else
	#if 0//csp modify
	pMainMenu->ApendItem(NULL,(CTRLPROC)&CPageDesktop::OnMenu, pbmpIcon[SHORTCUT_TOOLMENU][0],pbmpIcon[SHORTCUT_TOOLMENU][1]);
	pMainMenu->ApendItem(NULL,(CTRLPROC)&CPageDesktop::On1x1, pbmpIcon[SHORTCUT_TOOL1X1][0],pbmpIcon[SHORTCUT_TOOL1X1][1]);
	pMainMenu->ApendItem(NULL,(CTRLPROC)&CPageDesktop::OnTriangle, pbmpIcon[SHORTCUT_TOOLTRIANGLE][0],pbmpIcon[SHORTCUT_TOOLTRIANGLE][1]);
	pMainMenu->ApendItem(NULL,(CTRLPROC)&CPageDesktop::On2x2,pbmpIcon[SHORTCUT_TOOL2X2][0],pbmpIcon[SHORTCUT_TOOL2X2][1]);
	pMainMenu->ApendItem(NULL,(CTRLPROC)&CPageDesktop::OnDwellNo,pbmpIcon[SHORTCUT_TOOLDWELLNO][0],pbmpIcon[SHORTCUT_TOOLDWELLNO][1]);
	pMainMenu->ApendItem(NULL,(CTRLPROC)&CPageDesktop::OnColor,pbmpIcon[SHORTCUT_TOOLCOLOR][0],pbmpIcon[SHORTCUT_TOOLCOLOR][1]);
	pMainMenu->ApendItem(NULL,(CTRLPROC)&CPageDesktop::OnZoom,pbmpIcon[SHORTCUT_TOOLZOOM][0],pbmpIcon[SHORTCUT_TOOLZOOM][1]);
	pMainMenu->ApendItem(NULL,(CTRLPROC)&CPageDesktop::OnAudio,pbmpIcon[SHORTCUT_TOOLAUDIO][0],pbmpIcon[SHORTCUT_TOOLAUDIO][1]);
	pMainMenu->ApendItem(NULL,(CTRLPROC)&CPageDesktop::OnPtz,pbmpIcon[SHORTCUT_TOOLPTZ][0],pbmpIcon[SHORTCUT_TOOLPTZ][1]);
	pMainMenu->ApendItem(NULL,(CTRLPROC)&CPageDesktop::OnSnap,pbmpIcon[SHORTCUT_TOOLSNAP][0],pbmpIcon[SHORTCUT_TOOLSNAP][1]);
	pMainMenu->ApendItem(NULL,(CTRLPROC)&CPageDesktop::OnRec,pbmpIcon[SHORTCUT_TOOLREC][0],pbmpIcon[SHORTCUT_TOOLREC][1]);
	pMainMenu->ApendItem(NULL,(CTRLPROC)&CPageDesktop::OnPlay,pbmpIcon[SHORTCUT_TOOLPLAY][0],pbmpIcon[SHORTCUT_TOOLPLAY][1]);
	pMainMenu->ApendItem(NULL,(CTRLPROC)&CPageDesktop::OnEdit,pbmpIcon[SHORTCUT_TOOLEDIT][0],pbmpIcon[SHORTCUT_TOOLEDIT][1]);
	#endif
#endif
	
	//printf("CPageDesktop::CPageDesktop..7\n");
	
	//pMainMenu->ApendItem(LOADSTR(shortcut[SHORTCUT_TVADJUST]),(CTRLPROC)&CPageDesktop::OnTvAjust,pbmpIcon[SHORTCUT_TVADJUST][0],pbmpIcon[SHORTCUT_TVADJUST][1]);
	//pMainMenu->ApendItem(LOADSTR(shortcut[SHORTCUT_LOGOUT]),(CTRLPROC)&CPageDesktop::OnLogout);
#ifndef NOT_SUPPORT_SHUTDOWN
	//pMainMenu->ApendItem(LOADSTR(shortcut[SHORTCUT_LOGOUT]),(CTRLPROC)&CPageDesktop::OnLogout, pbmpIcon[SHORTCUT_LOGOUT][0],pbmpIcon[SHORTCUT_LOGOUT][1]); //add langzi 2009-11-9
#endif
	
	//pMainMenu->ApendItem();//separator
	
	//printf("CPageDesktop::CPageDesktop..8\n");
	
	//printf("CPageDesktop::CPageDesktop..9 m_screen=%d\n",m_screen);
	
	//m_pDevSplit = CDevSplit::instance(m_screen);
	m_pDevGraphics = CDevGraphics::instance(m_screen);
	m_pDevGraphics->GetRect(&m_RectScreen);
	//m_pDevVideo = CDevVideo::instance(m_screen);
	
	//printf("CPageDesktop::CPageDesktop..10\n");
	
	//shutdown count down page
	//m_ShutCountDown = 0;
	
	m_keyColor = m_pDevGraphics->GetKeyColor();
	
	//创建页面控件
	CRect rect(0, 0, 0, 0);
	
	//仅主GUI显示以下页面
	if(m_screen == 0)
	{
		//printf("CPageDesktop::CPageDesktop 11\n");
		
 		//m_pPageMain = new CPageMainFrameWork(NULL, "  main menu"/*"&titles.mainmenu"*/, icon_dvr, NULL/*this*/);
		//m_pPagePlayBack = new CPagePlayBackFrameWork(&m_RectScreen, ""/*"&titles.mainmenu"*/, icon_dvr, NULL/*this*/);
		
		m_pPageInputTray = new CPageInputTray(CRect(0, 0, 38, 26), NULL, NULL, this);
		//m_pPageInputTray = new CPageInputTray(NULL, NULL, NULL, this);
		//m_pPageInputTray->Open();
		
		//printf("CPageDesktop::CPageDesktop 111\n");
		
		m_pPageLogin = new CPageLogin(CRect(0, 0, 320, 250), "&CfgPtn.UserLogin"/*"&titles.mainmenu"*/, icon_dvr, this);
		
		//printf("CPageDesktop::CPageDesktop 112\n");
		
		int ret = BizGetSysInfo(&sysPara);
		
		if(ret < 0 || ((EM_BIZ_PAL != sysPara.nVideoStandard) && (EM_BIZ_NTSC != sysPara.nVideoStandard)))
		{
			printf("BizGetSysInfo Error\n");
			sysPara.nVideoStandard = EM_BIZ_PAL;
		}
		
		if(EM_BIZ_PAL == sysPara.nVideoStandard)
		{
			CurScreenHeight = 576;
		}
		else if(EM_BIZ_NTSC == sysPara.nVideoStandard)
		{
			CurScreenHeight = 480;
		}
		
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
		
		SBizSystemPara bizSysPara;
		ret = BizGetPara(&bizTar, &bizSysPara);
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetVideoFormat\n");
		}
		
		//printf("CPageDesktop::CPageDesktop 113\n");
		
		SBizDvrInfo bizDvrInfo;
		bizTar.emBizParaType = EM_BIZ_DVRINFO;
		
		ret = BizGetPara(&bizTar, &bizDvrInfo);
		if(ret!=0)
		{
			BIZ_DATA_DBG("Get EM_BIZ_DVRINFO nOutputSync\n");
		}
		
		//printf("CPageDesktop::CPageDesktop 1131,nOutputSync=%d,nOutput=%d\n",bizDvrInfo.nOutputSync,bizSysPara.nOutput);
		
		//bizDvrInfo.nOutputSync == EM_BIZ_OUTPUT_VGAORCVBS;//why???//csp modify
		//bizSysPara.nOutput = EM_BIZ_OUTPUT_CVBS;
		
		if((EM_BIZ_OUTPUT_VGAORCVBS == bizDvrInfo.nOutputSync) && (EM_BIZ_OUTPUT_CVBS == bizSysPara.nOutput))//csp modify
		{
			int nOffsetXCvbs = 48;
			int nOffsetYCvbs = 24;
			m_pPageStart = new CPageStartFrameWork(CRect(nOffsetXCvbs, m_RectScreen.bottom - nOffsetYCvbs - 40, m_RectScreen.right - nOffsetXCvbs, m_RectScreen.bottom - nOffsetYCvbs)/*&m_RectScreen*/, ""/*"&titles.mainmenu"*/, icon_dvr, this);
		}
		else
		{
			//csp modify
			//m_pPageStart = new CPageStartFrameWork(CRect(0, m_RectScreen.bottom - 40, m_RectScreen.right, m_RectScreen.bottom)/*&m_RectScreen*/, ""/*"&titles.mainmenu"*/, icon_dvr, this);
			
			//csp modify
			int nOffsetXCvbs = 0;
			int nOffsetYCvbs = 0;
			if(EM_BIZ_OUTPUT_HDMI_1920X1080 == bizSysPara.nOutput)
			{
				nOffsetXCvbs = 160;
				nOffsetYCvbs = 48;
			}
			else if(EM_BIZ_OUTPUT_HDMI_1280X720 == bizSysPara.nOutput)
			{
				nOffsetXCvbs = 112;
				nOffsetYCvbs = 30;
			}
			else if(EM_BIZ_OUTPUT_VGA_1280X1024 == bizSysPara.nOutput)
			{
				nOffsetXCvbs = 112;
				nOffsetYCvbs = 48;
			}
			else if(EM_BIZ_OUTPUT_VGA_1024X768 == bizSysPara.nOutput)
			{
				nOffsetXCvbs = 84;
				nOffsetYCvbs = 30;
			}
			else
			{
				nOffsetXCvbs = 48;
				nOffsetYCvbs = 24;
			}
			m_pPageStart = new CPageStartFrameWork(CRect(nOffsetXCvbs, m_RectScreen.bottom - nOffsetYCvbs - 40, m_RectScreen.right - nOffsetXCvbs, m_RectScreen.bottom - nOffsetYCvbs)/*&m_RectScreen*/, ""/*"&titles.mainmenu"*/, icon_dvr, this);
		}
		
		SetPage(EM_PAGE_START, m_pPageStart);
		
		//printf("CPageDesktop::CPageDesktop 114\n");
	}
	
	//printf("CPageDesktop::CPageDesktop 12\n");
	
	nScreanWidth = 720;
	nScreanHeight = 576;
	//static int nScreanWidth = 720, nScreanHeight = 576;
	//static char flagg = 1;
	//if(flagg)
	{
		GetVgaResolution(&nScreanWidth, &nScreanHeight);
		//flagg = 0;
	}
	
	m_curCover = 0;
	m_FlagPlaying = 0;
	m_nPlayBackFileFormat = 3;
	m_AllChnStatus = NULL;
	
	maxChn = GetMaxChnNum();
	InitSplitLine();
	
	int maxCoverPerChn = 4;
	
	InitCoverRect(maxChn, maxCoverPerChn);
	
	InitChnName(maxChn);
	InitTimeTitle(maxChn);
	InitChnState(maxChn, 5);

	InitRegion(maxChn);

	//add by liu
	InitChnKbps(maxChn);
	InitChnElseState(maxChn, 2);
	
	InitElecZoomTile(maxChn);
	InitElecZoomInvalidTile(maxChn);
	InitElecZoomAreaSplitLine(maxChn);
	InitElecZoomSplitLine(maxChn);
	
	bThisPageFlag = FALSE;
	m_IsElecZoomDrawStatus = FALSE;
	m_IsPbElecZoomDrawStatus = FALSE;
	
	//获得当前预览模式画面数
	switch(maxChn)
	{
	case 4:
		emCurPreviewMode = EM_BIZPREVIEW_4SPLITS;
		eMaxViewMode = EM_BIZPREVIEW_4SPLITS;
		RowNum = ColNum = 2;
		break;
	case 8:
		emCurPreviewMode = EM_BIZPREVIEW_9SPLITS;
		eMaxViewMode = EM_BIZPREVIEW_9SPLITS;
		RowNum = ColNum = 3;
		break;
	case 10:
		emCurPreviewMode = EM_BIZPREVIEW_10SPLITS;
		eMaxViewMode = EM_BIZPREVIEW_10SPLITS;
		RowNum = ColNum = 4;
		break;
	case 6:
		emCurPreviewMode = EM_BIZPREVIEW_6SPLITS;
		eMaxViewMode = EM_BIZPREVIEW_6SPLITS;
		RowNum = ColNum = 4;
		break;
	case 16:
		emCurPreviewMode = EM_BIZPREVIEW_16SPLITS;
		eMaxViewMode = EM_BIZPREVIEW_16SPLITS;
		RowNum = ColNum = 4;
		break;
	case 24:
		emCurPreviewMode = EM_BIZPREVIEW_25SPLITS;
		eMaxViewMode = EM_BIZPREVIEW_25SPLITS;
		RowNum = 5;
		ColNum = 5;
		break;
	case 32:
		emCurPreviewMode = EM_BIZPREVIEW_36SPLITS;
		eMaxViewMode = EM_BIZPREVIEW_36SPLITS;
		RowNum = ColNum = 6;
		break;
	default:
		emCurPreviewMode = EM_BIZPREVIEW_4SPLITS;
		eMaxViewMode = EM_BIZPREVIEW_4SPLITS;
		RowNum = ColNum = 2;
		break;
	}
	emLastPreMode = eMaxViewMode;
	
	for(int i=0; i<maxChn; i++)
	{
		//yaogang modify 20140913
		//if (i == 0)
			//printf("yg SetChnStateBMP\n");
		
		SetChnStateBMP(i,EM_STATE_RECORD,CHORTCUT_BITMAP_DIR"live_record.bmp",CHORTCUT_BITMAP_DIR"live_record.bmp");
		SetChnStateBMP(i,EM_STATE_MD,CHORTCUT_BITMAP_DIR"live_motion.bmp",CHORTCUT_BITMAP_DIR"live_motion.bmp");
		SetChnStateBMP(i,EM_STATE_ALARM,CHORTCUT_BITMAP_DIR"live_alarm.bmp",CHORTCUT_BITMAP_DIR"live_alarm.bmp");
		SetChnStateBMP(i,EM_STATE_TIMER,CHORTCUT_BITMAP_DIR"live_timer.bmp",CHORTCUT_BITMAP_DIR"live_timer.bmp");
		SetChnStateBMP(i,EM_STATE_MANUAL_REC,CHORTCUT_BITMAP_DIR"live_manual.bmp",CHORTCUT_BITMAP_DIR"live_manual.bmp");

		//add by liu
		SetChnElseStateBMP(i,EM_STATE_SNAP,CHORTCUT_BITMAP_DIR"snap.bmp",CHORTCUT_BITMAP_DIR"snap.bmp");
		//SetChnElseStateBMP(i,EM_STATE_LINK,CHORTCUT_BITMAP_DIR"link.bmp",CHORTCUT_BITMAP_DIR"unlink.bmp");
		//SetChnElseStateBMP(i,EM_STATE_MKPHONE,CHORTCUT_BITMAP_DIR"Microphoneon.bmp",CHORTCUT_BITMAP_DIR"Microphoneoff.bmp");
		//SetChnElseStateBMP(i,EM_STATE_SOUND,CHORTCUT_BITMAP_DIR"soundon.bmp",CHORTCUT_BITMAP_DIR"soundoff.bmp");
	}
	
    int bShowTime = GetTimeDisplayCheck();
	ShowTimeTitle(0, bShowTime);

	
	
	
	int  x = 20/*555*/, y = 20;
	
	x = x * nScreanWidth / 720;
	y = y * nScreanHeight / 576;	
	
	if(nScreanWidth == 720)
	{
		x = 50;
		y = 26;//zlb20111118
		
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
		
		SBizSystemPara bizSysPara;
		BizGetPara(&bizTar, &bizSysPara);
		
		if(bizSysPara.nVideoStandard == EM_BIZ_PAL)
		{
			y = 35;
		}
	}
	
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
	char tmp2[20] = {0};
	GetProductNumber(tmp2);
	if(0 == strcasecmp(tmp2, "R3104HD") || 0 == strcasecmp(tmp2, "R3110HDW") || 0 == strcasecmp(tmp2, "R3106HDW"))//csp modify 20130516
	{
		if(nScreanWidth > 1024)
		{
			SetTimeTitleRect(0, CRect(x, y, x+160+108+66, y+TEXT_HEIGHT-2+18));//32x32-28
			//SetTimeTitleRect(0, CRect(x, y, x+160+108, y+TEXT_HEIGHT-2+10));//28x28-22
			//SetTimeTitleRect(0, CRect(x, y, x+160+92, y+TEXT_HEIGHT-2+10));//24x24-20
		}
		else
		{
			SetTimeTitleRect(0, CRect(x, y, x+160, y+TEXT_HEIGHT-2));
		}
	}
	else
	{
		SetTimeTitleRect(0, CRect(x, y, x+160, y+TEXT_HEIGHT-2));
	}
	#else
	SetTimeTitleRect(0, CRect(x, y, x+172, y+TEXT_HEIGHT-5));
	//SetTimeTitleRect(0, CRect(x, y, x+152, y+TEXT_HEIGHT-5));
	//SetTimeTitleString(0, sGsr.szValue);
	#endif
	
	ShowElecZoomTile(0, FALSE);
	SetElecZoomRect(0, CRect(nScreanWidth/4*3,nScreanHeight/8*7,nScreanWidth/4*3+164, nScreanHeight/8*7+TEXT_HEIGHT-2));
	SetElecZoomTileString(0, "&CfgPtn.ZoomAreaSet");
	
	ShowElecZoomInvalidTile(0, FALSE);
	SetElecZoomInvalidRect(0, CRect(nScreanWidth/3,nScreanHeight/6*5,nScreanWidth/3+400,nScreanHeight/6*5+TEXT_HEIGHT-5));
	SetElecZoomInvalidTileString(0, "&CfgPtn.InvalidAreaSet");
	
	SetElecZoomSplitLineRect(0, nScreanWidth/4*3, nScreanHeight/4*3,  nScreanHeight/4, EM_SPLIT_V);
	SetElecZoomSplitLineRect(1, nScreanWidth/4*3, nScreanHeight/4*3,  nScreanWidth/4, EM_SPLIT_H);
	
	for(split_h = 0; split_h < 2; split_h++)
	{
		SetElecZoomAreaSplitLineRect(split_h, 128, 128, 128, EM_SPLIT_H);
	}
	for(split_v = 0; split_v < 2; split_v++)
	{
		SetElecZoomAreaSplitLineRect(split_v + split_h, 128, 128, 128, EM_SPLIT_V);
	}
	
	Is3520 = 1;
	Is9616S = 0;
	
	char tmp[30] = {0};
	GetProductNumber(tmp);
	
	if((0 == strcasecmp(tmp, "R9504S")) || (0 == strcasecmp(tmp, "R9508S")))
	{
		Is3520 = 0;
	}
	
	//csp modify
	if((0 == strcasecmp(tmp, "R9504")) || (0 == strcasecmp(tmp, "R9508")))
	{
		Is3520 = 0;
	}
	if((0 == strcasecmp(tmp, "R9516S")) || (0 == strcasecmp(tmp, "R9516")))
	{
		Is3520 = 0;
	}
	
	if(0 == strcasecmp(tmp, "R9616S"))
	{
		Is9616S = 1;
	}
	
	alarmIcon = VD_LoadBitmap(DATA_DIR"/temp/alarm_icon.bmp");//"/mnt/alarm_icon.bmp"
	pAlarmIcon = CreateStatic(CRect(0, 0, 0, 0), this, (VD_BITMAP*)NULL);
	
	m_pPageStart->GetRect(&rect);
	m_alarmList = new CPageAlarmList(NULL, TEXT_ALARM_LIST, NULL, this);
	SetPage(EM_PAGE_ALARMLIST, m_alarmList);
	
	m_bAlarmListStatus = FALSE;
	m_DeskStreamTimer = NULL;
	
	printf("CPageDesktop::CPageDesktop..end\n");
}

void CPageDesktop::ToggleAlarmIcon(bool isVisible)
{
	//printf("CPageDesktop::ToggleAlarmIcon:(%d)\n", isVisible);
	
	m_bAlarmListStatus = isVisible;
	
	if(isVisible && !m_FlagPlaying)
	{
		CRect rt;
		rt.left = (m_RectScreen.right - alarmIcon->width) / 2;
		rt.top = (m_RectScreen.bottom - alarmIcon->height) / 2;
		rt.right = rt.left + alarmIcon->width;
		rt.bottom = rt.top + alarmIcon->height;
		pAlarmIcon->SetBitmap(alarmIcon);
		pAlarmIcon->SetRect(rt);
		pAlarmIcon->Show(TRUE);
	}
	else
	{
		pAlarmIcon->Show(FALSE, TRUE);
	}
}

void CPageDesktop::HideAllOsdWithoutTime(int nChnNum)
{
	//int maxChn = GetMaxChnNum();
	
	int maxSplit = 2;
	int maxStateType = 5;
	s32 i, j;
	
	if(nChnNum == 1)
	{
		for(i=0; i<maxSplit; i++)
		{
			ShowSplitLine(i,FALSE);
		}
	}
	else
	{
		for(i=0; i<maxSplit; i++)
		{
			ShowSplitLine(i,TRUE);
		}
	}
	for(i=0; i<maxChn; i++)
	{
		ShowChnName(i, FALSE);
		ShowChnKbps(i, FALSE);
		for(j=0; j<maxStateType; j++)
		{
			ShowChnState(i, j, 0, 0, EM_CHNSTATE_HIDE);
		}
		
		for(j=0; j<m_nMaxElseStatetype; j++)
		{
			ShowChnElseState( i, j, 0, 0, EM_CHNSTATE_HIDE);
		}
	}
}

void CPageDesktop::HideDeskOsdWithoutTime(int nMaxChn)//仅隐藏画线
{
	int maxStateType = 5;
	
	for(s32 i=0; i<10; i++)
	{
		ShowSplitLine(i,FALSE);
	}
	
	for(s32 i=0; i<maxChn; i++)
	{
		ShowChnName(i, FALSE);
	}
}

int CPageDesktop::isLogin = 0;

CPageDesktop::~CPageDesktop()
{
	
}

#if 0
/*事件处理*/
void CPageDesktop::onAppEvent(appEventCode code, int index, appEventAction action, EVENT_HANDLER *param, const CConfigTable* data)
{
	switch(code)
	{
	case appEventVideoTitle:
		break;
	case appEventVideoSplit:	
		break;
	case appEventVideoTour:
		break;
	default:
		break;
	}
}
#endif

VD_BOOL CPageDesktop::UpdateData(UDM mode)
{
	if(mode == UDM_OK || mode == UDM_CANCEL)
	{
		return FALSE;
	}
	
	//-----------------------------------------------------------------------
	// add by zhouwei[3/31/2005]
	//-----------------------------------------------------------------------
	if(mode == UDM_OPENED)
	{
		//m_UpdateTimer.Start(this, (VD_TIMERPROC)&CPageDesktop::OnTimer, 0, 200);
		//SetFocusItem(NULL); //
		
		//for(int i = 0; i < GetMaxChnNum(); i++) m_vChannelName[i]->SetText(m_vChannelName[i]->GetText());
	}
	
	if(mode == UDM_OPEN)
	{
		//printf("\n\n\n\n\n\n\n\n\n*****desktop open*********\n\n\n\n\n\n\n\n\n");
		m_pPageInputTray->Open();//打开输入法页面, 等桌面打开时，会触发隐藏输入法页面
		//OnTourStateChanged(m_pDisplay->IsTourStarted(), m_pDisplay->IsTourLocked()); //
		//add by liu
		//yaogang modify 20140918
		//int bshow = GetChnKbpsDisplayCheck();
		if (m_DeskStreamTimer == NULL)
		{
			m_DeskStreamTimer = new CTimer("DeskStream_Timer");
		}
		if (m_DeskStreamTimer != NULL)
		{
			m_DeskStreamTimer->Start(this, (VD_TIMERPROC)&CPageDesktop::ShowDeskStreamInfo, 1000,1000);
		}
	}
	
	//-----------------------------------------------------------------------
	
	return TRUE;
}

//由于桌面属于自绘页面, PT_NULL类型, 所以需要自己响应AdjustRect函数.
VD_BOOL CPageDesktop::AdjustRect()
{
	if (!CPage::AdjustRect())
	{
		return FALSE;
	}
	
	m_pDevGraphics->GetRect(&m_RectScreen);
	SetRect(&m_RectScreen, FALSE);
	//SetTimeTitle(m_ConfigGUISet[0].bTimeTitleEn, TRUE); //时间标题的坐标随分辨率变化
	
	return TRUE;
}

#if 0//csp modify
//只有一种切换方式的画面切换
void CPageDesktop::OnSetSpt()
{
#if 0
	if(lastspt >= 0 && lastspt < N_SPLIT)
	{
		int spt,sub;	
		m_pDisplay->GetSplit(&spt,&sub);
		if(spt!=g_iSptRule[lastspt] ||sub!=0)
		{
			OnLogoShow(spt,sub);
		}
		m_pDisplay->SetSplit(g_iSptRule[lastspt], 0);
	}
#endif
}

//切换单画面
void CPageDesktop::OnSetSpt1Chan()
{
#if 0
	int temp = 0;
	
	if(pSubMenu[g_iSptDef[SPLIT1]] != NULL)
    {
		temp = pSubMenu[g_iSptDef[SPLIT1]]->GetCurSel();
	}
	if(temp <0 && temp >=m_pDisplay->GetSplitIndexCount(SPLIT1))
	{
		temp = 0;
	}
	m_pDisplay->SetSplit(SPLIT1, temp);//进入指定单画面james.xu
	
	//D1 preview
	ViSetD1(temp);
#endif
}

void CPageDesktop::OnSetSpt2Chan()
{
#if 0
	int temp = 0;
	int spt,sub;	
	m_pDisplay->GetSplit(&spt,&sub);
	if(pSubMenu[g_iSptDef[SPLIT2]] != NULL)
    {
		temp = pSubMenu[g_iSptDef[SPLIT2]]->GetCurSel();
	}
	if(temp <0 && temp >=m_pDisplay->GetSplitIndexCount(SPLIT2))
	{
		 temp = 0;
	}
	if(spt!=SPLIT2||sub!=temp)
	{
		OnLogoShow(spt,sub);
	}
	m_pDisplay->SetSplit(SPLIT4, temp);	
#endif
}

void CPageDesktop::OnSetSpt4Chan()
{
#if 0
	int temp = 0;
	int spt,sub;	
	m_pDisplay->GetSplit(&spt,&sub);
	if(pSubMenu[g_iSptDef[SPLIT4]] != NULL)
    {
		temp = pSubMenu[g_iSptDef[SPLIT4]]->GetCurSel();
	}
	if(temp < 0 && temp >=m_pDisplay->GetSplitIndexCount(SPLIT4))
	{
		temp = 0;
	}
	if(spt!=SPLIT4 ||sub!=temp)
	{
		OnLogoShow(spt,sub);
	}
	m_pDisplay->SetSplit( SPLIT4, temp);
#endif
}

void CPageDesktop::OnSetSpt6Chan(void)
{
#if 0
	int temp =0;
	int spt,sub;	
	m_pDisplay->GetSplit(&spt,&sub);
	if(pSubMenu[g_iSptDef[SPLIT6]] != NULL)
	{
		temp = pSubMenu[g_iSptDef[SPLIT6]]->GetCurSel();
	}
	if(temp<0||temp>=m_pDisplay->GetSplitIndexCount(SPLIT6))
	{
		temp = 0;
	}
	if(spt!=SPLIT6 ||sub!=temp*6)
	{
		OnLogoShow(spt,sub);
	}
	m_pDisplay->SetSplit( SPLIT6, temp*6);
#endif
}

void CPageDesktop::OnSetSpt8Chan()
{
#if 0
	int temp =0;
	int spt,sub;	
	m_pDisplay->GetSplit(&spt,&sub);
	//changed by yanjun 20061106
	if(pSubMenu[g_iSptDef[SPLIT8]]!=NULL)
	{
		temp = pSubMenu[g_iSptDef[SPLIT8]]->GetCurSel();
	}
	if(temp<0||temp>=m_pDisplay->GetSplitIndexCount(SPLIT8))
	{
		temp = 0;
	}
	if(spt != SPLIT8 ||sub != temp*8)
	{
		OnLogoShow(spt,sub);
	}
	m_pDisplay->SetSplit( SPLIT8, temp*8);
#endif
}

void CPageDesktop::OnSetSpt9Chan()
{
#if 0
	int temp = 0;
	int spt,sub;	
	m_pDisplay->GetSplit(&spt,&sub);
	if(pSubMenu[g_iSptDef[SPLIT9]] != NULL)
    {
		temp = pSubMenu[g_iSptDef[SPLIT9]]->GetCurSel();
	}
	if(temp<0 ||temp>=m_pDisplay->GetSplitIndexCount(SPLIT9))
	{
		temp = 0;
	}
	if(spt!=SPLIT9 ||sub!=temp)
	{
		OnLogoShow(spt,sub);
	}
	m_pDisplay->SetSplit(SPLIT9, temp);
#endif
}

void CPageDesktop::OnSetPIPChan()
{
#if 0
	int largech = 0;
	int smallch = 0;
	int group = 4;
	
	if(pPOPMenu != NULL)
	{
		largech = (pPOPMenu->GetCurSel()) * group;
		for(int i = largech+1; i < MIN(largech + group, g_nCapture); i++)
		{
			smallch |= BITMSK(i);
		}
		m_pDisplay->SetSplit(SPLITPIP, smallch << 16 |largech);
	}
#endif
}

/******************************************************************
在切换画面前先让LOGO图片不显示，以免图片压住分隔线。
没有对六画面和画中画模式进行处理added by chenwh for task14316
*******************************************************************************/
void CPageDesktop::OnLogoShow(int _spt,int _sub)
{
#if 0
	int i,spt,sub;
	spt = _spt;
	sub =_sub;
	uint dwChCoverState = m_pDisplay->GetChannelLockState() | g_Alarm.GetLossDetectState();
	
	int image[SPLIT_NR]={1, 2, 4, 8, 9, 16, 0, 6, 12};
	
	if(spt==SPLIT9)
	{
		if(sub==0)
		{
			for(i=0;i<image[SPLIT9];i++)
			{
				if(bmp_logo_middle && (dwChCoverState & BITMSK( i)))
					m_pStaticChannelLogo[i]->Show(FALSE);
			}
		}
		else
		{
			for(i=0;i<image[SPLIT9];i++)
			{
				if(bmp_logo_middle && (dwChCoverState & BITMSK( 7 + i)))
					m_pStaticChannelLogo[7 + i]->Show(FALSE);
			}
		}
	}
	else if(spt==SPLIT8)
	{
		if(sub >= 0 && sub < 8)
		{
			for(i=0;i<image[SPLIT8];i++)
			{
				if(bmp_logo_middle && (dwChCoverState & BITMSK( i)))
					m_pStaticChannelLogo[i]->Show(FALSE);
			}
		}
		else
		{
			for(i=0;i<image[SPLIT8];i++)
			{
				if(bmp_logo_middle && (dwChCoverState & BITMSK( 8 + i)))
					m_pStaticChannelLogo[8 + i]->Show(FALSE);
			}
		}
	}
	else if(spt == SPLIT6 || spt == SPLIT12)
	{
		//暂不处理
	}
	else
	{
		for(i=0;i<image[spt];i++)
		{
			if(bmp_logo_middle && (dwChCoverState & BITMSK(sub * image[spt] + i)))
				m_pStaticChannelLogo[sub * image[spt] + i]->Show(FALSE);
		}
	}
#endif
}

void CPageDesktop::OnPtzCtrl()
{
#if 0
	int ch;
	int spt, sub;
	ch = m_pDisplay->GetChannelAt(mouse_spt_x, mouse_spt_y);
	if(ch < 0)
	{
		return;
	}
	m_pDisplay->GetSplit(&spt, &sub);
	if(spt != SPLIT1)
	{
		m_pDisplay->SetSplit(SPLIT1, ch);
	}
	
	m_pPageFunction->pPageControlPtz->Open();
#endif
}

void CPageDesktop::OnImgConf()
{
#if 0
	int ch;
	int spt, sub;
	
	ch = m_pDisplay->GetChannelAt(mouse_spt_x, mouse_spt_y);
	if(ch < 0)
	{
		return;
	}
	m_pDisplay->GetSplit(&spt, &sub);
	if( spt != SPLIT1)
	{
		m_pDisplay->SetSplit(SPLIT1, ch);
	}
	
	m_pPageFunction->pPageImageColor->Open();
#endif
}

void CPageDesktop::OnTvAjust()
{
	CItem *pItem = FindPage(m_screen, "&titles.tvadjust");
	if(pItem)
	{
		pItem->Open();
	}
	else
	{
		//tracepoint();
	}
}

int isPreview = 0;
void CPageDesktop::OnMenuSplit()
{
	//m_pPageMain->pPagePlay->Open();
	
	printf("choose a preview mode\n");
	
	isPreview = !isPreview;
	
	if(isPreview)
	{
		SBizPreviewPara sBPP;
		sBPP.emBizPreviewMode = EM_BIZPREVIEW_1SPLIT;
		sBPP.nModePara = 0;
		BizStartPreview(&sBPP);
	}
	else
	{
		BizStopPreview();
	}
}

#ifdef FUNC_DISK_PLAY
void CPageDesktop::OnBackupPlay()
{
	//m_pPageMain->m_pPagePlay->OnBackupPlay();
}
#endif

void CPageDesktop::OnPlayback()
{
	CItem *pItem = FindPage(m_screen, "&titles.record");
	if(pItem)
	{
		pItem->Open();
	}
	else
	{
		//tracepoint();
	}
}

void CPageDesktop::OnBackUp()
{
	CItem *pItem = FindPage(m_screen, "&titles.backup");
	if(pItem)
	{
		pItem->Open();
	}
	else
	{
		//tracepoint();
	}
/*
	//2种方法打开均可
	m_pPageMain->pPageBackup->Open(); //james.xu backup
*/
}

//added by wyf on 090922
void CPageDesktop::OnSystemSilence()
{
#if 0
	int menuCurSel = pMainMenu->GetCurSel();
	CConfigGeneral	m_CCfgGeneral;
	m_CCfgGeneral.update();
	CONFIG_GENERAL &cfgGeneral = m_CCfgGeneral.getConfig();
	//主菜单显示的是"取消静音"字样，将"取消静音"变成"静音"
	if(cfgGeneral.bSilence)
	{
		cfgGeneral.bSilence = FALSE;
		//调"取消静音"的接口
		SystemSilence(cfgGeneral.bSilence);
		pMainMenu->SetItemText(menuCurSel, LOADSTR("pgquick.systemsilence"), FALSE);		
		pMainMenu->SetItemBmp(menuCurSel, bmp_systemsilence[0], bmp_systemsilence[1]); //设置一个静音的图片 add langzi 2009-11-9
	}
	else//主菜单显示的是"静音"字样，将"静音"变成"取消静音"
	{
		cfgGeneral.bSilence = TRUE;
		//调"静音"的接口
		SystemSilence(cfgGeneral.bSilence);
		pMainMenu->SetItemText(menuCurSel, LOADSTR("pgquick.systemnosilence"), FALSE);			
		pMainMenu->SetItemBmp(menuCurSel, bmp_systemnosilence[0], bmp_systemnosilence[1]); //设置一个另一静音图片 add langzi 2009-11-9		
	}
	m_CCfgGeneral.commit();
#endif
}   
//end added by wyf on 090922

void CPageDesktop::OnBackupOnline()
{
	CItem *pItem = FindPage(m_screen, "&bkup_data.startrecord");
	if(pItem)
	{
		pItem->Open();
	}
	else
	{
		//tracepoint();
	}
}

//报警输出功能回调
void CPageDesktop::OnCtrlAlmOut()
{
	CItem *pItem = FindPage(m_screen, "&titles.alo");
	if(pItem)
	{
		pItem->Open();
	}
	else
	{
		//tracepoint();
	}
}

//报警输入功能回调
void CPageDesktop::OnCtrlAlmIn()
{
	CItem *pItem = FindPage(m_screen, "&titles.ali");
	if(pItem)
	{
		pItem->Open();
	}
	else
	{
		//tracepoint();
	}
}

void CPageDesktop::OnMenu()
{
	m_pPageMain->Open();
}

void CPageDesktop::On1x1()
{
	//printf("choose a preview mode\n");
	
	isPreview = !isPreview;
	
	if(isPreview)
	{
		SBizPreviewPara sBPP;
		sBPP.nModePara = 0;
		sBPP.emBizPreviewMode = EM_BIZPREVIEW_1SPLIT;
		
		BizStartPreview(&sBPP);
	}
	else
	{
		BizStopPreview();
	}
}

void CPageDesktop::OnColor()
{
	
}

void CPageDesktop::OnSnap()
{
	
}

void CPageDesktop::OnPlay()
{
	m_pPagePlayBack->Open();
}

void CPageDesktop::OnEdit()
{
	
}

void CPageDesktop::OnTriangle()
{
	
}

void CPageDesktop::On2x2()
{
	
}

void CPageDesktop::OnDwellNo()
{
	
}

void CPageDesktop::OnZoom()
{
	
}

void CPageDesktop::OnAudio()
{
	
}

void CPageDesktop::OnPtz()
{
	
}
#endif

void CPageDesktop::OnRec()
{
	SBizParaTarget bizTar;
	SBizRecPara bizRecPara;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	
	int chnnum = GetVideoMainNum();
	int log_status=0;
	int count_flag=0;
	
	for(int i = 0; i < chnnum; i++)
	{
		bizTar.nChn = i;
		if(0 != BizGetPara(&bizTar, &bizRecPara))
		{
			printf("Get rec para error\n");
		}	
		
		if(!bizRecPara.bRecording&&bizRecPara.nEnable)
		{
			count_flag++;
			log_status=1;
			BizStartManualRec(i);
			bizRecPara.bRecording = !bizRecPara.bRecording;
			if(0 != BizSetPara(&bizTar, &bizRecPara))
			{
				printf("Set rec para error\n");
			}
		}
	}
	
	if(!count_flag)
	{
		for(int i = 0; i < chnnum; i++)
		{
			bizTar.nChn = i;
			if(0 != BizGetPara(&bizTar, &bizRecPara))
			{
				printf("Get rec para error\n");
			}
			
			if(bizRecPara.bRecording&&bizRecPara.nEnable)
			{
				log_status=2;
				BizStopManualRec(i);
				bizRecPara.bRecording = !bizRecPara.bRecording;
				if(0 != BizSetPara(&bizTar, &bizRecPara))
				{
					printf("Set rec para error\n");
				}
			}
		}
	}
	
 	if(log_status==1)
	{
		BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_START_MANUAL_REC);
	}
	else if (log_status==2)
	{
		BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_STOP_MANUAL_REC);
	}
	else
		printf("do nothing!\n");
}

#if 0//csp modify
void CPageDesktop::OnMatrix()
{
	CItem *pItem = FindPage(m_screen, "&titles.videomatrix");
	if (pItem)
	{
		pItem->Open();
	}
	else
	{
		//tracepoint();
	}
}

void CPageDesktop::OnMain()
{
	m_pPageMain->Open();
}

void CPageDesktop::OnLogout()
{
	CItem *pItem = FindPage(m_screen, "&titles.logout");
	
	if(pItem)
	{
		pItem->Open();
	}
	else
	{
		//tracepoint();
	}
}

void CPageDesktop::OnDelay(uint arg)
{
	VD_SendMessage(XM_OPENMAINPAGE);
}

void CPageDesktop::runLButtonDown(uint lpa) //单击操作add langzi 2009-11-9
{
#if 0
   	int px,py;
	int ch;
	int spt, sub; 
    
    px = VD_HIWORD(lpa);
	py = VD_LOWORD(lpa);
	//add by xie
	mouse_spt_x = px;
	mouse_spt_y = py;
	//end
	ch = m_pDisplay->GetChannelAt(px, py);
	if(ch >= 0)
	{//2
		m_pDisplay->GetSplit(&spt, &sub);
		if(spt == SPLIT1)//从单画面返回james.xu
		{//3
			OnLogoShow(spt, sub);
			m_pDisplay->RecoverSplit(ch);
			
			//restore CIF preview
			ViSetCIF(ch);
		}//3
		else if (spt == SPLITPIP)
		{//3
			int startch = ch - ch % 4;
			int chamask = 0;
			for (int i = startch; i < MIN(startch + 4, g_nCapture); i++)
			{
				if (i != ch)
				{
					chamask |= BITMSK(i);
				}
			}
			if (sub != ((chamask << 16) | ch))
			{
				m_pDisplay->SetSplit(SPLITPIP, chamask << 16 |ch);
			}
		}//3
		else
		{//3
			if(spt == SPLIT6 ||spt == SPLIT8)
			{
				if(sub != ch)
				{
					OnLogoShow(spt, sub);
				}
				m_pDisplay->SetSplit(spt, ch);
			}
			else //进入最大单画面james.xu
			{
				OnLogoShow(spt, sub);
				m_pDisplay->SetSplit(SPLIT1, ch);
				//D1 preview
				ViSetD1(ch);
			}
		}//3
	}//2
#endif
}
#endif

void CPageDesktop::SetLogin(int login)
{
	isLogin = login;
}

VD_BOOL CPageDesktop::MsgProc(uint msg, uint wpa, uint lpa)
{
	uchar key;
	//int px,py;
	int ch;
	int spt, sub;
	VD_RECT rect;
	//wangq added 20060105:使用static类型变量的原因是采用了延迟，单击事件导致menu被弹出采用消息机制，这个消息没有x y的值，所以必须对此进行保留。
	static int sppx, sppy;
	static uchar chnnum = GetVideoMainNum();
	static struct timeval nLastKeyDowm = {0, 0};
	struct timeval nKeyDowm;
	static uint nLastKey = 0;
	//int maxChn = GetMaxChnNum();
	
	//printf("CPageDesktop::MsgProc this=0x%08x\n",this);
	
	u8 lock_flag = 0;//cw_shutdown
	GetSystemLockStatus(&lock_flag);
	if(lock_flag)
	{
		return FALSE;
	}
	
	//没有Shift键，使用Function键来切换视频。
	if(wpa == KEY_SHIFT || (!GetKeyInputFlag(KIF_SHIFT) && wpa == KEY_FUNC))//切换视频输出类型
	{
		return TRUE;
	}
	
	if(msg == XM_OPENMAINPAGE)
	{
		//return m_pPageMain->Open();
		return TRUE;
	}
	
	//printf("CPageDesktop::MsgProc msg=0x%08x\n",msg);
	
	/*
	if(msg == XM_LBUTTONDBLCLK)
	{
		m_PageMainTimer.Stop();
	}
	*/
	
	if(msg == XM_KEYDOWN)//cw_panel
	{
		if(wpa == KEY_MENU)
		{
			wpa = KEY_MAIN;
		}
		
		if(wpa == KEY_SPLIT)//cw_panel
		{
			if(!m_IsElecZoomDrawStatus && !m_IsPbElecZoomDrawStatus)
			{
				PreviewToNextMod();
				return TRUE;
			}
		}
		
		if(wpa == KEY_ESC)//cw_panel
		{
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);
			if(IsElecZoomBegin)
			{
				//printf("放大模式下右键退出\n");
				ExitFromZoomStatue();
			}
			else if(m_IsElecZoomDrawStatus || m_IsPbElecZoomDrawStatus)
			{
				int ret = ExitFromDrawStatue();
				if(ret < 0)
				{
					return TRUE;
				}
			}
			else if((!m_IsPbElecZoomDrawStatus) && (!m_IsElecZoomDrawStatus))
			{
				mouse_spt_x = px = 0;
				mouse_spt_y = py = 560;
				
				#if 0//csp modify
				if(m_PageMainTimer.IsStarted())
				{
					m_PageMainTimer.Stop();
				}
				
				SetSilenceMenu();//added by wyf on 090925
				#endif
				
				if(!isLogin)
				{
					m_pPageLogin->Open();
				}
				else
				{
					m_pPageStart->Open();
				}
			}
			
			return 1;
		}
		
		if((wpa == KEY_UP) || (wpa == KEY_DOWN) || 
			(wpa == KEY_LEFT) || (wpa == KEY_RIGHT) || 
			 (wpa == KEY_RET))
		{
			//继续
		}
		else if(wpa == KEY_REC)
		{
			if(!isLogin)
			{
				m_pPageLogin->Open();
			}
		#if 1//csp modify 20130326
			else
			{
				OnRec();
			}
		#else
			OnRec();
		#endif
		}
		else if((wpa == KEY_REC) || (wpa == KEY_PTZ) || 
				(wpa == KEY_PLAY) || (wpa == KEY_SEARCH))
		{
			if(!isLogin)
			{
				m_pPageLogin->Open();
			}
			
			return CPage::MsgProc(msg, wpa, lpa);
		}
		else
		{
		 	if(!m_IsElecZoomDrawStatus && !m_IsPbElecZoomDrawStatus)
		 	{
				if(CPage::MsgProc(msg, wpa, lpa))
				{
					return TRUE;
				}
		 	}
		}
	}
	
	#if 0//csp modify 20130509
	if(msg == XM_TIMETICK)
	{
		//printf("msg == XM_TIMETICK\n");
		
		static int count = 1000000;
		char szCount[16] = {0};
		
		sprintf(szCount,"%d",count++);
		
		SetTimeTitleString(0, szCount);
	}
	#endif
	
	/*if(msg == XM_LBUTTONDOWN)
	{
		printf("msg == XM_LBUTTONDOWN\n");
	}
	*/
	
	if(msg == XM_LBUTTONDOWN)
	{
		//重要!!保存鼠标左键按下时侯的x、y值便于下次处理，如果需要的话。原因是双击事件之前，XM_LBUTTONDOWN事件先发生。
		sppx = VD_HIWORD(lpa);
		sppy = VD_LOWORD(lpa);
		
		if((!m_IsElecZoomDrawStatus) && (!m_IsPbElecZoomDrawStatus))
		{
			//触摸屏上按下通道名称时, 弹出右键菜单.
			key = wpa;
			if((key & MOUSE_TOUCHSCREEN))
			{
				px = VD_HIWORD(lpa);
				py = VD_LOWORD(lpa);
				//ch = m_pDisplay->GetChannelAt(px, py);
				//ch = 0;
				//if(ch >= 0)
				//{
					//m_pStaticChannelTitle[ch]->GetRect(&rect);
					//if(py >= rect.top && py <= rect.bottom)
					//{
					//	mouse_spt_x = px;
					//	mouse_spt_y = py;
					//	pMainMenu->TrackPopup(px, py);
					//	KillFocus(); //使鼠标双击无效
					//	return TRUE;
					//}
				//}
			}
			
			//add by xie
			mouse_spt_x = sppx;
			mouse_spt_y = sppy;
			//end
			
			//CRect m_rect;
			//pMainMenu->GetRect(&m_rect);
			//printf("l:%d t:%d r:%d b:%d \n",m_rect.left,m_rect.top,m_rect.right,m_rect.bottom);
			//if(PtInRect(pMainMenu, sppx, sppx))
			//{
			//	return 1;
			//}
			
			if(pMainMenu->IsOpened())
			{
				pMainMenu->Close();
			}
			else
			#if 0//csp modify
			#ifdef MON_ADDLOGOUT_FOR_LOGINUSR
			if(pMainMenuAddLogout->IsOpened())
			{
				pMainMenuAddLogout->Close();
			}
			else
			#endif
			#endif
			{
				#if 0//csp modify
				//m_PageMainTimer.Start(this, (TIMERPROC)&CPageDesktop::OnDelay, EXCEED_MS, 0);//delect langzi 2009-11-9
				m_PageMainTimer.Start(this, (VD_TIMERPROC)&CPageDesktop::runLButtonDown, EXCEED_MS, 0, lpa);//add langzi 2009-11-9
				#endif
			}
		}
		else if(IsElecZoomBegin)
		{
			if(0 == IsLbuttonDown)
			{
				mouse_eleczoom_x = sppx;
				mouse_eleczoom_y = sppy;
				IsLbuttonDown = 1;
			}
			//printf("放大模式下左键\n");
		}
		else
		{
			mouse_eleczoom_x = sppx;
			mouse_eleczoom_y = sppy;
			
			IsDrawElecZoomArea = 1;
			//printf("电子放大区域设置起点\n");
		}
	}

	CRect alarmIconRect;
	switch(msg) 
	{
		case XM_LBUTTONDOWN:
		{
			pAlarmIcon->GetRect(&alarmIconRect);
			if(alarmIconRect.PtInRect(px, py) && pAlarmIcon->IsShown())
			{
				if(!isLogin)
				{
					m_pPageLogin->Open();
				}
				else
				{
					m_alarmList->Open();
				}
			}

			int i, nIdx;
			CRect SnapRect;
			for (i=0; i<chnnum; i++)
			{
				nIdx = (m_nMaxElseStatetype*i + EM_STATE_SNAP)*2;
				m_vChnElseState[nIdx]->GetRect(&SnapRect);
				if (m_vChnElseState[nIdx]->IsShown() && SnapRect.PtInRect(px, py))
				{
					printf("%s chn%d snap\n", __func__, i);
					BizRequestSnap(i, 7, NULL, NULL);
				}
			}
				
		}break;
		case XM_LBUTTONDBLCLK:
			#if 0//csp modify
	        m_PageMainTimer.Stop();
	        #endif
			
	        px = VD_HIWORD(lpa);
	        py = VD_LOWORD(lpa);
			
	        mouse_spt_x = px;
	        mouse_spt_y = py;
/*
		  pAlarmIcon->GetRect(&alarmIconRect);
            if(alarmIconRect.PtInRect(px, py) && pAlarmIcon->IsShown())
            {
                if(!isLogin)
                {
                    m_pPageLogin->Open();
                }
                else
                {
                    m_alarmList->Open();
                }
                break;
            }
*/			
			emCurPreviewMode = GetCurPreviewMode_CW();//cw_preview
			
			//printf("XM_LBUTTONDBLCLK-1:(%d,%d)\n",mouse_spt_x,mouse_spt_y);
			
			if(m_IsElecZoomDrawStatus || IsElecZoomBegin || m_IsPbElecZoomDrawStatus)
			{
				//printf("绘画区域模式或者放大模式下双击\n");
				break;
			}
			
			//printf("###XM_LBUTTONDBLCLK-2:(%d,%d),emCurPreviewMode:%d\n",mouse_spt_x,mouse_spt_y,emCurPreviewMode);
		//printf("yg emCurPreviewMode: %d\n", emCurPreviewMode);
	        if(EM_BIZPREVIEW_1SPLIT != emCurPreviewMode)
	        {
	        	if(EM_BIZPREVIEW_PIP == emCurPreviewMode)//csp modify 20130501
				{
					//printf("here-1\n");
					emLastPreMode = EM_BIZPREVIEW_PIP;
					nPreviewPage = 0;
				}
				else
				{
					//printf("yg eMaxViewMode: %d\n", eMaxViewMode);
					switch(eMaxViewMode)
					{
						case EM_BIZPREVIEW_4SPLITS:
						{
							nPreviewPage = 0;
							emLastPreMode = EM_BIZPREVIEW_4SPLITS;
						}
						break;
						case EM_BIZPREVIEW_9SPLITS:
						{
							if(EM_BIZPREVIEW_4SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_4SPLITS;
							}
							else
							{
								emLastPreMode = EM_BIZPREVIEW_9SPLITS;
								nPreviewPage = 0;
							}
						}
						break;
						case EM_BIZPREVIEW_10SPLITS:
						{
							if(EM_BIZPREVIEW_4SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_4SPLITS;
							}
							else
							{
								emLastPreMode = EM_BIZPREVIEW_10SPLITS;
								nPreviewPage = 0;
							}
						}
						break;
						case EM_BIZPREVIEW_6SPLITS:
						{
							if(EM_BIZPREVIEW_4SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_4SPLITS;
							}
							else
							{
								emLastPreMode = EM_BIZPREVIEW_6SPLITS;
								nPreviewPage = 0;
							}
						}
						break;
						case EM_BIZPREVIEW_16SPLITS:
						{
							if(EM_BIZPREVIEW_4SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_4SPLITS;
							}
							else if(EM_BIZPREVIEW_9SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_9SPLITS;
							}
							else
							{
								emLastPreMode = EM_BIZPREVIEW_16SPLITS;
								nPreviewPage = 0;
							}
						}
						break;
						case EM_BIZPREVIEW_24SPLITS:
						{
							if(EM_BIZPREVIEW_4SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_4SPLITS;
							}
							else if(EM_BIZPREVIEW_9SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_9SPLITS;
							}
							else if(EM_BIZPREVIEW_16SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_16SPLITS;
							}
							else
							{
								emLastPreMode = EM_BIZPREVIEW_24SPLITS;
								nPreviewPage = 0;
							}
						}
						break;
						case EM_BIZPREVIEW_25SPLITS:
						{
							if(EM_BIZPREVIEW_4SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_4SPLITS;
							}
							else if(EM_BIZPREVIEW_9SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_9SPLITS;
							}
							else if(EM_BIZPREVIEW_16SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_16SPLITS;
							}
							else
							{
								emLastPreMode = EM_BIZPREVIEW_25SPLITS;
								nPreviewPage = 0;
							}
						}
						break;
						case EM_BIZPREVIEW_36SPLITS:
						{
							if(EM_BIZPREVIEW_4SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_4SPLITS;
							}
							else if(EM_BIZPREVIEW_9SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_9SPLITS;
							}
							else if(EM_BIZPREVIEW_16SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_16SPLITS;
							}
							else if(EM_BIZPREVIEW_24SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_24SPLITS;
							}
							else if(EM_BIZPREVIEW_25SPLITS == emCurPreviewMode)
							{
								emLastPreMode = EM_BIZPREVIEW_25SPLITS;
							}
							else
							{
								emLastPreMode = EM_BIZPREVIEW_36SPLITS;
								nPreviewPage = 0;
							}
						}
						break;
						default:
						break;
					}
				}
				
				//printf("XM_LBUTTONDBLCLK-3:(%d,%d),emCurPreviewMode:%d\n",mouse_spt_x,mouse_spt_y,emCurPreviewMode);
				
				SetPageFlag(TRUE);
				
				u32 nClikChn = ClikCurChn(emCurPreviewMode, nPreviewPage, px, py);
				
				//printf("XM_LBUTTONDBLCLK-4:(%d,%d),emCurPreviewMode:%d,nPreviewPage:%d,nClikChn:%d\n",mouse_spt_x,mouse_spt_y,emCurPreviewMode,nPreviewPage,nClikChn);
				
	            if(EM_BIZPREVIEW_10SPLITS == emCurPreviewMode)
	            {
	            	if((nClikChn == 0) || (nClikChn == 1) || (nClikChn == 4) || (nClikChn == 5))
	            	{
	            		nClikChn = 0;
	            	}
	            	else if((nClikChn == 2) || (nClikChn == 3) || (nClikChn == 6) || (nClikChn == 7))
	            	{
	            		nClikChn = 1;
	            	}
	            	else
	            	{
	            		nClikChn -= 6;
	            	}
	            }
		else if(EM_BIZPREVIEW_6SPLITS == emCurPreviewMode)
	            {
	            	if((nClikChn == 0) || (nClikChn == 1) || (nClikChn == 4) || (nClikChn == 5))
	            	{
	            		nClikChn = 0;
	            	}
	            	else if((nClikChn == 2) || (nClikChn == 3) || (nClikChn == 6) || (nClikChn == 7))
	            	{
	            		nClikChn = 1;
	            	}
	            	else
	            	{
	            		nClikChn = (nClikChn-4)/2;
	            	}
	            }
		else if(EM_BIZPREVIEW_PIP == emCurPreviewMode)//csp modify 20130501
				{
					int nModePara = m_pPageStart->GetCurPreview();
					int large = (nModePara >> 4) & 0xf;
					int small = (nModePara >> 0) & 0xf;
					
					//printf("nClikChn=%d,nModePara=0x%08x,large=%d,small=%d\n",nClikChn,nModePara,large,small);
					
					nClikChn = (nClikChn % 4);
					if(nClikChn == 3)
					{
						nClikChn = small;
					}
					else
					{
						nClikChn = large;
					}
				}
				
				//点击不在范围内或者无用通道
				if(8 == maxChn)
				{
					if(-1 == nClikChn || 8 == nClikChn)
					{
						break;
					}
				}
				else if((24 == maxChn) || (32 == maxChn))
				{
					if(-1 == nClikChn || nClikChn >= maxChn)
					{
						break;
					}
				}
				else
				{
					if(255 == nClikChn)
					{
						break;
					}
				}
				
	            emCurPreviewMode = EM_BIZPREVIEW_1SPLIT;
	            m_pPageStart->SinglePreview(TRUE);
		//printf("%s emCurPreviewMode: %d, nClikChn: %d\n", __func__, emCurPreviewMode, nClikChn);		
		   SetCurPreviewMode_CW(emCurPreviewMode);//cw_preview
	            SwitchPreview(emCurPreviewMode, nClikChn);
	        }
	        else
	        {
			//printf("yg bThisPageFlag: %d\n", bThisPageFlag);
			//printf("yg 2 emCurPreviewMode: %d\n", emCurPreviewMode);
			
	            if(bThisPageFlag)
	            {
	            	//回到上次预览的页面
	                emCurPreviewMode = emLastPreMode;
	                SetCurPreviewMode_CW(emCurPreviewMode);//cw_preview
	                BizResumePreview(0);
	            }
	            else
	            {
	                emCurPreviewMode = eMaxViewMode;
					SetCurPreviewMode_CW(emCurPreviewMode);//cw_preview
	                SwitchPreview(emCurPreviewMode, 0);
	            }
	            m_pPageStart->SinglePreview(FALSE);
	        }
			break;
		case XM_RBUTTONDOWN:
		case XM_RBUTTONDBLCLK:
		{
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);
			if(IsElecZoomBegin)
			{
				//printf("放大模式下右键退出\n");
				ExitFromZoomStatue();
			}
			else if(m_IsElecZoomDrawStatus || m_IsPbElecZoomDrawStatus)
			{
				int ret = ExitFromDrawStatue();
				
				if(ret < 0)
				{
					break;
				}
			}
			else if((!m_IsPbElecZoomDrawStatus) && (!m_IsElecZoomDrawStatus))
			{
				mouse_spt_x = px = 0;
				mouse_spt_y = py = 560;
				
				#if 0//csp modify
				if(m_PageMainTimer.IsStarted())
				{
					m_PageMainTimer.Stop();
				}
				//SetSilenceMenu();//added by wyf on 090925
				#endif
				
				//printf();
				if(!isLogin)
				{
					m_pPageLogin->Open();
				}
				else
				{
					m_pPageStart->Open();
				}
			}
		}
		break;
		//add by xie
		case XM_MOUSEMOVE:
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);
			
			mouse_spt_x = px;
			mouse_spt_y = py;
			
			if((m_IsPbElecZoomDrawStatus || m_IsElecZoomDrawStatus) && IsDrawElecZoomArea && !IsElecZoomBegin)
			{
				int ret = DrawZoomArea();
				
				if(ret < 0)
				{
					break;
				}
			}
			else if(IsElecZoomBegin && !IsDrawElecZoomArea)
			{
				if(IsElecZoomStatue && IsLbuttonDown)
				{
					int ret = ChangZoomArea();
					
					if(ret < 0)
					{
						break;
					}
				}
				//printf("放大模式下拖动鼠标,重设矩形区域\n");
			}
			break;
		//end
		case XM_LBUTTONUP:
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);
			
			if (IsElecZoomStatue)
			{
				IsLbuttonDown = 0;
			}
			
			if((m_IsPbElecZoomDrawStatus || m_IsElecZoomDrawStatus) && IsDrawElecZoomArea)
			{
				int ret = StartZoomVideoArea();
				
				if(ret < 0)
				{
					break;
				}
			}
			break;
		case XM_KEYDOWNDELAY:
		case XM_KEYDOWN:
			if(m_IsPbElecZoomDrawStatus || m_IsElecZoomDrawStatus)
			{
				break;
			}
			
			key = wpa;
			switch(key) {
			case KEY_LEFT:
				PreviewToNextMod();
				break;
			case KEY_RIGHT:
				PreviewToLastMod();
				break;
			case KEY_UP:
				PreviewToLastPic();
				break;
			case KEY_DOWN:
				//printf("KEY_DOWN - PreviewToNextPic\n");
				PreviewToNextPic();
				break;
			case KEY_0:
			case KEY_1:
			case KEY_2:
			case KEY_3:
			case KEY_4:
			case KEY_5:
			case KEY_6:
			case KEY_7:
			case KEY_8:
			case KEY_9:
			{
				if(msg == XM_KEYDOWNDELAY)
				{
					SetCurPreviewMode_CW(EM_BIZPREVIEW_1SPLIT);//cw_preview
					SwitchPreview(EM_BIZPREVIEW_1SPLIT, key - KEY_1);
					//printf("&&&&&&&&&&&&&&&&&&&&&&&&&&to chn[%d]\n", key - KEY_1 +1);
					break;
				}
				
				if(chnnum < 10)
				{
					nLastKey = 0;
				}
				else
				{
					gettimeofday(&nKeyDowm, NULL);
					if(((unsigned long long)nKeyDowm.tv_sec * 1000 + nKeyDowm.tv_usec / 1000)
						> ((unsigned long long)nLastKeyDowm.tv_sec * 1000 + nLastKeyDowm.tv_usec / 1000) + 700)
					{
						nLastKey = 0;
					}
				}
				if(!nLastKey)
				{
					if(chnnum >= 10)
					{
						if(chnnum/10 < key-1)
						{
							SetCurPreviewMode_CW(EM_BIZPREVIEW_1SPLIT);//cw_preview
							SwitchPreview(EM_BIZPREVIEW_1SPLIT, key - KEY_1);
							//printf("1*to chn[%d]\n", key - KEY_1 +1);
						}
						else
						{
							//printf("1.2*to chn[%d]\n", key - KEY_1 +1);
							nLastKey = key - 1;
							gettimeofday(&nLastKeyDowm, NULL);
						}
					}
					else
					{
						if((key>KEY_0) && ((key-2)<chnnum))
						{
							SetCurPreviewMode_CW(EM_BIZPREVIEW_1SPLIT);//cw_preview
							SwitchPreview(EM_BIZPREVIEW_1SPLIT, key - KEY_1);
							//printf("2*to chn[%d]\n", key - KEY_1 +1);
						}
					}
				}
				else
				{
					if(chnnum >= nLastKey * 10 + key - 1)
					{
						SetCurPreviewMode_CW(EM_BIZPREVIEW_1SPLIT);//cw_preview
						SwitchPreview(EM_BIZPREVIEW_1SPLIT, nLastKey * 10 + key - 2);
						//printf("3*to chn[%d]\n",  nLastKey * 10 + key-1);
					}
					else
					{
						SetCurPreviewMode_CW(EM_BIZPREVIEW_1SPLIT);//cw_preview
						SwitchPreview(EM_BIZPREVIEW_1SPLIT, key - 2);
						//printf("4****to chn[%d]\n", key - 1);
					}
					nLastKey = 0;
				}
				break;
			}
			case KEY_RET:
			case KEY_MAIN:
			case KEY_MENU:
				{
					#if 0//csp modify
					if(m_PageMainTimer.IsStarted())
					{
						m_PageMainTimer.Stop();
					}
					#endif
					
					if(mouse_spt_x < 0)
					{
						mouse_spt_x = 250;
					}
					if(mouse_spt_y < 0)
					{
						mouse_spt_y = 50;
					}
					
					#if 0//csp modify
					SetSilenceMenu();
					#endif
					
					if(!isLogin)
					{
						m_pPageLogin->Open();
					}
					else
					{
						//printf("startpage open\n");
						m_pPageStart->Open();
					}
				}
				break;
				
#ifdef MON_NETKEY_TOUR
			case KEY_AUTOTUR:
				//printf(">>>>>>>>>>>>>>>>>>>>>Get net key board tour key!\n");
				OnTourSwitch();
				break;
#endif
			default:
				break;
			}
			break;
		case XM_SHUTDOWNTIMEUP:	//如果有关机密码，弹出密码框
			//pPageCountDown->Close();
			//m_ShutTimer.Stop();
#if 0
#ifndef FUNC_ELISOR_BURN
			if(!g_localUser.isValidAuthority("ShutDown"))
			{
				if(g_CapsEx.ForATM == 2)
				{
					m_pDevVideo->SwitchTVMonitor(TRUE);
				}
				int tmpnum = PasswordBox("ShutDown");
				if(tmpnum != UDM_OK)
				{
					return FALSE;
				}
			}
#endif
#endif
			//g_Challenger.Shutdown();
			
			//m_ShutCountDown = 0;//如果关机没有成功，重置变量 		
			//m_ShutTimer.Stop();
			break;
		default:
			return FALSE;
	}
	
	return TRUE;
}

#if 0
void CPageDesktop::SetCustomTitle(int iChannel, int iIndex, VD_PCSTR pLable, VD_PCSTR pContext)
{

	//if(iChannel < 0 || iChannel >= g_nCapture)
	if(iChannel < 0 )
	{
		return;
	}
	if(iIndex < 0 || iIndex >= N_DESKTOP_INFO_LINE)
	{
		return;
	}

	CGuard guard(m_MutexState);
#ifdef FUNC_GZYZ_ATM
	if(pLable == NULL && pContext == NULL)
	{		
		m_ShowCardInfo[iChannel] &= ~BITMSK(iIndex);

	}
	if(!pLable)
	{
		memset(&m_CardName[iChannel * N_DESKTOP_INFO_LINE + iIndex][0], 0, DESKTOP_INFO_MAX_LEN); 		
	}
	else
	{
		memcpy(&m_CardName[iChannel*N_DESKTOP_INFO_LINE + iIndex][0], pLable, DESKTOP_INFO_MAX_LEN); 		
		m_ShowCardInfo[iChannel] |= BITMSK(iIndex);
	}
	if(!pContext)
	{
		memset(&m_CardContent[iChannel * N_DESKTOP_INFO_LINE + iIndex][0], 0, DESKTOP_INFO_MAX_LEN);
	}
	else
	{	
		memcpy(&m_CardContent[iChannel*N_DESKTOP_INFO_LINE + iIndex][0], pContext, DESKTOP_INFO_MAX_LEN);
		m_CardContent[iChannel*N_DESKTOP_INFO_LINE + iIndex][DESKTOP_INFO_MAX_LEN - 1] = 0;
		m_ShowCardInfo[iChannel] |= BITMSK(iIndex);
	}
#else
	if(pLable == NULL)
	{
		memset(&m_CardName[iChannel * N_DESKTOP_INFO_LINE + iIndex][0], 0, DESKTOP_INFO_MAX_LEN); 
		memset(&m_CardContent[iChannel * N_DESKTOP_INFO_LINE + iIndex][0], 0, DESKTOP_INFO_MAX_LEN);
		m_ShowCardInfo[iChannel] &= ~BITMSK(iIndex);

	}
	else if((pLable != NULL) &&(pContext != NULL))
	{
		memcpy(&m_CardName[iChannel*N_DESKTOP_INFO_LINE + iIndex][0], pLable, DESKTOP_INFO_MAX_LEN); 
		memcpy(&m_CardContent[iChannel*N_DESKTOP_INFO_LINE + iIndex][0], pContext, DESKTOP_INFO_MAX_LEN);
		m_CardContent[iChannel*N_DESKTOP_INFO_LINE + iIndex][DESKTOP_INFO_MAX_LEN - 1] = 0;
		m_ShowCardInfo[iChannel] |= BITMSK(iIndex);
	}
	else if((pContext == NULL) && (pLable != NULL))
	{
		memcpy(&m_CardName[iChannel*N_DESKTOP_INFO_LINE + iIndex][0], pLable, DESKTOP_INFO_MAX_LEN); 
		m_CardContent[iChannel*N_DESKTOP_INFO_LINE + iIndex][DESKTOP_INFO_MAX_LEN - 1] = 0;
		m_ShowCardInfo[iChannel] |= BITMSK(iIndex);
	}
#endif
	int spt, sub;
//	m_pDisplay->GetSplit(&spt, &sub);

#if 0
	if((spt == SPLIT1 && sub == iChannel) || (spt == SPLITPIP && (sub & 0xffff) == iChannel))
	{
		SetCardInfo(m_ConfigATM[0].bShowCardInfo);
		if ((m_ConfigATM[0].bShowCardInfo) && (bmp_logo_middle))
		{
			m_pStaticChannelLogo[iChannel]->Show(!(m_ShowCardInfo[iChannel]));
		}
	}
#endif
}
#endif

#if 0
void CPageDesktop::SetCardInfo(VD_BOOL value)
{
#if 0
	int i;
	int spt,sub;
#ifdef FUNC_GZYZ_ATM
	VD_RECT rect;
#endif
	m_pDisplay->GetSplit(&spt, &sub);
	//trace("spt=%d,value=%d\n",spt,value);

	if (((spt != SPLIT1) && (spt != SPLITPIP)) || !value)
	{
		for(i = 0; i < N_DESKTOP_INFO_LINE; i++)
		{
			m_pStaticCardName[i]->Show(FALSE);
			m_pStaticCardContent[i]->Show(FALSE);
		}
		
		for (i = 0; i < N_SYS_CH; i++)
		{
			m_ShowCardInfo[i] = 0;
		}
		return;
	}
	 //表示单画面监视,需要显示卡号叠加信息
	if ((spt != SPLITPIP) && (sub < 0 || sub >= g_nCapture))
	{
		return;
	}

	if (spt == SPLITPIP)
	{
		sub = sub & 0xffff;
	}

	//trace("sub =%d\n",sub);
	for(i = 0; i < N_DESKTOP_INFO_LINE; i++)
	{
		m_pStaticCardName[i]->SetText(&m_CardName[sub * N_DESKTOP_INFO_LINE + i][0]);
		m_pStaticCardContent[i]->SetText(&m_CardContent[sub * N_DESKTOP_INFO_LINE + i][0]);
		CRect rect;
		m_pStaticCardContent[i]->GetRect(&rect);
		rect.right = rect.left + g_Locales.GetTextExtent((char *)&m_CardContent[sub * N_DESKTOP_INFO_LINE + i][0]) + 4;
		m_pStaticCardContent[i]->SetRect(&rect);
#ifdef FUNC_GZYZ_ATM
		VD_BOOL showed = (strlen(&m_CardContent[sub * N_DESKTOP_INFO_LINE + i][0]) != 0 );
		//m_pStaticCardName[i]->Show(showed);
		m_pStaticCardContent[i]->Show(showed);
#else
		VD_BOOL showed = (strlen(&m_CardName[sub * N_DESKTOP_INFO_LINE + i][0]) != 0 );
		m_pStaticCardName[i]->Show(showed);
		m_pStaticCardContent[i]->Show(showed);
#endif
		//trace("m_CardContent[%d * 8 + %d][0]=%s\n",sub,i,&m_CardContent[sub * 8 + i][0]);
	}
#endif
}
#endif

#if 0//csp modify
/*设置下一个分割画面，用作手动切换到下一个窗口的回调函数*/
void CPageDesktop::SetNextSplit() 
{
	#if 0
	int spt, sub;
	int iTemSplitMode, iTemSub;
	VD_BOOL bSeted = TRUE;
	m_pDisplay->GetSplit(&spt,  &sub);
	iTemSplitMode = spt;
	iTemSub = sub;
	sub++;	
	do 
	{
		switch (spt) 
		{
			case RULE_SPLIT1:
				if (!(g_iSptMsk[g_nCapture-1] & BITMSK(g_iSptDef[SPLIT1])))
				{
					spt++;
					if (spt >= g_iSptDef[SPLIT_NR])	// 分割模式已经用完，则从头开始
					{
						spt = g_iSptDef[SPLIT1];
					}
					break;
				}
				if (SetSplitSub(&spt, &sub)) 
				{
					bSeted = FALSE;	// 如果修改成功，则退出循环
				}
				break;
			case RULE_SPLIT2:	// 不支持模式
				if (!(g_iSptMsk[g_nCapture-1] & BITMSK(g_iSptDef[SPLIT2])))
				{
					spt++;
					if (spt >= g_iSptDef[SPLIT_NR])	// 分割模式已经用完，则从头开始
					{
						spt = g_iSptDef[SPLIT1];
					}
					break;
				}
				if (SetSplitSub(&spt, &sub)) 
				{
					bSeted = FALSE;	// 如果修改成功，则退出循环
				}
				break;

			case RULE_SPLIT4:
				if (!(g_iSptMsk[g_nCapture-1] & BITMSK(g_iSptDef[SPLIT4])))
				{
					spt++;
					if (spt >= g_iSptDef[SPLIT_NR])	// 分割模式已经用完，则从头开始
					{
						spt = g_iSptDef[SPLIT1];
					}
					break;
				}
				if (SetSplitSub(&spt, &sub)) 
				{
					bSeted = FALSE;	// 如果修改成功，则退出循环
				}
				break;
			case RULE_SPLIT6:
				if (!(g_iSptMsk[g_nCapture-1] & BITMSK(g_iSptDef[SPLIT6])))
				{	
					spt++;
					if (spt >= g_iSptDef[SPLIT_NR])	// 分割模式已经用完，则从头开始					
					{
						spt = g_iSptDef[SPLIT1];
					}
					break;
				}
				if (SetSplitSub(&spt, &sub))  
				{
					bSeted = FALSE;	// 如果修改成功，则退出循环
				}

				break;
			case RULE_SPLIT8:	// 支持模式
				if (!(g_iSptMsk[g_nCapture-1] & BITMSK(g_iSptDef[SPLIT8])))
				{
					spt++;
					if (spt >= g_iSptDef[SPLIT_NR])	// 分割模式已经用完，则从头开始					
					{
						spt = g_iSptDef[SPLIT1];
					}
					break;
				}
				if (SetSplitSub(&spt, &sub))  
				{
					bSeted = FALSE;	// 如果修改成功，则退出循环
				}
				break;
			case RULE_SPLIT9:
				if (!(g_iSptMsk[g_nCapture-1] & BITMSK(g_iSptDef[SPLIT9])))
				{
					spt++;
					if (spt >= g_iSptDef[SPLIT_NR])	// 分割模式已经用完，则从头开始	
					{
						spt = g_iSptDef[SPLIT1];
					}
					break;
				}
				if (SetSplitSub(&spt, &sub))  
				{
					bSeted = FALSE;	// 如果修改成功，则退出循环
				}
				break;
			case RULE_SPLIT12:
				if (!(g_iSptMsk[g_nCapture-1] & BITMSK(g_iSptDef[SPLIT12])))
				{
					spt++;
					if (spt >= g_iSptDef[SPLIT_NR])	// 分割模式已经用完，则从头开始	
					{
						spt = g_iSptDef[SPLIT1];
					}
					break;
				}
				if (SetSplitSub(&spt, &sub))  
				{
					bSeted = FALSE;	// 如果修改成功，则退出循环
				}
				break;
			case RULE_SPLIT16:
				if (!(g_iSptMsk[g_nCapture-1] & BITMSK(g_iSptDef[SPLIT16])))
				{
					spt++;
					if (spt >= g_iSptDef[SPLIT_NR])	// 分割模式已经用完，则从头开始	
					{
						spt = g_iSptDef[SPLIT1];
					}
						break;
					}
				if (SetSplitSub(&spt, &sub)) 
				{
					bSeted = FALSE;	// 如果修改成功，则退出循环
				}
				break;
			case RULE_SPLITPIP:
			default:
				spt++;
				if (spt >= g_iSptDef[SPLITPIP])	// 分割模式已经用完，则从头开始
				{
					spt = g_iSptDef[SPLIT1];
				}	
				break;
		}
	}while (bSeted && (iTemSplitMode != spt || iTemSub != sub));
	m_pDisplay->SetSplit(g_iSptRule[spt], sub);
#endif
}

VD_BOOL CPageDesktop::SetSplitSub(int *spt, int *sub)
{
#if 0
	int _spt = g_iSptRule[*spt];
	int iMaxSubMode = m_pDisplay->GetSplitIndexCount(_spt);
	if (*sub >= iMaxSubMode)
	{
		(*spt)++;
		if (*spt  >= g_iSptDef[SPLIT_NR])	// 分割模式已经用完，则从头开始		
		{
			*spt = g_iSptDef[SPLIT1];
		}
		// 从新设置分割方式
		*sub = 0;
		return FALSE;
	}
	else	
	{
		return TRUE;
	}
#endif
}

//added by wyf on 090925
void CPageDesktop::SetSilenceMenu()
{
#if 0
	CConfigGeneral	m_CCfgGeneral;
	m_CCfgGeneral.update();
	CONFIG_GENERAL &cfgGeneral = m_CCfgGeneral.getConfig();
	int menuCurCounts = pMainMenu->GetCount();
	if(FALSE == cfgGeneral.bSilence)
	{
		pMainMenu->SetItemText(menuCurCounts-1, LOADSTR("pgquick.systemsilence"), FALSE);	
	}	
#endif
}
//end added by wyf on 090925 

void CPageDesktop::SetBitRateInfo(VD_BOOL bFlag, VD_BOOL bForced /* = FALSE*/) 
{
#if 0
	static VD_BOOL first = TRUE;
	int spt, sub;
	char pszBps [8][32];
	char chanNo[8][8];

	m_pDisplay->GetSplit(&spt, &sub);
	if(!bFlag || spt != SPLIT9)
	{
		for(int i = 0; i < 2; i++)
		{
			m_pBpsChannel[i]->Show(FALSE);
			m_pBpsUnit[i]->Show(FALSE);
		}

		for(int i = 0; i < 8; i++)
		{
			m_pStaticShowBps[i]->Show(FALSE);
			m_ChannelNo[i]->Show(FALSE);
		}
		first = TRUE;
		return;
	}
	if(first || bForced)
	{
		first = FALSE;
		VD_RECT BlankRect [N_SYS_CH]; 
		VD_RECT rectBps[8]; 
		VD_RECT rect;
		int ret = m_pDevSplit->GetBlankRects(BlankRect);
		if(ret == 1)
		{
			for(int i = 0; i < 2; i++)
			{
				rect.left = BlankRect[0].left + 2 + TEXT_WIDTH * 4 * i;
				rect.right = rect.left + TEXT_WIDTH * 3 / 2;
				rect.top = BlankRect[0].top + 2;
				rect.bottom = rect.top + TEXT_HEIGHT;
				m_pBpsChannel[i]->SetRect(&rect, TRUE);  
				m_pBpsChannel[i]->SetText("CH");
				m_pBpsChannel[i]->Show(TRUE);

				rect.left = BlankRect[0].left + 2 + TEXT_WIDTH * 4 * i + TEXT_WIDTH * 3 / 2;
				rect.right = rect.left + TEXT_WIDTH * 5 / 2;
				rect.top = BlankRect[0].top + 2;
				rect.bottom = rect.top + TEXT_HEIGHT;
				m_pBpsUnit[i]->SetRect(&rect, TRUE);  
				m_pBpsUnit[i]->SetText("Kb/S");
				m_pBpsUnit[i]->Show(TRUE);		
			}


			for(int i = 0; i < MIN(8, g_nCapture); i++)
			{
				rectBps[i].left = BlankRect[0].left + 2 + TEXT_WIDTH * 4 * (i / 4);
				rectBps[i].right = rectBps[i].left + TEXT_WIDTH * 3 / 2;
				rectBps[i].top	= BlankRect[0].top + TEXT_HEIGHT + 4 + TEXT_HEIGHT * (i % 4);
				rectBps[i].bottom = rectBps[i].top + TEXT_HEIGHT;

				m_ChannelNo[i]->SetRect(&rectBps[i], TRUE);
				m_ChannelNo[i]->Show(TRUE);

				rectBps[i].left = BlankRect[0].left + 2 + TEXT_WIDTH * 4 * (i / 4) + TEXT_WIDTH * 3 / 2;
				rectBps[i].right = rectBps[i].left + TEXT_WIDTH * 5 / 2;
				rectBps[i].top	= BlankRect[0].top + TEXT_HEIGHT + 4 + TEXT_HEIGHT * (i % 4);
				rectBps[i].bottom = rectBps[i].top + TEXT_HEIGHT;

				m_pStaticShowBps[i]->SetRect(&rectBps[i], TRUE); 
				m_pStaticShowBps[i]->Show(TRUE);
			}
		}
	}

	for (int i = 0; i < MIN(8, g_nCapture); i++)
	{
		int j = sub == 0 ? i : i + 8;
		CDevCapture *pDevCapture = CDevCapture::instance(j);
		sprintf(&chanNo[i][0], "%d", i+1);
		m_ChannelNo[i]->SetText(&chanNo[i][0]);
		sprintf(&pszBps[i][0], "%d", pDevCapture->GetBitRate());
		m_pStaticShowBps[i]->SetText(&pszBps[i][0]);
	}
#endif
}

void CPageDesktop::UpdateConfig()
{
#if 0
	int i;
	CRect rect;

	m_ConfigMonitorTour.update();
	m_ConfigGUISet.update();
	m_ConfigTVAdjust.update();
	//m_ConfigATM.update();
	m_ConfigChannelTitle.update();

  if(g_nCapture > 1)
	{//更新右键菜单
		if(pSubMenu[SPLIT1])
		{
			pSubMenu[SPLIT1]->RemoveAllItems();
		}
	}
	for (i = 0; i < g_nCapture; i++)
	{
		memset(&m_pszSpt[i][0], 0, CHANNEL_NAME_MAX_LEN);
		memcpy(&m_pszSpt[i][0], &m_ConfigChannelTitle[i].strName, CHANNEL_NAME_MAX_LEN);
		m_pStaticChannelTitle[i]->SetText(&m_pszSpt[i][0]);	
		m_pStaticChannelTitle[i]->GetRect(rect);
		///CDevGraphics::instance()->DeFlicker(rect, 5); //delete by nike.xie 20090914 //去抖动接口会引起页面控件严重变形
		if(g_nCapture > 1)
		{
				if(pSubMenu[SPLIT1])	
			{
				pSubMenu[SPLIT1]->ApendItem(&m_pszSpt[i][0], (CTRLPROC)&CPageDesktop::OnSetSpt1Chan);
			}
		}
	}	
#endif
}

void CPageDesktop::SetChannelTitle(VD_BOOL bFlag)
{
#if 0
	CRect cStateRect;
	VD_RECT video_rect;
	int i;

	if(bFlag && !m_ConfigGUISet[0].bChannelTitleEn)
	{
		return;
	}

/************************************************************************
	设置各通道的目标题和时间标题：
	1、获取各通道的坐标；
	2、根据要求，显示指定的内容；
************************************************************************/
	for (i = 0; i < g_nCapture; i++)
	{
		m_pDisplay->GetSplitRect(i, &video_rect);
		if(video_rect.bottom - video_rect.top < ZERO_RECT_SIZE_ENABLE && video_rect.right - video_rect.left < ZERO_RECT_SIZE_ENABLE)
		{
			m_pStaticChannelTitle[i]->Show(FALSE);
			continue;
		}
		m_rectTitle[i].left = video_rect.left + 4;
		m_rectTitle[i].right = MIN(video_rect.right - 24, m_rectTitle[i].left + g_Locales.GetTextExtent(m_pszSpt[i]) + 4);
		m_rectTitle[i].top = video_rect.bottom - TEXT_HEIGHT - 2;
		m_rectTitle[i].bottom = video_rect.bottom+1;

		if(bFlag)
		{
			m_pStaticChannelTitle[i]->SetRect(&m_rectTitle[i], FALSE);
		}
		m_pStaticChannelTitle[i]->Show(bFlag);
		CRect rect;

		m_pStaticChannelTitle[i]->GetRect(rect);
	}
#endif
}

void CPageDesktop::SetChannelLogo(VD_BOOL bFlag, VD_BOOL bForced /* = FALSE */)
{
 #if 0
	CRect cStateRect;
	VD_RECT video_rect;
	int i, j;
	uint dwChCoverState;
	static uint s_dwOldChCoverState = 0;
	uint dwChCoverStateChanged;
	int ww;
	int hh;

	//add by nike.xie 在所有通道之外的空闲区域显示/隐藏LOGO图片
	if(bmp_logo_blank)
	{		
		int w = 0;
		int h = 0;
		int flag = 0;
		int spt = 0;
		int sub = 0;

		//获取空闲通道的坐标驱动暂时不支持
		/*
		if(!m_pDisplay->GetSplitBlankRect(0, &video_rect))
		{
			trace("GetSplitBlankRect fail\r\n");
		}*/

		//获取 第八个通道的坐标
		m_pDisplay->GetSplitRect(g_nCapture-1, &video_rect);	
		w = video_rect.right-video_rect.left;
		h = video_rect.bottom - video_rect.top;
		if( (w>0) && (h>0) )
		{
			flag = 1;
		}

		//空白区域的坐标
		video_rect.left = video_rect.right+2;
		video_rect.right = video_rect.left + w;
		video_rect.top += 2;
		video_rect.bottom = video_rect.top + h;
		
		//图片的坐标,上下左右的坐标必须符合图片的实际大小
		//modefied by nike.xie 2009-6-26
		video_rect.left = (video_rect.left +video_rect.right - bmp_logo_blank->width)/2;	
		video_rect.right = video_rect.left +  bmp_logo_blank->width;
		video_rect.top = (video_rect.top +video_rect.bottom- bmp_logo_blank->height)/2;
		video_rect.bottom = video_rect.top + bmp_logo_blank->height;
		
		if(bFlag && flag)
		{
			//显示LOGO图片		
			m_pDisplay->GetSplit(&spt, &sub);	
			if(SPLIT9 == spt)
			{
				m_pStaticBlankLogo[0]->SetRect(&video_rect);			
				m_pStaticBlankLogo[0]->Show(TRUE);				
			}
		}
		else if(flag)
		{
			//隐藏LOGO图片
			m_pStaticBlankLogo[0]->SetRect(&video_rect);
			m_pStaticBlankLogo[0]->Show(FALSE);
		}
		
	}
	//end
	
	if(!bmp_logo_middle)
	{
		return;
	}

	//在视频丢失或锁定的区域显示LOGO
	dwChCoverState = m_pDisplay->GetChannelLockState() | g_Alarm.GetLossDetectState();
	dwChCoverStateChanged = (dwChCoverState ^ s_dwOldChCoverState);

	if(!bForced) //非强制刷新时, 保存旧的状态, 以便比较动态刷新
	{
		s_dwOldChCoverState = dwChCoverState;
	}

	for (i = 0; i < g_nCapture; i++)
	{
		m_pDisplay->GetSplitRect(i, &video_rect);
		//changed by yanjun 20061117
		//if(video_rect.bottom <ZERO_RECT_SIZE_ENABLE && video_rect.top <ZERO_RECT_SIZE_ENABLE && video_rect.left <ZERO_RECT_SIZE_ENABLE && video_rect.right<ZERO_RECT_SIZE_ENABLE)
		if(video_rect.bottom - video_rect.top < ZERO_RECT_SIZE_ENABLE && video_rect.right - video_rect.left < ZERO_RECT_SIZE_ENABLE)
		{
			continue;
		}

		if(dwChCoverStateChanged & BITMSK(i) || bForced)
		{
			if((dwChCoverState & BITMSK(i)) && bFlag)
			{
			#ifndef MON_CHAN_TITLE
				m_pStaticChannelTitle[i]->SetColor(STC_BK, m_bkColor, !bForced);
			#endif
				for (j = 0; j < ALLCHSTATES; j++)
				{
					m_pStaticChannelState[i][j]->SetColor(STC_BK, m_bkColor, !bForced);
				}
			}
			else
			{
			#ifndef MON_CHAN_TITLE
				m_pStaticChannelTitle[i]->SetColor(STC_BK, m_keyColor, !bForced);
			#endif
				for (j = 0; j < ALLCHSTATES; j++)
				{
					m_pStaticChannelState[i][j]->SetColor(STC_BK, m_keyColor, !bForced);
				}
			}
			m_DC.Lock();
			if((dwChCoverState & BITMSK(i)) && bFlag)
			{
				m_DC.SetBrush(m_bkColor);
			}
			else
			{
				m_DC.SetBrush(m_keyColor);
			}
			CHANNEL_RECT_ADJUST(video_rect);
			m_DC.SetRgnStyle(RS_FLAT);
			m_DC.Rectangle(CRect(video_rect.left, video_rect.top , video_rect.right, video_rect.bottom ));
			m_DC.UnLock();

			ww = video_rect.right + video_rect.left;			
			hh = video_rect.bottom + video_rect.top - TEXT_HEIGHT;

			if((dwChCoverState & BITMSK(i)) && bFlag)
			{
				m_pStaticChannelLogo[i]->SetRect(CRect((ww - m_iLogoWidth) / 2, (hh - m_iLogoHeight) / 2,
					(ww + m_iLogoWidth) / 2, (hh + m_iLogoHeight) / 2), FALSE);
			}
			
			//当前无图像或是没有叠加卡号信息，显示LOGO，否则隐藏
			m_pStaticChannelLogo[i]->Show((dwChCoverState & BITMSK(i)) && bFlag && !(m_ShowCardInfo[i]));
			
		}
	}	
#endif
}

void CPageDesktop::SetChannelState(VD_BOOL recordStatus, VD_BOOL alarmStatus, VD_BOOL bForced /* = FALSE */)
{
#if 0
	int i;
	uint dwChRecState, dwChLckState, dwChMtdState, dwChVlsState, dwChBldState, dwChIOState;
	static uint s_dwOldChRecState = 0, s_dwOldChLckState = 0, s_dwOldChMtdState = 0;
	static uint  s_dwOldChVlsState = 0, s_dwOldChBldState = 0, s_dwOldChIOState = 0;
	VD_RECT rect;
	VD_RECT video_rect;
	int rects_dx;//状态图标的间距
	uint dwChRecStateChanged;
	uint dwChLckStateChanged;
	uint dwChMtdStateChanged;
	uint dwChVlsStateChanged;
	uint dwChBldStateChanged;
	uint dwChIOStateChanged;

	dwChRecState = g_Record.GetState();
	dwChLckState = m_pDisplay->GetChannelLockState();
	dwChMtdState = g_Alarm.GetMotionDetectState();
	dwChVlsState = g_Alarm.GetLossDetectState();
	dwChIOState = g_Alarm.GetAlarmState();
	dwChBldState = g_Alarm.GetBlindDetectState();
	dwChIOState = g_Alarm.GetAlarmState();

	dwChRecStateChanged = (dwChRecState ^ s_dwOldChRecState);
	dwChLckStateChanged = (dwChLckState ^ s_dwOldChLckState);
	dwChMtdStateChanged = (dwChMtdState ^ s_dwOldChMtdState);
	dwChVlsStateChanged = (dwChVlsState ^ s_dwOldChVlsState);
	dwChBldStateChanged = (dwChBldState ^ s_dwOldChBldState);
	dwChIOStateChanged = (dwChIOState ^ s_dwOldChIOState);

	if(!bForced) //非强制刷新时, 保存旧的状态, 以便比较动态刷新
	{
		s_dwOldChRecState = dwChRecState;
		s_dwOldChLckState = dwChLckState;
		s_dwOldChMtdState = dwChMtdState;
		s_dwOldChVlsState = dwChVlsState;
		s_dwOldChBldState = dwChBldState;
		s_dwOldChIOState = dwChIOState;
	}

#if defined(DEF_2U_FRONTBOARD)
	//add by nike.xie  20091118 支持每个通道的录像灯点亮或关闭	
	static uint s_dwOldChRecLedState = 0;
	uint dwChRecLedStateChanged = (dwChRecState ^ s_dwOldChRecLedState);
	s_dwOldChRecLedState = dwChRecState;
	
	for(i = 0; i < g_nCapture; i++)
	{
		if(dwChRecLedStateChanged & BITMSK(i))
		{
			if ((dwChRecState & BITMSK(i)))
			{
				CDevFrontboard::instance()->LigtenLed(FB_LED_CHANNEL, TRUE, i);
			}
			else
			{
				CDevFrontboard::instance()->LigtenLed(FB_LED_CHANNEL, FALSE, i);
			}
		}
	}
	//end nike.xie
#endif

	for (i = 0; i < g_nCapture; i++)
	{
		m_pDisplay->GetSplitRect(i, &video_rect);
		CHANNEL_RECT_ADJUST(video_rect);
		//通道视频不可见
		//if(video_rect.bottom <ZERO_RECT_SIZE_ENABLE && video_rect.top <ZERO_RECT_SIZE_ENABLE && video_rect.left <ZERO_RECT_SIZE_ENABLE && video_rect.right<ZERO_RECT_SIZE_ENABLE)
		if(video_rect.bottom - video_rect.top < ZERO_RECT_SIZE_ENABLE && video_rect.right - video_rect.left < ZERO_RECT_SIZE_ENABLE)
		{
			continue;
		}
		rect.left = m_rectTitle[i].right;
		rect.top = video_rect.bottom - TEXT_HEIGHT - 2;

		rect.right = video_rect.right;
		rect.bottom = video_rect.bottom;
	
		/*-----------------------------------------------------------------------
		处理通道的状态图
		-----------------------------------------------------------------------*/
		if(rect.right - rect.left < CHSTATE_WIDTH * 5)
		{
			rects_dx = (rect.right - rect.left - CHSTATE_WIDTH) / 4;
		}
		else
		{
			rects_dx = CHSTATE_WIDTH;
		}
		rect.right = rect.left + CHSTATE_WIDTH;
		if(dwChRecStateChanged & BITMSK(i) || bForced)
		{
			if ((dwChRecState & BITMSK(i)) && recordStatus)
			{
				if (m_ConfigGUISet[0].bRecordStatus)
				{
					m_pStaticChannelState[i][RECORD]->SetRect(&rect,TRUE);
					m_pStaticChannelState[i][RECORD]->Show(FALSE,FALSE);
					m_pStaticChannelState[i][RECORD]->Show(TRUE);
				}
			}
			else
			{
				m_pStaticChannelState[i][RECORD]->Show(FALSE);
			}

			if (!bForced)
			{
				if ((dwChRecState & BITMSK(i)))
				{
					CDevFrontboard::instance()->LigtenLed(FB_LED_RECORD, TRUE);
				}
				else
				{
					CDevFrontboard::instance()->LigtenLed(FB_LED_RECORD, FALSE);
				}
			}
		}

		rect.left += rects_dx;
		rect.right = rect.left + CHSTATE_WIDTH;
		if(dwChMtdStateChanged & BITMSK(i) || bForced)
		{
			if ((dwChMtdState & BITMSK(i)) && alarmStatus)
			{
				if (m_ConfigGUISet[0].bAlarmStatus)
				{
					m_pStaticChannelState[i][MTD]->SetRect(&rect,TRUE);
					m_pStaticChannelState[i][MTD]->Show(FALSE,FALSE);
					m_pStaticChannelState[i][MTD]->Show(TRUE);
				}
			}
			else
			{
				m_pStaticChannelState[i][MTD]->Show(FALSE);
			}

			if (!bForced)
			{
				if ((dwChMtdState & BITMSK(i)))
				{
					CDevFrontboard::instance()->LigtenLed(FB_LED_ALARM, TRUE);
				}
				else
				{
					CDevFrontboard::instance()->LigtenLed(FB_LED_ALARM, FALSE);
				}
			}
		}
		rect.left += rects_dx;
		rect.right = rect.left + CHSTATE_WIDTH;
		if(dwChVlsStateChanged & BITMSK(i) || bForced)
		{
			if ((dwChVlsState & BITMSK(i)) && alarmStatus)
			{
				if (m_ConfigGUISet[0].bAlarmStatus)
				{
					m_pStaticChannelState[i][VLS]->SetRect(&rect,TRUE);
					m_pStaticChannelState[i][VLS]->Show(FALSE,FALSE);
					m_pStaticChannelState[i][VLS]->Show(TRUE);
				}
			}
			else
			{
				m_pStaticChannelState[i][VLS]->Show(FALSE);			
			}

			if (!bForced)
			{
				if ((dwChVlsState & BITMSK(i)))
				{
					CDevFrontboard::instance()->LigtenLed(FB_LED_ALARM, TRUE);
				}
				else
				{
					CDevFrontboard::instance()->LigtenLed(FB_LED_ALARM, FALSE);
				}
			}
		}
		rect.left += rects_dx;
		rect.right = rect.left + CHSTATE_WIDTH;
		if(dwChBldStateChanged & BITMSK(i) || bForced)
		{
			if ((dwChBldState & BITMSK(i)) && alarmStatus)
			{
				if (m_ConfigGUISet[0].bAlarmStatus)
				{
					m_pStaticChannelState[i][BLD]->SetRect(&rect,TRUE);
					m_pStaticChannelState[i][BLD]->Show(FALSE,FALSE);
					m_pStaticChannelState[i][BLD]->Show(TRUE);
				}				
			}
			else
			{
				m_pStaticChannelState[i][BLD]->Show(FALSE);
			}

			if (!bForced)
			{
				if ((dwChBldState & BITMSK(i)))
				{
					CDevFrontboard::instance()->LigtenLed(FB_LED_ALARM, TRUE);
				}
				else
				{
					CDevFrontboard::instance()->LigtenLed(FB_LED_ALARM, FALSE);
				}
			}
		}
		rect.left += rects_dx;
		rect.right = rect.left + CHSTATE_WIDTH;
		if(dwChLckStateChanged & BITMSK(i) || bForced)
		{
			if ((dwChLckState & BITMSK(i)) && alarmStatus)
			{
				m_pStaticChannelState[i][LOCK]->SetRect(&rect,TRUE);
				m_pStaticChannelState[i][LOCK]->Show(FALSE,FALSE);
				m_pStaticChannelState[i][LOCK]->Show(TRUE);
			}
			else
			{
				m_pStaticChannelState[i][LOCK]->Show(FALSE);
			}
		}

		if(dwChIOStateChanged & BITMSK(i) && !bForced)
		{
			if ((dwChIOState & BITMSK(i)))
			{
				CDevFrontboard::instance()->LigtenLed(FB_LED_ALARM, TRUE);
			}
			else
			{
				CDevFrontboard::instance()->LigtenLed(FB_LED_ALARM, FALSE);
			}
		}
	}
#endif
}

void CPageDesktop::SetTimeTitle(VD_BOOL bFlag, VD_BOOL bForced /* = FALSE */)
{
#if 0
	//trace("SetTimeTitle\n");
	SYSTEM_TIME st;

	char szTime[32] = {0};
	char szTimeTitle[64] = {0};
	char szWeek[32] = {0};

	memset(&st,0,sizeof(SYSTEM_TIME));
	memset(szTime,0,sizeof(szTime));
	memset(szTimeTitle,0,sizeof(szTimeTitle));
	VD_RECT rect_video, rect;
	VD_BOOL time_rect_changed = FALSE; 
	if(bFlag && !m_ConfigGUISet[0].bTimeTitleEn)
	{
		return;
	}

	if(bFlag || bForced)
	{
		SystemGetCurrentTime(&st);
		st.isdst = 0;//clear milli second
		if(m_iTimeFmt != CConfigLocation::getLatest().iTimeFormat || memcmp(&st, &m_OldSt, sizeof(SYSTEM_TIME)) || bForced)
		{
			FormatTimeString(&st, szTime, FT_FULL_AMPM);
			SYS_getsysweek(&st, szWeek, 0);
			sprintf(szTimeTitle," %s %s ", szTime, szWeek);
			m_pStaticTimeTitle->SetText(szTimeTitle);
			m_OldSt = st;
		}
		if (m_iTimeFmt != CConfigLocation::getLatest().iTimeFormat || bForced)
		{
			m_pDisplay->GetSplitFullRect(&rect_video);
			int ww = g_Locales.GetTextExtent(szTimeTitle);
			rect.right		= rect_video.right - TEXT_WIDTH * 2;
			rect.left		= rect.right - ww;
			rect.top		= rect_video.top + TEXT_HEIGHT;
			rect.bottom		= rect_video.top + TEXT_HEIGHT * 2;
			m_pStaticTimeTitle->SetRect(&rect);	
			m_iTimeFmt = CConfigLocation::getLatest().iTimeFormat;
			time_rect_changed = TRUE;
		}
	}	

	m_pStaticTimeTitle->Show(bFlag);

#ifdef APP_TOUR_TURN
	if(bFlag != m_bTitleFlag || bForced || time_rect_changed)
	{
		m_pDisplay->GetSplitFullRect(&rect_video);

		if(m_ConfigGUISet[0].bTimeTitleEn)
		{
			m_pStaticTimeTitle->GetRect(&rect);
			rect.right = rect.left;
			rect.left = rect.left - TEXT_WIDTH;
		}
		else
		{
			rect.right = rect_video.right - TEXT_WIDTH * 2;
			rect.left = rect.right - TEXT_WIDTH;
			rect.top = rect_video.top + TEXT_HEIGHT;
			rect.bottom = rect_video.top + TEXT_HEIGHT * 2;
		}

		// 轮巡按钮位置没有变化就不用重画按钮了
		// 因为在监视器上看到轮训按钮一直在那边闪,所以这样处理一下!
		VD_BOOL redrawTour = memcmp(&rect, &m_rectTour, sizeof(VD_RECT));

		m_pButtonTour->SetRect(&rect, redrawTour ? TRUE : FALSE);
		if (redrawTour)
		{
			m_rectTour = rect;
		}

		if(time_rect_changed && CConfigLocation::getLatest().iTimeFormat == TF_12)
		{
			m_pStaticTimeTitle->Draw();
		}

		m_bTitleFlag = bFlag;
	}
#endif
#endif
}

/*	$FXN :	
==	======================================================================
==	$DSC :	该函数用于定时刷新桌面的各类标题
==	$ARG :	
==		 :	
==	$RET :	
==	======================================================================
*/
void CPageDesktop::OnTimer(uint arg)
{
#if 0
	CGuard guard(m_MutexState); //进入其他页面弹出密码框会引起定时器锁调
	
	SetChannelLogo(TRUE);
//	SetTimeTitle(m_ConfigGUISet[0].bTimeTitleEn); //定时更新系统时间
//	SetChannelState(m_ConfigGUISet[0].bRecordStatus, m_ConfigGUISet[0].bAlarmStatus);	//定时更新通道状态
	
	if(m_count++ % 15 == 0)
	{
//		SetBitRateInfo(m_ConfigGUISet[0].bBitRateEn);  //当9画面分割时在空白画面显示码流
	}
#endif
}

void CPageDesktop::OnConfigChanged()
{
#if 0
	CGuard guard(m_MutexState);

	//擦除通道名称和标志先
	SetChannelTitle(FALSE);
	SetChannelState(FALSE, FALSE, TRUE);

	UpdateConfig();
	SetChannelTitle(m_ConfigGUISet[0].bChannelTitleEn);
	SetChannelState(m_ConfigGUISet[0].bRecordStatus, m_ConfigGUISet[0].bAlarmStatus, TRUE);
	SetTimeTitle(m_ConfigGUISet[0].bTimeTitleEn);
	SetBitRateInfo(m_ConfigGUISet[0].bBitRateEn);
	//SetCardInfo(m_ConfigATM[0].bShowCardInfo);
#endif
}

//必须成对调用, 才能保证互斥量不会出错
void CPageDesktop::OnSplitChange(VD_BOOL post)
{
#if 0
	if(!post)
	{
		m_MutexState.Enter();
	}

	if(post == TRUE)
	{
		SetBitRateInfo(m_ConfigGUISet[0].bBitRateEn, post);
		m_count = 0;
	}
	if(post==TRUE)
	{
		SetChannelLogo(post, TRUE);
		SetChannelTitle(post);
		SetChannelState(post, post, TRUE);
	}
	else
	{
		SetChannelTitle(post);
		SetChannelState(post, post, TRUE);
		SetChannelLogo(post, TRUE);
	    }
	//SetChannelLogo(post, TRUE);
	//SetChannelTitle(post);
	//SetChannelState(post, TRUE);
	SetTimeTitle(post, TRUE);

	if(post)
	{
		m_MutexState.Leave();
	}
#endif
}

//轮巡状态显示更新
void CPageDesktop::OnTourStateChanged(VD_BOOL started, VD_BOOL locked)
{
#if 0
	CGuard guard(m_MutexState);

#ifdef APP_TOUR_TURN
	if(started)
	{
		if(locked)
		{
			m_pButtonTour->SetBitmap(bmp_tour_disable, bmp_tour_disable, bmp_tour_disable, bmp_tour_disable);
		}
		else
		{
			m_pButtonTour->SetBitmap(bmp_tour_enable, bmp_tour_enable, bmp_tour_enable, bmp_tour_enable);
		}
		m_pButtonTour->Show(TRUE);
	}
	else
	{
		m_pButtonTour->Show(FALSE);
	}
#endif
#endif
}
#endif

#if 0
//轮巡使能与锁定的切换
void CPageDesktop::OnTourSwitch()
{
#ifdef APP_TOUR_TURN
	if(m_pDisplay->IsTourLocked())
	{
		m_pDisplay->UnlockTour();
	}
	else
	{
		m_pDisplay->LockTour();
	}
#endif
}
#endif

#if 0
/*配置结构更新时的回调函数*/
void CPageDesktop::OnConfigMonitorTour(CConfigMonitorTour& config, int& ret)
{
 	m_ConfigMonitorTour.update();
 	OnTourStateChanged(m_pDisplay->IsTourStarted(), m_pDisplay->IsTourLocked());
}

void CPageDesktop::OnConfigGUISet(CConfigGUISet& config, int& ret)
{	
	m_ConfigGUISet.update();
	g_GUI.CountDown();
}

void CPageDesktop::OnConfigTVAdjust(CConfigTVAdjust& config, int& ret)
{
	m_ConfigTVAdjust.update();
 	SetBitRateInfo(m_ConfigGUISet[0].bBitRateEn, TRUE);
}

void CPageDesktop::OnConfigChannelTitle(CConfigChannelTitle& config, int& ret)
{
	m_ConfigChannelTitle.update();	
}
#endif

void CPageDesktop::Draw()
{
	if(!DrawCheck())
	{
		return;
	}
	
	CRgn remain_rgn;
	VD_RECT video_rect;
	
	int i;
	//uint dwChCoverState = m_pDisplay->GetChannelLockState() | g_Alarm.GetLossDetectState();
	uint dwChCoverState = 0;
	
	remain_rgn.SetRectRgn(&m_Rect);//csp modify
	
	//可能有视频的画面
	m_DC.Lock();
	
#if 0
	for (i = 0; i < g_nCapture; i++)
	{
		m_pDisplay->GetSplitRect(i, &video_rect);
		if(video_rect.bottom == 0 && video_rect.top == 0 && video_rect.left == 0 && video_rect.right == 0)
		{
			continue;
		}
		if(bmp_logo_middle && (dwChCoverState & BITMSK(i)))
		{
			m_DC.SetBrush(m_bkColor);
		}
		else
		{
			m_DC.SetBrush(m_keyColor);
		}
		CHANNEL_RECT_ADJUST(video_rect);
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(video_rect.left, video_rect.top, video_rect.right, video_rect.bottom));
		remain_rgn.SubtractRect(CRect(video_rect.left, video_rect.top, video_rect.right, video_rect.bottom));
	}
#endif
	
#if 0
	//空闲画面
	for(i = 0; i < g_nCapture; i++)
	{
		if(!m_pDisplay->GetSplitBlankRect(i, &video_rect))
		{
			break;
		}
		if(bmp_logo_middle)
		{
			m_DC.SetBrush(m_bkColor);
		}
		else
		{
			m_DC.SetBrush(m_keyColor);
		}
		CHANNEL_RECT_ADJUST(video_rect);
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(video_rect.left, video_rect.top, video_rect.right, video_rect.bottom));
		remain_rgn.SubtractRect(CRect(video_rect.left, video_rect.top, video_rect.right, video_rect.bottom));
	}
#endif
	
	//其他区域
	if(!remain_rgn.IsRgnEmpty())
	{
		VD_PCRECT pcRect = remain_rgn.GetFirstRect();
		
		m_DC.SetBrush(m_keyColor);
		m_DC.SetRgnStyle(RS_FLAT);
		
		while(pcRect)
		{
			m_DC.Rectangle(pcRect);
			pcRect = remain_rgn.GetNextRect();
		}
	}
	
	m_DC.UnLock();
	
	CPage::Draw();
}

#if 0
void CPageDesktop::OnShutDown(uint param)
{
	//pPageCountDown->SetProgress(100 * (m_ShutCountDown + 1000) / (SHUT_TIME + 1000));
	
	if(m_ShutCountDown >= SHUT_TIME)
	{
		VD_SendMessage(XM_SHUTDOWNTIMEUP);
	}
	else
	{
		m_ShutCountDown += 1000;
	}
}
#endif

#if 0//csp modify
void CPageDesktop::OnSwitchVideoOutput(uint param)
{
#if 0
	CDevVideo * pDevVideo = CDevVideo::instance(m_screen);

	if(pDevVideo->SwitchOutput())
	{
		//视通的调用此函数以重新得到分辨率和重绘窗口
#if defined(DVR_GB)
		g_GUI.UpdateGraphics();
		m_pDisplay->UpdateSplitRect();
#endif
	}

	if(pDevVideo->GetCurrentOuputMode() == VIDEO_OUTPUT_TV)
	{
			/*初始化TV区域*/
		pDevVideo->SetTVMargin(
			m_ConfigTVAdjust[0].rctMargin.left,
			m_ConfigTVAdjust[0].rctMargin.top,
			m_ConfigTVAdjust[0].rctMargin.right,
			m_ConfigTVAdjust[0].rctMargin.bottom
			);

		/*初始化TV颜色*/
		pDevVideo->SetTVColor(
			m_ConfigTVAdjust[0].iBrightness,
			m_ConfigTVAdjust[0].iContrast
			);

		/*初始化TV去抖动*/
		pDevVideo->SetTVAntiDither(
			m_ConfigTVAdjust[0].iAntiDither
			);
	}
	/*end*/
#endif
}

//add by yanjun for config data updata
void CPageDesktop::UpdataDisplayConfig()
{
#if 0
	//m_pDisplay->LoadDisplayConfig(&m_sConfigDisplay);

	m_ConfigMonitorTour.update();
	m_ConfigGUISet.update();
	m_ConfigTVAdjust.update();
	//m_ConfigATM.update();
	m_ConfigChannelTitle.update();
#endif
}
#endif

#if 0
////////////////////////////////////////////////////////////////////////////////////////////
////////////CPageCountDown
CPageCountDown::CPageCountDown(VD_PCRECT pRect, VD_PCSTR psz, VD_BITMAP* icon, CPage * pParent):CPageFrame(pRect, psz, icon, pParent)
{
	SetRect(CRect(0, 0, 380 + CLIENT_X * 2, 172 + CLIENT_Y * 2), FALSE);

	//new items
	VD_RECT rect;
	GetClientRect(&rect);
	
	CRect m_rect;
	m_rect.SetRect(CLIENT_X, CLIENT_Y, rect.right - CLIENT_X, CLIENT_Y + 50);
	pProgress = CreateProgressCtrl(m_rect, this);

	CItem* pItem = GetCloseButton();
	if(pItem)
	{
		pItem->Enable(FALSE);
	}
}

CPageCountDown::~CPageCountDown()
{
	
}

VD_BOOL CPageCountDown::UpdateData(UDM mode)
{
	if(mode == UDM_OK || mode == UDM_CANCEL){
		return FALSE;
	}
	if(mode == UDM_OPEN)
	{
		pProgress->SetPos(0);
	}
	return TRUE;
}

void CPageCountDown::SetProgress(uint pos)
{
	pProgress->SetPos(pos);
}

/*!
	\b Description		:	消息处理\n
	调用这个类的目前只有关机状态和取消备份。为了把取消备份的键消息还给备份页面，处理了以下备份取消信息
	\b Argument			:	uint msg, PARAM wpa, PARAM lpa
	\param				:
	\param				:
	\return				

	\b Revisions		:	
*/
VD_BOOL CPageCountDown::MsgProc(uint msg, uint wpa, uint lpa)
{
	if (wpa == KEY_BACKUP)
	{
		CItem *pItemSpecial = FindPage(m_screen, "&bkup_data.startrecord");
		if (pItemSpecial)
		{
			return pItemSpecial->MsgProc(msg, wpa, lpa);	
		}
		else
		{
			//tracepoint();
		}
	}

	return CPage::MsgProc(msg, wpa, lpa);
}
#endif

#if 0//csp modify
#ifdef FUNC_QUICK_PLAY
void CPageDesktop::OnQuickPlayDefault()
{
    int ch;
	int spt, sub;
	
	ch = m_pDisplay->GetChannelAt(mouse_spt_x, mouse_spt_y);
	if(ch < 0)
	{
		return;
	}
	m_pDisplay->GetSplit(&spt, &sub);
	if( spt != SPLIT1)
	{
		m_pDisplay->SetSplit(SPLIT1, ch);
	}
	
	OnQuickPlay(ch, m_QuickPlaySelList[ch][0], m_QuickPlaySelList[ch][1]);
}

void CPageDesktop::OnQuickPlay(int ch, int RecType, int BackTime)
{
	m_QuickPlaySelList[ch][0] = RecType;
	m_QuickPlaySelList[ch][1] = BackTime;

	m_pPageMain->m_pPagePlay->QuickPlay(ch, RecType, BackTime + 1);
}

void CPageDesktop::GetQuickPlayMenuCurSel()
{
	int		RecType, BackTime;
	int		ch;
	int		spt, sub;
	
	ch = m_pDisplay->GetChannelAt(mouse_spt_x, mouse_spt_y);
	if(ch < 0)
	{
		return;
	}
	m_pDisplay->GetSplit(&spt, &sub);
	if( spt != SPLIT1)
	{
		m_pDisplay->SetSplit(SPLIT1, ch);
	}

	RecType	 	= pQuickMenu->GetCurSel();
	BackTime	= pMenuType->GetCurSel();
	OnQuickPlay(ch, RecType, BackTime);
}
#endif

void CPageDesktop::OnShutDown()
{
	
}

void CPageDesktop::OnLocksys()
{
	
}
#endif

//csp modify
//void CPageDesktop::InitSplitLine( int maxlines /*= 16*/, VD_COLORREF color /*= VD_RGB(255,255,255)*/ )
void CPageDesktop::InitSplitLine( int maxlines /*= 16*/, VD_COLORREF color /*= VD_RGB(232,232,232)*/ )
{
	for (int i = 0; i<maxlines; i++)
	{
		CStatic* pSplit = CreateStatic(CRect(0,0,0,0), this, "");
		pSplit->SetBkColor(color);
		pSplit->Show(FALSE);
		
		m_vSplitLine.push_back(pSplit);
	}
	
#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify 20130101
#if 1//csp modify 20130101
	//color = VD_RGB(232,232,232);
	std::vector<CStatic*> m_vFrameSplitLine;
	CStatic* pSplit = CreateStatic(CRect(0,0,2,nScreanHeight), this, "");
	pSplit->SetBkColor(color);
	pSplit->Show(TRUE);
	m_vFrameSplitLine.push_back(pSplit);
	pSplit = CreateStatic(CRect(0,0,nScreanWidth,2), this, "");
	pSplit->SetBkColor(color);
	pSplit->Show(TRUE);
	m_vFrameSplitLine.push_back(pSplit);
	pSplit = CreateStatic(CRect(0,nScreanHeight-2,nScreanWidth,nScreanHeight), this, "");
	pSplit->SetBkColor(color);
	pSplit->Show(TRUE);
	m_vFrameSplitLine.push_back(pSplit);
	pSplit = CreateStatic(CRect(nScreanWidth-2,0,nScreanWidth,nScreanHeight), this, "");
	pSplit->SetBkColor(color);
	pSplit->Show(TRUE);
	m_vFrameSplitLine.push_back(pSplit);
#else
	std::vector<CStatic*> m_vFrameSplitLine;
	CStatic* pSplit = CreateStatic(CRect(0,0,1,nScreanHeight), this, "");
	pSplit->SetBkColor(color);
	pSplit->Show(TRUE);
	m_vFrameSplitLine.push_back(pSplit);
	pSplit = CreateStatic(CRect(0,0,nScreanWidth,1), this, "");
	pSplit->SetBkColor(color);
	pSplit->Show(TRUE);
	m_vFrameSplitLine.push_back(pSplit);
	pSplit = CreateStatic(CRect(0,nScreanHeight-1,nScreanWidth,nScreanHeight), this, "");
	pSplit->SetBkColor(color);
	pSplit->Show(TRUE);
	m_vFrameSplitLine.push_back(pSplit);
	pSplit = CreateStatic(CRect(nScreanWidth-1,0,nScreanWidth,nScreanHeight), this, "");
	pSplit->SetBkColor(color);
	pSplit->Show(TRUE);
	m_vFrameSplitLine.push_back(pSplit);
#endif
#endif
	
#if 1//csp modify
	pStaticVerifySNResult = CreateStatic(CRect(nScreanWidth/2-200,nScreanHeight/2-14,nScreanWidth/2+200,nScreanHeight/2+14), this, "&CfgPtn.SNVerifyFailed");
	pStaticVerifySNResult->SetTextAlign(VD_TA_CENTER);
	
	//用贴图方法去除时间闪的问题
	//pStaticVerifySNResult->SetBkColor(VD_RGB(16,16,16));
	//pStaticVerifySNResult->SetBitmap(pbmpIcon[SHORTCUT_MAX][0]);
	
	pStaticVerifySNResult->SetTextColor(VD_RGB(255,16,16));
	
	pStaticVerifySNResult->Show(FALSE);
#endif
}

void CPageDesktop::SetSplitLineRect( int index, int x, int y, int length, EMSPLITLINETYPE type)
{
	//printf("SetSplitRectLine %d \n", m_Rect.Width());
	if(index <0 || index >= m_vSplitLine.size() || length<0)
	{
		return;
	}
	
	int cx, cy;
	switch(type)
	{
	case EM_SPLIT_H:
		cx = x+MIN(length, m_Rect.Width());
		cy = y+2;
		break;
	case EM_SPLIT_V:
		cy = y+MIN(length, m_Rect.Height());
		cx = x+2;
		break;
	default:
		return;
	}
	
	m_vSplitLine[index]->SetRect(CRect(x,y,cx,cy));
}

void CPageDesktop::ShowSplitLine( int index, BOOL bShow )
{
	//printf("ShowSplitLine m_vSplitLine.size(): %d\n", m_vSplitLine.size());
	if (index <0 || index >= m_vSplitLine.size())
	{
		return;
	}

	m_vSplitLine[index]->Show(bShow);
}
//   **********************************
void CPageDesktop::InitElecZoomSplitLine( int maxlines /*= 16*/, VD_COLORREF color /*= VD_RGB(255,255,255)*/ )
{
	for (int i = 0; i<maxlines; i++)
	{
		CStatic* pSplit = CreateStatic(CRect(0,0,0,0), this, "");
		pSplit->SetBkColor(color);
		pSplit->Show(FALSE);

		m_vElecZoomSplitLine.push_back(pSplit);
	}
}


void CPageDesktop::SetElecZoomSplitLineRect( int index, int x, int y, int length, EMSPLITLINETYPE type)
{
	//printf("SetSplitRectLine %d \n", m_Rect.Width());
	if (index <0 || index >= m_vElecZoomSplitLine.size() || length<0)
	{
		return;
	}

	int cx, cy;
	switch (type)
	{
	case EM_SPLIT_H:
		cx = x+MIN(length, m_Rect.Width());
		cy = y+2;
		break;
	case EM_SPLIT_V:
		cy = y+MIN(length, m_Rect.Height());
		cx = x+2;
		break;
	default:
		return;
	}
	
	m_vElecZoomSplitLine[index]->SetRect(CRect(x,y,cx,cy));
}

void CPageDesktop::ShowElecZoomSplitLine( int index, BOOL bShow )
{
	if (index <0 || index >= m_vElecZoomAreaSplitLine.size())
	{
		return;
	}

	m_vElecZoomSplitLine[index]->Show(bShow);
}
// ***********************************************

void CPageDesktop::InitElecZoomAreaSplitLine( int maxlines /*= 16*/, VD_COLORREF color /*= VD_RGB(255,255,255)*/ )
{
	for (int i = 0; i<maxlines; i++)
	{
		CStatic* pSplit = CreateStatic(CRect(0,0,0,0), this, "");
		pSplit->SetBkColor(color);
		pSplit->Show(FALSE);

		m_vElecZoomAreaSplitLine.push_back(pSplit);
	}
}


void CPageDesktop::SetElecZoomAreaSplitLineRect( int index, int x, int y, int length, EMSPLITLINETYPE type)
{
	//printf("SetSplitRectLine %d \n", m_Rect.Width());
	if (index <0 || index >= m_vElecZoomAreaSplitLine.size() || length<0)
	{
		return;
	}

	int cx, cy;
	switch (type)
	{
	case EM_SPLIT_H:
		cx = x+MIN(length, m_Rect.Width());
		cy = y+2;
		break;
	case EM_SPLIT_V:
		cy = y+MIN(length, m_Rect.Height());
		cx = x+2;
		break;
	default:
		return;
	}
	
	m_vElecZoomAreaSplitLine[index]->SetRect(CRect(x,y,cx,cy));
}

void CPageDesktop::ShowElecZoomAreaSplitLine( int index, BOOL bShow )
{
	if (index <0 || index >= m_vElecZoomAreaSplitLine.size())
	{
		return;
	}

	m_vElecZoomAreaSplitLine[index]->Show(bShow);
}

void CPageDesktop::InitChnName( int maxName /*= 4*/ )
{
	//VD_BITMAP* pBmp1 = VD_LoadBitmap(CHORTCUT_BITMAP_DIR"bk_ChnName.bmp");
	int w = 0,h = 0;
	GetVgaResolution(&w,&h);
	for(int i = 0; i<maxName; i++)
	{
		CStatic* pText = NULL;
		
		#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		pText = CreateStatic(CRect(0,0,0,0), this, "", TRUE/*, staticTransparent*/);
		#else
		pText = CreateStatic(CRect(0,0,0,0), this, "", TRUE, staticTransparent | staticCrochet);
		#endif
		
		#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		pText->SetTextColor(VD_RGB(180,180,180));
		#else
		pText->SetTextColor(VD_RGB(180,180,180));
		#endif
		
		if(w!=720)//电视输出不需要加粗
		{
			#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
			#else
			pText->SetTextFont(FS_BOLD);
			#endif
		}
		
		pText->SetTextAlign(VD_TA_LEFTTOP);
		
		#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		//去掉刷新桌面时通道名会有黑底闪烁
		//pText->SetBkColor(VD_RGB(255,0,255));
		pText->SetBitmap(pbmpIcon[SHORTCUT_MAX][0]);
		#else
		//去掉刷新桌面时通道名会有黑底闪烁
		//pText->SetBkColor(VD_RGB(255,0,255));
		pText->SetBitmap(pbmpIcon[SHORTCUT_MAX][0]);
		#endif
		
		//if (pBmp1)
		//{
		//	pText->SetBitmap(pBmp1);
		//}
		
		pText->Show(FALSE);
		
		m_vChannelName.push_back(pText);
	}
}

void CPageDesktop::SetChnName( int index, VD_PCRECT pRect, VD_PCSTR psz )
{
	if (index <0 || index >= m_vChannelName.size() || !psz || !pRect)
	{
		return;
	}
    
	VD_RECT Rect;
	memcpy(&Rect, pRect, sizeof(Rect));    
	Rect.right += 20;//添加通道名的显示宽度
	
	m_vChannelName[index]->SetText(psz);
	m_vChannelName[index]->SetRect(&Rect);
	//m_vChannelName[index]->SetTextColor(VD_RGB(255,255,255));
	
	//csp modify
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
}

void CPageDesktop::EditChnName( int index,  VD_PCSTR psz )
{
	if (index <0 || index >= m_vChannelName.size() || !psz)
	{
		return;
	}
	
	m_vChannelName[index]->SetText(psz);
	//m_vChannelName[index]->SetTextColor(VD_RGB(255,255,255));	
}

void CPageDesktop::EditChnWidth( int index ,int width)
{
	if (index <0 || index >= m_vChannelName.size() )
	{
		return;
	}

	CRect rect;
	m_vChannelName[index]->GetRect(&rect);

	rect.right = rect.left+width;
	m_vChannelName[index]->SetRect(rect);
}

void CPageDesktop::ShowChnName( int index, BOOL bShow )
{
	if (index <0 || index >= m_vChannelName.size())
	{
		return;
	}
	
	//printf("index = %d, bShow = %d \n",index, bShow);
	m_vChannelName[index]->Show(bShow,TRUE);
	
	//csp modify
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
}

void CPageDesktop::InitRegion(int maxName)
{
	int w = 0,h = 0;
	GetVgaResolution(&w,&h);
	for(int i = 0; i<16; i++)
	{
		CStatic* pText = NULL;
		
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		pText = CreateStatic(CRect(0,0,0,0), this, "", TRUE);
	#else
		pText = CreateStatic(CRect(0,0,0,0), this, "", TRUE);
	#endif
	pText->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		//pText->SetTextColor(VD_RGB(180,0,0));
	#else
		//pText->SetTextColor(VD_RGB(180,0,0));
	#endif
		
		if(w!=720)//电视输出不需要加粗
		{
		#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		#else
			pText->SetTextFont(FS_BOLD);
		#endif
		}
		
		pText->SetTextAlign(VD_TA_LEFTTOP);
		
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		//去掉刷新桌面时通道名会有黑底闪烁
		//pText->SetBkColor(VD_RGB(255,0,255));
		//pText->SetBitmap(pbmpIcon[SHORTCUT_MAX][0]);
	#else
		//去掉刷新桌面时通道名会有黑底闪烁
		//pText->SetBkColor(VD_RGB(255,0,255));
		//pText->SetBitmap(pbmpIcon[SHORTCUT_MAX][0]);
	#endif
		
		//if (pBmp1)
		//{
		//	pText->SetBitmap(pBmp1);
		//}
		
		//pText->Show(TRUE);
		pText->SetBkColor(VD_RGB(200,16,16));
		
		m_vRegion.push_back(pText);

		nChnRegion[i].x = 50;
		nChnRegion[i].x2 = 400;
		nChnRegion[i].y = 100;
		nChnRegion[i].y2= 400;
	}

}


void CPageDesktop::ShowRegion( int index, BOOL bShow )
{
	if(index <0 || index >= m_vRegion.size())
	{
		return;
	}
	
	m_vRegion[index]->Show(bShow);
	
	//csp modify
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
}


void CPageDesktop::DeleteRegion(int index)
{
	if (index <0 || index >= m_vRegion.size())
	{
		return;
	}

	m_vRegion[index]->SetText("");
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
}

void CPageDesktop::SetRegion( int index, VD_PCRECT pRect, VD_PCSTR psz )
{
	if (index <0 || index >= m_vRegion.size() || !psz || !pRect)
	{
		return;
	}
    
	VD_RECT Rect;
	memcpy(&Rect, pRect, sizeof(Rect));    
	Rect.right += 20;//添加显示宽度

	m_vRegion[index]->SetText(psz);
	m_vRegion[index]->SetRect(&Rect);

	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
}







/*add by liu*/
void CPageDesktop::InitChnKbps(int maxName)
{
	int w = 0,h = 0;
	GetVgaResolution(&w,&h);
	for(int i = 0; i<maxName; i++)
	{
		CStatic* pText = NULL;
		
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		pText = CreateStatic(CRect(0,0,0,0), this, "", TRUE/*, staticTransparent*/);
	#else
		pText = CreateStatic(CRect(0,0,0,0), this, "", TRUE, staticTransparent | staticCrochet);
	#endif
		
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		pText->SetTextColor(VD_RGB(180,180,180));
	#else
		pText->SetTextColor(VD_RGB(180,180,180));
	#endif
		
		if(w!=720)//电视输出不需要加粗
		{
		#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		#else
			pText->SetTextFont(FS_BOLD);
		#endif
		}
		
		pText->SetTextAlign(VD_TA_LEFTTOP);
		
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		//去掉刷新桌面时通道名会有黑底闪烁
		//pText->SetBkColor(VD_RGB(255,0,255));
		pText->SetBitmap(pbmpIcon[SHORTCUT_MAX][0]);
	#else
		//去掉刷新桌面时通道名会有黑底闪烁
		//pText->SetBkColor(VD_RGB(255,0,255));
		pText->SetBitmap(pbmpIcon[SHORTCUT_MAX][0]);
	#endif
		
		//if (pBmp1)
		//{
		//	pText->SetBitmap(pBmp1);
		//}
		
		pText->Show(FALSE);
		
		m_vChannelKbps.push_back(pText);
	}

}

void CPageDesktop::SetChnKbps( int index, VD_PCRECT pRect, VD_PCSTR psz )
{
	if (index <0 || index >= m_vChannelKbps.size() || !psz || !pRect)
	{
		return;
	}
    
	VD_RECT Rect;
	memcpy(&Rect, pRect, sizeof(Rect));    
	Rect.right += 20;//添加显示宽度

	m_vChannelKbps[index]->SetText(psz);
	m_vChannelKbps[index]->SetRect(&Rect);

	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
}

void CPageDesktop::ShowChnKbps( int index, BOOL bShow )
{
	if (index <0 || index >= m_vChannelKbps.size())
	{
		return;
	}
	
	//printf("index = %d, bShow = %d \n",index, bShow);
	m_vChannelKbps[index]->Show(bShow,TRUE);
	
	//csp modify
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
}

void CPageDesktop::InitTimeTitle( int max /*= 1*/ )
{
	int w = 0,h = 0;
	GetVgaResolution(&w,&h);
	for(int i = 0; i<max; i++)
	{
		CStatic* pText = NULL;
		
		#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		char tmp2[20] = {0};
		GetProductNumber(tmp2);
		if(0 == strcasecmp(tmp2, "R3104HD") || 0 == strcasecmp(tmp2, "R3110HDW") || 0 == strcasecmp(tmp2, "R3106HDW"))//csp modify 20130516
		{
			if(w > 1024)
			{
				pText = CreateStatic(CRect(0,0,0,0), this, "", TRUE, 0, FS_SMALL);
			}
			else
			{
				pText = CreateStatic(CRect(0,0,0,0), this, "", TRUE/*, staticTransparent*/);
			}
		}
		else
		{
			pText = CreateStatic(CRect(0,0,0,0), this, "", TRUE/*, staticTransparent*/);
		}
		#else
		pText = CreateStatic(CRect(0,0,0,0), this, "", TRUE, staticTransparent | staticCrochet);
		#endif
		
		#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		pText->SetTextColor(VD_RGB(255,255,255));
		#else
		pText->SetTextColor(VD_RGB(255,255,0));
		#endif
		
		#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		#else
		//用贴图方法去除时间闪的问题
		//pText->SetBkColor(VD_RGB(255,0,255));
		pText->SetBitmap(pbmpIcon[SHORTCUT_MAX][0]);
		#endif
		
		if(w!=720)//电视输出不需要加粗
		{
			#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
			#else
			pText->SetTextFont(FS_BOLD);
			#endif
		}
		
		#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		pText->SetTextAlign(VD_TA_LEFTTOP);
		#else
		pText->SetTextAlign(VD_TA_LEFT);
		#endif
		
		pText->Show(FALSE);
		
		m_vTimeTile.push_back(pText);
	}
}

void CPageDesktop::SetTimeTitleString( int index, VD_PCSTR psz )
{
	if(index <0 || index >= m_vTimeTile.size() || !psz)
	{
		return;
	}
	
	static u8 ampm = 0;
	
	m_vTimeTile[index]->SetText(psz);
	if((0 == ampm) && 
		(('m' == *(psz+strlen(psz)-1))
		|| ('M' == *(psz+strlen(psz)-1))))
	{
		CRect rect;
		m_vTimeTile[index]->GetRect(&rect);
		rect.right+=24;
		m_vTimeTile[index]->SetRect(&rect);
		ampm = 1;
	}
	else if((ampm) && 
		(('m' != *(psz+strlen(psz)-1))
		&& ('M' != *(psz+strlen(psz)-1))))
	{
		CRect rect;
		m_vTimeTile[index]->GetRect(&rect);
		rect.right-=24;
		m_vTimeTile[index]->SetRect(&rect);
		ampm = 0;
	}
	
	//csp modify
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
}

void CPageDesktop::SetTimeTitleRect( int index, VD_PCRECT pRect)
{
	if(index <0 || index >= m_vTimeTile.size() || !pRect)
	{
		return;
	}
	
	m_vTimeTile[index]->SetRect(pRect);
}

void CPageDesktop::ShowTimeTitle( int index, BOOL bShow )
{
	if(index <0 || index >= m_vTimeTile.size())
	{
		return;
	}
	
	m_vTimeTile[index]->Show(bShow);
	
	//csp modify
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
}

void CPageDesktop::SetElecZoomChn( int chn)
{
	m_CurChn = chn;
}

int CPageDesktop::GetElecZoomChn()
{
	return m_CurChn;
}

void CPageDesktop::SetAllElecZoomStatueEmpty()
{
	m_IsPbElecZoomDrawStatus = 0;
	m_IsElecZoomDrawStatus = 0;
	IsDrawElecZoomArea = 0;
	IsElecZoomBegin = 0;
	IsElecZoomStatue = 0;

	ShowElecZoomTile(0,FALSE);
	ShowElecZoomInvalidTile(0,FALSE);
	ShowElecZoomSplitLine(0,FALSE);
	ShowElecZoomSplitLine(1,FALSE);

	for(int i = 0; i < 4; i++)
	{
		ShowElecZoomAreaSplitLine(i, FALSE);
	}
}


void CPageDesktop::InitElecZoomTile( int max)
{
	for (int i = 0; i<max; i++)
	{
		CStatic* pText = CreateStatic(CRect(0,0,0,0), this, "", TRUE);
		pText->SetTextFont(FS_BOLD);
		pText->SetTextAlign(VD_TA_CENTER);
		pText->Show(FALSE);

		m_vElecZoomTile.push_back(pText);
	}
}

void CPageDesktop::SetElecZoomTileString( int index, VD_PCSTR psz )
{
	if (index <0 || index >= m_vElecZoomTile.size() || !psz)
	{
		return;
	}

	m_vElecZoomTile[index]->SetText(psz);
}

void CPageDesktop::SetElecZoomstatus(BOOL flag)
{
	m_IsElecZoomDrawStatus = flag;
	//printf(" ^_^ m_IsElecZoomDrawStatus = %d\n",m_IsElecZoomDrawStatus);
}

void CPageDesktop::SetPbElecZoomstatus(BOOL flag)
{
	m_IsPbElecZoomDrawStatus = flag;
	//printf(" ^_^ m_IsPbElecZoomDrawStatus = %d\n",m_IsPbElecZoomDrawStatus);
}
	
void CPageDesktop::SetElecZoomRect( int index, VD_PCRECT pRect)
{
	if (index <0 || index >= m_vElecZoomTile.size() || !pRect)
	{
		return;
	}

	m_vElecZoomTile[index]->SetRect(pRect);
}

void CPageDesktop::ShowElecZoomTile( int index, BOOL bShow )
{
	if (index <0 || index >= m_vElecZoomTile.size())
	{
		return;
	}

	m_vElecZoomTile[index]->Show(bShow);
}
//  *******************************************************************
void CPageDesktop::InitElecZoomInvalidTile( int max)
{
	for (int i = 0; i<max; i++)
	{
		CStatic* pText = CreateStatic(CRect(0,0,0,0), this, "", TRUE);
		pText->SetTextFont(FS_BOLD);
		pText->SetTextAlign(VD_TA_CENTER);
		pText->Show(FALSE);

		m_vElecZoomInvalidTile.push_back(pText);
	}
}

void CPageDesktop::SetElecZoomInvalidTileString( int index, VD_PCSTR psz )
{
	if (index <0 || index >= m_vElecZoomInvalidTile.size() || !psz)
	{
		return;
	}

	m_vElecZoomInvalidTile[index]->SetText(psz);
}
	
void CPageDesktop::SetElecZoomInvalidRect( int index, VD_PCRECT pRect)
{
	if (index <0 || index >= m_vElecZoomInvalidTile.size() || !pRect)
	{
		return;
	}

	m_vElecZoomInvalidTile[index]->SetRect(pRect);
}

void CPageDesktop::ShowElecZoomInvalidTile( int index, BOOL bShow )
{
	if (index <0 || index >= m_vElecZoomInvalidTile.size())
	{
		return;
	}

	m_vElecZoomInvalidTile[index]->Show(bShow);
}

void CPageDesktop::InitChnState( int maxChn /*= 4*/, int maxstatetype /*= 5*/ )
{
	m_nMaxStatetype = maxstatetype;
	for (int i = 0; i<maxChn; i++)
	{
		for (int kk = 0; kk <maxstatetype; kk++)
		{
			CStatic* pBMP1 = CreateStatic(CRect(0,0,0,0), this, (VD_BITMAP*)NULL);
			CStatic* pBMP2 = CreateStatic(CRect(0,0,0,0), this, (VD_BITMAP*)NULL);
			pBMP1->Show(FALSE);
			pBMP2->Show(FALSE);
			m_vChnState.push_back(pBMP1);
			m_vChnState.push_back(pBMP2);
		}
	}
}
//add by liu
void CPageDesktop::InitChnElseState(int maxChn/* = 16*/, int maxstatetype /*= 3*/)
{
	m_nMaxElseStatetype = maxstatetype;
	for (int i = 0; i<maxChn; i++)
	{
		for (int kk = 0; kk <maxstatetype; kk++)
		{
			CStatic* pBMP1 = CreateStatic(CRect(0,0,0,0), this, (VD_BITMAP*)NULL);
			CStatic* pBMP2 = CreateStatic(CRect(0,0,0,0), this, (VD_BITMAP*)NULL);
			
			pBMP1->Show(FALSE);
			pBMP2->Show(FALSE);
			m_vChnElseState.push_back(pBMP1);
			m_vChnElseState.push_back(pBMP2);
		}
	}
}

void CPageDesktop::SetChnElseStateBMP(int nChn, int nStatetype, VD_PCSTR pszBmpName1, VD_PCSTR pszBmpName2 )
{
	if (nChn <0 || nStatetype <0 || nStatetype >= m_nMaxElseStatetype)
		{
			return;
		}
	int nIdx = (m_nMaxElseStatetype*nChn + nStatetype)*2;
	if ((nIdx +1) >= m_vChnElseState.size())
	{
		return;
	}
	VD_BITMAP* pBmp1 = VD_LoadBitmap(pszBmpName1);
	VD_BITMAP* pBmp2 = VD_LoadBitmap(pszBmpName2);
	CRect rt;
	m_vChnElseState[nIdx + 0]->GetRect(&rt);
	if (pBmp1)
	{
		m_vChnElseState[nIdx + 0]->SetBitmap(pBmp1);
		//csp modify 20130429
		//m_vChnState[nIdx + 0]->SetRect(CRect(rt.left, rt.top, rt.left + pBmp1->width, rt.top+pBmp2->height), FALSE);
		m_vChnElseState[nIdx + 0]->SetRect(CRect(rt.left, rt.top, rt.left + pBmp1->width, rt.top+pBmp1->height), FALSE);
	}
	
	m_vChnElseState[nIdx + 1]->GetRect(&rt);
	if (pBmp2)
	{
		m_vChnElseState[nIdx + 1]->SetBitmap(pBmp2);
		m_vChnElseState[nIdx + 1]->SetRect(CRect(rt.left, rt.top, rt.left + pBmp2->width, rt.top+pBmp2->height), FALSE);
	}

}

void CPageDesktop::SetChnStateBMP( int nChn, int nStatetype, VD_PCSTR pszBmpName1, VD_PCSTR pszBmpName2 )
{
	if (nChn <0 || nStatetype <0 || nStatetype >= m_nMaxStatetype)
	{
		return;
	}
	
	int nIdx = (m_nMaxStatetype*nChn + nStatetype)*2;
	if ((nIdx +1) >= m_vChnState.size())
	{
		return;
	}
	
	VD_BITMAP* pBmp1 = VD_LoadBitmap(pszBmpName1);
	VD_BITMAP* pBmp2 = VD_LoadBitmap(pszBmpName2);
	
	CRect rt;
	m_vChnState[nIdx + 0]->GetRect(&rt);
	if (pBmp1)
	{
		m_vChnState[nIdx + 0]->SetBitmap(pBmp1);
		//csp modify 20130429
		//m_vChnState[nIdx + 0]->SetRect(CRect(rt.left, rt.top, rt.left + pBmp1->width, rt.top+pBmp2->height), FALSE);
		m_vChnState[nIdx + 0]->SetRect(CRect(rt.left, rt.top, rt.left + pBmp1->width, rt.top+pBmp1->height), FALSE);
	}
	
	m_vChnState[nIdx + 1]->GetRect(&rt);
	if (pBmp2)
	{
		m_vChnState[nIdx + 1]->SetBitmap(pBmp2);
		m_vChnState[nIdx + 1]->SetRect(CRect(rt.left, rt.top, rt.left + pBmp2->width, rt.top+pBmp2->height), FALSE);
	}
}

/*
nStatetype:

EM_STATE_RECORD, //录像状态                         0
EM_STATE_MD, //移动侦测					1
EM_STATE_ALARM, //报警录像					2
EM_STATE_TIMER, //定时录像					3
EM_STATE_MANUAL_REC, //手动录像			4

showtype:
EM_CHNSTATE_HIDE, 
EM_CHNSTATE_1,	
EM_CHNSTATE_2,
*/
void CPageDesktop::ShowChnState( int nChn, int nStatetype, int x, int y, EMCHNSTATESHOW showtype )
{	
	if (nChn < 0 || nStatetype < 0 || nStatetype >= m_nMaxStatetype)
	{
		return;
	}
	
	int nIdx = (m_nMaxStatetype*nChn + nStatetype)*2;
	if ((nIdx + 1) >= m_vChnState.size())
	{
		return;
	}
	
	CRect rt;
	//m_vChnState[nIdx + 0]->GetRect(&rt);
	//m_vChnState[nIdx + 0]->SetRect(CRect(x, y, x+rt.Width(), y+rt.Height()), TRUE);
	//printf("chn:%d,type:%d, old[%d,%d,%d,%d], new[%d,%d,%d,%d] \n",nChn,nStatetype, rt.left, rt.top, rt.right, rt.bottom ,x,y,x+rt.Width(),y+rt.Height());
	//m_vChnState[nIdx + 1]->GetRect(&rt);
	//m_vChnState[nIdx + 1]->SetRect(CRect(x, y, x+rt.Width(), y+rt.Height()), TRUE);
	//showtype = EM_CHNSTATE_1; //add by liu
	
	if(!m_FlagPlaying)
	{
		switch (showtype)
		{
		case EM_CHNSTATE_HIDE:
			if(!m_vChnState[nIdx]->IsShown())
			{
				break;
			}
			
			m_vChnState[nIdx + 0]->Show(FALSE,TRUE);
			m_vChnState[nIdx + 1]->Show(FALSE,TRUE);

			for(int i = nStatetype + 1; i < m_nMaxStatetype; i++)
			{
				nIdx = (m_nMaxStatetype*nChn + i)*2;
				if(m_vChnState[nIdx]->IsShown())
				{
					m_vChnState[nIdx]->Show(FALSE,TRUE);
					m_vChnState[nIdx]->GetRect(&rt);
					m_vChnState[nIdx]->SetRect(CRect(rt.left - rt.Width(), y, rt.left, y+rt.Height()), TRUE);
					//printf("chn:%d, type:%d  old[%d,%d,%d,%d], new[%d,%d,%d,%d] \n",nChn,i, rt.left, rt.top, rt.right, rt.bottom ,rt.left - rt.Width(), y, rt.left, y+rt.Height());
					m_vChnState[nIdx]->Show(TRUE,TRUE);
				}
			}
			break;
			
		case EM_CHNSTATE_1:
			if(m_vChnState[nIdx]->IsShown())
			{
				break;
			}
			
			m_vChnState[nIdx + 0]->GetRect(&rt);
			m_vChnState[nIdx + 0]->SetRect(CRect(x, y, x+rt.Width(), y+rt.Height()), TRUE);
			
			for(int i = 0; i < nStatetype; i++)
			{
				if(m_vChnState[(m_nMaxStatetype*nChn + i)*2]->IsShown())
				{
					m_vChnState[nIdx]->GetRect(&rt);
					m_vChnState[nIdx]->SetRect(CRect(rt.left + rt.Width(), y, rt.right + rt.Width(), y+rt.Height()), TRUE);
				}
			}
			
			m_vChnState[nIdx + 0]->Show(TRUE,TRUE);
			m_vChnState[nIdx + 1]->Show(FALSE,TRUE);
			
			for(int i = nStatetype + 1; i < m_nMaxStatetype; i++)
			{
				nIdx = (m_nMaxStatetype*nChn + i)*2;
				if(m_vChnState[nIdx]->IsShown())
				{
					m_vChnState[nIdx]->Show(FALSE,TRUE);
					m_vChnState[nIdx]->GetRect(&rt);
					m_vChnState[nIdx]->SetRect(CRect(rt.left + rt.Width(), y, rt.right + rt.Width(), y+rt.Height()), TRUE);
					//printf("chn:%d, type:%d old[%d,%d,%d,%d], new[%d,%d,%d,%d]\n",nChn,i, rt.left, rt.top, rt.right, rt.bottom ,rt.left + rt.Width(), y, rt.right + rt.Width(), y+rt.Height());
					m_vChnState[nIdx]->Show(TRUE,TRUE);
				}
			}
			break;
			
		case EM_CHNSTATE_2:
			//m_vChnState[nIdx + 0]->Show(FALSE,TRUE);
			//m_vChnState[nIdx + 1]->Show(TRUE,TRUE);
			break;
		}
	}
	else //m_FlagPlaying
	{
		switch (showtype)
		{
		case EM_CHNSTATE_HIDE:
			if(!m_AllChnStatus[nIdx])
			{
				break;
			}
			
			m_AllChnStatus[nIdx + 0] = 0;
			m_AllChnStatus[nIdx + 1] = 0;
			
			for(int i = nStatetype + 1; i < m_nMaxStatetype; i++)
			{
				nIdx = (m_nMaxStatetype*nChn + i)*2;
				if(m_AllChnStatus[nIdx])
				{
					m_vChnState[nIdx]->GetRect(&rt);
					m_vChnState[nIdx]->SetRect(CRect(rt.left - rt.Width(), y, rt.left, y+rt.Height()), TRUE);					
				}
			}
			break;
			
		case EM_CHNSTATE_1:
			if(m_AllChnStatus[nIdx])
			{
				break;
			}
			
			m_vChnState[nIdx + 0]->GetRect(&rt);
			m_vChnState[nIdx + 0]->SetRect(CRect(x, y, x+rt.Width(), y+rt.Height()), TRUE);

			for(int i = 0; i < nStatetype; i++)
			{
				if(m_AllChnStatus[(m_nMaxStatetype*nChn + i)*2])
				{
					m_vChnState[nIdx]->GetRect(&rt);
					m_vChnState[nIdx]->SetRect(CRect(rt.left + rt.Width(), y, rt.right + rt.Width(), y+rt.Height()), TRUE);
				}
			}

			m_AllChnStatus[nIdx + 0] = 1;
			m_AllChnStatus[nIdx + 1] = 0;

			for(int i = nStatetype + 1; i < m_nMaxStatetype; i++)
			{
				nIdx = (m_nMaxStatetype*nChn + i)*2;
				if(m_AllChnStatus[nIdx])
				{
					m_vChnState[nIdx]->GetRect(&rt);
					m_vChnState[nIdx]->SetRect(CRect(rt.left + rt.Width(), y, rt.right + rt.Width(), y+rt.Height()), TRUE);
				}
			}
			break;
			
		case EM_CHNSTATE_2:
			//m_vChnState[nIdx + 0]->Show(FALSE,TRUE);
			//m_vChnState[nIdx + 1]->Show(TRUE,TRUE);
			break;
		}
	}
	
	//csp modify
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
}

//yaogang modify 20150130
VD_BOOL CPageDesktop::ChnElseStateIsShow(int nChn, int nStatetype)
{
	if (nChn < 0 || nStatetype < 0 || nStatetype >= 3)
	{
		printf("%s param invalid, nChn: %d, nStatetype: %d\n", __func__, nChn, nStatetype);
		return FALSE;
	}
	int nIdx = (m_nMaxElseStatetype*nChn + nStatetype)*2;

	return m_vChnElseState[nIdx]->IsShown();
}

//add by liu
void CPageDesktop::ShowChnElseState( int nChn, int nStatetype, int x, int y, EMCHNSTATESHOW showtype )
{
	#ifdef NO_MODULE_SG
		return;
	#endif
	
	if (nChn < 0 || nStatetype < 0 || nStatetype >= 3)
	{
		return;
	}
	int nIdx = (m_nMaxElseStatetype*nChn + nStatetype)*2;
	CRect rt;

	//add by zcm
	//showtype = EM_CHNSTATE_1;
	//if(!m_FlagPlaying)
	//{
		switch (showtype)
		{
			case EM_CHNSTATE_HIDE:
			{
				m_vChnElseState[nIdx]->Show(FALSE,TRUE);
				m_vChnElseState[nIdx+1]->Show(FALSE,TRUE);
			} break;
			case EM_CHNSTATE_1:	
			{
				m_vChnElseState[nIdx]->GetRect(&rt);
				m_vChnElseState[nIdx]->SetRect(CRect(x, y, x+rt.Width(), y+rt.Height()), TRUE);	

				//yaogang modify 20150324
				if (nNVROrDecoder == 1)//nvr
					m_vChnElseState[nIdx]->Show(TRUE,TRUE);
			} break;
			case EM_CHNSTATE_2:
			{
				m_vChnElseState[nIdx+1]->GetRect(&rt);
				m_vChnElseState[nIdx+1]->SetRect(CRect(x, y, x+rt.Width(), y+rt.Height()), TRUE);	
				m_vChnElseState[nIdx+1]->Show(TRUE,TRUE);
			} break;
		}
	//}
	//else
	//{
	//	m_vChnElseState[nIdx]->Show(FALSE,TRUE);
	//	m_vChnElseState[nIdx+1]->Show(FALSE,TRUE);
	//}

	
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}


}

void CPageDesktop::SetModePlaying()
{
	if(m_FlagPlaying)
	{
		return;
	}
	
	m_FlagPlaying = 1;
	if(!m_AllChnStatus)
	{
		m_AllChnStatus = (u8*)calloc(GetVideoMainNum(), m_nMaxStatetype*2);
	}
	u8 chnnum = GetVideoMainNum();
	for(int i = 0; i < m_nMaxStatetype*chnnum*2; i++)
	{
		if(m_vChnState[i]->IsShown())
		{
			m_AllChnStatus[i] = 1;
		}
		else
		{
			m_AllChnStatus[i] = 0;
		}
	}
	
	ClearAllChnState();

//yaogang modify 20141105

	for (int i = 0; i<maxChn * m_nMaxElseStatetype * 2; i++)
	{	
		m_vChnElseState[i]->Show(FALSE,TRUE);
		m_vChnElseState[i]->Show(FALSE,TRUE);
	}

}

void CPageDesktop::SetModePreviewing()
{
	if(!m_FlagPlaying)
	{
		return;
	}
	
	m_FlagPlaying = 0;
	if(!m_AllChnStatus)
	{
		m_AllChnStatus = (u8*)calloc(GetVideoMainNum(), m_nMaxStatetype*2);
	}
	u8 chnnum = GetVideoMainNum();
	for(int i = 0; i < m_nMaxStatetype*chnnum*2; i++)
	{
		if(m_AllChnStatus[i])
		{
			m_vChnState[i]->Show(TRUE,TRUE);
		}
		else
		{
			m_vChnState[i]->Show(FALSE,TRUE);
		}
	}

//yaogang modify 20141105
/*
	for (int i = 0; i<maxChn * m_nMaxElseStatetype * 2; i++)
	{	
		m_vChnElseState[i]->Show(TRUE,TRUE);
		m_vChnElseState[i]->Show(TRUE,TRUE);
	}
*/
	
}

void CPageDesktop::ClearAllChnState()
{
	u8 chnnum = GetVideoMainNum();
	for(int i = 0; i < m_nMaxStatetype*chnnum*2; i++)
	{
		m_vChnState[i]->Show(FALSE,TRUE);		
	}
	
	//csp modify
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
}

void CPageDesktop::InitCoverRect(int maxCh, int maxRect, VD_COLORREF color)
{
	m_maxCH = maxCh;
	m_maxRect = maxRect;
	m_curCover = (int*)malloc(maxCh);
	memset(m_curCover, 0, maxCh);

	//printf("maxch=%d, maxrect = %d \n",maxCh,maxRect);

	//printf("InitCoverRect \n");
	for (int i = 0; i<m_maxCH*m_maxRect; i++)
	{
		//printf("CreateStatic \n");
		CStatic* pMask = CreateStatic(CRect(0,0,0,0), this, "");
		pMask->SetBkColor(color);
		pMask->Show(FALSE);

		m_vCoverRect.push_back(pMask);
	}

}

void CPageDesktop::SetCoverRect(int ch, int x, int y, int w, int h)
{
	if(m_curCover[ch] >= m_maxRect)
	{
		return;
	}

	//printf("m_curCover = %d,x=%d,y=%d,w=%d,h=%d\n",m_curCover,x,y,w,h);
	int index = ch*m_maxRect + m_curCover[ch];
	m_vCoverRect[index]->SetRect(CRect(x, y, x+w, y+h), TRUE);
	m_vCoverRect[index]->Show(TRUE,TRUE);

	m_curCover[ch]++;
}

void CPageDesktop::ClearCoverRect(int ch)
{
	for(int i=0; i<m_maxRect; i++)
	{
		m_vCoverRect[ch*m_maxRect + i]->SetRect(CRect(0, 0, 0, 0), TRUE);
	}

	m_curCover[ch] = 0;
}

void CPageDesktop::HideCoverRect()
{
	for(int j=0; j<GetVideoMainNum(); j++)
	{
		for(int i=0; i<m_maxRect; i++)
		{
			m_vCoverRect[j*m_maxCH + i]->Show(FALSE);
		}
	}
}


void CPageDesktop::ShowCoverRect()
{
	for(int j=0; j<GetVideoMainNum(); j++)
	{
		for(int i=0; i<m_maxRect; i++)
		{
			m_vCoverRect[j*m_maxCH + i]->Show(TRUE);
		}
	}
}

void CPageDesktop::SetCurPreviewMode(EMBIZPREVIEWMODE emMod)
{
    emCurPreviewMode = emMod;
}

void CPageDesktop::SetCurPreModePage(u32 nPage)
{
    nPreviewPage = nPage;
}

u32 CPageDesktop::ClikCurChn(EMBIZPREVIEWMODE emMod, u32 nPage, int x, int y)
{
	static int nScreanWidth = 800,  nScreanHeight = 600;
	
	float nOffSet25 = 0.0f;
	float height = 0.0f;
	
	u32 nPreWinPage = 4;//预览画面模式每页多少个预览窗口
	
	GetVgaResolution(&nScreanWidth, &nScreanHeight);
	
    switch(emMod)
    {
        case EM_BIZPREVIEW_4SPLITS:
        {
            RowNum = ColNum = 2;
            nPreWinPage = 4;
        }
        break;
        case EM_BIZPREVIEW_9SPLITS:
        {
            RowNum = ColNum = 3;
            nPreWinPage = 9;
        }
        break;
		case EM_BIZPREVIEW_6SPLITS:
        case EM_BIZPREVIEW_10SPLITS:
        case EM_BIZPREVIEW_16SPLITS:
        {
            RowNum = ColNum = 4;
            nPreWinPage = 16;
        }
        break;
        case EM_BIZPREVIEW_24SPLITS:
        {
            RowNum = 4;
			ColNum = 6;
            nPreWinPage = 24;
        }
        break;
		case EM_BIZPREVIEW_25SPLITS:
        {
			nOffSet25 = 8;
            RowNum = ColNum = 5;
            nPreWinPage = 25;
        }
        break;
        case EM_BIZPREVIEW_36SPLITS:
        {
            RowNum = ColNum = 6;
            nPreWinPage = 36;
        }
        break;
		//csp modify 20130501
		case EM_BIZPREVIEW_PIP:
		{
			RowNum = ColNum = 2;
			nPreWinPage = 4;
		}
		break;
        default:
        {
            RowNum = ColNum = 2;
            nPreWinPage = 4;
        }
        break;
    }
	
	if(nOffSet25 > 0)
	{
		//SBizSystemPara sysPara;
		//BizGetSysInfo(&sysPara);

		if(EM_BIZ_PAL == sysPara.nVideoStandard)
		{
			height = nScreanHeight * (CurScreenHeight - nOffSet25 * 2) / RowNum / CurScreenHeight;
			nOffSet25 = nScreanHeight * nOffSet25 / CurScreenHeight;			
		}
		else
		{
			nOffSet25 = 0.0f;
			height = nScreanHeight/RowNum;
		}
	}
	else
	{
		height = nScreanHeight/RowNum;
	}
	
    u32 width = nScreanWidth/ColNum;
	
    for(u32 i=0; i<RowNum; ++i)
    {
        for(u32 j=0; j<ColNum; ++j)
        {
            if((j*width<x && x<width*(j+1)) 
                && ((i*height + ((i > 0) ? nOffSet25 : 0)) <= y 
                    && (y < ((i+1)*height + nOffSet25 + (((nOffSet25 > 0) 
                    &&(i == RowNum - 1)) ? nOffSet25 : 0)))))
            {
                //最后一页时要全部填充完
                if(nPage == GetMaxChnNum()/nPreWinPage && nPage != 0)
                {
                    return nPreWinPage*nPage+i*ColNum+j-(nPreWinPage*(nPage+1)-GetMaxChnNum());
                } 
                else 
                {
                    return nPreWinPage*nPage+i*ColNum+j;
                }
            }
        }
    }
	
    return 255;
}

void CPageDesktop::SetPageFlag(BOOL bFlag)
{
	bThisPageFlag = bFlag;
}

void CPageDesktop::SetIsDwellFlag(BOOL bFlag)
{
	IsDwell = bFlag;
}

void CPageDesktop::SetCurVideoSize()//改变电子放大标准尺寸
{
	CurScreenWidth = 720;
	
	if(Is9616S && m_IsElecZoomDrawStatus && (m_CurChn > 7))
	{
		CurScreenHeight = 768;
		CurScreenWidth = 1024;
	}
	else if(EM_BIZ_PAL == sysPara.nVideoStandard)
	{
		CurScreenHeight = 576;
	}
	else
	{
		CurScreenHeight = 480;
	}
}

int CPageDesktop::ExitFromDrawStatue()
{
	printf("绘画模式下右键退出\n");
	
	if((m_IsPbElecZoomDrawStatus || m_IsElecZoomDrawStatus) && IsDrawElecZoomArea)
	{
		return -1;
	}
	
	ShowElecZoomTile(0,FALSE);
	ShowElecZoomInvalidTile(0,FALSE);
	
	if(m_IsPbElecZoomDrawStatus)
	{
		CPage** page = GetPage();
		
		//((CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK])->m_Mutex.Enter();//csp modify 20121118
		
		((CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK])->SetZoomStatue(0);
		((CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK])->SetExitStatue(1);
		
		//csp modify 20121118
		if(!((CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK])->GetPBFinished())
		{
			//printf("Open CPagePlayBackFrameWork\n");
			
			((CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK])->Open();
			
			//printf("Open CPagePlayBackFrameWork OK\n");
			
			//csp modify 20121118
			if(!((CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK])->GetPBFinished())
			{
				int num = ((CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK])->GetPbNum();
				if(1 == num)
				{
					
				}
				else
				{
					BizPlayBackZoom(4);
				}
			}
			else//csp modify 20121118
			{
				((CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK])->Close();
				
				m_IsPbElecZoomDrawStatus = 0;
				m_IsElecZoomDrawStatus = 0;
				
				SetModePreviewing();
				
				if(IsDwell)
				{
					usleep(500 * 1000);
					
					ControlPatrol(1);
					IsDwell = 0;
				}
			}
		}
		else//csp modify 20121118
		{
			printf("playback finished,do nothing\n");
			
			m_IsPbElecZoomDrawStatus = 0;
			m_IsElecZoomDrawStatus = 0;
			
			SetModePreviewing();
			
			if(IsDwell)
			{
				usleep(500 * 1000);
				
				ControlPatrol(1);
				IsDwell = 0;
			}
			
			//((CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK])->m_Mutex.Leave();//csp modify 20121118
			
			return TRUE;
		}
		
		//((CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK])->m_Mutex.Leave();//csp modify 20121118
	}
	else
	{
		BizSetZoomMode(0);
		
		switch(maxChn)
		{
			case 4:
				SetCurPreviewMode_CW(EM_BIZPREVIEW_4SPLITS);//cw_preview
				SwitchPreview(EM_BIZPREVIEW_4SPLITS, 0);
				break;
			case 8:
				SetCurPreviewMode_CW(EM_BIZPREVIEW_9SPLITS);//cw_preview
				SwitchPreview(EM_BIZPREVIEW_9SPLITS, 0);
				break;
			//csp modify 20130504
			case 10:
				SetCurPreviewMode_CW(EM_BIZPREVIEW_10SPLITS);//cw_preview
				SwitchPreview(EM_BIZPREVIEW_10SPLITS, 0);
				break;
			case 6:
				SetCurPreviewMode_CW(EM_BIZPREVIEW_6SPLITS);//cw_preview
				SwitchPreview(EM_BIZPREVIEW_6SPLITS, 0);
				break;
			case 16:
				SetCurPreviewMode_CW(EM_BIZPREVIEW_16SPLITS);//cw_preview
				SwitchPreview(EM_BIZPREVIEW_16SPLITS, 0);
				break;
			case 24:
				SetCurPreviewMode_CW(EM_BIZPREVIEW_25SPLITS);//cw_preview
				//SwitchPreview(EM_BIZPREVIEW_24SPLITS, 0);
				SwitchPreview(EM_BIZPREVIEW_25SPLITS, 0);
				break;
			case 32:
				SetCurPreviewMode_CW(EM_BIZPREVIEW_36SPLITS);//cw_preview
				SwitchPreview(EM_BIZPREVIEW_36SPLITS, 0);
				break;
			default:
				break;
		}
		
		#if 1//csp modify 20121118
		m_IsElecZoomDrawStatus = 0;
		m_IsPbElecZoomDrawStatus = 0;
		
		SetModePreviewing();
		
		if(IsDwell)
		{
			usleep(500 * 1000);
			
			ControlPatrol(1);
			IsDwell = 0;
		}
		#endif
	}
	
	#if 0//csp modify 20121118
	SetModePreviewing();
	
	m_IsElecZoomDrawStatus = 0;
	m_IsPbElecZoomDrawStatus = 0;
	
	if(IsDwell)
	{
		usleep(500 * 1000);
		
		ControlPatrol(1);
		IsDwell = 0;
	}
	#endif
	
	#if 1//csp modify 20121118
	m_IsElecZoomDrawStatus = 0;
	m_IsPbElecZoomDrawStatus = 0;
	#endif
	
	return TRUE;
}

void CPageDesktop::ExitFromZoomStatue()
{
	printf("放大模式下右键退出\n");
	
	ShowElecZoomSplitLine(0,FALSE);
	ShowElecZoomSplitLine(1,FALSE);
	
	for(int i = 0; i < 4; i++)
	{
		ShowElecZoomAreaSplitLine(i, FALSE);
		usleep(10 * 1000);
	}
	
	SBizPreviewElecZoom stCapRectExit;
	
	stCapRectExit.s32X = 128;
	stCapRectExit.s32Y = 128;
	stCapRectExit.u32Height = 128;
	stCapRectExit.u32Width = 128;
	
	IsElecZoomStatue = 0;
	IsElecZoomBegin = 0;
	
	if(m_IsPbElecZoomDrawStatus)
	{
		PreviewPbElecZoom(1, m_CurChn, &stCapRectExit);
	}
	else
	{
		//printf("ExitFromZoomStatue-1\n");
		PreviewElecZoom(1, m_CurChn, &stCapRectExit);
		//printf("ExitFromZoomStatue-2,m_CurChn=%d\n",m_CurChn);
		BizChnPreview(m_CurChn);
		//printf("ExitFromZoomStatue-3\n");
	}
	
	ShowElecZoomTile(0, TRUE);
}

int CPageDesktop::DrawZoomArea()
{
	//printf("开始绘画电子放大区域\n");
	
	int w = px - mouse_eleczoom_x;
	int h = py - mouse_eleczoom_y;
	
	ShowChnName(m_CurChn, FALSE);
	ShowElecZoomTile(0,FALSE);
	ShowElecZoomInvalidTile(0,FALSE);
	
	if((w < 0) || (h < 0))
	{
		SetElecZoomInvalidTileString(0, "&CfgPtn.WrongOperationAreaSet");
		ShowElecZoomInvalidTile(0,TRUE);
		//ShowElecZoomTile(0,FALSE);
		IsOperaterror = 1;
		
		return -1;
	}
	
	if(1 == IsOperaterror)
	{
		IsOperaterror = 0;
		ShowElecZoomInvalidTile(0,FALSE);
		ShowElecZoomTile(0,TRUE);
	}
	
	SetElecZoomAreaSplitLineRect(0, mouse_eleczoom_x, mouse_eleczoom_y, w, EM_SPLIT_H);
	SetElecZoomAreaSplitLineRect(1, mouse_eleczoom_x, py, w, EM_SPLIT_H);
	SetElecZoomAreaSplitLineRect(2, mouse_eleczoom_x, mouse_eleczoom_y, h, EM_SPLIT_V);
	SetElecZoomAreaSplitLineRect(3, px - 2, mouse_eleczoom_y, h, EM_SPLIT_V);
	
	for(int i = 0; i < 4; i++)
	{
		ShowElecZoomAreaSplitLine(i, TRUE);
	}
	
	return 0;
}

int CPageDesktop::ChangZoomArea()
{
	if(((mouse_eleczoom_x < iszoom_x1) || (mouse_eleczoom_x > iszoom_x2)) 
	    || ((mouse_eleczoom_y < iszoom_y1) || (mouse_eleczoom_y > iszoom_y2)))
	{
		//printf("鼠标不在放大矩形区域 !\n");
		return -1;
	}
	else
	{
		//printf("鼠标在放大矩形区域 !\n");
		if(IsLbuttonDown)
		{
			int nOffsetX = px - mouse_eleczoom_x;
			int nOffsetY = py - mouse_eleczoom_y;
			
			if (nOffsetX + iszoom_x1 < nScreanWidth * 3 / 4)
			{
				nOffsetX = nScreanWidth * 3 / 4 - iszoom_x1;
			}
			
			if (nOffsetX + iszoom_x2 > nScreanWidth)
			{
				nOffsetX = nScreanWidth - iszoom_x2;
			}
			
			if (nOffsetY + iszoom_y1 < nScreanHeight * 3 / 4)
			{
				nOffsetY = nScreanHeight * 3 / 4 - iszoom_y1;
			}
			
			if (nOffsetY + iszoom_y2 > nScreanHeight)
			{
				nOffsetY = nScreanHeight - iszoom_y2;
			}
			
			int nZoomOffsetX = nOffsetX * CurScreenWidth / nScreanWidth / 2 * 2;
			int nZoomOffsetY = nOffsetY * CurScreenHeight / nScreanHeight / 2 * 2;
			
			nOffsetX = nScreanWidth * 3 / 4 + ((stCapRect.s32X + nZoomOffsetX) * nScreanWidth / CurScreenWidth / 4 / 2 * 2) - iszoom_x1;
			nOffsetY = nScreanHeight * 3 / 4 + ((stCapRect.s32Y + nZoomOffsetY) * nScreanHeight / CurScreenHeight / 4 / 2 * 2) - iszoom_y1;
			stCapRect.s32X += nZoomOffsetX;
			stCapRect.s32Y += nZoomOffsetY;
			
			if(m_IsPbElecZoomDrawStatus)
			{//N制式下 10 = D1，13 = CIF  P制式下 0 = D1， 3 = CIF
	        	int nCurFormat_D1 = 0;
	        	if(480 == CurScreenHeight)
	        	{
	        		nCurFormat_D1 = 10;
	        	}
				
				//csp modify
				//if(BizGetPlayBackFileFormat() == nCurFormat_D1)
				if(1)
				{
					stPbCapRect.s32X = stCapRect.s32X / 2 * 2;
					stPbCapRect.s32Y = stCapRect.s32Y;
					stPbCapRect.u32Height = stCapRect.u32Height / 2 * 2;
					stPbCapRect.u32Width = (stCapRect.s32X + stCapRect.u32Width) / 2 * 2 - stPbCapRect.s32X;
				}
				else
				{
					stPbCapRect.s32X = stCapRect.s32X * 352 / CurScreenWidth / 2 * 2;
					stPbCapRect.s32Y = stCapRect.s32Y / 2;
					stPbCapRect.u32Height = stCapRect.u32Height / 2 / 2 * 2;
					stPbCapRect.u32Width = (stCapRect.s32X + stCapRect.u32Width) * 352 / 720 / 2 * 2 - stPbCapRect.s32X;
				}
				
				PreviewPbElecZoom(0, m_CurChn, &stPbCapRect);
			}
			else
			{
                unsigned int tmpY = stCapRect.s32Y;
                unsigned int tmpH = stCapRect.u32Height;
                if((m_CurChn > 0) && (0 == Is3520))
                {
                    stCapRect.s32Y >>= 1;
                    stCapRect.u32Height >>= 1;
    			    PreviewElecZoom(0, m_CurChn, &stCapRect);
                    stCapRect.s32Y = tmpY;
                    stCapRect.u32Height = tmpH;
                }
                else
                {
                	//printf("x = %d,y = %d,w = %d,h = %d\n",stCapRect.s32X,stCapRect.s32Y,stCapRect.u32Width,stCapRect.u32Height);
                    PreviewElecZoom(0, m_CurChn, &stCapRect);
                }
			}
			
			for(int i = 0; i < 4; i++)
			{
				ShowElecZoomAreaSplitLine(i, FALSE);
			}
			
			SetElecZoomAreaSplitLineRect(0, iszoom_x1 + nOffsetX, iszoom_y1 + nOffsetY, zoom_w, EM_SPLIT_H);
			SetElecZoomAreaSplitLineRect(1, iszoom_x1 + nOffsetX, iszoom_y2 + nOffsetY, zoom_w, EM_SPLIT_H);
			SetElecZoomAreaSplitLineRect(2, iszoom_x1 + nOffsetX, iszoom_y1 + nOffsetY, zoom_h, EM_SPLIT_V);
			SetElecZoomAreaSplitLineRect(3, iszoom_x2 + nOffsetX - 2, iszoom_y1 + nOffsetY, zoom_h, EM_SPLIT_V);
			
			for(int i = 0; i < 4; i++)
			{
				ShowElecZoomAreaSplitLine(i, TRUE);
			}
			
			iszoom_x1 += nOffsetX;
			iszoom_x2 += nOffsetX;
			iszoom_y1 += nOffsetY;
			iszoom_y2 += nOffsetY;
			mouse_eleczoom_x += nOffsetX;
			mouse_eleczoom_y += nOffsetY;
		}
	}
	return 0;
}

int CPageDesktop::StartZoomVideoArea()
{
	int w = CurScreenWidth * px / nScreanWidth / 2 * 2  - CurScreenWidth * mouse_eleczoom_x / nScreanWidth / 2 * 2;
	int h = CurScreenHeight * py / nScreanHeight / 2 * 2 - CurScreenHeight * mouse_eleczoom_y / nScreanHeight / 2 * 2;

    if((w < 0) || (h < 0))
	{
		ShowElecZoomInvalidTile(0,FALSE);
		for(int i = 0; i < 4; i++)
		{
			ShowElecZoomAreaSplitLine(i,FALSE);
		}
		
		ShowElecZoomTile(0,TRUE);
		if(!m_IsPbElecZoomDrawStatus)
		{
			ShowChnName(m_CurChn, TRUE);
		}
		
		IsElecZoomBegin = 0;
		IsDrawElecZoomArea = 0;
		
		return -1;;
	}

	if((w < 16) || (h < 16))
	{
		for(int i = 0; i < 4; i++)
		{
			ShowElecZoomAreaSplitLine(i,FALSE);
		}

		if(!m_IsPbElecZoomDrawStatus)
		{
			ShowChnName(m_CurChn, TRUE);
		}
		
		SetElecZoomInvalidTileString(0, "&CfgPtn.InvalidAreaSet");
		ShowElecZoomInvalidTile(0,TRUE);
		ShowElecZoomTile(0,FALSE);

		IsElecZoomBegin = 0;
		IsDrawElecZoomArea = 0;
		
		return -1;
	}
	
	if(!m_IsPbElecZoomDrawStatus)
	{
		ShowChnName(m_CurChn, TRUE);
	}
	
	if(0 == IsElecZoomBegin)
	{
		IsElecZoomBegin = 1;
	}
	
	if(IsElecZoomBegin)
	{
		stCapRect.u32Height = h;
		stCapRect.u32Width = w;

		if((stCapRect.u32Width < 16) || (stCapRect.u32Height < 16))
		{
			for(int i = 0; i < 4; i++)
			{
				ShowElecZoomAreaSplitLine(i,FALSE);
			}

            SetElecZoomInvalidTileString(0, "&CfgPtn.InvalidAreaSet");
			ShowElecZoomInvalidTile(0,TRUE);
			ShowElecZoomTile(0,FALSE);
			
			IsElecZoomBegin = 0;
			IsDrawElecZoomArea = 0;
			
			return -1;
		}
		
		stCapRect.s32X = CurScreenWidth * mouse_eleczoom_x / nScreanWidth / 2 * 2;
		stCapRect.s32Y = CurScreenHeight * mouse_eleczoom_y / nScreanHeight / 2 * 2;
		
        if (m_IsPbElecZoomDrawStatus)
        {
        	//N制式下 10 = D1，13 = CIF  P制式下 0 = D1， 3 = CIF
        	int nCurFormat_D1 = 0;
        	if(480 == CurScreenHeight)
        	{
        		nCurFormat_D1 = 10;
        	}
			
			//csp modify
    		//if(BizGetPlayBackFileFormat() == nCurFormat_D1)
    		if(1)
    		{
    			stPbCapRect.s32X = stCapRect.s32X / 2 * 2;
    			stPbCapRect.s32Y = stCapRect.s32Y;
    			stPbCapRect.u32Height = stCapRect.u32Height / 2 * 2;
    			stPbCapRect.u32Width = (stCapRect.s32X + stCapRect.u32Width) / 2 * 2 - stPbCapRect.s32X;
    		}
    		else
    		{
    			stPbCapRect.s32X = stCapRect.s32X * 352 / CurScreenWidth / 2 * 2;
    			stPbCapRect.s32Y = stCapRect.s32Y / 2;
    			stPbCapRect.u32Height = stCapRect.u32Height / 2 / 2 * 2;
    			stPbCapRect.u32Width = (stCapRect.s32X + stCapRect.u32Width) * 352 / 720 / 2 * 2 - stPbCapRect.s32X;
    		}
        }
		
		if((m_IsPbElecZoomDrawStatus && ((stPbCapRect.u32Width < 16) || (stPbCapRect.u32Height < 16)))
            || ((0 == Is3520) && (m_CurChn > 0) && ((stCapRect.u32Width/2 < 16) || (stCapRect.u32Height/2 < 16))))
		{
			for(int i = 0; i < 4; i++)
			{
				ShowElecZoomAreaSplitLine(i,FALSE);
			}
			
            SetElecZoomInvalidTileString(0, "&CfgPtn.InvalidAreaSet");
			ShowElecZoomInvalidTile(0,TRUE);
			ShowElecZoomTile(0,FALSE);
			
			IsElecZoomBegin = 0;
			IsDrawElecZoomArea = 0;
			
			return -1;
		}
		
		IsDrawElecZoomArea = 0;
		
		ShowElecZoomInvalidTile(0,FALSE);
		ShowElecZoomTile(0,FALSE);
		
		for(int i = 0; i < 4; i++)
		{
			ShowElecZoomAreaSplitLine(i, FALSE);
		}
		
		ShowElecZoomSplitLine(0,TRUE);
		ShowElecZoomSplitLine(1,TRUE);
		
		start_x = stCapRect.s32X * nScreanWidth / CurScreenWidth;
		start_y = stCapRect.s32Y * nScreanHeight / CurScreenHeight;
		
		end_x = stCapRect.s32X + w;
		end_y = stCapRect.s32Y + h;
		
		end_x = end_x * nScreanWidth / CurScreenWidth;
		end_y = end_y * nScreanHeight / CurScreenHeight;
		
		if(m_IsPbElecZoomDrawStatus)
		{
			PreviewPbElecZoom(0, m_CurChn, &stPbCapRect);
		}
		else
		{
            unsigned int tmpY =  stCapRect.s32Y;
            unsigned int tmpH =  stCapRect.u32Height;
            if ((m_CurChn > 0) && (0 == Is3520))
            {                    
                stCapRect.s32Y >>= 1;
                stCapRect.u32Height >>= 1;
				PreviewElecZoom(0, m_CurChn, &stCapRect);
                stCapRect.s32Y = tmpY;
                stCapRect.u32Height = tmpH;
            }
            else
            {
            	PreviewElecZoom(0, m_CurChn, &stCapRect);
            }
		}
		
		iszoom_x1 = nScreanWidth * 3 / 4 + start_x / 4;
		iszoom_x2 = nScreanWidth * 3 / 4 + end_x / 4;
		
		iszoom_y1 = nScreanHeight * 3 /4 + start_y / 4;
		iszoom_y2 = nScreanHeight * 3 / 4 + end_y / 4;
		zoom_w = iszoom_x2 - iszoom_x1;
		zoom_h = iszoom_y2 - iszoom_y1;

		SetElecZoomAreaSplitLineRect(0, iszoom_x1, iszoom_y1, zoom_w, EM_SPLIT_H);
		SetElecZoomAreaSplitLineRect(1, iszoom_x1, iszoom_y2, zoom_w, EM_SPLIT_H);
		SetElecZoomAreaSplitLineRect(2, iszoom_x1, iszoom_y1, zoom_h, EM_SPLIT_V);
		SetElecZoomAreaSplitLineRect(3, iszoom_x2 - 2, iszoom_y1, zoom_h, EM_SPLIT_V);

		for(int i = 0; i < 4; i++)
		{
			ShowElecZoomAreaSplitLine(i, TRUE);
		}
		
		IsElecZoomStatue = 1;
		
		//printf("***进入电子放大预览模式*****\n");
	}
	return 0;
}

