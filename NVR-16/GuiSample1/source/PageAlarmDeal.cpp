#include "GUI/Pages/PageAlarmDeal.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageSensorAlarm.h"
#include "GUI/Pages/PageIPCameraExtSensorAlarm.h"
#include "GUI/Pages/PageMDCfg.h"
#include "GUI/Pages/PageVideoLoss.h"

//#define NO_BIGVIDEO_ALARM	// 20111011 lsp 关闭大画面报警
//yaogang modify 20141010
/*
硬盘报警配置页面复用该页面
*/

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static VD_BITMAP * pBmp_tab_normal;
static VD_BITMAP * pBmp_tab_push;
static VD_BITMAP * pBmp_tab_select;

/*
extern void bizData_SaveIPCExtSensorAlarmPara(int ch, SIPCExt_SensorAlarmPara* pGAlarm);
extern void bizData_GetIPCExtSensorAlarmDefault(int ch, SIPCExt_SensorAlarmPara* pGAlarm);
extern void bizData_GetIPCExtSensorAlarmPara(int ch, SIPCExt_SensorAlarmPara* pGAlarm);

extern int bizData_GetSENSORAlarmDealDefault(u8 nCh, SGuiAlarmDispatch* pGDispatch);
extern void bizData_SaveSENSORAlarmDeal(u8 nCh, SGuiAlarmDispatch* pDispatch);
extern void bizData_GetSENSORAlarmDeal(u8 nCh, SGuiAlarmDispatch* pDispatch);
*/
extern void bizData_SaveHDDAlarmPara(SHDDAlarmPara* pGAlarm);
extern void bizData_GetHDDAlarmDefault(SHDDAlarmPara* pGAlarm);
extern void bizData_GetHDDAlarmPara(SHDDAlarmPara* pGAlarm);

extern int		bizData_GetHDDAlarmDealDefault(SGuiAlarmDispatch* pGDispatch);
extern void	bizData_SaveHDDAlarmDeal(SGuiAlarmDispatch* pDispatch);
extern void	bizData_GetHDDAlarmDeal(SGuiAlarmDispatch* pDispatch);


static int GetMaxChnNum()
{
	return GetVideoMainNum();
}

static int GetLinkRows()
{
	return GetMaxChnNum()>LINKAGE_ROWS? LINKAGE_ROWS : GetMaxChnNum();
}

int bizData_GetPtzAdvancedPara(uchar nChn, SGuiPtzAdvancedPara* pGDispatch);

static u32 nCtrlID[4][8]={
	{
		GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP,
		GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND,
		GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO,
		GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER,
		GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL,
		GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_TRIGGER,
		GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE,
		GSR_CONFIG_ALARM_SENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX
	},
	{
		GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SNAP,
		GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_SOUND,
		GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_BIGVIDEO,
		GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_TIRGGER,
		GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_BASE_MAIL,
		GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_TRIGGER,
		GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_TYPE,
		GSR_CONFIG_ALARM_VIDEOLOST_ALARMDEAL_LINKAGE_INDEX
	},
	{
		GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SNAP,
		GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_SOUND,
		GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_BIGVIDEO,
		GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_TIRGGER,
		GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_BASE_MAIL,
		GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_TRIGGER,
		GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_TYPE,
		GSR_CONFIG_ALARM_MD_MOTIONDETECT_ALARMDEAL_LINKAGE_INDEX
	},
	//yaogang modify 20141010
	{
		GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_SNAP,
		GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_SOUND,
		GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_BIGVIDEO,
		GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_TIRGGER,
		GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_BASE_MAIL,
		GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_TRIGGER,
		GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_TYPE,
		GSR_CONFIG_ALARM_IPCEXTSENSOR_ALARMDEAL_ALARMDEAL_LINKAGE_INDEX
	},
};

CPageAlarmDeal::CPageAlarmDeal( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/, CPage * pParent /*= NULL*/, uchar nParentLabel)
:CPageFrame(pRect, psz, icon, pParent)
, m_nCurID(0)
, m_emDealType(ALARMDEAL_NULL)
, m_nDealChn(-1)
{
	SetMargin(0,0,0,0);
    	nChMax = GetMaxChnNum();
	
	this->nParentLabel = nParentLabel;
	this->pParent = pParent;

	for(int i=0; i<ALARMDEAL_SUBPAGES; i++)
	{
		items[i].clear();
	}
	
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
	char* szSubPgName[ALARMDEAL_COMMBUTTON] = {
		"&CfgPtn.Alarm",
		"&CfgPtn.ToRecord",	
		"&CfgPtn.ToPTZ",
		"&CfgPtn.OK",
		"&CfgPtn.Exit",
	};

	pTabFrame = CreateTableBox(CRect(17, 55+pBmp_tab_normal->height, m_Rect.Width()-17, m_Rect.Height()-45),this,1,1);
	pTabFrame->SetFrameColor(VD_RGB(56,108,148));

	//yaogang modify 20150324
	SBizDvrInfo DvrInfo;
	nNVROrDecoder = 1; //跃天: 1 nvr，2 轮巡解码器，3 切换解码器
	if (BizGetDvrInfo(&DvrInfo) == 0)
	{
		if (DvrInfo.nNVROrDecoder != 1)// == 2
		{
			nNVROrDecoder = DvrInfo.nNVROrDecoder;
		}
	}
	
	CRect rtSub(26, 55, m_Rect.Width()-20, 55+pBmp_tab_normal->height);
	int i;
	for (i = 0; i<ALARMDEAL_SUBPAGES; i++)
	{
		pButton[i] = NULL;
		if (nNVROrDecoder != 1 && i==1) // == 2 去除触发录像
		{
			continue;
		}
		
		int btWidth = strlen(szSubPgName[i])*TEXT_WIDTH/2+10;
		rtSub.right = rtSub.left + btWidth;
		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageAlarmDeal::OnClickSubPage, NULL, buttonNormalBmp, TRUE);
		pButton[i]->SetBitmap(pBmp_tab_normal, pBmp_tab_select, pBmp_tab_push, pBmp_tab_push);
		rtSub.left = rtSub.right;
	}
	int btwidth = 85;
	int btspace = 16;
	rtSub.top = m_Rect.Height()-38;
	rtSub.bottom = rtSub.top + pBmp_button_normal->height;
	rtSub.left = m_Rect.Width() - 20 - btwidth*(ALARMDEAL_COMMBUTTON - ALARMDEAL_SUBPAGES) - btspace*(ALARMDEAL_COMMBUTTON - ALARMDEAL_SUBPAGES - 1);
	rtSub.right = rtSub.left + btwidth;

	pInfoBar = CreateStatic(CRect(17, 
								rtSub.top,
								317,
								rtSub.top+22), 
								this, 
								"");
	//pInfoBar->SetBkColor(VD_RGB(8,66,189));
	
	for (i=ALARMDEAL_SUBPAGES; i<ALARMDEAL_COMMBUTTON; i++)
	{
		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageAlarmDeal::OnClickSubPage, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);

		rtSub.left = rtSub.right + btspace;
		rtSub.right = rtSub.left + btwidth;
	}
 	InitPage0();
	//if (this->nParentLabel != 4)
	{
	 	InitPage1();
	 	InitPage2();
	}
	
	ShowSubPage(0,TRUE);
	pButton[0]->Enable(FALSE);
	
	if (this->nParentLabel == 4)//hdd
	{
		
		pButton[1]->Show(FALSE);
		pButton[2]->Show(FALSE);
		nChMax = 1;//HDD ALARM
	}
	
	BIZ_DATA_DBG("CPageAlarmDeal");
}

