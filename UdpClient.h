#pragma once
#include <WinSock2.h>
#include <string>
#pragma comment(lib,"ws2_32.lib")

class UdpClient
{
public:
	UdpClient():isInit(false)
	{

	}
	~UdpClient()
	{
		closesocket(m_sock);
	}
	int init()
	{
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2,1),&wsaData)) //调用Windows Sockets DLL
		{
			//printf("Winsock无法初始化!\n");
			WSACleanup();
			return -1;
		}

		m_sock=socket(AF_INET,SOCK_DGRAM,0);
		isInit = true;
		return 0;
	}
	int udpBind(int inPort)
	{
		struct sockaddr_in local;
		struct sockaddr_in from;
		int fromlen =sizeof(from);
		local.sin_family=AF_INET;
		local.sin_port=htons(inPort); ///监听端口
		local.sin_addr.s_addr=INADDR_ANY; ///本机
		bind(m_sock,(struct sockaddr*)&local,sizeof(local));
	}
	SOCKET getSock()
	{
		if (isInit)
		{
			return m_sock;
		}
		return 0;
	}
protected:
private:
	bool   isInit;
	SOCKET m_sock;
};