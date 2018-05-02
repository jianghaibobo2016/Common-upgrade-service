#include <string.h>
#include <iostream>
#include "UpFileAttrs.h"
#include "UpgradeServiceConfig.h"
using namespace std;

UpFileAttrs::UpFileAttrs() :
		fileDownloadPath(), newSoftVersion(), newSoftFileSize(0), upFileMD5code(), webUpMethod(
				false), inUpgrading(false), fileTransRecord(), forceUpgrade(
				false) {
}

UpFileAttrs::~UpFileAttrs() {
	delete[] fileDownloadPath;
	delete[] newSoftVersion;
	delete[] upFileMD5code;
}

SmartPtr<UpFileAttrs> UpFileAttrs::createFileAttrs() {
	static SmartPtr<UpFileAttrs> upFileAttrs(new UpFileAttrs());
	upFileAttrs->fileDownloadPath = new char[fileDownloadPathSize];
	upFileAttrs->newSoftVersion = new char[newSoftVersionSize];
	upFileAttrs->upFileMD5code = new char[upgradeFileMd5Size];
	return upFileAttrs;
}

UpFileAttrs::UpFileAttrs(const UpFileAttrs & tmpFileAttr) {
	fileDownloadPath = new char[fileDownloadPathSize];
	memcpy(fileDownloadPath, tmpFileAttr.fileDownloadPath,
			strlen(tmpFileAttr.fileDownloadPath));
	newSoftVersion = new char[newSoftVersionSize];
	memcpy(newSoftVersion, tmpFileAttr.newSoftVersion,
			strlen(tmpFileAttr.newSoftVersion));
	newSoftFileSize = tmpFileAttr.newSoftFileSize;
	upFileMD5code = new char[upgradeFileMd5Size];
	memcpy(upFileMD5code, tmpFileAttr.upFileMD5code,
			strlen(tmpFileAttr.upFileMD5code));
	webUpMethod = tmpFileAttr.webUpMethod;
	inUpgrading = tmpFileAttr.inUpgrading;
	fileTransRecord = tmpFileAttr.fileTransRecord;
	forceUpgrade = tmpFileAttr.forceUpgrade;
}

UpFileAttrs &UpFileAttrs::operator=(const UpFileAttrs &fileAttr) {
	if (this != &fileAttr) {
		setFileDownloadPath(fileAttr.getFileDownloadPath());
		setNewSoftVersion(fileAttr.getNewSoftVersion());
		setNewFileSize(fileAttr.getNewSoftFileSize());
		setUpFileMD5Code(fileAttr.getFileMD5Code());
		setWebUpMethod(fileAttr.getWebUpMethod());
		setInUpgradeStatus(fileAttr.getInUpgradeStatus());
		setFileTransRecord(fileAttr.getFileTrans());
		setForceUpgrade(fileAttr.getForceStatus());
	}
	return *this;
}

void UpFileAttrs::setFileDownloadPath(const INT8 *path, INT32 length) {
	memset(fileDownloadPath, 0, fileDownloadPathSize);
	memcpy(fileDownloadPath, path, length);
}
void UpFileAttrs::setNewSoftVersion(const INT8 *version, INT32 length) {
	memcpy(newSoftVersion, version, length);
}
void UpFileAttrs::setNewSoftFileSize(const INT32 size) {
	newSoftFileSize = size;
}
void UpFileAttrs::setFileMD5Code(const INT8 *MD5, INT32 length) {
	memcpy(upFileMD5code, MD5, length);
}

bool UpFileAttrs::clearMemberData() {
	memset(this->fileDownloadPath, 0, fileDownloadPathSize);
	memset(this->newSoftVersion, 0, newSoftVersionSize);
	this->newSoftFileSize = 0;
	memset(this->upFileMD5code, 0, upgradeFileMd5Size);
	webUpMethod = false;
	inUpgrading = false;
	return true;
}
