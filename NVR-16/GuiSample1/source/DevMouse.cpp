#include "APIs/System.h"
#include "Devices/DevMouse.h"
#include "GUI/Pages/BizData.h"

#if 1//csp modify 20130101
#define MOUSE_DBLCLK_INTERNAL 400	//鼠标双击有效时间
#define MOUSE_DBLCLK_AREA 8			//鼠标双击有效区域, 半径0之内
#define TOUCHSCREEN_DBLCLK_AREA 225 //触摸屏双击有效区域, 半径15之内
#else
#define MOUSE_DBLCLK_INTERNAL 350	//鼠标双击有效时间
#define MOUSE_DBLCLK_AREA 0			//鼠标双击有效区域, 半径0之内
#define TOUCHSCREEN_DBLCLK_AREA 225 //触摸屏双击有效区域, 半径15之内
#endif

CDevMouse* CDevMouse::_instance = NULL;

CDevMouse* CDevMouse::instance(void)
{
	if(NULL == _instance)
	{
		_instance = new CDevMouse();
	}
	return _instance;
}

void CDevMouse::ThreadProc()
{
	MOUSE_DATA data;

	//清空结构, 结构大小和库里面可能不一致.
	memset(&data, 0, sizeof(MOUSE_DATA));
	int ret = 0;

/************************************************************************
	
************************************************************************/	
	//printf("hehehehehehehehhehehe************\n");

	while(m_bLoop) 
	{	
		//printf("mouse\n");
		ret = MouseGetData(&data);
		if(-1 == ret)
		{
			//printf("MouseGetData error\n");
			//WaitForSem();
			OnMouseData(&data, 0);
			SystemSleep(1000);
			
			//ReleaseSem();
		}
		else if(-2 == ret)
		{
			//
		}
		else
		{
			//printf("CDevMouse::ThreadProc 2, data: 0x%x\n", data.key);
			//WaitForSem();
			
			OnMouseData(&data, sizeof(MOUSE_DATA));
			//ReleaseSem();
		}
	}
}

#if 0//csp modify 20121222
void CDevMouse::RunThread()
{
	m_bLoop = TRUE;
	ThreadProc();
}
#endif

#define CW_SEM
sem_t MouseSem;//cw_test

CDevMouse::CDevMouse() : CThread("DevMouse", TP_MOUSE), m_sigData(4)
{
#if !defined(SYS_NO_MOUSE)
	m_cKey = 0;
	m_dwLeftCnt = m_dwRightCnt = m_dwMiddleCnt = 0;
	m_dwInterval = MOUSE_DBLCLK_INTERNAL;
	m_Rect.left = 0;
	m_Rect.top = 0;
	m_Rect.right = 800;
	m_Rect.bottom = 600;
	MouseCreate();
	m_bFound = FALSE;
	m_bFoundTS = FALSE;
	CreateThread();
#endif

#ifdef CW_SEM
	if(0 != sem_init(&MouseSem, 0, 0))//cw 初始化加锁，开机画面解锁
	{
		printf("cw****MouseSem error\n");
	}
#endif
}

CDevMouse::~CDevMouse()
{
	//DestroyThread();
	MouseDestory();
}

