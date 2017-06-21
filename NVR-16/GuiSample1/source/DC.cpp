

#include "GUI/GDI/DC.h"
#include "GUI/Pages/BizData.h"

#define DC_DEFLICKER

//csp modify
VD_BOOL CDC::m_bModify = FALSE;

///////////////////////////////////////////////////////////////////
////////////        CDC
CDC::CDC(int type /* = DC_SCREEN */, int width /* = 0 */, int height /* = 0 */):m_PenWidth(0),m_PenStyle(0),m_FontColor(0),
m_FontStyle(FS_NORMAL),m_BrushColor(0),m_BrushStyle(0),m_KeyColor(0),m_BitmapMode(0),m_TextAlign(0),m_RgnStyle(0),m_Mutex(MUTEX_RECURSIVE)
{
	//csp modify 20130323
	m_pBuffer = NULL;
	
	m_nType = type;
	
	//printf("CDC::CDC()000\n");
	m_locals = CLocales::instance();
	bizData_SetLocales(m_locals);
	//printf("CDC::CDC()111\n");
	m_locals->GetFontSize(&m_FontSize);
	//printf("CDC::CDC()222\n");
	
	m_Pos.x = m_Pos.y = 0;
	
	Update();
	
	if(m_nType == DC_MEMORY && width > 0 && height > 0) //内存DC
	{
		m_pBuffer = new uchar[width * height * 2];
		if(m_pBuffer == NULL)
		{
			printf("#####################################################warning:create DC_MEMORY width=%d height=%d m_pBuffer=0x%08x\n",width,height,(unsigned int)m_pBuffer);
		}
		memset(m_pBuffer, 0, width * height * 2);
		m_Width = width;
		m_Height = height;
		m_Pitch = m_BPP * width;
	}
	
	//raster operation
	SetRop(ROP_COPY);
	//m_BkMode = BM_TRANSPARENT;
	m_BkMode = BM_NORMAL;
	
	m_TextAlign = VD_TA_LEFTTOP;
	m_BkColor = MakeColor(m_CS, VD_GetSysColor(VD_COLOR_WINDOW));
	m_FontColor = MakeColor(m_CS, VD_GetSysColor(VD_COLOR_WINDOWTEXT));
	m_PenColor = MakeColor(m_CS, VD_GetSysColor(COLOR_FRAME));
	m_TransColor = MakeColor(m_CS, VD_GetSysColor(COLOR_TRANSPARENT));
	
	//printf("3\n");
	
	m_bEnable = TRUE;//先开起来, 让SetBound设置成功
	SetBound(CRect(0, 0, m_Width, m_Height));
	
	//printf("4\n");
	
	if(type != DC_MEMORY)
	{
		m_bEnable = FALSE; //屏幕DC再次关闭开关
		
		//printf("CDC::CDC()555\n");
		
		m_pCursor = CCursor::instance(type);
		
		//printf("CDC::CDC()666\n");
	}
	
	//printf("CDC::CDC() end\n");
}

CDC::~CDC()
{
	if (m_nType == DC_MEMORY && m_pBuffer)
	{
		delete []m_pBuffer;
	}
}

void CDC::Lock()
{
	m_Mutex.Enter();
}

void CDC::UnLock()
{
	m_Mutex.Leave();
}

void CDC::Update()
{
	GRAPHICS_SURFACE surface;
	memset(&surface, 0, sizeof(GRAPHICS_SURFACE));
	CDevGraphics::instance((m_nType == DC_MEMORY) ? 0 : m_nType)->GetSurface(&surface);
	if(m_nType != DC_MEMORY)
	{
		m_Width = surface.width;
		m_Height = surface.height;
		m_pBuffer = surface.mem;
		m_Pitch = surface.pitch;
	}
	m_CS = surface.format;
	switch(m_CS)
	{
	case GRAPHICS_CF_RGB555:
		m_BPP = 2;
		break;
	case GRAPHICS_CF_RGB565:
		m_BPP = 2;
		break;
	default:
		m_BPP = 1;
		break;
	}
}

void CDC::SetBound(VD_PCRECT pRect)
{
	if (!m_bEnable)
	{
		return;
	}

	if (pRect)
	{
		CRect safe_rect(0, 0, m_Width, m_Height); //安全区域

		m_Bound = *pRect;
		safe_rect.IntersectRect(safe_rect, m_Bound);
		m_Rgn.SetRectRgn(&safe_rect);
	}
	else
	{
		m_Bound.SetRectEmpty();
		m_Rgn.SetRectRgn(NULL);
	}
}

void CDC::Enable(VD_BOOL flag)
{
	if (flag && !m_pBuffer)//没有缓冲,返回
	{
		return;
	}
	m_bEnable = flag;
}

VD_BOOL CDC::SubtractRect(VD_PCRECT pRect)
{
	if (!m_bEnable)
	{
		return FALSE;
	}

	m_Rgn.SubtractRect(pRect);
	return TRUE;
}

VD_BOOL CDC::UnionRect(VD_PCRECT pRect)
{
	if (!m_bEnable)
	{
		return FALSE;
	}

	m_Rgn.UnionRect(pRect);
	return TRUE;
}

VD_BOOL CDC::IntersectRect(VD_PCRECT pRect)
{
	if (!m_bEnable)
	{
		return FALSE;
	}

	m_Rgn.IntersectRect(pRect);
	return TRUE;
}

VD_BOOL CDC::SubtractRgn(CRgn * pRgn)
{
	if (!m_bEnable)
	{
		return FALSE;
	}

	m_Rgn.SubtractRgn(pRgn);
	return TRUE;
}

VD_BOOL CDC::UnionRgn(CRgn * pRgn)
{
	if (!m_bEnable)
	{
		return FALSE;
	}

	m_Rgn.UnionRgn(pRgn);
	return TRUE;
}

VD_BOOL CDC::IntersectRgn(CRgn * pRgn)
{
	if (!m_bEnable)
	{
		return FALSE;
	}

	m_Rgn.IntersectRgn(pRgn);
	return TRUE;
}

VD_BOOL CDC::GetRgn(CRgn * pRgn)
{
	if (!m_bEnable)
	{
		return FALSE;
	}

	*pRgn = m_Rgn;
	return TRUE;
}

VD_BOOL CDC::SetRgn(CRgn * pRgn)
{
	if (!m_bEnable)
	{
		return FALSE;
	}

	m_Rgn = *pRgn;
	return TRUE;
}

VD_BOOL CDC::SwapRgn(CRgn * pRgn)
{
	if (!m_bEnable)
	{
		return FALSE;
	}

	m_Rgn.SwapRgn(pRgn);
	return TRUE;
}

void CDC::SetRop(int rop/* = ROP_COPY */)
{
	m_ROP = rop;
	m_pPixelProc = SelectPixelProc(m_ROP, m_BPP-1);
	m_pHLineProc = SelectHLineProc(m_ROP, m_BPP-1);//(0,1)
	m_pBitmapProc = SelectBitmapProc(m_ROP, m_BPP-1);
}

void CDC::SetFont(VD_COLORREF color, FONT_STYLE style/* =FS_NORMAL */)
{
	m_FontColor = MakeColor(m_CS, color);
	m_FontStyle = style;
	
	if (m_FontStyle == FS_SMALL)
	{
		m_locals->GetFontSize(&m_FontSize, FONTSIZE_SMALL);	
	}
	else
	{
		m_locals->GetFontSize(&m_FontSize); 
	}
}

void CDC::SetFontSize(int height, double ratio)
{
}

void CDC::SetBrush(VD_COLORREF color, int style/* =BS_NORMAL */)
{
	m_BrushColor = MakeColor(m_CS, color);
	m_BrushStyle = style;
}
void CDC::SetBrush2(VD_COLORREF color, int style/* =BS_NORMAL */)
{
	m_BrushColor = MakeColor(m_CS, color) |(1<<15);	
	m_BrushStyle = style;
}


void CDC::SetPen(VD_COLORREF color, int style/* =PS_SOLID */, int width/* =1 */)
{
	m_PenColor = MakeColor(m_CS, color);
	//make light pen
	m_PenColorLight = ModulateColor(m_CS, color,40,40,40);
	m_PenColorLight = MakeColor(m_CS, m_PenColorLight);
	//make dark pen
	m_PenColorDark = ModulateColor(m_CS, color,-40,-40,-40);
	m_PenColorDark = MakeColor(m_CS, m_PenColorDark);
	m_PenStyle = style;
	m_PenWidth = width;
}

void CDC::SetPenWidth(int width)
{
	m_PenWidth = width;
}

