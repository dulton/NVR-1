#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageConfigFrameWork.h"
#include "GUI/Pages/PageBasicConfigFrameWork.h"
#include "GUI/Pages/PageShenGuangConfig.h"
//#include "GUI/Pages/PageAlarmPictureConfig.h"
#include "mydaemon.h"
#include "sg_platform.h"

#define CHORTCUT_BITMAP_DIR "/tmp/ui/data/Data/temp/"

//#define SHENGUANGCFG_SUBPAGES	5
//#define SHENGUANGCFG_BTNNUM	9

#define BTN_RZ_STYLE //默认、应用、退出。3键式

//#define FIX_DDNS_BUG

//#define WWW_ADD_LEN_MIN	1

static	MAINFRAME_ITEM	items[SHENGUANGCFG_SUBPAGES];

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_save;
static VD_BITMAP * pBmp_button_select;


static VD_BITMAP * pBmp_tab_normal;
static VD_BITMAP * pBmp_tab_push;
static VD_BITMAP * pBmp_tab_save;
static VD_BITMAP * pBmp_tab_select;

//得到IPC通道数目
static int GetMaxChnNum()
{
	return GetVideoMainNum();
}
const int Index2TypeVal[]={16, 12, 13, 14, 15, 17, 18,11};
void CPageShenGuangConfig::OnClickSubPage()
{
	//printf("OnClickSubPage\n");
	
	int i = 0, j, ret;
	BOOL bFind = FALSE;
	SGParam para;
	CButton *pFocusButton = (CButton *)GetFocusItem();

	pSSGUpload_result->SetText("");
	
	for(i = 0; i < SHENGUANGCFG_BTNNUM; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if(bFind)
	{
		
		switch (i)
		{
			//子页面
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			{
				//yaogang modify 20150316
				for(j=0; j<16; j++)
				{
					pUploadRet1[j]->Show(FALSE,TRUE);
					pUploadRet2[j]->Show(FALSE,TRUE);
				}
				
				SwitchPage(i,items[i].cursubindex);//yaogang modify 20150316			
				
			} break;
			case SHENGUANGCFG_SUBPAGES: //默认值
			{
				switch (curID)
				{
					case 0://用户报修
					{
						pComboBox0[0]->SetCurSel(4);//视频安防监控
						pComboBox0[1]->SetCurSel(2);//系统故障
					} break;
					case 1://维护维修
					{
						pComboBox1[0]->SetCurSel(0);//正常
						pComboBox1[1]->SetCurSel(0);//正常
					} break;
					case 2://测试保养
					{
						pComboBox2[0]->SetCurSel(0);//测试图像上传
						pCheckBox2All->SetValue(TRUE);
						for (i=0; i<16; i++)
						{
							pCheckBox2Chn[i]->SetValue(TRUE);
						}
					} break;
					case 3://验收管理
					{
						pCheckBox3All->SetValue(TRUE);
						for (i=0; i<16; i++)
						{
							pCheckBox3Chn[i]->SetValue(TRUE);
						}
					} break;
					case 4://基本设置
					{
						ret = bizData_GetSGparamDefault(&para);
						if (ret == 0)
						{
							if (para.RunEnable)
								pCheckBox4[0]->SetValue(1);
							else
								pCheckBox4[0]->SetValue(0);

							if (para.AlarmEnable)
								pCheckBox4[1]->SetValue(1);
							else
								pCheckBox4[1]->SetValue(0);

							if (para.DailyPicEnable)
								pCheckBox4[2]->SetValue(1);
							else
								pCheckBox4[2]->SetValue(0);

							pEdit4[1]->SetText(para.CenterIP);
						}
						else
						{
							printf("CPageShenGuangConfig: UpdateData: bizData_GetSGparamDefault failed\n");
						}
					} break;
				}
			} break;
			case SHENGUANGCFG_SUBPAGES+1: //上传/应用
			{
				int i;
				SValue SComBoxList[10];
				int nReal;
				SSG_MSG_TYPE msg;
				memset(&msg, 0, sizeof(msg));
				//GetDWellSwitchPictureList(dWellSwitchPicture, &nPicturesReal, 10);
				if (curID < 4)
				{
					pSSGUpload_result->SetText("&CfgPtn.Uploading");
				}
				switch (curID)
				{
					case 0://用户报修
					{
						msg.type = (EM_MSG_TYPE)Index2TypeVal[pComboBox0[0]->GetCurSel()];
						msg.chn = 0;
						
						GetSGSysTypeList(SComBoxList, &nReal, 10);
						//printf("yg UserRepair type: %s\n", SComBoxList[pComboBox0[1]->GetCurSel()].strDisplay);
						strcpy(msg.note, SComBoxList[pComboBox0[1]->GetCurSel()].strDisplay);
						//得到对应中文的utf8编码
						strcpy(msg.note, GetParsedString(msg.note));
						upload_sg(&msg);
						
					} break;
					case 1://维护维修
					{
						//设备维护
						msg.type = EM_MAINTAIN_CHECK;
						msg.chn = 0;
						
						GetSGMaintainTypeList(SComBoxList, &nReal, 10);
						//printf("yg Maintain type: %s\n", SComBoxList[pComboBox1[0]->GetCurSel()].strDisplay);
						strcpy(msg.note, SComBoxList[pComboBox1[0]->GetCurSel()].strDisplay);
						//得到对应中文的utf8编码
						strcpy(msg.note, GetParsedString(msg.note));
						//printf("yg Maintain type: %d-%s\n", msg.type, msg.note);
						upload_sg(&msg);

						//设备维修
						msg.type = EM_REPAIR_CHECK;
						msg.chn = 0;
						//printf("yg Repair type: %s\n", SComBoxList[pComboBox1[1]->GetCurSel()].strDisplay);
						strcpy(msg.note, SComBoxList[pComboBox1[1]->GetCurSel()].strDisplay);
						//得到对应中文的utf8编码
						strcpy(msg.note, GetParsedString(msg.note));
						//printf("yg Repair type: %d-%s\n", msg.type, msg.note);
						upload_sg(&msg);
					} break;
					case 2://测试保养
					{
						int type;
						//yaogang modify 20150316
						for(j=0; j<16; j++)
						{
							pUploadRet1[j]->Show(FALSE,TRUE);
							//pUploadRet2[j]->Show(FALSE,TRUE);
						}
						
						if (pComboBox2[0]->GetCurSel() == 0)//测试
						{
							//msg.type = EM_PIC_TEST_UPLOAD;
							type = 0;
						}
						else	//保养
						{
							//msg.type = EM_PIC_MAINTAIN_UPLOAD;
							type = 1;
						}
						//GetSGTestTypeList(SComBoxList, &nReal, 10);
						//strcpy(msg.note, SComBoxList[pComboBox2[0]->GetCurSel()].strDisplay);
						
						//printf("yg Test type: %d-%s\n", msg.type, msg.note);
						//得到对应中文的utf8编码
						//strcpy(msg.note, GetParsedString(msg.note));
						
						for (i=0; i<16; i++)
						{
							if (pCheckBox2Chn[i]->GetValue())
							{
								//msg.chn = i+1;
								//upload_sg(&msg);
								
								BizRequestSnap(i, type, NULL, NULL);
							}
						}
						
						
					} break;
					case 3://验收管理
					{
						char id[32];
						char pswd[32];
						//yaogang modify 20150316
						for(j=0; j<16; j++)
						{
							//pUploadRet1[j]->Show(FALSE,TRUE);
							pUploadRet2[j]->Show(FALSE,TRUE);
						}
						//msg.type = EM_PIC_ACCEPT_UPLOAD;
						//printf("yg Test type: %d-%s\n", msg.type, msg.note);
						//得到对应中文的utf8编码
						pEdit3[0]->GetText(id, sizeof(id)-1);
						pEdit3[1]->GetText(pswd, sizeof(pswd)-1);
						if (strlen(id) == 0 || strlen(pswd)==0)
						{
							printf("PoliceID or password is empty\n");
							pSSGUpload_result->SetText("&CfgPtn.PoliceIDorPWEmpty");
							break;
						}
						/*
						sprintf(msg.note, "%s: %s %s: %s",
							GetParsedString("&CfgPtn.PoliceID"), id, \
							GetParsedString("&CfgPtn.KeyWord"), pswd);
						*/
						for (i=0; i<16; i++)
						{
							if (pCheckBox3Chn[i]->GetValue())
							{
								//msg.chn = i+1;
								//upload_sg(&msg);
								BizRequestSnap(i, 2, id, pswd);
							}
						}
					} break;
					case 4://基本设置
					{
						
						//参数保存
						SSG_MSG_TYPE msg;
						memset(&msg, 0, sizeof(msg));
						msg.type = EM_DVR_PARAM_SAVE;
						msg.chn = 0;
						strcpy(msg.note, GetParsedString("&CfgPtn.ShenGuang"));
						strcat(msg.note, GetParsedString("&CfgPtn.ParamSave"));
						upload_sg(&msg);
						
						char serverip[100];
						char id[32];
						
						memset(id, 0, sizeof(id));
						pEdit4[0]->GetText(id, sizeof(id) -1);
						//strcpy(para.AgentID, id);
						
						memset(serverip, 0, sizeof(serverip));
						pEdit4[1]->GetText(serverip, sizeof(serverip) -1);
						//strcpy(para.CenterIP, serverip);
						
						if (strlen(serverip) == 0 || strlen(id) == 0)//GetParsedString
						{
							pSSGUpload_result->SetText(GetParsedString("&CfgPtn.IDorIPEmpty"));
							break;
						}
						
						ret = bizData_GetSGparam(&para);
						if (ret == 0)
						{
							strcpy(para.AgentID, id);
							strcpy(para.CenterIP, serverip);
							
							if (pCheckBox4[0]->GetValue())
							{
								para.RunEnable = 1;
							}
							else
							{
								para.RunEnable = 0;
							}

							if (pCheckBox4[1]->GetValue())
							{
								para.AlarmEnable= 1;
							}
							else
							{
								para.AlarmEnable = 0;
							}

							if (pCheckBox4[2]->GetValue())
							{
								para.DailyPicEnable= 1;
							}
							else
							{
								para.DailyPicEnable = 0;
							}

							ret = bizData_SaveSGparam(&para);
							if (ret != 0)
							{
								printf("CPageShenGuangConfig: UpdateData: bizData_SaveSGparam failed\n");
							}
							
						}
						else
						{
							printf("CPageShenGuangConfig: UpdateData: bizData_GetSGparam failed\n");
						}
						
					} break;
				}
			} break;
			case SHENGUANGCFG_SUBPAGES+2: //退出
			{
				this->Close();
			} break;
		}
	}
	else
	{
		if(pFocusButton == pButton4[0])//报警图片设置
		{
			m_pPageAlarmPictureCfg->Open();
		}
		else if (pFocusButton == pButton4[1]) //日常图片配置
		{
			m_pPageDailyPictureCfg->Open();
		}
	}


}


void CPageShenGuangConfig::SwitchPage(int mainID,int subID)
{
	if(mainID<0 || mainID>=SHENGUANGCFG_SUBPAGES)
	{
		return;
	}
	
	//printf("curID = %d, mainId = %d cursubindex = %d subID %d\n",curID,mainID, items[curID].cursubindex, subID);
	
	if((mainID == curID) && (subID == items[curID].cursubindex))
	{
		return;
	}
	
	//printf("###11curID = %d\n",curID);
	//printf("###11subID = %d\n",items[curID].cursubindex);
	
	//隐藏当前子页面，显示新的子页面
	ShowSubPage(curID,items[curID].cursubindex,FALSE);
	pButton[curID]->Enable(TRUE);
	
	if(curID != mainID)
	{
		curID = mainID;
		//LoadPara(mainID);//shixin
	}
	items[curID].cursubindex = subID;

	//yaogang modify 20141209 按键6: 上传改为应用"CfgPtn.Upload" "&CfgPtn.apply",
	if (curID == 4)//基本设置
	{
		pButton[6]->SetText("&CfgPtn.apply");
	}
	else
	{
		pButton[6]->SetText("&CfgPtn.Upload");
	}
	
	//printf("###22curID = %d \n",curID);
	//printf("###22subID = %d \n",items[curID].cursubindex);
	
	pButton[mainID]->Enable(FALSE);
	ShowSubPage(mainID, subID, TRUE);
}


void CPageShenGuangConfig::ShowSubPage( int mainID, int subID, BOOL bShow )
{
	if(mainID<0 || mainID>=SHENGUANGCFG_SUBPAGES)
	{
		return;
	}
	
	int count = items[mainID].SubItems[subID].size();
	//printf("CPageNetworkConfig mainID=%d, subID=%d, count=%d\n", mainID, subID, count);
	
	int i = 0;
	for(i=0; i<count; i++)
	{
		if(items[mainID].SubItems[subID][i])
		{
			items[mainID].SubItems[subID][i]->Show(bShow);
		}
		else
		{
			printf("CPageShenGuangConfig mainID=%d, subID=%d, i=%d item is NULL\n", mainID, subID, i);
		}
	}
	
	//if(mainID == 3)
		//ShowRegistButton();//shixin
	
	
	
	//else if(mainID == 0 && subID == 1)
	//	pButton01->Enable(pCheckBox01->GetValue());
    /*if(!m_IsLoader)
    {
        pStatus->SetText("");
    }*/
    
	//AdjHide();
	
	//printf("ShowSubPage 2222\n");
}

VD_BOOL CPageShenGuangConfig::UpdateData( UDM mode )
{
	SGParam para;
	int ret, i;
	
	if(UDM_OPEN == mode)
	{
		pSSGUpload_result->SetText("");
		//加载参数
		//bizData_GetSGparamDefault(&para);
		//printf("para.CenterIP: %s\n", para.CenterIP);
		//printf("para.SAlarmPicParam.IpcChn: 0x%x\n", para.SAlarmPicParam.IpcChn);
		ret = bizData_GetSGparam(&para);
		if (ret == 0)
		{
			//printf("%s--RunEnable: %d, AlarmEnable: %d, DailyPicEnable: %d\n", __func__, para.RunEnable, para.AlarmEnable, para.DailyPicEnable);
			
			if (para.RunEnable)
				pCheckBox4[0]->SetValue(1);
			else
				pCheckBox4[0]->SetValue(0);

			if (para.AlarmEnable)
				pCheckBox4[1]->SetValue(1);
			else
				pCheckBox4[1]->SetValue(0);

			if (para.DailyPicEnable)
				pCheckBox4[2]->SetValue(1);
			else
				pCheckBox4[2]->SetValue(0);

			pEdit4[0]->SetText(para.AgentID);
			pEdit4[1]->SetText(para.CenterIP);
		}
		else
		{
			printf("UpdateData: bizData_GetSGparam failed\n");
		}
	}
	
	return TRUE;
}

VD_BOOL CPageAlarmPictureConfig::UpdateData( UDM mode )
{
	SAlarmPicCFG para;
	int ret, i;
	char tmp[10];
	int chn = pComboBox3[0]->GetCurSel();
	printf("%s chn: %d\n", __func__, chn);
		
	if(UDM_OPEN == mode)
	{
		//加载参数
		//bizData_GetSGparamDefault(&para);
		//printf("para.CenterIP: %s\n", para.CenterIP);
		//printf("para.SAlarmPicParam.IpcChn: 0x%x\n", para.SAlarmPicParam.IpcChn);
		ret = bizData_GetSGAlarmParam(chn, &para);
		if (ret == 0)
		{
			sprintf(tmp, "%d", para.PreTimes);
			pEdit3[0]->SetText(tmp);
			sprintf(tmp, "%d", para.Interval);
			pEdit3[1]->SetText(tmp);
			sprintf(tmp, "%d", para.StillTimes);
			pEdit3[2]->SetText(tmp);

			//pComboBox3[0]->SetCurSel(para.AlarmInput);

			printf("para.SAlarmPicParam.IpcChn: 0x%x\n", para.IpcChn);
			if (para.IpcChn == 0xffffffff)//all
			{
				pCheckBoxAll->SetValue(1);
				for(i=0; i<16; i++)
				{
					pCheckBoxChn[i]->SetValue(1);
				}
			}
			else
			{
				pCheckBoxAll->SetValue(0);
				for(i=0; i<16; i++)
				{
					pCheckBoxChn[i]->SetValue(para.IpcChn & (1<<i));
				}
			}
		}
		else
		{
			printf("CPageAlarmPictureConfig: UpdateData: bizData_GetSGAlarmParam failed\n");
		}
	}
	
	return TRUE;
}

VD_BOOL CPageDailyPictureConfig::UpdateData( UDM mode )
{
	SDailyPicCFG para;
	int i, ret;
	
	if(UDM_OPEN == mode)
	{
		//加载参数
		//bizData_GetSGparamDefault(&para);
		//printf("para.CenterIP: %s\n", para.CenterIP);
		//printf("para.SAlarmPicParam.IpcChn: 0x%x\n", para.SAlarmPicParam.IpcChn);
		ret = bizData_GetSGDailyParam(&para);
		if (ret == 0)
		{
			if (para.Time1.TimeEnable)
				pCheckBoxTime[0]->SetValue(1);
			else
				pCheckBoxTime[0]->SetValue(0);

			if (para.Time2.TimeEnable)
				pCheckBoxTime[1]->SetValue(1);
			else
				pCheckBoxTime[1]->SetValue(0);
			
			SYSTEM_TIME time;
			//csp modify 20131213
			//GetSysTime(&time);
			GetSysTime_TZ(&time);
			time.hour = para.Time1.hour;
			time.minute= para.Time1.min;
			time.second= para.Time1.sec;
			pDateTime[0]->SetDateTime(&time);
			time.hour = para.Time2.hour;
			time.minute= para.Time2.min;
			time.second= para.Time2.sec;
			pDateTime[1]->SetDateTime(&time);

			printf("para.IpcChn1: 0x%x\n", para.IpcChn1);
			if (para.IpcChn1 == 0xffffffff)//all
			{
				pCheckBoxChnAll[0]->SetValue(1);
				for(i=0; i<16; i++)
				{
					pCheckBoxChn[0][i]->SetValue(1);
				}
			}
			else
			{
				pCheckBoxChnAll[0]->SetValue(0);
				for(i=0; i<16; i++)
				{
					pCheckBoxChn[0][i]->SetValue(para.IpcChn1 & (1<<i));
				}
			}

			printf("para.IpcChn2: 0x%x\n", para.IpcChn2);
			if (para.IpcChn2 == 0xffffffff)//all
			{
				pCheckBoxChnAll[1]->SetValue(1);
				for(i=0; i<16; i++)
				{
					pCheckBoxChn[1][i]->SetValue(1);
				}
			}
			else
			{
				pCheckBoxChnAll[1]->SetValue(0);
				for(i=0; i<16; i++)
				{
					pCheckBoxChn[1][i]->SetValue(para.IpcChn2 & (1<<i));
				}
			}
		}
		else
		{
			printf("CPageDailyPictureConfig: UpdateData: bizData_GetSGDailyParam failed\n");
		}
	}
	
	return TRUE;
}


#if 0
void CPageShenGuangConfig::ShowRegistButton( void )
{
	#ifdef FIX_DDNS_BUG
	int flag = pCheckBox3->GetValue();
	#endif
	
	//if(0 == pComboBox3[0]->GetCurSel())
	//EM_BIZ_DDNS_PROT prot=(EM_BIZ_DDNS_PROT)(GetProtocolValue(pComboBox3[0]->GetCurSel())+1);//cw_ddns
	if(prot==EM_BIZDDNS_PROT_TL ||prot==EM_BIZDDNS_PROT_DVRNET || prot==EM_BIZDDNS_PROT_JMDVR || prot==EM_BIZDDNS_PROT_JSJDVR)
	{
		#ifdef FIX_DDNS_BUG
		if(flag)
		{
			pButton3->Enable(TRUE);
		}
		else
		{
			pButton3->Enable(FALSE);
		}
		#else
		pButton3->Enable(TRUE);
		#endif
	}
	else
	{
		pButton3->Enable(FALSE);
	}
	
	#ifdef FIX_DDNS_BUG
	if(flag)
	{
		if(prot==EM_BIZDDNS_PROT_JMDVR)
		{
			pEdit3[2]->Enable(FALSE);
		}
		else
		{
			pEdit3[2]->Enable(TRUE);
		}
	}
	else
	{
		pEdit3[2]->Enable(FALSE);
	}
	#else
	//csp modify 20130322
	if(prot==EM_BIZDDNS_PROT_JMDVR)
	{
		pEdit3[2]->Enable(FALSE);
	}
	else
	{
		pEdit3[2]->Enable(TRUE);
	}
	#endif
}

#endif

void CPageShenGuangConfig::OnCheckBox()
{
	int i, flag;
	BOOL bFind1 = FALSE;
	BOOL bFind2 = FALSE;
	
	CCheckBox* pFocusButton = (CCheckBox *)GetFocusItem();

	if (pFocusButton == pCheckBox2All)//测试保养
	{
		flag = pCheckBox2All->GetValue();
		for (i=0; i<16; i++)
		{
			pCheckBox2Chn[i]->SetValue(flag);
		}
	}
	else if (pFocusButton == pCheckBox3All)//验收管理
	{
		flag = pCheckBox3All->GetValue();
		for (i=0; i<16; i++)
		{
			pCheckBox3Chn[i]->SetValue(flag);
		}
	}
	else
	{
		for (i = 0; i < 16; i++)
		{
			if (pFocusButton == pCheckBox2Chn[i] )
			{
				bFind1 = TRUE;
				break;
			}

			if (pFocusButton == pCheckBox3Chn[i] )
			{
				bFind2 = TRUE;
				break;
			}
		}

		if(bFind1)
		{
			flag = 0;
			for(i=0; i<16; i++)
			{
				if (pCheckBox2Chn[i]->GetValue())
				{
					flag++;
				}
				
			}

			if (flag == 16) 
			{
				pCheckBox2All->SetValue(1);

			}
			else
			{
				pCheckBox2All->SetValue(0);
			}

		}
		else if(bFind2)
		{
			flag = 0;
			for(i=0; i<16; i++)
			{
				if (pCheckBox3Chn[i]->GetValue())
				{
					flag++;
				}
			}
			
			if (flag == 16) 
			{
				pCheckBox3All->SetValue(1);
			}
			else
			{
				pCheckBox3All->SetValue(0);
			}
		}
	}
}

void CPageShenGuangConfig::OnEditChange3()
{
	#if 0//csp modify 20130322
	//EM_BIZ_DDNS_PROT prot=(EM_BIZ_DDNS_PROT)(GetProtocolValue(pComboBox3[0]->GetCurSel())+1);//cw_ddns
	if(prot==EM_BIZDDNS_PROT_JMDVR)
	{
		//printf("OnEditChange3\n");
		
		char szTmp[64] = {0};
		char szUser[32] = {0};
		
		pEdit3[0]->GetText(szUser, sizeof(szUser));
		
		int sel = pComboBox3[0]->GetCurSel();
		sprintf(szTmp, "%s.%s.net", szUser, pComboBox3[0]->GetString(sel));
		
		pEdit3[2]->SetText(szTmp);
	}
	#else
	//UpdateDomain();
	#endif
}

void CPageShenGuangConfig::OnCombox3()
{


}

void CPageShenGuangConfig::InitPageUserRepair()
{	
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.Width()-27,m_Rect.top+50+270);
	
	pTable3 = CreateTableBox(&rtTmp,this, 2,9,0);
	pTable3->SetColWidth(0,250);
	items[0].SubItems[0].push_back(pTable3);//
	
	const char* szStatic[2] = 
	{
		"&CfgPtn.RepairSystemName",
		"&CfgPtn.RepairSystemType",
		
	};
	
	
	int i = 0;
#if 0
	const char* szStaticName[] = 
	{		
		"&CfgPtn.BuildingIntercomSystem",
		"&CfgPtn.PerimeterAlarmSystem",
		"&CfgPtn.NetworkingAlarmSystem",
		"&CfgPtn.LocalAlarmSystem",
		"&CfgPtn.VideoMonitoringSystem",
		"&CfgPtn.AccessControlSystem",
		"&CfgPtn.ElectronicPatrolSystem",
		"&CfgPtn.OtherSecuritySystem",
	};
	const char* szStaticType[] = 
	{
		"&CfgPtn.FrontFault",
		"&CfgPtn.TerminalFault",
		"&CfgPtn.SystemFault",
	};
#endif
	CRect tmpRt;
	
	for(i=0; i<2; i++)
	{
		pTable3->GetTableRect(0,i,&tmpRt);
		pStatic3[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[i]);
		items[0].SubItems[0].push_back(pStatic3[i]);
	}

	int j;
	for(i=0; i<2; i++)
	{
		
		pTable3->GetTableRect(1,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pComboBox0[i] = CreateComboBox(CRect(rtTmp.left+tmpRt.left+2, rtTmp.top+tmpRt.top+1, 
			rtTmp.left+tmpRt.right-2 -10,rtTmp.top+tmpRt.bottom-1), 
			this, NULL, NULL, (CTRLPROC)&CPageShenGuangConfig::OnCombox3, 0);
		pComboBox0[i]->SetBkColor(VD_RGB(67,77,87));

		if (i == 0)
		{
		#if 0
			for (j=0; j<sizeof(szStaticName)/sizeof(szStaticName[0]); j++)
			{
				pComboBox0[i]->AddString(szStaticName[j]);
			}
		#endif
			SValue RepairNameList[10];
			int nRepairNameReal;
			GetRepairNameList(RepairNameList, &nRepairNameReal, 10);
			printf("%s GetRepairNameList num: %d, str[0]: %s\n", __FUNCTION__, \
				nRepairNameReal, RepairNameList[0].strDisplay);

			for (j=0; j<nRepairNameReal; j++)
			{
				pComboBox0[i]->AddString(RepairNameList[j].strDisplay);
			}
			
			pComboBox0[i]->SetCurSel(4);
		}
		else if(i == 1)
		{
		#if 0
			for (j=0; j<sizeof(szStaticType)/sizeof(szStaticType[0]); j++)
			{
				pComboBox0[i]->AddString(szStaticType[j]);
			}
		#endif
			SValue RepairTypeList[10];
			int nRepairTypeReal;
			GetRepairTypeList(RepairTypeList, &nRepairTypeReal, 10);
			printf("%s GetRepairTypeList num: %d, str[0]: %s\n", __FUNCTION__, \
				nRepairTypeReal, RepairTypeList[0].strDisplay);

			for (j=0; j<nRepairTypeReal; j++)
			{
				pComboBox0[i]->AddString(RepairTypeList[j].strDisplay);
			}
			
			pComboBox0[i]->SetCurSel(2);
		}

		//pComboBox0[i]->SetCurSel(0);
		pComboBox0[i]->Show(FALSE);
			
		#if 0
		GetScrStrInitComboxSelDo(
			(void*)pComboBox3[0],
			GSR_CONFIG_NETWORK_ADVANCE_DDNSDOMAIN,
			EM_GSR_COMBLIST,
			0
		);
		#endif
		
		items[0].SubItems[0].push_back(pComboBox0[i]);//

	}

	//items[0].cursubindex = 0;

	//ShowSubPage(0,0,FALSE);
	pButton[0]->Enable(FALSE);
	ShowSubPage(0, 0, TRUE);
	
}




void CPageShenGuangConfig::InitPageMaintain()
{	
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.Width()-27,m_Rect.top+50+270);
	
	pTable3 = CreateTableBox(&rtTmp,this, 2,9,0);
	pTable3->SetColWidth(0,250);
	items[1].SubItems[0].push_back(pTable3);//
	
	const char* szStatic[2] = 
	{
		"&CfgPtn.EquipmentMaintain",
		"&CfgPtn.EquipmentRepair",
		
	};
	
	CRect tmpRt;
	int i = 0;
	#if 0
	const char* szStatic1[] = 
	{		
		"&CfgPtn.Normal",
		"&CfgPtn.ExceptionRepair",
		"&CfgPtn.ExceptionUNRepair",
		"&CfgPtn.UNChoose",
	};
	#endif
	for(i=0; i<2; i++)
	{
		pTable3->GetTableRect(0,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic3[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[i]);
		items[1].SubItems[0].push_back(pStatic3[i]);
	
	}

	SValue MaintainTypeList[10];
	int nMaintainTypeReal;
	GetMaintainTypeList(MaintainTypeList, &nMaintainTypeReal, 10);
	printf("%s GetMaintainTypeList num: %d, str[0]: %s\n", __FUNCTION__, \
		nMaintainTypeReal, MaintainTypeList[0].strDisplay);
	
	int j;
	for(i=0; i<2; i++)
	{
		
		pTable3->GetTableRect(1,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);

		pComboBox1[i] = CreateComboBox(CRect(rtTmp.left+tmpRt.left+2, rtTmp.top+tmpRt.top+1, 
			rtTmp.left+tmpRt.right-2 -10,rtTmp.top+tmpRt.bottom-1), 
			this, NULL, NULL, (CTRLPROC)&CPageShenGuangConfig::OnCombox3, 0);
		pComboBox1[i]->SetBkColor(VD_RGB(67,77,87));

		for (j=0; j<nMaintainTypeReal; j++)
		{
			pComboBox1[i]->AddString(MaintainTypeList[j].strDisplay);
		}

		pComboBox1[i]->SetCurSel(0);
		pComboBox1[i]->Show(FALSE);
		#if 0
		GetScrStrInitComboxSelDo(
			(void*)pComboBox3[0],
			GSR_CONFIG_NETWORK_ADVANCE_DDNSDOMAIN,
			EM_GSR_COMBLIST,
			0
		);
		#endif
		
		items[1].SubItems[0].push_back(pComboBox1[i]);//

	}
	pButton[1]->Enable(TRUE);
	ShowSubPage(1, 0, FALSE);
	
}


void CPageShenGuangConfig::InitPageTest()
{	
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.Width()-27,m_Rect.top+50+270);
	
	pTable3 = CreateTableBox(&rtTmp,this, 2,9,0);
	pTable3->SetColWidth(0,100);
	items[2].SubItems[0].push_back(pTable3);//
	
	const char* szStatic[] = 
	{
		"&CfgPtn.OperateType",
		"&CfgPtn.IPChannel",
	};
	
	CRect tmpRt;
	int i = 0;
	#if 0
	const char* szStatic1[] = 
	{		
		"&CfgPtn.TestImageUpload",
		"&CfgPtn.MaintenanceImageUpload",
	};
	#endif
	for(i=0; i<sizeof(szStatic)/sizeof(szStatic[0]); i++)
	{
		pTable3->GetTableRect(0,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic3[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[i]);
		
	
			items[2].SubItems[0].push_back(pStatic3[i]);//
	
	}

	#if 0
	pTable3->GetTableRect(1,0,&tmpRt);
	pCheckBox3 = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top+3, 
		rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
		this,styleEditable, (CTRLPROC)&CPageShenGuangConfig::OnCheckBox3);
	pCheckBox3->SetValue(TRUE);
	items[3].SubItems[0].push_back(pCheckBox3);

	#endif

	SValue TestTypeList[10];
	int nTestTypeReal;
	GetTestTypeList(TestTypeList, &nTestTypeReal, 10);
	printf("%s GetTestTypeList num: %d, str[0]: %s\n", __FUNCTION__, \
		nTestTypeReal, TestTypeList[0].strDisplay);

	
	int j;
	for(i=0; i<1; i++)
	{
		
		pTable3->GetTableRect(1,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pComboBox2[i] = CreateComboBox(CRect(rtTmp.left+tmpRt.left+2, rtTmp.top+tmpRt.top+1, 
			rtTmp.left+tmpRt.right-2 -10,rtTmp.top+tmpRt.bottom-1), 
			this, NULL, NULL, (CTRLPROC)&CPageShenGuangConfig::OnCombox3, 0);
		pComboBox2[0]->SetBkColor(VD_RGB(67,77,87));

		for (j=0; j<nTestTypeReal; j++)
		{
			pComboBox2[i]->AddString(TestTypeList[j].strDisplay);
		}

		pComboBox2[i]->SetCurSel(0);
		pComboBox2[i]->Show(FALSE);
		#if 0
		GetScrStrInitComboxSelDo(
			(void*)pComboBox3[0],
			GSR_CONFIG_NETWORK_ADVANCE_DDNSDOMAIN,
			EM_GSR_COMBLIST,
			0
		);
		#endif
		
		items[2].SubItems[0].push_back(pComboBox2[0]);//

	}


	char szStaticNUM[3];
	
	//使所有通道使能
	pTable3->GetTableRect(1, 1, &tmpRt);
	pCheckBox2All = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top+3, 
		rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
		this, styleEditable, (CTRLPROC)&CPageShenGuangConfig::OnCheckBox);
	pCheckBox2All->SetValue(FALSE);
	items[2].SubItems[0].push_back(pCheckBox2All);

	int left, top;
	for (j=0; j<2; j++)//两行
	{
		pTable3->GetTableRect(1, j+2, &tmpRt);
		
		left = rtTmp.left+tmpRt.left+4;
		top = rtTmp.top+tmpRt.top+3;
		
		for(i=0; i<8; i++)
		{
			pCheckBox2Chn[i+8*j] = CreateCheckBox(CRect(left, top, 
				left+20, top+20), 
				this, styleEditable, (CTRLPROC)&CPageShenGuangConfig::OnCheckBox);
			pCheckBox2Chn[i+8*j]->SetValue(FALSE);
			items[2].SubItems[0].push_back(pCheckBox2Chn[i+8*j]);

			//left += 20+6;
			left += 20+3;
			sprintf(szStaticNUM, "%d", i+1+8*j);
			//printf("yg szStaticNUM: %s\n", szStaticNUM);
			pStaticChn[i+8*j] = CreateStatic(CRect(left, top, left+20, top+20), this, szStaticNUM);
			items[2].SubItems[0].push_back(pStaticChn[i+8*j]);
			//left += 20+10;
			left += 15;
			//图片上传结果显示
			pUploadRet1[i+8*j] =  CreateStatic(CRect(left, top, left+pBmpSuccess->width, top+pBmpSuccess->height), this, (VD_BITMAP*)NULL);
			//items[2].SubItems[0].push_back(pUploadRet1[i+8*j]);
			left += 20;
		}
	}
	
	pButton[2]->Enable(TRUE);
	ShowSubPage(2, 0, FALSE);
	
}



