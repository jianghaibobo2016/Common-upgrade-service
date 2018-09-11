#include <unistd.h>
#include <string.h>
#include <iostream>
#include <errno.h>
#include <arpa/inet.h>
#include "GlobDefine.h"
#include "Logger.h"
#include "NetTrans.h"
#include "UpgradeServiceConfig.h"
using namespace std;
using namespace FrameWork;
SocketException::SocketException(const std::string &message,
		bool inclSysMsg /* = true */) throw () :
		m_message(message), m_errorcode(0) {
	if (inclSysMsg) {
		m_errorcode = errno;
		m_message += ": ";
		m_message += strerror(m_errorcode);
	}
}

NetTrans::NetTrans(int domain, int type, int protocol) :
		Timer(1000), port(0) {
	if ((m_socket = ::socket(domain, type, protocol)) < 0)
		throw SocketException("Socket creation failed (socket)");
//	sleep(1);
}

NetTrans::NetTrans(const NetTrans &netTrans) :
		Timer(netTrans.getTick()) {
	m_socket = netTrans.m_socket;
	port = netTrans.port;
}

//NetTrans &NetTrans::operator=(const NetTrans &netT) {
//	setTick(netT.getTick());
//	m_socket = netT.m_socket;
//	port = netT.port;
//	return *this;
//}

NetTrans::~NetTrans() {
	cout << "Close socket!" << endl;
	close(m_socket);
}

INT32 NetTrans::socketBind(UINT16 localPort) {
	cout << "bind port: " << localPort << endl;
	struct sockaddr_in tmp_server_addr;
	memset((void *) &tmp_server_addr, 0, sizeof(struct sockaddr_in));
	tmp_server_addr.sin_family = AF_INET;
	tmp_server_addr.sin_addr.s_addr = htons(INADDR_ANY); /* INADDR_ANY */
	tmp_server_addr.sin_port = htons(localPort);
	//将地址结构绑定到套接字上./
	int ret = bind(m_socket, (struct sockaddr *) &tmp_server_addr,
			sizeof(tmp_server_addr));
	if (ret < 0) {
		Logger::GetInstance().Error(
				"%s() : Set of local address and port failed !", __FUNCTION__);
		throw SocketException(
				"Set of local address and port failed (sctp_bindx)");
		perror("bind error");
		return retError;
	}

	struct ip_mreq mreq;
	/*加入多播组*/
	mreq.imr_multiaddr.s_addr = inet_addr(MultiCastAddrRECV); /*多播地址*/
	mreq.imr_interface.s_addr = htonl(INADDR_ANY); /*网络接口为默认*/

	INT32 err = setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,
			sizeof(mreq));
	if (err < 0) {
		perror("setsockopt():IP_ADD_MEMBERSHIP");
		return retError;
	}
	//Disable loopback so you do not receive your own datagrams.
	{
		DP_C_S8 loopch = 0;
		if (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_LOOP,
				(DP_C_S8 *) &loopch, sizeof(loopch)) < 0) {
			perror("Setting IP_MULTICAST_LOOP error");
			close(m_socket);
			return retError;
		} else
			printf("Disabling the loopback...OK.\n");
	}
	return retOk;
}

void NetTrans::printBufferByHex(const INT8 *note, void *buff, UINT32 len) {
	printf("%s\n", note);
	INT32 iPos = 0;
	for (UINT32 iPos = 0; iPos < len; iPos++) {
		if (iPos % 10 == 0 && iPos != 0)
			printf("\n");
		printf("%02x ", ((UINT8 *) buff)[iPos]);
	}
	printf("\n");
}
