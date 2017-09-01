#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageRecordFrameWork.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/PageOSDPosSet.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageDesktop.h"  //cw_panel
#include "GUI/Pages/PageConfigFrameWork.h"
#include "Biz.h"
#include "sg_platform.h"


#undef DISABLE_PREREC_TIME

static	std::vector<CItem*>	items[RECCONFIG_SUBPAGES];
static std::vector<int> bModify;


static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static VD_BITMAP * pBmp_tab_normal;
static VD_BITMAP * pBmp_tab_push;
static VD_BITMAP * pBmp_tab_select;

static VD_BITMAP * pBmp_infoicon;

static int GetMaxChnNum()
{
	return GetVideoMainNum();
}

static int GetAudioChnNum()
{
	return GetAudioNum();
}

static int GetEnableRows()
{
	return GetMaxChnNum()>REC_ENABLE_ROWS?REC_ENABLE_ROWS:GetMaxChnNum();
}

static int GetEnableAudioRows()
{
	//return GetMaxChnNum()>REC_ENABLE_ROWS?REC_ENABLE_ROWS:GetMaxChnNum();
	return GetAudioChnNum()>REC_ENABLE_ROWS?REC_ENABLE_ROWS:GetAudioChnNum();
}

static int GetStreamRows()
{
	return GetMaxChnNum()>REC_STREAM_ROWS?REC_STREAM_ROWS:GetMaxChnNum();
}


static int GetRecTimeRows()
{
	return GetMaxChnNum()>REC_TIME_ROWS?REC_TIME_ROWS:GetMaxChnNum();
}

static int GetOsdRows()
{
	return GetMaxChnNum()>REC_OSD_ROWS?REC_OSD_ROWS:GetMaxChnNum();
}

CPageRecordFrameWork::CPageRecordFrameWork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
, curID(0),chnNum(4)
{
	audio_page = (GetAudioChnNum()%REC_ENABLE_ROWS)?(GetAudioChnNum()/REC_ENABLE_ROWS+1):(GetAudioChnNum()/REC_ENABLE_ROWS);
	
	//debug_trace_err("CPageOSDPosSet");
	
	//SetRect(CRect(0, 0, 700, 400/*370*/), FALSE);
	SetMargin(3, m_TitleHeight, 4, m_TitleHeight);
	
	//debug_trace_err("CPageOSDPosSet");
	
	for(int i=0; i<GetMaxChnNum(); i++)
	{
		bModify.push_back(0);
	}

	//pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/button_normal.bmp");
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//debug_trace_err("CPageOSDPosSet");
	
	//pBmp_button_push = VD_LoadBitmap(DATA_DIR"/button_push.bmp");
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//debug_trace_err("CPageOSDPosSet");
	
	//pBmp_button_select = VD_LoadBitmap(DATA_DIR"/button_select.bmp");
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//debug_trace_err("CPageOSDPosSet");
	
	//第一个页面的子页面控制按钮  录像设置，手动录像，录像布防
	pBmp_tab_normal = VD_LoadBitmap(DATA_DIR"/temp/tab.bmp");
	pBmp_tab_push = VD_LoadBitmap(DATA_DIR"/temp/tab_check.bmp");
	pBmp_tab_select = VD_LoadBitmap(DATA_DIR"/temp/tab_f.bmp");
	char* szSubPgName[RECCONFIG_BTNNUM+2] = {
		"&CfgPtn.Enable",
		"&CfgPtn.RecordBitrate",
		"&CfgPtn.RecTime",
		"&CfgPtn.Stamp",
		"&CfgPtn.RecycleRecord",
		"&CfgPtn.SnapParam",
		"&CfgPtn.Default",
		"&CfgPtn.apply",
		"&CfgPtn.Exit",
		"&CfgPtn.Prev",
		"&CfgPtn.Next",
	};	

	int szSubPgStrLen[RECCONFIG_BTNNUM+2] = {
		TEXT_WIDTH*2,//"&CfgPtn.Enable",
		TEXT_WIDTH*4,//"&CfgPtn.RecordBitrate",
		TEXT_WIDTH*4,//"&CfgPtn.Time",
		TEXT_WIDTH*4,//"&CfgPtn.Stamp",
		TEXT_WIDTH*4,//"&CfgPtn.RecycleRecord",
		TEXT_WIDTH*4,//"&CfgPtn.SnapParam",
		TEXT_WIDTH*3,//"&CfgPtn.Default",
		TEXT_WIDTH*2,//"&CfgPtn.apply",
		TEXT_WIDTH*2,//"&CfgPtn.Exit",
		TEXT_WIDTH*3,
		TEXT_WIDTH*3,
	};

	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);

	memset(pStatic0,0,sizeof(pStatic0));
	memset(pCheckBox0,0,sizeof(pCheckBox0));
	memset(pStatic1,0,sizeof(pStatic1));
	memset(pComboBox1,0,sizeof(pComboBox1));
	memset(pStatic2,0,sizeof(pStatic2));
	memset(pComboBox2,0,sizeof(pComboBox2));
	memset(pStatic3,0,sizeof(pStatic3));
	memset(pCheckBox3,0,sizeof(pCheckBox3));
	memset(pButton3,0,sizeof(pButton3));
	
	CTableBox* pTabFrame = CreateTableBox(CRect(17, 40, m_Rect.Width()-17, 40+290),this,1,1);
	//pTabFrame->SetFrameColor(VD_RGB(56,108,148));
	pTabFrame->SetFrameColor(VD_RGB(8,66,189));
	
	CRect rtSub(25, 10, m_Rect.Width()-20, 10+pBmp_tab_normal->height);
	int i;
	for (i = 0; i<RECCONFIG_SUBPAGES; i++)
	{
//yaogang modify 20141106
#ifdef DISABLE_RECBITRATE 
		if(i == 1)
		{
			continue;
		}
#endif

#ifdef DISABLE_PREREC_TIME 
		if(i == 2)
		{
			continue;
		}
#endif
//yaogang modify 20141106
#ifdef DISABLE_STAMP
		if(i == 3)
		{
			continue;
		}
#endif

#ifdef NO_MODULE_SG
		//yaogang modify 20150104
		if(i == RECCONFIG_SUBPAGES-1 )//SnapParam
		{
			continue;
		}
#endif		
		int btWidth = szSubPgStrLen[i]+10;
		
		rtSub.right = rtSub.left + btWidth;
		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageRecordFrameWork::OnClickSubPage, NULL, buttonNormalBmp, 1);
		pButton[i]->SetBitmap(pBmp_tab_normal, pBmp_tab_select, pBmp_tab_push, pBmp_tab_push);
		rtSub.left = rtSub.right;
	}
	

	CRect rtSub1(295, m_Rect.Height()-80, m_Rect.Width()-20, m_Rect.Height()-80+pBmp_button_normal->height);

	pInfoBar = CreateStatic(CRect(40, 
								rtSub1.top,
								317,
								rtSub1.top+22), 
							this, 
							"");
	//pInfoBar->SetBkColor(VD_RGB(67,77,87));
	 //pInfoBar->SetTextAlign(VD_TA_CENTER);

	pBmp_infoicon = VD_LoadBitmap(DATA_DIR"/temp/icon_info.bmp");
	pIconInfo = CreateStatic(CRect(17,
								rtSub1.top+1,
								37,
								rtSub1.top+21),
								this,
								"");
	pIconInfo->SetBitmap(pBmp_infoicon,NULL);
	pIconInfo->Show(FALSE);

	rtSub1.left = rtSub1.right;
	for (i=RECCONFIG_SUBPAGES; i<RECCONFIG_BTNNUM; i++)
	{
		rtSub1.left -= szSubPgStrLen[i]+10;
	}
	rtSub1.left -= 12*2;
    
	for(i=RECCONFIG_SUBPAGES; i<RECCONFIG_BTNNUM; i++)
	{
		int btWidth = szSubPgStrLen[i]+10;
		rtSub1.right = rtSub1.left + btWidth;
		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageRecordFrameWork::OnClickSubPage, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
		rtSub1.left = rtSub1.right+12;
	}
	
    rtSub1.left = 37;
	for(i = RECCONFIG_BTNNUM; i < RECCONFIG_BTNNUM+2; i++)
	{
        //printf("***%s\n", __FUNCTION__);
		//int btWidth = szSubPgStrLen[i]+10;
		rtSub1.right = rtSub1.left + 72;//btWidth;
		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageRecordFrameWork::OnClickSubPage, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
		rtSub1.left = rtSub1.right+12;
	}
    
	//CRect rtSub2(13, 35, m_Rect.Width()-18, 300);
	//pTable = CreateTableBox(&rtSub2,this, 1,1,0);
	
	m_pOSDSet = new CPageOSDPosSet(NULL);
	
	debug_trace_err("InitPage0");
	
	InitPage0();	//启用
	debug_trace_err("InitPage1");
	InitPage1();	//码流
	debug_trace_err("InitPage2");
	InitPage2();	//录像时间
	debug_trace_err("InitPage3");
	InitPage3();	//字符叠加
	debug_trace_err("InitPage4");
	InitPage4();	//循环录像
	debug_trace_err("InitPage5");
	InitPage5();	//抓图参数
	debug_trace_err("InitPage6");
	
	pTmpCombo = CreateComboBox(CRect(0,0,0,0),this,NULL);
	for(int i=0; i<20; i++)
	{
		pTmpCombo->AddString("1");
	}
	
	pTmpCheck = CreateCheckBox(CRect(0,0,0,0),this);
	
	pTmpCombo->Show(FALSE);
	pTmpCheck->Show(FALSE);
	
    m_IsLoader = 0;
}

