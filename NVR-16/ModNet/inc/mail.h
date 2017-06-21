#ifndef _MAIL_H_
#define _MAIL_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") 
#else
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <netdb.h>

// #include <linux/unistd.h>
// 
// _syscall0(pid_t, gettid);
#define SEMHANDLE  sem_t *
#define SOCKHANDLE int
#define INVALID_SOCKET		(-1)
#define SOCKET_ERROR		(-1)
#endif

#define MAIL_BOUNDARY1 "------=_NextPart_000_003C_01C9DB0E.F6FCD680"
#define MAIL_BOUNDARY2 "------=_NextPart_000_003D_01C9DB0E.F6FCD680"
#define MAX_ATTACHED_FILE_LEN (720 * 576 * 3 /2)
#define MAX_MAIL_BODY_LEN (2<<10)

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef BOOL
#define BOOL int
#endif

#ifdef __cplusplus
extern "C" {
#endif

//pw 2010/9/15
BOOL sendmail(char *s_name_from, char *s_mail_from, char *s_name_to, char *s_mail_to, char *s_user, char *s_password, char *s_mailserver, char *s_subject, char *s_body, char *s_attached, char *s_encode_type);

#ifdef __cplusplus
}
#endif

#endif
