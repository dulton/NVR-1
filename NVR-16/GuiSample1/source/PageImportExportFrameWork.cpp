#include "GUI/Pages/PageImportExportFrameWork.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PageAdvanceConfigFrameWork.h"


#define UPDATE_FILE_MAX 8
#define ROW_ITEM 4

char* importShortcutBmpName[4][2] = {    
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

CPageImportExportFrameWork::CPageImportExportFrameWork( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ ):CPageFrame(pRect, psz, icon, pParent)
{
	m_nCursel = 0;
	m_nFile = 0;
	
	//SetRect(CRect(0, 0, 700, 400/*370*/), FALSE);
	SetMargin(3, m_TitleHeight, 4, m_TitleHeight);

	pBmp_button_normal = VD_LoadBitmap(DATA_DIR"/temp/btn.bmp");
	ReplaceBitmapBits(pBmp_button_normal, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));
	
	pBmp_button_push = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_push, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	pBmp_button_select = VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp");
	ReplaceBitmapBits(pBmp_button_select, VD_GetSysColor(COLOR_TRANSPARENT) , VD_GetSysColor(VD_COLOR_WINDOW));

	CRect rtTemp;
	rtTemp.left = 10;
	rtTemp.top = 10;
	rtTemp.right = rtTemp.left + 160;
	rtTemp.bottom = rtTemp.top + 25;
	pComboBox = CreateComboBox(rtTemp,this, NULL, NULL, NULL, 0);
	pComboBox->SetBkColor(VD_RGB(67,77,87));

	char* szBtn[PG_BUTTONNUM] = 
	{
		"&CfgPtn.Refresh",
		"",
		"",
		"",
		"",
		"&CfgPtn.New",
		"&CfgPtn.Delete",
		"&CfgPtn.Import",
		"&CfgPtn.Export",
		"&CfgPtn.Exit",
	};

	char* szStatic[PG_STATICNUM] =
	{
		"&CfgPtn.Name",
		"&CfgPtn.FILESIZE",
		"&CfgPtn.Type",
		"&CfgPtn.DateModified",
		"0/0",
	};

	rtTemp.left = rtTemp.right + 15;
	rtTemp.top = 10;
	rtTemp.right = rtTemp.left + 80;
	rtTemp.bottom = rtTemp.top + 25;
	pButton[0] = CreateButton(rtTemp, this, szBtn[0], (CTRLPROC)&CPageImportExportFrameWork::OnClickBtn, NULL, buttonNormalBmp);
	pButton[0]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push);

	rtTemp.left = 10;
	rtTemp.top = 43;
	rtTemp.right = m_Rect.Width() - 15;
	rtTemp.bottom = rtTemp.top + 28*9;
	CRect tableRt(rtTemp.left,rtTemp.top,rtTemp.right,rtTemp.bottom);
	pTable0 = CreateTableBox(rtTemp, this, 4, 9);
	pTable0->SetColWidth(0, 260);
	pTable0->SetColWidth(1, 150);
	pTable0->SetColWidth(2, 60);

	rtTemp.left = 10;
	rtTemp.top = rtTemp.bottom-1;
	rtTemp.right = m_Rect.Width() - 15;
	rtTemp.bottom = rtTemp.top + 30;
	pTable1 = CreateTableBox(rtTemp, this, 1, 1);

	int i = 0;
	CRect rt;
	for(i=0; i<4; i++)
	{
		pTable0->GetTableRect(i, 0, &rt);
		pStatic[i] = CreateStatic(CRect(tableRt.left+rt.left, tableRt.top+rt.top, tableRt.left+rt.right,tableRt.top+rt.bottom),
									this, szStatic[i]);
		pStatic[i]->SetBkColor(VD_RGB(67,77,87));
		pStatic[i]->SetTextAlign(VD_TA_XLEFT);
	}

	pTable1->GetTableRect(0, 0, &rt);
	pStatic[i] = CreateStatic(CRect(rtTemp.left+rt.left+2, rtTemp.top+rt.top, rtTemp.left+50,rtTemp.top+rt.bottom),
									this, szStatic[i]);

	for(i=0; i<8; i++)
	{
		pTable0->GetTableRect(0, i+1, &rt);
		pName[i] = CreateStatic(CRect(tableRt.left+rt.left, tableRt.top+rt.top, tableRt.left+rt.right,tableRt.top+rt.bottom),
									this, "");
	}

	for(i=0; i<8; i++)
	{
		pTable0->GetTableRect(1, i+1, &rt);
		pSize[i] = CreateStatic(CRect(tableRt.left+rt.left, tableRt.top+rt.top, tableRt.left+rt.right,tableRt.top+rt.bottom),
									this, "");
	
	}

	for(i=0; i<8; i++)
	{
		pTable0->GetTableRect(2, i+1, &rt);
		pType[i] = CreateStatic(CRect(tableRt.left+rt.left, tableRt.top+rt.top, tableRt.left+rt.right,tableRt.top+rt.bottom),
									this, "");

	}

	for(i=0; i<8; i++)
	{
		pTable0->GetTableRect(3, i+1, &rt);
		pModifyTime[i] = CreateStatic(CRect(tableRt.left+rt.left, tableRt.top+rt.top, tableRt.left+rt.right,tableRt.top+rt.bottom),
									this, "");

	}

	#if 1
	for(i=1; i<5; i++)
	{
		pTable1->GetTableRect(0, 0, &rt);

		pBmpButtonNormal[i-1] = VD_LoadBitmap(importShortcutBmpName[i-1][0]);
		pBmpButtonSelect[i-1] = VD_LoadBitmap(importShortcutBmpName[i-1][1]);
		pButton[i] = CreateButton(CRect(rtTemp.left+rt.Width()-45*(5-i), rtTemp.top+rt.top+1, rtTemp.left+rt.Width()-45*(5-i)+pBmpButtonNormal[i-1]->width,rtTemp.top+rt.top+pBmpButtonNormal[i-1]->height),
									this, szBtn[i], (CTRLPROC)&CPageImportExportFrameWork::OnClickBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmpButtonNormal[i-1], pBmpButtonSelect[i-1], pBmpButtonSelect[i-1]);
		pButton[i]->Enable(FALSE);
	}
	#endif


	rtTemp.left = m_Rect.Width()- 90*5 - 5;
	rtTemp.top = rtTemp.bottom + 15;
	rtTemp.right = rtTemp.left + 80;
	rtTemp.bottom = rtTemp.top + 25;

	pInfoBar = CreateStatic(CRect(17, 
								rtTemp.top,
								317,
								rtTemp.top+22), 
							this, 
							"");
	//pInfoBar->SetBkColor(VD_RGB(8,66,189));

	for(i=5; i<PG_BUTTONNUM; i++)
	{
		pButton[i] = CreateButton(rtTemp, this, szBtn[i], (CTRLPROC)&CPageImportExportFrameWork::OnClickBtn, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmp_button_normal, pBmp_button_select, pBmp_button_push, pBmp_button_normal);

		rtTemp.left = rtTemp.right + 10;
		rtTemp.right = rtTemp.left + 80;
	}
}

