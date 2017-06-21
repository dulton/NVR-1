#include "GUI/Pages/PageModifyUser.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageUserManager.h"
#include "GUI/Pages/PageMessageBox.h"

//csp modify 20130519
static int GetMaxChnNum()
{
	return GetVideoMainNum();
}

static	MAINFRAME_ITEM	items[ADDUSER_SUBPAGES];

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static VD_BITMAP * pBmp_tab_normal;
static VD_BITMAP * pBmp_tab_push;
static VD_BITMAP * pBmp_tab_select;

CPageModifyUser::CPageModifyUser( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
, curID(0),lastPos(0)
{
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
	
	char* szSubPgName[ADDUSER_BTNNUM] = {
		"&CfgPtn.General",
		"&CfgPtn.Authority",
		"&CfgPtn.OK",
		"&CfgPtn.Exit",
	};
	int szSubPgNameLen[ADDUSER_BTNNUM] = {
		TEXT_WIDTH*4,//"&CfgPtn.General",
		TEXT_WIDTH*4,//"&CfgPtn.Authority",
		TEXT_WIDTH*2,//"&CfgPtn.OK",
		TEXT_WIDTH*2,//"&CfgPtn.Exit",
	};

	SetMargin(0, m_TitleHeight, 0, m_TitleHeight);
	
	CTableBox* pTabFrame = CreateTableBox(CRect(17, 40, m_Rect.Width()-17, 40+290),this,1,1);
	//pTabFrame->SetFrameColor(VD_RGB(56,108,148));
	pTabFrame->SetFrameColor(VD_RGB(8,66,189));
	
	CRect rtSub(25, 10, m_Rect.Width()-20, 10+pBmp_tab_normal->height);
	int i;
	for (i = 0; i<ADDUSER_SUBPAGES; i++)
	{
		int btWidth = szSubPgNameLen[i]+10;
		rtSub.right = rtSub.left + btWidth;
		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageModifyUser::OnClickSubPage, NULL, buttonNormalBmp, TRUE);
		pButton[i]->SetBitmap(pBmp_tab_normal, pBmp_tab_select, pBmp_tab_push, pBmp_tab_push);
		rtSub.left = rtSub.right;
	}

	CRect rtSub1(295, m_Rect.Height()-80, m_Rect.Width()-20, m_Rect.Height()-80+pBmp_button_normal->height);
	rtSub1.left = rtSub1.right;
	for (i=ADDUSER_SUBPAGES; i<ADDUSER_BTNNUM; i++)
	{
		rtSub1.left -= szSubPgNameLen[i]+10;
	}
	rtSub1.left -= 80;
	rtSub1.left += 3;

	for(i=ADDUSER_SUBPAGES; i<ADDUSER_BTNNUM; i++)
	{
		rtSub1.right = rtSub1.left + 80;
		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageModifyUser::OnClickBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
		rtSub1.left = rtSub1.right+12;
	}
	
	//CRect rtSub2(13, 35, m_Rect.Width()-18, 300);
	//pTable = CreateTableBox(&rtSub2,this, 1,1,0);
	
	InitPage0();
	InitPage1();
	
	//InitPage10();
	//InitPage11();
}

void CPageModifyUser::OnClickBtn()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 2; i < 4; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		switch(i)
		{
			case 2: //OK
			{
				SBizUserUserPara para;
				memset(&para, 0, sizeof(para));
				strcpy((char*)para.UserName, m_username);
				
				SBizParaTarget pTarget;
				pTarget.emBizParaType = EM_BIZ_USER_GETUSERINFO;
				if(0 != BizGetPara(&pTarget, &para))
				{
					printf("Get user info failed\n");
					break;
				}
				
				strcpy((char*)para.GroupBelong, pGroupList[pComboBox0[0]->GetCurSel()]);
				para.emIsBindPcMac = pCheckBox0[0]->GetValue();
				
				int n = 0;
				int j = 0;
				int num = 0;
				char tmp[20];
				char mac[2] = {0};
				memset(tmp, 0, sizeof(tmp));
				
				for(num = 0; num < PG_0_STATICMACNUM + 1; num++)
				{
					pEdit0[num]->GetText(mac,2);
					for(int i = 0; i<2; i++)
					{
						if(((mac[i] >= '0') && (mac[i] <= '9')) 
							|| ((mac[i] >= 'a') && (mac[i] <= 'f'))
							|| ((mac[i] >= 'A') && (mac[i] <= 'F')))
						{
							tmp[n] = mac[i];
							n++;
						}
						else
						{
							j = 1;
						}
					}
				}
				
				tmp[19] = '\0';
				if(j)
				{
					MessageBox("&CfgPtn.InvalidMacAddr", "&CfgPtn.WARNING" , MB_OK|MB_ICONWARNING);
					break;
				}
				sscanf(tmp, "%012llx", &para.PcMacAddress);
				
				para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_USER_MANAGE] = 0;
				if(0 == strcasecmp("Administrator", (char*)para.GroupBelong))
				{
					para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_USER_MANAGE] = 1;
				}
				
				para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_LOG_VIEW] = pCheckBox10[6]->GetValue();
				para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_SETUP] = pCheckBox10[0]->GetValue();
				para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SHUTDOWN] = pCheckBox10[11]->GetValue();
				para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_AUDIO_TALK] = pCheckBox10[7]->GetValue();
				para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_DATA_MANAGE] = pCheckBox10[5]->GetValue();
				para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_DISK_MANAGE] = pCheckBox10[4]->GetValue();
				para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_REMOTE_LOGIN] = pCheckBox10[8]->GetValue();
				para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_UPDATE] = pCheckBox10[10]->GetValue();
				para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_RECORD] = pCheckBox10[1]->GetValue();
				para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_PLAYBACK] = pCheckBox10[2]->GetValue();
				para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_RESET] = pCheckBox10[9]->GetValue();
				para.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_PTZCTRL] = pCheckBox10[3]->GetValue();
				
				memset(&para.UserAuthor.nLiveView[0], 0xff, 4*8*6);
				/*
				for(int j=0; j < 4; j++)
				{
					para.UserAuthor.nLiveView[j] = 0xffffffffffffffff;
					para.UserAuthor.nRecord[j] = 0xffffffffffffffff;
					para.UserAuthor.nPlayback[j] = 0xffffffffffffffff;
					para.UserAuthor.nBackUp[j] = 0xffffffffffffffff;
					para.UserAuthor.nPtzCtrl[j] = 0xffffffffffffffff;
					para.UserAuthor.nRemoteView[j] = 0xffffffffffffffff;
				}*/
				
				#if 1//csp modify 20130519
				{
					memset(&para.UserAuthor.nRemoteView[0], 0x00, 4*8);
					
					int j = 0;
					int num = 0;
					
					num = GetMaxChnNum();
					for(j=0; j<num+(num+7)/8; j++)
					{
						if((j%9) == 0)
						{
							continue;
						}
						
						if(pCheckBox10[13+j]->GetValue())
						{
							para.UserAuthor.nRemoteView[0] |= (1 << (j-j/9-1));
						}
					}
				}
				#endif
				
				pTarget.emBizParaType = EM_BIZ_USER_MODIFYUSER;
				if(0 != BizSetPara(&pTarget, &para))
				{
					printf("modify user failed\n");
					break;
				}
				
				this->Close();
				
				CPage** page = GetPage();
				CPageUserManager* pUserMgr = (CPageUserManager*)page[EM_PAGE_USERMANAGE];
				pUserMgr->UpdateData(UDM_OPENED);
				
				printf("ok\n");
				
				BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG, BIZ_LOG_SLAVE_CHANGE_USER_AUTH);
				
				//printf("curid = %d\n",curID);
			}break;
			case 3: //Exit
			{
				//printf("exit\n");
				this->Close();
			}
			default:
				break;
		}
	}
}

