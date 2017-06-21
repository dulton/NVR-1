#include "GUI/Pages/PageNetworkConfig.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageConfigFrameWork.h"
#include "GUI/Pages/PageBasicConfigFrameWork.h"

//csp modify 20130421
#define FIX_DDNS_BUG

#define WWW_ADD_LEN_MIN	1

static	MAINFRAME_ITEM	items[NETWORKCFG_SUBPAGES];

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static VD_BITMAP * pBmp_tab_normal;
static VD_BITMAP * pBmp_tab_push;
static VD_BITMAP * pBmp_tab_select;

void bizData_SaveNetStreamPara(uchar nChn, SGuiNetStreamPara* pGNet);
void bizData_GetNetStreamPara(uchar nChn, SGuiNetStreamPara* pGNet);
int bizData_GetNetStreamParaDefault(uchar nChn, SGuiNetStreamPara* pGNet);
s32 bizData_SaveNetPara(SGuiNetPara* pGNet);
int bizData_GetNetParaDefault(SGuiNetPara* pGNet);
void bizData_GetNetPara(SGuiNetPara* pGNet);

static STabSize TabSzNetStream[] = 
{
	{ 6, 6 },
};

static int GetMaxChnNum() { return GetVideoMainNum(); }

CPageNetworkConfig::CPageNetworkConfig( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
, curID(0),lastPos00(0),lastPos20(0)
{
	pTable21 = NULL;
	pScrollbar20 = NULL;
	
	nChMax = GetMaxChnNum();
	if(TabSzNetStream[0].nRow >= nChMax)
	{
		nRealRow = nChMax;
		nPageNum = 1;
		nHideRow = 0;
	}
	else
	{
		nRealRow = TabSzNetStream[0].nRow;
		nPageNum = (nChMax+nRealRow-1)/nRealRow;			
		nHideRow = nPageNum*nRealRow-nChMax;
	}
	
	nCurPage = 0;
	
	//SetRect(CRect(0, 0, 700, 400/*370*/), FALSE);
	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);
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
	
	#ifdef _RZ_P2P_
	const char* szSubPgName[NETWORKCFG_BTNNUM+2] = 
	{
		"&CfgPtn.Network",
		"&CfgPtn.Substream",
		"&CfgPtn.Email",
		"&CfgPtn.OtherSettings",
		"&CfgPtn.P2pcloud",//xdc
		"&CfgPtn.Default",
		"&CfgPtn.apply",
		"&CfgPtn.Exit",
		"&CfgPtn.Prev",
		"&CfgPtn.Next",
	};
	
	int szSubPgNameLen[NETWORKCFG_BTNNUM+2] = 
	{
		TEXT_WIDTH*4,//"&CfgPtn.Network",
		TEXT_WIDTH*5,//"&CfgPtn.Substream",
		TEXT_WIDTH*4,//"&CfgPtn.Email",
		TEXT_WIDTH*4,//"&CfgPtn.OtherSettings",
		TEXT_WIDTH*4,//"&CfgPtn.P2pcloud",//xdc
		TEXT_WIDTH*2+10,//"&CfgPtn.Default",
		TEXT_WIDTH*2,//"&CfgPtn.apply",
		TEXT_WIDTH*2,//"&CfgPtn.Exit",
		TEXT_WIDTH*3,
		TEXT_WIDTH*3,
	};
	#else
	const char* szSubPgName[NETWORKCFG_BTNNUM+2] = 
	{
		"&CfgPtn.Network",
		"&CfgPtn.Substream",
		"&CfgPtn.Email",
		"&CfgPtn.OtherSettings",
		"&CfgPtn.Default",
		"&CfgPtn.apply",
		"&CfgPtn.Exit",
		"&CfgPtn.Prev",
		"&CfgPtn.Next",
	};
	
	int szSubPgNameLen[NETWORKCFG_BTNNUM+2] = 
	{
		TEXT_WIDTH*4,//"&CfgPtn.Network",
		TEXT_WIDTH*5,//"&CfgPtn.Substream",
		TEXT_WIDTH*4,//"&CfgPtn.Email",
		TEXT_WIDTH*4,//"&CfgPtn.OtherSettings",
		TEXT_WIDTH*2+10,//"&CfgPtn.Default",
		TEXT_WIDTH*2,//"&CfgPtn.apply",
		TEXT_WIDTH*2,//"&CfgPtn.Exit",
		TEXT_WIDTH*3,
		TEXT_WIDTH*3,
	};
	#endif
	
	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);
	
	CTableBox* pTabFrame = CreateTableBox(CRect(17, 40, m_Rect.Width()-17, 40+290),this,1,1);
	//pTabFrame->SetFrameColor(VD_RGB(56,108,148));
	pTabFrame->SetFrameColor(VD_RGB(8,66,189));
	
	CRect rtSub(25, 10, m_Rect.Width()-20, 10+pBmp_tab_normal->height);
	int i;
	for(i=0; i<NETWORKCFG_SUBPAGES; i++)
	{
		int btWidth = szSubPgNameLen[i]+10;
		rtSub.right = rtSub.left + btWidth;
		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageNetworkConfig::OnClickSubPage, NULL, buttonNormalBmp, 1);
		pButton[i]->SetBitmap(pBmp_tab_normal, pBmp_tab_select, pBmp_tab_push, pBmp_tab_push);
		rtSub.left = rtSub.right;
	}
	
	CRect rtSub1(295, m_Rect.Height()-80, m_Rect.Width()-20, m_Rect.Height()-80+pBmp_button_normal->height);
	rtSub1.left = rtSub1.right;
	for(i=NETWORKCFG_SUBPAGES; i<NETWORKCFG_BTNNUM; i++)
	{
		rtSub1.left -= szSubPgNameLen[i]+10;
	}
	
	rtSub1.left -= 10;
	//rtSub1.left += 40;
	
    int ReLeft = rtSub1.left;
	
	for(i=NETWORKCFG_SUBPAGES; i<NETWORKCFG_BTNNUM; i++)
	{
        int btWidth = szSubPgNameLen[i]+10;
		rtSub1.right = rtSub1.left + btWidth;
		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageNetworkConfig::OnClickBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
		rtSub1.left = rtSub1.right+5;
	}
	
	//CRect rtSub2(13, 35, m_Rect.Width()-18, 300);
	//pTable = CreateTableBox(&rtSub2,this, 1,1,0);
	CRect staRect(20, m_Rect.Height()-82, ReLeft - 2/*25+TEXT_WIDTH*10*/, m_Rect.Height()-52);
	
	pStatus = CreateStatic(staRect, this, "");
    //pStatus->SetBkColor(VD_RGB(67,77,87));
    //pStatus->SetTextAlign(VD_TA_CENTER);
    
    rtSub1.left = 37;
    for(i=NETWORKCFG_BTNNUM; i<NETWORKCFG_BTNNUM+2; i++)
	{
        //printf("***%s\n", __FUNCTION__);
        //int btWidth = szSubPgNameLen[i];
		rtSub1.right = rtSub1.left + 72;//btWidth;
		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageNetworkConfig::OnClickBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
		rtSub1.left = rtSub1.right+12;
	}
    
	InitPage00();
	InitPage01();
	InitPage1();
	InitPage20();
	InitPage21();
	InitPage3();
	
#ifdef _RZ_P2P_
	InitPage4();
#endif
	
    m_IsLoader = 0;
}

// type - 0 dhcp 1 pppoe 2 mail 3 ddns; 
// state -  0 con, 1 disconn
// rslt  - 0 suc, 1 fail
void CPageNetworkConfig::SetNetStatus(uchar type, uchar state, uchar rslt)
{
	char szInfo[64] = {0};
	char *pszInfo = NULL;
	
	if(m_IsLoader)
	{
		return;
	}
	
	if(type == 1)
	{
		if(state == 0 && rslt == 0)
		{
			pszInfo = (char*)GetParsedString("&CfgPtn.PPPOEConnected");
			
		#if 1//csp modify
			pppoe_flag = 1;
			pButton01->SetText("&CfgPtn.Disconnect");
			
			SBizNetPara sPara;
			if(0 == BizNetReadCfgEx(&sPara))
			{
				IPADDR sIPIns;
				
				sIPIns.l = sPara.HostIP;
				pIpCtrlpppoecontext[0]->SetIPAddress(&sIPIns);
				
				sIPIns.l = sPara.Submask;
				pIpCtrlpppoecontext[1]->SetIPAddress(&sIPIns);
				
				sIPIns.l = sPara.GateWayIP;
				pIpCtrlpppoecontext[2]->SetIPAddress(&sIPIns);
				
				sIPIns.l = sPara.DNSIP;
				pIpCtrlpppoecontext[3]->SetIPAddress(&sIPIns);
			}
		#endif
		}
		else
		{
			pszInfo = (char*)GetParsedString("&CfgPtn.PPPOEDisConnected");
			
		#if 1//csp modify
			pppoe_flag = 0;
			pButton01->SetText("&CfgPtn.Dial");
			
			IPADDR sIPIns;
			
			sIPIns.l = 0;
			pIpCtrlpppoecontext[0]->SetIPAddress(&sIPIns);
			
			sIPIns.l = 0;
			pIpCtrlpppoecontext[1]->SetIPAddress(&sIPIns);
			
			sIPIns.l = 0;
			pIpCtrlpppoecontext[2]->SetIPAddress(&sIPIns);
			
			sIPIns.l = 0;
			pIpCtrlpppoecontext[3]->SetIPAddress(&sIPIns);
		#endif
		}
	}
	else
	{	
		switch(type)
		{
			case 0:
			{
				strcpy(szInfo, "DHCP ");
			} break;
			case 1:
			{
				strcpy(szInfo, "PPPoE ");
				
			} break;
			case 2:
			{
				strcpy(szInfo, "Mail Test ");
			} break;
			case 3:
			{
				strcpy(szInfo, "DDNS ");
			} break;
			default:
				return;
		}
		
		switch(state)
		{
			case 0:
			{
				pszInfo = strcat(szInfo, GetParsedString("&CfgPtn.Start"));			
			} break;
			case 1:
			{
				pszInfo = strcat(szInfo, GetParsedString("&CfgPtn.stop"));
			} break;
			default:
				return;
		}
		
		switch(rslt)
		{
			case 0:
			{
				pszInfo = strcat(szInfo, GetParsedString("&CfgPtn.Success"));
			} break;
			case 1:
			{
				pszInfo = strcat(szInfo, GetParsedString("&CfgPtn.Fail"));
			} break;
			default:
				return;
		}
	}
	
	if(pStatus)
		pStatus->SetText(pszInfo);
}

int CPageNetworkConfig::GetProtocolValue(int sel)//cw_ddns
{
	return GetEPortValue(sel);
}

