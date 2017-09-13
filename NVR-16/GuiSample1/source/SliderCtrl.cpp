//
//  "$Id: SliderCtrl.cpp 280 2008-12-17 06:04:55Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#include "GUI/Ctrls/Page.h"


VD_BITMAP* CSliderCtrl::bmp_slider_normal;
VD_BITMAP* CSliderCtrl::bmp_slider_select;
VD_BITMAP* CSliderCtrl::bmp_tracker_normal;
VD_BITMAP* CSliderCtrl::bmp_tracker_select;
VD_BITMAP* CSliderCtrl::bmp_tracker_pushed;


///////////////////////////////////////////////////////////////////
////////////        CSliderCtrl

#define TRACKER_WIDTH 150
#define SLIDER_WITH 245


CSliderCtrl::CSliderCtrl(VD_PCRECT pRect, CPage * pParent,int vmin,int vmax,CTRLPROC vproc,int vstyle, int nWheelStep)
:CItem(pRect, pParent, IT_SLIDERCTRL,vstyle|styleEditable|styleAutoFocus),
min(vmin),max(vmax),newpos(0),curpos(0),editpos(0),
m_pBmpNormal(NULL),m_pBmpElapsed(NULL),m_pBmpSelect(NULL),m_pBmpDisable(NULL),
m_bWithBmp(FALSE),m_bBmpIntergrate(FALSE),track(FALSE),tracker_offset(0),m_nWheelStep(nWheelStep),
m_onValueChanged(vproc)
{
	//如果这里不设置默认大小就比较麻烦。
	if (!pRect)
	{
		m_Rect = CRect(0,  0, TEXT_WIDTH*8,  CTRL_HEIGHT);
	}

	if(bmp_slider_normal && bmp_slider_select && bmp_tracker_normal && bmp_tracker_select )
	{
		tracker_width = bmp_tracker_normal->width;
		m_dwStyle|=styleUseBmp;
	}
	else
	{
		//printf("%s 1\n", __func__);
		tracker_width = m_Rect.Height()/2;
	}

	if ((m_dwStyle & sliderNoNum) == sliderNoNum)
	{
		m_iSliderWidth = m_Rect.Width();
	}
	else
	{
		m_iSliderWidth = m_Rect.Width() - TEXT_WIDTH*2;
	}
	if(m_iSliderWidth == 0)
	{
		m_iSliderWidth = 1;//防止作为除数时出错
	}
	SetRectEmpty(&rect_elapsed);
	SetRectEmpty(&rect_normal);
	SetRectEmpty(&rect_selected);
	if(m_dwStyle & sliderTransparent)
	{
		m_bTransparent = TRUE;
	}
	else
	{
		m_bTransparent = FALSE;
	}
	
}

CSliderCtrl::~CSliderCtrl()
{

}

CSliderCtrl* CreateSliderCtrl(VD_PCRECT pRect, CPage * pParent,int vmin /* = 0 */,int vmax /* = 100 */, CTRLPROC vproc /* = NULL */, int vstyle /* = 0 */, int nWheelStep /* = 1 */)
{
	//printf("%s %d, %d, %d, %d\n", __func__, pRect->left, pRect->right, pRect->top, pRect->bottom);
	return new CSliderCtrl(pRect, pParent, vmin, vmax, vproc,vstyle,nWheelStep);
}

/*void CSliderCtrl::SetState(ISTATE vstate)
{
	GetFlag(IF_CAPTURED) = FALSE;
	track = FALSE;
	CItem::SetState(vstate);
}*/

