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
    pcSettingNet += (1 + (INT32)pcSettingNet[0]);
#define COPYIP                                                                      \
    OFFSETPTR                                                                       \
    memcpy(netConfigTrans.ipT, pcSettingNet + 1, (INT32)pcSettingNet[0]);   

#define COPYSUBMASK                                                                 \
    OFFSETPTR                                                                       \
    memcpy(netConfigTrans.submaskT, pcSettingNet + 1, (INT32)pcSettingNet[0]);

#define COPYGATEWAY                                                                 \
    OFFSETPTR                                                                       \
    memcpy(netConfigTrans.gatewayT, pcSettingNet + 1, (INT32)pcSettingNet[0]);   

#define COPYSERVERIP                                                                \
    OFFSETPTR                                                                       \
    memcpy(netConfigTrans.serverIPT, pcSettingNet + 1, (INT32)pcSettingNet[0]); 

#define COPYSERVERPORT                                                              \
    OFFSETPTR                                                                       \
    memcpy(netConfigTrans.serverPortT, pcSettingNet + 1, (INT32)pcSettingNet[0]);   

#define COPYMAC                                                               	    \
    OFFSETPTR                                                                       \
    memcpy(netConfigTrans.MAC, pcSettingNet + 1, (INT32)pcSettingNet[0]);

#define COPYMASK                                                                    \
    OFFSETPTR                                                                       \
    memcpy(netConfigTrans.MASK, pcSettingNet + 1, (INT32)pcSettingNet[0]);
CMDParserUp::CMDParserUp() :
		settingNum(0) {
}
CMDParserUp::~CMDParserUp() {
}
UINT16 CMDParserUp::parserPCRequestHead(void *buffer, INT32 recvLen) {
	PC_DEV_Header *pcHead = (PC_DEV_Header *) buffer;

//	Logger::GetInstance().Info("Recv CMD %d with tag %X from PC !", pcHead->HeadCmd, pcHead->HeadTag);
	printf("Recv CMD %d with tag %X from PC !\n", pcHead->HeadCmd,
			pcHead->HeadTag);
	if (pcHead->HeadTag != PROTOCAL_PC_DEV_HEAD) {
		return (UINT16) retError;
	}
	/* modify */
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
INT32 CMDParserUp::parserPCSetNetCMD(void *buffer,
		SetNetworkTerminal *setNetworkTerminal,
		map<string, string> &retContent) {
	INT8 *pcSettingNet = (INT8 *) buffer;
	pcSettingNet += sizeof(PC_DEV_Header);
	UINT8 parameterNum = pcSettingNet[0];

	pcSettingNet += 1;
	{
		NetConfigTransWithServer netConfigTrans;
		UINT16 port = 0;
		GlobalProfile setServerConf;
		switch (parameterNum) {
		/* 03 495000 0E 3137322E31362E31302E31333300 */
		case 0x01:
			if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
					PCREQUESTMAC) == true) {
//			} else {
				COPYMAC
				;
				string mac;
				for (int i = 0; i < (INT32) pcSettingNet[0];) {
					mac += netConfigTrans.MAC[i];
					mac += netConfigTrans.MAC[i + 1];
					if (i + 3 < (INT32) pcSettingNet[0])
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
			} else if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
					PCREQUESTMASK) == true) {
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
			if ((INT32) pcSettingNet[0] == 0x02) {
				retContent[PCREQUESTSERVERIP] = "Match failed !";
				retContent[PCREQUESTSERVERPORT] = "Match failed !";
				return retError;
			} else {
				if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
						PCREQUESTSERVERIP) != true) {
					retContent[PCREQUESTSERVERIP] = "Match failed !";
					retContent[PCREQUESTSERVERPORT] = "Match failed !";
					return retError;
				} else {
					COPYSERVERIP
					;
				}
				OFFSETPTR
				;
				if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
						PCREQUESTSERVERPORT) != true) {
					retContent[PCREQUESTSERVERIP] = "Match failed !";
					retContent[PCREQUESTSERVERPORT] = "Match failed !";
					return retError;
				} else {
					COPYSERVERPORT
					;
				}
				sscanf(netConfigTrans.serverPortT, "%hu", &port);
				setServerConf.SetTCPCommServerIP(netConfigTrans.serverIPT,
						port);
				retContent[PCREQUESTSERVERIP] = "Set server IP OK!";
				retContent[PCREQUESTSERVERPORT] = "Set server Port OK!";
			}
			break;
		case 0x03:
			if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
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
			if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
					PCREQUESTSUBMASK) == true) {
				COPYSUBMASK
				;
				OFFSETPTR
				;
				if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
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
			} /*else if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
			 PCREQUESTSERVERIP) == true) {
			 COPYSERVERIP
			 ;
			 OFFSETPTR
			 ;
			 if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
			 PCREQUESTSERVERPORT) == true) {
			 COPYSERVERPORT
			 ;
			 if (setNetworkTerminal->setNetworkConfig(netConfigTrans.ipT,
			 NULL, NULL, NULL, INIFILE) != true) {
			 strcpy(&failReason, "Set IP failed !");
			 return retError;
			 }
			 sscanf(netConfigTrans.serverPortT, "%hu", &port);
			 setServerConf.SetTCPCommServerIP(netConfigTrans.serverIPT,
			 port);
			 } else {
			 strcpy(&failReason, "Match setting failed !");
			 return retError;
			 }
			 }*/else {
				retContent[PCREQUESTIP] = "Match setting failed !";
				retContent[PCREQUESTSUBMASK] = "Match setting failed !";
				retContent[PCREQUESTGATEWAY] = "Match setting failed !";
				return retError;
			}
			break;
		default:
