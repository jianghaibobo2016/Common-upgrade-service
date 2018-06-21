/*
 * UpdateProgram_Protocal.h
 *
 *  Created on: 2018年4月16日
 *      Author: mo
 */

#ifndef PROTOCAL_UPDATEPROGRAM_PROTOCAL_H_
#define PROTOCAL_UPDATEPROGRAM_PROTOCAL_H_

#include "UpgradeServiceConfig.h"
#pragma pack(push)
#pragma pack(1)

enum UPDATE_DEV_COMMAND {
	CMD_LOCALDEV_UPGRADE = 1,			  //设备升级
	CMD_LOCALDEV_TESTMODE = 2,		      //设备进入测试模式
	CMD_LOCALDEV_GETMASK = 3,			  //获取设备加密信息
	CMD_DEV_ONLINE = 4,			     	  //设备状态
	CMD_DEV_REPLY_UPGRADE = 5,			  //设备升级进度反馈
	CMD_SET_TERMINAL_CONFIG = 6			//设置终端配置－－服务器地址；服务器通信端口；服务器录播端口；设备名字
};

#if (DSP9903)
//zuofei config file
enum UPDATE_DEV_TYPE {
	UPDATE_DEV_AMP_TYPE = 1,			 //功放板
	UPDATE_DEV_PAGER_TYPE = 2,		 //寻呼器
	ERROR_TYPE
};
#endif

#if (DSP9906)
//zuofei config file
enum UPDATE_DEV_TYPE {
	UPDATE_DEV_AMP_TYPE = 1,			 //功放板
	ERROR_TYPE
};
#endif

#if (DSP9909)
//zuofei config file
enum UPDATE_DEV_TYPE {
	ERROR_TYPE
};
#endif
/************************************1. Package defines***********************/
//传输包结构:
//Tag 0x0101FBFC
//Cmd 0x000X:X为对应命令号
//Len 数据长度
//Data 数据内容
//公共包头
typedef struct UPDATE_DEV_Header_tag {
	UINT32 HeadTag;
	UINT16 HeadCmd;
	UINT16 DataLen;//Length of data of Struct
} UPDATE_DEV_Header;

//------------------------------1.设备升级-------------------------//
//升级程序控制
typedef struct UPDATE_SEND_UPDATEDEV_tag {
	UPDATE_DEV_Header header;
	UINT8 dev_type; // UPDATE_DEV_TYPE
} UPDATE_SEND_UPDATEDEV;

//视频板回复
typedef struct ARM_REPLAYUPDATE_UPDATEDEV_tag {
	UPDATE_DEV_Header header;
	UINT8 state; //0：失败 1：升级完成 2：版本相同 3：设备不在线
} ARM_REPLAYUPDATE_UPDATEDEV;
//------------------------------2.设备测试模式-------------------------//
//升级程序控制
typedef struct UPDATE_SEND_TESTMODE_tag {
	UPDATE_DEV_Header header;
	UINT8 Control; //0:停止 1：开始
} UPDATE_SEND_TESTMODE;

//------------------------------3.获取设备加密信息-----------------------//
//升级程序控制
typedef struct UPDATE_GET_MASK_tag {
	UPDATE_DEV_Header header;
} UPDATE_GET_MASK;

//视频板回复
typedef struct ARM_REPLAYUPDATE_GETMASK_tag {
	UPDATE_DEV_Header header;
	unsigned short m_mask[4]; //传送加密方式：~m_mask[0]+1,先取反后加一
} ARM_REPLAYUPDATE_GETMASK;

//------------------------------4.设备是否在线且可升级否-----------------------//
//升级程序
typedef struct UPDATE_GET_DEVSTATUS_tag {
	UPDATE_DEV_Header header;
	UINT8 dev_type; // UPDATE_DEV_TYPE
} UPDATE_GET_DEVSTATUS;

//视频板回复
enum UPGRADE_DEVMODULES_STATUS {
	OFFLINE = 0, NONEEDUP = 1, UPGRADE_VALID
};
typedef struct ARM_REPLAY_GETDEVSTATUS_tag {
	UPDATE_DEV_Header header;
	UINT8 dev_type; // UPDATE_DEV_TYPE
	UINT8 state; //0：离线 1：在线无需升级 2:在线可升级
} ARM_REPLAY_GETDEVSTATUS;

//------------------------------5.获取设备升级进度-----------------------//

//视频板回复
typedef struct ARM_REPLAY_GETUPDATEPROGRESS_tag {
	UPDATE_DEV_Header header;
	UINT8 dev_type; // UPDATE_DEV_TYPE
	UINT8 state;
} ARM_REPLAY_GETUPDATEPROGRESS;

//------------------------------6.设置终端配置 -----------------------//

//升级程序
typedef struct UPGRADE_REQUEST_SET_CONFIG_tag {
	UPDATE_DEV_Header header;
	UINT32 ServerIP; //inet_addr
	UINT16 CommunicationPort;
#if ((DSP9903) || (DSP9909))
	UINT16 RecordingPort; // DSP9903 ONLY
#endif
	INT8 DevName[60];
} UP_PROG_SET_CONF;

//服务程序回复
typedef struct SERVER_PROG_REPLY_SET_CONFIG_tag {
	UPDATE_DEV_Header header;
	UINT8 result; // fail : 0 ; success : 1
} SERVER_REPLY_SET_CONF;


#pragma pack(pop)
#endif /* PROTOCAL_UPDATEPROGRAM_PROTOCAL_H_ */
