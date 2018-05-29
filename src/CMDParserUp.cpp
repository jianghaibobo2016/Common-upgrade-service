/*
 * CMDParser.cpp
 *
 *  Created on: 2018年4月16日
 *      Author: JHB
 */
#include "CMDParserUp.h"
#include "UpgradeServiceConfig.h"
#include "GlobalProfile.h"
#include "FileOperation.h"
#include "DevSearch.h"
#include "Logger.h"

#include "LocalUDPTrans.h"
#include "HandleUp.h"

using namespace FrameWork;

#define OFFSETPTR                                                                   \
    pcSettingConfig += (1 + (INT32)pcSettingConfig[0]);
#define COPYIP                                                                      \
    OFFSETPTR                                                                       \
    memcpy(netConfigTrans.ipT, pcSettingConfig + 1, (INT32)pcSettingConfig[0]);

#define COPYSUBMASK                                                                 \
    OFFSETPTR                                                                       \
    memcpy(netConfigTrans.submaskT, pcSettingConfig + 1, (INT32)pcSettingConfig[0]);

#define COPYGATEWAY                                                                 \
    OFFSETPTR                                                                       \
    memcpy(netConfigTrans.gatewayT, pcSettingConfig + 1, (INT32)pcSettingConfig[0]);

#define COPYSERVERIP                                                                \
    OFFSETPTR                                                                       \
    memcpy(netConfigTrans.serverIPT, pcSettingConfig + 1, (INT32)pcSettingConfig[0]);

#define COPYSERVERPORT                                                              \
    OFFSETPTR                                                                       \
    memcpy(netConfigTrans.serverPortT, pcSettingConfig + 1, (INT32)pcSettingConfig[0]);

#define COPYMAC                                                               	    \
    OFFSETPTR                                                                       \
    memcpy(netConfigTrans.MAC, pcSettingConfig + 1, (INT32)pcSettingConfig[0]);

#define COPYMASK                                                                    \
    OFFSETPTR                                                                       \
    memcpy(netConfigTrans.MASK, pcSettingConfig + 1, (INT32)pcSettingConfig[0]);
CMDParserUp::CMDParserUp() :
		settingNum(0) {
}
CMDParserUp::~CMDParserUp() {
}
UINT16 CMDParserUp::parserPCRequestHead(void *buffer, INT32 recvLen) {

	PC_DEV_Header *pcHead = (PC_DEV_Header *) buffer;

	printf("Recv CMD %d with tag %X from PC !\n", pcHead->HeadCmd,
			pcHead->HeadTag);
	if (pcHead->HeadTag != PROTOCAL_PC_DEV_HEAD) {
		return (UINT16) retError;
	}
	/* modify */
	cout << "datalen: " << pcHead->DataLen << " recvLen: " << recvLen << endl;
	NetTrans::printBufferByHex("recv dev : ", buffer, recvLen);
	if ((pcHead->DataLen != recvLen - sizeof(PC_DEV_Header))
			&& (pcHead->DataLen != 0)) {
		return (UINT16) retError;
	}
	UINT16 headCMD = pcHead->HeadCmd;
	return headCMD;
}

/******************************************************************************
 * Description :
 Set net or settings after parsering CMD of params setting.
 * Return Value : On success, it return 0. On error, -1 is returned.
 * Author : JHB
 * Create Data : 04/01/2018
 * Revision History :
 *   04/01/2018  JHB    Created.
 *****************************************************************************/
