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
using namespace FrameWork;

HandleUp::HandleUp() /*:
 inUpgrade(false)*/{
}
HandleUp::~HandleUp() {
}

HandleUp::HandleUp(const HandleUp &handle) /*:
 inUpgrade(handle.inUpgrade)*/{
	inUpgrade = handle.inUpgrade;
}
HandleUp &HandleUp::operator=(const HandleUp &handle) {
	if (this != &handle) {
		inUpgrade = handle.inUpgrade;
	}
	return *this;
}

Mutex HandleUp::mutex;
bool HandleUp::inUpgrade;

HandleUp& HandleUp::getInstance() {
	static HandleUp handle;
	return handle;
}

void HandleUp::devSearchCMDHandle(sockaddr_in recvAddr,
		SetNetworkTerminal *setNetworkTerminal, UpFileAttrs &upFileAttr,
		INT32 sockfd) {
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	DevSearchTerminal devSearchTerminal(setNetworkTerminal);
	SmartPtr<DEV_Reply_GetDevMsg> tmpReMsg(new DEV_Reply_GetDevMsg);
	devSearchHandle(*tmpReMsg.get(), &devSearchTerminal);
	cout << "headtasearchgudp  "
			<< devSearchTerminal.devReplyMsg->header.HeadTag << endl;
	cout << "headtag  " << tmpReMsg->header.HeadTag << endl;
	cout << "HeadCmd  " << tmpReMsg->header.HeadCmd << endl;
	cout << "DataLen  " << tmpReMsg->header.DataLen << endl;
	struct sockaddr_in sin;
	sin.sin_addr.s_addr = tmpReMsg->DevServerIP;
	INT8 *devIP = NULL;
	devIP = inet_ntoa(sin.sin_addr);
	cout << "DevIP  " << tmpReMsg->DevIP << endl;
	cout << "DevMask  " << tmpReMsg->DevMask << endl;
	cout << "DevGateway  " << tmpReMsg->DevGateway << endl;
	cout << "DevServerIP  " << devIP << endl;
	cout << "DevServerPort  " << tmpReMsg->DevServerPort << endl;
	cout << "DevMACAddress  " << tmpReMsg->DevMACAddress << endl;
	cout << "DevID  " << tmpReMsg->DevID << endl;
	cout << "DevType  " << tmpReMsg->DevType << endl;
	cout << "HardVersion  " << tmpReMsg->HardVersion << endl;
	cout << "SoftVersion  " << tmpReMsg->SoftVersion << endl;
	cout << "DevName  " << tmpReMsg->DevName << endl;
	sendto(sockfd, tmpReMsg.get(), sizeof(DEV_Reply_GetDevMsg), 0,
			(struct sockaddr *) &recvAddr, tmp_server_addr_len);
	printf("found clint IP is:%s\n", inet_ntoa(recvAddr.sin_addr));
	if (getInUpgrade() == true) {
		INT32 retUpStatus = retOk;
		SmartPtr<DEV_Request_UpgradeReply> upgradeReply(
				new DEV_Request_UpgradeReply);
		upgradeReply->header.HeadCmd = 0x0005;
		INT8 replyText[msgLen] = { 0 };
		memcpy(replyText, "Upgrading...", strlen("Upgrading..."));
		INT8 sendtoBuffer[SendBufferSizeMax] = { 0 };
		if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
				*upgradeReply.get(), strlen(replyText), replyText, retUpStatus,
				setNetworkTerminal) == retOk) {
			sendto(sockfd, (INT8*) sendtoBuffer,
					sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
					(struct sockaddr *) &recvAddr, tmp_server_addr_len);
		}
	}
}

INT32 HandleUp::setNetworkHandle(INT8 *recvBuff, INT8 *sendtoBuff,
		DEV_Reply_ParameterSetting &devReplySetPara,
		SetNetworkTerminal *setNetworkTerminal, INT32 sockfd,
		sockaddr_in recvAddr) {
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	INT8 failReason[128] = { 0 };
	map<string, string> retContent;
	INT8 retSetNet = 0;
	retSetNet = CMDParserUp::parserPCSetNetCMD(recvBuff, setNetworkTerminal,
			retContent);
	devReplySetPara.header.HeadCmd = 0x0002;

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
		UINT32 reasonLen = strlen(iter->first.c_str()) + 3
				+ strlen(iter->second.c_str());
		if (devReplyHandle<DEV_Reply_ParameterSetting>(sendtoBuff,
				devReplySetPara, reasonLen, failReason, retSetNet,
				setNetworkTerminal) == retOk) {
			sendto(sockfd, sendtoBuff,
					sizeof(PC_DEV_Header) + devReplySetPara.header.DataLen, 0,
					(struct sockaddr *) &recvAddr, tmp_server_addr_len);
		}
	}
	return retOk;
}

