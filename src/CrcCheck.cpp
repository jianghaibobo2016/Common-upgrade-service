#include <string.h>
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include "Logger.h"
#include "CrcCheck.h"
#include "DevSearch.h"
#include "Upgrade.h"
#include "UpgradeServiceConfig.h"
#include "RCSP.h"
using namespace std;
using namespace FrameWork;
CrcCheck::CrcCheck() {
}

INT32 CrcCheck::parser_Package(const INT8 *filename, INT8 *newVersion,
		INT8 *itemName, INT8 *dependVersion) {
	cout << "today test seg  fault !!!!!!!!!!!!!!!!!!!!!!!!!-------- 2 "<<endl;
	if (filename == NULL) {
		return retError;
	}
	SmartPtr<PACK_HEAD> pack_head(new PACK_HEAD);
	INT32 n_read = 0;
	INT32 n_write = 0;
	INT32 count_write = 0;
	UINT32 crc = 0xffffffff;

	INT32 packhead_len = sizeof(PACK_HEAD);

	UINT8 buff[BUFFER_SIZE];
	bzero(buff, BUFFER_SIZE);
	FILE *src_fd = fopen(filename, "rb");
	if (src_fd == NULL) {
		printf("errr: %s\n", strerror(errno));
		printf("ERROR : Can not open file : %s !\n", filename);
		return retError;
	}
	cout << "today test seg  fault !!!!!!!!!!!!!!!!!!!!!!!!!-------- 31 "<<endl;
	n_read = fread(buff, 1, packhead_len, src_fd);
	memcpy(pack_head.get(), buff, packhead_len);
	if ((strncmp(pack_head->head, HEAD, 8)) != 0) {
		printf("ERROR : File's head is not right !\n");
		fclose(src_fd);
		return retError;
	} else
		;
	cout << "today test seg  fault !!!!!!!!!!!!!!!!!!!!!!!!!-------- 32  "<<strlen(pack_head->dependVersion)<<endl;
	/*Judge the depend version*/
	if (strlen(pack_head->dependVersion) != 0) {
		cout << "today test seg  fault !!!!!!!!!!!!!!!!!!!!!!!!!-------- 33  "<<strlen(pack_head->dependVersion)<<endl;
		INT8 depLocalVer[7] = { 0 };
		UpgradeDSP::getVersionByItemName(itemName, depLocalVer);
		if (strncmp(pack_head->dependVersion, depLocalVer, strlen(depLocalVer))
				> 0) {
			fclose(src_fd);
			Logger::GetInstance().Fatal(
					"%s(): Request %s version must not be lower than %s !",
					__FUNCTION__, itemName, pack_head->dependVersion);
			return retError;
		}
		cout << "today test seg  fault !!!!!!!!!!!!!!!!!!!!!!!!!-------- 34 "<<endl;
	} else if (strncmp(pack_head->m_version + strlen(TerminalDevType) + 1,
			ProductItemName, strlen(ProductItemName)) == 0) {
		cout << "today test seg  fault !!!!!!!!!!!!!!!!!!!!!!!!!-------- 35 "<<endl;
		fclose(src_fd);
		Logger::GetInstance().Fatal(
				"%s(): Upgrade item %s need a depend version !", __FUNCTION__,
				itemName);
		return retError;
	}
	cout << "today test seg  fault !!!!!!!!!!!!!!!!!!!!!!!!!-------- 36 "<<endl;
	/*Judge the depend version*/
	INT8 tmpVersion[32] = { 0 };
	if (newVersion != NULL) {
		cout << "today test seg  fault !!!!!!!!!!!!!!!!!!!!!!!!!-------- 37 "<<endl;
		memcpy(tmpVersion, pack_head->m_version, strlen(pack_head->m_version));
		memcpy(newVersion,
				tmpVersion + strlen(TerminalDevType) + 2 + strlen(itemName), 6);
	}
	FILE *dst_fd = fopen(newTarPackage, "wb+");
	cout << "today test seg  fault !!!!!!!!!!!!!!!!!!!!!!!!!-------- 38 "<<endl;
	while (1) {
		bzero(buff, BUFFER_SIZE);
		n_read = fread(buff, 1, BUFFER_SIZE, src_fd);
		if (n_read > 0) {
			n_write = fwrite(buff, 1, n_read, dst_fd);
			if (n_write == n_read) {
				crc = crc32(crc, buff, n_write);
				count_write += n_write;
			} else {
				printf("ERROR : Write into buff from file failed !\n");
				fclose(src_fd);
				fclose(dst_fd);
				return retError;
			}
		} else if (n_read < 0) {
			printf("ERROR : Read from file failed !\n");
			fclose(src_fd);
			fclose(dst_fd);
			return retError;
		} else
			break;
	}

	if (crc != pack_head->crcCode) {
		printf("ERROR : Crc32 check failed !\n");
		fclose(src_fd);
		fclose(dst_fd);
		return retError;
	} else
		;

	fclose(src_fd);
	fclose(dst_fd);
	return 0;
}