void CPageRecordFrameWork::OnClickSubPage()
{
	//printf("OnClickSubPage\n");
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
    
	for(i = 0; i < RECCONFIG_BTNNUM+2; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		//printf("find the focus button\n");
		switch(i)
		{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			{
				//printf("SwitchPage(%d)\n",i);
				SwitchPage(i);
				//printf("LoadTabData(%d)\n",curID);
				LoadTabData(curID);
			}break;
			case 6://默认值
			{
				switch(curID)
				{
					case 0://enable
					{
						int ret;
						for(i=0; i<GetMaxChnNum(); i++)
						{
							int value = GetRecordEnableDefault(i);
							bVideoEnable[i] = value;

							if(i < GetAudioChnNum())
							{
								ret = GetAudioEnableDefault(i);
								bAudioEnable[i] = ret;
							}
						}
						
						for(i=0; i<GetEnableRows(); i++)
						{
							int nCh = m_page0*REC_ENABLE_ROWS+i;
							if(nCh >= GetMaxChnNum())
							{
								break;
							}
							
							pCheckBox0[i]->SetValue(bVideoEnable[nCh]);
							//pCheckBox0[i+REC_ENABLE_ROWS]->SetValue(bAudioEnable[nCh]);
						}
						
						for(i=0; i<GetEnableAudioRows(); i++)
						{	
							int nCh = m_page0*REC_ENABLE_ROWS+i;
							if(nCh >= GetAudioChnNum())
							{
								break;
							}
							
							//pCheckBox0[i]->SetValue(bVideoEnable[nCh]);
							pCheckBox0[i+REC_ENABLE_ROWS]->SetValue(bAudioEnable[nCh]);
						}
						
						if(ret >= 0)
						{
							SetInfo("&CfgPtn.LoadDefaultSuccess");
						}
						else
						{
							SetInfo("&CfgPtn.LoadDefaultFail");
						}
					}break;
					case 1://code stream
					{
						//csp modify 20130429
						pCheckBox1->SetValue(FALSE);
						
						int ret = 0;
						int index = 0;
						for(int i=0; i<GetMaxChnNum(); i++)
						{
							#if 0//csp modify
							index = GetReso(i);
							#else
							index = GetResoDefault(i);
							#endif
							resolution[i] = index;
							
							index =	GetFramerateDefault(index,i);
							framerate[i] = index;
							
							index = GetEncodeDefault(i);
							encode[i] = index;
							
							if(index == 0)
							{
								//CBR
								qualityEnable[i] = 0;
								quality[i] = 0;
							}
							else
							{
								//VBR
								qualityEnable[i] = 1;
								index = GetQuality(i);
								quality[i] = index;
							}
							
							ret = GetBitrateDefault(i);
							bitrate[i] = ret;
						}
						
						SValue frameRateList[10];
						int nFrReal;
						int count = 0;
						
						for(int i=0; i<GetStreamRows(); i++)
						{
							int nCh = m_page1*REC_STREAM_ROWS+i;
							if(nCh >= GetMaxChnNum())
							{
								break;
							}
							
							pComboBox1[i]->SetCurSel(resolution[nCh]);
							
							int index = pComboBox1[i]->GetCurSel();
							GetFrameRateList(index, frameRateList, &nFrReal, 10);
							pComboBox1[i+REC_STREAM_ROWS]->RemoveAll();
							for(int j=0; j<nFrReal; j++)
							{
								pComboBox1[i+REC_STREAM_ROWS]->AddString(frameRateList[j].strDisplay);
							}
                            
                            #if 1
                            //add by Lirl on Nov/17/2011,帧率选择最后一项
                            count = pComboBox1[i+REC_STREAM_ROWS]->GetCount();
                            pComboBox1[i+REC_STREAM_ROWS]->SetCurSel(count-1);
                            //end
                            #else
							if(0 == resolution[nCh])
							{
								pComboBox1[i+REC_STREAM_ROWS]->SetCurSel(5);
							}
							else if(1 == resolution[nCh])
							{
								pComboBox1[i+REC_STREAM_ROWS]->SetCurSel(8);
							}
                            #endif
							
							pComboBox1[i+REC_STREAM_ROWS*2]->SetCurSel(encode[nCh]);
							pComboBox1[i+REC_STREAM_ROWS*3]->SetCurSel(quality[nCh]);
							pComboBox1[i+REC_STREAM_ROWS*3]->Enable(qualityEnable[nCh]);
							pComboBox1[i+REC_STREAM_ROWS*4]->SetCurSel(bitrate[nCh]);							
						}
						
                        //add by Lirl on Nov/17/2011,默认都选择最后一项
                        for(int i=0; i<GetMaxChnNum(); i++)
                        {
                            framerate[i] = count-1;
                        }
                        //end
						
						//CheckFrameLeft();
						
						if(ret >= 0)
						{
							SetInfo("&CfgPtn.LoadDefaultSuccess");
						}
						else
						{
							
							SetInfo("&CfgPtn.LoadDefaultFail");
						}
					}break;
					case 2://record time
					{
						int index = 0;
						for(int i=0; i<GetMaxChnNum(); i++)
						{
							//printf("GetTimeBeforeAlarm\n");
							index = GetTimeBeforeAlarmDefault(i);
							preRecTime[i] = index;
							
							index = GetTimeAfterAlarmDefault(i);
							delayRecTime[i] = index;
							
							index = GetExpirationDefault(i);
							recExpireTime[i] = index;
						}
						
						for(int i=0; i<GetRecTimeRows(); i++)
						{
							int nCh = m_page2*REC_TIME_ROWS+i;
							if(nCh >= GetMaxChnNum())
							{
								break;
							}
							
							pComboBox2[i]->SetCurSel(preRecTime[nCh]);
							pComboBox2[i+REC_TIME_ROWS]->SetCurSel(delayRecTime[nCh]);
							pComboBox2[i+REC_TIME_ROWS*2]->SetCurSel(recExpireTime[nCh]);
						}
						
						int ret = GetTimeBeforeAlarmDefault(0);
						pComboBox2[REC_TIME_ROWS*3]->SetCurSel(ret);
						
						index = GetTimeAfterAlarmDefault(0);
						pComboBox2[REC_TIME_ROWS*3+1]->SetCurSel(index);
						
						index = GetExpirationDefault(0);
						pComboBox2[REC_TIME_ROWS*3+2]->SetCurSel(index);
						
						if(ret >= 0)
						{
							SetInfo("&CfgPtn.LoadDefaultSuccess");
						}
						else
						{
							
							SetInfo("&CfgPtn.LoadDefaultFail");
						}
					}break;
					case 3://osd
					{
						int value = 0;
						int ret = 0;
						for(int i=0; i<GetMaxChnNum(); i++)
						{
							ret = GetChnNameCheckDefault(i);
							bChNameEnable[i] = ret;
							//pCheckBox3[i]->SetValue(ret);

							value = GetTimeStampCheckDefault(i);
							bTimeEnable[i] = value;
							//pCheckBox3[i+4]->SetValue(value);
						}

						for(int i=0; i<GetOsdRows(); i++)
						{
							int nCh = m_page3*REC_OSD_ROWS+i;
							if(nCh >= GetMaxChnNum())
							{
								break;
							}

							pCheckBox3[i]->SetValue(bChNameEnable[nCh]);
							pCheckBox3[i+REC_OSD_ROWS]->SetValue(bTimeEnable[nCh]);
							pButton3[i]->Enable(bChNameEnable[nCh] && bTimeEnable[nCh]);
						}

						if(ret >= 0)
						{
							SetInfo("&CfgPtn.LoadDefaultSuccess");
						}
						else
						{
							
							SetInfo("&CfgPtn.LoadDefaultFail");
						}
					}break;
					case 4://cyling record
					{	
						int ret = GetCyclingRecCheckDefault();
						pCheckBox4[0]->SetValue(ret);
							
						if(ret >= 0)
						{
							SetInfo("&CfgPtn.LoadDefaultSuccess");
						}
						else
						{
							
							SetInfo("&CfgPtn.LoadDefaultFail");
						}
					}break;
					case 5://Snap default value
					{
						SGuiSnapChnPara SnapPara;
						int chn = pComboBox50->GetCurSel();
						VD_BOOL flag;

						if (chn < GetMaxChnNum())
						{
							if (bizData_GetSnapChnParaDefault(chn, &SnapPara) != 0)
							{
								printf("%s: bizData_GetSnapChnParaDefault() failed, chn%d\n", __FUNCTION__, chn);
							}
							else
							{
								//printf("TimeSnap: enable: %d, Res: %d, Quality: %d, Interval: %d\n", 
								//	SnapPara.TimeSnap.Enable, SnapPara.TimeSnap.Res,
								//	SnapPara.TimeSnap.Quality, SnapPara.TimeSnap.Interval);
								
								flag = SnapPara.TimeSnap.Enable ? TRUE:FALSE;
								pCheckBox5[0]->SetValue(SnapPara.TimeSnap.Enable);
								pComboBox5[0][0]->Enable(flag);
								pComboBox5[0][1]->Enable(flag);
								pComboBox5[0][2]->Enable(flag);
								pComboBox5[0][0]->SetCurSel(SnapPara.TimeSnap.Res);
								pComboBox5[0][1]->SetCurSel(SnapPara.TimeSnap.Quality);
								pComboBox5[0][2]->SetCurSel(SnapPara.TimeSnap.Interval);

								//printf("EventSnap: enable: %d, Res: %d, Quality: %d, Interval: %d\n", 
								//	SnapPara.EventSnap.Enable, SnapPara.EventSnap.Res,
								//	SnapPara.EventSnap.Quality, SnapPara.EventSnap.Interval);
								
								flag = SnapPara.EventSnap.Enable ? TRUE:FALSE;
								pCheckBox5[1]->SetValue(SnapPara.EventSnap.Enable);
								pComboBox5[1][0]->Enable(flag);
								pComboBox5[1][1]->Enable(flag);
								pComboBox5[1][2]->Enable(flag);
								pComboBox5[1][0]->SetCurSel(SnapPara.EventSnap.Res);
								pComboBox5[1][1]->SetCurSel(SnapPara.EventSnap.Quality);
								pComboBox5[1][2]->SetCurSel(SnapPara.EventSnap.Interval);
							}
						}
						else //all
						{
							pComboBox5[0][0]->Enable(FALSE);
							pComboBox5[0][1]->Enable(FALSE);
							pComboBox5[0][2]->Enable(FALSE);
							pComboBox5[0][0]->SetCurSel(2);//352*288
							pComboBox5[0][1]->SetCurSel(1);//中
							pComboBox5[0][2]->SetCurSel(4);//5s

							pComboBox5[1][0]->Enable(FALSE);
							pComboBox5[1][1]->Enable(FALSE);
							pComboBox5[1][2]->Enable(FALSE);
							pComboBox5[1][0]->SetCurSel(2);
							pComboBox5[1][1]->SetCurSel(1);
							pComboBox5[1][2]->SetCurSel(4);
						}
					}break;
				}
			}break;
			case 7://应用
			{
				if(curID)
					WriteLogs(curID);
				
				//printf("file:%s, cur page:%d ,apply\n",__FILE__, curID);
				
				if(0 == curID) //启用子页面
				{	
					//启用配置参数保存
					SSG_MSG_TYPE msg;
					memset(&msg, 0, sizeof(msg));
					msg.type = EM_DVR_PARAM_SAVE;
					msg.chn = 0;
					strcpy(msg.note, GetParsedString("&CfgPtn.Record"));
					strcat(msg.note, GetParsedString("&CfgPtn.ParamSave"));
					upload_sg(&msg);

					int i = 0;
					int bAll = pCheckBox0[2*REC_ENABLE_ROWS]->GetValue();
					if(bAll == TRUE)
					{
						int allRec = pCheckBox0[2*REC_ENABLE_ROWS+1]->GetValue();
						int allAudio = pCheckBox0[2*REC_ENABLE_ROWS+2]->GetValue();
						
						for(i=0; i<GetMaxChnNum(); i++)
						{
							bVideoEnable[i] = allRec;
							//9624 Support four road audio coding
							if(i < GetAudioChnNum())
							{
								bAudioEnable[i] = allAudio;//9624
							}
						}
					}
					
					{
						for(i=0; i<GetAudioChnNum(); i++)
						{
							if(GetCurRecStatus(i) && GetRecordEnable(i) && bVideoEnable[i] && (GetAudioEnable(i) != bAudioEnable[i]))
							{								
								break;
							}
						}
						if(i<GetAudioChnNum())
						{
							UDM ret = MessageBox("&CfgPtn.ModifyAudioRecording", "&CfgPtn.WARNING" , MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2);
							if(UDM_OK != GetRunStatue())
							{
								break;
							}
						}
					}
					
					WriteLogs(curID);
					
					for(i=0; i<GetEnableRows(); i++)
					{		
						int nCh = m_page0*REC_ENABLE_ROWS+i;
						if(nCh >= GetMaxChnNum())
						{
							break;
						}
						
						pCheckBox0[i]->SetValue(bVideoEnable[nCh]);
						//pCheckBox0[i+REC_ENABLE_ROWS]->SetValue(bAudioEnable[nCh]);
					}
					
					for(i=0; i<GetEnableAudioRows(); i++)
					{
						int nCh = m_page0*REC_ENABLE_ROWS+i;
						if(nCh >= GetAudioChnNum())
						{
							break;
						}
						
						pCheckBox0[i+REC_ENABLE_ROWS]->SetValue(bAudioEnable[nCh]);
					}
					
					
					u32 ret_rec = 0;
					u32 ret_aud = 0;
					for(i=0; i<GetMaxChnNum(); i++)
					{
						//getchar();
						//printf("file:%s, line:%d, i:%d\n", __FILE__, __LINE__, i);
						pTmpCheck->SetValue(bVideoEnable[i]);
						ret_rec |= SaveScrStr2Cfg( pTmpCheck,
								GSR_CONFIG_RECORD_ENABLE_RECORD, 
								EM_GSR_CTRL_CHK, 
								i
							    );
					}
					
					for(i=GetMaxChnNum(); i < (GetMaxChnNum() + GetAudioChnNum()); i++)
					{
						pTmpCheck->SetValue(bAudioEnable[i-GetMaxChnNum()]);
						ret_aud |= SaveScrStr2Cfg( pTmpCheck,
								GSR_CONFIG_RECORD_ENABLE_AUDIO, 
								EM_GSR_CTRL_CHK, 
								i-GetMaxChnNum()
							    );
					}
					
					if((ret_aud == 0) && (ret_rec== 0))
					{
						SetInfo("&CfgPtn.SaveConfigSuccess");
					}
					else
					{
						SetInfo("&CfgPtn.SaveConfigFail");
					}
				}
				else if(1 == curID)//录像码流子页面
				{
					int i = 0;
					int j = 0;
					int bAll = pCheckBox1->GetValue();
					if(bAll == TRUE)
					{
						for(i=0; i<REC_STREAM_COLS-1; i++)
						{
							int index = pComboBox1[i+REC_STREAM_ROWS*5]->GetCurSel();
							
							switch(i)
							{
								#if 1//csp modify
								case 0:
									for(int k=0; k<GetMaxChnNum(); k++)
									{
										//csp modify 20130504
										if(VerifyReso(k,index) != 0)
										{
											continue;
										}
										
										resolution[k] = index;
									}
									break;
								#endif
								case 1:
									for(int k=0; k<GetMaxChnNum(); k++)
									{
										framerate[k] = index;
									}
									break;
								case 2:
									for(int k=0; k<GetMaxChnNum(); k++)
									{
										encode[k] = index;
										
										if(encode[k] == 0)
										{
											//CBR
											qualityEnable[k] = 0;
										}
										else
										{
											//VBR
											qualityEnable[k] = 1;
										}
									}
									break;
								case 3:
									for(int k=0; k<GetMaxChnNum(); k++)
									{
										quality[k] = index;
									}
									break;
								case 4:
									for(int k=0; k<GetMaxChnNum(); k++)
									{
										bitrate[k] = index;
									}
									break;
								default:
									break;
							}
						}
					}
                    
					{
						for(i=0; i<GetMaxChnNum(); i++)
						{
							if(GetCurRecStatus(i) && (GetReso(i) != resolution[i]))
							{
								break;
							}
						}
                        
						if(i<GetMaxChnNum())
						{
                            #if 0
                            char tmp[30] = {0};
		                    GetProductNumber(tmp);
                            if ((0 == strcasecmp(tmp, "R9504S"))
                            	|| (0 == strcasecmp(tmp, "R9508S")))
                            {
                                MessageBox("&CfgPtn.ModifyProhibition", "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
                                this->UpdateData(UDM_OPEN);
                                return;
                            }
                            else
                            #endif
                            {
                             	UDM ret = MessageBox("&CfgPtn.ModifyResolutionRecording", "&CfgPtn.WARNING" , MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2);
    							
     							if(UDM_OK != GetRunStatue())
								{
    								break;
    							}
                            }
						}
					}
					
					if(bAll == TRUE)
					{
						SValue frameRateList[10];
						int nFrReal;
						for(i=0; i<GetStreamRows(); i++)
						{
							int nCh = m_page1*REC_STREAM_ROWS+i;
							if(nCh >= GetMaxChnNum())
							{
								break;
							}
							
							pComboBox1[i]->SetCurSel(resolution[nCh]);
							
							int index = pComboBox1[i]->GetCurSel();
							GetFrameRateList(index, frameRateList, &nFrReal, 10);
							pComboBox1[i+REC_STREAM_ROWS]->RemoveAll();
							for(j=0; j<nFrReal; j++)
							{
								pComboBox1[i+REC_STREAM_ROWS]->AddString(frameRateList[j].strDisplay);
							}
							
							pComboBox1[i+REC_STREAM_ROWS]->SetCurSel(framerate[nCh]);
							pComboBox1[i+REC_STREAM_ROWS*2]->SetCurSel(encode[nCh]);
							pComboBox1[i+REC_STREAM_ROWS*3]->SetCurSel(quality[nCh]);
							pComboBox1[i+REC_STREAM_ROWS*3]->Enable(qualityEnable[nCh]);
							pComboBox1[i+REC_STREAM_ROWS*4]->SetCurSel(bitrate[nCh]);
						}
					}
					
					#if 0
					for(int i=0; i<GetMaxChnNum(); i++)
					{
						printf("resolution[%d] = %d \n",i,resolution[i]);
						printf("framerate[%d] = %d \n",i,framerate[i]);
						printf("encode[%d] = %d \n",i,encode[i]);
						printf("quality[%d] = %d \n",i,quality[i]);
						printf("bitrate[%d] = %d \n",i,bitrate[i]);
					}
					#endif
					
					//CheckFrameLeft();
					
					int id1[5] = 
					{
						GSR_CONFIG_RECORD_ENABLE_CODESTREAM_RESOLUTION,					
						GSR_CONFIG_RECORD_ENABLE_CODESTREAM_FRAMERATE,				
						GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ENCODE,				
						GSR_CONFIG_RECORD_ENABLE_CODESTREAM_QUALITY,					
						GSR_CONFIG_RECORD_ENABLE_CODESTREAM_MAXRATE,	
					};
					
					int ret = 0;
					
					for(j=0; j<GetMaxChnNum(); j++)
					{		
						for(i=0; i<REC_STREAM_COLS-1; i++)
						{
							switch(i)
							{
								case 0:
									pTmpCombo->SetCurSel(resolution[j]);
									break;
								case 1:
									pTmpCombo->SetCurSel(framerate[j]);
									break;
								case 2:
									pTmpCombo->SetCurSel(encode[j]);
									break;
								case 3:
									pTmpCombo->SetCurSel(quality[j]);
									break;
								case 4:
									pTmpCombo->SetCurSel(bitrate[j]);
									break;
								default:
									break;
							}
							
							//printf("i = %d,j = %d,pTmpCombo->GetCurSel() = %d \n",i,j,pTmpCombo->GetCurSel());
							//printf("i = %d,j = %d \n",i,j);
							//int start = GetTimeTick();
							ret = SaveScrStr2Cfg(pTmpCombo,
											id1[i], 
											EM_GSR_CTRL_COMB, 
											j
						    );
							//int end = GetTimeTick();
							//printf("i = %d,j = %d, end - start = %d \n",i,j,end-start);
						}
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
				else if(2 == curID) //录像时间子页面
				{
					//录像时间参数保存
					SSG_MSG_TYPE msg;
					memset(&msg, 0, sizeof(msg));
					msg.type = EM_DVR_PARAM_SAVE;
					msg.chn = 0;
					strcpy(msg.note, GetParsedString("&CfgPtn.RecTime"));
					strcat(msg.note, GetParsedString("&CfgPtn.ParamSave"));
					upload_sg(&msg);
					
					int i = 0;
					int j = 0;
					
					int bAll = pCheckBox2->GetValue();
					if(bAll == TRUE)
					{

						for(i=0; i<REC_TIME_COLS-1; i++)
						{
							int index = pComboBox2[REC_TIME_ROWS*3+i]->GetCurSel();
							for(j=0; j<GetMaxChnNum(); j++)
							{
							
								//pComboBox2[i*chnNum+j]->SetCurSel(index);
								switch(i)
								{
									case 0:
										preRecTime[j] = index;
										break;
									case 1:
										delayRecTime[j] = index;
										break;
									case 2:
										recExpireTime[j] = index;
										break;
									default:
										break;
								}

							}
						}
					}

					for(i=0; i<GetRecTimeRows(); i++)
					{
						int nCh = m_page2*REC_TIME_ROWS+i;
						if(nCh >= GetMaxChnNum())
						{
							break;
						}

						pComboBox2[i]->SetCurSel(preRecTime[nCh]);
						pComboBox2[i+REC_TIME_ROWS]->SetCurSel(delayRecTime[nCh]);
						pComboBox2[i+REC_TIME_ROWS*2]->SetCurSel(recExpireTime[nCh]);
					}

					#if 1
					int id1[3] = 
					{
						GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEBEFALARM,					
						GSR_CONFIG_RECORD_ENABLE_RECORDTIME_TIMEAFTALARM,				
						GSR_CONFIG_RECORD_ENABLE_RECORDTIME_EXPIRATION,				
					};

					int ret;
					for(i=0; i<REC_TIME_COLS-1; i++)
					{
					
						for(j=0; j<GetMaxChnNum(); j++)
						{
							
							switch(i)
							{
								case 0:
									pTmpCombo->SetCurSel(preRecTime[j]);
									break;
								case 1:
									pTmpCombo->SetCurSel(delayRecTime[j]);
									break;
								case 2:
									pTmpCombo->SetCurSel(recExpireTime[j]);
									break;
								default:
									break;
							}
							
							ret = SaveScrStr2Cfg(pTmpCombo ,
											id1[i], 
											EM_GSR_CTRL_COMB, 
											j
						    );

						}
					}

					if(ret == 0)
					{
						SetInfo("&CfgPtn.SaveConfigSuccess");
					}
					else
					{
						SetInfo("&CfgPtn.SaveConfigFail");
					}

					#endif

				}
				else if(3 == curID) //字符叠加子页面
				{
					int i = 0;
					int j = 0;
					
					int bAll = pCheckBox3[REC_OSD_ROWS*2]->GetValue();
					if(bAll == TRUE)
					{
						for(i=0; i<2; i++)
						{
							int allValue = pCheckBox3[REC_OSD_ROWS*2+1+i]->GetValue();
							for(j=0; j<GetMaxChnNum(); j++)
							{
							
								//pCheckBox3[i*chnNum+j]->SetValue(allValue);
								switch(i)
								{
									case 0:
										bChNameEnable[j] = allValue;
										break;
									case 1:
										bTimeEnable[j] = allValue;
										break;
									default:
										break;
								}

							}
						}
					}

					for(i=0; i<GetOsdRows(); i++)
					{
						int nCh = m_page3*REC_TIME_ROWS+i;
						if(nCh >= GetMaxChnNum())
						{
							break;
						}

						pCheckBox3[i]->SetValue(bChNameEnable[nCh]);
						pCheckBox3[i+REC_OSD_ROWS]->SetValue(bTimeEnable[nCh]);
                        			pButton3[i]->Enable(bChNameEnable[nCh] || bTimeEnable[nCh]);
					}

					int id1[2] = 
					{
						GSR_CONFIG_RECORD_ENABLE_OSD_CHNNAME,
						GSR_CONFIG_RECORD_ENABLE_OSD_TIMESTAMP,
					};

					int rtn;
					for(i=0; i<2; i++)
					{
						for(j=0; j<GetMaxChnNum(); j++)
						{
							switch(i)
							{
								case 0:
									pTmpCheck->SetValue(bChNameEnable[j]);
									break;
								case 1:
									pTmpCheck->SetValue(bTimeEnable[j]);
									break;
								default:
									break;
							}
										
							
							rtn = SaveScrStr2Cfg(pTmpCheck ,
											id1[i], 
											EM_GSR_CTRL_CHK, 
											j
						    );

						}
					}

					std::vector<CRect> chnOsd = m_pOSDSet->GetChnOsdRect();
					std::vector<CRect> timeOsd = m_pOSDSet->GetTimeOsdRect();

					SBizParaTarget bizTar;
					SBizCfgStrOsd bizStrOsd;
					bizTar.emBizParaType = EM_BIZ_STROSD;
						

					for(int i=0; i<GetMaxChnNum(); i++)
					{
						if(bModify[i])
						{
							printf("chn %d modify \n",i);
							bizTar.nChn = i;
							int ret = BizGetPara(&bizTar, &bizStrOsd);
	
							if(0==ret)
							{
								//strcpy(bizSysPara.strDevName, pStr);
	
								bizStrOsd.sEncChnNamePos.x = chnOsd[i].left;
								bizStrOsd.sEncChnNamePos.y = chnOsd[i].top;
	
								bizStrOsd.sEncTimePos.x = timeOsd[i].left;
								bizStrOsd.sEncTimePos.y = timeOsd[i].top;
								
								BizSetPara(&bizTar, &bizStrOsd);
							}

							bModify[i] = 0;
						}
					}

					if(rtn == 0)
					{
						SetInfo("&CfgPtn.SaveConfigSuccess");
					}
					else
					{
						SetInfo("&CfgPtn.SaveConfigFail");
					}

				}
				else if(4 == curID) //循环录像子页面
				{
					//循环录像参数保存
					SSG_MSG_TYPE msg;
					memset(&msg, 0, sizeof(msg));
					msg.type = EM_DVR_PARAM_SAVE;
					msg.chn = 0;
					strcpy(msg.note, GetParsedString("&CfgPtn.RecycleRecord"));
					strcat(msg.note, GetParsedString("&CfgPtn.ParamSave"));
					upload_sg(&msg);
					
					int ret = SaveScrStr2Cfg( pCheckBox4[0],
									GSR_CONFIG_RECORD_ENABLE_CYCLINGREC_REC, 
									EM_GSR_CTRL_CHK, 
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
				else if(5 == curID) //抓图子页面app
				{
					//抓图参数保存
					SSG_MSG_TYPE msg;
					memset(&msg, 0, sizeof(msg));
					msg.type = EM_DVR_PARAM_SAVE;
					msg.chn = 0;
					strcpy(msg.note, GetParsedString("&CfgPtn.LOG_SNAP_SUCCESS"));
					strcat(msg.note, GetParsedString("&CfgPtn.ParamSave"));
					upload_sg(&msg);
					
					//printf("yg 1\n");
					SGuiSnapChnPara SnapPara;
					int chn = pComboBox50->GetCurSel();
					int i;

					SnapPara.TimeSnap.Enable = pCheckBox5[0]->GetValue();
					SnapPara.TimeSnap.Res = 	pComboBox5[0][0]->GetCurSel();
					SnapPara.TimeSnap.Quality = pComboBox5[0][1]->GetCurSel();
					SnapPara.TimeSnap.Interval = pComboBox5[0][2]->GetCurSel();

					SnapPara.EventSnap.Enable = pCheckBox5[1]->GetValue();
					SnapPara.EventSnap.Res = 	pComboBox5[1][0]->GetCurSel();
					SnapPara.EventSnap.Quality = pComboBox5[1][1]->GetCurSel();
					SnapPara.EventSnap.Interval = pComboBox5[1][2]->GetCurSel();
					//printf("yg 2\n");

					if (chn < GetMaxChnNum())
					{
						//printf("%s yg 3, chn%d\n", __FUNCTION__, chn);
						if (bizData_SetSnapChnPara(chn, &SnapPara) != 0)
						{
							printf("%s: bizData_SetSnapChnParaDefault() failed, chn%d\n", __FUNCTION__, chn);
						}
						//printf("yg 4\n");
					}
					else //all
					{
						for(int i=0; i<GetMaxChnNum(); i++)
						{
							if (bizData_SetSnapChnPara(i, &SnapPara) != 0)
							{
								printf("%s: bizData_SetSnapChnParaDefault() failed, chn%d\n", __FUNCTION__, i);
							}
						}
					}
				}
			}break;
			case 8://退出
			{
				//printf("exit \n");
				this->Close();
			}break;
            case 9:
            {
	//yaogang modify 20141128
	#ifndef DISABLE_RECBITRATE
                //上一步
                CPage** page = GetPage();
                if(page && page[EM_PAGE_NETCFG])
                {
                    page[EM_PAGE_NETCFG]->UpdateData(UDM_GUIDE);
                    if (page[EM_PAGE_NETCFG]->Open())
                    {
                        this->Close();
                    }
                }
	#endif
            } break;
            case 10:
            {
//yaogang modify 20141128				
#ifndef DISABLE_RECBITRATE
                //下一步
                //保存配置
				int i = 0;
				int j = 0;
				int bAll = pCheckBox1->GetValue();
				if(bAll == TRUE)
				{
					for(i=0; i<REC_STREAM_COLS-1; i++)
					{	
						int index = pComboBox1[i+REC_STREAM_ROWS*5]->GetCurSel();

						switch(i)
						{
							case 0:			
								for(int k=0; k<GetMaxChnNum(); k++)
								{
									resolution[k] = index;
								}
								break;
							case 1:
								for(int k=0; k<GetMaxChnNum(); k++)
								{
									framerate[k] = index;
								}
								break;
							case 2:
								for(int k=0; k<GetMaxChnNum(); k++)
								{
									encode[k] = index;

									if(encode[k] == 0)
									{
										//CBR
										qualityEnable[k] = 0;
									}
									else
									{
										//VBR
										qualityEnable[k] = 1;
									}
								}
								break;
							case 3:
								for(int k=0; k<GetMaxChnNum(); k++)
								{
									quality[k] = index;
								}
								break;
							case 4:
								for(int k=0; k<GetMaxChnNum(); k++)
								{
									bitrate[k] = index;
								}
								break;
							default:
								break;

						}

					}
				}
                
				{
					for(i=0; i<GetMaxChnNum(); i++)
					{
						if(GetCurRecStatus(i) && (GetReso(i) != resolution[i]))
						{								
							break;
						}
					}
                    
					if(i<GetMaxChnNum())
					{
                        #if 0
                        char tmp[30] = {0};
	                    GetProductNumber(tmp);
                        if ((0 == strcasecmp(tmp, "R9504S"))
                        	|| (0 == strcasecmp(tmp, "R9508S")))
                        {
                            MessageBox("&CfgPtn.ModifyProhibition", "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
                            this->UpdateData(UDM_OPEN);
                            return;
                        }
                        else
                        #endif
                        {
                         	UDM ret = MessageBox("&CfgPtn.ModifyResolutionRecording", "&CfgPtn.WARNING" , MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2);
							
 							if(UDM_OK != GetRunStatue())
							{
								break;
							}
                        }
					}
				}

				if(bAll == TRUE)
				{
					SValue frameRateList[10];
					int nFrReal;
					for(i=0; i<GetStreamRows(); i++)
					{	
						int nCh = m_page1*REC_STREAM_ROWS+i;
						if(nCh >= GetMaxChnNum())
						{
							break;
						}
						
						pComboBox1[i]->SetCurSel(resolution[nCh]);
						
						int index = pComboBox1[i]->GetCurSel();
						GetFrameRateList(index, frameRateList, &nFrReal, 10);
						pComboBox1[i+REC_STREAM_ROWS]->RemoveAll();
						for(j=0; j<nFrReal; j++)
						{
							pComboBox1[i+REC_STREAM_ROWS]->AddString(frameRateList[j].strDisplay);
						}

						pComboBox1[i+REC_STREAM_ROWS]->SetCurSel(framerate[nCh]);
						pComboBox1[i+REC_STREAM_ROWS*2]->SetCurSel(encode[nCh]);
						pComboBox1[i+REC_STREAM_ROWS*3]->SetCurSel(quality[nCh]);
						pComboBox1[i+REC_STREAM_ROWS*3]->Enable(qualityEnable[nCh]);
						pComboBox1[i+REC_STREAM_ROWS*4]->SetCurSel(bitrate[nCh]);
						
					}
				}
				
				#if 0
				for(int i=0; i<GetMaxChnNum(); i++)
				{
					printf("resolution[%d] = %d \n",i,resolution[i]);
					printf("framerate[%d] = %d \n",i,framerate[i]);
					printf("encode[%d] = %d \n",i,encode[i]);
					printf("quality[%d] = %d \n",i,quality[i]);
					printf("bitrate[%d] = %d \n",i,bitrate[i]);
				}
				#endif

				int id1[5] = 
				{
					GSR_CONFIG_RECORD_ENABLE_CODESTREAM_RESOLUTION,					
					GSR_CONFIG_RECORD_ENABLE_CODESTREAM_FRAMERATE,				
					GSR_CONFIG_RECORD_ENABLE_CODESTREAM_ENCODE,				
					GSR_CONFIG_RECORD_ENABLE_CODESTREAM_QUALITY,					
					GSR_CONFIG_RECORD_ENABLE_CODESTREAM_MAXRATE,	
				};
				
				int ret;
				
				for(j=0; j<GetMaxChnNum(); j++)
				{		
					for(i=0; i<REC_STREAM_COLS-1; i++)
					{
						switch(i)
						{
							case 0:
								pTmpCombo->SetCurSel(resolution[j]);
								break;
							case 1:
								pTmpCombo->SetCurSel(framerate[j]);
								break;
							case 2:
								pTmpCombo->SetCurSel(encode[j]);
								break;
							case 3:
								pTmpCombo->SetCurSel(quality[j]);
								break;
							case 4:
								pTmpCombo->SetCurSel(bitrate[j]);
								break;
							default:
								break;
						}
						
						ret = SaveScrStr2Cfg( pTmpCombo,
										id1[i], 
										EM_GSR_CTRL_COMB, 
										j
					    );
					}
				}
				
				//				
                CPage** page = GetPage();
                if(page && page[EM_PAGE_DISKMGR])
                {
                    page[EM_PAGE_DISKMGR]->UpdateData(UDM_GUIDE);
                    if (page[EM_PAGE_DISKMGR]->Open())
                    {
                        this->Close();
                    }
                }
#endif
            } break;
			default:
				break;
		}
	}
}

CPageRecordFrameWork::~CPageRecordFrameWork()
{

}

VD_PCSTR CPageRecordFrameWork::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

void CPageRecordFrameWork::OnTrackMove0()
{
	int pos = pScroll0->GetPos();
    if (m_page0==pos) {
        return;
    }
    m_page0 = pos;

	//printf("m_page0 = %d, audio_page = %d, GetAudioChnNum = %d\n", m_page0,audio_page,GetAudioChnNum());
	
	for(int i=0; i<REC_ENABLE_ROWS; i++)
	{
		
		char szCh[16] = {0};
		int nCh = m_page0*REC_ENABLE_ROWS+i;
		//printf("nCh = %d\n", nCh);
		
		if(nCh >= GetMaxChnNum())
		{
			break;
		}
		
		sprintf(szCh,"%d",nCh+1);
		pStatic0[i+REC_ENABLE_COLS]->SetText(szCh);

		pCheckBox0[i]->SetValue(bVideoEnable[nCh]);
		
		if(nCh < GetAudioChnNum())
		{
			pCheckBox0[i+REC_ENABLE_ROWS]->SetValue(bAudioEnable[nCh]);
		}
	}
	
	AdjustEnableRows();
	AdjustAudioRows();

}

void CPageRecordFrameWork::OnVideoCheck0()
{
	
	int i = 0;
	BOOL bFind = FALSE;
	CCheckBox* pFocusCheck = (CCheckBox *)GetFocusItem();
	for (i = 0; i < REC_ENABLE_ROWS; i++)
	{
		if (pFocusCheck == pCheckBox0[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
		int ch = m_page0*REC_ENABLE_ROWS+i;

		bVideoEnable[ch] = pFocusCheck->GetValue();
	}
}


void CPageRecordFrameWork::OnAudioCheck0()
{
	int i = 0;
	BOOL bFind = FALSE;
	CCheckBox* pFocusCheck = (CCheckBox *)GetFocusItem();
	for (i = 0; i < REC_ENABLE_ROWS; i++)
	{
		if (pFocusCheck == pCheckBox0[i+REC_ENABLE_ROWS])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
		int ch = m_page0*REC_ENABLE_ROWS+i;
		
		bAudioEnable[ch] = pFocusCheck->GetValue();
	}

}

void CPageRecordFrameWork::InitPage0()
{
	m_page0 = 0;
	m_maxPage0 = 1;

	bVideoEnable = (int*)malloc(sizeof(int)*GetMaxChnNum());
	bAudioEnable = (int*)malloc(sizeof(int)*GetAudioChnNum());
	
	char* statictext[4] = {
		"&CfgPtn.Channel",
		"&CfgPtn.Enable",
		"&CfgPtn.Audio",
		"&CfgPtn.All",
	};
	
	CRect rtSubPage(m_Rect.left+25, m_Rect.top+50, 
		m_Rect.left+m_Rect.Width()-30,m_Rect.top+m_Rect.Height()-170);

	//printf("dddddddd  %d \n",m_Rect.top+m_Rect.Height()-170 - (m_Rect.top+43));
	pTable00 = CreateTableBox(&rtSubPage,this, 3,7,0);
	pTable00->SetColWidth(0,60);
	pTable00->SetColWidth(1,180);
	items[0].push_back(pTable00);

	if(GetMaxChnNum()>REC_ENABLE_ROWS)
	{
		m_maxPage0 = GetMaxChnNum()%REC_ENABLE_ROWS?GetMaxChnNum()/REC_ENABLE_ROWS+1:GetMaxChnNum()/REC_ENABLE_ROWS;
		
		pScroll0 = CreateScrollBar(CRect(m_Rect.Width()-30-25,
											m_Rect.top+80,
											m_Rect.Width()-30,
											m_Rect.top+m_Rect.Height()-170), 
											this,
											scrollbarY,
											0,
											(m_maxPage0-1)*10, 
											1, 
											(CTRLPROC)&CPageRecordFrameWork::OnTrackMove0);

		items[0].push_back(pScroll0);
	}

	int rows = GetEnableRows();
	CRect tmpRt;
	int i = 0;
	for(i=0; i<REC_ENABLE_COLS; i++)
	{

		pTable00->GetTableRect(i,0,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic0[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, statictext[i]);
		pStatic0[i]->SetBkColor(VD_RGB(67,77,87));
		pStatic0[i]->SetTextAlign(VD_TA_XLEFT);
		items[0].push_back(pStatic0[i]);
	}

	for(i=REC_ENABLE_COLS; i<REC_ENABLE_COLS+rows; i++)
	{

		pTable00->GetTableRect(0,i-2,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		char szCh[16] = {0};
		sprintf(szCh,"%d",i-2);
		pStatic0[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, szCh);
		items[0].push_back(pStatic0[i]);
	}

	pStatic0[REC_ENABLE_COLS+REC_ENABLE_ROWS] = CreateStatic(CRect(rtSubPage.left, rtSubPage.bottom+5, rtSubPage.left+200,rtSubPage.bottom+30), this, statictext[REC_ENABLE_COLS]);
	items[0].push_back(pStatic0[REC_ENABLE_COLS+REC_ENABLE_ROWS]);

	for(i=0; i<rows; i++)
	{
		
		pTable00->GetTableRect(1,i+1,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pCheckBox0[i] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
			rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+23), 
			this,styleEditable,(CTRLPROC)&CPageRecordFrameWork::OnVideoCheck0);
		pCheckBox0[i]->SetValue(TRUE);
		items[0].push_back(pCheckBox0[i]);

	}

	int rows_audio = GetEnableAudioRows();
	
	for(i=REC_ENABLE_ROWS; i<REC_ENABLE_ROWS+rows_audio; i++)
	{
		
		pTable00->GetTableRect(2,i-5,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pCheckBox0[i] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
			rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+23), 
			this,styleEditable,(CTRLPROC)&CPageRecordFrameWork::OnAudioCheck0);
		pCheckBox0[i]->SetValue(TRUE);
		items[0].push_back(pCheckBox0[i]);

	}

	rtSubPage.left = m_Rect.left+25;
	rtSubPage.top = m_Rect.top+m_Rect.Height()-137;//m_Rect.top+255;
	rtSubPage.right = m_Rect.left+m_Rect.Width()-30;
	rtSubPage.bottom = m_Rect.top+m_Rect.Height()-106;

	//printf("$$$$$$$$$$ rtSubPage.bottom = %d \n",rtSubPage.bottom);
	
	pTable01 = CreateTableBox(&rtSubPage,this, 3,1,0);
	pTable01->SetColWidth(0,60);
	pTable01->SetColWidth(1,180);

	items[0].push_back(pTable01);

	for(i=REC_ENABLE_ROWS*2; i<REC_ENABLE_ROWS*2+3; i++)
	{
		
		pTable00->GetTableRect(i-REC_ENABLE_ROWS*2,0,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pCheckBox0[i] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
			rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+23), 
			this);
		pCheckBox0[i]->SetValue(0);
		items[0].push_back(pCheckBox0[i]);
	}
	//ShowSubPage(0,TRUE);
	pButton[0]->Enable(FALSE);
}

void CPageRecordFrameWork::OnComboSel10()
{
	int i = 0;
	BOOL bFind = FALSE, bChangeFrameRate = FALSE;
	CComboBox* pFocusCombo = (CComboBox*)GetFocusItem();
	for(i = 0; i < REC_STREAM_ROWS; i++)
	{
		if(pFocusCombo == pComboBox1[i])
		{
			bChangeFrameRate = TRUE;
			bFind = TRUE;
			
			break;
		}
	}
	
	//printf("bFind = %d\n",bFind);
	
	// change resolution or framerate
	if(bFind)
	{
		SValue frameRateList[10];
		int nFrReal;
		
		int index = pComboBox1[i]->GetCurSel();
		GetFrameRateList(index, frameRateList, &nFrReal, 10);
		pComboBox1[i+REC_STREAM_ROWS]->RemoveAll();
		
		for(int j=0; j<nFrReal; j++)
		{
			pComboBox1[i+REC_STREAM_ROWS]->AddString(frameRateList[j].strDisplay);
		}
		
		int nCh = m_page1*REC_STREAM_ROWS+i;
		//pComboBox1[i+REC_STREAM_ROWS]->SetCurSel(framerate[nCh]);
		pComboBox1[i+REC_STREAM_ROWS]->SetCurSel(nFrReal-1);
        //pComboBox1[i+4*REC_STREAM_ROWS]->SetCurSel(8);
		framerate[nCh] = pComboBox1[i+REC_STREAM_ROWS]->GetCurSel();
		
		resolution[nCh] = pFocusCombo->GetCurSel();
	}
	
	if(bChangeFrameRate)
	{
		//CheckFrameLeft();
	}
}

void CPageRecordFrameWork::OnComboSel11()
{
	int i = 0;
	BOOL bFind = FALSE, bChangeFrameRate = FALSE;
	CComboBox* pFocusCombo = (CComboBox*)GetFocusItem();
	for(i = 0; i < REC_STREAM_ROWS; i++)
	{
		if(pFocusCombo == pComboBox1[i+REC_STREAM_ROWS])
		{
			bChangeFrameRate = TRUE;
			bFind = TRUE;
			
			break;
		}
	}
	
	//printf("bFind = %d\n",bFind);
	
	// change resolution or framerate
	if(bFind)
	{
		int nCh = m_page1*REC_STREAM_ROWS+i;
		
		framerate[nCh] = pFocusCombo->GetCurSel();
		
		//printf("framerate[%d] = %d \n",nCh,framerate[nCh]);
	}
	
	if(bChangeFrameRate)
	{
		//CheckFrameLeft();
	}
}

void CPageRecordFrameWork::OnComboSel12()
{
	int i = 0;
	BOOL bFind = FALSE;
	CComboBox* pFocusCombo = (CComboBox*)GetFocusItem();
	for(i = 0; i < REC_STREAM_ROWS; i++)
	{
		if(pFocusCombo == pComboBox1[i+REC_STREAM_ROWS*2])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		int nCh = m_page1*REC_STREAM_ROWS+i;
		int index = pComboBox1[i+REC_STREAM_ROWS*2]->GetCurSel();
		if(index == 0)
		{
			//CBR
			pComboBox1[i+REC_STREAM_ROWS*3]->Enable(FALSE);
			qualityEnable[nCh] = 0;
		}
		else
		{
			//VBR
			pComboBox1[i+REC_STREAM_ROWS*3]->Enable(TRUE);
			qualityEnable[nCh] = 1;
		}
		
		encode[nCh] = pFocusCombo->GetCurSel();
	}
}

void CPageRecordFrameWork::OnComboSel13()
{
	int i = 0;
	BOOL bFind = FALSE;
	CComboBox* pFocusCombo = (CComboBox*)GetFocusItem();
	for(i = 0; i < REC_STREAM_ROWS; i++)
	{
		if(pFocusCombo == pComboBox1[i+REC_STREAM_ROWS*3])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		int nCh = m_page1*REC_STREAM_ROWS+i;
		
		quality[nCh] = pFocusCombo->GetCurSel();
	}
}

void CPageRecordFrameWork::OnComboSel14()
{
	int i = 0;
	BOOL bFind = FALSE;
	CComboBox* pFocusCombo = (CComboBox*)GetFocusItem();
	for (i = 0; i < REC_STREAM_ROWS; i++)
	{
		if (pFocusCombo == pComboBox1[i+REC_STREAM_ROWS*4])
		{
			bFind = TRUE;
			break;
		}
	}

	if(bFind)
	{
		int nCh = m_page1*REC_STREAM_ROWS+i;

		bitrate[nCh] = pFocusCombo->GetCurSel();
	}
	
}

void CPageRecordFrameWork::OnComboSel15()
{
	int index = pComboBox1[5*REC_STREAM_ROWS+2]->GetCurSel();
	if(index == 0)
	{
		//CBR
		pComboBox1[5*REC_STREAM_ROWS+3]->Enable(FALSE);
	}
	else
	{
		//VBR
		pComboBox1[5*REC_STREAM_ROWS+3]->Enable(TRUE);
	}

}

void CPageRecordFrameWork::OnComboSel16()
{
	return;
	pStatic1[2]->Show(0);
	pStatic1[2]->Show(1,1);
	pStatic1[5]->Show(0);
	pStatic1[5]->Show(1,1);
}

void CPageRecordFrameWork::OnComboAllSel1()
{

	SValue frameRateList[10];
	int nFrReal;

	int index = pComboBox1[5*REC_STREAM_ROWS]->GetCurSel();
	GetFrameRateList(index, frameRateList, &nFrReal, 10);
	pComboBox1[5*REC_STREAM_ROWS+1]->RemoveAll();
	for(int j=0; j<nFrReal; j++)
	{
		pComboBox1[5*REC_STREAM_ROWS+1]->AddString(frameRateList[j].strDisplay);
	}

	pComboBox1[5*REC_STREAM_ROWS+1]->SetCurSel(nFrReal-1);

}

void CPageRecordFrameWork::OnTrackMove1()
{
	int nResolHnum = 0;//cw_9508S
	GetResoListH_NUM(&nResolHnum);
	
	SValue resoList2[10];
	int nResoReal2;
	GetResoList2(resoList2, &nResoReal2, 10);
	SValue resoList[10];
	int nResoReal;
	GetResoList(resoList, &nResoReal, 10);
	int pos = pScroll1->GetPos();
    if (m_page1==pos) {
        return;
    }
    m_page1 = pos;

	for(int i=0; i<REC_STREAM_ROWS; i++)
	{
		
		char szCh[16] = {0};
		int nCh = m_page1*REC_STREAM_ROWS+i;
		if(nCh >= GetMaxChnNum())
		{
			break;
		}
		
		sprintf(szCh,"%d",nCh+1);
		pStatic1[i+REC_STREAM_COLS]->SetText(szCh);

		if(nCh<nResolHnum)//cw_9508S
		{
			pComboBox1[i]->RemoveAll();
			for(int k = 0;k<nResoReal;k++)
			{
				pComboBox1[i]->AddString(resoList[k].strDisplay);
				pComboBox1[i]->SetCurSel(resolution[nCh]);
			}
		}
		else
		{
			pComboBox1[i]->RemoveAll();
			for(int k = 0;k<nResoReal2;k++)
			{
				pComboBox1[i]->AddString(resoList2[k].strDisplay);
				pComboBox1[i]->SetCurSel(resolution[nCh]);
			}
		}
		pComboBox1[i+REC_STREAM_ROWS]->SetCurSel(framerate[nCh]);
		pComboBox1[i+REC_STREAM_ROWS*2]->SetCurSel(encode[nCh]);
		pComboBox1[i+REC_STREAM_ROWS*3]->SetCurSel(quality[nCh]);
		pComboBox1[i+REC_STREAM_ROWS*3]->Enable(qualityEnable[nCh]);
		pComboBox1[i+REC_STREAM_ROWS*4]->SetCurSel(bitrate[nCh]);

	}

	AdjustStreamRows();


}

void CPageRecordFrameWork::InitPage1()
{
	m_page1 = 0;
	m_maxPage1 = 1;
	
	resolution = (int*)malloc(sizeof(int)*GetMaxChnNum());
	framerate = (int*)malloc(sizeof(int)*GetMaxChnNum());
	encode = (int*)malloc(sizeof(int)*GetMaxChnNum());
	quality = (int*)malloc(sizeof(int)*GetMaxChnNum());
	qualityEnable = (int*)malloc(sizeof(int)*GetMaxChnNum());
	bitrate = (int*)malloc(sizeof(int)*GetMaxChnNum());
	
	char* statictext[] = {
		"&CfgPtn.Channel",
		"&CfgPtn.Resolution",
		"&CfgPtn.Fps",
		"&CfgPtn.Encode",
		"&CfgPtn.Quality",
		"&CfgPtn.MaxBitrate",
		"&CfgPtn.All",
		"&CfgPtn.Remaing",
		": 0 (CIF)"
	};
	
	CRect rtSubPage(m_Rect.left+25, m_Rect.top+50, 
		m_Rect.left+m_Rect.Width()-30,m_Rect.top+m_Rect.Height()-170);
	
	//printf("dddddddd %d\n",m_Rect.top+m_Rect.Height()-170 - (m_Rect.top+43));
	pTable10 = CreateTableBox(&rtSubPage,this, 6,7,0);
	pTable10->SetColWidth(0,60);
	pTable10->SetColWidth(1,100);
	pTable10->SetColWidth(2,105);
	pTable10->SetColWidth(3,85);
	pTable10->SetColWidth(4,95);
	//pTable10->SetColWidth(5,180);
	items[1].push_back(pTable10);
	
	if(GetMaxChnNum()>REC_STREAM_ROWS)
	{
		m_maxPage1 = GetMaxChnNum()%REC_STREAM_ROWS?GetMaxChnNum()/REC_STREAM_ROWS+1:GetMaxChnNum()/REC_STREAM_ROWS;
		
		pScroll1 = CreateScrollBar(CRect(m_Rect.Width()-30-25,
											m_Rect.top+80,
											m_Rect.Width()-30,
											m_Rect.top+m_Rect.Height()-170), 
											this,
											scrollbarY,
											0,
											(m_maxPage1-1)*10, 
											1, 
											(CTRLPROC)&CPageRecordFrameWork::OnTrackMove1);

		items[1].push_back(pScroll1);
	}

	int rows = GetStreamRows();
	CRect tmpRt;
	int i = 0;
	for(i=0; i<REC_STREAM_COLS; i++)
	{
		pTable10->GetTableRect(i,0,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic1[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, statictext[i]);
		pStatic1[i]->SetBkColor(VD_RGB(67,77,87));
		pStatic1[i]->SetTextAlign(VD_TA_XLEFT);
		items[1].push_back(pStatic1[i]);
	}

	for(i=REC_STREAM_COLS; i<REC_STREAM_COLS+rows; i++)
	{
		pTable10->GetTableRect(0,i-5,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		char szCh[16] = {0};
		sprintf(szCh,"%d",i-(REC_STREAM_COLS-1));
		pStatic1[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, szCh);
		items[1].push_back(pStatic1[i]);
	}

	rtSubPage.top = rtSubPage.bottom+5;
	rtSubPage.right = rtSubPage.left+55;
	rtSubPage.bottom = rtSubPage.bottom+30;

	pStatic1[REC_STREAM_COLS+REC_STREAM_ROWS] = CreateStatic(rtSubPage, this, statictext[REC_STREAM_COLS]);
	items[1].push_back(pStatic1[REC_STREAM_COLS+REC_STREAM_ROWS]);

    /*
	for(i=REC_STREAM_COLS+REC_STREAM_ROWS; i<REC_STREAM_COLS+REC_STREAM_ROWS+2; i++)
	{
		
		pStatic1[i] = CreateStatic(rtSubPage, this, statictext[i-REC_STREAM_ROWS]);
		
		rtSubPage.left = rtSubPage.right+10;
		rtSubPage.right = rtSubPage.left+200;
		items[1].push_back(pStatic1[i]);
	}
	*/

	// remain frame count status
	//
	char szTmp[32] = {0};

	rtSubPage.left = rtSubPage.left-60;
	rtSubPage.right = rtSubPage.left+200;
	//sprintf(szTmp, ": %d(CIF), %d(D1).", nRemainCif, nRemainCif>>2);
	//pStatic1[i] = CreateStatic(rtSubPage, this, szTmp);
	//items[1].push_back(pStatic1[i]);
	
	rtSubPage.left = m_Rect.left+25;
	rtSubPage.top = m_Rect.top+50;
	rtSubPage.right = m_Rect.left+m_Rect.Width()-30;
	rtSubPage.bottom = m_Rect.top+m_Rect.Height()-175;
	
	int nResolHnum = 0;//cw_9508S
	GetResoListH_NUM(&nResolHnum);
	
	SValue resoList2[10];
	int nResoReal2;
	GetResoList2(resoList2, &nResoReal2, 10);
	
	SValue resoList[10];
	int nResoReal;
	GetResoList(resoList, &nResoReal, 10);
	
	SValue frameRateList[10];
	int nFrReal;
	GetFrameRateList(0, frameRateList, &nFrReal, 10);
	
	SValue encodeList[10];
	int nEncReal;
	GetEncodeList(encodeList, &nEncReal, 10);
	
	SValue qualityList[10];
	int nQualityReal;
	GetQualityList(qualityList, &nQualityReal, 10);
	
	SValue bitRateList[15];
	int nBrReal;
	GetBitrateList(bitRateList, &nBrReal, 15);
	
	int j = 0;
	for(i=0; i<REC_STREAM_COLS-1; i++)
	{
		CTRLPROC ctrlProc = NULL;
		switch(i)
		{
			case 0:
				ctrlProc = (CTRLPROC)&CPageRecordFrameWork::OnComboSel10;
				break;
			case 1:
				ctrlProc = (CTRLPROC)&CPageRecordFrameWork::OnComboSel11;
				break;
			case 2:
				ctrlProc = (CTRLPROC)&CPageRecordFrameWork::OnComboSel12;
				break;
			case 3:
				ctrlProc = (CTRLPROC)&CPageRecordFrameWork::OnComboSel13;
				break;
			case 4:
				ctrlProc = (CTRLPROC)&CPageRecordFrameWork::OnComboSel14;
				break;
			default:
				break;
		}
		
		for(j=0; j<rows; j++)
		{
			pTable10->GetTableRect(i+1,j+1,&tmpRt);
			//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);

			if((i == REC_STREAM_COLS-2) && (GetMaxChnNum()>REC_STREAM_ROWS))
			{
				pComboBox1[i*REC_STREAM_ROWS+j] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
						rtSubPage.left+tmpRt.right-2-25,rtSubPage.top+tmpRt.bottom-1), 
						this, NULL, NULL, (CTRLPROC)ctrlProc, 0);
			}
			else
			{	
				pComboBox1[i*REC_STREAM_ROWS+j] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
						rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
						this, NULL, NULL, (CTRLPROC)ctrlProc, 0);
			}
			
			pComboBox1[i*REC_STREAM_ROWS+j]->SetBkColor(VD_RGB(67,77,87));
			
			switch(i)
			{
				case 0:
				{
					if(j<nResolHnum)//cw_9508S
					{
						for(int k=0; k<nResoReal; k++)
						{
							pComboBox1[i*REC_STREAM_ROWS+j]->AddString(resoList[k].strDisplay);
						}
					}
					else
					{
						for(int k=0; k<nResoReal2; k++)
						{
							pComboBox1[i*REC_STREAM_ROWS+j]->AddString(resoList2[k].strDisplay);
						}
					}
					pComboBox1[i*REC_STREAM_ROWS+j]->SetCurSel(1);//9624 support resolution

				}break;
				case 1:
				{
					for(int k=0; k<nFrReal; k++)
					{
						pComboBox1[i*REC_STREAM_ROWS+j]->AddString(frameRateList[k].strDisplay);
					}
					pComboBox1[i*REC_STREAM_ROWS+j]->SetCurSel(8);
				}break;
				case 2:
				{
					for(int k=0; k<nEncReal; k++)
					{
						pComboBox1[i*REC_STREAM_ROWS+j]->AddString(encodeList[k].strDisplay);
					}
					pComboBox1[i*REC_STREAM_ROWS+j]->SetCurSel(0);
				}break;
				case 3:
				{	
					for(int k=0; k<nQualityReal; k++)
					{
						pComboBox1[i*REC_STREAM_ROWS+j]->AddString(qualityList[k].strDisplay);
					}
					pComboBox1[i*REC_STREAM_ROWS+j]->SetCurSel(3);
				}break;
				case 4:
				{
					for(int k=0; k<nBrReal; k++)
					{
						pComboBox1[i*REC_STREAM_ROWS+j]->AddString(bitRateList[k].strDisplay);
					}
					pComboBox1[i*REC_STREAM_ROWS+j]->SetCurSel(8);
				}break;
				default:
				{}break;
			}
			
			#if 0//csp modify
			if(i == 0)
			{
				pComboBox1[i*REC_STREAM_ROWS+j]->Enable(0);
			}
			#endif
			
			items[1].push_back(pComboBox1[i*REC_STREAM_ROWS+j]);
		}
	}
	
	rtSubPage.left = m_Rect.left+25;
	rtSubPage.top = m_Rect.top+m_Rect.Height()-137;//137;//m_Rect.top+255;
	rtSubPage.right = m_Rect.left+m_Rect.Width()-30;
	rtSubPage.bottom = m_Rect.top+m_Rect.Height()-106;//106;
	
	pTable11 = CreateTableBox(&rtSubPage,this, 6,1,0);
	pTable11->SetColWidth(0,60);
	pTable11->SetColWidth(1,100);
	pTable11->SetColWidth(2,105);
	pTable11->SetColWidth(3,85);
	pTable11->SetColWidth(4,95);
	
	//pTable11->SetColWidth(0,60);
	//pTable11->SetColWidth(1,180);
	items[1].push_back(pTable11);
	
	pTable11->GetTableRect(0,0,&tmpRt);
	
	pCheckBox1 = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
			rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+23), 
			this);
	
	items[1].push_back(pCheckBox1);
	
	for(i=REC_STREAM_ROWS*5; i<REC_STREAM_ROWS*5+5; i++)
	{
		pTable11->GetTableRect(i-(REC_STREAM_ROWS*5-1),0,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		if(i == REC_STREAM_ROWS*5)
		{
			pComboBox1[i] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
							rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
							this, NULL, NULL, (CTRLPROC)&CPageRecordFrameWork::OnComboAllSel1, 0);

		}
		else if( i == REC_STREAM_ROWS*5+2)
		{
			pComboBox1[i] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
							rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
							this, NULL, NULL, (CTRLPROC)&CPageRecordFrameWork::OnComboSel15, 0);
		}
		else
		{
			pComboBox1[i] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
							rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
							this, NULL, NULL, (CTRLPROC)&CPageRecordFrameWork::OnComboSel16/*NULL*/, 0);
		}
		
		pComboBox1[i]->SetBkColor(VD_RGB(67,77,87));
		
		switch(i-REC_STREAM_ROWS*5)
		{
			case 0:
			{
                char tmp[30] = {0};
                GetProductNumber(tmp);
				
                //R9508S只有前二路为D1,R9504S有四路D1
				if((nResoReal2>0) && (0 == strcasecmp(tmp, "R9508S") || 0 == strcasecmp(tmp, "R9508")))//csp modify
				{
					for(int k=0; k<nResoReal2; k++)
					{
						pComboBox1[i]->AddString(resoList2[k].strDisplay);
					}
					pComboBox1[i]->SetCurSel(1);					
				}
				else
				{
					for(int k=0; k<nResoReal; k++)
					{
						pComboBox1[i]->AddString(resoList[k].strDisplay);
					}
					pComboBox1[i]->SetCurSel(1);
				}
			}break;
			case 1:
			{
				for(int k=0; k<nFrReal; k++)
				{
					pComboBox1[i]->AddString(frameRateList[k].strDisplay);
				}
				pComboBox1[i]->SetCurSel(8);
			}break;
			case 2:
			{
				for(int k=0; k<nEncReal; k++)
				{
					pComboBox1[i]->AddString(encodeList[k].strDisplay);
				}
				pComboBox1[i]->SetCurSel(0);
			}break;
			case 3:
			{	
				for(int k=0; k<nQualityReal; k++)
				{
					pComboBox1[i]->AddString(qualityList[k].strDisplay);
				}
				pComboBox1[i]->SetCurSel(3);
			}break;
			case 4:
			{
				for(int k=0; k<nBrReal; k++)
				{
					pComboBox1[i]->AddString(bitRateList[k].strDisplay);
				}
				//csp modify
				//pComboBox1[i]->SetCurSel(8);
				pComboBox1[i]->SetCurSel(5);
			}break;
			default:
			{}break;
		}
		
		#if 0//csp modify
		if(i == REC_STREAM_ROWS*5)
		{
			pComboBox1[i]->Show(0);
			continue;
		}
		#endif
		
		items[1].push_back(pComboBox1[i]);
	}
	
	ShowSubPage(1,FALSE);
}

void CPageRecordFrameWork::OnTrackMove2()
{
	int pos = pScroll2->GetPos();
    if (m_page2==pos) {
        return;
    }
    m_page2 = pos;

	for(int i=0; i<GetRecTimeRows(); i++)
	{
		
		char szCh[16] = {0};
		int nCh = m_page2*REC_TIME_ROWS+i;
		if(nCh >= GetMaxChnNum())
		{
			break;
		}
		
		sprintf(szCh,"%d",nCh+1);
		pStatic2[i+REC_TIME_COLS]->SetText(szCh);
		pComboBox2[i]->SetCurSel(preRecTime[nCh]);
		pComboBox2[i+REC_TIME_ROWS]->SetCurSel(delayRecTime[nCh]);
		pComboBox2[i+REC_TIME_ROWS*2]->SetCurSel(recExpireTime[nCh]);
	}

	AdjustTimeRows();
}

void CPageRecordFrameWork::OnComboSel20()
{
	int i = 0;
	BOOL bFind = FALSE;
	CComboBox* pFocusCombo = (CComboBox*)GetFocusItem();
	for(i = 0; i < REC_TIME_ROWS; i++)
	{
		if(pFocusCombo == pComboBox2[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		int ch = m_page2*REC_TIME_ROWS+i;
		
		preRecTime[ch] = pFocusCombo->GetCurSel();
	}
}

void CPageRecordFrameWork::OnComboSel21()
{

	int i = 0;
	BOOL bFind = FALSE;
	CComboBox* pFocusCombo = (CComboBox*)GetFocusItem();
	for(i = 0; i < REC_TIME_ROWS; i++)
	{
		if(pFocusCombo == pComboBox2[i+REC_TIME_ROWS])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		int ch = m_page2*REC_TIME_ROWS+i;

		delayRecTime[ch] = pFocusCombo->GetCurSel();
	}
}

void CPageRecordFrameWork::OnComboSel22()
{
	int i = 0;
	BOOL bFind = FALSE;
	CComboBox* pFocusCombo = (CComboBox*)GetFocusItem();
	for (i = 0; i < REC_TIME_ROWS; i++)
	{
		if (pFocusCombo == pComboBox2[i+REC_TIME_ROWS*2])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
		int ch = m_page2*REC_TIME_ROWS+i;

		recExpireTime[ch] = pFocusCombo->GetCurSel();
	}
}

void CPageRecordFrameWork::InitPage2()
{
	m_page2 = 0;
	m_maxPage2 = 1;
	
	preRecTime = (int*)malloc(sizeof(int)*GetMaxChnNum());
	delayRecTime = (int*)malloc(sizeof(int)*GetMaxChnNum());
	recExpireTime = (int*)malloc(sizeof(int)*GetMaxChnNum());
	
	char* statictext[9] = {
		"&CfgPtn.Channel",
		"&CfgPtn.PrealarmRecordTime",
		"&CfgPtn.PostalarmRecordTime",
		"&CfgPtn.Expire",
		"&CfgPtn.All",	
	};
	
	CRect rtSubPage(m_Rect.left+25, m_Rect.top+50, 
		m_Rect.left+m_Rect.Width()-30,m_Rect.top+m_Rect.Height()-170);

	//printf("dddddddd  %d \n",m_Rect.top+m_Rect.Height()-170 - (m_Rect.top+43));
	pTable20 = CreateTableBox(&rtSubPage,this, 4,7,0);
	pTable20->SetColWidth(0,60);
	pTable20->SetColWidth(1,210);
	pTable20->SetColWidth(2,210);
	items[2].push_back(pTable20);

	if(GetMaxChnNum()>REC_TIME_ROWS)
	{
		m_maxPage2 = GetMaxChnNum()%REC_TIME_ROWS?GetMaxChnNum()/REC_TIME_ROWS+1:GetMaxChnNum()/REC_TIME_ROWS;
		
		pScroll2 = CreateScrollBar(CRect(m_Rect.Width()-30-25,
											m_Rect.top+80,
											m_Rect.Width()-30,
											m_Rect.top+m_Rect.Height()-170), 
											this,
											scrollbarY,
											0,
											(m_maxPage2-1)*10, 
											1, 
											(CTRLPROC)&CPageRecordFrameWork::OnTrackMove2);

		items[2].push_back(pScroll2);
	}

	int rows = GetRecTimeRows();
	CRect tmpRt;
	int i = 0;
	for(i=0; i<REC_TIME_COLS; i++)
	{

		pTable20->GetTableRect(i,0,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic2[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, statictext[i]);
		pStatic2[i]->SetBkColor(VD_RGB(67,77,87));
		pStatic2[i]->SetTextAlign(VD_TA_XLEFT);
		if(i<REC_TIME_COLS-1) //yzw add
		items[2].push_back(pStatic2[i]);
	}

	for(i=REC_TIME_COLS; i<REC_TIME_COLS+rows; i++)
	{

		pTable20->GetTableRect(0,i-3,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		char szCh[16] = {0};
		sprintf(szCh,"%d",i-(REC_TIME_COLS-1));
		pStatic2[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, szCh);
		items[2].push_back(pStatic2[i]);
	}

	rtSubPage.top = rtSubPage.bottom+5;
	rtSubPage.right = rtSubPage.left+120;
	rtSubPage.bottom = rtSubPage.bottom+30;

	pStatic2[REC_TIME_COLS+REC_TIME_ROWS] = CreateStatic(rtSubPage, this, statictext[REC_TIME_COLS]);
	items[2].push_back(pStatic2[REC_TIME_COLS+REC_TIME_ROWS]);
	
	rtSubPage.left = m_Rect.left+25;
	rtSubPage.top = m_Rect.top+50;
	rtSubPage.right = m_Rect.left+m_Rect.Width()-30;
	rtSubPage.bottom = m_Rect.top+m_Rect.Height()-175;


	SValue preAlarmTimeList[20];
	int nPreAlarmTimeReal;
	GetPreAlarmTimeList(preAlarmTimeList, &nPreAlarmTimeReal, 20);

	SValue delayAlarmTimeList[20];
	int nDelayAlarmTimeReal;
	GetDelayAlarmTimeList(delayAlarmTimeList, &nDelayAlarmTimeReal, 20);

	SValue expirationList[10];
	int nExpirationReal;
	GetExpirationList(expirationList, &nExpirationReal, 10);

		
	int j = 0;
	for(i=0; i<REC_TIME_COLS-1; i++)
	{
		CTRLPROC ctrlProc = NULL;
		switch(i)
		{
			case 0:
				ctrlProc = (CTRLPROC)&CPageRecordFrameWork::OnComboSel20;
				break;
			case 1:
				ctrlProc = (CTRLPROC)&CPageRecordFrameWork::OnComboSel21;
				break;
			case 2:
				ctrlProc = (CTRLPROC)&CPageRecordFrameWork::OnComboSel22;
				break;
			default:
				break;
		}
		
		for(j=0; j<rows; j++)
		{
			pTable20->GetTableRect(i+1,j+1,&tmpRt);
			//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
			if((i == REC_TIME_COLS-2) && ( GetMaxChnNum()>REC_TIME_ROWS))
			{
				
				pComboBox2[i*REC_TIME_ROWS+j] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
												rtSubPage.left+tmpRt.right-2-25,rtSubPage.top+tmpRt.bottom-1), 
												this, NULL, NULL, ctrlProc, 0);


			}
			else
			{
				pComboBox2[i*REC_TIME_ROWS+j] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
																rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
																this, NULL, NULL, ctrlProc, 0);
			}
		
			pComboBox2[i*REC_TIME_ROWS+j]->SetBkColor(VD_RGB(67,77,87));

			switch(i)
			{
				case 0:
				{
					for(int k=0; k<nPreAlarmTimeReal; k++)
					{
						pComboBox2[i*REC_TIME_ROWS+j]->AddString(preAlarmTimeList[k].strDisplay);
					}
					
					//pComboBox2[i*REC_TIME_ROWS+j]->SetCurSel(2);

				}break;
				case 1:
				{
					
					for(int k=0; k<nDelayAlarmTimeReal; k++)
					{
						pComboBox2[i*REC_TIME_ROWS+j]->AddString(delayAlarmTimeList[k].strDisplay);

					}
					//pComboBox2[i*REC_TIME_ROWS+j]->SetCurSel(2);

				}break;
				case 2:
				{
					for(int k=0; k<nExpirationReal; k++)
					{
						pComboBox2[i*REC_TIME_ROWS+j]->AddString(expirationList[k].strDisplay);

					}
					//pComboBox2[i*4+j]->SetCurSel(0);
				}break;
				default:
				{}break;
			}
			
			if(i<2) //yzw add
			items[2].push_back(pComboBox2[i*REC_TIME_ROWS+j]);
		}
	}



	/*rtSubPage.left = m_Rect.left+25;
	rtSubPage.top = m_Rect.top+265;
	rtSubPage.right = m_Rect.left+m_Rect.Width()-30;
	rtSubPage.bottom = m_Rect.top+m_Rect.Height()-112;*/

	rtSubPage.left = m_Rect.left+25;
	rtSubPage.top = m_Rect.top+m_Rect.Height()-137;//m_Rect.top+255;
	rtSubPage.right = m_Rect.left+m_Rect.Width()-30;
	rtSubPage.bottom = m_Rect.top+m_Rect.Height()-106;
	
	pTable21 = CreateTableBox(&rtSubPage,this, 4,1,0);
	pTable21->SetColWidth(0,60);
	pTable21->SetColWidth(1,210);
	pTable21->SetColWidth(2,210);

	items[2].push_back(pTable21);

	pTable21->GetTableRect(0,0,&tmpRt);

	pCheckBox2 = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
			rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+23), 
			this);

	items[2].push_back(pCheckBox2);

	for(i=REC_TIME_ROWS*3; i<REC_TIME_ROWS*3+3; i++)
	{
		pTable21->GetTableRect(i-(REC_TIME_ROWS*3-1),0,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pComboBox2[i] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
			rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
			this, NULL, NULL, NULL, 0);
		pComboBox2[i]->SetBkColor(VD_RGB(67,77,87));

		switch(i-REC_TIME_ROWS*3)
		{
			case 0:
			{
				for(int k=0; k<nPreAlarmTimeReal; k++)
				{
					pComboBox2[i]->AddString(preAlarmTimeList[k].strDisplay);

				}
				pComboBox2[i]->SetCurSel(2);

			}break;
			case 1:
			{
				
				for(int k=0; k<nDelayAlarmTimeReal; k++)
				{
					pComboBox2[i]->AddString(delayAlarmTimeList[k].strDisplay);

				}
				pComboBox2[i]->SetCurSel(2);
		

			}break;
			case 2:
			{
				for(int k=0; k<nExpirationReal; k++)
				{
					pComboBox2[i]->AddString(expirationList[k].strDisplay);

				}
				pComboBox2[i]->SetCurSel(0);
			}break;
			default:
			{}break;
		}
		
		if(i<REC_TIME_ROWS*3+2) //yzw add
		{
		    items[2].push_back(pComboBox2[i]);
		}
	}
	
	ShowSubPage(2,FALSE);

	//yzw add
	pStatic2[3]->Show(0);
	pComboBox2[REC_TIME_ROWS*3+2]->Show(0);
	for(j=0; j<rows; j++)
	{
		pComboBox2[2*REC_TIME_ROWS+j]->Show(0);
	}
}

void CPageRecordFrameWork::OnSetup()
{
	//printf("CPageRecordFrameWork::OnSetup \n");

	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < REC_OSD_ROWS+1; i++)
	{
		if (pFocusButton == pButton3[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
		int nChnID = 0;
		int vChn = 0;
		int vTime = 0;
		int vAll = 0;
		if(i<REC_OSD_ROWS)
		{
			nChnID = m_page3*REC_OSD_ROWS+i;
			vChn = bChNameEnable[nChnID];
			vTime = bTimeEnable[nChnID];

			bModify[nChnID] = 1;
				
		}
		else if(i==REC_OSD_ROWS)
		{
			printf("set All CH osd \n");

			nChnID = GetMaxChnNum();
			vAll = pCheckBox3[REC_OSD_ROWS*2]->GetValue();

			if(vAll == 1)
			{
				vChn = pCheckBox3[REC_OSD_ROWS*2+1]->GetValue();
				vTime = pCheckBox3[REC_OSD_ROWS*2+2]->GetValue();
			}

			for(int j=0; j<GetMaxChnNum(); j++)
			{
				bModify[j] = 1;
			}
		}

		

		int osdType = 0;
		if(vChn == 1 && vTime == 0)
		{
			osdType = OSD_TYPE_CHANNEL;
		}
		else if(vChn == 0 && vTime == 1)
		{
			osdType = OSD_TYPE_TIME;
		}
		else if(vChn == 1 && vTime == 1)
		{
			osdType = OSD_TYPE_CHANNEL|OSD_TYPE_TIME;
		}
		
		m_pOSDSet->SetOSDType(nChnID, osdType);
		m_pOSDSet->Open();
	}
}

void CPageRecordFrameWork::OnCheckAll3()
{
    //pButton3[REC_OSD_ROWS]->Enable(pCheckBox3[REC_OSD_ROWS*2]->GetValue());
    
	//printf("OnChNameCheck3 \n");
	int i = 0;
	BOOL bFind = FALSE;
	CCheckBox* pFocusCheck = (CCheckBox *)GetFocusItem();
	for (i=REC_OSD_ROWS*2; i<REC_OSD_ROWS*2+3; i++)
	{
		if (pFocusCheck == pCheckBox3[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
        BOOL bAllCheck = TRUE;

	/*
        for(i = REC_OSD_ROWS*2; i< REC_OSD_ROWS*2 + 3; i++)
        {
            if(!pCheckBox3[i]->GetValue())
            {
                bAllCheck &= FALSE;
                break;
            }
        }
	*/
	    bAllCheck=((pCheckBox3[REC_OSD_ROWS*2]->GetValue())
	               &&((pCheckBox3[REC_OSD_ROWS*2+1]->GetValue()) ||(pCheckBox3[REC_OSD_ROWS*2+2]->GetValue())));
		pButton3[REC_OSD_ROWS]->Enable(bAllCheck);
	}

}

void CPageRecordFrameWork::OnTrackMove3()
{
	
	int pos = pScroll3->GetPos();
    if (m_page3==pos) {
        return;
    }
    m_page3 = pos;

	for(int i=0; i<GetOsdRows(); i++)
	{
		
		char szCh[16] = {0};
		int nCh = m_page3*REC_OSD_ROWS+i;
		if(nCh >= GetMaxChnNum())
		{
			break;
		}
		
		sprintf(szCh,"%d",nCh+1);
		pStatic3[i+REC_OSD_COLS]->SetText(szCh);
		pCheckBox3[i]->SetValue(bChNameEnable[nCh]);
		pCheckBox3[i+REC_OSD_ROWS]->SetValue(bTimeEnable[nCh]);
        pButton3[i]->Enable(bChNameEnable[nCh] && bTimeEnable[nCh]);
	}


	AdjustOsdRows();

}

void CPageRecordFrameWork::OnChNameCheck3()
{
	//printf("OnChNameCheck3 \n");
	int i = 0;
	BOOL bFind = FALSE;
	CCheckBox* pFocusCheck = (CCheckBox *)GetFocusItem();
	for (i = 0; i < REC_OSD_ROWS; i++)
	{
		if (pFocusCheck == pCheckBox3[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
		int ch = m_page3*REC_OSD_ROWS+i;
		
		bChNameEnable[ch] = pFocusCheck->GetValue();
        pButton3[i]->Enable(bChNameEnable[ch] || bTimeEnable[ch]);
		//printf("bChNameEnable[%d] = %d \n",ch,bChNameEnable[ch]);
	}

}

void CPageRecordFrameWork::OnTimeCheck3()
{
	//printf("OnTimeCheck3 \n");
	int i = 0;
	BOOL bFind = FALSE;
	CCheckBox* pFocusCheck = (CCheckBox *)GetFocusItem();
	for (i = 0; i < REC_OSD_ROWS; i++)
	{
		if (pFocusCheck == pCheckBox3[i+REC_OSD_ROWS])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
		int ch = m_page3*REC_OSD_ROWS+i;

		bTimeEnable[ch] = pFocusCheck->GetValue();
        pButton3[i]->Enable(bChNameEnable[ch] || bTimeEnable[ch]);
	}
}

void CPageRecordFrameWork::InitPage3()
{
	m_page3 = 0;
	m_maxPage3 = 1;

	bChNameEnable = (int*)malloc(sizeof(int)*GetMaxChnNum());
	bTimeEnable = (int*)malloc(sizeof(int)*GetMaxChnNum());
    //bSetButtomEnable = (int*)malloc(sizeof(int)*GetMaxChnNum());
	
#if 1
	char* statictext[9] = {
		"&CfgPtn.Channel",
		"&CfgPtn.ChnName",
		"&CfgPtn.TimeStamp",
		"&CfgPtn.Position",
		"&CfgPtn.All",	
	};

	CRect rtSubPage(m_Rect.left+25, m_Rect.top+50, 
		            m_Rect.left+m_Rect.Width()-30,
		            m_Rect.top+m_Rect.Height()-170);

	//printf("dddddddd  %d \n",m_Rect.top+m_Rect.Height()-170 - (m_Rect.top+43));
	pTable30 = CreateTableBox(&rtSubPage,this, 4,7,0);
	pTable30->SetColWidth(0,60);
	pTable30->SetColWidth(1,180);
	pTable30->SetColWidth(2,180);
	items[3].push_back(pTable30);

	if(GetMaxChnNum()>REC_OSD_ROWS)
	{
		m_maxPage3 = GetMaxChnNum()%REC_OSD_ROWS?GetMaxChnNum()/REC_OSD_ROWS+1:GetMaxChnNum()/REC_OSD_ROWS;
		
		pScroll3 = CreateScrollBar(CRect(m_Rect.Width()-30-25,
									m_Rect.top+80,
									m_Rect.Width()-30,
									m_Rect.top+m_Rect.Height()-170), 
									this,
									scrollbarY,
									0,
									(m_maxPage0-1)*10, 
									1, 
									(CTRLPROC)&CPageRecordFrameWork::OnTrackMove3);

		items[3].push_back(pScroll3);
	}
	
	int rows = GetOsdRows();
	CRect tmpRt;
	int i = 0;
	for(i=0; i<REC_OSD_COLS; i++)
	{
		pTable30->GetTableRect(i,0,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic3[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, statictext[i]);
		pStatic3[i]->SetBkColor(VD_RGB(67,77,87));
		pStatic3[i]->SetTextAlign(VD_TA_XLEFT);
		items[3].push_back(pStatic3[i]);
	}
	
	for(i=REC_OSD_COLS; i<REC_OSD_COLS+rows; i++)
	{
		pTable30->GetTableRect(0,i-3,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		char szCh[16] = {0};
		sprintf(szCh,"%d",i-(REC_OSD_COLS-1));
		pStatic3[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, szCh);
		items[3].push_back(pStatic3[i]);
	}
	
	rtSubPage.top = rtSubPage.bottom+5;
	rtSubPage.right = rtSubPage.left+120;
	rtSubPage.bottom = rtSubPage.bottom+30;

	pStatic3[REC_OSD_COLS+REC_OSD_ROWS] = CreateStatic(rtSubPage, this, statictext[REC_OSD_COLS]);
	items[3].push_back(pStatic3[REC_OSD_COLS+REC_OSD_ROWS]);

	
	rtSubPage.left = m_Rect.left+25;
	rtSubPage.top = m_Rect.top+50;
	rtSubPage.right = m_Rect.left+m_Rect.Width()-30;
	rtSubPage.bottom = m_Rect.top+m_Rect.Height()-175;

		
	int j = 0;
	for(i=0; i<2; i++)
	{
		CTRLPROC ctrlProc = NULL;
		switch(i)
		{
			case 0:
				ctrlProc = (CTRLPROC)&CPageRecordFrameWork::OnChNameCheck3;
				break;
			case 1:
				ctrlProc = (CTRLPROC)&CPageRecordFrameWork::OnTimeCheck3;
				break;
			default:
				break;
		}
		
		for(j=0; j<rows; j++)
		{
			pTable30->GetTableRect(i+1,j+1,&tmpRt);
			//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
			pCheckBox3[i*REC_OSD_ROWS+j] = 
			               CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
						                    rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+23), 
						                    this,styleEditable,ctrlProc);
	
			items[3].push_back(pCheckBox3[i*REC_OSD_ROWS+j]);
		}
	}

	for(i=0; i<rows; i++)
	{
		pTable30->GetTableRect(3,i+1,&tmpRt);
		pButton3[i] = CreateButton(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
					   rtSubPage.left+tmpRt.left+94,rtSubPage.top+tmpRt.top+25), 
					   this, "&CfgPtn.Setup", (CTRLPROC)&CPageRecordFrameWork::OnSetup, 
					   NULL, buttonNormalBmp);
		pButton3[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);

		items[3].push_back(pButton3[i]);
	}


	/*
	rtSubPage.left = m_Rect.left+25;
	rtSubPage.top = m_Rect.top+265;
	rtSubPage.right = m_Rect.left+m_Rect.Width()-30;
	rtSubPage.bottom = m_Rect.top+m_Rect.Height()-112;
	*/

	rtSubPage.left = m_Rect.left+25;
	rtSubPage.top = m_Rect.top+m_Rect.Height()-137;//m_Rect.top+255;
	rtSubPage.right = m_Rect.left+m_Rect.Width()-30;
	rtSubPage.bottom = m_Rect.top+m_Rect.Height()-106;

	
	pTable31 = CreateTableBox(&rtSubPage,this, 4,1,0);
	pTable31->SetColWidth(0,60);
	pTable31->SetColWidth(1,180);
	pTable31->SetColWidth(2,180);

	items[3].push_back(pTable31);

	for(i=REC_OSD_ROWS*2; i<REC_OSD_ROWS*2+3; i++)
	{
		pTable31->GetTableRect(i-REC_OSD_ROWS*2,0,&tmpRt);

		//if(i == REC_OSD_ROWS*2)
		{
			 //all checkbox
			 pCheckBox3[i] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
									rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+23), 
									this, styleEditable,(CTRLPROC)&CPageRecordFrameWork::OnCheckAll3);
		}
        /*
		else
		{
			 pCheckBox3[i] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
									rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+23), 
									this);
		}
		*/
		
		items[3].push_back(pCheckBox3[i]);		
	}

	pTable31->GetTableRect(3,0,&tmpRt);
	pButton3[REC_OSD_ROWS] = CreateButton(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+2, 
				   rtSubPage.left+tmpRt.left+94,rtSubPage.top+tmpRt.top+25), 
				   this, "&CfgPtn.Setup", (CTRLPROC)&CPageRecordFrameWork::OnSetup, 
				   NULL, buttonNormalBmp);
	pButton3[REC_OSD_ROWS]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push,pBmp_button_normal);
	

	items[3].push_back(pButton3[REC_OSD_ROWS]);

	#endif
	
	ShowSubPage(3,FALSE);
}

void CPageRecordFrameWork::InitPage4()
{
	char* statictext[1] = {
		"&CfgPtn.RecycleRecord",
	};

	CRect rtSubPage(m_Rect.left+25, m_Rect.top+50, 
		            m_Rect.left+m_Rect.Width()-30,
		            m_Rect.top+m_Rect.Height()-110);

	//printf("dddddddd  %d \n",m_Rect.top+m_Rect.Height()-170 - (m_Rect.top+43));
	pTable40 = CreateTableBox(&rtSubPage,this, 2,9,0);
	items[4].push_back(pTable40);

	CRect tmpRt;
	pTable40->GetTableRect(0,0,&tmpRt);
	pStatic4[0] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, 
                                rtSubPage.top+tmpRt.top,
                                rtSubPage.left+tmpRt.right,
                                rtSubPage.top+tmpRt.bottom), 
                                this, statictext[0]);
	items[4].push_back(pStatic4[0]);

	pTable40->GetTableRect(1,0,&tmpRt);
	pCheckBox4[0] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+2, 
			                        rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+22), 
			                        this);
	items[4].push_back(pCheckBox4[0]);

	ShowSubPage(4,FALSE);
}

