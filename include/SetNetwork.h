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
#include "GlobDefine.h"
#include "UpgradeServiceConfig.h"
using namespace std;

/************************static const variables*******************************/
/*For modifying INI file.*/
static const INT32 SUBNETFLAG = 1;
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

//private:
	INT8 *ipT;
	INT8 *submaskT;
	INT8 *gatewayT;
};

/*Get and set net configuration main class*/
class SetNetwork {
public:

	SetNetwork();
	SetNetwork(const SetNetwork&);
	~SetNetwork();

	const NETWORKCONFIG& getNetConfStruct() const {
		return m_netWorkConfig;
	}

	const INT8* getIfname() const {
		return IFNAME;
	}

	void setNetConfStruct(const NETWORKCONFIG& netConfig) {
		m_netWorkConfig = netConfig;
	}

	void setIfname(const INT8* ifname) {
		memset(IFNAME, 0, 8);
		memcpy(IFNAME, ifname, strlen(ifname));
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

	static string ByteToHexString(const void *pData,
			int len/* , const string &split = "" */);
	bool setNet(INT32 mac, const INT8 *macaddr);
	bool setNet(const INT8 *ipaddr, const INT8 *subnet, const INT8 *gateway);
	INT32 stringToHex(const string &strNum);
	INT32 charToHex(const char cNum);
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

class IniConfigFile {
	friend class SetNetworkTerminal;
public:
	IniConfigFile();
	~IniConfigFile() {
	}
	const INT8 *iniFile;
	bool readIniConfFile(const INT8 *section, const INT8 *key, INT8 *value);
	INT32 setIniConfFile(const INT8 *section, const INT8 *key,
			const INT8 *value);
private:
	INT32 load_ini_file(const INT8 *file, INT8 *buf, INT32 *file_size);
	INT32 parse_file(const INT8 *section, const INT8 *key, const INT8 *buf,
			INT32 *sec_s, INT32 *sec_e, INT32 *key_s, INT32 *key_e,
			INT32 *value_s, INT32 *value_e);

	INT32 inline newline(INT8 c) {
		return ('\n' == c || '\r' == c) ? 1 : 0;
	}
	INT32 inline end_of_string(INT8 c) {
		return '\0' == c ? 1 : 0;
	}
	INT32 inline left_barce(INT8 c) {
		return LEFT_BRACE == c ? 1 : 0;
	}
	INT32 inline isright_brace(INT8 c) {
		return RIGHT_BRACE == c ? 1 : 0;
	}
};
/******************************clsaaes****************************************/
#endif /* SETNETWORK_H_ */
