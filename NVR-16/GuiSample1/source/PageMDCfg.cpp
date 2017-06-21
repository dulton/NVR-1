#include "GUI/Pages/PageMDCfg.h"

#include "GUI/Pages/PageAlarmDeal.h"
#include "GUI/Pages/PageMotionRgnSet.h"
#include "GUI/Pages/BizData.h"

static	std::vector<CItem*>	items[MDCFG_SUBPAGES];

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static VD_BITMAP * pBmp_tab_normal;
static VD_BITMAP * pBmp_tab_push;
static VD_BITMAP * pBmp_tab_select;

static VD_BITMAP * pBmp_infoicon;

static char* szMDDelayTime[6] = {
	"5",
	"10",
	"20",
	"30",
	"60",
	"120",
};


void bizData_SaveMDAlarmPara(int ch, SMDCfg* pGAlarm);
void bizData_GetMDAlarmDefault(int ch, SMDCfg* pGAlarm);
void bizData_GetMDAlarmPara(int ch, SMDCfg* pGAlarm);

int bizData_GetVMOTIONAlarmDealDefault(u8 nCh, SGuiAlarmDispatch* pGDispatch);
extern void bizData_SaveVMOTIONAlarmDeal(u8 nCh, SGuiAlarmDispatch* pDispatch);
extern void bizData_GetVMOTIONAlarmDeal(u8 nCh, SGuiAlarmDispatch* pDispatch);

static STabSize TabSzMD[] = {
	{ 5, 6 },
};

static int GetMaxChnNum() { return GetVideoMainNum(); }

CPageMDCfg::CPageMDCfg( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPageFrame(pRect, psz, icon, pParent)
, m_nCurID(0)
{
	nChMax = GetMaxChnNum();

	if(TabSzMD[0].nRow >= nChMax)
	{
		nRealRow = nChMax;
		nPageNum = 1;
		nHideRow = 0;
	}
	else
	{
		nRealRow = TabSzMD[0].nRow;
		nPageNum = (nChMax+nRealRow-1)/nRealRow;			
		nHideRow = nPageNum*nRealRow-nChMax;
	}
	
	nCurPage = 0;
	
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

	pBmp_tab_normal = VD_LoadBitmap(DATA_DIR"/temp/tab.bmp");
	pBmp_tab_push = VD_LoadBitmap(DATA_DIR"/temp/tab_check.bmp");
	pBmp_tab_select = VD_LoadBitmap(DATA_DIR"/temp/tab_f.bmp");

	//第一个页面的子页面控制按钮  录像设置，手动录像，录像布防
	char* szSubPgName[MDCFG_COMMBUTTON] = {
		"&CfgPtn.Motion",
		"&CfgPtn.Schedule",	
		"&CfgPtn.Default",
		"&CfgPtn.apply",
		"&CfgPtn.Exit",
	};

	pTabFrame = CreateTableBox(CRect(17, 55+pBmp_tab_normal->height, m_Rect.Width()-17, m_Rect.Height()-45),this,1,1);
	pTabFrame->SetFrameColor(VD_RGB(56,108,148));

	CRect rtSub(26, 55, m_Rect.Width()-20, 55+pBmp_tab_normal->height);
	int i;
	for (i = 0; i<MDCFG_SUBPAGES; i++)
	{
		int btWidth = strlen(szSubPgName[i])*TEXT_WIDTH/2+10;
		rtSub.right = rtSub.left + btWidth;
		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageMDCfg::OnClickSubPage, NULL, buttonNormalBmp, 1);
		pButton[i]->SetBitmap(pBmp_tab_normal, pBmp_tab_select, pBmp_tab_push, pBmp_tab_push);
		rtSub.left = rtSub.right;
	}

	int btwidth = 85;
	int btspace = 16;
	rtSub.top = m_Rect.Height()-38;
	rtSub.bottom = rtSub.top + pBmp_button_normal->height;
	rtSub.left = m_Rect.Width() - 20 - btwidth*(MDCFG_COMMBUTTON - MDCFG_SUBPAGES) - btspace*(MDCFG_COMMBUTTON - MDCFG_SUBPAGES - 1);
	rtSub.right = rtSub.left + btwidth;

	pInfoBar = CreateStatic(CRect(40, 
								rtSub.top,
								317,
								rtSub.top+22), 
							this, 
							"");
	//pInfoBar->SetBkColor(VD_RGB(8,66,189));

	pBmp_infoicon = VD_LoadBitmap(DATA_DIR"/temp/icon_info.bmp");
	pIconInfo = CreateStatic(CRect(17, 
								rtSub.top+1,
								37,
								rtSub.top+21), 
							this, 
							"");
	pIconInfo->SetBitmap(pBmp_infoicon,NULL);
	pIconInfo->Show(FALSE);
	
	for (i=MDCFG_SUBPAGES; i<MDCFG_COMMBUTTON; i++)
	{
		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageMDCfg::OnClickSubPage, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);

		rtSub.left = rtSub.right + btspace;
		rtSub.right = rtSub.left + btwidth;

	}
	bSchChange = (uchar*)malloc(sizeof(uchar)*nChMax);
	sch = (uint**)malloc(sizeof(uint*)*nChMax);
	for(int i=0; i<nChMax; i++)
	{
		sch[i] = (uint*)malloc(sizeof(uint)*7);

		for(int j=0; j<7; j++)
		{
			sch[i][j] = 0;
		}
	}
	
	//printf("InitPage0 \n");
	InitPage0();
	//printf("InitPage2 \n");
	
	InitPage1();
