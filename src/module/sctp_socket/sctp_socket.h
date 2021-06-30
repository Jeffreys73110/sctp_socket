// sctp_socket.h

#ifndef _SCTP_SOCKET_HEADER_DEF
#define _SCTP_SOCKET_HEADER_DEF


#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <iostream>

#define NATIVE_SCTP_FUNC_FLAG	true
#define DUMP_INFO_FLAG	true



#define MAX_STREAM 64


typedef void (*SCTP_EVENT_CALLBACK)(void *lpobj);

// Functions
int start_sctp_server(int &fd, char *ip, unsigned short port, sctp_event_subscribe *pses);
int start_sctp_client(int &fd, struct sockaddr_in *addr);
int sctp_recv(int fd, unsigned char *buf, int buf_len, int &msg_flags, SCTP_EVENT_CALLBACK event_cb);
int sctp_send(int fd, unsigned char *buf, int buf_len);
int close_sctp_socket(int fd);

int subscribe_sctp_event(int fd, const sctp_event_subscribe *pses);
int get_subscribed_sctp_event(int fd, sctp_event_subscribe *pses);
int get_sctp_status(int fd, sctp_status *pstatus);
void dump_sctp_event(void *buf);


#endif	// _SCTP_SOCKET_HEADER_DEF