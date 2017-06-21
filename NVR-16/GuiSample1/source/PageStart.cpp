
#include "GUI/Pages/PageStart.h"
// #include "System/AppConfig.h"
// #include "Functions/Display.h"
// #include "Functions/Alarm.h"

CPageStart::CPageStart(VD_PCRECT pRect, VD_PCSTR psz, VD_BITMAP* icon, CPage * pParent):CPage(pRect, psz, pParent,pageFullScreen)
{
	bmp_logo = (VD_BITMAP*)NULL;
	
	//if(AppConfig::instance()->getNumber("Global.ShowLogo", 1) == 1)
	{
		printf("load bmp begin\n");
		bmp_logo = VD_LoadBitmap(LOGO_DIR"/bmp_logo.bmp");
	}
	
	if(bmp_logo)
	{
		printf("logo : w=%d, h=%d, f=%d\n", bmp_logo->width, bmp_logo->height, bmp_logo->format);
	} 
	else
	{
		printf("load bmp faild\n");
	}
	
	//new items
	VD_RECT full_rect;
	CDevGraphics::instance(m_screen)->GetRect(&full_rect);
	
	printf("fulrect: l=%d,t=%d,r=%d,b=%d\n", full_rect.left, full_rect.top, full_rect.right, full_rect.bottom);
	
	CRect rect(0, 0, 0, 0);
	
	if(bmp_logo)
	{
		rect.left = full_rect.right/2 - bmp_logo->width/2;
		rect.right = rect.left+bmp_logo->width;
		if(bmp_logo->height>128)//clear view 和watchnet的为128
			rect.top = full_rect.bottom/2 - bmp_logo->height/2;
		else
			rect.top = full_rect.bottom/2 - bmp_logo->height;
		rect.bottom = rect.top+bmp_logo->height;//full_rect.bottom/2;
	}

	CRect m_rect;
	if(bmp_logo)
	{
		m_rect.SetRect(full_rect.right/2 - TEXT_WIDTH*6,rect.bottom+TEXT_HEIGHT, full_rect.right/2  + TEXT_WIDTH*6,  rect.bottom+TEXT_HEIGHT +TEXT_HEIGHT);
	}
	else
	{
		m_rect.SetRect(full_rect.right/2 - TEXT_WIDTH*6,full_rect.bottom/2, full_rect.right/2  + TEXT_WIDTH*6,  full_rect.bottom/2 + TEXT_HEIGHT);
	}

	//Begin delete by nike.xie 2008-02-17 		
	//pText = CreateStatic(m_rect, this,"VISIONDIGI",FALSE);
	//pText->SetTextAlign(TA_CENTER);
	//pText->SetBkColor(GetBitmapBkColor(bmp_logo, GetSysColor(COLOR_WINDOW)));
	//pText->SetTextColor(~GetBitmapBkColor(bmp_logo, GetSysColor(COLOR_WINDOW)));
	//End
	pText = NULL;
}

CPageStart::~CPageStart()
{
	
}

VD_BOOL CPageStart::Open(int state/*=CPageStart::PAGE_APPSTART*/)//默认为开机页面
{
	printf("startpage open \n");
	return CPage::Open();
}

VD_BOOL CPageStart::Close()
{
#ifndef WIN32
	sleep(2);               //开机画面停留5秒，可以使用户看见
#endif//WIN32
	return CPage::Close();
}

void CPageStart::SetText(VD_PCSTR psz)
{

	if(pText != NULL)
	{
		pText->SetText(psz);
	}
}

void CPageStart::Draw()
{
	printf("begin draw \n");
	if(!DrawCheck())
	{
		printf("drawchk fail\n");
		return;
	}

	m_DC.Lock();


       //Begin  add by nike.xie 2009-02-17       
       if(bmp_logo)
	{
		VD_RECT full_rect;
		CDevGraphics::instance(m_screen)->GetRect(&full_rect);
		
		//显示LOGO前,填充系统的背景颜色.这样可以不限制LOGO图片的尺寸
		m_DC.SetBrush(GetBitmapBkColor(bmp_logo, VD_GetSysColor(VD_COLOR_WINDOW)));
		//m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
		m_DC.Rectangle(CRect(0,0,full_rect.right-full_rect.left,full_rect.bottom-full_rect.top));

		VD_RECT rect;
		rect.left = (full_rect.right/2)-(bmp_logo->width/2);
		rect.top = (full_rect.bottom/2)-(bmp_logo->height/2);
		rect.right = (bmp_logo->width+rect.left);
		rect.bottom = (bmp_logo->height+rect.top);
		//显示bmp图片
		m_DC.Bitmap(&rect, bmp_logo, 0, 0);
		//End
	}

	m_DC.UnLock();

	CPage::Draw();
	
}

VD_BOOL CPageStart::UpdateData(UDM mode)
{
	if(mode == UDM_OPEN)
	{
		CDevGraphics::instance(m_screen)->SetAlpha(255, 0);//将图形叠加设为不透明
		CCursor::instance(0)->ShowCursor(FALSE);
	}
	else if(mode == UDM_CLOSED)
	{
		//CDevGraphics::instance(m_screen)->SetAlpha(CConfigGUISet::getLatest().iWindowAlpha, 0);//恢复成透明
		CDevGraphics::instance(m_screen)->SetAlpha(255, 0);//恢复成透明
		CCursor::instance(0)->ShowCursor(TRUE);
	}
	if(mode == UDM_OK || mode == UDM_CANCEL) 
	{
		return FALSE;
	}
	return TRUE;
}

