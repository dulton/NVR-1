
#include "GUI/GDI/Cursor.h"
#include "System/File.h"
#include "math.h"
#include "GUI/GDI/Draw.h"
#include "Devices/DevMouse.h"
#include <string.h>

//csp modify
#include "GUI/GDI/DC.h"

GRAPHICS_SURFACE CCursor::m_Surface;
CURSOR_SHAPE CCursor::cursor_shape[CR_ALL];

//csp modify
//CCursor::CCursor(int index)
CCursor::CCursor(int index) : m_Mutex(MUTEX_RECURSIVE)
{
	//现在还不能确定是否有鼠标外设，只是初始化鼠标光标
	m_bCursorShow = TRUE;
	m_bCursorEnabled = FALSE;
	bEnable = TRUE;
	
	CDevGraphics::instance(index)->GetSurface(&m_Surface);
	
	//init cursor position
	cursor_index = CR_NORMAL;
	cursor_x = m_Surface.width/2;
	cursor_y = m_Surface.height/2;
	SetCursorPos(cursor_x, cursor_y);
	m_Rect.SetRect(0, 0, m_Surface.width, m_Surface.height);
	
	//csp modify 20130327
	//m_pSavedBits = new uchar[32*32*2];
	//m_pCursorBits = new uchar[32*32*2];
	m_pSavedBits = new uchar[32*32*2 + 4*32*2];
	m_pCursorBits = new uchar[32*32*2 + 4*32*2];
}

CCursor::~CCursor()
{
	if(m_pSavedBits)
	{
		delete []m_pSavedBits;
		m_pSavedBits = NULL;
	}
	
	if(m_pCursorBits)
	{
		delete []m_pCursorBits;
		m_pCursorBits = NULL;
	}
}

CCursor* CCursor::instance(int index)
{
	//csp modify 20130323
	if(index != 0)
	{
		printf("#####################################################warning:CCursor::instance index error:0x%08x\n",index);
	}
	
	static CCursor* _instance[N_GUI_CORE];
	if(NULL == _instance[index])
	{
		_instance[index] = new CCursor(index);
	}
	return _instance[index];
}

