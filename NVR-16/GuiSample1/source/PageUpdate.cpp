#include "GUI/Pages/PageUpdate.h"
#include "biz.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageInfoBox.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/PagePlayBackFrameWork.h"//cw_remote

#define UPDATE_FILE_MAX	8
#define ROW_ITEM		4

static char* szBtBmpPath[4][2] = {
	{DATA_DIR"/temp/listex_btn_leftmost.bmp", DATA_DIR"/temp/listex_btn_leftmost_f.bmp"},
	{DATA_DIR"/temp/listex_btn_left.bmp", DATA_DIR"/temp/listex_btn_left_f.bmp"},
	{DATA_DIR"/temp/listex_btn_right.bmp", DATA_DIR"/temp/listex_right_f.bmp"},
	{DATA_DIR"/temp/listex_btn_rightmost.bmp", DATA_DIR"/temp/listex_btn_rightmost_f.bmp"},
};
static VD_BITMAP* pBmpBtPageCtrl[4][2];

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

#define BUTTONWIDTH 80
#define BUTTONHEIGHT 25
CPageUpdate::CPageUpdate( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPageFrame(pRect, psz, icon, pParent)
{
	SetMargin(0,0,0,0);
	count = 0;

	m_RemoteUpFlag = 0;
	
	m_pDevGraphics = CDevGraphics::instance(m_screen);
	m_pDevGraphics->GetRect(&m_screenRT);
	
	CRect rt(17, 55, 200, 60+CTRL_HEIGHT);
	pCombo = CreateComboBox(rt, this, NULL, NULL, (CTRLPROC)&CPageUpdate::OnComboBox, 0);

	pCombo->AddString("&CfgPtn.MainBoard");
	pCombo->AddString("&CfgPtn.Pannel");
	pCombo->AddString("&CfgPtn.StartLogo");
	pCombo->AddString("&CfgPtn.AppLogo");
	
	rt.left = rt.right + 20;
	rt.right = rt.left + BUTTONWIDTH;
	rt.top = 60;
	rt.bottom = rt.top + BUTTONHEIGHT - 1;
	pBtRefresh = CreateButton(rt, this, "&CfgPtn.Refresh", (CTRLPROC)&CPageUpdate::OnBtRefresh);
	pBtRefresh->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));

	rt.left = 17;
	rt.right = m_Rect.Width() - 17;
	rt.top = rt.bottom + 12;
	rt.bottom = rt.top + CTRL_HEIGHT*9;
	pTab[0] = CreateTableBox(rt, this, 4, 9);
	pTab[0]->SetColWidth(0, 265);
	pTab[0]->SetColWidth(1, 150);
	pTab[0]->SetColWidth(2, 55);

	char* szTitle[4] = {"&CfgPtn.Name","&CfgPtn.FILESIZE", "&CfgPtn.Type", "&CfgPtn.DateModified"};
	int i = 0;
	CRect rtTemp;
	for (i=0; i<4; i++)
	{
		pTab[0]->GetTableRect(i, 0, &rtTemp);

		pTitlePage[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, szTitle[i]);
        pTitlePage[i]->SetTextAlign(VD_TA_CENTER);
		pTitlePage[i]->SetBkColor(VD_RGB(67,77,87));
		pTitlePage[i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		//pTitlePage[i]->SetTextAlign(VD_TA_CENTER);
	}

	
	for(i=0; i<8; i++)
	{
		pTab[0]->GetTableRect(0, i+1, &rtTemp);
		pName[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");

	}

	for(i=0; i<8; i++)
	{
		pTab[0]->GetTableRect(1, i+1, &rtTemp);
		pSize[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");

	}

	for(i=0; i<8; i++)
	{
		pTab[0]->GetTableRect(2, i+1, &rtTemp);
		pType[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");

	}

	for(i=0; i<8; i++)
	{
		pTab[0]->GetTableRect(3, i+1, &rtTemp);
		pModifyDate[i]= CreateStatic(CRect(rt.left+rtTemp.left, rt.top+rtTemp.top, rt.left+rtTemp.right,rt.top+rtTemp.bottom), 
			this, "");
	}

	rt.top = rt.bottom -2;
	rt.bottom = rt.top + CTRL_HEIGHT;
	pTab[1] = CreateTableBox(rt, this, 1, 1);
	char tmp[10] = {0};
	int totalpage =0;
	int curpage = 0;
	rt.left += 5;
	rt.top += 2;
	rt.right = rt.left + TEXT_WIDTH*4;
	rt.bottom -=2;
	sprintf(tmp, "%d/%d", curpage,totalpage);
	pszResult = CreateStatic(rt, this, tmp);
	pszResult->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
	pszResult->SetTextAlign(VD_TA_LEFT);

	rt.left = m_Rect.Width()- 20 - 40*4 - 10*4;
	for (i =0; i<4; i++)
	{
		pBmpBtPageCtrl[i][0] = VD_LoadBitmap(szBtBmpPath[i][0]);
		pBmpBtPageCtrl[i][1] = VD_LoadBitmap(szBtBmpPath[i][1]);
		//printf("VD_LoadBitmap %x %x \n", pBmpBtPageCtrl[i][0], pBmpBtPageCtrl[i][1]);
		pBtPageCtr[i] = CreateButton(CRect(rt.left, rt.top, rt.left+40, rt.bottom), this, NULL, (CTRLPROC)&CPageUpdate::OnClickPageCtr, NULL, buttonNormalBmp);
		rt.left += 40+10;
		pBtPageCtr[i]->SetBitmap(pBmpBtPageCtrl[i][0], pBmpBtPageCtrl[i][1], pBmpBtPageCtrl[i][1]);
	}


	rt.right = m_Rect.Width() - 20;
	rt.left = rt.right - BUTTONWIDTH;
	rt.bottom = m_Rect.Height() - 10;
	rt.top = rt.bottom - BUTTONHEIGHT - 1;
	pBtExit = CreateButton(rt, this, "&CfgPtn.Exit", (CTRLPROC)&CPageUpdate::OnBtExit);
	pBtExit->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));

	rt.right = rt.left - 10;
	rt.left = rt.right - BUTTONWIDTH - 1;
	pBtUpdate = CreateButton(rt, this, "&CfgPtn.Upgrade", (CTRLPROC)&CPageUpdate::OnBtUpdate);
	pBtUpdate->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
    
	//rt.right = rt.left - 10;
	progWidth = rt.left - 10 -17 - 20;
	rt.left = 17;
	rt.right = rt.left+progWidth-10;
    rt.top += 2;
	rt.bottom = m_Rect.Height() - 13;
    
    percentInfo = CreateStatic(CRect(rt.left-1, rt.top-1, rt.right+1, rt.bottom+1), this, "");
	percentInfo->SetBkColor(VD_RGB(67,77,87));
    
	updateProg = CreateStatic(CRect(rt.left, rt.top, rt.right, rt.bottom), this, "");
	updateProg->SetBkColor(VD_RGB(8,66,189));	
	updateProg->SetTextAlign(VD_TA_CENTER);
    updateProg->SetBkWidth(0);
    //dateProg->SetText("0%");
	//progWidth = rt.right-rt.left;
	progRight = rt.right;

	stateInfo = CreateStatic(CRect(rt.left,rt.top,rt.left+progWidth/2,rt.bottom), this, "");
    stateInfo->Show(FALSE,TRUE);
}

CPageUpdate::~CPageUpdate()
{

}

void CPageUpdate::SetRemoteText(u8 nRslt)
{
	Clear();
	if(!nRslt)
		pName[0]->SetText("&CfgPtn.RemoteupgradeStart");
	else
		pName[0]->SetText("&CfgPtn.RemoteupgradeErr");	
}

void CPageUpdate::RemoteUpdateStart(u32 type, u32 nRslt) //cw_remote
{	
	u8 idx = 0;
	switch(type)
	{
		case EM_BIZ_EVENT_NET_STATE_UPDATEMAINBOARDSTART:
		case EM_BIZ_EVENT_NET_STATE_UPDATEPANNELSTART:
		case EM_BIZ_EVENT_NET_STATE_UPDATESTARTLOGOSTART:
		case EM_BIZ_EVENT_NET_STATE_UPDATEAPPLOGOSTART:
		{
			idx = type - EM_BIZ_EVENT_NET_STATE_UPDATEMAINBOARDSTART;
		} break;
		default:
			return;
	}
	
	if(!nRslt)
	{
		SetSystemLockStatus(1);
		m_RemoteUpFlag = 1;
		
		this->pCombo->SetCurSel(idx);
		
		this->SetUpdateProg(0);
		this->SetRemoteText(0);
	}
	else
	{
		SetSystemLockStatus(0);
		this->SetUpdateProg(0);
		this->SetRemoteText(1);
	}
}

int CPageUpdate::SetUpdateProg(int nProg)
{
	//formatProg->Show(TRUE,TRUE);
    /*
	if(nProg == 0)
	{
		nProg = 1;
	}
	*/
	if(nProg == -1)
	{
		nProg = 0;
	}
	
	int width = progWidth*nProg/100;

	/*
	CRect rt;
	rt.left = (m_screenRT.Width()-m_Rect.Width())/2+17;
	rt.top = (m_screenRT.Height()-m_Rect.Height())/2+m_Rect.Height()-35;
	rt.right = rt.left+width;
	rt.bottom = rt.top +23;
	updateProg->SetRect(rt,TRUE);
	*/

    if (width)
    {
	    updateProg->SetBkWidth(width);
        char szProg[5] = {0};

    	sprintf(szProg, "%d%%", nProg);
    	
    	updateProg->SetText(szProg);
    } 
    else 
    {
        updateProg->SetBkWidth(width);
        stateInfo->SetText("");
        stateInfo->Show(FALSE,TRUE);
        percentInfo->Show(TRUE,TRUE);
        updateProg->Show(TRUE,TRUE);
        updateProg->SetText("");
    }
}

void CPageUpdate::SetName(int row, char* szText)
{
	if( row<0 || row>7)
	{
		return;
	}

	pName[row]->SetText(szText);

}

void CPageUpdate::SetSize(int row, char* szText)
{
	if( row<0 || row>7)
	{
		return;
	}

	pSize[row]->SetText(szText);

}

void CPageUpdate::SetType(int row, char* szText)
{
	if( row<0 || row>7)
	{
		return;
	}

	pType[row]->SetText(szText);

}

void CPageUpdate::SetModifyDate(int row, char* szText)
{
	if( row<0 || row>7)
	{
		return;
	}

	pModifyDate[row]->SetText(szText);

}

void CPageUpdate::SetResult(char* szText)
{
	pszResult->SetText(szText);

}

void CPageUpdate::OnComboBox()
{
    curPage = 1;
	Refresh();
}

void CPageUpdate::OnClickPageCtr()
{
    #if 1
    //curPage
    if(curPage < 1)
	{
		return;
	}

    int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < 4; i++)
	{
		if (pFocusButton == pBtPageCtr[i])
		{
			bFind = TRUE;
			break;
		}
	}

    if (bFind)
        {
        //CleanUpBckCol();
		switch (i)
		{
			case 0:
				if(curPage > 1)
				{
                    printf("left most\n");
					curPage = 1;
                    Refresh();
				}
				break;

			case 1:
				if(curPage > 1)
				{
                    printf("left\n");
					curPage -= 1;
                    Refresh();
				}
				break;

			case 2:
				if(curPage < (m_nFile/8+(m_nFile%8?1:0)))
				{
                    printf("right\n");
					curPage += 1;
                    Refresh();
				}
				break;

			case 3:
				if(curPage < (m_nFile/8+(m_nFile%8?1:0)))
				{
                    printf("right most\n");
					curPage = (m_nFile/8+(m_nFile%8?1:0));
					Refresh();
				}
				break;

			default:
				break;
		} 				
	}
    #endif
}