void CPageModifyUser::OnClickSubPage()
{
	//printf("OnClickSubPage\n");
	
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < ADDUSER_BTNNUM; i++)
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
		
		switch(i)
		{
			case 0:
			case 1:
				SwitchPage(i,items[i].cursubindex);
				break;
			case 2:
				printf("ok\n");
				break;
			case 3:
				this->Close();
				break;
			default:
				break;
		}
	}
}

CPageModifyUser::~CPageModifyUser()
{
	
}

VD_PCSTR CPageModifyUser::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

void CPageModifyUser::InitPage0()
{
	//csp modify
	char* statictext[PG_MODIFYUSER_0_STATICNUM] = {
		"&CfgPtn.Username",
		"&CfgPtn.UserType",
		"&CfgPtn.BindingPCMACAddress",
		"&CfgPtn.PCMACADDRESS",
		"",
	};
	
	CRect rtSubPage(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.Width()-27,m_Rect.top+50+270);
	
	pTable0 = CreateTableBox(&rtSubPage,this, 2,9,0);
	//pTable0->SetColWidth(0,294);
	items[0].SubItems[0].push_back(pTable0);
	
	CRect tmpRt;
	int i = 0;
	for(i=0; i<PG_MODIFYUSER_0_STATICNUM-1; i++)//csp modify
	{
		pTable0->GetTableRect(0,i,&tmpRt);
		pStatic0[i] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+1, rtSubPage.top+tmpRt.top+1, rtSubPage.left+tmpRt.right-1,rtSubPage.top+tmpRt.bottom-1), this, statictext[i]);
		items[0].SubItems[0].push_back(pStatic0[i]);
	}
	
	pTable0->GetTableRect(1,0,&tmpRt);
	pStatic0[4] = CreateStatic(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), this, statictext[4]);
	items[0].SubItems[0].push_back(pStatic0[4]);
	pStatic0[4]->SetBkColor(VD_RGB(67,77,87));
	
	
	pTable0->GetTableRect(1,1,&tmpRt);	
	pComboBox0[0] = CreateComboBox(CRect(rtSubPage.left+tmpRt.left+2, rtSubPage.top+tmpRt.top+1, 
		rtSubPage.left+tmpRt.right-2,rtSubPage.top+tmpRt.bottom-1), 
		this, NULL, NULL, NULL, 0);
	pComboBox0[0]->SetBkColor(VD_RGB(67,77,87));
	items[0].SubItems[0].push_back(pComboBox0[0]);
	
	
	static u8 flag = 0;
	if(flag == 0)
	{
		pGroupList = (char**)calloc(16, sizeof(char*));
		for(int i = 0; i < 16; i++)
		{
			pGroupList[i] = (char*)calloc(15,1);
		}
		flag = 1;
	}
	
	SBizParaTarget pTarget;
	pTarget.emBizParaType = EM_BIZ_USER_GETGROUPLIST;
	
	SBizUserListPara para;
	para.list = pGroupList;
	para.nLength = 16;
	
	if(0 != BizGetPara(&pTarget, &para))
	{
		printf("Get Group list failed\n");
		
		nRealGroupNum = 0;
	}
	else
	{
		nRealGroupNum = para.nRealLength;
		
		if(para.nLength<nRealGroupNum)
		{
			nRealGroupNum = para.nLength;
			printf("nRealGroupNum :%d\n", nRealGroupNum);
		}
	}
	
	for(int j = 0; j < nRealGroupNum; j++)
	{
		//pComboBox0[0]->AddString(pGroupList[j]);
		if(!strcasecmp(pGroupList[j],"Administrator"))
		{
			pComboBox0[0]->AddString("&CfgPtn.AdministratorUser");
		}
		else
		{
			pComboBox0[0]->AddString("&CfgPtn.GuestUser");
		}
	}
	
	pTable0->GetTableRect(1,2,&tmpRt);
	pCheckBox0[0] = CreateCheckBox(CRect(rtSubPage.left+tmpRt.left+1, rtSubPage.top+tmpRt.top+4, 
								rtSubPage.left+tmpRt.left+21,rtSubPage.top+tmpRt.top+24), 
								this);
	items[0].SubItems[0].push_back(pCheckBox0[0]);
	
	
	pTable0->GetTableRect(1,3,&tmpRt);
	int vLen = (tmpRt.right-tmpRt.left)/TEXT_WIDTH*2;
	
	tmpRt.left = rtSubPage.left + tmpRt.left + 2;
	tmpRt.top = rtSubPage.top + tmpRt.top + 1;
	tmpRt.right = tmpRt.left + ROW_HEIGHT + 2;
	tmpRt.bottom = rtSubPage.top + tmpRt.bottom - 1;
	for(int i = 0; i < PG_MODIFYUSER_0_EDITNUM; i++)//csp modify
	{
		pEdit0[i] = CreateEdit(tmpRt, this,2,edit_KI_NOCH,NULL);;
		pEdit0[i]->SetBkColor(VD_RGB(67,77,87));
		items[0].SubItems[0].push_back(pEdit0[i]);

		tmpRt.left = tmpRt.right;
		tmpRt.right = tmpRt.left + 15;
		if(i < PG_0_STATICMACNUM)
		{
			pStaticMac[i] = CreateStatic(tmpRt, this, ":");
			pStaticMac[i]->SetText(":");
			pStaticMac[i]->SetTextAlign(VD_TA_CENTER);
			items[0].SubItems[0].push_back(pStaticMac[i]);

			tmpRt.left = tmpRt.right;
			tmpRt.right = tmpRt.left + ROW_HEIGHT + 2;
		}
	}
	items[0].cursubindex = 0;

	//pEdit0[0]->SetText("00");
	//pEdit0[0]->Enable(FALSE);

	ShowSubPage(0,0,TRUE);
	pButton[0]->Enable(FALSE);
}


