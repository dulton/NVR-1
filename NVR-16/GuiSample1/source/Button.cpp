//
//  "$Id: Button.cpp 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//  Description:	 
//  Revisions: 	 Year-Month-Day  SVN-Author  Modification
//

#include "GUI/Ctrls/Page.h"

///////////////////////////////////////////////////////////////////
////////////        CButton
//PGROUPITEMQUEUE CButton::m_Queue;

#define CUTPOS 8

VD_BITMAP* CButton::m_bmpDefNormal = NULL;
VD_BITMAP* CButton::m_bmpDefSelected = NULL;
VD_BITMAP* CButton::m_bmpDefDisabled = NULL;
VD_BITMAP* CButton::m_bmpDefPushed = NULL;

#if 0
VD_BITMAP*	bmp_button1;
VD_BITMAP*	bmp_button2;
VD_BITMAP*	bmp_button3;
VD_BITMAP*	bmp_button4;
#endif

CButton* CreateButton(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz, CTRLPROC onClicked, CTRLPROC onChanged , uint style, BOOL bPage)
{
	return new CButton(pRect,pParent,psz,onClicked,onChanged,style,bPage);
}
/*
CButton* CreateButton(PCRECT pRect, CPage * pParent, PCSTR psz, CTRLPROC onClicked,int iGroupID,uint style )
{
	return new CButton(pRect, pParent, psz,onClicked,iGroupID,style);
}
*/
CButton* CreateButton(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,VD_IID vid,uint style, BOOL bPage)
{
	return new CButton(pRect, pParent, psz, vid, style, bPage);
}

CButton* CreateButton(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,CPage *pPage,uint style, BOOL bPage)
{
	return new CButton(pRect, pParent, psz, pPage, style, bPage);
}

CButton::CButton(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,CTRLPROC onClicked, CTRLPROC onChanged/* = NULL*/, uint style, BOOL bPage/* = FALSE*/)
	:CItem(pRect, pParent, IT_BUTTON,style|styleEditable|styleAutoFocus)
{
	Init();
	if(psz)
	{
		string = GetParsedString(psz);
	}
	m_onClick = onClicked;
	m_onChanged = onChanged;
    m_bPage = bPage;

	m_TextColor = VD_RGB(255,255,255);

#if 0
	bmp_button1 = VD_LoadBitmap(DATA_DIR"/button_normal.bmp");
	bmp_button2 = VD_LoadBitmap(DATA_DIR"/button_select.bmp");
	bmp_button3 = VD_LoadBitmap(DATA_DIR"/button_push.bmp");
	bmp_button4 = VD_LoadBitmap(DATA_DIR"/button_normal.bmp");
#endif

	//printf("CButton %x, %x ,%x ,%x \n", bmp_button1, bmp_button2, bmp_button3, bmp_button4);

}

CButton::CButton(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,VD_IID vid,uint style, BOOL bPage)
	:CItem(pRect, pParent, IT_BUTTON,style|styleEditable|styleAutoFocus)
{
	Init();
	if(psz)
	{
		string = GetParsedString(psz);
	}
	id = vid;
}

CButton::CButton(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,CPage *pPage,uint style, BOOL bPage)
	:CItem(pRect, pParent, IT_BUTTON,style|styleEditable|styleAutoFocus)
{
	Init();
	if(psz)
	{
		string = GetParsedString(psz);
	}
	child = pPage;
}


CButton::~CButton()
{
}

void CButton::SetTextColor(VD_COLORREF color, VD_BOOL redraw /* = FALSE */)
{
	m_TextColor = color;
	if(redraw)
	{
		Draw();
	}
}

