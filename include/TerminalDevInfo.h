#ifndef TERMINALDEVINFO_H
#define TERMINALDEVINFO_H

#include "GlobDefine.h"
#include "SetNetwork.h"
#include "DevSearch.h"

class TerminalDevInfo
{
public:
	const INT8 *getDevMACInfo(){return devMAC;}
	const INT8 *getDevIDInfo(){return devID;}
	const INT8 *getDevTypeInfo(){return devType;}
	const INT8 *getDevHardVInfo(){return devHardVersion;}
	const INT8 *getDevSoftVInfo(){return devSoftVersion;}

public:
  TerminalDevInfo(SetNetworkTerminal *setNetworkTerminal, const INT8 *pathVersionFile);
  ~TerminalDevInfo(){}

private:
  const INT8 *devMAC;
  const INT8 *devID;
  const INT8 *devType;
  const INT8 *devHardVersion;
  const INT8 *devSoftVersion;
//   const INT8 *devName;
  SetNetworkTerminal *setNetworkTerminal;
  DevSearchTerminal *devSearchTerminal;
  bool getMac();
  bool getSoftVersion(const INT8 *pathVersionFile);
  bool getDevID();
};


#endif /* TERMINALDEVINFO_H */
