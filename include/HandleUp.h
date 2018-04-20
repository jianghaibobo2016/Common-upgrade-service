#ifndef HANDLEUP_H
#define HANDLEUP_H
#include "DevSearch.h"
#include "UpFileAttrs.h"
#include "FileTrans.h"
#include "UpdateProgram_Protocal.h"
#include "Mutex.h"
#pragma pack(push)
#pragma pack(1)

class HandleUp {
	static Mutex mutex;
public:
	HandleUp();
	~HandleUp();

	HandleUp(const HandleUp &handle);
	HandleUp &operator=(const HandleUp &handle);

	static HandleUp &getInstance();
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
	void devTestModeCntCMDHandle(INT8 *recvBuff);
	void devGetVersionCMDHandle(INT32 &sockfd, sockaddr_in recvAddr);
	/**************************mainly Handle function ************************/

	/**************************thread function ************************/
//	static void *TransUpgradeThreadFun(void *args);
	static void *UpgradeThreadFun(void *args);
	/**************************thread function ************************/

	static INT32 devSearchHandle(DEV_Reply_GetDevMsg &, DevSearchTerminal *);

	static INT32 setNetworkHandle(INT8 *recvBuff, INT8 *sendtoBuff,
			DEV_Reply_ParameterSetting &,
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
	void setInUpgrade(bool status) {
//		mutex.Lock();
		inUpgrade = status;
//		mutex.Unlock();
	}

private:

	static bool inUpgrade;

	template<typename T>
	static INT32 localUpHandle(T &);

	static INT32 upTerminalDevs(UPDATE_DEV_TYPE type);

	INT32 getMaskInfo(UINT16 *mask);
};
//typedef struct FileTransArgs_S_ {
//	FileTransArgs_S_(sockaddr_in addr, SetNetworkTerminal *setNet, INT32 *fd,
//			UpFileAttrs *fileAttr, FileTrans *file) :
//			recvAddr(addr), setNetworkTerminal(setNet), sockfd(fd), upFileAttr(
//					fileAttr), fileTrans(file) {
//	}
class UDPNetTrans;
typedef struct FileTransArgs_S_ {
	sockaddr_in recvAddr;
	SetNetworkTerminal *setNetworkTerminal;
	INT32 *sockfd;
	UpFileAttrs *upFileAttr;
	FileTrans *fileTrans;
	HandleUp *handle;
} FileTransArgs;
#pragma pack(pop)
#include "HandleUp.hpp"
#endif /* HANDLEUP_H */
