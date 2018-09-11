/*
 * SetNetwork.cpp
 *
 *  Created on: Mar 30, 2018
 *      Author: jhb
 */
#include <string.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/route.h>
#include "Logger.h"
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if_arp.h> /* ARPHRD_ETHER */
#include "SetNetwork.h"
#include "UpgradeServiceConfig.h"
#include "NetTrans.h"
#include "IniConfigFile.h"

using namespace std;
using namespace FrameWork;

/* define */
#define DOWNNET                                                                                             \
if ((upDownNetwork(downStatus) != true))                                                                    \
{                                                                                                           \
    Logger::GetInstance().Error("%s() : Down %s failed ! LINE : %d", __FUNCTION__, IFNAME, __LINE__);       \
    return false;                                                                                           \
}

#define UPNET                                                                                               \
if ((upDownNetwork(upStatus) != true))                                                                      \
{                                                                                                           \
    Logger::GetInstance().Error("%s() : Up %s failed ! LINE : %d", __FUNCTION__, IFNAME, __LINE__);         \
    return false;                                                                                           \
}
#define RESTOREMAC                                                                                          \
if (setNet(SETMAC, this->m_netWorkConfig.macAddr.c_str()) != true)                                          \
{                                                                                                           \
    Logger::GetInstance().Error("%s() : Restore MAC ! LINE : %d", __FUNCTION__, __LINE__);                  \
    return false;                                                                                           \
}

#define RESTORENET                                                                                          \
if (setNet(this->m_netWorkConfig.ipAddr.c_str(),                                                            \
           this->m_netWorkConfig.netmaskAddr.c_str(), this->m_netWorkConfig.gatewayAddr.c_str()) != true)   \
{                                                                                                           \
    Logger::GetInstance().Error("%s() : Restore network failed ! LINE : %d", __FUNCTION__, __LINE__);       \
    return false;                                                                                           \
}else                                                                                                       \
    Logger::GetInstance().Info("%s() : Restore network ! LINE : %d", __FUNCTION__, __LINE__);

#define SETMACADDR                                                                                          \
if (setNet(SETMAC, macaddrSet) != true)                                                                     \
{                                                                                                           \
    UPNET;                                                                                                  \
    RESTORENET;                                                                                             \
    Logger::GetInstance().Error("%s() : Set MAC ! LINE : %d", __FUNCTION__, __LINE__);                      \
    return false;                                                                                           \
}

#define SETMACINTOINI                                                                                       \
if (iniConfFile.setIniConfFile("NETWORK", "macaddr", macaddrSet) != retOk)                                     \
{                                                                                                           \
    Logger::GetInstance().Error("%s() : Set MAC into ini file ! LINE : %d", __FUNCTION__, __LINE__);        \
    DOWNNET                                                                                                 \
    RESTOREMAC;                                                                                             \
    UPNET;                                                                                                  \
    return false;                                                                                           \
}

Mutex SetNetwork::mutex;

SetNetwork::SetNetwork() :
		m_netWorkConfig(), IFNAME(NULL), networkStatus(errorStatus), initSet(
				false)/*,mutex(PTHREAD_MUTEX_INITIALIZER)*/{
	IFNAME = new INT8[8];
	memset(IFNAME, 0, 8);
}
//???
SetNetwork::SetNetwork(const SetNetwork& setNet) :
		networkStatus(errorStatus) {
	m_netWorkConfig = setNet.m_netWorkConfig;
	IFNAME = new INT8[10];
	memset(IFNAME, 0, 8);
	if (IFNAME != NULL)
		strcpy(IFNAME, setNet.IFNAME);
	initSet = setNet.initSet;
}

SetNetwork &SetNetwork::operator=(const SetNetwork &setNet) {
	if (this != &setNet) {
		setNetConfStruct(setNet.getNetConfStruct());
		setIfname(setNet.getIfname());
		setNetStatus(setNet.getNetStatus());
		setInitSet(setNet.getInitSet());
	}
	return *this;
}

