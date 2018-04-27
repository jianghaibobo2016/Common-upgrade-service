///////////
#include <iostream>
#include "Logger.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "Upgrade.h"
#include "CrcCheck.h"
#include "DevSearch.h"
#include "FileOperation.h"
#include "RCSP.h"
#include "HandleUp.h"
using namespace std;
using namespace FrameWork;
UpgradeDSP::UpgradeDSP(INT8 *upgradeFile) :
		upgradeFile(upgradeFile), fileWithoutPath(
				upgradeFile + strlen(upFilePath)), upStatus(errorVersionStatus), productItem(
				false), upResult(false), forceUpgrade(false)

{
	cout << "UpgradeDSP Obj construct " << endl;
	newVersion = new INT8[8];
	localVersion = new INT8[8];
	dependVersion = new INT8[8];
	itemName = new INT8[16];
	versionFileItemName = new INT8[32];
	upgraderecord = new INT8[msgLen];
}

UpgradeDSP::~UpgradeDSP() {
	delete[] newVersion;
	delete[] localVersion;
	delete[] dependVersion;
	delete[] itemName;
	delete[] versionFileItemName;
	delete[] upgraderecord;
	cout << "UpgradeDSP Objdestruct" << endl;
}

/*12 */
const INT8 UpgradeDSP::itemsSet[][16] = { "_V", "UBOOT", "KERNEL", "MROOTFS",
		"BROOTFS", "MediaApp", "ServerApp", "UPGRADE", "WEB", "NAND", "PRODUCT",
		"OVERFLAG" };

const INT8 UpgradeDSP::itemsVersionNameSet[11][32] = { "hardware_version",
		"uboot_version", "kernel_version", "main_rootfs_version",
		"backup_rootfs_version", "media_app_version", "server_app_version",
		"upgrade_version", "web_version", "nand_version", "product_version" };

INT32 UpgradeDSP::getItemName() {
	/*Only file name*/
	fileWithoutPath += strlen(TerminalDevType) + 1;
	INT32 i = 0;
	while (fileWithoutPath[i] != '_') {
		itemName[i] = fileWithoutPath[i];
		i++;
	}
	itemName[i] = '\0';
	fileWithoutPath -= strlen(TerminalDevType) + 1;

	i = 1;
	while (i < 12) {
		if ((compareUpgradeItem(itemsSet[i], itemsSet[11])) != 0 && i != 11) {
			if ((compareUpgradeItem(itemName, itemsSet[i])) != 0) {
				i++;
				continue;
			} else {
				if (i == 10)
					productItem = true;
				break;
			}
		} else {
			LOG(ERROR) << "The upgrade item is not matched!";
			return retError;
		}
	}
	return retOk;
}

upgradeFileStatus UpgradeDSP::getVersion() {
	fileWithoutPath += strlen(TerminalDevType) + 1;
	fileWithoutPath += strlen(itemName) + 1;
	INT32 i = 0;
	while (fileWithoutPath[i] != '\0') {
		newVersion[i] = fileWithoutPath[i];
		i++;
	}
	newVersion[i] = '\0';
	fileWithoutPath -= strlen(TerminalDevType) + 1;
	fileWithoutPath -= strlen(itemName) + 1;
	for (INT32 i = 1; i < 11; i++) {
		if (compareUpgradeItem(itemName, itemsSet[i]) == 0) {
			memcpy(versionFileItemName, itemsVersionNameSet[i],
					strlen(itemsVersionNameSet[i]));
			versionFileItemName[strlen(itemsVersionNameSet[i])] = '\0';
			DevSearchTerminal::getSoftwareVersion(itemsVersionNameSet[i],
					localVersion, pathVersionFile);
		}
	}
	INT32 retUpStatus = compareUpgradeItem(newVersion, localVersion);
	if (retUpStatus > 0)
		return higherVerison;
	else if (retUpStatus == 0)
		return equalVersion;
	else if (retUpStatus < 0)
		return lowerVersion;
	else
		return errorVersionStatus;
}

INT32 UpgradeDSP::parserFileName() {
	/*DPS9903*/
	fileWithoutPath = upgradeFile + strlen(upFilePath);
	if (compareUpgradeItem(fileWithoutPath, TerminalDevType) != 0) {
		return retError;
	}
	/* get NAND*//*compare item*/
	if (getItemName() != retOk) {
		memset(upgraderecord, 0, msgLen);
		strcpy(upgraderecord, "Upgrade file name error !");
		return retError;
	}
	if ((upStatus = getVersion()) != higherVerison
			&& getForceStatus() != true) {
		cout << __FUNCTION__ << "() upstatus : " << upStatus << endl;
		return retError;
	} else
		setUpStatus(higherVerison);
	return retOk;
}

