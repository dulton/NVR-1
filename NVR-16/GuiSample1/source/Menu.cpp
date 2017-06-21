//
//  "$Id: Menu.cpp 266 2008-12-15 09:37:38Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#include "GUI/Ctrls/Page.h"

VD_BITMAP* CMenu::m_bmpPanel = NULL;
VD_BITMAP* CMenu::m_bmpTitledPanel = NULL;
VD_BITMAP* CMenu::m_bmpBar = NULL;
VD_BITMAP* CMenu::m_bmpItemNormal = NULL;
VD_BITMAP* CMenu::m_bmpItemSelected = NULL;
VD_BITMAP* CMenu::m_bmpItemDisabled = NULL;

///////////////////////////////////////////////////////////////////
////////////        CMenu

#define MENU_TITLE_HIGHT 30      //下拉菜单的标题高度
#define PICT_ITEM_WITTH    100       //图片项的宽度
#define MENU_ITEM_WIDTH  100    //下拉菜单项的宽度
#define MENU_ITEM_HIGHT  30      //下拉菜单项的高度
#define MENU_SPLT_WIDTH  8       //分割符菜单项的宽度

CMenu* CreateCMenu(VD_PCRECT pRect, CPage * pParent, VD_PCSTR psz , CItem * pOwner , uint style, VD_BOOL IfDeskTop)
{
	if(style & styleDeskTop)
	{
		style =  styleDeskTop|stylePopup|styleVertical;  
		  
	}
	else if(style & styleNavTextInside)
	{
		style |= styleNavigation;
	}
	else if (style & styleDeskTopNew)
	{
		style =  styleDeskTopNew|stylePopup|styleHorizontal;
	}
	
	CMenu *pMenu = new CMenu(pRect, pParent,  psz, pOwner,style, IfDeskTop);
	pMenu->SetFrameWidth(1);
	return pMenu;
}

CMenu::CMenu(VD_PCRECT pRect, CPage * pParent,  VD_PCSTR psz /* = NULL */, CItem *pOwner /* = NULL */,uint style/* = stylePopup | styleVertical*/, VD_BOOL IfDeskTop)
:CItem(pRect, pParent, IT_MENU, style|styleAutoFocus|styleEditable)
{
	move_sel = -1;
	
	int i;
	m_pOwner = pOwner;
	m_IFDTop = IfDeskTop;
	m_autoSpread = TRUE;
	for(i = 0; i < MENU_MAX; i++)
	{
		items[i].string.erase();
		items[i].tip.erase();
		items[i].bmp1 = NULL;
		items[i].bmp2 = NULL;
		items[i].bmp3 = NULL;
		items[i].sub = NULL;
		items[i].proc = NULL;
		items[i].child = NULL;
		items[i].sprtr = FALSE;
	}
	cursel = -1;
	lastssel = -1;
	pParentMenu = NULL;
	num = 0;
	clicked = FALSE;
	if(psz)
	{
		title = GetParsedString(psz);
		string = title; //add by xie for test
	}

	SetRevPopup(FALSE);
	Init();
}

CMenu::~CMenu()
{
}

void CMenu::Init(void)
{
	m_iColNum = 0;
	m_iRowNum = 0;
	m_bmpBKNormal = NULL;
	m_bmpSingleNormal = NULL;
	m_bmpSingleSelect = NULL;
	m_iFrameWidth = 2;
	m_colBKTitle = 0;
	m_iConstColNum = 0;
	m_colBKNormal = VD_GetSysColor(VD_COLOR_WINDOW);
	m_colSingleNormal = VD_GetSysColor(VD_COLOR_WINDOW);
	m_colSingleSelect = VD_GetSysColor(VD_COLOR_WINDOW);
	m_colFontTitle = VD_GetSysColor(COLOR_TITLETEXT);
	m_colFrameNormal = VD_GetSysColor(VD_COLOR_WINDOW);
	m_colFrameSelect = VD_GetSysColor(COLOR_FRAMESELECTED);
	m_colBKFrame = VD_GetSysColor(COLOR_POPUP);
	
	if(m_dwStyle & styleVertical)
	{
		if(m_Rect.right || m_Rect.bottom)
		{
			m_Rect.right = 0;
			m_Rect.bottom = 0;
		}
		m_iySpace = 2;
		m_ixSpace = 4;
		x_offset = 0;
		m_iItemWidth = 0;
		m_iStartX = TEXT_WIDTH;
		if(title.size())
		{
			y_offset = MENU_TITLE_HIGHT;
			m_iItemHeight = CTRL_HEIGHT;
			m_bmpBKNormal = m_bmpTitledPanel;
			m_colBKTitle = VD_RGB(255,148,0);
		}
		else
		{
			y_offset = m_iySpace;
			m_iItemHeight = TEXT_HEIGHT;
			m_bmpBKNormal = m_bmpPanel;
		}
		m_bmpSingleNormal = m_bmpItemNormal;
		m_bmpSingleSelect = m_bmpItemSelected;
		m_colFontNormal = VD_GetSysColor(COLOR_CTRLTEXT);		
		m_colFontSelect = VD_GetSysColor(COLOR_CTRLTEXTSELECTED);
		m_colBKFrame = VD_RGB(100,100,100);  //add by xie
		m_iFrameWidth = 1;
	}
	else if(m_dwStyle & styleHorizontal)
	{
		if(m_Rect.Width() == 0 || m_Rect.Height() == 0)
		{
			m_Rect = CRect(0,0,620 + CLIENT_X * 2,88);
		}
		m_ixSpace = 0;
		m_iySpace = 0;
		x_offset = m_ixSpace;
		y_offset = m_iySpace;
		m_iItemHeight = 80;
		//m_iItemWidth = MENU_ITEM_WIDTH;
		m_iItemWidth = 40;

		m_bmpBKNormal = m_bmpBar;
		m_colFontNormal = VD_RGB(148,162,198);
		m_colFontSelect = VD_RGB(148,162,198);
	}
	else if(m_dwStyle & styleNavigation)
	{	
		if(m_Rect.Width() == 0 || m_Rect.Height() == 0)
		{
			m_Rect = CRect(0,0,430,360);
		}
		m_ixSpace = 0;
		m_iySpace = 0;
		x_offset = m_ixSpace;
		y_offset = m_iySpace;
		m_colFrameNormal = VD_GetSysColor(VD_COLOR_WINDOW);
		m_colFrameSelect = VD_GetSysColor(VD_COLOR_WINDOW);
		m_colFontNormal = VD_GetSysColor(COLOR_CTRLTEXT);
		m_colFontSelect = VD_GetSysColor(COLOR_CTRLTEXTSELECTED);
		m_colBKFrame = VD_GetSysColor(VD_COLOR_WINDOW);

		m_colBKNormal = VD_RGB(33,33,33); //xym GUI
		
		if(m_dwStyle & styleNavTextInside)
		{
			m_iItemHeight = 44;
			m_iItemWidth = 194;
		}
		else
		{
			///m_iItemHeight = 104;
			m_iItemHeight = 140;
			m_iItemWidth = 80;
		}
	}
	else
	{
	}
	m_customSize.w = -1;
	m_customSize.h = -1;
	m_customInterval.w = -1;
	m_customInterval.h = -1;
}

void CMenu::Draw(void)
{
	int i;
	if(!DrawCheck()){
		return;
	}
	DrawBk();
	for (i = 0; i < num; i++)
	{
		DrawSingle(i, (i == cursel));
	}


	if (m_dwStyle & styleDeskTopNew)
	{
		m_DC.Lock();

		m_DC.SetTextAlign(VD_TA_LEFT);
		m_DC.SetFont(m_colFontNormal);
		m_DC.SetBkMode(BM_TRANSPARENT);
		//m_DC.VD_TextOut(CRect(40,96,m_Rect.Width(),96+TEXT_HEIGHT),"SYS MENU");

		m_DC.UnLock();
	}
	


}

