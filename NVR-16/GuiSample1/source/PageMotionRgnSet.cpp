#include "GUI/Pages/PageMotionRgnSet.h"
#include "GUI/Pages/BizData.h"

#define BITMSK64(bit) (uint64)((uint64)1 << (bit))

static char* szBtBmpPath[5][2] = {
	{DATA_DIR"/temp/all.bmp", DATA_DIR"/temp/all_f.bmp"},
	{DATA_DIR"/temp/clear.bmp", DATA_DIR"/temp/clear_f.bmp"},
	{DATA_DIR"/temp/custom.bmp", DATA_DIR"/temp/custom_f.bmp"},
	{DATA_DIR"/temp/save.bmp", DATA_DIR"/temp/save_f.bmp"},
	{DATA_DIR"/temp/exit2.bmp", DATA_DIR"/temp/exit2_f.bmp"},
};

//cw_md
static u8 default_flag = 0;//csp modify
static CSliderCtrl *default_pSlider = NULL;//csp modify
static int default_pSlider_pos = 0;//csp modify
static uint64 default_region[64] = {0};//csp modify

CPageMotionRgnSet::CPageMotionRgnSet( VD_PCRECT pRect,VD_PCSTR psz /*=NULL*/,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPage(pRect, psz, pParent, pageFullScreen)
, m_nChannel(0)
{
	SetGrid(22, 18);
	row_start = 0;
	column_start = 0;
	row_end = 0;
	column_end = 0;
	row_start_old = 0;
	column_start_old = 0;
	row_end_old = 0;
	column_end_old = 0;
	IsDwell = 0;
	is_panel_show = FALSE;//cw_page
	m_Rgninit = FALSE;
	memset(region, 0, sizeof(region));
	memset(backup, 0, sizeof(backup));
	m_ColorSelected = VD_RGB(0,0,255);
	m_ColorEmpty = VD_RGB(255,0,0);//m_pDevGraphics->GetKeyColor();
	m_ColorEmpty = m_pDevGraphics->GetKeyColor();
	
	InitCtrlPanel();
}

CPageMotionRgnSet::~CPageMotionRgnSet()
{
	
}

VD_BOOL CPageMotionRgnSet::UpdateData( UDM mode )
{
	if(UDM_OPEN == mode)
	{
		//m_pPageSave->Open();
		m_Rgninit=TRUE;//cw_page
		if(GetDwellStartFlag())
		{
			IsDwell = 1;
			ControlPatrol(0);
		}
		BizSetZoomMode(1);//cw_zoom
		
		BizChnPreview(m_nChannel);
		
		if(default_flag == 1)//cw_md
		{
			printf("cw****get rgn\n");
			if(default_pSlider_pos == 0)
			{
				printf("cw_the first time\n");
				GetRgnCurDo(region, 
					GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET,
					EM_GSR_CTRL_RGNSET,
					m_nChannel
				);
				
				GetScrStrSetCurSelDo(
					(void*)pSlider,
					GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET_SENSITY, 
					EM_GSR_CTRL_SLIDE, 
					m_nChannel
				);
				memcpy(default_region,region,sizeof (region));
				default_pSlider_pos = pSlider->GetPos();
			}
			else
			{
				memcpy(region,default_region,sizeof (region));
				pSlider->SetPos(default_pSlider_pos);
			}
		}
		else
		{
			GetRgnCurDo(region, 
				GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET,
				EM_GSR_CTRL_RGNSET,
				m_nChannel
			);
				
			GetScrStrSetCurSelDo(
				(void*)pSlider,
				GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET_SENSITY, 
				EM_GSR_CTRL_SLIDE, 
				m_nChannel
			);
		}
		pTextInfo->SetText("");
		ShowCtrlPanel(1);
	}
	else if(UDM_CLOSED == mode)
	{
		BizSetZoomMode(0);//cw_zoom
		
		BizResumePreview(0);
		
		if(IsDwell)
		{
			usleep(500 * 1000);
			ControlPatrol(1);
			IsDwell = 0;
		}
	}
	
	return TRUE;
}

VD_BOOL CPageMotionRgnSet::MsgProc( uint msg, uint wpa, uint lpa )
{
	int r;
	int c;
	int key;
	int px, py;
	switch(msg) 
	{
		case XM_KEYDOWN:
		{
			switch(wpa)
			{
				case KEY_RIGHT:
				case KEY_LEFT:
				{
					CPage::MsgProc(msg, wpa,lpa);
					static CItem* last = NULL;
					
					CItem* temp = GetFocusItem();
					
					if(temp != last)
					{
						if(temp == pButton[0])
						{
							pTextInfo->SetText("&CfgPtn.SelectAll");
						}
						else if(temp == pButton[1])
						{
							pTextInfo->SetText("&CfgPtn.ClearAll");
						}
						//else if(temp == pButton[2])
						//{
						//	pTextInfo->SetText("&CfgPtn.Test");
						//}
						else if(temp == pButton[3])
						{
							pTextInfo->SetText("&CfgPtn.Save");
						}
						else if(temp == pButton[4])
						{
							pTextInfo->SetText("&CfgPtn.Exit");
						}
						else
						{
							pTextInfo->SetText("");
						}

						last = temp;
					}
					return TRUE;
				}break;
				default:
					break;
			}
		}break;

		case XM_LBUTTONDOWN:
		{
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);

			if (m_bPanelIsShown)
			{
				CRect rt;
				rt.left = (m_Rect.Width()-400)/2;
				rt.right = rt.left + 400;
				rt.bottom = m_Rect.Height() - 24;
				rt.top = rt.bottom - 60;
				if (!PtInRect(rt, px, py))
				{
					ShowCtrlPanel(FALSE);
					if(!GetFlag(IF_CAPTURED))
					{
						SetFlag(IF_CAPTURED, TRUE);
						if(GetAt(px, py, &column_start, &row_start)){
							//转换当前块的状态
							row_end = row_start;
							column_end = column_start;
							region[row_start] ^= BITMSK64(column_start);
							inset = TRUE;
						}
					}
				}
			}
			else
			{
				if(!GetFlag(IF_CAPTURED))
				{
					SetFlag(IF_CAPTURED, TRUE);
					if(GetAt(px, py, &column_start, &row_start)){
						//转换当前块的状态
						row_end = row_start;
						column_end = column_start;
						region[row_start] ^= BITMSK64(column_start);
						inset = TRUE;
					}
				}
			}
		}
		break;
		case XM_LBUTTONUP:
		{
			if(GetFlag(IF_CAPTURED))
			{
				SetFlag(IF_CAPTURED, FALSE);
				row_start = row_end;
				column_start = column_end;
				inset = FALSE;
			}
		}
		break;
		case XM_MOUSEMOVE:
		{
			px = VD_HIWORD(lpa);
			py = VD_LOWORD(lpa);
			if(inset)
			{
				GetAt(px, py, &column_end, &row_end);
			}

			static CItem* last = NULL;
			CItem* temp = GetItemAt(px, py);

			if(temp != last)
			{
				if(temp == pButton[0])
				{
					pTextInfo->SetText("&CfgPtn.SelectAll");
				}
				else if(temp == pButton[1])
				{
					pTextInfo->SetText("&CfgPtn.ClearAll");
				}
				//else if(temp == pButton[2])
				//{
				//	pTextInfo->SetText("&CfgPtn.Test");
				//}
				else if(temp == pButton[3])
				{
					pTextInfo->SetText("&CfgPtn.Save");
				}
				else if(temp == pButton[4])
				{
					pTextInfo->SetText("&CfgPtn.Exit");
				}
				else
				{
					pTextInfo->SetText("");
				}

				last = temp;

			}
		}
		break;
		case XM_RBUTTONDOWN:
			ShowCtrlPanel(!m_bPanelIsShown);
			return TRUE;
		break;
		default:
			return FALSE;
		}

	if(inset != FALSE)
	{
		VD_BOOL flag = (region[row_start] & BITMSK64(column_start))!=0;
//		printf("flag = %d row_start=%d column_start=%d\n", flag, row_start, column_start);
		int rmax = MAX(row_start,row_end);
		int rmin = MIN(row_start,row_end);
		int cmax = MAX(column_start,column_end);
		int cmin = MIN(column_start,column_end);
		for(r = rmin; r <= rmax; r++){
			for(c = cmin; c <= cmax; c++){
				if(flag){
					region[r] |= BITMSK64(c);
	//				printf("@@@@@@@@@@@region[r] = %lld,r=%d,c=%d\n", region[r],r,c);
				}else{
					region[r] &= ~BITMSK64(c);
				}
			}
		}
		DrawDynamic();
	}
	return CPage::MsgProc(msg, wpa, lpa);
//	return TRUE;
}