void CDC::SetBkColor(VD_COLORREF color)
{
	m_BkColor = MakeColor(m_CS, color);
}

void CDC::SetBkMode(int mode)
{
	m_BkMode = mode;
}

int CDC::GetBkMode()
{
	return m_BkMode;
}

void CDC::SetRgnStyle(int style)
{
	m_RgnStyle = style;
}

void CDC::SetPixel(int x, int y, VD_COLORREF c)
{
	if (!m_bEnable)
	{
		return;
	}

	if (m_Rgn.PtInRegion(x, y))
	{
		if (m_nType != DC_MEMORY)
		{
			m_pCursor->UpdateCursor(FALSE, CRect(x, y, x + 1, y + 1));
		}
		m_pPixelProc(m_pBuffer + m_Pitch * y + x * m_BPP, MakeColor(m_CS, c));
		if (m_nType != DC_MEMORY)
		{
			m_pCursor->UpdateCursor(TRUE, CRect(x, y, x + 1, y + 1));
		}
		
		//csp modify
		SetModify(TRUE);
	}
}

VD_COLORREF CDC::GetPixel(int x, int y)
{
	if (!m_bEnable)
	{
		return VD_RGB(0,0,0);
	}

	if (m_pBuffer == 0 || !PtInRect(&m_Bound, x, y))
	{
//		trace("CDC::PixelPicked param error: (%x), (%d %d)\n", (int)m_pBuffer, x, y);
		return FALSE;
	}
	return *(unsigned short*)(m_pBuffer + x*2 + y*m_Pitch);
}

void CDC::SetTextAlign(int align /* = TA_LEFTTOP */)
{
	m_TextAlign = align;
}

//判断字符是否构成单词: 英文,西欧,俄文,常用字符等拼在一起认为构成单词
#define IN_WORD(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') \
	|| x == '(' || x == ')' || (x >= '0' && x <= '9') \
	|| (x >= 0x0080 && x <= 0x00ff) \
	|| (x >= 0x0400 && x <= 0x04ff) \
	|| x == '"')  //added by wyf on 090909


void CDC::VD_TextOut(VD_PCRECT pRect, VD_PCSTR str, int len/* =1024 */)
{
	if (!pRect || !str)
	{
		return;
	}

	if (!m_bEnable)
	{
		return;
	}

	CRect rect(pRect);
	LPtoDP(rect);
	rect.NormalizeRect();

	//判断是否有相交区域
	VD_RECT update_rect;
	m_Rgn.GetBoundRect(&update_rect);
	if (!::IntersectRect(&update_rect, &update_rect, rect))
	{
		return;
	}

	if (m_nType != DC_MEMORY)
	{
		m_pCursor->UpdateCursor(FALSE, &update_rect);
	}

	int i;//字节计数
	int n;//字符计数
	int n1;//行开始的字符记数
	int w;//一行字符点阵的宽度

	ushort code; //字符Unicode
	int wordwidth;//单词宽度
	int cw;//字符宽度
	int cl;//字符字节数
	int width1;//字符串宽度
	int x = rect.left;
	int y = rect.top;
	if (len > (int)strlen(str))
	{
		len = strlen(str);
	}
	
	CRgn rgn;
	rgn.CreateRectRgn(rect);
	rgn.IntersectRgn(&m_Rgn);
	
	width1 = (m_locals->GetTextExtent(str, len, m_FontStyle) + 1) & 0xfffffffe;
	switch (m_TextAlign & 0x0f)
	{
	case VD_TA_LEFT:
		break;
	case VD_TA_XLEFT:
		x = x + 4;
		break;
	case VD_TA_XCENTER:
		x += (rect.Width() - width1) / 2;
		break;
	case VD_TA_RIGHT:
		x += (rect.Width() - width1);
		break;
	default:
		break;
	}

	switch (m_TextAlign & 0xf0)
	{
	case VD_TA_TOP:
		break;
	case VD_TA_YCENTER:
		y += (rect.Height() - m_FontSize.h) / 2;
		break;
	case VD_TA_BOTTOM:
		y += (rect.Height() - m_FontSize.h);
		break;
	default:
		break;
	}

	for (n = 0, n1 = 0, w = 0; n < len; n += cl, w += cw)
	{
		code = m_locals->GetCharCode(&str[n], &cl);
		if(cl == 0)
		{
			break;
		}

		if (m_FontStyle == FS_SMALL)
		{
			cw = m_locals->GetCharRaster(code, NULL, FONTSIZE_SMALL); 
		}
		else
		{
			cw = m_locals->GetCharRaster(code); 
		}

		//英文单词长度计算
		wordwidth = 0;
		if ((m_TextAlign&VD_TA_CHANGELINE ) && !IN_WORD(code) && n < len - 1)
		{
			int a, b;
			i = n + 1;

			code = m_locals->GetCharCode(&str[i], &b);
			if (IN_WORD(code))
			{
				while (i < len && !IN_WORD(code))
				{
					code = m_locals->GetCharCode(&str[i], &b);
					if(b == 0)
					{
						break;
					}
					i += b;

					if (m_FontStyle == FS_SMALL)
					{
						a = m_locals->GetCharRaster(code, NULL, FONTSIZE_SMALL);  
					}
					else
					{
						a = m_locals->GetCharRaster(code);
					}

					wordwidth += a;
				}
				while (i < len && IN_WORD(code))
				{
					code = m_locals->GetCharCode(&str[i], &b);
					if(b == 0)
					{
						break;
					}
					i += b;

					if (m_FontStyle == FS_SMALL)
					{
						 a = m_locals->GetCharRaster(code, NULL, FONTSIZE_SMALL);
					}
					else
					{
						a = m_locals->GetCharRaster(code);
					}

					wordwidth += a;
				}
			}
		}

		if (str[n] == '\n'|| x + cw + w + wordwidth > rect.right || n >= len - cl)
		{//换行操作
			if (n >= len - cl || !(m_TextAlign & VD_TA_CHANGELINE))
			{
				_TextInRgn(&rgn, x, y, &str[n1], n - n1 + cl);
			}
			else
			{
				_TextInRgn(&rgn, x, y, &str[n1], n - n1);
			}
			n1 = n + (wordwidth ? cl : 0);
			w = 0;
			y += m_FontSize.h + ROW_SPACE;
			if (y + m_FontSize.h > rect.bottom || !(m_TextAlign&VD_TA_CHANGELINE))
			{
				break;
			}
		}
	}
	if (m_nType != DC_MEMORY)
	{
		m_pCursor->UpdateCursor(TRUE, &update_rect);
	}
	
	//csp modify
	SetModify(TRUE);
}

void CDC::Bitmap(VD_PCRECT pRect, VD_BITMAP * pBitmap, int start_x /* = 0 */, int start_y /* = 0 */)
{
	if (!m_bEnable)
	{
		return;
	}

	if (pRect == NULL || pBitmap == NULL || pBitmap->bits == NULL)
	{
		return;
	}

	if (!::PtInRect(CRect(0, 0, pBitmap->width, pBitmap->height), start_x, start_y))
	{
//		trace("CDC::Bitmap Source offset (%d,%d) not match (0 ~ %d,0 ~ %d).\n",
//			start_x, start_y, pBitmap->width, pBitmap->height);
		return;
	}

	//位图每行数据4字节对齐
	int line = (pBitmap->width * m_BPP + 3) & 0xfffffc;
	
	CRect rect(pRect);
	LPtoDP(rect);
	rect.NormalizeRect();

	//判断是否有相交区域
	VD_RECT update_rect;
	m_Rgn.GetBoundRect(&update_rect);
	if (!::IntersectRect(&update_rect, &update_rect, rect))
	{
		return;
	}

	if (m_nType != DC_MEMORY)
	{
		m_pCursor->UpdateCursor(FALSE, &update_rect);
	}

	start_x = rect.left - start_x;
	start_y = rect.top - start_y;

	rect.IntersectRect(&rect, &m_Bound);
	rect.IntersectRect(&rect, CRect(start_x, start_y, start_x + pBitmap->width, start_y + pBitmap->height));

	CRgn rgn;
	rgn.CreateRectRgn(rect);
	rgn.IntersectRgn(&m_Rgn);

	_BitmapInRgn(&rgn, pBitmap, line, start_x, start_y);

	if (m_nType != DC_MEMORY)
	{
		m_pCursor->UpdateCursor(TRUE, &update_rect);
	}
	
	//csp modify
	SetModify(TRUE);
}

