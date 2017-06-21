#include "GUI/Pages/PageLogin.h"
#include "GUI/Pages/PageDesktop.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/BizData.h"

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static int GetMaxChnNum() { return GetVideoMainNum(); }

CPageLogin::CPageLogin( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
{
	SetMargin(0, 0, 0, 0);
	
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	
	int rd = 10;
	int space = 78;
	CRect rtTmp;
	
	rtTmp.left = 30;
	rtTmp.right = m_Rect.Width()-40;
	rtTmp.top =  53;
	rtTmp.bottom = rtTmp.top + 25;
	
	char* szButton[2] = 
	{
		"&CfgPtn.Login",
		"&CfgPtn.Exit",
	};
	
	char* szStatiz[2] = 
	{
		"&CfgPtn.Username",
		"&CfgPtn.Password",
	};
	
	int i = 0;
	for(i=0; i<2; i++)
	{
		pStatic[i] = CreateStatic(rtTmp, this, szStatiz[i]);

		rtTmp.top =  rtTmp.bottom + 5;
		rtTmp.bottom =  rtTmp.top + 25;
		
		int vLen = (rtTmp.right-rtTmp.left)/TEXT_WIDTH*2;
		if(i == 1)
		{
			pEdit[i] = CreateEdit(rtTmp, this,vLen,editPassword | edit_KI_NUMERIC,NULL);
			pEdit[i]->SetBkColor(VD_RGB(67,77,87));
		}
		else
		{
			//pEdit[i] = CreateEdit(rtTmp, this,vLen,0,NULL);
			pCombox = CreateComboBox(rtTmp, this, NULL, NULL, NULL, 0);//csp modify
			pCombox->SetCurSel(0);
			pCombox->SetBkColor(VD_RGB(67,77,87));
		}
		
		//pEdit[i]->SetBkColor(VD_RGB(67,77,87));		
		
		rtTmp.top =  rtTmp.bottom + 10;
		rtTmp.bottom =  rtTmp.top + 25;
	}

/*	rtTmp.left = 10;
	rtTmp.right = rtTmp.left + 260;
	rtTmp.top =  m_Rect.Height() - 40;
	rtTmp.bottom = rtTmp.top + 20;

	pInfo = CreateStatic(rtTmp, this, "");
	pInfo->SetTextColor(VD_RGB(255,0,0));
*/
	rtTmp.left = m_Rect.Width()/2 -100;
	rtTmp.right = rtTmp.left + 80;
	rtTmp.top =  m_Rect.Height() - 40;//rtTmp.bottom + 20;//m_Rect.Height() - 40;
	rtTmp.bottom = rtTmp.top + 25;
	
	for(i=0; i<2; i++)
	{
		pButton[i] = CreateButton(rtTmp, this, szButton[i], (CTRLPROC)&CPageLogin::OnClkBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
		rtTmp.left = rtTmp.right + 40;
		rtTmp.right = rtTmp.left + 80;
	}
	
	printf("CPageLogin::CPageLogin,end\n");
}

CPageLogin::~CPageLogin()
{

}

VD_PCSTR CPageLogin::GetDefualtTitleCenter()
{
	
	return "&CfgPtn.Basic";
}
VD_BOOL CPageLogin::UpdateData( UDM mode )
{
	static u8 flag =0;
	if(mode == UDM_OPEN)//(mode == UDM_CLOSED)
	{
		/*
		pInfo->SetText("");
		
		int i = 0;
		for(i=0; i<2; i++)
		{
			pEdit[i]->SetText("");
		}*/
		pEdit[1]->SetText("");
		
		if(flag == 0)
		{
			pUserList = (char**)calloc(128, sizeof(char*));
			for(int i = 0; i < 128; i++)
			{
				pUserList[i] = (char*)calloc(15,1);
			}
			flag = 1;
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
		
		pCombox->RemoveAll();
		for(int i = 0; i < nRealUserNum; i++)
		{
			pCombox->AddString(pUserList[i]);
		}
		pCombox->SetCurSel(0);
		
        #if 1
        usleep(1000*10);
        for(int i=0; i<GetMaxChnNum(); i++)
    	{
    		(((CPageDesktop*)(this->GetParent()))->m_vChannelName[i])->SetText((((CPageDesktop*)(this->GetParent()))->m_vChannelName[i])->GetText());
    	}
        #endif
	} 
	else if (mode == UDM_CLOSED) 
	{
        //printf("***When Login, that will be Refresh The page of the Desk!***\n");
        //((CPageDesktop *)(this->GetParent()))->UpdateData(UDM_OPEN);
        for(int i=0; i<GetMaxChnNum(); i++)
		{
			(((CPageDesktop*)(this->GetParent()))->m_vChannelName[i])->SetText((((CPageDesktop*)(this->GetParent()))->m_vChannelName[i])->GetText());
		}
    }
}

void CPageLogin::OnClkBtn()
{
	//printf("OnClkAlarmCfgItem\n");
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < 2; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		switch(i)
		{
			case 0:
			{
				//printf("Login\n");
				
				char szName[64] = {0};
				char szPwd[64] = {0};
				strcpy(szName, pUserList[pCombox->GetCurSel()]);
				pEdit[1]->GetText(szPwd,64);
				
				SBizUserLoginPara para;
				
				memset(&para, 0, sizeof(para));
				memset(para.UserName, 0, sizeof(para.UserName));
				memset(para.Password, 0, sizeof(para.Password));
				
				strncpy((char*)para.UserName, szName, sizeof(para.UserName));
				strncpy((char*)para.Password, szPwd, sizeof(para.Password));
				
				para.emFrom = EM_BIZUSER_LOGINFROM_GUI;
				
				if(0 == GetAuthCheck())
				{
					SBizParaTarget pTarget;
					pTarget.emBizParaType = EM_BIZ_USER_GETUSERINFO;
					SBizUserUserPara userPara;	
					memset(&userPara, 0, sizeof(userPara));
					strcpy((char*)userPara.UserName, pUserList[pCombox->GetCurSel()]);
					if(0 != BizGetPara(&pTarget, &userPara))
					{
						printf("Get user info failed\n");
						break;
					}
					strncpy((char*)para.Password, (char*)userPara.Password, 15);
				}
				
				u32 g_nUserId;
				s32 ret = BizUserLogin(&para,&g_nUserId);
				if(!ret)
				{
					SetGUILoginedUserID(g_nUserId);
					CPageDesktop::SetLogin(1);
					SetGUIUserIsLogined(1);
					Close();
					SetGUILoginedUserName((char*)para.UserName);//cw_log
					//printf("User [%s] logined from GUI ! ID[%d]\n", GetGUILoginedUserName(), GetGUILoginedUserID());
					//u32 key = (8 << 16) | (0 << 8) | 0;
					u32 key = (10 << 16) | (0 << 8) | 0;
					BizNetUploadAlarm(key);
					BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_LOGIN);
				}
				else if(ret == EM_BIZ_LOGIN_ERR_WRONGPASSWD)
				{
					//u32 key = (8 << 16) | (0 << 8) | 1;
					u32 key = (10 << 16) | (0 << 8) | 1;
					BizNetUploadAlarm(key);
					SetGUILoginedUserName((char*)para.UserName);  //cw_log
					BizGUiWriteLog(BIZ_LOG_MASTER_ERROR, BIZ_LOG_SLAVE_ERROR_LOGINPWD);
					//pInfo->SetText("username or password error !");
					MessageBox("&CfgPtn.CheckPasswordFailed","&CfgPtn.WARNING",MB_OK|MB_ICONWARNING);
				}				
				else if(ret == EM_BIZ_LOGIN_ERR_NOLOGINTWINCE)
				{
					//u32 key = (8 << 16) | (0 << 8) | 1;
					u32 key = (10 << 16) | (0 << 8) | 1;
					BizNetUploadAlarm(key);
					//pInfo->SetText("username or password error !");
					MessageBox("&CfgPtn.userhaslogined","&CfgPtn.WARNING",MB_OK|MB_ICONWARNING);
				}				
				else
				{
					//u32 key = (8 << 16) | (0 << 8) | 1;
					u32 key = (10 << 16) | (0 << 8) | 1;
					BizNetUploadAlarm(key);
					//pInfo->SetText("username or password error !");
					MessageBox("&CfgPtn.UnknownError", "&CfgPtn.WARNING",MB_OK|MB_ICONWARNING);
				}		
				
			}break;
			case 1:
			{
				//printf("CANCLE \n");

				/*pInfo->SetText("");

				for(i=0; i<2; i++)
				{
					pEdit[i]->SetText("");
				}*/
				
				Close();
			}break;
			default:
			break;
		}	
	}	
}