INT32 UpgradeDSP::parserItemPackage(INT8 *PCRequestVersion) {
	if (CrcCheck::parser_Package(upgradeFile, newVersion, itemName,
			dependVersion) != 0) {
		return retError;
	}
	if (FileOperation::isExistFile(newTarPackage) != true) {
		return retError;
	} else {
		FileOperation::deleteFile(upgradeFile);
	}
	return retOk;
}

void UpgradeDSP::getVersionByItemName(INT8 *itemName, INT8 *version) {
	for (INT32 i = 1; i < 11; i++) {
		if (strncmp(itemName, itemsSet[i], strlen(itemsSet[i])) == 0) {
			DevSearchTerminal::getSoftwareVersion(itemsVersionNameSet[i],
					version, pathVersionFile);
		}
	}
}

INT32 UpgradeDSP::modifyVersionFile() {
	if (!upResult)
		return retError;
	string versionLine;
	versionLine += getVersionFileItemName();
	versionLine += "=";
	versionLine += (getLocalVersion() + 1);
	string newVersionLine;
	newVersionLine += getVersionFileItemName();
	newVersionLine += "=";
	newVersionLine += (getNewVersion() + 1);

	chmod(pathVersionFile, S_IWUSR | S_IWGRP | S_IWOTH);
	ifstream in;
	char line[64];
	in.open(pathVersionFile);
	string strTemp;
	while (in.getline(line, sizeof(line))) {
		string strline = line;
		if (versionLine == strline) {
			strline = newVersionLine;
			strTemp += strline + '\n';
		} else {
			strTemp += strline + '\n';
		}
	}
	in.close();
	ofstream out(pathVersionFile);
	if (out.is_open()) {
		out.flush();
		out << strTemp;
		out.close();
	} else {
		return retError;
	}
	chmod(pathVersionFile, S_IRUSR | S_IRGRP | S_IROTH);
	return retOk;
}
void UpgradeDSP::clearObj() {
	upgradeFile = NULL;
	fileWithoutPath = NULL;
	memset(newVersion, 0, strlen(newVersion));
	memset(localVersion, 0, strlen(localVersion));
	upStatus = errorVersionStatus;;
	productItem = false;
	memset(itemName, 0, strlen(itemName));
	memset(versionFileItemName, 0, strlen(versionFileItemName));
	memset(upgraderecord, 0, strlen(upgraderecord));
	upResult = false;
}

UpgradeDSPSubItem::UpgradeDSPSubItem() :
		productTarFile(newTarPackage), eachItemUpStatus(true), mSubItems(), mSubItemName(), upSystem(
				false), upTerminalDevs(false), upDevType(ERROR_TYPE), forceUpgrade(
				false) {
	aUpSubItem = new UpgradeDSP(NULL);
}
UpgradeDSPSubItem::~UpgradeDSPSubItem() {
	delete aUpSubItem;
}
bool UpgradeDSPSubItem::getSubItems(
		map<INT32, DEV_MODULES_TYPE>& devModuleToUpgrade) {
	if (FileOperation::extractTarFile(productTarFile, mSubItems) != true) {
		FileOperation::deleteFile(productTarFile);
		return false;
	} else {
		for (UINT32 i = 1; i <= mSubItems.size(); i++) {
			string strTmp = mSubItems[i];
			INT32 npos = strTmp.find("_V");

			mSubItemName[i] = strTmp.substr(strlen(StringFind),
					npos - strlen(StringFind));
		}
		mSubItems.clear();
		cout << "test amp mapppppppppppppppppppppppppppppppppp::3 "
				<< mSubItemName[1] << endl;
		cout << "test amp sizeeeeeeeeeeeeeeeeedevM::4 "
				<< devModuleToUpgrade.size() << endl;
		//adjust order
		adjustOrder(devModuleToUpgrade);
		cout << "test amp mapppppppppppppppppppppppppppppppppp::2 "
				<< mSubItems[1] << endl;
	}
	FileOperation::deleteFile(productTarFile);
	return true;
}

