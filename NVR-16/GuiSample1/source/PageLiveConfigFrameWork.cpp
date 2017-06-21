#include "GUI/Pages/PageLiveConfigFrameWork.h"
#include "GUI/Pages/PageMaskSet.h"
#include "GUI/Pages/PageColorSetup.h"
#include "GUI/Pages/PageDesktop.h"
#include "GUI/Pages/PageConfigFrameWork.h"
#include "GUI/Pages/PageMessageBox.h"
#include "sg_platform.h"


static	MAINFRAME_ITEM	items[LIVECONFIG_SUBPAGES];

enum{//与下面的数组一一对应
	LIVECFG_BUTTON_LEFT,
	LIVECFG_BUTTON_RIGHT,
	LIVECFG_BUTTON_NUM,
};

static int GetMaxChnNum()
{
	return GetVideoMainNum();
}

static int GetRows()
{

	return GetMaxChnNum()>LIVECFG_ROWS?LIVECFG_ROWS:GetMaxChnNum();
}

static int GetMaskRows()
{
	return GetMaxChnNum()>MASK_ROWS?MASK_ROWS:GetMaxChnNum();
}

static int GetVideoSrcRows()
{
	return GetMaxChnNum()>VIDEOSRC_ROWS?VIDEOSRC_ROWS:GetMaxChnNum();
}

char* liveShortcutBmpName[LIVECFG_BUTTON_NUM][2] = {    
	{DATA_DIR"/temp/arrow_left.bmp",	DATA_DIR"/temp/arrow_left_f.bmp"},
	{DATA_DIR"/temp/search_play.bmp",	DATA_DIR"/temp/search_play_f.bmp"},
};

static VD_BITMAP* pBmpButtonNormal[LIVECFG_BUTTON_NUM];
static VD_BITMAP* pBmpButtonSelect[LIVECFG_BUTTON_NUM];


static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static VD_BITMAP * pBmp_tab_normal;
static VD_BITMAP * pBmp_tab_push;
static VD_BITMAP * pBmp_tab_select;

static VD_BITMAP * pBmp_infoicon;
static char tmp2[20] = {0};

void* p_gButtonDbg[2] = {0};

int nChnWidth = 720;

