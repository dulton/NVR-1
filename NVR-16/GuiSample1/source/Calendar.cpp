//
//  "$Id: Calendar.cpp 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#include "GUI/Ctrls/Page.h"
#include <time.h>

///////////////////////////////////////////////////////////////////
////////////        CCalendar
std::string CCalendar::m_strMonth = 
//"January|February|March|April|May|June|July|August|September|October|November|December";
"&CfgPtn.January|&CfgPtn.February|&CfgPtn.March|&CfgPtn.April|&CfgPtn.May|&CfgPtn.June|&CfgPtn.July|&CfgPtn.August|&CfgPtn.September|&CfgPtn.October|&CfgPtn.November|&CfgPtn.December";

std::string CCalendar::m_strWeeks[7] = 
{
		"&CfgPtn.Sunday",
		"&CfgPtn.Monday",
		"&CfgPtn.Tuesday",
		"&CfgPtn.Wednesday",
		"&CfgPtn.Thursday",
		"&CfgPtn.Friday",
		"&CfgPtn.Saturday"
	};
/*
{
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat"
};*/

int CCalendar::m_weekstart = 0;

CCalendar::CCalendar(VD_PCRECT pRect, CPage * pParent, CTRLPROC onMonthChanged /* = NULL */, CTRLPROC onDaySelected /* = NULL */,uint style /**/)
	:CItem(CRect(pRect->left, pRect->top + ROW_HEIGHT, pRect->right, pRect->bottom), pParent, IT_CALENDAR,style|styleEditable|styleAutoFocus)
{
	CRect m_rect;
#if 0
	m_rect.SetRect(pRect->left, pRect->top + 2, pRect->left + TEXT_WIDTH * 2, pRect->top + CTRL_HEIGHT);
	m_pPrev = CreateButton(m_rect, pParent, "<", (CTRLPROC)&CCalendar::OnPrevMonth);

	m_rect.SetRect(pRect->right - TEXT_WIDTH * 2, pRect->top + 2, pRect->right, pRect->top + CTRL_HEIGHT);
	m_pNext = CreateButton(m_rect, pParent, ">", (CTRLPROC)&CCalendar::OnNextMonth);

	m_rect.SetRect(pRect->left + TEXT_WIDTH * 5 / 2, pRect->top + 2, pRect->left + TEXT_WIDTH * 15 / 2, pRect->top + CTRL_HEIGHT);
	m_pMonth = CreateComboBox(m_rect, pParent, m_strMonth.c_str(), NULL, (CTRLPROC)&CCalendar::OnMonthChanged);

	m_rect.SetRect(pRect->left + TEXT_WIDTH * 8, pRect->top + 2, pRect->left + TEXT_WIDTH * 23 / 2, pRect->top + CTRL_HEIGHT);
	m_pYear = CreateNumberBox(m_rect, pParent, 2000, 2037, "%4d", NULL, (CTRLPROC)&CCalendar::OnMonthChanged);
#else

	//printf("calendar %d \n", pRect->right- pRect->left);
	m_rect.SetRect(pRect->left, pRect->top + 2, pRect->left + CTRL_HEIGHT1, pRect->top + CTRL_HEIGHT);
	m_pPrev = CreateButton(m_rect, pParent, "<", (CTRLPROC)&CCalendar::OnPrevMonth);

	m_rect.SetRect(pRect->right - CTRL_HEIGHT1 , pRect->top + 2, pRect->right, pRect->top + CTRL_HEIGHT);
	m_pNext = CreateButton(m_rect, pParent, ">", (CTRLPROC)&CCalendar::OnNextMonth);

	m_rect.SetRect(pRect->left + CTRL_HEIGHT1 + 2, pRect->top + 2, pRect->left + TEXT_WIDTH*3 + 11, pRect->top + CTRL_HEIGHT);
	m_pMonth = CreateComboBox(m_rect, pParent, m_strMonth.c_str(), NULL, (CTRLPROC)&CCalendar::OnMonthChanged);

	m_rect.SetRect(pRect->left + TEXT_WIDTH * 3+13, pRect->top + 2, pRect->right - CTRL_HEIGHT1-2, pRect->top + CTRL_HEIGHT);
	m_pYear = CreateNumberBox(m_rect, pParent, 2000, 2037, "%4d", NULL, (CTRLPROC)&CCalendar::OnMonthChanged);

#endif

	m_pPrev->SetOwner(this);
	m_pNext->SetOwner(this);
	m_pMonth->SetOwner(this);
	m_pYear->SetOwner(this);
	m_pMonth->SetCurSel(0);
	m_pYear->SetValue(2010);
	
	m_dwMask = 0;
	//先算出初始的值
	UpdateDays();
	m_iCursel = -1;

	m_onMonthChanged = onMonthChanged;
	m_onDaySelected = onDaySelected;//(CTRLPROC)&CCalendar::OnDaySelected;

}

