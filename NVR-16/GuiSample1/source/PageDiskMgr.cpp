#include "GUI/Pages/PageDiskMgr.h"
#include "biz.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/PagePlayBackFrameWork.h"//cw_remote

#include "mod_log.h"

//extern "C" int tl_power_atx_check();

#define DISABLE_DISK_STATE		// 20111017 by lanston
#define DISABLE_DISK_APPLY		// 20111017 by lanston

CPageDiskMgr::CPageDiskMgr( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPageFrame(pRect, psz, icon, pParent)
{
	bQuit = TRUE;
	
	SetMargin(0,0,0,0);
	
	m_dskdatalist.clear();//csp modify 20121018
	
	m_pDevGraphics = CDevGraphics::instance(m_screen);
	m_pDevGraphics->GetRect(&m_screenRT);
	
	//pDskInfoBuf = NULL; //zlb20111117 去掉malloc
	
	nDiskNum = 0;
	m_count = 0;
	m_nCursel = -1;
	int i=0;
	CRect rtTab;
	rtTab.left = 17;
	rtTab.top = 50;
	rtTab.right = m_Rect.Width()-17;
	rtTab.bottom = rtTab.top + 10*(CTRL_HEIGHT+2);
	
	#ifndef DISABLE_DISK_STATE
	pTab = CreateTableBox(rtTab, this, 7, 10);
	#else
	pTab = CreateTableBox(rtTab, this, 6, 10);
	#endif
	
	const char* szDskMgrTitle[] =
	{
		"&CfgPtn.ID",
		"&CfgPtn.Type",
		"&CfgPtn.DiskSize",
		"&CfgPtn.Free",
		"&CfgPtn.Status",
		#ifndef DISABLE_DISK_STATE
		"&CfgPtn.Properties",
		#endif
		"&CfgPtn.Source",
	};
	
	int szDskMgrTitleLen[] =
	{
		TEXT_WIDTH*2,//"&CfgPtn.ID",
		TEXT_WIDTH*2+TEXT_WIDTH,//"&CfgPtn.Type",
		TEXT_WIDTH*4+TEXT_WIDTH,//"&CfgPtn.Size",
		TEXT_WIDTH*4+TEXT_WIDTH,//"&CfgPtn.Free",
		TEXT_WIDTH*3+12,//"&CfgPtn.Status",
		#ifndef DISABLE_DISK_STATE
		TEXT_WIDTH*4,//"&CfgPtn.Properties",
		#endif
		TEXT_WIDTH*2-12//"&CfgPtn.Source",
	};
	
	nTabCol = sizeof(szDskMgrTitle)/sizeof(szDskMgrTitle[0]);
	
	for(i=0; i<nTabCol; i++)
	{
		if(i < nTabCol-1)
		{	
			pTab->SetColWidth(i, szDskMgrTitleLen[i]+6);
		}
		
		CRect rt;
		pTab->GetTableRect(i, 0, &rt);
		pDskMgrTitle[i] = CreateStatic(CRect(rt.left+rtTab.left, rt.top+rtTab.top, rt.right+rtTab.left, rt.bottom+rtTab.top), this, szDskMgrTitle[i]);
		pDskMgrTitle[i]->SetTextAlign(VD_TA_CENTER);
		pDskMgrTitle[i]->SetBkColor(VD_RGB(67,77,87));
	}
	
	for(i = 0; i < 9; i++)
	{
		for(int kk=0; kk<nTabCol; kk++)
		{
			CRect rt;
			pTab->GetTableRect(kk, i+1, &rt);
			#ifndef DISABLE_DISK_STATE
			if(5 == kk)
			{
				m_DskItems[i].pCombo = CreateComboBox(CRect(rt.left+rtTab.left, rt.top+rtTab.top, rt.right+rtTab.left, rt.bottom+rtTab.top), this, NULL);
				m_DskItems[i].pCombo->AddString("R/W");
				m_DskItems[i].pCombo->AddString("R");
				m_DskItems[i].pCombo->Show(FALSE);
			}
			else
			#endif
			{
				m_DskItems[i].pStatic[kk] = CreateStatic(CRect(rt.left+rtTab.left, rt.top+rtTab.top, rt.right+rtTab.left, rt.bottom+rtTab.top), this, "");
				m_DskItems[i].pStatic[kk]->SetTextAlign(VD_TA_CENTER);
				m_DskItems[i].pStatic[kk]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
				m_DskItems[i].pStatic[kk]->Show(FALSE);
			}
		}
	}
	
	const int btwidth = 80;
	
	percentInfo = CreateStatic(CRect(16, m_Rect.Height() - 62, m_Rect.Width()-16, m_Rect.Height() - 40), this, "");
	percentInfo->SetBkColor(VD_RGB(67,77,87));
	
	formatProg = CreateStatic(CRect(17, m_Rect.Height() - 61, m_Rect.Width()-17, m_Rect.Height() - 41), this, "");
	formatProg->SetBkColor(VD_RGB(8,66,189));
	formatProg->SetTextAlign(VD_TA_CENTER);
	formatProg->SetBkWidth(0);
	//formatProg->SetText("0 %");
	//formatProg->SetRect(CRect(17, m_Rect.Height() - 58, 300, m_Rect.Height() - 38),TRUE);
	//formatProg->Show(FALSE);
	
	stateInfo = CreateStatic(CRect(17, m_Rect.Height() - 38, m_Rect.Width() - 4*btwidth, m_Rect.Height() - 10), this, "");
	stateInfo->SetTextAlign(VD_TA_LEFT);
	//stateInfo->SetBkColor(VD_RGB(8,66,189));
	
	const char* szButton[] =
	{
		"&CfgPtn.Refresh",
		"&CfgPtn.Browse",
		"&CfgPtn.apply",
		"&CfgPtn.Format",
		"&CfgPtn.Exit",
		"&CfgPtn.Prev",
		"&CfgPtn.Next",
	};
	
	CRect rtBt;
#ifdef DISABLE_DISK_APPLY
	rtBt.left = m_Rect.Width()-17 - 3*btwidth - 2*6;
#else
	rtBt.left = m_Rect.Width()-17 - 5*btwidth - 4*6;
#endif
	rtBt.right = rtBt.left + btwidth;
	rtBt.top = m_Rect.Height() - 38;
	rtBt.bottom = rtBt.top + 24;
    
	for(i = 0; i < sizeof(szButton)/sizeof(szButton[0]) - 2; i++)
	{
#ifdef DISABLE_DISK_APPLY
		if(i == 1 || i == 2)
		{
			pButton[i] = NULL;
			continue;
		}
#endif
		
		pButton[i] = CreateButton(rtBt, this, szButton[i],(CTRLPROC)&CPageDiskMgr::OnClkButton);
		pButton[i]->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"));
		
		rtBt.left += btwidth+6;
		rtBt.right = rtBt.left + btwidth;
	}
	
    rtBt.left = 37;//(200*3);
    //rtBt.right = rtBt.left + 72;//btwidth;
    for(i = sizeof(szButton)/sizeof(szButton[0]) - 2; i < sizeof(szButton)/sizeof(szButton[0]); i++)
	{
        //printf("***%s\n", __FUNCTION__);
        rtBt.right = rtBt.left + 72;
		pButton[i] = CreateButton(rtBt, this, szButton[i],(CTRLPROC)&CPageDiskMgr::OnClkButton);
		pButton[i]->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"));
		
		rtBt.left = rtBt.right + 12;
		//rtBt.right = rtBt.left + 72;//btwidth;
	}
	
	m_pDskInfo = new CPageSysInfo(CRect(0, 0, 642, 418), "&CfgPtn.System", SIT_DSK, icon_dvr, this);
	
	SelectDskItem(0);
	
    m_IsLoader = 0;
}

CPageDiskMgr::~CPageDiskMgr()
{
	
}

int CPageDiskMgr::SetStateInfo(char* info)
{
	stateInfo->SetText(info);
}

int CPageDiskMgr::SetFormatProg(int nProg)
{
	//formatProg->Show(TRUE,TRUE);

	if(nProg == 0)
	{
		nProg = 1;
	}

	if(nProg == -1)
	{
		nProg = 0;
	}
	
	int maxWidth = m_Rect.Width()-34;
	int progWidth = maxWidth*nProg/100;

	/*
	CRect rt;
	rt.left = (m_screenRT.Width()-m_Rect.Width())/2+17;
	rt.top = (m_screenRT.Height()-m_Rect.Height())/2+m_Rect.Height()-58;
	rt.right = rt.left+progWidth;
	rt.bottom = rt.top+20;
	formatProg->SetRect(rt,TRUE);
	*/
	
	formatProg->SetBkWidth(progWidth);

	char szPercent[16] = {0};
	sprintf(szPercent,"%d %c",nProg,'%');
	formatProg->SetText(szPercent);

	if(nProg >= 100)
	{
		SetSystemLockStatus(0);
 		//MessageBox("&CfgPtn.FomatDone", "&CfgPtn.WARNING" , MB_OK|MB_ICONINFORMATION);
 		//m_DskItems[m_nCursel].pStatic[4]->SetText("OK");
	}
}

static bool lessmark(const DSKINFO& s1,const DSKINFO& s2)
{
	//return s1.index < s2.index;
	int d1 = atoi(s1.index.c_str());
	int d2 = atoi(s2.index.c_str());
	return d1 < d2;
}

//const char *pstr_disk_status[] = {"&CfgPtn.UNFORMAT","&CfgPtn.FORMATTING","&CfgPtn.INUSE","&CfgPtn.IDLE","&CfgPtn.SLEEP","&CfgPtn.ERROR"};
const char *pstr_disk_status[] = {"&CfgPtn.UNFORMAT","&CfgPtn.FORMATTING","&CfgPtn.INUSE","&CfgPtn.IDLE","&CfgPtn.IDLE","&CfgPtn.ERROR"};

void CPageDiskMgr::SetLineInfIns(DSKINFO* pInfo, uchar type, uint total, uint free, u8 status, char* pDevPath)
{
	char outinfo[32] = {0};
	char tmp[8] = {0};
	
	pInfo->type = (type == 0) ? "HDD" : "USB";
	
	printf("disk total %u\n", total);
	
	sprintf(outinfo, "%ld", total/MEGA);
	sprintf(tmp, ".%02dG", (total%MEGA)/TENKB);
	strcat(outinfo,tmp);	
	pInfo->capacity = outinfo;
	
	if(free == -1)
	{
		strcpy(outinfo, GetParsedString("&CfgPtn.checking"));
	}
	else
	{
		sprintf(outinfo, "%ld", free/MEGA);
		sprintf(tmp, ".%02dG", (free%MEGA/TENKB));
		strcat(outinfo,tmp);
	}
	pInfo->freespace = outinfo;
	
	//pInfo->state = status==2?"ERR":((status==0)?"OK":"RAW"));
	pInfo->state = pstr_disk_status[status];
	pInfo->attrib = 0;
	pInfo->source = pDevPath;	
}

void CPageDiskMgr::SetDskInfoLine(int idx, uchar type, uint total, uint free, char* pDevPath)
{
#if 0
	if(bQuit)
	{
		return;
	}
	
	//printf("line %d\n", __LINE__);
	
	DSKINFO info, *pInfoStack[9], *pTmpInfo;
	
	if(total == -1)//清除拔掉的磁盘信息
	{
		for(int i=0; i<nTabCol; i++)
		{
			#ifndef DISABLE_DISK_STATE
			if(i == 5)
			{
				m_DskItems[idx].pCombo->Show(FALSE);
			}
			else
			#endif
			{
				m_DskItems[idx].pStatic[i]->SetText("");
			}
		}

		u8 nTmpInfo = 0;
		std::vector<DSKINFO>::iterator p;
		for(p=m_dskdatalist.begin(); p!=m_dskdatalist.end(); p++)
		{
			if(nTmpInfo == idx)
			{
				m_dskdatalist.erase(p);
				break;
			}
			nTmpInfo++;
		}
	}
	else
	{
		//printf("line %d idx %d\n", __LINE__, idx);
		if(idx>=9) return;
		
		//printf("line %d \n", __LINE__);
		char tmp[8] = {0};
		sprintf(tmp, "%d", idx+1);
		info.index = tmp;
		
		SetLineInfIns(&info, type, total, free, 2, pDevPath);//新接入的硬盘IDLE

		m_dskdatalist.push_back(info);

		DSKITEM* pItem = &m_DskItems[idx];
		DSKINFO* pList = &m_dskdatalist[idx];

		//printf("##fill line ...2 type %d\n", type);
		
		for(int kk=0; kk<nTabCol; kk++)
		{
		#ifndef DISABLE_DISK_STATE
			if (5 == kk)
			{
				pItem->pCombo->SetCurSel(pList->attrib);
				pItem->pCombo->Show(TRUE);			
			}
			else
			{
				switch (kk)
				{
				case 0:
					pItem->pStatic[kk]->SetText(pList->index.c_str());
					break;
				case 1:
					pItem->pStatic[kk]->SetText(pList->type.c_str());
					break;
				case 2:
					pItem->pStatic[kk]->SetText(pList->capacity.c_str());
					break;
				case 3:
					pItem->pStatic[kk]->SetText(pList->freespace.c_str());
					break;
				case 4:
					pItem->pStatic[kk]->SetText(pList->state.c_str());
					break;
				case 6:
					pItem->pStatic[kk]->SetText(pList->source.c_str());
					break;
				}
				
				pItem->pStatic[kk]->Show(TRUE);
			}
		#else
			switch (kk)
			{
			case 0:
				pItem->pStatic[kk]->SetText(pList->index.c_str());
				break;
			case 1:
				pItem->pStatic[kk]->SetText(pList->type.c_str());
				break;
			case 2:
				pItem->pStatic[kk]->SetText(pList->capacity.c_str());
				break;
			case 3:
				pItem->pStatic[kk]->SetText(pList->freespace.c_str());
				break;
			case 4:
				pItem->pStatic[kk]->SetText(pList->state.c_str());
				break;
			case 5:
				pItem->pStatic[kk]->SetText(pList->source.c_str());
				break;
			}
			
			pItem->pStatic[kk]->Show(TRUE);
		#endif
		}
	}
#endif
}

void CPageDiskMgr::NotifyDiskChanged(void *param)
{
	SBizStoreDevManage *p = (SBizStoreDevManage *)param;
	if(p == NULL)
	{
		return;
	}
	
	printf("disk changed notify CPageDiskMgr page\n");
	
	BizSysSetRefreshDiskStatue(1);//传送磁盘刷新状态
	
	SBizStoreDevManage sStoreDevMgr;
	sStoreDevMgr.nDiskNum = p->nDiskNum;
	sStoreDevMgr.psDevList = p->psDevList;
	
	if(sStoreDevMgr.psDevList)
	{
		nDiskNum = sStoreDevMgr.nDiskNum;
		if(nDiskNum == 0)
		{
			//return;
		}
		
		m_dskdatalist.clear();
		
		//printf("NotifyDiskChanged disk num %d\n", nDiskNum);
		
		for(int i = 0; i < nDiskNum; i++)
		{
			SBizDevInfo* pList = &sStoreDevMgr.psDevList[i];
			
			char tmp[8] = {0};
			//sprintf(tmp, "%d", i+1);
			//printf("NotifyDiskChanged\n");
			sprintf(tmp, "%d", pList->disk_logic_idx);
			//printf("NotifyDiskChanged: %d\n", pList->disk_logic_idx);
			
			DSKINFO info;
			info.index = tmp;
			
			SetLineInfIns(&info, pList->nType, pList->nTotal, pList->nFree, pList->nStatus, pList->strDevPath);
			m_dskdatalist.push_back(info);
		}
		
		//yg modify 20140729
		//排序
		sort(m_dskdatalist.begin(), m_dskdatalist.end(), lessmark);//升序排序
	}
	
	for(int i = 0; i < 9; i++)
	{
		//显示前先清除界面中的内容
		if(i >= nDiskNum)
		{
			for(int kk = 0; kk < nTabCol; kk++)
			{
				m_DskItems[i].pStatic[kk]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
				m_DskItems[i].pStatic[kk]->SetText(NULL);
			}
			if(m_nCursel == i)
			{
				m_nCursel = -1;
			}
			continue;//break;
		}
		
		for(int kk = 0; kk < nTabCol; kk++)
		{
		#ifndef DISABLE_DISK_STATE
			if(5 == kk)
			{
				m_DskItems[i].pCombo->SetCurSel(m_dskdatalist[i].attrib);
				m_DskItems[i].pCombo->Show(TRUE);
			}
			else
			{
				switch(kk)
				{
				case 0:
					m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].index.c_str());
					break;
				case 1:
					m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].type.c_str());
					break;
				case 2:
					m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].capacity.c_str());
					break;
				case 3:
					m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].freespace.c_str());
					break;
				case 4:
					m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].state.c_str());
					break;
				case 6:
					m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].source.c_str());
					break;
				}
				
				m_DskItems[i].pStatic[kk]->Show(TRUE);
			}
		#else
			switch(kk)
			{
			case 0:
				m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].index.c_str());
				break;
			case 1:
				m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].type.c_str());
				break;
			case 2:
				m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].capacity.c_str());
				break;
			case 3:
				m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].freespace.c_str());
				break;
			case 4:
				m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].state.c_str());
				break;
			case 5:
				m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].source.c_str());
				break;
			}
			
			m_DskItems[i].pStatic[kk]->Show(TRUE);
		#endif
		}
	}
	
	BizSysSetRefreshDiskStatue(0);
}

