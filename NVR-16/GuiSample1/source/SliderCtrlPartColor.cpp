//
//  "$Id: SliderCtrl.cpp 280 2008-12-17 06:04:55Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//
//yaogang modify for playback 20170906


#include "GUI/Ctrls/Page.h"

Range::Range(int s, int e)
{
	if (s < e)
	{
		start = s;
		end = e;
	}
	else
	{
		start = end = 0;
	}
}

Range::~Range()
{
	
}

#if 0
//val 是否在this 中
VD_BOOL Range::isInRange(int val) //[s, e)
{
	if (start <= val && val < end)
	{
		return TRUE;
	}

	return FALSE;
}
#endif

//this 是否在r 中
VD_BOOL Range::isInRange(const Range &r) const
{
	if (start <= r.start && end <= r.end)
	{
		return TRUE;
	}

	return FALSE;
}

VD_BOOL Range::selfCheck() const
{
	if (start < end)
	{
		return TRUE;
	}

	return FALSE;
}

int Range::Width() const
{
	return end-start;
}

VD_BOOL operator ==(const Range &lhs, const Range &rhs)
{
	if (lhs.start == rhs.start 
		&& lhs.end == rhs.end)
	{
		return TRUE;
	}

	return FALSE;
}

VD_BOOL operator !=(const Range &lhs, const Range &rhs)
{
	if (lhs == rhs)
	{
		return FALSE;
	}

	return TRUE;
}

//friend
VD_BOOL isInRange(int val, const Range &r)//val 是否在r 中
{
	if (r.start <= val && val <= r.end)
	{
		return TRUE;
	}

	return FALSE;
}

VD_BOOL isInRangeWithoutStart(int val, const Range &r)
{
	if (r.start < val && val <= r.end)
	{
		return TRUE;
	}

	return FALSE;
}

VD_BOOL isInRangeWithoutEnd(int val, const Range &r)
{
	if (r.start <= val && val < r.end)
	{
		return TRUE;
	}

	return FALSE;
}

VD_BOOL isInRangeWithoutBoth(int val, const Range &r)	
{
	if (r.start < val && val < r.end)
	{
		return TRUE;
	}

	return FALSE;
}


//CSliderCtrlPartColor
CSliderCtrlPartColor::CSliderCtrlPartColor(VD_PCRECT pRect, CPage * pParent, int vmin,int vmax, int SplitLineMax, CTRLPROC vproc)
:CItem(pRect, pParent, IT_SLIDERCTRL, styleEditable|styleAutoFocus),
m_Mutex(MUTEX_RECURSIVE),
m_bTracker(TRUE),
m_sRealRange(vmin, vmax),
m_onValueChanged(vproc),
curpos(0),
newpos(0),
track(FALSE),
tracker_offset(0),
tracker_pick(0),
m_iSplitLineMax(SplitLineMax),
m_iSplitLineNum(0),
m_ColorSplitLine(VD_RGB(102, 102, 102)),
m_ColorBlank(VD_RGB(200, 200, 200)),
m_Color(VD_RGB(0, 174, 255)),
m_sDispRange(m_sRealRange)
{
	m_iSliderWidth = m_Rect.Width();
	tracker_width = m_Rect.Height()/2;
/*
	for (int i = 0; i<m_iSplitLineMax; i++)
	{
		CStatic* pSplit = CreateStatic(CRect(0,0,0,0), this, "");
		pSplit->SetBkColor(m_ColorSplitLine);
		pSplit->Show(FALSE);
		
		m_vSplitLine.push_back(pSplit);
	}
*/
}

CSliderCtrlPartColor::~CSliderCtrlPartColor()
{
	#if 0
	std::vector<CStatic*>::iterator iter;
	
	if (!m_vSplitLine.empty())
	{
		for (iter = m_vSplitLine.begin();
			iter != m_vSplitLine.end();
			++iter)
		{
			delete *iter;
			*iter = NULL;
		}

		m_vSplitLine.clear();
	}
	#endif
}

int CSliderCtrlPartColor::SetPos(int pos)
{
	CGuard guard(m_Mutex);
	
	if(curpos == pos)
	{
		return 0;
	}
	
	if (!isInRange(pos, m_sRealRange))
	{
		return 1;
	}

	if (!isInRange(pos, m_sDispRange))
	{
		return 1;
	}
	
	curpos = pos;
	UpdateTracker();
	Draw();
	
	return 0;
}

int CSliderCtrlPartColor::GetPos()
{
	CGuard guard(m_Mutex);
	
	return curpos;
}

int CSliderCtrlPartColor::GetAt(int px, int py)//是否在滑动块的区域内
{
	CGuard guard(m_Mutex);
	
	if(PtInRect(m_Rect, px, py))
	{
		//是否在滑动块的区域内
		if(px < m_Rect.left + tracker_offset || px > m_Rect.left + tracker_offset + tracker_width)
		{
			return CSliderCtrl::SA_POSITION;
		}
		else
		{
			return CSliderCtrl::SA_TRACK;
		}
	}
	
	return -1;
}

void CSliderCtrlPartColor::SetTrackerEnable(VD_BOOL b)
{
	CGuard guard(m_Mutex);
	
	if (b != m_bTracker)
	{
		m_bTracker = b;
		Draw();
	}
}

void CSliderCtrlPartColor::SetSplitLineNum(int n)
{
	CGuard guard(m_Mutex);
	
	if (n != m_iSplitLineNum)
	{
		m_iSplitLineNum = n;
		Draw();
	}
}

