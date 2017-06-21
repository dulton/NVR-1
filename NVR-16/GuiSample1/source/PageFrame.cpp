#include "GUI/Pages/PageFrame.h"
#include "GUI/Pages/PageMainFrameWork.h"
#include "GUI/Pages/PagePlayBackFrameWork.h"

#if defined(longse) && defined(CHIP_HISI351X_2X)
#define NEXXT
#endif

static VD_BOOL bmpLoaded = FALSE;

// static VD_BITMAP * m_pDeSayBmp;
// static VD_BITMAP * bmp_title;
// static VD_BITMAP * pBmp_frame_top;
// static VD_BITMAP * pBmp_frame_repeat;
// static VD_BITMAP * pBmp_frame_bottom;

#if 0//csp modify
static VD_BITMAP * pBmp_main_top;
static VD_BITMAP * pBmp_main_repeat;
static VD_BITMAP * pBmp_main_bottom;
static VD_BITMAP * pBmp_frame_separa;
static VD_BITMAP * pBmp_main_left;
static VD_BITMAP * pBmp_frame_bottomico;
static VD_BITMAP * pBmp_main_topico;
static VD_BITMAP * pBmp_main_subpagetitle;
static VD_BITMAP * pBmp_main_right;
#endif

static VD_BITMAP* m_pCloseNormalBmp = NULL;
static VD_BITMAP* m_pCloseSelectBmp = NULL;

char* closeBmpNormalString = DATA_DIR"/temp/exit.bmp";
char* closeBmpSelectString = DATA_DIR"/temp/exit_f.bmp";


static VD_BITMAP* m_pMFrameBmp_T = NULL;  //cw_page
static VD_BITMAP* m_pMFrameBmp_L = NULL;
static VD_BITMAP* m_pMFrameBmp_R = NULL;
static VD_BITMAP* m_pMFrameBmp_B = NULL;

char* pMframebk_T = DATA_DIR"/temp/main_top.bmp";
char* pMframebk_L = DATA_DIR"/temp/main_left.bmp";
char* pMframebk_R = DATA_DIR"/temp/main_right.bmp";
char* pMframebk_B = DATA_DIR"/temp/main_bottom.bmp";

static VD_BITMAP* m_pLoginBmp_T = NULL;  //cw_page
static VD_BITMAP* m_pLoginBmp_L = NULL;
static VD_BITMAP* m_pLoginBmp_R = NULL;
static VD_BITMAP* m_pLoginBmp_B = NULL;

char* pLoginbk_T = DATA_DIR"/temp/login_top.bmp";
char* pLoginbk_L = DATA_DIR"/temp/login_left.bmp";
char* pLoginbk_R = DATA_DIR"/temp/login_right.bmp";
char* pLoginbk_B = DATA_DIR"/temp/login_bottom.bmp";

static VD_BITMAP* m_MessageboxBmp_T = NULL;  //cw_page
static VD_BITMAP* m_MessageboxBmp_L = NULL;
static VD_BITMAP* m_MessageboxBmp_R = NULL;
static VD_BITMAP* m_MessageboxBmp_B = NULL;

char* Messageboxbk_T = DATA_DIR"/temp/mbox_top.bmp";
char* Messageboxbk_L = DATA_DIR"/temp/mbox_left.bmp";
char* Messageboxbk_R = DATA_DIR"/temp/mbox_right.bmp";
char* Messageboxbk_B = DATA_DIR"/temp/mbox_bottom.bmp";