void CPageDiskMgr::UpdateDiskData()
{
	if(BizSysGetCheckDiskStatue())//获取报警上传磁盘检测状态
	{
		return;
	}
	
	if(BizSysGetInsertDiskStatue())//获取新磁盘插入状态
	{
		sleep(3);
		return;
	}
	
	static char bFirstShow = 1;// 第一次显示的时候如果列表非空则不需要清空
	if(bFirstShow && m_dskdatalist.size() > 0)
	{
		bFirstShow = 0;
		return;
	}
	bFirstShow = 0;
	
	BizSysSetRefreshDiskStatue(1);//传送磁盘刷新状态
	
	u8 nMaxDiskNum = 9;//64;
	
	SBizDevInfo sDevList[nMaxDiskNum];
	memset(sDevList, 0, sizeof(SBizDevInfo)*nMaxDiskNum);
	
	SBizStoreDevManage sStoreDevMgr;
	sStoreDevMgr.nDiskNum = 0;
	sStoreDevMgr.psDevList = sDevList;
	
	if(sStoreDevMgr.psDevList)
	{
		//硬盘状态由modsys_UpdateDiskInfoFxn线程维护
		//此调用无法获取
		//unsigned char cur_atx_flag = tl_power_atx_check();
		//if(cur_atx_flag == 0)//电池供电
		//{
			
		//}
		//else
		{
			BizSysComplexDMGetInfo(&sStoreDevMgr, nMaxDiskNum);
		}
		
		nDiskNum = sStoreDevMgr.nDiskNum;
		if(nDiskNum == 0)
		{
			//return;
		}
		
		m_dskdatalist.clear();
		
		for(int i = 0; i < nDiskNum; i++)
		{
			SBizDevInfo* pList = &sStoreDevMgr.psDevList[i];
			
			char tmp[8] = {0};
			//sprintf(tmp, "%d", i+1);
			sprintf(tmp, "%d", pList->disk_logic_idx);
			//printf("UpdateDiskData: %d\n", pList->disk_logic_idx);
			
			DSKINFO info;
			info.index = tmp;
			//printf("UpdateDiskData\n");
			SetLineInfIns(&info, pList->nType, pList->nTotal, pList->nFree, pList->nStatus, pList->strDevPath);
			m_dskdatalist.push_back(info);
		}
		
		//yg modify 20140729
		//排序
		sort(m_dskdatalist.begin(), m_dskdatalist.end(), lessmark);//升序排序
	}
	
	for(int i = 0; i < 9; i++)
	{
		//显示前先清除界面中的内容
		if(i >= nDiskNum)
		{
			for(int kk = 0; kk < nTabCol; kk++)
			{
				m_DskItems[i].pStatic[kk]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
				m_DskItems[i].pStatic[kk]->SetText(NULL);
			}
			if(m_nCursel == i)
			{
				m_nCursel = -1;
			}
			continue;//break;
		}
		
		for(int kk = 0; kk < nTabCol; kk++)
		{
		#ifndef DISABLE_DISK_STATE
			if(5 == kk)
			{
				m_DskItems[i].pCombo->SetCurSel(m_dskdatalist[i].attrib);
				m_DskItems[i].pCombo->Show(TRUE);
			}
			else
			{
				switch(kk)
				{
				case 0:
					m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].index.c_str());
					break;
				case 1:
					m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].type.c_str());
					break;
				case 2:
					m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].capacity.c_str());
					break;
				case 3:
					m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].freespace.c_str());
					break;
				case 4:
					m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].state.c_str());
					break;
				case 6:
					m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].source.c_str());
					break;
				}
				
				m_DskItems[i].pStatic[kk]->Show(TRUE);
			}
		#else
			switch(kk)
			{
			case 0:
				m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].index.c_str());
				break;
			case 1:
				m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].type.c_str());
				break;
			case 2:
				m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].capacity.c_str());
				break;
			case 3:
				m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].freespace.c_str());
				break;
			case 4:
				m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].state.c_str());
				break;
			case 5:
				m_DskItems[i].pStatic[kk]->SetText(m_dskdatalist[i].source.c_str());
				break;
			}
			
			m_DskItems[i].pStatic[kk]->Show(TRUE);
		#endif
		}
	}
	
	BizSysSetRefreshDiskStatue(0);
}