void HandleUp::devParamSetCMDHandle(sockaddr_in recvAddr, INT8 *recvBuff,
		SetNetworkTerminal *setNetworkTerminal, INT32 sockfd) {
	SmartPtr<DEV_Reply_ParameterSetting> devReplyParaSetting(
			new DEV_Reply_ParameterSetting);
	INT8 sendtoBuffer[SendBufferSizeMax] = { 0 };
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	setNetworkHandle(recvBuff, sendtoBuffer, *devReplyParaSetting.get(),
			setNetworkTerminal, sockfd, recvAddr);
}

void HandleUp::devUpgradePCRequestCMDHandle(sockaddr_in &recvAddr,
		INT8 *recvBuff, SetNetworkTerminal *setNetworkTerminal, INT32 &sockfd,
		UpFileAttrs &upFileAttr, FileTrans &fileTrans,
		DEV_Request_FileProtocal *request) {
	INT8 sendtoBuffer[SendBufferSizeMax] = { 0 };
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	SmartPtr<DEV_Reply_DevUpgrade> devReply(new DEV_Reply_DevUpgrade);
	INT32 retHandle = upgradePCrequestHandle(recvBuff, sendtoBuffer,
			*devReply.get(), upFileAttr, setNetworkTerminal);
	INT32 retSendto = sendto(sockfd, sendtoBuffer,
			sizeof(PC_DEV_Header) + devReply->header.DataLen, 0,
			(struct sockaddr *) &recvAddr, tmp_server_addr_len);
	cout<< "up request first ::::::::::::::::::::"<<endl;
	if (retHandle != retOk) {
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
		pthread_t tid;
		INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
		if (pthread_create(&tid, NULL, UpgradeThreadFun,
				(void *) transArgs.get()) == 0) {
			printf("Create upgrade thread successfully!........\n");
		} else
			;
//		TerminalUpgradeHandle(recvAddr, recvBuff, setNetworkTerminal, sockfd,
//				upFileAttr, fileTrans, request);
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
		sendto(sockfd, (INT8*) request,
				sizeof(PC_DEV_Header) + request->header.DataLen, 0,
				(struct sockaddr *) &recvAddr, tmp_server_addr_len);
	} /*end send if*/
	ofstream f(upFileAttr.getFileDownloadPath(), ios::trunc);
	f.close();
}

