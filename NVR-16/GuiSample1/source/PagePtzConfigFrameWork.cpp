#include "GUI/Pages/PagePtzConfigFrameWork.h"
#include "GUI/Pages/PageCruiseLine.h"
#include "GUI/Pages/PagePtzTrack.h"
#include "GUI/Pages/PagePtzPreset.h"
#include "GUI/Pages/PagePresetSetup.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageConfigFrameWork.h"
#include "sg_platform.h"


static	std::vector<CItem*>	items[PTZCFG_SUBPAGES];

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static VD_BITMAP * pBmp_tab_normal;
static VD_BITMAP * pBmp_tab_push;
static VD_BITMAP * pBmp_tab_select;

static VD_BITMAP * pBmp_infoicon;

int bizData_GetPtzAdvancedPara(uchar nChn, SGuiPtzAdvancedPara* pGDispatch);
int bizData_GetPtzAdvancedParaDefault(uchar nChn, SGuiPtzAdvancedPara* pGDispatch);
int bizData_SavePtzAdvancedPara(uchar nChn, SGuiPtzAdvancedPara* pGDispatch);
int bizData_GetUartPara(uchar nChn, STabParaUart* pTabPara);
int bizData_GetUartParaDefault(uchar nChn, STabParaUart* pTabPara);
int bizData_SetUartPara(uchar nChn, STabParaUart* pTabPara);

static int GetMaxChnNum() 
{
    return GetVideoMainNum(); 
}

static STabSize TabSzPtz[] = {
	{ 5, 6 },
	{ 4, 8 }
};

CPagePtzConfigFrameWork::CPagePtzConfigFrameWork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
, curID(0)
{
	nChMax = GetMaxChnNum();

    for(int i=0; i<nChMax; ++i)
    {
        pcPtzAddr[i] = (char *)malloc(sizeof(char)*EDITLEN);
    }

	for(int i=0; i<TAB_NUM; i++)
	{
		if(TabSzPtz[i].nRow >= nChMax)
		{
			nRealRow[i] = nChMax;
			nPageNum[i] = 1;
			nHideRow[i] = 0;
		}
		else
		{
			nRealRow[i] = TabSzPtz[i].nRow;
			nPageNum[i] = (nChMax+nRealRow[i]-1)/nRealRow[i];			
			nHideRow[i] = nPageNum[i]*nRealRow[i]-nChMax;
		}
		
		nCurPage[i] = 0;
	}
	
	//SetRect(CRect(0, 0, 700, 400/*370*/), FALSE);
	SetMargin(3, m_TitleHeight, 4, m_TitleHeight);

	//pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/button_normal.bmp");
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//pBmp_button_push = VD_LoadBitmap(DATA_DIR"/button_push.bmp");
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//pBmp_button_select = VD_LoadBitmap(DATA_DIR"/button_select.bmp");
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	//第一个页面的子页面控制按钮  录像设置，手动录像，录像布防
	pBmp_tab_normal = VD_LoadBitmap(DATA_DIR"/temp/tab.bmp");
	pBmp_tab_push = VD_LoadBitmap(DATA_DIR"/temp/tab_check.bmp");
	pBmp_tab_select = VD_LoadBitmap(DATA_DIR"/temp/tab_f.bmp");
	char* szSubPgName[PTZCFG_BTNNUM] = {
		"&CfgPtn.SerialPort",
		"&CfgPtn.Advanced",	
		"&CfgPtn.Default",
		"&CfgPtn.apply",
		"&CfgPtn.Exit",
	};
	
	int szSubPgNameLen[PTZCFG_BTNNUM] = {
		TEXT_WIDTH*5,//"&CfgPtn.SerialPort",
		TEXT_WIDTH*5,//"&CfgPtn.Advanced",	
		TEXT_WIDTH*3,//"&CfgPtn.Default",
		TEXT_WIDTH*2,//"&CfgPtn.apply",
		TEXT_WIDTH*2//"&CfgPtn.Exit",
	};
	
	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);
	
	CTableBox* pTabFrame = CreateTableBox(CRect(17, 40, m_Rect.Width()-17, 40+290),this,1,1);
	//pTabFrame->SetFrameColor(VD_RGB(56,108,148));
	pTabFrame->SetFrameColor(VD_RGB(8,66,189));
	
	CRect rtSub(25, 10, m_Rect.Width()-20, 10+pBmp_tab_normal->height);
	int i;
	for (i = 0; i<PTZCFG_SUBPAGES; i++)
	{
		int btWidth = szSubPgNameLen[i]+10;
		rtSub.right = rtSub.left + btWidth;
		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPagePtzConfigFrameWork::OnClickSubPage, NULL, buttonNormalBmp, 1);
		pButton[i]->SetBitmap(pBmp_tab_normal, pBmp_tab_select, pBmp_tab_push, pBmp_tab_push);
		rtSub.left = rtSub.right;
	}

	CRect rtSub1(295, m_Rect.Height()-80, m_Rect.Width()-20, m_Rect.Height()-80+pBmp_button_normal->height);

	pInfoBar = CreateStatic(CRect(40, 
								rtSub1.top,
								317,
								rtSub1.top+22), 
    							this, 
    							"");
	//pInfoBar->SetBkColor(VD_RGB(8,66,189));

	
	pBmp_infoicon = VD_LoadBitmap(DATA_DIR"/temp/icon_info.bmp");
	pIconInfo = CreateStatic(CRect(17, 
								rtSub1.top+1,
								37,
								rtSub1.top+21), 
    							this, 
    							"");
	pIconInfo->SetBitmap(pBmp_infoicon,NULL);
	pIconInfo->Show(FALSE);
	
	rtSub1.left = rtSub1.right;
	for (i=PTZCFG_SUBPAGES; i<PTZCFG_BTNNUM; i++)
	{
		rtSub1.left -= szSubPgNameLen[i]+10;
	}
	rtSub1.left -= 12*2;

	for(i=PTZCFG_SUBPAGES; i<PTZCFG_BTNNUM; i++)
	{
		int btWidth = szSubPgNameLen[i]+10;
		rtSub1.right = rtSub1.left + btWidth;
		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPagePtzConfigFrameWork::OnClickSubPage, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
		rtSub1.left = rtSub1.right+12;
	}
	
	//CRect rtSub2(13, 35, m_Rect.Width()-18, 300);
	//pTable = CreateTableBox(&rtSub2,this, 1,1,0);

	//printf("CPagePtzConfigFrameWork 0000\n");
	
	//for(i=0; i<4; i++)
	#if 1
	{
		char title[32] = {0};
		//sprintf(title,"%s - %s",GetParsedString("&CfgPtn.Cruise"), GetParsedString("&CfgPtn.Channel"));
		//创建一个巡航线页面
		m_pPageCruiseLine = new CPageCruiseLine(NULL, NULL, icon_dvr, this, i);
	}
	#endif

	//printf("CPagePtzConfigFrameWork 000\n");
	
	CRect m_RectScreen;
	m_pDevGraphics = CDevGraphics::instance(m_screen);
	m_pDevGraphics->GetRect(&m_RectScreen);

	CRect rtFloat;
	rtFloat.left = (m_RectScreen.Width()-436)/2;
	rtFloat.right = rtFloat.left + 436;
	rtFloat.top = m_RectScreen.bottom - 212 ;
	rtFloat.bottom = rtFloat.top + 182;
	//printf("CPagePtzConfigFrameWork 0\n");
    //创建一个轨迹页面
	m_pPagePtzTrack =  new CPagePtzTrack(rtFloat, NULL , icon_dvr, this);


	//CRect rtFloat;
	rtFloat.left = (m_RectScreen.Width()-456)/2;
	rtFloat.right = rtFloat.left + 456;
	rtFloat.top = m_RectScreen.bottom - 212 ;
	rtFloat.bottom = rtFloat.top + 182;
	
	//printf("CPagePtzConfigFrameWork 00\n");
    //创建一个置点页面
	m_pPagePtzPreset = new CPagePtzPreset(rtFloat, NULL , icon_dvr, this);
    //分配云台页面数据结构的足够空间
	psGuiPtzPara = (SGuiPtzAdvancedPara *)malloc(sizeof(SGuiPtzAdvancedPara)*(nChMax+1));
	if(!psGuiPtzPara)
	{
		exit(1);
	}
	
	//printf("CPagePtzConfigFrameWork 1\n");
	//创建置点设置页面
	m_pPagePresetSetup = new CPagePresetSetup(NULL, "", icon_dvr, this);
	
	//printf("CPagePtzConfigFrameWork 11\n");
	
	InitPage0(); //初始化0页面
	//printf("CPagePtzConfigFrameWork 111\n");
	InitPage1(); //初始化1页面
	
	//printf("CPagePtzConfigFrameWork 1111\n");
}

