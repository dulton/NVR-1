#include "GUI/Pages/PageDataBackup.h"
#include "GUI/Pages/PageMessageBox.h"
#include "biz.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageBackupInfo.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/PageSnapPlayBack.h"
#include "GUI/Pages/PageDesktop.h"//cw_panel



#define RIGHT_PAGE_WIDTH	(180+24)

#define LEFT_PAGE_L	10
#define LEFT_PAGE_R	(m_Rect.Width()-RIGHT_PAGE_WIDTH-18)
#define RIGHT_PAGE_L (LEFT_PAGE_R+4)
#define RIGHT_PAGE_R (RIGHT_PAGE_L+RIGHT_PAGE_WIDTH)
#define FRAME_TOP	20

#define PAGE_TOP_HEIGHT  42 //yzw

static char* szBtBmpPath[4][2] = {
	{DATA_DIR"/temp/listex_btn_leftmost.bmp", DATA_DIR"/temp/listex_btn_leftmost_f.bmp"},
	{DATA_DIR"/temp/listex_btn_left.bmp", DATA_DIR"/temp/listex_btn_left_f.bmp"},
	{DATA_DIR"/temp/listex_btn_right.bmp", DATA_DIR"/temp/listex_right_f.bmp"},
	{DATA_DIR"/temp/listex_btn_rightmost.bmp", DATA_DIR"/temp/listex_btn_rightmost_f.bmp"},
};
static VD_BITMAP* pBmpBtPageCtrl[4][2];

static VD_BITMAP * pBmp_tab_normal;
static VD_BITMAP * pBmp_tab_push;
static VD_BITMAP * pBmp_tab_select;

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static	std::vector<CItem*>	items[BACKUP_SUBPAGES];
static	std::vector<CItem*>	itemsRight[BACKUP_SUBPAGES];


#define BUTTONWIDTH 80

static int GetMaxChnNum() { return GetVideoMainNum(); }

CPageDataBackup::CPageDataBackup( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
{
	//printf("%s yg 1\n", __func__);
	nChMax = GetMaxChnNum();

	//
	pBmp_tab_normal = VD_LoadBitmap(DATA_DIR"/temp/tab.bmp");
	pBmp_tab_push = VD_LoadBitmap(DATA_DIR"/temp/tab_check.bmp");
	pBmp_tab_select = VD_LoadBitmap(DATA_DIR"/temp/tab_f.bmp");
	//左右两个蓝色框
	pTabFrame1 = CreateTableBox(CRect(LEFT_PAGE_L, 10+pBmp_tab_normal->height, LEFT_PAGE_R, m_Rect.Height()-90),this,1,1);
	pTabFrame1->SetFrameColor(VD_RGB(56,108,148));
	
	pTabFrame2 = CreateTableBox(CRect(RIGHT_PAGE_L, 10+pBmp_tab_normal->height, RIGHT_PAGE_R, m_Rect.Height()-90),this,1,1);
	pTabFrame2->SetFrameColor(VD_RGB(56,108,148));
	
	
	char* szSubPgName[BACKUP_SUBPAGES] = {
		"&CfgPtn.Rec", //录像
		"&CfgPtn.Image", //图片
	};
	
	int szSubPgNameLen[BACKUP_SUBPAGES] = {
		TEXT_WIDTH*3,//"&CfgPtn.Rec",
		TEXT_WIDTH*3,//"&CfgPtn.Image",
	};

	CRect rtSub(25, 10, m_Rect.Width()-20, 10+pBmp_tab_normal->height);
	int i;
	for(i = 0; i<BACKUP_SUBPAGES; i++)
	{
		if (i==1)
		{
			#ifdef NO_MODULE_SG
				pButton[i] = NULL;
				continue;
			#endif
		}
		
		int btWidth = szSubPgNameLen[i]+10;
		rtSub.right = rtSub.left + btWidth;
		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageDataBackup::OnClickSubPage, NULL, buttonNormalBmp, 1);
		pButton[i]->SetBitmap(pBmp_tab_normal, pBmp_tab_select, pBmp_tab_push, pBmp_tab_push);
		rtSub.left = rtSub.right;
	}
	//
	//printf("%s yg 2\n", __func__);
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	
	m_pDevGraphics = CDevGraphics::instance(m_screen);
	m_pDevGraphics->GetRect(&m_screenRT);

	m_mCurID = 0;

	//rt.right = rt.left - 10;
	CRect rt;
	rt.left = RIGHT_PAGE_R - BUTTONWIDTH*3 - 12*2;
	rt.right = rt.left + BUTTONWIDTH;
	rt.top = m_Rect.Height()-m_TitleHeight-40;
	rt.bottom = rt.top+25;
	
	pInfoBar = CreateStatic(CRect(17, 
								rt.top,
								317,
								rt.top+22), 
							this, 
							"");
	//pInfoBar->SetBkColor(VD_RGB(8,66,189));
	
	pBtBackup = CreateButton(rt, this, "&CfgPtn.Backup", (CTRLPROC)&CPageDataBackup::OnBtBackup, NULL, buttonNormalBmp);
	
	CRect rt2;
	progWidth = rt.left - 10 -17;
	rt2.right = 17;
	rt2.left = 17;
	rt2.bottom = m_Rect.Height() - 12;
	Progbar = CreateStatic(rt2, this, "");
	Progbar->SetBkColor(VD_RGB(8,66,189));
	//progWidth = rt.right-rt.left;
	progRight = rt2.right;
	
	//stateInfo = CreateStatic(CRect(rt2.left,rt2.top,rt2.left+progWidth,rt2.bottom), this, "");
	
	rt.left += BUTTONWIDTH+12;
	rt.right = rt.left + BUTTONWIDTH;
	pBtSearch = CreateButton(rt, this, "&CfgPtn.Search", (CTRLPROC)&CPageDataBackup::OnBtSearch, NULL, buttonNormalBmp);

	rt.left += BUTTONWIDTH+12;
	rt.right = rt.left + BUTTONWIDTH;
	pBtQuit = CreateButton(rt, this, "&CfgPtn.Exit", (CTRLPROC)&CPageDataBackup::OnBtQuit, NULL, buttonNormalBmp);
	
	pBtBackup->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"));
	pBtSearch->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	pBtQuit->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	
	sFileList = (SBizRecfileInfo*)malloc(sizeof(SBizRecfileInfo)*MAX_SEARCH_FILE_NUM);
	//sSnapList = (SBizRecSnapInfo*)malloc(sizeof(SBizRecSnapInfo)*MAX_SEARCH_FILE_NUM);

	//printf("%s yg 3\n", __func__);
	
	InitPage0();
	//printf("%s yg 4\n", __func__);
	InitPage1();
	//printf("%s yg 5\n", __func__);

	ShowSubPage(0,TRUE);
	pButton[0]->Enable(FALSE);
}


CPageDataBackup::~CPageDataBackup()
{
	
}

void CPageDataBackup::InitPage0()
{
	//printf("%s yg 1\n", __func__);
	InitPageLeft0();
	//printf("%s yg 2\n", __func__);
	InitPageRight0();
	//printf("%s yg 3\n", __func__);
	ShowSubPage(0, FALSE);
}

