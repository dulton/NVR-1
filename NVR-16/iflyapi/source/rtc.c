#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

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

#define RTC_SET_TIME   _IOW('p', 0x0a, struct linux_rtc_time) /* Set RTC time    */
#define RTC_RD_TIME    _IOR('p', 0x09, struct linux_rtc_time) /* Read RTC time   */

#ifdef CONFIG_FEATURE_HWCLOCK_LONGOPTIONS
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
#endif

time_t read_rtc_v1(int utc)
{
	int rtc;
	struct tm tm_now;
	char *oldtz = 0;
	time_t t = 0;
	
	if (( rtc = open ( "/dev/rtc", O_RDONLY )) < 0 ) {
		if (( rtc = open ( "/dev/misc/rtc", O_RDONLY )) < 0 )
			printf ( "Could not access RTC" );
	}
	memset ( &tm_now, 0, sizeof( struct tm ));
	if ( ioctl ( rtc, RTC_RD_TIME, &tm_now ) < 0 )
		printf ( "Could not read time from RTC" );
	tm_now.tm_isdst = -1; // not known
	
	close ( rtc );
	
	if ( utc ) {
		oldtz = getenv ( "TZ" );
		setenv ( "TZ", "UTC 0", 1 );
		tzset ( );
	}
	
	t = mktime ( &tm_now );
	
	if ( utc ) {
		if ( oldtz )
			setenv ( "TZ", oldtz, 1 );
		else
			unsetenv ( "TZ" );
		tzset ( );
	}
	return t;
}

//define in tl_hslib.a
extern void tl_venc_update_basetime(void);

void write_rtc_v1(time_t t, int utc)
{
	int rtc;
	struct tm tm;

	tl_venc_update_basetime();
	
	if (( rtc = open ( "/dev/rtc", O_WRONLY )) < 0 ) {
		if (( rtc = open ( "/dev/misc/rtc", O_WRONLY )) < 0 )
			printf ( "Could not access RTC" );
	}
	
	tm = *( utc ? gmtime ( &t ) : localtime ( &t ));//根据时区获得相应时间
	tm. tm_isdst = 0;
	
	if ( ioctl ( rtc, RTC_SET_TIME, &tm ) < 0 )
		printf ( "Could not set the RTC time" );
	
	close ( rtc );
}

int show_clock_v1(int utc)
{
	struct tm *ptm;
	time_t t;
	char buffer [64];
	
	t = read_rtc_v1 ( utc );
	ptm = localtime ( &t );  /* Sets 'tzname[]' */
	
	strncpy ( buffer, ctime ( &t ), sizeof( buffer ));
	if ( buffer [0] )
		buffer [strlen ( buffer ) - 1] = 0;
	
	//printf ( "%s  %.6f seconds %s\n", buffer, 0.0, utc ? "" : ( ptm-> tm_isdst ? tzname [1] : tzname [0] ));
	printf ( "%s  %.6f seconds\n", buffer, 0.0 );
	
	return 0;
}

