//
//  "$Id: TimingCtrl.cpp 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#include "GUI/Ctrls/Page.h"
//#include "Configs/ConfigLocation.h"

VD_BITMAP* CTimingCtrl::bmp_tcb_normal;
VD_BITMAP* CTimingCtrl::bmp_tcb_selected;
VD_BITMAP* CTimingCtrl::bmp_tcb_disabled;

///////////////////////////////////////////////////////////////////
////////////        CTimingCtrl

CTimingCtrl* CreateTimingCtrl(VD_PCRECT pRect, CPage * pParent, CTRLPROC onChanged /* = NULL */,uint style /* = 0 */)
{
	return new CTimingCtrl(pRect,pParent,onChanged,style);
}

CTimingCtrl::CTimingCtrl(VD_PCRECT pRect, CPage * pParent, CTRLPROC onChanged /* = NULL */,uint style, enum time_fmt tmft/* = TF_24 */)
:CItem(pRect, pParent,IT_TIMINGCTRL,style|styleAutoFocus)//, FALSE)
,timefmt(tmft)
{
	int i;
	CRect m_rect;

	if(pRect!=NULL)
	{
		if (timefmt == TF_24)
		{
			m_rect.SetRect(pRect->left+TEXT_WIDTH*0+TEXTBOX_X,pRect->top+2,pRect->left+TEXT_WIDTH*1+TEXTBOX_X,pRect->bottom-2);
			pnb[0] = CreateNumberBox(m_rect, m_pParent,0,24,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);

			m_rect.SetRect(pRect->left+TEXT_WIDTH*1+TEXTBOX_X,pRect->top+2,pRect->left+TEXT_WIDTH*2+TEXTBOX_X,pRect->bottom-2);
			pnb[1] = CreateNumberBox(m_rect, m_pParent,0,59,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);

			m_rect.SetRect(pRect->left+TEXT_WIDTH*2+TEXTBOX_X,pRect->top+2,pRect->left+TEXT_WIDTH*3+TEXTBOX_X,pRect->bottom-2);
			psp = CreateStatic(m_rect, m_pParent, "  -");

			m_rect.SetRect(pRect->left+TEXT_WIDTH*3+TEXTBOX_X,pRect->top+2,pRect->left+TEXT_WIDTH*4+TEXTBOX_X,pRect->bottom-2);
			pnb[2] = CreateNumberBox(m_rect, m_pParent,0,24,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);

			m_rect.SetRect(pRect->left+TEXT_WIDTH*4+TEXTBOX_X,pRect->top+2,pRect->left+TEXT_WIDTH*5+TEXTBOX_X,pRect->bottom-2);
			pnb[3] = CreateNumberBox(m_rect, m_pParent,0,59,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);		
		}
		else
		{
			m_rect.SetRect(pRect->left+TEXT_WIDTH*0+TEXTBOX_X,pRect->top+2,pRect->left+TEXT_WIDTH*1+TEXTBOX_X+4,pRect->bottom-2);
			pnb[0] = CreateNumberBox(m_rect, m_pParent,0,24,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);
			m_rect.SetRect(pRect->left+TEXT_WIDTH*3/2+2+TEXTBOX_X,pRect->top+2,pRect->left+TEXT_WIDTH*5/2+TEXTBOX_X+4,pRect->bottom-2);
			pnb[1] = CreateNumberBox(m_rect, m_pParent,0,59,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);
			m_rect.SetRect(pRect->left+TEXT_WIDTH*5/2+TEXTBOX_X+3,pRect->top+2,pRect->left+TEXT_WIDTH*4+TEXTBOX_X+4,pRect->bottom-2);
			pcb[0] = CreateComboBox(m_rect, m_pParent,"AM|PM",NULL,NULL, styleNoBorder | comboNoButton);
			m_rect.SetRect(pRect->left+TEXT_WIDTH*9/2+TEXTBOX_X+2,pRect->top+2,pRect->left+TEXT_WIDTH*11/2+TEXTBOX_X+4,pRect->bottom-2);
			pnb[2] = CreateNumberBox(m_rect, m_pParent,0,24,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);
			m_rect.SetRect(pRect->left+TEXT_WIDTH*6+TEXTBOX_X+2,pRect->top+2,pRect->left+TEXT_WIDTH*7+TEXTBOX_X+4,pRect->bottom-2);
			pnb[3] = CreateNumberBox(m_rect, m_pParent,0,59,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);
			m_rect.SetRect(pRect->left+TEXT_WIDTH*7+TEXTBOX_X+3,pRect->top+2,pRect->left+TEXT_WIDTH*17/2+TEXTBOX_X,pRect->bottom-2);
			pcb[1] = CreateComboBox(m_rect, m_pParent,"AM|PM",NULL,NULL, styleNoBorder | comboNoButton);
		}
	}
	else
	{
		m_rect.SetRect(0,0,TEXT_WIDTH,CTRL_HEIGHT-4);
		if (timefmt == TF_24)
		{
			SetRect(CRect(0, 0, TEXT_WIDTH * 8, CTRL_HEIGHT), FALSE);
			pnb[0] = CreateNumberBox(m_rect,m_pParent,0,24,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);
			pnb[1] = CreateNumberBox(m_rect,m_pParent,0,59,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);
			psp = CreateStatic(m_rect, m_pParent, "  -");
			pnb[2] = CreateNumberBox(m_rect,m_pParent,0,24,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);
			pnb[3] = CreateNumberBox(m_rect,m_pParent,0,59,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);
		}
		else
		{
			pnb[0] = CreateNumberBox(m_rect,m_pParent,0,24,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);
			pnb[1] = CreateNumberBox(m_rect,m_pParent,0,59,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);
			pcb[0] = CreateComboBox(m_rect,m_pParent,"AM|PM",NULL,NULL, styleNoBorder | comboNoButton);

			pnb[2] = CreateNumberBox(m_rect,m_pParent,0,24,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);
			pnb[3] = CreateNumberBox(m_rect,m_pParent,0,59,"%02d",NULL,(CTRLPROC)&CTimingCtrl::OnTimeChanged,NULL,styleNoBorder);
			pcb[1] = CreateComboBox(m_rect,m_pParent,"AM|PM",NULL,NULL, styleNoBorder | comboNoButton);
		}
	}

	for(i = 0; i < 4; i++)
	{ 
		pnb[i]->SetOwner(this);
	}

	m_onChanged = onChanged;
	//默认为24小时制
	if (timefmt == TF_24)
	{
		psp->SetOwner(this);
	}
	else
	{
		pcb[0]->Enable(FALSE);
		pcb[0]->SetCurSel(-1);
		pcb[0]->SetOwner(this);
		pcb[1]->Enable(FALSE);
		pcb[1]->SetCurSel(-1);
		pcb[1]->SetOwner(this);
	}
	
	m_BkColor =	VD_GetSysColor(VD_COLOR_WINDOW);
}

