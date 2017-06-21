#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "portmanage.h"
#include "gnu_win.h"

typedef struct _port_entry
{	
	struct _port_entry_data
	{
		union
		{
			unsigned int port;
			unsigned int entries;
		};
	}data;
	struct _port_entry *next;
}PortEntry_t,PortLink_t;


typedef struct _port_manage
{
	unsigned int m_min;
	unsigned int m_max;
	PortLink_t m_link;
	Lock_t m_lock;
}PortManage_t;

static PortManage_t *gIpPortList=NULL;

//static void port_manage_init_default_ports()
void port_manage_init_default_ports()
{
	PORT_MANAGE_add_port(0);//reserved
	PORT_MANAGE_add_port(1);//tcpmux
	PORT_MANAGE_add_port(7);//echo
	PORT_MANAGE_add_port(19);//character generator
	PORT_MANAGE_add_port(21);//ftp
	PORT_MANAGE_add_port(22);//ssh
	PORT_MANAGE_add_port(23);//telnet
	PORT_MANAGE_add_port(25);//smtp
	PORT_MANAGE_add_port(31);//msg authentication
	PORT_MANAGE_add_port(42);//wins replication
	PORT_MANAGE_add_port(53);//DNS
	PORT_MANAGE_add_port(67);//bootstrap protocol server
	PORT_MANAGE_add_port(69);//trival file transfer
	PORT_MANAGE_add_port(80);//http
	PORT_MANAGE_add_port(99);//metagram relay
	PORT_MANAGE_add_port(102);//MTA-x 400
	
	PORT_MANAGE_add_port(554);// RTSP
	PORT_MANAGE_add_port(1935);//RTMP
	PORT_MANAGE_add_port(3702);//ONVIF
	
}

static void PORT_MANAGE_dump()
{
	int i;
	PortEntry_t *p;
	assert(gIpPortList != NULL);
	printf("### PORT MANGE min:%u max:%u count:%d ######\n",
		gIpPortList->m_min,gIpPortList->m_max,gIpPortList->m_link.data.entries);
	p = gIpPortList->m_link.next;
	for( i = 0; i < gIpPortList->m_link.data.entries; i++){
		assert( p != NULL);
		printf("\t@%05d %u\n",i+1,p->data.port);
		p = p->next;
	}
}

int PORT_MANAGE_init(unsigned int min,unsigned int max)
{
	if(gIpPortList != NULL) return 0;
	gIpPortList = (PortManage_t *)calloc(1,sizeof(PortManage_t));
	assert(gIpPortList != NULL);
	gIpPortList->m_min = min;
	gIpPortList->m_max = max;
	gIpPortList->m_link.data.entries = 0;
	gIpPortList->m_link.next = NULL;
	LOCK_init(&gIpPortList->m_lock);

	//port_manage_init_default_ports();
	return 0;
}

int PORT_MANAGE_destroy()
{
	int i;
	PortEntry_t *p,*tmp;
	if(gIpPortList == NULL) return 0;
	p = gIpPortList->m_link.next;
	for( i = 0; i < gIpPortList->m_link.data.entries; i++){
		assert( p != NULL);
		tmp = p->next;
		free(p);
		p = tmp;
	}
	LOCK_destroy(&gIpPortList->m_lock);
	gIpPortList->m_min = 0;
	gIpPortList->m_max = 0;
	free(gIpPortList);
	gIpPortList = NULL;
	return 0;
}


