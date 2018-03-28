/*
 * FileTrans.cpp
 *
 *  Created on: Mar 23, 2018
 *      Author: jhb
 */

#include <iostream>
#include "FileTrans.h"
#include "UpgradeServiceConfig.h"
using namespace std;
FileTrans::FileTrans()
    : startPosition(0),
      fileTotalSize(0),
      sendLen(0),
      UpPercentageOld(0),
      UpPercentage(0),
      fileRemained(0)
{}
FileTrans::~FileTrans()
{}
UINT32 FileTrans::iniPosition(UpFileAttrs &upFileAttr)
{
    startPosition = 0;
    fileRemained = upFileAttr.getNewSoftFileSize();
    cout << "total  size :: "<<fileRemained<<endl;
    fileTotalSize = fileRemained;
    if (fileRemained >= FileTransLen)
        sendLen = FileTransLen;
    else
        sendLen = fileRemained;

    UpPercentage = 1;
    return fileRemained;
}
FileTrans& FileTrans::changeRemainedPos()
{
    // if (!startPosition)
    // {
    //     return 0;
    // }
    if (fileRemained >= FileTransLen)
    {
        startPosition += FileTransLen;
        fileRemained -= FileTransLen;
        cout << "fileRemainedmmmmmmmmm: "<<fileRemained<<endl;
        if (fileRemained < FileTransLen)
            sendLen = fileRemained;
        else
            sendLen = FileTransLen;
        cout << "request len : "<< sendLen <<__FUNCTION__<<endl;
    }
    else if ((fileRemained != 0) && (fileRemained < FileTransLen))
    {
        startPosition = 0xFFFFFFFF;
        sendLen = 0;
        cout << "sendlenffffffffffffffffff: "<<sendLen<<" "<<__FUNCTION__<<endl;
        fileRemained -= fileRemained;
    }/* else if (fileRemained == 0)
    {
        startPosition = 0xFFFFFFFF;
        cout << "endllllllllllllllllll"<<endl;
        fileRemained = 0;
        sendLen = 0;
    } */

    return *this;
}

INT32 FileTrans::setPersentage()
{
    cout << "fileremain: "<<fileRemained<<" "<<"toal :: "<<fileTotalSize<<__FUNCTION__<<endl;
    if (fileTotalSize == 0)
        return retError;
    if (fileRemained == 0)
        UpPercentage = 50;
    else
    {
        float quotient = ((float)fileRemained / fileTotalSize) * 100;
        cout << "quotioent: " << quotient << endl;
        if (quotient < 100 && quotient >= 80)
            UpPercentage = 10;
        else if (quotient < 80 && quotient >= 60)
            UpPercentage = 20;
        else if (quotient < 60 && quotient >= 40)
            UpPercentage = 30;
        else if (quotient < 40 && quotient >= 20)
            UpPercentage = 40;
        else if (quotient < 20 && quotient >= 0)
            UpPercentage = 50;
    }
    cout << "UpPercentage : "<<UpPercentage<<endl;
    return retOk;
}

UINT32 FileTrans::clearFileTrans()
{
    this->startPosition = 0;
    this->fileTotalSize = 0;
    this->fileRemained = 0;
    this->sendLen = 0;
    this->UpPercentage = 0;
    this->UpPercentageOld = 0;
    return retOk;
}


