/*
 * HandleUp.h
 *
 *  Created on: Mar 23, 2018
 *      Author: jhb
 */
#ifndef HANDLEUP_H
#define HANDLEUP_H
#include "DevSearch.h"
#include "UpFileAttrs.h"
#include "FileTrans.h"
#include "UpdateProgram_Protocal.h"
#include "Mutex.h"
#include "UDPNetTrans.h"
#pragma pack(push)
#pragma pack(1)

class HandleUp {
	static Mutex mutex;
	static struct timeval start;
public:
	HandleUp();
//	HandleUp(UDPNetTrans &udpNet);
	~HandleUp();

	HandleUp(const HandleUp &handle);
//	HandleUp &operator=(const HandleUp &handle);

//	static HandleUp &getInstance();
	/**************************mainly Handle function ************************/
	void devSearchCMDHandle(sockaddr_in recvAddr,
			SetNetworkTerminal *setNetworkTerminal, UpFileAttrs &upFileAttr,
			INT32 sockfd);
	void devParamSetCMDHandle(sockaddr_in recvAddr, INT8 *recvBuff,
			SetNetworkTerminal *setNetworkTerminal, INT32 sockfd);
	void devUpgradePCRequestCMDHandle(sockaddr_in &recvAddr, INT8 *recvBuff,
			SetNetworkTerminal *setNetworkTerminal, INT32 &sockfd,
			UpFileAttrs &upFileAttr, FileTrans &fileTrans,
			DEV_Request_FileProtocal *request);
	void devFileTransCMDHandle(sockaddr_in &recvAddr, INT8 *recvBuff,
			SetNetworkTerminal *setNetworkTerminal, INT32 &sockfd,
			UpFileAttrs &upFileAttr, FileTrans &fileTrans,
			DEV_Request_FileProtocal *request);
	void devGetMaskCMDHandle(sockaddr_in &recvAddr,
			SetNetworkTerminal *setNetworkTerminal, INT32 &sockfd);
	void devTestModeCntCMDHandle(INT8 *recvBuff, sockaddr_in &recvAddr,
			SetNetworkTerminal *setNetworkTerminal, INT32 &sockfd);
	void devGetVersionCMDHandle(INT32 &sockfd, sockaddr_in recvAddr);
	void devGetCastModeCMDHandle(sockaddr_in &recvAddr,
			SetNetworkTerminal *setNetworkTerminal, INT32 &sockfd);
	/**************************mainly Handle function ************************/

	/**************************thread function ************************/
//	static void *TransUpgradeThreadFun(void *args);
	static void *UpgradeThreadFun(void *args);
	/**************************thread function ************************/

	static INT32 devSearchHandle(DEV_Reply_GetDevMsg &, DevSearchTerminal *);

	INT32 setNetworkHandle(INT8 *recvBuff, DEV_Reply_ParameterSetting &,
			SetNetworkTerminal *setNetworkTerminal, INT32 sockfd,
			sockaddr_in recvAdd);

	INT32 upgradePCrequestHandle(INT8 *recvBuff, INT8 *sendtoBuff,
			DEV_Reply_DevUpgrade &devReply, UpFileAttrs &upFileAttr,
			SetNetworkTerminal *setNetworkTerminal);

	static INT32 devRequestFileInit(DEV_Request_FileProtocal &request,
			UpFileAttrs &upFileAttr, FileTrans &fileTrans);

	static INT32 devRequestFile(DEV_Request_FileProtocal &request,
			FileTrans &fileTrans);

	static INT32 upMainRootfsRespond(INT32 m_socket, SetNetworkTerminal &net);

	static void sysReboot();

	template<typename T>
	static INT32 writeFileFromPC(INT8 *recvBuff, const INT8 *fileName);

	template<typename T>
	static INT32 devReplyHandle(INT8 *sendtoBuff, T &, UINT32 reasonLen,
			const INT8 *failReason, INT32 result,
			SetNetworkTerminal *setNetworkTerminal);

	bool getInUpgrade() {
		return inUpgrade;
	}

	bool getInFileTrans() {
		return inFileTrans;
	}

	UDPNetTrans &getUDPNetTransInstance() {
		return _udpNet;
	}

	UINT32 getFileTransStartTime() {
		return _fileTransStartTime.tv_sec;
	}
	void setInUpgrade(const bool status) {
		inUpgrade = status;
	}
	void setInFileTrans(const bool status) {
		inFileTrans = status;
	}

	template<typename T>
	static INT32 localUpHandle(T &);

	static INT32 getLoaclMaskFile(UINT16 *mask);

private:

	UDPNetTrans _udpNet;
	static bool inUpgrade;
	static bool inFileTrans;
	map<INT32, DEV_MODULES_TYPE> devModuleToUpgrade;
	struct timeval _fileTransStartTime;

	static INT32 upTerminalDevs(UPDATE_DEV_TYPE type, INT32 &sockfd,
			sockaddr_in &addr, SetNetworkTerminal *setNet);
	static INT32 executeDevModuleUp(const INT8 *module,
			map<INT32, string> &mExtract);

	INT32 getMaskInfo(UINT16 *mask);

	void inUpgradingMsgSend(sockaddr_in recvAddr,
			SetNetworkTerminal *setNetworkTerminal, INT32 sockfd);

	double diffTimeval(const struct timeval *end, const struct timeval *start);

};
class UDPNetTrans;

typedef struct FileTransArgs_S_ {
	sockaddr_in recvAddr;
	SetNetworkTerminal *setNetworkTerminal;
	INT32 *sockfd;
	UpFileAttrs *upFileAttr;
	FileTrans *fileTrans;
	HandleUp *handle;
//	UDPNetTrans *udpNet;
} FileTransArgs;
#pragma pack(pop)
#include "HandleUp.hpp"
#endif /* HANDLEUP_H */
