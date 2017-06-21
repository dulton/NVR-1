//
//  "$Id: CheckBox.cpp 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#include "GUI/Ctrls/Page.h"


VD_BITMAP* CCheckBox::m_bmpNormalTrue = NULL;
VD_BITMAP* CCheckBox::m_bmpPushedTrue = NULL;
VD_BITMAP* CCheckBox::m_bmpSelectedTrue = NULL;
VD_BITMAP* CCheckBox::m_bmpDisabledTrue = NULL;
VD_BITMAP* CCheckBox::m_bmpNormalFalse = NULL;
VD_BITMAP* CCheckBox::m_bmpSelectedFalse = NULL;
VD_BITMAP* CCheckBox::m_bmpPushedFalse = NULL;
VD_BITMAP* CCheckBox::m_bmpDisabledFalse = NULL;


CCheckBox::CCheckBox(VD_PCRECT pRect, CPage * pParent,uint style, CTRLPROC vproc, int vindex)
:CItem(pRect, pParent, IT_CHECKBOX,style|styleAutoFocus)
{
	m_vProc = vproc;
	index = vindex;
	if(m_bmpNormalFalse){
		int cw = (m_bmpNormalFalse->width - m_Rect.Width());
		int ch = (m_bmpNormalFalse->height - m_Rect.Height());
		SetRect(CRect(m_Rect.left, m_Rect.top, m_Rect.right+cw, m_Rect.bottom+ch), FALSE);
	}
	value = 0;
}

CCheckBox::~CCheckBox()
{

}

VD_SIZE& CCheckBox::GetDefaultSize(void)
{
	static VD_SIZE size = {CTRL_HEIGHT1,CTRL_HEIGHT1};
	return size;
}
CCheckBox* CreateCheckBox(VD_PCRECT pRect, CPage * pParent,uint style, CTRLPROC vproc, int vindex)
{
	return new CCheckBox(pRect, pParent,style, vproc, vindex);
}
void CCheckBox::Draw()
{
	if(!DrawCheck()){
		return;
	}
	m_DC.Lock();


	if (m_bmpNormalTrue && m_bmpNormalFalse && m_bmpSelectedFalse && m_bmpSelectedTrue)
	{
		m_DC.SetBkMode(BM_TRANSPARENT);

		if(GetFlag(IF_SELECTED)){
			if(value)
				m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_bmpSelectedTrue);
			else
				m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_bmpSelectedFalse);
		}else{
			if(value)
				m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_bmpNormalTrue);
			else
				m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_bmpNormalFalse);
		}
		
		if(index >= 0)
		{
			m_DC.SetTextAlign(VD_TA_CENTER);
			if (value)
			{
				m_DC.SetFont(VD_GetSysColor(COLOR_FRAME));
			}
			else
			{
				if (GetFlag(IF_SELECTED))
				{
					m_DC.SetFont(VD_GetSysColor(COLOR_FRAMESELECTED));	
				}
				else
				{
					m_DC.SetFont(VD_GetSysColor(COLOR_FRAME));
				}
			}
			m_DC.SetBkMode(BM_TRANSPARENT);
			char index_str[8];
			sprintf(index_str, "%d", index);
			m_DC.VD_TextOut(CRect(0,0,m_Rect.Width(),m_Rect.Height()), index_str);
		}
		else
		{
			if (GetFlag(IF_SELECTED))
			{
				m_DC.SetFont(VD_GetSysColor(COLOR_FRAMESELECTED));	
			}
			else
			{
				m_DC.SetFont(VD_GetSysColor(COLOR_FRAME));
			}
			m_DC.SetBkMode(BM_TRANSPARENT);
			m_DC.VD_TextOut(CRect(0,0,m_Rect.Width(),m_Rect.Height()), string.c_str());
		}
		m_DC.UnLock();
		return ;
	}


	m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));

	if(index < 0)
	{
		if(string.size())
		{
			m_DC.SetFont(VD_GetSysColor(COLOR_FRAME));
			if(!GetFlag(IF_SELECTED))
			{
				m_DC.SetPen(VD_GetSysColor(COLOR_FRAME), VD_PS_SOLID, 2);
			}
			else
			{
				m_DC.SetPen(VD_GetSysColor(COLOR_FRAMESELECTED), VD_PS_SOLID, 2);
				m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXT));
			}
			if(value)
			{
				if(!GetFlag(IF_SELECTED))
				{
					m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXT));
				}
				else
				{
					m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
				}
				m_DC.SetRgnStyle(RS_NORMAL);
			}
			else
			{
				if(GetFlag(IF_SELECTED))
				{
					m_DC.SetFont(VD_GetSysColor(COLOR_FRAMESELECTED));
				}
				m_DC.SetRgnStyle(RS_HOLLOW);
			}
			if(!GetFlag(IF_ENABLED))
			{
				m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXTDISABLED));
			}
			m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()),3,3);
			m_DC.SetTextAlign(VD_TA_CENTER);
			m_DC.VD_TextOut(CRect(0,0,m_Rect.Width(),m_Rect.Height()), string.c_str());
		}
		else
		{
			if(!GetFlag(IF_SELECTED))
			{
				m_DC.SetPen(VD_GetSysColor(COLOR_FRAME), VD_PS_SOLID, 2);
				if(value)
				{
					m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXT));
				}
				else
				{
					m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
				}
			}
			else
			{
				m_DC.SetPen(VD_GetSysColor(COLOR_FRAMESELECTED),VD_PS_SOLID,2);
				if(value)
				{
					m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
				}
				else
				{
					m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
				}
			}
			if(!GetFlag(IF_ENABLED))
			{
				if(value)
				{
					m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXTDISABLED));
				}
			}
			m_DC.SetRgnStyle(RS_NORMAL);
			m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()),3,3);
		}
	}
	else
	{
		m_DC.SetFont(VD_GetSysColor(COLOR_FRAME));
		if(!GetFlag(IF_SELECTED))
		{
			m_DC.SetPen(VD_GetSysColor(COLOR_FRAME), VD_PS_SOLID, 2);
		}
		else
		{
			m_DC.SetPen(VD_GetSysColor(COLOR_FRAMESELECTED), VD_PS_SOLID, 2);
			m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXT));
		}
		if(value)
		{
			if(!GetFlag(IF_SELECTED))
			{
				m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXT));
			}
			else
			{
				m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
			}
			m_DC.SetRgnStyle(RS_NORMAL);
		}
		else
		{
			if(GetFlag(IF_SELECTED))
			{
				m_DC.SetFont(VD_GetSysColor(COLOR_FRAMESELECTED));
			}
			m_DC.SetRgnStyle(RS_HOLLOW);
		}
		if(!GetFlag(IF_ENABLED))
		{
			m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXTDISABLED));
		}
		m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()),3,3);
		m_DC.SetTextAlign(VD_TA_CENTER);
		char index_str[8];
		sprintf(index_str, "%d", index);
		m_DC.VD_TextOut(CRect(0,0,m_Rect.Width(),m_Rect.Height()), index_str);
	}
	m_DC.UnLock();
}