void CMenu::DrawBk(void)
{
	m_DC.Lock();

	if(m_dwStyle & styleVertical)
	{
		if(m_bmpBKNormal)
		{
			if(title.size())
			{
				m_DC.SetBkMode(BM_NORMAL);
			}
			else
			{
				m_DC.SetBkMode(BM_TRANSPARENT);
			}
			if(title.size() && num < 2)
			{
				m_DC.WideBitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()-8),m_bmpBKNormal,CRect(0,0,m_bmpBKNormal->width,32));
				m_DC.WideBitmap(CRect(0,m_Rect.Height()-8,m_Rect.Width(),m_Rect.Height()),m_bmpBKNormal,CRect(0,m_bmpBKNormal->height-8,m_bmpBKNormal->width,m_bmpBKNormal->height));
			}
			else
			{
				m_DC.WideBitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()),m_bmpBKNormal);
			}

			if(title.size()){
				m_DC.SetBkMode(BM_TRANSPARENT);
				m_DC.SetFont(m_colFontTitle, FS_NORMAL);
				m_DC.SetTextAlign(VD_TA_CENTER);
				m_DC.VD_TextOut(CRect(0,10,m_Rect.Width(),MENU_TITLE_HIGHT),title.c_str());
			}
		}
		else 
		{

			m_DC.SetPen(m_colBKFrame);
			m_DC.SetBrush(m_colBKNormal);
			
			// 不画边框
			if (m_menuType == MT_NOBODER)
			{
				m_DC.SetRgnStyle(RS_FLAT);
			} 
			else
			{
				m_DC.SetRgnStyle(RS_NORMAL);
			}

			m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
			
			//add by xie  重新绘制右键菜单中的第一个Item的背景颜色
			if(strcmp(items[0].string.c_str(), (char*)LOADSTR("pgquick.sysmenu")) == 0)
			{
				m_DC.SetBrush(VD_RGB(100,100,100));
				m_DC.Rectangle(CRect(0,0,m_Rect.Width(), m_iItemHeight+m_iySpace));
			}			
			//end
			
			if(title.size())
			{
				m_DC.SetBrush(m_colBKTitle);
				m_DC.Rectangle(CRect(20,0,m_Rect.Width()-20,MENU_TITLE_HIGHT),4,4);

				m_DC.SetBkMode(BM_TRANSPARENT);
				//m_DC.SetFont(m_colFontTitle, FS_BOLD);
				m_DC.SetFont(m_colFontTitle, FS_NORMAL);
				m_DC.SetTextAlign(VD_TA_CENTER);
				m_DC.VD_TextOut(CRect(0,0,m_Rect.Width(),MENU_TITLE_HIGHT),title.c_str());
			}

		}
	}
	else //styleHorizontal | styleNavigation
	{
	

		if(m_bmpBKNormal){

			m_DC.SetBkMode(BM_NORMAL);
			m_DC.WideBitmap(CRect(0,0,m_Rect.Width(),m_Rect.Height()), m_bmpBKNormal/*, CRect(0,0,templet->width,m_Rect.Height())*/);

		}
		else
		{
			m_DC.SetBkMode(BM_NORMAL);
			m_DC.SetPen(m_colBKFrame); //背景的边框颜色
			m_DC.SetBrush(m_colBKNormal);  //背景颜色
			m_DC.SetRgnStyle(RS_NORMAL); //选择背景带有边框	
			m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));

			//xym GUI  //在主菜单的下方画出两条分割线
			if(!(m_dwStyle & stylePopup))
			{
				///m_DC.SetPen(RGB(214,214,222), PS_SOLID,2);
				m_DC.SetPen(VD_RGB(165,165,181), VD_PS_SOLID,2);//m_DC.SetPen(RGB(100,100,100), PS_SOLID,2);
				///m_DC.MoveTo(0, 75);  //此处为相对坐标,参照物为主菜单的坐标
				//m_DC.LineTo(m_Rect.Width(), 75);
				m_DC.MoveTo(0, 90);
				m_DC.LineTo(m_Rect.Width(), 90);				

				m_DC.SetPen(VD_RGB(165,165,181), VD_PS_SOLID,1);//m_DC.SetPen(RGB(120,120,120), PS_SOLID,1);
				m_DC.MoveTo(0, m_Rect.Height()-2);  //此处为相对坐标,参照物为主菜单的坐标
				m_DC.LineTo(m_Rect.Width(), m_Rect.Height()-2);
			}
			//end
			

			if (m_dwStyle & styleDeskTopNew)
			{
				m_DC.Lock();

				m_DC.SetPen(VD_RGB(102,102,102), VD_PS_SOLID, 4);
				m_DC.MoveTo(0, 0);
				m_DC.LineTo(0, m_Rect.Height());

				m_DC.SetPen(VD_RGB(113,113,120), VD_PS_SOLID, 2);
				m_DC.MoveTo(2, 3);
				m_DC.LineTo(2, m_Rect.Height()-3);

				m_DC.SetPen(VD_RGB(102,102,102), VD_PS_SOLID, m_Rect.Width());
				m_DC.MoveTo(4, 1);
				m_DC.LineTo(4, 4);	

				m_DC.SetPen(VD_RGB(113,113,120), VD_PS_SOLID, m_Rect.Width()-4);
				m_DC.MoveTo(2, 3);
				m_DC.LineTo(2, 4);	

				m_DC.SetPen(VD_RGB(102,102,102), VD_PS_SOLID, m_Rect.Width());
				m_DC.MoveTo(4, m_Rect.Height()-4);
				m_DC.LineTo(4, m_Rect.Height());

				m_DC.SetPen(VD_RGB(113,113,120), VD_PS_SOLID, m_Rect.Width());
				m_DC.MoveTo(4, m_Rect.Height()-4);
				m_DC.LineTo(4, m_Rect.Height()-3);

				m_DC.SetPen(VD_RGB(102,102,102), VD_PS_SOLID, 4);
				m_DC.MoveTo(m_Rect.Width()-4, 0);
				m_DC.LineTo(m_Rect.Width()-4, m_Rect.Height());

				m_DC.SetPen(VD_RGB(113,113,120), VD_PS_SOLID, 2);
				m_DC.MoveTo(m_Rect.Width()-4, 3);
				m_DC.LineTo(m_Rect.Width()-4, m_Rect.Height()-3);

				m_DC.UnLock();
			}
		}
				
	}

	m_DC.UnLock();
	return;
}

