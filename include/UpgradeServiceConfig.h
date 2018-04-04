#ifndef UPGRADESERVICECONFIG_H
#define UPGRADESERVICECONFIG_H
#include <iostream>
#include "GlobDefine.h"

/*net port*/
static const INT32 UpUDPTransPort = 45535;
static const INT32 UpAmplifierPort = 8001;
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
/* struct of protocal member size */

/*file trans config*/
static const INT32 BufferSizeMax = 1400 * 32;
static const INT32 FileTransLen = 1024 * 32;
static const INT32 SendBufferSizeMax = 1400;
/*file trans config*/

static const INT8 *TerminalDevTypeID = "0065";
static const INT8 *TerminalDevType = "DSP9903";
static const INT8 *TerminalHardVersion = "V01.00";
static const std::string pathXml = "/nand/ServerAPP/video_conf.xml";
static const INT8 *pathVersionFile = "/dsppa/HSversion";
static const INT8 *DependItemVersionName = "upgrade_version";
static const INT8 *ProductItemName = "PRODUCT";
static const INT8 *INIFILE = "/dsppa/SET_NET/DSP9903_NET_CONF.ini";
static const INT8 *upFilePath = "/nand/Update_File/";
static const INT8 *upFileDownload = "/nand/Update_File/DSP9903_PRODUCT_";
static const INT8* newTarPackage = "/nand/Update_File/upgradefiletar.tar";
static const INT8 *upgradeAmplifier = "Upgrade";
static const INT8 *AmplifierUpgrade = "DSP9903_AMPLIFIER_";
/* text :
 * "Upgrade failed!" 表示升级失败
 * "Upgrade successed!" 表示升级成功*/
static const INT8 *AmplifierUpFail = "Upgrade failed!";
static const INT8 *AmplifierUpsuccess = "Upgrade successed!";
static const INT8 *IFNAMETERMINAL = "eth0";
//static const INT8 *IFNAMETERMINAL = "ens33";
static const INT8 *PCREQUESTIP = "IP";
static const INT8 *PCREQUESTSUBMASK = "SubMask";
static const INT8 *PCREQUESTGATEWAY = "GateWay";
static const INT8 *PCREQUESTSERVERIP = "ServerIP";
static const INT8 *PCREQUESTSERVERPORT = "ServerPort";

/* upgrade */
static const INT8 *NONEEDTOUPGRADE = "No need to upgrade !";
static const INT8 *LOWERVERSION =
		"Request soft version lower than local soft version !";
static const INT8 *GETVERSIONFAILED = "Get local version failed!";
static const INT8 *UPFILENOTEXIST = "Upgrade file is not existed !";
//static const INT8 *PRODUCT_TYPE = "DSP9903";
static const INT8 *UpgradeShell = "upgrade.sh";
static const INT8 *UpgradeShellWithPath = "/nand/Update_File/upgrade.sh";
static const INT8 *SuccessUpShellRespond = "Upgrade successfully this time !";
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
static const INT8 *WEBREQUEST = "WEBTERMINAL";
static const INT8 *WEBBeginToUpgrade = "Begin to upgrade !";
/* upgrade */
#endif /* UPGRADESERVICECONFIG_H */
