#include <time.h>
#include "GUI/Pages/PageStartFrameWork.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/PagePlayBackFrameWork.h"
#include "GUI/Pages/PageStartFrameWork.h"
#include "GUI/Pages/PageChnSelect.h"
#include "GUI/Pages/PageAudioSelect.h"
#include "GUI/Pages/PageColorSetup.h"
#include "GUI/Pages/PagePtzCtl.h"
#include "GUI/Pages/PageDesktop.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMessageBox.h"

#include "GUI/Pages/PagePIP.h"//csp modify 20130501

#include "mod_encode.h"

enum{//与下面的数组一一对应
	START_TOOLMENU,
	
	START_TOOL1X1,
	START_TOOLTRIANGLE,
	START_TOOL2X2,
	START_TOOLTRIANGLE1,
	START_TOOL3X3,
	START_TOOLTRIANGLE2,
	START_TOOL4X4,
	START_TOOLTRIANGLE3,
	START_TOOL5X5,
	START_TOOLTRIANGLE4,
	START_TOOL6X6,
	
	START_ELECTRONICZOOM,
	START_TOOLDWELLNO,
	START_TOOLCOLOR,
	//START_TOOLZOOM,
	START_TOOLAUDIO, 
	START_TOOLPTZ,
	//START_TOOLSNAP,
	START_TOOLREC,
	START_TOOLPLAY,
	START_MAX,
};

#if 0//csp modify 20130501
char* startShortcut[START_MAX] = {
	"tool.menu",
	"tool.1x1", 
	"tool.triangle",
	"tool.2x2",
	
	// 8
	"tool.triangle",
	"tool.3x3",
	//16
	"tool.triangle",
	"tool.4x4",	
	//24
	"tool.triangle",
	"tool.5x5",
	//32
	"tool.triangle",
	"tool.6x6",
	
	"tool.electroniczoom",
	"tool.dwellno",
	"tool.color",
	//"tool.zoom",
	"tool.audio",
	"tool.ptz",
	//"tool.snap",
	"tool.rec",
	"tool.play",	
};
#endif

char* startShortcutBmpName[START_MAX][2] = {	  //add langzi 2009-11-9 右击菜单图标文件名
	{DATA_DIR"/temp/tool_menu.bmp",DATA_DIR"/temp/tool_menu_f.bmp"},
	{DATA_DIR"/temp/tool_1x1.bmp",DATA_DIR"/temp/tool_1x1_f.bmp"},	
	{DATA_DIR"/temp/tool_triangle.bmp",DATA_DIR"/temp/tool_triangle_f.bmp"},
	{DATA_DIR"/temp/tool_2x2.bmp",DATA_DIR"/temp/tool_2x2_f.bmp"},
	
	// 8 channel
	{DATA_DIR"/temp/tool_triangle.bmp",DATA_DIR"/temp/tool_triangle_f.bmp"},
	{DATA_DIR"/temp/tool_3x3.bmp",DATA_DIR"/temp/tool_3x3_f.bmp"},
	// 16 channel
	{DATA_DIR"/temp/tool_triangle.bmp",DATA_DIR"/temp/tool_triangle_f.bmp"},
	{DATA_DIR"/temp/tool_4x4.bmp",DATA_DIR"/temp/tool_4x4_f.bmp"},
	// 24 channel
	{DATA_DIR"/temp/tool_triangle.bmp",DATA_DIR"/temp/tool_triangle_f.bmp"},
	{DATA_DIR"/temp/tool_5x5.bmp",DATA_DIR"/temp/tool_5x5_f.bmp"},
	// 32 channel
	{DATA_DIR"/temp/tool_triangle.bmp",DATA_DIR"/temp/tool_triangle_f.bmp"},
	{DATA_DIR"/temp/tool_6x6.bmp",DATA_DIR"/temp/tool_6x6_f.bmp"},
	
	{DATA_DIR"/temp/tool_elezoom.bmp",DATA_DIR"/temp/tool_elezoom_f.bmp"},
	{DATA_DIR"/temp/tool_dwell_no.bmp",DATA_DIR"/temp/tool_dwell_no_f.bmp"},
	{DATA_DIR"/temp/tool_set_color.bmp",DATA_DIR"/temp/tool_set_color_f.bmp"},
	//{DATA_DIR"/temp/tool_zoom.bmp",DATA_DIR"/temp/tool_zoom_f.bmp"},
	{DATA_DIR"/temp/tool_audio_mute.bmp",DATA_DIR"/temp/tool_audio_mute_f.bmp"},
	{DATA_DIR"/temp/tool_ptz.bmp",DATA_DIR"/temp/tool_ptz_f.bmp"},
	//{DATA_DIR"/temp/tool_snap.bmp",DATA_DIR"/temp/tool_snap_f.bmp"},
	{DATA_DIR"/temp/tool_rec.bmp",DATA_DIR"/temp/tool_rec_f.bmp"},
	{DATA_DIR"/temp/tool_play.bmp",DATA_DIR"/temp/tool_play_f.bmp"},
};

static VD_BITMAP* pBmpButtonNormal[START_MAX];
static VD_BITMAP* pBmpButtonSelect[START_MAX];
static VD_BITMAP* pBmpButtonDisable[START_MAX];
static VD_BITMAP* pBmpPlay;
static VD_BITMAP* pBmpPlay_f;

static VD_BITMAP* pBmpAudio;
static VD_BITMAP* pBmpAudio_f;

//void SplitEx(CPageDesktop* pDeskTop, EMBIZPREVIEWMODE mode, u8 nModPara);

static VD_BITMAP* pBmpDwell;
static VD_BITMAP* pBmpDwell_f;
//static VD_BITMAP * pBmp_infoicon;
static VD_BITMAP * pBmp_button_normal;

static int GetMaxChnNum() { return GetVideoMainNum(); }

