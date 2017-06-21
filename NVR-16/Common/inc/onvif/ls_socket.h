#ifndef __LS_SOCK_H
#define __LS_SOCK_H 1

#include <stdint.h>
#include <sys/types.h>
#include <netdb.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _IN_
#define _IN_OUT_
#define _OUT_

extern int32_t ls_write_all (_IN_ int fd,  _IN_ const char *data, _IN_ size_t len);
extern int32_t ls_read_all (_IN_ int fd,  _OUT_ char *buf, _IN_ size_t len);

extern uint64_t ls_sock_hton_uint64(_IN_ uint64_t v);
extern uint64_t ls_sock_ntoh_uint64(_IN_ uint64_t v);

extern uint32_t ls_sock_get_rcvbuf(_IN_ int fd, _OUT_ size_t *buf_len);
extern uint32_t ls_sock_set_rcvbuf(_IN_ int fd, _IN_ size_t buf_len);
extern uint32_t ls_sock_get_sndbuf(_IN_ int fd, _OUT_ size_t *buf_len);
extern uint32_t ls_sock_set_sndbuf(_IN_ int fd, _IN_ size_t buf_len);

extern uint32_t ls_sock_tcp_listen(_OUT_ int *fd, _IN_ const char *host, _IN_ const char *serv, _OUT_ socklen_t *salen);
extern uint32_t ls_sock_tcp_connect_wait(_OUT_ int *fd, _IN_ const char *host_peer, _IN_ const char *serv_peer,
                                          _IN_ const char *host_local, _IN_ const char *serv_local, _IN_ int timeout_ms);
extern uint32_t ls_sock_tcp_connect_nowait(_OUT_ int *fd, _IN_ const char *host_peer, _IN_ const char *serv_peer,
                                            _IN_ const char *host_local, _IN_ const char *serv_local,
                                            _OUT_ uint8_t *connected);
extern uint32_t ls_sock_tcp_is_self_connection(_IN_ int fd, _OUT_ uint8_t *is_self_connection);

extern uint32_t ls_sock_tcp_get_keepalive(_IN_ int fd, _OUT_ int *alive, _OUT_ int *idle_s, _OUT_ int *intvl_s, _OUT_ int *cnt);
extern uint32_t ls_sock_tcp_set_keepalive_on(_IN_ int fd, _IN_ int idle_s, _IN_ int intvl_s, _IN_ int cnt);
extern uint32_t ls_sock_tcp_set_keepalive_off(_IN_ int fd);

extern uint32_t ls_sock_udp_server(_OUT_ int *fd, _IN_ const char *host, _IN_ const char *serv, _OUT_ socklen_t *salen);
extern uint32_t ls_sock_udp_client(_OUT_ int *fd, _IN_ const char *host_peer, _IN_ const char *serv_peer,
                                    _IN_ const char *host_local, _IN_ const char *serv_local,
                                    _OUT_ struct sockaddr_storage *sa_peer, _OUT_ socklen_t *salen_peer);
extern uint32_t ls_sock_udp_client_connect(_OUT_ int *fd, _IN_ const char *host_peer, _IN_ const char *serv_peer,
                                            _IN_ const char *host_local, _IN_ const char *serv_local,
                                            _OUT_ struct sockaddr_storage *sa_peer, _OUT_ socklen_t *salen_peer);

extern uint32_t ls_sock_mcast_join(_IN_ int fd, _IN_ const struct sockaddr *grp, _IN_ socklen_t grplen,
                                    _IN_ const char *ifname, _IN_ uint32_t ifindex);
extern uint32_t ls_sock_mcast_leave(_IN_ int fd, _IN_ const struct sockaddr *grp, _IN_ socklen_t grplen);
extern uint32_t ls_sock_mcast_join_source_group(_IN_ int fd, _IN_ const struct sockaddr *src, _IN_ socklen_t srclen,
                                                 _IN_ const struct sockaddr *grp, _IN_ socklen_t grplen,
                                                 _IN_ const char *ifname, _IN_ uint32_t ifindex);
extern uint32_t ls_sock_mcast_leave_source_group(_IN_ int fd, _IN_ const struct sockaddr *src, _IN_ socklen_t srclen,
                                                  _IN_ const struct sockaddr *grp, _IN_ socklen_t grplen);
