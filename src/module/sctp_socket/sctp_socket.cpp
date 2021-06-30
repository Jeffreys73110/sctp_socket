#include "sctp_socket.h"

int subscribe_sctp_event(int fd, const sctp_event_subscribe *pses)
{
	struct sctp_event_subscribe dft_ses =
		{
			.sctp_data_io_event = 1,
			.sctp_association_event = 1,
			.sctp_address_event = 1,
			.sctp_send_failure_event = 1,
			.sctp_peer_error_event = 1,
			.sctp_shutdown_event = 1,
			.sctp_partial_delivery_event = 1,
			.sctp_adaptation_layer_event = 1,
			.sctp_authentication_event = 1,
			.sctp_sender_dry_event = 0};

	if (!pses)
		pses = &dft_ses;

	if (setsockopt(fd, SOL_SCTP, SCTP_EVENTS, pses, sizeof(sctp_event_subscribe)) != 0)
	{
		perror("setsockopt fail for event subscribing:");
		return -2;
	}
	return 0;
}

int get_subscribed_sctp_event(int fd, sctp_event_subscribe *pses)
{
	if (!pses)
	{
		printf("get_subscribed_sctp_event(), parameter error, pses=%p\n", pses);
		return -1;
	}

	int slen = sizeof(sctp_event_subscribe);
	if (getsockopt(fd, SOL_SCTP, SCTP_EVENTS, (void *)pses, (socklen_t *)&slen) != 0)
	{
		perror("getsockopt fail for getting sctp_event:");
		return -2;
	}

#if DUMP_INFO_FLAG
	printf("subscribed_sctp_event:\n{\n");
	printf("\tsctp_data_io_event = %d\n", pses->sctp_data_io_event);
	printf("\tsctp_association_event = %d\n", pses->sctp_association_event);
	printf("\tsctp_address_event = %d\n", pses->sctp_address_event);
	printf("\tsctp_send_failure_event = %d\n", pses->sctp_send_failure_event);
	printf("\tsctp_peer_error_event = %d\n", pses->sctp_peer_error_event);
	printf("\tsctp_shutdown_event = %d\n", pses->sctp_shutdown_event);
	printf("\tsctp_partial_delivery_event = %d\n", pses->sctp_partial_delivery_event);
	printf("\tsctp_adaptation_layer_event = %d\n", pses->sctp_adaptation_layer_event);
	printf("\tsctp_authentication_event = %d\n", pses->sctp_authentication_event);
	printf("\tsctp_sender_dry_event = %d\n", pses->sctp_sender_dry_event);
	printf("}\n");
#endif // DUMP_INFO_FLAG

	return 0;
}

