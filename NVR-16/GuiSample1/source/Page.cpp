

#include "GUI/Ctrls/Page.h"
#include "System/Locales.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageSearch.h"
#include "GUI/Pages/PageMessageBox.h"
#include "GUI/Pages/PagePtzCtl.h"

VD_BITMAP* CPage::bmp_key_pageup;
VD_BITMAP* CPage::bmp_key_pagedown;
VD_BITMAP* CPage::bmp_key_function;
VD_BITMAP* CPage::bmp_key_enter;
VD_BITMAP* CPage::bmp_button_close[4];
VD_BITMAP* CPage::icon_dvr;


CPage::PAGE_MAP CPage::m_existentPages[N_GUI_CORE];
int				CPage::sm_layoutSpace[spaceCount] = {6};

///////////////////////////////////////////////////////////////////////////////
/////////             CPage

CPage::CPage(VD_PCRECT pRect, VD_PCSTR psz,CPage *pParent /* = NULL */, uint style /* = 0 */)
	: CItem(pRect, pParent, IT_PAGE, stylePopup|styleEditable|styleAutoFocus|style)
{
//	printf("CPage:CPage start\n");
	
//	printf("CPage:CPage 111\n");
	
	m_playoutSpace = sm_layoutSpace;
 	m_ifPageMain = FALSE;
	bmp_frame_title = VD_LoadBitmap(DATA_DIR"/bmp_frame_title.bmp");
	bmp_frame_mid = VD_LoadBitmap(DATA_DIR"/bmp_frame_mid.bmp");
	bmp_frame_bottom = VD_LoadBitmap(DATA_DIR"/bmp_frame_bottom.bmp");
	
//	printf("CPage:CPage 222\n");
	
	m_pIconBmp = icon_dvr;
	m_pNorBmp =  NULL;
	m_pSelBmp = NULL;
	m_iSpace = 10;
	m_iStartX = CLIENT_X;
	m_iStartY = CLIENT_Y;

	for(int i = 0; i <MAX_TABLE_NUM; i++)
	{
		m_iTable[i] = 0;
	}
	
	m_nAge = -1;
	m_iPage = 0;
	m_nPage = 0;
	m_bResizing = FALSE;
	m_bResizable = FALSE;

	m_pChildFocus = m_pChildFocusOld = NULL;
	m_pChildSelect = m_pChildSelectOld = NULL;
	m_pChildDflt = NULL;
	m_pChildSelect = NULL;
	m_pChildHead = m_pChildTail = NULL;

//	printf("CPage:CPage 333\n");

	if(psz)
	{
		string = GetParsedString(psz);
	}

//	printf("CPage:CPage 444\n");

	m_MutexMgr.Enter();
	m_existentPages[m_screen].insert(PAGE_MAP::value_type(string, this));
	m_MutexMgr.Leave();

//	printf("CPage:CPage 555\n");

	std::string name = "Page-" + string;
	//俄语情况下翻译超长可能导致thread name的数组m_Name[32]越界导致程序飞掉，所以进行特殊处理，其他语言下暂时没发现相应情况。
	//如果在非英文的情况下打印出的thread name是乱码，所以直接处理成"Page-Long"。并不影响其他调试。
	//2007-05-15 modified by wangqin 
	if (strlen(name.c_str()) >= 31)
	{
		name = "Page-Long";
	}
	m_pThread = new CItemThread(name.c_str(), this);
	
	m_pPopupCaller = NULL;
	
	m_bDrawBkgdOnly = FALSE;
	
//	printf("CPage:CPage 666\n");
	
	if(psz)
	{
		 m_pTitle = GetParsedString(psz);
	}
	
//	printf("CPage:CPage 777\n");

	if(pRect)
	{
		SetRect(pRect, FALSE);
	}

//	printf("CPage:CPage 888\n");

	if(m_dwStyle & pageFullScreen)
	{
		VD_RECT rect;

		m_pDevGraphics->GetRect(&rect);
//		printf("CPage:CPage 81\n");
		SetRect(&rect, FALSE);
//		printf("CPage:CPage 82\n");
		SetTitleSize(rect.right, rect.bottom);
//		printf("CPage:CPage 83\n");
		SetMargin(0, 0, 0, 0);
//		printf("CPage:CPage 84\n");
	}
	
//	printf("CPage:CPage 999\n");

	m_TitleHeight = m_Rect.Height();
	m_TitleWidth = m_Rect.Width();
	m_Margin.SetRectEmpty();
	ishPageMain = FALSE;

//	printf("CPage:CPage end\n");
}

CPage::~CPage()
{
	//printf("CPage::~CPage this=0x%08x\n",(unsigned int)this);
	//fflush(stdout);
	
	m_MutexMgr.Enter();
	m_existentPages[m_screen].erase(string);
	m_MutexMgr.Leave();
	
	if(m_pThread)
	{
		//printf("CPage::~CPage delete m_pThread\n");
		//fflush(stdout);
		
		delete m_pThread;
	}
	
	if(m_playoutSpace != sm_layoutSpace)
	{
		delete m_playoutSpace;
	}
	
	//delete all child items
	CItem *p = m_pChildHead, *temp;
	while(p)
	{
		temp = p->m_pBrother;
		delete p;
		p = temp;
	}
}

void CPage::SetMute(BOOL bMute)
{
	
}

void CPage::GetCurPlayMute(u8* mute)
{
	
}

void CPage::SetCurPlayMute(u8 mute)
{
	
}

CPage* CPage::FindPage(int screen, VD_PCSTR name, VD_BOOL mustOpened /* = FALSE */)
{
	CGuard guard(m_MutexMgr);

	PAGE_MAP::iterator pi;
	pi = m_existentPages[screen].find(GetParsedString(name));
	if(pi != m_existentPages[screen].end())
	{
		if(mustOpened && !((*pi).second)->IsOpened())
		{
			return NULL;
		}
		return (*pi).second;
	}
	else
	{
		return NULL;
	}
}

//添加项
void CPage::AddItem(CItem *pItem)
{
	if(!pItem){
		return;
	}

	if(pItem->m_pBrother || pItem->m_pBrotherR || m_pChildHead == pItem)
	{
		printf("AddItem error %p\n", pItem);
		return;
	}

	CGuard guard(m_MutexMgr);

	//	trace("add item (%d,%d,%d,%d)\n",pItem->x,pItem->y,pItem->m_Rect.Width(),pItem->m_Rect.Height());
	if(m_pChildHead == NULL){
		m_pChildHead = m_pChildTail = pItem;
	}else{
		m_pChildTail->m_pBrother = pItem;
		pItem->m_pBrotherR = m_pChildTail;
		m_pChildTail = pItem;
	}
}

//删除项
void CPage::DeleteItem(CItem *pItem)
{
	if(!pItem){
		return;
	}

	if(!pItem->m_pBrother && !pItem->m_pBrotherR && pItem != pItem)
	{
		printf("DeleteItem error %p\n", pItem);
		return;
	}

	CGuard guard(m_MutexMgr);

	if(m_pChildHead == pItem)
	{
		m_pChildHead = m_pChildHead->m_pBrother;
	}
	else
	{
		pItem->m_pBrotherR->m_pBrother = pItem->m_pBrother;
	}

	if(m_pChildTail == pItem)
	{
		m_pChildTail = m_pChildTail->m_pBrotherR;
	}
	else
	{
		pItem->m_pBrother->m_pBrotherR = pItem->m_pBrotherR;
	}

	pItem->m_pBrotherR = NULL;
	pItem->m_pBrother = NULL;
}

CItem* CPage::FindStartItem()
{
	CItem *p = m_pChildHead;
	CItem *best = NULL;
	VD_RECT rect, best_rect;
	VD_BOOL flag;
	
	while(p){
		p->GetRect(&rect);
		flag = FALSE;
		if(p->GetFlag(IF_OPENED) && p->GetFlag(IF_ENABLED) && p->GetFlag(IF_EDITABLED)
			&& !p->GetFlag(IF_POPUP) && p->GetFlag(IF_SHOWN) && p->GetFlag(IF_ORDERED)){
			if(best){
				best->GetRect(&best_rect);
				if(rect.top <= best_rect.bottom
					&& best_rect.top <= rect.bottom){//在同一行，判断x坐标
					flag = rect.left < best_rect.left;
				}else{//不在同一行，判断y坐标
					flag = rect.top < best_rect.top;
				}
			}
			if(!best || flag){
				best = p;
			}
		}
		p = p->m_pBrother;
	}
	return best;
}

void CPage::SetDefaultItem(CItem *pItem)
{
	m_pChildDflt = pItem;
}

CItem * CPage::GetFocusItem()
{
	return m_pChildFocus;
}

void CPage::SetFocusItem(CItem *pItem)
{
	if(m_pChildFocus != pItem){
		if(m_pChildFocus){
			m_pChildFocus->SetFocus(FALSE);
			m_pChildFocusOld = m_pChildFocus;
		}
		if(pItem){
			pItem->SetFocus(TRUE);
		}
		m_pChildFocus = pItem;
		UpdateInputTray();
	}
}

CItem* CPage::GetSelectItem()
{
	return m_pChildSelect;
}

//设置选中项，同时处理自动聚焦
void CPage::SetSelectItem(CItem *pItem)
{
	if(m_pChildSelect != pItem){
		
		if(m_pChildSelect){
			if(m_pChildSelect->GetFlag(IF_AUTOFOCUS) && m_pChildSelect == m_pChildFocus)
			{
				m_pChildSelect->SetFocus(FALSE);
				m_pChildFocusOld = m_pChildFocus;
			}
			m_pChildSelect->Select(FALSE);
			m_pChildSelectOld = m_pChildSelect;
		}
		
auto_focus:

		#if 1
		if(pItem){
			//printf("pItem->GetFlag(IF_AUTOFOCUS) \n");
			if(pItem->GetFlag(IF_AUTOFOCUS))
			{
				if(m_pChildFocus)
				{
					m_pChildFocus->SetFocus(FALSE);
				}
				pItem->SetFocus(TRUE);
			}
			//printf("item tpye = %d \n",pItem->m_iType);

			if(pItem->m_iType != IT_PAGE)
			{
				pItem->Select(TRUE);
			}
				
		}	
		#endif
		
		if(m_pChildSelect == m_pChildFocus && m_pChildSelect && m_pChildSelect->GetFlag(IF_AUTOFOCUS))
		{
			m_pChildFocus = NULL;
		}
		if(pItem && pItem->GetFlag(IF_AUTOFOCUS))
		{
			m_pChildFocus = pItem;
		}
		UpdateInputTray();
		m_pChildSelect = pItem;
	}
	else if(pItem && pItem->GetFlag(IF_AUTOFOCUS) && m_pChildFocus != pItem)
	{
		goto auto_focus;
	}
}