void CDC::StyledBitmap(VD_PCRECT rect, VD_BITMAP *pBitmap, VD_PCRECT areaEnlarge)
{	
	int line = (pBitmap->width * m_BPP + 3) & 0xfffffc;
	CRect rectUpdate;
	CRect rectDraw;
	int xoffset[9], yoffset[9];
	int bkmode = m_BkMode;
	int xstart, ystart;
	int wsrc , hsrc;
	int wdest, hdest;
	int ww, hh;
	int wblock, hblock;

	if (rect == NULL || pBitmap == NULL || pBitmap->bits == NULL || areaEnlarge == NULL)
	{
		return;
	}

	m_Rgn.GetBoundRect(&rectUpdate);
	if (!::IntersectRect(&rectUpdate, &rectUpdate, rect))
	{
		return;
	}
	xstart = 0;
	ystart = 0;
	wsrc = pBitmap->width;
	hsrc = pBitmap->height;
	wdest = rect->right - rect->left;
	hdest = rect->bottom - rect->top;
	
	if (wsrc  <= 0 || hsrc  <= 0 || wdest  <= 0 || hdest  <= 0)
	{
		return;
	}
	wblock = areaEnlarge->right - areaEnlarge->left;
	hblock = areaEnlarge->bottom - areaEnlarge->top;
	xoffset[0] = xoffset[3] = xoffset[6] = 0;
	xoffset[1] = xoffset[4] = xoffset[7] = areaEnlarge->left;
	xoffset[2] = xoffset[5] = xoffset[8] = areaEnlarge->right;
	yoffset[0] = yoffset[1] = yoffset[2] = 0;
	yoffset[3] = yoffset[4] = yoffset[5] = areaEnlarge->top;
	yoffset[6] = yoffset[7] = yoffset[8] = areaEnlarge->bottom;

	if (m_nType != DC_MEMORY)
	{
		m_pCursor->UpdateCursor(false, rectUpdate);
	}
	rectDraw.SetRect(rect->left, rect->top, rect->left + areaEnlarge->left, rect->top + areaEnlarge->top);
	_WideBitmapInRect(rectDraw, pBitmap, line, xoffset[0], yoffset[0]); //0
	m_BkMode = BM_NORMAL;
	for (ww=rect->left + areaEnlarge->left; ww <= rect->right-(pBitmap->width - areaEnlarge->right); ww += wblock)
	{
		rectDraw.SetRect(ww, rect->top, MIN((rect->right-(pBitmap->width - areaEnlarge->right)), (ww + wblock)), rect->top + areaEnlarge->top);
		_WideBitmapInRect(rectDraw, pBitmap, line, xoffset[1], yoffset[1]); //1		
	}
	m_BkMode = bkmode;
	rectDraw.SetRect(rect->right-(pBitmap->width - areaEnlarge->right), rect->top, rect->right, rect->top + areaEnlarge->top);
	_WideBitmapInRect(rectDraw, pBitmap, line, xoffset[2], yoffset[2]); //2
	m_BkMode = BM_NORMAL;
	for (hh=rect->top + areaEnlarge->top; hh <= rect->bottom-(pBitmap->height - areaEnlarge->bottom); hh += hblock)
	{
		rectDraw.SetRect(rect->left, hh, rect->left + areaEnlarge->left, 
			MIN((rect->bottom-(pBitmap->height - areaEnlarge->bottom)), (hh + hblock)));
		_WideBitmapInRect(rectDraw, pBitmap, line, xoffset[3], yoffset[3]); //3
		for (ww=rect->left + areaEnlarge->left; ww <= rect->right-(pBitmap->width - areaEnlarge->right); ww += wblock)
		{
			rectDraw.SetRect(ww, hh, MIN((rect->right-(pBitmap->width - areaEnlarge->right)), (ww + wblock)), 
				MIN((rect->bottom-(pBitmap->height - areaEnlarge->bottom)), (hh + hblock)));
			_WideBitmapInRect(rectDraw, pBitmap, line, xoffset[4], yoffset[4]); 
		}
		if (rect->right-wblock != rect->left)
		{
			rectDraw.SetRect(rect->right-(pBitmap->width - areaEnlarge->right), hh, rect->right, 
				MIN((rect->bottom- (pBitmap->height - areaEnlarge->bottom)), (hh + hblock)));
			_WideBitmapInRect(rectDraw, pBitmap, line, xoffset[5], yoffset[5]); //5
		}
	}
	m_BkMode = bkmode;
	if (rect->bottom-hblock != rect->top || rect->right - wblock != rect->left)
	{
		rectDraw.SetRect(rect->left, rect->bottom-(pBitmap->height - areaEnlarge->bottom), 
			areaEnlarge->left + rect->left, rect->bottom);
		_WideBitmapInRect(rectDraw, pBitmap, line, xoffset[6], yoffset[6]); //6
	}

	if (rect->bottom-hblock != rect->top)
	{
		m_BkMode = BM_NORMAL;
		for (ww=rect->left + areaEnlarge->left; ww <= rect->right-(pBitmap->width - areaEnlarge->right); ww += wblock)
		{
			rectDraw.SetRect(ww, rect->bottom-(pBitmap->height - areaEnlarge->bottom), 
				MIN((rect->right - (pBitmap->width - areaEnlarge->right)), (ww + wblock)), rect->bottom);
			_WideBitmapInRect(rectDraw, pBitmap, line, xoffset[7], yoffset[7]); //7		
		}
		m_BkMode = bkmode;
	}

	if (rect->bottom-hblock != rect->top || rect->right-wblock != rect->left)
	{
		rectDraw.SetRect(rect->right-(pBitmap->width - areaEnlarge->right), 
							rect->bottom-(pBitmap->height - areaEnlarge->bottom), rect->right, rect->bottom);
		_WideBitmapInRect(rectDraw, pBitmap, line, xoffset[8], yoffset[8]); //8
	}
	if (m_nType != DC_MEMORY)
	{
		m_pCursor->UpdateCursor(true, rectUpdate);
	}
	
	//csp modify
	SetModify(TRUE);
}

VD_BOOL CDC::BitBlt(VD_PCRECT pRect, CDC* pDC, int start_x /* = 0 */, int start_y /* = 0 */)
{
	if (!pRect || !pDC || pDC->m_CS != m_CS)
	{
		return FALSE;
	}
	VD_BITMAP bmp;
	bmp.width = pDC->m_Width;
	bmp.height = pDC->m_Height;
	bmp.bits = pDC->m_pBuffer;
	Bitmap(pRect, &bmp, start_x, start_y);
	
	//csp modify
	SetModify(TRUE);
	
	return TRUE;
}

void CDC::DPtoLP(VD_PPOINT pPoints, int nCount /* = 1 */) const
{
	for (int i = 0; i < nCount; i++)
	{
		pPoints->x -= m_Bound.left;
		pPoints->y -= m_Bound.top;
	}
}

void CDC::DPtoLP(VD_PRECT pRect) const
{
	OffsetRect(pRect, -m_Bound.left, -m_Bound.top);
}

void CDC::LPtoDP(VD_PPOINT pPoints, int nCount /* = 1 */) const
{
	for (int i = 0; i < nCount; i++)
	{
		pPoints->x += m_Bound.left;
		pPoints->y += m_Bound.top;
	}
}

void CDC::LPtoDP(VD_PRECT pRect) const
{
	OffsetRect(pRect, m_Bound.left, m_Bound.top);
}

int CDC::GetPenWidth()
{
	int i =0;
	printf("CDC::GetPenWidth() penwidth=%d\n",m_PenWidth);
	i = m_PenWidth;
	return i;
}

