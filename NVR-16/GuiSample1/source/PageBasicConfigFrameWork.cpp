#include "GUI/Pages/PageBasicConfigFrameWork.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/BizData.h"
#include "biz.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageInfoBox.h"
#include "GUI/Pages/PageConfigFrameWork.h"

//csp modify
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageDesktop.h"

#include "System/ntp_client.h"
#include "sg_platform.h"


extern "C" s32 ModAlarmSetTimeZoneOffset(int nOffset);
extern "C" s32 ModRecordSetTimeZoneOffset(int nOffset);

//csp modify
//static	std::vector<CItem*>	items[BASICCFG_SUBPAGES];
static	std::vector<CItem*>	items[BASICCFG_SUBPAGES+1];

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static VD_BITMAP * pBmp_tab_normal;
static VD_BITMAP * pBmp_tab_push;
static VD_BITMAP * pBmp_tab_select;

static VD_BITMAP * pBmp_infoicon;

//csp modify 20130421
//#define HIDE_NTP//QYJ

static CPageBasicConfigFrameWork* pagebasic = NULL;

static int GetMaxChnNum()
{
	return GetVideoMainNum();
}

static int synctimefromntp(int timesyncflag, char *ntpserveradress, int timeout)
{
	if(1 == timesyncflag)
	{
		int dateformatcursel = GetDateFormat();
		int timestandardcursel = GetTimeFormat();
		int timezone = GetTimeZone();
		int ipcsyncflag = GetTimeSyncToIPC();
		
		//int timesyncflag = GetNetworkSyscCheck();
		
		//char ntpserveradress[32] = {0};
		//GetNtpServer(ntpserveradress, 32);
		
		struct timeval tv;
		if(getTimeFromNtpserver(ntpserveradress, 0, timeout, &tv))
		{
			return -1;
		}
		
		struct timeval now;
		gettimeofday(&now, NULL);
		
		//time_t curr = time(NULL);
		//printf("curr=%d,now=%d,ntp=%d\n",curr,now.tv_sec,tv.tv_sec);
		
		if(abs(now.tv_sec-tv.tv_sec) > 1)
		{
			SBizDateTime sBizDT;
			memset(&sBizDT, 0, sizeof(sBizDT));
			
			if(0 == dateformatcursel)
			{
				sBizDT.emDateTimeFormat = EM_BIZDATETIME_YYYYMMDDHHNNSS;
			}
			else if(1 == dateformatcursel)
			{
				sBizDT.emDateTimeFormat = EM_BIZDATETIME_MMDDYYYYHHNNSS;
			}
			else if(2 == dateformatcursel)
			{
				sBizDT.emDateTimeFormat = EM_BIZDATETIME_DDMMYYYYHHNNSS;
			}
			else
			{
				sBizDT.emDateTimeFormat = EM_BIZDATETIME_YYYYMMDDHHNNSS;
			}
			
			if(0 == timestandardcursel)
			{
				sBizDT.emTimeFormat = EM_BIZTIME_24HOUR;
			}
			else if(1 == timestandardcursel)
			{
				sBizDT.emTimeFormat = EM_BIZTIME_12HOUR;
			}
			else
			{
				sBizDT.emTimeFormat = EM_BIZTIME_24HOUR;
			}
			
			time_t t = tv.tv_sec;
			u8 flag = 0;
			
			int i = 0;
			for(i = 0; i < GetVideoMainNum(); i++)
			{
				if(GetCurRecStatus(i))
				{
					break;
				}
			}
			
			if(i < GetVideoMainNum())
			{
				BizRecordStopAll();
				usleep(2000*1000); //多睡会儿吧，否则结束时间戳容易出问题
				flag = 1;
				
				t += 2;
			}
			
			struct tm tm_time;
			gmtime_r(&t, &tm_time);
			struct tm *ptm = &tm_time;
			
			sBizDT.nYear = ptm->tm_year + 1900;
			sBizDT.nMonth = ptm->tm_mon + 1;
			sBizDT.nDay = ptm->tm_mday;
			sBizDT.nHour = ptm->tm_hour;
			sBizDT.nMinute = ptm->tm_min;
			sBizDT.nSecond = ptm->tm_sec;
			
			BizSysComplexDTSet(&sBizDT);
			
			usleep(20*1000);//csp modify 20140406
			
			IPC_SetTimeZone(timezone, ipcsyncflag, 1);
			
			if(flag)
			{
				usleep(20*1000);
				for(i = 0; i < GetVideoMainNum(); i++)
				{
					BizRecordResume(i);
				}
			}
			
			BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_DST_CONFIG);
		}
	}
	
	return 0;
}

static u8 ntp_param_changed = 0;

//QYJ
static void* keepsynchronizing(void* p)
{
	printf("$$$$$$$$$$$$$$$$$$keepsynchronizing id:%d\n",getpid());
	
	unsigned int count = 0;
	
	while(1)
	{
		int timesyncflag = GetNetworkSyscCheck();
		
		char ntpserveradress[32] = {0};
		GetNtpServer(ntpserveradress, 32);
		
		int ret = synctimefromntp(timesyncflag, ntpserveradress, 10);
		
		//sleep(1*60*60);
		//sleep(2*60*60);
		//sleep(24*60*60);
		
		while(++count < (12*60*24))
		{
			sleep(5);
			
			if(ntp_param_changed)
			{
				ntp_param_changed = 0;
				break;
			}
			
			if(ret < 0)
			{
				if((count%12) == 0)
				{
					break;
				}
			}
		}
		
		count = 0;
	}
	
	printf("stop keepsynchronizing...\n");
	
	return 0;
}

static void* updatetimenow(void* p)
{
	printf("$$$$$$$$$$$$$$$$$$updatetimenow id:%d\n",getpid());
	
	printf("haha,start updatetimenow...\n");
	
	while(1)
	{
		if((0 == pagebasic->nThisPageOpened) || (pagebasic->nTimeChanged))
		//if((0 == pagebasic->nThisPageOpened) || (pagebasic->nTimeChanged) || (pagebasic->nDateChanged))
		{
			//csp modify
			//break;
			sleep(1);
			continue;
		}
		
		if(0 == pagebasic->nDateChanged)
		{
			pagebasic->updatetime(0);
		}
		
		if(0 == pagebasic->nTimeChanged)
		{
			pagebasic->updatetime(1);
		}
		
		//if(pagebasic->nEdittingTime)
		//{
		//	usleep(500*1000);
		//	continue;
		//}
		
		usleep(300*1000);
	}
	
	printf("haha,stop updatetimenow...\n");
}

CPageBasicConfigFrameWork::CPageBasicConfigFrameWork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
, curID(0)
{
	//SetRect(CRect(0, 0, 700, 400/*370*/), FALSE);
	SetMargin(3, m_TitleHeight, 4, m_TitleHeight);
	
	m_lastVFormat = 0;
	m_lastOutput = 0;
	m_lastLanguage = 0;
	
	//pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/button_normal.bmp");
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//pBmp_button_push = VD_LoadBitmap(DATA_DIR"/button_push.bmp");
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//pBmp_button_select = VD_LoadBitmap(DATA_DIR"/button_select.bmp");
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//第一个页面的子页面控制按钮  录像设置，手动录像，录像布防
	pBmp_tab_normal = VD_LoadBitmap(DATA_DIR"/temp/tab.bmp");
	pBmp_tab_push = VD_LoadBitmap(DATA_DIR"/temp/tab_check.bmp");
	pBmp_tab_select = VD_LoadBitmap(DATA_DIR"/temp/tab_f.bmp");
	char* szSubPgName[BASICCFG_BTNNUM+2] = {
		"&CfgPtn.System",
		"&CfgPtn.DateTime",
		"&CfgPtn.Default",
		"&CfgPtn.apply",
		"&CfgPtn.Exit",
		"&CfgPtn.Prev",
		"&CfgPtn.Next",
	};
	
	int szSubPgNameLen[BASICCFG_BTNNUM+2] = {
		TEXT_WIDTH*3,//"&CfgPtn.System",
		TEXT_WIDTH*5,//"&CfgPtn.DateTime",
		TEXT_WIDTH*3,//"&CfgPtn.Default",
		TEXT_WIDTH*2,//"&CfgPtn.apply",
		TEXT_WIDTH*2,//"&CfgPtn.Exit",
		TEXT_WIDTH*3,
		TEXT_WIDTH*3,
	};
	
	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);
	
	CTableBox* pTabFrame = CreateTableBox(CRect(17, 40, m_Rect.Width()-17, 40+290),this,1,1);
	//pTabFrame->SetFrameColor(VD_RGB(56,108,148));
	pTabFrame->SetFrameColor(VD_RGB(8,66,189));
	
	CRect rtSub(25, 10, m_Rect.Width()-20, 10+pBmp_tab_normal->height);
	int i;
	for(i = 0; i<BASICCFG_SUBPAGES; i++)
	{
		int btWidth = szSubPgNameLen[i]+10;
		rtSub.right = rtSub.left + btWidth;
		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageBasicConfigFrameWork::OnClickSubPage, NULL, buttonNormalBmp, TRUE);
		pButton[i]->SetBitmap(pBmp_tab_normal, pBmp_tab_select, pBmp_tab_push, pBmp_tab_push);
		rtSub.left = rtSub.right;
	}
	
	CRect rtSub1(295, m_Rect.Height()-80, m_Rect.Width()-20, m_Rect.Height()-80+pBmp_button_normal->height);
	
	pInfoBar = CreateStatic(CRect(40, rtSub1.top, 385, rtSub1.top+22), 
							this, 
							"");
    //pInfoBar->SetBkColor(VD_RGB(67,77,87));
    //pInfoBar->SetTextAlign(VD_TA_CENTER);
    
	pBmp_infoicon = VD_LoadBitmap(DATA_DIR"/temp/icon_info.bmp");
	pIconInfo = CreateStatic(CRect(17, rtSub1.top+1, 37, rtSub1.top+21), 
							this, 
							"");
	pIconInfo->SetBitmap(pBmp_infoicon,NULL);
	pIconInfo->Show(FALSE);
	//pInfoBar->SetBkColor(VD_RGB(8,66,189));	
	
	rtSub1.left = rtSub1.right;
	for(i=BASICCFG_SUBPAGES; i<BASICCFG_BTNNUM; i++)
	{
		rtSub1.left -= szSubPgNameLen[i]+10;
	}
	rtSub1.left -= 12*2;
	rtSub1.left += 3;
	
	for(i=BASICCFG_SUBPAGES; i<BASICCFG_BTNNUM; i++)
	{
		int btWidth = szSubPgNameLen[i]+10;
		rtSub1.right = rtSub1.left + btWidth;
		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageBasicConfigFrameWork::OnClickSubPage, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
		rtSub1.left = rtSub1.right+12;
	}
	
    rtSub1.left -= (200*3);
    for(i = BASICCFG_BTNNUM; i < BASICCFG_BTNNUM + 2; i++)
    {
        //printf("***%s\n", __FUNCTION__);
        int btWidth = szSubPgNameLen[i];
		rtSub1.right = rtSub1.left + btWidth;
		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageBasicConfigFrameWork::OnClickSubPage, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
		rtSub1.left = rtSub1.right+12;
    }
    pButton[BASICCFG_BTNNUM]->Enable(FALSE);
	
	//CRect rtSub2(13, 35, m_Rect.Width()-18, 300);
	//pTable = CreateTableBox(&rtSub2,this, 1,1,0);
	
	InitPage0();
	InitPage1();
	
	CRect infoRt = CRect(0, 0, 480, 300/*370*/);
	m_pPageInfoBox = new CPageInfoBox(&infoRt/*&recordRt*/, "&CfgPtn.Info"/*"&titles.mainmenu"*/,"&CfgPtn.Reboot", icon_dvr, this);
	
	pagebasic = this;
	
	m_IsLoader = 0;
}

