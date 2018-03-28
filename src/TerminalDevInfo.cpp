#include <Logger.h>
#include "TerminalDevInfo.h"
using namespace FrameWork;

TerminalDevInfo::TerminalDevInfo(SetNetworkTerminal *setNetworkTerminal, const INT8 *pathVersionFile)
    : devType(TerminalDevType),
      devHardVersion(TerminalHardVersion),
      setNetworkTerminal(setNetworkTerminal)
{
    getMac();
    getDevID();
    getSoftVersion(pathVersionFile);
}

bool TerminalDevInfo::getMac()
{
    if (setNetworkTerminal->getNetworkConfig() != true)
    {
        Logger::GetInstance().Error("Get network config failed !");
        return false;
    }
    static INT8 mac[13] = {0};
    strcpy(mac, SetNetworkTerminal::castMacToChar13(mac, setNetworkTerminal->m_netWorkConfig.macAddr));
    mac[12] = '\0';
    devMAC = mac;
    return true;
}
bool TerminalDevInfo::getSoftVersion(const INT8 *pathVersionFile)
{
    static INT8 version[7] = {0};
    DevSearchTerminal::getSoftwareVersion(ProductVersionName, version, pathVersionFile);
    devSoftVersion = version;
    return true;
}
bool TerminalDevInfo::getDevID()
{
    INT32 devTypeLen = strlen(TerminalDevType);
    INT32 devMacLen = strlen(devMAC);
    // INT8 devId[devTypeLen + devMacLen + 1] ;
    static INT8 devId[40] = {0};
    strncpy(devId, TerminalDevType, devTypeLen);
    strcpy(devId + devTypeLen, devMAC);
    devId[devTypeLen + devMacLen] = '\0';
    devID = devId;
    return true;
}
