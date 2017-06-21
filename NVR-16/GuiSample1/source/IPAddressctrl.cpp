//
//  "$Id: IPAddressctrl.cpp 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#include "GUI/Ctrls/Page.h"

VD_BITMAP* CIPAddressCtrl::bmp_ipb_normal = NULL;
VD_BITMAP* CIPAddressCtrl::bmp_ipb_selected = NULL;
VD_BITMAP* CIPAddressCtrl::bmp_ipb_pushed = NULL;
VD_BITMAP* CIPAddressCtrl::bmp_ipb_disabled = NULL;

///////////////////////////////////////////////////////////////////
////////////        CIPAddressCtrl
CIPAddressCtrl::CIPAddressCtrl(VD_PCRECT pRect, CPage * pParent,uint style)
:CItem(pRect, pParent, IT_IPADDRESSCTRL,style|styleAutoFocus)//, FALSE)
{
	CRect m_rect;
	m_spliter = ".";
	
	if(pRect!= NULL)
	{
		for(int i=0;i<4;i++)
		{
			m_rect.SetRect(pRect->left+TEXT_WIDTH*5/2*i+2,pRect->top+2,pRect->left+TEXT_WIDTH*5/2*i+TEXT_WIDTH*2+2,pRect->bottom-2);
			pnb[i] = CreateNumberBox(m_rect, pParent,0,255,"%3d",NULL,NULL,NULL,styleNoBorder);
			pnb[i]->SetOwner(this);
		}
	}
	else //pRect == NULL 自动布局用
	{
		for(int i=0;i<4;i++)
		{
			m_rect.SetRect(0,0,TEXT_WIDTH*2,CTRL_HEIGHT-4);
			pnb[i] = CreateNumberBox(m_rect,pParent,0,255,"%3d",NULL,NULL,NULL,styleNoBorder);
			pnb[i]->SetOwner(this);
		}
	}
	m_ColorFrameNormal = VD_GetSysColor(COLOR_FRAME);
	m_ColorFrameSelect = VD_GetSysColor(COLOR_FRAMESELECTED);
	m_BkColor = VD_GetSysColor(VD_COLOR_WINDOW);
}

CIPAddressCtrl::~CIPAddressCtrl()
{
}

void CIPAddressCtrl::SetBkColor(VD_COLORREF color, VD_BOOL redraw /* = FALSE */)
{
	m_BkColor = color;

	for(int i=0;i<4;i++)
	{
		pnb[i]->SetBkColor(m_BkColor);
	}
	
	if(redraw)
	{
		Draw();
	}
}


CIPAddressCtrl* CreateIPAddressCtrl(VD_PCRECT pRect, CPage * pParent,uint style)
{
	return new CIPAddressCtrl(pRect,pParent,style);
}

VD_SIZE& CIPAddressCtrl::GetDefaultSize(void)
{
	static VD_SIZE size = {TEXT_WIDTH*10,CTRL_HEIGHT};
	return size;
}

void CIPAddressCtrl::Draw()
{
	if(!DrawCheck()){
		return;
	}
	m_DC.Lock();


	m_DC.SetBrush(m_BkColor);
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
	m_DC.SetRgnStyle(RS_HOLLOW);
	m_DC.SetTextAlign(VD_TA_LEFTTOP);
	m_DC.SetBkMode(BM_TRANSPARENT);
	int penwidth = 0;
	penwidth = 2;
	if(GetFlag(IF_SELECTED))
	{	
		m_DC.SetPen(m_ColorFrameSelect, VD_PS_SOLID, penwidth);
	}
	else
	{
		m_DC.SetPen(m_ColorFrameNormal, VD_PS_SOLID, penwidth);
	}
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()), 3, 3);

	if(!GetFlag(IF_ENABLED))
	{
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTDISABLED));
	}
	else
	{
		m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXT));
	}
	m_DC.SetBkMode(BM_TRANSPARENT);
	for(int i=0;i<3;i++)
	{
		m_DC.VD_TextOut(CRect(2+TEXT_WIDTH*(4+i*5)/2,0,2+TEXT_WIDTH*(5+i*5)/2,TEXT_HEIGHT),m_spliter.c_str());
	}
	m_DC.UnLock();
}

