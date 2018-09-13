/*
 * HandleUp.cpp
 *
 *  Created on: Mar 23, 2018
 *      Author: jhb
 */
#include "HandleUp.h"
#include "UpgradeServiceConfig.h"
#include "CMDParserUp.h"
#include "Upgrade.h"
#include "FileOperation.h"
#include "LocalUDPTrans.h"
#include "Logger.h"
#include <unistd.h>
#include <errno.h>
#include <sys/reboot.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "CrcCheck.h"

#include <iomanip> //hex dec cout

#include <sys/time.h>
#include "Timer.h"

using namespace FrameWork;

HandleUp::HandleUp() :
		_udpNet(NULL), devModuleToUpgrade() {
}

//HandleUp::HandleUp(UDPNetTrans &udpNet) :
//		_udpNet(udpNet), devModuleToUpgrade() {
//}

HandleUp::~HandleUp() {
}

HandleUp::HandleUp(const HandleUp &handle) :
		_udpNet(handle._udpNet) {
//	_udpNet = handle._udpNet;
	inUpgrade = handle.inUpgrade;
	devModuleToUpgrade = handle.devModuleToUpgrade;
	memset(&_fileTransStartTime, 0, sizeof(struct timeval));
}
//
//HandleUp &HandleUp::operator=(const HandleUp &handle) {
//	if (this != &handle) {
//		_udpNet(handle._udpNet);
//		inUpgrade = handle.inUpgrade;
//		devModuleToUpgrade = handle.devModuleToUpgrade;
//	}
//	return *this;
//}

Mutex HandleUp::mutex;
bool HandleUp::inUpgrade = false;
bool HandleUp::inFileTrans = false;
struct timeval HandleUp::start;

//HandleUp& HandleUp::getInstance() {
//	static HandleUp handle;
//	return handle;*/
//}

void myprint(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	cout << tv.tv_sec << endl;
//    return NULL;
}

/******************************************************************************
 * Description :
 Get device's info and send. Send an extra msg while dev is upgrading.
 * Return Value : void.
 * Author : JHB
 * Create Data : 04/01/2018
 * Revision History :
 *   04/01/2018  JHB    Created.
 *****************************************************************************/
void HandleUp::devSearchCMDHandle(sockaddr_in recvAddr,
		SetNetworkTerminal *setNetworkTerminal, UpFileAttrs &upFileAttr,
		INT32 sockfd) {
	//If file trans was broke by closing PC software more than 6 seconds,
	//upgrading status is turned to false;
	{
		if (getInUpgrade() == true && getInFileTrans() == true) {
			struct timeval end;
			gettimeofday(&end, NULL);
			double diffTime = diffTimeval(&end, &start);
			if (diffTime > 6000000) {
				setInUpgrade(false);
				Logger::GetInstance().Info(
						"The interval of file trans more than 6s !");
			}
		}
	}

	INT32 retSend = 0;
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	DevSearchTerminal devSearchTerminal(setNetworkTerminal);
	SmartPtr<DEV_Reply_GetDevMsg> tmpReMsg(new DEV_Reply_GetDevMsg);
	devSearchHandle(*tmpReMsg.get(), &devSearchTerminal);
	cout << "HeadTag  " << setw(20) << hex
			<< devSearchTerminal.devReplyMsg->header.HeadTag << endl;
	cout << "HeadCmd  " << setw(20) << tmpReMsg->header.HeadCmd << endl;
	cout << "DataLen  " << dec << setw(20) << tmpReMsg->header.DataLen << endl;
	struct sockaddr_in sin;
	sin.sin_addr.s_addr = tmpReMsg->DevIP;
	cout << "DevIP  " << setw(22) << inet_ntoa(sin.sin_addr) << endl;
	sin.sin_addr.s_addr = tmpReMsg->DevMask;
	cout << "DevMask  " << setw(20) << inet_ntoa(sin.sin_addr) << endl;
	sin.sin_addr.s_addr = tmpReMsg->DevGateway;
	cout << "DevGateway  " << setw(17) << inet_ntoa(sin.sin_addr) << endl;
	sin.sin_addr.s_addr = tmpReMsg->DevServerIP;
	cout << "DevServerIP  " << setw(16) << inet_ntoa(sin.sin_addr) << endl;
	cout << "DevServerPort  " << setw(14) << tmpReMsg->CommunicationPort
			<< endl;
	cout << "DevMACAddress  " << setw(14) << tmpReMsg->DevMACAddress << endl;
	cout << "DevID  " << setw(22) << tmpReMsg->DevID << endl;
	cout << "DevType  " << setw(20) << tmpReMsg->DevType << endl;
	cout << "HardVersion  " << setw(16) << tmpReMsg->HardVersion << endl;
	cout << "SoftVersion  " << setw(16) << tmpReMsg->SoftVersion << endl;
	cout << "DevName  " << setw(24) << tmpReMsg->DevName << endl;
#if (DSP9903)
	cout << "Recording port : " << tmpReMsg->RecordingPort << endl;
#endif
	cout << "mask " << setw(18) << tmpReMsg->Mask[0] << " " << tmpReMsg->Mask[2]
			<< endl;

	struct sockaddr_in sendAddr;
	memset(&sendAddr, 0, sizeof(struct sockaddr_in));
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_addr.s_addr = inet_addr(MultiCastAddrSEND); //任何主机地址
	sendAddr.sin_port = htons(UpUDPTransPort);

	sendto(sockfd, tmpReMsg.get(), sizeof(DEV_Reply_GetDevMsg), 0,
			(struct sockaddr *) &recvAddr, tmp_server_addr_len);
	sendto(sockfd, tmpReMsg.get(), sizeof(DEV_Reply_GetDevMsg), 0,
			(struct sockaddr *) &sendAddr, tmp_server_addr_len);
//	getUDPNetTransInstance().UDPSendMsg(tmpReMsg.get(),
//			sizeof(DEV_Reply_GetDevMsg), recvAddr);
	printf("The client IP connected is : %s\n", inet_ntoa(recvAddr.sin_addr));

	if (getInUpgrade() == true) {
		inUpgradingMsgSend(recvAddr, setNetworkTerminal, sockfd);
		return;
	}

}

/******************************************************************************
 * Description :
 Parser CMD of params setting and send msg.
 * Return Value : On success, it return 0. On error, -1 is returned.
 * Author : JHB
 * Create Data : 04/01/2018
 * Revision History :
 *   04/01/2018  JHB    Created.
 *****************************************************************************/
