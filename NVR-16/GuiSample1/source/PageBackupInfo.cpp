#include "GUI/Pages/PageBackupInfo.h"
#include "biz.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMessageBox.h"

#define BAKINFO_ROWS 9
#define BTN_NUM 2 // 3 取消磁盘清理

CPageBackupInfo::CPageBackupInfo( VD_PCRECT pRect,VD_PCSTR psz,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPageFrame(pRect, psz, icon, pParent)
{
	SetMargin(0,0,0,0);

	m_pDevGraphics = CDevGraphics::instance(m_screen);
	m_pDevGraphics->GetRect(&m_screenRT);

	bStartBackup = FALSE;
	
	int i=0;
	CRect rtTab;
	rtTab.left = 17;
	rtTab.top = 60;
	rtTab.right = m_Rect.Width()-17;
	rtTab.bottom = rtTab.top + 30*BAKINFO_ROWS;
	pTab = CreateTableBox(rtTab, this, 2, BAKINFO_ROWS);

	char* szBakInfoTitle[BAKINFO_ROWS] =
	{
		"&CfgPtn.StartTime",
		"&CfgPtn.EndTime",
		"&CfgPtn.FileNum",
		"&CfgPtn.TotalSize",
		"&CfgPtn.StorageMedia",
		"&CfgPtn.FreeSize",
		"&CfgPtn.FileSize",//cw_backup
	//	"&CfgPtn.BackupPlayer",
		"&CfgPtn.SaveFileType",
		"",
	};
	
	CRect rt;
	for (i=0; i<BAKINFO_ROWS; i++)
	{
		pTab->GetTableRect(0, i, &rt);
		//if (6 != i) //cw_backup
		{
			pBakItem[i] = CreateStatic(CRect(rt.left+rtTab.left, rt.top+rtTab.top, rt.right+rtTab.left, rt.bottom+rtTab.top), this, szBakInfoTitle[i]);
		}
		//pBakItem[i]->SetTextAlign(VD_TA_CENTER);
		//pBakItem[i]->SetBkColor(VD_RGB(67,77,87));
	}
	
	for(i=0; i<4; i++)
	{
		pTab->GetTableRect(1, i, &rt);
		pBakVal[i] = CreateStatic(CRect(rt.left+rtTab.left, rt.top+rtTab.top, rt.right+rtTab.left, rt.bottom+rtTab.top), this, "");
	}
	
	pTab->GetTableRect(1,4,&rt);
	pCombo[0] = CreateComboBox(CRect(rt.left+rtTab.left+2, rt.top+rtTab.top+1, 
									 rt.right+rtTab.left-2,rt.bottom+rtTab.top-1),
							   this, NULL, NULL, NULL, 0);
	pCombo[0]->AddString("USB-1");
	pCombo[0]->SetCurSel(0);
	pCombo[0]->SetBkColor(VD_RGB(67,77,87));
	
	pTab->GetTableRect(1, 5, &rt);
	pBakVal[4] = CreateStatic(CRect(rt.left+rtTab.left, rt.top+rtTab.top, rt.right+rtTab.left, rt.bottom+rtTab.top), this, "");
	
	pTab->GetTableRect(1, 6, &rt);  //cw_backup
	pBakVal[5] = CreateStatic(CRect(rt.left+rtTab.left, rt.top+rtTab.top, rt.right+rtTab.left, rt.bottom+rtTab.top), this, "");
    	/*
	pTab->GetTableRect(1,6,&rt);
	pChk = CreateCheckBox(CRect(rt.left+rtTab.left+2, rt.top+rtTab.top+4, 
								rt.right+rtTab.left+22,rt.bottom+rtTab.top+24), 
						  this);
		*/
	
	char* playType[2] = 
	{
		"IFV",
		"AVI",
	};
	
	pTab->GetTableRect(1,7,&rt);
	//yaogang modify for snap backup
	//pTextFileType & pCombo[1]位置相同，图片备份时显示pTextFileType，录像备份时显示pCombo[1]
	pTextFileType = CreateStatic(CRect(rt.left+rtTab.left, rt.top+rtTab.top, rt.right+rtTab.left, rt.bottom+rtTab.top), this, "JPEG");
	
	pCombo[1] = CreateComboBox(CRect(rt.left+rtTab.left+2, rt.top+rtTab.top+1, 
									 rt.right+rtTab.left-2,rt.bottom+rtTab.top-1),
							   this, NULL, NULL, NULL, 0);
	
	for(int i=0; i<2; i++)
	{
		pCombo[1]->AddString(playType[i]);
	}
	
	pCombo[1]->SetCurSel(0);
	pCombo[1]->SetBkColor(VD_RGB(67,77,87));
	
	char* szButton[BTN_NUM] =
	{
		//"&CfgPtn.DiskCleanup",
		"&CfgPtn.Start",
		"&CfgPtn.Cancel",
	};
	
	percentInfo = CreateStatic(CRect(16, rtTab.bottom+12, m_Rect.Width()-16, rtTab.bottom+12+27), this, "",staticTransparent);
	percentInfo->SetBkColor(VD_RGB(67,77,87));
	//percentInfo->Show(FALSE);
	
	bakProg = CreateStatic(CRect(17, rtTab.bottom+13, m_Rect.Width()-17, rtTab.bottom+13+25), this, "");
	bakProg->SetBkColor(VD_RGB(8,66,189));
	bakProg->SetBkWidth(0);
	bakProg->SetTextAlign(VD_TA_CENTER);
	bakProg->SetText("0 %");
	
	int btwidth = 90;
	CRect rtBt;
	rtBt.left = m_Rect.Width()-17 - BTN_NUM*btwidth-(BTN_NUM-1)*12;
	rtBt.right = rtBt.left + btwidth;
	rtBt.top = m_Rect.Height() - 34;
	rtBt.bottom = rtBt.top + 25;
	
	for(i = 0; i<BTN_NUM; i++)
	{
		pCtlBtn[i] = CreateButton(rtBt, this, szButton[i],(CTRLPROC)&CPageBackupInfo::OnClkButton);
		pCtlBtn[i]->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"),VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"));
		
		rtBt.left += btwidth+12;
		rtBt.right = rtBt.left + btwidth;
	}

	m_WorkMode = EM_BACKUP_REC;
}