CTimingCtrl::~CTimingCtrl()
{
}

VD_SIZE& CTimingCtrl::GetDefaultSize(void)
{
	static VD_SIZE size = {TEXT_WIDTH*17/2+TEXTBOX_X*2,CTRL_HEIGHT};
	return size;
}

VD_BOOL CTimingCtrl::Open()
{
	return CItem::Open();
}

/*!
	\b Description		:	使能CTimingCtrl控件\n
	\b Argument			:	BOOL flag
	\param	flag		:	TRUE:使能, FALSE:不使能
	\b Revisions		:	
		- 2006-12-30	     zhongyj		Modify
*/
void CTimingCtrl::Enable(VD_BOOL flag)
{
	CItem::Enable(flag);
	for(int i=0;i<4;i++)
	{
		pnb[i]->Enable(flag);
	}
	
	if (timefmt == TF_24)
	{
		psp->Enable(flag);
	}
	else
	{
		for(int i = 0; i < 2; i++)
		{
			pcb[i]->Enable(flag);
		}
	}
}

void CTimingCtrl::Draw()
{
	if(!DrawCheck()){
		return;
	}

	m_DC.Lock();
	m_DC.SetBrush(m_BkColor);
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
	if(GetFlag(IF_ENABLED))
	{
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXT));
	}
	else
	{
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTDISABLED));
	}
	m_DC.SetBkMode(BM_TRANSPARENT);
	if (timefmt == TF_24)
	{
		m_DC.VD_TextOut(CRect(TEXTBOX_X+TEXT_WIDTH+2,0,TEXTBOX_X+TEXT_WIDTH + 8,TEXT_HEIGHT),":");
		m_DC.VD_TextOut(CRect(TEXTBOX_X+TEXT_WIDTH*4 + TEXT_WIDTH * 3 / 2 + 2,0,TEXTBOX_X+TEXT_WIDTH*4 + TEXT_WIDTH * 3 / 2 + 8,TEXT_HEIGHT),":");
	}
	else
	{
		m_DC.VD_TextOut(CRect(TEXTBOX_X+TEXT_WIDTH+6,0,TEXTBOX_X+TEXT_WIDTH*3/2+2,TEXT_HEIGHT),":");
		m_DC.VD_TextOut(CRect(TEXTBOX_X+TEXT_WIDTH*4+6,0,TEXTBOX_X+TEXT_WIDTH*9/2+2,TEXT_HEIGHT),"-");
		m_DC.VD_TextOut(CRect(TEXTBOX_X+TEXT_WIDTH*11/2+6,0,TEXTBOX_X+TEXT_WIDTH*7+2,TEXT_HEIGHT),":");
	}
	m_DC.UnLock();

	DrawFrame();
}

