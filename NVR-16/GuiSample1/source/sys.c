
#include "APIs/System.h"

#ifdef WIN32
//#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <time.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#include <sys/reboot.h>
#endif

int SystemGetCurrentTime(SYSTEM_TIME *ptime)
{
	return 0;
} 

int SystemSetCurrentTime(SYSTEM_TIME *ptime)
{
	return 0;
}

unsigned int SystemGetMSCount(void)
{
	unsigned int t = 0;
	
#ifdef WIN32
//	t = GetTickCount();
#else
	#if 0// yzw modify 20111123
	t = (time(NULL) & 0xffffffff) * 10;
	#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	t = (uint)((((uint64)tv.tv_sec * 1000) + tv.tv_usec / 1000) & 0xffffffff);
	#endif
#endif
	
	return t;
}

static uint64 m_tickoffs = 0;	//tick计数校正
#ifndef _SC_CLK_TCK
#define sysconf
#define _SC_CLK_TCK 100
#endif

//需要独立开发一个驱动
uint64 SystemGetMSCount64(void)
{
#ifdef WIN32
//	return GetTickCount64();
	return 0;
#else
	#if 0 // yzw modify 20111123
	uint64 tick = (uint64)time(NULL); // 产生从0开始的tick计数
	if(tick == (uint64)-1)
	{
		tick = (uint64)-errno;
	}
	if(m_tickoffs == 0)
	{
		m_tickoffs = 0 - tick;
	}

	return (tick + m_tickoffs) * (1000 / sysconf(_SC_CLK_TCK));
	
	#else
	
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (((uint64)tv.tv_sec * 1000) + tv.tv_usec / 1000) & 0xffffffff;
	
	#endif
#endif
} 

unsigned int SystemGetUSCount(void)
{
#if 0
	struct timeval		tv;
	struct timezone	tz;
	int			ret;

	gettimeofday(&tv, &tz);
	ret = tv.tv_sec * 1000 * 1000 + tv.tv_usec;

	return ret;
#endif
	return 0;
} 

//CPU
int SystemGetCPUUsage(void)
{
	return 0;
} 

//MEMORY
int SystemGetCaps(SYSTEM_CAPS * pCaps)
{
	unsigned long		total;
	unsigned long		used;
	unsigned long		mfree;
	unsigned long		shared;
	unsigned long		buffers;
	unsigned long		cached;

	FILE			*fp;
	char			buf[256];
	unsigned int		needs_conversion = 1;

	fp = fopen("/proc/meminfo", "r");
	if (fp == NULL)
	{
		return 0;
	}	
	if (fscanf(fp, "MemTotal: %lu %s\n", &total, buf) != 2)
	{
		fgets(buf, sizeof(buf), fp);		
		fscanf(fp, "Mem: %lu %lu %lu %lu %lu %lu",
			&total, &used, &mfree, &shared, &buffers, &cached);
		used	/= 1024;
		mfree	/= 1024;
		shared	/= 1024;
		buffers	/= 1024;
		cached	/= 1024;
		total	/= 1024;

	}
	else
	{
		needs_conversion = 0;
		fscanf(fp, "MemFree: %lu %s\n", &mfree, buf);
		if (fscanf(fp, "MemShared: %lu %s\n", &shared, buf) != 2)
		{
			shared = 0;
		}
		fscanf(fp, "Buffers: %lu %s\n", &buffers, buf);
		fscanf(fp, "Cached: %lu %s\n", &cached, buf);
		used = total - mfree;
	}
	fclose(fp);

	pCaps->MemoryTotal	= total;
	pCaps->MemoryLeft	= mfree;
	return 0;
}

//扩展的一些参数
int SystemGetCapsEx(SYSTEM_CAPS_EX * pCaps)
{
	memset(pCaps, 0,sizeof(SYSTEM_CAPS_EX));
	return 0;
}

void SystemSleep(unsigned int ms)
{
#ifdef WIN32
	//Sleep(ms);
#else
	usleep(ms * 1000);
#endif
	
} 

void SystemUSleep(unsigned int ms)
{
#ifdef WIN32
	//Sleep(ms/1000);
#else
	usleep(ms);
#endif
} 

static int g_BeepSilence = 0;
void SystemSilence(int isSilence)
{
	g_BeepSilence = (isSilence == 0)?0:1;
}

