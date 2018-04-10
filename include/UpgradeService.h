#ifndef UPGRADESERVICE_H
#define UPGRADESERVICE_H
#include <SetNetworkWithServer.h>
#include "VersionFileManage.h"
#include "DevSearch.h"
#include "GlobDefine.h"
#include "PCTransProtocal.h"
#include "Uncopyable.h"
#include "UDPNetTrans.h"

class UpgradeService: private Uncopyable {
public:
	UpgradeService(SetNetworkTerminal *setNetworkTerminal);
	~UpgradeService();

	const SetNetworkTerminal &getNetC() const {
		return *setNetworkTerminal;
	}

	INT32 start();

private:
	SetNetworkTerminal *setNetworkTerminal;

};

#endif /* UPGRADESERVICE_H */