CCalendar::~CCalendar()
{
}

VD_SIZE& CCalendar::GetDefaultSize(void)
{
	static VD_SIZE size = {TEXT_WIDTH*14,TEXT_HEIGHT*7 + ROW_HEIGHT};
	return size;
}

CCalendar* CreateCalendar(VD_PCRECT pRect, CPage * pParent, CTRLPROC onMonthChanged /* = NULL */, CTRLPROC onDaySelected /* = NULL */,uint style /* = 0 */)
{
	return new CCalendar(pRect,pParent,onMonthChanged,onDaySelected,style);
}
void CCalendar::Draw()
{
	if (!DrawCheck())
	{
		return;
	}

	int i;

	//画背景
	m_DC.SetBrush(/*VD_RGB(0,102,255)*/VD_GetSysColor(VD_COLOR_WINDOW));
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));

	//画星期
	m_DC.SetFont(/*VD_RGB(8,8,8)*/VD_GetSysColor(VD_COLOR_WINDOWTEXT));
	m_DC.SetTextAlign(VD_TA_CENTER);
	for (i = 0; i < 7; i++)
	{
#if 0
		m_DC.VD_TextOut(CRect(TEXT_WIDTH * 2 * i, 0, TEXT_WIDTH * 2 * (i + 1) - 1, TEXT_HEIGHT), m_strWeeks[i].c_str());
#else
		m_DC.VD_TextOut(CRect((m_Rect.Width()/7) * i, 0, (m_Rect.Width()/7) * (i + 1) - 1, TEXT_HEIGHT), m_strWeeks[i].c_str());
#endif
	}

	//画分割线
	m_DC.SetPen(VD_GetSysColor(COLOR_POPUP), VD_PS_SOLID, 2);
	m_DC.MoveTo(2, TEXT_WIDTH - 2);
	m_DC.LineTo(m_Rect.Width() - 2, TEXT_WIDTH - 2);

	DrawFrame();

	if (m_nStartWeekDay < 0)
	{
		return;
	}

	for (i = 0; i < m_nMaxDays; i++)
	{
		DrawSingle(i);
	}
}

void CCalendar::DrawSingle(int index)
{
	if (!DrawCheck() || index < 0 || index > m_nMaxDays || m_nStartWeekDay < 0)
	{
		return;
	}

	char buf[8];
	VD_BOOL flag = (m_dwMask & BITMSK(index));

	sprintf(buf, "%2d", index + 1);
	//m_DC.SetFont(VD_GetSysColor((index == m_iCursel) ? COLOR_CTRLTEXTSELECTED : COLOR_CTRLTEXT));
	m_DC.SetFont(VD_GetSysColor((index == m_iCursel) ? COLOR_CTRLTEXTSELECTED : COLOR_CTRLTEXT));
	m_DC.SetTextAlign(VD_TA_CENTER);

	index += m_nStartWeekDay;//转换成网格的序号
	if (flag)
	{
		//m_DC.SetBrush(VD_GetSysColor(COLOR_FRAME));
		m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
		//m_DC.SetRgnStyle(RS_FLAT);
	}
	else
	{
		m_DC.SetPen(VD_GetSysColor(COLOR_FRAME));
		//m_DC.SetPen(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
		//m_DC.SetRgnStyle(RS_HOLLOW);
	}
#if 0
	m_DC.Rectangle(CRect((index % 7) * TEXT_WIDTH * 2 + 2, (index / 7 + 1) * TEXT_HEIGHT + 2,
		(index % 7 + 1) * TEXT_WIDTH * 2 - 2, (index / 7 + 2) * TEXT_HEIGHT - 2), 1, 1);
	m_DC.VD_TextOut(CRect((index % 7) * TEXT_WIDTH * 2, (index / 7 + 1) * TEXT_HEIGHT,
		(index % 7 + 1) * TEXT_WIDTH * 2, (index / 7 + 2) * TEXT_HEIGHT), buf);
#else
	m_DC.Rectangle(CRect((index % 7) * (m_Rect.Width()/7) + 2, (index / 7 + 1) * TEXT_HEIGHT + 2,
		(index % 7 + 1) * (m_Rect.Width()/7) - 2, (index / 7 + 2) * TEXT_HEIGHT - 2), 1, 1);
	m_DC.VD_TextOut(CRect((index % 7) * (m_Rect.Width()/7), (index / 7 + 1) * TEXT_HEIGHT,
		(index % 7 + 1) * (m_Rect.Width()/7), (index / 7 + 2) * TEXT_HEIGHT), buf);
#endif
}

