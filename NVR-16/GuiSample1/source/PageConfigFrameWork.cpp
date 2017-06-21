#include "GUI/Pages/PageConfigFrameWork.h"
#include "GUI/Pages/PageRecordFrameWork.h"
#include "GUI/Pages/PageBasicConfigFrameWork.h"
#include "GUI/Pages/PageAdvanceConfigFrameWork.h"
#include "GUI/Pages/PageLiveConfigFrameWork.h"
#include "GUI/Pages/PageRecordScheduleFrameWork.h"
#include "GUI/Pages/PageAlarmCfgFrameWork.h" 
#include "GUI/Pages/PagePtzConfigFrameWork.h"
#include "GUI/Pages/PageAlarmOut.h"
#include "GUI/Pages/PageUserManager.h"
#include "GUI/Pages/PageOSDPosSet.h"
#include "GUI/Pages/PageNetworkConfig.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/PageShenGuangConfig.h"//shixin

#include "GUI/Pages/PageIPCameraConfigFrameWork.h"//NVR used
#include "sg_platform.h"


enum{//与下面的数组一一对应
	CONFIG_BUTTON_BASIC,
	CONFIG_BUTTON_LIVE,
	CONFIG_BUTTON_RECORD,
	CONFIG_BUTTON_SCHEDULE,	
	CONFIG_BUTTON_ALARM,
	CONFIG_BUTTON_NETWORK,
	CONFIG_BUTTON_USER,
	CONFIG_BUTTON_PTZ,
	CONFIG_BUTTON_ADVANCE,
	CONFIG_BUTTON_IPCAMERA,//NVR used
	CONFIG_BUTTON_SHENGUANG,//shixin
	CONFIG_BUTTON_NUM,
};

char* configShortcut[CONFIG_BUTTON_NUM] = {
	"&CfgPtn.Basic",
	"&CfgPtn.Live",	
	"&CfgPtn.Record",
	"&CfgPtn.RecSchedule",
	"&CfgPtn.Alarm",
	"&CfgPtn.Network",
	"&CfgPtn.Users",
	"&CfgPtn.PTZ",
	"&CfgPtn.Advanced",
	"&CfgPtn.IPCamera",//NVR used
	"&CfgPtn.ShenGuang",//shixin
};

char* configShortcutBmpName[CONFIG_BUTTON_NUM+1][2] = {    
	{DATA_DIR"/temp/basic_config.bmp",	DATA_DIR"/temp/basic_config_f.bmp"},
	{DATA_DIR"/temp/live.bmp",	DATA_DIR"/temp/live_f.bmp"},	
	{DATA_DIR"/temp/record.bmp",	DATA_DIR"/temp/record_f.bmp"},
	{DATA_DIR"/temp/schedule.bmp",	DATA_DIR"/temp/schedule_f.bmp"},
	//{DATA_DIR"/temp/schedule.bmp",	DATA_DIR"/temp/schedule_f.bmp"},
	{DATA_DIR"/temp/alarm.bmp",	DATA_DIR"/temp/alarm_f.bmp"},
	{DATA_DIR"/temp/network.bmp",	DATA_DIR"/temp/network_f.bmp"},
	{DATA_DIR"/temp/user.bmp",	DATA_DIR"/temp/user_f.bmp"},
	{DATA_DIR"/temp/ptz.bmp",	DATA_DIR"/temp/ptz_f.bmp"},
	{DATA_DIR"/temp/advance.bmp",	DATA_DIR"/temp/advance_f.bmp"},
	{DATA_DIR"/temp/camera.bmp",	DATA_DIR"/temp/camera_focus.bmp"},//NVR used
	{DATA_DIR"/temp/sg.bmp",	DATA_DIR"/temp/sg_focus.bmp"},//ShenGuang shixin
	{DATA_DIR"/temp/main_pic.bmp",	DATA_DIR"/temp/main_pic.bmp"}, //此项一定要保持在最后一项
};

//csp modify
//static VD_BITMAP* pBmpButtonNormal[CONFIG_BUTTON_NUM];
//static VD_BITMAP* pBmpButtonSelect[CONFIG_BUTTON_NUM];
static VD_BITMAP* pBmpButtonNormal[CONFIG_BUTTON_NUM+1];
static VD_BITMAP* pBmpButtonSelect[CONFIG_BUTTON_NUM+1];

