#include "Main.h"
#include "GUI/GUI.h"
//#include "System/AppConfig.h"
//#include "GUI/Pages/PageUpgrade.h"
//#include "GUI/Pages/PageRemote.h"
#include "GUI/Pages/PageDesktop.h"
//#include "GUI/Pages/PageNoModuleMessageBox.h"

//#include "Devices/DeviceManager.h"
//#include "Devices/DevFrontboard.h"
#include "Devices/DevGraphics.h"
#include "Devices/DevMouse.h"
//#include "Devices/DevVideo.h"

//#include "Functions/Encode.h"
#include "MultiTask/Thread.h"
//#include "Functions/IUpgrader.h"
//extern int g_USBUpdateFlag;
#include "GUI/Pages/PageStartFrameWork.h"
#include "GUI/Pages/PageMessageBox.h"

#include "GUI/Pages/BizData.h"

#if 1//csp modify
uchar color_table[COLOR_ALL][4] = 
{
	//Red		Green	Blue	Alpha	
	//{	33,		33,		33,		128	},//COLOR_WINDOW	
	{   16,		16,		16,		255 },
	//{    255,255,255,255           },
	//{	33,	33,	41,	128	},//COLOR_WINDOW	
	//{	21, 75,	117,	128	},//COLOR_WINDOW	
	//{	33,	41,	49,	128	},//COLOR_WINDOW	青灰色
	//{	38,		38,		38,		128	},//COLOR_POPUP
	{	50,		50,		50,		128	},//COLOR_POPUP		
	{	19,		19,		19,		128	},//COLOR_CTRL
	{	186,	231,	255,	128	},//COLOR_CTRLSELECTED
	{	180,	180,	180,	255	},//COLOR_WINDOWTEXT
	//{	122,	175,	207,	255	},//COLOR_WINDOWTEXT
	//{	255,	255,	255,	255	},//COLOR_TITLETEXT	
	{	255,	255,	255,	255	},//{	90,	90,	200,	255	},//{	63,	124,	192,	255	},//COLOR_TITLETEXT	
	//{	0,		63,		113,	128	},//COLOR_FRAME
	//{	56,		91,		156,	128	},//COLOR_FRAME
	  {	102,	102,	102,	128	},//COLOR_FRAME
	//{	102,	102,	102,	128	},//COLOR_FRAME
	{	0,		174,	255,	128	},//COLOR_FRAMESELECTED
	//{	21,		75,	117,	128	},//COLOR_FRAMESELECTED
	//{	255,	255,	255,	255	},//COLOR_CTRLTEXT
	{	180,	180,	180,	150	},//COLOR_CTRLTEXT
	//{	255,	255,	255,	255	},//COLOR_CTRLTEXT
	//{	26,		66,		190,	255	},//COLOR_CTRLTEXTSELECTED
	{	255,	255,	0,		255	},//COLOR_CTRLTEXTSELECTED
	{	90,		90,		90,		128	},//COLOR_CTRLTEXTDISABLED
	{	255,	0,		255,	0	},//COLOR_TRANSPARENT
};
#else
uchar color_table[COLOR_ALL][4]={
	//Red		Green	Blue	Alpha	
	//{	33,		33,		33,		128	},//COLOR_WINDOW	
	{   17,		17,		17,		255 },
	//{    255,255,255,255           },
	//{	33,	33,	41,	128	},//COLOR_WINDOW	
	//{	21, 75,	117,	128	},//COLOR_WINDOW	
	//{	33,	41,	49,	128	},//COLOR_WINDOW	青灰色
	//{	38,		38,		38,		128	},//COLOR_POPUP
	{	50,		50,		50,		128	},//COLOR_POPUP		
	{	19,		19,		19,		128	},//COLOR_CTRL
	{	186,	231,	255,	128	},//COLOR_CTRLSELECTED
	{	180,	180,	180,	255	},//COLOR_WINDOWTEXT
	//{	122,	175,	207,	255	},//COLOR_WINDOWTEXT
	//{	255,	255,	255,	255	},//COLOR_TITLETEXT	
	{	255,	255,	255,	255	},//{	90,	90,	200,	255	},//{	63,	124,	192,	255	},//COLOR_TITLETEXT	
	//{	0,		63,		113,	128	},//COLOR_FRAME
	//{	56,		91,		156,	128	},//COLOR_FRAME
	  {	102,	102,	102,	128	},//COLOR_FRAME
	//{	102,	102,	102,	128	},//COLOR_FRAME
	{	0,		174,	255,	128	},//COLOR_FRAMESELECTED
	//{	21,		75,	117,	128	},//COLOR_FRAMESELECTED
	//{	255,	255,	255,	255	},//COLOR_CTRLTEXT
	{	180,	180,	180,	150	},//COLOR_CTRLTEXT
	//{	255,	255,	255,	255	},//COLOR_CTRLTEXT
	//{	26,		66,		190,	255	},//COLOR_CTRLTEXTSELECTED
	{	255,	255,	0,		255	},//COLOR_CTRLTEXTSELECTED
	{	90,		90,		90,		128	},//COLOR_CTRLTEXTDISABLED
	{	255,	0,		255,	0	},//COLOR_TRANSPARENT
};
#endif

PATTERN_SINGLETON_IMPLEMENT(CGUI);

