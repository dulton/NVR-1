//
//  "$Id: Item.cpp 279 2008-12-17 05:58:45Z liwj $"
//
//  Copyright (c)2008-2010, RealVision Technology CO., LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#include "GUI/Ctrls/Page.h"
//#include "GUI/Pages/PagePlay.h"

/*///////////////////////////////////////////////////////////////////////////////////////////////
GUI内核中主要有2种锁，用来保证GUI对象的状态一致性，同时保证多线程同时工作的效率。而对于页面内部的
第一种锁：m_DC.m_Mutex 用来保护m_DC的区域，GDI对象，等等，通过操作m_DC.Lock()和m_DC.UnLock()来访问。
第二种锁：m_MutexMgr 用来保护活动页面链表，包括顶层页面，所有活动页面之间的链接关系，等等。典型的
	应用场合是打开或关闭页面时，需要用到这个锁，保证页面的关系不会出错。
为了避免死锁，必须保证m_MutexMgr的保护范围总是应大于m_Mutex,不能出现逆向的情况。
同时页面m_DC.m_Mutex保护的范围应大于其子控件m_DC.m_Mutex保护的范围。

页面内部的控件链表，以及其他成员变量是没有加锁的，因为大部分情况下访问成员变量的只有一个线程，如
创建页面的线程或者页面自身的消息处理线程。如果有特别需要，由应用层提供锁保护，而且应用层的锁的范围
应该确定，不能和m_MutexMgr或者m_DC.m_Mutex有双向的包含关系。

对于Draw()函数，因为(要求)其不改变控件的属性和m_DC的区域，所以使用m_DC.m_Mutex保护即可。
///////////////////////////////////////////////////////////////////////////////////////////////*/

///////////////////////////////////////////////////////////////////////////////
/////////             CItemThread
void CItemThread::ThreadProc()
{
	VD_MSG msg;
	
	//printf("CItemThread::ThreadProc:start......\n");
	//fflush(stdout);
	
	do
	{
		//printf("CItemThread::ThreadProc:entry loop......\n");
		//fflush(stdout);
		
		VD_RecvMessage(&msg);
		
		//printf("CItemThread::ThreadProc:VD_RecvMessage finish......\n");
		//fflush(stdout);

		//printf("yg VD_RecvMessage msg: 0x%x, %d,%d \n", msg.msg, VD_HIWORD(msg.lpa),VD_LOWORD(msg.lpa));
		
		if(msg.msg == XM_QUIT)
		{
			//printf("CItemThread::ThreadProc:XM_QUIT\n");
			//fflush(stdout);
			
			break;//线程出口1
		}
		
		//printf("CItemThread::ThreadProc:recv msg:%d\n",msg.msg);
		//fflush(stdout);
		
		//忽略在300毫秒之前发送的消息// 为什么忽略300毫秒前的消息?  nike.xie
		//modefied by xie  不遗漏左键消息。
		if ( (SystemGetMSCount() - msg.time < 300) || (XM_LBUTTONDOWN == msg.msg)  || (XM_LBUTTONUP == msg.msg))	
		{
			if(!m_pItem->PreMsgProc(msg.msg, msg.wpa, msg.lpa))
			{
				//printf("yg PreMsgProc\n");
				m_pItem->MsgProc(msg.msg, msg.wpa, msg.lpa);
				
				//csp modify
				//printf("CItemThread::ThreadProc:GetModify=%d\n",CDC::GetModify());
				//fflush(stdout);
				if(CDC::GetModify())
				{
					//printf("CItemThread::ThreadProc:GraphicsSyncVgaToCvbs\n");
					//fflush(stdout);
					GraphicsSyncVgaToCvbs();
					CDC::SetModify(FALSE);
				}
			}
			else
			{
				//csp modify
				if(CDC::GetModify())
				{
					GraphicsSyncVgaToCvbs();
					CDC::SetModify(FALSE);
				}
			}
		}
		
		//printf("CItemThread::ThreadProc:deal msg finish\n");
		//fflush(stdout);
	} while (m_bLoop);//线程出口2
	
	//printf("CItemThread::ThreadProc:quit loop\n");
	//fflush(stdout);
	
	if(m_pItem->m_iType == IT_PAGE)
	{
		CPage *pParentPage = ((CPage*)m_pItem)->GetParentPage();
		if(pParentPage)
		{
			pParentPage->OnChildClosed((CPage*)m_pItem);
			
			//csp modify
			if(CDC::GetModify())
			{
				GraphicsSyncVgaToCvbs();
				CDC::SetModify(FALSE);
			}
		}
	}
	
	//printf("CItemThread::ThreadProc:thread over\n");
	//fflush(stdout);
}

//采用短消息队列策略，鼠标移动时，占用的CPU少一点，但也会出现其他消息漏掉的情况?
CItemThread::CItemThread(VD_PCSTR name, CItem * pItem) : CThread(name, TP_GUI, 16)
{
	m_pItem = pItem;
}

CItemThread::~CItemThread()
{
	
}

///////////////////////////////////////////////////////////////////
////////////        CItem
int CItem::m_nInstance[IT_NR] = {0, };
CItem *CItem::m_pActive[] = {NULL, };
CItem *CItem::m_pTop[] = {NULL, };
CItem *CItem::m_pNoTopMost[] = {NULL, };
CItem * CItem::m_pInputTray[] = {NULL, };
CItem * CItem::m_pSpellPad[] = {NULL, };
VD_BOOL CItem::m_bKeyInputFlags[KIF_NR] = {TRUE, TRUE, TRUE, TRUE};
CMutex CItem::m_MutexMgr(MUTEX_RECURSIVE);

