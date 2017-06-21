#include "GUI/Pages/PageIPCameraConfigFrameWork.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/BizData.h"
#include "biz.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageInfoBox.h"
#include "GUI/Pages/PageConfigFrameWork.h"

#include "GUI/Pages/PageDesktop.h"

#define LEFT_PAGE_L			5
#define LEFT_PAGE_R			(m_Rect.Width()-8)//(245)
#define RIGHT_PAGE_L		5//(243)
#define RIGHT_PAGE_R		(m_Rect.Width()-8)

#define RIGHT_PAGE_WIDTH	(RIGHT_PAGE_R-RIGHT_PAGE_L)
#define LEFT_PAGE_WIDTH		(LEFT_PAGE_R-LEFT_PAGE_L)

static	std::vector<CItem*>	items[IPCAMERACFG_SUBPAGES+1];

static VD_BITMAP * pBmp_button_normal = NULL;
static VD_BITMAP * pBmp_button_push = NULL;
static VD_BITMAP * pBmp_button_select = NULL;

static VD_BITMAP * pBmp_tab_normal = NULL;
static VD_BITMAP * pBmp_tab_push = NULL;
static VD_BITMAP * pBmp_tab_select = NULL;

static VD_BITMAP * pBmp_infoicon = NULL;

static int GetMaxChnNum()
{
	return GetVideoMainNum();
}

static int GetIPCCount(ipc_node* head)
{
	int count = 0;
	ipc_node *p = head;
	
	while(p)
	{
		count++;
		p = p->next;
	}
	
	return count;
}

CPageIPCameraConfigFrameWork::CPageIPCameraConfigFrameWork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
{
	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);
	
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	
	memset(pTabFrame0, 0, sizeof(pTabFrame0));
	memset(pTable0, 0, sizeof(pTable0));
	
	memset(pStatic0, 0, sizeof(pStatic0));
	memset(pCombox0, 0, sizeof(pCombox0));
	memset(pCheckBox0, 0, sizeof(pCheckBox0));
	memset(pIpCtrl0, 0, sizeof(pIpCtrl0));
	memset(pEdit0, 0, sizeof(pEdit0));
	memset(pButton0, 0, sizeof(pButton0));
	memset(pScrollbar0, 0, sizeof(pScrollbar0));
	
	m_rtTable1 = CRect(0, 0, 0, 0);
	m_nCurMouseLine = -1;
	
	m_cols = 0;
	m_rows = 0;
	m_offset = 0;
	
	m_ipc_head = NULL;
	m_nPageIndex = 0;
	
	InitPage0();
	
	m_pPageIPCModify = new CPageIPCameraModifyFrameWork(NULL, "&CfgPtn.IPCamera", icon_dvr, this);
}

CPageIPCameraConfigFrameWork::~CPageIPCameraConfigFrameWork()
{
	
}

VD_PCSTR CPageIPCameraConfigFrameWork::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

