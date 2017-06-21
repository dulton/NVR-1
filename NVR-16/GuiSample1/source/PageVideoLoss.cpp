#include "GUI/Pages/PageVideoLoss.h"
#include "GUI/Pages/PageAlarmDeal.h"
#include "GUI/Pages/BizData.h"
#include "biz.h"


static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static VD_BITMAP * pBmp_infoicon;

static int GetMaxChnNum()
{
	return GetVideoMainNum();
}

static int GetVLossRows()
{
	return GetMaxChnNum()>VIDEOLOSS_ROWS?VIDEOLOSS_ROWS:GetMaxChnNum();
}

int bizData_GetVLOSTAlarmDealDefault(u8 nCh, SGuiAlarmDispatch* pGDispatch);
extern int bizData_SaveVLOSTAlarmDeal(u8 nCh, SGuiAlarmDispatch* pDispatch);
extern void bizData_GetVLOSTAlarmDeal(u8 nCh, SGuiAlarmDispatch* pDispatch);

CPageVideoLoss::CPageVideoLoss( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPageFrame(pRect, psz, icon, pParent)
{
	nChMax = GetVideoMainNum();
	SetMargin(0,0,0,0);

	m_page = 0;
	m_maxPage = 1;

	memset(pBtAlarmDeal,0,sizeof(pBtAlarmDeal));
	memset(pStaticChn,0,sizeof(pStaticChn));

	//pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/button_normal.bmp");
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	//pBmp_button_push = VD_LoadBitmap(DATA_DIR"/button_push.bmp");
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	//pBmp_button_select = VD_LoadBitmap(DATA_DIR"/button_select.bmp");
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	const int nBtDealWidth = 140;//145;
	const int nBtdefaultWidth = 85;
	const int nBtHeight = 22;//25;

	int i=0;

	CRect rtTab;
	rtTab.left = 17;
	rtTab.right = m_Rect.Width()-17;
	rtTab.top = 65;
	rtTab.bottom = rtTab.top + CTRL_HEIGHT*8;
	pTab[0] = CreateTableBox(rtTab, this, 2, 8);
	pTab[0]->SetColWidth(0, 40);

	if(GetMaxChnNum()>VIDEOLOSS_ROWS)
	{
		m_maxPage = GetMaxChnNum()%VIDEOLOSS_ROWS?GetMaxChnNum()/VIDEOLOSS_ROWS+1:GetMaxChnNum()/VIDEOLOSS_ROWS;
		
		pScroll = CreateScrollBar(CRect(m_Rect.Width()-17-25,
											m_Rect.top+65,
											m_Rect.Width()-17,
											m_Rect.top+65+CTRL_HEIGHT*8), 
											this,
											scrollbarY,
											0,
											(m_maxPage-1)*10, 
											1, 
											(CTRLPROC)&CPageVideoLoss::OnTrackMove);

	}

	rtTab.top = rtTab.bottom;
	rtTab.bottom = rtTab.top + CTRL_HEIGHT;
	CStatic *pTextAll = CreateStatic(rtTab,this, "&CfgPtn.All");
	pTextAll->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);

	rtTab.top = rtTab.bottom;
	rtTab.bottom = rtTab.top + 30;
	pTab[1] = CreateTableBox(rtTab, this, 2, 1);
	pTab[1]->SetColWidth(0, 40);

	CRect rtTmp;
	pTab[1]->GetTableRect(0, 0, &rtTmp);
	rtTmp.left += rtTab.left+5;
	rtTmp.top += rtTab.top+2;
	rtTmp.right = rtTmp.left + 24;
	rtTmp.bottom = rtTmp.top + 24;
	pChkChnAll = CreateCheckBox(rtTmp, this);
	
	pTab[1]->GetTableRect(1, 0, &rtTmp);
	rtTmp.left += rtTab.left+5;
	rtTmp.top += rtTab.top+1;
	rtTmp.right = rtTmp.left + nBtDealWidth;
	rtTmp.bottom = rtTmp.top + nBtHeight;
	pBtDealAll = CreateButton(rtTmp, this, "&CfgPtn.AlarmHandling", (CTRLPROC)&CPageVideoLoss::OnClkAlarmDealAll, NULL, buttonNormalBmp);
	pBtDealAll->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);

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
		pButton[i] = CreateButton(rtTmp, this, szApp[i], (CTRLPROC)&CPageVideoLoss::OnClkApp, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);

		rtTmp.left = rtTmp.right + 20;
		rtTmp.right = rtTmp.left + nBtdefaultWidth;
	}

	m_pPageAlarmDeal = new CPageAlarmDeal(pRect, "&CfgPtn.AlarmHandling", NULL, this, 1);

	BIZ_DATA_DBG("m_pPageAlarmDeal  1111");

	int offset_x = 17;
	int offset_y = 65;

	int rows = GetVLossRows();
	for (i=0; i<rows; i++)
	{
		pTab[0]->GetTableRect(1, i, &rtTmp);
		rtTmp.left += offset_x+5;
		rtTmp.top += offset_y+1;
		rtTmp.right = rtTmp.left + nBtDealWidth;
		rtTmp.bottom = rtTmp.top+ nBtHeight;
		pBtAlarmDeal[i] = CreateButton(rtTmp, this, "&CfgPtn.AlarmHandling", (CTRLPROC)&CPageVideoLoss::OnClkAlarmDeal, NULL, buttonNormalBmp);
		pBtAlarmDeal[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);

		char tmp[5] = {0};
		sprintf(tmp, "%d", i+1);
		pTab[0]->GetTableRect(0, i, &rtTmp);
		rtTmp.left += offset_x;
		rtTmp.top += offset_y;
		rtTmp.right += offset_x;
		rtTmp.bottom += offset_y;
		pStaticChn[i] = CreateStatic(rtTmp, this, tmp);
		pStaticChn[i]->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
	
	}
	
	psAlarmDispatchIns = (SGuiAlarmDispatch *)malloc(sizeof(SGuiAlarmDispatch)*(nChMax+1));
	if(psAlarmDispatchIns==NULL)
	{
		BIZ_DO_DBG("Not enough memory for psAlarmDispatchIns !!!\n");
		exit(1);
	}
}