void CPageUpdate::OnBtRefresh()
{
    curPage = 1;
	Refresh();
}

void CPageUpdate::OnBtUpdate()
{
	EMBIZUPGRADETARGET emUpdateTarget = EM_BIZUPGRADE_MAINBOARD;
    
	switch(pCombo->GetCurSel())
	{
		case 0:
			emUpdateTarget = EM_BIZUPGRADE_MAINBOARD;
			break;
		case 1:
			emUpdateTarget = EM_BIZUPGRADE_PANEL;
			break;
		case 2:
			emUpdateTarget = EM_BIZUPGRADE_STARTLOGO;
			break;
		case 3:
			emUpdateTarget = EM_BIZUPGRADE_APPLOGO;
			break;
		default:
			return;
	}
    
	if(m_nCursel>=0 && m_nCursel<8) //ROW_ITEM
	{
        //stateInfo->SetText("");
		if( strlen(pName[m_nCursel]->GetText())!=0 )
		{
			//SetUpdateProg(0);
			//stateInfo->Show(FALSE,TRUE);

            //percentInfo->Show(TRUE,TRUE);
            //updateProg->Show(TRUE,TRUE);
            SetUpdateProg(0);
            //updateProg->SetText("");

			Draw(); //if not draw, table title will disappear

			BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONTROL, BIZ_LOG_SLAVE_UPDATE);
			SetSystemLockStatus(1); //cw_shutdown
			int ret = BizSysComplexUpgrade(emUpdateTarget, (char*)pName[m_nCursel]->GetText());
			printf("PageUpdate.cpp %s over, ret: %d  yaogang\n", __func__, ret);
			fflush(stdout);
			if(ret!=0)
			{
				SetSystemLockStatus(0); //cw_shutdown
                percentInfo->Show(FALSE,TRUE);
                updateProg->Show(FALSE,TRUE);
				stateInfo->Show(TRUE,TRUE);
				stateInfo->SetText("&CfgPtn.UpdateFail");
			}
			else
			{
				printf("%s 1\n", __func__);
				fflush(stdout);
		                percentInfo->Show(FALSE,TRUE);
		                updateProg->Show(FALSE,TRUE);
				stateInfo->Show(TRUE,TRUE);
				stateInfo->SetText("&CfgPtn.UpdateOk");

				printf("%s 2\n", __func__);
				fflush(stdout);
				
				CRect infoRt = CRect(0, 0, 480, 300);
				CPageInfoBox infoBox(&infoRt, "&CfgPtn.Info", "&CfgPtn.UpdateOkReset", NULL, NULL);
				
				printf("%s 3\n", __func__);
				fflush(stdout);
				
				infoBox.Open();
				
				printf("%s 4\n", __func__);
				fflush(stdout);
				
				usleep(1000*1000);
				
				printf("update finish restart ! \n");
				
				BizSysComplexExit(EM_BIZSYSEXIT_RESTART);
				printf("%s 5\n", __func__);
				fflush(stdout);
				//infoBox.Close();
			}
			//SetUpdateProg(-1);
			//stateInfo->Show(TRUE,TRUE);
			//usleep(2000*1000);
			//stateInfo->SetText("");
		}
	}
}