int get_sctp_status(int fd, sctp_status *pstatus)
{
	if (!pstatus)
	{
		printf("get_subscribed_sctp_event(), parameter error, pstatus=%p\n", pstatus);
		return -1;
	}

	int slen = sizeof(sctp_status);
	if (getsockopt(fd, SOL_SCTP, SCTP_STATUS, (void *)pstatus, (socklen_t *)&slen) != 0)
	{
		perror("getsockopt fail for getting sctp_status:");
		return -2;
	}

#if DUMP_INFO_FLAG
	const char sctp_sstat_state_str[][32] = {"SCTP_EMPTY", "SCTP_CLOSED", "SCTP_COOKIE_WAIT", "SCTP_COOKIE_ECHOED", "SCTP_ESTABLISHED", "SCTP_SHUTDOWN_PENDING", "SCTP_SHUTDOWN_SENT", "SCTP_SHUTDOWN_RECEIVED", "SCTP_SHUTDOWN_ACK_SENT"};
	printf("sctp_status:\n{\n");
	printf("\tassoc_id = %d\n", pstatus->sstat_assoc_id);
	printf("\tstate = %s\n", sctp_sstat_state_str[pstatus->sstat_state]);
	printf("\trwnd = %u\n", pstatus->sstat_rwnd);
	printf("\tunackdata = %hu\n", pstatus->sstat_unackdata);
	printf("\tpenddata = %hu\n", pstatus->sstat_penddata);
	printf("\tinstrms = %hu\n", pstatus->sstat_instrms);
	printf("\toutstrms = %hu\n", pstatus->sstat_outstrms);
	printf("\tfragmentation_point = %u\n", pstatus->sstat_fragmentation_point);

	char addrbuf[INET6_ADDRSTRLEN];
	const char *ap;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;
	sctp_paddrinfo *pprimary = &pstatus->sstat_primary;
	if (pstatus->sstat_primary.spinfo_address.ss_family == AF_INET)
	{
		sin = (struct sockaddr_in *)&pprimary->spinfo_address;
		ap = inet_ntop(AF_INET, &sin->sin_addr, addrbuf,
					   INET6_ADDRSTRLEN);
	}
	else
	{
		sin6 = (struct sockaddr_in6 *)&pprimary->spinfo_address;
		ap = inet_ntop(AF_INET6, &sin6->sin6_addr, addrbuf,
					   INET6_ADDRSTRLEN);
	}
	const char sctp_spinfo_state_str[][32] = {"SCTP_INACTIVE", "SCTP_PF", "SCTP_ACTIVE", "SCTP_UNCONFIRMED", "SCTP_UNKNOWN"};
	printf("\tprimary = \n\t{\n");
	printf("\t\tspinfo_assoc_id = %d\n", pprimary->spinfo_assoc_id);
	printf("\t\taddress = %s\n", ap);
	printf("\t\tstate = %s\n", sctp_spinfo_state_str[pprimary->spinfo_state]);
	printf("\t\tcwnd = %u\n", pprimary->spinfo_cwnd);
	printf("\t\tsrtt = %u\n", pprimary->spinfo_srtt);
	printf("\t\trto = %u\n", pprimary->spinfo_rto);
	printf("\t\tmtu = %u\n", pprimary->spinfo_mtu);
	printf("\t}\n}\n");
#endif // DUMP_INFO_FLAG

	return 0;
}

int start_sctp_server(int &fd, char *ip, unsigned short port, sctp_event_subscribe *pses)
{
	if (!ip)
	{
		printf("start_sctp_server(), parameter error, ip=%p\n", ip);
		return -1;
	}

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	// sin.sin_addr.s_addr=inet_addr(ip);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0)
	{
		perror("[start_sctp_server] socket fail: ");
		return -2;
	}
	if (bind(fd, (sockaddr *)&sin, sizeof(sin)) < 0)
	{
		perror("[start_sctp_server] bind fail: ");
		return -3;
	}

	if (listen(fd, 1) < 0)
	{
		perror("[start_sctp_server] listen fail: ");
		return -4;
	}

	if (subscribe_sctp_event(fd, NULL) < 0)
	{
		printf("[start_sctp_server] subscribe sctp event fail, socket=%d\n", fd);
		return -5;
	}

	printf("[start_sctp_server] start sctp server successfully, socket=%d, listened port=%d\n", fd, port);
	return 0;
}

int start_sctp_client(int &fd, struct sockaddr_in *addr)
{
	if (!addr)
	{
		printf("[start_sctp_client] invalid parameter, addr=%p", addr);
		return -1;
	}

	ssize_t n;
	int perr;
	struct cmsghdr *cmsg;
	struct sctp_sndrcvinfo *sri;
	char cbuf[sizeof(*cmsg) + sizeof(*sri)];
	struct msghdr msg[1];
	struct iovec iov[1];
	int ret;
	struct sctp_initmsg initmsg;
	struct sctp_event_subscribe events;

	// Create a one-one SCTP socket
	if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) == -1)
	{
		perror("[start_sctp_client] socket");
		return -2;
	}
	// printf("create client socket successfully\n");

	//--- We are interested in association change events and we want to get sctp_sndrcvinfo in each receive.
	events.sctp_association_event = 1;
	events.sctp_data_io_event = 1;
	ret = setsockopt(fd, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events));
	if (ret < 0)
	{
		perror("[start_sctp_client] setsockopt SCTP_EVENTS");
	}

	// Set the SCTP stream parameters to tell the other side when setting up the association.
	memset(&initmsg, 0, sizeof(struct sctp_initmsg));
	initmsg.sinit_num_ostreams = MAX_STREAM;
	initmsg.sinit_max_instreams = MAX_STREAM;
	initmsg.sinit_max_attempts = MAX_STREAM;
	ret = setsockopt(fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg,
					 sizeof(struct sctp_initmsg));
	if (ret < 0)
	{
		perror("[start_sctp_client] setsockopt SCTP_INITMSG");
		return -3;
	}

	// Set the SCTP stream parameters to tell the other side when setting up the association.
	memset(&initmsg, 0, sizeof(struct sctp_initmsg));
	initmsg.sinit_num_ostreams = MAX_STREAM;
	initmsg.sinit_max_instreams = MAX_STREAM;
	initmsg.sinit_max_attempts = MAX_STREAM;
	ret = setsockopt(fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg,
					 sizeof(struct sctp_initmsg));
	if (ret < 0)
	{
		perror("[start_sctp_client] setsockopt SCTP_INITMSG");
		close(fd);
		return -4;
	}

	if (connect(fd, (struct sockaddr *)addr, sizeof(*addr)) == -1)
	{
		perror("connect");
		close(fd);
		return -5;
	}

	printf("[start_sctp_client] start sctp client successfully, socket=%d, connected to %s:%d\n", fd, inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
}

