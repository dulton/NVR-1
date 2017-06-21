#include "GUI/Pages/PageInfoFrameWork.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/PageSysInfo.h"
#include "GUI/Pages/PageEventList.h"
#include "GUI/Pages/PageNetwork.h"
#include "GUI/Pages/PageOnlineUser.h"
#include "GUI/Pages/PageSysLog.h"
#include "biz.h"
//add by liu
#include "GUI/Pages/PageStream.h"


enum INFO_ITEMS
{
	SYS_INFO,
	//EVENT_INFO,
	LOG_INFO,
	NET_INFO,
	//USERONLINE_INFO,
	//add by liu
	STREAM_INFO,
	EXIT_INFO,
};

static char* szShortcut[MAX_INFO_PAGES] = {
	"&CfgPtn.System",
	//"&CfgPtn.Event",	
	"&CfgPtn.Log",
	"&CfgPtn.Network",
	//"&CfgPtn.OnlineUsers",
	//add by liu
	"&CfgPtn.Stream",
	"&CfgPtn.Exit",
};

static char* szBmpName[MAX_INFO_PAGES+1][2] = {    
	{DATA_DIR"/temp/system.bmp",	DATA_DIR"/temp/system_f.bmp"},
	//{DATA_DIR"/temp/event.bmp",	DATA_DIR"/temp/event_f.bmp"},	
	{DATA_DIR"/temp/log.bmp",	DATA_DIR"/temp/log_f.bmp"},
	{DATA_DIR"/temp/network.bmp",	DATA_DIR"/temp/network_f.bmp"},
	//{DATA_DIR"/temp/online_user.bmp",	DATA_DIR"/temp/online_user_f.bmp"},
	//add by liu
	{DATA_DIR"/temp/info.bmp",	DATA_DIR"/temp/info_f.bmp"},
	{DATA_DIR"/temp/exit3.bmp",	DATA_DIR"/temp/exit3_f.bmp"},
    {DATA_DIR"/temp/main_pic.bmp",	DATA_DIR"/temp/main_pic.bmp"}, //此项一定要保持在最后一项
};

//csp modify
//static VD_BITMAP* pBmpButtonNormal[MAX_INFO_PAGES];
//static VD_BITMAP* pBmpButtonSelect[MAX_INFO_PAGES];
static VD_BITMAP* pBmpButtonNormal[MAX_INFO_PAGES+1];
static VD_BITMAP* pBmpButtonSelect[MAX_INFO_PAGES+1];

