#pragma once

#include <memory.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#ifdef unix
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string>
#define MAX_PATH 256
#else
#include <Windows.h>
#endif
#include <mutex>

#define MAX_BUFFER 1024 * 60

#define MAX_ATTR 64
#define SAVE_DAY 7

enum Log_Level
{
	Log_Info,
	Log_Opt,
    Log_Warning,
	Log_Err
};

class CLog
{
public:
	~CLog();
    static CLog* Instanse(const char* sDir = nullptr, const char* sPrifix = nullptr);
    bool AddLog(Log_Level level, const char* function, int line, const char* sFormat, ...);
    bool AddLogB(Log_Level level, const char* sFormat, ...);
private:
    CLog(const char* sDir = nullptr, const char* sPrifix = nullptr);
	CLog(const CLog&);
    char* CurTime(char* buff, struct tm* time1 = nullptr, time_t subSecond = 0);
	void Open(const char* sDate);
	void write(char* str, int nlength);
	void AutoDeleteFile();
private:
//	static CLog* m_log;
	FILE* m_file;
	char m_sText[MAX_BUFFER];
	char m_attrDir[MAX_ATTR];
	char m_attrTempName[MAX_ATTR];
	char m_attr[MAX_ATTR];
    char m_absDir[MAX_PATH];
	char m_absFile[MAX_PATH];
    bool m_bIsOpen;
    std::mutex m_lock;
};

#define InitLogInstance(x, y) CLog::Instanse(x, y)

#define Log(l, format, ...) CLog::Instanse()->AddLog(l, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define LogB(l, format, ...) CLog::Instanse()->AddLogB(l, format, ##__VA_ARGS__)