#ifndef DISABLE_ALARM_SCH
	//printf("InitPage1 \n");
#else
	pButton[1]->Show(FALSE, TRUE);
#endif
	
	ShowSubPage(0,TRUE);
	pButton[0]->Enable(FALSE);
	
	CRect rtPage(0,0,642,418);
	
	//csp modify 20130311
	if(strcmp(GetParsedString("&CfgPtn.MotionHandling"), "CfgPtn.MotionHandling") == 0)
	{
		m_pPageAlarmDeal = new CPageAlarmDeal(rtPage, "&CfgPtn.AlarmHandling", NULL, this, 2);
	}
	else
	{
		m_pPageAlarmDeal = new CPageAlarmDeal(rtPage, "&CfgPtn.MotionHandling", NULL, this, 2);
	}
	BIZ_DATA_DBG("m_pPageAlarmDeal 2222");
	
	m_pPageMDRgnSet = new CPageMotionRgnSet(NULL);
	
	BIZ_DATA_DBG("m_pPageAlarmDeal 3333");
	m_pPageMDRgnSet->SetGrid(22,18);
	
	psAlarmDispatchIns = (SGuiAlarmDispatch *)malloc(sizeof(SGuiAlarmDispatch)*(nChMax+1));
	if(psAlarmDispatchIns==NULL)
	{
		BIZ_DO_DBG("Not enough memory for psAlarmDispatchIns !!!\n");
		exit(1);
	}
}

CPageMDCfg::~CPageMDCfg()
{
	
}

void CPageMDCfg::OnTrackMove0()
{
	VD_BOOL bHide = FALSE;
	int 	idx;
	
	int pos = pScrollbar->GetPos();
    if(nCurPage==pos)
	{
        return;
    }
    nCurPage = pos;
	
	//printf("nCurPage = %d \n", nCurPage);
	
	int firstHide = nRealRow-nHideRow;
	int lastHide = nRealRow;
	int nRow     = nRealRow;
	int nCurPg   = nCurPage;
	int nPgNum   = nPageNum;
	int nCol     = TabSzMD[0].nCol;
	for(int i=0; i<nRealRow; i++)
	{
		CStatic*	pStatic = tMDPage0[i].pChnIndex;
		char szID[16] = {0};
		idx = nCurPage*nRealRow+i;
		sprintf(szID,"%d",idx+1);
		pStatic->SetText(szID);
	}
	
	bHide = (nCurPg == nPgNum-1)?FALSE:TRUE;
	
	for(int i=0; i<nRow; i++)
	{
		idx = nCurPage*nRealRow+i;
		tMDPage0[i].pChkEnable->SetValue(pTabPara[idx].bEnable);
		tMDPage0[i].pComboDelay->SetCurSel(pTabPara[idx].nDelay);
		
		if(idx >= nChMax - 1)
		{
			break;
		}
	}
	
	for(int i=firstHide; i<lastHide; i++)
	{
		tMDPage0[i].pChnIndex->Show(bHide,TRUE);
		tMDPage0[i].pChkEnable->Show(bHide,TRUE);
		tMDPage0[i].pComboDelay->Show(bHide,TRUE);
		tMDPage0[i].pBtRgnSet->Show(bHide,TRUE);
		tMDPage0[i].pBtAlarmDeal->Show(bHide,TRUE);
	}
}

void CPageMDCfg::OnEnable()
{
	int i = 0, j;
	BOOL bFind = FALSE;
	CCheckBox* pFocus = (CCheckBox*)GetFocusItem();
	for (i = 0; i<nRealRow; i++)
	{
		if (pFocus == tMDPage0[i].pChkEnable)
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		int nChnBase = nCurPage*nRealRow+i;
		pTabPara[nChnBase].bEnable = pFocus->GetValue();
	}
}

void CPageMDCfg::AdjHide()
{
	if( nPageNum-1 == nCurPage && nPageNum > 1 )
	{
		VD_BOOL bHide = FALSE;
		
		int firstHide = nRealRow-nHideRow;
		int lastHide = nRealRow;
		int nRow     = nRealRow;
		int nCol     = TabSzMD[0].nCol;
	
		for(int i=firstHide; i<lastHide; i++)
		{
			tMDPage0[i].pChnIndex->Show(bHide,TRUE);
			tMDPage0[i].pChkEnable->Show(bHide,TRUE);
			tMDPage0[i].pComboDelay->Show(bHide,TRUE);
			tMDPage0[i].pBtAlarmDeal->Show(bHide,TRUE);	
			tMDPage0[i].pBtRgnSet->Show(bHide,TRUE);
		}		
	}	
}

void CPageMDCfg::OnDelay()
{
	int i = 0, j;
	BOOL bFind = FALSE;
	CComboBox* pFocus = (CComboBox*)GetFocusItem();
	for (i = 0; i<nRealRow; i++)
	{
		if (pFocus == tMDPage0[i].pComboDelay)
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		int nChnBase = nCurPage*nRealRow+i;
		pTabPara[nChnBase].nDelay = pFocus->GetCurSel();
	}
}