void CPagePtzConfigFrameWork::OnClickSubPage()
{
	//printf("OnClickSubPage\n");
	static int m_nCurID = 0;
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < PTZCFG_BTNNUM; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		//printf("find the focus button\n");
		if(i<PTZCFG_SUBPAGES)
		{
            //选中子页面
			m_nCurID = i;
			SwitchPage(i);
		}
		else
		{
			if(i==PTZCFG_SUBPAGES+1) // apply
			{
               	printf("Apply...\n");
				switch(m_nCurID)
				{
					case 0://串口
					{						
						//参数保存
						SSG_MSG_TYPE msg;
						memset(&msg, 0, sizeof(msg));
						msg.type = EM_DVR_PARAM_SAVE;
						msg.chn = 0;
						strcpy(msg.note, GetParsedString("&CfgPtn.PTZ"));
						strcat(msg.note, GetParsedString("&CfgPtn.ParamSave"));
						upload_sg(&msg);
						
                        int ret = 0;
						for(i=0;i<nChMax;i++)
						{
							//字符串不全是数字/超出范围
							//csp modify
							//if(!Digital(pcPtzAddr[i]) || (*pcPtzAddr[i] != NULL) && (atoi(pcPtzAddr[i])<0 || atoi(pcPtzAddr[i])>255))
							if(!Digital(pcPtzAddr[i]) || (pcPtzAddr[i] != NULL) && (atoi(pcPtzAddr[i])<0 || atoi(pcPtzAddr[i])>255))
							{
								MessgeBox(i);
								return;
							}
						}
                        
						// all
						if(pCheckBox0[nRealRow[0]]->GetValue())
						{
                            //勾选全选时设置所有通道的当前属性
							for(i=0; i<nChMax; i++)
							{
								pTabPara[i].bEnable = pCheckBox0[nRealRow[0]+1]->GetValue();
								pTabPara[i].nBaudRate = pCombo0[2*nRealRow[0]]->GetCurSel();
								pTabPara[i].nProtocol = pCombo0[2*nRealRow[0]+1]->GetCurSel();
							}
                            
							//设置当前页面控件的显示数据
							for(i=0;i<nRealRow[0];i++)
							{
								pCheckBox0[i]->SetValue(pCheckBox0[nRealRow[0]+1]->GetValue());
								pCombo0[i]->SetCurSel(pCombo0[2*nRealRow[0]]->GetCurSel());
								pCombo0[i+nRealRow[0]]->SetCurSel(pCombo0[2*nRealRow[0]+1]->GetCurSel());
							}
						}

                        unsigned int tmp;
                        for (i=0; i<nChMax;i++)
                        {
                            sscanf(pcPtzAddr[i],"%u", &tmp);
                            pTabPara[i].nAddress = tmp;
                            ret |= bizData_SetUartPara(i, &pTabPara[i]);
                        }

						BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_SERIAL_CONFIG);

						if(ret == 0)
						{
							SetInfo("&CfgPtn.SaveConfigSuccess");
						}
						else
						{
							SetInfo("&CfgPtn.SaveConfigFail");
						}
					}
					break;
					case 1://高级配置
					{
						//参数保存
						SSG_MSG_TYPE msg;
						memset(&msg, 0, sizeof(msg));
						msg.type = EM_DVR_PARAM_SAVE;
						msg.chn = 0;
						strcpy(msg.note, GetParsedString("&CfgPtn.PTZ"));
						strcat(msg.note, GetParsedString("&CfgPtn.ParamSave"));
						upload_sg(&msg);

						
						//SPresetPara sPresetIns;
						//SGuiTourPath sCruiseLineIns;
						//STrackPara sTrackIns;
						int ret = 0;

                        /*
						for(int i=0; i<nChMax; i++)
						{
							//m_pPagePtzPreset->OnGetPara(i, &sPresetIns);
							//m_pPagePresetSetup->OnGetPara(i, psGuiPtzPara[i].nIsPresetSet);
                            m_pPageCruiseLine->OnGetPara(i, psGuiPtzPara[i].sTourPath);

                            /*
							for(int j=0; j<128; j++)
								psGuiPtzPara[i].nIsPresetSet[j] = sPresetIns.nId[j];
							
							//m_pPagePtzPreset->OnGetPara(i, &sPIns);
							ret |= bizData_SavePtzAdvancedPara(i, &psGuiPtzPara[i]);	
						}
                        */

                        /*
						int ret = 0;
						for(int j=0; j<nChMax; j++)
						{
                            //保存高级配置的相关数据到FLASH
							ret |= bizData_SavePtzAdvancedPara(j, &psGuiPtzPara[j]);
						}
						*/

						if(ret == 0)
						{
							SetInfo("&CfgPtn.SaveConfigSuccess");
						}
						else
						{
							SetInfo("&CfgPtn.SaveConfigFail");
						}
						//BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_CRUISELINE);
					}
					break;
				}
			}
			else if(i==PTZCFG_SUBPAGES)
			{
				//设置默认属性
				switch(curID)
				{
                    //串口页面
					case 0:
					{
						int ret = 0;
						for(int i=0; i<nChMax; i++)
						{
                            //获取串口页面的默认数据
							ret |= bizData_GetUartParaDefault(i,&pTabPara[i]);
						}
                        
						for(int i=0; i<nRealRow[0]; i++)
						{
                            char s[10]={0};
                            //地址
                            sprintf(s, "%d", pTabPara[nCurPage[0]*nRealRow[0]+i].nAddress);//cw_9508S
                            strncpy(pcPtzAddr[i],s,sizeof(s));
                            pEdit0[i]->SetText(s);
                            
							pCheckBox0[i]->SetValue(pTabPara[nCurPage[0]*nRealRow[0]+i].bEnable);                            
							pCombo0[i]->SetCurSel(pTabPara[nCurPage[0]*nRealRow[0]+i].nBaudRate);
							pCombo0[i+nRealRow[0]]->SetCurSel(pTabPara[nCurPage[0]*nRealRow[0]+i].nProtocol);
						}

                        if(ret == 0)
						{
							SetInfo("&CfgPtn.LoadDefaultSuccess");
						}
						else
						{
							SetInfo("&CfgPtn.LoadDefaultFail");
						}
						
					} break;
                    //高级配置页面
					case 1:
					{
                        sleep(2);
                        SetInfo("&CfgPtn.LoadDefaultSuccess");
                        /*
						int rtn = 0;
						for(int i=0; i<nChMax; i++)
							rtn = bizData_GetPtzAdvancedParaDefault(i, &psGuiPtzPara[i]);
                        
						if(rtn >= 0)
						{
							SetInfo("&CfgPtn.LoadDefaultSuccess");
						}
						else
						{
							SetInfo("&CfgPtn.LoadDefaultFail");
						}
						*/
					} break;
				}
				

			}
			else
			{
				//exit
				//this->GetParent()->GetParent()->Open();
//                this->m_pParent->Open();
				this->m_pParent->Show(TRUE);
                //this->Close(UDM_CLOSED);
                return (void)CPage::Close();

			}

		}
		
		
	}
	
}

