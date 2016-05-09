#include "stdafx.h"
#include "Timer.h"
#include <iostream>
#include <List>
using namespace std;
class Test: public Thread
{
public:
	Test(){	};
	~Test(){};
	void run()
	{
		while (!isShouldExit())
		{
			cs.Enter();
			if (m_list.empty())
			{
				wait(-1);
			}
			else
			{			
				cout<<"take 1..,which is "<<m_list.back()<<endl;
				m_list.pop_back();
				cs.Exit();
				//do service
			}
			cout<<"==========================="<<endl;
		}	
	}
	void addInt(int input)
	{
		cs.Enter();
		m_list.push_back(input);
		cs.Exit();
		notify();
	}
private:
	list<int> m_list;
	CriticalSectionLock cs;
};
class Test01: public MultiTimer
{
public:
	Test01(){}
	~Test01(){}
	void timerCallBack(int id)
	{
		if (id == 1)
		{
			cout<<"Timer 1 is time out..."<<endl;
		}
		else if (id == 2)
		{
			cout<<"Timer 2 is time out..."<<endl;
		}
	}
protected:
private:
};


int _tmain(int argc, _TCHAR* argv[])
{
	Test t;
	t.startThread();
	while (1)
	{
		int i;
		cin>>i;
		t.addInt(i);
	}
// 	Test01 t01;
// 	t01.startTimer(1, 1000);
// 	t01.startTimer(2, 5000);

	while (1)
	{
	}
	return 0;
}

