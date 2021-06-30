#include "sctp_dev.h"

#define DUMP_SCTP_EVENT_MESSAGE false
void handle_event(void *buf)
{
	if (!DUMP_SCTP_EVENT_MESSAGE)
		return;

	union sctp_notification *snp = (sctp_notification *)buf;
	sctp_assoc_change *sac = &snp->sn_assoc_change;
	const char sctp_sac_state_str[][32] = {"SCTP_COMM_UP", "SCTP_COMM_LOST", "SCTP_RESTART", "SCTP_SHUTDOWN_COMP", "SCTP_CANT_STR_ASSOC"};
	sctp_paddr_change *spc = &snp->sn_paddr_change;
	char addrbuf[INET6_ADDRSTRLEN];
	const char *ap;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;
	sctp_remote_error *sre = &snp->sn_remote_error;
	sctp_send_failed *ssf = &snp->sn_send_failed;
	sctp_shutdown_event *sse = &snp->sn_shutdown_event;
	sctp_pdapi_event *spe = &snp->sn_pdapi_event;
	sctp_adaptation_event *sae = &snp->sn_adaptation_event;
	sctp_authkey_event *sauth = &snp->sn_authkey_event;
	sctp_sender_dry_event *ssde = &snp->sn_sender_dry_event;

	switch (snp->sn_header.sn_type)
	{
	case SCTP_ASSOC_CHANGE:
		printf("[SCTP_Event] assoc_change: len=%hu, state=%s, error=%d, instr=%hu outstr=%hu\n",
			   sac->sac_length, sctp_sac_state_str[sac->sac_state], sac->sac_error,
			   sac->sac_inbound_streams, sac->sac_outbound_streams);
		break;

	case SCTP_PEER_ADDR_CHANGE:
		if (spc->spc_aaddr.ss_family == AF_INET)
		{
			sin = (struct sockaddr_in *)&spc->spc_aaddr;
			ap = inet_ntop(AF_INET, &sin->sin_addr, addrbuf,
						   INET6_ADDRSTRLEN);
		}
		else
		{
			sin6 = (struct sockaddr_in6 *)&spc->spc_aaddr;
			ap = inet_ntop(AF_INET6, &sin6->sin6_addr, addrbuf,
						   INET6_ADDRSTRLEN);
		}
		printf("[SCTP_Event] intf_change: %s, len=%hu state=%d, error=%d, assoc_id=%d\n",
			   ap, spc->spc_length, spc->spc_state, spc->spc_error, spc->spc_assoc_id);
		break;

	case SCTP_REMOTE_ERROR:
		printf("[SCTP_Event] remote_error: len=%hu, err=%hu, assoc_id=%d\n",
			   ntohs(sre->sre_length), ntohs(sre->sre_error), spc->spc_assoc_id);
		break;

	case SCTP_SEND_FAILED:
		printf("[SCTP_Event] send_failed: len=%hu err=%d, assoc_id=%d"
			   "ssf_info=\n{\n\tsinfo_stream=%d,\n\tsinfo_ssn=%d,\n\tsinfo_flags=%hu,\n\tsinfo_ppid=%hu,\n\tsinfo_context=%hu,\n\tsinfo_timetolive=%hu,\n\tsinfo_tsn=%hu,\n\tsinfo_cumtsn=%hu}\n",
			   ssf->ssf_length, ssf->ssf_error, ssf->ssf_assoc_id,
			   ssf->ssf_info.sinfo_stream, ssf->ssf_info.sinfo_ssn, ssf->ssf_info.sinfo_flags, ssf->ssf_info.sinfo_ppid, ssf->ssf_info.sinfo_context, ssf->ssf_info.sinfo_timetolive, ssf->ssf_info.sinfo_tsn, ssf->ssf_info.sinfo_cumtsn);
		break;

	case SCTP_SHUTDOWN_EVENT:
		printf("[SCTP_Event] shutdown_event: len=%hu, assoc_id=%d\n",
			   sse->sse_length, sse->sse_assoc_id);
		break;

	case SCTP_PARTIAL_DELIVERY_EVENT:
		printf("[SCTP_Event] partial_delivery_event: len=%hu, indication=%hu, assoc_id=%d\n",
			   spe->pdapi_length, spe->pdapi_indication, spe->pdapi_assoc_id);
		break;

	case SCTP_ADAPTATION_INDICATION:
		printf("[SCTP] adaption_indication: len=%hu, adaptation_ind=%hu, assoc_id=%d\n",
			   sae->sai_length, sae->sai_adaptation_ind, sae->sai_assoc_id);
		break;

	case SCTP_AUTHENTICATION_INDICATION:
		printf("[SCTP_Event] authentication_indication: len=%hu, keynumber=%d, altkeynumber=%d, indication=%hu, assoc_id=%d\n",
			   sauth->auth_length, sauth->auth_keynumber, sauth->auth_altkeynumber, sauth->auth_indication, sauth->auth_assoc_id);
		break;

	case SCTP_SENDER_DRY_EVENT:
		printf("[SCTP_Event] sender_dry_event: len=%hu, assoc_id=%d\n",
			   ssde->sender_dry_length, ssde->sender_dry_assoc_id);
		break;

	default:
		printf("unknown type: %hu\n", snp->sn_header.sn_type);
		break;
	}
	return;
}