void CButton::DrawCtrlButton(CTRL_STATE state /* = ctrlStateNormal */)
{
	switch(state)
	{
	case ctrlStateNormal:
		if (m_bmpDefNormal) 
		{
			m_DC.WideBitmap(CRect(0,0,CUTPOS, m_Rect.Height()), m_bmpDefNormal,CRect(0, 0,CUTPOS,m_bmpDefNormal->height));
			m_DC.WideBitmap(CRect(CUTPOS, 0, m_Rect.Width()-CUTPOS,m_Rect.Height()),m_bmpDefNormal,
				CRect(CUTPOS, 0, m_bmpDefNormal->width-CUTPOS,m_bmpDefNormal->height));
			m_DC.WideBitmap(CRect(m_Rect.Width()-CUTPOS,0,m_Rect.Width(),m_Rect.Height()), m_bmpDefNormal,
				CRect(m_bmpDefNormal->width-CUTPOS,0,m_bmpDefNormal->width,m_bmpDefNormal->height));
		}
		break;
	case ctrlStateSelect:
		if (m_bmpDefSelected) 
		{
			m_DC.WideBitmap(CRect(0,0,CUTPOS, m_Rect.Height()), m_bmpDefSelected,CRect(0, 0,CUTPOS,m_bmpDefSelected->height));
			m_DC.WideBitmap(CRect(CUTPOS, 0, m_Rect.Width()-CUTPOS,m_Rect.Height()),m_bmpDefSelected,
				CRect(CUTPOS, 0, m_bmpDefSelected->width-CUTPOS,m_bmpDefSelected->height));
			m_DC.WideBitmap(CRect(m_Rect.Width()-CUTPOS,0,m_Rect.Width(),m_Rect.Height()), m_bmpDefSelected,
				CRect(m_bmpDefSelected->width-CUTPOS,0,m_bmpDefSelected->width,m_bmpDefSelected->height));

		}
	    break;
	case ctrlStatePushed:
		if (m_bmpDefPushed) 
		{
			m_DC.WideBitmap(CRect(0,0,CUTPOS, m_Rect.Height()), m_bmpDefPushed,CRect(0, 0,CUTPOS,m_bmpDefPushed->height));
			m_DC.WideBitmap(CRect(CUTPOS, 0, m_Rect.Width()-CUTPOS,m_Rect.Height()),m_bmpDefPushed,
				CRect(CUTPOS, 0, m_bmpDefPushed->width-CUTPOS,m_bmpDefPushed->height));
			m_DC.WideBitmap(CRect(m_Rect.Width()-CUTPOS,0,m_Rect.Width(),m_Rect.Height()), m_bmpDefPushed,
				CRect(m_bmpDefPushed->width-CUTPOS,0,m_bmpDefPushed->width,m_bmpDefPushed->height));
		}
		break;
	default:
		printf("CButton::DrawButton wrong number%d\n",state);
	    break;
	}

}

VD_SIZE& CButton::GetDefaultSize(void)
{
	static VD_SIZE size = {TEXT_WIDTH * 4 + 13,CTRL_HEIGHT};
	return size;
}


void CButton::Init()
{
	id = ID_EMPTY;
	string.erase();
	m_onClick = NULL;
	m_onChanged = NULL;
	child = NULL;
	m_bDown = FALSE;
	m_pBmpNormal = NULL;
	m_pBmpSelect = NULL;
	m_pBmpDisable = NULL;
	m_pBmpPush = NULL;
	m_bWithBmp = FALSE;
	m_iBkMode = BM_NORMAL;
	
	if((m_bmpDefNormal != NULL)&&(m_bmpDefPushed != NULL) &&(m_bmpDefSelected!= NULL))
	{
		m_dwStyle |= styleUseBmp;
	}
}

		extern void* p_gButtonDbg[];
		
