
#include "GUI/GDI/Draw.h"
#include <string.h>
#include "Devices/DevGraphics.h"

extern "C" {
	void * pciTransfer(void *pDest, void *pSrc, int nBufSize);
}

/*Optimized memcpy/memset use a multiple register transfer instruction which
can move up to 32 bytes in a single operation (8 registers of 4 bytes each).
This is very fast but is uninterruptible and hogs the bus until it finishes.
Many such operations in quick sequence (to accomplish a larger transfer) mayme
lead to audio DMA data starvation.*/
#ifdef __linux__
//#define memcpy pciTransfer
#endif

static void pixel_set_1(uchar *buf, VD_COLORDEV color)
{
    *(buf) = (uchar) color;
}

static void pixel_set_2(uchar *buf, VD_COLORDEV color)
{
    *(ushort *) (buf) = (ushort) color;
}

static void pixel_set_3(uchar *buf, VD_COLORDEV color)
{
    /* no this code in new version
    *(WORD*) (buf) = (WORD) color;
    *(buf + 2) = (BYTE) (color >> 16);
    */
}

static void pixel_set_4(uchar *buf, VD_COLORDEV color)
{
    //*(uint *) buf = (uint) color;   // no this code in new version
}

static void pixel_and_1(uchar *buf, VD_COLORDEV color)
{
    *buf &= (uchar) color;
}

static void pixel_and_2(uchar *buf, VD_COLORDEV color)
{
    *(ushort *) buf &= (ushort) color;
}

static void pixel_and_3(uchar *buf, VD_COLORDEV color)
{/* no this code in new version
    *(WORD*) buf &= (WORD) color;
    *(buf + 2) &= (BYTE) (color >> 16);*/
}

static void pixel_and_4(uchar *buf, VD_COLORDEV color)
{
    //*(uint *) buf &= (uint) color; // no this code in new version
}

static void pixel_or_1(uchar *buf, VD_COLORDEV color)
{
    *buf |= (uchar) color;
}

static void pixel_or_2(uchar *buf, VD_COLORDEV color)
{
    *(ushort *) buf |= (ushort) color;
}

static void pixel_or_3(uchar *buf, VD_COLORDEV color)
{/* no this code in new version
    *(WORD*) buf |= (WORD) color;
    *(buf + 2) |= (BYTE) (color >> 16);*/
}

static void pixel_or_4(uchar *buf, VD_COLORDEV color)
{
    //*(uint *) buf |= (uint) color;// no this code in new version
}

static void pixel_xor_1(uchar *buf, VD_COLORDEV color)
{
    *buf ^= (uchar) color;
}

static void pixel_xor_2(uchar *buf, VD_COLORDEV color)
{
    *(ushort *) buf ^= (ushort) color;
}

static void pixel_xor_3(uchar *buf, VD_COLORDEV color)
{/* no this code in new version
    *(WORD*) buf ^= (WORD) color;
    *(buf + 2) ^= (BYTE) (color >> 16);*/
}

static void pixel_xor_4(uchar *buf, VD_COLORDEV color)
{
    //*(uint *) buf ^= (uint) color;//no this code in new version
}

static void hline_set_1(uchar *buf, int w, VD_COLORDEV color)
{
    memset (buf, color, w);
}

static void hline_set_2(uchar *buf, int w, VD_COLORDEV color)
{
	#if defined(CHIP_HISI3531) || defined(CHIP_HISI3521)
	if(color == 0)
	{
		//printf("*");
        memset(buf, 0x80, w*2);
        return;
    }
    #endif
    
	/*if(color == 0)
	{
		memset (buf, color, w*2);
	}
	else*/
	{
		if(!w)
		{
			return;
		}
		
		uint multi_color = VD_MAKELONG(color, color);
        uint* row = NULL;
		int ww = w;
		
		//首尾处理
		if((int)buf%4)
		{
			*(ushort*)buf = color;
			w--;
			row = (uint*)(buf+2);
		}
		else
		{
			row = (uint*)buf;
		}
		if((int)(buf+ww*2)%4)
		{
			*(ushort*)(buf+ww*2-2) = color;
			w--;
		}

		//4字节赋值
		while (w > 0) {
            *row = multi_color;
            row ++;
            w-=2;
        }
    }
}