CPageStartFrameWork::CPageStartFrameWork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
	//:CPageFrame(pRect, psz, icon, pParent)
	:CPageFloat(pRect, pParent)
{
	//csp modify 20130501
	m_pPagePIP = NULL;
	
	nChMax = GetMaxChnNum();
	
	int nChSwitchBtn = 1;
	int nChSwitchMax = 5;
	
	nCurPage = 0;
	OutputIndex = GetOutput();
	bIsSinglePreview = FALSE;
	
	//把所有预览通道设置为不选中
	bitvec.reset();
	
	switch(nChMax)
	{
		case 4:
		{
			nChSwitchBtn = 1;
		} break;
		case 8:
		case 10:
		case 6://csp modify
		{
			nChSwitchBtn = 2;
		} break;
		case 16:
		{
			nChSwitchBtn = 3;
		} break;
		case 24:
		{
			nChSwitchBtn = 4;
		} break;
		case 32:
		{
			nChSwitchBtn = 5;
		} break;
	}
	
	m_bStartPage = TRUE;
	m_bMD = FALSE;
	m_bPatrol = FALSE;
	m_pDesktop = m_pParent;
	m_nGoDesktop = FALSE;
	memset(&m_bRecord, 0 ,sizeof(m_bRecord));
	
	SetMargin(0,0,0,0);
	
	CRect rtTmp;
	rtTmp.left = 4;
	rtTmp.top = m_Rect.Height()-40 + 4;
	
	pBmpButtonNormal[0]  = VD_LoadBitmap(startShortcutBmpName[0][0]);
	pBmpButtonSelect[0] = VD_LoadBitmap(startShortcutBmpName[0][1]);
	
	pBmpAudio = VD_LoadBitmap(DATA_DIR"/temp/tool_audio.bmp");
	pBmpAudio_f = VD_LoadBitmap(DATA_DIR"/temp/tool_audio_f.bmp");
    pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	
	BizGetIsPatrolPara(0, &psPara, 0);
	if(1 == psPara.nIsPatrol)
	{
		m_bPatrol = 1;
	}
	
	pBmpDwell = VD_LoadBitmap(DATA_DIR"/temp/tool_dwell.bmp");
	pBmpDwell_f = VD_LoadBitmap(DATA_DIR"/temp/tool_dwell_f.bmp");
	
	//printf("pBmpButtonNormal[0] = %x\n",pBmpButtonNormal[0]);
	//printf("pBmpButtonSelect[0] = %x\n",pBmpButtonSelect[0]);
	pButton[0] = CreateButton(CRect(rtTmp.left+2, rtTmp.top , 
						rtTmp.left+pBmpButtonNormal[0]->width+2, rtTmp.top+pBmpButtonNormal[0]->height), 
						this, NULL, (CTRLPROC)&CPageStartFrameWork::OnClkStartBtn, NULL, buttonNormalBmp);
	pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0]);
	
	#define BTN_PIC_WIDTH_MAX 28
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("CPageStartFrameWork GetVideoFormat\n");
	}
	
	SBizDvrInfo bizDvrInfo;
	bizTar.emBizParaType = EM_BIZ_DVRINFO;
	ret = BizGetPara(&bizTar, &bizDvrInfo);
	if(ret!=0)
	{
		BIZ_DATA_DBG("CPageStartFrameWork Get EM_BIZ_DVRINFO nOutputSync\n");
	}
	
	bizDvrInfo.nOutputSync == EM_BIZ_OUTPUT_VGAORCVBS;//why???
	//bizSysPara.nOutput = EM_BIZ_OUTPUT_CVBS;
	
	int Space = 2;
	int SpaceXOffset = 0;
	if((EM_BIZ_OUTPUT_VGAORCVBS == bizDvrInfo.nOutputSync) && (EM_BIZ_OUTPUT_CVBS == bizSysPara.nOutput))
	{
		Space = 2;
		SpaceXOffset = 0;
	}
	else if((EM_BIZ_OUTPUT_VGAORCVBS == bizDvrInfo.nOutputSync) && (EM_BIZ_OUTPUT_VGA_800X600 == bizSysPara.nOutput))
	{
		Space = 4;
		SpaceXOffset = 0;
	}
	else if((EM_BIZ_OUTPUT_VGAORCVBS == bizDvrInfo.nOutputSync) && (EM_BIZ_OUTPUT_VGA_1024X768 == bizSysPara.nOutput))
	{
		Space = 6;
		SpaceXOffset = 0;
	}
	else if((EM_BIZ_OUTPUT_VGAORCVBS == bizDvrInfo.nOutputSync) && (EM_BIZ_OUTPUT_VGA_1280X1024 == bizSysPara.nOutput))
	{
		Space = 8;
		SpaceXOffset = 0;
	}
	else if((EM_BIZ_OUTPUT_VGAORCVBS == bizDvrInfo.nOutputSync) && (EM_BIZ_OUTPUT_HDMI_1280X720 == bizSysPara.nOutput))
	{
		Space = 8;
		SpaceXOffset = 0;
	}
	else if((EM_BIZ_OUTPUT_VGAORCVBS == bizDvrInfo.nOutputSync) && (EM_BIZ_OUTPUT_HDMI_1920X1080 == bizSysPara.nOutput))
	{
		Space = 12;
		SpaceXOffset = 0;
	}
	
	u32 nTmpx = m_Rect.Width() - (2+nChSwitchBtn+7)*(BTN_PIC_WIDTH_MAX + Space) + 15;
	
	#if 1//csp modify
	//if((EM_BIZ_OUTPUT_VGAORCVBS == bizDvrInfo.nOutputSync) && (EM_BIZ_OUTPUT_CVBS == bizSysPara.nOutput))
	{
		nTmpx -= 9;
	}
	#endif
	
	CRect rtSub1(rtTmp.left+pBmpButtonNormal[0]->width+2,//左起主菜单栏按键右+2
				rtTmp.top, 
				nTmpx - 2, //右到1x1按键-2
				rtTmp.top+pBmpButtonNormal[0]->height);//zlb20111118 适应CVBS
	pInfoBar = CreateStatic(rtSub1, 
				this, 
				"");
	pInfoBar->SetTextAlign(VD_TA_CENTER);
    #if 0
    pBmp_infoicon = VD_LoadBitmap(DATA_DIR"/temp/icon_info.bmp");
	pIconInfo = CreateStatic(rtSub1, 
				this, 
				"");
	pIconInfo->SetBitmap(pBmp_infoicon,NULL);
	pIconInfo->Show(FALSE);
    #endif
	
	rtTmp.left = nTmpx;// - nChSwitchBtn*18;
	
	//csp modify 20121130
	char tmp2[20] = {0};
	GetProductNumber(tmp2);
	
	int i;
	int start_max = START_MAX;
	SBizDvrInfo DvrInfo;
	if (BizGetDvrInfo(&DvrInfo) == 0)
	{
		if (DvrInfo.nNVROrDecoder != 1) //== 2 解码器版本
		{
			start_max -= 2; //去除START_TOOLREC, START_TOOLPLAY	
		}
	}
	
	
	for(i = 1; i<start_max; i++)
	{
		//csp modify 20130501
		if(0 == strcasecmp(tmp2, "R3104HD"))
		{
			if(strcmp(startShortcutBmpName[i][0],DATA_DIR"/temp/tool_set_color.bmp") == 0)
			{
				startShortcutBmpName[i][0] = (char *)DATA_DIR"/temp/tool_pip.bmp";
				startShortcutBmpName[i][1] = (char *)DATA_DIR"/temp/tool_pip_f.bmp";
			}
		}
		
		pBmpButtonNormal[i] = VD_LoadBitmap(startShortcutBmpName[i][0]);
		pBmpButtonSelect[i] = VD_LoadBitmap(startShortcutBmpName[i][1]);
		
		if((i<=1+2*nChSwitchBtn && (i%2)!=0 || (i == 2)) || i>1+2*nChSwitchMax)
		{
			pButton[i] = CreateButton(CRect((i==2)?(rtTmp.left+=SpaceXOffset):rtTmp.left, 
							rtTmp.top, 
							rtTmp.left+pBmpButtonNormal[i]->width, rtTmp.top+pBmpButtonNormal[i]->height), 
							this, NULL, (CTRLPROC)&CPageStartFrameWork::OnClkStartBtn, NULL, buttonNormalBmp);
			
			pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
			
			//csp modify 20121130
			int last_left = rtTmp.left;
			
			rtTmp.left += Space + (pBmpButtonNormal[i]->width > BTN_PIC_WIDTH_MAX ? BTN_PIC_WIDTH_MAX : pBmpButtonNormal[i]->width);
			
			#if 1//csp modify
			//if((EM_BIZ_OUTPUT_VGAORCVBS == bizDvrInfo.nOutputSync) && (EM_BIZ_OUTPUT_CVBS == bizSysPara.nOutput))
			{
				if(i == 1 && Space < 6)
				{
					rtTmp.left += (6-Space);
				}
			}
			#endif
			
			//csp modify 20121130
			if(0 == strcasecmp(tmp2, "R3104HD"))
			{
				if(strcmp(startShortcutBmpName[i][0],DATA_DIR"/temp/tool_set_color.bmp") == 0)
				{
					pButton[i]->Enable(FALSE);
					
					rtTmp.left = last_left;
				}
			}
		}
	}
	
	for(i=0; i<nChSwitchBtn-1; i++)
	{
		CRect rtFloat1;
		rtFloat1.left = -1;
		rtFloat1.right = -1;
		rtFloat1.top = -1;
		rtFloat1.bottom = -1;
		m_pPageChnSelMul[i] = new CPageChnSelect(rtFloat1, NULL,NULL,this, 1+i);
		m_pPageChnSelMul[i]->SetDesktop(m_pDesktop);
	}
	
	m_pDevGraphics = CDevGraphics::instance(m_screen);
	m_pDevGraphics->GetRect(&m_RectScreen);
	
	//printf("CPageDesktop::CPageDesktop 115\n");
	
	m_pPageMain = new CPageMainFrameWork(NULL, "&CfgPtn.MAINMENU"/*"&titles.mainmenu"*/, NULL, NULL/*this*/);
	//m_pPagePlayBack = new CPagePlayBackFrameWork(&m_RectScreen, ""/*"&titles.mainmenu"*/, icon_dvr, NULL/*this*/);
	
	//printf("CPageDesktop::CPageDesktop 116\n");
	
	CRect rtFloat;
	
	int screenWidth = m_RectScreen.right - m_RectScreen.left;
	int pagePbWidth = 800;
	
	if((EM_BIZ_OUTPUT_VGAORCVBS == bizDvrInfo.nOutputSync) && (EM_BIZ_OUTPUT_CVBS == bizSysPara.nOutput))
	{
		int nOffsetXCvbs = 48;
		pagePbWidth = screenWidth - 2 * nOffsetXCvbs;
	}
	
	pbFloat.left = (screenWidth-pagePbWidth)/2;
	//printf("rtFloat.left  = %d \n",rtFloat.left);
	pbFloat.right = pbFloat.left + pagePbWidth;
	pbFloat.top = m_Rect.bottom - 100;
	pbFloat.bottom = pbFloat.top + 100;
	m_pPagePlayBack = new CPagePlayBackFrameWork(pbFloat, NULL,NULL,NULL/*this*/);
	m_pPagePlayBack->SetDesktop(m_pDesktop);
	SetPage(EM_PAGE_PLAYBACK,m_pPagePlayBack);
	
	rtFloat.left = m_Rect.Width()- 12*36;
	rtFloat.right = rtFloat.left + 190;
	rtFloat.top = m_Rect.bottom - 40 - 60;
	rtFloat.bottom = m_Rect.bottom - 40;
	//printf("parent of start : %x\n", m_pParent);
	m_pPageChnSel = new CPageChnSelect(rtFloat, NULL,NULL,this);
	m_pPageChnSel->SetDesktop(m_pDesktop);
	
	m_pPageRecChnSel = new CPageChnSelect(rtFloat, NULL,NULL,this,1,nChMax);
	m_pPageRecChnSel->SetDesktop(m_pDesktop);
	
	rtFloat.left = m_Rect.Width()- 8*36;
	rtFloat.right = rtFloat.left + 280;
	rtFloat.top = m_Rect.bottom - 40 - 84;
	rtFloat.bottom = m_Rect.bottom - 40;
	m_pPageAudioSel = new CPageAudioSelect(rtFloat, NULL,NULL,this);
	
	rtFloat.left = m_Rect.Width()*3/5;
	rtFloat.right = rtFloat.left + 170;
	rtFloat.top = m_Rect.bottom/7;
    rtFloat.bottom = rtFloat.top + 365;
	m_pPageColorSetup = new CPageColorSetup(rtFloat, NULL,NULL,this);
	
	rtFloat.left = (m_RectScreen.right-636)/2;
	rtFloat.right = rtFloat.left + 636;
	rtFloat.top = m_RectScreen.bottom - 212;
	rtFloat.bottom = rtFloat.top + 182;
	/*
	rtFloat.left = 1024-636;
	rtFloat.right = 1024;
	rtFloat.top = 0;
	rtFloat.bottom = rtFloat.top + 182;
	//m_pPagePtzCtl = new CPagePtzCtl(rtFloat, NULL,NULL, this);
	printf("%s rtFloat.left: %d\n", __func__, rtFloat.left);
	printf("%s rtFloat.right: %d\n", __func__, rtFloat.right);
	printf("%s rtFloat.top: %d\n", __func__, rtFloat.top);
	printf("%s rtFloat.bottom: %d\n", __func__, rtFloat.bottom);
	*/
	m_pPagePtzCtl = new CPagePtzCtl(rtFloat, " Ptz Control", NULL, this);//不能修改为"Ptz Control"
	SetPatrol(m_bPatrol);
	
	//BIZ_DATA_DBG("check here CPagePtzCtl\n");
	
	//csp modify 20130501
	if(0 == strcasecmp(tmp2, "R3104HD"))
	{
		rtFloat.left = m_Rect.Width()- 12*36;
		rtFloat.right = rtFloat.left + 150;//rtFloat.left + 190;
		rtFloat.top = m_Rect.bottom - 40 - 94;//m_Rect.bottom - 40 - 60;
		rtFloat.bottom = m_Rect.bottom - 40;
		printf("PagePIP:(%d,%d,%d,%d)\n",rtFloat.left,rtFloat.top,rtFloat.right,rtFloat.bottom);
		fflush(stdout);
		m_pPagePIP = new CPagePIP(rtFloat, NULL, NULL, this);
		printf("PagePIP over\n");
		fflush(stdout);
	}

	//yaogang modify 20151203
	m_AutoCloseTimer = new CTimer("AutoClose_Timer");
}