void CPageNetworkConfig::OnClickBtn()
{
	int i = 0, ret = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = NETWORKCFG_SUBPAGES; i < NETWORKCFG_BTNNUM+2; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		#ifdef _RZ_P2P_
		switch(i-1)
		#else
		switch(i)
		#endif
		{
			case 4:
			{
				//printf("default\n");
				
				//printf("curid = %d\n",curID);
				//get network config
				SGuiNetPara sPIns;
				
				memset(&sPIns, 0, sizeof(sPIns));
				ret |= bizData_GetNetParaDefault(&sPIns);
				
				switch(curID)
				{
					case 0:
					{
						char szTmp[10] = {0};
						sprintf(szTmp, "%d", sPIns.HttpPort);
						pEdit00[0]->SetText(szTmp);
						sprintf(szTmp, "%d", sPIns.TCPPort);
						pEdit00[1]->SetText(szTmp);
						
						sprintf(szTmp, "%d", sPIns.MobilePort);
						pEdit00[2]->SetText(szTmp);
						
						pCheckBox00->SetValue(sPIns.DhcpFlag);
						
						if(0)//sPIns.DhcpFlag)
						{
							
						}
						else
						{
							IPADDR sIPIns;
							
							sIPIns.l = sPIns.HostIP;
							pIpCtrl00[0]->SetIPAddress(&sIPIns);
							sIPIns.l = sPIns.Submask;
							pIpCtrl00[1]->SetIPAddress(&sIPIns);
							sIPIns.l = sPIns.GateWayIP;
							pIpCtrl00[2]->SetIPAddress(&sIPIns);
							sIPIns.l = sPIns.DNSIP;
							pIpCtrl00[3]->SetIPAddress(&sIPIns);
							sIPIns.l = sPIns.DNSIPAlt;
							pIpCtrl00[4]->SetIPAddress(&sIPIns);
						}
						
						pCheckBox01->SetValue(sPIns.PPPOEFlag);
						pEdit01[0]->SetText((char*)sPIns.PPPOEUser);
						pEdit01[1]->SetText((char*)sPIns.PPPOEPasswd);
						
						//csp modify 20130321
						pCheckBoxupnp[0]->SetValue((sPIns.UPNPFlag)?1:0);
					} break;
					case 1:
					{
						for(int i=0; i<nChMax; i++)
						{
							ret |= bizData_GetNetStreamParaDefault( i, &pTabPara[i] );	
						}	
						
						int nChBase = nCurPage*nRealRow;
						
						for(int i=0; i<nRealRow; i++)
						{
							pComboBox1[i]->SetCurSel(pTabPara[nChBase+i].nVideoResolution);
							pComboBox1[i+nRealRow]->SetCurSel(pTabPara[nChBase+i].nFrameRate);
							pComboBox1[i+nRealRow*2]->SetCurSel(pTabPara[nChBase+i].nBitRateType);
							
							//csp modify
							//if(bizData_IsCbr(pComboBox1[i+nRealRow*2]->GetString(pTabPara[nChBase+i].nBitRateType)))
							if(pTabPara[nChBase+i].nBitRateType == 0)
							{
								pComboBox1[i+nRealRow*3]->Enable(FALSE);
							}
							else
								pComboBox1[i+nRealRow*3]->Enable(TRUE);
							
							pComboBox1[i+nRealRow*3]->SetCurSel(pTabPara[nChBase+i].nPicLevel);
							pComboBox1[i+nRealRow*4]->SetCurSel(pTabPara[nChBase+i].nBitRate);
						}
					} break;
					case 2:
					{
						LoadMailCfg(&sPIns);						
					} break;
					case 3:
					{
						pCheckBox3->SetValue(sPIns.DDNSFlag);
						
						if(sPIns.DDNSFlag > 0)
						{
							#ifdef FIX_DDNS_BUG
							u8 index = 0;
							BizConfigGetParaListIndex(EM_BIZ_CFG_PARALIST_DDNSDOMAIN, sPIns.DDNSFlag-1, &index);
							pComboBox3[0]->SetCurSel(index);
							#else
							pComboBox3[0]->SetCurSel(sPIns.DDNSFlag-1);
							#endif
						}
						else
						{
							pComboBox3[0]->SetCurSel(0);
						}
						
						if(strcmp((char*)sPIns.DDNSUser,"0") == 0)
						{
						    #if 1
							pEdit3[0]->SetText("");
							//pEdit3[1]->SetText((char*)sPIns.DDNSPasswd);
    						//pEdit3[2]->SetText((char*)sPIns.DDNSDomain);
							#else
							for(int i = 0; i < 3; i++)
							{
                                pEdit3[i]->SetText("");
							}
							#endif
						}
						else
						{
							pEdit3[0]->SetText((char*)sPIns.DDNSUser);
    						pEdit3[1]->SetText((char*)sPIns.DDNSPasswd);
    						pEdit3[2]->SetText((char*)sPIns.DDNSDomain);
						}
						
						pComboBox3[1]->SetCurSel(sPIns.UpdateIntvl);
						
						OnCheckBox3();//csp modify 20130706
					} break;
				}
				
				if(0 == ret)
				{
					pStatus->SetText("&CfgPtn.LoadDefaultSuccess");
				}
				else
				{
					pStatus->SetText("&CfgPtn.LoadDefaultFail");	
				}				
			} break;
			case 5:
			{
				u8 bReset = 0;
				
				printf("apply\n");
				
				SGuiNetPara sPIns;
				
				bizData_GetNetPara(&sPIns);
				
				switch(curID)
				{
					case 0:
					{
						if(items[curID].cursubindex==0)
						{
							char	szNote[256] = {0};
							char 	szTmp[3][10] = {0};
							u32 	nTmp;

							for(int k=0; k<3; k++)
							{
								pEdit00[k]->GetText(szTmp[k], sizeof(szTmp[k]));
	                            if (!Digital(szTmp[k])) 
	                            { //字符串不全是数字
	                                MessgeBox(k);
	                                return;
	                            }
	                            //是数字时是否有超出界
	                            if ((atoi(szTmp[k])<=0 || atoi(szTmp[k])>65535))
	                            {
	                                MessgeBox(k);
	                                return;
	                            }
							}
							
							nTmp = strtol(szTmp[0],0,10);
							if(nTmp!=sPIns.HttpPort)
							{
								sPIns.HttpPort = nTmp;						
							}

							nTmp = strtol(szTmp[1],0,10);
							if(nTmp!=sPIns.TCPPort)
							{
								bReset += 1;
								sprintf(szNote, "%s%s", 
									GetParsedString("&CfgPtn.ServerPort"),
									GetParsedString("&CfgPtn.NetCfgChangeMsg"));
								sPIns.TCPPort = nTmp;								
							}
							
							nTmp = strtol(szTmp[2],0,10);							
							if(nTmp!=sPIns.MobilePort)
							{
								bReset += 1;
								memset(szNote, 0, sizeof(szNote));
								sprintf(szNote, "%s%s", 
									GetParsedString("&CfgPtn.MobilePort"),
									GetParsedString("&CfgPtn.NetCfgChangeMsg"));
								sPIns.MobilePort = nTmp;								
							}
							
							if(bReset>1)
							{
								memset(szNote, 0, sizeof(szNote));
								sprintf(szNote, "%s%s", 
									GetParsedString("&CfgPtn.Network"),
									GetParsedString("&CfgPtn.NetCfgChangeMsg"));
							}
							
							if(bReset)
							{
								char *pNote = (char*)szNote;
								char *pErr = (char*)GetParsedString("&CfgPtn.WARNING");
								UDM ret = MessageBox(pNote, pErr, MB_OKCANCEL|MB_ICONWARNING);
								if(GetRunStatue() != UDM_OK)
								{
									UpdateData(UDM_OPEN);
									return;
								}
							}
							
							sPIns.DhcpFlag = pCheckBox00->GetValue();
							
							IPADDR sIPIns;
							pIpCtrl00[0]->GetIPAddress(&sIPIns);
							sPIns.HostIP = sIPIns.l;

							pIpCtrl00[1]->GetIPAddress(&sIPIns);
							sPIns.Submask = sIPIns.l;
							pIpCtrl00[2]->GetIPAddress(&sIPIns);
							sPIns.GateWayIP = sIPIns.l;
							pIpCtrl00[3]->GetIPAddress(&sIPIns);
							sPIns.DNSIP = sIPIns.l;
							pIpCtrl00[4]->GetIPAddress(&sIPIns);
							sPIns.DNSIPAlt = sIPIns.l;
							
							if(sPIns.DhcpFlag)
							{
								SwitchDHCP();
								BizNetStartDHCP();
							}
							else
							{
								BizNetStopDHCP();
							}
						}
						else
						{
							sPIns.PPPOEFlag = pCheckBox01->GetValue();
							
							pEdit01[0]->GetText((char*)sPIns.PPPOEUser, 64);
							pEdit01[1]->GetText((char*)sPIns.PPPOEPasswd, 64);
							
							if(0 == sPIns.PPPOEFlag)
							{
								BizNetStopPPPoE();
							}
							
							//csp modify 20130321
							sPIns.UPNPFlag = pCheckBoxupnp[0]->GetValue();
						}
					} break;
					case 1:
					{
						// set gui ctrls
						if(pCheckBox1->GetValue())
						{
							for(int i=0; i<nChMax; i++)
							{
								pTabPara[i].nVideoResolution 	= pComboBox1[nRealRow*5]->GetCurSel();
								pTabPara[i].nFrameRate 			= pComboBox1[1+nRealRow*5]->GetCurSel();
								pTabPara[i].nBitRateType		= pComboBox1[2+nRealRow*5]->GetCurSel();
								pTabPara[i].nPicLevel 			= pComboBox1[3+nRealRow*5]->GetCurSel();
								pTabPara[i].nBitRate 			= pComboBox1[4+nRealRow*5]->GetCurSel();
							}
							
							for(int i=0; i<5; i++)
							{
								int nSel = pComboBox1[nRealRow*5+i]->GetCurSel();
								for( int j=0; j<nRealRow; j++ )
								{
									pComboBox1[j+nRealRow*i]->SetCurSel(nSel);
									if(i == 2)
									{
										int nCombSel = j+nRealRow*(i+1);
										//csp modify
										//if(bizData_IsCbr(pComboBox1[j+nRealRow*i]->GetString(nSel)))
										if(nSel == 0)
										{
											pComboBox1[nCombSel]->Enable(FALSE);
										}
										else
										{
											pComboBox1[nCombSel]->Enable(TRUE);
										}
									}
								}
							}		
						}
						
						for(int i=0; i<nChMax; i++)
						{
							bizData_SaveNetStreamPara( i, &pTabPara[i] );
						}
						
						pStatus->SetText("&CfgPtn.SaveParaSuccess");
					} break;
					case 2:
					{
						char szTmp[64] = {0};
						
						if(items[curID].cursubindex==0)
						{
							pEdit20[0]->GetText(szTmp, sizeof(szTmp));
							strcpy(sPIns.sAdancePara.szSmtpServer, szTmp);
							
							pEdit20[1]->GetText(szTmp, sizeof(szTmp));
							sPIns.SMTPServerPort = strtol(szTmp, 0, 10);
							pEdit20[2]->GetText(sPIns.sAdancePara.szSendMailBox, sizeof(sPIns.sAdancePara.szSendMailBox));
							pEdit20[3]->GetText(sPIns.sAdancePara.szSMailPasswd, sizeof(sPIns.sAdancePara.szSMailPasswd));
							pEdit20[4]->GetText(sPIns.sAdancePara.szReceiveMailBox, sizeof(sPIns.sAdancePara.szReceiveMailBox));
							pEdit20[5]->GetText(sPIns.sAdancePara.szReceiveMailBox2, sizeof(sPIns.sAdancePara.szReceiveMailBox2));
							pEdit20[6]->GetText(sPIns.sAdancePara.szReceiveMailBox3, sizeof(sPIns.sAdancePara.szReceiveMailBox3));
						}
					} break;
					case 3:
					{
						//pCheckBox3->SetValue(sPIns.DDNSFlag);
						//pComboBox3->SetCurSel(0);
						pEdit3[0]->GetText((char*)sPIns.DDNSUser, 64);
						pEdit3[1]->GetText((char*)sPIns.DDNSPasswd, 64);
						pEdit3[2]->GetText((char*)sPIns.DDNSDomain, 64);
						//pComboBox3[1]->SetCurSel(0);
						
						sPIns.DDNSFlag = pCheckBox3->GetValue();
						if(sPIns.DDNSFlag)
						{
							sPIns.DDNSFlag = 1;
							sPIns.DDNSFlag += GetProtocolValue(pComboBox3[0]->GetCurSel());//cw_ddns
							//sPIns.DDNSFlag += pComboBox3[0]->GetCurSel();
						}
						
						sPIns.UpdateIntvl = pComboBox3[1]->GetCurSel();
						
						if(sPIns.DDNSFlag)
						{
							sBizNetDdnsPara sParaIns;
							memset(&sParaIns, 0, sizeof(sBizNetDdnsPara));
							
							sParaIns.eProt = (EM_BIZ_DDNS_PROT)sPIns.DDNSFlag;
							pEdit3[0]->GetText(sParaIns.szUser, sizeof(sParaIns.szUser));
							pEdit3[1]->GetText(sParaIns.szPasswd, sizeof(sParaIns.szPasswd));
							pEdit3[2]->GetText(sParaIns.szDomain, sizeof(sParaIns.szDomain));
							
							#if 0//test
							printf("ddns test#####################################\n");
							sprintf(sParaIns.szUser, "we688");
							sprintf(sParaIns.szPasswd, "dangerous1we");
							sprintf(sParaIns.szDomain, "longsesdi.f3322.org");
							#endif
							
							pStatus->SetText("&CfgPtn.DDNSStarting");
							
							if(0==BizNetDdnsCtrl(EM_BIZDDNS_START, &sParaIns))
							{
								pStatus->SetText("&CfgPtn.DDNSStartSuccess");	
							}
							else
							{
								pStatus->SetText("&CfgPtn.DDNSStartFail");	
							}
							
							//printf("set ddns over,curID=%d\n",curID);
						}
						else
						{
							pStatus->SetText("&CfgPtn.DDNSServiceStopping");
							
							BizNetDdnsCtrl(EM_BIZDDNS_STOP, 0);
							
							pStatus->SetText("&CfgPtn.DDNSServiceStopped");
						}
					} break;
					default:
						return;
				}
				
				if(curID!=1)
				{
					if(curID==0)
					{
						BizNetWriteCfg((SBizNetPara*)&sPIns);
					}
					
					const char *pszRslt = 0;
					//printf("ip %u mask %u gateway %u\n", sPIns.HostIP, sPIns.Submask, sPIns.GateWayIP);
					ret = bizData_SaveNetPara(&sPIns);
					
					//printf("hehe-1\n");
					
					if(curID!=3 && !(items[curID].cursubindex==1 && curID==0)) // PPPoE拨号，DDNS注册/登录不提示保存参数
					{
						if( 0 == ret )
						{
							pszRslt = GetParsedString("&CfgPtn.SaveParaSuccess");
						}
						else
						{
							pszRslt = GetParsedString("&CfgPtn.SaveParaFail");
						}
						
						pStatus->SetText(pszRslt);
					}
					
					//printf("hehe-2\n");
				}
				
				if(bReset)
				{
					//printf("#####reset by netconfigchange \n");
					SetSystemLockStatus(1);//cw_shutdown
					BizSysComplexExit(EM_BIZSYSEXIT_RESTART);
					
					while(1)
					{
					    sleep(1);
					}
				}
			}
			break;
			case 6:
			{
				//printf("exit\n");
				this->Close();
			} break;
            case 7:
            {
                //上一步
                CPage** page = GetPage();
                if(page && page[EM_PAGE_BASICCFG])
                {
                    //this->Close();
                    page[EM_PAGE_BASICCFG]->UpdateData(UDM_GUIDE);
                    if (page[EM_PAGE_BASICCFG]->Open())
                    {
                        this->Close();
                    }
                    //page[EM_PAGE_BASICCFG]->UpdateData(UDM_EMPTY);
                }
            } break;
            case 8:
            {
                //下一步
                //保存配置
                SGuiNetPara sPIns;

                //先Get填充完整数据,防止有垃圾数据填充
                bizData_GetNetPara(&sPIns);
                
				char	szNote[256] = {0};
				char 	szTmp[3][10] = {0};
				u32 	nTmp;

				for(int k=0; k<3; k++)
				{
					pEdit00[k]->GetText(szTmp[k], sizeof(szTmp[k]));
                    if (!Digital(szTmp[k])) 
                    { //字符串不全是数字
                        MessgeBox(k);
                        return;
                    }
                    //是数字时是否有超出界
                    if ((atoi(szTmp[k])<=0 || atoi(szTmp[k])>65535))
                    {
                        MessgeBox(k);
                        return;
                    }
				}
				
				nTmp = strtol(szTmp[0],0,10);							
				if(nTmp!=sPIns.HttpPort)
				{
					sPIns.HttpPort = nTmp;						
				}
                
				nTmp = strtol(szTmp[1],0,10);
                u8	bReset = 0;
                
				if(nTmp!=sPIns.TCPPort)
				{
					bReset += 1;
					sprintf(szNote, "%s%s", 
						GetParsedString("&CfgPtn.ServerPort"),
						GetParsedString("&CfgPtn.NetCfgChangeMsg"));
					sPIns.TCPPort = nTmp;								
				}
				
				nTmp = strtol(szTmp[2],0,10);							
				if(nTmp!=sPIns.MobilePort)
				{
					bReset += 1;
					memset(szNote, 0, sizeof(szNote));
					sprintf(szNote, "%s%s", 
						GetParsedString("&CfgPtn.MobilePort"),
						GetParsedString("&CfgPtn.NetCfgChangeMsg"));
					sPIns.MobilePort = nTmp;								
				}
				
				//csp modify
				if(bReset>1)
				{
					memset(szNote, 0, sizeof(szNote));
					sprintf(szNote, "%s%s", 
						GetParsedString("&CfgPtn.Network"),
						GetParsedString("&CfgPtn.NetCfgChangeMsg"));
				}
				if(bReset)
				{
					char *pNote = (char*)szNote;
					char *pErr = (char*)GetParsedString("&CfgPtn.WARNING");
					UDM ret = MessageBox(pNote, pErr, MB_OKCANCEL|MB_ICONWARNING);
					if(ret != UDM_OK)
					{
						UpdateData(UDM_OPEN);
						return;
					}
				}
                
				sPIns.DhcpFlag = pCheckBox00->GetValue();							

				IPADDR sIPIns;
				pIpCtrl00[0]->GetIPAddress(&sIPIns);
				sPIns.HostIP = sIPIns.l;
				
				pIpCtrl00[1]->GetIPAddress(&sIPIns);
				sPIns.Submask = sIPIns.l;
				pIpCtrl00[2]->GetIPAddress(&sIPIns);
				sPIns.GateWayIP = sIPIns.l;
				pIpCtrl00[3]->GetIPAddress(&sIPIns);
				sPIns.DNSIP = sIPIns.l;
				pIpCtrl00[4]->GetIPAddress(&sIPIns);
				sPIns.DNSIPAlt = sIPIns.l;
				
				if(sPIns.DhcpFlag)
				{
					SwitchDHCP();
					BizNetStartDHCP();
				}
				else
				{
					BizNetStopDHCP();
				}
                
                BizNetWriteCfg((SBizNetPara*)&sPIns);
                bizData_SaveNetPara(&sPIns);
				
				//csp modify
				if(bReset)
				{
					//printf("#####reset by netconfigchange\n");
					SetSystemLockStatus(1);//cw_shutdown
					BizSysComplexExit(EM_BIZSYSEXIT_RESTART);
					
					while(1)
					{
					    sleep(1);
					}
				}
                
                CPage** page = GetPage();
                if(page && page[EM_PAGE_RECORD])
                {
                    //this->Close();
                    page[EM_PAGE_RECORD]->UpdateData(UDM_GUIDE);
                    if (page[EM_PAGE_RECORD]->Open())
                    {
                        this->Close();
                    }
                    //page[EM_PAGE_RECORD]->UpdateData(UDM_EMPTY);
                }
            } break;
			default:
				break;

		}
	}
}