void CPageModifyUser::OnTrackMove()
{
	int pos = pScrollbar10->GetPos();
	if(lastPos<=5 && pos>5)
	{
		//printf("pos = %d\n",pos);
		SwitchPage(1,1);
		lastPos = pos;
	}
	
	if(lastPos>=5 && pos<5)
	{
		//printf("pos = %d\n",pos);
		SwitchPage(1,0);
		lastPos = pos;
	}
}

#if 1//csp modify 20130519
void CPageModifyUser::InitPage1()
{
	memset(pCheckBox10,0,sizeof(pCheckBox10));
	memset(pStatic10,0,sizeof(pStatic10));
	
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.Width()-27,m_Rect.top+50+270);
	
	pTable10 = CreateTableBox(&rtTmp,this, 1,1,0);
	items[1].SubItems[0].push_back(pTable10);
	
	char* szStatic[] = {
		"&CfgPtn.SystemSetup",
		"&CfgPtn.ManualRecord",
		"&CfgPtn.Playback",
		"&CfgPtn.PTZControl",
		"&CfgPtn.DiskManagement",
		"&CfgPtn.FileManagement",
		"&CfgPtn.LogSearch",
		"&CfgPtn.TwoWayAudio",
		"&CfgPtn.RemoteLogin",
		"&CfgPtn.SystemReset",
		"&CfgPtn.SystemUpgrade",
		"&CfgPtn.ShuwDown",
		
		//csp add 20130519
		"&CfgPtn.RemoteLiveView",
	};
	
	rtTmp.left = 30;
	rtTmp.top = 65;
	rtTmp.right = rtTmp.left+20;
	rtTmp.bottom = rtTmp.top+23;
	int i = 0;
	int j = 0;
	for(i=0; i<4; i++)
	{
		for(j=0; j<3; j++)
		{
			pCheckBox10[i*3+j] = CreateCheckBox(rtTmp, this);
			items[1].SubItems[0].push_back(pCheckBox10[i*3+j]);
			
			rtTmp.left = rtTmp.right+5;
			rtTmp.right = rtTmp.left+140;
			
			pStatic10[i*3+j] = CreateStatic(rtTmp, this, szStatic[i*3+j]);
			items[1].SubItems[0].push_back(pStatic10[i*3+j]);
			
			rtTmp.left = rtTmp.right+30;
			rtTmp.right = rtTmp.left+20;
		}
		
		rtTmp.left = 30;
		rtTmp.top = rtTmp.bottom + 10;//15;
		rtTmp.right = rtTmp.left+20;
		rtTmp.bottom = rtTmp.top+23;
	}
	
	rtTmp.top += 16;
	rtTmp.bottom += 16;
	
	pCheckBox10[12] = CreateCheckBox(rtTmp, this);
	//items[1].SubItems[0].push_back(pCheckBox10[12]);
	pCheckBox10[12]->Show(FALSE);
	
	//rtTmp.left = rtTmp.right+5;
	rtTmp.right = rtTmp.left+150;
	
	pStatic10[12] = CreateStatic(rtTmp, this, szStatic[12]);
	items[1].SubItems[0].push_back(pStatic10[12]);
	
	rtTmp.left = 30;
	rtTmp.top = rtTmp.bottom + 5;
	rtTmp.bottom = rtTmp.top + 30;
	if(GetMaxChnNum() == 4)
	{
		rtTmp.right = rtTmp.left + 58*5;
		pTable11 = CreateTableBox(&rtTmp,this,5,1,0);
	}
	else if(GetMaxChnNum() < 8)
	{
		rtTmp.right = rtTmp.left + 58*(GetMaxChnNum()+1);
		pTable11 = CreateTableBox(&rtTmp,this,GetMaxChnNum()+1,1,0);
	}
	else
	{
		rtTmp.right = rtTmp.left + 58*9;
		pTable11 = CreateTableBox(&rtTmp,this,9,1,0);
	}
	items[1].SubItems[0].push_back(pTable11);
	
	rtTmp.left = 40;
	rtTmp.top = rtTmp.top + 5;
	rtTmp.right = rtTmp.left+20;
	rtTmp.bottom = rtTmp.top+20;
	
	int idx_end = 13 + 9;
	if(GetMaxChnNum() == 4)
	{
		idx_end = 13 + 5;
	}
	else if(GetMaxChnNum() < 8)
	{
		idx_end = 13 + GetMaxChnNum() + 1;
	}
	else
	{
		idx_end = 13 + 9;
	}
	for(i=13; i<idx_end; i++)
	{
		CRect tmpRt;
		pTable11->GetTableRect(i-13,0,&tmpRt);
		//printf("###<%d,%d>###\n",tmpRt.left,tmpRt.right);
		
		rtTmp.left = tmpRt.left + 4 + 30;
		rtTmp.right = rtTmp.left + 20;
		
		if(i == 13)
		{
			pCheckBox10[i] = CreateCheckBox(rtTmp, 
											this, styleEditable, 
											(CTRLPROC)&CPageModifyUser::OnCheckBoxSelectAll);
		}
		else
		{
			pCheckBox10[i] = CreateCheckBox(rtTmp, this, styleEditable, (CTRLPROC)&CPageModifyUser::OnCheckBoxSelectItem);
		}
		
		items[1].SubItems[0].push_back(pCheckBox10[i]);
		
		rtTmp.left = rtTmp.right + 4;
		rtTmp.right = rtTmp.left + 24;
		
		char szText[16];
		if(i == 13)
		{
			sprintf(szText,"");
		}
		else
		{
			sprintf(szText,"%d",i-13);
		}
		pStatic10[i] = CreateStatic(rtTmp, this, szText);
		items[1].SubItems[0].push_back(pStatic10[i]);
	}
	
	if(GetMaxChnNum() > 8)
	{
		rtTmp.left = 30;
		rtTmp.top = rtTmp.bottom + 10;
		rtTmp.bottom = rtTmp.top + 30;
		rtTmp.right = rtTmp.left + 58*9;
		pTable12 = CreateTableBox(&rtTmp,this,9,1,0);
		items[1].SubItems[0].push_back(pTable12);
		
		rtTmp.top = rtTmp.top + 5;
		rtTmp.bottom = rtTmp.top+20;
		
		if(GetMaxChnNum() > 16)
		{
			idx_end = 22 + 9;
		}
		else
		{
			idx_end = 22 + (GetMaxChnNum()-8) + 1;
		}
		for(i=22; i<idx_end; i++)
		{
			CRect tmpRt;
			pTable12->GetTableRect(i-22,0,&tmpRt);
			//printf("###<%d,%d>###\n",tmpRt.left,tmpRt.right);
			
			rtTmp.left = tmpRt.left + 4 + 30;
			rtTmp.right = rtTmp.left + 20;
			
			if(i == 22)
			{
				pCheckBox10[i] = CreateCheckBox(rtTmp, 
												this, styleEditable, 
												(CTRLPROC)&CPageModifyUser::OnCheckBoxSelectAll);
			}
			else
			{
				pCheckBox10[i] = CreateCheckBox(rtTmp, this, styleEditable, (CTRLPROC)&CPageModifyUser::OnCheckBoxSelectItem);
			}
			
			items[1].SubItems[0].push_back(pCheckBox10[i]);
			
			rtTmp.left = rtTmp.right + 4;
			rtTmp.right = rtTmp.left + 24;
			
			char szText[16];
			if(i == 22)
			{
				sprintf(szText,"");
			}
			else
			{
				sprintf(szText,"%d",i-22+8);
			}
			pStatic10[i] = CreateStatic(rtTmp, this, szText);
			items[1].SubItems[0].push_back(pStatic10[i]);
		}
	}
	
	
	items[1].cursubindex = 0;
	ShowSubPage(1,0,FALSE);
}
#else
void CPageModifyUser::InitPage1()
{
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.Width()-27,m_Rect.top+50+270);
	
	pTable10 = CreateTableBox(&rtTmp,this, 1,1,0);
	items[1].SubItems[0].push_back(pTable10);
	
	char* szStatic[12] = {
		"&CfgPtn.SystemSetup",
		"&CfgPtn.ManualRecord",
		"&CfgPtn.Playback",
		"&CfgPtn.PTZControl",
		"&CfgPtn.DiskManagement",
		"&CfgPtn.FileManagement",
		"&CfgPtn.LogSearch",
		"&CfgPtn.TwoWayAudio",
		"&CfgPtn.RemoteLogin",
		"&CfgPtn.SystemReset",
		"&CfgPtn.SystemUpgrade",
		"&CfgPtn.ShuwDown",
	};
	
	rtTmp.left = 30;
	rtTmp.top = 65;
	rtTmp.right = rtTmp.left+20;
	rtTmp.bottom = rtTmp.top+23;
	int i = 0;
	int j = 0;
	for(i=0; i<4; i++)
	{
		for(j=0; j<3; j++)
		{
			pCheckBox10[i*3+j] = CreateCheckBox(rtTmp, this);
			items[1].SubItems[0].push_back(pCheckBox10[i*3+j]);
			
			rtTmp.left = rtTmp.right+5;
			rtTmp.right = rtTmp.left+140;
			
			pStatic10[i*3+j] = CreateStatic(rtTmp, this, szStatic[i*3+j]);
			items[1].SubItems[0].push_back(pStatic10[i*3+j]);
			
			rtTmp.left = rtTmp.right+30;
			rtTmp.right = rtTmp.left+20;
		}
		
		rtTmp.left = 30;
		rtTmp.top = rtTmp.bottom + 15;
		rtTmp.right = rtTmp.left+20;
		rtTmp.bottom = rtTmp.top+23;
	}
	
	items[1].cursubindex = 0;
	ShowSubPage(1,0,FALSE);
}
#endif

