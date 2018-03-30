#include "HandleUp.h"
#include "UpgradeServiceConfig.h"
#include "CMDParserUp.h"
#include "Upgrade.h"
#include "FileOperation.h"

HandleUp::HandleUp() {
}
HandleUp::~HandleUp() {
}

HandleUp& HandleUp::getInstance() {
	static HandleUp handle;
	return handle;
}

void HandleUp::devSearchCMDHandle(sockaddr_in recvAddr,
		SetNetworkTerminal *setNetworkTerminal, INT32 sockfd) {
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
	/* sendto */
}

INT32 HandleUp::setNetworkHandle(INT8 *recvBuff, INT8 *sendtoBuff,
		DEV_Reply_ParameterSetting &devReplySetPara,
		SetNetworkTerminal *setNetworkTerminal) {
	INT8 failReason[128] = { 0 };
	INT8 retSetNet = 0;
	retSetNet = CMDParserUp::parserPCSetNetCMD(recvBuff, setNetworkTerminal,
			*failReason);
	devReplySetPara.header.HeadCmd = 0x0002;
	if (devReplyHandle<DEV_Reply_ParameterSetting>(sendtoBuff, devReplySetPara,
			failReason, retSetNet, setNetworkTerminal) == retOk) {
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
			setNetworkTerminal);
	sendto(sockfd, sendtoBuffer,
			sizeof(PC_DEV_Header) + devReplyParaSetting->header.DataLen, 0,
			(struct sockaddr *) &recvAddr, tmp_server_addr_len);
}

