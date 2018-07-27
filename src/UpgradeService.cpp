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
	INT32 retBind = udpNetTrans->socketBind(UpUDPTransPort);
	if (retBind == retError)
		return retError;
	if (strlen(const_cast<SetNetworkTerminal*>(&getNetC())->getPCIP()) != 0)
	{
		HandleUp::upMainRootfsRespond(udpNetTrans->getSockfd(),
				*const_cast<SetNetworkTerminal*>(&getNetC()));
	}
	udpNetTrans->socketRunThread();//join



	return retOk;
}
