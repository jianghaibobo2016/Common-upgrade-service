/*
 * LogSizeLimit.cpp
 *
 *  Created on: Jul 19, 2018
 *      Author: jhb
 */

#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include "LogSizeLimit.h"
#include "IniConfigFile.h"
#include "Logger.h"
#include "FileOperation.h"
using namespace FrameWork;

LogSizeLimit::LogSizeLimit(const string iniFile) :
		_iniFile(iniFile), changeFlag(true) {
}

LogSizeLimit::~LogSizeLimit() {

}

bool LogSizeLimit::start() {
	pthread_t tid, changetid;
	pthread_create(&tid, NULL, run, (void*) this);
	pthread_create(&changetid, NULL, listenChanged, (void*) this);
	return true;
}

bool LogSizeLimit::readConfig() {
	//Clean map vars
	map<string, string>::iterator iter_tmp = _catelogAttr.begin();
	while (iter_tmp != _catelogAttr.end()) {
		_catelogAttr.erase(iter_tmp);
		iter_tmp++;
	}
	//Clean map vars
	iter_tmp = _specialFileAttr.begin();
	while (iter_tmp != _specialFileAttr.end()) {
		_specialFileAttr.erase(iter_tmp);
		iter_tmp++;
	}
	//Clean map vars
	map<string, vector<string> >::iterator mvIter_tmp = _logFiles.begin();
	while (mvIter_tmp != _logFiles.end()) {
		_logFiles.erase(mvIter_tmp);
		mvIter_tmp++;
	}

	if (!FileOperation::isExistFile(_iniFile)) {
		Logger::GetInstance().Fatal("Can not find ini file : %s !",
				_iniFile.c_str());
		return false;
	}
	//Create ini file handling var.
	IniConfigFile iniSet(_iniFile.c_str());

	INT8 logPath[LogFilePathMaxLen] = { 0 };
	INT8 logFileAttr[LogFileAttrMaxLen] = { 0 };

	INT8 specialFile_1[LogFilePathMaxLen] = { 0 };
	INT8 specialFileAttr_1[LogFileAttrMaxLen] = { 0 };

	const INT8 *catelogBase = "catelog_";
	const INT8 *limitBase = "logSizeLimit_";
	const INT8 *specialFileBase = "specialFile_";
	const INT8 *specialLimitBase = "specialFileSizeLimit_";

	INT8 catelogCnt = '1';
	bool isExist = true;

	INT8 catelogIndex[32] = { 0 };
	INT8 logSizeIndex[32] = { 0 };

	while (isExist) {

		memset(catelogIndex, 0, 32);
		memset(logSizeIndex, 0, 32);
		memset(logPath, 0, LogFilePathMaxLen);
		memset(logFileAttr, 0, LogFileAttrMaxLen);

		memcpy(catelogIndex, catelogBase, strlen(catelogBase));
		memcpy(catelogIndex + strlen(catelogBase), &catelogCnt, 1);
		memcpy(logSizeIndex, limitBase, strlen(limitBase));
		memcpy(logSizeIndex + strlen(limitBase), &catelogCnt, 1);

		//section key value
		do {
			if (iniSet.readIniConfFile("LogSizeManage", catelogIndex, logPath,
					LogFilePathMaxLen) != true) {
				Logger::GetInstance().Error("Can not locate %s !",
						catelogIndex);
				isExist = false;
				break;
			} else
				Logger::GetInstance().Info("Get a catelog %s=%s !",
						catelogIndex, logPath);

			if (iniSet.readIniConfFile("LogSizeManage", logSizeIndex,
					logFileAttr, LogFilePathMaxLen) != true) {
				Logger::GetInstance().Error(
						"Can not get size %s, using default size %s !",
						logSizeIndex, LogFileSizeDefault);
				memcpy(logFileAttr, LogFileSizeDefault,
						strlen(LogFileSizeDefault));
			} else
				Logger::GetInstance().Info("Get a log size attr %s !",
						logFileAttr);

			_catelogAttr[logPath] = logFileAttr;
			isExist = true;
			map<string, string>::iterator iter;
			iter = _catelogAttr.begin();
			for (UINT32 i = 1; i < _catelogAttr.size(); i++)
				iter++;
			cout << "_catelogAttr_size : " << _catelogAttr.size() << " begin : "
					<< iter->first << " end : " << iter->second << endl;

		} while (0); // read the normal catelogs

		memset(catelogIndex, 0, 32);
		memset(logSizeIndex, 0, 32);
		memset(logPath, 0, LogFilePathMaxLen);
		memset(logFileAttr, 0, LogFileAttrMaxLen);

		memcpy(catelogIndex, specialFileBase, strlen(specialFileBase));
		memcpy(catelogIndex + strlen(specialFileBase), &catelogCnt, 1);
		memcpy(logSizeIndex, specialLimitBase, strlen(specialLimitBase));
		memcpy(logSizeIndex + strlen(specialLimitBase), &catelogCnt, 1);

		do {
			if (iniSet.readIniConfFile("LogSizeManage", catelogIndex, logPath,
					LogFilePathMaxLen) != true) {
				Logger::GetInstance().Error("Can not locate a special log %s !",
						catelogIndex);
				break;
			} else
				Logger::GetInstance().Info("Get a special log %s=%s !",
						catelogIndex, logPath);

			if (iniSet.readIniConfFile("LogSizeManage", logSizeIndex,
					logFileAttr, LogFilePathMaxLen) != true) {
				Logger::GetInstance().Error(
						"Can not get log size %s, using default size %s !",
						logFileAttr, LogFileSizeDefault);
				memcpy(logFileAttr, LogFileSizeDefault,
						strlen(LogFileSizeDefault));
//				break;
			} else
				Logger::GetInstance().Info("Get a special log size %s !",
						logFileAttr);
			if (!isExist)
				isExist = true;

			_specialFileAttr[logPath] = logFileAttr;
			map<string, string>::iterator iter;
			iter = _specialFileAttr.begin();
			for (UINT32 i = 1; i <= _specialFileAttr.size(); i++) {
				cout << "_specialFileAttr_size : " << _specialFileAttr.size()
						<< " begin : " << iter->first << " end : "
						<< iter->second << endl;
				iter++;
			}
		} while (0); // read the special log files

		catelogCnt++;
	} //while

	return true;
}

