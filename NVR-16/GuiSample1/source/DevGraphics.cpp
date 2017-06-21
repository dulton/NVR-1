#include "Devices/DevGraphics.h"
//#include "Configs/ConfigLocation.h"

#include "GUI/Pages/BizData.h"
#include "APIs/Video.h"

//extern int g_nVideoOut;

static int m_index2 = 0;

CDevGraphics* CDevGraphics::_instance[] = {NULL, NULL, NULL, };

CDevGraphics* CDevGraphics::instance(int index)
{
	//assert(index < g_nVideoOut);
	
	#if 0//csp modify 20130323
	if(-1 == index)
	{
		_instance[0] = NULL;
		return _instance[0];
	}
	#endif
	
	#if 0//csp modify 20130323
	SBizParaTarget bizTar;
	bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
	SBizSystemPara bizSysPara;
	BizGetPara(&bizTar, &bizSysPara);
	#endif
	
	#if 0//csp modify 20130323
	if((0 == index) && (_instance[0] == NULL))
	{
		m_index2 = 0;
	}
	#endif
	
	#if 0//csp modify 20130323
	if(2 == index)//if((2 == index) || (bizSysPara.nShowFirstMenue))//csp modify
	{
		index = 0;
		m_index2 = 2;
	}
	#endif
	
	#if 1//csp modify 20130323
	if(index != 0)
	{
		printf("#####################################################warning:CDevGraphics::instance error,index=%d\n",index);
		index = 0;
	}
	#endif
	
	if(NULL == _instance[index])
	{
		//if(2 == index) index = 0;
		_instance[index] = new CDevGraphics(index);
	}
	
	return _instance[index];
}