void CPageDataBackup::InitPageLeft1()
{
	int i, j;
	
	CRect rt;
	char tmpchar[20] = {0};
	GetProductNumber(tmpchar);
	
	rt.left = LEFT_PAGE_L + 20;
	if(0 == strcasecmp(tmpchar, "R9632S") || 0 == strcasecmp(tmpchar, "NR3132"))
	{
		rt.top = 14+pBmp_tab_normal->height+10;
	}
	else
	{
		rt.top = 14+pBmp_tab_normal->height+20;
	}
	rt.right =	rt.left+100;
	rt.bottom = rt.top + TEXT_HEIGHT;
	pTextPG3BeginTime = CreateStatic(rt,this, "&CfgPtn.StartTime");
	rt.left = rt.right+10;
	rt.right =	rt.left+110;
	rt.bottom = rt.top + CTRL_HEIGHT;
	pBeginDateP3 = CreateDateTimeCtrl(rt, this, NULL, DTS_DATE);
	rt.left = rt.right+10;
	rt.right =	rt.left+110;
	pBeginTimeP3 = CreateDateTimeCtrl(rt, this, NULL, DTS_TIME);
	
	rt.left = LEFT_PAGE_L + 20;
	if(0 == strcasecmp(tmpchar, "R9632S") || 0 == strcasecmp(tmpchar, "NR3132"))
	{
		rt.top = 5 + rt.bottom;
	}
	else
	{
		rt.top = 10 + rt.bottom;
	}
	rt.right =	rt.left+100;
	rt.bottom = rt.top + TEXT_HEIGHT;
	pTextPG3EndTime = CreateStatic(rt,this, "&CfgPtn.EndTime");
	rt.left = rt.right+10;
	rt.right =	rt.left+110;
	rt.bottom = rt.top + CTRL_HEIGHT;
	pEndDateP3 = CreateDateTimeCtrl(rt, this, NULL, DTS_DATE);
	rt.left = rt.right+10;
	rt.right =	rt.left+110;
	pEndTimeP3 = CreateDateTimeCtrl(rt, this, NULL, DTS_TIME);

	rt.left = LEFT_PAGE_L + 20;
	if(0 == strcasecmp(tmpchar, "R9632S") || 0 == strcasecmp(tmpchar, "NR3132"))
	{
		rt.top = 5 + rt.bottom;
	}
	else
	{
		rt.top = 15 + rt.bottom;
	}
	
	rt.right =	rt.left+100;
	rt.bottom = rt.top + TEXT_HEIGHT;
	pTextPG3Chn = CreateStatic(rt,this, "&CfgPtn.SelectPlayChn");
	
	rt.left = rt.right+10;
	rt.right =	rt.left+230;
	if(0 == strcasecmp(tmpchar, "R9632S") || 0 == strcasecmp(tmpchar, "NR3132"))
	{
		rt.bottom = rt.top + 24*(nChMax/4+((nChMax%4)?1:0));
	}
	else
	{
		rt.bottom = rt.top + CTRL_HEIGHT*(nChMax/4+((nChMax%4)?1:0));
	}
	//rt.bottom = rt.top + CTRL_HEIGHT*(nChMax/4+((nChMax%4)?1:0));
	pTabPG3Chn = CreateTableBox(rt, this, 2, nChMax/4+((nChMax%4)?1:0));
	pTabPG3Chn->SetColWidth(0,30);
	items[1].push_back(pTextPG3Chn);
	items[1].push_back(pTabPG3Chn);
	
	CRect rect;	
	char tmp[5] = {0};
	for(i = 0; i < (nChMax/4+((nChMax%4)?1:0)); i++)
	{
		rect.left = rt.left + 3;
		rect.right = rt.left+27;
		if(0 == strcasecmp(tmpchar, "R9632S") || 0 == strcasecmp(tmpchar, "NR3132"))
		{
			rect.top = rt.top + 24*i + 2;
		}
		else
		{
			rect.top = rt.top + CTRL_HEIGHT*i + 3;
		}
		//rect.top = rt.top + CTRL_HEIGHT*i + 3;
		rect.bottom = rect.top+24;
		pPG3ChkChn[i*5] = CreateCheckBox(rect, this, styleEditable, (CTRLPROC)&CPageDataBackup::OnPG3AllChn);
		pPG3ChkChn[i*5]->SetValue(1);
		items[1].push_back(pPG3ChkChn[i*5]);
		
		rect.left = rt.left + 43;
		int left = rect.left;
		for(j = 0; j < 4; j++)
		{
			if(i*4+j+1>nChMax)
			{
				break;
			}
			rect.left = left+j*45;
			rect.right = rect.left+22;
			pPG3ChkChn[i*5+j+1] = CreateCheckBox(rect, this, styleEditable, (CTRLPROC)&CPageDataBackup::OnPG3Chn);
			pPG3ChkChn[i*5+j+1]->SetValue(1);
			sprintf(tmp, "%d", i*4+j+1);
			pPG3StaticChn[i*4+j] = CreateStatic(CRect(rect.right+2,rect.top,rect.right+21,rect.bottom-4), this, tmp);
			pPG3StaticChn[i*4+j]->SetTextAlign(VD_TA_LEFT);
			items[1].push_back(pPG3ChkChn[i*5+j+1]);
			items[1].push_back(pPG3StaticChn[i*4+j]);
		}
	}

	rt.left = LEFT_PAGE_L + 20;
	if(0 == strcasecmp(tmpchar, "R9632S") || 0 == strcasecmp(tmpchar, "NR3132"))
	{
		rt.top = 13 + rt.bottom;
	}
	else
	{
		rt.top = 18 + rt.bottom;
	}
	rt.right =	rt.left+100;
	rt.bottom = rt.top + TEXT_HEIGHT;
	//pTextPG0StartPlay = CreateStatic(rt,this, "&CfgPtn.StartPlay");
	pTextPG3PicType = CreateStatic(rt,this, "&CfgPtn.PicType");
	
	
	rt.left = rt.right+10;
	//rt.left -= 4;//csp modify 20131018
	//rt.top -= 8;
	//rt.right =  rt.left + TEXT_WIDTH*4;
	rt.right =	rt.left+230;
	rt.bottom = rt.top + 25;

	char* szPicType[] = {		
		"&CfgPtn.Timer",			//定时  已定义
		"&CfgPtn.MotionDet",		//移动侦测
		"&CfgPtn.Alarm2",			//报警 已定义
		"&CfgPtn.MotionOrAlarm",		//动测或报警
		"&CfgPtn.MotionAndAlarm",	//动测且报警
		//"&CfgPtn.Manual",			//手动 
		//"&CfgPtn.IntellDet",			//智能侦测
		"&CfgPtn.ManualSnap",		//手动截图
		//"&CfgPtn.PlayBackSnap",		//回放截图
		"&CfgPtn.All"				//全部
	};

	pComboBoxPicType = CreateComboBox(rt, this, NULL, NULL, NULL, 0);
	pComboBoxPicType->SetBkColor(VD_RGB(67,77,87));
	for (j=0; j<sizeof(szPicType)/sizeof(szPicType[0]); j++)
	{
		pComboBoxPicType->AddString(szPicType[j]);
	}
	pComboBoxPicType->SetCurSel(0);

	items[1].push_back(pTextPG3PicType);
	items[1].push_back(pComboBoxPicType);
	items[1].push_back(pTextPG3BeginTime);
	items[1].push_back(pBeginDateP3);
	items[1].push_back(pBeginTimeP3);
	items[1].push_back(pTextPG3EndTime);
	items[1].push_back(pEndDateP3);
	items[1].push_back(pEndTimeP3);
	
	//ShowSubPage(3, FALSE);

	//回放页面
	VD_RECT	m_RectScreen;
	CRect rtFloat;

	m_pDevGraphics = CDevGraphics::instance(m_screen);
	m_pDevGraphics->GetRect(&m_RectScreen);
	
	rtFloat.left = m_RectScreen.right-TEXT_WIDTH * 8;
	rtFloat.right = m_RectScreen.right;
	rtFloat.top = m_RectScreen.top;
	rtFloat.bottom = m_RectScreen.bottom;

	printf("%s TEXT_WIDTH: %d\n", __func__, TEXT_WIDTH);
	printf("%s rtFloat.left: %d\n", __func__, rtFloat.left);
	printf("%s rtFloat.right: %d\n", __func__, rtFloat.right);
	printf("%s rtFloat.top: %d\n", __func__, rtFloat.top);
	printf("%s rtFloat.bottom: %d\n", __func__, rtFloat.bottom);
	
	//printf("%s new CPageSnapPlayBack\n", __func__);
	//m_pPageSnapPB = new CPageSnapPlayBack(rtFloat, "Snap PlayBack", NULL, this);

}