CPagePtzConfigFrameWork::~CPagePtzConfigFrameWork()
{

}

VD_PCSTR CPagePtzConfigFrameWork::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

void CPagePtzConfigFrameWork::OnCombo0()
{
	int i = 0, j;
	BOOL bFind = FALSE;
	CComboBox* pFocusCombo = (CComboBox*)GetFocusItem();
	for (i = 0; i < 2*nRealRow[0]; i++)
	{
		if (pFocusCombo == pCombo0[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		int nChBase = nCurPage[0]*nRealRow[0];
        if(i<nRealRow[0])
		{
			pTabPara[nChBase+i].nBaudRate= pFocusCombo->GetCurSel();
		}
		else if(i<2*nRealRow[0])
		{				
			pTabPara[nChBase+i - nRealRow[0]].nProtocol= pFocusCombo->GetCurSel();
		}
	}	
}

void CPagePtzConfigFrameWork::OnCheckBox0()
{
	int i = 0, j;
	BOOL bFind = FALSE;
	CCheckBox* pFocus = (CCheckBox*)GetFocusItem();
	for (i = 0; i < nRealRow[0]; i++)
	{
		if (pFocus == pCheckBox0[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		int nChBase = nCurPage[0]*nRealRow[0];
		if(i<nRealRow[0])
		{
			pTabPara[nChBase+i].bEnable = pFocus->GetValue();
		}
	}
}

void CPagePtzConfigFrameWork::OnEdit0()
{
    int i = 0;
    int tmpIndex = 0;
    BOOL bFind = FALSE;
    CEdit *pFocus = (CEdit *)GetFocusItem();
    for (i = 0; i < nRealRow[0]; i++)
    {
        if (pFocus == pEdit0[i])
        {
            bFind = TRUE;
            tmpIndex = i;
            break;
        }
    }

    if (bFind)
    {
        int nChBase = nCurPage[0]*nRealRow[0];
        char tmp[10] = {0};
        if (i < nRealRow[0])
        {
            pFocus->GetText(tmp, sizeof(tmp));
        }
        /*
        if (!Digital(tmp))
        { //字符串不全是数字
            MessgeBox(tmpIndex);
            return;
        }

        //是数字时是否有超出界
        if ((*tmp != NULL) && (atoi(tmp)<0 || atoi(tmp)>255))
        {
            MessgeBox(tmpIndex);
            return;
        }
        */
        memset(pcPtzAddr[nChBase+i], 0, EDITLEN);
        memcpy(pcPtzAddr[nChBase+i], tmp, sizeof(tmp)); 
        //pTabPara[nChBase+i].nAddress = atoi(tmp);
    }
}

void CPagePtzConfigFrameWork::OnTrackMove0()
{
	VD_BOOL bHide = FALSE;
	int 	idx;
    char s[10]={0};
	
	int pos = pScrollbar[curID]->GetPos();
    if (nCurPage[curID]==pos)
    {
        return;
    }
    nCurPage[curID] = pos;

	//printf("nCurPage = %d \n", nCurPage[curID]);

	int firstHide = nRealRow[curID]-nHideRow[curID];
	int lastHide = nRealRow[curID];
	int nRow     = nRealRow[curID];
	int nCurPg   = nCurPage[curID];
	int nPgNum   = nPageNum[curID];
	int nCol     = TabSzPtz[curID].nCol;
	CStatic**	pStatic = (curID==0)?pStatic0:pStatic1;
	for(int i=0; i<nRealRow[curID]; i++)
	{
		char szID[16] = {0};
		idx = nCurPage[curID]*nRealRow[curID]+i;
		sprintf(szID,"%d",idx+1);
		pStatic[i+nCol]->SetText(szID);
	}	
	
	bHide = (nCurPg == nPgNum-1)?FALSE:TRUE;
			
	switch(curID)
	{
		case 0: // get current page, refresh ctrls on current page
		{
			for(int i=0; i<nRow; i++)
			{
                idx = nCurPage[curID]*nRealRow[curID]+i;
				pCheckBox0[i]->SetValue(pTabPara[idx].bEnable);
				pEdit0[i]->SetText(pcPtzAddr[idx]);
				pCombo0[i]->SetCurSel(pTabPara[idx].nBaudRate);
				pCombo0[i+nRow]->SetCurSel(pTabPara[idx].nProtocol);
				
				if(idx >= nChMax - 1)
				{
					break;
				}
			}
			
			for (int i=firstHide; i<lastHide; i++)
			{
				pStatic0[i+nCol]->Show(bHide,TRUE);			
				pCheckBox0[i]->Show(bHide,TRUE);
				pEdit0[i]->Show(bHide,TRUE);
				pCombo0[i]->Show(bHide,TRUE);
				pCombo0[i+nRow]->Show(bHide,TRUE);
			}
		} break;
		case 1:
		{
			// load para
			
			for (int i=firstHide; i<lastHide; i++)
			{					
				pStatic1[i+nCol]->Show(bHide,TRUE);
				pButton1[i]->Show(bHide,TRUE);
				pButton1[i+nRow]->Show(bHide,TRUE);
				pButton1[i+nRow*2]->Show(bHide,TRUE);
			}
		} break;
	}
}

void CPagePtzConfigFrameWork::InitPage0()
{	
	pTabPara = (STabParaUart*)malloc(sizeof(STabParaUart) * nChMax);
	if(!pTabPara)
	{
		printf("no mem!\n");
		exit(1);
	}
	
	//printf("111111\n");
	char* statictext[] = {
		"&CfgPtn.Channel",
		"&CfgPtn.Enable",
		"&CfgPtn.Address",
		"&CfgPtn.BaudRate",
		"&CfgPtn.Protocol",
		/*
		"1",
		"2",
		"3",
		"4",
		*/
		"&CfgPtn.All",
	};
	
	CRect rtSubPage(m_Rect.left+27, m_Rect.top+50, 
	m_Rect.left+m_Rect.Width()-27,m_Rect.top+50+210);
	
	//printf("dddddddd  %d \n",m_Rect.top+m_Rect.Height()-170 - (m_Rect.top+43));
	pTable00 = CreateTableBox(&rtSubPage,this, TabSzPtz[0].nCol,TabSzPtz[0].nRow+1,0);
	pTable00->SetColWidth(0,60);
	pTable00->SetColWidth(1,80);
	pTable00->SetColWidth(2,130);
	//pTable00->SetColWidth(3,150);
	pTable00->SetColWidth(3,150);
	items[0].push_back(pTable00);
	
	if(nPageNum[0]>1)
	{
		pScrollbar[0] = CreateScrollBar(CRect(m_Rect.Width()-27-SCROLLBAR,
											m_Rect.top+80,
											m_Rect.Width()-27,
											m_Rect.top+50+210), 
											this,
											scrollbarY,
											0,
											(nPageNum[0]-1)*10, 
											1, 
											(CTRLPROC)&CPagePtzConfigFrameWork::OnTrackMove0);

		items[0].push_back(pScrollbar[0]);
	}
	
	CRect tmpRt;
	int i = 0;
	for(i=0; i<TabSzPtz[0].nCol; i++)
	{

		pTable00->GetTableRect(i,0,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic0[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, statictext[i]);
		pStatic0[i]->SetBkColor(VD_RGB(67,77,87));
		pStatic0[i]->SetTextAlign(VD_TA_XLEFT);
		items[0].push_back(pStatic0[i]);
	}
	
	for(i=TabSzPtz[0].nCol; i<TabSzPtz[0].nCol+nRealRow[0]; i++)
	{
		pTable00->GetTableRect(0,i-TabSzPtz[0].nCol+1,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		char szTmp[10]={0};
		sprintf(szTmp, "%d", i-TabSzPtz[0].nCol+1);
		pStatic0[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, szTmp);
		items[0].push_back(pStatic0[i]);
	}
	
	pStatic0[i] = CreateStatic(CRect(rtSubPage.left, rtSubPage.bottom+5, rtSubPage.left+200,rtSubPage.bottom+30), this, statictext[TabSzPtz[0].nCol]);
	items[0].push_back(pStatic0[i]);
	
	for(i=0; i<nRealRow[0]; i++)
	{
		pTable00->GetTableRect(1,i+1,&tmpRt);
		pCheckBox0[i] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
                        				rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+23), 
                        				this, styleEditable, (CTRLPROC)&CPagePtzConfigFrameWork::OnCheckBox0);

		//BIZ_DT_DBG("ptz 1");
		
		GetScrStrSetCurSelDo(
			pCheckBox0[i],
			GSR_PTZCONFIG_SERIAL_ENABLE, 
			EM_GSR_CTRL_CHK, 
			i
		);

		items[0].push_back(pCheckBox0[i]);
	}
				
	for(i=0; i<nRealRow[0]; i++)
	{
		pTable00->GetTableRect(2,i+1,&tmpRt);
		pEdit0[i] = CreateEdit(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
            				rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
            				this, 3, edit_KI_NUMERIC, (CTRLPROC)&CPagePtzConfigFrameWork::OnEdit0);
		pEdit0[i]->SetBkColor(VD_RGB(67,77,87));

		items[0].push_back(pEdit0[i]);
	}
	
	for(i=0; i<nRealRow[0]; i++)
	{
		pTable00->GetTableRect(3,i+1,&tmpRt);
		pCombo0[i] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
            				rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
            				this, NULL, NULL, (CTRLPROC)&CPagePtzConfigFrameWork::OnCombo0, 0);
		pCombo0[i]->SetBkColor(VD_RGB(67,77,87));

		GetScrStrInitComboxSelDo(
			(void*)pCombo0[i],
			GSR_PTZCONFIG_SERIAL_BPS, 
			EM_GSR_COMBLIST, 
			i-nRealRow[0]
		);
		//BIZ_DT_DBG("ptz 3");

		items[0].push_back(pCombo0[i]);
	}
	
	for(i=nRealRow[0]; i<nRealRow[0]*2; i++)
	{
		pTable00->GetTableRect(4,i-nRealRow[0]+1,&tmpRt);
		if(nPageNum[0]>1)
		{
			pCombo0[i] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
                					rtSubPage.left+tmpRt.right-2-SCROLLBAR,rtSubPage.top+tmpRt.bottom-1), 
                					this, NULL, NULL, (CTRLPROC)&CPagePtzConfigFrameWork::OnCombo0, 0);
		}
		else
		{
			pCombo0[i] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
                					rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
                					this, NULL, NULL, (CTRLPROC)&CPagePtzConfigFrameWork::OnCombo0, 0);
		}
		
		pCombo0[i]->SetBkColor(VD_RGB(67,77,87));

		GetScrStrInitComboxSelDo(
			(void*)pCombo0[i],
			GSR_PTZCONFIG_SERIAL_PROTOCOL, 
			EM_GSR_COMBLIST,
			i-nRealRow[0]*2
		);
		
		items[0].push_back(pCombo0[i]);
	}


	rtSubPage.left = m_Rect.left+27;
	rtSubPage.top = rtSubPage.bottom + 30;//m_Rect.top+255;
	rtSubPage.right = m_Rect.left+m_Rect.Width()-27;
	rtSubPage.bottom = rtSubPage.top + 30;

	//printf("$$$$$$$$$$ rtSubPage.bottom = %d \n",rtSubPage.bottom);
	
	pTable01 = CreateTableBox(&rtSubPage,this, 5,1,0);
	pTable01->SetColWidth(0,60);
	pTable01->SetColWidth(1,80);
	pTable01->SetColWidth(2,130);
	pTable01->SetColWidth(3,150);
	items[0].push_back(pTable01);


	for(i=0; i<2; i++)
	{
		pTable01->GetTableRect(i,0,&tmpRt);
		pCheckBox0[i+nRealRow[0]] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
                        				rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+23), 
                        				this);
		if(i==0)
		{			
			GetScrStrSetCurSelDo(
				(void*)pCheckBox0[i+nRealRow[0]],
				GSR_PTZCONFIG_SERIAL_ALLCHECK, 
				EM_GSR_CTRL_CHK, 
				0
			);
		}
		/*
		else
		{				
			GetScrStrSetCurSelDo(
				(void*)pCheckBox0[i+4],
				GSR_PTZCONFIG_SERIAL_ENABLE, 
				EM_GSR_CTRL_CHK, 
				ALL_SEL
			);
		}
		*/
		
		items[0].push_back(pCheckBox0[i+nRealRow[0]]);
	}
	
	for(i=0; i<2; i++)
	{
		pTable01->GetTableRect(i+3,0,&tmpRt);
		pCombo0[i+nRealRow[0]*2] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
                    				rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
                    				this, NULL, NULL, NULL, 0);
		pCombo0[i+nRealRow[0]*2]->SetBkColor(VD_RGB(67,77,87));
		
		switch(i)
		{
			case 0:
			{
				GetScrStrInitComboxSelDo(
					(void*)pCombo0[i+nRealRow[0]*2],
					GSR_PTZCONFIG_SERIAL_BPS, 
					EM_GSR_COMBLIST, 
					0//ALL_SEL
				);
			}
			break;
			case 1:
			{
				GetScrStrInitComboxSelDo(
					(void*)pCombo0[i+nRealRow[0]*2],
					GSR_PTZCONFIG_SERIAL_PROTOCOL, 
					EM_GSR_COMBLIST, 
					0//ALL_SEL
				);
			}break;
			default:
				break;
		}
		
		items[0].push_back(pCombo0[i+nRealRow[0]*2]);
	}
	
	ShowSubPage(0,TRUE);
	pButton[0]->Enable(FALSE);
}