void CSliderCtrl::DrawSliderCtrl(CTRL_STATE state){
	switch(state)
	{
	case ctrlStateNormal:
		m_DC.WideBitmap(CRect(0,m_Rect.Height()/2-3,m_iSliderWidth,m_Rect.Height()/2+3),
			bmp_slider_normal,CRect(0,0,bmp_slider_normal->width,bmp_slider_normal->height));
		m_DC.WideBitmap(CRect(tracker_offset,m_Rect.Height()/2-bmp_tracker_normal->height/2,tracker_offset + tracker_width,m_Rect.Height()/2+bmp_tracker_normal->height/2), bmp_tracker_normal,NULL);
		break;
	case ctrlStateSelect:
		m_DC.WideBitmap(CRect(0,m_Rect.Height()/2-3,m_iSliderWidth,m_Rect.Height()/2+3),
			bmp_slider_select,CRect(0,0,bmp_slider_select->width,bmp_slider_select->height));
		m_DC.WideBitmap(CRect(tracker_offset,m_Rect.Height()/2-bmp_tracker_select->height/2,tracker_offset + tracker_width,m_Rect.Height()/2+bmp_tracker_select->height/2), bmp_tracker_select,NULL);
		break;
	default:
	    break;
	}

}
VD_BOOL CSliderCtrl::MsgProc(uint msg, uint wpa, uint lpa)
{
	int key;
	int px, py;
	signed char delta;
	int vtracker_offset;
	int len,temp;

	newpos = curpos;
	len  = 1;
	temp =  max;
	while(temp /= 10)
		len++;
	switch(msg){
	case XM_KEYDOWN:
		key = wpa;
		switch(key){
		case KEY_RET://cw_slider
			break;
		case KEY_UP:
 
			if(newpos>min)
				newpos--;
			break;

		case KEY_DOWN:
 
			if(newpos<max)
				newpos++;
			break;
		case KEY_0:
		case KEY_1:
		case KEY_2:
		case KEY_3:
		case KEY_4:
		case KEY_5:
		case KEY_6:
		case KEY_7:
		case KEY_8:
		case KEY_9:
		case KEY_10:
			if(m_dwStyle&sliderNoNum){
				break;
			}
			int num;
			num = (key-KEY_0)%10;
			if(editpos == 0)
				newpos = num;
			else
				newpos = newpos*10+num;
			if(editpos != 0 || num !=0)//第一个零不用移动editpos
			{
				editpos++;
			}
			if(editpos == len){
				if(newpos>max)
					newpos = max;
				editpos = 0;
			}
			break;
		default:
			return FALSE;
		}
		break;

	case XM_MOUSEWHEEL:
		if(m_dwStyle&sliderNoWheel){
			break;
		}
		delta = (char)VD_HIWORD(wpa);
		if(delta < 0){
//			if(newpos>min)
				newpos -= m_nWheelStep;
		}else if(delta > 0){
//			if(newpos<max)
				newpos += m_nWheelStep;
		}
		
		if(newpos<min)
			newpos=min;
		if(newpos>max)
			newpos=max;
		
		break;

	case XM_LBUTTONDOWN:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		temp = GetAt(px, py);
		if(temp == SA_POSITION){
			newpos = min + (max - min + 1) * (px - tracker_width/2 - m_Rect.left) / MAX(1, (m_iSliderWidth - tracker_width));
			if(newpos < min){
				newpos = min;
			}else if(newpos > max){
				newpos = max;
			}
		}else if(temp == SA_TRACK){
			track = TRUE;
			SetFlag(IF_CAPTURED, TRUE);
			tracker_pick = px - tracker_offset - m_Rect.left;
		}else{
			return FALSE;
		}
		break;

	case XM_LBUTTONUP://拖动游标结束时
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(track){
			if(max - min > 0){
				newpos = min + (max - min) * tracker_offset / MAX(1, (m_iSliderWidth - tracker_width));
			}
			track = FALSE;
			SetFlag(IF_CAPTURED, FALSE);
		}
		break;
		
	case XM_MOUSEMOVE://拖动游标时, curpos 不改变, tracker_offset 改变
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(track){
			if(px - tracker_pick < m_Rect.left){
				px = tracker_pick + m_Rect.left;
			}else if(px - tracker_pick > m_Rect.left + m_iSliderWidth - tracker_width){
				px = m_Rect.left + m_iSliderWidth - tracker_width + tracker_pick;
			}
			vtracker_offset = px - tracker_pick - m_Rect.left;
			if(vtracker_offset != tracker_offset){
				temp = tracker_offset;
				tracker_offset = vtracker_offset;
				if(max > min)//在拖动，重新计算游标值
				{
					newpos = min + (max - min) * tracker_offset / MAX(1, (m_iSliderWidth - tracker_width));
				}
				else
				{
					newpos = min;
				}
				DrawTracking(temp, tracker_offset);
			}
			return TRUE;
		}
		return FALSE;

	default:
		return FALSE;
	}

	//滑块拖动
	if(newpos != curpos)
	{
		curpos = newpos;
		ItemProc(m_onValueChanged);           //zmx 04.7.27
		vtracker_offset = tracker_offset;
		UpdateTracker();
		DrawTracking(vtracker_offset, tracker_offset);
	}

	return TRUE;
}

