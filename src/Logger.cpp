/*
 * Logger.cpp
 *
 *  Created on: 2013Äê9ÔÂ10ÈÕ
 *      Author: liu wenhui
 */

#include "Logger.h"
#include <iostream>
#include <cstring>
#include <time.h>
#include <cstdarg>
#include <cstdlib>
#include <assert.h>
#include "FileHelper.h"
#include "Mutex.h"

namespace FrameWork
{
Mutex LogMessage::mutex;
static DP_C_S8 _defaltFolder[] = "/var/tmp/";
static DP_C_S8 _appName[MaxFilePathLen];
static DP_C_S8 _appFolder[MaxFilePathLen];
static DP_C_S8 _destFolder[MaxFilePathLen];
static DP_C_S8 _destPrefix[MaxFilePathLen];
static LogLevel _destLevel;
static DP_C_S8 _levelInfos[][16] = {
    "Debug", "Info", "Warn", "Error", "Fatal"};
const int BUFFER_SIZE = 8196;
static DP_C_S8 _gBuffer[BUFFER_SIZE];
void combine_folder(DP_C_S8 **destpath, DP_C_S8 *basefolder, DP_C_S8 *relativefolder)
{
    int lenb = strlen(basefolder);
    int lenr = strlen(relativefolder);
    DP_C_S8 *pret = (DP_C_S8 *)malloc((lenb + lenr + 1) * sizeof(DP_C_S8));
    int pos = lenb - 1;
    memset(pret, 0, lenb + lenr + 1);
    while (pos > 0 && (basefolder[pos] != '/'))
        pos--;
    strncpy(*destpath, basefolder, pos + 1);
    if (relativefolder[0] == '\\' || relativefolder[0] == '/')
    {
        strncpy(*destpath + pos + 1, relativefolder + 1, lenr - 1);
    }
    else
    {
        strncpy(*destpath + pos + 1, relativefolder, lenr);
    }
}

static void InitPaths(const DP_C_S8 *filename, const DP_C_S8 *destFolder)
{
    memset(_appName, 0, MaxFilePathLen);
    memset(_appFolder, 0, MaxFilePathLen);
    memset(_destFolder, 0, MaxFilePathLen);
    memset(_destPrefix, 0, MaxFilePathLen);

    strcpy(_appName, filename);
    int len = strlen(filename), lend;
    int pos = len - 1, posd, start;
    while (pos > 0 && filename[pos] != PathSplitChar)
        pos--;
    strncpy(_appFolder, filename, pos + 1);
    lend = strlen(destFolder);
    posd = lend - 1;
    if (destFolder[lend - 1] != PathSplitChar)
    {
        //has prefix
        while (posd > 0 && destFolder[posd] != PathSplitChar)
            posd--;
    }
    if (destFolder[0] == '.' && destFolder[1] == PathSplitChar)
    {
        strncpy(_destFolder, filename, pos + 1);
        start = 2;
    strncpy(_destFolder + pos + 1, destFolder + start, posd - start + 1);
    strncpy(_destPrefix, filename, pos + 1);
    strncpy(_destPrefix + pos + 1, destFolder + start, lend - start);
    }
    else
    {
        pos = 8;
        strcpy(_destFolder, _defaltFolder);
        if (destFolder[0] != PathSplitChar)
        {
            start = 0;
        }
        else
        {
            start = 1;
        }
        strncpy(_destPrefix, destFolder, lend);
    }
    // printf("folder%s\n", _destPrefix);
}

void InitLogging(const DP_C_S8 *filename, LogLevel minlevel, const DP_C_S8 *destFolder)
{
    InitPaths(filename, destFolder);
    _destLevel = minlevel;
}

//static string GetLocalDate(void)
//{
//    time_t t = time(0);
//    tm *ld;
//    DP_C_S8 tmp[64] = "";
//    ld = localtime(&t);
//    strftime(tmp, sizeof(tmp), "%Y-%m-%d", ld);
//    return string(tmp);
//}
static string GetCurTime(void)
{
    time_t t = time(0);
    tm *ld;
    DP_C_S8 tmp[64] = "";
    ld = localtime(&t);
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", ld);
    return string(tmp);
}

Logger::Logger(LogLevel level, DP_C_S8 *folder, DP_C_S8 *prefix)
    : level(level)
{
    std::string path;
    path.append(prefix);
//    path.append(GetLocalDate());
    path.append(".log");
    FileHelper::CreateDir(folder);
    logPrefix.append(prefix);
    logPath = path;
    logFile.open(path.c_str(), ios::app | ios::out);
    logFile << "Log file created at:" << GetCurTime() << endl;
}

Logger::~Logger()
{
    logFile.close();
}

#define IMPLEMENT_LOG_FUNC1(cname, fname, lv)                                              \
    void cname::fname(string msg)                                                          \
    {                                                                                      \
        if (level <= lv)                                                                   \
        {                                                                                  \
            WriterMutexLock lock(&mutex);                                                  \
            logFile << "[" << GetCurTime().c_str() << "][" #lv "]" << msg.c_str() << endl; \
            logFile.flush();                                                               \
        }                                                                                  \
    }

#define PRINT_ARGS_TO_BUFFER(fmt, buf) \
    {                                  \
        memset(buf, 0, sizeof(buf));   \
        va_list argp;                  \
        va_start(argp, fmt);           \
        vsprintf(buf, fmt, argp);      \
        va_end(argp);                  \
    }

#define IMPLEMENT_LOG_FUNC2(cname, fname, lv)                                             \
    void cname::fname(const DP_C_S8 *format, ...)                                            \
    {                                                                                     \
        if (level <= lv)                                                                  \
        {                                                                                 \
            WriterMutexLock lock(&mutex);                                                 \
            PRINT_ARGS_TO_BUFFER(format, _gBuffer)                                        \
            std::cout << "[" << GetCurTime().c_str() << "][" #lv "]" << _gBuffer << endl; \
            logFile << "[" << GetCurTime().c_str() << "][" #lv "]" << _gBuffer << endl;   \
            logFile.flush();                                                              \
        }                                                                                 \
    }
#if 1



#endif
#define IMPLEMENT_LOG_FUNC(cname, fname, lv) \
    IMPLEMENT_LOG_FUNC1(cname, fname, lv)    \
    IMPLEMENT_LOG_FUNC2(cname, fname, lv)

IMPLEMENT_LOG_FUNC(Logger, Debug, DEBUG)
IMPLEMENT_LOG_FUNC(Logger, Info, INFO)
IMPLEMENT_LOG_FUNC(Logger, Warn, WARN)
IMPLEMENT_LOG_FUNC(Logger, Error, ERROR)
IMPLEMENT_LOG_FUNC(Logger, Fatal, FATAL)

Logger &Logger::GetInstance()
{
    static Logger _logger(_destLevel, _destFolder, _destPrefix);
    return _logger;
}

void Logger::Log(LogLevel lv, string msg)
{
    if (level <= lv)
    {
        WriterMutexLock lock(&mutex);
        logFile << "[" << GetCurTime().c_str() << "][" << _levelInfos[lv + 1] << "]" << msg.c_str() << endl;
        logFile.flush();
    }
}

void Logger::Log(LogLevel lv, const DP_C_S8 *format, ...)
{
    if (level <= lv)
    {
        WriterMutexLock lock(&mutex);
        PRINT_ARGS_TO_BUFFER(format, _gBuffer)
        logFile << "[" << GetCurTime().c_str() << "][" << _levelInfos[lv + 1] << "]" << _gBuffer << endl;
        logFile.flush();
    }
}

void Logger::Log(const DP_C_S8 *file, int line, LogLevel lv, string msg)
{
    if (level <= lv)
    {
        WriterMutexLock lock(&mutex);
        logFile << "[" << GetCurTime().c_str() << "][" << _levelInfos[lv + 1] << "][" << file << "][" << line << "]" << msg.c_str();
        logFile.flush();
    }
}

Logger *Logger::GetInstancePtr()
{
    return &GetInstance();
}
//
void Logger::Log(const DP_C_S8 *file, int line, LogLevel lv, const DP_C_S8 *format, ...)
{
    if (level <= lv)
    {
        WriterMutexLock lock(&mutex);
        /* call */
        PRINT_ARGS_TO_BUFFER(format, _gBuffer)
        logFile << "[" << GetCurTime().c_str() << "][" << _levelInfos[lv + 1] << "][" << file << "][" << line << "]";
        cout << "[" << _levelInfos[lv + 1] << "][" << file << "][" << line << "]"<<endl;
        logFile.flush();
    }
}
LogMessage::LogMessage(const DP_C_S8 *file, int line, LogLevel lv)
{
    logger = Logger::GetInstancePtr();
    mutex.Lock();
    logger->Log(file, line, lv, "");
}

LogMessage::~LogMessage()
{
    logger->stream() << endl;
    logger->stream().flush();
    mutex.Unlock();
}

} /* namespace FrameWork */
