#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <net/if.h>
#include <errno.h>
#include <ctype.h>
#include "ls_socket.h"

//#include "ls_log.h"
//#include "ls_io.h"
#include "ls_string.h"
//#include "ls_env.h"
#include "ls_error.h"
#include "ls_type.h"

#ifndef SO_RCVBUFFORCE
#define SO_RCVBUFFORCE 33
#endif

#ifndef SO_SNDBUFFORCE
#define SO_SNDBUFFORCE 32
#endif

#define SVX_SOCK_TCP_LISTEN_QUEUE_LEN 16
#define SVX_SOCK_ICMP_ECHO_HEAD_LEN   8
#define SVX_SOCK_RTP_PORT_MIN         10000
#define SVX_SOCK_RTP_PORT_MAX         65000




bool ls_io_set_nonblocking(int sock)
{
	int curFlags = fcntl(sock, F_GETFL, 0);
	return fcntl(sock, F_SETFL, curFlags|O_NONBLOCK) >= 0;
}
bool ls_io_unset_nonblocking(int sock)
{
	int curFlags = fcntl(sock, F_GETFL, 0);
	return fcntl(sock, F_SETFL, curFlags&(~O_NONBLOCK) ) >= 0;
}


/*
 * calculate checksum for internet protocol head: IPv4, ICMPv4, IGMPv4, ICMPv6, UDP, TCP.
 * Reference: RFC 1071.
 */
static uint16_t ls_sock_checksum(uint16_t *addr, size_t len)
{
    uint32_t sum = 0;

    while(len > 1)
    {
        sum += *addr++;
        len -= 2;
    }

    if(len > 0)
        sum += *(uint8_t *)addr;

    while(sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return (uint16_t)(~sum);
}

static int ls_sock_family_to_level(int family)
{
    switch (family)
    {
    case AF_INET:
        return IPPROTO_IP;
    case AF_INET6:
        return IPPROTO_IPV6;
    default:
        return -1;
    }
}

static int ls_sock_fd_to_family(int fd)
{
    struct sockaddr_storage ss;
    socklen_t               len;

    len = sizeof(ss);
    if(getsockname(fd, (struct sockaddr *)&ss, &len) < 0) return -1;
    return ss.ss_family;
}

static uint32_t ls_sock_tcp_connect_wait_do(int32_t fd, struct sockaddr *sa_peer, socklen_t salen_peer,
                                             struct sockaddr *sa_local, socklen_t salen_local, int timeout_ms)
{
    int            n, m;
    int            error = 0;
    const int      on = 1;
    socklen_t      len;
    in_port_t      port_local;
    fd_set         rset, wset;
    struct timeval tval;
	
	//printf("ls_sock_tcp_connect_wait_do-1\n");
	
    /* set SO_REUSEADDR option if not use random port */
    ls_sock_ntop_port(sa_local, &port_local);
    if(ntohs(port_local) > 0 && setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) return LS_ERRNO_SYSCALL;
	
	//printf("ls_sock_tcp_connect_wait_do-2\n");
	
    /* bind local addr */
    if(0 != bind(fd, sa_local, salen_local)) return LS_ERRNO_SYSCALL;
	
	//printf("ls_sock_tcp_connect_wait_do-3\n");
	
    /* <case 1> common connect */
    if(timeout_ms <= 0)
    {
        if(0 != connect(fd, sa_peer, salen_peer))
            return LS_ERRNO_SYSCALL;
        else
            return LS_ERRNO_OK;
    }
	
	//printf("ls_sock_tcp_connect_wait_do-4\n");
	
    /* <case 2> timeout connect */
    ls_io_set_nonblocking(fd);
    if(0 == (n = connect(fd, sa_peer, salen_peer)))
    {
    	//printf("ls_sock_tcp_connect_wait_do-5\n");
		
        /* connect completed immediately */
        ls_io_unset_nonblocking(fd);
        return LS_ERRNO_OK;
    }
    else if(n < 0)
    {
    	//printf("ls_sock_tcp_connect_wait_do-6,timeout_ms=%d\n",timeout_ms);
		
        if(errno != EINPROGRESS)
        {
			//lsprint("error:%d\n",errno);
			//perror("error");
			ls_io_unset_nonblocking(fd);
            return LS_ERRNO_SYSCALL;
        }
		
        tval.tv_sec  = timeout_ms / 1000;
        tval.tv_usec = (timeout_ms % 1000) * 1000;
		
		//printf("ls_sock_tcp_connect_wait_do-7,(sec:%d,usec:%d)\n",tval.tv_sec,tval.tv_usec);
        
        while(1)
        {
            FD_ZERO(&rset);
            FD_ZERO(&wset);
            FD_SET(fd, &rset);
            FD_SET(fd, &wset);
			
			//printf("ls_sock_tcp_connect_wait_do-8\n");
			
            if(0 == (m = select(fd + 1, &rset, &wset, NULL, &tval)))
            {
            	//printf("ls_sock_tcp_connect_wait_do-9\n");
				
                /* timeout */
                ls_io_unset_nonblocking(fd);
                errno = ETIMEDOUT;
                return LS_ERRNO_TIMEOUT;
            }
            else if(m < 0)
            {
            	//printf("ls_sock_tcp_connect_wait_do-10\n");
				
				/* error */
				if(EINTR == errno)
				{
					//printf("errno:EINTR\n");
					continue;
				}
				else
				{
					ls_io_unset_nonblocking(fd);
					return LS_ERRNO_SYSCALL;
				}
            }
            else
            {
            	//printf("ls_sock_tcp_connect_wait_do-11\n");
				
                /* sucessful */
                ls_io_unset_nonblocking(fd);
                if(FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset))
                {
                	//printf("ls_sock_tcp_connect_wait_do-12\n");
					
					if(FD_ISSET(fd, &rset))
					{
						//printf("ls_sock_tcp_connect_wait_do-13\n");
					}
					
					if(FD_ISSET(fd, &wset))
					{
						//printf("ls_sock_tcp_connect_wait_do-14\n");
					}
					
					len = sizeof(error);
					if(0 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) && 0 == error)
					{
						//printf("ls_sock_tcp_connect_wait_do-15\n");
						return LS_ERRNO_OK;
					}
					else
					{
						//printf("ls_sock_tcp_connect_wait_do-16\n");
						//lsprint("22222\n");//csp modify
						if(error)
							errno = error;
						return LS_ERRNO_SYSCALL;
					}
                }
                else
				{
					//printf("ls_sock_tcp_connect_wait_do-17\n");
					//lsprint("44444\n");//csp modify
                    return LS_ERRNO_SYSCALL;
				}
			}
        }
    }
    else
    {
    	//printf("ls_sock_tcp_connect_wait_do-18\n");
        ls_io_unset_nonblocking(fd);
        return LS_ERRNO_SYSCALL;
    }
}

