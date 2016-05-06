#ifndef WIN_TIMER_H
#define WIN_TIMER_H
#pragma comment( lib,"winmm.lib" )
#include "Thread.h"
#include <map>
class Timer
{
public:
	Timer(int id, int interval):m_id(id), m_periodTime(interval), m_countDownTime(interval), next(NULL), pre(NULL){}
	virtual ~Timer(){}
	int getId()
	{
		return m_id;
	}
private:
	int		m_id;
	int		m_periodTime;
	int		m_countDownTime;
	Timer*	next;
	Timer*	pre;
	friend class MultiTimer;
};
class MultiTimer: public Thread
{
public:
	MultiTimer():isThreadStart(false), firstTimer(NULL){}
	virtual ~MultiTimer(){}
	void startTimer(int id, int interval);
	void stopTimer(int id);
private:
	void addTimer(Timer* p);
	void removeTimer(Timer* p);
	void run();
	virtual void timerCallBack(int id) = 0;
protected:
private:
	std::map<int, Timer*>	timerMap;
	Timer*					firstTimer;
	bool					isThreadStart;
};
void MultiTimer::startTimer(int id, int interval)
{
	if (!isThreadStart)
	{
		startThread();
		isThreadStart = true;
	}
	std::map<int, Timer*>::iterator it = timerMap.find(id);
	if (it == timerMap.end())
	{
		Timer * p = new Timer(id, interval);
		timerMap.insert(std::make_pair(id, p));
		addTimer(p);
	}
}
void MultiTimer::stopTimer(int id)
{
	std::map<int, Timer*>::iterator it = timerMap.find(id);
	if (it != timerMap.end())
	{
		timerMap.erase(it);
		removeTimer(it->second);
		delete it->second;
	}
}
void MultiTimer::addTimer(Timer* p)
{
	if (NULL == p)
	{
		return;
	}
	if (NULL == firstTimer)
	{
		firstTimer = p;
	}
	else
	{
		Timer* t = firstTimer;
		while (t)
		{
			if (t->m_countDownTime <= p->m_countDownTime)
			{
				if (t->next != NULL)
				{
					t = t->next;
				}
				else
				{
					t->next = p;
					p->pre = t;
					p->next = NULL;
					break;
				}			
			}
			else
			{
				p->next = t;
				p->pre = t->pre;
				t->pre->next = p;
				t->pre = p;
				break;
			}
		}
	}
}
void MultiTimer::removeTimer(Timer* p)
{
	Timer* t = firstTimer;
	while (t)
	{
		if (t != p)
		{
			t = t->next;
		}
		else
		{
			if (t == firstTimer)
			{
				firstTimer = firstTimer->next;
				firstTimer->pre = NULL;
			}
			else
			{
				t->pre->next = t->next;
				if (t->next)
				{
					t->next->pre = t->pre;
				}
				p->pre = NULL;
				p->next = NULL;
			}
			break;
		}
	}
}
void MultiTimer::run()
{
	int lastTime = timeGetTime();
	while (!isShouldExit())
	{
		int now = timeGetTime();
		int elapsedTime = now - lastTime;
		
		if (elapsedTime > 0)
		{
			Timer* t = firstTimer;
			while (t)
			{
				t->m_countDownTime -= elapsedTime;
				if (t->m_countDownTime <= 0)
				{
					timerCallBack(t->getId());
					lastTime = now;
				}
				t = t->next;
			}
//			t = firstTimer;
// 			while (t)
// 			{
// 				Timer* temp = t->next;
// 				if (t->m_countDownTime <= 0)
// 				{
// 					t->m_countDownTime = t->m_periodTime;
// 					removeTimer(t);
// 					addTimer(t);
// 				}
// 				t = temp;
// 			}
// 			if (firstTimer->m_countDownTime > 0)
// 			{
// 				Sleep(firstTimer->m_countDownTime);
// 			}
		}	
	}
}
#endif
