/*
 * GlobDefine.h
 *
 *  Created on: Mar 23, 2018
 *      Author: jhb
 */
#ifndef _GLOBDEFINE_H
#define _GLOBDEFINE_H

// Defines unsigned types
typedef unsigned char           UINT8;
typedef unsigned short          UINT16;
typedef unsigned int            UINT32;
typedef unsigned long long      UINT64;

// Defines signed types
typedef char                    INT8;
typedef signed short            INT16;
typedef int                     INT32;
typedef signed long long        INT64;

const INT32 retOk = 0;
const INT32 retError = -1;


#ifdef FILE_LINE
#undef FILE_LINE
#endif
#ifndef FILE_LINE
#define FILE_LINE (LOG(DEBUG) << "check")
#endif

#ifdef VALID
#undef VALID
#endif

#ifndef VALID
#define VALID             (0)
#endif

#ifdef INVALID 
#undef INVALID
#endif

#ifndef INVALID
#define INVALID           (1)
#endif

#endif /* #ifndef _GLOBDEFINE_H */
