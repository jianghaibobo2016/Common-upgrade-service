#ifndef UPFILEATTRS_H
#define UPFILEATTRS_H
#include "GlobDefine.h"
#include "RCSP.h"
#include "FileTrans.h"
#include "UpgradeServiceConfig.h"
class UpFileAttrs {
public:

	static SmartPtr<UpFileAttrs> createFileAttrs();
	UpFileAttrs(const UpFileAttrs &);
	UpFileAttrs &operator=(const UpFileAttrs &fileAttr);

	void setFileDownloadPath(const INT8 *path, INT32 length);
	void setNewSoftVersion(const INT8 *version, INT32 length);
	void setNewSoftFileSize(const INT32 size);
	void setFileMD5Code(const INT8 *MD5, INT32 length);

	const INT8 *getFileDownloadPath() const {
		return fileDownloadPath;
	}
	const INT8 *getNewSoftVersion() const {
		return newSoftVersion;
	}
	const UINT32 getNewSoftFileSize() const {
		return newSoftFileSize;
	}
	const INT8 *getFileMD5Code() const {
		return upFileMD5code;
	}
	const bool getWebUpMethod() const {
		return webUpMethod;
	}
	const bool getInUpgradeStatus() const {
		return inUpgrading;
	}
	const FileTrans &getFileTrans() const {
		return fileTransRecord;
	}
	const bool getForceStatus() const {
		return forceUpgrade;
	}

	void setFileDownloadPath(const INT8 *path) {
		memset(fileDownloadPath, 0, fileDownloadPathSize);
		memcpy(fileDownloadPath, path, strlen(path));
	}
	void setNewSoftVersion(const INT8 *version) {
		memset(newSoftVersion, 0, newSoftVersionSize);
		memcpy(newSoftVersion, version, strlen(version));
	}
	void setNewFileSize(const UINT32 size) {
		newSoftFileSize = size;
	}
	void setUpFileMD5Code(const INT8 *code) {
		memset(upFileMD5code, 0, upgradeFileMd5Size);
		memcpy(upFileMD5code, code, strlen(code));
	}
	void setWebUpMethod(const bool method) {
		webUpMethod = method;
	}
	void setInUpgradeStatus(const bool status) {
		inUpgrading = status;
	}
	void setFileTransRecord(const FileTrans& fileTrans) {
		fileTransRecord = fileTrans;
	}
	void setForceUpgrade(const bool force) {
		forceUpgrade = force;
	}
	bool clearMemberData();
	~UpFileAttrs();
private:
	INT8 *fileDownloadPath; /* 64B */
	INT8 *newSoftVersion; /* 20B */
	UINT32 newSoftFileSize;
	INT8 *upFileMD5code; /* 16B */
	bool webUpMethod;
	bool inUpgrading;
	FileTrans fileTransRecord;

	bool forceUpgrade;

	UpFileAttrs();
};
#endif /* UPFILEATTRS_H */
