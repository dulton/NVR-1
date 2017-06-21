//
//  "$Id: NumberPad.cpp 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//


#include "GUI/Ctrls/Page.h"

VD_BITMAP* CNumberPad::m_bmpPanel = NULL;
VD_BITMAP* CNumberPad::m_bmpBtnNormal = NULL;
VD_BITMAP* CNumberPad::m_bmpBtnSelected = NULL;
VD_BITMAP* CNumberPad::m_bmpBtnPushed = NULL;
VD_BITMAP* CNumberPad::m_bmpBtnDisabled = NULL;

///////////////////////////////////////////////////////////////////
////////////        CNumberPad

VD_SIZE pad_array[] =      {{3, 4}, {7, 4}, {7, 4}, {3, 4}};       //各类型的版面大小(数字、大写字母、小写字母)
VD_SIZE pad_size[] =      {{TEXT_WIDTH, TEXT_HEIGHT}, {TEXT_WIDTH, TEXT_HEIGHT}, {TEXT_WIDTH, TEXT_HEIGHT}, {TEXT_WIDTH * 3 / 2, TEXT_HEIGHT}};       //各类型的版面大小(数字、大写字母、小写字母)

char* numpad_signs[IPT_TOTAL][MAX_PAD_ITEMS] =
{
	{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", " ", "\b"},
	{"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", " ", "U", "V", "W", "X", "Y", "Z", "\b"},
	{"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", " ", "u", "v", "w", "x", "y", "z", "\b"},
	{"/", ":", ".", "?", "-", "_", "@", "#", "%", "&", " ", "\b"},
};

//静态获取各种写字板的区域大小
void CNumberPad::GetPadSize(INPUT_PAD_TYPE iType, VD_PSIZE pSize)
{
	pSize->h = pad_array[iType].h * pad_size[iType].h + 6;	
	pSize->w = pad_array[iType].w * pad_size[iType].w + 6;	
}
 
CNumberPad* CreatNumberPad(VD_PCRECT pRect, CPage * pParent, CItem *pOwner /* = NULL */, INPUT_PAD_TYPE iType /* = IPT_NUMBER */,uint style /* = 0 */)
{
	return new CNumberPad(pRect,pParent,pOwner,iType,style);
}

CNumberPad::CNumberPad(VD_PCRECT pRect, CPage * pParent, CItem *pOwner /* = NULL */, INPUT_PAD_TYPE iType /*= IPT_NUMBER*/,uint style)
	:CItem(pRect, pParent, IT_NUMBERPAD,style|stylePopup|styleEditable|styleAutoFocus)//, TRUE, TRUE)
{	
	m_Cursel = -1;
	m_pOwner = pOwner;
	m_PadType = iType;
	if(m_PadType >= IPT_TOTAL)
	{
		m_PadType = IPT_NUMBER;
	}
	m_MaxItems = pad_array[m_PadType].w * pad_array[m_PadType].h;

	//确定各项的区域
	for(int i = 0; i < pad_array[m_PadType].h; i++)
	{
		for(int j = 0; j < pad_array[m_PadType].w; j++ )
		{
			m_ButtonRects[i * pad_array[m_PadType].w + j].left = j * pad_size[m_PadType].w + 4;
			m_ButtonRects[i * pad_array[m_PadType].w + j].top = i * pad_size[m_PadType].h + 4;
			m_ButtonRects[i * pad_array[m_PadType].w + j].right = (j + 1) * pad_size[m_PadType].w + 2;
			m_ButtonRects[i * pad_array[m_PadType].w + j].bottom = (i + 1) * pad_size[m_PadType].h + 2;
		}
	}
}

CNumberPad::~CNumberPad()
{
}

void CNumberPad::Draw()
{
	if(!DrawCheck())
	{
		return;
	}

	m_DC.Lock();
	if (m_bmpPanel) 
	{
		m_DC.WideBitmap(CRect(0, 0, m_Rect.Width(), m_Rect.Height()), m_bmpPanel);
	}
	else
	{
		m_DC.SetBrush(VD_GetSysColor(COLOR_POPUP));		
		m_DC.SetPen(VD_GetSysColor(COLOR_FRAMESELECTED), VD_PS_SOLID, 2);
		m_DC.SetRgnStyle(RS_NORMAL);
		m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()), 3, 3);
	}

	m_DC.UnLock();

	for(int i = 0; i < m_MaxItems; i++)
	{
		DrawSingle(i);
	}
}

