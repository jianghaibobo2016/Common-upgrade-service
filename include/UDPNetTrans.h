#ifndef UDPNETTRANS_H
#define UDPNETTRANS_H
#include "NetTrans.h"
#include "SetNetwork.h"
#include <sys/socket.h>
#include <netinet/in.h>

class UDPNetTrans: public NetTrans {
public:
	UDPNetTrans(SetNetworkTerminal *setNetworkTerminal);
	~UDPNetTrans();

	SetNetworkTerminal *setNetworkTerminal;
	INT32 socketRunThread();
	static void *pthreadStart(void*);
	INT32 socketSelect();
protected:
private:
	INT8 *buffer;
	bool UDPStatus;
};
#endif /* UDPNETTRANS_H */