void HandleUp::devUpgradePCRequestCMDHandle(sockaddr_in recvAddr,
		INT8 *recvBuff, SetNetworkTerminal *setNetworkTerminal, INT32 sockfd,
		UpFileAttrs &upFileAttr, FileTrans &fileTrans,
		DEV_Request_FileProtocal *request) {
	INT8 sendtoBuffer[SendBufferSizeMax] = { 0 };
	INT32 tmp_server_addr_len = sizeof(struct sockaddr_in);
	SmartPtr<DEV_Reply_DevUpgrade> devReply(new DEV_Reply_DevUpgrade);
	INT32 retHandle = upgradePCrequestHandle(recvBuff, sendtoBuffer,
			*devReply.get(), upFileAttr, setNetworkTerminal);
	sendto(sockfd, sendtoBuffer,
			sizeof(PC_DEV_Header) + devReply->header.DataLen, 0,
			(struct sockaddr *) &recvAddr, tmp_server_addr_len);
	if (retHandle != retOk) {
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
#endif
#if 1
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

void HandleUp::devFileTransCMDHandle(sockaddr_in recvAddr, INT8 *recvBuff,
		SetNetworkTerminal *setNetworkTerminal, INT32 sockfd,
		UpFileAttrs &upFileAttr, FileTrans &fileTrans,
		DEV_Request_FileProtocal *request) {

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

		if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
				*upgradeReply.get(), replyText, retUpStatus, setNetworkTerminal)
				== retOk) {
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
			cout << "ok md5!" << endl;
		}
		cout << "replytext : " << replyText << endl;
		if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
				*upgradeReply.get(), replyText, retUpStatus, setNetworkTerminal)
				== retOk) {
		}
		sendto(sockfd, (INT8*) sendtoBuffer,
				sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
				(struct sockaddr *) &recvAddr, tmp_server_addr_len);
		if (retUpStatus == retError)
			return;
		/*next step*/
		SmartPtr<UpgradeDSP> upDSPProduct(
				new UpgradeDSP(upFileAttr.getFileDownloadPath()));
		if (upDSPProduct->parserFileName() == retOk) {
			//
			cout << "up 1 !" << endl;
		} else {
			memset(sendtoBuffer, 0, SendBufferSizeMax);
			retUpStatus = retError;
			if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
					*upgradeReply.get(), upDSPProduct->getUpgraderecord(),
					retUpStatus, setNetworkTerminal) == retOk) {
			}
			sendto(sockfd, (INT8*) sendtoBuffer,
					sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
					(struct sockaddr *) &recvAddr, tmp_server_addr_len);
			if (retUpStatus == retError)
				return;
		}

		cout << "up 3 !" << endl;
		memset(sendtoBuffer, 0, SendBufferSizeMax);
		if (upDSPProduct->parserItemPackage(upFileAttr.getNewSoftVersion())
				== 0) {
			retUpStatus = retOk;
			upDSPProduct->setUpgraderecord("Upgrading 65%%.");
			cout << "up 2 !" << endl;
		} else {
			retUpStatus = retError;
			upDSPProduct->setUpgraderecord("Upgrade file error !");
			if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
					*upgradeReply.get(), upDSPProduct->getUpgraderecord(),
					retUpStatus, setNetworkTerminal) == retOk) {
			}
			sendto(sockfd, (INT8*) sendtoBuffer,
					sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
					(struct sockaddr *) &recvAddr, tmp_server_addr_len);
			if (retUpStatus == retError)
				return;
		}
		SmartPtr<UpgradeDSPSubItem> subItems(new UpgradeDSPSubItem);
		if (subItems->getSubItems() != true) {
			cout << "up 4 !" << endl;
			memset(replyText, 0, msgLen);
			memset(sendtoBuffer, 0, SendBufferSizeMax);
			retUpStatus = retError;
			sprintf(replyText, "Extract file error !");
			if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
					*upgradeReply.get(), replyText, retUpStatus,
					setNetworkTerminal) == retOk) {
			}
			sendto(sockfd, (INT8*) sendtoBuffer,
					sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
					(struct sockaddr *) &recvAddr, tmp_server_addr_len);
			if (retUpStatus == retError)
				return;
			cout << "up 6 !" << endl;
		}
		UINT32 itemsNum = subItems->getItemsNum();
		cout << "item num : " << itemsNum << endl;
		UINT32 percentUp = 65;
		for (UINT32 i = 1; i <= itemsNum; i++) {
			cout << "up 7 !" << endl;
			/////////////////////
			const_cast<UpgradeDSP*>(&subItems->getUpObj())->clearObj();
			cout << "up 7.5 clear" << endl;
			if (subItems->parserSubItemsFileName(i) == 0) {
				cout << "up 5 !" << endl;
			} else {
				if (const_cast<UpgradeDSP*>(&subItems->getUpObj())->getUpStatus()
						== equalVersion) {
					continue;
				} else {
					subItems->setEachItemUpResult(false);
					retUpStatus = retError;
					memset(sendtoBuffer, 0, SendBufferSizeMax);
					if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
							*upgradeReply.get(),
							const_cast<UpgradeDSP*>(&subItems->getUpObj())->getUpgraderecord(),
							retUpStatus, setNetworkTerminal) == retOk) {
					}
					sendto(sockfd, (INT8*) sendtoBuffer,
							sizeof(PC_DEV_Header)
									+ upgradeReply->header.DataLen, 0,
							(struct sockaddr *) &recvAddr, tmp_server_addr_len);
					break;
				}
			}
			/////////////////////
			memset(replyText, 0, msgLen);
			memset(sendtoBuffer, 0, SendBufferSizeMax);
			if (subItems->upgradeItem(i) == 0) {
				cout << "up 7.1 !" << endl;
				if (FileOperation::extractTarFile(newTarPackage,
						subItems->getExtractItem()) != true) {
					cout << "up 7.2" << endl;

				} else {
					if (!FileOperation::isExistFile(UpgradeShellWithPath)) {
						retUpStatus = retError;
						sprintf(replyText, "Upgrade file error !");
					}
				}

				if (subItems->excuteUpgradeShell(i) == 0) {
					if (subItems->modifyVersionFile() == retOk) {

						percentUp += 30 * (i / itemsNum);
						sprintf(replyText, "Upgrading %u%%.", percentUp);
						cout << "modify ok" << endl;
						retUpStatus = retOk;

					} else {
						retUpStatus = retError;
						sprintf(replyText, "Modify version file failed !");
					}
					if (devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
							*upgradeReply.get(), replyText, retUpStatus,
							setNetworkTerminal) == retOk) {
					}
					sendto(sockfd, (INT8*) sendtoBuffer,
							sizeof(PC_DEV_Header)
									+ upgradeReply->header.DataLen, 0,
							(struct sockaddr *) &recvAddr, tmp_server_addr_len);
					if (retUpStatus == retError)
						break;
					cout << "ok" << endl;
				} else {
					retUpStatus = retError;
					sprintf(replyText, "Modify version file failed !");
					if (HandleUp::devReplyHandle<DEV_Request_UpgradeReply>(
							sendtoBuffer, *upgradeReply.get(), replyText,
							retUpStatus, setNetworkTerminal) == retOk) {
					}
					sendto(sockfd, (INT8*) sendtoBuffer,
							sizeof(PC_DEV_Header)
									+ upgradeReply->header.DataLen, 0,
							(struct sockaddr *) &recvAddr, tmp_server_addr_len);
					cout << "up 8 !" << endl;
				}
				cout << "up 9 !" << endl;
			} else {
				retUpStatus = retError;
				memset(sendtoBuffer, 0, SendBufferSizeMax);
				if (HandleUp::devReplyHandle<DEV_Request_UpgradeReply>(
						sendtoBuffer, *upgradeReply.get(),
						const_cast<UpgradeDSP*>(&subItems->getUpObj())->getUpgraderecord(),
						retUpStatus, setNetworkTerminal) == retOk) {
				}
				sendto(sockfd, (INT8*) sendtoBuffer,
						sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
						(struct sockaddr *) &recvAddr, tmp_server_addr_len);
				break;
			}
		}
		if (subItems->getEachItemUpResult()) {
			upDSPProduct->setUpResult(true);
			cout << "all true" << endl;
			memset(replyText, 0, msgLen);
			memset(sendtoBuffer, 0, SendBufferSizeMax);
			if (upDSPProduct->modifyVersionFile() != retOk) {
				retUpStatus = retError;
				sprintf(replyText, "Upgrade failed !");
			} else {
				retUpStatus = retOk;
				sprintf(replyText, "Upgrade successed !");
			}
			if (HandleUp::devReplyHandle<DEV_Request_UpgradeReply>(sendtoBuffer,
					*upgradeReply.get(), replyText, retUpStatus,
					setNetworkTerminal) == retOk) {
			}
			sendto(sockfd, (INT8*) sendtoBuffer,
					sizeof(PC_DEV_Header) + upgradeReply->header.DataLen, 0,
					(struct sockaddr *) &recvAddr, tmp_server_addr_len);
#if 0
			upFileAttrs->setInUpgradeStatus(false);
#endif
			fileTrans.clearFileTrans();
			return;
		}

	}
}
INT32 HandleUp::devSearchHandle(DEV_Reply_GetDevMsg & devMsg,
		DevSearchTerminal * devSearch) {

	devMsg = *devSearch->getDevMsg(pathXml, pathVersionFile);
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
		upFileAttr.setInUpgradeStatus(true);
	}
	devReply.header.HeadCmd = 0x0003;
	if (retUpStatus == higherVerison) {
		status = retOk;
		strcpy(failReason, "Upgrading 0%");
	} else if (retUpStatus == lowerVersion) {
		status = retError;
		strcpy(failReason, "Upgrade version Error !");
	} else if (retUpStatus == equalVersion) {
		status = retError;
		strcpy(failReason, "No need to upgrade !");
	} else if (retUpStatus == errorVersionStatus) {
		status = retError;
		strcpy(failReason, "Upgrade operation Error !");
	}
	if (devReplyHandle<DEV_Reply_DevUpgrade>(sendtoBuff, devReply, failReason,
			status, setNetworkTerminal) == retOk) {
	}

	return status;
}

INT32 HandleUp::devRequestFileInit(DEV_Request_FileProtocal & request,
		UpFileAttrs & upFileAttr, FileTrans & fileTrans) {
	DEV_Request_FileProtocal mDevRequest;
	request.header.HeadTag = 0x0101FBFC;
	request.header.HeadCmd = 0x0004;
	request.header.DataLen = devTypeSize + hardVersionSize + softVersionSize
			+ sizeof(mDevRequest.StartPosition)
			+ sizeof(mDevRequest.FileDataLen);
	memcpy(request.DevType, TerminalDevType, strlen(TerminalDevType));
	memcpy(request.HardVersion, TerminalHardVersion,
			strlen(TerminalHardVersion));
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