CPageFrame::CPageFrame(VD_PCRECT pRect,VD_PCSTR psz /* = NULL */,VD_BITMAP* icon /* = NULL */,CPage * pParent /* = NULL */, uint vstyle /* = 0 */)
:CPage(pRect, psz, pParent, vstyle | pageAlignCenter)
{
	//printf("CPageFrame:CPageFrame start\n");
	
	m_bPlayBackPage = FALSE;
	m_bPlayBackHide = FALSE;
	//m_bStartPage = FALSE;
	
	SetStyle(FALSE);
	
	if(icon)
	{
		SetPageIcon(pageIconSmall, icon);
	}
	
	//m_pButtonClose = NULL;
	
	//set default page size
	if(!pRect)
	{
		SetRect(CRect(0, 0, pageMainFrame_w, pageMainFrame_h/*370*/), FALSE);
	}
	
	m_TitleHeight = 42;
	
	//set default title size
	SetTitleSize(m_Rect.Width(), m_TitleHeight);
	//set default page margin
	
	m_pCloseNormalBmp = VD_LoadBitmap(closeBmpNormalString);
	m_pCloseSelectBmp = VD_LoadBitmap(closeBmpSelectString);
	
	m_pMFrameBmp_T = VD_LoadBitmap(pMframebk_T); //cw_page
	m_pMFrameBmp_L = VD_LoadBitmap(pMframebk_L);
	m_pMFrameBmp_R = VD_LoadBitmap(pMframebk_R);
	m_pMFrameBmp_B = VD_LoadBitmap(pMframebk_B);
	
	m_pLoginBmp_T = VD_LoadBitmap(pLoginbk_T); //cw_page
	m_pLoginBmp_L = VD_LoadBitmap(pLoginbk_L);
	m_pLoginBmp_R = VD_LoadBitmap(pLoginbk_R);
	m_pLoginBmp_B = VD_LoadBitmap(pLoginbk_B);
	
	m_MessageboxBmp_T = VD_LoadBitmap(Messageboxbk_T); //cw_page
	m_MessageboxBmp_L = VD_LoadBitmap(Messageboxbk_L);
	m_MessageboxBmp_R = VD_LoadBitmap(Messageboxbk_R);
	m_MessageboxBmp_B = VD_LoadBitmap(Messageboxbk_B);
	
	CRect rtClose;
//#ifdef NEXXT
//	rtClose.left = m_Rect.Width() - m_pCloseNormalBmp->width - 0;//cw_page 15
//	rtClose.top = 0;//cw_page 7
//#else
	rtClose.left = m_Rect.Width() - m_pCloseNormalBmp->width - 15;//cw_page 15
	rtClose.top = 11;//cw_page 7
//#endif
	
	//csp modify
	//pCloseButton = CreateButton(CRect(rtClose.left, rtClose.top,
	//				rtClose.left+m_pCloseNormalBmp->width, rtClose.top+m_pCloseNormalBmp->height-1),
	//				this, NULL, (CTRLPROC)&CPageFrame::OnClose, NULL, buttonNormalBmp);
	pCloseButton = CreateButton(CRect(rtClose.left, rtClose.top,
					rtClose.left+m_pCloseNormalBmp->width, rtClose.top+m_pCloseNormalBmp->height),
					this, NULL, (CTRLPROC)&CPageFrame::OnClose, NULL, buttonNormalBmp);
	pCloseButton->SetBitmap(m_pCloseNormalBmp, m_pCloseSelectBmp, m_pCloseSelectBmp);
#ifdef NEXXT
	pCloseButton->Show(FALSE);
#endif
	
#if 1//这一段话有用
	CCheckBox* pChkSetDef = CreateCheckBox(CRect(0,0,0,0), this);
	pChkSetDef->SetDefaultBitmap(CKB_NORMAL_TRUE, DATA_DIR"/temp/check.bmp");
	pChkSetDef->SetDefaultBitmap(CKB_NORMAL_FALSE, DATA_DIR"/temp/check_no.bmp");
	pChkSetDef->SetDefaultBitmap(CKB_SELECTED_TRUE, DATA_DIR"/temp/check_f.bmp");
	pChkSetDef->SetDefaultBitmap(CKB_SELECTED_FALSE, DATA_DIR"/temp/check_no_f.bmp");
	pChkSetDef->SetDefaultBitmap(CKB_DISABLED_TRUE,DATA_DIR"/temp/check_n.bmp");
	pChkSetDef->SetDefaultBitmap(CKB_DISABLED_FALSE,DATA_DIR"/temp/check_no_n.bmp");
	pChkSetDef->Show(FALSE);
#endif
	
#if 0//csp modify
	//printf("CPageFrame:bmpLoaded=%d\n",bmpLoaded);
	if(!bmpLoaded)
	{
		bmpLoaded = TRUE;
		
		//m_pDeSayBmp = VD_LoadBitmap(DATA_DIR"/logo.bmp");
		//pBmp_frame_top = VD_LoadBitmap(DATA_DIR"/bmp_frame_top.bmp");
		//pBmp_frame_repeat = VD_LoadBitmap(DATA_DIR"/bmp_frame_repeat.bmp");
		//pBmp_frame_bottom = VD_LoadBitmap(DATA_DIR"/bmp_frame_bottom.bmp");
		
		//m_pCloseBmp = VD_LoadBitmap(DATA_DIR"/button_close_normal.bmp");
		//printf("m_pCloseBmp\n");
		//if (m_pCloseBmp)
		//{
		//	printf("m_pCloseBmp 1111\n");
		//	ReplaceBitmapBits(m_pCloseBmp, VD_GetSysColor(COLOR_TRANSPARENT) , VD_RGB(8,8,8));
		//	ReplaceBitmapBits(m_pCloseBmp, VD_RGB(0,0,0) , VD_RGB(8,8,8));
		//}
		
		pBmp_main_top = VD_LoadBitmap(DATA_DIR"/bmp_main_top.bmp");
		if (pBmp_main_top)
		{
			ReplaceBitmapBits(pBmp_main_top, VD_RGB(0,0,0) , VD_RGB(8,8,8));
		}
		pBmp_main_repeat = VD_LoadBitmap(DATA_DIR"/bmp_main_repeat.bmp");
		pBmp_main_bottom = VD_LoadBitmap(DATA_DIR"/bmp_main_bottom.bmp");
		if (pBmp_main_bottom)
		{
			ReplaceBitmapBits(pBmp_main_bottom, VD_RGB(0,0,0), VD_RGB(8,8,8));
		}
		//bmp_title = VD_LoadBitmap(DATA_DIR"/bmp_title.bmp");
		
		pBmp_frame_separa = VD_LoadBitmap(DATA_DIR"/bmp_frame_separ.bmp");
		pBmp_main_left = VD_LoadBitmap(DATA_DIR"/bmp_main_left.bmp");
		
		pBmp_frame_bottomico = VD_LoadBitmap(DATA_DIR"/bmp_frame_buttom_ico.bmp");
		ReplaceBitmapBits(pBmp_frame_bottomico, VD_RGB(0,0,0) , VD_RGB(8,8,8));
		
		pBmp_main_topico = VD_LoadBitmap(DATA_DIR"/bmp_main_topico.bmp");
		ReplaceBitmapBits(pBmp_main_topico, VD_RGB(0,0,0) , VD_RGB(8,8,8));
		
		pBmp_main_subpagetitle = VD_LoadBitmap(DATA_DIR"/bmp_main_subpagetitile.bmp");
		
		pBmp_main_right = VD_LoadBitmap(DATA_DIR"/bmp_main_right.bmp");
	}
#endif
	
	SetMargin(6, m_TitleHeight, 4, m_TitleHeight);
}

