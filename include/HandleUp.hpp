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
	cout << "text to send : " << failReason << endl;
	s_devReply.header.DataLen = sizeof(s_devReply.DevID)
			+ sizeof(s_devReply.Result) + sizeof(FailReasonLen)
			+ strlen(failReason);
	memcpy(sendtoBuff, &s_devReply, sizeof(T));
	memcpy(sendtoBuff + sizeof(T), &FailReasonLen,
			sizeof(FailReasonLen));
	memcpy(
			sendtoBuff + sizeof(T)
					+ sizeof(FailReasonLen), failReason, FailReasonLen);
	return retOk;
}

template<typename T>
INT32 HandleUp::writeFileFromPC(INT8 *recvBuff, const INT8 *fileName) {
	T *PCReply = (T*) recvBuff;
	ofstream fout(fileName, ios::app | ios::binary); //加入文件名
	fout.seekp(PCReply->StartPosition, ios::beg);
	fout.write(recvBuff + sizeof(T), PCReply->FileDataLen);
	if (fout.good() == false) {
		fout.close();
		return retError;
	}
	fout.close();
	return retOk;
}

#endif /* HANDLEUP_HPP_ */
