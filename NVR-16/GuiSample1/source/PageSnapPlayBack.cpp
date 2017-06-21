#include <time.h>

#include "GUI/Pages/PagePlayBackFrameWork.h"
#include "GUI/Pages/PageChnSelect.h"
#include "GUI/Pages/PageAudioSelect.h"
#include "GUI/Pages/PageColorSetup.h"
#include "GUI/Pages/PagePlayBackVoColorSetup.h"
#include "GUI/Pages/PagePlayrateSelect.h"
#include "GUI/Pages/PageDesktop.h"
#include "GUI/Pages/PageSearch.h"
#include "GUI/Pages/PageSnapPlayBack.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageDataBackup.h"
#include "GUI/Pages/PageBackupInfo.h"


#include "biz.h"

static	std::vector<CItem*>	items[2];//工作模式区别


CPageSnapPlayBack::CPageSnapPlayBack( VD_PCRECT pRect,VD_PCSTR psz /*= NULL*/,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/, uint vstyle /*= 0*/ )
:CPageFloat(pRect, pParent,psz)
{
	CRect rtTemp;
	int i, j;
/*
	printf("%s rt.left: %d\n", __func__, pRect->left);
	printf("%s rt.right: %d\n", __func__, pRect->right);
	printf("%s rt.top: %d\n", __func__, pRect->top);
	printf("%s rt.bottom: %d\n", __func__, pRect->bottom);
*/	
	nlist_rows = (pRect->bottom - pRect->top)/CTRL_HEIGHT - 6;//6 行用作控制按键和状态显示
	if (nlist_rows < 0)
	{
		printf("%s: nlist_rows < 0 !!!!!!\n", __func__);
		//exit(0);
	}
	printf("%s: nlist_rows: %d\n", __func__, nlist_rows);

	rtTemp.top = 0;
	rtTemp.bottom = rtTemp.top + CTRL_HEIGHT * nlist_rows;
	rtTemp.right = pRect->right - pRect->left;
	rtTemp.left = 0;
	/*
	printf("%s rtTemp.left: %d\n", __func__, rtTemp.left);
	printf("%s rtTemp.right: %d\n", __func__, rtTemp.right);
	printf("%s rtTemp.top: %d\n", __func__, rtTemp.top);
	printf("%s rtTemp.bottom: %d\n", __func__, rtTemp.bottom);
	*/
	CTableBox* pTablist = CreateTableBox(rtTemp, this, 3, nlist_rows);
	pTablist->SetColWidth(0, TEXT_WIDTH*5/2);
	pTablist->SetColWidth(1, TEXT_WIDTH*7/2);

	const char* szTitle[] = {
		"&CfgPtn.Channel",
		"&CfgPtn.Time",
		"&CfgPtn.Display",
	};
	const char* szBtBmpPath[4] = 
		{DATA_DIR"/temp/search_play.bmp", DATA_DIR"/temp/search_play_f.bmp", DATA_DIR"/temp/search_play.bmp", DATA_DIR"/temp/search_play_f.bmp"};
	const char* szBtBmpPath1[4][2] = {
			{DATA_DIR"/temp/listex_btn_leftmost.bmp", DATA_DIR"/temp/listex_btn_leftmost_f.bmp"},
			{DATA_DIR"/temp/listex_btn_left.bmp", DATA_DIR"/temp/listex_btn_left_f.bmp"},
			{DATA_DIR"/temp/listex_btn_right.bmp", DATA_DIR"/temp/listex_right_f.bmp"},
			{DATA_DIR"/temp/listex_btn_rightmost.bmp", DATA_DIR"/temp/listex_btn_rightmost_f.bmp"},
		};
	
	
	for (i=0; i<3; i++)
	{
		pTablist->GetTableRect(i, 0, &rtTemp);
		/*
		printf("%s rtTemp.left: %d\n", __func__, rtTemp.left);
		printf("%s rtTemp.right: %d\n", __func__, rtTemp.right);
		printf("%s rtTemp.top: %d\n", __func__, rtTemp.top);
		printf("%s rtTemp.bottom: %d\n", __func__, rtTemp.bottom);
		*/
		pTitleList[i]= CreateStatic(rtTemp, this, szTitle[i]);
		pTitleList[i]->SetBkColor(VD_RGB(67,77,87));
		pTitleList[i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		pTitleList[i]->SetTextAlign(VD_TA_XLEFT);
	}
	
	//nlist_rows 包括第一行显示列标题
	ppChn = (CStatic **)new CStatic *[nlist_rows - 1];
	if (ppChn == NULL)
	{
		printf("%s: ppChn == NULL !!!!!!\n", __func__);
		//exit(0);
	}
	ppChkbox = (CCheckBox **)new CCheckBox *[nlist_rows - 1];
	if (ppChkbox == NULL)
	{
		printf("%s: ppChkbox == NULL !!!!!!\n", __func__);
		//exit(0);
	}
	ppTime = (CStatic **)new CStatic *[nlist_rows - 1];
	if (ppTime == NULL)
	{
		printf("%s: ppTime == NULL !!!!!!\n", __func__);
		//exit(0);
	}
	
	ppBtDisplay = (CButton **)new CButton *[nlist_rows - 1];
	if (ppBtDisplay == NULL)
	{
		printf("%s: ppBtDisplay == NULL !!!!!!\n", __func__);
		//exit(0);
	}
	
	for (i=0; i<nlist_rows - 1; i++)
	{
		pTablist->GetTableRect(0, i+1, &rtTemp);

		ppChkbox[i] = CreateCheckBox(rtTemp, this, styleEditable, NULL);
		ppChkbox[i]->SetValue(0);
		items[EM_BACKUP].push_back(ppChkbox[i]);
		
		ppChn[i] = CreateStatic(CRect(rtTemp.left+24, rtTemp.top, rtTemp.left+44, rtTemp.bottom-4), this, "");
		//ppChn[i]->Show(FALSE);
		
	}

	for (i=0; i<nlist_rows - 1; i++)
	{
		pTablist->GetTableRect(1, i+1, &rtTemp);
		ppTime[i] = CreateStatic(rtTemp, this, "");
		//ppTime[i]->Show(FALSE);
	}

	for (i=0; i<nlist_rows - 1; i++)
	{
		pTablist->GetTableRect(2, i+1, &rtTemp);
 		ppBtDisplay[i] = CreateButton(CRect(rtTemp.left, rtTemp.top+2, rtTemp.left+40, rtTemp.bottom-2), this, NULL, (CTRLPROC)&CPageSnapPlayBack::OnClickDisPlay, NULL, buttonNormalBmp);
		ppBtDisplay[i]->SetBitmap(VD_LoadBitmap(szBtBmpPath1[2][0]), VD_LoadBitmap(szBtBmpPath1[2][1]), VD_LoadBitmap(szBtBmpPath1[2][1]));
		ppBtDisplay[i]->Show(FALSE);
		//ppBtDisplay[i]->Show(FALSE);
	}
	
	
	//
	rtTemp.left = 8;
	rtTemp.top =CTRL_HEIGHT * nlist_rows;
	rtTemp.bottom = rtTemp.top + CTRL_HEIGHT;

	rtTemp.top += 2;
	rtTemp.bottom -= 2;
	
	for(i =0; i<4; i++)
	{
		pBmpBtPage[i][0] = VD_LoadBitmap(szBtBmpPath1[i][0]);
		pBmpBtPage[i][1] = VD_LoadBitmap(szBtBmpPath1[i][1]);
		pBtPageCtrPage[i] = CreateButton(CRect(rtTemp.left, rtTemp.top, rtTemp.left+40, rtTemp.bottom), this, NULL, (CTRLPROC)&CPageSnapPlayBack::OnClickBt, NULL, buttonNormalBmp);
		rtTemp.left += 40+4;
		pBtPageCtrPage[i]->SetBitmap(pBmpBtPage[i][0], pBmpBtPage[i][1], pBmpBtPage[i][1]);

	}

	rtTemp.left = 8;
	rtTemp.top = rtTemp.bottom;
	rtTemp.bottom = rtTemp.top + CTRL_HEIGHT;

	int totalpage = 0;
	int curpage = 0;
	char tmp[10] = {0};
	sprintf(tmp, "%d/%d", curpage,totalpage);
	pszResultPage = CreateStatic(CRect(rtTemp.left, rtTemp.top+2, rtTemp.left+ TEXT_WIDTH*3, rtTemp.bottom-2), this, tmp);
	pszResultPage->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
	pszResultPage->SetTextAlign(VD_TA_LEFT);

	rtTemp.left += TEXT_WIDTH*3 + 4;
	pEditPageJump = CreateEdit(CRect(rtTemp.left, rtTemp.top+1, rtTemp.left+ TEXT_WIDTH*2, rtTemp.bottom-1), 
								this, TEXT_WIDTH*2, edit_KI_NUMERIC, NULL);
	pEditPageJump->SetBkColor(VD_RGB(67,77,87));

	rtTemp.left += TEXT_WIDTH*2 + 4;
	pBmpBtPage[2][0] = VD_LoadBitmap(szBtBmpPath1[2][0]);
	pBmpBtPage[2][1] = VD_LoadBitmap(szBtBmpPath1[2][1]);
	pBtPageJump = CreateButton(CRect(rtTemp.left, rtTemp.top+2, rtTemp.left+40, rtTemp.bottom-2), this, NULL, (CTRLPROC)&CPageSnapPlayBack::OnClickBt, NULL, buttonNormalBmp);
	pBtPageJump->SetBitmap(pBmpBtPage[2][0], pBmpBtPage[2][1], pBmpBtPage[2][1]);

	//备份
	rtTemp.left = 8;
	rtTemp.top = rtTemp.bottom+CTRL_HEIGHT;
	rtTemp.bottom = rtTemp.top + CTRL_HEIGHT;

	pBtBackup = CreateButton(CRect(rtTemp.left, rtTemp.top+2, rtTemp.left+TEXT_WIDTH*3, rtTemp.bottom-2), this, "&CfgPtn.Backup", (CTRLPROC)&CPageSnapPlayBack::OnClickBt, NULL, buttonNormalBmp);	
	pBtBackup->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	items[EM_BACKUP].push_back(pBtBackup);
	//退出
	rtTemp.left = 8 + TEXT_WIDTH*3 + 4;
	pBtExit = CreateButton(CRect(rtTemp.left, rtTemp.top+2, rtTemp.left+TEXT_WIDTH*3, rtTemp.bottom-2), this, "&CfgPtn.Exit", (CTRLPROC)&CPageSnapPlayBack::OnClickBt, NULL, buttonNormalBmp);
		
	pBtExit->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));

	//初始化
	m_curPage = -1;
	cur_line = -1;
	memset(&sSearchSnapResult, 0, sizeof(sSearchSnapResult));
	sSearchSnapResult.nFileNum = 0;
	sSearchSnapResult.psRecSnapInfo = NULL;
	
	//状态栏

	//默认显示回放模式
	work_mode = EM_PLAYBACK;
	ShowModeItem(EM_PLAYBACK, TRUE);
	ShowModeItem(EM_BACKUP, FALSE);
	//数据
	sFileList = (SBizRecSnapInfo*)calloc(sizeof(SBizRecSnapInfo), MAX_SEARCH_FILE_NUM);
	sSearchSnapResult.psRecSnapInfo = (SBizRecSnapInfo*)calloc(sizeof(SBizRecSnapInfo), MAX_SEARCH_FILE_NUM);
		
}

