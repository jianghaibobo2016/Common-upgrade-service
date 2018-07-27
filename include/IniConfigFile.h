/*
 * IniConfigFile.h
 *
 *  Created on: Jul 19, 2018
 *      Author: jhb
 */

#ifndef INICONFIGFILE_H_
#define INICONFIGFILE_H_

#include "SetNetwork.h"
#include "Mutex.h"
class IniConfigFile {
//	static Mutex *mutex;
//	friend class SetNetworkTerminal;
public:
	IniConfigFile(const INT8 *inifile);
	~IniConfigFile() {
	}
	bool readIniConfFile(const INT8 *section, const INT8 *key, INT8 *value,
			INT32 valueLen);
	INT32 setIniConfFile(const INT8 *section, const INT8 *key,
			const INT8 *value);
private:
	const INT8 *_iniConfigFile;
	INT32 load_ini_file(const INT8 *file, INT8 *buf, INT32 *file_size);
	INT32 parse_file(const INT8 *section, const INT8 *key, const INT8 *buf,
			INT32 *sec_s, INT32 *sec_e, INT32 *key_s, INT32 *key_e,
			INT32 *value_s, INT32 *value_e);

	INT32 inline newline(INT8 c) {
		return ('\n' == c || '\r' == c) ? 1 : 0;
	}
	INT32 inline end_of_string(INT8 c) {
		return '\0' == c ? 1 : 0;
	}
	INT32 inline left_barce(INT8 c) {
		return LEFT_BRACE == c ? 1 : 0;
	}
	INT32 inline isright_brace(INT8 c) {
		return RIGHT_BRACE == c ? 1 : 0;
	}
};

#endif /* INICONFIGFILE_H_ */