void CDevMouse::OnMouseData(void *pdat, int len)
{
	/*
	uchar nSystemLocked = 0;
	GetSystemLockStatus(&nSystemLocked);
	if(nSystemLocked)
	{
		return;
	}
	*/
	MOUSE_DATA *mousedata = (MOUSE_DATA*)pdat;
	uint msCount = SystemGetMSCount();
	uint postion;
	VD_POINT point_old; // 处理消息之前的坐标
	int max_area; // 双击有效的最大范围

	//printf("yg OnMouseData data: 0x%x, len: %d, m_bFound: %d\n", mousedata->key, len, m_bFound);
	
	#ifdef CW_SEM
	if(-1==sem_trywait(&MouseSem))
	{
		//printf("yg OnMouseData 1 mouse wait\n");//cw_test
		memset(mousedata, 0, sizeof(MOUSE_DATA));
		return;
	}
	#endif
	//printf("CDevMouse::OnMouseData len=%d,key=%d, x=%d,y=%d \n", len, mousedata->key, mousedata->x, mousedata->y);
	//检查是否连接上
	if(len == 0)
	{
		if(m_bFound)
		{
			m_bFound = FALSE;
			m_sigData(XM_MOUSELOST, 0, 0);
		}
		#ifdef CW_SEM
		sem_post(&MouseSem);//cw_test
		#endif
		return;
	}
	
	if(len && !m_bFound && !(mousedata->key & MOUSE_TOUCHSCREEN))
	{	
		m_bFound = TRUE;
		m_sigData(XM_MOUSEFOUND, 0, 0);
	}

	if(mousedata->key & MOUSE_TOUCHSCREEN)
	{
		m_bFoundTS = TRUE;
	}

	point_old = m_Point;
	if(mousedata->key & MOUSE_TOUCHSCREEN) // 触摸屏绝对坐标
	{
		m_PointOriginal.x = mousedata->ax;
		m_PointOriginal.y = mousedata->ay;
		if( m_Matrix.Divider != 0 )
		{
			m_Point.x = (int)(((m_Matrix.An * mousedata->ax) + 
				(m_Matrix.Bn * mousedata->ay) + 
				m_Matrix.Cn 
				) / m_Matrix.Divider * m_Rect.right / RELATIVE_MAX_X);

			m_Point.y = (int)(((m_Matrix.Dn * mousedata->ax) + 
				(m_Matrix.En * mousedata->ay) + 
				m_Matrix.Fn 
				) / m_Matrix.Divider * m_Rect.bottom / RELATIVE_MAX_Y);
		}

		//trace("%x %x %d %d\n", mousedata->ax, mousedata->ay, m_Point.x, m_Point.y);
		max_area = TOUCHSCREEN_DBLCLK_AREA;
	}
	else // 鼠标相对坐标
	{
		m_Point.x += mousedata->x;
		m_Point.y -= mousedata->y;
		//m_Point.y += mousedata->y;
		max_area = MOUSE_DBLCLK_AREA;
	}

	//约束鼠标移动的区域
	//printf("m_Rect.top: %d, m_Rect.bottom: %d\n", m_Rect.top, m_Rect.bottom);
	//printf("m_Rect.left: %d, m_Rect.right: %d\n", m_Rect.left, m_Rect.right);
	//printf("m_Point.x: %d, m_Point.y: %d\n", m_Point.x, m_Point.y);
	
	if(m_Point.x < m_Rect.left){
		m_Point.x = m_Rect.left;
	}else if(m_Point.x >= m_Rect.right){
		m_Point.x = m_Rect.right - 1;
	}
	if(m_Point.y < m_Rect.top){
		m_Point.y = m_Rect.top;
	}else if(m_Point.y >= m_Rect.bottom){
		m_Point.y = m_Rect.bottom - 1;
	}
	postion = VD_MAKELONG(m_Point.y, m_Point.x);
	//printf("m_Point.x=%d, y=%d mousedata->x=%d, y=%d ax=%d, ay=%d\n", m_Point.x, m_Point.y, mousedata->x, mousedata->y, mousedata->ax, mousedata->ay);
	
	if(point_old.x != m_Point.x || point_old.y != m_Point.y)
	{//moved
		//trace("mouse move: (%d,%d)\n", m_Point.x, m_Point.y);
		//printf("XM_MOUSEMOVE 1\n");
		m_sigData(XM_MOUSEMOVE, mousedata->key, postion);
		
		//双击失效
		if((m_PointClk.x - m_Point.x) * (m_PointClk.x - m_Point.x)
			+ (m_PointClk.y - m_Point.y) * (m_PointClk.y - m_Point.y) > max_area)
		{
			m_dwLeftCnt = 0;
			m_dwRightCnt = 0;
			m_dwMiddleCnt = 0;
		}
	}
	
	#ifndef CW_SEM
	uchar nSystemLocked = 0;
	GetSystemLockStatus(&nSystemLocked);
	if(nSystemLocked)
	{
		return;
	}
	#endif
	
	if(mousedata->z != 0)
	{
		//printf("mouse wheel rotation: (%d)\n", mousedata->z);
		m_sigData(XM_MOUSEWHEEL, VD_MAKELONG(mousedata->key, mousedata->z), postion);
	}
	
	if((m_cKey^mousedata->key)&MOUSE_LBUTTON)
	{
		if(mousedata->key&MOUSE_LBUTTON)
		{
			if(msCount - m_dwLeftCnt < m_dwInterval)
			{
				//printf("XM_LBUTTONDBLCLK\n");
				m_sigData(XM_LBUTTONDBLCLK, mousedata->key, postion);
				m_dwLeftCnt = 0;
			}
			else
			{
				//printf("XM_LBUTTONDOWN\n");
				m_sigData(XM_LBUTTONDOWN, mousedata->key, postion);
				m_dwLeftCnt = msCount;
			}
			m_PointClk = m_Point;
		}
		else
		{
			//printf("XM_LBUTTONUP\n");
			m_sigData(XM_LBUTTONUP, mousedata->key, postion);
		}
	}

	if((m_cKey^mousedata->key)&MOUSE_RBUTTON)
	{
		if(mousedata->key&MOUSE_RBUTTON)
		{
			//why???
			//cj@20110414
			if(m_Point.x >= m_Rect.right-32)
			{
				m_Point.x = m_Rect.right - 32;
			}
			postion = VD_MAKELONG(m_Point.y, m_Point.x);
			//printf("XM_MOUSEMOVE 2\n");
			m_sigData(XM_MOUSEMOVE, mousedata->key, postion);
			//end cj@20110414
			
			if(msCount - m_dwRightCnt < m_dwInterval)
			{
				//printf("XM_RBUTTONDBLCLK\n");
				m_sigData(XM_RBUTTONDBLCLK, mousedata->key, postion);
				m_dwRightCnt = 0;
			}
			else
			{
				//printf("XM_RBUTTONDOWN\n");
				m_sigData(XM_RBUTTONDOWN, mousedata->key, postion);
				m_dwRightCnt = msCount;
			}
			m_PointClk = m_Point;
		}
		else
		{
			//printf("XM_RBUTTONUP\n");
			m_sigData(XM_RBUTTONUP, mousedata->key, postion);
		}
	}

	if((m_cKey^mousedata->key)&MOUSE_MBUTTON)
	{
		if(mousedata->key&MOUSE_MBUTTON)
		{
			if(msCount - m_dwMiddleCnt < m_dwInterval)
			{
				//printf("XM_MBUTTONDBLCLK\n");
				m_sigData(XM_MBUTTONDBLCLK, mousedata->key, postion);
				m_dwMiddleCnt = 0;
			}
			else
			{
				//printf("XM_MBUTTONDOWN\n");
				m_sigData(XM_MBUTTONDOWN, mousedata->key, postion);
				m_dwMiddleCnt = msCount;
			}
			m_PointClk = m_Point;
		}
		else
		{
			//printf("XM_MBUTTONUP\n");
			m_sigData(XM_MBUTTONUP, mousedata->key, postion);
		}
	}
	m_cKey = mousedata->key;
	#ifdef CW_SEM
	sem_post(&MouseSem);//cw_test
	#endif
}