CGUI::CGUI() : m_InputIdleTimer("InputIdle"), m_CaretTimer("Caret")
{
	//csp modify 20130323
	m_pDevGraphics = NULL;
	m_pDevMouse = NULL;
	m_pDevFrontboard = NULL;
	pPageGuide = NULL;
	pPageStart = NULL;
	
	pPageDesktop = NULL;
//	pPageDesktop1 = NULL;//csp modify 20130323
//	pPageRemote = NULL;
	m_bPagesCreated = FALSE;
	m_LogoutCount = 0;
	m_LCDScreenClsDown = 0;
	m_OutPutChange = 0;
#ifdef MON_PROTECT_SCREEN
	m_ScreenSaverCount = 0;
#endif
}

CGUI::~CGUI()
{
	
}

void SetDevMouse(CDevMouse* mouse);

static void* GUIStatusCheck(void* p)
{
	CGUI *pGUI = (CGUI *)p;
	if(pGUI == NULL)
	{
		return 0;
	}
	
	sleep(10);
	
	printf("$$$$$$$$$$$$$$$$$$GUIStatusCheck id:%d\n",getpid());
	
	while(1)
	{
		sleep(1);
		
		pGUI->OnIdleCount(0);
	}
	
	return 0;
}

VD_BOOL CGUI::Start()
{
	//printf("CGUI::Start-1\n");
	
	VD_RECT rect;
	
	do
	{
		m_pDevMouse = CDevMouse::instance();
		usleep(100*1000);
	}while(!m_pDevMouse);
	
	//printf("CGUI::Start-2\n");

	//yaogang modify 20170725 YueTian New Board (无前面板)
	#if 0
	do
	{
		m_pDevFrontboard = CDevFrontboard::instance();
		usleep(100*1000);
	}while(!m_pDevFrontboard);
	#endif
	
	//printf("CGUI::Start-3\n");
	
	SetDevMouse(m_pDevMouse);
	//RunThread(0);

	//yaogang modify 20170725 YueTian New Board (无前面板)
	SetDevFrontBoard(m_pDevFrontboard);
	//RunThread(1);
	
	//printf("CGUI::Start-4\n");
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		BIZ_DATA_DBG("OnDevInput BizGetPara Err\n");
	}
	
	//printf("CGUI::Start-5\n");
	
	#if 0//csp modify
	if(!bizSysPara.nShowFirstMenue)//面板OK键和鼠标中键切换输出设备
	{
		m_pDevMouse->AttachInput(this, (SIG_DEV_MOUSE_INPUT)&CGUI::OnDevInput);
		m_pDevFrontboard->AttachInput(this, (SIG_DEV_FB_INPUT)&CGUI::OnDevInput);
		
		SetSystemLockStatus(0);//cw_reboot
		
		int nCount = 0;
		while ((0 == m_OutPutChange) && ++nCount < 20)
		{
			usleep(100 * 1000);
		}
		
		m_pDevMouse->DetachInput(this, (SIG_DEV_MOUSE_INPUT)&CGUI::OnDevInput);
		m_pDevFrontboard->DetachInput(this, (SIG_DEV_FB_INPUT)&CGUI::OnDevInput);
		
		if (m_OutPutChange)
		{
			if (EM_BIZ_OUTPUT_CVBS == bizSysPara.nOutput)
			{
				bizSysPara.nOutput = EM_BIZ_OUTPUT_VGA_1024X768;
			}
			#ifdef CHIP_HISI3531
			else if ((EM_BIZ_OUTPUT_VGA_800X600 == bizSysPara.nOutput)
						|| (EM_BIZ_OUTPUT_VGA_1024X768 == bizSysPara.nOutput)
						|| (EM_BIZ_OUTPUT_VGA_1280X1024 == bizSysPara.nOutput))
			{
				bizSysPara.nOutput = EM_BIZ_OUTPUT_HDMI_1920X1080;
			}
			#endif
			else
			{
				bizSysPara.nOutput = EM_BIZ_OUTPUT_CVBS;
			}
			
			ret = BizSetPara(&bizTar, &bizSysPara);
			if(ret!=0)
			{
				BIZ_DATA_DBG("OnDevInput BizSetPara Err\n");
			}
			
			printf("The device to display menu has been reset to %s !!!!\n", (EM_BIZ_OUTPUT_CVBS == bizSysPara.nOutput)?"CVBS":"VGA");
		}
	}
	#endif
	
	//设置系统颜色
	for(int i=0; i < COLOR_ALL; i++)
	{
		SetSysColor(i, VD_RGB(color_table[i][0], color_table[i][1], color_table[i][2]));
	}
	
	//printf("before CCalendar::SetStrings\n");
	CCalendar::SetStrings("1|2|3|4|5|6|7|8|9|10|11|12", "S|M|T|W|T|F|S");
	
	#if 0//csp modify//没有调用g_TimerManager.Start();
	m_CaretTimer.Start(this, (VD_TIMERPROC)&CGUI::OnCaret, 500, 500);
	m_InputIdleTimer.Start(this, (VD_TIMERPROC)&CGUI::OnIdleCount, 10000, 1000);
	#endif
	
	#if 0//csp modify 20130101
	pthread_t pid = 0;
	pthread_create(&pid, NULL, GUIStatusCheck, this);
	#endif
	
	//printf("\n\n\n\n");
	//sleep(10);
	
	// 初始化GDI对象
	//printf("before InitObjects...\n");
	//fflush(stdout);
	InitObjects();
	
	//printf("\n\n\n\n");
	//sleep(10);
	
