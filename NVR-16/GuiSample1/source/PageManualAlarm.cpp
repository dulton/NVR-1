#include "GUI/Pages/PageManualAlarm.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/BizData.h"


static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

CPageManualAlarm::CPageManualAlarm( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
, curID(0)
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
		m_Rect.Width()-10,m_Rect.top+10+330);

	pTable = CreateTableBox(&rtSubPage,this, 4,11,0);
	pTable->SetColWidth(0,60);
	pTable->SetColWidth(1,270);
	pTable->SetColWidth(2,140);
#if 1
	char* statictext[8] = 
	{
		"&CfgPtn.ID",
		"&CfgPtn.AlarmName",
		"&CfgPtn.IPAddress",
		"&CfgPtn.AlarmHandling",
		"1",
		"&CfgPtn.AlarmOut",
		"127.0.0.1",
		"&CfgPtn.All",
	};
#else
	char* statictext[8] = 
	{
		"ID",
		"Alarm Name",
		"IP",
		"Alarm Process",
		"1",
		"ALARM OUT 1",
		"127.0.0.1",
		"All",
	};

#endif	
	CRect tmpRt;
	int i = 0;
	for(i=0; i<4; i++)
	{
		pTable->GetTableRect(i,0,&tmpRt);
		pStatic[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, statictext[i]);
		pStatic[i]->SetBkColor(VD_RGB(67,77,87));
	}

	for(i=4; i<7; i++)
	{
		pTable->GetTableRect(i-4,1,&tmpRt);
		pStatic[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top+3, rtSubPage.left+tmpRt.right-3,rtSubPage.top+tmpRt.bottom-3), this, statictext[i]);
	}

	#if 0
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
	#endif
	
	pTable->GetTableRect(3,1,&tmpRt);
	pCheckBox[0] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+4, 
								rtSubPage.left+tmpRt.left+22,rtSubPage.top+tmpRt.top+24), 
								this);

	pCheckBox[1] = CreateCheckBox(CRect(17, 10+330+5, 17+20,10+330+5+20), this);

	pStatic[7] = CreateStatic(CRect(17+30, 10+330+5, 17+30+100,10+330+5+20), this, statictext[7]);
#if 1
	char* btntext[3] = 
	{
		"&CfgPtn.Alarm2",
		"&CfgPtn.ClearAlarm",
		"&CfgPtn.Exit"
	};
	
	int btntextLen[3] = 
	{
		TEXT_WIDTH*2+10, //"&CfgPtn.Alarm2",
		TEXT_WIDTH*4, //"&CfgPtn.ClearAlarm",
		TEXT_WIDTH*2, //"&CfgPtn.Exit"
	};
	
#endif

	
	CRect btnRt;
	btnRt.left = tmpRt.right-(TEXT_WIDTH)*8-20;
	btnRt.top = 10 + 330 + 5;
	//int btWidth = strlen(btntext[0])*TEXT_WIDTH/2+20;
	//btnRt.right = btnRt.left + btWidth;
	btnRt.bottom = btnRt.top + pBmp_button_normal->height;
	

	for(i=0; i<3; i++)
	{	
		int btWidth = btntextLen[i];
		btnRt.right = btnRt.left + btWidth;
		pButton[i] = CreateButton(btnRt,this, btntext[i], (CTRLPROC)&CPageManualAlarm::OnClkBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);

		btnRt.left = btnRt.right+3;
	}	

}



CPageManualAlarm::~CPageManualAlarm()
{

}

void CPageManualAlarm::OnClkBtn()
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
		switch(i)
		{
			case 0:
			{
				//alarm
				BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_MANUAL_ALARM);
			}break;
			case 1:
			{	
				//clear
				BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_CLEAR_ALARM);
			}break;
			case 2:
			{
				//exit
				this->Close();
			}break;
			default:
			break;
		}
		
	}
}

VD_PCSTR CPageManualAlarm::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

