///////////
#include <iostream>
#include <Logger.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "Upgrade.h"
#include "CrcCheck.h"
#include "DevSearch.h"
#include "FileOperation.h"
#include "RCSP.h"
using namespace std;
using namespace FrameWork;
#if 1
//UpgradeDSP::UpgradeDSP() :
//		upgradeFile(NULL), fileWithoutPath(NULL), newVersion(NULL), localVersion(
//		NULL), upStatus(errorVersionStatus), productItem(false), itemName(
//		NULL), upgraderecord(
//		NULL) {
//
//}
UpgradeDSP::UpgradeDSP(INT8 *upgradeFile) :
		upgradeFile(upgradeFile), fileWithoutPath(
				upgradeFile + strlen(upFilePath)), upStatus(errorVersionStatus), productItem(
				false), upResult(false)

// : versionNumber(versionNumber),
//   newSoftFileSize(0),
//    : devInfo(devInfo),
//      fileAttrs(fileAttrs),
//      itemsUpgrade(0),
//      fileAbsolutePath(UPFILEABSOLUTEPATH),
//      m_upgradeFileStatus(errorVersionStatus),
//      singleItem(true)
{
	cout << "construct " << endl;
//	fileWithoutPath = new INT8[32];
	newVersion = new INT8[8];
	localVersion = new INT8[8];
	itemName = new INT8[16];
	versionFileItemName = new INT8[32];
	upgraderecord = new INT8[msgLen];
//	subUpgrade = new int*[subNum];
//	for (int i = 0; i < subNum; ++i) {
//		subUpgrade[i] = new int[subNum];
}
//UpgradeDSP::UpgradeDSP(const UpgradeDSP &) {
//
//}
//UpgradeDSP& UpgradeDSP::operator=(const UpgradeDSP &rhs) {
//	cout << "========================" << endl;
////	memcpy(upgradeFile, rhs.upgradeFile, strlen(rhs.upgradeFile));
//	upgradeFile = rhs.upgradeFile;
////	memcpy(fileWithoutPath, rhs.fileWithoutPath, strlen(rhs.fileWithoutPath));
//	fileWithoutPath = rhs.fileWithoutPath;
//	memcpy(newVersion, rhs.newVersion, strlen(rhs.newVersion));
//	memcpy(localVersion, rhs.localVersion, strlen(rhs.localVersion));
//	memcpy(itemName, rhs.itemName, strlen(rhs.itemName));
//	memcpy(upgraderecord, rhs.upgraderecord, strlen(rhs.upgraderecord));
//	cout << "========================222" << endl;
//	return *this;
//}
// newSoftVersion = new INT8[20];
// upFileMD5code = new INT8[16];
// downloadFile = new INT8[16];
//	devReplyDevUpgrade = new DEV_Reply_DevUpgrade();
//}

UpgradeDSP::~UpgradeDSP() {
//	if (fileWithoutPath != NULL || newVersion != NULL || itemName != NULL
//			|| upgraderecord != NULL) {
	cout << "destruct1" << endl;
//	delete[] fileWithoutPath;
	delete[] newVersion;
	delete[] localVersion;
	delete[] itemName;
	delete[] versionFileItemName;
	delete[] upgraderecord;
//	}
	cout << "destruct2" << endl;
// delete[] newSoftVersion;
// delete[] upFileMD5code;
// delete[] downloadFile;
//	delete devReplyDevUpgrade;
}