void CPageMDCfg::SwitchPage( int subID )
{
	if (subID<0 || subID>=MDCFG_SUBPAGES )
	{
		return;
	}

	if (subID == m_nCurID)
	{
		return;
	}

	//LoadPara(); //yzw del

	//隐藏当前子页面，显示新的子页面
	ShowSubPage(m_nCurID, FALSE);
	pButton[m_nCurID]->Enable(TRUE);
	m_nCurID = subID;
	ShowSubPage(m_nCurID, TRUE);
	pButton[subID]->Enable(FALSE);
}

void CPageMDCfg::ShowSubPage( int subID, BOOL bShow )
{
	if (subID<0 || subID>=MDCFG_SUBPAGES )
	{
		return;
	}

	int count = items[subID].size();

	//printf("page %d, bshow=%d ,count=%d \n", subID, bShow, count);

	for (int i=0; i<count; i++)
	{
		if(items[subID][i])
		{
			items[subID][i]->Show(bShow);
		}
	}
	AdjHide();
}

void CPageMDCfg::OnClickSubPage()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < MDCFG_COMMBUTTON; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{	
		//printf("find the focus button %d\n", i);
		if(i <MDCFG_SUBPAGES)
		{
			SwitchPage(i);
		} 
		else
		{
			//default|apply| exit
			if(i==MDCFG_SUBPAGES+1) // apply
			{
				BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_MD_ENABLE);
				BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_MD_AREA_SETUP);
				BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_MD_ALARMDEAL);
				BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_MD_DETECTION_SCHE);
				
				//switch(m_nCurID)
				switch(m_nCurID)
				{
					case 0:
					{
						int ret = 0;
						int itemNum = nRealRow;
						if(pChkChnAllPage0->GetValue())
						{		
							int allEnable = pChkEnableAllPage0->GetValue();
							int allId = pComboDelayAllPage0->GetCurSel();
							
							for(i=0; i<nChMax; i++)
							{
								pTabPara[i].bEnable = allEnable;
								pTabPara[i].nDelay = allId;
							}
							for(i=0;i<itemNum;i++)
							{								
								tMDPage0[i].pChkEnable->SetValue(allEnable);

								tMDPage0[i].pComboDelay->SetCurSel(allId);
							}
							
							for(i=0; i<nChMax; i++)   //cw_md
							{
								if(psAlarmDispatchIns)
								{
									memcpy(&psAlarmDispatchIns[i],
										&psAlarmDispatchIns[nChMax],
										sizeof(SBizAlarmDispatch)
									);
								}
							}
							
							if (m_pPageMDRgnSet)  //cw_md;
							{
								m_pPageMDRgnSet->SaveAllChnRgnPara();
							}
								
						}

						for(i=0; i<nChMax; i++)
						{
							bizData_SaveMDAlarmPara(i, &pTabPara[i]);							
						}
						SaveDealPara2Cfg(nChMax);
						SetInfo("&CfgPtn.SaveConfigSuccess");
					}
					break;
					case 1:
					{
						int ret = 0;
						//for(int i=0; i<sizeof(sch)/sizeof(sch[0]); i++)
						for(int i = 0; i < nChMax; i++)
						{
							ret = SaveScrStr2Cfg(
								sch[i],
								GSR_CONFIG_ALARM_MD_SCH_SCH_SCH, 
								EM_GSR_CTRL_SCH_VAL, 
								i
							);
						}

						if(ret == 0)
						{
							SetInfo("&CfgPtn.SaveConfigSuccess");
						}
						else
						{
							SetInfo("&CfgPtn.SaveConfigFail");
						}
					}
					break;
				}
			}
			else if(i==MDCFG_SUBPAGES)
			{
				//default
				int ret = 0;
				if(m_nCurID == 0)
				{
					int ret = 0;
					for(i=0; i<nChMax; i++)
					{
						SMDCfg sPIns;
						bizData_GetMDAlarmDefault( i, &sPIns );
						pTabPara[i].bEnable = sPIns.bEnable;
						pTabPara[i].nDelay = sPIns.nDelay;
					}
					
					int nChBase = nCurPage*nRealRow;
					for(i = 0; i<nRealRow; i++)
					{
						tMDPage0[i].pChkEnable->SetValue(pTabPara[nChBase+i].bEnable);
						tMDPage0[i].pComboDelay->SetCurSel(pTabPara[nChBase+i].nDelay);
					}
					
					for(int i=0; i<nChMax; i++)
					{
						bizData_GetVMOTIONAlarmDealDefault(i, &psAlarmDispatchIns[i]);
					}
					
					//csp modify
					pChkChnAllPage0->SetValue(0);
					pChkEnableAllPage0->SetValue(pTabPara[0].bEnable);
					pComboDelayAllPage0->SetCurSel(pTabPara[0].nDelay);
					
					if(ret >= 0)
					{
						SetInfo("&CfgPtn.LoadDefaultSuccess");
					}
					else
					{
						SetInfo("&CfgPtn.LoadDefaultFail");
					}
				}
				else
				{
					int nCh = pComboChnPage1->GetCurSel();
						
					int ret = GetScrStrSetCurSelDo(
						(void*)&pSCHGridPage1[0],
						GSR_CONFIG_ALARM_MD_SCH_SCH_SCH+GSR_DEFAULT_OFFSET, 
						EM_GSR_CTRL_SCH,
						nCh
					);
					
					uint mask = 0;
					
					for(int i=0; i<7; i++)
					{
						pSCHGridPage1[i]->GetMask(&mask);
						//BIZ_DO_DBG("GET msk idx %d mask %x\n", i, mask);
						SetSch(nCh, i, &mask);
					}

					uint mask_cur[7] = {0};
					GetSch(nCh, mask_cur);
					for(int n=0; n<7; n++)
					{
						pSCHGridPage1[n]->SetMask(&mask_cur[n]);
					}
					
					if(ret >= 0)
					{
						SetInfo("&CfgPtn.LoadDefaultSuccess");
					}
					else
					{
						SetInfo("&CfgPtn.LoadDefaultFail");
					}
				}				
			}
			else
			{
				//exit
				this->Close();
			}
		}

	}

}