void CPageRecordFrameWork::OnComboBox_snap_chn_change()
{
	int chn = pComboBox50->GetCurSel();

	if (chn < GetMaxChnNum())
	{
		if (bizData_GetSnapChnPara(chn, &P5SnapParam[chn]) != 0)
		{
			printf("%s: bizData_GetSnapChnPara() failed\n", __FUNCTION__);
		}
		else
		{
			/*
			printf("TimeSnap: enable: %d, Res: %d, Quality: %d, Interval: %d\n", 
				P5SnapParam[chn].TimeSnap.Enable, P5SnapParam[chn].TimeSnap.Res,
				P5SnapParam[chn].TimeSnap.Quality, P5SnapParam[chn].TimeSnap.Interval);

			printf("EventSnap: enable: %d, Res: %d, Quality: %d, Interval: %d\n", 
				P5SnapParam[chn].EventSnap.Enable, P5SnapParam[chn].EventSnap.Res,
				P5SnapParam[chn].EventSnap.Quality, P5SnapParam[chn].EventSnap.Interval);
			*/
			pCheckBox5[0]->SetValue(P5SnapParam[chn].TimeSnap.Enable);
			pCheckBox5[1]->SetValue(P5SnapParam[chn].EventSnap.Enable);
			
			pComboBox5[0][0]->SetCurSel(P5SnapParam[chn].TimeSnap.Res);
			pComboBox5[0][1]->SetCurSel(P5SnapParam[chn].TimeSnap.Quality);
			pComboBox5[0][2]->SetCurSel(P5SnapParam[chn].TimeSnap.Interval);
			pComboBox5[1][0]->SetCurSel(P5SnapParam[chn].EventSnap.Res);
			pComboBox5[1][1]->SetCurSel(P5SnapParam[chn].EventSnap.Quality);
			pComboBox5[1][2]->SetCurSel(P5SnapParam[chn].EventSnap.Interval);

			pComboBox5[0][0]->Enable(P5SnapParam[chn].TimeSnap.Enable);
			pComboBox5[0][1]->Enable(P5SnapParam[chn].TimeSnap.Enable);
			pComboBox5[0][2]->Enable(P5SnapParam[chn].TimeSnap.Enable);
			pComboBox5[1][0]->Enable(P5SnapParam[chn].EventSnap.Enable);
			pComboBox5[1][1]->Enable(P5SnapParam[chn].EventSnap.Enable);
			pComboBox5[1][2]->Enable(P5SnapParam[chn].EventSnap.Enable);
		}
	}
	else //all
	{
		pCheckBox5[0]->SetValue(TRUE);
		pCheckBox5[1]->SetValue(TRUE);
		pComboBox5[0][0]->Enable(TRUE);
		pComboBox5[0][1]->Enable(TRUE);
		pComboBox5[0][2]->Enable(TRUE);
		pComboBox5[1][0]->Enable(TRUE);
		pComboBox5[1][1]->Enable(TRUE);
		pComboBox5[1][2]->Enable(TRUE);
		
		pComboBox5[0][0]->SetCurSel(1);//704*576
		pComboBox5[0][1]->SetCurSel(1);//中
		pComboBox5[0][2]->SetCurSel(4);//5s
		pComboBox5[1][0]->SetCurSel(2);
		pComboBox5[1][1]->SetCurSel(1);
		pComboBox5[1][2]->SetCurSel(4);
	}
}