#if 0
DEV_Reply_DevUpgrade *UpgradeDSP::devReplyUpgradeCheck(PC_Request_DevUpgrade *pcRequestDevUpgrade, INT8 *versionNumber)
{
	if (pcRequestDevUpgrade == NULL)
	{
		Logger::GetInstance().Error("%s() : PC request error !", __FUNCTION__);
		return NULL;
	}
// memcpy(this->newSoftVersion, pcRequestDevUpgrade->NewSoftVersion, strlen(pcRequestDevUpgrade->NewSoftVersion));
// this->newSoftFileSize = pcRequestDevUpgrade->NewSoftDocumentSize;
// memcpy(this->upFileMD5code, pcRequestDevUpgrade->UpgradeFileMd5, strlen(pcRequestDevUpgrade->UpgradeFileMd5));
// INT8
	UINT8 FailReasonLen = 0;
// devReplyDevUpgrade->DevID
// FailReason
	memcpy(&devReplyDevUpgrade->header, &devReplyDevUpgrade->header, sizeof(devReplyDevUpgrade->header));
	INT32 retCompare = compareUpgradeItem(pcRequestDevUpgrade->NewSoftVersion, versionNumber);
	if (retCompare < 0)
	{
		devReplyDevUpgrade->Result = 0;
		FailReasonLen = strlen(LOWERVERSION);
		devReplyDevUpgrade->header.DataLen = sizeof(devReplyDevUpgrade->DevID) +
		sizeof(devReplyDevUpgrade->Result) + strlen(LOWERVERSION);
	} else if (retCompare = 0)
	{
		devReplyDevUpgrade->Result = 0;
		FailReasonLen = strlen(NONEEDTOUPGRADE);
		devReplyDevUpgrade->header.DataLen = sizeof(devReplyDevUpgrade->DevID) +
		sizeof(devReplyDevUpgrade->Result) + strlen(NONEEDTOUPGRADE);
	} else
	{
		devReplyDevUpgrade->Result = 1;
		devReplyDevUpgrade->header.DataLen = sizeof(devReplyDevUpgrade->DevID) +
		sizeof(devReplyDevUpgrade->Result);
	}
	return devReplyDevUpgrade;
}
#endif
// bool UpgradeDSP::upgradeInit()
// {
//     return true ;
// }

//DSP9903_NAND_V00.01
/*12 */
const INT8 UpgradeDSP::itemsSet[][16] = { "_V", "UBOOT", "KERNEL", "MROOTFS",
		"BROOTFS", "MediaApp", "ServerApp", "UPGRADE", "WEB", "NAND", "PRODUCT",
		"OVERFLAG" };

const INT8 UpgradeDSP::itemsVersionNameSet[11][32] = { "hardware_version",
		"uboot_version", "kernel_version", "main_rootfs_version",
		"backup_rootfs_version", "media_app_version", "server_app_version",
		"upgrade_version", "web_version", "nand_version", "product_version" };

INT32 UpgradeDSP::getItemName() {
	/*danchunwenjianming*/
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
//	cout << "new version : " << newVersion << "local version : " << localVersion
//			<< endl;
	INT32 retUpStatus = compareUpgradeItem(newVersion, localVersion);
//	cout << "111retstatuc : " << retUpStatus << endl;
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
	if ((upStatus = getVersion()) != higherVerison) {
		cout << __FUNCTION__ << "() upstatus : " << upStatus << endl;
		return retError;
	}
	return retOk;
}

INT32 UpgradeDSP::parserItemPackage(INT8 *PCRequestVersion) {
	if (CrcCheck::parser_Package(upgradeFile, newVersion, itemName) != 0) {
		return retError;
	}
	if (FileOperation::isExistFile(newTarPackage) != true) {
		return retError;
	} else {
		FileOperation::deleteFile(upgradeFile);
	}
	return retOk;
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
	upStatus = equalVersion;
	productItem = false;
	memset(itemName, 0, strlen(itemName));
	memset(versionFileItemName, 0, strlen(versionFileItemName));
	memset(upgraderecord, 0, strlen(upgraderecord));
	upResult = false;
}

#if 0
INT32 UpgradeDSP::checkDownloadFile(INT8 *upgradeItem, INT8 *itemVersion) {
	INT8 upFileName[32] = {0};
	getItemName(upFileName, upgradeItem);
// const INT8* m_downloadFile = downloadFile;
	if ((parserFileName(upFileName, itemVersion)) != higherVerison)
	return retError;

	if (parserUpgradeFile(upgradeItem) != retOk) {

		return retError;
	} else {
		deleteFile(upgradeItem);
		// remove(upgradeItem);
	}
// this->runUpgradeShell();

	return retOk;

}

INT32 UpgradeDSP::parserUpgradeFile(INT8 *upgradeItem) {
	if (CrcCheck::parser_Package(upgradeItem) != 0) {
		return retError;
	}
	return retOk;
}

INT32 UpgradeDSP::extractTar(const string fileName) {

	return retOk;
}

INT32 UpgradeDSP::checkUpgradeStatus() {
	return 0;
}

bool UpgradeDSP::modifyVersionFile() {
	return true;
}

