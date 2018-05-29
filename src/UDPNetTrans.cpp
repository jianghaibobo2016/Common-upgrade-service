#include <iostream>
#include "Logger.h"
#include "CMDParserUp.h"
#include "GlobDefine.h"
#include "HandleUp.h"
#include "PCTransProtocal.h"
#include "UDPNetTrans.h"
#include "RCSP.h"
#include "UpFileAttrs.h"
#include "FileTrans.h"
#include <sys/time.h>

using namespace std;
using namespace FrameWork;
UDPNetTrans::UDPNetTrans(SetNetworkTerminal *setNetworkTerminal) :
		NetTrans(UNIXAF, DATAGRAM, 0), setNetworkTerminal(setNetworkTerminal), UDPStatus(
				false), _bufferLen(0), _sendSockAddr() {
	buffer = new INT8[BufferSizeMax];
	_sendMsgBuff = new INT8[SendBuffMaxSize];
	memset(buffer, 0, BufferSizeMax);
	memset(_sendMsgBuff, 0, SendBuffMaxSize);
	memset(&_sendSockAddr, 0, sizeof(struct sockaddr_in));
}

UDPNetTrans::~UDPNetTrans() {
	delete[] buffer;
	UDPStatus = true;
	delete[] _sendMsgBuff;
}

UDPNetTrans::UDPNetTrans(const UDPNetTrans &udpNet) :
		NetTrans(udpNet), setNetworkTerminal(udpNet.setNetworkTerminal), UDPStatus(
				udpNet.UDPStatus), _bufferLen(udpNet._bufferLen) {
	buffer = new INT8[BufferSizeMax];
	memcpy(buffer, udpNet.buffer, strlen(udpNet.buffer));
	_sendMsgBuff = new INT8[SendBuffMaxSize];
	memcpy(_sendMsgBuff, udpNet._sendMsgBuff, udpNet._bufferLen);
	memset(&_sendSockAddr, 0, sizeof(struct sockaddr_in));
	memcpy(&_sendSockAddr, &udpNet._sendSockAddr, sizeof(struct sockaddr_in));
}

UDPNetTrans &UDPNetTrans::operator=(const UDPNetTrans &udpNet) :
		NetTrans(udpNet), setNetworkTerminal(udpNet.setNetworkTerminal), UDPStatus(
				udpNet.UDPStatus), _bufferLen(udpNet._bufferLen) {

	buffer = new INT8[BufferSizeMax];
	memcpy(buffer, udpNet.buffer, strlen(udpNet.buffer));
	_sendMsgBuff = new INT8[SendBuffMaxSize];
	memcpy(_sendMsgBuff, udpNet._sendMsgBuff, udpNet._bufferLen);
	memset(&_sendSockAddr, 0, sizeof(struct sockaddr_in));
	memcpy(&_sendSockAddr, &udpNet._sendSockAddr, sizeof(struct sockaddr_in));
	return *this;
}