CPageLiveConfigFrameWork::CPageLiveConfigFrameWork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
, curID(0),chnNum(4)
{
	GetProductNumber(tmp2);
	
    OutputIndex = GetOutput();
	//SetRect(CRect(0, 0, 700, 400/*370*/), FALSE);
	SetMargin(3, m_TitleHeight, 4, m_TitleHeight);
	
	//pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/button_normal.bmp");
	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	
	//pBmp_button_push = VD_LoadBitmap(DATA_DIR"/button_push.bmp");
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	
	//pBmp_button_select = VD_LoadBitmap(DATA_DIR"/button_select.bmp");
	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	
	//第一个页面的子页面控制按钮  录像设置，手动录像，录像布防
	pBmp_tab_normal = VD_LoadBitmap(DATA_DIR"/temp/tab.bmp");
	pBmp_tab_push = VD_LoadBitmap(DATA_DIR"/temp/tab_check.bmp");
	pBmp_tab_select = VD_LoadBitmap(DATA_DIR"/temp/tab_f.bmp");
	char* szSubPgName[LIVECONFIG_BTNNUM] = {
		"&CfgPtn.Live",
		"&CfgPtn.MainMonnitor",
		"&CfgPtn.Spot",
		"&CfgPtn.Mask",
		"&CfgPtn.VideoSrc",
		"&CfgPtn.Default",
		"&CfgPtn.apply",
		"&CfgPtn.Exit",
	};
	int szSubPgNameLen[LIVECONFIG_BTNNUM] = {
		TEXT_WIDTH*3,//"&CfgPtn.Live",
		TEXT_WIDTH*4,//"&CfgPtn.MainMonnitor",	
		TEXT_WIDTH*2,//"&CfgPtn.Spot",
		TEXT_WIDTH*3,//"&CfgPtn.Mask",
		TEXT_WIDTH*5,//"&CfgPtn.VideoSrc",
		TEXT_WIDTH*3,//"&CfgPtn.Default",
		TEXT_WIDTH*2,//"&CfgPtn.apply",
		TEXT_WIDTH*2,//"&CfgPtn.Exit",
	};
	
	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);
	
	memset(pStatic0,0,sizeof(pStatic0));
	memset(pEdit0,0,sizeof(pEdit0));
	memset(pCheckBox0,0,sizeof(pCheckBox0));
	memset(pButton0,0,sizeof(pButton0));
	
	memset(pStatic3,0,sizeof(pStatic3));
	memset(pButton3,0,sizeof(pButton3));
	
	CTableBox* pTabFrame = CreateTableBox(CRect(17, 40, m_Rect.Width()-17, 40+290),this,1,1);
	//pTabFrame->SetFrameColor(VD_RGB(56,108,148));
	pTabFrame->SetFrameColor(VD_RGB(8,66,189));
	
	CRect rtSub(25, 10, m_Rect.Width()-20, 10+pBmp_tab_normal->height);
	int i;
	for(i=0; i<LIVECONFIG_SUBPAGES; i++)
	{
		if(i==2 || i==1)
		{
			
		}
		else
		{
			int btWidth = szSubPgNameLen[i]+10;
			rtSub.right = rtSub.left + btWidth;
			pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageLiveConfigFrameWork::OnClickSubPage, NULL, buttonNormalBmp,1);
			pButton[i]->SetBitmap(pBmp_tab_normal, pBmp_tab_select, pBmp_tab_push, pBmp_tab_push);
			rtSub.left = rtSub.right;
		}
	}
	
	CRect rtSub1(295, m_Rect.Height()-80, m_Rect.Width()-20, m_Rect.Height()-80+pBmp_button_normal->height);
	
	pInfoBar = CreateStatic(CRect(40, 
								rtSub1.top,
								380,
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
	for(i=LIVECONFIG_SUBPAGES; i<LIVECONFIG_BTNNUM; i++)
	{
		rtSub1.left -= szSubPgNameLen[i]+10;
	}
	rtSub1.left -= 12*2;
	
	for(i=LIVECONFIG_SUBPAGES; i<LIVECONFIG_BTNNUM; i++)
	{
		int btWidth = szSubPgNameLen[i]+10;
		rtSub1.right = rtSub1.left + btWidth;
		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageLiveConfigFrameWork::OnClickSubPage, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
		rtSub1.left = rtSub1.right+12;
	}
	
	//CRect rtSub2(13, 35, m_Rect.Width()-18, 300);
	//pTable = CreateTableBox(&rtSub2,this, 1,1,0);
	
	m_pMaskSet = new CPageMaskSet(NULL);
	
	//现场配置
	InitPage0();
	//printf("InitPage0  finish  line:%d\n",__LINE__);
	
	InitPage10();
	//printf("InitPage10  finish  line:%d\n",__LINE__);
	
	InitPage11();
	//printf("InitPage11  finish  line:%d\n",__LINE__);
	
	InitPage20();
	//printf("InitPage20  finish  line:%d\n",__LINE__);
	
	InitPage21();
	//printf("InitPage21  finish  line:%d\n",__LINE__);
	
	//遮挡
	InitPage3();
	//printf("InitPage3  finish  line:%d\n",__LINE__);
	
#if 1//csp modify 20130407
	if(0 == strcasecmp(tmp2, "R3104HD"))
	{
		InitPage4();
		//printf("InitPage4  finish  line:%d\n",__LINE__);
	}
	else
	{
		//csp modify 20130407
		memset(pStatic4,0,sizeof(pStatic4));
		memset(pComboBox4,0,sizeof(pComboBox4));
		
		pButton[LIVECONFIG_BT_VEDIO]->Show(0);
	}
#else
	pButton[LIVECONFIG_BT_VEDIO]->Show(0);
#endif
	
	CRect rectScreen;
	m_pDevGraphics = CDevGraphics::instance(m_screen);
	//printf("CPageDesktop::CPageDesktop..10\n");
	m_pDevGraphics->GetRect(&rectScreen);
	
	CRect rtFloat;
	rtFloat.left = rectScreen.Width()*3/5;
	rtFloat.right = rtFloat.left + 170;
	rtFloat.top = rectScreen.bottom/7+2;
    rtFloat.bottom = rtFloat.top + 365;
	
    #if 0
    if (0 == OutputIndex)
    {
        //CVBS
	    rtFloat.bottom = rectScreen.bottom*6/7-45;
    }
    else if (1 == OutputIndex)
    {
        //VGA=800x600
        rtFloat.bottom = rectScreen.bottom*6/7-65;
    }
    else if (2 == OutputIndex)
    {
        //VGA=1024x768
        rtFloat.bottom = rectScreen.bottom*6/7-185;
    }
    #endif
	
	m_pPageColorSetup = new CPageColorSetup(rtFloat, NULL,NULL, this);
	
	pTmpEdit = CreateEdit(CRect(0,0,0,0),this,32,0,NULL);
	pTmpCheck = CreateCheckBox(CRect(0,0,0,0),this);
	pTmpEdit->Show(FALSE);
	pTmpCheck->Show(FALSE);
}

void CPageLiveConfigFrameWork::OnClickSubPage()
{
	//printf("OnClickSubPage\n");
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < LIVECONFIG_SUBPAGES; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		//printf("find the focus button \n");
		SwitchPage(i,items[i].cursubindex);	
		
		if(i == 0)
		{
			//live
			AdjustLiveRows();
		}	
		else if(i == 3)
		{
			//mask
			AdjustMaskRows();
		}
	}
	else
	{
		//for(; i < LIVECONFIG_BTNNUM; i++)
		{
			if(pFocusButton == pButton[LIVECONFIG_BT_APPLY])
			{
				if(curID == LIVECONFIG_BT_LIVE)
				{
					printf("yaogang LIVECONFIG_BT_LIVE\n");				
					WriteLogs();

					//现场配置参数保存
					SSG_MSG_TYPE msg;
					memset(&msg, 0, sizeof(msg));
					msg.type = EM_DVR_PARAM_SAVE;
					msg.chn = 0;
					strcpy(msg.note, GetParsedString("&CfgPtn.Live"));
					strcat(msg.note, GetParsedString("&CfgPtn.ParamSave"));
					upload_sg(&msg);
					
					int i = 0;
					int bAll = pCheckBox0[LIVECFG_ROWS+1]->GetValue();
					
					if(bAll == TRUE)
					{
						int allDisplay = pCheckBox0[LIVECFG_ROWS+2]->GetValue();
			
						for(i=0; i<GetRows(); i++)
						{
							pCheckBox0[i]->SetValue(allDisplay);
						}

						for(i=0; i<GetMaxChnNum(); i++)
						{
							bCheckValue[i] = allDisplay;
						}
					}
					
					//printf("SaveScrStr2Cfg pCheckBox0 \n");
					//显示录像状态
					
					SaveScrStr2Cfg(pCheckBox0[LIVECFG_ROWS],
											GSR_CONFIG_LIVE_LIVEREC,
											EM_GSR_CTRL_CHK,
											0
											);
					
					int ret = 0;//int ret;//csp modify
					for(i=0; i<GetMaxChnNum(); i++)
					{
						//printf("chn %d, name = %s - 111\n",i,szChName[i]);
						pTmpEdit->SetText(szChName[i]);
						//printf("chn %d, name = %s - 222\n",i,szChName[i]);
						
						ret = SaveScrStr2Cfg(pTmpEdit,
									GSR_CONFIG_LIVE_CHNNAME,
									EM_GSR_CTRL_EDIT,
									i
								    );
								    
						//printf("chn %d, GetMaxChnNum = %d, show = %d, ret = %d\n",i,GetMaxChnNum(),bCheckValue[i],ret);
						pTmpCheck->SetValue(bCheckValue[i]);
						
						SaveScrStr2Cfg(pTmpCheck,
									GSR_CONFIG_LIVE_DISPLAY,
									EM_GSR_CTRL_CHK,
									i
									);
									
					}
					
					if(ret == 0)
					{
						SetInfo("&CfgPtn.SaveConfigSuccess");
					}
					else
					{
						SetInfo("&CfgPtn.SaveConfigFail");
					}
				}
				else if(curID == LIVECONFIG_BT_MAINMONITOR)
				{
					
				}
				else if(curID == LIVECONFIG_BT_SPOT)
				{
					
				}
				else if(curID == LIVECONFIG_BT_MASK)
				{
					//遮挡参数保存
					SSG_MSG_TYPE msg;
					memset(&msg, 0, sizeof(msg));
					msg.type = EM_DVR_PARAM_SAVE;
					msg.chn = 0;
					strcpy(msg.note, GetParsedString("&CfgPtn.Mask"));
					strcat(msg.note, GetParsedString("&CfgPtn.ParamSave"));
					upload_sg(&msg);

					int ret = 0;
					int nScreanWidth, nScreanHeight;
					GetVgaResolution(&nScreanWidth, &nScreanHeight);
					int Realheight=(GetVideoFormat()?480:576);//cw_mask
					for(int i=0; i<GetMaxChnNum(); i++)
					{
						OsdRect* osdrt = m_pMaskSet->GetMaskRect(i);
						
						if(osdrt->bModify)
						{
							SRect rectOsd[osdrt->realNum];//zlb20111117  去掉部分malloc
							for(int j=0; j<osdrt->realNum; j++)
							{
								rectOsd[j].x = osdrt->osdRect[j].left * 704 / nScreanWidth;
								rectOsd[j].y = osdrt->osdRect[j].top * Realheight / nScreanHeight;//cw_mask
								s32 r = osdrt->osdRect[j].right * 704 / nScreanWidth;
								s32 b = osdrt->osdRect[j].bottom * Realheight / nScreanHeight;//cw_mask
								rectOsd[j].w = r - rectOsd[j].x;
								rectOsd[j].h = b - rectOsd[j].y;
								if(rectOsd[j].w < 0)
								{
									rectOsd[j].w = 0;
								}
								if(rectOsd[j].h < 0)
								{
									rectOsd[j].h = 0;
								}
								rectOsd[j].x = rectOsd[j].x;
								rectOsd[j].y = rectOsd[j].y;
								rectOsd[j].w = rectOsd[j].w;
								rectOsd[j].h = rectOsd[j].h;
								//printf("chn%d rect%d:(%d,%d,%d,%d)\n",i,j,rectOsd[j].x,rectOsd[j].y,rectOsd[j].w,rectOsd[j].h);
							}
							//printf("CH = %d,realNum = %d \n",i,osdrt->realNum);
							ret = SetRectOsd(i, osdrt->realNum, rectOsd);	
							osdrt->bModify = FALSE;
							
							//free(rectOsd);//zlb20111117  去掉部分malloc
						}

					}

					if(ret == 0)
					{
						SetInfo("&CfgPtn.SaveConfigSuccess");
					}
					else
					{
						SetInfo("&CfgPtn.SaveConfigFail");
					}
					
				}
				#if 1//csp modify 20130407
				else if(curID == LIVECONFIG_BT_VEDIO)
				{
					int ret = memcmp(m_nVedioSrc, m_nVedioSrcLast, GetMaxChnNum());
					if(0 != ret)
					{
						if(UDM_OK != MessageBox("&CfgPtn.RebootOrNot", "&CfgPtn.WARNING", MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2))
						{
							return;
						}
						ret = SetVideoSrcIndex(m_nVedioSrc);
						if(ret == 0)
						{
							SetInfo("&CfgPtn.SaveConfigSuccess");
						}
						else
						{
							SetInfo("&CfgPtn.SaveConfigFail");
						}
						printf("重启......\n");
						SetSystemLockStatus(1);
						BizSysComplexExit(EM_BIZSYSEXIT_RESTART);
					}
				}
				#endif
				
				//break;
			}
			else if(pFocusButton == pButton[LIVECONFIG_BT_DEFAULT])
			{
				switch(curID)
				{		
					case 0://live
					{
						int ret;
						char pChName[32];
						for(int nCh=0; nCh<GetMaxChnNum(); nCh++)
						{
							ret = GetChnNameDefault(nCh,szChName[nCh],32);
							//pEdit0[nCh]->SetText(pChName);

							int value = GetDisplayNameCheckDefault(nCh);
							bCheckValue[nCh] = value;
							//pCheckBox0[nCh]->SetValue(value);
						}

						for(int i=0; i<GetRows(); i++)
						{
							int nCh = m_page0*LIVECFG_ROWS+i;

							pEdit0[i]->SetText(szChName[nCh]);
							pCheckBox0[i]->SetValue(bCheckValue[nCh]);
							
						}

						int value = GetLiveRecStateCheckDefault();
						pCheckBox0[LIVECFG_ROWS]->SetValue(value);

						if(ret == 0)
						{		
							SetInfo("&CfgPtn.LoadDefaultSuccess");
						}
						else
						{
							SetInfo("&CfgPtn.LoadDefaultFail");
						}

					}break;
					case 3:
					{
						for(int i=0; i<GetMaxChnNum(); i++)
						{
							m_pMaskSet->SetMaskRect(i);
							m_pMaskSet->SetMaskNum(0);
						}
						SetInfo("&CfgPtn.LoadDefaultSuccess");
					}
					break;
					#if 1//csp modify 20130407
					case LIVECONFIG_BT_VEDIO:
					{
						GetVideoSrcIndexDefault(m_nVedioSrc);
						int rows = GetVideoSrcRows();
						for(int row=0; row<rows; row++)
						{
							int nCh = m_page4*VIDEOSRC_ROWS+row;
							if(nCh>=GetMaxChnNum())	break;
							pComboBox4[row]->SetCurSel((int)m_nVedioSrc[nCh]);
						}
						SetInfo("&CfgPtn.LoadDefaultSuccess");
					}
					break;
					#endif
					default:
						break;
				}
			}
			else if(pFocusButton == pButton[LIVECONFIG_BT_EXIT])
			{
				this->Close();
				if((CPage*)(this->GetParentPage()))
				{
					((CPage*)(this->GetParentPage()))->Show(TRUE, TRUE);
				}
			}
		}
	}
}