CPageVideoLoss::~CPageVideoLoss()
{

}

void CPageVideoLoss::AdjustRows()
{
	int firstHide = VIDEOLOSS_ROWS-(m_maxPage*VIDEOLOSS_ROWS-GetMaxChnNum());
	int lastHide = VIDEOLOSS_ROWS;
	if(m_page == m_maxPage-1)
	{
		
		for(int i=firstHide; i<lastHide; i++)
		{	
			//printf("pStatic0[i+LIVECFG_COLS]  = %d \n",pStatic0[i+LIVECFG_COLS]);
			if(pStaticChn[i]
				&& pBtAlarmDeal[i])
			{
				pStaticChn[i]->Show(FALSE,TRUE);
				pBtAlarmDeal[i]->Show(FALSE,TRUE);
			}
			
		
		}
	}
	else
	{
		for(int i=firstHide; i<lastHide; i++)
		{
			if(pStaticChn[i]
				&& pBtAlarmDeal[i])
			{
				pStaticChn[i]->Show(TRUE,TRUE);
				pBtAlarmDeal[i]->Show(TRUE,TRUE);
			}
		}
	}

}

void CPageVideoLoss::OnTrackMove()
{
	int pos = pScroll->GetPos();
    if (m_page==pos) {
        return;
    }
    m_page = pos;

	for(int i=0; i<VIDEOLOSS_ROWS; i++)
	{
		
		char szID[16] = {0};
		int nID = m_page*VIDEOLOSS_ROWS+i;

		if(nID >= GetMaxChnNum())
		{
			break;
		}
		
		sprintf(szID,"%d",nID+1);
		pStaticChn[i]->SetText(szID);
	
	}

	AdjustRows();
}

void CPageVideoLoss::OnClkAlarmDeal()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < VIDEOLOSS_ROWS; i++)
	{
		if (pFocusButton == pBtAlarmDeal[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		int nCh = m_page*VIDEOLOSS_ROWS+i;
		//printf("alarm deal %d\n", nCh);
		if (m_pPageAlarmDeal)
		{
			printf("%s 1\n", __func__);
			m_pPageAlarmDeal->SetAlarmDeal(ALARMDEAL_VIDEOLOSS, nCh);
			printf("%s 2\n", __func__);
			m_pPageAlarmDeal->Open();
			printf("%s 3\n", __func__);
		}
		printf("%s 4\n", __func__);
	}
}

void CPageVideoLoss::OnClkAlarmDealAll()
{
	if (m_pPageAlarmDeal)
	{
		m_pPageAlarmDeal->SetAlarmDeal(ALARMDEAL_VIDEOLOSS, -1);
		m_pPageAlarmDeal->Open();
	}
}

void CPageVideoLoss::OnClkApp()
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
		case 0: //default
		{
			int ret = 0;
			for(int i=0; i<nChMax; i++)
			{
				ret = bizData_GetVLOSTAlarmDealDefault(i, &psAlarmDispatchIns[i]);
			}

			if(ret == 0)
			{		
				SetInfo("&CfgPtn.LoadDefaultSuccess");
			}
			else
			{
				SetInfo("&CfgPtn.LoadDefaultFail");
			}
		}break;
		case 1://apply
		{
			BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_VIDEOLOSS_ALARMDEAL);
				
			if(pChkChnAll->GetValue())
			{
				for(int j=0;j<nChMax;j++)
				{
					if(psAlarmDispatchIns)
					{
						memcpy(&psAlarmDispatchIns[j],
							&psAlarmDispatchIns[nChMax],
							sizeof(SBizAlarmDispatch)
						);
					}
				}
			}
			
			int ret = SaveDealPara2Cfg(nChMax);
			if(ret == 0)
			{		
				SetInfo("&CfgPtn.SaveConfigSuccess");
			}
			else
			{
				SetInfo("&CfgPtn.SaveConfigFail");
			}
		} break;
		case 2://exit
			this->Close();
			break;
		}
	}
}

