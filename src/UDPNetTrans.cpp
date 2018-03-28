#include "UDPNetTrans.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <cstring>
#include <iostream>
#include <Logger.h>

#include "CMDParserUp.h"
#include "DevSearch.h"
#include "GlobDefine.h"
#include "HandleUp.h"
#include "PCTransProtocal.h"
#include "RCSP.h"
#include "UpFileAttrs.h"
#include "FileTrans.h"
#include "md5.h"
#include "FileOperation.h"

using namespace std;
using namespace FrameWork;
UDPNetTrans::UDPNetTrans(SetNetworkTerminal *setNetworkTerminal) :
		NetTrans(UNIXAF, DATAGRAM, 0), setNetworkTerminal(setNetworkTerminal), UDPStatus(
				false) {
	buffer = new INT8[BufferSizeMax];
}
UDPNetTrans::~UDPNetTrans() {
	cout << "udp destory" << endl;
	delete buffer;
	UDPStatus = true;
}
// UDPNetTrans::UDPNetTrans(const UDPNetTrans &)
// {

// }

INT32 UDPNetTrans::socketRunThread() {

	// cout << "check 2" << endl;
	// cout << "2 socket: "<<m_socket<<endl;

	pthread_t tid;
	if (pthread_create(&tid, NULL, pthreadStart, (void *) this) == 0) {
		printf("Create thread unsuccessfully!........\n");
	} else
		;
	return retOk;
}

void *UDPNetTrans::pthreadStart(void *args) {
	// cout << "check 3" << endl;
	/* operator = */
	UDPNetTrans * udp = (UDPNetTrans*) args;
	udp->socketSelect();
	return NULL;
}