void CPageUpdate::OnBtExit()
{
	this->Close();
}

VD_BOOL CPageUpdate::UpdateData( UDM mode )
{
#if 1
	if (UDM_OPEN == mode)
	{
		pCombo->SetCurSel(0);
		curPage = 1;
		
		stateInfo->SetText("");
		updateProg->SetText("");
		if(!m_RemoteUpFlag)
		{
			Refresh();
		}
        pBtUpdate->Enable(FALSE);
        m_RemoteUpFlag = 0;
	} 
	else if (UDM_CLOSED == mode) 
	{
	    ((CPageMainFrameWork *)(this->GetParent()))->ReflushItemName();
        pBtUpdate->Enable(FALSE);
    }
#endif

	return TRUE;
}


s32 BizSysComplexGetUpdateFileInfo(EMBIZUPGRADETARGET emTgt, SBizUpdateFileInfoList* pInfo)
{
	pInfo->nFileNum = 0;
	
	return 0;
}

void CPageUpdate::Refresh()
{
	char szTmp[20] = {0};
    int nID = 0;
	SBizUpdateFileInfoList sUFInfoLst;
	memset(&sUFInfoLst, 0, sizeof(sUFInfoLst));
    pBtUpdate->Enable(FALSE);
	Clear(); //clean all the data
	ClearBkColor();

	sUFInfoLst.nMax = 128;
	SBizUpdateFileInfo sFileInfoList[sUFInfoLst.nMax];  //zlb20111117  去掉部分malloc
	sUFInfoLst.pInfo = sFileInfoList;

	//printf("CPageUpdate::Refresh 0 \n");
	if(sUFInfoLst.pInfo)
	{
		EMBIZUPGRADETARGET eType;
		//printf("CPageUpdate::Refresh 1 \n");
		switch(pCombo->GetCurSel())
		{
			case 0:
			{
				eType = EM_BIZUPGRADE_MAINBOARD;
			}break;
			case 1:
			{
				eType = EM_BIZUPGRADE_PANEL;
			}break;
			case 2:
			{
				eType = EM_BIZUPGRADE_STARTLOGO;
			}break;
			case 3:
			{
				eType = EM_BIZUPGRADE_APPLOGO;
			}break;
			default:
				return;
		}

		BizSysComplexUpgradeFilelist(eType, &sUFInfoLst);
		
        m_nFile = sUFInfoLst.nFileNum;
        if (0 == m_nFile) {
            curPage = 0;
        }
        
		for(int i=0; i<8/*sUFInfoLst.nFileNum*/; i++)
		{
            nID = (curPage-1)*8+i;
            if (nID >= m_nFile) {
                break;
            }
            if ((strcasecmp(sUFInfoLst.pInfo[nID].szName, "") != 0) && (sUFInfoLst.pInfo[nID].nSize > 0))
            {
    			pName[i]->SetText(sUFInfoLst.pInfo[nID].szName);
    			pType[i]->SetText(sUFInfoLst.pInfo[nID].szExt);
    			
    			memset(szTmp, 0, sizeof(szTmp));
				sprintf(szTmp, "%d%s", sUFInfoLst.pInfo[nID].nSize, (char*)GetParsedString("&CfgPtn.BYTE"));
    			pSize[i]->SetText(szTmp);
				
				//csp modify
    			//struct tm *tmptime = localtime((time_t*)&sUFInfoLst.pInfo[nID].nTimeStamp);
				struct tm tm0;
				struct tm *tmptime = &tm0;
				localtime_r((time_t*)&sUFInfoLst.pInfo[nID].nTimeStamp, tmptime);
				
    			memset(szTmp, 0, sizeof(szTmp));
    			sprintf(szTmp, "%04d-%02d-%02d", 
    					tmptime->tm_year+1900,
    					tmptime->tm_mon+1,
    					tmptime->tm_mday
    			);
    			
    			pModifyDate[i]->SetText(szTmp);
            }
		}
        //printf("***Total:[%d]\n", sUFInfoLst.nFileNum);		        
		
		memset(szTmp, 0, sizeof(szTmp));
		sprintf(szTmp, "%d/%d", curPage, (m_nFile%8)?(m_nFile/8+1):(m_nFile/8));

		pszResult->SetText(szTmp);
	}

	//free(sUFInfoLst.pInfo); //zlb20111117  去掉部分malloc
		
	return;
}