//struct dirent
//{
//   long d_ino; /* inode number 索引节点号 */
//   off_t d_off; /* offset to this dirent 在目录文件中的偏移 */
//   unsigned short d_reclen; /* length of this d_name 文件名长 */
//   unsigned char d_type; /* the type of d_name 文件类型 */其中d_type表明该文件的类型：文件(8)、目录(4)、链接文件(10)等。
//   char d_name [NAME_MAX+1]; /* file name (null-terminated) 文件名，最长255字符 */
//}

bool LogSizeLimit::readFileList(const INT8 *mapStrPath, const INT8 *basePath) {
	DIR *dir;
	struct dirent *direntPtr;
	char base[LogFilePathMaxLen] = { 0 };

	if ((dir = opendir(basePath)) == NULL) {
		Logger::GetInstance().Error("Can not open directory %s ! ", basePath);
		return false;
	}

	while ((direntPtr = readdir(dir)) != NULL) {
		if (strcmp(direntPtr->d_name, ".") == 0
				|| strcmp(direntPtr->d_name, "..") == 0) ///current dir OR parrent dir
			continue;
		else if (direntPtr->d_type == 8) {  ///file
			string fileName;
			fileName += basePath;
			if (basePath[strlen(basePath) - 1] != '/')
				fileName += '/';
			fileName += direntPtr->d_name;
			_logFiles[mapStrPath].push_back(fileName);
		} else if (direntPtr->d_type == 10)    ///link file
			Logger::GetInstance().Info("A link file : %s !", direntPtr->d_name);
		else if (direntPtr->d_type == 4)    ///dir
				{
			memset(base, '\0', sizeof(base));
			strcpy(base, basePath);
			if (base[strlen(base) - 1] != '/')
				strcat(base, "/");
			strcat(base, direntPtr->d_name);
			readFileList(mapStrPath, base);
		}
	}
	closedir(dir);
	return true;
}