bool UpgradeDSPSubItem::adjustOrder(
		map<INT32, DEV_MODULES_TYPE>& devModuleToUpgrade) {
	map<INT32, string> m_dev_type;
	map<INT32, string> m_devs;
	map<INT32, string> m_subItem;
	UINT32 i = 1;
	for (i = 1; i <= devModuleToUpgrade.size(); i++) {
		if (devModuleToUpgrade[i] == DEV_AMPLIFIER)
			m_dev_type[i] = AMPLIFIER;
		else if (devModuleToUpgrade[i] == DEV_PAGER)
			m_dev_type[i] = PAGER;
		else {
			return false;
		}
	}
	UINT32 nameLen = mSubItemName.size();
	UINT32 moduleLen = m_dev_type.size();
	INT32 flag = 0;
	INT32 mSubNum = 1;
	INT32 mDevNum = 1;
	bool matchFlag = false;

	for (i = 1; i <= nameLen; i++) {
		for (UINT32 j = 1; j <= moduleLen; j++) {
			if (mSubItemName[i] == m_dev_type[j]) {
				m_devs[mDevNum] = m_dev_type[j];
				mDevNum++;
				matchFlag = true;
				break;
			}
		}
		if (!matchFlag && mSubItemName[i] != AMPLIFIER
				&& mSubItemName[i] != PAGER) {
			mSubItems[mSubNum] = mSubItemName[i];
			mSubNum++;
		}
	}

	for (i = 1; i <= m_devs.size(); i++) {
		mSubItems[mSubNum] = m_devs[i];
	}

//	for (i = 1; i <= mSubItemName.size(); i++) {
//		if (mSubItemName[i] != m_dev_type[devNum]) {
//
//			m_subItem[i - flag] = mSubItems[i];
//		} else {
//			cout << "HHHHHHHHHHHHHHHHHHHHHHHHHHHHH" << endl;
//			m_subItem[mSubItemName.size() - (m_dev_type.size() - devNum)] =
//					mSubItems[i];
//			--devNum;
//			flag++;
////			}
//		}
//	}
//	if (devModuleToUpgrade.size() != 0) {
	for (i = 1; i <= mSubItems.size(); i++) {
		string tmp = upFilePath;
		tmp += m_subItem[i];
		m_subItem[i] = tmp;
		mSubItems[i] = m_subItem[i];
	}
//	}
	cout << "test amp mapppppppppppppppppppppppppppppppppp::1 " << mSubItems[1]
			<< endl;
	return true;
}