VD_BOOL CCursor::VD_LoadCursor(VD_PCSTR name, int index)
{
	int w, h, ow, oh;
	uchar* crxor,* crand, *buf;
	CFile file;
	CURSOR_SHAPE* pCr = &cursor_shape[index];
	pCr->valid = 0;
	
	//printf("VD_LoadCursor-1\n");
	
	buf = file.Load(name);
	
	if(!buf ||index < 0 || index >= CR_ALL){
		printf("CCursor::MakeCursor() parameter error!!!!!!!!!\n");
		file.UnLoad();
		return FALSE;
	}
	
	SINGLEMONOCURSOR* pcursor = (SINGLEMONOCURSOR*)buf;
	
	if(pcursor->cdcount != 1 || pcursor->crheader.biBitCount != 1){
		printf("CCursor::MakeCursor() Not single monochrome cursor file, cann't use. %d %d\n", pcursor->cdcount, pcursor->crheader.biBitCount);
		file.UnLoad();
		return FALSE;
	}
	if(pcursor->bwidth != 32 || pcursor->bheight != 32){
		printf("CCursor::MakeCursor() Not 32*32 cursor file, cann't use. %d %d\n", pcursor->bwidth, pcursor->bheight);
		file.UnLoad();
		return FALSE;
	}

	//printf("VD_LoadCursor-2\n");

	//根据光标文件的数据初始化光标结构
	pCr->valid = 1;
	pCr->color1 = 0x00080808;//*(uint *)&pcursor->crcolors[0];
	pCr->color2 = 0x00ffffff;//*(uint *)&pcursor->crcolors[1];
	pCr->xhotspot = pcursor->wxhotspot;
	pCr->yhotspot = pcursor->wyhotspot;
	pCr->andbits = new uchar[32*32*2];
	pCr->xorbits = new uchar[32*32*2];

	crxor = buf + sizeof(SINGLEMONOCURSOR);
	crand = crxor + pcursor->bwidth*pcursor->bheight/8;

	//printf("VD_LoadCursor-3\n");

	//产生阴影
	int shadow_buffer1[32][32];
	int shadow_buffer2[32][32];
	for(h = 0; h < 32; h++)
	{
		for(w = 0; w < 32; w++)
		{
			if(h - 1 > 0 && w - 4 > 0 && !(crand[((31-(h-1))*32+(w-4))/8]&(128>>((w-4)%8))))
			{
				shadow_buffer1[h][w] = 192;//最深阴影
			}
			else
			{
				shadow_buffer1[h][w] = 256;//最浅阴影
			}
		}
	}

	//printf("VD_LoadCursor-4...\n");

	for(h = 0; h < 32; h++)
	{
		for(w = 0; w < 32; w++)
		{
			shadow_buffer2[h][w] = 0;
			for(oh = -1; oh < 2; oh++)
			{
				for(ow = -1; ow < 2; ow++)
				{
					if(h + oh >= 0 && h + oh < 32 && w + ow >= 0 && w + ow < 32)
					{
						shadow_buffer2[h][w] += shadow_buffer1[h + oh][w + ow] * shadow_buffer1[h + oh][w + ow];
					}
					else
					{
						shadow_buffer2[h][w] += shadow_buffer1[h][w] * shadow_buffer1[h][w];
					}
				}
			}
			//printf("VD_LoadCursor-4.1\n");
			shadow_buffer2[h][w] = (int)sqrt((float)shadow_buffer2[h][w]) / 3;
			//printf("VD_LoadCursor-4.2\n");
		}
	}

	//printf("VD_LoadCursor-5\n");

	//位图扫描线格式：高位在左，低位在右
	for(h = 0; h < 32; h++){
		for(w = 0; w < 32; w++){
			if(crand[((31-h)*32+w)/8]&(128>>(w%8)))
			{
				*(ushort*)&pCr->andbits[(h*32+w)*2] = 
					MakeColor(m_Surface.format, shadow_buffer2[h][w], shadow_buffer2[h][w], shadow_buffer2[h][w]);
				//*(WORD*)&pCr->andbits[(h*32+w)*2] = 0xffff;
				if(crxor[((31-h)*32+w)/8]&(128>>(w%8)))
				{
					*(ushort*)&pCr->xorbits[(h*32+w)*2] = 0xffff;
				}
				else
				{
					*(ushort*)&pCr->xorbits[(h*32+w)*2] = 0x0000;
				}
			}
			else
			{
				*(ushort*)&pCr->andbits[(h*32+w)*2] = 0x0000;
				if(crxor[((31-h)*32+w)/8]&(128>>(w%8)))
				{
					*(ushort*)&pCr->xorbits[(h*32+w)*2] = MakeColor(m_Surface.format, pCr->color2);
				}
				else
				{
					*(ushort*)&pCr->xorbits[(h*32+w)*2] = MakeColor(m_Surface.format, pCr->color1);
				}
			}
			//trace("%04x %04x\n", *(WORD*)&pCr->andbits[(h*32+w)*2], *(WORD*)&pCr->xorbits[(h*32+w)*2]);
		}
	}

	//printf("VD_LoadCursor-6\n");

	file.UnLoad();

	//printf("VD_LoadCursor-7\n");

	return TRUE;
}

int CCursor::GetCursor()
{
	CGuard guard(m_Mutex);

	return cursor_index;
}