CPageStartFrameWork::~CPageStartFrameWork()
{
	
}

VD_PCSTR CPageStartFrameWork::GetDefualtTitleCenter()
{
	//printf("GetDefualtTitleCenter %d\n", curMainItemSel);
	return "Start Page";
}

void CPageStartFrameWork::SetMute(BOOL bMute)
{
	if(bMute)
	{
		pButton[START_TOOLAUDIO]->SetBitmap(pBmpButtonNormal[START_TOOLAUDIO], pBmpButtonSelect[START_TOOLAUDIO], pBmpButtonSelect[START_TOOLAUDIO]);
	
	}
	else
	{
		pButton[START_TOOLAUDIO]->SetBitmap(pBmpAudio,pBmpAudio_f, pBmpAudio_f);
	}
}

void CPageStartFrameWork::SetPatrol(BOOL bEnable)
{
	if(m_bPatrol)
	{
		//轮询
		SetDwellStartFlag(1);
		pButton[START_TOOLDWELLNO]->SetBitmap(pBmpDwell,pBmpDwell_f, pBmpDwell_f);

		m_pPageChnSel->SetCurDwellStatue(1);
	}
	else
	{
		//取消轮询
		SetDwellStartFlag(0);
		pButton[START_TOOLDWELLNO]->SetBitmap(pBmpButtonNormal[START_TOOLDWELLNO], 
											  pBmpButtonSelect[START_TOOLDWELLNO], 
											  pBmpButtonSelect[START_TOOLDWELLNO]);
		
		m_pPageChnSel->SetCurDwellStatue(0);
	}
}

#if 0
typedef struct
{
	u8 byChn;
	EMCHNTYPE emChnType;
} SChnTy;

void StreamDeal(SEncodeHead* psEncodeHead, u32 nContent)
{
	static int count = 0, fd = -1;
	static int fd1 = -1;
	
	if (psEncodeHead && nContent > 0)
	{
		SChnTy* psChn;
		
		psChn = (SChnTy *) nContent;
		if (0 == count)
		{
			fd = open("/dev/video.h264", O_RDWR | O_CREAT, 777);
			//fd1 = open("/root/video1.h264", O_RDWR | O_CREAT, 777);;
		}
		
		if (fd > 0)
		{
			if (count <= 500)
			{
				write(fd, psEncodeHead->pData, psEncodeHead->nLen);
				
				count++;
			}
			else
			{		
				close(fd);
				//close(fd1);
				fd = -1;
				//fd1 = -1;
			}
		}
		
		//printf("psEncodeHead->pData = %s\n", psEncodeHead->pData);
		//printf("psEncodeHead->dwLen = %d\n", psEncodeHead->nLen);
	}
}
#endif

s32 TextToOsd(PARAOUT u8* pBuffer, s32 w, s32 h, s8* strOsd, EMENCODEFONTSIZE emSize)
{
	/*
	u32 i = 0;
	for(i=0; i<w*h*2; i++)
	{
		pBuffer[i] = 0xf0;
	}	
	*/
	
	printf("TextToOsd !!!! string: %s strlen:%d\n",strOsd,strlen(strOsd));
	printf("TextToOsd !!!! w: %d, h: %d\n",w,h);
	
	CDC dc;
	dc.StringToBitmap(pBuffer,w,h,strOsd,strlen(strOsd));
}