#if 0//csp modify 20130101
	//printf("bizSysPara.nShowFirstMenue:%d\n",bizSysPara.nShowFirstMenue);
	if(bizSysPara.nShowFirstMenue)//恢复出厂设置后第一次开机,显示菜单输出选择界面
	{
		//面板OK键和鼠标中键切换输出设备
		m_pDevMouse->AttachInput(this, (SIG_DEV_MOUSE_INPUT)&CGUI::OnMouseInput);
		m_pDevFrontboard->AttachInput(this, (SIG_DEV_FB_INPUT)&CGUI::onFrontboardIntput);
		
		//csp modify
		//m_pDevGraphics = CDevGraphics::instance(2);
		m_pDevGraphics = CDevGraphics::instance();
		
		SetVideoStandard((EM_BIZ_NTSC == bizSysPara.nVideoStandard) ? 1 : 0);
		
		m_pDevGraphics->GetRect(&rect);
		memcpy(&sNewMouseRect, &rect, sizeof(VD_RECT));	
		m_pDevMouse->SetRect(&sNewMouseRect);
		
		//printf("before ShowStartPage\n");
		ShowStartPage();
		//printf("after ShowStartPage$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
		
		//csp modify
		if(CDC::GetModify())
		{
			GraphicsSyncVgaToCvbs();
			CDC::SetModify(FALSE);
		}
		
		//csp modify
		//m_pDevGraphics->DestroyGraphics(2);
		//m_pDevGraphics = CDevGraphics::instance(-1);
		
		//printf("before DetachInput\n");
		m_pDevMouse->DetachInput(this, (SIG_DEV_MOUSE_INPUT)&CGUI::OnMouseInput);
		m_pDevFrontboard->DetachInput(this, (SIG_DEV_FB_INPUT)&CGUI::onFrontboardIntput);				
		
		BizSystemRestart();//重启
	}
	else
#endif
	{
		SetSystemLockStatus(0);//cw_reboot
	}
	
	if(m_pDevGraphics == NULL)//csp modify 20130101
	{
		//printf("before CDevGraphics::instance\n");
		//fflush(stdout);
		
		m_pDevGraphics = CDevGraphics::instance();
	}
	
	//printf("before SetVideoStandard...\n");
	//fflush(stdout);
	
	//printf("\n\n\n\n");
	//fflush(stdout);
	//sleep(10);
	
	SetVideoStandard((EM_BIZ_NTSC == bizSysPara.nVideoStandard) ? 1 : 0);
	
	//printf("\n\n\n\n");
	//fflush(stdout);
	//sleep(10);
	
	m_pDevGraphics->GetRect(&rect);
	memcpy(&sNewMouseRect, &rect, sizeof(VD_RECT));
	//printf("DevMouse rect:(%d,%d,%d,%d)\n",sNewMouseRect.left,sNewMouseRect.right,sNewMouseRect.top,sNewMouseRect.bottom);
	m_pDevMouse->SetRect(&sNewMouseRect);
	
	SetSystemLockStatus(1);//cw_reboot

	if (m_pDevMouse)
		m_pDevMouse->AttachInput(this, (SIG_DEV_MOUSE_INPUT)&CGUI::OnMouseInput);

	if (m_pDevFrontboard)
		m_pDevFrontboard->AttachInput(this, (SIG_DEV_FB_INPUT)&CGUI::onFrontboardIntput);
	
	#if 0//debug for 3531
	printf("show logo$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
	m_pDevGraphics->ShowJpegPic("/tmp/ui/logo/logo.jpg", 1);//此处显示logo
	remove("/tmp/ui/logo/logo.jpg");//删除logo文件
	#else
	m_pDevGraphics->ShowJpegPic((char *)"/tmp2/logo.jpg", 1);//此处显示logo//csp test
	//printf("show app logo\n");
	//sleep(10);
	//printf("show app logo - 2\n");
	remove("/tmp2/logo.jpg");//删除logo文件
	#endif
	
	#if 1//csp modify 20130101
	pthread_t pid = 0;
	pthread_create(&pid, NULL, GUIStatusCheck, this);
	#endif
	
	//csp modify
	//if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
	
	printf("CGUI::Start-finish\n");
	
	//while(1) sleep(1);//csp test
	
	return TRUE;
}

VD_BOOL CGUI::Stop()
{
	//printf("CGUI::Stop()>>>>>>>>>\n");
	m_CaretTimer.Stop();
	return TRUE;
}

void CGUI::HideScreen()
{
#if 1//csp modify 20130101
	GraphicShowJpegPic(0, 0);
#else
	if(m_pDevGraphics == NULL)//csp modify
	{
		printf("warning:CGUI::HideScreen m_pDevGraphics is NULL############################################################\n");
		//m_pDevGraphics = CDevGraphics::instance();
		//printf("CGUI::HideScreen m_pDevGraphics is built:0x%08x\n",(unsigned int)m_pDevGraphics);
	}
	else
	{
		printf("CGUI::HideScreen m_pDevGraphics is OK############################################################\n");
	}
	
	m_pDevGraphics->ShowJpegPic(0, 0);//此处是清屏//仅仅清cvbs的屏
#endif
	
#if 0
	//csp modify
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
#endif
}