void CDC::Rectangle(VD_PCRECT pRect, int ea /* = 0 */, int eb /* = 0 */)
{
	VD_COLORREF old_pen_color;
	if (!pRect)
	{
		return;
	}
	if (!m_bEnable)
	{
		return;
	}

	CRect rect(pRect);
	LPtoDP(rect);
	rect.NormalizeRect();

	//判断是否有相交区域
	VD_RECT update_rect;
	m_Rgn.GetBoundRect(&update_rect);
	if (!::IntersectRect(&update_rect, &update_rect, rect))
	{
		return;
	}
	
	if (m_nType != DC_MEMORY)
	{
		m_pCursor->UpdateCursor(FALSE, &update_rect);
	}
	
	int point_pair[10][3];	//端点对
	int number;
	int ex1,ex2,ex3,ey1,ey2,ey3,xx,yy;
	int i;
	//implement round rectangle
	if (ea && eb)
	{
		number = 10;
		if (ea>rect.Width() / 2)
		{
			ea = rect.Width() / 2;
		}
		if (eb>rect.Height() / 2)
		{
			eb = rect.Height() / 2;
		}
		xx = rect.left + rect.right - 1;
		yy = rect.top + rect.bottom - 1;
		ex1 = ea * TRIGO_CONST3 / 1000;
		ex2 = ea * TRIGO_CONST2 / 1000;
		ex3 = ea * TRIGO_CONST1 / 1000;
		ey1 = eb * TRIGO_CONST3 / 1000;
		ey2 = eb * TRIGO_CONST2 / 1000;
		ey3 = eb * TRIGO_CONST1 / 1000;
		point_pair[0][0] = rect.left+ea;
		point_pair[0][2] = rect.top;
		point_pair[1][0] = rect.left+ea-ex3;
		point_pair[1][2] = rect.top+eb-ey1;
		point_pair[2][0] = rect.left+ea-ex2;
		point_pair[2][2] = rect.top+eb-ey2;
		point_pair[3][0] = rect.left+ea-ex1;
		point_pair[3][2] = rect.top+eb-ey3;
		point_pair[4][0] = rect.left;
		point_pair[4][2] = rect.top+eb;
		for (i=0;i<5;i++)
		{
			point_pair[i][1] = xx - point_pair[i][0];
		}
		for (i=0;i<5;i++)
		{
			point_pair[9-i][0] = point_pair[i][0];
			point_pair[9-i][1] = point_pair[i][1];
			point_pair[9-i][2] = yy - point_pair[i][2];
		}
	}
	else
	{
		number = 2;
		point_pair[0][0] = rect.left;
		point_pair[0][1] = rect.right - 1;
		point_pair[0][2] = rect.top;
		point_pair[1][0] = rect.left;
		point_pair[1][1] = rect.right - 1;
		point_pair[1][2] = rect.bottom - 1;
	}

	if (!(m_RgnStyle & RS_HOLLOW))
	{
		int y;
		int dy;
		int s_x,e_x;
		CRgn rgn;
		int dx1;
		int dx2;
		int ax1;
		int ax2;

		for (i=0;i<number-1;i++)
		{
			dy = point_pair[i+1][2] - point_pair[i][2];
			if (dy < 0)
			{
				continue;
			}
			if (dy == 0 || (point_pair[i+1][0] == point_pair[i][0] && point_pair[i+1][1] == point_pair[i][1]))
			{
				rgn.UnionRect(CRect(point_pair[i][0], point_pair[i][2], point_pair[i][1] + 1, point_pair[i+1][2] + 1));
				continue;
			}
			dx1 = (point_pair[i + 1][0] - point_pair[i][0]);
			dx2 = (point_pair[i + 1][1] - point_pair[i][1]);
			ax1 = (dx1 > 0) ? 1 : -1;
			ax2 = (dx2 > 0) ? 1 : -1;
			for (y = point_pair[i][2]; y <= point_pair[i + 1][2]; y++)
			{
				s_x = point_pair[i][0] + (dx1 * (y - point_pair[i][2]) * 2 / dy + ax1) / 2;
				e_x = point_pair[i][1] + (dx2 * (y - point_pair[i][2]) * 2 / dy + ax2) / 2;
				rgn.UnionRect(CRect(s_x, y, e_x + 1, y + 1));
			}
		}
		rgn.IntersectRgn(&m_Rgn);
		_FillRgn(&rgn);
	}
	if (!(m_RgnStyle & RS_FLAT))
	{
		//save pen
		old_pen_color = m_PenColor;
		//select pen
		if (m_RgnStyle&RS_SUNKEN)
		{
			m_PenColor = m_PenColorDark;
		}
		else if (m_RgnStyle&RS_RAISED)
		{
			m_PenColor = m_PenColorLight;
		}
		//draw top edge 
		_LineInRgn(&m_Rgn, point_pair[0][1], point_pair[0][2], point_pair[0][0], point_pair[0][2]);
		//select pen
		if (m_RgnStyle&RS_SUNKEN)
		{
			m_PenColor = m_PenColorLight;
		}
		else if (m_RgnStyle&RS_RAISED)
		{
			m_PenColor = m_PenColorDark;
		}
		//draw bottom edge
		_LineInRgn(&m_Rgn, point_pair[number-1][0], point_pair[number-1][2], point_pair[number-1][1], point_pair[number-1][2]);
		//draw right edge
		for (i=number-1;i>0;i--)
		{
			_LineInRgn(&m_Rgn, point_pair[i][1], point_pair[i][2], point_pair[i-1][1], point_pair[i-1][2]);
		}
		//select pen
		if (m_RgnStyle&RS_SUNKEN)
			m_PenColor = m_PenColorDark;
		else if (m_RgnStyle&RS_RAISED)
			m_PenColor = m_PenColorLight;
		//draw left edge
		for (i=0;i<number-1;i++)
		{
			_LineInRgn(&m_Rgn, point_pair[i][0], point_pair[i][2], point_pair[i+1][0], point_pair[i+1][2]);
		}
		//recover pen
		m_PenColor = old_pen_color;
	}
	
	if (m_nType != DC_MEMORY)
	{
		m_pCursor->UpdateCursor(TRUE, &update_rect);
	}
	
	//csp modify
	SetModify(TRUE);
}

void CDC::Trapezoid(int x1, int y1, int w1, int x2, int y2, int w2)
{
	int y;
	int dy;
	CRgn rgn;
	int s_x,e_x;

	if (!m_bEnable)
	{
		return;
	}

	CPoint point1(x1, y1);
	CPoint point2(x2, y2);
	LPtoDP(point1);
	LPtoDP(point2);


	dy = point2.y - point1.y;
	if (dy<=0){
		return;
	}
	int max1, min1, max2, min2;
	if (w1>0)
	{
		max1 = point1.x + w1;
		min1 = point1.x;
	}
	else
	{
		max1 = point1.x;
		min1 = point1.x + w1;
	}
	if (w2>0)
	{
		max2 = point2.x + w2;
		min2 = point2.x;
	}
	else
	{
		max2 = point2.x;
		min2 = point2.x + w2;
	}
	CRect rect(MIN(min1, min2), point1.y, MAX(max1, max2) + 1, point2.y + 1);
	rect.NormalizeRect();

	//判断是否有相交区域
	VD_RECT update_rect;
	m_Rgn.GetBoundRect(&update_rect);
	if (!::IntersectRect(&update_rect, &update_rect, rect))
	{
		return;
	}

	if (m_nType != DC_MEMORY)
	{
		m_pCursor->UpdateCursor(FALSE, &update_rect);
	}
	
	//trace("fill with %lx\n",color);
	for (y=point1.y;y<=point2.y;y++){
		if (w1 == 0)
			s_x = point1.x;
		else
			s_x = point1.x+w1*(y-point1.y)/dy;
		if (w2 == 0)
			e_x = point2.x;
		else
			e_x = point2.x+w2*(y-point1.y)/dy;
		rgn.UnionRect(CRect(s_x, y, e_x + 1, y + 1));
	}
	rgn.IntersectRgn(&m_Rgn);
	_FillRgn(&rgn);

	if (m_nType != DC_MEMORY)
	{
		m_pCursor->UpdateCursor(TRUE, &update_rect);
	}
	
	//csp modify
	SetModify(TRUE);
}

void CDC::MoveTo(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
	LPtoDP(&m_Pos);
}

void CDC::LineTo(int x, int y)
{
	if (!m_bEnable)
	{
		return;
	}

	CPoint pos(x, y);
	
	LPtoDP(&pos);

	_LineInRgn(&m_Rgn, m_Pos.x, m_Pos.y, pos.x, pos.y, TRUE);

	m_Pos = pos;

	//csp modify
	SetModify(TRUE);
}
//
//int CDC::GetTextExtent(PCSTR str, int len)
//{
//	if (!str){
//		return 0;
//	}
//	WORD code;//字符unicode
//	int n;
//	int w;//字符宽度
//	int l;//字符字节数
//	int width1 = 0;//字符串宽度
//
//	if (len>(int)strlen(str))
//	{
//		len = strlen(str);
//	}
//	for (n = 0; n < len; n += l, width1 += w)
//	{
//		code = m_locals->GetCharCode(&str[n], &l);
//		if(l == 0)
//		{
//			break;
//		}
//
//		if(m_FontStyle == FS_SMALL)
//		{
//			w = m_locals->GetCharRaster(code, NULL, FONTSIZE_SMALL);
//		}
//		else
//		{
//			w = m_locals->GetCharRaster(code);		
//		}
//
//	}
//	return width1;
//}


