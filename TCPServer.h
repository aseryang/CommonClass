#pragma once
#include <WinSock2.h>
#include <string>
#pragma comment(lib,"ws2_32.lib")
class TCPServer
{
public:
	TCPServer()
	{
		WSADATA wsa;
		WSAStartup(MAKEWORD(1,1),&wsa);
		m_socket=socket(AF_INET,SOCK_STREAM,0);
	}
	~TCPServer()
	{
		closesocket(m_socket);
	}
	int tcpBind(int port)
	{
		SOCKADDR_IN Sersock;//用于服务器的监听SOCKET
		ZeroMemory(&Sersock,sizeof(Sersock)); 
		Sersock.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//所有可用IP
		Sersock.sin_family = AF_INET; 
		Sersock.sin_port = htons(port);//端口号 
		int nbind = bind(m_socket,(SOCKADDR *)&Sersock,sizeof(SOCKADDR)); 
		if (SOCKET_ERROR==nbind) 
		{ 
			closesocket(m_socket);
			return -1; 
		}
		return 0;
	}
	int tcpAccept()
	{
		SOCKADDR_IN addrClient;
		int len=sizeof(SOCKADDR);
		SOCKET sockSer;
		sockSer=accept(m_socket,(SOCKADDR*)&addrClient,&len);
		return 0;
	}
	int tcpListen()
	{
		listen(m_socket,5);
		return 0;
	}
protected:
private:
	SOCKET m_socket;
};