void CPageNetworkConfig::AdjHide()
{
	if( nPageNum-1 == nCurPage && nPageNum > 1 )
	{
		VD_BOOL bHide = FALSE;
		
		int firstHide = nRealRow-nHideRow;
		int lastHide = nRealRow;
		int nRow     = nRealRow;
		int nCol     = TabSzNetStream[0].nCol;
		
		for(int i=firstHide; i<lastHide; i++)
		{
			pStatic1[i+nCol]->Show(bHide,TRUE);	
			pComboBox1[i]->Show(bHide,TRUE);
			pComboBox1[i+nRow]->Show(bHide,TRUE);
			pComboBox1[i+nRow*2]->Show(bHide,TRUE);
			pComboBox1[i+nRow*3]->Show(bHide,TRUE);
			pComboBox1[i+nRow*4]->Show(bHide,TRUE);
		}		
	}
}

void CPageNetworkConfig::LoadButtomCtl()
{
	if(m_IsLoader)
	{
		pButton[0]->Show(FALSE);
		pButton[1]->Show(FALSE);
		pButton[2]->Show(FALSE);
		pButton[3]->Show(FALSE);
		
		#ifdef _RZ_P2P_
		pButton[4]->Show(FALSE);
		pButton[5]->Show(FALSE);
		pButton[6]->Show(FALSE);
		
		pButton[7]->Show(TRUE);
		
		pButton[8]->Show(TRUE);
		pButton[9]->Show(TRUE);
		#else
		pButton[4]->Show(FALSE);
		pButton[5]->Show(FALSE);
		
		pButton[7]->Show(TRUE);
		pButton[8]->Show(TRUE);
		#endif
		
        pScrollbar00->Show(FALSE);
		
		#if 0//csp modify
		pEdit00[1]->Enable(FALSE);
		pEdit00[2]->Enable(FALSE);
		#endif
		
        pStatus->SetText("");
    }
	else
	{
		pButton[0]->Show(TRUE);
		pButton[1]->Show(TRUE);
		pButton[2]->Show(TRUE);
		pButton[3]->Show(TRUE);
		
		#ifdef _RZ_P2P_
		pButton[4]->Show(TRUE);
		pButton[5]->Show(TRUE);
		pButton[6]->Show(TRUE);
		
		pButton[7]->Show(TRUE);
		
		pButton[8]->Show(FALSE);
		pButton[9]->Show(FALSE);
		#else
		pButton[4]->Show(TRUE);
		pButton[5]->Show(TRUE);
		
		pButton[7]->Show(FALSE);
		pButton[8]->Show(FALSE);
		#endif
		
		pScrollbar00->Show(TRUE);
		
		pEdit00[1]->Enable(TRUE);
		pEdit00[2]->Enable(TRUE);
	}
}

void CPageNetworkConfig::OnClickSubPage()
{
	//printf("OnClickSubPage\n");
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < NETWORKCFG_SUBPAGES; i++)
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
		SwitchPage(i,items[i].cursubindex);
	}
}

CPageNetworkConfig::~CPageNetworkConfig()
{

}

VD_PCSTR CPageNetworkConfig::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

void CPageNetworkConfig::OnTrackMove00()
{
	int pos = pScrollbar00->GetPos();
    if (m_IsLoader || lastPos00 == pos) 
    {
        return;
    }
    //printf("***Page=%d\n", pos);
	
	if(lastPos00!=1 && pos==1)
	{
		//printf("pos = %d \n",pos);
		SwitchPage(0,1);
		lastPos00 = pos;
	}
	
	if(lastPos00!=0 && pos==0)
	{
		//printf("pos = %d \n",pos);
		SwitchPage(0,0);
		lastPos00 = pos;
	}
}

BOOL CPageNetworkConfig::Digital(const char * s)
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

void CPageNetworkConfig::OnEditChange00()
{
	//printf("OnEditChange00\n");
	int i = 0;
    int tmpIdex = 0;
    BOOL bFind = FALSE;
    CEdit *pFocus = (CEdit *)GetFocusItem();
    for (i = 0; i < 2; i++)
    {
        if (pFocus == pEdit00[i])
        {
            bFind = TRUE;
            tmpIdex = i;
            break;
        }
    }
    if (bFind)
    {
        #if 0
        char tmp[10] = {0};

        pFocus->GetText(tmp, sizeof(tmp));
        if (!Digital(tmp)) 
        { //字符串不全是数字
            MessgeBox(tmpIdex);
            return;
        }

        //是数字时是否有超出界
        if ((*tmp != NULL) && (atoi(tmp)<=0 || atoi(tmp)>65535))
        {
            MessgeBox(tmpIdex);
            return;
        }
        #endif
    }
}

void CPageNetworkConfig::InitPage00()
{
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.Width()-27 -22,m_Rect.top+50+270);

	pTable00 = CreateTableBox(&rtTmp,this, 2,9,0);
	pTable00->SetColWidth(0,160);
	items[0].SubItems[0].push_back(pTable00);

	CRect rtTmp1(m_Rect.Width()-27-23, m_Rect.top+50, 
		m_Rect.Width()-27,m_Rect.top+50+270);

	pTable01 = CreateTableBox(&rtTmp1,this, 1,1,0);
	items[0].SubItems[0].push_back(pTable01);

	pScrollbar00 = CreateScrollBar(CRect(m_Rect.Width()-30-20,53,m_Rect.Width()-30,53+264),
                                    this,
                                    scrollbarY,
								    0,
								    10,
								    1,
								    (CTRLPROC)&CPageNetworkConfig::OnTrackMove00);
	items[0].SubItems[0].push_back(pScrollbar00);
	
	const char* szStatic[] = {
		"&CfgPtn.HTTPPort",
		"&CfgPtn.ServerPort",
		"&CfgPtn.MobilePort",
		"&CfgPtn.ObtainanIPaddressautomatically",
		"&CfgPtn.IPAddress",
		"&CfgPtn.SubnetMask",
		"&CfgPtn.Gateway",
		"&CfgPtn.PrefeeredDNSServer",
		"&CfgPtn.AlternateDNSServer",
	};
	
	CRect tmpRt;
	int i = 0;
	for(i=0; i<sizeof(szStatic)/sizeof(szStatic[0]); i++)
	{

		pTable00->GetTableRect(0,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic00[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[i]);
		items[0].SubItems[0].push_back(pStatic00[i]);
	}
	
	for(i=0; i<3; i++)
	{
		pTable00->GetTableRect(1,i,&tmpRt);

		//int vLen = (tmpRt.right-tmpRt.left)/TEXT_WIDTH*2;
		
		pEdit00[i] = CreateEdit(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
								rtTmp.left+tmpRt.left+1+232/*rtTmp.left+tmpRt.right-1*/,rtTmp.top+tmpRt.bottom-1), 
								this,5/*vLen*/, edit_KI_NUMERIC,(CTRLPROC)&CPageNetworkConfig::OnEditChange00);
		pEdit00[i]->SetBkColor(VD_RGB(67,77,87));
	
		items[0].SubItems[0].push_back(pEdit00[i]);
	}
	
	pTable00->GetTableRect(1,3,&tmpRt);
	pCheckBox00 = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top+3, 
		rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
		this, styleEditable, (CTRLPROC)&CPageNetworkConfig::SwitchDHCP 
	);
	
	//  get & set value
	#if 0
	GetScrStrSetCurSelDo(
		pCheckBox00,
		GSR_CONFIG_NETWORK_DHCP, 
		EM_GSR_CTRL_COMB, 
		0
	);
	#endif
	
	items[0].SubItems[0].push_back(pCheckBox00);
	
	for(i=0; i<5; i++)
	{
		pTable00->GetTableRect(1,i+4,&tmpRt);
		
		pIpCtrl00[i] = CreateIPAddressCtrl(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
								rtTmp.left+tmpRt.left+1+235,rtTmp.top+tmpRt.bottom-1), 
								this);
		
		pIpCtrl00[i]->SetBkColor(VD_RGB(67,77,87));
		items[0].SubItems[0].push_back(pIpCtrl00[i]);
	}
	
	items[0].cursubindex = 0;
	pButton[0]->Enable(FALSE);	
}

void CPageNetworkConfig::OnTest01()
{
	char szUser[64];
	char szPasswd[64];
	
	pEdit01[0]->GetText(szUser, sizeof(szUser));
	pEdit01[1]->GetText(szPasswd, sizeof(szPasswd));
	
	#if 1//csp modify
	if(pppoe_flag)
	{
		pStatus->SetText("&CfgPtn.Disconnect");
		
		printf("PPPoE Disconnect......\n");
		
		if(0 != BizNetStopPPPoE())
		{
			pStatus->SetText("&CfgPtn.Fail");		
		}
	}
	else
	{
		pStatus->SetText("&CfgPtn.Dialing");
		
		printf("PPPoE Dialing......\n");
		
		if(0 != BizNetStartPPPoE(szUser, szPasswd))
		{
			pStatus->SetText("&CfgPtn.DialFail");		
		}
	}
	#else
	pStatus->SetText("&CfgPtn.Dialing");
	
	if(0 != BizNetStartPPPoE(szUser, szPasswd))
	{
		pStatus->SetText("&CfgPtn.DialFail");		
	}
	#endif
}

void CPageNetworkConfig::InitPage01()
{
	//debug_trace_err("CPageNetworkConfig");
	
	items[0].SubItems[1].push_back(pTable00);
	items[0].SubItems[1].push_back(pTable01);
	items[0].SubItems[1].push_back(pScrollbar00);
	
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.Width()-27 -22,m_Rect.top+50+270);
	
	const char* szStatic[3] = {
		"&CfgPtn.PPPoe",
		"&CfgPtn.Username",
		"&CfgPtn.Password",	
	};
	
	CRect tmpRt;
	int i = 0;
	for(i=0; i<3; i++)
	{
		pTable00->GetTableRect(0,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic01[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[i]);
		items[0].SubItems[1].push_back(pStatic01[i]);
	}
	
	pTable00->GetTableRect(1,0,&tmpRt);
	pCheckBox01 = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top+3, 
		rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
		this, styleEditable, (CTRLPROC)&CPageNetworkConfig::OnCheckBox01);
	
	pCheckBox01->SetValue(TRUE);
	items[0].SubItems[1].push_back(pCheckBox01);
	
	for(i=0; i<2; i++)
	{
		pTable00->GetTableRect(1,i+1,&tmpRt);
		
		int vLen = (tmpRt.right-tmpRt.left)/TEXT_WIDTH*2;
		
		int nStyle = 0;
		if(i == 1)
		{
			nStyle = editPassword;
		}
		pEdit01[i] = CreateEdit(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
								rtTmp.left+tmpRt.right-1,rtTmp.top+tmpRt.bottom-1), 
								this,vLen,nStyle | edit_KI_NOCH,(CTRLPROC)&CPageNetworkConfig::OnEditChange00);
		
		pEdit01[i]->SetBkColor(VD_RGB(67,77,87));
		items[0].SubItems[1].push_back(pEdit01[i]);
	}
	
	pTable00->GetTableRect(1,3,&tmpRt);
	pButton01 = CreateButton(CRect(rtTmp.left+tmpRt.left+2, rtTmp.top+tmpRt.top+2, 
								rtTmp.left+tmpRt.left+2+/*80*/100,rtTmp.top+tmpRt.bottom-2), this, "&CfgPtn.Dial", (CTRLPROC)&CPageNetworkConfig::OnTest01, NULL, buttonNormalBmp);
	pButton01->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
	items[0].SubItems[1].push_back(pButton01);
	
	#if 1//csp modify
	const char* szStaticpppoe[4] = {
		"&CfgPtn.IPAddress",
		"&CfgPtn.SubnetMask",
		"&CfgPtn.Gateway",
		"&CfgPtn.DNSServer"
	};
	
	pppoe_flag = 0;
	
	for(i=0; i<4; i++)
	{
		pTable00->GetTableRect(0,i+4,&tmpRt);
		
		pStaticpppoetitle[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStaticpppoe[i]);
		
		pStaticpppoetitle[i]->Enable(FALSE);
		
		items[0].SubItems[1].push_back(pStaticpppoetitle[i]);
	}
	for(i=0; i<4; i++)
	{
		pTable00->GetTableRect(1,i+4,&tmpRt);
		
		pIpCtrlpppoecontext[i] = CreateIPAddressCtrl(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
														rtTmp.left+tmpRt.left+1+235,rtTmp.top+tmpRt.bottom-1), 
														this);
		
		pIpCtrlpppoecontext[i]->Enable(FALSE);
		
		items[0].SubItems[1].push_back(pIpCtrlpppoecontext[i]);
	}
	
	const char* szStaticupnp[1] = 
	{
		"UPnP",
	};
	
	for(i=0; i<1; i++)
	{
		pTable00->GetTableRect(0,i+8,&tmpRt);
		
		pStaticupnptitle[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStaticupnp[i]);
		
		#ifdef CHIP_HISI351X_2X
		pStaticupnptitle[i]->Show(FALSE);
		#else
		items[0].SubItems[1].push_back(pStaticupnptitle[i]);
		#endif
	}
	for(i=0; i<1; i++)
	{
		pTable00->GetTableRect(1,i+8,&tmpRt);
		
		pCheckBoxupnp[i] = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top+3, 
			rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
			this, styleEditable, (CTRLPROC)&CPageNetworkConfig::OnCheckBoxupnp);
		
		#ifdef CHIP_HISI351X_2X
		pCheckBoxupnp[i]->Show(FALSE);
		#else
		items[0].SubItems[1].push_back(pCheckBoxupnp[i]);
		#endif
	}
	#endif
	
	ShowSubPage(0,1,FALSE);
	ShowSubPage(0,0,TRUE);
}