CPageAlarmDeal::~CPageAlarmDeal()
{

}

void CPageAlarmDeal::SwitchPage( int subID )
{
	//printf("curID = %d, newID = %d \n\n",m_nCurID,subID);
	
	if (subID<0 || subID>=ALARMDEAL_SUBPAGES )
	{
		return;
	}

	if (subID == m_nCurID)
	{
		return;
	}

	//printf("curPage = %d \n",m_nCurID);
	//printf("Switch to page %d \n", subID);
	//printf("newPage = %d \n\n",subID);
	//隐藏当前子页面，显示新的子页面
	ShowSubPage(m_nCurID, FALSE);
	pButton[m_nCurID]->Enable(TRUE);
	m_nCurID = subID;
	ShowSubPage(m_nCurID, TRUE);
	pButton[m_nCurID]->Enable(FALSE);
}

void CPageAlarmDeal::ShowSubPage( int subID, BOOL bShow )
{
	if (subID<0 || subID>=ALARMDEAL_SUBPAGES )
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
			//printf("items[%d][%d] = %x, isShow = %d \n",subID,i,items[subID][i],items[subID][i]->GetFlag(IF_SHOWN));
			//printf("isShow = %d \n", ((CItem*)(0x891138))->GetFlag(IF_SHOWN));
		}
	}
}

void CPageAlarmDeal::OnClickSubPage()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < ALARMDEAL_COMMBUTTON; i++)
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
		if(i <ALARMDEAL_SUBPAGES)
		{
			SwitchPage(i);
			
			if(i == 2)
			{
				AdjustLinkageRows();
			}
		} 
		else
		{
			//ok| exit
			if(i==ALARMDEAL_SUBPAGES)
			{
				//触发报警
				for (i = 0; i<nAlarmOutChn; i++)
				{
					sAlarmDispatchIns.nAlarmOut[i] = 
						pChkTriggerAlarmPG0[i]->GetValue()? i:0xff;
				}
				//声音报警
				sAlarmDispatchIns.nFlagBuzz = 
						pChkSoundPG0->GetValue()? 1:0;
				
				//邮件报警
				sAlarmDispatchIns.nFlagEmail = 
						pChkMailPG0->GetValue()? 1:0;

				if (nParentLabel != 4)
				{
					//大画面报警
					#ifndef NO_BIGVIDEO_ALARM
					sAlarmDispatchIns.nZoomChn = 
						(pComboBigVideoPG0->GetCurSel() == GetVideoMainNum()) ? 0xff : pComboBigVideoPG0->GetCurSel();
					#endif
					
					//Page1 第二个选项卡，触发通道录像
					//nRealChNum = sizeof(pChkTriggerChnPG1)/sizeof(pChkTriggerChnPG1[0]);
					int nRealChNum = GetMaxChnNum();
					for (i = 0; i<nRealChNum; i++)
					{
						sAlarmDispatchIns.nRecordChn[i] = 
							pChkTriggerChnPG1[i]->GetValue()?i:0xff;
					}

					//联动报警
					//nRealChNum = sizeof(pComboTypePG2)/sizeof(pComboTypePG2[0]);
					s32 nVal =0;
					for (i = 0; i<nRealChNum; i++)
					{
						if( 0==BizConfigGetParaListValue(
								EM_BIZ_CFG_PARALIST_PTZLINKTYPE, 
								type[i],
								&nVal )
						)
						{
							sAlarmDispatchIns.sAlarmPtz[i].nALaPtzType = nVal;
							sAlarmDispatchIns.sAlarmPtz[i].nId = id[i];
		                    			sAlarmDispatchIns.sAlarmPtz[i].nChn = i;
						}
					}
				}
				else
				{
					//硬盘报警专用   启用
					sHDDAlarmPara.bEnable = pChkEable->GetValue()? 1:0;
				}
				
				SetPara();
			}
			else
			{
				//exit
				this->Close();
			}
		}
	}
}

