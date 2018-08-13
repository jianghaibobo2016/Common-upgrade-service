/*
 * NetConfigTransWithServer.cpp
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
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if_arp.h> /* ARPHRD_ETHER */
#include <SetNetworkWithServer.h>
#include "DevSearch.h"
#include "Logger.h"
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

SetNetworkTerminal &SetNetworkTerminal::operator=(
		const SetNetworkTerminal &setNet) {
	if (this != &setNet) {
		SetNetwork::operator =(setNet);
		setPCIP(setNet.getPCIP());
	}
	return *this;
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
		NetConfigTrans(), RecordingPort(0), castMode(DefaultChar) {
	serverIPT = new INT8[18];
	CommunicationPort = new INT8[5];
	Name = new INT8[60];
	memset(serverIPT, 0, 18);
	memset(CommunicationPort, 0, 5);
	memset(Name, 0, 60);
}
NetConfigTransWithServer::~NetConfigTransWithServer() {
	delete[] serverIPT;
	delete[] CommunicationPort;
	delete[] Name;

}

InitSetConf::InitSetConf() :
		flag(0) {
	MAC = new INT8[13];
	MASK = new INT8[8];
	memset(MAC, 0, 13);
	memset(MASK, 0, 8);
}

InitSetConf::~InitSetConf() {
	delete[] MAC;
	delete[] MASK;
}