#define BUTTON_WIDTH 85

void CPageMDCfg::InitPage0()
{
	pTabPara = (SMDCfg*)malloc(sizeof(SMDCfg)*nChMax);
	if(!pTabPara)
	{
		printf("no mem left!\n");
		exit(1);
	}
	
	int i= 0;
	CRect rtTab(25, 55+pBmp_tab_normal->height+10, m_Rect.Width()-25, 55+pBmp_tab_normal->height+10 + (1+CTRL_HEIGHT)*7);
	pTabPage0[0] = CreateTableBox(rtTab, this, 5, 7);
	pTabPage0[0]->SetColWidth(0, 54+10);
	pTabPage0[0]->SetColWidth(1, 92);
	pTabPage0[0]->SetColWidth(2, 165);
	pTabPage0[0]->SetColWidth(3, 130);
	items[0].push_back(pTabPage0[0]);
	
	if(nPageNum>1)
	{
		pScrollbar = CreateScrollBar(CRect(m_Rect.Width()-25-SCROLLBAR,
											55+pBmp_tab_normal->height+10+30,
											m_Rect.Width()-25,
											55+pBmp_tab_normal->height+10+30+(1+CTRL_HEIGHT)*6), 
											this,
											scrollbarY,
											0,
											(nPageNum-1)*10, 
											1, 
											(CTRLPROC)&CPageMDCfg::OnTrackMove0);
		
		items[0].push_back(pScrollbar);
	}

	//csp modify 20130311
	char* szTitle_1[5] = {
		"&CfgPtn.Channel",
		"&CfgPtn.Enable",
		"&CfgPtn.HoldingTime",
		
		//"&CfgPtn.AlarmHandling",
		"&CfgPtn.MotionHandling",
		
		"&CfgPtn.Area",
	};
	char* szTitle_2[5] = {
		"&CfgPtn.Channel",
		"&CfgPtn.Enable",
		"&CfgPtn.HoldingTime",
		
		"&CfgPtn.AlarmHandling",
		//"&CfgPtn.MotionHandling",
		
		"&CfgPtn.Area",
	};
	
	//csp modify 20130311
	char **szTitle = szTitle_1;
	if(strcmp(GetParsedString("&CfgPtn.MotionHandling"), "CfgPtn.MotionHandling") == 0)
	{
		szTitle = szTitle_2;
	}
	
	for (i= 0; i<5; i++)
	{
		CRect rt;
		pTabPage0[0]->GetTableRect(i, 0, &rt);
		CStatic* pText = CreateStatic(CRect(rtTab.left+rt.left, rtTab.top+rt.top, rtTab.left+rt.right, rtTab.top+rt.bottom), this, szTitle[i]);
		pText->SetTextAlign(VD_TA_LEFT);
		pText->SetBkColor(VD_RGB(67,77,87));
		pText->SetTextAlign(VD_TA_XLEFT);
		pText->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		items[0].push_back(pText);
	}
	
	rtTab.top = rtTab.bottom;
	rtTab.bottom = rtTab.top + CTRL_HEIGHT;
	CStatic* pTextAll = CreateStatic(rtTab, this, "&CfgPtn.All");
	pTextAll->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
	items[0].push_back(pTextAll);
	
	rtTab.top = rtTab.bottom;
	rtTab.bottom = rtTab.top + CTRL_HEIGHT+3;
	pTabPage0[1] = CreateTableBox(rtTab, this, 5, 1);  //cw_md 4->5
	pTabPage0[1]->SetColWidth(0, 54+10);
	pTabPage0[1]->SetColWidth(1, 92);
	pTabPage0[1]->SetColWidth(2, 165);
	pTabPage0[1]->SetColWidth(3, 130);
	items[0].push_back(pTabPage0[1]);
	
	CRect rt;
	pTabPage0[1]->GetTableRect(0, 0, &rt);
	pChkChnAllPage0 = CreateCheckBox(CRect(rtTab.left+rt.left+5, rtTab.top+rt.top+2, rtTab.left+rt.left+5+CTRL_HEIGHT, rtTab.top+rt.bottom), this);
	pChkChnAllPage0->SetValue(0);
	items[0].push_back(pChkChnAllPage0);
	
	pTabPage0[1]->GetTableRect(1, 0, &rt);
	pChkEnableAllPage0 = CreateCheckBox(CRect(rtTab.left+rt.left+5, rtTab.top+rt.top+2, rtTab.left+rt.left+5+CTRL_HEIGHT, rtTab.top+rt.bottom), this);
	//pChkEnableAllPage0->SetValue(1);
		
	items[0].push_back(pChkEnableAllPage0);

	pTabPage0[1]->GetTableRect(2, 0, &rt);
	pComboDelayAllPage0 = CreateComboBox(CRect(rtTab.left+rt.left+1, rtTab.top+rt.top, rtTab.left+rt.right-1, rtTab.top+rt.bottom), this, NULL);

	GetScrStrInitComboxSelDo(
		(void*)pComboDelayAllPage0,
		GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME, 
		EM_GSR_COMBLIST, 
		1
	);
	pComboDelayAllPage0->SetCurSel(0);
	
	items[0].push_back(pComboDelayAllPage0);
	
	pTabPage0[1]->GetTableRect(3, 0, &rt);   //cw_md
	pBtAlarmDealAllPage0 = CreateButton(CRect(rtTab.left+rt.left+5, rtTab.top+rt.top+1, rtTab.left+rt.left+5+BUTTON_WIDTH, rtTab.top+rt.bottom-1), 
		this, "&CfgPtn.Setup", (CTRLPROC)&CPageMDCfg::OnClkAlarmDealAll, NULL, buttonNormalBmp);
	pBtAlarmDealAllPage0->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push); 
	items[0].push_back(pBtAlarmDealAllPage0);
	
	pTabPage0[1]->GetTableRect(4, 0, &rt);
	pBtRgnSetAllPage0 = CreateButton(CRect(rtTab.left+rt.left+5, rtTab.top+rt.top+1, rtTab.left+rt.left+5+BUTTON_WIDTH, rtTab.top+rt.bottom-1), 
		this, "&CfgPtn.Setup", (CTRLPROC)&CPageMDCfg::OnClkRgnSetAll, NULL, buttonNormalBmp);
	pBtRgnSetAllPage0->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push); 
	items[0].push_back(pBtRgnSetAllPage0);




	rtTab.left = 25;
	rtTab.top = 55+pBmp_tab_normal->height+10;
	for (i = 0; i<nRealRow; i++)
	{
		char tmpbuf[32] = {0};
		sprintf(tmpbuf, "%d", i+1);
		CRect rttmp;
		pTabPage0[0]->GetTableRect(0, i+1, &rttmp);
		//printf("tab: %d %d %d %d\n", rtTab.left+rttmp.left, rtTab.top+rttmp.top, rtTab.left+rttmp.right, rtTab.top+rttmp.bottom);
		tMDPage0[i].pChnIndex = CreateStatic(CRect(rtTab.left+rttmp.left+2, rtTab.top+rttmp.top, rtTab.left+rttmp.right, rtTab.top+rttmp.bottom), this, tmpbuf);
		tMDPage0[i].pChnIndex->SetTextAlign(VD_TA_LEFT);


		pTabPage0[0]->GetTableRect(1, i+1, &rttmp);
		tMDPage0[i].pChkEnable = CreateCheckBox(CRect(rtTab.left+rttmp.left+5, rtTab.top+rttmp.top+2, rtTab.left+rttmp.left+5+CTRL_HEIGHT, rtTab.top+rttmp.bottom), 
			this, styleEditable, (CTRLPROC)&CPageMDCfg::OnEnable);

		pTabPage0[0]->GetTableRect(2, i+1, &rttmp);
		tMDPage0[i].pComboDelay = CreateComboBox(CRect(rtTab.left+rttmp.left+1, rtTab.top+rttmp.top+1, rtTab.left+rttmp.right-1, rtTab.top+rttmp.bottom-1), 
			this, NULL, NULL ,(CTRLPROC)&CPageMDCfg::OnDelay);

		GetScrStrInitComboxSelDo(
			(void*)tMDPage0[i].pComboDelay,
			GSR_CONFIG_ALARM_MD_MOTIONDETECT_DELAYTIME, 
			EM_GSR_COMBLIST, 
			i
		);

		pTabPage0[0]->GetTableRect(3, i+1, &rttmp);
		tMDPage0[i].pBtAlarmDeal = CreateButton(CRect(rtTab.left+rttmp.left+5, rtTab.top+rttmp.top+1, rtTab.left+rttmp.left+5+BUTTON_WIDTH, rtTab.top+rttmp.bottom-1), 
			this, "&CfgPtn.Setup", (CTRLPROC)&CPageMDCfg::OnClkAlarmDeal, NULL, buttonNormalBmp);
		tMDPage0[i].pBtAlarmDeal->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push); 


		pTabPage0[0]->GetTableRect(4, i+1, &rttmp);
		tMDPage0[i].pBtRgnSet = CreateButton(CRect(rtTab.left+rttmp.left+5, rtTab.top+rttmp.top+1, rtTab.left+rttmp.left+5+BUTTON_WIDTH, rtTab.top+rttmp.bottom-1), 
			this, "&CfgPtn.Setup", (CTRLPROC)&CPageMDCfg::OnClkRgnSet, NULL, buttonNormalBmp);
		tMDPage0[i].pBtRgnSet->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push); 


		items[0].push_back(tMDPage0[i].pChnIndex);
 		items[0].push_back(tMDPage0[i].pChkEnable);
		items[0].push_back(tMDPage0[i].pComboDelay);
 		items[0].push_back(tMDPage0[i].pBtAlarmDeal);
		items[0].push_back(tMDPage0[i].pBtRgnSet);
	}
	
	ShowSubPage(0, FALSE);	
	//pButton[0]->Enable(FALSE);
}

