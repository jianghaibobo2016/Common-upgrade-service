/*
 * UpgradeMain.cpp
 *
 *  Created on: Apr 10, 2018
 *      Author: jhb
 */

#include <iostream>
#include <unistd.h>
#include "Logger.h"
#include <string.h>
#include <SetNetworkWithServer.h>
#include "UpgradeService.h"
using namespace FrameWork;
int main(int argc, char *argv[]) {
	const INT8 *ifname = IFNAMETERMINAL;
	SetNetworkTerminal getnetwork;
	InitLogging(argv[0], DEBUG, logPath);
	if (argc == 2) {
		const INT8 *PCIP = argv[1];
		getnetwork.setPCIP(PCIP);
	}
	getnetwork.setIfname(ifname);
	getnetwork.getNetworkConfig();
	cout << "start ifname : " << getnetwork.getIfname() << endl;
	UpgradeService upgradeService(&getnetwork);
	upgradeService.start();
	while (1) {
		sleep(1);
	}
}

