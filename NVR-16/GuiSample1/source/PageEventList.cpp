#include "GUI/Pages/PageEventList.h"

#define RIGHT_PAGE_WIDTH	180

#define LEFT_PAGE_L	10
#define LEFT_PAGE_R	(m_Rect.Width()-RIGHT_PAGE_WIDTH-18)
#define RIGHT_PAGE_L (m_Rect.Width()-RIGHT_PAGE_WIDTH-15)
#define RIGHT_PAGE_R (m_Rect.Width()-15)
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
CPageEventList::CPageEventList( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
{
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");

	InitPageLeft();
	InitPageRight();

	char* szStatic[3] = 
	{	
		"&CfgPtn.Moving",
		"&CfgPtn.Sensoring",
		"&CfgPtn.VideoLoss"
	};

	CRect rt;
	rt.left = LEFT_PAGE_L;
	rt.right = rt.left + 20;
	rt.top = m_Rect.Height()-m_TitleHeight-38;
	rt.bottom = rt.top + 20;
	
	int i = 0;
	for(i=0; i<3; i++)
	{
		
		pChkType[i] = CreateCheckBox(rt, this);
		pChkType[i]->SetValue(1);

		if(i == 2)
		{	
			rt.left = rt.right + 5;
			rt.right = rt.left + 90;
		}
		else
		{
			rt.left = rt.right + 5;
			rt.right = rt.left + 70;
		}

		pStaticType[i]= CreateStatic(rt,this, szStatic[i]);

		rt.left = rt.right + 5;
		rt.right = rt.left + 20;

	}

	rt.left = RIGHT_PAGE_R - BUTTONWIDTH*3 - 12*2;
	rt.right = rt.left + BUTTONWIDTH;
	rt.top = m_Rect.Height()-m_TitleHeight-40;
	rt.bottom = rt.top+25;
	pBtExport = CreateButton(rt, this, "&CfgPtn.Export", (CTRLPROC)&CPageEventList::OnBtExport, NULL, buttonNormalBmp);

	rt.left += BUTTONWIDTH+12;
	rt.right = rt.left + BUTTONWIDTH;
	pBtSearch = CreateButton(rt, this, "&CfgPtn.Search", (CTRLPROC)&CPageEventList::OnBtSearch, NULL, buttonNormalBmp);

	rt.left += BUTTONWIDTH+12;
	rt.right = rt.left + BUTTONWIDTH;
	pBtQuit = CreateButton(rt, this, "&CfgPtn.Exit", (CTRLPROC)&CPageEventList::OnBtQuit, NULL, buttonNormalBmp);

	pBtExport->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	pBtSearch->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	pBtQuit->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));

}

CPageEventList::~CPageEventList()
{

}

