#include "UDPNetTrans.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <iostream>
#include "Logger.h"
#include "CMDParserUp.h"
#include "GlobDefine.h"
#include "HandleUp.h"
#include "PCTransProtocal.h"
#include "RCSP.h"
#include "UpFileAttrs.h"
#include "FileTrans.h"

using namespace std;
using namespace FrameWork;
UDPNetTrans::UDPNetTrans(SetNetworkTerminal *setNetworkTerminal) :
		NetTrans(UNIXAF, DATAGRAM, 0), setNetworkTerminal(setNetworkTerminal), UDPStatus(
				false) {
	buffer = new INT8[BufferSizeMax];
}
UDPNetTrans::~UDPNetTrans() {
	cout << "udp destory" << endl;
	delete buffer;
	UDPStatus = true;
}

INT32 UDPNetTrans::socketRunThread() {
	pthread_t tid;
	if (pthread_create(&tid, NULL, pthreadStart, (void *) this) == 0) {
		printf("Create thread successfully!........\n");
	} else
		;
	return retOk;
}

void *UDPNetTrans::pthreadStart(void *args) {
	UDPNetTrans * udp = (UDPNetTrans*) args;
	udp->socketSelect();
	return NULL;
}

INT32 UDPNetTrans::socketSelect() {
	struct sockaddr_in recvSendAddr;
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	fd_set readfd; //读文件描述符集合
	INT32 ret_select = 0;
	INT32 ret_recv = 0;
	/*  超时 */
	FileTrans fileTrans;
	SmartPtr<UpFileAttrs> upFileAttrs = UpFileAttrs::createFileAttrs();
	SmartPtr<DEV_Request_FileProtocal> request(new DEV_Request_FileProtocal);
	while (!UDPStatus) {
		FD_ZERO(&readfd);
		FD_SET(m_socket, &readfd);
		ret_select = select(m_socket + 1, &readfd, NULL, NULL, NULL);
		if (ret_select < 0) {
			break;
		} else if (ret_select == 0) {
			printf("timeout\n");
			return retError;
			break;
		}
		if (FD_ISSET(m_socket, &readfd)) {
			memset(buffer, 0, BufferSizeMax);
			ret_recv = recvfrom(m_socket, buffer, BufferSizeMax, 0,
					(struct sockaddr *) &recvSendAddr,
					(socklen_t *) &tmp_server_addr_len); //recv_addr为服务器端地址
			if (ret_recv <= 0) {
				break;
			}
			struct sockaddr_in recvAddr;
			memcpy(&recvAddr, &recvSendAddr, sizeof(recvSendAddr));
			SetNetworkTerminal netSet(*setNetworkTerminal);
			INT32 sockfd = m_socket;
			switch (CMDParserUp::parserPCRequestHead(buffer, ret_recv)) {
			case CMD_DEV_SEARCH: {
				HandleUp::getInstance().devSearchCMDHandle(recvAddr, &netSet,
						sockfd);
			}/*end case 1*/
				break;
			case CMD_DEV_PARAMETER_SETTING: {
				HandleUp::getInstance().devParamSetCMDHandle(recvAddr, buffer,
						&netSet, sockfd);
			}/*end case 2*/
				break;
			case CMD_DEV_UPGRADE: {
				HandleUp::getInstance().devUpgradePCRequestCMDHandle(recvAddr,
						buffer, &netSet, sockfd, *upFileAttrs.get(), fileTrans,
						request.get());
			}/*end case 3*/
				break;
			case CMD_DEV_FILE_TRANSPORT: {
				HandleUp::getInstance().devFileTransCMDHandle(recvAddr, buffer,
						&netSet, sockfd, *upFileAttrs.get(), fileTrans,
						request.get());
			}/*end case 4*/
				break;
			case CMD_DEV_UPGRADE_REPLY: {

			}
				break;
			default:
				break;
			}

		} /* end if FD_ISSET */
//		} /* end if */
	} /* end while(1) */
	return retOk;
}

