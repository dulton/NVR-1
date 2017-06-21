#include "GUI/Pages/PageSensorAlarm.h"
#include "GUI/Pages/PageIPCameraExtSensorAlarm.h"
#include "GUI/Pages/PageMDCfg.h"
#include "GUI/Pages/PageVideoLoss.h"
#include "GUI/Pages/PageOtherAlarm.h"
#include "GUI/Pages/PageAlarmOut.h"
#include "GUI/Pages/bizdata.h"
#include "GUI/Pages/PageConfigFrameWork.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageAlarmCfgFrameWork.h"

#include <sys/time.h>

/*
enum ALARM_ITEMS
{
	SENSOR_ALARM,
	MOTION_ALARM,
	VIDEOLOSS_ALARM,
	//OTHER_ALARM,
	ALARMOUT_ALARM,
	EXIT_ALARM,
};

static char* szShortcut[MAX_ALARM_PAGES] = {
	"&CfgPtn.Sensor",
	"&CfgPtn.Motion",	
	"&CfgPtn.VideoLoss",
	//"&CfgPtn.OtherAlarm",
	"&CfgPtn.AlarmOut",
	"&CfgPtn.Exit",
};

static char* szBmpName[MAX_ALARM_PAGES+1][2] = {    
	{DATA_DIR"/temp/sensor.bmp",	DATA_DIR"/temp/sensor_f.bmp"},
	{DATA_DIR"/temp/motion.bmp",	DATA_DIR"/temp/motion_f.bmp"},	
	{DATA_DIR"/temp/videoloss.bmp",	DATA_DIR"/temp/videoloss_f.bmp"},
	//{DATA_DIR"/temp/other_alarm.bmp",	DATA_DIR"/temp/other_alarm_f.bmp"},
	{DATA_DIR"/temp/alarm_out.bmp",	DATA_DIR"/temp/alarm_out_f.bmp"},
	{DATA_DIR"/temp/exit3.bmp",	DATA_DIR"/temp/exit3_f.bmp"},
	{DATA_DIR"/temp/main_pic.bmp",	DATA_DIR"/temp/main_pic.bmp"}, //此项一定要保持在最后一项
};
*/
//yaogang modify 20141013
enum ALARM_ITEMS
{
	LOCAL_SENSOR_ALARM,
	IPC_COVER_ALARM,
	IPC_SENSOR_ALARM,
	MOTION_ALARM,
	VIDEOLOSS_ALARM,
	HDD_ALARM,
	EXT_ALARM,
	ALARMOUT_ALARM,
	EXIT_ALARM,
};

static const char* szShortcut[MAX_ALARM_PAGES] = {
	"&CfgPtn.LocalAlarm",
	"&CfgPtn.IPCCoverAlarm",
	"&CfgPtn.IPCExtAlarm",
	"&CfgPtn.Motion",	
	"&CfgPtn.VideoLoss",
	"&CfgPtn.HDDAlarm",
	"&CfgPtn.ExtAlarm",
	"&CfgPtn.AlarmOut",
	"&CfgPtn.Exit",
};

static const char* szBmpName[MAX_ALARM_PAGES+1][2] = {	
	{DATA_DIR"/temp/sensor.bmp",	DATA_DIR"/temp/sensor_f.bmp"},
	{DATA_DIR"/temp/camera.bmp",	DATA_DIR"/temp/camera_focus.bmp"},
	{DATA_DIR"/temp/sensor.bmp",	DATA_DIR"/temp/sensor_f.bmp"},	
	{DATA_DIR"/temp/motion.bmp",	DATA_DIR"/temp/motion_f.bmp"},	
	{DATA_DIR"/temp/videoloss.bmp", DATA_DIR"/temp/videoloss_f.bmp"},

	{DATA_DIR"/temp/hdd.bmp",	DATA_DIR"/temp/hdd_f.bmp"},
	{DATA_DIR"/temp/alarm.bmp",	DATA_DIR"/temp/alarm_f.bmp"},
	{DATA_DIR"/temp/alarm_out.bmp", DATA_DIR"/temp/alarm_out_f.bmp"},
	{DATA_DIR"/temp/exit3.bmp", DATA_DIR"/temp/exit3_f.bmp"},
	{DATA_DIR"/temp/main_pic.bmp",	DATA_DIR"/temp/main_pic.bmp"}, //此项一定要保持在最后一项
};

//csp modify
//static VD_BITMAP* pBmpButtonNormal[MAX_ALARM_PAGES];
//static VD_BITMAP* pBmpButtonSelect[MAX_ALARM_PAGES];
static VD_BITMAP* pBmpButtonNormal[MAX_ALARM_PAGES+1];
static VD_BITMAP* pBmpButtonSelect[MAX_ALARM_PAGES+1];