void CPageShenGuangConfig::InitPageAcceptManage()
{	
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.Width()-27,m_Rect.top+50+270);
	
	pTable3 = CreateTableBox(&rtTmp,this, 2,9,0);
	pTable3->SetColWidth(0,100);
	items[3].SubItems[0].push_back(pTable3);//
	
	const char* szStatic[] = 
	{
		"&CfgPtn.PoliceID",
		"&CfgPtn.KeyWord",	
		"&CfgPtn.IPChannel",
	};
	
	CRect tmpRt;
	int i, j;
	for(i=0; i<sizeof(szStatic)/sizeof(szStatic[0]); i++)
	{
		pTable3->GetTableRect(0,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic3[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[i]);
		
			items[3].SubItems[0].push_back(pStatic3[i]);//
	}

	
	for(i=0; i<2; i++)
	{
		pTable3->GetTableRect(1,i,&tmpRt);
		int vLen = (tmpRt.right-tmpRt.left)/TEXT_WIDTH*2;
		
		int nStyle = 0;
		//if(i == 1)
		//{
		//	nStyle = editPassword;
		//}
		
		#if 1//csp modify 20130322
		pEdit3[i] = CreateEdit(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
								rtTmp.left+tmpRt.right-1 - 10,rtTmp.top+tmpRt.bottom-1), 
								this,vLen,nStyle|edit_KI_NOCH, (CTRLPROC)&CPageShenGuangConfig::OnEditChange3);
		#else
		pEdit3[i] = CreateEdit(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
								rtTmp.left+tmpRt.right-1 - 10,rtTmp.top+tmpRt.bottom-1), 
								this,vLen,nStyle|edit_KI_NOCH, NULL);
		#endif
		
		pEdit3[i]->SetBkColor(VD_RGB(67,77,87));
		items[3].SubItems[0].push_back(pEdit3[i]);
	}

	char szStaticNUM[3];
	
	//使所有通道使能
	pTable3->GetTableRect(1, 2, &tmpRt);
	pCheckBox3All = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top+3, 
		rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
		this,styleEditable, (CTRLPROC)&CPageShenGuangConfig::OnCheckBox);
	pCheckBox3All->SetValue(FALSE);
	items[3].SubItems[0].push_back(pCheckBox3All);

	int left, top;
	for (j=0; j<2; j++)//两行
	{
		pTable3->GetTableRect(1, j+3, &tmpRt);
		
		left = rtTmp.left+tmpRt.left+4;
		top = rtTmp.top+tmpRt.top+3;
		
		for(i=0; i<8; i++)
		{
			pCheckBox3Chn[i+8*j] = CreateCheckBox(CRect(left, top, 
				left+20, top+20), 
				this, styleEditable, (CTRLPROC)&CPageShenGuangConfig::OnCheckBox);
			pCheckBox3Chn[i+8*j]->SetValue(FALSE);
			items[3].SubItems[0].push_back(pCheckBox3Chn[i+8*j]);

			left += 20+3;
			sprintf(szStaticNUM, "%d", i+1+8*j);
			pStaticChn[i+8*j] = CreateStatic(CRect(left, top, left+20, top+20), this, szStaticNUM);
			items[3].SubItems[0].push_back(pStaticChn[i+8*j]);
			left += 15;

			//图片上传结果显示
			pUploadRet2[i+8*j] =  CreateStatic(CRect(left, top, left+pBmpSuccess->width, top+pBmpSuccess->height), this, (VD_BITMAP*)NULL);
			//items[3].SubItems[0].push_back(pUploadRet1[i+8*j]);
			left += 20;
		}
	}
	
	pButton[3]->Enable(TRUE);
	ShowSubPage(3, 0, FALSE);

}


