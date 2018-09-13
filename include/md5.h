/*
 * md5.h
 *
 *  Created on: 2018年3月13日
 *      Author: dsppa
 */

#ifndef MD5_MD5_H_
#define MD5_MD5_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "GlobDefine.h"

#define READ_DATA_SIZE  1024
#define MD5_SIZE        16
#define MD5_STR_LEN     (MD5_SIZE * 2)
typedef struct
{
    unsigned int count[2];
    unsigned int state[4];
    UINT8 buffer[64];
} MD5_CTX;


#define F(x,y,z) ((x & y) | (~x & z))

#define G(x,y,z) ((x & z) | (y & ~z))

#define H(x,y,z) (x^y^z)

#define I(x,y,z) (y ^ (x | ~z))

#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))

#define FF(a,b,c,d,x,s,ac) \
{ \
    a += F(b,c,d) + x + ac; \
    a = ROTATE_LEFT(a,s); \
    a += b; \
}
#define GG(a,b,c,d,x,s,ac) \
{ \
    a += G(b,c,d) + x + ac; \
    a = ROTATE_LEFT(a,s); \
    a += b; \
}
#define HH(a,b,c,d,x,s,ac) \
{ \
    a += H(b,c,d) + x + ac; \
    a = ROTATE_LEFT(a,s); \
    a += b; \
}
#define II(a,b,c,d,x,s,ac) \
{ \
    a += I(b,c,d) + x + ac; \
    a = ROTATE_LEFT(a,s); \
    a += b; \
}
void MD5Init(MD5_CTX *context);
void MD5Update(MD5_CTX *context, UINT8 *input, unsigned int inputlen);
void MD5Final(MD5_CTX *context, UINT8 digest[16]);
void MD5Transform(unsigned int state[4], UINT8 block[64]);
void MD5Encode(UINT8 *output, unsigned int *input, unsigned int len);
void MD5Decode(unsigned int *output, UINT8 *input, unsigned int len);

bool GetFileMD5(const INT8 *file_path,UINT8 *md5_str);
//bool ChartoHex(INT8 *md5_str, INT8 *md5_str_end);
#endif /* MD5_MD5_H_ */