void CPageRecordFrameWork::OnCheckBox5()
{
	VD_BOOL flag;
	flag = pCheckBox5[0]->GetValue() ? TRUE : FALSE;
	pComboBox5[0][0]->Enable(flag);
	pComboBox5[0][1]->Enable(flag);
	pComboBox5[0][2]->Enable(flag);

	flag = pCheckBox5[1]->GetValue() ? TRUE : FALSE;
	pComboBox5[1][0]->Enable(flag);
	pComboBox5[1][1]->Enable(flag);
	pComboBox5[1][2]->Enable(flag);
}

void CPageRecordFrameWork::InitPage5()
{
	int i, loop;

	P5SnapParam = (SGuiSnapChnPara *)malloc(sizeof(SGuiSnapChnPara)*GetMaxChnNum());
	if(!P5SnapParam)
	{
		printf("CPageRecordFrameWork::InitPage5() P5SnapParam\n");  
		exit(1);
	}
	
	char* statictext[] = {
		"&CfgPtn.SelectPlayChn",	//通道选择
		"&CfgPtn.ParamType",	//参数类型
		"&CfgPtn.Enable",		//启用
		"&CfgPtn.Resolution",	//分辨率
		"&CfgPtn.SnapQuality",	//图片质量
		"&CfgPtn.SnapInterval",	//抓图时间间隔
	};

	CRect rtSubPage(m_Rect.left+25, m_Rect.top+50, 
		            m_Rect.left+m_Rect.Width()-30,
		            m_Rect.top+m_Rect.Height()-110);

	//printf("dddddddd  %d \n",m_Rect.top+m_Rect.Height()-170 - (m_Rect.top+43));
	pTable50 = CreateTableBox(&rtSubPage,this, 3,9,0);
	items[5].push_back(pTable50);

	CRect tmpRt;
	loop = sizeof(statictext)/sizeof(statictext[0]);
	for (i=0; i<loop; i++)
	{
		pTable50->GetTableRect(0,i,&tmpRt);
		pStatic5[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, 
	                                rtSubPage.top+tmpRt.top,
	                                rtSubPage.left+tmpRt.right,
	                                rtSubPage.top+tmpRt.bottom), 
	                                this, statictext[i]);
		items[5].push_back(pStatic5[i]);
	}
	
	char* statictext1[] = {
		"&CfgPtn.SnapTime",		//定时抓图
		"&CfgPtn.SnapEvent",	//事件抓图
	};
	for (i=0; i<2; i++)
	{
		pTable50->GetTableRect(i+1,1,&tmpRt);
		pStatic5[i+5] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, 
	                                rtSubPage.top+tmpRt.top,
	                                rtSubPage.left+tmpRt.right,
	                                rtSubPage.top+tmpRt.bottom), 
	                                this, statictext1[i]);
		items[5].push_back(pStatic5[i+5]);
	}
	

	pTable50->GetTableRect(1,0,&tmpRt);
	pComboBox50 = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
									rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
									this, NULL, NULL, (CTRLPROC)&CPageRecordFrameWork::OnComboBox_snap_chn_change);
	// 1,2...16,all
	GetScrStrInitComboxSelDo((void*)pComboBox50,
		GSR_CONFIG_RECORD_SNAP_CHN_CHOICE, 
		EM_GSR_COMBLIST, 
		0);
	//printf("%s pComboBox50->GetCount: %d\n", __FUNCTION__, pComboBox50->GetCount());
	pComboBox50->SetBkColor(VD_RGB(67,77,87));
	items[5].push_back(pComboBox50);

	//定时抓图 、事件抓图
	for (i=0; i<2; i++)
	{
		//启用使能
		pTable50->GetTableRect(i+1, 2, &tmpRt);
		pCheckBox5[i] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+2, 
			                        rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+22), 
			                        this, styleEditable, (CTRLPROC)&CPageRecordFrameWork::OnCheckBox5);
		items[5].push_back(pCheckBox5[i]);
		
		/******************************
		;CfgPtn.Resolution=分辨率
		;CfgPtn.AUTO=AUTO
		;CfgPtn.704*576=704*576(4CIF)
		;CfgPtn.352*288=352*288(CIF)
		;CfgPtn.176*144=176*144(QCIF)
		******************************/
		pTable50->GetTableRect(i+1, 3, &tmpRt);
		pComboBox5[i][0] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
										rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
										this, NULL);
		
		GetScrStrInitComboxSelDo((void*)pComboBox5[i][0],
			GSR_CONFIG_RECORD_SNAP_RES, 
			EM_GSR_COMBLIST, 
			0);
		
		//printf("%s SnapResolution GetCount: %d\n", __FUNCTION__, pComboBox5[i][0]->GetCount());
		pComboBox5[i][0]->SetBkColor(VD_RGB(67,77,87));
		items[5].push_back(pComboBox5[i][0]);
		/****************************
		图片质量
		;CfgPtn.SnapQuality=图片质量
		;CfgPtn.SnapHigh=高
		;CfgPtn.SnapMedium=中
		;CfgPtn.SnapLow=低
		****************************/
		pTable50->GetTableRect(i+1, 4, &tmpRt);
		pComboBox5[i][1] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
										rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
										this, NULL);
		
		GetScrStrInitComboxSelDo((void*)pComboBox5[i][1],
			GSR_CONFIG_RECORD_SNAP_QUALITY, 
			EM_GSR_COMBLIST, 
			0);
		//printf("%s SnapQuality GetCount: %d\n", __FUNCTION__, pComboBox5[i][1]->GetCount());
		pComboBox5[i][1]->SetBkColor(VD_RGB(67,77,87));
		items[5].push_back(pComboBox5[i][1]);
		/***************************************
		;CfgPtn.SnapInterval=抓图时间间隔
		;CfgPtn.1s=1秒
		;CfgPtn.2s=2秒
		;CfgPtn.3s=3秒
		;CfgPtn.4s=4秒
		;CfgPtn.5s=5秒
		;CfgPtn.10m=10分钟
		;CfgPtn.30m=30分钟
		;CfgPtn.1h=1小时
		;CfgPtn.12h=12小时
		;CfgPtn.24h=24小时
		***************************************/
		pTable50->GetTableRect(i+1, 5, &tmpRt);
		pComboBox5[i][2] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
										rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
										this, NULL);
		
		GetScrStrInitComboxSelDo((void*)pComboBox5[i][2],
			GSR_CONFIG_RECORD_SNAP_INTERVAL, 
			EM_GSR_COMBLIST, 
			0);
		//printf("%s SnapInterval GetCount: %d\n", __FUNCTION__, pComboBox5[i][2]->GetCount());
		pComboBox5[i][2]->SetBkColor(VD_RGB(67,77,87));
		items[5].push_back(pComboBox5[i][2]);
	}
	
	ShowSubPage(5,FALSE);
}