void CPageShenGuangConfig::InitPageBasicSetup()
{	
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.Width()-27,m_Rect.top+50+270);
	
	pTable3 = CreateTableBox(&rtTmp,this, 2,9,0);
	pTable3->SetColWidth(0,250);
	items[4].SubItems[0].push_back(pTable3);//
	
	const char* szStatic[6] = 
	{
		"&Agent ID",
		"&CfgPtn.MonitorCenter",	
		"&CfgPtn.RunStateEnable",
		"&CfgPtn.AlarmLinkEnable",
		"&CfgPtn.DayPicEnable",
		
	};
	
	CRect tmpRt;
	int i = 0;
	for(i=0; i<7; i++)
	{
#if 1
		if (i==4)
		{
			pTable3->GetTableRect(1,i,&tmpRt);
			pButton4[0] = CreateButton(CRect(rtTmp.left+tmpRt.left+2, rtTmp.top+tmpRt.top+2, 
										rtTmp.left+tmpRt.left+2+80+120,rtTmp.top+tmpRt.bottom-2), this,"&CfgPtn.AlarmPicConfig", (CTRLPROC)&CPageShenGuangConfig::OnClickSubPage, NULL, buttonNormalBmp);
			pButton4[0]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
			items[4].SubItems[0].push_back(pButton4[0]);
			
			//items[4].cursubindex = 0;

			
			continue;
		}
		if (i==5)
		{
			pTable3->GetTableRect(0,i,&tmpRt);
			//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
			pStatic3[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[i-1]);
			items[4].SubItems[0].push_back(pStatic3[i]);//
			continue;
		}

		if (i==6)
		{
			pTable3->GetTableRect(1,i,&tmpRt);
			pButton4[1] = CreateButton(CRect(rtTmp.left+tmpRt.left+2, rtTmp.top+tmpRt.top+2, 
										rtTmp.left+tmpRt.left+2+80+120,rtTmp.top+tmpRt.bottom-2), this,"&CfgPtn.DayPicConfig", (CTRLPROC)&CPageShenGuangConfig::OnClickSubPage, NULL, buttonNormalBmp);
			pButton4[1]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
			items[4].SubItems[0].push_back(pButton4[1]);
			
			//items[4].cursubindex = 0;

			
			continue;
		}

		#endif
		pTable3->GetTableRect(0,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic3[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[i]);
	
		//csp modify 20130706
		//if(i == 1)
		//{
		//	pStatic3[i]->Show(FALSE);
		//}
		//else
		//{
			items[4].SubItems[0].push_back(pStatic3[i]);//
		//}
	}

	for(i=0; i<2; i++)
	{
		pTable3->GetTableRect(1,i,&tmpRt);
		int vLen = (tmpRt.right-tmpRt.left)/TEXT_WIDTH*2;
		
		int nStyle = 0;
		//if(i == 1)
		//{
		//	nStyle = editPassword;
		//}
		
		#if 1//csp modify 20130322
		pEdit4[i] = CreateEdit(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
								rtTmp.left+tmpRt.right-1 - 10,rtTmp.top+tmpRt.bottom-1), 
								this,vLen,nStyle|edit_KI_NOCH, (CTRLPROC)&CPageShenGuangConfig::OnEditChange3);
		#else
		pEdit3[i] = CreateEdit(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
								rtTmp.left+tmpRt.right-1 - 10,rtTmp.top+tmpRt.bottom-1), 
								this,vLen,nStyle|edit_KI_NOCH, NULL);
		#endif
		
		pEdit4[i]->SetBkColor(VD_RGB(67,77,87));
		items[4].SubItems[0].push_back(pEdit4[i]);
		//ShowSubPage(4,0,FALSE);

		
	}


	
	for(i=0; i<3; i++)
	{
		pTable3->GetTableRect(1,i+2,&tmpRt);
		if (i == 2)
			pTable3->GetTableRect(1, i+3, &tmpRt);
		
		pCheckBox4[i] = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top+3, 
			rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
			this);
		pCheckBox4[i]->SetValue(FALSE);
		items[4].SubItems[0].push_back(pCheckBox4[i]);

	}
	pButton[4]->Enable(TRUE);
	ShowSubPage(4, 0, FALSE);
	
}
#if 0
void CPageShenGuangConfig::OnClickBtn()
{
	printf("CPageShenGuangConfig::OnClickBtn\n");
	
	int i = 0, j;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i=SHENGUANGCFG_SUBPAGES; i<SHENGUANGCFG_BTNNUM; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if(bFind)
	{
		switch (i)
		{
			case SHENGUANGCFG_SUBPAGES: //默认值
			{
				switch (curID)
				{
					case 0://用户报修
					{
						pComboBox0[0]->SetCurSel(4);//视频安防监控
						pComboBox0[1]->SetCurSel(2);//系统故障
					} break;
					case 1://维护维修
					{
						pComboBox1[0]->SetCurSel(0);//正常
						pComboBox1[1]->SetCurSel(0);//正常
					} break;
					case 2://测试保养
					{
						pComboBox2[0]->SetCurSel(0);//测试图像上传
						pCheckBox2All->SetValue(TRUE);
						for (j=0; j<16; j++)
						{
							pCheckBox2Chn[j]->SetValue(TRUE);
						}
					} break;
					case 3://验收管理
					{
						pCheckBox3All->SetValue(TRUE);
						for (j=0; j<16; j++)
						{
							pCheckBox3Chn[j]->SetValue(TRUE);
						}
					} break;
					case 4://基本设置
					{
					} break;
				}
			} break;
			case SHENGUANGCFG_SUBPAGES+1: //上传/应用
			{
				switch (curID)
				{
					case 0://用户报修
					{
					} break;
					case 1://维护维修
					{
					} break;
					case 2://测试保养
					{
					} break;
					case 3://验收管理
					{
					} break;
					case 4://基本设置
					{
						char id[100];
						char serverip[100];
						memset(id, 0, sizeof(id));
						memset(serverip, 0, sizeof(serverip));
						pEdit4[0]->GetText(id, sizeof(id) -1);
						pEdit4[1]->GetText(serverip, sizeof(serverip) -1);

						
					} break;
				}
			} break;
			case SHENGUANGCFG_SUBPAGES+2: //退出
			{
				this->Close();
			} break;
		}
	}
}
#endif
CPageShenGuangConfig::CPageShenGuangConfig( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent), curID(0)
{
	//nCurPage = 0;

	nCurPage = 0;
	
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

	//pBmp_button_save = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	//ReplaceBitmapBits(pBmp_button_save, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));



	//第一个页面的子页面控制按钮  录像设置，手动录像，录像布防
	pBmp_tab_normal = VD_LoadBitmap(DATA_DIR"/temp/tab.bmp");
	pBmp_tab_push = VD_LoadBitmap(DATA_DIR"/temp/tab_check.bmp");
	//pBmp_tab_save = VD_LoadBitmap(DATA_DIR"/temp/tab_f.bmp");
	pBmp_tab_select = VD_LoadBitmap(DATA_DIR"/temp/tab_f.bmp");

#ifdef BTN_RZ_STYLE
	const char* szSubPgName[SHENGUANGCFG_BTNNUM] = 
	{
		"&CfgPtn.UserRepair",
		"&CfgPtn.Maintain",
		"&CfgPtn.TestBaoyang",
		"&CfgPtn.AcceptManage",
		"&CfgPtn.BasicSetup",
		"&CfgPtn.Default",
		"&CfgPtn.Upload",//"&CfgPtn.apply",
		"&CfgPtn.Exit",
	};

	int szSubPgNameLen[SHENGUANGCFG_BTNNUM] = 
	{
		TEXT_WIDTH*4,
		TEXT_WIDTH*4,
		TEXT_WIDTH*4,
		TEXT_WIDTH*4,
		TEXT_WIDTH*4,
		TEXT_WIDTH*3,
		TEXT_WIDTH*2,
		TEXT_WIDTH*2,
	};

#else
	const char* szSubPgName[SHENGUANGCFG_BTNNUM+1] = 
	{
		"&CfgPtn.UserRepair",
		"&CfgPtn.Maintain",
		"&CfgPtn.TestBaoyang",
		"&CfgPtn.AcceptManage",
		"&CfgPtn.BasicSetup",//xd
		"&CfgPtn.apply",
		"&CfgPtn.Save",//保存
		"&CfgPtn.Exit",
		"&CfgPtn.Default",

	};
	int szSubPgNameLen[SHENGUANGCFG_BTNNUM+1] = 
	{
		TEXT_WIDTH*4,//"&CfgPtn.Network",
		TEXT_WIDTH*5,//"&CfgPtn.Substream",
		TEXT_WIDTH*4,//"&CfgPtn.Email",
		TEXT_WIDTH*4,//"&CfgPtn.OtherSettings",
		TEXT_WIDTH*4,//"&CfgPtn.P2pcloud",//xdc
		TEXT_WIDTH*2+10,//"&CfgPtn.Default",
		TEXT_WIDTH*2,//"&CfgPtn.apply",
		TEXT_WIDTH*2,//"&CfgPtn.Exit",
		TEXT_WIDTH*3,

	};

#endif
	


	//SetMargin(0, m_TitleHeight, 0, m_TitleHeight);
 	
 	CTableBox* pTabFrame = CreateTableBox(CRect(17, 40, m_Rect.Width()-17, 40+290),this,1,1);
 	//pTabFrame->SetFrameColor(VD_RGB(56,108,148));
 	pTabFrame->SetFrameColor(VD_RGB(8,66,189));
 	
 	CRect rtSub(25, 10, m_Rect.Width()-20, 10+pBmp_tab_normal->height);
 	int i;


	
 	for(i=0; i<SHENGUANGCFG_SUBPAGES; i++)
 	{
 		int btWidth = szSubPgNameLen[i]+10;
 		rtSub.right = rtSub.left + btWidth;
 		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageShenGuangConfig::OnClickSubPage, NULL, buttonNormalBmp, 1);
 		pButton[i]->SetBitmap(pBmp_tab_normal, pBmp_tab_select, pBmp_tab_push, pBmp_tab_push);//shixin
 		rtSub.left = rtSub.right;
 	}
	
 	CRect rtSub1(295, m_Rect.Height()-80, m_Rect.Width()-20, m_Rect.Height()-80+pBmp_button_normal->height);

 	rtSub1.left = rtSub1.right;
 	for(i=SHENGUANGCFG_SUBPAGES; i<SHENGUANGCFG_BTNNUM; i++)
 	{
 		rtSub1.left -= szSubPgNameLen[i]+10;
 	}
 	
 	rtSub1.left -= 10;
 	
     int ReLeft = rtSub1.left;
 	
 	for(i=SHENGUANGCFG_SUBPAGES; i<SHENGUANGCFG_BTNNUM; i++)
 	{
         int btWidth = szSubPgNameLen[i]+10;
 		rtSub1.right = rtSub1.left + btWidth;
 		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageShenGuangConfig::OnClickSubPage, NULL, buttonNormalBmp);
 		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
 		rtSub1.left = rtSub1.right+5;
 	}

	rtSub1.left = 25;
	rtSub1.right = ReLeft -10;
	pSSGUpload_result = CreateStatic(rtSub1, this, "");
	//pSSGUpload_result->SetBkColor(VD_RGB(67,77,87));
     	pSSGUpload_result->SetTextAlign(VD_TA_LEFT);
	//pSSGUpload_result->SetText("");
 	//CRect rtSub2(13, 35, m_Rect.Width()-18, 300);
 	//pTable = CreateTableBox(&rtSub2,this, 1,1,0);
 	

