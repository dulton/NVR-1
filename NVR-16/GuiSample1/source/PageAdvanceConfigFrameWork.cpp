//#include "GUI/Pages/PageRecordFrameWork.h"
//#include "GUI/Pages/PageSensorAlarmFrameWork.h"
#include "GUI/Pages/PageAdvanceConfigFrameWork.h"
//#include "GUI/Pages/PageAdvanceWarning.h"
#include "GUI/Pages/PageImportExportFrameWork.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageConfigFrameWork.h"
#include "GUI/Pages/PageResumeDefaultCfg.h"
#include "Biz.h"

//#include "GUI/Pages/PageCfgRecParam.h"

//static	MAINFRAME_ITEM	items[MAX_MAIN_ITEM];

#define DROP_BLACKWHITELIST

enum{//与下面的数组一一对应
	ADVANCEGFG_BUTTON_FACTORYRESET,
	ADVANCEGFG_BUTTON_IMPORT_EXPORT,
	#ifndef DROP_BLACKWHITELIST
	ADVANCEGFG_BUTTON_BLACKLIST,
	#endif
	ADVANCEGFG_BUTTON_MAINTAIN,
	ADVANCEGFG_BUTTON_EXIT,	
	ADVANCEGFG_BUTTON_NUM,
};

char* advanceCfgShortcut[ADVANCEGFG_BUTTON_NUM] = {
	"&CfgPtn.SystemReset",
	"&CfgPtn.ImportExport",	
	#ifndef DROP_BLACKWHITELIST
	"&CfgPtn.BlackWhiteList",
	#endif
	"&CfgPtn.MainTain",	
	"&CfgPtn.Exit",
};

char* advanceCfgShortcutBmpName[ADVANCEGFG_BUTTON_NUM+1][2] = {    
	{DATA_DIR"/temp/icon_cfg_reset.bmp",	DATA_DIR"/temp/icon_cfg_reset_f.bmp"},
	{DATA_DIR"/temp/icon_cfg_man.bmp",	DATA_DIR"/temp/icon_cfg_man_f.bmp"},	
	
	#ifndef DROP_BLACKWHITELIST
	{DATA_DIR"/temp/icon_cfg_blacklist.bmp",	DATA_DIR"/temp/icon_cfg_blacklist_f.bmp"},
	#endif

	{DATA_DIR"/temp/icon_cfg_maintain.bmp",	DATA_DIR"/temp/icon_cfg_maintain_f.bmp"},
	{DATA_DIR"/temp/icon_exit.bmp",	DATA_DIR"/temp/icon_exit_f.bmp"},
	{DATA_DIR"/temp/main_pic.bmp",	DATA_DIR"/temp/main_pic.bmp"}, //此项一定要保持在最后一项	
};

//csp modify
//static VD_BITMAP* pBmpButtonNormal[ADVANCEGFG_BUTTON_NUM];
//static VD_BITMAP* pBmpButtonSelect[ADVANCEGFG_BUTTON_NUM];
static VD_BITMAP* pBmpButtonNormal[ADVANCEGFG_BUTTON_NUM+1];
static VD_BITMAP* pBmpButtonSelect[ADVANCEGFG_BUTTON_NUM+1];