void CPageDiskMgr::OnClkButton()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	//printf("yg CPageDiskMgr::OnClkButton\n");
	for(i = 0; i < 5+2; i++)
	{
		if(pFocusButton == pButton[i] && pFocusButton != NULL)
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
				{
					//printf("yg Refresh\n");
					UpdateDiskData();
					
					if(m_nCursel == -1)
					{
						//未选中，禁用格式化按钮
						pButton[3]->Enable(FALSE);
					}
					
					usleep(200*1000);
				}
				break;
			case 1:
				{
					if(m_nCursel>=0)
					{
						m_pDskInfo->SetDiskIdx(m_nCursel);
						m_pDskInfo->Open();
					}
				}
				break;
			case 3:
				{
					Format();
				}
				break;
			case 4:
				{
					this->Close();
				} break;
            case 5:
                {
	//yaogang modify 20141128
	#ifdef DISABLE_RECBITRATE
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
	#else
                    //上一步
                    CPage** page = GetPage();
                    if(page && page[EM_PAGE_RECORD])
                    {
                        page[EM_PAGE_RECORD]->UpdateData(UDM_GUIDE);
                        if(page[EM_PAGE_RECORD]->Open())
                        {
                            this->Close();
                        }
                    }
	#endif
                } break;
            case 6:
                {
                    //下一步
                    CPage** page = GetPage();
                    if(page && page[EM_PAGE_GUIDE])
                    {
                        //page[EM_PAGE_GUIDE]->UpdateData(UDM_EMPTY);
                        if(page[EM_PAGE_GUIDE]->Open())
                        {
                            this->Close();
                        }
                    }
                } break;
			default:
				break;
		}
	}
}

