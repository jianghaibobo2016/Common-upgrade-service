#ifndef CRCCHECK_H
#define CRCCHECK_H
#include "GlobDefine.h"

#pragma pack(push, 1)
typedef struct _PACK_HEAD_S {
	_PACK_HEAD_S() :
			head(), fileSize(0), crcCode(0), m_version() {
	}
	INT8 head[8];
	INT32 fileSize;
	UINT32 crcCode;
	INT8 m_version[64];
	INT8 dependVersion[8];
} PACK_HEAD;
#pragma pack(pop)

static const INT32 BUFFER_SIZE = 1024 * 4;
static const INT8* HEAD = "UPGRADE";
//static const INT8* NEW_FILE = "upgradefiletar.tar";
class CrcCheck {
public:
	CrcCheck();
	~CrcCheck() {
	}

	static INT32 parser_Package(const INT8* filename, INT8 *newVersion,
			INT8 *itemName, INT8 *dependVersion);
private:
	static UINT32 crc32(UINT32 crc, UINT8 *buff, UINT32 size);
	static const UINT32 crc32_table[256];
};

#endif