//类似文本编辑的方式来移动控件
//左右键在行内移动，到行首或行末后换行
//上下键移动到上一行或下一行的同一x坐标的控件，
//或者那一行中烤左边的x坐标最近的控件
CItem* CPage::ShiftItem(int key, CItem *pCur, CItem *pOld)
{
	if(!pCur){				//还没有指定当前项
		if(!pOld || !pOld->GetFlag(IF_OPENED) || !pOld->GetFlag(IF_ENABLED) || !pOld->GetFlag(IF_EDITABLED) || !pOld->GetFlag(IF_SHOWN)){
			return pCur;	//当前项和先前项都不满足条件,返回,并不作处理
		}
		pCur = pOld;		//先前项满足条件,先前项成为当前项
	}

	//powered distance, distance in x-axis, distance in y-axis, nearest distance
//	int dn=0;
	CItem *best = NULL;
	CItem *p = m_pChildHead;
	VD_BOOL flag;	//是否更合适
	int offs;	//x或y轴需要加的偏移
	int offsn = 0;	//最佳项加的偏移

	VD_RECT rect;
	VD_RECT cur_rect;
	VD_RECT best_rect;

	pCur->GetRect(&cur_rect);		//获得当前项所在区域
	switch(key){
	case KEY_UP:
	case KEY_PREV:
	case KEY_DOWN:
	case KEY_NEXT:
		while(p){
			p->GetRect(&rect);
			flag = FALSE;
			offs = 0;
			if(p != pCur && p->GetFlag(IF_OPENED) && p->GetFlag(IF_ENABLED) //控件有效
				&& p->GetFlag(IF_EDITABLED) && !p->GetFlag(IF_POPUP) && p->GetFlag(IF_SHOWN) && p->GetFlag(IF_ORDERED)
				&& (p->m_iType != IT_RADIO || ((CRadio*)p)->IsHead()))//是单选框组的头项
			{
				if(0){
				}else{
					if(rect.bottom < cur_rect.top){
						offs = m_Rect.Height();
					}
					if(best){
						best->GetRect(&best_rect);
						if(rect.top + offs < cur_rect.bottom
							&& cur_rect.top < rect.bottom + offs){//和当前项在同一行，失败
							flag = FALSE;
						}
						else if(rect.top + offs < best_rect.bottom + offsn
							&& best_rect.top + offsn < rect.bottom + offs){//在同一行，判断x坐标

							if(rect.left < cur_rect.right && cur_rect.left < rect.right)
							{
								if(best_rect.left < cur_rect.right && cur_rect.left < best_rect.right)
								{
									flag = (rect.left < best_rect.left);
								}
								else
								{
									flag = TRUE;
								}
							}
							else if(best_rect.left < cur_rect.right && cur_rect.left < best_rect.right)
							{
								flag = FALSE;
							}
							else if(rect.left < cur_rect.left)
							{
								if(best_rect.left > cur_rect.left)
								{
									flag = TRUE;
								}
								else
								{
									flag = (rect.left > cur_rect.left);
								}
							}
							else
							{
								if(best_rect.left < cur_rect.left)
								{
									flag = FALSE;
								}
								else
								{
									flag = (rect.left < cur_rect.left);
								}
							}

						}else{//不在同一行，判断y坐标
							flag = ((key==KEY_DOWN || key==KEY_NEXT)?
								(rect.top + offs < best_rect.top + offsn)
								:(rect.top + offs > best_rect.top + offsn));
						}
					}
					if((!best
						&& (rect.top + offs > cur_rect.bottom || cur_rect.top > rect.bottom + offs))//和当前项不在同一行
						|| flag){
						best = p;
						offsn = offs;
					}
				}
			}
			p = p->m_pBrother;
		}
		break;
	case KEY_LEFT:
		if(key == KEY_LEFT)
		{
			p = pCur;
			while (p->m_pRelatedItems && p->m_pRelatedItems->m_pLeftRelated)//关联控件存在
			{
				p = p->m_pRelatedItems->m_pLeftRelated;
				if(p->GetFlag(IF_OPENED) && p->GetFlag(IF_ENABLED))
				{
					return p;
				}
			}
		}
	case KEY_RIGHT:
		if(key == KEY_RIGHT)
		{
			p = pCur;
			while (p->m_pRelatedItems && p->m_pRelatedItems->m_pRightRelated)//关联控件存在
			{
				p = p->m_pRelatedItems->m_pRightRelated;
				if(p->GetFlag(IF_OPENED) && p->GetFlag(IF_ENABLED))
				{
					return p;
				}
			}
		}
		p = m_pChildHead;
		while(p){
			p->GetRect(&rect);
			flag = FALSE;
			offs = 0;
			if(p != pCur && p->GetFlag(IF_OPENED) && p->GetFlag(IF_ENABLED) //控件有效
				&& p->GetFlag(IF_EDITABLED) && !p->GetFlag(IF_POPUP) && p->GetFlag(IF_SHOWN) && p->GetFlag(IF_ORDERED)
				&& (p->m_iType != IT_RADIO || ((CRadio*)p)->IsHead()))//是单选框组的头项
			{
				if(rect.top < cur_rect.bottom && cur_rect.top < rect.bottom){
					if(rect.left < cur_rect.left){
						offs = m_Rect.Height();
					}
				}else if(rect.top < cur_rect.top){
					offs = m_Rect.Height();
				}
				if(best)
				{
					best->GetRect(&best_rect);
					if(rect.top + offs < best_rect.bottom + offsn
						&& best_rect.top + offsn < rect.bottom + offs)
					{//在同一行，判断x坐标
						if (best_rect.top < cur_rect.bottom && cur_rect.top < best_rect.bottom)
						{
							flag = ((key == KEY_RIGHT)
								? ((rect.left < best_rect.left) && (rect.left > cur_rect.left))
								: ((rect.left > best_rect.left) && (rect.left < cur_rect.left)));
						}
						else
						{
							flag = ((key == KEY_RIGHT)?(rect.left < best_rect.left):(rect.left > best_rect.left));
						}
					}
					else
					{//不在同一行，判断y坐标
						flag = ((key==KEY_RIGHT)?(rect.top + offs < best_rect.top + offsn):(rect.top + offs > best_rect.top + offsn));
					}
				}
				if(!best || flag){
					best = p;
					offsn = offs;
				}
			}
			p = p->m_pBrother;
		}
		break;
	default:
		return pCur;
	}
	if(best){
		return best;
	}
	return pCur;
}

CItem * CPage::GetItemAt(int px, int py)
{
	//trace("GetItem(%d,%d)\n", px, py);
	CItem *p;
	VD_RECT rect;

	p = m_pTop[m_screen];
	while(p && p != this)
	{
		p->GetRect(&rect);
		if(p->m_pParent == this && p->GetFlag(IF_ENABLED) && p->GetFlag(IF_EDITABLED) && PtInRect(&rect, px, py) && p->GetFlag(IF_OPENED) && p->GetFlag(IF_SHOWN)){	
			return p;
		}
		p = p->m_pDown;
	}

	p = m_pChildHead;
	while(p){
		p->GetRect(&rect);
		if(!p->GetFlag(IF_POPUP) && p->GetFlag(IF_ENABLED) && p->GetFlag(IF_EDITABLED) && PtInRect(&rect, px, py) && p->GetFlag(IF_OPENED) && p->GetFlag(IF_SHOWN)){	
			return p;
		}
		p = p->m_pBrother;
	}
	return NULL;
}

VD_BOOL CPage::AdjustRect()
{
	CGuard guard(m_MutexMgr);

	VD_RECT rect;
	int age = m_pDevGraphics->GetAge();

	//年龄不一致时需要更新
	if(age == m_nAge)
	{
		return FALSE;
	}

	m_nAge = age;

	CItem *p;
	p = m_pChildHead;
	while(p)
	{
		p->m_DC.Lock();
		p->m_DC.Update();
		p->m_DC.UnLock();
		p = p->m_pBrother;
	}

	m_DC.Lock();
	m_DC.Update();
	m_DC.UnLock();

	m_pDevGraphics->GetRect(&rect);

	if(m_dwStyle & pageFullScreen)
	{
		SetRect(&rect,FALSE);//先不刷新，否则多draw一次
	}
	else if(m_dwStyle & pageAlignCenter)
	{
		if(ishPageMain)
		{
			if(0/*g_CapsEx.ForATM*/)
			{
				if((rect.right - rect.left == 704) && (rect.bottom - rect.top == 528))//解决640*480分辨率下页面超出屏幕问题
				{
					SetRect(CRect((rect.right-m_Rect.Width())/2, (rect.bottom-m_Rect.Height()-20), 
						(rect.right+m_Rect.Width())/2, (rect.bottom-20)), FALSE);
				}
				else
				{
					SetRect(CRect((rect.right-m_Rect.Width())/2, (rect.bottom-m_Rect.Height()-70), 
						(rect.right+m_Rect.Width())/2, (rect.bottom-70)), FALSE);
				}
			}
			else
			{
				
				SetRect(CRect((rect.right-m_Rect.Width())/2, (rect.bottom-m_Rect.Height()-70), 
					(rect.right+m_Rect.Width())/2, (rect.bottom-70)), FALSE);
			}
		}
		else
		{
			SetRect(CRect((rect.right-m_Rect.Width())/2, (rect.bottom-m_Rect.Height())/2, 
				(rect.right+m_Rect.Width())/2, (rect.bottom+m_Rect.Height())/2), FALSE);
		}
	}

	return TRUE;
}

