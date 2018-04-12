#include "CMDParserUp.h"
#include "UpgradeServiceConfig.h"
#include "GlobalProfile.h"
#include "FileOperation.h"
#include "DevSearch.h"

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

CMDParserUp::CMDParserUp() :
		settingNum(0) {
}
CMDParserUp::~CMDParserUp() {
}
UINT16 CMDParserUp::parserPCRequestHead(void *buffer, INT32 recvLen) {
	PC_DEV_Header *pcHead = (PC_DEV_Header *) buffer;
	cout << "pcHead->HeadTag : " << pcHead->HeadTag << endl;

	if (pcHead->HeadTag != 0x0101FBFC) {
		return (UINT16) retError;
	}
	/* modify */
	cout << "dalen: " << pcHead->DataLen << " recvlen: " << recvLen << endl;
	if ((pcHead->DataLen != recvLen - sizeof(PC_DEV_Header))
			&& (pcHead->DataLen != 0)) {
		return (UINT16) retError;
	}
	UINT16 headCMD = pcHead->HeadCmd;
	return headCMD;
}

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
#if 0
		case 0x01:
		if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
						PCREQUESTIP) != true) {

			strcpy(&failReason, "Match IP failed !");
			return retError;
		} else {
			COPYIP
			;
			if (setNetworkTerminal->setNetworkConfig(netConfigTrans.ipT,
							NULL, NULL, NULL, INIFILE) != true) {

				strcpy(&failReason, "Set IP failed !");
				return retError;
			}
		}
		break;
#endif
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
#if 0
			case 0x05:
			if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
							PCREQUESTIP) != true) {
				strcpy(&failReason, "Match IP failed !");
				return retError;
			} else {
				COPYIP
				;
				OFFSETPTR
				;
			}
			if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
							PCREQUESTSUBMASK) != true) {
				strcpy(&failReason, "Match submask failed !");
				return retError;
			} else {
				COPYSUBMASK
				;
				OFFSETPTR
				;
			}
			if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
							PCREQUESTGATEWAY) != true) {
				strcpy(&failReason, "Match gateway failed !");
				return retError;
			} else {
				COPYGATEWAY
				;
				OFFSETPTR
				;
			}
			if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
							PCREQUESTSERVERIP) != true) {
				strcpy(&failReason, "Match server IP failed !");
				return retError;
			} else {
				COPYSERVERIP
				;
				OFFSETPTR
				;
			}
			if (campareNetSetMatch(&pcSettingNet[0], pcSettingNet + 1,
							PCREQUESTSERVERPORT) != true) {
				strcpy(&failReason, "Match server port failed !");
				return retError;
			} else {
				COPYSERVERPORT
				;
			}
			if (setNetworkTerminal->setNetworkConfig(netConfigTrans.ipT,
							netConfigTrans.submaskT, netConfigTrans.gatewayT, NULL,
							INIFILE) != true) {
				strcpy(&failReason, "Set IP submask and gateway failed !");
				return retError;
			}
			sscanf(netConfigTrans.serverPortT, "%hu", &port);
			setServerConf.SetTCPCommServerIP(netConfigTrans.serverIPT, port);
			break;
#endif
		default:
//			strcpy(&failReason, "Number of settings error !");
			return retError;
			break;
		}
	} /* end field */
	return retOk;
}

upgradeFileStatus CMDParserUp::parserPCUpgradeCMD(void *buffer,
		UpFileAttrs &upFileAttr, INT8 *failReason) {
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
			pcUpgradeCMD + strlen(upFileAttr.getNewSoftVersion()), WEBREQUEST,
			strlen(WEBREQUEST)) != 0) {
		pcUpgradeCMD += newSoftVersionSize;
		INT32 fileSize = 0;
		memcpy(&fileSize, pcUpgradeCMD, sizeof(UINT32));
		upFileAttr.setNewSoftFileSize(fileSize);

		pcUpgradeCMD += sizeof(UINT32);
		upFileAttr.setFileMD5Code(pcUpgradeCMD, upgradeFileMd5Size);
		//printf("recv md5 : %u\n", upFileAttr.getFileMD5Code());
	} else {
		if (!FileOperation::isExistFile(upFileAttr.getFileDownloadPath())) {
			strcpy(failReason, UPFILENOTEXIST);
			return errorVersionStatus;
		}
//		strcpy(failReason, WEBBeginToUpgrade);
		upFileAttr.setWebUpMethod(true);
//		return higherVerison;
	}

	if (strlen(version) != 0) {
		INT32 retCompare = compareUpgradeItem(upFileAttr.getNewSoftVersion(),
				version, strlen(version));
		if (retCompare == retOk) {
			strcpy(failReason, NONEEDTOUPGRADE);
			return equalVersion;
		} else if (retCompare < retOk) {
			strcpy(failReason, LOWERVERSION);
			return lowerVersion;
		} else if (retCompare > retOk) {
			strcpy(failReason, WEBBeginToUpgrade);
			return higherVerison;
		}
	} else {
		strcpy(failReason, GETVERSIONFAILED);
		return errorVersionStatus;
	}
	return errorVersionStatus;
}

bool CMDParserUp::campareNetSetMatch(INT8 *nameLen, INT8 *name,
		const INT8 *reName) {
	UINT32 nameLenComp = (UINT32) nameLen[0];
	if (nameLenComp != strlen(PCREQUESTIP) + 1
			&& nameLenComp != strlen(PCREQUESTSUBMASK) + 1
			&& nameLenComp != strlen(PCREQUESTSERVERIP) + 1
			&& nameLenComp != strlen(PCREQUESTSERVERPORT) + 1) {
		cout << "wrong input ip submask serverip..." << endl;
		return false;
	}
	return !(bool) compareUpgradeItem(name, reName, nameLenComp);

	// return true;
}