CPageAdvanceConfigFrameWork::CPageAdvanceConfigFrameWork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
, curMainItemSel(0)
{
	int rd = 10;
	int space = 78;
	CRect rtTmp;
	rtTmp.left = m_Rect.Width()*1/3;
	rtTmp.top = 55 - 20;
	
	int j;
	
	//debug_trace_err("CPageImportExportFrameWork");
	
	for(j=0; j<ADVANCEGFG_BUTTON_NUM; j++)
	{
		pBmpButtonNormal[j] = VD_LoadBitmap(advanceCfgShortcutBmpName[j][0]);
		pBmpButtonSelect[j] = VD_LoadBitmap(advanceCfgShortcutBmpName[j][1]);
		pButton[j] = CreateButton(CRect(rtTmp.left, rtTmp.top , 
						rtTmp.left+pBmpButtonNormal[j]->width, rtTmp.top+pBmpButtonNormal[j]->height), 
						this, NULL, (CTRLPROC)&CPageAdvanceConfigFrameWork::OnClkAdvanceCfgItem, NULL, buttonNormalBmp);
		
		pButton[j]->SetBitmap(pBmpButtonNormal[j], pBmpButtonSelect[j], pBmpButtonSelect[j]);
		
		pStatic[j] = CreateStatic(CRect(rtTmp.left+pBmpButtonNormal[j]->width+15, rtTmp.top+8, 
						rtTmp.left+pBmpButtonNormal[j]->width+200, rtTmp.top+30), this, advanceCfgShortcut[j]);
		
		pStatic[j]->SetTextAlign(VD_TA_LEFT);
        //pStatic[j]->SetBkColor(VD_RGB(67,77,87));
		
		rtTmp.top += pBmpButtonNormal[j]->height + 25;
		
		//printf("buttonIndex = %d \n",buttonIndex);
	}
	
#if 0//#if 1//csp modify
	//Modify by Lirl on Feb/22/2012,删除显示H.264 DVR图片
	//Modify by Lirl on Jan/12/2012
	//添加显示右下角的图片,图片占用两个控件所占用的空间
	//图片大小:195*50//194*34
	pBmpButtonNormal[j] = VD_LoadBitmap(advanceCfgShortcutBmpName[j][0]);
	rtTmp.top = 310;
	rtTmp.top += 12;//csp modify
	rtTmp.left = 205+104+space+pBmpButtonNormal[ADVANCEGFG_BUTTON_NUM-1]->width;
	rtTmp.left += 4;//csp modify
	rtTmp.right = rtTmp.left + pBmpButtonNormal[ADVANCEGFG_BUTTON_NUM]->width;
	rtTmp.bottom = rtTmp.top + pBmpButtonNormal[ADVANCEGFG_BUTTON_NUM]->height;
	pStaticPic = CreateStatic(rtTmp, this, "");
	//pStaticPic->SetBkColor(VD_RGB(67,77,87));
	pStaticPic->SetBitmap(VD_LoadBitmap(advanceCfgShortcutBmpName[ADVANCEGFG_BUTTON_NUM][0]));
#endif
    
    pInfoBar = CreateStatic(CRect(m_Rect.Width()/2 - (200+50), 
								rtTmp.top,//-20, 
								m_Rect.Width()/2 + (180+50), 
								rtTmp.top+22), 
								this, 
								"");
    //pInfoBar->SetBkColor(VD_RGB(67,77,87));
    pInfoBar->SetTextAlign(VD_TA_CENTER);
    //Modify by Lirl on Jan/16/2012,set the font color to red
    pInfoBar->SetTextColor(VD_RGB(255,0,0));
	
	//debug_trace_err("CPageImportExportFrameWork");
	
	CRect warnRt = CRect(0, 0, 460, 260/*370*/);
	//m_pPageAdvanceWarn = new CPageAdvanceWarning(warnRt, "  Warning"/*"&titles.mainmenu"*/, icon_dvr, NULL/*this*/);
	
	m_pPageImportExport = new CPageImportExportFrameWork(NULL, "&CfgPtn.ImportExport"/*"&titles.mainmenu"*/, icon_dvr, NULL/*this*/);
	//CRect recordRt = CRect(0, 0, 700, 400/*370*/);
	//m_pPageRecord = new CPageRecordFrameWork(NULL/*&recordRt*/, "  Record Config"/*"&titles.mainmenu"*/, icon_dvr, NULL/*this*/);
	//m_pPageRecordSchedule = new CPageRecordScheduleFrameWork(NULL/*&recordRt*/, "  Record Schedule"/*"&titles.mainmenu"*/, icon_dvr, NULL/*this*/);
	//m_pPageSensorAlarm = new CPageSensorAlarmFrameWork(NULL/*&recordRt*/, "  Sensor Alarm"/*"&titles.mainmenu"*/, icon_dvr, NULL/*this*/);
	
	//debug_trace_err("CPageImportExportFrameWork");
	
	m_pPageMainTain = new CPageMainTain(CRect(0, 0, 480, 280), "&CfgPtn.MainTain"/*"&titles.mainmenu"*/, icon_dvr, this);
	
	//m_pPageResumeDefaultCfg = new CPageResumeDefaultCfg(NULL, "&CfgPtn.ResumeDefaultCfg"/*"&titles.mainmenu"*/, icon_dvr, NULL/*this*/);					
}

