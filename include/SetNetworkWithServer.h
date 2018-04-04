#ifndef SETNETWORK_H
#define SETNETWORK_H
#include "SetNetwork.h"

class NetConfigTransWithServer : public NetConfigTrans{
public:
	NetConfigTransWithServer();
	~NetConfigTransWithServer();

	INT8 *serverIPT;
	INT8 *serverPortT;
};

class SetNetworkTerminal: public SetNetwork {
public:

	SetNetworkTerminal();
	SetNetworkTerminal(const SetNetworkTerminal&);
	~SetNetworkTerminal();

	bool setServerNetConfig(const string &ip, const UINT16 &port);
private:
};

#endif /* SETNETWORK_H */