#if 0
void CGUI::onAppEvent(appEventCode code, int index, appEventAction action, EVENT_HANDLER *param, const CConfigTable* data)
{
	CItem *pItemSpecial = NULL;
	
	if ((appEventUpgrade == code) && (NULL != data))
	{
		if ((*data)["State"] == "Preparing")
		{
			infof("CGUI::onApp Logout.\n");
			Logout();		
			SystemSleep(100);
			return;
		}		
		else if((*data)["State"] == "Upgrading" )
		{
			int iPro = (*data)["Progress"].asInt();
			if (pPageUpgrade)
			{
				if (0 == iPro)
				{
					tracef("CGUI::open UpgradePage.\n");
					pPageUpgrade->Open();					
				}
				else if (100 == iPro)
				{			
					pPageUpgrade->SetProgress(iPro);
					pPageUpgrade->Close();					
					if(m_pPageUpgradeOK)
					{
						m_pPageUpgradeOK->Open();
						m_pPageUpgradeOK->UpdateSecond();
					}
				}				
				pPageUpgrade->SetProgress(iPro);
			}
		}
		
		/// 升级失败，提示失败信息；是否需要详细显示为何失败??
		if((*data)["State"] == "Failed")
		{
			if(m_pPageUpgradeOK)
			{
				m_pPageUpgradeOK->SetDisplayInfo(LOADSTR("advc_sysupgrade.msg_failed"));
				m_pPageUpgradeOK->Open();
				m_pPageUpgradeOK->UpdateSecond();
			}
		}
	}
	
	// 不必显示提示
	if(!m_bPagesCreated || action != appEventStart || !param || !param->bTip)
	{
		return;
	}
	
	switch(code)
	{
	case appEventAlarmLocal:
	case appEventAlarmNet:
	case appEventAlarmDecoder:
	case appEventVideoMotion:
	case appEventVideoLoss:
	case appEventVideoBlind:
	case appEventNetAbort:
	case appEventNetArp:
	case appEventDecConnect:
	case appEventStorageNotExist:   //add by nike.xie 2009-07-08
		pItemSpecial = CPage::FindPage(0, "&titles.alarm");
#ifdef MON_PROTECT_SCREEN
		m_ScreenSaverCount = 0;
#endif
		break;
	//delete by nike.xie 2009-07-08
	//case appEventStorageNotExist:		
		//pMessageBox->Open(LOADSTR("comstring.nodisk"));
		//break;
	//end
	case appEventStorageFailure:
	case appEventStorageLowSpace:
		pItemSpecial = CPage::FindPage(0, "&titles.hddinfo");
		break;		
	case appEventStorageReadErr:
		pMessageBox->Open(LOADSTR("comstring.diskreaderr"));
		break;
	case appEventStorageWriteErr:
		pMessageBox->Open(LOADSTR("comstring.diskwriteerr"));
		break;
	default:
		break;
	}
	
	if(pItemSpecial)
	{
		pItemSpecial->Open();
	}
	else
	{
		//tracepoint();
	}
}
#endif

