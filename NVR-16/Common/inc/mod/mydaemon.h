#ifndef _MY_DAEMON_H_
#define _MY_DAEMON_H_

#define MY_DAEMON_PORT					6683

#define MY_DAEMON_MAGIC					0x21753846

#define MY_DAEMON_PROCTYPE_WEBS			0
#define MY_DAEMON_PROCTYPE_PPPD			1
#define MY_DAEMON_PROCTYPE_UDHCPC		2
#define MY_DAEMON_PROCTYPE_DDNSUPDATE	3
#define MY_DAEMON_PROCTYPE_DVDRECORDER	4//wrchen 080527

//pw 2010/10/18
#define MY_DAEMON_PROCTYPE_FORMAT		5

#define MY_DAEMON_PROCTYPE_UPNP			6

#define MY_DAEMON_PROCTYPE_P2P			7

#define MY_DAEMON_PROCTYPE_3G			8

#define MY_DAEMON_PROCTYPE_PLATFORM		9

#define MY_DAEMON_OPTYPE_START			0
#define MY_DAEMON_OPTYPE_STOP			1
#define MY_DAEMON_OPTYPE_RESTART		2

typedef struct
{
	int  magic_no;
	int  proc_type;
	int  op_type;
	char arg1[64];
	char arg2[64];
	char arg3[64];
	char arg4[64];
}my_daemon_param;

#endif