void CMenu::DrawSingle(int index, VD_BOOL bDrawBk /* = TRUE */)
{
	if(!DrawCheck() || index<0 || index >= num){
		return;
	}

	m_DC.Lock();
	if (items[index].sprtr)
	{
		m_DC.SetPen(VD_GetSysColor(COLOR_POPUP), VD_PS_SOLID,1);
				
		if(m_dwStyle & styleVertical)
		{
			if(items[index].bmp1)
			{
				m_DC.WideBitmap(CRect(0,items[index].y_offset,m_Rect.Width(),items[index].y_offset+m_iItemHeight), items[index].bmp1);
			}
			else
			{
				m_DC.MoveTo(m_ixSpace,MENU_SPLT_WIDTH/2+items[index].y_offset);
				m_DC.LineTo(m_Rect.Width()-m_ixSpace,MENU_SPLT_WIDTH/2+items[index].y_offset);
			}
		}
		else if(m_dwStyle & styleHorizontal)
		{
			if(items[index].bmp1)
			{
				//设置菜单中间图片的位置				
				m_DC.WideBitmap(CRect(12+items[index].x_offset,4,92+items[index].x_offset,88), items[index].bmp1);
			}
			else
			{
				m_DC.MoveTo(items[index].x_offset +MENU_SPLT_WIDTH/2,m_ixSpace);
				m_DC.LineTo(items[index].x_offset +MENU_SPLT_WIDTH/2,m_Rect.Height()-m_ixSpace);
			}
			
		}
		else if(m_dwStyle & styleNavigation)
		{
			if(items[index].bmp1)
			{
				m_DC.WideBitmap(CRect(items[index].x_offset,items[index].y_offset,items[index].x_offset + m_iItemWidth,items[index].y_offset+m_iItemWidth), items[index].bmp1);
			}
			else
			{
				m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
				m_DC.SetRgnStyle(RS_FLAT);
				m_DC.Rectangle(CRect(items[index].x_offset,items[index].y_offset,items[index].x_offset + m_iItemWidth,items[index].y_offset+m_iItemWidth));
			}
		}
		else
		{
		}
		m_DC.UnLock();
		return;
	}

	if(m_dwStyle & styleVertical)
	{
		if(title.size() && num<2)
		{
			m_DC.UnLock();
			return;
		}
		//add by xie
		if((0 == index) && (strcmp(items[0].string.c_str(), (char*)LOADSTR("pgquick.sysmenu")) == 0))
		{	
			//第一个Item的边框颜色必须和DrawBk()中的第一个Item的背景颜色相同
			m_DC.SetPen(VD_RGB(100,100,100), VD_PS_SOLID, m_iFrameWidth);
			m_DC.SetFont(m_colFontNormal, FS_NORMAL);
			//第一个Item的背景颜色和边框颜色保持相同
			m_DC.SetBrush(VD_RGB(100,100,100));
		}
		//end
		else if ( cursel != index )
		{			
			m_DC.SetPen(m_colFrameNormal, VD_PS_SOLID, m_iFrameWidth);			
			//m_DC.SetFont(m_colFontNormal, FS_BOLD);
			m_DC.SetFont(m_colFontNormal, FS_NORMAL);
			m_DC.SetBrush(m_colSingleNormal);
		}
		else
		{
			m_DC.SetPen(m_colFrameSelect, VD_PS_SOLID, m_iFrameWidth);
			//m_DC.SetFont(m_colFontSelect, FS_BOLD);
			m_DC.SetFont(m_colFontSelect, FS_NORMAL);
			m_DC.SetBrush(m_colSingleSelect);
		}
		m_DC.SetTextAlign(VD_TA_LEFTTOP);
		m_DC.SetBkMode(BM_NORMAL);
		
		if ( cursel != index )
		{
			if(m_bmpSingleNormal)
			{
				m_DC.WideBitmap(CRect(0,items[index].y_offset,m_Rect.Width(),items[index].y_offset+m_iItemHeight), m_bmpSingleNormal);
			}
			else if(m_bmpBKNormal)
			{
				m_DC.WideBitmap(CRect(0,items[index].y_offset,m_Rect.Width(),items[index].y_offset+m_iItemHeight), m_bmpBKNormal ,
				CRect(0,12,m_bmpBKNormal->width,m_bmpBKNormal->height-12));
			}
			else
			{
				m_DC.SetRgnStyle(RS_NORMAL);
				m_DC.Rectangle(CRect(m_ixSpace,items[index].y_offset,m_Rect.Width()-m_ixSpace,items[index].y_offset+m_iItemHeight));
			}
		}
		else
		{
			if(m_bmpSingleSelect)
			{
				m_DC.WideBitmap(CRect(m_ixSpace,items[index].y_offset,m_Rect.Width()-m_ixSpace,items[index].y_offset+m_iItemHeight), m_bmpSingleSelect/*, CRect(0,0,m_bmpItemSelected->width,m_bmpItemSelected->height)*/);
			}
			else
			{
				m_DC.SetRgnStyle(RS_NORMAL);
				m_DC.SetPenWidth(2);
				m_DC.Rectangle(CRect(m_ixSpace,items[index].y_offset,m_Rect.Width()-m_ixSpace,items[index].y_offset+m_iItemHeight));
			}
		}
		
		m_DC.SetBkMode(BM_TRANSPARENT);
		int startX;
		startX = m_iStartX;

		int textTop = items[index].y_offset + (m_iItemHeight - TEXT_HEIGHT) / 2;
		if(cursel != index)  //增加判断图片更改  add langzi 2009-11-9 
		{
			if(items[index].bmp1)
			{
			    startX -= 18; 
				int bmpTop = items[index].y_offset + (m_iItemHeight - items[index].bmp1->height) / 2;
				int bmpRight = startX + items[index].bmp1->width;
				m_DC.Bitmap(CRect(startX,	bmpTop, bmpRight, bmpTop + items[index].bmp1->height), items[index].bmp1);
				
				m_DC.VD_TextOut(CRect(bmpRight + 10,textTop, m_Rect.Width()-startX, textTop + TEXT_HEIGHT),items[index].string.c_str());
			}
			else
			{
				m_DC.VD_TextOut(CRect(startX,textTop,m_Rect.Width()-TEXT_WIDTH,textTop + TEXT_HEIGHT),items[index].string.c_str());
			}
		}
		else             //add langzi 2009-11-9 
		{
			if(items[index].bmp2)
			{
			    startX -= 18; 
				int bmpTop = items[index].y_offset + (m_iItemHeight - items[index].bmp2->height) / 2;
				int bmpRight = startX + items[index].bmp2->width;
				m_DC.Bitmap(CRect(startX,	bmpTop, bmpRight, bmpTop + items[index].bmp2->height), items[index].bmp2);
				
				m_DC.VD_TextOut(CRect(bmpRight + 10,textTop, m_Rect.Width()-startX, textTop + TEXT_HEIGHT),items[index].string.c_str());
			}
			else
			{
				m_DC.VD_TextOut(CRect(startX,textTop,m_Rect.Width()-TEXT_WIDTH,textTop + TEXT_HEIGHT),items[index].string.c_str());
			}
		}

		if(items[index].sub)
		{
			m_DC.SetRgnStyle(RS_NORMAL);
			if ( cursel != index )
			{
				m_DC.SetBrush(m_colFontNormal);
			}
			else
			{
				m_DC.SetBrush(m_colFontSelect);
			}
			m_DC.Trapezoid(m_Rect.Width()-CTRL_HEIGHT1, 8+items[index].y_offset + (m_iItemHeight - TEXT_HEIGHT) / 2, 0,
				m_Rect.Width()-CTRL_HEIGHT1, 12+items[index].y_offset + (m_iItemHeight - TEXT_HEIGHT) / 2, 4);
			m_DC.Trapezoid(m_Rect.Width()-CTRL_HEIGHT1, 12+items[index].y_offset + (m_iItemHeight - TEXT_HEIGHT) / 2, 0,
				m_Rect.Width()-CTRL_HEIGHT1 + 4, 16+items[index].y_offset + (m_iItemHeight - TEXT_HEIGHT) / 2, -4);
		}
	}
	else if(m_dwStyle & styleHorizontal)
	{
		int stlen = 0;


		m_DC.SetBkMode(BM_NORMAL);

		m_DC.SetTextAlign(VD_TA_CENTER);

		stlen = g_Locales.GetTextExtent(items[index].string.c_str());
		if(cursel != index && items[index].bmp1 >= 0)
		{
//////////设置横向菜单图标未选中时的位置

			if (m_dwStyle & styleDeskTopNew)
			{

#if 0
				if (index == lastssel)
				{
					m_DC.SetPen(VD_GetSysColor(VD_COLOR_WINDOW), VD_PS_SOLID, 54+24);
					m_DC.MoveTo(items[index].x_offset, 25);
					m_DC.LineTo(items[index].x_offset, 25+9);

					m_DC.MoveTo(items[index].x_offset, 86);
					m_DC.LineTo(items[index].x_offset, 86+9);

					m_DC.SetPen(VD_GetSysColor(VD_COLOR_WINDOW), VD_PS_SOLID, 12);
					m_DC.MoveTo(items[index].x_offset, 25+9);
					m_DC.LineTo(items[index].x_offset, 25+9+54);


					m_DC.MoveTo(66+items[index].x_offset, 25+9);
					m_DC.LineTo(66+items[index].x_offset, 25+9+54);
				}
#endif

				//printf("left = %d \n",6+items[index].x_offset);
				//printf("right = %d \n",items[index].bmp1->width+items[index].x_offset+12);

				m_DC.Bitmap(CRect(6+items[index].x_offset,5,items[index].bmp1->width+items[index].x_offset+12,120), items[index].bmp1);
			} 
			else
			{
				m_DC.Bitmap(CRect(12+items[index].x_offset,4,92+items[index].x_offset,84), items[index].bmp1);
			}
			
			CDC * pDC = m_pParent->GetDC();
			pDC->Lock();
			pDC->SetTextAlign(VD_TA_CENTER);
			pDC->SetFont(m_colFontNormal);
			
			pDC->SetBkMode(BM_TRANSPARENT);

			if (m_dwStyle & styleDeskTopNew)
			{
				m_DC.SetFont(m_colFontNormal);
				m_DC.SetBkMode(BM_TRANSPARENT);
				m_DC.VD_TextOut(CRect(items[index].x_offset+12,0,items[index].x_offset+66,TEXT_HEIGHT),items[index].string.c_str());
			} 
			else
			{
				pDC->VD_TextOut(CRect(items[index].x_offset+50-stlen/2,88,items[index].x_offset+50+stlen/2,TEXT_HEIGHT+88),items[index].string.c_str());

			}
			
			pDC->UnLock();
		}
		if(cursel == index && items[index].bmp2 >= 0)
		{
//////////设置横向菜单图标选中时的位置

			if (m_dwStyle & styleDeskTopNew)
			{

#if 0
				m_DC.SetPen(VD_RGB(180,80,0), VD_PS_SOLID, 54+24);
				m_DC.MoveTo(items[index].x_offset, 25);
				m_DC.LineTo(items[index].x_offset, 25+9);

				m_DC.MoveTo(items[index].x_offset, 86);
				m_DC.LineTo(items[index].x_offset, 86+9);

				m_DC.SetPen(VD_RGB(180,80,0), VD_PS_SOLID, 12);
				m_DC.MoveTo(items[index].x_offset, 25+9);
				m_DC.LineTo(items[index].x_offset, 25+9+54);


				m_DC.MoveTo(66+items[index].x_offset, 25+9);
				m_DC.LineTo(66+items[index].x_offset, 25+9+54);
#endif

				lastssel = cursel;

				//printf("items[index].x_offset = %d \n",items[index].x_offset);
				//printf("items[index].bmp1->width = %d \n",items[index].bmp1->width);

				m_DC.Bitmap(CRect(6+items[index].x_offset,5,items[index].bmp2->width+items[index].x_offset+12,120), items[index].bmp2);
			} 
			else
			{
				m_DC.Bitmap(CRect(12+items[index].x_offset,4,92+items[index].x_offset,84), items[index].bmp2);

			}

			CDC * pDC = m_pParent->GetDC();
			pDC->Lock();
			pDC->SetTextAlign(VD_TA_CENTER);
			pDC->SetFont(m_colFontSelect);
			pDC->SetBkMode(BM_TRANSPARENT);

			if (m_dwStyle & styleDeskTopNew)
			{
				m_DC.SetFont(m_colFontSelect);
				m_DC.SetBkMode(BM_TRANSPARENT);
				m_DC.VD_TextOut(CRect(items[index].x_offset+12,0,items[index].x_offset+66,TEXT_HEIGHT),items[index].string.c_str());
			} 
			else
			{
				pDC->VD_TextOut(CRect(items[index].x_offset+50-stlen/2,88,items[index].x_offset+50+stlen/2,TEXT_HEIGHT+88),items[index].string.c_str());
			}
			
			pDC->UnLock();
		}
	}
	else if(m_dwStyle & styleNavigation)
	{
		m_DC.SetTextAlign(VD_TA_CENTER);
		m_DC.SetBkMode(BM_TRANSPARENT);
		if(cursel != index)
		{
			//m_DC.SetFont(m_colFontNormal, FS_BOLD);
			m_DC.SetFont(m_colFontNormal, FS_NORMAL);
			if(items[index].bmp1)
			{//trace("CMenu::DrawSingle cursel != index bmp1!=NULL cursel=[%d] index=[%d] string=[%s]\r\n",cursel, index,items[index].string.c_str());
				m_DC.SetBrush(m_colBKNormal);
				m_DC.SetRgnStyle(RS_FLAT);
				m_DC.Rectangle(CRect(items[index].x_offset,items[index].y_offset,items[index].x_offset + items[index].bmp1->width,items[index].y_offset+items[index].bmp1->height));

				m_DC.Bitmap(CRect(items[index].x_offset,items[index].y_offset,items[index].x_offset + items[index].bmp1->width,items[index].y_offset+items[index].bmp1->height),items[index].bmp1);
			}
			else
			{//trace("CMenu::DrawSingle cursel != index bmp1==NULL cursel=[%d] index=[%d] string=[%s]\r\n",cursel, index,items[index].string.c_str());
				m_DC.SetBrush(VD_RGB(255,255,255));
				m_DC.SetRgnStyle(RS_FLAT);
				m_DC.Rectangle(CRect(items[index].x_offset,items[index].y_offset,items[index].x_offset + m_iItemWidth,items[index].y_offset+m_iItemHeight));
			}
		}
		else
		{
			//m_DC.SetFont(m_colFontSelect, FS_BOLD);
			m_DC.SetFont(m_colFontSelect, FS_NORMAL);
			if(items[index].bmp2)
			{//trace("CMenu::DrawSingle cursel == index bmp2!=NULL cursel=[%d] index=[%d] string=[%s]\r\n",cursel, index,items[index].string.c_str());
				m_DC.Bitmap(CRect(items[index].x_offset,items[index].y_offset,items[index].x_offset + items[index].bmp2->width,items[index].y_offset+items[index].bmp2->height),items[index].bmp2);
			}
			else
			{//trace("CMenu::DrawSingle cursel == index bmp2==NULL cursel=[%d] index=[%d] string=[%s]\r\n",cursel, index,items[index].string.c_str());
				m_DC.SetBrush(VD_RGB(255,255,255));
				m_DC.SetRgnStyle(RS_FLAT);
				m_DC.Rectangle(CRect(items[index].x_offset,items[index].y_offset,items[index].x_offset + m_iItemWidth,items[index].y_offset+m_iItemHeight));
			}
		}
		
		int strWidth = g_Locales.GetTextExtent(items[index].string.c_str());
		if(m_dwStyle & styleNavTextInside)
		{
			m_DC.VD_TextOut(CRect(items[index].x_offset, items[index].y_offset + (m_iItemHeight - TEXT_HEIGHT)/2, items[index].x_offset + m_iItemWidth, items[index].y_offset + (m_iItemHeight + TEXT_HEIGHT)/2), items[index].string.c_str());
		}
		else if(m_dwStyle & stylePopup) //xym GUI  //此判断条件可以过滤主菜单不加文字
		{
			if (items[index].string.size())
			{	//modefied by nike.xie  //图片下的文字下移两个单位(使主菜单列表图标下的文字不可见)
				m_DC.VD_TextOut(CRect(items[index].x_offset - (strWidth - m_iItemWidth )/2, items[index].y_offset + m_iItemWidth+2, items[index].x_offset + (strWidth+ m_iItemWidth)/2, items[index].y_offset+m_iItemWidth + TEXT_HEIGHT),items[index].string.c_str());
			}
		}
		else   //add langzi 2009-11-9 
		{        //此判断条件主菜单加文字
		    if (items[index].string.size())  //add langzi 2009-11-9 
			{	
				int iBmpLen = 0;
				if(NULL != items[index].bmp1)  //取得图片的长度
				{
					iBmpLen = items[index].bmp1->width;
				}
				else
				{
					iBmpLen = m_iItemWidth;
				}
				m_DC.VD_TextOut(CRect(items[index].x_offset - (strWidth - iBmpLen )/2, items[index].y_offset + m_iItemWidth-18, items[index].x_offset + (strWidth+ iBmpLen)/2, items[index].y_offset+m_iItemWidth + TEXT_HEIGHT-18),items[index].string.c_str());
			}
		}
	}
	m_DC.UnLock();
}