void CPageImportExportFrameWork::EnableOpBtn(u8 bEnable)
{
	pButton[6]->Enable(bEnable);
	pButton[7]->Enable(bEnable);
	pButton[8]->Enable(bEnable);
}


VD_BOOL CPageImportExportFrameWork::checkModel(char * path)
{
	static char productnumber[20]={0};
	GetProductNumber(productnumber);
	int i=0;
	char line[128]={0};
	char checkname[]="Model=";
	char *tmp=NULL;
	FILE* fp=fopen(path,"r");
	while(fgets(line,sizeof(line),fp)&&i<10)
	{	
		if((tmp=strstr(line,checkname))!=NULL)
		{
			if(0==strncasecmp(tmp+strlen(checkname),productnumber,strlen(productnumber)))
			{
				return TRUE;
			}
			printf("error:line number=%s\n",line+sizeof(checkname));
		}
		i++;
		memset(line,0,sizeof(line));
	}
	return FALSE;
}

void CPageImportExportFrameWork::OnClickBtn()
{
	//printf("OnClickSubPage \n");

	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < PG_BUTTONNUM; i++)
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
				printf("refresh \n");
				Refresh();
			}break;
			case 1:
			{
				printf("left most \n");
			}break;
			case 2:
			{
				printf("left \n");
			}break;
			case 3:
			{
				printf("right \n");
			}break;
			case 4:
			{
				printf("right most \n");
			}break;
			case 5:
			{
				printf("new \n");

				if(m_nFile>=8)
				{
					char* pszTmp1 = (char*)GetParsedString("&CfgPtn.TooManyLogFile");
					char* pszTmp2 = (char*)GetParsedString("&CfgPtn.WARNING");
					
					MessageBox(pszTmp1, pszTmp2, MB_OK|MB_ICONWARNING);
				}
				else
				{
					char szNew[32] = {0};
					u8 j = 0;
					do
					{
						sprintf(szNew, "config%d.ini", j+1);
						for(i=0; i<8; i++)
						{				
							if(strcmp(pName[i]->GetText(), szNew)==0)
							{
								j++;
								break;
							}
						}

						if(i == 8)
						{							
							char * pDir = pComboBox->GetString(pComboBox->GetCurSel());
							if(strcmp("NULL", pDir) != 0)
							{
								char szPath[64] = {0};
								sprintf(szPath, "%s/config%d.ini", pDir, j+1);
								
								FILE *fp = fopen(szPath,"wb");
								if(fp)
								{
									fwrite(" ", 1, 1, fp);
									fclose(fp);
								}
							} break;
						}
					} while(1);
				}
				Refresh();
				EnableOpBtn(FALSE);
			}break;
			case 6:
			{
				printf("delete \n");
				char * pDir = pComboBox->GetString(pComboBox->GetCurSel());

				char szPath[64] = {0};
				sprintf(szPath, "%s/%s", pDir, pName[m_nCursel-1]->GetText());

				char* pszTmp1 = (char*)GetParsedString("&CfgPtn.DeleteFileMsg");
				char* pszTmp2 = (char*)GetParsedString("&CfgPtn.WARNING");
				
				int ret = MessageBox(pszTmp1, pszTmp2, MB_OKCANCEL|MB_ICONWARNING);
				if(GetRunStatue() == UDM_OK)
				{
					printf("delete file %s\n", szPath);
					remove(szPath);
					Refresh();
				}
				
				m_nCursel = 0;
				EnableOpBtn(FALSE);
			}break;
			case 7:
			{
				printf("import \n");
#define 	V2_CFG_FILE_SIZE_MIN	(20*1024)
				if( strtol((char*)pSize[m_nCursel-1]->GetText(), NULL, 10)<V2_CFG_FILE_SIZE_MIN )
				{
					MessageBox("&CfgPtn.LoadInvalidFile", 
						"&CfgPtn.WARNING", 
						MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2
					);
					
					break;
				}
				
				UDM ret= MessageBox("&CfgPtn.LoadCfgReset", 
					"&CfgPtn.WARNING", 
					MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON2
				);
				if(GetRunStatue() == UDM_OK)
				{		
					char szPath[64] = {0};
					strcpy(szPath, pComboBox->GetString(pComboBox->GetCurSel()));
					sprintf(szPath, "%s/%s", szPath, pName[m_nCursel-1]->GetText());
					
					if (!checkModel(szPath))
					{
						MessageBox("&CfgPtn.LoadCfgFail", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
						Refresh();
						EnableOpBtn(FALSE);
						break;
					}
					
					if( 0 == BizConfigResume(EM_BIZCONFIG_PARA_ALL, 0, szPath) )
					{						
						BizSysComplexExit(EM_BIZSYSEXIT_RESTART);
					}
					else
					{
						MessageBox("&CfgPtn.LoadCfgFail", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
					}
				}
				else if(GetRunStatue() == UDM_CANCEL)
				{
					printf(" cancel \n");
					Refresh();
				}
				EnableOpBtn(FALSE);
			}break;
			case 8:
			{
				printf("export \n");
				char szPath[64] = {0};
				sprintf(szPath, "%s/%s", pComboBox->GetString(pComboBox->GetCurSel()), pName[m_nCursel-1]->GetText());

				printf("backup to %s\n", szPath);
				
				if( 0 == BizConfigBackup(szPath) )
				{
					SetInfo("&CfgPtn.ExportCfgOk");
				}
				else
				{
					SetInfo("&CfgPtn.ExportCfgFail");
				}

				Refresh();
				EnableOpBtn(FALSE);
			}break;
			case 9:
			{
				printf("exit \n");
				this->Close();
			}break;
			default:
			break;

		}

		pComboBox->Draw();
		pButton[0]->Draw();
		pButton[5]->Draw();
		pButton[6]->Draw();
	}
	
}