//name：Name（设备名称）
//value：60B（utf8，包含结束符）
//name：IP
//value： xxx.xxx.xxx.xxx
//name：SubMask
//value：xxx.xxx.xxx.xxx
//name：GateWay
//value：xxx.xxx.xxx.xxx
//name：ServerIP
//value：xxx.xxx.xxx.xxx
//name：CommunicationPort
//value：xxxx (7001~7100)
//name：RecordingPort
//value：xxxx (7101~7200)
//name：Mac
//value:aaaabbbbcccc
//name：Mask
//value:1111FFFF1111FFFF
INT32 CMDParserUp::parserPCSetNetCMD(void *buffer, SetNetworkTerminal *net,
		map<string, string> &retContent) {
	INT8 *pcSettingConfig = (INT8 *) buffer;
	pcSettingConfig += sizeof(PC_DEV_Header);
	INT8 mac[13] = { 0 };
	strcpy(mac,
			SetNetworkTerminal::castMacToChar13(mac,
					net->getNetConfStruct().macAddr));
	mac[12] = '\0';
	INT8 devID[40] = { 0 };
	strncpy(devID, TerminalDevTypeID, strlen(TerminalDevTypeID));
	strcpy(devID + strlen(TerminalDevTypeID), mac);

	if (strncmp(pcSettingConfig, devID, strlen(devID)) != 0) {
		Logger::GetInstance().Info("Recv params setting cmd with devID : %s .",
				devID);
		return INVALID;
	} else
		pcSettingConfig += devIDSize;

	UINT8 parameterNum = pcSettingConfig[0];
	pcSettingConfig += 1;
	NetConfigTransWithServer netConfigTrans;
	InitSetConf initConfigTrans;
	if (parameterNum == Terminal9903Num) {
		if (obtainParams<NetConfigTransWithServer>(pcSettingConfig,
				netConfigTrans, Terminal9903Num) != true
				|| netConfigTrans.getFlag() != 28) {
			Logger::GetInstance().Error(
					"Obtain parameters failed with flag %d !",
					netConfigTrans.getFlag());
			return retError;
		}
		SmartPtr<UP_PROG_SET_CONF> serverConf(new UP_PROG_SET_CONF);
		if (setParams(net, netConfigTrans, *serverConf.get(), Terminal9903Num,
				retContent) == false){
			return retError;
		}
		else
			return retOk;

	} else if (parameterNum == TerminalWithoutRcdPNum) {
		if (obtainParams<NetConfigTransWithServer>(pcSettingConfig,
				netConfigTrans, TerminalWithoutRcdPNum) != true
				|| netConfigTrans.getFlag() != 22) {
			Logger::GetInstance().Error(
					"Obtain parameters failed with flag %d !",
					netConfigTrans.getFlag());
			return retError;
		}
		SmartPtr<UP_PROG_SET_CONF> serverConf(new UP_PROG_SET_CONF);
		if (setParams(net, netConfigTrans, *serverConf.get(),
				TerminalWithoutRcdPNum, retContent) == false)
			return retError;
		else
			return retOk;
	} else if (parameterNum == TermianlInitNum) {
		if (obtainParams<InitSetConf>(pcSettingConfig, initConfigTrans,
				TermianlInitNum) != true || initConfigTrans.getFlag() != 3) {
			Logger::GetInstance().Error(
					"Obtain parameters failed with flag %d !",
					initConfigTrans.getFlag());
			return retError;
		}
		NetTrans::printBufferByHex("recv mask before set params: ",
				const_cast<INT8 *>(initConfigTrans.getMASK()), 8);
		if (setParams(net, initConfigTrans, TermianlInitNum, retContent))
			return retOk;
		else
			return retError;
	} else {
		Logger::GetInstance().Error("Parameters' number : %d is incorrect !",
				(INT32) parameterNum);
	}

#if 0
	pcSettingConfig += 1;
	{
//		NetConfigTransWithServer netConfigTrans;
		UINT16 port = 0;
		GlobalProfile setServerConf;
		switch (parameterNum) {
			case 0x01:
			if (campareNetSetMatch(&pcSettingConfig[0], pcSettingConfig + 1,
							PCREQUESTMAC) == true) {
//			} else {
				COPYMAC
				;
				string mac;
				for (int i = 0; i < (INT32) pcSettingConfig[0];) {
					mac += netConfigTrans.MAC[i];
					mac += netConfigTrans.MAC[i + 1];
					if (i + 3 < (INT32) pcSettingConfig[0])
					mac += ":";
					i += 2;
				}
				if (setNetworkTerminal->setNetworkConfig(NULL, NULL, NULL,
								netConfigTrans.MAC, INIFILE) != true) {
					retContent[PCREQUESTMAC] = "Set MAC failed !";
					return retError;
				} else {
					retContent[PCREQUESTMAC] = "Set MAC OK !";
				}
			} else if (campareNetSetMatch(&pcSettingConfig[0],
							pcSettingConfig + 1, PCREQUESTMASK) == true) {
				COPYMASK

				vector<UINT16> vHexArray;
				vHexArray.resize(4);
				vHexArray[0] = ~((netConfigTrans.MASK[1] << 8
								| netConfigTrans.MASK[0]) - 1);
				vHexArray[1] = ~((netConfigTrans.MASK[3] << 8
								| netConfigTrans.MASK[2]) - 1);
				vHexArray[2] = ~((netConfigTrans.MASK[5] << 8
								| netConfigTrans.MASK[4]) - 1);
				vHexArray[3] = ~((netConfigTrans.MASK[7] << 8
								| netConfigTrans.MASK[6]) - 1);
				for (int i = 0; i < 4; i++) {
					printf("vHexArray[%d] : %04x\n", i, vHexArray[i]);
				}
				if (writeMaskFile(vHexArray) == 1) {
					retContent[PCREQUESTMASK] = "Get MASK OK !";
				} else
				retContent[PCREQUESTMASK] = "Get MASK failed !";
			} else {
				retContent[PCREQUESTMAC] = "Match failed !";
				return retError;
			}
			break;
			case 0x02:
			if ((INT32) pcSettingConfig[0] == 0x02) {
				retContent[PCREQUESTSERVERIP] = "Match failed !";
				retContent[PCREQUESTCOMMUNICATIONPORT] = "Match failed !";
				return retError;
			} else {
				if (campareNetSetMatch(&pcSettingConfig[0], pcSettingConfig + 1,
								PCREQUESTSERVERIP) != true) {
					retContent[PCREQUESTSERVERIP] = "Match failed !";
					retContent[PCREQUESTCOMMUNICATIONPORT] = "Match failed !";
					return retError;
				} else {
					COPYSERVERIP
					;
				}
				OFFSETPTR
				;
				if (campareNetSetMatch(&pcSettingConfig[0], pcSettingConfig + 1,
								PCREQUESTCOMMUNICATIONPORT) != true) {
					retContent[PCREQUESTSERVERIP] = "Match failed !";
					retContent[PCREQUESTCOMMUNICATIONPORT] = "Match failed !";
					return retError;
				} else {
					COPYSERVERPORT
					;
				}
				sscanf(netConfigTrans.CommunicationPort, "%hu", &port);
				setServerConf.SetTCPCommServerIP(netConfigTrans.serverIPT,
						port);
				retContent[PCREQUESTSERVERIP] = "Set server IP OK!";
				retContent[PCREQUESTCOMMUNICATIONPORT] = "Set server Port OK!";
			}
			break;
			case 0x03:
			if (campareNetSetMatch(&pcSettingConfig[0], pcSettingConfig + 1,
							PCREQUESTIP) != true) {
				retContent[PCREQUESTIP] = "Match IP failed !";
				retContent[PCREQUESTSUBMASK] = "Match submask failed !";
				retContent[PCREQUESTGATEWAY] = "Match gateway failed !";
				return retError;
			} else {
				COPYIP
				;
			}
			OFFSETPTR
			;
			if (campareNetSetMatch(&pcSettingConfig[0], pcSettingConfig + 1,
							PCREQUESTSUBMASK) == true) {
				COPYSUBMASK
				;
				OFFSETPTR
				;
				if (campareNetSetMatch(&pcSettingConfig[0], pcSettingConfig + 1,
								PCREQUESTGATEWAY) == true) {
					COPYGATEWAY
					;
					if (setNetworkTerminal->setNetworkConfig(netConfigTrans.ipT,
									netConfigTrans.submaskT, netConfigTrans.gatewayT,
									NULL, INIFILE) != true) {
						retContent[PCREQUESTIP] = "Set IP failed !";
						retContent[PCREQUESTSUBMASK] = "Set submask failed !";
						retContent[PCREQUESTGATEWAY] = "Set gateway failed !";
						return retError;
					} else {
						retContent[PCREQUESTIP] = "Set IP OK !";
						retContent[PCREQUESTSUBMASK] = "Set submask OK !";
						retContent[PCREQUESTGATEWAY] = "Set gateway OK !";
					}
				} else {
					retContent[PCREQUESTIP] = "Match setting failed !";
					retContent[PCREQUESTSUBMASK] = "Match setting failed !";
					retContent[PCREQUESTGATEWAY] = "Match setting failed !";
					return retError;
				}
				//useless
			} else {
				retContent[PCREQUESTIP] = "Match setting failed !";
				retContent[PCREQUESTSUBMASK] = "Match setting failed !";
				retContent[PCREQUESTGATEWAY] = "Match setting failed !";
				return retError;
			}
			break;
			default:
			return retError;
			break;
		}
	} /* end field */

#endif
	return retOk;
}

