#include "GUI/Pages/PagePresetSetup.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/PagePtzPreset.h"
#include "GUI/Pages/BizData.h"

static	MAINFRAME_ITEM	items[0];

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static VD_BITMAP * pBmp_tab_normal;
static VD_BITMAP * pBmp_tab_push;
static VD_BITMAP * pBmp_tab_select;

void bizData_GetPtzAdvancedPara(uchar nChn, SGuiPtzAdvancedPara* pGDispatch);
int bizData_SavePtzAdvancedPara(uchar nChn, SGuiPtzAdvancedPara* pGDispatch);

static int GetMaxChnNum() { return GetVideoMainNum(); }

CPagePresetSetup::CPagePresetSetup( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
, curID(0),m_page(0)
{
	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);    
    nChMax = GetMaxChnNum();
    for(int i=0; i<nChMax; ++i)
    {
        m_ChValue[i] = (uchar *)malloc(sizeof(uchar)*MY_UCHAR_MAX);
    }
	
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_tab_normal = VD_LoadBitmap(DATA_DIR"/temp/tab.bmp");
	pBmp_tab_push = VD_LoadBitmap(DATA_DIR"/temp/tab_check.bmp");
	pBmp_tab_select = VD_LoadBitmap(DATA_DIR"/temp/tab_f.bmp");

	CRect rtSubPage(m_Rect.left+10, 
					m_Rect.top+10, 
					m_Rect.Width()-10,
					m_Rect.top+340);

	pTable = CreateTableBox(&rtSubPage,this, 4,11,0);
	pTable->SetColWidth(0,60);
	pTable->SetColWidth(1,120);
	pTable->SetColWidth(2,0/*260*/);
	
	char* szTitle[4] = 
	{
		"&CfgPtn.ID",
		"&CfgPtn.Enable",
		"",//"&CfgPtn.Name"
		"&CfgPtn.Preset"
	};
	
	CRect tmpRt;
	for(int i=0; i<4; i++)
	{
		pTable->GetTableRect(i,0,&tmpRt);
		pTitle[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, szTitle[i]);
		pTitle[i]->SetBkColor(VD_RGB(67,77,87));
	}
    pTitle[2]->Show(FALSE,TRUE);

	for(int i=0; i<10; i++)
	{
		pTable->GetTableRect(0,i+1,&tmpRt);
		pID[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, "");
	}

    //add by Lirl on Nov/23/2011
    //全选显示
    pTable->GetTableRect(0,10+1,&tmpRt);
    pSelAll = CreateStatic(CRect(rtSubPage.left+tmpRt.left, m_Rect.Height()-7-25-m_TitleHeight, rtSubPage.left+tmpRt.right,m_Rect.Height()-7-m_TitleHeight), this, "&CfgPtn.All");
    pSelAll->Show(FALSE,TRUE);
    //end

	for(int i=0; i<10; i++)
	{
		pTable->GetTableRect(1,i+1,&tmpRt);
		pEnable[i] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+4,
                                    rtSubPage.left+tmpRt.left+22,rtSubPage.top+tmpRt.top+24),
                                    this,
                                    styleEditable,
                                    (CTRLPROC)&CPagePresetSetup::OnCheckBox
                                    );
	}

    //add by Lirl on Nov/23/2011
    //全选框
    pTable->GetTableRect(1,10+1,&tmpRt);
    pAllEnable[1] = CreateCheckBox(
		CRect(rtSubPage.left+tmpRt.left+2, m_Rect.Height()-7-25-m_TitleHeight, rtSubPage.left+tmpRt.left+22, m_Rect.Height()-7-m_TitleHeight), 
		this );
    pAllEnable[1]->Show(FALSE,TRUE);
    //end

    #if 1
	for(int i=0; i<10; i++)
	{
		pTable->GetTableRect(2,i+1,&tmpRt);
		int vLen = (tmpRt.right-tmpRt.left-4)/TEXT_WIDTH*2;
		pName[i] = CreateEdit(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+3, 
									rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-3), 
									this,vLen,0,NULL/*(CTRLPROC)&CPageBasicConfigFrameWork::OnEditChange0*/);
		pName[i]->SetBkColor(VD_RGB(67,77,87));
        pName[i]->Show(FALSE,TRUE);
		//pName[i]->Enable(FALSE);
		

	}
    #endif

	for(int i=0; i<10; i++)
	{
		pTable->GetTableRect(3,i+1,&tmpRt);
		int btWidth = TEXT_WIDTH*3 + 20;
		pPreset[i] = CreateButton(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+3, 
									rtSubPage.left+tmpRt.left+btWidth+2,rtSubPage.top+tmpRt.top+25),
									this, 
									"&CfgPtn.Setting", 
									(CTRLPROC)&CPagePresetSetup::OnClickPreset, NULL, buttonNormalBmp);
		pPreset[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);

	}

    //add by Lirl on Nov/23/2011
    //是否开启全选框
    pTable->GetTableRect(3,10+1,&tmpRt);
    pAllEnable[0] = CreateCheckBox(
		CRect(rtSubPage.left+tmpRt.left+2, m_Rect.Height()-7-25-m_TitleHeight, rtSubPage.left+tmpRt.left+22, m_Rect.Height()-7-m_TitleHeight), 
		this );
    pAllEnable[0]->Show(FALSE,TRUE);
    //end
	
	char* szBtn[2] = 
	{
		"&CfgPtn.OK",
		"&CfgPtn.Exit",
	};
	
	int btWidth = TEXT_WIDTH*2+20;
	CRect btnRt(m_Rect.Width()-2*btWidth-20,
				m_Rect.Height()-8-25-m_TitleHeight,
				m_Rect.Width()-2*btWidth-20+btWidth,
				m_Rect.Height()-8-25+24-m_TitleHeight);

	for(int i=0; i<2; i++)
	{	
		pBtnCtl[i] = CreateButton(btnRt,this, szBtn[i], (CTRLPROC)&CPagePresetSetup::OnClickCtlBtn, NULL, buttonNormalBmp);
		pBtnCtl[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);

		btnRt.left = btnRt.right + 10;
		btnRt.right = btnRt.left + btWidth;
	}	

	pScrollbar = CreateScrollBar(CRect(m_Rect.Width()-10-25,
										41,
										m_Rect.Width()-10,
										m_Rect.top+340), 
										this,
										scrollbarY,
									  	0,
									  	120, 
									  	1, 
									  	(CTRLPROC)&CPagePresetSetup::OnTrackMove);


	CRect m_RectScreen;
	m_pDevGraphics = CDevGraphics::instance(m_screen);
	m_pDevGraphics->GetRect(&m_RectScreen);
	
	CRect rtFloat;
	rtFloat.left = (m_RectScreen.Width()-456)/2;
	rtFloat.right = rtFloat.left + 456;
	rtFloat.top = m_RectScreen.bottom - 212 ;
	rtFloat.bottom = rtFloat.top + 182;
	
	//printf("CPagePresetSetup\n");
	
	m_pPagePtzPreset = new CPagePtzPreset(rtFloat, NULL , icon_dvr, this);
}