INT32 HandleUp::setNetworkHandle(INT8 *recvBuff,
		DEV_Reply_ParameterSetting &devReplySetPara,
		SetNetworkTerminal *setNetworkTerminal, INT32 sockfd,
		sockaddr_in recvAddr) {
	INT32 sockfd_tmp = sockfd;
	INT8 sendtoBuff[SendBufferSizeMax] = { 0 };
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	INT8 failReason[128] = { 0 };
	map<string, string> retContent;
	INT32 retSetNet = retOk;
	retSetNet = CMDParserUp::parserPCSetNetCMD(recvBuff, setNetworkTerminal,
			retContent);
	Logger::GetInstance().Info("parserPCSetNetCMD() return : %d", retSetNet);
//	if (retSetNet == INVALID)
//		return INVALID;
//	else if (retSetNet == SAMECONF)
//		return retOk;

	devReplySetPara.header.HeadCmd = 0x0002;
#if 0
	map<string, string>::iterator iter;
	for (iter = retContent.begin(); iter != retContent.end(); iter++) {
		memset(failReason, 0, 128);
		UINT8 NameLen = strlen(iter->first.c_str());
		memcpy(failReason, &NameLen, 1);
		memcpy(failReason + 1, iter->first.c_str(),
				strlen(iter->first.c_str()));
		failReason[strlen(iter->first.c_str()) + 1] = '\0';
		NameLen = strlen(iter->second.c_str());
		memcpy(failReason + 2 + strlen(iter->first.c_str()), &NameLen, 1);
		memcpy(failReason + 3 + strlen(iter->first.c_str()),
				iter->second.c_str(), strlen(iter->second.c_str()));
		failReason[strlen(iter->first.c_str()) + 3
		+ strlen(iter->second.c_str())] = '\0';
		UINT32 reasonLen = strlen(iter->first.c_str()) + 4
		+ strlen(iter->second.c_str());
		AutoLock autoLock(&mutex);
		cout << "Text to send : " << failReason << endl;
		devReplySetPara.header.HeadTag = PROTOCAL_PC_DEV_HEAD;
		INT8 mac[13] = {0};
		string strMac = SetNetworkTerminal::castMacToChar13(mac,
				setNetworkTerminal->getNetConfStruct().macAddr);
		memcpy(mac, strMac.c_str(), strlen(strMac.c_str()));
		mac[12] = '\0';
		strncpy(devReplySetPara.DevID, TerminalDevTypeID,
				strlen(TerminalDevTypeID));
		strcpy(devReplySetPara.DevID + strlen(TerminalDevTypeID), mac);
		if (retSetNet == retOk) {
			devReplySetPara.Result = 1;
		} else if (retSetNet == retError) {
			devReplySetPara.Result = 0;
		} else {
			cout << "This devReply unlock " << endl;
			return retError;
		}
		devReplySetPara.header.DataLen = sizeof(devReplySetPara.DevID)
		+ sizeof(devReplySetPara.Result) + reasonLen;
		memcpy(sendtoBuff, &devReplySetPara,
				sizeof(DEV_Reply_ParameterSetting));
		memcpy(sendtoBuff + sizeof(DEV_Reply_ParameterSetting), failReason,
				reasonLen);
	}
#endif
	if (retSetNet == retOk) {
		memcpy(failReason, MODIFYRESPOND_SUCCESS,
				strlen(MODIFYRESPOND_SUCCESS));
	} else if (retSetNet == retError) {
		memcpy(failReason, MODIFYRESPOND_FAIL, strlen(MODIFYRESPOND_FAIL));
	} else if (retSetNet == INVALID)
		return retOk;
	else if (retSetNet == SAMECONF) {
		memcpy(failReason, MODIFYRESPOND_SAMECONF,
				strlen(MODIFYRESPOND_SAMECONF));
	} else if (retSetNet == DEVINITSETRET) {
		memcpy(failReason, MODIFYRESPOND_SUCCESS,
				strlen(MODIFYRESPOND_SUCCESS));
	} else
		memcpy(failReason, MODIFYRESPOND_FAIL, strlen(MODIFYRESPOND_FAIL));
	if (devReplyHandle<DEV_Reply_ParameterSetting>(sendtoBuff, devReplySetPara,
			strlen(failReason), failReason, retSetNet, setNetworkTerminal)
			== retOk) {
	}
//	getUDPNetTransInstance().UDPSendMsg(sendtoBuff,
//			sizeof(PC_DEV_Header) + devReplySetPara.header.DataLen, recvAddr);
	struct sockaddr_in sendAddr;
	memset(&sendAddr, 0, sizeof(struct sockaddr_in));
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_addr.s_addr = inet_addr(MultiCastAddrSEND); //任何主机地址
	sendAddr.sin_port = htons(UpUDPTransPort);
	if (retSetNet != DEVINITSETRET)
		sendto(sockfd_tmp, sendtoBuff,
				sizeof(PC_DEV_Header) + devReplySetPara.header.DataLen, 0,
				(struct sockaddr *) &sendAddr, tmp_server_addr_len);

	if (retSetNet == retOk) {
		Logger::GetInstance().Info(
				"===System will reboot after 2 secs for configing network.===");
		sysReboot();
	} else if (retSetNet == SAMECONF) {
//		Logger::GetInstance().Info("Set parameters failed !");
	} else if (retSetNet == DEVINITSETRET) {
		INT32 socktmp = socket(AF_INET, SOCK_DGRAM, 0);
		sleep(1);
		sendto(socktmp, sendtoBuff,
				sizeof(PC_DEV_Header) + devReplySetPara.header.DataLen, 0,
				(struct sockaddr *) &sendAddr, tmp_server_addr_len);
		Logger::GetInstance().Info(
				"===System will reboot after 2 secs for configing network.===");
		sysReboot();
		close(socktmp);
	} else {
		Logger::GetInstance().Info("Set parameters failed !");
	}
	return retOk;
}

/******************************************************************************
 * Description :
 Going to set network after getting CMD.
 * Return Value : void.
 * Author : JHB
 * Create Data : 04/01/2018
 * Revision History :
 *   04/01/2018  JHB    Created.
 *****************************************************************************/
void HandleUp::devParamSetCMDHandle(sockaddr_in recvAddr, INT8 *recvBuff,
		SetNetworkTerminal *setNetworkTerminal, INT32 sockfd) {
	SmartPtr<DEV_Reply_ParameterSetting> devReplyParaSetting(
			new DEV_Reply_ParameterSetting);
	memset(devReplyParaSetting.get(), 0, sizeof(DEV_Reply_ParameterSetting));
	devReplyParaSetting->header.HeadCmd = 0x0002;
	if (getInUpgrade() == true) {
		inUpgradingMsgSend(recvAddr, setNetworkTerminal, sockfd);
		INT8 sendtoBuff[SendBufferSizeMax] = { 0 };
		INT8 failReason[128] = { 0 };
		memcpy(failReason, INUPGRADING, strlen(INUPGRADING));
		INT32 retSetNet = retError;
		if (devReplyHandle<DEV_Reply_ParameterSetting>(sendtoBuff,
				*devReplyParaSetting.get(), strlen(failReason), failReason,
				retSetNet, setNetworkTerminal) == retOk) {
		}
		struct sockaddr_in sendAddr;
		memset(&sendAddr, 0, sizeof(struct sockaddr_in));
		sendAddr.sin_family = AF_INET;
		sendAddr.sin_addr.s_addr = inet_addr(MultiCastAddrSEND); //任何主机地址
		sendAddr.sin_port = htons(UpUDPTransPort);
		INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);

		sendto(sockfd, sendtoBuff,
				sizeof(PC_DEV_Header) + devReplyParaSetting->header.DataLen, 0,
				(struct sockaddr *) &sendAddr, tmp_server_addr_len);
		return;
	}

	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	setNetworkHandle(recvBuff, *devReplyParaSetting.get(), setNetworkTerminal,
			sockfd, recvAddr);
}

/******************************************************************************
 * Description :
 Upgrade CMD will handled by this function. It sends a msg for ready upgrading
 or not to client, then goes to a thread to upgrade by web upgrade method or
 return for transfering file.
 * Return Value : void.
 * Author : JHB
 * Create Data : 04/01/2018
 * Revision History :
 *   04/01/2018  JHB    Created.
 *****************************************************************************/
