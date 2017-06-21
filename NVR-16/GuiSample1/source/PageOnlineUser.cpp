#include "GUI/Pages/PageOnlineUser.h"
#include "GUI/Pages/BizData.h"

char* onlineShortcutBmpName[4][2] = {    
	{DATA_DIR"/temp/listex_btn_leftmost.bmp",	DATA_DIR"/temp/listex_btn_leftmost_f.bmp"},
	{DATA_DIR"/temp/listex_btn_left.bmp",	DATA_DIR"/temp/listex_btn_left_f.bmp"},
	{DATA_DIR"/temp/listex_btn_right.bmp",	DATA_DIR"/temp/listex_right_f.bmp"},
	{DATA_DIR"/temp/listex_btn_rightmost.bmp",	DATA_DIR"/temp/listex_btn_rightmost_f.bmp"},
};

static VD_BITMAP* pBmpButtonNormal[4];
static VD_BITMAP* pBmpButtonSelect[4];


static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

CPageOnlineUser::CPageOnlineUser( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
{
	//SetRect(CRect(0, 0, 700, 400/*370*/), FALSE);
	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);
	
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	CRect rtTemp;
	rtTemp.left = 10;
	rtTemp.top = 10;
	rtTemp.right = m_Rect.Width() - 10;
	rtTemp.bottom = rtTemp.top + 300;
	
	
	char* szTitle[3] = 
	{
		"&CfgPtn.Username",
		"&CfgPtn.IPAddress",
		"&CfgPtn.Status",
	
	};

	char* szBtn[2] =
	{
		"&CfgPtn.Refresh",
		"&CfgPtn.Disconnect",
	};

	//CRect tableRt(rtTemp.left,rtTemp.top,rtTemp.right,rtTemp.bottom);
	pTable0 = CreateTableBox(rtTemp, this, 3, 10);


	int i = 0;
	CRect rt;
	for(i=0; i<3; i++)
	{
		pTable0->GetTableRect(i, 0, &rt);
		pTitle[i] = CreateStatic(CRect(rtTemp.left+rt.left, rtTemp.top+rt.top, rtTemp.left+rt.right,rtTemp.top+rt.bottom),
									this, szTitle[i]);
		pTitle[i]->SetBkColor(VD_RGB(67,77,87));
	}

	for(i=0; i<9; i++)
	{
		pTable0->GetTableRect(0, i+1, &rt);
		pUsername[i] = CreateStatic(CRect(rtTemp.left+rt.left, rtTemp.top+rt.top, rtTemp.left+rt.right,rtTemp.top+rt.bottom),
									this, "");

	}
		
	for(i=0; i<9; i++)
	{
		pTable0->GetTableRect(1, i+1, &rt);
		pIP[i] = CreateStatic(CRect(rtTemp.left+rt.left, rtTemp.top+rt.top, rtTemp.left+rt.right,rtTemp.top+rt.bottom),
									this, "");

	}

	
	for(i=0; i<9; i++)
	{
		pTable0->GetTableRect(2, i+1, &rt);
		pState[i] = CreateStatic(CRect(rtTemp.left+rt.left, rtTemp.top+rt.top, rtTemp.left+rt.right,rtTemp.top+rt.bottom),
									this, "");

	}
	
	debug_trace_err("CPageOnlineUser");
	
	//for(i=0; i<9; i++)
	{
		GetScrStrSetCurSelDo(
			&pUsername[0],
			GSR_INFO_ONLINE_USERNAME, 
			EM_GSR_CTRL_SZLIST, 
			9
		);
		GetScrStrSetCurSelDo(
			&pIP[0],
			GSR_INFO_ONLINE_IP, 
			EM_GSR_CTRL_SZLIST, 
			9
		);
		GetScrStrSetCurSelDo(
			&pState[0],
			GSR_INFO_ONLINE_STATE, 
			EM_GSR_CTRL_SZLIST, 
			9
		);
	}
	
	debug_trace_err("CPageOnlineUser");
	
	rtTemp.left = 10;
	rtTemp.top = rtTemp.bottom-1;
	rtTemp.right = m_Rect.Width() - 10;
	rtTemp.bottom = rtTemp.top + 30;
	pTable1 = CreateTableBox(rtTemp, this, 1, 1);

	pTable1->GetTableRect(0, 0, &rt);
	pResult = CreateStatic(CRect(rtTemp.left+rt.left+2, rtTemp.top+rt.top, rtTemp.left+50,rtTemp.top+rt.bottom),
									this, "0/0");
	
	for(i=0; i<4; i++)
	{
		pTable1->GetTableRect(0, 0, &rt);

		pBmpButtonNormal[i] = VD_LoadBitmap(onlineShortcutBmpName[i][0]);
		pBmpButtonSelect[i] = VD_LoadBitmap(onlineShortcutBmpName[i][1]);
		pCtl[i] = CreateButton(CRect(rtTemp.left+rt.Width()-45*(4-i), rtTemp.top+rt.top+1, rtTemp.left+rt.Width()-45*(4-i)+pBmpButtonNormal[i]->width,rtTemp.top+rt.top+pBmpButtonNormal[i]->height),
									this, NULL, (CTRLPROC)&CPageOnlineUser::OnClickCtl, NULL, buttonNormalBmp);
		pCtl[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
	}

	pInfoBar = CreateStatic(CRect(17, 
								m_Rect.Height()-32-m_TitleHeight,
								317,
								m_Rect.Height()-32-m_TitleHeight+22), 
							this, 
							"");
	//pInfoBar->SetBkColor(VD_RGB(8,66,189));
	
	for(i=0; i<2; i++)
	{

		pBtn[i] = CreateButton(CRect(m_Rect.Width()-120*(2-i), m_Rect.Height()-32-m_TitleHeight, m_Rect.Width()-120*(2-i)+110,m_Rect.Height()-7-m_TitleHeight),
									this, szBtn[i], (CTRLPROC)&CPageOnlineUser::OnClickBtn, NULL, buttonNormalBmp);
		pBtn[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_select);
	}


}

void CPageOnlineUser::OnClickBtn()
{

	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < 2; i++)
	{
		if (pFocusButton == pBtn[i])
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
			{
				printf("Refresh\n");
			}break;
			case 1:
			{
				printf("Disconnect\n");
			}break;
			default:
			break;
		}
	}
}