static void hline_set_3(uchar *buf, int w, VD_COLORDEV color)
{/* no this code in new version
    if (color == 0) {
        memset (buf, color, w*3);
    }
    else {
        BYTE* row = (BYTE*)buf;

        while (w > 0) {
            *(WORD*) row = (WORD) color;
            *(row + 2) = (BYTE) (color >> 16);
            row += 3;
            w--;
        }
    }*/
}

static void hline_set_4(uchar *buf, int w, VD_COLORDEV color)
{/* no this code in new version
    uint* row = (uint*)buf;

    if (color == 0) {
        memset (buf, color, w*3);
    }
    else {
        while (w > 0) {
            *row = color;
            row++;
            w--;
        }
    }*/
}

static void hline_and_1(uchar *buf, int w, VD_COLORDEV color)
{
    uchar* row = buf;

    while (w > 0) {
        *row &= (uchar) color;
        row++;
        w--;
    }
}

static void hline_and_2(uchar *buf, int w, VD_COLORDEV color)
{
    ushort* row = (ushort*)buf;

    while (w > 0) {
        *row &= (ushort) color;
        row++;
        w--;
    }
}

static void hline_and_3(uchar *buf, int w, VD_COLORDEV color)
{/* no this code in new version
    BYTE* row = (BYTE*)buf;

    while (w > 0) {
        *(WORD*) row &= (WORD) color;
        *(row + 2) &= (BYTE) (color >> 16);
        row += 3;
        w--;
    }*/
}

static void hline_and_4(uchar *buf, int w, VD_COLORDEV color)
{/* no this code in new version
    uint* row = (uint*)buf;

    while (w > 0) {
        *row &= color;
        row++;
        w--;
    }*/
}

static void hline_or_1(uchar *buf, int w, VD_COLORDEV color)
{
    uchar* row = buf;

    while (w > 0) {
        *row |= (uchar) color;
        row++;
        w--;
    }
}

static void hline_or_2(uchar *buf, int w, VD_COLORDEV color)
{
    ushort* row = (ushort*)buf;

    while (w > 0) {
        *row |= (ushort) color;
        row++;
        w--;
    }
}

static void hline_or_3(uchar *buf, int w, VD_COLORDEV color)
{/* no this code in new version
    BYTE* row = (BYTE*)buf;

    while (w > 0) {
        *(WORD*) row |= (WORD) color;
        *(row + 2) |= (BYTE) (color >> 16);
        row+=3;
        w--;
    }*/
}

static void hline_or_4(uchar *buf, int w, VD_COLORDEV color)
{/* no this code in new version
    uint* row = (uint*)buf;

    while (w > 0) {
        *row |= color;
        row++;
        w--;
    }*/
}

static void hline_xor_1(uchar *buf, int w, VD_COLORDEV color)
{
    uchar* row = buf;

    while (w > 0) {
        *row ^= (uchar) color;
        row++;
        w--;
    }
}

static void hline_xor_2(uchar *buf, int w, VD_COLORDEV color)
{
    ushort* row = (ushort*)buf;

    while (w > 0) {
        *row ^= (ushort) color;
        row++;
        w--;
    }
}

static void hline_xor_3(uchar *buf, int w, VD_COLORDEV color)
{/* no this code in new version
    BYTE* row = (BYTE*)buf;

    while (w > 0) {
        *(WORD*) row ^= (WORD) color;
        *(row + 2) ^= (BYTE) (color >> 16);
        row += 3;
        w--;
    }*/
}

static void hline_xor_4(uchar *buf, int w, VD_COLORDEV color)
{/* no this code in new version
    uint* row = (uint*)buf;

    while (w > 0) {
        *row ^= color;
        row++;
        w--;
    }*/
}