VD_BOOL CCheckBox::MsgProc(uint msg, uint wpa, uint lpa)
{
	int key;
	signed char delta;
	switch(msg){
	case XM_KEYDOWN:
		key = wpa;
		switch(key){

//		case KEY_UP://cw_tab
//		case KEY_DOWN:
		case KEY_RET:
			if(value)
				value = 0;
			else
				value = 1;
			break;
		default:
			return FALSE;
		}
		break;

	case XM_MOUSEWHEEL:
		delta = (char)VD_HIWORD(wpa);
		if(delta != 0){
			if(value)
				value = 0;
			else
				value = 1;
		}
		break;

	case XM_LBUTTONDOWN:
		if(value)
			value = 0;
		else
			value = 1;
		break;

	default:
		return FALSE;
	}

	ItemProc(m_vProc);
	Draw();
	return TRUE;
}

void CCheckBox::SetValue(int vvalue)
{
	if(vvalue)
		vvalue = TRUE;
	if(value == vvalue)
		return;
	value = vvalue;
	Draw();
}

int CCheckBox::GetValue()
{
	return value;
}

void CCheckBox::SetText(VD_PCSTR vstring)
{
	if (index != -1)
	{
		printf("error in construct or call this function index != -1 duplicate width vstring != NULL!\n");
	}
	if (!vstring)
	{
		printf("CCheckBox::SetText invalid string!\n");
		return;
	}
	string = vstring;
	Draw();

}
VD_PCSTR CCheckBox::GetText()
{
		if(index>0)
			{
				char index_str[8];
				sprintf(index_str, "%d", index);
				string = index_str;
			}
		return string.c_str();
}

void CCheckBox::SetDefaultBitmap(CheckBoxBitmap region, VD_PCSTR name)
{
	switch(region)
	{
	case CKB_NORMAL_TRUE:
		m_bmpNormalTrue = VD_LoadBitmap(name);
		break;
	case CKB_PUSHED_TRUE:
		m_bmpPushedTrue = VD_LoadBitmap(name);
		break;
	case CKB_SELECTED_TRUE:
		m_bmpSelectedTrue = VD_LoadBitmap(name);
		break;
	case CKB_DISABLED_TRUE:
		m_bmpDisabledTrue = VD_LoadBitmap(name);
		break;
	case CKB_NORMAL_FALSE:
		m_bmpNormalFalse = VD_LoadBitmap(name);
		break;
	case CKB_PUSHED_FALSE:
		m_bmpPushedFalse = VD_LoadBitmap(name);
		break;
	case CKB_SELECTED_FALSE:
		m_bmpSelectedFalse = VD_LoadBitmap(name);
		break;
	case CKB_DISABLED_FALSE:
		m_bmpDisabledFalse = VD_LoadBitmap(name);
		break;
	default:
		break;
	}
}

