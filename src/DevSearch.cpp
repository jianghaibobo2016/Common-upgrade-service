#include "Logger.h"
#include "DevSearch.h"
#include <stdarg.h>

using namespace FrameWork;

Mutex DevSearchTerminal::mutex;

DevSearchTerminal::DevSearchTerminal(SetNetworkTerminal *setNetworkTerminal) :
		setNetworkTerminal(setNetworkTerminal) {
	devReplyMsg = new DEV_Reply_GetDevMsg();
}
DevSearchTerminal::~DevSearchTerminal() {
	delete devReplyMsg;
	devReplyMsg = 0;
}
bool DevSearchTerminal::getSoftwareVersion(const INT8 *item, INT8 *version,
		const INT8* pathVersionFile) {
	cout << "This lock of auto lock of func :  "<<__FUNCTION__<<"()"<<endl;
	AutoLock autoLock(&mutex);

	INT32 get = retOk;
	const INT32 readBuffSize = 128;
	INT8 readBuff[readBuffSize] = { 0 };
	INT32 item_len = strlen(item);

	FILE *fp = fopen(pathVersionFile, "r");
	while ((fgets(readBuff, readBuffSize, fp)) != NULL) {
		if ((strncmp(readBuff, item, item_len)) == 0) {
			if ((strncmp(readBuff + item_len, "=", 1)) != 0) {
				Logger::GetInstance().Error(
						"%s() : ERROR : Incomplete item : %s.\n", __FUNCTION__,
						item);
				break;
			} else {
				memcpy(version, "V", 1);
				memcpy(version + 1, readBuff + item_len + 1, 5);
				version[6] = '\0';
				get = retOk;
				break;
			}
		}
	}
	fclose(fp);

	if (get == retOk) {
		// printf("Item : %s version : %s.\n", item, version);
		return true;
	} else
		return false;
}


DEV_Reply_GetDevMsg *DevSearchTerminal::getDevMsg(const string &pathXML,
		const INT8 *pathVersionFile) {
	if (setNetworkTerminal->getNetworkConfig() != true) {
		Logger::GetInstance().Error("Get network config failed !");
		return NULL;
	}
	INT8 mac[13] = { 0 };
	strcpy(mac,
			SetNetworkTerminal::castMacToChar13(mac,
					setNetworkTerminal->getNetConfStruct().macAddr));
	mac[12] = '\0';
	XMLParser xmlParser(pathXML);
	xmlParser.xmlInit();
	string serverIP = xmlParser.getString("TCPServer", "ServerIP",
			"172.16.0.228");
	UINT16 serverPort = xmlParser.getInt("TCPServer", "ServerPort", 7001);
	string devName = xmlParser.getString("TerminalInfo", "TerminalName",
			"Terminal");
	INT8 version[7] = { 0 };
	getSoftwareVersion(ProductVersionName, version, pathVersionFile);

	//
	devReplyMsg->header.HeadTag = 0x0101FBFC;
	devReplyMsg->header.HeadCmd = 0x0001;
	devReplyMsg->header.DataLen = 0x00BF;
	devReplyMsg->DevIP = inet_addr(
			setNetworkTerminal->getNetConfStruct().ipAddr.c_str());
	devReplyMsg->DevMask = inet_addr(
			setNetworkTerminal->getNetConfStruct().netmaskAddr.c_str());
	devReplyMsg->DevGateway = inet_addr(
			setNetworkTerminal->getNetConfStruct().gatewayAddr.c_str());
	devReplyMsg->DevServerIP = inet_addr(serverIP.c_str());
	devReplyMsg->DevServerPort = serverPort;
	strcpy(devReplyMsg->DevMACAddress, mac);
	strncpy(devReplyMsg->DevType, TerminalDevType, strlen(TerminalDevType));
	strncpy(devReplyMsg->DevID, TerminalDevTypeID, strlen(TerminalDevTypeID));
	strcpy(devReplyMsg->DevID + strlen(TerminalDevTypeID), mac);
	INT8 hardVersion[8] = { 0 };
	DevSearchTerminal::getSoftwareVersion("hardware_version", hardVersion,
			pathVersionFile);
	strcpy(devReplyMsg->HardVersion, hardVersion);
	strcpy(devReplyMsg->SoftVersion, version);
	strcpy(devReplyMsg->DevName, devName.c_str());

	return devReplyMsg;
}

XMLParser::XMLParser(const string &path) :
		path(path), _strcurpath(), _top(), _bChanged(false) {
}

bool XMLParser::xmlInit() {
	XMLResults result;
	_bChanged = false;
	_top = XMLNode::parseFile(path.c_str(), "VideoTerminalProfile", &result);
	if (_top.isEmpty()) {
		_top = XMLNode::createXMLTopNode("VideoTerminalProfile");
		_bChanged = true;
	}
	return true;
}
const string XMLParser::getString(const char *section, const char *strKey,
		const string &strDefualtValue) {
	XMLNode childnode = _top.getChildNode(section);
	if (childnode.isEmpty()) {
		childnode = _top.addChild(section);
		_bChanged = true;
	}
	string strValue = strDefualtValue;
	XMLNode curnode = childnode.getChildNode(strKey);
	if (curnode.isEmpty()) {
		XMLNode child = childnode.addChild(strKey);
		child.addText(strValue.c_str());
		_bChanged = true;
	} else {
		const char *pTemp = curnode.getText();
		if (pTemp)
			strValue = pTemp;
		else
			strValue = "";
	}
	return strValue;
}
const int XMLParser::getInt(const char *section, const char *strKey,
		const int defualtvalue) {
	string strValue;
	char strNum[10];
	sprintf(strNum, "%d", defualtvalue);
	strValue = strNum;
	strValue = getString(section, strKey, strValue);
	return atoi(strValue.c_str());
}
const unsigned short XMLParser::getUShort(const char *section,
		const char *strKey, const unsigned short defualtvalue) {
	return getInt(section, strKey, (int) defualtvalue);
}

void XMLParser::updateServerNetConfig(const char *section, const char *strKey,
		const char *fmt, ...) {
	// _bChanged = true;
	char strValue[2048];
	va_list vl;
	va_start(vl, fmt);
	vsnprintf(strValue, 2048, fmt, vl);
	va_end(vl);
	XMLNode childnode = _top.getChildNode(section);
	if (childnode.isEmpty()) {                      //section : empty child node
		childnode = _top.addChild(section);        //add section
		XMLNode node = childnode.addChild(strKey); //add key
		node.addText(strValue);
	} else { //section: find ok
		XMLNode node = childnode.getChildNode(strKey);
		if (node.isEmpty()) { //strkey is empty
			node = childnode.addChild(strKey);
			node.addText(strValue);
		} else {
			node.updateText(strValue);
		}
	}
}
