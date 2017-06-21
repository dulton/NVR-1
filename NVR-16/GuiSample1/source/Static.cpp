//
//  "$Id: Static.cpp 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#include "GUI/Ctrls/Page.h"
extern CStatic *pTest;
#include "Devices/DevGraphics.h"

///////////////////////////////////////////////////////////////////
////////////        CStatic

CStatic::CStatic(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz, VD_BOOL title /* = TRUE */, uint style /* = 0 */, FONT_STYLE TextFont /*= FS_NORMAL*/)
:CItem(pRect, pParent, IT_STATIC,style|styleAutoFocus)
{
	if(psz)
	{
		string = GetParsedString(psz);
	}
	bmp = NULL;

	m_bTitle = title;
	m_TextFont = TextFont;

	if(m_bTitle)
	{
		m_TextColor = VD_GetSysColor(VD_COLOR_WINDOWTEXT);
	}
	else
	{
		m_TextColor = VD_GetSysColor(COLOR_CTRLTEXT);
	}
	
	m_BkWidth = -1;
	
	m_bTextAlign = m_Rect.Height()>(CTRL_HEIGHT+TEXT_HEIGHT)?VD_TA_CHANGELINE:VD_TA_YCENTER;
	
	//m_BkColor  = VD_RGB(21,75,117);
	m_BkColor = VD_GetSysColor(VD_COLOR_WINDOW);
	if(m_dwStyle & staticEnclosed)
	{
		m_TextColor = VD_GetSysColor(COLOR_CTRLTEXT);
		m_EnclosedColor = VD_GetSysColor(COLOR_CTRL);
	}
	SetRectEmpty(&rect_normal);
}

CStatic::CStatic(VD_PCRECT pRect, CPage * pParent,VD_BITMAP* buf, uint style /* = 0 */):CItem(pRect, pParent, IT_STATIC,style)//, FALSE)
{
	bmp = buf;

	if(!(m_dwStyle & staticTransparent))
	{
		ReplaceBitmapBits(bmp, VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	}
	::SetRect(&rect_normal, 0, 0, m_Rect.Width(), m_Rect.Height());
	string.erase();
	

}
CStatic* CreateStatic(VD_PCRECT pRect, CPage * pParent,VD_PCSTR psz,VD_BOOL title,  uint style, FONT_STYLE TextFont)
{
	return new CStatic(pRect,pParent,psz,title,style,TextFont);
}

CStatic* CreateStatic(VD_PCRECT pRect, CPage * pParent,VD_BITMAP* buf, uint style)
{
	return new CStatic(pRect,pParent, buf, style);
}

VD_SIZE& CStatic::GetDefaultSize(void)
{
	static VD_SIZE size = {TEXT_WIDTH,TEXT_HEIGHT};
	return size;
}
CStatic::~CStatic()
{
}

void CStatic::AutoSetRect()
{

	::SetRect(&rect_normal, 0, 0, m_Rect.Width(), m_Rect.Height());
	return ;
}

void CStatic::Draw()
{
	if(!DrawCheck()){
		return;
	}
	
	//画背景不能放在DC锁内
	//if((m_dwStyle & staticTransparent) && (!bmp || !(m_dwStyle & staticSelfBk))) 
	if(((m_dwStyle & staticTransparent) && (!bmp || !(m_dwStyle & staticSelfBk))) 
		&& !(m_dwStyle & staticCrochet)) //add langzi 2009-12-3
	{
		DrawBackground();
	}
	
	m_DC.Lock();

	//bmp
	if(bmp)
	{
		if(m_dwStyle & staticSelfBk)
		{
			m_DC.SetBrush(m_BkColor);
			m_DC.SetRgnStyle(RS_FLAT);
			m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
		}

		if(m_dwStyle & staticTransparent)
		{
			m_DC.SetBkMode(BM_TRANSPARENT);
		}
		else
		{
			m_DC.SetBkMode(BM_NORMAL);
		}
		m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),bmp,rect_normal.left, rect_normal.top);
		m_DC.UnLock();
		return;
	}

	//自动判断字符串所需要的rect，主要用于解决在自动布局使用中出现的字符串截断现象。
	//创建的时候如果m_Rect为NULL、String也为NULL（程序后面会将string改写）的时候给static带来的字符串截断问题 
	//--by wangqin  20070718
	if (m_dwStyle & staticAutoRect)
	{
		if (string.c_str())
		{
			int width = g_Locales.GetTextExtent(string.c_str());
			//printf("strlen %d,rect width %d str = %s\n",width, m_Rect.Width(),string.c_str());
			if (width > m_Rect.Width())
			{
				VD_RECT rect(m_Rect);
				rect.right = m_Rect.right + (width - m_Rect.Width());
				SetRect(&rect);
			}
		}
	}
	if(!(m_dwStyle & staticTransparent))
	{
		m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
		//m_DC.SetBrush(VD_RGB(21,75,117));
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
		
		m_DC.SetBrush(m_BkColor);
		m_DC.SetRgnStyle(RS_FLAT);
		//m_DC.Rectangle(CRect(0,0,m_BkWidth,m_Rect.Height()));
		//m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));

		if(-1 == m_BkWidth)
		{
			m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
		}
		else
		{
			m_DC.Rectangle(CRect(0,0,m_BkWidth,m_Rect.Height()));
		}
	}
	if(string.c_str())
	{
	    //m_DC.SetBkMode(BM_TRANSPARENT); //delete langzi 2009-12-3
	    //add langzi 2009-12-3 增加设置钩边模式
	    if(m_dwStyle & staticCrochet)
	    {
	      m_DC.SetBkMode(BM_CROCHET);
	    }
		else
		{
		  m_DC.SetBkMode(BM_TRANSPARENT);
		}
		//end langzi
		m_DC.SetTextAlign(m_bTextAlign);
		if(m_dwStyle & staticEnclosed)
		{
			//m_DC.SetFont(m_EnclosedColor, FS_BOLD); 
			m_DC.SetFont(m_EnclosedColor, FS_NORMAL); //add langzi
			m_DC.VD_TextOut(CRect(-1,0,m_Rect.Width() -1,m_Rect.Height()),string.c_str());
			m_DC.VD_TextOut(CRect(1,0,m_Rect.Width()+1,m_Rect.Height()),string.c_str());
			m_DC.VD_TextOut(CRect(0, -1,m_Rect.Width(),m_Rect.Height() - 1),string.c_str());
			m_DC.VD_TextOut(CRect(0, 1,m_Rect.Width(),m_Rect.Height() + 1),string.c_str());
			m_DC.SetFont(m_TextColor, m_TextFont);
			m_DC.VD_TextOut(CRect(0,0,m_Rect.Width(),m_Rect.Height()),string.c_str());
		}
		else
		{
			if(GetFlag(IF_ENABLED))
			{
				m_DC.SetFont(m_TextColor, m_TextFont);
			}
			else
			{
				m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTDISABLED), m_TextFont);
			}
			m_DC.VD_TextOut(CRect(0,2,m_Rect.Width(),m_Rect.Height()+2),string.c_str());
		}
	}
	m_DC.UnLock();
}

