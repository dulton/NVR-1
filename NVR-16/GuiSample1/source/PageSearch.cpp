#include "GUI/Pages/PageSearch.h"
#include "biz.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMessageBox.h"
#include "System/Locales.h"
#include "GUI/Pages/PageDesktop.h"//cw_panel
#include "GUI/Pages/PageMainFrameWork.h"
#include "sg_platform.h"


#define CHM	8
	
static	std::vector<CItem*>	items[SEARCH_SUBPAGES];
static	std::vector<CItem*>	itemsRight[2];

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static VD_BITMAP * pBmp_tab_normal;
static VD_BITMAP * pBmp_tab_push;
static VD_BITMAP * pBmp_tab_select;

#define LEFT_PAGE_L	5
#define LEFT_PAGE_R	(m_Rect.Width()-180-25)
#define RIGHT_PAGE_L (m_Rect.Width()-180-27)
#define RIGHT_PAGE_R (m_Rect.Width()-5)

#define RIGHT_PAGE_WIDTH	(RIGHT_PAGE_R-RIGHT_PAGE_L)
#define LEFT_PAGE_WIDTH		(LEFT_PAGE_R-LEFT_PAGE_L)

static char* szBtBmpPathRecTime[4][2] = {
	{DATA_DIR"/temp/listex_btn_leftmost.bmp", DATA_DIR"/temp/listex_btn_leftmost_f.bmp"},
	{DATA_DIR"/temp/listex_btn_left.bmp", DATA_DIR"/temp/listex_btn_left_f.bmp"},
	{DATA_DIR"/temp/listex_btn_right.bmp", DATA_DIR"/temp/listex_right_f.bmp"},
	{DATA_DIR"/temp/listex_btn_rightmost.bmp", DATA_DIR"/temp/listex_btn_rightmost_f.bmp"},
};

static VD_BITMAP* pBmpBtPageCtrl[4][2];

static int GetMaxChnNum() { return GetVideoMainNum(); }

CPageSearch::CPageSearch( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
{
	nChMax = GetMaxChnNum();
	
	m_mCurID = 0;
	m_page2 = 0;
    bFromMainBoard = FALSE;
	
	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);
	
	//pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/button_normal.bmp");
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//pBmp_button_push = VD_LoadBitmap(DATA_DIR"/button_push.bmp");
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//pBmp_button_select = VD_LoadBitmap(DATA_DIR"/button_select.bmp");
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_tab_normal = VD_LoadBitmap(DATA_DIR"/temp/tab.bmp");
	pBmp_tab_push = VD_LoadBitmap(DATA_DIR"/temp/tab_check.bmp");
	pBmp_tab_select = VD_LoadBitmap(DATA_DIR"/temp/tab_f.bmp");
	
	//第一个页面的子页面控制按钮  录像设置，手动录像，录像布防
	char* szSubPgName[SEARCH_COMMBUTTON] = {
		"&CfgPtn.TimeSearch",
		"&CfgPtn.TypeSearch",
		/*"&CfgPtn.EventSearch",*/	
		"&CfgPtn.File",
		"&CfgPtn.Image",
		"&CfgPtn.StartPlay",//csp modify
		"&CfgPtn.Search",
	};
	
	int szSubPgNameLen[SEARCH_COMMBUTTON] = {
		TEXT_WIDTH*4,//"&CfgPtn.TimeSearch",
		TEXT_WIDTH*4,//"&CfgPtn.EventSearch",	
		TEXT_WIDTH*2,//"&CfgPtn.FileManagement",
		TEXT_WIDTH*3,//"&CfgPtn.Image",
		TEXT_WIDTH*2//"&CfgPtn.Search",
	};
	
	pTabFrame1 = CreateTableBox(CRect(LEFT_PAGE_L, 10+pBmp_tab_normal->height, LEFT_PAGE_R, m_Rect.Height()-50),this,1,1);
	pTabFrame1->SetFrameColor(VD_RGB(56,108,148));
	
	pTabFrame2 = CreateTableBox(CRect(RIGHT_PAGE_L, 10+pBmp_tab_normal->height, RIGHT_PAGE_R, m_Rect.Height()-50),this,1,1);
	pTabFrame2->SetFrameColor(VD_RGB(56,108,148));
	
	CRect rtSub(25, 10, m_Rect.Width()-20, 10+pBmp_tab_normal->height);
	
	int i;
	for(i = 0; i<SEARCH_SUBPAGES; i++)
	{
		if(i > 3 ||i==2)//yzw 不初始化"&CfgPtn.File",
		{
			pButton[i] = NULL;//csp modify
			continue;
		}

		if (i == 3)
		{
			#ifdef NO_MODULE_SG
				pButton[i] = NULL;//csp modify
				continue;
			#endif
		}
		
		int btWidth = szSubPgNameLen[i]+10;
		rtSub.right = rtSub.left + btWidth;
		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageSearch::OnClickSubPage, NULL, buttonNormalBmp, 1);
		pButton[i]->SetBitmap(pBmp_tab_normal, pBmp_tab_select, pBmp_tab_push, pBmp_tab_push);
		rtSub.left = rtSub.right;
	}
	
	rtSub.right = m_Rect.Width()-15;
	rtSub.left = rtSub.right - (szSubPgNameLen[0]+10);
	#if 1//csp modify
	int offset = rtSub.Width()+2;
	//int offset = 40+4;
	rtSub.left -= offset;
	rtSub.right -= offset;
	#endif
	//搜索
	rtSub.bottom = rtSub.top + pBmp_button_normal->height;
	pButton[SEARCH_COMMBUTTON-1] = CreateButton(rtSub, this, szSubPgName[SEARCH_COMMBUTTON-1], (CTRLPROC)&CPageSearch::OnClickSubPage, NULL, buttonNormalBmp);
	pButton[SEARCH_COMMBUTTON-1]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);

	//开始播放
	#if 1//csp modify
	rtSub.left += offset;
	rtSub.right += offset;
	//rtSub.right =  rtSub.left + 40;
	//rtSub.bottom = rtSub.top + 40;
	pButton[SEARCH_COMMBUTTON-2] = CreateButton(rtSub, this, szSubPgName[SEARCH_COMMBUTTON-2], (CTRLPROC)&CPageSearch::OnClickSubPage, NULL, buttonNormalBmp);
	pButton[SEARCH_COMMBUTTON-2]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);
	#endif
	
	InitCalendar();
	//printf("%s yg 1\n", __func__);
	InitPage0();//按时间
	InitPage1();//按类型
	//printf("%s yg 2\n", __func__);
	InitPage3();//图片
	//printf("%s yg 3\n", __func__);
	//printf("file:%s, function:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);
	
	#if 0//csp modify
	InitPage2();
	//printf("file:%s, function:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);
	#endif
	
	//InitPage3();
	//printf("file:%s, function:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);
	
	ShowSubPage(0,TRUE);
	
	pButton[0]->Enable(FALSE);
	//printf("%s yg 4\n", __func__);
}

CPageSearch::~CPageSearch()
{
	
}

void CPageSearch::SwitchPage( int subID )
{
	if(subID<0 || subID>=SEARCH_SUBPAGES)
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
	
	if(m_mCurID == 0 || m_mCurID == 3)
	{
		ShowSearchButton(0);
	}
	else
	{
		ShowSearchButton(1);
	}
}

void CPageSearch::ShowSubPage( int subID, BOOL bShow )
{
	if(subID<0 || subID>=SEARCH_SUBPAGES)
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
	
	//printf("ShowSubPage end\n");
}

void CPageSearch::OnClickSubPage()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < SEARCH_SUBPAGES; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	// 搜索
	//csp modify
	//if(pFocusButton==pButton[SEARCH_SUBPAGES])
	if(pFocusButton==pButton[SEARCH_COMMBUTTON-1])
	{
		switch(m_mCurID)
		{
			case 0://time
				break;
			case 1://event
				{
					for(int i  = 0; i<8;i++)
					{
						ClearLineOfPage1(i);
					}
                    
                    m_sSearchTypeStatus.nTimer = pChkPage1[0]->GetValue();
                    m_sSearchTypeStatus.nMoving = pChkPage1[1]->GetValue();
                    m_sSearchTypeStatus.nSensoring = pChkPage1[2]->GetValue();
                    m_sSearchTypeStatus.nManual = pChkPage1[3]->GetValue();
                    m_sSearchTypeStatus.nAll = pChkPage1[4]->GetValue();
                    
					SearchFilesWithEvent();
				}
				break;
			case 2://file
				{
					ClearPage2();
					SearchFilesWithFile();
				}
				break;
			//yaogang modify 20140105
			case 3://snap
				{
					//ClearPage3();
					OnClickSearchSnap();
				}
				break;
			default:
				break;
		}
	}
	
	// 回放
	//csp modify
	if(pFocusButton == pButton[SEARCH_COMMBUTTON-2])
	{
		StartPlay2();
		return;
	}
	
	if(bFind)
	{
		//printf("find the focus button %d\n", i);
		SwitchPage(i);
		
	/*	if (3 == i)
		{
			SwitchCalendar(1);
		}
		else
		{
			SwitchCalendar(0);
		}
	*/
	}
}

void CPageSearch::InitPage0()
#if 1
{
	int i=0;
	CRect rt;
	char* szBtBmpPath[3][4] = {
		{DATA_DIR"/temp/search_1x1.bmp", DATA_DIR"/temp/search_1x1_f.bmp", DATA_DIR"/temp/search_1x1_n.bmp", DATA_DIR"/temp/search_1x1_sel.bmp"},
		{DATA_DIR"/temp/search_2x2.bmp", DATA_DIR"/temp/search_2x2_f.bmp", DATA_DIR"/temp/search_2x2_n.bmp", DATA_DIR"/temp/search_2x2_sel.bmp"},
		{DATA_DIR"/temp/search_play.bmp", DATA_DIR"/temp/search_play_f.bmp", DATA_DIR"/temp/search_play.bmp", DATA_DIR"/temp/search_play_f.bmp"},
		};
	
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
	pTextPG0BeginTime = CreateStatic(rt,this, "&CfgPtn.StartTime");
	rt.left = rt.right+10;
	rt.right =	rt.left+110;
	rt.bottom = rt.top + CTRL_HEIGHT;
	pBeginDateP0 = CreateDateTimeCtrl(rt, this, NULL, DTS_DATE);
	rt.left = rt.right+10;
	rt.right =	rt.left+110;
	pBeginTimeP0 = CreateDateTimeCtrl(rt, this, NULL, DTS_TIME);
	
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
	pTextPG0EndTime = CreateStatic(rt,this, "&CfgPtn.EndTime");
	rt.left = rt.right+10;
	rt.right =	rt.left+110;
	rt.bottom = rt.top + CTRL_HEIGHT;
	pEndDateP0 = CreateDateTimeCtrl(rt, this, NULL, DTS_DATE);
	rt.left = rt.right+10;
	rt.right =	rt.left+110;
	pEndTimeP0 = CreateDateTimeCtrl(rt, this, NULL, DTS_TIME);
	
#if 1
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
	pTextPG0Chn = CreateStatic(rt,this, "&CfgPtn.SelectPlayChn");
	
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
	pTabPG0Chn = CreateTableBox(rt, this, 2, nChMax/4+((nChMax%4)?1:0));
	pTabPG0Chn->SetColWidth(0,30);
	items[0].push_back(pTextPG0Chn);
	items[0].push_back(pTabPG0Chn);
	
	CRect rect;	
	char tmp[5] = {0};
	for(int i = 0; i < (nChMax/4+((nChMax%4)?1:0)); i++)
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
		pPG0ChkChn[i*5] = CreateCheckBox(rect, this, styleEditable, (CTRLPROC)&CPageSearch::OnPG0AllChn);
		pPG0ChkChn[i*5]->SetValue(0);
		items[0].push_back(pPG0ChkChn[i*5]);
		
		rect.left = rt.left + 43;
		int left = rect.left;
		for(int j = 0; j < 4; j++)
		{
			if(i*4+j+1>nChMax)
			{
				break;
			}
			rect.left = left+j*45;
			rect.right = rect.left+22;
			pPG0ChkChn[i*5+j+1] = CreateCheckBox(rect, this, styleEditable, (CTRLPROC)&CPageSearch::OnPG0Chn);
			pPG0ChkChn[i*5+j+1]->SetValue(0);
			sprintf(tmp, "%d", i*4+j+1);
			pPG0StaticChn[i*4+j] = CreateStatic(CRect(rect.right+2,rect.top,rect.right+21,rect.bottom-4), this, tmp);
			pPG0StaticChn[i*4+j]->SetTextAlign(VD_TA_LEFT);
			items[0].push_back(pPG0ChkChn[i*5+j+1]);
			items[0].push_back(pPG0StaticChn[i*4+j]);
		}
	}
#else
	rt.left = LEFT_PAGE_L + 20;
	rt.top = 15 + rt.bottom;
	rt.right =	rt.left+100;
	rt.bottom = rt.top + TEXT_HEIGHT;
	pTextPG0WndMode = CreateStatic(rt,this, "&CfgPtn.SelectMode");
	rt.left = rt.right+10;
	rt.top -= 8;
	rt.right =  rt.left+40;
	rt.bottom = rt.top + 40;
	pBtPage0Wnd1x1 = CreateButton(rt, this, NULL, (CTRLPROC)&CPageSearch::OnClickWnd1x1, NULL, buttonNormalBmp);
	pBtPage0Wnd1x1->SetBitmap(VD_LoadBitmap(szBtBmpPath[0][0]), VD_LoadBitmap(szBtBmpPath[0][1]),VD_LoadBitmap(szBtBmpPath[0][3]),VD_LoadBitmap(szBtBmpPath[0][2]));

	m_pDevGraphics = CDevGraphics::instance(m_screen);
	m_pDevGraphics->GetRect(&m_RectScreen);
	u32 nLeftGap, nTopGap;
	nLeftGap = (m_RectScreen.right-m_Rect.Width())>>1;
	nTopGap = (m_RectScreen.bottom-m_Rect.Height())>>1;
	CRect 	rtFloat;
	VD_RECT rtGragh;
	pBtPage0Wnd1x1->GetRect(&rtGragh);
	rtFloat.left 	= nLeftGap+rtGragh.left;
	rtFloat.top 	= nTopGap+rtGragh.bottom+4;
	rtFloat.right 	= rtFloat.left+190;
	rtFloat.bottom 	= rtFloat.top+60;
	pPageChnSel = new CPageChnSelect(rtFloat, NULL,NULL,this);

	rt.left = rt.right+10;
	rt.right =  rt.left+40;
	pBtPage0Wnd2x2 = CreateButton(rt, this, NULL, (CTRLPROC)&CPageSearch::OnClickWnd2x2, NULL, buttonNormalBmp);
	pBtPage0Wnd2x2->SetBitmap(VD_LoadBitmap(szBtBmpPath[1][0]), VD_LoadBitmap(szBtBmpPath[1][1]),VD_LoadBitmap(szBtBmpPath[1][3]),VD_LoadBitmap(szBtBmpPath[1][2]));

	pBtPage0Wnd2x2->GetRect(&rtGragh);
	rtFloat.left 	= nLeftGap + rtGragh.left;
	rtFloat.top 	= nTopGap + rtGragh.bottom+4;
	rtFloat.right 	= rtFloat.left+215;
	rtFloat.bottom 	= rtFloat.top+60;
	pPageChnSel2X2 = new CPageChnSelect(rtFloat, NULL,NULL,this, 1);
	
	items[0].push_back(pTextPG0WndMode);
	items[0].push_back(pBtPage0Wnd1x1);
	items[0].push_back(pBtPage0Wnd2x2);