void RebootSystem()
{
	BizSysComplexExit(EM_BIZSYSEXIT_RESTART);
}

void CPageBasicConfigFrameWork::OnClickSubPage()
{
	//printf("OnClickSubPage##########################################\n");
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < BASICCFG_BTNNUM+2; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		switch(i)
		{
			case 0:
			case 1:
			{
				SwitchPage(i);
			}break;
			case 2://默认值
			{
				//printf("cur page:%d,default\n",curID);
				if(0 == curID)//系统子页面
				{
					char devName[32] = {0};
					int ret = GetDevNameDefault(devName,31);
					pEdit0[0]->SetText(devName);
					
					char devID[32] = {0};
					GetDevIdDefault(devID,31);
					pEdit0[1]->SetText(devID);
					
					int index = GetVideoFormatDefault();
					pComboBox0[0]->SetCurSel(index);
					
					int value = GetAuthCheckDefault();
					pCheckBox0[0]->SetValue(value);
					
					value = GetTimeDisplayCheckDefault();
					pCheckBox0[1]->SetValue(value);

					value = GetChnKbpsDisplayCheckDefault();
					pCheckBox0[3]->SetValue(value);
					
					value = GetShowGuideDefault();
					pCheckBox0[2]->SetValue(value);
					
					index = GetMaxUserDefault();
					pComboBox0[1]->SetCurSel(index);
					
					index = GetOutputDefault();
					pComboBox0[2]->SetCurSel(index);
					
					index = GetLanguageDefault();
					//printf("OnClickSubPage:GetLanguageDefault=%d\n",GetLanguageDefault());
					pComboBox0[3]->SetCurSel(index);
					
					//index = GetScreenSaverDefault();
					index = GetDWellSwitchTimeDefault();
					pComboBox0[4]->SetCurSel(index);

					index = GetDWellSwitchPictureDefault();
					pComboBox0[5]->SetCurSel(index);
					
					//csp modify
					index = GetLockTimeIndexDefault();
					pComboBox01[0]->SetCurSel(index);
					
					//csp modify
					SBizParaTarget bizTar;
					bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
					SBizSystemPara bizSysPara;
					BizGetDefaultPara(&bizTar, &bizSysPara);
					pEdit01[0]->SetText(bizSysPara.sn);
					
					if(ret == 0)
					{
						SetInfo("&CfgPtn.LoadDefaultSuccess");
					}
					else
					{
						SetInfo("&CfgPtn.LoadDefaultFail");
					}
				}
				else if(1 == curID)//日期和时间子页面
				{
					int ret = GetDateFormatDefault();
					pComboBox1[0]->SetCurSel(ret);
					
					int index = GetTimeFormatDefault();
					pComboBox1[1]->SetCurSel(index);
					
					index = GetTimeZoneDefault();
					pComboBox1[2]->SetCurSel(index);
					
					//QYJ
					int value = GetNetworkSyscCheckDefault();
					pCheckBox1[0]->SetValue(value);
					pButton1[0]->Enable(value);
					
					//QYJ
					char pServer[32] = {0};
					GetNtpServerDefault(pServer,32);
					pEdit1[0]->SetText(pServer);
					
					//csp modify 20131217
					value = GetTimeSyncToIPCDefault();
					pCheckBox1[1]->SetValue(value);
					
					SYSTEM_TIME date;
					//csp modify 20131213
					//GetSysDate(&date);
					GetSysDate_TZ(&date);
					pDateTime1[0]->SetDateTime(&date);
					
					SYSTEM_TIME time;
					//csp modify 20131213
					//GetSysTime(&time);
					GetSysTime_TZ(&time);
					pDateTime1[1]->SetDateTime(&time);
					
					if(ret >= 0)
					{
						SetInfo("&CfgPtn.LoadDefaultSuccess");
					}
					else
					{
						SetInfo("&CfgPtn.LoadDefaultFail");
					}		
				}				
			}break;
			case 3://应用
			{
				//printf("cur page:%d ,apply\n",curID);
				if(0 == curID) //系统子页面
				{
					//printf("language %d\n", pComboBox0[3]->GetCurSel());
					//pCdc->SetLanguage(pComboBox0[3]->GetCurSel());
					SSG_MSG_TYPE msg;
					memset(&msg, 0, sizeof(msg));
					msg.type = EM_DVR_PARAM_SAVE;
					msg.chn = 0;
					//系统参数保存
					strcpy(msg.note, GetParsedString("&CfgPtn.CfgSYSTEMSave"));
					upload_sg(&msg);
					
					u8 flag_DevID_valid = 0;
					char tmp[50];
					memset(tmp, 0, sizeof(tmp));
					pEdit0[1]->GetText(tmp,sizeof(tmp));
					if((strlen(tmp)<6) && strlen(tmp))
					{
						int i;
						for(i=0; i<strlen(tmp); i++)
						{
							if((tmp[i]<'0') || (tmp[i]>'9'))
							{
								break;
							}
						}
						
						if(i>=strlen(tmp))
						{
							sscanf(tmp, "%d", &i);
							if(i<=0xff)//255
							{
								flag_DevID_valid = 1;
							}
						}
					}
					if(!flag_DevID_valid)
					{
						MessageBox("&CfgPtn.InvalidDeviceId","&CfgPtn.WARNING",MB_OK|MB_ICONERROR);
						break;
					}
					
					int curVFormat = pComboBox0[0]->GetCurSel();
					int curOutput = pComboBox0[2]->GetCurSel();
					int curLanguage = pComboBox0[3]->GetCurSel();
					
					int nRestart = 0;
					
					if(curVFormat != m_lastVFormat
						|| curOutput != m_lastOutput
						|| curLanguage != m_lastLanguage)
					{
						//m_pPageInfoBox->SetOkCallBack(RebootSystem);
						
						UDM ret = MessageBox("&CfgPtn.RebootOrNot", "&CfgPtn.WARNING", MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2);
						if(GetRunStatue() == UDM_OK)
						{
							nRestart = 1;
						} 
						else if(GetRunStatue() == UDM_CANCEL)
						{
							//add by Lirl on Nov/08/2011
			                            UpdateData(UDM_OPEN);//点击取消按钮时刷新页面
			                            break;
			                        }
					}
					
                    #if 1
                    //add by Lirl on Nov/17/2011,制式改变时转换到对应制式的满帧
                    if((curVFormat!=m_lastVFormat) && (EM_BIZ_NTSC==(EMBIZVIDSTANDARD)GetVideoFormatReal()))
					{
                           SBizEncodePara sBizEncParaMain;
                           SBizEncodePara sBizEncParaSub;
                           for(int i=0; i<GetMaxChnNum(); i++)
                           {
                                BizGetVMainEncodePara(0,&sBizEncParaMain, i);
                                if(30 == sBizEncParaMain.nFrameRate 
                                    || 0 == sBizEncParaMain.nFrameRate)
                                {
                                    sBizEncParaMain.nFrameRate = 25;
                                    BizSetVMainEncodePara(&sBizEncParaMain, i);
                                }
								
                                BizGetVSubEncodePara(0,&sBizEncParaSub, i);
                                if(30 == sBizEncParaSub.nFrameRate 
                                    || 0 == sBizEncParaSub.nFrameRate)
                                {
                                    sBizEncParaSub.nFrameRate = 25;
                                    BizSetVSubEncodePara(&sBizEncParaSub, i);
                                }
                           }
					}
					
                    if((curVFormat!=m_lastVFormat) && (EM_BIZ_PAL==(EMBIZVIDSTANDARD)GetVideoFormatReal()))
					{
                           SBizEncodePara sBizEncParaMain;
                           SBizEncodePara sBizEncParaSub;
                           for(int i=0; i<GetMaxChnNum(); i++)
                           {
                                BizGetVMainEncodePara(0,&sBizEncParaMain, i);
                                if(25 == sBizEncParaMain.nFrameRate 
                                    || 0 == sBizEncParaMain.nFrameRate)
                                {
                                    sBizEncParaMain.nFrameRate = 30;
                                    BizSetVMainEncodePara(&sBizEncParaMain, i);
                                }

                                BizGetVSubEncodePara(0,&sBizEncParaSub, i);
                                if(25 == sBizEncParaSub.nFrameRate 
                                	|| 0 == sBizEncParaSub.nFrameRate)
                                {
                                    sBizEncParaSub.nFrameRate = 30;
                                    BizSetVSubEncodePara(&sBizEncParaSub, i);
                                }
                           }
					}
                    //end
                    #endif
					
					WriteLogs();
					
					u32 id1[2] = {
						GSR_CONFIG_BASIC_SYSTEM_DEVICENAME,
						GSR_CONFIG_BASIC_SYSTEM_DEVICEID
					};
					
					int i = 0;
                    int ret = 0;
					for(i=0; i<2; i++)
					{
						ret |= SaveScrStr2Cfg(pEdit0[i],
										id1[i], 
										EM_GSR_CTRL_EDIT, 
										0);
						//printf("ret1 = %d\n", ret);
					}
                    
					u32 id2[4] = {
						GSR_CONFIG_BASIC_SYSTEM_AUTHCHECK,
						GSR_CONFIG_BASIC_SYSTEM_TIMEDISPLAY,
						GSR_CONFIG_BASIC_SYSTEM_SHOWGUIDE,
						GSR_CONFIG_BASIC_SYSTEM_SHOWCHNKBPS,
					};
					
					for(i=0; i<4; i++)
					{
						ret |= SaveScrStr2Cfg(pCheckBox0[i],
									id2[i], 
									EM_GSR_CTRL_CHK, 
									0);
						//printf("ret2 = %d\n", ret);
					}

					u32 id3[5] = {
						 GSR_CONFIG_BASIC_SYSTEM_VIDEOFORMAT,
						 GSR_CONFIG_BASIC_SYSTEM_MAXNETUSERS,				
						 GSR_CONFIG_BASIC_SYSTEM_OUTPUT,			
						 GSR_CONFIG_BASIC_SYSTEM_LANGUAGE,				
						 GSR_CONFIG_BASIC_SYSTEM_SCRENNSAVER,
					};

RESAVE:
					for(i=0; i<5; i++)
					{
						if ((0 == nRestart) && ((GSR_CONFIG_BASIC_SYSTEM_VIDEOFORMAT == id3[i])
							|| (GSR_CONFIG_BASIC_SYSTEM_OUTPUT == id3[i])
							|| (GSR_CONFIG_BASIC_SYSTEM_LANGUAGE == id3[i])
							))
						{
							continue;
						}
						ret |= SaveScrStr2Cfg(pComboBox0[i],
								id3[i], 
								EM_GSR_CTRL_COMB, 
								0);
						//printf("ret3 = %d\n", ret);
					}
					
					//CDC* pCdc = GetDC();
					
					ret |= SetDWellSwitchTimeIndex(pComboBox0[4]->GetCurSel());

					//printf("yg pComboBox0[5]->GetCurSel(): %d\n", pComboBox0[5]->GetCurSel());
					ret |= SetDWellSwitchPictureIndex(pComboBox0[5]->GetCurSel());
					
					ret |= SetLockTimeIndex(pComboBox01[0]->GetCurSel());//csp modify
					
					#if 1//#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
					#else
					SBizParaTarget bizTar;
					bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
					SBizSystemPara bizSysPara;
					BizGetPara(&bizTar, &bizSysPara);
					memset(tmp, 0, sizeof(tmp));
					pEdit01[0]->GetText(tmp,sizeof(tmp));
					memset(bizSysPara.sn, 0, sizeof(bizSysPara.sn));
					strcpy(bizSysPara.sn, tmp);
					BizSetPara(&bizTar, &bizSysPara);
					//验证序列号
					if(BizVerifySN() == 0)
					{
						BizRecordSetWorkingEnable(1);
						
						CPage** page = GetPage();
				        if(page && page[EM_PAGE_DESKTOP])
				        {
				        	CPageDesktop *pDesktop = (CPageDesktop *)page[EM_PAGE_DESKTOP];
							if(pDesktop->pStaticVerifySNResult)
							{
								pDesktop->pStaticVerifySNResult->Show(FALSE);
							}
				        }
					}
					else
					{
						BizRecordSetWorkingEnable(0);
						
						CPage** page = GetPage();
				        if(page && page[EM_PAGE_DESKTOP])
				        {
				        	CPageDesktop *pDesktop = (CPageDesktop *)page[EM_PAGE_DESKTOP];
							if(pDesktop->pStaticVerifySNResult)
							{
								pDesktop->pStaticVerifySNResult->Show(TRUE);
							}
				        }
					}
					#endif
					
					if(ret == 0)
					{
						SetInfo("&CfgPtn.SaveConfigSuccess");
					}
					else
					{
						SetInfo("&CfgPtn.SaveConfigFail");
					}
					
					usleep(1000 * 1000 * 1);
					
					/*if(m_lastLanguage != curLanguage)
					{
						printf("lalalalalalalal***\n");
						SBizSystemPara sysPara;
						if(0 == BizGetSysInfo(&sysPara))
						{
							CLocales* pLocale = NULL;
							bizData_GetLocales(&pLocale);
							printf("lalalalalalalal***\n");
							if(pLocale)
							{
								printf("lalalalalal %d\n",sysPara.nLangId);						
								pLocale->SetLanguage(sysPara.nLangId);
							}
						}
					}*/
					
					m_lastVFormat = curVFormat;
					m_lastOutput = curOutput;
					m_lastLanguage = curLanguage;
					
					if(nRestart && (0 == ret))
					{
					    //printf("***nRestart = %d, ret = %d\n", nRestart, ret);
                        SetInfo("");
						SetSystemLockStatus(1);//cw_shutdown
						m_pPageInfoBox->Open();
						BizSysComplexExit(EM_BIZSYSEXIT_RESTART);
                        //usleep(1000 * 3000);
                        //m_pPageInfoBox->Close();
                        //Modify by Lirl on Jan/16/2012,set the font color to red
                        pInfoBar->SetTextColor(VD_RGB(255,0,0), 1);
                        SetInfo("&CfgPtn.SysRestartNow");
					}
					else if(nRestart && ret)
					{
					    //Modify by Lirl on Feb/28/2012
					    //防止参数选项没有完全存在就重启
					    //printf("***nRestart = %d, ret = %d\n", nRestart, ret);
					    static int count = 0;
					    if(count++ < 3)
					    {
                            goto RESAVE;
                        }
					}
				}
				else if(1 == curID) //日期和时间子页面
				{
					SSG_MSG_TYPE msg;
					memset(&msg, 0, sizeof(msg));
					msg.type = EM_DVR_PARAM_SAVE;
					msg.chn = 0;
					//日期和时间参数保存
					strcpy(msg.note, GetParsedString("&CfgPtn.DateTimeSave"));
					upload_sg(&msg);
					
					u32 id1[3] = {
						GSR_CONFIG_BASIC_DATETIME_DATEFORMAT,					
						GSR_CONFIG_BASIC_DATETIME_TIMEFORMAT, 				
						GSR_CONFIG_BASIC_DATETIME_TIMEZONE, 
					};
					
					char pServer[32] = {0};
					pEdit1[0]->GetText(pServer,32);
					
					int i = 0;
					int ret = 0;
					for(i=0; i<3; i++)
					{
						ret |= SaveScrStr2Cfg(pComboBox1[i],
                								id1[i], 
                								EM_GSR_CTRL_COMB, 
                								0);
					}
					
					//QYJ
					u32 id2[1] = {GSR_CONFIG_BASIC_DATETIME_NETSYN};
					for(i=0; i<1; i++)
					{
						printf("saving netsyn\n");
						ret |= SaveScrStr2Cfg(pCheckBox1[i],
									id2[i], 
									EM_GSR_CTRL_CHK, 
									0);
						//printf("ret = %d\n", ret);
					}
					
					//QYJ
					u32 id3[1] = {GSR_CONFIG_BASIC_DATETIME_NTPSERVER};
					for(i=0; i<1; i++)
					{
						ret |= SaveScrStr2Cfg(pEdit1[i],
										id3[i], 
										EM_GSR_CTRL_EDIT, 
										0);
						//printf("ret = %d\n", ret);
					}
					
					u32 id4[1] = {GSR_CONFIG_BASIC_DATETIME_SYNCTIMETOIPC};
					for(i=0; i<1; i++)
					{
						printf("saving synctimetoipc\n");
						ret |= SaveScrStr2Cfg(pCheckBox1[1+i],
									id4[i], 
									EM_GSR_CTRL_CHK, 
									0);
						//printf("ret = %d\n", ret);
					}
					
					//printf("######OnSave1######\n");
					//IPC 同步时间在此
					OnSave1();//yzw add
					
					ntp_param_changed = 1;
					
					if(ret == 0)
					{
						SetInfo("&CfgPtn.SaveConfigSuccess");
					}
					else
					{
						SetInfo("&CfgPtn.SaveConfigFail");
					}
				}
			}break;
			case 4://退出
			{
				//printf("exit\n");				
				this->Close();				
			}break;
            case 5:
            {
                //上一步
                ;
            } break;
            case 6:
            {
                //下一步
                CPage** page = GetPage();
                if(page && page[EM_PAGE_NETCFG])
                {
                    //先保存配置
                    //日期和时间
                    u32 id0[3] = {
						GSR_CONFIG_BASIC_DATETIME_DATEFORMAT,					
						GSR_CONFIG_BASIC_DATETIME_TIMEFORMAT, 				
						GSR_CONFIG_BASIC_DATETIME_TIMEZONE, 
					};
					
					//int i = 0;
					for(i=0; i<3; i++)
					{
						SaveScrStr2Cfg(pComboBox1[i],
        								id0[i], 
        								EM_GSR_CTRL_COMB, 
        								0);
					}
					
					//SaveScrStr2Cfg(pCheckBox1[1],
					SaveScrStr2Cfg(pCheckBox1[2],
										GSR_CONFIG_BASIC_SYSTEM_SHOWGUIDE, 
										EM_GSR_CTRL_CHK, 
										0);
					
					//QYJ
					char pServer[32] = {0};
					pEdit1[0]->GetText(pServer,32);
					
					//QYJ
					u32 id2[1] = {GSR_CONFIG_BASIC_DATETIME_NETSYN};
					for(i=0; i<1; i++)
					{
						printf("saving netsyn\n");
						SaveScrStr2Cfg(pCheckBox1[i],
								id2[i], 
								EM_GSR_CTRL_CHK, 
								0);
						//printf("ret = %d\n", ret);
					}
					
					//QYJ
					u32 id3[1] = {GSR_CONFIG_BASIC_DATETIME_NTPSERVER};
					for(i=0; i<1; i++)
					{
						SaveScrStr2Cfg(pEdit1[i],
										id3[i], 
										EM_GSR_CTRL_EDIT, 
										0);
						//printf("ret = %d\n", ret);
					}
					
					u32 id4[1] = {GSR_CONFIG_BASIC_DATETIME_SYNCTIMETOIPC};
					for(i=0; i<1; i++)
					{
						printf("saving synctimetoipc\n");
						SaveScrStr2Cfg(pCheckBox1[1+i],
									id4[i], 
									EM_GSR_CTRL_CHK, 
									0);
						//printf("ret = %d\n", ret);
					}
					
					//printf("######OnSave1######\n");
					
					OnSave1();
                    //
					
					ntp_param_changed = 1;
                    
                    page[EM_PAGE_NETCFG]->UpdateData(UDM_GUIDE);
                    if(page[EM_PAGE_NETCFG]->Open())
                    {
                        this->Close();
                    }
                    //page[EM_PAGE_NETCFG]->UpdateData(UDM_EMPTY);
                }
            }
			break;
			default:
			break;
		}
	}
	
	//csp modify
	if(bFind)
	{
		nDateChanged = 0;
		nTimeChanged = 0;
		nEdittingTime = 0;
	}
}