VD_BOOL CPageDiskMgr::MsgProc( uint msg, uint wpa, uint lpa )
{
	int px,py;
	int temp;
	static int chSel=0;
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
    	case XM_KEYDOWN://cw_tab
    		{
    			switch(wpa)
    			{
    				case KEY_0:
    				case KEY_1:
    				case KEY_2:
    				case KEY_3:
    				case KEY_4:
    				case KEY_5:
    				case KEY_6:
    				case KEY_7:
    				case KEY_8:
    				case KEY_9:
    				{
    					chSel=wpa - KEY_0;
    					if(chSel<=nDiskNum&&chSel>=KEY_0)
    					{
    						SelectDskItem(chSel-1);
    					}
    				}break;
    				
    				case KEY_UP:
    				{
    					if (--chSel>=KEY_0)
    						SelectDskItem(chSel-1);
    					else
    					{	
    						chSel=nDiskNum;
    						SelectDskItem(nDiskNum-1);
    					}
    				}break;
    				
    				case KEY_DOWN:
    				{
    					if (++chSel<=nDiskNum)
    						SelectDskItem(chSel-1);
    					else
    					{
    						chSel=KEY_0;
    						SelectDskItem(KEY_0-1);
    					}
    				}break;
    				
    				default: return CPageFrame::MsgProc(msg, wpa, lpa);
    			}
    			return TRUE;
    		}break;
    	case XM_LBUTTONDOWN:
    		{
    			px = VD_HIWORD(lpa);
    			py = VD_LOWORD(lpa);
				
    			VD_RECT rtScreen;
    			m_pDevGraphics->GetRect(&rtScreen);
				
    			int offsetx = (rtScreen.right - m_Rect.Width())/2;
    			int offsety = (rtScreen.bottom - m_Rect.Height())/2;
				
    			BOOL bFind = FALSE;
    			int i = 0;
    			for(i = 0; i < 9; i++)
    			{
    				for(int kk=0; kk<nTabCol; kk++)
    				{
    					VD_RECT rt;
    					pTab->GetTableRect(kk, i+1, &rt);
    					rt.left +=17+offsetx;
    					rt.top +=50+offsety;
    					rt.right +=17+offsetx;
    					rt.bottom +=50+offsety;
    					//printf("i=%d kk =%d ,x=%d, y=%d, %d %d %d %d \n",i, kk, px, py, rt.left, rt.top, rt.right,rt.bottom);
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
    				//printf("SelectDskItem %d\n", i);
    				SelectDskItem(i);
    			}
    		}
    		break;
    	default:
    		break;
    }
	
	return 	CPageFrame::MsgProc(msg, wpa, lpa);
}

