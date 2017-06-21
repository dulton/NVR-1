
#include "GUI/Pages/PageBlank.h"

CPageBlank::CPageBlank(VD_PCRECT pRect,VD_PCSTR psz /* = NULL */, CPage * pParent /* = NULL */, uint vstyle /* = 0 */)
:CPage(pRect, psz, pParent, vstyle | pageFullScreen)
{
	m_pButtonClose = NULL;
}

CPageBlank::~CPageBlank()
{
}

void CPageBlank::Draw()
{
	m_DC.Lock();
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.SetBrush(CDevGraphics::instance(m_screen)->GetKeyColor());
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
	m_DC.UnLock();

	CPage::Draw();
}


CButton* CPageBlank::GetCloseButton()
{
	return m_pButtonClose;
}
