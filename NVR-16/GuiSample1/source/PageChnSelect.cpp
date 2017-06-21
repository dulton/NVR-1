#include "GUI/Pages/PageChnSelect.h"
#include "GUI/Pages/PageDesktop.h"
#include "GUI/Pages/PageStartFrameWork.h"
#include "GUI/Pages/PageSearch.h"
#include "GUI/Pages/BizData.h"

enum{//与下面的数组一一对应
	CHN_BUTTON_OK,
	CHN_BUTTON_CANCLE,
	CHN_BUTTON_NUM,
};

char* chnShortcutBmpName[CHN_BUTTON_NUM][2] = {    
	{DATA_DIR"/temp/ok.bmp",	DATA_DIR"/temp/ok_f.bmp"},
	{DATA_DIR"/temp/exit2.bmp",	DATA_DIR"/temp/exit2_f.bmp"},	
};

static VD_BITMAP* pBmpButtonNormal[CHN_BUTTON_NUM];
static VD_BITMAP* pBmpButtonSelect[CHN_BUTTON_NUM];

static BOOL nSelectChn[CHN_MAX] = {0};

static int GetMaxChnNum() { return GetVideoMainNum(); }

CPageChnSelect::CPageChnSelect( 
	VD_PCRECT 	pRect,
	VD_PCSTR 	psz /*= NULL*/,
	VD_BITMAP* 	icon /*= NULL*/,
	CPage * 	pParent /*= NULL*/, 
	uint 		vstyle /*= 0 single, =1 4 channels in one line*/,
	uchar 		nChnNum /*=0 通道总数目*/
 ):CPageFloat(pRect, pParent)
{
	m_bChnSelectPage = TRUE;
	
	m_nIsDwell = 0;
	//m_pDesktop = pParent;
	
	nChMax = GetMaxChnNum();
	if(nChnNum != 0)
	{
		nChMax = nChnNum;
	}
	
	CRect rtTmp;
	CRect rtTmpAll;
	
	//csp modify 20130504
	CRect rtTmpMax;
	
	m_vstyle = vstyle;
	
	int i = 0, nLines = (nChMax + ROW_ITEM - 1) / (ROW_ITEM);
	
	m_nLines = nLines;
	
	u8 nChkBox = (vstyle!=0)?(nChMax+nLines):nChMax;
	m_nCheckBox = nChkBox;
	
	//printf("&&&&&&&&&&&&&&&&&&&&&&vstyle:%d, nChMax:%d, nLines:%d, nChkBox:%d\n",vstyle, nChMax, nLines, nChkBox);
	
	pCheckBox = (CCheckBox **)malloc(sizeof(CCheckBox *) * (nChkBox));
	pStatic = (CStatic **)malloc(sizeof(CStatic *) * (nChMax));
	
	if(vstyle==0)
	{
		for(i=0; i<nChkBox; i++)
		{
			if(0 == (i % (ROW_ITEM)))
			{
				rtTmp.left = 4;
				rtTmp.right = rtTmp.left + CHKBOXW;
				rtTmp.top = 4 + (i / (ROW_ITEM)) * (CHKBOXW + GAP);
				rtTmp.bottom = rtTmp.top + (CHKBOXW);
			}
			
			pCheckBox[i] = CreateCheckBox(rtTmp,this,styleEditable,(CTRLPROC)&CPageChnSelect::OnCheckSel);
			
			//pCheckBox[i]->SetValue(TRUE);
			
			rtTmp.left = rtTmp.right + GAP1 + 1;
			rtTmp.right = rtTmp.left+STATICW;
			rtTmp.bottom = rtTmp.top+STATICW;
			
			char chnId[4] = {0};
			sprintf(chnId,"%d",i+1);
			pStatic[i] = CreateStatic(rtTmp, this, chnId);
			pStatic[i]->SetTextAlign(VD_TA_CENTER);
			
			rtTmp.left = rtTmp.right + GAP1 - 1;
			rtTmp.right = rtTmp.left+CHKBOXW;
			
			//csp modify 20130504
			if((i % (ROW_ITEM)) == ((ROW_ITEM) - 1))
			{
				rtTmpMax = rtTmp;
			}
			if(i == nChkBox-1)
			{
				rtTmpMax.top = rtTmp.top;
				rtTmpMax.bottom = rtTmp.bottom;
			}
		}
		
		pCheckBox[0]->SetValue(TRUE);
	}
	else
	{
		for(i=0; i<nChkBox; i++)
		{
			if(0 == (i % (ROW_ITEM+1)))
			{
				rtTmp.left = 4;
				rtTmp.right = rtTmp.left + (CHKBOXW + GAP);
				rtTmp.top = 4 + (i / (ROW_ITEM+1)) * (CHKBOXW + GAP);
				rtTmp.bottom = rtTmp.top + (CHKBOXW + GAP);
				
				pCheckBox[i] = CreateCheckBox(rtTmp,this,styleEditable,(CTRLPROC)&CPageChnSelect::OnHeadCheckSel);
				
				BIZ_DT_DBG("CPageChnSelect 0\n");
				
				rtTmp.left = rtTmp.right;
				rtTmp.right = rtTmp.left+(CHKBOXW);
			}
			else
			{
				pCheckBox[i] = CreateCheckBox(rtTmp,this,styleEditable,(CTRLPROC)&CPageChnSelect::OnCheckSel);
				
				//pCheckBox[i]->SetValue(TRUE);
				
				rtTmp.left = rtTmp.right + GAP1 + 1;
				rtTmp.right = rtTmp.left+CHKBOXW;
				rtTmp.bottom = rtTmp.top+STATICW;
				
				char chnId[4] = {0};
				
				u8 nRealIdx;
				nRealIdx = i - (i / (ROW_ITEM+1) + 1); /* pCheckBox比pStatic每行多一个 */
				
				sprintf(chnId,"%d",i-(i/(ROW_ITEM+1)+1)+1);
				pStatic[nRealIdx] = CreateStatic(rtTmp, this, chnId);
				pStatic[nRealIdx]->SetTextAlign(VD_TA_CENTER);
				
				rtTmp.left = rtTmp.right;
				rtTmp.right = rtTmp.left+CHKBOXW;
			}
			
			//csp modify 20130504
			if((i % (ROW_ITEM+1)) == (ROW_ITEM))
			{
				rtTmpMax = rtTmp;
			}
			if(i == nChkBox-1)
			{
				rtTmpMax.top = rtTmp.top;
				rtTmpMax.bottom = rtTmp.bottom;
			}
		}	
		
		pCheckBox[1]->SetValue(TRUE);
	}
	
	curSel = 0;
	
	//csp modify 20130504
	if((nChMax % (ROW_ITEM)) != 0)
	{
		rtTmp = rtTmpMax;
		
		rtTmp.left =  rtTmp.left - 30*2 - 6;
		rtTmp.right = rtTmp.left + 30;
		rtTmp.top = rtTmp.top + (CHKBOXW + GAP)+5 + 2;
		rtTmp.bottom = rtTmp.top + 24;
		
		rtTmpAll.top = rtTmp.top - 3;
		rtTmpAll.bottom = rtTmp.bottom - 3;
	}
	else
	{
		rtTmp.left =  rtTmp.left - 30*2 - 6;
		rtTmp.right = rtTmp.left + 30;
		rtTmp.top = rtTmp.top + (CHKBOXW + GAP)+5;
		rtTmp.bottom = rtTmp.top + 24;
		
		rtTmpAll.top = rtTmp.top;
		rtTmpAll.bottom = rtTmp.bottom;
	}
	
	for(i=0; i<2; i++)
	{
		BIZ_DT_DBG("CPageChnSelect 4\n");
		
		pBmpButtonNormal[i] = VD_LoadBitmap(chnShortcutBmpName[i][0]);
		pBmpButtonSelect[i] = VD_LoadBitmap(chnShortcutBmpName[i][1]);
		
		pButton[i] = CreateButton(rtTmp, this, NULL, (CTRLPROC)&CPageChnSelect::OnClkButton, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
		
		rtTmp.left = rtTmp.right+6;
		rtTmp.right = rtTmp.left+pBmpButtonNormal[i]->width;
	}
	
	rtTmpAll.left = 4;
	rtTmpAll.right = rtTmpAll.left + TEXT_WIDTH * 2;
	
	pStaticAll = CreateStatic(rtTmpAll, this, "&CfgPtn.All");
	
	rtTmpAll.left = rtTmpAll.right + 2;
	rtTmpAll.right = rtTmpAll.left + CHKBOXW;
	
	//csp modify 20130504
	if((nChMax % (ROW_ITEM)) != 0)
	{
		rtTmpAll.top += 4;
		rtTmpAll.bottom += 4;
	}
	
	pCheckBoxAll = CreateCheckBox(rtTmpAll, this,styleEditable,(CTRLPROC)&CPageChnSelect::OnCheckAllSel);
	
	pStaticAll->Show(FALSE);
	pCheckBoxAll->Show(FALSE);
	
	BIZ_DT_DBG("CPageChnSelect 5\n");
	
	m_nIsPlaying = 0;
	m_nIsSearching = 0;
	m_nIsPbElecZoom = 0;
	m_nIsElecZoom = 0;
	m_nIsRecorDing = 0;
}

void CPageChnSelect::OnCheckAllSel()
{
	//static int CurAllSel = 0;
	
	int i = 0;
	CCheckBox* pFocus = (CCheckBox*)GetFocusItem();
	
	if(pFocus==pCheckBoxAll)
	{
		if(m_vstyle!=0)
		{
			if(pFocus->GetValue())
			{
				for(i=0; i<m_nCheckBox; i++)
				{
					pCheckBox[i]->SetValue(TRUE);
				}
			}
			else
			{
				for(i=0; i<m_nCheckBox; i++)
				{
					pCheckBox[i]->SetValue(FALSE);
				}
			}
		}
	}
}

void CPageChnSelect::OnCheckSel()
{
	u8 nMaxChnNum = GetMaxChnNum();
	CCheckBox* pFocus = (CCheckBox*)GetFocusItem();
	
	int i = 0;
	if(m_vstyle!=0)
	{
		for(i=0; i<m_nCheckBox; i++)
		{
			if(pFocus==pCheckBox[i])
			{
				break;
			}
		}
		
		pCheckBox[(i/(ROW_ITEM+1))*(ROW_ITEM+1)]->SetValue(FALSE);
		
		curSel = 0;
		
        for(int i = 1; i <= m_nLines; i++)
        {
			CheckVLineCheckBox(i);
        }
	}
	else
	{
		curSel = 0;
		
		for(i=0; i<m_nCheckBox; i++)
		{
			pCheckBox[i]->SetValue(FALSE);
			
			if(pFocus==pCheckBox[i])
			{
				curSel = i;
			}
		}
		
		pFocus->SetValue(TRUE);
	}
}

void CPageChnSelect::OnHeadCheckSel()
{
	int i = 0;
	u8 nLine;
	u8 nMaxChnNum = GetVideoMainNum();
	
	CCheckBox* pFocus = (CCheckBox*)GetFocusItem();
	
	for(i=0; i<m_nCheckBox; i++)
	{
		if(pFocus==pCheckBox[i])
		{
			break;
		}
	}
	
	nLine = i/(ROW_ITEM+1)+1;
	//pCheckBox[curSel]->SetValue(FALSE);
	
	u8 nHeadChkIdx = (nLine-1)*(ROW_ITEM+1);
	
	if(pFocus->GetValue())
	{
		//pFocus->SetValue(FALSE); // checkbox默认事件响应已经完成setvalue 
		
		for(i=0; i<ROW_ITEM; i++)
		{
			//csp modify 20130504
			if(nHeadChkIdx+i+1 >= m_nCheckBox)
			{
				break;
			}
			
			pCheckBox[nHeadChkIdx+i+1]->SetValue(TRUE);
		}
		
		curSel = 0;
	}
	else
	{
		//pFocus->SetValue(TRUE);
		
		for(i=0; i<ROW_ITEM; i++)
		{
			//csp modify 20130504
			if(nHeadChkIdx+i+1 >= m_nCheckBox)
			{
				break;
			}
			
			pCheckBox[nHeadChkIdx+i+1]->SetValue(FALSE);
		}
		
		curSel = nHeadChkIdx;
	}
}

void CPageChnSelect::OnClkButton()
{
	int i = 0;
	u8 nMaxChnNum = GetVideoMainNum();
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < CHN_BUTTON_NUM; i++)
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
			case CHN_BUTTON_OK:
			{
				if((!m_nIsRecorDing) && (!m_nIsPlaying) && (!m_nIsSearching) && (!m_nIsElecZoom) && (!m_nIsPbElecZoom))
				{
					StartPreviewSelect();//切换预览画面
					this->Close();
				}				
				else if(m_nIsElecZoom)
				{
					StartPreviewElecZoom();//预览电子放大
				}
				else if(m_nIsPlaying)
				{
					if(m_nIsPbElecZoom)
					{
						StartPbElecZoom();//回放电子放大
					}
					else
					{//回放通道选择
						for(int i=0; i<m_nCheckBox; i++)
						{
							if(pCheckBox[i]->GetValue() == TRUE)
							{
								u64 tmp = 0;
								BizPlayBackGetRealPlayChn(&tmp);
								if(!(tmp & (1 << i)))
								{
									this->Close();
									MessageBox("&CfgPtn.NoPlayBackFile", "&CfgPtn.WARNING" ,
						       		 MB_OK|MB_ICONWARNING);
									return;
								}
								
								for(int j=0;j<16;j++)
								{
									((CPageDesktop*)m_pDesktop)->ShowSplitLine(j, FALSE);
								}
								
								BizPlayBackZoom(i);
								
								CPage** page = GetPage();
								((CPagePlayBackFrameWork*)page[EM_PAGE_PLAYBACK])->SetIsZoomed(1);
								break;
							}
						}

						for(int i=0; i<m_nCheckBox; i++)
						{
							if(pCheckBox[i]->GetValue() == TRUE)
							{
								nSelectChn[i] = TRUE;
							}
							else
							{
								nSelectChn[i] = FALSE;
							}
						}
						
						this->Close();
					}
					m_nIsPlaying = 0;
				}
				else if(m_nIsRecorDing)
				{
					StartManuelRec();//手动录像启动
				}
				else if(m_nIsSearching)
				{
					this->Close();
					m_nIsSearching = 0;
				}
			}break;
			case CHN_BUTTON_CANCLE:
			{
				m_nIsElecZoom = 0;
				m_nIsPbElecZoom = 0;
				m_nIsRecorDing = 0;
				this->Close();
			}break;
			default:
			break;
		}
	}
}