CPageBasicConfigFrameWork::~CPageBasicConfigFrameWork()
{
	
}
void CPageBasicConfigFrameWork::RecvNotifyClose()	//跃天解码板CMS remote CTRL
{
	if ( (GetFlag(IF_SHOWN)) && (curID == 1) )
	{
		this->Close();	
	}
}

VD_PCSTR CPageBasicConfigFrameWork::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

//csp modify
void CPageBasicConfigFrameWork::OnTrackMove0()
{
#if 1
	int pos = pScrollbar0->GetPos();
	//printf("OnTrackMove0:curID=%d,pos=%d,pg0_page=%d\n",curID,pos,pg0_page);
	if(curID != 0 || pg0_page == pos)
	{
		//printf("hehe1\n");
		return;
	}
	
	if(pg0_page!=1 && pos==1)
	{
		//printf("hehe2\n");
		ShowSubPage(curID, FALSE);
		pg0_page = pos;
		//printf("hehe2.1\n");
		ShowSubPage(curID, TRUE);
	}
	
	if(pg0_page!=0 && pos==0)
	{
		//printf("hehe3\n");
		ShowSubPage(curID, FALSE);
		pg0_page = pos;
		ShowSubPage(curID, TRUE);
	}
	
	//printf("hehe4\n");
#endif
}