void CPagePtzConfigFrameWork::OnPresetSetup()
{
	//printf("OnPresetSetup\n");
	
	/*
	this->m_pParent->GetParent()->Close();
	this->m_pParent->Close();
	this->Close();

	SPresetPara sPIns;

	memset(&sPIns, 0, sizeof(sPIns));
	memcpy(sPIns.nId, psGuiPtzPara->nIsPresetSet, 128);
	
	for(int i=0; i<nChMax; i++)
		m_pPagePtzPreset->OnSetPara(i,&sPIns);
	
	m_pPagePtzPreset->Open();
	*/
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < nRealRow[1]; i++)
	{
		if(pFocusButton == pButton1[i])
		{
			bFind = TRUE;
			break;
		}
		//printf("########pButton1[%d] = %x\n",i,pButton1[i]);
	}
	
	//printf("########pFocusButton = %x\n",pFocusButton);
	
	if(bFind)
	{
		m_pPagePresetSetup->SetChn(i + nCurPage[curID]*nRealRow[1]);
		printf(">>>>>>>>>\n");
		//m_pPagePresetSetup->OnSetPara(i, psGuiPtzPara[i].nIsPresetSet);
		m_pPagePresetSetup->Open();
		bChildClose = TRUE;
		this->Close(UDM_CLOSED);
		bChildClose = FALSE;
	}
}

