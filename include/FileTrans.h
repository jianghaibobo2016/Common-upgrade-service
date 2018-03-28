/*
 * FileTrans.h
 *
 *  Created on: Mar 23, 2018
 *      Author: jhb
 */

#ifndef FILETRANS_H_
#define FILETRANS_H_


#include "GlobDefine.h"
#include "Uncopyable.h"
#include "UpFileAttrs.h"

class FileTrans : private Uncopyable
{
public:
    FileTrans();
    ~FileTrans();

    UINT32 iniPosition(UpFileAttrs &upFileAttr);

    void setFileRemained(const UINT32 remainedSize){fileRemained = remainedSize;}
    void setOldPercent(const UINT32 oldPercent){UpPercentageOld = oldPercent;}

    UINT32 getStartPos(){return startPosition;}
    UINT32 getSendLen(){return sendLen;}
    UINT32 getOldPercent(){return UpPercentageOld;}
    UINT32 getNewPercent(){return UpPercentage;}
    UINT32 getFileRemainedLen(){return fileRemained;}

    FileTrans& changeRemainedPos();
    INT32 setPersentage();

    UINT32 clearFileTrans();
private:
    UINT32 startPosition;
    UINT32 fileTotalSize;
    UINT32 sendLen;
    INT32 UpPercentageOld;
    INT32 UpPercentage;
    UINT32 fileRemained;
};

#endif /* FILETRANS_H_ */