void dump_sctp_event(void *buf)
{
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
		printf("[SCTP_Event] assoc_change: len=%hu, state=%s, error=%hu, instr=%hu outstr=%hu\n",
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
		printf("[SCTP_Event] intf_change: %s, len=%u state=%d, error=%d, assoc_id=%d\n",
			   ap, spc->spc_length, spc->spc_state, spc->spc_error, spc->spc_assoc_id);
		break;

	case SCTP_REMOTE_ERROR:
		printf("[SCTP_Event] remote_error: len=%u, err=%hu, assoc_id=%d\n",
			   ntohs(sre->sre_length), ntohs(sre->sre_error), spc->spc_assoc_id);
		break;

	case SCTP_SEND_FAILED:
		printf("[SCTP_Event] send_failed: len=%u err=%d, assoc_id=%d\n"
			   "ssf_info=\n{\n\tsinfo_stream=%d,\n\tsinfo_ssn=%d,\n\tsinfo_flags=%hu,\n\tsinfo_ppid=%hu,\n\tsinfo_context=%hu,\n\tsinfo_timetolive=%hu,\n\tsinfo_tsn=%hu,\n\tsinfo_cumtsn=%hu}\n",
			   ssf->ssf_length, ssf->ssf_error, ssf->ssf_assoc_id,
			   ssf->ssf_info.sinfo_stream, ssf->ssf_info.sinfo_ssn, ssf->ssf_info.sinfo_flags, ssf->ssf_info.sinfo_ppid, ssf->ssf_info.sinfo_context, ssf->ssf_info.sinfo_timetolive, ssf->ssf_info.sinfo_tsn, ssf->ssf_info.sinfo_cumtsn);
		break;

	case SCTP_SHUTDOWN_EVENT:
		printf("[SCTP_Event] shutdown_event: len=%u, assoc_id=%d\n",
			   sse->sse_length, sse->sse_assoc_id);
		break;

	case SCTP_PARTIAL_DELIVERY_EVENT:
		printf("[SCTP_Event] partial_delivery_event: len=%u, indication=%hu, assoc_id=%d\n",
			   spe->pdapi_length, spe->pdapi_indication, spe->pdapi_assoc_id);
		break;

	case SCTP_ADAPTATION_INDICATION:
		printf("[SCTP] adaption_indication: len=%u, adaptation_ind=%hu, assoc_id=%d\n",
			   sae->sai_length, sae->sai_adaptation_ind, sae->sai_assoc_id);
		break;

	case SCTP_AUTHENTICATION_INDICATION:
		printf("[SCTP_Event] authentication_indication: len=%u, keynumber=%d, altkeynumber=%d, indication=%hu, assoc_id=%d\n",
			   sauth->auth_length, sauth->auth_keynumber, sauth->auth_altkeynumber, sauth->auth_indication, sauth->auth_assoc_id);
		break;

	case SCTP_SENDER_DRY_EVENT:
		printf("[SCTP_Event] sender_dry_event: len=%u, assoc_id=%d\n",
			   ssde->sender_dry_length, ssde->sender_dry_assoc_id);
		break;

	default:
		printf("[SCTP_Event] unknown type: %hu\n", snp->sn_header.sn_type);
		break;
	}
	return;
}