void CPageBasicConfigFrameWork::InitPage0()
{
	char* statictext[10] = {
		"&CfgPtn.DeviceName",
		"&CfgPtn.DeviceID",
		"&CfgPtn.VideoFormat",
		"&CfgPtn.PasswordCheck",
		"&CfgPtn.ShowSystemTime",
		"&CfgPtn.MaxOnlineUsers",//5
		"&CfgPtn.VideoOutput",
		"&CfgPtn.Language",
		"&CfgPtn.DWellSwitchTime",
		"&CfgPtn.DWellSwitchPicture",
		//"&CfgPtn.ShowGuide",
		//"&CfgPtn.Screensaver",//
	};
	
	#if 1//csp modify
	pg0_page = 0;
	#endif
	
	CRect rtSubPage(m_Rect.left+27, m_Rect.top+50, 
		            m_Rect.Width()-27,m_Rect.top+50+270);
	
	#if 1//csp modify
	pTable0 = CreateTableBox(&rtSubPage,this,3,9,0);
	pTable0->SetColWidth(0,280);
	pTable0->SetColWidth(1,286);
	#else
	pTable0 = CreateTableBox(&rtSubPage,this,2,9,0);
	pTable0->SetColWidth(0,294);
	#endif
	items[0].push_back(pTable0);
	
	//csp modify
	pScrollbar0 = CreateScrollBar(CRect(m_Rect.Width()-30-20+2,53,m_Rect.Width()-30+2,53+264),
                                    this,
                                    scrollbarY,
								    0,
								    10,
								    1,
								    (CTRLPROC)&CPageBasicConfigFrameWork::OnTrackMove0);
	items[0].push_back(pScrollbar0);
	
	#if 1//csp modify
	items[2].push_back(pTable0);
	items[2].push_back(pScrollbar0);
	#endif
	
	CRect tmpRt;
	int i = 0;
	
	#if 1//csp modify
	//yaogang modify 20140918
	char* statictext01[4] = /*2*/
	{
		"&CfgPtn.KeyLock",
		"&CfgPtn.SN",
		"&CfgPtn.ShowChnKbps", //显示实时码流
		"&CfgPtn.ShowGuide",//显示向导
	};
	for(i=0; i<4; i++)
	{
		if (i < 2)
		{
			pTable0->GetTableRect(0, i, &tmpRt);
			pStatic01[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top+3, 
											rtSubPage.left+tmpRt.right-3,rtSubPage.top+tmpRt.bottom), 
											this, statictext01[i]);
		}
		else
		{
			pTable0->GetTableRect(0, i-1, &tmpRt);
			pStatic01[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top+3, 
											rtSubPage.left+tmpRt.right-3,rtSubPage.top+tmpRt.bottom), 
											this, statictext01[i]);
		}
		
		pStatic01[i]->Show(FALSE);
		
		#if 1//#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
		if(i != 1)
		{
			items[2].push_back(pStatic01[i]);
		}
		#else
		items[2].push_back(pStatic01[i]);
		#endif
		
		if(i == 0)
		{
			pTable0->GetTableRect(1,i,&tmpRt);
			pComboBox01[i] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
											rtSubPage.left+tmpRt.right-15,rtSubPage.top+tmpRt.bottom-1), 
											this, NULL, NULL, NULL, 0);
			pComboBox01[i]->SetBkColor(VD_RGB(67,77,87));
			
			SValue LockTimeList[10];
			int nLockTime = 0;
			GetLockTimeList(LockTimeList, &nLockTime, 10);
			//printf("InitPage0 nLockTime = %d\n",nLockTime);
			for(int k=0; k<nLockTime; k++)
			{
				pComboBox01[i]->AddString(LockTimeList[k].strDisplay);
				//printf("InitPage0 AddString : %s\n",LockTimeList[k].strDisplay);
			}
			
			pComboBox01[i]->SetCurSel(0);
			pComboBox01[i]->Show(FALSE);
			
			items[2].push_back(pComboBox01[i]);
			
			
			
			pTable0->GetTableRect(1,1,&tmpRt);
			pEdit01[i] = CreateEdit(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
									rtSubPage.left+tmpRt.right-15,rtSubPage.top+tmpRt.bottom-1), 
									this,20,0,
									(CTRLPROC)&CPageBasicConfigFrameWork::OnEditChange0);
			pEdit01[i]->SetBkColor(VD_RGB(67,77,87));
			
			pEdit01[i]->Show(FALSE);
			#if 1//#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)//csp modify
			#else
			items[2].push_back(pEdit01[i]);
			#endif
		}
		if (i == 2)  //显示实时码流
		{
			pTable0->GetTableRect(1,1,&tmpRt);//上移一行
			pCheckBox0[3] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+4, 
									    rtSubPage.left+tmpRt.left+22,rtSubPage.top+tmpRt.top+24), 
									this);
			pCheckBox0[3]->Show(FALSE);
			items[2].push_back(pCheckBox0[3]);
		}
		if (i == 3)
		{
			//显示向导
			pTable0->GetTableRect(1, 2, &tmpRt);
			        
			pCheckBox0[2] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+4, 
											   rtSubPage.left+tmpRt.left+22,rtSubPage.top+tmpRt.top+24), 
											this);
			pCheckBox0[2]->Show(FALSE);
			items[2].push_back(pCheckBox0[2]);
		}
	}
	#endif
	
	for(i=0; i<10; i++)
	{
		pTable0->GetTableRect(0,i,&tmpRt);
		if(i > 5)//yzw add this line
		{
			pTable0->GetTableRect(0, i-1, &tmpRt);//yzw add this line
		}
		pStatic0[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top+3, 
										rtSubPage.left+tmpRt.right-3,rtSubPage.top+tmpRt.bottom), 
										this, statictext[i]);
		if((i<10) && (i!=5))//yzw add this line
			items[0].push_back(pStatic0[i]);
	}
	
	for(i=0; i < 2; i++)
	{
		pTable0->GetTableRect(1,i,&tmpRt);
		
		pEdit0[i] = CreateEdit(CRect(rtSubPage.left+tmpRt.left+1, rtSubPage.top+tmpRt.top+1, 
									rtSubPage.left+tmpRt.right-15,rtSubPage.top+tmpRt.bottom-1), 
									this,(1==i)?3:20,(1==i)?edit_KI_NUMERIC:0,
									(CTRLPROC)&CPageBasicConfigFrameWork::OnEditChange0);
		pEdit0[i]->SetBkColor(VD_RGB(67,77,87));
		items[0].push_back(pEdit0[i]);
	}
	
	SValue videoFormatList[10];
	int nVideoFormatReal;
	GetVideoFormatList(videoFormatList, &nVideoFormatReal, 10);

	pTable0->GetTableRect(1,2,&tmpRt);
	//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
	pComboBox0[0] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
										rtSubPage.left+tmpRt.right-15,rtSubPage.top+tmpRt.bottom-1), 
										this, NULL, NULL, NULL, 0);
	
	for(int k=0; k<nVideoFormatReal; k++)
	{
		pComboBox0[0]->AddString(videoFormatList[k].strDisplay);
	}
	
	pComboBox0[0]->SetBkColor(VD_RGB(67,77,87));
	items[0].push_back(pComboBox0[0]);	
	
	for(i=0; i < 2; i++)
	{
	        //if (2 != i)
	        {
			pTable0->GetTableRect(1,i+3,&tmpRt);
	        }
	        //else
	        //{
	        //	pTable0->GetTableRect(1,i+6,&tmpRt);
	        //}
		pCheckBox0[i] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+4, 
									    rtSubPage.left+tmpRt.left+22,rtSubPage.top+tmpRt.top+24), 
									this);
		
		items[0].push_back(pCheckBox0[i]);
	}
	
	SValue maxUserList[10];
	int nMaxUserReal;
	GetMaxUserList(maxUserList, &nMaxUserReal, 10);
	
	SValue outputList[10];
	int nOutputReal;
	GetOutputList(outputList, &nOutputReal, 10);
	
	//csp modify 20121224//大于20种语言怎么办???
	SValue languageList[20];
	int nLanguageReal = 0;
	GetLanguageList(languageList, &nLanguageReal, 20);
	
	SValue screenSaverList[10];
	int nScreenSaverReal;
	GetScreenSaverList(screenSaverList, &nScreenSaverReal, 10);
	
    SValue dWellTimeList[10];
    int nDWellTime;
    GetDWellSwitchTimeList(dWellTimeList, &nDWellTime, 10);
	/*
	printf("yg nDWellTime: %d\n", nDWellTime);
	for(i=0; i < nDWellTime; i++)
	{
		printf("yg dWellTimeList[%d]: %d\n", i, dWellTimeList[i].nValue);
		printf("yg dWellTimeList[%d]: %s\n", i, dWellTimeList[i].strDisplay);
	}
	*/
	SValue dWellSwitchPicture[10];//轮巡画面选择: 单画面，四画面，九画面
	int nPicturesReal;
	GetDWellSwitchPictureList(dWellSwitchPicture, &nPicturesReal, 10);
	printf("yg nPicturesReal: %d\n", nPicturesReal);
	for(i=0; i < nPicturesReal; i++)
	{
		BizConfigGetParaListValue(
					EM_BIZ_CFG_PARALIST_SWITCHPICTURE, 
					i, 
					&dWellSwitchPicture[i].nValue);
		printf("yg DWellSwitchPicture[%d]: %d\n", i, dWellSwitchPicture[i].nValue);
		printf("yg DWellSwitchPicture[%d]: %s\n", i, dWellSwitchPicture[i].strDisplay);
	}
	
	for(i=0; i<5; i++)
	{
		pTable0->GetTableRect(1,i+5,&tmpRt);
		if (i)//yzw add this line
		{
			pTable0->GetTableRect(1,i+4,&tmpRt);//yzw add this line
		}
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pComboBox0[i+1] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
											rtSubPage.left+tmpRt.right-15,rtSubPage.top+tmpRt.bottom-1), 
											this, NULL, NULL, NULL, 0);
		
		pComboBox0[i+1]->SetBkColor(VD_RGB(67,77,87));
		
		switch(i+1)
		{
			case 1:
			{
				for(int k=0; k<nMaxUserReal; k++)
				{
					pComboBox0[i+1]->AddString(maxUserList[k].strDisplay);
				}
			}break;
			case 2:
			{
				for(int k=0; k<nOutputReal; k++)
				{
					pComboBox0[i+1]->AddString(outputList[k].strDisplay);
					pComboBox0[2]->isListExist = 1;
				}
			}break;
			case 3:
			{
				for(int k=0; k<nLanguageReal; k++)
				{
					pComboBox0[i+1]->AddString(languageList[k].strDisplay);
					pComboBox0[3]->isListExist = 1;
				}
			}break;
			case 4:
			{
				for(int k=0; k<nDWellTime; k++)
				{
					pComboBox0[i+1]->AddString(dWellTimeList[k].strDisplay);
				}
			}break;
			case 5:
			{
				for(int k=0; k<nPicturesReal; k++)
				{
					pComboBox0[i+1]->AddString(dWellSwitchPicture[k].strDisplay);
				}
			}break;
			default:
			{}break;
		}
		
	    	if (i) //yzw add this line
	    	{
	    		items[0].push_back(pComboBox0[i+1]);
	    	}
	}
	
	//轮训时间
	//pComboBox0[4]->Show(FALSE);//yzw add this line//csp modify
	//pStatic0[8]->Show(FALSE);//yzw add this line//csp modify
	
	pComboBox0[1]->Show(FALSE);//yzw add this line
	pStatic0[5]->Show(FALSE);//yzw add this line
	
	ShowSubPage(0,TRUE);//zlbtmp
	
	pButton[0]->Enable(FALSE);
}

