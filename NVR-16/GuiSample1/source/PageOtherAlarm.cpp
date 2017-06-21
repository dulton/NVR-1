#include "GUI/Pages/PageOtherAlarm.h"
#include "GUI/Pages/BizData.h"
#include "biz.h"

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static VD_BITMAP * pBmp_infoicon;

CPageOtherAlarm::CPageOtherAlarm( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPageFrame(pRect, psz, icon, pParent)
{
	SetMargin(0,0,0,0);

	//pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/button_normal.bmp");
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	//pBmp_button_push = VD_LoadBitmap(DATA_DIR"/button_push.bmp");
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	//pBmp_button_select = VD_LoadBitmap(DATA_DIR"/button_select.bmp");
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	const int nBtdefaultWidth = 85;
	const int nBtHeight = 25;

	int i=0;
	CRect rtTmp;

	CRect rtTab;
	rtTab.left = 17;
	rtTab.right = m_Rect.Width()-17;
	rtTab.top = 65;
	rtTab.bottom = rtTab.top + CTRL_HEIGHT*10;
	pTab = CreateTableBox(rtTab, this, 2, 10);

	char* szLeft[6] = {
		"&CfgPtn.AlarmType",
		"",
		"&CfgPtn.Buzzer",
		"&CfgPtn.ToAlarmOut",
		"&CfgPtn.Email",
		"&CfgPtn.DiskShortageAlarm",
	};
	for (i = 0; i<6; i++)
	{
		pTab->GetTableRect(0,i, &rtTmp);
		rtTmp.left += rtTab.left;
		rtTmp.right += rtTab.left;
		rtTmp.top += rtTab.top;
		rtTmp.bottom += rtTab.top;
		CStatic* pText = new CStatic(rtTmp, this, szLeft[i]);
	}

	
	pTab->GetTableRect(1,0, &rtTmp);
	rtTmp.left += rtTab.left+5;
	rtTmp.right = rtTmp.left + 160;
	rtTmp.top += rtTab.top;
	rtTmp.bottom += rtTab.top;
	pComboAlarmType = CreateComboBox(rtTmp, this, "Disk Full");
	GetScrStrInitComboxSelDo(
		(void*)pComboAlarmType,
		GSR_CONFIG_ALARM_OTHER_ALARMTYPE, 
		EM_GSR_COMBLIST, 
		0
	);

	pTab->GetTableRect(1,2, &rtTmp);
	rtTmp.left += rtTab.left+5;
	rtTmp.right = rtTmp.left + 24;
	rtTmp.top += rtTab.top +2;
	rtTmp.bottom = rtTmp.top + 24;
	pChkSoundAlarm = CreateCheckBox(rtTmp, this);
	GetScrStrSetCurSelDo(
		(void*)pChkSoundAlarm,
		GSR_CONFIG_ALARM_OTHER_SOUNDALARM, 
		EM_GSR_CTRL_CHK, 
		0
	);

	pTab->GetTableRect(1,3, &rtTmp);
	rtTmp.left += rtTab.left+5;
	rtTmp.right = rtTmp.left + 24;
	rtTmp.top += rtTab.top +2;
	rtTmp.bottom = rtTmp.top + 24;
	pChkMail = CreateCheckBox(rtTmp, this);
	GetScrStrSetCurSelDo(
		(void*)pChkMail,
		GSR_CONFIG_ALARM_OTHER_MAIL, 
		EM_GSR_CTRL_CHK, 
		0
	);

	pTab->GetTableRect(1,4, &rtTmp);
	rtTmp.left += rtTab.left + 5;
	rtTmp.top += rtTab.top + 2;
	rtTmp.right = rtTmp.left + 24;
	rtTmp.bottom = rtTmp.top + 24;
	for (i = 0; i<sizeof(pChkTriggerAlarm)/sizeof(pChkTriggerAlarm[0]); i++)
	{
		pChkTriggerAlarm[i] = CreateCheckBox(rtTmp, this);
		GetScrStrSetCurSelDo(
			(void*)pChkTriggerAlarm[i],
			GSR_CONFIG_ALARM_OTHER_TRIGGERALARM, 
			EM_GSR_CTRL_CHK, 
			i
		);

		char tmp[5] = {0};
		sprintf(tmp, "%d", i+1);
		CStatic* pText = CreateStatic(CRect(rtTmp.right+2, rtTmp.top, rtTmp.right+30, rtTmp.bottom), 
			this, tmp);
		pText->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);

		rtTmp.left = rtTmp.right + 30;
		rtTmp.right = rtTmp.left + 24;
	}

	pTab->GetTableRect(1,5, &rtTmp);
	rtTmp.left += rtTab.left+5;
	rtTmp.right = rtTmp.left + 160;
	rtTmp.top += rtTab.top;
	rtTmp.bottom += rtTab.top;
	pComboDiskAlarm = CreateComboBox(rtTmp, this, "128 MB");
	GetScrStrInitComboxSelDo(
		(void*)pComboDiskAlarm,
		GSR_CONFIG_ALARM_OTHER_DISKALARM, 
		EM_GSR_COMBLIST, 
		0
	);


	//buttons
	rtTmp.left = m_Rect.Width()-17-nBtdefaultWidth*3-20*2;
	rtTmp.right = rtTmp.left + nBtdefaultWidth;
	rtTmp.top = m_Rect.Height()-40;
	rtTmp.bottom = rtTmp.top + nBtHeight;

	pInfoBar = CreateStatic(CRect(40, 
								rtTmp.top,
								317,
								rtTmp.top+22), 
							this, 
							"");
	//pInfoBar->SetBkColor(VD_RGB(8,66,189));

	pBmp_infoicon = VD_LoadBitmap(DATA_DIR"/temp/icon_info.bmp");
	pIconInfo = CreateStatic(CRect(17, 
								rtTmp.top+1,
								37,
								rtTmp.top+21), 
							this, 
							"");
	pIconInfo->SetBitmap(pBmp_infoicon,NULL);
	pIconInfo->Show(FALSE);
	
	char* szApp[3] = {
		"&CfgPtn.Default",
		"&CfgPtn.apply",
		"&CfgPtn.Exit"
	};

	for (i=0; i<3; i++)
	{
		pButton[i] = CreateButton(rtTmp, this, szApp[i], (CTRLPROC)&CPageOtherAlarm::OnClkApp, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);

		rtTmp.left = rtTmp.right + 20;
		rtTmp.right = rtTmp.left + nBtdefaultWidth;
	}


}