void CCursor::SetCursor(int index)
{
	CGuard guard(m_Mutex);

	//如果鼠标设备不可用，将鼠标光标设为空
	if(index < 0 || index >= CR_ALL || index == cursor_index){
		return;
	}
	old_cursor_rect.SetRect(
		cursor_x - cursor_shape[cursor_index].xhotspot,
		cursor_y - cursor_shape[cursor_index].yhotspot,
		cursor_x - cursor_shape[cursor_index].xhotspot + 32,
		cursor_y - cursor_shape[cursor_index].yhotspot + 32
		);
	old_cursor_rect_valid.IntersectRect(old_cursor_rect, m_Rect);
	new_cursor_rect.SetRect(
		cursor_x - cursor_shape[index].xhotspot,
		cursor_y - cursor_shape[index].yhotspot,
		cursor_x - cursor_shape[index].xhotspot + 32,
		cursor_y - cursor_shape[index].yhotspot + 32
		);
	new_cursor_rect_valid.IntersectRect(new_cursor_rect, m_Rect);
	if(m_bCursorShow&&m_bCursorEnabled)
	{
		HideCursor();
	}
	cursor_index = index;
	if(m_bCursorShow&&m_bCursorEnabled)
	{
		ShowCursor();
	}
}

void CCursor::SetCursorPos (int x, int y, VD_BOOL update)
{
	//if(!bEnable)
	//{
		//printf("bEnable = %d \n",bEnable);
		//return;
	//}	
	
	CGuard guard(m_Mutex);

	cursor_pos_changed = TRUE;
	if(x < 0){
		x = 0;
	}else if(x >= m_Surface.width){
		x = m_Surface.width - 1;
	}
	if(y < 0){
		y = 0;
	}else if(y > m_Surface.height){
		y = m_Surface.height - 1;
	}
	old_cursor_rect.SetRect(
		cursor_x - cursor_shape[cursor_index].xhotspot,
		cursor_y - cursor_shape[cursor_index].yhotspot,
		cursor_x - cursor_shape[cursor_index].xhotspot + 32,
		cursor_y - cursor_shape[cursor_index].yhotspot + 32
		);
	//printf("old rect %d %d %d %d\n", old_cursor_rect.left, old_cursor_rect.top, old_cursor_rect.right, old_cursor_rect.bottom);
	old_cursor_rect_valid.IntersectRect(old_cursor_rect, m_Rect);
	cursor_x = x;
	cursor_y = y;
	new_cursor_rect.SetRect(
		cursor_x - cursor_shape[cursor_index].xhotspot,
		cursor_y - cursor_shape[cursor_index].yhotspot,
		cursor_x - cursor_shape[cursor_index].xhotspot + 32,
		cursor_y - cursor_shape[cursor_index].yhotspot + 32
		);
	//printf("new rect %d %d %d %d\n", new_cursor_rect.left, new_cursor_rect.top, new_cursor_rect.right, new_cursor_rect.bottom);
	new_cursor_rect_valid.IntersectRect(new_cursor_rect, m_Rect);
	if(m_bCursorShow&&m_bCursorEnabled)
	{
		HideCursor();
		ShowCursor();
	}
#ifdef WIN32
	GraphicsSetCursorPos(x, y);//for windows
#endif
	//GraphicsSetAlpha(200, 0);
	//*(WORD*)(m_Surface.mem + x * 2+ y * m_Surface.pitch) = 0x0000ff00;
	if (update)
	{
		CDevMouse::instance()->SetMousePoint(x, y);
	}
}

void CCursor::GetCursorPos(VD_PPOINT pPoint)
{
	CGuard guard(m_Mutex);

	pPoint->x = cursor_x;
	pPoint->y = cursor_y;
}

void CCursor::ShowCursor(VD_BOOL bShow)
{
	CGuard guard(m_Mutex);

	if(m_bCursorShow == bShow)
	{
		return;
	}

	m_bCursorShow = bShow;
	if(m_bCursorEnabled)
	{
		if(bShow)
		{
			new_cursor_rect_valid.IntersectRect(new_cursor_rect, m_Rect);
			old_cursor_rect_valid.SetRectEmpty();
			ShowCursor();
		}
		else
		{
			old_cursor_rect = new_cursor_rect;
			old_cursor_rect_valid.IntersectRect(old_cursor_rect, m_Rect);
			new_cursor_rect_valid.SetRectEmpty();
			HideCursor();
		}
	}
}