void CPageDataBackup::InitPageRight1()
{
	CRect rtCal(RIGHT_PAGE_L+3, 12+pBmp_tab_normal->height, RIGHT_PAGE_R-3, 12+pBmp_tab_normal->height+TEXT_HEIGHT*8+10);
	
	//BIZ_DO_DBG("CPageSearch::InitCalendar width %d\n",  RIGHT_PAGE_R+6 - RIGHT_PAGE_L);
	
	printf("Calendar rect:(%d,%d,%d,%d)\n",rtCal.left,rtCal.right,rtCal.top,rtCal.bottom);
	
	pCalendar = CreateCalendar(rtCal, this, NULL, (CTRLPROC)&CPageDataBackup::OnDaySelected);
	itemsRight[1].push_back(pCalendar);
}
void CPageDataBackup::UpdateCalendar(SYSTEM_TIME* pTime)
{	
	pCalendar->SetDate(pTime);
	pCalendar->SetMask(1<<(pTime->day-1));	
}


void CPageDataBackup::InitPage1()
{
	InitPageLeft1();
	InitPageRight1();

	//图片回放和备份做在一起
	//printf("%s new CPageSnapPlayBack\n", __func__);
	//m_pPageSnapPB = new CPageSnapPlayBack(rtFloat, "Snap PlayBack", NULL, this);
	
	ShowSubPage(1, FALSE);
}

//yaogang modify 20150105
void CPageDataBackup::OnPG3AllChn()
{
	int 	i		= 0;
	BOOL	bFind	= FALSE;
	
	CCheckBox* pFocus = (CCheckBox *)GetFocusItem();
	for(i = 0; i < (nChMax/4+((nChMax%4)?1:0)); i++)
	{
		if(pFocus == pPG3ChkChn[i*5])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		for(int j=1; j<5; j++)
		{
			pPG3ChkChn[i*5+j]->SetValue(pPG3ChkChn[i*5]->GetValue());
		}
	}
}

void CPageDataBackup::OnPG3Chn()
{
	
	int i, j;
	BOOL	bFind	= FALSE;
	CCheckBox* pFocus = (CCheckBox *)GetFocusItem();
	int num;
	
	for(i = 0; i < (nChMax/4+((nChMax%4)?1:0)); i++)
	{
		num = 0;
		for(int j=1; j<5; j++)
		{
			if(pPG3ChkChn[i*5+j]->GetValue())
			{
				num++;
			}
		}
		if (num == 4)
		{
			pPG3ChkChn[i*5]->SetValue(1);
		}
		else
		{
			pPG3ChkChn[i*5]->SetValue(0);
		}
	}
}


void CPageDataBackup::OnDaySelected()
{
	//获得当前选中日期
	SYSTEM_TIME stSelTime;
	pCalendar->GetDate(&stSelTime);
	
	printf("%4d-%02d-%02d\n", stSelTime.year, stSelTime.month, stSelTime.day);
	
	//将选中的填充
	pCalendar->SetMask(1<<(stSelTime.day-1));
	
	SYSTEM_TIME stTime;//cw_calendar
	pCalendar->GetDate(&stTime);
	stTime.hour = 0;
	stTime.minute = 0;
	stTime.second = 0;
	pBeginDateP3->SetDateTime(&stTime);
	pBeginTimeP3->SetDateTime(&stTime);
	stTime.hour = 23;
	stTime.minute = 59;
	stTime.second = 59;
	pEndDateP3->SetDateTime(&stTime);
	pEndTimeP3->SetDateTime(&stTime);
	
 	return;
}

void CPageDataBackup::OnClickSubPage()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	
	for(i = 0; i < BACKUP_SUBPAGES; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		printf("%s find the focus button %d\n", __func__, i);
		
		SwitchPage(i);
	}
}

void CPageDataBackup::ShowSubPage( int subID, BOOL bShow )
{
	if(subID<0 || subID>=BACKUP_SUBPAGES)
	{
		return;
	}
	
	int count = items[subID].size();
	
	for (int i=0; i<count; i++)
	{
		if(items[subID][i])
		{
			items[subID][i]->Show(bShow);
		}
	}

	count = itemsRight[subID].size();
	
	for (int i=0; i<count; i++)
	{
		if(itemsRight[subID][i])
		{
			itemsRight[subID][i]->Show(bShow);
		}
	}
	
	//printf("ShowSubPage end\n");
}

void CPageDataBackup::SwitchPage( int subID )
{
	if(subID<0 || subID>=BACKUP_SUBPAGES)
	{
		return;
	}
	
	if(subID == m_mCurID)
	{
		return;
	}
	
	//隐藏当前子页面，显示新的子页面
	ShowSubPage(m_mCurID, FALSE);
	pButton[m_mCurID]->Enable(TRUE);
	m_mCurID = subID;
	ShowSubPage(m_mCurID, TRUE);
	pButton[subID]->Enable(FALSE);
	
	if(m_mCurID == 0)
	{
		pBtBackup->Show(TRUE);
	}
	else
	{
		pBtBackup->Show(FALSE);
	}
}

int CPageDataBackup::SetProgress(int nProg)
{
	//formatProg->Show(TRUE,TRUE);

	if(nProg == 0)
	{
		nProg = 1;
	}

	if(nProg == -1)
	{
		nProg = 0;
		//Progbar->SetText("&UiLang.BackupDone");
		//Progbar->SetText("&UiLang.BackupDone");
	}
	
	int width = progWidth*nProg/100;

	//printf("width = %d \n",width);

	CRect rt;
	rt.left = (m_screenRT.Width()-m_Rect.Width())/2+17;
	rt.top = (m_screenRT.Height()-m_Rect.Height())/2+m_Rect.Height()-35;
	rt.right = rt.left+width;
	rt.bottom = rt.top +23;

	Progbar->SetRect(rt,TRUE);
}