void CMenu::Select(VD_BOOL flag)
{
	SetFlag(IF_SELECTED, flag);
}

int CMenu::GetAt(int px, int py)
{
	int i;
    if(PtInRect(m_Rect, px, py))
	{		
		if(m_dwStyle & styleVertical)
		{
			if(title.size() && num == 1)
			{
				//return 0;
			}
			for (i = 0; i < num; i++)
			{
	
				if(!items[i].sprtr && py >= m_Rect.top + items[i].y_offset && py <  m_Rect.top + items[i].y_offset + m_iItemHeight)
				{
					return i;
				}
			}
		}
		else if(m_dwStyle & styleHorizontal)
		{
			for (i = 0; i < num; i++)
			{
				/*
				if(!items[i].sprtr && px >= m_Rect.left + items[i].x_offset && px <  m_Rect.left + items[i].x_offset + m_iItemWidth)
				{
					return i;
				}
				*/

				//gg edit   2001/5/09
				if(!items[i].sprtr && px >= m_Rect.left + items[i].x_offset && px <  m_Rect.left + items[i].x_offset + items[i].bmp1->width)
				{
					return i;
				}
			}
		}
		else if(m_dwStyle & styleNavigation)
		{
			int temp = -1;
			if(title.size() && num == 1)
			{
				return 0;
			}
			for (i = 0; i < num; i++)
			{
				if(!items[i].sprtr 
					&& py >= m_Rect.top + items[i].y_offset 
					&& py <  m_Rect.top + items[i].y_offset + m_iItemHeight
					&& px >= m_Rect.left + items[i].x_offset
					&& px <  m_Rect.left + items[i].x_offset + m_iItemWidth
				 )
				{
					temp = i;
					break;
				}
				else
				{
					temp = -1;
				}
			}
			return temp;
		}
		else
		{
		}
	}
	return cursel;              
}