/************************************************************************/
/*
1/修改了CItem类的实现，将控件的属性，比如POPUP/EDIT等属性通过设置标志位进行，在
控件创建的时候将标志传入，参数style为控件自己的参数.
POPUP/styleEditable等属性由创建处传入，通过与m_dwStyle进行位运算获得。

2/ Item控件不包含任何默认属性。所有的属性移植到控件中。
如果有冲突的地方，则必须在创建的时候手动传入这个参数。
						--by wangqin  20070315		
*/
/************************************************************************/
//CItem::CItem(PCRECT pRect, CPage * pParent, ITEM_TYPE type, BOOL vedit /* = TRUE */,BOOL vpopup /* = FALSE */, BOOL vautofocus /* = TRUE */)
CItem::CItem(VD_PCRECT pRect, CPage * pParent, ITEM_TYPE type,uint style)
:m_screen(GetScreen(pParent)), m_dwStyle(style),m_dwFlags(0),m_DC(m_screen + DC_SCREEN)
{
	//csp modify 20130323
	//if(m_screen < 0 || m_screen > 2)
	if(m_screen != 0)
	{
		printf("#####################################################warning:m_screen error:0x%08x\n",m_screen);
	}
	
	m_pThread = NULL;
	m_pDown = m_pUp = NULL;
	m_pBrother = NULL;
	m_pBrotherR = NULL;
	m_pRelatedItems = NULL;
	m_iType = type;
	m_pParent = reinterpret_cast<CPage *>(pParent);
	m_pOwner = NULL;
	m_dwFlags = 0;
	
	m_pDevGraphics = CDevGraphics::instance(m_screen);
	m_pCursor = CCursor::instance(m_screen);
	
	if(pRect)
	{
		m_Rect = *pRect;
	}
	else//防止在自动布局时出错，所以要先清空m_Rect
	{
		m_Rect.SetRectEmpty();
	}
	
	tip.erase();
	
	//states
	SetFlag(IF_OPENED, FALSE);
	SetFlag(IF_EDITABLED, m_dwStyle&styleEditable);
	SetFlag(IF_POPUP, m_dwStyle&stylePopup);
	SetFlag(IF_AUTOFOCUS, m_dwStyle&styleAutoFocus);
	
	SetFlag(IF_ENABLED, TRUE);
	SetFlag(IF_CAPTURED, FALSE);
	SetFlag(IF_FOCUSED, FALSE);
	
	if(m_dwStyle&stylePopup)
	{
		SetFlag(IF_SHOWN, FALSE);
	}
	else
	{
		SetFlag(IF_SHOWN, TRUE);
	}
	SetFlag(IF_SELECTED, FALSE);
	SetFlag(IF_ORDERED, TRUE);

	m_Margin.SetRectEmpty();

	//increase instances count
	m_nInstance[m_iType]++;

	//rect
	if(m_pParent)
	{
		m_pParent->ClientToScreen(&m_Rect);
		m_pParent->AddItem(this);//在父项中的子项链表中添加此项
	}
}

CItem::~CItem()
{
	if(m_pParent)
	{
		m_pParent->DeleteItem(this);
	}

	//decrease instances count
	m_nInstance[m_iType]--;
}

void CItem::ItemProc(CTRLPROC proc)
{
	if(proc)
	{
		m_pCursor->SetCursor(CR_WAIT);
		if(m_pOwner)
		{
			//printf("ItemProc:m_pOwner \n");
			(m_pOwner->*proc)();
		}
		else if (m_pParent)
		{
			//printf("ItemProc:m_pParent \n");
			(m_pParent->*proc)();
		}
		m_pCursor->SetCursor(CR_NORMAL);
	}
}

void CItem::ShowTip(VD_BOOL flag)
{
	if(m_pParent){
		if(flag && tip.size()){
			m_pParent->ShowTip(tip.c_str());
		}else{
			m_pParent->ShowTip(NULL);
		}
	}
}

VD_SIZE& CItem::GetDefaultSize(void)
{
	static VD_SIZE size = {0};
	return size;
}

//检查规则：控件已打开，且已显示，而且父页面已打开且已显示
VD_BOOL CItem::DrawCheck()
{
	return (GetFlag(IF_OPENED) && GetFlag(IF_SHOWN) && (dynamic_cast<CPage*>(this) || !m_pParent || m_pParent->DrawCheck()));
}

//设置是否能被编辑
void CItem::Editable(VD_BOOL flag)
{
	SetFlag(IF_EDITABLED, flag);
}

void CItem::Attach()
{
	if(m_dwStyle & (pageTopMost | pageActiveMost))
	{

		if(m_pTop[m_screen])
		{
			m_pTop[m_screen]->m_pUp = this;
		}
		m_pDown = m_pTop[m_screen];
		m_pUp = NULL;
		m_pTop[m_screen] = this;

	}
	else
	{

		if(m_pNoTopMost[m_screen])
		{
			m_pUp = m_pNoTopMost[m_screen]->m_pUp;
			if(m_pUp)
			{
				m_pUp->m_pDown = this;
			}
			m_pNoTopMost[m_screen]->m_pUp = this;
		}
		m_pDown = m_pNoTopMost[m_screen];
		if(m_pTop[m_screen] == m_pNoTopMost[m_screen])
		{
			m_pTop[m_screen] = this;
		}
		m_pNoTopMost[m_screen] = this;
	}
}