CPageConfigFrameWork::CPageConfigFrameWork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
, curMainItemSel(0)
{
	int rd = 10;
	int space = 78;
	CRect rtTmp;
	rtTmp.left = 52;
	rtTmp.top =  20;

	SBizDvrInfo DvrInfo;
	u8 nNVROrDecoder = 1; 
	//跃天: 1 nvr，2 轮巡解码器，3 切换解码器
	if (BizGetDvrInfo(&DvrInfo) == 0)
	{
		nNVROrDecoder = DvrInfo.nNVROrDecoder;
	}
	
	int i, cnt, line, col, left, top;
	cnt = 0;
	for(i=0; i<CONFIG_BUTTON_NUM; i++)
	{
		pButton[i] = NULL;
		pStatic[i] = NULL;
		//printf("%s i: %d, cnt: %d\n", __func__, i, cnt);
		if (nNVROrDecoder != 1)//== 2
		{
			if ( i == CONFIG_BUTTON_RECORD \
				|| i == CONFIG_BUTTON_SCHEDULE \
				|| i == CONFIG_BUTTON_SHENGUANG )
			{
				continue;
			}
		}
		else //NVR 不带深广平台版本
		{
			#ifdef NO_MODULE_SG
			if (i == CONFIG_BUTTON_SHENGUANG)
			{
				continue;
			}
			#endif
		}
		

		line = cnt /4;
		col = cnt % 4;
		left = rtTmp.left + (72+space) * col;
		top = rtTmp.top + (40+space) * line;
		
		pBmpButtonNormal[i] = VD_LoadBitmap(configShortcutBmpName[i][0]);
		pBmpButtonSelect[i] = VD_LoadBitmap(configShortcutBmpName[i][1]);
		pButton[i] = CreateButton(CRect(left, top , 
						left+pBmpButtonNormal[i]->width, top+pBmpButtonNormal[i]->height), 
						this, NULL, (CTRLPROC)&CPageConfigFrameWork::OnClkConfigItem, NULL, buttonNormalBmp);
		
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
		
		pStatic[i] = CreateStatic(CRect(left-30-8, top+72, 
						left+pBmpButtonNormal[i]->width+30+8, top+102), this, configShortcut[i]);
		
		pStatic[i]->SetTextAlign(VD_TA_CENTER);

		cnt++;
	}


	
	#if 0
	int i;
	int j;
	for(i=0; i<3; i++)
	{
		for(j=0; j<4; j++)
		{
			int buttonIndex = (i*4+j);
			int staticIndex = buttonIndex;
			
			if(buttonIndex >= CONFIG_BUTTON_NUM)
			{
                //保证最后一次能把显示图片加载进数组中
                pBmpButtonNormal[buttonIndex] = VD_LoadBitmap(configShortcutBmpName[buttonIndex][0]);
				break;
			}
			
			pBmpButtonNormal[buttonIndex] = VD_LoadBitmap(configShortcutBmpName[buttonIndex][0]);
			pBmpButtonSelect[buttonIndex] = VD_LoadBitmap(configShortcutBmpName[buttonIndex][1]);
			pButton[buttonIndex] = CreateButton(CRect(rtTmp.left, rtTmp.top , 
							rtTmp.left+pBmpButtonNormal[buttonIndex]->width, rtTmp.top+pBmpButtonNormal[buttonIndex]->height), 
							this, NULL, (CTRLPROC)&CPageConfigFrameWork::OnClkConfigItem, NULL, buttonNormalBmp);
			
			pButton[buttonIndex]->SetBitmap(pBmpButtonNormal[buttonIndex], pBmpButtonSelect[buttonIndex], pBmpButtonSelect[buttonIndex]);
			
			pStatic[staticIndex] = CreateStatic(CRect(rtTmp.left-30-8, rtTmp.top+72, 
							rtTmp.left+pBmpButtonNormal[buttonIndex]->width+30+8, rtTmp.top+102), this, configShortcut[staticIndex]);
			
			pStatic[staticIndex]->SetTextAlign(VD_TA_CENTER);
            //pStatic[staticIndex]->SetBkColor(VD_RGB(67,77,87));
			
			rtTmp.left += 72+space;
			
			//printf("buttonIndex = %d\n",buttonIndex);
		}
		
		rtTmp.top += 40+space;
		rtTmp.left = 52;
	}
	#endif
//yaogang modify 20141106 删除显示H.264 DVR图片	
#if 0//csp modify
	//Modify by Lirl on Feb/22/2012,删除显示H.264 DVR图片
	//Modify by Lirl on Jan/12/2012
	//添加显示右下角的图片,图片占用两个控件所占用的空间
	//图片大小:195*50//194*34
	rtTmp.top = 310;
	rtTmp.top += 12;//csp modify
	rtTmp.left = 205+72+space+pBmpButtonNormal[CONFIG_BUTTON_NUM-1]->width;
	rtTmp.left += 4;//csp modify
	rtTmp.right = rtTmp.left + pBmpButtonNormal[CONFIG_BUTTON_NUM]->width;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[CONFIG_BUTTON_NUM]->height;
	pStaticPic = CreateStatic(rtTmp, this, "");
	//pStaticPic->SetBkColor(VD_RGB(67,77,87));
	pStaticPic->SetBitmap(VD_LoadBitmap(configShortcutBmpName[CONFIG_BUTTON_NUM][0]));
#endif
	
	//debug_trace_err("CPageRecordFrameWork");
	
	//CRect recordRt = CRect(0, 0, 700, 400/*370*/);
	m_pPageRecord = new CPageRecordFrameWork(NULL/*&recordRt*/, "&CfgPtn.Record"/*"&titles.mainmenu"*/, icon_dvr, this);
    SetPage(EM_PAGE_RECORD, (CPage *)m_pPageRecord);
	
	//debug_trace_err("CPageRecordFrameWork");
	
	m_pPageBasic = new CPageBasicConfigFrameWork(NULL/*&recordRt*/, "&CfgPtn.Basic"/*"&titles.mainmenu"*/, icon_dvr, this);
    SetPage(EM_PAGE_BASICCFG, (CPage *)m_pPageBasic);
	
	//debug_trace_err("CPageRecordFrameWork");
	
	m_pPageAdvance = new CPageAdvanceConfigFrameWork(NULL/*&recordRt*/, "&CfgPtn.Advanced"/*"&titles.mainmenu"*/, icon_dvr, this);
	
	//debug_trace_err("CPageRecordFrameWork44");
	
	m_pPageLive = new CPageLiveConfigFrameWork(NULL/*&recordRt*/, "&CfgPtn.Live"/*"&titles.mainmenu"*/, icon_dvr,this);
	SetPage(EM_PAGE_LIVECFG, m_pPageLive);
	
	//debug_trace_err("CPageRecordFrameWork321");
	
	#if 1
	m_pPagePtzCfg = new CPagePtzConfigFrameWork(NULL/*&recordRt*/, "&CfgPtn.PTZ"/*"&titles.mainmenu"*/, icon_dvr, this);
	//debug_trace_err("CPageRecordFrameWork11");
	#endif
	
	m_pPageRecordSche = new CPageRecordScheduleFrameWork(NULL/*&recordRt*/, "&CfgPtn.RecSchedule"/*"&CfgPtn.RecordSchedule""&titles.mainmenu"*/, icon_dvr, this);
	
	//debug_trace_err("CPageRecordFrameWork123");
	
	m_pPageAlarmCfg = new CPageAlarmCfgFrameWork(NULL, "&CfgPtn.Alarm", icon_dvr, this);
	
	//debug_trace_err("CPageUserManager");
	
	m_pPageUserMgr = new CPageUserManager(NULL, "&CfgPtn.Users", icon_dvr, this);
	
	m_pUserMgr = m_pPageUserMgr;
	
	//PUBPRT("Here");
	SetPage(EM_PAGE_USERMANAGE, (CPage *)m_pUserMgr);
	
	//PUBPRT("Here");
	m_pPageNetworkCfg = new CPageNetworkConfig(NULL, "&CfgPtn.Network", icon_dvr, this);
	//PUBPRT("Here");
	if(m_pPageNetworkCfg)
	{
		SetPage(EM_PAGE_NETCFG, (CPage*)m_pPageNetworkCfg);
	}

	
	m_pPageShenGuangCfg = new CPageShenGuangConfig(NULL, "&CfgPtn.ShenGuang", icon_dvr, this);//shixin
	//yaogang modify 20150127
	SetPage(EM_PAGE_SG_PLATFORM, m_pPageShenGuangCfg);
	
	//NVR used
	m_pPageIPCamera = new CPageIPCameraConfigFrameWork(NULL, "&CfgPtn.IPCamera", icon_dvr, this);
	//printf("NVR m_pPageIPCamera=0x%08x\n",(unsigned int)m_pPageIPCamera);
}