//设置光标最大区域, 即屏幕分辨率
void CDevMouse::SetRect(VD_PCRECT pRect)
{
	if(pRect)
	{
		m_Rect = *pRect;
		m_Point.x = m_Rect.right / 2;
		m_Point.y = m_Rect.bottom / 2;
	}
}

//得到触摸屏按下时的原始坐标
void CDevMouse::GetOriginalPoint(VD_PPOINT pPoint)
{
	*pPoint = m_PointOriginal;
}

//设置触摸屏校准坐标, 3对点一起传入
VD_BOOL CDevMouse::SetCalibratePoints(VD_PCPOINT pPointDisplay, VD_PCPOINT pPointScreen)
{
	m_Matrix.Divider = ((pPointScreen[0].x - pPointScreen[2].x) * (pPointScreen[1].y - pPointScreen[2].y)) - 
		((pPointScreen[1].x - pPointScreen[2].x) * (pPointScreen[0].y - pPointScreen[2].y)) ;

	if( m_Matrix.Divider == 0 )
	{
		return FALSE;
	}
	else
	{
		m_Matrix.An = ((pPointDisplay[0].x - pPointDisplay[2].x) * (pPointScreen[1].y - pPointScreen[2].y)) - 
			((pPointDisplay[1].x - pPointDisplay[2].x) * (pPointScreen[0].y - pPointScreen[2].y)) ;

		m_Matrix.Bn = ((pPointScreen[0].x - pPointScreen[2].x) * (pPointDisplay[1].x - pPointDisplay[2].x)) - 
			((pPointDisplay[0].x - pPointDisplay[2].x) * (pPointScreen[1].x - pPointScreen[2].x)) ;

		m_Matrix.Cn = ((int64)pPointScreen[2].x * pPointDisplay[1].x - pPointScreen[1].x * pPointDisplay[2].x) * pPointScreen[0].y +
			(pPointScreen[0].x * pPointDisplay[2].x - pPointScreen[2].x * pPointDisplay[0].x) * pPointScreen[1].y +
			(pPointScreen[1].x * pPointDisplay[0].x - pPointScreen[0].x * pPointDisplay[1].x) * pPointScreen[2].y ;

		m_Matrix.Dn = ((pPointDisplay[0].y - pPointDisplay[2].y) * (pPointScreen[1].y - pPointScreen[2].y)) - 
			((pPointDisplay[1].y - pPointDisplay[2].y) * (pPointScreen[0].y - pPointScreen[2].y)) ;

		m_Matrix.En = ((pPointScreen[0].x - pPointScreen[2].x) * (pPointDisplay[1].y - pPointDisplay[2].y)) - 
			((pPointDisplay[0].y - pPointDisplay[2].y) * (pPointScreen[1].x - pPointScreen[2].x)) ;

		m_Matrix.Fn = ((int64)pPointScreen[2].x * pPointDisplay[1].y - pPointScreen[1].x * pPointDisplay[2].y) * pPointScreen[0].y +
			(pPointScreen[0].x * pPointDisplay[2].y - pPointScreen[2].x * pPointDisplay[0].y) * pPointScreen[1].y +
			(pPointScreen[1].x * pPointDisplay[0].y - pPointScreen[0].x * pPointDisplay[1].y) * pPointScreen[2].y ;
	}

	return TRUE;
}

int CDevMouse::SetDoubleClickInterval(uint dwInterval)
{
	uint l_dwInterval;

	l_dwInterval = m_dwInterval;
	m_dwInterval = dwInterval;
	return l_dwInterval;
}

VD_BOOL CDevMouse::AttachInput(CObject * pObj, SIG_DEV_MOUSE_INPUT pProc)
{
	if(m_sigData.Attach(pObj, pProc) < 0)
	{
		return FALSE;
	}
	return TRUE;
}

VD_BOOL CDevMouse::DetachInput(CObject * pObj, SIG_DEV_MOUSE_INPUT pProc)
{
	if(m_sigData.Detach(pObj, pProc) < 0)
	{
		return FALSE;
	}
	return TRUE;
}

VD_BOOL CDevMouse::HasFound()
{
	return m_bFound;
}

VD_BOOL CDevMouse::HasFoundTS()
{
	return m_bFoundTS;
}
void CDevMouse::SetMousePoint(int x, int y)
{
	m_Point.x = x;
	m_Point.y = y;
}

void CDevMouse::ClearSignal()
{
	//printf("sig vec size = %d \n",m_sigData.vecSize());
	m_sigData.ClearSignal();
}