void CPage::SetRect(VD_PCRECT pRect, VD_BOOL bReDraw)
{
//	printf("CPage:SetRect 1\n");
	CGuard guard(m_MutexMgr);
	
	assert(pRect);
	//save old rectangle
	VD_RECT old_rect = m_Rect;
	CRgn rgn;
	
	//set owner rectangle, clip parent rectangle
	m_Rect = *pRect;
	
	//m_DC.SetBound(pRect);
//	printf("CPage:SetRect2\n");
	if(memcmp(pRect, &old_rect, sizeof(VD_RECT)) == 0)
	{
		return;
	}
	
//	printf("CPage:SetRect 3\n");
	//set children rectangle
	CRect item_rect;
	CItem *p = m_pChildHead;
	while(p)
	{
		if(!p->GetFlag(IF_POPUP) )
		{
			if(!(p->GetOwner()&& (p->m_iType ==IT_SCROLLBAR)))
			{
				//caculate items rectangle
				p->GetRect(&item_rect);
				item_rect.OffsetRect(m_Rect.left - old_rect.left, m_Rect.top - old_rect.top);
				p->SetRect(&item_rect, FALSE);
			}
		}
		p = p->m_pBrother;
	}

//	printf("CPage:SetRect 4\n");
	rgn.CreateRectRgn(&old_rect);
//	printf("CPage:SetRect 5\n");
	rgn.SubtractRect(m_Rect);

//	printf("CPage:SetRect 6\n");
	//更新其他窗口区域
	GlobalInvalidateRect(m_Rect);
//	printf("CPage:SetRect 7\n");
	GlobalValidateRgn(&rgn);
	if(m_TitleWidth){
		m_TitleWidth = m_Rect.Width();
	}

//	printf("CPage:SetRect 8\n");
	//更新自身窗口区域
	UpdateRgn();
	
//	printf("CPage:SetRect 9\n");
	if(bReDraw)
	{
		Draw();
	}
//	printf("CPage:SetRect 10\n");
}

void CPage::Resize(int ox, int oy)
{
	VD_RECT rect;
	CDC dc;

	if(ox == 0 && oy == 0)
	{
		return;
	}
	
	m_pDevGraphics->GetRect(&rect);
	
	dc.Enable(TRUE);
	dc.Lock();
	dc.SetRop(ROP_XOR);
	//modefied by nike.xie
	//dc.SetPen(GetSysColor(COLOR_CTRLTEXTSELECTED), PS_SOLID, 2);	
	dc.SetPen(VD_GetSysColor(COLOR_POPUP), VD_PS_SOLID, 2);
	//end
	dc.SetRgnStyle(RS_HOLLOW);
	dc.Rectangle(&m_RectResizing);
	m_pCursor->SetCursorPos(m_RectResizing.left + m_iResizeXOffs, m_RectResizing.top + m_iResizeYOffs);
	m_RectResizing.OffsetRect(ox, oy);
	if(m_RectResizing.left + 32 > rect.right)
	{
		m_RectResizing.OffsetRect(rect.right - m_RectResizing.left - 32, 0);
	}
	if(m_RectResizing.top + 32 > rect.bottom)
	{
		m_RectResizing.OffsetRect(0, rect.bottom - m_RectResizing.top - 32);
	}
	if(m_RectResizing.right - 32 < rect.left)
	{
		m_RectResizing.OffsetRect(rect.left - m_RectResizing.right + 32, 0);
	}
	if(m_RectResizing.bottom - 32 < rect.top)
	{
		m_RectResizing.OffsetRect(0, rect.top - m_RectResizing.bottom + 32);
	}
	dc.Rectangle(&m_RectResizing);
	dc.SetRop(ROP_COPY);
	dc.UnLock();
	dc.Enable(FALSE);
}

void CPage::ResizeStart()
{
	m_bResizing = TRUE;
	SetFlag(IF_CAPTURED, true);
	GetRect(&m_RectResizing);
}

void CPage::ResizeEnd()
{
	m_bResizing = FALSE;
	SetFlag(IF_CAPTURED, false);
	Show(FALSE);  //new version code
	SetRect(m_RectResizing);
	Show(TRUE);   //new version code
}

//输入焦点丢失
void CPage::KillFocus()
{
	m_bLClicked = FALSE;
	m_bRClicked = FALSE;
	m_bMClicked = FALSE;
}

void CPage::Scroll(int oPage)
{
	
}

void CPage::SetTitleSize(int w, int h)
{
	m_TitleHeight = h;
	m_TitleWidth = w;
}

void CPage::SetMargin(int l, int t, int r, int b)
{
	m_Margin.SetRect(l, t, r, b);
}

CPage* CPage::GetParentPage()
{
	return m_pParent;
}

//打开页面
VD_BOOL CPage::Open()
{
	//CDevMouse* mouse1 = GetDevMouse();
	//mouse1->ClearSignal();
	
	//SetActive();
	//WaitForSem();
	
	//ReleaseSem();
	
	//this->Activate();
	//m_pActive[m_screen] = this;
	
	//printf("VD_BOOL CPage::Open()111111111111111   \n");
	//m_pCursor->Enable(FALSE);
	
	//SetSystemLockStatus(1);
	m_pCursor->SetCursor(CR_BUSY);
	if(!UpdateData(UDM_OPEN))
	{
		//update failed
		m_pCursor->SetCursor(CR_NORMAL);
		//m_pCursor->Enable(TRUE);
		//ReleaseSem();
		//SetSystemLockStatus(0);
		
		return FALSE;
	}
	m_pCursor->SetCursor(CR_NORMAL);
	
	m_MutexMgr.Enter();
	if(GetFlag(IF_OPENED))
	{
		//trace("CPage::Open(),page `%s` is already opened.\n", m_pTitle);
		//printf("page '%s' is already opened.\n",m_pTitle.c_str());
		printf("page is already opened.\n");
		m_MutexMgr.Leave();
		SetActive();// 如果已经打开，激活自己
		//m_pCursor->Enable(TRUE);
		//ReleaseSem();
		//SetSystemLockStatus(0);
		
		return FALSE;
	}
	SetFlag(IF_OPENED, TRUE);//立即将该状态置位,防止多次打开
	m_MutexMgr.Leave();
	//printf("CPage::Open 1\n");
	AdjustRect();
	//printf("CPage::Open 2\n");
	m_DC.Lock();
	m_DC.Enable(TRUE);
	m_DC.UnLock();
	
	//printf("CPage::Open 3\n");
	
	//打开所有控件
	CItem *p = m_pChildHead;
	while(p)
	{
		if(!p->GetFlag(IF_POPUP))
		{
			p->Open();
			//p->Draw();
		}
		p = p->m_pBrother;
	}
	
	m_pChildFocusOld = m_pChildSelectOld = NULL;
	
	//printf("CPage::Open 4\n");
	p = FindStartItem();
	
	//printf("CPage::Open 5\n");
	if(!p || p->GetFlag(IF_AUTOFOCUS))
	{
		m_pChildFocus = p;
	}
	m_pChildSelect = p;
	
	//printf("CPage::Open 6\n");
	
	//printf("Show(TRUE, FALSE);\n");
	Show(TRUE);
	
	//printf("CPage::Open 7\n");
	UpdateInputTray();
	//printf("CPage::Open 8\n");
	//页面完全打开后的操作，比如重新设置焦点
	//set focus state, no need draw
	if(m_pChildSelect)
	{
		if(m_pChildSelect->GetFlag(IF_AUTOFOCUS))
		{
			m_pChildSelect->SetFocus(TRUE);
		}
		m_pChildSelect->Select(TRUE);
	}
	//printf("CPage::Open 9\n");
	
	UpdateData(UDM_OPENED);
	//printf("CPage::Open 10\n");
	//delete by nike.xie 2009-08-04 删除此功能后,控件字体恢复清晰
	
	#if 0
	//对所有的静态控件做抗抖动处理
	CItem *pItem = m_pChildHead;
	
	m_pCursor->ShowCursor(FALSE);
	while(pItem)
	{
		if(pItem->m_iType == IT_STATIC)
		{
			CRect rect;
			
			pItem->GetRect(rect);
			CDevGraphics::instance()->DeFlicker(rect);
		}
		pItem = pItem->m_pBrother;
	}
	m_pCursor->ShowCursor(TRUE);
	#endif
	
	//printf("CPage::Open 11\n");
	KillFocus();
	//printf("CPage::Open 12\n");
	
	if(m_pThread && !(m_dwStyle & pageDialog))
	{
		//printf("CPage::Open CreateThread\n");
		//fflush(stdout);
		
		m_pThread->CreateThread();
	}
	//printf("CPage::Open 13\n");
	
	//m_pCursor->Enable(TRUE);
	
	//printf("VD_BOOL CPage::Open()222222222222222222  \n");
	//ReleaseSem();
	
	//SetSystemLockStatus(0);
	
	//GraphicsSyncVgaToCvbs();//csp modify
	
	//csp modify
	if(CDC::GetModify())
	{
		GraphicsSyncVgaToCvbs();
		CDC::SetModify(FALSE);
	}
	
	//printf("CPage::Open finish\n");
	//fflush(stdout);
	
	return TRUE;
}

VD_BOOL CPage::Close(UDM mode)
{
	m_pCursor->SetCursor(CR_BUSY);
	if(!UpdateData(mode)){
		m_pCursor->SetCursor(CR_NORMAL);
		return FALSE;
	}
	m_pCursor->SetCursor(CR_NORMAL);
	m_ExitState = mode;
	
	return Close();
}

//#define GUI_TEST//csp modify