/******************************************************************************
 * Description : Parser the upgrade file from PC.
 * Return Value : It returns a status of upgrade file' version. On error, 2 is
 * returned.
 * Author : JHB
 * Create Data : 04/01/2018
 * Revision History :
 *   04/01/2018  JHB    Created.
 *****************************************************************************/
upgradeFileStatus CMDParserUp::parserPCUpgradeCMD(void *buffer,
		UpFileAttrs &upFileAttr, INT8 *failReason,
		map<INT32, DEV_MODULES_TYPE> &devModuleToUpgrade) {
	INT8 *pcRequestBuffer = (INT8 *) buffer;
	pcRequestBuffer += sizeof(PC_DEV_Header);
	INT8 hardVersion[8] = { 0 };
	DevSearchTerminal::getSoftwareVersion("hardware_version", hardVersion,
			pathVersionFile);
	if (compareUpgradeItem(pcRequestBuffer, hardVersion, strlen(hardVersion))
			!= true) {

	}
	pcRequestBuffer += hardVersionSize;
	/* new software version */
	upFileAttr.setNewSoftVersion(pcRequestBuffer, 6);
	INT8 fileDownloadName[fileDownloadPathSize] = { 0 };
	memcpy(fileDownloadName, upFileDownload, strlen(upFileDownload));
	strcat(fileDownloadName, upFileAttr.getNewSoftVersion());
	upFileAttr.setFileDownloadPath(fileDownloadName, strlen(fileDownloadName));
	INT8 version[7] = { 0 };
	DevSearchTerminal::getSoftwareVersion(ProductVersionName, version,
			pathVersionFile);
	/*WEB request to upgrade CMD judgement*/
	if (compareUpgradeItem(
			pcRequestBuffer + strlen(upFileAttr.getNewSoftVersion()),
			FORCEUPGRADE, strlen(FORCEUPGRADE)) == 0) {
		strcat(fileDownloadName, FORCEUPGRADE);
		upFileAttr.setFileDownloadPath(fileDownloadName,
				strlen(fileDownloadName));

		upFileAttr.setForceUpgrade(true);
		if (compareUpgradeItem(
				pcRequestBuffer + strlen(upFileAttr.getNewSoftVersion())
						+ strlen(FORCEUPGRADE), PCREQUEST, strlen(PCREQUEST))
				== 0) {
			pcRequestBuffer += newSoftVersionSize;
			INT32 fileSize = 0;
			memcpy(&fileSize, pcRequestBuffer, sizeof(UINT32));
			upFileAttr.setNewSoftFileSize(fileSize);

			pcRequestBuffer += sizeof(UINT32);
			upFileAttr.setFileMD5Code(pcRequestBuffer, upgradeFileMd5Size);
		} else if (compareUpgradeItem(
				pcRequestBuffer + strlen(upFileAttr.getNewSoftVersion())
						+ strlen(FORCEUPGRADE), WEBREQUEST, strlen(WEBREQUEST))
				== 0) {
			Logger::GetInstance().Info("Get file download with path: %s",
					upFileAttr.getFileDownloadPath());
			if (!FileOperation::isExistFile(upFileAttr.getFileDownloadPath())) {
				strcpy(failReason, UPFILENOTEXIST);
				return errorVersionStatus;
			}
			upFileAttr.setWebUpMethod(true);

		}
	} else if (compareUpgradeItem(
			pcRequestBuffer + strlen(upFileAttr.getNewSoftVersion()),
			WEBREQUEST, strlen(WEBREQUEST)) == 0) {
		Logger::GetInstance().Info("Get file download with path: %s",
				upFileAttr.getFileDownloadPath());
		if (!FileOperation::isExistFile(upFileAttr.getFileDownloadPath())) {
			strcpy(failReason, UPFILENOTEXIST);
			return errorVersionStatus;
		}
		upFileAttr.setWebUpMethod(true);

	} else {
		pcRequestBuffer += newSoftVersionSize;
		INT32 fileSize = 0;
		memcpy(&fileSize, pcRequestBuffer, sizeof(UINT32));
		upFileAttr.setNewSoftFileSize(fileSize);

		pcRequestBuffer += sizeof(UINT32);
		upFileAttr.setFileMD5Code(pcRequestBuffer, upgradeFileMd5Size);

	}
	strcpy(failReason, BeginToUpgrade);
	return higherVerison;

	return errorVersionStatus;
}

