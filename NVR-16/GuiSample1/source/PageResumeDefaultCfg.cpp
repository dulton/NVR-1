#include "GUI/Pages/PageResumeDefaultCfg.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/PageMessageBox.h"

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;


static EMBIZCONFIGPARATYPE DefaultCfgArr[] = 
{
	EM_BIZCONFIG_PARA_SYSTEM ,
	EM_BIZCONFIG_PARA_PREVIEW ,
	EM_BIZCONFIG_PARA_IMAGE ,
	EM_BIZCONFIG_PARA_MAIN_OSD ,
	EM_BIZCONFIG_PARA_STR_OSD ,
	EM_BIZCONFIG_PARA_RECT_OSD ,
	EM_BIZCONFIG_PARA_SENSOR ,
	EM_BIZCONFIG_PARA_VMOTION ,
	EM_BIZCONFIG_PARA_VLOST ,
	EM_BIZCONFIG_PARA_VBLIND ,
	EM_BIZCONFIG_PARA_ALARMOUT ,
	//EM_BIZCONFIG_PARA_BUZZ ,
	//EM_BIZCONFIG_PARA_SENSOR_SCH ,
	//EM_BIZCONFIG_PARA_VMOTION_SCH ,
	//EM_BIZCONFIG_PARA_VLOST_SCH ,
	//EM_BIZCONFIG_PARA_VBLIND_SCH ,
	//EM_BIZCONFIG_PARA_ALARMOUT_SCH ,
	//EM_BIZCONFIG_PARA_BUZZ_SCH ,
	EM_BIZCONFIG_PARA_ENC_MAIN ,
	EM_BIZCONFIG_PARA_ENC_SUB ,
	EM_BIZCONFIG_PARA_RECORD ,
	EM_BIZCONFIG_PARA_TIMER_REC_SCH ,
	EM_BIZCONFIG_PARA_SENSOR_REC_SCH ,
	EM_BIZCONFIG_PARA_VMOTION_REC_SCH ,
	//EM_BIZCONFIG_PARA_VLOST_REC_SCH ,
	//EM_BIZCONFIG_PARA_VBLIND_REC_SCH ,
	EM_BIZCONFIG_PARA_USER ,
	EM_BIZCONFIG_PARA_PTZ ,
	EM_BIZCONFIG_PARA_NETWORK ,
	EM_BIZCONFIG_PARA_DVR_PROPERTY 
};

