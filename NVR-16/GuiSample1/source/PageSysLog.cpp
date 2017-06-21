//#include "biz.h"
//#include "common_basetypes.h"
//#include "common_geometric.h"
#include "GUI/Pages/PageSysLog.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageInfoFrameWork.h"
//#include "GUI/Pages/BizData.h"
#include <time.h>
//#include "biz.h"

//SBizLogResult* pLogSearchResult;
//void SetOneRow(int page, int row, SBizLogResult* src);

#define RIGHT_PAGE_WIDTH	175

#define LEFT_PAGE_L	5
#define LEFT_PAGE_R	(m_Rect.Width()-RIGHT_PAGE_WIDTH-18)
#define RIGHT_PAGE_L (m_Rect.Width()-RIGHT_PAGE_WIDTH-18)
#define RIGHT_PAGE_R (m_Rect.Width()-14)
#define FRAME_TOP	20

static char* szBtBmpPath[4][2] = {
	{DATA_DIR"/temp/listex_btn_leftmost.bmp", DATA_DIR"/temp/listex_btn_leftmost_f.bmp"},
	{DATA_DIR"/temp/listex_btn_left.bmp", DATA_DIR"/temp/listex_btn_left_f.bmp"},
	{DATA_DIR"/temp/listex_btn_right.bmp", DATA_DIR"/temp/listex_right_f.bmp"},
	{DATA_DIR"/temp/listex_btn_rightmost.bmp", DATA_DIR"/temp/listex_btn_rightmost_f.bmp"},
};

static VD_BITMAP* pBmpBtPageCtrl[4][2];

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

#define BUTTONWIDTH 80

