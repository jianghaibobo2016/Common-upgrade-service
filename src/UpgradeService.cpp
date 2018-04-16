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
	udpNetTrans->socketRunThread();
	if (strlen(const_cast<SetNetworkTerminal*>(&getNetC())->getPCIP()) != 0)
	{
		cout << "pcip: "<<const_cast<SetNetworkTerminal*>(&getNetC())->getPCIP()<<endl;
		HandleUp::upMainRootfsRespond(udpNetTrans->getSockfd(),
				*const_cast<SetNetworkTerminal*>(&getNetC()));
	}
	while (1) {
		sleep(1);
	}
	return retOk;
}