void CItem::Detach()
{
	if(m_pUp)
	{
		m_pUp->m_pDown = m_pDown;
	}
	if(m_pDown)
	{
		m_pDown->m_pUp = m_pUp;
	}
	if(m_pNoTopMost[m_screen] == this)
	{
		m_pNoTopMost[m_screen] = m_pDown;
	}
	if(m_pTop[m_screen] == this)
	{
		m_pTop[m_screen] = m_pDown;
	}
}

void CItem::Activate()
{
	if(dynamic_cast<CPage*>(m_pActive[m_screen]))
	{
		m_pActive[m_screen]->VD_SendMessage(XM_DEACTIVATE);
	}
	m_pActive[m_screen] = this;
	if(dynamic_cast<CPage*>(m_pActive[m_screen]))
	{
		m_pActive[m_screen]->VD_SendMessage(XM_ACTIVATE);
	}
}

//#define GUI_TEST//csp modify

void CItem::Show(VD_BOOL flag, VD_BOOL redraw /* = TRUE */)
{
	if(flag && GetFlag(IF_SHOWN) || !flag && !GetFlag(IF_SHOWN)) //状态没有改变，退出
	{
		return;
	}
	
	CGuard guard(m_MutexMgr);
	
//	printf("CItem::Show 1 \n");
	SetFlag(IF_SHOWN, flag);
	
//	printf("CItem::Show 2 \n");
	if(GetFlag(IF_POPUP))
	{
//		printf("CItem::Show 3 flag:%d \n", flag);
		if(flag)
		{
			// attach to link
//			printf("CItem::Show 3.1 \n");
			Attach();

//			printf("CItem::Show 3.2 \n");
			// activate it
			if(!(m_dwStyle & pageActiveMost))
			{
				Activate();
			}

//			printf("CItem::Show 3.3 \n");
			//set rect when opening, needed?
			UpdateRgn();
//			printf("CItem::Show 3.4 \n");
			GlobalInvalidateRect(m_Rect);
			
//			printf("CItem::Show 3.5 \n");
			//draw items
			if(redraw)
			{
				Draw();
			}
//			printf("CItem::Show 3.6 \n");
		}
		else
		{
#ifdef GUI_TEST
			if(CDC::GetModify())//csp modify
			{
				printf("CItem::Show:framebuffer changed-1\n");
				CDC::SetModify(FALSE);
			}
#endif
			
//			printf("CItem::Show 3.7 \n");
			// detech from link
			Detach();
			
#ifdef GUI_TEST
			if(CDC::GetModify())//csp modify
			{
				printf("CItem::Show:framebuffer changed-2\n");
				CDC::SetModify(FALSE);
			}
#endif
			
//			printf("CItem::Show 3.8 \n");
			// if this item is active, activate the down item of it in stead.
			if(m_pActive[m_screen] == this)
			{
				if(m_pDown)
				{
					m_pActive[m_screen] = NULL;
					m_pDown->Activate();
				}
			}
			
#ifdef GUI_TEST
			if(CDC::GetModify())//csp modify
			{
				printf("CItem::Show:framebuffer changed-3\n");
				CDC::SetModify(FALSE);
			}
#endif
			
//			printf("CItem::Show 3.9 \n");
			GlobalValidateRect(m_Rect);
			
#ifdef GUI_TEST
			if(CDC::GetModify())//csp modify
			{
				printf("CItem::Show:framebuffer changed-4\n");
				CDC::SetModify(FALSE);
			}
#endif
			
//			printf("CItem::Show 3.10 \n");
			ClearRgn();
			
#ifdef GUI_TEST
			if(CDC::GetModify())//csp modify
			{
				printf("CItem::Show:framebuffer changed-5\n");
				CDC::SetModify(FALSE);
			}
#endif
			
//			printf("CItem::Show 3.11 \n");
			m_pUp = m_pDown = NULL;
		}
	}
	else
	{
//		printf("CItem::Show 4 \n");
		if(flag)
		{
			//重新计算区域
			VD_RECT rect;
			m_pParent->m_DC.Lock();
			m_DC.Lock();

			m_DC.SetBound(m_Rect);
			m_pParent->GetRect(&rect);
			m_pParent->m_DC.SubtractRect(m_Rect);
			m_DC.IntersectRect(&rect);
			
			CItem * pItem = m_pParent->m_pUp;
			while(pItem)
			{
				pItem->GetRect(&rect);
				m_DC.SubtractRect(&rect);
				pItem = pItem->m_pUp;
			}
			
			m_DC.UnLock();
			m_pParent->m_DC.UnLock();
			
			if(redraw)
			{
				Draw();
			}
		}
		else
		{
			VD_RECT rect;
			CRgn rgn;
			m_pParent->m_DC.Lock();
			m_DC.Lock();

			m_DC.SwapRgn(&rgn);

			// 重绘兄弟控件被遮挡区域，而兄弟控件区域不需要更新。有一个bug，当两个以上的控件空间重叠时，
			// 重绘顺序不一定是所期望的，以后兄弟控件可能也要进行Z序排列.
			CItem *p = m_pParent->m_pChildHead;
			while(p){
				if(!p->GetFlag(IF_POPUP) && p->GetFlag(IF_SHOWN) && !p->m_pOwner && p != this)
				{
					p->GetRect(&rect);
					if(rgn.RectInRegion(&rect))
					{
						if(redraw)
						{
							CRgn rgnItem;
							p->m_DC.Lock();
							p->m_DC.SwapRgn(&rgnItem);
							p->m_DC.SetBound(&p->m_Rect);
							p->m_DC.IntersectRgn(&rgn);
							p->Draw();
							p->m_DC.SwapRgn(&rgnItem);
							p->m_DC.UnLock();
						}
						rgn.SubtractRect(&rect);
					}
				}
				p = p->m_pBrother;
			}

			// 重绘父页面被遮挡区域，保存在rgn中。
			if(redraw)
			{
				m_pParent->m_DC.SwapRgn(&rgn);
				m_pParent->m_bDrawBkgdOnly = TRUE;
				m_pParent->Draw();
				m_pParent->m_bDrawBkgdOnly = FALSE;
				m_pParent->m_DC.SwapRgn(&rgn);
			}

			// 然后更新父页面区域。
			m_pParent->m_DC.UnionRgn(&rgn);

			m_DC.UnLock();
			m_pParent->m_DC.UnLock();
		}
	}
}