void CCalendar::DrawFrame()
{
	////printf("DrawFrame 0\n");
	if (!DrawCheck())
	{
		return;
	}
	////printf("DrawFrame 1\n");

	m_DC.SetRgnStyle(RS_HOLLOW);
	if(GetFlag(IF_SELECTED)){	
		m_DC.SetPen(VD_GetSysColor(COLOR_FRAMESELECTED),VD_PS_SOLID,2);
	}else{
		m_DC.SetPen(VD_GetSysColor(COLOR_FRAME),VD_PS_SOLID,2);
	}
	////printf("DrawFrame 2\n");
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()),3,3);
	////printf("DrawFrame 3\n");
}

void CCalendar::Select(VD_BOOL flag)
{
	SetFlag(IF_SELECTED, flag);
	DrawFrame();
}

int CCalendar::GetAt(int px, int py)
{
	int index;

	if (m_nStartWeekDay < 0)
	{
		return -1;
	}

	CRect rect(0, TEXT_HEIGHT, TEXT_WIDTH * 14, TEXT_HEIGHT * 7);
	
	ClientToScreen(&rect);
	if(!rect.PtInRect(px, py))
	{
		return -1;
	}
	
#if 0
	index = (px - rect.left) / (TEXT_WIDTH * 2) + (py - rect.top) / TEXT_HEIGHT * 7;
#else
	index = (px - rect.left) / (m_Rect.Width()/7) + (py - rect.top) / TEXT_HEIGHT * 7;
#endif

	if(index < m_nStartWeekDay || index >= m_nStartWeekDay + m_nMaxDays)
	{
		return -1;
	}

	return index - m_nStartWeekDay;
}

VD_BOOL CCalendar::MsgProc(uint msg, uint wpa, uint lpa)
{
	int px, py;
	int temp, newsel;

	newsel = m_iCursel;

	switch(msg)
	{
	case XM_LBUTTONDOWN:
	case XM_LBUTTONDBLCLK:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		temp = GetAt(px, py);
		////printf("XM_LBUTTONDOWN %d\n",temp);
		if(temp >= 0 )
		{
			m_iCursel = newsel = temp;
			////printf("m_onDaySelected = %p \n", m_onDaySelected);
			ItemProc(m_onDaySelected);
			//Select(TRUE);
		}
		else
		{
			return FALSE;
		}
		break;
	case XM_MOUSEMOVE:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		newsel = GetAt(px, py);
		break;
	
	default:
		return FALSE;
	}
	//if m_Cursel changed
	if(newsel != m_iCursel && msg!=XM_MOUSEMOVE )
	{
		temp = m_iCursel;
		//m_iCursel = newsel;
		DrawSingle(newsel);
		DrawSingle(temp);
	}
	//printf("msgproc m_iCursel %d\n", m_iCursel);
	return TRUE;
}

void CCalendar::UpdateDays()
{
	int year = m_pYear->GetValue();
	int month = m_pMonth->GetCurSel();
	struct tm t;
	time_t t_of_day;

	//得到星期几
	t.tm_year=year-1900;
	t.tm_mon=month;
	t.tm_mday=1;
	t.tm_hour=0;
	t.tm_min=0;
	t.tm_sec=1;
	t.tm_isdst=0;
	t_of_day=mktime(&t);
	
	if(t_of_day > 0)
	{
		m_nStartWeekDay = (t.tm_wday + 7 - m_weekstart) % 7;
	}
	else
	{
		m_nStartWeekDay = -1;
	}

	//printf("UpdateDays %d\n",m_nStartWeekDay);
	
	m_nMaxDays = GetDaysOfMonth(year, month + 1);
}

