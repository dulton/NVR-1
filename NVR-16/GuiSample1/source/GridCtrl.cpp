//
//  "$Id: GridCtrl.cpp 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#include "GUI/Ctrls/Page.h"

///////////////////////////////////////////////////////////////////
////////////        CGridCtrl


CGridCtrl::CGridCtrl(VD_PCRECT lpRect, CPage * pParent, int vcolumn, int vrow, CTRLPROC ondblClicked, uint style)
:CItem(lpRect, pParent, IT_GRIDCTRL,style|styleEditable|styleAutoFocus)
{
	SetGrid(vrow, vcolumn);
	m_ondbclk = ondblClicked;
	row_start = 0;
	column_start = 0;
	row_end = 0;
	column_end = 0;
	row_start_old = 0;
	column_start_old = 0;
	row_end_old = 0;
	column_end_old = 0;
	m_ColorSelected = VD_RGB(255,128,128);
	m_ColorEmpty = m_pDevGraphics->GetKeyColor();
}

CGridCtrl::~CGridCtrl()
{
}


CGridCtrl* CreateGridCtrl(VD_PCRECT pRect, CPage * pParent, int vcolumn,int vrow,CTRLPROC ondblClicked,uint style /* =0 */)
{
	return new CGridCtrl(pRect, pParent, vcolumn, vrow, ondblClicked, style);
}


void CGridCtrl::Draw()
{
	if(!DrawCheck()){
		return;
	}
	row_end = row_start;
	column_end = column_start;
	inset = FALSE;
	int r;
	int c;
	m_DC.Lock();
	m_DC.SetPen(VD_GetSysColor(COLOR_FRAME), VD_PS_SOLID, 2);

	//注意画线的方向
	for (r = 0; r <= row; r++)
	{
		m_DC.MoveTo(m_Rect.Width() - 1,  r * m_Rect.Height() / row - 1);
		m_DC.LineTo(0, r * m_Rect.Height() / row - 1);
	}
	for (c = 0; c <= column; c++)
	{
		m_DC.MoveTo(c * m_Rect.Width() / column - 1, 0);
		m_DC.LineTo(c * m_Rect.Width() / column - 1, m_Rect.Height() - 1);
	}

	//下边界和右边界
	m_DC.MoveTo(m_Rect.Width() - 1, 0);
	m_DC.LineTo(m_Rect.Width() - 1, m_Rect.Height() - 1);
	m_DC.LineTo(0, m_Rect.Height() - 1);
	m_DC.UnLock();

	DrawDynamic(TRUE);
}

void CGridCtrl::DrawDynamic(VD_BOOL all)
{
	if(!DrawCheck()){
		return;
	}
	int r;
	int c;
	int rmax;
	int rmin;
	int cmax;
	int cmin;
	int height = m_Rect.Height();
	int width = m_Rect.Width();

	//只画修改块
	m_DC.Lock();
	for(r = 0; r < row; r++){
		for(c = 0; c < column; c++){
			if(all || (region[r] & BITMSK(c))!=(backup[r] & BITMSK(c))){
				if(region[r] & BITMSK(c)){
					m_DC.SetBrush(m_ColorSelected);
				}else{
					m_DC.SetBrush(m_ColorEmpty);
				}
				m_DC.SetRgnStyle(RS_FLAT);
				m_DC.Rectangle(CRect(c*width / column+1,r*height / row+1,(c+1)*width / column-1,(r+1)*height / row-1));
			}
		}
	}

	//画修改块的边框
	if(all
		||row_start_old != row_start || column_start_old != column_start
		||row_end_old != row_end || column_end_old != column_end
		||(region[row_end] & BITMSK(column_start))!=(backup[row_end] & BITMSK(column_start))){
		rmax = MAX(row_start_old,row_end_old);
		rmin = MIN(row_start_old,row_end_old);
		cmax = MAX(column_start_old,column_end_old);
		cmin = MIN(column_start_old,column_end_old);
		m_DC.SetPen(VD_GetSysColor(COLOR_FRAME), VD_PS_SOLID, 2);
		m_DC.SetRgnStyle(RS_HOLLOW);
		//m_DC.Rectangle(CRect(cmin*width / column - 1, rmin*height / row - 1,
		//	(cmax + 1)*width / column + 1, (rmax + 1)*height / row + 1));
		m_DC.Rectangle(CRect(cmin*width / column, rmin*height,
			(cmax + 1)*width / column - 1, (rmax + 1)*height / row - 1));
		rmax = MAX(row_start,row_end);
		rmin = MIN(row_start,row_end);
		cmax = MAX(column_start,column_end);
		cmin = MIN(column_start,column_end);

		m_DC.SetPen(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), VD_PS_SOLID, 2);
		m_DC.Rectangle(CRect(cmin*width / column, rmin*height,
			(cmax + 1)*width / column - 1, (rmax + 1)*height / row - 1));
	}
	row_start_old = row_start;
	column_start_old = column_start;
	row_end_old = row_end;
	column_end_old = column_end;
	m_DC.UnLock();

	GetMask(backup);
}
//根据坐标的到块的序号
VD_BOOL CGridCtrl::GetAt(int px, int py, int* pc, int* pr)
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