void CPageAlarmDeal::SetAlarmDeal( EMALARMDEAL_TYPE type, int nChn )
{
	m_emDealType = type;
	m_nDealChn = nChn;

	//if (ALARMDEAL_IPCEXTSENSOR == type)
		//printf("SetAlarmDeal chn=%d\n", m_nDealChn);
	
#if 1
	if (nNVROrDecoder == 1)//nvr //yaogang modify 20150324
	{
		if(ALARMDEAL_VIDEOLOSS == type)
		{
			if(pButton[1]->IsShown())
			{
				CRect rtBt1;
				CRect rtBt2;

				pButton[1]->GetRect(&rtBt1);
				pButton[2]->GetRect(&rtBt2);

				pButton[1]->Show(FALSE);

				rtBt2.left -= rtBt1.Width();
				rtBt2.right -= rtBt1.Width();
				pButton[2]->SetRect(rtBt2, FALSE);			
			}
			//ShowSubPage(1,FALSE);
			//printf("ALARMDEAL_VIDEOLOSS  \n");
			//SwitchPage(0);
		}
		else if (ALARMDEAL_HDD == type)
		{
			pButton[1]->Show(FALSE);
			pButton[2]->Show(FALSE);
		}
		else
		{
			if(!pButton[1]->IsShown())
			{
				CRect rtBt1;
				CRect rtBt2;
				
				pButton[1]->GetRect(&rtBt1);
				pButton[2]->GetRect(&rtBt2);
				
				rtBt2.left += rtBt1.Width();
				rtBt2.right += rtBt1.Width();
				pButton[2]->SetRect(rtBt2, FALSE);
				pButton[1]->Show(TRUE);

				//SwitchPage(0);
			}
			//printf("NOT  ALARMDEAL_VIDEOLOSS  \n");
			
			//SwitchPage(0);
		}
	}
	
#endif
}

void CPageAlarmDeal::SetPara()
{
	CPageSensorAlarm* 	pSensorPage;
	CPageMDCfg* 		pMdcfgPage;
	CPageVideoLoss* 	pVidlostPage;
	CPageIPCameraExtSensorAlarm* pIPCExtSensorPage;
	
	switch(nParentLabel)
	{
		case 0:
		{
			pSensorPage = (CPageSensorAlarm*)pParent;

			pSensorPage->SavePara(m_nDealChn, &sAlarmDispatchIns);
		} break;
		case 1:
		{
			pVidlostPage = (CPageVideoLoss*)pParent;

			pVidlostPage->SavePara(m_nDealChn, &sAlarmDispatchIns);
		} break;
		case 2:
		{
			pMdcfgPage = (CPageMDCfg*)pParent;

			pMdcfgPage->SavePara(m_nDealChn, &sAlarmDispatchIns);
		} break;
		case 3:
		{
			pIPCExtSensorPage = (CPageIPCameraExtSensorAlarm*)pParent;

			pIPCExtSensorPage->SavePara(m_nDealChn, &sAlarmDispatchIns);
		} break;
		case 4:
		{
			printf("yg SaveHDDAlarmPara bEnable: %d\n", sHDDAlarmPara.bEnable);
			bizData_SaveHDDAlarmPara(&sHDDAlarmPara);
			bizData_SaveHDDAlarmDeal(&sAlarmDispatchIns);
		} break;
	}
	
	//csp modify
	SetInfo("&CfgPtn.SaveConfigSuccess");
}

void CPageAlarmDeal::GetPara()
{
	CPageSensorAlarm* 	pSensorPage;
	CPageMDCfg* 		pMdcfgPage;
	CPageVideoLoss* 	pVidlostPage;
	CPageIPCameraExtSensorAlarm* pIPCExtSensorPage;

	//printf("m_nDealChn = %d \n",m_nDealChn);
	
	switch(nParentLabel)
	{
		case 0:
		{
			pSensorPage = (CPageSensorAlarm*)pParent;
			
			pSensorPage->GetPara(m_nDealChn, &sAlarmDispatchIns);
		} break;
		case 1:
		{
			pVidlostPage = (CPageVideoLoss*)pParent;
			
			pVidlostPage->GetPara(m_nDealChn, &sAlarmDispatchIns);
		} break;
		case 2:
		{
			pMdcfgPage = (CPageMDCfg*)pParent;
			pMdcfgPage->GetPara(m_nDealChn, &sAlarmDispatchIns);
		} break;
		case 3:
		{
			pIPCExtSensorPage = (CPageIPCameraExtSensorAlarm*)pParent;
			
			pIPCExtSensorPage->GetPara(m_nDealChn, &sAlarmDispatchIns);
		} break;
		case 4://hdd alarm
		{
			//GetHddPara(&sAlarmDispatchIns);
			bizData_GetHDDAlarmPara(&sHDDAlarmPara);
			bizData_GetHDDAlarmDeal(&sAlarmDispatchIns);
			
		} break;
	}
}