CPageOtherAlarm::~CPageOtherAlarm()
{

}

void CPageOtherAlarm::OnClkApp()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < 3; i++)
	{
		if (pFocusButton == pButton[i])
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
				printf("default \n");
			}break;
			case 1:
			{
				printf("apply\n");
			}break;
			case 2:
			{
				printf("exit\n");
				this->Close();
			}break;
			default:
			break;

		}
	}
}

VD_BOOL CPageOtherAlarm::UpdateData( UDM mode )
{
	if (UDM_OPEN == mode)
	{
		GetScrStrSetCurSelDo(
			(void*)pComboAlarmType,
			GSR_CONFIG_ALARM_OTHER_ALARMTYPE, 
			EM_GSR_CTRL_COMB, 
			0
		);
		
		GetScrStrSetCurSelDo(
			(void*)pChkSoundAlarm,
			GSR_CONFIG_ALARM_OTHER_SOUNDALARM, 
			EM_GSR_CTRL_CHK, 
			0
		);

		
		for ( int i = 0; i<sizeof(pChkTriggerAlarm)/sizeof(pChkTriggerAlarm[0]); i++)
		{
			GetScrStrSetCurSelDo(
				(void*)pChkTriggerAlarm[i],
				GSR_CONFIG_ALARM_OTHER_TRIGGERALARM, 
				EM_GSR_CTRL_CHK, 
				i
			);
		}
		
		GetScrStrSetCurSelDo(
			(void*)pChkMail,
			GSR_CONFIG_ALARM_OTHER_MAIL, 
			EM_GSR_CTRL_CHK, 
			0
		);
		GetScrStrInitComboxSelDo(
			(void*)pComboDiskAlarm,
			GSR_CONFIG_ALARM_OTHER_DISKALARM, 
			EM_GSR_COMBLIST, 
			0
		);
	}
	return TRUE;
}

void CPageOtherAlarm::SetInfo(char* szInfo)
{
	pIconInfo->Show(TRUE);
	
	pInfoBar->SetText(szInfo);

}

void CPageOtherAlarm::ClearInfo()
{
	pIconInfo->Show(FALSE);
	
	pInfoBar->SetText("");

}

VD_BOOL CPageOtherAlarm::MsgProc( uint msg, uint wpa, uint lpa )
{

	int px,py;
	switch(msg)
	{
	case XM_MOUSEMOVE:
		{
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);
			static CItem* last = NULL;
			CItem* temp = GetItemAt(px, py);

			if(temp != last)
			{
				if(temp == NULL)
				{
					ClearInfo();
				}

				last = temp;

			}
		}
		break;
	default:
		//return 0;
		break;
	}

	return CPage::MsgProc(msg, wpa, lpa);

}


