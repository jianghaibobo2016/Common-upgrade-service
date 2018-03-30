#include <string.h>
#include "UpFileAttrs.h"
#include "UpgradeServiceConfig.h"

UpFileAttrs::UpFileAttrs() :
		fileDownloadPath(), newSoftVersion(), newSoftFileSize(0), upFileMD5code(), webUpMethod(
				false), inUpgrading(false), fileTransRecord() {
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
void UpFileAttrs::setFileDownloadPath(const INT8 *path, INT32 length) {
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

bool UpFileAttrs::clearMemberData(UpFileAttrs &) {
	memset(this->fileDownloadPath, 0, fileDownloadPathSize);
	memset(this->newSoftVersion, 0, newSoftVersionSize);
	this->newSoftFileSize = 0;
	memset(this->upFileMD5code, 0, upgradeFileMd5Size);
	webUpMethod = false;
	inUpgrading = false;
	return true;
}