CPageLiveConfigFrameWork::~CPageLiveConfigFrameWork()
{
	
}

VD_PCSTR CPageLiveConfigFrameWork::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

void CPageLiveConfigFrameWork::OnEditChange0()
{
	int i = 0;
	BOOL bFind = FALSE;
	CEdit* pFocusEdit = (CEdit *)GetFocusItem();
	for(i = 0; i < LIVECFG_ROWS; i++)
	{
		if(pFocusEdit == pEdit0[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	//printf("OnEditChange0 \n");
	
	if(bFind)
	{
		int ch = m_page0*LIVECFG_ROWS+i;

		//bCheckValue[ch] = pFocusCheck->GetValue();
		pFocusEdit->GetText(szChName[ch],32);
	}
}

void CPageLiveConfigFrameWork::OnSetup0()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < LIVECFG_ROWS+1; i++)
	{
		if(pFocusButton == pButton0[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		//printf("i = %d\n",i);
		if(i == LIVECFG_ROWS)
		{
			//all
			m_pPageColorSetup->SetColorChn(GetMaxChnNum());
		}
		else
		{
			m_pPageColorSetup->SetColorChn(m_page0*LIVECFG_ROWS+i);

			//printf("m_page0*LIVECONFIG_LIVECFG_ROWS+i = %d \n",m_page0*LIVECONFIG_LIVECFG_ROWS+i);
		}
		
		//this->Close();
		m_pPageColorSetup->Open();
	}
}

void CPageLiveConfigFrameWork::OnCheckAll0()
{
	int value = pCheckBox0[7]->GetValue();
	
	//csp modify 20121130
	if(0 == strcasecmp(tmp2, "R3104HD"))
	{
		pButton0[6]->Enable(FALSE);
	}
	else
	{
		pButton0[6]->Enable(value);

		if(value == 1)
		{
			pButton0[6]->SetTextColor(VD_RGB(255,255,255));
		}
		else
		{
			pButton0[6]->SetTextColor(VD_RGB(67,77,87));
		}
	}
}

void CPageLiveConfigFrameWork::OnCheck0()
{
	int i = 0;
	BOOL bFind = FALSE;
	CCheckBox* pFocusCheck = (CCheckBox *)GetFocusItem();
	for(i = 0; i < LIVECFG_ROWS; i++)
	{
		if(pFocusCheck == pCheckBox0[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		int ch = m_page0*LIVECFG_ROWS+i;
		
		bCheckValue[ch] = pFocusCheck->GetValue();
	}
}

void CPageLiveConfigFrameWork::OnTrackMove0()
{
	int pos = pScroll0->GetPos();
    if (m_page0==pos) {
        return;
    }
    m_page0 = pos;

	//printf("m_page0 = %d \n",m_page0);

	for(int i=0; i<LIVECFG_ROWS; i++)
	{
		
		char szID[16] = {0};
		int nID = m_page0*LIVECFG_ROWS+1+i;
		sprintf(szID,"%d",nID);
		pStatic0[i+4]->SetText(szID);

		pEdit0[i]->SetText(szChName[nID-1]);
		pCheckBox0[i]->SetValue(bCheckValue[nID-1]);
		
		if(nID >= GetMaxChnNum())
		{
			break;
		}
		
		/*
		pEnable[i]->SetValue(psGuiPtzPara.nIsPresetSet[nID]);

		char szName[32] = {0};
		sprintf(szName,"preset%d",nID);
		pName[i]->SetText(szName);
		*/
	}

	AdjustLiveRows();
}

extern int GetChnNameMax( void );

void CPageLiveConfigFrameWork::InitPage0()
{
	m_page0 = 0;
	m_maxPage0 = 1;
	
	int maxLen = 32;
	bCheckValue = (int*)malloc(sizeof(int)*GetMaxChnNum());
	szChName = (char**)malloc(sizeof(char*)*GetMaxChnNum());
	for(int i=0; i<GetMaxChnNum(); i++)
	{
		szChName[i] = (char*)malloc(maxLen);
		memset(szChName[i],0,maxLen);
	}
	
	char* statictext[6] = {
		"&CfgPtn.Channel",
		"&CfgPtn.ChnName",
		"&CfgPtn.ShowName",
		"&CfgPtn.Color",
		"&CfgPtn.All",
		"&CfgPtn.RecordingStatus"	
	};
	
	CRect rtSubPage(m_Rect.left+27, m_Rect.top+50, 
	m_Rect.left+m_Rect.Width()-27,m_Rect.top+50+210);
	
	//printf("dddddddd %d\n",m_Rect.top+m_Rect.Height()-170 - (m_Rect.top+43));
	pTable00 = CreateTableBox(&rtSubPage,this, 4,7,0);
	pTable00->SetColWidth(0,60);
	pTable00->SetColWidth(1,260);
	pTable00->SetColWidth(2,130);
	items[0].SubItems[0].push_back(pTable00);

	if(GetMaxChnNum()>LIVECFG_ROWS)
	{
		m_maxPage0 = GetMaxChnNum()%LIVECFG_ROWS?GetMaxChnNum()/LIVECFG_ROWS+1:GetMaxChnNum()/LIVECFG_ROWS;

		pScroll0 = CreateScrollBar(CRect(m_Rect.Width()-27-25,
											m_Rect.top+80,
											m_Rect.Width()-27,
											m_Rect.top+50+210), 
											this,
											scrollbarY,
											0,
											(m_maxPage0-1)*10, 
											1, 
											(CTRLPROC)&CPageLiveConfigFrameWork::OnTrackMove0);

		items[0].SubItems[0].push_back(pScroll0);
	}

	int rows = GetRows();
	CRect tmpRt;
	int i = 0;
	for(i=0; i<LIVECFG_COLS; i++)
	{
		pTable00->GetTableRect(i,0,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic0[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, statictext[i]);
		pStatic0[i]->SetBkColor(VD_RGB(67,77,87));
		pStatic0[i]->SetTextAlign(VD_TA_XLEFT);
		
		//csp modify 20121130
		if(0 == strcasecmp(tmp2, "R3104HD"))
		{
			if(i == 3)
			{
				pStatic0[i]->Enable(FALSE);
			}
		}
		
		items[0].SubItems[0].push_back(pStatic0[i]);
	}
	
	for(i=LIVECFG_COLS; i<LIVECFG_COLS+rows; i++)
	{
		pTable00->GetTableRect(0,i-3,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		char szCH[8] = {0};
		sprintf(szCH,"%d",i-3);
		pStatic0[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, szCH);
		items[0].SubItems[0].push_back(pStatic0[i]);
	}
	
	int vLen = GetChnNameMax();
	
	for(i=0; i<rows; i++)
	{
		pTable00->GetTableRect(1,i+1,&tmpRt);
		
		//int vLen = (tmpRt.right-tmpRt.left-4)/TEXT_WIDTH*2;
		
		pEdit0[i] = CreateEdit(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
								rtSubPage.left+tmpRt.right-40,rtSubPage.top+tmpRt.bottom-1), //cw_test 2 
								this,vLen,0,(CTRLPROC)&CPageLiveConfigFrameWork::OnEditChange0);
		pEdit0[i]->SetBkColor(VD_RGB(67,77,87));
		items[0].SubItems[0].push_back(pEdit0[i]);
	}

	for(i=0; i<rows; i++)
	{
		pTable00->GetTableRect(2,i+1,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pCheckBox0[i] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
			rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+23), 
			this,styleEditable,(CTRLPROC)&CPageLiveConfigFrameWork::OnCheck0);
		pCheckBox0[i]->SetValue(TRUE);
		items[0].SubItems[0].push_back(pCheckBox0[i]);
	}

	for(i=0; i<rows; i++)
	{
		pTable00->GetTableRect(3,i+1,&tmpRt);
		//pTable00->GetTableRect(3,1,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pButton0[i] = CreateButton(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+1, 
					   rtSubPage.left+tmpRt.left+94,rtSubPage.top+tmpRt.top+25), this, "&CfgPtn.Setup", (CTRLPROC)&CPageLiveConfigFrameWork::OnSetup0, NULL, buttonNormalBmp);
		pButton0[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);
		
		//csp modify 20121130
		if(0 == strcasecmp(tmp2, "R3104HD"))
		{
			pButton0[i]->Enable(FALSE);
		}
		
		items[0].SubItems[0].push_back(pButton0[i]);
	}
	
	rtSubPage.top = rtSubPage.bottom+5;
	rtSubPage.right = rtSubPage.left+120;
	rtSubPage.bottom = rtSubPage.bottom+30;

	for(i=LIVECFG_COLS+LIVECFG_ROWS; i<LIVECFG_COLS+LIVECFG_ROWS+2; i++)
	{
		pStatic0[i] = CreateStatic(rtSubPage, this, statictext[i-6]);
		rtSubPage.left += 300;
		rtSubPage.right = rtSubPage.left + 260;
		items[0].SubItems[0].push_back(pStatic0[i]);
	}
	pStatic0[i-1]->SetTextAlign(VD_TA_RIGHT|VD_TA_YCENTER);

	rtSubPage.left = rtSubPage.right-300+5;
	rtSubPage.right = rtSubPage.left+20;
	rtSubPage.bottom = rtSubPage.top+20;

	pCheckBox0[LIVECFG_ROWS] = CreateCheckBox(rtSubPage, this);
	pCheckBox0[LIVECFG_ROWS]->SetValue(TRUE);
	//pCheckBox0[4]->Enable(FALSE);
	items[0].SubItems[0].push_back(pCheckBox0[LIVECFG_ROWS]);
	
	rtSubPage.left = m_Rect.left+27;
	rtSubPage.top = rtSubPage.bottom + 5;//m_Rect.top+255;
	rtSubPage.right = m_Rect.left+m_Rect.Width()-27;
	rtSubPage.bottom = rtSubPage.top + 30;
	
	//printf("$$$$$$$$$$ rtSubPage.bottom = %d\n",rtSubPage.bottom);
	
	pTable01 = CreateTableBox(&rtSubPage,this, 4,1,0);
	pTable01->SetColWidth(0,60);
	pTable01->SetColWidth(1,260);
	pTable01->SetColWidth(2,130);
	items[0].SubItems[0].push_back(pTable01);
	
	pTable01->GetTableRect(0,0,&tmpRt);
	//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
	pCheckBox0[LIVECFG_ROWS+1] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
		rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+23), 
		this,styleEditable,(CTRLPROC)&CPageLiveConfigFrameWork::OnCheckAll0);
	//pCheckBox0[LIVECFG_ROWS+1]->SetValue(TRUE);//csp modify
	items[0].SubItems[0].push_back(pCheckBox0[LIVECFG_ROWS+1]);

	pTable01->GetTableRect(2,0,&tmpRt);
	//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
	pCheckBox0[LIVECFG_ROWS+2] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
		rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+23), 
		this);
	//pCheckBox0[LIVECFG_ROWS+2]->SetValue(TRUE);//csp modify
	items[0].SubItems[0].push_back(pCheckBox0[LIVECFG_ROWS+2]);
	
	pTable01->GetTableRect(3,0,&tmpRt);
	//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
	pButton0[LIVECFG_ROWS] = CreateButton(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+1, 
				   rtSubPage.left+tmpRt.left+94,rtSubPage.top+tmpRt.top+25), this, "&CfgPtn.Setup", (CTRLPROC)&CPageLiveConfigFrameWork::OnSetup0, NULL, buttonNormalBmp);
	pButton0[LIVECFG_ROWS]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push,pBmp_button_normal);
	
	//csp modify 20121130
	if(0 == strcasecmp(tmp2, "R3104HD"))
	{
		pButton0[LIVECFG_ROWS]->Enable(FALSE);
	}
	
	items[0].SubItems[0].push_back(pButton0[LIVECFG_ROWS]);
	
	items[0].cursubindex = 0;
	ShowSubPage(0,0,TRUE);
	pButton[0]->Enable(FALSE);
}

