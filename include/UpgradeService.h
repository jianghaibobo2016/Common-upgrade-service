#ifndef UPGRADESERVICE_H
#define UPGRADESERVICE_H
#include "VersionFileManage.h"
#include "SetNetwork.h"
#include "DevSearch.h"
#include "GlobDefine.h"
#include "PCTransProtocal.h"
#include "Uncopyable.h"
#include "UDPNetTrans.h"

class UpgradeService : private Uncopyable
{
public:
    SetNetworkTerminal *setNetworkTerminal;
public:
    UpgradeService(SetNetworkTerminal *setNetworkTerminal);
    ~UpgradeService();

    INT32 start();

private:

};

#endif /* UPGRADESERVICE_H */
