/*
 * FileOperation.h
 *
 *  Created on: 2018年4月16日
 *      Author: JHB
 */
#ifndef FILEOPERATION_H_
#define FILEOPERATION_H_

#include <string>
#include <map>
#include "GlobDefine.h"
using namespace std;

class FileOperation {
public:
	FileOperation();
	~FileOperation(void);

	bool createFile(string fileName);
	static bool isExistFile(string fileName);
	static bool isExisteDirectory(string path);
	static bool deleteFile(string fileName);

	bool alterFileName(string fileName, string newname);

	/* extrat */
	static bool extractTarFile(string fileName, map<INT32, string> &subItems);

protected:
	string m_strPath;
private:
};
#endif /* FILEOPERATION_H_ */