void CPageDataBackup::InitPageLeft0()
{
	//printf("%s yg 1\n", __func__);
	CRect rt;
	rt.left = LEFT_PAGE_L+4;
	rt.top = FRAME_TOP + CTRL_HEIGHT;
	rt.right =  LEFT_PAGE_R-4;
	rt.bottom =rt.top + CTRL_HEIGHT*TAB_LINES;

	m_nPageSize = TAB_LINES-1;
	m_nTotalPages = 0;
	m_nCurPage = 0;

#ifdef SHOW_STATE
#define ROW_CNR	5
#else
#define ROW_CNR	4
#endif

	pTabPageLeft[0] = CreateTableBox(rt, this, ROW_CNR, TAB_LINES);
	pTabPageLeft[0]->SetColWidth(0, 28);
	pTabPageLeft[0]->SetColWidth(1, TEXT_WIDTH*3-22);
	pTabPageLeft[0]->SetColWidth(2, (LEFT_PAGE_R-LEFT_PAGE_L-23-TEXT_WIDTH*3)/2+7/*TEXT_WIDTH*7*/);
	pTabPageLeft[0]->SetColWidth(3, (LEFT_PAGE_R-LEFT_PAGE_L-23-TEXT_WIDTH*3)/2+3/*TEXT_WIDTH*7*/);

	items[0].push_back(pTabPageLeft[0]);
	//printf("%s yg 2 ROW_CNR: %d\n", __func__, ROW_CNR);
	
	char* szTitle[5] = {
		"",
		"&CfgPtn.Channel",
		"&CfgPtn.Start", 
		"&CfgPtn.End",
		"&CfgPtn.Status"
	};
	int i = 0;
	CRect rtTemp;
	for (i=0; i<ROW_CNR; i++)
	{	
		pTabPageLeft[0]->GetTableRect(i, 0, &rtTemp);

		pTitlePageLeft[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, szTitle[i]);
		pTitlePageLeft[i]->SetBkColor(VD_RGB(67,77,87));
		pTitlePageLeft[i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		//pTitlePageLeft[i]->SetTextAlign(VD_TA_XLEFT);
		pTitlePageLeft[i]->SetTextAlign(VD_TA_CENTER);

		items[0].push_back(pTitlePageLeft[i]);
		//printf("%s yg 3 i: %d\n", __func__, i);
	}
	//printf("%s yg 3\n", __func__);
	
	for(i=0; i<TAB_LINES - 1; i++)
	{
		pTabPageLeft[0]->GetTableRect(1, i+1, &rtTemp);
		pChn[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");

		items[0].push_back(pChn[i]);
	}
	//printf("%s yg 4\n", __func__);

	for(i=0; i<TAB_LINES - 1; i++)
	{
		pTabPageLeft[0]->GetTableRect(2, i+1, &rtTemp);
		pBegin[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");

		items[0].push_back(pBegin[i]);
	}

	for(i=0; i<TAB_LINES - 1; i++)
	{
		pTabPageLeft[0]->GetTableRect(3, i+1, &rtTemp);
		pEnd[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");

		items[0].push_back(pEnd[i]);
	}

#ifdef SHOW_STATE
	for(i=0; i<TAB_LINES - 1; i++)
	{
		pTabPageLeft[0]->GetTableRect(4, i+1, &rtTemp);
		pState[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");

		items[0].push_back(pState[i]);
	}
#endif

	rt.top = rt.bottom -2;
	rt.bottom =rt.top + CTRL_HEIGHT;

	pTabPageLeft[1] = CreateTableBox(rt, this, 1, 1);
	items[0].push_back(pTabPageLeft[1]);

	char tmp[10] = {0};
	int totalpage =0;
	int curpage = 0;
	rt.left += 5;
	rt.top += 2;
	rt.right = rt.left + TEXT_WIDTH*2 + 8;
	rt.bottom -=2;
	sprintf(tmp, "%d/%d", curpage,totalpage);
	pszResult = CreateStatic(rt, this, tmp);
	pszResult->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
	pszResult->SetTextAlign(VD_TA_LEFT);
	items[0].push_back(pszResult);

	rt.left = LEFT_PAGE_R - 22 - 40*4 - 10*4;

 	pChkChnLeft[TAB_LINES - 1] = CreateCheckBox(CRect(rt.left-40, rt.top, rt.left-10, rt.bottom), this, styleEditable, (CTRLPROC)&CPageDataBackup::OnClickChooseChn);
	items[0].push_back(pChkChnLeft[TAB_LINES - 1]);
	
 	pszChooseAll = CreateStatic(CRect(rt.left-90, rt.top, rt.left-40, rt.bottom), this, "&CfgPtn.All");
	items[0].push_back(pszChooseAll);
	
	for (i =0; i<4; i++)
	{
 		pBmpBtPageCtrl[i][0] = VD_LoadBitmap(szBtBmpPath[i][0]);
 		pBmpBtPageCtrl[i][1] = VD_LoadBitmap(szBtBmpPath[i][1]);
		//printf("VD_LoadBitmap %x %x \n", pBmpBtPageCtrl[i][0], pBmpBtPageCtrl[i][1]);
		pBtPageCtr[i] = CreateButton(CRect(rt.left, rt.top, rt.left+40, rt.bottom), this, NULL, (CTRLPROC)&CPageDataBackup::OnClickPageCtr, NULL, buttonNormalBmp);
		rt.left += 40+10;
		pBtPageCtr[i]->SetBitmap(pBmpBtPageCtrl[i][0], pBmpBtPageCtrl[i][1], pBmpBtPageCtrl[i][1]);
		items[0].push_back(pBtPageCtr[i]);
	}

	rt.left = LEFT_PAGE_L+4;
	rt.top = FRAME_TOP + CTRL_HEIGHT * 2;
	for (i = 0; i< TAB_LINES - 1; i++)
	{
		CRect rtTemp;
		pTabPageLeft[0]->GetTableRect(0, i, &rtTemp);
		pChkChnLeft[i] = CreateCheckBox(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), this, styleEditable, (CTRLPROC)&CPageDataBackup::OnClickChooseChn);
		items[0].push_back(pChkChnLeft[i]);
	}

	m_pPageBackupInfo = new CPageBackupInfo(NULL, "&CfgPtn.BackupInfo"/*"&titles.mainmenu"*/, icon_dvr, this);
	SetPage(EM_PAGE_BACKUP, m_pPageBackupInfo);

	m_nCurMouseLine = 0;
}

void CPageDataBackup::InitPageRight0()
{
	CRect rtFrame(RIGHT_PAGE_L+2, FRAME_TOP + CTRL_HEIGHT, RIGHT_PAGE_R-2, FRAME_TOP+CTRL_HEIGHT*(TAB_LINES+2)-2);

	pTabPageLeft[1] = CreateTableBox(rtFrame, this, 1, 1);
	itemsRight[0].push_back(pTabPageLeft[1]);

	CRect rtTime;
	rtTime.left = rtFrame.right-114;
	rtTime.right = rtFrame.right-4;
	rtTime.top = rtFrame.top+10;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pBeginDatePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_DATE);
	itemsRight[0].push_back(pBeginDatePR);

	rtTime.top = rtTime.bottom+2;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pBeginTimePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_TIME);
	itemsRight[0].push_back(pBeginTimePR);

	rtTime.top = rtTime.bottom+8;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pEndDatePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_DATE);
	itemsRight[0].push_back(pEndDatePR);

	rtTime.top = rtTime.bottom+2;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pEndTimePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_TIME);
	itemsRight[0].push_back(pEndTimePR);

	pStaticBegin = CreateStatic(CRect(rtFrame.left+2, rtFrame.top+10,rtFrame.right-114,rtFrame.top+34), this, "&CfgPtn.StartTime");
	pStaticBegin->SetTextAlign(VD_TA_CENTER);
	itemsRight[0].push_back(pStaticBegin);
	
	pStaticEnd = CreateStatic(CRect(rtFrame.left+2, rtFrame.top+75,rtFrame.right-114,rtFrame.top+98), this, "&CfgPtn.EndTime");
	pStaticEnd->SetTextAlign(VD_TA_CENTER);
	itemsRight[0].push_back(pStaticEnd);
	
	int nRowChn 	= 4;
	int nRow 		= (nChMax+3)/nRowChn;
	int nRemainCh 	= nChMax%nRowChn;
	int nBtnSize    = 21;//(m_Rect.Height()-60-rtCal.bottom-5)/nRow;
	int nLeftGap    = 4;
	int nBtnGap     = 0;

	pTabChnSearch = CreateTableBox(CRect(rtFrame.left+2, rtTime.bottom+5, rtFrame.right-2, rtTime.bottom+5+nBtnSize*nRow+4), this, 2, 1);
	pTabChnSearch->SetColWidth(0, nBtnSize+6);
	itemsRight[0].push_back(pTabChnSearch);
	
	CRect rt;
	
	int nLeftPosBase = rtFrame.left+nLeftGap;
	int nTopPosBase = rtTime.bottom+5 + 2;
	
	int i, j;
	for(j=0; j<nRow; j++)
	{
		rt.left = nLeftPosBase;
		rt.right = rt.left + nBtnSize + nBtnGap;
		rt.top = nTopPosBase + j*nBtnSize;
		rt.bottom = rt.top + nBtnSize;
		pChkChn[j*(nRowChn+1)] = CreateCheckBox(rt, this, styleEditable, (CTRLPROC)&CPageDataBackup::OnCalendarAllChn);
		pChkChn[j*(nRowChn+1)]->SetValue(1);
		itemsRight[0].push_back(pChkChn[j*(nRowChn+1)]);
		
		rt.left = rt.right + nBtnGap + 10;
		rt.right = rt.left + nBtnSize + nBtnGap;
		
		for(i=1; i<5; i++)
		{
			if(j*nRowChn+i>nChMax)
			{
				break;
			}
			
			pChkChn[j*(nRowChn+1)+i] = CreateCheckBox(rt, this);
			pChkChn[j*(nRowChn+1)+i]->SetValue(1);
			itemsRight[0].push_back(pChkChn[j*(nRowChn+1)+i]);
			
			rt.left = rt.right + 2;
			rt.right = rt.left + 16;
			char tmp[5] = {0};
			sprintf(tmp, "%d", i+j*nRowChn);
			pStaticChn[i-1+j*(nRowChn)] = CreateStatic(rt, this, tmp);
			pStaticChn[i-1+j*(nRowChn)]->SetTextAlign(VD_TA_LEFT);
			itemsRight[0].push_back(pStaticChn[i-1+j*(nRowChn)]);
			
			rt.left  = rt.right;
			rt.right = rt.left + nBtnSize + nBtnGap;
		}
		
		if(i != 5) break;
	}
}