#ifndef BTN_RZ_STYLE
 	CRect staRect(20, m_Rect.Height()-82, ReLeft - 2/*25+TEXT_WIDTH*10*/, m_Rect.Height()-52);//shixin
	
 	pStatus = CreateStatic(staRect, this, "");
     //pStatus->SetBkColor(VD_RGB(67,77,87));
     //pStatus->SetTextAlign(VD_TA_CENTER);
     
     rtSub1.left = 37;

	 
     for(i=SHENGUANGCFG_BTNNUM; i<SHENGUANGCFG_BTNNUM+1; i++)
 	{
         //printf("***%s\n", __FUNCTION__);
         //int btWidth = szSubPgNameLen[i];
 		rtSub1.right = rtSub1.left + 72;//btWidth;
 		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageShenGuangConfig::OnClickBtn, NULL, buttonNormalBmp);
 		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
 		rtSub1.left = rtSub1.right+12;
 	}
#endif

	pBmpSuccess = VD_LoadBitmap(CHORTCUT_BITMAP_DIR"pic_upload_success.bmp");		
	pBmpFailure = VD_LoadBitmap(CHORTCUT_BITMAP_DIR"pic_upload_failure.bmp");	
	
	InitPageUserRepair();
	InitPageMaintain();
	InitPageTest();
	InitPageAcceptManage();
	InitPageBasicSetup();
	 
	// m_pPageAlarmPictureCfg = new CPageAlarmPictureConfig(NULL, "&CfgPtn.ShenGuang", icon_dvr, this);//shixin
	
 	//RegisterFlag = 0;
 	m_pPageAlarmPictureCfg = new CPageAlarmPictureConfig(NULL, "&CfgPtn.AlarmPicConfig", icon_dvr, this);
	m_pPageDailyPictureCfg = new CPageDailyPictureConfig(NULL, "&CfgPtn.DayPicConfig", icon_dvr, this);
	
}

