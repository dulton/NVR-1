//
//  "$Id: DateTimeCtrl.cpp 294 2008-12-18 05:52:36Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#include "GUI/Ctrls/Page.h"
//#include "Configs/ConfigLocation.h"
#define DTC_WIDTH  (TEXT_WIDTH*25/2)

VD_BITMAP* CDateTimeCtrl::m_bmpNormal = NULL;
VD_BITMAP* CDateTimeCtrl::m_bmpSelected = NULL;
VD_BITMAP* CDateTimeCtrl::m_bmpPushed = NULL;
VD_BITMAP* CDateTimeCtrl::m_bmpDisabled = NULL;

#define DRAWTIMECOMM1 (pnb[3] && pnb[3]->GetFlag(IF_SHOWN) && pnb[4] && pnb[4]->GetFlag(IF_SHOWN))
#define DRAWTIMECOMM2 (pnb[4] && pnb[4]->GetFlag(IF_SHOWN) && pnb[5] && pnb[5]->GetFlag(IF_SHOWN))
#define DRAWDATECOMM1 (pnb[0] && pnb[0]->GetFlag(IF_SHOWN) && pnb[1] && pnb[1]->GetFlag(IF_SHOWN))
#define DRAWDATECOMM2 (pnb[1] && pnb[1]->GetFlag(IF_SHOWN) && pnb[2] && pnb[2]->GetFlag(IF_SHOWN))

///////////////////////////////////////////////////////////////////
////////////        CDateTimeCtrl
 
