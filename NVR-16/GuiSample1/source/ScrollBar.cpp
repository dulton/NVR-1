//
//  "$Id: ScrollBar.cpp 279 2008-12-17 05:58:45Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#include "GUI/Ctrls/Page.h"

VD_BITMAP* CScrollBar::m_bmpNormal = NULL;
VD_BITMAP* CScrollBar::m_bmpSelected = NULL;
VD_BITMAP* CScrollBar::m_bmpPushed = NULL;
VD_BITMAP* CScrollBar::m_bmpDisabled = NULL;

///////////////////////////////////////////////////////////////////
////////////        CScrollBar
CScrollBar::CScrollBar(VD_PCRECT pRect, CPage * pParent,uint vstyle,int vmin ,int vmax , int vpage, CTRLPROC onTrackMove)
: CItem(pRect, pParent, IT_SCROLLBAR,vstyle|styleEditable|styleAutoFocus)
{
	//	modified history:scrollbar控件是否属于弹出式通过控件属性参数vstyle进行判断		--by wangqin  20070316
	min = vmin;
	max = vmax/10;
	if(vpage<=0){
		page = 1;
	}else{
		page = vpage;
	}
	curpos = 0;
	track = FALSE;
	tracker_offset = 0;
	//tracker_height = m_Rect.Height();
	//m_BkColor = VD_GetSysColor(VD_COLOR_WINDOW);
	m_BkColor = VD_GetSysColor(COLOR_FRAME);
	m_BorderColor = VD_GetSysColor(COLOR_FRAME);
	m_SelBorderColor = VD_GetSysColor(COLOR_FRAMESELECTED);
	m_MidLineColor = VD_GetSysColor(COLOR_CTRLTEXT);

	UpdateTracker();

	m_onTrackMove = onTrackMove;
	
}

CScrollBar::~CScrollBar()
{
}

/*
 *	类似listbox需要重新处理.--by wangqin 需要整合所有资源和item类似.
 */
CScrollBar* CreateScrollBar(VD_PCRECT pRect, 
												CPage * pParent,
												uint vstyle /* = scrollbarY */, 
												int vmin /* = 0 */,
												int vmax /* = 100 */, 
												int vpage /* = 1 */,
												CTRLPROC onTrackMove)
{
	return new CScrollBar(pRect,pParent,vstyle,vmin,vmax,vpage,onTrackMove);
}

VD_BOOL CScrollBar::MsgProc(uint msg, uint wpa, uint lpa)
{
	int px, py;
	signed char delta;
	int temp; 
	int newpos;
	int vtracker_offset;
	newpos = curpos;
	switch(msg){
		
	case XM_KEYDOWN:
		switch(wpa)
		{
			case KEY_RET:
			{
				if(newpos<max)
					newpos++;
				else
					newpos=0;
			}break;
			case KEY_UP:
			{
				newpos--;
				if (newpos<min)
					newpos=max;
			}break;
			case KEY_DOWN:
			{
				newpos++;
				if(newpos>max)
					newpos=0;
			}break;
			default:
				return FALSE;		
		}
		
	case XM_MOUSEWHEEL:
		delta = (char)VD_HIWORD(wpa);
		if(delta < 0){
			if(newpos>min)
				newpos--;
		}else if(delta > 0){
			
			if(newpos<max)
				newpos++;
		}
		break;

	case XM_LBUTTONDOWN:
	case XM_LBUTTONDBLCLK:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		temp = GetAt(px, py);
		if(temp == SA_PAGEUP){
			newpos -= MAX(1, page - 1);
			if(newpos < min){
				newpos = min;
			}
		}else if(temp == SA_PAGEDOWN){
			newpos += MAX(1, page - 1);
			if(newpos > max - page + 1){
				newpos = max - page + 1;
			}
		}else if(temp == SA_TRACK){
			track = TRUE;
			SetFlag(IF_CAPTURED, TRUE);
			if(m_pOwner){
				m_pOwner->SetCapture(TRUE);
			}
			tracker_pick = py - tracker_offset - m_Rect.top;

		}else{
			return FALSE;
		}
		break;

	case XM_LBUTTONUP:
		if(track){
			if(max - min > 0){
				curpos = min + (max - min + 1 - page) * tracker_offset / MAX(1, (m_Rect.Height() - tracker_height));
			}

			//if(m_onTrackMove )
			//{
			//	(this->*m_onTrackMove)(curpos);
			//}	

			ItemProc(m_onTrackMove);
			
			track = FALSE;
			SetFlag(IF_CAPTURED, FALSE);
			if(m_pOwner){
				m_pOwner->SetCapture(FALSE);
				m_pOwner->MsgProc(XM_SCROLL, curpos, (uint)this);
			}
			Draw();
			return TRUE;
		}else{
			return FALSE;
		}
		break;
		
	case XM_MOUSEMOVE:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(track){
			if(py - tracker_pick < m_Rect.top){
				py = m_Rect.top + tracker_pick;
				//printf("@@@22  py = %d \n",py);
			}else if(py - tracker_pick > m_Rect.bottom - tracker_height){
				py = m_Rect.bottom - tracker_height + tracker_pick;
			}
			vtracker_offset = py - m_Rect.top - tracker_pick;

			if(vtracker_offset != tracker_offset){
				DrawBar(FALSE);
				tracker_offset = vtracker_offset;
				DrawBar(TRUE);
			}
		}else{
			return FALSE;
		}
		break;

	default:
		return FALSE;
	}

	//滑块拖动
	if(newpos != curpos){
		curpos = newpos;
		if(m_pOwner){
			m_pOwner->MsgProc(XM_SCROLL, curpos, (uint)this);
		}
		DrawBar(FALSE);
		UpdateTracker();
		DrawBar(TRUE);
	}

	//if(!track && m_onTrackMove)
	//{
	//	(this->*m_onTrackMove)(newpos);
	//}	

	if( !track)
	{
		ItemProc(m_onTrackMove);
	}
	
	return TRUE;
}

