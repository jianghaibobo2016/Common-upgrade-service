/*
 * HandleUp.hpp
 *
 *  Created on: Mar 23, 2018
 *      Author: jhb
 */

#ifndef HANDLEUP_HPP_
#define HANDLEUP_HPP_
#include "HandleUp.h"
#include "PCTransProtocal.h"

/******************************************************************************
 * Description :
 Form buffer for sending with msg by different struct obj.
 * Return Value : On success, it return 0. On error, -1 is returned.
 * Author : JHB
 * Create Data : 04/01/2018
 * Revision History :
 *   04/01/2018  JHB    Created.
 *****************************************************************************/
template<typename T>
INT32 HandleUp::devReplyHandle(INT8 *sendtoBuff, T &s_devReply,
		UINT32 reasonLen, const INT8 *failReason, INT32 result,
		SetNetworkTerminal *setNetworkTerminal) {
	cout << "This lock of auto lock of func :  "<<__FUNCTION__<<"()"<<endl;
	AutoLock autoLock(&mutex);
	cout << "Text to send : " << failReason << endl;
	s_devReply.header.HeadTag = PROTOCAL_PC_DEV_HEAD;
	INT8 mac[13] = { 0 };
	string strMac = SetNetworkTerminal::castMacToChar13(mac,
			setNetworkTerminal->getNetConfStruct().macAddr);
	memcpy(mac, strMac.c_str(), strlen(strMac.c_str()));
	mac[12] = '\0';
	strncpy(s_devReply.DevID, TerminalDevTypeID, strlen(TerminalDevTypeID));
	strcpy(s_devReply.DevID + strlen(TerminalDevTypeID), mac);
	if (result == retOk) {
		s_devReply.Result = 1;
	} else if (result == retError) {
		s_devReply.Result = 0;
	} else {
		cout << "This devReply unlock "<<endl;
		return retError;
	}
	UINT8 FailReasonLen = '\0';
	memcpy(&FailReasonLen, &reasonLen, 1);
	s_devReply.header.DataLen = sizeof(s_devReply.DevID)
			+ sizeof(s_devReply.Result) + sizeof(FailReasonLen) + reasonLen;
	memcpy(sendtoBuff, &s_devReply, sizeof(T));
	memcpy(sendtoBuff + sizeof(T), &FailReasonLen, sizeof(FailReasonLen));
	memcpy(sendtoBuff + sizeof(T) + sizeof(FailReasonLen), failReason,
			reasonLen);
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
template<typename T>
INT32 HandleUp::localUpHandle(T &upControl) {
	upControl.header.HeadTag = PROTOCAL_PC_DEV_HEAD;
	upControl.header.DataLen = sizeof(T);
	return retOk;
}

#endif /* HANDLEUP_HPP_ */