void CDC::_FillRgn(CRgn* pRgn)
{
	VD_PCRECT pcRect = pRgn->GetFirstRect();
	while (pcRect)
	{
		_FillRect(pcRect);
		pcRect = pRgn->GetNextRect();
	}
}
void CDC::_LineInRgn(CRgn* pRgn, int x1, int y1, int x2, int y2, VD_BOOL bUpdateCursor /* = FALSE */)
{
	int dx = x2 - x1;
	int dy = y2 - y1;
	int width;
	int inc1 = 0;
	int inc2 = 0;
	int incx;
	int incy;
	
	if (ABS(dx) >= ABS(dy))
	{
		inc2 = (dx < 0) ? 1 : -1;
	}
	else
	{
		inc1 = (dy < 0) ? -1 : 1;
	}
	if (m_PenWidth > 11)
	{
		//printf("CDC::_LineInRgn  warning m_PenWidth bigger than 10 %d\n",m_PenWidth);
	}

	for (width = 0; width < m_PenWidth; width++)
	{
		incx = inc1*width;
		incy = inc2*width;
		VD_PCRECT pcRect = pRgn->GetFirstRect();
		while (pcRect)
		{
			if (m_nType != DC_MEMORY && bUpdateCursor)
			{
				m_pCursor->UpdateCursor(FALSE, x1+incx, y1+incy, x2+incx, y2+incy);
			}
			_LineInRect(pcRect, x1+incx, y1+incy, x2+incx, y2+incy);
			if (m_nType != DC_MEMORY && bUpdateCursor)
			{
				m_pCursor->UpdateCursor(TRUE, x1+incx, y1+incy, x2+incx, y2+incy);
			}
			pcRect = pRgn->GetNextRect();
		}
	}
}

void CDC::_TextInRgn(CRgn* pRgn, int x, int y, VD_PCSTR str, int len)
{
	VD_PCRECT pcRect = pRgn->GetFirstRect();
	while (pcRect)
	{
		if ((m_FontStyle == FS_NORMAL) || (m_FontStyle == FS_SMALL))
		{
			_TextInRect(pcRect, x, y, str, len);
		}
		else if (m_FontStyle == FS_BOLD)
		{
			_TextInRect(pcRect, x, y, str, len);
			_TextInRect(pcRect, x + 1, y, str, len);
		}
		else
		{
			_TextInRect(pcRect, x, y, str, len);
			_TextInRect(pcRect, x, y+1, str, len);
			_TextInRect(pcRect, x, y+2, str, len);
			_TextInRect(pcRect, x+1, y, str, len);
			_TextInRect(pcRect, x+1, y+2, str, len);
			_TextInRect(pcRect, x+2, y, str, len);
			_TextInRect(pcRect, x+2, y+1, str, len);
			_TextInRect(pcRect, x+2, y+2, str, len);
		}
		pcRect = pRgn->GetNextRect();
	}
}

inline void CDC::_BitmapInRgn(CRgn* pRgn, VD_BITMAP * pBitmap, int line, int offs_x, int offs_y)
{
	VD_PCRECT pcRect = pRgn->GetFirstRect();
	while (pcRect)
	{
		_BitmapInRect(pcRect, pBitmap, line, offs_x, offs_y);
		pcRect = pRgn->GetNextRect();
	}
}

void CDC::_FillRect(VD_PCRECT pRect)
{
	int y;

	int offset = pRect->left * m_BPP;
	int width = pRect->right - pRect->left;

	for (y = pRect->top; y < pRect->bottom; y++)
	{
		m_pHLineProc(m_pBuffer + m_Pitch * y + offset, width, m_BrushColor);
	}
}

void CDC::_LineInRect(VD_PCRECT pRect, int x1, int y1, int x2, int y2)
{
	int dx;
	int dy;
	int x;
	int y;
	int p;
	int const1;
	int const2;
	int inc;
	int flag=1;//法向
	int tmp;
	
	if (!LineClipper(pRect, &x1, &y1, &x2, &y2) && !PtInRect(pRect, x1, y1) && !PtInRect(pRect, x2, y2))
	{
		return;
	}

	//	trace("line (%3d,%3d) to (%3d,%3d)\n",x1,y1,x2,y2);
	dx = x2 - x1;
	dy = y2 - y1;
	
	if (dx == 0){
		inc = (dy > 0)?1:-1;
		m_pPixelProc(m_pBuffer + m_Pitch * y1 + x1 * m_BPP, m_PenColor);
		while (y1 != y2){
			y1 += inc;
			m_pPixelProc(m_pBuffer + m_Pitch * y1 + x1 * m_BPP, m_PenColor);
		}
		return;
	}
	if (dy == 0){
		inc = (dx > 0)?1:-1;
		x = MIN(x1, x2);
		dx = ABS(dx) + 1;
		m_pHLineProc(m_pBuffer + m_Pitch * y1 + x * m_BPP, dx, m_PenColor);
		return;
	}
	if (dx*dy>0)
		inc =1;
	else
		inc = -1;
	if (ABS(dx)>ABS(dy)){
		if (dx<0){
			tmp = x1;
			x1 = x2;
			x2 = tmp;
			tmp = y1;
			y1 = y2;
			y2 = tmp;
			dx = -dx;
			flag = -1;
		}
		dy = ABS(dy);
		p = 2*dy-dx;
		const1 = 2*dy;
		const2 = 2*(dy-dx);
		x = x1;
		y = y1;
		m_pPixelProc(m_pBuffer + m_Pitch * y + x * m_BPP, m_PenColor);
		while (x<x2){
			x++;
			if (p<0)
				p+=const1;
			else{
				y+=inc;
				p+=const2;
			}
			if (m_PenStyle==VD_PS_DOT&&x%2==0)
				continue;
			m_pPixelProc(m_pBuffer + m_Pitch * y + x * m_BPP, m_PenColor);
		}
	}
	else{
		if (dy<0){
			tmp = x1;
			x1 = x2;
			x2 = tmp;
			tmp = y1;
			y1 = y2;
			y2 = tmp;
			dy = -dy;
			flag = -1;
		}
		dx = ABS(dx);
		p = 2*dx-dy;
		const1 = 2*dx;
		const2 = 2*(dx-dy);
		x = x1;
		y = y1;
		m_pPixelProc(m_pBuffer + m_Pitch * y + x * m_BPP, m_PenColor);
		while (y<y2){
			y++;
			if (p<0)
				p+=const1;
			else{
				x+=inc;
				p+=const2;
			}
			if (m_PenStyle==VD_PS_DOT&&y%2==0)
				continue;
			m_pPixelProc(m_pBuffer + m_Pitch * y + x * m_BPP, m_PenColor);
		}
	} 
}