void CPageRecordFrameWork::SwitchPage( int subID)
{
	
	if (subID<0 || subID>=RECCONFIG_SUBPAGES )
	{
		return;
	}

	if (subID == curID)
	{
		return;
	}

	//隐藏当前子页面，显示新的子页面
	//printf("yg SwitchPage 1, curID: %d, subID: %d\n", curID, subID);
	ShowSubPage(curID, FALSE);
	//printf("yg SwitchPage 2, curID: %d, subID: %d\n", curID, subID);;
	pButton[curID]->Enable(TRUE);
	
	curID = subID;
	//printf("yg SwitchPage 3, curID: %d, subID: %d\n", curID, subID);
	pButton[curID]->Enable(FALSE);
	//printf("yg SwitchPage 4, curID: %d, subID: %d\n", curID, subID);
	ShowSubPage(curID, TRUE);
	//printf("yg SwitchPage 5, curID: %d, subID: %d\n", curID, subID);
}

void CPageRecordFrameWork::ShowSubPage( int subID, BOOL bShow )
{
	if (subID<0 || subID>=RECCONFIG_SUBPAGES )
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

	//printf("ShowSubPage 2222\n");
}

void CPageRecordFrameWork::LoadTabData(int tabID)
{
	switch(tabID)
	{
		//printf("%d curID \n",curID);
		case 0://enable
		{
			AdjustEnableRows();
			AdjustAudioRows();
			
			for(int i=0; i<GetMaxChnNum(); i++)
			{
				int value = GetRecordEnable(i);
				bVideoEnable[i] = value;				
				//value = GetAudioEnable(i);
				//bAudioEnable[i] = value;
			}

			for(int i=0; i<GetAudioChnNum(); i++)
			{
				int value = GetAudioEnable(i);
				bAudioEnable[i] = value;
			}

			for(int i=0; i<GetEnableRows(); i++)
			{
				int nCh = m_page0*REC_ENABLE_ROWS+i;
				if(nCh >= GetMaxChnNum())
				{
					break;
				}
				
				pCheckBox0[i]->SetValue(bVideoEnable[nCh]);
				//pCheckBox0[i+REC_ENABLE_ROWS]->SetValue(bAudioEnable[nCh]);
			}

			for(int i=0; i<GetEnableAudioRows(); i++)
			{
				int nCh = m_page0*REC_ENABLE_ROWS+i;
				if(nCh >= GetAudioChnNum())
				{
					break;
				}				
				//pCheckBox0[i]->SetValue(bVideoEnable[nCh]);
				pCheckBox0[i+REC_ENABLE_ROWS]->SetValue(bAudioEnable[nCh]);
			}
		}break;
//#ifndef DISABLE_RECBITRATE
		case 1://code stream
		{
			AdjustStreamRows();
			
			#if 1
			SValue frameRateList[10];
			int nFrReal;
			int index;
			
			for(int i=0; i<GetMaxChnNum(); i++)
			{
				index = GetReso(i);
				resolution[i] = index;
				
				index = GetFramerate(index,i);
				framerate[i] = index;
				
				index = GetEncode(i);
				encode[i] = index;
				
				if(index == 0)
				{
					//CBR
					qualityEnable[i] = 0;
					quality[i] = 0;
				}
				else
				{
					//VBR
					qualityEnable[i] = 1;
					index = GetQuality(i);
					quality[i] = index;
				}
				
				index = GetBitrate(i);
				bitrate[i] = index;
			}
			
			#if 0
			for(int i=0; i<GetMaxChnNum(); i++)
			{
				printf("resolution[%d] = %d \n",i,resolution[i]);
				printf("framerate[%d] = %d \n",i,framerate[i]);
				printf("encode[%d] = %d \n",i,encode[i]);
				printf("quality[%d] = %d \n",i,quality[i]);
				printf("bitrate[%d] = %d \n",i,bitrate[i]);
			}
			#endif
			
			int start = 0;
			int end = 0;
			for(int i=0; i<GetStreamRows(); i++)
			{	
				//printf("********** chn %d,starttime = %lld \n",i,GetTimeTick());			
				int nCh = m_page1*REC_STREAM_ROWS+i;
				if(nCh >= GetMaxChnNum())
				{
					break;
				}

				pComboBox1[i]->SetCurSel(resolution[nCh]);
				
				int index = pComboBox1[i]->GetCurSel();
				
				//start = GetTimeTick();
				GetFrameRateList(index, frameRateList, &nFrReal, 10);
				//end = GetTimeTick();
				//printf(" %d , end - start = %d \n",i,end - start);
				
				pComboBox1[i+REC_STREAM_ROWS]->RemoveAll();
				for(int j=0; j<nFrReal; j++)
				{
					pComboBox1[i+REC_STREAM_ROWS]->AddString(frameRateList[j].strDisplay);
				}
                
                //int nFrameCount = pComboBox1[i+REC_STREAM_ROWS]->GetCount();
				
				pComboBox1[i+REC_STREAM_ROWS]->SetCurSel(framerate[nCh]); //framerate[i]
				pComboBox1[i+REC_STREAM_ROWS*2]->SetCurSel(encode[nCh]);
				pComboBox1[i+REC_STREAM_ROWS*3]->SetCurSel(quality[nCh]);
				pComboBox1[i+REC_STREAM_ROWS*3]->Enable(qualityEnable[nCh]);
				pComboBox1[i+REC_STREAM_ROWS*4]->SetCurSel(bitrate[nCh]);

				if(i == 0)
				{
					pComboBox1[REC_STREAM_ROWS*5+1]->RemoveAll();
					for(int j=0; j<nFrReal; j++)
					{
						pComboBox1[REC_STREAM_ROWS*5+1]->AddString(frameRateList[j].strDisplay);
					}
				}
			}

            char tmp[30] = {0};
            GetProductNumber(tmp);

            //R9508S只有前二路为D1,R9504S有四路D1
			if(0 == strcasecmp(tmp, "R9508S") || 0 == strcasecmp(tmp, "R9508"))//csp modify
			{
			    pComboBox1[REC_STREAM_ROWS*5]->SetCurSel(0);//cw_9508S pComboBox1[0]->GetCurSel()
		    }
			else
			{
                pComboBox1[REC_STREAM_ROWS*5]->SetCurSel(pComboBox1[0]->GetCurSel()); //与第一行一致
            }
            
			//pComboBox1[REC_STREAM_ROWS*5+1]->SetCurSel(pComboBox1[REC_STREAM_ROWS]->GetCurSel());
			//add by Lirl on Nov/17/2011
			int count = pComboBox1[REC_STREAM_ROWS*5+1]->GetCount();
			pComboBox1[REC_STREAM_ROWS*5+1]->SetCurSel(count-1);
            //end
			pComboBox1[REC_STREAM_ROWS*5+2]->SetCurSel(0/*pComboBox1[REC_STREAM_ROWS*2]->GetCurSel()*/);
			pComboBox1[REC_STREAM_ROWS*5+3]->SetCurSel(0/*pComboBox1[REC_STREAM_ROWS*3]->GetCurSel()*/);
			pComboBox1[REC_STREAM_ROWS*5+3]->Enable(pComboBox1[REC_STREAM_ROWS*5+2]->GetCurSel()?1:0);
			pComboBox1[REC_STREAM_ROWS*5+4]->SetCurSel(5/*pComboBox1[REC_STREAM_ROWS*4]->GetCurSel()*/);
			//CheckFrameLeft();
			#endif
			//pStatic1[11]->SetText(GetCodeStreamLeft());
		}break;//record time
//#endif
#ifndef DISABLE_PREREC_TIME
		case 2:
		{	
			AdjustTimeRows();
			
			#if 1
			int index = 0;
			for(int i=0; i<GetMaxChnNum(); i++)
			{
				index = GetTimeBeforeAlarm(i);
				preRecTime[i] = index;

				index = GetTimeAfterAlarm(i);
				delayRecTime[i] = index;

				index = GetExpiration(i);
				recExpireTime[i] = index;
			}

			for(int i=0; i<GetRecTimeRows(); i++)
			{
				int nCh = m_page2*REC_TIME_ROWS+i;
				if(nCh >= GetMaxChnNum())
				{
					break;
				}

				pComboBox2[i]->SetCurSel(preRecTime[nCh]);
				pComboBox2[i+REC_TIME_ROWS]->SetCurSel(delayRecTime[nCh]);
				pComboBox2[i+REC_TIME_ROWS*2]->SetCurSel(recExpireTime[nCh]);
			}

			index = GetTimeBeforeAlarm(0);
			pComboBox2[3*REC_TIME_ROWS]->SetCurSel(index);

			index = GetTimeAfterAlarm(0);
			pComboBox2[3*REC_TIME_ROWS+1]->SetCurSel(index);

			index = GetExpiration(0);
			pComboBox2[3*REC_TIME_ROWS+2]->SetCurSel(index);
			#endif			
		}break;
#endif
#ifndef DISABLE_STAMP

		case 3://osd Stamp
		{
			AdjustOsdRows();
			
			#if 1
			int value = 0;
			for(int i=0; i<GetMaxChnNum(); i++)
			{
				value = GetChnNameCheck(i);
				bChNameEnable[i] = value;

				value = GetTimeStampCheck(i);
				bTimeEnable[i] = value;
			}

			for(int i=0; i<GetOsdRows(); i++)
			{
				int nCh = m_page3*REC_OSD_ROWS+i;
				if(nCh >= GetMaxChnNum())
				{
					break;
				}

				pCheckBox3[i]->SetValue(bChNameEnable[nCh]);
				pCheckBox3[i+REC_OSD_ROWS]->SetValue(bTimeEnable[nCh]);

                //Modify by Lirl on Feb/02/2012,通道名及时间戳同时选中时设置按钮才可用
                if (bChNameEnable[nCh] || bTimeEnable[nCh])
                {
                    //bSetButtomEnable[nCh] = 1;
                    pButton3[i]->Enable(TRUE);
                }
                else
                {
                    //bSetButtomEnable[nCh] = 0;
                    pButton3[i]->Enable(FALSE);
                }
                //end
			}

            BOOL bAllCheck = TRUE;
		/*
            for(int i = REC_OSD_ROWS*2; i< REC_OSD_ROWS*2 + 3; i++)
            {
                if(!pCheckBox3[i]->GetValue())
                {
                    bAllCheck &= FALSE;
                    break;
                }
            }
		*/
		    bAllCheck=((pCheckBox3[REC_OSD_ROWS*2]->GetValue())
		                &&((pCheckBox3[REC_OSD_ROWS*2+1]->GetValue()) ||(pCheckBox3[REC_OSD_ROWS*2+2]->GetValue())));
			pButton3[REC_OSD_ROWS]->Enable(bAllCheck);
			#endif
		}break;
#endif
		case 4://cyling record
		{	
			int value = GetCyclingRecCheck();
			pCheckBox4[0]->SetValue(value);
		}break;
		case 5://Snap
		{	
			int i;
			int nChMax = GetVideoMainNum();

			for (i=0; i<nChMax; i++)
			{
				if (bizData_GetSnapChnPara(i, &P5SnapParam[i]) != 0)
				{
					printf("%s: bizData_GetSnapChnPara() failed\n", __FUNCTION__);
				}
				
			}

			pComboBox50->SetCurSel(0);//打开时，显示通道0
			pCheckBox5[0]->SetValue(P5SnapParam[0].TimeSnap.Enable);
			pComboBox5[0][0]->SetCurSel(P5SnapParam[0].TimeSnap.Res);
			pComboBox5[0][1]->SetCurSel(P5SnapParam[0].TimeSnap.Quality);
			pComboBox5[0][2]->SetCurSel(P5SnapParam[0].TimeSnap.Interval);
			pComboBox5[0][0]->Enable(P5SnapParam[0].TimeSnap.Enable);
			pComboBox5[0][1]->Enable(P5SnapParam[0].TimeSnap.Enable);
			pComboBox5[0][2]->Enable(P5SnapParam[0].TimeSnap.Enable);

			pCheckBox5[1]->SetValue(P5SnapParam[0].EventSnap.Enable);
			pComboBox5[1][0]->SetCurSel(P5SnapParam[0].EventSnap.Res);
			pComboBox5[1][1]->SetCurSel(P5SnapParam[0].EventSnap.Quality);
			pComboBox5[1][2]->SetCurSel(P5SnapParam[0].EventSnap.Interval);
			pComboBox5[1][0]->Enable(P5SnapParam[0].EventSnap.Enable);
			pComboBox5[1][1]->Enable(P5SnapParam[0].EventSnap.Enable);
			pComboBox5[1][2]->Enable(P5SnapParam[0].EventSnap.Enable);

			
		}break;
        default:
        {} break;
	}
}

