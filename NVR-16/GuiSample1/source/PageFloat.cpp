#include "GUI/Pages/PageFloat.h"

CPageFloat::CPageFloat( VD_PCRECT pRect,CPage * pParent /*= NULL*/,VD_PCSTR pTitle)
:CPage(pRect, pTitle, pParent, 0/*vstyle*/)
{
	m_bStartPage = FALSE;
	m_bChnSelectPage = FALSE;
	m_bAudioSelectPage = FALSE;
	m_bPtzCtlPage = FALSE;
	m_bPlayBackPage = FALSE;
	m_bPlayBackHide = FALSE;
	
	SetStyle(FALSE);
	
	//set default page size
	//printf("CPageFloat::CPageFloat %d %d %d %d \n", pRect->left, pRect->top, pRect->right, pRect->bottom);
	if(!pRect)
	{
		SetRect(CRect(0, 0, 100, 100), FALSE);
	}
}

CPageFloat::~CPageFloat()
{
	
}

void CPageFloat::Draw()
{
	m_DC.Lock();
	
	//printf("left=%d,top=%d,right=%d,bottom=%d\n",m_Rect.left,m_Rect.top,m_Rect.right,m_Rect.bottom);
	
	if (TRUE == m_bStartPage)
	{
		int StartFrameTop = m_Rect.Height()-40;
		//pCloseButton->Show(FALSE);
		m_DC.SetBrush(VD_RGB(110,110,110));
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(0,StartFrameTop,m_Rect.Width(),m_Rect.Height()), 0, 0);
		
		m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(4,StartFrameTop+4,m_Rect.Width()-4,m_Rect.Height()-4), 3, 3);
	}
	else if (TRUE == m_bPlayBackPage)
	{
		//pCloseButton->Show(FALSE);
		
		m_DC.SetBrush(VD_RGB(110,110,110));
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()), 0, 0);
		
		m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(2,2,m_Rect.Width()-2,m_Rect.Height()-2), 2, 2);
	}
	else 
	{
		m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
		
		m_DC.SetBrush(VD_RGB(128,128,128));
		m_DC.Rectangle(CRect(0, 0, m_Rect.Width(), 1));
		m_DC.Rectangle(CRect(0, m_Rect.Height()-1, m_Rect.Width(), m_Rect.Height()));
		m_DC.Rectangle(CRect(0, 0, 1, m_Rect.Height()));
		m_DC.Rectangle(CRect( m_Rect.Width()-1, 0, m_Rect.Width(), m_Rect.Height()));
		
		if(m_bChnSelectPage == TRUE)
		{
			m_DC.Rectangle(CRect(3, 2, m_Rect.Width()-3, 3));
			m_DC.Rectangle(CRect(3, m_Rect.Height()-28-5, m_Rect.Width()-3, m_Rect.Height()-28-4));
			m_DC.Rectangle(CRect(3, 2, 4, m_Rect.Height()-28-4));
			m_DC.Rectangle(CRect( m_Rect.Width()-4, 2, m_Rect.Width()-3, m_Rect.Height()-28-4));
		}
		
		if(m_bAudioSelectPage == TRUE)
		{
			m_DC.Rectangle(CRect(0, m_Rect.Height()*2/3-3, m_Rect.Width(), m_Rect.Height()*2/3-2));
		}
		
		if(m_bPtzCtlPage == TRUE)
		{
			//printf("m_bPtzCtlPage == TRUE\n");
			
			m_DC.Rectangle(CRect(0, 42, m_Rect.Width(), 43));
			m_DC.Rectangle(CRect(0, m_Rect.Height()-30, m_Rect.Width(), m_Rect.Height()-29));
			
#if 0
			if(m_bPtzCtlPageDrawLine)
			{
				m_DC.Rectangle(CRect(224, 0, 225, 40));
				m_DC.Rectangle(CRect(m_Rect.Width()-221, 0, m_Rect.Width()-220, 40));
			}
#endif
		}
	}
	
	m_DC.UnLock();
	
	CPage::Draw();
}

VD_BOOL CPageFloat::MsgProc( uint msg, uint wpa, uint lpa )
{
	//printf("CPageFrame::MsgProc\n");
	
	int px,py;
	int temp;
	//printf("yg CPageFloat::MsgProc msg = 0x%x, file:%s, func:%s\n", msg, __FILE__, __FUNCTION__);
	
	return CPage::MsgProc(msg, wpa, lpa);
	
	switch(msg)
	{
	case XM_LBUTTONDOWN:
		break;
		
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		
		if(m_bPlayBackHide)
		{
			//printf("l=%d,t=%d,r=%d,b=%d\n",m_pbRect.left,m_pbRect.top,m_pbRect.right,m_pbRect.bottom);
			SetRect(&m_pbRect,TRUE);
			m_bPlayBackHide = FALSE;
			
			return 0;
		}
		//printf("CPageFloat::MsgProc XM_LBUTTONDOWN\n");
		
		break;
	case XM_RBUTTONDOWN:
	case XM_RBUTTONDBLCLK:
		if(m_bPlayBackHide)
		{
			//Show(TRUE);
			//CRect rect(0,500,800,600);
			//SetRect(&m_Rect,TRUE);
			SetRect(&m_pbRect,TRUE);
			m_bPlayBackHide = FALSE;
			
			return 0;
		}
		else
		{
			if(m_bPlayBackPage)
			{
				//printf("m_bPlayBackPage = %d \n",m_bPlayBackPage);
				return 0;
			}
			//else
			//{
			//	Close(UDM_CLOSED);
			//}
		}
		break;
	default:
		//return 0;
		break;
	}
	//printf("yg CPageFloat::MsgProc over 1\n");
	//return 	CPage::MsgProc(msg, wpa, lpa);
	return CPage::MsgProc(msg, wpa, lpa);
}