uint32_t ls_sock_tcp_connect_wait(int *fd, const char *host_peer, const char *serv_peer, 
                                   const char *host_local, const char *serv_local, int timeout_ms)
{
    uint32_t ret = LS_ERRNO_NOTFND;
	int error = 0;
    struct addrinfo hints, *res_peer = NULL, *ressave_peer = NULL, *res_local = NULL, *ressave_local = NULL;
	
    if(NULL == fd || NULL == host_peer || NULL == serv_peer || NULL == host_local || NULL == serv_local)
        return LS_ERRNO_INVAL;
	
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	
    if(0 != getaddrinfo(host_peer, serv_peer, &hints, &res_peer)) return LS_ERRNO_SYSCALL;
    if(0 != getaddrinfo(host_local, serv_local, &hints, &res_local)) return LS_ERRNO_SYSCALL;
	
    ressave_peer = res_peer;
    ressave_local = res_local;
	
	//printf("host_peer=%s,serv_peer=%s,host_local=%s,serv_local=%s\n",host_peer,serv_peer,host_local,serv_local);
	//printf("res_peer=0x%08x,res_local=0x%08x\n",res_peer,res_local);
    
    do
    {
        do
        {
            if(0 > (*fd = socket(res_peer->ai_family, res_peer->ai_socktype, res_peer->ai_protocol)))
			{
				continue;
            }
			
			//printf("local_addr=0x%08x\n",*((unsigned int *)res_local->ai_addr->sa_data));
			
            if(0 != (error = ls_sock_tcp_connect_wait_do(*fd, res_peer->ai_addr, res_peer->ai_addrlen,
                                                 res_local->ai_addr, res_local->ai_addrlen, timeout_ms)))
            {
            	//printf("ls_sock_tcp_connect_wait_do failed\n");
                close(*fd);
				*fd = -1;//csp modify
                continue;
            }
			//printf("ls_sock_tcp_connect_wait_do success,local_addr=0x%08x\n",*((unsigned int *)res_local->ai_addr->sa_data));
			
            ret = LS_ERRNO_OK;
            break;
        }while(NULL != (res_local = res_local->ai_next));
		
        if(LS_ERRNO_OK == ret) break;
		
        res_local = ressave_local;
		
		//printf("peer_addr=0x%08x\n",*((unsigned int *)res_peer->ai_addr->sa_data));
    }while(NULL != (res_peer = res_peer->ai_next));
	
	freeaddrinfo(ressave_peer);
	freeaddrinfo(ressave_local);
	
	return ret;
}

uint32_t ls_sock_tcp_connect_nowait(int *fd, const char *host_peer, const char *serv_peer, 
                                     const char *host_local, const char *serv_local, uint8_t *connected)
{


    uint32_t        ret = LS_ERRNO_NOTFND;
    struct addrinfo hints, *res_peer, *ressave_peer, *res_local, *ressave_local;
    in_port_t       port_local;
    const int       on = 1;
    int             n;

    if(NULL == fd || NULL == host_peer || NULL == serv_peer || NULL == host_local || NULL == serv_local || NULL == connected)
        return LS_ERRNO_INVAL;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if(0 != getaddrinfo(host_peer, serv_peer, &hints, &res_peer)) return LS_ERRNO_SYSCALL;
    if(0 != getaddrinfo(host_local, serv_local, &hints, &res_local)) return LS_ERRNO_SYSCALL;
    ressave_peer = res_peer;
    ressave_local = res_local;

    *connected = 0;
    do
    {
        do
        {
            /* create socket */
            if((*fd = socket(res_peer->ai_family, res_peer->ai_socktype, res_peer->ai_protocol)) < 0) continue;

            /* set SO_REUSEADDR option if not use random port */
            ls_sock_ntop_port(res_local->ai_addr, &port_local);
            if(ntohs(port_local) > 0)
            {
                if(setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
                {
                    close(*fd);
                    continue;
                }
            }

            /* bind local addr */
            if(0 != bind(*fd, res_local->ai_addr, res_local->ai_addrlen))
            {
                close(*fd);
                continue;
            }
            
            /* set nonblocking */
            ls_io_set_nonblocking(*fd);

            /* do connect */
            if(0 == (n = connect(*fd, res_peer->ai_addr, res_peer->ai_addrlen)))
            {
                /* connect completed immediately */
                ls_io_unset_nonblocking(*fd);
                *connected = 1;
				
            }
            else if(n < 0)
            {
                if(errno == EINPROGRESS || errno == EINTR)
                {
                    /* the connection shall be established asynchronously */
                    *connected = 0;
                }
                else
                {
                    /* error */
                    close(*fd);
                    continue;
                }
            }
            else
            {
                /* error */
                close(*fd);
                continue;
            }
            
            ret = LS_ERRNO_OK;
            break;
        }
        while(NULL != (res_local = res_local->ai_next));

        if(LS_ERRNO_OK == ret) break;

        res_local = ressave_local;
    }
    while(NULL != (res_peer = res_peer->ai_next));
    
    freeaddrinfo(ressave_peer);
    freeaddrinfo(ressave_local);
    return ret;
}

uint32_t ls_sock_tcp_is_self_connection(int fd, uint8_t *is_self_connection)
{
    struct sockaddr_storage  local_addr;
    struct sockaddr_storage  peer_addr;
    struct sockaddr_in      *local_sin  = NULL;
    struct sockaddr_in      *peer_sin   = NULL;
    struct sockaddr_in6     *local_sin6 = NULL;
    struct sockaddr_in6     *peer_sin6  = NULL;
    uint32_t                 ret;
    
    if(fd < 0 || NULL == is_self_connection) return LS_ERRNO_INVAL;
    if(0 != (ret = ls_sock_get_local_sockaddr(fd, &local_addr))) return ret;
    if(0 != (ret = ls_sock_get_peer_sockaddr(fd, &peer_addr)))   return ret;

    *is_self_connection = 0; /* not self connection */

    if(local_addr.ss_family == peer_addr.ss_family)
    {
        switch(local_addr.ss_family)
        {
        case AF_INET:
            local_sin = (struct sockaddr_in *)(&local_addr);
            peer_sin = (struct sockaddr_in *)(&peer_addr);
            if(local_sin->sin_addr.s_addr == peer_sin->sin_addr.s_addr && local_sin->sin_port == peer_sin->sin_port)
            {
                *is_self_connection = 1; /* self connection */
            }
            break;
        case AF_INET6:
            local_sin6 = (struct sockaddr_in6 *)(&local_addr);
            peer_sin6 = (struct sockaddr_in6 *)(&peer_addr);
            if(0 == memcmp(local_sin6->sin6_addr.s6_addr, peer_sin6->sin6_addr.s6_addr, 16) &&
               local_sin6->sin6_port == peer_sin6->sin6_port)
            {
                *is_self_connection = 1; /* self connection */
            }
            break;
        default:
            return LS_ERRNO_NOTSPT;
        }
    }

    return LS_ERRNO_OK;
}

uint32_t ls_sock_tcp_listen(int *fd, const char *host, const char *serv, socklen_t *salen)
{
    uint32_t        ret = LS_ERRNO_OK;
    const int       on  = 1;
    struct addrinfo hints, *res, *ressave;

    if(NULL == fd || NULL == host || NULL == serv) return LS_ERRNO_INVAL;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if(0 != getaddrinfo(host, serv, &hints, &res)) return LS_ERRNO_SYSCALL;
    ressave = res;

    do
    {
        if(0 <= (*fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) &&
           0 == setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) &&
           0 == bind(*fd, res->ai_addr, res->ai_addrlen) &&
           0 == listen(*fd, SVX_SOCK_TCP_LISTEN_QUEUE_LEN) &&
           0 == ls_io_set_nonblocking(*fd))
        {
            if(salen) *salen = res->ai_addrlen;
            break;
        }
        else
            close(*fd);
    }
    while((res = res->ai_next) != NULL);

    if(NULL == res) ret = LS_ERRNO_NOTFND;
    freeaddrinfo(ressave);
    return ret;
}

uint32_t ls_sock_ntop_ip(struct sockaddr *sa, char *ip, size_t len)
{
    struct sockaddr_in  *sin  = NULL;
    struct sockaddr_in6 *sin6 = NULL;

    if(NULL == sa || NULL == ip || 0 == len) return LS_ERRNO_INVAL;
    
    memset(ip, 0, len);
    switch(sa->sa_family)
    {
    case AF_INET:
        {
            if(len < INET_ADDRSTRLEN) return LS_ERRNO_NOBUF;
            sin = (struct sockaddr_in *)sa;
            if(NULL == inet_ntop(AF_INET, &(sin->sin_addr), ip, (socklen_t)len)) return LS_ERRNO_SYSCALL;
            return LS_ERRNO_OK;
        }
    case AF_INET6:
        {
            if(len < INET6_ADDRSTRLEN) return LS_ERRNO_NOBUF;
            sin6 = (struct sockaddr_in6 *)sa;
            if(NULL == inet_ntop(AF_INET6, &(sin6->sin6_addr), ip, (socklen_t)len)) return LS_ERRNO_SYSCALL;
            return LS_ERRNO_OK;
        }
    default:
        return LS_ERRNO_NOTSPT;
    }
}