void CPageLiveConfigFrameWork::OnClkButton10()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < 2; i++)
	{
		if (pFocusButton == pButton10[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	static int patrolId = 1;
	
	if(bFind)
	{
		switch(i)
		{
			case 0:
			{
				//left
				if(patrolId == 1)
				{
					patrolId = 4;
				}
				else
				{
					patrolId--;
				}
				
			}break;
			case 1:
			{
				//right
				if(patrolId == 4)
				{
					patrolId = 1;
				}
				else
				{
					patrolId++;
				}
			}break;
			default:
				break;
		}

		char strID[8] = {0};
		sprintf(strID,"%d/4",patrolId);
		pStatic10[1]->SetText(strID);
	}

}

void CPageLiveConfigFrameWork::OnCombox10()
{

	int curSel = pComboBox10[0]->GetCurSel();

	SwitchPage(1,1-curSel);

}

void CPageLiveConfigFrameWork::OnCombox20()
{
	int curSel = pComboBox20[0]->GetCurSel();

	SwitchPage(2,1-curSel);
}

void CPageLiveConfigFrameWork::InitPage10()
{
	char* statictext[3] = 
	{
		"&CfgPtn.DisplayMode",
		"1/1",
		"&CfgPtn.DwellTime",
	};
	
	pStatic10[0] = CreateStatic(CRect(230, 50, 340,70), this, statictext[0]);
	items[1].SubItems[0].push_back(pStatic10[0]);
	
	SValue splitList[10];
	int nSplitReal;
	GetSplitList(splitList, &nSplitReal, 10);
	
	pComboBox10[0] = CreateComboBox(CRect(340,50,450,75), 
			this, NULL, NULL, NULL/*(CTRLPROC)&CPageLiveConfigFrameWork::OnCombox10*/, 0);
	
	for(int k=0; k<nSplitReal; k++)
	{
		pComboBox10[0]->AddString(splitList[k].strDisplay);
	}
	//pComboBox10[0]->AddString("1 X 1");
	//pComboBox10[0]->AddString("2 X 2");
	//pComboBox10[0]->SetCurSel(0);
	pComboBox10[0]->SetBkColor(VD_RGB(67,77,87));
	items[1].SubItems[0].push_back(pComboBox10[0]);
	
	pStatic10[1] = CreateStatic(CRect(m_Rect.Width()-27-100, 50, m_Rect.Width()-27,70), this, statictext[1]);
	pStatic10[1]->SetTextAlign(VD_TA_RIGHT);
	items[1].SubItems[0].push_back(pStatic10[1]);
	
	CRect rtSubPage(27, 84, m_Rect.Width()-27,84+190);
	
	//printf("dddddddd %d\n",m_Rect.top+m_Rect.Height()-170 - (m_Rect.top+43));
	pTable10 = CreateTableBox(&rtSubPage,this, 2,2,0);
	items[1].SubItems[0].push_back(pTable10);
	
	pTable11 = CreateTableBox(CRect(rtSubPage.left,rtSubPage.bottom-1,rtSubPage.right,rtSubPage.bottom+45),
								this, 1,1,0);
	items[1].SubItems[0].push_back(pTable11);
	
	pComboBox10[1] = CreateComboBox(CRect(127,120,221,145), 
			this, NULL, NULL, NULL, 0);
	pComboBox10[1]->AddString("CH 1");
	pComboBox10[1]->AddString("CH 2");
	pComboBox10[1]->AddString("CH 3");
	pComboBox10[1]->AddString("CH 4");
	pComboBox10[1]->SetCurSel(0);
	pComboBox10[1]->SetBkColor(VD_RGB(67,77,87));
	items[1].SubItems[0].push_back(pComboBox10[1]);

	pComboBox10[2] = CreateComboBox(CRect(421,120,515,145), 
		this, NULL, NULL, NULL, 0);
	pComboBox10[2]->AddString("CH 1");
	pComboBox10[2]->AddString("CH 2");
	pComboBox10[2]->AddString("CH 3");
	pComboBox10[2]->AddString("CH 4");
	pComboBox10[2]->SetCurSel(1);
	pComboBox10[2]->SetBkColor(VD_RGB(67,77,87));
	items[1].SubItems[0].push_back(pComboBox10[2]);

	pComboBox10[3] = CreateComboBox(CRect(127,215,221,240), 
		this, NULL, NULL, NULL, 0);
	pComboBox10[3]->AddString("CH 1");
	pComboBox10[3]->AddString("CH 2");
	pComboBox10[3]->AddString("CH 3");
	pComboBox10[3]->AddString("CH 4");
	pComboBox10[3]->SetCurSel(2);
	pComboBox10[3]->SetBkColor(VD_RGB(67,77,87));
	items[1].SubItems[0].push_back(pComboBox10[3]);

	pComboBox10[4] = CreateComboBox(CRect(421,215,515,240), 
		this, NULL, NULL, NULL, 0);
	pComboBox10[4]->AddString("CH 1");
	pComboBox10[4]->AddString("CH 2");
	pComboBox10[4]->AddString("CH 3");
	pComboBox10[4]->AddString("CH 4");
	pComboBox10[4]->SetCurSel(3);
	pComboBox10[4]->SetBkColor(VD_RGB(67,77,87));
	items[1].SubItems[0].push_back(pComboBox10[4]);

	pBmpButtonNormal[0] = VD_LoadBitmap(liveShortcutBmpName[0][0]);
	pBmpButtonSelect[0] = VD_LoadBitmap(liveShortcutBmpName[0][1]);
	pButton10[0] = CreateButton(CRect(45, 275, 
				   45+pBmpButtonNormal[0]->width,275+pBmpButtonNormal[0]->height), this, NULL, (CTRLPROC)&CPageLiveConfigFrameWork::OnClkButton10, NULL, buttonNormalBmp);
	pButton10[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0]);
	items[1].SubItems[0].push_back(pButton10[0]);


	pStatic10[2] = CreateStatic(CRect(200, 285, 300 ,305), this, statictext[2]);
	items[1].SubItems[0].push_back(pStatic10[2]);
	
	//printf("##########222222222221\n");
	
	SValue dwellList[20];
	int nDwellReal;
	GetDwellTimeList(dwellList, &nDwellReal, 20);

	pComboBox10[5] = CreateComboBox(CRect(320,283,420,308), 
		this, NULL, NULL, NULL, 0);
	
	for(int k=0; k<nDwellReal; k++)
	{
		pComboBox10[5]->AddString(dwellList[k].strDisplay);
	}
	/*
	pComboBox10[5]->AddString("0");
	pComboBox10[5]->AddString("5");
	pComboBox10[5]->AddString("10");
	pComboBox10[5]->AddString("30");
	pComboBox10[5]->SetCurSel(1);*/
	pComboBox10[5]->SetBkColor(VD_RGB(67,77,87));
	items[1].SubItems[0].push_back(pComboBox10[5]);

	
	pBmpButtonNormal[1] = VD_LoadBitmap(liveShortcutBmpName[1][0]);
	pBmpButtonSelect[1] = VD_LoadBitmap(liveShortcutBmpName[1][1]);
	pButton10[1] = CreateButton(CRect(555, 275, 
				   555+pBmpButtonNormal[1]->width,275+pBmpButtonNormal[1]->height), this, NULL, (CTRLPROC)&CPageLiveConfigFrameWork::OnClkButton10, NULL, buttonNormalBmp);
	pButton10[1]->SetBitmap(pBmpButtonNormal[1], pBmpButtonSelect[1], pBmpButtonSelect[1]);
	items[1].SubItems[0].push_back(pButton10[1]);
	
	items[1].cursubindex = 1;
	ShowSubPage(1,0,FALSE);
}

void CPageLiveConfigFrameWork::InitPage11()
{
	items[1].SubItems[1].push_back(pStatic10[0]);
	items[1].SubItems[1].push_back(pComboBox10[0]);
	items[1].SubItems[1].push_back(pStatic10[1]);
	
	CRect rtSubPage(27, 84, m_Rect.Width()-27,84+190);
	pTable12 = CreateTableBox(&rtSubPage,this, 1, 1, 0);
	items[1].SubItems[1].push_back(pTable12);
	
	items[1].SubItems[1].push_back(pTable11);
	
	pComboBox11 = CreateComboBox(CRect(274,167,368,192), this, NULL, NULL, NULL, 0);
	
	SValue chList[32];//SValue chList[10];//csp modify
	int nChReal = 0;
	GetChnList(chList, &nChReal, 32);
	
	for(int k=0; k<nChReal; k++)
	{
		pComboBox11->AddString(chList[k].strDisplay);
	}
	/*pComboBox11->AddString("CH 1");
	pComboBox11->AddString("CH 2");
	pComboBox11->AddString("CH 3");
	pComboBox11->AddString("CH 4");*/
	pComboBox11->SetCurSel(0);
	pComboBox11->SetBkColor(VD_RGB(67,77,87));
	items[1].SubItems[1].push_back(pComboBox11);
	
	items[1].SubItems[1].push_back(pButton10[0]);
	items[1].SubItems[1].push_back(pStatic10[2]);
	items[1].SubItems[1].push_back(pComboBox10[5]);
	items[1].SubItems[1].push_back(pButton10[1]);
	
	ShowSubPage(1,1,FALSE);
}

void CPageLiveConfigFrameWork::OnClkButton20()
{
	printf("OnClkButton2\n");
}

void CPageLiveConfigFrameWork::InitPage20()
{
	char* statictext[3] = 
	{
		"&CfgPtn.DisplayMode",
		"1/1",
		"&CfgPtn.DwellTime",
	};
	
	//printf("CPageLiveConfigFrameWork::InitPage20-1-xxxxxx,statictext[0]=%s\n",statictext[0]);
	//fflush(stdout);
	
	pStatic20[0] = CreateStatic(CRect(230, 50, 340, 70), this, statictext[0]);
	
	//printf("CPageLiveConfigFrameWork::InitPage20-1.0\n");
	//fflush(stdout);
	
	items[2].SubItems[0].push_back(pStatic20[0]);
	
	//printf("CPageLiveConfigFrameWork::InitPage20-1.1\n");
	
	SValue splitList[10];
	int nSplitReal;
	GetSplitList(splitList, &nSplitReal, 10);
	
	//printf("CPageLiveConfigFrameWork::InitPage20-1.2,nSplitReal=%d\n",nSplitReal);
	
	pComboBox20[0] = CreateComboBox(CRect(340,50,450,75), 
			this, NULL,  NULL, NULL /*(CTRLPROC)&CPageLiveConfigFrameWork::OnCombox20*/,0);
	
	//printf("CPageLiveConfigFrameWork::InitPage20-1.3\n");
	
	for(int k=0; k<nSplitReal; k++)
	{
		pComboBox20[0]->AddString(splitList[k].strDisplay);
	}
	
	//printf("CPageLiveConfigFrameWork::InitPage20-1.4\n");
	
	//pComboBox20[0]->AddString("2 X 2");
	//pComboBox20[0]->SetCurSel(0);
	pComboBox20[0]->SetBkColor(VD_RGB(67,77,87));
	items[2].SubItems[0].push_back(pComboBox20[0]);
	
	//printf("CPageLiveConfigFrameWork::InitPage20-1.5\n");
	
	pStatic20[1] = CreateStatic(CRect(m_Rect.Width()-27-100, 50, m_Rect.Width()-27,70), this, statictext[1]);
	pStatic20[1]->SetTextAlign(VD_TA_RIGHT);
	items[2].SubItems[0].push_back(pStatic20[1]);
	
	//printf("CPageLiveConfigFrameWork::InitPage20-2\n");
	
	CRect rtSubPage(27, 84, m_Rect.Width()-27,84+190);
	
	//printf("dddddddd %d\n",m_Rect.top+m_Rect.Height()-170 - (m_Rect.top+43));
	pTable20 = CreateTableBox(&rtSubPage,this, 2,2,0);
	items[2].SubItems[0].push_back(pTable20);
	
	pTable21 = CreateTableBox(CRect(rtSubPage.left,rtSubPage.bottom-1,rtSubPage.right,rtSubPage.bottom+45),
								this, 1,1,0);
	items[2].SubItems[0].push_back(pTable21);
	
	pComboBox20[1] = CreateComboBox(CRect(127,120,221,145), 
			this, NULL, NULL, NULL, 0);
	pComboBox20[1]->AddString("CH 1");
	pComboBox20[1]->AddString("CH 2");
	pComboBox20[1]->AddString("CH 3");
	pComboBox20[1]->AddString("CH 4");
	pComboBox20[1]->SetCurSel(0);
	pComboBox20[1]->SetBkColor(VD_RGB(67,77,87));
	items[2].SubItems[0].push_back(pComboBox20[1]);
	
	pComboBox20[2] = CreateComboBox(CRect(421,120,515,145), 
		this, NULL, NULL, NULL, 0);
	pComboBox20[2]->AddString("CH 1");
	pComboBox20[2]->AddString("CH 2");
	pComboBox20[2]->AddString("CH 3");
	pComboBox20[2]->AddString("CH 4");
	pComboBox20[2]->SetCurSel(1);
	pComboBox20[2]->SetBkColor(VD_RGB(67,77,87));
	items[2].SubItems[0].push_back(pComboBox20[2]);
	
	pComboBox20[3] = CreateComboBox(CRect(127,215,221,240), 
		this, NULL, NULL, NULL, 0);
	pComboBox20[3]->AddString("CH 1");
	pComboBox20[3]->AddString("CH 2");
	pComboBox20[3]->AddString("CH 3");
	pComboBox20[3]->AddString("CH 4");
	pComboBox20[3]->SetCurSel(2);
	pComboBox20[3]->SetBkColor(VD_RGB(67,77,87));
	items[2].SubItems[0].push_back(pComboBox20[3]);
	
	pComboBox20[4] = CreateComboBox(CRect(421,215,515,240), 
		this, NULL, NULL, NULL, 0);
	pComboBox20[4]->AddString("CH 1");
	pComboBox20[4]->AddString("CH 2");
	pComboBox20[4]->AddString("CH 3");
	pComboBox20[4]->AddString("CH 4");
	pComboBox20[4]->SetCurSel(3);
	pComboBox20[4]->SetBkColor(VD_RGB(67,77,87));
	items[2].SubItems[0].push_back(pComboBox20[4]);
	
	//printf("CPageLiveConfigFrameWork::InitPage20-3\n");
	
	pBmpButtonNormal[0] = VD_LoadBitmap(liveShortcutBmpName[0][0]);
	pBmpButtonSelect[0] = VD_LoadBitmap(liveShortcutBmpName[0][1]);
	pButton20[0] = CreateButton(CRect(45, 275, 
				   45+pBmpButtonNormal[0]->width,275+pBmpButtonNormal[0]->height), this, NULL, (CTRLPROC)&CPageLiveConfigFrameWork::OnClkButton20, NULL, buttonNormalBmp);
	pButton20[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0]);
	items[2].SubItems[0].push_back(pButton20[0]);
	
	pStatic20[2] = CreateStatic(CRect(200, 285, 300 ,305), this, statictext[2]);
	items[2].SubItems[0].push_back(pStatic20[2]);
	
	SValue dwellList[20];
	int nDwellReal;
	GetDwellTimeList(dwellList, &nDwellReal, 20);
	
	pComboBox20[5] = CreateComboBox(CRect(320,283,420,308), 
		this, NULL, NULL, NULL, 0);
	
	for(int k=0; k<nDwellReal; k++)
	{
		pComboBox20[5]->AddString(dwellList[k].strDisplay);
	}
	/*
	pComboBox20[5]->AddString("0");
	pComboBox20[5]->AddString("5");
	pComboBox20[5]->AddString("10");
	pComboBox20[5]->AddString("30");
	pComboBox20[5]->SetCurSel(1);*/
	pComboBox20[5]->SetBkColor(VD_RGB(67,77,87));
	items[2].SubItems[0].push_back(pComboBox20[5]);
	
	//printf("CPageLiveConfigFrameWork::InitPage20-4\n");
	
	pBmpButtonNormal[1] = VD_LoadBitmap(liveShortcutBmpName[1][0]);
	pBmpButtonSelect[1] = VD_LoadBitmap(liveShortcutBmpName[1][1]);
	pButton20[1] = CreateButton(CRect(555, 275, 
				   555+pBmpButtonNormal[1]->width,275+pBmpButtonNormal[1]->height), this, NULL, (CTRLPROC)&CPageLiveConfigFrameWork::OnClkButton20, NULL, buttonNormalBmp);
	pButton20[1]->SetBitmap(pBmpButtonNormal[1], pBmpButtonSelect[1], pBmpButtonSelect[1]);
	items[2].SubItems[0].push_back(pButton20[1]);
	
	items[2].cursubindex = 1;
	
	//printf("CPageLiveConfigFrameWork::InitPage20-5\n");
	
	ShowSubPage(2,0,FALSE);
}

