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
	setvbuf(stdout, (char *) NULL, _IOLBF, 0);
	const INT8 *ifname = IFNAMETERMINAL;
	SetNetworkTerminal getnetwork;
	InitLogging(argv[0], DEBUG, logPath);
	if (argc == 2) {
		const INT8 *PCIP = argv[1];
		if (CheckNetConfig::GetInstance().checkIP(PCIP, 0) == false) {
			Logger::GetInstance().Error("PCIP input wrong !");
			return retError;
		}
		getnetwork.setPCIP(PCIP);
	} else if (argc != 1) {
		Logger::GetInstance().Error("Number of arguments wrong !");
		return retError;
	}
	getnetwork.setIfname(ifname);
	getnetwork.getNetworkConfig();
	UpgradeService upgradeService(&getnetwork);
	upgradeService.start();
	return retOk;
}