#endif
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
	pTextPG0StartPlay = CreateStatic(rt,this, "&CfgPtn.StartPlay");
	rt.left = rt.right+10;
	rt.left -= 4;//csp modify 20131018
	rt.top -= 8;
	rt.right =  rt.left + 40;
	rt.bottom = rt.top + 40;
	//printf("StartPlay rect:(%d,%d,%d,%d)\n",rt.left,rt.right,rt.top,rt.bottom);
	pBtPage0StartPlay = CreateButton(rt, this, NULL, (CTRLPROC)&CPageSearch::OnClickStartPlay, NULL, buttonNormalBmp);
	pBtPage0StartPlay->SetBitmap(VD_LoadBitmap(szBtBmpPath[2][0]), VD_LoadBitmap(szBtBmpPath[2][1]),VD_LoadBitmap(szBtBmpPath[2][3]),VD_LoadBitmap(szBtBmpPath[2][2]));
	
	items[0].push_back(pTextPG0StartPlay);
	items[0].push_back(pBtPage0StartPlay);
	items[0].push_back(pTextPG0BeginTime);
	items[0].push_back(pBeginDateP0);
	items[0].push_back(pBeginTimeP0);
	items[0].push_back(pTextPG0EndTime);
	items[0].push_back(pEndDateP0);
	items[0].push_back(pEndTimeP0);
	ShowSubPage(0, FALSE);
	m_nCurPlayMode = 0;
	
	m_nFlagHaveMaxPlayChn = 0;
	
	char tmp2[20] = {0};
	GetProductNumber(tmp2);
	if((0 == strcasecmp(tmp2, "R9624T"))
		|| (0 == strcasecmp(tmp2, "R9632S"))
		 || (0 == strcasecmp(tmp2, "R9616S"))
		  || (0 == strcasecmp(tmp2, "R9624SL"))
		   || (0 == strcasecmp(tmp2, "R9608S")))
	{
		m_nFlagHaveMaxPlayChn = 1;
		m_nMaxPlayChn = 4;
	}
	else if((0 == strcasecmp(tmp2, "R3104HD"))
			|| (0 == strcasecmp(tmp2, "R3110HDW"))
			|| (0 == strcasecmp(tmp2, "R3106HDW")))
	{
		m_nFlagHaveMaxPlayChn = 1;
		//csp modify
		//m_nMaxPlayChn = 1;
		m_nMaxPlayChn = 4;
	}
	else//cw_9508S
	{
		m_nFlagHaveMaxPlayChn = 1;
		m_nMaxPlayChn = 4;
	}
}
#else
{
	int i=0;
	CRect rt;
	rt.left = LEFT_PAGE_L + 10;
	rt.top = 14+pBmp_tab_normal->height+CTRL_HEIGHT*2;
	rt.right =  LEFT_PAGE_R -10;
	rt.bottom = m_Rect.Height()-60 - 30;
	
	pRecTimeBox = CreateRecTimeBox(rt, this, (CTRLPROC)&CPageSearch::OnClickTrackPage0);
 	
 	items[0].push_back(pRecTimeBox);
	
	nCurRecTimePg = 0;
	nRecTimePgNum = (nChMax+CHN_PER_PAGE-1)/CHN_PER_PAGE;
	nLastPgChnNum = nChMax%CHN_PER_PAGE;
	
#if 1
	char test[10] = {0};
	for (i=0; i<nLastPgChnNum; i++)
	{
		sprintf(test, "%d", i+1);
		
		realIdx[i] = pRecTimeBox->InsertItem(i, test);
		
		//RecTimeData data;
		//data.starttime = i*3600+900;
		//data.endtime = (i+5)*3600+2700;
		
		//pRecTimeBox->AddItemTimeData(realidx, &data);
		
		//data.starttime = data.endtime + 7200;
		//data.endtime = data.starttime + 5*3600+2400;
		
		//pRecTimeBox->AddItemTimeData(realidx, &data);
		nRecTimeChnNum++;
	}
#endif
	
	if(nChMax > CHN_PER_PAGE)
	{
		rt.top =  m_Rect.Height()-60 - 20;
		rt.left = LEFT_PAGE_R - 40*4 - 10*4;
		rt.right = rt.left + 40*4 - 10*4;
		rt.bottom = rt.top + 24;
		
		for (i =0; i<4; i++)
		{
	 		pBmpBtPageCtrl[i][0] = VD_LoadBitmap(szBtBmpPathRecTime[i][0]);
	 		pBmpBtPageCtrl[i][1] = VD_LoadBitmap(szBtBmpPathRecTime[i][1]);
			pBtPageCtr[i] = CreateButton(CRect(rt.left, rt.top, rt.left+40, rt.bottom), this, NULL, (CTRLPROC)&CPageSearch::OnClickPageCtr, NULL, buttonNormalBmp);
			rt.left += 40+10;
			pBtPageCtr[i]->SetBitmap(pBmpBtPageCtrl[i][0], pBmpBtPageCtrl[i][1], pBmpBtPageCtrl[i][1]);
			
 			items[0].push_back(pBtPageCtr[i]);
		}
	}
	
	char* szBtBmpPath[2][4] = {
	char* szBtBmpPath[3][4] = {
		{DATA_DIR"/temp/search_1x1.bmp", DATA_DIR"/temp/search_1x1_f.bmp", DATA_DIR"/temp/search_1x1_n.bmp", DATA_DIR"/temp/search_1x1_sel.bmp"},
		{DATA_DIR"/temp/search_2x2.bmp", DATA_DIR"/temp/search_2x2_f.bmp", DATA_DIR"/temp/search_2x2_n.bmp", DATA_DIR"/temp/search_2x2_sel.bmp"},
		{DATA_DIR"/temp/search_play.bmp", DATA_DIR"/temp/search_play_f.bmp", DATA_DIR"/temp/search_play.bmp", DATA_DIR"/temp/search_play_f.bmp"},
		};

	rt.left = LEFT_PAGE_L + 10;
	rt.top = 14+pBmp_tab_normal->height+8;
	rt.right =  rt.left+40;
	rt.bottom = rt.top + 40;
	pBtPage0Wnd1x1 = CreateButton(rt, this, NULL, (CTRLPROC)&CPageSearch::OnClickWnd1x1, NULL, buttonNormalBmp);
	pBtPage0Wnd1x1->SetBitmap(VD_LoadBitmap(szBtBmpPath[0][0]), VD_LoadBitmap(szBtBmpPath[0][1]),VD_LoadBitmap(szBtBmpPath[0][3]),VD_LoadBitmap(szBtBmpPath[0][2]));

	BIZ_DT_DBG("CPageSearch::InitPage0\n");

	m_pDevGraphics = CDevGraphics::instance(m_screen);
	m_pDevGraphics->GetRect(&m_RectScreen);

	u32 nLeftGap, nTopGap;

	nLeftGap = (m_RectScreen.right-m_Rect.Width())>>1;
	nTopGap = (m_RectScreen.bottom-m_Rect.Height())>>1;

	CRect 	rtFloat;
	VD_RECT rtGragh;
	pBtPage0Wnd1x1->GetRect(&rtGragh);

	rtFloat.left 	= nLeftGap+rtGragh.left;
	rtFloat.top 	= nTopGap+rtGragh.bottom+4;
	rtFloat.right 	= rtFloat.left+190;
	rtFloat.bottom 	= rtFloat.top+60;

	BIZ_DT_DBG("CPageSearch::InitPage0 1\n");

	pPageChnSel = new CPageChnSelect(rtFloat, NULL,NULL,this);

	rt.left = rt.right+6;
	rt.right =  rt.left+40;
	pBtPage0Wnd2x2 = CreateButton(rt, this, NULL, (CTRLPROC)&CPageSearch::OnClickWnd2x2, NULL, buttonNormalBmp);
	pBtPage0Wnd2x2->SetBitmap(VD_LoadBitmap(szBtBmpPath[1][0]), VD_LoadBitmap(szBtBmpPath[1][1]),VD_LoadBitmap(szBtBmpPath[1][3]),VD_LoadBitmap(szBtBmpPath[1][2]));

	pBtPage0Wnd2x2->GetRect(&rtGragh);
	rtFloat.left 	= nLeftGap + rtGragh.left;
	rtFloat.top 	= nTopGap + rtGragh.bottom+4;
	rtFloat.right 	= rtFloat.left+215;
	rtFloat.bottom 	= rtFloat.top+60;
	BIZ_DT_DBG("CPageSearch::InitPage0 2\n");

	pPageChnSel2X2 = new CPageChnSelect(rtFloat, NULL,NULL,this, 1);
	BIZ_DT_DBG("CPageSearch::InitPage0 3\n");

	rt.left = rt.right+6;
	rt.right =  rt.left+40;
#if 1
	pBtPage0StartPlay = CreateButton(rt, this, NULL, (CTRLPROC)&CPageSearch::OnClickStartPlay, NULL, buttonNormalBmp);
	pBtPage0StartPlay->SetBitmap(VD_LoadBitmap(szBtBmpPath[2][0]), VD_LoadBitmap(szBtBmpPath[2][1]),VD_LoadBitmap(szBtBmpPath[2][3]),VD_LoadBitmap(szBtBmpPath[2][2]));
	//pBtPage0StartPlay->Enable(FALSE);
#endif
	// add by spliang for debug
	//
	rtFloat.left = 0;
	rtFloat.right = m_Rect.Width();
	rtFloat.top = m_Rect.bottom - 100;
	rtFloat.bottom = rtFloat.top+100;
	//m_pPagePlayBack = new CPagePlayBackFrameWork(rtFloat, NULL,NULL,this);
	// ===
	
	rt.left = rt.right+50;
	rt.top = 14+pBmp_tab_normal->height;
	rt.right =  rt.left+100;
	rt.bottom = rt.top + TEXT_HEIGHT;
	pTextPG0BeginTime = CreateStatic(rt,this, "&CfgPtn.StartTime");

	rt.top = 14+pBmp_tab_normal->height+TEXT_HEIGHT;
	rt.right =  rt.left+110;
	rt.bottom = rt.top + CTRL_HEIGHT;
	pBeginDateP0 = CreateDateTimeCtrl(rt, this, NULL, DTS_DATE);

	rt.left = rt.right+10;
	rt.right =  rt.left+110;
	pBeginTimeP0 = CreateDateTimeCtrl(rt, this, NULL, DTS_TIME);

	items[0].push_back(pBtPage0Wnd1x1);
	items[0].push_back(pBtPage0Wnd2x2);
	items[0].push_back(pBtPage0StartPlay);
	items[0].push_back(pTextPG0BeginTime);
	items[0].push_back(pBeginDateP0);
	items[0].push_back(pBeginTimeP0);

	ShowSubPage(0, FALSE);
	m_nCurPlayMode = 0;
}
#endif

void CPageSearch::RefreshRecTime()
{
	pRecTimeBox->Draw();
	
	// show channel 
	

	// show record time 
}

void CPageSearch::OnClickPageCtr()
{
	int i = 0;
	BOOL bFind = FALSE;
	char szTmp[4] = {0};
	
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < 4; i++)
	{
		if (pFocusButton == pBtPageCtr[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if (bFind)
	{
		pRecTimeBox->RemoveAllItem(TRUE);
		
		int nLastRecTimePg = nCurRecTimePg;
		switch(i)
		{
			case 0:
			{
				nCurRecTimePg = 0;
			} break;
			case 1:
			{
				nCurRecTimePg -= 1;
				if(nCurRecTimePg < 0)
				{
					nCurRecTimePg = nRecTimePgNum-1;
				}
			} break;
			case 2:
			{
				nCurRecTimePg += 1;
				if(nCurRecTimePg >= nRecTimePgNum)
				{
					nCurRecTimePg = 0;
				}
			} break;
			case 3:
			{
				nCurRecTimePg = nRecTimePgNum-1;
			} break;
		}

		int nChStart = nCurRecTimePg*CHN_PER_PAGE;
		int nChNum;

		nChNum = CHN_PER_PAGE;
		if(nCurRecTimePg == nRecTimePgNum-1)
		{
			nChNum = nLastPgChnNum;
		}

		printf("$$$btn %d nChStart %d\n", i, nChStart);
		
		for(int j=0; j<nChNum; j++)
		{
			sprintf(szTmp, "%d", j+nChStart+1);
			realIdx[j] = pRecTimeBox->InsertItem(j, szTmp);
		}

		RefreshRecTime();
	}
}

void CPageSearch::InitPage1()
{
	CRect rt;
	rt.left = LEFT_PAGE_L + 4;
	rt.top = 14+pBmp_tab_normal->height;
	rt.right =  LEFT_PAGE_R -4;
	rt.bottom = rt.top + (CTRL_HEIGHT-2)*9;
	pTabPage1[0] = CreateTableBox(rt, this, 4, 9);
	pTabPage1[0]->SetColWidth(0, TEXT_WIDTH*2-2);
	pTabPage1[0]->SetColWidth(1, TEXT_WIDTH*3-10-2-10);
	pTabPage1[0]->SetColWidth(2, 152+5+2+5);
	
	//printf("pTabPage1[0] %x\n", pTabPage1[0]);
	
	items[1].push_back(pTabPage1[0]);
	
	char* szTitle[4] = {
		"&CfgPtn.Channel", 
		"&CfgPtn.Type", 
		"&CfgPtn.StartTime", 
		"&CfgPtn.EndTime"
	};
	
	int i = 0;
	for(i=0; i<4; i++)
	{
		CRect rtTemp;
		pTabPage1[0]->GetTableRect(i, 0, &rtTemp);
		
		pTitlePage1[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), this, szTitle[i]);
		pTitlePage1[i]->SetBkColor(VD_RGB(67,77,87));
		pTitlePage1[i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		pTitlePage1[i]->SetTextAlign(VD_TA_XLEFT);
		//pTitlePage1[i]->SetTextAlign(VD_TA_LEFT);
		
		items[1].push_back(pTitlePage1[i]);
	}
	
	//printf("file:%s, function:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);
	
	for(i=0; i<CHM; i++)
	{
		CRect rtTemp;
		pTabPage1[0]->GetTableRect(0, i+1, &rtTemp);
		pChn1[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), this, "");
		items[1].push_back(pChn1[i]);
	}
	
	//printf("file:%s, function:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);
	
	for(i=0; i<CHM; i++)
	{
		CRect rtTemp;
		pTabPage1[0]->GetTableRect(1, i+1, &rtTemp);
		pType1[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), this, "");
		items[1].push_back(pType1[i]);
	}
	
	//printf("file:%s, function:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);
	
	for(i=0; i<CHM; i++)
	{
		CRect rtTemp;
		pTabPage1[0]->GetTableRect(2, i+1, &rtTemp);
		pBegin1[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), this, "");
		items[1].push_back(pBegin1[i]);
	}
	
	//printf("file:%s, function:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);
	
	for(i=0; i<CHM; i++)
	{
		CRect rtTemp;
		pTabPage1[0]->GetTableRect(3, i+1, &rtTemp);
		pEnd1[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), this, "");
		items[1].push_back(pEnd1[i]);
	}
	
	//printf("file:%s, function:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);
	
	rt.top = rt.bottom-2;
	rt.bottom = rt.top + CTRL_HEIGHT;
	pTabPage1[1] = CreateTableBox(rt, this, 1, 1);
	items[1].push_back(pTabPage1[1]);
	
	char tmp[10] = {0};
	int totalpage = 0;
	int curpage = 0;
	rt.left += 3;
	rt.top += 2;
	rt.right = rt.left + TEXT_WIDTH*3;
	rt.bottom -= 2;
	sprintf(tmp, "%d/%d", curpage,totalpage);
	pszResultPage1 = CreateStatic(rt, this, tmp);
	pszResultPage1->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
	pszResultPage1->SetTextAlign(VD_TA_LEFT);
	items[1].push_back(pszResultPage1);
	rt.left = rt.right;
	rt.right = rt.left + TEXT_WIDTH*5;
	pTips = CreateStatic(rt, this, "");
	pTips->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
	pTips->SetTextAlign(VD_TA_LEFT);
	items[1].push_back(pTips);
	
	char* szBtBmpPath[4][2] = {
		{DATA_DIR"/temp/listex_btn_leftmost.bmp", DATA_DIR"/temp/listex_btn_leftmost_f.bmp"},
		{DATA_DIR"/temp/listex_btn_left.bmp", DATA_DIR"/temp/listex_btn_left_f.bmp"},
		{DATA_DIR"/temp/listex_btn_right.bmp", DATA_DIR"/temp/listex_right_f.bmp"},
		{DATA_DIR"/temp/listex_btn_rightmost.bmp", DATA_DIR"/temp/listex_btn_rightmost_f.bmp"},
		};
	
	rt.left = LEFT_PAGE_R -4 - 40*4 - 10*4;
	
	for(i =0; i<4; i++)
	{
		pBmpBtPage1[i][0] = VD_LoadBitmap(szBtBmpPath[i][0]);
		pBmpBtPage1[i][1] = VD_LoadBitmap(szBtBmpPath[i][1]);
		printf("%s rt.top: %d\n", __func__, rt.top);
		printf("%s rt.bottom: %d\n", __func__, rt.bottom);
		pBtPageCtrPage1[i] = CreateButton(CRect(rt.left, rt.top, rt.left+40, rt.bottom), this, NULL, (CTRLPROC)&CPageSearch::OnClickPageCtrPage1, NULL, buttonNormalBmp);
		rt.left += 40+10;
		pBtPageCtrPage1[i]->SetBitmap(pBmpBtPage1[i][0], pBmpBtPage1[i][1], pBmpBtPage1[i][1]);
		
		items[1].push_back(pBtPageCtrPage1[i]);
	}
	
	rt.left = 20;
	rt.right = rt.left+24;
	rt.top = rt.bottom +4;
	rt.bottom = rt.top + TEXT_HEIGHT;
	
	//printf("file:%s, function:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);
	
	char* szChkText[5] = {
	"&CfgPtn.Timer", 
	"&CfgPtn.Moving",
	"&CfgPtn.Sensoring", 
	"&CfgPtn.Manual", 
	"&CfgPtn.All"};
	for(i = 0; i<5; i++)
	{
		//printf("file:%s, function:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);
		if(i<4)
			pChkPage1[i] = CreateCheckBox(CRect(rt.left + (i>1?(i-2):i)*120, rt.top + (i>>1?1:0)*(TEXT_HEIGHT+4), rt.left+24+(i>1?(i-2):i)*120,rt.bottom + (i>>1?1:0)*(TEXT_HEIGHT+4)), this, styleEditable, (CTRLPROC)&CPageSearch::OnEventType);
		else
			pChkPage1[i] = CreateCheckBox(CRect(rt.left + (i>1?(i-2):i)*120, rt.top + (i>>1?1:0)*(TEXT_HEIGHT+4), rt.left+24+(i>1?(i-2):i)*120,rt.bottom + (i>>1?1:0)*(TEXT_HEIGHT+4)), this, styleEditable, (CTRLPROC)&CPageSearch::OnEventAllType);
		pTextChkPage1[i] = CreateStatic(CRect(rt.left+24+(i>1?(i-2):i)*120, rt.top + (i>>1?1:0)*(TEXT_HEIGHT+4), rt.left+((i>1?(i-2):i)+1)*120,rt.bottom + (i>>1?1:0)*(TEXT_HEIGHT+4)), this, szChkText[i]);
		pTextChkPage1[i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		pTextChkPage1[i]->SetTextAlign(VD_TA_LEFT);
		pChkPage1[i]->SetValue(1);
		
		items[1].push_back(pChkPage1[i]);
		items[1].push_back(pTextChkPage1[i]);
	}
	
    //Modify by Lirl on Feb/06/2012,不支持传感器报警设备
    if(!GetSenSorNum())
	{
		pChkPage1[2]->SetValue(0);
		pChkPage1[2]->Enable(FALSE);
		pTextChkPage1[2]->Enable(FALSE);
	}
    //end
	
	//printf("file:%s, function:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);
	
	ShowSubPage(1, FALSE);
	
	m_nCurMouseLine = -1;
	
	int m_chSel = 0;//cw_tab
	int bInTab = 0;
}