void CPageBasicConfigFrameWork::InitPage1()
{
	char* statictext[10] = {
		"&CfgPtn.DateFormat",
		"&CfgPtn.TimeFormat",
		"&CfgPtn.TimeZone",
		"&CfgPtn.SyncTimeWithNTPServer",
		"&CfgPtn.NTPServer",
		"",
		"&CfgPtn.SystemDate",
		"&CfgPtn.SystemTime",
		"&CfgPtn.SyncTimeToIPC",
		"&CfgPtn.ShowGuide",
	};
	
	CRect rtSubPage(m_Rect.left+27, m_Rect.top+50, 
		            m_Rect.Width()-27,m_Rect.top+50+270);
	
	//csp modify 20131217
	//pTable1 = CreateTableBox(&rtSubPage,this,2,9,0);
	pTable1 = CreateTableBox(&rtSubPage,this,2,10,0);
	pTable1->SetColWidth(0,294);
	items[1].push_back(pTable1);
	
	CRect tmpRt;
	int i = 0;
	for(i=0; i < 10; i++)
	{
		#ifdef HIDE_NTP
		if(i <= 5)
		{
			pTable1->GetTableRect(0,i,&tmpRt);
		}
		else
		{
			pTable1->GetTableRect(0,i-4,&tmpRt);
		}
		#else
		pTable1->GetTableRect(0,i,&tmpRt);
		#endif
		
		pStatic1[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top+3,
									rtSubPage.left+tmpRt.right-3,rtSubPage.top+tmpRt.bottom), 
									this, statictext[i]);
		
		#ifdef HIDE_NTP
		if(i >= 2 && i <= 5)
		{
			pStatic1[i]->Show(FALSE);
		}
		else
		{
			items[1].push_back(pStatic1[i]);
		}
		#else
		items[1].push_back(pStatic1[i]);
		#endif
	}
	
	SValue dateFormatList[10];
	int nDateFormatReal;
	GetDateFormatList(dateFormatList, &nDateFormatReal, 10);
	
	SValue timeFormatList[10];
	int nTimeFormatReal;
	GetTimeFormatList(timeFormatList, &nTimeFormatReal, 10);
	
	SValue timeZoneList[50];
	int nTimeZoneReal;
	GetTimeZoneList(timeZoneList, &nTimeZoneReal, 50);
	
	for(i=0; i<3; i++)
	{
		pTable1->GetTableRect(1,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		if(1 == i)
		{
			pComboBox1[i] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
											rtSubPage.left+tmpRt.right-15,rtSubPage.top+tmpRt.bottom-1), 
											this, NULL, NULL,(CTRLPROC)&CPageBasicConfigFrameWork::OnComb, 0);
		}
        else
        {
            pComboBox1[i] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
											rtSubPage.left+tmpRt.right-15,rtSubPage.top+tmpRt.bottom-1), 
											this, NULL, NULL, NULL, 0);
        }
		pComboBox1[i]->SetBkColor(VD_RGB(67,77,87));
		
		/*
		if(i == 2)
		{
			pComboBox1[i]->Enable(FALSE);
		}
		*/
		
		#ifdef HIDE_NTP
		u8 push_flag = 1;
		#endif
		
		switch(i)
		{
			case 0:
			{
				for(int k=0; k<nDateFormatReal; k++)
				{
					pComboBox1[i]->AddString(dateFormatList[k].strDisplay);
				}
			}break;
			case 1:
			{
				for(int k=0; k<nTimeFormatReal; k++)
				{
					pComboBox1[i]->AddString(timeFormatList[k].strDisplay);
				}
			}break;
			case 2:
			{
				for(int k=0; k<nTimeZoneReal; k++)
				{
					pComboBox1[i]->AddString(timeZoneList[k].strDisplay);
				}
				
				//QYJ
				#ifdef HIDE_NTP
				pComboBox1[i]->Enable(FALSE);
				#else
				//pComboBox1[i]->Enable(FALSE);
				#endif
				
				#ifdef HIDE_NTP
				pComboBox1[i]->Show(FALSE);
				push_flag = 0;
				#endif
			}break;
			default:
			{}break;
		}
		
		#ifdef HIDE_NTP
		if(push_flag)
		{
			items[1].push_back(pComboBox1[i]);
		}
		#else
		items[1].push_back(pComboBox1[i]);
		#endif
	}
    
	pTable1->GetTableRect(1,4,&tmpRt);
	int vLen = (tmpRt.right-tmpRt.left-4)/TEXT_WIDTH*2;
	pEdit1[0] = CreateEdit(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
								    rtSubPage.left+tmpRt.right-15,rtSubPage.top+tmpRt.bottom-1), 
								this,vLen,0,(CTRLPROC)&CPageBasicConfigFrameWork::OnEditChange1);
	pEdit1[0]->SetBkColor(VD_RGB(67,77,87));
	//QYJ
	#ifdef HIDE_NTP
	pEdit1[0]->Enable(FALSE);
	#else
	//pEdit1[0]->Enable(FALSE);
	#endif
	#ifdef HIDE_NTP
	pEdit1[0]->Show(FALSE);
	#else
	items[1].push_back(pEdit1[0]);
	#endif
	
	int btWidth = strlen("Synchronize")*TEXT_WIDTH/2+20;
	
	pTable1->GetTableRect(1,5,&tmpRt);
	pButton1[0] = CreateButton(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+2, 
								    rtSubPage.left+tmpRt.left+btWidth,rtSubPage.top+tmpRt.top+27), 
								this, "&CfgPtn.UpdateNow", 
								(CTRLPROC)&CPageBasicConfigFrameWork::OnSynchronize1, NULL, buttonNormalBmp);
	pButton1[0]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
	//QYJ
	#ifdef HIDE_NTP
	pButton1[0]->Enable(FALSE);
	#else
	//pButton1[0]->Enable(FALSE);
	#endif
	#ifdef HIDE_NTP
	pButton1[0]->Show(FALSE);
	#else
	items[1].push_back(pButton1[0]);
	#endif
	
	#ifdef HIDE_NTP
	pTable1->GetTableRect(1,6-4,&tmpRt);
	#else
	pTable1->GetTableRect(1,6,&tmpRt);
	#endif
	pDateTime1[0] = CreateDateTimeCtrl(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+3, 
								    rtSubPage.left+tmpRt.left+126,rtSubPage.top+tmpRt.top+27),
								this,(CTRLPROC)&CPageBasicConfigFrameWork::OnDateTimeChange0,DTS_DATE);
	pDateTime1[0]->SetBkColor(VD_RGB(67,77,87));
	//pDateTime1[0]->Show(FALSE);
	items[1].push_back(pDateTime1[0]);
	
	#ifdef HIDE_NTP
	pTable1->GetTableRect(1,7-4,&tmpRt);
	#else
	pTable1->GetTableRect(1,7,&tmpRt);
	#endif
	pDateTime1[1] = CreateDateTimeCtrl(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+3, 
								    rtSubPage.left+tmpRt.left+126,rtSubPage.top+tmpRt.top+27),
								this,(CTRLPROC)&CPageBasicConfigFrameWork::OnDateTimeChange1,DTS_TIME);
	pDateTime1[1]->SetBkColor(VD_RGB(67,77,87));    
	//pDateTime1[1]->Show(FALSE);
	items[1].push_back(pDateTime1[1]);
	
	
	pTable1->GetTableRect(1,8,&tmpRt);
	pButton1[1] = CreateButton(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
								    rtSubPage.left+tmpRt.left+btWidth,rtSubPage.top+tmpRt.top+25), 
								this, "&CfgPtn.SaveNow",
								(CTRLPROC)&CPageBasicConfigFrameWork::OnSave1, NULL, buttonNormalBmp);
	pButton1[1]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	//items[1].push_back(pButton1[1]);
	pButton1[1]->Show(FALSE);
	
	//csp modify 20131217
	//for(i = 0; i < 2; i++)
	for(i = 0; i < 3; i++)
    {
        if(i)
        {
        	#ifdef HIDE_NTP
			pTable1->GetTableRect(1,8-4,&tmpRt);
			#else
    	    pTable1->GetTableRect(1,8+i-1,&tmpRt);
			#endif
			
			pCheckBox1[i] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+4, 
										rtSubPage.left+tmpRt.left+22,rtSubPage.top+tmpRt.top+24), 
										this);
        }
        else
        {
            pTable1->GetTableRect(1,3,&tmpRt);
			
			pCheckBox1[i] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+4, 
										rtSubPage.left+tmpRt.left+22,rtSubPage.top+tmpRt.top+24), 
										this, styleEditable, (CTRLPROC)&CPageBasicConfigFrameWork::OnCheckBoxNTPEnable);
        }
    	
		#ifdef HIDE_NTP
		if(i == 0)
		{
			pCheckBox1[i]->Show(FALSE);
		}
		else
		{
			items[1].push_back(pCheckBox1[i]);
		}
		#else
    	items[1].push_back(pCheckBox1[i]);
		#endif
    }
	//QYJ
	#ifdef HIDE_NTP
	pCheckBox1[0]->Enable(FALSE);
	#else
	//pCheckBox1[0]->Enable(FALSE);
	#endif
	
	ShowSubPage(1,FALSE);
	
	nThisPageOpened = 0;
	nDateChanged = 0;
	nTimeChanged = 0;
	nEdittingTime = 0;
	
	#ifndef HIDE_NTP
	pthread_t pid1 = 0;
	pthread_create(&pid1, NULL, keepsynchronizing, NULL);
	#endif
}