int CMenu::GetCurSel()
{
	return cursel;
}

void CMenu::SetCurSel(int sel)
{
	if(sel==cursel){
		return;
	}
	int temp = cursel;
	if(sel<0 || sel>num-1)
	{
		cursel = -1;
	}
	else
	{
		cursel = sel;
	}
	DrawSingle(temp);
	DrawSingle(cursel);
	if(m_pParent)
	{
		if(cursel >= 0 && items[cursel].tip.c_str())
		{
			if( items[cursel].bmp3)
			{
				m_pParent->ShowTip(items[cursel].tip.c_str(),items[cursel].bmp3);
			}
			else
			{
				m_pParent->ShowTip(items[cursel].tip.c_str());
			}
		}
		else
		{
            m_pParent->ShowTip(NULL, NULL);
		}
	}
}

//xym gui 增加显示提示信息
void CMenu::DrawTip(int sel)
{
	if ( sel > num-1 )
	{
		return;
	}

	if ((!(m_dwStyle & stylePopup)) && (m_pParent != NULL))
	{
		if(sel  < 0)
		{
			m_pParent->DrawTip(NULL);
		}
		else if(items[sel].tip.size())
		{
			m_pParent->DrawTip(items[sel].tip.c_str());			
		}
		move_sel = sel;					
	}		
	
}