void CPageModifyUser::InitPage10()
{
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
	m_Rect.Width()-27,m_Rect.top+50+270);

	pTable10 = CreateTableBox(&rtTmp,this, 1,1,0);
	//pTable0->SetColWidth(0,294);
	items[1].SubItems[0].push_back(pTable10);


	rtTmp.left = m_Rect.Width()-30-20;
	rtTmp.top = 53;
	rtTmp.right = rtTmp.left + 20;
	rtTmp.bottom = rtTmp.top+264;
	pScrollbar10 = CreateScrollBar(rtTmp, this,scrollbarY,
								   0,100, 1, (CTRLPROC)&CPageModifyUser::OnTrackMove);

	items[1].SubItems[0].push_back(pScrollbar10);	

	char* szStatic[20] = {
		"&CfgPtn.LogSearch",
		"&CfgPtn.SystemSetup",
		"&CfgPtn.ShuwDown",
		"&CfgPtn.TwoWayAudio",
		"&CfgPtn.FileManagement",
		"&CfgPtn.DiskManagement",
		"&CfgPtn.RemoteLogin",
		"&CfgPtn.LiveView",
		"",
		"1",
		"2",
		"3",
		"4",
		"&CfgPtn.ManualRecord",
		"",
		"1",
		"2",
		"3",
		"4",
		"&CfgPtn.Playback",
	};

	rtTmp.left = 30;
	rtTmp.top = 55;
	rtTmp.right = rtTmp.left+20;
	rtTmp.bottom = rtTmp.top+20;
	int i = 0;
	int j = 0;
	for(i=0; i<2; i++)
	{
		for(j=0; j<3; j++)
		{
			
			pCheckBox10[i*3+j] = CreateCheckBox(rtTmp, this);
			items[1].SubItems[0].push_back(pCheckBox10[i*3+j]);
			
			rtTmp.left = rtTmp.right+5;
			rtTmp.right = rtTmp.left+120;

			pStatic10[i*3+j] = CreateStatic(rtTmp, this, szStatic[i*3+j]);
			items[1].SubItems[0].push_back(pStatic10[i*3+j]);

			rtTmp.left = rtTmp.right+50;
			rtTmp.right = rtTmp.left+20;
		}

		rtTmp.left = 30;
		rtTmp.top = rtTmp.bottom + 10;
		rtTmp.right = rtTmp.left+20;
		rtTmp.bottom = rtTmp.top+20;
	
	}


	for(i=6; i<8; i++)
	{
		pCheckBox10[i] = CreateCheckBox(rtTmp, this);
		items[1].SubItems[0].push_back(pCheckBox10[i]);
		
		rtTmp.left = rtTmp.right+5;
		rtTmp.right = rtTmp.left+150;

		pStatic10[i] = CreateStatic(rtTmp, this, szStatic[i]);
		items[1].SubItems[0].push_back(pStatic10[i]);

		rtTmp.left = 30;
		rtTmp.top = rtTmp.bottom + 40;
		rtTmp.right = rtTmp.left+20;
		rtTmp.bottom = rtTmp.top+20;
	}


	rtTmp.left = 30;
	rtTmp.top = rtTmp.top - 35;
	rtTmp.right = rtTmp.left+330;
	rtTmp.bottom = rtTmp.top+30;
	pTable11 = CreateTableBox(&rtTmp,this, 5,1,0);
	//pTable0->SetColWidth(0,294);
	items[1].SubItems[0].push_back(pTable11);

	rtTmp.left = 40;
	rtTmp.top = rtTmp.top +5;
	rtTmp.right = rtTmp.left+20;
	rtTmp.bottom = rtTmp.top+20;

	
	for(i=8; i<13; i++)
	{
		if(i == 8)
		{
			pCheckBox10[i] = CreateCheckBox(rtTmp, this, styleEditable, (CTRLPROC)&CPageModifyUser::OnCheckBoxSelectAll);
		}
		else
		{
			pCheckBox10[i] = CreateCheckBox(rtTmp, this);
		}
		items[1].SubItems[0].push_back(pCheckBox10[i]);

		rtTmp.left = rtTmp.right+5;
		rtTmp.right = rtTmp.left+30;

		pStatic10[i] = CreateStatic(rtTmp, this, szStatic[i]);
		items[1].SubItems[0].push_back(pStatic10[i]);

		rtTmp.left = rtTmp.right+10;
		rtTmp.right = rtTmp.left+20;
	}

	rtTmp.left = 30;
	rtTmp.top = rtTmp.bottom +10;
	rtTmp.right = rtTmp.left+20;
	rtTmp.bottom = rtTmp.top+20;


	pCheckBox10[13] = CreateCheckBox(rtTmp, this);
	items[1].SubItems[0].push_back(pCheckBox10[13]);
	
	rtTmp.left = rtTmp.right+5;
	rtTmp.right = rtTmp.left+150;

	pStatic10[13] = CreateStatic(rtTmp, this, szStatic[13]);
	items[1].SubItems[0].push_back(pStatic10[13]);

	rtTmp.left = 30;
	rtTmp.top = rtTmp.bottom +5;
	rtTmp.right = rtTmp.left+330;
	rtTmp.bottom = rtTmp.top+30;
	pTable12 = CreateTableBox(&rtTmp,this, 5,1,0);
	//pTable0->SetColWidth(0,294);
	items[1].SubItems[0].push_back(pTable12);

	rtTmp.left = 40;
	rtTmp.top = rtTmp.top +5;
	rtTmp.right = rtTmp.left+20;
	rtTmp.bottom = rtTmp.top+20;
	
	for(i=14; i<19; i++)
	{
		if(i == 14)
		{
			pCheckBox10[i] = CreateCheckBox(rtTmp, this, styleEditable, (CTRLPROC)&CPageModifyUser::OnCheckBoxSelectAll);
		}
		else
		{
			pCheckBox10[i] = CreateCheckBox(rtTmp, this);
		}

		items[1].SubItems[0].push_back(pCheckBox10[i]);

		rtTmp.left = rtTmp.right+5;
		rtTmp.right = rtTmp.left+30;

		pStatic10[i] = CreateStatic(rtTmp, this, szStatic[i]);
		items[1].SubItems[0].push_back(pStatic10[i]);

		rtTmp.left = rtTmp.right+10;
		rtTmp.right = rtTmp.left+20;
	}

	rtTmp.left = 30;
	rtTmp.top = rtTmp.bottom +10;
	rtTmp.right = rtTmp.left+20;
	rtTmp.bottom = rtTmp.top+20;


	pCheckBox10[19] = CreateCheckBox(rtTmp, this);
	items[1].SubItems[0].push_back(pCheckBox10[19]);

	rtTmp.left = rtTmp.right+5;
	rtTmp.right = rtTmp.left+150;

	pStatic10[19] = CreateStatic(rtTmp, this, szStatic[19]);
	items[1].SubItems[0].push_back(pStatic10[19]);


	
	items[1].cursubindex = 0;
	ShowSubPage(1,0,FALSE);
}