int CPageUpdate::GetCurFileNum()//cw_tab
{
	int num;
	if (m_nFile<8)
		num= m_nFile;
	else 
	{
		if(curPage*8>m_nFile)
			num =m_nFile-((curPage-1)*8);
		else
			num= 8;
	}
	return num;
}


VD_BOOL CPageUpdate::MsgProc( uint msg, uint wpa, uint lpa )
{
        u8 lock_flag = 0;  //cw_lock
        static int chSel=0;
        GetSystemLockStatus(&lock_flag);
        if(lock_flag)
        {
        	return FALSE;
        }
	CItem*tmp=GetFocusItem();
	int px,py;
	int temp;
	switch(msg)
	{
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
					if ((tmp!=NULL)&&(tmp!=pCombo))
					{
						chSel=wpa -KEY_0;
						if(chSel<=GetCurFileNum()&&chSel>=KEY_0)
						{
							SelectDskItem(chSel-1);
						}
						return TRUE;
					}
				}break;
				
				case KEY_UP:
				{
					if ((tmp!=NULL)&&(tmp!=pCombo))
					{
						if (--chSel>=KEY_0)
							SelectDskItem(chSel-1);
						else
						{	
							chSel=GetCurFileNum();
							SelectDskItem(GetCurFileNum()-1);
						}
						return TRUE;
					}
				}break;
				
				case KEY_DOWN:
				{
					if ((tmp!=NULL)&&(tmp!=pCombo))
					{
						if (++chSel<=GetCurFileNum())
							SelectDskItem(chSel-1);
						else
						{
							chSel=KEY_0;
							SelectDskItem(chSel-1);
						}
						return TRUE;
					}
				}break;
				default: break;
			}
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
			for(i = 0; i<UPDATE_FILE_MAX; i++)
			{
				for (int kk=0; kk<ROW_ITEM; kk++)
				{
					VD_RECT rt;
					pTab[0]->GetTableRect(kk, i+1, &rt);
					rt.left +=17+offsetx;
					rt.top +=97+offsety;//-CTRL_HEIGHT;
					rt.right +=17+offsetx;
					rt.bottom +=97+offsety;//-CTRL_HEIGHT;
					//printf("i=%d kk =%d ,x=%d, y=%d, %d %d %d %d \n",i, kk, px, py, rt.left, rt.top, rt.right,rt.bottom);
					if (PtInRect(&rt, px, py))
					{
						//printf("found point %d\n", kk);
						bFind = TRUE;
						goto SELITEM;
					}

				}

			}