CDevGraphics::CDevGraphics(int index) : m_index(index)
{
	if((0 == index) || (2 == index)) //½çÃæ²Ëµ¥Ê¹ÓÃ
	{
		//printf("CDevGraphics::CDevGraphics\n");
		//fflush(stdout);
		
		//printf("\n\n\n\n");
		//sleep(10);
		
		m_nAge = 0;
		
		m_currentResolution.w = 1024;
		m_currentResolution.h = 768;
		
		SBizParaTarget bizTar;
		bizTar.emBizParaType = EM_BIZ_SYSTEMPARA;
		SBizSystemPara bizSysPara;
		int ret = BizGetPara(&bizTar, &bizSysPara);
		if(ret!=0)
		{
			BIZ_DATA_DBG("GetVideoFormat\n");
		}
		
		//printf("CDevGraphics::SYSTEMPARA\n");
		//fflush(stdout);
		
		//printf("\n\n\n\n");
		//sleep(10);
		
		SBizDvrInfo bizDvrInfo;
		bizTar.emBizParaType = EM_BIZ_DVRINFO;
		ret = BizGetPara(&bizTar, &bizDvrInfo);
		if(ret!=0)
		{
			BIZ_DATA_DBG("Get EM_BIZ_DVRINFO nOutputSync\n");
		}
		
		//printf("CDevGraphics::DVRINFO\n");
		//fflush(stdout);
		
		//printf("\n\n\n\n");
		//sleep(10);
		
		if((EM_BIZ_OUTPUT_VGAORCVBS == bizDvrInfo.nOutputSync) && (0 == m_index2))
		{
			if(EM_BIZ_OUTPUT_CVBS == bizSysPara.nOutput)
			{
				GraphicsSetOutput(EM_GRAPH_OUTPUT_CVBS);
			}
			else
			{
				GraphicsSetOutput(EM_GRAPH_OUTPUT_VGA);
			}
		}
		
		//printf("CDevGraphics::SetOutput\n");
		//fflush(stdout);
		
		//printf("\n\n\n\n");
		//sleep(10);
		
		GraphicsSetChipType(EM_GRAPH_CHIP_HISI3515);		
		if((0 == strcasecmp("R9624T", bizDvrInfo.sproductnumber)) 
			|| (0 == strcasecmp("R9624SL", bizDvrInfo.sproductnumber))
			|| (0 == strcasecmp("R9616S", bizDvrInfo.sproductnumber))
			|| (0 == strcasecmp("R9632S", bizDvrInfo.sproductnumber))
			|| (0 == strcasecmp("R9608S", bizDvrInfo.sproductnumber)))
		{
			GraphicsSetChipType(EM_GRAPH_CHIP_HISI3520);
			printf("¼ì²âµ½Ð¾Æ¬3520\n");
			fflush(stdout);
		}
		else if((0 == strcasecmp("R3104HD", bizDvrInfo.sproductnumber))
				|| (0 == strcasecmp("R3110HDW", bizDvrInfo.sproductnumber))
				|| (0 == strcasecmp("R3106HDW", bizDvrInfo.sproductnumber))
				|| (0 == strcasecmp("R3116", bizDvrInfo.sproductnumber))
				|| (0 == strcasecmp("R3116W", bizDvrInfo.sproductnumber)))
		{
			GraphicsSetChipType(EM_GRAPH_CHIP_HISI3531);
			printf("¼ì²âµ½Ð¾Æ¬3531\n");
			fflush(stdout);
		}
		else if((0 == strcasecmp("R2104", bizDvrInfo.sproductnumber))
				|| (0 == strcasecmp("R2104W", bizDvrInfo.sproductnumber))
				|| (0 == strcasecmp("R2108", bizDvrInfo.sproductnumber))
				|| (0 == strcasecmp("R2108W", bizDvrInfo.sproductnumber))
				|| (0 == strcasecmp("R2116", bizDvrInfo.sproductnumber))
				|| (0 == strcasecmp("R2016", bizDvrInfo.sproductnumber)))
		{
			GraphicsSetChipType(EM_GRAPH_CHIP_HISI3521);
			printf("¼ì²âµ½Ð¾Æ¬3521\n");
			fflush(stdout);
		}
		else if((0 == strcasecmp("NR2116", bizDvrInfo.sproductnumber)))
		{
			GraphicsSetChipType(EM_GRAPH_CHIP_HISI3521);
			printf("¼ì²âµ½Ð¾Æ¬3521\n");
			fflush(stdout);
		}
		else if((0 == strcasecmp("NR3116", bizDvrInfo.sproductnumber)))
		{
			GraphicsSetChipType(EM_GRAPH_CHIP_HISI3531);
			printf("¼ì²âµ½Ð¾Æ¬3531\n");
			fflush(stdout);
		}
		else if((0 == strcasecmp("NR3132", bizDvrInfo.sproductnumber)))
		{
			GraphicsSetChipType(EM_GRAPH_CHIP_HISI3531);
			printf("¼ì²âµ½Ð¾Æ¬3531\n");
			fflush(stdout);
		}
		else if((0 == strcasecmp("NR3124", bizDvrInfo.sproductnumber)))
		{
			GraphicsSetChipType(EM_GRAPH_CHIP_HISI3531);
			printf("¼ì²âµ½Ð¾Æ¬3531\n");
			fflush(stdout);
		}
		else if((0 == strcasecmp("NR1004", bizDvrInfo.sproductnumber)))
		{
			GraphicsSetChipType(EM_GRAPH_CHIP_HISI3521);
			printf("¼ì²âµ½Ð¾Æ¬3521\n");
			fflush(stdout);
		}
		else if((0 == strcasecmp("NR1008", bizDvrInfo.sproductnumber)))
		{
			GraphicsSetChipType(EM_GRAPH_CHIP_HISI3521);
			printf("¼ì²âµ½Ð¾Æ¬3521\n");
			fflush(stdout);
		}
		
		//printf("\n\n\n\n");
		//sleep(10);
		
		//printf("m_index2=%d\n",m_index2);
		if(0 == m_index2)
		{
			//printf("do GraphicsCreate********\n");
			//fflush(stdout);
			
			GraphicsCreate();
		}
		else
		{
			//printf("do GraphicsCreate2********\n");
			//fflush(stdout);
			
			GraphicsCreate2();
		}
		
		int width = 800, height = 600;
		GetVgaResolution(&width, &height);
		
		if(EM_BIZ_NTSC == bizSysPara.nVideoStandard)
		{
			GraphicsSetVstd(VIDEO_STANDARD_NTSC);
		}
		else
		{
			GraphicsSetVstd(VIDEO_STANDARD_PAL);
		}
		
		m_currentResolution.w = width;
		m_currentResolution.h = height;
		GraphicsSetResolution(width, height);
		
		//SetKeyColor(0x00000000, 0x00000000);
		//SetAlpha(255);
		
		//printf("\n\n\n\n");
		//sleep(10);
	}
	else
	{
		
	}
}

CDevGraphics::~CDevGraphics()
{
	//printf("CDevGraphics::~CDevGraphics********\n");
	
	//unload files
#ifdef GRAPHICS_API_VESION_2
	Graphics2Destroy(m_index);
#else
	GraphicsDestory();
#endif
}

void CDevGraphics::SetAlpha(int alpha, int delta /* = 0 */)
{
#ifdef GRAPHICS_API_VESION_2
	Graphics2SetAlpha(m_index, alpha, delta);
#else
	GraphicsSetAlpha(alpha, delta);
#endif
}