SetNetwork::~SetNetwork() {
	delete IFNAME;
}

bool SetNetwork::getNetworkConfig() {
	cout << "This lock of auto lock of func :  " << __FUNCTION__ << "()"
			<< endl;
	AutoLock autoLock(&mutex);
	INT32 sock;
	struct sockaddr_in sin;
	struct ifreq ifr;
	FILE *fp;
	INT8 buf[256]; // 128 is enough for linux
	INT8 iface[16];
	unsigned int dest_addr, gate_addr;
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		FILE_LINE;
		Logger::GetInstance().Error("Create sock fail : %s !", strerror(errno));
		return false;
	}
	memset(&ifr, 0, sizeof(ifr));
	if (IFNAME == NULL) {
		Logger::GetInstance().Error("Please set IFNAME !");
		close(sock);
		return false;
	}

	//No such file or directory ! ioctl()
	//mac addr
	memset(&ifr,0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, IFNAME, IFNAMSIZ - 1);
	INT32 retIO = ioctl(sock, SIOCGIFHWADDR, &ifr);
	if (retIO < 0) {
		FILE_LINE;
		Logger::GetInstance().Error("Get MAC fail : %s !", strerror(errno));
		close(sock);
		return false;
	} else {
//		 printf("%02x\n", ifr.ifr_hwaddr.sa_data[3]);
		m_netWorkConfig.macAddr = ByteToHexString(ifr.ifr_hwaddr.sa_data, 6);
	}
//	cout << "mac in func " << m_netWorkConfig.macAddr << endl;
	//ip addr
	if (ioctl(sock, SIOCGIFADDR, &ifr) != 0) {

		FILE_LINE;
		Logger::GetInstance().Error("Get IP fail : %s !", strerror(errno));
		close(sock);
		return false;
	} else {

		memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
		m_netWorkConfig.ipAddr = inet_ntoa(sin.sin_addr);
	}

	//net mask
	if (ioctl(sock, SIOCGIFNETMASK, &ifr) < 0) {
		FILE_LINE;
		Logger::GetInstance().Error("Get NETMASK fail : %s !", strerror(errno));
		close(sock);
		return false;
	} else {
		memcpy(&sin, &ifr.ifr_netmask, sizeof(sin));
		m_netWorkConfig.netmaskAddr = inet_ntoa(sin.sin_addr);
	}
	close(sock);
	//gateway

	fp = fopen("/proc/net/route", "r");
	if (fp == NULL) {
		FILE_LINE;
		Logger::GetInstance().Error("Get NETMASK fail : %s !", strerror(errno));
		return false;
	}
	/* Skip title line */
	if (!fgets(buf, sizeof(buf), fp))
		return false;
	INT32 countOut = 20;
	while (fgets(buf, sizeof(buf), fp)) {
		countOut--;
		if (sscanf(buf, "%s\t%lX\t%lX", iface, (long unsigned int *) &dest_addr,
				(long unsigned int *) &gate_addr) != 3 || dest_addr != 0) {
			if (countOut >= 0)
				continue;
			else {
				fclose(fp);
				return false;
			}
		}
		struct in_addr addr;

		memcpy(&addr, &gate_addr, sizeof(gate_addr));
		m_netWorkConfig.gatewayAddr = inet_ntoa(addr);
		break;
	} /* while */
	fclose(fp);
	return true;

} /* getNetworkConfig */
INT8* SetNetwork::castMacToChar13(INT8 *macDest, string macaddr) {
	mutex.Lock();
	// sprintf(macResAddress, "%02x-%02x-%02x-%02x-%02x-%02x", macAddress[0]&0xff, macAddress[1]&0xff, macAddress[2]&0xff, macAddress[3]&0xff, macAddress[4]&0xff, macAddress[5]&0xff);
	string strMACAddress = macaddr;
	int j = 0;
	for (size_t i = 0; i <= strMACAddress.size(); i++) {
		if (i == strMACAddress.size()) {
			macDest[j] = '\0';
			break;
		}
		if (strMACAddress[i] != '-') {
			if (strMACAddress[i] >= 'a' && strMACAddress[i] <= 'f') {
				strMACAddress[i] = strMACAddress[i] - ' ';
			}
			macDest[j] = strMACAddress[i];
			j++;
		}
	}
	mutex.Unlock();
	cout << macDest << endl;
	return macDest;
}