int sctp_recv(int fd, unsigned char *buf, int buf_len, int &msg_flags, SCTP_EVENT_CALLBACK event_cb)
{
	if (!buf)
	{
		printf("sctp_recv(), parameter error, buf=%p\n", buf);
		return -1;
	}

	int rcv = 0, cv = 0;
	const int MaxCount = 100;

#if (!NATIVE_SCTP_FUNC_FLAG)
	struct iovec iov = {buf, buf_len};
	union
	{
		struct cmsghdr hdr;
		unsigned char buf[CMSG_SPACE(sizeof(sctp_sndrcvinfo))];
	} cmsgbuf;
	struct msghdr msg =
		{
			.msg_name = NULL,
			.msg_namelen = 0,
			.msg_iov = &iov,
			.msg_iovlen = 1,
			.msg_control = cmsgbuf.buf,
			.msg_controllen = sizeof(cmsgbuf.buf),
			.msg_flags = 0};
	struct cmsghdr *cmsg;

	for (int n = 0; n < MaxCount; n++)
	{
		rcv = recvmsg(fd, &msg, 0); //TODO: receive and record coming IP to distinguish who the eNB is
		msg_flags = msg.msg_flags;

		if (rcv <= 0)
		{
			cv = rcv;
			break;
		}

		// All data delivered
		cv += rcv;
		if (msg_flags & MSG_EOR)
		{
#if DUMP_INFO_FLAG
			if ((msg_flags & MSG_NOTIFICATION) == 0)
			{
				//--- print sri
				for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg))
				{
					if (cmsg->cmsg_len == CMSG_LEN(sizeof(sctp_sndrcvinfo)) &&
						cmsg->cmsg_level == IPPROTO_SCTP &&
						cmsg->cmsg_type == SCTP_SNDRCV)
					{
						sctp_sndrcvinfo *sri = (sctp_sndrcvinfo *)CMSG_DATA(cmsg);
						printf("cmsg=0x%x\nsri = 0x%x\n{", (void *)cmsg, (void *)sri);
						printf("\tsinfo_stream = %d\n", sri->sinfo_stream);
						printf("\tsinfo_ssn = %d\n", sri->sinfo_ssn);
						printf("\tsinfo_flags = %d\n", sri->sinfo_flags);
						printf("\tsinfo_ppid = 0x%x\n", sri->sinfo_ppid);
						printf("\tsinfo_context = %d\n", sri->sinfo_context);
						printf("\tsinfo_timetolive = 0x%x\n", sri->sinfo_timetolive);
						printf("\tsinfo_tsn = 0x%x\n", sri->sinfo_tsn);
						printf("\tsinfo_cumtsn = 0x%x\n", sri->sinfo_cumtsn);
						printf("\tsinfo_assoc_id = %d\n", sri->sinfo_assoc_id);
						printf("}\n");
					}
				}
			}
#endif // DUMP_INFO_FLAG

			// printf("All data delivered, cv = %d, %d @ %s\n", cv, __LINE__, __FILE__);
			break;
		}
	}