void CPageStartFrameWork::OnClkStartBtn()
{
	static int j = 0;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	BOOL bFind = FALSE;
	int i = 0;

	//yaogang modify 20151203
	m_AutoCloseTimer->Start(this, (VD_TIMERPROC)&CPageStartFrameWork::AutoCloseTimerFxn, AutoCloseTimeoutms, 0);
	
	for(i = 0; i < START_MAX; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	u32 g_nUserId = GetGUILoginedUserID();
	u32 chn_tmp = 0;				
	
	if(bFind)
	{
		switch(i)
		{
            //记录点击预览通道按键的次数
            //static u32 nCurPage = 0;
			case START_TOOLMENU:
			{
				SetSystemLockStatus(1);//cw_lock
				
                BOOL bPageIsClosed = FALSE;
				
				#if 0//csp modify
				//csp modify:为了解决点击开始菜单后的鼠标残留问题
				VD_POINT pos;
				m_pCursor->GetCursorPos(&pos);
				m_pCursor->ShowCursor(FALSE);
				m_pCursor->SetCursorPos(0,0);
				
				bPageIsClosed = this->Close(UDM_EMPTY);
				
				//csp modify:为了解决点击开始菜单后的鼠标残留问题
				m_pCursor->SetCursorPos(pos.x,pos.y);
				m_pCursor->ShowCursor(TRUE);
				
				SetSystemLockStatus(0);
				#else
				//csp modify:为了解决点击开始菜单后的鼠标残留问题
				VD_POINT pos;
				m_pCursor->GetCursorPos(&pos);
				m_pCursor->ShowCursor(FALSE);
				m_pCursor->SetCursorPos(0,0);
				
				bPageIsClosed = this->Close();
				
				//csp modify:为了解决点击开始菜单后的鼠标残留问题
				m_pCursor->SetCursorPos(pos.x,pos.y);
				m_pCursor->ShowCursor(TRUE);
				
				if(bPageIsClosed) 
				{
					if(m_pPageMain->GetFlag(IF_OPENED))//cw_mainpage
					{
						printf("cw*****Toolmenu open error\n");
						m_pPageMain->Close();
					}
					printf("START_TOOLMENU\n");
					m_pPageMain->Open();
					printf("START_TOOLMENU 2\n");
					SetSystemLockStatus(0);
				}
				#endif
			}break;
            //单画面预览
			case START_TOOL1X1:
			{
				//printf("########curPre = %d\n",m_curPre);
			    if(!bitvec.test(1))
                {
                    nCurPage = 0;
                    bitvec.reset();
                    bitvec.set(1);
                }
				if(EM_BIZPREVIEW_1SPLIT == previewMode)
				{
					SetCurPreviewMode_CW(previewMode);//cw_preview
					SwitchPreview(previewMode, 0xff); //0xff模式不切换，通道自动切换
				}
				else
				{
                    previewMode = EM_BIZPREVIEW_1SPLIT;
					SetCurPreviewMode_CW(EM_BIZPREVIEW_1SPLIT);//cw_preview
					SwitchPreview(EM_BIZPREVIEW_1SPLIT, 0);
				}
                nCurPage %= nChMax;
                ((CPageDesktop *)(this->GetParent()))->SetCurPreviewMode(previewMode);
                ((CPageDesktop *)(this->GetParent()))->SetCurPreModePage(nCurPage++);
			}break;
			case START_TOOLTRIANGLE:
			{
				printf("START_TOOLTRIANGLE\n");
				
				CRect rect;
				pFocusButton->GetRect(&rect);
				rect.right = rect.left + (CHKBOXW+STATICW)*ROW_ITEM+(GAP-1)*(2*ROW_ITEM-1);
				rect.bottom = rect.top - GAP;
				rect.top = rect.bottom - (nChMax+ROW_ITEM-1)/ROW_ITEM*(CHKBOXW+GAP) - 40;
				
				m_pPageChnSel->SetRect(&rect, TRUE);
				m_pPageChnSel->Open();
			}break;
			case START_TOOLTRIANGLE1:
			case START_TOOLTRIANGLE2:
			case START_TOOLTRIANGLE3:
			case START_TOOLTRIANGLE4:
			{
				printf("START_TOOLTRIANGLE %d\n", i);
				
				CRect rect;
				pFocusButton->GetRect(&rect);
				rect.right = rect.left + (CHKBOXW+STATICW+GAP1)*ROW_ITEM + CHKBOXW + GAP*(ROW_ITEM*2-1);
				rect.bottom = rect.top - GAP;
				rect.top = rect.bottom - (nChMax+ROW_ITEM-1)/ROW_ITEM*(CHKBOXW+GAP) - 40;
				
				int nChnSelIdx = (i-START_TOOLTRIANGLE1)>>1;
				
				m_pPageChnSelMul[nChnSelIdx]->Open();
				m_pPageChnSelMul[nChnSelIdx]->SetRect(&rect, TRUE);				
			}break;
            //4画面预览
			case START_TOOL2X2:
			{
                //测试之前是否有点击过
                if(!bitvec.test(2))
                {
                	//printf("START_TOOL2X2-no click\n");
                    nCurPage = 0;
                    bitvec.reset();
                    bitvec.set(2);
                    SinglePreview(FALSE);
                }
				else
				{
					//printf("START_TOOL2X2-click,nCurPage=%d\n",nCurPage);
				}
                if(bIsSinglePreview)
                {
                	//printf("START_TOOL2X2-1\n");
                    BizResumePreview(0);
                    SinglePreview(FALSE);
                    ((CPageDesktop *)(this->GetParent()))->SetCurPreviewMode(EM_BIZPREVIEW_4SPLITS);
                    return;
                }
				if(EM_BIZPREVIEW_4SPLITS == previewMode)
				{
					//printf("START_TOOL2X2-2\n");
					SetCurPreviewMode_CW(previewMode);//cw_preview
					SwitchPreview(previewMode, 0xff);//0xff模式不切换，通道自动切换
				}
				else
				{
					//printf("START_TOOL2X2-3\n");
               		previewMode = EM_BIZPREVIEW_4SPLITS;
					SetCurPreviewMode_CW(EM_BIZPREVIEW_4SPLITS);//cw_preview
					SwitchPreview(EM_BIZPREVIEW_4SPLITS, 0);
				}
                nCurPage %= ((nChMax%EM_BIZPREVIEW_4SPLITS)?(nChMax/EM_BIZPREVIEW_4SPLITS+1):(nChMax/EM_BIZPREVIEW_4SPLITS));
                ((CPageDesktop *)(this->GetParent()))->SetCurPreviewMode(previewMode);
                ((CPageDesktop *)(this->GetParent()))->SetCurPreModePage(nCurPage++);
			}break;
            //9画面预览
			case START_TOOL3X3:
			{
				EMBIZPREVIEWMODE t = EM_BIZPREVIEW_9SPLITS;
				char tmp[20] = {0};
				GetProductNumber(tmp);
				if(0 == strcasecmp(tmp, "R3110HDW"))
				{
					t = EM_BIZPREVIEW_10SPLITS;
				}
				else if(0 == strcasecmp(tmp, "R3106HDW"))
				{
					t = EM_BIZPREVIEW_6SPLITS;
				}
                //测试之前是否有点击过
                if(!bitvec.test(3))
                {
                    nCurPage = 0;
                    bitvec.reset();
                    bitvec.set(3);
                    SinglePreview(FALSE);
                }
				if(bIsSinglePreview)
				{
					BizResumePreview(0);
					SinglePreview(FALSE);
					((CPageDesktop *)(this->GetParent()))->SetCurPreviewMode(t);
					return;
				}
				if(t == previewMode)
				{
					SetCurPreviewMode_CW(previewMode);//cw_preview
					SwitchPreview(previewMode, 0xff);//0xff模式不切换，通道自动切换
				}
				else
				{
                 	previewMode = t;
					SetCurPreviewMode_CW(t);//cw_preview
					SwitchPreview(t, 0);
				}
				nCurPage %= ((nChMax%t)?(nChMax/t+1):(nChMax/t));
				//printf("START_TOOL3X3:previewMode=%d,nCurPage=%d\n",previewMode,nCurPage);
				((CPageDesktop *)(this->GetParent()))->SetCurPreviewMode(previewMode);
				((CPageDesktop *)(this->GetParent()))->SetCurPreModePage(nCurPage++);
			}break;
            //16画面预览
			case START_TOOL4X4:
			{
                //测试之前是否有点击过
                if(!bitvec.test(4))
                {
                    nCurPage = 0;
                    bitvec.reset();
                    bitvec.set(4);
                    SinglePreview(FALSE);
                }
                if(bIsSinglePreview)
                {
                    BizResumePreview(0);
                    SinglePreview(FALSE);
                    ((CPageDesktop *)(this->GetParent()))->SetCurPreviewMode(EM_BIZPREVIEW_16SPLITS);
                    return;
                }
				if(EM_BIZPREVIEW_16SPLITS == previewMode)
				{
					SetCurPreviewMode_CW(previewMode);//cw_preview
					SwitchPreview(previewMode, 0xff);//0xff模式不切换，通道自动切换
				}
				else
				{
                    previewMode = EM_BIZPREVIEW_16SPLITS;
					SetCurPreviewMode_CW(EM_BIZPREVIEW_16SPLITS);//cw_preview
					SwitchPreview(EM_BIZPREVIEW_16SPLITS, 0);
				}
                nCurPage %= ((nChMax%EM_BIZPREVIEW_16SPLITS)?(nChMax/EM_BIZPREVIEW_16SPLITS+1):(nChMax/EM_BIZPREVIEW_16SPLITS));
                ((CPageDesktop *)(this->GetParent()))->SetCurPreviewMode(previewMode);
                ((CPageDesktop *)(this->GetParent()))->SetCurPreModePage(nCurPage++);
			}break;
            //25画面预览
			case START_TOOL5X5:
			{
                //测试之前是否有点击过
                if(!bitvec.test(5))
                {
                    nCurPage = 0;
                    bitvec.reset();
                    bitvec.set(5);
                    SinglePreview(FALSE);
                }
                if(bIsSinglePreview)
                {
                    BizResumePreview(0);
                    SinglePreview(FALSE);
                    ((CPageDesktop *)(this->GetParent()))->SetCurPreviewMode(EM_BIZPREVIEW_25SPLITS);
                    return;
                }
				if(EM_BIZPREVIEW_25SPLITS == previewMode)
				{
					SetCurPreviewMode_CW(previewMode);//cw_preview
					SwitchPreview(previewMode, 0xff);//0xff模式不切换，通道自动切换
				}
				else
				{
                    previewMode = EM_BIZPREVIEW_25SPLITS;
					SetCurPreviewMode_CW(EM_BIZPREVIEW_25SPLITS);//cw_preview
					SwitchPreview(EM_BIZPREVIEW_25SPLITS, 0);
				}
                nCurPage %= ((nChMax%EM_BIZPREVIEW_25SPLITS)?(nChMax/EM_BIZPREVIEW_25SPLITS+1):(nChMax/EM_BIZPREVIEW_25SPLITS));
                ((CPageDesktop *)(this->GetParent()))->SetCurPreviewMode(previewMode);
                ((CPageDesktop *)(this->GetParent()))->SetCurPreModePage(nCurPage++);
			}break;
            //36画面预览 
			case START_TOOL6X6:
			{
                //测试之前是否有点击过
                if(!bitvec.test(6))
                {
                    nCurPage = 0;
                    bitvec.reset();
                    bitvec.set(6);
                    SinglePreview(FALSE);
                }
                if(bIsSinglePreview)
                {
                    BizResumePreview(0);
                    SinglePreview(FALSE);
                    ((CPageDesktop *)(this->GetParent()))->SetCurPreviewMode(EM_BIZPREVIEW_36SPLITS);
                    return;
                }
				if(EM_BIZPREVIEW_36SPLITS == previewMode)
				{
					SetCurPreviewMode_CW(previewMode);//cw_preview
					SwitchPreview(previewMode, 0xff);//0xff模式不切换，通道自动切换
				}
				else
				{
                    previewMode = EM_BIZPREVIEW_36SPLITS;
					SetCurPreviewMode_CW(EM_BIZPREVIEW_36SPLITS);//cw_preview
					SwitchPreview(EM_BIZPREVIEW_36SPLITS, 0);
				}
                nCurPage %= ((nChMax%EM_BIZPREVIEW_36SPLITS)?(nChMax/EM_BIZPREVIEW_36SPLITS+1):(nChMax/EM_BIZPREVIEW_36SPLITS));
                ((CPageDesktop *)(this->GetParent()))->SetCurPreviewMode(previewMode);
                ((CPageDesktop *)(this->GetParent()))->SetCurPreModePage(nCurPage++);
			}break;
			case START_ELECTRONICZOOM:
			{
				//电子放大
				printf("START_ELECTRONICZOOM\n");
				
				CRect rect;
				pFocusButton->GetRect(&rect);
				rect.right = rect.left + (CHKBOXW+STATICW)*ROW_ITEM+(GAP-1)*(2*ROW_ITEM-1);
				rect.bottom = rect.top - GAP;
				rect.top = rect.bottom - (nChMax+ROW_ITEM-1)/ROW_ITEM*(CHKBOXW+GAP) - 40;
				
				m_pPageChnSel->SetCurStatusElecZoom(1);
				m_pPageChnSel->SetRect(&rect, TRUE);
				m_pPageChnSel->Open();
			}break;
			case START_TOOLDWELLNO:
			{
				printf("START_TOOLDWELLNO\n");
				
				SetCurPreviewMode_CW(EM_BIZPREVIEW_1SPLIT);//cw_preview
				//g_curPreviewMode = mode;
				m_bPatrol = !m_bPatrol;
				ControlPatrol(m_bPatrol);
				
				psPara.nIsPatrol = m_bPatrol;
				BizSetIsPatrolPara(&psPara, 0);
				if(m_bPatrol)
				{
					//轮询
					SetDwellStartFlag(1);
					pButton[START_TOOLDWELLNO]->SetBitmap(pBmpDwell,pBmpDwell_f, pBmpDwell_f);

					BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_START_DWELL);
					m_pPageChnSel->SetCurDwellStatue(1);
				}
				else
				{
					//取消轮询
					SetDwellStartFlag(0);
					pButton[START_TOOLDWELLNO]->SetBitmap(pBmpButtonNormal[START_TOOLDWELLNO], 
														  pBmpButtonSelect[START_TOOLDWELLNO], 
														  pBmpButtonSelect[START_TOOLDWELLNO]);

					BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_STOP_DWELL);
					m_pPageChnSel->SetCurDwellStatue(0);
				}
			}break;
			case START_TOOLCOLOR:
			{
				char tmp2[20] = {0};
				GetProductNumber(tmp2);
				if(0 == strcasecmp(tmp2, "R3104HD"))//csp modify 20130501
				{
					printf("START_TOOLPIP\n");
					
					//SetSystemLockStatus(1);
					
					//this->Close();
					
					CRect rect;
					pFocusButton->GetRect(&rect);
					
					CRect rect2;
					m_pPagePIP->GetRect(&rect2);
					
					rect.right = rect.left + rect2.Width();
					rect.bottom = rect.top - GAP;
					rect.top = rect.bottom - rect2.Height();
					m_pPagePIP->SetRect(&rect, TRUE);
					
					m_pPagePIP->Open();
					
					//SetSystemLockStatus(0);
				}
				else
				{
					printf("START_TOOLCOLOR\n");
					
					SetSystemLockStatus(1);//cw_lock
					
					this->Close();
					m_pPageColorSetup->SetStartPageFlag(1);
					m_pPageColorSetup->Open();
					
					SetSystemLockStatus(0);//cw_lock
					
					SetCurPreviewMode_CW(EM_BIZPREVIEW_1SPLIT);//cw_preview
					SwitchPreview(EM_BIZPREVIEW_1SPLIT, 0);
				}
			}break;
#if 0
		case START_TOOLZOOM:
			{
				printf("START_TOOLZOOM\n");
				
				u32 nChn = 1;
				SChnTy sChn;
				EMCHNTYPE emChnType = EM_CHN_VIDEO_MAIN;
				ModEncodeRequestStart(nChn, emChnType, StreamDeal, (u32)&sChn);
				ModEncodeRequestStop(nChn, emChnType);
				ModEncodeRequestStop(1,emChnType);
			}break;
#endif
			case START_TOOLAUDIO:
			{
				printf("START_TOOLAUDIO\n");
				
				u8 chn = 0, vol = 0, mute = 0;
				if(0 == GetAudioOutInfo(&chn, &vol, &mute))
				{
					m_pPageAudioSel->SetAudioInfo(chn, vol, mute);
				}
				m_pPageAudioSel->Open();
			}break;
			case START_TOOLPTZ:
			{
				//printf("START_TOOLPTZ\n");
				
				EMBIZUSERCHECKAUTHOR author = BizUserCheckAuthority(EM_BIZ_USER_AUTHOR_PTZCTRL,&chn_tmp,g_nUserId);
				if(author == EM_BIZ_USER_AUTHOR_NO)
				{
					MessageBox("&CfgPtn.NoAuthority", "&CfgPtn.WARNING"  , MB_OK|MB_ICONWARNING);
				}
				else //if(author == EM_BIZ_USER_AUTHOR_YES)
				{
					SetSystemLockStatus(1);//cw_lock
					this->Close();
					//printf("m_pPagePtzCtl = %d \n",m_pPagePtzCtl);
					m_pPagePtzCtl->Open();
					SetSystemLockStatus(0);
					BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_CONTROL_PTZ);
				}
			}break;
#if 0
			case START_TOOLSNAP:
			{
				printf("START_TOOLSNAP\n");
				
				#if 1
				m_bMD = !m_bMD;

				for(int i=0; i<4; i++)
				{
					SBizAlarmStatus status;
					status.nChn = i;
					status.nCurStatus = m_bMD;
					EventMDDeal(&status);
				}
				#endif
			}break;
#endif
			case START_TOOLREC:
			{
				//printf("START_TOOLREC\n");
				
                u8 bDiskIsExist = 0;
                
                //检测是否有硬盘,无则弹出提示框
                bDiskIsExist = GetHardDiskNum();
                if (!bDiskIsExist) {
                    MessageBox("&CfgPtn.DiskIsNotExist", "&CfgPtn.WARNING" ,
                    MB_OK|MB_ICONWARNING);
                    return;
                }

				EMBIZUSERCHECKAUTHOR author = BizUserCheckAuthority(EM_BIZ_USER_AUTHOR_RECORD,&chn_tmp,g_nUserId);
				if(author == EM_BIZ_USER_AUTHOR_NO)
				{
					MessageBox("&CfgPtn.NoAuthority", "&CfgPtn.WARNING"  , MB_OK|MB_ICONWARNING);
				}
				else //if(author == EM_BIZ_USER_AUTHOR_YES)
				{					
					CRect rect;
					pFocusButton->GetRect(&rect);
					rect.left = rect.left - (CHKBOXW+STATICW)*3 - 25;
					rect.right = rect.left + (CHKBOXW+STATICW)*ROW_ITEM+(GAP-1)*(2*ROW_ITEM-1) + 25;
					rect.bottom = rect.top - GAP;
					rect.top = rect.bottom - (nChMax+ROW_ITEM-1)/ROW_ITEM*(CHKBOXW+GAP) - 40;
					
					m_pPageRecChnSel->SetCurStatusRecorDing(1);
					m_pPageRecChnSel->SetRect(&rect, TRUE);
					m_pPageRecChnSel->Open();
				}
			}break;
			case START_TOOLPLAY:
			{
				//printf("START_TOOLPLAY\n");
				
                u8 bDiskIsExist = 0;
                //检测是否有硬盘,无则弹出提示框
                bDiskIsExist = GetHardDiskNum();
                if (!bDiskIsExist) {
                    MessageBox("&CfgPtn.DiskIsNotExist", "&CfgPtn.WARNING" ,
                    MB_OK|MB_ICONWARNING);
                    return;
                }
				
				EMBIZUSERCHECKAUTHOR author = BizUserCheckAuthority(EM_BIZ_USER_AUTHOR_PLAYBACK,&chn_tmp,g_nUserId);
				if(author == EM_BIZ_USER_AUTHOR_NO)
				{
					MessageBox("&CfgPtn.NoAuthority", "&CfgPtn.WARNING"  , MB_OK|MB_ICONWARNING);
				}
				else//if(author == EM_BIZ_USER_AUTHOR_YES)
				{
					CPage** page = GetPage();//cw_rec
					((CPageDesktop*)page[EM_PAGE_DESKTOP])->SetModePlaying();
					SetSystemLockStatus(1);//cw_shutdown
					
					SBizSearchPara sBizSearchParam;
					memset(&sBizSearchParam,0,sizeof(sBizSearchParam));
					
					time_t tTmp;
					struct tm tmstrTmp;
					struct tm* pTime;
					
					memset(&tmstrTmp,0,sizeof(struct tm));
					tTmp = time(NULL);
					
					//csp modify 20140406
					//csp modify 20131213
					int nTimeZone = GetTimeZone();
					tTmp += GetTimeZoneOffset(nTimeZone);
					
					//csp modify
					//pTime = localtime(&tTmp);
					//tmstrTmp = *pTime;
					pTime = &tmstrTmp;
					localtime_r(&tTmp, pTime);
					
					tmstrTmp.tm_hour = 0;
					tmstrTmp.tm_min = 0;
					tmstrTmp.tm_sec = 0;
					sBizSearchParam.nStartTime = mktime(&tmstrTmp);
					
					tmstrTmp.tm_hour = 23;
					tmstrTmp.tm_min = 59;
					tmstrTmp.tm_sec = 59;
					sBizSearchParam.nEndTime = mktime(&tmstrTmp);
					
					//csp modify 20131213
					sBizSearchParam.nStartTime -= GetTimeZoneOffset(nTimeZone);
					sBizSearchParam.nEndTime -= GetTimeZoneOffset(nTimeZone);
					
					sBizSearchParam.nMaskChn = 0x0000000f;//1101
					
					//csp modify
					#if 0
					if(GetMaxChnNum() == 8)
					{
						sBizSearchParam.nMaskChn = 0xff;
						m_pPagePlayBack->SetPlayChnNum(8);
					}
					else if(GetMaxChnNum() == 16)
					{
						sBizSearchParam.nMaskChn = 0xffff;
						m_pPagePlayBack->SetPlayChnNum(16);
					}
					else
					#endif
					{
						sBizSearchParam.nMaskChn = 0x0f;
						m_pPagePlayBack->SetPlayChnNum(4);
					}
					
					sBizSearchParam.nMaskType = 0xff;
					
					//m_pPagePlayBack->SetPlayChnNum(4);//cw_test 4//csp modify
					
					#if 0//csp modify
					m_pPagePlayBack->SetPlayChnNum(1);
					sBizSearchParam.nMaskChn = 0x00000001;
					#endif
					
					m_pPagePlayBack->SetPbInfo(sBizSearchParam);
					m_pPagePlayBack->SetPreviewMode(previewMode);
					
					m_pCursor->SetCursor(CR_WAIT);//csp modify
					
					this->Close();
					
					m_pPagePlayBack->SetRect(&pbFloat,TRUE);
					m_pPagePlayBack->Open();
					
					BizStopPreview();
					
					BizStartPlayback(EM_BIZPLAY_TYPE_TIME,&sBizSearchParam);
					SetSystemLockStatus(0);//cw_shutdown
					
					m_pCursor->SetCursor(CR_NORMAL);//csp modify
				}
			}
			break;
			default:
			break;
		}
		
        ((CPageDesktop *)(this->GetParent()))->SetPageFlag(FALSE);		
	}
}