void CPageDiskMgr::SelectDskItemEx( int index )
{
	if(index < 0 || index > 9 || index >= m_dskdatalist.size())
	{
		return;
	}
	
	for(int kk=0; kk<nTabCol; kk++)
	{
	#ifndef DISABLE_DISK_STATE
		if(kk != 5)
		{
			if (m_nCursel >= 0)
			{
				m_DskItems[m_nCursel].pStatic[kk]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
				m_DskItems[m_nCursel].pStatic[kk]->Draw();
			}

			m_DskItems[index].pStatic[kk]->SetBkColor(VD_RGB(56,108,148));
			m_DskItems[index].pStatic[kk]->Draw();
		}
	#else
		if(m_nCursel >= 0)
		{
			m_DskItems[m_nCursel].pStatic[kk]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			m_DskItems[m_nCursel].pStatic[kk]->Draw();
		}
		
		m_DskItems[index].pStatic[kk]->SetBkColor(VD_RGB(56,108,148));
		m_DskItems[index].pStatic[kk]->Draw();
	#endif
	}
	
	m_nCursel = index;
	
	if(strcmp(m_DskItems[m_nCursel].pStatic[1]->GetText(), "HDD") == 0)
		pButton[3]->Enable(TRUE);
	else
		pButton[3]->Enable(FALSE);
}

