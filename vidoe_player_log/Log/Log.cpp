#include "Log.h"
#pragma warning(disable: 4996)

CLog* CLog::m_log = NULL;
CLog::CLog(const char* sDir, const char* sPrifix)
{
	m_file = NULL;
	m_bIsOpen = FALSE;
	InitializeCriticalSection(&m_Lock);

	memset(m_absDir, 0x00, sizeof(m_absDir));
	if (!sDir)
	{
		::GetModuleFileNameA(NULL, m_absDir, MAX_PATH);
		char *pos = strrchr(m_absDir, '\\');
		if (pos) *pos = 0;
	}
	else {
		int n = sprintf(m_absDir, sDir);
		m_absDir[n] = 0;
	}

	int n = sprintf(m_attr, ".txt");
	m_attr[n] = 0;
    n = sprintf(m_attrDir, "log");
	m_attrDir[n] = 0;

	if (!sPrifix)
        n = sprintf(m_attrTempName, "log_");
	else
		n = sprintf(m_attrTempName, sPrifix);
	m_attrTempName[n] = 0;
	char timebuff[32] = {};
	Open(CurTime(timebuff));
}

CLog::CLog(const CLog&)
{
}

CLog::~CLog()
{
	DeleteCriticalSection(&m_Lock);
	if (m_file) fclose(m_file);
}

CLog* CLog::Instanse(const char* sDir, const char* sPrifix)
{
	if (!m_log)
		m_log = new CLog(sDir, sPrifix);

	return m_log;
}

bool CLog::AddLog(Log_Level level, const char* sFormat, ...)
{
	if (!m_bIsOpen) return true;
	EnterCriticalSection(&m_Lock);
	struct tm tm1;
	char sDate[32] = {};
	bool bret = true;
	if (!strstr(m_absFile, CurTime(sDate, &tm1)) || !m_file)
	{
		bret = false;
		Open(sDate);
	}
	memset(m_sText, 0x00, sizeof(m_sText));
	int n = 0;
	switch (level)
	{
        case Log_Warning:
        n += sprintf(m_sText, "%-10s", "[Warning]");
        break;
	case Log_Err:
        n += sprintf(m_sText, "%-10s", "[Error]");
		break;
	case Log_Opt:
        n += sprintf(m_sText, "%-10s", "[Option]");
		break;
	default:
        n += sprintf(m_sText, "%-10s", "[Info]");
		break;
	}

	n += sprintf(m_sText + n, " %04d-%02d-%02d %02d:%02d:%02d ", tm1.tm_year + 1900, 1 + tm1.tm_mon, tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
	char* pNext = m_sText + n;
	va_list args;
	va_start(args, sFormat);
	vsprintf_s(pNext, sizeof(m_sText) - n, sFormat, args);
	va_end(args);

	write(m_sText, strlen(m_sText));
	LeaveCriticalSection(&m_Lock);

	return bret;
}

char* CLog::CurTime(char* buff, struct tm* time1, time_t subSecond)
{
	time_t t = time(NULL);
	t -= subSecond;
	struct tm tm1;
#ifdef WIN32  
	localtime_s(&tm1, &t);
#else  
	localtime_r(&t, &tm1);
#endif 
	if (time1) *time1 = tm1;
	int n = sprintf(buff, "%04d-%02d-%02d", tm1.tm_year + 1900, 1 + tm1.tm_mon, tm1.tm_mday);
	buff[n] = 0;
	return buff;
}

void CLog::Open(const char* sDate)
{
	if (m_file)	fclose(m_file);
	memset(m_absFile, 0x00, sizeof(m_absFile));
	int n = sprintf(m_absFile, "%s/%s", m_absDir, m_attrDir);
	CreateDirectoryA(m_absFile, NULL);
	sprintf(m_absFile + n, "/%s%s%s", m_attrTempName, sDate, m_attr);
	m_file = fopen(m_absFile, "a+");
	m_bIsOpen = m_file != NULL;
	if (!m_bIsOpen) return;
	char buff[100] = "===============================START=================================";
	write(buff, strlen(buff));
	AutoDeleteFile();
}

void CLog::write(char* str, int nlength)
{
	if (!m_file) return;
	strcat(str, "\n");
        fwrite(str, nlength + 1, 1, m_file);
        fflush(m_file);
}

void CLog::AutoDeleteFile()
{
	char findFile[MAX_PATH] = {};
	sprintf(findFile, "%s/%s/*%s", m_absDir, m_attrDir, m_attr);
	struct tm tm1;
	char sDate[32] = {};
	CurTime(sDate, &tm1, SAVE_DAY * 24 * 3600);
	strcat(sDate, m_attr);
	auto attrlen = strlen(m_attrTempName);
	WIN32_FIND_DATAA finder;
	HANDLE hFind = FindFirstFileA(findFile, &finder);
	if (hFind == INVALID_HANDLE_VALUE) return;
	do
	{
		if ((finder.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) == FILE_ATTRIBUTE_ARCHIVE)
		{
			auto pDest = strstr(finder.cFileName, m_attrTempName);
			if (pDest)
			{
				pDest += attrlen;
				if (strcmp(sDate, pDest) >= 0)
				{
					char delfile[MAX_PATH] = {};
					sprintf(delfile, "%s/%s/%s", m_absDir, m_attrDir, finder.cFileName);
					DeleteFileA(delfile);
				}
			}
		}
	} while (FindNextFileA(hFind, &finder));

	FindClose(hFind);
}