VD_BOOL CPageAlarmDeal::UpdateData( UDM mode )
{
	int i = 0;
	//printf("%s yg\n", __func__);
	if( UDM_OPEN == mode)
	{
		GetPara();
		//printf("%s 1\n", __func__);
	        for (i = 0; i<nChMax; i++)
	        {
	        	//printf("i = %d\n", i);	
			bizData_GetPtzAdvancedPara((uchar)i, &psGuiPtzPara[i]);
			PRINT_HERE;
			memset(&psGuiPtzCount[i], 0, sizeof(psGuiPtzCount[i]));

			for (int j = 0; j<TOURPATH_MAX; j++)
			{
				if (psGuiPtzPara[i].sTourPath[j].nPathNo)
				{
					psGuiPtzCount[i].nTourPathCount[j] = 1;
				}
			}

			for (int j = 0; j<TOURPIONT_MAX; j++)
			{
				if (psGuiPtzPara[i].nIsPresetSet[j])
				{
					psGuiPtzCount[i].nPresetCount[j] = 1;
				}
			}
	        }
		PRINT_HERE;
		//printf("%s 2\n", __func__);
		for (i = 0; i<nAlarmOutChn; i++)
		{
			//printf("sAlarmDispatchIns.nAlarmOut[%d] = 0x%02x\n", i, sAlarmDispatchIns.nAlarmOut[i]);
			pChkTriggerAlarmPG0[i]->SetValue((sAlarmDispatchIns.nAlarmOut[i] == 0xff) ? 0 : 1);
		}
		
		pChkSoundPG0->SetValue(sAlarmDispatchIns.nFlagBuzz);
		
		if (nParentLabel != 4)
		{
			#ifndef NO_BIGVIDEO_ALARM
			pComboBigVideoPG0->SetCurSel((sAlarmDispatchIns.nZoomChn == 0xff) ? GetVideoMainNum() : sAlarmDispatchIns.nZoomChn);
			#endif
		}
		else//hdd alarm
		{
			//yaogang modify 20141010
			//yaogang modify for bad disk 默认使能
			//pChkEable->SetValue(sHDDAlarmPara.bEnable);
			pChkEable->SetValue(1);
		}
		pChkMailPG0->SetValue(sAlarmDispatchIns.nFlagEmail);
		//printf("%s 3\n", __func__);
		//nRealChNum = sizeof(pChkTriggerChnPG1)/sizeof(pChkTriggerChnPG1[0]);
		int nRealChNum = GetMaxChnNum();
		int IsRecStatue[4] = {1,1,1,1};
		for (i = 0; i<nRealChNum; i++)
		{
			//printf("sAlarmDispatchIns.nRecordChn[%d] = 0x%02x\n", i, sAlarmDispatchIns.nRecordChn[i]);
			int IsFalse = (sAlarmDispatchIns.nRecordChn[i] == 0xff) ? 0 : 1;
			pChkTriggerChnPG1[i]->SetValue(IsFalse);

			if(0 == IsFalse)
			{
				IsRecStatue[i/8] = 0;
			}
			
			if(((i + 1)%8) == 0)
			{
				pChkTriggerAllPG1[i/8]->SetValue(IsRecStatue[i/8]);
			}
		}
		PRINT_HERE;
		//printf("%s 4\n", __func__);
		//nRealChNum = sizeof(pComboTypePG2)/sizeof(pComboTypePG2[0]);
		u8 nIdx=0;
		for (i = 0; i<nRealChNum; i++)
		{
			if( 0==BizConfigGetParaListIndex(
						EM_BIZ_CFG_PARALIST_PTZLINKTYPE,
						sAlarmDispatchIns.sAlarmPtz[i].nALaPtzType,
						&nIdx ) 
			)
			{
				//pComboTypePG2[i]->SetCurSel(nIdx);
				type[i] = nIdx;				
			}
			//pComboIndexPG2[i]->SetCurSel(sAlarmDispatchIns.sAlarmPtz[i].nId-1);
			id[i] = sAlarmDispatchIns.sAlarmPtz[i].nId; 
		}
		PRINT_HERE;
		//printf("%s 5\n", __func__);
		for(i=0; i<LINKAGE_ROWS; i++)
		{
			int nCh = m_page2*LINKAGE_ROWS+i;
			int tmpRec = -1;
			int setIndex = 0;
			BOOL bTmpFlag = FALSE;
			char s[10] = {0};
			if(nCh >= GetMaxChnNum())
			{
				break;
			}

			//printf("type[%d] = %d \n",nCh,type[nCh]);
			//printf("id[%d] = %d \n",nCh,id[nCh]);
			pComboTypePG2[i]->SetCurSel(type[nCh]);
			pComboIndexPG2[i]->RemoveAll();
			if (0 == type[nCh] || 3 == type[nCh]) 
			{ //none / track
				pComboIndexPG2[i]->AddString("&CfgPtn.NONE");
				pComboIndexPG2[i]->SetCurSel(0);
				id[nCh] = 0;
				continue;
			}
			else if (1 == type[nCh])
			{ //presetPoint
				for (int j = 0; j<TOURPIONT_MAX; j++)
				{
					if (1 == psGuiPtzCount[nCh].nPresetCount[j])
					{
						sprintf(s, "%d", j+1);
						tmpRec++;
						pComboIndexPG2[i]->AddString(s);

						if (id[nCh] == atoi(pComboIndexPG2[i]->GetString(tmpRec))) 
						{
							setIndex = tmpRec;
						}
					}
				}
				if (!pComboIndexPG2[i]->GetCount()) 
				{
					pComboIndexPG2[i]->AddString("&CfgPtn.NONE");
					pComboIndexPG2[i]->SetCurSel(0);
					continue;
				}

				//printf("##alarm deal ptz preset  i %d setIndex %d\n", i, setIndex);
				pComboIndexPG2[i]->SetCurSel(setIndex);
			}
			else if (2 == type[nCh]) 
			{ //tourLine
				for (int j = 0; j<TOURPATH_MAX; j++)
				{
					if (1 == psGuiPtzCount[nCh].nTourPathCount[j])
					{
						sprintf(s, "%d", j+1);
						tmpRec++;
						pComboIndexPG2[i]->AddString(s);

						if (id[nCh] == atoi(pComboIndexPG2[i]->GetString(tmpRec))) 
						{
							setIndex = tmpRec;
						}
					}
				}
				if (!pComboIndexPG2[i]->GetCount())
				{
					pComboIndexPG2[i]->AddString("&CfgPtn.NONE");
					pComboIndexPG2[i]->SetCurSel(0);
					continue;
				}

				pComboIndexPG2[i]->SetCurSel(setIndex);
			}
		}
		//printf("%s 6\n", __func__);
	}
	else if( UDM_CLOSED == mode)
	{
		//SwitchPage(0);
	}

	return TRUE;
}


