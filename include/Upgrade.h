//////////////////
#ifndef UPGRADE_H
#define UPGRADE_H
#include <string.h>
#include "GlobDefine.h"
#include "PCTransProtocal.h"
#include "UpdateProgram_Protocal.h"
#include "UpgradeServiceConfig.h"
#include "md5.h"
#include "Uncopyable.h"
#include <map>
#include <vector>

enum upgradeFileStatus {
	lowerVersion = -1, equalVersion = 0, higherVerison, errorVersionStatus
};

/*, private Uncopyable 加了继承类的vector之后不可用了*/
class UpgradeDSP {
public:
	UpgradeDSP(INT8 *upgradeFile);
//	UpgradeDSP(const UpgradeDSP &);
//	UpgradeDSP& operator=(const UpgradeDSP &rhs);
	~UpgradeDSP();

	bool upgradeStart();
	bool upgradeStepStart();
	bool upgradeStepUpgrade();

	INT32 parserFileName();
	INT32 parserItemPackage(INT8 *PCRequestVersion);

	INT8 *getUpgradeFile() {
		return upgradeFile;
	}
	upgradeFileStatus getUpStatus() {
		return upStatus;
	}
	INT8 *getMemberItemName() {
		return itemName;
	}
	INT8 *getVersionFileItemName() {
		return versionFileItemName;
	}
	INT8 *getNewVersion() {
		return newVersion;
	}
	INT8 *getLocalVersion() {
		return localVersion;
	}
	INT8 *getUpgraderecord() {
		return upgraderecord;
	}
	bool getUpResult() {
		return upResult;
	}
	const bool getForceStatus() const {
		return forceUpgrade;
	}
	void setUpgradeFile(INT8 *file) {
		upgradeFile = file;
	}
	void setUpStatus(upgradeFileStatus status) {
		upStatus = status;
	}
	void setUpgraderecord(const INT8 *record) {
		memset(upgraderecord, 0, msgLen);
		memcpy(upgraderecord, record, strlen(record));
	}
	void setItemName(const INT8 *name) {
		memset(itemName, 0, 16);
		memcpy(itemName, name, strlen(name));
	}
	void setUpResult(bool result) {
		upResult = result;
	}
	void setForceUpgrade(const bool force) {
		forceUpgrade = force;
	}
	static void getVersionByItemName(INT8 *itemName, INT8 *version);
	INT32 modifyVersionFile();
	void clearObj();
private:
	/* private variables */
	INT8 *upgradeFile;
	INT8 *fileWithoutPath;
	INT8 *newVersion;
	INT8 *localVersion;
	INT8 *dependVersion;
	upgradeFileStatus upStatus;
	bool productItem;
	INT8 *itemName;
	INT8 *versionFileItemName;
	INT8 *upgraderecord;
	bool upResult;

	bool forceUpgrade;

	static const INT8 itemsSet[][16];
	static const INT8 itemsVersionNameSet[11][32];
	/* const */
	/* functions */
	INT32 getItemName();
	upgradeFileStatus getVersion();
	inline bool isProductItem() {
		return productItem;
	}
	inline INT32 compareUpgradeItem(const INT8* ptr, const INT8* item) const {
		return strncmp(ptr, item, strlen(item));
	}
	void runUpgradeShell();
};

class UpgradeDSPSubItem {
public:
	UpgradeDSPSubItem();
	~UpgradeDSPSubItem();

	UINT32 getItemsNum() {
		return mSubItems.size();
	}
	bool getEachItemUpResult() {
		return eachItemUpStatus;
	}
	bool getUpSystem() {
		return upSystem;
	}
	bool getUpTerminalDevs() {
		return upTerminalDevs;
	}
	UPDATE_DEV_TYPE getUpDevType() {
		return upDevType;
	}

	const bool getForceStatus() const {
		return forceUpgrade;
	}
	void setEachItemUpResult(bool result) {
		if (result == false)
			cout << "setEachItemUpResult(false)" << endl;
		eachItemUpStatus = result;
	}
	void setUpSystem(bool upSys) {
		upSystem = upSys;
	}
	void setUpTerminalDevs(bool upAmp) {
		upTerminalDevs = upAmp;
	}
	void setUpDevType(UPDATE_DEV_TYPE type) {
		upDevType = type;
	}
	void setForceUpgrade(const bool force) {
		forceUpgrade = force;
	}
	map<INT32, string>& getExtractItem() {
		return mSubItemName;
	}
	const UpgradeDSP &getUpObj() {
		return *aUpSubItem;
	}

	bool getSubItems(map<INT32, DEV_MODULES_TYPE>& devModuleToUpgrade);
	bool adjustOrder(map<INT32, DEV_MODULES_TYPE>& devModuleToUpgrade);
	INT32 parserSubItemsFileName(UINT32 num);
	INT32 upgradeItem(UINT32 num);
	INT32 excuteUpgradeShell(UINT32 num, INT8 *PCIP);
	static bool excuteDevShell();
	INT32 modifyVersionFile();
	map<INT32, string> mSubItems;
private:
	const INT8 *productTarFile;
	bool eachItemUpStatus;
	map<INT32, string> mSubItemName;
	UpgradeDSP *aUpSubItem;
	volatile bool upSystem; //////////
	bool upTerminalDevs;
	UPDATE_DEV_TYPE upDevType;

	bool forceUpgrade;

};
#endif