void CPageMDCfg::OnClkAlarmDeal()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < nRealRow; i++)
	{
		if (pFocusButton == tMDPage0[i].pBtAlarmDeal)
		{
			bFind = TRUE;
			break;
		}
	}


	if (bFind)
	{	
		//printf("alarm deal %d\n", i);
		if (m_pPageAlarmDeal)
		{
			m_pPageAlarmDeal->SetAlarmDeal(ALARMDEAL_MOTION, i + nCurPage*nRealRow);
			m_pPageAlarmDeal->Open();
		}
	}
}

void CPageMDCfg::OnClkAlarmDealAll()  //cw_md
{
	if (m_pPageAlarmDeal)
	{
		m_pPageAlarmDeal->SetAlarmDeal(ALARMDEAL_MOTION, -1);
		m_pPageAlarmDeal->Open();
	}
}

void CPageMDCfg::OnClkRgnSet()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < nRealRow; i++)
	{
		if (pFocusButton == tMDPage0[i].pBtRgnSet)
		{
			bFind = TRUE;
			break;
		}
	}


	if (bFind)
	{	
		if (m_pPageMDRgnSet)
		{
			m_pPageMDRgnSet->SetMotionChn(i + nCurPage*nRealRow);
			m_pPageMDRgnSet->Open();
		}
	}
				
}


