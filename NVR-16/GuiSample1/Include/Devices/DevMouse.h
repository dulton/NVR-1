

#ifndef __DEV_MOUSE_H__
#define __DEV_MOUSE_H__

#include "APIs/Mouse.h"
#include "MultiTask/Thread.h"
#include "System/Object.h"
#include "System/Signals.h"



typedef TSignal3<uint, uint, uint>::SigProc SIG_DEV_MOUSE_INPUT;

typedef struct _CALIBRATE_MATRIX // 64位保证不溢出
{
	int64    An,     /* A = An/Divider */
		Bn,     /* B = Bn/Divider */
		Cn,     /* C = Cn/Divider */
		Dn,     /* D = Dn/Divider */
		En,     /* E = En/Divider */
		Fn,     /* F = Fn/Divider */
		Divider ;
} CALIBRATE_MATRIX ;

class CDevMouse : public CThread
{
public:
	CDevMouse();
	virtual ~CDevMouse();

private:
	void OnMouseData(void *pdat, int len);

public:

	int SetDoubleClickInterval(uint dwInterval);
	VD_BOOL AttachInput(CObject * pObj, SIG_DEV_MOUSE_INPUT pProc);
	VD_BOOL DetachInput(CObject * pObj, SIG_DEV_MOUSE_INPUT pProc);
	void SetRect(VD_PCRECT pRect);
	void GetOriginalPoint(VD_PPOINT pPoint);
	VD_BOOL SetCalibratePoints(VD_PCPOINT pPointDisplay, VD_PCPOINT pPointScreen);
	VD_BOOL HasFound();
	VD_BOOL HasFoundTS();
	void SetMousePoint(int x, int y);
	virtual void ThreadProc();
	static CDevMouse* instance(void); 

	void ClearSignal();
	//void RunThread();//csp modify 20121222

protected:
private:
	TSignal3<uint, uint, uint> m_sigData;
	VD_POINT m_Point;
	VD_POINT m_PointOriginal;
	char m_cKey;
	uint m_dwInterval;
	uint m_dwLeftCnt;
	uint m_dwRightCnt;
	uint m_dwMiddleCnt;
	VD_POINT m_PointClk;		//上次点击鼠标时的坐标
	VD_RECT m_Rect;
	CALIBRATE_MATRIX m_Matrix;	// 校准矩阵
	VD_BOOL m_bFound;
	VD_BOOL m_bFoundTS;
	static CDevMouse* _instance; 
};

#endif// __DEV_MOUSE_H__
