#include "GUI/Pages/PageUserManager.h"
#include "GUI/Pages/PageEditPwd.h"

#include "GUI/Pages/PageAddUser.h"
#include "GUI/Pages/PageModifyUser.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageConfigFrameWork.h"
#include "GUI/Pages/BizData.h"


static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static char* szBtBmpPath[4][2] = {
	{DATA_DIR"/temp/listex_btn_leftmost.bmp", DATA_DIR"/temp/listex_btn_leftmost_f.bmp"},
	{DATA_DIR"/temp/listex_btn_left.bmp", DATA_DIR"/temp/listex_btn_left_f.bmp"},
	{DATA_DIR"/temp/listex_btn_right.bmp", DATA_DIR"/temp/listex_right_f.bmp"},
	{DATA_DIR"/temp/listex_btn_rightmost.bmp", DATA_DIR"/temp/listex_btn_rightmost_f.bmp"},
};

static VD_BITMAP* pBmpBtPageCtrl[4][2];

CPageUserManager::CPageUserManager( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
, m_nCursel(-1),m_itemNum(0)
{
	//SetRect(CRect(0, 0, 700, 400/*370*/), FALSE);
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

	CRect rtSubPage(m_Rect.left+10, m_Rect.top+10, 
		            m_Rect.Width()-10,m_Rect.top+10+290);

	pTable = CreateTableBox(&rtSubPage,this, 3,10,0);
	//pTable->SetColWidth(0,300);
	//pTable->SetColWidth(1,120);
	pTable->SetColWidth(0,(m_Rect.Width()-10)/3);
	pTable->SetColWidth(1,(m_Rect.Width()-10)/3);

	char* szTitle[3] = {
		"&CfgPtn.Username",
		"&CfgPtn.UserType",
		"&CfgPtn.PCMACADDRESS",
	};
		
	CRect tmpRt;
	int i = 0;
	for(i=0; i<3; i++)
	{
		pTable->GetTableRect(i,0,&tmpRt);
		pStatic[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, szTitle[i]);
		pStatic[i]->SetTextAlign(VD_TA_XLEFT);
		pStatic[i]->SetBkColor(VD_RGB(67,77,87));
	}	

	int j = 0;
	for(j=0; j<9; j++)
	{
		for(i=0; i<3; i++)
		{
			int index = j*3 + i + 3;
			pTable->GetTableRect(i,j+1,&tmpRt);
			pStatic[index] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, "");
		}
	}
	
	CRect rt(m_Rect.left+10, m_Rect.top+10+290-2, 
		    m_Rect.Width()-10,m_Rect.top+10+290+CTRL_HEIGHT);
	//rt.left = rtSubPage.left+10;
	//rt.top = rtSubPage.bottom -2;
	//rt.bottom =rt.top + CTRL_HEIGHT;
	pTabPage = CreateTableBox(rt, this, 1, 1);
	
	rt.left += 15;
	rt.top += 5;
	rt.right = rt.left + TEXT_WIDTH*4;
	rt.bottom -= 4;
	pszResult = CreateStatic(rt, this, "");
	pszResult->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
	pszResult->SetTextAlign(VD_TA_LEFT);
	
	#if 0//csp modify
	rt.left = rt.Width() + 300;
	rt.top -= 5;
	for (i =0; i<4; i++)
	{
 		pBmpBtPageCtrl[i][0] = VD_LoadBitmap(szBtBmpPath[i][0]);
 		pBmpBtPageCtrl[i][1] = VD_LoadBitmap(szBtBmpPath[i][1]);
		pBtPageCtr[i] = CreateButton(CRect(rt.left, rt.top+3, rt.left+pBmpBtPageCtrl[i][0]->width, rt.top+pBmpBtPageCtrl[i][0]->height+3),
		                            this, NULL, (CTRLPROC)&CPageUserManager::OnClickPageCtr, NULL, buttonNormalBmp);
		rt.left += 40+10;
		pBtPageCtr[i]->SetBitmap(pBmpBtPageCtrl[i][0], pBmpBtPageCtrl[i][1], pBmpBtPageCtrl[i][1]);
	}
	#endif
	
	char* btntext[5] = {
		"&CfgPtn.ADD",
		"&CfgPtn.Setup",
		"&CfgPtn.Delete",
		"&CfgPtn.Password",
		"&CfgPtn.Exit",
	};
	
	CRect btnRt(10,m_Rect.Height()-22-10-m_TitleHeight-5,10+80,m_Rect.Height()-10-m_TitleHeight-3);
	
	for(i=0; i<5; i++)
	{	
		int btWidth = 5*TEXT_WIDTH;//strlen(btntext[i])*TEXT_WIDTH/2+20;
		//btnRt.right = btnRt.left + btWidth;
		pButton[i] = CreateButton(btnRt,this, btntext[i], (CTRLPROC)&CPageUserManager::OnClkBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
		
		if(i==2)
		{
			btnRt.left = btnRt.right + 10;
			btnRt.right = btnRt.left + 100;
		}
		else if(i==3)
		{
			btnRt.left = m_Rect.Width()-80-10;;
			btnRt.right = btnRt.left + 80;
		}
		else
		{
			btnRt.left = btnRt.right + 10;
			btnRt.right = btnRt.left + 80;
		}
	}	
	
	m_pPageEditPwd = new CPageEditPwd(CRect(0, 0, 480, 280), "&CfgPtn.ChangePassword"/*"&titles.mainmenu"*/, icon_dvr, this);
	m_pPageModifyUser = new CPageModifyUser(NULL, "&CfgPtn.MODIFYUSER"/*"&titles.mainmenu"*/, icon_dvr, this);
	m_pPageAddUser = new CPageAddUser(NULL, "&CfgPtn.ADDUSER"/*"&titles.mainmenu"*/, icon_dvr, this);
	
	m_nCurMouseLine = 0;
}

#if 0//csp modify
void CPageUserManager::OnClickPageCtr()
{
	if(curPage < 1)
	{
		return;
	}
	if(nRealUserNum < 10)
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
	
	if(bFind)
	{
        //CleanUpBckCol();
		switch(i)
		{
			case 0:
				if(curPage > 1)
				{
                    CleanUpBckCol();
					for(int i = 0; i < 9; i++)
					{
						SetOneRow(1, i+1, pUserList);
					}
					curPage = 1;
				}
				break;
			case 1:
				if(curPage > 1)
				{
                    CleanUpBckCol();
					for(int i = 0; i < 9; i++)
					{
						SetOneRow(curPage - 1, i+1, pUserList);
					}
					curPage -= 1;
				}
				break;
			case 2:
				if(curPage < (nRealUserNum / 9 + (nRealUserNum % 9 ? 1 : 0)))
				{
                    CleanUpBckCol();
					for(int i = 0; i < 9; i++)
					{
						SetTable(0, i+1, "");
						SetTable(1, i+1, "");
						SetTable(2, i+1, "");						
					}
					
					for(int i = 0; i < 9; i++)
					{
						SetOneRow(curPage + 1, i+1, pUserList);
					}
					curPage += 1;
				}
				break;
			case 3:
				if(curPage < (nRealUserNum / 9 + (nRealUserNum % 9 ? 1 : 0)))
				{
                    CleanUpBckCol();
					curPage = (nRealUserNum / 9 + (nRealUserNum % 9 ? 1 : 0));
					for(int i = 0; i < 9; i++)
					{
						SetTable(0, i+1, "");
						SetTable(1, i+1, "");
						SetTable(2, i+1, "");
					}
					
					for(int i = 0; i < 9; i++)
					{
						SetOneRow(curPage, i+1, pUserList);
					}
				}
				break;
			default:
				break;
		}
		
		char tmp[20] = {0};
		sprintf(tmp, "%d/%d", curPage, (nRealUserNum%9)?(nRealUserNum/9+1):(nRealUserNum/9));
		pszResult->SetText(tmp);
	}
}
#endif

void CPageUserManager::SetTable(int col,int row,char* text)
{
	int index = row*3 + col/* + 3*/;
	
	if(index<3 || index >=30)
	{
		return;
	}
	
	pStatic[index]->SetText(text);
	
	m_itemNum ++;
}

CPageUserManager::~CPageUserManager()
{
	
}

void CPageUserManager::OnClkBtn()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < 5; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	u32 g_nUserId = GetGUILoginedUserID();
	
	if(bFind)
	{
		if((i == 1) || (i == 2) || (i == 3))
		{
			if(m_nCursel < 1)
			{
				return;
			}
		}
		
		if((i == 1) || (i == 2) || (i == 0))
		{
			EMBIZUSERCHECKAUTHOR author = BizUserCheckAuthority(EM_BIZ_USER_AUTHOR_USER_MANAGE,NULL,g_nUserId);
			if(author == EM_BIZ_USER_AUTHOR_YES)
			{
				//
			}
			else if(author == EM_BIZ_USER_AUTHOR_NO)
			{
				MessageBox("&CfgPtn.NoAuthority", "&CfgPtn.WARNING"  , MB_OK|MB_ICONWARNING);
				return;
			}
			else
			{
				return;
			}
		}
		
		switch(i)
		{
			case 0:
			{
				//printf("add\n");
				m_pPageAddUser->Open();
			}break;
			case 1:
			{
				//printf("setup\n");
				m_pPageModifyUser->SetUser(pUserList[m_nCursel-1+9*(curPage-1)]);
				m_pPageModifyUser->Open();
				
			}break;
			case 2:
			{
                //add by Lirl on Nov/18/2011
                if(strcasecmp("admin", pUserList[m_nCursel-1+9*(curPage-1)]) == 0)
				{
                    MessageBox("&CfgPtn.UserAdminDeleteFails", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
                    return;
                }
                //end
				
				UDM ret1 = MessageBox("&CfgPtn.Deletethisuser", "&CfgPtn.WARNING" , MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2);
				if(UDM_OK != GetRunStatue())
				{
					break;
				}
				
				s32 ret = BizUserDeleteUser(pUserList[m_nCursel-1+9*(curPage-1)]);
				if(0 == ret)
				{
					//
				}
				else if(EM_BIZ_DELUSER_INVALID_NAME == ret)
				{
					MessageBox("&CfgPtn.NoAuthority", "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
					break;
				}
				else if(EM_BIZ_DELUSER_USER_LOGINED == ret)
				{
					MessageBox("&CfgPtn.Userdeletelogined", "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
					break;
				}
				else
				{
					MessageBox("&CfgPtn.UnknownError", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
					break;
				}
				
				//SetTable(0,nRealUserNum-1,"");
				//SetTable(1,nRealUserNum-1,"");
				//SetTable(2,nRealUserNum-1,"");	
				m_itemNum -=3;
				SelectItem(m_nCursel-1);
				UpdateData(UDM_OPENED);
				
				BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_DELETE_USER);

				//printf("delete  \n");
			}
			break;
			case 3:
			{
				//printf("edit pwd  \n");

				SBizParaTarget pTarget;
				pTarget.emBizParaType = EM_BIZ_USER_GETUSERINFO;
				SBizUserUserPara userPara;					
				strcpy((char*)userPara.UserName, GetGUILoginedUserName());
				if(0 != BizGetPara(&pTarget, &userPara))
				{
					printf("Get user info failed\n");
					break;
				}

				if((0 != strcasecmp("administrator",(char*)userPara.GroupBelong))
                    && (0 != strcasecmp(pUserList[m_nCursel-1+9*(curPage-1)], GetGUILoginedUserName())))
				{
					MessageBox("&CfgPtn.NoAuthority", "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
					break;
				}
				
				strcpy((char*)userPara.UserName, pUserList[m_nCursel-1+9*(curPage-1)]);
				if(0 != BizGetPara(&pTarget, &userPara))
				{
					printf("Get user info failed\n");
					break;
				}

				m_pPageEditPwd->SetUser((char*)userPara.UserName);
				m_pPageEditPwd->SetOldPasswd((char*)userPara.Password);
				m_pPageEditPwd->Open();
			}
			break;
			case 4:
			{
				//printf("exit \n");
                CleanUpBckCol();
				this->Close();				
			}break;
			default:
			break;
		}
		
	}
}

VD_PCSTR CPageUserManager::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

int CPageUserManager::GetCurUserNum()//cw_tab
{
	int num;
	if (nRealUserNum<9)
	{
		num = nRealUserNum;
	}
	else 
	{
		if(curPage*9>nRealUserNum)
		{
			num = nRealUserNum-((curPage-1)*9);
		}
		else
		{
			num = 9;
		}
	}
	return num;
}



VD_BOOL CPageUserManager::MsgProc( uint msg, uint wpa, uint lpa )
{
	int px,py;
	int temp;
	static int chSel=0;
	switch(msg)
	{
		case XM_KEYDOWN://cw_tab
		{
			switch(wpa)
			{
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
					chSel=wpa -KEY_0;
					if(chSel<=GetCurUserNum() && chSel>=KEY_0)
					{
						SelectItem(chSel);
					}
				}break;				
				case KEY_UP:
				{
					if (--chSel>=KEY_0)
						SelectItem(chSel);
					else
					{	
						chSel=GetCurUserNum();
						SelectItem(GetCurUserNum());
					}
				}break;				
				case KEY_DOWN:
				{
					if (++chSel<=GetCurUserNum())
						SelectItem(chSel);
					else
					{
						chSel=KEY_0;
						SelectItem(chSel);
					}
				}break;				
				default: 
				    return CPageFrame::MsgProc(msg, wpa, lpa);
			}
			return TRUE;
		}break;
        case XM_RBUTTONDBLCLK:
        case XM_RBUTTONDOWN:
            {
                CleanUpBckCol();
            } break;
		case XM_LBUTTONDOWN:
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
			for(i = 0; i<3; i++)
			{
				for ( row=1; row<10; row++)
				{
					VD_RECT rt;
					pTable->GetTableRect(i, row, &rt);
					rt.left +=10+offsetx;
					rt.top +=52+offsety;
					rt.right +=10+offsetx;
					rt.bottom +=52+offsety;
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
				if(msg == XM_LBUTTONDOWN)
				{
					SelectItem(row);
				}
				else
				{
					MouseMoveToLine(row);
				}
			}
			else
			{
				if(msg == XM_MOUSEMOVE)
				{
					MouseMoveToLine(0);
				}
			}
		}
		break;
	default:
		break;
	}

	return 	CPageFrame::MsgProc(msg, wpa, lpa);
}

void CPageUserManager::SelectItem( int index )
{
	if (index < 1 || index>m_itemNum )
	{
		return;
	}

	//printf(" idx=%d, cur =%d \n", index, m_nCursel);

	if (index == m_nCursel)
	{
		return;
	}

	if( strcmp(pStatic[index*3]->GetText(),"") == 0)
	{
		return ;
	}

	if (m_nCursel >0)
	{
		for(int i=0; i<3; i++)
		{
			pStatic[m_nCursel*3+i]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pStatic[m_nCursel*3+i]->Draw();
		}		
	}

	m_nCursel = index;
		
	for(int i=0; i<3; i++)
	{
		pStatic[m_nCursel*3+i]->SetBkColor(VD_RGB(56,108,148));
		pStatic[m_nCursel*3+i]->Draw();
	}
}

void CPageUserManager::RefreshPageString(void)
{
    int i, j;
    int index;

    for(j = 0; j <= 9; j++)
	{
		for(i = 0; i < 3; i++)
		{
			index = j*3 + i;
			pStatic[index]->SetText(pStatic[index]->GetText());
		}
	}
}

void CPageUserManager::MouseMoveToLine( int index )
{
	if(index == m_nCurMouseLine)
	{
		return;
	}
	
	if((m_nCurMouseLine>0)
        && (strcmp(pStatic[m_nCurMouseLine*3]->GetText(),"") != 0))
	{
		for(int i=0; i<3; i++)
		{
			pStatic[m_nCurMouseLine*3+i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
		}
	}
	m_nCurMouseLine = index;
	
	if (index < 1 || index>9 )
	{
		return;
	}

	if( strcmp(pStatic[index*3]->GetText(),"") == 0)
	{
		return ;
	}

	for(int i=0; i<3; i++)
	{
		pStatic[index*3+i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
	}	
}

void CPageUserManager::CleanUpBckCol()
{
	if (m_nCursel >0)
	{
		for(int i=0; i<3; i++)
		{
			pStatic[m_nCursel*3+i]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pStatic[m_nCursel*3+i]->Draw();
		}		
	}
    
    m_nCursel = 0;
}

VD_BOOL CPageUserManager::UpdateData( UDM mode )
{
	//printf("File:%s, Func:%s, Mode:%d\n", __FILE__,__FUNCTION__,mode);
	static time_t now = 0;
	if(UDM_OPEN == mode)
	{
		now = time(NULL);
	}
	
	if((UDM_OPENED == mode) && (time(NULL)-now<1))
	{
		return TRUE;
	}

	static u8 flag = 0;

	if(UDM_CLOSED == mode)
	{
		//printf("PageUserManager closed, set curpage = 0\n");
		curPage = 0;
		for(int i = 0; i < 9; i++)
		{
			SetTable(0, i+1, "");
			SetTable(1, i+1, "");
			SetTable(2, i+1, "");
		}

        ((CPageConfigFrameWork *)(this->GetParent()))->ReflushItemName();
	}
	
	if((UDM_OPEN == mode) || (UDM_OPENED == mode))
	{
		if(UDM_OPEN == mode)
		{
			curPage = 1;
		}
		
		if(flag == 0)
		{
			pUserList = (char**)calloc(128, sizeof(char*));
			for(int i = 0; i < 128; i++)
			{
				pUserList[i] = (char*)calloc(15,1);
			}
			flag =1;
		}
		
		SBizParaTarget pTarget;
		pTarget.emBizParaType = EM_BIZ_USER_GETUSERLIST;
		
		SBizUserListPara para;
		para.list = pUserList;
		para.nLength = 128;
		
		if(0 != BizGetPara(&pTarget, &para))
		{
			printf("Get user list failed\n");
			return FALSE;
		}
		nRealUserNum = para.nRealLength;
		//printf("Get user list OK! user number :%d\n", nRealUserNum);

		if((0 == nRealUserNum%9) && (nRealUserNum/9+1 == curPage))
		{
			curPage -= 1;
			if(curPage < 1)
			{
				return FALSE;
			}
		}

		if(UDM_OPENED == mode)
		{
			for(int i = 0; i < 9; i++)
			{
				SetTable(0, i+1, "");
				SetTable(1, i+1, "");
				SetTable(2, i+1, "");
			}
		}
		
		for(int i=0; i<9; i++)
		{
			SetOneRow(curPage,i+1,pUserList);
		}
		
		#if 0//csp modify
		char tmp[10] = {0};
		sprintf(tmp, "%d/%d", curPage, (nRealUserNum%9)?(nRealUserNum/9+1):(nRealUserNum/9));
		pszResult->SetText(tmp);
		#endif
	}
	
	return TRUE;
}

void CPageUserManager::SetOneRow(int page, int index, char** pUserList)
{
	if((page < 1) || (index < 1) || (index > 9) || (9*(page-1)+index > nRealUserNum) || (NULL == pUserList))
	{
		return;
	}
	
	SBizParaTarget pTarget;
	pTarget.emBizParaType = EM_BIZ_USER_GETUSERINFO;
	SBizUserUserPara userPara;	
	memset(&userPara, 0, sizeof(userPara));
	strcpy((char*)userPara.UserName, pUserList[9*(page-1)+index-1]);
	if(0 != BizGetPara(&pTarget, &userPara))
	{
		printf("Get user info failed\n");
		return;
	}

	SetTable(0,index,(char*)userPara.UserName);
	if(!strcasecmp((char*)userPara.GroupBelong,"Administrator"))
	{
		SetTable(1,index,"&CfgPtn.AdministratorUser");
	}
	else
	{
		SetTable(1,index,"&CfgPtn.GuestUser");
	}

	if(userPara.emIsBindPcMac)
	{
		char mac[20] = {0};
		sprintf(mac, "%012llx", userPara.PcMacAddress);
		char mac2[20] = {0};
		int j = 0;
		for(int i = 0; i<strlen(mac); i++)
		{
			mac2[i+j] = mac[i];
			if((i%2) && (i<strlen(mac)-1))
			{
				j++;
				mac2[i+j] = '-';
			}
		}
		SetTable(2,index,mac2);
	}
	else
	{
		SetTable(2,index,"&CfgPtn.NOBINDPCMAC");
	}
	
}

void CPageUserManager::SetFatherPageTitle()
{
	for(int i=0; i<3; i++)
	{
		pStatic[i]->SetText((pStatic[i]->GetName()).c_str());
	}
}