//激活/禁止控件
void CItem::Enable(VD_BOOL flag)
{
	if(flag && GetFlag(IF_ENABLED) || !flag && !GetFlag(IF_ENABLED)) //状态没有改变，退出
	{
		return;
	}

	SetFlag(IF_ENABLED, flag);
	Draw();
}

//选中/不选中控件
void CItem::Select(VD_BOOL flag)
{
	
	if(m_pParent){
		if(flag && tip.size()){
			m_pParent->ShowTip(tip.c_str());
		}else{
			m_pParent->ShowTip(NULL);
		}
	}
	
	
	SetFlag(IF_SELECTED, flag);
	if(m_pOwner && m_pOwner->m_iType != IT_LISTBOX)
	{
		m_pOwner->Select(flag);
	}
	//printf("Select \n");

	
	Draw();
}

void CItem::SetFocus(VD_BOOL flag)
{
	SetFlag(IF_FOCUSED, flag);
	if(!flag)
	{
		SetFlag(IF_CAPTURED, FALSE);
	}
	if(!GetFlag(IF_AUTOFOCUS))
	{
		//printf("!GetFlag(IF_AUTOFOCUS)  \n");
		Draw();
	}
}

VD_BOOL CItem::GetFlag(ITEM_FLAG index)
{
	return (m_dwFlags&BITMSK(index)) ? TRUE : FALSE;
}

void CItem::SetFlag(ITEM_FLAG index, VD_BOOL flag)
{
	if(flag)
	{
		m_dwFlags |= BITMSK(index);
	}
	else
	{
		m_dwFlags &= ~BITMSK(index);
	}
}

uint CItem:: ReadFlag()//cw_mainpage
{
	return m_dwFlags;
}
//设置自动聚焦
void CItem::SetAutoFocus(VD_BOOL flag)
{
	SetFlag(IF_AUTOFOCUS, TRUE);
}

//得到显示/隐藏状态
VD_BOOL CItem::IsShown()
{
	return GetFlag(IF_SHOWN);
}

//得到激活/禁止状态
VD_BOOL CItem::IsEnabled()
{
	return GetFlag(IF_ENABLED);
}

//得到选中/不选中状态
VD_BOOL CItem::IsSelected()
{
	return GetFlag(IF_SELECTED);
}

//设置鼠标捕获
void CItem::SetCapture(VD_BOOL flag)
{
	SetFlag(IF_CAPTURED, flag);
}

//得到鼠标捕获状态
VD_BOOL CItem::GetCapture()
{
	return GetFlag(IF_CAPTURED);
}

//得到输入焦点状态
VD_BOOL CItem::GetFocus()
{
	return GetFlag(IF_FOCUSED);
}

//设置拥有者
VD_BOOL CItem::SetOwner(CItem * pOwner)
{
	m_pOwner = pOwner;
	return TRUE;
}

//得到拥有者
CItem * CItem::GetOwner()
{
	return m_pOwner;
}

//得到所在控件
CItem* CItem::GetParent()
{
	return m_pParent;
}

//得到兄弟控件
CItem* CItem::GetBrother()
{
	return m_pBrother;
}

CItem* CItem::GetActive(int screen)
{
	return m_pActive[screen];
}