void CPageModifyUser::InitPage11()
{
	CRect rtTmp(m_Rect.left+27, m_Rect.top+50, 
		m_Rect.Width()-27,m_Rect.top+50+270);

	items[1].SubItems[1].push_back(pTable10);
	items[1].SubItems[1].push_back(pScrollbar10);

	char* szStatic[23] = {
		"",
		"1",
		"2",
		"3",
		"4",
		"&CfgPtn.Backup",
		"",
		"1",
		"2",
		"3",
		"4",
		"&CfgPtn.PTZControl",
		"",
		"1",
		"2",
		"3",
		"4",
		"&CfgPtn.RemoteLiveView",
		"",
		"1",
		"2",
		"3",
		"4",
	};

	rtTmp.left = 30;
	rtTmp.top = 55;
	rtTmp.right = rtTmp.left+330;
	rtTmp.bottom = rtTmp.top+30;
	pTable13 = CreateTableBox(&rtTmp,this, 5,1,0);
	//pTable0->SetColWidth(0,294);
	items[1].SubItems[1].push_back(pTable13);

	rtTmp.left = 40;
	rtTmp.top = rtTmp.top +5;
	rtTmp.right = rtTmp.left+20;
	rtTmp.bottom = rtTmp.top+20;
	
	int i;
	for(i=0; i<5; i++)
	{
		if(i == 0)
		{
			pCheckBox11[i] = CreateCheckBox(rtTmp, this, styleEditable, (CTRLPROC)&CPageModifyUser::OnCheckBoxSelectAll);
		}
		else
		{
			pCheckBox11[i] = CreateCheckBox(rtTmp, this);
		}

		items[1].SubItems[1].push_back(pCheckBox11[i]);

		rtTmp.left = rtTmp.right+5;
		rtTmp.right = rtTmp.left+30;

		pStatic11[i] = CreateStatic(rtTmp, this, szStatic[i]);
		items[1].SubItems[1].push_back(pStatic11[i]);

		rtTmp.left = rtTmp.right+10;
		rtTmp.right = rtTmp.left+20;
	}

	rtTmp.bottom += 5;
		
	int j;
	for(j=0; j<3; j++)
	{
		rtTmp.left = 30;
		rtTmp.top = rtTmp.bottom +5;
		rtTmp.right = rtTmp.left+20;
		rtTmp.bottom = rtTmp.top+20;


		pCheckBox11[5+j*6] = CreateCheckBox(rtTmp, this);
		items[1].SubItems[1].push_back(pCheckBox11[5+j*6]);
		
		rtTmp.left = rtTmp.right+5;
		rtTmp.right = rtTmp.left+180;

		pStatic11[5+j*6] = CreateStatic(rtTmp, this, szStatic[5+j*6]);
		items[1].SubItems[1].push_back(pStatic11[5+j*6]);


		rtTmp.left = 30;
		rtTmp.top = rtTmp.bottom+5;
		rtTmp.right = rtTmp.left+330;
		rtTmp.bottom = rtTmp.top+30;
		pTable14[j] = CreateTableBox(&rtTmp,this, 5,1,0);
		//pTable0->SetColWidth(0,294);
		items[1].SubItems[1].push_back(pTable14[j]);

		
		int i;
		for(i=0; i<5; i++)
		{
			if(i == 0)
			{
				pCheckBox11[(j+1)*6+i] = CreateCheckBox(CRect(40+65*i,rtTmp.top +5,60+65*i,rtTmp.top+25), this, styleEditable, (CTRLPROC)&CPageModifyUser::OnCheckBoxSelectAll);
			}
			else
			{
				pCheckBox11[(j+1)*6+i] = CreateCheckBox(CRect(40+65*i,rtTmp.top +5,60+65*i,rtTmp.top+25)
											, this);
			}

			items[1].SubItems[1].push_back(pCheckBox11[(j+1)*6+i]);


			pStatic11[(j+1)*6+i] = CreateStatic(CRect(65+65*i,rtTmp.top +5,95+65*i,rtTmp.top+25)
											, this, szStatic[(j+1)*6+i]);
			items[1].SubItems[1].push_back(pStatic11[(j+1)*6+i]);
		}	
	}

	ShowSubPage(1,1,FALSE);
}