void CGUI::InitObjects()
{
	//make cursors
	//printf("CGUI::InitObjects-1\n");
	CCursor::VD_LoadCursor(DATA_DIR"/cursors/arrow.cur", CR_NORMAL);
	//printf("CGUI::InitObjects-2\n");
	CCursor::VD_LoadCursor(DATA_DIR"/cursors/busy.cur", CR_BUSY);
	//printf("CGUI::InitObjects-3\n");
	CCursor::VD_LoadCursor(DATA_DIR"/cursors/wait.cur", CR_WAIT);
	CCursor::VD_LoadCursor(DATA_DIR"/cursors/move.cur", CR_MOVE);
	CCursor::VD_LoadCursor(DATA_DIR"/cursors/size1.cur", CR_SIZE1);
	CCursor::VD_LoadCursor(DATA_DIR"/cursors/size2.cur", CR_SIZE2);
	CCursor::VD_LoadCursor(DATA_DIR"/cursors/size3.cur", CR_SIZE3);
	CCursor::VD_LoadCursor(DATA_DIR"/cursors/size4.cur", CR_SIZE4);
	CCursor::VD_LoadCursor(DATA_DIR"/cursors/zoomin.cur", CR_ZOOMIN);
	
	return;
	
#if 0//csp modify
	// bitmaps for page 
	CPage::SetDefaultBitmap(PGB_KEY_PAGEDOWN, DATA_DIR"/bmp_pageup.bmp");
	CPage::SetDefaultBitmap(PGB_KEY_PAGEUP, DATA_DIR"/bmp_pagedown.bmp");
	CPage::SetDefaultBitmap(PGB_KEY_FUNCTION, DATA_DIR"/bmp_function.bmp");
	CPage::SetDefaultBitmap(PGB_KEY_ENTER, DATA_DIR"/bmp_enter.bmp");
	CPage::SetDefaultBitmap(PGB_ICON_SMALL, DATA_DIR"/dvr.bmp");
	CPage::SetDefaultBitmap(PGB_CLOSE_BUTTON_NORMAL, DATA_DIR"/button_close_normal.bmp");
	CPage::SetDefaultBitmap(PGB_CLOSE_BUTTON_SELECTED, DATA_DIR"/button_close_selected.bmp");
	CPage::SetDefaultBitmap(PGB_CLOSE_BUTTON_PUSHED, DATA_DIR"/button_close_pushed.bmp");
	CPage::SetDefaultBitmap(PGB_CLOSE_BUTTON_DISABLED, DATA_DIR"/button_close_disabled.bmp");
	
	// bitmaps for menu
	CMenu::SetDefaultBitmap(MNB_PANEL, DATA_DIR"/menu/tpl_menu_popup_without_title.bmp");
	CMenu::SetDefaultBitmap(MNB_TITLED_PANEL, DATA_DIR"/menu/tpl_menu_popup_with_title.bmp");
	CMenu::SetDefaultBitmap(MNB_ITEM_NORMAL, DATA_DIR"/menu/tpl_menu_popup_item1.bmp");
	CMenu::SetDefaultBitmap(MNB_ITEM_SELECTED, DATA_DIR"/menu/tpl_menu_popup_item2.bmp");
	CMenu::SetDefaultBitmap(MNB_BAR, DATA_DIR"/menu/tpl_menu_bar.bmp");
	
	// bitmaps for group box
	CGroupBox::SetDefaultBitmap(GBB_TAB, DATA_DIR"/bmp_tab.bmp");
	
	// bitmaps for check box
	CCheckBox::SetDefaultBitmap(CKB_NORMAL_TRUE, DATA_DIR"/checkbox/checkbox01.bmp");
	CCheckBox::SetDefaultBitmap(CKB_SELECTED_TRUE, DATA_DIR"/checkbox/checkbox11.bmp");
	CCheckBox::SetDefaultBitmap(CKB_NORMAL_FALSE, DATA_DIR"/checkbox/checkbox00.bmp");
	CCheckBox::SetDefaultBitmap(CKB_SELECTED_FALSE, DATA_DIR"/checkbox/checkbox10.bmp");
	
	// bitmaps for slider
	CSliderCtrl::SetDefaultBitmap(SDB_RAIL_NORMAL, DATA_DIR"/ctrl/slider/slider_normal.bmp");
	CSliderCtrl::SetDefaultBitmap(SDB_RAIL_SELECTED, DATA_DIR"/ctrl/slider/slider_select.bmp");
	CSliderCtrl::SetDefaultBitmap(SDB_TRACKER_NORMAL, DATA_DIR"/ctrl/slider/tracker_normal.bmp");  
	CSliderCtrl::SetDefaultBitmap(SDB_TRACKER_SELECTED, DATA_DIR"/ctrl/slider/tracker_select.bmp");
	
	// bitmaps for combo box
	CComboBox::SetDefaultBitmap(CBB_NORMAL, DATA_DIR"/combobox/combobox_normal.bmp");
	CComboBox::SetDefaultBitmap(CBB_PUSHED, DATA_DIR"/combobox/combobox_push.bmp");
	CComboBox::SetDefaultBitmap(CBB_SELECTED, DATA_DIR"/combobox/combobox_select.bmp");
	CComboBox::SetDefaultBitmap(CBB_DISABLED, DATA_DIR"/combobox/combobox_disable.bmp");
	
	// bitmaps for radio
	//CRadio::SetDefaultBitmap(RAD_NORMAL, DATA_DIR"/radio/radio_normal.bmp");
	//CRadio::SetDefaultBitmap(RAD_PUSHED, DATA_DIR"/radio/radio_push.bmp");
	//CRadio::SetDefaultBitmap(RAD_SELECTED, DATA_DIR"/radio/radio_select.bmp");
	//CRadio::SetDefaultBitmap(RAD_DISABLED, DATA_DIR"/radio/radio_disable.bmp");
	
	//bitmap for number pad
	CNumberPad::SetDefaultBitmap(NPB_PANEL, DATA_DIR"/popup/bmp_popup.bmp");
	CNumberPad::SetDefaultBitmap(NPB_BUTTON_NORMAL, DATA_DIR"/button/button_normal.bmp");
	CNumberPad::SetDefaultBitmap(NPB_BUTTON_SELECTED, DATA_DIR"/button/button_select.bmp");
	
	// bitmaps for date time ctrl
	CDateTimeCtrl::SetDefaultBitmap(DTB_NORMAL, DATA_DIR"/textbox/textbox1.bmp");
	CDateTimeCtrl::SetDefaultBitmap(DTB_SELECTED, DATA_DIR"/textbox/textbox2.bmp");
	
	// bitmaps for edit
	CEdit::SetDefaultBitmap(ETB_NORMAL, DATA_DIR"/textbox/textbox1.bmp");
	CEdit::SetDefaultBitmap(ETB_SELECTED, DATA_DIR"/textbox/textbox2.bmp");
	
	// bitmaps for ip address ctrl
	CIPAddressCtrl::SetDefaultBitmap(IPB_NORMAL, DATA_DIR"/textbox/textbox1.bmp");
	CIPAddressCtrl::SetDefaultBitmap(IPB_SELECTED, DATA_DIR"/textbox/textbox2.bmp");
	
	// bitmaps for list box
	CListBox::SetDefaultBitmap(LTB_NORMAL, DATA_DIR"/textbox/textbox1.bmp");
	CListBox::SetDefaultBitmap(LTB_SELECTED, DATA_DIR"/textbox/textbox2.bmp");
	
	// bitmaps for number box
	CNumberBox::SetDefaultBitmap(NBB_NORMAL, DATA_DIR"/textbox/textbox1.bmp");
	CNumberBox::SetDefaultBitmap(NBB_SELECTED, DATA_DIR"/textbox/textbox2.bmp");
	
	// bitmaps for scroll bar
	CScrollBar::SetDefaultBitmap(SBB_NORMAL, DATA_DIR"/textbox/textbox1.bmp");
	CScrollBar::SetDefaultBitmap(SBB_SELECTED, DATA_DIR"/textbox/textbox2.bmp");
	
	// bitmaps for timing ctrl
	CTimingCtrl::SetDefaultBitmap(TCB_NORMAL, DATA_DIR"/textbox/textbox1.bmp");
	CTimingCtrl::SetDefaultBitmap(TCB_SELECTED, DATA_DIR"/textbox/textbox2.bmp");
	
	// bitmaps for button
	CButton::SetDefaultBitmap(BTB_NORMAL, DATA_DIR"/Button/button_normal.bmp");
	CButton::SetDefaultBitmap(BTB_PUSHED, DATA_DIR"/Button/button_push.bmp");
	CButton::SetDefaultBitmap(BTB_SELECTED, DATA_DIR"/Button/button_select.bmp");
	CButton::SetDefaultBitmap(BTB_DISABLED, DATA_DIR"/Button/button_disable.bmp");
#endif
}