void CPageLiveConfigFrameWork::InitPage21()
{
	items[2].SubItems[1].push_back(pStatic20[0]);
	items[2].SubItems[1].push_back(pComboBox20[0]);
	items[2].SubItems[1].push_back(pStatic20[1]);
	
	CRect rtSubPage(27, 84, m_Rect.Width()-27,84+190);
	pTable22 = CreateTableBox(&rtSubPage,this, 1,1,0);
	items[2].SubItems[1].push_back(pTable22);
	
	items[2].SubItems[1].push_back(pTable21);
	
	pComboBox21 = CreateComboBox(CRect(274,167,368,192), 
			this, NULL, NULL, NULL, 0);
	
	SValue chList[32];//SValue chList[10];//csp modify
	int nChReal = 0;
	GetChnList(chList, &nChReal, 32);
	
	for(int k=0; k<nChReal; k++)
	{
		pComboBox21->AddString(chList[k].strDisplay);
	}
	/*
	pComboBox21->AddString("CH 1");
	pComboBox21->AddString("CH 2");
	pComboBox21->AddString("CH 3");
	pComboBox21->AddString("CH 4");
	pComboBox21->SetCurSel(0);*/
	
	pComboBox21->SetBkColor(VD_RGB(67,77,87));
	items[2].SubItems[1].push_back(pComboBox21);

	items[2].SubItems[1].push_back(pButton20[0]);
	items[2].SubItems[1].push_back(pStatic20[2]);
	items[2].SubItems[1].push_back(pComboBox20[5]);
	items[2].SubItems[1].push_back(pButton20[1]);
	
	ShowSubPage(2,1,FALSE);
}