void CCursor::EnableCursor(VD_BOOL bFlag)
{
	CGuard guard(m_Mutex);

	if(m_bCursorEnabled == bFlag)
	{
		return;
	}

	m_bCursorEnabled = bFlag;

	if(m_bCursorShow)
	{
		if(bFlag)
		{
			new_cursor_rect_valid.IntersectRect(new_cursor_rect, m_Rect);
			old_cursor_rect_valid.SetRectEmpty();
			ShowCursor();
		}
		else
		{
			old_cursor_rect = new_cursor_rect;
			old_cursor_rect_valid.IntersectRect(old_cursor_rect, m_Rect);
			new_cursor_rect_valid.SetRectEmpty();
			HideCursor();
		}
	}
}

void CCursor::ShowCursor()
{
	if(!cursor_shape[cursor_index].valid || !m_Surface.mem)
	{
		return;
	}

	VD_RECT rects[4], common_rect;
	int i, rectn;

	//保存光标区域原来的位图
	if(::IntersectRect(&common_rect, new_cursor_rect_valid, old_cursor_rect_valid))
	{
		
		int oy;
		::OffsetRect(&common_rect, -old_cursor_rect.left, -old_cursor_rect.top);
		for(oy = common_rect.top; oy < common_rect.bottom; oy++)
		{
			memcpy(m_pCursorBits + (32 * (oy - common_rect.top)) * 2,
				m_pSavedBits + (32 * oy + common_rect.left) * 2,
				(common_rect.right - common_rect.left) * 2);
		}
		::OffsetRect(&common_rect, old_cursor_rect.left - new_cursor_rect.left, old_cursor_rect.top - new_cursor_rect.top);
		for(oy = common_rect.top; oy < common_rect.bottom; oy++)
		{
			memcpy(m_pSavedBits + (32 * oy + common_rect.left) * 2,
				m_pCursorBits + (32 * (oy - common_rect.top)) * 2,
				(common_rect.right - common_rect.left) * 2);
		}
	};


	rectn = ::SubtractRect(rects, new_cursor_rect_valid, old_cursor_rect_valid);
	for(i = 0; i < rectn; i++)
	{
		::OffsetRect(&rects[i], -new_cursor_rect.left, -new_cursor_rect.top);
		CursorBitBlt(m_Surface.mem + new_cursor_rect.left * 2 + new_cursor_rect.top * m_Surface.pitch,
			m_pSavedBits,
			&rects[i],
			TRUE);
	}

	//将原位图拷贝到光标位图，并和光标形状做叠加
	memcpy(m_pCursorBits, m_pSavedBits, 32 * 32 * 2);

	uint* andbits = (uint*) cursor_shape[cursor_index].andbits;
	uint* xorbits = (uint*) cursor_shape[cursor_index].xorbits;
	uint* dst = (uint*)m_pCursorBits;
	uint old_dst;

	//yg modify 20140819 周成明修改
	for (i = 0; i < 512; i++) {
		if(xorbits[i])
			dst[i] = xorbits[i];
	}

	/*
		//old_dst = dst[i];
		//dst[i] &= andbits[i];
		//dst[i] ^= xorbits[i];
		//if(xorbits[i])
			//dst[i] = xorbits[i];

		
		if(VD_HIWORD(old_dst) != 0 && VD_HIWORD(dst[i]) == 0)//去掉锁结色
		{
			dst[i] |= 0x00010000;
		}
		if(VD_LOWORD(old_dst) != 0 && VD_LOWORD(dst[i]) == 0)
		{
			dst[i] |= 0x00000001;
		}
		
	}*/
	
	#if 0//defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	{
		int a = 0, b = 31, c = 0, d = 31, x = 31;
		for(i = 0; i < 32; i++)
		{
			if((m_pCursorBits[i*2]==0x00)||(m_pCursorBits[i*2+1]==0x00))
			{
				break;
			}
		}
		a = i;
		if(a == 32)
		{
			b = 32;
		}
		else
		{
			for(i = 31; i >= a; i--)
			{
				if((m_pCursorBits[i*2]==0x00)||(m_pCursorBits[i*2+1]==0x00))
				{
					break;
				}
			}
			b = i;
		}
		for(i = 0; i < 32; i++)
		{
			if((m_pCursorBits[i*2+31*32*2]==0x00) || (m_pCursorBits[i*2+31*32*2+1]==0x00))
			{
				break;
			}
		}
		c = i;
		if(c == 32)
		{
			d = 32;
		}
		else
		{
			for(i = 31; i >= c; i--)
			{
				if((m_pCursorBits[i*2+31*32*2]==0x00) || (m_pCursorBits[i*2+31*32*2+1]==0x00))
				{
					break;
				}
			}
			d = i;
		}
		
		//csp modify 20130323
		//if(b == 32 || d == 32)
		//{
		//	//printf("a[%d],b[%d],c[%d],d[%d],x[%d]\n",a,b,c,d,x);
		//	a = c = 0;
		//	b = d = 31;
		//	x = 31;
		//	goto Replace_Background;
		//}
		
		if((a != c) || (b != d))
		{
			if(a != c)
			{
				if(a > c)
				{
					for(i = 0; i < 32; i++)
					{
						if((m_pCursorBits[32*2*i+c*2]==0x00) || (m_pCursorBits[32*2*i+c*2+1]==0x00))
						{
							break;
						}
					}
				}
				else
				{
					for(i = 0; i < 32; i++)
					{
						if((m_pCursorBits[32*2*i+a*2]!=0x00) && (m_pCursorBits[32*2*i+a*2+1]!=0x00))
						{
							break;
						}
					}
				}
			}
			else
			{
				if(b > d)
				{
					for(i = 0; i < 32; i++)
					{
						if((m_pCursorBits[32*2*i+b*2]!=0x00) && (m_pCursorBits[32*2*i+b*2+1]!=0x00))
						{
							break;
						}
					}
				}
				else
				{
					for(i = 0; i < 32; i++)
					{
						if((m_pCursorBits[32*2*i+d*2]==0x00) || (m_pCursorBits[32*2*i+d*2+1]==0x00))
						{
							break;
						}
					}
				}
			}
			x = i - 1;
		}
		
Replace_Background://csp modify 20130323
		//printf("a[%d],b[%d],c[%d],d[%d],x[%d]\n",a,b,c,d,x);
		
		for(i = 0; i <= x; i++)
		{
			for(int j = a; j <= b; j++)
			{
				if(0x00 == m_pCursorBits[i * 64 + j * 2])
				{
					m_pCursorBits[i * 64 + j * 2] = 0x80;
				}
				if(0x00 == m_pCursorBits[i * 64 + j * 2 + 1])
				{
					m_pCursorBits[i * 64 + j * 2 + 1] = 0x80;
				}
			}
		}
		for(i = x + 1; i < 32; i++)
		{
			for(int j = c; j <= d; j++)
			{
				if(0x00 == m_pCursorBits[i * 64 + j * 2])
				{
					m_pCursorBits[i * 64 + j * 2] = 0x80;
				}
				if(0x00 == m_pCursorBits[i * 64 + j * 2 + 1])
				{
					m_pCursorBits[i * 64 + j * 2 + 1] = 0x80;
				}
			}
		}
	}
	#endif
	
	//显示叠加后的光标位图
	common_rect = new_cursor_rect_valid;
	
	//printf("cursor: l=%d r=%d \n", common_rect.left, common_rect.right);

	::OffsetRect(&common_rect, - new_cursor_rect.left, - new_cursor_rect.top);
	CursorBitBlt(m_Surface.mem + new_cursor_rect.left * 2 + new_cursor_rect.top * m_Surface.pitch,
		m_pCursorBits,
		&common_rect,
		FALSE);

	//GraphicsSyncVgaToCvbs();//csp modify
}


