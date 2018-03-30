#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <Logger.h>
#include "NetTrans.h"
#include <iostream>
using namespace std;
using namespace FrameWork;

SocketException::SocketException(const std::string &message, bool inclSysMsg /* = true */) throw()
    : m_message(message), m_errorcode(0)
{
    if (inclSysMsg)
    {
        m_errorcode = errno;
        m_message += ": ";
        m_message += strerror(m_errorcode);
    }
}

NetTrans::NetTrans(int domain, int type, int protocol)
:port(0)
{
    if ((m_socket = ::socket(domain, type, protocol)) < 0)
        throw SocketException("Socket creation failed (socket)");
    int nRecvBuf=32*1024;//设置为32K
    setsockopt(m_socket,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
    //发送缓冲区
    int nSendBuf=32*1024;//设置为32K
    setsockopt(m_socket,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));
}
//NetTrans::NetTrans()
//:m_socket(0),
// port(0)
//{}
NetTrans::~NetTrans()
{
    cout <<"netclose"<<endl;
    close(m_socket);
}

void NetTrans::socketBind(UINT16 localPort)
{
    // cout << "localAddress "  << localAddress << endl;
    cout << "port: "<<localPort<<endl;
    struct sockaddr_in tmp_server_addr;
    // cout <<"check 1"<<endl;
    memset((void *)&tmp_server_addr, 0, sizeof(struct sockaddr_in));
    tmp_server_addr.sin_family = AF_INET;
    tmp_server_addr.sin_addr.s_addr = htons(INADDR_ANY); /* INADDR_ANY */
    tmp_server_addr.sin_port = htons(localPort);
    // st_socket_manage->m_udp_attrs.m_server_addr = tmp_server_addr; //服务器端地址
    //将地址结构绑定到套接字上./
    int ret = bind(m_socket, (struct sockaddr *)&tmp_server_addr,
                   sizeof(tmp_server_addr));
    if (ret < 0)
    {
        Logger::GetInstance().Error("%s() : Set of local address and port failed !", __FUNCTION__);
        throw SocketException("Set of local address and port failed (sctp_bindx)");
        perror("bind error");
    }
    cout << "retbind: "<<ret<<endl;
}
