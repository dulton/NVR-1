#include "GUI/Ctrls/Page.h"

CTableBox::CTableBox( VD_PCRECT pRect, CPage * pParent, int col, int row, uint style /*= 0*/,EM_TABLE_DRAWSTYLE drawStyle  )
:CItem(pRect, pParent, IT_TABLEBOX, style|styleAutoFocus)//, FALSE)
{
	if (!pRect)
	{
		return;
	}

	m_drawStyle = drawStyle;

	m_nCol = col>1? col:1;
	m_nRow = row>1? row:1;

	//printf("CTableBox::CTableBox 11\n");
	int i = 0;
	for (i=0; i<m_nCol; i++)
	{
		int defwidth = (pRect->right - pRect->left)/m_nCol;
		m_vColW.push_back(defwidth);
	}

	for (i=0; i<m_nRow; i++)
	{
		int defwidth = (pRect->bottom - pRect->top)/m_nRow;
		m_vRowH.push_back(defwidth);
	}
	//printf("CTableBox::CTableBox end\n");
	m_clorFrame = VD_GetSysColor(COLOR_FRAME);
}

CTableBox::~CTableBox()
{

}

void CTableBox::Draw()
{
	//printf("CTableBox::Draw() 111 \n");
	if(!DrawCheck()){
		return;
	}

	//printf("CTableBox::Draw() 222 \n");
	m_DC.Lock();
	//draw region

	if(m_drawStyle == EM_SOLID)
	{
		m_DC.SetBrush(VD_GetSysColor(VD_COLOR_WINDOW));
		m_DC.SetRgnStyle(RS_FLAT);
		m_DC.Rectangle(CRect(0,0,m_Rect.Width(),m_Rect.Height()));
	}

#if 0
	m_DC.FillRect(CRect(0, 0, m_Rect.Width(), 2) ,m_clorFrame/*VD_GetSysColor(COLOR_FRAME)*/);
	m_DC.FillRect(CRect(0, m_Rect.Height()-2, m_Rect.Width(), m_Rect.Height()) ,m_clorFrame/*VD_GetSysColor(COLOR_FRAME)*/);
	m_DC.FillRect(CRect(0, 0, 2, m_Rect.Height()) ,m_clorFrame/*VD_GetSysColor(COLOR_FRAME)*/);
	m_DC.FillRect(CRect( m_Rect.Width()-2, 0, m_Rect.Width(), m_Rect.Height()) ,m_clorFrame/*VD_GetSysColor(COLOR_FRAME)*/);

	int i = 0;
	int x=0, y=0;
	for (i = 0; i< m_nCol-1; i++)
	{
		x += m_vColW[i];
		
		
		m_DC.FillRect(CRect(x, 0, x + 1, m_Rect.Height()) ,m_clorFrame/*VD_GetSysColor(COLOR_FRAME)*/);
	}

	//printf("row = %d \n", m_nRow);

	for (i = 0; i< m_nRow-1; i++)
	{
		y += m_vRowH[i];

		m_DC.FillRect(CRect(0, y,  m_Rect.Width(), y+1) ,m_clorFrame/*VD_GetSysColor(COLOR_FRAME)*/);
	}
#else
	m_DC.SetBrush(m_clorFrame);
	
	m_DC.Rectangle(CRect(0, 0, m_Rect.Width(), 2)) ;
	m_DC.Rectangle(CRect(0, m_Rect.Height()-2, m_Rect.Width(), m_Rect.Height()));
	m_DC.Rectangle(CRect(0, 0, 2, m_Rect.Height()));
	m_DC.Rectangle(CRect( m_Rect.Width()-2, 0, m_Rect.Width(), m_Rect.Height()));

	int i = 0;
	int x=0, y=0;
	for (i = 0; i< m_nCol-1; i++)
	{
		x += m_vColW[i];
		
		m_DC.Rectangle(CRect(x, 0, x + 1, m_Rect.Height()));
	}

	//printf("row = %d \n", m_nRow);

	for (i = 0; i< m_nRow-1; i++)
	{
		y += m_vRowH[i];

		if (EM_GRAPH_OUTPUT_CVBS == GraphicsGetOutput())
		{
			m_DC.Rectangle(CRect(0, y,  m_Rect.Width(), y+2));
		}
		else
		{
			m_DC.Rectangle(CRect(0, y,  m_Rect.Width(), y+1));
		}
	}
#endif
	
	m_DC.UnLock();
	//printf("CTableBox::Draw() 333 \n");

}

void CTableBox::SetColWidth( int col, int width )
{
	if (col < -1 || col>=m_nCol || width<0)
	{
		return;
	}

	if (-1 == col)
	{
		for (int i = 0; i<m_nCol; i++)
		{
			m_vColW[i] = width;
		}
	}else
	{
		m_vColW[col] = width;
	}

	Draw();
}

void CTableBox::SetRowHeight( int row, int height )
{
	if (row < -1 || row>=m_nRow || height<0)
	{
		return;
	}

	if (-1 == row)
	{
		for (int i = 0; i<m_nRow; i++)
		{
			m_vRowH[i] = height;
		}
	}else
	{
		m_vRowH[row] = height;
	}

	Draw();
}

void CTableBox::GetTableRect( int col, int row, VD_PRECT pRect )
{
	if (row < -1 || row>=m_nRow || col < -1 || col>=m_nCol || !pRect)
	{
		return;
	}

	int i = 0;
	
#if 0
	pRect->left = m_Rect.left;
	pRect->right = m_Rect.left;
	pRect->top = m_Rect.top;
	pRect->bottom = m_Rect.top;	
#else
	pRect->left = m_Margin.left;
	pRect->right = 0;
	pRect->top = m_Margin.top;
	pRect->bottom = 0;	
#endif

	for (i= 0; i<col+1; i++)
	{
		pRect->right += m_vColW[i];
		pRect->left = pRect->right - m_vColW[i];
		//printf("m_vColW[i] %d\n",m_vColW[i]
	}

	if (1 == i)
	{
		pRect->left += 2;
	}
	else
	{
		pRect->left +=1;
	}
	
	if ((col+1) == m_nCol)
	{
		pRect->right = m_Rect.Width()-2;
	}
	else
	{
		pRect->right -= 1;
	}

	for (i = 0; i<row+1; i++)
	{
		pRect->bottom += m_vRowH[i];
		pRect->top = pRect->bottom -  m_vRowH[i];
		//printf("m_vRowH[i] %d\n",m_vRowH[i]);
	}

	if (1 == i)
	{
		pRect->top += 2;
	}
	else
	{
		pRect->top +=1;
	}

	if ((row+1) == m_nRow)
	{
		pRect->bottom = m_Rect.Height()-2;
	}
	else
	{
		pRect->bottom -= 1;
	}

	pRect->top += 1;
	pRect->bottom -= 1;
	pRect->left += 2;
	pRect->right -= 2;
}

void CTableBox::SetFrameColor( VD_COLORREF color )
{
	m_clorFrame = color;
	Draw();
}

CTableBox* CreateTableBox( VD_PCRECT pRect, CPage * pParent, int col,int row,uint style ,EM_TABLE_DRAWSTYLE drawStyle )
{
	return new CTableBox(pRect, pParent, col, row, style);
}

