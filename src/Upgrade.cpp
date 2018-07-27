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
	memset(newVersion, 0, 8);
	memset(localVersion, 0, 8);
	memset(dependVersion, 0, 8);
	memset(itemName, 0, 16);
	memset(versionFileItemName, 0, 32);
	memset(upgraderecord, 0, msgLen);
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
		if (i >= 6)
			break;
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

	cout << "newVersion::" << localVersion << " " << __FUNCTION__ << endl;
	if (retUpStatus > 0)
		return higherVerison;
	else if (retUpStatus == 0) {
		strcpy(upgraderecord, NONEEDTOUPGRADE);
		return equalVersion;
	} else if (retUpStatus < 0) {
		strcpy(upgraderecord, "Upgrade file version error !");
		return lowerVersion;
	} else
		return errorVersionStatus;
}

INT32 UpgradeDSP::parserFileName() {
	/*DPS9903*/
	memset(upgraderecord, 0, msgLen);
	fileWithoutPath = upgradeFile + strlen(upFilePath);
	Logger::GetInstance().Info("File name without path : %s", fileWithoutPath);
	if (compareUpgradeItem(fileWithoutPath, TerminalDevType) != 0) {
		strcpy(upgraderecord, "Upgrade file name error1 !");
		return retError;
	}
	/* get NAND*//*compare item*/
	if (getItemName() != retOk) {
		strcpy(upgraderecord, "Upgrade file name error 2!");
		return retError;
	}
	if (getForceStatus() != true)
		Logger::GetInstance().Info("Not force upgrade method !");
	if ((upStatus = getVersion()) != higherVerison
			&& getForceStatus() != true) {
		if (FileOperation::isExistFile(upgradeFile)) {
			FileOperation::deleteFile(upgradeFile);
		}
		Logger::GetInstance().Info("%s() : Upgrade status : %d", __FUNCTION__,
				upStatus);
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
	Logger::GetInstance().Info("New version in line : %s !",
			newVersionLine.c_str());

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
	upStatus = errorVersionStatus;
	;
	productItem = false;
	memset(itemName, 0, strlen(itemName));
	memset(versionFileItemName, 0, strlen(versionFileItemName));
	memset(upgraderecord, 0, strlen(upgraderecord));
	upResult = false;
}

UpgradeDSPSubItem::UpgradeDSPSubItem() :
		mSubItems(), productTarFile(newTarPackage), eachItemUpStatus(true), mSubItemName(), upSystem(
				false), upTerminalDevs(false), upDevType(ERROR_TYPE), forceUpgrade(
				false) {
	aUpSubItem = new UpgradeDSP(NULL);
}
UpgradeDSPSubItem::~UpgradeDSPSubItem() {
	delete aUpSubItem;
}

bool UpgradeDSPSubItem::getSubItems(
		map<INT32, DEV_MODULES_TYPE>& devModuleToUpgrade) {
	for (UINT32 i = 1; i <= mSubItems.size(); i++) {
		string strTmp = mSubItems[i];
		INT32 npos = strTmp.find("_V");

		mSubItemName[i] = strTmp.substr(strlen(StringFind),
				npos - strlen(StringFind));
	}
//		mSubItems.clear();
	cout << "test amp map::3 " << mSubItemName[1] << endl;
	cout << "test amp sizedevM::4 " << devModuleToUpgrade.size() << endl;
	//adjust order
	adjustOrder(devModuleToUpgrade);
	cout << "test amp map::2 " << mSubItems[1] << endl;
	return true;
}

bool UpgradeDSPSubItem::adjustOrder(
		map<INT32, DEV_MODULES_TYPE>& devModuleToUpgrade) {
	map<INT32, string> m_dev_type;
	map<INT32, string> m_devs;
	map<INT32, string> m_subItem;
	UINT32 i = 1;
	for (i = 1; i <= devModuleToUpgrade.size(); i++) {
#if !(DSP9909)
		if (devModuleToUpgrade[i] == DEV_AMPLIFIER)
			m_dev_type[i] = AMPLIFIER;
#endif
#if (DSP9903)
		else if (devModuleToUpgrade[i] == DEV_PAGER)
			m_dev_type[i] = PAGER;
#endif
#if !(DSP9909)
		else {
			return false;
		}
#endif
	}
	UINT32 nameLen = mSubItems.size();
	UINT32 moduleLen = m_dev_type.size();
	INT32 flag = 0;
	INT32 mSubNum = 1;
	INT32 mDevNum = 1;
	bool matchFlag = false;

	for (i = 1; i <= nameLen; i++) {
		for (UINT32 j = 1; j <= moduleLen; j++) {
			if (mSubItems[i].find(m_dev_type[j]) != string::npos) {
				m_devs[j] = mSubItems[i];
//				mDevNum++;
				matchFlag = true;
				break;
			}
		}
		if (!matchFlag && mSubItemName[i] != AMPLIFIER
				&& mSubItemName[i] != PAGER) {
//			mSubItems[mSubNum] = mSubItemName[i];
			m_subItem[mSubNum] = mSubItems[i];
			cout << m_subItem[mSubNum] << endl;
			matchFlag = false;
			mSubNum++;
		}
		matchFlag = false;
	}

	mSubItems.clear();
	for (i = 1; i <= m_subItem.size(); i++) {
		mSubItems[i] = m_subItem[i];
		cout << "test amp map::111 " << mSubItems[i] << endl;
	}
	cout << "size::2:::::::" << mSubItems.size() << endl;
	mSubNum = m_subItem.size();
	cout << "msubnum::::" << mSubNum << endl;
	for (i = 1; i <= m_devs.size(); i++) {
		mSubItems[mSubNum + i] = m_devs[i];
		cout << "test amp map::11 " << mSubItems[1] << endl;
	}
//	cout << "test amp mapppppppppppppppppppppppppppppppppp::22 " << mSubItems[1]
//			<< endl;
//	if (devModuleToUpgrade.size() != 0) {
	cout << "size::3:::::::" << mSubItems.size() << endl;
	UINT32 itemsNum = mSubItems.size();
	cout << "jaingui:::::" << itemsNum << endl;
	for (i = 1; i <= itemsNum; i++) {
		string tmp = upFilePath;
		tmp += mSubItems[i];
		mSubItems[i] = tmp;
		cout << "path item ::::" << itemsNum << " " << mSubItems[i] << endl;
//		mSubItems[i] = m_subItem[i];
	}
//	}
	cout << "test amp map::33 " << mSubItems.size() << endl;
	return true;
}

INT32 UpgradeDSPSubItem::parserSubItemsFileName(UINT32 num) {

	if (strncmp(mSubItems[num].c_str() + strlen(upFilePath), AmplifierUpgrade,
			strlen(AmplifierUpgrade)) == 0) {
		Logger::GetInstance().Info("Upgrade Amplifier !!");
		this->setUpTerminalDevs(true);
#if !(DSP9909)
		this->setUpDevType(UPDATE_DEV_AMP_TYPE);
#endif
		aUpSubItem->setItemName(AMPLIFIER);
	} else if (strncmp(mSubItems[num].c_str() + strlen(upFilePath),
			PagerUpgrade, strlen(PagerUpgrade)) == 0) {
		Logger::GetInstance().Info("Upgrade Pager !!");
		this->setUpTerminalDevs(true);
#if (DSP9903)
		this->setUpDevType(UPDATE_DEV_PAGER_TYPE);
#endif
		aUpSubItem->setItemName(PAGER);
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
				Logger::GetInstance().Info("Upgrade mainrootfs !!");
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
	UINT32 iPosBuff = 0;
	char module[3] = { 0 };
	char upStatus[3] = { 0 };
	bool upSuccessed = false;
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
		if ((strncmp(buff, UpShellRespondTag_1, 2)) == 0) {
			iPosBuff += 2;
			memcpy(module, &buff[iPosBuff], 2);
			iPosBuff += 2;
			if ((strncmp(&buff[iPosBuff], UpShellRespondTag_3_Success, 2))
					== 0) {
				cout << "trueeeeeeeeee " << buff[iPosBuff] << endl;
				upSuccessed = true;
			} else {
				memcpy(upStatus, &buff[iPosBuff], 2);
				upSuccessed = false;
			}
			iPosBuff = 0;
		} else
			continue;
	}
	if (upSuccessed == true) {
		memset(record, 0, msgLen);
		Logger::GetInstance().Info("Upgrade item : %s--%s successed !", module,
				aUpSubItem->getMemberItemName());
//		sprintf(record, "Upgrade item : %s--%s successed !", module,
//				aUpSubItem->getMemberItemName());
		aUpSubItem->setUpResult(true);
	} else if (upSuccessed == false) {
		memset(record, 0, msgLen);
		sprintf(record, "Upgrade item : %s--%s failed for %s !", module,
				aUpSubItem->getMemberItemName(), upStatus);
		aUpSubItem->setUpgraderecord(record);
		aUpSubItem->setUpResult(false);
		pclose(fstream);
		return retError;
	}
	memset(buff, 0, 256);
	pclose(fstream);
	FileOperation::deleteFile(UpgradeShellWithPath);
	return retOk;
}

bool UpgradeDSPSubItem::excuteDevShell() {
	FILE *fstream = NULL;
	char buff[256] = { 0 };
	string exeCMD = "cd ";
	exeCMD += upFilePath;
	exeCMD += " && chmod +x ";
	exeCMD += UpgradeShell;
	exeCMD += " && ./";
	exeCMD += UpgradeShell;
	if (NULL == (fstream = popen(exeCMD.c_str(), "r"))) {
		fprintf(stderr, "execute command failed: %s", strerror(errno));
		INT8 record[msgLen] = { 0 };
		return retError;
	}
	INT8 record[msgLen] = { 0 };
	while (NULL != fgets(buff, sizeof(buff), fstream)) {
		printf("buff: %s", buff);
		//()==0
		if ((strncmp(buff, SuccessUpShellRespond, strlen(SuccessUpShellRespond))
				== 0)) {
			return true;
		} else if ((strncmp(buff, FailUpShellRespond,
				strlen(FailUpShellRespond)) == 0)) {
			pclose(fstream);
			return false;
		}
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