CPageConfigFrameWork::~CPageConfigFrameWork()
{
	
}

VD_PCSTR CPageConfigFrameWork::GetDefualtTitleCenter()
{
	return "Basic Config";
}

void CPageConfigFrameWork::OnTest()
{
	//printf("OnTest\n");
}

void CPageConfigFrameWork::OnClkConfigItem()
{
	//printf("OnClkConfigItem-1,CONFIG_BUTTON_NUM=%d\n",CONFIG_BUTTON_NUM);
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < CONFIG_BUTTON_NUM; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	//printf("OnClkConfigItem-2\n");
	
	if(bFind)
	{
		//printf("OnClkConfigItem-3,index=%d\n",i);
		
		switch(i)
		{
			case CONFIG_BUTTON_BASIC:
			{
				//printf("CONFIG_BUTTON_BASIC\n");
				//printf("m_pPageBasic = %x\n",m_pPageBasic);
				m_pPageBasic->Open();
			}break;
			case CONFIG_BUTTON_LIVE:
			{
				//printf("CONFIG_BUTTON_LIVE\n");
				m_pPageLive->Open();
			}break;
			case CONFIG_BUTTON_RECORD:
			{
				//printf("CONFIG_BUTTON_RECORD\n");
				//this->Close();
				m_pPageRecord->Open();
			}break;
			case CONFIG_BUTTON_SCHEDULE:
			{
				//printf("CONFIG_BUTTON_SCHEDULE\n");
				//this->Close();
				m_pPageRecordSche->Open();
			}break;
			case CONFIG_BUTTON_ALARM:
			{
				m_pPageAlarmCfg->Open();
			}
			break;
			case CONFIG_BUTTON_NETWORK:
			{
				m_pPageNetworkCfg->Open();
			}break;
			case CONFIG_BUTTON_SHENGUANG:
			{
				//m_pPageNetworkCfg->Open();
				m_pPageShenGuangCfg->Open();
			}break;//shixin
			case CONFIG_BUTTON_USER:
			{
				m_pPageUserMgr->Open();
			}break;
			case CONFIG_BUTTON_PTZ:
			{
				SetSystemLockStatus(1);
				
				m_pPagePtzCfg->Open();
				/*
				this->GetParent()->Close();

				bOpenPtz = TRUE;

				this->Close(UDM_CLOSED);
				bOpenPtz = FALSE;
				*/
				this->GetParent()->Show(FALSE);//cw_test
				this->Show(FALSE);
				SetSystemLockStatus(0);
			}break;
			case CONFIG_BUTTON_ADVANCE:
			{
				m_pPageAdvance->Open();
			}break;
			//NVR used
			case CONFIG_BUTTON_IPCAMERA:
			{
				//printf("OnClkConfigItem-CONFIG_BUTTON_IPCAMERA-1\n");
				m_pPageIPCamera->Open();
				//printf("OnClkConfigItem-CONFIG_BUTTON_IPCAMERA-2\n");
			}break;
			default:
			break;
		}
	}
}

