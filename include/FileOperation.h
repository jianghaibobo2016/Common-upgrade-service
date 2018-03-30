/*
 * FileOperation.h
 *
 *  Created on: Mar 25, 2018
 *      Author: jhb
 */

#ifndef FILEOPERATION_H_
#define FILEOPERATION_H_

#include <string>
#include <map>
#include "GlobDefine.h"
using namespace std;

class FileOperation {
public:
	// 构造函数，dir为文件夹名称：标注、书签、试题、模型及动画、media、界面等
	FileOperation();
	~FileOperation(void);

	bool createFile(string fileName);
	static bool isExistFile(string fileName);
	static bool deleteFile(string fileName);

	bool alterFileName(string fileName, string newname);

	/* extrat */
	static bool extractTarFile(string fileName, map<INT32, string> &subItems);

protected:
	bool isExisteDirectory(string path);
	string m_strPath;
private:
	// 工作目录
};
#endif /* FILEOPERATION_H_ */