void CPageAlarmDeal::InitPage0()
{
	int i= 0;
	CRect rtTab(25, 55+pBmp_tab_normal->height+10, m_Rect.Width()-25, m_Rect.Height()-55);
	pTabPG0 = CreateTableBox(rtTab, this, 2, rtTab.Height()/CTRL_HEIGHT);
	items[0].push_back(pTabPG0);

#ifndef NO_BIGVIDEO_ALARM
	const char* szLeft[6] = {
		"&CfgPtn.Buzzer",
		"&CfgPtn.ShowFullScreen",
		"&CfgPtn.ToAlarmOut",
		"&CfgPtn.Email",
		"",
		"&CfgPtn.Snap",
	};
	int nDealItemCnr = 4;
#else
	char* szLeft[] = {
		"&CfgPtn.Buzzer",
		//CfgPtn.ShowFullScreen",
		"&CfgPtn.ToAlarmOut",
		"&CfgPtn.Email",
		"",
		"&CfgPtn.Snap",
	};
	int nDealItemCnr = 3;
#endif

	nAlarmOutChn = GetAlarmOutNum();

	CRect rt;
	for (i = 0; i<nDealItemCnr; i++)
	{
		pTabPG0->GetTableRect(0,i, &rt);
		CStatic* pText = CreateStatic(CRect(rtTab.left+rt.left+5, rtTab.top+rt.top, rtTab.left+rt.right, rtTab.top+rt.bottom), 
			this, szLeft[i]);

		pText->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);

		
		if(nAlarmOutChn == 0 && i==2) //yzw add
		{
			pText->Show(0);
			continue;
		}

		if (nParentLabel == 4)//hdd 
		{
			if (i == 0)
			{
				pText->SetText("&CfgPtn.Enable");//启用
			}
			else if (i == 1)
			{
				pText->SetText("&CfgPtn.Buzzer");//声音
			}
		}
		items[0].push_back(pText);
	}

	
	#if 0 // 关闭抓图
	pTabPG0->GetTableRect(0, 6, &rt);
	rt.left += rtTab.left + 5;
	rt.top += rtTab.top + 3;
	rt.right = rt.left + 24;
	rt.bottom = rt.top + 24;
	pChkSnapAllPG0 = CreateCheckBox(rt, this, styleEditable, (CTRLPROC)&CPageAlarmDeal::OnSnapAllChn);
	
	items[0].push_back(pChkSnapAllPG0);
	
	rt.left = rt.right + 30;
	rt.right = rt.left + 24;
	for (i = 0; i<4; i++)
	{
		pChkSnapChnPG0[i] = CreateCheckBox(rt, this);
		
		GetScrStrSetCurSelDo(
			(void*)pChkSnapChnPG0[i],
			nCtrlID[nParentLabel][0],  
			EM_GSR_CTRL_CHK, 
			i
		);
		
		char tmp[5] = {0};
		sprintf(tmp, "%d", i+1);
		CStatic* pText = CreateStatic(CRect(rt.right+2, rt.top, rt.right+30, rt.bottom), 
			this, tmp);
		pText->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);

		items[0].push_back(pChkSnapChnPG0[i]);
		items[0].push_back(pText);

		rt.left = rt.right + 30;
		rt.right = rt.left + 24;
	}
	#endif

	int row = 0;

	if (nParentLabel == 4)//hdd
	{	
		pTabPG0->GetTableRect(1, row++, &rt);
		rt.left += rtTab.left + 5;
		rt.top += rtTab.top + 3;
		rt.right = rt.left + 24;
		rt.bottom = rt.top + 24;
		pChkEable = CreateCheckBox(rt, this);
		//yaogang modify for bad disk
		//默认使能报警
		pChkEable->SetValue(1);
		pChkEable->Enable(FALSE);
		items[0].push_back(pChkEable);
	}
	//声音报警
	pTabPG0->GetTableRect(1, row++, &rt);
	rt.left += rtTab.left + 5;
	rt.top += rtTab.top + 3;
	rt.right = rt.left + 24;
	rt.bottom = rt.top + 24;
	pChkSoundPG0 = CreateCheckBox(rt, this);

	/*
	GetScrStrSetCurSelDo(
		(void*)pChkSoundPG0,
		nCtrlID[nParentLabel][1],
		EM_GSR_CTRL_CHK, 
		m_nDealChn
	);
	*/
	
	items[0].push_back(pChkSoundPG0);

	
//yaogang modify 20141010
//大画面报警
if (nParentLabel != 4)
{
#ifndef NO_BIGVIDEO_ALARM
	pTabPG0->GetTableRect(1, row++, &rt);
	rt.left += rtTab.left + 5;
	rt.top += rtTab.top + 1;
	rt.right = rt.left + 120;
	rt.bottom = rt.top + 26;
	pComboBigVideoPG0 = CreateComboBox(rt, this, NULL);
	
	GetScrStrInitComboxSelDo(
		(void*)pComboBigVideoPG0,
		nCtrlID[nParentLabel][2],
		EM_GSR_COMBLIST, 
		0
	); 

	pComboBigVideoPG0->AddString("&CfgPtn.NONE");
		
	items[0].push_back(pComboBigVideoPG0);
#endif
}
	
	//触发报警
	pTabPG0->GetTableRect(1, row++, &rt);
	rt.left += rtTab.left + 5;
	rt.top += rtTab.top + 3;
	rt.right = rt.left + 24;
	rt.bottom = rt.top + 24;
	
	if (nAlarmOutChn > sizeof(pChkTriggerAlarmPG0)/sizeof(pChkTriggerAlarmPG0[0]))
	{
		nAlarmOutChn = sizeof(pChkTriggerAlarmPG0)/sizeof(pChkTriggerAlarmPG0[0]);
	}
		
	for (i = 0; i<nAlarmOutChn; i++)
	{
		pChkTriggerAlarmPG0[i] = CreateCheckBox(rt, this);

		/*
		GetScrStrSetCurSelDo(
			(void*)pChkTriggerAlarmPG0[i],
			nCtrlID[nParentLabel][3], 
			EM_GSR_CTRL_CHK, 
			m_nDealChn
		);
		*/

		char tmp[5] = {0};
		sprintf(tmp, "%d", i+1);
		CStatic* pText = CreateStatic(CRect(rt.right+2, rt.top, rt.right+30, rt.bottom), 
			this, tmp);
		pText->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
		
		rt.left = rt.right + 30;
		rt.right = rt.left + 24;

		if(!GetAlarmOutNum()) //yzw add
		{
			pText->Show(0);
			pChkTriggerAlarmPG0[i]->Show(0);
			continue;
		}
		
		items[0].push_back(pChkTriggerAlarmPG0[i]);
		items[0].push_back(pText);
	}	
	//邮件报警
	pTabPG0->GetTableRect(1, row++, &rt);
	rt.left += rtTab.left + 5;
	rt.top += rtTab.top + 3;
	rt.right = rt.left + 24;
	rt.bottom = rt.top + 24;
	pChkMailPG0 = CreateCheckBox(rt, this);
	/*
	GetScrStrSetCurSelDo(
		(void*)pChkMailPG0,
		nCtrlID[nParentLabel][4],  
		EM_GSR_CTRL_CHK, 
		m_nDealChn
	);
	*/
			
	items[0].push_back(pChkMailPG0);
	
	//ShowSubPage(0, TRUE);
	//pButton[0]->Enable(FALSE);
}

