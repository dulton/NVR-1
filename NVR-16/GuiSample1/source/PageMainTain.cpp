//#include "GUI/Pages/PageRecordFrameWork.h"
//#include "GUI/Pages/PageSensorAlarmFrameWork.h"
#include "GUI/Pages/PageMainTain.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMessageBox.h"

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

#define TIME_ROW_ITEM 4
#define TIME_CTR 24
#define TIME_STATIC 24

CPageMainTain::CPageMainTain( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
{
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	m_pDevGraphics = CDevGraphics::instance(m_screen);

	char* szTitle[3] = {
		"&CfgPtn.Type",
		"&CfgPtn.Time", 
		"&CfgPtn.Enable",
	};
	
	char* szButton[2] = {
		"&CfgPtn.OK",
		"&CfgPtn.Cancel",
	};

	CRect rt;
	rt.left = 10;
	rt.top = 10;
	rt.right = m_Rect.Width()-22;
	rt.bottom =rt.top + 30;
	
	pTabPage0 = CreateTableBox(rt, this, 3, 1);
	pTabPage0->SetColWidth(0, 80);

	int i = 0;
	CRect rtTemp;
	for (i = 0; i < 2; i++)
	{
		pTabPage0->GetTableRect(i, 0, &rtTemp);

		if (i > 0)
		{
			pCombox[i - 1] = CreateComboBox(CRect(rt.left+rtTemp.left+1, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom - 1), 
                            		this, NULL, NULL, (CTRLPROC)&CPageMainTain::OnComTypeSelect, 0);
			pCombox[i - 1]->SetBkColor(VD_RGB(67,77,87));
		}
		else
		{
			pStatic0[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, szTitle[i]);
			pStatic0[i]->SetTextAlign(VD_TA_CENTER);
		}
	}

	pTabPage0->GetTableRect(2, 0, &rtTemp);
	
	for (i = 1; i < 4; i++)//在同一位置再创建三个下拉选项表
	{
		pCombox[i] = CreateComboBox(CRect(rt.left+rtTemp.left+1, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom - 1), 
        	            		this, NULL, NULL, NULL, 1);
		pCombox[i]->SetBkColor(VD_RGB(67,77,87));
	}

	SValue typeList[10];
	int nTypeReal = 0;
	
	GetMainTainTypeList(typeList, &nTypeReal, 10);//添加类型列表

	for (i = 0; i < nTypeReal; i++)
	{
		pCombox[0]->AddString(typeList[i].strDisplay);
	}

	char s[10] = {0};//添加日期列表
	
	for( int i=0; i<31; i++)
	{
		sprintf(s,"%d",i+1);
		pCombox[1]->AddString(s);
	}
	
	pCombox[1]->SetCurSel(0);
	
	SValue weekList[10];
	int nweekReal = 0;
	
	GetWeekList(weekList, &nweekReal, 10);//添加星期列表

	for (i = 0; i < nweekReal; i++)
	{
		pCombox[2]->AddString(weekList[i].strDisplay);
	}
	
	pCombox[3]->AddString("&CfgPtn.NOOPTION");//设置无选项

	for (i = 0; i < 4; i++)
	{
		pCombox[i]->SetCurSel(0);

		if (i > 0)
		{
			pCombox[i]->Show(FALSE,TRUE);
		}
	}
	
	pCombox[3]->Show(TRUE,TRUE);
	
	rt.top = rt.bottom - 1;
	rt.bottom =rt.top + 160;
	
	pTabPage1 = CreateTableBox(rt, this, 2, 1);
	pTabPage1->SetColWidth(0, 80);

	pTabPage1->GetTableRect(0, 0, &rtTemp);
	pStatic0[1]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom),this, szTitle[1]);
	pStatic0[1]->SetTextAlign(VD_TA_CENTER);
	
	CRect rtTmp;
	pCheckbox = (CCheckBox **)malloc(sizeof(CCheckBox*) * (TIME_CTR + 1));
	pStatic1 = (CStatic **)malloc(sizeof(CStatic *) * (TIME_CTR));

	if(NULL == pCheckbox)
	{
		printf("pCheckBox malloc in pagemaintain failed!!\n");
	}

	if(NULL == pStatic1)
	{
		printf("pStatic1 malloc in pagemaintain failed!!\n");
	}
	
	for(i=0; i<TIME_CTR; i++)
	{
		if (0 == (i % (TIME_ROW_ITEM)))
		{
			rtTmp.left = 4+rt.left+rtTemp.left + 80;
			rtTmp.right = rtTmp.left + CHKBOXW;
			rtTmp.top = 4 + rt.top+rtTemp.top + (i / (TIME_ROW_ITEM)) * (CHKBOXW + GAP);
			rtTmp.bottom = rtTmp.top + (CHKBOXW);
		}

		pCheckbox[i] = CreateCheckBox(rtTmp,this,styleEditable,(CTRLPROC)&CPageMainTain::OnCheckSel);

		rtTmp.left = rtTmp.right + GAP1 + 1;
		rtTmp.right = rtTmp.left+TIME_STATIC*2;
		rtTmp.bottom = rtTmp.top+TIME_STATIC;

		char chnId[6] = {0};
		sprintf(chnId,"%2d",i);
		strcat(chnId, ":00");
		
		pStatic1[i] = CreateStatic(rtTmp, this, chnId);
		pStatic1[i]->SetTextAlign(VD_TA_CENTER);

		rtTmp.left = rtTmp.right + GAP1 - 1;
		rtTmp.right = rtTmp.left+CHKBOXW;
	}

	pCheckbox[12]->SetValue(1);

	rtTmp.left = rt.left;
	rtTmp.right = rtTmp.left+rtTemp.right;
	rtTmp.top =  rtTmp.bottom + 8;
	rtTmp.bottom = rtTmp.top + TIME_STATIC;

	pStatic0[2] = CreateStatic(rtTmp, this, szTitle[2]);
	pStatic0[2]->SetTextAlign(VD_TA_CENTER);

	rtTmp.left = 4+rt.left+rtTemp.left + 80;
	rtTmp.right = rtTmp.left + CHKBOXW;
	pCheckbox[TIME_CTR] = CreateCheckBox(rtTmp,this,styleEditable,NULL/*(CTRLPROC)&CPageMainTain::OnCheckSel*/);
	pCheckbox[TIME_CTR]->SetValue(0);
	
	rtTmp.left = m_Rect.Width()-200;
	rtTmp.right = rtTmp.left + 80;
	rtTmp.bottom = rtTmp.top + 25;
	
	for(i=0; i<2; i++)
	{
		pButton[i] = CreateButton(rtTmp, this, szButton[i], (CTRLPROC)&CPageMainTain::OnClkBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
		rtTmp.left = rtTmp.right + 10;
		rtTmp.right = rtTmp.left + 80;
		pButton[i]->Show(TRUE);
	}

	CurTimeSel = 0;
}