void CPageMDCfg::OnClkRgnSetAll()   //cw_md
{
	if (m_pPageMDRgnSet)
	{
		m_pPageMDRgnSet->SetMotionChn(-1);
		m_pPageMDRgnSet->Open();
	}
			
}

void CPageMDCfg::InitPage1()
{
	int i= 0;
	CRect rt;
	rt.left = 55;
	rt.top = 55+pBmp_tab_normal->height+10;
	rt.right = rt.left +130;
	rt.bottom = rt.top + TEXT_HEIGHT;

	CStatic* pText1 = CreateStatic(rt, this, "&CfgPtn.Channel");
	pText1->SetTextAlign(VD_TA_LEFT);
	items[1].push_back(pText1);

	rt.left = rt.right;
	rt.right = rt.left + 100;
	rt.bottom = rt.top + CTRL_HEIGHT;
	pComboChnPage1 = CreateComboBox(rt, this, NULL
	, NULL, (CTRLPROC)&CPageMDCfg::OnComboBoxChn, 0);
	
	GetScrStrInitComboxSelDo(
		(void*)pComboChnPage1,
		GSR_CONFIG_ALARM_MD_SCH_SCH_CHN, 
		EM_GSR_COMBLIST, 
		0
	);

	pComboChnPage1->SetCurSel(0);
	
	items[1].push_back(pComboChnPage1);

	rt.left = 55;
	rt.top = 55+pBmp_tab_normal->height+40;
	rt.right = m_Rect.Width()-55;
	rt.bottom = rt.top + TEXT_HEIGHT*8;

	pTabSCHPage1 = CreateTableBox(rt, this, 2, 8);
	pTabSCHPage1->SetColWidth(0, 100);
	items[1].push_back(pTabSCHPage1);


	for (i = 0 ; i < 6; i++)
	{
		CRect rtTime;
		pTabSCHPage1->GetTableRect(1, 0, &rtTime);

		char szTmp[10] = {0};
		sprintf(szTmp, "|%02d:00", i*4);

		CStatic* pSCHTime = CreateStatic(CRect(rt.left+rtTime.left +i*(rtTime.Width()/6)-3+(i>3?(i-3):0), rt.top+rtTime.top, rt.left+rtTime.right,rt.top+rtTime.bottom), 
			this, szTmp);

		pSCHTime->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		pSCHTime->SetTextAlign(VD_TA_LEFTTOP);

		items[1].push_back(pSCHTime);
	}

	char* szWeekDay[7] = {
		"&CfgPtn.Sunday",
		"&CfgPtn.Monday",
		"&CfgPtn.Tuesday",
		"&CfgPtn.Wednesday",
		"&CfgPtn.Thursday",
		"&CfgPtn.Friday",
		"&CfgPtn.Saturday"
	};
	
	for (i= 0; i<7; i++)
	{
		CRect rtGrid;
		pTabSCHPage1->GetTableRect(1, i+1, &rtGrid);
		pSCHGridPage1[i] = CreateGridCtrl(CRect(rt.left+rtGrid.left, rt.top+rtGrid.top, rt.left+rtGrid.right,rt.top+rtGrid.bottom),
			this, 1, 24);

		pSCHGridPage1[i]->SetColor(VD_RGB(72,110,216), VD_RGB(67,76,80));

		items[1].push_back(pSCHGridPage1[i]);

		CRect rtWeek;
		pTabSCHPage1->GetTableRect(0, i+1, &rtWeek);
		CStatic* pWeekDay = CreateStatic(CRect(rt.left+rtWeek.left, rt.top+rtWeek.top, rt.left+rtWeek.right,rt.top+rtWeek.bottom),
			this, szWeekDay[i]);
		items[1].push_back(pWeekDay);
	}
	
	GetScrStrSetCurSelDo(
		(void*)&pSCHGridPage1[0],
		GSR_CONFIG_ALARM_MD_SCH_SCH_SCH, 
		EM_GSR_CTRL_SCH, 
		pComboChnPage1->GetCurSel()
	);

	rt.left = 55;
	rt.top = rt.bottom+20;
	rt.right = rt.left+130;
	rt.bottom = rt.top + TEXT_HEIGHT;

	CStatic* pText2 = CreateStatic(rt, this, "&CfgPtn.ApplySettingsTo");
	pText2->SetTextAlign(VD_TA_LEFT);
	items[1].push_back(pText2);

	rt.left = rt.right;
	rt.right = rt.left+ 100;
	rt.bottom = rt.top + CTRL_HEIGHT;
	pComboCopy2ChnPage1 = CreateComboBox(rt, this, NULL);
	
	GetScrStrInitComboxSelDo(
		(void*)pComboCopy2ChnPage1,
		GSR_CONFIG_ALARM_MD_SCH_SCH_COPYTO, 
		EM_GSR_COMBLIST, 
		0
	);

	pComboCopy2ChnPage1->SetCurSel(0);
	
	items[1].push_back(pComboCopy2ChnPage1);

	rt.left = rt.right+10;
	rt.right = rt.left+ 120;
	rt.bottom = rt.top + TEXT_HEIGHT;
	CStatic* pText3 = CreateStatic(rt, this, "&CfgPtn.Channel");
	pText3->SetTextAlign(VD_TA_LEFT);
	items[1].push_back(pText3);

	rt.left = rt.right;
	rt.right = rt.left+ BUTTON_WIDTH;
	rt.bottom = rt.top + 25;
	pBtCopyPage1 = CreateButton(rt, this, "&CfgPtn.Copy", (CTRLPROC)&CPageMDCfg::OnClkCopy, NULL, buttonNormalBmp);
	pBtCopyPage1->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);
	items[1].push_back(pBtCopyPage1);

	ShowSubPage(1, FALSE);
}