INT32 CMDParserUp::isDevModulesUpgradeEnable(
		map<INT32, DEV_MODULES_TYPE> &devModuleToUpgrade,
		map<INT32, DEV_MODULES_TYPE> &devModules, UpFileAttrs & upFileAttr) {
	LocalUDPTrans netTrans;
	UPDATE_GET_DEVSTATUS getDevStatus;
	UINT32 num = 1;
	INT32 validDevModule = 1;
	for (; num <= devModules.size(); num++) {
		getDevStatus.header.HeadCmd = 0x0004;
		getDevStatus.dev_type = devModules[num];
		cout << "sendto servarappp type :: " << getDevStatus.dev_type << endl;
		HandleUp::localUpHandle<UPDATE_GET_DEVSTATUS>(getDevStatus);
		INT32 retSend = sendto(netTrans.getSockfd(), &getDevStatus,
				sizeof(UPDATE_SEND_UPDATEDEV), 0,
				(struct sockaddr*) netTrans.getAddr(), *netTrans.getAddrLen());
		if (retSend <= 0) {
			cout << "errorsend" << endl;
			return retError;
		} else {
		}
		Logger::GetInstance().Info("Ask device type %d is online or not!",
				getDevStatus.dev_type);
		struct timeval timeout = { 8, 0 }; //10s
		setsockopt(netTrans.getSockfd(), SOL_SOCKET, SO_RCVTIMEO,
				(const char *) &timeout, sizeof(timeout));
		INT8 recvBuff[16] = { 0 };
		INT32 retRecv = recvfrom(netTrans.getSockfd(), recvBuff,
				sizeof(recvBuff), 0, (struct sockaddr*) netTrans.getAddr(),
				netTrans.getAddrLen());
		if (retRecv == -1) {
			Logger::GetInstance().Error("Device type %d no recv !",
					getDevStatus.dev_type);
			return retError;
		} else if (retRecv == 0)
			cout << "no recv" << endl;
		else if (retRecv > 0) {
			for (INT32 i = 0; i < retRecv; i++)
				printf("recv : %02x\t", recvBuff[i]);
			ARM_REPLAY_GETDEVSTATUS *devStatus =
					(ARM_REPLAY_GETDEVSTATUS*) recvBuff;
			if (devStatus->header.HeadCmd != CMD_DEV_ONLINE) {
				Logger::GetInstance().Error("Device type %d recv error!",
						getDevStatus.dev_type);
				return retError;
			} else {
				if (devStatus->dev_type != devModules[num]) {
					Logger::GetInstance().Error(
							"Want to get device type %d but recv device type %d !",
							devModules[num], devStatus->dev_type);
					return retError;
				}
				if (upFileAttr.getForceStatus() == true) {
					if (devStatus->state == UPGRADE_VALID
							|| devStatus->state == NONEEDUP) {
						devModuleToUpgrade[validDevModule] =
								(DEV_MODULES_TYPE) devStatus->dev_type;
						validDevModule++;
					}
				} else {
					if (devStatus->state == UPGRADE_VALID) {
						devModuleToUpgrade[validDevModule] =
								(DEV_MODULES_TYPE) devStatus->dev_type;
						cout << "need devs type :::: "
								<< devModuleToUpgrade[validDevModule] << endl;
						validDevModule++;
					}
				}
			}
		}

	}
	map<INT32, DEV_MODULES_TYPE>::iterator iter;
	for (iter = devModuleToUpgrade.begin(); iter != devModuleToUpgrade.end();
			iter++) {
		cout << "up a dev name : " << iter->first << " and status : "
				<< iter->second << endl;
	}
	return retOk;
}

