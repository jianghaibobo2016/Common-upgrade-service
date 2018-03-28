#ifndef SETNETWORK_H
#define SETNETWORK_H
#include <string>
#include <vector>
#include "GlobDefine.h"
using namespace std;

#ifdef FILE_LINE
#undef FILE_LINE
#endif
#ifndef FILE_LINE
#define FILE_LINE (LOG(DEBUG) << "check")
#endif

// static const INT8 *IFNAME = "eth0";

static const INT32 SUBNETFLAG = 1;
static const INT32 SETMAC = 1;
static const INT8 LEFT_BRACE = '[';
static const INT8 RIGHT_BRACE = ']';

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

enum NetworkStatus {
	upStatus = 0, downStatus, errorStatus
};

class NetConfigTrans {
public:
	NetConfigTrans();
	~NetConfigTrans();

	INT8 *ipT;
	INT8 *submaskT;
	INT8 *gatewayT;
	INT8 *serverIPT;
	INT8 *serverPortT;
};

class SetNetwork {
public:
	SetNetwork() {
	}
	virtual ~SetNetwork() {
	}

	virtual bool getNetworkConfig() = 0;
	virtual bool setNetworkConfig(const INT8 *ipaddr, const INT8 *subnet,
			const INT8 *gateway, const INT8 *macaddr, const INT8 *iniFile) = 0;
};

class SetNetworkTerminal: public SetNetwork {
public:

	NETWORKCONFIG m_netWorkConfig;
	INT8* IFNAME;

	SetNetworkTerminal();
	SetNetworkTerminal(const SetNetworkTerminal&);
	~SetNetworkTerminal();
	bool getNetworkConfig();
	bool setNetworkConfig(const INT8 *ipaddr, const INT8 *subnet,
			const INT8 *gateway, const INT8 *macaddr, const INT8 *iniFile);
	bool setServerNetConfig(const string &ip, const UINT16 &port);
	bool upDownNetwork(NetworkStatus networkStatus);

	static INT8* castMacToChar13(INT8* macDest, string macaddr);
private:
	static string ByteToHexString(const void *pData,
			int len/* , const string &split = "" */);
	NetworkStatus networkStatus;
	bool setNet(INT32 mac, const INT8 *macaddr);
	bool setNet(const INT8 *ipaddr, const INT8 *subnet, const INT8 *gateway);
	INT32 stringToHex(const string &strNum);
	INT32 charToHex(const char cNum);
	bool cSplitString(const string strKey, const string strChar,
			vector<string> &vMac);
};

class CheckNetConfig {
	// friend class SetNetworkTerminal;/* class SetNetworkTerminal can call private func */
public:

	CheckNetConfig() {
	}
	;
	~CheckNetConfig() {
	}
	;
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
	// IniConfigFile(INT8 *iniFile);
	~IniConfigFile() {
	}
	;
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

#endif /* SETNETWORK_H */
