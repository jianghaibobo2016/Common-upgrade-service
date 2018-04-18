/*
 * FileOperation.cpp
 *
 *  Created on: Mar 25, 2018
 *      Author: jhb
 */

#include "FileOperation.h"
#include "UpgradeServiceConfig.h"
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <unistd.h>
#include <dirent.h>  // delete dir rmdir
#include <errno.h>

// #include <io.h>
//
FileOperation::FileOperation() {
	// 给m_strPath赋初值
	// string path = _pgmptr; // exe文件所在目录，带*.exe
	// m_strPath = path.substr(0, path.find_last_of('\\') + 1);
	// m_strPath += dir;

	// if (!IsExisteDirectory(m_strPath))
	// {
	//     string str = "md \"" + m_strPath + "\"";
	//     system(str.c_str());
	// }
}

FileOperation::~FileOperation(void) {
}

//#include <sys/types.h>
//#include <sys/stat.h>
//int mkdir(const char *pathname, mode_t mode);
//S_IRWXU
//00700权限，代表该文件所有者拥有读，写和执行操作的权限
//S_IRUSR(S_IREAD)
//00400权限，代表该文件所有者拥有可读的权限
//S_IWUSR(S_IWRITE)
//00200权限，代表该文件所有者拥有可写的权限
//S_IXUSR(S_IEXEC)
//00100权限，代表该文件所有者拥有执行的权限
//S_IRWXG
//00070权限，代表该文件用户组拥有读，写和执行操作的权限
//S_IRGRP
//00040权限，代表该文件用户组拥有可读的权限
//S_IWGRP
//00020权限，代表该文件用户组拥有可写的权限
//S_IXGRP
//00010权限，代表该文件用户组拥有执行的权限
//S_IRWXO
//00007权限，代表其他用户拥有读，写和执行操作的权限
//S_IROTH
//00004权限，代表其他用户拥有可读的权限
//S_IWOTH
//00002权限，代表其他用户拥有可写的权限
//S_IXOTH
//00001权限，代表其他用户拥有执行的权限
bool FileOperation::createFile(string filename) {
	// string path = m_strPath + '\\' + filename;
	// fstream file;
	// file.open(path, ios::out);
	// if (!file)
	// {
	//     return false;
	// }
	// file.close();

	return true;
}
bool FileOperation::isExistFile(string fileName) {
	if (0 == access(fileName.c_str(), 0))
		return true;
	else
		return false;
}
bool FileOperation::deleteFile(string fileName) {
	if (isExistFile(fileName)) {
		if (0 == remove(fileName.c_str()))
			return true;
		else
			return false;
	} else
		return false;
}

bool FileOperation::alterFileName(string filename, string newname) {
	// string path = m_strPath + '\\' + filename;
	// newname = m_strPath + '\\' + newname;
	// // int rename(char *oldname, char *newname);
	// // 更改文件名,成功返回0，否则返回-1
	// if (-1 == rename(path.c_str(), newname.c_str()))
	// {
	//     return false;
	// }

	return true;
}
bool FileOperation::extractTarFile(string fileName,
		map<INT32, string> &subItems) {
	cout << "extract 1 "<<endl;
	if (isExistFile(fileName) != true) {
		return false;
	}
	FILE *fstream = NULL;
	char buff[fileDownloadPathSize] = { 0 };
	memset(buff, 0, sizeof(buff));
	string exeCMD = "tar -zxvf ";
	exeCMD += fileName;
	exeCMD += " -C ";
	exeCMD += upFilePath;
	exeCMD += " 2>/dev/null";
	///////////chmod +x
	cout << "extract 1 "<<endl;
	if (NULL == (fstream = popen(exeCMD.c_str(), "r"))) {
		fprintf(stderr, "execute command failed: %s", strerror(errno));
		return false;
	}
	cout << "extract 1 "<<endl;
	char items[][32] = { 0 };
	int num = 1, ch = 0, bufflen = 0;
	while (NULL != fgets(buff, sizeof(buff), fstream)) {
		printf("buff: %s\n", buff);
		buff[strlen(buff) - 1] = '\0';
		subItems.insert(pair<INT32, string>(num, buff));
		num++;
	}
	map<INT32, string>::iterator iter;
	for (iter = subItems.begin(); iter != subItems.end(); iter++)
		cout << iter->first << ' ' << iter->second << endl;
	pclose(fstream);
	FileOperation::deleteFile(fileName);
	return true;
}

bool FileOperation::isExisteDirectory(string path) {
	if (-1 != access(path.c_str(), 0)) {
		return true;
	}
	return false;
}