void HandleUp::devFileTransCMDHandle(sockaddr_in &recvAddr, INT8 *recvBuff,
		SetNetworkTerminal *setNetworkTerminal, INT32 &sockfd,
		UpFileAttrs &upFileAttr, FileTrans &fileTrans,
		DEV_Request_FileProtocal *request) {
//	SmartPtr<FileTransArgs> transArgs(
//			new FileTransArgs(&recvAddr, recvBuff, setNetworkTerminal, &sockfd,
//					&upFileAttr, &fileTrans, request));
//	transArgs->recvAddr = &recvAddr;
//	transArgs->recvBuff = recvBuff;
//	transArgs->setNetworkTerminal = setNetworkTerminal;
//	transArgs->sockfd = &sockfd;
//	transArgs->upFileAttr = upFileAttr;
//	transArgs->fileTrans = &fileTrans;
//	transArgs->request = request;
//	pthread_t tid;
//	if (pthread_create(&tid, NULL, TransUpgradeThreadFun,
//			(void *) transArgs.get()) == 0) {
//		printf("Create file transport thread successfully!........\n");
//	} else
//		;
#if 1
	SmartPtr<DEV_Request_UpgradeReply> upgradeReply(
			new DEV_Request_UpgradeReply);
	INT8 replyText[msgLen] = { 0 };
	INT8 sendtoBuffer[SendBufferSizeMax] = { 0 };
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	INT32 retUpStatus = retOk;
	writeFileFromPC<PC_Reply_FileProtocal>(recvBuff,
			upFileAttr.getFileDownloadPath());

	fileTrans.changeRemainedPos().setPersentage();
	if (devRequestFile(*request, fileTrans) == retOk) {
		sendto(sockfd, (INT8*) request,
				sizeof(PC_DEV_Header) + request->header.DataLen, 0,
				(struct sockaddr *) &recvAddr, tmp_server_addr_len);
	}
	if (fileTrans.getNewPercent() > fileTrans.getOldPercent()) {
		upgradeReply->header.HeadCmd = 0x0005;
		memset(replyText, 0, msgLen);
		sprintf(replyText, "Upgrading %u%%", fileTrans.getNewPercent());
		replyText[strlen(replyText)] = '\0';
		cout << "text : " << replyText << endl;
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
		UINT8 md5_str[MD5_SIZE];
		if (!GetFileMD5(upFileAttr.getFileDownloadPath(), md5_str)) {
			cout << "get file md5 error" << endl;
		}
		int iret = memcmp(upFileAttr.getFileMD5Code(), md5_str,
		MD5_SIZE);
		memset(replyText, 0, msgLen);
		memset(sendtoBuffer, 0, SendBufferSizeMax);
		if (iret != 0) {
			retUpStatus = retError;
			cout << "MD5 error" << endl;
			sprintf(replyText, "File error, please try again !");
		} else {
			retUpStatus = retOk;
			sprintf(replyText, "Upgrading 55%%.");
			replyText[strlen(replyText)] = '\0';
			Logger::GetInstance().Info("File MD5 check finished !");
		}
		cout << "replytext : " << replyText << endl;
		if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
				*upgradeReply.get(), strlen(replyText), replyText, retUpStatus,
				setNetworkTerminal) == retOk) {
		}
		sendto(sockfd, (INT8*) sendtoBuffer,
				sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
				(struct sockaddr *) &recvAddr, tmp_server_addr_len);
		if (retUpStatus == retError)
			return;
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
			printf("Create upgrade thread successfully!........\n");
		} else
			;
//		pthread_mutex_destroy(&mutex);
//		pthread_mutex_destroy(&mutex);
//		TerminalUpgradeHandle(recvAddr, recvBuff, setNetworkTerminal, sockfd,
//				upFileAttr, fileTrans, request);
	}
#endif
	return;
}

void HandleUp::devGetMaskCMDHandle(sockaddr_in &recvAddr,
		SetNetworkTerminal *setNetworkTerminal, INT32 &sockfd) {
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	SmartPtr<DEV_Request_MaskInfo> maskInfo(new DEV_Request_MaskInfo);
	maskInfo->header.HeadTag = PROTOCAL_PC_DEV_HEAD;
	maskInfo->header.HeadCmd = CMD_DEV_GETMASK;
	maskInfo->header.DataLen = sizeof(maskInfo->DevID)
			+ sizeof(maskInfo->m_mask);
	char mac[13] = { 0 };
	strcpy(mac,
			setNetworkTerminal->castMacToChar13(mac,
					setNetworkTerminal->getNetConfStruct().macAddr));
	mac[12] = '\0';
	strncpy(maskInfo->DevID, TerminalDevTypeID, strlen(TerminalDevTypeID));
	strcpy(maskInfo->DevID + strlen(TerminalDevTypeID), mac);
	INT32 retGet = getMaskInfo(maskInfo->m_mask);
	if (retGet == retOk) {
		INT32 sendtoret = sendto(sockfd, (INT8*) maskInfo.get(),
				sizeof(DEV_Request_MaskInfo), 0, (struct sockaddr *) &recvAddr,
				tmp_server_addr_len);
		if (sendtoret > 0)
			return;
		else {
			return;
		}
	}
}

void HandleUp::devTestModeCntCMDHandle(INT8 *recvBuff) {
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
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	sendto(sockfd, (INT8 *) devVersion.get(), sizeof(DEV_VersionNum), 0,
			(struct sockaddr *) &recvAddr, tmp_server_addr_len);
}

