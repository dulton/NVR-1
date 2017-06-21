#ifndef __RTSP_SESSION_H__
#define __RTSP_SESSION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "gnu_win.h"

typedef struct _session
{	
	struct _SessionElem
	{
		union
		{
			struct
			{
				char ip[20];
				int transport;
				char stream[64];
				void *rtsp;
			};
			struct
			{
				int entries;
				Lock_t m_lock;
			};
		};
	}data;
	struct _session *next;
}RtspSession_t;

int RTSP_session_init();
RtspSession_t *RTSP_session_add(char *ip_dst,int transport,char *stream,void *ctx);
int RTSP_session_del(char *ip_dst,int transport,char *stream);
RtspSession_t* RTSP_session_find(char *ip_dst,int transport,char *stream);
int RTSP_session_entries();
RtspSession_t* RTSP_session_get(int index);
int RTSP_session_destroy();
int RTSP_session_dump();

#ifdef __cplusplus
}
#endif
#endif