void CPageAlarmDeal::InitPage1()
{	

#if 1
	pChkTriggerChnPG1 = (CCheckBox**)malloc(GetMaxChnNum()*sizeof(CCheckBox*));
	memset(pChkTriggerChnPG1,0,sizeof(pChkTriggerChnPG1));
	pChkTriggerAllPG1 = (CCheckBox**)calloc(GetMaxChnNum()/8 + ((GetMaxChnNum()%8)?1:0), sizeof(CCheckBox*));
	
	int i= 0;
	CRect rtTab(25, 55+pBmp_tab_normal->height+10, m_Rect.Width()-25, m_Rect.Height()-55);
	pTabPG1 = CreateTableBox(rtTab, this, 1, rtTab.Height()/CTRL_HEIGHT);
	items[1].push_back(pTabPG1);

	int rows = GetMaxChnNum()%8?GetMaxChnNum()/8+1:GetMaxChnNum()/8;
	for(int j=0; j<rows; j++)
	{
		CRect rt;
		pTabPG0->GetTableRect(0, j, &rt);
		rt.left += rtTab.left + 5;
		rt.top += rtTab.top + 1;
		rt.right = rt.left + 24;
		rt.bottom = rt.top + 23;
		pChkTriggerAllPG1[j] = CreateCheckBox(rt, this, styleEditable, (CTRLPROC)&CPageAlarmDeal::OnTriAllChn);		

		items[1].push_back(pChkTriggerAllPG1[j]);

		int tmpLeft = rt.right + 30;

		rt.left = tmpLeft;
		rt.right = rt.left + 24;
		for (i = 0; i<8; i++)
		{
			int nCh = j*8 + i;
			if(nCh >= GetMaxChnNum())
			{
				break;
			}
			
			pChkTriggerChnPG1[nCh] = CreateCheckBox(rt, this);

			/*
			GetScrStrSetCurSelDo(
				(void*)pChkTriggerChnPG1[nCh],
				nCtrlID[nParentLabel][5],  
				EM_GSR_CTRL_CHK, 
				nCh
			);
			*/

			char tmp[5] = {0};
			sprintf(tmp, "%d", nCh+1);
			CStatic* pText = CreateStatic(CRect(rt.right+2, rt.top-1, rt.right+30, rt.bottom-1), 
				this, tmp);
			pText->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);

			items[1].push_back(pChkTriggerChnPG1[nCh]);
			items[1].push_back(pText);

			rt.left = rt.right + 30;
			rt.right = rt.left + 24;
		}

		//rt.top += CTRL_HEIGHT;
		//rt.bottom += CTRL_HEIGHT;
	}


	ShowSubPage(1, FALSE);

#endif
}

void CPageAlarmDeal::AdjustLinkageRows()
{	
	int firstHide = LINKAGE_ROWS-(m_maxPage2*LINKAGE_ROWS-GetMaxChnNum());
	int lastHide = LINKAGE_ROWS;
	if(m_page2 == m_maxPage2-1)
	{	
		for(int i=firstHide; i<lastHide; i++)
		{
			
			if(pStaticIdPG2[i]
				&& pComboTypePG2[i]
				&& pComboIndexPG2[i])
			{
				pStaticIdPG2[i]->Show(FALSE,TRUE);
				pComboTypePG2[i]->Show(FALSE,TRUE);
				pComboIndexPG2[i]->Show(FALSE,TRUE);
			}			
		}
	}
	else
	{
		for(int i=firstHide; i<lastHide; i++)
		{
			if(pStaticIdPG2[i]
				&& pComboTypePG2[i]
				&& pComboIndexPG2[i])
			{
				pStaticIdPG2[i]->Show(TRUE,TRUE);
				pComboTypePG2[i]->Show(TRUE,TRUE);
				pComboIndexPG2[i]->Show(TRUE,TRUE);
			}
		}
	}
}

