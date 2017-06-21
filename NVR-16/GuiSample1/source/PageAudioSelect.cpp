#include "GUI/Pages/PageAudioSelect.h"
#include "GUI/Pages/PageStartFrameWork.h"
#include "GUI/Pages/BizData.h"

#include "biz.h"

enum{//与下面的数组一一对应
	AUDIO_BUTTON_OK,
	AUDIO_BUTTON_CANCLE,
	AUDIO_BUTTON_NUM,
};

char* audioShortcutBmpName[AUDIO_BUTTON_NUM][2] = {    
	{DATA_DIR"/temp/ok.bmp",	DATA_DIR"/temp/ok_f.bmp"},
	{DATA_DIR"/temp/exit2.bmp",	DATA_DIR"/temp/exit2_f.bmp"}	
};

static VD_BITMAP* pBmpButtonNormal[AUDIO_BUTTON_NUM];
static VD_BITMAP* pBmpButtonSelect[AUDIO_BUTTON_NUM];


//static int GetMaxChnNum() { return GetVideoMainNum(); }
static int GetMaxAudioNum() { return GetAudioNum(); }

CPageAudioSelect::CPageAudioSelect( VD_PCRECT pRect,VD_PCSTR psz /*= NULL*/,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/, uint vstyle /*= 0*/ )
:CPageFloat(pRect, pParent)
{
	nChMax = GetMaxAudioNum();
	
	m_bAudioSelectPage = TRUE;
	
	CRect rtTmp;
	rtTmp.left = 3;
	rtTmp.right = rtTmp.left + 70;
	rtTmp.top = 3;
	rtTmp.bottom = rtTmp.top +20;

	pStatic[0] = CreateStatic(rtTmp, this, "&CfgPtn.Volume");
	pStatic[0]->SetTextAlign(VD_TA_LEFT);

	rtTmp.left = rtTmp.right + 10;
	rtTmp.right = rtTmp.left+100; 

	pSlider = CreateSliderCtrl(CRect(rtTmp.left,rtTmp.top+2,rtTmp.right,rtTmp.bottom+2 ),this,0, 100, (CTRLPROC)&CPageAudioSelect::OnSliderChange, sliderNoNum);

	rtTmp.left = rtTmp.right + 10;
	rtTmp.right = rtTmp.left+20; 

	pCheckBox[0] = CreateCheckBox(CRect(rtTmp.left,rtTmp.top+2,rtTmp.right,rtTmp.bottom+2 ),this,styleEditable,(CTRLPROC)&CPageAudioSelect::OnCheckBox);
	pCheckBox[0]->SetValue(FALSE);
	nMute = 0;

	rtTmp.left = rtTmp.right + 10;
	rtTmp.right = rtTmp.left+50; 

	pStatic[1] = CreateStatic(rtTmp, this, "&CfgPtn.Mute");
	pStatic[1]->SetTextAlign(VD_TA_LEFT);

	rtTmp.left = 3;
	rtTmp.right = rtTmp.left + 70;
	rtTmp.top = rtTmp.bottom +3;
	rtTmp.bottom = rtTmp.top +20;

	pStatic[2] = CreateStatic(rtTmp, this, "&CfgPtn.AudioChn");
	pStatic[2]->SetTextAlign(VD_TA_LEFT);

	
	rtTmp.left = rtTmp.right + 10;
	rtTmp.right = rtTmp.left+100; 

	pComboBox = CreateComboBox(CRect(rtTmp.left,rtTmp.top+2,rtTmp.right,rtTmp.bottom+2),
				this, NULL, NULL, NULL, 0);

	#if 0
	pComboBox->AddString("1");
	pComboBox->AddString("2");
	pComboBox->AddString("3");
	pComboBox->AddString("4");
	#else
	for(int i=0; i<nChMax; i++)
	{
		char szTmp[4]={0};
		sprintf(szTmp, "%d", i+1);
		pComboBox->AddString(szTmp);
	}
	#endif
	pComboBox->SetCurSel(0);

	rtTmp.left = rtTmp.right + 10 ;
	rtTmp.right = rtTmp.left+20; 

	pCheckBox[1] = CreateCheckBox(CRect(rtTmp.left,rtTmp.top+2,rtTmp.right,rtTmp.bottom+2 ),this,styleEditable,(CTRLPROC)&CPageAudioSelect::OnCheckBox);
	pCheckBox[1]->SetValue(TRUE);

	rtTmp.left = rtTmp.right + 10 ;
	rtTmp.right = rtTmp.left+50; 

	pStatic[3] = CreateStatic(rtTmp, this, "&CfgPtn.Auto");
	pStatic[3]->SetTextAlign(VD_TA_LEFT);


	rtTmp.left =  3;
	rtTmp.right = rtTmp.left + 42;
	rtTmp.top = m_Rect.Height()*2/3;
	rtTmp.bottom = rtTmp.top + 20;

	pStatic[4] = CreateStatic(rtTmp, this, "&CfgPtn.VOL:");
	pStatic[4]->SetTextAlign(VD_TA_LEFT);

	rtTmp.left =  rtTmp.right+5;
	rtTmp.right = rtTmp.left + 40;
	rtTmp.top = m_Rect.Height()*2/3;
	rtTmp.bottom = rtTmp.top + 20;

	pStatic[5] = CreateStatic(rtTmp, this, "0");
	pStatic[5]->SetTextAlign(VD_TA_LEFT);

	rtTmp.left =  m_Rect.Width()/2 +105;
	rtTmp.right = rtTmp.left + 30;
	rtTmp.top = m_Rect.Height()*2/3;
	rtTmp.bottom = rtTmp.top + 25;
	pBmpButtonNormal[1] = VD_LoadBitmap(audioShortcutBmpName[1][0]);
	pBmpButtonSelect[1] = VD_LoadBitmap(audioShortcutBmpName[1][1]);
	pButton[1] = CreateButton(rtTmp, this, NULL, (CTRLPROC)&CPageAudioSelect::OnClkButton, NULL, buttonNormalBmp);
	pButton[1]->SetBitmap(pBmpButtonNormal[1], pBmpButtonSelect[1], pBmpButtonSelect[1]);
		
	rtTmp.right = rtTmp.left -10;
	rtTmp.left =  rtTmp.right -30;
	pBmpButtonNormal[0] = VD_LoadBitmap(audioShortcutBmpName[0][0]);
	pBmpButtonSelect[0] = VD_LoadBitmap(audioShortcutBmpName[0][1]);
	pButton[0] = CreateButton(rtTmp, this, NULL, (CTRLPROC)&CPageAudioSelect::OnClkButton, NULL, buttonNormalBmp);
	pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0]);

	m_nIsPreview = 0;
}

