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
	virtual void timerCallBack(int id) = 0;
private:
	void addTimer(Timer* p);
	void removeTimer(Timer* p);
	void run();
	void updateElapsedTime(int et);
	void callAllTimers();
	
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
				
				if (t->pre)
				{
					t->pre->next = p;
				}
				t->pre = p;
				if (NULL == t->next)
				{
					firstTimer = p;
				}
				break;
			}
		}
	}
}
void MultiTimer::removeTimer(Timer* p)
{
	if (NULL == p)
	{
		return;
	}

	if (p->pre != NULL)
	{
		p->pre->next = p->next;
		if (p->next)
		{
			p->next->pre = p->pre;
		}
	}
	else
	{
		firstTimer = p->next;
		if (p->next)
		{
			p->next->pre = NULL;
		}
	}

	p->pre = NULL;
	p->next = NULL;
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
			updateElapsedTime(elapsedTime);
			lastTime = now;
			if (firstTimer->m_countDownTime <= 0)
			{
				callAllTimers();
			}
			else
			{
				Sleep(50);
			}
		}	
	}
}
void MultiTimer::updateElapsedTime(int et)
{
	Timer* t = firstTimer;
	while (t)
	{
		t->m_countDownTime -= et;
		t = t->next;
	}
}
void MultiTimer::callAllTimers()
{
	while (firstTimer != NULL && firstTimer->m_countDownTime <= 0)
	{
		Timer* temp = firstTimer;
		timerCallBack(temp->getId());
		temp->m_countDownTime = temp->m_periodTime;
		removeTimer(temp);
		addTimer(temp);	
	}
}
#endif