#define ENCODE_LINE	2
#define ENCODE_ALL	(30+2)

void CPageNetworkConfig::OnCombSel()
{
	int i = 0, j;
	BOOL bFind = FALSE;
	CComboBox* pFocusCombo = (CComboBox*)GetFocusItem();
	for (i = 0; i < (TabSzNetStream[0].nCol-1)*(1+nRealRow); i++)
	{
		if (pFocusCombo == pComboBox1[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		int nChBase = nCurPage*nRealRow;
		if(i<nRealRow)
		{
			pTabPara[nChBase+i].nVideoResolution = pFocusCombo->GetCurSel();
		}
		else if(i<2*nRealRow)
		{
			pTabPara[nChBase+i-nRealRow].nFrameRate = pFocusCombo->GetCurSel();
		}
		else if(i<3*nRealRow)
		{				
			pTabPara[nChBase+i-2*nRealRow].nBitRateType = pFocusCombo->GetCurSel();	
			
			//csp modify
			//if( bizData_IsCbr(pFocusCombo->GetString(pFocusCombo->GetCurSel())) )
			if(pFocusCombo->GetCurSel() == 0)
			{
				pComboBox1[i+nRealRow]->Enable(FALSE);
			}
			else
				pComboBox1[i+nRealRow]->Enable(TRUE);	
		}
		else if(i<4*nRealRow)
		{
			pTabPara[nChBase+i-3*nRealRow].nPicLevel = pFocusCombo->GetCurSel();				
		}
		else if(i<5*nRealRow)
		{
			pTabPara[nChBase+i-4*nRealRow].nBitRate = pFocusCombo->GetCurSel();				
		}
		else if(i == 5*nRealRow+2)
		{
			//csp modify
			//if( bizData_IsCbr(pFocusCombo->GetString(pFocusCombo->GetCurSel())) )
			if(pFocusCombo->GetCurSel() == 0)
			{
				pComboBox1[i+1]->Enable(FALSE);
			}
			else
				pComboBox1[i+1]->Enable(TRUE);
		}
	}
}

void CPageNetworkConfig::OnTrackMove0()
{
	VD_BOOL bHide = FALSE;
	int 	idx;
	
	int pos = pScrollbar->GetPos();
    if(nCurPage==pos) 
    {
        return;
    }
    nCurPage = pos;
	
	//printf("nCurPage = %d\n", nCurPage);
	
	int firstHide = nRealRow-nHideRow;
	int lastHide = nRealRow;
	int nRow     = nRealRow;
	int nCurPg   = nCurPage;
	int nPgNum   = nPageNum;
	int nCol     = TabSzNetStream[0].nCol;
	CStatic**	pStatic = pStatic1;
	for(int i=0; i<nRealRow; i++)
	{
		char szID[16] = {0};
		idx = nCurPage*nRealRow+i;
		sprintf(szID,"%d",idx+1);
		pStatic[i+nCol]->SetText(szID);
	}
	
	bHide = (nCurPg == nPgNum-1)?FALSE:TRUE;
	
	for(int i=0; i<nRow; i++)
	{
		idx = nCurPage*nRealRow+i;
		pComboBox1[i]->SetCurSel(pTabPara[idx].nVideoResolution);
		pComboBox1[i+nRow]->SetCurSel(pTabPara[idx].nFrameRate);
		pComboBox1[i+nRow*2]->SetCurSel(pTabPara[idx].nBitRateType);
		
		//csp modify
		//if(bizData_IsCbr(pComboBox1[i+nRealRow*2]->GetString(pTabPara[idx].nBitRateType)))//cw_substream
		if(pTabPara[idx].nBitRateType == 0)
		{
			pComboBox1[i+nRealRow*3]->Enable(FALSE);
		}
		else
		{
			pComboBox1[i+nRealRow*3]->Enable(TRUE);
		}
		pComboBox1[i+nRow*3]->SetCurSel(pTabPara[idx].nPicLevel);
		pComboBox1[i+nRow*4]->SetCurSel(pTabPara[idx].nBitRate);
		
		if(idx >= nChMax - 1)
		{
			break;
		}
	}
	
	for(int i=firstHide; i<lastHide; i++)
	{
		pStatic1[i+nCol]->Show(bHide,TRUE);
		pComboBox1[i]->Show(bHide,TRUE);
		pComboBox1[i+nRow]->Show(bHide,TRUE);
		pComboBox1[i+nRow*2]->Show(bHide,TRUE);
		pComboBox1[i+nRow*3]->Show(bHide,TRUE);
		pComboBox1[i+nRow*4]->Show(bHide,TRUE);
	}
}

void CPageNetworkConfig::InitPage1()
{
	//debug_trace_err("CPageNetworkConfig mark");
	
	pTabPara = (SGuiNetStreamPara*)malloc(sizeof(SGuiNetStreamPara)*nChMax);
	if(!pTabPara)
	{
		printf("no mem left!\n");
		exit(1);
	}
	
	const char* statictext[] = 
	{
		"&CfgPtn.Channel",
		"&CfgPtn.Resolution",
		"&CfgPtn.Fps",
		"&CfgPtn.Encode",
		"&CfgPtn.Quality",
		"&CfgPtn.MaxBitrate",
		/*
		"1",
		"2",
		"3",
		"4",
		*/
		"&CfgPtn.All",
		"&CfgPtn.Remaing",
		": 0 (CIF)"
	};
	
	CRect rtSubPage(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.left+m_Rect.Width()-27,m_Rect.top+50+210);

	//printf("dddddddd  %d \n",m_Rect.top+m_Rect.Height()-170 - (m_Rect.top+43));
	pTable10 = CreateTableBox(&rtSubPage,this, TabSzNetStream[0].nCol,TabSzNetStream[0].nRow+1,0);
	pTable10->SetColWidth(0,60);
	pTable10->SetColWidth(1,100);
	pTable10->SetColWidth(2,105);
	pTable10->SetColWidth(3,95);
	pTable10->SetColWidth(4,105);
	//pTable10->SetColWidth(5,180);

	items[1].SubItems[0].push_back(pTable10);

	if(nPageNum>1)
	{
		pScrollbar = CreateScrollBar(CRect(m_Rect.Width()-27-SCROLLBAR,
											m_Rect.top+80,
											m_Rect.Width()-27,
											m_Rect.top+50+210), 
											this,
											scrollbarY,
											0,
											(nPageNum-1)*10, 
											1, 
											(CTRLPROC)&CPageNetworkConfig::OnTrackMove0);

		items[1].SubItems[0].push_back(pScrollbar);
	}
	
	CRect tmpRt;
	int i = 0;
	for(i=0; i<TabSzNetStream[0].nCol; i++)
	{
		pTable10->GetTableRect(i,0,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic1[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, statictext[i]);
		pStatic1[i]->SetBkColor(VD_RGB(67,77,87));
		pStatic1[i]->SetTextAlign(VD_TA_XLEFT);
		items[1].SubItems[0].push_back(pStatic1[i]);
	}
	
	for(; i<nRealRow+TabSzNetStream[0].nCol; i++)
	{
		pTable10->GetTableRect(0,i-5,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		char szTmp[10] = {0};
		sprintf(szTmp, "%d", i-TabSzNetStream[0].nCol+1);
		pStatic1[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+3, rtSubPage.top+tmpRt.top, rtSubPage.left+tmpRt.right,rtSubPage.top+tmpRt.bottom), this, szTmp);
		items[1].SubItems[0].push_back(pStatic1[i]);
	}
	
	rtSubPage.top = rtSubPage.bottom+5;
	rtSubPage.right = rtSubPage.left+120;
	rtSubPage.bottom = rtSubPage.bottom+30;
	
	for(; i<nRealRow+TabSzNetStream[0].nCol+1; i++)
	{
		pStatic1[i] = CreateStatic(rtSubPage, this, statictext[i-nRealRow]);
		rtSubPage.left += 120;
		rtSubPage.right += 250;
		items[1].SubItems[0].push_back(pStatic1[i]);
	}
	
	//debug_trace_err("CPageNetworkConfig mark11");
	
	rtSubPage.left = 27;
	rtSubPage.top = m_Rect.top+50;
	rtSubPage.right = m_Rect.Width()-27;
	rtSubPage.bottom = rtSubPage.top +210;
	
	SValue resoList[10];
	int nResoReal;
	GetSubResoList(resoList, &nResoReal, 10);
	
	SValue frameRateList[10];
	int nFrReal;
	GetFrameRateList(0, frameRateList, &nFrReal, 10);
	
	SValue encodeList[10];
	int nEncReal;
	GetEncodeList(encodeList, &nEncReal, 10);
	
	SValue qualityList[10];
	int nQualityReal;
	GetQualityList(qualityList, &nQualityReal, 10);
	
	SValue bitRateList[15];
	int nBrReal;
	GetBitrateList(bitRateList, &nBrReal, 15);
	
	int j = 0;
	for(i=0; i<TabSzNetStream[0].nCol-1; i++)
	{
		CComboBox* pComboTmp = NULL;
		
		for(j=0; j<nRealRow; j++)
		{
			pTable10->GetTableRect(i+1,j+1,&tmpRt);
			//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
			
			if(i == 2) // 编码类型选择定码率时，不可以选择画质等级
			{
				pComboTmp = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
				rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1),
					this, NULL, NULL, (CTRLPROC)&CPageNetworkConfig::OnCombSel, 0);
			}
			else if(i == 4 && nPageNum>1)
			{
				pComboTmp = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
				rtSubPage.left+tmpRt.right-2-SCROLLBAR,rtSubPage.top+tmpRt.bottom-1),
					this, NULL, NULL, (CTRLPROC)&CPageNetworkConfig::OnCombSel, 0);				
			}
			else
			{
				pComboTmp = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
				rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1),
					this, NULL, NULL, (CTRLPROC)&CPageNetworkConfig::OnCombSel, 0);
			}
			
			pComboTmp->SetBkColor(VD_RGB(67,77,87));
			
			switch(i)
			{
				case 0:
				{
					for(int k=0; k<nResoReal; k++)
					{
						pComboTmp->AddString(resoList[k].strDisplay);
					}
				}break;
				case 1:
				{
					for(int k=0; k<nFrReal; k++)
					{
						pComboTmp->AddString(frameRateList[k].strDisplay);
					}
				}break;
				case 2:
				{
					for(int k=0; k<nEncReal; k++)
					{
						pComboTmp->AddString(encodeList[k].strDisplay);
						//printf("AddString %s\n",encodeList[k].strDisplay);
					}
				}break;
				case 3:
				{	
					for(int k=0; k<nQualityReal; k++)
					{
						pComboTmp->AddString(qualityList[k].strDisplay);
					}
				}break;
				case 4:
				{
					for(int k=0; k<nBrReal; k++)
					{
						pComboTmp->AddString(bitRateList[k].strDisplay);
					}
				}break;
				default:
				{}break;
			}
			items[1].SubItems[0].push_back(pComboTmp);
			pComboBox1[i*nRealRow+j] = pComboTmp;
		}
	}
	
	//debug_trace_err("CPageNetworkConfig mark12");
	
	rtSubPage.left = m_Rect.left+27;
	rtSubPage.top = m_Rect.Height()-125;//m_Rect.top+255;
	rtSubPage.right = m_Rect.Width()-27;
	rtSubPage.bottom = rtSubPage.top + 30;
	
	pTable11 = CreateTableBox(&rtSubPage,this, 6,1,0);
	pTable11->SetColWidth(0,60);
	pTable11->SetColWidth(1,100);
	pTable11->SetColWidth(2,105);
	pTable11->SetColWidth(3,95);
	pTable11->SetColWidth(4,105);
	items[1].SubItems[0].push_back(pTable11);

	pTable11->GetTableRect(0,0,&tmpRt);

	pCheckBox1 = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+4, rtSubPage.top+tmpRt.top+3, 
			rtSubPage.left+tmpRt.left+24,rtSubPage.top+tmpRt.top+23), 
			this);

	items[1].SubItems[0].push_back(pCheckBox1);

	int nChCombNum = (TabSzNetStream[0].nCol-1)*nRealRow;
	for(i=nChCombNum; i<nChCombNum+TabSzNetStream[0].nCol-1; i++)
	{
		pTable11->GetTableRect(i-nChCombNum+1,0,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		if(i==nChCombNum+2) 
		{
			pComboBox1[i] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
				rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
				this, NULL, NULL, (CTRLPROC)&CPageNetworkConfig::OnCombSel, 0);
		}
		else
		{
			pComboBox1[i] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
				rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
				this, NULL, NULL, NULL, 0);
		}

		pComboBox1[i]->SetBkColor(VD_RGB(67,77,87));

		switch(i-nChCombNum)
		{
			case 0:
			{
				for(int k=0; k<nResoReal; k++)
				{
					pComboBox1[i]->AddString(resoList[k].strDisplay);
				}
			}break;
			case 1:
			{
				for(int k=0; k<nFrReal; k++)
				{
					pComboBox1[i]->AddString(frameRateList[k].strDisplay);
				}
			}break;
			case 2:
			{
				for(int k=0; k<nEncReal; k++)
				{
					pComboBox1[i]->AddString(encodeList[k].strDisplay);
				}
			}break;
			case 3:
			{	
				for(int k=0; k<nQualityReal; k++)
				{
					pComboBox1[i]->AddString(qualityList[k].strDisplay);
				}
			}break;
			case 4:
			{
				for(int k=0; k<nBrReal; k++)
				{
					pComboBox1[i]->AddString(bitRateList[k].strDisplay);
				}
			}break;
			default:
			{}break;
		}
		
		pComboBox1[i]->SetCurSel(0);
		
		items[1].SubItems[0].push_back(pComboBox1[i]);
	}
	
	//debug_trace_err("CPageNetworkConfig mark13");
	
	items[1].cursubindex = 0;
	ShowSubPage(1,0,FALSE);
}