void CPageAudioSelect::OnClkButton()
{
	//printf("CPageAudioSelect OnClkButton##########################\n");
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton* pFocusCheck = (CButton*)GetFocusItem();
	for(i = 0; i < 2; i++)
	{
		if(pFocusCheck == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(!bFind)
	{
		return;
	}
	
	if(i == 1)
	{
		this->Close();
		return;
	}
	
	u8 nEnable = (u8)pCheckBox[0]->GetValue();
	
	//静音
	//pSlider->SetPos(0);
	m_pParent->SetMute(nEnable);
	ControlMute(nEnable);
	
	if((!nMute) && nEnable)
	{
		BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_MUTE);
	}
	nMute = nEnable;
	
	if(TRUE == pCheckBox[1]->GetValue())
	{
		//Auto
		nAudioChn = 0xff;
		ControlAudioOutChn(0xff);
	}
	else
	{
		int curIndex = pComboBox->GetCurSel();
		ControlAudioOutChn(curIndex);
		nAudioChn = curIndex;
	}
	
	int vol = pSlider->GetPos();
	//ControlVolume(vol);
	/*if(0 == vol)
	{
		printf("********1\n");
		ControlMute(1);
	}
	else if((nVolume == 0) && (nMute == 0))
	{
		printf("********2\n");
		ControlMute(0);
	}*/
	nVolume = vol;
	SetAudioOutInfo(nAudioChn,nVolume,nMute);
	
	this->Close();
	
	if(nVolume != vol)
	{
		BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_CHANGE_VOICE);
	}
}

