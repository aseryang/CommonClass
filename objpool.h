#pragma once
#include <queue>
#include <vector>
#ifdef MY_LIB_DEBUG
#include <iostream>
#endif
using namespace std;
template<class T> 
class ObjPool
{
public:
	ObjPool(int defaultSize)
	{
		m_increaseSize = defaultSize;
		createNewObjs();
	}
	~ObjPool()
	{
		for (int i = 0; i < m_allArray.size(); ++i)
		{
			delete [] m_allArray[i];
		}
	}
	void createNewObjs()
	{
#ifdef MY_LIB_DEBUG
		cout<<"FUNC: createNewObjs"<<endl;
#endif
		T * pArray = new T[m_increaseSize];
		for (int i = 0; i < m_increaseSize; ++i)
		{
			m_queAllObj.push(&pArray[i]);
		}
		m_allArray.push_back(pArray);
	}
	T* getOneObj()
	{
#ifdef MY_LIB_DEBUG
		cout<<"FUNC: getOneObj"<<endl;
#endif
		if (m_queAllObj.size() <= 0)
		{
			createNewObjs();
		}
		T * temp = m_queAllObj.back();
		m_queAllObj.pop();
		return temp;
	}
	void recycle(T * singleObj)
	{
#ifdef MY_LIB_DEBUG
		cout<<"FUNC: recycle"<<endl;
#endif
		m_queAllObj.push(singleObj);
	}
private:
	queue<T*>	m_queAllObj;
	vector<T*>	m_allArray;
	int			m_increaseSize;
};