//			strcpy(&failReason, "Number of settings error !");
			return retError;
			break;
		}
	} /* end field */
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
	INT8 *pcUpgradeCMD = (INT8 *) buffer;
	pcUpgradeCMD += sizeof(PC_DEV_Header);
	INT8 hardVersion[8] = { 0 };
	DevSearchTerminal::getSoftwareVersion("hardware_version", hardVersion,
			pathVersionFile);
	if (compareUpgradeItem(pcUpgradeCMD, hardVersion, strlen(hardVersion))
			!= true) {

	}
	pcUpgradeCMD += hardVersionSize;
	/* new software version */
	upFileAttr.setNewSoftVersion(pcUpgradeCMD, 6);
	INT8 fileDownloadName[fileDownloadPathSize] = { 0 };
	memcpy(fileDownloadName, upFileDownload, strlen(upFileDownload));
	strcat(fileDownloadName, upFileAttr.getNewSoftVersion());
	upFileAttr.setFileDownloadPath(fileDownloadName, strlen(fileDownloadName));
	INT8 version[7] = { 0 };
	DevSearchTerminal::getSoftwareVersion(ProductVersionName, version,
			pathVersionFile);
	/*WEB request to upgrade CMD judgement*/
	if (compareUpgradeItem(
			pcUpgradeCMD + strlen(upFileAttr.getNewSoftVersion()), FORCEUPGRADE,
			strlen(FORCEUPGRADE)) == 0) {
		if (!FileOperation::isExistFile(upFileAttr.getFileDownloadPath())) {
			strcpy(failReason, UPFILENOTEXIST);
			return errorVersionStatus;
		}
		upFileAttr.setWebUpMethod(true);
		upFileAttr.setForceUpgrade(true);
	} else if (compareUpgradeItem(
			pcUpgradeCMD + strlen(upFileAttr.getNewSoftVersion()), WEBREQUEST,
			strlen(WEBREQUEST)) == 0) {
		if (!FileOperation::isExistFile(upFileAttr.getFileDownloadPath())) {
			strcpy(failReason, UPFILENOTEXIST);
			return errorVersionStatus;
		}
		upFileAttr.setWebUpMethod(true);

	} else {
		pcUpgradeCMD += newSoftVersionSize;
		INT32 fileSize = 0;
		memcpy(&fileSize, pcUpgradeCMD, sizeof(UINT32));
		upFileAttr.setNewSoftFileSize(fileSize);

		pcUpgradeCMD += sizeof(UINT32);
		upFileAttr.setFileMD5Code(pcUpgradeCMD, upgradeFileMd5Size);
	}