void CPageSearch::InitPage2()
{
	CRect rt;
	rt.left = LEFT_PAGE_L + 4;
	rt.top = 14+pBmp_tab_normal->height;
	rt.right =  LEFT_PAGE_R -4;
	rt.bottom = rt.top + CTRL_HEIGHT*9;
#ifdef SHOW_STATE
#define ROW_CNR	5
#else
#define ROW_CNR	4
#endif
	pTabPage2[0] = CreateTableBox(rt, this, ROW_CNR, 9);
	pTabPage2[0]->SetColWidth(0, 28);
	pTabPage2[0]->SetColWidth(1, TEXT_WIDTH*2);
	pTabPage2[0]->SetColWidth(2, TEXT_WIDTH*7+2);
	//pTabPage2[0]->SetColWidth(3, TEXT_WIDTH*5);
	
	items[2].push_back(pTabPage2[0]);
	
	char* szTitle[5] = {
		"",
		"&CfgPtn.Channel",
		"&CfgPtn.StartTime",
		"&CfgPtn.EndTime",
		"&CfgPtn.Status"
	};
	
	int i = 0;
	for(i=0; i<ROW_CNR; i++)
	{
		CRect rtTemp;
		pTabPage2[0]->GetTableRect(i, 0, &rtTemp);
		
		pTitlePage2[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), this, szTitle[i]);
		pTitlePage2[i]->SetBkColor(VD_RGB(67,77,87));
		pTitlePage2[i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		pTitlePage2[i]->SetTextAlign(VD_TA_XLEFT);
		//pTitlePage2[i]->SetTextAlign(VD_TA_LEFT);
		
		items[2].push_back(pTitlePage2[i]);
	}
	
	rt.top = rt.bottom-2;
	rt.bottom = rt.top + CTRL_HEIGHT;
	pTabPage2[1] = CreateTableBox(rt, this, 1, 1);
	items[2].push_back(pTabPage2[1]);
	
	char tmp[10] = {0};
	int totalpage = 0;
	int curpage = 0;
	rt.left += 3;
	rt.top += 2;
	rt.right = rt.left + TEXT_WIDTH*4;
	rt.bottom -= 2;
	sprintf(tmp, "%d/%d", curpage,totalpage);
	pszResultPage2 = CreateStatic(rt, this, tmp);
	pszResultPage2->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
	pszResultPage2->SetTextAlign(VD_TA_LEFT);
	items[2].push_back(pszResultPage2);
	
#if 1//csp modify
	char* szBtBmpPath[4][2] = {
		{DATA_DIR"/temp/listex_btn_leftmost.bmp", DATA_DIR"/temp/listex_btn_leftmost_f.bmp"},
		{DATA_DIR"/temp/listex_btn_left.bmp", DATA_DIR"/temp/listex_btn_left_f.bmp"},
		{DATA_DIR"/temp/listex_btn_right.bmp", DATA_DIR"/temp/listex_right_f.bmp"},
		{DATA_DIR"/temp/listex_btn_rightmost.bmp", DATA_DIR"/temp/listex_btn_rightmost_f.bmp"},
	};
#endif
	
	rt.left = m_Rect.Width()-RIGHT_PAGE_WIDTH - 15 -7 - 40*4 - 10*4;
	
	pChkPage2[8] = CreateCheckBox(CRect(rt.left-40, rt.top, rt.left-10, rt.bottom), this, styleEditable, (CTRLPROC)&CPageSearch::OnClickChooseChn);
	items[2].push_back(pChkPage2[8]);
	
	pszChooseAll = CreateStatic(CRect(rt.left-80, rt.top, rt.left-40, rt.bottom), this, "&CfgPtn.All");
	items[2].push_back(pszChooseAll);
	
	for(i =0; i<4; i++)
	{
		pBmpBtPage2[i][0] = VD_LoadBitmap(szBtBmpPath[i][0]);
		pBmpBtPage2[i][1] = VD_LoadBitmap(szBtBmpPath[i][1]);
		pBtPageCtrPage2[i] = CreateButton(CRect(rt.left, rt.top, rt.left+40, rt.bottom), this, NULL, (CTRLPROC)&CPageSearch::OnClickPageCtrPage2, NULL, buttonNormalBmp);
		rt.left += 40+10;
		pBtPageCtrPage2[i]->SetBitmap(pBmpBtPage2[i][0], pBmpBtPage2[i][1], pBmpBtPage2[i][1]);
		
		items[2].push_back(pBtPageCtrPage2[i]);
	}
	
#ifdef SHOW_STATE
	rt.left = 14;
	pBtDealFile[0] = CreateButton(CRect(rt.left, rt.bottom+2, rt.left +70, rt.bottom+27), this, "Del", (CTRLPROC)&CPageSearch::OnDelFilePage2, NULL, buttonNormalBmp);
	pBtDealFile[1] = CreateButton(CRect(rt.left+90, rt.bottom+2, rt.left +160, rt.bottom+27), this, "Lock", (CTRLPROC)&CPageSearch::OnLockFilePage2, NULL, buttonNormalBmp);
	
	pBtDealFile[0]->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	pBtDealFile[1]->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	items[2].push_back(pBtDealFile[0]);
	items[2].push_back(pBtDealFile[1]);
#endif
	
	rt.left = LEFT_PAGE_L + 4;
	rt.top = 14+pBmp_tab_normal->height;
	rt.right = LEFT_PAGE_R -4;
	rt.bottom = rt.top + CTRL_HEIGHT*9;
	
	for(i=0; i<CHM; i++)
	{
		CRect rtTemp;
		pTabPage2[0]->GetTableRect(1, i+1, &rtTemp);
		pChn2[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");
		items[2].push_back(pChn2[i]);
	}
	
	for(i=0; i<CHM; i++)
	{
		CRect rtTemp;
		pTabPage2[0]->GetTableRect(2, i+1, &rtTemp);
		pBegin2[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");
		items[2].push_back(pBegin2[i]);
	}
	
	for(i=0; i<CHM; i++)
	{
		CRect rtTemp;
		pTabPage2[0]->GetTableRect(3, i+1, &rtTemp);
		pEnd2[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");
		items[2].push_back(pEnd2[i]);
	}
	
#ifdef SHOW_STATE
	for(i=0; i<CHM; i++)
	{
		CRect rtTemp;
		pTabPage2[0]->GetTableRect(4, i+1, &rtTemp);
		pState2[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");
		items[2].push_back(pState2[i]);
	}
#endif
	
	//rt.left = 14;
	//rt.top = 14+pBmp_tab_normal->height+29;
	for(i = 0; i< 8; i++)
	{
		CRect rtTemp;
		pTabPage2[0]->GetTableRect(0, i+1, &rtTemp);
		pChkPage2[i] = CreateCheckBox(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top+2, rt.left+rtTemp.right,rt.top+rtTemp.bottom+2), this, styleEditable, (CTRLPROC)&CPageSearch::OnClickChooseChn);
		items[2].push_back(pChkPage2[i]);
	}
	
	//pBegin[0]->SetText("07/21/2011 21:32:20");
	
	ShowSubPage(2, FALSE);
}

void CPageSearch::InitPage3()
#if 1 //类似"按时间"子页面
{
	int i=0;
	CRect rt;
	char* szBtBmpPath[3][4] = {
		{DATA_DIR"/temp/search_1x1.bmp", DATA_DIR"/temp/search_1x1_f.bmp", DATA_DIR"/temp/search_1x1_n.bmp", DATA_DIR"/temp/search_1x1_sel.bmp"},
		{DATA_DIR"/temp/search_2x2.bmp", DATA_DIR"/temp/search_2x2_f.bmp", DATA_DIR"/temp/search_2x2_n.bmp", DATA_DIR"/temp/search_2x2_sel.bmp"},
		{DATA_DIR"/temp/search_play.bmp", DATA_DIR"/temp/search_play_f.bmp", DATA_DIR"/temp/search_play.bmp", DATA_DIR"/temp/search_play_f.bmp"},
		};
	
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
	items[3].push_back(pTextPG3Chn);
	items[3].push_back(pTabPG3Chn);
	
	CRect rect;	
	char tmp[5] = {0};
	for(int i = 0; i < (nChMax/4+((nChMax%4)?1:0)); i++)
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
		pPG3ChkChn[i*5] = CreateCheckBox(rect, this, styleEditable, (CTRLPROC)&CPageSearch::OnPG3AllChn);
		pPG3ChkChn[i*5]->SetValue(1);
		items[3].push_back(pPG3ChkChn[i*5]);
		
		rect.left = rt.left + 43;
		int left = rect.left;
		for(int j = 0; j < 4; j++)
		{
			if(i*4+j+1>nChMax)
			{
				break;
			}
			rect.left = left+j*45;
			rect.right = rect.left+22;
			pPG3ChkChn[i*5+j+1] = CreateCheckBox(rect, this, styleEditable, (CTRLPROC)&CPageSearch::OnPG3Chn);
			pPG3ChkChn[i*5+j+1]->SetValue(1);
			sprintf(tmp, "%d", i*4+j+1);
			pPG3StaticChn[i*4+j] = CreateStatic(CRect(rect.right+2,rect.top,rect.right+21,rect.bottom-4), this, tmp);
			pPG3StaticChn[i*4+j]->SetTextAlign(VD_TA_LEFT);
			items[3].push_back(pPG3ChkChn[i*5+j+1]);
			items[3].push_back(pPG3StaticChn[i*4+j]);
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
	rt.left = rt.right+10;
	rt.left -= 4;//csp modify 20131018
	rt.top -= 8;
	rt.right =  rt.left + TEXT_WIDTH*4;
	rt.bottom = rt.top + 25;
	//printf("StartPlay rect:(%d,%d,%d,%d)\n",rt.left,rt.right,rt.top,rt.bottom);

	pBtPage3Search = CreateButton(rt, this, "&CfgPtn.Search", (CTRLPROC)&CPageSearch::OnClickSearchSnap, NULL, buttonNormalBmp);
		
	pBtPage3Search->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	items[3].push_back(pBtPage3Search);
	items[3].push_back(pTextPG3BeginTime);
	items[3].push_back(pBeginDateP3);
	items[3].push_back(pBeginTimeP3);
	items[3].push_back(pTextPG3EndTime);
	items[3].push_back(pEndDateP3);
	items[3].push_back(pEndTimeP3);
	ShowSubPage(3, FALSE);

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
	
	printf("%s new CPageSnapPlayBack\n", __func__);
	m_pPageSnapPB = new CPageSnapPlayBack(rtFloat, "Snap PlayBack", NULL, this);
	SetPage(EM_PAGE_SNAP, m_pPageSnapPB);
	
}
#else //原有的图片回放子页面
{
	u32 nLeftRight = LEFT_PAGE_R;//+40;
	
	CRect rt;
	rt.left = LEFT_PAGE_L + 4;
	rt.top = 14+pBmp_tab_normal->height;
	rt.right =  nLeftRight -4;
	rt.bottom = rt.top + CTRL_HEIGHT*10;
	
	int pic_h = rt.Height();
	int pic_w = pic_h*352/288;
	int totalwidth = rt.Width();
	rt.left += (totalwidth-pic_w)/2;
	rt.right -= (totalwidth-pic_w)/2;
	
	pTabPicFrmPage3 = CreateTableBox(rt, this, 1, 1);
	
	items[3].push_back(pTabPicFrmPage3);
	
	char* szBtBmpPath[4][2] = {
		{DATA_DIR"/temp/listex_btn_leftmost.bmp", DATA_DIR"/temp/listex_btn_leftmost_f.bmp"},
		{DATA_DIR"/temp/listex_btn_left.bmp", DATA_DIR"/temp/listex_btn_left_f.bmp"},
		{DATA_DIR"/temp/listex_btn_right.bmp", DATA_DIR"/temp/listex_right_f.bmp"},
		{DATA_DIR"/temp/listex_btn_rightmost.bmp", DATA_DIR"/temp/listex_btn_rightmost_f.bmp"},
	};

#define PAGE_CTRL_BTN_WIDTH	40

	int btSpace = 1;
	rt.left = nLeftRight -4 - PAGE_CTRL_BTN_WIDTH*4 - btSpace*3;
	rt.right =  nLeftRight -4;
	rt.top = 14+pBmp_tab_normal->height+ CTRL_HEIGHT*10;
	rt.bottom = rt.top + 25;

	int i = 0;
	for (i =0; i<4; i++)
	{
		pBmpBtPage3[i][0] = VD_LoadBitmap(szBtBmpPath[i][0]);
		pBmpBtPage3[i][1] = VD_LoadBitmap(szBtBmpPath[i][1]);
		pBtPageCtrPage3[i] = CreateButton(CRect(rt.left, rt.top, rt.left+40, rt.bottom), this, NULL, (CTRLPROC)&CPageSearch::OnClickPageCtrPage3, NULL, buttonNormalBmp);
		rt.left += PAGE_CTRL_BTN_WIDTH+btSpace;
		pBtPageCtrPage3[i]->SetBitmap(pBmpBtPage3[i][0], pBmpBtPage3[i][1], pBmpBtPage3[i][1]);
		
		items[3].push_back(pBtPageCtrPage3[i]);
	}

	char* szDealPic[4] = {
	"&CfgPtn.Delete", 
	"&CfgPtn.Lock", 
	"&CfgPtn.Save", 
	"&CfgPtn.SaveAll"};
	
	int szDealPicLen[4] = {
		TEXT_WIDTH*2,//"&CfgPtn.Delete", 
		TEXT_WIDTH*2,//"&CfgPtn.Lock", 
		TEXT_WIDTH*2,//"&CfgPtn.Save", 
		TEXT_WIDTH*4 //"&CfgPtn.SaveAll"
	};
	
	rt.left = 14;
	
	for (i = 0; i<4; i++)
	{
		rt.right = rt.left+szDealPicLen[i];
		pBtDealPic[i] = CreateButton(rt, this, szDealPic[i], (CTRLPROC)&CPageSearch::OnClickDealPicPage3, NULL, buttonNormalBmp);
		rt.left = rt.right+2;
		
		pBtDealPic[i]->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
		items[3].push_back(pBtDealPic[i]);
	}
	
	ShowSubPage(3, FALSE);
}
#endif

void CPageSearch::InitCalendar()
{
	CRect rtCal(RIGHT_PAGE_L+3, 12+pBmp_tab_normal->height, RIGHT_PAGE_R-3, 12+pBmp_tab_normal->height+TEXT_HEIGHT*8+10);
	
	//BIZ_DO_DBG("CPageSearch::InitCalendar width %d\n",  RIGHT_PAGE_R+6 - RIGHT_PAGE_L);
	
	printf("Calendar rect:(%d,%d,%d,%d)\n",rtCal.left,rtCal.right,rtCal.top,rtCal.bottom);
	
	pCalendar = CreateCalendar(rtCal, this, NULL, (CTRLPROC)&CPageSearch::OnDaySelected);
	itemsRight[0].push_back(pCalendar);
	
	CRect rtTime;
	rtTime.left = rtCal.right-112;
	rtTime.right = rtCal.right-2;
	rtTime.top = rtCal.top+10;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pBeginDatePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_DATE);
	
	rtTime.top = rtTime.bottom+2;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pBeginTimePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_TIME);
	
	rtTime.top = rtTime.bottom+16;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pEndDatePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_DATE);
	
	rtTime.top = rtTime.bottom+2;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pEndTimePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_TIME);
	
	pStaticBegin = CreateStatic(CRect(rtCal.left, rtCal.top+10,rtCal.right-112,rtCal.top+34), this, "&CfgPtn.StartTime");
	pStaticBegin->SetTextAlign(VD_TA_CENTER);
	pStaticEnd = CreateStatic(CRect(rtCal.left, rtCal.top+84,rtCal.right-112,rtCal.top+108), this, "&CfgPtn.EndTime");
	pStaticEnd->SetTextAlign(VD_TA_CENTER);
	
	itemsRight[1].push_back(pBeginDatePR);
	itemsRight[1].push_back(pBeginTimePR);
	itemsRight[1].push_back(pEndDatePR);
	itemsRight[1].push_back(pEndTimePR);
	itemsRight[1].push_back(pStaticBegin);
	itemsRight[1].push_back(pStaticEnd);
	
	int nRowChn 	= 4;
	int nRow 		= (nChMax+3)/nRowChn;
	int nRemainCh 	= nChMax%nRowChn;
	int nBtnSize    = 21;//(m_Rect.Height()-60-rtCal.bottom-5)/nRow;
	int nLeftGap    = 4;
	int nBtnGap     = 0;
	
	pTabChnSearch = CreateTableBox(CRect(rtCal.left+2, rtTime.bottom+5, rtCal.right-1, rtTime.bottom+5+nBtnSize*nRow+4), this, 2, 1);
	pTabChnSearch->SetColWidth(0,nBtnSize+6);
	itemsRight[1].push_back(pTabChnSearch);
	
	int nLeftPosBase = rtCal.left+nLeftGap;
	int nTopPosBase = rtTime.bottom+5 + 2;
	
	CRect rt;
	int i, j;
	
	for(j = 0; j < 8; j++)
	{
		for(i = 0; i < 5; i++)
		{
			pChkChn[j * 5 + i] = NULL;
		}
	}
	
	for(j=0; j<nRow; j++)
	{
		rt.left = nLeftPosBase;
		rt.right = rt.left + nBtnSize + nBtnGap;
		rt.top = nTopPosBase + j*nBtnSize;
		rt.bottom = rt.top + nBtnSize;
		pChkChn[j*(nRowChn+1)] = CreateCheckBox(rt, this, styleEditable, (CTRLPROC)&CPageSearch::OnCalendarAllChn);
		pChkChn[j*(nRowChn+1)]->SetValue(1);
		itemsRight[1].push_back(pChkChn[j*(nRowChn+1)]);
		
		rt.left = rt.right + nBtnGap + 10;
		rt.right = rt.left + nBtnSize + nBtnGap;
		
		for(i=1; i<5; i++)
		{
			if(j*nRowChn+i>nChMax)
			{
				break;
			}
			
			pChkChn[j*(nRowChn+1)+i] = CreateCheckBox(rt, this, styleEditable, (CTRLPROC)&CPageSearch::OnCalendarChn);
			pChkChn[j*(nRowChn+1)+i]->SetValue(1);
			itemsRight[1].push_back(pChkChn[j*(nRowChn+1)+i]);
			
			rt.left = rt.right+2;
			rt.right = rt.left + 16;
			char tmp[5] = {0};
			sprintf(tmp, "%d", i+j*nRowChn);
			pStaticChn[i-1+j*(nRowChn)] = CreateStatic(rt, this, tmp);
			pStaticChn[i-1+j*(nRowChn)]->SetTextAlign(VD_TA_LEFT);
			itemsRight[1].push_back(pStaticChn[i-1+j*(nRowChn)]);
			
			rt.left  = rt.right;
			rt.right = rt.left + nBtnSize + nBtnGap;
		}
		
		if(i != 5) break;
	}
	
	SwitchCalendar(0);
}