CPageAlarmCfgFrameWork::CPageAlarmCfgFrameWork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPageFrame(pRect, psz, icon, pParent)
{
	SetMargin(0,0,0,0);//会影响子控件的起始坐标//csp modify
	
	int i = 0;
	CRect rt;
	BIZ_DT_DBG("0");

	int alarm_pages = MAX_ALARM_PAGES;
	//yaogang modify 20150324
	SBizDvrInfo DvrInfo;
	u8 nNVROrDecoder = 1; //跃天: 1 nvr，2 轮巡解码器，3 切换解码器
	if (BizGetDvrInfo(&DvrInfo) == 0)
	{
		nNVROrDecoder = DvrInfo.nNVROrDecoder;
	}

	int cnt = 0;
	for(i=0; i< MAX_ALARM_PAGES; i++)
	{
		BIZ_DT_DBG("MAX_ALARM_PAGES %d\n", i);
		if (i==5 && nNVROrDecoder != 1)//nNVROrDecoder==2
		{
			pButton[i] = NULL;
			pStatic[i] = NULL;
			continue;
		}
		
		pBmpButtonNormal[i] = VD_LoadBitmap(szBmpName[i][0]);
		pBmpButtonSelect[i] = VD_LoadBitmap(szBmpName[i][1]);
		
		#define PIC_BTN_WIDTH	72
		
		rt.left = 51 + (cnt%4)*(PIC_BTN_WIDTH+78);
		rt.right = rt.left +PIC_BTN_WIDTH;
		rt.top = 62 + (cnt/4)*(PIC_BTN_WIDTH+48);
		rt.bottom = rt.top + PIC_BTN_WIDTH;
		pButton[i] = CreateButton(rt, this, NULL, (CTRLPROC)&CPageAlarmCfgFrameWork::OnClkItem, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
		
		rt.left -= 20+18;
		rt.right += 20+18;
		rt.top = 134 + (cnt/4)*(PIC_BTN_WIDTH+48);
		rt.bottom = rt.top + 30;//PIC_BTN_WIDTH+48;
		pStatic[i] = CreateStatic(rt, this, szShortcut[i]);
		pStatic[i]->SetTextAlign(VD_TA_TOP|VD_TA_XCENTER);
        //pStatic[i]->SetBkColor(VD_RGB(67,77,87));
		cnt++;
		BIZ_DT_DBG("SetTextAlign %d\n", i);
	}

//yaogang modify 20141106 删除显示H.264 DVR图片	
#if 0//csp modify
	//Modify by Lirl on Feb/22/2012,删除显示H.264 DVR图片
	//Modify by Lirl on Jan/12/2012
	//添加显示右下角的图片,图片占用两个控件所占用的空间
	//图片大小:195*50//194*34
	pBmpButtonNormal[i] = VD_LoadBitmap(szBmpName[i][0]);
	rt.top = 322+m_TitleHeight;//352;//csp modify
	rt.left = 205+72+84+pBmpButtonNormal[MAX_ALARM_PAGES-1]->width;
	rt.right = rt.left + pBmpButtonNormal[MAX_ALARM_PAGES]->width;
	rt.bottom = rt.top + pBmpButtonNormal[MAX_ALARM_PAGES]->height;
	pStaticPic = CreateStatic(rt, this, "");
	//pStaticPic->SetBkColor(VD_RGB(67,77,87));
	pStaticPic->SetBitmap(VD_LoadBitmap(szBmpName[MAX_ALARM_PAGES][0]));
#endif
    
	#if 1	
	CRect rtPage(0,0,642,418);
	//m_pPageSenser = new CPageSensorAlarm(rtPage, "&CfgPtn.SensorTitle", NULL, this);
	m_pPageSenser = new CPageSensorAlarm(rtPage, "&CfgPtn.LocalAlarm", NULL, this);
	BIZ_DT_DBG("2");
	//yaogang modify 20141010
	m_pPageIPCCover = new CPageIPCameraExtSensorAlarm(rtPage, "&CfgPtn.IPCCoverAlarm", IPCCover, NULL, this);
	m_pPageIPCExtSensor= new CPageIPCameraExtSensorAlarm(rtPage, "&CfgPtn.IPCExtAlarm", ExtSensorIPC, NULL, this);
	BIZ_DT_DBG("21");
	m_pPage485ExtSensor = new CPageIPCameraExtSensorAlarm(rtPage, "&CfgPtn.ExtAlarm", ExtSensor485, NULL, this);
	BIZ_DT_DBG("22");

	if (nNVROrDecoder != 1)//nNVROrDecoder==2
	{
		m_pPageHDD = NULL;
	}
	else
	{
		m_pPageHDD = new CPageAlarmDeal(rtPage, "&CfgPtn.HDDAlarm", NULL, this, 4);
	}
	//yaogang modify 20141010 end
	
	m_pPageMDCfg = new CPageMDCfg(rtPage, "&CfgPtn.MotionTitle", NULL, this);
	BIZ_DT_DBG("3");
	m_pPageVideoLoss = new CPageVideoLoss(rtPage, "&CfgPtn.VideoLossTitle", NULL, this);
	BIZ_DT_DBG("4");
	//m_pPageOtherAlarm = new CPageOtherAlarm(rtPage, "&CfgPtn.OtherAlarmTitle", NULL, this);
	BIZ_DT_DBG("5");
	m_pPageAlarmOut = new CPageAlarmOut(rtPage, "&CfgPtn.AlarmOutTitle", NULL, this);
	#endif
	
	BIZ_DT_DBG("6\n");
}

CPageAlarmCfgFrameWork::~CPageAlarmCfgFrameWork()
{

}

//add by Lirl on Nov/18/2011
void CPageAlarmCfgFrameWork::ReflushItemName()
{
	for (int i=0; i< MAX_ALARM_PAGES; i++)
	{
		if (pStatic[i])
			pStatic[i]->SetText(pStatic[i]->GetText());
	}
	
   	//pButton[MAX_ALARM_PAGES-1]->Draw();
}

VD_BOOL CPageAlarmCfgFrameWork::UpdateData(UDM mode)
{
    if (UDM_OPEN==mode || UDM_OPENED==mode) 
    {
        ReflushItemName();
    } 
    else if (UDM_CLOSED == mode)
    {
        ((CPageConfigFrameWork *)(this->GetParent()))->ReflushItemName();
    }

    return TRUE;
}
//end

void CPageAlarmCfgFrameWork::OnClkItem()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < MAX_ALARM_PAGES; i++)
	{
		if (pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		switch(i)
    	{
    		case LOCAL_SENSOR_ALARM:
    			{
    				if(!GetSenSorNum())//yzw add
    				{
    					MessageBox("&CfgPtn.HardWareNotSupport","&CfgPtn.WARNING",MB_OK|MB_ICONWARNING);
    					break;
    				}
    				
    				m_pPageSenser->Open();
    			} break;
		//yaogang modify 20141010
		case IPC_COVER_ALARM:
    			{
				//printf("yg CPageAlarmCfgFrameWork IPC_SENSOR_ALARM page open\n");
    				if(!GetVideoMainNum())//yaogang modify 20141010
    				{
    					MessageBox("&CfgPtn.HardWareNotSupport","&CfgPtn.WARNING",MB_OK|MB_ICONWARNING);
    					break;
    				}
    				
    				m_pPageIPCCover->Open();
    			} break;
		case IPC_SENSOR_ALARM:
    			{
				//printf("yg CPageAlarmCfgFrameWork IPC_SENSOR_ALARM page open\n");
    				if(!GetVideoMainNum())//yaogang modify 20141010
    				{
    					MessageBox("&CfgPtn.HardWareNotSupport","&CfgPtn.WARNING",MB_OK|MB_ICONWARNING);
    					break;
    				}
    				
    				m_pPageIPCExtSensor->Open();
    			} break;
		case EXT_ALARM:
    			{
				//printf("yg CPageAlarmCfgFrameWork IPC_SENSOR_ALARM page open\n");
    				if(!GetVideoMainNum())//yaogang modify 20141010
    				{
    					MessageBox("&CfgPtn.HardWareNotSupport","&CfgPtn.WARNING",MB_OK|MB_ICONWARNING);
    					break;
    				}
    				
    				m_pPage485ExtSensor->Open();
    			} break;
		case HDD_ALARM:
			{
				m_pPageHDD->SetAlarmDeal(ALARMDEAL_HDD, -1);
				
				m_pPageHDD->Open();
			}break;
    		case MOTION_ALARM:
    			{
    				PRINT_HERE;
    				//struct timeval tv;
    				//gettimeofday(&tv, NULL);
    				//printf("before open %llu\n", (u64)tv.tv_sec*1000000+tv.tv_usec);
    				m_pPageMDCfg->Open();
    				//gettimeofday(&tv, NULL);
    				//printf("after  open %llu\n", (u64)tv.tv_sec*1000000+tv.tv_usec);
    				PRINT_HERE;
    			} break;
    		case VIDEOLOSS_ALARM:
    			{
    				m_pPageVideoLoss->Open();
    			} break;
    		//case OTHER_ALARM:
    			{
    				//m_pPageOtherAlarm->Open();
    			} break;
    		case ALARMOUT_ALARM:
    			{
					//csp modify
					if(!GetAlarmOutNum())
    				{
    					MessageBox("&CfgPtn.HardWareNotSupport","&CfgPtn.WARNING",MB_OK|MB_ICONWARNING);
    					break;
    				}
					
    				m_pPageAlarmOut->Open();
    			} break;
    		case EXIT_ALARM:
    			{
                    ((CPageConfigFrameWork *)(this->GetParent()))->ReflushItemName();
    				Close(UDM_CLOSED);
    			} break;
    		default:
    			break;
    	}
	}
}