void CButton::Draw()
{
	if(!DrawCheck()){
		return;
	}
	
	int tileWidth = 1;
	
	//画背景不能放在DC锁内
	if(((m_dwStyle&buttonNormalBmp) == buttonNormalBmp)&& m_pParent)
	{
		DrawBackground();
	}
	m_DC.Lock();
	
	//图片
	if((m_dwStyle&buttonNormalBmp) == buttonNormalBmp)
	{
		m_DC.SetBkMode(m_iBkMode);
		if(!GetFlag(IF_ENABLED))
		{
			if ((m_dwStyle&buttonFullBmpRect) == buttonFullBmpRect)
			{
				m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_pBmpNormal,rect_disabled.left, rect_disabled.top);
			}
			else
			{
				if (m_pBmpDisable)
				{
					if (m_pBmpDisable->width < m_Rect.Width() || m_pBmpDisable->height < m_Rect.Height())
					{
						m_DC.WideBitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()), m_pBmpDisable, CRect(tileWidth,0,m_pBmpDisable->width-2*tileWidth,m_pBmpDisable->height));
					} 
					else
					{
						m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_pBmpDisable);
					}
					
				}
			}
		}
		else if(m_bDown)
		{
		
			if ((m_dwStyle&buttonFullBmpRect) == buttonFullBmpRect)
			{				
				m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_pBmpNormal,rect_pushed.left, rect_pushed.top);
			}
			else
			{
				if (m_pBmpPush)
				{
					if (m_pBmpPush->width < m_Rect.Width() || m_pBmpPush->height < m_Rect.Height())
					{
						m_DC.WideBitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()), m_pBmpPush, CRect(tileWidth,0,m_pBmpPush->width-2*tileWidth,m_pBmpPush->height));
					} 
					else
					{
						m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_pBmpPush);
					}
					
				}
			}
		}
		else if(GetFlag(IF_SELECTED))
		{
			if ((m_dwStyle&buttonFullBmpRect) == buttonFullBmpRect)
			{
				m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_pBmpNormal,rect_selected.left, rect_selected.top);
			}
			else
			{
				if (m_pBmpSelect)
				{				
					if (m_pBmpSelect->width < m_Rect.Width() || m_pBmpSelect->height < m_Rect.Height())
					{
						m_DC.WideBitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()), m_pBmpSelect, CRect(tileWidth,0,m_pBmpSelect->width-2*tileWidth,m_pBmpSelect->height));
					} 
					else
					{
						m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_pBmpSelect);
					}
					
				}
			}
		}
		else
		{
			if ((m_dwStyle&buttonFullBmpRect) == buttonFullBmpRect)
			{
				//buttonFullBmpRect和buttonNormalBmp属性设置不一样，否则出错！		2007-04-25		wangqin		
				m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_pBmpNormal,rect_normal.left, rect_normal.top);
			}
			else
			{
				if (m_pBmpNormal)
				{
					if (m_pBmpNormal->width < m_Rect.Width() || m_pBmpNormal->height < m_Rect.Height())
					{
						m_DC.WideBitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()), m_pBmpNormal, CRect(tileWidth,0,m_pBmpNormal->width-2*tileWidth,m_pBmpNormal->height));
					} 
					else
					{
						m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_pBmpNormal);
					}
					
				}
			}
		}
	}
	//非图片
	else
	{
		//背景+边框
		if ((m_dwStyle&styleUseBmp) != styleUseBmp)
		{
			m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
			m_DC.SetRgnStyle(RS_FLAT);
			m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
			if (!(m_dwStyle & styleNoBorder))
			{
				m_DC.SetRgnStyle(RS_HOLLOW);
				if (GetFlag(IF_SELECTED))
				{
					m_DC.SetPen(VD_GetSysColor(COLOR_FRAMESELECTED),VD_PS_SOLID,2);
				}
				else
				{
					m_DC.SetPen(VD_GetSysColor(COLOR_FRAME),VD_PS_SOLID,2);
				}
				m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()),5,7);
			}
		}
		else// usebmpslice
		{
			if(!GetFlag(IF_ENABLED))
			{
				DrawCtrlButton(ctrlStateNormal);
			}
			else if(m_bDown)
			{
				DrawCtrlButton(ctrlStatePushed);
			}
			else if(GetFlag(IF_SELECTED))
			{
				DrawCtrlButton(ctrlStateSelect);
			}
			else
			{
				DrawCtrlButton(ctrlStateNormal);
			}
		}
	}
	//字符
	
	if(!GetFlag(IF_ENABLED))
	{
        //Modify by Lirl on Jan/17/2012,设置按键按下不可用时的颜色
        if (m_bPage)
        {
            m_DC.SetFont(VD_RGB(255,255,255)); //页面按键按下不可用时的颜色
        }
        else
        {
		    m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTDISABLED));
        }
	}
	else if(GetFlag(IF_SELECTED))
	{
		//m_DC.SetBrush(GetSysColor(COLOR_CTRL));
		if((m_dwStyle&styleUseBmp) != styleUseBmp)
		{
			m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));  //cj@20110422
			//m_DC.SetFont(VD_RGB(180,80,0));
		}
	}
	else
	{
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXT));	//cj@20110422
		//m_DC.SetFont(VD_RGB(50,50,50));
	}
	m_DC.SetTextAlign(VD_TA_CENTER);
	m_DC.SetBkMode(BM_TRANSPARENT);
	if(m_bDown)
	{
		m_DC.VD_TextOut(CRect(2,4,m_Rect.Width(),m_Rect.Height()+1),string.c_str());
	}
	else
	{
		m_DC.VD_TextOut(CRect(0,3,m_Rect.Width(),m_Rect.Height()+3),string.c_str());
	}

	//根据判断是否具有贴图属性来判断是否要刷新这个button的区域，而不通过oem宏。--by wangqin 20070412 modified
	if (((m_dwStyle&buttonNormalBmp) != buttonNormalBmp) && ((m_dwStyle & styleNoBorder)!=styleNoBorder))
	{
		if((m_dwStyle & styleUseBmp)!= styleUseBmp){
			m_DC.SetPen(VD_GetSysColor(VD_COLOR_WINDOW),VD_PS_SOLID,1);
			m_DC.SetRgnStyle(RS_RAISED|RS_HOLLOW);
			m_DC.Rectangle(CRect(3,3,m_Rect.Width()-3,m_Rect.Height()-3),3,5);
		}
	}
	m_DC.UnLock();
}