void CDevGraphics::SetKeyColor(VD_COLORREF dwKeyLow, VD_COLORREF dwKeyHigh)
{
	m_KeyColor = dwKeyLow;
#ifdef GRAPHICS_API_VESION_2
	Graphics2SetColorKey(m_index, dwKeyLow, dwKeyHigh);
#else
	GraphicsSetColorKey(dwKeyLow, dwKeyHigh);
#endif
}

VD_COLORREF CDevGraphics::GetKeyColor ()
{
	return m_KeyColor;
}

int CDevGraphics::GetColorFormat()
{
	return m_Surface.format;
}

void CDevGraphics::GetRect(VD_PRECT pRect)
{
	if(pRect)
	{
		*pRect = m_Rect;
	}
}

void CDevGraphics::GetSurface(GRAPHICS_SURFACE *pSurface)
{
	if(pSurface)
	{
		*pSurface = m_Surface;
	}
}


bool CDevGraphics::SetResolution(int w, int h)
{
#ifdef _WIN32
	return false;
#else
#ifdef GRAPHICS_SUPPORT_SETRESOLUTION
	m_currentResolution.w = w;
	m_currentResolution.h = h;
	return TRUE;
#else
	return false;
#endif
#endif
}
VD_BOOL CDevGraphics::SetVideoStandard(uint dwStandard)
{
	if(0)//(0 != m_index2)
	{
		SetKeyColor(0x00000000, 0x00000000);
		SetAlpha(0xff);
		return TRUE;
	}
	if(!GraphicsSetVstd(dwStandard)) 
	{
		if(!UpdateSurface())
		{
			printf("CDevGraphics::CDevGraphics() UpdateSurface failed!!!!!\n");
		}
		SetKeyColor(0x00000000, 0x00000000);
		SetAlpha(0xff);//zlbfix original value = 200
		return TRUE;
	}
	return FALSE;
}

VD_BOOL CDevGraphics::DestroyGraphics(int index)
{
	if((2 == index) && (2 == m_index2))
	{
		//É±ËÀ sync to cvbs Ïß³Ì
		GraphicsDestory2();
		m_index2 = 0;
		
		#if 0//csp modify 20130323
		//csp modify
		if(_instance[0] != NULL)
		{
			delete _instance[0];
			_instance[0] = NULL;
		}
		#endif
	}
	return TRUE;
}

VD_BOOL CDevGraphics::UpdateSurface()
{
	if(0 == m_index2)
	{
		//printf("before UpdateSurface GraphicsGetBuffer\n");
		//fflush(stdout);
		
		//printf("\n\n\n\n");
		//fflush(stdout);
		//sleep(10);
		
		if(GraphicsGetBuffer(&m_Surface))
		{
			return FALSE;
		}
		
		//printf("after UpdateSurface GraphicsGetBuffer\n");
		//fflush(stdout);
		
		//printf("\n\n\n\n");
		//fflush(stdout);
		//sleep(10);
	}
	else if(2 == m_index2)
	{
		if(GraphicsGetBuffer2(&m_Surface))//¼¤»î sync to cvbs Ïß³Ì
		{
			return FALSE;
		}
		printf("GraphicsGetBuffer2 OK!!!!!\n");
	}
	else
	{
		return FALSE;
	}
	
	m_nAge++;
	
	printf("==========================\n");
	printf("mem    = 0x%08x 0x%08x 0x%08x\n", (int)m_Surface.mem, (int)m_Surface.mem0, (int)m_Surface.mem1);
	printf("pitch  = %d\n", m_Surface.pitch);
	printf("width  = %d\n", m_Surface.width);
	printf("height = %d\n", m_Surface.height);
	printf("format = %d\n", m_Surface.format);
	printf("==========================\n");

	m_Rect.SetRect(0, 0, m_Surface.width, m_Surface.height);
	return TRUE;
}

int CDevGraphics::GetAge()
{
	return m_nAge;
}

VD_BOOL CDevGraphics::DeFlicker(VD_PRECT pRect, int level)
{
#ifndef WIN32
	return GraphicsDeFlicker(pRect, level) == 0 ? TRUE : FALSE;
#else
	return FALSE;
#endif//WIN32
}

void CDevGraphics::ShowJpegPic(char* pFileName, unsigned char nShow)
{
	if(nShow)
	{
		if(pFileName)
		{
			GraphicShowJpegPic(1, pFileName);
		}
	}
	else
	{
		GraphicShowJpegPic(0, 0);
	}
}