VD_BOOL CPageDataBackup::UpdateData( UDM mode )
{
	u8 nMax = GetVideoMainNum();
	time_t long_time = time(NULL);
	
	//csp modify 20131213
	int nTimeZone = GetTimeZone();
	long_time += GetTimeZoneOffset(nTimeZone);
	
	//csp modify
	//struct tm *tmptime = localtime(&long_time);
	struct tm tm0;
	struct tm *tmptime = &tm0;
	localtime_r((time_t*)&long_time, tmptime);
	
	SYSTEM_TIME stTime1, stTime2;
	stTime1.year = tmptime->tm_year+1900;
	stTime1.month = tmptime->tm_mon+1;
	stTime1.day = tmptime->tm_mday;
	stTime1.hour = 0;
	stTime1.minute = 0;
	stTime1.second = 0;
	stTime2 = stTime1;
	stTime2.hour = tmptime->tm_hour;
	stTime2.minute = tmptime->tm_min;
	stTime2.second = tmptime->tm_sec;

	pBeginDatePR->SetDateTime(&stTime1);
	pBeginTimePR->SetDateTime(&stTime1);
	pEndDatePR->SetDateTime(&stTime2);
	pEndTimePR->SetDateTime(&stTime2);

	UpdateCalendar(&stTime1);
	pBeginDateP3->SetDateTime(&stTime1);
	pBeginTimeP3->SetDateTime(&stTime1);

	pEndDateP3->SetDateTime(&stTime2);
	pEndTimeP3->SetDateTime(&stTime2);

	if(UDM_OPEN == mode)
	{
		switch(m_mCurID)
		{
			case 0://录像
			{
				memset( m_nSelect, 0, sizeof(m_nSelect) );
				m_nTotalPages = 0;
				m_nCurPage = 0;
				m_nFiles = 0;
				
				char szChn[16] = {0};
				int i = 0;
				/*
				for(i=0; i<nMax; i++)
				{
					//sprintf(szChn,"%d",i+1);
					sprintf(szChn,"",i+1);
					SetChn(i,szChn);
				}
				*/
				for(i=0; i<TAB_LINES-1; i++)
				{
					pChkChnLeft[i]->SetValue(0);
					pChkChnLeft[i]->Enable(FALSE);
					
					SetChn(i, "");
					SetBegin(i, "");
					SetEnd(i, "");
				}
				
				SetResult("0/0");
				
				
				pBtBackup->Show(TRUE);
				pBtBackup->Enable(TRUE);
				//stateInfo->SetText("");
				SetInfo("");
			}break;
			case 1://图片
			{
				pBtBackup->Show(FALSE);
			}break;
		}
		
	}
	else if(UDM_CLOSED == mode)
	{
		memset( m_nSelect, 0, sizeof(m_nSelect) );
		m_nFiles = 0;
		((CPageMainFrameWork *)(this->GetParent()))->ReflushItemName();
	}
	
	return TRUE;
}

void CPageDataBackup::SetChn(int row, char* szText)
{
	if( row<0 || row>8)
	{
		return;
	}

	pChn[row]->SetText(szText);
}

void CPageDataBackup::SetBegin(int row, char* szText)
{
	if( row<0 || row>8)
	{
		return;
	}

	pBegin[row]->SetText(szText);
}

void CPageDataBackup::SetEnd(int row, char* szText)
{
	if( row<0 || row>8)
	{
		return;
	}

	pEnd[row]->SetText(szText);
}

void CPageDataBackup::SetState(int row, char* szText)
{
	if( row<0 || row>8)
	{
		return;
	}

	pState[row]->SetText(szText);
}

void CPageDataBackup::SetResult(char* szText)
{
	pszResult->SetText(szText);
}

void CPageDataBackup::OnClickPageCtr()
{
	CButton* pFocus = (CButton*)GetFocusItem();
	int nJumpPage = 0, nNewItems, nOldItems, nBaseItem;

	pChkChnLeft[TAB_LINES-1]->SetValue(0);
	if (pFocus == pBtPageCtr[0]) //first page
	{
	    if (0 == m_nCurPage)
	    {
            return;
	    }
	    else
	    {
		    nJumpPage = 0;
		}
	}
	else if (pFocus == pBtPageCtr[1])//pre page
	{
	    if (0 == m_nCurPage)
	    {
            return;
	    }
	    else
	    {
    		nJumpPage = m_nCurPage - 1;
    		if (nJumpPage<=0)  //cw_9508S
    			nJumpPage=0;
		}
	}
	else if (pFocus == pBtPageCtr[2])//next  page
	{
	    if (m_nTotalPages - 1 == m_nCurPage)
	    {
            return;
	    }
	    else
	    {
    		nJumpPage = m_nCurPage + 1;
    		if (nJumpPage>=m_nTotalPages)//cw_9508S
    			nJumpPage=m_nTotalPages - 1;
		}
	}
	else if (pFocus == pBtPageCtr[3])//last  page
	{
	    if (m_nTotalPages - 1 == m_nCurPage)
	    {
            return;
	    }
	    else
	    {
		    nJumpPage = m_nTotalPages - 1;
		}
	}
	else
	{
		return;
	}
	
	if (nJumpPage >= 0 && nJumpPage < m_nTotalPages)
	{
	    //if (nJumpPage != m_nCurPage)//cw_9508S
		{
			char szTime[64]={0};
			char szCh[4]={0};
			int i;
			u8 bstatus=1;//cw_backup
			nBaseItem = m_nCurPage * m_nPageSize;
			nOldItems = (m_nFiles > nBaseItem + m_nPageSize) ? m_nPageSize : (m_nFiles - nBaseItem);
			nBaseItem = nJumpPage * m_nPageSize;
			nNewItems = (m_nFiles > nBaseItem + m_nPageSize) ? m_nPageSize : (m_nFiles - nBaseItem);
			for (i = 0; i < nNewItems; i++)
			{
				SBizRecfileInfo* pFInfo = &m_sFileList[nBaseItem + i];
				pChkChnLeft[i]->Enable(TRUE);
				pChkChnLeft[i]->SetValue(m_nSelect[nBaseItem + i]);
				
				sprintf(szCh, "%d", pFInfo->nChn);
				SetChn(i,szCh);
				GetTimeForBackup(pFInfo->nStartTime, szTime);
				SetBegin(i, szTime);
				GetTimeForBackup(pFInfo->nEndTime, szTime);
				SetEnd(i, szTime);
				/*
				if(m_nSelect[nBaseItem + i] == 1)  //cw_backup
					pChkChnLeft[9]->SetValue(1);
				else
					pChkChnLeft[9]->SetValue(0);
					*/
				bstatus&=m_nSelect[nBaseItem + i];//cw_backup
			}
			
			if(bstatus)  //cw_backup
				pChkChnLeft[TAB_LINES-1]->SetValue(1);
			else
				pChkChnLeft[TAB_LINES-1]->SetValue(0);
			
			for (; i < m_nPageSize; i++)
			{
				pChkChnLeft[i]->SetValue(0);
				pChkChnLeft[i]->Enable(FALSE);
				
				SetChn(i, "");
				SetBegin(i, "");
				SetEnd(i, "");
			}
			m_nCurPage = nJumpPage;
			SetProg(m_nCurPage + 1, m_nTotalPages);
		}
	}
}