bool CMDParserUp::screeningParams(INT8* name, INT8* value,
		NetConfigTransWithServer &config) {
	if (compareUpgradeItem(name, PCREQUESTIP, strlen(PCREQUESTIP) + 1) == 0) {
		if (config.setIPT(value) != true)
			return false;
	} else if (compareUpgradeItem(name, PCREQUESTSUBMASK,
			strlen(PCREQUESTSUBMASK) + 1) == 0) {
		if (config.setSubmaskT(value) != true)
			return false;
	} else if (compareUpgradeItem(name, PCREQUESTGATEWAY,
			strlen(PCREQUESTGATEWAY) + 1) == 0) {
		if (config.setgatewayT(value) != true)
			return false;
	} else if (compareUpgradeItem(name, PCREQUESTSERVERIP,
			strlen(PCREQUESTSERVERIP) + 1) == 0) {
		if (config.setServerIPT(value) != true)
			return false;
	} else if (compareUpgradeItem(name, PCREQUESTCOMMUNICATIONPORT,
			strlen(PCREQUESTCOMMUNICATIONPORT) + 1) == 0) {
		if (config.setCommunicationPort(value) != true)
			return false;
	} else if (compareUpgradeItem(name, PCREQUESTNAME,
			strlen(PCREQUESTNAME) + 1) == 0) {
		if (config.setName(value) != true)
			return false;
	}
#if (DSP9903)
	else if (compareUpgradeItem(name, PCREQUESTRCORDINGPORT,
			strlen(PCREQUESTRCORDINGPORT) + 1) == 0) {
		UINT16 port = 0;
		sscanf(value, "%hu", &port);
		if (config.setRecordingPort(port) != true)
			return false;
	}
#endif
	else {
		Logger::GetInstance().Error(
				"No matched parameter name : %s with its value : %s!", name,
				value);
		return false;
	}
	return true;
}
bool CMDParserUp::screeningParams(INT8* name, INT8* value,
		InitSetConf &config) {
	if (compareUpgradeItem(name, PCREQUESTMAC, strlen(PCREQUESTMAC) + 1) == 0) {
		if (config.setMAC(value) != true)
			return false;
	} else if (compareUpgradeItem(name, PCREQUESTMASK,
			strlen(PCREQUESTMASK) + 1) == 0) {
		if (config.setMASK(value) != true)
			return false;
	} else {
		Logger::GetInstance().Error(
				"No matched parameter name : %s with its value : %s!", name,
				value);
		return false;
	}
	return true;
}

