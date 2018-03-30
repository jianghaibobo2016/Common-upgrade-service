#ifndef DEVSEARCH_H
#define DEVSEARCH_H

#include <SetNetworkWithServer.h>
#include "PCTransProtocal.h"
#include "xmlParser.h"
#include "UpgradeServiceConfig.h"


class DevSearch
{
public:
    DevSearch(){}
    virtual ~DevSearch(){}
    virtual DEV_Reply_GetDevMsg *getDevMsg(const string &pathXML, const INT8 *pathVersionFile) = 0;
};
class DevSearchTerminal : public DevSearch
{
public:
public:
    DevSearchTerminal(SetNetworkTerminal *setNetworkTerminal);
    virtual ~DevSearchTerminal();

    DEV_Reply_GetDevMsg *getDevMsg(const string &pathXML, const INT8 *pathVersionFile);
    static bool getSoftwareVersion(const INT8* item, INT8* version, const INT8* pathVersionFile);

    DEV_Reply_GetDevMsg *devReplyMsg;
    //modify to private !
    SetNetworkTerminal *setNetworkTerminal;
private:

};


class XMLParser
{
public:
    XMLParser(const string &path);
    ~XMLParser(){}

    const string path;

    bool xmlInit();
    const string getString(const char *section, const char *strKey, const string &strDefualtValue);
    const int getInt(const char* section, const char* strKey, const int defualtvalue);
    const unsigned short getUShort(const char* section, const char* strKey, const unsigned short defualtvalue);
    void updateServerNetConfig(const char *section, const char *strKey, const char *fmt, ...);

  private:
    string _strcurpath;
    XMLNode _top;
    bool _bChanged;
};
#endif /* DEVSEARCH_H */
