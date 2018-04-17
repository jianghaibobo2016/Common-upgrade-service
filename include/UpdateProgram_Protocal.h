/*
 * UpdateProgram_Protocal.h
 *
 *  Created on: 2018年4月16日
 *      Author: mo
 */

#ifndef PROTOCAL_UPDATEPROGRAM_PROTOCAL_H_
#define PROTOCAL_UPDATEPROGRAM_PROTOCAL_H_

#pragma pack(push)
#pragma pack(1)


enum UPDATE_DEV_COMMAND{
	CMD_LOCALDEV_UPGRADE 	= 1,			  //设备升级
	CMD_LOCALDEV_TESTMODE	= 2,		      //设备进入测试模式
	CMD_LOCALDEV_GETMASK     = 3,			  //获取设备加密信息
};

enum UPDATE_DEV_TYPE{
	UPDATE_DEV_AMP = 1,			 //功放板
	UPDATE_DEV_PAGER = 2,		 //寻呼器
	ERROR_TYPE
};

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
	UINT16 DataLen;
} UPDATE_DEV_Header;

//------------------------------1.设备升级-------------------------//
//升级程序控制
typedef struct UPDATE_SEND_UPDATEDEV_tag{
	UPDATE_DEV_Header header;
	UINT8 dev_type; // UPDATE_DEV_TYPE
}UPDATE_SEND_UPDATEDEV;

//视频板回复
typedef struct ARM_REPLAYUPDATE_UPDATEDEV_tag{
	UPDATE_DEV_Header header;
	UINT8 state;//0：失败 1：升级完成
}ARM_REPLAYUPDATE_UPDATEDEV;

//------------------------------2.设备测试模式-------------------------//
//升级程序控制
typedef struct UPDATE_SEND_TESTMODE_tag{
	UPDATE_DEV_Header header;
	UINT8 Control;//0:停止 1：开始
}UPDATE_SEND_TESTMODE;

//------------------------------3.获取设备加密信息-----------------------//
//升级程序控制
typedef struct UPDATE_GET_MASK_tag{
	UPDATE_DEV_Header header;
}UPDATE_GET_MASK;

//视频板回复
typedef struct ARM_REPLAYUPDATE_GETMASK_tag{
	UPDATE_DEV_Header header;
	unsigned short m_mask[4];//传送加密方式：~m_mask[0]+1,先取反后加一
}ARM_REPLAYUPDATE_GETMASK;
#pragma pack(pop)
#endif /* PROTOCAL_UPDATEPROGRAM_PROTOCAL_H_ */