CPageFrame::~CPageFrame()
{
	
}

void CPageFrame::Draw()
{
	m_DC.Lock();
	
	if(TRUE == m_bPlayBackPage)
	{
		int PbFrameTop = 502;
		pCloseButton->Show(FALSE);
		m_DC.SetBrush(VD_RGB(110,110,110));
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(0,PbFrameTop,m_Rect.Width(),m_Rect.Height()), 0, 0);
		
		m_DC.SetBrush(VD_RGB(33,33,41));
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(2,PbFrameTop+2,m_Rect.Width()-2,m_Rect.Height()-2), 2, 2);
	}
	else
	{
		VD_COLORREF frameBkColor = VD_GetSysColor(VD_COLOR_WINDOW);//cw_page
		if((m_pMFrameBmp_T == NULL) ||(m_pMFrameBmp_L == NULL)||(m_pMFrameBmp_R == NULL)||(m_pMFrameBmp_B == NULL)||
			(m_pLoginBmp_T == NULL) ||(m_pLoginBmp_L == NULL) ||(m_pLoginBmp_R == NULL) ||(m_pLoginBmp_B == NULL) ||
			(m_MessageboxBmp_T == NULL) ||(m_MessageboxBmp_L == NULL) ||(m_MessageboxBmp_R == NULL) ||(m_MessageboxBmp_B == NULL))
		{
			//printf("here-1\n");
			//fflush(stdout);
			
			// top
			m_DC.SetBrush(VD_RGB(80,80,80));
			m_DC.SetRgnStyle(RS_FLAT);
			m_DC.Rectangle(CRect(0, 0, m_Rect.Width(), 6), 0, 0);
			
			//m_DC.FillRect(CRect(0, 0, m_Rect.Width(), 6), VD_RGB(80,80,80));
			
			m_DC.SetBrush(VD_RGB(110,110,110));
			m_DC.SetRgnStyle(RS_FLAT);
			m_DC.Rectangle(CRect(0, 6, m_Rect.Width(), m_TitleHeight), 0, 0);
			
			//m_DC.FillRect(CRect(0, 6, m_Rect.Width(), m_TitleHeight), VD_RGB(110,110,110));
			
			//VD_COLORREF frameBkColor = VD_GetSysColor(VD_COLOR_WINDOW);
			m_DC.SetBrush(frameBkColor);
			//m_DC.SetBrush(VD_RGB(33,33,41));
			//m_DC.SetBrush(VD_RGB(33,41,49));
			m_DC.SetRgnStyle(RS_FLAT);
			m_DC.Rectangle(CRect(8,0,m_Rect.Width()-8,m_TitleHeight-2), 2, 2);
			
			//m_DC.FillRect(CRect(8, 0, m_Rect.Width()-8, 4), VD_RGB(60,60,60));
			int r, g, b;
			r = VD_GetRValue(frameBkColor);
			g = VD_GetGValue(frameBkColor);
			b = VD_GetBValue(frameBkColor);
			for(int i = 0; i<6; i++)
			{
				r += 8;
				g += 8;
				b += 8;
				
				m_DC.SetBrush(VD_RGB(r,g,b));
				m_DC.SetRgnStyle(RS_FLAT);
				m_DC.Rectangle(CRect(8, i, m_Rect.Width()-8, i+1), 0, 0);
				//m_DC.FillRect(CRect(8, i, m_Rect.Width()-8, i+1), VD_RGB(r,g,b));
			}
			
			m_DC.SetBrush(VD_RGB(120,120,120));
			m_DC.SetRgnStyle(RS_FLAT);
			m_DC.Rectangle(CRect(0, 6, 8, 7), 0, 0);
			m_DC.Rectangle(CRect(m_Rect.Width()-8, 6, m_Rect.Width()-7, 7), 0, 0);
			if(EM_GRAPH_OUTPUT_CVBS == GraphicsGetOutput())
			{
				m_DC.Rectangle(CRect(0, 5, m_Rect.Width(), 7), 0, 0);
			}
			else
			{
				m_DC.Rectangle(CRect(0, 6, m_Rect.Width(), 7), 0, 0);
			}
			
			//m_DC.FillRect(CRect(0, 6, 8, 7), VD_RGB(120,120,120));
			//m_DC.FillRect(CRect(m_Rect.Width()-8, 6, m_Rect.Width()-7, 7), VD_RGB(120,120,120));
			
			//m_DC.FillRect(CRect(0, 6, m_Rect.Width(), 7), VD_RGB(120,120,120));
			
			m_DC.SetBrush(VD_RGB(130,130,130));
			m_DC.SetRgnStyle(RS_FLAT);
			m_DC.Rectangle(CRect(8, 0, 9, m_TitleHeight-3), 0, 0);
			
			m_DC.SetBrush(VD_RGB(80,80,80));
			m_DC.SetRgnStyle(RS_FLAT);
			m_DC.Rectangle(CRect(m_Rect.Width()-9, 4, m_Rect.Width()-8, m_TitleHeight-3), 0, 0);
			
			//m_DC.FillRect(CRect(8, 0, 9, m_TitleHeight-3), VD_RGB(130,130,130));
			//m_DC.FillRect(CRect(m_Rect.Width()-9, 4, m_Rect.Width()-8, m_TitleHeight-3), VD_RGB(80,80,80));
			
			//mid and down

			m_DC.SetBrush(VD_RGB(110,110,110));
			m_DC.SetRgnStyle(RS_FLAT);
			m_DC.Rectangle(CRect(0,m_TitleHeight, m_Rect.Width(),m_Rect.Height()), 0, 0);

			m_DC.SetBrush(frameBkColor);
			//m_DC.SetBrush(VD_RGB(33,33,41));
			m_DC.SetRgnStyle(RS_FLAT);
			m_DC.Rectangle(CRect(3,m_TitleHeight+1,m_Rect.Width()-3,m_Rect.Height()-4), 2, 2);

			//描线
			m_DC.SetBrush(VD_RGB(120,120,120));
			m_DC.SetRgnStyle(RS_FLAT);
			m_DC.Rectangle(CRect(0, 0, 1, m_Rect.Height()), 0, 0);
			m_DC.Rectangle(CRect(2, m_TitleHeight+3, 3, m_Rect.Height()-3), 0, 0);
			m_DC.Rectangle(CRect(m_Rect.Width()-3, m_TitleHeight+3, m_Rect.Width()-2, m_Rect.Height()-3), 0, 0);
			m_DC.Rectangle(CRect(m_Rect.Width()-1, 5, m_Rect.Width(), m_Rect.Height()), 0, 0);
			//m_DC.FillRect(CRect(0, 0, 1, m_Rect.Height()), VD_RGB(120,120,120));
			//m_DC.FillRect(CRect(2, m_TitleHeight+3, 3, m_Rect.Height()-3), VD_RGB(120,120,120));

			//m_DC.FillRect(CRect(m_Rect.Width()-3, m_TitleHeight+3, m_Rect.Width()-2, m_Rect.Height()-3), VD_RGB(120,120,120));
			//m_DC.FillRect(CRect(m_Rect.Width()-1, 5, m_Rect.Width(), m_Rect.Height()), VD_RGB(120,120,120));

			//m_DC.FillRect(CRect(8, m_Rect.Height()-3, m_Rect.Width()-8, m_Rect.Height()), VD_RGB(128,128,128));
		}
		else
		{
			//printf("here-2,m_Rect.Width()=%d\n",m_Rect.Width());
			//fflush(stdout);
			
			switch(m_Rect.Width())
			{
				case pageMainFrame_w:
				{
					m_DC.Bitmap(CRect(0, 0, m_pMFrameBmp_L->width, m_Rect.Height()),m_pMFrameBmp_L);
					m_DC.Bitmap(CRect(m_Rect.Width()-m_pMFrameBmp_R->width,0, m_Rect.Width(), m_Rect.Height()),m_pMFrameBmp_R);
					m_DC.Bitmap(CRect(0, m_Rect.Height()-m_pMFrameBmp_B->height, m_Rect.Width(), m_Rect.Height()),m_pMFrameBmp_B);
					//csp modify
					//m_DC.Bitmap(CRect(0, 0, m_Rect.Width(),m_pMFrameBmp_T->width),m_pMFrameBmp_T);
					m_DC.Bitmap(CRect(0, 0, m_Rect.Width(),m_pMFrameBmp_T->height),m_pMFrameBmp_T);
					m_DC.SetBrush(frameBkColor);
					m_DC.SetRgnStyle(RS_RAISED);
					m_DC.Rectangle(CRect(m_pMFrameBmp_L->width-2,m_pMFrameBmp_T->height-2,m_Rect.Width()-m_pMFrameBmp_R->width+2,m_Rect.Height()-m_pMFrameBmp_B->height+2), 0, 0);	
					break;
				}
				case pageLogin_w:
				{
					m_DC.Bitmap(CRect(0, 0, m_pLoginBmp_L->width, m_Rect.Height()),m_pLoginBmp_L);
					m_DC.Bitmap(CRect(m_Rect.Width()-m_pLoginBmp_R->width,0, m_Rect.Width(), m_Rect.Height()),m_pLoginBmp_R);
					m_DC.Bitmap(CRect(0, m_Rect.Height()-m_pLoginBmp_B->height, m_Rect.Width(), m_Rect.Height()),m_pLoginBmp_B);
					//csp modify
					//m_DC.Bitmap(CRect(0, 0, m_Rect.Width(),m_pLoginBmp_T->width),m_pLoginBmp_T);
					m_DC.Bitmap(CRect(0, 0, m_Rect.Width(),m_pLoginBmp_T->height),m_pLoginBmp_T);
					m_DC.SetBrush(frameBkColor);
					m_DC.SetRgnStyle(RS_RAISED);
					m_DC.Rectangle(CRect(m_pLoginBmp_L->width-2,m_pLoginBmp_T->height-2,m_Rect.Width()-m_pLoginBmp_R->width+2,m_Rect.Height()-m_pLoginBmp_B->height+2), 0, 0);
					break;
				}
				//csp modify
				//case Messagebox_h:
				case Messagebox_w:
				{
					//printf("here-3,m_Rect.Width()=%d,m_MessageboxBmp_L=0x%08x\n",m_Rect.Width(),m_MessageboxBmp_L);
					//fflush(stdout);
					
					m_DC.Bitmap(CRect(0, 0, m_MessageboxBmp_L->width, m_Rect.Height()),m_MessageboxBmp_L);
					m_DC.Bitmap(CRect(m_Rect.Width()-m_MessageboxBmp_R->width,0, m_Rect.Width(), m_Rect.Height()),m_MessageboxBmp_R);
					m_DC.Bitmap(CRect(0, m_Rect.Height()-m_MessageboxBmp_B->height, m_Rect.Width(), m_Rect.Height()),m_MessageboxBmp_B);
					//csp modify
					//m_DC.Bitmap(CRect(0, 0, m_Rect.Width(),m_MessageboxBmp_T->width),m_MessageboxBmp_T);
					m_DC.Bitmap(CRect(0, 0, m_Rect.Width(),m_MessageboxBmp_T->height),m_MessageboxBmp_T);
					m_DC.SetBrush(frameBkColor);
					m_DC.SetRgnStyle(RS_RAISED);			
					m_DC.Rectangle(CRect(m_MessageboxBmp_L->width-2,m_MessageboxBmp_T->height-2,m_Rect.Width()-m_MessageboxBmp_R->width+2,m_Rect.Height()-m_MessageboxBmp_B->height+2), 0, 0);
					break;
				}
				default:
					break;
			}
		}
		
		m_DC.SetBkMode(BM_TRANSPARENT);
		
		if(m_pIconBmp)
		{
			m_DC.Bitmap(CRect(12,0,44,32),m_pIconBmp);//画图标
		}
		
		DrawTitle_left();
		
		if(TRUE == m_ifPageMain)
		{
			CPageMainFrameWork* pPage = (CPageMainFrameWork *)this;
			DrawTitle_center(pPage->GetDefualtTitleCenter());
		}
	}
	
	m_DC.UnLock();
	
	//printf("CPage::Draw()-1\n");
	//fflush(stdout);
	
	CPage::Draw();
	
	//printf("CPage::Draw()-2\n");
	//fflush(stdout);
}