void CStatic::SetText(VD_PCSTR psz)
{
	if(psz)
	{
		string = GetParsedString(psz);
	}
	else
	{
		string.erase();
	}
	Draw();
}

void CStatic::SetBitmap(VD_BITMAP* buf, VD_PCRECT pRectNormal)
{
/*	if (!buf) {
		return;
	}
*/	m_dwStyle |= staticTransparent;
	bmp = buf;
	if(pRectNormal)
	{
		rect_normal = *pRectNormal;
	}

	Draw();
}

void CStatic::SetTextColor(VD_COLORREF color, VD_BOOL redraw /* = FALSE */)
{
	m_TextColor = color;
	if(redraw)
	{
		Draw();
	}
}

void CStatic::SetBkColor(VD_COLORREF color, VD_BOOL redraw /* = FALSE */)
{
	m_BkColor = color;
	if(redraw)
	{
		Draw();
	}
}

void CStatic::SetBkWidth(int bkWidth, VD_BOOL redraw)
{
	m_BkWidth = bkWidth;
	if(redraw)
	{
		Draw();
	}

}

void CStatic::SetColor(int nRegion, VD_COLORREF color, VD_BOOL redraw /* = FALSE */)
{
	switch(nRegion) {
	case STC_TEXT:
		m_TextColor = color;
		break;
	case STC_BK:
		m_BkColor = color;
		break;
	case STC_BORDER:
		m_BorderColor = color;
		break;
	case STC_ENCLOSED:
		m_EnclosedColor = color;
	default:
		break;
	}
	if(redraw)
	{
		Draw();
	}
}

void CStatic::SetTextAlign(int align/* = TA_LEFTTOP*/)
{
	m_bTextAlign = align;
}

VD_PCSTR CStatic::GetText()
{
	return string.c_str();
}

void CStatic::SetTextFont(FONT_STYLE TextFont)
{
	m_TextFont = TextFont;
	Draw();
}
