#ifndef CMDPARSERUP_H
#define CMDPARSERUP_H
#include <SetNetworkWithServer.h>
#include "GlobDefine.h"
#include "PCTransProtocal.h"
#include "Upgrade.h"
#include "UpFileAttrs.h"
#include "TerminalDevInfo.h"
class CMDParserUp
{
public:
    CMDParserUp();
    ~CMDParserUp();

    static UINT16 parserPCRequestHead(void *buffer, INT32 recvLen);
    static INT32 parserPCSetNetCMD(void *buffer, SetNetworkTerminal *, INT8 &failReason);
    static upgradeFileStatus parserPCUpgradeCMD(void *buffer, UpFileAttrs&, INT8 *failReason);
private:
    static bool campareNetSetMatch(INT8 *nameLen, INT8 *name, const INT8 *reName);
    static inline INT32 compareUpgradeItem(INT8 *ptr, const INT8 *item, INT32 len) 
    {return strncmp(ptr, item, strlen(item)); }

    // static  getValue();
};

#endif /* CMDPARSERUP_H */