INT32 CrcCheck::getDevModules(const INT8* filename,
		map<INT32, DEV_MODULES_TYPE>&devModules) {
	if (filename == NULL) {
		return retError;
	}
	SmartPtr<PACK_HEAD> pack_head(new PACK_HEAD);
	INT32 n_read = 0;
	INT32 count_write = 0;

	INT32 packhead_len = sizeof(PACK_HEAD);
	UINT8 buff[BUFFER_SIZE];
	bzero(buff, BUFFER_SIZE);
	FILE *src_fd = fopen(filename, "rb");
	if (src_fd == NULL) {
		printf("errr: %s\n", strerror(errno));
		printf("ERROR : Can not open file : %s !\n", filename);
		return retError;
	}

	n_read = fread(buff, 1, packhead_len, src_fd);
	memcpy(pack_head.get(), buff, packhead_len);
	if ((strncmp(pack_head->head, HEAD, 8)) != 0) {
		printf("ERROR : File's head is not right !\n");
		fclose(src_fd);
		return retError;
	} else
		fclose(src_fd);
	cout << "today test !!!!!!!!!!!!!!!!!!!!!!!!!-------- 4 " << endl;
	INT32 nums = 0;
	for (; nums < TerminalDevsMaxNum; nums++) {
		if (strlen(pack_head->TerminalDevs[nums]) != 0) {
			cout << "pack" << pack_head->TerminalDevs[nums] << endl;

			INT32 retCMP = strncmp(pack_head->TerminalDevs[nums], AMPLIFIER,
					strlen(AMPLIFIER));
			if (retCMP == 0) {
				cout << "today test !!!!!!!!!!!!!!!!!!!!!!!!!-------- 5 "
						<< endl;

				devModules[nums + 1] = DEV_AMPLIFIER;
			} else if (strncmp(pack_head->TerminalDevs[nums], PAGER, strlen(PAGER)) == 0)
				devModules[nums + 1] = DEV_PAGER;
			else
				devModules[nums + 1] = INVALID_TYPE;
		} else
			break;
	}
	cout << "today test !!!!!!!!!!!!!!!!!!!!!!!!!-------- 6 " << devModules[1]
			<< endl;
	return retOk;
}

UINT32 CrcCheck::crc32(UINT32 crc, UINT8 *buff, UINT32 size) {
	if (((NULL == buff) || (NULL == crc32_table)) == 1) {
		printf("[CRC32 ERR] : NULL POINT INPUTTED!\n");
		return -1;
	}
	UINT32 i;
	for (i = 0; i < size; i++) {
		crc = crc32_table[(crc ^ buff[i]) & 0xff] ^ (crc >> 8);
	}
	return crc;
}

UINT32 const CrcCheck::crc32_table[256] = { 0, 102971031, 96753217, 65495254,
		34248685, 69837178, 130990508, 32372539, 68497370, 37099853, 30505371,
		133320460, 102742071, 3968672, 64745078, 100194529, 27813083, 126571084,
		74199706, 38732813, 61010742, 92390817, 107390327, 4559840, 96291585,
		60718486, 7937344, 106572759, 129490156, 26536571, 41126573, 72399930,
		55626166, 91488033, 110403575, 12036448, 22699611, 125397196, 77465626,
		45956749, 122021484, 23515387, 42053677, 77760186, 89092481, 57427734,
		9119680, 111677783, 49851757, 81501178, 121436972, 18861499, 15874688,
		114363415, 87452865, 51731030, 116228791, 13546528, 53073142, 84599393,
		82253146, 46408653, 19088155, 117470604, 111252332, 8562171, 57001261,
		88535994, 78316673, 42480150, 24072896, 122446935, 45399222, 77040161,
		124840695, 22273120, 12462939, 110960076, 91913498, 56183693, 118093751,
		19579168, 47030774, 82745185, 84107354, 52451021, 13055515, 115605644,
		52222061, 88076026, 114855468, 16496827, 18239360, 120944919, 80878017,
		49360726, 99703514, 64121933, 3476635, 102119948, 133942583, 30997408,
		37722998, 68988385, 31749376, 130499479, 69215041, 33756630, 65987309,
		97375354, 103462060, 623163, 72957441, 41552022, 27093056, 129916631,
		106146284, 7380859, 60293037, 95734074, 4985307, 107947852, 92817306,
		61567245, 38176310, 73773217, 126013559, 27387616, 79764919, 48376608,
		17124342, 119962977, 114002522, 15252685, 51371035, 86829708, 13906541,
		116851962, 84960300, 53695163, 48145792, 83727127, 119206849, 20563286,
		90798444, 55201787, 11349805, 109975994, 123989633, 21026838, 44546240,
		75796055, 24925878, 123691041, 79167735, 43726432, 58114395, 89520076,
		112367386, 9544077, 126997505, 28500630, 39158336, 74888407, 94061548,
		62420347, 6231469, 108798778, 59046875, 94883148, 104902042, 6527757,
		26111030, 128801441, 71973495, 40439008, 104444122, 1738317, 66971291,
		98488332, 70461239, 34607520, 32993654, 131352545, 36478720, 68135319,
		132696385, 30146518, 2492653, 101006970, 98721452, 63006779, 39882459,
		71546956, 128243866, 25685517, 6953270, 105459617, 95309687, 59603424,
		108372225, 5674902, 61994816, 93503959, 75445996, 39583867, 29057197,
		127424058, 63498752, 99343511, 101497921, 3115734, 29523437, 132205434,
		67513260, 35986747, 131974618, 33485645, 35230619, 70952204, 97997367,
		66348192, 1246326, 103822049, 19941229, 118714874, 83104044, 47654843,
		54186112, 85583383, 117343937, 14528598, 86337719, 50748960, 14761718,
		113379425, 120586074, 17615309, 48998683, 80256908, 9970614, 112923937,
		89945591, 58671968, 43168859, 78742220, 123134490, 24499341, 76352620,
		44972795, 21584429, 124415162, 109550465, 10792214, 54775232, 90241879 };
