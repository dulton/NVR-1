#include "GUI/Pages/PageOSDPosSet.h"
#include "biz.h"

static	std::vector<CRect> chnPos;
static	std::vector<CRect> timePos;

static int GetMaxChnNum()
{
	return GetVideoMainNum();
}

CPageOSDPosSet::CPageOSDPosSet( VD_PCRECT pRect,VD_PCSTR psz /*=NULL*/,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/ )
:CPage(pRect, psz, pParent, pageFullScreen)
, m_nChannel(0)
, m_nOSDType(0)
, m_old_x(0)
, m_old_y(0)
, m_nDown(0)
{
	SetMargin(0,0,0,0);

	CRect rt;
	rt.SetRect(10, 10, 100, 100);
	pStaticTimeTitle = CreateStatic(rt, this, "Time");
	pStaticTimeTitle->SetTextAlign(VD_TA_CENTER);
	pStaticTimeTitle->Show(FALSE);

	rt.SetRect(10, 200, 100, 300);
	pStaticChannelTitle = CreateStatic(rt, this, "Channel", TRUE/*, staticTransparent*/);
	pStaticChannelTitle->SetTextAlign(VD_TA_CENTER);
	pStaticChannelTitle->Show(FALSE);

	m_maxChn = GetMaxChnNum();

	SBizParaTarget bizTar;
	SBizCfgStrOsd bizStrOsd;


	for(int i=0; i<m_maxChn; i++)
	{
		bizTar.emBizParaType = EM_BIZ_STROSD;
		bizTar.nChn = i;
		BizGetPara(&bizTar, &bizStrOsd);

		rt.left = bizStrOsd.sEncChnNamePos.x;
		rt.top = bizStrOsd.sEncChnNamePos.y;
		rt.right = rt.left + 5*TEXT_WIDTH/2;
		rt.bottom = rt.top + TEXT_HEIGHT;
		chnPos.push_back(rt);

		rt.left = bizStrOsd.sEncTimePos.x;
		rt.top = bizStrOsd.sEncTimePos.y;
		rt.right = rt.left + 20*TEXT_WIDTH/2;
		rt.bottom = rt.top + TEXT_HEIGHT;
		timePos.push_back(rt);
	}

	IsDwell = 0;

	m_pDevGraphics = CDevGraphics::instance(m_screen); 
	m_pDevGraphics->GetRect(&m_screenRect);
	
}

CPageOSDPosSet::~CPageOSDPosSet()
{
	
}

void CPageOSDPosSet::Draw()
{
	m_DC.Lock();
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.SetBrush(CDevGraphics::instance(m_screen)->GetKeyColor());
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
	m_DC.UnLock();

	CPage::Draw();
}


void CPageOSDPosSet::AdjustRectToCif(CRect* pRect)
{
	int w = pRect->Width();
	int h = pRect->Height();
	
	pRect->left = pRect->left*(352-w)/(m_screenRect.Width()-w);
	pRect->top = pRect->top*288/m_screenRect.Height();
	
	if(pRect->left+w > 352)
	{
		pRect->left = 352 - w;
	}
	
	if(pRect->top+h > 288)
	{
		pRect->top = 288 - h;
	}
	
	pRect->bottom = pRect->top + h;
	pRect->right = pRect->left + w;
}

void CPageOSDPosSet::AdjustRectToScreen(CRect* pRect)
{
	int w = pRect->Width();
	int h = pRect->Height();
	
	pRect->left = pRect->left*(m_screenRect.Width()-w)/(352-w);
	pRect->top = pRect->top*m_screenRect.Height()/288;
	
	if(pRect->left+w > m_screenRect.Width())
	{
		pRect->left = m_screenRect.Width() - w;
	}
	
	if(pRect->top+h > m_screenRect.Height())
	{
		pRect->top = m_screenRect.Height() - h;
	}
	
	pRect->bottom = pRect->top + h;
	pRect->right = pRect->left + w;
}

