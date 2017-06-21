

#ifndef __DEV_GRAPHICS_H__
#define __DEV_GRAPHICS_H__

#include "config-x.h"
#include "APIs/Garphics.h"
#include "MultiTask/Guard.h"
#include "GUI/GDI/Region.h"

class CDevGraphics {
public:
	CDevGraphics(int index);
	virtual ~CDevGraphics();
	
public:
	void GetSurface(GRAPHICS_SURFACE *pSurface);
	
	int GetAge();
	
	/// 设置分辨率
	/// \param [in] w x分辨率
	/// \param [in] h y分辨率
	/// \return 设置是否成功
	bool SetResolution(int w, int h);
	
	void SetAlpha(int alpha, int delta = 0);
	void SetKeyColor(VD_COLORREF dwKeyLow, VD_COLORREF dwKeyHigh);
	VD_COLORREF GetKeyColor();
	int GetColorFormat();
	
	void GetRect(VD_PRECT pRect);
	VD_BOOL SetVideoStandard(uint dwStandard);
	VD_BOOL UpdateSurface();
	
	/// 区域抗抖动
	VD_BOOL DeFlicker(VD_PRECT pRect, int level = 1);
	static CDevGraphics* instance(int index = 0);
	
	VD_BOOL DestroyGraphics(int index = 2);
	
	void ShowJpegPic(char* pFileName, unsigned char nShow);
	
private:
	
	VD_COLORREF m_KeyColor;
	
	CMutex m_Mutex;
	
	int m_nAge;
	
	CRect m_Rect;
	
	CSize m_currentResolution;
	
	GRAPHICS_SURFACE m_Surface;
	
	int	m_index;
	//int m_index2;
	
	static CDevGraphics* _instance[N_VIDEO_OUT]; 
};

#endif// __DEV_GRAPHICS_H__

