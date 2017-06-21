#include <stdint.h>
#include <sys/types.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <uuid/uuid.h>

#include "ls_socket.h"
#include "ls_discovery.h"
#include "ls_sha.h"
#include "ls_base64.h"

#define ONVIF_WSDISCOVERY_HOST "239.255.255.250"
#define ONVIF_WSDISCOVERY_SERV "3702"

void event_loop(int socket, BackgroundHandlerProc * proc, void * clientData, int timeout)
{
	fd_set readSet;
	fd_set writeSet;
	fd_set exceptionSet;
	
	//printf("event_loop start...\n");
	//fflush(stdout);
	
	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	FD_ZERO(&exceptionSet);
	
	FD_SET((unsigned)socket, &readSet);
	
	//csp modify
	//struct timeval tv_timeToDelay;
	//tv_timeToDelay.tv_sec = timeout;
	//tv_timeToDelay.tv_usec = 0;
	
	//struct sockaddr_in client;
	//int count = 0;
	
	while(true)
	{
		//csp modify
		struct timeval tv_timeToDelay;
		tv_timeToDelay.tv_sec = timeout;
		tv_timeToDelay.tv_usec = 0;
		
		//csp modify
		//int selectResult = select(socket + 1, &readSet, &writeSet, &exceptionSet, &tv_timeToDelay);
		int selectResult = select(socket + 1, &readSet, NULL, NULL, &tv_timeToDelay);
		
		//csp modify
		//if(selectResult > 0 && FD_ISSET(socket, &readSet) && FD_ISSET(socket, &readSet)/*sanity check*/)
		if(selectResult > 0 && FD_ISSET(socket, &readSet)/*sanity check*/)
		{
			/*if( (len = recvfrom(socket, buf, MAXBUF, 0, (struct sockaddr *)&client, (socklen_t*)&n)) < 0 )
			{
				perror("recvfrom");
			}
			else
			{
				buf[len] = 0;
				lsprint("form:%s len:%d\n%s\n",inet_ntoa(client.sin_addr),len,buf);
				//lsprint("form:%s len:%d\n",inet_ntoa(client.sin_addr),len);
			}*/
			//cout<<++count<<endl;
			
			//printf("do event_loop...\n");
			//fflush(stdout);
			
			(*proc)(socket, 0, (void *)clientData);
		}
		else
		{
			break;
		}
	}
	/*struct sockaddr_in client;
	char buf[MAXBUF];
	int n = sizeof(client);
	int len = 0;
	while(1)
	{
		if( (len = recvfrom(socket, buf, MAXBUF, 0, (struct sockaddr *)&client, (socklen_t*)&n)) < 0 )
		{
			perror("recvfrom");
		}
		else
		{
			buf[len] = 0;
			lsprint("form:%s len:%d\n%s\n",inet_ntoa(client.sin_addr),len,buf);
		}
	}*/
}

onvif_wsdiscovery::onvif_wsdiscovery()
{
	//this->eventloop = eventloop;
    this->udp_fd = -1;
    this->send_once_already = 0;
	
	receivers.clear();
	
	//discovery
	//init();
	//send_once_wsdiscovery_multi();
}

onvif_wsdiscovery::~onvif_wsdiscovery()
{
    //pthread_mutex_destroy(&send_once_mutex);//csp modify
	
    //ls_eventloop_fd_unregister(eventloop, udp_fd);
    
    //csp modify
    if(udp_fd != -1)
    {
    	close(udp_fd);
		udp_fd = -1;
	}
	
    while(!receivers.empty())
	{
		//delete (device_info *)receivers.front();
		delete receivers.front();
		receivers.pop_front();
	}
}