void CPageStartFrameWork::SetRecordingStatue(u8 nChn, u8 flag)
{
	m_bRecord[nChn] = flag;
}

void CPageStartFrameWork::DealManualRec()
{
	SBizParaTarget bizTar;
	SBizRecPara bizRecPara;
	bizTar.emBizParaType = EM_BIZ_RECPARA;
	
	int chnnum = GetVideoMainNum();
	int log_rec_start = 0;
	int log_rec_stop = 0;

	for(int i = 0; i < chnnum; i++)
	{
		bizTar.nChn = i;

		if(0 != BizGetPara(&bizTar, &bizRecPara))
		{	
			printf("Get rec para error \n");
		}

		if(bizRecPara.bRecording != m_bRecord[i])
		{
			bizRecPara.bRecording = m_bRecord[i];
			if(0 != BizSetPara(&bizTar, &bizRecPara))
			{	
				printf("Set rec para error \n");
			}
			
			if(m_bRecord[i])
			{
				BizStartManualRec(i);
				log_rec_start = 1;
			}
			else
			{
				BizStopManualRec(i);
				log_rec_stop = 1;
			}
		}
		else
		{
			continue;
		}
	}

	if(log_rec_start)
	{
		BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_START_MANUAL_REC);
	}

	if(log_rec_stop)
	{
		BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_STOP_MANUAL_REC);
	}
}