void CPagePresetSetup::OnClickCtlBtn()
{
    printf("onClickCtlBtn\n");

    int i = 0;
    BOOL    bFind = FALSE;
    CButton *pFocusButton = (CButton *)GetFocusItem();

    for (i = 0; i < 2; i++)
    {
        if (pFocusButton == pBtnCtl[i])
        {
            bFind = TRUE;
            break;
        }
    }

    if (bFind)
    {
        switch (i)
        {
            case 0: //OK
            {
                printf("ok\n");
                //add by Lirl on Nov/23/2011
                if (pAllEnable[0]->GetValue()) {
                    for (int i=0; i<TOURPIONT_MAX; ++i) {
                        psGuiPtzPara.nIsPresetSet[i] = pAllEnable[1]->GetValue();
                    }
                    
                    for (int i=0; i<10; ++i) {
                		int nID = m_page*10+i;
                        pEnable[i]->SetValue(psGuiPtzPara.nIsPresetSet[nID]);
                	}
                }
                //end
                
                bizData_SavePtzAdvancedPara(m_nChn, &psGuiPtzPara);
                break;
            }
            case 1: //exit
            {
                printf("exit\n");
                this->m_pParent->Open();
                return (void)CPage::Close();
                break;
            }
            default:
                break;
        }
    }
}