//关
VD_BOOL CPage::Close()
{
	if (m_dwStyle &pageChild)
	{
		//printf(" page child close \n");
		Show(FALSE);
		return TRUE;
	}
	
#ifdef GUI_TEST
	if(CDC::GetModify())//csp modify
	{
		printf("CPage::Close:framebuffer changed-1\n");
		CDC::SetModify(FALSE);
	}
#endif
	
	m_MutexMgr.Enter();
	if(!GetFlag(IF_OPENED)){
		//printf("CPage::Close(),page is already closed.\n");
		m_MutexMgr.Leave();
		return FALSE;
	}
	SetFlag(IF_OPENED, FALSE);
	m_MutexMgr.Leave();
	
#ifdef GUI_TEST
	if(CDC::GetModify())//csp modify
	{
		printf("CPage::Close:framebuffer changed-2\n");
		CDC::SetModify(FALSE);
	}
#endif
	
	m_DC.Lock();
	m_DC.Enable(FALSE);
	m_DC.UnLock();
	
#ifdef GUI_TEST
	if(CDC::GetModify())//csp modify
	{
		printf("CPage::Close:framebuffer changed-3\n");
		CDC::SetModify(FALSE);
	}
#endif
	
	//printf("CPage::Close\n");
	//关闭所有控件
	CItem *p = m_pChildHead;
	while(p)
	{
		if(!p->GetFlag(IF_POPUP))
		{
			p->Close();
		}
		p = p->m_pBrother;
	}
	
#ifdef GUI_TEST
	if(CDC::GetModify())//csp modify
	{
		printf("CPage::Close:framebuffer changed-4\n");
		CDC::SetModify(FALSE);
	}
#endif
	
	Show(FALSE);
	
#ifdef GUI_TEST
	if(CDC::GetModify())//csp modify
	{
		printf("CPage::Close:framebuffer changed-5\n");
		CDC::SetModify(FALSE);
	}
#endif
	
	UpdateInputTray();
	
#ifdef GUI_TEST
	if(CDC::GetModify())//csp modify
	{
		printf("CPage::Close:framebuffer changed-6\n");
		CDC::SetModify(FALSE);
	}
#endif
	
	//set focus state, no need draw
	//如果原焦点控件的附属控件打开或关闭，此处可能会引起死锁。
	SetFocusItem(NULL);
	SetSelectItem(NULL);
	UpdateData(UDM_CLOSED);
	
#ifdef GUI_TEST
	if(CDC::GetModify())//csp modify
	{
		printf("CPage::Close:framebuffer changed-7\n");
		CDC::SetModify(FALSE);
	}
#endif
	
	KillFocus();
	
#ifdef GUI_TEST
	if(CDC::GetModify())//csp modify
	{
		printf("CPage::Close:framebuffer changed-8\n");
		CDC::SetModify(FALSE);
	}
#endif
	
	if(m_pThread && !(m_dwStyle & pageDialog))
	{
		m_pThread->DestroyThread();
	}
	
	return TRUE;
}

VD_BOOL CPage::InvalidateRgn(CRgn * pRgn)
{
	CItem *pItem;
	pItem = m_pChildHead;
	while(pItem){
		if(!pItem->GetFlag(IF_POPUP))
		{
			pItem->InvalidateRgn(pRgn);
		}
		pItem = pItem->m_pBrother;
	}

	CItem::InvalidateRgn(pRgn);

	return TRUE;
}

//可能会改变pRgn的内容
VD_BOOL CPage::ValidateRgn(CRgn * pRgn)
{
	//draw validate region
	CItem *pItem;
	UpdateRgn();
	pItem = m_pChildHead;
	while(pItem){
		if(!pItem->GetFlag(IF_POPUP))
		{
			pItem->m_DC.Lock();
			pItem->m_DC.IntersectRgn(pRgn);
			pItem->m_DC.UnLock();
		}
		pItem = pItem->m_pBrother;
	}

	m_DC.Lock();
	m_DC.IntersectRgn(pRgn);
	m_DC.UnLock();

	Draw();

	//recover visible region
	UpdateRgn();

	return TRUE;
}

void CPage::UpdateRgn()
{
	CRgn rgn;
	VD_RECT rect;
	//printf("CPage:UpdateRgn 0\n");
	CGuard guard(m_MutexMgr);

	//printf("CPage:UpdateRgn 1\n");
	if(!GetFlag(IF_OPENED))
	{
		return;
	}
	
	
	CItem *pChild = m_pChildHead;
	CItem * pItem = m_pUp;
	
	m_DC.Lock();
//	printf("CPage:UpdateRgn 2\n");
	m_DC.SetBound(m_Rect);
	//printf("CPage:UpdateRgn 3\n");

	//m_DC.UnLock();

	//printf("this = 0x%x child = 0x%x \n", this, pChild );

	while(pChild){
		//caculate items rectangle
		//printf("pChild 0x%x", pChild);
		pChild->UpdateRgn();
		pChild = pChild->m_pBrother;
	}

	//m_DC.Lock();

//	printf("CPage:UpdateRgn 4\n");
	while(pItem)
	{
		//get rect of top item
		pItem->GetRect(&rect);
		
		//invalid unvisible region
		pChild = m_pChildHead;
		while(pChild)
		{
			if(!pChild->GetFlag(IF_POPUP))
			{
				pChild->m_DC.Lock();
				pChild->m_DC.SubtractRect(&rect);
				pChild->m_DC.UnLock();
			}
			pChild = pChild->m_pBrother;
		}
		m_DC.SubtractRect(&rect);
		
		//point to next top item
		pItem = pItem->m_pUp;
	}
//	printf("CPage:UpdateRgn 5\n");
	m_DC.UnLock();
//	printf("CPage:UpdateRgn 6\n");
}

void CPage::ClearRgn()
{
	CItem *pChild = m_pChildHead;
	
	m_DC.Lock();
	m_DC.SetBound(m_Rect);
	m_DC.UnLock();
	
	while(pChild)
	{
		//caculate items rectangle
		pChild->ClearRgn();
		pChild = pChild->m_pBrother;
	}
}

//绘制页面
void CPage::Draw()
{
	if(!DrawCheck()){
		return;
	}
	
	m_DC.Lock();//保护m_bDrawBkgdOnly
	//不能再调用子项的绘制函数，否则引起死循环。
	if(m_bDrawBkgdOnly)
	{
		m_DC.UnLock();
		return;
	}
	
	CItem *p = m_pChildHead;
	while(p){
		//p->GetRect(&rect);
		//trace("item (%d %d %d %d)\n", rect.left, rect.top, rect.right, rect.bottom);
		if(!p->GetFlag(IF_POPUP) && p->GetFlag(IF_SHOWN))
		{
			//printf("Draw:  %x \n",p);
			p->Draw();
			//p->Open();
		}
		p = p->m_pBrother;
	}
	m_DC.UnLock();
}

//首先交给输入提示框处理
VD_BOOL CPage::PreMsgProc(uint msg, uint wpa, uint lpa)
{
	CItem *pItemSpecial = NULL;

	switch(msg)
	{
	case XM_KEYDOWN:
	case XM_KEYUP:
		pItemSpecial = m_pInputTray[m_screen];
		if(pItemSpecial && pItemSpecial->GetOwner() == m_pChildFocus && m_pChildFocus
			&& (m_pChildFocus->GetType() == IT_EDIT || (m_pChildFocus->GetType() == IT_NUMBERBOX && !GetKeyInputFlag(KIF_NUMERIC)))
			&& pItemSpecial->IsOpened() && pItemSpecial->MsgProc(msg, wpa, lpa))
		{
			return TRUE;
		}
	default:
		return FALSE;
	}
	return FALSE;
}

