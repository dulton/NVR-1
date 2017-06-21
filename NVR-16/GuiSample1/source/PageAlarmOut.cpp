#include "GUI/Pages/PageAlarmOut.h"
#include "GUI/Pages/BizData.h"

static std::vector<CItem*>	items[ALARMOUT_SUBPAGES];

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static VD_BITMAP * pBmp_tab_normal;
static VD_BITMAP * pBmp_tab_push;
static VD_BITMAP * pBmp_tab_select;

static VD_BITMAP * pBmp_infoicon;

static const char* szAlarmDelayTime[7] = {
	"5",
	"10",
	"20",
	"30",
	"60",
	"120",
	//"&CfgPtn.Always",
};

static int GetMaxChnNum() 
{ 
    return GetVideoMainNum();
}

CPageAlarmOut::CPageAlarmOut( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPageFrame(pRect, psz, icon, pParent)
, m_nCurID(0)
{
	nChMax = GetMaxChnNum();
	
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
	const char* szSubPgName[ALARMOUT_COMMBUTTON] = {
		"&CfgPtn.AlarmOut",
		"&CfgPtn.Schedule",
		"&CfgPtn.Buzzer",
		"&CfgPtn.Default",
		"&CfgPtn.apply",
		"&CfgPtn.Exit",
	};
	
	int szSubPgNameLen[ALARMOUT_COMMBUTTON] = {
		TEXT_WIDTH*4,//"&CfgPtn.AlarmOut",
		TEXT_WIDTH*4,//"&CfgPtn.Schedule",	
		TEXT_WIDTH*4,//"&CfgPtn.Buzzer",
		TEXT_WIDTH*3,//"&CfgPtn.Default",
		TEXT_WIDTH*2,//"&CfgPtn.apply",
		TEXT_WIDTH*2//"&CfgPtn.Exit",
	};


	pTabFrame = CreateTableBox(CRect(17, 55+pBmp_tab_normal->height, m_Rect.Width()-17, m_Rect.Height()-45),this,1,1);
	pTabFrame->SetFrameColor(VD_RGB(56,108,148));


	CRect rtSub(26, 55, m_Rect.Width()-20, 55+pBmp_tab_normal->height);
	int i;
	for (i = 0; i<ALARMOUT_SUBPAGES; i++)
	{
#ifdef DISABLE_ALARM_SCH
		if(i == 1) 
		{
		    continue;
		}
#endif

		if(!GetAlarmOutNum() && (i == 0 || i == 1)) 
		{
		    m_nCurID = 2; continue;
		} //yzw add
		
		int btWidth = szSubPgNameLen[i]+10;
		rtSub.right = rtSub.left + btWidth;
		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageAlarmOut::OnClickSubPage, NULL, buttonNormalBmp, 1);
		pButton[i]->SetBitmap(pBmp_tab_normal, pBmp_tab_select, pBmp_tab_push, pBmp_tab_push);
		rtSub.left = rtSub.right;
	}

	int btwidth = 85;
	int btspace = 16;
	rtSub.top = m_Rect.Height()-38;
	rtSub.bottom = rtSub.top + pBmp_button_normal->height;
	rtSub.left = m_Rect.Width() - 20 - btwidth*(ALARMOUT_COMMBUTTON-ALARMOUT_SUBPAGES) - btspace*(ALARMOUT_COMMBUTTON-ALARMOUT_SUBPAGES-1);
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
	
	for (i=ALARMOUT_SUBPAGES; i<ALARMOUT_COMMBUTTON; i++)
	{
		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageAlarmOut::OnClickSubPage, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);

		rtSub.left = rtSub.right + btspace;
		rtSub.right = rtSub.left + btwidth;
	}
	
	InitPage0();
	
	InitPage1();
 	
	InitPage2();

	if(!GetAlarmOutNum())
	{	
		ShowSubPage(2,TRUE);
		pButton[2]->Enable(FALSE);
	}
	else
	{
		ShowSubPage(0,TRUE);
		pButton[0]->Enable(FALSE);
	}	
}

CPageAlarmOut::~CPageAlarmOut()
{

}