void CPageNetworkConfig::OnTest20()
{
	//printf("OnTest2\n");
	
	s32 ret;
	char szSmtp[64] = {0};
	char szSendBox[64] = {0};
	char szUserName[32] = {0};
	char szPasswd[64] = {0};
	char szRcvBox[64] = {0};
	char szToUser[32] = {0};
	char szBody[512] = {0};
	
	pEdit20[0]->GetText(szSmtp, sizeof(szSmtp));
	pEdit20[2]->GetText(szSendBox, sizeof(szSendBox));
	
	if(strlen(szSendBox) <= 5) return;
	
	memset(szUserName,0,sizeof(szUserName));//csp modify
	
	int i=0;
	while(szSendBox[i]!='@' && szSendBox[i]!='\0')//csp modify//while(szSendBox[i]!='@')//csp modify
	{
		szUserName[i] = szSendBox[i];
		i++;
	}
	
	pEdit20[3]->GetText(szPasswd, sizeof(szPasswd));
	
	SBizMailContent sMailIns;
	memset(&sMailIns, 0, sizeof(SBizMailContent));
	
	sMailIns.pSrcUsr = szUserName;
	sMailIns.pSrcBox = szSendBox;
	sMailIns.pUserName = szUserName;
	sMailIns.pPasswd = szPasswd;
	
	sMailIns.pSmtp = szSmtp;
	
	//csp modify
	//sMailIns.pSubject = (char*)GetParsedString("&CfgPtn.TestMailSubject");//"Mail Config Test for Tongli DVR!";
	//sMailIns.pSubject = "测试邮箱...";
	sMailIns.pSubject = (char *)"Mail config test...";
	
	sMailIns.bAttach  = 0;
	
	//send to recvbox 
	for(int j=0; j<3; j++)
	{
		memset(szRcvBox, 0, sizeof(szRcvBox));
		memset(szToUser, 0, sizeof(szToUser));
		
		pEdit20[4+j]->GetText(szRcvBox, sizeof(szRcvBox));
		
		TestMailInfo(j, 0, 0);
		if(strlen(szRcvBox)<5)
		{
			TestMailInfo(j, 1, 1);
			continue;
		}
		
		memset(szToUser,0,sizeof(szToUser));//csp modify
		
		i = 0;
		while(szRcvBox[i]!='@' && szRcvBox[i]!='\0')//while(szRcvBox[i]!='@')//csp modify
		{
			szToUser[i] = szRcvBox[i];
			i++;
		}
		
		sMailIns.pToBox = szRcvBox;
		sMailIns.pToUsr = szToUser;
		
		ret = -1;
		
		if(strlen(szRcvBox) > 5)
		{
			sprintf(szBody, "Test Ok, Sendbox %s and receivebox %s is valid!\n", szSendBox, szRcvBox);
			
			sMailIns.pBody = szBody;
			
			ret = BizNetSendMail(&sMailIns);
		}
		
		TestMailInfo(j, 1, ret?1:0);
	}
}

// idx 0-2;
// act 0 start test, 1 end test
// rslt 0 success, 1 fail

#define RECV_MAIL_MAX	3

void CPageNetworkConfig::TestMailInfo(uchar idx, uchar act, uchar rslt)
{
	char szInfo[64] = {0};
	char szInfo1[64] = {0};
	char *pszInfo = NULL;
	static uchar bBoxValid[RECV_MAIL_MAX] = {0,};
	u8 nValidBoxCnr = 0;

	if(act)
	{
		bBoxValid[idx] = rslt?0:1;
		
		if(idx == RECV_MAIL_MAX-1)
		{			
			for(int i=0; i<3; i++)
			{
				if(bBoxValid[i])
				{
					nValidBoxCnr++;
					sprintf(szInfo1, "%s %d ", szInfo1, i+1);
				}
			}

			if(nValidBoxCnr>0)
			{
				sprintf(szInfo, "%s%s%s%s",
					GetParsedString("&CfgPtn.The"),
					szInfo1, 
					GetParsedString("&CfgPtn.Box"), 
					GetParsedString("&CfgPtn.valid")
					); // box valid.
			}
			else
			{
				sprintf(szInfo, "%s%s%s", 
					GetParsedString("&CfgPtn.NONE"), 
					GetParsedString("&CfgPtn.Box"), 
					GetParsedString("&CfgPtn.valid")
					); // box valid.
			}
			
			pStatus->SetText(szInfo);

			memset(bBoxValid, 0, sizeof(bBoxValid));
		}
		else
		{
			sprintf(szInfo, "%s %s %d %s %s", 
				GetParsedString("&CfgPtn.Test"),
				GetParsedString("&CfgPtn.The"), 
				idx+1, 
				GetParsedString("&CfgPtn.Box"),
				GetParsedString("&CfgPtn.End")
			);
			pStatus->SetText(szInfo);			
		}
	}
	else
	{
		sprintf(szInfo, "%s %s %d %s..", 
			GetParsedString("&CfgPtn.Testing"),
			GetParsedString("&CfgPtn.The"),	
			idx+1,
			GetParsedString("&CfgPtn.Box")
			);
		pStatus->SetText(szInfo);		
	}
}

void CPageNetworkConfig::OnEditChange2()
{
	//printf("OnEditChange2\n");
}

void CPageNetworkConfig::OnTrackMove20()
{
#if 0
	int pos = pScrollbar20->GetPos();
	if(lastPos20<=5 && pos>5)
	{
		//printf("pos = %d \n",pos);
		SwitchPage(2,1);
		lastPos20 = pos;
	}
	
	if(lastPos20>=5 && pos<5)
	{
		//printf("pos = %d \n",pos);
		SwitchPage(2,0);
		lastPos20 = pos;
	}

	//printf("@@@  curid = %d \n",curID);

#endif
}

void CPageNetworkConfig::InitPage20()
{
	//debug_trace_err("CPageNetworkConfig mark2");
	
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, m_Rect.Width()/*-27*/ -22,m_Rect.top+50+270);// 取消邮件高级设置
	
	pTable20 = CreateTableBox(&rtTmp,this, 2,9,0);// 取消邮件高级设置
	pTable20->SetColWidth(0,250);
	items[2].SubItems[0].push_back(pTable20);
	
	/*
	CRect rtTmp1(m_Rect.Width()-27 -23, m_Rect.top+50, m_Rect.Width()-27,m_Rect.top+50+270);
	
	pTable21 = CreateTableBox(&rtTmp1,this, 1,1,0);
	items[2].SubItems[0].push_back(pTable21);
	*/
	
#if 0 // 取消邮件高级设置
	pScrollbar20 = CreateScrollBar(CRect(m_Rect.Width()-30-20,53,m_Rect.Width()-30,53+264), this,scrollbarY,
								   0,100, 1, (CTRLPROC)&CPageNetworkConfig::OnTrackMove20);
	
	items[2].SubItems[0].push_back(pScrollbar20);
#endif
	
	const char* szStatic[8] = 
	{
		"&CfgPtn.SMTPServer",
		"&CfgPtn.Port",
		"",//"&CfgPtn.SSLCheck",
		"&CfgPtn.Sendddress",
		"&CfgPtn.Password",
		"&CfgPtn.ReceiveAddress",
		"&CfgPtn.ReceiveAddress1",
		"&CfgPtn.ReceiveAddress2",
	};
	
	CRect tmpRt;
	int i = 0;
	for(i=0; i<8; i++)
	{
		pTable20->GetTableRect(0,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic20[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[i]);
		items[2].SubItems[0].push_back(pStatic20[i]);
	}
	
	for(i=0; i<2; i++)
	{
		pTable20->GetTableRect(1,i,&tmpRt);
		
		int vLen = (tmpRt.right-tmpRt.left)/TEXT_WIDTH*2;
		
		uint vstyle = edit_KI_NOCH;
		
		if(i == 1)
		{
			vstyle |= edit_KI_NUMERIC;
		}
		pEdit20[i] = CreateEdit(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
								rtTmp.left+tmpRt.right-1-16,rtTmp.top+tmpRt.bottom-1), 
								this,vLen, vstyle,(CTRLPROC)&CPageNetworkConfig::OnEditChange2);
		
		pEdit20[i]->SetBkColor(VD_RGB(67,77,87));
		items[2].SubItems[0].push_back(pEdit20[i]);
	}
#if 0
	//Test for NumberBox creat^^
	
	pTable20->GetTableRect(1,2,&tmpRt);
	CRect m_rect;
	m_rect.SetRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top+3, 
		rtTmp.left+tmpRt.left+200,rtTmp.top+tmpRt.top+23);
	pNumberBox20 = CreateNumberBox(m_rect, 
		this,
		0,
		65535,
		"%d",
		NULL,
		NULL);
	pNumberBox20->SetBkColor(VD_RGB(67,77,87));
	items[2].SubItems[0].push_back(pNumberBox20);
#endif
#if 0
	pTable20->GetTableRect(1,2,&tmpRt);
	pCheckBox20 = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top+3, 
		rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
		this);
	pCheckBox20->SetValue(TRUE);
	items[2].SubItems[0].push_back(pCheckBox20);
#endif
	
	for(i=3; i<8; i++)
	{
		pTable20->GetTableRect(1,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		int vLen = (tmpRt.right-tmpRt.left)/TEXT_WIDTH*2;
		
		int nStyle = 0;
		if(i == 4)
		{
			nStyle = editPassword;
		}
		
		pEdit20[i-1] = CreateEdit(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
								rtTmp.left+tmpRt.right-1-16,rtTmp.top+tmpRt.bottom-1), 
								this,vLen,nStyle|edit_KI_NOCH,(CTRLPROC)&CPageNetworkConfig::OnEditChange2);
		pEdit20[i-1]->SetBkColor(VD_RGB(67,77,87));
		items[2].SubItems[0].push_back(pEdit20[i-1]);
	}
	
	pTable20->GetTableRect(1,8,&tmpRt);
	pButton20 = CreateButton(CRect(rtTmp.left+tmpRt.left+2, rtTmp.top+tmpRt.top+2, 
								rtTmp.left+tmpRt.left+2+80,rtTmp.top+tmpRt.bottom-2), 
								this,"&CfgPtn.Test", (CTRLPROC)&CPageNetworkConfig::OnTest20, NULL, buttonNormalBmp);
	pButton20->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
	items[2].SubItems[0].push_back(pButton20);
	
	items[2].cursubindex = 0;
	
	ShowSubPage(2,0,FALSE);
}

void CPageNetworkConfig::InitPage21()
{
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.Width()-27 -22,m_Rect.top+50+270);
	
	const char* szStatic[] = 
	{
		"&CfgPtn.Advanced",
		//"&CfgPtn.AttachingImage",
		"&CfgPtn.SnapTimeInterval",
	};
	
	CRect tmpRt;
	int i = 0;
	int nPage21ItemNum = sizeof(szStatic)/sizeof(char*);
	for(i=0; i<2; i++)
	{
		pTable20->GetTableRect(0,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d \n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic21[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[i]);
		items[2].SubItems[1].push_back(pStatic21[i]);
	}
	
	for(i=0; i<2; i++)
	{
		pTable20->GetTableRect(1,i+1,&tmpRt);
		int vLen = (tmpRt.right-tmpRt.left)/TEXT_WIDTH*2;
		pEdit21[i] = CreateEdit(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
								rtTmp.left+tmpRt.right-1,rtTmp.top+tmpRt.bottom-1), 
								this,vLen,0,(CTRLPROC)&CPageNetworkConfig::OnEditChange00);
		pEdit21[i]->SetBkColor(VD_RGB(67,77,87));
		items[2].SubItems[1].push_back(pEdit21[i]);
	}
	
	ShowSubPage(2,1,FALSE);
}

void CPageNetworkConfig::UpdateDomain()
{
	return;
	
	char szTmp[64] = {0};
	char szUser[32] = {0};
	
	pEdit3[0]->GetText(szUser, sizeof(szUser));
	
	int sel = pComboBox3[0]->GetCurSel();
	switch(sel)
	{
		case 0:
		{
			sprintf(szTmp, ".%s.com", pComboBox3[0]->GetString(sel));			
		} break;
		case 1:
		{
			sprintf(szTmp, ".%s.org", pComboBox3[0]->GetString(sel));	
		} break;
		case 2:
		{
			sprintf(szTmp, ".%s.com", pComboBox3[0]->GetString(sel));	
		} break;
	}
	
	pEdit3[2]->SetText(szTmp);
}