void *sctp_server_event(void *pobj)
{
	int server_fd = *(int *)pobj;
	if (server_fd < 0)
	{
		printf("[sctp_server_event] server_fd=%d\n", server_fd);
		return NULL;
	}

	struct sockaddr_in clientaddr; // client address
	int fdmax;					   // maximum file descriptor number
	int newfd;					   // newly accept()ed socket descriptor
	unsigned int addrlen = sizeof(clientaddr);
	int select_res, recv_res;

	fd_set read_fds, master_fds;
	/* clear the master and temp sets */
	FD_ZERO(&master_fds);
	FD_ZERO(&read_fds);

	// add the listener to the master_fds set
	FD_SET(server_fd, &master_fds);
	// keep track of the biggest file descriptor
	fdmax = server_fd; // so far, it's this one

	while (1)
	{
		read_fds = master_fds;
		select_res = select(fdmax + 1, &read_fds, NULL, NULL, NULL);

		// printf("----- select, res=%d\n", select_res);
		if (select_res == -1)
			perror("Server-select() error");
		else
		{
			for (int fd = 0; fd <= fdmax; fd++)
			{
				if (FD_ISSET(fd, &read_fds))
				{
					if (fd == server_fd)
					{
						printf("--------------------------------\n");
						/* handle new connections */
						newfd = accept(fd, (struct sockaddr *)&clientaddr, &addrlen);
						if (newfd == -1)
						{
							perror("MME server-accept() error");
						}
						else
						{
							printf("\033[1;32mAccept new socket\033[m, socket=%d from %s:%d\n", newfd, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
							//--- add new fd to fdset
							// add to master_fds set
							FD_SET(newfd, &master_fds);
							// keep track of the maximum
							if (newfd > fdmax)
								fdmax = newfd;
						}
					}
					else
					{
						printf("--------------------------------\n");
						uint8_t msg[1024];
						bool close_conn = false;
						int msg_flags = 0;
						recv_res = sctp_recv(fd, msg, sizeof(msg), msg_flags, handle_event);
						if (recv_res < 0)
						{
							if (errno != EWOULDBLOCK)
							{
								perror("recv() failed");
								close_conn = true;
							}
						}
						else if (recv_res == 0)
						{
							printf("[sctp_server_event] SCTP_CLOSE, socket=%d\n", fd);
							close_conn = true;
						}
						else
						{
							//--- recv msg_notification
							if (msg_flags & MSG_NOTIFICATION)
							{
								printf("\033[1;33m[sctp_server_event] notification\033[m (%d)\n", recv_res);
								continue;
							}

							//--- get sctp_status including assoc_id
							sctp_status status;
							get_sctp_status(fd, &status);

							//--- recv sctp data
							printf("\033[1;32m[sctp_server_event] recvbuf\033[m (%d): %s\n", recv_res, msg);

							//--- send back sctp data
							int ret = sctp_send(fd, msg, recv_res);
							if (ret < 0)
							{
								printf("[sctp_server_event] sctp_send() fail, ret=%d, fd=%d, buf=%p, buf_len=%d\n", ret, fd, msg, recv_res);
							}
						}

						if (close_conn)
						{
							// release eNB data
							printf("\033[1;32m[sctp_server_event] SCTP_CLOSE\033[m, socket=%d\n", fd);
							// close socket
							close(fd);
							FD_CLR(fd, &master_fds);
							if (fd == fdmax)
							{
								while (!FD_ISSET(fdmax, &master_fds))
									fdmax -= 1;
							}
						}
					}
				}
			}
		}
	}
}

void *sctp_client_event(void *pobj)
{
	int fd = *(int *)pobj;

	while (1)
	{
		printf("--------------------------------\n");
		uint8_t msg[1024];
		bool close_conn = false;
		int msg_flags = 0;
		int recv_res = sctp_recv(fd, msg, sizeof(msg), msg_flags, handle_event);
		if (recv_res < 0)
		{
			if (errno != EWOULDBLOCK)
			{
				perror("recv() failed");
				close_conn = true;
			}
		}
		else if (recv_res == 0)
		{
			printf("[sctp_client_event] Connection closed, socket=%d\n", fd);
			close_conn = true;
		}
		else
		{
			//--- get sctp_status including assoc_id
			if (msg_flags & MSG_NOTIFICATION)
			{
				printf("\033[1;33m[sctp_client_event] notification\033[m (%d)\n", recv_res);
				continue;
			}

			sctp_status status;
			get_sctp_status(fd, &status);

			//--- recv sctp data
			printf("\033[1;33m[sctp_client_event] recvbuf\033[m (%d): %s\n", recv_res, msg);

			break;
		}
	}
}

int main(int argc, char **argv)
{
	printf("----------------------------------------------------\n\n");

	//--- start sctp server
	int server_fd = 0;
	int ret = start_sctp_server(server_fd, (char *)"127.0.0.1", 36000, NULL);
	if (ret < 0)
	{
		printf("start_sctp_server failed, \n");
	}

	//--- Create a thread to listen network traffic for server
	pthread_t server_tid = 0;
	int server_threadValid = pthread_create(&server_tid, NULL, sctp_server_event, &server_fd);
	if (server_threadValid != 0)
	{
		fprintf(stderr, "pthread_create failed, thread Valid=%d\n", server_threadValid);
	}

	usleep(10000);

	//--- start sctp client
	int client_fd = 0;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(36000);

	ret = start_sctp_client(client_fd, &addr);
	if (ret < 0)
	{
		printf("start_sctp_client failed, ret=%d\n", ret);
	}

	//--- Create a thread to receive network traffic for client
	pthread_t client_tid = 0;
	int client_threadValid = pthread_create(&client_tid, NULL, sctp_client_event, &client_fd);
	if (client_threadValid != 0)
	{
		fprintf(stderr, "pthread_create failed, thread Valid=%d\n", client_threadValid);
	}

	//--- send data from client to server
	uint8_t test_data[] = "1234567890!@#$%^&*()abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	ret = sctp_send(client_fd, test_data, sizeof(test_data));
	if (ret < 0)
	{
		printf("sctp_client send fail, ret=%d, fd=%d, buf_len=%lu\n", ret, client_fd, sizeof(test_data));
	}

	usleep(100000);

	//--- close sctp client
	close_sctp_socket(client_fd);
	if (client_threadValid)
		pthread_cancel(client_tid);

	usleep(100000);

	pthread_join(client_tid, NULL);

	printf("\n");
	printf("Terminate server\n");

	return 0;
}