CPageBackupInfo::~CPageBackupInfo()
{
	
}

int CPageBackupInfo::SetBakProg(int nProg)
{
	//formatProg->Show(TRUE,TRUE);
	if(nProg>=5 && nBackProg+5 > nProg && nProg!=100) return 0;
	
	nBackProg = nProg;
	
	int maxWidth = m_Rect.Width()-34;
	int progWidth = maxWidth*nProg/100;
	
	/*
	CRect rt;
	rt.left = (m_screenRT.Width()-m_Rect.Width())/2+17;
	rt.top = (m_screenRT.Height()-m_Rect.Height())/2+m_Rect.Height()-75;
	rt.right = rt.left+progWidth;
	rt.bottom = rt.top+20;
	*/
	
	char szPercent[32] = {0};
	if(nProg == 100)
	{
		//SetSystemLockStatus(0);//cw_backup
		bStartBackup = FALSE;
		strcpy(szPercent,"&CfgPtn.BackupOk");
	}
	else if(nProg < 0)
	{
		//SetSystemLockStatus(0);//cw_backup
		strcpy(szPercent,"&CfgPtn.BackupFail");
        bStartBackup = FALSE;
		nProg = 100;//cw_backup
	//	return 0;
	}
	else if(nProg < 100)
	{
		sprintf(szPercent,"%d %c",nProg,'%');
	}

	//if((nProg%3) == 0)
	{
		bakProg->SetText(szPercent);
		bakProg->SetBkWidth(progWidth);
	}
	
	return 0;
}