CPageInfoFrameWork::CPageInfoFrameWork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPageFrame(pRect, psz, icon, pParent)
{
	SetMargin(0,0,0,0);//会影响子控件的起始坐标//csp modify
	
	int i = 0;
    CRect rt;
	for(i=0; i< MAX_INFO_PAGES; i++)
	{
		pBmpButtonNormal[i] = VD_LoadBitmap(szBmpName[i][0]);
		pBmpButtonSelect[i] = VD_LoadBitmap(szBmpName[i][1]);
		
		rt.left = 51 + (i%4)*(72+78);
		rt.right = rt.left +72;
		rt.top = 62 + (i/4)*(72+48);
		rt.bottom = rt.top + 72;
		pButton[i] = CreateButton(rt, this, NULL, (CTRLPROC)&CPageInfoFrameWork::OnClkItem, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
		
		rt.left -= 38;
		rt.right += 38;
		rt.top = 134 + (i/4)*(72+48);
		rt.bottom = rt.top + 30;//72+48;
		pStatic[i] = CreateStatic(rt, this, szShortcut[i]);
		pStatic[i]->SetTextAlign(VD_TA_TOP|VD_TA_XCENTER);
        //pStatic[i]->SetBkColor(VD_RGB(67,77,87));
	}
//yaogang modify 20141106 删除显示H.264 DVR图片	
#if 0//csp modify
	//Modify by Lirl on Feb/22/2012,删除显示H.264 DVR图片
	//Modify by Lirl on Jan/12/2012
	//添加显示右下角的图片,图片占用两个控件所占用的空间
	//图片大小:195*50//194*34
	pBmpButtonNormal[i] = VD_LoadBitmap(szBmpName[i][0]);
	rt.top = 322+m_TitleHeight;//352;//csp modify
	rt.left = 205+72+84+pBmpButtonNormal[MAX_INFO_PAGES-1]->width;
	rt.right = rt.left + pBmpButtonNormal[MAX_INFO_PAGES]->width;
	rt.bottom = rt.top + pBmpButtonNormal[MAX_INFO_PAGES]->height;
	printf("CPageInfoFrameWork H264DVR pos:(%d,%d,%d,%d),parent pos:(%d,%d,%d,%d),m_TitleHeight:%d\n",
		rt.left,rt.right,rt.top,rt.bottom,
		m_Rect.left,m_Rect.right,m_Rect.top,m_Rect.bottom,
		m_TitleHeight);
	pStaticPic = CreateStatic(&rt, this, "");
	//pStaticPic->SetBkColor(VD_RGB(67,77,87));
	pStaticPic->SetBitmap(VD_LoadBitmap(szBmpName[MAX_INFO_PAGES][0]));
#endif
	
	m_pPageSysInfo = new CPageSysInfo(CRect(0, 0, 642, 418), "&CfgPtn.System", SIT_SYS, icon_dvr, this);
	//m_pPageEventList = new CPageEventList(CRect(0, 0, 642, 418), "&CfgPtn.Event",icon_dvr, this);
	m_pPageNetwork = new CPageNetwork(CRect(0, 0, 642, 418), "&CfgPtn.Network",icon_dvr, this);
	//m_pPageOnline = new CPageOnlineUser(CRect(0, 0, 642, 418), "&CfgPtn.OnlineUsers",icon_dvr, this);
	m_pPageSysLog = new CPageSysLog(CRect(0, 0, 642, 418), "&CfgPtn.LOGLIST",icon_dvr, this);
	//add by liu
	m_pPageStream = new CPageStream(CRect(0, 0, 642, 418), "&CfgPtn.Stream",icon_dvr, this);
}

CPageInfoFrameWork::~CPageInfoFrameWork()
{
	
}

void CPageInfoFrameWork::OnClkItem()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < MAX_INFO_PAGES; i++)
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
		case SYS_INFO:
			{
				m_pPageSysInfo->Open();
			}
			break;
		//case EVENT_INFO:
			{
		//		m_pPageEventList->Open();
			}
			break;
		case LOG_INFO:
			{
				u32 g_nUserId = GetGUILoginedUserID();
				EMBIZUSERCHECKAUTHOR author = BizUserCheckAuthority(EM_BIZ_USER_AUTHOR_LOG_VIEW,NULL,g_nUserId);
				if(author == EM_BIZ_USER_AUTHOR_NO)
				{
					MessageBox("&CfgPtn.NoAuthority", "&CfgPtn.WARNING"  , MB_OK|MB_ICONWARNING);
				}
				else //if(author == EM_BIZ_USER_AUTHOR_YES)
				{
					m_pPageSysLog->Open();
					printf("PAGE LOGS\n");
				}				
			}
			break;	
		case NET_INFO:
			{
				m_pPageNetwork->Open();
			}
			break;
		#if 0
		case USERONLINE_INFO:
			{
				m_pPageOnline->Open();
			}
			break;
		#endif
		//add by liu
		case STREAM_INFO:
			{
				
				m_pPageStream->Open();
			}
		break;
		case EXIT_INFO:
			{
				this->Close();
			}
		break;
		default:
			break;
		}
	}
}

//add by Lirl on Nov/18/2011
void CPageInfoFrameWork::ReflushItemName()
{
    for(int i=0; i< MAX_INFO_PAGES; i++)
	{
		pStatic[i]->SetText(pStatic[i]->GetText());
	}
}

VD_BOOL CPageInfoFrameWork::UpdateData( UDM mode )
{
	if (UDM_OPEN==mode || UDM_OPENED==mode)
	{
        ReflushItemName();
	}
	else if (UDM_CLOSED == mode)
	{
        ((CPageMainFrameWork *)(this->GetParent()))->ReflushItemName();
    }
	
    return TRUE;
}
//end