void CPagePtzConfigFrameWork::OnCruiseLineSetup()
{
	//printf("OnCruiseLineSetup,nRealRow[1]=%d\n",nRealRow[1]);
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = nRealRow[1]; i < nRealRow[1]*2; i++)
	{
		//printf("########pButton1[%d] = 0x%08x\n",i,(unsigned int)pButton1[i]);
		if(pFocusButton == pButton1[i])
		{
			bFind = TRUE;
			break;
		}
	}
	//printf("########pFocusButton = 0x%08x,bFind=%d\n",pFocusButton,bFind);
	
	if(bFind)
	{
		//printf("OnCruiseLineSetup:chn=%d\n",i-nRealRow[1] + nCurPage[curID]*nRealRow[1]);
		//fflush(stdout);
		
		m_pPageCruiseLine->SetChn(i-nRealRow[1] + nCurPage[curID]*nRealRow[1]);
		
		//GetCruiseLine(i-nRealRow[1]);
		//m_pPageCruiseLine->OnSetPara(i-nRealRow[1], psGuiPtzPara[i-nRealRow[1]].sTourPath);
		
		//printf("OnCruiseLineSetup:step2\n");
		//fflush(stdout);
		
		m_pPageCruiseLine->Open();
		
		//printf("OnCruiseLineSetup:step3\n");
		//fflush(stdout);
		
        bChildClose = TRUE;
        this->Close(UDM_CLOSED);
        bChildClose = FALSE;
		
		//printf("OnCruiseLineSetup:step4\n");
		//fflush(stdout);
	}
}