void CPageModifyUser::OnEditChange0()
{
	//printf("OnEditChange0\n");
}

void CPageModifyUser::SwitchPage( int mainID,int subID)
{
	
	if (mainID<0 || mainID>=ADDUSER_SUBPAGES )
	{
		return;
	}
	
	if ((mainID == curID) && (subID == items[curID].cursubindex))
	{
		return;
	}

	//隐藏当前子页面，显示新的子页面
	ShowSubPage(curID,items[curID].cursubindex,FALSE);
	pButton[curID]->Enable(TRUE);


	curID = mainID;
	items[curID].cursubindex = subID;

	pButton[mainID]->Enable(FALSE);
	ShowSubPage(mainID,subID, TRUE);
}

void CPageModifyUser::ShowSubPage( int mainID,int subID, BOOL bShow )
{
	
	if (mainID<0 || mainID>=ADDUSER_SUBPAGES )
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


void CPageModifyUser::SetUser(char* username)
{

	strcpy(m_username,username);
}

VD_BOOL CPageModifyUser::UpdateData( UDM mode )
{
	if((UDM_OPEN == mode) || (UDM_OPENED == mode))
	{
		pStatic0[4]->SetText(m_username);
		
		SBizParaTarget pTarget;
		pTarget.emBizParaType = EM_BIZ_USER_GETUSERINFO;
		SBizUserUserPara userPara;		
		memset(&userPara, 0, sizeof(userPara));
		strcpy((char*)userPara.UserName, m_username);
		if(0 != BizGetPara(&pTarget, &userPara))
		{
			printf("Get user info failed\n");
			return FALSE;
		}
		
		for(int j = 0; j < nRealGroupNum; j++)
		{
			if(0 == strcasecmp((char*)userPara.GroupBelong, pGroupList[j]))
			{
				pComboBox0[0]->SetCurSel(j);
				break;
			}
		}
		
		int value = userPara.emIsBindPcMac?1:0;
		pCheckBox0[0]->SetValue(value);
		
		int n = 0;
		int num = 0;
		char tmp[20] = {0};
		char mac[2] = {0};
		sprintf(tmp, "%012llx", userPara.PcMacAddress);
		
		for(int num = 0; num < PG_0_STATICMACNUM + 1; num++)
		{
			for(int i = 0; i < 2; i++)
			{
				mac[i] = tmp[n];
				n++;
			}
			pEdit0[num]->SetText(mac);
		}
		
		value = userPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_LOG_VIEW]?1:0;
		pCheckBox10[6]->SetValue(value);
		value = userPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_SETUP]?1:0;
		pCheckBox10[0]->SetValue(value);
		value = userPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SHUTDOWN]?1:0;
		pCheckBox10[11]->SetValue(value);
		value = userPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_AUDIO_TALK]?1:0;
		pCheckBox10[7]->SetValue(value);
		value = userPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_DATA_MANAGE]?1:0;
		pCheckBox10[5]->SetValue(value);
		value = userPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_DISK_MANAGE]?1:0;
		pCheckBox10[4]->SetValue(value);
		value = userPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_REMOTE_LOGIN]?1:0;
		pCheckBox10[8]->SetValue(value);
		value = userPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_UPDATE]?1:0;
		pCheckBox10[10]->SetValue(value);
		value = userPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_RECORD]?1:0;
		pCheckBox10[1]->SetValue(value);
		value = userPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_PLAYBACK]?1:0;
		pCheckBox10[2]->SetValue(value);
		value = userPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_SYSTEM_RESET]?1:0;
		pCheckBox10[9]->SetValue(value);
		value = userPara.UserAuthor.nAuthor[EM_BIZ_USER_AUTHOR_PTZCTRL]?1:0;
		pCheckBox10[3]->SetValue(value);
		
        BOOL bUserAut = TRUE;
		
        //add by Lirl on Nov/15/2011
        //管理员的权限不能设置,只对普通用户有用
        if(strcasecmp("admin", (char*)userPara.UserName) == 0)
        {
            bUserAut = FALSE;
        } 
        else
        {
            bUserAut = TRUE;
        }
		
        //权限页面设置
        for(int i=0; i<4; ++i)
        {
            for(int j=0; j<3; ++j) 
            {
                pCheckBox10[i*3+j]->Enable(bUserAut);
            }
        }
		
		#if 1//csp modify 20130519
		{
			pCheckBox10[12]->Enable(bUserAut);
			
			int j = 0;
			int num = 0;
			
			num = GetMaxChnNum();
			for(j=0; j<num+(num+7)/8; j++)
			{
				pCheckBox10[13+j]->Enable(bUserAut);
				
				if((j%9) == 0)
				{
					pCheckBox10[13+j]->SetValue(0);
					continue;
				}
				
				if(userPara.UserAuthor.nRemoteView[0] & (1<<(j-j/9-1)))
				{
					pCheckBox10[13+j]->SetValue(1);
				}
				else
				{
					pCheckBox10[13+j]->SetValue(0);
				}
			}
		}
		#endif
		
        //用户群组设置
        //pComboBox0[0]->Enable(bUserAut);	
		pComboBox0[0]->Enable(FALSE);//暂时不开放管理员用户的添加 yzw
        //end
	}
    else if (UDM_CLOSED == mode)
    {
        ((CPageUserManager *)(this->GetParent()))->RefreshPageString();
    }

	return TRUE;
}