bool CMDParserUp::setParams(SetNetworkTerminal *net,
		NetConfigTransWithServer &config, UP_PROG_SET_CONF &serverConf,
		INT32 num, map<string, string> &retContent) {
	if ((strncmp(net->getNetConfStruct().ipAddr.c_str(), config.getIPT(),
			strlen(config.getIPT())) != 0)
			|| (strncmp(net->getNetConfStruct().netmaskAddr.c_str(),
					config.getSubmaskT(), strlen(config.getSubmaskT())) != 0)
			|| (strncmp(net->getNetConfStruct().gatewayAddr.c_str(),
					config.getGatewayT(), strlen(config.getGatewayT())) != 0)) {
		cout << "to set net ............" << endl;
		if (net->setNetworkConfig(config.getIPT(), config.getSubmaskT(),
				config.getGatewayT(),
				NULL, INIFILE) != true) {
			cout << "test 8 " << endl;
			retContent[PCREQUESTIP] = "Set IP failed !";
			retContent[PCREQUESTSUBMASK] = "Set submask failed !";
			retContent[PCREQUESTGATEWAY] = "Set gateway failed !";
			retContent[PCREQUESTSERVERIP] = "Set server ip failed !";
			retContent[PCREQUESTCOMMUNICATIONPORT] =
					"Set communication port failed !";
			retContent[PCREQUESTNAME] = "Set terminal name failed !";
#if (DSP9903)
			retContent[PCREQUESTRCORDINGPORT] = "Set recording port failed !";
#endif
			return false;
		} else {
			cout << "test 9 " << endl;
			retContent[PCREQUESTIP] = "Set IP ok !";
			retContent[PCREQUESTSUBMASK] = "Set submask ok !";
			retContent[PCREQUESTGATEWAY] = "Set gateway ok !";
		}
	}

	serverConf.header.HeadCmd = CMD_SET_TERMINAL_CONFIG;
	serverConf.ServerIP = inet_addr(config.getServerIPT());
	UINT16 tmpPort = 0;
	sscanf(config.getCommunicationPort(), "%hu", &tmpPort);
	serverConf.CommunicationPort = tmpPort;
	memcpy(&serverConf.DevName, config.getName(), strlen(config.getName()));

	if (num == Terminal9903Num) {
#if (DSP9903)
		serverConf.RecordingPort = config.getRecordingPort();
#endif
	}

	bool setStatus = true;
	LocalUDPTrans netTrans;
	HandleUp::localUpHandle<UP_PROG_SET_CONF>(serverConf);
	INT32 retSend = sendto(netTrans.getSockfd(), &serverConf,
			sizeof(NetConfigTransWithServer), 0,
			(struct sockaddr*) netTrans.getAddr(), *netTrans.getAddrLen());
	if (retSend <= 0) {
		Logger::GetInstance().Error("Send to Server program failed !");
		setStatus = false;
	} else {
		Logger::GetInstance().Info("Send to Server program %d bytes .",
				retSend);
		INT8 recvBuff[sizeof(SERVER_REPLY_SET_CONF)] = { 0 };
		struct timeval timeout = { 2, 0 }; //2s for waiting
		setsockopt(netTrans.getSockfd(), SOL_SOCKET, SO_RCVTIMEO,
				(const char *) &timeout, sizeof(timeout));
		INT32 retRecv = recvfrom(netTrans.getSockfd(), recvBuff,
				sizeof(SERVER_REPLY_SET_CONF), 0,
				(struct sockaddr*) netTrans.getAddr(), netTrans.getAddrLen());
		if (retRecv == -1) {
			Logger::GetInstance().Error("Recv from Server program error !");
			setStatus = false;
		} else if (retRecv > 0) {
			SERVER_REPLY_SET_CONF *serverReply =
					(SERVER_REPLY_SET_CONF*) recvBuff;
			if (serverReply->header.HeadCmd == CMD_SET_TERMINAL_CONFIG) {
				Logger::GetInstance().Info("Server set params result : %d ",
						(INT32) serverReply->result);
				if (serverReply->result == 1)
					return true;
				else if (serverReply->result == 0)
					setStatus = false;
			} else {
				Logger::GetInstance().Error(
						"Incorrect reply from server program with cmd : %d !",
						serverReply->header.HeadCmd);
				setStatus = false;
			}
		}

	}
	if (setStatus == true) {
		retContent[PCREQUESTSERVERIP] = "Set server ip ok !";
		retContent[PCREQUESTCOMMUNICATIONPORT] = "Set terminal name ok !";
		retContent[PCREQUESTNAME] = "Set terminal name ok !";
		cout << "okkkkkkkkkkkkkkkkkkkkkk" << endl;
#if (DSP9903)
		retContent[PCREQUESTRCORDINGPORT] = "Set recording port ok !";
#endif
	} else {
		retContent[PCREQUESTSERVERIP] = "Set server ip failed !";
		retContent[PCREQUESTCOMMUNICATIONPORT] =
				"Set communication port failed !";
		retContent[PCREQUESTNAME] = "Set terminal name failed !";
#if (DSP9903)
		retContent[PCREQUESTRCORDINGPORT] = "Set recording port failed !";
#endif
		return false;
	}
	return true;
}

