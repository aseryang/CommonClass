#pragma once
#include <WinSock2.h>
#include <string>
#pragma comment(lib,"ws2_32.lib")
class TCPConnection
{
public:
	TCPConnection()
	{
		WSADATA wsa;
		WSAStartup(MAKEWORD(1,1),&wsa);
		m_socket=socket(AF_INET,SOCK_STREAM,0);
	}
	~TCPConnection()
	{

	}
	int tcpBind()
	{
		return 0;
	}
	int tcpConnect(std::string strIp, int port)
	{
		SOCKADDR_IN SocketSendIn;
		SocketSendIn.sin_family = AF_INET;
		SocketSendIn.sin_addr.S_un.S_addr = inet_addr(strIp.c_str());
		SocketSendIn.sin_port = htons(port);
		int nRet = connect(m_socket,(SOCKADDR*)&SocketSendIn,sizeof(SOCKADDR));
		if (nRet==SOCKET_ERROR)
		{
			return -1;
		}
		return 0;
	}
protected:
private:
	SOCKET m_socket;
};