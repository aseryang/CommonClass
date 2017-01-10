#pragma once
#include <Windows.h>
class MyShareMem
{
private:
	MyShareMem():isInit(false),
		lpBase(NULL),
		hMapping(INVALID_HANDLE_VALUE),
		bufferSize(0)
	{

	}
public:
	static MyShareMem* getInstance()
	{
		if (nullptr == pInstance)
		{
			pInstance = new MyShareMem;
		}
		return pInstance;
	}
	bool  init(const char* nameString, int size = 0)
	{
		bool isSucceed = true;
		if (size > 0)
		{
			bufferSize = size;
			hMapping = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,size, (LPCTSTR)nameString);
			if (hMapping)
			{
				lpBase = MapViewOfFile(hMapping,FILE_MAP_ALL_ACCESS,0,0,0);
			}
			else
			{
				isSucceed = false;
			}
		}
		else
		{
			hMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS,NULL,(LPCTSTR)nameString);
			if (hMapping)
			{
				lpBase = MapViewOfFile(hMapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
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
	void getShareMemData(char* szContent, int size)
	{
		if (!isInit || nullptr == szContent)
		{
			return;
		}
		memcpy(szContent, (char*)lpBase, size);

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