VD_BOOL CPage::MsgProc(uint msg, uint wpa, uint lpa)
{
	//printf("CPage::MsgProc 22222\n");
	//printf("CPage::MsgProc msg: 0x%x\n", msg);
	if(m_dwStyle & pageChild)
	{
		m_pParent->MsgProc(msg, wpa, lpa);
	}
	
	CItem *pItemSpecial = NULL;
	int ret;
	int key;
	int px;
	int py;
	CItem* pItem;
	VD_RECT full_rect;
	m_ExitState = UDM_EMPTY;
	if(m_bResizing){
		printf("CPage::MsgProc m_bResizing\n");
		goto resizing_proc;
	}
	switch(msg)
	{
	case XM_KEYDOWN://按键按下
		//printf("file:%s, line:%d\n", __FILE__, __LINE__);
		key = wpa;
		switch(key){
		case KEY_RET:		//确认键
			ret = m_pChildFocus?m_pChildFocus->MsgProc(msg, wpa, lpa):FALSE;
			if(!ret)
			{
				if(m_pChildDflt)
				{
					SetFocusItem(m_pChildDflt);
					SetSelectItem(m_pChildDflt);
				}
				else
				{
					//return Close(UDM_OK);  //cw_panel close induce page exit
				}
				UpdateInputTray();
			}
			break;
		case KEY_ESC:		//取消键
			ret = (m_pChildFocus)?m_pChildFocus->MsgProc(msg, wpa, lpa):FALSE;
			if(m_pChildFocus && !m_pChildFocus->GetFlag(IF_AUTOFOCUS))//控件退出激活状态
			{
				m_pChildFocus->SetFocus(FALSE);
				m_pChildFocus = NULL;
			}
			else if(!ret)
			{
				return Close(UDM_CANCEL);
			}
			UpdateInputTray();
			break;
		case KEY_SHUT://cw_panel
			printf("recvfrom frontboard, Forced shutdown\n");
			SetSystemLockStatus(1);
			if(TRUE!=FxnKeycodeShut())
			{
				SetSystemLockStatus(0);//cw_lock
			}
			break;
		case KEY_LEFT:
		case KEY_RIGHT:
		case KEY_UP:
		case KEY_DOWN:
		case KEY_PREV:
		case KEY_NEXT:
			if(!m_pChildFocus || !m_pChildFocus->MsgProc(msg, wpa, lpa))
			{
				pItem = ShiftItem(key, m_pChildSelect, m_pChildSelectOld);//切换选中项
				if(pItem == NULL){//焦点不存在或者焦点对方向键不响应
					return FALSE;
				}
				SetSelectItem(pItem);//设置选中项
			}
			break;
		/*  //cw_panel
		case KEY_SPLIT:
			PreviewToNextMod();
			break;
		*/
		case KEY_CHNPLUS:
			//printf("KEY_CHNPLUS - PreviewToNextPic\n");
			PreviewToNextPic();
			break;
		case KEY_CHNMINUS:
			PreviewToLastPic();
			break;
		case KEY_MUTE:
			{
				CPage** page = GetPage();
				if(this == page[EM_PAGE_PLAYBACK])
				{
					u8 mute;
					this->GetCurPlayMute(&mute);
					this->SetCurPlayMute(mute?0:1);
				}
				else
				{
					u8 chn, vol, mute;
					if(0 == GetAudioOutInfo(&chn, &vol, &mute))
					{
						SetAudioOutInfo(chn, vol, mute?0:1);
						ControlMute(mute?0:1);
						CPage** page = GetPage();
						page[EM_PAGE_START]->SetMute((BOOL)mute?0:1);
					}
				}
			}	
			break;
		case KEY_VOIP:
			break;
		case KEY_REC://Enter record page
		case KEY_SEARCH://Enter search page
		case KEY_PLAY://Enter search page
		case KEY_INFO://Enter info address page
		case KEY_ALARM:
		case KEY_MENU:
		case KEY_BACKUP:
		case KEY_ALARMIN:
		case KEY_PTZ:
			{
				CPage** page = GetPage();
				if(this == page[EM_PAGE_PLAYBACK])
				{
					break;
				}
			}
			
			if(!IsGUIUserLogined())
			{
				break;
			}
			ret = m_pChildFocus?m_pChildFocus->MsgProc(msg, wpa, lpa):FALSE;
			//if(!ret && !(m_dwStyle & pageDialog) && m_pParent != m_pInputTray[m_screen])//弹出式窗口和选词窗口不用处理系统键,否则死锁
			//printf("ret = %d\n", ret);
			if(!ret)
			{
				if(key == KEY_REC)
				{
					pItemSpecial = FindPage(m_screen, "&CfgPtn.Record", FALSE);
				}
				else if(key == KEY_SEARCH || key == KEY_PLAY)
				{
					//pItemSpecial = FindPage(m_screen, "&titles.search");
					//yaogang modify 20141106
					//pItemSpecial = FindPage(m_screen, "&CfgPtn.DataSearch", FALSE);
					pItemSpecial = FindPage(m_screen, "&CfgPtn.VideoPlayback", FALSE);
				}
				else if(key == KEY_INFO)
				{
					pItemSpecial = FindPage(m_screen, "&CfgPtn.System", FALSE);
				}
				else if(key == KEY_ALARM)
				{
					pItemSpecial = FindPage(m_screen, "&CfgPtn.Alarm", FALSE);
				}
				else if (key == KEY_BACKUP)
				{
					pItemSpecial = FindPage(m_screen, "&CfgPtn.Backup");
				}
				else if(key == KEY_MENU)
				{
					pItemSpecial = FindPage(m_screen, "&CfgPtn.MAINMENU", FALSE);					
				}
				else if (key == KEY_ALARMIN)
				{					
					pItemSpecial = FindPage(m_screen, "&CfgPtn.SensorTitle", FALSE);
				}
				else if (key == KEY_PTZ)
				{					
					pItemSpecial = FindPage(m_screen, " Ptz Control", FALSE);
				}
				if(pItemSpecial)
				{
					if(key==KEY_PTZ)
					{
						if(((CPagePtzCtl* )pItemSpecial)->GetCruiseState())
							return FALSE;
					}
					
					if(pItemSpecial == this) //主菜单已经打开, 不再响应此按键, 否则CloseAll+Open会引起死锁
					{
						pItemSpecial = NULL;
					}
					else
					{
						CloseAll();
					}
				}
				
				if(pItemSpecial)
				{
					//cw_panel
					if(key == KEY_SEARCH || key == KEY_PLAY)
					{
						((CPageSearch* )pItemSpecial)->UpdateData(UDM_EMPTY);
					}
					
					if(pItemSpecial->Open())
					{
						if (key == KEY_BACKUP)
						{
							//模拟发键
							pItemSpecial->MsgProc(msg, wpa, lpa);
							pItemSpecial->MsgProc(XM_KEYUP, wpa, lpa);
						}
                        //add by Lirl on Nov/08/2011
                        if(key == KEY_SEARCH || key == KEY_PLAY)
                        {
                            ((CPageSearch*)pItemSpecial)->FromPanel();
                        }
                        //end
					}
				}
			}
			break;
		default://other keys
			{
				//printf("default key\n");
				return m_pChildFocus?m_pChildFocus->MsgProc(msg, wpa, lpa):FALSE;
			}
		}
		break;

	case XM_MOUSEMOVE:		//鼠标移动
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(!m_pChildFocus || !m_pChildFocus->GetFlag(IF_CAPTURED))//输入没有被捕获
		{
			pItem = GetItemAt(px, py);
			SetSelectItem(pItem);
		}
		ret = m_pChildFocus?m_pChildFocus->MsgProc(msg, wpa, lpa):FALSE;
		return ret;

	case XM_LBUTTONDOWN:	//鼠标左键单击
	case XM_LBUTTONDBLCLK:	//鼠标左键双击
		//printf("CPAGE:MAG:LBUTTONDOWN 111, m_bLClicked: %d\n", m_bLClicked);
		if(!m_bLClicked){//双击转化为单击
			msg = XM_LBUTTONDOWN;
			m_bLClicked = TRUE;
		}
		goto mouse_proc;
	case XM_RBUTTONDOWN:	//鼠标右键单击
	case XM_RBUTTONDBLCLK:	//鼠标右键双击
		if(!m_bRClicked){//双击转化为单击
			msg = XM_RBUTTONDOWN;
			m_bRClicked = TRUE;
		}
		goto mouse_proc;
	case XM_MBUTTONDOWN:	//鼠标中键单击
	case XM_MBUTTONDBLCLK:	//鼠标中键双击
		if(!m_bMClicked){//双击转化为单击
			msg = XM_MBUTTONDOWN;
			m_bMClicked = TRUE;
		}
		goto mouse_proc;
	case XM_LBUTTONUP:		//左键弹开
	case XM_RBUTTONUP:		//右键弹开
	case XM_MBUTTONUP:		//中键弹开
mouse_proc:
		//printf("CPAGE:MAG:LBUTTONDOWN mouse_proc\n");
		m_mouseX = px = VD_HIWORD(lpa);
		m_mouseY = py = VD_LOWORD(lpa);
		if(m_pChildFocus && m_pChildFocus->GetFlag(IF_CAPTURED) && m_pChildFocus->MsgProc(msg, wpa, lpa))
		{//输入被焦点捕获，所以由焦点处理
			//printf("yg mouse_proc 1\n");
			return TRUE;
		}else{
			//printf("yg mouse_proc 2\n");
			pItem = GetItemAt(px, py);		//获取当前焦点控件
			if(msg == XM_LBUTTONDOWN || msg == XM_LBUTTONDBLCLK)
			{
				//printf("yg mouse_proc 3\n");
				if(!pItem)
				{
					//printf("yg mouse_proc 4\n");
					SetSelectItem(NULL);
					m_pDevGraphics->GetRect(&full_rect);
					if(m_iType == IT_PAGE
						&& (msg == XM_LBUTTONDOWN)
						&& px >= m_Rect.left && py >= m_Rect.top && px <= m_Rect.right && py <= m_Rect.top + m_TitleHeight
						&& m_Rect != full_rect && m_bResizable)
					{//开始移动页面
						//printf("yg mouse_proc 5\n");
						m_iResizeXOffs = px - m_Rect.left;
						m_iResizeYOffs = py - m_Rect.top;
						ResizeStart();
						break;
					}
					return FALSE;
				}
				//printf("yg mouse_proc 6\n");
				if(pItem != m_pChildFocus )
				{
					//printf("yg mouse_proc 7\n");
					SetFocusItem(pItem);
					SetSelectItem(pItem);
				}
				//printf("yg mouse_proc 8\n");
				if(pItem->MsgProc(msg, wpa, lpa)){		//转到相应控件的处理函数
					//printf("cc pItem->MsgProc \n");
					break;
				};
			}
			
			if(msg == XM_RBUTTONDOWN || msg == XM_RBUTTONDBLCLK){//右键双击或单击，则取消退出
				//if(m_bPlayBackPage)
				//{
				//
				//}
				//else
				//{
					//printf("CPage::Close(UDM_CANCEL)\n");
					if(Close(UDM_CANCEL)){
						break;
					}
				//}
			}
			/*if(msg == XM_LBUTTONDBLCLK){//左键双击，则保存退出
				if(Close(UDM_OK)){
					break;
				};
			}*/
			//printf("yg mouse_proc m_pChildFocus: %d\n", m_pChildFocus);
			return m_pChildFocus?m_pChildFocus->MsgProc(msg, wpa, lpa):FALSE;
		}
		break;

	default:
		return m_pChildFocus?m_pChildFocus->MsgProc(msg, wpa, lpa):FALSE;
	}
	
	return TRUE;
	
resizing_proc:
	switch(msg){
	case XM_KEYDOWN:
		key = wpa;
		switch(key){
		case KEY_LEFT:
			Resize(-16, 0);
			break;
			
		case KEY_RIGHT:
			Resize(16, 0);
			break;
			
		case KEY_UP:
		case KEY_PREV:
			Resize(0, -16);
			break;
			
		case KEY_DOWN:
		case KEY_NEXT:
			Resize(0, 16);
			break;
			
		default://other keys
			return FALSE;
		}
		break;
		
	case XM_MOUSEMOVE:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		Resize(px - (m_iResizeXOffs + m_RectResizing.left), py - (m_iResizeYOffs + m_RectResizing.top));
		break;
		
	case XM_LBUTTONUP:
		ResizeEnd();
		break;
		
	default:
		return FALSE;
	}
	return TRUE;
}

UDM CPage::PopupRun()
{
	if(!(m_dwStyle & pageDialog))
	{
		printf("CPage::PopupRun() only for dialog\n");
		return UDM_EMPTY;
	}

	// 搜索对话框的调用者
	m_MutexMgr.Enter();
	CItem* pItem = m_pTop[m_screen];
	int id = ThreadGetID();
	while(pItem)
	{
		if(pItem->m_pThread && pItem->m_pThread->GetThreadID() == id)
		{
			m_pPopupCaller = reinterpret_cast<CPage*>(pItem);
			break;
		}
		pItem = pItem->m_pDown;
	}
	m_MutexMgr.Leave();

	//save old cursor
	int old_cursor = m_pCursor->GetCursor();

	Open();
	m_pThread->ClearMessage();
	m_pThread->m_bLoop = TRUE;
	VD_MSG msg;
	while(m_pThread->m_bLoop)
	{
		//printf("m_pThread = %x \n",m_pThread);
		m_pThread->VD_RecvMessage(&msg);
		if(!PreMsgProc(msg.msg, msg.wpa, msg.lpa))
		{
			MsgProc(msg.msg, msg.wpa, msg.lpa);
			if(m_ExitState == UDM_OK || m_ExitState == UDM_CANCEL)
			{
				goto run_end;//already be closed
			}
		}
	}
	Close(UDM_CANCEL);
	
run_end:
	//restore old cursor
	m_pCursor->SetCursor(old_cursor);
	
	return m_ExitState;
}

VD_PCSTR CPage::GetTitle()
{
	return m_pTitle.c_str();
}

void  CPage::SetTitle(VD_PCSTR psz)
{
	if(psz)
	{
		m_pTitle = psz;
	}
	else
	{
		m_pTitle.erase();
	}
}

int CPage::GetThreadID()
{
	if(m_pThread)
	{
		return m_pThread->GetThreadID();
	}
	return -1;
}