void CPageMDCfg::LoadPara()
{
	int  i;
	u8 nChMax = GetVideoMainNum();

	for(i=0; i<nChMax; i++)
	{
		bizData_GetMDAlarmPara(i, &pTabPara[i]);
		bizData_GetVMOTIONAlarmDeal(i, &psAlarmDispatchIns[i]);
	}
	memcpy(&psAlarmDispatchIns[nChMax], &psAlarmDispatchIns[0], sizeof(SGuiAlarmDispatch));  //cw_md
	
	for(i=0; i<nRealRow; i++)
	{
		int chn = nCurPage*6 + i;   //cw_md
		tMDPage0[i].pComboDelay->SetCurSel(pTabPara[chn].nDelay);
		tMDPage0[i].pChkEnable->SetValue(pTabPara[chn].bEnable);
	}
	
	{
		int nCh = 0;
		for(nCh = 0; nCh < nChMax; nCh++)
		{
			GetScrStrSetCurSelDo(
				(void*)&pSCHGridPage1[0],
				GSR_CONFIG_ALARM_MD_SCH_SCH_SCH,
				EM_GSR_CTRL_SCH,
				nCh/*pComboChnPage1->GetCurSel()*/
			);
			
			uint mask = 0;
			
			for(int i=0; i<7; i++)
			{
				pSCHGridPage1[i]->GetMask(&mask);
				BIZ_DO_DBG("GET msk idx %d mask %x\n", i, mask);
				SetSch(nCh, i, &mask);
			}
		}
		uint mask[7] = {0};
		int curIndex = pComboChnPage1->GetCurSel();
		GetSch(curIndex, mask);
		for(int i=0; i<7; i++)
		{
			pSCHGridPage1[i]->SetMask(&mask[i]);
		}

	}
}

VD_BOOL CPageMDCfg::UpdateData( UDM mode )
{
	if (UDM_OPEN == mode)
	{
		LoadPara();
	} 
	else if (UDM_CLOSED == mode) 
	{
        ((CPageAlarmCfgFrameWork *)(this->GetParent()))->ReflushItemName();
    }
	
	return TRUE;
}

void CPageMDCfg::GetPara(uchar nCh, SGuiAlarmDispatch *pPara)
{
	int nChMax = GetVideoMainNum();

	if(nCh>=nChMax)
	{
		nCh = nChMax;
	}
	
	if(pPara&&psAlarmDispatchIns&&nCh<=nChMax)
	{
		memcpy( pPara, &psAlarmDispatchIns[nCh], sizeof(SBizAlarmDispatch) );
    }

	BIZ_DO_DBG("CH %d Sound %d Mail %d\n", nCh, pPara->nFlagBuzz, pPara->nFlagEmail);	
	return;
}

