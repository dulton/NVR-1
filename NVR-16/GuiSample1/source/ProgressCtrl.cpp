//
//  "$Id: ProgressCtrl.cpp 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//  Description:	 
//  Revisions: 	 Year-Month-Day  SVN-Author  Modification
//

#include "GUI/Ctrls/Page.h"

///////////////////////////////////////////////////////////////////
////////////        CProgressCtrl
CProgressCtrl::CProgressCtrl(VD_PCRECT pRect, CPage * pParent,int vmin,int vmax,uint style)
:CItem(pRect, pParent, IT_PROGRESSCTRL,style|styleEditable|styleAutoFocus)//, FALSE)
{
	min = vmin;
	max = vmax;
	curpos = 0;
}

CProgressCtrl::~CProgressCtrl()
{
}

CProgressCtrl* CreateProgressCtrl(VD_PCRECT pRect, CPage * pParent,int vmin /* = 0 */,int vmax /* = 100 */,uint style /* = 0 */)
{
	return new CProgressCtrl(pRect,pParent,vmin,vmax,style);
}
void CProgressCtrl::Draw()
{
	if(!DrawCheck()){
		return;
	}

	m_DC.Lock();
	m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
	m_DC.SetRgnStyle(RS_HOLLOW);
	m_DC.SetPen(VD_GetSysColor(COLOR_FRAME),VD_PS_SOLID,2);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()), 3, 3);
	if(curpos>min){
		m_DC.SetPen(VD_GetSysColor(COLOR_CTRLTEXT),VD_PS_SOLID,2);
		m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXT));
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(4,4,(m_Rect.Width()-8)*(curpos-min)/(max-min)+4,m_Rect.Height()-4), 1, 1);
	}
	m_DC.UnLock();
}
void CProgressCtrl::SetPos(int pos)
{
	if(pos < min || pos > max)
		return;
	curpos = pos;
	Draw();
}

VD_SIZE& CProgressCtrl::GetDefaultSize(void)
{
	static VD_SIZE size ={TEXT_WIDTH*10,CTRL_HEIGHT};
	return size;
}

int CProgressCtrl::GetPos(){
	return curpos;
}
void CProgressCtrl::StepIt(int step)
{
	curpos += step;
	if(curpos>max)
		curpos = max;
	else if(curpos<min)
		curpos = min;
	Draw();
}

void CProgressCtrl::SetRange(int vmin, int vmax)
{
	if (vmin > vmax)
	{
		return;
	}
	
	min = vmin;
	max = vmax;

	if (curpos < min)
	{
		curpos = min;
	}
	if (curpos >max)
	{
		curpos = max;
	}
}

