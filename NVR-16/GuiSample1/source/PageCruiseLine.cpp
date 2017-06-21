#include "GUI/Pages/PageCruiseLine.h"
#include "GUI/Pages/PageCruisePoint.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PagePtzConfigFrameWork.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PagePtzCtl.h"

char* cruiseLineShortcutBmpName[4][2] = {    
	{DATA_DIR"/temp/listex_btn_leftmost.bmp",	DATA_DIR"/temp/listex_btn_leftmost_f.bmp"},
	{DATA_DIR"/temp/listex_btn_left.bmp",	DATA_DIR"/temp/listex_btn_left_f.bmp"},
	{DATA_DIR"/temp/listex_btn_right.bmp",	DATA_DIR"/temp/listex_right_f.bmp"},
	{DATA_DIR"/temp/listex_btn_rightmost.bmp",	DATA_DIR"/temp/listex_btn_rightmost_f.bmp"},
};

static VD_BITMAP* pBmpButtonNormal[4];
static VD_BITMAP* pBmpButtonSelect[4];


static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

#define ID_START    2
#define ID_END      11
#define NAME_START 11
#define NAME_END   20

void bizData_GetPtzAdvancedPara(uchar nChn, SGuiPtzAdvancedPara* pGDispatch);
int bizData_SavePtzAdvancedPara(uchar nChn, SGuiPtzAdvancedPara* pGDispatch);

CPageCruiseLine::CPageCruiseLine( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/, int nChn ):CPageFrame(pRect, psz, icon, pParent)
{
	m_nTotal = 0;
	bFirstSel = TRUE;

	this->pParent = pParent;
	
	nChMax = GetVideoMainNum();
    m_nCurMouseLine = 0;

	//printf("CPageCruiseLine 0 \n");
	
	for(int i=0; i<nChMax; i++)
	{
		#if 0
		psGuiTourPath[i] = (SGuiTourPath*)malloc(sizeof(SGuiTourPath)*4);
		if(psGuiTourPath==NULL)
		{
			printf("no enough mem , req mem size %d\n", sizeof(SGuiTourPath)*4);
			exit(1);
		}
		#else
		psGuiTourPath[i] = (SGuiTourPath*)malloc(sizeof(SGuiTourPath)*TOURPATH_MAX);
		if(psGuiTourPath[i]==NULL)
		{
			exit(1);
		}
		#endif
	}

    bitTourAlive.set(); //全置为1
    bitRecTourStartStatus.set();

	//printf("CPageCruiseLine 1 \n");

	m_nChn = nChn;
	//SetRect(CRect(0, 0, 700, 400/*370*/), FALSE);
	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);

	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	char* szBtn[10] = 
	{
		"",
		"",
		"",
		"",
		"&CfgPtn.ADD",
		"&CfgPtn.Setup",
		"&CfgPtn.Delete",
		"&CfgPtn.Clear",
		"&CfgPtn.OK",
		"&CfgPtn.Exit",
	};

	char* szStatic[5] =
	{
		"&CfgPtn.ID",
		"&CfgPtn.Name",
		"1",
		"&CfgPtn.Cruise",
		"1/1",
	};

	CRect rtTemp;
	rtTemp.left = 10;
	rtTemp.top = 10;
	rtTemp.right = m_Rect.Width() - 10;
	rtTemp.bottom = rtTemp.top + 28*10;
	CRect tableRt(rtTemp.left,rtTemp.top,rtTemp.right,rtTemp.bottom);
	pTable0 = CreateTableBox(rtTemp, this, 2, 10);
	pTable0->SetColWidth(0, 60);

	
	rtTemp.left = 10;
	rtTemp.top = rtTemp.bottom-1;
	rtTemp.right = m_Rect.Width() - 10;
	rtTemp.bottom = rtTemp.top + 30;
	pTable1 = CreateTableBox(rtTemp, this, 1, 1);


	//printf("CPageCruiseLine 2 \n");
	
	int i = 0;
	CRect rt;
	for(i=0; i<2; i++)
	{
		pTable0->GetTableRect(i, 0, &rt);
		pStatic[i] = CreateStatic(CRect(tableRt.left+rt.left+2, tableRt.top+rt.top, tableRt.left+rt.right,tableRt.top+rt.bottom),
									this, szStatic[i]);
        pStatic[i]->SetBkColor(VD_RGB(67,77,87));
	}

	//id static
	for(i=ID_START; i<ID_END; i++)
	{
		pTable0->GetTableRect(0, i-1, &rt);
		pStatic[i] = CreateStatic(CRect(tableRt.left+rt.left, tableRt.top+rt.top, tableRt.left+rt.right,tableRt.top+rt.bottom),
									this, "");
	}


	//name static
	for(i=NAME_START; i<NAME_END; i++)
	{
		pTable0->GetTableRect(1, i-10, &rt);
		pStatic[i] = CreateStatic(CRect(tableRt.left+rt.left, tableRt.top+rt.top, tableRt.left+rt.right,tableRt.top+rt.bottom),
									this, "");
	}

	//id 1
	//pStatic[ID_START]->SetText("1");

	//name 1
	//pStatic[NAME_START]->SetText("Cruise Line");
	
	pTable1->GetTableRect(0, 0, &rt);
	pStatic[20] = CreateStatic(CRect(rtTemp.left+rt.left+2, rtTemp.top+rt.top, rtTemp.left+50,rtTemp.top+rt.bottom),
									this, szStatic[4]);

	//printf("CPageCruiseLine 3 \n");
	
	for(i=0; i<4; i++)
	{
		pTable1->GetTableRect(0, 0, &rt);

		pBmpButtonNormal[i] = VD_LoadBitmap(cruiseLineShortcutBmpName[i][0]);
		pBmpButtonSelect[i] = VD_LoadBitmap(cruiseLineShortcutBmpName[i][1]);
		pButton[i] = CreateButton(CRect(rtTemp.left+rt.Width()-45*(4-i), rtTemp.top+rt.top+1, rtTemp.left+rt.Width()-45*(4-i)+pBmpButtonNormal[i]->width,rtTemp.top+rt.top+pBmpButtonNormal[i]->height),
									this, szBtn[i], (CTRLPROC)&CPageCruiseLine::OnClickBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
	}

	

	rtTemp.left = 10;
	rtTemp.top = rtTemp.bottom + 18;
	rtTemp.right = rtTemp.left + 80;
	rtTemp.bottom = rtTemp.top + 25 - 3;
	for(i=4; i<8; i++)
	{
		pButton[i] = CreateButton(rtTemp, this, szBtn[i], (CTRLPROC)&CPageCruiseLine::OnClickBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);

		rtTemp.left = rtTemp.right + 10;
		rtTemp.right = rtTemp.left + 80;
	}
	
	//printf("before CPageCruisePoint\n");

	rtTemp.left = rtTemp.right +50;
	//rtTemp.top = rtTemp.bottom + 15;
	rtTemp.right = rtTemp.left + 60;
	//rtTemp.bottom = rtTemp.top + 25;
	for(i=8; i<10; i++)
	{
		pButton[i] = CreateButton(rtTemp, this, szBtn[i], (CTRLPROC)&CPageCruiseLine::OnClickBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);

		rtTemp.left = rtTemp.right + 10;
		rtTemp.right = rtTemp.left + 60;
	}
	
	//printf("before CPageCruisePoint\n");

	m_pPageCruisePoint = new CPageCruisePoint(NULL, "&CfgPtn.Cruise", icon_dvr, this/*, i*/);

	//printf("after CPageCruisePoint\n");
}

