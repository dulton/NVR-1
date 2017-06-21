#ifndef _NTP_CLIENT_H_
#define _NTP_CLIENT_H_

#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

int getTimeFromNtpserver(char *ntpserveradress, int timezoneoffset, int timeout, struct timeval *tv);

#ifdef __cplusplus
}
#endif

#endif //_NTP_CLIENT_H_