/******************************************************************************
 * Description :
 Set network according inputs.
 IniFile is a configuration file of network.
 * Return Value : On success ,it return true. On error, false is returned.
 * Author : JHB
 * Create Data : 04/01/2018
 * Revision History :
 *   04/01/2018  JHB    Created.
 *****************************************************************************/
bool SetNetwork::setNetworkConfig(const INT8 *ipaddr, const INT8 *subnet,
		const INT8 *gateway, const INT8* macaddr, const INT8* iniFile) {
	CheckNetConfig checkNetConfig;
	INT8 m_ipaddr[20] = { 0 };
	if (ipaddr != NULL)
		memcpy(m_ipaddr, ipaddr, strlen(ipaddr));
	INT8 m_subnet[20] = { 0 };
	if (subnet != NULL)
		memcpy(m_subnet, subnet, strlen(subnet));
	INT8 m_gateway[20] = { 0 };
	if (gateway != NULL)
		memcpy(m_gateway, gateway, strlen(gateway));
	INT8 m_macaddr[40] = { 0 };
	if (macaddr != NULL) {
		memcpy(m_macaddr, macaddr, strlen(macaddr));
	}
	/* args check */
	if (strlen(m_ipaddr) == 0 && strlen(m_macaddr) == 0) {
		Logger::GetInstance().Error("None input!");
		return false;
	}
	if (iniFile == NULL) {
		Logger::GetInstance().Error("%s() :None ini file input!", __FUNCTION__);
		return false;
	}
	if (!initSet) {
		// SetNetworkTerminal setNet;
		if (this->getNetworkConfig() != true) {
			Logger::GetInstance().Error("%s() :Get network config failed !",
					__FUNCTION__);
			return false;
		}
	}
	IniConfigFile iniConfFile(INIFILE);
	/*  */
	Logger::GetInstance().Info(
			"%s() :Get net setting request:\nIP:\t%s\nNETMASK:\t%s\nGATEWA:\t%s\nMAC:\t%s\n",
			__FUNCTION__, m_ipaddr, m_subnet, m_gateway, m_macaddr);
//	sync();

	if (strlen(m_macaddr) == 0) {
		cout << "check restart prog 2 " << endl;
		if (m_subnet == NULL) {
			if (checkNetConfig.checkIP(m_ipaddr, 0) != true) {
				Logger::GetInstance().Error("%s() : Ip input is incorrect !",
						__FUNCTION__);
				return false;
			}
			if (setNet(m_ipaddr, this->m_netWorkConfig.netmaskAddr.c_str(),
					this->m_netWorkConfig.gatewayAddr.c_str()) != true) {
				RESTORENET;
				Logger::GetInstance().Error(
						"%s %s() : Set ip netmask m_gateway !",
						__FILE__, __FUNCTION__);
				return false;
			}
			if (iniConfFile.setIniConfFile("NETWORK", "ipaddr", m_ipaddr)
					!= retOk) {
				Logger::GetInstance().Error("%s() : Set ip into ini file !",
						__FUNCTION__);
				RESTORENET;
				return false;
			}
		} else if (m_subnet != NULL && m_gateway != NULL) {
			if (checkNetConfig.checkGateway(m_ipaddr, m_subnet, m_gateway)
					!= true) {
				Logger::GetInstance().Error(
						"%s() : Ip netmask gateyway input is incorrect !",
						__FUNCTION__);
				return false;
			}
			if (setNet(m_ipaddr, m_subnet, m_gateway) != true) {
				RESTORENET;
				Logger::GetInstance().Error("%s() : Set ip netmask gateway !",
						__FUNCTION__);
				return false;
			}
			if ((iniConfFile.setIniConfFile("NETWORK", "ipaddr", m_ipaddr)
					!= retOk)
					|| (iniConfFile.setIniConfFile("NETWORK", "netmask",
							m_subnet) != retOk)
					|| (iniConfFile.setIniConfFile("NETWORK", "gateway",
							m_gateway) != retOk)) {
				Logger::GetInstance().Error(
						"%s() : Set network config into ini file !",
						__FUNCTION__);
				RESTORENET;
				return false;
			}
		} else
			return false;
	} else {
		if (strlen(m_macaddr) != 12) {
			Logger::GetInstance().Error("%s() :Mac input is incorrect !",
					__FUNCTION__);
		}
		//modify macCheck::20aabbcc..--->> 20:aa:bb:cc...
		INT8 macCheck[18] = { 0 };
		UINT32 i, iPos = 0;
		for (i = 0; i < strlen(m_macaddr); i++) {
			if (i != 0 && i % 2 == 0) {
				memcpy(&macCheck[iPos], ":", 1);
				iPos++;
				memcpy(&macCheck[iPos], &m_macaddr[i], 1);
				iPos++;
			} else {
				memcpy(&macCheck[iPos], &m_macaddr[i], 1);
				iPos++;
			}
		}
		INT8 macaddrSet[18] = { 0 };
		memcpy(macaddrSet, macCheck, strlen(macCheck));
		if (checkNetConfig.checkMAC(macCheck) != true) {
			Logger::GetInstance().Error("%s() : Mac input is incorrect !",
					__FUNCTION__);
			return false;
		}
		if (strlen(m_ipaddr) != 0) {
			if (strlen(m_subnet) == 0) {
				if (checkNetConfig.checkIP(m_ipaddr, 0) != true) {
					Logger::GetInstance().Error(
							"%s() : Ip input is incorrect !", __FUNCTION__);
					return false;
				}
				DOWNNET
				SETMACADDR;
				UPNET;
				SETMACINTOINI;
				if (setNet(m_ipaddr, this->m_netWorkConfig.netmaskAddr.c_str(),
						this->m_netWorkConfig.gatewayAddr.c_str()) == false) {
					DOWNNET
					RESTOREMAC;
					UPNET;
					if (iniConfFile.setIniConfFile("NETWORK", "macaddr",
							this->m_netWorkConfig.macAddr.c_str()) != retOk) {
						Logger::GetInstance().Error(
								"%s() : Set MAC into ini file !", __FUNCTION__);

						return false;
					}
					RESTORENET;
					Logger::GetInstance().Error("%s() : Set ip !",
							__FUNCTION__);
					return false;
				}
				if (iniConfFile.setIniConfFile("NETWORK", "ipaddr", m_ipaddr)
						!= retOk) {
					Logger::GetInstance().Error("%s() : Set ip into ini file !",
							__FUNCTION__);
					DOWNNET
					RESTOREMAC;
					UPNET;
					RESTORENET;
					return false;
				}
			} else if (strlen(m_subnet) != 0 && strlen(m_gateway) != 0) {
				if (checkNetConfig.checkGateway(m_ipaddr, m_subnet, m_gateway)
						!= true) {
					Logger::GetInstance().Error(
							"%s() : Ip netmask gateway input is incorrect !",
							__FUNCTION__);
					return false;
				}
				DOWNNET
				SETMACADDR;
				UPNET;
				SETMACINTOINI;
				if (setNet(m_ipaddr, m_subnet, m_gateway) != true) {
					DOWNNET
					RESTOREMAC;
					UPNET;
					RESTORENET;
					Logger::GetInstance().Error(
							"%s() : Set ip netmask gateway !", __FUNCTION__);
					return false;
				}

				if ((iniConfFile.setIniConfFile("NETWORK", "ipaddr", m_ipaddr)
						!= retOk)
						|| (iniConfFile.setIniConfFile("NETWORK", "netmask",
								m_subnet) != retOk)
						|| (iniConfFile.setIniConfFile("NETWORK", "gateway",
								m_gateway) != retOk)) {
					Logger::GetInstance().Error(
							"%s() : Set network config into ini file !",
							__FUNCTION__);
					DOWNNET
					RESTOREMAC;
					UPNET;
					RESTORENET;
					return false;
				}
			} else {
				Logger::GetInstance().Error(
						"%s() : Need netmask and gateway input !",
						__FUNCTION__);
				return false;
			}
		} else {
			DOWNNET
			SETMACADDR;
			UPNET;
			SETMACINTOINI;
			RESTORENET;
		}
	}
	return true;
}