void CPageAlarmDeal::OnTrackMove2()
{	
	int pos = pScroll2->GetPos();

    if (m_page2==pos) 
    {
        return;
    }
    m_page2 = pos;

	for(int i=0; i<LINKAGE_ROWS; i++)
	{
		
		char szCh[16] = {0};
        int tmpRec = -1;
        int setIndex = 0;
        char s[10] = {0};
		int nCh = m_page2*LINKAGE_ROWS+i;
		if(nCh  >= GetMaxChnNum())
		{
			break;
		}
		
		sprintf(szCh,"%d",nCh+1);
		pStaticIdPG2[i]->SetText(szCh);

		//printf("type[%d] = %d \n",nCh,type[nCh]);
		pComboTypePG2[i]->SetCurSel(type[nCh]);
		//pComboIndexPG2[i]->SetCurSel(id[nCh]);
		pComboIndexPG2[i]->RemoveAll();
        if (0 == type[nCh] || 3 == type[nCh])
        { //none / track
            pComboIndexPG2[i]->AddString("&CfgPtn.NONE");
            pComboIndexPG2[i]->SetCurSel(0);
            id[nCh] = 0;
            continue;
        } 
        else if (1 == type[nCh])
        { //presetPoint
            for (int j = 0; j<TOURPIONT_MAX; j++)
            {
                if (1 == psGuiPtzCount[nCh].nPresetCount[j])
                {
                    sprintf(s, "%d", j+1);
                    tmpRec++;
                    pComboIndexPG2[i]->AddString(s);

                    if (id[nCh] == atoi(pComboIndexPG2[i]->GetString(tmpRec))) 
                    {
                        setIndex = tmpRec;
                    }
                }
            }
            if (!pComboIndexPG2[i]->GetCount()) 
            {
                pComboIndexPG2[i]->AddString("&CfgPtn.NONE");
                pComboIndexPG2[i]->SetCurSel(0);
                continue;
            }

            pComboIndexPG2[i]->SetCurSel(setIndex);
        }
        else if (2 == type[nCh])
        { //tourLine
            for (int j = 0; j<TOURPATH_MAX; j++)
            {
                if (1 == psGuiPtzCount[nCh].nTourPathCount[j])
                {
                    sprintf(s, "%d", j+1);
                    tmpRec++;
                    pComboIndexPG2[i]->AddString(s);

                    if (id[nCh] == atoi(pComboIndexPG2[i]->GetString(tmpRec))) 
                    {
                        setIndex = tmpRec;
                    }
                }
            }
            if (!pComboIndexPG2[i]->GetCount()) 
            {
                pComboIndexPG2[i]->AddString("&CfgPtn.NONE");
                pComboIndexPG2[i]->SetCurSel(0);
                continue;
            }

            pComboIndexPG2[i]->SetCurSel(setIndex);
        }
	}

	AdjustLinkageRows();

}