// 得到页面的退出状态
// UDM_EMPTY  - 没有退出
// UDM_OK     -	确定退出
// UDM_CANCEL - 取消退出
UDM CPage::GetExitState()
{
	return m_ExitState;
}

void CPage::GetMargin(VD_PRECT rect)
{
	if (rect)
	{
		rect->left = m_Margin.left;
		rect->top = m_Margin.top;
		rect->right = m_Margin.right;
		rect->bottom = m_Margin.bottom;	
	}
}

void CPage::UpdateInputTray()
{
	CItem *pCurrent = CItem::GetActive(m_screen);
	CItem *pOldOwner;

	while(pCurrent && pCurrent->m_iType != IT_PAGE)
	{
		pCurrent = pCurrent->m_pParent;
	}

	if(pCurrent && pCurrent->m_iType == IT_PAGE)
	{
		
		CPage *pPage = ((CPage*)pCurrent);
		CItem *pChildFocus;
		VD_RECT rect;
		VD_RECT owner_rect;
		CItem *pInputTray = m_pInputTray[m_screen];

		pChildFocus = pPage->GetFocusItem();
		if(pInputTray && (pOldOwner = pInputTray->GetOwner()) != pChildFocus)//处理焦点转移
		{
			pInputTray->SetOwner(pChildFocus);
			
			//当前页面切换到选词条或者从选词条切换到其他页面,都无需处理,否则反而引起死锁
			if(pPage->m_pParent != pInputTray
				&& (!pOldOwner || !pOldOwner->m_pParent || pOldOwner->m_pParent->m_pParent != pInputTray))
			{
				//if(pChildFocus && (pChildFocus->m_iType == IT_EDIT
				//	|| (pChildFocus->m_iType == IT_NUMBERBOX && !CItem::GetKeyInputFlag(KIF_NUMERIC))))//没有数字键，需要给出按键转换提示
				if(pChildFocus && (pChildFocus->m_iType == IT_EDIT))
				{
					pInputTray->GetRect(&rect);
					if(pChildFocus->m_pOwner)
					{
						pChildFocus->m_pOwner->GetRect(&owner_rect);
						::OffsetRect(&rect, owner_rect.right - rect.left, owner_rect.bottom - rect.bottom);
					}
					else
					{
						::OffsetRect(&rect, pChildFocus->m_Rect.right - rect.left, pChildFocus->m_Rect.bottom - rect.bottom);
					}
					pInputTray->Show(FALSE);
					pInputTray->SetRect(&rect, FALSE);
					//printf("aaaaaaaaaa \n");
					pInputTray->Show(TRUE);
				}
				else
				{
					pInputTray->Show(FALSE);
				}
			}
		}
	}
}

/*
* 自动控件布局规则1
*适用范围通常页面的自动控件布局,具体是指页面上就只有控件的布局.
*如果页面上还有页面自己绘制的部分.自动布局可能出现重叠e.g.录像设置
*布局风格: 第一个控件的位置工程师可以自己设定默认为CLIENT_X,以后地控件就是根据第一个控件的位置来依次布局
* 如果最后一排显示都是按钮的话,这排会在底端居中显示
*/
void CPage::AutoLayout()
{
	AUTO_LAYOUT m_LayOut;  
	int left,top,right,bottom; //控件坐标
	int pre_bottom;//保存前一行的bottom，用于行重新布局
	VD_BOOL mark_row = FALSE; //标记该行是否已经重新布局过
	VD_BOOL mark_second = FALSE; //标记是否已经二次布局过
	int tempTable[4] = {0,0,0,0}; //存放前一次界面布局时的Table1到Table4的坐标

	LayoutInit(&m_LayOut);

	pre_bottom = m_LayOut.maxBottom; 

	int inRowTab[MAX_TABLE_NUM] = {0,0,0,0,0,0,0}; //标记一行中同一Table已经出现过，非0为已经出现过
	//获得有几行控件,每行有几个控件
	while(m_LayOut.pCur)
	{
		/*如果最后一个控件带有换行属性，则去掉该属性，以避免后面处理出错*/
		if(m_LayOut.pCur->m_pBrother == NULL)
		{
			if((m_LayOut.pCur->m_dwStyle & changeLine) == changeLine)
			{
				m_LayOut.pCur->m_dwStyle = m_LayOut.pCur->m_dwStyle & ~changeLine;
			}
			else if(m_LayOut.pCur->m_pOwner != NULL && (m_LayOut.pCur->m_pOwner->m_dwStyle & changeLine) == changeLine) //最后一行是复合控件，且构造成了换行
			{
				m_LayOut.pCur->m_pOwner->m_dwStyle = m_LayOut.pCur->m_pOwner->m_dwStyle & ~changeLine;
			}
		}

		/*解决一行中两次及两次以上使用同一Table时会死循环的问题
		只处理第一个使用该Table属性的控件，清除掉同一行其他控件的该Table属性*/
		if((m_LayOut.pCur->m_dwStyle & alignTableMask) != 0)
		{
			int table = (m_LayOut.pCur->m_dwStyle & alignTableMask) >> 24;
			if(!inRowTab[table - 1])
			{
				inRowTab[table - 1] += 1;
			}
			else
			{
				printf("@@@@@@@@@@@@@@@@@>>>>>>>>>>>>>>WARNING!!!!!!> Same Table in same row .......\n");
				m_LayOut.pCur->m_dwStyle &= ~(m_LayOut.pCur->m_dwStyle & alignTableMask);
			}
		}

		m_LayOut.row[m_LayOut.rowNum-1]++;

		if(	(((m_LayOut.pCur->m_dwStyle & changeLine) == changeLine) && 
			(m_LayOut.pCur->m_pBrother->m_pOwner == NULL))	
			|| ((m_LayOut.pCur->m_pOwner != NULL) && 	/*处理复合控件换行所加条件*/
			(m_LayOut.pCur->m_pBrother != NULL) && 
			(m_LayOut.pCur->m_pBrother->m_pOwner == NULL) && 
			((m_LayOut.pCur->m_pOwner->m_dwStyle & changeLine) == changeLine)))
		{
			m_LayOut.rowNum++;
			for(int i = 0; i < MAX_TABLE_NUM; i++)
			{
				inRowTab[i] = 0;
			}
			if(m_LayOut.rowNum > ROW_MAX_NUM)
				break;
		}
		m_LayOut.pCur = m_LayOut.pCur->m_pBrother;
	}
	//endof
	
	//开始遍历所有控件
	m_LayOut.pCur = m_LayOut.head;
	while(m_LayOut.pCur)		
	{
		if(m_LayOut.pPre == m_LayOut.pCur)
		{
			m_LayOut.pLineHead = m_LayOut.pCur;  //记录行首指针
		}

		//if(m_LayOut.curRow == 3 && m_LayOut.curColumn == 4)
		//{
		//	tracepoint();
		//}
		/*计算控件的坐标*/
		left = SetCtrlLeft(&m_LayOut);
		top = SetCtrlTop(&m_LayOut);
		right = SetCtrlRight(&m_LayOut, left);
		bottom = SetCtrlBottom(&m_LayOut, top);

		//trace("@@@@@@@@@@@ left = %d ,top = %d ,right = %d , bottom = %d  \n ", left, top, right, bottom);
		//设置当前控件的坐标
		m_LayOut.pCur->m_Rect.SetRect(left,top,right,bottom);
		m_LayOut.pCur->m_Margin.SetRectEmpty();
		this->ClientToScreen(&(m_LayOut.pCur->m_Rect));//把相对坐标转为绝对坐标

		/*--------换行--------*/
		if( ( (m_LayOut.pCur->m_dwStyle & changeLine) == changeLine && 
			(m_LayOut.pCur->m_pBrother->m_pOwner == NULL) )
			|| (m_LayOut.pCur->m_pOwner != NULL &&      //处理符合控件换行所加条件
			(m_LayOut.pCur->m_pOwner->m_dwStyle & changeLine) == changeLine &&
			m_LayOut.pCur->m_pBrother->m_pOwner == NULL))
		{
			if((m_LayOut.pCur->m_Rect.right + m_LayOut.startX) > m_Rect.right)
			{
				if(!mark_row)//如果没有排过序
				{
					m_LayOut.pCur = m_LayOut.pLineHead;
					m_LayOut.pPre = m_LayOut.pCur;
					m_LayOut.width = m_LayOut.width/4; //行重新布局 列间距减半
					m_LayOut.curColumn = 1;
					mark_row = TRUE;
					m_LayOut.maxBottom = pre_bottom;
					m_LayOut.reRowLayout = TRUE;
					continue; //行重新布局
				}
			}
			pre_bottom = m_LayOut.maxBottom;
			m_LayOut.width = m_iSpace;;
			mark_row = FALSE;
			m_LayOut.pCur = m_LayOut.pCur->m_pBrother;
			m_LayOut.pPre = m_LayOut.pCur;
			m_LayOut.curRow++;
			m_LayOut.curColumn = 1;
			m_LayOut.reRowLayout = FALSE;
		}
		else 
		{
			m_LayOut.pPre = m_LayOut.pCur;
			m_LayOut.pCur = m_LayOut.pCur->m_pBrother;
			m_LayOut.curColumn++;
		}

		/*******以下代码用于二次布局**********/	
		if(m_LayOut.curRow >= m_LayOut.rowNum)
		{
			//将Table坐标columnLeft[i]与前一次的Table坐标tempTable[i]作比较
			//如果不完全相等，则更新empTable[i]，设置重新布局标志secondLayout为TRUE
			for(int i = 0; i < 4; i++)
			{
				if(tempTable[i] != m_LayOut.columnLeft[i])
				{
					tempTable[i] = m_LayOut.columnLeft[i];
					mark_second = FALSE;
					m_LayOut.secondLayout = TRUE;
				}
			}
			if(!mark_second)
			{
				if(m_LayOut.secondLayout)
				{
					m_LayOut.pCur = m_LayOut.head;
					m_LayOut.pPre = m_LayOut.pCur;
					m_LayOut.pLineHead = m_LayOut.pCur;
					m_LayOut.reRowLayout = FALSE;
					m_LayOut.curRow = 1;
					m_LayOut.curColumn = 1;
					mark_second = TRUE;
					mark_row = FALSE;
					continue; //二次布局
				}		
			}
			m_LayOut.secondLayout = FALSE;
		}
	}
	return ;
}

