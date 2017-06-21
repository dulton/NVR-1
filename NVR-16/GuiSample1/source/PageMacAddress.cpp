#include "GUI/Pages/PageMacAddress.h"
#include "GUI/Pages/PageNetwork.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMessageBox.h"

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

CPageEditMacAddress::CPageEditMacAddress( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
{
	SetMargin(0, 0, 0, 0);
	
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	char* szButton[2] = 
	{
		"&CfgPtn.OK",
		"&CfgPtn.Cancel",
	};
	
	int rd = 10;
	int space = 78;
	CRect rtTmp;
	
	rtTmp.left = 60;
	rtTmp.right = m_Rect.Width()-10;
	rtTmp.top =  63;
	rtTmp.bottom = rtTmp.top + 25;

	pStatic[0] = CreateStatic(rtTmp, this, "");
	
	rtTmp.left = 60;
	rtTmp.right = rtTmp.left + ROW_HEIGHT + 4;
	rtTmp.top =  rtTmp.bottom + TEXT_WIDTH;
	rtTmp.bottom =  rtTmp.top + TEXT_WIDTH + 1;
		
	int i = 0;
	for(i=0; i<6; i++)
	{
		pEdit[i] = CreateEdit(rtTmp, this,2,edit_KI_NOCH,NULL);
		pEdit[i]->SetBkColor(VD_RGB(67,77,87));

		rtTmp.left = rtTmp.right + 5;
		rtTmp.right = rtTmp.left + TEXT_WIDTH;
		if(i < 5)
		{
			pStatic[i+1] = CreateStatic(rtTmp, this, ":");
			pStatic[i+1]->SetText(":");
			pStatic[i+1]->SetTextAlign(VD_TA_CENTER);

			rtTmp.left = rtTmp.right + 5;;
			rtTmp.right = rtTmp.left + ROW_HEIGHT + 4;
		}
	}
	pEdit[0]->SetText("00");
	pEdit[0]->Enable(FALSE);

	rtTmp.left = m_Rect.Width()-180;
	rtTmp.right = rtTmp.left + 80;
	rtTmp.top =  m_Rect.Height() - 35;
	rtTmp.bottom = rtTmp.top + 22;
	for(i=0; i<2; i++)
	{
		pButton[i] = CreateButton(rtTmp, this, szButton[i], (CTRLPROC)&CPageEditMacAddress::OnClkBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
		rtTmp.left = rtTmp.right + 10;
		rtTmp.right = rtTmp.left + 80;
	}
}

CPageEditMacAddress::~CPageEditMacAddress()
{
	
}

VD_PCSTR CPageEditMacAddress::GetDefualtTitleCenter()
{
	return "&CfgPtn.Basic";
}

extern "C" int SetDefaultGateway(u32 dwIp);

void CPageEditMacAddress::OnClkBtn()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < 2; i++)
	{
		if (pFocusButton == pButton[i])
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
				char mac[2] = {0};
				int j = 0;
				int n = 0;
				int num = 0;
				int rtn = 0;
				
				SBizParaTarget sParaTgtIns;
				SBizNetPara sCfgIns;
				
				memset(&sCfgIns, 0, sizeof(sCfgIns));
				sParaTgtIns.emBizParaType = EM_BIZ_NETPARA;
				rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
				if(rtn != 0)
				{
					printf("%s get net param error!\n",__func__);
					break;
				}
				for(num = 0; num < 6; num++)
				{
					pEdit[num]->GetText(mac,2);
					for(int i = 0; i<2; i++)
					{
						if(((mac[i] >= '0') && (mac[i] <= '9')) 
							|| ((mac[i] >= 'a') && (mac[i] <= 'f'))
							|| ((mac[i] >= 'A') && (mac[i] <= 'F')))
						{
							sCfgIns.MAC[n] = mac[i];
							n++;
						}
						else
						{
							j = 1;
						}
					}
					sCfgIns.MAC[n] = ':';
					n++;
				}
				sCfgIns.MAC[17]='\0';
				printf("mac:%s\n",sCfgIns.MAC);
				
				if(j)
				{
					MessageBox("&CfgPtn.InvalidMacAddr", "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
					break;
				}
				
				rtn = BizSetPara(&sParaTgtIns, &sCfgIns);
				if(rtn < 0)
				{
					printf("modify MAC error!\n");
					break;
				}
				else
				{
					//printf("before SetMacAddress:%s\n",(char *)sCfgIns.MAC);
					//system("ifconfig");
					
					SetMacAddress((char *)sCfgIns.MAC);
					
					//printf("after SetMacAddress:%s\n",(char *)sCfgIns.MAC);
					//system("ifconfig");
					
					SetDefaultGateway(sCfgIns.GateWayIP);
					//BizNetWriteCfg(&sCfgIns);
				}
				
				m_nPageNetwork->UpdateMacAddress((char *)sCfgIns.MAC);
				this->Close();
			}break;
			case 1:
			{
				this->Close();
			}
			break;
			default:
			break;
		}
	}
}

void CPageEditMacAddress::SetParentPage(CPage* page)
{
	m_nPageNetwork = (CPageNetwork*)page;
}

void CPageEditMacAddress::SetEditValue(const char* szText)
{
	pStatic[0]->SetText(szText);
	int n = 3;
	char mac[2] = {0};
	for(int i = 1; i < 6; i++)
	{
		for(int j = 0; j < 2; j++)
		{
			mac[j] = szText[n];
			n++;
		}
		n++;
		pEdit[i]->SetText(mac);	
	}
}

VD_BOOL CPageEditMacAddress::MsgProc(uint msg, uint wpa, uint lpa)
{
    switch(msg) {
        case XM_RBUTTONDOWN:
        case XM_RBUTTONDBLCLK:
            {            
                this->Close();
            } break;
        default:
            break;
    }

    return CPage::MsgProc(msg, wpa, lpa);
}

