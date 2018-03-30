/*
 * HandleUp.hpp
 *
 *  Created on: Mar 23, 2018
 *      Author: jhb
 */

#ifndef HANDLEUP_HPP_
#define HANDLEUP_HPP_
#include "HandleUp.h"
template<typename T>
INT32 HandleUp::devReplyHandle(INT8 *sendtoBuff, T &s_devReply,
		INT8 *failReason, INT32 result,
		SetNetworkTerminal *setNetworkTerminal) {
	s_devReply.header.HeadTag = 0x0101FBFC;
	INT8 mac[13] = { 0 };
	strcpy(mac,
			SetNetworkTerminal::castMacToChar13(mac,
					setNetworkTerminal->m_netWorkConfig.macAddr));
	mac[12] = '\0';
	strncpy(s_devReply.DevID, TerminalDevTypeID, strlen(TerminalDevTypeID));
	strcpy(s_devReply.DevID + strlen(TerminalDevTypeID), mac);
	if (result == retOk) {
		s_devReply.Result = 1;
	} else if (result == retError) {
		s_devReply.Result = 0;
	} else
		return retError;
	UINT8 FailReasonLen = strlen(failReason);
	s_devReply.header.DataLen = sizeof(s_devReply.DevID)
			+ sizeof(s_devReply.Result) + sizeof(FailReasonLen)
			+ strlen(failReason);
	memcpy(sendtoBuff, &s_devReply, sizeof(DEV_Reply_ParameterSetting));
	memcpy(sendtoBuff + sizeof(DEV_Reply_ParameterSetting), &FailReasonLen,
			sizeof(FailReasonLen));
	memcpy(
			sendtoBuff + sizeof(DEV_Reply_ParameterSetting)
					+ sizeof(FailReasonLen), failReason, strlen(failReason));
	return retOk;
}

template<typename T>
INT32 HandleUp::writeFileFromPC(INT8 *recvBuff, const INT8 *fileName) {
	T *PCReply = (T*) recvBuff;
//	cout << "filename " << fileName << endl;
#if 1
	/////////////app
	ofstream fout(fileName, ios::app | ios::binary); //加入文件名
//	cout << "write pos : " << PCReply->StartPosition << endl;
	fout.seekp(PCReply->StartPosition, ios::beg);
//	cout << "file pos:: " << fout.tellp();
	fout.write(recvBuff + sizeof(T), PCReply->FileDataLen);
//	cout << "file pos:: " << fout.tellp();
//	cout << "write len : " << PCReply->FileDataLen << endl;
	if (fout.good() == false) {
		fout.close();
//		cout << "write 5 !" << endl;
		return retError;
	}
//	cout << "write 6 !" << endl;
	fout.close();
#endif
//	FILE *fd = fopen(fileName, "wb+");
//	fseek(fd, PCReply->StartPosition, SEEK_SET);
//	fwrite(recvBuff + sizeof(T), 1, PCReply->FileDataLen, fd);
//	fclose(fd);
	return retOk;
}

#endif /* HANDLEUP_HPP_ */