void CGUI::ShowStartPage()
{
#if 0//csp modify 20130323
	//printf("######file:%s, func:%s, line:%d\n",__FILE__, __FUNCTION__, __LINE__);
	pPageStart = new CPageMenueOutputDev(NULL, "&CfgPtn.DispalyMenueDevice");
	//printf("file:%s, func:%s, line:%d\n",__FILE__, __FUNCTION__, __LINE__);
	if(pPageStart)
	{
		//printf("file:%s, func:%s, line:%d\n",__FILE__, __FUNCTION__, __LINE__);
		pPageStart->Open();
		//printf("file:%s, func:%s, line:%d\n",__FILE__, __FUNCTION__, __LINE__);
	}
	//printf("file:%s, func:%s, line:%d\n",__FILE__, __FUNCTION__, __LINE__);
	//u8 flag = 1;
	CCursor::instance(0)->EnableCursor(TRUE);//cw_test
	//csp modify
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
	SetSystemLockStatus(0);//cw_reboot
	while(!pPageStart->IsClosed)
	{
		usleep(50000);
		//if(flag++ > 100) break;
	}
#endif
}

VD_BOOL CGUI::CreatePages()
{
	//create pages
	
	//sleep(6);//csp test
	
	printf("CGUI::CreatePages start\n");
	
	pPageGuide = new CPageGuide(NULL, "&CfgPtn.Guide");
	printf("CGUI::Guide:%p\n", pPageGuide);
	SetPage(EM_PAGE_GUIDE, (CPage *)pPageGuide);
	
	printf("CGUI::CreatePages 1\n");
	
	pPageDesktop = new CPageDesktop(NULL, "Desktop");
	printf("CGUI::Desktop:%p\n", pPageDesktop);
	SetPage(EM_PAGE_DESKTOP, pPageDesktop);
	
	printf("CGUI::CreatePages 2\n");
	
// 	if(g_nVideoOut > 1)
// 	{
// 		pPageDesktop1 = new CPageDesktop(NULL, "Desktop", NULL, (CPage*)1);
// 	}
//	pMessageBox = new CPageNoModuleMessageBox(NULL, "&comstring.commessage");
	
	//dump items
	//printf("CGUI::CreatePages 3\n");
	//CItem::DumpInstances();//csp modify
	//printf("CGUI::CreatePages 4\n");
	
	m_bPagesCreated = TRUE;
	
	//printf("CGUI::CreatePages 5\n");
	
#if 0//csp modify 20130101
	//open the desktop
	pPageDesktop->Open();//打开桌面
	
#if 0
//	if(pPageDesktop1)
//	{
//		pPageDesktop1->Open();//打开桌面1
//	}
#endif
	
	//printf("CGUI::CreatePages 6\n");
	CCursor::instance(0)->EnableCursor(m_pDevMouse->HasFound());//add by nike.xie 20091020//在此处设置鼠标状态
	//printf("CGUI::CreatePages end\n");
	//CCursor::instance(0)->ShowCursor(TRUE);
#endif
	
	return TRUE;
}

void CGUI::onFrontboardIntput(uint message, uint param0, uint param1)
{
	OnHumanInput(message, param0, param1, 0);
}

void CGUI::OnMouseInput(uint message, uint param0, uint param1)
{
	OnHumanInput(message, param0, param1, 0);
}

void CGUI::OnDevInput(uint message, uint param0, uint param1)
{
	if (0 == m_OutPutChange)
	{
		if ((XM_KEYDOWN == message && KEY_RET == param0) || (XM_MBUTTONDOWN == message))
		{
			m_OutPutChange = 1;
		}
	}
}

