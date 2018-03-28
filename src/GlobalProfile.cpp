/*
 * GlobalProfile.cpp
 *
 *  Created on: Mar 23, 2018
 *      Author: jhb
 */





#include "GlobalProfile.h"

GlobalProfilePtr GlobalProfile::g_Profile = 0;

GlobalProfile::GlobalProfile() {
	loadFromXML();
}
GlobalProfile::~GlobalProfile() {
}
void GlobalProfile::GetTCPCommServerIP(string& ip, uint16_t& port) {
	ip = _TCPServerIP;
	port = _TCPServerPort;
}
void GlobalProfile::SetTCPCommServerIP(const string& ip, const uint16_t& port) {
	_TCPServerIP = ip;
	_TCPServerPort = port;
	ProfilePtr profile = new Profile(PROFILE_PATH);
	profile->updateValue("TCPServer", "ServerIP", "%s", ip.c_str());
	profile->updateValue("TCPServer", "ServerPort", "%d", port);
	// delete profile;
}
void GlobalProfile::loadFromXML() {
	if (access(PROFILE_DIR, F_OK) != 0) {
		if (mkdir(PROFILE_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
		}
	}
	ProfilePtr profile = new Profile(PROFILE_PATH);
	_TCPServerIP = profile->getString("TCPServer", "ServerIP", "172.16.0.228");
	_TCPServerPort = profile->getUShort("TCPServer", "ServerPort", 7001);
}