void CSliderCtrl::Draw()
{
	if(!DrawCheck()){
		return;
	}

	if((m_dwStyle & sliderSetBmp) && m_bTransparent)
	{
		DrawBackground();
	}
	
	m_DC.Lock();
	if((m_dwStyle & sliderSetBmp) == sliderSetBmp)
	{//支持控件图片

		if(m_bTransparent)
		{
			m_DC.SetBkMode(BM_TRANSPARENT);
		}
		else
		{
			m_DC.SetBkMode(BM_NORMAL);
		}
		if(GetFlag(IF_ENABLED))
		{
			//计算需要平铺的区域
			CRect bmp_background_rect(m_pBmpBackground->width * (tracker_offset+tracker_width / 2) / m_iSliderWidth,
				0, m_pBmpBackground->width, m_pBmpBackground->height);
			CRect bmp_elapsed_rect(0, 0, m_pBmpElapsed->width * (tracker_offset+tracker_width / 2) / m_iSliderWidth,
				m_pBmpElapsed->height);
			

			m_DC.WideBitmap(CRect(tracker_offset+tracker_width/2,0,m_Rect.Width(),m_Rect.Height()), m_pBmpBackground, bmp_background_rect);
			m_DC.WideBitmap(CRect(0,0,tracker_offset+tracker_width/2,m_Rect.Height()),m_pBmpElapsed, bmp_elapsed_rect);
		}
		else
		{
			m_DC.WideBitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()), m_pBmpBackground);
		}
		m_DC.UnLock();
		DrawFrame();
		return;
	}

	//画背景
	m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));

	m_DC.UnLock();
	DrawFrame();
}

void CSliderCtrl::DrawFrame()
{
	if(!DrawCheck()){
		return;
	}

	m_DC.Lock();
	if((m_dwStyle & sliderSetBmp) == sliderSetBmp)
	{
		m_DC.SetBkMode(BM_TRANSPARENT);
		if(!GetFlag(IF_ENABLED))
		{
			if (m_pBmpDisable)
			{
				m_DC.Bitmap(CRect(tracker_offset,0,tracker_offset+tracker_width,m_Rect.Height()),m_pBmpDisable);
			}
		}
		else if(GetFlag(IF_SELECTED))
		{
			if (m_pBmpSelect)
			{
				m_DC.Bitmap(CRect(tracker_offset,0,tracker_offset+tracker_width,m_Rect.Height()),m_pBmpSelect);
			}
		}
		else 
		{
			if (m_pBmpNormal)
			{
				m_DC.Bitmap(CRect(tracker_offset,0,tracker_offset+tracker_width,m_Rect.Height()),m_pBmpNormal);
			}
		}
		m_DC.UnLock();
		return;
	}

	if((m_dwStyle & styleUseBmp) == styleUseBmp)
	{
		m_DC.SetBkMode(BM_TRANSPARENT);
		if(GetFlag(IF_SELECTED))
		{
			DrawSliderCtrl(ctrlStateSelect);
		}
		else
		{
			DrawSliderCtrl(ctrlStateNormal);
		}
	}
	else
	{
		m_DC.SetBkMode(BM_TRANSPARENT);
		if(GetFlag(IF_SELECTED))
		{	
				m_DC.SetPen(VD_GetSysColor(COLOR_FRAMESELECTED),VD_PS_SOLID,2);
				m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
		}
		else
		{

				m_DC.SetPen(VD_GetSysColor(COLOR_FRAME),VD_PS_SOLID,2);
				m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXT));
		}
		if(!GetFlag(IF_ENABLED))
		{
			m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXTDISABLED));
		}

		//画边框
		m_DC.SetRgnStyle(RS_HOLLOW);
		m_DC.Rectangle(CRect(0,m_Rect.Height()/2-3,m_iSliderWidth,m_Rect.Height()/2+3), 1, 1);
		//画游标
		m_DC.SetRgnStyle(RS_NORMAL);
		m_DC.Rectangle(CRect(tracker_offset,m_Rect.Height()/6,tracker_offset + tracker_width,m_Rect.Height()*5/6), 3, 3);

	}

	m_DC.UnLock();

	DrawPosText();
}

