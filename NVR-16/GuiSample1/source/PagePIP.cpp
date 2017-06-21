#include "GUI/Pages/PagePIP.h"
#include "GUI/Pages/PageDesktop.h"
#include "GUI/Pages/PageStartFrameWork.h"
#include "GUI/Pages/BizData.h"

static char* chnShortcutBmpName[2][2] = {    
	{DATA_DIR"/temp/ok.bmp",	DATA_DIR"/temp/ok_f.bmp"},
	{DATA_DIR"/temp/exit2.bmp",	DATA_DIR"/temp/exit2_f.bmp"},	
};

static VD_BITMAP* pBmpButtonNormal[2] = {NULL};
static VD_BITMAP* pBmpButtonSelect[2] = {NULL};

CPagePIP::CPagePIP( 
	VD_PCRECT 	pRect,
	VD_PCSTR 	psz /*= NULL*/,
	VD_BITMAP* 	icon /*= NULL*/,
	CPage * 	pParent /*= NULL*/
 ):CPageFloat(pRect, pParent)
{
	m_bChnSelectPage = TRUE;
	
	m_nChMax = GetVideoMainNum();
	//printf("CPagePIP m_nChMax=%d\n",m_nChMax);
	
	int i;
	CRect rtTmp;
	
	char *pNote = (char*)GetParsedString("&CfgPtn.PIPLargeCH");
	if(strcmp(pNote,"CfgPtn.PIPLargeCH") == 0)
	{
		pNote = "Large";
	}
	
	rtTmp.left = 6;
	rtTmp.top = 6;
	rtTmp.right = rtTmp.left + 64;
	rtTmp.bottom = rtTmp.top + 20;
	pStatic[0] = CreateStatic(&rtTmp, this, pNote);
	
	pNote = (char*)GetParsedString("&CfgPtn.PIPSmallCH");
	if(strcmp(pNote,"CfgPtn.PIPSmallCH") == 0)
	{
		pNote = "Small";
	}
	
	rtTmp.top = rtTmp.bottom + 8;
	rtTmp.bottom = rtTmp.top + 20;
	pStatic[1] = CreateStatic(&rtTmp, this, pNote);
	
	rtTmp.left = rtTmp.right + 8;
	rtTmp.top = 6;
	rtTmp.right = rtTmp.left + 60;
	rtTmp.bottom = rtTmp.top + 20;
	pComboBox[0] = CreateComboBox(&rtTmp, this, NULL, NULL, NULL, 0);
	for(i=0;i<m_nChMax;i++)
	{
		char info[8] = {0};
		sprintf(info,"%d",i+1);
		//printf("AddString %s\n",info);
		pComboBox[0]->AddString(info);
	}
	pComboBox[0]->SetCurSel(0);
	
	rtTmp.top = rtTmp.bottom + 8;
	rtTmp.bottom = rtTmp.top + 20;
	pComboBox[1] = CreateComboBox(&rtTmp, this, NULL, NULL, NULL, 0);
	for(i=0;i<m_nChMax;i++)
	{
		char info[8] = {0};
		sprintf(info,"%d",i+1);
		//printf("AddString %s\n",info);
		pComboBox[1]->AddString(info);
	}
	//pComboBox[1]->SetCurSel(0);
	pComboBox[1]->SetCurSel(1);
	
	rtTmp.top = rtTmp.bottom + 12;
	rtTmp.right = rtTmp.left - 6;
	
	for(i=0; i<2; i++)
	{
		pBmpButtonNormal[i] = VD_LoadBitmap(chnShortcutBmpName[i][0]);
		pBmpButtonSelect[i] = VD_LoadBitmap(chnShortcutBmpName[i][1]);
		
		rtTmp.left = rtTmp.right + 6;
		rtTmp.right = rtTmp.left + pBmpButtonNormal[i]->width;
		rtTmp.bottom = rtTmp.top + pBmpButtonNormal[i]->height;
		
		pButton[i] = CreateButton(rtTmp, this, NULL, (CTRLPROC)&CPagePIP::OnClkButton, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
	}
}

CPagePIP::~CPagePIP()
{
	
}

void CPagePIP::StartPIP()
{
	int large = pComboBox[0]->GetCurSel();
	int small = pComboBox[1]->GetCurSel();
	
	if(large == small)
	{
		char *pNote = (char*)GetParsedString("&CfgPtn.PIPCHError");
		char *pErr = (char*)GetParsedString("&CfgPtn.WARNING");
		if(strcmp(pNote,"CfgPtn.PIPCHError") == 0)
		{
			pNote = "Please select two different channels!";
		}
		UDM ret = MessageBox(pNote, pErr, MB_OK|MB_ICONWARNING);
		if(ret != UDM_OK)
		{
			//UpdateData(UDM_OPEN);
			//return;
		}
		UpdateData(UDM_OPEN);
		return;
	}
	
	u8 nModePara = ((large & 0xf) << 4) | (small & 0xf);
	
	((CPageStartFrameWork *)(this->GetParent()))->SetPIPPreviewMode(nModePara);
	
	this->Close();
}

void CPagePIP::OnClkButton()
{
	int i = 0;
	u8 nMaxChnNum = GetVideoMainNum();
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
		case 0://OK
			StartPIP();
			break;
		case 1://CANCLE
			this->Close();
			break;
		default:
			break;
		}
	}
}

VD_BOOL CPagePIP::UpdateData( UDM mode )
{
	if(mode == UDM_OPEN)
	{
		
	}
	
	return TRUE;
}

VD_BOOL CPagePIP::MsgProc( uint msg, uint wpa, uint lpa )
{
    switch(msg)
	{
		case XM_KEYDOWN:
		{
			switch(wpa)
			{
				case KEY_RET:
				case KEY_UP:
				case KEY_DOWN:
				case KEY_RIGHT:
				case KEY_LEFT:
				case KEY_ESC:
					break;
				default:
					return false;
			}
		}
		break;
		case XM_RBUTTONDOWN:
		case XM_RBUTTONDBLCLK:
		{
			
		}
		break;
    }
	
	return CPage::MsgProc(msg, wpa, lpa);
}