void CPageBackupInfo::OnClkButton()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < 5; i++)
	{
		if(pFocusButton == pCtlBtn[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		switch(i)
		{
		/*
			case 0://磁盘清理
				{
					
					//static int count = 0;
					//SetBakProg((count++)%101);
				}
				break;
		*/
			case 0://开始
				{
					SetBakProg(0);
					
					pCtlBtn[0]->Enable(FALSE);
					if (EM_BACKUP_REC == m_WorkMode)
					{
						if(pCombo[1]->GetCurSel() == 0)
						{
							m_backupAttr->fmt = EM_BIZFORMAT_ORIGINAL;
						}
						else
						{
							m_backupAttr->fmt = EM_BIZFORMAT_AVI;
						}
					}
					else if (EM_BACKUP_SNAP == m_WorkMode)
					{
						m_backupAttr->fmt = EM_BIZFORMAT_JPEG;
					}
					printf("%s m_backupAttr->fmt: %d\n", __func__, m_backupAttr->fmt);
					//SetSystemLockStatus(1);  //cw_backup
					//备份过程中的屏蔽已经在MsgProc函数里处理了
					bStartBackup = TRUE;
					BizSysComplexBackup(m_fileType, m_backupAttr, m_backupCondition);
					usleep(10*1000);
					SetFreeSize((u32)-1);
					// 已调整为由杂项在备份完成后通过回调通知业务层剩余容量
					//GetUdevStoreInfo();
				}
				break;
			case 1://取消
				{
					CancelBackup();
				}
				break;
			default:
				break;
		}
	}
}

void CPageBackupInfo::GetUdevStoreInfo()
{
	//update data
	u8 nMaxDiskNum = 64;
	SBizStoreDevManage sStoreDevMgr;

	sStoreDevMgr.nDiskNum = 0;
	SBizDevInfo sDevInfo[nMaxDiskNum]; //zlb20111117 去掉malloc
	sStoreDevMgr.psDevList = sDevInfo;//(SBizDevInfo*)malloc(sizeof(SBizDevInfo)*nMaxDiskNum);//zlb20111117 去掉malloc

	memset(sStoreDevMgr.psDevList, 0, sizeof(SBizDevInfo)*nMaxDiskNum);

	u8 nDiskNum;

	nUdskNum = 0;
	
	if(sStoreDevMgr.psDevList)
	{
		//printf("BizSysComplexDMGetInfo\n");
		BizSysComplexDMGetInfo(&sStoreDevMgr, nMaxDiskNum);

		//printf("BizSysComplexDMGetInfo finish\n");
		
		nDiskNum = sStoreDevMgr.nDiskNum;
		SBizDevInfo* pList; 
		
		pBakVal[3]->SetText("");
		pBakVal[4]->SetText("");
		
		for (int i =0; i<nDiskNum; i++)
		{
			pList = &sStoreDevMgr.psDevList[i];

			if(pList->nType == 1)
			{
				SetUdskInfo(pList->nTotal, pList->nFree);
				
				nUdskNum = 1;
				
				break;
			}
		}

		if(nUdskNum <= 0)
		{
			pCtlBtn[0]->Enable(FALSE);
		}
	}

	//if(sStoreDevMgr.psDevList)
		//free(sStoreDevMgr.psDevList); //zlb20111117 去掉malloc
}

VD_BOOL CPageBackupInfo::UpdateData( UDM mode )
{
	if (UDM_CLOSED == mode)
	{
		bVisible = 0;
		SetBakProg(0);
	}
	else if(UDM_OPEN == mode)
	{
		bChecking = 0;
		bVisible = 1;
		SetBakProg(0);
        bStartBackup = FALSE;
		GetUdevStoreInfo();
	}
	
	return TRUE;
}

void CPageBackupInfo::SetStartTime( char* szStartTime)
{
	pBakVal[0]->SetText(szStartTime);
}

void CPageBackupInfo::SetEndTime( char* szEndTime)
{
	pBakVal[1]->SetText(szEndTime);
}

void CPageBackupInfo::SetFileNum( int num)
{
	char szNum[16] = {0};
	sprintf(szNum,"%d",num);
	pBakVal[2]->SetText(szNum);
}

void CPageBackupInfo::SetFileSize( u32 filesizeMB, u32 filesizeKB )//cw_backup
{
	char fsize[32] = {0};
	char tmp[8] = {0};
	sprintf(fsize, "%ld", filesizeMB);
	sprintf(tmp, ".%02ldMB", filesizeKB/TENKB);
	strcat(fsize,tmp);
	pBakVal[5]->SetText(fsize);
}

void CPageBackupInfo::SetBackupTitle(EM_BACKUPTYPE type)//cw_backup
{
	char tmp[100] = {0};
	switch(type)
	{
		case EM_BACKUP_FILE:
			sprintf(tmp, "%s", GetParsedString("&CfgPtn.BackupInfowithFile"));
			//sprintf(tmp, "%s - %s", GetParsedString("&CfgPtn.BackupInfo"),GetParsedString("&CfgPtn.BackupInfowithFile"));
			break;
		case EM_BACKUP_TIME:
			sprintf(tmp, "%s", GetParsedString("&CfgPtn.BackupInfowithTime"));
			//sprintf(tmp, "%s - %s", GetParsedString("&CfgPtn.BackupInfo"),GetParsedString("&CfgPtn.BackupInfowithTime"));
			break;
		default:
			sprintf(tmp, "%s", GetParsedString("&CfgPtn.BackupInfo"));
			break;
	}
	SetTitle(tmp);
	return;
}

void CPageBackupInfo::SetUdskInfo( u32 totalSize, u32 freeSize )
{
	if(bVisible)
	{
		if(totalSize == -1)
		{
			pBakVal[3]->SetText("");
			pBakVal[4]->SetText("");
			
			pCtlBtn[0]->Enable(FALSE);
			
			return;
		}
		
		SetTotalSize(totalSize);
		SetFreeSize(freeSize);
	}
}

void CPageBackupInfo::SetTotalSize( u32 totalSize)
{		
	char outinfo[32] = {0};
	char tmp[8] = {0};
	sprintf(outinfo, "%ld", totalSize/MEGA);
	sprintf(tmp, ".%02ldG", (totalSize%MEGA)/TENKB);
	strcat(outinfo,tmp);
	
	pBakVal[3]->SetText(outinfo);
}

void CPageBackupInfo::SetFreeSize(u32 freeSize)
{
	if(freeSize == (u32)-1)
	{
		// 检测剩余空间的过程中禁止备份
		pBakVal[4]->SetText("&CfgPtn.checking");
		pCtlBtn[0]->Enable(FALSE);
		bChecking = 1;
	}
	else
	{
		char outinfo[32] = {0};
		char tmp[8] = {0};
		
		sprintf(outinfo, "%ld", freeSize/MEGA);
		sprintf(tmp, ".%02ldG", (freeSize%MEGA/TENKB));
		strcat(outinfo,tmp);
		
		pBakVal[4]->SetText(outinfo);
		pCtlBtn[0]->Enable(TRUE);
		bChecking = 0;
	}
}

void CPageBackupInfo::SetBackupInfo(EMBIZFILETYPE fileType, SBizBackTgtAtt* attribute, SBizSearchCondition* condition)
{
	m_fileType = fileType;
	m_backupAttr = attribute;
	m_backupCondition = condition;
}

VD_BOOL CPageBackupInfo::MsgProc(uint msg, uint wpa, uint lpa)
{
    //在备份开始时只有取消按键可用,其它都屏蔽掉
    if(bStartBackup)
    {
        int px,py;
        px = VD_HIWORD(lpa);
        py = VD_LOWORD(lpa);
        CItem* temp = GetItemAt(px, py);
		
        if((temp == pCtlBtn[1]) //取消按键不屏蔽
            ||(msg == XM_MOUSEMOVE && temp != pCtlBtn[1]) //防止鼠标移开取消按键时底色不会恢复
            ) 
        {
	        /*
	        	if(temp == pCtlBtn[1] && msg !=XM_MOUSEMOVE )
	        	{
				CancelBackup();
			}
	            	return CPageFrame::MsgProc(msg, wpa, lpa);
	            	*/
			return CPageFrame::MsgProc(msg, wpa, lpa);//cw_backup 原代码会导致焦点变色异常
		} 
		else
		{
			switch(msg)
			{
				case XM_KEYDOWN:
				{
					switch(wpa)
					{
						case KEY_ESC:
						{
							CancelBackup();
						};
						default:
		            		return TRUE;
					}
				} break;
				default:
					return TRUE;
			}
		}
    }
    return CPageFrame::MsgProc(msg, wpa, lpa);
}

void CPageBackupInfo::CancelBackup()
{
	if(bStartBackup)
	{
		if(nBackProg<100 && nBackProg>0)
		{
			BizSysCmplexBreakBackup(2);
			UDM ret = MessageBox("&CfgPtn.ConfirmToQuitBackup", 
				"&CfgPtn.WARNING", 
				MB_OKCANCEL|MB_ICONWARNING|MB_DEFBUTTON2
			);						
			if(ret == UDM_OK)
			{
				BizSysCmplexBreakBackup(1);
        		bStartBackup = FALSE;
    			WaitCheckFree(3);
				this->Close();
			}
			else
			{
				BizSysCmplexBreakBackup(0);
				bStartBackup = TRUE;
			}
		}
		else
		{
			BizSysCmplexBreakBackup(1);
    		bStartBackup = FALSE;
    		WaitCheckFree(3);
			this->Close();
		}
	}
}

void CPageBackupInfo::WaitCheckFree(int times)
{
	int i = 0;
	
	while( i++<times && bChecking )
	{
		printf("wait free set ... %d\n", i);
		sleep(1);
	}
}
void CPageBackupInfo::SetSnapMode()
{
	m_WorkMode = EM_BACKUP_SNAP;
	pCombo[1]->Show(FALSE);
	pTextFileType->Show(TRUE);
}
void CPageBackupInfo::SetRecMode()
{
	m_WorkMode = EM_BACKUP_REC;
	pCombo[1]->Show(TRUE);
	pTextFileType->Show(FALSE);
}