CDateTimeCtrl::CDateTimeCtrl(VD_PCRECT pRect, CPage * pParent, CTRLPROC vproc /* = NULL */, uint dwStyle/* = DTC_NORMAL*/)
:CItem(pRect, pParent, IT_DATETIMECTRL,dwStyle|styleAutoFocus)//, FALSE)
{
	CRect m_rect;
	int i;
	m_vProc = vproc;
	datefmt = DF_YYMMDD;

	for (i = 0; i < 6; i++)
	{
		pnb[i]= 0;
	}
	pcb = 0;
	single = FALSE;//初始化,不显示分隔符
	m_dtStyle = dwStyle; 
	
	if(pRect != NULL)
	{
		int iScreenShift = pRect->left + TEXTBOX_X;
		if ((dwStyle & DTS_DATE) && (dwStyle & DTS_TIME))
		{
			m_iDtcNumWidth = (pRect->right - pRect->left) / DTC_ITEMS;
			m_rect.SetRect(iScreenShift,pRect->top+2,iScreenShift+m_iDtcNumWidth*4,pRect->bottom-2);
			pnb[0] = CreateNumberBox(m_rect, pParent,2000,2037,"%4d",NULL,(CTRLPROC)&CDateTimeCtrl::OnDateChanged,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
			m_rect.SetRect(iScreenShift+m_iDtcNumWidth*5,pRect->top+2,iScreenShift+m_iDtcNumWidth*7,pRect->bottom-2);
			pnb[1] = CreateNumberBox(m_rect, pParent,1,12,"%02d",NULL,(CTRLPROC)&CDateTimeCtrl::OnDateChanged,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
			m_rect.SetRect(iScreenShift+m_iDtcNumWidth*8,pRect->top+2,iScreenShift+m_iDtcNumWidth*10,pRect->bottom-2);
			pnb[2] = CreateNumberBox(m_rect, pParent,1,31,"%02d",NULL,NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
			if(pRect->right - pRect->left > TEXT_WIDTH*8 && pRect->bottom - pRect->top < CTRL_HEIGHT*2)
			{
				single = TRUE;//swhat 's the "single" mean??? remarked by shaowy
				m_rect.SetRect(iScreenShift+m_iDtcNumWidth*11,pRect->top+2,iScreenShift+m_iDtcNumWidth*13,pRect->bottom-2);
				pnb[3] = CreateNumberBox(m_rect, pParent,0,23,"%02d",NULL,(CTRLPROC)&CDateTimeCtrl::OnHourChanged,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
				m_rect.SetRect(iScreenShift+m_iDtcNumWidth*14,pRect->top+2,iScreenShift+m_iDtcNumWidth*16,pRect->bottom-2);
				pnb[4] = CreateNumberBox(m_rect, pParent,0,59,"%02d",NULL,NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
				m_rect.SetRect(iScreenShift+m_iDtcNumWidth*17,pRect->top+2,iScreenShift+m_iDtcNumWidth*19,pRect->bottom-2);
				pnb[5] = CreateNumberBox(m_rect, pParent,0,59,"%02d",NULL,NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
				m_rect.SetRect(iScreenShift+m_iDtcNumWidth*39/2,pRect->top+2,pRect->right - TEXTBOX_X,pRect->bottom-2);
				pcb = CreateComboBox(m_rect, pParent,"AM|PM",NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging1, styleNoBorder | comboNoButton);
			}
			else
			{
				single = FALSE;
				m_rect.SetRect(iScreenShift,(pRect->top+pRect->bottom)/2+2,iScreenShift+m_iDtcNumWidth*2,pRect->bottom-2);
				pnb[3] = CreateNumberBox(m_rect, pParent,0,23,"%02d",NULL,NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
				m_rect.SetRect(iScreenShift+m_iDtcNumWidth*3,(pRect->top+pRect->bottom)/2+2,iScreenShift+m_iDtcNumWidth*5,pRect->bottom-2);
				pnb[4] = CreateNumberBox(m_rect, pParent,0,59,"%02d",NULL,NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
				m_rect.SetRect(iScreenShift+m_iDtcNumWidth*6,(pRect->top+pRect->bottom)/2+2,iScreenShift+m_iDtcNumWidth*8,pRect->bottom-2);
				pnb[5] = CreateNumberBox(m_rect, pParent,0,59,"%02d",NULL,NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
				m_rect.SetRect(iScreenShift+m_iDtcNumWidth*9,(pRect->top+pRect->bottom)/2+2,iScreenShift+m_iDtcNumWidth*12,pRect->bottom-2);
				pcb = CreateComboBox(m_rect, pParent,"AM|PM",NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging1, styleNoBorder | comboNoButton);
			}
		       
			for(i = 0; i < 6; i++)
			{
				pnb[i]->SetOwner(this);
			}
			pcb->SetOwner(this);
			pcb->SetCurSel(-1);
		}
		else if ((dwStyle & DTS_DATE))
		{
			m_iDtcNumWidth = (pRect->right - pRect->left) / 10 - 1;
			pnb[0] = new CNumberBox(CRect(iScreenShift,pRect->top+2,iScreenShift+m_iDtcNumWidth*4,pRect->bottom-2), pParent,2000,2037,"%4d",NULL,(CTRLPROC)&CDateTimeCtrl::OnDateChanged,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
			pnb[1] = new CNumberBox(CRect(iScreenShift+m_iDtcNumWidth*5,pRect->top+2,iScreenShift+m_iDtcNumWidth*7,pRect->bottom-2), pParent,1,12,"%02d",NULL,(CTRLPROC)&CDateTimeCtrl::OnDateChanged,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
			pnb[2] = new CNumberBox(CRect(iScreenShift+m_iDtcNumWidth*8,pRect->top+2,iScreenShift+m_iDtcNumWidth*10,pRect->bottom-2), pParent,1,31,"%02d",NULL,NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
			for(i = 0; i < 3; i++)
			{
				pnb[i]->SetOwner(this);
			}
		}
		else if ((dwStyle & DTS_TIME))
		{
			m_iDtcNumWidth = (pRect->right - pRect->left) / 11;
			pnb[3] = new CNumberBox(CRect(iScreenShift,pRect->top+2,iScreenShift+m_iDtcNumWidth*2,pRect->bottom-2), pParent,0,23,"%02d",NULL,(CTRLPROC)&CDateTimeCtrl::OnHourChanged,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
			pnb[4] = new CNumberBox(CRect(iScreenShift+m_iDtcNumWidth*3,pRect->top+2,iScreenShift+m_iDtcNumWidth*5,pRect->bottom-2), pParent,0,59,"%02d",NULL,NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
			pnb[5] = new CNumberBox(CRect(iScreenShift+m_iDtcNumWidth*6,pRect->top+2,iScreenShift+m_iDtcNumWidth*8,pRect->bottom-2), pParent,0,59,"%02d",NULL,NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
			pcb = new CComboBox(CRect(iScreenShift+m_iDtcNumWidth*8,pRect->top+2,iScreenShift+m_iDtcNumWidth*10+4,pRect->bottom-2), pParent,"AM|PM",NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging1,styleNoBorder | comboNoButton);
			for(i = 3; i < 6; i++)
			{
				pnb[i]->SetOwner(this);
			}
	  		 pcb->SetOwner(this);
	  		 pcb->Enable(FALSE);
	  		 pcb->SetCurSel(-1);
		}
		else
		{
			assert(0);
		}
		m_iDtcLinePos = m_iDtcNumWidth * 21 / 2 + TEXTBOX_X;
	}
	else
	{
		int NumberBox_width;
		m_iDtcNumWidth = DTC_WIDTH / DTC_ITEMS;
		m_iDtcLinePos = m_iDtcNumWidth * 21 / 2 + TEXTBOX_X;
		NumberBox_width = m_iDtcNumWidth*2;
		m_vProc = vproc;
		datefmt = DF_YYMMDD;
		if (m_dtStyle & DTS_DATE)
		{
			m_rect.SetRect(0,0,NumberBox_width*2,CTRL_HEIGHT-4);
			pnb[0] = CreateNumberBox(m_rect, pParent,2000,2037,"%4d",NULL,(CTRLPROC)&CDateTimeCtrl::OnDateChanged,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
			m_rect.SetRect(0,0,NumberBox_width,CTRL_HEIGHT-4);
			pnb[1] = CreateNumberBox(m_rect, pParent,1,12,"%02d",NULL,(CTRLPROC)&CDateTimeCtrl::OnDateChanged,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
			m_rect.SetRect(0,0,NumberBox_width,CTRL_HEIGHT-4);
			pnb[2] = CreateNumberBox(m_rect, pParent,1,31,"%02d",NULL,NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
		}
		if (m_dtStyle & DTS_TIME)
		{
			single = TRUE;
			m_rect.SetRect(0,0,NumberBox_width,CTRL_HEIGHT-4);
			pnb[3] = CreateNumberBox(m_rect, pParent,0,23,"%02d",NULL,(CTRLPROC)&CDateTimeCtrl::OnHourChanged,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
			m_rect.SetRect(0,0,NumberBox_width,CTRL_HEIGHT-4);
			pnb[4] = CreateNumberBox(m_rect, pParent,0,59,"%02d",NULL,NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
			m_rect.SetRect(0,0,NumberBox_width,CTRL_HEIGHT-4);
			pnb[5] = CreateNumberBox(m_rect, pParent,0,59,"%02d",NULL,NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging,styleNoBorder);
			m_rect.SetRect(0,0,NumberBox_width+m_iDtcNumWidth - 6,CTRL_HEIGHT-4);
			pcb = CreateComboBox(m_rect, pParent,"AM|PM",NULL,(CTRLPROC)&CDateTimeCtrl::OnTimeChanging1, styleNoBorder | comboNoButton);
		}

		for(i = 0; i < 6; i++)
		{
			if (pnb[i])
			{
				pnb[i]->SetOwner(this);
			}
		}
		if (pcb)
		{
			pcb->SetOwner(this);
			pcb->SetCurSel(-1);	
		}
	}

	//默认为24小时制
	timefmt = TF_24;
	//pcb->Enable(FALSE);
	

	width = CComboBox::m_bmpNormal ? 1:2; //暂时用CComboBox::m_bmpNormal来设置width，以后会改掉

	if (pnb[0] && !(m_dtStyle & DTS_YEAR)) 
	{
		pnb[0]->Show(FALSE);
	}
	if (pnb[1] && !(m_dtStyle & DTS_MONTH)) 
	{
		pnb[1]->Show(FALSE);
	}
	if (pnb[2] && !(m_dtStyle & DTS_DAY)) 
	{
		pnb[2]->Show(FALSE);
	}
	if (pnb[3] && !(m_dtStyle & DTS_HOUR)) 
	{
		pnb[3]->Show(FALSE);
		if (pcb)
		{
			pcb->Show(FALSE);
		}
	}
	if (pnb[4] && !(m_dtStyle & DTS_MINUTE)) 
	{
		pnb[4]->Show(FALSE);
	}
	if (pnb[5] && !(m_dtStyle & DTS_SECOND)) 
	{
		pnb[5]->Show(FALSE);
	}

	m_BkColor = VD_GetSysColor(VD_COLOR_WINDOW);
}

CDateTimeCtrl::~CDateTimeCtrl()
{
}


VD_SIZE& CDateTimeCtrl::GetDefaultSize(void)
{
	static VD_SIZE size = {TEXT_WIDTH*25/2,CTRL_HEIGHT};
	return size;
}

CDateTimeCtrl* CreateDateTimeCtrl(VD_PCRECT pRect, CPage * pParent, CTRLPROC vproc, uint dwStyle )
{
	return new CDateTimeCtrl(pRect, pParent, vproc, dwStyle);
}

VD_BOOL CDateTimeCtrl::Open()
{
// 	SetTimeFormat(CConfigLocation::getLatest().iDateFormat,
// 		CConfigLocation::getLatest().iDateSeparator,
// 		CConfigLocation::getLatest().iTimeFormat);
	return CItem::Open();
}

void CDateTimeCtrl::SetRect(VD_PCRECT pRect, VD_BOOL bReDraw)
{
	CItem::SetRect(pRect, bReDraw);

	if (bReDraw == FALSE)
	{
		return;
	}
	
	if ((m_dtStyle & DTS_DATE) && (m_dtStyle & DTS_TIME))
	{
		CRect rect[7];
		int i;
		rect[0].SetRect(0, 0, m_iDtcNumWidth * 4, m_Rect.bottom - m_Rect.top -4);
		rect[1].SetRect(0, 0, m_iDtcNumWidth * 2, m_Rect.bottom - m_Rect.top -4);
		rect[2].SetRect(0, 0, m_iDtcNumWidth * 2, m_Rect.bottom - m_Rect.top -4);
		rect[3].SetRect(0, 0, m_iDtcNumWidth * 2, m_Rect.bottom - m_Rect.top -4);
		rect[4].SetRect(0, 0, m_iDtcNumWidth * 2, m_Rect.bottom - m_Rect.top -4);
		rect[5].SetRect(0, 0, m_iDtcNumWidth * 2, m_Rect.bottom - m_Rect.top -4);
		rect[6].SetRect(0, 0, m_iDtcNumWidth * 3, m_Rect.bottom - m_Rect.top -4);
		if (pnb[0]->GetFlag(IF_SHOWN) == FALSE)
		{
			rect[0].SetRect(0, 0, 0, 0);
		}
		if (pnb[3]->GetFlag(IF_SHOWN) == FALSE)
		{
			rect[3].SetRect(0, 0, 0, 0);
		}

		for(i = 0; i < 7; i++)
		{
			// 偏移量要和m_Margin.left 和m_Margin.top 相互对应
			rect[i].OffsetRect(m_Rect.left + TEXTBOX_X, m_Rect.top + 2);
		}
		switch(datefmt) 
		{
		case DF_YYMMDD:
			rect[0].OffsetRect(0, 0);
			rect[1].OffsetRect(m_iDtcNumWidth * 5, 0);
			rect[2].OffsetRect(m_iDtcNumWidth * 8, 0);
			break;
		case DF_MMDDYY:
			rect[0].OffsetRect(m_iDtcNumWidth * 6, 0);
			rect[1].OffsetRect(0, 0);
			rect[2].OffsetRect(m_iDtcNumWidth * 3, 0);
			break;
		case DF_DDMMYY:
			rect[0].OffsetRect(m_iDtcNumWidth * 6, 0);
			rect[1].OffsetRect(m_iDtcNumWidth * 3, 0);
			rect[2].OffsetRect(0, 0);
			break;
		default:
			break;
		}
		rect[3].OffsetRect(m_iDtcNumWidth * 11, 0);
		rect[4].OffsetRect(m_iDtcNumWidth * 14, 0);
		rect[5].OffsetRect(m_iDtcNumWidth * 17, 0);
		rect[6].OffsetRect(m_iDtcNumWidth * 39 / 2, 0);

		for(i = 0; i < 6; i++)
		{
			pnb[i]->SetRect(rect[i], TRUE);
		}
		pcb->SetRect(rect[6], TRUE);
    }
	else if (m_dtStyle & DTS_DATE) 
   	{
		CRect rect[7];
		int i;
		rect[0].SetRect(0, 0, m_iDtcNumWidth * 4, m_Rect.bottom - m_Rect.top -4);
		rect[1].SetRect(0, 0, m_iDtcNumWidth * 2, m_Rect.bottom - m_Rect.top -4);
		rect[2].SetRect(0, 0, m_iDtcNumWidth * 2, m_Rect.bottom - m_Rect.top -4);
		if (pnb[0]->GetFlag(IF_SHOWN) == FALSE)
		{
			rect[0].SetRect(0, 0, 0, 0);
		}

		for(i = 0; i < 3; i++)
		{
			rect[i].OffsetRect(m_Rect.left + TEXTBOX_X, m_Rect.top + 2);
		}
		switch(datefmt) 
		{
			case DF_YYMMDD:
				rect[0].OffsetRect(0, 0);
				rect[1].OffsetRect(m_iDtcNumWidth * 5, 0);
				rect[2].OffsetRect(m_iDtcNumWidth * 8, 0);
				break;
			case DF_MMDDYY:
				rect[0].OffsetRect(m_iDtcNumWidth * 6, 0);
				rect[1].OffsetRect(0, 0);
				rect[2].OffsetRect(m_iDtcNumWidth * 3, 0);
				break;
			case DF_DDMMYY:
				rect[0].OffsetRect(m_iDtcNumWidth * 6, 0);
				rect[1].OffsetRect(m_iDtcNumWidth * 3, 0);
				rect[2].OffsetRect(0, 0);
				break;
			default:
				break;
		}
		for(i = 0; i < 3; i++)
		{
			pnb[i]->SetRect(rect[i], TRUE);
		}
		
   }
   else if (m_dtStyle & DTS_TIME)
   {
        CRect rect[7];
		int i;
		rect[3].SetRect(0, 0, m_iDtcNumWidth * 2, m_Rect.bottom - m_Rect.top -4);
		rect[4].SetRect(0, 0, m_iDtcNumWidth * 2, m_Rect.bottom - m_Rect.top -4);
		rect[5].SetRect(0, 0, m_iDtcNumWidth * 2, m_Rect.bottom - m_Rect.top -4);
		rect[6].SetRect(0, 0, m_iDtcNumWidth * 3, m_Rect.bottom - m_Rect.top -4);
		if (pnb[3]->GetFlag(IF_SHOWN) == FALSE)
		{
			rect[3].SetRect(0, 0, 0, 0);
		}

		for(i = 3; i < 7; i++)
		{
			rect[i].OffsetRect(m_Rect.left + TEXTBOX_X, m_Rect.top + 2);
		}
		rect[3].OffsetRect(m_iDtcNumWidth * 0, 0);
		rect[4].OffsetRect(m_iDtcNumWidth * 3, 0);
		rect[5].OffsetRect(m_iDtcNumWidth * 6, 0);
		rect[6].OffsetRect(m_iDtcNumWidth * 28 / 2, 0);
		for(i = 3; i < 6; i++)
		{
			pnb[i]->SetRect(rect[i], TRUE);
		}
		pcb->SetRect(rect[6], TRUE);
   	}
}
int CDateTimeCtrl::GetDaysOfMonth(int year,int monthth)
{
	if(monthth == 2)
	{
		if  ((year%4==0 && year %100!=0)||(year%400 ==0))
		{
			return 29;
		}
		else
		{
			return 28;
		}
	}
	if(monthth == 1||monthth == 3||monthth == 5||monthth == 7||monthth == 8||monthth == 10||monthth == 12)
		return 31;
	return 30;
}

void CDateTimeCtrl::OnDateChanged()
{
	if ((m_dtStyle & DTS_DATE) && (m_dtStyle & DTS_TIME))
	{
		int year = pnb[0]->GetValue();
		int monthth = pnb[1]->GetValue();
		int maxday = GetDaysOfMonth(year, monthth);
		pnb[2]->SetRange(1, maxday);
	}
	else if (m_dtStyle & DTS_DATE)      
	{
		int year = pnb[0]->GetValue();
		int monthth = pnb[1]->GetValue();
		int maxday = GetDaysOfMonth(year, monthth);
		pnb[2]->SetRange(1, maxday);

	}
	else if (m_dtStyle & DTS_TIME) 
	{
		;
	}
	else
	{
		assert(0);
	}
}

void CDateTimeCtrl::OnHourChanged()
{
	if ((m_dtStyle & DTS_DATE) && (m_dtStyle & DTS_TIME))       
	{
		time.hour = pnb[3]->GetValue();
		if(timefmt == TF_12)
		{
			time.hour = Hour12To24(time.hour, pcb->GetCurSel());
		}
	}
	else if (m_dtStyle & DTS_TIME)
	{
		time.hour = pnb[3]->GetValue();
		if(timefmt == TF_12)
		{
			time.hour = Hour12To24(time.hour, pcb->GetCurSel());
		}
	}
	else
	{
		assert(0);
	}
}

void CDateTimeCtrl::OnTimeChanging()
{
	ItemProc(m_vProc);
}

void CDateTimeCtrl::OnTimeChanging1()
{
    //printf("OnTimeChanging****************\n");
    time.hour = Hour12To24(time.hour,  pcb->GetCurSel());
    //printf("time.hour = %d********\n", time.hour);
	ItemProc(m_vProc);
}

void CDateTimeCtrl::Draw()
{
	if(!DrawCheck())
	{
		return;
	}
	m_DC.Lock();
	//m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
	m_DC.SetBrush(m_BkColor);
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
	
	m_DC.SetBkMode(BM_TRANSPARENT);
	if(!GetFlag(IF_ENABLED))
	{
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTDISABLED));
	}
	else
	{
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXT));
	}

	m_DC.UnLock();
	DrawSplit();
	DrawFrame();
}
void CDateTimeCtrl::DrawSplit()
{
	m_DC.Lock();
	char spector[8];

	switch(datesprtr) {
	case DS_DOT:
		strcpy(spector,".");
		break;
	case DS_DASH:
		strcpy(spector,"-");
		break;
	case DS_SLASH:
		strcpy(spector,"/");
		break;
	default:
		strcpy(spector,"-");
		break;
	}
	if(datefmt == DF_YYMMDD)
	{
		if (DRAWDATECOMM1)
		{
			m_DC.VD_TextOut(CRect(TEXTBOX_X+m_iDtcNumWidth*4,0,TEXTBOX_X+m_iDtcNumWidth*5,m_Rect.Height()),spector);
		}
		if (DRAWDATECOMM2)
		{
			m_DC.VD_TextOut(CRect(TEXTBOX_X+m_iDtcNumWidth*7,0,TEXTBOX_X+m_iDtcNumWidth*8,m_Rect.Height()),spector);
		}
	}else{
		if (DRAWDATECOMM1)
		{
			m_DC.VD_TextOut(CRect(TEXTBOX_X+m_iDtcNumWidth*2,0,TEXTBOX_X+m_iDtcNumWidth*3,m_Rect.Height()),spector);
		}
		if (DRAWDATECOMM2)
		{
			m_DC.VD_TextOut(CRect(TEXTBOX_X+m_iDtcNumWidth*5,0,TEXTBOX_X+m_iDtcNumWidth*6,m_Rect.Height()),spector);
		}
	}
	if ((m_dtStyle & DTS_DATE) && (m_dtStyle & DTS_TIME))
	{
		if(single)
		{
			if (DRAWTIMECOMM1)
			{
				 m_DC.VD_TextOut(CRect(TEXTBOX_X+m_iDtcNumWidth*13,0,TEXTBOX_X+m_iDtcNumWidth*15,m_Rect.Height()),":");
			}
			if (DRAWTIMECOMM2)
			{
				m_DC.VD_TextOut(CRect(TEXTBOX_X+m_iDtcNumWidth*16,0,TEXTBOX_X+m_iDtcNumWidth*18,m_Rect.Height()),":");
			}
		}
		else
		{
			if (DRAWDATECOMM1)
			{
				m_DC.VD_TextOut(CRect(TEXTBOX_X+m_iDtcNumWidth*3,m_Rect.Height()/2,TEXTBOX_X+m_iDtcNumWidth*4,m_Rect.Height()),":");
			}
			if (DRAWDATECOMM2)
			{
				m_DC.VD_TextOut(CRect(TEXTBOX_X+m_iDtcNumWidth*6,m_Rect.Height()/2,TEXTBOX_X+m_iDtcNumWidth*7,m_Rect.Height()),":");
			}
		}
	}
	else if (m_dtStyle & DTS_DATE)      
	{
	}
	else if (m_dtStyle & DTS_TIME)  
	{//此时不用区分single
		//if(single)
		{
			if (DRAWTIMECOMM1)
			{
				m_DC.VD_TextOut(CRect(TEXTBOX_X+m_iDtcNumWidth*2,0,TEXTBOX_X+m_iDtcNumWidth*15,m_Rect.Height()),":");
			}
			if (DRAWTIMECOMM2)
			{
				m_DC.VD_TextOut(CRect(TEXTBOX_X+m_iDtcNumWidth*5,0,TEXTBOX_X+m_iDtcNumWidth*18,m_Rect.Height()),":");
			}
		}
		
	}
	else
	{
	}
	m_DC.UnLock();
}

void CDateTimeCtrl::DrawFrame()
{
	if(!DrawCheck()){
		return;
	}
	m_DC.Lock();

	VD_COLORREF frame_color = VD_GetSysColor(GetFlag(IF_SELECTED)?COLOR_FRAMESELECTED:COLOR_FRAME);

	m_DC.SetRgnStyle(RS_HOLLOW);
	m_DC.SetPen(frame_color,VD_PS_SOLID,width);
	if (!(m_dwStyle & styleNoBorder))
	{
		if(CComboBox::m_bmpNormal != NULL) //暂时用CComboBox::m_bmpNormal决定控件风格，以后会改掉
		{
			m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()),0,0);
		}
		else
		{
			m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()),3,3);
		}
	}
	m_DC.SetPen(frame_color);
	if((m_dtStyle & DTS_DATE) && (m_dtStyle & DTS_TIME))
	{
		if(single)
		{
			
			m_DC.MoveTo(m_iDtcLinePos, 2);
			m_DC.LineTo(m_iDtcLinePos, m_Rect.Height() - 3);
		}
		else
		{
			
			m_DC.MoveTo(0, m_Rect.Height()/2);
			m_DC.LineTo(m_Rect.right, m_Rect.Height()/2);
		}
	}

	m_DC.UnLock();
}

void CDateTimeCtrl::Select(VD_BOOL flag)
{
	SetFlag(IF_SELECTED, flag);
	DrawFrame();
}

void CDateTimeCtrl::SetDateTime(SYSTEM_TIME *p)
{
	if ((m_dtStyle & DTS_DATE) && (m_dtStyle & DTS_TIME))
	{
		if(!p)
			return;
		GetDateTime(&time);
		if(memcmp(&time,p,sizeof(SYSTEM_TIME))==0)
			return;
		time = *p;

		pnb[0]->SetValue(p->year);
		pnb[1]->SetValue(p->month);
		pnb[2]->SetValue(p->day);
		if(timefmt == TF_12)
		{
			pnb[3]->SetValue(Hour24To12(p->hour));
			pcb->SetCurSel(Hour24ToAMPM(p->hour));
		}
		else
		{
			pnb[3]->SetValue(p->hour);
		}
		pnb[4]->SetValue(p->minute);
		pnb[5]->SetValue(p->second);

		int maxday = GetDaysOfMonth(p->year, p->month);
		pnb[2]->SetRange(1, maxday);
	}
	else if (m_dtStyle & DTS_DATE) 
	{
		if(!p)
			return;
		

		GetDateTime(&time);
		if(memcmp(&time,p,sizeof(SYSTEM_TIME))==0)
			return;
		time = *p;

		pnb[0]->SetValue(p->year);
		pnb[1]->SetValue(p->month);
		pnb[2]->SetValue(p->day);

		int maxday = GetDaysOfMonth(p->year, p->month);
		pnb[2]->SetRange(1, maxday);
	}
	else if (m_dtStyle & DTS_TIME)
	{
		if(!p)
			return;
		GetDateTime(&time);
		if(memcmp(&time,p,sizeof(SYSTEM_TIME))==0)
			return;
		time = *p;

		if(timefmt == TF_12)
		{
			pnb[3]->SetValue(Hour24To12(p->hour));
			pcb->SetCurSel(Hour24ToAMPM(p->hour));              
		}
		else
		{
			pnb[3]->SetValue(p->hour);
		}
		pnb[4]->SetValue(p->minute);
		pnb[5]->SetValue(p->second);

	}	
}

void CDateTimeCtrl::GetDateTime(SYSTEM_TIME *p)
{
	if ((m_dtStyle & DTS_DATE) && (m_dtStyle & DTS_TIME))
	{
		if(!p)
			return;
		p->year = pnb[0]->GetValue();
		p->month = pnb[1]->GetValue();
		p->day = pnb[2]->GetValue();
		p->hour = pnb[3]->GetValue();
		if(timefmt == TF_12)
		{
			p->hour = Hour12To24(p->hour,  pcb->GetCurSel());
		}
		p->minute = pnb[4]->GetValue();
		p->second = pnb[5]->GetValue();
	}
	else if (m_dtStyle & DTS_DATE)  
	{
		if(!p)
			return;
		p->year = pnb[0]->GetValue();
		p->month = pnb[1]->GetValue();
		p->day = pnb[2]->GetValue();
	}
	else if (m_dtStyle & DTS_TIME)  
   	{
	   if(!p)
			return;
	   	p->hour = pnb[3]->GetValue();
		if(timefmt == TF_12)
		{
			p->hour = Hour12To24(p->hour,  pcb->GetCurSel());
		}
		p->minute = pnb[4]->GetValue();           
		p->second = pnb[5]->GetValue();
	}
	else
	{
		assert(0);
	}
}

void CDateTimeCtrl::SetTimeFormat(int vdatefmt, int vsprtrfmt, int vtimefmt)
{
	if ((m_dtStyle & DTS_DATE) && (m_dtStyle & DTS_TIME))
	{
		if(datefmt != vdatefmt)
		{
			//move coordinate for new date format
			CRect rect[3];
			int i;
			rect[0].SetRect(0, 0, m_iDtcNumWidth * 4, m_Rect.bottom - m_Rect.top -4);
			rect[1].SetRect(0, 0, m_iDtcNumWidth * 2, m_Rect.bottom - m_Rect.top -4);
			rect[2].SetRect(0, 0, m_iDtcNumWidth * 2, m_Rect.bottom - m_Rect.top -4);
			for(i = 0; i < 3; i++)
			{
				rect[i].OffsetRect(m_Rect.left + TEXTBOX_X, m_Rect.top + 2);
			}
			switch(vdatefmt) 
			{
			case DF_YYMMDD:
				rect[0].OffsetRect(0, 0);
				rect[1].OffsetRect(m_iDtcNumWidth * 5, 0);
				rect[2].OffsetRect(m_iDtcNumWidth * 8, 0);
				break;
			case DF_MMDDYY:
				rect[0].OffsetRect(m_iDtcNumWidth * 6, 0);
				rect[1].OffsetRect(0, 0);
				rect[2].OffsetRect(m_iDtcNumWidth * 3, 0);
				break;
			case DF_DDMMYY:
				rect[0].OffsetRect(m_iDtcNumWidth * 6, 0);
				rect[1].OffsetRect(m_iDtcNumWidth * 3, 0);
				rect[2].OffsetRect(0, 0);
				break;
			default:
				break;
			}
			for(i = 0; i < 3; i++)
			{
				pnb[i]->SetRect(rect[i], TRUE);
			}
		}
	
		if(timefmt != vtimefmt)
		{
			if(vtimefmt == TF_12)
			{
				pcb->Enable(TRUE);
				pcb->SetCurSel(Hour24ToAMPM(time.hour));
				pnb[3]->SetValue(Hour24To12(time.hour));
				pnb[3]->SetRange(1, 12);
			}
			else
			{
				pcb->Enable(FALSE);
				pcb->SetCurSel(-1);
				pnb[3]->SetRange(0, 23);
				pnb[3]->SetValue(time.hour);
			}
		}

		datefmt = vdatefmt;
		datesprtr = vsprtrfmt;
		timefmt = vtimefmt;
		Draw();
		for(int i=0;i<6;i++)
		{
			pnb[i]->Draw();
		}
		pcb->Draw();
	}
	else if (m_dtStyle & DTS_DATE)  
	{
		if(datefmt != vdatefmt)
		{
			//move coordinate for new date format
			CRect rect[3];
			int i;
			rect[0].SetRect(0, 0, m_iDtcNumWidth * 4, m_Rect.bottom - m_Rect.top -4);
			rect[1].SetRect(0, 0, m_iDtcNumWidth * 2, m_Rect.bottom - m_Rect.top -4);
			rect[2].SetRect(0, 0, m_iDtcNumWidth * 2, m_Rect.bottom - m_Rect.top -4);
			for(i = 0; i < 3; i++)
			{
				rect[i].OffsetRect(m_Rect.left + TEXTBOX_X, m_Rect.top + 2);
			}
			switch(vdatefmt) 
			{
				case DF_YYMMDD:
					rect[0].OffsetRect(0, 0);
					rect[1].OffsetRect(m_iDtcNumWidth * 5, 0);
					rect[2].OffsetRect(m_iDtcNumWidth * 8, 0);
					break;
				case DF_MMDDYY:
					rect[0].OffsetRect(m_iDtcNumWidth * 6, 0);
					rect[1].OffsetRect(0, 0);
					rect[2].OffsetRect(m_iDtcNumWidth * 3, 0);
					break;
				case DF_DDMMYY:
					rect[0].OffsetRect(m_iDtcNumWidth * 6, 0);
					rect[1].OffsetRect(m_iDtcNumWidth * 3, 0);
					rect[2].OffsetRect(0, 0);
					break;
				default:
					break;
			}
			for(i = 0; i < 3; i++)
			{
				pnb[i]->SetRect(rect[i], TRUE);
			}
		}
	
		
		datefmt = vdatefmt;
		datesprtr = vsprtrfmt;
		
		Draw();
		for(int i=0;i<3;i++) 
		{
			pnb[i]->Draw();
		}
		
	}
	else if (m_dtStyle & DTS_TIME)
	{
		if(timefmt != vtimefmt)
		{
			if(vtimefmt == TF_12)
			{
				pcb->Enable(TRUE);
				pcb->SetCurSel(Hour24ToAMPM(time.hour));
				pnb[3]->SetValue(Hour24To12(time.hour));
				pnb[3]->SetRange(1, 12);
			}
			else
			{
				pcb->Enable(FALSE);
				pcb->SetCurSel(-1);
				pnb[3]->SetRange(0, 23);
				pnb[3]->SetValue(time.hour);
                printf("time.hour = %d\n", time.hour);
			}
		}
		datefmt = vdatefmt;
		datesprtr = vsprtrfmt;
		timefmt = vtimefmt;
		Draw();
		for(int i=3;i<6;i++) 
		{
			pnb[i]->Draw();
		}
		pcb->Draw();

	}
	else
	{
		assert(0);
	}
}

void CDateTimeCtrl::Enable(VD_BOOL flag)
{
	CItem::Enable(flag);
	if ((m_dtStyle & DTS_DATE) && (m_dtStyle & DTS_TIME))
	{
		for(int i=0;i<6;i++)        
			pnb[i]->Enable(flag);
		if(timefmt == TF_12)
		{
			pcb->Enable(flag);
		}
	}
	else if (m_dtStyle & DTS_DATE) 
	{
		for(int i=0;i<3;i++)        
		pnb[i]->Enable(flag);
	}
	else if (m_dtStyle & DTS_TIME)
	{
		for(int i=3;i<6;i++)        
		pnb[i]->Enable(flag);
		if(timefmt == TF_12)
		{
			pcb->Enable(flag);
    	}
	}
	else
	{
	}
}

void CDateTimeCtrl::Show(VD_BOOL flag)
{
	CItem::Show(flag);
	if (m_dtStyle & DTS_DATE)
	{
		if (m_dtStyle & DTS_YEAR)
		{
			pnb[0]->Show(flag);
		}
		if (m_dtStyle & DTS_MONTH)
		{
			pnb[1]->Show(flag);
		}
		if (m_dtStyle & DTS_DAY)
		{
			pnb[2]->Show(flag);
		}
	}
	if (m_dtStyle & DTS_TIME)
	{
		if (m_dtStyle & DTS_HOUR)
		{
			pnb[3]->Show(flag);
			pcb->Show(flag);
		}
		if (m_dtStyle & DTS_MINUTE)
		{
			pnb[4]->Show(flag);
		}
		if (m_dtStyle & DTS_SECOND)
		{
			pnb[5]->Show(flag);
		}
	}
	DrawSplit();
}

void CDateTimeCtrl::SetBkColor(VD_COLORREF color, VD_BOOL redraw /* = FALSE */)
{
	m_BkColor = color;

	for(int i=0;i<6;i++)
	{
		if(pnb[i])
		{
			pnb[i]->SetBkColor(m_BkColor);
		}
	}

	if(pcb)
	{
		pcb->SetBkColor(m_BkColor);
	}

	if(redraw)
	{
		Draw();
	}
}


void CDateTimeCtrl::Show( VD_BOOL flag, VD_BOOL redraw /*= TRUE*/ )
{
	CItem::Show(flag, redraw);
	if (m_dtStyle & DTS_DATE)
	{
		if (m_dtStyle & DTS_YEAR)
		{
			pnb[0]->Show(flag);
		}
		if (m_dtStyle & DTS_MONTH)
		{
			pnb[1]->Show(flag);
		}
		if (m_dtStyle & DTS_DAY)
		{
			pnb[2]->Show(flag);
		}
	}
	if (m_dtStyle & DTS_TIME)
	{
		if (m_dtStyle & DTS_HOUR)
		{
			pnb[3]->Show(flag);
			pcb->Show(flag);
		}
		if (m_dtStyle & DTS_MINUTE)
		{
			pnb[4]->Show(flag);
		}
		if (m_dtStyle & DTS_SECOND)
		{
			pnb[5]->Show(flag);
		}
	}

	DrawSplit();
}
int CDateTimeCtrl::Hour12To24(int hour12, int am)
{
	if(am == 0)
	{
		if(hour12 == 12)
		{
			return 0;
		}
        else if (hour12 > 12)
        {
            return hour12 - 12;
        }
	}
	else
	{
		if(hour12 != 12)
		{
			return hour12 + 12;
		}
	}
	return hour12;
}

int CDateTimeCtrl::Hour24To12(int hour24)
{
	if(hour24 > 12)
	{
		return hour24 - 12;
	}
	else if(hour24 == 0)
	{
		return 12;
	}
	else
	{
		return hour24;
	}
}

int CDateTimeCtrl::Hour24ToAMPM(int hour24)
{
	if(hour24 >= 12)
	{
		return 1; //下午
	}
	return 0; //上午
}
void CDateTimeCtrl::SetDefaultBitmap(DateTimeCtrlBitmap region, VD_PCSTR name)
{
	switch(region)
	{
	case DTB_NORMAL:
		m_bmpNormal = VD_LoadBitmap(name);
		break;
	case DTB_SELECTED:
		m_bmpSelected = VD_LoadBitmap(name);
		break;
	case DTB_PUSHED:
		m_bmpPushed = VD_LoadBitmap(name);
		break;
	case DTB_DISABLED:
		m_bmpDisabled = VD_LoadBitmap(name);
		break;
	default:
		break;
	}
}