VD_BOOL CPageVideoLoss::UpdateData( UDM mode )
{
	if (UDM_OPEN == mode)
	{
		for(int i=0; i<nChMax; i++)
		{
			bizData_GetVLOSTAlarmDeal(i, &psAlarmDispatchIns[i]);
		}
		memcpy(&psAlarmDispatchIns[nChMax], &psAlarmDispatchIns[0], sizeof(SGuiAlarmDispatch));

        //pChkChnAll->SetValue(0);
	} 
	else if (UDM_CLOSED == mode)
	{
        ((CPageAlarmCfgFrameWork *)(this->GetParent()))->ReflushItemName();
    }
	return TRUE;
}


void CPageVideoLoss::GetPara(uchar nCh, SGuiAlarmDispatch *pPara)
{
	int nChMax = GetVideoMainNum();
	
	if(nCh>=nChMax)
	{
		nCh = nChMax;
	}
	
	if(pPara&&psAlarmDispatchIns&&nCh<=nChMax)
		memcpy( pPara, &psAlarmDispatchIns[nCh], sizeof(SBizAlarmDispatch) );
	
	BIZ_DO_DBG("CH %d Sound %d Mail %d\n", nCh, pPara->nFlagBuzz, pPara->nFlagEmail);
	
	return;
}

void CPageVideoLoss::SavePara(uchar nCh, SGuiAlarmDispatch *pPara)
{
	int nChMax = GetVideoMainNum();
	
	if(nCh>=nChMax)
	{
		nCh = nChMax;
	}
	
	BIZ_DO_DBG("CH %d Sound %d Mail %d\n", nCh, pPara->nFlagBuzz, pPara->nFlagEmail);
	
	//printf("CH %d Sound %d Mail %d ***\n", nCh, pPara->nFlagBuzz, pPara->nFlagEmail);
	if(pPara&&psAlarmDispatchIns&&nCh<=nChMax)
		memcpy( &psAlarmDispatchIns[nCh], pPara, sizeof(SBizAlarmDispatch) );
	
	return;
}

int CPageVideoLoss::SaveDealPara2Cfg(uchar nRealChNum)
{
	if(!psAlarmDispatchIns) return -1;
	
	int nChMax = GetVideoMainNum();
	
	if(nChMax>nRealChNum)
		nChMax = nRealChNum;

	int ret = 0;
	for(int i=0; i<nChMax; i++)
	{
		BIZ_DO_DBG("CH %d Sound %d Mail %d\n", i, psAlarmDispatchIns[i].nFlagBuzz, psAlarmDispatchIns[i].nFlagEmail);
		ret =  bizData_SaveVLOSTAlarmDeal(i, &psAlarmDispatchIns[i]);
	}

	return ret;
}

void CPageVideoLoss::SetInfo(char* szInfo)
{
	pIconInfo->Show(TRUE);
	
	pInfoBar->SetText(szInfo);
}

void CPageVideoLoss::ClearInfo()
{
	pIconInfo->Show(FALSE);
	
	pInfoBar->SetText("");
}

VD_BOOL CPageVideoLoss::MsgProc( uint msg, uint wpa, uint lpa )
{
	int px,py;
	switch(msg)
	{
	case XM_MOUSEWHEEL://cw_scrollbar
		{
			int px = VD_HIWORD(lpa);
			int py = VD_LOWORD(lpa);
			CItem* pItem = GetItemAt(px, py);
			if(pItem == NULL)
			{
				if(GetMaxChnNum()>VIDEOLOSS_ROWS)
				{
					pScroll->MsgProc(msg, wpa, lpa);
					break;
				}
			}
			break;
		}
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