void CDC::StringToBitmap(uchar* m_pTxtBuffer, int w, int h, VD_PCSTR str, int len)
{
	ushort code;//字符unicode
	int n;//字符计数
	int cw;//字符宽度
	int cl;//字符字节数
	int space = 5;
	int x = space, ox, ox_start, ox_end;//点阵偏移
	int y = 0, oy, oy_start, oy_end;//点阵偏移
	int yoffset;//y坐标
	uchar* p;//点阵缓冲
	uchar raster[128];
 	//add langzi 2009-11-28	钩边功能
 	//uchar * m_pTxtBuffer = NULL; //贴图指针
	int temp_x = 0, temp_y = 0; //临时起始坐标
	int Rect_w = w; //贴图宽度
	int Rect_h = h; //贴图长度
	
	if (NULL == m_pTxtBuffer)
	{
		return;
	}

	oy_start = 0;

	if (m_FontSize.h >= h)
	{
		oy_end = h;
	}
	else
	{
		oy_end = m_FontSize.h;
	}

	memset(m_pTxtBuffer, 0, m_BPP * w * oy_end);

	if (BM_NORMAL == m_BkMode && space <  w)
	{
		for (oy = 0; oy < oy_end; oy++)
		{
			yoffset = m_BPP * w * oy;
			for (ox = 0; ox < space; ox++)
			{
				m_pPixelProc(m_pTxtBuffer + yoffset + ox * m_BPP, m_BkColor);
				*((ushort*)(m_pTxtBuffer + yoffset + ox * m_BPP)) |= 0x8000;
			}
		}
	}

	for (n = 0; n < len; n += cl, x += cw)
	{
		code = m_locals->GetCharCode(&str[n], &cl);
		if(cl == 0)
		{
			break;
		}

		if (m_FontStyle == FS_SMALL)
		{
			cw = m_locals->GetCharRaster(code, raster, FONTSIZE_SMALL);
		}
		else
		{
			cw = m_locals->GetCharRaster(code, raster);
		}

		p = raster + oy_start * (m_FontSize.w + 7) / 8;

		ox_start = 0;
		ox_end = cw;

		if (x + cw >= w)
		{
			if (x >= w)//右边字符被裁减
			{
				break;
			}
			ox_end = w - x;//右边界上的字符精确裁减
		}
		
		//m_FontColor = 0x;
		if (m_BkMode == BM_NORMAL)
		{
			//printf(" 11111111111111 m_BkMode == BM_NORMAL w=%d m_BPP=%d cw=%d oy_start=%d oy_end=%d\n"
			//	, w, m_BPP, cw, oy_start, oy_end);
			for (oy = oy_start; oy < oy_end; oy++)
			{
				yoffset = m_BPP * w * (y + oy);
				for (ox = ox_start; ox < ox_end; ox++)
				{
					if ( * (p + ox / 8) & (128 >> (ox % 8)))
					{
						//printf("*");
						
						m_pPixelProc(m_pTxtBuffer + yoffset + (x + ox) * m_BPP, m_FontColor);
						*((ushort*)(m_pTxtBuffer + yoffset + (x + ox) * m_BPP)) |= 0x8000;
					}
					else
					{
						//printf(" ");
						m_pPixelProc(m_pTxtBuffer + yoffset + (x + ox) * m_BPP, m_BkColor);
						*((ushort*)(m_pTxtBuffer + yoffset + (x + ox) * m_BPP)) |= 0x8000;
					}
				}
				//printf("\n");
				p += (m_FontSize.w + 7) / 8;
			}
		}
		else
		{
			//printf(" 2222222222222   m_BkMode == BM_NORMAL  \n");
			
			w -= 2;
			h -= 2;
		  //add langzi 2009-12-3 增加钩边模式
		  if(m_BkMode == BM_CROCHET)
		  {
			 
			  uchar * p_temp;//点阵临时缓冲 		 
			  p_temp = p;
			  for(oy = oy_start; oy < oy_end; oy++)
			  {
				  yoffset =  Rect_w * (y - temp_y + oy + 1) * m_BPP;
				  for (ox = ox_start; ox < ox_end; ox++)
				  { 					
					 if (*(p + ox / 8) & (128 >> (ox % 8)))
					 {
						m_pPixelProc(m_pTxtBuffer + yoffset - Rect_w * m_BPP + (x - temp_x + ox + 1) * m_BPP, m_BkColor);
					 }
				  }
				  p += (m_FontSize.w + 7) / 8;
			  } 
			  p = p_temp;
			  for(oy = oy_start; oy < oy_end; oy++)
			  {
				  yoffset =  Rect_w * (y - temp_y + oy + 1) * m_BPP;
				  for (ox = ox_start; ox < ox_end; ox++)
				  { 					
					 if (*(p + ox / 8) & (128 >> (ox % 8)))
					 {
						m_pPixelProc(m_pTxtBuffer + yoffset +  Rect_w * m_BPP  + (x - temp_x + ox + 1) * m_BPP, m_BkColor);
					 }
				  }
				  p += (m_FontSize.w + 7) / 8;
			  } 
			  p = p_temp;
			  for(oy = oy_start; oy < oy_end; oy++)
			  {
				  yoffset =  Rect_w * (y - temp_y + oy + 1) * m_BPP;
				  for (ox = ox_start; ox < ox_end; ox++)
				  { 					
					 if (*(p + ox / 8) & (128 >> (ox % 8)))
					 {
						m_pPixelProc(m_pTxtBuffer + yoffset + (x - temp_x + ox) * m_BPP, m_BkColor);
					 }
				  }
				  p += m_FontSize.w / 8;
			  } 
			  p = p_temp;
			  for(oy = oy_start; oy < oy_end; oy++)
			  {
				  yoffset =  Rect_w * (y - temp_y + oy + 1) * m_BPP;
				  for (ox = ox_start; ox < ox_end; ox++)
				  { 					
					 if (*(p + ox / 8) & (128 >> (ox % 8)))
					 {
						m_pPixelProc(m_pTxtBuffer + yoffset + (x - temp_x + ox + 2) * m_BPP, m_BkColor);
					 }
				  }
				   p += (m_FontSize.w + 7) / 8;
			  } 
			  p = p_temp;
			  for(oy = oy_start; oy < oy_end; oy++)
			  {
				  yoffset =  Rect_w * (y - temp_y + oy + 1) * m_BPP;
				  for (ox = ox_start; ox < ox_end; ox++)
				  { 					
					 if (*(p + ox / 8) & (128 >> (ox % 8)))
					 {
						m_pPixelProc(m_pTxtBuffer + yoffset + (x - temp_x + ox + 1) * m_BPP, m_FontColor);
                        *((ushort*)(m_pTxtBuffer + yoffset + (x - temp_x + ox) * m_BPP)) |= 0x8000;
					 }
				  }
				   p += (m_FontSize.w + 7) / 8;
			  } 
			  p = p_temp;
			  p_temp = NULL;
		  }
		  else	//end langzi 2009-12-3
		  {
			 for (oy = oy_start; oy < oy_end; oy++)
			 {
				yoffset = Rect_w * (y + oy) * m_BPP;
				for (ox = ox_start; ox < ox_end; ox++)
				{
					if (*(p + ox / 8) & (128 >> (ox % 8)))
					{
						m_pPixelProc(m_pTxtBuffer + yoffset + (x + ox) * m_BPP, m_FontColor);
					}
				}
				 p += (m_FontSize.w + 7) / 8;
			 }
		  }
		}
	}
	//add lanzi 2009-11-28
	if(m_BkMode == BM_CROCHET)
	{
	   for (oy = 1; oy < Rect_h - 1; oy++)
	   {
		   int yoffset_text_buff = oy * Rect_w * m_BPP;
		   yoffset = Rect_w * (temp_y + oy - 1) * m_BPP;
		   for (ox = 1; ox < Rect_w - 1; ox++)
		   {		
			  for(int number_color = 0; number_color < m_BPP; number_color++)
			  {
				 *(m_pTxtBuffer + yoffset + (temp_x + ox - 1)* m_BPP + number_color) = *(m_pTxtBuffer + yoffset_text_buff  + ox * m_BPP + number_color);
			  }
		   }
		}
	}

	if (BM_NORMAL == m_BkMode && x <  w)
	{
		space = (space < w - x) ? space : w - x;
		for (oy = 0; oy < oy_end; oy++)
		{
			yoffset = m_BPP * w * oy;
			for (ox = x; ox < x + space; ox++)
			{
				m_pPixelProc(m_pTxtBuffer + yoffset + ox * m_BPP, m_BkColor);
				*((ushort*)(m_pTxtBuffer + yoffset + ox * m_BPP)) |= 0x8000;
			}
		}
	}
	//end langzi
}