void CPageMDCfg::SavePara(uchar nCh, SGuiAlarmDispatch *pPara)
{
	int nChMax = GetVideoMainNum();
	
	if(nCh>=nChMax)
	{
		nCh = nChMax;
	}

	BIZ_DO_DBG("CH %d Sound %d Mail %d linktype %d linkidx %d\n", 
		nCh, pPara->nFlagBuzz, pPara->nFlagEmail,
		pPara->sAlarmPtz[0].nALaPtzType, pPara->sAlarmPtz[0].nId
	);

	//printf("CH %d Sound %d Mail %d ***\n", nCh, pPara->nFlagBuzz, pPara->nFlagEmail);
	
	if(pPara&&psAlarmDispatchIns&&nCh<=nChMax)
		memcpy( &psAlarmDispatchIns[nCh], pPara, sizeof(SBizAlarmDispatch) );
		
	return;
}

void CPageMDCfg::SaveDealPara2Cfg(uchar nRealChNum)
{
	if(!psAlarmDispatchIns) return;
	
	int nChMax = GetVideoMainNum();
	
	if(nChMax>nRealChNum)
		nChMax = nRealChNum;
	
	for(int i=0; i<nChMax; i++)
	{
		BIZ_DO_DBG("CH %d Sound %d Mail %d\n", i, psAlarmDispatchIns[i].nFlagBuzz, psAlarmDispatchIns[i].nFlagEmail);
		bizData_SaveVMOTIONAlarmDeal(i, &psAlarmDispatchIns[i]);
	}
}

void CPageMDCfg::SetSch( uchar nCh, uchar idx, uint* psch)
{
	sch[nCh][idx] = *psch;
	//memcpy(sch[curID][nCh][idx], sch, sizeof(uint));
}

void CPageMDCfg::GetSch( uchar nCh, uint* psch)
{
	 //*sch = sch[curID][nCh][idx];
	memcpy(psch, sch[nCh], sizeof(uint)*7);
}

VD_BOOL CPageMDCfg::MsgProc( uint msg, uint wpa, uint lpa )
{
	int px,py;
	int temp;
	uchar nCh;
	CGridCtrl** pGrid;

	int curID = m_nCurID;
	
	switch(msg)
	{
	case XM_MOUSEWHEEL:   //cw_scrollbar
		{
			int px = VD_HIWORD(lpa);
			int py = VD_LOWORD(lpa);
			CItem* pItem = GetItemAt(px, py);
			if(pItem == NULL)
			{
				if(GetMaxChnNum()>TabSzMD[0].nRow)
				{		
					if (curID == 0)
					{
						pScrollbar->MsgProc(msg, wpa, lpa);
						break;
					}
				}
			}
			break;
		}
	case XM_LBUTTONUP:
		{
			nCh = GetCurChn();
			pGrid = GetCurGrid();
			
			uint msk = 0;
			
			for(int i=0; i<7; i++)
			{
				pGrid[i]->GetMask(&msk);
				SetSch(nCh, i, &msk);
			}

			/*
			printf("SET %x %x %x %x %x %x %x\n", 
				sch[0][0],
				sch[0][1],
				sch[0][2],
				sch[0][3],
				sch[0][4],
				sch[0][5],
				sch[0][6]
			);*/
		}
		break;
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
		break;
	}
	
	return 	CPageFrame::MsgProc(msg, wpa, lpa);
}

uchar CPageMDCfg::GetCurChn()
{
	uchar nCh = 0;
	
	nCh = pComboChnPage1->GetCurSel();
	/*
	switch(curID)
	{
		case 0:
		{
			nCh = pComboBox0[0]->GetCurSel();
		} break;
		case 1:
		{
			nCh = pComboBox1[0]->GetCurSel();
		} break;
		case 2:
		{
			nCh = pComboBox2[0]->GetCurSel();
		} break;
		default:
			break;
	}
	*/
	
	return nCh;
}

CGridCtrl** CPageMDCfg::GetCurGrid()
{	
	return pSCHGridPage1;
	/*
	switch(curID)
	{
		case 0:
		{
			return pGridSCH0;
		} break;
		case 1:
		{
			return pGridSCH1;
		} break;
		case 2:
		{
			return pGridSCH2;
		} break;
		default:
			break;
	}

	return NULL; 
	*/
}

void CPageMDCfg::OnComboBoxChn()
{
	int nChn = pComboChnPage1->GetCurSel();
	
	for (int i=0; i<7; i++)
	{
		pSCHGridPage1[i]->SetMask(&sch[nChn][i]);
	}	
}

void CPageMDCfg::OnClkCopy()
{
	printf("OnClkCopy \n");
	
	uint schIns[7]={0};
	
	uchar nCh = pComboChnPage1->GetCurSel();
	uchar nToCh = pComboCopy2ChnPage1->GetCurSel();
	
	GetSch(nCh, schIns);
	
	if(nToCh>=GetVideoMainNum())
	{
		for(int j=0; j<GetVideoMainNum(); j++)
		{
			if(j!=nCh)
			{
				for(int i=0; i<7; i++)
				{
					SetSch(j, i, &schIns[i]);
				}
			}
		}
	}
	else
	{
		for(int i=0; i<7; i++)
		{
			SetSch(nToCh, i, &schIns[i]);
		}
	}
}

void CPageMDCfg::SetInfo(char* szInfo)
{
	pIconInfo->Show(TRUE);
	
	pInfoBar->SetText(szInfo);
}

void CPageMDCfg::ClearInfo()
{
	pIconInfo->Show(FALSE);
	
	pInfoBar->SetText("");
}