VD_BOOL CMenu::MsgProc(uint msg, uint wpa, uint lpa)
{
	uchar key;
	int px,py,temp;
	CMenu* p = this;

	switch(msg)
	{
	case XM_KEYDOWN:
		key = wpa;
		switch(key){
		case KEY_RET:
		case KEY_RIGHT:
			if(key == KEY_RIGHT && !(m_dwStyle & styleDeskTop) && GetFlag(IF_POPUP))
			{
				key = KEY_DOWN;
				goto proc_down;
			}
			else 	if(!GetFlag(IF_POPUP) && cursel != -1 && items[cursel].sub && items[cursel].sub->GetFlag(IF_OPENED))
			{
				return items[cursel].sub->MsgProc(msg,wpa,lpa);
			}
			else
			{
				temp = cursel;
				goto proc_exec;
			}

		case KEY_ESC:
		case KEY_LEFT:
			if(key == KEY_LEFT && !(m_dwStyle & styleDeskTop) && GetFlag(IF_POPUP))
			{
				key = KEY_UP;
				goto proc_up;
			}
			if(!GetFlag(IF_POPUP) && cursel != -1 && items[cursel].sub && items[cursel].sub->GetFlag(IF_OPENED))
			{
				return items[cursel].sub->MsgProc(msg,wpa,lpa);
			}

			if(cursel != -1 && items[cursel].sub && items[cursel].sub->GetFlag(IF_OPENED))
			{
				items[cursel].sub->Close();
				int temp1 = cursel;
				cursel = -1;
				SetCurSel(temp1);
			}
			else if(!GetFlag(IF_POPUP))//主菜单ESC键不处理
			{
				int temp2 = cursel;
				cursel = -1;
				SetCurSel(temp2);
				return FALSE;
			}
			if(key == KEY_ESC || pParentMenu)
			{
				Close();
				if(key == KEY_ESC && pParentMenu && !pParentMenu->GetFlag(IF_POPUP))
				{
					pParentMenu->clicked = FALSE;
				}
				if(pParentMenu)
				{
					m_pParent->SetSelectItem(pParentMenu);
				}
				if(key == KEY_LEFT && pParentMenu && !pParentMenu->GetFlag(IF_POPUP))//如果子菜单不响应向左键值，则传给父菜单处理
				{
					pParentMenu->MsgProc(XM_KEYDOWN, KEY_LEFT, 0);
				}
			}
			if(key == KEY_ESC && !(m_dwStyle & styleDeskTop) && GetFlag(IF_POPUP))
			{
				return  m_pParent->MsgProc(msg, wpa, lpa);
			}
			break;

		case KEY_UP:
			if(GetFlag(IF_POPUP) && !(m_dwStyle & styleDeskTop) )
			{
				return pParentMenu->MsgProc(msg, wpa, lpa);
			}
proc_up:
			if(cursel < 0 || cursel >= num){
				temp = num - 1;
			}else{
				temp = cursel - 1;
			}
			do{
				//add by xie
				if((0 == temp) &&(strcmp(items[temp].string.c_str(), (char*)LOADSTR("pgquick.sysmenu")) == 0))
				{
					temp = num - 1;
				}
				//end
				
				if(temp < 0){
					temp = num - 1;
				}
				if(!items[temp].sprtr){
					break;
				}
				
				temp--;
			}while(temp != cursel);
			if(temp != cursel){
				if(cursel != -1 && items[cursel].sub && items[cursel].sub->GetFlag(IF_OPENED))
				{
					items[cursel].sub->Close();
				}
				SetCurSel(temp);
				if(!GetFlag(IF_POPUP)/* && clicked*/){
					key = KEY_RIGHT;
					goto proc_exec;
				}
			}
			break;

		case KEY_DOWN:
			if(GetFlag(IF_POPUP) && !(m_dwStyle & styleDeskTop))
			{
				return pParentMenu->MsgProc(msg, wpa, lpa);
			}
proc_down:
			if(cursel < 0 || cursel >= num){
				temp = 0;
			}else{
				temp = cursel + 1;
			}
			do{
				if(temp >= num){
					temp = 0;
				}

				//add by xie
				if((0 == temp) &&(strcmp(items[temp].string.c_str(), (char*)LOADSTR("pgquick.sysmenu")) == 0))
				{		
					temp = 1;
				}
				//end
				
				if(!items[temp].sprtr){
					break;
				}				
				temp++;
			}while(temp != cursel);
			if(temp != cursel){
				if(cursel != -1 && items[cursel].sub && items[cursel].sub->GetFlag(IF_OPENED))
				{
					items[cursel].sub->Close();
				}
				SetCurSel(temp);
				if(!GetFlag(IF_POPUP)/* && clicked*/){
					key = KEY_RIGHT;
					goto proc_exec;
				}
			}
			break;

		default:
			return FALSE;
		}
		break;
	case XM_LBUTTONDOWN:
	case XM_MOUSEMOVE:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		key = 0;
		temp = GetAt(px, py);

		//add by xie
		if((0 == temp) &&(strcmp(items[temp].string.c_str(), (char*)LOADSTR("pgquick.sysmenu")) == 0))
		{
			return TRUE;
		}
		//end
		// 非自动展开不处理
		if (!m_autoSpread && msg == XM_MOUSEMOVE)
		{
			return TRUE;
		}


//		printf("mousemove tmp = %d, cur = %d\n", temp, cursel);

		if(temp == -1 && cursel != -1)
		{
			SetCurSel(cursel);
			return TRUE;
		}
		//弹出与关闭子菜单
		if(temp != cursel || (!GetFlag(IF_POPUP) && msg == XM_LBUTTONDOWN))
		{
			if(temp != cursel && cursel != -1 && items[cursel].sub && items[cursel].sub->GetFlag(IF_OPENED))
			{
				items[cursel].sub->Close();
			}
proc_exec:
			SetCurSel(temp);//设置当前选中项,-1表示不选中任何项
			if(temp >= 0)
			{
				VD_RECT full_rect;
				if(items[temp].sub && !items[temp].sub->GetFlag(IF_OPENED))
				{
					m_pParent->GetRect(&full_rect);
					int x = 0;
					int y = 0;

					if(m_dwStyle & styleDeskTop)
					{
						if(m_Rect.right + items[temp].sub->m_Rect.Width() <= full_rect.right - 4)
						{
							x = m_Rect.right;
						}
						else
						{
							x = m_Rect.left - items[temp].sub->m_Rect.Width();
						}

						y = m_Rect.top + items[temp].y_offset;
						if(y + items[temp].sub->m_Rect.Height() > full_rect.bottom )
						{
							y = (full_rect.bottom )- items[temp].sub->m_Rect.Height();
						}
					}
					else
					{
						///x = m_Rect.left - 40;
						///y = m_Rect.top;
						//xym GUI//设置子菜单的显示起始坐标
						CRect rect;
						items[temp].sub->GetRect(rect);
						x = m_Rect.left + (m_Rect.Width() - rect.Width())/2; 
						y = m_Rect.top+100;///y = m_Rect.top+80;
					}

					items[temp].sub->TrackPopup(x, y);
				//	if(!(m_dwStyle & styleDeskTop))
				//	{
				//		items[temp].sub->SetCurSel(0);
				//	}
					if(msg == XM_KEYDOWN){
						items[temp].sub->SetCurSel(0);
					}
					break;
				}
			}
		}
		//执行菜单项上面的操作
		if(msg == XM_MOUSEMOVE || (msg == XM_KEYDOWN && key == KEY_RIGHT))
		{
			if(msg != XM_MOUSEMOVE && pParentMenu && !pParentMenu->GetFlag(IF_POPUP))
			{//如果子菜单不响应向右键值，则传给父菜单处理
				pParentMenu->MsgProc(XM_KEYDOWN, KEY_RIGHT, 0);
			}
			break;
		}
		if(temp >= 0 && !items[temp].sub)
		{
			while(p->pParentMenu)
			{
				p = p->pParentMenu;
			}
			if(p->GetFlag(IF_POPUP))         //增加条件，否则进入页面后不能返回
			{
				p->Close();
			}
		}
		if(temp >= 0 && items[temp].child && m_pParent)
		{
			m_pCursor->SetCursor(CR_BUSY);
			items[temp].child->Open();
			m_pCursor->SetCursor(CR_NORMAL);
		}
		if(temp >= 0 && items[temp].proc && m_pParent)
		{
			m_pCursor->SetCursor(CR_WAIT);
			(m_pParent->*items[temp].proc)();
			m_pCursor->SetCursor(CR_NORMAL);
		}
		break;
	//add by xie 处理用户在右键菜单上的双击左键的消息
	case XM_LBUTTONDBLCLK:
		return TRUE;
	//end
	default:
		return FALSE;
	}

	return TRUE;
}

void CMenu::ApendItem(VD_PCSTR psz, CTRLPROC vproc, VD_BITMAP* icon1, VD_BITMAP* icon2, VD_BITMAP* icon3, VD_PCSTR tip)
{
	if(num >= MENU_MAX)
	{
		return; 
	}

	if(psz)
	{
		items[num].string = GetParsedString(psz);
	}
	else
	{
		items[num].string.erase();
	}

	items[num].proc = vproc;
	items[num].bmp1 = icon1;
	items[num].bmp2 = icon2;
	items[num].bmp3 = icon3;

	if(tip)
	{
		items[num].tip = GetParsedString(tip);
	}
	SetItemArray();
}

void CMenu::ApendItem(VD_PCSTR psz, CMenu *pMenu, VD_BITMAP* icon1, VD_BITMAP* icon2, VD_BITMAP* icon3, VD_PCSTR tip)
{
	if(num >= MENU_MAX)
	{
		return;
	}

	if(psz)
	{
		items[num].string = GetParsedString(psz);
	}
	else
	{
		items[num].string.erase();
	}
	items[num].sub = pMenu;
	items[num].bmp1 = icon1;
	items[num].bmp2 = icon2;
	items[num].bmp3 = icon3;
	if(tip)
	{
		items[num].tip = GetParsedString(tip);
	}
	items[num].sub->pParentMenu = this;

	SetItemArray();
}

void CMenu::ApendItem(VD_PCSTR psz, CPage *pPage, VD_BITMAP* icon1, VD_BITMAP* icon2, VD_BITMAP* icon3, VD_PCSTR tip)
{
	if(num >= MENU_MAX)
	{
		return;
	}

//	printf("ApendItem 1\n");

	if(psz)
	{
//		printf("ApendItem 2\n");
		items[num].string = GetParsedString(psz);
	}
	else
	{
		items[num].string.erase();
	}

//	printf("ApendItem 3\n");

	items[num].child = pPage;
	items[num].bmp1 = icon1;
	items[num].bmp2 = icon2;
	items[num].bmp3 = icon3;

	if(tip)
	{
//		printf("ApendItem 4\n");
		items[num].tip = GetParsedString(tip);
	}

//	printf("ApendItem 5\n");
	SetItemArray();
//	printf("ApendItem 6\n");
}