void CPageFrame::DrawTitle_left()
{
	// 左标题区域
	CRect rtTitleLeft;
	
#if 0
	rtTitleLeft.top = 0;
	rtTitleLeft.bottom = pBmp_main_top->height;
	if(TRUE == m_ifPageMain)
	{
		rtTitleLeft.left = pBmp_main_topico->width +24+2;
		rtTitleLeft.right = rtTitleLeft.left + TEXT_WIDTH*6;
	}
	else
	{
		rtTitleLeft.left = 5;
		rtTitleLeft.right = rtTitleLeft.left + TEXT_WIDTH*6;
	}
	
	m_DC.Lock();
	
	//top
	VD_BITMAP * pPageBmp = pBmp_main_top;
	int h = pPageBmp->height;
	int w = pPageBmp->width;
	int i;
	for(i = rtTitleLeft.left; i< rtTitleLeft.right-w; i+= w)
	{
		m_DC.Bitmap(CRect(i, rtTitleLeft.top, i+w, h), pPageBmp, 0, 0);
	}
	
	m_DC.Bitmap(CRect(i, rtTitleLeft.top, rtTitleLeft.right, h), pPageBmp, 0, 0);
#endif
	
	m_DC.Lock();
	
	#ifdef NEXXT
	rtTitleLeft.left = m_Rect.Width()/2 - 90;
	rtTitleLeft.top = 14;
	
	rtTitleLeft.bottom = 14+24;
	rtTitleLeft.right = m_Rect.Width()/2 + 90;
	
	m_DC.SetTextAlign(VD_TA_XCENTER);
	#else
	rtTitleLeft.left = 20;
	rtTitleLeft.top = 14;
	
	rtTitleLeft.bottom = 14+24;
	rtTitleLeft.right = 200;
	
	m_DC.SetTextAlign(VD_TA_LEFT);
	#endif
	
	m_DC.SetFont(VD_GetSysColor(VD_COLOR_WINDOWTEXT));//标题前景
	m_DC.VD_TextOut(rtTitleLeft,GetTitle());//字
	
	m_DC.UnLock();
}