#if 1//csp modify 20130519
void CPageModifyUser::OnCheckBoxSelectItem()
{
	CCheckBox *pFocusButton = (CCheckBox *)GetFocusItem();
	
	int i = 0;
	int num = 0;
	
	num = GetMaxChnNum();
	if(num > 8)
	{
		num = 8;
	}
	for(i=0; i<num; i++)
	{
		if((pFocusButton == pCheckBox10[13+1+i]))
		{
			if(!pFocusButton->GetValue())
			{
				pCheckBox10[13]->SetValue(0);
			}
			return;
		}
	}
	
	if(GetMaxChnNum() <= 8)
	{
		return;
	}
	
	num = GetMaxChnNum() - 8;
	if(num > 8)
	{
		num = 8;
	}
	for(i=0; i<num; i++)
	{
		if((pFocusButton == pCheckBox10[22+1+i]))
		{
			if(!pFocusButton->GetValue())
			{
				pCheckBox10[22]->SetValue(0);
			}
			return;
		}
	}
}

void CPageModifyUser::OnCheckBoxSelectAll()
{
	//u8 chn = 4;
	
	BOOL bFind = FALSE;
	CCheckBox *pFocusButton = (CCheckBox *)GetFocusItem();
	if((pFocusButton == pCheckBox10[13]))
	{
		bFind = TRUE;
	}
	else if((pFocusButton == pCheckBox10[22]))
	{
		bFind = TRUE;
	}
    
	if(!bFind)
	{
		return;
	}
	
	if(pFocusButton == pCheckBox10[13])
	{
		int num = GetMaxChnNum();
		if(num > 8)
		{
			num = 8;
		}
		for(int i=1; i<num+1; i++)
		{
			pCheckBox10[13+i]->SetValue(pCheckBox10[13]->GetValue());
		}
	}
	else if((pFocusButton == pCheckBox10[22]))
	{
		if(GetMaxChnNum() <= 8)
		{
			return;
		}
		
		int num = GetMaxChnNum() - 8;
		if(num > 8)
		{
			num = 8;
		}
		for(int i=1; i<num+1; i++)
		{
			pCheckBox10[22+i]->SetValue(pCheckBox10[22]->GetValue());
		}
	}
}
#else
void CPageModifyUser::OnCheckBoxSelectAll()
{
	u8 chn = 4;
	
	BOOL bFind = FALSE;
	CCheckBox *pFocusButton = (CCheckBox *)GetFocusItem();
	if((pFocusButton == pCheckBox10[8])
		|| (pFocusButton == pCheckBox10[14])
		|| (pFocusButton == pCheckBox11[0])
		|| (pFocusButton == pCheckBox11[6])
		|| (pFocusButton == pCheckBox11[12])
		|| (pFocusButton == pCheckBox11[18]))
	{
		bFind = TRUE;
	}	
	if(!bFind)
	{
		return;
	}

	if(pFocusButton == pCheckBox10[8])
	{
		for(int i=1; i<5; i++)
		{
			pCheckBox10[8+i]->SetValue(pCheckBox10[8]->GetValue()); 	
		}
	}
	else if(pFocusButton == pCheckBox10[14])
	{
		for(int i=1; i<5; i++)
		{
			pCheckBox10[14+i]->SetValue(pCheckBox10[14]->GetValue());		
		}
	}
	else if(pFocusButton == pCheckBox11[0])
	{
		for(int i=1; i<5; i++)
		{
			pCheckBox11[0+i]->SetValue(pCheckBox11[0]->GetValue()); 	
		}
	}
	else if(pFocusButton == pCheckBox11[6])
	{
		for(int i=1; i<5; i++)
		{
			pCheckBox11[6+i]->SetValue(pCheckBox11[6]->GetValue()); 	
		}
	}
	else if(pFocusButton == pCheckBox11[12])
	{
		for(int i=1; i<5; i++)
		{
			pCheckBox11[12+i]->SetValue(pCheckBox11[12]->GetValue());		
		}
	}
	else if(pFocusButton == pCheckBox11[18])
	{
		for(int i=1; i<5; i++)
		{
			pCheckBox11[18+i]->SetValue(pCheckBox11[18]->GetValue());		
		}
	}
}
#endif