void CGUI::OnHumanInput(uint msg, uint wpa, uint lpa, int screen)
{
#if 1//csp modify
	if(msg != XM_KEEPALIVE)
	{
		CountDown();//自动注销倒计时
	}
#endif
	
	//printf("OnHumanInput :msg:0x%x %d,%d \n", msg,VD_HIWORD(lpa),VD_LOWORD(lpa));
//	return;
	
	uchar key;
	int px,py;
	
	//遥控功能没有打开时不处理遥控键，除非是地址设置页面。
// 	if((lpa & FB_FLAG_REMOTE) && !m_bRemoteEnabled && (msg == XM_KEYDOWN || msg == XM_KEYUP))
// 	{
// 		if (wpa != KEY_ADDR && !(pPageRemote && pPageRemote->IsActive()))
// 			return;
// 	}
	
	//鼠标丢失和找到的消息处理
	if(msg == XM_MOUSEFOUND)
	{
		CCursor::instance(screen)->EnableCursor(TRUE);

		//csp modify
		if(CDC::GetModify())
		{
			GraphicsSyncVgaToCvbs();
			CDC::SetModify(FALSE);
		}
	}
	if(msg == XM_MOUSELOST)
	{
		CCursor::instance(screen)->EnableCursor(FALSE);
		
		//csp modify
		if(CDC::GetModify())
		{
			GraphicsSyncVgaToCvbs();
			CDC::SetModify(FALSE);
		}
	}
	
	//电源键的处理
	if((msg == XM_KEYDOWN || msg == XM_KEYUP) && wpa == KEY_SHUT && pPageDesktop)
	{
		pPageDesktop->VD_SendMessage(msg, wpa, lpa);
	}
	
#ifndef MON_OPEN_TVMONITORBYPOWER//add by yanjun 20061212    
// 	else if(msg != XM_KEEPALIVE && g_CapsEx.ForATM == 2)	//唤醒液晶面板
// 	{
// 		if(!m_pDevVideo->GetTVMonitorState())
// 		{
// 			m_pDevVideo->SwitchTVMonitor(TRUE);
// 		}
// 		m_LCDScreenClsDown = 0;//置位
// 	}
#endif
	//除了保活消息以外的任何消息
// 	if(msg != XM_KEEPALIVE)
// 	{
// 		if (g_CapsEx.ForATM == 2)
// 		{
// 			m_LCDScreenClsDown = 0;//置位		
// 		}
// 		CountDown(); //自动注销倒计时
// 	}
	
	switch(msg)
	{
	case XM_KEYDOWN://系统按键
		key = wpa;
		//monitoring
		switch(key)
		{
		case KEY_SPLIT:
			//g_Display.NextSplitMode();
			break; //让多画面键继续被其他页面处理
		case KEY_SPLIT1:
			//g_Display.SetSplit(SPLIT1, 0);
			return;
		case KEY_SPLIT4:
			//g_Display.SetSplit(SPLIT4, 0);
			return;
		case KEY_SPLIT8:
			//g_Display.SetSplit(SPLIT8, 0);
			return;
		case KEY_SPLIT9:
			//g_Display.SetSplit(SPLIT9, 0);
			return;
		case KEY_SPLIT16:
			//g_Display.SetSplit(SPLIT16, 0);
			return;
		case KEY_ADDR://Enter remote address page
// 			if(!pPageRemote->IsActive())
// 			{
// 				pPageRemote->Open();
// 			}
			return;
		}
		break;
	//录像码流为0, 不响应保活消息
	case XM_KEEPALIVE:
		{
// 			if (!g_Encode.IsAlive() && (false == IUpgrader::instance()->getState())) //modified by nike.xie 判断系统是否处于非升级状态
// 			{
// 				trace("System Will Reboot for Encoders are Not Working...\n");
// 				return;
// 			}
#ifdef FUNC_THREAD_DETECT
			std::string  threadName;
			if(g_ThreadManager.GetTimeOutThread(threadName))
			{
				printf("System Will Reboot for Thread are Not Working Normal...\n");
				
				// 将错误日志写入文件信息			
				static CFile errorFile;
				
				if(!errorFile.Open("/mnt/mtd/Config/error", CFile::modeReadWrite|CFile::modeCreate| CFile::modeNoTruncate))
				{
					printf("Open Thread error file failed\n");
					return;
				}
				else
				{
					printf("Open Thread error file successfully\n");
				}
				if(errorFile.GetLength() > 16 * 1024)
				{
					// 文件过大，就不写了
					errorFile.Close();
					return;
				}
				
				char tmp[1024];
				memset(tmp, 0, sizeof(tmp));
				SYSTEM_TIME sys;
				SystemGetCurrentTime(&sys);
				sprintf(tmp, "Thread: %s is time out at %4d-%2d-%2d %2d-%2d-%2d.\n", threadName.c_str(),sys.year,
						sys.month, sys.day,sys.hour,sys.minute,sys.second);
				
				errorFile.Seek(0, SEEK_END);
				errorFile.Write(tmp, strlen(tmp));
				errorFile.Close();
				return;
			} 
#endif
		}
	
		if (m_pDevFrontboard)
			m_pDevFrontboard->KeepAlive();
		return;
	default:
		break;
	}
	
	//移动光标
	if(msg == XM_MOUSEMOVE)
	{
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		//printf("mousemove : %d,%d \n", px,py);
		CCursor::instance(screen)->SetCursorPos(px, py);
		
		//csp modify
		if(CDC::GetModify())
		{
			GraphicsSyncVgaToCvbs();
			CDC::SetModify(FALSE);
		}
	}
	//分发
	//printf("yg into VD_DispatchMessage, msg:0x%x %d,%d \n", msg,VD_HIWORD(lpa),VD_LOWORD(lpa));
	CItem::VD_DispatchMessage(screen, msg, wpa, lpa);
	
	//闪烁遥控器指示灯
	CGuard guard(m_Mutex);
	if((msg == XM_KEYDOWN || msg == XM_KEYUP) && (lpa & FB_FLAG_REMOTE) && m_bRemoteEnabled)
	{
		if (m_pDevFrontboard)
			m_pDevFrontboard->LigtenLed(LED_REMOTE, FALSE);
		
		SystemSleep(20);

		if (m_pDevFrontboard)
			m_pDevFrontboard->LigtenLed(LED_REMOTE, TRUE);
	}
}

//extern std::string INI_DEFAULT_USER_NAME;
//extern std::string INI_DEFAULT_USER_PWD;

void CGUI::Logout(int iPageCloseFlag)
{
#if 1//csp modify
	//printf("Logout-1\n");
	
	if(GetPlayBakStatus()==1)
	{
		//printf("Logout-2\n");
		//CPage** page = GetPage();
		//((CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK])->StopPb();
		m_LogoutCount = 0;
		return;
	}
	
	//printf("Logout-3\n");
	
	if(IsGUIUserLogined())
	{
		//printf("Logout-4\n");
		
		u32 g_nUserId = GetGUILoginedUserID();
		if(0 == BizUserLogout(g_nUserId))
		{
			CPageDesktop::SetLogin(0);
			SetGUIUserIsLogined(0);
			
			//printf("Logout-5\n");
			
			CItem::CloseAll(iPageCloseFlag);
			
			//Close();
			//printf("LogOut OK......\n");
			
			BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_LOGOFF);
			
			//BizSysComplexExit( EM_BIZSYSEXIT_LOCK );
		}
		
		//printf("Logout-6\n");
	}
	
	//printf("Logout-7\n");