INT32 UpgradeDSPSubItem::parserSubItemsFileName(UINT32 num) {

	if (strncmp(mSubItems[num].c_str() + strlen(upFilePath), AmplifierUpgrade,
			strlen(AmplifierUpgrade)) == 0) {
		cout << "up amplifier !!!!!!!!!!!!!!!!!!!!!!1" << endl;
		this->setUpTerminalDevs(true);
		this->setUpDevType(UPDATE_DEV_AMP);
	} else if (strncmp(mSubItems[num].c_str() + strlen(upFilePath),
			PagerUpgrade, strlen(PagerUpgrade)) == 0) {
		cout << "up pager !!!!!!!!!!!!!!!!!!!!!!2" << endl;
		this->setUpTerminalDevs(true);
		this->setUpDevType(UPDATE_DEV_PAGER);
	}

	aUpSubItem->setUpgradeFile(const_cast<INT8 *>(mSubItems[num].c_str()));
	INT8 record[msgLen] = { 0 };
	if (!this->getUpTerminalDevs()) {
		if (this->getForceStatus() == true)
			aUpSubItem->setForceUpgrade(true);
		if (aUpSubItem->parserFileName() != retOk) { //segmentation fault
			sprintf(record, "Can not upgrade : %s to version %s !",
					aUpSubItem->getMemberItemName(),
					aUpSubItem->getNewVersion());

			aUpSubItem->setUpgraderecord(record);
			return retError;
		}
		// high lower equal
		if (aUpSubItem->getUpStatus() != higherVerison) {

			sprintf(record, "Can not upgrade : %s to version %s !",
					aUpSubItem->getMemberItemName(),
					aUpSubItem->getNewVersion());
			aUpSubItem->setUpgraderecord(record);
			return retError;
		} else {
			if (strncmp(mSubItems[num].c_str() + strlen(upFilePath),
					MainRootfsUpgrade, strlen(MainRootfsUpgrade)) == 0) {
				cout << "up mainrootfs !!!!!!!!!!!!!!!!!!!!!!3" << endl;
				this->setUpSystem(true);
			}
		}
	}
	return retOk;
}
INT32 UpgradeDSPSubItem::upgradeItem(UINT32 num) {
	if (aUpSubItem->parserItemPackage(NULL) == retError) {
		FileOperation::deleteFile(mSubItems[num]);
		INT8 record[msgLen] = { 0 };
		sprintf(record, "Upgrade item : %s failed !",
				aUpSubItem->getMemberItemName());
		aUpSubItem->setUpgraderecord(record);
		return retError;
	} else
		FileOperation::deleteFile(mSubItems[num]);

	return retOk;
}
INT32 UpgradeDSPSubItem::excuteUpgradeShell(UINT32 num, INT8 *PCIP) {
	FILE *fstream = NULL;
	char buff[256] = { 0 };
	string exeCMD = "cd ";
	exeCMD += upFilePath;
	exeCMD += " && chmod +x ";
	exeCMD += UpgradeShell;
	exeCMD += " && ./";
	exeCMD += UpgradeShell;
	if (this->getUpSystem()) {
		exeCMD += " ";
		cout << "PCIP::: " << PCIP << endl;
		exeCMD += PCIP;
	}
	if (NULL == (fstream = popen(exeCMD.c_str(), "r"))) {
		fprintf(stderr, "execute command failed: %s", strerror(errno));
		INT8 record[msgLen] = { 0 };
		sprintf(record, "Upgrade item : %s failed !",
				aUpSubItem->getMemberItemName());
		aUpSubItem->setUpgraderecord(record);
		return retError;
	}
	INT8 record[msgLen] = { 0 };
	while (NULL != fgets(buff, sizeof(buff), fstream)) {
		printf("buff: %s", buff);
		//()==0
		if ((strncmp(buff, SuccessUpShellRespond, strlen(SuccessUpShellRespond))
				== 0)) {
			memset(record, 0, msgLen);
			sprintf(record, "Upgrade item : %s successed !",
					aUpSubItem->getMemberItemName());
			aUpSubItem->setUpResult(true);
		} else if ((strncmp(buff, FailUpShellRespond,
				strlen(FailUpShellRespond)) == 0)) {
			memset(record, 0, msgLen);
			sprintf(record, "Upgrade item : %s failed !",
					aUpSubItem->getMemberItemName());
			aUpSubItem->setUpgraderecord(record);
			aUpSubItem->setUpResult(false);
			pclose(fstream);
			return retError;
		}
		memset(buff, 0, 256);
	}
	pclose(fstream);
	FileOperation::deleteFile(UpgradeShellWithPath);
	return retOk;
}
//S_IRUSR 拥有者读
//* S_IWUSR 拥有者写
//* S_IXUSR 拥有者执行
//* S_IRGRP 组读
//* S_IWGRP 组写
//* S_IXGRP 组执行
//* S_IROTH 其他用户读
//* S_IWOTH 其他用户写
//* S_IXOTH 其他用户执行
//(string strPort, string strKey,
//		string strPath, string& strResult)
INT32 UpgradeDSPSubItem::modifyVersionFile() {
	if (!aUpSubItem->getUpResult()) {
		eachItemUpStatus = false;
		return retError;
	}
	string versionLine;
	versionLine += aUpSubItem->getVersionFileItemName();
	versionLine += "=";
	versionLine += (aUpSubItem->getLocalVersion() + 1);
	string newVersionLine;
	newVersionLine += aUpSubItem->getVersionFileItemName();
	newVersionLine += "=";
	newVersionLine += (aUpSubItem->getNewVersion() + 1);
	chmod(pathVersionFile, S_IWUSR | S_IWGRP | S_IWOTH);
	ifstream in;
	char line[64];
	in.open(pathVersionFile);
	string strTemp;
	while (in.getline(line, sizeof(line))) {
		string strline = line;
		if (versionLine == strline) {
			strline = newVersionLine;
			strTemp += strline + '\n';
		} else {
			strTemp += strline + '\n';
		}
	}
	in.close();
	ofstream out(pathVersionFile);
	if (out.is_open()) {
		out.flush();
		out << strTemp;
		out.close();
	} else {
		return retError;
	}
	chmod(pathVersionFile, S_IRUSR | S_IRGRP | S_IROTH);
	return retOk;
}

