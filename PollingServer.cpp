#include <iostream>
#include <WinSock2.h>

using namespace std;

#pragma comment(lib, "ws2_32")

int main()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ListenSockAddr = { 0 , };
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;// inet_addr("127.0.0.1");
	ListenSockAddr.sin_port = htons(22222);

	bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));

	listen(ListenSocket, 5);

	fd_set ReadSocketList;
	fd_set CopyReadSocketList;

	FD_ZERO(&ReadSocketList);
	FD_SET(ListenSocket, &ReadSocketList);

	TIMEVAL TimeOut;
	TimeOut.tv_sec = 0;
	TimeOut.tv_usec = 10;

	while (true)
	{
		CopyReadSocketList = ReadSocketList;
		int ChangeSocketCount = select(0, &CopyReadSocketList, nullptr, nullptr, &TimeOut);

		if (ChangeSocketCount == 0)
		{
			continue;
		}
		else
		{
			for (int i = 0; i < (int)ReadSocketList.fd_count; ++i)
			{
				if (FD_ISSET(ReadSocketList.fd_array[i], &CopyReadSocketList))
				{
					if (ReadSocketList.fd_array[i] == ListenSocket)
					{
						SOCKADDR_IN ClientSockAddr = { 0 , };
						int ClientSockAddrSize = sizeof(ClientSockAddr);
						SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSockAddr, &ClientSockAddrSize);
						FD_SET(ClientSocket, &ReadSocketList);
					}
					else
					{
						char Buffer[1024] = { 0, };
						int RecvLength = recv(ReadSocketList.fd_array[i], Buffer, sizeof(Buffer), 0);
						if (RecvLength <= 0)
						{
							closesocket(ReadSocketList.fd_array[i]);
							FD_CLR(ReadSocketList.fd_array[i], &ReadSocketList);
						}
						else
						{
							// Polling 방식 적용
							for (int j = 0; j < (int)ReadSocketList.fd_count; ++j)
							{
								int SendLength = send(ReadSocketList.fd_array[j], Buffer, RecvLength, 0);
							}
						}
					}
				}
			}
		}
	}

	closesocket(ListenSocket);

	WSACleanup();

	return 0;
}