void CPageAlarmOut::SwitchPage( int subID )
{
	if (subID<0 || subID>=ALARMOUT_SUBPAGES )
	{
		return;
	}

	if (subID == m_nCurID)
	{
		return;
	}

	//隐藏当前子页面，显示新的子页面
	ShowSubPage(m_nCurID, FALSE);
	pButton[m_nCurID]->Enable(TRUE);
	m_nCurID = subID;
	ShowSubPage(m_nCurID, TRUE);
	pButton[subID]->Enable(FALSE);
}

void CPageAlarmOut::ShowSubPage( int subID, BOOL bShow )
{
	if (subID<0 || subID>=ALARMOUT_SUBPAGES )
	{
		return;
	}

	int count = items[subID].size();
	for (int i=0; i<count; i++)
	{
		if(items[subID][i])
		{			
			items[subID][i]->Show(bShow);
		}
	}
}

void CPageAlarmOut::OnClickSubPage()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < ALARMOUT_COMMBUTTON; i++)
	{
		if (pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{	
		//printf("find the focus button %d\n", i);
		if (i < ALARMOUT_SUBPAGES)
		{
			SwitchPage(i);	
		}
		else
		{
			if(i==ALARMOUT_SUBPAGES+1) // apply
			{
				WriteLogs();
				
				switch(m_nCurID)
				{
					case 0:
					{
						if(pChkChnAllPage0->GetValue())
						{
							for(i=0;i<sizeof(tAlarmOutPage0)/sizeof(tAlarmOutPage0[0]);i++)
							{
								tAlarmOutPage0[i].pComboDelay->SetCurSel(pComboDelayAllPage0->GetCurSel());
								
								#if 1//csp modify 20140525
								tAlarmOutPage0[i].pComboType->SetCurSel(pComboTypeAllPage0->GetCurSel());
								#else
								SaveScrStr2Cfg(
									tAlarmOutPage0[i].pComboDelay,
									GSR_CONFIG_ALARM_ALARMOUT_DELAYTIME, 
									EM_GSR_CTRL_COMB, 
									i
								);
								#endif
							}
						}
						else
						{
							#if 0//csp modify 20140525
							for(i=0;i<sizeof(tAlarmOutPage0)/sizeof(tAlarmOutPage0[0]);i++)
							{
								SaveScrStr2Cfg(
									tAlarmOutPage0[i].pComboDelay,
									GSR_CONFIG_ALARM_ALARMOUT_DELAYTIME, 
									EM_GSR_CTRL_COMB, 
									i
								);
							}
							#endif
						}
						
						int ret = 0;
						for(i=0;i<sizeof(tAlarmOutPage0)/sizeof(tAlarmOutPage0[0]);i++)
						{
							#if 1//csp modify 20140525
							SBizParaTarget sParaTgtIns;
							SBizAlarmOutPara bizSysPara;
							sParaTgtIns.emBizParaType = EM_BIZ_ALARMOUTPARA;
							sParaTgtIns.nChn = i;
							int rtn = BizGetPara(&sParaTgtIns, &bizSysPara);
							if(0 == rtn)
							{
								int val = tAlarmOutPage0[i].pComboDelay->GetCurSel();
								s32 nValue = 0;
								if( 0==BizConfigGetParaListValue(
									EM_BIZ_CFG_PARALIST_DELAY, 
									val, 
									&nValue )
								)
								{
									BIZ_DT_DBG("ALARMOUT_DELAYTIME set %d %d\n", val, nValue);
									bizSysPara.nDelay = nValue;
									bizSysPara.nType = tAlarmOutPage0[i].pComboType->GetCurSel();
									char szStr[EDIT_MAX] = {0};
									tAlarmOutPage0[i].pEditName->GetText(szStr, sizeof(szStr));
									strncpy(bizSysPara.name, szStr, MAX_ALANAME_LEN);
									BizSetPara(&sParaTgtIns, &bizSysPara);
								}
								else
								{
									ret = -1;
								}
							}
							else
							{
								BIZ_DATA_DBG("ALARMOUT set err\n");
								ret = -1;
							}
							#else
							ret = SaveScrStr2Cfg(
								tAlarmOutPage0[i].pEditName,
								GSR_CONFIG_ALARM_ALARMOUT_NAME, 
								EM_GSR_CTRL_EDIT, 
								i
							);
							#endif
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
					case 1:
					{
						int ret = 0;
						for(int i=0; i<sizeof(sch)/sizeof(sch[0]); i++)
						{
							ret = SaveScrStr2Cfg(
								sch[i],
								GSR_CONFIG_ALARM_ALARMOUT_SCH_SCH, 
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
					case 2:
					{
						int ret = 0;
						ret = SaveScrStr2Cfg(
							pChkChnSoundPage2,
							GSR_CONFIG_ALARM_ALARMOUT_SOUNDALARM_ALARM, 
							EM_GSR_CTRL_CHK, 
							0
						);
						
						SaveScrStr2Cfg(
							pComboDelayPage2,
							GSR_CONFIG_ALARM_ALARMOUT_SOUNDALARM_DELAY, 
							EM_GSR_CTRL_COMB, 
							0
						);

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
			else if(i==ALARMOUT_SUBPAGES)//default
			{
				//default
				char tmpbuf[32] = {0};
				
				switch(m_nCurID)
				{
					case 0:
					{
						int ret = 0;
						debug_trace_err("CPageAlarmOut 330");
						
						//pChkChnAllPage0->SetValue(GetAlarmOutAllChnCheckCur());
						pChkChnAllPage0->SetValue(0);//csp modify 20140525
						
						for(int i=0;i<sizeof(tAlarmOutPage0)/sizeof(tAlarmOutPage0[0]);i++)
						{
							tAlarmOutPage0[i].pComboDelay->SetCurSel(GetAlarmOutDelayTimeCurDefault(i));
							
							memset(tmpbuf,0,sizeof(tmpbuf));
							ret = GetAlarmOutNameCurDefault(i, tmpbuf);
							
							tAlarmOutPage0[i].pEditName->SetText(tmpbuf);
							
							//csp modify 20140525
							int rtn = 0;
							SBizParaTarget sParaTgtIns;
							SBizAlarmOutPara sCfgIns;
							memset(&sCfgIns, 0, sizeof(sCfgIns));
							sParaTgtIns.emBizParaType = EM_BIZ_ALARMOUTPARA;
							sParaTgtIns.nChn = i;
							rtn = BizGetDefaultPara(&sParaTgtIns, &sCfgIns);
							if(0 == rtn)
							{
								tAlarmOutPage0[i].pComboType->SetCurSel(sCfgIns.nType?1:0);
								if(i == 0)
								{
									pComboTypeAllPage0->SetCurSel(sCfgIns.nType?1:0);
									pComboDelayAllPage0->SetCurSel(0);
								}
							}
						}
						
						if(ret == 0)
						{
							SetInfo("&CfgPtn.LoadDefaultSuccess");
						}
						else
						{
							SetInfo("&CfgPtn.LoadDefaultFail");
						}
					} break;
					case 1:
					{
						int ret = 0;
						debug_trace_err("CPageAlarmOut 331");
						uint mask[7];
						
						int nCh = 
							pComboChnPage1->GetCurSel();
						
						ret = GetAlarmOutSCHCurDefault(nCh, mask);
						
						for(int i=0;i<7;i++)
						{
							pSCHGridPage1[i]->SetMask(&mask[i]);
						}

						for(int i=0; i<7; i++)
						{
							//pSCHGridPage1[i]->GetMask(&mask);
							BIZ_DO_DBG("GET msk idx %d mask %x\n", i, mask);
							SetSch(nCh, i, &mask[i]);
						}

						if(ret == 0)
						{
							SetInfo("&CfgPtn.LoadDefaultSuccess");
						}
						else
						{
							SetInfo("&CfgPtn.LoadDefaultFail");
						}
					
					} break;
					case 2:
					{
						debug_trace_err("CPageAlarmOut 332");
						int ret = GetAlarmOutSoundAlarmCheckDefault();
						pChkChnSoundPage2->SetValue(ret);
						pComboDelayPage2->SetCurSel(GetAlarmOutSoundAlarmDelayTimeCurDefault());

						if(ret >= 0)
						{
							SetInfo("&CfgPtn.LoadDefaultSuccess");
						}
						else
						{
							SetInfo("&CfgPtn.LoadDefaultFail");
						}
					} break;
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

void CPageAlarmOut::InitPage0()
{
	int i = 0;
	CRect rtTab(25, 55+pBmp_tab_normal->height+10, m_Rect.Width()-25, 55+pBmp_tab_normal->height+10 + CTRL_HEIGHT*7);
	#if 1//csp modify 20140525
	pTabPage0[0] = CreateTableBox(rtTab, this, 4, 7);
	pTabPage0[0]->SetColWidth(0, 54+10);
	pTabPage0[0]->SetColWidth(1, 280);
	pTabPage0[0]->SetColWidth(2, 100);
	#else
	pTabPage0[0] = CreateTableBox(rtTab, this, 3, 7);
	pTabPage0[0]->SetColWidth(0, 54+10);
	pTabPage0[0]->SetColWidth(1, 330);
	#endif
	
	items[0].push_back(pTabPage0[0]);
	
	#if 1//csp modify 20140525
	const char* szTitle[4] = 
	#else
	const char* szTitle[3] = 
	#endif
	{
		"&CfgPtn.Channel",
		"&CfgPtn.RelayName",
		"&CfgPtn.HoldingTime",
		"&CfgPtn.Type",//csp modify 20140525
	};
	
	int nRealNum;
	SValue psValueList[CBX_MAX];
	
	#if 1//csp modify 20140525
	for (i=0; i<4; i++)
	#else
	for (i=0; i<3; i++)
	#endif
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
	rtTab.bottom = rtTab.top + CTRL_HEIGHT;
	#if 1//csp modify 20140525
	pTabPage0[1] = CreateTableBox(rtTab, this, 4, 1);
	pTabPage0[1]->SetColWidth(0, 54+10);
	pTabPage0[1]->SetColWidth(1, 280);
	pTabPage0[1]->SetColWidth(2, 100);
	#else
	pTabPage0[1] = CreateTableBox(rtTab, this, 3, 1);
	pTabPage0[1]->SetColWidth(0, 54+10);
	pTabPage0[1]->SetColWidth(1, 330);
	#endif
	
	items[0].push_back(pTabPage0[1]);
	
	CRect rt;
	pTabPage0[1]->GetTableRect(0, 0, &rt);
	pChkChnAllPage0 = CreateCheckBox(CRect(rtTab.left+rt.left+5, rtTab.top+rt.top+1, rtTab.left+rt.left+5+CTRL_HEIGHT, rtTab.top+rt.bottom), this);
	
	//pChkChnAllPage0->SetValue(GetAlarmOutAllChnCheckCur());
	
	items[0].push_back(pChkChnAllPage0);
	
	pTabPage0[1]->GetTableRect(2, 0, &rt);
	pComboDelayAllPage0 = CreateComboBox(CRect(rtTab.left+rt.left+1, rtTab.top+rt.top, rtTab.left+rt.right-1, rtTab.top+rt.bottom), this, NULL);
	
	GetAlarmOutDelayTimeList(psValueList, &nRealNum, CBX_MAX);
	
	for (i=0; i< nRealNum; i++)
	{
		pComboDelayAllPage0->AddString(psValueList[i].strDisplay);
	}
	
	//pComboDelayAllPage0->SetCurSel(0); // 0 all
	
	items[0].push_back(pComboDelayAllPage0);
	
	//csp modify 20140525
	pTabPage0[1]->GetTableRect(3, 0, &rt);
	pComboTypeAllPage0 = CreateComboBox(CRect(rtTab.left+rt.left+1, rtTab.top+rt.top, rtTab.left+rt.right-1, rtTab.top+rt.bottom), this, NULL);
	pComboTypeAllPage0->AddString("&CfgPtn.NormalClose");
	pComboTypeAllPage0->AddString("&CfgPtn.NormalOpen");
	//pComboTypeAllPage0->SetCurSel(0);
	items[0].push_back(pComboTypeAllPage0);
	
	//debug_trace_err("CPageAlarmOut");
	
	rtTab.left = 25;
	rtTab.top = 55+pBmp_tab_normal->height+10;
	for (i = 0; i< sizeof(tAlarmOutPage0)/sizeof(tAlarmOutPage0[0]); i++)
	{
		char tmpbuf[32] = {0};
		sprintf(tmpbuf, "%d", i+1);
		CRect rttmp;
		pTabPage0[0]->GetTableRect(0, i+1, &rttmp);
		//printf("tab: %d %d %d %d\n", rtTab.left+rttmp.left, rtTab.top+rttmp.top, rtTab.left+rttmp.right, rtTab.top+rttmp.bottom);
		tAlarmOutPage0[i].pChnIndex = CreateStatic(CRect(rtTab.left+rttmp.left+2, rtTab.top+rttmp.top, rtTab.left+rttmp.right, rtTab.top+rttmp.bottom), this, tmpbuf);
		tAlarmOutPage0[i].pChnIndex->SetTextAlign(VD_TA_LEFT);
		
		pTabPage0[0]->GetTableRect(1, i+1, &rttmp);
		tAlarmOutPage0[i].pEditName = CreateEdit(CRect(rtTab.left+rttmp.left+1, rtTab.top+rttmp.top+1, rtTab.left+rttmp.right-1, rtTab.top+rttmp.bottom-1), 
			this, 12);
		
		//memset(tmpbuf,0,sizeof(tmpbuf));
		//GetAlarmOutNameCur(i, tmpbuf);
		
		//sprintf(tmpbuf, "Alarm Out %d", i+1);
		//tAlarmOutPage0[i].pEditName->SetText(tmpbuf);
		
		pTabPage0[0]->GetTableRect(2, i+1, &rttmp);
		tAlarmOutPage0[i].pComboDelay = CreateComboBox(CRect(rtTab.left+rttmp.left+1, rtTab.top+rttmp.top+1, rtTab.left+rttmp.right-1, rtTab.top+rttmp.bottom-1), this, NULL);
		for (int kk=0; kk < nRealNum; kk++)
		{
			tAlarmOutPage0[i].pComboDelay->AddString(psValueList[kk].strDisplay);
			//tAlarmOutPage0[i].pComboDelay->SetCurSel(GetAlarmOutDelayTimeCur(kk));
		}
		
		//csp modify 20140525
		pTabPage0[0]->GetTableRect(3, i+1, &rttmp);
		tAlarmOutPage0[i].pComboType = CreateComboBox(CRect(rtTab.left+rttmp.left+1, rtTab.top+rttmp.top+1, rtTab.left+rttmp.right-1, rtTab.top+rttmp.bottom-1), this, NULL);
		tAlarmOutPage0[i].pComboType->AddString("&CfgPtn.NormalClose");
		tAlarmOutPage0[i].pComboType->AddString("&CfgPtn.NormalOpen");
		//tAlarmOutPage0[i].pComboType->SetCurSel(0);
		
		items[0].push_back(tAlarmOutPage0[i].pChnIndex);
		items[0].push_back(tAlarmOutPage0[i].pEditName);
		items[0].push_back(tAlarmOutPage0[i].pComboDelay);
		
		//csp modify 20140525
		items[0].push_back(tAlarmOutPage0[i].pComboType);
	}	
	
	ShowSubPage(0, FALSE);
}

void CPageAlarmOut::InitPage1()
{
	//debug_trace_err("CPageAlarmOut");
	int i = 0;
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
	pComboChnPage1 = CreateComboBox(rt, this, NULL, NULL,
		(CTRLPROC)&CPageAlarmOut::OnComboBoxChn );
	
	int nRealNum;
	SValue psValueList[CBX_MAX];

	GetAlarmOutSCHChnList(psValueList, &nRealNum, CBX_MAX);
	
	for (i=0; i<nRealNum; i++)
	{
		pComboChnPage1->AddString(psValueList[i].strDisplay);
	}
	pComboChnPage1->SetCurSel(0);
	
	//uint mask[7];
	
	//GetAlarmOutSCHCur(0, mask);
	
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
	
	const char* szWeekDay[7] = {
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

		//pSCHGridPage1[i]->SetMask(&mask[i]);
		
		items[1].push_back(pSCHGridPage1[i]);

		CRect rtWeek;
		pTabSCHPage1->GetTableRect(0, i+1, &rtWeek);
		CStatic* pWeekDay = CreateStatic(CRect(rt.left+rtWeek.left, rt.top+rtWeek.top, rt.left+rtWeek.right,rt.top+rtWeek.bottom),
			this, szWeekDay[i]);
		items[1].push_back(pWeekDay);
	}

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

	GetAlarmOutSCHApplyToList(psValueList, &nRealNum, CBX_MAX);
	for (i=0; i<nRealNum; i++)
	{
		pComboCopy2ChnPage1->AddString(psValueList[i].strDisplay);
	}
	pComboCopy2ChnPage1->SetCurSel(GetAlarmOutSCHApplyToCur());
	
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
	pBtCopyPage1 = CreateButton(rt, this, "&CfgPtn.Copy", (CTRLPROC)&CPageAlarmOut::OnClkCopy, NULL, buttonNormalBmp);
	pBtCopyPage1->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);
	items[1].push_back(pBtCopyPage1);	

	ShowSubPage(1, FALSE);
}

void CPageAlarmOut::InitPage2()
{	
	//debug_trace_err("CPageAlarmOut 222");
	
	int i= 0;
	CRect rtTab(25, 55+pBmp_tab_normal->height+10, m_Rect.Width()-25, 55+pBmp_tab_normal->height+10 + CTRL_HEIGHT*9);
	pTabPage2 = CreateTableBox(rtTab, this, 2, 9);
	items[2].push_back(pTabPage2);
	
	const char* szLeft[2] = {
		"&CfgPtn.Buzzer",
		"&CfgPtn.HoldingTime",		
	};
	
	for (i= 0; i<2; i++)
	{
		CRect rt;
		pTabPage2->GetTableRect(0, i, &rt);
		CStatic* pText= CreateStatic(CRect(rtTab.left+rt.left, rtTab.top+rt.top, rtTab.left+rt.right, rtTab.top+rt.bottom), this, szLeft[i]);
		pText->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
		items[2].push_back(pText);
	}

	CRect rt;
	pTabPage2->GetTableRect(1, 0, &rt);
	pChkChnSoundPage2 = CreateCheckBox(CRect(rtTab.left+rt.left+5, rtTab.top+rt.top+2, rtTab.left+rt.left+5+CTRL_HEIGHT, rtTab.top+rt.bottom), this);
	//pChkChnSoundPage2->SetValue(GetAlarmOutSoundAlarmCheck());
	items[2].push_back(pChkChnSoundPage2);

	pTabPage2->GetTableRect(1, 1, &rt);
	pComboDelayPage2 = CreateComboBox(CRect(rtTab.left+rt.left+1, rtTab.top+rt.top+1, rtTab.left+rt.right-1, rtTab.top+rt.bottom-1), this, NULL);

	int nRealNum;
	SValue psValueList[CBX_MAX];
	
	GetAlarmOutSoundAlarmDelayTimeList(psValueList, &nRealNum,CBX_MAX);
	for (i=0; i<nRealNum; i++)
	{
		pComboDelayPage2->AddString(psValueList[i].strDisplay);
	}

	//pComboDelayPage2->SetCurSel(GetAlarmOutSoundAlarmDelayTimeCur());
	//pComboDelayPage2->SetCurSel(0);
	
	items[2].push_back(pComboDelayPage2);	

	ShowSubPage(2, FALSE);
}

VD_BOOL CPageAlarmOut::UpdateData( UDM mode )
{
	char tmpbuf[32] = {0};
	
	if(UDM_OPEN == mode)
	{
		//switch(m_nCurID)
		{
			//case 0:
			{
				//debug_trace_err("CPageAlarmOut 330");
				
				//pChkChnAllPage0->SetValue(GetAlarmOutAllChnCheckCur());
				
				for(int i=0;i<sizeof(tAlarmOutPage0)/sizeof(tAlarmOutPage0[0]);i++)
				{
					tAlarmOutPage0[i].pComboDelay->SetCurSel(GetAlarmOutDelayTimeCur(i));
					
					memset(tmpbuf,0,sizeof(tmpbuf));
					GetAlarmOutNameCur(i, tmpbuf);
					
					tAlarmOutPage0[i].pEditName->SetText(tmpbuf);
					
					//csp modify 20140525
					int rtn = 0;
					SBizParaTarget sParaTgtIns;
					SBizAlarmOutPara sCfgIns;
					memset(&sCfgIns, 0, sizeof(sCfgIns));
					sParaTgtIns.emBizParaType = EM_BIZ_ALARMOUTPARA;
					sParaTgtIns.nChn = i;
					rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
					if(0 == rtn)
					{
						tAlarmOutPage0[i].pComboType->SetCurSel(sCfgIns.nType?1:0);
						if(i == 0)
						{
							pComboTypeAllPage0->SetCurSel(sCfgIns.nType?1:0);
						}
					}
				}
				pComboDelayAllPage0->SetCurSel(0);
			} //break;
			//case 1:
			#ifndef DISABLE_ALARM_SCH
			{
				//debug_trace_err("CPageAlarmOut 331");
				
				uint mask[7];
				
				int nCh = pComboChnPage1->GetCurSel();
				
				GetAlarmOutSCHCur(nCh, mask);
				
				for(int i=0;i<7;i++)
				{
					pSCHGridPage1[i]->SetMask(&mask[i]);
				}

				for(int i=0; i<7; i++)
				{
					//pSCHGridPage1[i]->GetMask(&mask);
					//BIZ_DO_DBG("GET msk idx %d mask %x\n", i, mask);
					SetSch(nCh, i, &mask[i]);
				}
			
			} //break;
			#endif
			//case 2:
			{
				//debug_trace_err("CPageAlarmOut 332");
				pChkChnSoundPage2->SetValue(GetAlarmOutSoundAlarmCheck());
				pComboDelayPage2->SetCurSel(GetAlarmOutSoundAlarmDelayTimeCur());
			} //break;
		}
	}
	else if (UDM_CLOSED == mode) 
	{
        ((CPageAlarmCfgFrameWork *)(this->GetParent()))->ReflushItemName();
    }
	
	return TRUE;
}

void CPageAlarmOut::WriteLogs()
{
	if(m_nCurID == 1)
	{
		BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_ALARMOUT_SCHE);
	}
	
	if(pChkChnSoundPage2->GetValue() != GetAlarmOutSoundAlarmCheck())
	{
		if(pChkChnSoundPage2->GetValue())
		{
			BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_ENABLE_SOUND_ALARM);
		}
		else
		{
			BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_DISABLE_SOUND_ALARM);
		}	
	}
}
		

void CPageAlarmOut::SetSch( uchar nCh, uchar idx, uint* psch)
{
	sch[nCh][idx] = *psch;
	//memcpy(sch[curID][nCh][idx], sch, sizeof(uint));
}

void CPageAlarmOut::GetSch( uchar nCh, uint* psch)
{
	//*sch = sch[curID][nCh][idx];
	memcpy(psch, sch[nCh], sizeof(uint)*7);
}


VD_BOOL CPageAlarmOut::MsgProc( uint msg, uint wpa, uint lpa )
{
	int px,py;
	int temp;
	uchar nCh;
	CGridCtrl** pGrid;

	int curID = m_nCurID;
	
	switch(msg)
    {
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
    			);
    			*/
    		} break;
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
    		} break;
    	default:
    		break;
    }

	return 	CPageFrame::MsgProc(msg, wpa, lpa);
}

uchar CPageAlarmOut::GetCurChn()
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

CGridCtrl** CPageAlarmOut::GetCurGrid()
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


void CPageAlarmOut::OnComboBoxChn()
{
	int nChn = pComboChnPage1->GetCurSel();

	for (int i=0; i<7; i++)
	{
		pSCHGridPage1[i]->SetMask(&sch[nChn][i]);
	}	
}

void CPageAlarmOut::OnClkCopy()
{
	printf("OnClkCopy \n");
	
	uint schIns[7]={0};
	
	uchar nCh = pComboChnPage1->GetCurSel();
	uchar nToCh = pComboCopy2ChnPage1->GetCurSel();

	GetSch(nCh, schIns);

	int nChMax = GetMaxChnNum();
	if(nToCh>=nChMax)
	{
		for(int j=0; j<nChMax; j++)
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

void CPageAlarmOut::SetInfo(char* szInfo)
{
	pIconInfo->Show(TRUE);
		
	pInfoBar->SetText(szInfo);
}

void CPageAlarmOut::ClearInfo()
{
	pIconInfo->Show(FALSE);
	
	pInfoBar->SetText("");
}