CPageImportExportFrameWork::~CPageImportExportFrameWork()
{

}

VD_PCSTR CPageImportExportFrameWork::GetDefualtTitleCenter()
{
	return "&CfgPtn.MAINMENU";
}

void CPageImportExportFrameWork::SetResult(char* szText)
{
	pStatic[4]->SetText(szText);
}


void CPageImportExportFrameWork::SetName(int row, char* szText)
{
	if( row<0 || row>7)
	{
		return;
	}

	pName[row]->SetText(szText);
}

void CPageImportExportFrameWork::SetSize(int row, char* szText)
{
	if( row<0 || row>7)
	{
		return;
	}

	pSize[row]->SetText(szText);

}

void CPageImportExportFrameWork::SetType(int row, char* szText)
{
	if( row<0 || row>7)
	{
		return;
	}

	pType[row]->SetText(szText);

}

void CPageImportExportFrameWork::SetModifyTime(int row, char* szText)
{
	if( row<0 || row>7)
	{
		return;
	}

	pModifyTime[row]->SetText(szText);
}

VD_BOOL CPageImportExportFrameWork::UpdateData( UDM mode )
{	
	if (UDM_OPEN == mode)
	{
		int max = 2; // temp set to 4
		
		SBizUpdateFileInfoList sPIns;
		memset(&sPIns, 0, sizeof(sPIns));
		
		sPIns.nMax = max;
		SBizUpdateFileInfo sUpdateFileList[max];
		sPIns.pInfo = sUpdateFileList; //zlb20111117  去掉部分malloc
		memset(sPIns.pInfo, 0, sizeof(SBizUpdateFileInfo)*sPIns.nMax);

		pComboBox->RemoveAll();
		
		if( 0 == BizSysComplexUsbDevMountDirlist(&sPIns) )
		{
	
			if(sPIns.nFileNum > 0)
			{
				for( int i=0; i<sPIns.nFileNum; i++ )
				{
					pComboBox->AddString(sPIns.pInfo[i].szName);
				}
	
				pComboBox->SetCurSel(0);

				Refresh();

				if (m_nCursel > 0)
				{
					pName[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
					pName[m_nCursel-1]->Draw();
					pSize[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
					pSize[m_nCursel-1]->Draw();
					pType[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
					pType[m_nCursel-1]->Draw();
					pModifyTime[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
					pModifyTime[m_nCursel-1]->Draw();
				}

				pButton[5]->Enable(TRUE);
			}
			else
			{
				pComboBox->AddString("NULL");
				
				for(int j=0; j<4; j++)
				{
					pButton[j+5]->Enable(FALSE);
				}
			}
		}
		
		//if(sPIns.pInfo) free(sPIns.pInfo); //zlb20111117  去掉部分malloc
		
		m_nCursel = 0;
		
		EnableOpBtn(FALSE);
	}
	else if(UDM_CANCEL == mode)
	{
		for(int j=0; j<UPDATE_FILE_MAX; j++)
		{
			pName[j]->SetText("");
			pSize[j]->SetText("");
			pType[j]->SetText("");
			pModifyTime[j]->SetText("");
		}
	}
    else if (UDM_CLOSED == mode)
    {
        //((CPageAdvanceConfigFrameWork *)(this->GetParent()))->ReflushItemName();
    }
	
	return TRUE;
}

void CPageImportExportFrameWork::Refresh()
{
	char szDir[32] = {0};

	int max = 8; // temp set to 8

	m_nCurRow = 0;
	m_nCursel = 0;

	if(pComboBox->GetCount()<=0)
	{
		m_nFile = 0;

		EnableOpBtn(FALSE);
		return;
	}
		
	for(int j=0; j<UPDATE_FILE_MAX; j++)
	{
		pName[j]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pName[j]->Draw();
		pSize[j]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pSize[j]->Draw();
		pType[j]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pType[j]->Draw();
		pModifyTime[j]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pModifyTime[j]->Draw();
	}
	
	for(int j=0; j<UPDATE_FILE_MAX; j++)
	{
		pName[j]->SetText("");
		pSize[j]->SetText("");
		pType[j]->SetText("");
		pModifyTime[j]->SetText("");
	}
	
	SBizUpdateFileInfoList sPIns;
	memset(&sPIns, 0, sizeof(sPIns));
	sPIns.nMax = max;
	SBizUpdateFileInfo sUpdateFileList[max];
	sPIns.pInfo = sUpdateFileList; //zlb20111117  去掉部分malloc
	if(!sPIns.pInfo) return;
	
	strcpy(szDir, pComboBox->GetString(0));

	m_nFile = 0;
	
	if( 0==BizSysComplexConfigFilelist( szDir, &sPIns ) )
	{
		char szTmp[20] = {0};
		
		if(sPIns.nFileNum>UPDATE_FILE_MAX)
		{
			sPIns.nFileNum = UPDATE_FILE_MAX;
		}
		
		if(sPIns.nFileNum > 0)
		{			
			for(int i=0; i<sPIns.nFileNum; i++)
			{
				/*
				printf("name: %s\n", sPIns.pInfo[i].szName);
				printf("ext : %s\n", sPIns.pInfo[i].szExt);
				printf("size: %d\n", sPIns.pInfo[i].nSize);
				printf("stamp: %d\n", sPIns.pInfo[i].nTimeStamp);
				*/
				pName[i]->SetText(sPIns.pInfo[i].szName);
				pType[i]->SetText(sPIns.pInfo[i].szExt);
				
				memset(szTmp, 0, sizeof(szTmp));
				sprintf(szTmp, "%d%s", sPIns.pInfo[i].nSize, (char*)GetParsedString("&CfgPtn.BYTE"));
				pSize[i]->SetText(szTmp);
				
				//csp modify
				//struct tm *tmptime = localtime((time_t*)&sPIns.pInfo[i].nTimeStamp);
				struct tm tm0;
				struct tm *tmptime = &tm0;
				localtime_r((time_t*)&sPIns.pInfo[i].nTimeStamp, tmptime);
				
				memset(szTmp, 0, sizeof(szTmp));
				sprintf(szTmp, "%04d-%02d-%02d", 
						tmptime->tm_year+1900,
						tmptime->tm_mon+1,
						tmptime->tm_mday
				);
				
				pModifyTime[i]->SetText(szTmp);
			}	
		}
		else
		{			
			EnableOpBtn(FALSE);
		}
		
		m_nFile = sPIns.nFileNum;

		memset(szTmp, 0, sizeof(szTmp));
		sprintf(szTmp, "%d/%d", m_nFile, m_nFile);

		pStatic[4]->SetText(szTmp);		
	}

	//if(sPIns.pInfo) free(sPIns.pInfo);  //zlb20111117  去掉部分malloc
}

void CPageImportExportFrameWork::SetFileCount(int i, int nFile)
{
	char szTmp[32] = {0};
	memset(szTmp, 0, sizeof(szTmp));
	sprintf(szTmp, "%d/%d", i, nFile);

	pStatic[4]->SetText(szTmp);		
}

void CPageImportExportFrameWork::SelectDskItem( int index )
{
	if (index < 0 || index>=UPDATE_FILE_MAX || !strcmp(pName[index]->GetText(), ""))
	{
		return;
	}

	//printf(" idx=%d, cur =%d \n", index, m_nCursel);

	if (index == m_nCursel-1)
	{
		return;
	}

	if (m_nCursel > 0)
	{
		pName[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pName[m_nCursel-1]->Draw();
		pSize[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pSize[m_nCursel-1]->Draw();
		pType[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pType[m_nCursel-1]->Draw();
		pModifyTime[m_nCursel-1]->SetBkColor(VD_GetSysColor(VD_COLOR_WINDOW));
		pModifyTime[m_nCursel-1]->Draw();
	}
	
	pName[index]->SetBkColor(VD_RGB(56,108,148));
	pName[index]->Draw();
	pSize[index]->SetBkColor(VD_RGB(56,108,148));
	pSize[index]->Draw();
	pType[index]->SetBkColor(VD_RGB(56,108,148));
	pType[index]->Draw();
	pModifyTime[index]->SetBkColor(VD_RGB(56,108,148));
	pModifyTime[index]->Draw();

	m_nCursel = index+1;

	SetFileCount(m_nCursel, m_nFile);
	
	EnableOpBtn(TRUE);
}


VD_BOOL CPageImportExportFrameWork::MsgProc( uint msg, uint wpa, uint lpa )
{
	int px,py;
	int temp;
	switch(msg)
	{
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
				#define TABLE_TOP	(m_TitleHeight+43)
				#define TABLE_LEFT	(m_TitleHeight+10)
				
					VD_RECT rt;
					pTable0->GetTableRect(kk, i+1, &rt);
					rt.left +=TABLE_LEFT+offsetx;
					rt.top +=TABLE_TOP+offsety;//-CTRL_HEIGHT;
					rt.right +=TABLE_LEFT+offsetx;
					rt.bottom +=TABLE_TOP+offsety;//-CTRL_HEIGHT;
					//printf("i=%d kk =%d ,x=%d, y=%d, %d %d %d %d \n",i, kk, px, py, rt.left, rt.top, rt.right,rt.bottom);
					if (PtInRect(&rt, px, py))
					{
						//printf("found point %d\n", kk);
						if(m_nFile>0)
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
				SelectDskItem(i);
			}
		}
		break;
	default:
		break;
	}

	return 	CPageFrame::MsgProc(msg, wpa, lpa);
}

void CPageImportExportFrameWork::SetInfo(char* szInfo)
{
	//pInfoBar->SetText(szInfo);
	pInfoBar->SetText("");
}

void CPageImportExportFrameWork::ClearInfo()
{
	pInfoBar->SetText("");

}


#if 0
// 
typedef struct _sImportExprotTabRow
{
	char* pszLine[4];
} IExportTabRow;

// eCtrl
// 0-left, 1-right, 2-leftmost, 3-rightmost
// 
typedef struct _sImportExprotTabStatic
{
	CStatic** pStatic; // static array in tab, from left to right, from top to down
} IExportTabStatic;

void CPageImportExportFrameWork::TabPageCtrl( 
	u8 					eCtrl, 
	CTableBox* 			pTab, 
	IExportTabStatic* 	pTabStatic, 
	void* 				pContent, 
	u32 				nTolRow 
)
{
	if(!pTab || !pContent)
	{
		printf("NULL tab, content!\n");

		return;
	}
	
	IExportTabRow *pRow = pContent;
	CStatic** pStatic = pTabStatic->pStatic;
	u8 nRow = pTab->m_nRow;
	u8 nCol = pTab->m_nCol;

	switch(eCtrl)
	{
		case 0:
		{
			// 上翻，当前行<=nRow, 翻到头
			if(m_nCurRow <= nRow)
			{
				m_nCurRow = 0;
			}
			else
				m_nCurRow -= nRow;

			for(int i=0; i<nCol; i++)
				pStatic[nCol*m_nCurRow+i]->SetText(pRow->pszLine[i])
			
		} break;
		case 1:
		{
			// 下翻，当前行>=nTolRow-nRow-1, 翻到头
			if(m_nCurRow >= nTolRow-nRow-1)
			{
				m_nCurRow = nTolRow-nRow-1;
			}
			else
				m_nCurRow += nRow;

			for(int i=0; i<nCol; i++)
				pStatic[nCol*m_nCurRow+i]->SetText(pRow->pszLine[i])
		} break;
		case 2:
		{} break;
		case 3:
		{} break;
	}
}
#endif