void CPageIPCameraConfigFrameWork::InitPage0()
{
	const char* statictext[] = 
	{
		"&CfgPtn.Channel",
		"&CfgPtn.Enable",
		"&CfgPtn.Protocol",
		"&CfgPtn.TransMode",
		"&CfgPtn.IPAddress",
		"&CfgPtn.Port",
		"&CfgPtn.Username",
		"&CfgPtn.Password"
	};
	
	CRect tmpRt;
	
	int count = sizeof(statictext)/sizeof(statictext[0]);
	//printf("CPageIPCameraConfigFrameWork::InitPage0-count=%d\n",count);
	
	CRect rtFrame0(LEFT_PAGE_L, 10, LEFT_PAGE_R, 140);
	pTabFrame0[0] = CreateTableBox(rtFrame0,this,1,1);
	pTabFrame0[0]->SetFrameColor(VD_RGB(56,108,170));
	
	CRect rtFrame1(RIGHT_PAGE_L, 138, RIGHT_PAGE_R, 330);
	pTabFrame0[1] = CreateTableBox(rtFrame1,this,1,1);
	pTabFrame0[1]->SetFrameColor(VD_RGB(56,108,170));
	
	CRect rtFrame2(RIGHT_PAGE_L, 328, RIGHT_PAGE_R, 366);
	pTabFrame0[2] = CreateTableBox(rtFrame2,this,1,1);
	pTabFrame0[2]->SetFrameColor(VD_RGB(56,108,170));
	
	CRect rtTable0(rtFrame0.left+4,rtFrame0.top+5,rtFrame0.right-4,rtFrame0.bottom-5);
	pTable0[0] = CreateTableBox(rtTable0,this,4,4);
	pTable0[0]->SetColWidth(0,84);
	pTable0[0]->SetColWidth(1,244);
	pTable0[0]->SetColWidth(2,84);
	
	int i = 0;
	for(i = 0; i < count; i++)
	{
		if(i&1)
		{
			pTable0[0]->GetTableRect(2, i/2, &tmpRt);
		}
		else
		{
			pTable0[0]->GetTableRect(0, i/2, &tmpRt);
		}
		pStatic0[i] = CreateStatic(CRect(rtTable0.left+tmpRt.left, rtTable0.top+tmpRt.top, 
									rtTable0.left+tmpRt.right, rtTable0.top+tmpRt.bottom), 
									this, statictext[i]);
		pStatic0[i]->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
		//pStatic0[i]->SetBkColor(VD_RGB(67,77,87));
	}
	
	pTable0[0]->GetTableRect(1, 0, &tmpRt);
	pCombox0[0] = CreateComboBox(CRect(rtTable0.left+tmpRt.left+1, rtTable0.top+tmpRt.top+1, 
								rtTable0.left+tmpRt.right-1, rtTable0.top+tmpRt.bottom-1), this, NULL, NULL, (CTRLPROC)&CPageIPCameraConfigFrameWork::OnComboBox0, 0);
	int j = 0;
	for(j = 0; j < GetMaxChnNum(); j++)
	{
		char chninfo[4];
		sprintf(chninfo,"%d",j+1);
		pCombox0[0]->AddString(chninfo);
		pCombox0[0]->SetCurSel(0);
	}
	
	pTable0[0]->GetTableRect(3, 0, &tmpRt);
	pCheckBox0[0] = CreateCheckBox(CRect(rtTable0.left+tmpRt.left+0, rtTable0.top+tmpRt.top+2, 
									rtTable0.left+tmpRt.left+20, rtTable0.top+tmpRt.top+22), this);
	
	pTable0[0]->GetTableRect(1, 1, &tmpRt);
	pCombox0[1] = CreateComboBox(CRect(rtTable0.left+tmpRt.left+1, rtTable0.top+tmpRt.top+1, 
								rtTable0.left+tmpRt.right-1, rtTable0.top+tmpRt.bottom-1), this, NULL, NULL, NULL, 0);
	pCombox0[1]->AddString("ONVIF");
	pCombox0[1]->AddString("I13");
	pCombox0[1]->SetCurSel(0);
	
	pTable0[0]->GetTableRect(3, 1, &tmpRt);
	pCombox0[2] = CreateComboBox(CRect(rtTable0.left+tmpRt.left+1, rtTable0.top+tmpRt.top+1, 
								rtTable0.left+tmpRt.right-1, rtTable0.top+tmpRt.bottom-1), this, NULL, NULL, NULL, 0);
	pCombox0[2]->AddString("TCP");
	pCombox0[2]->AddString("UDP");
	pCombox0[2]->SetCurSel(0);
	
	pTable0[0]->GetTableRect(1, 2, &tmpRt);
	pIpCtrl0[0] = CreateIPAddressCtrl(CRect(rtTable0.left+tmpRt.left+1, rtTable0.top+tmpRt.top+1, 
									rtTable0.left+tmpRt.right-1, rtTable0.top+tmpRt.bottom-1), this);
	IPADDR sIPIns;
	sIPIns.l = inet_addr("192.168.1.88");
	pIpCtrl0[0]->SetIPAddress(&sIPIns);
	
	for(i = 0; i < 3; i++)
	{
		if(i&1)
		{
			pTable0[0]->GetTableRect(1, (5+i)/2, &tmpRt);
		}
		else
		{
			pTable0[0]->GetTableRect(3, (5+i)/2, &tmpRt);
		}
		if(i == 0)
		{
			pEdit0[i] = CreateEdit(CRect(rtTable0.left+tmpRt.left+1, rtTable0.top+tmpRt.top+1, 
								rtTable0.left+tmpRt.right-1, rtTable0.top+tmpRt.bottom-1), this, 16, 
								edit_KI_NUMERIC, NULL);
		}
		else if(i == 2)
		{
			pEdit0[i] = CreateEdit(CRect(rtTable0.left+tmpRt.left+1, rtTable0.top+tmpRt.top+1, 
								rtTable0.left+tmpRt.right-1, rtTable0.top+tmpRt.bottom-1), this, 16, 
								editPassword, NULL);
		}
		else
		{
			pEdit0[i] = CreateEdit(CRect(rtTable0.left+tmpRt.left+1, rtTable0.top+tmpRt.top+1, 
								rtTable0.left+tmpRt.right-1, rtTable0.top+tmpRt.bottom-1), this, 16, 
								0, NULL);
		}
	}
	
	pEdit0[0]->SetText("80");
	pEdit0[1]->SetText("admin");
	pEdit0[2]->SetText("admin");
	
	const char* szTitle[] = 
	{
		"&CfgPtn.ID",
		"&CfgPtn.IPAddress",
		"&CfgPtn.Port",
		"&CfgPtn.Protocol",
		"&CfgPtn.DeviceName",
		"&CfgPtn.More"
	};
	
	m_cols = sizeof(szTitle)/sizeof(szTitle[0]);
	m_rows = 7;
	m_offset = count+m_cols;
	
	//CRect rtTable1(rtFrame1.left+4,rtFrame1.top+5,rtFrame1.right-4,rtFrame1.bottom-5);
	CRect rtTable1(rtFrame1.left+4,rtFrame1.top+5,rtFrame1.right-24,rtFrame1.bottom-5);
	pTable0[1] = CreateTableBox(rtTable1, this, m_cols, m_rows);
	pTable0[1]->SetColWidth(0, 52);
	pTable0[1]->SetColWidth(1, 132);
	pTable0[1]->SetColWidth(2, 64);
	pTable0[1]->SetColWidth(3, 80);
	pTable0[1]->SetColWidth(4, 132);
	
	m_rtTable1 = rtTable1;
	
	for(i = 0; i < m_cols; i++)
	{
		pTable0[1]->GetTableRect(i, 0, &tmpRt);
		
		pStatic0[count+i] = CreateStatic(CRect(rtTable1.left+tmpRt.left, rtTable1.top+tmpRt.top, 
									rtTable1.left+tmpRt.right, rtTable1.top+tmpRt.bottom), 
									this, szTitle[i]);
		pStatic0[count+i]->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
		pStatic0[count+i]->SetBkColor(VD_RGB(67,77,87));
	}
	
	for(j = 1; j < m_rows; j++)
	{
		for(i = 0; i < m_cols; i++)
		{
			pTable0[1]->GetTableRect(i, j, &tmpRt);
			
			pStatic0[count+j*m_cols+i] = CreateStatic(CRect(rtTable1.left+tmpRt.left, rtTable1.top+tmpRt.top, 
											rtTable1.left+tmpRt.right, rtTable1.top+tmpRt.bottom), 
											this, NULL, TRUE, 0, FS_NORMAL);
			pStatic0[count+j*m_cols+i]->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
		}
	}

	//csp modify
	pScrollbar0[0] = CreateScrollBar(CRect(m_rtTable1.right,m_rtTable1.top,m_rtTable1.right+20,m_rtTable1.bottom),
                                    this,
                                    scrollbarY,
								    0,
								    0,
								    1,
								    (CTRLPROC)&CPageIPCameraConfigFrameWork::OnTrackMove0);
	
	#if 0
	const char* szDevInfo[] = 
	{
		"1",
		"208.238.238.238",
		"58888",
		"ONVIF",
		"JMV IPCamera",
		"JMV IPCamera"
	};
	
	for(i = 0; i < m_cols; i++)
	{
		pStatic0[count+m_cols+i]->SetText(szDevInfo[i]);
	}
	#endif
	
	CRect rt(rtFrame2.left+2, rtFrame2.top+4, rtFrame2.left+2+68, rtFrame2.bottom-4);
	pStatic0[count+m_rows*m_cols] = CreateStatic(rt, this, NULL, TRUE, 0, FS_NORMAL);
	pStatic0[count+m_rows*m_cols]->SetText("&CfgPtn.Protocol");
	
	rt.left = rt.right+4;
	rt.right = rt.left+80;
	pCombox0[3] = CreateComboBox(rt, this, NULL, NULL, NULL, 0);
	pCombox0[3]->AddString("&CfgPtn.All");
	pCombox0[3]->AddString("ONVIF");
	pCombox0[3]->AddString("I13");
	pCombox0[3]->SetCurSel(0);
	
	rt.left = rt.right+4;
	rt.right = rt.left+76;
	pButton0[0] = CreateButton(rt, this, "&CfgPtn.Search", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[0]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	
	rt.left = rt.right+4;
	rt.right = rt.left+76;
	pButton0[1] = CreateButton(rt, this, "&CfgPtn.Modify", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[1]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	
	#if 1
	rt.left = rt.right+4;
	rt.right = rt.left+152;
	pButton0[2] = CreateButton(rt, this, "&CfgPtn.AutoAssign", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[2]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	#else
	const char* szBtBmpPath[4][2] = 
	{
		{DATA_DIR"/temp/listex_btn_leftmost.bmp", DATA_DIR"/temp/listex_btn_leftmost_f.bmp"},
		{DATA_DIR"/temp/listex_btn_left.bmp", DATA_DIR"/temp/listex_btn_left_f.bmp"},
		{DATA_DIR"/temp/listex_btn_right.bmp", DATA_DIR"/temp/listex_right_f.bmp"},
		{DATA_DIR"/temp/listex_btn_rightmost.bmp", DATA_DIR"/temp/listex_btn_rightmost_f.bmp"},
	};
	
	rt.top += 3;
	rt.bottom = rt.top + 25;
	
	rt.left = rt.right+4;
	rt.right = rt.left+40;
	
	for(i = 0; i < 4; i++)
	{
		//pButton0[2+i] = CreateButton(rt, this, NULL, NULL, NULL, buttonNormalBmp);
		//pButton0[2+i]->SetBitmap(VD_LoadBitmap(szBtBmpPath[i][0]), VD_LoadBitmap(szBtBmpPath[i][1]), VD_LoadBitmap(szBtBmpPath[i][1]));
		
		rt.left = rt.right+2;
		rt.right = rt.left+40;
	}
	#endif
	
	rt.top = rtFrame2.top+4;
	rt.bottom = rtFrame2.bottom-4;
	
	//rt.left = rt.right+4;
	//rt.right = rt.left+58;
	rt.left += (2+12)+(168);
	rt.right = rt.left+60;
	pButton0[6] = CreateButton(rt, this, "&CfgPtn.apply", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[6]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	
	rt.left = rt.right+4;
	rt.right = rt.left+60;
	pButton0[7] = CreateButton(rt, this, "&CfgPtn.Exit", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[7]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
}

void CPageIPCameraConfigFrameWork::OnComboBox0()
{
	SBizIPCameraPara ipcam;
	memset(&ipcam, 0, sizeof(ipcam));
	ipcam.channel_no = pCombox0[0]->GetCurSel();
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
	bizTar.nChn = ipcam.channel_no;
	BizGetPara(&bizTar, (void *)&ipcam);
	
	pCheckBox0[0]->SetValue(ipcam.enable);
	if(ipcam.protocol_type == PRO_TYPE_ONVIF)
	{
		pCombox0[1]->SetCurSel(0);
	}
	else if(ipcam.protocol_type == PRO_TYPE_HAIXIN)
	{
		pCombox0[1]->SetCurSel(1);
	}
	else
	{
		pCombox0[1]->SetCurSel(0);
	}
	pCombox0[2]->SetCurSel((ipcam.trans_type == TRANS_TYPE_TCP)?0:1);
	IPADDR sIPIns;
	sIPIns.l = ipcam.dwIp;
	pIpCtrl0[0]->SetIPAddress(&sIPIns);
	char szInfo[32];
	sprintf(szInfo, "%d", ipcam.wPort);
	pEdit0[0]->SetText(szInfo);
	pEdit0[1]->SetText(ipcam.user);
	pEdit0[2]->SetText(ipcam.pwd);
}

void CPageIPCameraConfigFrameWork::OnTrackMove0()
{
	//printf("CPageIPCameraConfigFrameWork::OnTrackMove0 start\n");
	if(m_ipc_head)
	{
		int nums = GetIPCCount(m_ipc_head);
		int pages = (nums+m_rows-1-1)/(m_rows-1);
		if(pages < 1)
		{
			pages = 1;
		}
		
		int pos = pScrollbar0[0]->GetPos();
		int start_idx = (pos)*(m_rows-1);
		int end_idx = (pos+1)*(m_rows-1);
		//printf("CPageIPCameraConfigFrameWork::OnTrackMove0:start_idx=%d,end_idx=%d,pos=%d,pages=%d\n",start_idx,end_idx,pos,pages);
		
		m_nPageIndex = pos;
		
		for(int j = 0; j < m_rows - 1; j++)
		{
			for(int i = 0; i < m_cols; i++)
			{
				pStatic0[m_offset+j*m_cols+i]->SetText(NULL);
				pStatic0[m_offset+j*m_cols+i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
			}
		}
		m_nCurMouseLine = -1;
		
		int count = 0;
		ipc_node *p = m_ipc_head;
		while(p)
		{
			//printf("CPageIPCameraConfigFrameWork::OnTrackMove0 count=%d\n",count);
			if(count >= start_idx && count < end_idx)
			{
				char szInfo[64];
				sprintf(szInfo, "%d", count+1);
				pStatic0[m_offset+(count-start_idx)*m_cols+0]->SetText(szInfo);
				struct in_addr host;
				host.s_addr = p->ipcam.dwIp;
				strcpy(szInfo, inet_ntoa(host));
				pStatic0[m_offset+(count-start_idx)*m_cols+1]->SetText(szInfo);
				sprintf(szInfo, "%d", p->ipcam.wPort);
				pStatic0[m_offset+(count-start_idx)*m_cols+2]->SetText(szInfo);
				if(p->ipcam.protocol_type == PRO_TYPE_ONVIF)
				{
					strcpy(szInfo, "ONVIF");
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_HAIXIN)
				{
					strcpy(szInfo, "I13");
				}
				else
				{
					memset(szInfo, 0, sizeof(szInfo));
				}
				pStatic0[m_offset+(count-start_idx)*m_cols+3]->SetText(szInfo);
			}
			count++;
			if(count >= end_idx)
			{
				break;
			}
			p = p->next;
		}
	}
}

void CPageIPCameraConfigFrameWork::OnClickSubPage()
{
	//printf("CPageIPCameraConfigFrameWork::OnClickSubPage-1\n");
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < sizeof(pButton0)/sizeof(pButton0[0]); i++)
	{
		if(pFocusButton == pButton0[i] && pFocusButton != NULL)
		{
			bFind = TRUE;
			break;
		}
	}
	
	//printf("CPageIPCameraConfigFrameWork::OnClickSubPage-2\n");
	
	if(!bFind)
	{
		return;
	}
	
	//printf("CPageIPCameraConfigFrameWork::OnClickSubPage-3,i=%d\n",i);
	
	switch(i)
	{
	case 0://搜索
	case 2://自动分配
		{
			unsigned char check_conflict = (i == 2) ? 1 : 0;
			
			int sel = pCombox0[3]->GetCurSel();
			unsigned int pro_type = PRO_TYPE_ONVIF;
			if(sel == 1)
			{
				pro_type = PRO_TYPE_ONVIF;
			}
			else if(sel == 2)
			{
				pro_type = PRO_TYPE_HAIXIN;
			}
			else
			{
				pro_type = (unsigned int)(-1);
			}
			
			if(m_ipc_head)
			{
				IPC_Free(m_ipc_head);
				m_ipc_head = NULL;
			}
			
			for(int j = 0; j < m_rows - 1; j++)
			{
				for(int i = 0; i < m_cols; i++)
				{
					pStatic0[m_offset+j*m_cols+i]->SetText(NULL);
					pStatic0[m_offset+j*m_cols+i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
				}
			}
			m_nCurMouseLine = -1;
			
			int ret = IPC_Search(&m_ipc_head, pro_type, check_conflict);
			if(ret < 0 || m_ipc_head == NULL)
			{
				pScrollbar0[0]->SetRange(0,0,FALSE);
				pScrollbar0[0]->SetPage(1,FALSE);
				pScrollbar0[0]->SetPos(0,TRUE);
				m_nPageIndex = 0;
				return;
			}
			
			int nums = GetIPCCount(m_ipc_head);
			int pages = (nums+m_rows-1-1)/(m_rows-1);
			if(pages < 1)
			{
				pages = 1;
			}
			
			pScrollbar0[0]->SetRange(0,pages-1,FALSE);
			pScrollbar0[0]->SetPage(1,FALSE);
			pScrollbar0[0]->SetPos(0,TRUE);
			
			m_nPageIndex = 0;
			
			//int i = 0;
			//for(i = 0; i < m_cols*(m_rows-1); i++)
			//{
			//	pStatic0[m_offset+i]->SetText(NULL);
			//}
			
			int count = 0;
			ipc_node *p = m_ipc_head;
			while(p)
			{
				char szInfo[64];
				sprintf(szInfo, "%d", count+1);
				pStatic0[m_offset+count*m_cols+0]->SetText(szInfo);
				struct in_addr host;
				host.s_addr = p->ipcam.dwIp;
				strcpy(szInfo, inet_ntoa(host));
				pStatic0[m_offset+count*m_cols+1]->SetText(szInfo);
				sprintf(szInfo, "%d", p->ipcam.wPort);
				pStatic0[m_offset+count*m_cols+2]->SetText(szInfo);
				if(p->ipcam.protocol_type == PRO_TYPE_ONVIF)
				{
					strcpy(szInfo, "ONVIF");
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_HAIXIN)
				{
					strcpy(szInfo, "I13");
				}
				else
				{
					memset(szInfo, 0, sizeof(szInfo));
				}
				pStatic0[m_offset+count*m_cols+3]->SetText(szInfo);
				count++;
				if(count == m_rows-1)
				{
					break;
				}
				p = p->next;
			}
			
			if(i == 2)
			{
				int j = 0;
				for(j = 0; j < GetMaxChnNum(); j++)
				{
					SBizIPCameraPara ipcam;
					memset(&ipcam, 0, sizeof(ipcam));
					
					SBizParaTarget bizTar;
					bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
					bizTar.nChn = j;
					BizGetPara(&bizTar, (void *)&ipcam);
					
					if(ipcam.enable)
					{
						ipcam.channel_no = j;
						ipcam.enable = 0;
						
						BizSetPara(&bizTar, (void *)&ipcam);
					}
				}
				
				count = 0;
				p = m_ipc_head;
				while(p)
				{
					SBizIPCameraPara ipcam;
					memset(&ipcam, 0, sizeof(ipcam));
					
					ipcam.channel_no = count;
					ipcam.enable = 1;//p->ipcam.enable;
					ipcam.protocol_type = p->ipcam.protocol_type;
					ipcam.trans_type = p->ipcam.trans_type;
					ipcam.stream_type = p->ipcam.stream_type;
					ipcam.ipc_type = p->ipcam.ipc_type;
					ipcam.dwIp = p->ipcam.dwIp;
					ipcam.wPort = p->ipcam.wPort;
					strcpy(ipcam.user, p->ipcam.user);
					strcpy(ipcam.pwd, p->ipcam.pwd);
					struct in_addr host;
					host.s_addr = ipcam.dwIp;
					sprintf(ipcam.address, "http://%s:%d/onvif/device_service", inet_ntoa(host), ipcam.wPort);
					
					SBizParaTarget bizTar;
					bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
					bizTar.nChn = ipcam.channel_no;
					BizSetPara(&bizTar, (void *)&ipcam);
					
					count++;
					
					p = p->next;
				}
				
				pCombox0[0]->SetCurSel(0);
				OnComboBox0();
			}
		}
		break;
	case 1://修改网络摄像机网络参数
		{
			//printf("modify ipcamera\n");
			
			//test
			//IPC_CMD_Reboot(0);
			//break;
			
			if(m_nCurMouseLine == -1)
			{
				break;
			}
			
			int idx = 0;
			ipc_node *p = m_ipc_head;
			while(p)
			{
				if(idx == m_nPageIndex*(m_rows-1)+m_nCurMouseLine)
				{
					//if(p->ipcam.protocol_type == PRO_TYPE_ONVIF || p->ipcam.net_mask == 0)
					if(p->ipcam.protocol_type == PRO_TYPE_ONVIF)
					{
						ipc_neteork_para_t nw;
						memset(&nw,0,sizeof(nw));
						if(IPC_GetNetworkParam(&p->ipcam, &nw) == 0)
						{
							p->ipcam.net_mask = nw.net_mask;
							p->ipcam.net_gateway = nw.net_gateway;
							p->ipcam.dns1 = nw.dns1;
							p->ipcam.dns2 = nw.dns2;
						}
					}
					m_pPageIPCModify->SetIPC(&p->ipcam);
					m_pPageIPCModify->Open();
					break;
				}
				idx++;
				p = p->next;
			}
		}
		break;
	case 6://应用
		{
			SBizIPCameraPara ipcam;
			memset(&ipcam, 0, sizeof(ipcam));
			ipcam.channel_no = pCombox0[0]->GetCurSel();
			ipcam.enable = pCheckBox0[0]->GetValue()?1:0;
			if(pCombox0[1]->GetCurSel() == 0)
			{
				ipcam.protocol_type = PRO_TYPE_ONVIF;
			}
			else if(pCombox0[1]->GetCurSel() == 1)
			{
				ipcam.protocol_type = PRO_TYPE_HAIXIN;
			}
			else
			{
				ipcam.protocol_type = PRO_TYPE_ONVIF;
			}
			ipcam.trans_type = pCombox0[2]->GetCurSel()?/*TRANS_TYPE_UDP*/TRANS_TYPE_TCP:TRANS_TYPE_TCP;
			ipcam.stream_type = STREAM_TYPE_MAIN;
			ipcam.ipc_type = IPC_TYPE_720P;
			IPADDR sIPIns;
			pIpCtrl0[0]->GetIPAddress(&sIPIns);
			ipcam.dwIp = sIPIns.l;
			char szInfo[32];
			memset(szInfo, 0, sizeof(szInfo));
			pEdit0[0]->GetText(szInfo, sizeof(szInfo));
			ipcam.wPort = strtol(szInfo, 0, 10);
			pEdit0[1]->GetText(ipcam.user, sizeof(ipcam.user));
			pEdit0[2]->GetText(ipcam.pwd, sizeof(ipcam.pwd));
			struct in_addr host;
			host.s_addr = ipcam.dwIp;
			sprintf(ipcam.address, "http://%s:%d/onvif/device_service", inet_ntoa(host), ipcam.wPort);
			
			SBizParaTarget bizTar;
			bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
			bizTar.nChn = ipcam.channel_no;
			BizSetPara(&bizTar, (void *)&ipcam);
		}
		break;
	case 7://退出
		{
			//printf("exit\n");
			this->Close();
		}
		break;
	default:
		break;
	}
}

void CPageIPCameraConfigFrameWork::SwitchPage(int subID)
{
	
}

void CPageIPCameraConfigFrameWork::ShowSubPage(int subID, BOOL bShow)
{
	
}

VD_BOOL CPageIPCameraConfigFrameWork::UpdateData(UDM mode)
{
	if(UDM_OPEN == mode)
	{
		if(m_ipc_head)
		{
			IPC_Free(m_ipc_head);
			m_ipc_head = NULL;
		}
		
		pScrollbar0[0]->SetRange(0,0,FALSE);
		pScrollbar0[0]->SetPage(1,FALSE);
		pScrollbar0[0]->SetPos(0,TRUE);
		
		m_nPageIndex = 0;
		
		for(int j = 0; j < m_rows - 1; j++)
		{
			for(int i = 0; i < m_cols; i++)
			{
				pStatic0[m_offset+j*m_cols+i]->SetText(NULL);
				pStatic0[m_offset+j*m_cols+i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
			}
		}
		m_nCurMouseLine = -1;
		
		pCombox0[0]->SetCurSel(0);
		OnComboBox0();
	}
	else if(UDM_OPENED == mode)
	{
		
	}
	else if(UDM_CLOSED == mode)
	{
		((CPageConfigFrameWork *)(this->GetParent()))->ReflushItemName();
	}
	return TRUE;
}

VD_BOOL CPageIPCameraConfigFrameWork::MsgProc( uint msg, uint wpa, uint lpa )
{
	int px, py;
	
	u8 lock_flag = 0;
	GetSystemLockStatus(&lock_flag);
	if(lock_flag)
	{
		return FALSE;
	}
	
	switch(msg)
	{
	case XM_LBUTTONDBLCLK:
	//case XM_MOUSEMOVE:
	case XM_LBUTTONDOWN:
		{
			//CItem* pItem = GetFocusItem();
			//printf("GetFocusItem:0x%08x\n",(unsigned int)pItem);
			//if(pItem)
			//{
			//	printf("FocusItem type:%d\n",(unsigned int)pItem->GetType());
			//}
			
			//if(CPage::MsgProc(msg, wpa, lpa))
			if(CPageFrame::MsgProc(msg, wpa, lpa))
			{
				return TRUE;
			}
			
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);
			
			VD_RECT rtScreen;
			m_pDevGraphics->GetRect(&rtScreen);
			int offsetx = (rtScreen.right - m_Rect.Width())/2;
			int offsety = (rtScreen.bottom - m_Rect.Height())/2;
			
			BOOL bFind = FALSE;
			CTableBox* table = pTable0[1];
			int i = 0;
			for(i = 0; i < m_rows-1; i++)
			{
				for(int kk = 0; kk < m_cols; kk++)
				{
					VD_RECT rt;
					table->GetTableRect(kk, i+1, &rt);
					rt.left += m_rtTable1.left+offsetx;
					rt.top += m_TitleHeight+m_rtTable1.top+offsety;
					rt.right += m_rtTable1.left+offsetx;
					rt.bottom += m_TitleHeight+m_rtTable1.top+offsety;
					//printf("i=%d kk=%d, x=%d y=%d, [%d %d %d %d]\n", i, kk, px, py, rt.left, rt.top, rt.right, rt.bottom);
					if(PtInRect(&rt, px, py))
					{
						bFind = TRUE;
						goto SELITEM;
					}
				}
			}
			
SELITEM:
			if(bFind)
			{
				//printf("here-1\n");
				if(XM_LBUTTONDBLCLK == msg)
				{
					//printf("here-2\n");
					int idx = 0;
					ipc_node *p = m_ipc_head;
					while(p)
					{
						//printf("here-3\n");
						if(idx == m_nPageIndex*(m_rows-1)+i)
						{
							IPADDR sIPIns;
							sIPIns.l = p->ipcam.dwIp;
							pIpCtrl0[0]->SetIPAddress(&sIPIns);
							char szInfo[32];
							sprintf(szInfo, "%d", p->ipcam.wPort);
							pEdit0[0]->SetText(szInfo);
							if(p->ipcam.protocol_type == PRO_TYPE_ONVIF)
							{
								pCombox0[1]->SetCurSel(0);
							}
							else if(p->ipcam.protocol_type == PRO_TYPE_HAIXIN)
							{
								pCombox0[1]->SetCurSel(1);
							}
							else
							{
								pCombox0[1]->SetCurSel(0);
							}
							break;
						}
						idx++;
						p = p->next;
					}
					MouseMoveToLine(i);
				}
				else
				{
					MouseMoveToLine(i);
				}
			}
			else
			{
				if(XM_MOUSEMOVE == msg)
				{
					MouseMoveToLine(-1);
				}
			}
			
			return TRUE;
		}
		break;
	}
	
	//return CPage::MsgProc(msg, wpa, lpa);
	return CPageFrame::MsgProc(msg, wpa, lpa);
}

VD_BOOL CPageIPCameraConfigFrameWork::Open()
{
	VD_BOOL ret = CPage::Open();
	return ret;
}

void CPageIPCameraConfigFrameWork::MouseMoveToLine(int index)
{
	if(index == m_nCurMouseLine)
	{
		return;
	}
	
	if((m_nCurMouseLine >= 0) && (strcmp(pStatic0[m_offset+m_nCurMouseLine*m_cols]->GetText(),"") != 0))
	{
		for(int i = 0; i < m_cols; i++)
		{
			pStatic0[m_offset+m_nCurMouseLine*m_cols+i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
		}
	}
	
	m_nCurMouseLine = -1;
	
	if(index < 0 || index >= m_rows-1)
	{
		return;
	}
	
	m_nCurMouseLine = index;
	
	if(strcmp(pStatic0[m_offset+index*m_cols]->GetText(),"") == 0)
	{
		return;
	}
	
	for(int i = 0; i < m_cols; i++)
	{
		pStatic0[m_offset+m_nCurMouseLine*m_cols+i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
	}
}

void CPageIPCameraConfigFrameWork::UpdateIPC(ipc_unit *pCam)
{
	if(pCam == NULL)
	{
		return;
	}
	
	if(m_nCurMouseLine == -1)
	{
		return;
	}
	
	int idx = 0;
	ipc_node *p = m_ipc_head;
	while(p)
	{
		if((idx == m_nPageIndex*(m_rows-1)+m_nCurMouseLine) && (p->ipcam.protocol_type == pCam->protocol_type) && (strcmp(p->ipcam.uuid, pCam->uuid) == 0))
		{
			struct in_addr host;
			host.s_addr = pCam->dwIp;
			pStatic0[m_offset+m_nCurMouseLine*m_cols+1]->SetText(inet_ntoa(host));
			p->ipcam = *pCam;
			
			int i;
			for(i=0;i<GetMaxChnNum();i++)
			{
				SBizIPCameraPara ipcam;
				memset(&ipcam, 0, sizeof(ipcam));
				ipcam.channel_no = i;
				
				SBizParaTarget bizTar;
				bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
				bizTar.nChn = ipcam.channel_no;
				BizGetPara(&bizTar, (void *)&ipcam);
				if((ipcam.protocol_type == pCam->protocol_type) && (strcmp(ipcam.uuid, pCam->uuid) == 0))
				{
					ipcam.dwIp = pCam->dwIp;
					strcpy(ipcam.user, pCam->user);
					strcpy(ipcam.pwd, pCam->pwd);
					BizSetPara(&bizTar, (void *)&ipcam);
				}
			}
			
			break;
		}
		
		idx++;
		p = p->next;
	}
}

CPageIPCameraModifyFrameWork::CPageIPCameraModifyFrameWork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
{
	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);
	
	memset(pStatic0, 0, sizeof(pStatic0));
	memset(pIpCtrl0, 0, sizeof(pIpCtrl0));
	memset(pButton0, 0, sizeof(pButton0));
	
	pStatus = NULL;
	
	memset(&ipcam, 0, sizeof(ipcam));
	
	InitPage0();
}

CPageIPCameraModifyFrameWork::~CPageIPCameraModifyFrameWork()
{
	
}

VD_PCSTR CPageIPCameraModifyFrameWork::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

void CPageIPCameraModifyFrameWork::InitPage0()
{
	const char* statictext[] = 
	{
		"&CfgPtn.IPAddress",
		"&CfgPtn.SubnetMask",
		"&CfgPtn.Gateway",
		"&CfgPtn.DNSServer",//"&CfgPtn.PrefeeredDNSServer",
		"&CfgPtn.AlternateDNSServer"
	};
	
	int count = sizeof(statictext)/sizeof(statictext[0]);
	
	CRect tmpRt(80, 8, 80+160, 8+32);
	
	int i = 0;
	for(i = 0; i < count; i++)
	{
		tmpRt.top += 40;
		tmpRt.bottom += 40;
		pStatic0[i] = CreateStatic(CRect(tmpRt.left, tmpRt.top, tmpRt.right, tmpRt.bottom), 
									this, statictext[i]);
		pStatic0[i]->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
		//pStatic0[i]->SetBkColor(VD_RGB(67,77,87));
	}
	
	tmpRt = CRect(80+160+16, 8, 80+160+16+240, 8+32);
	
	for(i = 0; i < count; i++)
	{
		tmpRt.top += 40;
		tmpRt.bottom += 40;
		pIpCtrl0[i] = CreateIPAddressCtrl(CRect(tmpRt.left, tmpRt.top, tmpRt.right, tmpRt.bottom), this);
		IPADDR sIPIns;
		sIPIns.l = 0;//inet_addr("192.168.1.88");
		pIpCtrl0[i]->SetIPAddress(&sIPIns);
	}
	
	CRect staRect(80, 328+4-4, 220, 366-4-4);
	pStatus = CreateStatic(staRect, this, ""/*"Hello World"*/);
	
	CRect rt(480, 328+4-4, 480+60, 366-4-4);
	
	pButton0[0] = CreateButton(rt, this, "&CfgPtn.apply", (CTRLPROC)&CPageIPCameraModifyFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[0]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	
	rt.left = rt.right+4;
	rt.right = rt.left+60;
	pButton0[1] = CreateButton(rt, this, "&CfgPtn.Exit", (CTRLPROC)&CPageIPCameraModifyFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[1]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
}

VD_BOOL CPageIPCameraModifyFrameWork::UpdateData(UDM mode)
{
	if(UDM_OPEN == mode)
	{
		IPADDR sIPIns;
		
		sIPIns.l = ipcam.dwIp;
		pIpCtrl0[0]->SetIPAddress(&sIPIns);
		
		sIPIns.l = ipcam.net_mask;
		pIpCtrl0[1]->SetIPAddress(&sIPIns);
		
		sIPIns.l = ipcam.net_gateway;
		pIpCtrl0[2]->SetIPAddress(&sIPIns);
		
		sIPIns.l = ipcam.dns1;
		pIpCtrl0[3]->SetIPAddress(&sIPIns);
		
		sIPIns.l = ipcam.dns2;
		pIpCtrl0[4]->SetIPAddress(&sIPIns);
		
		pStatus->SetText("");
	}
	else if(UDM_OPENED == mode)
	{
		pStatic0[4]->Show(FALSE, TRUE);
		pIpCtrl0[4]->Show(FALSE, TRUE);
	}
	else if(UDM_CLOSED == mode)
	{
		
	}
	return TRUE;
}

void CPageIPCameraModifyFrameWork::OnClickSubPage()
{
	//printf("CPageIPCameraModifyFrameWork::OnClickSubPage-1\n");
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < sizeof(pButton0)/sizeof(pButton0[0]); i++)
	{
		if(pFocusButton == pButton0[i] && pFocusButton != NULL)
		{
			bFind = TRUE;
			break;
		}
	}
	
	//printf("CPageIPCameraModifyFrameWork::OnClickSubPage-2\n");
	
	if(!bFind)
	{
		return;
	}
	
	//printf("CPageIPCameraModifyFrameWork::OnClickSubPage-3,i=%d\n",i);
	
	switch(i)
	{
	case 0://应用
		{
			ipc_neteork_para_t nw;
			IPADDR sIPIns;
			
			pIpCtrl0[0]->GetIPAddress(&sIPIns);
			nw.ip_address = sIPIns.l;
			
			pIpCtrl0[1]->GetIPAddress(&sIPIns);
			nw.net_mask = sIPIns.l;
			
			pIpCtrl0[2]->GetIPAddress(&sIPIns);
			nw.net_gateway = sIPIns.l;
			
			pIpCtrl0[3]->GetIPAddress(&sIPIns);
			nw.dns1 = sIPIns.l;
			
			pIpCtrl0[4]->GetIPAddress(&sIPIns);
			nw.dns2 = sIPIns.l;
			
			if(IPC_SetNetworkParam(&ipcam, &nw) == 0)
			{
				pStatus->SetText("&CfgPtn.SaveParaSuccess");
				
				ipcam.dwIp = nw.ip_address;
				ipcam.net_mask = nw.net_mask;
				ipcam.net_gateway = nw.net_gateway;
				ipcam.dns1 = nw.dns1;
				ipcam.dns2 = nw.dns2;
				
				CPageIPCameraConfigFrameWork *pPG = (CPageIPCameraConfigFrameWork *)m_pParent;
				pPG->UpdateIPC(&ipcam);
			}
			else
			{
				pStatus->SetText("&CfgPtn.SaveParaFail");
			}
		}
		break;
	case 1://退出
		{
			//printf("exit\n");
			this->Close();
		}
		break;
	default:
		break;
	}
}

VD_BOOL CPageIPCameraModifyFrameWork::MsgProc( uint msg, uint wpa, uint lpa )
{
	u8 lock_flag = 0;
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
			
		}
		break;
		case XM_KEYDOWN:
			switch(wpa)
			{
				case KEY_RIGHT:
				case KEY_LEFT:
				case KEY_UP:
				case KEY_DOWN:
				{
					CPage::MsgProc(msg, wpa, lpa);
					
					static CItem* last = NULL;
					CItem* temp = GetFocusItem();
					
					if(temp != last)
					{
						pStatus->SetText("");
						
						last = temp;
					}
					
					return TRUE;
				}
				default:
					break;
			}
		case XM_MOUSEMOVE:
			{
				CPage::MsgProc(msg, wpa, lpa);
				
				static CItem* last = NULL;
				CItem* temp = GetFocusItem();
				
				if(temp != last)
				{
					pStatus->SetText("");
					
					last = temp;
				}
				
				return TRUE;
			}
			break;
		default:
			break;
	}
	
	return CPage::MsgProc(msg, wpa, lpa);
}

void CPageIPCameraModifyFrameWork::SetIPC(ipc_unit *pCam)
{
	if(pCam == NULL)
	{
		return;
	}
	ipcam = *pCam;
}

void CPageIPCameraModifyFrameWork::GetIPC(ipc_unit *pCam)
{
	if(pCam == NULL)
	{
		return;
	}
	*pCam = ipcam;
}