bool SetNetwork::setNet(const INT8 *ipaddr, const INT8 *subnet,
		const INT8 *gateway) {
	if (ipaddr == NULL || subnet == NULL || gateway == NULL)
		return false;
	/* ipaddr */
	INT32 sockfd = 0;
	INT32 retIOCTL = 0;
	struct ifreq ifr;
	struct sockaddr_in *sin;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		Logger::GetInstance().Error("Socket : %s !", strerror(errno));
		return false;
	}
	int nOptval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &nOptval,
			sizeof(int)) < 0) {
	}
	memset(&ifr, 0, sizeof(ifr));
	if (IFNAME == NULL) {
		Logger::GetInstance().Error("Please set IFNAME !");
		close(sockfd);
		return false;
	}
	strcpy(ifr.ifr_name, IFNAME);
	sin = (struct sockaddr_in *) &ifr.ifr_addr;
	sin->sin_family = AF_INET;

	if (inet_aton((const INT8 *) &ipaddr, &(sin->sin_addr)) < 0) {
		Logger::GetInstance().Error("Func inet_aton !");
		close(sockfd);
		return false;
	}

	sin->sin_addr.s_addr = inet_addr(ipaddr);
	retIOCTL = ioctl(sockfd, SIOCSIFADDR, &ifr);
	if (retIOCTL < 0) {

		Logger::GetInstance().Error("%s() : Func ioctl (IP) !", __FUNCTION__);
		close(sockfd);
		return false;
	}

	if (inet_aton((const INT8 *) &subnet, &(sin->sin_addr)) < 0) {
		Logger::GetInstance().Error("Func inet_aton !");
		close(sockfd);
		return false;
	}
	sin->sin_addr.s_addr = inet_addr(subnet);

	retIOCTL = ioctl(sockfd, SIOCSIFNETMASK, &ifr);
	if (retIOCTL < 0) {

		Logger::GetInstance().Error("Func ioctl (netmask) !");
		close(sockfd);
		return false;
	}
	/* subnet */

	/* gateway */
	struct rtentry rt;

	/* Set default gateway */
	memset(&rt, 0, sizeof(rt));

	rt.rt_dst.sa_family = AF_INET;
	((struct sockaddr_in *) &rt.rt_dst)->sin_addr.s_addr = 0;

	rt.rt_gateway.sa_family = AF_INET;
	((struct sockaddr_in *) &rt.rt_gateway)->sin_addr.s_addr = inet_addr(
			gateway);

	rt.rt_genmask.sa_family = AF_INET;
	((struct sockaddr_in *) &rt.rt_genmask)->sin_addr.s_addr = 0;

	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	retIOCTL = ioctl(sockfd, SIOCADDRT, &rt);
	if ((retIOCTL < 0) && (errno != 17)) {
		Logger::GetInstance().Error("Func ioctl (gateway) %s!",
				strerror(errno));
		close(sockfd);
		return false;
	}

	close(sockfd);
	return true;
	/* gateway */
}
bool SetNetwork::setNet(INT32 mac, const INT8 *macaddr) {
	struct ifreq temp;
	struct sockaddr *addr;
	cout << "mac before stringToHex " << macaddr << " " << __FUNCTION__ << " "
			<< __LINE__ << endl;
	INT32 fd = 0;
	INT32 ret = -1;
	INT8 tmpMac[6] = { 0 };

	const string sChar = ":";
	string strMac = macaddr;
	vector<string> vMac;
	if (!cSplitString(strMac, sChar, vMac)) {
		cout << "Get mac error --FormGetDevValues" << endl;
		return false;
	}
	for (INT32 i = 0; i < 6; i++) {
		tmpMac[i] = stringToHex(vMac.operator[](i));
//		printf("%02x\n", tmpMac[i]);
	}

	if ((0 != getuid()) && (0 != geteuid()))
		return false;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return false;
	}
	int nOptval = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void*) &nOptval,
			sizeof(int)) < 0) {
	}
	strcpy(temp.ifr_name, IFNAME);
	addr = (struct sockaddr *) &temp.ifr_hwaddr;

	addr->sa_family = ARPHRD_ETHER;
	memcpy(addr->sa_data, tmpMac, 6);
	ret = ioctl(fd, SIOCSIFHWADDR, &temp);
	if (ret != 0) {
		Logger::GetInstance().Error("%s() : ioctl error !", __FUNCTION__);
		close(fd);
		return false;
	}

	close(fd);
	return true;
}
string SetNetwork::ByteToHexString(const void *pData, INT32 len) {
	const string &split = "";
	INT8 strNum[100] = { 0 };
	string strResult;
	for (INT32 i = 0; i < len; i++) {
		if (i == len - 1){
			sprintf(strNum, "%02x%s", *((UINT8 *) pData + i), split.c_str());
//			cout << "strNum:: "<<strNum<<endl;
		}
		else{
			sprintf(strNum, "%02x-%s", *((UINT8 *) pData + i), split.c_str());
//			cout << "strNum:: "<<strNum<<endl;
		}

		strResult += strNum;
	}
//	cout << "strresult : " << strResult.c_str() << endl;
	// sprintf(strResult, "%02x-%02x-%02x-%02x-%02x-%02x", macAddress[0] & 0xff, macAddress[1] & 0xff, macAddress[2] & 0xff, macAddress[3] & 0xff, macAddress[4] & 0xff, macAddress[5] & 0xff);
	return strResult;
}