void CPageBasicConfigFrameWork::OnDateTimeChange0()
{
	nDateChanged = 1;
}

void CPageBasicConfigFrameWork::OnDateTimeChange1()
{
	//printf("%s\n", __func__);
	nTimeChanged = 1;
}

void CPageBasicConfigFrameWork::OnEditChange0()
{
	//printf("OnEditChange0\n");
}

void CPageBasicConfigFrameWork::OnEditChange1()
{
	//printf("OnEditChange1\n");
}

void CPageBasicConfigFrameWork::OnSynchronize1()
{
	//printf("unsupported now!\n");
	char pServer[32] = {0};
	pEdit1[0]->GetText(pServer,32);
	int ret = synctimefromntp(1, pServer, 5);
	if(ret == 0)
	{
		ntp_param_changed = 0;
		SetInfo("&CfgPtn.Success");
	}
	else
	{
		SetInfo("&CfgPtn.Fail");
	}
}

void CPageBasicConfigFrameWork::OnComb()
{
    pDateTime1[1]->SetTimeFormat(0, 0, pComboBox1[1]->GetCurSel());
}

void CPageBasicConfigFrameWork::OnCheckBoxNTPEnable()
{
	int value = pCheckBox1[0]->GetValue();
	pButton1[0]->Enable(value);
}

