#include "GUI/Pages/PageMaskSet.h"
#include "GUI/Pages/BizData.h"

static int GetMaxChnNum()
{
	return GetVideoMainNum();
}

CPageMaskSet::CPageMaskSet( VD_PCRECT pRect,VD_PCSTR psz /*=NULL*/,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPage(pRect, psz, pParent, pageFullScreen)
, m_nChannel(0)
, m_bDown(FALSE)
, m_start_x(0)
, m_start_y(0)
, m_nCurMask(0)
{
	SetMargin(0,0,0,0);
	
	char tmp2[20] = {0};
	GetProductNumber(tmp2);
	
	if((0 == strcasecmp(tmp2, "R9624T"))
		|| (0 == strcasecmp(tmp2, "R9632S"))
		|| (0 == strcasecmp(tmp2, "R9624SL"))
		|| (0 == strcasecmp(tmp2, "R9516S"))
		|| (0 == strcasecmp(tmp2, "R9616S"))
		|| (0 == strcasecmp(tmp2, "R9608S"))
		|| (0 == strcasecmp(tmp2, "R3104HD"))//csp modify
		|| (0 == strcasecmp(tmp2, "R3110HDW"))//csp modify
		|| (0 == strcasecmp(tmp2, "R3106HDW"))//csp modify
		|| (0 == strcasecmp(tmp2, "R3116"))//csp modify
		|| (0 == strcasecmp(tmp2, "R3116W"))//csp modify
		|| (0 == strcasecmp(tmp2, "R2104"))//csp modify
		|| (0 == strcasecmp(tmp2, "R2104W"))//csp modify
		|| (0 == strcasecmp(tmp2, "R2108"))//csp modify
		|| (0 == strcasecmp(tmp2, "R2108W"))//csp modify
		|| (0 == strcasecmp(tmp2, "R2116"))//csp modify
		|| (0 == strcasecmp(tmp2, "R2016")))//csp modify
	{
		m_MaskMax = 1;
	}
	else if((0 == strcasecmp(tmp2, "NR2116")))
	{
		m_MaskMax = 1;
	}
	else if((0 == strcasecmp(tmp2, "NR3116")))
	{
		m_MaskMax = 1;
	}
	else if((0 == strcasecmp(tmp2, "NR3132")))
	{
		m_MaskMax = 1;
	}
	else if((0 == strcasecmp(tmp2, "NR3124")))
	{
		m_MaskMax = 1;
	}
	else if((0 == strcasecmp(tmp2, "NR1004")))
	{
		m_MaskMax = 1;
	}
	else if((0 == strcasecmp(tmp2, "NR1008")))
	{
		m_MaskMax = 1;
	}
	else
	{
		m_MaskMax = 4;
	}
	
	for(int i=0; i<m_MaskMax; i++)
	{
		m_pMasklist[i] = CreateStatic(CRect(0,0,0,0), this, "");
		m_pMasklist[i]->SetBkColor(VD_RGB(120,120,120));
	}
	
	IsDwell = 0;
	int maxChn = GetMaxChnNum();
	osdRects = (OsdRect*)calloc(maxChn, sizeof(OsdRect));//cw_mask
	//osdRects = (OsdRect*)malloc(maxChn*sizeof(OsdRect));
}

CPageMaskSet::~CPageMaskSet()
{
	
}

VD_BOOL CPageMaskSet::UpdateData( UDM mode )
{
	//printf("UpdateData 11\n");
	if(UDM_OPEN == mode)
	{
		//read cur mask
		
		//m_pMasklist[0]->SetRect(CRect(20,20,100,100));
		//m_nCurMask =1;
		BizSetZoomMode(1);  //cw_zoom
		
		if(GetDwellStartFlag())
		{
			IsDwell = 1;
			ControlPatrol(0);
		}
		
		int realNum;
		SRect rt[10];
		GetRectOsd(m_nChannel,10,&realNum,rt);

		for(int i=0; i<realNum; i++)
		{
			if(i>=m_MaskMax)
			{
				break;
			}
			
			int nScreanWidth, nScreanHeight;
			GetVgaResolution(&nScreanWidth, &nScreanHeight);
			int Realheight=(GetVideoFormat()?480:576);//cw_mask
			int l = rt[i].x * nScreanWidth / 704;
			int t = rt[i].y * nScreanHeight / Realheight;
			int r = (rt[i].w+rt[i].x) * nScreanWidth / 704;
			int b = (rt[i].h+rt[i].y) * nScreanHeight / Realheight;
			m_pMasklist[i]->SetRect(CRect(l,t,r,b));
		}

		m_nCurMask = realNum;

	}
	else if(UDM_CLOSED == mode)
	{
        BizSetZoomMode(0);//cw_zoom
        
		osdRects[m_nChannel].bModify = TRUE;
		osdRects[m_nChannel].realNum = m_nCurMask;
		
		for(int i=0;i<m_nCurMask; i++)
		{
			m_pMasklist[i]->GetRect(&osdRects[m_nChannel].osdRect[i]);
		}
		
		if(IsDwell)
		{
			usleep(500 * 1000);
			ControlPatrol(1);
			IsDwell = 0;
		}
		
		BizResumePreview(0);
	}
	//printf("UpdateData 22\n");
	return TRUE;
}

OsdRect* CPageMaskSet::GetMaskRect(int iChannel)
{
	return &osdRects[iChannel];
}

void CPageMaskSet::AdjustMaskList(int maskId)
{
	for(int i=maskId+1; i<m_nCurMask; i++)
	{
		CRect rt;
		m_pMasklist[i]->GetRect(&rt);
		m_pMasklist[i-1]->SetRect(rt);

	}

	m_pMasklist[m_nCurMask-1]->SetRect(CRect(0,0,0,0));
	m_nCurMask--;
}

VD_BOOL CPageMaskSet::MsgProc( uint msg, uint wpa, uint lpa )
{
	if(CPage::MsgProc(msg, wpa, lpa))
	{
		return TRUE;
	}
	
	int px, py;
	switch(msg)
	{
	case XM_LBUTTONDOWN:
		if(m_nCurMask >= m_MaskMax)
		{
			return TRUE;
		}
		
		m_start_x = VD_HIWORD(lpa);
		m_start_y = VD_LOWORD(lpa);
		m_bDown = TRUE;
		//printf("XM_LBUTTONDOWN = %d \n",m_nCurMask);
		break;
	case XM_LBUTTONDBLCLK:
	{
		m_start_x = VD_HIWORD(lpa);
		m_start_y = VD_LOWORD(lpa);
		
		//printf("XM_LL\n");
		
		//int realNum = m_nCurMask[m_nChannel];
		for(int i=m_MaskMax-1; i>=0; i--)
		{
			CRect tmpRt;
			m_pMasklist[i]->GetRect(&tmpRt);
				
			if(PtInRect(&tmpRt,m_start_x,m_start_y))
			{
				printf("i = %d\n",i);
				m_pMasklist[i]->SetRect(CRect(0,0,0,0));
				//m_nCurMask--;

				AdjustMaskList(i);

				printf("m_nCurMask = %d\n",m_nCurMask);

				break;
			}
		}

		//printf("XM_LLL\n");

		//printf("m_nCurMask[m_nChannel] %d\n",m_nCurMask[m_nChannel]);
		//printf("m_bDown %d\n",m_bDown);
	}break;

	case XM_LBUTTONUP:
		if (m_nCurMask >= m_MaskMax)
		{
			return TRUE;
		}
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if (px != m_start_x && py != m_start_y)
		{
			AdjustMaskNum();
		}
		m_bDown = FALSE;

		//printf("up\n");
		break;

	case XM_MOUSEMOVE:
		if (m_nCurMask >= m_MaskMax)
		{
			return TRUE;
		}
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if (m_bDown)
		{
			//printf("mouse move %d\n", m_nDown);
			//int offsetX = m_start_x-px;
			//int offsetY = m_start_y-py;
			if (px != m_start_x && py != m_start_y)
			//if(ABS(offsetX)>=3 && ABS(offsetY)>=3)
			{
				CRect rt(MIN(px,m_start_x), MIN(py,m_start_y), MAX(px,m_start_x), MAX(py,m_start_y));

				DrawMask(m_nCurMask, rt);	
			}
		}
		break;
	}

	return TRUE;
}

void CPageMaskSet::Draw()
{
	//printf("Draw 33\n");
	m_DC.Lock();
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.SetBrush(CDevGraphics::instance(m_screen)->GetKeyColor());
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
	m_DC.UnLock();

	CPage::Draw();
}

void CPageMaskSet::DrawMask(int index, VD_PCRECT pRect)
{
	//printf("DrawMask 000 %d\n", m_MaskMax);
	
	if(index <0 || index >m_MaskMax || !pRect)
	{
		return;
	}
	
	//printf("DrawMask index %d rect:(%d,%d,%d,%d)\n", index, pRect->left, pRect->right, pRect->top, pRect->bottom);
	
	m_pMasklist[index]->SetRect(pRect);
}

void CPageMaskSet::SetMaskRect( int iChannel )
{
	osdRects[iChannel].bModify = TRUE;
	osdRects[iChannel].realNum = 0;
	for(int j=0; j<m_MaskMax; j++)
	{
		osdRects[iChannel].osdRect[j].left = 0;
		osdRects[iChannel].osdRect[j].top = 0;
		osdRects[iChannel].osdRect[j].right = 0;
		osdRects[iChannel].osdRect[j].bottom = 0;
		
		#if 0
		rectOsd[j].x = osdrt->osdRect[j].left * 704 / nScreanWidth;
		rectOsd[j].y = osdrt->osdRect[j].top * 576 / nScreanHeight;
		s32 r = osdrt->osdRect[j].right * 704 / nScreanWidth;
		s32 b = osdrt->osdRect[j].bottom * 576 / nScreanHeight;
		rectOsd[j].w = r - rectOsd[j].x;
		rectOsd[j].h = b - rectOsd[j].y;
		#endif
	}
}

void CPageMaskSet::SetMaskNum(int num)
{
	m_nCurMask = num;
}

void CPageMaskSet::SetMaskChannel( int iChannel )
{
	m_nChannel = iChannel;
	m_bDown = FALSE;
	m_start_x = 0;
	m_start_y = 0;

	for (int i=0; i<m_MaskMax; i++)
	{
		m_pMasklist[i]->SetRect(CRect(0,0,0,0));
	}

	m_nCurMask = 0;
}

void CPageMaskSet::AdjustMaskNum()
{
	int i = 0;
	CRect rtCur;
	int cur = m_nCurMask;
	m_pMasklist[cur]->GetRect(&rtCur);
	
	//int toDelMsk = 0;

	if(rtCur.Width()<=3 && rtCur.Height()<=3)
	{
		m_pMasklist[cur]->SetRect(CRect(0,0,0,0));
		
		return;
	}

	for (i =0; i<cur; i++ )
	{
		CRect rt;
		m_pMasklist[i]->GetRect(&rt);

		if (PtInRect(rt, rtCur.left, rtCur.top)
			&& PtInRect(rt, rtCur.right, rtCur.bottom))
		{
			//toDelMsk += 1<<cur;		

			m_nCurMask--;

			break;
		}
		
// 		if (PtInRect(rtCur, rt.left, rt.top)
// 			&& PtInRect(rtCur, rt.right, rt.bottom))
// 		{
// 			toDelMsk += 1<<i;
// 		}

	}

	m_nCurMask++;
}