void CPagePtzConfigFrameWork::OnTrackSetup()
{
	printf("OnTrackSetup\n");
	
    int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = nRealRow[1]*2; i < nRealRow[1]*3; i++)
	{
		if(pFocusButton == pButton1[i])
		{
			bFind = TRUE;
			break;
		}
	}
    
	if(bFind)
	{		
	    SetSystemLockStatus(1);//cw_lock
        m_pPagePtzTrack->ParentSendCh(i-nRealRow[1]*2 + nCurPage[curID]*nRealRow[1]);
        bChildClose = TRUE;
        this->Close(UDM_CLOSED);
        bChildClose = FALSE;
    	m_pPagePtzTrack->Open();
	    SetSystemLockStatus(0);
	}
}

void CPagePtzConfigFrameWork::InitPage1()
{
	char* statictext[] = {
		"&CfgPtn.Channel",
		"&CfgPtn.Preset",
		"&CfgPtn.Cruise",
		"&CfgPtn.Track",
		/*
		"1",
		"2",
		"3",
		"4",
		*/
	};
	
	CRect rtSubPage(m_Rect.left+27, m_Rect.top+50, 
	m_Rect.left+m_Rect.Width()-27,m_Rect.top+50+270);
	
	//printf("dddddddd  %d \n",m_Rect.top+m_Rect.Height()-170 - (m_Rect.top+43));
	pTable10 = CreateTableBox(&rtSubPage,this, TabSzPtz[1].nCol,TabSzPtz[1].nRow+1,0);
	pTable10->SetColWidth(0,60);
	pTable10->SetColWidth(1,176);
	pTable10->SetColWidth(2,176);
	items[1].push_back(pTable10);
#if 1
	if(nPageNum[1]>1)
	{
		pScrollbar[1] = CreateScrollBar(CRect(m_Rect.Width()-27-SCROLLBAR,
											m_Rect.top+80,
											m_Rect.Width()-27,
											m_Rect.top+50+30+240), 
											this,
											scrollbarY,
											0,
											(nPageNum[1]-1)*10, 
											1, 
											(CTRLPROC)&CPagePtzConfigFrameWork::OnTrackMove0);

		items[1].push_back(pScrollbar[1]);
	}
#endif
	CRect tmpRt;
	int i = 0;
	for(i=0; i<TabSzPtz[1].nCol; i++)
	{

		pTable10->GetTableRect(i,0,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic1[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, statictext[i]);
		pStatic1[i]->SetBkColor(VD_RGB(67,77,87));
		pStatic1[i]->SetTextAlign(VD_TA_XLEFT);
		items[1].push_back(pStatic1[i]);
	}
	
	for(i=TabSzPtz[1].nCol; i<TabSzPtz[1].nCol+nRealRow[1]; i++)
	{
		pTable10->GetTableRect(0,i-TabSzPtz[1].nCol+1,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		char szTmp[10] = {0};
		sprintf(szTmp, "%d", i-TabSzPtz[1].nCol+1);
		pStatic1[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, szTmp);
		items[1].push_back(pStatic1[i]);
	}
	
	for(i=0; i<nRealRow[1]; i++)
	{
		pTable10->GetTableRect(1,i+1,&tmpRt);
		pButton1[i] = CreateButton(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
					   rtSubPage.left+tmpRt.left+94,rtSubPage.top+tmpRt.top+25), this, "&CfgPtn.Setup", (CTRLPROC)&CPagePtzConfigFrameWork::OnPresetSetup, NULL, buttonNormalBmp);
		pButton1[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);
		
		items[1].push_back(pButton1[i]);
		
		//printf("CPagePtzConfigFrameWork::InitPage1-pButton1[%d]=0x%08x\n",i,(unsigned int)pButton1[i]);
	}

	for(i=nRealRow[1]; i<nRealRow[1]*2; i++)
	{
		pTable10->GetTableRect(2,i-nRealRow[1]+1,&tmpRt);
		pButton1[i] = CreateButton(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
					   rtSubPage.left+tmpRt.left+94,rtSubPage.top+tmpRt.top+25), this, "&CfgPtn.Setup", (CTRLPROC)&CPagePtzConfigFrameWork::OnCruiseLineSetup, NULL, buttonNormalBmp);
		pButton1[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);
		
		items[1].push_back(pButton1[i]);
		
		//printf("CPagePtzConfigFrameWork::InitPage1-pButton1[%d]=0x%08x\n",i,(unsigned int)pButton1[i]);
	}
	
	for(i=nRealRow[1]*2; i<nRealRow[1]*3; i++)
	{
		pTable10->GetTableRect(3,i-nRealRow[1]*2+1,&tmpRt);
		pButton1[i] = CreateButton(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
					   rtSubPage.left+tmpRt.left+94,rtSubPage.top+tmpRt.top+25), this, "&CfgPtn.Setup", (CTRLPROC)&CPagePtzConfigFrameWork::OnTrackSetup, NULL, buttonNormalBmp);
		pButton1[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);
		
		items[1].push_back(pButton1[i]);
		
		//printf("CPagePtzConfigFrameWork::InitPage1-pButton1[%d]=0x%08x\n",i,(unsigned int)pButton1[i]);
	}
	
	ShowSubPage(1,FALSE);
	
	//for(i=0; i<nRealRow[1]*3; i++)
	//{
	//	printf("CPagePtzConfigFrameWork::check-pButton1[%d]=0x%08x\n",i,(unsigned int)pButton1[i]);
	//}
}