void *HandleUp::UpgradeThreadFun(void *args) {
	FileTransArgs *upgradeArgs = (FileTransArgs *) args;
	sockaddr_in sendaddr;
	memset(&sendaddr, 0, sizeof(sockaddr_in));
	memcpy(&sendaddr, &upgradeArgs->recvAddr, sizeof(sockaddr_in));
	INT8 *PCIP1 = inet_ntoa(sendaddr.sin_addr);
	cout << "PCIPLLLLLLLLLLLLLLLLL!!!!!!!!111:" << " " << PCIP1 << endl;
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	INT32 retsendfirs0 = sendto(*upgradeArgs->sockfd, "111111111111",
			sizeof("111111111111"), 0, (struct sockaddr *) &sendaddr,
			tmp_server_addr_len);
	cout << "web send to first 0 " << retsendfirs0 << endl;
	SmartPtr<SetNetworkTerminal> setNet(
			new SetNetworkTerminal(*upgradeArgs->setNetworkTerminal));
	INT32 sockfd = *upgradeArgs->sockfd;
	cout <<"sockfd : 00000000000000"<<sockfd<<endl;
	cout << "================================================"<<endl;
	SmartPtr<UpFileAttrs> fileAttrs(new UpFileAttrs(*upgradeArgs->upFileAttr));
	SmartPtr<FileTrans> fileTrans(new FileTrans(*upgradeArgs->fileTrans));
//	SmartPtr<HandleUp> handle(new HandleUp(*upgradeArgs->handle));

	INT32 retsendfirs1 = sendto(sockfd, "111111111111", sizeof("111111111111"),
			0, (struct sockaddr *) &sendaddr, tmp_server_addr_len);
	cout << "web send to first 1 " << retsendfirs1 << endl;

	SmartPtr<DEV_Request_UpgradeReply> upgradeReply(
			new DEV_Request_UpgradeReply);
	INT8 replyText[msgLen] = { 0 };
	INT8 sendtoBuffer[SendBufferSizeMax] = { 0 };
	INT32 retUpStatus = retOk;

	upgradeReply->header.HeadCmd = 0x0005;
	SmartPtr<UpgradeDSP> upDSPProduct(
			new UpgradeDSP(
					const_cast<INT8*>(fileAttrs->getFileDownloadPath())));
	if (upDSPProduct->parserFileName() == retOk) {
		cout << "up 1 !" << endl;
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
		cout << "first sendto :: " << retsendtotmp << endl;
		if (retUpStatus == retError)
			return NULL;
	}

	cout << "up 3 !" << endl;
	memset(sendtoBuffer, 0, SendBufferSizeMax);

	if (upDSPProduct->parserItemPackage(
			const_cast<INT8*>(fileAttrs->getNewSoftVersion())) == 0) {
		retUpStatus = retOk;
		upDSPProduct->setUpgraderecord("Upgrading 65%.");
		cout << "up 2 !" << endl;
		if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
				*upgradeReply.get(), strlen(upDSPProduct->getUpgraderecord()),
				upDSPProduct->getUpgraderecord(), retUpStatus, setNet.get())
				== retOk) {
		}
		INT32 sendlen = sizeof(PC_DEV_Header) + upgradeReply->header.DataLen;
