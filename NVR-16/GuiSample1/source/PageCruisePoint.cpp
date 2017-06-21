#include "GUI/Pages/PageCruisePoint.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageCruiseLine.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PagePtzCtl.h"
char* cruisePointShortcutBmpName[11][2] = {    
	{DATA_DIR"/temp/listex_btn_leftmost.bmp",	DATA_DIR"/temp/listex_btn_leftmost_f.bmp"},
	{DATA_DIR"/temp/listex_btn_left.bmp",	DATA_DIR"/temp/listex_btn_left_f.bmp"},
	{DATA_DIR"/temp/listex_btn_right.bmp",	DATA_DIR"/temp/listex_right_f.bmp"},
	{DATA_DIR"/temp/listex_btn_rightmost.bmp",	DATA_DIR"/temp/listex_btn_rightmost_f.bmp"},
	{DATA_DIR"/temp/btn_add.bmp",	DATA_DIR"/temp/btn_add_f.bmp"},
	{DATA_DIR"/temp/btn_del.bmp",	DATA_DIR"/temp/btn_del_f.bmp"},
	{DATA_DIR"/temp/btn_modify.bmp",	DATA_DIR"/temp/btn_modify_f.bmp"},
	{DATA_DIR"/temp/btn_top.bmp",	DATA_DIR"/temp/btn_top_f.bmp"},
	{DATA_DIR"/temp/btn_up.bmp",	DATA_DIR"/temp/btn_up_f.bmp"},
	{DATA_DIR"/temp/btn_down.bmp",	DATA_DIR"/temp/btn_down_f.bmp"},
	{DATA_DIR"/temp/btn_bottom.bmp",	DATA_DIR"/temp/btn_bottom_f.bmp"},
};

static VD_BITMAP* pBmpButtonNormal[11];
static VD_BITMAP* pBmpButtonSelect[11];


static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

#define PRESET_START 5
#define PRESET_END 13
#define RATE_START 13
#define RATE_END 21
#define TIME_START 21
#define TIME_END 29

//static VD_BITMAP * pBmp_infoicon;