//xym GUI //显示提示信息
void CPageFrame::DrawTip(const char *pcTip)
{
#if 0//csp modify
	m_DC.Lock();
	
	m_DC.Bitmap(CRect(0, m_Rect.Height() - pBmp_main_bottom->height, m_Rect.Width() / 2, m_Rect.Height()), pBmp_main_bottom);
	m_DC.Bitmap(CRect(m_Rect.Width() / 2, m_Rect.Height() - pBmp_main_bottom->height, m_Rect.Width(), m_Rect.Height()), pBmp_main_bottom, 
		pBmp_main_bottom->width - m_Rect.Width() / 2, 0);
	
	if ( (NULL != pcTip) && (strlen(pcTip) > 0) )
	{
		m_DC.SetTextAlign(VD_TA_YCENTER);
		m_DC.SetFont(VD_RGB(150,150,150), FS_NORMAL);
		m_DC.VD_TextOut(CRect(24, m_Rect.Height() - pBmp_main_bottom->height, m_Rect.Width()-2, m_Rect.Height()), pcTip);		
	}
	
	m_DC.UnLock();
#endif
	
	return;
}

void CPageFrame::SetRect(VD_PCRECT pRect, VD_BOOL bReDraw)
{
	CPage::SetRect(pRect, bReDraw);
	
// 	if(m_pButtonClose)//for close button position can't be handled well by CPage
// 	{
// 		CRect rect(m_Rect.Width() - 24, 0, m_Rect.Width(), m_TitleHeight);
// 		ClientToScreen(rect);
// 		m_pButtonClose->SetRect(rect, bReDraw);
// 	}
}

