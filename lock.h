#ifndef LOCK_H
#define LOCK_H

#include <Windows.h>

/// capsulate a simple windows thread mutex lock
class ThreadMutex
{
public:
	ThreadMutex() {
		InitializeCriticalSection(&csection_);
	}
	~ThreadMutex() {
		DeleteCriticalSection(&csection_);
	}

	void Lock() {
		EnterCriticalSection(&csection_);
	}
	void UnLock() {
		LeaveCriticalSection(&csection_);
	}

private:
	CRITICAL_SECTION csection_;
};

/// a simple mutex lock guard
class MutexGuard
{
public:
	explicit MutexGuard(ThreadMutex& mtx) : mtx_(mtx) {
		mtx_.Lock();
	}
	~MutexGuard(){
		mtx_.UnLock();
	}

private:
	ThreadMutex& mtx_;
};

#endif//LOCK_H