void CPageDiskMgr::SelectDskItem( int index )
{
	if(index < 0 || index > 9 || index >= m_dskdatalist.size())
	{
		return;
	}
	
	//printf("idx=%d, cur =%d\n", index, m_nCursel);
	
	if(index == m_nCursel)
	{
		return;
	}
	
	for(int kk=0; kk<nTabCol; kk++)
	{
	#ifndef DISABLE_DISK_STATE
		if(kk != 5)
		{
			if(m_nCursel >= 0)
			{
				m_DskItems[m_nCursel].pStatic[kk]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
				m_DskItems[m_nCursel].pStatic[kk]->Draw();
			}

			m_DskItems[index].pStatic[kk]->SetBkColor(VD_RGB(56,108,148));
			m_DskItems[index].pStatic[kk]->Draw();
		}
	#else
		if(m_nCursel >= 0)
		{
			m_DskItems[m_nCursel].pStatic[kk]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			m_DskItems[m_nCursel].pStatic[kk]->Draw();
		}
		
		m_DskItems[index].pStatic[kk]->SetBkColor(VD_RGB(56,108,148));
		m_DskItems[index].pStatic[kk]->Draw();
	#endif
	}
	
	m_nCursel = index;
	
	if(strcmp(m_DskItems[m_nCursel].pStatic[1]->GetText(), "HDD") == 0)
		pButton[3]->Enable(TRUE);
	else
		pButton[3]->Enable(FALSE);
}