void CPageRecordFrameWork::AdjustAudioRows()
{
	int i;
	int tem = REC_ENABLE_ROWS - (audio_page*REC_ENABLE_ROWS - GetAudioChnNum());
	
	if (audio_page > 1)
	{
		if (m_page0 == audio_page - 1)
		{
			for (i = tem; i < REC_ENABLE_ROWS; i++)
			{
				if (pCheckBox0[i+REC_ENABLE_ROWS])
				{
					pCheckBox0[i+REC_ENABLE_ROWS]->Show(FALSE);
				}
			}
			
			for (i = 0; i < tem; i++)
			{
				if (pCheckBox0[i+REC_ENABLE_ROWS])
				{
					pCheckBox0[i+REC_ENABLE_ROWS]->Show(1);
				}
			}
		}
		else if (m_page0 >= audio_page && m_page0 < m_maxPage0)
		{
			for (i = 0; i < REC_ENABLE_ROWS; i++)
			{
				pCheckBox0[i+REC_ENABLE_ROWS]->Show(FALSE);
			}
		}
		else 
		{
			for (i = 0; i < REC_ENABLE_ROWS; i++)
			{
				pCheckBox0[i+REC_ENABLE_ROWS]->Show(TRUE);
			}
		}
	}
	else
	{
		if (m_page0 == audio_page - 1)
		{
			for (i = 0; i < GetAudioChnNum(); i++)
			{
				pCheckBox0[i+REC_ENABLE_ROWS]->Show(TRUE);
			}
		}
		else
		{
			for (i = 0; i < GetAudioChnNum(); i++)
			{
				pCheckBox0[i+REC_ENABLE_ROWS]->Show(FALSE);
			}
		}
	}
}

