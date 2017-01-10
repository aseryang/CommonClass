#pragma once
#include <Windows.h>
class MyShareMem
{
private:
	MyShareMem():isInit(false)
	{

	}
public:
	static MyShareMem* getInstance()
	{
		if (nullptr == pInstance)
		{
			pInstance = new MyShareMem;
		}
	}
	bool  init(const char* nameString, int size = 0)
	{
		bool isSucceed = true;
		if (size > 0)
		{
			bufferSize = size;
			HANDLE hMapping = CreateFileMapping(NULL,NULL,PAGE_READWRITE,0,size, (LPCTSTR)nameString);
			if (hMapping)
			{
				LPVOID lpBase = MapViewOfFile(hMapping,FILE_MAP_WRITE|FILE_MAP_READ,0,0,0);
			}
			else
			{
				isSucceed = false;
			}
		}
		else
		{
			HANDLE hMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS,NULL,(LPCTSTR)nameString);
			if (hMapping)
			{
				LPVOID lpBase = MapViewOfFile(hMapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
			}
			else
			{
				isSucceed = false;
			}
		}
		if (isSucceed)
		{
			isInit = true;
		}
		return isSucceed;
	}
	void writeShareMemData(char* szContent, int size)
	{
		if (!isInit)
		{
			return;
		}
		memcpy((char*)lpBase, szContent, size);
		bufferSize = size;
	}
	void getShareMemData(char* szContent)
	{
		if (!isInit || nullptr == szContent)
		{
			return;
		}
		memcpy(szContent, (char*)lpBase, bufferSize);

	}
	~MyShareMem()
	{
		UnmapViewOfFile(lpBase);
		CloseHandle(hMapping);
	}
protected:
private:
	static MyShareMem* pInstance;
	HANDLE hMapping;
	LPVOID lpBase;
	int    bufferSize;
	bool   isInit;
	
};