CPageAdvanceConfigFrameWork::~CPageAdvanceConfigFrameWork()
{
	
}

VD_PCSTR CPageAdvanceConfigFrameWork::GetDefualtTitleCenter()
{
	return "&CfgPtn.Basic";
}

VD_BOOL CPageAdvanceConfigFrameWork::MsgProc( uint msg, uint wpa, uint lpa ) //cw_lock
{	
     u8 lock_flag = 0;  //cw_lock
     GetSystemLockStatus(&lock_flag);
     if(lock_flag)
     {
     	return FALSE;
     }
     return CPage::MsgProc(msg, wpa, lpa);

}
void CPageAdvanceConfigFrameWork::OnClkAdvanceCfgItem()
{
	//printf("OnClkAlarmCfgItem\n");
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < ADVANCEGFG_BUTTON_NUM; i++)
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
			case ADVANCEGFG_BUTTON_FACTORYRESET:
			{
				printf("ADVANCEGFG_BUTTON_FACTORYRESET\n");
				
				u32 g_nUserId = GetGUILoginedUserID();
				EMBIZUSERCHECKAUTHOR author = BizUserCheckAuthority(EM_BIZ_USER_AUTHOR_SYSTEM_RESET,NULL,g_nUserId);
				if(author == EM_BIZ_USER_AUTHOR_YES)
				{
				#if 0
					m_pPageResumeDefaultCfg->Open();
				#else
					UDM ret= MessageBox("&CfgPtn.ResetallsettingSystemwillreboot", "&CfgPtn.WARNING", MB_OKCANCEL|MB_ICONWARNING|MB_DEFBUTTON2);
					if(GetRunStatue() == UDM_OK)
					{
                        pInfoBar->SetText(GetParsedString("&CfgPtn.SysResetNow"));
						if( 0 == BizConfigDefault() )//恢复默认设置
						{
							SetSystemLockStatus(1);//cw_lock
							pInfoBar->SetText(GetParsedString("&CfgPtn.SysRestartNow"));
							BizSysComplexExit(EM_BIZSYSEXIT_RESTART);
						}
						else
						{
							MessageBox("&CfgPtn.ResumeDefaulyParaFail", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
						}
					}
					else if(GetRunStatue() == UDM_CANCEL)
					{
                        ReflushItemName();
						printf(" cancel \n");
					}
				#endif
				}
				else if(author == EM_BIZ_USER_AUTHOR_NO)
				{
					MessageBox("&CfgPtn.NoAuthority", "&CfgPtn.WARNING"  , MB_OK|MB_ICONWARNING);
				}
			}break;
			case ADVANCEGFG_BUTTON_IMPORT_EXPORT:
			{
				m_pPageImportExport->Open();
			}break;
#ifndef DROP_BLACKWHITELIST
			case ADVANCEGFG_BUTTON_BLACKLIST:
			{
				printf("ADVANCEGFG_BUTTON_BLACKLIST \n");
			}break;
#endif
			case ADVANCEGFG_BUTTON_MAINTAIN:
			{
				printf("ADVANCEGFG_BUTTON_MAINTAIN \n");
				m_pPageMainTain->Open();
				
			}break;
			case ADVANCEGFG_BUTTON_EXIT:
			{
                ((CPageConfigFrameWork *)(this->GetParent()))->ReflushItemName();
				this->Close();
			}break;
			default:
			break;
		}
		
	}
	
}

//add by Lirl on Nov/18/2011
void CPageAdvanceConfigFrameWork::ReflushItemName()
{
    for(int j=0; j<ADVANCEGFG_BUTTON_NUM; j++)
	{
		pStatic[j]->SetText(pStatic[j]->GetText());
	}
}

VD_BOOL CPageAdvanceConfigFrameWork::UpdateData( UDM mode )
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