void CPageMotionRgnSet::Draw()
{
	row_end = row_start;
	column_end = column_start;
//	DrawDynamic(TRUE); //cw_page
	if(inset == TRUE ||m_Rgninit==TRUE)
	{
		m_Rgninit=FALSE;
		DrawDynamic(TRUE);
	}
	else
		DrawDynamic(FALSE);
	
	//printf("CPageMotionRgnSet::Draw() col = %d\n", column);
	CPage::Draw();
}

void CPageMotionRgnSet::DrawDynamic( VD_BOOL all /*= FALSE*/ )
{
	int r;
	int c;
	int height = m_Rect.Height();
	int width = m_Rect.Width();
//	printf("CPageMotionRgnSet::DrawDynamic w=%d \n", width);
	//只画修改块
	m_DC.Lock();
	#if 0
	for(r = 0; r < row; r++)
	{
		for(c = 0; c < column; c++)
		{
			if(all || (region[r] & BITMSK64(c))!=(backup[r] & BITMSK64(c))){
				//printf("CPageMotionRgnSet::DrawDynamic begin r=%d,c=%d,region[r]=%x backup[r]=%x\n",r,c,region[r],backup[r]);
				if(region[r] & BITMSK64(c)){
					m_DC.SetBrush(m_ColorSelected);
				}else{
					m_DC.SetBrush(m_ColorEmpty);
				}
				m_DC.SetRgnStyle(RS_FLAT);
				m_DC.Rectangle(CRect(c*width / column, r*height / row ,
					(c + 1)*width / column, (r + 1)*height / row));
				m_DC.SetBrush(m_pDevGraphics->GetKeyColor());

				m_DC.SetRgnStyle(RS_FLAT);
				m_DC.Rectangle(CRect(c*width / column+1,r*height / row+1,(c+1)*width / column-1,(r+1)*height / row-1));
			}			
		}
	}
	#else
//	r=is_panel_show?(row-3):0;//cw_page 3 rows for refresh panel
	if(all&&(!is_panel_show))
	{
		int drawLength=0;
		m_DC.SetBrush(m_pDevGraphics->GetKeyColor());
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
		m_DC.SetBrush(m_ColorSelected);
		m_DC.SetRgnStyle(RS_FLAT);
		
		for(r=0; r < row; r++)
		{
			for(c = 0; c <= column; c++)
			{
				if((c!=column)&&(region[r] & BITMSK64(c)))
				{
					drawLength++;
					continue;
				}
				else
				{
					m_DC.Rectangle(CRect((c-drawLength)*width /column, r*height /row ,c*width /column, r*height /row+1));
					m_DC.Rectangle(CRect((c-drawLength)*width /column, (r+1)*height /row-1 ,c*width /column, (r+1)*height /row));
					drawLength=0;
				}
			}
		}
		
		for(c=0; c < column; c++)
		{
			for(r= 0; r <= row ; r++)
			{
				if((r!=row)&&(region[r] & BITMSK64(c)))
				{
					drawLength++;
					continue;
				}
				else
				{
					m_DC.Rectangle(CRect(c*width /column, (r-drawLength)*height /row ,c*width /column+1, r*height /row));
					m_DC.Rectangle(CRect((c+1)*width /column-1, (r-drawLength)*height /row ,(c+1)*width /column, r*height /row));
					drawLength=0;
				}
			}
		}
		GetMask(backup);
	}
	r=is_panel_show?(row-3):0;//cw_page 3 rows for refresh panel
	for( ; r < row; r++)
	{
		for(c = 0; c < column; c++)
		{
			if(is_panel_show||(region[r] & BITMSK64(c))!=(backup[r] & BITMSK64(c)))
			{
				if(region[r] & BITMSK64(c)){
					m_DC.SetBrush(m_ColorSelected);
				}else{
					m_DC.SetBrush(m_ColorEmpty);
				}
				m_DC.SetRgnStyle(RS_FLAT);
				m_DC.Rectangle(CRect(c*width / column, r*height / row ,
					(c + 1)*width / column, (r + 1)*height / row));
				m_DC.SetBrush(m_pDevGraphics->GetKeyColor());
				
				m_DC.SetRgnStyle(RS_FLAT);
				m_DC.Rectangle(CRect(c*width / column+1,r*height / row+1,(c+1)*width / column-1,(r+1)*height / row-1));
			}	
		}
	}
	#endif
	row_start_old = row_start;
	column_start_old = column_start;
	row_end_old = row_end;
	column_end_old = column_end;
	m_DC.UnLock();
	GetMask(backup);
}