void CPagePresetSetup::OnCheckBox()
{
       printf("OnCheckBox\n");
       m_page = pScrollbar->GetPos();

       int i = 0;
       BOOL bFind = FALSE;
       CCheckBox *pFocusButton = (CCheckBox *)GetFocusItem();

       for (i = 0; i < 10; i++)
       {
            if (pFocusButton == pEnable[i])
            {
                int nID = m_page*10+i;
                printf("CheckBox: %d - %d\n", i, pEnable[i]->GetValue());
                #if 0
                if (1 == pEnable[i]->GetValue())
                    pPreset[i]->Enable(TRUE);
                else
                    pPreset[i]->Enable(FALSE);
                #endif
                psGuiPtzPara.nIsPresetSet[nID] = pEnable[i]->GetValue();
                break;
            }
       }
}

VD_BOOL CPagePresetSetup::Close(UDM mode)
{
    if (!bChileClose) {
        this->m_pParent->Open();
    }
    return CPage::Close();
}


VD_BOOL CPagePresetSetup::UpdateData( UDM mode )
{
    if (UDM_OPEN == mode)
	{
		bizData_GetPtzAdvancedPara(m_nChn, &psGuiPtzPara);
		
        #if 0
        for (int i=30; i < 66; ++i)
        {
            printf("***checkbox:[%d]:[%d]\n", i, psGuiPtzPara.nIsPresetSet[i]);
        }
        #endif

        //add by Lirl on Nov/23/2011
        pAllEnable[0]->SetValue(0);
        pAllEnable[1]->SetValue(0);
        //end

		LoadPage();
        bChileClose = FALSE;
	}
	
	return TRUE;
}


void CPagePresetSetup::OnTrackMove()
{
	
	int pos = pScrollbar->GetPos();
    if (m_page==pos) {
        return;
    }
    m_page = pos;

	for(int i=0; i<10; i++)
	{
		char szID[16] = {0};
		int nID = m_page*10+i;
		sprintf(szID,"%d",nID+1);
		pID[i]->SetText(szID);
        //pEnable[i]->SetValue((int)m_ChValue[m_nChn][nID]);
        pEnable[i]->SetValue(psGuiPtzPara.nIsPresetSet[nID]);

        /*
		char szName[32] = {0};
		sprintf(szName,"preset%d",nID+1);
		pName[i]->SetText(szName);
		*/
	}

	if(m_page == 12)
	{
		for(int i=8; i<10; i++)
		{
			pID[i]->Show(FALSE,TRUE);
			pEnable[i]->Show(FALSE,TRUE);
			//pName[i]->Show(FALSE,TRUE);
			pPreset[i]->Show(FALSE,TRUE);
		}
	}
	else
	{
		for(int i=8; i<10; i++)
		{
			pID[i]->Show(TRUE,TRUE);
			pEnable[i]->Show(TRUE,TRUE);
			//pName[i]->Show(TRUE,TRUE);
			pPreset[i]->Show(TRUE,TRUE);
		}
	}

}

void CPagePresetSetup::LoadPage()
{
	
	m_page = pScrollbar->GetPos();

	for(int i=0; i<10; i++)
	{
		char szID[16] = {0};
		int nID = m_page*10+i;
		sprintf(szID,"%d",nID+1);
		pID[i]->SetText(szID);
        //pEnable[i]->SetValue((int)m_ChValue[m_nChn][nID]);
        pEnable[i]->SetValue(psGuiPtzPara.nIsPresetSet[nID]);

        /*
		char szName[32] = {0};
		sprintf(szName,"preset%d",nID+1);
		pName[i]->SetText(szName);
		*/
	}

	if(m_page == 12)
	{
		for(int i=8; i<10; i++)
		{
			pID[i]->Show(FALSE,TRUE);
			pEnable[i]->Show(FALSE,TRUE);
			//pName[i]->Show(FALSE,TRUE);
			pPreset[i]->Show(FALSE,TRUE);
		}
	}
	else
	{
		for(int i=8; i<10; i++)
		{
			pID[i]->Show(TRUE,TRUE);
			pEnable[i]->Show(TRUE,TRUE);
			//pName[i]->Show(TRUE,TRUE);
			pPreset[i]->Show(TRUE,TRUE);
		}
	}

}


