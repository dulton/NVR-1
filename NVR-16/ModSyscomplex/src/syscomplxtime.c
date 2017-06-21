// file description
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "syscomplxtime.h"
#include "update.h"

#ifdef HI3520D
#include "hi_rtc.h"
#endif

//** macro

#define RTC_SET_TIME   _IOW('p', 0x0a, struct linux_rtc_time) /* Set RTC time    */
#define RTC_RD_TIME    _IOR('p', 0x09, struct linux_rtc_time) /* Read RTC time   */

//** typedef

/* Copied from linux/rtc.h to eliminate the kernel dependency */
struct linux_rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

//** local var

//** global var
extern SModSyscplx sModSysIns;

//** local functions
//static u32 getLocalTime(void);
static BOOL IsLoopYear(int year);
static int GetDaysNum(int year,int mon);
static void write_rtc(time_t t, int utc);

//** global functions

s32 modSysCmplx_GetTime(SDateTime* psDateTime)
{
	time_t currtime;
	struct tm *ptm;
	//u8 dtfmt;
	
	//now no complete struct to get sync_time, so use linux time
#if 0//def _DAYTIME_
	currtime = getLocalTime();
#else
	currtime = time(NULL);
#endif
	
	struct tm tm_time;
	localtime_r(&currtime, &tm_time);
	ptm = &tm_time;
	
	psDateTime->nYear 	= ptm->tm_year+1900;
	psDateTime->nMonth	= ptm->tm_mon+1;
	psDateTime->nDay	= ptm->tm_mday;
	psDateTime->nHour	= ptm->tm_hour;
	psDateTime->nMinute = ptm->tm_min;
	psDateTime->nSecode = ptm->tm_sec;
	psDateTime->nWday	= ptm->tm_wday;
	psDateTime->emDateTimeFormat = (EMDATETIMEFORMAT)sModSysIns.eTimeFmt;
	psDateTime->emTimeFormat = (EMTIMEFORMAT)sModSysIns.eTmFormat;
	
	return 0;
}

s32 modSysCmplx_SetTime(const SDateTime* psDateTime)
{
	struct tm tm_time;
	int dwYear,dwMon,dwDay,dwHour,dwMin,dwSec;
	
	if(NULL==psDateTime)
	{
		printf("NULL time struct!!!\n");
		return -1;
	}
	
	dwYear = psDateTime->nYear;
	dwMon = psDateTime->nMonth;
	dwDay = psDateTime->nDay;
	dwHour = psDateTime->nHour;
	dwMin = psDateTime->nMinute;
	dwSec = psDateTime->nSecode;
	
	if( (0 == dwYear) //时间不改，只改格式时将时间设为全0
		&&(0 == dwMon)
		&&(0 == dwDay)
		&&(0 == dwHour)
		&&(0 == dwMin)
		&&(0 == dwSec)
		)
	{
		if(EM_DATETIME_IGNORE > psDateTime->emDateTimeFormat && 
			EM_DATETIME_YYYYMMDDHHNNSS <= psDateTime->emDateTimeFormat
		)
		{
			sModSysIns.eTimeFmt = (EMDATETIMEFORMAT)psDateTime->emDateTimeFormat;
			sModSysIns.eTmFormat = (EMTIMEFORMAT)psDateTime->emTimeFormat;
		}
		return 0;
	}
	else if( ( dwYear>2037 || dwYear<1999 ) 		||
		( dwMon<=0 || dwMon>12) 			||
		( dwDay<=0 || dwDay>31) 			||
		( dwDay>GetDaysNum(dwYear, dwMon) ) ||
		( dwHour<0 || dwHour>23 ) 			||
		( dwMin<0  || dwMin>59 ) 			||
		( dwSec<0  || dwSec>59 )
	)
	{
		return -1;// invalid time 
	}
	
	tm_time.tm_year = dwYear - 1900;
	tm_time.tm_mon 	= dwMon - 1;
	tm_time.tm_mday = dwDay;
	
	tm_time.tm_hour = dwHour;
	tm_time.tm_min 	= dwMin;
	tm_time.tm_sec 	= dwSec;
	
	//printf("y:%d, m:%d, d:%d, h:%d, m:%d, s:%d\n", dwYear - 1900,dwMon - 1,dwDay,dwHour,dwMin,dwSec);
	
	time_t t = mktime(&tm_time);
	
	stime(&t);//set system time
	write_rtc(t,1);//set rtc
	
	if(EM_DATETIME_IGNORE > psDateTime->emDateTimeFormat && 
		EM_DATETIME_YYYYMMDDHHNNSS <= psDateTime->emDateTimeFormat)
	{
		sModSysIns.eTimeFmt = (EMDATETIMEFORMAT)psDateTime->emDateTimeFormat;
		sModSysIns.eTmFormat = (EMTIMEFORMAT)psDateTime->emTimeFormat;
	}
	
	return 0;
}

static BOOL IsLoopYear(int year)
{
	if(!(year&3))
	{
		if(year%100==0)
		{
			if(year%400==0)
			{
				return TRUE;
			}
		}
		else
		{
			return TRUE;
		}
	}
	return FALSE;
}

static int GetDaysNum(int year,int mon)
{
	if(mon==2)
	{
		if(IsLoopYear(year)) return 29;
		else return 28;
	}
	else
	{
		if(mon>=8)
		{
			if(mon&1) return 30;
			else return 31;
		}
		else
		{
			if(mon&1) return 31;
			else return 30;
		}
	}
	return 30;
}

extern void tl_venc_update_basetime(void);
//extern void nvr_sync_time(void);//csp modify 20140406