CPageResumeDefaultCfg::CPageResumeDefaultCfg(VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon,CPage * pParent):CPageFrame(pRect, psz, icon, pParent)
{
	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);

	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	char* szButton[2] = 
	{
		"&CfgPtn.OK",
		"&CfgPtn.Exit",
	};
	
	CRect rtTmp(600, m_Rect.Height()-110, m_Rect.Width()-20, m_Rect.Height()-110+pBmp_button_normal->height);
	rtTmp.left -= 80*2 + 40;
	rtTmp.right = rtTmp.left+80;
	
	int i = 0;
	for(i=0; i<2; i++)
	{
		pButton[i] = CreateButton(rtTmp, this, szButton[i], (CTRLPROC)&CPageResumeDefaultCfg::OnClkBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
		rtTmp.left = rtTmp.right + 40;
		rtTmp.right = rtTmp.left + 80;
	}
	
	CRect rtTmp1(m_Rect.left+27, m_Rect.top+10, 
		m_Rect.Width()-27,m_Rect.top+50+240);

	pTable = CreateTableBox(&rtTmp1,this, 1,1,0);

	printf("line %d ... \n", __LINE__);
	
	char* szStatic[] = 
	{
		"CfgPtn.CfgSYSTEM",
		"CfgPtn.CfgPREVIEW",
		"CfgPtn.CfgIMAGE",
		"CfgPtn.CfgMAINOSD",
		"CfgPtn.CfgSTROSD",
		"CfgPtn.CfgRECTOSD",
		"CfgPtn.CfgSENSOR",
		"CfgPtn.CfgVMOTION",
		"CfgPtn.CfgVLOST",
		"CfgPtn.CfgVBLIND",
		"CfgPtn.CfgALARMOUT",
		//"CfgPtn.CfgBUZZ",
		//"CfgPtn.CfgSENSORSCH",
		//"CfgPtn.CfgVMOTIONSCH",
		//"CfgPtn.CfgVLOSTSCH",
		//"CfgPtn.CfgVBLINDSCH",
		//"CfgPtn.CfgALARMOUTSCH",
		//"CfgPtn.CfgBUZZSCH",
		//"CfgPtn.CfgVOIMAGE",
		"CfgPtn.CfgENCMAIN",
		"CfgPtn.CfgENCSUB",
		"CfgPtn.CfgRECORD",
		"CfgPtn.CfgTIMERRECSCH",
		"CfgPtn.CfgSENSORRECSCH",
		"CfgPtn.CfgVMOTIONRECSCH",
		//"CfgPtn.CfgVLOSTRECSCH",
		//"CfgPtn.CfgVBLINDRECSCH",
		"CfgPtn.CfgUSER",
		"CfgPtn.CfgPTZ",
		"CfgPtn.CfgNETWORK",
		"CfgPtn.CfgDVRPROPERTY"
	};
	
	m_nDefaultItemCnr = sizeof(szStatic)/sizeof(szStatic[0]);
	
	rtTmp1.left = 30;
	rtTmp1.top = 20;
	rtTmp1.right = rtTmp1.left+20;
	rtTmp1.bottom = rtTmp1.top+20;
	int j = 0;

	int nCol = 3;
	int nRow = (m_nDefaultItemCnr+nCol-1)/nCol;
	for(i=0; i<nRow; i++)
	{
		for(j=0; j<nCol; j++)
		{
			if(nCol*i+j>=m_nDefaultItemCnr) break;
			
			pCheckBox[i*nCol+j] = CreateCheckBox(rtTmp1, this);
			
			rtTmp1.left = rtTmp1.right+5;
			rtTmp1.right = rtTmp1.left+140;

			pStatic[i*nCol+j] = CreateStatic(rtTmp1, this, szStatic[i*nCol+j]);

			rtTmp1.left = rtTmp1.right+30;
			rtTmp1.right = rtTmp1.left+20;
		}

		if(j<nCol) break;
		
		rtTmp1.left = 30;
		rtTmp1.top = rtTmp1.bottom + 15;
		rtTmp1.right = rtTmp1.left+20;
		rtTmp1.bottom = rtTmp1.top+20;	
	}

    pInfoBar = CreateStatic(CRect(m_Rect.Width()/2 - 200, 
								rtTmp.bottom+8,
								m_Rect.Width()/2 + 180,
								rtTmp.bottom+28), 
								this, 
								"");
    pInfoBar->SetTextAlign(VD_TA_CENTER);
}

void CPageResumeDefaultCfg::OnClkBtn()
{
	int i, j;
	
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (j = 0; j < 2; j++)
	{
		if (pFocusButton == pButton[j])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
		switch(j)
		{
			case 0: // ok
			{
				UDM ret= MessageBox("&CfgPtn.ResetallsettingSystemwillreboot", "&CfgPtn.WARNING", MB_OKCANCEL|MB_ICONWARNING|MB_DEFBUTTON2);
				if(GetRunStatue() == UDM_OK)
				{
					s32 rtn = 0;
					int nRst = 0;

			        pInfoBar->SetText(GetParsedString("&CfgPtn.SysResetNow"));
					SetSystemLockStatus(1);//cw_lock
					for(i=0; i<m_nDefaultItemCnr; i++)
					{
						if(pCheckBox[i]->GetValue())
						{
							rtn |= BizConfigResumeDefault(DefaultCfgArr[i], 0); // 从默认配置文件中恢复配置	

							printf("resume default config %d \n", DefaultCfgArr[i]);
							nRst++;
						}
					}

					if(nRst>0)
					{
						if(rtn == 0)
						{
							pInfoBar->SetText(GetParsedString("&CfgPtn.SysRestartNow"));
							BizSysComplexExit(EM_BIZSYSEXIT_RESTART);
						}
						else
						{
							MessageBox("&CfgPtn.ResumeDefaulyParaFail", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
						}
					}
				}
			} break;
			case 1: // cancel
			{
				this->Close();
			} break;
		}
	}
	
	return;
}
	
CPageResumeDefaultCfg::~CPageResumeDefaultCfg()
{}

VD_BOOL CPageResumeDefaultCfg::UpdateData( UDM mode )
{	
	int i;
	if(mode == UDM_OPEN)
	{
		for(i=0; i<m_nDefaultItemCnr; i++)
		{
			pCheckBox[i]->SetValue(1);
		}
	}
	
	return TRUE;
}