VD_BOOL CPageMotionRgnSet::GetAt( int px, int py, int* pc, int* pr )
{
	int r;
	int c;

	r = (py - m_Rect.top) * row / m_Rect.Height();
	c = (px - m_Rect.left) * column / m_Rect.Width();
	if(r < 0)
	{
		r = 0;
	}
	else if (r >= row)
	{
		r = row - 1;
	}
	if(c < 0)
	{
		c = 0;
	}
	else if (c >= column)
	{
		c = column - 1;
	}
	*pc = c;
	*pr = r;

	return TRUE;
}

void CPageMotionRgnSet::SetGrid( int vcolumn, int vrow )
{
	column = MIN(vcolumn,64);
	row = MIN(vrow,64);
}

void CPageMotionRgnSet::SetMask( uint64* p )
{
	memcpy(region, p, sizeof(uint64)*MIN(row,64));
//	Draw();
	DrawDynamic(TRUE);//cw_page
}

void CPageMotionRgnSet::GetMask( uint64* p )
{
	memcpy(p, region, sizeof(uint64)*MIN(row,64));
}

void CPageMotionRgnSet::SetColor( VD_COLORREF selected, VD_COLORREF empty )
{
	m_ColorSelected = selected;
	m_ColorEmpty = empty;
}

void CPageMotionRgnSet::SetMotionChn( int nChn )
{
	int nChMax = GetVideoMainNum();  //cw_md
	if ( nChn == -1)
	{
		default_flag = 1;
		m_nChannel = 0;   //the default set base on the first channel
	}
	else
		m_nChannel = nChn;
}

