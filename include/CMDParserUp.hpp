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
#include "NetTrans.h"

template<typename T>
bool CMDParserUp::obtainParams(INT8 *buff, T &config, INT32 num) {
	INT8 *tmpBuff = buff;
	UINT32 iPos = 0;
	UINT32 nameLen = 0;
	UINT32 valueLen = 0;
	NetTrans::printBufferByHex("recv obtain : ", buff, 200);
	for (INT32 i = 0; i < num; i++) {
		nameLen = (UINT32) tmpBuff[iPos];
		iPos += 1;
		SmartPtr<INT8> name(new INT8[nameLen]);
		memcpy(name.get(), &tmpBuff[iPos], nameLen);
		iPos += nameLen;
		valueLen = (UINT32) tmpBuff[iPos];
		iPos += 1;
		SmartPtr<INT8> value(new INT8[valueLen]);
		FrameWork::Logger::GetInstance().Info("Each value length : %d",
				valueLen);
		memcpy(value.get(), &tmpBuff[iPos], valueLen);
		iPos += valueLen;
		if (screeningParams(name.get(), value.get(), config) != true) {
			FrameWork::Logger::GetInstance().Error(
					"Can not set parameter name : %s with its value : %s!",
					name.get(), value.get());
			return false;
		} else
			FrameWork::Logger::GetInstance().Info(
					"Set parameter name : %s with its value : %s!", name.get(),
					value.get());
	}
	return true;
}

#endif /* CMDPARSERUP_HPP_ */