void CPageDataBackup::BackupwithTime(EMBIZFILETYPE emFileType,SBizBackTgtAtt* sBackTgtAtt,SBizSearchCondition* sSearchCondition)//cw_bakcup
{
	u32 nMaskChn = 0;
	u8 nMax = GetVideoMainNum();
	u64 filesize=0; 
	SBizSearchPara sSearchPara;
	SBizSearchResult *psResult, sSearchResult;
	
	memset(&sSearchPara, 0, sizeof(sSearchPara));
	memset(&sSearchResult, 0, sizeof(sSearchResult));
	
	memset(m_nSelect, 0, sizeof(m_nSelect));
	
	// fill conditon
	
	sSearchPara.nMaskType = 0xff;
	
	int i=0;
	
	for(int j=0; j<(nChMax+3)/4; j++)
	{
		for(int i = 1; i < 5; i++)
		{
			//csp modify
			if(j*4+i > nChMax) break;
			
			if(pChkChn[j*5+i]->GetValue())
			{
				nMaskChn |= (1<<(j*4+i-1));
			}
		}
	}
	
	SYSTEM_TIME sSearchDateIns, sSearchTimeIns;
	
	memset(&sSearchDateIns, 0, sizeof(sSearchDateIns));
	memset(&sSearchTimeIns, 0, sizeof(sSearchTimeIns));
	
	pBeginDatePR->GetDateTime(&sSearchDateIns);
	pBeginTimePR->GetDateTime(&sSearchTimeIns);
	
	sSearchPara.nStartTime = 
		MakeTimeForBackup(
			sSearchDateIns.year,
			sSearchDateIns.month,
			sSearchDateIns.day,
			sSearchTimeIns.hour,
			sSearchTimeIns.minute,
			sSearchTimeIns.second
		);
	
	GetTimeForBackup(sSearchPara.nStartTime, m_sTime);	
	
	memset(&sSearchDateIns, 0, sizeof(sSearchDateIns));
	memset(&sSearchTimeIns, 0, sizeof(sSearchTimeIns));
	
	pEndDatePR->GetDateTime(&sSearchDateIns);
	pEndTimePR->GetDateTime(&sSearchTimeIns);
	
	sSearchPara.nEndTime = 
		MakeTimeForBackup(
			sSearchDateIns.year,
			sSearchDateIns.month,
			sSearchDateIns.day,
			sSearchTimeIns.hour,
			sSearchTimeIns.minute,
			sSearchTimeIns.second
		);
	
	GetTimeForBackup(sSearchPara.nEndTime, m_eTime);	
	
	sSearchPara.nMaskChn = nMaskChn;
	
	// search by time
	
	sSearchResult.psRecfileInfo = (SBizRecfileInfo*)m_sFileList;
	
	//函数功能:文件搜索
	if( 0==BizSysComplexDMSearch(
		emFileType,
		&sSearchPara, 
		&sSearchResult, 
		MAX_SEARCH_FILE_NUM) )
	{
		psResult = &sSearchResult;
		
		if(0==psResult->nFileNum)
		{
			printf("cw***%s,FileNum error\n",__func__);
			return;
		}
		
		SBizRecfileInfo* pFInfo;
		for(int i = 0; i < psResult->nFileNum; i++)
		{
			pFInfo = &psResult->psRecfileInfo[i];
			filesize += pFInfo->nSize;
			//printf("filesize %llu\n", filesize);
		}
		
		/*
		u32 fReserve = (1<<27);//reserve 128mb
		printf("fsz %llu limit %u fReserve %d\n", filesize, (0xffffffff - fReserve), fReserve);
		if(filesize >= (u64)((u32)0xffffffff - fReserve))//max u32
		{
			MessageBox("&CfgPtn.BackupSpill","&CfgPtn.WARNING",MB_OK|MB_ICONWARNING);
 			return;
		}
		*/
		
		if(filesize==0)
		{
			printf("cw***%s,FileSize error\n",__func__);
			return;
		}
		if(MAX_SEARCH_FILE_NUM == psResult->nFileNum)
		{
			char tmp[100] = {0};
			sprintf(tmp, "%s:%d", GetParsedString("&CfgPtn.ReachMaxSupportSearchNum"),psResult->nFileNum);
			MessageBox(tmp, "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
		}
	}
	
	sSearchCondition->sSearchResult.nFileNum = psResult->nFileNum;
	sSearchCondition->sSearchResult.psRecfileInfo = psResult->psRecfileInfo;
	if(m_pPageBackupInfo)
	{
		m_pPageBackupInfo->SetBackupInfo(emFileType,sBackTgtAtt,sSearchCondition);
		m_pPageBackupInfo->SetFileNum(psResult->nFileNum);
		m_pPageBackupInfo->SetStartTime(m_sTime);
		m_pPageBackupInfo->SetEndTime(m_eTime);
		m_pPageBackupInfo->SetFileSize(filesize/(u64)MEGA, filesize%(u64)MEGA);
		m_pPageBackupInfo->SetBackupTitle(EM_BACKUP_TIME);
		m_pPageBackupInfo->SetRecMode();
		m_pPageBackupInfo->Open();
	}
	return;
}

void CPageDataBackup::OnBtBackup()
{
	#if 1
	s32 i, nCount = 0;
	u64 filesize = 0;//cw_backup
	EMBIZFILETYPE emFileType = EM_BIZFILE_REC;
	
	sBackTgtAtt.fmt = EM_BIZFORMAT_ORIGINAL;
	sBackTgtAtt.devt = EM_BIZBACKUP_USB;
	sSearchCondition.emSearchWay = EM_BIZSEARCH_BY_FILE;
	
	sSearchCondition.sSearchResult.nFileNum = 0;
	sSearchCondition.sSearchResult.psRecfileInfo = sFileList;
	
	for(i = 0; i < m_nFiles; i++)
	{
		if(m_nSelect[i])
		{
			memcpy(&sFileList[nCount], &m_sFileList[i], sizeof(SBizRecfileInfo));
			filesize += sFileList[nCount].nSize;//cw_backup
			/*
			u32 fReserve = (1<<27);//reserve 128mb
 			if(filesize >= (~(0<<31) - fReserve))//max u32
			{
				MessageBox("&CfgPtn.BackupSpill","&CfgPtn.WARNING",MB_OK|MB_ICONWARNING);
				nCount = 0;
				break;
			}
			*/
			nCount++;
		}
	}
	
	// 无U盘则提示
	u8 nMaxDiskNum = 64;
	
	SBizStoreDevManage sStoreDevMgr;
	sStoreDevMgr.nDiskNum = 0;
	sStoreDevMgr.psDevList = (SBizDevInfo*)malloc(sizeof(SBizDevInfo)*nMaxDiskNum);
	
	memset(sStoreDevMgr.psDevList, 0, sizeof(SBizDevInfo)*nMaxDiskNum);
	
	u8 nDiskNum = 0;
	
	if(sStoreDevMgr.psDevList)
	{
		BizSysComplexDMGetInfo(&sStoreDevMgr, nMaxDiskNum);
		
		if(sStoreDevMgr.nDiskNum > 0)
		{
			for(int i=0; i<sStoreDevMgr.nDiskNum; i++)
			{
				if(sStoreDevMgr.psDevList[i].nType == 1)
				{
					nDiskNum++;
				}
			}
		}
		
		free(sStoreDevMgr.psDevList);
		sStoreDevMgr.psDevList = NULL;//csp modify 20131213
		
		if(nDiskNum <= 0)
		{
			MessageBox("&CfgPtn.BackupNoUdisk", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
			return;
		}
	}
	else
	{
		return;
	}
	
 	if(nCount > 0)
	{
		//SetProgress(0);
		//stateInfo->Show(FALSE,TRUE);
		//ClearInfo();
		//Draw();//if not draw, table title will disappear
		sSearchCondition.sSearchResult.nFileNum = nCount;
		BizGUiWriteLog(BIZ_LOG_MASTER_BACKUP, BIZ_LOG_SLAVE_START_BACKUP);
		
		/*
  		int ret = BizSysComplexBackup( emFileType, &sBackTgtAtt, &sSearchCondition);
		if(ret!=0)
		{
			SetInfo("Backup fail");
		}
		else
		{
			SetInfo("Backup success!");
		}
		BizGUiWriteLog(BIZ_LOG_MASTER_BACKUP, BIZ_LOG_SLAVE_END_BACKUP);
		SetProgress(-1);
		stateInfo->Show(TRUE,TRUE);
		*/
		
		if(m_pPageBackupInfo)
		{
			m_pPageBackupInfo->SetBackupInfo(emFileType,&sBackTgtAtt,&sSearchCondition);
			m_pPageBackupInfo->SetFileNum(nCount);
			m_pPageBackupInfo->SetStartTime(m_sTime);
			m_pPageBackupInfo->SetEndTime(m_eTime);
			m_pPageBackupInfo->SetFileSize(filesize/(u64)MEGA, filesize%(u64)MEGA);
			m_pPageBackupInfo->SetBackupTitle(EM_BACKUP_FILE);//cw_backup
			m_pPageBackupInfo->SetRecMode();
			m_pPageBackupInfo->Open();
		}
	}
	else
	{
		//按时间备份//cw_backup
		BackupwithTime(emFileType,&sBackTgtAtt,&sSearchCondition);
	}
	#endif
}

void CPageDataBackup::OnBtSearch()
{
	EMBIZFILETYPE emFileType;
	SBizSearchPara sSearchPara;
	switch (m_mCurID)
	{
		case 0://录像
		{
			emFileType = EM_BIZFILE_REC;
			u32 nMaskChn = 0;
			u8 nMax = GetVideoMainNum();
			
			SBizSearchResult *psResult, sSearchResult;
			
			m_nTotalPages = 0;
			m_nCurPage = 0;
			
			memset(&sSearchPara, 0, sizeof(sSearchPara));
			memset(&sSearchResult, 0, sizeof(sSearchResult));
			
			memset(m_nSelect, 0, sizeof(m_nSelect));
			
			// fill contditon
			
			sSearchPara.nMaskType = 0xff;
			
			int i=0;
			
			for(int j=0; j<(nChMax+3)/4; j++)
			{
				for(int i = 1; i < 5; i++)
				{
					//csp modify
					if(j*4+i > nChMax) break;
					
					if(pChkChn[j*5+i]->GetValue())
					{
						nMaskChn |= (1<<(j*4+i-1));
					}
				}
			}

			SYSTEM_TIME sSearchDateIns, sSearchTimeIns;

			memset(&sSearchDateIns, 0, sizeof(sSearchDateIns));
			memset(&sSearchTimeIns, 0, sizeof(sSearchTimeIns));
			
			pBeginDatePR->GetDateTime(&sSearchDateIns);
			pBeginTimePR->GetDateTime(&sSearchTimeIns);

			
			sSearchPara.nStartTime = 
				MakeTimeForBackup(
					sSearchDateIns.year,
					sSearchDateIns.month,
					sSearchDateIns.day,
					sSearchTimeIns.hour,
					sSearchTimeIns.minute,
					sSearchTimeIns.second
				);
			
			/*
			//yaogang 20150729
			sSearchDateIns.year = 1970;
			
			sSearchPara.nStartTime = 
				MakeTimeForBackup(
					sSearchDateIns.year,
					sSearchDateIns.month,
					sSearchDateIns.day,
					sSearchTimeIns.hour,
					sSearchTimeIns.minute,
					sSearchTimeIns.second
				);
			
			int nTimeZone = GetTimeZone();
			sSearchPara.nStartTime += GetTimeZoneOffset(nTimeZone);
			*/
			
			GetTimeForBackup(sSearchPara.nStartTime, m_sTime);	
			
			memset(&sSearchDateIns, 0, sizeof(sSearchDateIns));
			memset(&sSearchTimeIns, 0, sizeof(sSearchTimeIns));
			
			pEndDatePR->GetDateTime(&sSearchDateIns);
			pEndTimePR->GetDateTime(&sSearchTimeIns);
			
			sSearchPara.nEndTime = 
				MakeTimeForBackup(
					sSearchDateIns.year,
					sSearchDateIns.month,
					sSearchDateIns.day,
					sSearchTimeIns.hour,
					sSearchTimeIns.minute,
					sSearchTimeIns.second
				);
			
			GetTimeForBackup(sSearchPara.nEndTime, m_eTime);	
			
			sSearchPara.nMaskChn = nMaskChn;
					
			// search by time
			#if 1
			sSearchResult.psRecfileInfo = (SBizRecfileInfo*)m_sFileList;
			
			//函数功能:文件搜索
			if( 0==BizSysComplexDMSearch(
				emFileType,
				&sSearchPara, 
				&sSearchResult, 
				MAX_SEARCH_FILE_NUM) )
			{
				psResult = &sSearchResult;
				
				// fill result
				
				SBizRecfileInfo* pFInfo;
				
				char szTime[64]={0};
				char szCh[4]={0};
				
				int i, nEnd = MIN(psResult->nFileNum, m_nPageSize);
				
				for(i=0; i< nEnd; i++)
				{
					pFInfo = &psResult->psRecfileInfo[i];
					pChkChnLeft[i]->Enable(TRUE);
					//pChkChnLeft[i]->SetValue(1);

					sprintf(szCh, "%d", pFInfo->nChn);
					SetChn(i,szCh);
					GetTimeForBackup(pFInfo->nStartTime, szTime);		
					SetBegin(i, szTime);
					GetTimeForBackup(pFInfo->nEndTime, szTime);
					SetEnd(i, szTime);
					pChkChnLeft[i]->SetValue(0);
				}
				
				for (; i < m_nPageSize; i++)
				{
					pChkChnLeft[i]->SetValue(0);
					pChkChnLeft[i]->Enable(FALSE);
					
					SetChn(i, "");
					SetBegin(i, "");
					SetEnd(i, "");
				}
				pChkChnLeft[i]->SetValue(0);
				m_nFiles = psResult->nFileNum;
				
				m_nTotalPages = (m_nFiles + m_nPageSize - 1) / m_nPageSize;
				SetProg(m_nCurPage + 1, m_nTotalPages);
				pBtBackup->Enable(TRUE);
				//pChkChnLeft[9]->SetValue(1);
				
				if(MAX_SEARCH_FILE_NUM == psResult->nFileNum)
				{
					char tmp[100] = {0};
					sprintf(tmp, "%s:%d", GetParsedString("&CfgPtn.ReachMaxSupportSearchNum"),psResult->nFileNum);
					MessageBox(tmp, "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
				}
			}
			#endif
		}break;
		case 1://图片
		{
			printf("%s() time: %u\n", __func__, time(NULL));
	
			emFileType = EM_BIZFILE_SNAP;
			u32 nMaskChn = 0;
			u8 nMax = GetVideoMainNum();

			//set SBizSearchPara
			memset(&sSearchPara, 0, sizeof(sSearchPara));
			
			//回放时搜索所有类型图片
			sSearchPara.nMaskType = 0xff;

			/*功能未完成
			int type_idx = pComboBoxPicType->GetCurSel();
			switch (type_idx)
			{
				case 0:	//定时
				{
					sSearchPara.nMaskType = 1<<0;
				} break;
				case 1:	//移动侦测
				{
					sSearchPara.nMaskType = 1<<1;
				} break;
				case 2:	//报警
				{
					sSearchPara.nMaskType = 1<<2;
				} break;
				case 3:	//动测或报警
				{
					sSearchPara.nMaskType = (1<<1) | (1<<2);
				} break;
				case 4:	//动测且报警
				{
					sSearchPara.nMaskType = (1<<1) | (1<<2) | (1<<7);
				} break;
				case 5:	//手动截图
				{
					sSearchPara.nMaskType = (1<<4);
				} break;
				case 6:	//全部
				{
					sSearchPara.nMaskType = 0xff;
				} break;
				default :	//全部
				{
					sSearchPara.nMaskType = 0xff;
				}
				
			}
			*/
			
			//指定通道掩码
			for(int i=0; i<nMax; i++)
			{
				if(pPG3ChkChn[(i/4+1)+i]->GetValue())
				{
					nMaskChn |= (1<<i);
				}
			}
			sSearchPara.nMaskChn = nMaskChn;

			
			SYSTEM_TIME start;
			SYSTEM_TIME stop;
			
			pBeginDateP3->GetDateTime(&start);
			pBeginTimeP3->GetDateTime(&start);
			pEndDateP3->GetDateTime(&stop);
			pEndTimeP3->GetDateTime(&stop);	
			struct tm tM;
			memset(&tM, 0, sizeof(tM));	
			tM.tm_year = start.year - 1900;
			tM.tm_mon = start.month - 1;
			tM.tm_mday = start.day;
			tM.tm_hour = start.hour;
			tM.tm_min = start.minute;
			tM.tm_sec = start.second;
			tM.tm_isdst = 0;
			tM.tm_wday = 0;
			tM.tm_yday = 0;
			sSearchPara.nStartTime = mktime(&tM);
			printf("\t start time: %d/%d/%d %d:%d:%d\n", start.year,start.month,start.day,start.hour,start.minute,start.second);

			memset(&tM, 0, sizeof(tM));	
			tM.tm_year = stop.year - 1900;
			tM.tm_mon = stop.month - 1;
			tM.tm_mday = stop.day;
			tM.tm_hour = stop.hour;
			tM.tm_min = stop.minute;
			tM.tm_sec = stop.second;
			tM.tm_isdst = 0;
			tM.tm_wday = 0;
			tM.tm_yday = 0;
			sSearchPara.nEndTime = mktime(&tM);
			printf("\t end time: %d/%d/%d %d:%d:%d\n", stop.year,stop.month,stop.day,stop.hour,stop.minute,stop.second);
			
			int nTimeZone = GetTimeZone();
			sSearchPara.nStartTime -= GetTimeZoneOffset(nTimeZone);
			sSearchPara.nEndTime -= GetTimeZoneOffset(nTimeZone);

			CPage** page = GetPage();
			m_pPageSnapPB = (CPageSnapPlayBack *)page[EM_PAGE_SNAP];
			
			//传递搜索参数PageSnapPlayBack.cpp
			m_pPageSnapPB->SetSearchPara(&sSearchPara);
			m_pPageSnapPB->SetWorkmode(EM_BACKUP);

			//pageSnapPlayBack  open
			SetSystemLockStatus(1);//cw_lock
			//sBizSearchParam.nMaskType = 0xf;
			//CPage** page = GetPage();//cw_rec
			((CPageDesktop*)page[EM_PAGE_DESKTOP])->SetModePlaying();
			this->Close();
			//m_pParent->Close();
			m_pPageSnapPB->SetBackupPage(this);
			//m_pPagePlayBack->SetPlayChnNum(m_nCurPlayMode);
			BizStopPreview();
			
			m_pPageSnapPB->Open();
			//BizStartPlayback(EM_BIZPLAY_TYPE_TIME,&sBizSearchParam);
			SetSystemLockStatus(0);
		}break;			
	}
	
}

void CPageDataBackup::OnBtQuit()
{
	this->Close();
}

void CPageDataBackup::OnClickChooseChn()
{
	int i, nEnd, nStart;

	CCheckBox* pFocus = (CCheckBox*)GetFocusItem();
	int val = pFocus->GetValue();
	
	if(pFocus==pChkChnLeft[TAB_LINES - 1])
	{
		/*
		for (i = 0 ; i < m_nFiles; i++)
		{
			m_nSelect[i] = val;
		}
		*/

		nStart = m_nCurPage * m_nPageSize;		
		nEnd = nStart + m_nPageSize;
		nEnd = nEnd > m_nFiles ? m_nFiles : nEnd;
		for (i = nStart; i < nEnd; i++)
		{
			m_nSelect[i] = val;
			pChkChnLeft[i - nStart]->SetValue(val);
		}
	}
	else
	{
		for(i=0 ;i<TAB_LINES - 1; i++)
		{
			if(pFocus == pChkChnLeft[i])
			{
				break;
			}
		}

		if (i < TAB_LINES - 1)
		{
			nEnd = m_nCurPage * m_nPageSize + i;
			if (nEnd < m_nFiles)
			{
				m_nSelect[nEnd] = val;
			}
		}
	}
}

void CPageDataBackup::OnCalendarAllChn()
{
	int 	i 		= 0, j;
	BOOL 	bFind 	= FALSE;
	
	CCheckBox* pFocus = (CCheckBox *)GetFocusItem();
	for(i = 0; i < (nChMax+3)/4; i++)
	{
		if(pFocus == pChkChn[i*5])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		for(j=1; j<5; j++)
		{
			if(i*4+j > nChMax) break;
			
			{
				pChkChn[i*5+j]->SetValue(pChkChn[i*5]->GetValue());
			}
		}
	}
	
	return;
}

void CPageDataBackup::OnCalendarChn()
{
	int i, j;
	BOOL bFind = FALSE;
	CCheckBox *pFocusButton = (CCheckBox *)GetFocusItem();
	for(j=0; j<(nChMax+3)/4; j++)
	{
		for(i = 1; i < 5; i++)
		{
			//csp modify
			if(j*4+i > nChMax) break;
			
			if(pFocusButton == pChkChn[j*5+i])
			{
				//csp modify 20130504
				pChkChn[j*5]->SetValue(0);
				
				bFind = TRUE;
				break;
			}
		}
		
		if(bFind) break;
	}
	
	if(!bFind)
	{
		return;
	}
	
	return;
}


void CPageDataBackup::SetProg(int curpage, int totalpage)
{
	char tmp[10] = {0};
	sprintf(tmp, "%d/%d", curpage,totalpage);
	pszResult->SetText(tmp);
	
	return;
}

void CPageDataBackup::SetInfo(char* szInfo)
{
	pInfoBar->SetText(szInfo);

}

void CPageDataBackup::ClearInfo()
{
	pInfoBar->SetText("");

}

VD_BOOL CPageDataBackup::MsgProc( uint msg, uint wpa, uint lpa )
{
	int px,py;
	int temp;
	static int chSel=0;
	switch(msg)
	{
		case XM_KEYDOWN://cw_tab
		{
			CPageFrame::MsgProc(msg, wpa, lpa);
			CItem*tmp=GetSelectItem();
			if (tmp!=NULL)
			{
				for(int i=0;i<9;i++)
				{
					if (tmp==pChkChnLeft[i])
					{
						MouseMoveToLine(i+1);
						break;
					}
					else
						MouseMoveToLine(-1);
				}
			}
			return TRUE;
		}break;
		case XM_MOUSEMOVE:
		{
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);
			VD_RECT rtScreen;
			m_pDevGraphics->GetRect(&rtScreen);
			int offsetx = (rtScreen.right - m_Rect.Width())/2;
			int offsety = (rtScreen.bottom - m_Rect.Height())/2;
			
			BOOL bFind = FALSE;
			int i = 0;
			CTableBox* table = pTabPageLeft[0];
			for(i = 1; i<TAB_LINES; i++)
			{
				for (int kk=0; kk<4; kk++)
				{
					VD_RECT rt;
					table->GetTableRect(kk, i, &rt);
					rt.left +=10+offsetx;
					rt.top +=FRAME_TOP+PAGE_TOP_HEIGHT+offsety+CTRL_HEIGHT;
					rt.right +=10+offsetx;
					rt.bottom +=FRAME_TOP+PAGE_TOP_HEIGHT+offsety+CTRL_HEIGHT;
					if (PtInRect(&rt, px, py))
					{
						bFind = TRUE;
						goto SELITEM;
					}
				}
			}

SELITEM:
			if (bFind)
			{
				MouseMoveToLine(i);
			}
			else
			{
				MouseMoveToLine(0);
			}
		}
		break;
	default:
		break;
	}
	
	return CPageFrame::MsgProc(msg, wpa, lpa);
}

void CPageDataBackup::MouseMoveToLine( int index )
{
	if(index == m_nCurMouseLine)
	{
		return;
	}
	if((m_nCurMouseLine>0) && (strcmp(pChn[m_nCurMouseLine-1]->GetText(),"") != 0))
	{
		pChn[m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
		pBegin[m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
		pEnd[m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
	}
	m_nCurMouseLine = index;
	if (index < 1 || index>9 )
	{
		return;
	}

	if( strcmp(pChn[index-1]->GetText(),"") == 0)
	{
		return ;
	}

	pChn[m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
	pBegin[m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
	pEnd[m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
}



