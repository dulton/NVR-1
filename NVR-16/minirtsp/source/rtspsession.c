#include <stdio.h>
#include <stdlib.h>
#include "rtspdef.h"
#include "rtspsession.h"

#include <string.h>

RtspSession_t *g_RtspServerGrp = NULL;

int RTSP_session_init()//初始化一个会话链表
{
	RtspSession_t *s = NULL;
	s = (RtspSession_t *)calloc(1,sizeof(RtspSession_t));
	if(s == NULL)
	{
		printf("ERR: RTSP_session_init failed!\n");
		return -1;
	}
	s->data.entries = 0;
	s->next = NULL;
	LOCK_init(&s->data.m_lock);
	g_RtspServerGrp = s;
	return 0;
}

//向链表添加会话
RtspSession_t *RTSP_session_add(char *ip_dst,int transport,char *stream,void *ctx)
{
	RtspSession_t *s=NULL;
	RtspSession_t *p=g_RtspServerGrp;
	if(g_RtspServerGrp == NULL)
	{
		RTSP_session_init();
	}
	
	LOCK_lock(&g_RtspServerGrp->data.m_lock);
	
	p = g_RtspServerGrp;
	
	s = (RtspSession_t *)calloc(1,sizeof(RtspSession_t));
	if(s == NULL)
	{
		printf("ERR: RTSP_session_add failed");
		LOCK_unlock(&g_RtspServerGrp->data.m_lock);
		return NULL;
	}
	strcpy(s->data.ip,ip_dst);
	strcpy(s->data.stream,stream);
	s->data.transport =transport;
	s->data.rtsp = ctx;
	s->next = NULL;
	//
	while(p->next) p = p->next;
	p->next = s;
	g_RtspServerGrp->data.entries ++;
	
	LOCK_unlock(&g_RtspServerGrp->data.m_lock);
	
	RTSP_session_dump();
	
	return s;
}

int RTSP_session_del(char *ip_dst,int transport,char *stream)
{
	int focus = false;
	RtspSession_t *q=g_RtspServerGrp;
	RtspSession_t *p= NULL;
	
	if(g_RtspServerGrp == NULL)
	{
		printf("ERR: g_RtspServerGrp is NULL,please check it!!!\n");
		return -1;
	}
	
	LOCK_lock(&g_RtspServerGrp->data.m_lock);
	
	p = g_RtspServerGrp->next;
	while(p)
	{
		if((strcmp(p->data.ip,ip_dst) == 0) &&
			(strcmp(p->data.stream,stream) == 0) &&
			(p->data.transport == transport))
		{
			focus = true;
			break;
		}
		q = p;
		p = p->next;
	}
	if(focus)
	{
		q->next = p->next;
		free(p);
		p = NULL;
	}
	g_RtspServerGrp->data.entries--;
	
	LOCK_unlock(&g_RtspServerGrp->data.m_lock);
	
	RTSP_session_dump();
	
	return 0;
}

RtspSession_t* RTSP_session_find(char *ip_dst,int transport,char *stream)
{
	int focus = false;
	RtspSession_t *p = NULL;
	
	if(g_RtspServerGrp == NULL) return NULL;
	
	LOCK_lock(&g_RtspServerGrp->data.m_lock);
	p = g_RtspServerGrp->next;
	while(p)
	{
		if((strcmp(p->data.ip,ip_dst) == 0) &&
			(strcmp(p->data.stream,stream) == 0) &&
			(p->data.transport == transport))
		{
			focus = true;
			break;
		}
		p = p->next;
	}
	if(focus)
	{
		printf("find session:%s sussess!\n",ip_dst);
		LOCK_unlock(&g_RtspServerGrp->data.m_lock);
		return p;
	}
	else
	{
		printf("find session:%s failed!\n",ip_dst);
		LOCK_unlock(&g_RtspServerGrp->data.m_lock);
		return NULL;
	}
}

int RTSP_session_entries()
{
	int entr;
	LOCK_lock(&g_RtspServerGrp->data.m_lock);
	entr = g_RtspServerGrp->data.entries;
	LOCK_unlock(&g_RtspServerGrp->data.m_lock);
	return entr;
}

RtspSession_t* RTSP_session_get(int index)
{
	int i;
	//int focus = false;
	RtspSession_t *p=g_RtspServerGrp;
	
	LOCK_lock(&g_RtspServerGrp->data.m_lock);
	if((index <= 0) || (index > g_RtspServerGrp->data.entries))
	{
		LOCK_unlock(&g_RtspServerGrp->data.m_lock);
		return NULL;
	}
	
	for(i=0;i<index;i++) p = p->next;
	
	LOCK_unlock(&g_RtspServerGrp->data.m_lock);
	return p;
}

int RTSP_session_destroy()
{
	RtspSession_t *p=g_RtspServerGrp,*q;
	Lock_t lock_tmp;
	LOCK_lock(&g_RtspServerGrp->data.m_lock);
	lock_tmp = g_RtspServerGrp->data.m_lock;
	while(p)
	{
		q = p->next;
		free(p);
		p = q;
	}
	g_RtspServerGrp = NULL;
	LOCK_unlock(&lock_tmp);
	LOCK_destroy(&lock_tmp);
	return 0;
}

int RTSP_session_dump()
{
	int i=0;
	RtspSession_t *p=g_RtspServerGrp->next;
	printf("total session entries:%d\n",g_RtspServerGrp->data.entries);
	while(p)
	{
		i++;
		printf("session%d ip:%s transport:%d stream:%s\n",i,p->data.ip,p->data.transport,p->data.stream);
		p = p->next;
	}
	return 0;
}