void CPageNetworkConfig::OnEditChange3()
{
	#if 1//csp modify 20130322
	EM_BIZ_DDNS_PROT prot=(EM_BIZ_DDNS_PROT)(GetProtocolValue(pComboBox3[0]->GetCurSel())+1);//cw_ddns
	if(prot==EM_BIZDDNS_PROT_JMDVR)
	{
		//printf("OnEditChange3\n");
		
		char szTmp[64] = {0};
		char szUser[32] = {0};
		
		pEdit3[0]->GetText(szUser, sizeof(szUser));
		
		int sel = pComboBox3[0]->GetCurSel();
		sprintf(szTmp, "%s.%s.net", szUser, pComboBox3[0]->GetString(sel));
		
		pEdit3[2]->SetText(szTmp);
	}
	#else
	UpdateDomain();
	#endif
}

void CPageNetworkConfig::OnTest3()
{
	sBizNetDdnsPara sParaIns;
	memset(&sParaIns, 0, sizeof(sBizNetDdnsPara));
	sParaIns.eProt = (EM_BIZ_DDNS_PROT)(GetProtocolValue(pComboBox3[0]->GetCurSel())+1);//EM_BIZDDNS_PROT_TL;
	pEdit3[0]->GetText(sParaIns.szUser, sizeof(sParaIns.szUser));
	pEdit3[1]->GetText(sParaIns.szPasswd, sizeof(sParaIns.szPasswd));
	pEdit3[2]->GetText(sParaIns.szDomain, sizeof(sParaIns.szDomain));
	
	pStatus->SetText("&CfgPtn.DDNSRegistering");
	if( 0==BizNetDdnsCtrl(EM_BIZDDNS_REGIST, &sParaIns) )
	{
		pStatus->SetText("&CfgPtn.DDNSServiceTestSuccess");
	}
	else
		pStatus->SetText("&CfgPtn.DDNSServiceTestFailed");
}

void CPageNetworkConfig::OnCheckBox01()
{
	pButton01->Enable(pCheckBox01->GetValue());
	if(pCheckBox01->GetValue())
	{	
		pCheckBox00->SetValue(0);
		pEdit01[0]->Enable(TRUE);
		pEdit01[1]->Enable(TRUE);
	}
	else
	{
		pEdit01[0]->Enable(FALSE);
		pEdit01[1]->Enable(FALSE);
	}
}

//csp modify 20130321
void CPageNetworkConfig::OnCheckBoxupnp()
{
	
}

void CPageNetworkConfig::OnCheckBox3()
{
	//pButton3->Enable(pCheckBox3->GetValue());
	
	#ifdef FIX_DDNS_BUG
	int flag = pCheckBox3->GetValue();
	if(flag)
	{
		pComboBox3[0]->Enable(TRUE);
		pEdit3[0]->Enable(TRUE);
		pEdit3[1]->Enable(TRUE);
		pEdit3[2]->Enable(TRUE);
	}
	else
	{
		pComboBox3[0]->Enable(FALSE);
		pEdit3[0]->Enable(FALSE);
		pEdit3[1]->Enable(FALSE);
		pEdit3[2]->Enable(FALSE);
	}
	
	ShowRegistButton();
	#endif
}

void CPageNetworkConfig::OnCombox3()
{
	if(m_nLastSel != pComboBox3[0]->GetCurSel())
	{
		UpdateDomain();
	}
	
	m_nLastSel = pComboBox3[0]->GetCurSel();
	
	SGuiNetPara sPIns;
	
	memset(&sPIns, 0, sizeof(sPIns));
	bizData_GetNetPara(&sPIns);
	
#ifdef FIX_DDNS_BUG
	u8 index = 0;
	if(sPIns.DDNSFlag > 0)
	{
		BizConfigGetParaListIndex(EM_BIZ_CFG_PARALIST_DDNSDOMAIN, sPIns.DDNSFlag-1, &index);
	}
	else
	{
		index = 0xff;
	}
	if(m_nLastSel!=index)
#else
	if(m_nLastSel!=sPIns.DDNSFlag-1)
#endif
	{
		for(int i=0; i<3; i++)
		{
			pEdit3[i]->Empty();
		}
	}
	else
	{
 	#ifdef FIX_DDNS_BUG
		if(strcmp((char*)sPIns.DDNSUser,"0") == 0)
			pEdit3[0]->SetText("");
		else
			pEdit3[0]->SetText((char*)sPIns.DDNSUser);
		
		if(strcmp((char*)sPIns.DDNSPasswd,"0") == 0)
			pEdit3[1]->SetText("");
		else
			pEdit3[1]->SetText((char*)sPIns.DDNSPasswd);
		
		if(strcmp((char*)sPIns.DDNSDomain,"0") == 0)
			pEdit3[2]->SetText("");
		else
			pEdit3[2]->SetText((char*)sPIns.DDNSDomain);
	#else
		pEdit3[0]->SetText((char*)sPIns.DDNSUser);
		pEdit3[1]->SetText((char*)sPIns.DDNSPasswd);
		pEdit3[2]->SetText((char*)sPIns.DDNSDomain);
	#endif
	}
    
	ShowRegistButton();
}

void CPageNetworkConfig::InitPage3()
{	
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.Width()-27,m_Rect.top+50+270);
	
	pTable3 = CreateTableBox(&rtTmp,this, 2,9,0);
	pTable3->SetColWidth(0,250);
	items[3].SubItems[0].push_back(pTable3);
	
	const char* szStatic[6] = 
	{
		"&CfgPtn.DDNS",
		"&CfgPtn.DDNSServer",
		"&CfgPtn.Username",
		"&CfgPtn.Password",
		"&CfgPtn.HostDomain",
		"&CfgPtn.UpdateInterval",
	};
	
	CRect tmpRt;
	int i = 0;
	for(i=0; i<6; i++)
	{
		pTable3->GetTableRect(0,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic3[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+3, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right,rtTmp.top+tmpRt.bottom), this, szStatic[i]);
		
		//csp modify 20130706
		if(i == 5)
		{
			pStatic3[i]->Show(FALSE);
		}
		else
		{
			items[3].SubItems[0].push_back(pStatic3[i]);
		}
	}
	
	pTable3->GetTableRect(1,0,&tmpRt);
	pCheckBox3 = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top+3, 
		rtTmp.left+tmpRt.left+24,rtTmp.top+tmpRt.top+23), 
		this,styleEditable, (CTRLPROC)&CPageNetworkConfig::OnCheckBox3);
	pCheckBox3->SetValue(TRUE);
	items[3].SubItems[0].push_back(pCheckBox3);
	
	pTable3->GetTableRect(1,1,&tmpRt);
	//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
	pComboBox3[0] = CreateComboBox(CRect(rtTmp.left+tmpRt.left+2, rtTmp.top+tmpRt.top+1, 
		rtTmp.left+tmpRt.right-2 -10,rtTmp.top+tmpRt.bottom-1), 
		this, NULL, NULL, (CTRLPROC)&CPageNetworkConfig::OnCombox3, 0);
	pComboBox3[0]->SetBkColor(VD_RGB(67,77,87));
	
	GetScrStrInitComboxSelDo(
		(void*)pComboBox3[0],
		GSR_CONFIG_NETWORK_ADVANCE_DDNSDOMAIN,
		EM_GSR_COMBLIST,
		0
	);
	
	items[3].SubItems[0].push_back(pComboBox3[0]);
	
	for(i=0; i<3; i++)
	{
		pTable3->GetTableRect(1,i+2,&tmpRt);
		int vLen = (tmpRt.right-tmpRt.left)/TEXT_WIDTH*2;
		
		int nStyle = 0;
		if(i == 1)
		{
			nStyle = editPassword;
		}
		
		#if 1//csp modify 20130322
		pEdit3[i] = CreateEdit(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
								rtTmp.left+tmpRt.right-1 - 10,rtTmp.top+tmpRt.bottom-1), 
								this,vLen,nStyle|edit_KI_NOCH, (CTRLPROC)&CPageNetworkConfig::OnEditChange3);
		#else
		pEdit3[i] = CreateEdit(CRect(rtTmp.left+tmpRt.left+1, rtTmp.top+tmpRt.top+1, 
								rtTmp.left+tmpRt.right-1 - 10,rtTmp.top+tmpRt.bottom-1), 
								this,vLen,nStyle|edit_KI_NOCH, NULL);
		#endif
		
		pEdit3[i]->SetBkColor(VD_RGB(67,77,87));
		items[3].SubItems[0].push_back(pEdit3[i]);
	}
	
	//pEdit3[2]->Enable(FALSE);
	
	pTable3->GetTableRect(1,5,&tmpRt);
	//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
	pComboBox3[1] = CreateComboBox(CRect(rtTmp.left+tmpRt.left+2, rtTmp.top+tmpRt.top+1, 
		rtTmp.left+tmpRt.right-2 -10,rtTmp.top+tmpRt.bottom-1), 
		this, NULL, NULL, NULL, 0);
	pComboBox3[1]->SetBkColor(VD_RGB(67,77,87));
	
	GetScrStrInitComboxSelDo(
		(void*)pComboBox3[1],
		GSR_CONFIG_NETWORK_ADVANCE_DDNSUPINTVL, 
		EM_GSR_COMBLIST, 
		0
	);
	
	//items[3].SubItems[0].push_back(pComboBox3[1]);//csp modify 20130706
	pComboBox3[1]->Show(FALSE);//csp modify 20130706
	
	pTable3->GetTableRect(1,7,&tmpRt);
	pButton3 = CreateButton(CRect(rtTmp.left+tmpRt.left+2, rtTmp.top+tmpRt.top+2, 
								rtTmp.left+tmpRt.left+2+80,rtTmp.top+tmpRt.bottom-2), this,"&CfgPtn.Register", (CTRLPROC)&CPageNetworkConfig::OnTest3, NULL, buttonNormalBmp);
	pButton3->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
	items[3].SubItems[0].push_back(pButton3);
	
	items[3].cursubindex = 0;
	
	ShowSubPage(3,0,FALSE);
}

#ifdef _RZ_P2P_
//#include <linux/types.h>
//#include <mtd/mtd-user.h>

struct erase1
{
	unsigned int start;
	unsigned int size;
};

void startP2Pserver()
{
	//system("home/killp2p");
	system("/home/p2p");
}

void CPageNetworkConfig::OnCheckBox4()
{
	
}

char* encryption(char *str)//发送数据加密
{
 	return str;
}

char* decode(char *str)//接收数据解密
{
	return str;
}

int compare(char *p1,char *p2,int tt)
{
	return (0 == memcmp(p1,p2,tt));
}

typedef enum
{
	P2P_REG_IDLE,
	P2P_REG_HEART,
	P2P_REG_HEART2,
	P2P_REG_HEART3,
	P2P_REG_HEART4,
	P2P_REG_HEART5,
	P2P_REG_READY,
	P2P_REG_GET_MAC,
	P2P_REG_GET_SN,
	P2P_REG_GET_END,
	P2P_REG_WAIT_CLOSE,
	P2P_REG_CLOSE,
}EmP2PRegStatus;

const u8 g_tx_head[] = {0x58, 0x44, 0x43};
const u8 g_tx_tail[] = {0x52, 0x5a};

void tongxunheart(Tongxun *tt1)
{
	int i = 0;
	
	memset(tt1, 0, sizeof(Tongxun));
	
	memcpy(tt1->head, g_tx_head, sizeof(g_tx_head));
	memcpy(tt1->weibu, g_tx_tail, sizeof(g_tx_tail));
	
	tt1->leixing = P2P_TX_HEART;
	
	for(i = 0; i < TX_DATA_LEN; i++)
	{
		tt1->shuju[i] = '1';
	}
	
	unsigned int temp = tt1->leixing;
	for(i = 0; i < TX_DATA_LEN; i++)
	{
		temp += tt1->shuju[i];
	}
	tt1->jiaoyan = (temp % 256);
}

void tongxundata(Tongxun *tt1, int dataflag)
{
	int i = 0;
	
	memset(tt1, 0, sizeof(Tongxun));
	
	memcpy(tt1->head, g_tx_head, sizeof(g_tx_head));
	memcpy(tt1->weibu, g_tx_tail, sizeof(g_tx_tail));
	
	tt1->leixing = P2P_TX_CMD;
	
	if(dataflag == 1)
	{
		for(i = 0; i < TX_DATA_LEN; i++)
		{
			tt1->shuju[i] = i+48;
		}
	}
	else if(dataflag == 2)
	{
		memcpy(tt1->shuju, "MAC", 3);
	}
	else if(dataflag == 3)
	{
		memcpy(tt1->shuju, "DVR", 3);
	}
	else if(dataflag == 4)
	{
		memcpy(tt1->shuju, "OK", 2);
	}
	
	unsigned int temp = tt1->leixing;
	for(i = 0; i < TX_DATA_LEN; i++)
	{
		temp += tt1->shuju[i];
	}
	tt1->jiaoyan = (temp % 256);
}

int tongxunyanzheng(Tongxun *tt1)
{
	int i = 0;
	
	if(memcmp(tt1->head, g_tx_head, sizeof(g_tx_head)) || memcmp(tt1->weibu, g_tx_tail, sizeof(g_tx_tail)))
	{
		return 0;
	}
	
	unsigned int temp = tt1->leixing;
	for(i = 0; i < TX_DATA_LEN; i++)
	{
		temp += tt1->shuju[i];
	}
	
	if(tt1->jiaoyan != (temp % 256))
	{
		return 0;
	}
	
	return 1;
}

static int send_loop(int s, char *buf, unsigned int sndsize)
{
	int remian = sndsize;
	int sendlen = 0;
	int ret = 0;
	while(remian > 0)
	{
		ret = send(s, buf+sendlen, remian, 0);
		if(ret <= 0)
		{
			return ret;
		}
		sendlen += ret;
		remian -= ret;
	}
	return sendlen;
}

static int recv_loop(int s, char *buf, unsigned int rcvsize)
{
	u32 remian = rcvsize;
	u32 recvlen = 0;
	int ret = 0;
	while(remian > 0)
	{
		ret = recv(s, buf+recvlen, remian, 0);
		if(ret <= 0)
		{
			return ret;
		}
		recvlen += ret;
		remian -= ret;
	}
	return recvlen;
}

