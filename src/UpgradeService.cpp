#include <unistd.h>
#include "UpgradeService.h"
#include "HandleUp.h"

UpgradeService::UpgradeService(SetNetworkTerminal *setNetworkTerminal) :
		setNetworkTerminal(setNetworkTerminal) {
}

UpgradeService::~UpgradeService() {
}

INT32 UpgradeService::start() {
	/* this */
	UDPNetTrans *udpNetTrans = new UDPNetTrans(setNetworkTerminal);
	udpNetTrans->socketBind(UpUDPTransPort);
	if (strlen(const_cast<SetNetworkTerminal*>(&getNetC())->getPCIP()) != 0)
	{
		HandleUp::upMainRootfsRespond(udpNetTrans->getSockfd(),
				*const_cast<SetNetworkTerminal*>(&getNetC()));
	}
	udpNetTrans->socketRunThread();//join
//	while (1) {
//		sleep(1);
//	}
	return retOk;
}