void CPageMotionRgnSet::InitCtrlPanel()
{
	CRect rtTab;
	rtTab.left = (m_Rect.Width()-400)/2;
	rtTab.right = rtTab.left + 400;
	rtTab.bottom = m_Rect.Height() - 24;
	rtTab.top = rtTab.bottom - 60;
	pTabFrame = CreateTableBox(rtTab, this, 1, 1);
	m_pItemPanel.push_back(pTabFrame);	

	CRect rt;
	rt.left = rtTab.left +5;
	rt.right = rt.left + TEXT_WIDTH*3;
	rt.top = rtTab.top + 5;
	rt.bottom = rt.top + TEXT_HEIGHT;
	CStatic* pTextSensitive = CreateStatic(rt, this, "&CfgPtn.Sensitivity");
	m_pItemPanel.push_back(pTextSensitive);	

	rt.left = rt.right;
	rt.right = rt.left + 100;
	pSlider = CreateSliderCtrl(rt,this, 1, 5);
	default_pSlider = pSlider;//cw_md
	pSlider->SetPos(6);
	m_pItemPanel.push_back(pSlider);	

	const int btwidth = 30;
	const int btheight = 25;
	const int btspace = 10;
	
	rt.left = rtTab.right - 5 -btwidth*5 -btspace*4;
	rt.right = rt.left + btwidth;
	rt.bottom = rt.top + btheight;

	for (int i =0; i<5; i++)
	{
		if(i == 2)
			continue;
		
		pButton[i] = CreateButton(rt, this, NULL, (CTRLPROC)&CPageMotionRgnSet::OnClkCtrlPanel, NULL, buttonNormalBmp);
		pButton[i]->SetBitmap(VD_LoadBitmap(szBtBmpPath[i][0]), VD_LoadBitmap(szBtBmpPath[i][1]), VD_LoadBitmap(szBtBmpPath[i][1]));

		rt.left = rt.right + btspace;
		rt.right = rt.left + btwidth;
		m_pItemPanel.push_back(pButton[i]);	
	}

	rt.left = rtTab.left + 5;
	rt.right = rt.left + 200;
	rt.top = rt.bottom + 4;
	rt.bottom = rt.top + 20;
	pTextInfo = CreateStatic(rt, this, "");
	m_pItemPanel.push_back(pTextInfo);	

	m_bPanelIsShown = TRUE;
}