CButton* CPageFrame::GetCloseButton()
{
//	return m_pButtonClose;
	return NULL;
}

void CPageFrame::DrawTitle_center(const char* string )
{
	return;
	
#if 0
	// 左标题区域
	CRect rtTitleCenter;

	rtTitleCenter.top = 0;
	rtTitleCenter.bottom = pBmp_main_top->height;
	if (TRUE == m_ifPageMain)
	{
		rtTitleCenter.left = pBmp_main_topico->width +24 + TEXT_WIDTH*6;
		rtTitleCenter.right = m_Rect.Width()-rtTitleCenter.left;
	} 
	else
	{
		rtTitleCenter.left = 24 + TEXT_WIDTH*6;
		rtTitleCenter.right = m_Rect.Width()-rtTitleCenter.left;
	}

	m_DC.Lock();

	//top			
	VD_BITMAP * pPageBmp = pBmp_main_top;
	int h = pPageBmp->height;
	int w = pPageBmp->width;
	int i;
	for (i = rtTitleCenter.left; i< rtTitleCenter.right-w; i+= w)
	{
		m_DC.Bitmap(CRect(i, rtTitleCenter.top, i+w, h), pPageBmp, 0, 0);
	}

	m_DC.Bitmap(CRect(i, rtTitleCenter.top, rtTitleCenter.right, h), pPageBmp, 0, 0);

	//字
	m_DC.SetTextAlign(VD_TA_CENTER);

	m_DC.SetFont(VD_GetSysColor(COLOR_TITLETEXT));//标题前景
	m_DC.VD_TextOut(rtTitleCenter,string);
	m_DC.UnLock();
#endif
}