void CPageLiveConfigFrameWork::OnTrackMove3()
{
	int pos = pScroll3->GetPos();
    if (m_page3==pos) {
        return;
    }

    m_page3 = pos;

	for(int i=0; i<MASK_ROWS; i++)
	{
		
		char szID[16] = {0};
		int nID = m_page3*MASK_ROWS+1+i;
		sprintf(szID,"%d",nID);
		pStatic3[i+MASK_COLS]->SetText(szID);
		
		if(nID >= GetMaxChnNum())
		{
			break;
		}
		
		/*
		pEnable[i]->SetValue(psGuiPtzPara.nIsPresetSet[nID]);

		char szName[32] = {0};
		sprintf(szName,"preset%d",nID);
		pName[i]->SetText(szName);
		*/
	}

	AdjustMaskRows();

}

void CPageLiveConfigFrameWork::OnTrackMove4()
{
	
}

void CPageLiveConfigFrameWork::OnSetup3()
{
	//printf("OnSetup3 \n");

	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < MASK_ROWS; i++)
	{
		if (pFocusButton == pButton3[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		int nCh = m_page3*MASK_ROWS+i;
		
		// 此处增加预览切换结果检查避免预览切换不成功时遮盖osd和多画面视频叠加的混乱状态
		if(0 == SwitchPreviewEx(EM_BIZPREVIEW_1SPLIT, nCh))
		{
			SetCurPreviewMode_CW(EM_BIZPREVIEW_1SPLIT);//cw_preview
			
			m_pMaskSet->SetMaskChannel(nCh);
			m_pMaskSet->Open();
		}
	}
}

void CPageLiveConfigFrameWork::InitPage3()
{	
	m_page3 = 0;
	m_maxPage3 = 1;
	
	char* statictext[2] = {
		"&CfgPtn.Channel",
		"&CfgPtn.MaskArea",
	};
	
	CRect rtSubPage(27, 50, m_Rect.Width()-27,50+270);
	pTable30 = CreateTableBox(&rtSubPage,this, 2,9,0);
	pTable30->SetColWidth(0,60);
	items[3].SubItems[0].push_back(pTable30);
	
	if(GetMaxChnNum()>MASK_ROWS)
	{
		m_maxPage3 = GetMaxChnNum()%MASK_ROWS?GetMaxChnNum()/MASK_ROWS+1:GetMaxChnNum()/MASK_ROWS;
		
		pScroll3 = CreateScrollBar(CRect(m_Rect.Width()-27-25,
											m_Rect.top+80,
											m_Rect.Width()-27,
											m_Rect.top+50+270), 
											this,
											scrollbarY,
											0,
											(m_maxPage3-1)*10, 
											1, 
											(CTRLPROC)&CPageLiveConfigFrameWork::OnTrackMove3);
		
		items[3].SubItems[0].push_back(pScroll3);
	}
	
	int rows = GetMaskRows();
	
	CRect tmpRt;
	int i = 0;
	for(i=0; i<MASK_COLS; i++)
	{
		pTable30->GetTableRect(i,0,&tmpRt);
		pStatic3[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, statictext[i]);
		pStatic3[i]->SetBkColor(VD_RGB(67,77,87));
		pStatic3[i]->SetTextAlign(VD_TA_XLEFT);
		items[3].SubItems[0].push_back(pStatic3[i]);
	}

	for(i=MASK_COLS; i<MASK_COLS+rows; i++)
	{
		pTable30->GetTableRect(0,i-1,&tmpRt);
		char szCh[16] = {0};
		sprintf(szCh,"%d",i-1);
		pStatic3[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, szCh);
		items[3].SubItems[0].push_back(pStatic3[i]);
	}
	
	for(i=0; i<rows; i++)
	{
		pTable30->GetTableRect(1,i+1,&tmpRt);
		
		pButton3[i] = CreateButton(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
						   rtSubPage.left+tmpRt.left+94,rtSubPage.top+tmpRt.top+25), this, "&CfgPtn.Setup", (CTRLPROC)&CPageLiveConfigFrameWork::OnSetup3, NULL, buttonNormalBmp);
		pButton3[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);
		
		items[3].SubItems[0].push_back(pButton3[i]);
	}

	items[3].cursubindex = 0;
	ShowSubPage(3,0,FALSE);
}