bool SetNetwork::upDownNetwork(NetworkStatus networkStatus) {
	INT32 fd, rtn;
	struct ifreq ifr;

	if (!this->IFNAME) {
		Logger::GetInstance().Error("%s() : Please set IFNAME !", __FUNCTION__);
		return false;
	}

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		Logger::GetInstance().Error("Socket : %s !", strerror(errno));
		return false;
	}

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, (const INT8 *) this->IFNAME, IFNAMSIZ - 1);

	if ((rtn = ioctl(fd, SIOCGIFFLAGS, &ifr)) == 0) {
		if (networkStatus == downStatus)
			ifr.ifr_flags &= ~IFF_UP;
		else if (networkStatus == upStatus)
			ifr.ifr_flags |= IFF_UP;
		else {
			Logger::GetInstance().Error("%s() : Wrong up ro down status !",
					__FUNCTION__);
			return false;
		}
	}

	if ((rtn = ioctl(fd, SIOCSIFFLAGS, &ifr)) != 0) {
		Logger::GetInstance().Error("%s() : ioctl error !", __FUNCTION__);
		return false;
	}

	close(fd);

	return true;
}

CheckNetConfig &CheckNetConfig::GetInstance() {
	static CheckNetConfig _checkNetConf;
	return _checkNetConf;
}
bool CheckNetConfig::checkIP(const INT8* ipaddr, const INT32 subnetFlag) {
	const INT8* ptrIP = ipaddr;
	cout << "ptrIP: " << ptrIP << endl;
	const INT8* dot = ".";
	INT8** section;
	section = new INT8 *[4];
	for (INT32 i = 0; i < 4; i++) {
		section[i] = new INT8[3];
		memset(section[i], 0, 3);
	}

	// INT8*
	INT32 dotIndex = 0, chIndex = 0, ipLen = strlen(ptrIP);
	if (NULL == ipaddr) {
		Logger::GetInstance().Error("None input1!");
		delete[] section;
		return false;
	}

	/* segmentation  and check number*/
	do {

		if (ptrIP[0] == ' ') {
			Logger::GetInstance().Error("Error input2!");
			delete[] section;
			return false;
		}

		if (strncmp(ptrIP, dot, 1) != 0) {

			if (isdigit(ptrIP[0])) {
				if (dotIndex > 3 || chIndex > 2) {
					Logger::GetInstance().Error("Error input3!");
					delete[] section;
					return false;
				}
				memset(&section[dotIndex][chIndex], ptrIP[0], 1);

				ptrIP += 1;
				chIndex++;
				ipLen--;
			} else if (ptrIP[0] == '\0')
				break;
			else {
				Logger::GetInstance().Error("Error input4!");
				delete[] section;
				return false;
			}
		} else {
			ptrIP += 1;
			dotIndex++;
			chIndex = 0;
		}
	} while (ipLen > 0);

	if (dotIndex != 3) {
		Logger::GetInstance().Error("Error input5!");
		delete[] section;
		return false;
	}

	/* check number */
	while (dotIndex >= 0) {
		if (subnetFlag == SUBNETFLAG) {
			if (atoi(section[dotIndex]) > 255 || atoi(section[dotIndex]) < 0
					|| strlen(section[dotIndex]) == 0) {
				Logger::GetInstance().Error("Error input6!");
				delete[] section;
				return false;
			}
		} else {
//			section[dotIndex] = '\0';

//			cout << "check 11111: " << section[dotIndex] << "xiaoyu0"
//					<< atoi(section[dotIndex]) << "len:"
//					<< strlen(section[dotIndex]) << endl;
//			cout << "check : " << atoi(section[dotIndex]) << "xiaoyu0"
//					<< atoi(section[dotIndex]) << "len:"
//					<< strlen(section[dotIndex]) << endl;
			if (atoi(section[dotIndex]) >= 255 || atoi(section[dotIndex]) < 0
					|| strlen(section[dotIndex]) == 0) {
				Logger::GetInstance().Error("Error input7!");
				delete[] section;
				return false;
			}
		}
		dotIndex--;
	}
	if (atoi(section[0]) == 0 && atoi(section[1]) == 0 && atoi(section[2]) == 0
			&& atoi(section[3]) == 0) {
		Logger::GetInstance().Error("Net address is invalid!");
		delete[] section;
		return false;
	}

	//?inet_addr() ???
	if (subnetFlag != SUBNETFLAG) {
		if (INADDR_NONE == inet_addr(ipaddr)) {
			Logger::GetInstance().Error("Net address is invalid!");
			delete[] section;
			return false;
		}
	}
	delete[] section;
	return true;
}