/*设置控件的 Left 值*/
int CPage::SetCtrlLeft(AUTO_LAYOUT *pAL)
{
	int left = 0;
	int column = 0;
	CRect rect;

	if((pAL->pPre == pAL->pCur) && 
		(pAL->pCur->m_dwStyle & alignCenter) == alignCenter) //居中
	{
		
		CItem *pp;
		int total_width = 0;
		int ctrl_width = 0;
		int i = 0;
		CRect w_rect(m_Rect);
		this->ScreenToClient(w_rect);
		for(i = pAL->curColumn, pp = pAL->pCur; i <= pAL->row[pAL->curRow - 1]; i++, pp = pp->m_pBrother)
		{
			if(pp->m_Rect.Width() != 0)
			{
				ctrl_width = pp->m_Rect.Width();
			}
			else if(pp->GetDefaultSize().w != 0)
			{
				if(pp->m_iType == IT_STATIC
					|| pp->m_iType == IT_BUTTON
					|| pp->m_iType == IT_COMBOBOX)
				{
					int offset = 0;
					offset = pp->m_iType == IT_STATIC ? 0 : 8;
					ctrl_width = pp->GetDefaultSize().w >= (g_Locales.GetTextExtent(pp->GetText()) + offset) ?	
										pp->GetDefaultSize().w : (g_Locales.GetTextExtent(pp->GetText()) + offset); 		
				}
				else
				{
					ctrl_width = pp->GetDefaultSize().w;
				}
			}
			else
			{
				printf(">>>>>>>>>>>> Ctrl %d don't set width!!!!!! \n",pp->m_iType);
				ctrl_width = TEXT_WIDTH * 6; 
			}
			if(NULL == pp->m_pOwner)
			{
				total_width += ctrl_width + pAL->width;
			}
		}
		
		left = m_Rect.Width()/2 - (total_width - pAL->width)/2;
	}
	else if((pAL->pCur->m_dwStyle & alignTableMask) != 0) //Table
	{
		column = (pAL->pCur->m_dwStyle & alignTableMask) >> 24;

		//处理Table的坐标，一行中每个Table的坐标都根据其前一个控件的Right确定
		//最后columnLeft[column - 1]中存放各行相应Table的最大值
		if(column > 4) //自动处理前4个Table坐标，后三个不自动处理
		{
			left = pAL->columnLeft[column - 1];			
		}
		else if(column > 0 && column <= 4)
		{
			if(1 == column) //Table1的坐标为界面起始X坐标
			{
				left = pAL->startX;
			}
			else //
			{
				CRect preRect(pAL->pPre->m_Rect);
				this->ScreenToClient(preRect);
				
				//行的第一个控件，如果不是Table1,则直接读数组columnLeft[column - 1]，
				//而不根据其前一个控件确定，因为它的前一个控件是上一行的尾控件
				if((pAL->pCur != pAL->pPre) 
					&& ((preRect.right + pAL->width) > pAL->columnLeft[column - 1]))
				{					
					pAL->columnLeft[column - 1] = preRect.right + pAL->width;					
				}
				left = pAL->columnLeft[column - 1];
			}
		}
	}
	else if((pAL->pCur->m_dwStyle & inflateAll) != 0) //对齐方式
	{
		switch(pAL->pCur->m_dwStyle & inflateAll)
		{
		case (alignRight | alignBottom):
		case alignRight:
			{
				CItem *pp;
				int column_num = 0;
				int total_width = 0;
				int ctrl_width = 0;
				int i = 0;
				VD_BOOL mark = FALSE;
				CRect w_rect(m_Rect);
				this->ScreenToClient(w_rect);
				for(i = pAL->curColumn, pp = pAL->pCur; i <= pAL->row[pAL->curRow - 1]; i++, pp = pp->m_pBrother)
				{
					if(pp->m_Rect.Width() != 0)
					{
						ctrl_width = pp->m_Rect.Width();
					}
					else if(pp->GetDefaultSize().w != 0)
					{
						if(pp->m_iType == IT_STATIC
							|| pp->m_iType == IT_BUTTON
							|| pp->m_iType == IT_COMBOBOX)
						{
							int offset = 0;
							offset = pp->m_iType == IT_STATIC ? 0 : 8;
							ctrl_width = pp->GetDefaultSize().w >= (g_Locales.GetTextExtent(pp->GetText()) + offset) ?  
												pp->GetDefaultSize().w : (g_Locales.GetTextExtent(pp->GetText()) + offset);			
						}
						else
						{
							ctrl_width = pp->GetDefaultSize().w;
						}
					}
					else
					{
						printf(">>>>>>>>>>>> Ctrl %d don't set width!!!!!! \n",pp->m_iType);
						ctrl_width = TEXT_WIDTH * 6; 
					}
					if((pp->m_dwStyle & alignTableMask) != 0)
					{
						column = (pp->m_dwStyle & alignTableMask) >> 24;
						left = pAL->columnLeft[column - 1] - total_width - pAL->width * column_num ;
						mark = TRUE;
						break; //退出循环，不再遍历控件
					}
					total_width += ctrl_width;
					column_num++;
				}

				if(!mark)
				{
					left = w_rect.right - total_width - pAL->width * (column_num - 1) - pAL->startX;
				}
			}
			break;
		default:
			if(pAL->pPre == pAL->pCur)
			{
				left = pAL->startX;
			}
			break;
		}
	}
	else  //无Table,且无对齐方式
	{
		if(pAL->pPre == pAL->pCur)  //每一排的第一个控件
		{			
			left = pAL->startX;
		}
		else// 不是每一行的第一个
		{
			//this->ClientToScreen(&(p->m_Rect));//
			//不是每一行的第一个,那么它的left 和top 需要跟据前一个的属性来确定
			rect = pAL->pPre->m_Rect;
			this->ScreenToClient(&rect);//绝对坐标转为相对坐标
			switch(pAL->pPre->m_iType)
			{
				//如果该上一个控件是由其他控件组成的,比如时间日期框.
				//那么我们需要对这类控件进行布局.
			case IT_DATETIMECTRL:
			case IT_TIMINGCTRL:
				left = rect.left + TEXTBOX_X/*内置控件的间距*/;
				break;
			case IT_IPADDRESSCTRL:
				left = rect.left + 2/*内置控件的间距*/;
				break;
			default :

				if((pAL->pCur->m_pOwner)!= NULL)
				{
					switch((pAL->pCur->m_pOwner)->m_iType)
					{
					case IT_DATETIMECTRL:
						left = rect.left + rect.Width() + (pAL->pCur->m_pOwner->GetDefaultSize().w)/DTC_ITEMS;
						break;
					case IT_IPADDRESSCTRL:
						left = rect.left +TEXT_WIDTH*5/2;
						break;
					case IT_TIMINGCTRL:
						left = rect.left + rect.Width()+TEXT_WIDTH/2;
						break;
					default:
						left = rect.left + rect.Width() + TEXT_WIDTH;
						break;
					}
				}
				else
				{
					//调整两个相连是checkbox控件的间距
					if((pAL->pPre->m_iType == IT_CHECKBOX) && (pAL->pCur->m_iType == IT_CHECKBOX))
					{
						if(strcmp(pAL->pCur->GetText(),""))
						{
							left = rect.left + rect.Width()+3;
						}
						else
						{
							if((pAL->pCur->m_dwStyle & smallWidth) == smallWidth) //用smallWidth的CheckBox处理报警输入输出界面
							{
								left = rect.left + rect.Width() + m_playoutSpace[spaceCheckBoxSmall]; 
							}
							else
							{
								left = rect.left + rect.Width() + TEXT_WIDTH; //处理编码界面的CheckBox
							}
						}
					}
					else if(pAL->pPre->m_pOwner != NULL)
					{
						CRect owner_rect;
						owner_rect  = pAL->pPre->m_pOwner->m_Rect;
						this->ScreenToClient(&owner_rect);
						if((pAL->pCur->m_dwStyle & smallWidth) == smallWidth)
						{
							left =  owner_rect.right + 2;
						}
						else if(pAL->pCur->m_iType == IT_CHECKBOX && !strcmp(pAL->pCur->GetText(),"")) //对录像定时界面的 CheckBox 特殊处理
						{
							left =  owner_rect.right + pAL->width * 3/2;
						}
						else
						{
							left =  owner_rect.right + pAL->width;
						}				
					}
					else
					{
						if((pAL->pCur->m_dwStyle & smallWidth) == smallWidth)
						{
							left = rect.left + rect.Width() + 2;  /*两个控件的列间距*/
						}
						else
						{
							left = rect.left + rect.Width() + pAL->width;  /*两个控件的列间距*/
						}
					}
					break;
				}
			}
		}
	}

	return left;
}

/*设置控件的 Top 值*/
int CPage::SetCtrlTop(AUTO_LAYOUT *pAL)
{
	int top = 0;
	CRect temp_rect = m_Rect;
	this->ScreenToClient(&temp_rect);

	if(pAL->pPre == pAL->pCur) //每行的第一个控件
	{
		if(((pAL->pCur->m_dwStyle & inflateY) != inflateY)
			&& ((pAL->pCur->m_dwStyle & alignBottom) == alignBottom))
		{
			int temp = pAL->rowNum - pAL->curRow;
			//modefid by xuaiwei20070918 解决在设置了Margin的底部值时，自动布局页面出错的问题；
			top = temp_rect.bottom - 2*CLIENT_Y  - (temp + 1) * CTRL_HEIGHT - 4*temp-m_Margin.bottom;	
		}
		else if(1 == pAL->curRow) //第一行
		{
			top = pAL->startY;
		}
		else 
		{
			top = pAL->maxBottom + 4;
		}
	}
	else  //不是每行的第一个控件
	{
		CRect rect;
		rect = pAL->pPre->m_Rect;
		this->ScreenToClient(&rect);
		switch(pAL->pPre->m_iType)
		{
			//如果该上一个控件是由其他控件组成的,比如时间日期框.
			//那么我们需要对这类控件进行布局.
			//begin
		case IT_DATETIMECTRL:
		case IT_TIMINGCTRL:
			top = rect.top + 2;
			break;
		case IT_IPADDRESSCTRL:
			top = rect.top + 2;
			break;
		default :
			if(((pAL->pCur->m_dwStyle & inflateY) != inflateY)
				&& ((pAL->pCur->m_dwStyle & alignBottom) == alignBottom))
			{
				int temp = pAL->rowNum - pAL->curRow;
				top = m_Rect.Height() - 2*CLIENT_Y - m_TitleHeight - (temp + 1) * CTRL_HEIGHT - 4*temp;	
			}
			else
			{
				top = rect.top;
			}
			break;
		}
	}
	return top;
}

