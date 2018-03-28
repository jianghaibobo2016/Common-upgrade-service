#include "HandleUp.h"
#include "UpgradeServiceConfig.h"
#include "CMDParserUp.h"
#include "Upgrade.h"

HandleUp::HandleUp() {
}
HandleUp::~HandleUp() {
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
	cout << "headtag  " << devMsg.header.HeadTag << endl;
	cout << "headtasearchg  "
			<< devSearch->getDevMsg(pathXml, pathVersionFile)->header.HeadTag
			<< endl;
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
	} else if (retUpStatus == equalVersion){
		retUpStatus = retError;
		strcpy(failReason, "No need to upgrade !");
	}else if (retUpStatus == errorVersionStatus){
		retUpStatus = retError;
		strcpy(failReason, "Upgrade operation Error !");
	}
	if (devReplyHandle<DEV_Reply_DevUpgrade>(sendtoBuff, devReply, failReason,
			retUpStatus, setNetworkTerminal) == retOk) {
		cout << "handle ok! " << endl;
	}

	return retOk;
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
	cout << "request len : "<< request.FileDataLen <<__FUNCTION__<<endl;
	return retOk;
}

INT32 HandleUp::devRequestFile(DEV_Request_FileProtocal &request,
		FileTrans &fileTrans) {
	request.StartPosition = fileTrans.getStartPos();
	request.FileDataLen = fileTrans.getSendLen();
	cout << "request len : "<< request.FileDataLen <<__FUNCTION__<<endl;
	return retOk;
}