void CPageMotionRgnSet::ShowCtrlPanel( BOOL bShow )
{
	is_panel_show=TRUE;//cw_page
	m_bPanelIsShown = bShow;

	int count = m_pItemPanel.size();

	if(bShow)
	{
		for(int i=0; i<count; i++)
		{
			if(m_pItemPanel[i])
			{
				m_pItemPanel[i]->Show(bShow);
			}
		}
	}
	else
	{
		for(int i=count-1; i>=0; i--)
		{
			if(m_pItemPanel[i])
			{
				m_pItemPanel[i]->Show(bShow);
			}
		}
	}
	is_panel_show= FALSE;//cw_page
}

void CPageMotionRgnSet:: SaveAllChnRgnPara()   //cw_md
{	
	if ( default_pSlider_pos == 0)
	{
		printf("cw_the first time\n");
		GetRgnCurDo(region, 
		GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET,
		EM_GSR_CTRL_RGNSET,
		m_nChannel
		);
		
		GetScrStrSetCurSelDo(
			(void*)pSlider,
			GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET_SENSITY, 
			EM_GSR_CTRL_SLIDE, 
			m_nChannel
		);
		memcpy(default_region,region,sizeof (region));
		default_pSlider_pos = pSlider->GetPos();
	}
	default_pSlider->SetPos(default_pSlider_pos);
	for(int i = 0;i<GetVideoMainNum();i++)
	{
		SaveScrStr2Cfg(default_pSlider,
			GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET_SENSITY, 
			EM_GSR_CTRL_SLIDE, 
			i
		);
		
		SaveScrStr2Cfg(default_region,
			GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET, 
			EM_GSR_CTRL_RGNSET, 
			i
		);
	}
}

void CPageMotionRgnSet::OnClkCtrlPanel()
{
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for (i = 0; i < 5; i++)
	{
		if (pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}

	uint64 tmp[64];
	if (bFind)
	{	
		switch (i)
		{
		case 0:  //all
			{
				memset(tmp, -1, sizeof(tmp));
				SetMask(tmp);
				ShowCtrlPanel(1);//cw_page
			}
			break;
		case 1:  //claer
			{
				memset(tmp, 0, sizeof(tmp));
				SetMask(tmp);
				ShowCtrlPanel(1);//cw_page
			}
			break;
		case 2:  //test
			{

			}
			break;
		case 3:  //save
			{
				if (default_flag == 1)  //cw_md
				{	
					printf("cw****save rgn\n");
					GetMask(tmp);
					memcpy(default_region,tmp,sizeof (tmp));
					default_pSlider_pos = pSlider->GetPos();
					default_flag = 0;
				}
				else
				{
					SaveScrStr2Cfg(pSlider,
						GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET_SENSITY, 
						EM_GSR_CTRL_SLIDE, 
						m_nChannel
					);
					GetMask(tmp);
					SaveScrStr2Cfg(tmp,
						GSR_CONFIG_ALARM_MD_MOTIONDETECT_RGNSET, 
						EM_GSR_CTRL_RGNSET, 
						m_nChannel
					);
				}

			}
			break;
		case 4:  //close
			{
				if ( default_flag == 1)  //cw_md
					default_flag = 0;
					
				ShowCtrlPanel(0);
				Close();
			}
			break;
		}
	}
}

