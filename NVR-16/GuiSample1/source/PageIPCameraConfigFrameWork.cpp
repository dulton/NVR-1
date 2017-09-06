#include "GUI/Pages/PageIPCameraConfigFrameWork.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/BizData.h"
#include "biz.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageInfoBox.h"
#include "GUI/Pages/PageConfigFrameWork.h"
#include "GUI/Pages/PageDesktop.h"
#include "lib_venc.h"

//#define IPC_PROTOCOL_TEST //test record disk error by yaogang 20170222

extern int IPC_GetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);
extern int IPC_SetNetworkParam(ipc_unit *ipcam, ipc_neteork_para_t *pnw);

//add by liu
#define YUETIAN_PROTOCOL

enum IPC_PROTOCOL
{
	PROTOCOL_ALL = 0,
	PROTOCOL_ONVIF,
	PROTOCOL_YT,
	PROTOCOL_YTHW,
	PROTOCOL_NVR,
#ifdef IPC_PROTOCOL_TEST
	PROTOCOL_IPC_TEST,
#endif
};


#define LEFT_PAGE_L			5
#define LEFT_PAGE_R			(m_Rect.Width()-8)
#define RIGHT_PAGE_L		5
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
	m_rtTable2 = CRect(0, 0, 0, 0);
	
	m_cols = 0;
	m_rows = 0;
	m_offset = 0;
	
	m_cols2 = 0;
	m_rows2 = 0;
	m_offset2 = 0;
	
	m_ipc_head = NULL;
	m_nPageIndex = 0;
	m_nCurMouseLine = -1;
	
	m_channel_list = NULL;
	m_nPageIndex2 = 0;
	m_nCurMouseLine2 = -1;

	//yaogang modify 20160304
	SBizDvrInfo DvrInfo;
	if (BizGetDvrInfo(&DvrInfo) == 0)
	{
		nNVROrDecoder = DvrInfo.nNVROrDecoder; //解码器or  NVR版本
	}
	
	InitPage0();
	
	m_pPageIPCModify = new CPageIPCameraModifyFrameWork(NULL, "&CfgPtn.IPCamera", icon_dvr, this);
	m_pPageIPCSetChn = new CPageIPCameraSetChnFrameWork(NULL, "&CfgPtn.IPCamera", icon_dvr, this);
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
	int i = 0, j = 0;
	
	CRect rtFrame0(LEFT_PAGE_L, 6, LEFT_PAGE_R, 146);
	pTabFrame0[0] = CreateTableBox(rtFrame0,this,1,1);
	pTabFrame0[0]->SetFrameColor(VD_RGB(56,108,170));
	
	CRect rtFrame1(RIGHT_PAGE_L, 144, RIGHT_PAGE_R, 174);
	pTabFrame0[1] = CreateTableBox(rtFrame1,this,1,1);
	pTabFrame0[1]->SetFrameColor(VD_RGB(56,108,170));
	
	CRect rtFrame2(RIGHT_PAGE_L, 172, RIGHT_PAGE_R, 340);
	pTabFrame0[2] = CreateTableBox(rtFrame2,this,1,1);
	pTabFrame0[2]->SetFrameColor(VD_RGB(56,108,170));
	
	CRect rtFrame3(RIGHT_PAGE_L, 338, RIGHT_PAGE_R, 368);
	pTabFrame0[3] = CreateTableBox(rtFrame3,this,1,1);
	pTabFrame0[3]->SetFrameColor(VD_RGB(56,108,170));
	
	CRect tmpRt;
	CRect tmpRt2;
	
	int button_index = 0;
	
	const char* szTitle[] = 
	{
		"&CfgPtn.ID",
		"&CfgPtn.IPAddress",
		"&CfgPtn.Port",
		"&CfgPtn.Protocol",
		"&CfgPtn.Edit",
		"&CfgPtn.Hit"
	};
	
	m_cols = sizeof(szTitle)/sizeof(szTitle[0]);
	m_rows = 6;
	
	//printf("&&&&&&&&& m_cols = %d &&&&&&&&\n",m_cols);
	
	CRect rtTable1(rtFrame0.left+4,rtFrame0.top+1,rtFrame0.right-24,rtFrame0.bottom-3);
	pTable0[0] = CreateTableBox(rtTable1, this, m_cols, m_rows);
	int index = 0;
	pTable0[0]->SetColWidth(index++, 52);
	pTable0[0]->SetColWidth(index++, 132);
	pTable0[0]->SetColWidth(index++, 64);
	pTable0[0]->SetColWidth(index++, 80);
	pTable0[0]->SetColWidth(index++, 132);
	
	m_rtTable1 = rtTable1;
	
	int count = 0;
	
	for(i = 0; i < m_cols; i++)
	{
		pTable0[0]->GetTableRect(i, 0, &tmpRt);
		
		tmpRt.top -= 1;
		tmpRt.bottom += 1;
		
		pStatic0[count+i] = CreateStatic(CRect(rtTable1.left+tmpRt.left, rtTable1.top+tmpRt.top, 
									rtTable1.left+tmpRt.right, rtTable1.top+tmpRt.bottom), 
									this, szTitle[i]);
		pStatic0[count+i]->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
		pStatic0[count+i]->SetBkColor(VD_RGB(67,77,87));
	}
	
	m_offset = count + m_cols;
	
	for(j = 1; j < m_rows; j++)
	{
		for(i = 0; i < m_cols; i++)
		{
			pTable0[0]->GetTableRect(i, j, &tmpRt);
			
			if(i <= 3)
			{
				pStatic0[count+j*m_cols+i] = CreateStatic(CRect(rtTable1.left+tmpRt.left, rtTable1.top+tmpRt.top, 
												rtTable1.left+tmpRt.right, rtTable1.top+tmpRt.bottom), 
												this, NULL, TRUE, 0, FS_NORMAL);
				pStatic0[count+j*m_cols+i]->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
			}
			else if(i == 4)
			{
				pButton0[button_index] = CreateButton(CRect(rtTable1.left+tmpRt.left, rtTable1.top+tmpRt.top, 
												rtTable1.left+tmpRt.right, rtTable1.top+tmpRt.bottom), 
												this, "&CfgPtn.Modify", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, 
												NULL, 0, FALSE);
				pButton0[button_index]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
				button_index++;
			}
			else if(i == 5)
			{
				pCheckBox0[j-1] = CreateCheckBox(CRect(rtTable1.left+tmpRt.left, rtTable1.top+tmpRt.top, 
												rtTable1.left+tmpRt.right, rtTable1.top+tmpRt.bottom), this, 
												styleEditable, (CTRLPROC)&CPageIPCameraConfigFrameWork::OnCheckSel);
			}
		}
	}
	
	pScrollbar0[0] = CreateScrollBar(CRect(m_rtTable1.right,m_rtTable1.top,m_rtTable1.right+20,m_rtTable1.bottom),
                                    this,
                                    scrollbarY,
								    0,
								    0,
								    1,
								    (CTRLPROC)&CPageIPCameraConfigFrameWork::OnTrackMove0);
	
	CRect rt(rtFrame1.left+2, rtFrame1.top+2, rtFrame1.left+2, rtFrame1.bottom-2);
	
	rt.left = rt.right+4;
	rt.right = rt.left+68;
	pStatic0[count + m_rows*m_cols] = CreateStatic(rt, this, NULL, TRUE, 0, FS_NORMAL);
	pStatic0[count + m_rows*m_cols]->SetText("&CfgPtn.Protocol");
	
	rt.left = rt.right+4;
	rt.right = rt.left+80;
	pCombox0[0] = CreateComboBox(rt, this, NULL, NULL, NULL, 0);
	pCombox0[0]->AddString("&CfgPtn.All");
	pCombox0[0]->AddString("ONVIF");
	//pCombox0[0]->AddString("I13");
	//pCombox0[0]->AddString("I99");
	
	//pCombox0[0]->AddString("I36");
	//add by liu
	#ifdef YUETIAN_PROTOCOL
		pCombox0[0]->AddString("&CfgPtn.YueTian");
	#else
		pCombox0[0]->AddString("I36");
	#endif

	//XM
	pCombox0[0]->AddString("&CfgPtn.YueTianHW");

	if (nNVROrDecoder != 1)//解码器版本
	{
		pCombox0[0]->AddString("NVR");
	}
	
	#ifdef IPC_PROTOCOL_TEST
	printf("%s IPC_PROTOCOL_TEST\n", __func__);
		pCombox0[0]->AddString("IpcTest");
	#endif	
	
	//pCombox0[0]->AddString("FH8610");
	//pCombox0[0]->AddString("ANNI");
	//pCombox0[0]->AddString("RIVER");
	pCombox0[0]->SetCurSel(0);
	
	rt.left = rt.right+8;
	rt.right = rt.left+76;
	pButton0[button_index] = CreateButton(rt, this, "&CfgPtn.Search", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[button_index]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	button_index++;
	
	rt.left = rt.right+8;
	rt.right = rt.left+108;
	pButton0[button_index] = CreateButton(rt, this, "&CfgPtn.AutoAssign", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[button_index]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	button_index++;
	
	rt.left = rt.right+8;
	rt.right = rt.left+76;
	pButton0[button_index] = CreateButton(rt, this, "&CfgPtn.ADD", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[button_index]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	button_index++;
	
	const char* DevList[] = 
	{
		"&CfgPtn.Channel",
		"&CfgPtn.IPAddress",
		"&CfgPtn.Port",
		"&CfgPtn.Protocol",
		//"&CfgPtn.Status",
		"&CfgPtn.Edit",
		"&CfgPtn.Hit"
	};
	
	m_cols2 = sizeof(DevList)/sizeof(DevList[0]);
	m_rows2 = 7;
	
	//printf("&&&&&&&&& m_cols2 = %d &&&&&&&&\n",m_cols2);
	
	CRect rtTable2(rtFrame2.left+4,rtFrame2.top+3,rtFrame2.right-24,rtFrame2.bottom-3);
	pTable0[1] = CreateTableBox(rtTable2, this, m_cols2, m_rows2);
	index = 0;
	pTable0[1]->SetColWidth(index++, 52);
	pTable0[1]->SetColWidth(index++, 132);
	pTable0[1]->SetColWidth(index++, 64);
	pTable0[1]->SetColWidth(index++, 80);
	//pTable0[1]->SetColWidth(index++, 80);
	//pTable0[1]->SetColWidth(index++, 96);
	pTable0[1]->SetColWidth(index++, 132);
	
	m_rtTable2 = rtTable2;
	
	int count1 = count + m_rows*m_cols + 1;
	
	for(i = 0; i < m_cols2; i++)
	{
		pTable0[1]->GetTableRect(i, 0, &tmpRt2);
		
		pStatic0[count1+i] = CreateStatic(CRect(rtTable2.left+tmpRt2.left, rtTable2.top+tmpRt2.top, 
									rtTable2.left+tmpRt2.right, rtTable2.top+tmpRt2.bottom), 
									this, DevList[i]);
		pStatic0[count1+i]->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
		pStatic0[count1+i]->SetBkColor(VD_RGB(67,77,87));
	}
	
	m_offset2 = count1 + m_cols2;
	
	for(j = 1; j < m_rows2; j++)
	{
		for(i = 0; i < m_cols2; i++)
		{
			pTable0[1]->GetTableRect(i, j, &tmpRt2);
			
			//if(i <= 4)
			if(i <= 3)
			{
				pStatic0[count1+j*m_cols2+i] = CreateStatic(CRect(rtTable2.left+tmpRt2.left, rtTable2.top+tmpRt2.top, 
												rtTable2.left+tmpRt2.right, rtTable2.top+tmpRt2.bottom), 
												this, NULL, TRUE, 0, FS_NORMAL);
				pStatic0[count1+j*m_cols2+i]->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
			}
			//else if(i == 5)
			else if(i == 4)
			{
				tmpRt2.top -= 1;
				tmpRt2.bottom += 1;
				
				pButton0[button_index] = CreateButton(CRect(rtTable2.left+tmpRt2.left, rtTable2.top+tmpRt2.top, 
												rtTable2.left+tmpRt2.right, rtTable2.top+tmpRt2.bottom), 
												this, "&CfgPtn.Setup", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, 
												NULL, 0, FALSE);
				pButton0[button_index]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
				button_index++;
			}
			//else if(i == 6)
			else if(i == 5)
			{
				pCheckBox0[m_rows-1+j-1] = CreateCheckBox(CRect(rtTable2.left+tmpRt2.left, rtTable2.top+tmpRt2.top, 
												rtTable2.left+tmpRt2.right, rtTable2.top+tmpRt2.bottom), this, 
												styleEditable, (CTRLPROC)&CPageIPCameraConfigFrameWork::OnCheckSel);
			}
		}
	}
	
 	printf("&&&&&&&&& count = %d &&&&&&&&\n",count);//0
	printf("&&&&&&&&& count1 = %d &&&&&&&&\n",count1);//37
	
	pScrollbar0[1] = CreateScrollBar(CRect(m_rtTable2.right,m_rtTable2.top,m_rtTable2.right+20,m_rtTable2.bottom),
                                this,
                                scrollbarY,
							    0,
							    0,
							    1,
							    (CTRLPROC)&CPageIPCameraConfigFrameWork::OnTrackMove1);
	
	CRect rt1(rtFrame3.left+2, rtFrame3.top+2, rtFrame3.left+2, rtFrame3.bottom-2);
	
	rt1.left = rt1.right+4;
	rt1.right = rt1.left+80;
	pButton0[button_index] = CreateButton(rt1, this, "&CfgPtn.ADD", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[button_index]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	button_index++;
	
	rt1.left = rt1.right+4;
	rt1.right = rt1.left+80;
	pButton0[button_index] = CreateButton(rt1, this, "&CfgPtn.Delete", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[button_index]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	button_index++;
	
	rt1.left = 420;
	rt1.right = rt1.left+96;
	pButton0[button_index] = CreateButton(rt1, this, "&CfgPtn.apply", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[button_index]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	button_index++;
	
	rt1.left = rt1.right+4;
	rt1.right = rt1.left+96;
	pButton0[button_index] = CreateButton(rt1, this, "&CfgPtn.Exit", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[button_index]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	button_index++;
	
	rt1.left -= 100;
	rt1.right = rt1.left+96;
	pButton0[button_index] = CreateButton(rt1, this, "&CfgPtn.Prev", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[button_index]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	button_index++;
	
	rt1.left = rt1.right+4;
	rt1.right = rt1.left+96;
	pButton0[button_index] = CreateButton(rt1, this, "&CfgPtn.Next", (CTRLPROC)&CPageIPCameraConfigFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[button_index]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	button_index++;
}

void CPageIPCameraConfigFrameWork::OnCheckSel()
{
	int i = 0;
	BOOL bFind = FALSE;
	CCheckBox *pFocusCheckBox = (CCheckBox *)GetFocusItem();
	for(i = 0; i < sizeof(pCheckBox0)/sizeof(pCheckBox0[0]); i++)
	{
		if(pFocusCheckBox == pCheckBox0[i] && pFocusCheckBox != NULL)
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(!bFind)
	{
		return;
	}
	
	if(i < m_rows-1)
	{
		int pos = m_nPageIndex;
		int start_idx = (pos)*(m_rows-1);
		int end_idx = (pos+1)*(m_rows-1);
		
		int count = 0;
		ipc_node *p = m_ipc_head;
		while(p)
		{
			if(count == start_idx+i)
			{
				p->ipcam.reserved[0] = (char)pFocusCheckBox->GetValue();//
				break;
			}
			count++;
			if(count >= end_idx)
			{
				break;
			}
			p = p->next;
		}
	}
	else if(i < m_rows-1+m_rows2-1)
	{
		i -= (m_rows-1);
		
		int pos = m_nPageIndex2;
		int start_idx = (pos)*(m_rows2-1);
		int end_idx = (pos+1)*(m_rows2-1);
		
		int count = 0;
		ipc_node *p = m_channel_list;
		while(p)
		{
			if(count == start_idx+i)
			{
				p->ipcam.reserved[0] = (char)pFocusCheckBox->GetValue();
				break;
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

void CPageIPCameraConfigFrameWork::OnTrackMove0()
{
	if(m_ipc_head)
	{
		int pos = pScrollbar0[0]->GetPos();
		if(m_nPageIndex == pos)
		{
			return;
		}
		
		m_nPageIndex = pos;
		m_nCurMouseLine = -1;
		
		int start_idx = (pos)*(m_rows-1);
		int end_idx = (pos+1)*(m_rows-1);
		
		for(int j = 0; j < m_rows - 1; j++)
		{
			for(int i = 0; i < m_cols; i++)
			{
				if(pStatic0[m_offset+j*m_cols+i] != NULL)
				{
					pStatic0[m_offset+j*m_cols+i]->SetText(NULL);
					pStatic0[m_offset+j*m_cols+i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
				}
			}
			
			pCheckBox0[j]->Show(FALSE);
			pButton0[j]->Show(FALSE);
			pCheckBox0[j]->SetValue(0);
		}
		
		int count = 0;
		ipc_node *p = m_ipc_head;
		while(p)
		{
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
				/*
				else if(p->ipcam.protocol_type == PRO_TYPE_HAIXIN)
				{
					strcpy(szInfo, "I13");
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_JUAN)
				{
					strcpy(szInfo, "I99");
				}
				*/
				else if(p->ipcam.protocol_type == PRO_TYPE_KLW)
				{
					//strcpy(szInfo, "I36");
					//add by liu
					#ifdef YUETIAN_PROTOCOL
						strcpy(szInfo, "&CfgPtn.YueTian");
					#else
						strcpy(szInfo, "I36");
					#endif
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_XM)
				{
					strcpy(szInfo, "&CfgPtn.YueTianHW");
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_NVR)
				{
					strcpy(szInfo, "NVR");
				}
			#ifdef IPC_PROTOCOL_TEST
				else if(p->ipcam.protocol_type == PRO_TYPE_IPC_TEST)
				{
					strcpy(szInfo, "IpcTest");
				}
			#endif
				
				/*
				else if(p->ipcam.protocol_type == PRO_TYPE_FULLHAN)
				{
					strcpy(szInfo, "FH8610");
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_ANNI)
				{
					strcpy(szInfo, "ANNI");
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_RIVER)
				{
					strcpy(szInfo, "RIVER");
				}
				*/
				else
				{
					memset(szInfo, 0, sizeof(szInfo));
				}
				pStatic0[m_offset+(count-start_idx)*m_cols+3]->SetText(szInfo);
				pCheckBox0[count-start_idx]->Show(TRUE);
				pButton0[count-start_idx]->Show(TRUE);
				pCheckBox0[count-start_idx]->SetValue((int)p->ipcam.reserved[0]);
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

void CPageIPCameraConfigFrameWork::OnTrackMove1()
{
	if(m_channel_list)
	{
		int pos = pScrollbar0[1]->GetPos();
		if(m_nPageIndex2 == pos)
		{
			return;
		}
		
		m_nPageIndex2 = pos;
		m_nCurMouseLine2 = -1;
		
		int start_idx = (pos)*(m_rows2-1);
		int end_idx = (pos+1)*(m_rows2-1);
		
		int show_num = 0;
		
		int count = 0;
		ipc_node *p = m_channel_list;
		while(p)
		{
			if(count >= start_idx && count < end_idx)
			{
				count -= start_idx;
				
				char szInfo[64];
				sprintf(szInfo, "%d", p->ipcam.channel_no+1);
				pStatic0[m_offset2+count*m_cols2+0]->SetText(szInfo);
				struct in_addr host;
				host.s_addr = p->ipcam.dwIp;
				strcpy(szInfo, inet_ntoa(host));
				pStatic0[m_offset2+count*m_cols2+1]->SetText(szInfo);
				sprintf(szInfo, "%d", p->ipcam.wPort);
				pStatic0[m_offset2+count*m_cols2+2]->SetText(szInfo);
				if(p->ipcam.protocol_type == PRO_TYPE_ONVIF)
				{
					strcpy(szInfo, "ONVIF");
				}
				/*
				else if(p->ipcam.protocol_type == PRO_TYPE_HAIXIN)
				{
					strcpy(szInfo, "I13");
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_JUAN)
				{
					strcpy(szInfo, "I99");
				}
				*/
				else if(p->ipcam.protocol_type == PRO_TYPE_KLW)
				{
					//strcpy(szInfo, "I36");
					//add by liu
					#ifdef YUETIAN_PROTOCOL
						strcpy(szInfo, "&CfgPtn.YueTian");
					#else
						strcpy(szInfo, "I36");
					#endif
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_XM)
				{
					strcpy(szInfo, "&CfgPtn.YueTianHW");
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_NVR)
				{
					strcpy(szInfo, "NVR");
				}
			#ifdef IPC_PROTOCOL_TEST
				else if(p->ipcam.protocol_type == PRO_TYPE_IPC_TEST)
				{
					strcpy(szInfo, "IpcTest");
				}
			#endif
				/*
				else if(p->ipcam.protocol_type == PRO_TYPE_FULLHAN)
				{
					strcpy(szInfo, "FH8610");
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_ANNI)
				{
					strcpy(szInfo, "ANNI");
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_RIVER)
				{
					strcpy(szInfo, "RIVER");
				}
				*/
				else
				{
					memset(szInfo, 0, sizeof(szInfo));
				}
				pStatic0[m_offset2+count*m_cols2+3]->SetText(szInfo);
				//sprintf(szInfo, "%s", IPC_GetLinkStatus(p->ipcam.channel_no)?"Online":"Offline");
				//pStatic0[m_offset2+count*m_cols2+4]->SetText(szInfo);
				pButton0[8+count]->Show(TRUE);
				pCheckBox0[5+count]->Show(TRUE);
				pCheckBox0[5+count]->SetValue((int)p->ipcam.reserved[0]);
				
				count += start_idx;
				
				show_num++;
			}
			count++;
			p = p->next;
		}
		
		if(show_num < m_rows2-1)
		{
			for(int j = show_num; j < m_rows2-1; j++)
			{
				for(int i = 0; i < m_cols2; i++)
				{
					if(pStatic0[m_offset2+j*m_cols2+i] != NULL)
					{
						pStatic0[m_offset2+j*m_cols2+i]->SetText(NULL);
						pStatic0[m_offset2+j*m_cols2+i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
					}
				}
				
				pCheckBox0[5+j]->Show(FALSE);
				pButton0[8+j]->Show(FALSE);
				pCheckBox0[5+j]->SetValue(0);
			}
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
	{//搜索到的IPC，显示5列，响应修改按键
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		{
			MouseMoveToLine(i);
			
			int pos = m_nPageIndex;
			int start_idx = (pos)*(m_rows-1);
			int end_idx = (pos+1)*(m_rows-1);
			
			int count = 0;
			ipc_node *p = m_ipc_head;
			while(p)
			{
				if(count == start_idx+i)
				{
					//if(p->ipcam.protocol_type == PRO_TYPE_ONVIF || p->ipcam.net_mask == 0)
					//if(p->ipcam.protocol_type == PRO_TYPE_ONVIF || p->ipcam.protocol_type == PRO_TYPE_KLW || p->ipcam.protocol_type == PRO_TYPE_ANNI || p->ipcam.protocol_type == PRO_TYPE_RIVER)
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
				count++;
				if(count >= end_idx)
				{
					break;
				}
				p = p->next;
			}
		}
		break;
	case 5://搜索
	case 6://自动分配
		{
			unsigned char check_conflict = (i == 6) ? 1 : 0;
			
			int sel = pCombox0[0]->GetCurSel();
			unsigned int pro_type = (unsigned int)(-1);//PROTOCOL_ALL
			
			if(sel == PROTOCOL_ONVIF)
			{
				pro_type = PRO_TYPE_ONVIF;
			}
			else if(sel == PROTOCOL_YT)
			{
				pro_type = PRO_TYPE_KLW;
			}
			else if(sel == PROTOCOL_YTHW)
			{
				pro_type = PRO_TYPE_XM;
			}			
			else if (sel == PROTOCOL_NVR)
			{
				if (nNVROrDecoder != 1)//解码器版本
				{
					pro_type = PRO_TYPE_NVR;
				}
			}
		#ifdef IPC_PROTOCOL_TEST
			else if(sel == PROTOCOL_IPC_TEST)
			{
				pro_type = PRO_TYPE_IPC_TEST;
			}
		#endif
			
			if(m_ipc_head)
			{
				IPC_Free(m_ipc_head);
				m_ipc_head = NULL;
			}
			
			m_nPageIndex = 0;
			m_nCurMouseLine = -1;
			
			for(int j = 0; j < m_rows - 1; j++)
			{
				for(int i = 0; i < m_cols; i++)
				{
					if(pStatic0[m_offset+j*m_cols+i] != NULL)
					{
						pStatic0[m_offset+j*m_cols+i]->SetText(NULL);
						pStatic0[m_offset+j*m_cols+i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
					}
				}
				
				pCheckBox0[j]->Show(FALSE);
				pButton0[j]->Show(FALSE);
				pCheckBox0[j]->SetValue(0);
			}
			
			int ret = IPC_Search(&m_ipc_head, pro_type, check_conflict);
			if(ret < 0 || m_ipc_head == NULL)
			{
				pScrollbar0[0]->SetRange(0,0,FALSE);
				pScrollbar0[0]->SetPage(1,FALSE);
				pScrollbar0[0]->SetPos(0,TRUE);
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
			
			int count = 0;
			ipc_node *p = m_ipc_head;
			while(p)//根据搜索结果填充条目
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
				/*
				else if(p->ipcam.protocol_type == PRO_TYPE_HAIXIN)
				{
					strcpy(szInfo, "I13");
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_JUAN)
				{
					strcpy(szInfo, "I99");
				}
				*/
				else if(p->ipcam.protocol_type == PRO_TYPE_KLW)
				{
					//strcpy(szInfo, "I36");
					//add by liu
					#ifdef YUETIAN_PROTOCOL
						strcpy(szInfo, "&CfgPtn.YueTian");
					#else
						strcpy(szInfo, "I36");
					#endif
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_XM)
				{
					strcpy(szInfo, "&CfgPtn.YueTianHW");
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_NVR)
				{
					strcpy(szInfo, "NVR");
				}
			#ifdef IPC_PROTOCOL_TEST
				else if(p->ipcam.protocol_type == PRO_TYPE_IPC_TEST)
				{
					strcpy(szInfo, "IpcTest");
				}
			#endif
				/*
				else if(p->ipcam.protocol_type == PRO_TYPE_FULLHAN)
				{
					strcpy(szInfo, "FH8610");
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_ANNI)
				{
					strcpy(szInfo, "ANNI");
				}
				else if(p->ipcam.protocol_type == PRO_TYPE_RIVER)
				{
					strcpy(szInfo, "RIVER");
				}
				*/
				else
				{
					memset(szInfo, 0, sizeof(szInfo));
				}
				pStatic0[m_offset+count*m_cols+3]->SetText(szInfo);
				pCheckBox0[count]->Show(TRUE);
				pButton0[count]->Show(TRUE);
				count++;
				if(count == m_rows-1)
				{
					break;
				}
				p = p->next;
			}
			
			if(i == 6)
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

						//printf("BizSetPara chn%d\n", j);
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
					if(count >= GetMaxChnNum())
					{
						break;
					}
					
					p = p->next;
				}
				
				GetChannelList();
			}
		}
		break;
	case 7://添加摄像机
		{
			unsigned char refresh = 0;
			ipc_node *p = m_ipc_head;
			while(p)
			{
				if(p->ipcam.reserved[0])
				{
					if(!AddIPC(p->ipcam))
					{
						break;
					}
					
					refresh = 1;
				}
				
				p = p->next;
			}
			
			if(refresh)
			{
				GetChannelList();
			}
		}
		break;
		//已添加的IPC，显示6列，响应设置按键
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
		{
			int chn = 0;
			
			int pos = m_nPageIndex2;
			int start_idx = (pos)*(m_rows2-1);
			int end_idx = (pos+1)*(m_rows2-1);
			
			int count = 0;
			ipc_node *p = m_channel_list;
			while(p)
			{
				if(count == start_idx+(i-8))
				{
					chn = p->ipcam.channel_no;
					m_pPageIPCSetChn->SetChn(chn,0);
					m_pPageIPCSetChn->Open();
					
					break;
				}
				count++;
				if(count >= end_idx)
				{
					break;
				}
				p = p->next;
			}
		}
		break;
	case 14://添加通道
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
				
				if(!ipcam.enable)
				{
					break;
				}
			}
			if(j >= GetMaxChnNum())
			{
				j = 0;
			}
			m_pPageIPCSetChn->SetChn(j,1);
			m_pPageIPCSetChn->Open();
		}
		break;
	case 15://删除通道
		{
			unsigned char refresh = 0;
			ipc_node *p = m_channel_list;
			while(p)
			{
				if(p->ipcam.reserved[0])//删除选中的通道
				{
					int chn = p->ipcam.channel_no;
					
					SBizIPCameraPara ipcam;
					memset(&ipcam, 0, sizeof(ipcam));
					
					SBizParaTarget bizTar;
					bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
					bizTar.nChn = chn;
					BizGetPara(&bizTar, (void *)&ipcam);
					
					if(ipcam.enable)
					{
						ipcam.channel_no = chn;
						ipcam.enable = 0;

						//printf("delete ipc chn%d\n", ipcam.channel_no);
						BizSetPara(&bizTar, (void *)&ipcam);
					}
					
					refresh = 1;
				}
				
				p = p->next;
			}
			
			if(refresh)
			{
				GetChannelList();
			}
		}
		break;
	case 17://退出
		{
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

VD_BOOL CPageIPCameraConfigFrameWork::AddIPC_nvr(int firstChn, ipc_unit &camera)
{
	int maxChn = GetMaxChnNum();
	int nvrChnNum = camera.main_audio_sw;//NVR通道数
	int i;
	SBizIPCameraPara ipcam;
	SBizParaTarget bizTar;
	
	//printf("%s nvr maxChnNum: %d, link firstChn: %d\n", 
	//	__func__, nvrChnNum, firstChn);

	if (firstChn + nvrChnNum > maxChn)
	{
		printf("firstChn + nvrChnNum > system chn num\n");
		return FALSE;
	}

	i = firstChn;
	for(i=firstChn; i<firstChn+nvrChnNum; i++)
	//for(i=firstChn; i<firstChn+4; i++)
	{
		memset(&ipcam, 0, sizeof(ipcam));
		
		bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
		bizTar.nChn = i;
		BizGetPara(&bizTar, (void *)&ipcam);

		ipcam.channel_no = i;//系统通道号
		ipcam.main_audio_sw = nvrChnNum;//NVR最大通道数
		ipcam.sub_audio_sw = i-firstChn;//NVR自己的通道号，指示要申请NVR 的哪一路码流
		
		ipcam.enable = 1;
		ipcam.protocol_type = camera.protocol_type;
		ipcam.trans_type = camera.trans_type;
		ipcam.stream_type = camera.stream_type;
		ipcam.ipc_type = camera.ipc_type;
		ipcam.dwIp = camera.dwIp;
		ipcam.wPort = camera.wPort;
		strcpy(ipcam.user, camera.user);
		strcpy(ipcam.pwd, camera.pwd);
		strcpy(ipcam.name, camera.name);
		strcpy(ipcam.uuid, camera.uuid);
		struct in_addr host;
		host.s_addr = camera.dwIp;

		printf("%s nvr ip: %s, channel_no: %d\n", \
			__func__, inet_ntoa(host), ipcam.channel_no);
		BizSetPara(&bizTar, (void *)&ipcam);
	}
	

	return TRUE;
}


VD_BOOL CPageIPCameraConfigFrameWork::AddIPC(ipc_unit &camera)
{
	int i=0;
	//csp modify 20140423
	for(i=0;i<GetMaxChnNum();i++)
	{
		SBizIPCameraPara ipcam;
		memset(&ipcam, 0, sizeof(ipcam));
		ipcam.channel_no = i;
		
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
		bizTar.nChn = ipcam.channel_no;
		BizGetPara(&bizTar, (void *)&ipcam);
		
		if(ipcam.enable)
		{
			if((ipcam.protocol_type == camera.protocol_type) && (ipcam.dwIp == camera.dwIp))
			{
				if(ipcam.wPort != camera.wPort)
				{
					printf("yg addipc enable\n");
					ipcam.channel_no = i;
					ipcam.enable = 1;
					ipcam.protocol_type = camera.protocol_type;
					ipcam.trans_type = camera.trans_type;
					ipcam.stream_type = camera.stream_type;
					ipcam.ipc_type = camera.ipc_type;
					ipcam.dwIp = camera.dwIp;
					ipcam.wPort = camera.wPort;
					strcpy(ipcam.user, camera.user);
					strcpy(ipcam.pwd, camera.pwd);
					strcpy(ipcam.name, camera.name);
					strcpy(ipcam.uuid, camera.uuid);
					struct in_addr host;
					host.s_addr = camera.dwIp;
					sprintf(ipcam.address, "http://%s:%d/onvif/device_service", inet_ntoa(host), camera.wPort);
					//yaogang modify 20141010
					camera.main_audio_sw = ipcam.main_audio_sw;
					camera.sub_audio_sw = ipcam.sub_audio_sw;
					
					BizSetPara(&bizTar, (void *)&ipcam);
				}
				
				return TRUE;
			}
		}
	}
	for(i=0;i<GetMaxChnNum();i++)
	{
		SBizIPCameraPara ipcam;
		memset(&ipcam, 0, sizeof(ipcam));
		ipcam.channel_no = i;
		
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
		bizTar.nChn = ipcam.channel_no;
		BizGetPara(&bizTar, (void *)&ipcam);
		
		if(!ipcam.enable)
		{
			printf("yg addipc unable\n");
			if (PRO_TYPE_NVR == camera.protocol_type)
			{
				return AddIPC_nvr(i, camera);
			}
			
			ipcam.channel_no = i;
			ipcam.enable = 1;
			ipcam.protocol_type = camera.protocol_type;
			ipcam.trans_type = camera.trans_type;
			ipcam.stream_type = camera.stream_type;
			ipcam.ipc_type = camera.ipc_type;
			ipcam.dwIp = camera.dwIp;
			ipcam.wPort = camera.wPort;
			strcpy(ipcam.user, camera.user);
			strcpy(ipcam.pwd, camera.pwd);
			strcpy(ipcam.name, camera.name);
			strcpy(ipcam.uuid, camera.uuid);
			struct in_addr host;
			host.s_addr = camera.dwIp;
			sprintf(ipcam.address, "http://%s:%d/onvif/device_service", inet_ntoa(host), camera.wPort);

			//yaogang modify 20141010
			camera.main_audio_sw = ipcam.main_audio_sw;
			camera.sub_audio_sw = ipcam.sub_audio_sw;
			
			BizSetPara(&bizTar, (void *)&ipcam);
			
			
			return TRUE;
		}
	}
	
	return FALSE;
}

void CPageIPCameraConfigFrameWork::GetChannelList()
{
	ipc_node *head = NULL;
	ipc_node *tail = NULL;
	
	if(m_channel_list)
	{
		IPC_Free(m_channel_list);
		m_channel_list = NULL;
	}
	
	int i=0;
	for(i=0;i<GetMaxChnNum();i++)
	{
		SBizIPCameraPara ipcam;
		memset(&ipcam, 0, sizeof(ipcam));
		ipcam.channel_no = i;
		
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
		bizTar.nChn = ipcam.channel_no;
		BizGetPara(&bizTar, (void *)&ipcam);
		
		if(ipcam.enable)
		{
			ipc_node *pNode = (ipc_node *)malloc(sizeof(ipc_node));
			if(pNode == NULL)
			{
				printf("Not enough space to save new ipc info.\n");
				if(head)
				{
					IPC_Free(head);
					head = NULL;
				}
				break;
			}
			
			memset(pNode, 0, sizeof(*pNode));
			pNode->next = NULL;
			
			pNode->ipcam.channel_no = i;
			pNode->ipcam.enable = ipcam.enable;
			pNode->ipcam.protocol_type = ipcam.protocol_type;
			pNode->ipcam.trans_type = ipcam.trans_type;
			pNode->ipcam.stream_type = ipcam.stream_type;
			pNode->ipcam.ipc_type = ipcam.ipc_type;
			pNode->ipcam.dwIp = ipcam.dwIp;
			pNode->ipcam.wPort = ipcam.wPort;
			strcpy(pNode->ipcam.user, ipcam.user);
			strcpy(pNode->ipcam.pwd, ipcam.pwd);
			strcpy(pNode->ipcam.name, ipcam.name);
			strcpy(pNode->ipcam.uuid, ipcam.uuid);
			struct in_addr host;
			host.s_addr = ipcam.dwIp;
			sprintf(pNode->ipcam.address, "http://%s:%d/onvif/device_service", inet_ntoa(host), ipcam.wPort);
			
			if(head == NULL)
			{
				head = pNode;
				tail = pNode;
			}
			else
			{
				tail->next = pNode;
				tail = pNode;
			}
		}
	}
	
	m_channel_list = head;
	
	int count = 0;
	ipc_node *p = m_channel_list;
	while(p)
	{
		if(count < m_rows2 - 1)
		{
			char szInfo[64];
			sprintf(szInfo, "%d", p->ipcam.channel_no+1);
			pStatic0[m_offset2+count*m_cols2+0]->SetText(szInfo);
			struct in_addr host;
			host.s_addr = p->ipcam.dwIp;
			strcpy(szInfo, inet_ntoa(host));
			pStatic0[m_offset2+count*m_cols2+1]->SetText(szInfo);
			sprintf(szInfo, "%d", p->ipcam.wPort);
			pStatic0[m_offset2+count*m_cols2+2]->SetText(szInfo);
			if(p->ipcam.protocol_type == PRO_TYPE_ONVIF)
			{
				strcpy(szInfo, "ONVIF");
			}
			/*
			else if(p->ipcam.protocol_type == PRO_TYPE_HAIXIN)
			{
				strcpy(szInfo, "I13");
			}
			else if(p->ipcam.protocol_type == PRO_TYPE_JUAN)
			{
				strcpy(szInfo, "I99");
			}
			*/
			else if(p->ipcam.protocol_type == PRO_TYPE_KLW)
			{
				//strcpy(szInfo, "I36");
				//add by liu
				#ifdef YUETIAN_PROTOCOL
					strcpy(szInfo, "&CfgPtn.YueTian");
				#else
					strcpy(szInfo, "I36");
				#endif
			}
			else if(p->ipcam.protocol_type == PRO_TYPE_XM)
			{
				strcpy(szInfo, "&CfgPtn.YueTianHW");
			}
			else if(p->ipcam.protocol_type == PRO_TYPE_NVR)
			{
				strcpy(szInfo, "NVR");
			}
		#ifdef IPC_PROTOCOL_TEST
			else if(p->ipcam.protocol_type == PRO_TYPE_IPC_TEST)
			{
				strcpy(szInfo, "IpcTest");
			}
		#endif
			/*
			else if(p->ipcam.protocol_type == PRO_TYPE_FULLHAN)
			{
				strcpy(szInfo, "FH8610");
			}
			else if(p->ipcam.protocol_type == PRO_TYPE_ANNI)
			{
				strcpy(szInfo, "ANNI");
			}
			else if(p->ipcam.protocol_type == PRO_TYPE_RIVER)
			{
				strcpy(szInfo, "RIVER");
			}
			*/
			else
			{
				memset(szInfo, 0, sizeof(szInfo));
			}
			pStatic0[m_offset2+count*m_cols2+3]->SetText(szInfo);
			//sprintf(szInfo, "%s", IPC_GetLinkStatus(p->ipcam.channel_no)?"Online":"Offline");
			//pStatic0[m_offset2+count*m_cols2+4]->SetText(szInfo);
			pButton0[8+count]->Show(TRUE);
			pCheckBox0[5+count]->Show(TRUE);
			pCheckBox0[5+count]->SetValue(0);
		}
		count++;
		p = p->next;
	}
	
	if(count < m_rows2 - 1)
	{
		for(int j = count; j < m_rows2 - 1; j++)
		{
			for(int i = 0; i < m_cols2; i++)
			{
				if(pStatic0[m_offset2+j*m_cols2+i] != NULL)
				{
					pStatic0[m_offset2+j*m_cols2+i]->SetText(NULL);
					pStatic0[m_offset2+j*m_cols2+i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
				}
			}
			
			pCheckBox0[5+j]->Show(FALSE);
			pButton0[8+j]->Show(FALSE);
			pCheckBox0[5+j]->SetValue(0);
		}
	}
	
	int pages = (count+m_rows2-1-1)/(m_rows2-1);
	if(pages < 1)
	{
		pages = 1;
	}
	
	pScrollbar0[1]->SetRange(0,pages-1,FALSE);
	pScrollbar0[1]->SetPage(1,FALSE);
	pScrollbar0[1]->SetPos(0,TRUE);
	
	m_nPageIndex2 = 0;
	m_nCurMouseLine2 = -1;
}

VD_BOOL CPageIPCameraConfigFrameWork::UpdateData(UDM mode)
{
	if(UDM_OPEN == mode)
	{
		//printf("CPageIPCameraConfigFrameWork::UpdateData-UDM_OPEN\n");
		
		if(m_ipc_head)
		{
			IPC_Free(m_ipc_head);
			m_ipc_head = NULL;
		}
		
		pScrollbar0[0]->SetRange(0,0,FALSE);
		pScrollbar0[0]->SetPage(1,FALSE);
		pScrollbar0[0]->SetPos(0,TRUE);
		
		m_nPageIndex = 0;
		m_nCurMouseLine = -1;
		
		for(int j = 0; j < m_rows - 1; j++)
		{
			for(int i = 0; i < m_cols; i++)
			{
				if(pStatic0[m_offset+j*m_cols+i] != NULL)
				{
					pStatic0[m_offset+j*m_cols+i]->SetText(NULL);
					pStatic0[m_offset+j*m_cols+i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
				}
			}
			
			pCheckBox0[j]->Show(FALSE);
			pButton0[j]->Show(FALSE);
			pCheckBox0[j]->SetValue(0);
		}
		
		GetChannelList();
		
		pButton0[16]->Show(FALSE);
		//pButton0[17]->Show(FALSE);
		pButton0[18]->Show(FALSE);
		pButton0[19]->Show(FALSE);
	}
	else if(UDM_OPENED == mode)
	{
		//printf("CPageIPCameraConfigFrameWork::UpdateData-UDM_OPENED\n");
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
	case XM_LBUTTONDOWN:
	//case XM_MOUSEMOVE:
		{
			if(CPageFrame::MsgProc(msg, wpa, lpa))
			{
				//printf("default proc success\n");
				return TRUE;
			}
			else
			{
				//printf("default proc failed\n");
			}
			
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);
			
			VD_RECT rtScreen;
			m_pDevGraphics->GetRect(&rtScreen);
			int offsetx = (rtScreen.right - m_Rect.Width())/2;
			int offsety = (rtScreen.bottom - m_Rect.Height())/2;
			
			BOOL bFind = FALSE;
			CTableBox *table = pTable0[0];
			int i = 0;
			for(i = 0; i < m_rows-1; i++)
			{
				//for(int kk = 0; kk < m_cols; kk++)
				for(int kk = 0; kk <= 3; kk++)
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
				if(XM_LBUTTONDBLCLK == msg)
				{
					int idx = 0;
					ipc_node *p = m_ipc_head;
					while(p)
					{
						if(idx == m_nPageIndex*(m_rows-1)+i)
						{
							if(AddIPC(p->ipcam))
							{
								GetChannelList();
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
	default:
		break;
	}
	
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
			if(pStatic0[m_offset+m_nCurMouseLine*m_cols+i] != NULL)
			{
				pStatic0[m_offset+m_nCurMouseLine*m_cols+i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
			}
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
		if(pStatic0[m_offset+m_nCurMouseLine*m_cols+i] != NULL)
		{
			pStatic0[m_offset+m_nCurMouseLine*m_cols+i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
		}
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
	memset(pEdit0, 0, sizeof(pEdit0));//csp modify 20140411
	
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
		"&CfgPtn.AlternateDNSServer",
		"&CfgPtn.Username",
		"&CfgPtn.Password"
	};
	
	int count = sizeof(statictext)/sizeof(statictext[0]);
	
	CRect tmpRt(80, 8, 80+160, 8+32);
	
	int i = 0;
	for(i = 0; i < count; i++)
	{
		tmpRt.top += 40;
		tmpRt.bottom += 40;
		
		//csp modify 20140411
		if(i == 5)
		{
			tmpRt.top -= 40;
			tmpRt.bottom -= 40;
		}
		
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
		
		//csp modify 20140411
		if(i == 5)
		{
			tmpRt.top -= 40;
			tmpRt.bottom -= 40;
		}
		
		//csp modify 20140411
		if(i < 5)
		{
			pIpCtrl0[i] = CreateIPAddressCtrl(CRect(tmpRt.left, tmpRt.top, tmpRt.right, tmpRt.bottom), this);
			IPADDR sIPIns;
			sIPIns.l = 0;//inet_addr("192.168.1.88");
			pIpCtrl0[i]->SetIPAddress(&sIPIns);
		}
		else if(i == 5)
		{
			pEdit0[i] = CreateEdit(CRect(tmpRt.left, tmpRt.top, tmpRt.right, tmpRt.bottom), this, 16, 0, NULL);
		}
		else
		{
			pEdit0[i] = CreateEdit(CRect(tmpRt.left, tmpRt.top, tmpRt.right, tmpRt.bottom), this, 16, editPassword, NULL);
		}
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
		
		//csp modify 20140411
		pEdit0[5]->SetText(ipcam.user);
		pEdit0[6]->SetText(ipcam.pwd);
		
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
			
			//csp modify 20140411
			pEdit0[5]->GetText(ipcam.user,sizeof(ipcam.user));
			pEdit0[6]->GetText(ipcam.pwd,sizeof(ipcam.pwd));
			
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

CPageIPCameraSetChnFrameWork::CPageIPCameraSetChnFrameWork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
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
	
	m_chn = 0;
	m_add_flag = 0;
	
	InitPage0();
}

CPageIPCameraSetChnFrameWork::~CPageIPCameraSetChnFrameWork()
{
	
}

VD_PCSTR CPageIPCameraSetChnFrameWork::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

VD_BOOL CPageIPCameraSetChnFrameWork::SetChn(int chn, unsigned char add)
{
	m_chn = chn;
	m_add_flag = add;
	
	if(m_add_flag)
	{
		pCombox0[0]->Enable(TRUE);
	}
	else
	{
		pCombox0[0]->Enable(FALSE);
	}
	
	pCombox0[0]->SetCurSel(chn);
	
	OnComboBox0();
	
	return TRUE;
}

VD_BOOL CPageIPCameraSetChnFrameWork::UpdateData(UDM mode)
{
	if(UDM_OPEN == mode)
	{
		//printf("CPageIPCameraSetChnFrameWork::open\n");
	}
	else if(UDM_OPENED == mode)
	{
		//printf("CPageIPCameraSetChnFrameWork::opened\n");
	}
	else if(UDM_CLOSED == mode)
	{
		//printf("CPageIPCameraSetChnFrameWork::exit\n");
	}
	return TRUE;
}

VD_BOOL CPageIPCameraSetChnFrameWork::MsgProc( uint msg, uint wpa, uint lpa )
{
	return CPageFrame::MsgProc(msg, wpa, lpa);
}

void CPageIPCameraSetChnFrameWork::InitPage0()
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
	
	CRect rtFrame0(LEFT_PAGE_L, 10, LEFT_PAGE_R, 330);
	pTabFrame0[0] = CreateTableBox(rtFrame0,this,1,1);
	pTabFrame0[0]->SetFrameColor(VD_RGB(56,108,170));
	
	CRect rtTable0(rtFrame0.left+4,rtFrame0.top+5,rtFrame0.right-4,rtFrame0.bottom-5);
	pTable0[0] = CreateTableBox(rtTable0,this,4,10);
	pTable0[0]->SetColWidth(0,84);
	pTable0[0]->SetColWidth(1,244);
	pTable0[0]->SetColWidth(2,84);
	
	int i = 0;
	for(i = 0; i < count; i++)//0-7
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
								rtTable0.left+tmpRt.right-1, rtTable0.top+tmpRt.bottom-1), this, NULL, NULL, (CTRLPROC)&CPageIPCameraSetChnFrameWork::OnComboBox0, 0);
	int j = 0;
	for(j = 0; j < GetMaxChnNum(); j++)
	{
		char chninfo[4];
		sprintf(chninfo,"%d",j+1);
		pCombox0[0]->AddString(chninfo);
	}
	printf("yg init pCombox0[0] num: %d\n", pCombox0[0]->GetCount());
	pCombox0[0]->SetCurSel(0);
	
	pTable0[0]->GetTableRect(3, 0, &tmpRt);
	pCheckBox0[0] = CreateCheckBox(CRect(rtTable0.left+tmpRt.left+0, rtTable0.top+tmpRt.top+4, 
									rtTable0.left+tmpRt.left+20, rtTable0.top+tmpRt.top+22), this);
	
	pTable0[0]->GetTableRect(1, 1, &tmpRt);
	pCombox0[1] = CreateComboBox(CRect(rtTable0.left+tmpRt.left+1, rtTable0.top+tmpRt.top+1, 
								rtTable0.left+tmpRt.right-1, rtTable0.top+tmpRt.bottom-1), this, NULL, NULL, (CTRLPROC)&CPageIPCameraSetChnFrameWork::OnComboBox1, 0);
	pCombox0[1]->AddString("ONVIF");
	//pCombox0[1]->AddString("I13");
	//pCombox0[1]->AddString("I99");
	
	//pCombox0[1]->AddString("I36");
	//add by liu
	#ifdef YUETIAN_PROTOCOL
		pCombox0[1]->AddString("&CfgPtn.YueTian");
	#else
		pCombox0[1]->AddString("I36");
	#endif
	pCombox0[1]->AddString("&CfgPtn.YueTianHW");
	//pCombox0[1]->AddString("FH8610");
	//pCombox0[1]->AddString("ANNI");
	//pCombox0[1]->AddString("RIVER");
	pCombox0[1]->SetCurSel(0);
	
	pTable0[0]->GetTableRect(3, 1, &tmpRt);
	pCombox0[2] = CreateComboBox(CRect(rtTable0.left+tmpRt.left+1, rtTable0.top+tmpRt.top+1, 
								rtTable0.left+tmpRt.right-1, rtTable0.top+tmpRt.bottom-1), this, NULL, NULL, NULL, 0);
	pCombox0[2]->AddString("TCP");
	//pCombox0[2]->AddString("UDP");
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

	//yaogang modify 20140918
	/*
	private_yuetian   init
	主码流
	码流控制ComboBox: 768/1024/1536/2048/3072/4096/5120/6144/8192
	码流类型ComboBox: 固定码流/可变码流
	子码流
	码流控制ComboBox: 768/1024/1536/2048/3072/4096/5120/6144/8192
	码流类型ComboBox: 固定码流/可变码流
	*/
	const char *private_yuetian[]=
	{
		"&CfgPtn.MainStream",
		"&CfgPtn.SubStream",
		"&CfgPtn.BitRateCtl",
		"&CfgPtn.BitRateCtl",
		"&CfgPtn.BitRateType",
		"&CfgPtn.BitRateType",
		"&CfgPtn.Audio",
		//"&CfgPtn.Audio",
		"&CfgPtn.IPCReBoot",
	};
	/*
	const char *main_bit_rate[] = 
	{
		"&CfgPtn.Brt768",   //768Kb
		"&CfgPtn.Brt1024", //1024Kb
		"&CfgPtn.Brt1536", //1536Kb
		"&CfgPtn.Brt2048", //2048Kb
		"&CfgPtn.Brt3072", //3072Kb
		"&CfgPtn.Brt4096", //4096Kb
		"&CfgPtn.Brt5120", //5120Kb
		"&CfgPtn.Brt6144", //6144Kb
		"&CfgPtn.Brt8192", //8192Kb
	};
	const char *sub_bit_rate[] = 
	{
		"&CfgPtn.Brt256", //256Kb
		"&CfgPtn.Brt384", //384Kb
		"&CfgPtn.Brt512", //512Kb
		"&CfgPtn.Brt768", //768Kb
		"&CfgPtn.Brt1024", //1024Kb
	};
	*/
	SValue bitRateList[15];
	int nBrReal;
	GetBitrateList(bitRateList, &nBrReal, 15);
	
	const char *bit_rate_type[] = 
	{
		"&CfgPtn.ConstantBitRate", //固定码流
		"&CfgPtn.VariableBitRate", //可变码流
	};
	
	int count1 = sizeof(private_yuetian)/sizeof(private_yuetian[0]);

	for(i = 0; i < count1; i++)//8    8-15
	{
		if(i&1)
		{
			pTable0[0]->GetTableRect(2, (i+count)/2, &tmpRt);
		}
		else
		{
			pTable0[0]->GetTableRect(0,  (i+count)/2, &tmpRt);
		}
		pStatic0[i+count] = CreateStatic(CRect(rtTable0.left+tmpRt.left, rtTable0.top+tmpRt.top, 
									rtTable0.left+tmpRt.right, rtTable0.top+tmpRt.bottom), 
									this, private_yuetian[i]);
		pStatic0[i+count]->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
		pStatic0[i+count]->Enable(TRUE);
	}
	//main bit rate
	pTable0[0]->GetTableRect(1, 5, &tmpRt);
	pCombox0[3] = CreateComboBox(CRect(rtTable0.left+tmpRt.left+1, rtTable0.top+tmpRt.top+1,  \
								rtTable0.left+tmpRt.right-1, rtTable0.top+tmpRt.bottom-1), this, NULL, NULL, NULL, 0);
	/*
	for (i=0; i < sizeof(main_bit_rate)/sizeof(main_bit_rate[0]); i++)
	{
		pCombox0[3]->AddString(main_bit_rate[i]);
	}
	pCombox0[3]->SetCurSel(5);//4096Kb
	*/
	for (i=0; i < nBrReal; i++)
	{
		pCombox0[3]->AddString(bitRateList[i].strDisplay);
	}
	pCombox0[3]->SetCurSel(8);//4096Kb
	
	pTable0[0]->GetTableRect(1, 6, &tmpRt);
	pCombox0[4] = CreateComboBox(CRect(rtTable0.left+tmpRt.left+1, rtTable0.top+tmpRt.top+1,  \
								rtTable0.left+tmpRt.right-1, rtTable0.top+tmpRt.bottom-1), this, NULL, NULL, NULL, 0);
	for (i=0; i < sizeof(bit_rate_type)/sizeof(bit_rate_type[0]); i++)
	{
		pCombox0[4]->AddString(bit_rate_type[i]);
	}
	pCombox0[4]->SetCurSel(0);

	//sub bit rate
	pTable0[0]->GetTableRect(3, 5, &tmpRt);
	pCombox0[5] = CreateComboBox(CRect(rtTable0.left+tmpRt.left+1, rtTable0.top+tmpRt.top+1,  \
								rtTable0.left+tmpRt.right-1, rtTable0.top+tmpRt.bottom-1), this, NULL, NULL, NULL, 0);

	/*
	for (i=0; i < sizeof(sub_bit_rate)/sizeof(sub_bit_rate[0]); i++)
	{
		pCombox0[5]->AddString(sub_bit_rate[i]);
	}
	pCombox0[5]->SetCurSel(3);//768Kb
	*/
	for (i=0; i < nBrReal; i++)
	{
		pCombox0[5]->AddString(bitRateList[i].strDisplay);
	}
	pCombox0[5]->SetCurSel(3);//4096Kb
	
	pTable0[0]->GetTableRect(3, 6, &tmpRt);
	pCombox0[6] = CreateComboBox(CRect(rtTable0.left+tmpRt.left+1, rtTable0.top+tmpRt.top+1,  \
								rtTable0.left+tmpRt.right-1, rtTable0.top+tmpRt.bottom-1), this, NULL, NULL, NULL, 0);
	for (i=0; i < sizeof(bit_rate_type)/sizeof(bit_rate_type[0]); i++)
	{
		pCombox0[6]->AddString(bit_rate_type[i]);
	}
	pCombox0[6]->SetCurSel(0);

	//reboot IP
	pTable0[0]->GetTableRect(3, 7, &tmpRt);
	pCheckBox0[1] = CreateCheckBox(CRect(rtTable0.left+tmpRt.left+0, rtTable0.top+tmpRt.top+4, \
									rtTable0.left+tmpRt.left+20, rtTable0.top+tmpRt.top+22), this);
	pCheckBox0[1]->SetValue(0);
	//audio switch
	pTable0[0]->GetTableRect(1, 7, &tmpRt);
	pCheckBox0[2] = CreateCheckBox(CRect(rtTable0.left+tmpRt.left+0, rtTable0.top+tmpRt.top+4, \
									rtTable0.left+tmpRt.left+20, rtTable0.top+tmpRt.top+22), this);
	//音频开关变更将导致IPC重启
	printf("yg count: %d, count1: %d\n", count, count1);
	const char *yuetian_caution = "&CfgPtn.caution_rstIPC";
	pStatic0[count+count1] = CreateStatic(CRect(rtTable0.left+tmpRt.left+24, rtTable0.top+tmpRt.top, 
									rtTable0.left+tmpRt.right, rtTable0.top+tmpRt.bottom), 
									this, yuetian_caution);
	pStatic0[count+count1]->SetTextAlign(VD_TA_LEFT|VD_TA_YCENTER);
	pStatic0[count+count1]->Enable(TRUE);
	/*
	pTable0[0]->GetTableRect(3, 7, &tmpRt);
	pCheckBox0[3] = CreateCheckBox(CRect(rtTable0.left+tmpRt.left+0, rtTable0.top+tmpRt.top+4, \
									rtTable0.left+tmpRt.left+20, rtTable0.top+tmpRt.top+22), this);
	*/
	//yaogang modify 20140918 end
	
	CRect rt(420, 328+4, 420+96, 366-4);
	
	rt.left = 420;
	rt.right = rt.left+96;
	pButton0[0] = CreateButton(rt, this, "&CfgPtn.apply", (CTRLPROC)&CPageIPCameraSetChnFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[0]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
	
	rt.left = rt.right+4;
	rt.right = rt.left+96;
	pButton0[1] = CreateButton(rt, this, "&CfgPtn.Exit", (CTRLPROC)&CPageIPCameraSetChnFrameWork::OnClickSubPage, NULL, 0, FALSE);
	pButton0[1]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
}

void CPageIPCameraSetChnFrameWork::OnClickSubPage()
{
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
	
	if(!bFind)
	{
		return;
	}
	
	switch(i)
	{
	case 0://应用
		{
			unsigned int protocol = 0;//界面选择的协议
			SBizIPCameraPara ipcam;
			memset(&ipcam, 0, sizeof(ipcam));
			ipcam.channel_no = pCombox0[0]->GetCurSel();
			
			SBizParaTarget bizTar;
			bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
			bizTar.nChn = ipcam.channel_no;
			BizGetPara(&bizTar, (void *)&ipcam);

			if (ipcam.protocol_type == PRO_TYPE_NVR)
			{
				break;
			}
			
			//printf("yg OnClickSubPage ipcam.uuid: %s\n", ipcam.uuid);
			
			//只能是已经用跃天协议连接的IPC才能继续操作
			if (ipcam.enable && (ipcam.protocol_type == PRO_TYPE_KLW))
			{
				//yaogang modify 20140918
				//set venc 主码流控制带宽
				int ret;
				venc_parameter_t venc_para;
				memset(&venc_para, 0, sizeof(venc_para));
				ret = BizConfigGetParaListValue(EM_BIZ_CFG_PARALIST_BITRATE, \
						pCombox0[3]->GetCurSel(), (s32 *)&venc_para.bit_rate);
				if (ret != 0)
				{
					printf("yg OnClickSubPage BizConfigGetParaListValue() failed, \
							pCombox0[3] index: %d\n", pCombox0[3]->GetCurSel());
					break;
				}
				//printf("venc_para.bit_rate : %d\n", venc_para.bit_rate);
				venc_para.is_cbr = pCombox0[4]->GetCurSel()+1;
				//printf("venc_para.is_cbr : %d\n", venc_para.is_cbr);
				venc_para.frame_rate = 25;//KLW协议没有用到，在这里只是让调用通过
				tl_venc_set_para(ipcam.channel_no, 0, &venc_para);


				//次码流控制带宽
				memset(&venc_para, 0, sizeof(venc_para));
				BizConfigGetParaListValue(EM_BIZ_CFG_PARALIST_BITRATE, \
						pCombox0[5]->GetCurSel(), (s32*)&venc_para.bit_rate);
				if (ret != 0)
				{
					printf("yg OnClickSubPage BizConfigGetParaListValue() failed, \
							pCombox0[5] index: %d\n", pCombox0[3]->GetCurSel());
					break;
				}
				//printf("venc_para.bit_rate : %d\n", venc_para.bit_rate);
				venc_para.is_cbr = pCombox0[6]->GetCurSel()+1;
				//printf("venc_para.is_cbr : %d\n", venc_para.is_cbr);
				venc_para.frame_rate = 25;
				tl_venc_set_para(ipcam.channel_no, 1, &venc_para);

				//音频开关
				int bswitch = pCheckBox0[2]->GetValue() ? 1:0;
				printf("yg bswitch: %d\n", bswitch);
				IPC_CMD_SetAudioSwitchStatus(ipcam.channel_no, bswitch);

				
				//reboot ipc
				if (pCheckBox0[1]->GetValue())
				{
					
					IPC_Stop(ipcam.channel_no);
					IPC_Stop(ipcam.channel_no+16);
					IPC_CMD_Reboot(ipcam.channel_no);
					IPC_Start(ipcam.channel_no);
					IPC_Start(ipcam.channel_no+16);

					break;
				}
				
			}

			if(pCombox0[1]->GetCurSel() == 0)
			{
				protocol = PRO_TYPE_ONVIF;
			}			
			else if(pCombox0[1]->GetCurSel() == 1)
			{
				//ipcam.protocol_type = PRO_TYPE_KLW;
				protocol = PRO_TYPE_KLW;
			}
			else
			{
				protocol = PRO_TYPE_XM;
			}

			ipcam.enable = pCheckBox0[0]->GetValue() ? 1:0;
			ipcam.protocol_type = protocol;
			ipcam.trans_type = pCombox0[2]->GetCurSel()? /*TRANS_TYPE_UDP*/TRANS_TYPE_TCP:TRANS_TYPE_TCP;
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
			
			bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
			bizTar.nChn = ipcam.channel_no;

			BizSetPara(&bizTar, (void *)&ipcam);

			//printf("yg OnClickSubPage ipcam.uuid: %s\n", ipcam.uuid);
			
			CPageIPCameraConfigFrameWork *pPG = (CPageIPCameraConfigFrameWork *)m_pParent;
			pPG->GetChannelList();
			
		}
		break;
	case 1://退出
		{
			this->Close();
		}
		break;
	default:
		break;
	}
}

void CPageIPCameraSetChnFrameWork::OnComboBox0()
{
	SBizIPCameraPara ipcam;
	memset(&ipcam, 0, sizeof(ipcam));
	ipcam.channel_no = pCombox0[0]->GetCurSel();

	//printf("yg OnComboBox0: GetCurSel: %d\n", pCombox0[0]->GetCurSel());
	
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
	bizTar.nChn = ipcam.channel_no;
	BizGetPara(&bizTar, (void *)&ipcam);
	
	pCheckBox0[0]->SetValue(ipcam.enable);
	if(ipcam.protocol_type == PRO_TYPE_ONVIF)
	{
		pCombox0[1]->SetCurSel(0);
	}/*
	else if(ipcam.protocol_type == PRO_TYPE_HAIXIN)
	{
		pCombox0[1]->SetCurSel(1);
	}
	else if(ipcam.protocol_type == PRO_TYPE_JUAN)
	{
		pCombox0[1]->SetCurSel(2);
	}
	*/
	else if(ipcam.protocol_type == PRO_TYPE_KLW)
	{
		pCombox0[1]->SetCurSel(1);
	}
	else if(ipcam.protocol_type == PRO_TYPE_XM)
	{
		pCombox0[1]->SetCurSel(2);
	}
	else if(ipcam.protocol_type == PRO_TYPE_NVR)
	{
		pCombox0[1]->SetCurSel(3);
	}
	/*
	else if(ipcam.protocol_type == PRO_TYPE_FULLHAN)
	{
		pCombox0[1]->SetCurSel(4);
	}
	else if(ipcam.protocol_type == PRO_TYPE_ANNI)
	{
		pCombox0[1]->SetCurSel(5);
	}
	else if(ipcam.protocol_type == PRO_TYPE_RIVER)
	{
		pCombox0[1]->SetCurSel(6);
	}
	*/
	else
	{
		pCombox0[1]->SetCurSel(0);
	}
	pCombox0[2]->SetCurSel(0/*(ipcam.trans_type == TRANS_TYPE_TCP)?0:1*/);
	IPADDR sIPIns;
	sIPIns.l = ipcam.dwIp;
	pIpCtrl0[0]->SetIPAddress(&sIPIns);
	char szInfo[32];
	sprintf(szInfo, "%d", ipcam.wPort);
	pEdit0[0]->SetText(szInfo);
	pEdit0[1]->SetText(ipcam.user);
	pEdit0[2]->SetText(ipcam.pwd);

	//yaogang modify 20140918 
	OnComboBox1();

	//yaogang modify 20140918 end
}

//yaogang modify 20140918
void CPageIPCameraSetChnFrameWork::OnComboBox1()
{
	int i;
	int rtn;
	u8 index;
	SBizEncodePara bizEncPara;
	SBizParaTarget sParaTgtIns;
	venc_parameter_t venc_para;
	unsigned int protocol = 0;//界面选择的协议
	
	int chn = pCombox0[0]->GetCurSel();

	
	for (i=8; i<17; i++)//8-15
	{
		pStatic0[i]->Show(FALSE);
		pStatic0[i]->Enable(TRUE);
	}
	
	for (i=3; i<7; i++)
	{
		pCombox0[i]->Show(FALSE);
	}
	pCheckBox0[1]->SetValue(0);
	pCheckBox0[1]->Show(FALSE);//IPC REBOOT
	pCheckBox0[2]->Show(FALSE);//MAIN AUDIO SWITCH
	//pCheckBox0[3]->Show(FALSE);//SUB AUDIO SWITCH
/*
	if (m_add_flag)//添加通道时不显示设置码流
	{
		//printf("m_add_flag  add channel\n");
		return;
	}
*/
	SBizParaTarget bizTar;
	SBizIPCameraPara ipcam;
	memset(&ipcam, 0, sizeof(ipcam));
	ipcam.channel_no = chn;
	bizTar.emBizParaType = EM_BIZ_IPCAMERAPARA;
	bizTar.nChn = pCombox0[0]->GetCurSel();
	BizGetPara(&bizTar, (void *)&ipcam);

	//设置未连接通道时，直接返回
	if (ipcam.enable == 0)
	{
		printf("CPageIPCameraSetChnFrameWork-ipcam.enable == 0\n");
		return;
	}
	
	//设置已连接通道时，非跃天协议连接不显示
	if (ipcam.protocol_type != PRO_TYPE_KLW)
	{
		printf("CPageIPCameraSetChnFrameWork-ipcam.protocol_type != PRO_TYPE_KLW\n");
		return;
	}

	if (pCombox0[1]->GetCurSel() == 1)
	{
		protocol = PRO_TYPE_KLW;
	}
	
	//Combox选择非跃天协议时不显示
	if (protocol != PRO_TYPE_KLW)
	{
		printf("CPageIPCameraSetChnFrameWork-protocol != PRO_TYPE_KLW\n");
		return;
	}
	
	int stream;//0: 主码流1: 子码流
	for (stream=0; stream<2; stream++)
	{
		rtn = tl_venc_get_para(chn, stream, &venc_para);
		if(0 == rtn)
		{
			rtn =BizConfigGetParaListIndex(EM_BIZ_CFG_PARALIST_BITRATE, \
				venc_para.bit_rate, &index);
			if (rtn == 0)
			{	
				pCombox0[3+2*stream]->SetCurSel(index);
				//printf("yg BitRate BizConfigGetParaListIndex: %d\n", index);
			}
			else
			{
				printf("yg chn%d BitRate BizConfigGetParaListIndex failed\n", chn+16*stream);
			}
				
			pCombox0[4+2*stream]->SetCurSel(venc_para.is_cbr-1);
		}
		else
		{
			pCombox0[3]->SetCurSel(8);//4096Kb
			pCombox0[4]->SetCurSel(0);
			pCombox0[5]->SetCurSel(3);//768Kb
			pCombox0[6]->SetCurSel(0);
			printf("yg chn%d tl_venc_get_para failed\n", chn);
		}
	}
	
	//show
	for (i=8; i<17; i++)
	{
		pStatic0[i]->Show(TRUE);
	}
	for (i=3; i<7; i++)
	{
		pCombox0[i]->Show(TRUE);
	}
	pCheckBox0[1]->Show(TRUE);
	//printf("yg main_audio_sw %d\n", ipcam.main_audio_sw);
	//pCheckBox0[2]->SetValue(ipcam.main_audio_sw);
	pCheckBox0[2]->SetValue(IPC_CMD_GetAudioSwitchStatus(ipcam.channel_no));
	pCheckBox0[2]->Show(TRUE);
	//printf("yg sub_audio_sw %d\n", ipcam.sub_audio_sw);
	//pCheckBox0[3]->SetValue(ipcam.sub_audio_sw);
	//pCheckBox0[3]->Show(TRUE);
	
	/*
	ipc_unit cam;
	if(IPC_Get(ipcam.channel_no, &cam) == 0)
	{
		printf("yg page main_audio_sw: %d\n", cam.main_audio_sw);
		printf("yg page sub_audio_sw: %d\n", cam.sub_audio_sw);
	}
	*/
	
		
}

