#include "GUI/Pages/PageRecordScheduleFrameWork.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageConfigFrameWork.h"
#include "sg_platform.h"

static	std::vector<CItem*>	items[RECSCHEDULE_SUBPAGES];

static VD_BITMAP * pBmp_button_normal;
static VD_BITMAP * pBmp_button_push;
static VD_BITMAP * pBmp_button_select;

static VD_BITMAP * pBmp_tab_normal;
static VD_BITMAP * pBmp_tab_push;
static VD_BITMAP * pBmp_tab_select;

static VD_BITMAP * pBmp_infoicon;

enum{//与下面的数组一一对应
	SCHEDULE_BUTTON_EDITPEN,
	SCHEDULE_BUTTON_ERASER,
	SCHEDULE_BUTTON_NUM,
};


char* scheShortcutBmpName[2][2] = {    
	{DATA_DIR"/temp/edit_pen.bmp",	DATA_DIR"/temp/edit_pen_f.bmp"},
	{DATA_DIR"/temp/eraser.bmp",	DATA_DIR"/temp/eraser_f.bmp"},	

};

static VD_BITMAP* pBmpButtonNormal[3];
static VD_BITMAP* pBmpButtonSelect[3];

static int GetMaxChnNum()
{
	return GetVideoMainNum();
}

CPageRecordScheduleFrameWork::CPageRecordScheduleFrameWork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
, curID(0)
{
	curID = 0;
	
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
	char* szSubPgName[RECSCHEDULE_BTNNUM] = {
		"&CfgPtn.TimeRec",
		"&CfgPtn.MotionRec",	
		"&CfgPtn.SensorRec",
		"&CfgPtn.Default",
		"&CfgPtn.apply",
		"&CfgPtn.Exit",
	};
	int  szSubPgNameLen[RECSCHEDULE_BTNNUM] = {
		TEXT_WIDTH*4,//"&CfgPtn.Schedule",
		TEXT_WIDTH*6,//"&CfgPtn.MotionRec",	
		TEXT_WIDTH*7,//"&CfgPtn.SensorRec",
		TEXT_WIDTH*3,//"&CfgPtn.Default",
		TEXT_WIDTH*2,//"&CfgPtn.apply",
		TEXT_WIDTH*2,//"&CfgPtn.Exit",
	};
	
	CTableBox* pTabFrame = CreateTableBox(CRect(10, 10+pBmp_tab_normal->height, m_Rect.Width()-15, m_Rect.Height()-90),this,1,1);
	//pTabFrame->SetFrameColor(VD_RGB(56,108,148));
	pTabFrame->SetFrameColor(VD_RGB(8,66,189));
	CRect rtSub(25, 10, m_Rect.Width()-20, 10+pBmp_tab_normal->height);
	int i;
	for (i = 0; i<RECSCHEDULE_SUBPAGES; i++)
	{
		int btWidth = szSubPgNameLen[i]+10;
		rtSub.right = rtSub.left + btWidth;
		pButton[i] = CreateButton(rtSub, this, szSubPgName[i], (CTRLPROC)&CPageRecordScheduleFrameWork::OnClickSubPage, NULL, buttonNormalBmp, 1);
		pButton[i]->SetBitmap(pBmp_tab_normal, pBmp_tab_select, pBmp_tab_push, pBmp_tab_push);
		rtSub.left = rtSub.right;

	}

	if(!GetSenSorNum()) pButton[2]->Show(0);//yzw add

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
	for (i=RECSCHEDULE_SUBPAGES; i<RECSCHEDULE_BTNNUM; i++)
	{
		rtSub1.left -= szSubPgNameLen[i]+10;
	}
	rtSub1.left -= 12*2;

	for(i=RECSCHEDULE_SUBPAGES; i<RECSCHEDULE_BTNNUM; i++)
	{
		int btWidth = szSubPgNameLen[i]+10;
		rtSub1.right = rtSub1.left + btWidth;
		pButton[i] = CreateButton(rtSub1, this, szSubPgName[i], (CTRLPROC)&CPageRecordScheduleFrameWork::OnClickSubPage, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_push);
		rtSub1.left = rtSub1.right+12;
	}
	
	//CRect rtSub2(13, 35, m_Rect.Width()-18, 300);
	//pTable = CreateTableBox(&rtSub2,this, 1,1,0);

	
	bSchChange = (uchar**)malloc(sizeof(uchar*)*3);
	for(int i=0; i<3; i++)
	{
		bSchChange[i] = (uchar*)malloc(sizeof(uchar)*GetMaxChnNum());

		for(int j=0; j<GetMaxChnNum(); j++)
		{
			bSchChange[i][j] = 0;
		}
	}

	sch = (uint***)malloc(sizeof(uint**)*3);
	for(int i=0; i<3; i++)
	{
		sch[i] = (uint**)malloc(sizeof(uint*)*GetMaxChnNum());

		for(int j=0; j<GetMaxChnNum(); j++)
		{
			sch[i][j] = (uint*)malloc(sizeof(uint)*7);

			for(int k=0; k<7; k++)
			{
				sch[i][j][k] = 0;
			}
		}
	}

	/*
	for(int i=0; i<3; i++)
		for(int j=0; j<4; j++)
			for(int k=0; k<7; k++)
			{
				printf("sch[%d][%d][%d] = %d \n",i,j,k,sch[i][j][k]);
			}
	*/
	

	InitPage0();
	InitPage1();
	InitPage2();

}