CPageSnapPlayBack::~CPageSnapPlayBack()
{

}

VD_BOOL CPageSnapPlayBack::UpdateData( UDM mode )
{
	SBizPreviewElecZoom MenuRect;
	MenuRect.s32X = m_Rect.left;
	MenuRect.s32Y = m_Rect.top;
	MenuRect.u32Width = m_Rect.right - m_Rect.left;
	MenuRect.u32Height = m_Rect.bottom - m_Rect.top;
/*
	printf("%s MenuRect.s32X: %d\n", __func__, MenuRect.s32X);
	printf("%s MenuRect.s32Y: %d\n", __func__, MenuRect.s32Y);
	printf("%s MenuRect.u32Width: %d\n", __func__, MenuRect.u32Width);
	printf("%s MenuRect.u32Height: %d\n", __func__, MenuRect.u32Height);
*/
	if(UDM_OPEN == mode)
	{
		printf("%s PreviewSnapSet !\n", __func__);
		/*
		if (NULL == sSearchSnapResult.psRecSnapInfo)
		{
			sSearchSnapResult.psRecSnapInfo = (SBizRecSnapInfo*)calloc(sizeof(SBizRecSnapInfo), 4000);
		}
		*/
		//处理界面
		SetPlayBakStatus(1);
		if ( EM_PLAYBACK == work_mode )
		{
			if(m_pPageSearch)//cw_test
			{
				//printf("1Playback open from search cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
				m_pPageSearch->GetParentPage()->Show(FALSE);
			}
		}
		else if (EM_BACKUP == work_mode)
		{
			if(m_pPageBackup)
			{
				m_pPageBackup->GetParentPage()->Show(FALSE);
			}
		}
		
		
		CPage** pPageList = GetPage();

		m_pDesktop = pPageList[EM_PAGE_DESKTOP];
		//((CPageDesktop*)m_pDesktop)->HideAllOsdWithoutTime(1);
		((CPageDesktop*)m_pDesktop)->HideAllOsdWithoutTime(1);
		
		
		((CPageDesktop*)m_pDesktop)->ShowTimeTitle(0,FALSE);
		
		((CPageDesktop*)m_pDesktop)->SetModePlaying(); //yzw
		

		for(int i = 0; i < 10; i++)
		{
			((CPageDesktop*)m_pDesktop)->ShowSplitLine(i, FALSE);
		}
		
		//初始化VDEC-->VO   lib_preview.c tl_snap_preview_init()
		PreviewSnapSet(0, &MenuRect);
		
		//磁盘搜索图片
		SearchSnap();
		
		//printf("%s yg 1\n", __func__);
		//设置搜索结果列表
		pEditPageJump->SetText("");
		if (sSearchSnapResult.nFileNum > 0)
		{
			m_curPage = 0;
			SetPageList(m_curPage);
			//printf("%s yg 2\n", __func__);
			MouseMoveToLine(0);

			SBizRecSnapInfo *pSnapInfo;
			pSnapInfo = &sSearchSnapResult.psRecSnapInfo[0];
			BizSnapDisplay(pSnapInfo);
			//printf("%s yg 3\n", __func__);
		}
		else
		{
			ClearPage();
			pszResultPage->SetText("0/0");
			
		}
	}
	else if (UDM_CLOSED == mode)
	{
		SetPlayBakStatus(0);
		PreviewSnapSet(1, &MenuRect);

		//SetPlayBakStatus(0);
		
		SetSystemLockStatus(1);//cw_lock
		
		BizResumePreview(1);//yaogang modify 20140918
		
		((CPageDesktop*)m_pDesktop)->SetModePreviewing();//yzw
		
		if(GetTimeDisplayCheck())
		{
			((CPageDesktop*)m_pDesktop)->ShowTimeTitle(0,TRUE);
		}

		if ( EM_PLAYBACK == work_mode )
		{
			if(m_pPageSearch)
			{
				m_pPageSearch->GetParentPage()->Show(TRUE);//cw_test
				printf("playback over PageSearch Open cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_tes
				m_pPageSearch->Open();//cw_test UpdateData(UDM_OPENED)
				m_pPageSearch = NULL;
			}
		}
		else if (EM_BACKUP == work_mode)
		{
			if(m_pPageBackup)
			{
				m_pPageBackup->GetParentPage()->Show(TRUE);//cw_test
				printf("playback over PageBackup Open cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_tes
				m_pPageBackup->Open();//cw_test UpdateData(UDM_OPENED)
				m_pPageBackup = NULL;
			}
		}
		
		SetSystemLockStatus(0);
	}
	return TRUE;
}

void CPageSnapPlayBack::SetSearchPara(SBizSearchPara *psSearchPara)
{
	if (NULL != psSearchPara)
	{
		memcpy(&sSearchPara, psSearchPara, sizeof(SBizSearchPara));
	}
}

void CPageSnapPlayBack::OnClickDisPlay()
{
	int i = 0;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	SBizRecSnapInfo *pSnapInfo;
	int nrows = nlist_rows - 1;//第一行显示列标题
	//显示
	for (i=0; i<nrows; i++)
	{
		if(pFocusButton == ppBtDisplay[i])
		{
			break;
		}
	}
	
	printf("%s click row: %d\n", __func__, i);
	if (i < nrows)
	{
		if ((m_curPage*nrows + i) < sSearchSnapResult.nFileNum)
		{
			MouseMoveToLine(i);
			
			pSnapInfo = &sSearchSnapResult.psRecSnapInfo[m_curPage*nrows + i];
			BizSnapDisplay(pSnapInfo);
		}
		else
		{
			printf("%s (m_curPage*nrows + i) >= sSearchSnapResult.nFileNum\n", __func__);
		}
	}
}

void CPageSnapPlayBack::OnClickBt()
{
	int i = 0;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	int nrows = nlist_rows - 1;//第一行显示列标题
	int last_page = (sSearchSnapResult.nFileNum + nrows -1)/nrows -1;//n-1
	int old_page = m_curPage;
	//页控制
	for(i = 0; i < 4; i++)
	{
		if(pFocusButton == pBtPageCtrPage[i])
		{
			break;
		}
	}

	if (i < 4)
	{
		switch (i)
		{
			case 0://首页
			{
				printf("%s first page\n", __func__);
				//SetPageList(0);
				m_curPage = 0;
			}break;
			case 1://上一页
			{
				printf("%s pre page\n", __func__);
				m_curPage--;
				if (m_curPage < 0)
				{
					m_curPage = 0;
					//SetPageList(m_curPage);
				}
				
			}break;
			case 2://下一页
			{
				printf("%s next page\n", __func__);
				m_curPage++;
				if (m_curPage > last_page)
				{
					m_curPage = last_page;
					//SetPageList(m_curPage);
				}
			}break;
			case 3://尾页
			{
				printf("%s last page\n", __func__);
				m_curPage = last_page;
				//SetPageList(m_curPage);
			}break;
			default:
				break;
		}
		
		if (old_page != m_curPage)
		{
			SetPageList(m_curPage);
		}
	}
	else
	{
		if (pFocusButton == pBtPageJump)
		{
			printf("%s jump page\n", __func__);
			char szInfo[32];
			memset(szInfo, 0, sizeof(szInfo));
			pEditPageJump->GetText(szInfo, sizeof(szInfo));
			int choice_page = strtol(szInfo, 0, 10);//1~n
			if ((choice_page > 0) && (choice_page <= last_page+1))
			{
				m_curPage = choice_page-1;//0~n-1
				if (old_page != m_curPage)
				{
					SetPageList(m_curPage);
				}
			}
		}
		else if (pFocusButton == pBtExit)
		{
			printf("%s exit\n", __func__);
			this->Close();
		}
		else if (pFocusButton == pBtBackup)
		{
			printf("%s pBtBackup down\n", __func__);
			//备份
			int i;//line
			int nCount = 0;
			u64 filesize = 0;//cw_backup
			EMBIZFILETYPE emFileType = EM_BIZFILE_SNAP;

			sBackTgtAtt.fmt = EM_BIZFORMAT_JPEG;
			sBackTgtAtt.devt = EM_BIZBACKUP_USB;
			sSearchCondition.emSearchWay = EM_BIZSEARCH_BY_FILE;
			
			sSearchCondition.sSearchResult.nFileNum = 0;
			sSearchCondition.sSearchResult.psRecSnapInfo = sFileList;
			
			for (i = 0; i < nrows; i++)
			{
				if(m_curPage*nrows+i+1>sSearchSnapResult.nFileNum)
				{
					break;
				}
				if (ppChkbox[i]->GetValue())
				{
					memcpy(&sFileList[nCount], \
						&sSearchSnapResult.psRecSnapInfo[m_curPage*nrows+i], \
						sizeof(SBizRecSnapInfo));
					
					filesize += sFileList[nCount].nSize;//cw_backup
					nCount++;
				}
				
			}
			printf("%s filenum: %d, filesize: %lld\n", __func__, nCount, filesize);
			// 无U盘则提示
			u8 nMaxDiskNum = 64;
			
			SBizStoreDevManage sStoreDevMgr;
			sStoreDevMgr.nDiskNum = 0;
			sStoreDevMgr.psDevList = (SBizDevInfo*)malloc(sizeof(SBizDevInfo)*nMaxDiskNum);
			
			memset(sStoreDevMgr.psDevList, 0, sizeof(SBizDevInfo)*nMaxDiskNum);
			
			u8 nDiskNum = 0;
			
			if(sStoreDevMgr.psDevList)
			{
				BizSysComplexDMGetInfo(&sStoreDevMgr, nMaxDiskNum);
				
				if(sStoreDevMgr.nDiskNum > 0)
				{
					for(int i=0; i<sStoreDevMgr.nDiskNum; i++)
					{
						if(sStoreDevMgr.psDevList[i].nType == 1)
						{
							nDiskNum++;
						}
					}
				}
				
				free(sStoreDevMgr.psDevList);
				sStoreDevMgr.psDevList = NULL;//csp modify 20131213
				
				if(nDiskNum <= 0)
				{
					MessageBox("&CfgPtn.BackupNoUdisk", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
					return;
				}
			}
			else
			{
				return;
			}
			printf("%s yg 1\n", __func__);
			//备份
			if(nCount > 0)
			{
				CPage** page = GetPage();
				m_pPageBackupInfo = (CPageBackupInfo *)page[EM_PAGE_BACKUP];
				
				char m_sTime[32];
				char m_eTime[32];
				GetTimeForBackup(sSearchPara.nStartTime, m_sTime);
				GetTimeForBackup(sSearchPara.nEndTime, m_eTime);
				
				sSearchCondition.sSearchResult.nFileNum = nCount;
				BizGUiWriteLog(BIZ_LOG_MASTER_BACKUP, BIZ_LOG_SLAVE_START_BACKUP);
								
				if(m_pPageBackupInfo)
				{
					printf("%s yg 2\n", __func__);
					this->Close();
					m_pPageBackupInfo->SetBackupInfo(emFileType, &sBackTgtAtt, &sSearchCondition);
					m_pPageBackupInfo->SetFileNum(nCount);
					m_pPageBackupInfo->SetStartTime(m_sTime);
					m_pPageBackupInfo->SetEndTime(m_eTime);
					m_pPageBackupInfo->SetFileSize(filesize/(u64)MEGA, filesize%(u64)MEGA);
					m_pPageBackupInfo->SetBackupTitle(EM_BACKUP_FILE);//cw_backup
					m_pPageBackupInfo->SetSnapMode();
					m_pPageBackupInfo->Open();
				}
			}
		}
	}
}

VD_BOOL CPageSnapPlayBack::MsgProc( uint msg, uint wpa, uint lpa )
{
	switch(msg)
	{
		case XM_RBUTTONDOWN:
			this->Close();
			printf("%s: Close()!!!!!!\n", __func__);
			break;
		default:
    			break;
	}

	return CPage::MsgProc(msg, wpa, lpa);
}

void CPageSnapPlayBack::SetDesktop(CPage* pDesktop)
{
	m_pDesktop = pDesktop;
}

void CPageSnapPlayBack::SetSearchPage(CPageSearch* pPage)
{
	m_pPageSearch = pPage;
}
void CPageSnapPlayBack::SetBackupPage(CPageDataBackup* pPage)
{
	m_pPageBackup = pPage;
}

void CPageSnapPlayBack::SearchSnap()
{	
	EMBIZFILETYPE emFileType = EM_BIZFILE_SNAP;

	sSearchSnapResult.nFileNum = 0;
	if(NULL == sSearchSnapResult.psRecfileInfo)
	{
		printf("%s sSearchSnapResult.psRecfileInfo==NULL\n", __func__);
		return;
	}
	memset(sSearchSnapResult.psRecSnapInfo, 0, 4000*sizeof(SBizRecSnapInfo));
	
	printf("%s Start snap search, time:%u\n", __func__, time(NULL));
	if( 0==BizSysComplexDMSearch(
		emFileType,
		&sSearchPara, 
		&sSearchSnapResult, 
		4000 ))
	{
		printf("snap num : %d\n", sSearchSnapResult.nFileNum);
		/*
		m_nFiles = psResult->nFileNum;
		m_curPage1 = 0;
		SetProg(0,0);
		SetPage1(m_curPage1);
		if(psResult->nFileNum)
		{
			MouseMoveToLine(-1);//cw_tab
			pTips->SetText("&CfgPtn.DoubleClickToPlay");
		}
		else
		{
			pTips->SetText("&CfgPtn.NoRecordFiles");
		}
		
		if(psResult->nFileNum >= 4000)
		{
			char tmp[100] = {0};
			sprintf(tmp, "%s:%d", GetParsedString("&CfgPtn.ReachMaxSupportSearchNum"),psResult->nFileNum);
			MessageBox(tmp, "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
		}
		*/
	}
	else
	{
		printf("%s BizSysComplexDMSearch() failed\n");
	}
	
	printf("%s End snap search, time:%d\n", __func__, time(NULL));
}

void CPageSnapPlayBack::SetPageList(int nPage)
{
	int nrows = nlist_rows -1;//第一行显示列标题
	int pages = (sSearchSnapResult.nFileNum + nrows -1)/nrows;
	
	if ((sSearchSnapResult.nFileNum == 0) || (nPage <0) || (nPage >= pages))
	{
		return;
	}
	
	int i;
	for(i = 0; i < nrows; i++)
	{
		if(m_curPage*nrows+i+1>sSearchSnapResult.nFileNum)
		{
			break;
		}
		//包括第一行显示列标题,i+1
		SetLineOfPage(i, &sSearchSnapResult.psRecSnapInfo[m_curPage*nrows + i]);
	}
	for(; i<nrows; i++)
	{
		ClearLineOfPage(i);
	}
	//SetProg(m_curPage1+1,(sSearchResult0.nFileNum/8+((sSearchResult0.nFileNum%8)?1:0)));
	
	//MouseMoveToLine(-1);//cw_tab
	//m_chSel=0;
	//if(bInTab)
	//	MouseMoveToLine(0);
	char tmp[10] = {0};
	
	sprintf(tmp, "%d/%d", nPage+1, pages);//从第一页开始计数
	pszResultPage->SetText(tmp);
}

void CPageSnapPlayBack::SetLineOfPage(int line, SBizRecSnapInfo *pSnapInfo)
{
	char szCh[8] = {0};
	char szTime[32] = {0};
	if( (line >= 0) && (line < nlist_rows-1))
	{
		ppChkbox[line]->SetValue(0);
		
		if (work_mode == EM_BACKUP)
		{
			ppChkbox[line]->Show(TRUE);
		}
		else
		{
			ppChkbox[line]->Show(FALSE);
		}
		
		memset(szCh, 0, sizeof(szCh));
		sprintf(szCh,"%d",pSnapInfo->nChn+1);
		ppChn[line]->SetText(szCh);
		
		memset(szTime, 0, sizeof(szTime));
		GetTimeForBackup2(pSnapInfo->nStartTime, szTime);	
		//printf("chn[%02d], %s, %d\n",pSnapInfo->nChn,szTime,pSnapInfo->nStartTime);//捕获信息用，这次不要关闭
		ppTime[line]->SetText(szTime);
		
		ppBtDisplay[line]->Show(TRUE);
	}
}

void CPageSnapPlayBack::ClearLineOfPage(int line)
{
	if( (line >= 0) && (line < nlist_rows-1))
	{
		ppChkbox[line]->SetValue(0);
		ppChkbox[line]->Show(FALSE);
		
		ppChn[line]->SetText("");
		ppTime[line]->SetText("");
		ppBtDisplay[line]->Show(FALSE);
	}
}
void CPageSnapPlayBack::ClearPage()
{
	int line;
	for (line = 0;line < nlist_rows-1; line++)
		ClearLineOfPage(line);
}


void CPageSnapPlayBack::MouseMoveToLine( int line)
{
	int nrows = nlist_rows -1;//第一行显示列标题
	int pages = (sSearchSnapResult.nFileNum + nrows -1)/nrows;

	printf("%s(): nrows: %d, pages: %d, cur_line: %d, line: %d\n", __func__, nrows, pages, cur_line, line);

	if( (line >= 0) && (line < nrows) )
	{
		if( (m_curPage*nrows+line < sSearchSnapResult.nFileNum) && (cur_line != line) )
		{
			if (cur_line >= 0) 
			{
				ppChn[cur_line]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
				ppTime[cur_line]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
			}
			ppChn[line]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
			ppTime[line]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);

			cur_line = line;
		}
	}
}
void CPageSnapPlayBack::ShowModeItem(EM_PAGE_SNAP_MODE mode, BOOL bShow)
{
	int count;
	if (mode >= 0 && mode < EM_SNAP_MAX)
	{
		count= items[mode].size();
		
		for (int i=0; i<count; i++)
		{
			if(items[mode][i])
			{
				items[mode][i]->Show(bShow);
			}
		}
	}
}

void CPageSnapPlayBack::SetWorkmode(EM_PAGE_SNAP_MODE mode)
{	
	printf("%s mode: %d, work_mode: %d\n", __func__, mode, work_mode);
	if (mode >= 0 && mode < EM_SNAP_MAX)
	{
		if (mode != work_mode)
		{
			ShowModeItem(work_mode, FALSE);
			work_mode = mode;
			ShowModeItem(work_mode, TRUE);
		}
	}
}


