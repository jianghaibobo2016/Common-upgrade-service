#include <iostream>
#include <Logger.h>
#include <string.h>
#include <net/if.h>
#include <net/if_arp.h> /* ARPHRD_ETHER */
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <SetNetworkWithServer.h>
// #include "LocalResource.h"
#include "DevSearch.h"
#include "CrcCheck.h"
#include "UDPNetTrans.h"
#include "UpgradeService.h"
#include "UpgradeServiceConfig.h"
using namespace FrameWork;
//int macAddrGet(UINT8 *mac);
int main(int argc, char *argv[])
{
    InitLogging(argv[0], DEBUG, "./log_");
    #if 0
    SetNetworkTerminal getnetwork;
    getnetwork.IFNAME = new INT8[10];
    const INT8 *ifname = "eth0";
    strncpy(getnetwork.IFNAME, ifname, strlen(ifname));
    // getnetwork.GetNetworkConfig();

    // cout << "MAC::: "<<getnetwork.m_netWorkConfig.macAddr<<endl;
   
    // delete  getnetwork.IFNAME;
    //  char *ip = argv[1];
    // if(CheckNetConfig::GetInstance().CheckGateway( ip, argv[2],argv[3]) == true)
    // if(CheckNetConfig::GetInstance().CheckSubnet( ip, argv[2]) == true)
    
// if(CheckNetConfig::GetInstance().checkMAC( ip) == true)
    // const INT8 *  ifname = "eth0";
    // getnetwork.IFNAME = new INT8 [10];
    // strncpy(getnetwork.IFNAME, ifname, strlen(ifname));
    // cout << "ifname class : " << getnetwork.IFNAME << endl;
    // getnetwork.IFNAME = ifname;
    // if(getnetwork.SetNet(ip, argv[2], argv[3]) == true)
    // IniConfigFile iniconfig;
    // iniconfig.iniFile = new INT8 [32];
    // const INT8 *iniFile = "/dsppa/SET_NET/DSP9903_NET_CONF.ini";
    // strncpy(iniconfig.iniFile, iniFile, strlen(iniFile));
    // if (iniconfig.SetIniConfFile("NETWORK", "netmask", "172.16.10.133") == 1)
    // INT8 buff[1024] = {0};
    // UINT8* mac = new UINT8[14];
    // macAddrGet(mac);
    // cout << "mac: "<< mac<<endl;
    // if (iniconfig.ReadIniConfFile("NETWORK", "macaddr", buff) == true)
    // cout << "strtok: " << strtok(argv[1], ":") << endl;
    // cout << "strtok: " << strtok(NULL, ":") << endl;
    // cout << "strtok: " << strtok(NULL, ":") << endl;
    // cout << "strtok: " << strtok(NULL, ":") << endl;
    // cout << "strtok: " << strtok(NULL, ":") << endl;
    #if 1
    // if(getnetwork.SetNet(SETMAC, "10:22:aa:ff:bb:cc") == true)
// string iniFile = "/dsppa/SET_NET/DSP9903_NET_CONF.ini";
const INT8 *iniFile = "/dsppa/SET_NET/DSP9903_NET_CONF.ini";
INT8 miniFile[32] = {0};
memcpy(miniFile, iniFile, strlen(iniFile));
// if ((getnetwork.SetNet("172.16.10.133", "255.128.0.0", "172.16.16.2")) == true)
// down完up
if (getnetwork.setNetworkConfig("172.16.10.133", NULL, NULL, "34:bb:cc:dd:ee:4f", miniFile) == true)
    // if (getnetwork.setNetworkConfig("172.179.6.4", "255.255.255.240", "172.179.6.2", "a2:bb:cc:dd:ee:2f", miniFile) == true)
    // if (getnetwork.setNetworkConfig("172.16.10.133", "255.255.0.0", "172.16.10.1", "a2:bb:cc:dd:ee:2f", miniFile) == true)
    // if (getnetwork.setNetworkConfig("172.179.6.243", NULL, NULL, "aa:bb:cc:dd:ee:af", miniFile) == true)
    // if ((getnetwork.UpDownNetwork(upStatus)) == true)
    cout << "true" << endl;
else
    cout << "false" << endl;
    #endif
// delete mac;
// if (getnetwork.setNetworkConfig(NULL, NULL, NULL, "20:17:aa:bb:cc:00", miniFile) == true)
    // if (getnetwork.setNetworkConfig("172.179.6.243", NULL, NULL, "a1:bb:cc:dd:ee:4f", miniFile) == true)
    if (getnetwork.setNetworkConfig("172.16.10.134", NULL, NULL, "a0:bb:cc:dd:ee:4f", miniFile) == true)
        cout << "true" << endl;
    else
        cout << "false" << endl;
    if (getnetwork.setNetworkConfig("172.179.6.6", "255.255.255.240", "172.179.6.2", NULL, miniFile) == true)
        cout << "true" << endl;
    else
        cout << "false" << endl;
    if (getnetwork.setNetworkConfig("172.179.6.9", NULL, NULL, "66:bb:cc:dd:ee:4f", miniFile) == true)
        cout << "true" << endl;
    else
        cout << "false" << endl;
    if (getnetwork.setNetworkConfig("172.179.6.13", NULL, NULL, NULL, miniFile) == true)
        cout << "true" << endl;
    else
        cout << "false" << endl;
    delete getnetwork.IFNAME;
    // iniconfig.SetIniConfFile("NETWORK", "gateway", "172.16.10.1f33");
    // iniconfig.SetIniConfFile("NETWORK", "macaddr", "172.16.10.13f3");
    // delete iniconfig.iniFile;
    #if 0
     getnetwork.GetNetworkConfig();
    cout << "ip:1 " << getnetwork.m_netWorkConfig.ipAddr << endl;
    cout << "ip2: " << getnetwork.m_netWorkConfig.gatewayAddr << endl;
    cout << "ip3: " << getnetwork.m_netWorkConfig.macAddr << endl;
    cout << "4: " << getnetwork.m_netWorkConfig.netmaskAddr << endl;
     
    
    // cout <<"ip: "<< getnetwork.m_netWorkConfig.ipAddr<<endl;
    // const string ip = "172.16.10.100";
    // const string netmask = "255.224.0.0";
    // const string gateway = "172.16.10.1";
    // LocalResource::SetLocalIP(ip, netmask, gateway);
    // std::string ip = LocalResource::get_ip();
    // printf("ip: %s: ", ip.c_str());

    // mac address
    // char *this_mac = new char[6];
    // LocalResource::get_mac(this_mac);
    // printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\n", this_mac[0] & 0xff, this_mac[1] & 0xff, this_mac[2] & 0xff, this_mac[3] & 0xff, this_mac[4] & 0xff, this_mac[5] & 0xff);
    return 0;
    #endif
        return 0;
#endif


/* test dev search */
    #if 0
    static string pathXml = "/nand/ServerAPP/video_conf.xml";
    static const INT8* pathVersionFile = "/dsppa/HSversion";
    SetNetworkTerminal getnetwork;
    getnetwork.IFNAME = new INT8[10];
    const INT8 *ifname = "eth0";
    strncpy(getnetwork.IFNAME, ifname, strlen(ifname));

    SetNetworkTerminal testDevSearch(getnetwork);
    DevSearchTerminal devSearchT(&testDevSearch);

    DEV_Reply_GetDevMsg *tmpReMsg ;

    tmpReMsg = devSearchT.getDevMsg(pathXml, pathVersionFile);
    printf("tmpmsg: %p\n", &tmpReMsg);
    // memcmp(de;
    cout << "headtag  " << tmpReMsg->header.HeadTag << endl;
    cout << "HeadCmd  " << tmpReMsg->header.HeadCmd << endl;
    cout << "DataLen  " << tmpReMsg->header.DataLen << endl;
    struct sockaddr_in sin;
    sin.sin_addr.s_addr = tmpReMsg->DevServerIP;
    INT8* devIP = NULL;
    devIP = inet_ntoa(sin.sin_addr);
    cout << "DevIP  " << tmpReMsg->DevIP << endl;
    cout << "DevMask  " << tmpReMsg->DevMask << endl;
    cout << "DevGateway  " << tmpReMsg->DevGateway << endl;
    cout << "DevServerIP  " << devIP << endl;
    cout << "DevServerPort  " << tmpReMsg->DevServerPort << endl;
    cout << "DevMACAddress  " << tmpReMsg->DevMACAddress << endl;
    cout << "DevID  " << tmpReMsg->DevID << endl;
    cout << "DevType  " << tmpReMsg->DevType << endl;
    cout << "HardVersion  " << tmpReMsg->HardVersion << endl;
    cout << "SoftVersion  " << tmpReMsg->SoftVersion << endl;
    cout << "DevName  " << tmpReMsg->DevName << endl;
    // delete tmpReMsg;
    // tmpReMsg = 0;
#endif
    #if 0
    const INT8* filename = argv[1];
    CrcCheck::parser_Package(filename);

    #endif

#if 0
    static const INT8 *pathVersionFile = "/dsppa/HSversion";
    SetNetworkTerminal getnetwork;
    getnetwork.IFNAME = new INT8[10];
    const INT8 *ifname = "eth0";
    strncpy(getnetwork.IFNAME, ifname, strlen(ifname));
    getnetwork.getNetworkConfig();

    SetNetworkTerminal testDevSearch(getnetwork);

    TerminalDevInfo ter(&testDevSearch, pathVersionFile);
    // printf("ter.mac: %u\n", ter.devMAC[1]);
    cout << "ter.mac: " << ter.devMAC << endl;
    cout << "ter.devID: " << ter.devID << endl;
    cout << "ter.devType: " << ter.devType << endl;
    cout << "ter.devHardVersion: " << ter.devHardVersion << endl;
    cout << "ter.devSoftVersion: " << ter.devSoftVersion << endl;
#endif
#if 1
    // UDPNetTrans *udpNetTrans = new UDPNetTrans();
    // udpNetTrans->socketBind(45535);
    // udpNetTrans->socketRunThread();
    // udpNetTrans->socketSelect();
    // static string pathXml = "/nand/ServerAPP/video_conf.xml";
    // static const INT8 *pathVersionFile = "/dsppa/HSversion";
    SetNetworkTerminal getnetwork;
//    getnetwork.IFNAME = new INT8[10];
    const INT8 *ifname = IFNAMETERMINAL;
    getnetwork.setIfname(ifname);
    getnetwork.getNetworkConfig();
    SetNetworkTerminal terminalNet(getnetwork);
    cout << "start ifname : "<<terminalNet.getIfname()<<endl;
    UpgradeService upgradeService(&terminalNet);
    upgradeService.start();


    // DevSearchTerminal devSearchT(&testDevSearch);


    // while(1)
    // {

    cout << "sleeping " << endl;
    // }
    sleep(300);
    // delete udpNetTrans;

#endif 
}
// int macAddrSet(UINT8 *mac)
// {
//     struct ifreq temp;
//     struct sockaddr *addr;