void CDC::_TextInRect(VD_PCRECT pRect, int x, int y, VD_PCSTR str, int len)
{
	ushort code;//字符unicode
	int n;//字符计数
	int cw = 0;//字符宽度
	int cl;//字符字节数
	int ox, ox_start, ox_end;//点阵偏移
	int oy, oy_start, oy_end;//点阵偏移
	int yoffset;//y坐标
	uchar * p;//点阵缓冲
	uchar raster[128] = {0};
    //add langzi 2009-11-28   钩边功能
    uchar * m_pTxtBuffer = NULL; //贴图指针
	int temp_x = pRect->left, temp_y = pRect->top; //临时起始坐标
	int Rect_w = pRect->right - pRect->left + 2; //贴图宽度
	int Rect_h = pRect->bottom - pRect->top + 2; //贴图长度
	#ifdef DC_DEFLICKER
	int ox_left = 0, ox_right = 0;//点阵偏移
	int oy_top = 0, oy_bottom = 0;//点阵偏移
	#endif

	if(m_BkMode == BM_CROCHET)
	{
	    m_pTxtBuffer = new uchar[Rect_h  * Rect_w  * m_BPP];
	    if(m_pTxtBuffer == NULL) return;
	    memset(m_pTxtBuffer, 0, sizeof(uchar) * Rect_h * Rect_w * m_BPP);
	}
	//end langzi
    
	if (y < pRect->top)
	{
		oy_start = pRect->top - y;
	}
	else
	{
		oy_start = 0;
	}
    
	if (y + m_FontSize.h >= pRect->bottom)
	{
		oy_end = pRect->bottom - y;
	}
	else
	{
		oy_end = m_FontSize.h;
	}
    
	#ifdef DC_DEFLICKER
	ox_left = (x < pRect->left) ? pRect->left : x;
	oy_top = y + oy_start;
	#endif

	for (n = 0; n < len; n += cl, x += cw)
	{
		code = m_locals->GetCharCode(&str[n], &cl);
		if(cl == 0)
		{
			break;
		}

		if (m_FontStyle == FS_SMALL)
		{
			cw = m_locals->GetCharRaster(code, raster, FONTSIZE_SMALL);
		}
		else
		{
			cw = m_locals->GetCharRaster(code, raster);
		}

		p = raster + oy_start * (m_FontSize.w + 7) / 8;

		ox_start = 0;
		ox_end = cw;
		if (x < pRect->left)
		{
			if (x + cw < pRect->left)//左边字符被裁减
			{
				continue;
			}
			ox_start = pRect->left - x;//左边界上的字符精确裁减
		}
		if (x + cw >= pRect->right)
		{
			if (x >= pRect->right)//右边字符被裁减
			{
				break;
			}
			ox_end = pRect->right - x;//右边界上的字符精确裁减
		}

		if (m_BkMode == BM_NORMAL)
		{
			for (oy = oy_start; oy < oy_end; oy++)
			{
				yoffset = m_Pitch * (y + oy);
				for (ox = ox_start; ox < ox_end; ox++)
				{
					if ( * (p + ox / 8) & (128 >> (ox % 8)))
					{
						m_pPixelProc(m_pBuffer + yoffset + (x + ox) * m_BPP, m_FontColor);
					}
					else
					{
						m_pPixelProc(m_pBuffer + yoffset + (x + ox) * m_BPP, m_BkColor);
					}
				}
				 p += (m_FontSize.w + 7) / 8;
			}
		}
		else
		{
		    //add langzi 2009-12-3 增加钩边模式
            if(m_BkMode == BM_CROCHET)
            {             
                uchar * p_temp;//点阵临时缓冲          
			    p_temp = p;
			    for(oy = oy_start; oy < oy_end; oy++)
			    {
				    yoffset =  Rect_w * (y - temp_y + oy + 1) * m_BPP;
				    for (ox = ox_start; ox < ox_end; ox++)
				    {					    
					    if (*(p + ox / 8) & (128 >> (ox % 8)))
					    {
						    m_pPixelProc(m_pTxtBuffer + yoffset - Rect_w * m_BPP + (x - temp_x + ox + 1) * m_BPP, m_BkColor);
					    }
				    }
				    p += (m_FontSize.w + 7) / 8;
			    }	
	            p = p_temp;
			    for(oy = oy_start; oy < oy_end; oy++)
			    {
				    yoffset =  Rect_w * (y - temp_y + oy + 1) * m_BPP;
				    for (ox = ox_start; ox < ox_end; ox++)
				    {					    
					    if (*(p + ox / 8) & (128 >> (ox % 8)))
					    {
						    m_pPixelProc(m_pTxtBuffer + yoffset +  Rect_w * m_BPP  + (x - temp_x + ox + 1) * m_BPP, m_BkColor);
					    }
				    }
				    p += (m_FontSize.w + 7) / 8;
			    }	
	            p = p_temp;
			    for(oy = oy_start; oy < oy_end; oy++)
			    {
				    yoffset =  Rect_w * (y - temp_y + oy + 1) * m_BPP;
				    for (ox = ox_start; ox < ox_end; ox++)
				    {					    
					    if (*(p + ox / 8) & (128 >> (ox % 8)))
					    {
						    m_pPixelProc(m_pTxtBuffer + yoffset + (x - temp_x + ox) * m_BPP, m_BkColor);
					    }
				    }
				    p += (m_FontSize.w + 7) / 8;
			    }	
	            p = p_temp;
			    for(oy = oy_start; oy < oy_end; oy++)
			    {
				    yoffset =  Rect_w * (y - temp_y + oy + 1) * m_BPP;
				    for (ox = ox_start; ox < ox_end; ox++)
				    {					    
					    if (*(p + ox / 8) & (128 >> (ox % 8)))
					    {
						    m_pPixelProc(m_pTxtBuffer + yoffset + (x - temp_x + ox + 2) * m_BPP, m_BkColor);
					    }
				    }
				    p += (m_FontSize.w + 7) / 8;
			    }	
	            p = p_temp;
			    for(oy = oy_start; oy < oy_end; oy++)
			    {
				    yoffset =  Rect_w * (y - temp_y + oy + 1) * m_BPP;
				    for (ox = ox_start; ox < ox_end; ox++)
				    {					    
					    if (*(p + ox / 8) & (128 >> (ox % 8)))
					    {
						    m_pPixelProc(m_pTxtBuffer + yoffset + (x - temp_x + ox + 1) * m_BPP, m_FontColor);
					    }
				    }
				    p += (m_FontSize.w + 7) / 8;
			    }	
	            p = p_temp;
			    p_temp = NULL;
            }
		    else  //end langzi 2009-12-3
		    {
			    for (oy = oy_start; oy < oy_end; oy++)
			    {
				    yoffset = m_Pitch * (y + oy);
				    for (ox = ox_start; ox < ox_end; ox++)
				    {
					    if (*(p + ox / 8) & (128 >> (ox % 8)))
					    {
						    m_pPixelProc(m_pBuffer + yoffset + (x + ox) * m_BPP, m_FontColor);
					    }
				    }
				    p += (m_FontSize.w + 7) / 8;
			    }
		    }
		}
	}
    
	//add lanzi 2009-11-28
	//if条件用于把黑底恢复为之前的透明状态,并勾出字体黑边
	if(m_BkMode == BM_CROCHET)
	{
	   for (oy = 0; oy < Rect_h; oy++)
	   {
	       int yoffset_text_buff = oy * Rect_w * m_BPP;
		   yoffset = m_Pitch * (temp_y + oy - 2);
		   for (ox = 0; ox < Rect_w; ox++)
		   {		
		      for(int number_color = 0; number_color < m_BPP; number_color++)
		      {
		         *(m_pBuffer + yoffset + (temp_x + ox)* m_BPP + number_color) = *(m_pTxtBuffer + yoffset_text_buff  + ox * m_BPP + number_color);
		      }
		   }
	    }
	   delete [] m_pTxtBuffer;
	}
    
	m_pTxtBuffer = NULL;
	
	#ifdef DC_DEFLICKER
	ox_right = x + cw;
	oy_bottom = y + oy_end;
	
	VD_RECT rect;
	rect.left = ox_left;
	rect.top = oy_top;
	rect.right = ox_right;
	rect.bottom = oy_bottom;
	CDevGraphics::instance()->DeFlicker(&rect);
	#endif
	//end langzi
}

//参数offs_x, offs_y表示图片显示在屏幕上时, 图片的左上角相对于原点的偏移
inline void CDC::_BitmapInRect(VD_PCRECT pRect, VD_BITMAP * pBitmap, int line, int offs_x, int offs_y)
{
	int oy;
	int offset_dst = pRect->top * m_Pitch + pRect->left * m_BPP;
	int offset_src = (pRect->top - offs_y) * line + (pRect->left - offs_x) * m_BPP;
	int width = pRect->right - pRect->left;
	int height = pRect->bottom - pRect->top;

	//位图拷贝
	for (oy = 0; oy < height; oy++)
	{
		m_pBitmapProc(m_pBuffer + oy * m_Pitch + offset_dst,
			width,
			pBitmap->bits +  oy * line + offset_src,
			m_BkMode,
			m_TransColor);
	}
}

inline void CDC::_WideBitmapInRect(VD_PCRECT pRect, VD_BITMAP * pBitmap, int line, int offs_x, int offs_y)
{
	CRect rect(pRect);

	offs_x = rect.left - offs_x;
	offs_y = rect.top - offs_y;
	rect.IntersectRect(&rect, &m_Bound);
	
	CRgn rgn;
	rgn.CreateRectRgn(rect);
	rgn.IntersectRgn(&m_Rgn);
	
	_BitmapInRgn(&rgn, pBitmap, line, offs_x, offs_y);
}

