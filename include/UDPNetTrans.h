#ifndef UDPNETTRANS_H
#define UDPNETTRANS_H
#include "NetTrans.h"
#include "SetNetwork.h"
#include <sys/socket.h>
#include <netinet/in.h>

class UDPNetTrans : public NetTrans
{
public:
  UDPNetTrans(SetNetworkTerminal *setNetworkTerminal);
  ~UDPNetTrans();

  SetNetworkTerminal *setNetworkTerminal;
  // INT32 netSocketInit();
  INT32 socketRunThread();
  static void *pthreadStart(void*);
  INT32 socketSelect();
  INT32 socketSendto(INT8 *sendtoBuff, size_t bufferLen, sockaddr_in clientAddr);

protected:
  // UDPNetTrans(const UDPNetTrans &);
private:
  INT8 *buffer;
  bool UDPStatus;
};
#endif /* UDPNETTRANS_H */
