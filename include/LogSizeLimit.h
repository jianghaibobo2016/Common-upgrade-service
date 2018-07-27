/*
 * LogSizeLimit.h
 *
 *  Created on: Jul 19, 2018
 *      Author: jhb
 */

#ifndef LOGSIZELIMIT_H_
#define LOGSIZELIMIT_H_

#include <map>
#include <vector>
#include <string.h>
#include "GlobDefine.h"
#include "Mutex.h"

static const INT8 *IniFileName = "/dsppa/LogService/LogSizeManage.ini";
static const INT8 * LogFileSizeDefault = "2M";
static const INT8 *LogPipeName = "/dsppa/LogService/LOGSIZE.fifo";
static const INT8 *ChangedStr = "changed";
static const UINT32 CleanInterval = 4;
static const UINT32 LogFilePathMaxLen = 64;
static const UINT32 LogFileAttrMaxLen = 8;

class LogSizeLimit {
//	static Mutex mutex;
public:
	LogSizeLimit(const string iniFile);
	~LogSizeLimit();
	bool start();
	bool getChangeFlag() {
		return changeFlag;
	}
	void setChangeFlag(bool status) {
		changeFlag = status;
	}
private:
	string _iniFile;
	map<string, string> _catelogAttr;
	map<string, string> _specialFileAttr;

	map<string, vector<string> > _logFiles;
	bool changeFlag;

	static void *run(void *args);
	static void *listenChanged(void*args);
	bool readConfig();
	bool limitLogFile();

	bool readFileList(const INT8 *mapStrPath, const INT8 *basePath);
	bool checkAndCleanLog(string logName, INT32 size);
	INT32 transSizeToBytes(const INT8 * size);
	bool readPipeMsg();
};

#endif /* LOGSIZELIMIT_H_ */