#else
	sctp_sndrcvinfo sndrcvinfo;
	for (int n = 0; n < MaxCount; n++)
	{
		rcv = sctp_recvmsg(fd, buf + cv, buf_len - cv, (struct sockaddr *)NULL, 0, &sndrcvinfo, &msg_flags);
		if (rcv <= 0)
		{
			cv = rcv;
			break;
		}

		//--- All data delivered
		cv += rcv;
		if (msg_flags & MSG_EOR)
		{
#if DUMP_INFO_FLAG
			if ((msg_flags & MSG_NOTIFICATION) == 0)
			{
				//--- print sri
				printf("sri =\n{");
				printf("\tsinfo_stream = %d\n", sndrcvinfo.sinfo_stream);
				printf("\tsinfo_ssn = %d\n", sndrcvinfo.sinfo_ssn);
				printf("\tsinfo_flags = %d\n", sndrcvinfo.sinfo_flags);
				printf("\tsinfo_ppid = 0x%x\n", sndrcvinfo.sinfo_ppid);
				printf("\tsinfo_context = %d\n", sndrcvinfo.sinfo_context);
				printf("\tsinfo_timetolive = 0x%x\n", sndrcvinfo.sinfo_timetolive);
				printf("\tsinfo_tsn = 0x%x\n", sndrcvinfo.sinfo_tsn);
				printf("\tsinfo_cumtsn = 0x%x\n", sndrcvinfo.sinfo_cumtsn);
				printf("\tsinfo_assoc_id = %d\n", sndrcvinfo.sinfo_assoc_id);
				printf("}\n");
			}
#endif // DUMP_INFO_FLAG

			// printf("All data delivered, cv = %d, %d @ %s\n", cv, __LINE__, __FILE__);
			break;
		}
	}
#endif

	//--- Intercept notifications here
	if (msg_flags & MSG_NOTIFICATION)
	{
#if DUMP_INFO_FLAG
		dump_sctp_event(buf);
#endif // DUMP_INFO_FLAG
		if (event_cb)
			event_cb(buf);
	}

	return cv;
}

int sctp_send(int fd, unsigned char *buf, int buf_len)
{
	static int stream_count = 0;
	if (!buf || buf_len <= 0)
	{
		printf("sctp_send(), parameter error, buf=%p, buf_len=%d\n", buf, buf_len);
		return -1;
	}

	struct iovec iov = {buf, (size_t)buf_len};
#if (!NATIVE_SCTP_FUNC_FLAG)
	//--- method 1 that can't manage ppid and other arguments
	struct msghdr msg =
		{
			.msg_name = NULL,
			.msg_namelen = 0,
			.msg_iov = &iov,
			.msg_iovlen = 1,
			.msg_control = NULL,
			.msg_controllen = 0,
			.msg_flags = 0};

	if (sendmsg(fd, &msg, 0) < 0)
	{
		perror("sctp send error: ");
		return -2;
	}

	// //--- method 2 with "Invalid argument" error after the second sending
	// union
	// {
	// 	struct cmsghdr hdr;
	// 	unsigned char buf[CMSG_SPACE(sizeof(sctp_sndrcvinfo))];
	// } cmsgbuf;
	// struct msghdr msg =
	// 	{
	// 		.msg_name = NULL,
	// 		.msg_namelen = 0,
	// 		.msg_iov = &iov,
	// 		.msg_iovlen = 1,
	// 		.msg_control = msg.msg_control = cmsgbuf.buf,
	// 		.msg_controllen = msg.msg_controllen = sizeof(cmsgbuf.buf),
	// 		.msg_flags = 0};

	// struct cmsghdr *cmsg;
	// cmsg = CMSG_FIRSTHDR(&msg);
	// cmsg->cmsg_len = CMSG_LEN(sizeof(sctp_sndrcvinfo));
	// cmsg->cmsg_level = IPPROTO_SCTP;
	// cmsg->cmsg_type = SCTP_SNDRCV;

	// struct sctp_sndrcvinfo *sri = (sctp_sndrcvinfo *)CMSG_DATA(cmsg);
	// // ppid: refer to [https://www.iana.org/assignments/sctp-parameters/sctp-parameters.xhtml#sctp-parameters-25]
	// sri->sinfo_ppid = htonl(0x12); // 0x12: S1AP
	// sri->sinfo_stream = 0;		   // stream 0
	// sri->sinfo_ssn = stream_count++;

	// if (sendmsg(fd, &msg, 0) < 0)
	// {
	// 	perror("sctp send error: ");
	// 	return -2;
	// }

#else
	if (sctp_sendmsg(fd, buf, buf_len, NULL, 0, htonl(0x12), 0, 0, 0, 0) < 0)
	{
		perror("sctp send error: ");
		return -2;
	}
#endif // (!NATIVE_SCTP_FUNC_FLAG)

	return 0;
}

int close_sctp_socket(int fd)
{
	if (fd > 0)
		close(fd);

	return 0;
}