CPageCruisePoint::CPageCruisePoint( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
{
	bFirstSel = TRUE;
	
	m_nTotal = 0;
    bPrFlag = FALSE;
    m_nCurMouseLine = 0;
    bPreviewStatus = FALSE;
	
	this->pParent = pParent;
	
	nChMax = GetVideoMainNum();
    eMaxViewMode = EM_BIZPREVIEW_4SPLITS;
    switch(nChMax)
	{
		case 4:
		{
			eMaxViewMode = EM_BIZPREVIEW_4SPLITS;
		} break;
		case 8:
		{
			eMaxViewMode = EM_BIZPREVIEW_9SPLITS;
		} break;
		//csp modify 20130504
		case 10:
		{
			eMaxViewMode = EM_BIZPREVIEW_10SPLITS;
		} break;
		case 6:
		{
			eMaxViewMode = EM_BIZPREVIEW_6SPLITS;
		} break;
		case 16:
		{
			eMaxViewMode = EM_BIZPREVIEW_16SPLITS;
		} break;
		case 24:
		{
			//eMaxViewMode = EM_BIZPREVIEW_24SPLITS;
			eMaxViewMode = EM_BIZPREVIEW_25SPLITS;
		} break;
        default:
            break;
    }
    
	//for(int i=0; i<nChMax; i++)
	//{
    psGuiTourPath = (SGuiTourPath*)malloc(sizeof(SGuiTourPath)*TOURPATH_MAX);
    if(psGuiTourPath==NULL)
    {
    exit(1);
    }
    memset(psGuiTourPath, 0, sizeof(SGuiTourPath)*TOURPATH_MAX);
	//}
	
	//SetRect(CRect(0, 0, 700, 400/*370*/), FALSE);
	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);

	m_nCursel = 0;

	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	CRect rtTemp;
	rtTemp.left = 10;
	rtTemp.top = 10;
	rtTemp.right = rtTemp.left + 40;
	rtTemp.bottom = rtTemp.top + 20;
	pStatic[0] = CreateStatic(rtTemp,this, "&CfgPtn.ID");
	
	rtTemp.left = rtTemp.right;
	rtTemp.right = rtTemp.left + 100;
	rtTemp.bottom = rtTemp.top + 25;
	pComboBox = CreateComboBox(rtTemp,this, NULL, NULL, (CTRLPROC)&CPageCruisePoint::OnComboBox, 0);
	
	rtTemp.left = rtTemp.right + 20;
	rtTemp.right = rtTemp.left + 60;
	rtTemp.bottom = rtTemp.top + 20;
	pStatic[1] = CreateStatic(rtTemp,this, "&CfgPtn.Name");
    pStatic[1]->Show(0);


	rtTemp.left = rtTemp.right + 20;
	rtTemp.right = rtTemp.left + 200;
	rtTemp.bottom = rtTemp.top + 25;
	int vLen = (rtTemp.right-rtTemp.left-4)/TEXT_WIDTH*2;
	pEdit = CreateEdit(rtTemp, this,vLen,0,(CTRLPROC)&CPageCruisePoint::OnEditChange);
    pEdit->Enable(FALSE);
    pEdit->Show(0);

	char* szBtn[3] = 
	{
		"&CfgPtn.Preview",
		"&CfgPtn.OK",
		"&CfgPtn.Exit",

	};

	char* szStatic[4] =
	{
		"&CfgPtn.Preset",
		"&CfgPtn.Speed",
		"&CfgPtn.Time",
		"0/0",
	};


	rtTemp.left = 10;
	rtTemp.top = 43;
	rtTemp.right = m_Rect.Width() - 45;
	rtTemp.bottom = rtTemp.top + 28*9;
	CRect tableRt(rtTemp.left,rtTemp.top,rtTemp.right,rtTemp.bottom);
	pTable0 = CreateTableBox(rtTemp, this, 3, 9);

	rtTemp.left = 10;
	rtTemp.top = rtTemp.bottom-1;
	rtTemp.right = m_Rect.Width() - 45;
	rtTemp.bottom = rtTemp.top + 30;
	pTable1 = CreateTableBox(rtTemp, this, 1, 1);

	int i = 0;
	CRect rt;
	for(i=0; i<3; i++)
	{
		pTable0->GetTableRect(i, 0, &rt);
		pStatic[i+2] = CreateStatic(CRect(tableRt.left+rt.left+2, tableRt.top+rt.top, tableRt.left+rt.right,tableRt.top+rt.bottom),
									this, szStatic[i]);
		pStatic[i+2]->SetBkColor(VD_RGB(67,77,87));
	}

	for(i=PRESET_START; i<PRESET_END; i++)
	{
		pTable0->GetTableRect(0, i-4, &rt);
		pStatic[i] = CreateStatic(CRect(tableRt.left+rt.left, tableRt.top+rt.top, tableRt.left+rt.right,tableRt.top+rt.bottom),
									this, "");
	}

	for(i=RATE_START; i<RATE_END; i++)
	{
		pTable0->GetTableRect(1, i-12, &rt);
		pStatic[i] = CreateStatic(CRect(tableRt.left+rt.left, tableRt.top+rt.top, tableRt.left+rt.right,tableRt.top+rt.bottom),
									this, "");
	}

	for(i=TIME_START; i<TIME_END; i++)
	{
		pTable0->GetTableRect(2, i-20, &rt);
		pStatic[i] = CreateStatic(CRect(tableRt.left+rt.left, tableRt.top+rt.top, tableRt.left+rt.right,tableRt.top+rt.bottom),
									this, "");
	}

	pTable1->GetTableRect(0, 0, &rt);
	pStatic[29] = CreateStatic(CRect(rtTemp.left+rt.left+2, rtTemp.top+rt.top, rtTemp.left+50,rtTemp.top+rt.bottom),
									this, szStatic[3]);
	

	for(i=0; i<4; i++)
	{
		pTable1->GetTableRect(0, 0, &rt);

		pBmpButtonNormal[i] = VD_LoadBitmap(cruisePointShortcutBmpName[i][0]);
		pBmpButtonSelect[i] = VD_LoadBitmap(cruisePointShortcutBmpName[i][1]);
		pButton[i] = CreateButton(CRect(rtTemp.left+rt.Width()-45*(4-i), rtTemp.top+rt.top+1, rtTemp.left+rt.Width()-45*(4-i)+pBmpButtonNormal[i]->width,rtTemp.top+rt.top+pBmpButtonNormal[i]->height),
									this, NULL, (CTRLPROC)&CPageCruisePoint::OnClickBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
	}

	rtTemp.left = m_Rect.Width()- 90*3 - 5;
	rtTemp.top = rtTemp.bottom + 15;
	rtTemp.right = rtTemp.left + 80;
	rtTemp.bottom = rtTemp.top + 25 - 2;
	for(i=4; i<7; i++)
	{
		pButton[i] = CreateButton(rtTemp, this, szBtn[i-4], (CTRLPROC)&CPageCruisePoint::OnClickBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);

		rtTemp.left = rtTemp.right + 10;
		rtTemp.right = rtTemp.left + 80;
	}
    pButton[4]->Show(FALSE);
	
	rtTemp.left = m_Rect.Width()- 30 - 10;
	rtTemp.top = 75;
	rtTemp.right = rtTemp.left + 30;
	rtTemp.bottom = rtTemp.top + 25;
	
	for(i=7; i<14; i++)
	{
		pBmpButtonNormal[i-3] = VD_LoadBitmap(cruisePointShortcutBmpName[i-3][0]);
		pBmpButtonSelect[i-3] = VD_LoadBitmap(cruisePointShortcutBmpName[i-3][1]);

		pButton[i] = CreateButton(rtTemp,this, NULL, (CTRLPROC)&CPageCruisePoint::OnClickBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmpButtonNormal[i-3], pBmpButtonSelect[i-3], pBmpButtonSelect[i-3]);

		rtTemp.top = rtTemp.bottom + 10;
		rtTemp.bottom = rtTemp.top + 25;
	}

    CRect rtSub1(295, m_Rect.Height()-80, m_Rect.Width()-20, m_Rect.Height()-80+pBmp_button_normal->height);

	pInfoBar = CreateStatic(CRect(40, 
								rtSub1.top,
								317,
								rtSub1.top+22), 
							this, 
							"");
    pInfoBar->SetTextAlign(VD_TA_CENTER);
    #if 0
	pBmp_infoicon = VD_LoadBitmap(DATA_DIR"/temp/icon_info.bmp");
	pIconInfo = CreateStatic(CRect(17, 
								rtSub1.top+1,
								37,
								rtSub1.top+21), 
							this, 
							"");
	pIconInfo->SetBitmap(pBmp_infoicon,NULL);
	pIconInfo->Show(FALSE);
    #endif

    #if 1
	CRect m_RectScreen;
	
	m_pDevGraphics = CDevGraphics::instance(m_screen);
	m_pDevGraphics->GetRect(&m_RectScreen);

	nLeftGap = (m_RectScreen.right-m_Rect.Width())>>1;
	nTopGap = (m_RectScreen.bottom-m_Rect.Height())>>1;
    #endif

	pButton[7]->GetRect(&rtAdd);
	
	rtAdd.right 	 = nLeftGap + rtAdd.right - 30;
	
	rtAdd.left	 = rtAdd.right-190;
	
	rtAdd.top		 = nTopGap + rtAdd.bottom;
	rtAdd.bottom 	 = rtAdd.top + 140 - 10;

	pAddCruisePoint = new CPageAddCruisePoint(rtAdd, NULL, NULL, this);

	pButton[9]->GetRect(&rtMod);
	rtMod.right 	 = nLeftGap + rtMod.right;	
	rtMod.left	 	 = rtMod.right-200;	
	rtMod.top		 = nTopGap + rtMod.bottom;
	rtMod.bottom 	 = rtMod.top + 140;
	
}

void CPageCruisePoint::OnEditChange()
{
	printf("OnEditChange  \n");

}


void CPageCruisePoint::OnClickBtn()
{
	//printf("OnClickSubPage \n");


	int i = 0;
    nSelLine = pComboBox->GetCurSel();
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < 14; i++)
	{
		if (pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
		switch (i)
		{
            static int snPos = 1;
			case 0:
			{
				//printf("left most \n");
                if (0 != m_nCurPage) 
                {
                    m_nCurPage = 0;
                    RefreshList();
                }
                else
                {
                    //CancelSel();
                    return;
                }
			}break;
			case 1:
			{
				//printf("left \n");
                if (m_nCurPage > 0 && m_nCurPage <= m_nMaxPage) 
                {
                    --m_nCurPage;
                    RefreshList();
                } 
                else
                {
                    //CancelSel();
                    return;
                }
			}break;
			case 2:
			{
				//printf("right \n");
                if (m_nCurPage >= 0 && m_nCurPage < m_nMaxPage) 
                {
                    ++m_nCurPage;
                    RefreshList();
                } 
                else
                {
                    //CancelSel();
                    return;
                }
			}break;
			case 3:
			{
				//printf("right most \n");
                if (m_nCurPage != m_nMaxPage) 
                {
                    m_nCurPage = m_nMaxPage;
                    RefreshList();
                }
                else 
                {
                    //CancelSel();
                    return;
                }                
			}break;
			case 4:
			{
				//printf("preview \n");                               
                //SwitchPreview(EM_BIZPREVIEW_1SPLIT, nCh);                
                nSelLine = atoi(this->pComboBox->GetString(nSelLine)) - 1;

                if (FALSE == bPrFlag)
                {
                    //printf("Start preview\n");
                    bPrFlag = TRUE;
                    pButton[4]->SetText("");
                    pButton[4]->SetText(GetParsedString("&CfgPtn.StopPreview"));
         	        SetCurPreviewMode_CW(EM_BIZPREVIEW_1SPLIT);//cw_preview
                    SwitchPreview(EM_BIZPREVIEW_1SPLIT, nCh);
                    //BizPtzCtrl((u8)nCh, EM_BIZPTZ_CMD_START_TOUR, nSelLine);
                    StartPreview();
                }
                else
                {
                    //printf("Stop preview\n");
                    StopPreview();
                }
			}break;
			case 5:
			{
				//printf("ok \n");                
				((CPageCruiseLine*)(this->pParent))->SetCruisePoint(nCh, psGuiTourPath);		
			}break;
			case 6:
			{
				//printf("exit \n");
                
                //如果之前点击了预览在退出时关闭
                StopPreview();
                //this->pParent->Open();
				this->Close(UDM_CLOSED);
			}break;
			case 7:
			{
				//printf("add \n");
                CancelSel();
				#if 1
                pAddCruisePoint->onSetCh(nCh);
				pAddCruisePoint->SetRect(&rtAdd);
				pAddCruisePoint->Open();
                #else
                snPos = 1;
                for (int i=0; i<TOURPIONT_MAX; ++i) 
                {
                    SGuiTourPoint sPIns;
                    sPIns.nPresetPos = snPos++;
                    sPIns.nSpeed = 1;
                    sPIns.nDwellTime = 1;
                    AddCruisePoint(&sPIns);
                }
                #endif
				
			}break;
			case 8:
			{
				//printf("del \n");
                if (0 == m_nCursel) 
                {
                    //add by Lirl on Nov/21/2011
    				MessageBox("&CfgPtn.ChooseDelectPreset", "" , MB_OK|MB_ICONWARNING);
                    //end
                    return;
                }
				//add by Lirl on Nov/19/2011
				UDM ret = MessageBox("&CfgPtn.DelectPreset", "&CfgPtn.WARNING" , MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2);
				if(UDM_OK != GetRunStatue())
				{
					return;
				}
                //end
                snPos = 1;
                pStatic[PRESET_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
                pStatic[PRESET_START+m_nCursel-1]->Draw();
                pStatic[RATE_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
                pStatic[RATE_START+m_nCursel-1]->Draw();
                pStatic[TIME_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
                pStatic[TIME_START+m_nCursel-1]->Draw();
				DelCruisePoint();
                if ((m_nCurPage != m_nMaxPage) 
                    || (0 == m_nTotal && 0 == m_nCurPage && 0 == m_nMaxPage))
                {
                    RefreshList();
                }
                m_nCursel = 0;
			}break;
			case 9:
			{
				//printf("modify \n");
				if (0 == m_nCursel)
				{
                    //add by Lirl on Nov/21/2011
    				MessageBox("&CfgPtn.ChooseModifyPreset", "" , MB_OK|MB_ICONWARNING);
    				//end
                    return;
                } 
                else if(m_nCursel>0)
                {
					pAddCruisePoint->SetCurCruisePoint(
						(char*)pStatic[PRESET_START+m_nCursel-1]->GetText(),
						(char*)pStatic[RATE_START+m_nCursel-1]->GetText(),
						(char*)pStatic[TIME_START+m_nCursel-1]->GetText()
					);

					#if 0
					CRect rt, rt1;
					
					pAddCruisePoint->GetRect(&rt);
					pButton[9]->GetRect(&rt1);

					rt.top		 = rt1.bottom;
					rt.bottom 	 = rt.top + 140;
					#endif
					pAddCruisePoint->onSetCh(nCh);
					pAddCruisePoint->SetRect(&rtMod);
					pAddCruisePoint->Open();

				}
			}break;
			case 10:
			{
				//printf("top \n");
                TopCruisePoint();
			}break;
			case 11:
			{
				//printf("up \n");
                UpCruisePoint();
			}break;
			case 12:
			{
				//printf("down \n");
                DownCruisePoint();
			}break;
			case 13:
			{
				//printf("bottom \n");
                BottomCruisePoint();
			}break;
			default:
			break;

		}
	}

	
	
}

CPageCruisePoint::~CPageCruisePoint()
{

}

VD_PCSTR CPageCruisePoint::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}


int CPageCruisePoint::GetCurPointNum()//cw_tab
{
	int num;
	if (m_nTotal<8)
		num= m_nTotal;
	else 
	{
		if((m_nCurPage+1)*8>m_nTotal)
			num =m_nTotal-(m_nCurPage*8);
		else
			num= 8;
	}
	return num;
}

VD_BOOL CPageCruisePoint::MsgProc( uint msg, uint wpa, uint lpa )
{
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
					CItem*tmp=GetFocusItem();
					if ((tmp!=NULL)&&(tmp!=pComboBox))
					{
						m_chSel=wpa -KEY_0;
						if((m_chSel<=GetCurPointNum())&&m_chSel>=0)
						{
							SelectItem(m_chSel);
						}	
						return TRUE;
					}
 				}break;
				case KEY_RIGHT:
				case KEY_LEFT:
				case KEY_UP:
				case KEY_DOWN:
				{
					CPageFrame::MsgProc(msg, wpa,lpa);
		    		static CItem* last = NULL;
					CItem* temp = GetFocusItem();   //cw_panel
                    if(temp != last)
    				{
	    				if(temp == pButton[7])
	    				{
	    					SetInfo("&CfgPtn.ADD");
	    				}
	    				else if(temp == pButton[8])
	    				{
	    					SetInfo("&CfgPtn.Delete");
	    				}
	    				else if(temp == pButton[9])
	    				{
	    					SetInfo("&CfgPtn.ModifyPreset");
	    				}
	    				else if(temp == pButton[10])
	    				{
	    					SetInfo("&CfgPtn.TopOfTable");
	    				}
                        else if (temp == pButton[11])
                        {
                       		SetInfo("&CfgPtn.UpToForward");
                    	}
                       else if (temp == pButton[12])
                    	{
                        	SetInfo("&CfgPtn.DownToBack");
                    	}
                       	else if (temp == pButton[13])
                       	{
                            	SetInfo("&CfgPtn.BottomOfTable");
                       	}
	    				else
	    				{
	    					ClearInfo();
	    				}
	    				last = temp;
    				}
					return TRUE;
				}break;
			case KEY_PTZ:
				return FALSE;
				default:
					break;
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
			int row = 0;
			for(i = 0; i<3; i++)
			{
				for ( row=1; row<9; row++)
				{
					VD_RECT rt;
					pTable0->GetTableRect(i, row, &rt);
					rt.left +=10+offsetx;
					rt.top +=85+offsety;
					rt.right +=10+offsetx;
					rt.bottom +=85+offsety;
					//printf("i=%d  ,x=%d, y=%d, %d %d %d %d \n",i, px, py, rt.left, rt.top, rt.right,rt.bottom);
					if (PtInRect(&rt, px, py))
					{
						//printf("m_nTotal %d\n", m_nTotal);
						if(m_nTotal>0)
						{
							bFind = TRUE;
							goto SELITEM;
						}
					}
				}
			}

SELITEM:
			if (bFind)
			{
				//printf("SelectDskItem %d\n", row);
                SelectItem(row);
			}
		}
		break;
        case XM_RBUTTONDOWN:
	    case XM_RBUTTONDBLCLK:
            {
                //this->pParent->Open();
                StopPreview();
                this->Close(UDM_CLOSED);
            }
            break;
         case XM_MOUSEMOVE:
    		{
    			px = VD_HIWORD(lpa);
    			py = VD_LOWORD(lpa);
    			static CItem* last = NULL;
    			CItem* temp = GetItemAt(px, py);
                VD_RECT rtScreen;
    			m_pDevGraphics->GetRect(&rtScreen);
    			int offsetx = (rtScreen.right - m_Rect.Width())/2;
    			int offsety = (rtScreen.bottom - m_Rect.Height())/2;
                

    			if(temp != last)
    			{
    				if(temp == pButton[7])
    				{
    					SetInfo("&CfgPtn.ADD");
    				}
    				else if(temp == pButton[8])
    				{
    					SetInfo("&CfgPtn.Delete");
    				}
    				else if(temp == pButton[9])
    				{
    					SetInfo("&CfgPtn.ModifyPreset");
    				}
    				else if(temp == pButton[10])
    				{
    					SetInfo("&CfgPtn.TopOfTable");
    				}
                    else if (temp == pButton[11])
                    {
                        SetInfo("&CfgPtn.UpToForward");
                    }
                    else if (temp == pButton[12])
                    {
                        SetInfo("&CfgPtn.DownToBack");
                    }
                    else if (temp == pButton[13])
                    {
                        SetInfo("&CfgPtn.BottomOfTable");
                    }
    				else
    				{
    					ClearInfo();
    				}

    				last = temp;

    			}

                BOOL bFind = FALSE;
    			int i = 0;
    			int row = 0;
    			for(i = 0; i<3; i++)
    			{
    				for ( row=1; row<9; row++)
    				{
    					VD_RECT rt;
    					pTable0->GetTableRect(i, row, &rt);
    					rt.left +=10+offsetx;
    					rt.top +=85+offsety;
    					rt.right +=10+offsetx;
    					rt.bottom +=85+offsety;
    					//printf("i=%d  ,x=%d, y=%d, %d %d %d %d \n",i, px, py, rt.left, rt.top, rt.right,rt.bottom);
    					if (PtInRect(&rt, px, py))
    					{
    						if(m_nTotal>0)
    						{
    							bFind = TRUE;
    							goto SELITEM1;
    						}
    					}

    				}
    			}
SELITEM1:
    			if (bFind)
    			{
    				//printf("SelectDskItem %d\n", row);
    				MouseMoveToLine(row);
    			}
    			else
    			{
                    MouseMoveToLine(0);
                }
         }
	default:
		break;
	}

	return 	CPageFrame::MsgProc(msg, wpa, lpa);
}

