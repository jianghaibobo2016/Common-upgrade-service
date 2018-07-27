#ifndef UPGRADESERVICECONFIG_H
#define UPGRADESERVICECONFIG_H
#include <iostream>
#include "GlobDefine.h"
// UpgradeServiceConfig.h

/*switch type*/
#define DSP9903 1
#define DSP9906 0
#define DSP9909 0
/*switch type*/
/*type switch setting*/
#if (DSP9903)
/*upgrade soft version*/
static const INT8 *UpgradeSoftVersion = "V01.34.01";
/*03: adding that system will up rootfs and reboot with failed upgrading some modules.
 * 04:modify protocol of set mask and network
 * 05 : upgrade zip : Begin to upgrade! fix this bug.
 * 30.01 HandleUp.cpp 620 modified
 * */
/*upgrade soft version*/
//static const TERMINAL_DEV_TYPE Terminal_Dev_DSP = TERMINAL_DEV_DSP9903;
static const INT8 *TerminalDevTypeID = "0065";
static const INT8 *TerminalDevType = "DSP9903";
static const INT8 *StringFind = "DSP9903_";
static const INT8 *INIFILE = "/dsppa/SET_NET/DSP9903_NET_CONF.ini";
static const INT8 *upFileDownload = "/nand/Update_File/DSP9903_PRODUCT_";
static const INT8 *AmplifierUpgrade = "DSP9903_Amplifier_";
static const INT8 *PagerUpgrade = "DSP9903_Pager_";
static const INT8 *MainRootfsUpgrade = "DSP9903_MROOTFS_";
/*log path setting*/
static const INT8 *logPath = "/nand/log/UpgradeService_9903";
/*log path setting*/
/*mask.bin setting*/
static const bool getMask = true;
/*mask.bin setting*/
/*Devs type*/
enum DEV_MODULES_TYPE {
	DEV_AMPLIFIER = 1,			 //功放板
	DEV_PAGER = 2,//寻呼器
	INVALID_TYPE
};
/*Devs type*/
#endif

#if (DSP9906)
/*upgrade soft version*/
static const INT8 *UpgradeSoftVersion = "V01.23.01";
/*upgrade soft version*/
//static const TERMINAL_DEV_TYPE Terminal_Dev_DSP = TERMINAL_DEV_DSP9906;
static const INT8 *TerminalDevTypeID = "0066";
static const INT8 *TerminalDevType = "DSP9906";
static const INT8 *StringFind = "DSP9906_";
static const INT8 *INIFILE = "/dsppa/SET_NET/DSP9906_NET_CONF.ini";
static const INT8 *upFileDownload = "/nand/Update_File/DSP9906_PRODUCT_";
static const INT8 *AmplifierUpgrade = "DSP9906_Amplifier_";
static const INT8 *PagerUpgrade = "DSP9906_Pager_INVALID";
static const INT8 *MainRootfsUpgrade = "DSP9906_MROOTFS_";
/*log path setting*/
static const INT8 *logPath = "/nand/log/UpgradeService_9906";
/*log path setting*/
/*mask.bin setting*/
static const bool getMask = false;
static const UINT16 mask[4] = { 0xffff, 0xffff, 0xffff, 0xffff };
/*mask.bin setting*/
/*Devs type*/
enum DEV_MODULES_TYPE {
	DEV_AMPLIFIER = 1,			 //功放板
	INVALID_TYPE
};
/*Devs type*/
#endif
/*type switch setting*/


#if (DSP9909)
/*upgrade soft version*/
static const INT8 *UpgradeSoftVersion = "V01.03.02";
/*upgrade soft version*/
static const INT8 *TerminalDevTypeID = "0068";
static const INT8 *TerminalDevType = "DSP9909";
static const INT8 *StringFind = "DSP9909_";
static const INT8 *INIFILE = "/dsppa/SET_NET/DSP9909_NET_CONF.ini";
static const INT8 *upFileDownload = "/nand/Update_File/DSP9909_PRODUCT_";
static const INT8 *AmplifierUpgrade = "DSP9909_Amplifier_INVALID";
static const INT8 *PagerUpgrade = "DSP9909_Pager_INVALID";
static const INT8 *MainRootfsUpgrade = "DSP9909_MROOTFS_";
/*log path setting*/
static const INT8 *logPath = "/nand/log/UpgradeService_9909";
/*log path setting*/
/*mask.bin setting*/
static const bool getMask = true;
/*mask.bin setting*/
/*Devs type*/
enum DEV_MODULES_TYPE {
//	DEV_AMPLIFIER = 1,			 //功放板
	INVALID_TYPE
};
/*Devs type*/
#endif
/*type switch setting*/

/*devs type define*/
static const INT8 *AMPLIFIER = "Amplifier";
static const INT8 *PAGER = "Pager";
/*devs type define*/

/*net port*/
static const INT32 UpUDPTransPort = 45535;
static const INT32 UpRespondPort = 65530;
static const INT32 UpLocalPort = 4307;
static const INT8 *MultiCastAddrRECV = "239.0.0.5";
static const INT8 *MultiCastAddrSEND = "239.0.0.6";
/*net port*/