void CNumberPad::DrawSingle(int index)
{
	char ch;
	int w, h; //程序绘制的标志的大小

	if(!DrawCheck() || index < 0 || index > m_MaxItems)
	{
		return;
	}

	m_DC.Lock();

	m_DC.SetBrush(VD_GetSysColor(COLOR_POPUP));		
	m_DC.SetPen(VD_GetSysColor(COLOR_POPUP), VD_PS_SOLID, 2);
	m_DC.SetRgnStyle(RS_NORMAL);
	m_DC.Rectangle(&m_ButtonRects[index], 1, 1);

	m_DC.SetPen(VD_GetSysColor(COLOR_POPUP));		
	m_DC.SetRgnStyle(RS_HOLLOW|RS_RAISED);
	m_DC.Rectangle(&m_ButtonRects[index], 1, 1);
	if(m_PadType == IPT_MARK && index >= 0 && index <= 9) //画符号的背景数字
	{
		m_DC.SetTextAlign(VD_TA_LEFTTOP);
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTDISABLED));
		m_DC.VD_TextOut(&m_ButtonRects[index], numpad_signs[IPT_NUMBER][index]);
	}
	if(index == m_Cursel)
	{
		m_DC.SetPen(VD_GetSysColor(COLOR_CTRLTEXTSELECTED), VD_PS_SOLID, 2);
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
		m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
	}
	else
	{
		m_DC.SetPen(VD_GetSysColor(COLOR_CTRLTEXT), VD_PS_SOLID, 2);
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXT));
		m_DC.SetBrush(VD_GetSysColor(COLOR_CTRLTEXT));
	}

	m_DC.SetTextAlign(VD_TA_CENTER);
	m_DC.SetBkMode(BM_TRANSPARENT);

	ch = numpad_signs[m_PadType][index][0];

	if(ch == '\b') //clear
	{
		w = h = m_ButtonRects[index].Height() - 8;
		m_DC.MoveTo((m_ButtonRects[index].left + m_ButtonRects[index].right - w) / 2,
			(m_ButtonRects[index].top + m_ButtonRects[index].bottom) / 2);
		m_DC.LineTo((m_ButtonRects[index].left + m_ButtonRects[index].right + w) / 2,
			(m_ButtonRects[index].top + m_ButtonRects[index].bottom) / 2);
		m_DC.MoveTo((m_ButtonRects[index].left + m_ButtonRects[index].right - w) / 2 + 3,
			(m_ButtonRects[index].top + m_ButtonRects[index].bottom) / 2 - 4);
		m_DC.LineTo((m_ButtonRects[index].left + m_ButtonRects[index].right - w) / 2,
			(m_ButtonRects[index].top + m_ButtonRects[index].bottom) / 2 - 1);
		m_DC.MoveTo((m_ButtonRects[index].left + m_ButtonRects[index].right - w) / 2,
			(m_ButtonRects[index].top + m_ButtonRects[index].bottom) / 2);
		m_DC.LineTo((m_ButtonRects[index].left + m_ButtonRects[index].right - w) / 2 + 3,
			(m_ButtonRects[index].top + m_ButtonRects[index].bottom) / 2 + 3);
	}
	else if(ch == ' ') //space
	{
		w = h = m_ButtonRects[index].Height() - 8;
		m_DC.MoveTo((m_ButtonRects[index].left + m_ButtonRects[index].right - w) / 2,
			m_ButtonRects[index].bottom - 10);
		m_DC.LineTo((m_ButtonRects[index].left + m_ButtonRects[index].right - w) / 2,
			m_ButtonRects[index].bottom - 4);
		m_DC.LineTo((m_ButtonRects[index].left + m_ButtonRects[index].right + w) / 2,
			m_ButtonRects[index].bottom - 4);
		m_DC.LineTo((m_ButtonRects[index].left + m_ButtonRects[index].right + w) / 2,
			m_ButtonRects[index].bottom - 10);
	}
	else
	{
		if(m_PadType == IPT_MARK)
		{
			VD_RECT rect = m_ButtonRects[index];
			rect.left += TEXT_WIDTH / 2;
			m_DC.VD_TextOut(&rect, numpad_signs[m_PadType][index]);
		}
		else
		{
			m_DC.VD_TextOut(&m_ButtonRects[index], numpad_signs[m_PadType][index]);
		}
	}
	m_DC.UnLock();
}

