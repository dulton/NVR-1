
#include "GUI/GDI/DC.h"
#include "GUI/GDI/Bitmap.h"
#include "System/File.h"
#include "MultiTask/Mutex.h"

#include <string>
#include <map>

typedef std::map<std::string, VD_BITMAP*> BITMAP_MAP;
static  BITMAP_MAP bitmaps;
static CMutex bmpMutex(MUTEX_RECURSIVE);
static uchar color_table[256 * 2]; //转换后的目标设备颜色表

VD_BITMAP* VD_LoadBitmap(VD_PCSTR name)
{
	CGuard guard(bmpMutex);

	BITMAP_MAP::iterator pi = bitmaps.find(name);
	if(pi != bitmaps.end())
	{
		VD_BITMAP* pBmp = (*pi).second;
		pBmp->reffrence++;

		return pBmp;
	}

	uchar * pBuffer;
	uchar * pBits;
	ushort cur_color;
	int ox,oy,ww,hh,pitch;
	CFile file;
	GRAPHICS_SURFACE surface;

	pBuffer = file.Load(name);
	if(!pBuffer)
	{
		return NULL;
	}

	VD_BITMAPFILEHEADER * pBmpFile = (VD_BITMAPFILEHEADER *)(pBuffer);
	VD_BITMAPINFO * pBmpInfo = (VD_BITMAPINFO *)(pBuffer + sizeof(VD_BITMAPFILEHEADER));

	if(pBmpFile->bfType!=('M'<<8)+'B' || (pBmpInfo->bmiHeader.biBitCount!=16 && pBmpInfo->bmiHeader.biBitCount!=8)){
		printf("Not 8/16 bits map file, can't use. %d. %s\n",pBmpInfo->bmiHeader.biBitCount, name);
		file.UnLoad();
		return NULL;
	}

	CDevGraphics::instance()->GetSurface(&surface);

	VD_BITMAP* pBmp = new VD_BITMAP;

	pBmp->bits = NULL;
	ww = pBmp->width = pBmpInfo->bmiHeader.biWidth;
	hh = pBmp->height = pBmpInfo->bmiHeader.biHeight;
	pitch = (ww * (pBmpInfo->bmiHeader.biBitCount / 8) + 3) & 0xfffffffc; // 4字节对齐
	ww = (ww + 1) & 0xfffffffe; // 宽度取偶数
	pBits = pBuffer + pBmpFile->bfOffBits + (hh - 1) * pitch;

	pBmp->reffrence = 1;
	pBmp->format = surface.format;
	pBmp->bits = new uchar[ww * hh * 2];
	if(!pBmp->bits){
		printf("Alloc buffer for bmp failed\n");
		file.UnLoad();
		return FALSE;
	}
	uint arrayCount;

	//color format convert
	switch(pBmpInfo->bmiHeader.biBitCount) {
	case 8:
		// The number of entries in the array depends on the 
		// values of the biBitCount and biClrUsed members of the BITMAPINFOHEADER structure. 
		
		arrayCount = pBmpInfo->bmiHeader.biClrUsed;
		// 针对有些图片该值为0而加的，根据文件头部大小计算颜色表长度!!!
		// 和MSDN上说的不一样???
		if (arrayCount == 0)
		{
			arrayCount = (pBmpFile->bfOffBits - sizeof(VD_BITMAPINFOHEADER) - sizeof(VD_BITMAPFILEHEADER)) / sizeof(VD_RGBQUAD);
		}
		for(uint i = 0; i < arrayCount; i++)
		{
			((ushort*)color_table)[i] = (ushort)MakeColor(surface.format, pBmpInfo->bmiColors[i].rgbRed, pBmpInfo->bmiColors[i].rgbGreen, pBmpInfo->bmiColors[i].rgbBlue);
		}
		for(oy=0;oy<hh;oy++)
		{
			for(ox=0;ox<ww;ox++)
			{
				*(ushort*)(pBmp->bits + oy*ww*2 + ox*2) = ((ushort*)color_table)[*(pBits - oy*pitch + ox)];
			}
		}
		break;
	case 16:
		if(pBmp->format == GRAPHICS_CF_RGB555){
			for(oy=0;oy<hh;oy++){
				for(ox=0;ox<ww;ox++){
					*(ushort*)(pBmp->bits + oy*ww*2 + ox*2) =
						*(ushort*)(pBits - oy*pitch + ox*2);
					
					#if 0//csp modify
					if(strstr(name,"main_pic.bmp"))
					{
						if(ox == 0)
						{
							printf("main_pic.bmp bitmap color=0x%04x\n",*(ushort*)(pBits - oy*pitch + ox*2));
						}
					}
					#endif
				}
			}
		}else if(pBmp->format == GRAPHICS_CF_RGB565){
			for(oy=0;oy<hh;oy++){
				for(ox=0;ox<ww;ox++){
					cur_color = *(ushort*)(pBits - oy*pitch + ox*2);
					*(ushort*)(pBmp->bits + oy*ww*2 + ox*2) = 
						(ushort)MakeColor(GRAPHICS_CF_RGB565, ((cur_color&0x7C00)>>7),((cur_color&0x03E0)>>2), ((cur_color&0x001F)<<3));
				}
			}
		}
		break;
	case 24:
		break;
	default:
		break;
	}
	
	file.UnLoad();
	bitmaps.insert(BITMAP_MAP::value_type(name, pBmp));
	
	//csp modify
	//ReplaceBitmapBits(pBmp, VD_RGB(0,0,0), VD_RGB(8,8,8));
	ReplaceBitmapBits(pBmp, VD_RGB(0,0,0), VD_RGB(16,16,16));
	
	//过滤掉图标的背景色
	ReplaceBitmapBits(pBmp, VD_RGB(32,32,40), VD_RGB(16,16,16));//csp modify
	
	//过滤掉"H.264 DVR"图标的背景色
	ReplaceBitmapBits(pBmp, VD_RGB(0,8,16), VD_RGB(16,16,16));//csp modify
	ReplaceBitmapBits(pBmp, VD_RGB(0,8,8), VD_RGB(16,16,16));//csp modify
	ReplaceBitmapBits(pBmp, VD_RGB(0,0,8), VD_RGB(16,16,16));//csp modify
	
	return pBmp;
}

