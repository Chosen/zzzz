#include "Utilities.h"


#include <Windows.h>
#include <stdio.h>

#define DEBUGVIEW_LOG 1
void Log2DebugView(const char* format, ...)
{
#if DEBUGVIEW_LOG	
	char buf[4096]	= {0};
	SYSTEMTIME stm;
	GetLocalTime(&stm);
	_snprintf(buf, 32, "%.2d:%.2d:%.2d.%.3d ", stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds);

	char* p = buf + strlen(buf);
	va_list args;
	va_start(args, format);
	vsprintf(p, format, args);
	va_end(args);
	OutputDebugString(buf);
#endif//DEBUGVIEW_LOG
}


/// a high time counter
double QueryTickCount()
{
	LARGE_INTEGER frequency;
	LARGE_INTEGER counter;
	double dFreq,dCounter;
	QueryPerformanceFrequency(&frequency);       
	QueryPerformanceCounter(&counter);   
	dFreq   =(double)(frequency.QuadPart);
	dCounter=(double)(counter.QuadPart);
	return (dCounter/dFreq*1000.0*1000.0);
}
