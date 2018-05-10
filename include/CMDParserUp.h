/*
 * CMDParser.h
 *
 *  Created on: 2018年4月16日
 *      Author: JHB
 */
#ifndef CMDPARSERUP_H
#define CMDPARSERUP_H

#include <SetNetworkWithServer.h>
#include "GlobDefine.h"
#include "PCTransProtocal.h"
#include "Upgrade.h"
#include "UpFileAttrs.h"

class CMDParserUp {
public:
	CMDParserUp();
	~CMDParserUp();

	static UINT16 parserPCRequestHead(void *buffer, INT32 recvLen);
	static INT32 parserPCSetNetCMD(void *buffer, SetNetworkTerminal *,
			map<string, string> &retContent);
	static upgradeFileStatus parserPCUpgradeCMD(void *buffer, UpFileAttrs&,
			INT8 *failReason, map<INT32, DEV_MODULES_TYPE> &devModuleToUpgrade);

	const INT32 getSettingNum() const {
		return settingNum;
	}
	void setSettingNum(INT32 num) {
		settingNum = num;
	}

	static INT32 isDevModulesUpgradeEnable(
			map<INT32, DEV_MODULES_TYPE> &devModuleToUpgrade,
			map<INT32, DEV_MODULES_TYPE>&devModules, UpFileAttrs &upFileAttr);
private:

	INT32 settingNum;

	static bool campareNetSetMatch(INT8 *nameLen, INT8 *name,
			const INT8 *reName);
	static inline INT32 compareUpgradeItem(INT8 *ptr, const INT8 *item,
			INT32 len) {
		return strncmp(ptr, item, strlen(item));
	}
	static INT32 writeMaskFile(vector<UINT16> date);

};

#endif /* CMDPARSERUP_H */