CPageShenGuangConfig::~CPageShenGuangConfig()
{

}	


VD_PCSTR CPageShenGuangConfig::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

/*
//0: 上传成功
//1: 服务器连接失败
//2: 发送失败
//3: 接收失败
//4: DNS解析失败
//5: 图片信息有误
//6: 图片base64转码失败
//7: 获取磁盘信息失败
//8: 申请内存失败
*/
const char *FailText[] = {
	"&CfgPtn.UploadSuccess",
	"&CfgPtn.ConnectFail",
	"&CfgPtn.SendFail",
	"&CfgPtn.RecvFail",
	"&CfgPtn.DNSResolveFail",
	"&CfgPtn.PicInvalid",
	"&CfgPtn.Base64Fail",
	"&CfgPtn.GetDiskInfoFail",
	"&CfgPtn.MemoryFaild",
};

void CPageShenGuangConfig::RecvNotify(u8 chn, u8 ErrVal)
{
	char tmp[100] = {0};

	if (ErrVal > 8)
	{
		printf("%s ErrVal invalid: %d\n", __func__, ErrVal);
		return;
	}
	if (chn > GetMaxChnNum())
	{
		printf("%s chn invalid: %d\n", __func__, chn);
		return;
	}
	
	if (chn == 0)
	{
		sprintf(tmp, "%s", GetParsedString(FailText[ErrVal]));
	}
	else
	{
		sprintf(tmp, "%s %d %s", GetParsedString("&CfgPtn.Channel"), chn, GetParsedString(FailText[ErrVal]));
	}

	if (ErrVal == 0)//pic upload success
	{
		if (curID == 2) //测试包养
		{
			pUploadRet1[chn -1]->SetBitmap(pBmpSuccess);
			pUploadRet1[chn -1]->Show(TRUE,TRUE);
		}
		else if (curID == 3) //验收管理
		{
			pUploadRet2[chn -1]->SetBitmap(pBmpSuccess);
			pUploadRet2[chn -1]->Show(TRUE,TRUE);
		}
	}
	else //failure
	{
		if (curID == 2) //测试包养
		{
			pUploadRet1[chn -1]->SetBitmap(pBmpFailure);
			pUploadRet1[chn -1]->Show(TRUE,TRUE);
		}
		else if (curID == 3) //验收管理
		{
			pUploadRet2[chn -1]->SetBitmap(pBmpFailure);
			pUploadRet2[chn -1]->Show(TRUE,TRUE);
		}
	}
	//pSSGUpload_result->SetText(tmp);
}



CPageAlarmPictureConfig::CPageAlarmPictureConfig( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
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

	//pBmp_button_save = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	//ReplaceBitmapBits(pBmp_button_save, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));



	//第一个页面的子页面控制按钮  录像设置，手动录像，录像布防
	pBmp_tab_normal = VD_LoadBitmap(DATA_DIR"/temp/tab.bmp");
	pBmp_tab_push = VD_LoadBitmap(DATA_DIR"/temp/tab_check.bmp");
	//pBmp_tab_save = VD_LoadBitmap(DATA_DIR"/temp/tab_f.bmp");
	pBmp_tab_select = VD_LoadBitmap(DATA_DIR"/temp/tab_f.bmp");

#ifdef BTN_RZ_STYLE
	const char* szSubPgName[3] = 
	{
		"&CfgPtn.Default",
		"&CfgPtn.apply",
		//"&CfgPtn.Save",
		"&CfgPtn.Exit",//"&CfgPtn.Prev",
	};
	
	int szSubPgNameLen[3] = 
	{
		TEXT_WIDTH*3,//"&CfgPtn.Default",
		TEXT_WIDTH*2,//"&CfgPtn.apply",
		TEXT_WIDTH*2,//"&CfgPtn.Exit",
	};

#else
	const char* szSubPgName[4] = 
	{
		"&CfgPtn.apply",
		"&CfgPtn.Save",
		"&CfgPtn.Prev",
		"&CfgPtn.Default",

	};
	
	int szSubPgNameLen[4] = 
	{
		TEXT_WIDTH*2+10,//"&CfgPtn.Default",
		TEXT_WIDTH*2,//"&CfgPtn.apply",
		TEXT_WIDTH*2,//"&CfgPtn.Exit",
		TEXT_WIDTH*3,

	};

#endif
	

	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);
 	
 	CTableBox* pTabFrame = CreateTableBox(CRect(17, 40, m_Rect.Width()-17, 40+290),this,1,1);
 	//pTabFrame->SetFrameColor(VD_RGB(56,108,148));
 	pTabFrame->SetFrameColor(VD_RGB(8,66,189));
 	
 	//CRect rtSub(25, 10, m_Rect.Width()-20, 10+pBmp_tab_normal->height);
 	int i;

	CRect rtSub1(295, m_Rect.Height()-80, m_Rect.Width()-20, m_Rect.Height()-80+pBmp_button_normal->height);
 	rtSub1.left = rtSub1.right;

 	for(i=0; i<3; i++)
 	{
 		rtSub1.left -= szSubPgNameLen[i]+10;
 	}
 	
 	rtSub1.left -= 10;
 	//rtSub1.left += 40;
 	
     int ReLeft = rtSub1.left;
 	
 	for(i=0; i<3; i++)
 	{
         int btWidth = szSubPgNameLen[i]+10;
 		rtSub1.right = rtSub1.left + btWidth;
 		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageAlarmPictureConfig::OnClickBtn, NULL, buttonNormalBmp);
 		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
 		rtSub1.left = rtSub1.right+5;
 	}
 	
 	//CRect rtSub2(13, 35, m_Rect.Width()-18, 300);
 	//pTable = CreateTableBox(&rtSub2,this, 1,1,0);
 	