void CPageLiveConfigFrameWork::InitPage4()
{	
	m_page4 = 0;
	m_maxPage4 = 1;
	
	char* statictext[2] = {
		"&CfgPtn.Channel",
		"&CfgPtn.VideoSrc",
	};
	
	CRect rtSubPage(27, 50, m_Rect.Width()-27,50+270);
	pTable4 = CreateTableBox(&rtSubPage,this, 2,9,0);
	pTable4->SetColWidth(0,60);
	items[4].SubItems[0].push_back(pTable4);
	
	if((GetMaxChnNum()>VIDEOSRC_ROWS) && (0 != strcasecmp(tmp2, "R3110HDW")) && (0 != strcasecmp(tmp2, "R3106HDW")))
	{
		m_maxPage4 = GetMaxChnNum()%VIDEOSRC_ROWS?GetMaxChnNum()/VIDEOSRC_ROWS+1:GetMaxChnNum()/VIDEOSRC_ROWS;
		
		pScroll4 = CreateScrollBar(CRect(m_Rect.Width()-27-25,
											m_Rect.top+80,
											m_Rect.Width()-27,
											m_Rect.top+50+270), 
											this,
											scrollbarY,
											0,
											(m_maxPage4-1)*10, 
											1, 
											(CTRLPROC)&CPageLiveConfigFrameWork::OnTrackMove4);
		
		items[4].SubItems[0].push_back(pScroll4);
	}
	
	int rows = GetVideoSrcRows();
	
	CRect tmpRt;
	int i = 0;
	for(i=0; i<VIDEOSRC_COLS; i++)
	{
		pTable4->GetTableRect(i,0,&tmpRt);
		pStatic4[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, statictext[i]);
		pStatic4[i]->SetBkColor(VD_RGB(67,77,87));
		pStatic4[i]->SetTextAlign(VD_TA_XLEFT);
		items[4].SubItems[0].push_back(pStatic4[i]);
	}
	
	for(i=VIDEOSRC_COLS; i<VIDEOSRC_COLS+rows; i++)
	{
		pTable4->GetTableRect(0,i-VIDEOSRC_COLS+1,&tmpRt);
		char szCh[16] = {0};
		sprintf(szCh,"%d",i-VIDEOSRC_COLS+1);
		pStatic4[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, szCh);
		if((i > VIDEOSRC_COLS+1) && (0 == strcasecmp(tmp2, "R3110HDW")))
		{
			pStatic4[i]->Show(0);
		}
		else if((i > VIDEOSRC_COLS+1) && (0 == strcasecmp(tmp2, "R3106HDW")))
		{
			pStatic4[i]->Show(0);
		}
		else
		{
			items[4].SubItems[0].push_back(pStatic4[i]);
		}
	}
	
	SValue nVideoSrcList[10];
	int nReal = 0;
	GetVideoSrcTypeList(nVideoSrcList, &nReal, 10);
	for(i=0; i<rows; i++)
	{
		pTable4->GetTableRect(1,i+1,&tmpRt);
		
		pComboBox4[i] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
						   rtSubPage.left+tmpRt.left+98,rtSubPage.top+tmpRt.top+25), this, NULL, NULL, (CTRLPROC)&CPageLiveConfigFrameWork::OnComb4Sel, 0);
		for(int j = 0; j < nReal; j++)
		{
			pComboBox4[i]->AddString(nVideoSrcList[j].strDisplay);
		}
		pComboBox4[i]->SetBkColor(VD_RGB(67,77,87));
		if((i > 1) && (0 == strcasecmp(tmp2, "R3110HDW")))
		{
			pComboBox4[i]->Show(0);
		}
		else if((i > 1) && (0 == strcasecmp(tmp2, "R3106HDW")))
		{
			pComboBox4[i]->Show(0);
		}
		else
		{
			items[4].SubItems[0].push_back(pComboBox4[i]);
		}
	}
	
	items[4].cursubindex = 0;
	ShowSubPage(4,0,FALSE);
}