void CPageBasicConfigFrameWork::OnSave1()
{
	SBizDateTime sBizDT;
	memset(&sBizDT, 0, sizeof(sBizDT));
	
	int sel = pComboBox1[0]->GetCurSel();
	if(0 == sel)//(0==strcmp(pComboBox1[0]->GetString(sel),"yy.MM.dd"))
	{
		sBizDT.emDateTimeFormat = EM_BIZDATETIME_YYYYMMDDHHNNSS;
	}
	else if(1 == sel)
	{
        sBizDT.emDateTimeFormat = EM_BIZDATETIME_MMDDYYYYHHNNSS;
    }
    else if(2 == sel)
    {
        sBizDT.emDateTimeFormat = EM_BIZDATETIME_DDMMYYYYHHNNSS;
    }
    /*
	else
	{}
	*/
	
    sel = pComboBox1[1]->GetCurSel();
	if(0 == sel)
	{
		sBizDT.emTimeFormat = EM_BIZTIME_24HOUR;
	}
	else if(1 == sel)
	{
        sBizDT.emTimeFormat = EM_BIZTIME_12HOUR;
    }
    /*
	else
	{}
	*/
	
	u8 i = 0;
	u8 flag = 0;
	
	u8 syncing = 1;
	
	if((nDateChanged) || (nTimeChanged))
	{
		//syncing = 0;//csp modify 20140406
		
		SYSTEM_TIME date, time;
		pDateTime1[0]->GetDateTime(&date);
		pDateTime1[1]->GetDateTime(&time);
		
		sBizDT.nYear = date.year;
		sBizDT.nMonth = date.month;
		sBizDT.nDay = date.day;
		sBizDT.nHour = time.hour;
		sBizDT.nMinute = time.minute;
		sBizDT.nSecond = time.second;
		printf("%04d-%02d-%02d %02d:%02d:%02d\n",
			sBizDT.nYear,sBizDT.nMonth,sBizDT.nDay,
			sBizDT.nHour,sBizDT.nMinute,sBizDT.nSecond);
		
		struct tm tm_time;
		tm_time.tm_year = sBizDT.nYear - 1900;
		tm_time.tm_mon = sBizDT.nMonth - 1;
		tm_time.tm_mday = sBizDT.nDay;
		tm_time.tm_hour = sBizDT.nHour;
		tm_time.tm_min = sBizDT.nMinute;
		tm_time.tm_sec = sBizDT.nSecond;
		time_t t = mktime(&tm_time);
		
		int nTimeZone = pComboBox1[2]->GetCurSel();
		t -= GetTimeZoneOffset(nTimeZone);
		
		/*
		printf("BizSysComplexDTSet %d %d %d %d %d %d\n",
			sBizDT.nYear,// = date.year;
			sBizDT.nMonth,// = date.month;
			sBizDT.nDay,// = date.day;
			sBizDT.nHour,// = date.hour;
			sBizDT.nMinute,// = date.minute;
			sBizDT.nSecond// = date.second;		
		);
		*/
		
		for(i = 0; i < GetVideoMainNum(); i++)
		{
			if(GetCurRecStatus(i))
			{
				break;
			}
		}
		
		if((i < GetVideoMainNum()) && ((nDateChanged) || (nTimeChanged)))
		{
			UDM ret = MessageBox("&CfgPtn.ModifyTimeRecording", "&CfgPtn.WARNING" ,
			                        MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2);
			
			if(UDM_OK != GetRunStatue())
			{
				return;
			}
			
			BizRecordStopAll();
			usleep(2000*1000); //多睡会儿吧，否则结束时间戳容易出问题
			flag = 1;
			
			t += 2;
		}
		
		gmtime_r(&t,&tm_time);
		
		sBizDT.nYear = tm_time.tm_year + 1900;
		sBizDT.nMonth = tm_time.tm_mon + 1;
		sBizDT.nDay = tm_time.tm_mday;
		sBizDT.nHour = tm_time.tm_hour;
		sBizDT.nMinute = tm_time.tm_min;
		sBizDT.nSecond = tm_time.tm_sec;
	}
	
	//csp modify 20131213
	int nTimeZone = pComboBox1[2]->GetCurSel();
	BizSysComplexSetTimeZone(nTimeZone);
	
	BizSysComplexDTSet(&sBizDT);
	
	//csp modify 20131213
	ModAlarmSetTimeZoneOffset(GetTimeZoneOffset(nTimeZone));
	ModRecordSetTimeZoneOffset(GetTimeZoneOffset(nTimeZone));
	
	usleep(20*1000);//csp modify 20140406
	
	//csp modify 20131213
	int syncflag = pCheckBox1[1]->GetValue();
	IPC_SetTimeZone(nTimeZone, syncflag, syncing);
	
	if(flag)
	{
		usleep(20*1000);
		for(i = 0; i < GetVideoMainNum(); i++)
		{
			BizRecordResume(i);
		}
	}
	
	BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_DST_CONFIG);
	
	//SetInfo("&CfgPtn.ChangeSysTime");
	//printf("OnSave1\n");
}

void CPageBasicConfigFrameWork::SwitchPage(int subID)
{	
	if(subID < 0 || subID >= BASICCFG_SUBPAGES)
	{
		return;
	}
	
	if(subID == curID)
	{
		return;
	}
	
	//printf("###curID = %d\n",curID);
	//printf("###subID = %d\n",subID);
	
	//隐藏当前子页面，显示新的子页面
	ShowSubPage(curID, FALSE);
	pButton[curID]->Enable(TRUE);
	curID = subID;
	pButton[curID]->Enable(FALSE);
	ShowSubPage(curID, TRUE);
}

void CPageBasicConfigFrameWork::ShowSubPage( int subID, BOOL bShow )
{
	if(subID<0 || subID>=BASICCFG_SUBPAGES)
	{
		return;
	}
	
	//csp modify
	if(subID == 0 && pg0_page == 1)
	{
		subID = 2;
	}
	
	int count = items[subID].size();
	
	//Modify by Lirl on Mar/05/2012
	//"时间和日期"页面,count-1为是了不显示最后一个checkbox控件,向导用
	#if 1//csp modify
	((subID == 1) && !m_IsLoader)? (count -= 1): count;
	#else
	(subID && !m_IsLoader)? (count -= 1): count;
	#endif
	
	//printf("ShowSubPage count=%d\n",count);
	
	for(int i=0; i<count; i++)
	{
		if(items[subID][i])
		{
			items[subID][i]->Show(bShow);
		}
	}
	
	//要显式的置FALSE,否则鼠标放上控件上还会再次显示出来
	#if 1//csp modify
	if(subID == 1 && !m_IsLoader)
	{
		items[subID][count]->Show(FALSE);
	}
	#else
	if(subID && !m_IsLoader)
	{
		items[subID][count]->Show(FALSE);
	}
	#endif
    
	//printf("ShowSubPage 2222\n");
}

void CPageBasicConfigFrameWork::updatetime(int flag)
{
	if(0 == flag)
	{
		SYSTEM_TIME date;
		//csp modify 20131213
		//GetSysDate(&date);
		GetSysDate_TZ(&date);
		//printf("year : %d,month : %d,day : %d\n",date.year,date.month,date.wday);
		pDateTime1[0]->SetDateTime(&date);
	}
	else
	{
		SYSTEM_TIME time;
		//csp modify 20131213
		//GetSysTime(&time);
		GetSysTime_TZ(&time);
		//printf("%s\n", __func__);
		//printf("hour : %d,min : %d,sec : %d\n",date.hour,date.minute,date.second);
		pDateTime1[1]->SetDateTime(&time);
	}
}