void CPageAudioSelect::OnCheckBox()
{
	printf("OnCheckBox\n");
	int i = 0;
	BOOL bFind = FALSE;
	CCheckBox* pFocusCheck = (CCheckBox*)GetFocusItem();
	for (i = 0; i < 2; i++)
	{
		if (pFocusCheck == pCheckBox[i])
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
				printf("Check Mute\n");				
				if(TRUE == pCheckBox[0]->GetValue())
				{
					//静音
					//pSlider->SetPos(0);
					pSlider->Enable(0);
					//((CPageStartFrameWork*)m_pParent)->SetMute(TRUE);
					pStatic[5]->SetText("0");
				}
				else
				{
					pSlider->Enable(1);
					OnSliderChange();
				}				
			}break;
			case 1:
			{
				printf("Check Auto\n");
				if(TRUE == pCheckBox[1]->GetValue())
				{
					pComboBox->Enable(0);
				}
				else
				{
					pComboBox->Enable(1);
				}	
			}break;
			default:
			break;
		}		
	}
}

void CPageAudioSelect::OnSliderChange()
{
	//printf("OnSliderChange\n");
	char strVol[4] = {0};
	int vol = pSlider->GetPos();
	sprintf(strVol,"%d",vol);
	
	pStatic[5]->SetText(strVol);
	
	/*
	if(!vol)
	{
		//静音
		pCheckBox[0]->SetValue(TRUE);
		((CPageStartFrameWork*)m_pParent)->SetMute(TRUE);
		
	}
	else
	{
		pCheckBox[0]->SetValue(FALSE);
		((CPageStartFrameWork*)m_pParent)->SetMute(FALSE);
	}*/
}

CPageAudioSelect::~CPageAudioSelect()
{
	
}

void CPageAudioSelect::SetPreviewAudio()
{
	m_nIsPreview = 1;
}

VD_BOOL CPageAudioSelect::SetAudioInfo(u8 chn, u8 vol, u8 mute)
{
	if(chn == 0xff)
	{
		pCheckBox[1]->SetValue(1);
		pComboBox->Enable(0);
	}
	else
	{
		pCheckBox[1]->SetValue(0);
		pComboBox->Enable(1);
		pComboBox->SetCurSel((int)chn);
	}
	
	pSlider->SetPos((int)vol);
	OnSliderChange();
	
	if(mute)
	{
		pCheckBox[0]->SetValue(1);
		pSlider->Enable(0);
		pStatic[5]->SetText("0");
	}
	else
	{
		pCheckBox[0]->SetValue(0);
		pSlider->Enable(1);
		OnSliderChange();
	}
	
	nAudioChn = chn;
	nVolume = vol;
	nMute = mute;
}

VD_BOOL CPageAudioSelect::MsgProc( uint msg, uint wpa, uint lpa )
{
/*
	s32 px, py;

	switch(msg)
	{
	case XM_LBUTTONDOWN:
	case XM_LBUTTONDBLCLK:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(PtInRect(m_Rect, px, py))
		{
			break;
		}
		msg = XM_RBUTTONDOWN;
	case XM_RBUTTONDOWN:
	case XM_RBUTTONDBLCLK:

		u8 nEnable = (u8)pCheckBox[0]->GetValue();

		//静音
		//pSlider->SetPos(0);
		m_pParent->SetMute(nEnable);
		ControlMute(nEnable);

		if(TRUE == pCheckBox[1]->GetValue())
		{
			//Auto
			ControlAudioOutChn(0xff);
		}
		else
		{
			int curIndex = pComboBox->GetCurSel();
			ControlAudioOutChn(curIndex);
		}
		
		int vol = pSlider->GetPos();
		ControlVolume(vol);
		
		break;
	default:
		//return 0;
		break;
	}
*/
	//return 	CPage::MsgProc(msg, wpa, lpa);
	return CPage::MsgProc(msg, wpa, lpa);
}