void CIPAddressCtrl::Select(VD_BOOL flag)
{
	SetFlag(IF_SELECTED, flag);
	m_DC.Lock();
	int penwidth = 0;
	penwidth = 2;
	if(GetFlag(IF_SELECTED))
	{	
		m_DC.SetPen(m_ColorFrameSelect, VD_PS_SOLID, penwidth);
	}
	else
	{
		m_DC.SetPen(m_ColorFrameNormal, VD_PS_SOLID, penwidth);
	}
	m_DC.SetRgnStyle(RS_HOLLOW);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()), 3, 3);
	m_DC.UnLock();
}

void CIPAddressCtrl::Enable(VD_BOOL flag)
{
	CItem::Enable(flag);
	for(int i=0;i<4;i++)
		pnb[i]->Enable(flag);
}

void CIPAddressCtrl::Show(VD_BOOL flag, VD_BOOL redraw )

{
	CItem::Show(flag,redraw);
	for (int i =0 ;i<4;i++)
	{
		pnb[i]->Show(flag);
	}
}

void CIPAddressCtrl::Show(VD_BOOL flag)
{
	CItem::Show(flag);
	for (int i =0 ;i<4;i++)
	{
		pnb[i]->Show(flag);
	}
}


void CIPAddressCtrl::setSplit(const std::string &spliter)
{
	m_spliter = spliter;
}

void CIPAddressCtrl::setMaxNumber(int iMaxNumber)
{
	for(int i=0;i<4;i++)
	{
		pnb[i]->SetRange(0, iMaxNumber);
	}
}

void CIPAddressCtrl::SetIPAddress(IPADDR *p)
{
	if(!p)
		return;
	int i;
	GetIPAddress(&ipa);
	if(memcmp(&ipa,p,sizeof(IPADDR))==0)
		return;
	ipa = *p;
	for(i=0;i<4;i++)
		pnb[i]->SetValue(p->c[i]);
}

void CIPAddressCtrl::GetIPAddress(IPADDR *p)
{
	if(!p)
		return;
	int i;
	for(i=0;i<4;i++)
		p->c[i] = pnb[i]->GetValue();
}

void CIPAddressCtrl::SetSerialNumber(SERIALNUMBER *p)
{
	if(!p)
		return;
	int i;
	SERIALNUMBER serialNumber;
	
	GetSerialNumber(&serialNumber);
	if(memcmp(&serialNumber,p,sizeof(SERIALNUMBER))==0)
		return;
	serialNumber = *p;
	for(i=0;i<4;i++)
		pnb[i]->SetValue(p->iNumber[i]);

}
void CIPAddressCtrl::GetSerialNumber(SERIALNUMBER *p)
{
	if(!p)
		return;
	int i;
	for(i=0;i<4;i++)
		p->iNumber[i] = pnb[i]->GetValue();
}

void CIPAddressCtrl::SetDefaultBitmap(IPAddressCtrlBitmap region, VD_PCSTR name)
{
	switch(region) {
		case IPB_NORMAL:
		bmp_ipb_normal = VD_LoadBitmap(name);
		break;
	case IPB_SELECTED:
		bmp_ipb_selected = VD_LoadBitmap(name);
		break;
	case IPB_PUSHED:
		bmp_ipb_pushed = VD_LoadBitmap(name);
		break;
	case IPB_DISABLED:
		bmp_ipb_disabled = VD_LoadBitmap(name);
		break;
	default:
		break;
	}
}

void CIPAddressCtrl::setSerialFormat(VD_PCSTR pcFmt)
{
	int i;
	
	if (NULL == pcFmt)
	{
		return;
	}
	
	for(i=0;i<4;i++)
	{
		if (NULL != pnb[i])
		{
			pnb[i]->SetFormat(pcFmt);			
		}
	}
}