bool LogSizeLimit::limitLogFile() {
	changeFlag = true;
	map<string, string>::iterator mIter;
	for (mIter = _catelogAttr.begin(); mIter != _catelogAttr.end(); mIter++) {
		if (!FileOperation::isExisteDirectory(mIter->first)) {
			Logger::GetInstance().Fatal("Catelog %s is not existed !",
					mIter->first.c_str());
		} else {
			Logger::GetInstance().Info("catelog %s is existed !",
					mIter->first.c_str());
			readFileList(mIter->first.c_str(), mIter->first.c_str());
		}
	}

	//
	map<string, vector<string> >::iterator mvIter = _logFiles.begin();
	vector<string>::iterator vIter;
	bool deleteFlag = false;
	while (mvIter != _logFiles.end()) {
		vIter = mvIter->second.begin();
		while (vIter != mvIter->second.end()) {
			Logger::GetInstance().Info("Log file : %s : %s ",
					mvIter->first.c_str(), vIter->c_str());
			mIter = _specialFileAttr.begin();
			for (; mIter != _specialFileAttr.end(); mIter++) {
				deleteFlag = false;
				//Remove the special log file from the normal catelog
				if (strncmp(vIter->c_str(), mIter->first.c_str(),
						strlen(vIter->c_str())) == 0) {
					Logger::GetInstance().Info(
							"Remove log file for special log : %s !",
							vIter->c_str());
					vIter = _logFiles[mvIter->first].erase(vIter);
					deleteFlag = true;
					break;
				}
			}
			if (!deleteFlag)
				vIter++;
		}
		mvIter++;
	}

	// Check change signal
	while (changeFlag) {
		mIter = _catelogAttr.begin();
		for (; mIter != _catelogAttr.end(); mIter++) {
			vIter = _logFiles[mIter->first].begin();
			for (; vIter != _logFiles[mIter->first].end(); vIter++) {
//				cout << "log clean : " << *vIter << " size ; "
//						<< transSizeToBytes(mIter->second.c_str()) << endl;
				checkAndCleanLog(*vIter,
						transSizeToBytes(mIter->second.c_str()));
			}
		}

		mIter = _specialFileAttr.begin();
		for (; mIter != _specialFileAttr.end(); mIter++) {
//			cout << "special log clean : "<<mIter->first<<endl;
			checkAndCleanLog(mIter->first,
					transSizeToBytes(mIter->second.c_str()));
		}
		sleep(CleanInterval);
	}

	return true;
}

bool LogSizeLimit::checkAndCleanLog(string logName, INT32 size) {
	struct stat statbuff;
	if (-1 == stat(logName.c_str(), &statbuff)) {
//		Logger::GetInstance().Error("Can not Stat() log file %s !",
//				logName.c_str());
		return false;
	}

	//Clean file
	if (statbuff.st_size >= size) {
		fstream fout(logName.c_str(), ios::out | ios::trunc);
		fout.close();
	}
	return true;
}

// Get bytes
INT32 LogSizeLimit::transSizeToBytes(const INT8 * size) {
	if (size[strlen(size) - 1] == 'M' || size[strlen(size) - 1] == 'm') {
		INT8 msize[32] = { 0 };
		memcpy(msize, size, strlen(size) - 1);
		return atoi(msize) * 1024 * 1024;
	} else if (size[strlen(size) - 1] == 'K' || size[strlen(size) - 1] == 'k') {
		INT8 ksize[32] = { 0 };
		memcpy(ksize, size, strlen(size) - 1);
		return atoi(ksize) * 1024;
	} else {
		Logger::GetInstance().Error("Unknow size %s !", size);
		return 0;
	}
}

void *LogSizeLimit::run(void *args) {
	LogSizeLimit *logLimit = (LogSizeLimit*) args;
	do {
		logLimit->readConfig();
		logLimit->limitLogFile();
	} while (logLimit->getChangeFlag() == false);

	return NULL;
}

void *LogSizeLimit::listenChanged(void*args) {
	LogSizeLimit *logLimit = (LogSizeLimit*) args;
	logLimit->readPipeMsg();
	return NULL;
}

// Listen fifo signal
bool LogSizeLimit::readPipeMsg() {
	INT8 buf_r[100];
	INT32 fd;
	INT32 nread;

	if ((mkfifo(LogPipeName, O_CREAT | O_EXCL) < 0) && (errno != EEXIST))
		Logger::GetInstance().Error("Can not create fifo server !");

	memset(buf_r, 0, sizeof(buf_r));
	fd = open(LogPipeName, O_RDONLY | O_NONBLOCK, 0);
	if (fd == -1) {
		Logger::GetInstance().Error("Can not open fifo %s for %s !",
				LogPipeName, strerror(errno));
		return false;
	}
	while (1) {
		memset(buf_r, 0, sizeof(buf_r));
		if ((nread = read(fd, buf_r, 100)) == -1) {
			// if (errno == EAGAIN)
			// printf("no data yet\n");
		}
		if ((strncmp(buf_r, ChangedStr, strlen(ChangedStr))) == 0) {
			Logger::GetInstance().Info("Get changed cmd !");
			setChangeFlag(false);
		}
		sleep(2);
	}
	unlink(LogPipeName);
	return true;
}
