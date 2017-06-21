#include "GUI/Pages/PageNetwork.h"
#include "GUI/Pages/PageMacAddress.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageInfoFrameWork.h"

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

CPageNetwork::CPageNetwork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPageFrame(pRect, psz, icon, pParent)
{
	SetMargin(0,0,0,0);

	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	pFrame = CreateTableBox(CRect(17, 60, m_Rect.Width()-17, m_Rect.Height()-17), this, 1, 1);
	pFrame->SetFrameColor(VD_RGB(56,108,148));

	CRect tableRT(22, 65, m_Rect.Width()-22, m_Rect.Height()-22);
	pTab = CreateTableBox(tableRT, this, 2, MAX_NETWORK_INFO);

	char* szTitle[MAX_NETWORK_INFO] =
	{
		"&CfgPtn.HTTPPort",
		"&CfgPtn.ServerPort",
        "&CfgPtn.MobilePort", //add by Lirl on Nov/03/2011
		"&CfgPtn.IP",
		"&CfgPtn.SubnetMask",
		"&CfgPtn.Gateway",
		"&CfgPtn.PrefeeredDNSServer",
		//"&CfgPtn.AlternateDNSServer", //去掉备用DNS显示项,添加手机端口显示
		"&CfgPtn.NetworkApproach",
		"&CfgPtn.Status",
		"&CfgPtn.DDNS",
		"&CfgPtn.PCMACADDRESS"
	};
	
	int i=0;
	for (i=0; i<MAX_NETWORK_INFO; i++)
	{
		CRect rt;
		pTab->GetTableRect(0, i, &rt);
		pTitle[i] = CreateStatic(CRect(rt.left+tableRT.left+3, rt.top+tableRT.top, rt.right+tableRT.left, rt.bottom+tableRT.top), this, szTitle[i]);
	}
	
	for (i=0; i<MAX_NETWORK_INFO; i++)
	{
		CRect rt;
		pTab->GetTableRect(1, i, &rt);
		pValue[i] = CreateStatic(CRect(rt.left+tableRT.left+3, rt.top+tableRT.top, rt.right+tableRT.left, rt.bottom+tableRT.top), this, "");
	}
	
	char* btntext[1] = 
	{
		"&CfgPtn.MODIFY",
	};
	
	CRect rt;
	
	pTab->GetTableRect(1, MAX_NETWORK_INFO - 1, &rt);
	
	pButton = CreateButton(CRect(rt.left+tableRT.left+200, rt.top+tableRT.top, rt.right+tableRT.left-1, rt.bottom+tableRT.top),this, btntext[0], (CTRLPROC)&CPageNetwork::OnClkBtn, NULL, buttonNormalBmp);
	pButton->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	pButton->Show(FALSE, FALSE);
	
	m_pPageEditMacAddress = new CPageEditMacAddress(CRect(0, 0, 480, 280), "", icon_dvr, this);
	
	m_pPageEditMacAddress->SetParentPage((CPage *)this);
}

CPageNetwork::~CPageNetwork()
{
	
}

void CPageNetwork::OnClkBtn()
{
	//char szStr[18];
	//pValue[10]->GetText(szStr, sizeof(szStr));
	m_pPageEditMacAddress->SetEditValue(pValue[10]->GetText());
	m_pPageEditMacAddress->Open();
}

void CPageNetwork::SetValue(int row, char* szText)
{
	if(row<0 || row>10)
	{
		return;
	}
	
	pValue[row]->SetText(szText);
}

VD_BOOL CPageNetwork::UpdateData( UDM mode )
{
	if (UDM_OPEN == mode)
	{
		#if 1
		GetScrStrSetCurSelDo(
			&pValue[0],
			GSR_INFO_NETINFO_BASE, 
			EM_GSR_CTRL_SZLIST, 
			MAX_NETWORK_INFO
		);
		#endif
	} 
	else if (UDM_CLOSED == mode)
	{
        ((CPageInfoFrameWork *)(this->GetParent()))->ReflushItemName();
    }

	return TRUE;
}

VD_BOOL CPageNetwork::UpdateMacAddress(char *pBuf)
{
	pValue[10]->SetText(pBuf);
}

VD_BOOL CPageNetwork::MsgProc(uint msg, uint wpa, uint lpa)
{
	int px,py;
	static time_t cur = 0;
	static time_t pre = 0;
	static int click_time = 0;
	switch(msg) 
	{
		case XM_LBUTTONDBLCLK:
			{
				px = VD_HIWORD(lpa);
				py = VD_LOWORD(lpa);
				
				VD_RECT rtScreen;
				
				m_pDevGraphics->GetRect(&rtScreen);
				
				int offsetx = (rtScreen.right - m_Rect.Width())/2;
				int offsety = (rtScreen.bottom - m_Rect.Height())/2;
				
				VD_RECT rt;
				
				pTab->GetTableRect(1, MAX_NETWORK_INFO - 1, &rt);
				rt.left += 20 + offsetx;
				rt.top += 66 + offsety;
				rt.right += 24 + offsetx;
				rt.bottom += 66 + offsety;
				if (PtInRect(&rt, px, py))
				{
					cur = time(NULL);
					
					if ((time(NULL) - pre) <= 1)//第一次双击会不计算
					{
						click_time++;
					}
					
					pre = cur;
					
					if(3 == click_time)
					{
						click_time = 0;
						OnClkBtn();
					}
				}
			}
			break;
		case XM_RBUTTONDOWN:
        case XM_RBUTTONDBLCLK:	
	        {            
	            this->Close();
	        }
			break;
		default:
			break;
		}
	return CPage::MsgProc(msg, wpa, lpa);
}