CItem* CItem::SetActive()
{
	return m_pActive[m_screen]; // 屏蔽显示激活页面功能
	
#if 0//csp modify
	CGuard guard(m_MutexMgr);
	CPage* pHolder = dynamic_cast<CPage*>(this);
	
	if(this == m_pActive[m_screen] || pHolder == NULL)
	{
		return m_pActive[m_screen];
	}
	
	CItem* pNewActive = this;
	
	// 关键是处理对话框激活的问题
	if((m_dwStyle & pageDialog) && pHolder->m_pPopupCaller)
	{
		pHolder = pHolder->m_pPopupCaller;
	}
	
	// 目标页面的对话框已经激活，无需处理
	CItem *pItem = pHolder->m_pUp;
	while (pItem)
	{
		CPage *pPage = dynamic_cast<CPage*>(pItem);
		if(pPage && pPage->m_pPopupCaller == pHolder && pPage == m_pActive[m_screen])
		{
			return m_pActive[m_screen];
		}
		pItem = pItem->m_pUp;
	}
	
	// 底下没有其他页面的页面被认为是桌面，激活桌面时只设置激活状态，不改变Z序
	if(m_pDown == NULL)
	{
		Activate();
		return m_pActive[m_screen];
	}
	
	// 调整Z序，计算待更新的区域
	CRgn rgn;
	pHolder->Detach();
	pHolder->Attach();
	rgn.SetRectRgn(pHolder->m_Rect);
	
	pItem = pHolder->m_pUp;
	while(pItem && pItem != pHolder)
	{
		CPage *pPage = dynamic_cast<CPage*>(pItem);
		if(pPage && pPage->m_pPopupCaller == pHolder)
		{
			pItem->Detach();
			pItem->Attach();
			rgn.SubtractRect(pItem->m_Rect);
			rgn.UnionRect(pItem->m_Rect);
			pNewActive = pItem;
		}
		else
		{
			rgn.IntersectRect(pItem->m_Rect);
		}
		pItem = pItem->m_pUp;
	}
	
	//实施更新
	pHolder->UpdateRgn();
	pItem = pHolder->m_pUp;
	while(pItem && pItem != pHolder)
	{
		CPage *pPage = dynamic_cast<CPage*>(pItem);
		if(pPage && pPage->m_pPopupCaller == pHolder)
		{
			pItem->UpdateRgn();
		}
		pItem = pItem->m_pUp;
	}
	
	pHolder->GlobalInvalidateRgn(&rgn);
	
	pItem = m_pTop[m_screen];
	while(pItem)
	{
		CPage *pPage = dynamic_cast<CPage*>(pItem);
		VD_RECT rect;
		pItem->GetRect(&rect);
		if(pPage && (pPage == pHolder || pPage->m_pPopupCaller == pHolder))
		{
			if(rgn.RectInRegion(&rect))
			{
				CRgn validRgn;
				validRgn.CreateRectRgn(&rect);
				validRgn.IntersectRgn(&rgn);
				pItem->ValidateRgn(&validRgn);
			}
		}
		rgn.SubtractRect(&rect);
		pItem = pItem->m_pDown;
		if(pItem == pHolder)
		{
			break;
		}
	}
	pHolder->GlobalValidateRgn(&rgn);
	
	//激活新的页面
	pNewActive->Activate();
	
	return m_pActive[m_screen];
#endif
}

VD_BOOL CItem::IsActive()
{
	return (this == m_pActive[m_screen]);
}

void CItem::SetTip(VD_PCSTR psz)
{
	if(psz)
	{
		tip = GetParsedString(psz);
	}
	else
	{
		tip.erase();
	}
}

void  CItem::GetRect(VD_PRECT pRect) const
{
	*pRect = m_Rect;
}

void CItem::ScreenToClient(VD_PPOINT pPoint)
{
	pPoint->x -= m_Rect.left + m_Margin.left;
	pPoint->y -= m_Rect.top + m_Margin.top;
}

void CItem::ScreenToClient(VD_PRECT pRect)
{
	::OffsetRect(pRect, -(m_Rect.left + m_Margin.left), -(m_Rect.top + m_Margin.top));
}

void CItem::ClientToScreen(VD_PRECT pRect)
{
	::OffsetRect(pRect, (m_Rect.left + m_Margin.left), (m_Rect.top + m_Margin.top));
}

void  CItem::GetClientRect(VD_PRECT pRect)
{
	pRect->left = 0;
	pRect->top = 0;
	pRect->right = m_Rect.Width() - m_Margin.left - m_Margin.right;
	pRect->bottom = m_Rect.Height() - m_Margin.top - m_Margin.bottom;
}

void CItem::DrawBackground()
{

	if(!GetFlag(IF_OPENED) || !m_pParent || m_pParent->m_bDrawBkgdOnly == TRUE)
	{
		return;
	}

	
	CRgn rgn;

	//注意控件的Lock和页面的Lock嵌套的顺序, 只能是页面嵌套控件的
	m_pParent->m_DC.Lock();
	m_DC.Lock();
	m_DC.SwapRgn(&rgn);
	m_pParent->m_DC.SwapRgn(&rgn);
	m_pParent->m_bDrawBkgdOnly = TRUE;
	m_pParent->Draw();
	m_pParent->m_bDrawBkgdOnly = FALSE;
	m_pParent->m_DC.SwapRgn(&rgn);
	m_DC.SwapRgn(&rgn);
	m_DC.UnLock();
	m_pParent->m_DC.UnLock();
}