uint32_t ls_sock_ntop_port(struct sockaddr *sa, uint16_t *port)
{
    struct sockaddr_in  *sin  = NULL;
    struct sockaddr_in6 *sin6 = NULL;

    if(NULL == sa || NULL == port) return LS_ERRNO_INVAL;

    switch(sa->sa_family)
    {
    case AF_INET:
        {
            sin = (struct sockaddr_in *)sa;
            *port = ntohs(sin->sin_port);
            return LS_ERRNO_OK;
        }
    case AF_INET6:
        {
            sin6 = (struct sockaddr_in6 *)sa;
            *port = ntohs(sin6->sin6_port);
            return LS_ERRNO_OK;
        }
    default:
        return LS_ERRNO_NOTSPT;
    }
}

uint32_t ls_sock_ntop_addr(struct sockaddr *sa, char *addr, size_t len)
{
    struct sockaddr_in  *sin  = NULL;
    struct sockaddr_in6 *sin6 = NULL;
    size_t               n;

    if(NULL == sa || NULL == addr || 0 == len) return LS_ERRNO_INVAL;
    
    memset(addr, 0, len);
    switch(sa->sa_family)
    {
    case AF_INET:
        {
            if(len < INET_ADDRSTRLEN + sizeof("[]:65535") - 1) return LS_ERRNO_NOBUF;
            sin = (struct sockaddr_in *) sa;
            addr[0] = '[';
            if(NULL == inet_ntop(AF_INET, &sin->sin_addr, addr + 1, (socklen_t)(len - 1))) return LS_ERRNO_SYSCALL;
            n = strlen(addr);
            snprintf(addr + n, len - n, "]:%d", ntohs(sin->sin_port));
            return LS_ERRNO_OK;
        }
    case AF_INET6:
        {
            if(len < INET6_ADDRSTRLEN + sizeof("[]:65535") - 1) return LS_ERRNO_NOBUF;
            sin6 = (struct sockaddr_in6 *) sa;
            addr[0] = '[';
            if(NULL == inet_ntop(AF_INET6, &sin6->sin6_addr, addr + 1, (socklen_t)(len - 1))) return LS_ERRNO_SYSCALL;
            n = strlen(addr);
            snprintf(addr + n, len - n, "]:%d", ntohs(sin6->sin6_port));
            return LS_ERRNO_OK;
        }
    default:
        return LS_ERRNO_NOTSPT;
    }
}

uint32_t ls_sock_pton(char *host, char *serv, struct sockaddr_storage *sa)
{
    struct addrinfo hints, *res;

    if(NULL == host || NULL == serv || NULL == sa) return LS_ERRNO_INVAL;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = 0;

    if(0 != getaddrinfo(host, serv, &hints, &res)) return LS_ERRNO_SYSCALL;

    memcpy(sa, res->ai_addr, res->ai_addrlen);

    freeaddrinfo(res);
    return LS_ERRNO_OK;
}

uint32_t ls_sock_get_local_ip(int fd, char *ip, size_t len)
{
    struct sockaddr_storage local_addr;
    socklen_t               local_addr_len = sizeof(local_addr);

    if(fd < 0 || NULL == ip || 0 == len) return LS_ERRNO_INVAL;

    if(0 != getsockname(fd, (struct sockaddr *)&local_addr, &local_addr_len)) return LS_ERRNO_SYSCALL;

    return ls_sock_ntop_ip((struct sockaddr *)&local_addr, ip, len);
}

uint32_t ls_sock_get_local_port(int fd, uint16_t *port)
{
    struct sockaddr_storage local_addr;
    socklen_t               local_addr_len = sizeof(local_addr);

    if(fd < 0 || NULL == port) return LS_ERRNO_INVAL;

    if(0 != getsockname(fd, (struct sockaddr *)&local_addr, &local_addr_len)) return LS_ERRNO_SYSCALL;
    
    return ls_sock_ntop_port((struct sockaddr *)&local_addr, port);
}

uint32_t ls_sock_get_local_addr(int fd, char *addr, size_t len)
{
    struct sockaddr_storage local_addr;
    socklen_t               local_addr_len = sizeof(local_addr);

    if(fd < 0 || NULL == addr || 0 == len) return LS_ERRNO_INVAL;

    if(0 != getsockname(fd, (struct sockaddr *)&local_addr, &local_addr_len)) return LS_ERRNO_SYSCALL;

    return ls_sock_ntop_addr((struct sockaddr *)&local_addr, addr, len);
}

uint32_t ls_sock_get_local_sockaddr(int fd, struct sockaddr_storage *sockaddr)
{
    socklen_t sockaddr_len = sizeof(struct sockaddr_storage);

    if(fd < 0 || NULL == sockaddr) return LS_ERRNO_INVAL;

    if(0 != getsockname(fd, (struct sockaddr *)sockaddr, &sockaddr_len)) return LS_ERRNO_SYSCALL;

    return LS_ERRNO_OK;
}

uint32_t ls_sock_get_peer_ip(int fd, char *ip, size_t len)
{
    struct sockaddr_storage peer_addr;
    socklen_t               peer_addr_len = sizeof(peer_addr);

    if(fd < 0 || NULL == ip || 0 == len) return LS_ERRNO_INVAL;

    if(0 != getpeername(fd, (struct sockaddr *)&peer_addr, &peer_addr_len)) return LS_ERRNO_SYSCALL;

    return ls_sock_ntop_ip((struct sockaddr *)&peer_addr, ip, len);
}

uint32_t ls_sock_get_peer_port(int fd, uint16_t *port)
{
    struct sockaddr_storage peer_addr;
    socklen_t               peer_addr_len = sizeof(peer_addr);

    if(fd < 0 || NULL == port) return LS_ERRNO_INVAL;

    if(0 != getpeername(fd, (struct sockaddr *)&peer_addr, &peer_addr_len)) return LS_ERRNO_SYSCALL;

    return ls_sock_ntop_port((struct sockaddr *)&peer_addr, port);
}

uint32_t ls_sock_get_peer_addr(int fd, char *addr, size_t len)
{
    struct sockaddr_storage peer_addr;
    socklen_t               peer_addr_len = sizeof(peer_addr);
    
    if(fd < 0 || NULL == addr || 0 == len) return LS_ERRNO_INVAL;

    if(0 != getpeername(fd, (struct sockaddr *)&peer_addr, &peer_addr_len)) return LS_ERRNO_SYSCALL;

    return ls_sock_ntop_addr((struct sockaddr *)&peer_addr, addr, len);
}

uint32_t ls_sock_get_peer_sockaddr(int fd, struct sockaddr_storage *sockaddr)
{
    socklen_t sockaddr_len = sizeof(struct sockaddr_storage);

    if(fd < 0 || NULL == sockaddr) return LS_ERRNO_INVAL;

    if(0 != getpeername(fd, (struct sockaddr *)sockaddr, &sockaddr_len)) return LS_ERRNO_SYSCALL;

    return LS_ERRNO_OK;
}

uint32_t ls_sock_cmp_ip(struct sockaddr *sa1, struct sockaddr *sa2)
{
    struct sockaddr_in  *sin_1  = NULL;
    struct sockaddr_in  *sin_2  = NULL;
    struct sockaddr_in6 *sin6_1 = NULL;
    struct sockaddr_in6 *sin6_2 = NULL;

    if(NULL == sa1 || NULL == sa2) return LS_ERRNO_INVAL;
    if(sa1->sa_family != sa2->sa_family) return LS_ERRNO_INVAL;

    switch(sa1->sa_family)
    {
    case AF_INET:
        sin_1 = (struct sockaddr_in *)sa1;
        sin_2 = (struct sockaddr_in *)sa2;
        if(0 != memcmp(&sin_1->sin_addr, &sin_2->sin_addr, sizeof(struct in_addr))) return LS_ERRNO_NOTEQL;
        return LS_ERRNO_OK;
    case AF_INET6: 
        sin6_1 = (struct sockaddr_in6 *)sa1;
        sin6_2 = (struct sockaddr_in6 *)sa2;
        if(0 != memcmp(&sin6_1->sin6_addr, &sin6_2->sin6_addr, sizeof(struct in6_addr))) return LS_ERRNO_NOTEQL;
        return LS_ERRNO_OK;
    default:
        return LS_ERRNO_NOTSPT;
    }
}