//将按下事件作为主要事件，即按下是直接执行某些操作，原因是遥控器无弹起事件
VD_BOOL CButton::MsgProc(uint msg, uint wpa, uint lpa)
{
	int key;
	int px, py;
	
	switch(msg){
	case XM_KEYDOWN:
		key = wpa;
		switch(key){
		case KEY_RET:
			if(!m_bDown)
			{
				m_bDown = TRUE;
				ItemProc(m_onChanged);
				Draw();

				m_bDown = FALSE;
				goto proc_exec;
			}
			break;
		default:
			return FALSE;
		}
		break;

	case XM_KEYUP:
		key = wpa;
		switch(key)
		{
		case KEY_RET:
			ItemProc(m_onChanged);
			Draw();
			break;
		default:
			return FALSE;
		}
		break;

			
	case XM_LBUTTONDOWN:
	case XM_LBUTTONDBLCLK:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(!GetFlag(IF_CAPTURED))
		{
			SetFlag(IF_CAPTURED, TRUE);
			m_bDown = TRUE;
			Draw();
			ItemProc(m_onChanged);
		}
		break;
		
	case XM_LBUTTONUP:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(GetFlag(IF_CAPTURED))
		{
			SetFlag(IF_CAPTURED, FALSE);
			if(!PtInRect(m_Rect, px, py))
			{
				
				m_bDown = FALSE;
				Draw();
				ItemProc(m_onChanged);
				break;
			}
			else
			{
				
				m_bDown = FALSE;
				ItemProc(m_onChanged);
			}

proc_exec:
			

			if(id==ID_OK)
			{
				m_pParent->Close(UDM_OK);
			}
			else if(id==ID_CANCEL)
			{
				m_pParent->Close(UDM_CANCEL);
			}
			else if(id==ID_DEFAULT)
			{
				m_pParent->UpdateData(UDM_DFLT);
			}
			else if(id==ID_COPY)
			{
				m_pParent->UpdateData(UDM_COPY);
			}
			else if(id==ID_PASTE)
			{
				m_pParent->UpdateData(UDM_PASTE);
			}
			else
			{
				ItemProc(m_onClick);
				if(child)
				{
					child->Open();
				}
			}
			Draw();
		}
		break;

	case XM_MOUSEMOVE:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(GetFlag(IF_CAPTURED))
		{
			if(PtInRect(m_Rect, px, py))
			{
				if(!m_bDown)
				{
					m_bDown = TRUE;
					ItemProc(m_onChanged);
					Draw();
				}
			}
			else
			{
				if(m_bDown)
				{
					m_bDown = FALSE;
					ItemProc(m_onChanged);
					Draw();
				}
			}
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void CButton::SetText(VD_PCSTR psz)
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

VD_PCSTR CButton::GetText()
{
	return string.c_str();
}

void CButton::SetBitmap(VD_BITMAP * pBmpNormal, VD_PCRECT pRectNormal, VD_PCRECT pRectSelected, VD_PCRECT pRectDisabled, VD_PCRECT pRectPushed, VD_BOOL bTransparent /* = TRUE*/)
{
	if (!pBmpNormal) {
		return;
	}
	m_iBkMode = bTransparent ? BM_TRANSPARENT : BM_NORMAL;
	//bmp 和拼起来的buttonbmp 是对立的,此外，这个和普通的buttonNormalBmp也有区别，区别在于区域的设置计算上。
	m_dwStyle &= ~styleUseBmp;
	m_dwStyle = m_dwStyle |buttonNormalBmp|buttonFullBmpRect;
	m_pBmpNormal = pBmpNormal;
	
	if(pRectNormal)
	{
		rect_normal = *pRectNormal;
	}
	if(pRectSelected)
	{
		rect_selected = *pRectSelected;
	}
	if(pRectDisabled)
	{
		rect_disabled = *pRectDisabled;
	}
	if(pRectPushed)
	{
		rect_pushed = *pRectPushed;
	}
	Draw();
}

void CButton::SetBitmap(VD_BITMAP* pBmpNormal, VD_BITMAP* pBmpSelect, VD_BITMAP* pBmpPush, VD_BITMAP* pBmpDisable, VD_BOOL bTransparent /* = TRUE*/)
{
	if (pBmpNormal && pBmpSelect && pBmpPush )
	{
		m_dwStyle &= ~styleUseBmp;
		m_dwStyle |= buttonNormalBmp;

		m_iBkMode = bTransparent ? BM_TRANSPARENT : BM_NORMAL;
		m_pBmpNormal = pBmpNormal;
		m_pBmpDisable = pBmpDisable;
		m_pBmpSelect = pBmpSelect;
		m_pBmpPush = pBmpPush;

		Draw();
	}
}

VD_BOOL CButton::GetState()
{
	return m_bDown;
}

void CButton::SetDefaultBitmap(ButtonBitmap region, VD_PCSTR name)
{
	switch(region)
	{
	case BTB_NORMAL:
		m_bmpDefNormal = VD_LoadBitmap(name);
		break;
	case BTB_SELECTED:
		m_bmpDefSelected = VD_LoadBitmap(name);
		break;
	case BTB_PUSHED:
		m_bmpDefPushed = VD_LoadBitmap(name);
		break;
	case BTB_DISABLED:
		m_bmpDefDisabled = VD_LoadBitmap(name);
		break;
	default:
		break;
	}
}