/* struct of protocal member size */
static const INT32 devMACAddressSize = 13;
static const INT32 devIDSize = 40;
static const INT32 devTypeSize = 20;
static const INT32 hardVersionSize = 20;
static const INT32 softVersionSize = 20;
static const INT32 devNameSize = 60;
static const INT32 newSoftVersionSize = 20;
static const INT32 upgradeFileMd5Size = 16;
static const INT32 fileDownloadPathSize = 64;
static const INT32 msgLen = 64;
static const INT32 TerminalDevsMaxNum = 8;
static const INT32 TerminalDevsNameLenMax = 20;
/* struct of protocal member size */

/*file trans config*/
static const INT32 BufferSizeMax = 1400 * 32;
static const UINT32 SendBuffMaxSize = 1400;
static const INT32 FileTransLen = 1024 * 32;
static const INT32 SendBufferSizeMax = 1400;
/*file trans config*/

//static const INT8 *TerminalDevTypeID = "0065";
static const INT8 *TerminalHardVersion = "V01.00";
static const std::string pathXml = "/nand/ServerAPP/video_conf.xml";
static const INT8 *pathVersionFile = "/dsppa/HSversion";
static const INT8 *DependItemVersionName = "upgrade_version";
static const INT8 *ProductItemName = "PRODUCT";
static const INT8 *UpgradeItemName = "UPGRADE";
static const INT8 *upFilePath = "/nand/Update_File/";

static const INT8* newTarPackage = "/nand/Update_File/upgradefiletar.tar";
//static const INT8 *AmplifierUpgrade = "DSP9903_ServerApp_";

/* text :
 * "Upgrade failed!" 表示升级失败
 * "Upgrade successed!" 表示升级成功*/
static const INT8 *AmplifierUpFail = "Upgrade failed!";
static const INT8 *AmplifierUpsuccess = "Upgrade successed!";
static const INT8 *IFNAMETERMINAL = "eth0";
//static const INT8 *IFNAMETERMINAL = "ens33";

/*Net Set Tag*/
static const INT8 *PCREQUESTIP = "IP";
static const INT8 *PCREQUESTSUBMASK = "SubMask";
static const INT8 *PCREQUESTGATEWAY = "GateWay";
static const INT8 *PCREQUESTSERVERIP = "ServerIP";
static const INT8 *PCREQUESTCOMMUNICATIONPORT = "CommunicationPort";
static const INT8 *PCREQUESTRCORDINGPORT = "RecordingPort";
static const INT8 *PCREQUESTNAME = "Name";
static const INT8 *PCREQUESTMAC = "Mac";
static const INT8 *PCREQUESTMASK = "Mask";
static const INT8 *MASKPATH = "/usr/share/mask.bin";
static const INT8 *MODIFYRESPOND_SUCCESS = "Modify successed !";
static const INT8 *MODIFYRESPOND_FAIL = "Modify failed !";
static const INT8 *MODIFYRESPOND_SAMECONF = "Same config !";
/*Net Set Tag*/

/* upgrade */
static const INT8 *NONEEDTOUPGRADE = "No need to upgrade !";
static const INT8 *INUPGRADING = "Upgrading...";
static const INT8 *LOWERVERSION =
		"Request soft version lower than local soft version !";
static const INT8 *GETVERSIONFAILED = "Get local version failed!";
static const INT8 *UPFILENOTEXIST = "Upgrade file is not existed !";
static const INT8 *upSysRespond = "Upgrade system successed !";
//static const INT8 *PRODUCT_TYPE = "DSP9903";
static const INT8 *UpgradeShell = "upgrade.sh";
static const INT8 *UpgradeShellWithPath = "/nand/Update_File/upgrade.sh";
static const INT8 *SuccessUpShellRespond = "Upgrade successfully this time !";
static const INT8 *UpShellRespondTag_1 = "0F";
static const INT8 *UpShellRespondTag_3_Success = "00";
static const INT8 *FailUpShellRespond = "Fail to upgrade this time !";
static const INT8 *ProductVersionName = "product_version";
static const INT8 *UbootVersionName = "uboot_version";
static const INT8 *KernelVersionName = "kernel_version";
static const INT8 *MainRootfsVersionName = "main_rootfs_version";
static const INT8 *BackupRootfsVersionName = "backup_rootfs_version";
static const INT8 *MediaAppVersionName = "media_app_version";
static const INT8 *ServerAppVersionName = "server_app_version";
static const INT8 *UpgradeVersionName = "upgrade_version";
static const INT8 *WebVersionName = "web_version";
static const INT8 *NandVersionName = "nand_version";
static const INT32 subNum = 9;
/*WEB request */
static const INT8 *WEBREQUEST = "WEB";
static const INT8 *PCREQUEST = "PC";
//static const INT8 *VOIDREQUEST = '\0';

/*Force to upgrade*/
static const INT8 *FORCEUPGRADE = "_FORCE";

static const INT8 *UPFILESYSTEM = "Upgrade system....";
static const INT8 *BeginToUpgrade = "Begin to upgrade !";
static const INT8 *PRODUCTUPFAILED = "Upgrade failed for some modules!";
static const INT8 *PRODUCTUPFAILED_UPSYS =
		"Upgrade failed for some modules! Begin to upgrade system !";
/* upgrade */

#endif /* UPGRADESERVICECONFIG_H */