void HandleUp::devUpgradePCRequestCMDHandle(sockaddr_in &recvAddr,
		INT8 *recvBuff, SetNetworkTerminal *setNetworkTerminal, INT32 &sockfd,
		UpFileAttrs &upFileAttr, FileTrans &fileTrans,
		DEV_Request_FileProtocal *request) {
	if (getInUpgrade() == true) {
		Logger::GetInstance().Info("INUPGRADING...");
		inUpgradingMsgSend(recvAddr, setNetworkTerminal, sockfd);
		return;
	}
	INT8 sendtoBuffer[SendBufferSizeMax] = { 0 };
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	SmartPtr<DEV_Reply_DevUpgrade> devReply(new DEV_Reply_DevUpgrade);
	/*set inUpgrading*/

	INT32 retHandle = upgradePCrequestHandle(recvBuff, sendtoBuffer,
			*devReply.get(), upFileAttr, setNetworkTerminal);
	NetTrans::printBufferByHex("sendtoBuffer after upgradePCrequestHandle() : ",
			sendtoBuffer, sizeof(PC_DEV_Header) + devReply->header.DataLen);

	INT32 retSendto = sendto(sockfd, sendtoBuffer,
			sizeof(PC_DEV_Header) + devReply->header.DataLen, 0,
			(struct sockaddr *) &recvAddr, tmp_server_addr_len);
	if (retHandle != retOk) {
		Logger::GetInstance().Info(
				"Can not send msg of upgrade request repond ");
		return;
	}

	if (upFileAttr.getWebUpMethod() == true) {

		SmartPtr<FileTransArgs> transArgs(new FileTransArgs);
		memcpy(&transArgs->recvAddr, &recvAddr, sizeof(sockaddr_in));
		transArgs->setNetworkTerminal = setNetworkTerminal;
		transArgs->sockfd = &sockfd;
		transArgs->upFileAttr = &upFileAttr;
		transArgs->fileTrans = &fileTrans;
		transArgs->handle = this;
//		transArgs->udpNet = _udpNet;
		pthread_t tid;
		INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
		if (pthread_create(&tid, NULL, UpgradeThreadFun,
				(void *) transArgs.get()) == 0) {
			Logger::GetInstance().Info(
					"Create upgrade thread successfully!........");
		} else {
			setInUpgrade(false);
		}

		sleep(1);
		return;
	}
#if 0
	if (!upFileAttrs->getInUpgradeStatus()) {
		cout << "upstatus 1 !"<<endl;
		fileTrans.clearFileTrans();
		fileTrans.iniPosition(upFileAttrs->getNewSoftFileSize());
		upFileAttrs->setInUpgradeStatus(true);
		cout << "first request sendlen : " << fileTrans.getSendLen()
		<< endl;
	} else {
		cout << "upstatus 2 !"<<endl;
		upFileAttrs->setFileTransRecord(fileTrans);
	}
#else
	fileTrans.clearFileTrans();
	fileTrans.iniPosition(upFileAttr.getNewSoftFileSize());
#endif
	memset(request, 0, sizeof(DEV_Request_FileProtocal));
	if (devRequestFileInit(*request, upFileAttr, fileTrans) == retOk) {
		NetTrans::printBufferByHex("request after devRequestFileInit() : ",
				request, sizeof(PC_DEV_Header) + request->header.DataLen);
//		sleep(1);
//		getUDPNetTransInstance().UDPSendMsg(request,
//				sizeof(PC_DEV_Header) + request->header.DataLen, recvAddr);
		INT32 retRequest = sendto(sockfd, (INT8*) request,
				sizeof(PC_DEV_Header) + request->header.DataLen, 0,
				(struct sockaddr *) &recvAddr, tmp_server_addr_len);
		Logger::GetInstance().Info("Dev Request file trans sendto PC len: %d !",
				retRequest);
		setInFileTrans(true);
		memset(&_fileTransStartTime, 0, sizeof(struct timeval));
		gettimeofday(&_fileTransStartTime, NULL);
		Logger::GetInstance().Info("File trans starting time : %u",
				_fileTransStartTime.tv_sec);
	} /*end send if*/
	ofstream f(upFileAttr.getFileDownloadPath(), ios::trunc);
	f.close();
}

void HandleUp::devFileTransCMDHandle(sockaddr_in &recvAddr, INT8 *recvBuff,
		SetNetworkTerminal *setNetworkTerminal, INT32 &sockfd,
		UpFileAttrs &upFileAttr, FileTrans &fileTrans,
		DEV_Request_FileProtocal *request) {

	setInFileTrans(true);
	memset(&start, 0, sizeof(struct timeval));
	gettimeofday(&start, NULL);

	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	SmartPtr<DEV_Request_UpgradeReply> upgradeReply(
			new DEV_Request_UpgradeReply);
	INT8 replyText[msgLen] = { 0 };
	INT8 sendtoBuffer[SendBufferSizeMax] = { 0 };
	INT32 retUpStatus = retOk;
	INT32 retWrite = writeFileFromPC<PC_Reply_FileProtocal>(recvBuff,
			upFileAttr.getFileDownloadPath());
	if (retWrite == retError) {
		setInUpgrade(false);
		return;
	}

	fileTrans.changeRemainedPos().setPersentage();
	if (devRequestFile(*request, fileTrans) == retOk) {
		sendto(sockfd, (INT8*) request,
				sizeof(PC_DEV_Header) + request->header.DataLen, 0,
				(struct sockaddr *) &recvAddr, tmp_server_addr_len);
		memset(&_fileTransStartTime, 0, sizeof(struct timeval));
		gettimeofday(&_fileTransStartTime, NULL);
	}

	if (fileTrans.getNewPercent() > fileTrans.getOldPercent()) {
		upgradeReply->header.HeadCmd = 0x0005;
		memset(replyText, 0, msgLen);
		sprintf(replyText, "Upgrading %u%%", fileTrans.getNewPercent());
		replyText[strlen(replyText)] = '\0';

		Logger::GetInstance().Info("---------replyText : %s---------",
				replyText);

		if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
				*upgradeReply.get(), strlen(replyText), replyText, retUpStatus,
				setNetworkTerminal) == retOk) {
		}
		sendto(sockfd, (INT8*) sendtoBuffer,
				sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
				(struct sockaddr *) &recvAddr, tmp_server_addr_len);
	}
	fileTrans.setOldPercent(fileTrans.getNewPercent());
	if (0 == fileTrans.getFileRemainedLen()) {
		setInFileTrans(false);
		UINT8 md5_str[MD5_SIZE];
		memset(replyText, 0, msgLen);
		if (!GetFileMD5(upFileAttr.getFileDownloadPath(), md5_str)) {
			Logger::GetInstance().Fatal("MD5 checked failed !");
			sprintf(replyText, "File error, please try again !");
			retUpStatus = retError;
		}
		int iret = memcmp(upFileAttr.getFileMD5Code(), md5_str,
		MD5_SIZE);
		memset(sendtoBuffer, 0, SendBufferSizeMax);
		if (iret != 0) {
			retUpStatus = retError;
			Logger::GetInstance().Fatal("MD5 checked error !");
			sprintf(replyText, "File error, please try again !");
		} else {
			retUpStatus = retOk;
			sprintf(replyText, "Upgrading 55%%.");
			replyText[strlen(replyText)] = '\0';
			Logger::GetInstance().Info("File MD5 check finished !");
		}
		Logger::GetInstance().Info("---------replyText : %s---------",
				replyText);
		if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
				*upgradeReply.get(), strlen(replyText), replyText, retUpStatus,
				setNetworkTerminal) == retOk) {
		}
//		getUDPNetTransInstance().UDPSendMsg(sendtoBuffer,
//				sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, recvAddr);
		sendto(sockfd, (INT8*) sendtoBuffer,
				sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
				(struct sockaddr *) &recvAddr, tmp_server_addr_len);
		if (retUpStatus == retError) {
			setInUpgrade(false);
			return;
		}
		/*next step*/
		SmartPtr<FileTransArgs> transArgs(new FileTransArgs);
		memcpy(&transArgs->recvAddr, &recvAddr, sizeof(sockaddr_in));
		transArgs->setNetworkTerminal = setNetworkTerminal;
		transArgs->sockfd = &sockfd;
		transArgs->upFileAttr = &upFileAttr;
		transArgs->fileTrans = &fileTrans;
		transArgs->handle = this;
		pthread_t tid;
		if (pthread_create(&tid, NULL, UpgradeThreadFun,
				(void *) transArgs.get()) == 0) {
			Logger::GetInstance().Info(
					"Create upgrade handle thread successfully!");
		} else {
			setInUpgrade(false);
		}
		sleep(1);
	}
	return;
}

