#include "GUI/Ctrls/Page.h"

#define  ITEM_HEAD_WIDTH	32 //(TEXT_WIDTH*2)

CRecFileTimeBox::CRecFileTimeBox( VD_PCRECT pRect, CPage * pParent, CTRLPROC onClkTrack /*= NULL*/, uint style /*= 0*/ )
:CItem(pRect, pParent, IT_RECTIMEBOX,style|styleEditable|styleAutoFocus)//, FALSE)
{
	if (!pRect)
	{
		return;
	}
	
	if (m_Rect.Width() < ITEM_HEAD_WIDTH)
	{
		return;
	}
	
	m_dbSecPerPix = 24*3600*1.0/(m_Rect.Width() - ITEM_HEAD_WIDTH);
	if (!m_dbSecPerPix)
	{
		printf("how hell width you set!!!\n");
		return;
	}
	m_hourSplitWidth = 3600*4/m_dbSecPerPix;
	m_hourwidth = 3600/m_dbSecPerPix;
	
	m_nTrackTimePos = 0;
	
	m_vProc = onClkTrack;
}

CRecFileTimeBox::~CRecFileTimeBox()
{
	
}

void CRecFileTimeBox::Draw()
{
	if(!DrawCheck()){
		return;
	}

	DrawFrame();
	DrawItem();
	DrawTrack(m_nTrackTimePos);
}


void CRecFileTimeBox::DrawFrame()
{
	m_DC.Lock();

	m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));

#if 0
	m_DC.FillRect(CRect(0, 0, m_Rect.Width(), 2) ,VD_GetSysColor(COLOR_FRAME));
	m_DC.FillRect(CRect(0, m_Rect.Height()-2, m_Rect.Width(), m_Rect.Height()) ,VD_GetSysColor(COLOR_FRAME));
	m_DC.FillRect(CRect(0, 0, 2, m_Rect.Height()) ,VD_GetSysColor(COLOR_FRAME));
	m_DC.FillRect(CRect( m_Rect.Width()-2, 0, m_Rect.Width(), m_Rect.Height()) ,VD_GetSysColor(COLOR_FRAME));
#else
	m_DC.SetBrush(VD_GetSysColor(COLOR_FRAME));
	m_DC.Rectangle(CRect(0, 0, m_Rect.Width(), 2));
	m_DC.Rectangle(CRect(0, m_Rect.Height()-2, m_Rect.Width(), m_Rect.Height()));
	m_DC.Rectangle(CRect(0, 0, 2, m_Rect.Height()));
	m_DC.Rectangle(CRect( m_Rect.Width()-2, 0, m_Rect.Width(), m_Rect.Height()));
#endif


#if 0
	m_DC.FillRect(CRect(0, CTRL_HEIGHT-1, m_Rect.Width(), CTRL_HEIGHT) ,VD_RGB(255,255,255)/*VD_GetSysColor(COLOR_FRAME)*/);
#else
	m_DC.SetBrush(VD_RGB(128,128,128));
	m_DC.Rectangle(CRect(0, CTRL_HEIGHT-1, m_Rect.Width(), CTRL_HEIGHT));
#endif

	int i = 0;
	m_DC.SetFont(VD_GetSysColor(VD_COLOR_WINDOWTEXT));
	m_DC.SetTextAlign(VD_TA_LEFT|VD_TA_BOTTOM);

	for (i= 0 ;i< 6; i++)
	{
#if 0
		m_DC.FillRect(CRect(ITEM_HEAD_WIDTH + i*m_hourSplitWidth, CTRL_HEIGHT/2, ITEM_HEAD_WIDTH + i*m_hourSplitWidth+1, CTRL_HEIGHT) ,VD_RGB(255,255,255)/*VD_GetSysColor(COLOR_FRAME)*/);
#else
		m_DC.Rectangle(CRect(ITEM_HEAD_WIDTH + i*m_hourSplitWidth, CTRL_HEIGHT/2, ITEM_HEAD_WIDTH + i*m_hourSplitWidth+1, CTRL_HEIGHT));
#endif
		char hourbuf[10] = {0};
		sprintf(hourbuf, "%02d:00", i*4);
		m_DC.VD_TextOut(CRect(ITEM_HEAD_WIDTH + i*m_hourSplitWidth + 2, 2, ITEM_HEAD_WIDTH + (i+1)*m_hourSplitWidth-1, CTRL_HEIGHT-1), hourbuf);
	}

	m_DC.UnLock();
}