static void bitmap_set_1(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{
    if (mode != BM_TRANSPARENT) {
		memcpy(buf, src, w);
    }
    else {
        uchar* row = buf;

        if (mode == BM_TRANSPARENT) {
            while (w > 0) {
                if (*src != color)
                    *row = *src;
                row++;
                src--;
                w--;
            }
        }
        else while (w > 0) {
            *row = *src;
            row++;
            src++;
            w--;
        }
    }
}

static void bitmap_set_2(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{
  
        ushort* dstrow = (ushort*)buf;
        ushort* srcrow = (ushort*)src;
		
        if (mode == BM_TRANSPARENT) {
            while (w > 0) {
                if (*srcrow != color)
                    *dstrow = *srcrow;
                dstrow++;
                srcrow++;
                w--;
            }
        }
        else while (w > 0) {
            *dstrow = *srcrow;
            dstrow++;
            srcrow++;
            w--;
        }

}

static void bitmap_set_3(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{/* no this code in new version
    if (mode != BM_TRANSPARENT) {
		memcpy(buf, src, w*3);
    }
    else {
        BYTE* row = (BYTE*)buf;

        if (mode == BM_TRANSPARENT) {
            while (w > 0) {
                if (((* (uint*)row) & 0x00FFFFFF) != color) {
                    *row = *src;
                    *(row + 1) = *(src + 1);
                    *(row + 2) = *(src + 2);
                }
                row += 3;
                src += 3;
                w--;
            }
        }
        else while (w > 0) {
            *row = *src;
            *(row + 1) = *(src + 1);
            *(row + 2) = *(src + 2);
            row += 3;
            src += 3;
            w--;
        }
    }*/
}

static void bitmap_set_4(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{/* no this code in new version
    uint* dstrow = (uint*)buf;
    uint* srcrow = (uint*)src;

    if (mode != BM_TRANSPARENT) {
		memcpy(buf, src, w*4);
    }
    else {
        if (mode == BM_TRANSPARENT) {
            while (w > 0) {
                if (*srcrow != color)
                    *dstrow = *srcrow;
                dstrow++;
                srcrow++;
                w--;
            }
        }
        else while (w > 0) {
            *dstrow = *srcrow;
            dstrow++;
            srcrow++;
            w--;
        }
    }*/
}

static void bitmap_and_1(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{
    uchar* row = buf;

    if (mode == BM_TRANSPARENT) {
        while (w > 0) {
            if (*src != color)
                *row &= *src;
            row++;
            src++;
            w--;
        }
    }
    else while (w > 0) {
        *row &= *src;
        row++;
        src++;
        w--;
    }
}

static void bitmap_and_2(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{
    ushort* dstrow = (ushort*)buf;
    ushort* srcrow = (ushort*)src;

    if (mode == BM_TRANSPARENT) {
        while (w > 0) {
            if (*srcrow != color)
                *dstrow &= *srcrow;
            dstrow++;
            srcrow++;
            w--;
        }
    }
    else while (w > 0) {
        *dstrow &= *srcrow;
        dstrow++;
        srcrow++;
        w--;
    }
}

static void bitmap_and_3(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{/* no this code in new version
    BYTE* row = (BYTE*)buf;

    if (mode == BM_TRANSPARENT) {
        while (w > 0) {
            if (((* (uint*)row) & 0x00FFFFFF) != color) {
               *row &= *src;
               *(row + 1) &= *(src + 1);
               *(row + 2) &= *(src + 2);
            }
            row += 3;
            src += 3;
            w--;
        }
    }
    else while (w > 0) {
        *row &= *src;
        *(row + 1) &= *(src + 1);
        *(row + 2) &= *(src + 2);
        row += 3;
        src += 3;
        w--;
    }*/
}

static void bitmap_and_4(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{/* no this code in new version
    uint* dstrow = (uint*)buf;
    uint* srcrow = (uint*)src;

    if (mode == BM_TRANSPARENT) {
        while (w > 0) {
            if (*srcrow != color)
                *dstrow &= *srcrow;
            dstrow++;
            srcrow++;
            w--;
        }
    }
    else while (w > 0) {
        *dstrow &= *srcrow;
        dstrow++;
        srcrow++;
        w--;
    }*/
}

static void bitmap_or_1(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{
    uchar* row = buf;

    if (mode == BM_TRANSPARENT) {
        while (w > 0) {
            if (*src != color)
                *row |= *src;
            row++;
            src++;
            w--;
        }
    }
    else while (w > 0) {
        *row |= *src;
        row++;
        src++;
        w--;
    }
}

static void bitmap_or_2(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{
    ushort* dstrow = (ushort*)buf;
    ushort* srcrow = (ushort*)src;

    if (mode == BM_TRANSPARENT) {
        while (w > 0) {
            if (*srcrow != color)
                *dstrow |= *srcrow;
            dstrow++;
            srcrow++;
            w--;
        }
    }
    else while (w > 0) {
        *dstrow |= *srcrow;
        dstrow++;
        srcrow++;
        w--;
    }
}

static void bitmap_or_3(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{/* no this code in new version
    BYTE* row = (BYTE*)buf;

    if (mode == BM_TRANSPARENT) {
        while (w > 0) {
            if (((* (uint*)row) & 0x00FFFFFF) != color) {
                *row |= *src;
                *(row + 1) |= *(src + 1);
                *(row + 2) |= *(src + 2);
            }
            row += 3;
            src += 3;
            w--;
        }
    }
    else while (w > 0) {
        *row |= *src;
        *(row + 1) |= *(src + 1);
        *(row + 2) |= *(src + 2);
        row += 3;
        src += 3;
        w--;
    }*/
}

static void bitmap_or_4(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{/* no this code in new version
    uint* dstrow = (uint*)buf;
    uint* srcrow = (uint*)src;

    if (mode == BM_TRANSPARENT) {
        while (w > 0) {
            if (*srcrow != color)
                *dstrow |= *srcrow;
            dstrow++;
            srcrow++;
            w--;
        }
    }
    while (w > 0) {
        *dstrow |= *srcrow;
        dstrow++;
        srcrow++;
        w--;
    }*/
}

static void bitmap_xor_1(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{
    uchar* row = buf;

    if (mode == BM_TRANSPARENT) {
        while (w > 0) {
            if (*src != color)
                *row ^= *src;
            row++;
            src++;
            w--;
        }
    }
    else while (w > 0) {
        *row ^= *src;
        row++;
        src++;
        w--;
    }
}

static void bitmap_xor_2(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{
    ushort* dstrow = (ushort*)buf;
    ushort* srcrow = (ushort*)src;

    if (mode == BM_TRANSPARENT) {
        while (w > 0) {
            if (*srcrow != color)
                *dstrow ^= *srcrow;
            dstrow++;
            srcrow++;
            w--;
        }
    }
    else while (w > 0) {
        *dstrow ^= *srcrow;
        dstrow++;
        srcrow++;
        w--;
    }
}

static void bitmap_xor_3(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{/* no this code in new version
    BYTE* row = (BYTE*)buf;

    if (mode == BM_TRANSPARENT) {
        while (w > 0) {
            if (((* (uint*)row) & 0x00FFFFFF) != color) {
                *row ^= *src;
                *(row + 1) ^= *(src + 1);
                *(row + 2) ^= *(src + 2);
            }
            row += 3;
            src += 3;
            w--;
        }
    }
    while (w > 0) {
        *row ^= *src;
        *(row + 1) ^= *(src + 1);
        *(row + 2) ^= *(src + 2);
        row += 3;
        src += 3;
        w--;
    }*/
}

static void bitmap_xor_4(uchar *buf, int w, uchar *src, int mode, VD_COLORDEV color)
{/* no this code in new version
    uint* dstrow = (uint*)buf;
    uint* srcrow = (uint*)src;

    if (mode == BM_TRANSPARENT) {
        while (w > 0) {
            if (*srcrow != color)
                *dstrow ^= *srcrow;
            dstrow++;
            srcrow++;
            w--;
        }
    }
    while (w > 0) {
        *dstrow ^= *srcrow;
        dstrow++;
        srcrow++;
        w--;
    }*/
}

#define  N_ROP 4
#define  N_bpp 4

static PIXELPROC pixel_procs [N_ROP][N_bpp] =
{
    {pixel_set_1, pixel_set_2, pixel_set_3, pixel_set_4},
    {pixel_and_1, pixel_and_2, pixel_and_3, pixel_and_4},
    {pixel_or_1,  pixel_or_2,  pixel_or_3,  pixel_or_4},
    {pixel_xor_1, pixel_xor_2, pixel_xor_3, pixel_xor_4}
};

static HLINEPROC hline_procs [N_ROP][N_bpp] =
{
    {hline_set_1, hline_set_2, hline_set_3, hline_set_4},
    {hline_and_1, hline_and_2, hline_and_3, hline_and_4},
    {hline_or_1,  hline_or_2,  hline_or_3,  hline_or_4},
    {hline_xor_1, hline_xor_2, hline_xor_3, hline_xor_4}
};

static BITMAPPROC bitmap_procs [N_ROP][N_bpp] =
{
    {bitmap_set_1, bitmap_set_2, bitmap_set_3, bitmap_set_4},
    {bitmap_and_1, bitmap_and_2, bitmap_and_3, bitmap_and_4},
    {bitmap_or_1,  bitmap_or_2,  bitmap_or_3,  bitmap_or_4},
    {bitmap_xor_1, bitmap_xor_2, bitmap_xor_3, bitmap_xor_4}
};

PIXELPROC SelectPixelProc(int rop, int bpp)
{
	return pixel_procs[rop][bpp];
}

HLINEPROC SelectHLineProc(int rop, int bpp)
{
	return hline_procs[rop][bpp];
}

BITMAPPROC SelectBitmapProc(int rop, int bpp)
{
	return bitmap_procs[rop][bpp];
}

//system color
VD_COLORREF SystemColor[COLOR_ALL];

void SetSysColor(int index, VD_COLORREF color)
{
	if(index >= 0 || index < COLOR_ALL)
	{
		SystemColor[index] = color;
	}
}

VD_COLORREF VD_GetSysColor(int index)
{
	if(index >= 0 || index < COLOR_ALL)
	{
		return SystemColor[index];
	}
	return VD_RGB(0,255,0);
}


VD_COLORDEV MakeColor(int format, int r, int g, int b)
{
	r = MIN(255, r);
	r = MAX(0, r);
	g = MIN(255, g);
	g = MAX(0, g);
	b = MIN(255, b);
	b = MAX(0, b);
	switch(format) {
	case GRAPHICS_CF_RGB555:
		return (VD_COLORREF)(((r<<7)&0x7C00)+((g<<2)&0x03E0)+((b>>3)&0x001F));
	case GRAPHICS_CF_RGB565:
		return (VD_COLORREF)(((r<<8)&0xF800)+((g<<3)&0x07E0)+((b>>3)&0x001F));
	default:
		return 0x0000ff00;
	}
}

VD_COLORDEV MakeColor(int format, VD_COLORREF color)
{
	return MakeColor(format, VD_GetRValue(color), VD_GetGValue(color), VD_GetBValue(color));
}

VD_COLORREF ParseColor(int format, VD_COLORDEV color)
{
	int r, g, b;

	switch(format) {
	case GRAPHICS_CF_RGB555:
		r = (color & 0x7C00) >> 7;
		g = (color & 0x03E0) >> 2;
		b = (color & 0x001F) << 3;
		break;
	case GRAPHICS_CF_RGB565:
		r = (color & 0xF800) >> 8;
		g = (color & 0x07E0) >> 3;
		b = (color & 0x001F) << 3;
		break;
	default:
		r = 0;
		g = 0xff;
		b = 0;
		break;
	}
	return VD_RGB(r, g, b);
}

VD_COLORREF ModulateColor(int format, VD_COLORREF c, int r, int g, int b)
{
	r += VD_GetRValue(c);
	g += VD_GetGValue(c);
	b += VD_GetBValue(c);
	r = MIN(255, r);
	g = MIN(255, g);
	b = MIN(255, b);
	switch(format){//去掉锁结色
	case GRAPHICS_CF_RGB555:
		r = MAX(8, r);
		g = MAX(8, g);
		b = MAX(8, b);
		break;
	case GRAPHICS_CF_RGB565:
		r = MAX(8, r);
		g = MAX(4, g);
		b = MAX(8, b);
		break;
	default:
		r = MAX(1, r);
		g = MAX(1, g);
		b = MAX(1, b);
	}
	return VD_RGB(r, g, b);
}