void HandleUp::devGetMaskCMDHandle(sockaddr_in &recvAddr,
		SetNetworkTerminal *setNetworkTerminal, INT32 &sockfd) {
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	SmartPtr<DEV_Request_MaskInfo> maskInfo(new DEV_Request_MaskInfo);
	memset(maskInfo.get(), 0, sizeof(DEV_Request_MaskInfo));
	maskInfo->header.HeadTag = PROTOCAL_PC_DEV_HEAD;
	maskInfo->header.HeadCmd = CMD_DEV_GETMASK;
	maskInfo->header.DataLen = sizeof(maskInfo->DevID)
			+ sizeof(maskInfo->m_mask);
	INT8 mac[13] = { 0 };
	strcpy(mac,
			setNetworkTerminal->castMacToChar13(mac,
					setNetworkTerminal->getNetConfStruct().macAddr));
	mac[12] = '\0';
	strncpy(maskInfo->DevID, TerminalDevTypeID, strlen(TerminalDevTypeID));
	strcpy(maskInfo->DevID + strlen(TerminalDevTypeID), mac);
	INT32 retGet = getMaskInfo(maskInfo->m_mask);
	if (retGet == retOk) {
		struct sockaddr_in sendAddr;
		memset(&sendAddr, 0, sizeof(struct sockaddr_in));
		sendAddr.sin_family = AF_INET;
		sendAddr.sin_addr.s_addr = inet_addr(MultiCastAddrSEND); //任何主机地址
		sendAddr.sin_port = htons(UpUDPTransPort);
		INT32 sendtoret = sendto(sockfd, (INT8*) maskInfo.get(),
				sizeof(DEV_Request_MaskInfo), 0, (struct sockaddr *) &sendAddr,
				tmp_server_addr_len);
		if (sendtoret <= 0) {
			Logger::GetInstance().Info("Send mask error !");
			return;
		} else {
			NetTrans::printBufferByHex("Send to PC mask info : ",
					maskInfo.get(), sizeof(DEV_Request_MaskInfo));
			return;
		}
	} else if (retGet == retError) {
		Logger::GetInstance().Error("Can not get mask !");
	}
}
void HandleUp::devTestModeCntCMDHandle(INT8 *recvBuff, sockaddr_in &recvAddr,
		SetNetworkTerminal *setNetworkTerminal, INT32 &sockfd) {
	if (getInUpgrade() == true) {
		inUpgradingMsgSend(recvAddr, setNetworkTerminal, sockfd);
		return;
	}
	PC_Run_TestMode *devTestMode = (PC_Run_TestMode*) recvBuff;
	if (devTestMode->Control == 0 || devTestMode->Control == 1) {
		UPDATE_SEND_TESTMODE sendTestMode;
		sendTestMode.header.HeadCmd = CMD_LOCALDEV_TESTMODE;
		memcpy(&sendTestMode.Control, &devTestMode->Control, 1);
//		sendTestMode.Control = devTestMode->Control;
		localUpHandle<UPDATE_SEND_TESTMODE>(sendTestMode);
		LocalUDPTrans netTrans;
		INT32 retSend = sendto(netTrans.getSockfd(), &sendTestMode,
				sizeof(UPDATE_SEND_TESTMODE), 0,
				(struct sockaddr*) netTrans.getAddr(), *netTrans.getAddrLen());
		if (retSend <= 0) {
			Logger::GetInstance().Info("Can not change test mode to %d !",
					devTestMode->Control);
			cout << "errorsend" << endl;
			return;
		} else {
			Logger::GetInstance().Info("Change test mode to %d !",
					sendTestMode.Control);
			cout << "retsend : " << retSend << "struct size : "
					<< sizeof(UPDATE_SEND_TESTMODE) << endl;
		}
	}
}

void HandleUp::devGetVersionCMDHandle(INT32 &sockfd, sockaddr_in recvAddr) {
	SmartPtr<DEV_VersionNum> devVersion(new DEV_VersionNum);
	devVersion->header.HeadTag = PROTOCAL_PC_DEV_HEAD;
	devVersion->header.HeadCmd = 0x0009;
	devVersion->header.DataLen = sizeof(devVersion->version);
	DevSearchTerminal::getSoftwareVersion("product_version",
			devVersion->version, pathVersionFile);
	cout << "Get Software version : " << devVersion->version << endl;
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	INT32 retSend = sendto(sockfd, (INT8 *) devVersion.get(),
			sizeof(DEV_VersionNum), 0, (struct sockaddr *) &recvAddr,
			tmp_server_addr_len);
	cout << "version send to " << retSend << endl;
}

void HandleUp::devGetCastModeCMDHandle(sockaddr_in &recvAddr,
		SetNetworkTerminal *setNetworkTerminal, INT32 &sockfd) {
	if (getInUpgrade() == true) {
		inUpgradingMsgSend(recvAddr, setNetworkTerminal, sockfd);
		return;
	}
	INT8 mac[13] = { 0 };
	strcpy(mac,
			setNetworkTerminal->castMacToChar13(mac,
					setNetworkTerminal->getNetConfStruct().macAddr));
	mac[12] = '\0';
	SmartPtr<DEV_CastMode> devCastMode(new DEV_CastMode);
	devCastMode->header.HeadTag = PROTOCAL_PC_DEV_HEAD;
	devCastMode->header.HeadCmd = 0x000a;
	devCastMode->header.DataLen = 0x29;
	strncpy(devCastMode->DevID, TerminalDevTypeID, strlen(TerminalDevTypeID));
	strcpy(devCastMode->DevID + strlen(TerminalDevTypeID), mac);

	XMLParser xmlParser(pathXml);
	xmlParser.xmlInit();
	INT32 icastMode = xmlParser.getInt("TerminalNet", "IsUseMultiCast", 0);
	cout << "icastMode : " << icastMode << endl; ///////////////////0 1 is reality
	memcpy(&devCastMode->castMode, &icastMode, 1);
	struct sockaddr_in sendAddr;
	memset(&sendAddr, 0, sizeof(struct sockaddr_in));
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_addr.s_addr = inet_addr(MultiCastAddrSEND); //任何主机地址
	sendAddr.sin_port = htons(UpUDPTransPort);
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	INT32 sendtoret = sendto(sockfd, (INT8*) devCastMode.get(),
			sizeof(DEV_Request_MaskInfo), 0, (struct sockaddr *) &sendAddr,
			tmp_server_addr_len);
	if (sendtoret <= 0) {
		Logger::GetInstance().Info("Send CastMode error !");
		return;
	} else {
		NetTrans::printBufferByHex("Send to PC CastMode info : ",
				devCastMode.get(), sizeof(DEV_CastMode));
		return;
	}
}

void *HandleUp::UpgradeThreadFun(void *args) {
	FileTransArgs *upgradeArgs = (FileTransArgs *) args;
	sockaddr_in sendaddr;
	memset(&sendaddr, 0, sizeof(sockaddr_in));
	memcpy(&sendaddr, &upgradeArgs->recvAddr, sizeof(sockaddr_in));
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);

