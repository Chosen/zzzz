#pragma warning(disable:4996)
#include "logger.h"
#include "lock.h"
#include <stdio.h>
#include <stdarg.h>
#include <process.h>

static int	 logLevel		= INFO;		// the configured log level
static FILE* logfp			= NULL;			// the log file pointer
static int	 withTimeStamp	= 1;			// default is with timestamp when output log msg
static int	 running		= 1;

static char* levels[8] = {
	"NONE",
	"DEBG",
	"INFO",
	"WARN",
	"ERROR"
};

typedef struct logmsg
{
	struct logmsg*	next;
	int				level;
	char			msg[2040];
}logmsg;

struct logmsgs
{
	logmsg*		head;
	logmsg*		tail;
	ThreadMutex	lock;
	HANDLE		logEvent;
};
static struct logmsgs loglst;

typedef void (*OutputFunc)(char* buf);
static OutputFunc pfLogFunc = NULL;		// the global log func

static void Output2File(char* buf)
{
	if (logfp)
	{
		fputs(buf, logfp);
		fflush(logfp);
	}
}
static void Output2DbgView(char* buf)
{
#ifdef WIN32
	OutputDebugString(buf);
#endif//WIN32
}
static void Output2Scrn(char* buf)
{
	printf(buf);
}
static void Output2Syslog(char* buf)
{
#ifdef __linux__
	syslog(1, buf);
#endif
}

static void CreateLogFile(const char* filename)
{
	char buf0[256] = "";
	char buf1[256] = "";
	const char* ps = strstr(filename, ".") ;
	if (ps == NULL)
	{
		strcpy(buf0, filename);
	}
	else
	{
		int namelen = ps - filename;
		strncpy(buf0, filename, namelen);
		strcpy(buf1, ".");
		strcat(buf1, filename + namelen + 1);
	}

	char buf[512] = "";
	SYSTEMTIME stm;
	GetLocalTime(&stm);
	int len = _snprintf(buf, 511, "%s_%04d%02d%02d%s", buf0, stm.wYear, stm.wMonth, stm.wDay, buf1);

	if (logfp != NULL)
	{
		fclose(logfp);
		logfp = NULL;
	}

	logfp = fopen(buf, "a+");
}

static logmsg* GetLogMsgItem(logmsgs& lst)
{
	logmsg* amsg = NULL;
	MutexGuard lk(loglst.lock);
	if (loglst.head != NULL)
	{
		amsg = loglst.head;
		if (loglst.head == loglst.tail)
		{
			loglst.head = NULL;
			loglst.tail = NULL;
		}
		else
		{
			loglst.head = loglst.head->next;
		}
	}
	return amsg;
}

static unsigned int __stdcall LogThreadEntry(void* arg)
{
	while (running)
	{
		int res = WaitForSingleObject(loglst.logEvent, 1000);
		if (res == WAIT_TIMEOUT)
		{
			continue;
		}
		if (res == WAIT_FAILED)
		{
			SyncLOG(ERRO, "Logger wait for event failed!");
			break;
		}

		// write all the msg to file
		while (true)
		{
			logmsg* amsg = GetLogMsgItem(loglst);
			if (amsg == NULL)
				break;
			
			// output log msg
			if (pfLogFunc)
				pfLogFunc(amsg->msg);
			delete amsg;
		}
		
	}//while running

	return 0;
}
//////////////////////////////////////////////////////////////////////////
void CreateLOG(const char* filename, OutputType outType)
{
	if (filename == NULL)
		return;

	if (outType == WritFile)
	{
		CreateLogFile(filename);
		pfLogFunc = Output2File;
	}
	else if (outType == PrintScrn)
	{
		pfLogFunc = Output2Scrn;
	}
	else if (outType == Debugview)
	{
		pfLogFunc = Output2DbgView;
	}
	else if (outType == SygLog)
	{
		pfLogFunc = Output2Syslog;
	}

	// init the logger
	running		= 1;
	loglst.head = NULL;
	loglst.tail = NULL;
	loglst.logEvent = CreateEvent(NULL, false, false, NULL);

	_beginthreadex(NULL, 0, LogThreadEntry, NULL, 0, NULL);
}

void CloseLOG()
{
	running = 0;
	logLevel= 9;
	if (logfp)
	{
		pfLogFunc = NULL;
		fclose(logfp);
		logfp = NULL;
	}
}

void SetLogLevel(LogLevel minLevel)
{
	logLevel = minLevel;
}

void SyncLOG(LogLevel level, const char* fmt, ...)
{
	if (level < logLevel)
		return;

	char buf[8192]	= {0};
	int  len		= 0;

	if (withTimeStamp)
	{
		SYSTEMTIME stm;
		GetLocalTime(&stm);
		len = _snprintf(buf, 32, "%.2d:%.2d:%.2d.%.3d [%s] ", stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds, levels[level]);
	}

	// format the log msg
	char* p = buf + len;
	va_list args;
	va_start(args, fmt);
	vsprintf(p, fmt, args);
	va_end(args);
	strcat(buf, "\r\n");

	// call output log func
	if (pfLogFunc)
		pfLogFunc(buf);
}

/// asynchronise logger func
void AsyncLOG(LogLevel level, const char* fmt, ...)
{
	if (level < logLevel)
		return;

	logmsg* amsg = new logmsg;
	if (amsg == NULL)
		return;

	int len	= 0;
	if (withTimeStamp)
	{
		SYSTEMTIME stm;
		GetLocalTime(&stm);
		len = _snprintf(amsg->msg, 32, "%.2d:%.2d:%.2d.%.3d [%s] ", stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds, levels[level]);
	}

	// format the log msg
	char* p = amsg->msg + len;
	va_list args;
	va_start(args, fmt);
	vsprintf(p, fmt, args);
	va_end(args);
	strcat(amsg->msg, "\r\n");
	amsg->level = level;

	MutexGuard lk(loglst.lock);
	if (loglst.head == NULL)
	{
		loglst.head = amsg;
		loglst.tail = amsg;
	}
	else
	{
		if (loglst.tail != NULL)
			loglst.tail->next = amsg;
		loglst.tail = amsg;
	}
	SetEvent(loglst.logEvent);
}