void CPagePtzConfigFrameWork::SwitchPage( int subID)
{
	if (subID<0 || subID>=PTZCFG_SUBPAGES )
	{
		return;
	}
	
	if (subID == curID)
	{
		return;
	}
	
	LoadPara();
	
	//隐藏当前子页面，显示新的子页面
	ShowSubPage(curID, FALSE);
	pButton[curID]->Enable(TRUE);
	curID = subID;
	pButton[curID]->Enable(FALSE);
	ShowSubPage(curID, TRUE);
}

void CPagePtzConfigFrameWork::AdjHide()
{
	if ((nPageNum[curID]-1 == nCurPage[curID]) && (nPageNum[curID] > 1))
	{
		VD_BOOL bHide = FALSE;
		
		int firstHide = nRealRow[curID]-nHideRow[curID];
		int lastHide = nRealRow[curID];
		int nRow     = nRealRow[curID];
		int nCol     = TabSzPtz[curID].nCol;

		switch(curID)
		{
			case 0:
			{
				for(int i=firstHide; i<lastHide; i++)
				{
					pStatic0[i+nCol]->Show(bHide,TRUE);			
					pCheckBox0[i]->Show(bHide,TRUE);
					pEdit0[i]->Show(bHide,TRUE);
					pCombo0[i]->Show(bHide,TRUE);
					pCombo0[i+nRow]->Show(bHide,TRUE);
				}
			} break;
			case 1:
			{			
				for(int i=firstHide; i<lastHide; i++)
				{
					pStatic1[i+nCol]->Show(bHide,TRUE);
					pButton1[i]->Show(bHide,TRUE);
					pButton1[i+nRow]->Show(bHide,TRUE);
					pButton1[i+nRow*2]->Show(bHide,TRUE);
				}
			} break;
		}		
	}
}

void CPagePtzConfigFrameWork::ShowSubPage( int subID, BOOL bShow )
{
	if (subID<0 || subID>=PTZCFG_SUBPAGES )
	{
		return;
	}	

	int count = items[subID].size();

    if (1 == subID && TRUE == bShow)
    {
        pButton[PTZCFG_SUBPAGES]->Show(FALSE);
    }
    else if ( 0 == subID && TRUE == bShow) 
    {
        pButton[PTZCFG_SUBPAGES]->Show(TRUE);
    }
	
	//printf("ShowSubPage count= %d  \n",count);
	for (int i=0; i<count; i++)
	{
		if(items[subID][i] )
		{
			items[subID][i]->Show(bShow);
		}
	}

	AdjHide();
}

void CPagePtzConfigFrameWork::LoadPara()
{
    //if (FALSE == m_bOpenFlag)
    //{
		//csp modify 20130303
		//pCombo0[nRealRow[0]*2]->SetCurSel(5);
		pCombo0[nRealRow[0]*2]->SetCurSel(4);
		pCombo0[1+nRealRow[0]*2]->SetCurSel(3);
		pCheckBox0[nRealRow[0]]->SetValue(0);
		pCheckBox0[nRealRow[0]+1]->SetValue(0);
    	
    	for(int i=0; i<nChMax; i++)
    	{
    		bizData_GetUartPara(i, &pTabPara[i]);
            sprintf(pcPtzAddr[i], "%u", pTabPara[i].nAddress);
    	}
    	
    	for(int i=0; i<nRealRow[0]; i++)
    	{
    		int nCurRow = nCurPage[0]*nRealRow[0]+i;
            //char s[10] = {0};

            //sprintf(s,"%d", pcPtzAddr[nCurRow]);
    		
    		pCheckBox0[i]->SetValue(pTabPara[nCurRow].bEnable);
    		pEdit0[i]->SetText(pcPtzAddr[nCurRow]);
    		pCombo0[i]->SetCurSel(pTabPara[nCurRow].nBaudRate);
    		pCombo0[i+nRealRow[0]]->SetCurSel(pTabPara[nCurRow].nProtocol);
    	}
		
        for(int i=0; i<nChMax; i++)
        {
            bizData_GetPtzAdvancedPara(i, &psGuiPtzPara[i]);
        }
        //m_bOpenFlag = FALSE;
    //}
}