void CSliderCtrl::DrawTracking(int old_track_offset, int new_track_offset)
{
	if(!DrawCheck())
	{
		return;
	}

	m_DC.Lock();
	//计算需要平铺的区域
	if((m_dwStyle & sliderSetBmp) == sliderSetBmp)
	{
		CRect bmp_background_rect, bmp_elapsed_rect;
		m_DC.SetBkMode(BM_NORMAL);
		if (new_track_offset > old_track_offset)
		{
			bmp_elapsed_rect.SetRect(
				m_pBmpElapsed->width * old_track_offset / m_iSliderWidth,
				0,
				m_pBmpElapsed->width * (new_track_offset + tracker_width / 2) / m_iSliderWidth,
				m_pBmpElapsed->height);
			m_DC.WideBitmap(CRect(old_track_offset, 0, new_track_offset + tracker_width / 2, m_Rect.Height()),
				m_pBmpElapsed, bmp_elapsed_rect);
			
			if (new_track_offset < old_track_offset + tracker_width / 2)
			{
				bmp_background_rect.SetRect(
					m_pBmpBackground->width * (new_track_offset + tracker_width / 2) / m_iSliderWidth,
					0,
					m_pBmpBackground->width * (old_track_offset + tracker_width) / m_iSliderWidth,
					m_pBmpBackground->height);
				m_DC.WideBitmap(CRect(new_track_offset + tracker_width / 2, 0, old_track_offset + tracker_width, m_Rect.Height()),
					m_pBmpBackground, bmp_background_rect);
			}
		}
		else
		{
			bmp_background_rect.SetRect(
				m_pBmpBackground->width * (new_track_offset + tracker_width / 2) / m_iSliderWidth,
				0,
				m_pBmpBackground->width * (old_track_offset + tracker_width) / m_iSliderWidth,
				m_pBmpBackground->height);
			m_DC.WideBitmap(CRect(new_track_offset + tracker_width / 2, 0, old_track_offset + tracker_width, m_Rect.Height()),
				m_pBmpBackground, bmp_background_rect);

			if (old_track_offset < new_track_offset + tracker_width / 2)
			{
				bmp_elapsed_rect.SetRect(
					m_pBmpElapsed->width * old_track_offset / m_iSliderWidth,
					0,
					m_pBmpElapsed->width * (new_track_offset + tracker_width / 2) / m_iSliderWidth,
					m_pBmpElapsed->height);
				m_DC.WideBitmap(CRect(old_track_offset, 0, new_track_offset + tracker_width / 2, m_Rect.Height()),
					m_pBmpElapsed, bmp_elapsed_rect);
			}
		}
		m_DC.UnLock();
		DrawFrame();
		return;
	}


	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
	m_DC.Rectangle(CRect(old_track_offset,0, old_track_offset + tracker_width, m_Rect.Height()));

	m_DC.UnLock();

	DrawFrame();
}

//显示数值
void CSliderCtrl::DrawPosText()
{
	char buf[16];
	sprintf(buf,"%d",newpos);
	m_DC.Lock();


	if (GetFlag(IF_SELECTED))
	{	
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
	}
	else
	{
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXT));
	}

	if (!(m_dwStyle&sliderNoNum))
	{
		m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(m_iSliderWidth,0,m_Rect.Width(),TEXT_HEIGHT));
		m_DC.SetTextAlign(VD_TA_CENTER);
		m_DC.VD_TextOut(CRect(m_iSliderWidth,0,m_Rect.Width(),TEXT_HEIGHT),buf);
	}
	m_DC.UnLock();
}