SELITEM:
			if (bFind)
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


void CPageUpdate::Clear()
{
	for(int i=0; i<UPDATE_FILE_MAX; i++)
	{
		pName[i]->SetText("");
		pSize[i]->SetText("");
		pType[i]->SetText("");
		pModifyDate[i]->SetText("");
	}
}

void CPageUpdate::ClearBkColor()
{
    if (m_nCursel >=0)
	{
		pName[m_nCursel]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pName[m_nCursel]->Draw();
		pSize[m_nCursel]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pSize[m_nCursel]->Draw();
		pType[m_nCursel]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pType[m_nCursel]->Draw();
		pModifyDate[m_nCursel]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pModifyDate[m_nCursel]->Draw();
	}
	
	m_nCursel = -1;
}

void CPageUpdate::SelectDskItem( int index )
{
	if (index < 0 || index>=UPDATE_FILE_MAX )
	{
		return;
	}

	//printf(" idx=%d, cur =%d \n", index, m_nCursel);
	if ((index == m_nCursel) || 
        (strcasecmp(pName[index]->GetText(), "") == 0) ||
        (strtol(pSize[index]->GetText(), 0, 10) <= 0))
	{
		return;
	}

	if (m_nCursel >=0)
	{
		pName[m_nCursel]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pName[m_nCursel]->Draw();
		pSize[m_nCursel]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pSize[m_nCursel]->Draw();
		pType[m_nCursel]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pType[m_nCursel]->Draw();
		pModifyDate[m_nCursel]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pModifyDate[m_nCursel]->Draw();
	}
	
	pName[index]->SetBkColor(VD_RGB(56,108,148));
	pName[index]->Draw();
	pSize[index]->SetBkColor(VD_RGB(56,108,148));
	pSize[index]->Draw();
	pType[index]->SetBkColor(VD_RGB(56,108,148));
	pType[index]->Draw();
	pModifyDate[index]->SetBkColor(VD_RGB(56,108,148));
	pModifyDate[index]->Draw();
    
	m_nCursel = index;
    pBtUpdate->Enable(TRUE);
}