VD_BOOL CPageOSDPosSet::UpdateData( UDM mode )
{
	if (UDM_OPEN == mode)
	{
		if(GetDwellStartFlag())
		{
			IsDwell = 1;
			ControlPatrol(0);
		}
		
		if (m_nOSDType & OSD_TYPE_CHANNEL)
		{
			pStaticChannelTitle->Show(TRUE);

			//printf("pStaticChannelTitle->Show(TRUE)  \n");
		}
		else
		{
			pStaticChannelTitle->Show(FALSE);
		}

		if (m_nOSDType & OSD_TYPE_TIME)
		{
			pStaticTimeTitle->Show(TRUE);

			//printf("pStaticTimeTitle->Show(TRUE)  \n");
		}
		else
		{
			pStaticTimeTitle->Show(FALSE);
		}
	}
	else if (UDM_CLOSED == mode)
	{
		if(IsDwell)
		{
			usleep(500 * 1000);
			ControlPatrol(1);
			IsDwell = 0;
		}
		
		if (m_nOSDType & OSD_TYPE_CHANNEL)
		{
			//pStaticChannelTitle->Show(TRUE);
			if(m_nChannel == m_maxChn)//all
			{
				for(int i=0; i<m_maxChn; i++)
				{
					pStaticChannelTitle->GetRect(&chnPos[i]);
					AdjustRectToCif(&chnPos[i]);
					//printf("ALL chnPos:chn = %d, x = %d,y = %d\n",i,chnPos[i].left,chnPos[i].top);
				}
			}
			else
			{
				pStaticChannelTitle->GetRect(&chnPos[m_nChannel]);
				AdjustRectToCif(&chnPos[m_nChannel]);
				//printf("chn = %d, x = %d,y = %d\n",m_nChannel,chnPos[m_nChannel].left,chnPos[m_nChannel].top);
			}
		}
	

		if (m_nOSDType & OSD_TYPE_TIME)
		{
			//pStaticTimeTitle->Show(TRUE);
			if(m_nChannel == m_maxChn)//all
			{
				for(int i=0; i<m_maxChn; i++)
				{
					pStaticTimeTitle->GetRect(&timePos[i]);
					AdjustRectToCif(&timePos[i]);
					//printf("ALL timePos:chn = %d, x = %d,y = %d \n",i,timePos[i].left,timePos[i].top);
				}
			}
			else
			{
				pStaticTimeTitle->GetRect(&timePos[m_nChannel]);
				AdjustRectToCif(&timePos[m_nChannel]);
				//printf("chn = %d, x = %d,y = %d \n",m_nChannel,timePos[m_nChannel].left,timePos[m_nChannel].top);
			}
		}
		
		BizResumePreview(0); //yzw add
	}

	return TRUE;
}


int CPageOSDPosSet::GetChannel()
{
	return m_nChannel;
}

int CPageOSDPosSet::GetOsdType()
{

	return m_nOSDType;
}

std::vector<CRect> CPageOSDPosSet::GetChnOsdRect()
{

	return chnPos;
}


std::vector<CRect> CPageOSDPosSet::GetTimeOsdRect()
{
	return timePos;
}

static int CheckConfict(CRect *pRect1, CRect *pRect2)
{
	if(((pRect1->top >= pRect2->top) && (pRect1->bottom - pRect2->bottom <= 3*TEXT_HEIGHT))
			|| ((pRect2->top >= pRect1->top) && (pRect2->bottom - pRect1->bottom <= 3*TEXT_HEIGHT)))
	{
		return 1;
	}

	return 0;
}

VD_BOOL CPageOSDPosSet::MsgProc( uint msg, uint wpa, uint lpa )
{
	if(CPage::MsgProc(msg, wpa, lpa))
	{
		return TRUE;
	}

//	printf("CPageOSDPosSet::MsgProc\n");

	int px, py;
	switch(msg)
	{
	case XM_KEYDOWN:
		if(wpa!=KEY_ESC)
		{
			return TRUE;
		}
	case XM_LBUTTONDOWN:
		m_old_x = VD_HIWORD(lpa);
		m_old_y = VD_LOWORD(lpa);

		if ((m_nOSDType & OSD_TYPE_TIME) && PtInRect(m_rtOSD[1], m_old_x, m_old_y))
		{
			m_nDown = 2;
		}
		else if ((m_nOSDType & OSD_TYPE_CHANNEL) && PtInRect(m_rtOSD[0], m_old_x, m_old_y))
		{
			m_nDown = 1;
		} 

		//printf("m_nDown %d\n", m_nDown);
		break;

	case XM_LBUTTONUP:
		m_nDown = 0;
		break;

	case XM_MOUSEMOVE:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		
		if (m_nDown)
		{
			//zlbfix 20111201
			if (m_nDown > 2 || m_nDown < 0)
			{
				return TRUE;
			}
			
			//printf("mouse move %d\n", m_nDown);
			if (px != m_old_x || py != m_old_y)
			{
				//zlbfix 20111201
				CRect TmpRect;
				int nIndex = m_nDown - 1;
				
				memcpy(&TmpRect, &m_rtOSD[nIndex], sizeof(CRect));

				TmpRect.OffsetRect(px - m_old_x, py -m_old_y);
			
				int width = TmpRect.Width();
				int height = TmpRect.Height();

				if (TmpRect.left <0)
				{
					TmpRect.left = 0;
				}

				if (TmpRect.left > m_Rect.Width() - width)
				{
					TmpRect.left = m_Rect.Width() - width;
				}
					
				TmpRect.right = TmpRect.left + width;

				if (TmpRect.top <0)
				{
					TmpRect.top = 0;
				}

				if (TmpRect.top > m_Rect.Height() - height)
				{
					TmpRect.top = m_Rect.Height() - height;
				}

				TmpRect.bottom = TmpRect.top + height;

				if (CheckConfict(&TmpRect, &m_rtOSD[1 - nIndex]))
				{
					return TRUE;
				}

				memcpy(&m_rtOSD[nIndex], &TmpRect, sizeof(CRect));

				m_old_x = px;
				m_old_y = py;
			}
			DrawOSD();
		}
		break;
	}

	return TRUE;
}