CPageMainTain::~CPageMainTain()
{

}

VD_BOOL CPageMainTain::UpdateData( UDM mode )
{
	if (UDM_OPEN == mode)
	{
		int CurMainTain = 0;
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_SYSTEMMAINTAIN;
		
		SBizMainTainPara bizPreCfg;
		BizGetPara(&bizTar, &bizPreCfg);

		CurMainTain = (int)bizPreCfg.nMainType;

		pCombox[0]->SetCurSel(CurMainTain);

		pCombox[3 - CurMainTain]->SetCurSel((int)bizPreCfg.nSubType);
		
		OnComTypeSelect();
		
		for(int i=0; i<TIME_CTR; i++)
		{
			if(i == bizPreCfg.ntime)
			{
				CurTimeSel = i;
				pCheckbox[i]->SetValue(TRUE);
			}
			else
			{
				pCheckbox[i]->SetValue(FALSE);
			}
		}

		pCheckbox[TIME_CTR]->SetValue((int)bizPreCfg.nEnable);
	}
	else if (UDM_CLOSED== mode)
	{
		Close();
	}
	
	return TRUE;
}


void CPageMainTain::OnComTypeSelect()
{
	int CurSelect = 0;
	CurSelect = pCombox[0]->GetCurSel();

	for (int i = 1; i< 4; i++)
	{
		pCombox[i]->Show(FALSE,TRUE);
	}
	
	pCombox[3 - CurSelect]->Show(TRUE,TRUE);
}

void CPageMainTain::OnClkBtn()
{
	int i = 0;
	UDM ret = UDM_OK;
	BOOL bFind = FALSE;

	SBizMainTainPara CurMainTainPara;
	
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
				u8 CurEnable = (u8)pCheckbox[TIME_CTR]->GetValue();
				u8 CurMainType = (u8)pCombox[0]->GetCurSel();
				u8 CurSubType = (u8)pCombox[3 - CurMainType]->GetCurSel();
				u8 CurTime = 0;

				for(u8 i=0; i<TIME_CTR; i++)
				{
					if(pCheckbox[i]->GetValue())
					{
						CurTime = i;
						break;
					}
				}
				
				CurMainTainPara.nMainType = CurMainType;
				CurMainTainPara.nSubType = CurSubType;
				CurMainTainPara.nEnable = CurEnable;
				CurMainTainPara.ntime = CurTime;
				printf("CurTime = %d\n",CurTime);

				if (2 == CurMainTainPara.nMainType && CurMainTainPara.nSubType > 27)
				{
					ret= MessageBox("&CfgPtn.SomeMouthNoOption", "&CfgPtn.WARNING", MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2);
				}

				if (UDM_OK == ret)
				{
					SBizParaTarget bizTar;
					bizTar.emBizParaType = EM_BIZ_SYSTEMMAINTAIN;
				
					BizSetPara(&bizTar, &CurMainTainPara);
					this->Close();
				}
			}
			break;
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

void CPageMainTain::OnCheckSel()
{
	CCheckBox* pFocus = (CCheckBox*)GetFocusItem();

	u8 i = 0;
	CurTimeSel = 0;

	for(i=0; i<TIME_CTR; i++)
	{
		if(pFocus == pCheckbox[i])
		{
			break;
		}
	}

	for(i=0; i<TIME_CTR; i++)
	{
		if(pFocus==pCheckbox[i])
		{
			CurTimeSel = i;
			pFocus->SetValue(TRUE);
		}
		else
		{
			pCheckbox[i]->SetValue(FALSE);
		}
	}
}

VD_BOOL CPageMainTain::MsgProc(uint msg, uint wpa, uint lpa)
{
    return CPage::MsgProc(msg, wpa, lpa);
}