void CPageDiskMgr::Format()
{
	const char* szDskMgrTitle[MAX_DSKMGR_TITLE] = 
	{
		"&CfgPtn.ID",
		"&CfgPtn.Type",
		"&CfgPtn.DiskSize",
		"&CfgPtn.Free",
		"&CfgPtn.Status",
		#ifndef DISABLE_DISK_STATE
		"&CfgPtn.Properties",
		#endif
		"&CfgPtn.Source",
	};	
	
	int chnnum = GetVideoMainNum();
	
	u8 flag_pauseRecord = 0;
	u8 *flag_chn = (u8*)calloc(1, chnnum);
	
	for(u8 i = 0; i < chnnum; i++)
	{
		if(GetCurRecStatus(i) && (GetRecordEnable(i)))// || GetAudioEnable(i)))
		{
			flag_pauseRecord = 1;
			flag_chn[i] = 1;
			//break;
		}
	}
	
	if(flag_pauseRecord)
	{
		UDM ret1 = MessageBox("&CfgPtn.FormatRecording", "&CfgPtn.WARNING", MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2);
		if(UDM_OK != GetRunStatue())
		{
			//Draw();
			for(int ii=0; ii<nTabCol; ii++)
			{
				pDskMgrTitle[ii]->SetText(szDskMgrTitle[ii]);//messagebox关闭后该页面抬头字出现乱码
			}
			free(flag_chn);
			return;
		}
		
		for(int i=0; i<nTabCol; i++)
		{
			pDskMgrTitle[i]->SetText(szDskMgrTitle[i]);
		}
	}
	
	m_DskItems[m_nCursel].pStatic[4]->SetText(pstr_disk_status[HDD_FORMATTING]);
	
	ModLogSetFormatFlag(1);//csp modify 20140812
	
	for(int i = 0; i < chnnum; i++)
	{
		BizRecordPause(i);
	}
	//yaogang modify 20150314
	BizSnapRecordPause(0); //0: format
	sleep(3);
	
	
	if(flag_pauseRecord)
	{
		usleep(5*1000*1000); //这样不保险
		sync();
	}
	
	SetStateInfo("&CfgPtn.Formating");
	SetFormatProg(0);
	
    if(m_IsLoader)
    {
        pButton[5]->Show(FALSE);
        pButton[6]->Show(FALSE);
    }
	
	char szDev[32] = {0};	
	memset(szDev, 0, sizeof(szDev));//zlbfix20110715 format
	
	printf("start format...\n");
	strncpy(szDev, m_DskItems[m_nCursel].pStatic[nTabCol-1]->GetText(), 8);//zlbfix20110715 format
	printf("format dev: %s\n", szDev);
	int ret = BizSysComplexDMFormat(szDev, EM_BIZFORMAT_FAT32_IFV);
	
	m_DskItems[m_nCursel].pStatic[4]->SetText(pstr_disk_status[HDD_IDLE]);
	
	ModLogSetFormatFlag(0);//csp modify 20140812
	
	if(ret == 0)
	{
		SetStateInfo("&CfgPtn.FomatDone");
		BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_FORMAT_SUCCESS);
	}
	else
	{
		SetStateInfo("&CfgPtn.FormatFail");
		BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_FORMAT_FAIL);
	}
	
	printf("format finish!!!\n");
	
	SetFormatProg(100);
	
	//UpdateDiskData();
	
	usleep(2000*1000);
	SetStateInfo("");
	SetFormatProg(-1);
    
    if(m_IsLoader)
    {
        pButton[5]->Show(TRUE);
        pButton[6]->Show(TRUE);
    }
	
	//if(flag_pauseRecord)
	{
		for(int i = 0; i < chnnum; i++)
		{
			//if(flag_chn[i])
			{
				BizRecordResume(i);
			}
		}
	}
	//yaogang modify 20150314
	BizSnapRecordResume(0);
	
	free(flag_chn);
}