CTRL_CLASS ctrl_classes[IT_NR] = {
	{"BitmapButtons    ", sizeof(CBitmapButton)},
	{"Buttons          ", sizeof(CButton)},
	{"CheckBoxs        ", sizeof(CCheckBox)},
	{"ComboBoxs        ", sizeof(CComboBox)},
	{"DateTimeCtrls    ", sizeof(CDateTimeCtrl)},
	{"Edits            ", sizeof(CEdit)},
	{"GridCtrls        ", sizeof(CGridCtrl)},
	{"GroupBoxs        ", sizeof(CGroupBox)},
	{"IPAddressCtrls   ", sizeof(CIPAddressCtrl)},
	{"ListBoxs         ", sizeof(CListBox)},
	{"Menus            ", sizeof(CMenu)},
	{"NumberBoxs       ", sizeof(CNumberBox)},
	{"NumberPads       ", sizeof(CNumberPad)},
	{"Pages            ", sizeof(CPage)},
	{"ProgressCtrls    ", sizeof(CProgressCtrl)},
	{"ScrollBars       ", sizeof(CScrollBar)},
	{"SliderCtrls      ", sizeof(CSliderCtrl)},
	{"Statics          ", sizeof(CStatic)},
	{"TimingCtrls      ", sizeof(CTimingCtrl)},
	{"Radios           ", sizeof(CRadio)},
	{"Calendars        ", sizeof(CCalendar)},
//	{"TimeTable        ", sizeof(CTimeTable)},
	{"TableBoxs        ", sizeof(CTableBox)},
	{"RecTimeBox       ", sizeof(CRecFileTimeBox)},
};

void CItem::DumpInstances()
{
	/*
	trace("GUI Objects:\n");
	unsigned long size = 0;
	int n = 0;
	for(int i = 0; i < IT_NR; i++)
	{
		trace("%16s %8d x %8d\n", ctrl_classes[i].name, m_nInstance[i], ctrl_classes[i].size);
		size += m_nInstance[i] * ctrl_classes[i].size;
		n += m_nInstance[i];
	}
	trace("%d Items have been created, Total size = %ld\n", n, size);
	*/
}

//打开页面
VD_BOOL CItem::Open()
{
	m_MutexMgr.Enter();
	if(GetFlag(IF_OPENED)){
		
		m_MutexMgr.Leave();
		return FALSE;
	}
	SetFlag(IF_OPENED, TRUE);//立即将该状态置位,防止多次打开
	m_MutexMgr.Leave();

	//data exchange
	m_DC.Lock();
	m_DC.Enable(TRUE);
	m_DC.UnLock();

	if(GetFlag(IF_POPUP))
	{
		Show(TRUE);
	}
	else
	{
		if(m_pParent && m_pParent->IsShown()) //如果页面已经打开, 单独打开控件需要单独更新区域
		{
			UpdateRgn();
		}
	}
	return TRUE;
}

//关
VD_BOOL CItem::Close()
{
	m_MutexMgr.Enter();
	if(!GetFlag(IF_OPENED)){
		//trace("CItem::Close(),page `%s` is already closed.\n", pTitle);
		m_MutexMgr.Leave();
		//printf("CItem::Close() 111 \n");
		return FALSE;
	}
	SetFlag(IF_OPENED, FALSE);
	m_MutexMgr.Leave();

	m_DC.Lock();
	m_DC.Enable(FALSE);
	m_DC.UnLock();

	if(GetFlag(IF_POPUP))
	{
		//printf("CItem::Close() 222 \n");
		Show(FALSE);
		//printf("CItem::Close() 333 \n");
	}
	return TRUE;
}

//得到打开/关闭状态
VD_BOOL CItem::IsOpened()
{
	CGuard guard(m_MutexMgr);

	return GetFlag(IF_OPENED);
}

VD_BOOL CItem::InvalidateRgn(CRgn * pRgn)
{
	m_DC.Lock();
	m_DC.SubtractRgn(pRgn);
	m_DC.UnLock();

	return TRUE;
}

//可能会改变pRgn的内容
VD_BOOL CItem::ValidateRgn(CRgn * pRgn)
{
	//draw validate region
	m_DC.Lock();
	m_DC.SwapRgn(pRgn);
	m_DC.UnLock();

//	printf("ValidateRgn 000 \n");
	Draw();
//	printf("ValidateRgn 111 \n");
	//union old region, but rectangles are not mergered
	m_DC.Lock();
	m_DC.UnionRgn(pRgn);
	m_DC.UnLock();

	return TRUE;
}

void CItem::UpdateRgn()
{
	CRgn rgn;
	VD_RECT rect;

	CGuard guard(m_MutexMgr);
	if(!GetFlag(IF_OPENED))
	{
		return;
	}

	if(GetFlag(IF_POPUP))
	{
		m_DC.Lock();
		CItem * pItem = m_pUp;

		m_DC.SetBound(m_Rect);

		while(pItem)
		{
			//get rect of top item
			pItem->GetRect(&rect);
			
			//invalid unvisible region
			m_DC.SubtractRect(&rect);
			
			//point to next top item
			pItem = pItem->m_pUp;
		}
		m_DC.UnLock();
	}
	else
	{
		if(GetFlag(IF_SHOWN))
		{
			m_DC.Lock();
			m_DC.SetBound(m_Rect);
			m_pParent->GetRect(&rect);
			m_DC.IntersectRect(&rect);
			m_DC.UnLock();
			
			m_pParent->m_DC.Lock();
			m_pParent->m_DC.SubtractRect(m_Rect);
			m_pParent->m_DC.UnLock();
		}
	}
}

void CItem::ClearRgn()
{
	m_DC.Lock();
	m_DC.SetBound(m_Rect);
	m_DC.UnLock();
}

void CItem::GlobalInvalidateRect(VD_PCRECT pRect)
{
	CRgn rgn;

	rgn.CreateRectRgn(pRect);
	GlobalInvalidateRgn(&rgn);
}

