#ifndef _WIN_THREAD_H
#define _WIN_THREAD_H
#include <windows.h>

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

private:
	HANDLE	m_hThreadHandle;
	bool	m_bShouldExit;
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