uint32_t ls_sock_cmp_port(struct sockaddr *sa1, struct sockaddr *sa2)
{
    struct sockaddr_in  *sin_1  = NULL;
    struct sockaddr_in  *sin_2  = NULL;
    struct sockaddr_in6 *sin6_1 = NULL;
    struct sockaddr_in6 *sin6_2 = NULL;

    if(NULL == sa1 || NULL == sa2) return LS_ERRNO_INVAL;
    if(sa1->sa_family != sa2->sa_family) return LS_ERRNO_INVAL;

    switch(sa1->sa_family)
    {
    case AF_INET:
        sin_1 = (struct sockaddr_in *)sa1;
        sin_2 = (struct sockaddr_in *)sa2;
        if(sin_1->sin_port != sin_2->sin_port) return LS_ERRNO_NOTEQL;
        return LS_ERRNO_OK;
    case AF_INET6:
        sin6_1 = (struct sockaddr_in6 *)sa1;
        sin6_2 = (struct sockaddr_in6 *)sa2;
        if(sin6_1->sin6_port != sin6_2->sin6_port) return LS_ERRNO_NOTEQL;
        return LS_ERRNO_OK;
    default:
        return LS_ERRNO_NOTSPT;
    }
}

uint32_t ls_sock_cmp_addr(struct sockaddr *sa1, struct sockaddr *sa2)
{
    struct sockaddr_in  *sin_1  = NULL;
    struct sockaddr_in  *sin_2  = NULL;
    struct sockaddr_in6 *sin6_1 = NULL;
    struct sockaddr_in6 *sin6_2 = NULL;

    if(NULL == sa1 || NULL == sa2) return LS_ERRNO_INVAL;
    if(sa1->sa_family != sa2->sa_family) return LS_ERRNO_INVAL;

    switch(sa1->sa_family)
    {
    case AF_INET:
        sin_1 = (struct sockaddr_in *)sa1;
        sin_2 = (struct sockaddr_in *)sa2;
        if(0 != memcmp(&sin_1->sin_addr, &sin_2->sin_addr, sizeof(struct in_addr))) return LS_ERRNO_NOTEQL;
        if(sin_1->sin_port != sin_2->sin_port) return LS_ERRNO_NOTEQL;
        return LS_ERRNO_OK;
    case AF_INET6: 
        sin6_1 = (struct sockaddr_in6 *)sa1;
        sin6_2 = (struct sockaddr_in6 *)sa2;
        if(0 != memcmp(&sin6_1->sin6_addr, &sin6_2->sin6_addr, sizeof(struct in6_addr))) return LS_ERRNO_NOTEQL;
        if(sin6_1->sin6_port != sin6_2->sin6_port) return LS_ERRNO_NOTEQL;
        return LS_ERRNO_OK;
    default:
        return LS_ERRNO_NOTSPT;
    }
}

/*
 * A> common UDP server: pass on host: "0.0.0.0" or "0::0", serv: the well-known port.
 * B> receiving multicast: pass on the multicast address and port.
 *                         (don't forget to call ls_sock_mcast_join after calling this function)
 */
uint32_t ls_sock_udp_server(int *fd, const char *host, const char *serv, socklen_t *salen)
{
    uint32_t        ret = LS_ERRNO_OK;
    struct addrinfo hints, *res, *ressave;
    const int       on = 1;

    if(NULL == fd || NULL == host || NULL == serv) return LS_ERRNO_INVAL;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if(0 != getaddrinfo(host, serv, &hints, &res)) return LS_ERRNO_SYSCALL;
    ressave = res;

    do
    {
        if(0 <= (*fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) &&
           0 == setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) &&
           0 == bind(*fd, res->ai_addr, res->ai_addrlen))
        {
            if(salen) *salen = res->ai_addrlen;
            break;
        }
        else
            close(*fd);
    }
    while(NULL != (res = res->ai_next));

    if(NULL == res) ret = LS_ERRNO_NOTFND;
    freeaddrinfo(ressave);
    return ret;
}

static uint32_t ls_sock_udp_client_inner(int *fd, const char *host_peer, const char *serv_peer,
                                          const char *host_local, const char *serv_local, 
                                          struct sockaddr_storage *sa_peer, socklen_t *salen_peer, uint8_t if_connect)
{
    uint32_t        ret = LS_ERRNO_NOTFND;
    struct addrinfo hints, *res_peer, *ressave_peer, *res_local, *ressave_local;
    in_port_t       port_local;
    const int       on = 1;
	
    if(NULL == fd || NULL == host_peer || NULL == serv_peer || NULL == host_local || NULL == serv_local)
        return LS_ERRNO_INVAL;
	
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
	
    if(0 != getaddrinfo(host_peer, serv_peer, &hints, &res_peer)) return LS_ERRNO_SYSCALL;
    if(0 != getaddrinfo(host_local, serv_local, &hints, &res_local)) return LS_ERRNO_SYSCALL;
	
	ressave_peer = res_peer;
    ressave_local = res_local;
	
	//printf("modify bind param...\n");
	
