#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>

using namespace std;

#pragma comment(lib,"ws2_32")

int main()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));

	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(10880);

	bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));

	listen(ListenSocket, 5);

	fd_set ReadSocketList;		// 서버에 요청을 보낸 소켓 리스트를 넣는 구조체
	FD_ZERO(&ReadSocketList);	// 초기화 작업

	// 이 매크로 의미는 찾아보기...
	FD_SET(ListenSocket, &ReadSocketList);

	struct timeval TimeOut;		// 얼마나 기다릴지에 대한 시간을 정의한 구조체
	TimeOut.tv_sec = 0;			// 초 단위
	TimeOut.tv_usec = 100;		// 마이크로 초 단위

	// 복사 한 소켓 리스트를 담기 위한 구조체 선언 및 초기화
	fd_set CopySocketLists;
	FD_ZERO(&CopySocketLists);

	while (true)
	{
		// 아래 select 에서 소켓 리스트를 날려버리기 때문에 원본을 복사해주도록 합니다.
		CopySocketLists = ReadSocketList;

		// 아래와 같은 방법을 polling 이라고 합니다.
		// 이를 위에서 정의한 0.1초마다 계속해서 맞게 왔는지 원본과 비교해서 물어보는 작업을 합니다.
		// 항상 구조체들은 포인터로 전달하도록 합니다.

		// 1. 가장 먼저 이벤트가 발생했는지(0.1초 마다) 물어봅니다.
		int EventSocketCount = select(0, &CopySocketLists, nullptr, nullptr, &TimeOut);
		if (EventSocketCount == 0)
		{
			// no event
			//cout << "done ?" << endl;
		}
		else if (EventSocketCount < 0)
		{
			// error
		}
		else
		{
			// process
			// 2. 이후 만약 이벤트가 발생했다면..
			// 총 갯수를 OS 가 알려준 것과 내가 가진것을 비교 합니다.
			for (int i = 0; i < (int)ReadSocketList.fd_count; i++)
			{
				// 첫 번째 인자 안에 소켓이 두번째 인자에 들어있는지 물어봅니다. 
				if (FD_ISSET(ReadSocketList.fd_array[i], &CopySocketLists))
				{
					// 3. 연결 처리에 대한 작업입니다.
					// 만약 맞다면 이것이 ListenSocket 의 것과 동일한지도 물어봅니다.
					if (ReadSocketList.fd_array[i] == ListenSocket)
					{
						SOCKADDR_IN ClientSockAddr;
						memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
						int ClientSockAddrLength = sizeof(ClientSockAddr);

						SOCKET NewClientSocket = accept(ReadSocketList.fd_array[i], (SOCKADDR*)&ClientSockAddr, &ClientSockAddrLength);

						// 감시 리스트에 추가
						FD_SET(NewClientSocket, &ReadSocketList);
						// 접속하는 녀석의 아이디를 찍어주는 출력문
						cout << "connect client : " << inet_ntoa(ClientSockAddr.sin_addr) << endl;
					}
					else
					{
						// recv
						char Buffer[1024] = { 0, };
						int RecvLength = recv(ReadSocketList.fd_array[i], Buffer, 1024, 0);

						if (RecvLength == 0)
						{
							// Disconnected
							cout << "Disconnected client : " << ReadSocketList.fd_array[i] << endl;

							// 해당 순번의 소켓을 ReadSocketList 이 리스트에서 제거 합니다.
							FD_CLR(ReadSocketList.fd_array[i], &ReadSocketList);
						}
						else if (RecvLength < 0)
						{
							// Disconnected and Lan cut Error
							cout << "Error Disconnected client : " << ReadSocketList.fd_array[i] << endl;
							FD_CLR(ReadSocketList.fd_array[i], &ReadSocketList);
						}
						else
						{
							cout << "recv client : " << ReadSocketList.fd_array[i] << ", " << Buffer << endl;
							send(ReadSocketList.fd_array[i], Buffer, RecvLength, 0);
						}
					}
				}
			}
		}
	}


	WSACleanup();

	return 0;
}