void CPageCruiseLine::OnClickBtn()
{
	//printf("OnClickSubPage\n");

	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < 10; i++)
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
			case 0:
			{
				//printf("CH %d :left most \n",m_nChn+1);
                if (0!=m_nCurPage)
                {
                    m_nCurPage = 0;
                    LoadPage();
                } 
                else 
                {
                    //CancelSel();
                    return;
                }                
			}break;
			case 1:
			{
				//printf("CH %d :left \n",m_nChn+1);
                if (m_nCurPage>0 && m_nCurPage<=m_nMaxPage && 0!=m_nCurPage)
                {
                    --m_nCurPage;
                    LoadPage();
                } 
                else 
                {
                    //CancelSel();
                    return;
                }
			}break;
			case 2:
			{
				//printf("CH %d :right \n",m_nChn+1);
                if (m_nCurPage>=0 && m_nCurPage<m_nMaxPage && m_nCurPage!=m_nMaxPage)
                {
                    ++m_nCurPage;
                    LoadPage();
                } 
                else
                {
                    //CancelSel();
                    return;
                }
			}break;
			case 3:
			{
				//printf("CH %d :right most \n",m_nChn+1);
                if (m_nCurPage != m_nMaxPage) 
                {
                    m_nCurPage = m_nMaxPage;
                    LoadPage();
                } 
                else 
                {
                    //CancelSel();
                    return;
                }
			}break;
			case 4:
			{
				//printf("CH %d :add \n",m_nChn+1);
				
				//add by Lirl on Nov/30/2011,检测对应通道是否有巡航线在执行
				if (-1 == BizPtzCheckTouring(m_nChn))
				{
                    char s[32] = {0};
                    sprintf(s, "%s", GetParsedString("&CfgPtn.TouringStopFirst"));
                    MessageBox(s, "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
                    return;
                }
				//end
				
				SGuiTourPath sPIns;
                if (m_nCurPage != m_nMaxPage)
                {
                    m_nCurPage = m_nMaxPage;
                    LoadPage();
                }

				memset(&sPIns, 0, sizeof(sPIns));				
				
				uchar nId = (uchar)-1;
				
				OnGetPathId(m_nChn, &nId);
                if (nId > TOURPATH_MAX) //TOURPATH_MAX
                {
					UDM ret= MessageBox("&CfgPtn.AddCruiseLineOverRange", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
					if(GetRunStatue() == UDM_OK)
					{
                        pStatic[1]->SetText("&CfgPtn.Name");
						break;
					}
				}
				sPIns.nPathNo = nId;
				
				if(nId <= TOURPATH_MAX)
				{
					AddPath(&sPIns);
				}

                if (bitTourAlive.any()) 
                {
                    pButton[7]->Enable(TRUE);
                }
                else 
                {
                    pButton[7]->Enable(FALSE);
                }
                
                return;
			}break;
			case 5:
			{
				//printf("CH %d, LN %d:setup \n",m_nChn+1, m_nCursel);

                //add by Lirl on Nov/30/2011,检测对应通道是否有巡航线在执行
				if (-1 == BizPtzCheckTouring(m_nChn)) 
				{
                    char s[32] = {0};
                    sprintf(s, "%s", GetParsedString("&CfgPtn.TouringStopFirst"));
                    MessageBox(s, "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
                    return;
                }
				//end
				
				if(m_nCursel>0)
				{
					m_pPageCruisePoint->SetSelItem(
            					m_nChn, 
            					strtol(pStatic[ID_START+m_nCursel-1]->GetText(), 0, 10),
            					strtol(pStatic[ID_START+m_nCursel-1]->GetText(), 0, 10), 
            					(char*)pStatic[NAME_START+m_nCursel-1]->GetText());
						
					//m_pPageCruisePoint->OnSetPara(m_nCursel-1, psGuiTourPath[m_nChn][m_nCursel-1].sTourPoint);
					m_pPageCruisePoint->Open();
                    //this->Close();
				}
                return;
			}break;
			case 6:
			{
				//printf("CH %d :delete \n",m_nChn+1);
				//add by Lirl on Nov/30/2011,检测对应通道是否有巡航线在执行
				if (-1 == BizPtzCheckTouring(m_nChn)) 
				{
                    char s[32] = {0};
                    sprintf(s, "%s", GetParsedString("&CfgPtn.TouringStopFirst"));
                    MessageBox(s, "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
                    return;
                }
				//end
				
				//add by Lirl on Nov/19/2011
				UDM ret = MessageBox("&CfgPtn.DelectCruise", "&CfgPtn.WARNING", MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2);
				if(UDM_OK != GetRunStatue())
				{
					return;
				}
                //end
				if(m_nCursel>0) 
				{
					DelPath(strtol(pStatic[ID_START+m_nCursel-1]->GetText(), 0, 10) - 1);
				}

                if (0 == m_nTotal && m_nCurPage>0) 
                {
                    m_nCurPage--;
                }
                
                if (m_nCurPage != m_nMaxPage) 
                {
				    LoadPage();
                }
			}break;
			case 7:
			{
				//printf("CH %d :clear \n",m_nChn+1);	

                //add by Lirl on Nov/30/2011,检测对应通道是否有巡航线在执行
				if (-1 == BizPtzCheckTouring(m_nChn))
				{
                    char s[32] = {0};
                    sprintf(s, "%s", GetParsedString("&CfgPtn.TouringStopFirst"));
                    MessageBox(s, "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
                    return;
                }
				//end

                //add by Lirl on Nov/19/2011
                UDM ret = MessageBox("&CfgPtn.DelectAllCruise", "&CfgPtn.WARNING" , MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2);
				if(UDM_OK != GetRunStatue())
				{
					return;
				}
                //end
				
				for(i=0; i<m_nTotal; i++)
				{
					pStatic[ID_START+i]->SetText("");				
					pStatic[NAME_START+i]->SetText("");
				}

				//bitTourAlive.reset();
                
				for(int j=0; j<TOURPATH_MAX; j++)
				{
                    if (bitTourAlive.test(j)) 
                    {
                        bitTourAlive.reset(j);
    					psGuiTourPath[m_nChn][j].nPathNo = 0;
                        for (int i=0; i<TOURPIONT_MAX; i++)
                        {
                            psGuiTourPath[m_nChn][j].sTourPoint[i].nPresetPos = 0;
                            psGuiTourPath[m_nChn][j].sTourPoint[i].nDwellTime = 0;
                            psGuiTourPath[m_nChn][j].sTourPoint[i].nSpeed = 0;
                        }
                    }
				}
				
				m_nTotal = 0;
				m_nCursel = 0;
                m_nCurPage = 0;
                LoadPage();
			}break;
			case 8:
			{
				//printf("CH %d :ok \n",m_nChn+1);
                /*
                for (int i=0; i<TOURPATH_MAX; ++i)
                {
                    m_pPageCruisePoint->OnGetPara(i, psGuiTourPath[m_nChn][i].sTourPoint);
                }
                */
				
				//((CPagePtzConfigFrameWork*)(this->pParent))->SaveCruiseLine(m_nChn, psGuiTourPath[m_nChn]);
                memcpy(psGuiPtzPara.sTourPath, psGuiTourPath[m_nChn], sizeof(psGuiPtzPara.sTourPath));
                int ret = 0;
                int saveFlag = 0;

                //add by Lirl on Nov/30/2011
                //有删除或添加操作时才作第一次保存
                for (int i=0; i<TOURPATH_MAX; i++)//cw_ptz
                {
           		    if (0 == bitTourAlive.test(i)) 
           		    {
                        psGuiPtzPara.sTourPath[i].nPathNo = 0;
                    }
                }
				/*
                for (int i=0; i<TOURPATH_MAX; i++)
                {		
                    if (bitRecTourStartStatus.test(i) ^ bitTourAlive.test(i)) {
                        saveFlag = 1;
                        break;
                    }
                }
		*/
        //        if (saveFlag)
		        {
                    ret = bizData_SavePtzAdvancedPara(m_nChn, &psGuiPtzPara);
                    bitRecTourStartStatus = bitTourAlive;
                }

                //第二次保存把删除航线置为0
                /*
                if (0 == ret) {
                    for (int i=0; i<TOURPATH_MAX; i++)
                    {
                        if (0 == bitTourAlive.test(i)) {
                            psGuiPtzPara.sTourPath[i].nPathNo = 0;
                        }
                    }
                    
                    if (saveFlag) {
                        usleep(500 * 1000);
                    }
                    
                    ret = bizData_SavePtzAdvancedPara(m_nChn, &psGuiPtzPara);
                }
                */
                //end
                
                #if 0
                for (int i=0; i<25; ++i)
                {
                    printf("***No:[%d]\n", psGuiPtzPara.sTourPath[i].nPathNo);
                }
                #endif
                return;
			}break;
			case 9:
			{
				//printf("CH %d :exit \n",m_nChn+1);
                //((CPagePtzConfigFrameWork *)(this->GetParent()))->SetOpenFlag(TRUE);
                #if 1
                if (m_nCursel >0)
            	{
            		pStatic[NAME_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
                    pStatic[ID_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
            		pStatic[NAME_START+m_nCursel-1]->Draw();
                    pStatic[ID_START+m_nCursel-1]->Draw();
            	}
                #endif
                this->GetParent()->Open();
				//this->Close(UDM_CLOSED);
				return (void)CPage::Close();
			}break;
			default:
			break;
		}
        
		CancelSel();

        if (bitTourAlive.any()) 
        {
            pButton[7]->Enable(TRUE);
        } 
        else 
        {
            pButton[7]->Enable(FALSE);
        }
	}
	
}

void CPageCruiseLine::CancelSel()
{
    #if 1
    for (int i=0; i < 9; ++i)
	{
		pStatic[NAME_START+i]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
        pStatic[ID_START+i]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pStatic[NAME_START+i]->Draw();
		pStatic[ID_START+i]->Draw();
    }
    #else
    if (m_nCursel > 0)
	{
		pStatic[NAME_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
        pStatic[ID_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pStatic[NAME_START+m_nCursel-1]->Draw();
        pStatic[ID_START+m_nCursel-1]->Draw();
	}
    #endif
    pButton[5]->Enable(FALSE);
    pButton[6]->Enable(FALSE);
    m_nCursel = 0;
}

CPageCruiseLine::~CPageCruiseLine()
{

}

VD_PCSTR CPageCruiseLine::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

int CPageCruiseLine::GetCurCLineNum()//cw_tab
{
	int num;
	if (m_nTotal<9)
	{
		num= m_nTotal;
    }
	else 
	{
		if(m_nMaxPage*9>m_nTotal)
		{
			num =m_nTotal-((m_nMaxPage-1)*9);
		}
		else
		{
			num= 9;
		}
	}
	return num;
}



VD_BOOL CPageCruiseLine::MsgProc( uint msg, uint wpa, uint lpa )
{
	int px,py;
	int temp;
	static int chSel=0;
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
					chSel=wpa -KEY_0;
					if(chSel<=GetCurCLineNum()&&chSel>=KEY_0)
					{
						SelectItem(chSel);
					}
				}break;
				
				case KEY_UP:
				{
					if (--chSel>=KEY_0)
					{
						SelectItem(chSel);
					}
					else
					{	
						chSel=GetCurCLineNum();
						SelectItem(GetCurCLineNum());
					}
				}break;
				
				case KEY_DOWN:
				{
					if (++chSel<=GetCurCLineNum())
					{
						SelectItem(chSel);
					}
					else
					{
						chSel=KEY_0;
						SelectItem(chSel);
					}
				}break;

				case KEY_PTZ:
					return FALSE;
				default: return CPageFrame::MsgProc(msg, wpa, lpa);
			}
			return TRUE;
		}break;
        case XM_RBUTTONDOWN:
	    case XM_RBUTTONDBLCLK:
            {
                #if 1
                if (m_nCursel >0)
            	{
            		pStatic[NAME_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
                    pStatic[ID_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
            		pStatic[NAME_START+m_nCursel-1]->Draw();
                    pStatic[ID_START+m_nCursel-1]->Draw();
            	}
                #endif
                this->GetParent()->Open();
				//this->Close(UDM_CLOSED);
				return CPage::Close();
	        } break;
	    case XM_LBUTTONDOWN:
        case XM_MOUSEMOVE:
    		{
    			px = VD_HIWORD(lpa);
    			py = VD_LOWORD(lpa);
    			VD_RECT rtScreen;
    			m_pDevGraphics->GetRect(&rtScreen);
    			int offsetx = (rtScreen.right - m_Rect.Width())/2;
    			int offsety = (rtScreen.bottom - m_Rect.Height())/2;

    			BOOL bFind = FALSE;
    			int i = 0;
    			for(i = 1; i<10; i++)
    			{
    				for (int kk=0; kk<2; kk++)
    				{
    					VD_RECT rt;
    					pTable0->GetTableRect(kk, i, &rt);
    					rt.left +=10+offsetx;
    					rt.top +=52+offsety;
    					rt.right +=10+offsetx;
    					rt.bottom +=52+offsety;
    					//printf("i=%d kk =%d ,x=%d, y=%d, %d %d %d %d \n",i, kk, px, py, rt.left, rt.top, rt.right,rt.bottom);
    					if (PtInRect(&rt, px, py))
    					{
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
    				//printf("SelectDskItem %d\n", i);
    				if (msg == XM_LBUTTONDOWN) 
    				{
                        //pButton[5]->Enable(TRUE);
    				    SelectItem(i);
                    }
                    else
                    {
                        MouseMoveToLine(i);
                    }
    				
    			} 
    			else
    			{
                    if (msg == XM_MOUSEMOVE)
                    {
                        MouseMoveToLine(0);
                    }
                }
    		}
		break;
	default:
		break;
	}

	return 	CPageFrame::MsgProc(msg, wpa, lpa);
}

void CPageCruiseLine::MouseMoveToLine(int index)
{
	if(index == m_nCurMouseLine)
	{
		return;
	}
	if((m_nCurMouseLine>0) && (strcmp(pStatic[ID_START+m_nCurMouseLine-1]->GetText(),"") != 0))
	{
		pStatic[ID_START+m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
        pStatic[NAME_START+m_nCurMouseLine-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT), 1);
	}
	m_nCurMouseLine = index;
	
	if (index < 1 || index>9 )
	{
		return;
	}

	if(strcmp(pStatic[ID_START+index-1]->GetText(),"") == 0)
	{
		return ;
	}

	pStatic[ID_START+index-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
    pStatic[NAME_START+index-1]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), 1);
}

void CPageCruiseLine::SelectItem( int index )
{
	if (index < 1 || index>10 )
	{
		return;
	}

	//printf(" idx=%d, cur =%d \n", index, m_nCursel);

	if (index == m_nCursel && !bFirstSel)
	{
        bFirstSel = TRUE;
		return;
	}

    if (0 == strcmp(pStatic[NAME_START+index-1]->GetText(), ""))
    {
        return;
    }
    
	bFirstSel = FALSE;
    pButton[5]->Enable(TRUE);
    pButton[6]->Enable(TRUE);
	
	if (m_nCursel >0)
	{
		pStatic[NAME_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
        pStatic[ID_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pStatic[NAME_START+m_nCursel-1]->Draw();
        pStatic[ID_START+m_nCursel-1]->Draw();
	}

	pStatic[NAME_START+index-1]->SetBkColor(VD_RGB(56,108,148));
    pStatic[ID_START+index-1]->SetBkColor(VD_RGB(56,108,148));
	pStatic[NAME_START+index-1]->Draw();
    pStatic[ID_START+index-1]->Draw();

	m_nCursel = index;
}

void CPageCruiseLine::GetTourLine(SGuiTourPath* psPIns)
{
	memcpy(psGuiTourPath[m_nChn], psPIns, sizeof(SGuiTourPath)*TOURPATH_MAX);
}

VD_BOOL CPageCruiseLine::UpdateData( UDM mode )
{
	if(UDM_OPEN == mode)
	{
        //第一页
        m_nlastPage = m_nCurPage = 0;
        m_nMaxPage = 0;
        m_nCursel = 0;
		m_nTotal = 0;
        bitTourAlive.set(); //全置为1
        bitRecTourStartStatus.set();
        
        if(m_nCursel >0)
        {
            pStatic[NAME_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
            pStatic[ID_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
            pStatic[NAME_START+m_nCursel-1]->Draw();
            pStatic[ID_START+m_nCursel-1]->Draw();
        }
		
		//SetStatus();
		
		//SGuiTourPath* pPIns;
		bizData_GetPtzAdvancedPara(m_nChn, &psGuiPtzPara);
        memcpy(psGuiTourPath[m_nChn], psGuiPtzPara.sTourPath, sizeof(psGuiPtzPara.sTourPath));
		
        for(int i=0; i<TOURPATH_MAX; i++)
        {
            if(0 == psGuiTourPath[m_nChn][i].nPathNo)
            {
                bitTourAlive.reset(i);
                bitRecTourStartStatus.reset(i); //记录,方便为保存时好作比较
            }
        }
        //std::cout <<"***bitTourAlive:" <<bitTourAlive <<std::endl;
		//((CPagePtzConfigFrameWork*)(this->pParent))->GetCruiseLine(m_nChn, psGuiTourPath[m_nChn]);
		#if 0
        for(int i=0; i<25; ++i)
        {
            printf("***No:[%d]\n", psGuiPtzPara.sTourPath[i].nPathNo);
        }
        #endif
        //先把设置和删除按键置为Enable，选择了对应的航线时才可用
        pButton[5]->Enable(FALSE);
        pButton[6]->Enable(FALSE);
		
        //有巡航线时才对"清除全部"按钮设置为可用
        if(bitTourAlive.any())
        {
            pButton[7]->Enable(TRUE);
        }
        else
        {
            pButton[7]->Enable(FALSE);
        }
		
        LoadPage();
		
        #if 0
		for(int i=0; i<ID_END-ID_START; i++)
		{
			pStatic[ID_START+i]->SetText("");
			pStatic[NAME_START+i]->SetText("");
		}
		
		char szCl[32] = {0};
		
		for(int i=0; i<TOURPATH_MAX; i++)
		{
			if(TOUR_LIMIT<=m_nTotal)
			{
				break;
			}
			
			pPIns = &psGuiTourPath[m_nChn][i];
			
			//printf("Got path no %d\n", pPIns->nPathNo);
			
			if(pPIns->nPathNo>=TOURPATH_MAX || pPIns->nPathNo<=0) continue;
			
			//id 1
			sprintf(szCl, "%d", pPIns->nPathNo);
			pStatic[ID_START+m_nTotal]->SetText(szCl);
			
			//name 1
			sprintf(szCl, "&CfgPtn.Cruise", pPIns->nPathNo);
			
			pStatic[NAME_START+m_nTotal]->SetText(szCl);
			
			m_nTotal++;
			
			SetStatus();
		}
        #endif
		CItem *pItemSpecial = NULL;
		pItemSpecial = FindPage(m_screen, " Ptz Control", FALSE);
		((CPagePtzCtl* )pItemSpecial)->SetCruiseStage(TRUE);
	}
	if(UDM_CLOSED == mode)
	{
		CItem *pItemSpecial = NULL;
		pItemSpecial = FindPage(m_screen, " Ptz Control", FALSE);
		((CPagePtzCtl* )pItemSpecial)->SetCruiseStage(FALSE);
	}
	return TRUE;
}

#if 0
void CPageCruiseLine::OnSetPara(uchar nCh, SGuiTourPath* psPIns)
{
	if(psGuiTourPath[nCh]&&nCh<=nChMax)
	{
		memcpy(psGuiTourPath[nCh], psPIns, sizeof(SGuiTourPath)*TOURPATH_MAX);
	}
	
	return;
}

void CPageCruiseLine::OnGetPara(uchar nCh, SGuiTourPath* psPIns)
{
	if(psGuiTourPath[nCh]&&nCh<=nChMax)
	{
		memcpy(psPIns, psGuiTourPath[nCh], sizeof(SGuiTourPath)*TOURPATH_MAX);
	}
	
	return;
}
#endif

void CPageCruiseLine::DelItem(uchar nIdx)
{
	int  i;
	
	// last item
	// delete
	if(m_nCursel==m_nTotal)
	{
		pStatic[ID_START+m_nTotal-1]->SetText("");		
		pStatic[NAME_START+m_nTotal-1]->SetText("");		
	}
	// else
	// move all-down to up
	else
	{
		for(i=0; i<m_nTotal-m_nCursel; i++)
		{
			pStatic[ID_START+m_nCursel+i-1]->SetText(
				pStatic[ID_START+m_nCursel+i]->GetText()
			);
			
			pStatic[NAME_START+m_nCursel+i-1]->SetText(
				pStatic[NAME_START+m_nCursel+i]->GetText()
			);
		}
		
		pStatic[ID_START+m_nCursel+i-1]->SetText("");
		pStatic[NAME_START+m_nCursel+i-1]->SetText("");
	}
	
	// focus item
	// clear
	if(pStatic[ID_START+m_nCursel-1]->GetText()=="")
	{
		pStatic[NAME_START+m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pStatic[NAME_START+m_nCursel-1]->Draw();		
	}
	
	SetStatus();
}

void CPageCruiseLine::DelPath(uchar nPathNo)
{
	//csp modify
	//if(nPathNo < 0 || nPathNo >= TOURPATH_MAX)
	if(nPathNo >= TOURPATH_MAX)
	{
		return;
	}
	
    //删除某条巡航线时清空所有的置点
    if (bitTourAlive.test(nPathNo)) 
    {
        bitTourAlive.reset(nPathNo);
        for (int i=0; i<TOURPIONT_MAX; i++)
        {
            psGuiTourPath[m_nChn][nPathNo].sTourPoint[i].nPresetPos = 0;
            psGuiTourPath[m_nChn][nPathNo].sTourPoint[i].nDwellTime = 0;
            psGuiTourPath[m_nChn][nPathNo].sTourPoint[i].nSpeed = 0;
        }
    }
    /*
	for(int j=0; j<TOURPATH_MAX; j++)
	{
		if(psGuiTourPath[m_nChn][j].nPathNo==nPathNo)
		{
			//psGuiTourPath[m_nChn][j].nPathNo = 0;
			bitTourAlive.reset(j);
            for (int i=0; i<TOURPIONT_MAX; i++)
            {
                psGuiTourPath[m_nChn][j].sTourPoint[i].nPresetPos = 0;
                psGuiTourPath[m_nChn][j].sTourPoint[i].nDwellTime = 0;
                psGuiTourPath[m_nChn][j].sTourPoint[i].nSpeed = 0;
            }
			break;
		}
	}
	*/

	// clear static 
	// move down to up
	DelItem(m_nCursel);

	m_nTotal--;

	SetStatus();
}

void CPageCruiseLine::SetStatus()
{
	char szSta[6] = {0};
    if (0 == m_nTotal && 0 == m_nCurPage && 0 == m_nMaxPage)
    {
        m_nCurPage = -1;
        m_nMaxPage = -1;
    }

	sprintf(szSta, "%d/%d", m_nCurPage+1, m_nMaxPage+1);

	pStatic[20]->SetText(szSta);
}

void CPageCruiseLine::AddPath(SGuiTourPath* pPIns)
{
	char szCl[32] = {0};

	//printf("Got path no %d\n", pPIns->nPathNo);
	if(pPIns->nPathNo>TOURPATH_MAX || pPIns->nPathNo<=0)
	{
        return;
    }

    m_nTotal = 0;
    for (int i = 0; i<9; ++i)
    {
        if (strtol(pStatic[ID_START+i]->GetText(), 0, 10)>0)
        {
            ++m_nTotal;
        }
        else
        {
            break;
        }
    }
    
	if (m_nTotal >= 9)
	{
        ++m_nMaxPage;
        m_nTotal = 0;
        m_nCurPage = m_nMaxPage;
        for(int i=0; i<TOURPATH_MAX; i++)
    	{
    		if(0 == bitTourAlive.test(i))
    		{
    			psGuiTourPath[m_nChn][i].nPathNo = pPIns->nPathNo;
                bitTourAlive.set(i);
    			break;
    		}
    	}

        for (int i = 0; i<9; ++i)
        {
            pStatic[ID_START+i]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
            pStatic[ID_START+i]->Draw();
            pStatic[NAME_START+i]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
            pStatic[NAME_START+i]->Draw();
            
            pStatic[ID_START+i]->SetText("");
            pStatic[NAME_START+i]->SetText("");
        }
        
        //id 1
    	sprintf(szCl, "%d", pPIns->nPathNo);
    	pStatic[ID_START+m_nTotal]->SetText(szCl);

    	//name 1
    	sprintf(szCl, "&CfgPtn.Cruise", pPIns->nPathNo);	
    	pStatic[NAME_START+m_nTotal]->SetText(szCl);
        //LoadPage();
    }
    else
    {
        if (-1 == m_nCurPage && -1 == m_nMaxPage)
        {
            m_nCurPage = 0;
            m_nMaxPage = 0;
        }
        //id 1
    	sprintf(szCl, "%d", pPIns->nPathNo);
    	pStatic[ID_START+m_nTotal]->SetText(szCl);

    	//name 1
    	sprintf(szCl, "&CfgPtn.Cruise");	
    	pStatic[NAME_START+m_nTotal]->SetText(szCl);
        
    	for(int i=0; i<TOURPATH_MAX; i++)
    	{
    		if(0 == bitTourAlive.test(i))
    		{
    			psGuiTourPath[m_nChn][i].nPathNo = pPIns->nPathNo;
                bitTourAlive.set(i);
    			break;
    		}
    	}
    }

    CancelSel();

    /*
    pStatic[NAME_START+m_nTotal]->SetBkColor(VD_RGB(56,108,148));
    pStatic[ID_START+m_nTotal]->SetBkColor(VD_RGB(56,108,148));
	pStatic[NAME_START+m_nTotal]->Draw();
    pStatic[ID_START+m_nTotal]->Draw();
    */

    ++m_nTotal;
    SelectItem(m_nTotal);
    //m_nCursel = m_nTotal;
	SetStatus();
}


void CPageCruiseLine::OnGetPathId(uchar nCh, uchar* nId)
{
	//int i, j;
    //std::cout<<"**bitTourAlive:" <<bitTourAlive <<std::endl;
    for (int i = 0; i<TOURPATH_MAX; i++)
    {
        if (0 == bitTourAlive.test(i))
        {
            *nId = i+1;
            //bitTourAlive.set(i);
            return;
        }
    }
    #if 0
	for(i=1; i<=TOURPATH_MAX; i++)
	{
		for(j=0; j<TOURPATH_MAX; j++)
		{
			//if(psGuiTourPath[nCh][j].nPathNo==i)
			if (0 == bitTourAlive.test(j))
			{
				break;
			}
		}
		
		if(j==TOURPATH_MAX)
		{
			*nId = i;
			return;
		}
	}
    #endif
}

VD_BOOL CPageCruiseLine::Close(UDM mode)
{
    this->m_pParent->Open();
    return CPage::Close();
}

void CPageCruiseLine::GetCruisePoint(uchar nCh, SGuiTourPath* psPIns)
{
	memcpy( psPIns, psGuiTourPath[nCh], sizeof(SGuiTourPath)*TOURPATH_MAX);
    #if 0
    printf("***Fn:(%s)\n", __FUNCTION__);
    for (int i = 0; i<6; ++i)
    {
        printf("***nCh:[%d], Line:[%d], Point:[%d], Speed:[%d]\n",
            m_nChn,
            psGuiTourPath[m_nChn][nCh].nPathNo,
            psGuiTourPath[m_nChn][nCh].sTourPoint[i].nPresetPos,
            psGuiTourPath[m_nChn][nCh].sTourPoint[i].nSpeed);
    }
    #endif
}

void CPageCruiseLine::SetCruisePoint(uchar nCh, SGuiTourPath* psPIns)
{
	memcpy( psGuiTourPath[nCh], psPIns, sizeof(SGuiTourPath)*TOURPATH_MAX);
    #if 0
    printf("***Fn:(%s)\n", __FUNCTION__);
    for (int i = 0; i<6; ++i)
    {
        printf("***nCh:[%d], Line:[%d], Point:[%d], Speed:[%d]\n",
            m_nChn,
            psGuiTourPath[m_nChn][nCh].nPathNo,
            psGuiTourPath[m_nChn][nCh].sTourPoint[i].nPresetPos,
            psGuiTourPath[m_nChn][nCh].sTourPoint[i].nSpeed);
    }
    #endif
}

void CPageCruiseLine::SetChn(int chn)
{
	m_nChn = chn;
	
	char title[256] = {0};//char title[32] = {0};//csp modify
	sprintf(title, "%s - %s%d", GetParsedString("&CfgPtn.Cruise"), GetParsedString("&CfgPtn.Channel"), m_nChn+1);
	
	SetTitle(title);
}

int CPageCruiseLine::bGetCrLineNum(int index)
{
    //return psGuiTourPath[m_nChn][index].nPathNo;
    return bitTourAlive.test(index);
}

void CPageCruiseLine::LoadPage()
{
    //SGuiTourPath* pPIns;
    char szCl[32] = {0};

    m_nCursel = 0;
    m_nTotal = 0;
    m_nMaxPage = 0;

    int nCLIdx = -1;
    int k = 0;
    
    for(int i=0; i<ID_END-ID_START; i++)
    {
        pStatic[ID_START+i]->SetText("");
        pStatic[NAME_START+i]->SetText("");
    }
    for (int i=0; i<TOURPATH_MAX; ++i)
    {
        if (psGuiTourPath[m_nChn][i].nPathNo>0 && bitTourAlive.test(i))
        {
            ++nCLIdx;
            memset(&pPIns[nCLIdx], 0, sizeof(SGuiTourPath));
            memcpy(&pPIns[nCLIdx], &psGuiTourPath[m_nChn][i], sizeof(SGuiTourPath));

            if (k>=9)
                continue;
            
            int nID = m_nCurPage*9+k;
            if (pPIns[nID].nPathNo>0 && nID<=nCLIdx)
            {
                //id 1
            	sprintf(szCl, "%d", pPIns[nID].nPathNo);
            	pStatic[ID_START+k]->SetText(szCl);

            	//name 1
            	sprintf(szCl, "&CfgPtn.Cruise");    	
            	pStatic[NAME_START+k]->SetText(szCl);

                ++m_nTotal;
                ++k;
            }
        }
    }

    if (nCLIdx>=0)
    {
        m_nMaxPage = nCLIdx / 9;
    }
    else
    {
        m_nMaxPage = 0;
    }

    /*
    for(int i=0; i<9; i++)
    {
    	if(TOUR_LIMIT<=m_nTotal)
    	{
    		break;
    	}
    	int nID = m_nCurPage*9+i;
        
    	pPIns = &psGuiTourPath[m_nChn][nID];

    	//printf("Got path no %d\n", pPIns->nPathNo);
    	
    	if(pPIns->nPathNo>TOURPATH_MAX || pPIns->nPathNo<=0)
            continue;

    	//id 1
    	sprintf(szCl, "%d", pPIns->nPathNo);
    	pStatic[ID_START+m_nTotal]->SetText(szCl);

    	//name 1
    	sprintf(szCl, "&CfgPtn.Cruise", pPIns->nPathNo);    	
    	pStatic[NAME_START+m_nTotal]->SetText(szCl);

    	m_nTotal++;    				
    }
    */
    
    SetStatus();
    #if 0
    for(int i=0; i<9; i++)
	{
		char szID[16] = {0};
		int nID = m_nCurPage*9+1+i;
		sprintf(szID,"%d",nID);
		pID[i]->SetText(szID);


		pEnable[i]->SetValue(psGuiPtzPara.nIsPresetSet[nID]);

		char szName[32] = {0};
		sprintf(szName,"preset%d",nID);
		pName[i]->SetText(szName);
	}

    if(m_nCurPage == 15)
	{
		for(int i=8; i<9; i++)
		{
			pID[i]->Show(FALSE,TRUE);
			pEnable[i]->Show(FALSE,TRUE);
			pName[i]->Show(FALSE,TRUE);
			pPreset[i]->Show(FALSE,TRUE);
		}
	}
	else
	{
		for(int i=8; i<9; i++)
		{
			pID[i]->Show(TRUE,TRUE);
			pEnable[i]->Show(TRUE,TRUE);
			pName[i]->Show(TRUE,TRUE);
			pPreset[i]->Show(TRUE,TRUE);
		}
	}
    #endif
}

