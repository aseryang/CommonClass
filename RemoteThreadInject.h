#pragma once
#include <windows.h>
#include <TlHelp32.h>

class RemoteThreadInject
{
private:
	RemoteThreadInject(){}
public:
	static RemoteThreadInject* getInstance()
	{
		if (nullptr == pInstance)
		{
			pInstance = new RemoteThreadInject;
		}
		return pInstance;
	}
	bool init()
	{
		//提权，获取SE_DEBUG_NAME 权限
		//可以在其他进程的内存空间中写入，创建线程
		if(0 == EnableDebugPrivilege(true))
			return false;
		return true;
	}
	bool doInject(LPWSTR targetExePath, LPWSTR srcDllPath, bool isCreateProcess = false)
	{
		DWORD dwPID;
		HANDLE hMainThread = NULL;
		//TEXT("Target.exe")
		if (!isCreateProcess)
		{
			if (!GetProcessIdByName(targetExePath, &dwPID))
				return false;
		}
		else
		{
			if (!GetProcessIdByCreateIt(targetExePath, &dwPID, hMainThread))
				return false;
		}

		//通过上传远程线程加载dll
		//将msg.dll放置在系统目录下
		//TEXT("E:\\VS2010_PROJECT\\Msg\\Release\\msg.dll")
		if (!LoadRemoteDll(dwPID, srcDllPath, hMainThread, isCreateProcess))
			return false;
		
		return true;
	}
private:
	bool EnableDebugPrivilege(bool fEnable)
	{ 
		bool   fOK	=   FALSE; 
		HANDLE	hToken	=   NULL; 
		if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken)){ 
			TOKEN_PRIVILEGES	tp; 
			tp.PrivilegeCount	=1; 
			LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tp.Privileges[0].Luid); 
			tp.Privileges[0].Attributes	=   fEnable   ?   SE_PRIVILEGE_ENABLED   :   0; 
			AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(tp),NULL,NULL); 
			int ret = GetLastError();
			fOK	=   (GetLastError()==ERROR_SUCCESS); 
			CloseHandle(hToken); 
		} 
		return fOK; 
	} 
	BOOL GetProcessIdByCreateIt(LPWSTR szProcessName, LPDWORD lpPID, HANDLE& hMainThread)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory( &si, sizeof(si) );
		ZeroMemory( &pi, sizeof(pi) ); 
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOW;

		//TCHAR cmdline[MAXBYTE] =_T("calc.exe");
		BOOL bRet = ::CreateProcess(
			szProcessName,
			NULL,			
			NULL,
			NULL,
			FALSE,
			CREATE_SUSPENDED, //需要注意的参数
			NULL,
			NULL,
			&si,
			&pi);

		*lpPID = pi.dwProcessId;
		hMainThread = pi.hThread;

		return TRUE;
	}

	BOOL GetProcessIdByName(LPWSTR szProcessName, LPDWORD lpPID)
	{
		//变量及其初始化
		STARTUPINFO st;
		PROCESS_INFORMATION pi;
		PROCESSENTRY32 ps;
		HANDLE hSnapshot;
		ZeroMemory(&st, sizeof(STARTUPINFO));
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		st.cb = sizeof(STARTUPINFO);
		ZeroMemory(&ps, sizeof(PROCESSENTRY32));
		ps.dwSize = sizeof(PROCESSENTRY32);

		//遍历进程
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE)
			return FALSE;
		if (!Process32First(hSnapshot, &ps))
			return FALSE;

		do
		{
			//比较进程名
			if (lstrcmpi(ps.szExeFile, szProcessName) == 0)
			{
				//找到了
				*lpPID = ps.th32ProcessID;
				CloseHandle(hSnapshot);
				return TRUE;
			}
		}
		while (Process32Next(hSnapshot, &ps));
		//没有找到
		CloseHandle(hSnapshot);
		return FALSE;
	}

	BOOL LoadRemoteDll(DWORD dwProcessId, LPTSTR lpszLibName, HANDLE hMainThreadHandle, bool isCreatedProcess = false)
	{
		BOOL   bResult            = FALSE;
		HANDLE hProcess            = NULL;
		HANDLE hThread            = NULL;
		PSTR   pszLibFileRemote = NULL;
		DWORD cch;
		PTHREAD_START_ROUTINE pfnThreadRrn;
		__try
		{
			//获得想要注入代码的进程的句柄
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
			if (NULL == hProcess)
				__leave;
			//计算DLL路径名需要的字节数
			int len = sizeof(WCHAR);
			cch = 2*(1 + lstrlen(lpszLibName));
			//在远程线程中为路径名分配空间
			pszLibFileRemote = (PSTR)VirtualAllocEx(hProcess, NULL, cch, MEM_COMMIT, PAGE_READWRITE);

			if (pszLibFileRemote == NULL)
				__leave;
			//将DLL的路径名复制到远程进程的地址空间
			if (!WriteProcessMemory(hProcess, (PVOID)pszLibFileRemote, (PVOID)lpszLibName, cch, NULL))
				__leave;
			//获得LoadLibraryA在Kernel.dll中得真正地址
			pfnThreadRrn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
			if (pfnThreadRrn == NULL)
				__leave;

			if (!isCreatedProcess)
			{
				hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRrn, (PVOID)pszLibFileRemote, 0, NULL);
			}
			else
			{
				hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRrn, (PVOID)pszLibFileRemote, CREATE_SUSPENDED, NULL);
			}
			if (hThread == NULL)
			{
				int ret = GetLastError();
				__leave;
			}
			
			if (isCreatedProcess && hThread && hMainThreadHandle)
			{
				ResumeThread(hMainThreadHandle);
				ResumeThread(hThread);		
			}

			//等待远程线程终止
			WaitForSingleObject(hThread, INFINITE);
			DWORD dwExitCode;
			GetExitCodeThread(hThread,&dwExitCode);
			bResult = TRUE;
		}
		__finally
		{
			//关闭句柄
			if (pszLibFileRemote != NULL)
				VirtualFreeEx(hProcess, (PVOID)pszLibFileRemote, 0, MEM_RELEASE);
			if (hThread != NULL)
				CloseHandle(hThread);
			if (hProcess != NULL)
				CloseHandle(hProcess);
		}
		return bResult;
	}
private:
	static RemoteThreadInject* pInstance;
};