void CPageChnSelect::StartPreviewElecZoom()
{
	for(int i=0; i<m_nCheckBox; i++)
	{
		if(pCheckBox[i]->GetValue() == TRUE)
		{
			//printf("chn = %d,进入电子放大区域设置!!\n",i);
			BizSetZoomMode(1);
			if(GetCurDwellStatue())
			{
				((CPageDesktop*)m_pDesktop)->SetIsDwellFlag(1);
				ControlPatrol(0);
			}
			
			((CPageDesktop*)m_pDesktop)->HideDeskOsdWithoutTime(nChMax);//仅隐藏画线，通道名
			((CPageDesktop*)m_pDesktop)->SetModePlaying();
			
			BizChnPreview(i);
			((CPageDesktop*)m_pDesktop)->ShowChnName(i, TRUE);
			((CPageDesktop*)m_pDesktop)->SetElecZoomChn(i);
			((CPageDesktop*)m_pDesktop)->SetElecZoomstatus(1);
			((CPageDesktop*)m_pDesktop)->ShowElecZoomTile(0,TRUE);
			((CPageDesktop*)m_pDesktop)->SetCurVideoSize();
			
			this->Close();
			((CPageDesktop *)(this->GetParent()))->Close();
			
			break;
		}
	}
	
	for(int i=0; i<m_nCheckBox; i++)
	{
		if(pCheckBox[i]->GetValue() == TRUE)
		{
			nSelectChn[i] = TRUE;
		}
		else
		{
			nSelectChn[i] = FALSE;
		}
	}
	
	m_nIsElecZoom = 0;
}