void CItem::GlobalValidateRect(VD_PCRECT pRect)
{
	CRgn rgn;
	
	//printf("GlobalValidateRect %d %d %d %d \n", pRect->left, pRect->top, pRect->right, pRect->bottom);
	
	rgn.CreateRectRgn(pRect);
	GlobalValidateRgn(&rgn);
}

void CItem::GlobalInvalidateRgn(CRgn * pRgn)
{
	CItem * pItem = m_pDown;
	while(pItem)
	{
		pItem->InvalidateRgn(pRgn);
		pItem = pItem->m_pDown;
	}
}

void CItem::GlobalValidateRgn(CRgn * pRgn)
{
	CRgn rgn;
	CItem * pItem;
	VD_RECT rect;
	
	//剪去上层页面的区域
	pItem = m_pUp;
	while(pItem)
	{
		pItem->GetRect(&rect);
		pRgn->SubtractRect(&rect);
		pItem = pItem->m_pUp;
	}
	
	//printf("GlobalValidateRgn 000 \n");
	
	//刷新下层页面的区域
	pItem = m_pDown;
	while(pItem)
	{
		//printf("GlobalValidateRgn 111 \n");
		pItem->GetRect(&rect);
		//printf("GlobalValidateRgn 222 \n");
		if(pRgn->RectInRegion(&rect))
		{
			//printf("GlobalValidateRgn 333 \n");
			rgn.CreateRectRgn(&rect);
			rgn.IntersectRgn(pRgn);
			//printf("GlobalValidateRgn 444 \n");
			pItem->ValidateRgn(&rgn);
			//printf("GlobalValidateRgn 555 \n");
			pRgn->SubtractRect(&rect);
			//printf("GlobalValidateRgn 666 \n");
		}
		pItem = pItem->m_pDown;
	}

	//printf("GlobalValidateRgn end \n");
}

void CItem::SetRect(VD_PCRECT pRect, VD_BOOL bReDraw)
{
	CGuard guard(m_MutexMgr);
	
	assert(pRect);
	//save old rectangle
	VD_RECT old_rect = m_Rect;
	CRgn rgn;
	
	if(memcmp(pRect, &old_rect, sizeof(VD_RECT)) == 0)
	{
		return;
	}
	
	if(GetFlag(IF_POPUP))
	{
		m_Rect = *pRect;

		rgn.CreateRectRgn(&old_rect);
		rgn.SubtractRect(m_Rect);

		//更新其他窗口区域
		GlobalInvalidateRect(m_Rect);
		GlobalValidateRgn(&rgn);

		//更新自身窗口区域
		UpdateRgn();

		if(bReDraw)
		{
			Draw();
		}
	}
	else
	{
		if(GetFlag(IF_SHOWN))
		{	
			m_DC.Lock();
			m_DC.SubtractRect(pRect); // 减去新的区域，避免不必要重绘
			m_DC.UnLock();
			Show(FALSE, bReDraw);
			m_Rect = *pRect;
			Show(TRUE, bReDraw);
		}
		else
		{
			m_Rect = *pRect;
		}
	}
}

VD_BOOL CItem::VD_SendMessage(uint msg, uint wpa, uint lpa)
{
	if(m_pThread)
	{
		return m_pThread->VD_SendMessage(msg, wpa, lpa);
	}

	CItem *pItem = m_pParent;

	while(pItem)
	{
		if(pItem->m_pThread)
		{
			return pItem->m_pThread->VD_SendMessage(msg, wpa, lpa);
		}
		pItem = pItem->m_pParent;
	}

	return FALSE;
}

VD_BOOL CItem::VD_DispatchMessage(int screen, uint msg, uint wpa, uint lpa)
{
	//printf("VD_DispatchMessage :screen:%d, wpa: %d \n", screen, wpa);
	//printf("VD_DispatchMessage :msg:0x%x %d,%d \n", msg,VD_HIWORD(lpa),VD_LOWORD(lpa));
	
	if(screen < 0 || screen >= N_GUI_CORE)
	{
		//printf("VD_DispatchMessage FALSE 1\n");
		return FALSE;
	}

	CGuard guard(m_MutexMgr);

	if(m_pActive[screen] && (msg < XM_MOUSEMOVE || msg > XM_MBUTTONDBLCLK)) // 非鼠标消息
	{
		//printf("VD_DispatchMessage not mouse msg\n");
		m_pActive[screen]->VD_SendMessage(msg, wpa, lpa);
	}
	else // 鼠标消息
	{
		int px = VD_HIWORD(lpa);
		int py = VD_LOWORD(lpa);
		CItem * pItem;
		VD_RECT rect;

		pItem = m_pTop[screen];
		while(pItem)
		{
			// 如果该控件捕获输入,就直接将消息发送给它
			if (pItem->GetFlag(IF_CAPTURED))
			{
				//printf("item capture input\n");
				return pItem->VD_SendMessage(msg, wpa, lpa);
			}
			pItem->GetRect(&rect);
			if(PtInRect(&rect, px, py)) // 找到光标所在的控件
			{
				//printf("find cursor of item\n");
				if(pItem->m_dwStyle & pageActiveMost)
				{
					return pItem->VD_SendMessage(msg, wpa, lpa);
				}
				
				if (msg == XM_MOUSEMOVE) // 鼠标移动消息暂时只发给激活的控件
				{
					CPage *pPage = dynamic_cast<CPage*>(pItem);
					if(pItem == m_pActive[screen] || 
						(dynamic_cast<CMenu*>(m_pActive[screen]) &&  pPage  && (m_pActive[screen]->GetParent() == pPage) && dynamic_cast<CMenu*>(pPage->GetItemAt(px,py))) || 
						(dynamic_cast<CMenu*>(m_pActive[screen]) && dynamic_cast<CMenu*>(pItem) && (m_pActive[screen]->GetParent() == pItem->GetParent())))
					{
						return pItem->VD_SendMessage(msg, wpa, lpa);
					}
					else
					{
						return FALSE;
					}
				}
				else
				{
					// 处理鼠标按键消息
					//printf("mouse VD_DispatchMessage :msg:0x%x\n", msg);
					if((msg == XM_LBUTTONDOWN || msg == XM_MBUTTONDOWN || msg == XM_RBUTTONDOWN) && pItem != m_pActive[screen])
					{
						// 如果被激活的控件是非页面的弹出式控件，直接发给它处理。
						if(m_pActive[screen] && m_pActive[screen]->GetFlag(IF_POPUP) && dynamic_cast<CPage*>(m_pActive[screen]) == 0)
						{
							//printf("1\n");
							return m_pActive[screen]->VD_SendMessage(msg, wpa, lpa);
						}

						// 激活光标下新的控件， 激活成功后消息发给它.
						// 当该控件是一个页面且有子页面的时候, 可能会激活失败.
						if(pItem->SetActive() == pItem)
						{
							//printf("2\n");
							return m_pActive[screen]->VD_SendMessage(msg, wpa, lpa);
						}
					}

					if(!m_pActive[screen]) 
					{
						//printf("3\n");
						return FALSE;
					}
					//printf("4\n");
					return m_pActive[screen]->VD_SendMessage(msg, wpa, lpa);
				}
			}
			pItem = pItem->m_pDown;
		}
	}
	
	return FALSE;
}