static void write_rtc(time_t t, int utc)
{
	int rtc;
	struct tm tm;
	
#ifdef HI3520D
	const char *dev_name = "/dev/hi_rtc";
	rtc = open(dev_name, O_RDWR);
	if(rtc < 0)
	{
		printf("open %s failed\n", dev_name);
		return;
	}
#else
	if (( rtc = open( "/dev/rtc", O_WRONLY )) < 0 ) {
		if (( rtc = open ( "/dev/misc/rtc", O_WRONLY )) < 0 )
			printf ( "Could not access RTC" );
	}
#endif
	
	//csp modify
	//tm = *( utc ? gmtime ( &t ) : localtime ( &t ));//根据时区获得相应时间
	if(utc)
	{
		gmtime_r(&t, &tm);
	}
	else
	{
		localtime_r(&t, &tm);
	}
	
	tm.tm_isdst = 0;
	
#ifdef HI3520D
	rtc_time_t hitm;
	hitm.year   = tm.tm_year + 1900;
	hitm.month  = tm.tm_mon + 1;
	hitm.date   = tm.tm_mday;
	hitm.hour   = tm.tm_hour;
	hitm.minute = tm.tm_min;
	hitm.second = tm.tm_sec;
	hitm.weekday = 0;
	int ret = ioctl(rtc, HI_RTC_SET_TIME, &hitm);
	if(ret < 0)
	{
		printf("ioctl: HI_RTC_SET_TIME failed\n");
	}
	//printf("HI_RTC_SET_TIME:\n");
	//printf("year:%d\n", hitm.year);
	//printf("month:%d\n",hitm.month);
	//printf("date:%d\n", hitm.date);
	//printf("hour:%d\n", hitm.hour);
	//printf("minute:%d\n", hitm.minute);
	//printf("second:%d\n", hitm.second);
#else
	if ( ioctl ( rtc, RTC_SET_TIME, &tm ) < 0 )
		printf ( "Could not set the RTC time" );
#endif
	
	close( rtc );
	
	tl_venc_update_basetime();
	//nvr_sync_time();//csp modify 20140406
	
	//printf("write_rtc:rz_venc_update_basetime......\n");
	
	//read_rtc(utc);
}

time_t read_rtc(int utc)
{
	int rtc;
	struct tm tm_now;
	
#ifndef HI3520D
	char *oldtz = 0;
#endif
	
	time_t t = 0;
	
#ifdef HI3520D
//printf("%s HI3520D\n", __func__);
	const char *dev_name = "/dev/hi_rtc";
	rtc = open(dev_name, O_RDWR);
	if(rtc < 0)
	{
		printf("open %s failed\n", dev_name);
		return 0;
	}
	
	rtc_time_t hitm;
	memset ( &hitm, 0, sizeof(hitm) );
	memset ( &tm_now, 0, sizeof( struct tm ) );
	int ret = ioctl(rtc, HI_RTC_RD_TIME, &hitm);
	if(ret < 0)
	{
		printf("ioctl: HI_RTC_RD_TIME failed\n");
	}
	/*
	printf("HI_RTC_RD_TIME:\n");
	printf("year:%d\n", hitm.year);
	printf("month:%d\n",hitm.month);
	printf("date:%d\n", hitm.date);
	printf("hour:%d\n", hitm.hour);
	printf("minute:%d\n", hitm.minute);
	printf("second:%d\n", hitm.second);
	printf("weekday:%d\n", hitm.weekday);
	*/
	if (hitm.year <= 2000)//20130101 00:00:00 星期二
	{
		printf("%s RTC exception --- year <= 2000, set to 20130101 08:00:00\n", __func__);
		tm_now.tm_year = 2013 - 1900;
		tm_now.tm_mon = 1 - 1;
		tm_now.tm_mday = 1;
		tm_now.tm_hour = 0;
		tm_now.tm_min = 0;
		tm_now.tm_sec = 0;
		tm_now.tm_wday = 2;
	}
	else
	{
		tm_now.tm_year = hitm.year - 1900;
		tm_now.tm_mon = hitm.month - 1;
		tm_now.tm_mday = hitm.date;
		tm_now.tm_hour = hitm.hour;
		tm_now.tm_min = hitm.minute;
		tm_now.tm_sec = hitm.second;
		tm_now.tm_wday = hitm.weekday;
	}
	
	
#else
	if (( rtc = open ( "/dev/rtc", O_RDONLY )) < 0 ) {
		if (( rtc = open ( "/dev/misc/rtc", O_RDONLY )) < 0 )
			printf ( "Could not access RTC" );
	}
	
	memset ( &tm_now, 0, sizeof( struct tm ));
	if ( ioctl ( rtc, RTC_RD_TIME, &tm_now ) < 0 )
		printf ( "Could not read time from RTC" );
#endif
	
	tm_now.tm_isdst = -1; // not known
	
	close ( rtc );
	
#ifndef HI3520D
	if ( utc ) {
		oldtz = getenv ( "TZ" );
		setenv ( "TZ", "UTC 0", 1 );
		tzset ( );
	}
#endif
	
	t = mktime ( &tm_now );
	
#ifndef HI3520D
	if ( utc ) {
		if ( oldtz )
			setenv ( "TZ", oldtz, 1 );
		else
			unsetenv ( "TZ" );
		tzset ( );
	}
#endif
	
	return t;
}

#if 0
static u32 getLocalTime(void)
{
	ifly_reserved_t reserved_param;
	GetReservedParam(&reserved_param);
	
	int sync_time = time(NULL);
	int timeZoneCheck = (reserved_param.time_zone - 12) * 3600;
	int daytimeCheck  = reserved_param.dst_enable * 3600;
	sync_time = sync_time + timeZoneCheck + daytimeCheck;
	
	return (sync_time);
}
#endif