void CSliderCtrlPartColor::SetSplitLineColor(VD_COLORREF color /*= VD_RGB(232,232,232)*/ )
{
	CGuard guard(m_Mutex);
	
	if (color != m_ColorSplitLine)
	{
		m_ColorSplitLine = color;
		Draw();
	}
}

void CSliderCtrlPartColor::SetColor(VD_COLORREF color /*= VD_RGB(232,232,232)*/ )
{
	CGuard guard(m_Mutex);
	
	if (color != m_Color)
	{
		m_Color = color;
		Draw();
	}
}

void CSliderCtrlPartColor::SetBlankColor(VD_COLORREF color /*= VD_RGB(232,232,232)*/ )
{
	CGuard guard(m_Mutex);
	
	if (color != m_ColorBlank)
	{
		m_ColorBlank = color;
		Draw();
	}
}

//把指定区域放大显示
int CSliderCtrlPartColor::SetDisplayRange(const Range &r)
{
	CGuard guard(m_Mutex);

	if (!r.selfCheck())
	{
		return 1;
	}

	if (!r.isInRange(m_sRealRange))
	{
		return 1;
	}
	 
	if (r == m_sDispRange)
	{
		return 0;
	}


	m_sDispRange = r;

	Draw();

	return 0;
}

//同时显示区域和分割线数量
//eg: playback module 24hr/2hr/1hr/30min
int CSliderCtrlPartColor::SetDisplayRange(const Range &r, int split_line_num)
{
	CGuard guard(m_Mutex);
	BOOL b_draw = FALSE;

	if (!r.selfCheck())
	{
		return 1;
	}
	
	if (!r.isInRange(m_sRealRange))
	{
		return 1;
	}
	
	if (r != m_sDispRange)
	{
		m_sDispRange = r;
		b_draw = TRUE;
	}

	if (split_line_num != m_iSplitLineNum)
	{
		m_iSplitLineNum = split_line_num;
		b_draw = TRUE;		
	}

	if (b_draw)
	{
		Draw();
	}

	return 0;
}

void CSliderCtrlPartColor::Draw()
{
	DrawBackground();
	DrawRange();
	DrawSplitLine();
	DrawTracker();
}

//绘制顺序
//	DrawBackground
//	DrawRange
//	DrawSplitLine
//	DrawTracker

void CSliderCtrlPartColor::DrawBackground()
{
	m_DC.Lock();

	m_DC.SetBrush(m_ColorBlank);
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
	
	m_DC.UnLock();
}

void CSliderCtrlPartColor::DrawRange()
{
	CGuard guard(m_Mutex);
	
	m_DC.Lock();
	
	m_DC.UnLock();
}

//private:
//进度条(m_iSliderWidth)等分10份，除去两头，就有9个分割线
void CSliderCtrlPartColor::DrawSplitLine()
{
	float f_interval = (float)m_iSliderWidth/m_iSplitLineNum;
	int i_pos = 0;
	int i = 0;
	CGuard guard(m_Mutex);
	
	m_DC.Lock();

	m_DC.SetBrush(m_ColorSplitLine);
	m_DC.SetRgnStyle(RS_FLAT);
	
	for (i=1; i<m_iSplitLineNum; ++i)
	{
		i_pos = f_interval * i;//在f_interval为小数时，消除累计误差

		m_DC.Rectangle(CRect(i_pos, 0, i_pos+1, m_Rect.Height()));
	}
	
	m_DC.UnLock();
}

void CSliderCtrlPartColor::UpdateTracker()
{
	CGuard guard(m_Mutex);
	newpos = curpos;
	
	tracker_offset = (m_iSliderWidth - tracker_width) * (curpos- m_sDispRange.start) / m_sDispRange.Width();
}

void CSliderCtrlPartColor::DrawTracker()
{
	CGuard guard(m_Mutex);
	m_DC.Lock();
	
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(tracker_offset, 0, tracker_offset + tracker_width, m_Rect.Height()));
	
	m_DC.UnLock();
}


VD_BOOL CSliderCtrlPartColor::MsgProc(uint msg, uint wpa, uint lpa)
{
	int px, py;
	int vtracker_offset;
	int temp;

	newpos = curpos;
	switch (msg)
	{
		case XM_LBUTTONDOWN:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		temp = GetAt(px, py);
		if(temp == CSliderCtrl::SA_POSITION){
			newpos = m_sDispRange.start + (m_sDispRange.Width()+ 1) * (px - tracker_width/2 - m_Rect.left) / MAX(1, (m_iSliderWidth - tracker_width));
			if(newpos < m_sDispRange.start){
				newpos = m_sDispRange.start;
			}else if(newpos > m_sDispRange.end){
				newpos = m_sDispRange.end;
			}
		}else if(temp == CSliderCtrl::SA_TRACK){
			track = TRUE;
			SetFlag(IF_CAPTURED, TRUE);
			tracker_pick = px - m_Rect.left - tracker_offset;
		}else{
			return FALSE;
		}
		break;

	case XM_LBUTTONUP://拖动游标结束时
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(track)
		{			
			newpos = m_sDispRange.start + m_sDispRange.Width() * tracker_offset / MAX(1, (m_iSliderWidth - tracker_width));
			
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
			
			if(vtracker_offset != tracker_offset)
			{
				tracker_offset = vtracker_offset;
				
				newpos = m_sDispRange.start+ m_sDispRange.Width() * tracker_offset / MAX(1, (m_iSliderWidth - tracker_width));
				
				Draw();
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
		Draw();
	}

	return TRUE;
}

