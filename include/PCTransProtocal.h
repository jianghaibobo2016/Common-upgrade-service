/*
 * PCTransProtocal.h
 *
 *  Created on: 2018年3月5日
 *      Author: dsppa
 */

#ifndef PCTRANSPROTOCAL_H_
#define PCTRANSPROTOCAL_H_
#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <stdlib.h>
#include <vector>
#include <map>
#include <fstream>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include "GlobDefine.h"
//#include "Public.h"
// #define CONFPATH "/etc/network/interfaces"
// #define CHARSIZE 1024
// #define NETCARDNAME "ens33"//"bond0"
// #define VERSIONPATH "/home/dsppa/DSPPA/ServiceVersion.xml"
#define PROTOCAL_PC_DEV_HEAD 0x0101FBFC
#define PROTOCAL_PC_DEV_MAXSIZE 1400
using namespace std;
// Defines unsigned types
// typedef unsigned char           UINT8;
// typedef unsigned short          UINT16;
// typedef unsigned int            UINT32;
// typedef unsigned long long      UINT64;

// Defines signed types
// typedef signed char             INT8;
// typedef signed short            INT16;
// typedef signed int              INT32;
// typedef signed long long        INT64;
#pragma pack(push)
#pragma pack(1)
enum PC_DEV_COMMAND{
	CMD_DEV_SEARCH = 1,                   //设备搜索
	CMD_DEV_PARAMETER_SETTING = 2,        //参数设置
	CMD_DEV_UPGRADE = 3,				  //设备升级
	CMD_DEV_FILE_TRANSPORT = 4,			  //文件传输
	CMD_DEV_UPGRADE_REPLY = 5,            //升级反馈
	CMD_DEV_BROADCAST_SELF_ADDRESS = 6,   //设备广播自己地址
	CMD_WEB_REQUEST_UPGRADE = 100,		  //终端web升级终端
	CMD_REQUEST_UPGRADE = 101			  //

};
/************************************1. Package defines***********************/
//传输包结构:
//Tag 0x0101FBFC
//Cmd 0x000X:X为对应命令号
//Len 数据长度
//Data 数据内容

//公共包头
typedef struct PC_DEV_Header_tag {
	UINT32 HeadTag;
	UINT16 HeadCmd;
	UINT16 DataLen;
} PC_DEV_Header;

//----------------------------------1.搜索协议---------------------------------------//
//PC端发送
typedef struct PC_Request_GetDevMsg_tag{
	PC_DEV_Header header;
}PC_Request_GetDevMsg;
//设备端返回
typedef struct DEV_Reply_GetDevMsg_tag{
	PC_DEV_Header header;
	UINT32 DevIP;            //设备IP //网路字节顺序(下同)
	UINT32 DevMask;    		 //设备子网掩码
	UINT32 DevGateway;		 //设备网关
	UINT32 DevServerIP;      //设备服务器IP
	UINT16 DevServerPort;    //设备服务器端口
	char DevMACAddress[13];  //设备MAC地址 //ASCII字符大写，包含结束符
	char DevID[40];          //设备ID(前四个字符为设备类型)
	char DevType[20];        //设备型号
	char HardVersion[20];    //硬件版本
	char SoftVersion[20];    //软件版本
	char DevName[60];  	 	 //设备名称;
}DEV_Reply_GetDevMsg;
/////14items
//-------------------------------------------------------------------//


//---------------------------------2.参数设置-------------------------//
//PC端发送
typedef struct PC_Request_ParameterSetting_tag{
	PC_DEV_Header header;
	UINT8 ParameterNum; //add 1B
	//unsigned char NameLen;
	//char* name;
	//unsigned char ValueLen;
	//char* value;
//......
}PC_Request_ParameterSetting;
//设备端返回
typedef struct DEV_Reply_ParameterSetting_tag{
	PC_DEV_Header header;
	char DevID[40]; //设备ID(前四个字符为设备类型)//add
	UINT8 Result; //1:成功,0:失败
	// UINT8 FailReasonLen;
	// char* FailReason;
//......
}DEV_Reply_ParameterSetting; 	//暂时没有具体失败理由. 
//-------------------------------------------------------------------//

//---------------------------------3.升级协议-------------------------//
//PC端发送
typedef struct PC_Request_DevUpgrade_tag{
	PC_DEV_Header header;
	char HardVersion[20];       //硬件版本
	char NewSoftVersion[20];    //新软件版本 V01.01
	UINT32 NewSoftDocumentSize; //新软件版本文件大小
	char UpgradeFileMd5[16];	//MD5校验值//add
}PC_Request_DevUpgrade;
//设备端返回
typedef struct DEV_Reply_DevUpgrade_tag{
	PC_DEV_Header header;
	char DevID[40]; //设备ID(前四个字符为设备类型)//add
	UINT8 Result; //1:可以升级,0:不可以升级
	//UINT8 FailReasonLen;
	//char* FailReason;
//......
}DEV_Reply_DevUpgrade;
//-------------------------------------------------------------------//

//------------------------------4.文件传输协议-------------------------//
//设备端请求
typedef struct DEV_Request_FileProtocal_tag{
	PC_DEV_Header header;
	char DevType[20];           //设备型号
	char HardVersion[20];       //硬件版本
	char NewSoftVersion[20];    //新软件版本
	UINT32 StartPosition;       //起始位置,0xFFFFFFFF传输完毕，PC不用返回
	UINT16 FileDataLen;         //数据长度
}DEV_Request_FileProtocal;
//PC端返回
typedef struct PC_Reply_FileProtocal_tag{
	PC_DEV_Header header;
	UINT32 StartPosition;       //起始位置,0xFFFFFFFF传输完毕，PC不用返回
	UINT16 FileDataLen;         //数据长度
	//char* FileData            //数据内容
}PC_Reply_FileProtocal;
//-------------------------------------------------------------------//

//------------------------------5.升级过程反馈-------------------------//
//设备端请求
typedef struct DEV_Request_UpgradeReply_tag{
	PC_DEV_Header header;
	char DevID[40]; //设备ID(前四个字符为设备类型)//add
	UINT8 Result; //升级结果反馈 成功:1, 失败:0
	//char* text
}DEV_Request_UpgradeReply;
//PC端无返回

//-------------------------------------------------------------------//

//------------------------------6.设备广播自己的地址-------------------------//
//设备端请求
typedef struct DEV_Request_DevBroadcast_tag{
	PC_DEV_Header header;
	char DevType[20];           //设备型号
	char HardVersion[20];       //硬件版本
	char SoftVersion[20];       //软件版本
	UINT32 IP;
}DEV_Request_DevBroadcast;
//PC端无返回

//-------------------------------------------------------------------//
struct InitServiceMsg{//服务器保存的信息
	char cDevID[40];
	char cHardVersion[20];
	char cDevType[20];
};
struct UpgradeMsg{//服务器升级信息
	char cNewSoftVersion[20];
	int iNewSoftSize;
	char cMD5[16];
	int iFirstPos;
};
#endif /* PCTRANSPROTOCAL_H_ */
