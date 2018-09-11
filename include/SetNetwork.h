/*
 * SetNetwork.h
 *
 *  Created on: Mar 30, 2018
 *      Author: jhb
 */

#ifndef SETNETWORK_H_
#define SETNETWORK_H_

#include <string.h>
#include <vector>
#include <pthread.h>
#include "GlobDefine.h"
#include "UpgradeServiceConfig.h"
#include "Mutex.h"
using namespace std;

/************************static const variables*******************************/
/*For modifying INI file.*/
static const INT32 SUBNETFLAG = 1; /// check subnet
static const INT32 SETMAC = 1;
static const INT8 LEFT_BRACE = '[';
static const INT8 RIGHT_BRACE = ']';
/************************static const variables*******************************/

/****************************local struct*************************************/
/*For recording net configuration.*/
#pragma pack(push, 1)
typedef struct _NETWORKCONFIG_S_ {
	_NETWORKCONFIG_S_() :
			ipAddr(), netmaskAddr(), gatewayAddr(), macAddr() {
	}
	string ipAddr;
	string netmaskAddr;
	string gatewayAddr;
	string macAddr;
} NETWORKCONFIG;
#pragma pack(pop)
/****************************local struct*************************************/

/*****************************local enum**************************************/
/*For net status.*/
enum NetworkStatus {
	upStatus = 0, downStatus, errorStatus
};
/*****************************local enum**************************************/

/******************************clsaaes****************************************/
/*Class mem vars storage net data, with allocaing memory*/
class NetConfigTrans {
public:
	NetConfigTrans();
	~NetConfigTrans();

	const INT8 *getIPT() const {
		return ipT;
	}
	const INT8 *getSubmaskT() const {
		return submaskT;
	}
	const INT8 *getGatewayT() const {
		return gatewayT;
	}

	const INT32 getFlag() const {
		return flag;
	}

	bool setIPT(const INT8 *ip) {
		if (strlen(ip) > 18) {
			return false;
		}
		memset(ipT, 0, 18);
		memcpy(ipT, ip, strlen(ip));
		flag += 1;
		return true;
	}
	bool setSubmaskT(const INT8 *submask) {
		if (strlen(submask) > 18) {
			return false;
		}
		memset(submaskT, 0, 18);
		memcpy(submaskT, submask, strlen(submask));
		flag += 2;
		return true;
	}
	bool setgatewayT(const INT8 *gateway) {
		if (strlen(gateway) > 18) {
			return false;
		}
		memset(gatewayT, 0, 18);
		memcpy(gatewayT, gateway, strlen(gateway));
		flag += 3;
		return true;
	}

	INT32 flag;
private:
	INT8 *ipT;
	INT8 *submaskT;
	INT8 *gatewayT;

};

/*Get and set net configuration main class*/
class SetNetwork {
	static Mutex mutex;
public:

	SetNetwork();
	SetNetwork(const SetNetwork&);
	SetNetwork &operator=(const SetNetwork &setNet);
	~SetNetwork();

	const NETWORKCONFIG& getNetConfStruct() const {
		return m_netWorkConfig;
	}

	const INT8* getIfname() const {
		return IFNAME;
	}
	const NetworkStatus &getNetStatus() const {
		return networkStatus;
	}

	const bool &getInitSet() const {
		return initSet;
	}
	void setNetConfStruct(const NETWORKCONFIG& netConfig) {
		memset(&m_netWorkConfig, 0, sizeof(NETWORKCONFIG));
		memcpy(&m_netWorkConfig, &netConfig, sizeof(NETWORKCONFIG));
//		m_netWorkConfig = netConfig;
	}

	void setIfname(const INT8* ifname) {
		memset(IFNAME, 0, 8);
		memcpy(IFNAME, ifname, strlen(ifname));
	}
	void setNetStatus(const NetworkStatus &netConfig) {
		networkStatus = netConfig;
	}
	void setInitSet(bool set) {
		initSet = set;
	}

	bool getNetworkConfig();
	bool setNetworkConfig(const INT8 *ipaddr, const INT8 *subnet,
			const INT8 *gateway, const INT8 *macaddr, const INT8 *iniFile);
	bool upDownNetwork(NetworkStatus networkStatus);

	static INT8* castMacToChar13(INT8* macDest, string macaddr);
private:
	NETWORKCONFIG m_netWorkConfig;
	INT8* IFNAME;
	NetworkStatus networkStatus;
	bool initSet;
//	pthread_mutex_t mutex;

	static string ByteToHexString(const void *pData,
			int len/* , const string &split = "" */);
	bool setNet(INT32 mac, const INT8 *macaddr);
	bool setNet(const INT8 *ipaddr, const INT8 *subnet, const INT8 *gateway);
	INT32 stringToHex(const string &strNum);
	INT32 charToHex(const DP_C_S8 cNum);
	bool cSplitString(const string strKey, const string strChar,
			vector<string> &vMac);
};

class CheckNetConfig {
public:
	CheckNetConfig() {
	}
	~CheckNetConfig() {
	}
	static CheckNetConfig &GetInstance();
	bool checkSubnet(const INT8 *ipaddr, const INT8 *subnet);
	bool checkGateway(const INT8 *ipaddr, const INT8 *subnet,
			const INT8 *gateway);
	bool checkMAC(INT8* mac);
	bool checkIP(const INT8* ipaddr, const INT32 subnetFlag);

private:
};

/******************************clsaaes****************************************/
#endif /* SETNETWORK_H_ */
