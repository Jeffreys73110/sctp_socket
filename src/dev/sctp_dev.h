// sctp_dev.h

#ifndef _SCTP_DEV_HEADER_DEF
#define _SCTP_DEV_HEADER_DEF

#include "../module/sctp_socket/sctp_socket.h"

void handle_event(void *buf);
void *sctp_server_event(void *pobj);
void *sctp_client_event(void *pobj);
int main(int argc, char **argv);

#endif // _SCTP_DEV_HEADER_DEF