void CPageLiveConfigFrameWork::OnComb4Sel()
{
	int i = 0, j = 0;
	BOOL bFind = FALSE;
	CComboBox* pFocusCombo = (CComboBox*)GetFocusItem();
	for(i = 0; i < GetVideoSrcRows(); i++)
	{
		if(pFocusCombo == pComboBox4[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		m_nVedioSrc[m_page4*VIDEOSRC_ROWS+i] = pFocusCombo->GetCurSel();
	}
}

void CPageLiveConfigFrameWork::SwitchPage( int mainID,int subID)
{
	
	if (mainID<0 || mainID>=LIVECONFIG_SUBPAGES )
	{
		return;
	}

	//printf("curID = %d, mainId = %d \n",curID,mainID);
	
	if ( (mainID == curID) && (subID == items[curID].cursubindex) )
	{
		return;
	}

	//printf("###11curID = %d \n",curID);
	//printf("###11subID = %d \n",items[curID].cursubindex);

	//隐藏当前子页面，显示新的子页面
	ShowSubPage(curID,items[curID].cursubindex,FALSE);
	pButton[curID]->Enable(TRUE);

	curID = mainID;
	items[curID].cursubindex = subID;

	//printf("###22curID = %d \n",curID);
	//printf("###22subID = %d \n",items[curID].cursubindex);
	pButton[mainID]->Enable(FALSE);
	ShowSubPage(mainID,subID, TRUE);
}

void CPageLiveConfigFrameWork::ShowSubPage( int mainID,int subID, BOOL bShow )
{
	if (mainID<0 || mainID>=LIVECONFIG_SUBPAGES )
	{
		return;
	}
	
	int count = items[mainID].SubItems[subID].size();
	int i = 0;
	for (i=0; i<count; i++)
	{
		if(items[mainID].SubItems[subID][i])
		{
			items[mainID].SubItems[subID][i]->Show(bShow);
		}
	}
	
	//printf("ShowSubPage 2222 \n");
}

VD_BOOL CPageLiveConfigFrameWork::UpdateData( UDM mode )
{
	if(UDM_OPEN == mode)
	{
	#if 1
		//pStatic10[1]->SetText("1/4");
		char pChName[32];
		int rows = GetRows();
		for(int row=0; row<rows; row++)
		{
			int nCh = m_page0*LIVECFG_ROWS+row;
			GetChnName(nCh,pChName,32);
			pEdit0[row]->SetText(pChName);

			int value = GetDisplayNameCheck(nCh);
			pCheckBox0[row]->SetValue(value);
		}

		int value = GetLiveRecStateCheck();
		pCheckBox0[LIVECFG_ROWS]->SetValue(value);

		for(int i=0; i<GetMaxChnNum(); i++)
		{
			GetChnName(i, szChName[i], 32);
			bCheckValue[i] = GetDisplayNameCheck(i);
		}
	#endif
		
		/*
		int index = GetMainSplit();
		pComboBox10[0]->SetCurSel(index);
		
		index = GetSpotSplit();
		pComboBox20[0]->SetCurSel(index);
		
		index = GetMainDwellTime();
		pComboBox10[5]->SetCurSel(index);
		
		index = GetSpotDwellTime();
		pComboBox20[5]->SetCurSel(index);
		*/
		
		//printf("line:%d\n",__LINE__);
		
#if 1//csp modify 20130407
		if(0 == strcasecmp(tmp2, "R3104HD"))
		{
			GetVideoSrcIndex(m_nVedioSrc);
			memcpy(m_nVedioSrcLast,m_nVedioSrc,sizeof(m_nVedioSrc));
			rows = GetVideoSrcRows();
			for(int row=0; row<rows; row++)
			{
				int nCh = m_page4*VIDEOSRC_ROWS+row;
				if(nCh>=GetMaxChnNum())	break;
				pComboBox4[row]->SetCurSel((int)m_nVedioSrc[nCh]);
			}
		}
#endif
	}
	else if(UDM_CANCEL == mode)
	{
		/*SBizPreviewPara sBPP;
		sBPP.emBizPreviewMode = m_previewMode;
		sBPP.nModePara = 0;
		BizStartPreview(&sBPP);
		*/
	}
    else if (UDM_CLOSED == mode)
    {
        ((CPageConfigFrameWork *)(this->GetParent()))->ReflushItemName();
    }
	
	return TRUE;
}

void CPageLiveConfigFrameWork::AdjustLiveRows()
{
	int firstHide = LIVECFG_ROWS-(m_maxPage0*LIVECFG_ROWS-GetMaxChnNum());
	int lastHide = LIVECFG_ROWS;
	if(m_page0 == m_maxPage0-1)
	{
		
		for(int i=firstHide; i<lastHide; i++)
		{	
			//printf("pStatic0[i+LIVECFG_COLS]  = %d \n",pStatic0[i+LIVECFG_COLS]);
			if(pStatic0[i+LIVECFG_COLS]
				&& pEdit0[i]
				&& pCheckBox0[i]
				&& pButton0[i])
			{
				//printf("fk !!!! \n");
				pStatic0[i+LIVECFG_COLS]->Show(FALSE,TRUE);
				pEdit0[i]->Show(FALSE,TRUE);
				pCheckBox0[i]->Show(FALSE,TRUE);
				pButton0[i]->Show(FALSE,TRUE);
			}
			
		
		}
	}
	else
	{
		for(int i=firstHide; i<lastHide; i++)
		{
			if(pStatic0[i+LIVECFG_COLS]
				&& pEdit0[i]
				&& pCheckBox0[i]
				&& pButton0[i])
			{
				pStatic0[i+LIVECFG_COLS]->Show(TRUE,TRUE);
				pEdit0[i]->Show(TRUE,TRUE);
				pCheckBox0[i]->Show(TRUE,TRUE);
				pButton0[i]->Show(TRUE,TRUE);
			}
		}
	}
}

void CPageLiveConfigFrameWork::AdjustMaskRows()
{
	int firstHide = MASK_ROWS-(m_maxPage3*MASK_ROWS-GetMaxChnNum());
	int lastHide = MASK_ROWS;
	if(m_page3 == m_maxPage3-1)
	{	
		for(int i=firstHide; i<lastHide; i++)
		{
			
			if(pStatic3[i+MASK_COLS]
				&& pButton3[i])
			{
				pStatic3[i+MASK_COLS]->Show(FALSE,TRUE);
				pButton3[i]->Show(FALSE,TRUE);
			}
			
		}
	}
	else
	{
		for(int i=firstHide; i<lastHide; i++)
		{
			if(pStatic3[i+MASK_COLS]
				&& pButton3[i])
			{
				pStatic3[i+MASK_COLS]->Show(TRUE,TRUE);
				pButton3[i]->Show(TRUE,TRUE);
			}
		}
	}
}


void CPageLiveConfigFrameWork::WriteLogs()
{
	char pChName[32];
	char pChName2[32];
	
	for(int nCh=0; nCh<GetMaxChnNum(); nCh++)
	{
		GetChnName(nCh,pChName,32);
		if(strcmp(szChName[nCh], pChName))
		{
			BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_CHN_NAME);
			break;
		}
	}
}

void CPageLiveConfigFrameWork::SetCurPreviewMode(EMBIZPREVIEWMODE emMode)
{
	m_previewMode = emMode;
}

void CPageLiveConfigFrameWork::SetInfo(char* szInfo)
{
	pIconInfo->Show(TRUE);
	
	pInfoBar->SetText(szInfo);
}

void CPageLiveConfigFrameWork::ClearInfo()
{
	pIconInfo->Show(FALSE);
	
	pInfoBar->SetText("");
}

VD_BOOL CPageLiveConfigFrameWork::MsgProc( uint msg, uint wpa, uint lpa )
{
	int px,py;
	switch(msg)
	{
	case XM_MOUSEWHEEL://cw_scrollbar
		{
			int px = VD_HIWORD(lpa);
			int py = VD_LOWORD(lpa);
			CItem* pItem = GetItemAt(px, py);
			if(pItem == NULL)
			{
				if (curID == LIVECONFIG_BT_LIVE)
				{
					if(GetMaxChnNum()>LIVECFG_ROWS)
					{
						pScroll0->MsgProc(msg, wpa, lpa);
						break;
					}
				}
				else if (curID == LIVECONFIG_BT_MASK)
				{
					if(GetMaxChnNum()>MASK_ROWS)
					{
						pScroll3->MsgProc(msg, wpa, lpa);
						break;
					}
				}
			}
			break;
		}
	case XM_MOUSEMOVE:
		{
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);
			static CItem* last = NULL;
			CItem* temp = GetItemAt(px, py);
			
			if(temp != last)
			{
				if(temp == (pButton3[0]?(CButton*)pButton3[0]:(CButton*)!pButton3[0])
					|| temp == (pButton3[1]?(CButton*)pButton3[1]:(CButton*)!pButton3[1])
					|| temp == (pButton3[2]?(CButton*)pButton3[2]:(CButton*)!pButton3[2]) 
					|| temp == (pButton3[3]?(CButton*)pButton3[3]:(CButton*)!pButton3[3])
					|| temp == (pButton3[4]?(CButton*)pButton3[4]:(CButton*)!pButton3[4])
					|| temp == (pButton3[5]?(CButton*)pButton3[5]:(CButton*)!pButton3[5])
					|| temp == (pButton3[6]?(CButton*)pButton3[6]:(CButton*)!pButton3[6])
					|| temp == (pButton3[7]?(CButton*)pButton3[7]:(CButton*)!pButton3[7]))
				{
					if((0 == strcasecmp(tmp2, "R9624T"))
						|| (0 == strcasecmp(tmp2, "R9632S"))
						|| (0 == strcasecmp(tmp2, "R9516S"))
						 || (0 == strcasecmp(tmp2, "R9624SL"))
						  || (0 == strcasecmp(tmp2, "R9616S"))
						   || (0 == strcasecmp(tmp2, "R9608S"))
						   || (0 == strcasecmp(tmp2, "R3104HD"))//csp modify
						   || (0 == strcasecmp(tmp2, "R3110HDW"))//csp modify
						   || (0 == strcasecmp(tmp2, "R3106HDW"))//csp modify
						   || (0 == strcasecmp(tmp2, "R3116"))//csp modify
						   || (0 == strcasecmp(tmp2, "R3116W"))//csp modify
						   || (0 == strcasecmp(tmp2, "R2104"))//csp modify
						   || (0 == strcasecmp(tmp2, "R2104W"))//csp modify
						   || (0 == strcasecmp(tmp2, "R2108"))//csp modify
						   || (0 == strcasecmp(tmp2, "R2108W"))//csp modify
						   || (0 == strcasecmp(tmp2, "R2116"))//csp modify
						   || (0 == strcasecmp(tmp2, "R2016")))//csp modify
					{
						SetInfo("&CfgPtn.MaxMaskCif");
					}
					else if((0 == strcasecmp(tmp2, "NR2116")))
					{
						SetInfo("&CfgPtn.MaxMaskCif");
					}
					else if((0 == strcasecmp(tmp2, "NR3116")))
					{
						SetInfo("&CfgPtn.MaxMaskCif");
					}
					else if((0 == strcasecmp(tmp2, "NR3132")))
					{
						SetInfo("&CfgPtn.MaxMaskCif");
					}
					else if((0 == strcasecmp(tmp2, "NR3124")))
					{
						SetInfo("&CfgPtn.MaxMaskCif");
					}
					else if((0 == strcasecmp(tmp2, "NR1004")))
					{
						SetInfo("&CfgPtn.MaxMaskCif");
					}
					else if((0 == strcasecmp(tmp2, "NR1008")))
					{
						SetInfo("&CfgPtn.MaxMaskCif");
					}
					else
					{
						SetInfo("&CfgPtn.MaxMask");
					}
				}
				else if(temp == pCheckBox0[7])
				{
					SetInfo("&CfgPtn.SetAllChannels");
				}
				else if(temp == (pButton0[0]?(CButton*)pButton0[0]:(CButton*)!pButton0[0])
						  || temp == (pButton0[1]?(CButton*)pButton0[1]:(CButton*)!pButton0[1])
						  || temp == (pButton0[2]?(CButton*)pButton0[2]:(CButton*)!pButton0[2])
						  || temp == (pButton0[3]?(CButton*)pButton0[3]:(CButton*)!pButton0[3])
						  || temp == (pButton0[4]?(CButton*)pButton0[4]:(CButton*)!pButton0[4])
						  || temp == (pButton0[5]?(CButton*)pButton0[5]:(CButton*)!pButton0[5]))
				{
					SetInfo("&CfgPtn.ModifyVideoColor");
				}
				else if(temp == pButton0[6])
				{
					SetInfo("&CfgPtn.ModifyAllVideoColor");
				}
				#if 1//csp modify 20130407
				else if(temp == (pComboBox4[0]?(CComboBox*)pComboBox4[0]:(CComboBox*)!pComboBox4[0])
					|| temp == (pComboBox4[1]?(CComboBox*)pComboBox4[1]:(CComboBox*)!pComboBox4[1])
					|| temp == (pComboBox4[2]?(CComboBox*)pComboBox4[2]:(CComboBox*)!pComboBox4[2]) 
					|| temp == (pComboBox4[3]?(CComboBox*)pComboBox4[3]:(CComboBox*)!pComboBox4[3])
					|| temp == (pComboBox4[4]?(CComboBox*)pComboBox4[4]:(CComboBox*)!pComboBox4[4])
					|| temp == (pComboBox4[5]?(CComboBox*)pComboBox4[5]:(CComboBox*)!pComboBox4[5])
					|| temp == (pComboBox4[6]?(CComboBox*)pComboBox4[6]:(CComboBox*)!pComboBox4[6])
					|| temp == (pComboBox4[7]?(CComboBox*)pComboBox4[7]:(CComboBox*)!pComboBox4[7]))
				{
					SetInfo("&CfgPtn.RebootByModifyVideoSrc");
				}
				#endif
				else
				{
					ClearInfo();
				}
				
				last = temp;
			}
		}
		break;
	case XM_RBUTTONDOWN:
		{
			if((CPage*)(this->GetParentPage()))
			{
				((CPage*)(this->GetParentPage()))->Show(TRUE, TRUE);
			}
		}
		break;
		default:
		//return 0;
		break;
	}
	
	return CPage::MsgProc(msg, wpa, lpa);
}