uint32_t onvif_wsdiscovery::init(unsigned int dwLocalIp)
{
	//csp modify
	struct in_addr in;
	in.s_addr = dwLocalIp;
	char localip[32] = {0};
	strcpy(localip, inet_ntoa(in));
	
	//csp modify
	if(0 != ls_sock_udp_client(&udp_fd, ONVIF_WSDISCOVERY_HOST, ONVIF_WSDISCOVERY_SERV, "0.0.0.0", ONVIF_WSDISCOVERY_SERV, &sa_multi, &salen_multi))
	//if(0 != ls_sock_udp_client(&udp_fd, ONVIF_WSDISCOVERY_HOST, ONVIF_WSDISCOVERY_SERV, localip, ONVIF_WSDISCOVERY_SERV, &sa_multi, &salen_multi))
	{
		//csp modify
		if(udp_fd != -1)
		{
			close(udp_fd);
			udp_fd = -1;
		}
		
		return 1;
	}
	
	//lsprint("udp_fd:%d\n",udp_fd);//csp modify
	
#if 1//csp modify
	/************************************************************************/
	/*加入组播                                                */
	/************************************************************************/
	{
		struct ip_mreq mreq;
		memset(&mreq,0,sizeof(struct ip_mreq));
		mreq.imr_multiaddr.s_addr = inet_addr(ONVIF_WSDISCOVERY_HOST);
		mreq.imr_interface.s_addr = dwLocalIp;//INADDR_ANY;
		if(setsockopt(udp_fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,(const char*)&mreq,sizeof(mreq)) == -1)
		{
			printf("onvif_wsdiscovery::init - setsockopt IP_ADD_MEMBERSHIP error!\n");
		}
		
		int optval = 0;
		if(setsockopt(udp_fd,IPPROTO_IP,IP_MULTICAST_LOOP,(char *)&optval,sizeof(int)) == -1)
		{
			printf("onvif_wsdiscovery::init - 套接字选项设置(禁止环回)失败,错误码=%s\n",strerror(errno));
		}
		else
		{
			//printf("onvif_wsdiscovery::init - 套接字选项设置(禁止环回)成功\n");
		}
	}
#endif
	
#if 0//csp modify
    if(0 != pthread_mutex_init(&send_once_mutex, NULL))
    {
        close(udp_fd);
		udp_fd = -1;//csp modify
        return 1;
    }
	
	//if(0 != ls_eventloop_fd_register(eventloop, udp_fd, SVX_EV_READ, handle_udp_helper, this))
	{
		pthread_mutex_destroy(&send_once_mutex);
		//close(udp_fd);
		return 1;
	}
#endif
	
    return 0;
}

uint32_t onvif_wsdiscovery::send_wsdiscovery_multi()
{
    char   wsd_str[4096];
    uuid_t uuid;
    char   uuid_str[64];
	
	//printf("send_wsdiscovery_multi start...\n");
	//fflush(stdout);
	
    uuid_generate(uuid);
    uuid_unparse_lower(uuid, uuid_str);
	
	#if 1//csp modify
	snprintf(wsd_str, sizeof(wsd_str),
             "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
             "<Envelope xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\""
             " xmlns=\"http://www.w3.org/2003/05/soap-envelope\">"
             "<Header>"
             "<wsa:MessageID xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\">uuid:%s</wsa:MessageID>"
             "<wsa:To xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\">"
             "urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To>"
             "<wsa:Action xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\">"
             "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</wsa:Action>"
             "</Header>"
             "<Body>"
             "<Probe xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
             " xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
             " xmlns=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\">"
             "<Types>dn:NetworkVideoTransmitter</Types>"
             "<Scopes />"
             "</Probe>"
             "</Body>"
             "</Envelope>", uuid_str);
	#else
    snprintf(wsd_str, sizeof(wsd_str),
             "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
             "<Envelope xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\""
             "          xmlns=\"http://www.w3.org/2003/05/soap-envelope\">"
             "  <Header>"
             "    <wsa:MessageID xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\">uuid:%s</wsa:MessageID>"
             "    <wsa:To xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\">"
             "      urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To>"
             "    <wsa:Action xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\">"
             "      http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</wsa:Action>"
             "  </Header>"
             "  <Body>"
             "    <Probe xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
             "           xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
             "           xmlns=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\">"
             "      <Types>dn:NetworkVideoTransmitter</Types>"
             "      <Scopes />"
             "    </Probe>"
             "  </Body>"
             "</Envelope>", uuid_str);
	#endif
	
    if(-1 == sendto(udp_fd, wsd_str, strlen(wsd_str), 0, (struct sockaddr *)&sa_multi, salen_multi))
    {
        lsprint("Onvif_wsdiscovery, send PROBE failed. peer:[%s]:%s. [errno:%d. %m]\n",
                ONVIF_WSDISCOVERY_HOST, ONVIF_WSDISCOVERY_SERV, errno);
        return 1;
    }
	
	lsprint("^^^Onvif_wsdiscovery, send PROBE. peer:[%s]:%s.###\n", ONVIF_WSDISCOVERY_HOST, ONVIF_WSDISCOVERY_SERV);
	fflush(stdout);
	
	//csp modify
	//event_loop(udp_fd,handle_udp_helper,this,1);
	//event_loop(udp_fd,handle_udp_helper,this,2);
	event_loop(udp_fd,handle_udp_helper,this,3);//大华高速球搜不到的问题是因为时间太短的原因吗???
	
    return 0;
}