    do
    {
        do
        {
			/* create socket */
			if((*fd = socket(res_peer->ai_family, res_peer->ai_socktype, res_peer->ai_protocol)) < 0)
			{
				continue;
			}
			
            /* set SO_REUSEADDR option if not use random port */
            ls_sock_ntop_port(res_local->ai_addr, &port_local);
            if(ntohs(port_local) > 0)
            {
                if(setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
                {
                    close(*fd);
					*fd = -1;//csp modify
                    continue;
                }
            }
			
            /* bind local addr */
			//csp modify
			//struct sockaddr_in local;
			//local.sin_family = AF_INET;
			//local.sin_port = htons(atoi(serv_local));
			//local.sin_addr.s_addr = inet_addr(host_local);
			//if(0 != bind(*fd, (struct sockaddr *)&local, sizeof(local)))
			if(0 != bind(*fd, res_local->ai_addr, res_local->ai_addrlen))
			{
				close(*fd);
				*fd = -1;//csp modify
				continue;
			}
			
            if(if_connect)
            {
                /* connect to peer */
                if(0 != connect(*fd, res_peer->ai_addr, res_peer->ai_addrlen))
                {
                    close(*fd);
					*fd = -1;//csp modify
                    continue;
                }
            }
			
            /* return peer sockaddr */
            if(sa_peer)    memcpy(sa_peer, res_peer->ai_addr, res_peer->ai_addrlen);
            if(salen_peer) *salen_peer = res_peer->ai_addrlen;
			
            ret = LS_ERRNO_OK;
            break;
        }while(NULL != (res_local = res_local->ai_next));//while(0);//csp modify
		
        if(LS_ERRNO_OK == ret) break;
		
        res_local = ressave_local;
    }
    while(NULL != (res_peer = res_peer->ai_next));
	
    freeaddrinfo(ressave_peer);
    freeaddrinfo(ressave_local);
	
    return ret;
}

uint32_t ls_sock_udp_client(int *fd, const char *host_peer, const char *serv_peer, 
                             const char *host_local, const char *serv_local,
                             struct sockaddr_storage *sa_peer, socklen_t *salen_peer)
{
    return ls_sock_udp_client_inner(fd, host_peer, serv_peer, host_local, serv_local, sa_peer, salen_peer, 0);
}

uint32_t ls_sock_udp_client_connect(int *fd, const char *host_peer, const char *serv_peer,
                                     const char *host_local, const char *serv_local,
                                     struct sockaddr_storage *sa_peer, socklen_t *salen_peer)
{
    return ls_sock_udp_client_inner(fd, host_peer, serv_peer, host_local, serv_local, sa_peer, salen_peer, 1);
}

uint32_t ls_sock_get_local_ip_by_peer(char *host_peer, char *ip, size_t len)
{
    uint32_t        ret = LS_ERRNO_OK;
    int             fd;
    struct addrinfo hints, *res, *ressave;

    if(NULL == host_peer || NULL == ip || 0 == len) return LS_ERRNO_INVAL;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if(0 != getaddrinfo(host_peer, "0", &hints, &res)) return LS_ERRNO_SYSCALL;
    ressave = res;

    do
    {
        if(0 <= (fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) &&
           0 == connect(fd, res->ai_addr, res->ai_addrlen) &&
           0 == ls_sock_get_local_ip(fd, ip, len))
        {
            close(fd);
            break;
        }
        else
            close(fd);
    }
    while(NULL != (res = res->ai_next));

    if(NULL == res) ret = LS_ERRNO_NOTFND;
    freeaddrinfo(ressave);
    return ret;
}

uint32_t ls_sock_mcast_join(int fd, const struct sockaddr *grp, socklen_t grplen, const char *ifname, uint32_t ifindex)
{
    struct group_req req;

    if(fd < 0 || NULL == grp || 0 == grplen) return LS_ERRNO_INVAL;

    memset(&req, 0, sizeof(req));

    if(ifindex > 0)
    {
        req.gr_interface = ifindex;
    }
    else if (NULL != ifname)
    {
        if(0 == (req.gr_interface = if_nametoindex(ifname)))
        {
            errno = ENXIO; /* if name not found */
            return LS_ERRNO_NOTFND;
        }
    }
    else
    {
        req.gr_interface = 0;
    }

    if(grplen > sizeof(req.gr_group))
    {
        errno = EINVAL;
        return LS_ERRNO_INVAL;
    }
    memcpy(&req.gr_group, grp, grplen);

    if(0 != setsockopt(fd, ls_sock_family_to_level(grp->sa_family), MCAST_JOIN_GROUP, &req, sizeof(req)))
        return LS_ERRNO_SYSCALL;

    return LS_ERRNO_OK;
}

uint32_t ls_sock_mcast_leave(int fd, const struct sockaddr *grp, socklen_t grplen)
{
    struct group_req req;

    if(fd < 0 || NULL == grp || 0 == grplen) return LS_ERRNO_INVAL;

    memset(&req, 0, sizeof(req));
    req.gr_interface = 0;

    if (grplen > sizeof(req.gr_group))
    {
        errno = EINVAL;
        return LS_ERRNO_INVAL;
    }
    memcpy(&req.gr_group, grp, grplen);

    if(0 != setsockopt(fd, ls_sock_family_to_level(grp->sa_family), MCAST_LEAVE_GROUP, &req, sizeof(req)))
        return LS_ERRNO_SYSCALL;

    return LS_ERRNO_OK;
}

uint32_t ls_sock_mcast_join_source_group(int fd, const struct sockaddr *src, socklen_t srclen,
                                          const struct sockaddr *grp, socklen_t grplen,
                                          const char *ifname, uint32_t ifindex)
{
    struct group_source_req req;

    if(fd < 0 || NULL == src || NULL == grp) return LS_ERRNO_INVAL;

    memset(&req, 0, sizeof(req));

    if(ifindex > 0)
    {
        req.gsr_interface = ifindex;
    }
    else if(NULL != ifname)
    {
        if(0 == (req.gsr_interface = if_nametoindex(ifname)))
        {
            errno = ENXIO; /* if name not found */
            return LS_ERRNO_NOTFND;
        }
    }
    else
    {
        req.gsr_interface = 0;
    }

    if(grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source))
    {
        errno = EINVAL;
        return LS_ERRNO_INVAL;
    }
    memcpy(&req.gsr_group, grp, grplen);
    memcpy(&req.gsr_source, src, srclen);

    if(0 != setsockopt(fd, ls_sock_family_to_level(grp->sa_family), MCAST_JOIN_SOURCE_GROUP, &req, sizeof(req)))
        return LS_ERRNO_SYSCALL;

    return LS_ERRNO_OK;
}

uint32_t ls_sock_mcast_leave_source_group(int fd, const struct sockaddr *src, socklen_t srclen,
                                           const struct sockaddr *grp, socklen_t grplen)
{
    struct group_source_req req;

    if(fd < 0 || NULL == src || NULL == grp) return LS_ERRNO_INVAL;

    memset(&req, 0, sizeof(req));

    req.gsr_interface = 0;
    if(grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source))
    {
        errno = EINVAL;
        return LS_ERRNO_INVAL;
    }
    memcpy(&req.gsr_group, grp, grplen);
    memcpy(&req.gsr_source, src, srclen);

    if(0 != setsockopt(fd, ls_sock_family_to_level(grp->sa_family), MCAST_LEAVE_SOURCE_GROUP, &req, sizeof(req)))
        return LS_ERRNO_SYSCALL;

    return LS_ERRNO_OK;
}

uint32_t ls_sock_mcast_block_source(int fd, const struct sockaddr *src, socklen_t srclen,
                                     const struct sockaddr *grp, socklen_t grplen)
{
    struct group_source_req req;

    if(fd < 0 || NULL == src || NULL == grp) return LS_ERRNO_INVAL;

    memset(&req, 0, sizeof(req));

    req.gsr_interface = 0;
    if(grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source))
    {
        errno = EINVAL;
        return LS_ERRNO_INVAL;
    }
    memcpy(&req.gsr_group, grp, grplen);
    memcpy(&req.gsr_source, src, srclen);

    if(0 != setsockopt(fd, ls_sock_family_to_level(grp->sa_family), MCAST_BLOCK_SOURCE, &req, sizeof(req)))
        return LS_ERRNO_SYSCALL;

    return LS_ERRNO_OK;
}

uint32_t ls_sock_mcast_unblock_source(int fd, const struct sockaddr *src, socklen_t srclen,
                                       const struct sockaddr *grp, socklen_t grplen)
{
    struct group_source_req req;

    if(fd < 0 || NULL == src || NULL == grp) return LS_ERRNO_INVAL;

    memset(&req, 0, sizeof(req));

    req.gsr_interface = 0;
    if(grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source))
    {
        errno = EINVAL;
        return LS_ERRNO_INVAL;
    }
    memcpy(&req.gsr_group, grp, grplen);
    memcpy(&req.gsr_source, src, srclen);

    if(0 != setsockopt(fd, ls_sock_family_to_level(grp->sa_family), MCAST_UNBLOCK_SOURCE, &req, sizeof(req)))
        return LS_ERRNO_SYSCALL;

    return LS_ERRNO_OK;
}

uint32_t ls_sock_mcast_get_ttl(int fd, int *ttl)
{
    if(fd < 0 || NULL == ttl) return LS_ERRNO_INVAL;

    switch(ls_sock_fd_to_family(fd))
    {
    case AF_INET:
        {
            u_char    val;
            socklen_t len = sizeof(val);   
            if(0 != getsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &val, &len)) return LS_ERRNO_SYSCALL;
            *ttl = (int)val;
            return LS_ERRNO_OK;
        }
    case AF_INET6:
        {
            int       val;
            socklen_t len = sizeof(val);
            if(0 != getsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &val, &len)) return LS_ERRNO_SYSCALL;
            *ttl = val;
            return LS_ERRNO_OK;
        }
    default:
        {
            errno = EAFNOSUPPORT;
            return LS_ERRNO_NOTSPT;
        }
    }
}