int PORT_MANAGE_add_port(unsigned int port)
{
	int i;
	int flag = 0;
	PortEntry_t *s=NULL;
	PortEntry_t *p,*prev=NULL;
	//PortEntry_t *ins_p;

	
	if(!(port >= gIpPortList->m_min && port <= gIpPortList->m_max)){
		return 0;
	}
	assert(gIpPortList != NULL);

	LOCK_lock(&gIpPortList->m_lock);
	
	p = &gIpPortList->m_link;
	for( i = 0; i < gIpPortList->m_link.data.entries; i++){
		prev = p;
		p = p->next;
		assert( p != NULL);
		if(p->data.port == port){
			printf("this port is exist!\n");
			LOCK_unlock(&gIpPortList->m_lock);
			return 0;
		}else if(p->data.port > port){
			flag = 1;
			break;
		}
	}

	assert( p != NULL);
	s = (PortEntry_t *)calloc(1,sizeof(PortEntry_t));
	assert(s != NULL);
	s->data.port = port;
	if(flag == 1){
		s->next = p;
		prev->next = s;
	}else{
		s->next = NULL;
		p->next = s;
	}
	gIpPortList->m_link.data.entries++;
	
	LOCK_unlock(&gIpPortList->m_lock);
	
	PORT_MANAGE_dump();

	return 0;
}

int PORT_MANAGE_apply1_port(unsigned int * const port)
{
	int i;
	int flag = 0;
	PortEntry_t *s=NULL;
	PortEntry_t *p;
	unsigned int ins_port;
	PortEntry_t *ins_p;
	
	assert(gIpPortList != NULL);
	assert(port != NULL);

	LOCK_lock(&gIpPortList->m_lock);

	// find insert point in list
	if(gIpPortList->m_link.data.entries  == 0){
		ins_port = gIpPortList->m_min;
		ins_p = &gIpPortList->m_link;
		flag = 1;
	}else if(gIpPortList->m_link.data.entries  == 1){
		if((gIpPortList->m_link.next->data.port - gIpPortList->m_min) > 0){
			ins_port = gIpPortList->m_min;
			ins_p = &gIpPortList->m_link;
		}else{
			ins_port = gIpPortList->m_link.next->data.port;
			ins_p = gIpPortList->m_link.next;
		}
		flag = 1;
	}else{
		if((gIpPortList->m_link.next->data.port - gIpPortList->m_min) > 0){
			ins_port = gIpPortList->m_min;
			ins_p = &gIpPortList->m_link;
			flag = 1;
		}else{
			p = gIpPortList->m_link.next;
			for( i = 0; i < (gIpPortList->m_link.data.entries -1); i++){
				assert( p != NULL || p->next != NULL);
				assert( p->next->data.port > p->data.port);
				if((p->next->data.port - p->data.port) > 1){
					ins_port = p->data.port + 1;
					ins_p = p;
					flag = 1;
					break;
				}
				p = p->next;
			}
		}
	}
	// 
	// new entry
	if(flag == 0){
		ins_p = p;
		ins_port = ins_p->data.port+1;
	}
	assert(ins_p != NULL);
	assert(ins_port <= gIpPortList->m_max);
	s = (PortEntry_t *)calloc(1,sizeof(PortEntry_t));
	assert(s != NULL);
	s->data.port = ins_port;
	s->next = ins_p->next;
	ins_p->next = s;
	gIpPortList->m_link.data.entries++;
	//
	*port = ins_port;
	
	LOCK_unlock(&gIpPortList->m_lock);

	PORT_MANAGE_dump();
	return 0;
}

