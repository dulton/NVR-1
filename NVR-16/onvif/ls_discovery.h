

/* -*- C++ -*- */
#ifndef __ONVIF_WSDISCOVERY_H
#define __ONVIF_WSDISCOVERY_H 1

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <list>
#include <string>
#include "ls_type.h"
#include "ls_receiver_ptz.h"

using namespace std;

//typedef  int receiver_onvif;

typedef void BackgroundHandlerProc(int fd, uint8_t revents, void *clientData);



class onvif_wsdiscovery
{
public:
    onvif_wsdiscovery();
    virtual ~onvif_wsdiscovery();
    virtual uint32_t init(unsigned int dwLocalIp = 0);
    uint32_t add_receiver(discovery_device *receiver);
    uint32_t send_wsdiscovery_multi();
    uint32_t send_once_wsdiscovery_multi();
public:
	list<discovery_device *>  receivers;
private:
    static void handle_udp_helper(int fd, uint8_t revents, void *clientData);
    void handle_udp(int fd, uint8_t revents);
private:
 //   ls_eventloop_t            *eventloop;
    
    int                         udp_fd;
    struct sockaddr_storage     sa_multi;
    socklen_t                   salen_multi;
   
    ssize_t                     buf_len;
    uint8_t                     send_once_already;
    pthread_mutex_t             send_once_mutex;
};

#endif