void CMenu::ApendItem(VD_BITMAP* icon1)
{
	VD_RECT rect = m_Rect;

	if(num >= MENU_MAX)
	{
		return;
	}

	items[num].sprtr = TRUE;
	if(icon1)
	{
		items[num].bmp1 = icon1;
	}
	if(m_dwStyle & styleVertical)
	{
		if(icon1)
		{
			items[num].y_offset = y_offset;
			items[num].x_offset = x_offset;
			y_offset += m_iItemHeight;

			//校正菜单区域
			rect.bottom = rect.top + y_offset + m_iySpace;
		}
		else
		{
			items[num].y_offset = y_offset;
			items[num].x_offset = x_offset;
			y_offset += 8;

			//校正菜单区域
			rect.bottom = rect.top + y_offset + MENU_SPLT_WIDTH;
		}		
	}
	else if(m_dwStyle & styleHorizontal)
	{
		if(icon1)
		{
			items[num].x_offset = x_offset;
			items[num].y_offset = y_offset;
			x_offset += m_iItemWidth;
		}
		else
		{
			items[num].x_offset = x_offset;
			items[num].y_offset = y_offset;
			x_offset += MENU_SPLT_WIDTH;
		}
	}
	else if(m_dwStyle & styleNavigation)
	{
		SetIconArray(num, items);
	}
	else
	{
	}

	num++;
	if(!(m_dwStyle & styleNavigation))
	{
		SetRect(&rect, TRUE);
	}
}

void CMenu::RemoveAllItems()
{
	VD_RECT rect = m_Rect;

	cursel = -1;
	num = 0;
	if(title.size())
	{
		y_offset = MENU_TITLE_HIGHT;
	}
	else
	{
		y_offset = m_iySpace;
	}
	x_offset = m_ixSpace;

	rect.right = rect.left + x_offset;
	rect.bottom = rect.top + y_offset;

	SetRect(&rect, TRUE);
}

VD_BOOL CMenu::SetCustomPosition(int sel, const VD_POINT &pt)
{
	if (sel < num)
	{
		items[sel].x_offset = pt.x;
		items[sel].y_offset = pt.y;
		return TRUE;
	}
	return FALSE;
}

VD_BOOL CMenu::SetCustomColumnSize(const VD_SIZE &size)
{
	m_customSize = size;
	return TRUE;
}

VD_BOOL CMenu::SetCustomInterval(const VD_SIZE &size)
{
	m_customInterval = size;
	return TRUE;
}

void CMenu::TrackPopup(int px, int py)
{
	Close();
	m_pParent->SetSelectItem(this);
	VD_RECT full_rect;

	if(isRevPopup)
	{
		m_pParent->GetRect(&full_rect);
		full_rect.bottom -= 116;
	}
	else
	{
		m_pDevGraphics->GetRect(&full_rect);
	}
	

	if(px + m_Rect.Width() > full_rect.right)
	{
		px = full_rect.right - m_Rect.Width();
	}
	/*old code
	if(py + m_Rect.Height() > full_rect.bottom)
	{
		py = full_rect.bottom - m_Rect.Height();
	}*/

	//new code
	if(py + m_Rect.Height() > full_rect.bottom + MENU_TITLE_HIGHT/2)
	{
		py = full_rect.bottom - m_Rect.Height() -MENU_TITLE_HIGHT/2;
	}

	if(title.size() && num < 2)
	{
		if(isRevPopup)
		{
			m_Rect.bottom = full_rect.bottom;
			m_Rect.top = m_Rect.bottom - MENU_TITLE_HIGHT-10;
		}
		else
		{
			m_Rect.bottom = m_Rect.top + MENU_TITLE_HIGHT + 8;
		}
		
	}
	m_Rect.OffsetRect(px - m_Rect.left, py - m_Rect.top);
	cursel = -1;
	SetRect(m_Rect, FALSE);
	CItem::Open();
}

VD_BOOL CMenu::Close()
{
	if(!GetFlag(IF_OPENED)){
		return FALSE;
	}
	CItem::Close();
	if(cursel != -1 && items[cursel].sub){//关闭子菜单
		items[cursel].sub->Close();
	}
	if(GetFlag(IF_POPUP))
	{
		m_pParent->SetSelectItem(NULL);
	}
	else
	{
		clicked = FALSE;
		cursel = -1;
	}
	return TRUE;
}

void CMenu::TrackSubMenu()
{
	if(!GetFlag(IF_POPUP) && cursel != -1 && items[cursel].sub && !items[cursel].sub->GetFlag(IF_OPENED))
	{
		CRect full_rect;
		m_pDevGraphics->GetRect(&full_rect);
		int x, y;
		x = m_Rect.left + cursel*PICT_ITEM_WITTH ;
		if(x < full_rect.left)
		{
			x = full_rect.left;
		}
		if(x + items[cursel].sub->m_Rect.Width() > full_rect.right)
		{
			x = full_rect.right - items[cursel].sub->m_Rect.Width();
		}
	    if(isRevPopup)
		{
			y = full_rect.bottom - 50 - m_Rect.Height();
		}
		else
		{
			y = m_Rect.bottom;
		}
		
		items[cursel].sub->TrackPopup(x, y);
		items[cursel].sub->SetCurSel(0);
		clicked = TRUE;
	}
}

//设置菜单项的回调函数
void CMenu::SetItemCallback(int index, CTRLPROC vproc)
{
	if(index<0 || index >= num)
	{
		return;
	}

	items[index].proc = vproc;
}

//设置菜单项文本
void CMenu::SetItemText(int index, VD_PCSTR psz, VD_BOOL redraw /* = FALSE */)
{
	VD_RECT rect = m_Rect;

	if(index<0 || index >= num)
	{
		return;
	}

	if(psz)
	{
		items[index].string = psz;
	}
	else
	{
		items[index].string.erase();
	}

	if(m_dwStyle & styleVertical)
	{
		SetItemRight(&rect);
	}

	SetRect(&rect, redraw);
}

//设置菜单项图片 add langzi 2009-11-9 
void CMenu::SetItemBmp(int index, VD_BITMAP* icon1/* = NULL*/, VD_BITMAP* icon2/* = NULL*/, VD_BITMAP* icon3 /*= NULL*/, VD_BOOL redraw /*= FALSE*/) //设置一个相的图片add langzi 2009-11-9 
{
    VD_RECT rect = m_Rect;

	if(index<0 || index >= num)
	{
		return;
	}
    items[index].bmp1 = icon1;
	items[index].bmp2 = icon2;
	items[index].bmp3 = icon3;
	
	if(m_dwStyle & styleVertical)
	{
	   SetItemRight(&rect);
	}
	SetRect(&rect, redraw);
}

int CMenu::GetCount()
{
	return num;
}

VD_BOOL CMenu::GetItem(int index, MENU_ITEM* pItem)
{
	if(index < 0 || index >= num || pItem == NULL)
	{
		return FALSE;
	}
	
	MENU_ITEM& item = items[index];

	pItem->bmp1 = item.bmp1;
	pItem->bmp2 = item.bmp2;
	pItem->bmp3 = item.bmp3;
	pItem->string = item.string;
	pItem->tip = item.tip;
	pItem->sub = item.sub;
	pItem->proc = item.proc;
	pItem->child = item.child;
	pItem->sprtr = item.sprtr;
	pItem->y_offset = item.y_offset;
	pItem->x_offset = item.x_offset;

	return TRUE;
}

