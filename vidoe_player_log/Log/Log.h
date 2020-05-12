#pragma once

#include <stdio.h>
#include <windows.h>
#include <time.h>

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
	static CLog* Instanse(const char* sDir = NULL, const char* sPrifix = NULL);
	bool AddLog(Log_Level level, const char* sFormat, ...);
private:
	CLog(const char* sDir = NULL, const char* sPrifix = NULL);
	CLog(const CLog&);
	char* CurTime(char* buff, struct tm* time1 = NULL, time_t subSecond = 0);
	void Open(const char* sDate);
	void write(char* str, int nlength);
	void AutoDeleteFile();
private:
	static CLog* m_log;
	FILE* m_file;
	char m_sText[MAX_BUFFER];
	char m_attrDir[MAX_ATTR];
	char m_attrTempName[MAX_ATTR];
	char m_attr[MAX_ATTR];
	char m_absDir[MAX_PATH];
	char m_absFile[MAX_PATH];
	BOOL m_bIsOpen;
	CRITICAL_SECTION m_Lock;
};

#define InitLogInstance(x, y) CLog::Instanse(x, y)
#define Log(l,x,...) CLog::Instanse()->AddLog(l,x,__VA_ARGS__)