uint32_t ls_sock_mcast_set_ttl(int fd, int ttl)
{
    if(fd < 0 || ttl < 0) return LS_ERRNO_INVAL;

    switch(ls_sock_fd_to_family(fd))
    {
    case AF_INET:
        {
            u_char val = (u_char)ttl;
            if(0 != setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &val, sizeof(val))) return LS_ERRNO_SYSCALL;
            return LS_ERRNO_OK;
        }
    case AF_INET6:
        {
            int val = ttl;
            if(0 != setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &val, sizeof(val))) return LS_ERRNO_SYSCALL;
            return LS_ERRNO_OK;
        }
    default:
        {
            errno = EAFNOSUPPORT;
            return LS_ERRNO_NOTSPT;
        }
    }
}

uint32_t ls_sock_mcast_get_if(int fd, uint32_t *ifindex)
{
    if(fd < 0 || NULL == ifindex) return LS_ERRNO_INVAL;

    switch(ls_sock_fd_to_family(fd))
    {
    case AF_INET:
        {
            struct in_addr      inaddr;
            socklen_t           len = sizeof(inaddr);
            struct ifaddrs     *ifaddr, *ifa;
            struct sockaddr_in *sa;
            uint32_t            ret = LS_ERRNO_NOTFND;

            /* get in_addr from fd */
            if(0 != getsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &inaddr, &len)) return LS_ERRNO_SYSCALL;

            /* get ifname from in_addr, get ifindex from ifname */
            if(0 != getifaddrs(&ifaddr)) return LS_ERRNO_SYSCALL;
            for(ifa = ifaddr; NULL != ifa; ifa = ifa->ifa_next)
            {
                if(ifa->ifa_addr && (ifa->ifa_flags & IFF_UP) && AF_INET == ifa->ifa_addr->sa_family)
                {
                    sa = (struct sockaddr_in *)(ifa->ifa_addr);
                    if(0 == memcmp(&(sa->sin_addr), &inaddr, sizeof(struct in_addr)))
                    {
                        if(0 != (*ifindex = if_nametoindex(ifa->ifa_name)))
                        {
                            ret = LS_ERRNO_OK; /* got it */
                            break;
                        }
                    }
                }
            }
            freeifaddrs(ifaddr);
            return ret;
        }
    case AF_INET6:
        {
            u_int     idx;
            socklen_t len = sizeof(idx);
            if(0 != getsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &idx, &len)) return LS_ERRNO_SYSCALL;
            *ifindex = (uint32_t)idx;
            return LS_ERRNO_OK;
        }
    default:
        {
            errno = EAFNOSUPPORT;
            return LS_ERRNO_NOTSPT;
        }
    }
}

uint32_t ls_sock_mcast_set_if(int fd, const char *ifname, uint32_t ifindex)
{
    if(fd < 0) return LS_ERRNO_INVAL;

    switch(ls_sock_fd_to_family(fd))
    {
    case AF_INET:
        {
            struct in_addr inaddr;
            struct ifreq   ifreq;

            if(ifindex > 0)
            {
                if(NULL == if_indextoname(ifindex, ifreq.ifr_name))
                {
                    errno = ENXIO; /* if index not found */
                    return LS_ERRNO_NOTFND;
                }
                goto doioctl;
            }
            else if(NULL != ifname)
            {
                strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
            doioctl:
                if(ioctl(fd, SIOCGIFADDR, &ifreq) < 0) return LS_ERRNO_SYSCALL;
                memcpy(&inaddr, &((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr, sizeof(struct in_addr));
            }
            else
            {
                inaddr.s_addr = htonl(INADDR_ANY); /* remove prev. set default */
            }

            if(0 != setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &inaddr, sizeof(struct in_addr)))
                return LS_ERRNO_SYSCALL;
            
            return LS_ERRNO_OK;
        }
    case AF_INET6:
        {
            u_int idx = ifindex;
            
            if(0 == idx)
            {
                if(NULL == ifname)
                {
                    errno = EINVAL; /* must supply either index or name */
                    return LS_ERRNO_INVAL;
                }
                if(0 == (idx = if_nametoindex(ifname)))
                {
                    errno = ENXIO; /* if name not found */
                    return LS_ERRNO_NOTFND;
                }
            }
            if(0 != setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &idx, sizeof(idx)))
                return LS_ERRNO_SYSCALL;
            
            return LS_ERRNO_OK;
        }
    default:
        {
            errno = EAFNOSUPPORT;
            return LS_ERRNO_NOTSPT;
        }
    }
}

uint32_t ls_sock_mcast_get_loop(int fd, unsigned int *loop)
{
    if(fd < 0 || NULL == loop) return LS_ERRNO_INVAL;

    switch(ls_sock_fd_to_family(fd))
    {
    case AF_INET:
        {
            u_char val;
            socklen_t len = sizeof(val);
            if(0 != getsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &val, &len)) return LS_ERRNO_SYSCALL;
            *loop = (unsigned int)val;
            return LS_ERRNO_OK;
        }
    case AF_INET6:
        {
            u_int val;
            socklen_t len = sizeof(val);
            if(0 != getsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &val, &len)) return LS_ERRNO_SYSCALL;
            *loop = (unsigned int)val;
            return LS_ERRNO_OK;
        }
    default:
        {
            errno = EAFNOSUPPORT;
            return LS_ERRNO_NOTSPT;
        }
    }
}

uint32_t ls_sock_mcast_set_loop(int fd, unsigned int loop)
{
    if(fd < 0) return LS_ERRNO_INVAL;

    switch(ls_sock_fd_to_family(fd))
    {
    case AF_INET:
        {
            u_char val = (u_char)loop;
            if(0 != setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &val, sizeof(val))) return LS_ERRNO_SYSCALL;
            return LS_ERRNO_OK;
        }
    case AF_INET6:
        {
            u_int val = (u_int)loop;
            if(0 != setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &val, sizeof(val))) return LS_ERRNO_SYSCALL;
            return LS_ERRNO_OK;
        }
    default:
        {
            errno = EAFNOSUPPORT;
            return LS_ERRNO_NOTSPT;
        }
    }
}

uint32_t ls_sock_icmp(int *fd, const char *host, const char *serv, struct sockaddr_storage *sa, socklen_t *salen)
{
    uint32_t        ret = LS_ERRNO_OK;
    struct addrinfo hints, *res, *ressave;

    if(NULL == fd || NULL == host || NULL == serv) return LS_ERRNO_INVAL;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = 0;

    if(0 != getaddrinfo(host, serv, &hints, &res)) return LS_ERRNO_SYSCALL;
    ressave = res;

    do
    {
        switch(res->ai_family)
        {
        case AF_INET:
            *fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
            break;
        case AF_INET6:
            *fd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
            break;
        default:
            *fd = -1;
            break;
        }

        if(*fd >= 0)
            break; /* success */
    }
    while(NULL != (res = res->ai_next));

    if(NULL == res)
        ret = LS_ERRNO_NOTFND;
    else
    {
        /* return address and address length */
        if(NULL != sa)    memcpy(sa, res->ai_addr, res->ai_addrlen);
        if(NULL != salen) *salen = res->ai_addrlen;
    }

    freeaddrinfo(ressave);
    return ret;
}

uint32_t ls_sock_icmp_echo_init(int fd, struct sockaddr_storage *sa)
{
    struct icmp6_filter filt;

    if(fd < 0 || NULL == sa) return LS_ERRNO_INVAL;

    switch(sa->ss_family)
    {
    case AF_INET:
        return LS_ERRNO_OK;
    case AF_INET6:
        ICMP6_FILTER_SETBLOCKALL(&filt);
        ICMP6_FILTER_SETPASS(ICMP6_ECHO_REPLY, &filt);
        if(0 != setsockopt(fd, SOL_ICMPV6, ICMP6_FILTER, &filt, sizeof(filt))) return LS_ERRNO_SYSCALL;
        return LS_ERRNO_OK;
    default:
        return LS_ERRNO_NOTSPT;
    }
}