void CPageStartFrameWork::SetCurPreview(int curPre)
{
	m_curPre = curPre;
}

int CPageStartFrameWork::GetCurPreview()
{
	return m_curPre;
}

void CPageStartFrameWork::SetCurPreviewMode(EMBIZPREVIEWMODE emMode)
{
	previewMode = emMode;
}

EMBIZPREVIEWMODE CPageStartFrameWork::GetCurPreviewMode()
{
	return previewMode;
}

void  CPageStartFrameWork::SetDesktop(CPage* pDesktop)
{
	m_pDesktop = pDesktop;
}

//void CPageStartFrameWork::EventPreviewFreshSplit( EMBIZPREVIEWMODE mode, u8 nModPara)
//
void CPageStartFrameWork::SplitEx(CPageDesktop* pDeskTop, EMBIZPREVIEWMODE mode, u8 nModPara)
{
	//CPageDesktop* pDeskTop = (CPageDesktop*)m_pDesktop;

	switch(mode)
	{
		case EM_BIZPREVIEW_1SPLIT:
		{
			pDeskTop->ShowSplitLine(0,FALSE);
			pDeskTop->ShowSplitLine(1,FALSE);

			for(int j=0; j<4; j++)
			{
				pDeskTop->ShowChnName(j,FALSE);
			}

			pDeskTop->ShowChnName(nModPara,TRUE);			
		}
		break;
		case EM_BIZPREVIEW_4SPLITS:
		{
			pDeskTop->ShowSplitLine(0,TRUE);
			pDeskTop->ShowSplitLine(1,TRUE);
			
			for(int i=0; i<4; i++)
			{
				pDeskTop->ShowChnName(i,TRUE);
			}
			
			for(int i=0; i<4; i++)
			{
				pDeskTop->ShowChnName(i+4,FALSE);
			}
		}
		break;
		default:
			break;
	}
}