void CPageChnSelect::StartPbElecZoom()
{
	for(int i=0; i<m_nCheckBox; i++)
	{
		if(pCheckBox[i]->GetValue() == TRUE)
		{
			u64 tmp = 0;
			BizPlayBackGetRealPlayChn(&tmp);
			if(tmp & (1 << i) )
			{
			#ifndef CHIP_HISI3531//csp modify 20130509
				if(((CPagePlayBackFrameWork *)(this->GetParent()))->GetPbPauseStatue())
				{
					this->Close();
					MessageBox("&CfgPtn.PlayBackFilePause", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
					m_nIsPbElecZoom = 0;
					return;
				}
				else
			#endif
				{
					int foramt = BizPlayBackGetVideoFormat(i);
					if(foramt < 0)
					{
						printf("The format of Real playback file is error!!!\n");
					}
					else
					{
						BizSetPlayBackFileFormat(foramt);
					}
					BizPlayBackZoom(i);
				}
			}
			else
			{
				this->Close();
				MessageBox("&CfgPtn.NoPlayBackFile", "&CfgPtn.WARNING" ,
	       		 MB_OK|MB_ICONWARNING);
				m_nIsPbElecZoom = 0;
				return;
			}
			
			for(int j=0;j<10;j++)
			{
				((CPageDesktop*)m_pDesktop)->ShowSplitLine(j, FALSE);
			}
			
			this->Close();
			
			((CPagePlayBackFrameWork *)(this->GetParent()))->SetZoomStatue(1);
			((CPagePlayBackFrameWork *)(this->GetParent()))->Close();

			((CPagePlayBackFrameWork *)(this->GetParent()))->SetIsZoomed(0);
			((CPageDesktop*)m_pDesktop)->SetElecZoomChn(i);
			((CPageDesktop*)m_pDesktop)->SetPbElecZoomstatus(1);
			((CPageDesktop*)m_pDesktop)->ShowElecZoomTile(0,TRUE);
			((CPageDesktop*)m_pDesktop)->SetCurVideoSize();
			
			break;
		}
	}
	
	for(int i=0; i<m_nCheckBox; i++)
	{
		if(pCheckBox[i]->GetValue() == TRUE)
		{
			nSelectChn[i] = TRUE;
		}
		else
		{
			nSelectChn[i] = FALSE;
		}
	}
	
	m_nIsPbElecZoom = 0;
}

void CPageChnSelect::StartManuelRec()
{
	CPage** page = GetPage();
	for(int i=0; i<m_nCheckBox; i++)
	{
		if((i % (ROW_ITEM+1)))
		{
			int RealChn = i-(i/(ROW_ITEM+1)+1);
			if(pCheckBox[i]->GetValue() == TRUE)
			{
				//printf("RealChn = %d\n",RealChn);
				((CPageStartFrameWork*)page[EM_PAGE_START])->SetRecordingStatue(RealChn,TRUE);
			}
			else
			{
				((CPageStartFrameWork*)page[EM_PAGE_START])->SetRecordingStatue(RealChn,FALSE);
			}
		}
	}
	
	((CPageStartFrameWork*)page[EM_PAGE_START])->DealManualRec();
	
	this->Close();
	m_nIsRecorDing = 0;
}

void CPageChnSelect::StartPreviewSelect()
{
	for(int i=0; i<m_nCheckBox; i++)
	{
		if(pCheckBox[i]->GetValue() == TRUE)
		{
			SetCurPreviewMode_CW(EM_BIZPREVIEW_1SPLIT);//cw_preview
			SwitchPreview(EM_BIZPREVIEW_1SPLIT, i);
			break;
		}
	}
	
	for(int i=0; i<m_nCheckBox; i++)
	{
		if(pCheckBox[i]->GetValue() == TRUE)
		{
			nSelectChn[i] = TRUE;
		}
		else
		{
			nSelectChn[i] = FALSE;
		}
	}
	
	this->Close();
}

void CPageChnSelect::SetDesktop(CPage* pDesktop)
{
	m_pDesktop = pDesktop;
}

void CPageChnSelect::SetSearchPage(CPageSearch* page)
{
	m_pSearchPage = page;
}

int CPageChnSelect::SetCurDwellStatue(int flag)
{
	m_nIsDwell = flag;
}

int CPageChnSelect::GetCurDwellStatue()
{
	return m_nIsDwell;
}

CPageChnSelect::~CPageChnSelect()
{
	
}

void CPageChnSelect::SetCurStatusPlaying(u8 flag)
{
	m_nIsPlaying = flag;
}

void CPageChnSelect::SetCurStatusSearching(u8 flag)
{
	m_nIsSearching = flag;
}

void CPageChnSelect::SetCurStatusElecZoom(u8 flag)
{
	m_nIsElecZoom= flag;
}

void CPageChnSelect::SetPbCurStatusElecZoom(u8 flag)
{
	m_nIsPbElecZoom= flag;
}

void CPageChnSelect::SetCurStatusRecorDing(u8 flag)
{
	m_nIsRecorDing = flag;
}

VD_BOOL CPageChnSelect::UpdateData( UDM mode )
{
 	if(mode == UDM_OPEN)
	{
		if(m_vstyle != 0)
		{
			int RealChn = 0;
			int CheckBoxAllValue = 1;
			
			SBizParaTarget bizTar;
			SBizRecPara bizRecPara;
			bizTar.emBizParaType = EM_BIZ_RECPARA;
			
			for(int i=0; i<m_nCheckBox; i++)
			{
				if((i % (ROW_ITEM+1)))
				{
					RealChn = i-(i/(ROW_ITEM+1)+1);
					bizTar.nChn = RealChn;
					
					if(0 != BizGetPara(&bizTar, &bizRecPara))
					{	
						printf("Get rec para error\n");
					}
					
					CheckBoxAllValue &= bizRecPara.bRecording;
					pCheckBox[i]->SetValue(bizRecPara.bRecording);
				}
			}
			
			if(m_nIsRecorDing)
			{
				pCheckBoxAll->SetValue(CheckBoxAllValue);
				pStaticAll->Show(TRUE);
				pCheckBoxAll->Show(TRUE);
			}
			
            for(int i = 1; i <= m_nLines; i++)
            {
                CheckVLineCheckBox(i);
            }
		}
	}
	
	return TRUE;
}

void  CPageChnSelect::GetSel(int* pStyle, int* pChMask)
{
	if(pStyle && pChMask)
	{
		*pStyle = m_vstyle;

		*pChMask = 0;
		if(m_vstyle==0)
		{
			for(int i=0; i<m_nCheckBox; i++)
			{
				if(pCheckBox[i]->GetValue())
				{
					*pChMask |= (1<<i);
					break;
				}
			}
		}
		else
		{
			for(int i=0; i<nChMax; i++)
			{
				if(nSelectChn[i])
				{
					*pChMask |= (1<<i);
					break;
				}
			}
		}
	}
}

void CPageChnSelect::CheckVLineCheckBox(int nLine)
{
	u8 nHeadChkIdx = (nLine-1)*(ROW_ITEM+1);
	
	for(int i=0; i<ROW_ITEM; i++)
	{
		//csp modify 20130504
		if(nHeadChkIdx+i+1 >= m_nCheckBox)
		{
			break;
		}
		
		if(!pCheckBox[nHeadChkIdx+i+1]->GetValue())
		{
			pCheckBox[nHeadChkIdx]->SetValue(FALSE);
			return;
		}
	}
	
	pCheckBox[nHeadChkIdx]->SetValue(TRUE);
}

VD_BOOL CPageChnSelect::MsgProc( uint msg, uint wpa, uint lpa )//cw_panel
{
    switch(msg)
	{
		case XM_KEYDOWN://cw_panel
		{
			switch(wpa)
			{
				case KEY_RET:
				case KEY_UP:
				case KEY_DOWN:
				case KEY_RIGHT:
				case KEY_LEFT:
				case KEY_ESC:
					break;
				default:
					return false;
			}
		}
		break;
		case XM_RBUTTONDOWN:
		case XM_RBUTTONDBLCLK:
		{
			m_nIsElecZoom = 0;
			m_nIsPbElecZoom = 0;
			m_nIsRecorDing = 0;
		}
		break;
    }
	return CPage::MsgProc(msg, wpa, lpa);
}