void CItem::CloseAll(int iPageCloseFlag)   //modified by nike.xie 20090903//增加页面判断标志的缺省参数
{
	CGuard guard(m_MutexMgr);
	
	//close all pages
	for(int i = 0; i < N_GUI_CORE; i++)
	{
		CItem * pItem = m_pTop[i];
		if(!pItem)
		{
			continue;
		}
		CItem * temp = pItem->m_pDown;
		while(pItem)
		{
			temp = pItem->m_pDown;
			if(pItem->m_iType == IT_PAGE)
			{
				//add by nike.xie 20090903 //增加判断关闭得页面是否录像回放
				if ( (iPageCloseFlag == 1) && (((CPage*)pItem)->string == LOADSTR("titles.search")) )
				{
					//如果录像回放处于播放状态，则不关闭此页面
// 					if(((CPagePlay*)pItem)->GetPagePlayState() == PAGEPLAY_STATE_PLAYING)
// 					{
// 						break;
// 					}
				}
				//end
				
				if(((CPage*)pItem)->m_dwStyle & pageDialog)
				{
					pItem->m_pThread->m_bLoop = FALSE;
					pItem->VD_SendMessage(XM_KEYDOWN, KEY_ESC);//如果发送出错，说明队列以满，此时m_bLoop可以保证线程循环退出
				}
				else if(!(((CPage*)pItem)->m_dwStyle & pageNoExpire))
				{
					m_MutexMgr.Leave();
					((CPage*)pItem)->Close(UDM_CANCEL);
					m_MutexMgr.Enter();
				}
			}
			else//弹出式控件,如果没有以下代码，Honeywell菜单注销时会死锁，还没有其他方法解决
			{
				pItem->Close();
			}
			pItem = temp;
		}
	}
}

CDC * CItem::GetDC()
{
	return &m_DC;
}

ITEM_TYPE CItem::GetType()
{
	return m_iType;
}

int CItem::GetScreen(CPage* page)
{
	CPage* p = reinterpret_cast<CPage *>(page);
	if(p)
	{
		return p->m_screen;
	}
	else
	{
		//csp modify 20130323
		if(page)
		{
			printf("#########################GetScreen error:0x%08x\n",(int)(page));
			//exit(-1);
		}
	}
	return (int)(page);
}

void CItem::AdjustRectAll()
{
	CGuard guard(m_MutexMgr);
	
	for(int i = 0; i < N_GUI_CORE; i++)
	{
		CItem * pItem = m_pTop[i];
		while(pItem)
		{
			pItem->AdjustRect();
			pItem = pItem->m_pDown;
		}
	}
}

void CItem::SetKeyInputFlag(KEY_INPUT_FLAG index, int flag)
{
	m_bKeyInputFlags[index] = flag;
}

int CItem::GetKeyInputFlag(KEY_INPUT_FLAG index)
{
	return m_bKeyInputFlags[index];
}

void CItem::SetRelatedItem(CItem *up, CItem *down, CItem *left, CItem *right)
{
	if (m_pRelatedItems == NULL) 
	{
		m_pRelatedItems = new RELATEDITEM;
	}
	m_pRelatedItems->m_pUpRelated = up;
	m_pRelatedItems->m_pDownRelated = down;
	m_pRelatedItems->m_pLeftRelated = left;
	m_pRelatedItems->m_pRightRelated = right;

	return;
}

std::string CItem::GetName()
{
	return string;
}

CItemThread* CItem::GetThread()
{
	return  m_pThread;

}

uint CItem::GetStyle()
{
	return m_dwStyle;
}

