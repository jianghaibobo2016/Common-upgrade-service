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

template<typename T>
INT32 HandleUp::devReplyHandle(INT8 *sendtoBuff, T &s_devReply,
		UINT32 reasonLen, const INT8 *failReason, INT32 result,
		SetNetworkTerminal *setNetworkTerminal) {
	cout << "TTTTTTTTTTTTTTTTTTTTTTtset 1 " << endl;
	s_devReply.header.HeadTag = PROTOCAL_PC_DEV_HEAD;
	cout << "TTTTTTTTTTTTTTTTTTTTTTtset 2 " << endl;
	INT8 mac[13] = { 0 };
	cout << "TTTTTTTTTTTTTTTTTTTTTTtset 2.1 " << endl;
	if (setNetworkTerminal == NULL){
		cout << "NULLLLLLLLLLLLLLLLLLLLLLL"<<endl;
	}
	cout << "macaddrStruct ::::"<<setNetworkTerminal->getNetConfStruct().macAddr<<endl;
//	cout << "macaddrStruct ::::"<<setNetworkTerminal->getNetConfStruct().macAddr<<endl;
	string strMac = SetNetworkTerminal::castMacToChar13(mac,
			setNetworkTerminal->getNetConfStruct().macAddr);
	cout << "strmac ::::"<<strMac<<endl;
	cout << "TTTTTTTTTTTTTTTTTTTTTTtset 2.2 " << endl;
	memcpy(mac, strMac.c_str(), strlen(strMac.c_str()));
//	strcpy(mac,
//			SetNetworkTerminal::castMacToChar13(mac,
//					setNetworkTerminal->getNetConfStruct().macAddr));
	cout << "TTTTTTTTTTTTTTTTTTTTTTtset 3 " << endl;
	mac[12] = '\0';
	strncpy(s_devReply.DevID, TerminalDevTypeID, strlen(TerminalDevTypeID));
	cout << "TTTTTTTTTTTTTTTTTTTTTTtset 4 " << endl;
	strcpy(s_devReply.DevID + strlen(TerminalDevTypeID), mac);
	cout << "TTTTTTTTTTTTTTTTTTTTTTtset 5 " << endl;
	if (result == retOk) {
		s_devReply.Result = 1;
	} else if (result == retError) {
		s_devReply.Result = 0;
	} else
		return retError;
	cout << "TTTTTTTTTTTTTTTTTTTTTTtset 6 " << endl;
//	UINT8 FailReasonLen = strlen(failReason);
	UINT8 FailReasonLen = '\0';
	cout << "TTTTTTTTTTTTTTTTTTTTTTtset 7 " << endl;
	memcpy(&FailReasonLen, &reasonLen, 1);
	cout << "TTTTTTTTTTTTTTTTTTTTTTtset 8 " << endl;
	cout << "text to send : " << failReason << endl;
	s_devReply.header.DataLen = sizeof(s_devReply.DevID)
			+ sizeof(s_devReply.Result) + sizeof(FailReasonLen) + reasonLen;
	memcpy(sendtoBuff, &s_devReply, sizeof(T));
	memcpy(sendtoBuff + sizeof(T), &FailReasonLen, sizeof(FailReasonLen));
	memcpy(sendtoBuff + sizeof(T) + sizeof(FailReasonLen), failReason,
			FailReasonLen);
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
