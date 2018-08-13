/*
 * NetConfigTransWithServer.h
 *
 *  Created on: Mar 30, 2018
 *      Author: jhb
 */
#ifndef SETNETWORK_H
#define SETNETWORK_H
#include "SetNetwork.h"
#include "NetTrans.h"

static const INT8 DefaultChar = 'D';

class NetConfigTransWithServer: public NetConfigTrans {
public:
	NetConfigTransWithServer();
	~NetConfigTransWithServer();

	const INT8 *getServerIPT() const {
		return serverIPT;
	}
	const INT8 *getCommunicationPort() const {
		return CommunicationPort;
	}
	const UINT16 getRecordingPort() const {
		return RecordingPort;
	}
	const INT8 *getName() const {
		return Name;
	}
	const INT8 getCastMode() const {
		return castMode;
	}

	bool setServerIPT(const INT8*ip) {
		if (strlen(ip) > 18) {
			return false;
		}
		memset(serverIPT, 0, 18);
		memcpy(serverIPT, ip, strlen(ip));
		flag += 4;
		return true;
	}

	bool setCommunicationPort(const INT8*port) {
		if (strlen(port) > 5) {
			return false;
		}
		memset(CommunicationPort, 0, 5);
		memcpy(CommunicationPort, port, strlen(port));
		flag += 5;
		return true;
	}

	bool setRecordingPort(const UINT16 port) {
		RecordingPort = port;
		flag += 6;
		return true;
	}
	bool setName(const INT8*name) {
		if (strlen(name) > 60) {
			return false;
		}
		memset(Name, 0, 60);
		memcpy(Name, name, strlen(name));
		flag += 7;
		return true;
	}
	bool setCastMode(INT8 mode) {
		memset(&castMode, 0, 1);
		memcpy(&castMode, &mode, 1);
		flag += 8;
		return true;
	}

private:

	INT8 *serverIPT;
	INT8 *CommunicationPort;
	UINT16 RecordingPort;
	INT8 *Name;
	INT8 castMode;

};

class InitSetConf {
public:
	InitSetConf();
	~InitSetConf();

	const INT8 *getMAC() const {
		return MAC;
	}
	const INT8* getMASK() const {
		return MASK;
	}
	const INT32 getFlag() const {
		return flag;
	}
	bool setMAC(const INT8 *mac) {
		if (strlen(mac) > 13) {
			return false;
		}
		memset(MAC, 0, 13);
		memcpy(MAC, mac, strlen(mac));
		flag += 1;
		return true;
	}

	bool setMASK(const INT8 *mask) {
		if (strlen(mask) > 8) {
			return false;
		}
		memset(MASK, 0, 8);
//		for (INT32 i = 0; i < 4; i + 2)
//			sscanf(&MASK[i], "%hu", &mask[i * 2]);
		memcpy(MASK, mask, 8);
		NetTrans::printBufferByHex("set MASK in set mask : ", MASK, 8);
		flag += 2;
		return true;
	}

private:
	INT8 *MAC;
	INT8 *MASK;

	INT32 flag;
};

class SetNetworkTerminal: public SetNetwork {
public:

	SetNetworkTerminal();
	SetNetworkTerminal(const SetNetworkTerminal&);
	SetNetworkTerminal &operator=(const SetNetworkTerminal &setNet);
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