void CPageCruisePoint::MouseMoveToLine(int index)
{
	if(index == m_nCurMouseLine)
	{
		return;
	}
	if((m_nCurMouseLine>0) && (strcmp(pStatic[PRESET_START+m_nCurMouseLine-1]->GetText(),"") != 0))
	{
		pStatic[PRESET_START+m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
        pStatic[RATE_START+m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
        pStatic[TIME_START+m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
	}
	m_nCurMouseLine = index;
	
	if (index < 1 || index>8 )
	{
		return;
	}

	if(strcmp(pStatic[PRESET_START+index-1]->GetText(),"") == 0)
	{
		return ;
	}

	pStatic[PRESET_START+index-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
    pStatic[RATE_START+index-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
    pStatic[TIME_START+index-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
}

void CPageCruisePoint::SelectItem( int index )
{
	if (index < 1 || index>8 || index>m_nTotal )
	{
		return;
	}

	if (index == m_nCursel && !bFirstSel)
	{
		return;
	}

	bFirstSel = FALSE;
	
	if (m_nCursel > 0)
	{
        pStatic[PRESET_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
        pStatic[PRESET_START+m_nCursel-1]->Draw();
        pStatic[RATE_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
        pStatic[RATE_START+m_nCursel-1]->Draw();
        pStatic[TIME_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
        pStatic[TIME_START+m_nCursel-1]->Draw();

        //m_nCursel += 8;
	}
    
    if (0 == m_nCursel)
    {
        ++m_nCursel;
    }
	m_nCursel = index;
    pStatic[PRESET_START+index-1]->SetBkColor(VD_RGB(56,108,148));
    pStatic[PRESET_START+index-1]->Draw();
    pStatic[RATE_START+index-1]->SetBkColor(VD_RGB(56,108,148));
    pStatic[RATE_START+index-1]->Draw();
    pStatic[TIME_START+index-1]->SetBkColor(VD_RGB(56,108,148));
    pStatic[TIME_START+index-1]->Draw();

    nCurPathNoIdx = m_nCurPage*8+m_nCursel-1;
}

void CPageCruisePoint::SetCruseiPoint(int nCLIdx, SGuiTourPoint* pPoint)
{
	char szTmp[32] = {0};

	sprintf(szTmp, "%d", pPoint->nPresetPos);
	pStatic[PRESET_START+nCLIdx]->SetText(szTmp);
	sprintf(szTmp, "%d", pPoint->nSpeed);
	pStatic[RATE_START+nCLIdx]->SetText(szTmp);
	sprintf(szTmp, "%d", pPoint->nDwellTime);
	pStatic[TIME_START+nCLIdx]->SetText(szTmp);

	//m_nTotal++;
	
	return;
}

void CPageCruisePoint::RefreshList()
{
	int nCLIdx = -1;
	
	nTPNo = strtol(pComboBox->GetString(pComboBox->GetCurSel()), 0, 10);

    for(int i=0; i<PRESET_END-PRESET_START; i++)
	{
	    pStatic[PRESET_START+i]->SetText("");
	    pStatic[RATE_START+i]->SetText("");
	    pStatic[TIME_START+i]->SetText("");
	}

    CancelSel();

    //memset(psGuiTourPoint, 0, sizeof(SGuiTourPoint)*TOURPIONT_MAX);
	for(int i=0; i<TOURPATH_MAX; i++)
	{
		if(psGuiTourPath[i].nPathNo==nTPNo)
		{
			nCurPathNoIdx = i;
			int k = 0;
            
			for(int j=0; j<TOURPIONT_MAX; j++)
			{
				if( psGuiTourPath[i].sTourPoint[j].nPresetPos>0 )
				{
                    //可以得出总的项数，以便求得总的页数
                    ++nCLIdx;

                    //放到一个顺序存放数组里,方便以页的形式显示
                    memset(&psGuiTourPoint[nCLIdx], 0, sizeof(SGuiTourPoint));
                    memcpy(&psGuiTourPoint[nCLIdx], &psGuiTourPath[i].sTourPoint[j], sizeof(SGuiTourPoint));

                    
                    //最多只显示8项的数据
                    if (k>=8)
                    {
                        continue;
                    }
                    int nID = m_nCurPage*8+k;
                    if (psGuiTourPoint[nID].nPresetPos>0 && nID <= nCLIdx)
                    {
    					SetCruseiPoint(k, &psGuiTourPoint[nID]);
					    k++;
                    }
				}
			}
            //求出总的最大页数
            if (nCLIdx>=0)
            {
                m_nMaxPage = nCLIdx/8;
            }
            //nCLIdx == -1
            else
                m_nMaxPage = 0;
            
			break;
		}

        #if 0
        int nID = m_nCurPage*8;
        for(int i = 0; i<8 && psGuiTourPoint[nID+i].nPresetPos>0; ++i)
        {
            SetCruseiPoint(i, &psGuiTourPoint[nID+i]);
        }
        #endif
	}
    m_nTotal = 0;
    for (int i = 0; i<8; ++i)
    {
        if ((strtol(pStatic[PRESET_START+i]->GetText(), 0, 10)> 0) 
            && (NULL != pStatic[RATE_START+i]->GetText()) 
            && (NULL != pStatic[TIME_START+i]->GetText()))
        {
            ++m_nTotal;
        }
    }

	SetStatus();
}


VD_BOOL CPageCruisePoint::UpdateData( UDM mode )
{
    char szTmp[32] = {0};
    
    if (UDM_OPEN == mode)
	{	
        //第一页
        m_nCurPage = 0;
        bPrFlag = FALSE;
        
		for(int j=0; j<8; j++)
		{
			pStatic[PRESET_START+j]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pStatic[PRESET_START+j]->Draw();
			pStatic[RATE_START+j]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pStatic[RATE_START+j]->Draw();
			pStatic[TIME_START+j]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pStatic[TIME_START+j]->Draw();
		}
		
		nTPNo = nCurPathNo;//strtol(pComboBox->GetString(pComboBox->GetCurSel()), 0, 10);
	
		m_nCursel = 0;
		m_nTotal = 0;

		BIZ_DO_DBG("CPageCruisePoint::UpdateData 0\n");
		
		((CPageCruiseLine*)this->pParent)->GetCruisePoint(nCh, psGuiTourPath);

		BIZ_DO_DBG("CPageCruisePoint::UpdateData 1\n");

		pComboBox->RemoveAll();
		pEdit->SetText("");
		
		for(int i=0; i<PRESET_END-PRESET_START+1; i++)
		{
			pStatic[PRESET_START+i]->SetText("");
			pStatic[RATE_START+i]->SetText("");
			pStatic[TIME_START+i]->SetText("");
		}

        //把已经添加的巡航线添加到combox的列表中，同时显示当前巡航线置点属性
        //int nTmp = 0;
		for( int i=0; i<TOURPATH_MAX; i++)
		{
            //nTmp = ((CPageCruiseLine*)this->pParent)->bGetCrLineNum(i);
			if(((CPageCruiseLine*)this->pParent)->bGetCrLineNum(i))
			{
                sprintf(szTmp, "%d", i + 1);
				pComboBox->AddString(szTmp);
			}
         }

        //找到与之对应的正确通道号
        for (int i = 0; i < pComboBox->GetCount(); ++i)
        {
            if (nID == strtol(pComboBox->GetString(i), 0, 10)) 
            {
                pComboBox->SetCurSel(i);
                break;
            } 
            else 
            {
                pComboBox->SetCurSel(0);
            }
        }
        nSelLine = pComboBox->GetCurSel();
		RefreshList();
		pEdit->SetText(szName);


		CItem *pItemSpecial = NULL;
		pItemSpecial = FindPage(m_screen, " Ptz Control", FALSE);
		((CPagePtzCtl* )pItemSpecial)->SetCruiseStage(TRUE);
	}
	if (UDM_CLOSED== mode)
	{	
		CItem *pItemSpecial = NULL;
		pItemSpecial = FindPage(m_screen, " Ptz Control", FALSE);
		((CPagePtzCtl* )pItemSpecial)->SetCruiseStage(FALSE);
	}
	return TRUE;
}


void CPageCruisePoint::SetSelItem(int nCh, int nCurPathNo, int nId, char* szName)
{
    this->nCh = nCh;
	this->nCurPathNo = nCurPathNo;
	this->nID = nId;
	strcpy(this->szName, szName);	
}

void CPageCruisePoint::OnComboBox()
{
    //切换航线时都统一翻到第一页，防止出现没有的页面数据
    m_nCurPage = 0;
	RefreshList();
    //把上次点击的加亮恢复
    if (0 == m_nCursel)
    {
        ++m_nCursel;
    }
    
    pStatic[PRESET_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
    pStatic[PRESET_START+m_nCursel-1]->Draw();
    pStatic[RATE_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
    pStatic[RATE_START+m_nCursel-1]->Draw();
    pStatic[TIME_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
    pStatic[TIME_START+m_nCursel-1]->Draw();
    //更新个置点总数
    m_nTotal = 0;
    for(int i=0; i<PRESET_END-PRESET_START+1; i++)
    {
        if (strtol(pStatic[PRESET_START+i]->GetText(), 0, 10)>0)
        {
            ++m_nTotal;
        }
        else
        {
            break;
        }
    }
    //防止点击第一行时无效
    m_nCursel = 0;
	
	return;
}

void CPageCruisePoint::SetStatus()
{
	char szSta[6] = {0};

    if (0 == m_nTotal && 0 == m_nCurPage && 0 == m_nMaxPage)
    {
        sprintf(szSta, "%d/%d", m_nCurPage, m_nMaxPage);
    }
    else
    {
	    sprintf(szSta, "%d/%d", m_nCurPage+1, m_nMaxPage+1);
    }

	pStatic[29]->SetText(szSta);
}

int CPageCruisePoint::AddToTable(SGuiTourPoint* pPIns)
{
	int i, j;
	char szCl[32] = {0};

    int nSumAllPos = 0;
    //计算总添加的置点个数
    for (int j=0; j<TOURPIONT_MAX; ++j)
    {
        if (psGuiTourPath[nCurPathNoIdx].sTourPoint[j].nPresetPos>0) 
        {
            ++nSumAllPos;
        }
    }
    //判断是否大于128个置点数
    if (nSumAllPos>=TOURPIONT_MAX)
    {
        UDM ret= MessageBox("&CfgPtn.AddCruisePointOverRange", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
		if(GetRunStatue() == UDM_OK)
		{
            pEdit->Draw();
			return -1;
		}
    }
    
    m_nTotal = 0;
    for(int i=0; i<PRESET_END-PRESET_START; i++)
    {
        if ((strtol(pStatic[PRESET_START+i]->GetText(), 0, 10)>0)
            && (NULL != pStatic[RATE_START+i]->GetText()) 
            && (NULL != pStatic[TIME_START+i]->GetText()))
        {
            ++m_nTotal;
        }
        else
        {
            break;
        }
    }    
    #if 0
	if(m_nTotal>=7)
	{
        UDM ret= MessageBox("&CfgPtn.AddCruisePointOverRange", "&CfgPtn.WARNING", MB_OK);
        if(ret == UDM_OK)
            {
                pStatic[1]->SetText("&CfgPtn.Name");
                pEdit->SetText(szName);
                return;
            }
    }
    #endif
    //printf("***InFn:%s, total:[%d]\n", __FUNCTION__, m_nTotal);
    if(m_nTotal>=0 && m_nTotal<8)
    {
    	if(pPIns->nPresetPos>TOURPIONT_MAX || pPIns->nPresetPos<=0)
    	{
            return -1;
        }

    	//preset
    	sprintf(szCl, "%d", pPIns->nPresetPos);
    	pStatic[PRESET_START+m_nTotal]->SetText(szCl);

    	//rate
    	sprintf(szCl, "%d", pPIns->nSpeed);	
    	pStatic[RATE_START+m_nTotal]->SetText(szCl);

    	// time
    	sprintf(szCl, "%d", pPIns->nDwellTime);	
    	pStatic[TIME_START+m_nTotal]->SetText(szCl);

    	m_nTotal++;
    }
    else if (8 == m_nTotal)
    {
        //RefreshList();
        m_nTotal = 0;
        ++m_nMaxPage;
        m_nCurPage = m_nMaxPage;
                
		for(int j=0; j<8; j++)
		{
			pStatic[PRESET_START+j]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pStatic[PRESET_START+j]->Draw();
			pStatic[RATE_START+j]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pStatic[RATE_START+j]->Draw();
			pStatic[TIME_START+j]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
			pStatic[TIME_START+j]->Draw();

            pStatic[PRESET_START+j]->SetText("");
			pStatic[RATE_START+j]->SetText("");
			pStatic[TIME_START+j]->SetText("");
		}

        if(pPIns->nPresetPos>TOURPIONT_MAX || pPIns->nPresetPos<=0)
        {
            return -1;
        }

        //preset
    	sprintf(szCl, "%d", pPIns->nPresetPos);
    	pStatic[PRESET_START+m_nTotal]->SetText(szCl);

    	//rate
    	sprintf(szCl, "%d", pPIns->nSpeed);	
    	pStatic[RATE_START+m_nTotal]->SetText(szCl);

    	// time
    	sprintf(szCl, "%d", pPIns->nDwellTime);	
    	pStatic[TIME_START+m_nTotal]->SetText(szCl);

    	m_nTotal++;
    } else {
        return -1;
    }
	
	SetStatus();
}


void CPageCruisePoint::AddCruisePoint( SGuiTourPoint* pPIns)
{
	int i, j;
	char szCl[32] = {0};	

    //获得当前航线号
	nCurPathNo = strtol(pComboBox->GetString(pComboBox->GetCurSel()), 0, 10);

    //获得对应数组中的通道号
	for(i=0; i<TOURPATH_MAX; i++)
	{
		if(psGuiTourPath[i].nPathNo==nCurPathNo)
		{
            nCurPathNoIdx = i;
			break;
		}
	}
    
    if (m_nCurPage != m_nMaxPage)
    {
        m_nCurPage = m_nMaxPage;
        RefreshList();
    }
    //m_nCurPage = m_nMaxPage;
    //RefreshList();

    int rtnValue = 0;
    rtnValue = AddToTable(pPIns);
    if (-1 == rtnValue)
    {
        return;
    }    
    
	if(i!=TOURPATH_MAX)
	{
        j = m_nMaxPage*8+m_nTotal-1;
        memcpy(&psGuiTourPath[i].sTourPoint[j], pPIns, sizeof(SGuiTourPoint));
	}
    SelectItem(m_nTotal);
}

void CPageCruisePoint::ChangeCruisePoint()
{
	if(m_nCursel>0)
	{
		
	}
}

// cruise point position in the tablebox == index in the struct array 
//
void CPageCruisePoint::DelCruisePoint()
{
	int  i;
	
	uint nPreset = strtol(pStatic[PRESET_START+m_nCursel-1]->GetText(), 0, 10);
	uint nRate = strtol(pStatic[RATE_START+m_nCursel-1]->GetText(), 0, 10);
	uint nTime = strtol(pStatic[TIME_START+m_nCursel-1]->GetText(), 0, 10);
	
	if(m_nCursel<=0||m_nTotal<=0)
	{
        return;
    }
	
	// last item
	// delete
	if(m_nCursel==m_nTotal)
	{
		pStatic[PRESET_START+m_nTotal-1]->SetText("");		
		pStatic[RATE_START+m_nTotal-1]->SetText("");	
		pStatic[TIME_START+m_nTotal-1]->SetText("");		
	}
	// else
	// move all-down to up
	else
	{
		for(i=0; i<m_nTotal-m_nCursel; i++)
		{
			pStatic[PRESET_START+m_nCursel+i-1]->SetText(
				pStatic[PRESET_START+m_nCursel+i]->GetText()
			);
			
			pStatic[RATE_START+m_nCursel+i-1]->SetText(
				pStatic[RATE_START+m_nCursel+i]->GetText()
			);
			pStatic[TIME_START+m_nCursel+i-1]->SetText(
				pStatic[TIME_START+m_nCursel+i]->GetText()
			);
		}
		
		pStatic[PRESET_START+m_nCursel+i-1]->SetText("");
		pStatic[RATE_START+m_nCursel+i-1]->SetText("");
		pStatic[TIME_START+m_nCursel+i-1]->SetText("");
	}
	
	// focus item
	// clear
	if(m_nTotal==m_nCursel)
	{
		pStatic[PRESET_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pStatic[PRESET_START+m_nCursel-1]->Draw();
		pStatic[RATE_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pStatic[RATE_START+m_nCursel-1]->Draw();
		pStatic[TIME_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pStatic[TIME_START+m_nCursel-1]->Draw();
	}
	
	int j = m_nCurPage*8+m_nCursel-1;
	for(int i=0; i<TOURPATH_MAX; i++)
	{
		if(psGuiTourPath[i].nPathNo==nTPNo)
		{
			for(; j<(m_nMaxPage+1)*8; j++)
			{
                if(j==TOURPATH_MAX-1) 
                { //127
                    psGuiTourPath[i].sTourPoint[j].nPresetPos = 0;
                    psGuiTourPath[i].sTourPoint[j].nSpeed = 0;
                    psGuiTourPath[i].sTourPoint[j].nDwellTime = 0;
					break;
                } 
                else if (j >= TOURPATH_MAX)
                {
                    break;
                }
                memcpy(&psGuiTourPath[i].sTourPoint[j], &psGuiTourPath[i].sTourPoint[j+1], sizeof(SGuiTourPoint));
			}
            //空出来的那个元素用0来填充
            if (j != TOURPATH_MAX) 
            {
                psGuiTourPath[i].sTourPoint[j].nPresetPos = 0;
                psGuiTourPath[i].sTourPoint[j].nSpeed = 0;
                psGuiTourPath[i].sTourPoint[j].nDwellTime = 0;
            }
            break;
		}
	}

	if(m_nTotal>0)
	{
		m_nTotal--;
    }

	if(m_nTotal<=0) 
	{
        m_nCursel = 0;
        if (0 == m_nTotal && m_nCurPage>0)
        {
            m_nCurPage--;
        }
    }
}

void CPageCruisePoint::CancelSel()
{
    for (int i=0; i < 8; ++i)
	{
		pStatic[PRESET_START+i]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
        pStatic[RATE_START+i]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
        pStatic[TIME_START+i]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pStatic[PRESET_START+i]->Draw();
		pStatic[RATE_START+i]->Draw();
        pStatic[TIME_START+i]->Draw();
    }
    m_nCursel = 0;
}

void CPageCruisePoint::TopCruisePoint()
{
    int  i;
	
	if(m_nCursel<=0||m_nTotal<=0)
        return;	

	if(m_nCursel==1)
	{
		return;		
	}
	else
	{
        char pPresetTmp[6] = {0};
        char pRateTmp[6] = {0};
        char pTimeTmp[6] = {0};

        memcpy(pPresetTmp,pStatic[PRESET_START+m_nCursel-1]->GetText(), sizeof(pStatic[PRESET_START+m_nCursel-1]->GetText()));
        memcpy(pRateTmp,pStatic[RATE_START+m_nCursel-1]->GetText(), sizeof(pStatic[RATE_START+m_nCursel-1]->GetText()));
        memcpy(pTimeTmp,pStatic[TIME_START+m_nCursel-1]->GetText(), sizeof(pStatic[TIME_START+m_nCursel-1]->GetText()));

        int j = m_nCurPage*8 + m_nCursel-1;
        SGuiTourPoint sTmpRec;
        memcpy(&sTmpRec, &psGuiTourPath[nTPNo-1].sTourPoint[m_nCurPage*8], sizeof(SGuiTourPoint));
        memcpy(&psGuiTourPath[nTPNo-1].sTourPoint[m_nCurPage*8], &psGuiTourPath[nTPNo-1].sTourPoint[j], sizeof(SGuiTourPoint));
        for (i=m_nCursel; i>1 && j>=m_nCurPage*8; --i, --j)
        {
            pStatic[PRESET_START+i-1]->SetText(pStatic[PRESET_START+i-1-1]->GetText());
            pStatic[RATE_START+i-1]->SetText(pStatic[RATE_START+i-1-1]->GetText());
            pStatic[TIME_START+i-1]->SetText(pStatic[TIME_START+i-1-1]->GetText());

            memcpy(&psGuiTourPath[nTPNo-1].sTourPoint[j], &psGuiTourPath[nTPNo-1].sTourPoint[j-1], sizeof(SGuiTourPoint));
        }

        pStatic[PRESET_START+i-1]->SetText(pPresetTmp);
        pStatic[RATE_START+i-1]->SetText(pRateTmp);
        pStatic[TIME_START+i-1]->SetText(pTimeTmp);

        memcpy(&psGuiTourPath[nTPNo-1].sTourPoint[m_nCurPage*8+1], &sTmpRec, sizeof(SGuiTourPoint));

        CancelSel();

        m_nCursel = 1;

        pStatic[PRESET_START+m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
        pStatic[PRESET_START+m_nCursel-1]->Draw();
        pStatic[RATE_START+m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
        pStatic[RATE_START+m_nCursel-1]->Draw();
        pStatic[TIME_START+m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
        pStatic[TIME_START+m_nCursel-1]->Draw();
    }
    
    return;
}

void CPageCruisePoint::BottomCruisePoint()
{
    int  i;
	
	if(m_nCursel<=0||m_nTotal<=0)
        return;	

	if(m_nCursel==m_nTotal)
	{
		return;		
	}
	else
	{
        char pPresetTmp[6] = {0};
        char pRateTmp[6] = {0};
        char pTimeTmp[6] = {0};
        
        memcpy(pPresetTmp,pStatic[PRESET_START+m_nCursel-1]->GetText(), sizeof(pStatic[PRESET_START+m_nCursel-1]->GetText()));
        memcpy(pRateTmp,pStatic[RATE_START+m_nCursel-1]->GetText(), sizeof(pStatic[RATE_START+m_nCursel-1]->GetText()));
        memcpy(pTimeTmp,pStatic[TIME_START+m_nCursel-1]->GetText(), sizeof(pStatic[TIME_START+m_nCursel-1]->GetText()));

        int j = m_nCurPage*8 + m_nCursel-1;
        SGuiTourPoint sTmpRec;
        
        memcpy(&sTmpRec, &psGuiTourPath[nTPNo-1].sTourPoint[m_nCurPage*8+m_nTotal-1], sizeof(SGuiTourPoint));
        memcpy(&psGuiTourPath[nTPNo-1].sTourPoint[m_nCurPage*8+m_nTotal-1], &psGuiTourPath[nTPNo-1].sTourPoint[j], sizeof(SGuiTourPoint));

        for (i=m_nCursel; i<m_nTotal && j<m_nCurPage*8+m_nTotal-1; ++i, ++j)
        {
            pStatic[PRESET_START+i-1]->SetText(pStatic[PRESET_START+i-1+1]->GetText());
            pStatic[RATE_START+i-1]->SetText(pStatic[RATE_START+i-1+1]->GetText());
            pStatic[TIME_START+i-1]->SetText(pStatic[TIME_START+i-1+1]->GetText());

            memcpy(&psGuiTourPath[nTPNo-1].sTourPoint[j], &psGuiTourPath[nTPNo-1].sTourPoint[j+1], sizeof(SGuiTourPoint));
        }

        pStatic[PRESET_START+i-1]->SetText(pPresetTmp);
        pStatic[RATE_START+i-1]->SetText(pRateTmp);
        pStatic[TIME_START+m_nCursel-1]->SetText(pTimeTmp);

        memcpy(&psGuiTourPath[nTPNo-1].sTourPoint[j-1], &sTmpRec, sizeof(SGuiTourPoint));

        CancelSel();

        m_nCursel = m_nTotal;

        pStatic[PRESET_START+m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
        pStatic[PRESET_START+m_nCursel-1]->Draw();
        pStatic[RATE_START+m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
        pStatic[RATE_START+m_nCursel-1]->Draw();
        pStatic[TIME_START+m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
        pStatic[TIME_START+m_nCursel-1]->Draw();
    }

    return;
}

void CPageCruisePoint::UpCruisePoint()
{
    int  i;
	
	if(m_nCursel<=0||m_nTotal<=0)
	{
        return;	
    }

	if(m_nCursel==1)
	{
		return;		
	}
	else
	{
        char pPresetTmp[6] = {0};
        char pRateTmp[6] = {0};
        char pTimeTmp[6] = {0};
        int cursel = 0;
        
        memcpy(pPresetTmp,pStatic[PRESET_START+m_nCursel-1]->GetText(), sizeof(pStatic[PRESET_START+m_nCursel-1]->GetText()));
        memcpy(pRateTmp,pStatic[RATE_START+m_nCursel-1]->GetText(), sizeof(pStatic[RATE_START+m_nCursel-1]->GetText()));
        memcpy(pTimeTmp,pStatic[TIME_START+m_nCursel-1]->GetText(), sizeof(pStatic[TIME_START+m_nCursel-1]->GetText()));

        pStatic[PRESET_START+m_nCursel-1]->SetText(pStatic[PRESET_START+m_nCursel-1-1]->GetText());
        pStatic[RATE_START+m_nCursel-1]->SetText(pStatic[RATE_START+m_nCursel-1-1]->GetText());
        pStatic[TIME_START+m_nCursel-1]->SetText(pStatic[TIME_START+m_nCursel-1-1]->GetText());

        pStatic[PRESET_START+m_nCursel-1-1]->SetText(pPresetTmp);
        pStatic[RATE_START+m_nCursel-1-1]->SetText(pRateTmp);
        pStatic[TIME_START+m_nCursel-1-1]->SetText(pTimeTmp);

        if (1 == m_nCursel)
        {
            return;
        }
        
        int j = m_nCurPage*8 + m_nCursel-1;
        SGuiTourPoint sTmpRec;

        memcpy(&sTmpRec, &psGuiTourPath[nTPNo-1].sTourPoint[j], sizeof(SGuiTourPoint));

        memcpy(&psGuiTourPath[nTPNo-1].sTourPoint[j], &psGuiTourPath[nTPNo-1].sTourPoint[j-1], sizeof(SGuiTourPoint));
        memcpy(&psGuiTourPath[nTPNo-1].sTourPoint[j-1], &sTmpRec, sizeof(SGuiTourPoint));
        
        cursel = m_nCursel;
        CancelSel();
        m_nCursel = cursel;
        m_nCursel--;

        pStatic[PRESET_START+m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
        pStatic[PRESET_START+m_nCursel-1]->Draw();
        pStatic[RATE_START+m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
        pStatic[RATE_START+m_nCursel-1]->Draw();
        pStatic[TIME_START+m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
        pStatic[TIME_START+m_nCursel-1]->Draw();
    }

    return;
}

void CPageCruisePoint::DownCruisePoint()
{
    int  i;
    int cursel = 0;
	
	if(m_nCursel<=0||m_nTotal<=0)
	{
        return;	
    }

	if(m_nCursel==m_nTotal)
	{
		return;		
	}
	else
	{
        char pPresetTmp[6] = {0};
        char pRateTmp[6] = {0};
        char pTimeTmp[6] = {0};
        
        memcpy(pPresetTmp,pStatic[PRESET_START+m_nCursel-1]->GetText(), sizeof(pStatic[PRESET_START+m_nCursel-1]->GetText()));
        memcpy(pRateTmp,pStatic[RATE_START+m_nCursel-1]->GetText(), sizeof(pStatic[RATE_START+m_nCursel-1]->GetText()));
        memcpy(pTimeTmp,pStatic[TIME_START+m_nCursel-1]->GetText(), sizeof(pStatic[TIME_START+m_nCursel-1]->GetText()));

        pStatic[PRESET_START+m_nCursel-1]->SetText(pStatic[PRESET_START+m_nCursel-1+1]->GetText());
        pStatic[RATE_START+m_nCursel-1]->SetText(pStatic[RATE_START+m_nCursel-1+1]->GetText());
        pStatic[TIME_START+m_nCursel-1]->SetText(pStatic[TIME_START+m_nCursel-1+1]->GetText());

        pStatic[PRESET_START+m_nCursel-1+1]->SetText(pPresetTmp);
        pStatic[RATE_START+m_nCursel-1+1]->SetText(pRateTmp);
        pStatic[TIME_START+m_nCursel-1+1]->SetText(pTimeTmp);
        
        if (m_nTotal == m_nCursel) 
        {
            return;
        }

        int j = m_nCurPage*8 + m_nCursel-1;
        SGuiTourPoint sTmpRec;

        memcpy(&sTmpRec, &psGuiTourPath[nTPNo-1].sTourPoint[j], sizeof(SGuiTourPoint));

        memcpy(&psGuiTourPath[nTPNo-1].sTourPoint[j], &psGuiTourPath[nTPNo-1].sTourPoint[j+1], sizeof(SGuiTourPoint));
        memcpy(&psGuiTourPath[nTPNo-1].sTourPoint[j+1], &sTmpRec, sizeof(SGuiTourPoint));
        
        cursel = m_nCursel;
        CancelSel();
        m_nCursel = cursel;
        m_nCursel++;

        pStatic[PRESET_START+m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
        pStatic[PRESET_START+m_nCursel-1]->Draw();
        pStatic[RATE_START+m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
        pStatic[RATE_START+m_nCursel-1]->Draw();
        pStatic[TIME_START+m_nCursel-1]->SetBkColor(VD_RGB(56,108,148));
        pStatic[TIME_START+m_nCursel-1]->Draw();
    }

    return;
}

void CPageCruisePoint::ModCruisePoint(SGuiTourPoint* psPIns)
{
	char szTmp[10] = {0};
	if(psPIns)
	{
		sprintf(szTmp, "%d", psPIns->nPresetPos);
		pStatic[PRESET_START+m_nCursel-1]->SetText(szTmp);
		sprintf(szTmp, "%d", psPIns->nSpeed);
		pStatic[RATE_START+m_nCursel-1]->SetText(szTmp);
		sprintf(szTmp, "%d", psPIns->nDwellTime);
		pStatic[TIME_START+m_nCursel-1]->SetText(szTmp);

        //printf("****Modify:[%d]\n", nCurPathNoIdx);

		for(int i=0; i<TOURPATH_MAX; i++)
		{
			if(psGuiTourPath[i].nPathNo==nTPNo)
			{
                int k = 0;
                //计算修改的是数组里哪一项
                for (int j = 0; j<TOURPIONT_MAX; j++)
                {
                    if (psGuiTourPath[i].sTourPoint[j].nPresetPos>0)
                    {
                        if (k == nCurPathNoIdx)
                        {
                            memcpy(&psGuiTourPath[i].sTourPoint[nCurPathNoIdx], psPIns, sizeof(SGuiTourPoint));
                            break;
                        }
                        k++;
                    }
                }
				//memcpy(&psGuiTourPath[i].sTourPoint[nCurPathNoIdx], psPIns, sizeof(SGuiTourPoint));				
				break;
			}
		}
	}
}

void CPageCruisePoint::SetInfo(char * szInfo)
{
    //pIconInfo->Show(TRUE);
		
	pInfoBar->SetText(szInfo);
}

void CPageCruisePoint::ClearInfo()
{
    //pIconInfo->Show(FALSE);
	
	pInfoBar->SetText("");
}

void CPageCruisePoint::OnSetPara(uchar nCh, SGuiTourPoint * psPreset)
{
    if (nCh<TOURPIONT_MAX && psPreset)
    {
        memcpy(psGuiTourPath[nCh].sTourPoint, psPreset, sizeof(SGuiTourPoint)*TOURPIONT_MAX);
    }
    return;
}

void CPageCruisePoint::OnGetPara(uchar nCh, SGuiTourPoint * psPreset)
{
    if (nCh<TOURPIONT_MAX && psPreset)
    {
        memcpy(psPreset, psGuiTourPath[nCh].sTourPoint, sizeof(SGuiTourPoint)*TOURPIONT_MAX);
    }
    return;
}

void CPageCruisePoint::LoadPage()
{
    
}

void CPageCruisePoint::Redraw()
{
    pStatic[TIME_START+5]->Draw();
}

VD_BOOL CPageCruisePoint::Close(UDM mode)
{
    StopPreview();
    return CPage::Close();
}

void CPageCruisePoint::StartPreview()
{
    nCurPathNo = strtol(pComboBox->GetString(pComboBox->GetCurSel()), 0, 10);

    int i = 0;
    for(i=0; i<TOURPATH_MAX; i++)
	{
		if(psGuiTourPath[i].nPathNo==nCurPathNo)
			break;
	}

    if (i != TOURPATH_MAX) 
    {
        int j;
        for (int j=0; j<TOURPIONT_MAX; ++j)
        {
            if (!bPreviewStatus 
                && (0<psGuiTourPath[i].sTourPoint[j].nPresetPos && psGuiTourPath[i].sTourPoint[j].nPresetPos<TOURPIONT_MAX) 
                && psGuiTourPath[i].sTourPoint[j].nSpeed>0 
                && psGuiTourPath[i].sTourPoint[j].nDwellTime>0
                )
            {                
                //set the speed
                BizPtzCtrl((u8)nCh, EM_BIZPTZ_CMD_SETSPEED, psGuiTourPath[i].sTourPoint[j].nSpeed);
                //goto
                BizPtzCtrl((u8)nCh, EM_BIZPTZ_CMD_PRESET_GOTO, psGuiTourPath[i].sTourPoint[j].nPresetPos);
                //delay
                sleep(psGuiTourPath[i].sTourPoint[j].nDwellTime);
            }
        }

        bPreviewStatus = TRUE;
        StopPreview();
    }
}

void CPageCruisePoint::StopPreview()
{
    if (bPrFlag && bPreviewStatus) 
    {
        printf("Stop preview\n");
        bPrFlag = FALSE;
	    SetCurPreviewMode_CW(eMaxViewMode);//cw_preview
        SwitchPreview(eMaxViewMode, 0);
        bPreviewStatus = FALSE;
        //BizPtzCtrl((u8)nCh, EM_BIZPTZ_CMD_STOP_TOUR, nSelLine);
    }

    pButton[4]->SetText("");
    pButton[4]->SetText(GetParsedString("&CfgPtn.Preview"));
}