int CNumberPad::GetAt(int px, int py)
{
	int i;
	CRect rect;
	
	for(i = 0; i < m_MaxItems; i++)
	{
		rect = m_ButtonRects[i];
		ClientToScreen(rect);
		if(rect.PtInRect(px, py))
		{
			//printf("####  %d  \n",i);
			return i;
		}
	}
	return -1;
}

VD_BOOL CNumberPad::MsgProc(uint msg, uint wpa, uint lpa)
{
	int px, py;
	int temp, newsel;
	INPUT_CHAR input_char;

	CItem *pInputTray = m_pInputTray[m_screen];
	newsel = m_Cursel;
	if(!m_pOwner)
	{
		return FALSE;
	}
	switch(msg)
	{
	case XM_LBUTTONDOWN:
	case XM_LBUTTONDBLCLK:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		temp = GetAt(px, py);
		if(temp >= 0 )
		{
			newsel = temp;
			if(pInputTray)
			{
				input_char.dw = 0;
				input_char.ch[0] = numpad_signs[m_PadType][temp][0];
				if(input_char.ch[0])
				{
					if(m_PadType == IPT_NUMBER && input_char.ch[0] != '\b' && input_char.ch[0] != ' ')
					{
						pInputTray->MsgProc(XM_KEYDOWN, KEY_0 + numpad_signs[m_PadType][temp][0] - '0', 0);//序号转化为数字,发送给数字框
					}
					else
					{
						pInputTray->MsgProc(XM_CHAR, input_char.dw, 0);//序号转化为数字,发送给数字框
					}
				}
			}
		}
		else
		{
			return FALSE;
		}
		break;
		
	case XM_MOUSEMOVE:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		temp = GetAt(px, py);
		newsel = temp;
		break;
	
	default:
		return FALSE;
	}
	//if m_Cursel changed
	if(newsel != m_Cursel)
	{
		temp = m_Cursel;
		m_Cursel = newsel;
		DrawSingle(m_Cursel);
		DrawSingle(temp);
	}
	return TRUE;
}
void CNumberPad::SetDefaultBitmap(NumberPadBitmap region, VD_PCSTR name)
{
	switch(region) 
	{
	case NPB_PANEL:
		m_bmpPanel = VD_LoadBitmap(name);
		break;
	case NPB_BUTTON_NORMAL:
		m_bmpBtnNormal= VD_LoadBitmap(name);
		break;
	case NPB_BUTTON_PUSHED:
		m_bmpBtnPushed = VD_LoadBitmap(name);
		break;
	case NPB_BUTTON_SELECTED:
		m_bmpBtnSelected = VD_LoadBitmap(name);
		break;
	case NPB_BUTTON_DISABLED:
		m_bmpBtnDisabled = VD_LoadBitmap(name);
		break;
	default:
		break;
	}
}