VD_BOOL CPageBasicConfigFrameWork::UpdateData( UDM mode )
{
	if(UDM_OPEN == mode)
	{
		char devName[32] = {0};
		GetDevName(devName,31);
		pEdit0[0]->SetText(devName);
		
		char devID[32] = {0};
		GetDevId(devID,31);
		pEdit0[1]->SetText(devID);
		
		int index = GetVideoFormat();
		pComboBox0[0]->SetCurSel(index);
		
		int value = GetAuthCheck();
		pCheckBox0[0]->SetValue(value);
		
		value = GetTimeDisplayCheck();
		pCheckBox0[1]->SetValue(value);

		value = GetChnKbpsDisplayCheck();
		pCheckBox0[3]->SetValue(value);
		
        	value = GetShowGuide();
		pCheckBox0[2]->SetValue(value);
		
		index = GetMaxUser();
		pComboBox0[1]->SetCurSel(index);
		
		index = GetOutput();
		pComboBox0[2]->SetCurSel(index);
		
		index = GetLanguageIndex();
		//printf("UpdateData:GetLanguageIndex=%d\n",index);
		pComboBox0[3]->SetCurSel(index);
		
		//index = GetScreenSaver();
		index = GetDWellSwitchTimeIndex();
		pComboBox0[4]->SetCurSel(index);

		index = GetDWellSwitchPictureIndex();
		pComboBox0[5]->SetCurSel(index);
		
		//csp modify
		index = GetLockTimeIndex();
		pComboBox01[0]->SetCurSel(index);
		
		//csp modify
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
		SBizSystemPara bizSysPara;
		BizGetPara(&bizTar, &bizSysPara);
		pEdit01[0]->SetText(bizSysPara.sn);
		
		index = GetDateFormat();
		pComboBox1[0]->SetCurSel(index);
		
		index = GetTimeFormat();
		pComboBox1[1]->SetCurSel(index);
		
		index = GetTimeZone();
		pComboBox1[2]->SetCurSel(index);
		
		value = GetNetworkSyscCheck();
		pCheckBox1[0]->SetValue(value);
		pButton1[0]->Enable(value);
		
		char pServer[32] = {0};
		GetNtpServer(pServer,32);
		pEdit1[0]->SetText(pServer);
		
		//csp modify 20131217
		value = GetTimeSyncToIPC();
		pCheckBox1[1]->SetValue(value);
		
		SYSTEM_TIME date;
		//csp modify 20131213
		//GetSysDate(&date);
		GetSysDate_TZ(&date);
		//printf("year : %d,month : %d,day : %d\n",date.year,date.month,date.wday);
		pDateTime1[0]->SetDateTime(&date);
		
		SYSTEM_TIME time;
		//csp modify 20131213
		//GetSysTime(&time);
		GetSysTime_TZ(&time);
		//printf("hour : %d,min : %d,sec : %d\n",date.hour,date.minute,date.second);
		pDateTime1[1]->SetDateTime(&time);
        OnComb();
		
		pButton[0]->SetText("&CfgPtn.System");
		
		m_lastVFormat = pComboBox0[0]->GetCurSel();
		m_lastOutput = pComboBox0[2]->GetCurSel();
		m_lastLanguage = pComboBox0[3]->GetCurSel();
		
        LoadButtomCtl();
	}
	else if(UDM_OPENED == mode)
	{
		if(0 == nThisPageOpened)
		{
			nThisPageOpened = 1;
			
			//csp modify
			static unsigned char create = 0;
			if(!create)
			{
				pthread_t pid = 0;
				pthread_create(&pid, NULL, updatetimenow, NULL);
				
				create = 1;
			}
		}
		
		//csp modify
		nDateChanged = 0;
		nTimeChanged = 0;
		nEdittingTime = 0;
		
		//m_pThread->ClearMessage();
	}
    else if(UDM_CLOSED == mode)
    {
		if(m_IsLoader)
		{
			m_IsLoader = 0;//~m_IsLoader;
			LoadButtomCtl();
			SetTitle(GetParsedString("&CfgPtn.Basic"));
			SwitchPage(0);
		}
		else
		{
			((CPageConfigFrameWork *)(this->GetParent()))->ReflushItemName();
		}
		nThisPageOpened = 0;
		nDateChanged = 0;
		nTimeChanged = 0;
		nEdittingTime = 0;
    }
    else if(UDM_GUIDE == mode)
	{
	    m_IsLoader = 1;//~m_IsLoader;
	    SetTitle(GetParsedString("&CfgPtn.Guide"));
        SwitchPage(1);
		
		//csp modify
		nDateChanged = 0;
		nTimeChanged = 0;
		nEdittingTime = 0;
	}
	
	return TRUE;
}

void CPageBasicConfigFrameWork::WriteLogs()
{
	if(pComboBox0[0]->GetCurSel() != GetVideoFormat())
	{
		BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_VIDEOFORMAT);
	}
	
	if(pCheckBox0[0]->GetValue() != GetAuthCheck())
	{
		if(pCheckBox0[0]->GetValue())
		{
			BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_ENABLE_PWDCHECK);
		}
		else
		{
			BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_DISABLE_PWDCHECK);
		}
	}
	
	if(pComboBox0[1]->GetCurSel() != GetMaxUser())
	{
		BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_MAX_NETUSER);
	}
	
	if(pComboBox0[2]->GetCurSel() != GetOutput())
	{
		BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_VO);
	}
	
	if(pComboBox0[3]->GetCurSel() != GetLanguageIndex())
	{
		BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_LANGUAGE);
	}
}

void CPageBasicConfigFrameWork::SetInfo(char* szInfo)
{
    if (m_IsLoader)
    {
        return;
    }
    
	pIconInfo->Show(TRUE);
		
	pInfoBar->SetText(szInfo);
}

void CPageBasicConfigFrameWork::ClearInfo()
{
	if (m_IsLoader)
    {
        return;
    }
    
	pIconInfo->Show(FALSE);
	
	pInfoBar->SetText("");
}

void CPageBasicConfigFrameWork::LoadButtomCtl()
{
    if(m_IsLoader)
    {
		pButton[0]->Show(FALSE);
		pButton[1]->Show(FALSE);
		pButton[2]->Show(FALSE);
		pButton[3]->Show(FALSE);
		
		pButton[5]->Show(TRUE);
		pButton[6]->Show(TRUE);
		
		//csp modify 20131217
		//pStatic1[8]->SetText("&CfgPtn.ShowGuide");
		//pCheckBox1[1]->SetValue(GetShowGuide());
		pStatic1[9]->SetText("&CfgPtn.ShowGuide");
		pCheckBox1[2]->SetValue(GetShowGuide());
	}
	else
	{
		pButton[0]->Show(TRUE);
		pButton[1]->Show(TRUE);
		pButton[2]->Show(TRUE);
		pButton[3]->Show(TRUE);
		
		pButton[5]->Show(FALSE);
		pButton[6]->Show(FALSE);
		
		//csp modify 20131217
		//pStatic1[8]->SetText("");
		pStatic1[9]->SetText("");
	}
}

VD_BOOL CPageBasicConfigFrameWork::MsgProc( uint msg, uint wpa, uint lpa )
{
	u8 lock_flag = 0;//cw_shutdown
	GetSystemLockStatus(&lock_flag);
	if(lock_flag)
	{
		return FALSE;
	}
	
	int px,py;
	switch(msg)
    {
        case XM_RBUTTONDOWN:
        case XM_RBUTTONDBLCLK:
        {
            if(m_IsLoader)
            {
            	return TRUE;
            }
        } break;
    	case XM_KEYDOWN://cw_panel
    		switch(wpa)
        	{
        		case KEY_RIGHT:
        		case KEY_LEFT:
        		case KEY_UP:
        		case KEY_DOWN:
        		{
        			CPage::MsgProc(msg, wpa, lpa);
        			static CItem* last = NULL;
        			CItem* temp = GetFocusItem();//cw_panel
        			if(pEdit0[1]->IsPadOpen() || pEdit0[0]->IsPadOpen())
        			{
        				break;
        			}
        			
        			if(temp != last)
        			{
        				if(temp == pEdit0[1])
        				{
        					SetInfo("&CfgPtn.MaxScope");
        				}
        				else if(temp == pComboBox0[0])
        				{
        					SetInfo("&CfgPtn.ChangeVideoFormat");
        				}
        				else if(temp == pComboBox0[2])
        				{
        					SetInfo("&CfgPtn.ChangeOutput");
        				}
        				else if(temp == pComboBox0[3])
        				{
        					if(pComboBox0[3]->isListExist && !pComboBox0[2]->isListExist)
        					{
        						//SetInfo("&CfgPtn.ChangeLanguage");
        					}
        					else 
        					{
        						SetInfo("&CfgPtn.ChangeLanguage");
        					}
        				}
        				else
        				{
        					ClearInfo();
        				}

        				last = temp;
        			}
        			return TRUE;
        		}
        		default:break;
        	}
    	case XM_MOUSEMOVE:
    		{
    			static CItem* last = NULL;
    			/*
	    			px = VD_HIWORD(lpa);
	    			py = VD_LOWORD(lpa);
	    			CItem* temp = GetItemAt(px, py);
	    			*/
    			CItem* temp = GetFocusItem();//cw_panel
    			if(pEdit0[1]->IsPadOpen() || pEdit0[0]->IsPadOpen())
    			{
    				break;
    			}
    			
    			if(temp != last)
    			{
    				if(temp == pEdit0[1])
    				{
    					SetInfo("&CfgPtn.MaxScope");
    				}
    				else if(temp == pComboBox0[0])
    				{
    					SetInfo("&CfgPtn.ChangeVideoFormat");
    				}
    				else if(temp == pComboBox0[2])
    				{
    					SetInfo("&CfgPtn.ChangeOutput");
    				}
    				else if(temp == pComboBox0[3])
    				{
    					if(pComboBox0[3]->isListExist && !pComboBox0[2]->isListExist)
    					{
    						//SetInfo("&CfgPtn.ChangeLanguage");
    					}
    					else 
    					{
    						SetInfo("&CfgPtn.ChangeLanguage");
    					}
    				}
    				else if(temp == pDateTime1[1])
    				{
    					nEdittingTime = 1;
    				}
    				else
    				{
    					ClearInfo();
    					nEdittingTime = 0;
    				}
					
    				last = temp;
    			}
    		}
    		break;
    	default:    		
    		break;
    }
    
	return CPage::MsgProc(msg, wpa, lpa);
}


VD_BOOL CPageBasicConfigFrameWork::Open()
{
	//WaitForSem();	
	
	VD_BOOL ret = CPage::Open();
	
	//ReleaseSem();
	return ret;
}