void CSliderCtrl::SetFocus(VD_BOOL flag)
{
	if(!flag)
	{
		editpos = 0;
		if(curpos < min)
			curpos = min;
		else if(curpos > max)
			curpos = max;
	}
	CItem::SetFocus(flag);
}
	
void CSliderCtrl::Select(VD_BOOL flag)
{
	//printf("CSliderCtrl::Select\n");
	ShowTip(flag);

	SetFlag(IF_SELECTED, flag);
	if(!DrawCheck()){
		return;
	}

	DrawFrame();
}

int CSliderCtrl::GetAt(int px, int py)
{
	if(PtInRect(m_Rect, px, py))
	{
		//是否在滑动块的区域内
		if(px < m_Rect.left + tracker_offset || px > m_Rect.left + tracker_offset + tracker_width)
		{
			return SA_POSITION;
		}
		else
		{
			return SA_TRACK;
		}
	}
	return -1;
}

void CSliderCtrl::SetPos(int pos)
{
	if(curpos == pos || pos < min || pos > max)
		return;
	curpos = pos;
	UpdateTracker();
	Draw();
}

int CSliderCtrl::GetPos(){
	return curpos;
}

VD_BOOL CSliderCtrl::GetTrackState(int * piTrackerOffset)
{
	*piTrackerOffset = min + (max - min) * tracker_offset / MAX(1, (m_iSliderWidth - tracker_width));
	return track;
}

void CSliderCtrl::StepIt(int step)
{
	curpos += step;
	if(curpos>max)
		curpos = max;
	else if(curpos<min)
		curpos = min;
	UpdateTracker();
	Draw();
}

void CSliderCtrl::SetBitmap(VD_BITMAP * pBmpNormal, VD_PCRECT pRectElapsed, VD_PCRECT pRectNormal, VD_PCRECT pRectSelected)
{

	if (!pBmpNormal) {
		return;
	}
	m_dwStyle&=~styleUseBmp;
	m_dwStyle|=sliderSetBmp;

	m_pBmpNormal = pBmpNormal;
	
	if(pRectElapsed)
	{
		rect_elapsed = *pRectElapsed;
	}
	if(pRectNormal)
	{
		rect_normal = *pRectNormal;
		tracker_width = rect_normal.right - rect_normal.left;
	}
	if(pRectSelected)
	{
		rect_selected = *pRectSelected;
	}
	Draw();
}

void CSliderCtrl::SetBitmap(VD_BITMAP* pBmpNormal, VD_BITMAP* pBmpSelect, VD_BITMAP* pBmpElapsed, VD_BITMAP* pBmpDisable, VD_BITMAP * pBmpBackground, int iTrackerWidth)
{
	m_dwStyle&=~styleUseBmp;
	m_dwStyle|=sliderSetBmp;

	m_pBmpNormal = pBmpNormal;
	m_pBmpSelect = pBmpSelect;
	m_pBmpElapsed = pBmpElapsed;
	m_pBmpDisable = pBmpDisable;
	m_pBmpBackground = pBmpBackground;
	tracker_width = iTrackerWidth;

	Draw();
}

//如果没有在拖动，重新计算滑块的坐标
void CSliderCtrl::UpdateTracker()
{
	newpos = curpos;
	if(max > min)
	{
		tracker_offset = (m_iSliderWidth - tracker_width) * (curpos- min) / (max - min);
	}
	else
	{
		tracker_offset = 0;
	}
}
void CSliderCtrl::SetDefaultBitmap(SliderCtrlBitmap region, VD_PCSTR name)
{
	printf("CSliderCtrl::SetDefaultBitmap region: %d\n");
	switch(region) {
	case SDB_RAIL_NORMAL:
		bmp_slider_normal =VD_LoadBitmap(name);
		break;
	case SDB_RAIL_SELECTED:
		bmp_slider_select =VD_LoadBitmap(name);
		break;
	case SDB_TRACKER_NORMAL:
		bmp_tracker_normal =VD_LoadBitmap(name);
		break;
	case SDB_TRACKER_SELECTED:
		bmp_tracker_select =VD_LoadBitmap(name);
		break;
	case SDB_TRACKER_PUSHED:
		bmp_tracker_pushed =VD_LoadBitmap(name);
		break;
	default:
		break;
	}

}
