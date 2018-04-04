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
    udpNetTrans->socketBind(UpUDPTransPort);
    udpNetTrans->socketRunThread();
    while(1){}
    return retOk;
}