void CPageDiskMgr::LoadButtomCtl()
{
    if (m_IsLoader)
    {
        pButton[5]->Show(TRUE);
        pButton[6]->Show(TRUE);
    }
    else
    {
        pButton[5]->Show(FALSE);
        pButton[6]->Show(FALSE);
    }
}

void CPageDiskMgr::SetInfoProg(u8 nType, char* pSrc, u32 nTotal, u32 nFree)
{
#if 0
	if(!bQuit)
	{
		if(pSrc)
		{
			int i = 0;
			char szTmp[32] = {0};
			
			u8 nTmpInfo = 0;
			
			std::string sTmp = pSrc;
			std::vector<DSKINFO>::iterator p;
			for(p=m_dskdatalist.begin(); p!=m_dskdatalist.end(); p++, i++)
			{
				if(sTmp == p->source)
				{
					break;
				}
			}
			
			if(p!=m_dskdatalist.end())
			{
				char outinfo[32] = {0};
				if(nTotal == -1)// 清除拔掉的磁盘信息
				{
					SetDskInfoLine(i, nType, nTotal, nFree, pSrc);
					nDiskNum--;
				}
				else if(nFree == -1)
				{
					strcpy(outinfo, GetParsedString("&CfgPtn.checking"));
					
					m_DskItems[i].pStatic[3]->SetText(outinfo);
				}
				else
				{
					char tmp[8] = {0};
					sprintf(outinfo, "%ld", nTotal/MEGA);
					sprintf(tmp, ".%02dG", (nTotal%MEGA)/TENKB);
					strcat(outinfo,tmp);	
					m_DskItems[i].pStatic[2]->SetText(outinfo);
					
					// 检测剩余空间的过程中禁止备份
					sprintf(outinfo, "%ld", nFree/MEGA);
					sprintf(tmp, ".%02dG", (nFree%MEGA/TENKB));
					strcat(outinfo,tmp);
				
					m_DskItems[i].pStatic[3]->SetText(outinfo);
				}
			}
			else
			{
				SetDskInfoLine(nDiskNum, nType, nTotal, nFree, pSrc);
				
				nDiskNum++;
			}
		}
	}
	//printf("num %d\n", nDiskNum);
#endif
}

VD_BOOL CPageDiskMgr::UpdateData( UDM mode )
{
	if(UDM_OPENED == mode)
	{
		bQuit = FALSE;
        LoadButtomCtl();
		
		//update data
		UpdateDiskData();
		
		if(m_nCursel == -1)
		{
			//未选中，禁用格式化按钮
			pButton[3]->Enable(FALSE);
		}
	}
	else if(UDM_CANCEL == mode)
	{
		for(int i=0; i<9; i++ )//m_dskdatalist.size(); i++)
		{
			for(int kk=0; kk<nTabCol; kk++)
			{
				switch(kk)
				{
				case 0:
					m_DskItems[i].pStatic[kk]->SetText("");
					break;
				case 1:
					m_DskItems[i].pStatic[kk]->SetText("");
					break;
				case 2:
					m_DskItems[i].pStatic[kk]->SetText("");
					break;
				case 3:
					m_DskItems[i].pStatic[kk]->SetText("");
					break;
				case 4:
					m_DskItems[i].pStatic[kk]->SetText("");
					break;
				case 5:
					m_DskItems[i].pStatic[kk]->SetText("");
					break;
				}
				
				m_DskItems[i].pStatic[kk]->Show(FALSE);
			}
		}
	}
	else if(UDM_CLOSED == mode)
	{
		bQuit = TRUE;
		/*if(pDskInfoBuf)  //zlb20111117  去掉部分malloc
		{
			free(pDskInfoBuf);
			pDskInfoBuf = NULL;
		}*/
		if (m_IsLoader)
		{
		    m_IsLoader = 0;//~m_IsLoader;
		    LoadButtomCtl();
		    SetTitle(GetParsedString("&CfgPtn.DiskManagement"));
		}
        else
        {
           ((CPageMainFrameWork *)(this->GetParent()))->ReflushItemName(); 
        }
	}
    else if(UDM_GUIDE == mode)
	{
        m_IsLoader = 1;//~m_IsLoader;
        SetTitle(GetParsedString("&CfgPtn.Guide"));
    }
	
	return TRUE;
}

void CPageDiskMgr::RemoteFormatStart(uchar nIdx, int nRslt)
{		
	if(GetPlayBakStatus()==1)
	{
		CPage** page = GetPage();
		((CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK])->StopPb(); 
	}
	
	if(!nRslt)
	{
		SetSystemLockStatus(1);
		SelectDskItemEx(nIdx);
	
		this->SetFormatProg(0);
	}
	else
	{
		SetSystemLockStatus(0);
		//yaogang modify 20141128
		usleep(500*1000);
		this->SetFormatProg(-1);
		usleep(200*1000);
		this->Close();
	}	
}