// apply two continuous ports, the first port is port1, the second port would be {port1 + 1}
int PORT_MANAGE_apply2_port(unsigned int * const port1)
{
	int i;
	int flag = 0;
	PortEntry_t *s=NULL;
	PortEntry_t *p;
	unsigned int ins_port;
	PortEntry_t *ins_p;
	
	assert(gIpPortList != NULL);
	assert(port1 != NULL);
	
	LOCK_lock(&gIpPortList->m_lock);

	// find insert point in list
	if(gIpPortList->m_link.data.entries  == 0){
		ins_port = gIpPortList->m_min;
		ins_p = &gIpPortList->m_link;
		flag = 1;
	}else if(gIpPortList->m_link.data.entries  == 1){
		if((gIpPortList->m_link.next->data.port - gIpPortList->m_min) > 1){
			ins_port = gIpPortList->m_min;
			ins_p = &gIpPortList->m_link;
		}else{
			ins_port = gIpPortList->m_link.next->data.port;
			ins_p = gIpPortList->m_link.next;
		}
		flag = 1;
	}else{
		if((gIpPortList->m_link.next->data.port - gIpPortList->m_min) > 1){
			ins_port = gIpPortList->m_min;
			ins_p = &gIpPortList->m_link;
			flag = 1;
		}else{
			p = gIpPortList->m_link.next;
			for( i = 0; i < (gIpPortList->m_link.data.entries -1); i++){
				assert( p != NULL || p->next != NULL);
				assert( p->next->data.port > p->data.port);
				if((p->next->data.port - p->data.port) > 2){
					ins_port = p->data.port + 1;
					ins_p = p;
					flag = 1;
					break;
				}
				p = p->next;
			}
		}
	}
	// 
	// new entry 1
	if(flag == 0){
		ins_p = p;
		ins_port = ins_p->data.port+1;
	}
	assert(ins_p != NULL);
	assert(ins_port <= gIpPortList->m_max);
	s = (PortEntry_t *)calloc(1,sizeof(PortEntry_t));
	assert(s != NULL);
	s->data.port = ins_port;
	s->next = ins_p->next;
	ins_p->next = s;
	gIpPortList->m_link.data.entries++;
	// new entry 2
	ins_p = s;
	ins_port++;
	assert(ins_p != NULL);
	assert(ins_port <= gIpPortList->m_max);
	s = (PortEntry_t *)calloc(1,sizeof(PortEntry_t));
	assert(s != NULL);
	s->data.port = ins_port;
	s->next = ins_p->next;
	ins_p->next = s;
	gIpPortList->m_link.data.entries++;

	//
	*port1 = ins_port - 1;
	
	LOCK_unlock(&gIpPortList->m_lock);
	
	PORT_MANAGE_dump();
	return 0;
}

int PORT_MANAGE_free_port(unsigned int port)
{
	int i;
	PortEntry_t *p,*prev=NULL;//,*tmp;
	assert(gIpPortList != NULL);

	LOCK_lock(&gIpPortList->m_lock);

	prev = &gIpPortList->m_link;
	p = prev->next;
	for( i = 0; i < gIpPortList->m_link.data.entries; i++){
		assert( p != NULL);
		if(p->data.port == port) break;
		prev = p;
		p = p->next;
	}
	if(p && p->data.port == port){
		prev->next = p->next;
		free(p);
		gIpPortList->m_link.data.entries--;
	}
	
	LOCK_unlock(&gIpPortList->m_lock);
	
	PORT_MANAGE_dump();
	return 0;
}

#ifdef PM_TEST
int main(int argc,char *argv[])
{
	unsigned int port;
	int cmd = 0;
	int pos = 0;
	const char *usage=
		"*************************************************\r\n"\
		"**************PM DEBUG **************************\r\n"\
		"*********** 0 quit or destroy ********************\r\n"\
		"*********** 1 init *******************************\r\n"\
		"*********** 2 apply1 ****************************\r\n"\
		"*********** 3 apply2 ****************************\r\n"\
		"*********** 4 add ****************************\r\n"\
		"*********** 5 dump ****************************\r\n"\
		"*********** 6 free ******************************\r\n";
	
	while(1){
		printf(usage);
		printf("input your command: ");
		scanf("%d",&cmd);
		switch(cmd){
			case 0:
				PORT_MANAGE_destroy();
				exit(1);
			case 1:
				PORT_MANAGE_init(0,200);
				break;
			case 2:
				PORT_MANAGE_apply1_port(&port);
				printf("Got 1 Port %u!!!!\n",port);
				break;
			case 3:
				PORT_MANAGE_apply2_port(&port);
				printf("Got 2 Port %u!!!!\n",port);
				break;
			case 4:
				printf("input adding port: ");
				scanf("%u",&port);
				PORT_MANAGE_add_port(port);
				break;
			case 5:
				PORT_MANAGE_dump();
				break;
			case 6:
				printf("input free port: ");
				scanf("%u",&port);
				PORT_MANAGE_free_port(port);
				break;
			default:
				PORT_MANAGE_dump();
				break;
		}
		printf("\r\n");
	}
	
	return 0;
}
#endif

