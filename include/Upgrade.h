//////////////////
#ifndef UPGRADE_H
#define UPGRADE_H
#include <string.h>
#include "GlobDefine.h"
#include "PCTransProtocal.h"
#include "UpgradeServiceConfig.h"
#include "md5.h"
#include "Uncopyable.h"
#include <map>
#include <vector>

#ifdef FILE_LINE
#undef FILE_LINE
#endif
#ifndef FILE_LINE
#define FILE_LINE (LOG(DEBUG) << "check")
#endif

enum upgradeFileStatus {
	lowerVersion = -1, equalVersion = 0, higherVerison, errorVersionStatus
};

//class Upgrade {
//public:
////	virtual bool upgradeInit() = 0;
////	virtual INT32 checkDownloadFile() = 0;
////	virtual INT32 parserUpgradeFile() = 0;
////	virtual INT32 checkUpgradeStatus() = 0;
////	virtual bool modifyVersionFile() = 0;
//
//	Upgrade() {
//	}
//	virtual ~Upgrade() {
//	}
//
//	// protected:
//
//	// private:
//};

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
	void setItemName(INT8 *name) {
		itemName = name;
	}
	void setUpResult(bool result) {
		upResult = result;
	}
	INT32 modifyVersionFile();
	void clearObj();
private:
	/* private variables */
	INT8 *upgradeFile;
	INT8 *fileWithoutPath;
	INT8 *newVersion;
	INT8 *localVersion;
	upgradeFileStatus upStatus;
	bool productItem;
	INT8 *itemName;
	INT8 *versionFileItemName;
	INT8 *upgraderecord;
	bool upResult;

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
	void setEachItemUpResult(bool result) {
		eachItemUpStatus = result;
	}
	map<INT32, string>& getExtractItem() {
		return mUpSubItem;
	}
	const UpgradeDSP &getUpObj() {
		return *aUpSubItem;
	}
	bool getSubItems();
	INT32 parserSubItemsFileName(UINT32 num);
	INT32 upgradeItem(UINT32 num);
	INT32 excuteUpgradeShell(UINT32 num);
	INT32 modifyVersionFile();
private:
	const INT8 *productTarFile;
	bool eachItemUpStatus;
	map<INT32, string> mSubItems;
	map<INT32, string> mUpSubItem;
	UpgradeDSP *aUpSubItem;
};
#endif

