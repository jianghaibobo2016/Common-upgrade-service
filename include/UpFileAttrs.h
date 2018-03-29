#ifndef UPFILEATTRS_H
#define UPFILEATTRS_H
#include "GlobDefine.h"
#include "RCSP.h"
class UpFileAttrs {
public:

	static SmartPtr<UpFileAttrs> createFileAttrs();

	void setFileDownloadPath(const INT8 *path, INT32 length);
	void setNewSoftVersion(const INT8 *version, INT32 length);
	void setNewSoftFileSize(const INT32 size);
	void setFileMD5Code(const INT8 *MD5, INT32 length);

	INT8 *getFileDownloadPath() {
		return fileDownloadPath;
	}
	INT8 *getNewSoftVersion() {
		return newSoftVersion;
	}
	UINT32 getNewSoftFileSize() {
		return newSoftFileSize;
	}
	INT8 *getFileMD5Code() {
		return upFileMD5code;
	}

	bool clearMemberData(UpFileAttrs &);
	~UpFileAttrs();
private:
	INT8 *fileDownloadPath; /* 64B */
	INT8 *newSoftVersion; /* 20B */
	UINT32 newSoftFileSize;
	INT8 *upFileMD5code; /* 16B */
	UpFileAttrs();
};
#endif /* UPFILEATTRS_H */