INT32 UDPNetTrans::socketSelect() {

	struct sockaddr_in recvSendAddr;
	int tmp_server_addr_len = sizeof(struct sockaddr_in);

	fd_set readfd; //读文件描述符集合
	INT32 ret_select = 0;
	INT32 ret_recv = 0;
	/*  超时 */
	SmartPtr<UpFileAttrs> upFileAttrs = UpFileAttrs::createFileAttrs();
	FileTrans fileTrans;
	SmartPtr<DEV_Request_FileProtocal> request(new DEV_Request_FileProtocal);
	while (!UDPStatus) {

		cout << "socket : " << m_socket << endl;
		struct timeval tv;

		FD_ZERO(&readfd);
		//将套接字文件描述符加入到文件描述符集合中
		FD_SET(m_socket, &readfd);

		ret_select = select(m_socket + 1, &readfd, NULL, NULL, NULL);
		if (ret_select < 0) {
			break;
		} else if (ret_select == 0) {
			printf("timeout\n");
			return retError;
			break;
		}
		//			//接收到数据
		if (FD_ISSET(m_socket, &readfd)) {
			memset(buffer, 0, BufferSizeMax);

			ret_recv = recvfrom(m_socket, buffer, BufferSizeMax, 0,
					(struct sockaddr *) &recvSendAddr,
					(socklen_t *) &tmp_server_addr_len); //recv_addr为服务器端地址
			printf("recvmsg is %s, len: %d\n", buffer, ret_recv);
			if (ret_recv <= 0) {
				break;
			}
			switch (CMDParserUp::parserPCRequestHead(buffer, ret_recv)) {
			case CMD_DEV_SEARCH: {
				DevSearchTerminal devSearchTerminal(setNetworkTerminal);
				SmartPtr<DEV_Reply_GetDevMsg> tmpReMsg(new DEV_Reply_GetDevMsg);
				HandleUp::devSearchHandle(*tmpReMsg.get(), &devSearchTerminal);
				cout << "headtasearchgudp  "
						<< devSearchTerminal.devReplyMsg->header.HeadTag
						<< endl;
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
				sendto(m_socket, tmpReMsg.get(), sizeof(DEV_Reply_GetDevMsg), 0,
						(struct sockaddr *) &recvSendAddr, tmp_server_addr_len);
				printf("found clint IP is:%s\n",
						inet_ntoa(recvSendAddr.sin_addr));
				/* sendto */

			}/*end case 1*/
				break;
			case CMD_DEV_PARAMETER_SETTING: {
				SmartPtr<DEV_Reply_ParameterSetting> devReplyParaSetting(
						new DEV_Reply_ParameterSetting);
				INT8 sendtoBuffer[BufferSizeMax] = { 0 };
				HandleUp::setNetworkHandle(buffer, sendtoBuffer,
						*devReplyParaSetting.get(), setNetworkTerminal);
				if (socketSendto(sendtoBuffer,
						sizeof(PC_DEV_Header)
								+ devReplyParaSetting->header.DataLen,
						recvSendAddr) <= 0) {
					cout << "sendto1 error" << endl;
				} else {
					cout << "errorsetnet 1 " << endl;
				}
			}/*end case 2*/
				break;
			case CMD_DEV_UPGRADE: {
				INT8 sendtoBuffer[BufferSizeMax] = { 0 };
				SmartPtr<DEV_Reply_DevUpgrade> devReply(
						new DEV_Reply_DevUpgrade);
				HandleUp::upgradePCrequestHandle(buffer, sendtoBuffer,
						*devReply.get(), *upFileAttrs.get(),
						setNetworkTerminal);

				if (socketSendto(sendtoBuffer,
						sizeof(PC_DEV_Header) + devReply->header.DataLen,
						recvSendAddr) <= 0) {
					cout << "sendto 2error" << endl;
				} else {
					cout << "errorsetnet 2 " << endl;
				}
				fileTrans.clearFileTrans();
				fileTrans.iniPosition(*upFileAttrs.get());
				cout << "first request sendlen : " << fileTrans.getSendLen()
						<< endl;
				memset(request.get(), 0, sizeof(DEV_Request_FileProtocal));
//							cout << "test 4! " << endl;
				if (HandleUp::devRequestFileInit(*request.get(),
						*upFileAttrs.get(), fileTrans) == retOk) {
//								cout << "test 5! " << endl;
					if (socketSendto((INT8*) request.get(),
							sizeof(PC_DEV_Header) + request->header.DataLen,
							recvSendAddr) <= 0) {
						cout << "sendto 3error" << endl;
					} else {
						cout << "errorsetnet 3 " << endl;
					}
//								cout << "test 6! " << endl;
				} /*end send if*/
				ofstream f(upFileAttrs->getFileDownloadPath(), ios::trunc);
				f.close();
			} /*end case 3*/
				break;
			case CMD_DEV_FILE_TRANSPORT: {
				SmartPtr<DEV_Request_UpgradeReply> upgradeReply(
						new DEV_Request_UpgradeReply);
				INT8 replyText[msgLen] = { 0 };
				INT8 sendtoBuffer[BufferSizeMax] = { 0 };
				INT32 retUpStatus = retOk;
				HandleUp::writeFileFromPC<PC_Reply_FileProtocal>(buffer,
						upFileAttrs->getFileDownloadPath());
				fileTrans.changeRemainedPos().setPersentage();
				if (HandleUp::devRequestFile(*request.get(), fileTrans)
						== retOk) {
//							cout << "trans 3 !" << endl;
					if (socketSendto((INT8*) request.get(),
							sizeof(PC_DEV_Header) + request->header.DataLen,
							recvSendAddr) <= 0) {
						cout << "sendto 3error" << endl;
					} else {
						cout << "errorsetnet 4  " << endl;
					}/*end send if*/
				}
				if (fileTrans.getNewPercent() > fileTrans.getOldPercent()) {
					cout << "fffffffffffffffffffffffffffffffffffffff" << endl;
//							cout << "per: : " << fileTrans.getNewPercent()
//									<< "old:: " << fileTrans.getOldPercent()
//									<< endl;
					upgradeReply->header.HeadCmd = 0x0005;

					memset(replyText, 0, msgLen);
					sprintf(replyText, "Upgrading %u%%.",
							fileTrans.getNewPercent());
					cout << "replytext : " << replyText << endl;
					if (HandleUp::devReplyHandle<DEV_Request_UpgradeReply>(
							sendtoBuffer, *upgradeReply.get(), replyText,
							retUpStatus, setNetworkTerminal) == retOk) {
					}
					if (socketSendto((INT8*) sendtoBuffer,
							sizeof(PC_DEV_Header)
									+ upgradeReply->header.DataLen,
							recvSendAddr) <= 0) {
						cout << "sendto 3error" << endl;
					} else {
						cout << "errorsetnet 5  " << endl;
					}/*end send if*/
				}
				fileTrans.setOldPercent(fileTrans.getNewPercent());
				if (0 == fileTrans.getFileRemainedLen()) {
					UINT8 md5_str[MD5_SIZE];
					if (!GetFileMD5(upFileAttrs->getFileDownloadPath(),
							md5_str)) {
						cout << "get file md5 error" << endl;
					}
					int iret = memcmp(upFileAttrs->getFileMD5Code(), md5_str,
					MD5_SIZE);
					memset(replyText, 0, msgLen);
					if (iret != 0) {
						retUpStatus = retError;
						cout << "MD5 error" << endl;
						sprintf(replyText, "File error, please try again !");
//						return false;
					} else {
						retUpStatus = retOk;
						sprintf(replyText, "Upgrading 55%%.");
						cout << "ok md5!" << endl;
					}
					cout << "replytext : " << replyText << endl;
					if (HandleUp::devReplyHandle<DEV_Request_UpgradeReply>(
							sendtoBuffer, *upgradeReply.get(), replyText,
							retUpStatus, setNetworkTerminal) == retOk) {
					}
					if (socketSendto((INT8*) sendtoBuffer,
							sizeof(PC_DEV_Header)
									+ upgradeReply->header.DataLen,
							recvSendAddr) <= 0) {
						cout << "sendto 3error" << endl;
					} else {
						cout << "errorsetnet 5  " << endl;
					}/*end send if*/
					if (retUpStatus == retError)
						break;
					/*next step*/
					SmartPtr<UpgradeDSP> upDSPProduct(
							new UpgradeDSP(upFileAttrs->getFileDownloadPath()));
					cout << "file : out par paskc 1 : "
							<< upDSPProduct->getUpgradeFile() << endl;
					if (upDSPProduct->parserFileName() == 0) {
						cout << "up 1 !" << endl;
						cout << "file : out par paskc 2 : "
								<< upDSPProduct->getUpgradeFile() << endl;
						if (upDSPProduct->parserItemPackage() == 0) {
							cout << "up 2 !" << endl;
						}
						cout << "up 3 !" << endl;
					}
					cout << "version 1: " << upDSPProduct->getNewVersion()
							<< endl;
					SmartPtr<UpgradeDSPSubItem> subItems(new UpgradeDSPSubItem);
//								break;
					if (subItems->getSubItems() == true) {
						cout << "up 4 !" << endl;
//						if (subItems->parserSubItemsFileName(1) == 0) {
//							cout << "vvvv00002: "
//									<< const_cast<UpgradeDSP*>(&subItems->getUpObj())->getNewVersion()
//									<< endl;
//							cout << "up 5 !" << endl;
//						}
						cout << "up 6 !" << endl;
					}

					UINT32 itemsNum = subItems->getItemsNum();
					cout << "item num : " << itemsNum << endl;
					//2 ->1
					for (UINT32 i = 1; i <= itemsNum; i++) {
						cout << "up 7 !" << endl;
						/////////////////////
						if (subItems->parserSubItemsFileName(i) == 0) {
							cout << "vvvv00002: "
									<< const_cast<UpgradeDSP*>(&subItems->getUpObj())->getNewVersion()
									<< endl;
							cout << "up 5 !" << endl;
						}
						/////////////////////
						if (subItems->upgradeItem(i) == 0) {
							cout << "up 7.1 !" << endl;
							if (FileOperation::extractTarFile(newTarPackage,
									subItems->getExtractItem()) != true) {
								cout << "up 7.2" << endl;

							}
							if (subItems->excuteUpgradeShell(i) == 0) {
								if (subItems->modifyVersionFile() == retOk) {
									const_cast<UpgradeDSP*>(&subItems->getUpObj())->clearObj();
									cout << "modify ok" << endl;
								} else
									break;
								cout << "ok" << endl;
							}
							cout << "up 8 !" << endl;
						}
						cout << "up 9 !" << endl;
					}
					cout << "version 2: " << upDSPProduct->getNewVersion()
							<< endl;
					if (subItems->getEachItemUpResult()) {
						upDSPProduct->setUpResult(true);
						cout << "all true" << endl;
						upDSPProduct->modifyVersionFile();
					}
				}/*end of next step*/

			}/*end case 4*/
				break;
			case CMD_DEV_UPGRADE_REPLY: {

			}
				break;
			case CMD_WEB_REQUEST_UPGRADE: {
				cout << "webUpg" << endl;
			}
				break;
			default:
				break;
			}

		} /* end if FD_ISSET */
//		} /* end if */
		// break;
	} /* end while(1) */
	return retOk;
}

INT32 UDPNetTrans::socketSendto(INT8 *sendtoBuff, size_t bufferLen,
		sockaddr_in clientAddr) {
	return sendto(m_socket, sendtoBuff, bufferLen, 0,
			(struct sockaddr *) &clientAddr, sizeof(struct sockaddr_in));
}