#if 0 
void CPageBasicConfigFrameWork::InitPage1()
{
	char* statictext[PG_1_STATICNUM] = {
		"Date Format",
		"Time Format",
		"Time Zone",
		"Network Synchronization",
		"NTP Server",
		"",
		"System Date",
		"System Time",
		"",
	};

	CRect rtSubPage(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.Width()-27,m_Rect.top+50+270);

	pTable1 = CreateTableBox(&rtSubPage,this, 2,9,0);
	pTable1->SetColWidth(0,294);
	items[1].push_back(pTable1);

	CRect tmpRt;
	int i = 0;
	for(i=0; i<9; i++)
	{
		pTable1->GetTableRect(0,i,&tmpRt);
		pStatic1[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top+3, rtSubPage.left+tmpRt.right-3,rtSubPage.top+tmpRt.bottom-3), this, statictext[i]);
		items[1].push_back(pStatic1[i]);
	}



	for(i=0; i<3; i++)
	{
		pTable1->GetTableRect(1,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pComboBox1[i] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+3, 
			rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-3), 
			this, NULL, NULL, NULL, NULL);

		switch(i)
		{
			case 0:
			{
				pComboBox1[i]->AddString("MM.dd.yy");
				pComboBox1[i]->AddString("yy.MM.dd");
				pComboBox1[i]->SetCurSel(0);

			}break;
			case 1:
			{
				
				pComboBox1[i]->AddString("24 Hours");
				pComboBox1[i]->AddString("12 Hours");
				pComboBox1[i]->SetCurSel(0);

			}break;
			case 2:
			{
				pComboBox1[i]->AddString("GMT");
				pComboBox1[i]->SetCurSel(0);
			}break;
			default:
			{}break;
		}
	
		items[1].push_back(pComboBox1[i]);
	}



	pTable1->GetTableRect(1,3,&tmpRt);
	pCheckBox1[0] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+4, 
								rtSubPage.left+tmpRt.left+22,rtSubPage.top+tmpRt.top+24), 
								this);
	items[1].push_back(pCheckBox1[0]);


	
	pTable1->GetTableRect(1,4,&tmpRt);
	int vLen = (tmpRt.right-tmpRt.left-4)/TEXT_WIDTH*2;
	pEdit1[0] = CreateEdit(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+3, 
								rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-3), 
								this,vLen,0,(CTRLPROC)&CPageBasicConfigFrameWork::OnEditChange1);
	items[1].push_back(pEdit1[0]);
	
	int btWidth = strlen("Synchronize")*TEXT_WIDTH/2+20;
	pTable1->GetTableRect(1,5,&tmpRt);
	pButton1[0] = CreateButton(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+3, 
								rtSubPage.left+tmpRt.left+btWidth,rtSubPage.top+tmpRt.top+27), 
								this, "Synchronize", (CTRLPROC)&CPageBasicConfigFrameWork::OnSynchronize1, NULL, buttonNormalBmp);
	pButton1[0]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	items[1].push_back(pButton1[0]);
	
	pTable1->GetTableRect(1,6,&tmpRt);
	pDateTime1[0] = CreateDateTimeCtrl(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+3, 
								rtSubPage.left+tmpRt.left+120,rtSubPage.top+tmpRt.top+27),
								this,NULL,DTS_DATE);
	
	//pDateTime1[0]->Show(FALSE);
	items[1].push_back(pDateTime1[0]);
	
	pTable1->GetTableRect(1,7,&tmpRt);
	pDateTime1[1] = CreateDateTimeCtrl(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+3, 
								rtSubPage.left+tmpRt.left+120,rtSubPage.top+tmpRt.top+27),
								this,NULL,DTS_TIME);
	//pDateTime1[1]->Show(FALSE);
	items[1].push_back(pDateTime1[1]);
	
	pTable1->GetTableRect(1,8,&tmpRt);
	pButton1[1] = CreateButton(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+2, 
								rtSubPage.left+tmpRt.left+btWidth,rtSubPage.top+tmpRt.top+26), 
								this, "Save", (CTRLPROC)&CPageBasicConfigFrameWork::OnSave1, NULL, buttonNormalBmp);
	pButton1[1]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	items[1].push_back(pButton1[1]);
	
	ShowSubPage(1,FALSE);
}
#endif

VD_BOOL CPageManualAlarm::UpdateData( UDM mode )
{
	if (UDM_OPEN == mode)
	{
	}
	return TRUE;
}