/*设置控件的 Right 值， 计算 right 前必须先计算 left 并且把 left 作为参数传进去*/
int CPage::SetCtrlRight(AUTO_LAYOUT *pAL, int left)
{
	int right = 0;
	if((pAL->pCur->m_dwStyle & inflateX) == inflateX)
	{
		CItem *pp;
		int column_num = 0;
		int total_width = 0;
		int ctrl_width = 0;
		int i = 0;
		VD_BOOL mark = FALSE;
		CRect temp_rect = m_Rect;
		this->ScreenToClient(&temp_rect);

		for(i = pAL->curColumn + 1, pp = pAL->pCur->m_pBrother; i <= pAL->row[pAL->curRow - 1]; i++, pp = pp->m_pBrother)
		{
			if(pp->m_Rect.Width() != 0)
			{
				ctrl_width = pp->m_Rect.Width();
			}
			else if(pp->GetDefaultSize().w != 0)
			{
				ctrl_width = pp->GetDefaultSize().w;
			}
			else
			{
				printf(">>>>>>>>>>>> Ctrl %d don't set width!!!!!! \n",pp->m_iType);
				ctrl_width = TEXT_WIDTH * 6; 
			}

			if((pAL->pCur->m_dwStyle & alignTableMask) != 0)
			{
				int column = (pAL->pCur->m_dwStyle & alignTableMask) >> 24;
				right = pAL->columnLeft[column - 1] - total_width - pAL->width * column_num;
				mark = TRUE;
				break; //退出循环，不再遍历控件
			}
			total_width += ctrl_width;
			column_num++;
		}

		if(!mark)
		{
			right = temp_rect.right - total_width - pAL->width * column_num - pAL->startX;
		}
	}
	else
	{
		if(pAL->pCur->m_Rect.Width() != 0)
		{
			if(pAL->reRowLayout && 
				pAL->pCur->m_iType == IT_NUMBERBOX &&
				pAL->pCur->m_pOwner == NULL)
			{
				right = left + pAL->pCur->m_Rect.Width()/2;
			}
			else
			{
				right = left + pAL->pCur->m_Rect.Width();
			}	
		}
		else if(pAL->pCur->GetDefaultSize().w != 0)
		{

			if(pAL->pCur->m_iType == IT_STATIC
				|| pAL->pCur->m_iType == IT_BUTTON
				|| pAL->pCur->m_iType == IT_COMBOBOX)
			{
				int offset = pAL->pCur->m_iType == IT_STATIC ? 0 : 16;
				int temp = pAL->pCur->GetDefaultSize().w >= (g_Locales.GetTextExtent(pAL->pCur->GetText()) + offset) ?  
							pAL->pCur->GetDefaultSize().w : (g_Locales.GetTextExtent(pAL->pCur->GetText()) + offset);
				right = left + temp;
			}
			else if(pAL->pCur->m_iType == IT_CHECKBOX && strcmp(pAL->pCur->GetText(),""))
			{
				right = left + pAL->pCur->GetDefaultSize().w + 4;
			}
			else
			{
				right = left + pAL->pCur->GetDefaultSize().w;
			}
		}
		else
		{
			if(pAL->pCur->m_iType == IT_GROUPBOX)
			{
				right = m_Rect.Width() - CLIENT_X * 2;
			}
			else // 如果都是O的话,那么是不是出了什么问题.哈哈.
			{
				printf(">>>>>>>>>>>> Ctrl %d don't set width!!!!!! \n",pAL->pCur->m_iType);
				right = left + TEXT_WIDTH * 6;
			}
		}
	}
	return right;
}

/*设置控件的 bottom 值， 计算 bottom 前必须先计算 top 并且把 top 作为参数传进去*/
int CPage::SetCtrlBottom(AUTO_LAYOUT *pAL, int top)
{
	int bottom = 0;
	CRect temp_rect = m_Rect;
	this->ScreenToClient(&temp_rect);
	if((pAL->pCur->m_dwStyle & inflateY) == inflateY)
	{
		int temp = pAL->rowNum - pAL->curRow;
		//modefid by xuaiwei20070918 解决在进行膨胀时，自动布局控件超出页面的问题；
		bottom = temp_rect.bottom - (CTRL_HEIGHT + 4) * temp - 2 * (pAL->startY)-m_Margin.bottom; 
	}
	else
	{
		if(pAL->pCur->m_Rect.Height() != 0)
		{
			bottom = top + pAL->pCur->m_Rect.Height();
		}
		else if(pAL->pCur->GetDefaultSize().h != 0)
		{
			if(pAL->pCur->m_iType == IT_CHECKBOX && strcmp(pAL->pCur->GetText(), ""))
			{
				bottom = top + pAL->pCur->GetDefaultSize().h + 4;
			}
			else if(pAL->pCur->m_iType == IT_GROUPBOX && !strcmp(pAL->pCur->GetText(), ""))
			{
				bottom = top + pAL->pCur->GetDefaultSize().h / 2;
			}
			else
			{
				bottom = top + pAL->pCur->GetDefaultSize().h;
			}			
		}
		else
		{
			printf(">>>>>>>>>>>> Ctrl %d no height!!!!!!!!!!!! \n", pAL->pCur->m_iType);
			bottom = top + CTRL_HEIGHT;
		}
	}

	/*设置 maxBottom */
	if(pAL->pCur == pAL->pPre)
	{
		pAL->maxBottom = bottom;
	}
	else
	{
		pAL->maxBottom = pAL->maxBottom >= bottom ? pAL->maxBottom : bottom;
	}
	return bottom;
}
void CPage::LayoutInit(AUTO_LAYOUT *pAL)
{
	pAL->pCur = m_pChildHead;
	pAL->head = pAL->pCur;
	pAL->pPre = pAL->pCur;
	pAL->pLineHead = NULL;

	pAL->rowNum = 1;
	pAL->curRow = 1;
	pAL->curColumn = 1;
	pAL->column = 0;
	pAL->width = m_iSpace;

	pAL->reRowLayout = FALSE;
	pAL->secondLayout = FALSE; 
	
	memset(pAL->row,0,sizeof(pAL->row));

	/*初始化起始坐标*/
	pAL->startX = m_iStartX;
	pAL->startY = m_iStartY;

	pAL->maxBottom = pAL->startY;

	//初始化列队齐Left
	for(int i = 0; i < MAX_TABLE_NUM; i++)
	{
		pAL->columnLeft[i] = m_iTable[i];
	}

	//获得页面控件需要自动布局的首控件指针
	//如果该页面上有关闭控件,应该不放在自动布局中.
	//我们认为他的下一个控件为首控件
	while((pAL->pCur && !pAL->pCur->GetFlag(IF_ORDERED)) || (pAL->pCur && pAL->pCur->GetFlag(IF_POPUP)))
	{
		pAL->pCur = pAL->pCur->m_pBrother;
	}
	pAL->head = pAL->pCur;
	pAL->pPre = pAL->pCur;
	return;
}

/*设置控件间距*/
void CPage::SetCtrlSpace(int width)
{
	if(width > 0)
	{
		m_iSpace = width;
	} 
	return ;
}
void CPage::SetStartPoint(int x/* = CLIENT_X*/, int y/* = CLIENT_Y*/)
{
	m_iStartX = x;
	m_iStartY = y;
	return;
}

void CPage::SetCtrlSpace(LayoutSpace type, int space)
{
	if (m_playoutSpace == sm_layoutSpace)
	{
		m_playoutSpace = new int[spaceCount];
	}
	m_playoutSpace[type] = space;
}

void CPage::SetPageIcon(PAGEICON pageIcon, VD_BITMAP* bmp)
{
	if (bmp)
	{
		switch(pageIcon)
		{
		case pageIconSmall:
			m_pIconBmp = bmp;
			break;
		case pageIconNor:
			m_pNorBmp = bmp;
			break;
		case pageIconSel:
			m_pSelBmp = bmp;
			break;
		default:
			break;
		}
	}
}
VD_BITMAP* CPage::GetPageIcon(PAGEICON pageIcon)
{
	VD_BITMAP* bmp = NULL;
	switch(pageIcon)
	{
	case pageIconSmall:
		  bmp = m_pIconBmp;
		break;
	case pageIconNor:
		bmp = m_pNorBmp;
		break;
	case pageIconSel:
		bmp = m_pSelBmp;
		break;
	default:
		break;
	}
	return bmp;
}

VD_PCSTR  CPage::GetTip(void)
{
	return tip.c_str();
}

void CPage::SetParentPage(CPage* pParent)
{
	if(m_pParent)
	{
		m_pParent->DeleteItem(this);
	}
	m_pParent = pParent;
	if(m_pParent)
	{
		m_pParent->AddItem(this);
	}
} 
void CPage::SetTable(int TableNum, int left)
{
	if(TableNum >= 1 && TableNum <= MAX_TABLE_NUM)
	{
		m_iTable[TableNum-1] = m_iStartX + left;
	}
	return;
}

void CPage::SetDefaultBitmap(PageBitmap region, VD_PCSTR name)
{
	switch(region) {
	case PGB_KEY_PAGEUP:
		bmp_key_pageup = VD_LoadBitmap(name);
		break;
	case PGB_KEY_PAGEDOWN:
		bmp_key_pagedown = VD_LoadBitmap(name);
		break;
	case PGB_KEY_FUNCTION:
		bmp_key_function = VD_LoadBitmap(name);
		break;
	case PGB_KEY_ENTER:
		bmp_key_enter = VD_LoadBitmap(name);
		break;
	case PGB_CLOSE_BUTTON_NORMAL:
		bmp_button_close[0] = VD_LoadBitmap(name);
		break;
	case PGB_CLOSE_BUTTON_SELECTED:
		bmp_button_close[1] = VD_LoadBitmap(name);
		break;
	case PGB_CLOSE_BUTTON_PUSHED:
		bmp_button_close[2] = VD_LoadBitmap(name);
		break;
	case PGB_CLOSE_BUTTON_DISABLED:
		bmp_button_close[3] = VD_LoadBitmap(name);
		break;
	case PGB_ICON_SMALL:
		icon_dvr = VD_LoadBitmap(name);
		break;
	default:
		break;
	}
}