void CRecFileTimeBox::DrawItem()
{
	m_DC.Lock();

	m_DC.SetFont(VD_GetSysColor(VD_COLOR_WINDOWTEXT));
	m_DC.SetTextAlign(VD_TA_CENTER);

	int i = 0;
	int count = m_items.size();

	m_DC.SetBrush(VD_RGB(67,77,87));
	m_DC.SetRgnStyle(RS_FLAT);
	m_DC.Rectangle(CRect(ITEM_HEAD_WIDTH, CTRL_HEIGHT,m_Rect.Width()-2,CTRL_HEIGHT + count*TEXT_HEIGHT));

	for (i=0; i<count; i++)
	{	
#if 0
		m_DC.FillRect(CRect(0, CTRL_HEIGHT + (i+1)*TEXT_HEIGHT, m_Rect.Width(),  CTRL_HEIGHT + (i+1)*TEXT_HEIGHT+1) ,VD_RGB(128,128,128)/*VD_GetSysColor(COLOR_FRAME)*/);
#else
		m_DC.SetBrush(VD_RGB(128,128,128));
		m_DC.Rectangle(CRect(0, CTRL_HEIGHT + (i+1)*TEXT_HEIGHT, m_Rect.Width(),  CTRL_HEIGHT + (i+1)*TEXT_HEIGHT+1));
#endif
		m_DC.VD_TextOut(CRect(0, CTRL_HEIGHT + i*TEXT_HEIGHT+1,ITEM_HEAD_WIDTH, CTRL_HEIGHT + (i+1)*TEXT_HEIGHT-1), m_items[i].szItem.c_str());

		
		int datacount = m_items[i].recTime.size();
			
		m_DC.SetBrush(VD_RGB(72,110,216));
		m_DC.SetRgnStyle(RS_FLAT);

		for (int idx =0; idx< datacount; idx++)
		{
			CRect rtData;
			rtData.left = ITEM_HEAD_WIDTH + m_items[i].recTime[idx].starttime/m_dbSecPerPix;
			rtData.top = CTRL_HEIGHT + i*TEXT_HEIGHT+1;
			rtData.right = ITEM_HEAD_WIDTH + m_items[i].recTime[idx].endtime/m_dbSecPerPix;
			rtData.bottom = CTRL_HEIGHT + (i+1)*TEXT_HEIGHT-1;
			m_DC.Rectangle(&rtData);
		}

	}

	for (int kk = 0; kk<24; kk++)
	{
		CRect rt;
		rt.left = ITEM_HEAD_WIDTH + kk*m_hourwidth;
		rt.right = rt.left+1;
		rt.top =  CTRL_HEIGHT;
		rt.bottom = CTRL_HEIGHT + count*TEXT_HEIGHT;

#if 0
		m_DC.FillRect(rt ,VD_RGB(128,128,128)/*VD_GetSysColor(COLOR_FRAME)*/);
#else
		m_DC.SetBrush(VD_RGB(128,128,128));
		m_DC.Rectangle(rt);
#endif
	}

	m_DC.UnLock();

}

int CRecFileTimeBox::InsertItem( int index, VD_PCSTR string, VD_BOOL redraw /*= FALSE*/ )
{
	if(index < 0 || index > m_items.size())
	{
		index = m_items.size();
	}

	RecItem item;
	item.szItem = string;
	m_items.insert(m_items.begin() + index, item);

	if (redraw)
	{
		Draw();
	}

	return index;
}

void CRecFileTimeBox::DeleteItem( int index, VD_BOOL redraw /*= FALSE*/ )
{
	if(index < 0 || index > (m_items.size()-1))
	{
		return;
	}

	m_items.erase(m_items.begin() + index);

	if (redraw)
	{
		Draw();
	}

}

void CRecFileTimeBox::RemoveAllItem( VD_BOOL redraw /*= FALSE*/ )
{
	if(m_items.size() <1)
	{
		return;
	}

	m_items.erase(m_items.begin(), m_items.end());
	
	if(redraw)
	{
		Draw();
	}
}


void CRecFileTimeBox::AddItemTimeData(int index, RecTimeData* pData, VD_BOOL redraw /*= FALSE*/ )
{
	if(index < 0 || index > (m_items.size()-1) || !pData)
	{
		return;
	}

	RecTimeData timeItem;
	timeItem.starttime = MIN(pData->starttime, pData->endtime);
	timeItem.endtime = MAX(pData->starttime, pData->endtime);
	m_items[index].recTime.push_back(timeItem);

	if (redraw)
	{
		Draw();
	}

}

