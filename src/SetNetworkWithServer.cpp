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
#include <arpa/inet.h>
#include <net/if_arp.h> /* ARPHRD_ETHER */
#include <SetNetworkWithServer.h>
#include "DevSearch.h"
#include "UpgradeServiceConfig.h"

using namespace std;
using namespace FrameWork;

SetNetworkTerminal::SetNetworkTerminal() :
		SetNetwork() {
	PCIP = new INT8[32];
	memset(PCIP, 0, 32);
}
//???
SetNetworkTerminal::SetNetworkTerminal(
		const SetNetworkTerminal& setNetworkTerminal) :
		SetNetwork(setNetworkTerminal) {
	PCIP = new INT8[32];
	memset(PCIP, 0, 32);
	setPCIP(setNetworkTerminal.getPCIP());
}
SetNetworkTerminal::~SetNetworkTerminal() {
	delete[] PCIP;
}
bool SetNetworkTerminal::setServerNetConfig(const string &ip,
		const UINT16 &port) {
	XMLParser xmlParser(pathXml);
	xmlParser.xmlInit();
	xmlParser.updateServerNetConfig("TCPServer", "ServerIP", "%s", ip.c_str());
	xmlParser.updateServerNetConfig("TCPServer", "ServerPort", "%d", port);
	return true;
}
NetConfigTransWithServer::NetConfigTransWithServer() :
		NetConfigTrans() {
	serverIPT = new INT8[18];
	serverPortT = new INT8[5];
	MAC = new INT8[13];
	MASK = new INT8[16];
}
NetConfigTransWithServer::~NetConfigTransWithServer() {
	delete[] serverIPT;
	delete[] serverPortT;
	delete[] MAC;
	delete[] MASK;
}