//		INT8 *PCIP1 = inet_ntoa(sendaddr.sin_addr);
//		cout << "PCIPLLLLLLLLLLLLLLLLL!!!!!!!!111:" << " " << PCIP1 << endl;
		INT32 retSendTmp = sendto(sockfd, (INT8 *) sendtoBuffer, sendlen, 0,
				(struct sockaddr *) &sendaddr, tmp_server_addr_len);
	} else {
		retUpStatus = retError;
		upDSPProduct->setUpgraderecord("Upgrade file error !");
		if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
				*upgradeReply.get(), strlen(upDSPProduct->getUpgraderecord()),
				upDSPProduct->getUpgraderecord(), retUpStatus, setNet.get())
				== retOk) {
		}
		INT32 sendtoret = sendto(sockfd, (INT8 *) sendtoBuffer,
				sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
				(struct sockaddr *) &sendaddr, tmp_server_addr_len);
		if (retUpStatus == retError)
			return NULL;
	}
	SmartPtr<UpgradeDSPSubItem> subItems(new UpgradeDSPSubItem);
	if (subItems->getSubItems() != true) {
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
		if (retUpStatus == retError)
			return NULL;
		cout << "up 6 !" << endl;
	}
	UINT32 itemsNum = subItems->getItemsNum();
	cout << "item num : " << itemsNum << endl;
	UINT32 percentUp = 65;
	for (UINT32 i = 1; i <= itemsNum; i++) {
		cout << "up 7 !" << endl;
		const_cast<UpgradeDSP *>(&subItems->getUpObj())->clearObj();
		cout << "up 7.5 clear" << endl;
		INT32 retParser = subItems->parserSubItemsFileName(i);
		if (retParser == retOk) {
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
						INT32 retsendfirs2 = sendto(*upgradeArgs->sockfd,
								"111111111111", sizeof("111111111111"), 0,
								(struct sockaddr *) &sendaddr,
								tmp_server_addr_len);
						cout << "web send to first 2 " << retsendfirs2 << endl;
					} else {
						retUpStatus = retError;
						subItems->setEachItemUpResult(false);
						sprintf(replyText, "Modify version file failed !");
					}
				} else {
					INT32 retUpAmp = HandleUp::upTerminalDevs(
							subItems->getUpDevType());
					if (retUpAmp == retOk) {
						percentUp += (30 / itemsNum);
						sprintf(replyText, "Upgrading %u%%.", percentUp);
						cout << "up Amplifier ok" << endl;
						retUpStatus = retOk;
					} else if (retUpAmp == retError) {
						retUpStatus = retError;
						subItems->setEachItemUpResult(false);
						sprintf(replyText, "Upgrade Amplifier failed");
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
				if (retUpStatus == retError)
					break;
				cout << "ok" << endl;
			} else {
				retUpStatus = retError;
				subItems->setEachItemUpResult(false);
				sprintf(replyText, "Modify version file failed !");
				if (HandleUp::devReplyHandle<DEV_Request_UpgradeReply>(
						sendtoBuffer, *upgradeReply.get(), strlen(replyText),
						replyText, retUpStatus, setNet.get()) == retOk) {
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
			break;
		}
	}

	if (subItems->getEachItemUpResult() == true) {
		upDSPProduct->setUpResult(true);
		cout << "all true" << endl;
		memset(replyText, 0, msgLen);
		memset(sendtoBuffer, 0, SendBufferSizeMax);
		if (upDSPProduct->modifyVersionFile() != retOk) {
			retUpStatus = retError;
			sprintf(replyText, "Upgrade failed !");
		} else {
			retUpStatus = retOk;
			if (subItems->getUpSystem() == true) {
				cout << "upsystemmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm"<<endl;
				if (fileAttrs->getWebUpMethod())
					memcpy(replyText, UPFILESYSTEM, strlen(UPFILESYSTEM));
				else
					sprintf(replyText, "Upgrade system....");
			} else {
				sprintf(replyText, "Upgrade successed !");
			}
		}
		if (HandleUp::devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
				*upgradeReply.get(), strlen(replyText), replyText, retUpStatus,
				setNet.get()) == retOk) {
		}
		sendto(sockfd, (INT8 *) sendtoBuffer,
				sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
				(struct sockaddr *) &sendaddr, tmp_server_addr_len);
		upgradeArgs->handle->setInUpgrade(false);

		fileTrans->clearFileTrans();
		sync();
		sleep(2);
//		HandleUp::sysReboot();
		return NULL;
	}

	return NULL;
}