//
	SmartPtr<SetNetworkTerminal> setNet(
			new SetNetworkTerminal(*upgradeArgs->setNetworkTerminal));
	INT32 sockfd = *upgradeArgs->sockfd;
	SmartPtr<UpFileAttrs> fileAttrs(new UpFileAttrs(*upgradeArgs->upFileAttr));
	SmartPtr<FileTrans> fileTrans(new FileTrans(*upgradeArgs->fileTrans));

	SmartPtr<DEV_Request_UpgradeReply> upgradeReply(
			new DEV_Request_UpgradeReply);
	INT8 replyText[msgLen] = { 0 };
	INT8 sendtoBuffer[SendBufferSizeMax] = { 0 };
	INT32 retUpStatus = retOk;

	upgradeReply->header.HeadCmd = 0x0005;
	SmartPtr<UpgradeDSP> upDSPProduct(
			new UpgradeDSP(
					const_cast<INT8*>(fileAttrs->getFileDownloadPath())));

	if (fileAttrs->getForceStatus() == true)
		upDSPProduct->setForceUpgrade(true);
	if (upDSPProduct->parserFileName() == retOk) {
		Logger::GetInstance().Info("New version : %s",
				upDSPProduct->getNewVersion());
	} else {
		memset(sendtoBuffer, 0, SendBufferSizeMax);
		retUpStatus = retError;
		if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
				*upgradeReply.get(), strlen(upDSPProduct->getUpgraderecord()),
				upDSPProduct->getUpgraderecord(), retUpStatus, setNet.get())
				== retOk) {
		}
		INT32 retsendtotmp = sendto(sockfd, (INT8 *) sendtoBuffer,
				sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
				(struct sockaddr *) &sendaddr, tmp_server_addr_len);
		if (retUpStatus == retError) {
			upgradeArgs->handle->setInUpgrade(false);
			return NULL;
		}
	}

	memset(sendtoBuffer, 0, SendBufferSizeMax);
	//get map devs from file head
	map<INT32, DEV_MODULES_TYPE> devModules;
	CrcCheck::getDevModules(fileAttrs->getFileDownloadPath(), devModules);
	if (upDSPProduct->parserItemPackage(
			const_cast<INT8*>(fileAttrs->getNewSoftVersion())) == 0) {
		retUpStatus = retOk;
		upDSPProduct->setUpgraderecord("Upgrading 65%.");

		if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
				*upgradeReply.get(), strlen(upDSPProduct->getUpgraderecord()),
				upDSPProduct->getUpgraderecord(), retUpStatus, setNet.get())
				== retOk) {
		}
		INT32 sendlen = sizeof(PC_DEV_Header) + upgradeReply->header.DataLen;
		INT32 retSendTmp = sendto(sockfd, (INT8 *) sendtoBuffer, sendlen, 0,
				(struct sockaddr *) &sendaddr, tmp_server_addr_len);
	} else {
		retUpStatus = retError;
		upDSPProduct->setUpgraderecord("Upgrade file name error !");
		if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
				*upgradeReply.get(), strlen(upDSPProduct->getUpgraderecord()),
				upDSPProduct->getUpgraderecord(), retUpStatus, setNet.get())
				== retOk) {
		}
		INT32 sendtoret = sendto(sockfd, (INT8 *) sendtoBuffer,
				sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
				(struct sockaddr *) &sendaddr, tmp_server_addr_len);
		if (retUpStatus == retError) {
			upgradeArgs->handle->setInUpgrade(false);
			return NULL;
		}
	}

	SmartPtr<UpgradeDSPSubItem> subItems(new UpgradeDSPSubItem);
	//adjust order
	//dev module check
//	map<INT32, string> mExtract;
	if (FileOperation::extractTarFile(newTarPackage, subItems->mSubItems)
			== true) {
	}

	for (UINT32 i = 1; i <= devModules.size(); i++) {
#if !(DSP9909)
		if (devModules[i] == DEV_AMPLIFIER) {
			executeDevModuleUp(AMPLIFIER, subItems->mSubItems);
		}
#endif
#if (DSP9903)
		else if (devModules[i] == DEV_PAGER) {
			executeDevModuleUp(PAGER, subItems->mSubItems);
		}
#endif
	}