void CPageRecordFrameWork::AdjustEnableRows()
{
	int firstHide = REC_ENABLE_ROWS-(m_maxPage0*REC_ENABLE_ROWS-GetMaxChnNum());
	int lastHide = REC_ENABLE_ROWS;
	if (m_page0 == m_maxPage0-1)
	{
		for (int i=firstHide; i<lastHide; i++)
		{
			if (pStatic0[i+REC_ENABLE_COLS])
			{
				pStatic0[i+REC_ENABLE_COLS]->Show(FALSE,TRUE);
			}
			if (pCheckBox0[i])
			{
				pCheckBox0[i]->Show(FALSE,TRUE);
			}
			//if(pCheckBox0[i+REC_ENABLE_ROWS])
			//{
			//	pCheckBox0[i+REC_ENABLE_ROWS]->Show(FALSE,TRUE);
			//}	
		}
	}
	else
	{
		for (int i=firstHide; i<lastHide; i++)
		{
			if (pStatic0[i+REC_ENABLE_COLS])
			{
				pStatic0[i+REC_ENABLE_COLS]->Show(TRUE,TRUE);
			}
			if (pCheckBox0[i])
			{
				pCheckBox0[i]->Show(TRUE,TRUE);
			}
		}
	}
}


void CPageRecordFrameWork::AdjustStreamRows()
{
	int firstHide = REC_STREAM_ROWS-(m_maxPage1*REC_STREAM_ROWS-GetMaxChnNum());
	int lastHide = REC_STREAM_ROWS;
	if (m_page1 == m_maxPage1-1)
	{
		for (int i=firstHide; i<lastHide; i++)
		{
			if (pStatic1[i+REC_STREAM_COLS]
				&& pComboBox1[i]
				&& pComboBox1[i+REC_STREAM_ROWS]
				&& pComboBox1[i+REC_STREAM_ROWS*2]
				&& pComboBox1[i+REC_STREAM_ROWS*3]
				&& pComboBox1[i+REC_STREAM_ROWS*4])
			{
				pStatic1[i+REC_STREAM_COLS]->Show(FALSE,TRUE);

				pComboBox1[i]->Show(FALSE,TRUE);
				pComboBox1[i+REC_STREAM_ROWS]->Show(FALSE,TRUE);
				pComboBox1[i+REC_STREAM_ROWS*2]->Show(FALSE,TRUE);
				pComboBox1[i+REC_STREAM_ROWS*3]->Show(FALSE,TRUE);
				pComboBox1[i+REC_STREAM_ROWS*4]->Show(FALSE,TRUE);
			}	
		}
	}
	else
	{
		for (int i=firstHide; i<lastHide; i++)
		{
			if (pStatic1[i+REC_STREAM_COLS]
				&& pComboBox1[i]
				&& pComboBox1[i+REC_STREAM_ROWS]
				&& pComboBox1[i+REC_STREAM_ROWS*2]
				&& pComboBox1[i+REC_STREAM_ROWS*3]
				&& pComboBox1[i+REC_STREAM_ROWS*4])
			{
				pStatic1[i+REC_STREAM_COLS]->Show(TRUE,TRUE);

				pComboBox1[i]->Show(TRUE,TRUE);
				pComboBox1[i+REC_STREAM_ROWS]->Show(TRUE,TRUE);
				pComboBox1[i+REC_STREAM_ROWS*2]->Show(TRUE,TRUE);
				pComboBox1[i+REC_STREAM_ROWS*3]->Show(TRUE,TRUE);
				pComboBox1[i+REC_STREAM_ROWS*4]->Show(TRUE,TRUE);
			}
		}
	}
}