void CPageEventList::InitPageLeft()
{
	CRect rt;
	rt.left = LEFT_PAGE_L;
	rt.top = FRAME_TOP;
	rt.right =  LEFT_PAGE_R;
	rt.bottom =rt.top + CTRL_HEIGHT*10;

	pTabPageLeft[0] = CreateTableBox(rt, this, 4, 10);
	pTabPageLeft[0]->SetColWidth(0, 60);
	pTabPageLeft[0]->SetColWidth(1, TEXT_WIDTH*5);
	pTabPageLeft[0]->SetColWidth(2, TEXT_WIDTH*5+10);
	//pTabPageLeft[0]->SetColWidth(3, TEXT_WIDTH*5);

	char* szTitle[4] = {
		
		"&CfgPtn.Channel", 
		"&CfgPtn.Type", 
		"&CfgPtn.StartTime", 
		"&CfgPtn.EndTime"
	};
	//"Chn", "Type", "Begin", "End"};
	int i = 0;
	CRect rtTemp;
	for (i=0; i<4; i++)
	{
		pTabPageLeft[0]->GetTableRect(i, 0, &rtTemp);

		pTitlePageLeft[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, szTitle[i]);
		pTitlePageLeft[i]->SetBkColor(VD_RGB(67,77,87));
		pTitlePageLeft[i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		pTitlePageLeft[i]->SetTextAlign(VD_TA_LEFT);
	}

	for(i=0; i<9; i++)
	{
		pTabPageLeft[0]->GetTableRect(0, i+1, &rtTemp);
		pChn[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");

	}

	for(i=0; i<9; i++)
	{
		pTabPageLeft[0]->GetTableRect(1, i+1, &rtTemp);
		pType[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");

	}

	for(i=0; i<9; i++)
	{
		pTabPageLeft[0]->GetTableRect(2, i+1, &rtTemp);
		pBegin[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");

	}

	for(i=0; i<9; i++)
	{
		pTabPageLeft[0]->GetTableRect(3, i+1, &rtTemp);
		pEnd[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");
	}

	rt.top = rt.bottom -2;
	rt.bottom =rt.top + CTRL_HEIGHT;

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
	for (i =0; i<4; i++)
	{
 		pBmpBtPageCtrl[i][0] = VD_LoadBitmap(szBtBmpPath[i][0]);
 		pBmpBtPageCtrl[i][1] = VD_LoadBitmap(szBtBmpPath[i][1]);
		//printf("VD_LoadBitmap %x %x \n", pBmpBtPageCtrl[i][0], pBmpBtPageCtrl[i][1]);
		pBtPageCtr[i] = CreateButton(CRect(rt.left, rt.top, rt.left+40, rt.bottom), this, NULL, (CTRLPROC)&CPageEventList::OnClickPageCtr, NULL, buttonNormalBmp);
		rt.left += 40+10;
		pBtPageCtr[i]->SetBitmap(pBmpBtPageCtrl[i][0], pBmpBtPageCtrl[i][1], pBmpBtPageCtrl[i][1]);
	}



}

void CPageEventList::OnClickPageCtr()
{

	printf("OnClickPageCtr \n");

}

void CPageEventList::InitPageRight()
{
	CRect rtFrame(RIGHT_PAGE_L+2, FRAME_TOP, RIGHT_PAGE_R-2, FRAME_TOP+CTRL_HEIGHT*11-2);

	pTabPageLeft[1] = CreateTableBox(rtFrame, this, 1, 1);

	CRect rtTime;
	rtTime.left = rtFrame.right-114;
	rtTime.right = rtFrame.right-4;
	rtTime.top = rtFrame.top+10;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pBeginDatePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_DATE);

	rtTime.top = rtTime.bottom+2;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pBeginTimePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_TIME);

	rtTime.top = rtTime.bottom+8;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pEndDatePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_DATE);

	rtTime.top = rtTime.bottom+2;
	rtTime.bottom = rtTime.top + CTRL_HEIGHT;
	pEndTimePR = CreateDateTimeCtrl(rtTime, this, NULL, DTS_TIME);

	pStaticBegin = CreateStatic(CRect(rtFrame.left+2, rtFrame.top+10,rtFrame.right-114,rtFrame.top+34), this, "&CfgPtn.Start");

	pStaticEnd = CreateStatic(CRect(rtFrame.left+2, rtFrame.top+84,rtFrame.right-114,rtFrame.top+108), this, "&CfgPtn.End");


	pTabChnSearch = CreateTableBox(CRect(rtFrame.left+2, rtTime.bottom+5, rtFrame.right-2, rtTime.bottom+5+25), this, 2, 1);
	pTabChnSearch->SetColWidth(0,24);

	CRect rt;
	rt.left = rtFrame.left+2;
	rt.right = rt.left +24;
	rt.top = rtTime.bottom+5 + 2;
	rt.bottom = rt.top + 18;
	pChkChn[0] = CreateCheckBox(rt, this);
	pChkChn[0]->SetValue(1);

	rt.left = rt.right + 2;
	rt.right = rt.left +24;
	for (int i=1; i<5; i++)
	{
		pChkChn[i] = CreateCheckBox(rt, this);

		rt.left = rt.right;
		rt.right = rt.left +12;
		char tmp[5] = {0};
		sprintf(tmp, "%d", i);
		pStaticChn[i-1] = CreateStatic(rt, this, tmp);
		pStaticChn[i-1]->SetTextAlign(VD_TA_LEFT);

		rt.left = rt.right;
		rt.right = rt.left +24;

		pChkChn[i]->SetValue(1);
	}
}

void CPageEventList::SetChn(int row, char* szText)
{
	if(row<0 || row>8)
	{
		return;
	}

	pChn[row]->SetText(szText);

}

void CPageEventList::SetType(int row, char* szText)
{
	if( row<0 || row>8)
	{
		return;
	}

	pType[row]->SetText(szText);

}

void CPageEventList::SetBegin(int row, char* szText)
{
	if( row<0 || row>8)
	{
		return;
	}

	pBegin[row]->SetText(szText);

}

void CPageEventList::SetEnd(int row, char* szText)
{
	if( row<0 || row>8)
	{
		return;
	}

	pEnd[row]->SetText(szText);

}

void CPageEventList::SetResult(char* szText)
{
	pszResult->SetText(szText);

}

VD_BOOL CPageEventList::UpdateData( UDM mode )
{
	if (UDM_OPEN == mode)
	{
		//printf("CPageEventList:: UDM_OPEN == mode \n");
		SetChn(0,"1");
		SetChn(1,"2");
		SetChn(2,"3");
		SetChn(3,"4");
		
		SetResult("4/4");
		
		time_t long_time = time(NULL);
		
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
		//pBeginTimePR->SetDateTime(&stTime);
		pEndDatePR->SetDateTime(&stTime);
		pEndTimePR->SetDateTime(&stTime);
	}
	
	return TRUE;
}

void CPageEventList::OnBtExport()
{
	
}

void CPageEventList::OnBtSearch()
{
	
}

void CPageEventList::OnBtQuit()
{
	this->Close();
}