CPageSysLog::CPageSysLog( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
{
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	
	InitPageLeft();
	InitPageRight();
	
	char* szStatic[3] = 
	{	
		"&CfgPtn.Motion",
		"&CfgPtn.Sensor",
		"&CfgPtn.VideoLoss"
	};
	
	CRect rt;
	rt.left = LEFT_PAGE_L;
	rt.right = rt.left + 20;
	rt.top = m_Rect.Height()-m_TitleHeight-38;
	rt.bottom = rt.top + 20;
	
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
	
	pBtExport = CreateButton(rt, this, "&CfgPtn.Export", (CTRLPROC)&CPageSysLog::OnBtExport, NULL, buttonNormalBmp);
	
	rt.left += BUTTONWIDTH+12;
	rt.right = rt.left + BUTTONWIDTH;
	pBtSearch = CreateButton(rt, this, "&CfgPtn.Search", (CTRLPROC)&CPageSysLog::OnBtSearch, NULL, buttonNormalBmp);
	
	rt.left += BUTTONWIDTH+12;
	rt.right = rt.left + BUTTONWIDTH;
	pBtQuit = CreateButton(rt, this, "&CfgPtn.Exit", (CTRLPROC)&CPageSysLog::OnBtQuit, NULL, buttonNormalBmp);
	
	pBtExport->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	pBtSearch->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	pBtQuit->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	
	m_nCurMouseLine = 0;
}

CPageSysLog::~CPageSysLog()
{
	
}

void CPageSysLog::InitPageLeft()
{
	CRect rt;
	rt.left = LEFT_PAGE_L;
	rt.top = FRAME_TOP;
	rt.right = LEFT_PAGE_R;
	rt.bottom = rt.top + CTRL_HEIGHT*10;
	
	pTabPageLeft[0] = CreateTableBox(rt, this, 4, 10);
	pTabPageLeft[0]->SetColWidth(0, 114);
	//csp modify
	//pTabPageLeft[0]->SetColWidth(1, 60-4);
	pTabPageLeft[0]->SetColWidth(1, 60);
	pTabPageLeft[0]->SetColWidth(2, TEXT_WIDTH*5+36+4);
	//pTabPageLeft[0]->SetColWidth(3, TEXT_WIDTH*5);
	
	//csp modify
	//char* szTitle[4] = {"&CfgPtn.Type", "&CfgPtn.Username", "&CfgPtn.Time", "&CfgPtn.IP"};
	char* szTitle[4] = {"&CfgPtn.Type", "&CfgPtn.User", "&CfgPtn.Time", "&CfgPtn.IP"};
	
	CRect rtTemp;
	
	int i = 0;
	for(i=0; i<4; i++)
	{
		pTabPageLeft[0]->GetTableRect(i, 0, &rtTemp);
		
		pTitlePageLeft[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, szTitle[i]);
		pTitlePageLeft[i]->SetBkColor(VD_RGB(67,77,87));
		pTitlePageLeft[i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		pTitlePageLeft[i]->SetTextAlign(VD_TA_XLEFT);
		//pTitlePageLeft[i]->SetTextAlign(VD_TA_LEFT);
	}
	
	for(i=0; i<9; i++)
	{
		pTabPageLeft[0]->GetTableRect(0, i+1, &rtTemp);
		pType[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");
	}
	
	for(i=0; i<9; i++)
	{
		pTabPageLeft[0]->GetTableRect(1, i+1, &rtTemp);
		pUsername[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");
	}
	
	for(i=0; i<9; i++)
	{
		pTabPageLeft[0]->GetTableRect(2, i+1, &rtTemp);
		pTime[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");
	}
	
	for(i=0; i<9; i++)
	{
		pTabPageLeft[0]->GetTableRect(3, i+1, &rtTemp);
		pIP[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");
	}
	
	rt.top = rt.bottom -2;
	rt.bottom = rt.top + CTRL_HEIGHT;
	
	pTabPageLeft[1] = CreateTableBox(rt, this, 1, 1);
	
	char tmp[10] = {0};
	int totalpage =0;
	int curpage = 0;
	rt.left += 5;
	rt.top += 2;
	rt.right = rt.left + TEXT_WIDTH*4;
	rt.bottom -=2;
	sprintf(tmp, "%d/%d", curpage,totalpage);
	pszResult = CreateStatic(rt, this, tmp);
	pszResult->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
	pszResult->SetTextAlign(VD_TA_LEFT);
	
	rt.left = m_Rect.Width()-RIGHT_PAGE_WIDTH - 15 -7 - 40*4 - 10*4;
	for(i =0; i<4; i++)
	{
 		pBmpBtPageCtrl[i][0] = VD_LoadBitmap(szBtBmpPath[i][0]);
 		pBmpBtPageCtrl[i][1] = VD_LoadBitmap(szBtBmpPath[i][1]);
		//printf("VD_LoadBitmap %x %x \n", pBmpBtPageCtrl[i][0], pBmpBtPageCtrl[i][1]);
		pBtPageCtr[i] = CreateButton(CRect(rt.left, rt.top, rt.left+40, rt.bottom), this, NULL, (CTRLPROC)&CPageSysLog::OnClickPageCtr, NULL, buttonNormalBmp);
		rt.left += 40+10;
		pBtPageCtr[i]->SetBitmap(pBmpBtPageCtrl[i][0], pBmpBtPageCtrl[i][1], pBmpBtPageCtrl[i][1]);
	}
}

void CPageSysLog::OnClickPageCtr()
{
	if(curPage < 1)
	{
		return;
	}
	
	if(pLogSearchResult->nRealNum < 10)
	{
		return;
	}
	
	int i = 0;
	BOOL bFind = FALSE;
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
		switch (i)
		{
			case 0:
				if(curPage > 1)
				{
					for(int i = 0; i < 9; i++)
					{
						SetOneRow(1, i, pLogSearchResult);
					}
					curPage = 1;
				}
				break;

			case 1:
				if(curPage > 1)
				{
					for(int i = 0; i < 9; i++)
					{
						SetOneRow(curPage - 1, i, pLogSearchResult);
					}
					curPage -= 1;
				}
				break;

			case 2:
				if(curPage < (pLogSearchResult->nRealNum / 9 + (pLogSearchResult->nRealNum % 9 ? 1 : 0)))
				{
					for(int i = 0; i < 9; i++)
					{
						SetType(i, "");
						SetUsername(i, "");
						SetTime(i, "");
						SetIP(i, "");
					}	
					for(int i = 0; i < 9; i++)
					{
						if(curPage * 9 + i + 1 > pLogSearchResult->nRealNum)
						{
							break;
						}
						SetOneRow(curPage + 1, i, pLogSearchResult);
					}
					curPage += 1;
				}
				break;

			case 3:
				if(curPage < (pLogSearchResult->nRealNum / 9 + (pLogSearchResult->nRealNum % 9 ? 1 : 0)))
				{
					curPage = (pLogSearchResult->nRealNum / 9 + (pLogSearchResult->nRealNum % 9 ? 1 : 0));
					for(int i = 0; i < 9; i++)
					{
						SetType(i, "");
						SetUsername(i, "");
						SetTime(i, "");
						SetIP(i, "");
					}
					for(int i = 0; i < 9; i++)
					{
						if((curPage - 1) * 9 + i + 1 > pLogSearchResult->nRealNum)
						{
							break;
						}
						SetOneRow(curPage, i, pLogSearchResult);
					}
				}
				break;

			default:
				break;
		}

		if ((m_nCursel > 0) && (m_nCursel < 10))
		{
			if(strcmp(pType[m_nCursel-1]->GetText(),"") == 0)
			{
				pType[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
				pType[m_nCursel-1]->Draw();
				pUsername[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
				pUsername[m_nCursel-1]->Draw();
				pTime[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
				pTime[m_nCursel-1]->Draw();
				pIP[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
				pIP[m_nCursel-1]->Draw();
			}
			else
			{
				pType[m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
				pType[m_nCursel-1]->Draw();
				pUsername[m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
				pUsername[m_nCursel-1]->Draw();
				pTime[m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
				pTime[m_nCursel-1]->Draw();
				pIP[m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
				pIP[m_nCursel-1]->Draw();
			}
		}
		
		char tmp[20] = {0};
		sprintf(tmp, "%d%c%d", curPage, '/', (pLogSearchResult->nRealNum % 9) ? (pLogSearchResult->nRealNum / 9 + 1) : (pLogSearchResult->nRealNum / 9));
		SetResult(tmp);					
	}
}

void CPageSysLog::InitPageRight()
{
	CRect rtFrame(RIGHT_PAGE_L+2, FRAME_TOP, RIGHT_PAGE_R-2, FRAME_TOP+CTRL_HEIGHT*11-2);

	pTabPageRight[0] = CreateTableBox(rtFrame, this, 1, 1);

	CRect rtTime;
	rtTime.left = rtFrame.right-114;
	rtTime.right = rtFrame.right-4;
	rtTime.top = rtFrame.top+10;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pBeginDatePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_DATE);

	rtTime.top = rtTime.bottom+2;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pBeginTimePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_TIME);

	rtTime.top = rtTime.bottom+5;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pEndDatePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_DATE);

	rtTime.top = rtTime.bottom+2;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pEndTimePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_TIME);

	pStaticBegin = CreateStatic(CRect(rtFrame.left+2, rtFrame.top+10,rtFrame.right-114,rtFrame.top+34), this, "&CfgPtn.Start");
    pStaticBegin->SetTextAlign(VD_TA_CENTER);
	pStaticEnd = CreateStatic(CRect(rtFrame.left+2, rtFrame.top+75,rtFrame.right-114,rtFrame.top+98), this, "&CfgPtn.End");
    pStaticEnd->SetTextAlign(VD_TA_CENTER);

	CRect searchRt(rtFrame.left, rtTime.bottom+5, rtFrame.right,  FRAME_TOP+CTRL_HEIGHT*11-2);
	pTabChnSearch = CreateTableBox(searchRt, this, 2, 7);
	pTabChnSearch->SetColWidth(0,28);

	CRect tmpRt;
	int i = 0;
	for(i=0; i<7; i++)
	{
		pTabChnSearch->GetTableRect(0,i,&tmpRt);	
		pChk[i] = CreateCheckBox(CRect(searchRt.left+tmpRt.left-1, searchRt.top+tmpRt.top, 
		searchRt.left+tmpRt.left+19,searchRt.top+tmpRt.top+18), 
		this);
		pChk[i]->SetValue(1);
	}

	char* szStatic[7] = 
	{
		"&CfgPtn.Operation",
		"&CfgPtn.Setup",
		"&CfgPtn.Playback",
		"&CfgPtn.Backup",
		"&CfgPtn.Search",
		"&CfgPtn.Information",
		"&CfgPtn.Error",
	};

	for(i=0; i<7; i++)
	{
		pTabChnSearch->GetTableRect(1,i,&tmpRt);	
		pStaticChk[i] = CreateStatic(CRect(searchRt.left+tmpRt.left+2, searchRt.top+tmpRt.top, searchRt.left+tmpRt.right,searchRt.top+tmpRt.bottom+2), this, szStatic[i]);
	}
}


void CPageSysLog::SetType(int row, char* szText)
{
	if(row<0 || row>8)
	{
		return;
	}

	pType[row]->SetText(szText);
}
void CPageSysLog::SetUsername(int row, char* szText)
{
	if(row<0 || row>8)
	{
		return;
	}

	pUsername[row]->SetText(szText);
}

void CPageSysLog::SetUsernameWithChn(int row, u8 chn)
{
	char szText[10];
	if(row<0 || row>8)
	{
		return;
	}
	
	sprintf(szText, "%d", chn);

	pUsername[row]->SetText(szText);
}

void CPageSysLog::SetTime(int row, char* szText)
{
	if(row<0 || row>8)
	{
		return;
	}

	pTime[row]->SetText(szText);
}

void CPageSysLog::SetIP(int row, char* szText)
{
	if(row<0 || row>8)
	{
		return;
	}

	pIP[row]->SetText(szText);
}

void CPageSysLog::SetResult(char* szText)
{
	pszResult->SetText(szText);
}


VD_BOOL CPageSysLog::MsgProc( uint msg, uint wpa, uint lpa )
{

	int px,py;
	int temp;
	switch(msg)
	{
	case XM_LBUTTONDOWN:
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
			int row = 0;
			for(i = 0; i<4; i++)
			{
				for ( row=1; row<10; row++)
				{
					VD_RECT rt;
					pTabPageLeft[0]->GetTableRect(i, row, &rt);
					rt.left +=10+offsetx;
					rt.top +=62+offsety;
					rt.right +=10+offsetx;
					rt.bottom +=62+offsety;
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
				if(msg == XM_MOUSEMOVE)
				{
					MouseMoveToLine(row);
				}				
				else
				{
					VD_BOOL ret = SelectItem(row);
					if(ret && (msg == XM_LBUTTONDBLCLK) && (strcmp(pTime[m_nCursel-1]->GetText(),"") != 0))
					{
						char tmp[200] = {0};
						sprintf(tmp, "%s\n%s\n%s\n%s", pType[m_nCursel-1]->GetText(),
													   pUsername[m_nCursel-1]->GetText(),
													   pTime[m_nCursel-1]->GetText(),
													   pIP[m_nCursel-1]->GetText());
						MessageBox(tmp,"&CfgPtn.SysLogsDetail",MB_ICONINFORMATION|MB_OK);
					}
				}
			}
			else
			{
				if(msg == XM_MOUSEMOVE)
					MouseMoveToLine(0);
			}
		}
		break;
	default:
		break;
	}

	return 	CPageFrame::MsgProc(msg, wpa, lpa);
}

VD_BOOL CPageSysLog::SelectItem( int index )
{
	if( m_nCursel == index)
	{
		return TRUE;
	}
	
	if (index < 1 || index>9)
	{
		return FALSE;
	}

	if( strcmp(pTime[index-1]->GetText(),"") == 0)
	{
		return FALSE;
	}

	if ((m_nCursel > 0) && (m_nCursel < 10))
	{
		pType[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pType[m_nCursel-1]->Draw();
		pUsername[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pUsername[m_nCursel-1]->Draw();
		pTime[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pTime[m_nCursel-1]->Draw();
		pIP[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pIP[m_nCursel-1]->Draw();
	}

	m_nCursel = index;

	pType[m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
	pType[m_nCursel-1]->Draw();
	pUsername[m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
	pUsername[m_nCursel-1]->Draw();
	pTime[m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
	pTime[m_nCursel-1]->Draw();
	pIP[m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
	pIP[m_nCursel-1]->Draw();	

	return TRUE;
}

void CPageSysLog::MouseMoveToLine( int index )
{
	if(index == m_nCurMouseLine)
	{
		return;
	}
	if((m_nCurMouseLine>0) && (strcmp(pType[m_nCurMouseLine-1]->GetText(),"") != 0))
	{
		pType[m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
		pUsername[m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
		pTime[m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
		pIP[m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
	}
	m_nCurMouseLine = index;
	
	if (index < 1 || index>9 )
	{
		return;
	}

	if( strcmp(pType[index-1]->GetText(),"") == 0)
	{
		return ;
	}

	pType[m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
	pUsername[m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
	pTime[m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
	pIP[m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
}

void CPageSysLog::GetLogsFileName(char* name)
{
	time_t now;
	time(&now);
	
	//csp modify 20131213
	int nTimeZone = GetTimeZone();
	now += GetTimeZoneOffset(nTimeZone);
	
	//csp modify
	//struct tm *p = localtime(&now);
	struct tm tm0;
	struct tm *p = &tm0;
	localtime_r(&now, p);
	
	sprintf(name, "Logs_%d%02d%02d%02d%02d%02d.log", p->tm_year+1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
}

void CPageSysLog::OnBtExport()
{
    if (!pLogSearchResult->nRealNum)
    {
        MessageBox("&CfgPtn.NoDataExport", "&CfgPtn.WARNING", MB_ICONWARNING|MB_OK);
        return;
    }
    
	SBizUpdateFileInfoList sPIns;
	memset(&sPIns, 0, sizeof(sPIns));
	sPIns.nMax = 2;
	SBizUpdateFileInfo sBizFileInfo[2];
	memset(sBizFileInfo, 0, sizeof(sBizFileInfo));
	sPIns.pInfo = sBizFileInfo;
	int ret = BizSysComplexUsbDevMountDirlist(&sPIns);
	//printf("ret = %d, usbNum = %d\n", ret, sPIns.nFileNum);
	if((0 != ret) || (sPIns.nFileNum < 1))
	{
		MessageBox("&CfgPtn.NoUsbDevice", "&CfgPtn.WARNING", MB_ICONWARNING|MB_OK);
		return;
	}
	
	char tmp[100] = {0};
	strcpy(tmp, pInfoBar->GetText());
	SetInfo("&CfgPtn.IsExportingLogs");
	
	if(nFlagSearched < 1)
	{
		printf("Do search first before export......\n");
		SYSTEM_TIME start;
		SYSTEM_TIME stop;		
		pBeginDatePR->GetDateTime(&start);
		pBeginTimePR->GetDateTime(&start);
		pEndDatePR->GetDateTime(&stop);
		pEndTimePR->GetDateTime(&stop);
		SBizLogSearchPara para;
		memset(&para, 0, sizeof(para));		
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
		para.nStart = mktime(&tM);
		tM.tm_year = stop.year - 1900;
		tM.tm_mon = stop.month - 1;
		tM.tm_mday = stop.day;
		tM.tm_hour = stop.hour;
		tM.tm_min = stop.minute;
		tM.tm_sec = stop.second;
		tM.tm_isdst = 0;
		tM.tm_wday = 0;
		tM.tm_yday = 0;
		para.nEnd = mktime(&tM);
		
		//csp modify 20131213
		int nTimeZone = GetTimeZone();
		para.nStart -= GetTimeZoneOffset(nTimeZone);
		para.nEnd -= GetTimeZoneOffset(nTimeZone);
		
		for(int i = 0; i < 7; i++)
		{
			para.nMasterMask |= (pChk[i]->GetValue() ? (1 << i) : 0);
		}
		
		if(0 != BizSearchLog(&para, pLogSearchResult))
		{
			printf("search logs failed\n");
			MessageBox("&CfgPtn.ExportLogsFileFailed", "&CfgPtn.WARNING", MB_ICONWARNING|MB_OK);
			SetInfo(tmp);
			return;
		}
	}
	
	char path[100] = {0};
	char filename[30] = {0};
	GetLogsFileName(filename);
	sprintf(path, "%s/%s", sPIns.pInfo[0].szName, filename);
	if(!BizExportLog(pLogSearchResult, (u8*)path))
	{
		printf("ExportLog OK!!!!!!\n");
		MessageBox("&CfgPtn.ExportLogsFileSuccess", "", MB_ICONINFORMATION|MB_OK);
	}
	else
	{
		printf("ExportLog Failed!!!!!!\n");
		MessageBox("&CfgPtn.ExportLogsFileFailed", "&CfgPtn.WARNING", MB_ICONWARNING|MB_OK);
	}
	
	SetInfo(tmp);
}

void CPageSysLog::OnBtSearch()
{
	SYSTEM_TIME start;
	SYSTEM_TIME stop;
	
	pBeginDatePR->GetDateTime(&start);
	pBeginTimePR->GetDateTime(&start);
	pEndDatePR->GetDateTime(&stop);
	pEndTimePR->GetDateTime(&stop);
	
	SBizLogSearchPara para;
	memset(&para, 0, sizeof(para));
	
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
	para.nStart = mktime(&tM);
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
	para.nEnd = mktime(&tM);
	
	//csp modify 20131213
	int nTimeZone = GetTimeZone();
	para.nStart -= GetTimeZoneOffset(nTimeZone);
	para.nEnd -= GetTimeZoneOffset(nTimeZone);
	
	printf("start:%ld, end:%ld\n", para.nStart, para.nEnd);
	
/*/
	time_t now;
	time(&now);
	printf("now:%ld, start:%ld, end:%ld,   %ld\n", now, para.nStart, para.nEnd, mktime(&tM));
	
	//csp modify
	//struct tm *p = localtime(&now);
	struct tm tm0;
	struct tm *p = &tm0;
	localtime_r(&now, p);
	
	printf("%d/%d/%d %d:%d:%d %d\n", p->tm_year+1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, p->tm_isdst);
	p->tm_wday = 0;
	p->tm_yday = 0;
	printf("%lld\n", mktime(p));
	p = &tM;
	printf("%lld\n", mktime(p));
	
	//printf("now:%lld, start:%lld, end:%lld\n", now, para.nStart, para.nEnd);
/*/
	
	for(int i = 0; i < 7; i++)
	{
		para.nMasterMask |= (pChk[i]->GetValue() ? (1 << i) : 0);
	}
	
	SetInfo("&CfgPtn.IsSearchingLogs");
	
	if(0 != BizSearchLog(&para, pLogSearchResult))
	{
		printf("search logs failed\n");
		ClearInfo();
		MessageBox("&CfgPtn.SearchLogsFileFailed", "&CfgPtn.WARNING", MB_ICONWARNING|MB_OK);
		return;
	}

	printf("Mask:%02x, Results:%d\n", para.nMasterMask, pLogSearchResult->nRealNum);
	
	char tmp[20] = {0};
	sprintf(tmp, "%d%c%d", pLogSearchResult->nRealNum ? 1 : 0, '/', 
            (pLogSearchResult->nRealNum % 9) ? (pLogSearchResult->nRealNum / 9 + 1) : (pLogSearchResult->nRealNum / 9));
	SetResult(tmp);

	for(int i = 0; i < 9; i++)
	{
		SetType(i, "");
		SetUsername(i, "");
		SetTime(i, "");
		SetIP(i, "");
	}
	for(int i = 0; i < (pLogSearchResult->nRealNum > 9 ? 9 : pLogSearchResult->nRealNum); i++)
	{
		SetOneRow(1, i, pLogSearchResult);
	}
	
	if(pLogSearchResult->nRealNum)
	{
		SetInfo("&CfgPtn.DoubleClickForDetail");
	}
	else
	{
		ClearInfo();
		MessageBox("&CfgPtn.SearchLogsFileFailed", "&CfgPtn.WARNING", MB_ICONWARNING|MB_OK);
	}
	
	curPage = 1;
	
	nFlagSearched = 1;
	
    //Modify by Lirl on Feb/06/2012,点击搜索按钮时取消之前选中行
	if ((m_nCursel > 0) && (m_nCursel < 10))
	{
		pType[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pType[m_nCursel-1]->Draw();
		pUsername[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pUsername[m_nCursel-1]->Draw();
		pTime[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pTime[m_nCursel-1]->Draw();
		pIP[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pIP[m_nCursel-1]->Draw();
	}
    //end
}

void CPageSysLog::OnBtQuit()
{
	this->Close();
}

void CPageSysLog::SetInfo(char* szInfo)
{
	pInfoBar->SetText(szInfo);
}

void CPageSysLog::ClearInfo()
{
	pInfoBar->SetText("");
}

VD_BOOL CPageSysLog::UpdateData( UDM mode )
{
	//printf("update data PAGE LOGS\n");
	
	static u8 flag = 1;
	ClearInfo();
	
	if(UDM_OPEN == mode)
	{
		for(int i = 0; i < 9; i++)
		{
			SetType(i, "");
			SetUsername(i, "");
			SetTime(i, "");
			SetIP(i, "");
			SetResult("0/0");
		}
		
		if(flag)
		{
			flag = 0;
			pLogSearchResult = (SBizLogResult*)calloc(1, sizeof(SBizLogResult));
			pLogSearchResult->nMaxLogs = 0x4ff;
			pLogSearchResult->psLogList = (SBizLogInfoResult*)calloc(pLogSearchResult->nMaxLogs, sizeof(SBizLogInfoResult));
		} 
		
		if((m_nCursel > 0) && (m_nCursel < 10))
		{
			pType[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pType[m_nCursel-1]->Draw();
			pUsername[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pUsername[m_nCursel-1]->Draw();
			pTime[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pTime[m_nCursel-1]->Draw();
			pIP[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pIP[m_nCursel-1]->Draw();
			m_nCursel = 0;
		}
		
		time_t long_time = time(NULL);
		
		//csp modify 20131213
		int nTimeZone = GetTimeZone();
		long_time += GetTimeZoneOffset(nTimeZone);
		
		//csp modify
		//struct tm *tmptime = localtime(&long_time);
		struct tm tm0;
		struct tm *tmptime = &tm0;
		localtime_r(&long_time, tmptime);
		
		SYSTEM_TIME stTime;
		stTime.year = tmptime->tm_year+1900;
		stTime.month = tmptime->tm_mon+1;
		stTime.day = tmptime->tm_mday;
		stTime.hour = tmptime->tm_hour;
		stTime.minute = tmptime->tm_min;
		stTime.second = tmptime->tm_sec;
    	pBeginDatePR->SetDateTime(&stTime);
		pEndDatePR->SetDateTime(&stTime);
		pEndTimePR->SetDateTime(&stTime);
	}
	else if (UDM_CLOSED == mode)
	{
        pLogSearchResult->nRealNum = 0;
        ((CPageInfoFrameWork *)(this->GetParent()))->ReflushItemName();
    }
	
	curPage = -1;
	nFlagSearched = 0;
	
	return TRUE;
}

VD_BOOL CPageSysLog::GetLogType(unsigned short type, char* tmp)
{
	switch(type)//(src->psLogList[(page-1)*9+row].nType)
	{		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_START_DWELL     :
			strcpy(tmp, "&CfgPtn.LOG_START_DWELL");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_STOP_DWELL      :
			strcpy(tmp, "&CfgPtn.LOG_STOP_DWELL");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_MUTE            :
			strcpy(tmp, "&CfgPtn.LOG_MUTE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_CHANGE_VOICE    :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_VOICE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_CONTROL_PTZ     :
			strcpy(tmp, "&CfgPtn.LOG_CONTROL_PTZ");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_SNAP_SUCCESS    :
			strcpy(tmp, "&CfgPtn.LOG_SNAP_SUCCESS");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_START_MANUAL_REC:
			strcpy(tmp, "&CfgPtn.LOG_START_MANUAL_REC");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_STOP_MANUAL_REC :
			strcpy(tmp, "&CfgPtn.LOG_STOP_MANUAL_REC");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_MANUAL_ALARM    :
			strcpy(tmp, "&CfgPtn.LOG_MANUAL_ALARM");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_CLEAR_ALARM     :
			strcpy(tmp, "&CfgPtn.LOG_CLEAR_ALARM");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_START_MD        :
			strcpy(tmp, "&CfgPtn.LOG_START_MD");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_STOP_MD         :
			strcpy(tmp, "&CfgPtn.LOG_STOP_MD");
			break;
	//yaogang modify 20141127
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_VIDEO_LOST        :
			strcpy(tmp, "&CfgPtn.LOG_VIDEO_LOST");
			break;
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_VIDEO_RESUME         :
			strcpy(tmp, "&CfgPtn.LOG_VIDEO_RESUME");
			break;

		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_START_LOCAL_ALARM        :
			strcpy(tmp, "&CfgPtn.LOG_START_LOCAL_ALARM");
			break;
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_STOP_LOCAL_ALARM         :
			strcpy(tmp, "&CfgPtn.LOG_STOP_LOCAL_ALARM");
			break;

		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_START_IPCEXT        :
			strcpy(tmp, "&CfgPtn.LOG_START_IPCEXT");
			break;
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_STOP_IPCEXT         :
			strcpy(tmp, "&CfgPtn.LOG_STOP_IPCEXT");
			break;
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_START_485EXT        :
			strcpy(tmp, "&CfgPtn.LOG_START_485EXT");
			break;
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_STOP_485EXT         :
			strcpy(tmp, "&CfgPtn.LOG_STOP_485EXT");
			break;
			
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_DISK_LOST         :
			strcpy(tmp, "&CfgPtn.LOG_DISK_LOST");
			break;

		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_DISK_WRERR         :
			strcpy(tmp, "&CfgPtn.LOG_DISK_WRERR");
			break;

		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_DISK_NONE         :
			strcpy(tmp, "&CfgPtn.LOG_DISK_NONE");
			break;
			
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_IPCCOVER         :
			strcpy(tmp, "&CfgPtn.LOG_IPCCOVER");
			break;
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_IPCCOVER_RESUME         :
			strcpy(tmp, "&CfgPtn.LOG_IPCCOVER_RESUME");
			break;
	//yaogang modify end
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_POWER_OFF       :
			strcpy(tmp, "&CfgPtn.LOG_POWER_OFF");
			break;
		
		//csp modify
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_POWER_ON		:
			strcpy(tmp, "&CfgPtn.LOG_POWER_ON");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_REBOOT   	     :
			strcpy(tmp, "&CfgPtn.LOG_REBOOT");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_LOGIN           :
			strcpy(tmp, "&CfgPtn.LOG_LOGIN");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_LOGOFF          :
			strcpy(tmp, "&CfgPtn.LOG_LOGOFF");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_FORMAT_SUCCESS  :
			strcpy(tmp, "&CfgPtn.LOG_FORMAT_SUCCESS");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_FORMAT_FAIL     :
			strcpy(tmp, "&CfgPtn.LOG_FORMAT_FAIL");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONTROL<<8)|BIZ_LOG_SLAVE_UPDATE          :
			strcpy(tmp, "&CfgPtn.LOG_UPDATE");
			break;
		
		//系统设置
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_ENABLE_PWDCHECK             :
			strcpy(tmp, "&CfgPtn.LOG_ENABLE_PWDCHECK");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_DISABLE_PWDCHECK            :
			strcpy(tmp, "&CfgPtn.LOG_DISABLE_PWDCHECK");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_VO                   :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_VO");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_MAX_NETUSER          :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_MAX_NETUSER");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_VIDEOFORMAT          :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_VIDEOFORMAT");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_LANGUAGE             :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_LANGUAGE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_ZONE                 :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_ZONE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_ENABLE_AUTO_TIMESYNC        :
			strcpy(tmp, "&CfgPtn.LOG_ENABLE_AUTO_TIMESYNC");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_DISABLE_AUTO_TIMESYNC       :
			strcpy(tmp, "&CfgPtn.LOG_DISABLE_AUTO_TIMESYNC");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_NTP_SERVER           :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_NTP_SERVER");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_DST_CONFIG                  :
			strcpy(tmp, "&CfgPtn.LOG_DST_CONFIG");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_CHN_NAME             :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_CHN_NAME");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_REC_ENABLE           :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_REC_ENABLE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_REC_AUDIO_ENABLE     :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_REC_AUDIO_ENABLE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_REC_CODESTREAM       :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_REC_CODESTREAM");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_TIME_BEFORE_ALARM    :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_TIME_BEFORE_ALARM");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_TIME_AFTER_ALARM     :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_TIME_AFTER_ALARM");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_REC_EXPIRETIME       :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_REC_EXPIRETIME");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_OSD                  :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_OSD");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_TIMER_RECSCHE        :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_TIMER_RECSCHE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_MD_RECSCHE           :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_MD_RECSCHE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_SENSORALARM_RECSCHE  :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_SENSORALARM_RECSCHE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_SENSOR_ENABLE        :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_SENSOR_ENABLE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_SENSOR_TYPE          :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_SENSOR_TYPE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_SENSOR_ALARMDEAL     :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_SENSOR_ALARMDEAL");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_SENSOR_DETECTION_SCHE:
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_SENSOR_DETECTION_SCHE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_MD_ENABLE            :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_MD_ENABLE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_MD_AREA_SETUP        :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_MD_AREA_SETUP");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_MD_ALARMDEAL         :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_MD_ALARMDEAL");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_MD_DETECTION_SCHE    :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_MD_DETECTION_SCHE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_VIDEOLOSS_ALARMDEAL  :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_VIDEOLOSS_ALARMDEAL");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_ALARMOUT_SCHE        :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_ALARMOUT_SCHE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_ENABLE_SOUND_ALARM          :
			strcpy(tmp, "&CfgPtn.LOG_ENABLE_SOUND_ALARM");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_DISABLE_SOUND_ALARM         :
			strcpy(tmp, "&CfgPtn.LOG_DISABLE_SOUND_ALARM");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_IP                   :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_IP");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_HTTP_PORT            :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_HTTP_PORT");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_SERVER_PORT          :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_SERVER_PORT");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_NET_CODESTREAM       :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_NET_CODESTREAM");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_OUTBOX_CONFIG        :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_OUTBOX_CONFIG");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_INBOX_CONFIG         :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_INBOX_CONFIG");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_DNS_CONFIG           :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_DNS_CONFIG");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_ADD_USER                    :
			strcpy(tmp, "&CfgPtn.LOG_ADD_USER");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_DELETE_USER                 :
			strcpy(tmp, "&CfgPtn.LOG_DELETE_USER");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_USER_AUTH            :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_USER_AUTH");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_PWD                  :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_PWD");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_SERIAL_CONFIG        :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_SERIAL_CONFIG");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_CHANGE_CRUISELINE           :
			strcpy(tmp, "&CfgPtn.LOG_CHANGE_CRUISELINE");
			break;
		
		case (BIZ_LOG_MASTER_SYSCONFIG<<8)|BIZ_LOG_SLAVE_FACTORY_RESET               :
			strcpy(tmp, "&CfgPtn.LOG_FACTORY_RESET");
			break;

		//回放
		case (BIZ_LOG_MASTER_PLAYBACK<<8)|BIZ_LOG_SLAVE_PB_STOP 		:
			strcpy(tmp, "&CfgPtn.LOG_PB_STOP");
			break;
		
		case (BIZ_LOG_MASTER_PLAYBACK<<8)|BIZ_LOG_SLAVE_PB_PAUSE 	:
			strcpy(tmp, "&CfgPtn.LOG_PB_PAUSE");
			break;
		
		case (BIZ_LOG_MASTER_PLAYBACK<<8)|BIZ_LOG_SLAVE_PB_RESUME 	:
			strcpy(tmp, "&CfgPtn.LOG_PB_RESUME");
			break;
		
		case (BIZ_LOG_MASTER_PLAYBACK<<8)|BIZ_LOG_SLAVE_PB_BACKWARD:
			strcpy(tmp, "&CfgPtn.LOG_PB_BACKWARD");
			break;
		
		case (BIZ_LOG_MASTER_PLAYBACK<<8)|BIZ_LOG_SLAVE_PB_FORWARD :
			strcpy(tmp, "&CfgPtn.LOG_PB_FORWARD");
			break;
			
		case (BIZ_LOG_MASTER_PLAYBACK<<8)|BIZ_LOG_SLAVE_PB_START :  //cw_log
			strcpy(tmp, "&CfgPtn.LOG_PB_START");
			break;
			
		//备份
		case (BIZ_LOG_MASTER_BACKUP<<8)|BIZ_LOG_SLAVE_START_BACKUP:
			strcpy(tmp, "&CfgPtn.LOG_START_BACKUP");
			break;

		case (BIZ_LOG_MASTER_BACKUP<<8)|BIZ_LOG_SLAVE_END_BACKUP:
			strcpy(tmp, "&CfgPtn.LOG_END_BACKUP");
			break;

		//搜索
		case (BIZ_LOG_MASTER_SEARCH<<8)|BIZ_LOG_SLAVE_SEARCH_DATA_BYTIME:
			strcpy(tmp, "&CfgPtn.LOG_SEARCH_DATA_BYTIME");
			break;

		case (BIZ_LOG_MASTER_SEARCH<<8)|BIZ_LOG_SLAVE_SEARCH_RECTIME:
			strcpy(tmp, "&CfgPtn.LOG_SEARCH_RECTIME");
			break;

		//查看信息
		case BIZ_LOG_MASTER_CHECKINFO<<8:
			strcpy(tmp, "&CfgPtn.LOG_MASTER_CHECKINFO");
			break;

		//错误
		case BIZ_LOG_MASTER_ERROR<<8:
			strcpy(tmp, "&CfgPtn.LOG_MASTER_ERROR");
			break;

		case (BIZ_LOG_MASTER_ERROR<<8)|BIZ_LOG_SLAVE_ERROR_LOGINPWD:
			strcpy(tmp, "&CfgPtn.LOG_LOGIN_PWD_ERROR");
			break;
			
		case (BIZ_LOG_MASTER_ERROR<<8)|BIZ_LOG_SLAVE_ERROR_HDD_ABNORMAL:
			strcpy(tmp, "&CfgPtn.LOG_ERROR_HDD_ABNORMAL");
			break;
			
		case (BIZ_LOG_MASTER_ERROR<<8)|BIZ_LOG_SLAVE_ERROR_ENC_ABNORMAL:
			strcpy(tmp, "&CfgPtn.LOG_ERROR_ENC_ABNORMAL");
			break;
			
		case (BIZ_LOG_MASTER_ERROR<<8)|BIZ_LOG_SLAVE_ERROR_FILE_ABNORMAL:
			strcpy(tmp, "&CfgPtn.LOG_ERROR_FILE_ABNORMAL");
			break;
		
		default:
			return FALSE;
	}
	
	return TRUE;
}

void CPageSysLog::SetOneRow(int page, int row, SBizLogResult* src)
//void SetOneRow(int page, int row, SBizLogResult* src)
{
	if((page<1) || (row <0) || (row>8) || ((page-1)*9+row+1 > src->nRealNum))
	{
		return;
	}
	
	char tmp[60] = {0};
	
	if(!GetLogType(src->psLogList[(page-1)*9+row].nType, tmp))
	{
		return;
	}
	
	SetType(row,tmp);
	//yaogang modify for add nchn
	u8 master_type = (src->psLogList[(page-1)*9+row].nType >>8) & 0xff;
	u8 slave_type = src->psLogList[(page-1)*9+row].nType & 0xff;
	if ( (master_type == BIZ_LOG_MASTER_SYSCONTROL) 
		&& (slave_type >= BIZ_LOG_SLAVE_START_MD) 
		&& (slave_type <= BIZ_LOG_SLAVE_IPCCOVER_RESUME)
		&& (src->psLogList[(page-1)*9+row].nchn < 17) )//magic1--nchn
	{
		SetUsernameWithChn(row, src->psLogList[(page-1)*9+row].nchn);//在用户名的列显示序号
	}
	else
	{
		SetUsername(row, src->psLogList[(page-1)*9+row].aUsername);
	}
	
	//struct tm *tM = gmtime(&src->psLogList[(page-1)*9+row].nTime);
	
	//csp modify 20131213
	int nTimeZone = GetTimeZone();
	time_t long_time = src->psLogList[(page-1)*9+row].nTime;
	long_time += GetTimeZoneOffset(nTimeZone);
	struct tm stm;
	localtime_r(&long_time, &stm);
	struct tm *tM = &stm;
	
	memset(tmp, 0, sizeof(tmp));
	
	//csp modify 20140406
	sprintf(tmp, "%d%c%02d%c%02d %02d:%02d:%02d", 
			tM->tm_year+1900, '/', tM->tm_mon+1, '/', tM->tm_mday, 
			tM->tm_hour, tM->tm_min, tM->tm_sec);
	SetTime(row,tmp);
	
	strcpy(tmp,src->psLogList[(page-1)*9+row].aIp);
	if(0 == strcasecmp("GUI", tmp))
	{
		strcpy(tmp, "&CfgPtn.LocalGui");
	}
	SetIP(row,tmp);
}