void CRecFileTimeBox::DrawTrack( int timepos )
{
	if (timepos < 0 || timepos>24*3600)
	{
		return;
	}
	
	m_DC.Lock();
	
	int w = 2;
	int h = m_items.size()*TEXT_HEIGHT;
	
	m_DC.SetBrush(VD_RGB(255,255,80));
	m_DC.SetRgnStyle(RS_FLAT);
	
	CRect rtTrack;
	rtTrack.left = ITEM_HEAD_WIDTH + (int)(timepos/m_dbSecPerPix);
	rtTrack.top = CTRL_HEIGHT;
	rtTrack.right = rtTrack.left+w;
	rtTrack.bottom = rtTrack.top + h;
	m_DC.Rectangle(&rtTrack);
	
	m_DC.UnLock();
}

VD_BOOL CRecFileTimeBox::MsgProc( uint msg, uint wpa, uint lpa )
{
	int key;
	int px, py;
	int newpos = m_nTrackTimePos/m_dbSecPerPix;
	BOOL bTrack = FALSE;

//	printf("CRecFileTimeBox::MsgProc %d %d %d\n", msg, wpa, lpa);

	switch(msg){
	case XM_LBUTTONDOWN:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);

		if (PtInRect(CRect(m_Rect.left+ITEM_HEAD_WIDTH, m_Rect.top+CTRL_HEIGHT, m_Rect.right, m_Rect.top+CTRL_HEIGHT+m_items.size()*TEXT_HEIGHT), px, py))
		{
#if 0
			if (PtInRect(CRect(m_Rect.left+ITEM_HEAD_WIDTH + m_nTrackTimePos/m_dbSecPerPix, 
				m_Rect.top+CTRL_HEIGHT,
				m_Rect.left+ITEM_HEAD_WIDTH + m_nTrackTimePos/m_dbSecPerPix+2, 
				m_Rect.top+m_items.size()*TEXT_HEIGHT), 
				px, py)) //track
			{
				bTrack = TRUE;
				SetFlag(IF_CAPTURED, TRUE);
			} 
			else //pos
#endif
			{
				
				newpos = px-m_Rect.left-ITEM_HEAD_WIDTH;

				//printf("set track pos %d \n", newpos);

				if(newpos < 0){
					newpos = 0;
				}else if(newpos > m_Rect.Width()-ITEM_HEAD_WIDTH){
					newpos =  m_Rect.Width()-ITEM_HEAD_WIDTH;
				}
			}

		} 
		else
		{
			return FALSE;
		}		
		break;

	case XM_LBUTTONUP:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(bTrack){
			newpos = px-m_Rect.left-ITEM_HEAD_WIDTH;
			if(newpos < 0){
				newpos = 0;
			}else if(newpos > m_Rect.Width()-ITEM_HEAD_WIDTH){
				newpos =  m_Rect.Width()-ITEM_HEAD_WIDTH;
			}

			bTrack = FALSE;
			SetFlag(IF_CAPTURED, FALSE);
		}
		break;

#if 0
	case XM_MOUSEMOVE:
		px = VD_HIWORD(lpa);
		py = VD_LOWORD(lpa);
		if(bTrack){
				newpos = px-m_Rect.left-ITEM_HEAD_WIDTH;
				if(newpos < 0){
					newpos = 0;
				}else if(newpos > m_Rect.Width()-ITEM_HEAD_WIDTH){
					newpos =  m_Rect.Width()-ITEM_HEAD_WIDTH;
				}
				m_nTrackTimePos = newpos*m_dbSecPerPix;
				DrawItem();
				DrawTrack(m_nTrackTimePos);

			}
			return TRUE;	
#endif
	default:
		return FALSE;
	}

	//»¬¿éÍÏ¶¯
	int curpos = m_nTrackTimePos/m_dbSecPerPix;
	if(newpos != curpos)
	{
		m_nTrackTimePos = newpos*m_dbSecPerPix;
		
		ItemProc(m_vProc);

		DrawItem();
		DrawTrack(m_nTrackTimePos);
	}

	return TRUE;
}

void CRecFileTimeBox::SetTrackTimePos( int timepos )
{
	if (timepos < 0 || timepos>24*3600)
	{
		return;
	}
	
	if (timepos != m_nTrackTimePos)
	{
		m_nTrackTimePos = timepos;
		DrawItem();
		DrawTrack(m_nTrackTimePos);
	}	
}

int CRecFileTimeBox::GetTrackTimePos()
{
	return m_nTrackTimePos;
}
CRecFileTimeBox* CreateRecTimeBox( VD_PCRECT pRect, CPage * pParent, CTRLPROC onClkTrack, uint style /*= 0*/ )
{
	return new CRecFileTimeBox(pRect, pParent, onClkTrack, style);
}