void CPageAlarmDeal::OnComboTypeChange2()
{
	int i = 0;
	BOOL bFind = FALSE;
	CComboBox* pFocusCombo = (CComboBox*)GetFocusItem();
	for (i = 0; i < LINKAGE_ROWS; i++)
	{
		if (pFocusCombo == pComboTypePG2[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if(bFind)
	{
		
		int nCh = m_page2*LINKAGE_ROWS+i;
        BOOL bTmpFlag = FALSE;
        int tmpRec = -1;
        char s[10] = {0};

		type[nCh] = pFocusCombo->GetCurSel();
        switch(type[nCh]) 
        {
            case 0: 
            { //none
                pComboIndexPG2[i]->RemoveAll();
                pComboIndexPG2[i]->AddString("&CfgPtn.NONE");
                pComboIndexPG2[i]->SetCurSel(0);
                id[nCh] = 0;
            } break;
            case 1: 
            { //presetPoint
                pComboIndexPG2[i]->RemoveAll();
                for (int j = 0; j<TOURPIONT_MAX; j++)
                {
                    if (1 == psGuiPtzCount[nCh].nPresetCount[j])
                    {
                        sprintf(s, "%d", j+1);
                        pComboIndexPG2[i]->AddString(s);
                    }
                }
                
                if (!pComboIndexPG2[i]->GetCount()) {
                    pComboIndexPG2[i]->AddString("&CfgPtn.NONE");
                }
                pComboIndexPG2[i]->SetCurSel(0);
                id[nCh] = atoi(pComboIndexPG2[i]->GetString(0));
            } break;
            case 2: 
            { //tourLine
                pComboIndexPG2[i]->RemoveAll();
                for (int j = 0; j<TOURPATH_MAX; j++)
                {
                    if (1 == psGuiPtzCount[nCh].nTourPathCount[j])
                    {
                        sprintf(s, "%d", j+1);
                        pComboIndexPG2[i]->AddString(s);
                    }
                }
                
                if (!pComboIndexPG2[i]->GetCount())
                {
                    pComboIndexPG2[i]->AddString("&CfgPtn.NONE");
                }
                pComboIndexPG2[i]->SetCurSel(0);
                id[nCh] = atoi(pComboIndexPG2[i]->GetString(0));
            } break;
            case 3: 
            { //track
                pComboIndexPG2[i]->RemoveAll();
                pComboIndexPG2[i]->AddString("&CfgPtn.NONE");
                pComboIndexPG2[i]->SetCurSel(0);                
                id[nCh] = 0;
            } break;
            default:
              break;
        }

		//printf("***type[%d] = %d \n", nCh, type[nCh]);
	}	
}

void CPageAlarmDeal::OnComboIndexChange2()
{
	int i = 0;
    int selIndex = 0;
	BOOL bFind = FALSE;
	CComboBox* pFocusCombo = (CComboBox*)GetFocusItem();
	for (i = 0; i < LINKAGE_ROWS; i++)
	{
		if (pFocusCombo == pComboIndexPG2[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if(bFind)
	{
		int nCh = m_page2*LINKAGE_ROWS+i;
        selIndex = pFocusCombo->GetCurSel();
        if (-1 == selIndex) 
        {
            id[nCh] = 0;
            return;
        }
		id[nCh] = atoi(pFocusCombo->GetString(selIndex));
        printf("***id[%d] = %d\n", nCh, id[nCh]);
	}	
	
}


void CPageAlarmDeal::InitPage2()
{
	#if 1
	m_page2 = 0;
	m_maxPage2 = 1;

    psGuiPtzPara = (SGuiPtzAdvancedPara *)malloc(sizeof(SGuiPtzAdvancedPara)*(GetMaxChnNum()+1));
	if(!psGuiPtzPara)
	{
		 exit(1);
	}

    psGuiPtzCount = (sGuiPtzAdvanPara *)malloc(sizeof(sGuiPtzAdvanPara)*(GetMaxChnNum()+1));
    if (!psGuiPtzCount)
    {
        exit(1);
    }

	type = (int*)malloc(sizeof(int)*GetMaxChnNum());
	id = (int*)malloc(sizeof(int)*GetMaxChnNum());

	memset(pStaticIdPG2,0,sizeof(pStaticIdPG2));
	memset(pComboTypePG2,0,sizeof(pComboTypePG2));
	memset(pComboIndexPG2,0,sizeof(pComboIndexPG2));
	
	int i= 0;
	CRect rtTab(25, 55+pBmp_tab_normal->height+10, m_Rect.Width()-25, m_Rect.Height()-60);
	pTabPG2 = CreateTableBox(rtTab, this, 3, rtTab.Height()/CTRL_HEIGHT);
	pTabPG2->SetColWidth(0, 55+10);
	pTabPG2->SetColWidth(1, 260);
	items[2].push_back(pTabPG2);

	if(GetMaxChnNum() > LINKAGE_ROWS)
	{
		m_maxPage2 = (GetMaxChnNum()%LINKAGE_ROWS)?(GetMaxChnNum()/LINKAGE_ROWS+1):(GetMaxChnNum()/LINKAGE_ROWS);
		
		pScroll2 = CreateScrollBar(CRect(m_Rect.Width()-25-25,
											55+pBmp_tab_normal->height+10+30,
											m_Rect.Width()-25,
											m_Rect.Height()-60), 
									this,
									scrollbarY,
									0,
									(m_maxPage2-1)*10, 
									1, 
									(CTRLPROC)&CPageAlarmDeal::OnTrackMove2);

		items[2].push_back(pScroll2);
	}

	char* szTitle[3] = {
		"&CfgPtn.Channel",
		"&CfgPtn.Type",
		"&CfgPtn.No",
	};

	CRect rt;
	for (i=0; i<LINKAGE_COLS; i++)
	{
		pTabPG2->GetTableRect(i, 0, &rt);
		CStatic* pText = CreateStatic(CRect(rtTab.left+rt.left, rtTab.top+rt.top, rtTab.left+rt.right, rtTab.top+rt.bottom), 
			this, szTitle[i]);

		pText->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
		pText->SetBkColor(VD_RGB(67,77,87));
	
		items[2].push_back(pText);
	}

	int rows = GetLinkRows();
	for (i=0; i<rows; i++)
	{
		pTabPG2->GetTableRect(0, i+1, &rt);

		char tmp[5] = {0};
		sprintf(tmp, "%d", i+1);
		pStaticIdPG2[i] = CreateStatic(CRect(rtTab.left+rt.left+5, rtTab.top+rt.top, rtTab.left+rt.right, rtTab.top+rt.bottom), 
			this, tmp);

		pTabPG2->GetTableRect(1, i+1, &rt);
		pComboTypePG2[i] = CreateComboBox(CRect(rtTab.left+rt.left+5, rtTab.top+rt.top+1, rtTab.left+rt.right-5, rtTab.top+rt.bottom-1), 
			this, NULL, NULL, (CTRLPROC)&CPageAlarmDeal::OnComboTypeChange2);

		GetScrStrInitComboxSelDo(
			(void*)pComboTypePG2[i],
			nCtrlID[nParentLabel][6],   
			EM_GSR_COMBLIST, 
			i
		);

		pTabPG2->GetTableRect(2, i+1, &rt);
		if(GetMaxChnNum() > LINKAGE_ROWS)
		{
			pComboIndexPG2[i] = CreateComboBox(CRect(rtTab.left+rt.left+5, rtTab.top+rt.top+1, rtTab.left+rt.right-5-25, rtTab.top+rt.bottom-1), 
												this, NULL, NULL, (CTRLPROC)&CPageAlarmDeal::OnComboIndexChange2);
		}
		else
		{
			pComboIndexPG2[i] = CreateComboBox(CRect(rtTab.left+rt.left+5, rtTab.top+rt.top+1, rtTab.left+rt.right-5, rtTab.top+rt.bottom-1), 
												this, NULL, NULL, (CTRLPROC)&CPageAlarmDeal::OnComboIndexChange2);
		}

		/*
		GetScrStrInitComboxSelDo(
			(void*)pComboIndexPG2[i],
			nCtrlID[nParentLabel][7],  
			EM_GSR_COMBLIST, 
			i
		);
		*/

		items[2].push_back(pStaticIdPG2[i]);
		items[2].push_back(pComboTypePG2[i]);
		items[2].push_back(pComboIndexPG2[i]);
	}

	ShowSubPage(2, FALSE);

	#endif
}

void CPageAlarmDeal::OnTriAllChn()
{
	int i = 0;
	BOOL bFind = FALSE;
	CCheckBox* pFocusButton = (CCheckBox *)GetFocusItem();
	for (i = 0; i < GetMaxChnNum()%8?GetMaxChnNum()/8+1:GetMaxChnNum()/8; i++)
	{
		if (pFocusButton == pChkTriggerAllPG1[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
		for(int j=0; j<8; j++)
		{
			if(j+i*8 >= GetMaxChnNum())
			{
				break;
			}
			pChkTriggerChnPG1[j+i*8]->SetValue(pChkTriggerAllPG1[i]->GetValue());
		}
	}
	
	return;
}

void CPageAlarmDeal::SetInfo(char* szInfo)
{
	pInfoBar->SetText(szInfo);
}

void CPageAlarmDeal::ClearInfo()
{
	pInfoBar->SetText("");
}

void CPageAlarmDeal::OnSnapAllChn()
{
	for(int i=0; i<sizeof(pChkSnapChnPG0)/sizeof(pChkSnapChnPG0[0]); i++)
	{
		pChkSnapChnPG0[i]->SetValue(pChkSnapAllPG0->GetValue());
	}
	
	return;
}

VD_BOOL CPageAlarmDeal::MsgProc( uint msg, uint wpa, uint lpa )//cw_scroll
{
    /*
	int px,py;
	switch(msg)
	{
	case XM_MOUSEWHEEL://cw_scrollbar
		{
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);
			CItem* pItem = GetItemAt(px, py);
			if(pItem == NULL)
			{
				if (m_nCurID == 2)
				{
					pScroll2->MsgProc(msg, wpa, lpa);
					break;
				}	
			}
			break;
		}
	default:
		break;
	}*/
	
	//csp modify
	int px,py;
	switch(msg)
	{
	/*case XM_MOUSEWHEEL://cw_scrollbar
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
	*/
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