VD_BOOL CPageStartFrameWork::UpdateData(UDM mode)
{
	static u8 flag = 1;

	//yaogang modify 20151203
	if(UDM_CLOSED == mode)
	{
		m_AutoCloseTimer->Stop();
	}
	else
	{
		m_AutoCloseTimer->Start(this, (VD_TIMERPROC)&CPageStartFrameWork::AutoCloseTimerFxn, AutoCloseTimeoutms, 0);
	}
	//printf("CPageStartFrameWork::UpdateData mode=%d\n",mode);
	
	if(UDM_OPEN == mode)
	{
		//printf("CPageStartFrameWork::UpdateData mode=UDM_OPEN\n");
		
		//csp modify 20131115
		if(GetCurPreviewMode_CW() >= nChMax)
		{
			nCurPage = 0;
			bitvec.reset();
		}
		
		SBizParaTarget bizTar;
		SBizRecPara bizRecPara;
		bizTar.emBizParaType = EM_BIZ_RECPARA;
		for(int i = 0; i < nChMax; i++)
		{
			bizTar.nChn = i;
			
			if(0 != BizGetPara(&bizTar, &bizRecPara))
			{
				printf("Get rec para error\n");
			}
			m_bRecord[i] = bizRecPara.bRecording;
		}
		
		if(flag)
		{
			u8 chn = 0, vol = 0, mute = 0;
			if(0 == GetAudioOutInfo(&chn, &vol, &mute))
			{
				flag = 0;
				SetMute((BOOL)mute);
				m_pPageAudioSel->SetAudioInfo(chn, vol, mute);
			}
		}
	}
	else if(UDM_CLOSED == mode)
	{
		//printf("CPageStartFrameWork::UpdateData mode=UDM_CLOSED\n");
		
		if(1)//(m_nGoDesktop)
		{
		#if 1//csp modify
			//printf("m_nGoDesktop==TRUE\n");
			//SplitEx((CPageDesktop*)m_pDesktop, previewMode, m_curPre);
			ClearInfo();
			for(int i=0; i<GetMaxChnNum(); i++)
			{
				char nameTmp[50] = {0};
				if(strlen((((CPageDesktop*)m_pDesktop)->m_vChannelName[i])->GetText()))
				{
					strcpy(nameTmp, (((CPageDesktop*)m_pDesktop)->m_vChannelName[i])->GetText());
				}
				else
				{
					sprintf(nameTmp, "%s", " ");
				}
				(((CPageDesktop*)m_pDesktop)->m_vChannelName[i])->SetText(nameTmp/*(((CPageDesktop*)m_pDesktop)->m_vChannelName[i])->GetText()*/);
			}
			//((CItem *)(m_pDesktop))->Draw();
			m_nGoDesktop = FALSE;
		#endif
		}
	}
	
	return TRUE;
}
//yaogang modify 20151203
void CPageStartFrameWork::AutoCloseTimerFxn(uint param)
{
	this->Close();
}