#ifndef BTN_RZ_STYLE 	
 	CRect staRect(20, m_Rect.Height()-82, ReLeft - 2/*25+TEXT_WIDTH*10*/, m_Rect.Height()-52);//shixin




 	
 	pStatus = CreateStatic(staRect, this, "");
     //pStatus->SetBkColor(VD_RGB(67,77,87));
     //pStatus->SetTextAlign(VD_TA_CENTER);
     
     rtSub1.left = 37;

	 
     for(i=3; i<4; i++)
 	{
         //printf("***%s\n", __FUNCTION__);
         //int btWidth = szSubPgNameLen[i];
 		rtSub1.right = rtSub1.left + 72;//btWidth;
 		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageAlarmPictureConfig::OnClickBtn, NULL, buttonNormalBmp);
 		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
 		rtSub1.left = rtSub1.right+12;
 	}
#endif
	
	InitPageAlarmPictureConfig();
	

	 //InitPageUserRepair();
	 

}

void CPageAlarmPictureConfig::InitPageAlarmPictureConfig()
{
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
	m_Rect.Width()-27,m_Rect.top+50+270);
	
	pTable3 = CreateTableBox(&rtTmp,this, 2,9,0);
	pTable3->SetColWidth(0,130);
	//items[0].SubItems[0].push_back(pTable3);//
	
	const char* szStatic[] = 
	{
		"&CfgPtn.LeadTimeInSecond",
		"&CfgPtn.IntervalTimeInSecond",	
		"&CfgPtn.AlarmInput",
		"&CfgPtn.RecordTimeInSeconds",
		"&CfgPtn.IPChannel",
		//"&CfgPtn.D1",

	};
	
	CRect tmpRt;
	int i = 0;
	//for(i=0; i<6; i++)
	for(i=0; i<sizeof(szStatic)/sizeof(szStatic[0]); i++)
	{
		pTable3->GetTableRect(0,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic3[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[i]);
		
			//items[0].SubItems[0].push_back(pStatic3[i]);
	}
	
	for(i=0; i<2; i++)
	{
		pTable3->GetTableRect(1,i,&tmpRt);
		int vLen = (tmpRt.right-tmpRt.left)/TEXT_WIDTH*2;
		
		int nStyle = 0;
		
		#if 1//csp modify 20130322
		pEdit3[i] = CreateEdit(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
								rtTmp.left+tmpRt.right-1 - 10,rtTmp.top+tmpRt.bottom-1), 
								this,vLen,nStyle|edit_KI_NUMERIC, (CTRLPROC)&CPageAlarmPictureConfig::OnEditChange3);

		#endif
		
		pEdit3[i]->SetBkColor(VD_RGB(67,77,87));
		//items[0].SubItems[0].push_back(pEdit3[i]);
	}

//报警输入通道数目
	SBizParaTarget bizTar;
	SBizDvrInfo bizDvrInfo;
	bizTar.emBizParaType = EM_BIZ_DVRINFO;
	
	if(BizGetPara(&bizTar, &bizDvrInfo) !=0)
	{
		BIZ_DATA_DBG("Get EM_BIZ_DVRINFO nOutputSync\n");
	}
	m_SensorNum = bizDvrInfo.nSensorNum;
	m_IpcNum = bizDvrInfo.nVidMainNum;
	printf("%s m_SensorNum: %d\n", __func__, m_SensorNum);
	printf("%s m_IpcNum: %d\n", __func__, m_IpcNum);
	
	int j;
	char tmp[32];
	for(i=2; i<3; i++)
	{
		
		pTable3->GetTableRect(1,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pComboBox3[0] = CreateComboBox(CRect(rtTmp.left+tmpRt.left+2, rtTmp.top+tmpRt.top+1, 
			rtTmp.left+tmpRt.right-2 -10,rtTmp.top+tmpRt.bottom-1), 
			this, NULL, NULL, (CTRLPROC)&CPageAlarmPictureConfig::OnCombox3, 0);
		pComboBox3[0]->SetBkColor(VD_RGB(67,77,87));

		//本机报警n
		for (j=0; j<m_SensorNum; j++)
		{
			sprintf(tmp, "%s-%2d", GetParsedString("&CfgPtn.LocalAlarm"), j+1);
			pComboBox3[0]->AddString(tmp);
		}
		//IPC外接传感器报警
		for (j=0; j<m_IpcNum; j++)
		{
			sprintf(tmp, "IPC-%2d", j+1);
			pComboBox3[0]->AddString(tmp);
		}
		pComboBox3[0]->SetCurSel(0);
#if 0
		GetScrStrInitComboxSelDo(
			(void*)pComboBox3[0],
			GSR_CONFIG_ALARM_IPCEXTSENSOR_SCH_CHN, 
			EM_GSR_COMBLIST, 
			0);
		pComboBox3[0]->SetCurSel(0);
#endif		
	//	items[0].SubItems[0].push_back(pComboBox3[0]);//

	}



	
	for(i=3; i<4; i++)
	{
		pTable3->GetTableRect(1,i,&tmpRt);
		int vLen = (tmpRt.right-tmpRt.left)/TEXT_WIDTH*2;
		
		int nStyle = 0;
		
		#if 1//csp modify 20130322
		pEdit3[i-1] = CreateEdit(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
									rtTmp.left+tmpRt.right-1 - 10,rtTmp.top+tmpRt.bottom-1), 
									this,vLen,nStyle|edit_KI_NUMERIC, (CTRLPROC)&CPageAlarmPictureConfig::OnEditChange3);
		#endif
		
		pEdit3[i-1]->SetBkColor(VD_RGB(67,77,87));
		//items[0].SubItems[0].push_back(pEdit3[i]);
		
	}
#if 0
	for(i=4; i<6; i++)
	{
	   printf("start print checkbox \n");


		pTable3->GetTableRect(1,i,&tmpRt);
		pCheckBox3[i-4] = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top+3, 
			rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
			this,styleEditable, (CTRLPROC)&CPageAlarmPictureConfig::OnCheckBox3);
		pCheckBox3[i-4]->SetValue(TRUE);
		//items[0].SubItems[0].push_back(pCheckBox3[i-4]);

		 printf("complete print checkbox \n");


	}
#endif
	char szStaticNUM[3];
	
	//使所有通道使能
	pTable3->GetTableRect(1, 4, &tmpRt);
	pCheckBoxAll = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top+3, 
		rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
		this, styleEditable, (CTRLPROC)&CPageAlarmPictureConfig::OnCheckBox);
	pCheckBoxAll->SetValue(FALSE);

	int left, top;
	for (j=0; j<2; j++)//两行
	{
		pTable3->GetTableRect(1, j+5, &tmpRt);
		
		left = rtTmp.left+tmpRt.left+4;
		top = rtTmp.top+tmpRt.top+3;
		
		for(i=0; i<8; i++)
		{
			pCheckBoxChn[i+8*j] = CreateCheckBox(CRect(left, top, 
				left+20, top+20), 
				this, styleEditable, (CTRLPROC)&CPageAlarmPictureConfig::OnCheckBox);
			pCheckBoxChn[i+8*j]->SetValue(FALSE);

			left += 20+6;
			
			sprintf(szStaticNUM, "%d", i+1+8*j);
			pStaticChn[i+8*j] = CreateStatic(CRect(left, top, left+20, top+20), this, szStaticNUM);
			
			left += 20+10;
			
		}
	}
	
	//ShowSubPage(3,0,FALSE);

}

void CPageAlarmPictureConfig::OnCheckBox()
{
	
	int i, flag;
	BOOL bFind = FALSE;
	
	CCheckBox* pFocusButton = (CCheckBox *)GetFocusItem();
	
	if (pFocusButton == pCheckBoxAll)
	{
		flag = pCheckBoxAll->GetValue();
		for (i=0; i<16; i++)
		{
			pCheckBoxChn[i]->SetValue(flag);
		}
	}
	else
	{
		for (i = 0; i < 16; i++)
		{
			if (pFocusButton == pCheckBoxChn[i] )
			{
				bFind = TRUE;
				break;
			}
		}
		if(bFind)
		{
			flag = 0;
			for(i=0; i<16; i++)
			{
				if (pCheckBoxChn[i]->GetValue())
				{
					flag++;
				}
				
			}

			if (flag == 16) 
			{
				pCheckBoxAll->SetValue(1);

			}
			else
			{
				pCheckBoxAll->SetValue(0);
			}

		}
	}
}

void CPageAlarmPictureConfig::OnEditChange3()
{
	#if 0//csp modify 20130322
	//EM_BIZ_DDNS_PROT prot=(EM_BIZ_DDNS_PROT)(GetProtocolValue(pComboBox3[0]->GetCurSel())+1);//cw_ddns
	if(prot==EM_BIZDDNS_PROT_JMDVR)
	{
		//printf("OnEditChange3\n");
		
		char szTmp[64] = {0};
		char szUser[32] = {0};
		
		pEdit3[0]->GetText(szUser, sizeof(szUser));
		
		int sel = pComboBox3[0]->GetCurSel();
		sprintf(szTmp, "%s.%s.net", szUser, pComboBox3[0]->GetString(sel));
		
		pEdit3[2]->SetText(szTmp);
	}
	#else
	//UpdateDomain();
	#endif
}

void CPageAlarmPictureConfig::ShowSubPage( int mainID, int subID, BOOL bShow )
{
	if(mainID<0 || mainID>=SHENGUANGCFG_SUBPAGES)
	{
		return;
	}
	
	int count = items[mainID].SubItems[subID].size();
	//printf("CPageNetworkConfig mainID=%d, subID=%d, count=%d\n", mainID, subID, count);
	
	int i = 0;
	for(i=0; i<count; i++)
	{
		if(items[mainID].SubItems[subID][i])
		{
			items[mainID].SubItems[subID][i]->Show(bShow);
		}
		else
		{
			printf("CPageShenGuangConfig mainID=%d, subID=%d, i=%d item is NULL\n", mainID, subID, i);
		}
	}
	
	//if(mainID == 3)
		//ShowRegistButton();//shixin
	
	
	
	//else if(mainID == 0 && subID == 1)
	//	pButton01->Enable(pCheckBox01->GetValue());
    /*if(!m_IsLoader)
    {
        pStatus->SetText("");
    }*/
    
	//AdjHide();
	
	//printf("ShowSubPage 2222\n");
}