void CPageOnlineUser::OnClickCtl()
{
	//printf("OnClickSubPage \n");

	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < 4; i++)
	{
		if (pFocusButton == pCtl[i])
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
			{
				printf("left most \n");
			}break;
			case 1:
			{
				printf("left \n");
			}break;
			case 2:
			{
				printf("right \n");
			}break;
			case 3:
			{
				printf("right most \n");
			}break;
			default:
			break;

		}
	}
	
}

CPageOnlineUser::~CPageOnlineUser()
{

}

VD_PCSTR CPageOnlineUser::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

void CPageOnlineUser::SetUsername(int row, char* szText)
{
	if( row<0 || row>8)
	{
		return;
	}

	pUsername[row]->SetText(szText);
		
}

void CPageOnlineUser::SetIP(int row, char* szText)
{
	if( row<0 || row>8)
	{
		return;
	}

	pIP[row]->SetText(szText);

}

void CPageOnlineUser::SetState(int row, char* szText)
{
	if( row<0 || row>8)
	{
		return;
	}

	pState[row]->SetText(szText);
	
}

VD_BOOL CPageOnlineUser::UpdateData( UDM mode )
{
	if (UDM_OPEN == mode)
	{
		for(int i=0; i<9; i++)
		{
			GetScrStrSetCurSelDo(
				&pUsername[0],
				GSR_INFO_ONLINE_USERNAME, 
				EM_GSR_CTRL_SZLIST, 
				9
			);
			GetScrStrSetCurSelDo(
				&pIP[0],
				GSR_INFO_ONLINE_IP, 
				EM_GSR_CTRL_SZLIST, 
				9
			);
			GetScrStrSetCurSelDo(
				&pState[0],
				GSR_INFO_ONLINE_STATE, 
				EM_GSR_CTRL_SZLIST, 
				9
			);
		}
	}
	return TRUE;
}

void CPageOnlineUser::SetInfo(char* szInfo)
{
	pInfoBar->SetText(szInfo);

}

void CPageOnlineUser::ClearInfo()
{
	pInfoBar->SetText("");

}


