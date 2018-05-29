#ifndef UDPNETTRANS_H
#define UDPNETTRANS_H
#include "NetTrans.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <SetNetworkWithServer.h>

class UDPNetTrans: public NetTrans {
public:
	UDPNetTrans(SetNetworkTerminal *setNetworkTerminal);
	~UDPNetTrans();

	UDPNetTrans(const UDPNetTrans &udpNet);
	UDPNetTrans &operator=(const UDPNetTrans &udpNet);

	SetNetworkTerminal *setNetworkTerminal;
	INT32 socketRunThread();
	static void *pthreadStart(void*);
	INT32 socketSelect();

	static void callBackFunc(void*args);
	void indirectFunc(void);
	void UDPSendMsg(const void *msg, UINT32 len);

protected:
private:
	INT8 *buffer;
	bool UDPStatus;

	void setSockAddr(const struct sockaddr_in &addr) {
		memset(&_sendSockAddr, 0, sizeof(sockaddr_in));
		memcpy(&_sendSockAddr, &addr, sizeof(sockaddr_in));
		cout << inet_ntoa(addr.sin_addr) << endl;
	}
	INT8 * _sendMsgBuff;
	UINT32 _bufferLen;
	struct sockaddr_in _sendSockAddr;

};
#endif /* UDPNETTRANS_H */