bool CMDParserUp::setParams(SetNetworkTerminal *net, InitSetConf &config,
		INT32 num, map<string, string> &retContent) {
	if (strncmp(net->getNetConfStruct().macAddr.c_str(), config.getMAC(),
			strlen(config.getMAC())) != 0) {
		string mac;
		for (int i = 0; i < 13;) {
			mac += config.getMAC()[i];
			mac += config.getMAC()[i + 1];
			if (i + 3 < 13)
				mac += ":";
			i += 2;
		}
		if (net->setNetworkConfig(NULL, NULL, NULL, config.getMAC(), INIFILE)
				!= true) {
			retContent[PCREQUESTMAC] = "Set MAC failed !";
			retContent[PCREQUESTMASK] = "Set MASK failed !";
			return retError;
		} else {
			retContent[PCREQUESTMAC] = "Set MAC OK !";
		}
	}

	NetTrans::printBufferByHex("recv mask : ",
			const_cast<INT8 *>(config.getMASK()), 8);
	vector<UINT16> vHexArray;
	vHexArray.resize(4);
	vHexArray[0] = ~((config.getMASK()[1] << 8 | config.getMASK()[0]) - 1);
	vHexArray[1] = ~((config.getMASK()[3] << 8 | config.getMASK()[2]) - 1);
	vHexArray[2] = ~((config.getMASK()[5] << 8 | config.getMASK()[4]) - 1);
	vHexArray[3] = ~((config.getMASK()[7] << 8 | config.getMASK()[6]) - 1);
	for (int i = 0; i < 4; i++) {
		printf("vHexArray[%d] : %04x\n", i, vHexArray[i]);
	}
	if (writeMaskFile(vHexArray) == 1) {
		retContent[PCREQUESTMASK] = "Set MASK OK !";
	} else
		retContent[PCREQUESTMASK] = "Set MASK failed !";

	return true;
}
#if 0
bool CMDParserUp::getConf(INT8 *buff, NetConfigTransWithServer &config,
		INT32 num) {
	INT8 *tmpBuff = buff;
	UINT32 iPos = 0;
	UINT32 nameLen = 0;
	UINT32 valueLen = 0;
	for (INT32 i = 0; i < num; i++) {
		nameLen = (UINT32) tmpBuff[iPos];
		iPos += 1;
		INT8 name[iPos] = {0};
		memcpy(name, &tmpBuff[iPos], nameLen);
		iPos += nameLen;
		valueLen = (UINT32) tmpBuff[iPos];
		iPos += 1;
		INT8 value[valueLen] = {0};
		memcpy(value, &tmpBuff[iPos], valueLen);
		iPos += valueLen;

	}
	return true;
}