uint32_t ls_sock_icmp_echo_send(int fd, struct sockaddr_storage *sa)
{
    static uint16_t  nsent = 0;
    struct icmp      icmp;
    struct icmp6_hdr icmp6;

    if(fd < 0 || NULL == sa) return LS_ERRNO_INVAL;

    switch(sa->ss_family)
    {
    case AF_INET:
        icmp.icmp_type = ICMP_ECHO;
        icmp.icmp_code = 0;
        icmp.icmp_id = getpid();
        icmp.icmp_seq = nsent++;
        icmp.icmp_cksum = 0;
        icmp.icmp_cksum = ls_sock_checksum((uint16_t *)(&icmp), SVX_SOCK_ICMP_ECHO_HEAD_LEN);
        if(0 > sendto(fd, &icmp, SVX_SOCK_ICMP_ECHO_HEAD_LEN, 0, (struct sockaddr *)sa, sizeof(struct sockaddr_in)))
            return LS_ERRNO_SYSCALL;
        return LS_ERRNO_OK;
    case AF_INET6:
        icmp6.icmp6_type = ICMP6_ECHO_REQUEST;
        icmp6.icmp6_code = 0;
        icmp6.icmp6_id = getpid();
        icmp6.icmp6_seq = nsent++;
        if(0 > sendto(fd, &icmp6, SVX_SOCK_ICMP_ECHO_HEAD_LEN, 0, (struct sockaddr *)sa, sizeof(struct sockaddr_in6)))
            return LS_ERRNO_SYSCALL;
        return LS_ERRNO_OK;
    default:
        return LS_ERRNO_NOTSPT;
    }
}

uint32_t ls_sock_icmp_echo_recv(int fd, struct sockaddr_storage *sa)
{
    char      buf[128];
    socklen_t len = sizeof(struct sockaddr_storage);
    ssize_t   n;

    if(fd < 0 || NULL == sa) return LS_ERRNO_INVAL;

    if((n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)sa, &len)) <= 0) return LS_ERRNO_SYSCALL;
    
    switch(sa->ss_family)
    {
    case AF_INET:
        {
            struct ip *ip;
            struct icmp *icmp;
            int ip_head_len, icmp_len;
            ip = (struct ip *)buf;                                              /* start of IP header */
            if(IPPROTO_ICMP != ip->ip_p) return LS_ERRNO_NOTFND;               /* not ICMP */
            ip_head_len = ip->ip_hl * 4;                                        /* length of IP header */
            icmp = (struct icmp *)(buf + ip_head_len);                          /* start of ICMP header */
            icmp_len = n - ip_head_len;                                         /* ICMP package length */
            if(icmp_len < SVX_SOCK_ICMP_ECHO_HEAD_LEN) return LS_ERRNO_NOTFND; /* malformed packet */
            if(ICMP_ECHOREPLY != icmp->icmp_type) return LS_ERRNO_NOTFND;      /* not ICMP ping reply */
            if(getpid() != icmp->icmp_id) return LS_ERRNO_NOTFND;              /* not our process */
            return LS_ERRNO_OK;                                                /* OK */
        }
    case AF_INET6:
        {
            struct icmp6_hdr *icmp6;
            icmp6 = (struct icmp6_hdr *)buf;                                   /* start of ICMPv6 header */
            if(n < SVX_SOCK_ICMP_ECHO_HEAD_LEN) return LS_ERRNO_NOTFND;       /* malformed packet */
            if(ICMP6_ECHO_REPLY != icmp6->icmp6_type) return LS_ERRNO_NOTFND; /* not ICMPv6 ping reply */
            if(getpid() != icmp6->icmp6_id) return LS_ERRNO_NOTFND;           /* not our process */
            return LS_ERRNO_OK;                                               /* OK */
        }
    default:
        return LS_ERRNO_NOTSPT;
    }
}

uint64_t ls_sock_hton_uint64(uint64_t v)
{
    union {uint32_t u32[2]; uint64_t u64;} u;
    u.u32[0] = htonl(v >> 32);
    u.u32[1] = htonl((v << 32) >> 32);
    return u.u64;
}

uint64_t ls_sock_ntoh_uint64(uint64_t v)
{
    union {uint32_t u32[2]; uint64_t u64;} u;
    u.u64 = v;
    return ((uint64_t)ntohl(u.u32[0]) << 32) | (uint64_t)ntohl(u.u32[1]);
}

uint32_t ls_sock_get_rcvbuf(int fd, size_t *buf_len)
{
    socklen_t buf_len_size = sizeof(size_t);

    if(fd < 0 || NULL == buf_len) return LS_ERRNO_INVAL;
    
    if(0 != getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void *)buf_len, &buf_len_size)) return LS_ERRNO_SYSCALL;

    return LS_ERRNO_OK;
}

uint32_t ls_env_if_root()
{
	char p[] = "USER";
    if( strcmp(getenv(p),"root") )
		return 0;
	else 
		return 1;
}
uint32_t ls_sock_set_rcvbuf(int fd, size_t buf_len)
{
    size_t    real_buf_len      = 0;
    socklen_t real_buf_len_size = sizeof(size_t);
    uint32_t  if_root           = ls_env_if_root();

    if(fd < 0 || 0 == buf_len) return LS_ERRNO_INVAL;

    if(0 != setsockopt(fd, SOL_SOCKET, if_root ? SO_RCVBUFFORCE : SO_RCVBUF, &(buf_len), sizeof(size_t))) return LS_ERRNO_SYSCALL;
    if(0 != getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void *)&real_buf_len, &real_buf_len_size)) return LS_ERRNO_SYSCALL;
    
    if(real_buf_len < buf_len)
    {
        lsprint("ls_sock_set_rcvbuf failed. Use %s. Want %zu. Got %zu.\n",
                if_root ? "SO_RCVBUFFORCE" : "SO_RCVBUF", buf_len, real_buf_len);

        if(!if_root)
        {
            lsprint("Please use the ROOT user, "
                    "or add the line \"net.core.rmem_max = %zu\" to /etc/sysctl.conf.\n", buf_len);
        }
        return LS_ERRNO_NOTSPT;
    }

    return LS_ERRNO_OK;
}

uint32_t ls_sock_get_sndbuf(int fd, size_t *buf_len)
{
    socklen_t buf_len_size = sizeof(size_t);

    if(fd < 0 || NULL == buf_len) return LS_ERRNO_INVAL;

    if(0 != getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *)buf_len, &buf_len_size)) return LS_ERRNO_SYSCALL;

    return LS_ERRNO_OK;
}







uint32_t ls_sock_set_sndbuf(int fd, size_t buf_len)
{
    size_t    real_buf_len      = 0;
    socklen_t real_buf_len_size = sizeof(size_t);
    uint32_t  if_root           = ls_env_if_root();

    if(fd < 0 || 0 == buf_len) return LS_ERRNO_INVAL;

    if(0 != setsockopt(fd, SOL_SOCKET, if_root ? SO_SNDBUFFORCE : SO_SNDBUF, &(buf_len), sizeof(size_t))) return LS_ERRNO_SYSCALL;
    if(0 != getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *)&real_buf_len, &real_buf_len_size)) return LS_ERRNO_SYSCALL;
    
    if(real_buf_len < buf_len)
    {
        lsprint( "ls_sock_set_sndbuf failed. Use %s. Want %zu. Got %zu.\n",
                if_root ? "SO_SNDBUFFORCE" : "SO_SNDBUF", buf_len, real_buf_len);

        if(!if_root)
        {
            lsprint("Please use the ROOT user, "
                    "or add the line \"net.core.wmem_max = %zu\" to /etc/sysctl.conf.\n", buf_len);
        }
        return LS_ERRNO_NOTSPT;
    }

    return LS_ERRNO_OK;
}