void CPageSearch::SwitchCalendar( int type )
{
	if (type <0 || type >1)
	{
		return;
	}
	
	int i=0;
	int count = itemsRight[1-type].size();
	for (i=0; i<count; i++)
	{
		if(itemsRight[1-type][i])
		{
			itemsRight[1-type][i]->Show(FALSE);
		}
	}
	
	count = itemsRight[type].size();
	for (i=0; i<count; i++)
	{
		if(itemsRight[type][i])
		{
			itemsRight[type][i]->Show(TRUE);
		}
	}
}

void CPageSearch::OnClickPageCtrPage1()
{
	if(sSearchResult0.nFileNum == 0)
	{
		return;
	}
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < 4; i++)
	{
		if (pFocusButton == pBtPageCtrPage1[i])
		{
			bFind = TRUE;
			break;
		}
	}
	int old_page = m_curPage1;
	if (bFind)
	{
		switch(i)
		{
			case 0: //  第一页
			{
				if(m_curPage1 != 0)
				{
					m_curPage1 = 0;
				}
			}break;
			case 1: //  上一页
			{
				if(m_curPage1 > 0)
				{
					m_curPage1--;
				}
			}break;
			case 2: //  下一页
			{				
				if(m_curPage1 < (sSearchResult0.nFileNum/8+((sSearchResult0.nFileNum%8)?1:0))-1)
				{
					m_curPage1++;
				}
			}break;
			case 3: //  最后一页
			{
				m_curPage1 = (sSearchResult0.nFileNum/8+((sSearchResult0.nFileNum%8)?1:0))-1;
			}break;
			default:
			break;
		}		
	}
	if(old_page != m_curPage1)
	{
		SetPage1(m_curPage1);
		//SetProg(m_curPage1+1,(sSearchResult0.nFileNum/8+((sSearchResult0.nFileNum%8)?1:0)));
	}
}

void CPageSearch::ClearPage2()
{
	int maxRows = 8;
	for(int i=0; i<maxRows; i++)
	{
		SetChn2(i,"");
		SetBegin2(i,"");
		SetEnd2(i,"");
		pszResultPage2->SetText("");
	}
}