//
	INT32 retIsEnable = 0;
	/////////modify "!="
	retIsEnable = CMDParserUp::isDevModulesUpgradeEnable(
			upgradeArgs->handle->devModuleToUpgrade, devModules,
			*fileAttrs.get());
	if (retIsEnable == retError) {
		memset(replyText, 0, msgLen);
		memset(sendtoBuffer, 0, SendBufferSizeMax);
		retUpStatus = retError;
		/////////////////////////////////////////////////////////////////////////////////
		sprintf(replyText, "Get dev modules upgrade enable failed !");
		if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
				*upgradeReply.get(), strlen(replyText), replyText, retUpStatus,
				setNet.get()) == retOk) {
		}
		sendto(sockfd, (INT8 *) sendtoBuffer,
				sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
				(struct sockaddr *) &sendaddr, tmp_server_addr_len);
		if (retUpStatus == retError) {
//			upgradeArgs->handle->setInUpgrade(false);
			subItems->setEachItemUpResult(false);
		}

	}

	//set function dev module?
	if (subItems->getSubItems(upgradeArgs->handle->devModuleToUpgrade)
			!= true) {
		cout << "up 4 !" << endl;
		memset(replyText, 0, msgLen);
		memset(sendtoBuffer, 0, SendBufferSizeMax);
		retUpStatus = retError;
		sprintf(replyText, "Extract file error !");
		if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
				*upgradeReply.get(), strlen(replyText), replyText, retUpStatus,
				setNet.get()) == retOk) {
		}

		sendto(sockfd, (INT8 *) sendtoBuffer,
				sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
				(struct sockaddr *) &sendaddr, tmp_server_addr_len);
		if (retUpStatus == retError) {
			upgradeArgs->handle->setInUpgrade(false);
			return NULL;
		}
		cout << "up 6 !" << endl;
	}

	UINT32 itemsNum = subItems->getItemsNum();
	cout << "item num : " << itemsNum << endl;
	UINT32 percentUp = 65;
	for (UINT32 i = 1; i <= itemsNum; i++) {

		cout << "up 7 !" << endl;
		const_cast<UpgradeDSP *>(&subItems->getUpObj())->clearObj();
		cout << "up 7.5 clear" << endl;
		if (fileAttrs->getForceStatus() == true)
			subItems->setForceUpgrade(true);
		INT32 retParser = subItems->parserSubItemsFileName(i);
		if (retParser == retOk) {
			Logger::GetInstance().Info("Upgrade item : %s",
					const_cast<UpgradeDSP *>(&subItems->getUpObj())->getMemberItemName());
			cout << "up 5 !" << endl;
		} /*else if (retParser == 1)
		 continue;*/
		else {
			if (const_cast<UpgradeDSP *>(&subItems->getUpObj())->getUpStatus()
					== equalVersion) {
				FileOperation::deleteFile(
						const_cast<UpgradeDSP *>(&subItems->getUpObj())->getUpgradeFile());
				continue;
			} else {
				subItems->setEachItemUpResult(false);
				retUpStatus = retError;
				memset(sendtoBuffer, 0, SendBufferSizeMax);
				if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
						*upgradeReply.get(),
						strlen(
								const_cast<UpgradeDSP *>(&subItems->getUpObj())->getUpgraderecord()),
						const_cast<UpgradeDSP *>(&subItems->getUpObj())->getUpgraderecord(),
						retUpStatus, setNet.get()) == retOk) {
				}
				sendto(sockfd, (INT8 *) sendtoBuffer,
						sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
						(struct sockaddr *) &sendaddr, tmp_server_addr_len);
				upgradeArgs->handle->setInUpgrade(false);
				break;
			}
		}
		memset(replyText, 0, msgLen);
		memset(sendtoBuffer, 0, SendBufferSizeMax);
		if (subItems->upgradeItem(i) == 0) {
			cout << "up 7.1 !" << endl;
			if (FileOperation::extractTarFile(newTarPackage,
					subItems->getExtractItem()) != true) {
				cout << "up 7.2" << endl;
			} else {
				cout << "up 7.3" << endl;
				if (!FileOperation::isExistFile(UpgradeShellWithPath)) {
					cout << "up 7.4" << endl;
					retUpStatus = retError;
					sprintf(replyText, "Upgrade file error !");
					continue;
				}
				cout << "up 7.5" << endl;
			}
			INT8 *PCIP = inet_ntoa(upgradeArgs->recvAddr.sin_addr);
			cout << "up 7.6" << endl;
			if (subItems->excuteUpgradeShell(i, PCIP) == 0) {
				if (!subItems->getUpTerminalDevs()) {
					if (subItems->modifyVersionFile() == retOk) {
						percentUp += (30 / itemsNum);
						sprintf(replyText, "Upgrading %u%%.", percentUp);
						cout << "modify ok" << endl;
						retUpStatus = retOk;
						INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
					} else {
						retUpStatus = retError;
						subItems->setEachItemUpResult(false);
						sprintf(replyText, "Modify version file failed !");
					}
				} else {
					/////////process
					INT32 retUpAmp = HandleUp::upTerminalDevs(
							subItems->getUpDevType(), sockfd, sendaddr,
							setNet.get());
					if (retUpAmp == retOk) {
						percentUp += (30 / itemsNum);
						sprintf(replyText, "Upgrading %u%%.", percentUp);
						cout << "up devs ok type :" << subItems->getUpDevType()
								<< endl;
						retUpStatus = retOk;
					} else if (retUpAmp == retError) {
						retUpStatus = retError;
						subItems->setEachItemUpResult(false);
						sprintf(replyText, "Upgrade devs failed type : %d",
								(INT32) subItems->getUpDevType());
					}
				}
				if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
						*upgradeReply.get(), strlen(replyText), replyText,
						retUpStatus, setNet.get()) == retOk) {
				}
				INT32 retsendto = sendto(sockfd, (INT8 *) sendtoBuffer,
						sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
						(struct sockaddr *) &sendaddr, tmp_server_addr_len);
				cout << "retsendot ::::" << retsendto << endl;
				if (retUpStatus == retError) {
					continue;
				}
				cout << "ok" << endl;
			} else {
				retUpStatus = retError;
				subItems->setEachItemUpResult(false);
//				sprintf(replyText, "Modify item : %s version file failed !",
//						subItems->getExtractItem()[i].c_str());
				if (HandleUp::devReplyHandle<DEV_Request_UpgradeReply>(
						sendtoBuffer, *upgradeReply.get(),
						strlen(
								const_cast<UpgradeDSP *>(&subItems->getUpObj())->getUpgraderecord()),
						const_cast<UpgradeDSP *>(&subItems->getUpObj())->getUpgraderecord(),
						retUpStatus, setNet.get()) == retOk) {
				}
				sendto(sockfd, (INT8 *) sendtoBuffer,
						sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
						(struct sockaddr *) &sendaddr, tmp_server_addr_len);
				cout << "up 8 !" << endl;
			}
			cout << "up 9 !" << endl;
		} else {
			retUpStatus = retError;
			subItems->setEachItemUpResult(false);
			memset(sendtoBuffer, 0, SendBufferSizeMax);
			if (HandleUp::devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
					*upgradeReply.get(),
					strlen(
							const_cast<UpgradeDSP *>(&subItems->getUpObj())->getUpgraderecord()),
					const_cast<UpgradeDSP *>(&subItems->getUpObj())->getUpgraderecord(),
					retUpStatus, setNet.get()) == retOk) {
			}
			sendto(sockfd, (INT8 *) sendtoBuffer,
					sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
					(struct sockaddr *) &sendaddr, tmp_server_addr_len);
			continue;
		}
	}	//end of for()

	cout << "get new version :: 11 " << upDSPProduct->getNewVersion() << endl;
	if (subItems->getEachItemUpResult() == true) {
		upDSPProduct->setUpResult(true);
		cout << "all true" << endl;
		memset(replyText, 0, msgLen);
		cout << "test go to main() fault 1" << endl;
		memset(sendtoBuffer, 0, SendBufferSizeMax);
		cout << "test go to main() fault 2" << endl;
		if (upDSPProduct->modifyVersionFile() != retOk) {
			cout << "test go to main() fault 3" << endl;
			retUpStatus = retError;
			sprintf(replyText, "Upgrade failed !");
			cout << "test go to main() fault 4" << endl;
		} else {
			retUpStatus = retOk;
			cout << "test go to main() fault 5" << endl;
			if (subItems->getUpSystem() == true) {
				cout << "test go to main() fault 6" << endl;
				cout << "upsystemmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm" << endl;
				if (fileAttrs->getWebUpMethod())
					memcpy(replyText, UPFILESYSTEM, strlen(UPFILESYSTEM));
				else
					memcpy(replyText, UPFILESYSTEM, strlen(UPFILESYSTEM));
			} else {
				cout << "test go to main() fault 7" << endl;
				sprintf(replyText, "Upgrade succeeded !");
			}
			cout << "test go to main() fault 8" << endl;
		}
		cout << "test go to main() fault 9" << endl;
	} else {
		cout << "test go to main() fault 10" << endl;
		retUpStatus = retError;
		memset(replyText, 0, msgLen);
		memset(sendtoBuffer, 0, SendBufferSizeMax);
		cout << "test go to main() fault 11" << endl;
		if (subItems->getUpSystem() != true)
			memcpy(replyText, PRODUCTUPFAILED, strlen(PRODUCTUPFAILED));
		else
			memcpy(replyText, PRODUCTUPFAILED_UPSYS,
					strlen(PRODUCTUPFAILED_UPSYS));
	}
	cout << "test go to main() fault 12" << endl;
//	upgradeArgs->upFileAttr->clearMemberData();
	cout << "test go to main() fault 13" << endl;
	if (HandleUp::devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
			*upgradeReply.get(), strlen(replyText), replyText, retUpStatus,
			setNet.get()) == retOk) {
		cout << "test go to main() fault 14" << endl;
	}
	cout << "test go to main() fault 15" << endl;
	INT32 retSendFinal = sendto(sockfd, (INT8 *) sendtoBuffer,
			sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
			(struct sockaddr *) &sendaddr, tmp_server_addr_len);
	cout << "test go to main() fault 16" << endl;
	cout << "retSendFinal: " << retSendFinal << endl;
	upgradeArgs->handle->setInUpgrade(false);

	fileTrans->clearFileTrans();
	Logger::GetInstance().Info(
			"===System will reboot after 2 secs for upgrading system===.");
	HandleUp::sysReboot();
	return NULL;
}

INT32 HandleUp::devSearchHandle(DEV_Reply_GetDevMsg & devMsg,
		DevSearchTerminal * devSearch) {
	devMsg = *devSearch->getDevMsg(pathXml, pathVersionFile);
	return retOk;
}

/******************************************************************************
 * Description :
 To know the version to upgrade is available or not.
 * Return Value : On available, it return 0. On unavailable, -1 is returned.
 * Author : JHB
 * Create Data : 04/01/2018
 * Revision History :
 *   04/01/2018  JHB    Created.
 *****************************************************************************/