void CCalendar::OnPrevMonth()
{
	int year = m_pYear->GetValue();
	int month = m_pMonth->GetCurSel();

	if(month > 0)
	{
		m_pMonth->SetCurSel(month - 1);
	}
	else if(year > 2000)
	{
		m_pYear->SetValue(year - 1);
		m_pMonth->SetCurSel(11); //11 - 十二月
	}
	else
	{
		return;
	}

	OnMonthChanged();
}

void CCalendar::OnNextMonth()
{
	int year = m_pYear->GetValue();
	int month = m_pMonth->GetCurSel();

	if(month < 11)
	{
		m_pMonth->SetCurSel(month + 1);
	}
	else if(year < 2037)
	{
		m_pYear->SetValue(year + 1);
		m_pMonth->SetCurSel(0); //0 - 一月
	}
	else
	{
		return;
	}

	OnMonthChanged();
}

void CCalendar::OnMonthChanged()
{
	UpdateDays();
	Draw();
	ItemProc(m_onMonthChanged);
}

void CCalendar::OnDaySelected()
{
	////printf("CCalendar::OnDaySelected\n");
	SetMask(0x1);
	
 	return;
}

void CCalendar::SetDate(SYSTEM_TIME *p)
{
	if(!p)
	{
		return;
	}

	m_pMonth->SetCurSel(p->month-1);
	m_pYear->SetValue(p->year);
	m_iCursel = p->day - 1;

	//printf("m_iCursel %d\n", m_iCursel);
	
	UpdateDays();

	Draw();
}

VD_BOOL CCalendar::GetDate(SYSTEM_TIME *p)
{
	if(!p || m_nStartWeekDay < 0)
	{
		return FALSE;
	}

	p->month = m_pMonth->GetCurSel() + 1;
	p->year = m_pYear->GetValue();

	if(m_iCursel < 0)//没有选中时返回当月的第一天
	{
		p->day = 1;
	}
	else
	{
		p->day = m_iCursel + 1;
	}
	//printf("m_iCursel %d\n",m_iCursel);

	//printf("p->day %d\n",p->day);
	
	p->hour = 0;
	p->minute = 0;
	p->second = 0;

	return TRUE;
}

VD_BOOL CCalendar::SetStrings(VD_PCSTR month, VD_PCSTR week)
{
	int i, j;
	std::string temp;
	int offs = 0;
	int offs_old = 0;

	if(month && week)
	{
		m_strMonth = month;
		temp = week;
		for(j = 0; j < 7; j++)
		{
			i = (j + 7 - m_weekstart) % 7;
			offs = temp.find('|', offs_old);
			if(offs < 0)
			{
				m_strWeeks[i].assign(temp.c_str() + offs_old, temp.size() - offs_old);
				while(m_strWeeks[i].rfind(' ') == m_strWeeks[i].size() - 1)
				{
					m_strWeeks[i].erase(m_strWeeks[i].end() - 1);
				}
				break;
			}
			m_strWeeks[i].assign(temp.c_str() + offs_old, offs - offs_old);
			while(m_strWeeks[i].rfind(' ') == m_strWeeks[i].size() - 1)
			{
				m_strWeeks[i].erase(m_strWeeks[i].end() - 1);
			}
			offs_old = offs + 1;
		}
		return TRUE;
	}
	return FALSE;
}

void CCalendar::SetWeekStart(int weekstart)
{
	m_weekstart = weekstart;
}

int CCalendar::GetDaysOfMonth(int year,int monthth)
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

void CCalendar::SetMask(uint mask)
{
	if(mask == m_dwMask)
	{
		return;
	}
	////printf("SetMask \n");
	
	m_dwMask = mask;
	Draw();
}

void CCalendar::Show( VD_BOOL flag, VD_BOOL redraw /*= TRUE*/ )
{
	m_pPrev->Show(flag, redraw);
	m_pNext->Show(flag, redraw);
	m_pMonth->Show(flag, redraw);
	m_pYear->Show(flag, redraw);
	
	CItem::Show(flag, redraw);
}
