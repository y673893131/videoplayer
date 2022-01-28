#include "Log.h"
#include <sys/timeb.h>
#ifdef unix
#include <unistd.h>
#endif

#ifdef WIN32
#pragma warning(disable: 4996)
#endif

#ifdef unix
int GetModuleFileNameA(char* name, int size)
{
    if(readlink("/proc/self/exe", name, size) !=-1 )
    {
        return -1;
    }
    return strlen(name);
}
#endif

CLog* CLog::m_log = nullptr;
CLog::CLog(const char* sDir, const char* sPrifix)
{
    m_file = nullptr;
    m_bIsOpen = false;
    memset(m_absDir, 0x00, sizeof(m_absDir));
    if (!sDir)
    {
#ifdef unix
        ::GetModuleFileNameA(m_absDir, MAX_PATH);
#else
        ::GetModuleFileNameA(nullptr, m_absDir, MAX_PATH);
#endif
        char *pos = strrchr(m_absDir, '\\');
        if (pos) *pos = 0;
    }
    else {
        int n = sprintf(m_absDir, "%s", sDir);
        m_absDir[n] = 0;
    }

    int n = sprintf(m_attr, ".txt");
    m_attr[n] = 0;
    n = sprintf(m_attrDir, "log");
    m_attrDir[n] = 0;

    if (!sPrifix)
        n = sprintf(m_attrTempName, "log_");
    else
        n = sprintf(m_attrTempName, "%s", sPrifix);
    m_attrTempName[n] = 0;
    char timebuff[32] = {};
    Open(CurTime(timebuff));
}

CLog::CLog(const CLog&)
{
}

CLog::~CLog()
{
    if (m_file) fclose(m_file);
}

CLog* CLog::Instanse(const char* sDir, const char* sPrifix)
{
    if (!m_log)
        m_log = new CLog(sDir, sPrifix);
    return m_log;
}

bool CLog::AddLog(Log_Level level, const char *function, int line, const char *sFormat, ...)
{
    if (!m_bIsOpen) return true;
    std::lock_guard<std::mutex> lock(m_lock);

    struct tm tm1;
    char sDate[32] = {};
    bool bret = true;
    int nMicroSec = 0;
    if (!strstr(m_absFile, CurTime(sDate, &tm1, 0, &nMicroSec)) || !m_file)
    {
        bret = false;
        Open(sDate);
    }
    memset(m_sText, 0x00, sizeof(m_sText));
    int n = 0;
    switch (level)
    {
    case Log_Debug:
        n += sprintf(m_sText, "%-10s", "[Debug]");
        break;
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

    n += sprintf(m_sText + n, " %04d-%02d-%02d %02d:%02d:%02d.%03d ", tm1.tm_year + 1900, 1 + tm1.tm_mon, tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec, nMicroSec);
    if(function) n += sprintf(m_sText + n, " [%s:%d] ", function, line);

    char* pNext = m_sText + n;
    va_list args;
    va_start(args, sFormat);
    vsprintf(pNext, sFormat, args);
    va_end(args);

    write(m_sText, strlen(m_sText), level);
    return bret;
}

char* CLog::CurTime(char* buff, struct tm* time1, time_t subSecond, int* microSec)
{
    struct timeb tmb;
    ftime(&tmb);

    tmb.time -= subSecond;
    struct tm tm1;
#ifdef WIN32
    localtime_s(&tm1, &tmb.time);
#else
    localtime_r(&tmb.time, &tm1);
#endif
    if (time1) *time1 = tm1;
    if(microSec) *microSec = tmb.millitm;
    int n = sprintf(buff, "%04d-%02d-%02d", tm1.tm_year + 1900, 1 + tm1.tm_mon, tm1.tm_mday);
    buff[n] = 0;
    return buff;
}

void CLog::Open(const char* sDate)
{
    if (m_file)	fclose(m_file);
    memset(m_absFile, 0x00, sizeof(m_absFile));
    int n = sprintf(m_absFile, "%s/%s", m_absDir, m_attrDir);
#ifdef WIN32
    CreateDirectoryA(m_absFile, nullptr);
#else
    mkdir(m_absFile, S_IRWXU);
#endif
    sprintf(m_absFile + n, "/%s%s%s", m_attrTempName, sDate, m_attr);
    m_file = fopen(m_absFile, "a+");
    m_bIsOpen = m_file != nullptr;
    if (!m_bIsOpen) return;

    char start[] = "===============================START=================================";
    write(start, static_cast<int>(strlen(start)), Log_Info);
    AutoDeleteFile();
}

void CLog::write(char* str, int nlength, int nLevel)
{
    if (!m_file) return;
        strcat(str, "\n");
#ifdef WIN32
    OutputDebugStringA(str);
#else
    fwrite(str, strlen(str), 1, stdout);
    fflush(stdout);
#endif
    if(nLevel == Log_Debug)
    {
        return;
    }

    fwrite(str, static_cast<unsigned int>(nlength + 1), 1, m_file);
    fflush(m_file);
}

#ifdef unix
bool is_dir(const char* path)
{
    struct stat S_stat;
    //get file state
    if (lstat(path, &S_stat) < 0)
        return false;

    if (S_ISDIR(S_stat.st_mode))
        return true;
    else
        return false;
}

void CLog::AutoDeleteFile()
{
    char srcPath[MAX_PATH] = {};
    sprintf(srcPath, "%s/%s", m_absDir, m_attrDir);
    auto pDir = opendir(srcPath);
    if (pDir == nullptr)
        return;

    struct tm tm1;
    char sDate[32] = {};
    CurTime(sDate, &tm1, SAVE_DAY * 24 * 3600);
    strcat(sDate, m_attr);

    struct dirent* pDirent = nullptr;
    while ((pDirent = readdir(pDir)) != nullptr)
    {
        if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0 || pDirent->d_type != DT_REG)
            continue;
        char szTmpPath[1024] = {0};
        sprintf(szTmpPath, "%s/%s", srcPath, pDirent->d_name);
        if(is_dir(szTmpPath))
            continue;

        std::string sTmp(pDirent->d_name);

        int pos = sTmp.find_last_of(m_attrTempName);
        if(pos > 0)
        {
            char* pDest = &pDirent->d_name[pos + 1];
            if (strcmp(sDate, pDest) >= 0)
                remove(szTmpPath);
        }
    }

    closedir(pDir);
}
#else
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

#endif