INT32 UDPNetTrans::socketRunThread() {
	pthread_t tid;
	struct timeval tv;
	if (pthread_create(&tid, NULL, pthreadStart, (void *) this) == 0) {
		gettimeofday(&tv, NULL);
		Logger::GetInstance().Info("System time is %d", (INT32) tv.tv_sec);
		Logger::GetInstance().Info("The upgrade soft version is %s.",
				UpgradeSoftVersion);
		printf("Create UDP thread successfully!........\n");
	} else
		;
	void *result;
	pthread_join(tid, &result);
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
	SmartPtr<HandleUp> upHandle(new HandleUp(*this));
	struct sockaddr_in recvAddr;
	SetNetworkTerminal netSet(*setNetworkTerminal);

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

			upHandle->_udpNet.setSockAddr(recvSendAddr);
			UINT32 id = upHandle->_udpNet.addEvent(5, 2, callBackFunc,
					&upHandle->_udpNet, true);
//			cout << "time id : " << id << "time id in Node "
//					<< eventQueue.getTopNode()->id << endl;
			upHandle->_udpNet.asyncStart();
			memcpy(&recvAddr, &recvSendAddr, sizeof(recvSendAddr));
			INT32 sockfd = m_socket;
			switch (CMDParserUp::parserPCRequestHead(buffer, ret_recv)) {
			case CMD_DEV_SEARCH: {
				cout
						<< "=====================dev search start================================"
						<< endl;
				Logger::GetInstance().Info(
						"Get dev search command from PC : %s !",
						inet_ntoa(recvAddr.sin_addr));
				NetTrans::printBufferByHex("recv dev : ", buffer,
						strlen(buffer));
				upHandle->devSearchCMDHandle(recvAddr, &netSet,
						*upFileAttrs.get(), sockfd);
				cout
						<< "=====================dev search end================================"
						<< endl;
			}/*end case 1*/
				break;
			case CMD_DEV_PARAMETER_SETTING: {
				cout << "test 10 " << endl;
				upHandle->devParamSetCMDHandle(recvAddr, buffer, &netSet,
						sockfd);
			}/*end case 2*/
				break;
			case CMD_DEV_UPGRADE: {
				upHandle->devUpgradePCRequestCMDHandle(recvAddr, buffer,
						&netSet, sockfd, *upFileAttrs.get(), fileTrans,
						request.get());
			}/*end case 3*/
				break;
			case CMD_DEV_FILE_TRANSPORT: {
				upHandle->devFileTransCMDHandle(recvAddr, buffer, &netSet,
						sockfd, *upFileAttrs.get(), fileTrans, request.get());
			}/*end case 4*/
				break;
			case CMD_DEV_UPGRADE_REPLY: {

			}
				break;
			case CMD_DEV_TESTMODE: {
				Logger::GetInstance().Info(
						"Get test mode change command from PC : %s !",
						inet_ntoa(recvAddr.sin_addr));
				upHandle->devTestModeCntCMDHandle(buffer);
			}/*end case 7*/
				break;
			case CMD_DEV_GETMASK: {
				upHandle->devGetMaskCMDHandle(recvAddr, &netSet, sockfd);
			}/*end case 8*/
				break;
			case CMD_DEV_REQUESTVERSION: {
				upHandle->devGetVersionCMDHandle(sockfd, recvAddr);
			}/*end case 9*/
				break;
			default: {
				Logger::GetInstance().Error("CMD unknow !");
			}
				break;
			}

		} /* end if FD_ISSET */
//		} /* end if */
	} /* end while(1) */
	return retOk;
}

void UDPNetTrans::callBackFunc(void*args) {
	UDPNetTrans *udpT = (UDPNetTrans*) args;
	udpT->indirectFunc();
}

void UDPNetTrans::indirectFunc(void) {
	if (eventQueue.getTopNode()->id == 5) {
		INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
		if (strlen(_sendMsgBuff) != 0) {
			INT32 retSend = sendto(m_socket, _sendMsgBuff, _bufferLen, 0,
					(struct sockaddr *) &_sendSockAddr, tmp_server_addr_len);
			cout << "Send data length : " << retSend << endl;
			if (retSend < 0) {
				Logger::GetInstance().Error("Send failed !");
				NetTrans::printBufferByHex("Buff : ", _sendMsgBuff, _bufferLen);
			}
		}
		struct timeval tv;
		gettimeofday(&tv, NULL);
//			cout << tv.tv_sec << " s and us : "<<tv.tv_usec<< endl;
		memset(_sendMsgBuff, 0, _bufferLen);

	}
}

void UDPNetTrans::UDPSendMsg(const void *msg, UINT32 len) {
	if (len > SendBuffMaxSize) {
		Logger::GetInstance().Error("Msg length %d is too long to send !", len);
		return;
	}
	memset(_sendMsgBuff, 0, SendBuffMaxSize);
	memcpy(_sendMsgBuff, msg, len);
	_bufferLen = len;

}
