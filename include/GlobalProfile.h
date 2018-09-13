/*
 * GlobalProfile.h
 *
 *  Created on: Mar 23, 2018
 *      Author: jhb
 */

#ifndef GLOBALPROFILE_H_
#define GLOBALPROFILE_H_


#include "SharedXML.h"
#include "HandleXML.h"
#include <stdint.h>
#include <stdarg.h>
#include "xmlParser.h"
#include <map>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

#define PROFILE_DIR "/nand/ServerAPP"
#define PROFILE_PATH   "/nand/ServerAPP/video_conf.xml"
// #define LOGINUSERS_PATH  "/nand/ServerAPP/video_conf.xml"   //"/etc/dsppa/video_conf.xml"


class GlobalProfile: public SharedXML {
private:
	class Profile: public SharedXML {
	public:
		Profile(const string &path) {
			_strcurpath = path;
			_bChanged = false;
			XMLResults result;
			_top = XMLNode::parseFile(path.c_str(), "VideoTerminalProfile", &result);
			if (_top.isEmpty()) {
				_top = XMLNode::createXMLTopNode("VideoTerminalProfile");
				_bChanged = true;
			}

		}
		~Profile() {
			if (_bChanged) {
				XMLError ret = _top.writeToFile(_strcurpath.c_str());
			}
		}
		const string getString(const char* section, const char* strKey, const string& strDefualtValue) {
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
				const char * pTemp = curnode.getText();
				if (pTemp)
					strValue = pTemp;
				else
					strValue = "";
			}
			return strValue;
		}
		 int getInt(const char* section, const char* strKey, const int defualtvalue) {
			string strValue;
			char strNum[10];
			sprintf(strNum, "%d", defualtvalue);
			strValue = strNum;
			strValue = getString(section, strKey, strValue);
			return atoi(strValue.c_str());
		}
		 unsigned short getUShort(const char* section, const char* strKey, const unsigned short defualtvalue) {
			return getInt(section, strKey, (int) defualtvalue);
		}

		void updateValue(const char* section, const char* strKey, const char* fmt, ...) {
			_bChanged = true;
			char strValue[2048];
			va_list vl;
			va_start(vl, fmt);
			vsnprintf(strValue, 2048, fmt, vl);
			va_end(vl);
			XMLNode childnode = _top.getChildNode(section);
			if (childnode.isEmpty()) { //section : empty child node
				childnode = _top.addChild(section); //add section
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

	private:
		string _strcurpath;
		XMLNode _top;
		bool _bChanged;
	};
	typedef HandleXML<Profile> ProfilePtr;

	struct sUserInfo {
		string pass;
		string cardid;
	};
public:
	GlobalProfile();
	virtual ~GlobalProfile();

	static HandleXML<GlobalProfile> instance() {
		if (!g_Profile) {
			g_Profile = new GlobalProfile();
		}
		return g_Profile;
	}
	static HandleXML<GlobalProfile> g_Profile;


	void GetTCPCommServerIP(string& ip, uint16_t& port);
	void SetTCPCommServerIP(const string& ip, const uint16_t& port);


private:

	void loadFromXML();
	//TCP server
	string _TCPServerIP;
	uint16_t _TCPServerPort;

};
typedef HandleXML<GlobalProfile> GlobalProfilePtr;


#endif /* GLOBALPROFILE_H_ */