void CDC::WideBitmap(VD_PCRECT pRectDst, VD_BITMAP * pBitmap, VD_PCRECT pRectSrc /* = NULL */)
{
	if (!m_bEnable)
	{
		return;
	}

	
	int off_x[9],off_y[9];
	int i;
	int bkmode = m_BkMode;
	int start_x, start_y;
	int src_w ,src_h, dst_w,dst_h,ww,hh,block_w,block_h;
	CRect max_rect(0,0,pBitmap->width,pBitmap->height);

	if (pRectSrc == NULL)
	{
		pRectSrc = max_rect;
	}

	start_x = pRectSrc->left;
	start_y = pRectSrc->top;

	if (!::PtInRect(CRect(0, 0, pBitmap->width, pBitmap->height), start_x, start_y))
	{
//		trace("CDC::WideBitmap Source offset (%d,%d) not match (0 ~ %d,0 ~ %d).\n",
//			pRectSrc->left, pRectSrc->top, pBitmap->width, pBitmap->height);
		return;
	}

	if (pRectDst == NULL || pBitmap == NULL || pBitmap->bits == NULL)
	{
		return;
	}

	//位图每行数据4字节对齐
	int line = (pBitmap->width * m_BPP + 3) & 0xfffffc;
	
	CRect rect(pRectDst);
	LPtoDP(rect);
	rect.NormalizeRect();

	//判断是否有相交区域
	VD_RECT update_rect;
	m_Rgn.GetBoundRect(&update_rect);
	if (!::IntersectRect(&update_rect, &update_rect, rect))
	{
		return;
	}

	src_w = pRectSrc->right - pRectSrc->left;
	src_h = pRectSrc->bottom - pRectSrc->top;
	dst_w = pRectDst->right - pRectDst->left;
	dst_h = pRectDst->bottom - pRectDst->top;

	if (src_w <= 0 || src_h <= 0 || dst_w <= 0 ||dst_h <= 0)
	{
		return;
	}

	if (src_w == dst_w)
	{
		block_w = dst_w;//宽度匹配
	}
	else if (src_w > dst_w)
	{
		block_w = dst_w / 2;//宽度压缩
	}
	else
	{
		block_w = src_w / 4;//宽度拉伸
	}
	block_w = MAX(block_w, 1);//防止出错
	if (src_h == dst_h)
	{
		block_h = dst_h;//高度匹配
	}
	else if (src_h >= dst_h)
	{
		block_h = dst_h / 2;//高度压缩
	}
	else
	{
		block_h = src_h / 4;//高度拉伸
	}
	block_h = MAX(block_h, 1);//防止出错

	for (i = 0; i < 9; i++)
	{
		if (i % 3 == 0)
		{
			off_x[i] = pRectSrc->left;
		}
		else if (i % 3 == 1)
		{
			off_x[i] = pRectSrc->left + block_w;
		}
		else
		{
			off_x[i] = pRectSrc->right - block_w;
		}

		if (i / 3 == 0)
		{
			off_y[i] = pRectSrc->top;
		}
		else if (i / 3 == 1)
		{
			off_y[i] = pRectSrc->top + block_h;
		}
		else
		{
			off_y[i] = pRectSrc->bottom - block_h;
		}
	}

	if (m_nType != DC_MEMORY)
	{
		m_pCursor->UpdateCursor(FALSE, &update_rect);
	}

	_WideBitmapInRect(CRect(rect.left,rect.top,rect.left+block_w,rect.top+block_h),pBitmap,line,off_x[0],off_y[0]); //0

	m_BkMode = BM_NORMAL;
	for (ww=rect.left+block_w;ww<rect.right-block_w;ww+=block_w){
		_WideBitmapInRect(CRect(ww,rect.top,MIN((rect.right-block_w),(ww+block_w)),rect.top+block_h),pBitmap, line,off_x[1],off_y[1]); //1		
	}
	m_BkMode = bkmode;

	_WideBitmapInRect(CRect(rect.right-block_w,rect.top,rect.right,rect.top+block_h),pBitmap, line,off_x[2],off_y[2]); //2

	m_BkMode = BM_NORMAL;
	for (hh=rect.top+block_h;hh<rect.bottom-block_h;hh+=block_h)
	{
		_WideBitmapInRect(CRect(rect.left,hh,rect.left+block_w,MIN((rect.bottom-block_h),(hh+block_h))),pBitmap, line,off_x[3],off_y[3]); //3
		for (ww=rect.left+block_w;ww<rect.right-block_w;ww+=block_w)
		{
			_WideBitmapInRect(CRect(ww,hh,MIN((rect.right-block_w),(ww+block_w)),MIN((rect.bottom-block_h),(hh+block_h))),pBitmap, line,off_x[4],off_y[4]); //4		
		}
		if (rect.right-block_w != rect.left)
		{
			_WideBitmapInRect(CRect(rect.right-block_w,hh,rect.right,MIN((rect.bottom-block_h),(hh+block_h))),pBitmap, line,off_x[5],off_y[5]); //5
		}
	}
	m_BkMode = bkmode;

	if (rect.bottom-block_h != rect.top || rect.right - block_w != rect.left)
	{
		_WideBitmapInRect(CRect(rect.left,rect.bottom-block_h,rect.left+block_w,rect.bottom),pBitmap, line,off_x[6],off_y[6]); //6
	}
	
	if (rect.bottom-block_h != rect.top)
	{
		m_BkMode = BM_NORMAL;
		for (ww=rect.left+block_w;ww<rect.right-block_w;ww+=block_w){
			_WideBitmapInRect(CRect(ww,rect.bottom-block_h,MIN((rect.right-block_w),(ww+block_w)),rect.bottom),pBitmap, line,off_x[7],off_y[7]); //7		
		}
		m_BkMode = bkmode;
	}

	if (rect.bottom-block_h != rect.top || rect.right-block_w != rect.left)
	{
		_WideBitmapInRect(CRect(rect.right-block_w,rect.bottom-block_h,rect.right,rect.bottom),pBitmap, line,off_x[8],off_y[8]); //8
	}

	if (m_nType != DC_MEMORY)
	{
		m_pCursor->UpdateCursor(TRUE, &update_rect);
	}
	
	//csp modify
	SetModify(TRUE);
}

void CDC::FillRect( VD_PCRECT pRect , VD_COLORREF color)
{
	if (!m_bEnable)
	{
		return;
	}

	if (pRect == NULL)
	{
		return;
	}


	int width = pRect->right - pRect->left;
	if (width < 0)
	{
		width = -width;
	}

	int height = pRect->bottom - pRect->top;
	if (height < 0)
	{
		height = -height;
	}
	//位图每行数据4字节对齐
	int line = (width * m_BPP + 3) & 0xfffffc;

	CRect rect(pRect);
	LPtoDP(rect);
	rect.NormalizeRect();

//	printf("FillRect 111 \n");

	//判断是否有相交区域
	VD_RECT update_rect;
	m_Rgn.GetBoundRect(&update_rect);
	if (!::IntersectRect(&update_rect, &update_rect, rect))
	{
		return;
	}

//	printf("FillRect 222 \n");

	if (m_nType != DC_MEMORY)
	{
		m_pCursor->UpdateCursor(FALSE, &update_rect);
	}

	int start_x = rect.left - 0;
	int start_y = rect.top - 0;

//	printf("FillRect 333\n");
	rect.IntersectRect(&rect, &m_Bound);
	rect.IntersectRect(&rect, CRect(start_x, start_y, start_x + width, start_y + height));

//	printf("FillRect 444 \n");
	CRgn rgn;
	rgn.CreateRectRgn(rect);
	rgn.IntersectRgn(&m_Rgn);

	int oy;
	int offset_dst = rect.top * m_Pitch + rect.left * m_BPP;
	int offset_src = (rect.top - start_y) * line + (rect.left - start_x) * m_BPP;

//	printf("FillRect 555 0x%x\n", color);
	//位图拷贝
	ushort bits[1024];
	int size = (sizeof(bits)/sizeof(ushort));
	for (int i = 0; i< size; i++)
	{
		bits[i] = (ushort)MakeColor(m_CS,color);
		//printf("FillRect 0x%x \n", bits[i]);
	}

//	printf("FillRect 666\n");
	for (oy = 0; oy < height; oy++)
	{
		//printf("fillrect: oy=%d, pitch=%d line=%d, src=%d width=%d\n", oy, m_Pitch, line, offset_src, width);

		m_pBitmapProc(m_pBuffer + oy * m_Pitch + offset_dst,
			width,
			(unsigned char*)bits + offset_src,
			m_BkMode,
			m_TransColor);
	}

//	printf("FillRect 777\n");
	if (m_nType != DC_MEMORY)
	{
		m_pCursor->UpdateCursor(TRUE, &update_rect);
	}
//	printf("FillRect end\n");
	
	//csp modify
	SetModify(TRUE);
}

void CDC::SetLanguage(int index)
{
	m_locals->SetLanguage(index);
}

//csp modify
VD_BOOL CDC::GetModify()
{
	return m_bModify;
}

void CDC::SetModify(VD_BOOL flag)
{
	m_bModify = flag;
}