void CPageRecordScheduleFrameWork::OnClickSubPage()
{
	//printf("OnClickSubPage \n");
	int m_nCurID = 0;
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < RECSCHEDULE_BTNNUM; i++)
	{
		if (pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind)
	{
		//printf("find the focus button \n");
		if(i<RECSCHEDULE_SUBPAGES)
		{
			m_nCurID = i;
			SwitchPage(i);	
		}
		else
		{
			if(i==RECSCHEDULE_SUBPAGES+1) // apply
			{
				WriteLogs(curID);
				
				switch(curID)
				{
					case 0://定时录像
					{						
						//定时录像参数保存
						SSG_MSG_TYPE msg;
						memset(&msg, 0, sizeof(msg));
						msg.type = EM_DVR_PARAM_SAVE;
						msg.chn = 0;
						strcpy(msg.note, GetParsedString("&CfgPtn.TimeRec"));
						strcat(msg.note, GetParsedString("&CfgPtn.ParamSave"));
						upload_sg(&msg);
						/*
						printf("curID %d %x %x %x %x %x %x %x\n", 
							curID,
							sch[curID][0][0],
							sch[curID][0][1],
							sch[curID][0][2],
							sch[curID][0][3],
							sch[curID][0][4],
							sch[curID][0][5],
							sch[curID][0][6]
						);
						*/

						int ret = 0;
						for(int i=0; i<GetMaxChnNum(); i++)
						{
							ret = SaveScrStr2Cfg(
								sch[curID][i],
								GSR_CONFIG_SCHEDULE_TIMEREC_SCH, 
								EM_GSR_CTRL_SCH_VAL, 
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
					break;
					case 1://移动报警录像
					{
						//移动报警录像参数保存
						SSG_MSG_TYPE msg;
						memset(&msg, 0, sizeof(msg));
						msg.type = EM_DVR_PARAM_SAVE;
						msg.chn = 0;
						strcpy(msg.note, GetParsedString("&CfgPtn.MotionRec"));
						strcat(msg.note, GetParsedString("&CfgPtn.ParamSave"));
						upload_sg(&msg);
						/*
						printf("curID %d %x %x %x %x %x %x %x\n", 
							curID,
							sch[curID][0][0],
							sch[curID][0][1],
							sch[curID][0][2],
							sch[curID][0][3],
							sch[curID][0][4],
							sch[curID][0][5],
							sch[curID][0][6]
						);
						*/

						int ret = 0;
						for(int i=0; i<GetMaxChnNum(); i++)
						{
							ret = SaveScrStr2Cfg(
								sch[curID][i],
								GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH, 
								EM_GSR_CTRL_SCH_VAL, 
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
					break;
					case 2://传感器报警录像
					{
						//传感器报警录像参数保存
						SSG_MSG_TYPE msg;
						memset(&msg, 0, sizeof(msg));
						msg.type = EM_DVR_PARAM_SAVE;
						msg.chn = 0;
						strcpy(msg.note, GetParsedString("&CfgPtn.SensorRec"));
						strcat(msg.note, GetParsedString("&CfgPtn.ParamSave"));
						upload_sg(&msg);
						/*
						printf("curID %d %x %x %x %x %x %x %x\n", 
							curID,
							sch[curID][0][0],
							sch[curID][0][1],
							sch[curID][0][2],
							sch[curID][0][3],
							sch[curID][0][4],
							sch[curID][0][5],
							sch[curID][0][6]
						);
						*/
						int ret = 0;
						for(int i=0; i<GetMaxChnNum(); i++)
						{
							ret = SaveScrStr2Cfg(
								sch[curID][i],
								GSR_CONFIG_SCHEDULE_SENSORALARMREC_SCH, 
								EM_GSR_CTRL_SCH_VAL, 
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
					break;
				}
			}
			else if(i==RECSCHEDULE_SUBPAGES) 
			{
				//default
				u8 nCh;
				uint mask;
				
				switch(curID)
				{
					case 0:
					{
						nCh = pComboBox0[0]->GetCurSel();
						GetScrStrSetCurSelDo(
							(void*)&pGridSCH0[0],
							GSR_CONFIG_SCHEDULE_TIMEREC_SCH+GSR_DEFAULT_OFFSET, 
							EM_GSR_CTRL_SCH,
							nCh
						);
						for(i=0; i<7; i++)
						{
							pGridSCH0[i]->GetMask(&mask);
							SetSch(nCh, i, &mask);
						}

						uint mask_cur[7] = {0};
						GetSch(nCh, mask_cur);
						for(int n=0; n<7; n++)
						{
							pGridSCH0[n]->SetMask(&mask_cur[n]);
						}
					} break;
					case 1:
					{
						nCh = pComboBox1[0]->GetCurSel();
						GetScrStrSetCurSelDo(
							(void*)&pGridSCH1[0],
							GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH+GSR_DEFAULT_OFFSET, 
							EM_GSR_CTRL_SCH,
							nCh
						);
						for(i=0; i<7; i++)
						{
							pGridSCH1[i]->GetMask(&mask);
							//printf("GET msk idx %d mask %x\n", i, mask);
							SetSch(nCh, i, &mask);
						}
						
						uint mask_cur[7] = {0};
						GetSch(nCh, mask_cur);
						for(int n=0; n<7; n++)
						{
							pGridSCH1[n]->SetMask(&mask_cur[n]);
						}
					} break;
					case 2:
					{
						nCh = pComboBox2[0]->GetCurSel();
						GetScrStrSetCurSelDo(
							(void*)&pGridSCH2[0],
							GSR_CONFIG_SCHEDULE_SENSORALARMREC_SCH+GSR_DEFAULT_OFFSET, 
							EM_GSR_CTRL_SCH,
							nCh
						);
						for(i=0; i<7; i++)
						{
							pGridSCH2[i]->GetMask(&mask);
							SetSch(nCh, i, &mask);
						}

						uint mask_cur[7] = {0};
						GetSch(nCh, mask_cur);
						for(int n=0; n<7; n++)
						{
							pGridSCH2[n]->SetMask(&mask_cur[n]);
						}
					} break;
				}

	
				SetInfo("&CfgPtn.LoadDefaultSuccess");
					
				
			}
			else
			{
				//exit
				this->Close();
			}
			
		}
	}
	
}

CPageRecordScheduleFrameWork::~CPageRecordScheduleFrameWork()
{

}

VD_PCSTR CPageRecordScheduleFrameWork::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

void CPageRecordScheduleFrameWork::OnDBClkGrid(CGridCtrl** pGridSCH, uchar nCh)
{
	int i = 0;
	BOOL bFind = FALSE;
	CGridCtrl *pGrid = (CGridCtrl *)GetFocusItem();
	for (i = 0; i < 7; i++)
	{
		if (pGrid == pGridSCH[i])
		{
			bFind = TRUE;
			break;
		}
	}

	//printf("grid $$$$$$$ i = %d \n",i);

	if (bFind)
	{
		uint mask = 0;
		pGridSCH[i]->GetMask(&mask);

		sch[curID][nCh][i] = mask; 
		
		printf("weekday : %d mask 0x%x, \n", i, mask);

		if (0xFFFFFF == mask)
		{
			printf("start at 00:00 ,end of 23:59\n");
		} 
		else if(0 == mask)
		{
			printf("not masked \n");
		}
		else
		{
			BOOL bMasked = FALSE;
			BOOL bOldMask = bMasked;
			for (i=0; i< 24; i++)
			{
				if (mask & BITMSK(i))
				{
					bMasked = TRUE;
				}
				else
				{
					bMasked = FALSE;
				}

				if (bMasked != bOldMask)
				{
					if (bMasked)
					{
						printf("start at %02d:00 \n", i);
					} 
					else
					{
						printf("end at %02d:59 \n", i-1);
					}

					bOldMask = bMasked;
				}
			}

			if (/*(24 == i) && */bMasked)
			{
				printf("end at %02d:59 \n", i-1);
			}
		}

	}

}

void CPageRecordScheduleFrameWork::OnDBClkGrid0()
{
	printf("OnDBClkGrid0 \n");
	OnDBClkGrid(pGridSCH0, pComboBox0[0]->GetCurSel());
}

void CPageRecordScheduleFrameWork::OnDBClkGrid1()
{
	printf("OnDBClkGrid1 \n");
	OnDBClkGrid(pGridSCH1, pComboBox1[0]->GetCurSel());
}

void CPageRecordScheduleFrameWork::OnDBClkGrid2()
{
	printf("OnDBClkGrid1 \n");
	OnDBClkGrid(pGridSCH2, pComboBox2[0]->GetCurSel());
}

void CPageRecordScheduleFrameWork::OnClkEditPen0()
{
	printf("OnClkEditPen0 \n");
}

void CPageRecordScheduleFrameWork::OnClkEraser0()
{
	printf("OnClkEraser0 \n");
}

void CPageRecordScheduleFrameWork::OnClkCopy0()
{
	printf("OnClkCopy0 \n");
	uint schIns[7]={0};
	
	uchar nCh = pComboBox0[0]->GetCurSel();
	uchar nToCh = pComboBox0[1]->GetCurSel();

	GetSch(nCh, schIns);

	printf("nCh = %d  ,GET %x %x %x %x %x %x %x   copyto %d\n", 
		nCh,
		schIns[0],
		schIns[1],
		schIns[2],
		schIns[3],
		schIns[4],
		schIns[5],
		schIns[6],
		nToCh
	);

	/*
	for(int i=0; i<3; i++)
		for(int j=0; j<4; j++)
			for(int k=0; k<7; k++)
			{
				printf("sch[%d][%d][%d] = %d \n",i,j,k,sch[i][j][k]);
			}
	*/
	
	if(nToCh>=GetMaxChnNum())
	{
		for(int j=0; j<GetMaxChnNum(); j++)
		{
			//printf("");
			if(j!=nCh)
			{
				for(int i=0; i<7; i++)
				{

					
					SetSch(j, i, &schIns[i]);
					
				}
			}
		}
	}
	else
	{
		for(int i=0; i<7; i++)
		{
			SetSch(nToCh, i, &schIns[i]);
		}
	}


}

void CPageRecordScheduleFrameWork::OnComboBox0()
{
	CComboBox* pCombo;
	CGridCtrl** pGridSch;
	
	switch(curID)
	{
		case 0:
		{
			pCombo = pComboBox0[0];
			pGridSch = pGridSCH0;
		} break;
		case 1:
		{
			pCombo = pComboBox1[0];
			pGridSch = pGridSCH1;
		} break;
		case 2:
		{
			pCombo = pComboBox2[0];
			pGridSch = pGridSCH2;
		} break;
		default:
			break;
	}
	
	int nChn = pCombo->GetCurSel();

	for (int i=0; i<7; i++)
	{
		pGridSch[i]->SetMask(&sch[curID][nChn][i]);
	}
}


void CPageRecordScheduleFrameWork::OnClkEditPen1()
{
	printf("OnClkEditPen1 \n");
}

void CPageRecordScheduleFrameWork::OnClkEraser1()
{
	printf("OnClkEraser1 \n");
}

void CPageRecordScheduleFrameWork::OnClkCopy1()
{
	printf("OnClkCopy1\n");
	
	uint schIns[7]={0};
	
	uchar nCh = pComboBox1[0]->GetCurSel();
	uchar nToCh = pComboBox1[1]->GetCurSel();

	GetSch(nCh, schIns);
	if(nToCh>=GetMaxChnNum())
	{
		for(int j=0; j<GetMaxChnNum(); j++)
		{
			if(j!=nCh)
			{
				for(int i=0; i<7; i++)
				{
					SetSch(j, i, &schIns[i]);
				}
			}
		}
	}
	else
	{
		for(int i=0; i<7; i++)
		{
			SetSch(nToCh, i, &schIns[i]);
		}
	}
}

void CPageRecordScheduleFrameWork::OnComboBox1()
{
	int nChn = pComboBox1[0]->GetCurSel();
	
	uint scheMask[7] = {0};
	GetMotionAlarmSchedule(nChn,scheMask,7);
		
	for (int i=0; i<7; i++)
	{
		uint mask = scheMask[i];
		pGridSCH1[i]->SetMask(&mask);
	}
}


void CPageRecordScheduleFrameWork::OnClkEditPen2()
{
	printf("OnClkEditPen2\n");
}

void CPageRecordScheduleFrameWork::OnClkEraser2()
{
	printf("OnClkEraser2 \n");
}

void CPageRecordScheduleFrameWork::OnClkCopy2()
{
	printf("OnClkCopy2 \n");
	
	uint schIns[7]={0};
	
	uchar nCh = pComboBox2[0]->GetCurSel();
	uchar nToCh = pComboBox2[1]->GetCurSel();

	GetSch(nCh, schIns);
	if(nToCh>=GetMaxChnNum())
	{
		for(int j=0; j<GetMaxChnNum(); j++)
		{
			if(j!=nCh)
			{
				for(int i=0; i<7; i++)
				{
					SetSch(j, i, &schIns[i]);
				}
			}
		}
	}
	else
	{
		for(int i=0; i<7; i++)
		{
			SetSch(nToCh, i, &schIns[i]);
		}
	}
}

void CPageRecordScheduleFrameWork::OnComboBox2()
{
	int nChn = pComboBox2[0]->GetCurSel();
	
	uint scheMask[7] = {0};
	GetSensorAlarmSchedule(nChn,scheMask,7);
		
	for (int i=0; i<7; i++)
	{
		uint mask = scheMask[i];
		pGridSCH2[i]->SetMask(&mask);
	}
	
}

void CPageRecordScheduleFrameWork::InitPage0()
{
	char* statictext[16] = {
		"&CfgPtn.Channel",
		"&CfgPtn.Sunday",
		"&CfgPtn.Monday",
		"&CfgPtn.Tuesday",
		"&CfgPtn.Wednesday",
		"&CfgPtn.Thursday",
		"&CfgPtn.Friday",
		"&CfgPtn.Saturday",
		"|00:00",
		"|04:00",
		"|08:00",
		"|12:00",
		"|16:00",
		"|20:00",
		"&CfgPtn.ApplySettingsTo",
		"&CfgPtn.Channel",
	};


	CRect rtSubPage0(m_Rect.left+30, m_Rect.top+48, 
		m_Rect.left+90,m_Rect.top+72);

	pStatic0[0] = CreateStatic(rtSubPage0, this, statictext[0]);
	items[0].push_back(pStatic0[0]);

	pComboBox0[0] = CreateComboBox(CRect(rtSubPage0.left+120, m_Rect.top+48, 
				rtSubPage0.left+230,m_Rect.top+72), 
				this, NULL, NULL, (CTRLPROC)&CPageRecordScheduleFrameWork::OnComboBox0, 0);

	SValue chList[64];
	int nChReal;
	GetChnList(chList, &nChReal, 64);

	for(int k=0; k<nChReal; k++)
	{
		pComboBox0[0]->AddString(chList[k].strDisplay);
	}
	
	pComboBox0[0]->SetBkColor(VD_RGB(67,77,87));
	/*pComboBox0[0]->AddString("1");
	pComboBox0[0]->AddString("2");
	pComboBox0[0]->AddString("3");
	pComboBox0[0]->AddString("4");*/
	pComboBox0[0]->SetCurSel(0);
	items[0].push_back(pComboBox0[0]);
	
	/*
	pBmpButtonNormal[0] = VD_LoadBitmap(scheShortcutBmpName[0][0]);
	pBmpButtonSelect[0] = VD_LoadBitmap(scheShortcutBmpName[0][1]);
	rtSubPage0.left += 518;
	rtSubPage0.right = rtSubPage0.left +  pBmpButtonNormal[0]->width;
	pButton0[0] = CreateButton(rtSubPage0, this, NULL, (CTRLPROC)&CPageRecordScheduleFrameWork::OnClkEditPen0, NULL, buttonNormalBmp);
	pButton0[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0]);
	items[0].push_back(pButton0[0]);

	pBmpButtonNormal[1] = VD_LoadBitmap(scheShortcutBmpName[1][0]);
	pBmpButtonSelect[1] = VD_LoadBitmap(scheShortcutBmpName[1][1]);
	rtSubPage0.left = rtSubPage0.right+5;
	rtSubPage0.right = rtSubPage0.left +  pBmpButtonNormal[1]->width;
	pButton0[1] = CreateButton(rtSubPage0, this, NULL, (CTRLPROC)&CPageRecordScheduleFrameWork::OnClkEraser0, NULL, buttonNormalBmp);
	pButton0[1]->SetBitmap(pBmpButtonNormal[1], pBmpButtonSelect[1], pBmpButtonSelect[1]);
	items[0].push_back(pButton0[1]);
	*/
	
	//rtSub.left = rtSub.right;
	
	CRect rtSubPage1(m_Rect.left+30, m_Rect.top+78, 
		m_Rect.right-30,m_Rect.top+78+29*7);

	pTable0 = CreateTableBox(rtSubPage1, this, 2, 8);
	const int hourwidth = 18;
	pTable0->SetColWidth(0, rtSubPage1.Width() - 24*hourwidth);

	items[0].push_back(pTable0);

	int i = 0;

	for(i=1; i<8; i++)
	{
		CRect rt;
		pTable0->GetTableRect(0, i, &rt);
		pStatic0[i] = CreateStatic(CRect(rtSubPage1.left+rt.left+2, rtSubPage1.top+rt.top, rtSubPage1.left+rt.right,rtSubPage1.top+rt.bottom),
									this, statictext[i]);
		items[0].push_back(pStatic0[i]);
	}

	for(i=8; i<14; i++)
	{
		CRect rt;
		pTable0->GetTableRect(1, 0, &rt);	
		rt.left += (i-8)*((rt.right-rt.left)/6)-3;//+2;
		if(i>11)
			rt.left+=(i-11);
		pStatic0[i] = CreateStatic(CRect(rtSubPage1.left+rt.left, rtSubPage1.top+rt.top, rtSubPage1.left+rt.right,rtSubPage1.top+rt.bottom),
									this, statictext[i]);
		items[0].push_back(pStatic0[i]);
	}


	uint scheMask[7] = {0};
	GetTimerSchedule(0,scheMask,7);
	
	for (i=0; i<7; i++)
	{
		CRect rt;
		pTable0->GetTableRect(1, i+1, &rt);
		pGridSCH0[i] = CreateGridCtrl(CRect(rtSubPage1.left+rt.left, rtSubPage1.top+rt.top, rtSubPage1.left+rt.right,rtSubPage1.top+rt.bottom + 1),
			this, 1, 24, (CTRLPROC)&CPageRecordScheduleFrameWork::OnDBClkGrid0,0);

		pGridSCH0[i]->SetColor(VD_RGB(72,110,216), VD_RGB(67,76,80));

		uint mask = {0};//scheMask[i];
		pGridSCH0[i]->SetMask(&mask);
		items[0].push_back(pGridSCH0[i]);
	}

	CRect rtSubPage2(m_Rect.left+30, m_Rect.top+294, 
	m_Rect.left+140,m_Rect.top+314);

	pStatic0[14] = CreateStatic(rtSubPage2, this, statictext[14]);
	items[0].push_back(pStatic0[14]);

	pComboBox0[1] = CreateComboBox(CRect(rtSubPage2.left+120, m_Rect.top+290, 
				rtSubPage2.left+230,m_Rect.top+314), 
				this, NULL, NULL, NULL, 0);

	SValue copyChnList[64];
	int nCopyChnReal;
	GetCopyChnList(copyChnList, &nCopyChnReal,64);

	for(int k=0; k<nCopyChnReal; k++)
	{
		pComboBox0[1]->AddString(copyChnList[k].strDisplay);
	}

	/*
	pComboBox0[1]->AddString("All");
	pComboBox0[1]->AddString("1");
	pComboBox0[1]->AddString("2");
	pComboBox0[1]->AddString("3");
	pComboBox0[1]->AddString("4");*/
	pComboBox0[1]->SetCurSel(0);
	pComboBox0[1]->SetBkColor(VD_RGB(67,77,87));
	items[0].push_back(pComboBox0[1]);

	rtSubPage2.left = rtSubPage2.left+240;
	rtSubPage2.right = rtSubPage2.left+60;
	pStatic0[15] = CreateStatic(rtSubPage2, this, statictext[15]);
	items[0].push_back(pStatic0[15]);

	int btWidth = strlen("Copy")*TEXT_WIDTH/2+20;
	rtSubPage2.left += 275;
	rtSubPage2.right = rtSubPage2.left + btWidth;
	//rtSubPage2.top += 2;
	rtSubPage2.bottom += 2;
	//rtSubPage2.bottom = rtSubPage2.top + pBmp_tab_normal->height;
	pButton0[2] = CreateButton(rtSubPage2, this, "&CfgPtn.Copy", (CTRLPROC)&CPageRecordScheduleFrameWork::OnClkCopy0, NULL, buttonNormalBmp);
	pButton0[2]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);
	items[0].push_back(pButton0[2]);
	
	ShowSubPage(0,TRUE);
	pButton[0]->Enable(FALSE);
}

void CPageRecordScheduleFrameWork::InitPage1()
{
	char* statictext[16] = 
	{
		"&CfgPtn.Channel",
		"&CfgPtn.Sunday",
		"&CfgPtn.Monday",
		"&CfgPtn.Tuesday",
		"&CfgPtn.Wednesday",
		"&CfgPtn.Thursday",
		"&CfgPtn.Friday",
		"&CfgPtn.Saturday",
		"|00:00",
		"|04:00",
		"|08:00",
		"|12:00",
		"|16:00",
		"|20:00",
		"&CfgPtn.ApplySettingsTo",
		"&CfgPtn.Channel",
	};


	CRect rtSubPage0(m_Rect.left+30, m_Rect.top+48, 
		m_Rect.left+90,m_Rect.top+72);

	pStatic1[0] = CreateStatic(rtSubPage0, this, statictext[0]);
	items[1].push_back(pStatic1[0]);

	pComboBox1[0] = CreateComboBox(CRect(rtSubPage0.left+120, m_Rect.top+48, 
				rtSubPage0.left+230,m_Rect.top+72), 
				this, NULL, NULL, (CTRLPROC)&CPageRecordScheduleFrameWork::OnComboBox0, 0);

	SValue chList[64];
	int nChReal;
	GetChnList(chList, &nChReal, 64);

	for(int k=0; k<nChReal; k++)
	{
		pComboBox1[0]->AddString(chList[k].strDisplay);
	}
	/*pComboBox1[0]->AddString("1");
	pComboBox1[0]->AddString("2");
	pComboBox1[0]->AddString("3");
	pComboBox1[0]->AddString("4");*/
	pComboBox1[0]->SetCurSel(0);
	pComboBox1[0]->SetBkColor(VD_RGB(67,77,87));
	items[1].push_back(pComboBox1[0]);

	/*
	pBmpButtonNormal[0] = VD_LoadBitmap(scheShortcutBmpName[0][0]);
	pBmpButtonSelect[0] = VD_LoadBitmap(scheShortcutBmpName[0][1]);
	rtSubPage0.left += 518;
	rtSubPage0.right = rtSubPage0.left +  pBmpButtonNormal[0]->width;
	pButton1[0] = CreateButton(rtSubPage0, this, NULL, (CTRLPROC)&CPageRecordScheduleFrameWork::OnClkEditPen1, NULL, buttonNormalBmp);
	pButton1[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0]);
	items[1].push_back(pButton1[0]);

	pBmpButtonNormal[1] = VD_LoadBitmap(scheShortcutBmpName[1][0]);
	pBmpButtonSelect[1] = VD_LoadBitmap(scheShortcutBmpName[1][1]);
	rtSubPage0.left = rtSubPage0.right+5;
	rtSubPage0.right = rtSubPage0.left +  pBmpButtonNormal[1]->width;
	pButton1[1] = CreateButton(rtSubPage0, this, NULL, (CTRLPROC)&CPageRecordScheduleFrameWork::OnClkEraser1, NULL, buttonNormalBmp);
	pButton1[1]->SetBitmap(pBmpButtonNormal[1], pBmpButtonSelect[1], pBmpButtonSelect[1]);
	items[1].push_back(pButton1[1]);
	*/
	
	//rtSub.left = rtSub.right;
		
	CRect rtSubPage1(m_Rect.left+30, m_Rect.top+78, 
		m_Rect.right-30,m_Rect.top+78+29*7);

	pTable1 = CreateTableBox(rtSubPage1, this, 2, 8);
	const int hourwidth = 18;
	pTable1->SetColWidth(0, rtSubPage1.Width() - 24*hourwidth);

	items[1].push_back(pTable1);

	int i = 0;

	for(i=1; i<8; i++)
	{
		CRect rt;
		pTable1->GetTableRect(0, i, &rt);
		pStatic1[i] = CreateStatic(CRect(rtSubPage1.left+rt.left+2, rtSubPage1.top+rt.top, rtSubPage1.left+rt.right,rtSubPage1.top+rt.bottom),
									this, statictext[i]);
		items[1].push_back(pStatic1[i]);
	}

	for(i=8; i<14; i++)
	{
		CRect rt;
		pTable1->GetTableRect(1, 0, &rt);
		rt.left += (i-8)*((rt.right-rt.left)/6)-3;//+2;
		if(i>11)
			rt.left+=(i-11);
		pStatic1[i] = CreateStatic(CRect(rtSubPage1.left+rt.left, rtSubPage1.top+rt.top, rtSubPage1.left+rt.right,rtSubPage1.top+rt.bottom),
									this, statictext[i]);
		items[1].push_back(pStatic1[i]);
	}

	uint scheMask[7] = {0};
	GetMotionAlarmSchedule(0,scheMask,7);
	
	for (i=0; i<7; i++)
	{
		CRect rt;
		pTable1->GetTableRect(1, i+1, &rt);
		pGridSCH1[i] = CreateGridCtrl(CRect(rtSubPage1.left+rt.left, rtSubPage1.top+rt.top, rtSubPage1.left+rt.right,rtSubPage1.top+rt.bottom),
			this, 1, 24, (CTRLPROC)&CPageRecordScheduleFrameWork::OnDBClkGrid1,0);

		pGridSCH1[i]->SetColor(VD_RGB(72,110,216), VD_RGB(67,76,80));

		uint mask = {0};//scheMask[i];
		pGridSCH1[i]->SetMask(&mask);
		items[1].push_back(pGridSCH1[i]);
	}

	CRect rtSubPage2(m_Rect.left+30, m_Rect.top+294, 
	m_Rect.left+140,m_Rect.top+314);

	pStatic1[14] = CreateStatic(rtSubPage2, this, statictext[14]);
	items[1].push_back(pStatic1[14]);

	pComboBox1[1] = CreateComboBox(CRect(rtSubPage2.left+120, m_Rect.top+290, 
				rtSubPage2.left+230,m_Rect.top+314), 
				this, NULL, NULL, NULL, 0);

	SValue copyChnList[64];
	int nCopyChnReal;
	GetCopyChnList(copyChnList, &nCopyChnReal, 64);

	for(int k=0; k<nCopyChnReal; k++)
	{
		pComboBox1[1]->AddString(copyChnList[k].strDisplay);
	}

	/*
	pComboBox1[1]->AddString("All");
	pComboBox1[1]->AddString("1");
	pComboBox1[1]->AddString("2");
	pComboBox1[1]->AddString("3");
	pComboBox1[1]->AddString("4");*/
	pComboBox1[1]->SetCurSel(0);
	pComboBox1[1]->SetBkColor(VD_RGB(67,77,87));
	items[1].push_back(pComboBox1[1]);

	rtSubPage2.left = rtSubPage2.left+240;
	rtSubPage2.right = rtSubPage2.left+60;
	pStatic1[15] = CreateStatic(rtSubPage2, this, statictext[15]);
	items[1].push_back(pStatic1[15]);

	int btWidth = strlen("Copy")*TEXT_WIDTH/2+20;
	rtSubPage2.left += 275;
	rtSubPage2.right = rtSubPage2.left + btWidth;
	//rtSubPage2.top += 2;
	rtSubPage2.bottom += 2;
	//rtSubPage2.bottom = rtSubPage2.top + pBmp_tab_normal->height;
	pButton1[2] = CreateButton(rtSubPage2, this, "&CfgPtn.Copy", (CTRLPROC)&CPageRecordScheduleFrameWork::OnClkCopy1, NULL, buttonNormalBmp);
	pButton1[2]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);
	items[1].push_back(pButton1[2]);
	
	ShowSubPage(1,FALSE);
}

void CPageRecordScheduleFrameWork::InitPage2()
{
	char* statictext[16] = 
	{
		"&CfgPtn.Channel",
		"&CfgPtn.Sunday",
		"&CfgPtn.Monday",
		"&CfgPtn.Tuesday",
		"&CfgPtn.Wednesday",
		"&CfgPtn.Thursday",
		"&CfgPtn.Friday",
		"&CfgPtn.Saturday",
		"|00:00",
		"|04:00",
		"|08:00",
		"|12:00",
		"|16:00",
		"|20:00",
		"&CfgPtn.ApplySettingsTo",
		"&CfgPtn.Channel",
	};

	CRect rtSubPage0(m_Rect.left+30, m_Rect.top+48, 
		m_Rect.left+90,m_Rect.top+72);

	pStatic2[0] = CreateStatic(rtSubPage0, this, statictext[0]);
	items[2].push_back(pStatic2[0]);

	pComboBox2[0] = CreateComboBox(CRect(rtSubPage0.left+120, m_Rect.top+48, 
				rtSubPage0.left+230,m_Rect.top+72), 
				this, NULL, NULL, (CTRLPROC)&CPageRecordScheduleFrameWork::OnComboBox0, 0);

	SValue chList[64];
	int nChReal;
	GetChnList(chList, &nChReal, 64);

	for(int k=0; k<nChReal; k++)
	{
		pComboBox2[0]->AddString(chList[k].strDisplay);
	}

	/*
	pComboBox2[0]->AddString("1");
	pComboBox2[0]->AddString("2");
	pComboBox2[0]->AddString("3");
	pComboBox2[0]->AddString("4");*/
	pComboBox2[0]->SetCurSel(0);
	pComboBox2[0]->SetBkColor(VD_RGB(67,77,87));
	items[2].push_back(pComboBox2[0]);

	/*
	pBmpButtonNormal[0] = VD_LoadBitmap(scheShortcutBmpName[0][0]);
	pBmpButtonSelect[0] = VD_LoadBitmap(scheShortcutBmpName[0][1]);
	rtSubPage0.left += 518;
	rtSubPage0.right = rtSubPage0.left +  pBmpButtonNormal[0]->width;
	pButton2[0] = CreateButton(rtSubPage0, this, NULL, (CTRLPROC)&CPageRecordScheduleFrameWork::OnClkEditPen2, NULL, buttonNormalBmp);
	pButton2[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0]);
	items[2].push_back(pButton2[0]);

	pBmpButtonNormal[1] = VD_LoadBitmap(scheShortcutBmpName[1][0]);
	pBmpButtonSelect[1] = VD_LoadBitmap(scheShortcutBmpName[1][1]);
	rtSubPage0.left = rtSubPage0.right+5;
	rtSubPage0.right = rtSubPage0.left +  pBmpButtonNormal[1]->width;
	pButton2[1] = CreateButton(rtSubPage0, this, NULL, (CTRLPROC)&CPageRecordScheduleFrameWork::OnClkEraser2, NULL, buttonNormalBmp);
	pButton2[1]->SetBitmap(pBmpButtonNormal[1], pBmpButtonSelect[1], pBmpButtonSelect[1]);
	items[2].push_back(pButton2[1]);
	*/
	
	//rtSub.left = rtSub.right;
		
	CRect rtSubPage1(m_Rect.left+30, m_Rect.top+78, 
		m_Rect.right-30,m_Rect.top+78+29*7);

	pTable2 = CreateTableBox(rtSubPage1, this, 2, 8);
	const int hourwidth = 18;
	pTable2->SetColWidth(0, rtSubPage1.Width() - 24*hourwidth);

	items[2].push_back(pTable2);

	int i = 0;
	for(i=1; i<8; i++)
	{
		CRect rt;
		pTable2->GetTableRect(0, i, &rt);
		pStatic2[i] = CreateStatic(CRect(rtSubPage1.left+rt.left+2, rtSubPage1.top+rt.top, rtSubPage1.left+rt.right,rtSubPage1.top+rt.bottom),
									this, statictext[i]);
		items[2].push_back(pStatic2[i]);
	}
	
	for(i=8; i<14; i++)
	{
		CRect rt;
		pTable2->GetTableRect(1, 0, &rt);
		rt.left += (i-8)*((rt.right-rt.left)/6)-3;//+2;
		if(i>11)
			rt.left+=(i-11);
		pStatic2[i] = CreateStatic(CRect(rtSubPage1.left+rt.left, rtSubPage1.top+rt.top, rtSubPage1.left+rt.right,rtSubPage1.top+rt.bottom),
									this, statictext[i]);
		items[2].push_back(pStatic2[i]);
	}
	//printf("1111111111111111111 \n");
	uint scheMask[7] = {0};
	GetSensorAlarmSchedule(0,scheMask,7);
	//printf("22222222222222222 \n");
	for (i=0; i<7; i++)
	{
		CRect rt;
		pTable2->GetTableRect(1, i+1, &rt);
		pGridSCH2[i] = CreateGridCtrl(CRect(rtSubPage1.left+rt.left, rtSubPage1.top+rt.top, rtSubPage1.left+rt.right,rtSubPage1.top+rt.bottom),
			this, 1, 24, (CTRLPROC)&CPageRecordScheduleFrameWork::OnDBClkGrid2,0);

		pGridSCH2[i]->SetColor(VD_RGB(72,110,216), VD_RGB(67,76,80));

		uint mask = {0};//scheMask[i];
		pGridSCH2[i]->SetMask(&mask);
		items[2].push_back(pGridSCH2[i]);
	}

	CRect rtSubPage2(m_Rect.left+30, m_Rect.top+294, 
	m_Rect.left+140,m_Rect.top+314);

	pStatic2[14] = CreateStatic(rtSubPage2, this, statictext[14]);
	items[2].push_back(pStatic2[14]);

	pComboBox2[1] = CreateComboBox(CRect(rtSubPage2.left+120, m_Rect.top+290, 
				rtSubPage2.left+230,m_Rect.top+314), 
				this, NULL, NULL, NULL, 0);

	SValue copyChnList[64];
	int nCopyChnReal;
	GetCopyChnList(copyChnList, &nCopyChnReal, 64);

	for(int k=0; k<nCopyChnReal; k++)
	{
		pComboBox2[1]->AddString(copyChnList[k].strDisplay);
	}

	/*
	pComboBox2[1]->AddString("All");
	pComboBox2[1]->AddString("1");
	pComboBox2[1]->AddString("2");
	pComboBox2[1]->AddString("3");
	pComboBox2[1]->AddString("4");*/
	pComboBox2[1]->SetCurSel(0);
	pComboBox2[1]->SetBkColor(VD_RGB(67,77,87));
	items[2].push_back(pComboBox2[1]);

	rtSubPage2.left = rtSubPage2.left+240;
	rtSubPage2.right = rtSubPage2.left+60;
	pStatic2[15] = CreateStatic(rtSubPage2, this, statictext[15]);
	items[2].push_back(pStatic2[15]);
	//printf("5555555555555555555 \n");
	int btWidth = strlen("Copy")*TEXT_WIDTH/2+20;
	rtSubPage2.left += 275;
	rtSubPage2.right = rtSubPage2.left + btWidth;
	//rtSubPage2.top += 2;
	rtSubPage2.bottom += 2;
	//rtSubPage2.bottom = rtSubPage2.top + pBmp_tab_normal->height;
	pButton2[2] = CreateButton(rtSubPage2, this, "&CfgPtn.Copy", (CTRLPROC)&CPageRecordScheduleFrameWork::OnClkCopy2, NULL, buttonNormalBmp);
	pButton2[2]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);
	items[2].push_back(pButton2[2]);
	
	ShowSubPage(2,FALSE);

	//printf("6666666666666666666 \n");
}

void CPageRecordScheduleFrameWork::SwitchPage( int subID)
{
	
	if (subID<0 || subID>=RECSCHEDULE_SUBPAGES )
	{
		return;
	}

	if (subID == curID)
	{
		return;
	}

	//隐藏当前子页面，显示新的子页面
	ShowSubPage(curID, FALSE);
	pButton[curID]->Enable(TRUE);
	curID = subID;
	pButton[curID]->Enable(FALSE);
	ShowSubPage(curID, TRUE);
}

void CPageRecordScheduleFrameWork::ShowSubPage( int subID, BOOL bShow )
{
	
	if (subID<0 || subID>=RECSCHEDULE_SUBPAGES )
	{
		return;
	}


	int count = items[subID].size();
	//printf("ShowSubPage count= %d  \n",count);
	for (int i=0; i<count; i++)
	{
		if(items[subID][i])
		{
			items[subID][i]->Show(bShow);
		}
	}

	//printf("ShowSubPage 2222 \n");
}

void CPageRecordScheduleFrameWork::GridSave()
{
	uchar nCh;
	CGridCtrl** pGrid;
	nCh = GetCurChn();
	pGrid = GetCurGrid();
	uint msk = 0;
	for(int i=0; i<7; i++)
	{
		pGrid[i]->GetMask(&msk);
		SetSch(nCh, i, &msk);
	}
}

VD_BOOL CPageRecordScheduleFrameWork::MsgProc( uint msg, uint wpa, uint lpa )
{
	int px,py;
	int temp;
	uchar nCh;
	CGridCtrl** pGrid;
	CComboBox** pCombox;
	CItem*tmp=GetFocusItem();
	
	switch(msg)
	{
	case XM_KEYDOWN:
		{
			pGrid = GetCurGrid();
			if((wpa==KEY_RET)&&(tmp!=NULL))
			{
				int i=0;
				for(i=0;i<7;i++)
				{
					if (tmp==pGrid[i])
						break;
				}
				if(i<7)
					GridSave();
			}
		}break;
	case XM_LBUTTONUP:
		{
			nCh = GetCurChn();
			pGrid = GetCurGrid();
			uint msk = 0;
			
			for(int i=0; i<7; i++)
			{
				pGrid[i]->GetMask(&msk);
				SetSch(nCh, i, &msk);
			}

		}
	case XM_MOUSEMOVE:
		{
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);
			static CItem* last = NULL;
			CItem* temp = GetItemAt(px, py);

			if(temp != last)
			{
				if(temp == NULL)
				{
					ClearInfo();
				}

				last = temp;

			}
		}
		break;
		break;
	default:
		break;
	}

	return 	CPageFrame::MsgProc(msg, wpa, lpa);
}

uchar CPageRecordScheduleFrameWork::GetCurChn()
{
	uchar nCh = 0;
	
	switch(curID)
	{
		case 0:
		{
			nCh = pComboBox0[0]->GetCurSel();
		} break;
		case 1:
		{
			nCh = pComboBox1[0]->GetCurSel();
		} break;
		case 2:
		{
			nCh = pComboBox2[0]->GetCurSel();
		} break;
		default:
			break;
	}

	return nCh;
}

CGridCtrl** CPageRecordScheduleFrameWork::GetCurGrid()
{	
	switch(curID)
	{
		case 0:
		{
			return pGridSCH0;
		} break;
		case 1:
		{
			return pGridSCH1;
		} break;
		case 2:
		{
			return pGridSCH2;
		} break;
		default:
			break;
	}

	return NULL;
}

CComboBox** CPageRecordScheduleFrameWork::GetCurCombox()
{	
	switch(curID)
	{
		case 0:
		{
			return pComboBox0;
		} break;
		case 1:
		{
			return pComboBox1;
		} break;
		case 2:
		{
			return pComboBox2;
		} break;
		default:
			break;
	}

	return NULL;
}

VD_BOOL CPageRecordScheduleFrameWork::UpdateData( UDM mode )
{
	int nCh, i;
	uint mask=0;

	//return TRUE;
	
	if (UDM_OPEN == mode)
	{
		int tmp = curID;
		for(curID = 0; curID<3; curID++) //yzw add 需要更新所有的
		{			
			switch(curID)
			{
				case 0:
				{
					#if 1
					//nCh = pComboBox0[0]->GetCurSel();
					for(nCh = 0; nCh < GetMaxChnNum(); nCh++)
					{
						GetScrStrSetCurSelDo(
							(void*)&pGridSCH0[0],
							GSR_CONFIG_SCHEDULE_TIMEREC_SCH, 
							EM_GSR_CTRL_SCH,
							nCh
						);
						
						for(i=0; i<7; i++)
						{
							pGridSCH0[i]->GetMask(&mask);
							SetSch(nCh, i, &mask);
						}
					}
					#endif
					uint mask[7] = {0};
					int curIndex = pComboBox0[0]->GetCurSel();
					GetSch(curIndex, mask);
					for(int i=0; i<7; i++)
					{
						pGridSCH0[i]->SetMask(&mask[i]);
					}
					
				}break;
				case 1:
				{
					//nCh = pComboBox1[0]->GetCurSel();
					for(nCh = 0; nCh < GetMaxChnNum(); nCh++)
					{
						GetScrStrSetCurSelDo(
							(void*)&pGridSCH1[0],
							GSR_CONFIG_SCHEDULE_MOTIONALARMREC_SCH, 
							EM_GSR_CTRL_SCH,
							nCh
						);
						for(i=0; i<7; i++)
						{
							pGridSCH1[i]->GetMask(&mask);
							//printf("GET msk idx %d mask %x\n", i, mask);
							SetSch(nCh, i, &mask);
						}
					}

					uint mask[7] = {0};
					int curIndex = pComboBox1[0]->GetCurSel();
					GetSch(curIndex, mask);
					for(int i=0; i<7; i++)
					{
						pGridSCH1[i]->SetMask(&mask[i]);
					}
					
				} break;
				case 2:
				{
					//nCh = pComboBox2[0]->GetCurSel();
					for(nCh = 0; nCh < GetMaxChnNum(); nCh++)
					{
						GetScrStrSetCurSelDo(
							(void*)&pGridSCH2[0],
							GSR_CONFIG_SCHEDULE_SENSORALARMREC_SCH, 
							EM_GSR_CTRL_SCH,
							nCh
						);
						for(i=0; i<7; i++)
						{
							pGridSCH2[i]->GetMask(&mask);
							SetSch(nCh, i, &mask);
						}
					}

					uint mask[7] = {0};
					int curIndex = pComboBox2[0]->GetCurSel();
					GetSch(curIndex, mask);
					for(int i=0; i<7; i++)
					{
						pGridSCH2[i]->SetMask(&mask[i]);
					}
				} break;
			}
		}
		/*
		printf("GET %x %x %x %x %x %x %x\n", 
			sch[curID][0][0],
			sch[curID][0][1],
			sch[curID][0][2],
			sch[curID][0][3],
			sch[curID][0][4],
			sch[curID][0][5],
			sch[curID][0][6]
		);
		*/

		/*
		for(int i=0; i<3; i++)
			for(int j=0; j<4; j++)
				for(int k=0; k<7; k++)
				{
					printf("sch[%d][%d][%d] = %d \n",i,j,k,sch[i][j][k]);
				}
		*/

		
		curID = tmp;
	} 
	else if (UDM_CLOSED == mode)
	{
        ((CPageConfigFrameWork *)(this->GetParent()))->ReflushItemName();
    }
	
	return TRUE;
}

void CPageRecordScheduleFrameWork::WriteLogs(int PageIndex)
{
	switch(PageIndex)
	{
		case 0:
			BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG,BIZ_LOG_SLAVE_CHANGE_TIMER_RECSCHE);
			break;
		case 1:
			BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG,BIZ_LOG_SLAVE_CHANGE_MD_RECSCHE);
			break;
		case 2:
			BizGUiWriteLog(BIZ_LOG_MASTER_SYSCONFIG,BIZ_LOG_SLAVE_CHANGE_SENSORALARM_RECSCHE);
			break;
		default:
			break;
	}
}


void CPageRecordScheduleFrameWork::SetSch( uchar nCh, uchar idx, uint* psch)
{
	sch[curID][nCh][idx] = *psch;
	//memcpy(sch[curID][nCh][idx], sch, sizeof(uint));
}

void CPageRecordScheduleFrameWork::GetSch( uchar nCh, uint* psch)
{
	 //*sch = sch[curID][nCh][idx];


		
	memcpy(psch, sch[curID][nCh], sizeof(uint)*7);

	/*
	printf("**********************************\n");
	for(int i=0; i<3; i++)
		for(int j=0; j<4; j++)
			for(int k=0; k<7; k++)
			{
				printf("sch[%d][%d][%d] = %d \n",i,j,k,sch[i][j][k]);
			}
	*/
		
}


void CPageRecordScheduleFrameWork::ShowSch( uchar nCh )
{
	CGridCtrl** pSch = NULL;
	switch(curID)
	{
		case 0:
		{
			pSch = pGridSCH0;
		} break;
		case 1:
		{
			pSch = pGridSCH1;
		} break;
		case 2:
		{
			pSch = pGridSCH2;
		} break;
		default:
			return;
	}
	
	for(int i=0; i<7; i++)
		pSch[i]->SetMask(&sch[curID][nCh][i]);
}

void CPageRecordScheduleFrameWork::SetInfo(char* szInfo)
{
	pIconInfo->Show(TRUE);
	
	pInfoBar->SetText(szInfo);

}

void CPageRecordScheduleFrameWork::ClearInfo()
{
	pIconInfo->Show(FALSE);
	
	pInfoBar->SetText("");

}