extern uint32_t ls_sock_mcast_block_source(_IN_ int fd, _IN_ const struct sockaddr *src, _IN_ socklen_t srclen,
                                            _IN_ const struct sockaddr *grp, _IN_ socklen_t grplen);
extern uint32_t ls_sock_mcast_unblock_source(_IN_ int fd, _IN_ const struct sockaddr *src, _IN_ socklen_t srclen,
                                              _IN_ const struct sockaddr *grp, _IN_ socklen_t grplen);
extern uint32_t ls_sock_mcast_get_ttl(_IN_ int fd, _OUT_ int *ttl);
extern uint32_t ls_sock_mcast_set_ttl(_IN_ int fd, _IN_ int ttl);
extern uint32_t ls_sock_mcast_get_if(_IN_ int fd, _OUT_ uint32_t *ifindex);
extern uint32_t ls_sock_mcast_set_if(_IN_ int fd, _IN_ const char *ifname, _IN_ uint32_t ifindex);
extern uint32_t ls_sock_mcast_get_loop(_IN_ int fd, _OUT_ unsigned int *loop);
extern uint32_t ls_sock_mcast_set_loop(_IN_ int fd, _IN_ unsigned int loop);

extern uint32_t ls_sock_icmp(_OUT_ int *fd, _IN_ const char *host, _IN_ const char *serv,
                              _OUT_ struct sockaddr_storage *sa, _OUT_ socklen_t *salen);
extern uint32_t ls_sock_icmp_echo_init(_IN_ int fd, _IN_ struct sockaddr_storage *sa);
extern uint32_t ls_sock_icmp_echo_send(_IN_ int fd, _IN_ struct sockaddr_storage *sa);
extern uint32_t ls_sock_icmp_echo_recv(_IN_ int fd, _IN_ struct sockaddr_storage *sa);

extern uint32_t ls_sock_ntop_ip(_IN_ struct sockaddr *sa, _OUT_ char *ip, _IN_ size_t len);
extern uint32_t ls_sock_ntop_port(_IN_ struct sockaddr *sa, _OUT_ uint16_t *port);
extern uint32_t ls_sock_ntop_addr(_IN_ struct sockaddr *sa, _OUT_ char *addr, _IN_ size_t len);
extern uint32_t ls_sock_pton(_IN_ char *host, _IN_ char *serv, _OUT_ struct sockaddr_storage *sa);

extern uint32_t ls_sock_get_local_ip(_IN_ int fd, _OUT_ char *ip, _IN_ size_t len);
extern uint32_t ls_sock_get_local_port(_IN_ int fd, _OUT_ uint16_t *port);
extern uint32_t ls_sock_get_local_addr(_IN_ int fd, _OUT_ char *addr, _IN_ size_t len);
extern uint32_t ls_sock_get_local_sockaddr(_IN_ int fd, _OUT_ struct sockaddr_storage *sockaddr);
extern uint32_t ls_sock_get_local_ip_by_peer(_IN_ char *host_peer, _OUT_ char *ip, _IN_ size_t len);

extern uint32_t ls_sock_get_peer_ip(_IN_ int fd, _OUT_ char *ip, _IN_ size_t len);
extern uint32_t ls_sock_get_peer_port(_IN_ int fd, _OUT_ uint16_t *port);
extern uint32_t ls_sock_get_peer_addr(_IN_ int fd, _OUT_ char *addr, _IN_ size_t len);
extern uint32_t ls_sock_get_peer_sockaddr(_IN_ int fd, _OUT_ struct sockaddr_storage *sockaddr);

extern uint32_t ls_sock_cmp_ip(_IN_ struct sockaddr *sa1, _IN_ struct sockaddr *sa2);
extern uint32_t ls_sock_cmp_port(_IN_ struct sockaddr *sa1, _IN_ struct sockaddr *sa2);
extern uint32_t ls_sock_cmp_addr(_IN_ struct sockaddr *sa1, _IN_ struct sockaddr *sa2);

extern uint32_t ls_sock_udp_rtp_rtcp_server(_OUT_ int *rtp_fd, _OUT_ int *rtcp_fd,
                                             _IN_ const char *host, _IN_ const char *rtp_port,
                                             _OUT_ uint16_t *ret_rtp_port, _OUT_ uint16_t *ret_rtcp_port,
                                             _OUT_ socklen_t *salen);

#ifdef __cplusplus
}
#endif

#endif