void CTimingCtrl::DrawFrame()
{
	if(!DrawCheck()){
		return;
	}

	m_DC.Lock();

	int penwidth = 0;
	penwidth = 2;
	VD_COLORREF FrameColorSelect, FrameColorNormal;
	FrameColorSelect = VD_GetSysColor(COLOR_FRAMESELECTED);
	FrameColorNormal = VD_GetSysColor(COLOR_FRAME);
	if(GetFlag(IF_SELECTED))
	{
		m_DC.SetPen(FrameColorSelect, VD_PS_SOLID, penwidth);
	}
	else
	{
		m_DC.SetPen(FrameColorNormal, VD_PS_SOLID, penwidth);
	}
	m_DC.SetRgnStyle(RS_HOLLOW);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()), 3, 3);
	m_DC.UnLock();
}

void CTimingCtrl::Select(VD_BOOL flag)
{
	SetFlag(IF_SELECTED, flag);
	DrawFrame();
}

void CTimingCtrl::Show(VD_BOOL flag)
{
	int i;
	CItem::Show(flag);
	for(i = 0; i < 4; i++)
	{
		pnb[i]->Show(flag);
	}
	
	if (timefmt == TF_24)
	{
		psp->Show(flag);
	}
	else
	{
		for(i = 0; i < 2; i++)
		{
			pcb[i]->Show(flag);
		}
	}
}
void CTimingCtrl::SetTime(TSECT *p)
{

	if(!p)
		return ;
	GetTime(&tsect);
	if(memcmp(&tsect,p,6)==0){//6 byte for time
		return ;
	}

	tsect = *p;

	if(timefmt == TF_24)
	{
		pnb[0]->SetValue(p->StartHour);
	}
	else
	{
		pnb[0]->SetValue(Hour24To12(p->StartHour));
		pcb[0]->SetCurSel(Hour24ToAMPM(p->StartHour));
	}
	pnb[1]->SetValue(p->StartMin);
	if(timefmt == TF_24)
	{
		pnb[2]->SetValue(p->EndHour);
	}
	else
	{
		pnb[2]->SetValue(Hour24To12(p->EndHour));
		pcb[1]->SetCurSel(Hour24ToAMPM(p->EndHour));
	}
	pnb[3]->SetValue(p->EndMin);
}

void CTimingCtrl::SetTime(TIMESECTION *p)
{

	if(!p)
		return ;
	GetTime(&timesec);
	if(memcmp(&timesec, p, sizeof(TIMESECTION)) == 0){//6 byte for time
		return ;
	}

	timesec = *p;

	if(timefmt == TF_24)
	{
		pnb[0]->SetValue(p->startHour);
	}
	else
	{
		pnb[0]->SetValue(Hour24To12(p->startHour));
		pcb[0]->SetCurSel(Hour24ToAMPM(p->startHour));
	}
	pnb[1]->SetValue(p->startMinute);
	if(timefmt == TF_24)
	{
		pnb[2]->SetValue(p->endHour);
	}
	else
	{
		pnb[2]->SetValue(Hour24To12(p->endHour));
		pcb[1]->SetCurSel(Hour24ToAMPM(p->endHour));
	}
	pnb[3]->SetValue(p->endMinute);
}