void ReleaseBitmap(VD_BITMAP* pBitmap)
{
	CGuard guard(bmpMutex);

	if(pBitmap)
	{
		pBitmap->reffrence--;
		if(pBitmap->reffrence <= 0)
		{
			for(BITMAP_MAP::iterator pi = bitmaps.begin(); pi != bitmaps.end(); pi++)
			{
				if((*pi).second == pBitmap)
				{
					bitmaps.erase(pi);
					break;
				}
			}

			if(pBitmap->bits)
			{
				delete []pBitmap->bits;
			}

			delete pBitmap;
		}
	}
}

void SaveScreenBitmap(VD_PCRECT pcRect, VD_PCSTR filename)
{
	if(!pcRect || !filename)
	{
		printf("SaveBitmap invalid parameter\n");
		return;
	}
	printf("SaveScreenBitmap rect(%d,%d,%d,%d) filename:%s\n", pcRect->left, pcRect->top, pcRect->right, pcRect->bottom, filename);
	GRAPHICS_SURFACE surface;
	VD_COLORREF	keyColor;

	CDevGraphics::instance()->GetSurface(&surface);
	keyColor = CDevGraphics::instance()->GetKeyColor();

	CRect rect(pcRect);
	rect.NormalizeRect();
	rect.IntersectRect(pcRect, CRect(0, 0, surface.width, surface.height));

	ushort null_color = MakeColor(surface.format ,keyColor);
	FILE *fp;
	long count=0;
	int ox, oy;
	int r,g,b;
	int ww = (rect.Width() + 1) / 2 * 2;
	int hh = rect.Height();
	int size = ww*hh*2;
	uchar* bits,* pBits; 
	ushort cur_color;
	VD_BITMAPFILEHEADER bmpHeader;
	VD_BITMAPINFO bmpInfo;

	bmpHeader.bfType = ('M'<<8)+'B';
	bmpHeader.bfSize = size + sizeof(VD_BITMAPFILEHEADER) + sizeof(VD_BITMAPINFOHEADER);
	bmpHeader.bfReserved1 = 0;
	bmpHeader.bfReserved2 = 0;
	bmpHeader.bfOffBits = sizeof(VD_BITMAPFILEHEADER) + sizeof(VD_BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biSize = sizeof(VD_BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = rect.Width();
	bmpInfo.bmiHeader.biHeight = rect.Height();
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = 16;
	bmpInfo.bmiHeader.biCompression = 0;
	bmpInfo.bmiHeader.biSizeImage = size;
	bmpInfo.bmiHeader.biXPelsPerMeter = 0;
	bmpInfo.bmiHeader.biYPelsPerMeter = 0;
	bmpInfo.bmiHeader.biClrUsed = 0;
	bmpInfo.bmiHeader.biClrImportant = 0;

	if( (fp = fopen( filename, "wb")) == NULL ) 
	{
		printf( "Can not create file: %s!\n", filename);
		return;
	}
	count=fwrite(&bmpHeader, 1, sizeof(VD_BITMAPFILEHEADER), fp);
	if (count!= sizeof(VD_BITMAPFILEHEADER))
	{	
		fclose(fp);
		printf( "Write BMP file header failed!\n");
		return;
	}
	count=fwrite(&(bmpInfo.bmiHeader), 1, sizeof(VD_BITMAPINFOHEADER), fp);
	if (count!= sizeof(VD_BITMAPINFOHEADER))
	{
		fclose(fp);
		printf( "Write BMP file info failed!\n");	
		return;
	}
	bits = surface.mem + rect.top * surface.pitch + rect.left * 2;
	pBits = new uchar[size];
	//color format convert
	if(surface.format == GRAPHICS_CF_RGB555){
		for(oy=0;oy<hh;oy++){
			for(ox=0;ox<ww;ox++){
				cur_color = *(ushort*)(bits + oy*surface.pitch + ox*2);
				if(cur_color == null_color){
					*(ushort*)(pBits + (hh-oy-1)*ww*2 + ox*2) = 0xFFFF;
				}else{
					*(ushort*)(pBits + (hh-oy-1)*ww*2 + ox*2) = cur_color;
				}
			}
		}
	}else if(surface.format == GRAPHICS_CF_RGB565){
		for(oy=0;oy<hh;oy++){
			for(ox=0;ox<ww;ox++){
				cur_color = *(ushort*)(bits + oy*surface.pitch + ox*2);
				if(cur_color == null_color){
					*(ushort*)(pBits + (hh-oy-1)*ww*2 + ox*2) = 0xFFFF;
				}else{
					r = (cur_color&0xF800)>>8;
					g = (cur_color&0x07E0)>>3;
					b = (cur_color&0x001F)<<3;
					*(ushort*)(pBits + (hh-oy-1)*ww*2 + ox*2) = 
						(ushort)(((r<<7)&0x7C00)+((g<<2)&0x03E0)+((b>>3)&0x001F));
				}
			}
		}
	}
	count=fwrite(pBits, 1, size, fp);
	if (count!=size)
	{		
		printf( "Write BMP data failed!\n");
	}else{
		printf("BMP file is writen to %s\n", filename);
	}
	delete []pBits;
	fclose(fp);
}

//COLORDEV transcolor = GetSysColor(COLOR_TRANSPARENT);
//COLORDEV windowcolor = GetSysColor(COLOR_WINDOW);

void ReplaceBitmapBits(VD_BITMAP* pBitmap, VD_COLORREF matchColor, VD_COLORREF replaceColor)
{
	if (pBitmap == NULL || pBitmap->bits == NULL){
		return;
	}

	int x, y;

	matchColor = MakeColor(pBitmap->format, matchColor);
	replaceColor = MakeColor(pBitmap->format, replaceColor);
	for (y = 0; y < pBitmap->height; y++)
	{
		for (x = 0; x < pBitmap->width; x++)
		{
			if (*(ushort*)(pBitmap->bits + (y * pBitmap->width + x) * 2) == matchColor)
			{
				*(ushort*)(pBitmap->bits + (y * pBitmap->width + x) * 2) = replaceColor; 
			}
		}
	}
}

void ReplaceBitmapBits(VD_BITMAP* pBitmap, VD_COLORREF matchColor, VD_BITMAP * replaceBitmap, int start_x /* = 0 */, int start_y /* = 0 */)
{
	if (pBitmap == NULL || pBitmap->bits == NULL || replaceBitmap == NULL || replaceBitmap->bits == NULL){
		return;
	}

	int x, y, w, h, wwd, wws;
	matchColor = MakeColor(pBitmap->format, matchColor);

	w = MIN(pBitmap->width, replaceBitmap->width - start_x);
	h = MIN(pBitmap->height, replaceBitmap->height - start_y);

	// 16位位图4字节对齐
	wwd = (pBitmap->width + 1) / 2 * 2;
	wws = (replaceBitmap->width + 1) / 2 * 2;

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			if (*(ushort*)(pBitmap->bits + (y * wwd + x) * 2) == matchColor)
			{
				*(ushort*)(pBitmap->bits + (y * wwd + x) * 2) =
					*(ushort*)(replaceBitmap->bits + ((y + start_y) * wws + x + start_x) * 2); 
			}
		}
	}
}

VD_COLORREF GetBitmapBkColor(VD_BITMAP* pBitmap, VD_COLORREF defaultColor)
{
	if (pBitmap == NULL || pBitmap->bits == NULL)
	{
		return defaultColor;
	}

	return ParseColor(pBitmap->format, *(ushort*)pBitmap->bits);
}