//void UpgradeDSP::getItemName(INT8 *upFileName, INT8 *upFilePath) {
//	INT32 iPos = 0;
//	INT32 itemLen = strlen(upFilePath);
//	while (&upFilePath[itemLen - 1] != pathSymbol) {
//		upFileName[iPos] = upFilePath[itemLen - 1];
//		itemLen--;
//		iPos++;
//	}
//	INT8 tmp = 0;
//	INT32 length = strlen(upFileName);
//	// INT32 iPos_2 = 0;
//	iPos = 0;
//	for (iPos; iPos < length / 2; iPos++) {
//		tmp = upFileName[length - iPos - 1];
//		upFileName[length - iPos - 1] = upFileName[iPos];
//		upFileName[iPos] = tmp;
//	}
//}
#endif
void UpgradeDSP::runUpgradeShell() {
	cout << "run" << endl;
	INT8 cmd[1024] = { 0 };
	strncat(cmd, "cat ", strlen("cat "));
// strncat(cmd + 4, downloadFile, strlen(downloadFile));

	/* Product package */
// if ((strncmp(downloadFile + strlen(PRODUCT_TYPE) + 1, itemsSet[10], strlen(itemsSet[10]))) != 0)
// {
//     system(cmd);
// }else
// {
//     itemsUpgrade=2;
//     cout <<"items"<<itemsUpgrade<<endl;
//     for (int i = 0; i < itemsUpgrade; i++)
//     {
//         system("echo product");
//         system(cmd);
//     }
// }
// system(strUpgrade);
	return;
}
#endif

UpgradeDSPSubItem::UpgradeDSPSubItem() :
		productTarFile(newTarPackage), eachItemUpStatus(true), mSubItems(), mUpSubItem() {
	aUpSubItem = new UpgradeDSP(NULL);
}
UpgradeDSPSubItem::~UpgradeDSPSubItem() {
	delete aUpSubItem;
}
bool UpgradeDSPSubItem::getSubItems() {
	if (FileOperation::extractTarFile(productTarFile, mSubItems) != true) {
		FileOperation::deleteFile(productTarFile);
		return false;
	} else {
		for (UINT32 i = 1; i <= mSubItems.size(); i++) {
			string tmp = upFilePath;
			tmp += mSubItems[i];
			mSubItems[i] = tmp;
		}
	}
	FileOperation::deleteFile(productTarFile);
	return true;
}
INT32 UpgradeDSPSubItem::parserSubItemsFileName(UINT32 num) {
	aUpSubItem->setUpgradeFile(const_cast<INT8 *>(mSubItems[num].c_str()));
	INT8 record[msgLen] = { 0 };
	if (aUpSubItem->parserFileName() != retOk) { //segmentation fault
		sprintf(record, "Can not upgrade : %s to version %s !",
				aUpSubItem->getMemberItemName(), aUpSubItem->getNewVersion());
		aUpSubItem->setUpgraderecord(record);
		return retError;
	}
	// high lower equal
	if (aUpSubItem->getUpStatus() != higherVerison) {

		sprintf(record, "Can not upgrade : %s to version %s !",
				aUpSubItem->getMemberItemName(), aUpSubItem->getNewVersion());
		aUpSubItem->setUpgraderecord(record);
		return retError;
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
INT32 UpgradeDSPSubItem::excuteUpgradeShell(UINT32 num) {
	FILE *fstream = NULL;
	char buff[256] = { 0 };
	string exeCMD = "cd ";
	exeCMD += upFilePath;
	exeCMD += " && ./";
	exeCMD += UpgradeShell;
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
		if (strncmp(buff, SuccessUpShellRespond,
				strlen(SuccessUpShellRespond) == 0)) {
			memset(record, 0, msgLen);
			sprintf(record, "Upgrade item : %s successed !",
					aUpSubItem->getMemberItemName());
			aUpSubItem->setUpResult(true);
		} else if (strncmp(buff, FailUpShellRespond,
				strlen(FailUpShellRespond) == 0)) {
			memset(record, 0, msgLen);
			sprintf(record, "Upgrade item : %s failed !",
					aUpSubItem->getMemberItemName());
			aUpSubItem->setUpgraderecord(record);
			aUpSubItem->setUpResult(false);
			pclose(fstream);
			return retError;
		}
		aUpSubItem->setUpResult(true);
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