void CPageOSDPosSet::SetOSDType( int iChannel, int osdtype, VD_BOOL bRedraw )
{
	printf("iChannel = %d\n",iChannel);
	printf("osdtype = %d\n",osdtype);
	
	m_nChannel = iChannel;
	m_nOSDType = osdtype;

	m_old_x = 0;
	m_old_y = 0;
	m_nDown = 0;
	//printf("CPageOSDPosSet::SetOSDType %d %x \n", iChannel, osdtype);
	
	SBizParaTarget bizTar;
	SBizCfgStrOsd bizStrOsd;
		
	if(iChannel == m_maxChn)//allzzzzz
	{
		bizTar.nChn = 0;
		bizTar.emBizParaType = EM_BIZ_STROSD;
		int ret = BizGetPara(&bizTar, &bizStrOsd);
		if(ret != 0)
		{
			printf("Get osd cfg error\n");
		}

		s8 chnname[32] = "CH All";
		
		strcpy(chnname,"CH All");
		
		pStaticChannelTitle->SetText(chnname);

		BizPreviewZoomChn(bizTar.nChn);
	}
	else
	{
		bizTar.nChn = iChannel;

		bizTar.emBizParaType = EM_BIZ_STROSD;
		int ret = BizGetPara(&bizTar, &bizStrOsd);
		if(ret != 0)
		{
			printf("Get osd cfg error\n");
		}

		pStaticChannelTitle->SetText(bizStrOsd.strChnName);
		BizPreviewZoomChn(bizTar.nChn);
	}

	CRect rt;
	char nameChn[30] = {0};
	strcpy(nameChn, pStaticChannelTitle->GetText());
	
	if(osdtype & OSD_TYPE_CHANNEL)
	{
		rt.left = bizStrOsd.sEncChnNamePos.x;
		//rt.left = 20;
		//rt.right = strlen(pStaticChannelTitle->GetText())*TEXT_WIDTH/2;
		
		rt.right = rt.left + strlen(nameChn)*10 + 50;//TEXT_WIDTH/2;//80;
		
		rt.top = bizStrOsd.sEncChnNamePos.y;
		
		//printf("chnname x = %d, y = %d\n",rt.left,rt.top);
		//rt.top = 500;
		rt.bottom = rt.top + TEXT_HEIGHT;
		
		//printf("before chnname x = %d, y = %d\n",rt.left,rt.top);
		
		AdjustRectToScreen(&rt);
		
		//printf("after chnname x = %d, y = %d\n",rt.left,rt.top);
		
		pStaticChannelTitle->SetRect(rt);
		
		m_rtOSDOld[0] = rt;
		m_rtOSD[0] = rt;
	}
	
	if(osdtype & OSD_TYPE_TIME)
	{
		SYSTEM_TIME time_para;
		char time_arr[20];
		
		GetSysDate(&time_para);
		GetSysTime(&time_para);
		
		sprintf(time_arr, "%04d-%02d-%02d %02d:%02d:%02d", time_para.year, time_para.month, time_para.day, time_para.hour, time_para.minute, time_para.second);
		//pStaticTimeTitle->SetText("2011-5-19 17:00:00");
		
		pStaticTimeTitle->SetText(time_arr);
		rt.left = bizStrOsd.sEncTimePos.x;
		//rt.left = 20;
		rt.right = rt.left + 200;
		rt.top = bizStrOsd.sEncTimePos.y;
		//rt.top = 20;
		rt.bottom = rt.top + TEXT_HEIGHT;
		
		
		//printf("before time x = %d, y = %d\n",rt.left,rt.top);
		//printf("sEncTimePos.x = %d,sEncTimePos.y = %d \n",rt.left ,rt.top);
		AdjustRectToScreen(&rt);
		
		//printf("after time x = %d, y = %d\n",rt.left,rt.top);
		
		pStaticTimeTitle->SetRect(rt);
		
		m_rtOSDOld[1] = rt;
		m_rtOSD[1] = rt;
	}
	
	if (bRedraw)
	{
		Draw();
	}
}

void CPageOSDPosSet::DrawOSD()
{
// 	printf("m_rtOSD[0] : %d %d %d %d\n", m_rtOSD[0].left, m_rtOSD[0].top, m_rtOSD[0].right, m_rtOSD[0].bottom);
// 	printf("m_rtOSD[1] : %d %d %d %d\n", m_rtOSD[1].left, m_rtOSD[1].top, m_rtOSD[1].right, m_rtOSD[1].bottom);
	
	if ((m_nOSDType & OSD_TYPE_CHANNEL) && (m_rtOSDOld[0] != m_rtOSD[0]))
	{
		//printf("set chn\n");
		pStaticChannelTitle->SetRect(m_rtOSD[0]);
		m_rtOSDOld[0] = m_rtOSD[0];
	}
	
	if ((m_nOSDType & OSD_TYPE_TIME) && (m_rtOSDOld[1] != m_rtOSD[1]))
	{
		//printf("set time\n");
		pStaticTimeTitle->SetRect(m_rtOSD[1]);
		m_rtOSDOld[1] = m_rtOSD[1];
	}
	
	pStaticChannelTitle->SetText(pStaticChannelTitle->GetText());
	pStaticTimeTitle->SetText(pStaticTimeTitle->GetText());
}