void CPageAlarmPictureConfig::OnCombox3()
{
	int chn = pComboBox3[0]->GetCurSel();
	int ret, i;
	char tmp[10];
	SAlarmPicCFG para;

	ret = bizData_GetSGAlarmParam(chn, &para);
	if (ret == 0)
	{
		sprintf(tmp, "%d", para.PreTimes);
		pEdit3[0]->SetText(tmp);
		sprintf(tmp, "%d", para.Interval);
		pEdit3[1]->SetText(tmp);
		sprintf(tmp, "%d", para.StillTimes);
		pEdit3[2]->SetText(tmp);

		printf("chn%d, para.SAlarmPicParam.IpcChn: 0x%x\n", chn, para.IpcChn);
		if (para.IpcChn == 0xffffffff)//all
		{
			pCheckBoxAll->SetValue(1);
			for(i=0; i<m_IpcNum; i++)
			{
				pCheckBoxChn[i]->SetValue(1);
			}
		}
		else
		{
			pCheckBoxAll->SetValue(0);
			for(i=0; i<m_IpcNum; i++)
			{
				pCheckBoxChn[i]->SetValue(para.IpcChn & (1<<i));
			}
		}
	}
	else
	{
		printf("CPageAlarmPictureConfig: UpdateData: bizData_GetSGAlarmParam failed\n");
	}

}

void CPageAlarmPictureConfig::OnClickBtn()
{
	int i = 0, j, ret;
	BOOL bFind = FALSE;
	SAlarmPicCFG para;
	char tmp[10];
	int chn = pComboBox3[0]->GetCurSel();
	
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i=0; i<3; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	if (bFind)
	{
		switch (i)
		{
			case 0: //默认值
			{
				ret = bizData_GetSGAlarmParamDefault(chn, &para);
				if (ret == 0)
				{
					sprintf(tmp, "%d", para.PreTimes);
					pEdit3[0]->SetText(tmp);
					sprintf(tmp, "%d", para.Interval);
					pEdit3[1]->SetText(tmp);
					sprintf(tmp, "%d", para.StillTimes);
					pEdit3[2]->SetText(tmp);

					//pComboBox3[0]->SetCurSel(para.SAlarmPicParam.AlarmInput);

					printf("para.IpcChn: 0x%x\n", para.IpcChn);
					if (para.IpcChn == 0xffffffff)//all
					{
						pCheckBoxAll->SetValue(1);
						for(i=0; i<16; i++)
						{
							pCheckBoxChn[i]->SetValue(1);
						}
					}
					else
					{
						pCheckBoxAll->SetValue(0);
						for(i=0; i<16; i++)
						{
							pCheckBoxChn[i]->SetValue(para.IpcChn & (1<<i));
						}
					}
				}
				else
				{
					printf("CPageAlarmPictureConfig: OnClickBtn: bizData_GetSGAlarmParamDefault failed\n");
				}
			} break;
			case 1: //应用
			{
				//参数保存
				SSG_MSG_TYPE msg;
				memset(&msg, 0, sizeof(msg));
				msg.type = EM_DVR_PARAM_SAVE;
				msg.chn = 0;
				strcpy(msg.note, GetParsedString("&CfgPtn.AlarmPicConfig"));
				strcat(msg.note, GetParsedString("&CfgPtn.ParamSave"));
				upload_sg(&msg);
						
				ret = bizData_GetSGAlarmParam(chn, &para);
				if (ret == 0)
				{
					pEdit3[0]->GetText(tmp, sizeof(tmp));
					para.PreTimes = atoi(tmp);
					
					pEdit3[1]->GetText(tmp, sizeof(tmp));
					para.Interval = atoi(tmp);
					
					pEdit3[2]->GetText(tmp, sizeof(tmp));
					para.StillTimes = atoi(tmp);

					//para.SAlarmPicParam.AlarmInput = pComboBox3[0]->GetCurSel();

					para.IpcChn = 0;
					if (pCheckBoxAll->GetValue())//all
					{
						para.IpcChn = 0xffffffff;
					}
					else
					{
						for(i=0; i<16; i++)
						{
							if (pCheckBoxChn[i]->GetValue())
								para.IpcChn |= 1<<i;
						}
					}
					printf("para.SAlarmPicParam.IpcChn: 0x%x\n", para.IpcChn);

					ret = bizData_SaveSGAlarmParam(chn, &para);
					if (ret != 0)
					{
						printf("CPageAlarmPictureConfig: OnClickBtn: bizData_SaveSGAlarmParam failed\n");
					}
				}
				else
				{
					printf("CPageAlarmPictureConfig: OnClickBtn: bizData_GetSGAlarmParam failed\n");
				}
			} break;
			case 2: //退出
			{
				this->Close();
			} break;
		}
	}
}

CPageAlarmPictureConfig::~CPageAlarmPictureConfig()
{

}	


VD_PCSTR CPageAlarmPictureConfig::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}




CPageDailyPictureConfig::CPageDailyPictureConfig( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
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

	//pBmp_button_save = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	//ReplaceBitmapBits(pBmp_button_save, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));



	//第一个页面的子页面控制按钮  录像设置，手动录像，录像布防
	pBmp_tab_normal = VD_LoadBitmap(DATA_DIR"/temp/tab.bmp");
	pBmp_tab_push = VD_LoadBitmap(DATA_DIR"/temp/tab_check.bmp");
	//pBmp_tab_save = VD_LoadBitmap(DATA_DIR"/temp/tab_f.bmp");
	pBmp_tab_select = VD_LoadBitmap(DATA_DIR"/temp/tab_f.bmp");

#ifdef BTN_RZ_STYLE

	const char* szSubPgName[3] = 
	{
		"&CfgPtn.Default",
		"&CfgPtn.apply",//"&CfgPtn.Save",
		"&CfgPtn.Exit",//"&CfgPtn.Prev",
		
	};
	
	int szSubPgNameLen[3] = 
	{
		TEXT_WIDTH*3,//"&CfgPtn.Default",
		TEXT_WIDTH*2,//"&CfgPtn.apply",
		TEXT_WIDTH*2,//"&CfgPtn.Exit",
		
	};
	
#else

	const char* szSubPgName[4] = 
	{
		"&CfgPtn.apply",
		"&CfgPtn.Save",
		"&CfgPtn.Prev",
		"&CfgPtn.Default",

	};
	
	int szSubPgNameLen[4] = 
	{
		TEXT_WIDTH*2+10,//"&CfgPtn.Default",
		TEXT_WIDTH*2,//"&CfgPtn.apply",
		TEXT_WIDTH*2,//"&CfgPtn.Exit",
		TEXT_WIDTH*3,

	};
	
#endif

	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);
	
	CTableBox* pTabFrame = CreateTableBox(CRect(17, 40, m_Rect.Width()-17, 40+290),this,1,1);
	//pTabFrame->SetFrameColor(VD_RGB(56,108,148));
	pTabFrame->SetFrameColor(VD_RGB(8,66,189));
	
	//CRect rtSub(25, 10, m_Rect.Width()-20, 10+pBmp_tab_normal->height);
	int i;

	CRect rtSub1(295, m_Rect.Height()-80, m_Rect.Width()-20, m_Rect.Height()-80+pBmp_button_normal->height);
	rtSub1.left = rtSub1.right;

	for(i=0; i<3; i++)
	{
		rtSub1.left -= szSubPgNameLen[i]+10;
	}
	
	rtSub1.left -= 10;
	//rtSub1.left += 40;
	
	 int ReLeft = rtSub1.left;
	
	for(i=0; i<3; i++)
	{
		 int btWidth = szSubPgNameLen[i]+10;
		rtSub1.right = rtSub1.left + btWidth;
		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageDailyPictureConfig::OnClickBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
		rtSub1.left = rtSub1.right+5;
	}
	
	//CRect rtSub2(13, 35, m_Rect.Width()-18, 300);
	//pTable = CreateTableBox(&rtSub2,this, 1,1,0);
	

#ifndef BTN_RZ_STYLE	
	CRect staRect(20, m_Rect.Height()-82, ReLeft - 2/*25+TEXT_WIDTH*10*/, m_Rect.Height()-52);//shixin
	
	pStatus = CreateStatic(staRect, this, "");
	 //pStatus->SetBkColor(VD_RGB(67,77,87));
	 //pStatus->SetTextAlign(VD_TA_CENTER);
	 
	 rtSub1.left = 37;
	 
	 for(i=3; i<4; i++)
	{
		 //printf("***%s\n", __FUNCTION__);
		 //int btWidth = szSubPgNameLen[i];
		rtSub1.right = rtSub1.left + 72;//btWidth;
		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageDailyPictureConfig::OnClickBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
		rtSub1.left = rtSub1.right+12;
	}
#endif	
		InitPageDailyPictureConfig();


}