void CPageSearch::OnClickPageCtrPage2()
{
	if(sSearchResult.nFileNum == 0)
	{
		return;
	}
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < 4; i++)
	{
		if (pFocusButton == pBtPageCtrPage2[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
		switch(i)
		{
			case 0: //  第一页
			{
				//printf("maxPage = %d \n",m_maxPage2);
				//printf("Page = %d \n",m_page2);
				if(m_page2 != 0)
				{
					ClearPage2();
					m_page2 = 0;
					SetPage2(m_page2);
				}
				
			}break;
			case 1: //  上一页
			{
				if(m_page2 > 0)
				{
					ClearPage2();
					SetPage2(m_page2-1);
					m_page2--;
				}
			}break;
			case 2: //  下一页
			{
				
				if(m_page2 < m_maxPage2-1)
				{
					ClearPage2();
					SetPage2(m_page2+1);
					m_page2++;
				}
			}break;
			case 3: //  最后一页
			{
				if(m_page2 != m_maxPage2-1)
				{
					ClearPage2();
					m_page2 = m_maxPage2-1;
					SetPage2(m_page2);
				}
				
			}break;
			default:
			break;
		}
		
	}
}

void CPageSearch::OnDelFilePage2()
{

}

void CPageSearch::OnLockFilePage2()
{

}
/*
void CPageSearch::OnClickPageCtrPage3()
{

}

void CPageSearch::OnClickDealPicPage3()
{

}
*/
void CPageSearch::OnClickWnd1x1()
{
	pPageChnSel->SetCurStatusSearching(1);
	pPageChnSel->Open();
	m_nCurPlayMode = 1;
	char* szBtBmpPath[2][4] = {
			{DATA_DIR"/temp/search_1x1.bmp", DATA_DIR"/temp/search_1x1_f.bmp", DATA_DIR"/temp/search_1x1_n.bmp", DATA_DIR"/temp/search_1x1_sel.bmp"},
			{DATA_DIR"/temp/search_2x2.bmp", DATA_DIR"/temp/search_2x2_f.bmp", DATA_DIR"/temp/search_2x2_n.bmp", DATA_DIR"/temp/search_2x2_sel.bmp"},
		};
	pBtPage0Wnd1x1->SetBitmap(VD_LoadBitmap(szBtBmpPath[0][3]), VD_LoadBitmap(szBtBmpPath[0][3]),VD_LoadBitmap(szBtBmpPath[0][3]),VD_LoadBitmap(szBtBmpPath[0][2]));
	pBtPage0Wnd2x2->SetBitmap(VD_LoadBitmap(szBtBmpPath[1][0]), VD_LoadBitmap(szBtBmpPath[1][1]),VD_LoadBitmap(szBtBmpPath[1][3]),VD_LoadBitmap(szBtBmpPath[1][2]));
}

void CPageSearch::OnClickWnd2x2()
{
	if(GetVideoMainNum()>4)
	{
		pPageChnSel2X2->SetCurStatusSearching(1);
		pPageChnSel2X2->Open();
	}	
	m_nCurPlayMode = 4;
	char* szBtBmpPath[2][4] = {
			{DATA_DIR"/temp/search_1x1.bmp", DATA_DIR"/temp/search_1x1_f.bmp", DATA_DIR"/temp/search_1x1_n.bmp", DATA_DIR"/temp/search_1x1_sel.bmp"},
			{DATA_DIR"/temp/search_2x2.bmp", DATA_DIR"/temp/search_2x2_f.bmp", DATA_DIR"/temp/search_2x2_n.bmp", DATA_DIR"/temp/search_2x2_sel.bmp"},
		};
	pBtPage0Wnd1x1->SetBitmap(VD_LoadBitmap(szBtBmpPath[0][0]), VD_LoadBitmap(szBtBmpPath[0][1]),VD_LoadBitmap(szBtBmpPath[0][3]),VD_LoadBitmap(szBtBmpPath[0][2]));
	pBtPage0Wnd2x2->SetBitmap(VD_LoadBitmap(szBtBmpPath[1][3]), VD_LoadBitmap(szBtBmpPath[1][3]),VD_LoadBitmap(szBtBmpPath[1][3]),VD_LoadBitmap(szBtBmpPath[1][2]));
}

//csp modify
void CPageSearch::StartPlay2()
{
	printf(">>>>>>>>>startplay2, time:%d\n", time(NULL));

	//本机回放
	SSG_MSG_TYPE msg;
	memset(&msg, 0, sizeof(msg));
	msg.type = EM_DVR_PLAYBACK;
	msg.chn = 0;
	strcpy(msg.note, GetParsedString("&CfgPtn.Local"));
	strcat(msg.note, GetParsedString("&CfgPtn.Playback"));
	upload_sg(&msg);
	
	u32 chn_tmp = 0;
	u32 g_nUserId = GetGUILoginedUserID();
	EMBIZUSERCHECKAUTHOR author = BizUserCheckAuthority(EM_BIZ_USER_AUTHOR_PLAYBACK,&chn_tmp,g_nUserId);
	if(author == EM_BIZ_USER_AUTHOR_NO)
	{
		MessageBox("&CfgPtn.NoAuthority", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
		return;
	}
	
	SBizSearchPara sBizSearchParam;
	memset(&sBizSearchParam,0,sizeof(sBizSearchParam));
	
	SYSTEM_TIME start;
	SYSTEM_TIME stop;
	
	pBeginDatePR->GetDateTime(&start);
	pBeginTimePR->GetDateTime(&start);
	
	pEndDatePR->GetDateTime(&stop);
	pEndTimePR->GetDateTime(&stop);
	
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
	sBizSearchParam.nStartTime = mktime(&tM);
	
	tM.tm_year = stop.year - 1900;
	tM.tm_mon = stop.month - 1;
	tM.tm_mday = stop.day;
	tM.tm_hour = stop.hour;
	tM.tm_min = stop.minute;
	tM.tm_sec = stop.second;
	tM.tm_isdst = 0;
	tM.tm_wday = 0;
	tM.tm_yday = 0;
	sBizSearchParam.nEndTime = mktime(&tM);
	
	//csp modify 20131213
	int nTimeZone = GetTimeZone();
	sBizSearchParam.nStartTime -= GetTimeZoneOffset(nTimeZone);
	sBizSearchParam.nEndTime -= GetTimeZoneOffset(nTimeZone);
	
	//printf("start:%ld, end:%ld\n", sBizSearchParam.nStartTime, sBizSearchParam.nEndTime);
	
	//start play
	if(sBizSearchParam.nEndTime > sBizSearchParam.nStartTime)
	{
		int num = 0;
		for(int i=0; i<(nChMax/4+((nChMax%4)?1:0)); i++)
		{
			for(int j=1;j<5;j++)
			{
				//csp modify 20130504
				if(i*4+j>nChMax) break;
				
				if(pChkChn[i*5+j]->GetValue())
				{
					sBizSearchParam.nMaskChn |= (1 << (i*4+j-1));
					num++;
				}
			}
		}
		if(!num)
		{
			return;
		}
		//csp modify
		else if(num > m_nMaxPlayChn)
		{
			char tmp[100] = {0};
			sprintf(tmp, "%s: %d", GetParsedString("&CfgPtn.ReachMaxSupportPlayNum"),m_nMaxPlayChn);
			MessageBox(tmp, "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
			return;
		}
		else
		{
			if(num == 1)
			{
				m_nCurPlayMode = 1;
			}
			else if(num <= 4)
			{
				m_nCurPlayMode = 4;
			}
			else if(num <= 9)
			{
				m_nCurPlayMode = 9;
			}
			else if(num <= 16)
			{
				m_nCurPlayMode = 16;
			}
			else if(num <= 24)
			{
				m_nCurPlayMode = 24;
			}
			else if(num <= 32)
			{
				m_nCurPlayMode = 35;//36???
			}
			else
			{
				return;
			}
		}
		
		//csp modify
		//sBizSearchParam.nMaskType = 0xf;
		if(pChkPage1[4]->GetValue())
		{
			sBizSearchParam.nMaskType = 0xf;
		}
		else
		{
			sBizSearchParam.nMaskType = 0x0;
			for(int i = 0; i < 4; i++)
			{
				if(pChkPage1[i]->GetValue())
				{
					sBizSearchParam.nMaskType |= 1<<i;
				}
			}
		}
		
		SetSystemLockStatus(1);//cw_lock
		
		CPage** page = GetPage();//cw_rec
		((CPageDesktop*)page[EM_PAGE_DESKTOP])->SetModePlaying();
		
		this->Close();
		//this->m_pParent->Close();//cw_test
		
		m_pPagePlayBack->SetSearchPage((CPageSearch *)this);
		m_pPagePlayBack->SetPlayChnNum(m_nCurPlayMode);
		
		BizStopPreview();
		
		m_pPagePlayBack->Open();
		
		BizStartPlayback(EM_BIZPLAY_TYPE_TIME,&sBizSearchParam);
		
		SetSystemLockStatus(0);
	}
}

void CPageSearch::OnClickStartPlay()
{
	//if(!m_nCurPlayMode)
	//{
	//	return;
	//}
	
	printf(">>>>>>>>>startplay, time:%d\n", time(NULL));
	
	//本机回放
	SSG_MSG_TYPE msg;
	memset(&msg, 0, sizeof(msg));
	msg.type = EM_DVR_PLAYBACK;
	msg.chn = 0;
	strcpy(msg.note, GetParsedString("&CfgPtn.Local"));
	strcat(msg.note, GetParsedString("&CfgPtn.Playback"));
	upload_sg(&msg);
	
	u32 chn_tmp = 0;
	u32 g_nUserId = GetGUILoginedUserID();
	EMBIZUSERCHECKAUTHOR author = BizUserCheckAuthority(EM_BIZ_USER_AUTHOR_PLAYBACK,&chn_tmp,g_nUserId);
	if(author == EM_BIZ_USER_AUTHOR_NO)
	{
		MessageBox("&CfgPtn.NoAuthority", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
		return;
	}
	
	SBizSearchPara sBizSearchParam;
	memset(&sBizSearchParam,0,sizeof(sBizSearchParam));
	
#if 1
	SYSTEM_TIME start;
	SYSTEM_TIME stop;
	
	pBeginDateP0->GetDateTime(&start);
	pBeginTimeP0->GetDateTime(&start);
	pEndDateP0->GetDateTime(&stop);
	pEndTimeP0->GetDateTime(&stop);
	struct tm tM;
	memset(&tM, 0, sizeof(tM));	
	tM.tm_year = start.year - 1900;
	//yaogang 20150729
	//tM.tm_year = 70;
	tM.tm_mon = start.month - 1;
	tM.tm_mday = start.day;
	tM.tm_hour = start.hour;
	tM.tm_min = start.minute;
	tM.tm_sec = start.second;
	tM.tm_isdst = 0;
	tM.tm_wday = 0;
	tM.tm_yday = 0;
	sBizSearchParam.nStartTime = mktime(&tM);
	
	tM.tm_year = stop.year - 1900;
	tM.tm_mon = stop.month - 1;
	tM.tm_mday = stop.day;
	tM.tm_hour = stop.hour;
	tM.tm_min = stop.minute;
	tM.tm_sec = stop.second;
	tM.tm_isdst = 0;
	tM.tm_wday = 0;
	tM.tm_yday = 0;
	sBizSearchParam.nEndTime = mktime(&tM);
	//printf("%s stop.year: %d\n", __func__, tM.tm_year);
	
	//csp modify 20131213
	int nTimeZone = GetTimeZone();
	//yaogang 20150729
	sBizSearchParam.nStartTime -= GetTimeZoneOffset(nTimeZone);
	sBizSearchParam.nEndTime -= GetTimeZoneOffset(nTimeZone);
	
	//printf("start:%ld, end:%ld\n", sBizSearchParam.nStartTime, sBizSearchParam.nEndTime);
#else
	u32 nBgnTime;
	SYSTEM_TIME sTime, sDate;	
	pBeginTimeP0->GetDateTime(&sTime);
	nBgnTime = MakeTimeForBackup(
		1970, 1, 1,
		sTime.hour,
		sTime.minute,
		sTime.second
	);	
	pRecTimeBox->SetTrackTimePos(nBgnTime);
	pBeginDateP0->GetDateTime(&sDate);
	sBizSearchParam.nStartTime = 
		MakeTimeForBackup(
			sDate.year, 
			sDate.month, 
			sDate.day,
			sTime.hour,
			sTime.minute,
			sTime.second
		);
	
	time_t tTmp;
	struct tm tmstrTmp;
	struct tm* pTime;
	
	memset(&tmstrTmp,0,sizeof(struct tm));
	tTmp = time(NULL);
	
	//csp modify
	//pTime = localtime(&tTmp);
	//tmstrTmp = *pTime;
	pTime = &tmstrTmp;
	localtime_r(&tTmp, pTime);
	
	sBizSearchParam.nEndTime = mktime(&tmstrTmp);
#endif
	
	// start play
	if(sBizSearchParam.nEndTime>sBizSearchParam.nStartTime)
	{
		#if 1
		int num = 0;
		for(int i=0; i<(nChMax/4+((nChMax%4)?1:0)); i++)
		{
			for(int j=1;j<5;j++)
			{
				//csp modify 20130504
				if(i*4+j>nChMax) break;
				
				if(pPG0ChkChn[i*5+j]->GetValue())
				{
					sBizSearchParam.nMaskChn |= (1 << (i*4+j-1));
					num++;
				}
			}
		}
		if(!num)
		{
			return;
		}
		else
		{
			if(num == 1)
			{
				m_nCurPlayMode = 1;
			}
			else if(num <= 4)
			{
				m_nCurPlayMode = 4;
			}
			else if(num <= 9)
			{
				m_nCurPlayMode = 9;
			}
			else if(num <= 16)
			{
				m_nCurPlayMode = 16;
			}
			else if(num <= 24)
			{
				m_nCurPlayMode = 24;
			}
			else if(num <= 32)
			{
				m_nCurPlayMode = 35;
			}
			else
			{
				return;
			}
		}
		SetSystemLockStatus(1);//cw_lock
		sBizSearchParam.nMaskType = 0xf;
		CPage** page = GetPage();//cw_rec
		((CPageDesktop*)page[EM_PAGE_DESKTOP])->SetModePlaying();
		this->Close();
		//this->m_pParent->Close();//cw_test
		m_pPagePlayBack->SetSearchPage((CPageSearch *) this);
		m_pPagePlayBack->SetPlayChnNum(m_nCurPlayMode);
		BizStopPreview();
		m_pPagePlayBack->Open();
		BizStartPlayback(EM_BIZPLAY_TYPE_TIME,&sBizSearchParam);
		SetSystemLockStatus(0);
		#else
		if(4==GetVideoMainNum())
		{
			u8 nChSel = 0;
			int nStyle = 0, nChnMask = 0;
			if(m_nCurPlayMode == 1)
			{
				pPageChnSel->GetSel(&nStyle,&nChnMask);
				sBizSearchParam.nMaskChn = nChnMask;
				printf("nStyle = %d, nChnMask = 0x%02x\n", nStyle,nChnMask);
			}
			else
			{
				sBizSearchParam.nMaskChn = 0xf;
			}			
			sBizSearchParam.nMaskType = 0xf;
			
			this->Close();
			this->m_pParent->Close();
			m_pPagePlayBack->SetSearchPage((CPageSearch *) this);
			m_pPagePlayBack->SetPlayChnNum(m_nCurPlayMode);
			//m_pPagePlayBack->SetPreviewMode(EM_BIZPREVIEW_4SPLITS);					
			m_pPagePlayBack->Open();
			BizStopPreview();
			
			BizStartPlayback(EM_BIZPLAY_TYPE_TIME,&sBizSearchParam);
		}
		#endif
	}
}

void CPageSearch::OnClickTrackPage0()
{
	int ntime = pRecTimeBox->GetTrackTimePos();
	if(pBeginTimeP0)
	{
		SYSTEM_TIME systime;
		systime.hour = ntime/3600;
		systime.minute = ntime/60 - systime.hour*60;
		systime.second = ntime%60;
		pBeginTimeP0->SetDateTime(&systime);
	}
}

void CPageSearch::SetResult2(int page)
{
	char szResult[16] = {0};
	sprintf(szResult,"%d/%d",page+1,m_maxPage2);
	pszResultPage2->SetText(szResult);
}

void CPageSearch::SetPage2(int page)
{	
	SBizRecfileInfo* pFInfo = NULL;
	
	char szCh[8] = {0};
	char szTime[32] = {0};
	int maxRow = 8;
	for(int i=0; i<maxRow; i++)
	{
		int fileNo = page*maxRow + i;
		if(page >= m_maxPage2 || fileNo >= sSearchResult.nFileNum)
		{
			break;
		}
		
		pFInfo = &sSearchResult.psRecfileInfo[fileNo];
		sprintf(szCh,"%d",pFInfo->nChn);
		SetChn2(i,szCh);
		GetTimeForBackup(pFInfo->nStartTime, szTime);	
		SetBegin2(i, szTime);
		GetTimeForBackup(pFInfo->nEndTime, szTime);
		SetEnd2(i, szTime);
	}
	
	SetResult2(page);
}

void CPageSearch::SearchFilesWithEvent()
{
	printf("%s>>>>>>begin search, time:%d\n", __func__,time(0));
	
	EMBIZFILETYPE emFileType = EM_BIZFILE_REC;
	u32 nMaskChn = 0;
	u8 nMax = GetVideoMainNum();
	u32 begin[nMax];//zlb20111117  去掉部分malloc
	u32 end[nMax];//zlb20111117  去掉部分malloc
	
	SBizSearchResult *psResult;
	
	memset(begin, 0, sizeof(u32) * nMax);
	memset(end, 0, sizeof(u32) * nMax);
	
	memset(&sSearchPara, 0, sizeof(sSearchPara));
	
	sSearchResult0.nFileNum = 0;
	memset(sSearchResult0.psRecfileInfo, 0, 4000*sizeof(SBizRecfileInfo));
	
	if(pChkPage1[4]->GetValue())
	{
		sSearchPara.nMaskType = 0xf;
	}
	else
	{
		sSearchPara.nMaskType = 0x0;//csp modify
		for(int i = 0; i < 4; i++)
		{
			if(pChkPage1[i]->GetValue())
			{
				sSearchPara.nMaskType |= 1<<i;
			}
		}
	}
	
/*	if( pChkChn[0]->GetValue() )
	{
		for( int i=0; i<nMax; i++ )
			nMaskChn |= (1<<i);
	}
	else
	{
		for( int i=0; i<nMax; i++ )
		{
			if(pChkChn[1+i]->GetValue())
			{
				nMaskChn |= (1<<i);
			}
		}
	}
*/
	for(int i=0; i<nMax; i++)
	{
		if(pChkChn[(i/4+1)+i]->GetValue())
		{
			nMaskChn |= (1<<i);
		}
	}
	sSearchPara.nMaskChn = nMaskChn;
	
/*
	SYSTEM_TIME tSysDate, tSysTime;	
	pCalendar->GetDate(&tSysDate);
	sSearchPara.nStartTime = 
		MakeTimeForBackup(
			tSysDate.year,
			tSysDate.month,
			tSysDate.day,
			0,
			0,
			0
		);
	
	sSearchPara.nEndTime = 
		MakeTimeForBackup(
			tSysDate.year,
			tSysDate.month,
			tSysDate.day,
			23,
			59,
			59
		);
*/
	
	SYSTEM_TIME start;
	SYSTEM_TIME stop;
	
	pBeginDatePR->GetDateTime(&start);
	pBeginTimePR->GetDateTime(&start);
	pEndDatePR->GetDateTime(&stop);
	pEndTimePR->GetDateTime(&stop);	
	struct tm tM;
	memset(&tM, 0, sizeof(tM));	
	tM.tm_year = start.year - 1900;
	//yaogang 20150729
	//tM.tm_year = 70;
	tM.tm_mon = start.month - 1;
	tM.tm_mday = start.day;
	tM.tm_hour = start.hour;
	tM.tm_min = start.minute;
	tM.tm_sec = start.second;
	tM.tm_isdst = 0;
	tM.tm_wday = 0;
	tM.tm_yday = 0;
	sSearchPara.nStartTime = mktime(&tM);
	//printf("%d/%d/%d %d:%d:%d\n", start.year,start.month,start.day,start.hour,start.minute,start.second);
	
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
	
	//csp modify 20131213
	int nTimeZone = GetTimeZone();
	//yaogang 20150729 
	sSearchPara.nStartTime -= GetTimeZoneOffset(nTimeZone);
	sSearchPara.nEndTime -= GetTimeZoneOffset(nTimeZone);
	
	if(!sSearchResult0.psRecfileInfo)
	{
		//free(begin);//zlb20111117  去掉部分malloc
		//free(end);
		return;
	}
	if( 0==BizSysComplexDMSearch(
		emFileType,
		&sSearchPara, 
		&sSearchResult0, 
		4000 ))
	{
		psResult = &sSearchResult0;
		printf("file num : %d\n", psResult->nFileNum);
		m_nFiles = psResult->nFileNum;
		m_curPage1 = 0;
		SetProg(0,0);
		SetPage1(m_curPage1);
		if(psResult->nFileNum)
		{
			MouseMoveToLine(-1);//cw_tab
			pTips->SetText("&CfgPtn.DoubleClickToPlay");
		}
		else
		{
			pTips->SetText("&CfgPtn.NoRecordFiles");
		}
		
		if(psResult->nFileNum >= 4000)
		{
			char tmp[100] = {0};
			sprintf(tmp, "%s:%d", GetParsedString("&CfgPtn.ReachMaxSupportSearchNum"),psResult->nFileNum);
			MessageBox(tmp, "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
		}
	}
	
	//zlb20111117  去掉部分malloc
	/*if(begin)
	{
		free(begin);
	}
	
	if(end)
	{
		free(end);
	}*/
	
	printf(">>>>>>End search, time:%d\n", time(NULL));
}

void CPageSearch::SetPage1(int page)
{
	if((page <0) || (sSearchResult0.nFileNum == 0) || (sSearchResult0.nFileNum/8+(sSearchResult0.nFileNum%8?1:0)-1<page))
	{
		return;
	}
	
	int i;
	for(i = 0; i < 8; i++)
	{
		if(m_curPage1*8+i+1>sSearchResult0.nFileNum)
		{
			break;
		}
		SetLineOfPage1(i,&sSearchResult0.psRecfileInfo[m_curPage1*8+i]);
	}
	for(; i<8; i++)
	{
		ClearLineOfPage1(i);
	}
	SetProg(m_curPage1+1,(sSearchResult0.nFileNum/8+((sSearchResult0.nFileNum%8)?1:0)));
	
	MouseMoveToLine(-1);//cw_tab
	m_chSel=0;
	if(bInTab)
		MouseMoveToLine(0);
	
	return;
}

void CPageSearch::SetLineOfPage1(int line, SBizRecfileInfo* info)
{
	char szCh[8] = {0};
	memset(szCh, 0, sizeof(szCh));
	sprintf(szCh,"%d",info->nChn);
	pChn1[line]->SetText(szCh);
	
	char szTime[32] = {0};
	memset(szTime, 0, sizeof(szTime));
	GetTimeForBackup(info->nStartTime, szTime);	
	printf("chn[%02d], %s, %d\n",info->nChn,szTime,info->nStartTime);//捕获信息用，这次不要关闭
	pBegin1[line]->SetText(szTime);
	
	memset(szTime, 0, sizeof(szTime));
	GetTimeForBackup(info->nEndTime, szTime);
	printf("chn[%02d], %s, %d\n\n",info->nChn,szTime,info->nEndTime);
	pEnd1[line]->SetText(szTime);
	
	char* type[5] = {
		"&CfgPtn.Timer", 
		"&CfgPtn.Moving",
		"&CfgPtn.Sensoring", 
		"&CfgPtn.Manual",
		"Unknown"};
	u8 nType = 0;
	if(((m_sSearchTypeStatus.nMoving) || (m_sSearchTypeStatus.nAll)) && (info->nType & (1 << 1)))
	{
		nType = 1;
	}
	else if(((m_sSearchTypeStatus.nSensoring) || (m_sSearchTypeStatus.nAll)) && (info->nType & (1 << 2)))
	{
		nType = 2;
	}
	else if(((m_sSearchTypeStatus.nTimer) || (m_sSearchTypeStatus.nAll)) && (info->nType & (1 << 0)))
	{
		nType = 0;
	}
	else if(((m_sSearchTypeStatus.nManual) || (m_sSearchTypeStatus.nAll)) && (info->nType & (1 << 3)))
	{
		nType = 3;
	}
	else
	{
		nType = 4;
	}
	pType1[line]->SetText(type[nType]);
}

void CPageSearch::ClearLineOfPage1(int line)
{
	pChn1[line]->SetText("");
	pBegin1[line]->SetText("");
	pEnd1[line]->SetText("");
	pType1[line]->SetText("");
}

void CPageSearch::OnEventType()
{
	pChkPage1[4]->SetValue(0);
}

void CPageSearch::OnEventAllType()
{
	int value = pChkPage1[4]->GetValue();
	for(int i = 0; i<4; i++)
	{
		pChkPage1[i]->SetValue(value);
	}
}

void CPageSearch::SearchFilesWithFile()
{
	printf("%s>>>>>>begin search, time:%d\n", __func__,time(0));
	
	EMBIZFILETYPE emFileType = EM_BIZFILE_REC;
	u32 nMaskChn = 0;
	u8 nMax = GetVideoMainNum();
	u32 begin[nMax];//zlb20111117  去掉部分malloc
	u32 end[nMax];
	SBizSearchResult *psResult;
	
	memset(begin, 0, sizeof(u32) * nMax);
	memset(end, 0, sizeof(u32) * nMax);
	
	memset(&sSearchPara, 0, sizeof(sSearchPara));
	//memset(&sSearchResult, 0, sizeof(sSearchResult));
	sSearchResult.nFileNum = 0;
	memset(sSearchResult.psRecfileInfo, 0, 4000*sizeof(SBizRecfileInfo));
	
	// fill condition
	
	sSearchPara.nMaskType = 0xf;
	
	if(pChkChn[0]->GetValue())
	{
		for(int i=0; i<nMax; i++)
			nMaskChn |= (1<<i);
	}
	else
	{
		for(int i=0; i<nMax; i++)
		{
			if(pChkChn[1+i]->GetValue())
			{
				nMaskChn |= (1<<i);
			}
		}
	}
	sSearchPara.nMaskChn = nMaskChn;
	
	SYSTEM_TIME tSysDate, tSysTime;
	//pBeginDateP0->GetDateTime(&tSysDate);
	pCalendar->GetDate(&tSysDate);
	
	sSearchPara.nStartTime = 
		MakeTimeForBackup(
			tSysDate.year,
			tSysDate.month,
			tSysDate.day,
			0,
			0,
			0
		);
	
	sSearchPara.nEndTime = 
		MakeTimeForBackup(
			tSysDate.year,
			tSysDate.month,
			tSysDate.day,
			23,
			59,
			59
		);
	
	//printf("sSearchPara.nStartTime %d\n", sSearchPara.nStartTime);
	
	// search by time
#if 1
	//函数功能:文件搜索
//	sSearchResult.psRecfileInfo = 
//		(SBizRecfileInfo*)malloc(sizeof(SBizRecfileInfo)*4000);
	
	if(!sSearchResult.psRecfileInfo)
	{		
		//free(begin); //zlb20111117  去掉部分malloc
		//free(end);
		return;
	}	
	if( 0==BizSysComplexDMSearch(
		emFileType,
		&sSearchPara, 
		&sSearchResult, 
		4000 ))
	{
		psResult = &sSearchResult;
		printf("file num : %d\n", psResult->nFileNum);
		m_nFiles = psResult->nFileNum;
		m_maxPage2 = psResult->nFileNum/8 + (psResult->nFileNum%8 ? 1 : 0);
		
		//if(psResult->nFileNum>0)
		{
			ClearPage2();
			m_page2 = 0;
			SetPage2(m_page2);
		}
	}
	
//	if(sSearchResult.psRecfileInfo)
//		free(sSearchResult.psRecfileInfo);
	
#endif
	
	 //zlb20111117  去掉部分malloc
	/*if(begin)
	{
		free(begin);
	}
	
	if(end)
	{
		free(end);
	}*/
}

/*
void CPageSearch::SetChn(int row, char* szText)
{
	if( row<0 || row>8)
	{
		return;
	}

	//pChkPage2[row]->Enabel(TRUE);
	pChn[row]->SetText(szText);
}

void CPageSearch::SetBegin(int row, char* szText)
{
	if( row<0 || row>8)
	{
		return;
	}

	pBegin[row]->SetText(szText);

}

void CPageSearch::SetEnd(int row, char* szText)
{
	if( row<0 || row>8)
	{
		return;
	}

	pEnd[row]->SetText(szText);

}

void CPageSearch::SetState(int row, char* szText)
{
	if( row<0 || row>8)
	{
		return;
	}

	pState[row]->SetText(szText);

}
*/

void CPageSearch::SetChn2(int row, char* szText)
{
	if(row<0 || row>8)
	{
		return;
	}
	
	//pChkPage2[row]->Enabel(TRUE);
	pChn2[row]->SetText(szText);
}

void CPageSearch::SetBegin2(int row, char* szText)
{
	if(row<0 || row>8)
	{
		return;
	}
	
	pBegin2[row]->SetText(szText);
}

void CPageSearch::SetEnd2(int row, char* szText)
{
	if(row<0 || row>8)
	{
		return;
	}
	
	pEnd2[row]->SetText(szText);
}

void CPageSearch::OnDaySelected()
{
	//获得当前选中日期
	SYSTEM_TIME stSelTime;
	pCalendar->GetDate(&stSelTime);
	
	printf("%4d-%02d-%02d\n", stSelTime.year, stSelTime.month, stSelTime.day);
	
	//将选中的填充
	pCalendar->SetMask(1<<(stSelTime.day-1));
	
	SYSTEM_TIME stTime;//cw_calendar

	if (m_mCurID == 0)
	{
		pCalendar->GetDate(&stTime);
		stTime.hour = 0;
		stTime.minute = 0;
		stTime.second = 0;
		pBeginDateP0->SetDateTime(&stTime);
		pBeginTimeP0->SetDateTime(&stTime);
		stTime.hour = 23;
		stTime.minute = 59;
		stTime.second = 59;
		pEndDateP0->SetDateTime(&stTime);
		pEndTimeP0->SetDateTime(&stTime);
	}
	else if (m_mCurID == 3)
	{
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
	}
	
 	return;
}

void CPageSearch::UpdateCalendar(SYSTEM_TIME* pTime)
{	
	pCalendar->SetDate(pTime);
	pCalendar->SetMask(1<<(pTime->day-1));	
}

VD_BOOL CPageSearch::UpdateData( UDM mode )
{
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
	
	static u8 flag = 0;
	if(flag == 0)
	{
		memset(&sSearchResult0, 0, sizeof(sSearchResult0));
		sSearchResult0.psRecfileInfo = (SBizRecfileInfo*)calloc(sizeof(SBizRecfileInfo), 4000);
		memset(&sSearchResult, 0, sizeof(sSearchResult));
		sSearchResult.psRecfileInfo = (SBizRecfileInfo*)calloc(sizeof(SBizRecfileInfo), 4000);
		//memset(&sSearchSnapResult, 0, sizeof(sSearchSnapResult));
		//sSearchSnapResult.psRecSnapInfo = (SBizRecSnapInfo*)calloc(sizeof(SBizRecSnapInfo), 4000);

		if(sSearchResult.psRecfileInfo 
			&& sSearchResult0.psRecfileInfo)
			//&& sSearchSnapResult.psRecSnapInfo)
		{
			flag = 1;
		}
		else
		{
			if(sSearchResult.psRecfileInfo)
			{
				free(sSearchResult.psRecfileInfo);
				sSearchResult.psRecfileInfo = NULL;//csp modify
			}
			if(sSearchResult0.psRecfileInfo)
			{
				free(sSearchResult0.psRecfileInfo);
				sSearchResult0.psRecfileInfo = NULL;//csp modify
			}
			/*
			if (sSearchSnapResult.psRecSnapInfo)
			{
				free(sSearchSnapResult.psRecSnapInfo);
				sSearchSnapResult.psRecSnapInfo = NULL;
			}
			*/
		}
		CPage** page = GetPage();
		m_pPagePlayBack = (CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK];
	}
	
	static CPage** page = NULL;//cw_panel
	if(!page)
	{
		page = GetPage();
	}
	
	if(UDM_OPEN == mode)
	{
		//printf("Pagesearch open cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
		//this->m_pParent->Show(TRUE);//cw_test  this->m_pParent->Close();
		//m_mCurID = 0; //需要全更新，不能只更新某个页
		switch(m_mCurID)
		{
			case 0:
			{
				//printf("haha0\n");
				UpdateCalendar(&stTime1);
				//printf("haha1\n");
				//pBeginDateP0->SetDateTime(&stTime1);
				//printf("haha2\n");				
			} break;
			case 1:
			{
				UpdateCalendar(&stTime1);
			} break;
			case 2:
			{
				UpdateCalendar(&stTime1);
			} break;
			case 3:
			{
				UpdateCalendar(&stTime1);
				/*
				CRect rt;
				
				pTabFrame1->GetRect(&rt);
				rt.right += 40;
				pTabFrame1->SetRect(&rt);
				pTabFrame2->GetRect(&rt);
				rt.left += 40;
				pTabFrame2->SetRect(&rt);
				*/
			} break;
		}
		
		//ClearPage2();
		
		if(m_mCurID == 0 || m_mCurID == 3)
		{
			ShowSearchButton(0);
		}
		else
		{
			ShowSearchButton(1);
		}		
	}
	else if (UDM_CLOSED == mode)
	{
		QuitTab();//cw_tab
		
        #if 0
        //add by Lirl on Nov/08/2011        
        if (bTmpRecord || bFromMainBoard) {
            bTmpRecord = bFromMainBoard;
            bFromMainBoard = !bFromMainBoard;
        }
		
        if (bTmpRecord) {
            bFromMainBoard = bTmpRecord;
        }
        
        if (bFromMainBoard) {
            bTmpRecord = FALSE;
        }
        //end
        #endif
		
        //add by Lirl on Nov/14/2011
        if (bFromMainBoard) 
		{
			printf("closed myself FromMainBoard^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
			//this->m_pParent->Open();//cw_test
		}
		else
		{
			printf("closed myself Panel^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
			this->m_pParent->Open();//cw_test
		}
        //end
        
		//ClearPage2();
		
		MouseMoveToLine(-1);
		
		char tmp2[20] = {0};
		SBizDvrInfo sInfo;
		s32 ret = BizGetDvrInfo(&sInfo);
		if(ret == 0)
		{
			strcpy(tmp2, sInfo.sproductnumber);
		}
		if((0 == strcasecmp(tmp2, "NR3132")) || (0 == strcasecmp(tmp2, "NR3124")))
		{
			
		}
		else
		{
			for(int i=0; i<GetVideoMainNum(); i++)//cw_panel
			{
				(((CPageDesktop*)page[EM_PAGE_DESKTOP])->m_vChannelName[i])->SetText((((CPageDesktop*)page[EM_PAGE_DESKTOP])->m_vChannelName[i])->GetText());
			}
		}
		
		((CPageMainFrameWork *)(this->GetParent()))->ReflushItemName();
	}
	else if (UDM_OPENED == mode)
	{
		printf("Pagesearch opened cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
        //add by Lirl on Nov/08/2011
        //if (bFromMainBoard) {
		//    this->m_pParent->Open();
        //}
        //end
 //		this->Open();//cw_test
		//SetPage2(m_page2);
	}
	else if(UDM_EMPTY == mode)
	{
		printf("Pagesearch empty cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
//        	bFromMainBoard = TRUE;
		for(int i  = 0; i<8;i++)
		{
			ClearLineOfPage1(i);
		}
		sSearchResult0.nFileNum = 0;
		m_curPage1 = 0;
		pszResultPage1->SetText("0/0");
		pTips->SetText("");
		//ClearPage2();
		//sSearchResult.nFileNum = 0;
		
		pBeginDateP0->SetDateTime(&stTime1);
		pBeginTimeP0->SetDateTime(&stTime1);
		pBeginDateP3->SetDateTime(&stTime1);
		pBeginTimeP3->SetDateTime(&stTime1);
		pBeginDatePR->SetDateTime(&stTime1);
		pBeginTimePR->SetDateTime(&stTime1);
		
		stTime2.year = tmptime->tm_year+1900;
		stTime2.month = tmptime->tm_mon+1;
		stTime2.day = tmptime->tm_mday;
		stTime2.hour = tmptime->tm_hour;
		stTime2.minute = tmptime->tm_min;
		stTime2.second = tmptime->tm_sec;
		pEndDateP0->SetDateTime(&stTime2);
		pEndTimeP0->SetDateTime(&stTime2);
		pEndDateP3->SetDateTime(&stTime2);
		pEndTimeP3->SetDateTime(&stTime2);
		pEndDatePR->SetDateTime(&stTime2);
		pEndTimePR->SetDateTime(&stTime2);
		
		//m_nCurPlayMode = 4;
		/*char* szBtBmpPath[2][4] = {
				{DATA_DIR"/temp/search_1x1.bmp", DATA_DIR"/temp/search_1x1_f.bmp", DATA_DIR"/temp/search_1x1_n.bmp", DATA_DIR"/temp/search_1x1_sel.bmp"},
				{DATA_DIR"/temp/search_2x2.bmp", DATA_DIR"/temp/search_2x2_f.bmp", DATA_DIR"/temp/search_2x2_n.bmp", DATA_DIR"/temp/search_2x2_sel.bmp"},
			};
		pBtPage0Wnd1x1->SetBitmap(VD_LoadBitmap(szBtBmpPath[0][0]), VD_LoadBitmap(szBtBmpPath[0][1]),VD_LoadBitmap(szBtBmpPath[0][3]),VD_LoadBitmap(szBtBmpPath[0][2]));
		pBtPage0Wnd2x2->SetBitmap(VD_LoadBitmap(szBtBmpPath[1][3]), VD_LoadBitmap(szBtBmpPath[1][3]),VD_LoadBitmap(szBtBmpPath[1][3]),VD_LoadBitmap(szBtBmpPath[1][2]));*/
	}
	
	return TRUE;
}

void CPageSearch::OnClickChooseChn()
{
	int val = pChkPage2[8]->GetValue();

	CCheckBox* pFocus = (CCheckBox*)GetFocusItem();
	if(pFocus==pChkPage2[8])
	{	
		for(int i=0 ;i<CHM; i++)
		{
			pChkPage2[i]->SetValue(val);
		}
		SetProg(m_nFiles,m_nFiles);
	}
	else
	{
		int nSelFile = 0;
		
		for(int i=0 ;i<CHM; i++)
		{
			if( pChkPage2[i]->GetValue() )
			{
				nSelFile++;
			}
		}
		SetProg(nSelFile,m_nFiles);
	}
	
	return;
}

void CPageSearch::OnCalendarChn()
{
	int 	i		= 0;
	BOOL	bFind	= FALSE;
	
	CCheckBox* pFocus = (CCheckBox *)GetFocusItem();
	for(i = 0; i < (nChMax/4+((nChMax%4)?1:0)); i++)
	{
		for(int j=1; j<5; j++)
		{
			if(i*4+j>nChMax) break;
			
			if(pFocus == pChkChn[i*5+j])
			{
				pChkChn[i*5]->SetValue(0);
				return;
			}
		}
	}
}

void CPageSearch::OnCalendarAllChn()
{
	int 	i 		= 0;
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
		for(int j=1; j<5; j++)
		{
			if(i*4+j>nChMax) break;
			
			pChkChn[i*5+j]->SetValue(pChkChn[i*5]->GetValue());
		}
	}
	
	return;
}

void CPageSearch::OnPG0AllChn()
{
	int 	i		= 0;
	BOOL	bFind	= FALSE;
	
	CCheckBox* pFocus = (CCheckBox *)GetFocusItem();
	for(i = 0; i < (nChMax/4+((nChMax%4)?1:0)); i++)
	{
		if(pFocus == pPG0ChkChn[i*5])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		if(m_nFlagHaveMaxPlayChn && pPG0ChkChn[i*5]->GetValue())
		{
			int num = 0;
			for(int k = 0; k < (nChMax/4+((nChMax%4)?1:0)); k++)
			{
				if(i == k)
				{
					continue;
				}
				
				for(int j=1; j<5; j++)
				{
					if(k*4+j>nChMax) break;
					
					if(pPG0ChkChn[k*5+j]->GetValue())
						num++;
				}
			}
			
			//csp modify 20130504
			if(i == (nChMax/4+((nChMax%4)?1:0))-1)
			{
				if(nChMax%4)
				{
					num += (nChMax%4);
				}
				else
				{
					num += 4;
				}
			}
			else
			{
				num += 4;
			}
			
			//csp modify 20130504
			//if(num + 4 > m_nMaxPlayChn)
			if(num > m_nMaxPlayChn)
			{
				pPG0ChkChn[i*5]->SetValue(0);
				char tmp[100] = {0};
				sprintf(tmp, "%s: %d", GetParsedString("&CfgPtn.ReachMaxSupportPlayNum"),m_nMaxPlayChn);
				MessageBox(tmp, "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
				return;
			}
		}
		
		for(int j=1; j<5; j++)
		{
			if(i*4+j>nChMax) break;
			
			pPG0ChkChn[i*5+j]->SetValue(pPG0ChkChn[i*5]->GetValue());
		}
	}
	
	return;
}

void CPageSearch::OnPG0Chn()
{
	int 	i		= 0;
	BOOL	bFind	= FALSE;
	
	CCheckBox* pFocus = (CCheckBox *)GetFocusItem();
	
	int num = 0;
	if(m_nFlagHaveMaxPlayChn)
	{
		for(int k = 0; k < (nChMax/4+((nChMax%4)?1:0)); k++)
		{
			for(int j=1; j<5; j++)
			{
				if(k*4+j>nChMax) break;
				
				if(pPG0ChkChn[k*5+j]->GetValue())
					num++;
			}
		}
	}
	
	for(i = 0; i < (nChMax/4+((nChMax%4)?1:0)); i++)
	{
		for(int j=1; j<5; j++)
		{
			if(i*4+j>nChMax) break;
			
			if(pFocus == pPG0ChkChn[i*5+j])
			{
				if(m_nFlagHaveMaxPlayChn && pPG0ChkChn[i*5+j]->GetValue() && (num > m_nMaxPlayChn))
				{
					pPG0ChkChn[i*5+j]->SetValue(0);
					char tmp[100] = {0};
					sprintf(tmp, "%s: %d", GetParsedString("&CfgPtn.ReachMaxSupportPlayNum"),m_nMaxPlayChn);
					MessageBox(tmp, "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
					return;
				}
				
				pPG0ChkChn[i*5]->SetValue(0);
				return;
			}
		}
	}
	
	return;
}

void CPageSearch::ShowSearchButton(VD_BOOL show)
{
	pButton[SEARCH_COMMBUTTON-1]->Show(show);
	pButton[SEARCH_COMMBUTTON-2]->Show(show);//csp modify
	if(show)
	{
		SwitchCalendar(1);
	}
	else
	{
		SwitchCalendar(0);
	}
	/*pTabChnSearch->Show(show);
	for(int i=0;i<nChMax;i++)
	{
		pStaticChn[i]->Show(show);
	}
	for(int i=0;i<5*8;i++)
	{
		if(!pChkChn[i])
			break;
		pChkChn[i]->Show(show);
	}*/
}

void CPageSearch::SetProg(int curpage, int totalpage)
{
	char tmp[15] = {0};
	switch(m_mCurID)
	{
		case 0:
		{
		}
		break;
		case 1:
		{
			sprintf(tmp, "%d/%d", curpage,totalpage);
			pszResultPage1->SetText(tmp);
		}
		break;
		case 2:
		{	
			sprintf(tmp, "%d/%d", curpage,totalpage);
			pszResultPage2->SetText(tmp);
		}
		break;
		case 3:
		{
		}
		break;
	}
	
	return;
}

int CPageSearch::GetCurFileNum()//cw_tab
{
	int num;
	if (sSearchResult0.nFileNum<8)
		num= sSearchResult0.nFileNum;
	else 
	{
		if((m_curPage1+1)*8>sSearchResult0.nFileNum)
			num =sSearchResult0.nFileNum-(m_curPage1*8);
		else
			num= 8;
	}
	return num;
}

void CPageSearch::QuitTab()
{
	bInTab=0;
	MouseMoveToLine(-1);
	m_chSel=0;
}

#define MAX_SEARCH_TITLE	3

#if 1
VD_BOOL CPageSearch::MsgProc( uint msg, uint wpa, uint lpa )
{
	int px,py;
	int temp;
	
	if(msg == XM_KEYDOWN)//cw_panel
	{
		if(wpa == KEY_PTZ)
		{
			CItem *pItemSpecial = FindPage(m_screen, " Ptz Control", FALSE);
			this->Close();
			this->m_pParent->Close();
			pItemSpecial->Open();
			return TRUE;
		}
	}
	
	if((m_mCurID==1)||(m_mCurID==2))
	switch(msg)
	{
		case XM_KEYDOWN://cw_tab
		{
			switch(wpa)
			{
				case KEY_ESC:
				{
					if(bInTab)
					{
						QuitTab();
						return TRUE;
					}
				}break;
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
					CItem*tmp=GetFocusItem();
					if (tmp!=NULL)
					{
						if ((tmp->GetType()!=IT_EDIT)&&(tmp->GetType()!=IT_NUMBERBOX))
						{
							bInTab=1;
							m_chSel=wpa -2;
							if((m_chSel<GetCurFileNum())&&m_chSel>=0)
							{
								MouseMoveToLine(m_chSel);
							}
							return TRUE;
						}
					}
					else
					{
						bInTab=1;
						m_chSel=wpa -2;
						if((m_chSel<GetCurFileNum())&&m_chSel>=0)
						{
							MouseMoveToLine(m_chSel);
						}		
					}
					//printf("m_chSel = %d\n",m_chSel);
 				}break;
				
				case KEY_UP:
				{
					if (bInTab)
					{
						if (--m_chSel>=0)
							MouseMoveToLine(m_chSel);
						else
						{	
							m_chSel=GetCurFileNum()-1;
							MouseMoveToLine(m_chSel);
						}
						return TRUE;
					}
				}break;
				
				case KEY_DOWN:
				{
					if (bInTab)
					{
						if (++m_chSel<GetCurFileNum())
							MouseMoveToLine(m_chSel);
						else
						{
							m_chSel=0;
							MouseMoveToLine(m_chSel);
						}
						return TRUE;
					}
				}break;
				
				case KEY_PLAY:
				{
					if(bInTab)
					{
						u32 chn_tmp = 0;
						u32 g_nUserId = GetGUILoginedUserID();
						EMBIZUSERCHECKAUTHOR author = BizUserCheckAuthority(EM_BIZ_USER_AUTHOR_PLAYBACK,&chn_tmp,g_nUserId);
						if(author == EM_BIZ_USER_AUTHOR_NO)
						{
							MessageBox("&CfgPtn.NoAuthority", "&CfgPtn.WARNING"  , MB_OK|MB_ICONWARNING);
						}
						else
						{
							chn_tmp = m_chSel;
							QuitTab();
							SetSystemLockStatus(1);  //cw_lock
							PlayFile(m_mCurID, chn_tmp+(m_curPage1)*8);
							SetSystemLockStatus(0);
						}
					}
				}break;
				default: break;
			}
		}break;
		case XM_LBUTTONDBLCLK:
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
			CTableBox* table = (m_mCurID==1)?(pTabPage1[0]):(pTabPage2[0]);
			for(i = 0; i<8; i++)
			{
				for (int kk=0; kk<MAX_SEARCH_TITLE+1; kk++)
				{
					VD_RECT rt;
					table->GetTableRect(kk, i+1, &rt);
					rt.left +=10+offsetx;
					rt.top +=84+offsety;
					rt.right +=10+offsetx;
					rt.bottom +=84+offsety;
					//printf("i=%d kk =%d ,x=%d, y=%d, %d %d %d %d \n",i, kk, px, py, rt.left, rt.top, rt.right,rt.bottom);
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
				if(XM_LBUTTONDBLCLK == msg)
				{
					u32 chn_tmp = 0;
					u32 g_nUserId = GetGUILoginedUserID();
					EMBIZUSERCHECKAUTHOR author = BizUserCheckAuthority(EM_BIZ_USER_AUTHOR_PLAYBACK,&chn_tmp,g_nUserId);
					if(author == EM_BIZ_USER_AUTHOR_NO)
					{
						MessageBox("&CfgPtn.NoAuthority", "&CfgPtn.WARNING"  , MB_OK|MB_ICONWARNING);
					}
					else
					{
						//本机回放
						SSG_MSG_TYPE msg;
						memset(&msg, 0, sizeof(msg));
						msg.type = EM_DVR_PLAYBACK;
						msg.chn = 0;
						strcpy(msg.note, GetParsedString("&CfgPtn.Local"));
						strcat(msg.note, GetParsedString("&CfgPtn.Playback"));
						upload_sg(&msg);
						
						SetSystemLockStatus(1);  //cw_lock	
						PlayFile(m_mCurID, i+(((m_mCurID==2)?m_page2:m_curPage1)*8));
						SetSystemLockStatus(0);
					}
				}
				else
				{
					MouseMoveToLine(i);
				}
			}
			else
			{
				if(XM_MOUSEMOVE == msg)
				{
					MouseMoveToLine(-1);
				}
			}
		}
		break;
	default:
		break;
	}
	
	return CPageFrame::MsgProc(msg, wpa, lpa);
}
#endif
void CPageSearch::SelectDskItem( int index )
{
	if (index < 0 || index>7)
	{
		return;
	}

	//printf(" idx=%d, cur =%d \n", index, m_nCursel);

	if (index == m_nCursel)
	{
		return;
	}

	for (int kk=0; kk<MAX_SEARCH_TITLE; kk++)
	{
		if (m_nCursel >=0)
		{
			pChn2[kk]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pChn2[kk]->Draw();
			pBegin2[kk]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pBegin2[kk]->Draw();
			pEnd2[kk]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pEnd2[kk]->Draw();
		}

		pChn2[kk]->SetBkColor(VD_RGB(56,108,148));
		pChn2[kk]->Draw();
		pBegin2[kk]->SetBkColor(VD_RGB(56,108,148));
		pBegin2[kk]->Draw();
		pEnd2[kk]->SetBkColor(VD_RGB(56,108,148));
		pEnd2[kk]->Draw();
	}
	
	m_nCursel = index;
}

void CPageSearch::MouseMoveToLine( int index )
{
	if(index == m_nCurMouseLine)
	{
		return;
	}
	if((m_nCurMouseLine>=0) && (strcmp(pChn1[m_nCurMouseLine]->GetText(),"") != 0))
	{
		pChn1[m_nCurMouseLine]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
		pType1[m_nCurMouseLine]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
		pBegin1[m_nCurMouseLine]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
		pEnd1[m_nCurMouseLine]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
	}
	m_nCurMouseLine = index;
	
	if (index < 0 || index>7 )
	{
		return;
	}

	if( strcmp(pChn1[index]->GetText(),"") == 0)
	{
		return ;
	}

	pChn1[m_nCurMouseLine]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
	pType1[m_nCurMouseLine]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
	pBegin1[m_nCurMouseLine]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
	pEnd1[m_nCurMouseLine]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
}


void CPageSearch::SetPlayPage(CPagePlayBackFrameWork* pPagePlayBack)
{
	m_pPagePlayBack = pPagePlayBack;
}

void CPageSearch::FromPanel()
{
    bFromMainBoard = FALSE;
}

void CPageSearch::FromMainpage()
{
	bFromMainBoard = TRUE;
}

void CPageSearch::PlayFile(int tabPage, int nId)
{
	if((tabPage == 2) && (nId < sSearchResult.nFileNum))
	{
		CPage** page = GetPage();		//cw_rec
		((CPageDesktop*)page[EM_PAGE_DESKTOP])->SetModePlaying();
		this->Close();
		//this->m_pParent->Show(FALSE);//cw_test  this->m_pParent->Close();
		//m_pPagePlayBack->SetRect(&pbFloat,TRUE);
		m_pPagePlayBack->SetSearchPage((CPageSearch *) this);
		m_pPagePlayBack->SetPlayChnNum(1);
		//m_pPagePlayBack->SetPreviewMode(EM_BIZPREVIEW_4SPLITS);	
		BizStopPreview();				
		printf(" 1 PlayBack Open^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
		m_pPagePlayBack->Open();
		printf("1 fileNum:%d, nId:%d\nfile's chn:%d, type:%d\n", sSearchResult.nFileNum, nId, sSearchResult.psRecfileInfo[nId].nChn, sSearchResult.psRecfileInfo[nId].nType);
		BizStartPlayback(EM_BIZPLAY_TYPE_FILE, &sSearchResult.psRecfileInfo[nId]);
	}
	else if((tabPage == 1) && (nId < sSearchResult0.nFileNum))
	{
		CPage** page = GetPage();		//cw_rec
		((CPageDesktop*)page[EM_PAGE_DESKTOP])->SetModePlaying(); 
		this->Close();
		//this->m_pParent->Show(FALSE);//cw_test  this->m_pParent->Close();
		//m_pPagePlayBack->SetRect(&pbFloat,TRUE);
		m_pPagePlayBack->SetSearchPage((CPageSearch *) this);
		m_pPagePlayBack->SetPlayChnNum(1);
		//m_pPagePlayBack->SetPreviewMode(EM_BIZPREVIEW_4SPLITS);	
		BizStopPreview();
		printf("2 PlayBack Open^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
		m_pPagePlayBack->Open();
		printf("2 fileNum:%d, nId:%d\nfile's chn:%d, type:%d\n", sSearchResult0.nFileNum, nId, sSearchResult0.psRecfileInfo[nId].nChn, sSearchResult0.psRecfileInfo[nId].nType);
		BizStartPlayback(EM_BIZPLAY_TYPE_FILE, &sSearchResult0.psRecfileInfo[nId]);
	}
/*	u32 nBgnTime, nEndTime;
	SYSTEM_TIME sTime, sDate;
	int ret;
	
	CPage** pPageList = GetPage();
	((CPagePlayBackFrameWork*)pPageList[EM_PAGE_PLAYBACK])->SetDesktop(pPageList[EM_PAGE_DESKTOP]);

	struct tm* pTmPtr;
	
	strptime((char*)pBegin2[m_nCursel]->GetText(),"%Y-%m-%d %H:%M:%S", pTmPtr);    
	
	// update timebox
	//
	nBgnTime = MakeTimeForBackup(
		pTmPtr->tm_year+1900, 
		pTmPtr->tm_mon, 
		pTmPtr->tm_mday,
		pTmPtr->tm_hour,
		pTmPtr->tm_min,
		0 //pTmPtr->tm_sec
	);
	
	strptime((char*)pEnd2[m_nCursel]->GetText(),"%Y-%m-%d %H:%M:%S", pTmPtr);    
	
	// update timebox
	//
	nEndTime = MakeTimeForBackup(
		pTmPtr->tm_year+1900, 
		pTmPtr->tm_mon, 
		pTmPtr->tm_mday,
		pTmPtr->tm_hour,
		pTmPtr->tm_min+1,
		0 //pTmPtr->tm_sec
	);

	printf("Search play file starttime %d, endtime %d\n", nBgnTime, nEndTime);
	
#if 1
	// fill search condition
	SBizSearchPara sBizSearchParam;
	memset(&sBizSearchParam,0,sizeof(sBizSearchParam));

	pBeginDateP0->GetDateTime(&sDate);
	sBizSearchParam.nStartTime = nBgnTime;
	sBizSearchParam.nEndTime = nEndTime;

	// start play
	if(sBizSearchParam.nEndTime>sBizSearchParam.nStartTime)
	{
		BIZ_DO_DBG("Data search start play ...\n");

		this->Close();
		this->m_pParent->Close();
		
		//m_pDesktop->Show(FALSE, TRUE);
		//m_pDesktop->Close();
		
		//CRect rect(0,500,800,600);
		//m_pPagePlayBack->SetRect(&rect,TRUE);
		//m_pPagePlayBack->Open();

		BIZ_DO_DBG("Data search start play ...2\n");
		BizStopPreview();

		printf("sBizSearchParam.nMaskChn %d\n", sBizSearchParam.nMaskChn);
		
		sBizSearchParam.nMaskChn = (1<<strtol(pChn2[m_nCursel]->GetText(), 0, 10)-1);
		
		printf("sBizSearchParam.nMaskChn %d\n", sBizSearchParam.nMaskChn);
		
		sBizSearchParam.nMaskType = 0xff;
#if 0
		m_pPagePlayBack->SetPbInfo(sBizSearchParam);
		m_pPagePlayBack->SetPreviewMode(nPreviewMode);
#else
		//m_pPagePlayBack->SetRect(&pbFloat,TRUE);

		printf("bizData_GetPreviewMode() %d\n", bizData_GetPreviewMode());

		((CPagePlayBackFrameWork*)pPageList[EM_PAGE_PLAYBACK])->SetPlayChnNum(1);//nChnNum);
		((CPagePlayBackFrameWork*)pPageList[EM_PAGE_PLAYBACK])->SetPbInfo(sBizSearchParam);
		((CPagePlayBackFrameWork*)pPageList[EM_PAGE_PLAYBACK])->SetPreviewMode(EM_BIZPREVIEW_4SPLITS);
		((CPagePlayBackFrameWork*)pPageList[EM_PAGE_PLAYBACK])->Open();
#endif
#if 0
	u8 nChn;
	u8 nType;
	u32 nStartTime;
	u32 nEndTime;
	u8 nFormat;      //3:cif; 4:4cif
	u8 nStreamFlag;  //0:视频流;1:音频流
	u32 nSize;
	u32 nOffset;
	u8 nDiskNo;
	u8 nPtnNo;
	u16 nFileNo;
	
		SBizRecfileInfo sFileIns;
		memset(&sFileIns, 0, sizeof(sFileIns));
		sFileIns.nChn = 
#endif		
		BizStartPlayback(EM_BIZPLAY_TYPE_TIME,&sBizSearchParam);
	}
#endif
	*/
}

//yaogang modify 20150105
void CPageSearch::OnPG3AllChn()
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

void CPageSearch::OnPG3Chn()
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

void CPageSearch::OnClickSearchSnap()
{
	printf("%s() time: %u\n", __func__, time(NULL));
	
	EMBIZFILETYPE emFileType = EM_BIZFILE_SNAP;
	u32 nMaskChn = 0;
	u8 nMax = GetVideoMainNum();

	//set SBizSearchPara
	memset(&sSearchPara, 0, sizeof(sSearchPara));
	
	//回放时搜索所有类型图片
	sSearchPara.nMaskType = 0xff;
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
	
	//传递搜索参数PageSnapPlayBack.cpp
	m_pPageSnapPB->SetSearchPara(&sSearchPara);
	m_pPageSnapPB->SetWorkmode(EM_PLAYBACK);

	//pageSnapPlayBack  open
	SetSystemLockStatus(1);//cw_lock
	//sBizSearchParam.nMaskType = 0xf;
	CPage** page = GetPage();//cw_rec
	((CPageDesktop*)page[EM_PAGE_DESKTOP])->SetModePlaying();
	this->Close();
	//m_pParent->Close();
	m_pPageSnapPB->SetSearchPage((CPageSearch *) this);
	//m_pPagePlayBack->SetPlayChnNum(m_nCurPlayMode);
	BizStopPreview();
	
	m_pPageSnapPB->Open();
	//BizStartPlayback(EM_BIZPLAY_TYPE_TIME,&sBizSearchParam);
	SetSystemLockStatus(0);
	
	//m_pParent->Close();
	//this->Close();
	//m_pPageSnapPB->Open();

	//return;
}




