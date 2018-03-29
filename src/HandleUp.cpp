#include "HandleUp.h"
#include "UpgradeServiceConfig.h"
#include "CMDParserUp.h"
#include "Upgrade.h"

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
	int tmp_server_addr_len = sizeof(struct sockaddr_in);
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
	cout << "check setting " << endl;

//	DEV_Reply_ParameterSetting *devReplyParaSetting =
//			new DEV_Reply_ParameterSetting();
	INT8 failReason[128] = { 0 };

	INT8 retSetNet = 0;
	retSetNet = CMDParserUp::parserPCSetNetCMD(recvBuff, setNetworkTerminal,
			*failReason);
	devReplySetPara.header.HeadCmd = 0x0002;
	if (devReplyHandle<DEV_Reply_ParameterSetting>(sendtoBuff, devReplySetPara,
			failReason, retSetNet, setNetworkTerminal) == retOk) {

//		cout << "check s3 " << endl;
//		UDPNetTrans mSend;
//		if (socketSendto(sendtoBuffer,
//				sizeof(PC_DEV_Header) + devReplySetPara.header.DataLen,
//				recvSendAddr) <= 0) {
//			cout << "sendto error" << endl;
//		}
//	} else {
//		cout << "errorsetnet " << endl;
//	}
//	delete[] failReason;
//	delete[] sendtoBuffer;
		cout << "check s3 " << endl;
	}
	return retOk;
}

INT32 HandleUp::devSearchHandle(DEV_Reply_GetDevMsg &devMsg,
		DevSearchTerminal *devSearch) {

	devMsg = *devSearch->getDevMsg(pathXml, pathVersionFile);
	return retOk;
}
INT32 HandleUp::upgradePCrequestHandle(INT8 *recvBuff, INT8 *sendtoBuff,
		DEV_Reply_DevUpgrade &devReply, UpFileAttrs &upFileAttr,
		SetNetworkTerminal *setNetworkTerminal) {
	INT8 failReason[128] = { 0 };
	INT32 retUpStatus = 0;
	retUpStatus = CMDParserUp::parserPCUpgradeCMD(recvBuff, upFileAttr,
			failReason);
	cout << "upstatus : " << retUpStatus << endl;

	devReply.header.HeadCmd = 0x0003;
	if (retUpStatus == higherVerison) {
		retUpStatus = retOk;
		strcpy(failReason, "Upgrading 0%");
	} else if (retUpStatus == lowerVersion) {
		retUpStatus = retError;
		strcpy(failReason, "Upgrade version Error !");
	} else if (retUpStatus == equalVersion) {
		retUpStatus = retError;
		strcpy(failReason, "No need to upgrade !");
	} else if (retUpStatus == errorVersionStatus) {
		retUpStatus = retError;
		strcpy(failReason, "Upgrade operation Error !");
	}
	cout << "failReason : no need : " << failReason << endl;
	if (devReplyHandle<DEV_Reply_DevUpgrade>(sendtoBuff, devReply, failReason,
			retUpStatus, setNetworkTerminal) == retOk) {
		cout << "handle ok! " << endl;
	}

	return retUpStatus;
}

INT32 HandleUp::devRequestFileInit(DEV_Request_FileProtocal &request,
		UpFileAttrs &upFileAttr, FileTrans &fileTrans) {
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
	cout << "request len : " << request.FileDataLen << __FUNCTION__ << endl;
	return retOk;
}

INT32 HandleUp::devRequestFile(DEV_Request_FileProtocal &request,
		FileTrans &fileTrans) {
	request.StartPosition = fileTrans.getStartPos();
	request.FileDataLen = fileTrans.getSendLen();
	cout << "request len : " << request.FileDataLen << __FUNCTION__ << endl;
	return retOk;
}
