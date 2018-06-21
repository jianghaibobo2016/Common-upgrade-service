#ifndef UDPNETTRANS_H
#define UDPNETTRANS_H
#include "NetTrans.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <SetNetworkWithServer.h>
#include "Mutex.h"

//typedef struct _SOCKADDR_S_ {
//	_SOCKADDR_S_() :
//			_bSockRunFlag(false), _sendSockAddr() {
//	}
//	bool _bSockRunFlag;
//	struct sockaddr_in _sendSockAddr;
//} SOCKADDR_S;

//typedef struct _

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
	void UDPSendMsg(const void *msg, UINT32 len, struct sockaddr_in &addr);

protected:
private:
	INT8 *buffer;
	bool UDPStatus;

	static Mutex mutex;

//	map<UINT32, SOCKADDR_S> _mSockAddr;

//	UINT32 getIdleTimerID();
	void setSockAddr(const struct sockaddr_in &addr) {
		memset(&_sendSockAddr, 0, sizeof(sockaddr_in));
		memcpy(&_sendSockAddr, &addr, sizeof(sockaddr_in));
	}
	INT8 * _sendMsgBuff;
	UINT32 _bufferLen;
	struct sockaddr_in _sendSockAddr;

};
#endif /* UDPNETTRANS_H */