INT32 HandleUp::upgradePCrequestHandle(INT8 * recvBuff, INT8 * sendtoBuff,
		DEV_Reply_DevUpgrade & devReply, UpFileAttrs & upFileAttr,
		SetNetworkTerminal * setNetworkTerminal) {
	INT8 failReason[128] = { 0 };
	upgradeFileStatus retUpStatus = errorVersionStatus;
	INT32 status = 0;
	retUpStatus = CMDParserUp::parserPCUpgradeCMD(recvBuff, upFileAttr,
			failReason, devModuleToUpgrade);
	if (retUpStatus == higherVerison) {
		if (FileOperation::isExistFile(upFileAttr.getFileDownloadPath())
				== false && upFileAttr.getWebUpMethod() == true) {
			Logger::GetInstance().Error("File : %s is not existed !",
					upFileAttr.getFileDownloadPath());
			memset(failReason, 0, 128);
			memcpy(failReason, UPFILENOTEXIST, strlen(UPFILENOTEXIST));
			status = retError;
		} else {
			setInUpgrade(true);
			status = retOk;
		}
	} else if (retUpStatus == lowerVersion) {

		memset(failReason, 0, 128);
		memcpy(failReason, "Upgrade file version lower !",
				strlen("Upgrade file version lower !"));
		status = retError;
	} else if (retUpStatus == equalVersion) {
		if (FileOperation::isExistFile(upFileAttr.getFileDownloadPath())) {
			FileOperation::deleteFile(upFileAttr.getFileDownloadPath());
		}
		status = retError;
	} else if (retUpStatus == errorVersionStatus) {
		memset(failReason, 0, 128);
		memcpy(failReason, "Upgrade file version error !",
				strlen("Upgrade file version error !"));
		status = retError;
	}
	devReply.header.HeadCmd = 0x0003;
	if (devReplyHandle<DEV_Reply_DevUpgrade>(sendtoBuff, devReply,
			strlen(failReason), failReason, status, setNetworkTerminal)
			== retOk) {
	}
	return status;
}

INT32 HandleUp::devRequestFileInit(DEV_Request_FileProtocal & request,
		UpFileAttrs & upFileAttr, FileTrans & fileTrans) {
	DEV_Request_FileProtocal mDevRequest;
	request.header.HeadTag = PROTOCAL_PC_DEV_HEAD;
	request.header.HeadCmd = 0x0004;
	request.header.DataLen = devTypeSize + hardVersionSize + softVersionSize
			+ sizeof(mDevRequest.StartPosition)
			+ sizeof(mDevRequest.FileDataLen);
	memcpy(request.DevType, TerminalDevType, strlen(TerminalDevType));
	INT8 hardVersion[8] = { 0 };
	DevSearchTerminal::getSoftwareVersion("hardware_version", hardVersion,
			pathVersionFile);
	memcpy(request.HardVersion, hardVersion, strlen(hardVersion));
	memcpy(request.NewSoftVersion, upFileAttr.getNewSoftVersion(),
			strlen(upFileAttr.getNewSoftVersion()));
	request.StartPosition = fileTrans.getStartPos();
	request.FileDataLen = fileTrans.getSendLen();
	return retOk;
}

INT32 HandleUp::devRequestFile(DEV_Request_FileProtocal & request,
		FileTrans & fileTrans) {
	request.StartPosition = fileTrans.getStartPos();
	request.FileDataLen = fileTrans.getSendLen();
	return retOk;
}

INT32 HandleUp::upTerminalDevs(UPDATE_DEV_TYPE type, INT32 &sockfd,
		sockaddr_in &addr, SetNetworkTerminal *setNet) {
	LocalUDPTrans netTrans;
	UPDATE_SEND_UPDATEDEV devUp;
	devUp.header.HeadCmd = CMD_LOCALDEV_UPGRADE;
	devUp.dev_type = type;
	INT8 recvBuff[16] = { 0 };
	localUpHandle<UPDATE_SEND_UPDATEDEV>(devUp);
	sleep(10);
	INT32 retSend = sendto(netTrans.getSockfd(), &devUp,
			sizeof(UPDATE_SEND_UPDATEDEV), 0,
			(struct sockaddr*) netTrans.getAddr(), *netTrans.getAddrLen());
	if (retSend <= 0) {
		cout << "errorsend" << endl;
		return retError;
	}
	Logger::GetInstance().Info("Will upgrade device type %d !", devUp.dev_type);
	struct timeval timeout /*= { 300, 0 }*/; //3s
#if !(DSP9909)
	if (type == UPDATE_DEV_AMP_TYPE) {
		timeout.tv_sec = 60;
		timeout.tv_usec = 0;
	}
#endif
#if (DSP9903)
	else if (type == UPDATE_DEV_PAGER_TYPE) {
		timeout.tv_sec = 60;
		timeout.tv_usec = 0;
	}
#endif
	SmartPtr<DEV_Request_UpgradeReply> upgradeReply(
			new DEV_Request_UpgradeReply);
	upgradeReply->header.HeadCmd = CMD_DEV_UPGRADE_REPLY;
	INT8 sendtoBuffer[SendBufferSizeMax] = { 0 };
	INT8 replyText[msgLen] = { 0 };
	INT32 retUpStatus = retOk;
	setsockopt(netTrans.getSockfd(), SOL_SOCKET, SO_RCVTIMEO,
			(const INT8 *) &timeout, sizeof(timeout));
	while (1) {
		INT32 retRecv = recvfrom(netTrans.getSockfd(), recvBuff,
				sizeof(recvBuff), 0, (struct sockaddr*) netTrans.getAddr(),
				netTrans.getAddrLen());
		if (retRecv == -1) {
			Logger::GetInstance().Error("Upgrade device type %d no recv !",
					devUp.dev_type);
			cout << "recv error" << endl;
			return retError;
		} else if (retRecv > 0) {
			if (retRecv == sizeof(ARM_REPLAYUPDATE_UPDATEDEV)) {
				ARM_REPLAYUPDATE_UPDATEDEV *upReply =
						(ARM_REPLAYUPDATE_UPDATEDEV*) recvBuff;
				cout << "recv ret : " << retRecv << endl;
				NetTrans::printBufferByHex("Recv ret : ", recvBuff, retRecv);
//				for (INT32 i = 0; i < retRecv; i++) {
//					printf("recv result :: %02x\t", recvBuff[i]);
//				}
				if (upReply->header.HeadCmd == CMD_LOCALDEV_UPGRADE) {
					if (upReply->state == 1 || upReply->state == 2) {
						cout << "get up dev  " << upReply->state
								<< " up success" << endl;
						return retOk;
					} else if (upReply->state == 0 || upReply->state == 3) {
						cout << "get up dev  " << upReply->state << " up failed"
								<< endl;
						return retError;
					} else {
						Logger::GetInstance().Error(
								"Upgrade error with state : %d",
								upReply->state);
						return retError;
					}
				} else
					return retError;
			} else if (retRecv == sizeof(ARM_REPLAY_GETUPDATEPROGRESS)) {
				ARM_REPLAY_GETUPDATEPROGRESS *devReplyInfo =
						(ARM_REPLAY_GETUPDATEPROGRESS *) recvBuff;
				cout << "recv ret : " << retRecv << endl;
				INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
				if (devReplyInfo->header.HeadCmd == CMD_DEV_REPLY_UPGRADE) {
					memset(sendtoBuffer, 0, SendBufferSizeMax);
					memset(replyText, 0, msgLen);
					sprintf(replyText, "Upgrade module %d, process : %d%%",
							(INT32) devReplyInfo->dev_type,
							(INT32) devReplyInfo->state);

					if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
							*upgradeReply.get(), strlen(replyText), replyText,
							retUpStatus, setNet) == retOk) {
					}
					INT32 retsendto = sendto(sockfd, (INT8 *) sendtoBuffer,
							sizeof(PC_DEV_Header)
									+ upgradeReply->header.DataLen, 0,
							(struct sockaddr *) &addr, tmp_server_addr_len);
					cout << "retsendot ::::" << retsendto << endl;
				}
			}

		}

	}
	return retOk;
}

INT32 HandleUp::executeDevModuleUp(const INT8 *module,
		map<INT32, string> &mExtract) {
	INT8 devModuleFilePath[fileDownloadPathSize] = { 0 };
	for (UINT32 j = 1; j <= mExtract.size(); j++) {
		memset(devModuleFilePath, 0, fileDownloadPathSize);
		cout << "mextractjjjjjj:::" << mExtract[j] << endl;
		if (mExtract[j].find(module) != string::npos) {
			cout << "today test seg  fault !!!!!!!!!!!!!!!!!!!!!!!!!-------- 4 "
					<< mExtract[j] << endl;
			string path = upFilePath;
			path += mExtract[j];
			memcpy(devModuleFilePath, path.c_str(), path.size());
			INT32 retParser = CrcCheck::parser_Package(devModuleFilePath, NULL,
			NULL,
			NULL);
			if (retParser != retOk) {
			}
			map<INT32, string> m_subItem;
			if (FileOperation::extractTarFile(newTarPackage, m_subItem)
					!= true) {
				cout << "up 7.2" << endl;
			} else {
				cout << "up 7.3" << endl;
				if (!FileOperation::isExistFile(UpgradeShellWithPath)) {
				}
				if (UpgradeDSPSubItem::excuteDevShell() == true)
					cout << "up 7.5" << endl;
			}
		}
	}
	return retOk;
}