VD_BOOL CPagePtzConfigFrameWork::UpdateData( UDM mode )
{
	if(UDM_OPEN == mode)
	{
		//int i;
		//for(i=0; i<nRealRow[1]*3; i++)
		//{
		//	printf("CPagePtzConfigFrameWork::UpdateData1-pButton1[%d]=0x%08x\n",i,(unsigned int)pButton1[i]);
		//}
		
		LoadPara();
        bChildClose = FALSE;
		
		//for(i=0; i<nRealRow[1]*3; i++)
		//{
		//	printf("CPagePtzConfigFrameWork::UpdateData2-pButton1[%d]=0x%08x\n",i,(unsigned int)pButton1[i]);
		//}
	}
	else if(UDM_CLOSED == mode) 
	{
        ((CPageConfigFrameWork *)(this->GetParent()))->ReflushItemName();
    }
	
	return TRUE;
}

void CPagePtzConfigFrameWork::SaveCruiseLine(uchar nCh, SGuiTourPath* pPIns)
{
	memcpy(psGuiPtzPara[nCh].sTourPath, pPIns, sizeof(SGuiTourPath)*TOURPATH_MAX);
}

void CPagePtzConfigFrameWork::SavePresetSet(uchar nCh, uchar * pCChar, uchar npPInt)
{
    memcpy(psGuiPtzPara[nCh].nIsPresetSet, pCChar, npPInt);

    for (int i = 0; i <= npPInt; i++)
    {
        if (1 == pCChar[i])
        {
            printf("SavePresetSet Chn:ChBox: %d:%03d:1 OK!\n", nCh, i);
        }
    }
}
void CPagePtzConfigFrameWork::GetCruiseLine(uchar nCh)
{	
	m_pPageCruiseLine->GetTourLine(psGuiPtzPara[nCh].sTourPath);
}

void CPagePtzConfigFrameWork::SetInfo(char* szInfo)
{
	pIconInfo->Show(TRUE);
	
	pInfoBar->SetText(szInfo);

}

void CPagePtzConfigFrameWork::ClearInfo()
{
	pIconInfo->Show(FALSE);
	
	pInfoBar->SetText("");

}

void CPagePtzConfigFrameWork::ChildClose()
{
    bChildClose = TRUE;
}

VD_BOOL CPagePtzConfigFrameWork::Close(UDM mode)
{
	if (!bChildClose)
	{
		this->m_pParent->Show(TRUE);//cw_test
		//this->m_pParent->Open();
	}
	return CPage::Close();
}

VD_BOOL CPagePtzConfigFrameWork::MsgProc( uint msg, uint wpa, uint lpa )
{
	
     u8 lock_flag = 0;  //cw_shutdown
     GetSystemLockStatus(&lock_flag);
     if(lock_flag)
     {
     	return FALSE;
     }
	int px,py;
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
			int px = VD_HIWORD(lpa);
			int py = VD_LOWORD(lpa);
			CItem* pItem = GetItemAt(px, py);
			if(pItem == NULL)
			{
				if(GetMaxChnNum()>TabSzPtz[0].nRow)
				{
					if (curID == 0)
					{
						pScrollbar[0]->MsgProc(msg, wpa, lpa);
						break;
					}
				}
				if(GetMaxChnNum()>TabSzPtz[1].nRow)
				{
					if (curID == 1)
					{
						pScrollbar[1]->MsgProc(msg, wpa, lpa);
						break;
					}
				}
			}
			break;
		}
        break;
	case XM_MOUSEMOVE:
		{
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);
			static CItem* last = NULL;
			CItem* temp = GetItemAt(px, py);

			if(temp != last)
			{
                for(int i=0; i<nRealRow[0]; i++)
                {
                    if (temp == pEdit0[i])
                    {
                        char s[32] = {0};
                        sprintf(s, "%s: 0-255",
                            GetParsedString("&CfgPtn.AddrRange"));
                        SetInfo(s);
                        break;
                    }
                }
                
				if(temp == NULL)
				{
					ClearInfo();
				}

				last = temp;
			}
		}
		break;
    case XM_RBUTTONDOWN:
    case XM_RBUTTONDBLCLK:
        {            
			/*           //cw_test
            //this->GetParent()->GetParent()->Open();
            this->m_pParent->Open();
            //this->Close(UDM_CLOSED);
            return CPage::Close();
            */
        }
	default:
		//return 0;
		break;
	}

	return CPage::MsgProc(msg, wpa, lpa);

}

void CPagePtzConfigFrameWork::SetOpenFlag(BOOL bFlag)
{
    m_bOpenFlag = bFlag;
}

BOOL CPagePtzConfigFrameWork::Digital(const char * s)
{
    while (*s)
    {
        if (!isdigit(*s++))
        {
            return 0;
        }
    }
    
    return 1;
}

void CPagePtzConfigFrameWork::MessgeBox(int index)
{
    //pEdit0[index]->SetText("");
    
    char s[32] = {0};
    sprintf(s, "%d-%s %s: 0-255", index+1,
        GetParsedString("&CfgPtn.Channel"),
        GetParsedString("&CfgPtn.AddrRange"));
    MessageBox(s, "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
    return;
}