VD_BOOL CPageStartFrameWork::MsgProc( uint msg, uint wpa, uint lpa )
{
	//printf("CPageFrame::MsgProc\n");
	u8 lock_flag = 0;//cw_lock
	u8 startTimer = 0;
	GetSystemLockStatus(&lock_flag);
	if(lock_flag)
	{
		return FALSE;
	}
	int px,py;
	int temp;
	switch(msg)
	{
	case XM_KEYDOWN://cw_panel
		switch(wpa)
		{
			case KEY_RIGHT:
			case KEY_LEFT:
			{
				CPageFloat::MsgProc(msg, wpa,lpa);
				{
	    			static CItem* last = NULL;
					
					CItem* temp = GetFocusItem();//cw_panel
	                if(temp != last)
	                {
	    				if(temp == pButton[START_TOOLMENU])
	    				{
	    					SetInfo("&CfgPtn.MAINMENU");
	    				}
	    				else if(temp == pButton[START_TOOL1X1])
	    				{
	    					SetInfo("&CfgPtn.SingleScreenPreview");
	    				}
	    				else if(temp == pButton[START_TOOLTRIANGLE])
	    				{
	    					SetInfo("&CfgPtn.ChooseSingleScreenChannel");
	    				}
	    				else if(temp == pButton[START_TOOL2X2])
	    				{
	    					SetInfo("&CfgPtn.2x2PreviewMode");
	    				}
	                    else if (temp == pButton[START_TOOL3X3] && GetMaxChnNum()>=8)
	                    {
	                        SetInfo("&CfgPtn.3x3PreviewMode");
	                    }
	                    else if (temp == pButton[START_TOOL4X4] && GetMaxChnNum()>=16)
	                    {
	                        SetInfo("&CfgPtn.4x4PreviewMode");
	                    }
	                    else if (temp == pButton[START_TOOL5X5] && GetMaxChnNum()>=24)
	                    {
	                        SetInfo("&CfgPtn.5x5PreviewMode");
	                    }
	                    else if (temp == pButton[START_TOOL6X6] && GetMaxChnNum()>=32)
	                    {
	                        SetInfo("&CfgPtn.6x6PreviewMode");
	                    }
	                    else if (temp == pButton[START_ELECTRONICZOOM])
	                    {
	                        SetInfo("&CfgPtn.ElectronicZoomFunction");
	                    }
	                    else if (temp == pButton[START_TOOLDWELLNO])
	                    {
	                        SetInfo("&CfgPtn.PreviewPoll");
	                    }
	                    else if (temp == pButton[START_TOOLCOLOR])
	                    {
							char tmp2[20] = {0};
							GetProductNumber(tmp2);
							if(0 == strcasecmp(tmp2, "R3104HD"))//csp modify 20130501
							{
								char *pNote = (char*)GetParsedString("&CfgPtn.PIPMode");
								if(strcmp(pNote,"CfgPtn.PIPMode") == 0)
								{
									pNote = "PIP";
								}
								SetInfo(pNote);
							}
							else
							{
								SetInfo("&CfgPtn.ImageSettings");
							}
	                    }
	                    else if (temp == pButton[START_TOOLAUDIO])
	                    {
	                        SetInfo("&CfgPtn.SoundSettings");
	                    }
	                    else if (temp == pButton[START_TOOLPTZ])
	                    {
	                        SetInfo("&CfgPtn.PTZControl");
	                    }
	                    else if (temp == pButton[START_TOOLREC])
	                    {
	                        SetInfo("&CfgPtn.VideoRecording");
	                    }
	                    else if (temp == pButton[START_TOOLPLAY])
	                    {
	                        SetInfo("&CfgPtn.VideoPlayback");
	                    }
	    				else
	    				{
	    					ClearInfo();
	    				}
						
	    				last = temp;
	                }
	            }
				return TRUE;
			}
			default:
				break;
		}
		break;//csp modify 20131115
	case XM_MOUSEMOVE:
		{
			static CItem* last = NULL;
			startTimer = 1;
			
			CItem* temp = GetFocusItem();//cw_panel
		    if(temp != last)
		    {
				if(temp == pButton[START_TOOLMENU])
				{
					SetInfo("&CfgPtn.MAINMENU");
				}
				else if(temp == pButton[START_TOOL1X1])
				{
					SetInfo("&CfgPtn.SingleScreenPreview");
				}
				else if(temp == pButton[START_TOOLTRIANGLE])
				{
					SetInfo("&CfgPtn.ChooseSingleScreenChannel");
				}
				else if(temp == pButton[START_TOOL2X2])
				{
					SetInfo("&CfgPtn.2x2PreviewMode");
				}
		        else if (temp == pButton[START_TOOL3X3] && GetMaxChnNum()>=8)
		        {
		            SetInfo("&CfgPtn.3x3PreviewMode");
		        }
		        else if (temp == pButton[START_TOOL4X4] && GetMaxChnNum()>=16)
		        {
		            SetInfo("&CfgPtn.4x4PreviewMode");
		        }
		        else if (temp == pButton[START_TOOL5X5] && GetMaxChnNum()>=24)
		        {
		            SetInfo("&CfgPtn.5x5PreviewMode");
		        }
		        else if (temp == pButton[START_TOOL6X6] && GetMaxChnNum()>=32)
		        {
		            SetInfo("&CfgPtn.6x6PreviewMode");
		        }
		        else if (temp == pButton[START_ELECTRONICZOOM])
		        {
		            SetInfo("&CfgPtn.ElectronicZoomFunction");
		        }
		        else if (temp == pButton[START_TOOLDWELLNO])
		        {
		            SetInfo("&CfgPtn.PreviewPoll");
		        }
		        else if (temp == pButton[START_TOOLCOLOR])
		        {
					char tmp2[20] = {0};
					GetProductNumber(tmp2);
					if(0 == strcasecmp(tmp2, "R3104HD"))//csp modify 20130501
					{
						char *pNote = (char*)GetParsedString("&CfgPtn.PIPMode");
						if(strcmp(pNote,"CfgPtn.PIPMode") == 0)
						{
							pNote = "PIP";
						}
						SetInfo(pNote);
					}
					else
					{
						SetInfo("&CfgPtn.ImageSettings");
					}
		        }
		        else if (temp == pButton[START_TOOLAUDIO])
		        {
		            SetInfo("&CfgPtn.SoundSettings");
		        }
		        else if (temp == pButton[START_TOOLPTZ])
		        {
		            SetInfo("&CfgPtn.PTZControl");
		        }
		        else if (temp == pButton[START_TOOLREC])
		        {
		            SetInfo("&CfgPtn.VideoRecording");
		        }
		        else if (temp == pButton[START_TOOLPLAY])
		        {
		            SetInfo("&CfgPtn.VideoPlayback");
		        }
				else
				{
					ClearInfo();
				}
				
				last = temp;
		    }
		}
		break;
    case XM_RBUTTONDOWN:
    case XM_RBUTTONDBLCLK:
		{
			m_nGoDesktop = TRUE;
			startTimer = 1;
		}
		break;
    case XM_LBUTTONDBLCLK:
        {
			startTimer = 1;
            //防止双击时会进入点击下的单通道画面
            msg = XM_LBUTTONDOWN;
			
			//csp modify 20131115
			if(m_pParent)
			{
				//printf("m_pParent--1=0x%08x\n",m_pParent);
				CPageFloat::MsgProc(XM_RBUTTONDOWN, wpa, lpa);
				//this->Close();
				//printf("m_pParent--2=0x%08x\n",m_pParent);
				m_pParent->VD_SendMessage(XM_LBUTTONDBLCLK, wpa, lpa);
				//printf("m_pParent--3=0x%08x\n",m_pParent);
				return TRUE;
			}
        }
		break;//csp modify 20131115
    default:
    	break;
	}

	if (startTimer)
	{
		//yaogang modify 20151203
		m_AutoCloseTimer->Start(this, (VD_TIMERPROC)&CPageStartFrameWork::AutoCloseTimerFxn, AutoCloseTimeoutms, 0);
	}
	
	return CPageFloat::MsgProc(msg, wpa, lpa);
}

void CPageStartFrameWork::SetInfo(char * szInfo)
{
    //pIconInfo->Show(TRUE);
	
	pInfoBar->SetText(szInfo);
}

void CPageStartFrameWork::ClearInfo()
{
    //pIconInfo->Show(FALSE);
	
	pInfoBar->SetText("");
}

//csp modify 20130501
void CPageStartFrameWork::SetPIPPreviewMode(int nModePara)
{
	nCurPage = 0;
	bitvec.reset();
	SinglePreview(FALSE);
	
	SetCurPreviewMode(EM_BIZPREVIEW_PIP);
	//SetCurPreview(nModePara);
	
	SetCurPreviewMode_CW(EM_BIZPREVIEW_PIP);
	SwitchPreview(EM_BIZPREVIEW_PIP, nModePara);
	
	((CPageDesktop *)(this->GetParent()))->SetCurPreviewMode(EM_BIZPREVIEW_PIP);
	((CPageDesktop *)(this->GetParent()))->SetCurPreModePage(0);
}

void CPageStartFrameWork::SinglePreview(BOOL bIsSigPre)
{
    bIsSinglePreview = bIsSigPre;
}

