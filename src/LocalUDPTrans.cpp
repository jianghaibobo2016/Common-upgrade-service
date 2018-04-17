/*
 * LocalUDPTrans.cpp
 *
 *  Created on: Apr 16, 2018
 *      Author: jhb
 */
#include <netinet/in.h>
#include <string.h>
#include "LocalUDPTrans.h"
#include "UpgradeServiceConfig.h"


LocalUDPTrans::LocalUDPTrans() :
		NetTrans(UNIXAF, DATAGRAM, 0), addrLen(sizeof(struct sockaddr_in)) {
	memset(&addr,0,sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY); //任何主机地址
	addr.sin_port = htons(UpLocalPort);
}

LocalUDPTrans::~LocalUDPTrans() {

}