bool CMDParserUp::getInitSet(INT8 *buff, InitSetConf &initConf) {
	return true;
}

bool CMDParserUp::campareNetSetMatch(INT8 *nameLen, INT8 *name,
		const INT8 *reName) {
	UINT32 nameLenComp = (UINT32) nameLen[0];
	if (nameLenComp != strlen(PCREQUESTIP) + 1
			&& nameLenComp != strlen(PCREQUESTSUBMASK) + 1
			&& nameLenComp != strlen(PCREQUESTSERVERIP) + 1
			&& nameLenComp != strlen(PCREQUESTCOMMUNICATIONPORT) + 1
			&& nameLenComp != strlen(PCREQUESTMAC) + 1
			&& nameLenComp != strlen(PCREQUESTMASK) + 1) {
		cout << "wrong input ip submask serverip..." << endl;
		return false;
	}
	return !(bool) compareUpgradeItem(name, reName, nameLenComp);

}
#endif

INT32 CMDParserUp::writeMaskFile(vector<UINT16> date) {

	INT32 fileDes = open(MASKPATH, O_CREAT | O_RDWR | O_TRUNC, 0666);
	if (fileDes == -1) {

		return retError;
	}
	INT8 buf[1024] = { 0 };

	buf[1] = (INT8) (date[0] >> 8);
	buf[0] = (INT8) date[0];
	buf[3] = (INT8) (date[1] >> 8);
	buf[2] = (INT8) date[1];
	buf[5] = (INT8) (date[2] >> 8);
	buf[4] = (INT8) date[2];
	buf[7] = (INT8) (date[3] >> 8);
	buf[6] = (INT8) date[3];
//	for (INT32 i = 0; i < 8; i++)
	for (INT32 i = 0; i < 8; i++) {
		if (write(fileDes, &buf[i], 1) == -1) {
		}
	}
	return retOk;
}
