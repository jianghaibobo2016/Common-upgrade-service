/*
 * UpgradeMain.cpp
 *
 *  Created on: Apr 10, 2018
 *      Author: jhb
 */

#include <iostream>
#include "Logger.h"
#include <string.h>
//#include <net/if.h>
//#include <net/if_arp.h> /* ARPHRD_ETHER */
//#include <sys/socket.h>
//#include <sys/ioctl.h>
//#include <unistd.h>
//#include <netinet/in.h>
#include <SetNetworkWithServer.h>
#include "UpgradeService.h"
using namespace FrameWork;
//int macAddrGet(UINT8 *mac);
int main(int argc, char *argv[]) {
	const INT8 *ifname = IFNAMETERMINAL;
	SetNetworkTerminal getnetwork;
	InitLogging(argv[0], DEBUG, "./log_");
	if (argc == 2){
		const INT8 *PCIP = argv[1];
		getnetwork.setPCIP(PCIP);
	}
	getnetwork.setIfname(ifname);
	getnetwork.getNetworkConfig();
	cout << "start ifname : " << getnetwork.getIfname() << endl;
	UpgradeService upgradeService(&getnetwork);
	upgradeService.start();
	while (1)
		;
}