bool CheckNetConfig::checkSubnet(const INT8 *ipaddr, const INT8 *subnet) {
	if (subnet == NULL) {
		Logger::GetInstance().Error("None input!");
		return false;
	}
	if (!checkIP(ipaddr, 0) || !checkIP(subnet, SUBNETFLAG))
		return false;

	UINT32 i = 0, hostsP1 = 0, n[4] = { 0 };
	sscanf(subnet, "%u.%u.%u.%u", &n[3], &n[2], &n[1], &n[0]);
	for (i = 0; i < 4; ++i)
		hostsP1 += n[i] << (i * 8);
	hostsP1 = ~hostsP1 + 1;
	if ((hostsP1 & (hostsP1 - 1)) != 0)
		return false;

	return true;
}
bool CheckNetConfig::checkGateway(const INT8 *ipaddr, const INT8 *subnet,
		const INT8 *gateway) {
	if (ipaddr == NULL || subnet == NULL || gateway == NULL) {
		Logger::GetInstance().Error("Error input!");
		return false;
	}

	if (!checkIP(ipaddr, 0) || !checkSubnet(ipaddr, subnet)
			|| !checkIP(gateway, 0))
		return false;
	UINT32 uIP = 0, uSub = 0, uGate = 0, i = 0, n[4] = { 0 };
	sscanf(ipaddr, "%u.%u.%u.%u", &n[3], &n[2], &n[1], &n[0]);
	for (i = 0; i < 4; ++i)
		uIP += n[i] << (i * 8);
	memset(n, 0, 4);
	sscanf(subnet, "%u.%u.%u.%u", &n[3], &n[2], &n[1], &n[0]);
	for (i = 0; i < 4; ++i)
		uSub += n[i] << (i * 8);
	memset(n, 0, 4);
	sscanf(gateway, "%u.%u.%u.%u", &n[3], &n[2], &n[1], &n[0]);
	for (i = 0; i < 4; ++i)
		uGate += n[i] << (i * 8);

	if ((uIP & uSub) != (uGate & uSub)) {
		Logger::GetInstance().Error("Subnet is not matched with gateway!");
		return false;
	}
	return true;
}
bool CheckNetConfig::checkMAC(INT8 *mac) {
	if (strlen(mac) != 17)
		return false;
	INT8 *strtmp = strtok(mac, ":");
	INT32 hexnum = 0;
	while (strtmp != NULL) {
		if (strlen(strtmp) != 2)
			return false;
		if ((strtmp[0] >= '0' && strtmp[0] <= '9')
				|| (strtmp[0] >= 'A' && strtmp[0] <= 'F')
				|| (strtmp[0] >= 'a' && strtmp[0] <= 'f')) {
			if ((strtmp[1] >= '0' && strtmp[1] <= '9')
					|| (strtmp[1] >= 'A' && strtmp[1] <= 'F')
					|| (strtmp[1] >= 'a' && strtmp[1] <= 'f')) {
				hexnum++;
				strtmp = strtok(NULL, ":");
			} else {
				return false;
			}
		} else {
			return false;
		}
	}
	if (hexnum != 6)
		return false;
	return true;
}

