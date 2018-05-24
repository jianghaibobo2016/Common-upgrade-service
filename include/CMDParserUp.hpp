/*
 * CMDParserUp.hpp
 *
 *  Created on: May 24, 2018
 *      Author: jhb
 */

#ifndef CMDPARSERUP_HPP_
#define CMDPARSERUP_HPP_

#include "CMDParserUp.h"
#include "Logger.h"

template<class T>
bool CMDParserUp::obtainParams(INT8 *buff, T &config, INT32 num) {
	INT8 *tmpBuff = buff;
	UINT32 iPos = 0;
	UINT32 nameLen = 0;
	UINT32 valueLen = 0;
	for (INT32 i = 0; i < num; i++) {
		nameLen = (UINT32) tmpBuff[iPos];
		iPos += 1;
		INT8 name[iPos] = { 0 };
		memcpy(name, &tmpBuff[iPos], nameLen);
		iPos += nameLen;
		valueLen = (UINT32) tmpBuff[iPos];
		iPos += 1;
		INT8 value[valueLen] = { 0 };
		memcpy(value, &tmpBuff[iPos], valueLen);
		iPos += valueLen;
		if (screeningParams(name, value, config) != true) {
			FrameWork::Logger::GetInstance().Error(
					"Can not set parameter name : %s with its value : %s!", name,
					value);
			return false;
		}
	}
	return true;
}

#endif /* CMDPARSERUP_HPP_ */