void CCursor::HideCursor()
{
	if(!cursor_shape[cursor_index].valid || !m_Surface.mem)
	{
		return;
	}
	
	VD_RECT rects[4];
	int i, rectn;
	
	rectn = ::SubtractRect(rects, old_cursor_rect_valid, new_cursor_rect_valid);
	
	//恢复光标区域原来的位图
	for(i = 0; i < rectn; i++)
	{
		::OffsetRect(&rects[i], -old_cursor_rect.left, -old_cursor_rect.top);
		CursorBitBlt(m_Surface.mem + old_cursor_rect.left * 2 + old_cursor_rect.top * m_Surface.pitch,
			m_pSavedBits,
			&rects[i],
			FALSE);
	}
	
	//GraphicsSyncVgaToCvbs();//csp modify
}

void CCursor::CursorBitBlt(uchar *screen, uchar *mem, VD_PCRECT pRect, VD_BOOL bSave)
{
	int oy;
	ushort* dstrow;
	ushort* srcrow;

	for(oy = pRect->top; oy < pRect->bottom; oy++)
	{
		if(bSave)
		{
			dstrow = (ushort*)(mem + (pRect->left + oy * 32) * 2);
			srcrow = (ushort*)(screen + oy * m_Surface.pitch + pRect->left * 2);
		}
		else
		{
			dstrow = (ushort*)(screen + oy * m_Surface.pitch + pRect->left * 2);
			srcrow = (ushort*)(mem + (pRect->left + oy * 32) * 2);
			
			//csp modify
			CDC::SetModify(TRUE);
		}
		
		memcpy(dstrow, srcrow, (pRect->right - pRect->left) * 2);
	}
}