//     int fd = 0;
//     int ret = -1;

//     if ((0 != getuid()) && (0 != geteuid()))
//         return -1;

//     if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
//     {
//         return -1;
//     }

//     strcpy(temp.ifr_name, "eth0");
//     addr = (struct sockaddr *)&temp.ifr_hwaddr;

//     addr->sa_family = ARPHRD_ETHER;
//     memcpy(addr->sa_data, mac, 6);

//     ret = ioctl(fd, SIOCSIFHWADDR, &temp);

//     close(fd);
//     return ret;
// }

// int macAddrGet(UINT8 *mac)
//{
//    struct ifreq temp;
//    struct sockaddr *addr;
//
//    int fd = 0;
//    int ret = -1;
//
//    if ((0 != getuid()) && (0 != geteuid()))
//        return -1;
//
//    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
//    {
//        return -1;
//    }
//
//    strncpy(temp.ifr_name, "eth0" ,IFNAMSIZ - 1);
//    addr = (struct sockaddr *)&temp.ifr_hwaddr;
//
//    addr->sa_family = ARPHRD_ETHER;
//
//    ret = ioctl(fd, SIOCGIFHWADDR, &temp);
//    cout << "ret: "<<ret<<endl;
//    close(fd);
//
//    if (ret < 0)
//        return -1;
//cout << "data: "<<addr->sa_data<<endl;
//    memcpy(mac, addr->sa_data, 6);
//    cout << "mac 1 "<<mac<<endl;
//
//    return ret;
//}
