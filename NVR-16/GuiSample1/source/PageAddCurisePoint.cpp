#include "GUI/Pages/PageAddCurisePoint.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageCruisePoint.h"


const int nLeft = 4;
const int nTop = 4;

const int nStaticWidth = 60;
const int nStaticHeight = 20;

const int nComboxWidth = 100;
const int nComboxHeight = 20;

const int nButtonWidth = 30;

#define CHN_BUTTON_NUM 2

static char* chnShortcutBmpName[CHN_BUTTON_NUM][2] = {    
	{DATA_DIR"/temp/ok.bmp",	DATA_DIR"/temp/ok_f.bmp"},
	{DATA_DIR"/temp/exit2.bmp",	DATA_DIR"/temp/exit2_f.bmp"},	
};

static VD_BITMAP* pBmpButtonNormal[CHN_BUTTON_NUM];
static VD_BITMAP* pBmpButtonSelect[CHN_BUTTON_NUM];
int bizData_GetPtzAdvancedPara(uchar nChn, SGuiPtzAdvancedPara* pGDispatch);

CPageAddCruisePoint::CPageAddCruisePoint(VD_PCRECT pRect,VD_PCSTR psz/* = NULL*/,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/)
:CPageFloat(pRect, pParent)
{
	bModify = FALSE;
    bPresetIsNone = FALSE;
	
	char *szStatic[] = 
	{
		"&CfgPtn.Preset",
		"&CfgPtn.Speed",
		"&CfgPtn.HoldingTime"
	};

	m_pParent = pParent;
	
	CRect rtTmp;

	int nCtrl = sizeof(szStatic)/sizeof(szStatic[0]);

	BIZ_DO_DBG("CPageAddCruisePoint 0,  ctrl count %d\n", nCtrl);
	
	//memset(&rtTmp, 0, sizeof(rtTmp));
	rtTmp.top = nTop;
	
	for(int i=0; i<nCtrl; i++)
	{
		rtTmp.left = nLeft;
		rtTmp.right = rtTmp.left + nStaticWidth;
		if(i==1)
		{
			rtTmp.top += (nStaticHeight+4);
			rtTmp.bottom = rtTmp.top + nStaticHeight;
		}
		else
		{
			rtTmp.bottom = rtTmp.top + nStaticHeight;
		}
	
		pStatic[i] = CreateStatic(rtTmp, this, szStatic[i]);
		pStatic[i]->SetTextAlign(VD_TA_LEFT);

		rtTmp.top += (nStaticHeight+4);
	}
	
	BIZ_DO_DBG("CPageAddCruisePoint 1\n");
	
	rtTmp.top = nTop;
	
	int nDSize = 8; 

	//memset(&rtTmp, 0, sizeof(rtTmp));
	for(int i=0; i<nCtrl; i++)
	{
		rtTmp.left = nLeft + nStaticWidth + 10;
		rtTmp.right = rtTmp.left + nComboxWidth +10;
		if(i==1)
		{
			rtTmp.top += (nStaticHeight+3);
			rtTmp.bottom = rtTmp.top + nStaticHeight+1;
		}
		else
		{
			rtTmp.bottom = rtTmp.top + nStaticHeight+1;
		}
		
		BIZ_DO_DBG("CPageAddCruisePoint pCombox left %d top %d right %d bottom %d\n",
			rtTmp.left, rtTmp.top, rtTmp.right, rtTmp.bottom
		);
		
		if(i==0)
		{
			pCombox[i] = CreateComboBox(rtTmp,this,NULL,NULL, (CTRLPROC)&CPageAddCruisePoint::OnClickPresetCombox);
            /*
			char szTmp[4] = {0};
			for(int j=0; j<128; j++)
			{
				sprintf(szTmp, "%d", j+1);
				pCombox[i]->AddString(szTmp);
			}
			*/

			//pCombox[i]->SetDropDownSize(&nDSize);
		}
		else
		{
			pCombox[i] = CreateComboBox(rtTmp,this,NULL,NULL);
			
			//nDSize = 3;
			
			//pCombox[i]->SetDropDownSize(&nDSize);
		}

		rtTmp.top += (nStaticHeight+4);
	}

	char *szRate[] = {
		"1","2","3","4","5","6","7","8","9", "10"
	};

	for(int i=0; i<sizeof(szRate)/sizeof(szRate[0]); i++)
	{
		pCombox[1]->AddString(szRate[i]);
	}

	char tmp[4] = {0};
	for(int i=0; i<255;i++)
	{
		sprintf(tmp, "%d", i+1);
		pCombox[2]->AddString(tmp);
	}

	for(int i=0; i<nCtrl; i++)
	{
        if (0 == i)
        {
            pCombox[i]->SetCurSel(0);
        }
        else
        {
		    pCombox[i]->SetCurSel(4);
        }
	}
	
	BIZ_DO_DBG("CPageAddCruisePoint 2\n");
	
	rtTmp.left = rtTmp.right - (2*nButtonWidth);
	rtTmp.top = rtTmp.bottom + 4;
	
	nCtrl = sizeof(pButton)/sizeof(pButton[0]);
	for(int i=0; i<nCtrl; i++)
	{
		pBmpButtonNormal[i] = VD_LoadBitmap(chnShortcutBmpName[i][0]);
		pBmpButtonSelect[i] = VD_LoadBitmap(chnShortcutBmpName[i][1]);
		
		rtTmp.right = rtTmp.left+pBmpButtonNormal[i]->width;
		rtTmp.bottom = rtTmp.top+pBmpButtonNormal[i]->height;
		
		pButton[i] = CreateButton(rtTmp, this, NULL, (CTRLPROC)&CPageAddCruisePoint::OnClickButtion, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
		
	 	rtTmp.left = rtTmp.right + 2;
		//rtTmp.right = rtTmp.left+nButtonWidth;
		
		BIZ_DO_DBG("CPageAddCruisePoint pButton left %d top %d right %d bottom %d\n",
			rtTmp.left, rtTmp.top, rtTmp.right, rtTmp.bottom);
	}
	
	BIZ_DO_DBG("CPageAddCruisePoint 3\n");
	
	//CEdit* pEdit;
	
	rtTmp.left = nLeft + nStaticWidth + 10;
	rtTmp.right = rtTmp.left + nComboxWidth + 10;
	rtTmp.top = nTop + nComboxHeight + 3;
	rtTmp.bottom = rtTmp.top + nComboxHeight + 2;
	
	int vLen = (rtTmp.right-rtTmp.left-4)/TEXT_WIDTH*2;
	pEdit = CreateEdit( &rtTmp, this, vLen+4, 0, NULL );
    pEdit->Enable(FALSE);
    pEdit->Show(FALSE, FALSE);
	
	BIZ_DO_DBG("CPageAddCruisePoint 4\n");
}

void CPageAddCruisePoint::OnClickButtion()
{
	int i;
	BOOL bFind = FALSE;
	
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < sizeof(pButton)/sizeof(pButton[0]); i++)
	{
		if (pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}

	switch(i)
	{
		case 0:  //OK
		{
			SGuiTourPoint sPIns;
			
			BIZ_DO_DBG("OnClickButtion 0\n");
            if (-1 == pCombox[0]->GetCurSel() || bPresetIsNone)
            {
                return;
            }
			
			sPIns.nPresetPos = strtol(pCombox[0]->GetString(pCombox[0]->GetCurSel()), 0, 10);
			BIZ_DO_DBG("OnClickButtion 1\n");
			sPIns.nSpeed = strtol(pCombox[1]->GetString(pCombox[1]->GetCurSel()), 0, 10);
			BIZ_DO_DBG("OnClickButtion 2\n");
			sPIns.nDwellTime = strtol(pCombox[2]->GetString(pCombox[2]->GetCurSel()), 0, 10);
			BIZ_DO_DBG("OnClickButtion 3\n");

			if(bModify)
			{
                ((CPageCruisePoint*)(this->m_pParent))->ModCruisePoint(&sPIns);								
			} 
			else
			{
				((CPageCruisePoint*)(this->m_pParent))->AddCruisePoint(&sPIns);
			}

            bModify = FALSE;

			BIZ_DO_DBG("OnClickButtion 4\n");
            this->Close();
            ((CPageCruisePoint*)(this->m_pParent))->Redraw();
		} break;
		case 1: //Exit
		{
            bPresetIsNone = FALSE;
            bModify = FALSE;
			this->Close();
			//this->m_pParent->Open();
		} break;
		default:
			break;
	}
}

void CPageAddCruisePoint::OnClickPresetCombox()
{
	;
}

CPageAddCruisePoint::~CPageAddCruisePoint()
{
    bPresetIsNone = FALSE;
}

VD_BOOL CPageAddCruisePoint::MsgProc(uint msg, uint wpa, uint lpa)
{
    switch (msg)
    {
        case XM_RBUTTONDOWN:
        case XM_RBUTTONDBLCLK:
        {            
            bPresetIsNone = FALSE;
            this->Close();
        }
            break;
        default:
		    break;
    }
    
    return CPage::MsgProc(msg, wpa, lpa);
}

VD_BOOL CPageAddCruisePoint::UpdateData( UDM mode )
{
	switch( mode )
	{
		case UDM_OPEN:
		{
            char s[10] = {0};
            bizData_GetPtzAdvancedPara(nCurCh, &psGuiPtzPara);
            pCombox[0]->RemoveAll();

            for (int i = 0; i<TOURPIONT_MAX; i++)
            {
                if (psGuiPtzPara.nIsPresetSet[i])
                {
                    sprintf(s, "%d", i+1);
                    pCombox[0]->AddString(s);
                }
            }

            for (int i = 0; i<pCombox[0]->GetCount(); ++i) 
            {
                if (nRecPreset == (int)strtol(pCombox[0]->GetString(i), 0, 10))
                {
                    pCombox[0]->SetCurSel(i);
                    break;
                }
                else 
                {
                    pCombox[0]->SetCurSel(0);
                }
            }

            if (0 == pCombox[0]->GetCount()) 
            {
                bPresetIsNone = TRUE;
                pCombox[0]->AddString("&CfgPtn.NONE");
                pCombox[0]->SetCurSel(0);
            }
            
			pEdit->SetText(GetParsedString("&CfgPtn.Preset"));
            
            #if 0
            pButton[0]->Enable(TRUE);
            if (0 == pCombox[0]->GetCount())
            {
                pButton[0]->Enable(FALSE);
            }
            #endif
		} break;
        case UDM_CLOSED:
        {
            bPresetIsNone = FALSE;
            bModify = FALSE;
        } break;
        default:
            break;
	}

	return TRUE;
}

void CPageAddCruisePoint::onSetCh(int nChn)
{
    nCurCh = nChn;
}

void CPageAddCruisePoint::SetCurCruisePoint(char* szPreset, char* szRate, char* szTime)
{
	bModify = TRUE;

	char* pTmp[3] = {
		szPreset, szRate, szTime
	};

    nRecPreset = atoi(szPreset);

	for(int j=0; j<3; j++)
	{
		for(int i=0; i<pCombox[j]->GetCount(); i++)
		{
			if(strcmp(pTmp[j], pCombox[j]->GetString(i))==0)
			{
				pCombox[j]->SetCurSel(i);
			}
		}
	}
}
	