void CPageFrame::OnClose()
{
	if((CPage*)(this->GetParentPage()))
	{
		if(((CPage*)(this->GetParentPage()))->GetFlag(IF_OPENED))
		{
			((CPage*)(this->GetParentPage()))->Show(TRUE ,TRUE);
		}
	}
	
	Close(UDM_CLOSED);
}

VD_BOOL CPageFrame::MsgProc( uint msg, uint wpa, uint lpa )
{
	//printf("CPageFrame::MsgProc\n");
	
	int px,py;
	int temp;
	switch(msg)
	{
	case XM_LBUTTONDOWN:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		//printf("CPageFrame::MsgProc x=%d, y=%d, %x\n", px, py, m_pCloseBmp);
		//if (m_pCloseBmp && PtInRect(CRect(m_Rect.right-m_pCloseBmp->width, m_Rect.top,m_Rect.right,m_Rect.top+m_TitleHeight), px, py))
		//{
		//	Close(UDM_CLOSED);
		//}
		
		/*if(m_bPlayBackHide)
		{
			//Show(TRUE);
			CRect rect(0,500,800,600);
			SetRect(&rect,TRUE);
			m_bPlayBackHide = FALSE;
			
			return 0;
		}*/
		
		break;
	case XM_RBUTTONDOWN:
	case XM_RBUTTONDBLCLK:
		if(m_bPlayBackPage)
		{
			//printf("CPageFrame::MsgProc:m_bPlayBackPage = %d\n",m_bPlayBackPage);
			return 0;
		}
		else
		{
			char tmp2[20] = {0};
			SBizDvrInfo sInfo;
			s32 ret = BizGetDvrInfo(&sInfo);
			if(ret == 0)
			{
				strcpy(tmp2, sInfo.sproductnumber);
			}
			
			//printf("CPageFrame::MsgProc:Not Playback Page\n");
			
			SetSystemLockStatus(1);
			if((0 == strcasecmp(tmp2, "NR3132")) || (0 == strcasecmp(tmp2, "NR3124")))
			{
				Close(UDM_CANCEL);
			}
			else
			{
				Close(UDM_CLOSED);
			}
			SetSystemLockStatus(0);
			
			if((0 == strcasecmp(tmp2, "NR3132")) || (0 == strcasecmp(tmp2, "NR3124")))
			{
				printf("CPageFrame::MsgProc quit\n");
				return TRUE;
			}
		}
		
		break;
	default:
		//return 0;
		break;
	}
	
	//return 	CPage::MsgProc(msg, wpa, lpa);
	return CPage::MsgProc(msg, wpa, lpa);
}

