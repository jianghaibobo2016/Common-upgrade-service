#ifndef SETNETWORK_H
#define SETNETWORK_H
#include "SetNetwork.h"

class NetConfigTransWithServer: public NetConfigTrans {
public:
	NetConfigTransWithServer();
	~NetConfigTransWithServer();

	INT8 *serverIPT;
	INT8 *serverPortT;

	INT8 *MAC;
	INT8 *MASK;
};

class SetNetworkTerminal: public SetNetwork {
public:

	SetNetworkTerminal();
	SetNetworkTerminal(const SetNetworkTerminal&);
	~SetNetworkTerminal();

	INT8 *getPCIP() const {
		return PCIP;
	}
	void setPCIP(const INT8 *pcip) {
		memset(PCIP, 0, 32);
		memcpy(PCIP, pcip, strlen(pcip));
	}

	bool setServerNetConfig(const string &ip, const UINT16 &port);
private:
	INT8 *PCIP;
};

#endif /* SETNETWORK_H */