//for rectangle
void CCursor::UpdateCursor(VD_BOOL bShow, VD_PCRECT pcRect)
{
	if(!bShow)
	{
		m_Mutex.Enter();
	}
	
	if(IntersectRect(NULL, new_cursor_rect, pcRect))
	{
		if(bShow)
		{
			if(m_bCursorShow&&m_bCursorEnabled)
			{
				new_cursor_rect_valid.IntersectRect(new_cursor_rect, pcRect);
				old_cursor_rect_valid.SetRectEmpty();
				ShowCursor();
			}
		}
		else
		{
			if(m_bCursorShow&&m_bCursorEnabled)
			{
				old_cursor_rect = new_cursor_rect;
				old_cursor_rect_valid.IntersectRect(old_cursor_rect, pcRect);
				new_cursor_rect_valid.SetRectEmpty();
				HideCursor();
			}
		}
	}
	
	if(bShow)
	{
		m_Mutex.Leave();
	}
}

//for line
void CCursor::UpdateCursor(VD_BOOL bShow, int x1, int y1, int x2, int y2)
{
	if(!bShow)
	{
		m_Mutex.Enter();
	}

	CRect line_rect;

	if(LineClipper(new_cursor_rect, &x1, &y1, &x2, &y2)
		|| PtInRect(new_cursor_rect, x1, y1)
		|| PtInRect(new_cursor_rect, x2, y2))
	{
		line_rect.SetRect(x1, y1, x2, y2);
		line_rect.NormalizeRect();
		line_rect.right++;
		line_rect.bottom++;
		if(bShow)
		{
			if(m_bCursorShow&&m_bCursorEnabled)
			{
				new_cursor_rect_valid.IntersectRect(new_cursor_rect, line_rect);
				old_cursor_rect_valid.SetRectEmpty();
				ShowCursor();
			}
		}
		else
		{
			if(m_bCursorShow&&m_bCursorEnabled)
			{
				old_cursor_rect = new_cursor_rect;
				old_cursor_rect_valid.IntersectRect(old_cursor_rect, line_rect);
				new_cursor_rect_valid.SetRectEmpty();
				HideCursor();
			}
		}
	}
	if(bShow)
	{
		m_Mutex.Leave();
	}
}

/*
void CCursor::Enable(VD_BOOL bFlag)
{
	bEnable = bFlag;
}
*/

