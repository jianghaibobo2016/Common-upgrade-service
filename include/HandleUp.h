#ifndef HANDLEUP_H
#define HANDLEUP_H
#include "DevSearch.h"
#include "UpFileAttrs.h"
#include "FileTrans.h"
class UDPNetTrans;
class HandleUp {
public:
	HandleUp();
	~HandleUp();

	static HandleUp &getInstance();

	void devSearchCMDHandle(sockaddr_in recvAddr,
			SetNetworkTerminal *setNetworkTerminal, INT32 sockfd);
	void devParamSetCMDHandle(sockaddr_in recvAddr, INT8 *recvBuff,
			SetNetworkTerminal *setNetworkTerminal, INT32 sockfd);
	void devUpgradePCRequestCMDHandle(sockaddr_in recvAddr, INT8 *recvBuff,
			SetNetworkTerminal *setNetworkTerminal, INT32 sockfd,
			UpFileAttrs &upFileAttr, FileTrans &fileTrans,
			DEV_Request_FileProtocal *request);
	void devFileTransCMDHandle(sockaddr_in recvAddr, INT8 *recvBuff,
			SetNetworkTerminal *setNetworkTerminal, INT32 sockfd,
			UpFileAttrs &upFileAttr, FileTrans &fileTrans,
			DEV_Request_FileProtocal *request);

	static INT32 devSearchHandle(DEV_Reply_GetDevMsg &, DevSearchTerminal *);

	static INT32 setNetworkHandle(INT8 *recvBuff, INT8 *sendtoBuff,
			DEV_Reply_ParameterSetting &,
			SetNetworkTerminal *setNetworkTerminal);

	static INT32 upgradePCrequestHandle(INT8 *recvBuff, INT8 *sendtoBuff,
			DEV_Reply_DevUpgrade &devReply, UpFileAttrs &upFileAttr,
			SetNetworkTerminal *setNetworkTerminal);

	static INT32 devRequestFileInit(DEV_Request_FileProtocal &request,
			UpFileAttrs &upFileAttr, FileTrans &fileTrans);

	static INT32 devRequestFile(DEV_Request_FileProtocal &request,
			FileTrans &fileTrans);

	template<typename T>
	static INT32 writeFileFromPC(INT8 *recvBuff, const INT8 *fileName);

	template<typename T>
	static INT32 devReplyHandle(INT8 *sendtoBuff, T &, INT8 *failReason,
			INT32 result, SetNetworkTerminal *setNetworkTerminal);
private:
};
#include "HandleUp.hpp"
#endif /* HANDLEUP_H */