VD_BOOL CGridCtrl::MsgProc(uint msg, uint wpa, uint lpa)
{
	int r;
	int c;
	int key;
	int px, py;
	switch(msg) {
	case XM_KEYDOWN:
		key = wpa;
		switch(key){
		case KEY_RET:
		case KEY_FUNC:
			if(!inset){
				//切换当前块的状态
				region[row_start] ^= BITMSK(column_start);
				inset = TRUE;
			}else{
				row_start = row_end;
				column_start = column_end;
				inset = FALSE;
			}
			break;
		case KEY_UP:
			/*
			if(inset){
				if(row_end > 0)
					row_end--;
			}else{
				if(row_start > 0){
					row_start--;
					row_end = row_start;
				}
			}
			break;
			*/
			return FALSE;
		case KEY_DOWN:
			/*
			if(inset){
				if(row_end < row-1)
					row_end++;
			}else{
				if(row_start < row-1){
					row_start++;
					row_end = row_start;
				}
			}
			break;
			*/
			return FALSE;
		case KEY_LEFT:
			if(inset){
				if(column_end > 0)
					column_end--;
			}else{
				if(column_start > 0){
					column_start--;
					column_end = column_start;
				}
			}
			break;
		case KEY_RIGHT:
			if(inset){
				if(column_end < column-1)
					column_end++;
			}else{
				if(column_start < column-1){
					column_start++;
					column_end = column_start;
				}
			}
			break;
		default:
			return FALSE;
		}
		break;

	case XM_LBUTTONDOWN:
	case XM_LBUTTONDBLCLK:  //add by cj@2000503
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(!GetFlag(IF_CAPTURED))
		{
			SetFlag(IF_CAPTURED, TRUE);
			if(GetAt(px, py, &column_start, &row_start)){
				//转换当前块的状态
				row_end = row_start;
				column_end = column_start;
				region[row_start] ^= BITMSK(column_start);
				inset = TRUE;
			}
		}

		if (XM_LBUTTONDBLCLK == msg) //add by cj@2000503
		{
			ItemProc(m_ondbclk);
		}
		break;

	case XM_LBUTTONUP:
		if(GetFlag(IF_CAPTURED))
		{
			SetFlag(IF_CAPTURED, FALSE);
			row_start = row_end;
			column_start = column_end;
			inset = FALSE;
		}
		break;

	case XM_MOUSEMOVE:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(inset){
			GetAt(px, py, &column_end, &row_end);
		}
		break;

	default:
		return FALSE;
	}

	VD_BOOL flag = region[row_start] & BITMSK(column_start);
	
	int rmax = MAX(row_start,row_end);
	int rmin = MIN(row_start,row_end);
	int cmax = MAX(column_start,column_end);
	int cmin = MIN(column_start,column_end);
	for(r = rmin; r <= rmax; r++){
		for(c = cmin; c <= cmax; c++){
			if(flag){
				region[r] |= BITMSK(c);
			}else{
				region[r] &= ~BITMSK(c);
			}
		}
	}
//	printf("&&&&&&&&&&&&&&&^^^^^^%u,%u,%u,%u\n",region[0],region[1],region[2],region[3]);
//	printf("111Gridcw^^^^^^^!!!%s,%d,%d,%d,%d,%d,%d,%d\n",__func__,__LINE__,row,column,row_end,row_start,column_end,column_start);//cw_test
//	printf("222Gridcw^^^^^^^!!!%s,%d,%d,%d,%d,%d,%d,%d\n",__func__,__LINE__,row,column,rmax,rmin,cmax,cmin);//cw_test
	DrawDynamic();
	return TRUE;
}

void CGridCtrl::SetMask(uint* p)
{
	memcpy(region, p, sizeof(uint)*row/*18*/);
	Draw();
}

void CGridCtrl::GetMask(uint* p)
{
	memcpy(p, region, sizeof(uint)*row/*18*/);
}

void CGridCtrl::SetGrid(int vcolumn, int vrow)
{
	row = MIN(vrow,18);
	column = MIN(vcolumn,32);
}

void CGridCtrl::SetColor(VD_COLORREF selected, VD_COLORREF empty)
{
	m_ColorSelected = selected;
	m_ColorEmpty = empty;
}

void CGridCtrl::Select(VD_BOOL flag)
{
	SetFlag(IF_SELECTED, flag);
}