//check 9903?
//	map<INT32, DEV_MODULES_TYPE> devModules;
//	CrcCheck::getDevModules(upFileAttr.getFileDownloadPath(), devModules);

//	isDevModulesUpgradeEnable(devModuleToUpgrade, devModules, upFileAttr);

	/*	if (devModuleToUpgrade.size() == 0
	 && upFileAttr.getForceStatus() == false) {
	 if (strlen(version) != 0) {
	 INT32 retCompare = compareUpgradeItem(
	 const_cast<INT8*>(upFileAttr.getNewSoftVersion()), version,
	 strlen(version));
	 if (retCompare == retOk) {
	 strcpy(failReason, NONEEDTOUPGRADE);
	 return equalVersion;
	 } else if (retCompare < retOk) {
	 strcpy(failReason, LOWERVERSION);
	 return lowerVersion;
	 } else if (retCompare > retOk) {
	 strcpy(failReason, BeginToUpgrade);
	 return higherVerison;
	 }
	 } else {
	 strcpy(failReason, GETVERSIONFAILED);
	 return errorVersionStatus;
	 }
	 } else {*/
	strcpy(failReason, BeginToUpgrade);
	return higherVerison;
//	}

	return errorVersionStatus;
}

INT32 CMDParserUp::isDevModulesUpgradeEnable(
		map<INT32, DEV_MODULES_TYPE> &devModuleToUpgrade,
		map<INT32, DEV_MODULES_TYPE>&devModules, UpFileAttrs &upFileAttr) {
	LocalUDPTrans netTrans;
	UPDATE_GET_DEVSTATUS getDevStatus;
	UINT32 num = 1;
	INT32 validDevModule = 1;
	for (; num <= devModules.size(); num++) {
		getDevStatus.header.HeadCmd = 0x0004;
		getDevStatus.dev_type = devModules[num];
		cout << "sendto servarappp type :: " <<getDevStatus.dev_type<< endl;
		HandleUp::localUpHandle<UPDATE_GET_DEVSTATUS>(getDevStatus);
		INT32 retSend = sendto(netTrans.getSockfd(), &getDevStatus,
				sizeof(UPDATE_SEND_UPDATEDEV), 0,
				(struct sockaddr*) netTrans.getAddr(), *netTrans.getAddrLen());
		if (retSend <= 0) {
			cout << "errorsend" << endl;
			return retError;
		} else {
			cout << "sendto serverapp : " << retSend << endl;
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
			cout << "recv len :: "<<retRecv<<endl;
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

bool CMDParserUp::campareNetSetMatch(INT8 *nameLen, INT8 *name,
		const INT8 *reName) {
	UINT32 nameLenComp = (UINT32) nameLen[0];
	if (nameLenComp != strlen(PCREQUESTIP) + 1
			&& nameLenComp != strlen(PCREQUESTSUBMASK) + 1
			&& nameLenComp != strlen(PCREQUESTSERVERIP) + 1
			&& nameLenComp != strlen(PCREQUESTSERVERPORT) + 1
			&& nameLenComp != strlen(PCREQUESTMAC) + 1
			&& nameLenComp != strlen(PCREQUESTMASK) + 1) {
		cout << "wrong input ip submask serverip..." << endl;
		return false;
	}
	return !(bool) compareUpgradeItem(name, reName, nameLenComp);

}

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