#else
	CItem::CloseAll(iPageCloseFlag);
#endif
	
//	CGuard guard(m_Mutex);//csp modify
	
// 	if (g_localUser.valid() && !g_userManager.isDefaultUser(g_localUser.getName()))
// 	{
// 		g_localUser.logout();
// 		g_localUser.login(INI_DEFAULT_USER_NAME, INI_DEFAULT_USER_PWD, "GUI");
// 	}
}

void CGUI::OnIdleCount(uint param)
{
#if 1//csp modify
	//printf("OnIdleCount-1\n");
	
	static u16 nLockTime = 0;
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	int ret = BizGetPara(&bizTar, &bizSysPara);
	if(ret!=0)
	{
		//BIZ_DATA_DBG("OnIdleCount\n");
		m_LogoutCount = 0;
		return;
	}
	
	//printf("OnIdleCount-2\n");
	
	if(nLockTime != bizSysPara.nLockTime)
	{
		nLockTime = bizSysPara.nLockTime;
		m_LogoutCount = 0;
		return;
	}
	
	//printf("OnIdleCount nLockTime=%d,m_LogoutCount=%d\n",nLockTime,m_LogoutCount);
	
	if(nLockTime == 0)
	{
		m_LogoutCount = 0;
	}
	else
	{
		if(++m_LogoutCount > nLockTime)
		{
			m_LogoutCount = 0;
			//printf("Logout......\n");
			Logout(1);
		}
	}
#endif
	
#if 0
	if(CConfigGeneral::getLatest().iAutoLogout == 0)
	{
		m_LogoutCount = 0;
	}
	else
	{
		if(++m_LogoutCount > CConfigGeneral::getLatest().iAutoLogout * 60)
		{
			m_LogoutCount = 0;
			Logout(1);
			
			//即使用户没有登陆, 以下代码也会被调用到, 保证了ATM自动关闭液晶面板.
			/*if(g_CapsEx.ForATM == 2)	//带液晶的ATM机
			{
				m_pDevVideo->SwitchTVMonitor(FALSE);
			}*/
		}
	}
	
	if(CConfigGeneral::getLatest().iLCDScreenClsDown == 0)
	{
		m_LCDScreenClsDown = 0;
	}
	else
	{
		if(++m_LCDScreenClsDown > CConfigGeneral::getLatest().iLCDScreenClsDown * 60)
		{
			m_LCDScreenClsDown = 0;
			
			//即使用户没有登陆, 以下代码也会被调用到, 保证了ATM自动关闭液晶面板.
			if(g_CapsEx.ForATM == 2)	//带液晶的ATM机
			{
				m_pDevVideo->SwitchTVMonitor(FALSE);
			}
		}
	}
	
	#ifdef MON_PROTECT_SCREEN
	if(CConfigGUISet::getLatest().bScreenSaveEn) //屏幕保护开启
	{
		if(++m_ScreenSaverCount > CConfigGUISet::getLatest().iScreenSaveTime * 60)
		{
			m_ScreenSaverCount = 0;

			if(pPageDesktop)
			{
				g_Challenger.GetPageStart()->Open(CPageStart::PAGE_APPSCREENPROTECT);
			}
		}
	}
	else
	{
		m_ScreenSaverCount = 0;
		/*if(pPageDesktop)
		{tracepoint();
			g_Challenger.GetPageStart()->Close();
		}*/
	}
	#endif
#endif
}

void CGUI::OnCaret(uint param)
{
	//printf("OnCaret-1\n");
	
	for(int i = 0; i < N_GUI_CORE; i++)
	{
		CItem::VD_DispatchMessage(i, XM_CARET, 0, 0);
	}
}

void CGUI::CountDown()
{
#ifdef MON_PROTECT_SCREEN
	m_ScreenSaverCount = 0;
#endif
	//printf("clear m_LogoutCount......\n");
	m_LogoutCount = 0;
}

CPage * CGUI::GetDesktop()
{
	return pPageDesktop;
}

void CGUI::RemoteEnable(VD_BOOL bFlag)
{
	CGuard guard(m_Mutex);
	
	if (m_pDevFrontboard)
		m_pDevFrontboard->LigtenLed(LED_REMOTE, bFlag);
	//g_Cmos.WriteFlag(CMOS_REMOTE_LOCK, !bFlag);
	m_bRemoteEnabled = bFlag;
}

void CGUI::SetVideoStandard(uint dwStandard)
{
	m_pDevGraphics->SetVideoStandard(dwStandard);
	UpdateGraphics();
}

void CGUI::UpdateGraphics()
{
	CGuard guard(m_Mutex);
	
	VD_RECT old_rect, new_rect;
	m_pDevGraphics->GetRect(&old_rect);
	//m_pDevGraphics->UpdateSurface();
	m_pDevGraphics->GetRect(&new_rect);
	if(memcmp(&old_rect, &new_rect, sizeof(VD_RECT)))//(memcpy(&old_rect, &new_rect, sizeof(VD_RECT)))
	{
		//printf("%s old/new: l(%d/%d, t(%d/%d), r(%d/%d), b(%d/%d)\n",
		//	__func__, old_rect.left, new_rect.left, old_rect.top, new_rect.top, old_rect.right, new_rect.right, old_rect.bottom, new_rect.bottom);
		m_pDevMouse->SetRect(&new_rect);
		memcpy(&sNewMouseRect, &new_rect, sizeof(VD_RECT));
		CItem::AdjustRectAll();
	}
}

//-----------------------------------------------------
// preview special
//-----------------------------------------------------
// screen preview mode