uint32_t ls_sock_tcp_get_keepalive(int fd, int *alive, int *idle_s, int *intvl_s, int *cnt)
{
    socklen_t alive_size   = sizeof(int);
    socklen_t idle_s_size  = sizeof(int);
    socklen_t intvl_s_size = sizeof(int);
    socklen_t cnt_size     = sizeof(int);
    
    if(fd < 0 || NULL == alive || NULL == idle_s || NULL == intvl_s || NULL == cnt) return LS_ERRNO_INVAL;

    if(0 != getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, alive, &alive_size)) return LS_ERRNO_SYSCALL;
    if(0 != getsockopt(fd, SOL_TCP, TCP_KEEPIDLE, idle_s, &idle_s_size)) return LS_ERRNO_SYSCALL;
    if(0 != getsockopt(fd, SOL_TCP, TCP_KEEPINTVL, intvl_s, &intvl_s_size)) return LS_ERRNO_SYSCALL;
    if(0 != getsockopt(fd, SOL_TCP, TCP_KEEPCNT, cnt, &cnt_size)) return LS_ERRNO_SYSCALL;

    return LS_ERRNO_OK;
}

uint32_t ls_sock_tcp_set_keepalive_on(int fd, int idle_s, int intvl_s, int cnt)
{
    int alive = 1;

    if(fd < 0 || idle_s < 0 || intvl_s < 0 || cnt < 0) return LS_ERRNO_INVAL;

    if(0 != setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &alive, sizeof(alive))) return LS_ERRNO_SYSCALL;
    if(0 != setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &idle_s, sizeof(idle_s))) return LS_ERRNO_SYSCALL;
    if(0 != setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &intvl_s, sizeof(intvl_s))) return LS_ERRNO_SYSCALL;
    if(0 != setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt))) return LS_ERRNO_SYSCALL;

    return LS_ERRNO_OK;
}

uint32_t ls_sock_tcp_set_keepalive_off(int fd)
{
    int alive = 0;

    if(fd < 0) return LS_ERRNO_INVAL;

    if(0 != setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &alive, sizeof(alive))) return LS_ERRNO_SYSCALL;

    return LS_ERRNO_OK;
}

uint32_t ls_sock_udp_rtp_rtcp_server(int *rtp_fd, int *rtcp_fd, const char *host, const char *rtp_port,
                                      uint16_t *ret_rtp_port, uint16_t *ret_rtcp_port, socklen_t *salen)
{
    static uint16_t rtp_port_seed = SVX_SOCK_RTP_PORT_MIN;
    uint16_t        rtp_port_i;
    uint16_t        rtcp_port_i;
    char            rtp_port_str[6];
    char            rtcp_port_str[6];
    uint8_t         if_create_success  = 0;
    uint32_t        create_loop_max = 25000;
    uint32_t        ret;

    if(NULL == rtp_fd || NULL == rtcp_fd || NULL == host || NULL == rtp_port) return LS_ERRNO_INVAL;

    if(0 == strcmp(rtp_port, "0"))
    {
        /* ramdon local port */
        while(!if_create_success && (create_loop_max--) > 0)
        {
            rtp_port_i  = rtp_port_seed;
            rtcp_port_i = rtp_port_seed + 1;
            rtp_port_seed += 2;
            if(rtp_port_seed >= SVX_SOCK_RTP_PORT_MAX)
                rtp_port_seed = SVX_SOCK_RTP_PORT_MIN;

        //    snprintf(rtp_port_str, sizeof(rtp_port_str), "%"PRIu16, rtp_port_i);
          //  snprintf(rtcp_port_str, sizeof(rtcp_port_str), "%"PRIu16, rtcp_port_i);
            
            if(0 == ls_sock_udp_server(rtp_fd, host, rtp_port_str, salen) &&
               0 == ls_io_set_nonblocking(*rtp_fd))
            {
                if(0 == ls_sock_udp_server(rtcp_fd, host, rtcp_port_str, NULL) &&
                   0 == ls_io_set_nonblocking(*rtcp_fd))
                {
                    if(ret_rtp_port)  *ret_rtp_port  = rtp_port_i;
                    if(ret_rtcp_port) *ret_rtcp_port = rtcp_port_i;
                    return LS_ERRNO_OK;
                }
                else
                {
                    close(*rtp_fd);
                }
            }
        }
    }
    else
    {
        /* constant local port */
        if(0 != (ret = ls_str_to_uint16(rtp_port, &rtp_port_i))) return ret;
        if(0 != rtp_port_i % 2) return LS_ERRNO_INVAL; /* RTP client port MUST be even */
        rtcp_port_i = rtp_port_i + 1;

      //  snprintf(rtp_port_str, sizeof(rtp_port_str), "%"PRIu16, rtp_port_i);
       // snprintf(rtcp_port_str, sizeof(rtcp_port_str), "%"PRIu16, rtcp_port_i);

        if(0 == ls_sock_udp_server(rtp_fd, host, rtp_port_str, salen) &&
           0 == ls_io_set_nonblocking(*rtp_fd))
        {
            if(0 == ls_sock_udp_server(rtcp_fd, host, rtcp_port_str, NULL) &&
               0 == ls_io_set_nonblocking(*rtcp_fd))
            {
                if(ret_rtp_port)  *ret_rtp_port  = rtp_port_i;
                if(ret_rtcp_port) *ret_rtcp_port = rtcp_port_i;
                return LS_ERRNO_OK;
            }
            else
            {
                close(*rtp_fd);
            }
        }
    }

    *rtp_fd  = -1;
    *rtcp_fd = -1;
    return LS_ERRNO_NOTFND;
}





int32_t ls_read_all (int fd,  char *buf, size_t len)
{
	int32_t m;
	size_t n ;
	uint32_t length;
	long flags;
	char *fbuf = NULL;
	char *p = buf;
	//char tmp[1024];
	struct timeval timeout={1,0}; 

	//char endflag[5];
	//int endflagsize = 0;
	flags = fcntl (fd, F_GETFL);
	fcntl (fd, F_SETFL, flags  & ~O_NONBLOCK);

	
	m = 0;
	n = 0;
	length = 0;
	setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
	while(1)
	{
		m = read(fd,buf+n,1024);
		if( m <=0 )
			break;
		n += m;
		if(fbuf == NULL)
		{
			fbuf = strstr(buf,"Content-Length");
			if( fbuf != NULL )
			{
				while( !isdigit(*fbuf) ) ++fbuf;
				ls_str_to_uint(fbuf,&length);
				length += fbuf -buf;
				p = fbuf;
				while ( (p < buf + n ) ) 
				{
					if( p + 4 < buf + n  && p[0] == '\r' &&p[1] == '\n' && p[2] == '\r' && p[3] == '\n')
					{
							length += 4;
							break;
					}
					++p;
					++length;
				}
				//while( *p++ != '<' &&  ) ++length;
			}
		}
		else
		{
			lsprint("%d:%d\n",n,length);
			if ( n >= length ) 
				break;
		}
		
	}
	/*for (n = 0; n < len; n += m)
	{
	
		//log_annoying ("read (%d, %p, %d) ...", fd, rbuf + n, len - n);
		m = read (fd, rbuf + n, len - n);
		// log_annoying ("... = %d", m);

		if (m == 0 || rbuf[n+m-1] == 0 )
		{
			//r = 0;
			break;
		}
		else if (m == -1)
		{
			if (errno != EAGAIN)
			{
				r = -1;
				break;
			}
			else
				m = 0;
		}
	}
	*/
	fcntl (fd, F_SETFL, flags);
	return n;
}

int32_t ls_write_all (int fd,  const char *data, size_t len)
{
	int32_t m;
	size_t n ; 
	const char *wdata = data;

	for (n = 0; n < len; n += m)
	{
		//  log_annoying ("write (%d, %p, %d) ...", fd, wdata + n, len - n);
		m = write (fd, wdata + n, len - n);
		//   log_annoying ("... = %d", m);
		if (m == 0)
			return 0;
		else if (m == -1)
		{
			if (errno != EAGAIN)
				return -1;
			else
				m = 0;
		}
	}

	return len;
}
