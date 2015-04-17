#ifndef LOG_H
#define LOG_H

typedef enum
{
	PrintScrn,		// to print screen
	WritFile,		// to log file
	Debugview,		// to debugview, only support on windows
	SygLog			// to sys log, only support on linux
}OutputType;

typedef enum
{
	NONE,		// if log level specified as this, will not output any msg
	DEBUG,
	INFO,
	WARN,
	ERRO
}LogLevel;

/// create a logger by the filename, and specify how to output log msgs
extern void CreateLOG(const char* filename, OutputType outType);

/// close the logger
extern void CloseLOG();

/// set the minimum log level
extern void SetLogLevel(LogLevel minLevel);

/// log a piece of msg
extern void SyncLOG(LogLevel level, const char* fmt, ...);

/// not implement yet
extern void AsyncLOG(LogLevel level, const char* fmt, ...);

#endif//LOG_H
