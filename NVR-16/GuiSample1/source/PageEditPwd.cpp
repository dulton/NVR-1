//#include "GUI/Pages/PageRecordFrameWork.h"
//#include "GUI/Pages/PageSensorAlarmFrameWork.h"
#include "GUI/Pages/PageUserManager.h"
#include "GUI/Pages/PageEditPwd.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMessageBox.h"

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;


CPageEditPwd::CPageEditPwd( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
{
	SetMargin(0, 0, 0, 0);
	
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
		
	int rd = 10;
	int space = 78;
	CRect rtTmp;
	
	rtTmp.left = 10;
	rtTmp.right = m_Rect.Width()-10;
	rtTmp.top =  63;
	rtTmp.bottom = rtTmp.top + 25;

	char* szButton[2] = {
		"&CfgPtn.OK",
		"&CfgPtn.Cancel",
	};

	char* szStatiz[3] = {
		"&CfgPtn.OldPassword",
		"&CfgPtn.NewPassword",
		"&CfgPtn.ConfirmPassword"
	};


	int  szStatizLen[3] = {
		TEXT_WIDTH*4,//"&CfgPtn.OldPassword",
		TEXT_WIDTH*4,//"&CfgPtn.NewPassword",
		TEXT_WIDTH*4//"&CfgPtn.ConfirmPassword",
	};
	
	int i = 0;
	for(i=0; i<3; i++)
	{
		pStatic[i] = CreateStatic(rtTmp, this, szStatiz[i]);

		rtTmp.top =  rtTmp.bottom + 5;
		rtTmp.bottom =  rtTmp.top + 25;
		
		int vLen = szStatizLen[i];
		
		pEdit[i] = CreateEdit(rtTmp, this, 6 ,editPassword|edit_KI_NUMERIC,NULL);
		pEdit[i]->SetBkColor(VD_RGB(67,77,87));

		rtTmp.top =  rtTmp.bottom + 3;
		rtTmp.bottom =  rtTmp.top + 25;
	}
	
	rtTmp.left = m_Rect.Width()-180;
	rtTmp.right = rtTmp.left + 80;
	rtTmp.top =  m_Rect.Height() - 30;
	rtTmp.bottom = rtTmp.top + 22;
	
	for(i=0; i<2; i++)
	{
		pButton[i] = CreateButton(rtTmp, this, szButton[i], (CTRLPROC)&CPageEditPwd::OnClkBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
		rtTmp.left = rtTmp.right + 10;
		rtTmp.right = rtTmp.left + 80;
	}
}

CPageEditPwd::~CPageEditPwd()
{

}

VD_PCSTR CPageEditPwd::GetDefualtTitleCenter()
{	
	return "&CfgPtn.Basic";
}

void CPageEditPwd::OnClkBtn()
{
	//printf("OnClkAlarmCfgItem \n");

	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < 2; i++)
	{
		if (pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
		switch(i)
		{
			case 0:
			{
				char buf0[15];
				pEdit[0]->GetText(buf0,14);
				char buf1[15];
				pEdit[1]->GetText(buf1,14);
				char buf2[15];
				pEdit[2]->GetText(buf2,14);
				if((0 == strcmp(buf0, m_oldPasswd)) && (0 == strcmp(buf1, buf2)))
				{
					SBizParaTarget pTarget;
					pTarget.emBizParaType = EM_BIZ_USER_GETUSERINFO;
					SBizUserUserPara userPara;					
					strcpy((char*)userPara.UserName, m_username);
					if(0 != BizGetPara(&pTarget, &userPara))
					{
						printf("Get user info failed\n");
						break;
					}
					strcpy((char*)userPara.Password,buf2);
					pTarget.emBizParaType = EM_BIZ_USER_MODIFYUSER;
					if(0 != BizSetPara(&pTarget, &userPara))
					{
						printf("Set user info failed\n");
						break;
					}
					
					MessageBox("&CfgPtn.ModifyPasswdSuccess", "" , MB_OK|MB_ICONINFORMATION);
					this->Close(UDM_CLOSED);
					printf("OK \n");

					BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_PWD);
				}
				else if(0 != strcmp(buf0, m_oldPasswd))
				{
					MessageBox("&CfgPtn.CheckPasswordFailed", "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
					break;
				}
				else if(0 != strcmp(buf1, buf2))
				{
					MessageBox("&CfgPtn.TwoDifferentPasswd", "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
					break;
				}
				pEdit[0]->Empty();
				pEdit[1]->Empty();
				pEdit[2]->Empty();
			}break;
			case 1:
			{
				//printf("CANCLE \n");
				this->Close(UDM_CLOSED);

				CPageUserManager* FatherPage = (CPageUserManager*)this->GetParent();
				FatherPage->SetFatherPageTitle();
			}break;
			default:
			break;
		}
		
	}
}

void CPageEditPwd::SetUser(char* username)
{
	strcpy(m_username,username);
}

void CPageEditPwd::SetOldPasswd(char* passwd)
{
	strcpy(m_oldPasswd, passwd);
}

VD_BOOL CPageEditPwd::MsgProc(uint msg, uint wpa, uint lpa)
{
    switch(msg) 
    {
        case XM_RBUTTONDOWN:
        case XM_RBUTTONDBLCLK:
            {            
                this->Close(UDM_CLOSED);

				CPageUserManager* FatherPage = (CPageUserManager*)this->GetParent();
				FatherPage->SetFatherPageTitle();
            } break;
        default:
            break;
    }

    ((CPageUserManager *)(this->GetParent()))->RefreshPageString();

    return CPage::MsgProc(msg, wpa, lpa);
}

VD_BOOL CPageEditPwd::Close(UDM mode)
{
    int ret = 0;
    ret = CPage::Close();
    
    CPageUserManager* FatherPage = (CPageUserManager*)this->GetParent();
	FatherPage->SetFatherPageTitle();
    return ret;
}