//add by Lirl on Nov/18/2011
VD_BOOL CPageConfigFrameWork::UpdateData(UDM mode)
{
    if(UDM_OPEN==mode || UDM_OPENED==mode)
    {
    	//printf("pageconfig open-1^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
        ReflushItemName();
		//printf("pageconfig open-2^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test

		//if(UDM_OPEN == mode)
		{
			//监管平台上传DVR参数设置
			SSG_MSG_TYPE msg;
			memset(&msg, 0, sizeof(msg));
			msg.type = EM_DVR_PARAM_SET;
			msg.chn = 0;
			strcpy(msg.note, GetParsedString("&CfgPtn.ParamSet"));
			upload_sg(&msg);
		}
    }
    else if(UDM_CLOSED == mode)
    {
    	//printf("pageconfig close^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
        ((CPageMainFrameWork *)(this->GetParent()))->ReflushItemName();
    }
	
    return TRUE;
}

void CPageConfigFrameWork::ReflushItemName()
{
    int staticIndex = 0;
    for(int i=0; i<3; i++)
	{
		for(int j=0; j<4; j++)
		{
			staticIndex = (i*4+j);
			if(staticIndex >= CONFIG_BUTTON_NUM)
			{
				return;
			}

			if (pStatic[staticIndex])
				pStatic[staticIndex]->SetText(pStatic[staticIndex]->GetText());
		}
	}
}
//end

VD_BOOL CPageConfigFrameWork::MsgProc( uint msg, uint wpa, uint lpa )
{
	switch(msg)
	{
		case XM_RBUTTONDOWN:
		{
			if((CPage*)(this->GetParentPage()))
			{
				if(((CPage*)(this->GetParentPage()))->GetFlag(IF_OPENED))
				{
					((CPage*)(this->GetParentPage()))->Show(TRUE ,TRUE);
				}
				else
				{
					((CPage*)(this->GetParentPage()))->Open();
				}
			}
		}
		break;
		default:
			break;
	}
	
	return CPageFrame::MsgProc(msg, wpa, lpa);
	//return CPage::MsgProc(msg, wpa, lpa);
}

#if 1//csp modify
VD_BOOL CPageConfigFrameWork::Close(UDM mode)
{
	//cw_test
	/*
	if (!bOpenPtz)
	{
		this->m_pParent->Open();
	}
	*/
    return CPage::Close();
}
#endif