INT32 SetNetwork::stringToHex(const string &strNum) {
	if (strNum.length() != 2) {
		cout << "string ::::" << strNum << endl;
		cout << "String to Hex false" << endl;
		return 0;
	}
	INT32 h, l;
	h = charToHex(strNum[0]);
	l = charToHex(strNum[1]);
	return h * 16 + l;
}

INT32 SetNetwork::charToHex(const DP_C_S8 cNum) {
	if (cNum >= '0' && cNum <= '9') {
		return cNum - '0';
	} else if (cNum >= 'a' && cNum <= 'f') {
		return cNum - 'a' + 10;
	} else if (cNum >= 'A' && cNum <= 'F') {
		return cNum - 'A' + 10;
	} else {
		cout << "chartohex num range error" << endl;
		return 0;
	}
}
bool SetNetwork::cSplitString(const string strKey, const string strChar,
		vector<string> &vMac) {
	string::size_type pos1, pos2;
	pos2 = strKey.find(strChar);
	pos1 = 0;
	while (string::npos != pos2) {
		vMac.push_back(strKey.substr(pos1, pos2 - pos1));
		pos1 = pos2 + strChar.size();
		pos2 = strKey.find(strChar, pos1);
	}
	if (pos1 != strKey.length()) {
		vMac.push_back(strKey.substr(pos1));
		return true;
	} else {
		return false;
	}
}

NetConfigTrans::NetConfigTrans() :
		flag(0) {
	ipT = new INT8[18];
	submaskT = new INT8[18];
	gatewayT = new INT8[18];
}
NetConfigTrans::~NetConfigTrans() {
	delete[] ipT;
	delete[] submaskT;
	delete[] gatewayT;
}