uint32_t onvif_wsdiscovery::send_once_wsdiscovery_multi()
{
    uint32_t ret = 0;
	
    if(!send_once_already)
    {
        //pthread_mutex_lock(&send_once_mutex);//csp modify
		
        if(!send_once_already)
        {
            send_once_already = 1;
            ret = send_wsdiscovery_multi();
        }
        
        //pthread_mutex_unlock(&send_once_mutex);//csp modify
    }
	
    return ret;
}

uint32_t onvif_wsdiscovery::add_receiver(discovery_device *receiver)
{
    if(NULL == receiver) return 1;
	
    receivers.push_back(receiver);
	
    return 0;
}

void onvif_wsdiscovery::handle_udp_helper(int fd, uint8_t revents, void *clientData)
{
	//printf("handle_udp_helper start...\n");
	//fflush(stdout);
	
    onvif_wsdiscovery *me = static_cast<onvif_wsdiscovery *>(clientData);
    me->handle_udp(fd, revents);
}

void onvif_wsdiscovery::handle_udp(int fd, uint8_t revents)
{
    struct sockaddr_storage  sa_peer;
    socklen_t                salen_peer = salen_multi;
    char                     peer_ip_str[64];
    discovery_device         *receiver_onvif = NULL;
    xmlDocPtr                doc = NULL;
    xmlNodePtr               node = NULL;
    xmlXPathContextPtr       xpath_context = NULL;
    xmlXPathObjectPtr        xpath_object = NULL;
    xmlChar                  *xaddrs = NULL;
    char                     xaddrs_buf[256];
	char					 uuid_buf[256];
	uint8_t                  buf[10*1024];//csp modify
	
    ///////if(!(revents & SVX_EV_READ)) return;
	
	//printf("handle_udp start...\n");
	//fflush(stdout);
	
    /* get the ws-discovery response */
    buf_len = recvfrom(udp_fd, buf, sizeof(buf), 0, (struct sockaddr *)&sa_peer, &salen_peer);
    if(buf_len <= 0) return;
	
	buf[buf_len] = '\0';//csp modify
	
	//lsprint("handle_udp:%s\n\n\n",buf);
	//fflush(stdout);
	
	/* check receiver via peer IP */
	memset(peer_ip_str, 0, sizeof(peer_ip_str));//csp modify
	if(0 != ls_sock_ntop_ip((struct sockaddr *)&sa_peer, peer_ip_str, sizeof(peer_ip_str)))
	{
		return;
	}
	
	//printf("handle_udp - 1, peer_ip_str=%s\n",peer_ip_str);
	
	for(list<discovery_device *>::iterator it = receivers.begin(); it != receivers.end(); it++)
    {
		//cout<<(*it)->get_ip()<<endl;
		int len = 0;
		if(0 == strcmp((*it)->get_ip(len), peer_ip_str))
		{
			return;
		}
    }
	
	//printf("handle_udp - 2\n");
	
	receiver_onvif = new device_ptz();
	if(NULL == receiver_onvif)
	{
		return;
	}
	
	//printf("handle_udp - 3\n");
	
	receivers.push_back(receiver_onvif);
	receiver_onvif->set_ip(peer_ip_str,strlen(peer_ip_str));
    
	/* parse xaddrs */
	do
	{
		//printf("parse xaddrs-1@@@\n");
		//fflush(stdout);
		
		if(NULL == (doc = xmlParseMemory((const char *)buf, buf_len))) break;
		
		//printf("parse xaddrs-2\n");
		//fflush(stdout);
		
		if(NULL == (xpath_context = xmlXPathNewContext(doc))) break;
		
		//printf("parse xaddrs-3\n");
		//fflush(stdout);
		
		if(0 != xmlXPathRegisterNs(xpath_context, BAD_CAST"SOAP-ENV", BAD_CAST"http://www.w3.org/2003/05/soap-envelope")) break;
		//if(0 != xmlXPathRegisterNs(xpath_context, BAD_CAST"SOAP-ENC", BAD_CAST"http://www.w3.org/2003/05/soap-encoding")) break;
		if(0 != xmlXPathRegisterNs(xpath_context, BAD_CAST"wsa", BAD_CAST"http://schemas.xmlsoap.org/ws/2004/08/addressing")) break;
		if(0 != xmlXPathRegisterNs(xpath_context, BAD_CAST"ns1", BAD_CAST"http://schemas.xmlsoap.org/ws/2005/04/discovery")) break;
		if(0 != xmlXPathRegisterNs(xpath_context, BAD_CAST"dn", BAD_CAST"http://www.onvif.org/ver10/network/wsdl")) break;
		
		//printf("parse xaddrs-4\n");
		//fflush(stdout);
		
		if(NULL == (xpath_object = xmlXPathEvalExpression(BAD_CAST"/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:ProbeMatches/ns1:ProbeMatch/ns1:XAddrs", xpath_context))) break;
		if(xmlXPathNodeSetIsEmpty(xpath_object->nodesetval)) break;
		
		node = xpath_object->nodesetval->nodeTab[0];
		
		//printf("parse xaddrs-5,node=0x%08x\n",(unsigned int)node);
		//fflush(stdout);
		
		if(XML_ELEMENT_NODE != node->type) break;
		if(NULL == (xaddrs = xmlNodeGetContent(node))) break;
		strncpy(xaddrs_buf, (const char *)xaddrs, sizeof(xaddrs_buf));
		//cout<<xaddrs_buf<<endl;
		
		printf("xaddrs=%s\n",xaddrs_buf);
		fflush(stdout);
		
		receiver_onvif->set_xaddrs(xaddrs_buf,strlen(xaddrs_buf));
		
		if(NULL != xaddrs) xmlFree(xaddrs);
		if(NULL != xpath_object)
		{
			//csp modify
			xmlXPathFreeObject(xpath_object);
			xpath_object = NULL;
		}
		
		/*parse uuid*/
		if(NULL == (xpath_object = xmlXPathEvalExpression(BAD_CAST"/SOAP-ENV:Envelope/SOAP-ENV:Body/ns1:ProbeMatches/ns1:ProbeMatch/wsa:EndpointReference/wsa:Address", xpath_context))) break;
		if(xmlXPathNodeSetIsEmpty(xpath_object->nodesetval)) break;
		node = xpath_object->nodesetval->nodeTab[0];
		if(XML_ELEMENT_NODE != node->type) break;
		if(NULL == (xaddrs = xmlNodeGetContent(node))) break;
		strncpy(uuid_buf, (const char *)xaddrs, sizeof(uuid_buf));
		
		//printf("uuid=%s\n",uuid_buf);
		//fflush(stdout);
		
		receiver_onvif->set_uuid(uuid_buf,strlen(uuid_buf));
		
		if(NULL != xaddrs) xmlFree(xaddrs);
		if(NULL != xpath_object)
		{
			//csp modify
			xmlXPathFreeObject(xpath_object);
			xpath_object = NULL;
		}
	}while(0);
	
	//csp modify
	if(NULL != xpath_object)
	{
		xmlXPathFreeObject(xpath_object);
		xpath_object = NULL;
	}
	
	if(NULL != xpath_context) xmlXPathFreeContext(xpath_context);
	if(NULL != doc) xmlFreeDoc(doc);
	xmlCleanupParser();
}