void CPageNetworkConfig::mtdopmac(char *mac)
{
	int ret = 0;
	
	char data2[17] = {0};
	memcpy(data2, mac, 17);
	
	int fd = open("/dev/mtd/6", O_RDWR);
	if(fd == -1)
	{
		printf("unable to open mtd6\n");
	}
	else
	{
		struct erase1 mtd;
		mtd.start = 0;
		mtd.size = 0x20000;
		ret = ioctl(fd, MEMERASE, (char*)&mtd);
		
		lseek(fd, 0, 0);
		ret = write(fd, data2, sizeof(data2));
		
		close(fd);
	}
}

void CPageNetworkConfig::mtdopuuid(char *uid)
{
	int ret = 0;
	
	char data2[16] = {0};
	memcpy(data2, uid, 16);
	
	char reg[2];
	reg[0] = 'R';
	reg[0] = 0;
	
	int fd = open("/dev/mtd/6", O_RDWR);
	if(fd == -1)
	{
		printf("unable to open mtd6\n");
	}
	else
	{
		lseek(fd, 17, 0);
		ret = write(fd, data2, sizeof(data2));
		
		lseek(fd, 34, 0);
		ret = write(fd, reg, sizeof(reg));
		
		close(fd);
	}
}

void CPageNetworkConfig::modifymac(char *macaddr)
{
	int i = 0;
	int j = 0;
	
	int n = 0;
	
	int err = 0;
	
	SBizParaTarget sParaTgtIns;
	SBizNetPara sCfgIns;
	
	memset(&sCfgIns, 0, sizeof(sCfgIns));
	sParaTgtIns.emBizParaType = EM_BIZ_NETPARA;
	sParaTgtIns.nChn = 0;
	
	int rtn = BizGetPara(&sParaTgtIns, &sCfgIns);
	if(rtn != 0)
	{
		printf("get net param error!\n");
		return;
	}
	
	for(j=0; j<6; j++)
	{
		for(i=0; i<2; i++)
		{
			char value = macaddr[j*3+i];
			if(((value >= '0') && (value <= '9')) 
				|| ((value >= 'a') && (value <= 'f'))
				|| ((value >= 'A') && (value <= 'F')))
			{
				sCfgIns.MAC[n++] = value;
			}
			else
			{
				err = 1;
				break;
			}
		}
		
		if(err)
		{
			break;
		}
		
		sCfgIns.MAC[n++] = ':';
	}
	sCfgIns.MAC[17] = '\0';
	
	if(err)
	{
		MessageBox("&CfgPtn.InvalidMacAddr", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
		return;
	}
	
	rtn = BizSetPara(&sParaTgtIns, &sCfgIns);
	if(rtn < 0)
	{
		printf("modify MAC error!\n");
		return;
	}
	else
	{
		SetMacAddress((char *)sCfgIns.MAC);
	}
}

void CPageNetworkConfig::setnetwork()
{
	
}

void CPageNetworkConfig::p2pregister()
{
	char p2psn[17] = {0};
	char macaddr[18] = {0};
	
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if(s == -1)
	{
		perror("failed to socket");
		return;
	}
	
	struct sockaddr_in address;
	socklen_t address_len = sizeof(address);
	
	memset(&address,0,sizeof(address));
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("192.168.1.30");
	address.sin_port = htons(6666);
	
	if(connect(s, (struct sockaddr *)&address, address_len) == -1)
	{
		perror("failed to connect");
		close(s);
		return;
	}
	
	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));
//	tv.tv_sec = 10;
//	tv.tv_usec = 0;
//	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
	
	EmP2PRegStatus status = P2P_REG_HEART;
	
	Tongxun sndtx;
	memset(&sndtx, 0, sizeof(sndtx));
	
	tongxunheart(&sndtx);
	int ret = send_loop(s, (char *)&sndtx, sizeof(sndtx));
	if(ret != sizeof(sndtx))
	{
		perror("failed to send");
		close(s);
		return;
	}
	printf("send leixing = %c\n", sndtx.leixing);
	
	int err = 0;
	
	while(1)
	{
		Tongxun rcvtx;
		memset(&rcvtx, 0, sizeof(rcvtx));
		
		ret = recv_loop(s, (char *)&rcvtx, sizeof(rcvtx));
		if(ret != sizeof(rcvtx))
		{
			perror("failed to recv");
			err = 1;
			break;
		}
		
		if(!tongxunyanzheng(&rcvtx))
		{
			printf("yanzheng error\n");
			err = 1;
			break;
		}
		
		printf("recv leixing = %c\n", rcvtx.leixing);
		
		if(rcvtx.leixing == P2P_TX_LOST)
		{
			printf("connection lost\n");
			err = 1;
			break;
		}
		
		switch(status)
		{
		case P2P_REG_HEART:
		case P2P_REG_HEART2:
		case P2P_REG_HEART3:
		case P2P_REG_HEART4:
		case P2P_REG_HEART5:
			{
				if(rcvtx.leixing != P2P_TX_HEART)
				{
					printf("not heart packet\n");
					err = 1;
					break;
				}
				
				if(status != P2P_REG_HEART5)
				{
					status = (EmP2PRegStatus)((int)status + 1);
					tongxunheart(&sndtx);
				}
				else
				{
					status = P2P_REG_READY;
					tongxundata(&sndtx, 1);
				}
			}
			break;
		case P2P_REG_READY:
			{
				//if(rcvtx.leixing != P2P_TX_CMD)
				if(rcvtx.leixing != P2P_TX_HEART)
				{
					printf("not data1 packet\n");
					err = 1;
					break;
				}
				
				int i = 0;
				for(i = 0; i < TX_DATA_LEN; i++)
				{
					if(rcvtx.shuju[i] != (i+48))
					{
						printf("data1 packet error\n");
						err = 1;
						break;
					}
				}
				
				if(err)
				{
					break;
				}
				
				printf("h0123456789...\n");
				
				status = P2P_REG_GET_MAC;
				tongxundata(&sndtx, 2);
			}
			break;
		case P2P_REG_GET_MAC:
			{
				if(rcvtx.leixing != P2P_TX_CMD)
				{
					printf("not data2 packet\n");
					err = 1;
					break;
				}
				
				if(memcmp((char *)rcvtx.shuju, "MAC", 3) != 0)
				{
					printf("data2 packet error\n");
					err = 1;
					break;
				}
				
				memcpy(macaddr, rcvtx.shuju+3, 17);
				macaddr[17] = '\0';
				printf("mac : %s\n", macaddr);
				
				status = P2P_REG_GET_SN;
				tongxundata(&sndtx, 3);
			}
			break;
		case P2P_REG_GET_SN:
			{
				if(rcvtx.leixing != P2P_TX_CMD)
				{
					printf("not data3 packet\n");
					err = 1;
					break;
				}
				
				if(memcmp((char *)rcvtx.shuju, "DVR", 3) != 0)
				{
					printf("data3 packet error\n");
					err = 1;
					break;
				}
				
				memcpy(p2psn, rcvtx.shuju+3, 16);
				p2psn[16] = '\0';
				printf("sn : %s\n", p2psn);
				
				status = P2P_REG_GET_END;
				tongxundata(&sndtx, 4);
			}
			break;
		case P2P_REG_GET_END:
			{
				if(rcvtx.leixing != P2P_TX_CMD)
				{
					printf("not end packet\n");
					err = 1;
					break;
				}
				
				int i = 0;
				for(i = 0; i < TX_DATA_LEN; i++)
				{
					if(rcvtx.shuju[i] != (i+48))
					{
						printf("end packet error\n");
						err = 1;
						break;
					}
				}
				if(i == TX_DATA_LEN)
				{
					printf("end packet is right\n");
				}
				
				status = P2P_REG_WAIT_CLOSE;
				tongxunheart(&sndtx);
			}
			break;
		case P2P_REG_WAIT_CLOSE:
			{
				if(rcvtx.leixing != P2P_TX_OVER)
				{
					printf("not over packet\n");
					err = 1;
					break;
				}
				
				status = P2P_REG_CLOSE;
			}
			break;
		case P2P_REG_CLOSE:
			break;
		default:
			printf("status error : %d\n", status);
			err = 1;
			break;
		}
		
		if(err || status == P2P_REG_CLOSE)
		{
			break;
		}
		
		ret = send_loop(s, (char *)&sndtx, sizeof(sndtx));
		if(ret != sizeof(sndtx))
		{
			perror("failed to send");
			err = 1;
			break;
		}
		
		printf("send leixing = %c\n", sndtx.leixing);
	}
	
	close(s);
	
	if(err)
	{
		
	}
}

void CPageNetworkConfig::InitPage4()
{	
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
			m_Rect.Width()-27,m_Rect.top+50+270);
	
	pTable4 = CreateTableBox(&rtTmp,this, 2, 9, 0);
	pTable4->SetColWidth(0,250);
	items[4].SubItems[0].push_back(pTable4);
	
	const char* szStatic[] = 
	{
		"&CfgPtn.Startup",
		"&CfgPtn.UUIDNumber",
		"&CfgPtn.MacAddr",
		"&CfgPtn.Status",
		
		"&CfgPtn.UnRegistered",//"&CfgPtn.Registered",
		"&CfgPtn.UnRegistered",//"&CfgPtn.Registered",
		
		"&CfgPtn.UnStarted",
	};
	
	CRect tmpRt;
	int i = 0;
	for(i=0; i<4; i++)
	{
		pTable4->GetTableRect(0,i,&tmpRt);
		//printf("l:%d,t:%d,r:%d,b:%d\n",tmpRt.left,tmpRt.top,tmpRt.right,tmpRt.bottom);
		pStatic4[i] = CreateStatic(CRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top, rtTmp.left+tmpRt.right, rtTmp.top+tmpRt.bottom), this, szStatic[i]);
		
		items[4].SubItems[0].push_back(pStatic4[i]);
	}
	
	pTable4->GetTableRect(1,0,&tmpRt);
	pCheckBox4 = CreateCheckBox(CRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top+3, 
		rtTmp.left+tmpRt.left+24, rtTmp.top+tmpRt.top+23), 
		this, styleEditable, (CTRLPROC)&CPageNetworkConfig::OnCheckBox4);
	items[4].SubItems[0].push_back(pCheckBox4);
	
	for(i=1; i<4; i++)
	{
		pTable4->GetTableRect(1,i,&tmpRt);
		pStatic4[4+i-1] = CreateStatic(CRect(rtTmp.left+tmpRt.left+4, rtTmp.top+tmpRt.top+1, 
			rtTmp.left+tmpRt.right-2-10, rtTmp.top+tmpRt.bottom-1), this, szStatic[4+i-1]);
		items[4].SubItems[0].push_back(pStatic4[4+i-1]);
	}
	
	pTable4->GetTableRect(1,6,&tmpRt);
	pButton4 = CreateButton(CRect(rtTmp.left+tmpRt.left+2, rtTmp.top+tmpRt.top+2, 
								rtTmp.left+tmpRt.left+2+80, rtTmp.top+tmpRt.bottom-2), this, "&CfgPtn.Register", (CTRLPROC)&CPageNetworkConfig::p2pregister, NULL, buttonNormalBmp);
	pButton4->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);
	items[4].SubItems[0].push_back(pButton4);
	
	items[4].cursubindex = 0;
	
	u8 p2p_open_flag = 0;
	u8 p2p_registered_flag = 0;
	
	char mac[18] = {0};
	char uuid[17] = {0};
	char testid[15] = {0};
	
	int fdinit = open("/dev/mtd/6", O_RDWR);
	if(fdinit != -1)
	{
		lseek(fdinit, 0, 0);
		read(fdinit, mac, 17);
		mac[17] = '\0';
		
		lseek(fdinit, 17, 0);
		read(fdinit, uuid, 16);
		uuid[16] = '\0';
		
		lseek(fdinit, 33, 0);
		read(fdinit, &p2p_open_flag, sizeof(p2p_open_flag));
		
		lseek(fdinit, 34, 0);
		read(fdinit, &p2p_registered_flag, sizeof(p2p_registered_flag));
		
		memcpy(testid, uuid, sizeof(testid)-1);
		
		pStatic4[4]->SetText(testid);//uuid
		pStatic4[5]->SetText(mac);//mac
		
		close(fdinit);
	}
	
	if(p2p_open_flag == 'o')
	{
		pCheckBox4->SetValue(TRUE);
		pStatic4[6]->SetText("&CfgPtn.Started");
	}
	else
	{
		pCheckBox4->SetValue(FALSE);
		pStatic4[6]->SetText("&CfgPtn.UnStarted");
	}
	
	if(p2p_registered_flag == 'R')
	{
		pButton4->Enable(FALSE);
	}
	else
	{
		pButton4->Enable(TRUE);
	}
	
	ShowSubPage(4,0,FALSE);
}
#endif

void CPageNetworkConfig::ShowRegistButton( void )
{
	#ifdef FIX_DDNS_BUG
	int flag = pCheckBox3->GetValue();
	#endif
	
	//if(0 == pComboBox3[0]->GetCurSel())
	EM_BIZ_DDNS_PROT prot=(EM_BIZ_DDNS_PROT)(GetProtocolValue(pComboBox3[0]->GetCurSel())+1);//cw_ddns
	if(prot==EM_BIZDDNS_PROT_TL ||prot==EM_BIZDDNS_PROT_DVRNET || prot==EM_BIZDDNS_PROT_JMDVR || prot==EM_BIZDDNS_PROT_JSJDVR)
	{
		#ifdef FIX_DDNS_BUG
		if(flag)
		{
			pButton3->Enable(TRUE);
		}
		else
		{
			pButton3->Enable(FALSE);
		}
		#else
		pButton3->Enable(TRUE);
		#endif
	}
	else
	{
		pButton3->Enable(FALSE);
	}
	
	#ifdef FIX_DDNS_BUG
	if(flag)
	{
		if(prot==EM_BIZDDNS_PROT_JMDVR)
		{
			pEdit3[2]->Enable(FALSE);
		}
		else
		{
			pEdit3[2]->Enable(TRUE);
		}
	}
	else
	{
		pEdit3[2]->Enable(FALSE);
	}
	#else
	//csp modify 20130322
	if(prot==EM_BIZDDNS_PROT_JMDVR)
	{
		pEdit3[2]->Enable(FALSE);
	}
	else
	{
		pEdit3[2]->Enable(TRUE);
	}
	#endif
}

