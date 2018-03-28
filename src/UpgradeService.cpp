#include "UpgradeService.h"

UpgradeService::UpgradeService(SetNetworkTerminal *setNetworkTerminal)
    : setNetworkTerminal(setNetworkTerminal)
{
}

UpgradeService::~UpgradeService()
{
    // delete setNetworkTerminal;
}

INT32 UpgradeService::start()
{
    /* this */
    UDPNetTrans *udpNetTrans = new UDPNetTrans(setNetworkTerminal);
    udpNetTrans->socketBind(45535);
    udpNetTrans->socketRunThread();
    while(1){}
    return retOk;
}