void CPagePresetSetup::OnClickPreset()
{
	printf("OnClickPreset \n");
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < 10; i++)
	{
		if (pFocusButton == pPreset[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
        /*
		SPresetPara sPIns;

		memset(&sPIns, 0, sizeof(sPIns));
		memcpy(sPIns.nId, psGuiPtzPara.nIsPresetSet, 128);
		
		for(int i=0; i<nChMax; i++)
			m_pPagePtzPreset->OnSetPara(i,&sPIns);
		*/
		SetSystemLockStatus(1); //cw_lock
		u32 nSelNum = strtol(pID[i]->GetText(), 0, 10);
        curID = nSelNum - 1;
        //把当前通道号及编号传给子窗口
		m_pPagePtzPreset->ParentSendPara(m_nChn, curID);
        bChileClose = TRUE;
	    this->Close(UDM_CLOSED);
        bChileClose = FALSE;
		m_pPagePtzPreset->Open();
		SetSystemLockStatus(0); //cw_lock
	}
	

}



CPagePresetSetup::~CPagePresetSetup()
{
	
}

VD_PCSTR CPagePresetSetup::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

void CPagePresetSetup::SetChn(int chn)
{
	m_nChn = chn;
	
	char title[256] = {0};//char title[64] = {0};//csp modify
	char *s0[2] = {
		"&CfgPtn.Preset",
		"&CfgPtn.Channel"
	};
	sprintf(title, "%s - %s%d", GetParsedString(s0[0]), GetParsedString(s0[1]), m_nChn+1);
	
	SetTitle(title);
}

void CPagePresetSetup::OnSetPara(uchar nChn, uchar * pPchar)
{
    if (pPchar&&nChn<=nChMax)
    {
        memcpy(m_ChValue[nChn], pPchar, sizeof(m_ChValue));
    }
    return;
}

void CPagePresetSetup::OnGetPara(uchar nChn, uchar * pPchar)
{
    if (pPchar&&nChn<=nChMax)
    {
        memcpy(pPchar, m_ChValue[nChn], sizeof(m_ChValue));
    }
    return;
}


VD_BOOL CPagePresetSetup::MsgProc( uint msg, uint wpa, uint lpa )
{
	//printf("CPageFrame::MsgProc\n");
     u8 lock_flag = 0;  //cw_shutdown
     GetSystemLockStatus(&lock_flag);
     if(lock_flag)
     {
     	return FALSE;
     }
	int px,py;
	int temp;
	switch(msg)
	{
	case XM_KEYDOWN:
		{
			if(wpa==KEY_PTZ)
				return FALSE;
			else
				break;
		}
	case XM_MOUSEWHEEL:   //cw_scrollbar
		{
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);
			CItem* pItem = GetItemAt(px, py);
			if(pItem == NULL)
			{
				//if(GetMaxChnNum()>10)
				{
					if (curID == 0)
					{
						pScrollbar->MsgProc(msg, wpa, lpa);
						break;
					}	
				}
			}
			break;
		}
	case XM_RBUTTONDOWN:
	case XM_RBUTTONDBLCLK:
		printf("OnPresetSetup \n");
		//this->m_pParent->GetParent()->Open();
		//((CPagePtzConfigFrameWork *)(this->GetParent()))->SetOpenFlag(TRUE);		
		this->m_pParent->Open();
        //this->Close(UDM_CLOSED);
        return CPage::Close();
		
		break;
	default:
		//return 0;
		break;
	}

	//return 	CPage::MsgProc(msg, wpa, lpa);
	return CPage::MsgProc(msg, wpa, lpa);

}