void CPageDailyPictureConfig::InitPageDailyPictureConfig()
{
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
	m_Rect.Width()-27,m_Rect.top+50+270);
	
	pTable3 = CreateTableBox(&rtTmp,this, 1,9,0);
	pTable3->SetColWidth(0,250);
	//items[3].SubItems[0].push_back(pTable3);//

	const char* szStatic[4] = 
	{
		"&CfgPtn.Time1",
		"&CfgPtn.IPChannel",
		"&CfgPtn.Time2",
		"&CfgPtn.IPChannel",
		
	};

	const char* chnnumber[16] = {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16",};
	
	CRect tmpRt;
	int i = 0;
	int j = 0;
	
	//时间1
	pTable3->GetTableRect(0, 0, &tmpRt);
	pStatic3[0] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[0]);

	pTable3->GetTableRect(0,i,&tmpRt);
	pCheckBoxTime[0] = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+100, rtTmp.top+tmpRt.top+3, 
		rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
		this,styleEditable, (CTRLPROC)&CPageDailyPictureConfig::OnCheckBox);
	pCheckBoxTime[0]->SetValue(FALSE);

	//pTable3->GetTableRect(0,0,&tmpRt);
	pDateTime[0] = CreateDateTimeCtrl(CRect(rtTmp.left+tmpRt.left+2+300, rtTmp.top+tmpRt.top+3, 
						    rtTmp.left+tmpRt.left+126+300,rtTmp.top+tmpRt.top+27),
						this,(CTRLPROC)&CPageDailyPictureConfig::OnDateTimeChange1,DTS_TIME);
	pDateTime[0]->SetBkColor(VD_RGB(67,77,87));  

	//IP通道1
	pTable3->GetTableRect(0, 1, &tmpRt);
	pStatic3[1] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[1]);

	//pTable3->GetTableRect(0,i,&tmpRt);
	pCheckBoxChnAll[0] = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+100, rtTmp.top+tmpRt.top+3, 
		rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
		this,styleEditable, (CTRLPROC)&CPageDailyPictureConfig::OnCheckBox);
	
	pCheckBoxChnAll[0]->SetValue(FALSE);

	for(j=0;j<8;j++)
	{
		pTable3->GetTableRect(0, 2, &tmpRt);
		pCheckBoxChn[0][j] = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+50*(j+1)+50, rtTmp.top+tmpRt.top+3, 
			rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
			this,styleEditable, (CTRLPROC)&CPageDailyPictureConfig::OnCheckBox);
		pCheckBoxChn[0][j]->SetValue(FALSE);

		//pTable3->GetTableRect(0,i,&tmpRt);
		pStatic1[j] = CreateStatic(CRect(rtTmp.left+tmpRt.left+50*(j+1)+30+50, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, chnnumber[j]);

	}	
	for(j=8;j<16;j++)
	{
		pTable3->GetTableRect(0, 3, &tmpRt);
		pCheckBoxChn[0][j] = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+50*(j-7)+50, rtTmp.top+tmpRt.top+3, 
			rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
			this,styleEditable, (CTRLPROC)&CPageDailyPictureConfig::OnCheckBox);
		pCheckBoxChn[0][j]->SetValue(FALSE);

		//pTable3->GetTableRect(0,i,&tmpRt);
		pStatic1[j] = CreateStatic(CRect(rtTmp.left+tmpRt.left+50*(j-7)+30+50, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, chnnumber[j]);

	}

	//时间2
	pTable3->GetTableRect(0, 5, &tmpRt);
	pStatic3[2] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[2]);

	//pTable3->GetTableRect(0,i,&tmpRt);
	pCheckBoxTime[1] = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+100, rtTmp.top+tmpRt.top+3, 
		rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
		this,styleEditable, (CTRLPROC)&CPageDailyPictureConfig::OnCheckBox);
	pCheckBoxTime[1]->SetValue(FALSE);

	//pTable3->GetTableRect(0,0,&tmpRt);
	pDateTime[1] = CreateDateTimeCtrl(CRect(rtTmp.left+tmpRt.left+2+300, rtTmp.top+tmpRt.top+3, 
						    rtTmp.left+tmpRt.left+126+300,rtTmp.top+tmpRt.top+27),
						this,(CTRLPROC)&CPageDailyPictureConfig::OnDateTimeChange1,DTS_TIME);
	pDateTime[1]->SetBkColor(VD_RGB(67,77,87));  

	//IP通道2
	pTable3->GetTableRect(0, 6, &tmpRt);
	pStatic3[3] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[3]);

	//pTable3->GetTableRect(0,i,&tmpRt);
	pCheckBoxChnAll[1] = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+100, rtTmp.top+tmpRt.top+3, 
		rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
		this,styleEditable, (CTRLPROC)&CPageDailyPictureConfig::OnCheckBox);
	
	pCheckBoxChnAll[1]->SetValue(FALSE);

	for(j=0;j<8;j++)
	{
		pTable3->GetTableRect(0, 7, &tmpRt);
		pCheckBoxChn[1][j] = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+50*(j+1)+50, rtTmp.top+tmpRt.top+3, 
			rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
			this,styleEditable, (CTRLPROC)&CPageDailyPictureConfig::OnCheckBox);
		pCheckBoxChn[1][j]->SetValue(FALSE);

		pStatic2[j] = CreateStatic(CRect(rtTmp.left+tmpRt.left+50*(j+1)+30+50, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, chnnumber[j]);

	}	
	for(j=8;j<16;j++)
	{
		pTable3->GetTableRect(0, 8, &tmpRt);
		pCheckBoxChn[1][j] = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+50*(j-7)+50, rtTmp.top+tmpRt.top+3, 
			rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
			this,styleEditable, (CTRLPROC)&CPageDailyPictureConfig::OnCheckBox);
		pCheckBoxChn[1][j]->SetValue(FALSE);

		pStatic2[j] = CreateStatic(CRect(rtTmp.left+tmpRt.left+50*(j-7)+30+50, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, chnnumber[j]);

	}
}

void CPageDailyPictureConfig::OnCheckBox()
{

	int i = 0;
	BOOL bFind1 = FALSE;
	BOOL bFind2 = FALSE;
	CCheckBox* pFocusButton = (CCheckBox *)GetFocusItem();

	if (pFocusButton == pCheckBoxChnAll[0])
	{
		//int i=0;
		for(i=0;i<16;i++)
		{
			pCheckBoxChn[0][i]->SetValue(pCheckBoxChnAll[0]->GetValue());
		}
	}
	else if (pFocusButton == pCheckBoxChnAll[1])
	{
		//int i=0;
		for(i=0;i<16;i++)
		{
			pCheckBoxChn[1][i]->SetValue(pCheckBoxChnAll[1]->GetValue());
		}
	}
	else
	{

		for (i = 0; i < 16; i++)
		{
			if (pFocusButton == pCheckBoxChn[0][i])
			{
				bFind1 = TRUE;
				break;
			}

			if (pFocusButton == pCheckBoxChn[1][i])
			{
				bFind2 = TRUE;
				break;
			}
		}

		if(bFind1)
		{
			int flag = 0;

			for(i=0; i<16; i++)
			{
				if (pCheckBoxChn[0][i]->GetValue())
				{
					flag++;
				}
				
			}

			if (flag == 16) 
			{
				pCheckBoxChnAll[0]->SetValue(1);

			}
			else
			{
				pCheckBoxChnAll[0]->SetValue(0);
			}

		}
		else if(bFind2)
		{
			int flag = 0;

			for(i=0; i<16; i++)
			{
				if (pCheckBoxChn[1][i]->GetValue())
				{
					flag++;
				}
			}
			
			if (flag == 16) 
			{
				pCheckBoxChnAll[1]->SetValue(1);
			}
			else
			{
				pCheckBoxChnAll[1]->SetValue(0);
			}
		}
	}
}

void CPageDailyPictureConfig::OnDateTimeChange1()
{
	//nTimeChanged = 1;
}

void CPageDailyPictureConfig::OnClickBtn()
{
	int i = 0, j, ret;
	BOOL bFind = FALSE;
	SDailyPicCFG para;
	
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i=0; i<3; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	if (bFind)
	{
		switch (i)
		{
			case 0: //默认值
			{
				ret = bizData_GetSGDailyParamDefault(&para);
				if (ret == 0)
				{
					if (para.Time1.TimeEnable)
						pCheckBoxTime[0]->SetValue(1);
					else
						pCheckBoxTime[0]->SetValue(0);

					if (para.Time2.TimeEnable)
						pCheckBoxTime[1]->SetValue(1);
					else
						pCheckBoxTime[1]->SetValue(0);
					
					SYSTEM_TIME time;
					//csp modify 20131213
					//GetSysTime(&time);
					GetSysTime_TZ(&time);
					time.hour = para.Time1.hour;
					time.minute= para.Time1.min;
					time.second= para.Time1.sec;
					pDateTime[0]->SetDateTime(&time);
					time.hour = para.Time2.hour;
					time.minute= para.Time2.min;
					time.second= para.Time2.sec;
					pDateTime[1]->SetDateTime(&time);

					printf("para.SDailyPicParam.IpcChn1: 0x%x\n", para.IpcChn1);
					if (para.IpcChn1 == 0xffffffff)//all
					{
						pCheckBoxChnAll[0]->SetValue(1);
						for(i=0; i<16; i++)
						{
							pCheckBoxChn[0][i]->SetValue(1);
						}
					}
					else
					{
						pCheckBoxChnAll[0]->SetValue(0);
						for(i=0; i<16; i++)
						{
							pCheckBoxChn[0][i]->SetValue(para.IpcChn1 & (1<<i));
						}
					}

					printf("para.SDailyPicParam.IpcChn2: 0x%x\n", para.IpcChn2);
					if (para.IpcChn2 == 0xffffffff)//all
					{
						pCheckBoxChnAll[1]->SetValue(1);
						for(i=0; i<16; i++)
						{
							pCheckBoxChn[1][i]->SetValue(1);
						}
					}
					else
					{
						pCheckBoxChnAll[1]->SetValue(0);
						for(i=0; i<16; i++)
						{
							pCheckBoxChn[1][i]->SetValue(para.IpcChn2 & (1<<i));
						}
					}
				}
				else
				{
					printf("CPageDailyPictureConfig: OnClickBtn: bizData_GetSGDailyParamDefault failed\n");
				}
			} break;
			case 1: //应用
			{
				//参数保存
				SSG_MSG_TYPE msg;
				memset(&msg, 0, sizeof(msg));
				msg.type = EM_DVR_PARAM_SAVE;
				msg.chn = 0;
				strcpy(msg.note, GetParsedString("&CfgPtn.DayPicConfig"));
				strcat(msg.note, GetParsedString("&CfgPtn.ParamSave"));
				upload_sg(&msg);
				
				ret = bizData_GetSGDailyParam(&para);
				if (ret == 0)
				{
					if (pCheckBoxTime[0]->GetValue())
						para.Time1.TimeEnable = 1;
					else
						para.Time1.TimeEnable = 0;

					if (pCheckBoxTime[1]->GetValue())
						para.Time2.TimeEnable = 1;
					else
						para.Time2.TimeEnable = 0;
					
					SYSTEM_TIME time;
					/*
					//GetSysTime(&time);
					GetSysTime_TZ(&time);
					time.hour = para.SDailyPicParam.Time1.hour;
					time.minute= para.SDailyPicParam.Time1.min;
					time.second= para.SDailyPicParam.Time1.sec;
					pDateTime[0]->SetDateTime(&time);
					time.hour = para.SDailyPicParam.Time2.hour;
					time.minute= para.SDailyPicParam.Time2.min;
					time.second= para.SDailyPicParam.Time2.sec;
					pDateTime[1]->SetDateTime(&time);
					*/
					pDateTime[0]->GetDateTime(&time);
					para.Time1.hour = time.hour;
					para.Time1.min = time.minute;
					para.Time1.sec = time.second;
					
					pDateTime[1]->GetDateTime(&time);
					para.Time2.hour = time.hour;
					para.Time2.min = time.minute;
					para.Time2.sec = time.second;

					para.IpcChn1 = 0;
					if (pCheckBoxChnAll[0]->GetValue())//all
					{
						para.IpcChn1 = 0xffffffff;
					}
					else
					{
						for(i=0; i<16; i++)
						{
							if (pCheckBoxChn[0][i]->GetValue())
								para.IpcChn1 |= 1<<i;
						}
					}
					printf("para.SDailyPicParam.IpcChn1: 0x%x\n", para.IpcChn1);

					para.IpcChn2 = 0;
					if (pCheckBoxChnAll[1]->GetValue())//all
					{
						para.IpcChn2 = 0xffffffff;
					}
					else
					{
						for(i=0; i<16; i++)
						{
							if (pCheckBoxChn[1][i]->GetValue())
								para.IpcChn2 |= 1<<i;
						}
					}
					printf("para.SDailyPicParam.IpcChn2: 0x%x\n", para.IpcChn2);

					ret = bizData_SaveSGDailyParam(&para);
					if (ret != 0)
					{
						printf("CPageDailyPictureConfig: OnClickBtn: bizData_SaveSGDailyParam failed\n");
					}
				}
				else
				{
					printf("CPageDailyPictureConfig: OnClickBtn: bizData_GetSGDailyParam failed\n");
				}
			} break;
			case 2: //退出
			{
				this->Close();
			} break;
		}
	}
}


CPageDailyPictureConfig::~CPageDailyPictureConfig()
{

}	


VD_PCSTR CPageDailyPictureConfig::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}







	