void CPageRecordFrameWork::AdjustTimeRows()
{
	
	int firstHide = REC_TIME_ROWS-(m_maxPage2*REC_TIME_ROWS-GetMaxChnNum());
	int lastHide = REC_TIME_ROWS;
	if(m_page2 == m_maxPage2-1)
	{
		for(int i=firstHide; i<lastHide; i++)
		{
			if(pStatic2[i+REC_TIME_COLS]
				&& pComboBox2[i]
				&& pComboBox2[i+REC_TIME_ROWS]
				&& pComboBox2[i+REC_TIME_ROWS*2])
			{
				pStatic2[i+REC_TIME_COLS]->Show(FALSE,TRUE);
				pComboBox2[i]->Show(FALSE,TRUE);
				pComboBox2[i+REC_TIME_ROWS]->Show(FALSE,TRUE);
				//pComboBox2[i+REC_TIME_ROWS*2]->Show(FALSE,TRUE);			
			}
		}
	}
	else
	{
		for(int i=firstHide; i<lastHide; i++)
		{
			if(pStatic2[i+REC_TIME_COLS]
				&& pComboBox2[i]
				&& pComboBox2[i+REC_TIME_ROWS]
				&& pComboBox2[i+REC_TIME_ROWS*2])
			{
				pStatic2[i+REC_TIME_COLS]->Show(TRUE,TRUE);
				pComboBox2[i]->Show(TRUE,TRUE);
				pComboBox2[i+REC_TIME_ROWS]->Show(TRUE,TRUE);
				//pComboBox2[i+REC_TIME_ROWS*2]->Show(TRUE,TRUE);
			}
		}
	}
}


void CPageRecordFrameWork::AdjustOsdRows()
{
	
	int firstHide = REC_OSD_ROWS-(m_maxPage3*REC_OSD_ROWS-GetMaxChnNum());
	int lastHide = REC_OSD_ROWS;
	if(m_page3 == m_maxPage3-1)
	{
		for(int i=firstHide; i<lastHide; i++)
		{	
			if(pStatic3[i+REC_OSD_COLS]
				&& pCheckBox3[i]
				&& pCheckBox3[i+REC_OSD_ROWS]
				&& pButton3[i])
			{
				pStatic3[i+REC_OSD_COLS]->Show(FALSE,TRUE);
				pCheckBox3[i]->Show(FALSE,TRUE);
				pCheckBox3[i+REC_OSD_ROWS]->Show(FALSE,TRUE);
				pButton3[i]->Show(FALSE,TRUE);
			}		
		}
	}
	else
	{
		for(int i=firstHide; i<lastHide; i++)
		{
			if(pStatic3[i+REC_OSD_COLS]
				&& pCheckBox3[i]
				&& pCheckBox3[i+REC_OSD_ROWS]
				&& pButton3[i])
			{									
				pStatic3[i+REC_OSD_COLS]->Show(TRUE,TRUE);
				pCheckBox3[i]->Show(TRUE,TRUE);
				pCheckBox3[i+REC_OSD_ROWS]->Show(TRUE,TRUE);
				pButton3[i]->Show(TRUE,TRUE);
			}
		}
	}	
}

VD_BOOL CPageRecordFrameWork::UpdateData( UDM mode )
{
	static CPage** page = NULL;//cw_panel
	if(!page)
	{
		page = GetPage();
	}
	//printf("yg CPageRecordFrameWork mode: %d, curID: %d\n", mode, curID);
	if(UDM_OPEN == mode)
	{
		if(1 != curID)
		{
			LoadTabData(1);
		}
		
		LoadTabData(curID);
		/*for(int i = 0; i < 5; i++)
		{
			//printf("%d ", i);
			LoadTabData(i);
		}
		*/
		for(int i = 0; i < RECCONFIG_SUBPAGES; i++)
		{
			if(i == curID)
			{
				//ShowSubPage(i, 1);
			}
			else
			{
				ShowSubPage(i, 0);
			}
		}
        	LoadButtomCtl();
	}
	else if(UDM_CLOSED == mode)
	{
		//SwitchPage(0);
		pCheckBox0[2*REC_ENABLE_ROWS]->SetValue(FALSE);
		pCheckBox1->SetValue(FALSE);
		pCheckBox2->SetValue(FALSE);
		pCheckBox3[2*REC_OSD_ROWS]->SetValue(FALSE);
		
		char tmp2[20] = {0};
		SBizDvrInfo sInfo;
		s32 ret = BizGetDvrInfo(&sInfo);
		if(ret == 0)
		{
			strcpy(tmp2, sInfo.sproductnumber);
		}
		if((0 == strcasecmp(tmp2, "NR3132")) || (0 == strcasecmp(tmp2, "NR3124")))
		{
			
		}
		else
		{
			for(int i=0; i<GetVideoMainNum(); i++)//cw_panel
			{
				(((CPageDesktop*)page[EM_PAGE_DESKTOP])->m_vChannelName[i])->SetText((((CPageDesktop*)page[EM_PAGE_DESKTOP])->m_vChannelName[i])->GetText());
			}
		}
		
	        if(m_IsLoader)
	        {
			m_IsLoader = 0;//~m_IsLoader;
			LoadButtomCtl();
			SetTitle(GetParsedString("&CfgPtn.Record"));
	        }
	        else
	        {
			((CPageConfigFrameWork *)(this->GetParent()))->ReflushItemName();
	        }
	}
	else if(UDM_GUIDE == mode)
	{
	        m_IsLoader = 1;//~m_IsLoader;
	        SetTitle(GetParsedString("&CfgPtn.Guide"));
		
	        SwitchPage(1);
		//LoadTabData(curID);
	}
	
	return TRUE;
}

void CPageRecordFrameWork::WriteLogs(int PageIndex)
{
	switch(PageIndex)
	{
		case 0://enable
		{
			for(int i=0; i<GetMaxChnNum(); i++)
			{
				if(GetRecordEnable(i) != bVideoEnable[i])
				{
					BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_REC_ENABLE);
					break;
				}
			}
            
			for(int i=0; i<GetMaxChnNum(); i++)
			{
				if(GetAudioEnable(i) != bAudioEnable[i])
				{
					BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_REC_AUDIO_ENABLE);
					break;
				}
			}
		}break;
		case 1://code stream
		{
			BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_REC_CODESTREAM);
		}break;
		case 2://record time
		{
			int index = 0;
			for(int i=0; i<GetMaxChnNum(); i++)
			{
				if(GetTimeBeforeAlarm(i) != preRecTime[i])
				{
					BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_TIME_BEFORE_ALARM);
					break;
				}
			}
            
			for(int i=0; i<GetMaxChnNum(); i++)
			{
				if(GetTimeAfterAlarm(i) != delayRecTime[i])
				{
					BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_TIME_AFTER_ALARM);
					break;
				}
			}
            
			for(int i=0; i<GetMaxChnNum(); i++)
			{
				if(GetExpiration(i) != recExpireTime[i])
				{
					BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_REC_EXPIRETIME);
					break;
				}
			}
		}break;
		case 3://osd
		{			
			BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_OSD);
		}break;		
		default:
			break;
	}
}


void CPageRecordFrameWork::OnCheckBoxSelectAll()
{

}

void CPageRecordFrameWork::OnCheckBoxSelectAll2()
{

}

extern int GetCifEncCap();

void CPageRecordFrameWork::CheckFrameLeft()
{
	char szTmp[32]={0};

	// later change by machine type
	nRemainCif = GetCifEncCap(); // assume 4D1==16CIF

	int nQCif = 0;
	int nVal;
	
	for(int i=0; i<GetMaxChnNum(); i++)
	{
		//if( strcmp(pComboBox1[i]->GetString(pComboBox1[i]->GetCurSel()), "Cif")==0 ) // or cif, Cif
		if(resolution[i] == 1)//cif
		{
			nVal = GetFrameRateVal( 1, framerate[i]);

			//printf("nVal = %d framerate[i] = %d\n",nVal,framerate[i]);
			if(nVal>0)
			{
				nRemainCif -= nVal;
			}
			//printf("codestream %d\n", nVal);
		}
		//else if( strcmp(pComboBox1[i]->GetString(pComboBox1[i]->GetCurSel()), "D1")==0 )
		else if(resolution[i] == 2)	//D1
		{
			nVal = GetFrameRateVal( 2, framerate[i]);
			if(nVal>0)
			{
				nRemainCif -= nVal*4;
			}
			//printf("codestream %d\n", nVal);
		}
		//else if( strcmp(pComboBox1[i]->GetString(pComboBox1[i]->GetCurSel()), "QCif")==0 )
		else if(resolution[i] == 0) //qcif
		{
			nVal = GetFrameRateVal( 0, framerate[i]);
			if(nVal>0)
			{
				nQCif += nVal;
			}
			//printf("codestream %d\n", nVal);
		}
	}
	
	nRemainCif -= (nQCif+15)/16;

	//printf("%d %d\n",nRemainCif, nRemainCif>>2);
	
	sprintf(szTmp, ": %d(CIF), %d(D1).", nRemainCif, nRemainCif>>2);

	//printf("%s", szTmp);
	
	pStatic1[REC_STREAM_ROWS+REC_STREAM_COLS+1]->SetText(szTmp);
	//pStatic1[REC_STREAM_ROWS+REC_STREAM_COLS+1]->Draw();
}

void CPageRecordFrameWork::SetInfo(char* szInfo)
{
    if (m_IsLoader)
    {
        return;
    }
    
	pIconInfo->Show(TRUE);
	
	pInfoBar->SetText(szInfo);

}

void CPageRecordFrameWork::ClearInfo()
{
    if (m_IsLoader)
    {
        return;
    }
    
	pIconInfo->Show(FALSE);
	
	pInfoBar->SetText("");

}

void CPageRecordFrameWork::LoadButtomCtl()
{
    if (m_IsLoader)
    {
        pButton[0]->Show(FALSE);
#ifndef DISABLE_RECBITRATE
        pButton[1]->Show(FALSE);
#endif
        pButton[2]->Show(FALSE);
#ifndef DISABLE_STAMP
        pButton[3]->Show(FALSE);
#endif

        pButton[4]->Show(FALSE);
        //pButton[5]->Show(FALSE);
        pButton[6]->Show(FALSE);
        pButton[7]->Show(FALSE);

        pButton[9]->Show(TRUE);
        pButton[10]->Show(TRUE);
    }
    else
    {
        pButton[0]->Show(TRUE);
#ifndef DISABLE_RECBITRATE
        pButton[1]->Show(TRUE);
#endif
        pButton[2]->Show(TRUE);
#ifndef DISABLE_STAMP
        pButton[3]->Show(TRUE);
#endif
        pButton[4]->Show(TRUE);
        //pButton[5]->Show(TRUE);
        pButton[6]->Show(TRUE);
        pButton[7]->Show(TRUE);

        pButton[9]->Show(FALSE);
        pButton[10]->Show(FALSE);
    }
}

int CPageRecordFrameWork::ReflushStaticString(int nIdex)
{
    pStatic1[nIdex]->SetText(pStatic1[nIdex]->GetText());

    return TRUE;
}

VD_BOOL CPageRecordFrameWork::MsgProc( uint msg, uint wpa, uint lpa )
{

	int px,py;
	switch(msg)
    {
        case XM_RBUTTONDBLCLK:
        case XM_RBUTTONDOWN:
        {
            if (m_IsLoader)
            {
                return TRUE;
            }
        } break;
    	case XM_MOUSEWHEEL:   //cw_scrollbar
    		{
    			int px = VD_HIWORD(lpa);
    			int py = VD_LOWORD(lpa);
    			CItem* pItem = GetItemAt(px, py);
    			if(pItem == NULL)
    			{
    				if(GetMaxChnNum()>REC_ENABLE_ROWS)
    				{
    					if (curID == 0)
    					{
    						pScroll0->MsgProc(msg, wpa, lpa);
    						break;
    					}
    					else if (curID == 1)
    					{
    						pScroll1->MsgProc(msg, wpa, lpa);
    						break;
    					}
    					else if (curID == 2)
    					{
    						pScroll2->MsgProc(msg, wpa, lpa);
    						break;
    					}
    					else if (curID == 3)
    					{
    						pScroll3->MsgProc(msg, wpa, lpa);
    						break;
    					}	
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
    				if(temp == pCheckBox0[2*REC_ENABLE_ROWS]
    					|| temp == pCheckBox1 
    					|| temp == pCheckBox2 
    					|| temp == pCheckBox3[2*REC_OSD_ROWS] )
    				{
    					SetInfo("&CfgPtn.SetAllChannels");
    				}
    				else
    				{
    					ClearInfo();
    				}

    				last = temp;
    			}
    		}break;
        case XM_LBUTTONDOWN:
        case XM_LBUTTONDBLCLK:
            {
                if (1 == curID)
                {
                    CItem* temp = GetFocusItem();

                    if (temp == pComboBox1[REC_STREAM_ROWS*5+1]
                         && pComboBox1[REC_STREAM_ROWS*5+1]->GetListBoxStatus())
                    {
                        return CPage::MsgProc(msg, wpa, lpa)? ReflushStaticString(1): FALSE;
                    }
                    else if (temp == pComboBox1[REC_STREAM_ROWS*5+4]
                            && pComboBox1[REC_STREAM_ROWS*5+4]->GetListBoxStatus())
                    {
                        return CPage::MsgProc(msg, wpa, lpa)? ReflushStaticString(5): FALSE;
                    }
                }
            } break;
        case XM_LBUTTONUP:
            {
                pComboBox2[REC_TIME_ROWS]->Draw();
            }break;
    	default:
    		//return 0;
    		break;
    }
	
	int ret=CPage::MsgProc(msg, wpa, lpa);
	pComboBox1[REC_STREAM_COLS*4]->Draw();
	return ret;

}