INT32 HandleUp::devSearchHandle(DEV_Reply_GetDevMsg & devMsg,
		DevSearchTerminal * devSearch) {
//	cout << "lock test 5 "<<endl;
	devMsg = *devSearch->getDevMsg(pathXml, pathVersionFile);
//	cout << "lock test 6 "<<endl;
	return retOk;
}
INT32 HandleUp::upgradePCrequestHandle(INT8 * recvBuff, INT8 * sendtoBuff,
		DEV_Reply_DevUpgrade & devReply, UpFileAttrs & upFileAttr,
		SetNetworkTerminal * setNetworkTerminal) {
	INT8 failReason[128] = { 0 };
	upgradeFileStatus retUpStatus = errorVersionStatus;
	INT32 status = 0;
	retUpStatus = CMDParserUp::parserPCUpgradeCMD(recvBuff, upFileAttr,
			failReason);
	if (retUpStatus == higherVerison) {
		setInUpgrade(true);
//		upFileAttr.setInUpgradeStatus(true);
	}
	devReply.header.HeadCmd = 0x0003;
	if (retUpStatus == higherVerison) {
		status = retOk;
	} else if (retUpStatus == lowerVersion) {
		status = retError;
	} else if (retUpStatus == equalVersion) {
		status = retError;
	} else if (retUpStatus == errorVersionStatus) {
		status = retError;
	}
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

INT32 HandleUp::upTerminalDevs(UPDATE_DEV_TYPE type) {
	LocalUDPTrans netTrans;
	UPDATE_SEND_UPDATEDEV devUp;
	devUp.header.HeadCmd = CMD_LOCALDEV_UPGRADE;
	devUp.dev_type = type;
	INT8 recvBuff[16] = { 0 };
	localUpHandle<UPDATE_SEND_UPDATEDEV>(devUp);

	INT32 retSend = sendto(netTrans.getSockfd(), &devUp,
			sizeof(UPDATE_SEND_UPDATEDEV), 0,
			(struct sockaddr*) netTrans.getAddr(), *netTrans.getAddrLen());
	if (retSend <= 0) {
		cout << "errorsend" << endl;
		return retError;
	}
	Logger::GetInstance().Info("Will upgrade device type %d !", devUp.dev_type);
//	cout << "retsend : " << retSend << endl;
	INT32 retRecv = recvfrom(netTrans.getSockfd(), recvBuff, sizeof(recvBuff),
			0, (struct sockaddr*) netTrans.getAddr(), netTrans.getAddrLen());
	if (retRecv == -1) {
		Logger::GetInstance().Info("Upgrade device type %d no recv !",
				devUp.dev_type);
		cout << "recv error" << endl;
		return retError;
	} else if (retRecv > 0) {
		ARM_REPLAYUPDATE_UPDATEDEV *upReply =
				(ARM_REPLAYUPDATE_UPDATEDEV*) recvBuff;
		cout << "recv ret : " << retRecv << endl;
		if (upReply->header.HeadCmd == CMD_LOCALDEV_UPGRADE) {
			if (upReply->state == 1) {
				return retOk;
			} else if (upReply->state == 0) {
				return retError;
			} else {
				return retError;
			}
		}
	}
	return retOk;
}

INT32 HandleUp::upMainRootfsRespond(INT32 m_socket, SetNetworkTerminal &net) {
	SmartPtr<DEV_Request_UpgradeReply> devReply(new DEV_Request_UpgradeReply);
	devReply->header.HeadCmd = 0x0005;
	struct sockaddr_in addr;
	socklen_t addrlen;
	addrlen = sizeof(struct sockaddr_in);
	memset(&addr, 0, addrlen);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(net.getPCIP()); //任何主机地址
	addr.sin_port = htons(UpRespondPort);
	INT8 sendtoBuff[SendBufferSizeMax] = { 0 };
	INT8 respond[32] = { 0 };
	memcpy(respond, upSysRespond, strlen(upSysRespond));
	devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuff, *devReply.get(),
			strlen(respond), respond, 0, &net);
	INT32 retSend = sendto(m_socket, sendtoBuff,
			sizeof(PC_DEV_Header) + devReply->header.DataLen, 0,
			(struct sockaddr*) &addr, addrlen);
	return retSend;
}

void HandleUp::sysReboot() {
	reboot(RB_AUTOBOOT);
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
		cout << "errorsend" << endl;
		return retError;
	}
	cout << "retsend : " << retSend << endl;
	INT8 recvBuff[16] = { 0 };
	INT32 retRecv = recvfrom(netTrans.getSockfd(), recvBuff, sizeof(recvBuff),
			0, (struct sockaddr*) netTrans.getAddr(), netTrans.getAddrLen());
	if (retRecv == -1) {
		/////////////////////////////////////////////enter error!
		cout << "recv error" << endl;
		return retError;
	} else if (retRecv > 0) {
		cout << "recv ret : " << retRecv << " recv buff : " << recvBuff << endl;
		SmartPtr<ARM_REPLAYUPDATE_GETMASK> devReply(
				new ARM_REPLAYUPDATE_GETMASK);
		devReply = (ARM_REPLAYUPDATE_GETMASK*) recvBuff;
		if (devReply->header.HeadCmd == CMD_LOCALDEV_GETMASK) {
			memcpy(mask, devReply->m_mask, sizeof(devReply->m_mask));
		} else {
			cout << "get failed!" << endl;
			return retError;
		}
	}
	return retOk;
}