#if 0
extern gpio_addr_t gpio_addr[];
#endif

void SystemBeep(unsigned int dw_frequence, unsigned int delay)
{
#if 0
	unsigned int SC_SYSSTAT;
	int ret = 0;

	if(g_BeepSilence == 1)
		return 0;

	if ((ret = gpio_init()) < 0)
	{
		ERR_PRINT("Gpio Init Error\n");
		return ;
	}
	SC_SYSSTAT = MemGetU32(gpio_addr[8].mapaddr + 0x20);
	SC_SYSSTAT &= (~(1 << 9)); 
	MemPutU32(gpio_addr[8].mapaddr + 0x20, SC_SYSSTAT);

	gpio_dirsetbit(2, 1, 1);

	if (gpio_writebit(2, 1, 1) < 0)
	{
		ERR_PRINT("Beep Write 1 Fail\n");
		return ;
	}

	usleep(1000 * delay);

	if (gpio_writebit(2, 1, 0) < 0)
	{
		ERR_PRINT("Beep Write 0 Fail\n");
		return ;
	}
#endif	
} 

//重启操作必须使flash设置为读模式wyt 20091107
static void Reboot_FIx_Flash(void)
{
	return;
}

//void SystemReboot(void)
//{
//	return;
//} 

void SystemShutdown(void)
{
	return;
} 

int SystemLogInit(void)
{
	return 0;
} 

int SystemLogWrite(char *fmt, ...)
{
	va_list	ap;
	int		rv = 0;

	va_start(ap, fmt);
	rv = vfprintf(stdout, fmt, ap);
	va_end(ap);

	return rv;
}

#ifdef WIN32
#define snprintf _snprintf
#else
#define snprintf snprintf
#endif

int SystemGetSerialNumber(char *pData, int len)
{
	int ret = 0;
	char id[32] = "000000000000000000";
#if 0
	NetGetMAC("eth0", pData, len);
#endif

	snprintf(pData, len, "%s", id);

	return 0;
}   

int SystemGetDeviceType(char* type)
{
	if(NULL == type)
	{
		printf("SystemGetDeviceType ERROR!\n");
		return 0;
	}
	strcpy(type,"VR9816");

	return 0;
}


/// 获取版本号
int SystemGetVersion(char *version)
{
	//char *systemver = (char *)envgetvalue("systemver");
	if(NULL == version)
	{
		printf("SystemGetDeviceType ERROR!\n");
		return 0;
	}
	strcpy(version,"3.0");
	return 0;
}


enum ui_language_t SystemGetUILanguage(void)
{
	return UI_LANGUAGE_CHINESE_SIMPLE;
}

int SystemSetUILanguage(enum ui_language_t lang)
{

	return 0;

}





// 获得显示分辨率

#define VGA_PIXEL_1024X768		0X1		// e2prom值为１表示1024x768
#define VGA_PIXEL_800X600		0xf		// e2prom值为0xf是默认值，表示800x600

int SystemGetVGAPixelCaps(int *pCaps)
{
	if(NULL == pCaps)
	{
		return -1;
	}

	*pCaps = 0;
	*pCaps = (1 << SYS_VGAPIXEL_800x600)
		|(1 << SYS_VGAPIXEL_1024x768);

	return 0;
}


int SystemGetVGAPixel()
{
#if 0
	char *vgapixel_str = (char *)envgetvalue("vgapixel");
	int vgapixel_v = (e2prom_flags >> 2) & 0xf;

	if(NULL == vgapixel_str){
		if(VGA_PIXEL_800X600 == vgapixel_v)
		{
			return SYS_VGAPIXEL_800x600;
		}
		else if(VGA_PIXEL_1024X768 == vgapixel_v)
		{
			return SYS_VGAPIXEL_1024x768;
		}
		else
		{
			return SYS_VGAPIXEL_800x600;
		}

	} else {
		if(0 == strcmp("800", vgapixel_str))
		{
			return SYS_VGAPIXEL_800x600;
		}
		else if(0 == strcmp("1024", vgapixel_str))
		{
			return SYS_VGAPIXEL_1024x768;
		}
		else
		{
			return SYS_VGAPIXEL_800x600;
		}
	}
#endif
	return SYS_VGAPIXEL_1024x768;


}

// 设置显示分辨率
int SystemSetVGAPixel(int vgapixel)
{
	return 0;
}