void CPageNetworkConfig::SwitchPage(int mainID,int subID)
{
	if(mainID<0 || mainID>=NETWORKCFG_SUBPAGES)
	{
		return;
	}
	
	//printf("curID = %d, mainId = %d cursubindex = %d subID %d\n",curID,mainID, items[curID].cursubindex, subID);
	
	if((mainID == curID) && (subID == items[curID].cursubindex))
	{
		return;
	}
	
	//printf("###11curID = %d\n",curID);
	//printf("###11subID = %d\n",items[curID].cursubindex);
	
	//隐藏当前子页面，显示新的子页面
	ShowSubPage(curID,items[curID].cursubindex,FALSE);
	pButton[curID]->Enable(TRUE);
	
	if(curID != mainID)
	{
		curID = mainID;
		LoadPara(mainID);
	}
	items[curID].cursubindex = subID;
	
	//printf("###22curID = %d \n",curID);
	//printf("###22subID = %d \n",items[curID].cursubindex);
	
	pButton[mainID]->Enable(FALSE);
	ShowSubPage(mainID, subID, TRUE);
}

void CPageNetworkConfig::ShowSubPage( int mainID, int subID, BOOL bShow )
{
	if(mainID<0 || mainID>=NETWORKCFG_SUBPAGES)
	{
		return;
	}
	
	int count = items[mainID].SubItems[subID].size();
	//printf("CPageNetworkConfig mainID=%d, subID=%d, count=%d\n", mainID, subID, count);
	
	int i = 0;
	for(i=0; i<count; i++)
	{
		if(items[mainID].SubItems[subID][i])
		{
			items[mainID].SubItems[subID][i]->Show(bShow);
		}
		else
		{
			printf("CPageNetworkConfig mainID=%d, subID=%d, i=%d item is NULL\n", mainID, subID, i);
		}
	}
	
	if(mainID == 3)
		ShowRegistButton();
	
	/*
	if(mainID == 3)
		pButton3->Enable(pCheckBox3->GetValue());
	*/
	
	//else if(mainID == 0 && subID == 1)
	//	pButton01->Enable(pCheckBox01->GetValue());
    if(!m_IsLoader)
    {
        pStatus->SetText("");
    }
    
	AdjHide();
	
	//printf("ShowSubPage 2222\n");
}

void CPageNetworkConfig::LoadPara( int curID ) // 0 common netpara 1 stream para
{
	//get network config
	
	//switch(curID)
	if(curID != 1)
	{
		SGuiNetPara sPIns;
		
		memset(&sPIns, 0, sizeof(sPIns));
		bizData_GetNetPara( &sPIns );
		
		char szTmp[10] = {0};
		sprintf(szTmp, "%d", sPIns.HttpPort);
		pEdit00[0]->SetText(szTmp);
		sprintf(szTmp, "%d", sPIns.TCPPort);
		pEdit00[1]->SetText(szTmp);
		sprintf(szTmp, "%d", sPIns.MobilePort);
		pEdit00[2]->SetText(szTmp);
		pCheckBox00->SetValue(sPIns.DhcpFlag);
		
		IPADDR sIPIns;
		
		sIPIns.l = sPIns.HostIP;
		pIpCtrl00[0]->SetIPAddress(&sIPIns);
		sIPIns.l = sPIns.Submask;
		pIpCtrl00[1]->SetIPAddress(&sIPIns);
		sIPIns.l = sPIns.GateWayIP;
		pIpCtrl00[2]->SetIPAddress(&sIPIns);
		sIPIns.l = sPIns.DNSIP;
		pIpCtrl00[3]->SetIPAddress(&sIPIns);
		sIPIns.l = sPIns.DNSIPAlt;
		pIpCtrl00[4]->SetIPAddress(&sIPIns);
		
		pCheckBox01->SetValue(sPIns.PPPOEFlag);
		pButton01->Enable(pCheckBox01->GetValue());
		pEdit01[0]->SetText((char*)sPIns.PPPOEUser);
		pEdit01[1]->SetText((char*)sPIns.PPPOEPasswd);
		
		//csp modify 20130321
		pCheckBoxupnp[0]->SetValue((sPIns.UPNPFlag)?1:0);
		
		LoadMailCfg(&sPIns);
		
		pCheckBox3->SetValue(sPIns.DDNSFlag);
		if(sPIns.DDNSFlag > 0)
		{
		#ifdef FIX_DDNS_BUG
			u8 index = 0;
			BizConfigGetParaListIndex(EM_BIZ_CFG_PARALIST_DDNSDOMAIN, sPIns.DDNSFlag-1, &index);
			m_nLastSel = index;
			
			pComboBox3[0]->Enable(TRUE);
			pEdit3[0]->Enable(TRUE);
			pEdit3[1]->Enable(TRUE);
			pEdit3[2]->Enable(TRUE);
		#else
			m_nLastSel = sPIns.DDNSFlag-1;
		#endif
		}
		else
		{
		#ifdef FIX_DDNS_BUG
			pComboBox3[0]->Enable(FALSE);
			pEdit3[0]->Enable(FALSE);
			pEdit3[1]->Enable(FALSE);
			pEdit3[2]->Enable(FALSE);
		#endif
			m_nLastSel = 0;
		}
		
		pComboBox3[0]->SetCurSel(m_nLastSel);
		
		if(strcmp((char*)sPIns.DDNSUser,"0") == 0)
			pEdit3[0]->SetText("");
		else
			pEdit3[0]->SetText((char*)sPIns.DDNSUser);
		
		if(strcmp((char*)sPIns.DDNSPasswd,"0") == 0)
			pEdit3[1]->SetText("");
		else
			pEdit3[1]->SetText((char*)sPIns.DDNSPasswd);
		
		if(strcmp((char*)sPIns.DDNSDomain,"0") == 0)
			pEdit3[2]->SetText("");
		else
			pEdit3[2]->SetText((char*)sPIns.DDNSDomain);
		
		pComboBox3[1]->SetCurSel(sPIns.UpdateIntvl);
	}
	else//子码流
	{
		for(int i=0; i<nChMax; i++)
		{
			bizData_GetNetStreamPara( i, &pTabPara[i] );	
		}
		
		// set gui ctrls
		int nChBase = nCurPage*nRealRow;
		
		for(int i=0; i<nRealRow; i++)
		{
			pComboBox1[i]->SetCurSel(pTabPara[nChBase+i].nVideoResolution);
			pComboBox1[i+nRealRow]->SetCurSel(pTabPara[nChBase+i].nFrameRate);
			pComboBox1[i+nRealRow*2]->SetCurSel(pTabPara[nChBase+i].nBitRateType);
			
			//csp modify
			//if(bizData_IsCbr(pComboBox1[i+nRealRow*2]->GetString(pTabPara[nChBase+i].nBitRateType)))
			if(pTabPara[nChBase+i].nBitRateType == 0)
			{
				pComboBox1[i+nRealRow*3]->Enable(FALSE);
			}
			else
			{
				pComboBox1[i+nRealRow*3]->Enable(TRUE);
			}
			
			pComboBox1[i+nRealRow*3]->SetCurSel(pTabPara[nChBase+i].nPicLevel);
			pComboBox1[i+nRealRow*4]->SetCurSel(pTabPara[nChBase+i].nBitRate);
		}
		
		for(int i=0; i<5; i++)
		{
			pComboBox1[5*nRealRow+i]->SetCurSel(pComboBox1[nRealRow*i]->GetCurSel());
		}
		
		//csp modify
		//if(bizData_IsCbr(pComboBox1[2+nRealRow*5]->GetString(pComboBox1[2*nRealRow]->GetCurSel())))
		if(pComboBox1[2*nRealRow]->GetCurSel() == 0)
		{
			pComboBox1[3+nRealRow*5]->Enable(FALSE);
		}
		else
		{
			pComboBox1[3+nRealRow*5]->Enable(TRUE);
		}
	} 
}

// enable/disable ctrls
void CPageNetworkConfig::CtrlEnable(VD_BOOL flag)
{
	
}

VD_BOOL CPageNetworkConfig::UpdateData( UDM mode )
{
	if(UDM_OPEN == mode)
	{
		LoadPara(0);
		LoadPara(1);
		UpdateDomain();
		
		//printf("##curid %d\n", curID);
		
		if(curID == 3)
		{
			ShowRegistButton();
		}
        
        LoadButtomCtl();
	}
    else if(UDM_CLOSED == mode)
    {
        if(m_IsLoader)
        {
            m_IsLoader = 0;//~m_IsLoader;
            LoadButtomCtl();
            SetTitle(GetParsedString("&CfgPtn.Network"));
        }
        else
        {
            ((CPageConfigFrameWork *)(this->GetParent()))->ReflushItemName();
        }
    }
    else if(UDM_GUIDE == mode)
    {
        m_IsLoader = 1;//~m_IsLoader;
        SetTitle(GetParsedString("&CfgPtn.Guide"));
    }
	
	return TRUE;
}

#if 1
void CPageNetworkConfig::SetDynNetPara(uint ip, uint submsk, uint gw, uint dns1)
{
	IPADDR sIPIns;

	sIPIns.l = ip;
	pIpCtrl00[0]->SetIPAddress(&sIPIns);
	sIPIns.l = submsk;
	pIpCtrl00[1]->SetIPAddress(&sIPIns);
	sIPIns.l = gw;
	pIpCtrl00[2]->SetIPAddress(&sIPIns);
	sIPIns.l = dns1;
	pIpCtrl00[3]->SetIPAddress(&sIPIns);	
}
#endif

void CPageNetworkConfig::SwitchDHCP()
{
	if(pCheckBox00->GetValue())
		pCheckBox01->SetValue(0);
}

void CPageNetworkConfig::MessgeBox(int index)
{
    //pEdit00[index]->SetText("");
    char s[32] = {0};
    if (0 == index) 
    {
        sprintf(s, "%s%s: 1-65535", GetParsedString("&CfgPtn.HTTPPort"), GetParsedString("&CfgPtn.Range"));
    }
    else if (1 == index) 
    {
        sprintf(s, "%s%s: 1-65535", GetParsedString("&CfgPtn.ServerPort"), GetParsedString("&CfgPtn.Range"));
    }
    else if (2 == index)
    {
        sprintf(s, "%s%s: 1-65535", GetParsedString("&CfgPtn.MobilePort"), GetParsedString("&CfgPtn.Range"));
    }
    
    MessageBox(s, "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);    
    //pIpCtrl00[index]->Show(FALSE, FALSE);
    //pIpCtrl00[index]->Show(TRUE, TRUE);
    //pIpCtrl00[index]->Draw();
}

void CPageNetworkConfig::ClearInfo()
{
    if (m_IsLoader)
    {
        return;
    }
    
	pStatus->SetText("");
}

VD_BOOL CPageNetworkConfig::MsgProc( uint msg, uint wpa, uint lpa )
{
	u8 lock_flag = 0;//cw_shutdown
	GetSystemLockStatus(&lock_flag);
	if(lock_flag)
	{
		return FALSE;
	}
	int px,py;
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
    	case XM_MOUSEWHEEL://cw_scrollbar
    		{
                if (m_IsLoader)
                {
                    return TRUE;
                }
                
    			int px = VD_HIWORD(lpa);
    			int py = VD_LOWORD(lpa);
    			CItem* pItem = GetItemAt(px, py);
    			if(pItem == NULL)
    			{
    				if (curID == 0)
    				{
    					pScrollbar00->MsgProc(msg, wpa, lpa);
    					break;
    				}
    				if(GetMaxChnNum()>TabSzNetStream[0].nRow)
    				{
    					if (curID == 1)
    					{
    						pScrollbar->MsgProc(msg, wpa, lpa);
    						break;
    					}
    				}
    			}
    			break;
    		}
    	case XM_MOUSEMOVE:
    		{
    		/*
    			px = VD_HIWORD(lpa);
    			py = VD_LOWORD(lpa);
    			static CItem* last = NULL;
    			CItem* temp = GetItemAt(px, py);

    			if(temp != last)
    			{
    				ClearInfo();

    				last = temp;
    			}
    			*/
    		}

    		break;
    	default:
    		
    		break;
    }

	return CPage::MsgProc(msg, wpa, lpa);
}
			
void CPageNetworkConfig::LoadMailCfg(SGuiNetPara* pCfg)
{
	char szTmp[30] = {0};
	
	//pEdit20[0]->SetText(bizData_inet_ntoa(sPIns.SMTPServerIP));
	if(strlen(pCfg->sAdancePara.szSmtpServer)>WWW_ADD_LEN_MIN)
		pEdit20[0]->SetText(pCfg->sAdancePara.szSmtpServer);
	else
		pEdit20[0]->SetText("");

	sprintf(szTmp, "%d", pCfg->SMTPServerPort);
	pEdit20[1]->SetText(szTmp);
	if(strlen(pCfg->sAdancePara.szSendMailBox)>WWW_ADD_LEN_MIN)
		pEdit20[2]->SetText(pCfg->sAdancePara.szSendMailBox);
	else
		pEdit20[2]->SetText("");

	if(strlen(pCfg->sAdancePara.szSMailPasswd)>1)
		pEdit20[3]->SetText(pCfg->sAdancePara.szSMailPasswd);
	else
		pEdit20[3]->SetText("");
	if(strlen(pCfg->sAdancePara.szReceiveMailBox)>WWW_ADD_LEN_MIN)
		pEdit20[4]->SetText(pCfg->sAdancePara.szReceiveMailBox);
	else
		pEdit20[4]->SetText("");
	if(strlen(pCfg->sAdancePara.szReceiveMailBox2)>WWW_ADD_LEN_MIN)
		pEdit20[5]->SetText(pCfg->sAdancePara.szReceiveMailBox2);
	else
		pEdit20[5]->SetText("");
	if(strlen(pCfg->sAdancePara.szReceiveMailBox3)>WWW_ADD_LEN_MIN)
		pEdit20[6]->SetText(pCfg->sAdancePara.szReceiveMailBox3);
	else
		pEdit20[6]->SetText("");	
}

