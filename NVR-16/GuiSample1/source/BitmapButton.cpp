//
//  "$Id: BitmapButton.cpp 117 2008-11-27 09:44:43Z liwj $"
//
//  Copyright (c)2008-2010, ZheJiang XuanChuang Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#include "GUI/Ctrls/Page.h"


CBitmapButton* CreateBitmapButton(
								 VD_PCRECT pRect, 
								 CPage * pParent, 
								 VD_PCSTR psz,
								 VD_BITMAP* res1,
								 VD_BITMAP* res2,
								 VD_BITMAP* res3,
								 CPage *pPage,
								 uint style
								 )
{
	return new CBitmapButton(pRect,pParent,psz,res1,res2,res3,pPage,style);
}




CBitmapButton::CBitmapButton(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz,VD_BITMAP* res1,VD_BITMAP* res2,VD_BITMAP* res3,CPage *pPage,uint style)
	:CItem(pRect, pParent,IT_BITMAPBUTTON,style|styleEditable|styleAutoFocus)
{
	if(psz)
	{
		string = GetParsedString(psz);
	}
	child = pPage;
	
	bdown = FALSE;
	bmp1 = res1;
	bmp2 = res2;
	bmp3 = res3;
	ReplaceBitmapBits(bmp1,VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	ReplaceBitmapBits(bmp2,VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
	ReplaceBitmapBits(bmp3,VD_GetSysColor(COLOR_TRANSPARENT), VD_GetSysColor(VD_COLOR_WINDOW));
}

CBitmapButton::~CBitmapButton()
{
}


VD_SIZE& CBitmapButton::GetDefaultSize(void)
{
	static VD_SIZE size = {BMP_WIDTH, BMP_HEIGHT};

	return size;
}

void CBitmapButton::Draw()
{
	if(!DrawCheck()){
		return;
	}
	m_DC.Lock();
	m_DC.SetBkMode(BM_NORMAL);
	if(!GetFlag(IF_EDITABLED)){
		m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),bmp3);
	}else if(bdown){//按下时显示成选中状态，避免由bdown设置不当造成的显示错误
		m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),bmp1);
	}else if(GetFlag(IF_SELECTED)){
		m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),bmp2);
	}else{
		m_DC.Bitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),bmp1);
	}
	if(string.c_str()){
		//文字宽度会超过控件宽度而无法显示，所以在父页面上再画一次
		CDC * pDC = m_pParent->GetDC();
		pDC->Lock();
		int width = g_Locales.GetTextExtent(string.c_str());
		CRect rect((m_Rect.left + m_Rect.right - width) / 2, m_Rect.bottom - TEXT_HEIGHT, (m_Rect.left + m_Rect.right + width) / 2, m_Rect.bottom);
		if(GetFlag(IF_SELECTED)){
			pDC->SetFont(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
		}
		else
		{
		pDC->SetFont(VD_GetSysColor(COLOR_CTRLTEXT));
		}
		pDC->DPtoLP(rect);
		pDC->SetTextAlign(VD_TA_CENTER);
		pDC->VD_TextOut(rect,string.c_str());
		pDC->UnLock();
		//画图标下面文字
		m_DC.SetBkMode(BM_TRANSPARENT);
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
		m_DC.Rectangle(CRect(0, m_Rect.Height() - TEXT_HEIGHT, m_Rect.Width(), m_Rect.Height()));
		m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
		m_DC.Rectangle(CRect(0, m_Rect.Height() - TEXT_HEIGHT, m_Rect.Width(), m_Rect.Height()));
		if(GetFlag(IF_SELECTED))
		{
			m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXTSELECTED));
		}
		else
		{
			m_DC.SetFont(VD_GetSysColor(COLOR_CTRLTEXT));
		}
		m_DC.SetTextAlign(VD_TA_CENTER);
		m_DC.VD_TextOut(CRect(0, m_Rect.Height() - TEXT_HEIGHT, m_Rect.Width(), m_Rect.Height()),string.c_str());
	}
	m_DC.UnLock();
}

//将按下事件作为主要事件，即按下是直接执行某些操作，原因是遥控器无弹起事件
VD_BOOL CBitmapButton::MsgProc(uint msg, uint wpa, uint lpa)
{
	int px, py;
	int key;
	switch(msg){
	case XM_KEYDOWN:
		key = wpa;
		switch(key){
		case KEY_RET:
			if(!bdown)
			{
				bdown = TRUE;
				Draw();
				
				if(child)
				{
					child->Open();
				}
				bdown = FALSE;
				Draw();
			}
			break;
		default:
			return FALSE;
		}
		break;
		
	case XM_KEYUP:
		key = wpa;
		switch(key){
		case KEY_RET:
			
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
			bdown = TRUE;
			Draw();
		}
		break;
		
	case XM_LBUTTONUP:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(GetFlag(IF_CAPTURED))
		{
			SetFlag(IF_CAPTURED, FALSE);
			bdown = FALSE;
			Draw();
			if(PtInRect(m_Rect, px, py))
			{
				if(child)
				{
					child->Open();
				}
			}
		}
		break;
			
	case XM_MOUSEMOVE:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(GetFlag(IF_CAPTURED)){
			if(PtInRect(m_Rect, px, py)){
				if(!bdown){
					bdown = TRUE;
					Draw();
				}
			}else{
				if(bdown){
					bdown = FALSE;
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

VD_PCSTR CBitmapButton::GetText()
{
	return string.c_str();
}
