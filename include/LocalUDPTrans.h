/*
 * LocalUDPTrans.h
 *
 *  Created on: Apr 16, 2018
 *      Author: jhb
 */

#ifndef LOCALUDPTRANS_H_
#define LOCALUDPTRANS_H_
#include "NetTrans.h"
class LocalUDPTrans: public NetTrans {

public:
	LocalUDPTrans();
	~LocalUDPTrans();

	sockaddr_in *getAddr() {
		return &addr;
	}

	socklen_t *getAddrLen() {
		return &addrLen;
	}

private:
	struct sockaddr_in addr;
	socklen_t addrLen;
};

#endif /* LOCALUDPTRANS_H_ */
