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
		//��Ȩ����ȡSE_DEBUG_NAME Ȩ��
		//�������������̵��ڴ�ռ���д�룬�����߳�
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

		//ͨ���ϴ�Զ���̼߳���dll
		//��msg.dll������ϵͳĿ¼��
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
			CREATE_SUSPENDED, //��Ҫע��Ĳ���
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
		//���������ʼ��
		STARTUPINFO st;
		PROCESS_INFORMATION pi;
		PROCESSENTRY32 ps;
		HANDLE hSnapshot;
		ZeroMemory(&st, sizeof(STARTUPINFO));
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		st.cb = sizeof(STARTUPINFO);
		ZeroMemory(&ps, sizeof(PROCESSENTRY32));
		ps.dwSize = sizeof(PROCESSENTRY32);

		//��������
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE)
			return FALSE;
		if (!Process32First(hSnapshot, &ps))
			return FALSE;

		do
		{
			//�ȽϽ�����
			if (lstrcmpi(ps.szExeFile, szProcessName) == 0)
			{
				//�ҵ���
				*lpPID = ps.th32ProcessID;
				CloseHandle(hSnapshot);
				return TRUE;
			}
		}
		while (Process32Next(hSnapshot, &ps));
		//û���ҵ�
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
			//�����Ҫע�����Ľ��̵ľ��
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
			if (NULL == hProcess)
				__leave;
			//����DLL·������Ҫ���ֽ���
			int len = sizeof(WCHAR);
			cch = 2*(1 + lstrlen(lpszLibName));
			//��Զ���߳���Ϊ·��������ռ�
			pszLibFileRemote = (PSTR)VirtualAllocEx(hProcess, NULL, cch, MEM_COMMIT, PAGE_READWRITE);

			if (pszLibFileRemote == NULL)
				__leave;
			//��DLL��·�������Ƶ�Զ�̽��̵ĵ�ַ�ռ�
			if (!WriteProcessMemory(hProcess, (PVOID)pszLibFileRemote, (PVOID)lpszLibName, cch, NULL))
				__leave;
			//���LoadLibraryA��Kernel.dll�е�������ַ
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

			//�ȴ�Զ���߳���ֹ
			WaitForSingleObject(hThread, INFINITE);
			DWORD dwExitCode;
			GetExitCodeThread(hThread,&dwExitCode);
			bResult = TRUE;
		}
		__finally
		{
			//�رվ��
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