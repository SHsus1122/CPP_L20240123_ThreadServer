#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <vector>
#include <set>
#include <map>

#pragma comment(lib, "ws2_32")

std::set<SOCKET> SessionList;

CRITICAL_SECTION SessionCS;

// 아래처럼 코드를 작성한 것은 클라이언트와 지속적으로 통신하며,
// 클라이언트로부터 데이터를 수신하면 그 데이터를 다시 클라이언트에게 전송하는 방식입니다.
unsigned WINAPI WorkerThread(void* Arg)
{
	// 함수에 전달된 인자 "Arg" 로부터 클라이언트 소켓(ClientSocket)을 가져옵니다.
	// 함수가 스레드로 실행될 때 클라이언트 소켓을 전달하기 위해 사용됩니다.
	SOCKET ClientSocket = *((SOCKET*)Arg);
	while (true)
	{
		char Buffer[1024] = { 0, };
		int RecvLength = recv(ClientSocket, Buffer, sizeof(Buffer), 0);
		if (RecvLength <= 0)
		{
			EnterCriticalSection(&SessionCS);
			SessionList.erase(ClientSocket);
			LeaveCriticalSection(&SessionCS);
			closesocket(ClientSocket);
			//ExitThread(-1);
			break;
		}
		else
		{
			EnterCriticalSection(&SessionCS);
			for (auto ConenctSocket : SessionList)
			{
				int SendLength = send(ConenctSocket, Buffer, RecvLength, 0);
			}
			LeaveCriticalSection(&SessionCS);
		}
	}
	return 0;
}

int main()
{
	InitializeCriticalSection(&SessionCS);

	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ListenSockAddr = { 0 , };
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(22222);

	bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));

	listen(ListenSocket, 5);

	while (true)
	{
		SOCKADDR_IN ClientSockAddr = { 0 , };
		int ClientSockAddrSize = sizeof(ClientSockAddr);
		SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSockAddr, &ClientSockAddrSize);

		//thread 실행, worker thread
		//CreateThread();
		// 
		// _beginthreadex 함수를 사용하여 새로운 스레드를 생성합니다.
		// 생성된 스레드는 WorkerThread 함수를 실행하고, 해당 스레드에게 "ClientSocket" 를 전달하기 위해 
		// 함수 인자로 "(void*)& ClientSocket" 을 전달합니다. 반환된 스레드 핸들은 "ThreadHandle" 에 저장됩니다.
		HANDLE ThreadHandle = (HANDLE)_beginthreadex(0, 0, WorkerThread, (void*)&ClientSocket, 0, 0);

		//TerminateThread(ThreadHandle, -1); // 사용 X
		EnterCriticalSection(&SessionCS);
		SessionList.insert(ClientSocket);
		LeaveCriticalSection(&SessionCS);

	}

	closesocket(ListenSocket);

	WSACleanup();

	DeleteCriticalSection(&SessionCS);

	return 0;
}