void CMenu::SetIconArray(int number, MENU_ITEM *pitem)
{
	int colNum[12]={1,2,3,2,3,3,4,4,4,5,4,4};
	if(m_iConstColNum)
	{
		m_iColNum = m_iConstColNum;
	}
	else
	{
		m_iColNum = colNum[number];
	}

	// 计算菜单行数
	m_iRowNum = (number + m_iColNum) / m_iColNum;
	int clumn_width = m_customSize.w;
	int row_height =  m_customSize.h;

	if (clumn_width < 0)
	{
		clumn_width = (m_Rect.Width()  - CLIENT_X/*- 2*CLIENT_X*/) / m_iColNum;
	}
	if (row_height < 0)
	{
		row_height = (m_Rect.Height() - 2*CLIENT_Y) / m_iRowNum;
	}
	
	int offset_x = m_customInterval.w;
	int offset_y = m_customInterval.h;

	if (offset_x < 0)
	{
		///offset_x = (clumn_width - m_iItemWidth) / 2;
		//xym GUI
		if(m_dwStyle & stylePopup)
		{
			offset_x = (clumn_width - (m_iItemWidth-20)) / 2;
		}
		else
		{
			offset_x = (m_Rect.Width()-(m_iItemWidth-5)*m_iColNum)/2; //xym GUI  
		}
		//end		
	}
	if (offset_y < 0)
	{
		///offset_y = m_IFDTop ? (m_Rect.Height() - m_iItemHeight - CLIENT_Y) : (CLIENT_Y  + (row_height - m_iItemHeight) / 2);
		offset_y = m_IFDTop ? (0) : (CLIENT_Y  + (row_height - m_iItemHeight) / 2); //xym GUI //主菜单放在最上面			
	}
	
	int i,j;
	for(i=0;i<m_iRowNum;i++)
	{
		for(j=0;j<m_iColNum;j++)
		{
			pitem[i * m_iColNum + j].x_offset  = clumn_width*j + offset_x;			
			pitem[i * m_iColNum + j].y_offset  = row_height*i + offset_y;
		}
	}
}

void CMenu::SetItemHeight(int itemHeight)
{
	m_iItemHeight = itemHeight;
	return;
}
void CMenu::SetItemWidth(int itemWidth)
{
	m_iItemWidth = itemWidth;
	return;
}

void CMenu::SetYSpace(int ySpace)
{
	m_iySpace = ySpace;
	if(!title.size())
	{
		y_offset = m_iySpace;
	}
	return;
}
void CMenu::SetXSpace(int xSpace)
{
	m_ixSpace = xSpace;
	if(!(m_dwStyle & styleVertical))
	{
		x_offset = m_ixSpace;
	}	
	return;
}

void CMenu::SetItemRight(VD_RECT* pRect)
{
	if(m_dwStyle & styleVertical)
	{
		if(0 == m_iItemWidth)
		{
			int width = 0;
			if(items[num].bmp1)
			{
				//width = g_Locales.GetTextExtent(items[num].string.c_str()) + items[num].bmp1->width + TEXT_WIDTH*3; //delete langzi 2009-11-9 
				width = g_Locales.GetTextExtent(items[num].string.c_str()) + items[num].bmp1->width + TEXT_WIDTH*3 - 28; //add langzi 2009-11-9 右击菜单提前 
			}
			else if(title.size() && num < 2)
			{
				width = g_Locales.GetTextExtent(items[num].string.c_str()) + TEXT_WIDTH*4;
			}
			else
			{
				width = g_Locales.GetTextExtent(items[num].string.c_str()) + TEXT_WIDTH*2;
			}
			if((pRect->left  + width) > pRect->right)
			{
				pRect->right = pRect->left + width;
			}
		}
		else
		{
			pRect->right = pRect->left + m_iItemWidth;
		}
	}
	return;
}

void CMenu::SetMenuBmp(VD_BITMAP* bkNor, VD_BITMAP* singleNor, VD_BITMAP* singleSel, VD_BOOL redraw /*= TRUE*/)
{
	if(bkNor)
	{
		m_bmpBKNormal = bkNor;
	}
	if(singleNor)
	{
		m_bmpSingleNormal = singleNor;
	}
	if(singleSel)
	{
		m_bmpSingleSelect = singleSel;
	}
		
	if(redraw)
	{
		Draw();
	}
	return;
}
void CMenu::SetMenuColor(MENU_COLOR_TYPE colorType, VD_COLORREF color, VD_BOOL redraw/* = TRUE*/)
{
	switch(colorType)
	{
	case colBKNormal:
		m_colBKNormal = color;
		break;
	case colBKTitle:
		m_colBKTitle = color;
		break;
	case colSingleNormal:
		m_colSingleNormal = color;
		break;
	case colSingleSelect:
		m_colSingleSelect = color;
		break;
	case colFontNormal:
		m_colFontNormal = color;
		break;
	case colFontSelect:
		m_colFontSelect = color;
		break;
	case colFontTitle:
		m_colFontTitle = color;
		break;
	case colFrameNormal:
		m_colFrameNormal = color;
		break;
	case colFrameSelect:
		m_colFrameSelect = color;
		break;
	case colBKFrame:
		m_colBKFrame = color;
		break;
	default:
		break;
	}
	if(redraw)
	{
		Draw();
	}
}

void CMenu::SetItemArray(void)
{
	VD_RECT rect = m_Rect;
	if(m_dwStyle & styleVertical)
	{
		items[num].y_offset = y_offset;
		items[num].x_offset = x_offset;
		y_offset += m_iItemHeight;

		//校正菜单区域
		rect.bottom = rect.top + y_offset + m_iySpace;
		//printf("SetItemArray 1\n");
		SetItemRight(&rect);
	}
	else if(m_dwStyle & styleHorizontal)
	{
		items[num].x_offset = x_offset;
		items[num].y_offset = y_offset;
		if(num == 0)
		{
			x_offset += 336;	
		}
		else
		{
			x_offset += items[num].bmp1->width + 4;	

		}

	}
	else if(m_dwStyle & styleNavigation)
	{
		//printf("SetItemArray 2\n");
		SetIconArray(num, items);
	}
	else
	{
	}

	//printf("SetItemArray 3\n");
	num++;
	if(!(m_dwStyle & styleNavigation))
	{
		SetRect(&rect, TRUE);
	}
	//printf("SetItemArray 4\n");
	return;
}

void CMenu::SetNavColumnNum(int columnNum)
{
	if(m_dwStyle & styleNavigation)
	{
		m_iConstColNum = columnNum;
	}
}

void CMenu::SetFrameWidth(int width, VD_BOOL redraw /*= FALSE*/)
{
	if(width != m_iFrameWidth)
	{
		m_iFrameWidth = width;
	}
	if(redraw)
	{
		Draw();
	}
}

void CMenu::SetAutoSpread(VD_BOOL autoSpread)
{
	m_autoSpread = autoSpread;
}

void CMenu::SetDefaultBitmap(MenuBitmap region, VD_PCSTR name)
{
	switch(region)
	{
	case MNB_PANEL:
		m_bmpPanel = VD_LoadBitmap(name);
		break;
	case MNB_TITLED_PANEL:
		m_bmpTitledPanel = VD_LoadBitmap(name);
		break;
	case MNB_BAR:
		m_bmpBar = VD_LoadBitmap(name);
		break;
	case MNB_ITEM_NORMAL:
		m_bmpItemNormal = VD_LoadBitmap(name);	
		break;
	case MNB_ITEM_SELECTED:
		m_bmpItemSelected = VD_LoadBitmap(name);
		break;
	case MNB_ITEM_DISABLED:
		m_bmpItemDisabled = VD_LoadBitmap(name);
		break;
	default:
		break;
	}
}