VD_BOOL CTimingCtrl::GetTime(TSECT *p)
{
	if(!p)
	{
		return FALSE;
	}
	
	p->StartHour = pnb[0]->GetValue();
	if(timefmt == TF_12)
	{
		p->StartHour = Hour12To24(p->StartHour,  pcb[0]->GetCurSel());
	}
	p->StartMin = pnb[1]->GetValue();
	p->EndHour = pnb[2]->GetValue();
	if(timefmt == TF_12)
	{
		p->EndHour = Hour12To24(p->EndHour,  pcb[1]->GetCurSel());
	}
	p->EndMin = pnb[3]->GetValue();
	
	return (p->StartHour * 60 + p->StartMin < p->EndHour * 60 + p->EndMin);
}

VD_BOOL CTimingCtrl::GetTime(TIMESECTION *p)
{
	if(!p)
	{
		return FALSE;
	}

	p->startHour = pnb[0]->GetValue();
	if(timefmt == TF_12)
	{
		p->startHour = Hour12To24(p->startHour,  pcb[0]->GetCurSel());
	}
	p->startMinute = pnb[1]->GetValue();
	p->endHour= pnb[2]->GetValue();
	if(timefmt == TF_12)
	{
		p->endHour = Hour12To24(p->endHour,  pcb[1]->GetCurSel());
	}
	p->endMinute = pnb[3]->GetValue();

	return (p->startHour * 60 + p->startMinute < p->endHour * 60 + p->endMinute);
}

void CTimingCtrl::OnTimeChanged()
{
	//得到小时值
	tsect.StartHour = pnb[0]->GetValue();
	tsect.EndHour = pnb[2]->GetValue();
	timesec.startHour = pnb[0]->GetValue();
	timesec.endHour = pnb[2]->GetValue();
	if(timefmt == TF_12)
	{
		tsect.StartHour = Hour12To24(tsect.StartHour, pcb[0]->GetCurSel());
		tsect.EndHour = Hour12To24(tsect.EndHour, pcb[1]->GetCurSel());
		timesec.startHour = Hour12To24(timesec.startHour, pcb[0]->GetCurSel());
		timesec.endHour = Hour12To24(timesec.endHour, pcb[1]->GetCurSel());
	}

	//校验
	if(tsect.StartHour == 24){
		pnb[1]->SetValue(0);
	}
	if(tsect.EndHour == 24){
		pnb[3]->SetValue(0);
	}

	if(timesec.startHour == 24){
		pnb[1]->SetValue(0);
	}
	if(timesec.endHour == 24){
		pnb[3]->SetValue(0);
	}

	//得到分钟值,可能是校验后的值
	tsect.StartMin = pnb[1]->GetValue();
	tsect.EndMin = pnb[3]->GetValue();

	timesec.startMinute = pnb[1]->GetValue();
	timesec.endMinute = pnb[3]->GetValue();

// 	if(CConfigLocation::getLatest().iTimeFormat == TF_12)
// 	{
// 	}

	//调用时间变动回调函数
	ItemProc(m_onChanged);
}

CNumberBox* CTimingCtrl::GetNumberBox(int index)
{
	if(index >= 0 && index <= 3){
		return pnb[index];
	}
	return NULL;
}


int CTimingCtrl::Hour12To24(int hour12, int am)
{
	if(am == 0)
	{
		if(hour12 == 12)
		{
			return 0;
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

int CTimingCtrl::Hour24To12(int hour24)
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

int CTimingCtrl::Hour24ToAMPM(int hour24)
{
	if(hour24 >= 12 && hour24 < 24)
	{
		return 1; //下午
	}
	return 0; //上午
}

void CTimingCtrl::SetDefaultBitmap(TimingCtrlBitmap region, VD_PCSTR name)
{
	switch(region) {
	case TCB_NORMAL:
		bmp_tcb_normal = VD_LoadBitmap(name);
		break;
	case TCB_SELECTED:
		bmp_tcb_selected = VD_LoadBitmap(name);
		break;
	case TCB_DISABLED:
		bmp_tcb_disabled = VD_LoadBitmap(name);
		break;
	default:
		break;
	}
}