INT32 HandleUp::upMainRootfsRespond(INT32 m_socket, SetNetworkTerminal &net) {
	SmartPtr<DEV_Request_UpgradeReply> devReply(new DEV_Request_UpgradeReply);
	devReply->header.HeadCmd = 0x0005;
	socklen_t addrlen;
	addrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;
	memset(&addr, 0, addrlen);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(net.getPCIP()); //任何主机地址
	addr.sin_port = htons(UpUDPTransPort);
	struct sockaddr_in addr_OldVersion;
	memset(&addr_OldVersion, 0, addrlen);
	addr_OldVersion.sin_family = AF_INET;
	addr_OldVersion.sin_addr.s_addr = inet_addr(net.getPCIP()); //任何主机地址
	addr_OldVersion.sin_port = htons(UpRespondPort);
	INT8 sendtoBuff[SendBufferSizeMax] = { 0 };
	INT8 respond[32] = { 0 };
	memcpy(respond, upSysRespond, strlen(upSysRespond));
	devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuff, *devReply.get(),
			strlen(respond), respond, 0, &net);
	sleep(2);
	INT32 retSend = sendto(m_socket, sendtoBuff,
			sizeof(PC_DEV_Header) + devReply->header.DataLen, 0,
			(struct sockaddr*) &addr, addrlen);
	cout << "sendto pc upgrade system success ...num : " << retSend << endl;

	INT32 retSend_OldVersion = sendto(m_socket, sendtoBuff,
			sizeof(PC_DEV_Header) + devReply->header.DataLen, 0,
			(struct sockaddr*) &addr_OldVersion, addrlen);
	cout << "sendto pc_OldVersion upgrade system success ...num : "
			<< retSend_OldVersion << endl;

	return retSend;
}

void HandleUp::sysReboot() {
	sync();
	sleep(2);
	reboot(RB_AUTOBOOT);
}

INT32 HandleUp::getLoaclMaskFile(UINT16 *mask) {
	FILE *fd = fopen(MASKPATH, "r");
	if (NULL == fd) {
		Logger::GetInstance().Info("Open Mask file : %s failed for %s !",
				MASKPATH, strerror(errno));
		for (INT32 i = 0; i < 4; i++) {
			mask[i] = 0;
		}
		return retError;
	}
	struct stat stat_buf;
	stat(MASKPATH, &stat_buf);
	if (stat_buf.st_size != 8) {
		Logger::GetInstance().Error("Mask file %s size error : %d", MASKPATH,
				stat_buf.st_size);
		fclose(fd);
		for (INT32 i = 0; i < 4; i++) {
			mask[i] = 0;
		}
		return retError;
	}
	UINT8 maskFile[8] = { 0 };
	INT32 retRead = fread(maskFile, sizeof(UINT8), 8, fd);
	if (retRead != 8) {
		Logger::GetInstance().Error("Read mask file error");
		fclose(fd);
		for (INT32 i = 0; i < 4; i++) {
			mask[i] = 0;
		}
		return retError;
	}

	for (INT32 i = 0; i < 4; i++) {
		mask[i] = ~(maskFile[i * 2] | maskFile[i * 2 + 1]) + 1;
	}
	fclose(fd);
	return retOk;
}
INT32 HandleUp::getMaskInfo(UINT16 *mask) {
	LocalUDPTrans netTrans;
	UPDATE_GET_MASK getMask;
	getMask.header.HeadCmd = CMD_LOCALDEV_GETMASK;
	localUpHandle<UPDATE_GET_MASK>(getMask);

	INT32 retSend = sendto(netTrans.getSockfd(), &getMask,
			sizeof(UPDATE_GET_MASK), 0, (struct sockaddr*) netTrans.getAddr(),
			*netTrans.getAddrLen());
	if (retSend <= 0) {
		Logger::GetInstance().Info("Send msg to request mask failed ! %s",
				strerror(errno));
		return retError;
	}
	Logger::GetInstance().Info("Send msg size to request mask : %d .", retSend);
	INT8 recvBuff[16] = { 0 };
	struct timeval timeout = { 2, 0 }; //2s
	setsockopt(netTrans.getSockfd(), SOL_SOCKET, SO_RCVTIMEO,
			(const INT8 *) &timeout, sizeof(timeout));
	INT32 retRecv = recvfrom(netTrans.getSockfd(), recvBuff, sizeof(recvBuff),
			0, (struct sockaddr*) netTrans.getAddr(), netTrans.getAddrLen());
	if (retRecv == -1) {
		Logger::GetInstance().Error("Recv failed form server ! %s",
				strerror(errno));
		for (UINT32 i = 0; i < 4; i++) {
			mask[i] = 0;
		}
		return retError;
	} else if (retRecv > 0) {
		NetTrans::printBufferByHex("Recv buff and size : ", recvBuff, retRecv);
		ARM_REPLAYUPDATE_GETMASK *devReply =
				(ARM_REPLAYUPDATE_GETMASK*) recvBuff;
		if (devReply->header.HeadCmd == CMD_LOCALDEV_GETMASK) {
			memcpy(mask, devReply->m_mask, sizeof(devReply->m_mask));
		} else {
			cout << "get failed!" << endl;
			return retError;
		}
	}
	return retOk;
}

void HandleUp::inUpgradingMsgSend(sockaddr_in recvAddr,
		SetNetworkTerminal *setNetworkTerminal, INT32 sockfd) {

	struct sockaddr_in sendAddr;
	memset(&sendAddr, 0, sizeof(struct sockaddr_in));
	sendAddr.sin_family = AF_INET;
	sendAddr.sin_addr.s_addr = inet_addr(MultiCastAddrSEND); //任何主机地址
	sendAddr.sin_port = htons(UpUDPTransPort);

	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);

	INT32 retUpStatus = retOk;
	SmartPtr<DEV_Request_UpgradeReply> upgradeReply(
			new DEV_Request_UpgradeReply);
	upgradeReply->header.HeadCmd = 0x0005;
	INT8 replyText[msgLen] = { 0 };
	memcpy(replyText, INUPGRADING, strlen(INUPGRADING));
	INT8 sendtoBuffer[SendBufferSizeMax] = { 0 };
	if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
			*upgradeReply.get(), strlen(replyText), replyText, retUpStatus,
			setNetworkTerminal) == retOk) {
		sendto(sockfd, sendtoBuffer,
				sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
				(struct sockaddr *) &recvAddr, tmp_server_addr_len);
		sendto(sockfd, sendtoBuffer,
				sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
				(struct sockaddr *) &sendAddr, tmp_server_addr_len);
//			getUDPNetTransInstance().UDPSendMsg(sendtoBuffer,
//					sizeof(PC_DEV_Header) + upgradeReply->header.DataLen,
//					recvAddr);
	}
}

double HandleUp::diffTimeval(const struct timeval *end,
		const struct timeval *start) {
	double diffTime = 0.0;
	time_t second_diff = end->tv_sec - start->tv_sec;
	suseconds_t usecond_diff = end->tv_usec - start->tv_usec;
	diffTime = second_diff;
	diffTime *= 1000000.0;
	diffTime += usecond_diff;
	return diffTime;
}

