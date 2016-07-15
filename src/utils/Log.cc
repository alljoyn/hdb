#include "utils/Log.h"

#include "qcc/Mutex.h"

#include <time.h>
#include <sys/time.h>

extern FILE* gLogFile;
extern qcc::Mutex* gLogFileMtx;

static struct tm *t;
static timeval tv;
static char timeStr[24] = {0, };

char* GetCurrentTimeStr()
{
	gettimeofday(&tv, NULL);
	t = localtime(&tv.tv_sec);
	sprintf(timeStr, "%04d/%02d/%02d/%02d:%02d:%02d:%03ld", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec/1000);
	return timeStr;
}

void WriteLog(const char* str,...)
{
	if (gLogFile == NULL)
		return;

	gLogFileMtx->Lock();

	va_list argList;
	va_start(argList, str);
	vfprintf(gLogFile, str, argList);
	va_end(argList);

	fflush(gLogFile);
	
	gLogFileMtx->Unlock();

	return;
}