void CScrollBar::Draw()
{
	if(!DrawCheck()){
		return;
	}

	m_DC.Lock();

	m_DC.SetRgnStyle(RS_NORMAL);
	if(GetFlag(IF_SELECTED))	
		m_DC.SetPen(m_SelBorderColor, VD_PS_SOLID, 2);
	else
		m_DC.SetPen(m_BorderColor, VD_PS_SOLID, 2);
	m_DC.SetBrush(m_BkColor);
	//m_DC.SetBrush(VD_RGB(255,0,0));
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));

	
	m_DC.UnLock();

	DrawBar(TRUE);
}

void CScrollBar::DrawBar(VD_BOOL paint)
{
	if(!DrawCheck()){
		return;
	}

	m_DC.Lock();
	if(paint)
	{

		m_DC.SetRgnStyle(RS_HOLLOW|RS_RAISED);
		if(GetFlag(IF_SELECTED))	
			m_DC.SetPen(m_SelBorderColor, VD_PS_SOLID, 2);
		else
			m_DC.SetPen(m_BorderColor, VD_PS_SOLID, 2);
		//m_DC.Rectangle(CRect(1, tracker_offset + 1, m_Rect.Width() - 1, tracker_offset + tracker_height - 1));
		m_DC.Rectangle(CRect(2, tracker_offset + 1, m_Rect.Width() - 2, tracker_offset + tracker_height - 1));
		m_DC.SetPen(m_MidLineColor, VD_PS_SOLID, 2);
		m_DC.MoveTo(2, tracker_offset + tracker_height/2);
		m_DC.LineTo(m_Rect.Width()-4, tracker_offset + tracker_height/2);
		m_DC.MoveTo(4, tracker_offset + tracker_height/2 - 4);
		m_DC.LineTo(m_Rect.Width() - 4, tracker_offset + tracker_height/2 - 4);
		m_DC.MoveTo(4, tracker_offset + tracker_height/2 + 4);
		m_DC.LineTo(m_Rect.Width() - 4, tracker_offset + tracker_height/2 + 4);

	}
	else
	{

		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.SetBrush(m_BkColor);
		//m_DC.Rectangle(CRect(1, tracker_offset + 1, m_Rect.Width() - 1, tracker_offset + tracker_height - 1));
		m_DC.Rectangle(CRect(2, tracker_offset + 1, m_Rect.Width() - 2, tracker_offset + tracker_height - 1));

	}
	m_DC.UnLock();
}

int CScrollBar::GetAt(int px, int py)
{
	if(PtInRect(m_Rect, px, py)){
		if(py < m_Rect.top + tracker_offset){
			return SA_PAGEUP;
		}else if(py > m_Rect.top + tracker_offset + tracker_height){
			return SA_PAGEDOWN;
		}else{
			return SA_TRACK;
		}
	}
	return -1;
}

void CScrollBar::SetRange(int vmin, int vmax, VD_BOOL redraw /* = TRUE */)
{
	if(vmin > vmax){
		return;
	}
	min = vmin;
	max = vmax;
	if(curpos < min){
		curpos = min;
	}
	if(curpos >max){
		curpos = max;
	}
	UpdateTracker();
	if(redraw){
		Draw();
	}
}

void CScrollBar::SetPos(int pos, VD_BOOL redraw /* = TRUE */)
{
	if(pos < min || pos > max)
		return;
	curpos = pos;
	UpdateTracker();
	if(redraw){
		Draw();
	}
}

int CScrollBar::GetPos(){
	return curpos;
}

void CScrollBar::SetPage(int vpage, VD_BOOL redraw /* = TRUE */)
{
	if(vpage <= 0 || vpage >= max - min + 1)
		return;
	page = vpage;
	UpdateTracker();
	if(redraw){
		Draw();
	}
}

void CScrollBar::UpdateTracker()
{
	tracker_height = MAX(CTRL_HEIGHT1, m_Rect.Height() * page / (max - min + 1));
	if(max - min + 1 - page <= 0) // 
	{
		tracker_offset = 0;
	}
	else
	{
		tracker_offset = MAX(1,(m_Rect.Height()-tracker_height))*MAX(0,curpos)/(max - min + 1 - page);
	}
}

void CScrollBar::SetColor(int nRegion, VD_COLORREF color, VD_BOOL redraw /* = FALSE */)
{
	switch(nRegion) {
	case SLB_BK:
		m_BkColor = color;
		break;
	case SLB_BORDER:
		m_BorderColor = color;
		break;
	case SLB_SELBORDER:
		m_SelBorderColor = color;
		break;
	case SLB_NOSEL_ITEM_TEXT:
		m_MidLineColor = color;
		break;
	default:
		break;
	}
	if(redraw)
	{
		Draw();
	}
}

void CScrollBar::SetDefaultBitmap(ScrollBarBitmap region, VD_PCSTR name)
{
	switch(region)
	{
	case SBB_NORMAL:
		m_bmpNormal = VD_LoadBitmap(name);
		break;
	case SBB_SELECTED:
		m_bmpSelected = VD_LoadBitmap(name);
		break;
	case SBB_PUSHED:
		m_bmpPushed = VD_LoadBitmap(name);
		break;
	case SBB_DISABLED:
		m_bmpDisabled = VD_LoadBitmap(name);
		break;
	default:
		break;
	}

}


