#ifndef _WIN_THREAD_H
#define _WIN_THREAD_H
#include <windows.h>
class CriticalSectionLock
{
public:
	CriticalSectionLock()
	{
		InitializeCriticalSection(&m_cs);		
	}
	virtual ~CriticalSectionLock()
	{
		DeleteCriticalSection(&m_cs);
	}
	void Enter()
	{
		EnterCriticalSection(&m_cs);
	}
	void Exit()
	{
		LeaveCriticalSection(&m_cs);
	}
private:
	CRITICAL_SECTION m_cs;
};
class WaitEvent
{
public:
	WaitEvent()
	{
		handle = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	virtual ~WaitEvent()
	{
		CloseHandle(handle);
		handle = NULL;
	}
	void wait(int ms)
	{
		WaitForSingleObject(handle, ms);
	}
	void notify()
	{
		SetEvent(handle);
	}
private:
	HANDLE handle;
};
class Thread
{
public:
	Thread():m_hThreadHandle(NULL), m_bShouldExit(false){};
	virtual ~Thread(){
		if (m_hThreadHandle)
		{
			CloseHandle(m_hThreadHandle);
			m_hThreadHandle = NULL;
		}
		m_bShouldExit = false;
	};
	virtual void run() = 0;
	void startThread();
	void stopThread();
	bool isShouldExit();
	static DWORD WINAPI ThreadEnter(LPVOID lpParameter);
	void wait(int time)
	{
		m_waitEvent.wait(time);
	}
	void notify()
	{
		m_waitEvent.notify();
	}

private:
	HANDLE		m_hThreadHandle;
	bool		m_bShouldExit;
	WaitEvent	m_waitEvent;
};

bool Thread::isShouldExit()
{
	return m_bShouldExit;
}

DWORD Thread::ThreadEnter(LPVOID lpParameter)
{
	Thread * pObj = (Thread*)lpParameter;
	if (NULL == pObj)
	{
		return -1;
	}
	while (!pObj->isShouldExit())
	{
		pObj->run();
	}
	return 0;
}

void Thread::startThread()
{
	m_hThreadHandle = CreateThread(NULL, 0, ThreadEnter, this, 0, NULL);
}

void Thread::stopThread()
{
	m_bShouldExit = true;
}
#endif
