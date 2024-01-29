#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <map>

#include "Packet.h"

using namespace std;

#pragma comment(lib,"ws2_32")

void ProcessPacket(SOCKET DataSocket);

map<SOCKET, Player> SessionList;

int main()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));

	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(22222);

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
			continue;
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
					if (ReadSocketList.fd_array[i] == ListenSocket)
					{
						SOCKADDR_IN ClientSockAddr = { 0 , };
						int ClientSockAddrSize = sizeof(ClientSockAddr);
						SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSockAddr, &ClientSockAddrSize);
						FD_SET(ClientSocket, &ReadSocketList);
					}
					else
					{
						//char Buffer[1024] = { 0, };
						ProcessPacket(ReadSocketList.fd_array[i]);
						//int RecvLength = recv(ReadSocketList.fd_array[i], Buffer, sizeof(Buffer), 0);
						//if (RecvLength <= 0)
						//{
						//	closesocket(ReadSocketList.fd_array[i]);
						//	FD_CLR(ReadSocketList.fd_array[i], &ReadSocketList);
						//}
						//else
						//{
						//	//ProcessPacket
						//	
						//	//for (int j = 0; j < (int)ReadSocketList.fd_count; ++j)
						//	//{
						//	//	int SendLength = send(ReadSocketList.fd_array[j], Buffer, RecvLength, 0);
						//	//}
						//}
					}
				}
			}
		}
	}

	closesocket(ListenSocket);

	WSACleanup();

	return 0;
}

void ProcessPacket(SOCKET DataSocket)
{
	char Header[1024] = { 0, };

	// Header, 4Byte
	int RecvLength = recv(DataSocket, Header, 4, MSG_WAITALL);	// MSG_WAITALL 4 바이트를 받을 때 까지 대기
	if (RecvLength <= 0)
	{
	}

	unsigned short DataSize = 0;
	EPacketType PacketType = EPacketType::Max;

	// Disassemble Header(헤더 분해)
	memcpy(&DataSize, &Header[0], 2);
	memcpy(&PacketType, &Header[2], 2);

	// 자료가 뒤집혀서 올 것이기 때문에 이를 원래대로 되돌리는 작업
	DataSize = ntohs(DataSize);
	PacketType = (EPacketType)(ntohs((u_short)PacketType));

	char Data[1024] = { 0, };

	RecvLength = recv(DataSocket, Data, DataSize, MSG_WAITALL);
	if (RecvLength > 0)
	{
		// Packet Type
		switch (PacketType)
		{
		case EPacketType::C2S_Login:
			// 원래는 고유 ID 를 만들어야 하는데 여기서는 생략
			// 모바일에서는 wifi 가 바뀌면 IP 가 바뀌기 때문에 이 코드로는 모바일에서 사용하면 안됩니다.
			// 그래서 이를 방지하기 위해서는 실제로는 UUID 를 사용하는 편입니다.
			Player NewPlayer;
			memcpy(&NewPlayer.ID, &Data[0], 4);
			memcpy(&NewPlayer.X, &Data[4], 4);
			memcpy(&NewPlayer.Y, &Data[8], 4);

			//NewPlayer.ID = ntohl(NewPlayer.ID);
			NewPlayer.ID = (int)DataSocket;			// PlayerID 는 소켓의 넘버
			NewPlayer.X = ntohl(NewPlayer.X);
			NewPlayer.Y = ntohl(NewPlayer.Y);

			SessionList[DataSocket] = NewPlayer;

			cout << "Connect Client : " << SessionList.size() << endl;

			// 위에서 로그인 할 때 계정을 만들었으니 이제 이것을 클라이언트에게 전달
			// S2C_Login
			PacketManager::PlayerData = NewPlayer;
			PacketManager::Type = EPacketType::S2C_Login;
			PacketManager::Size = 12;
			PacketManager::MakePacket(Data);

			int SendLength = send(DataSocket, Data, PacketManager::Size + 4, 0);

			// S2C_Spawn, 현재 플레이어리스트를 연결된 모든 클라이언트들에게 줍니다.
			char SendData[1024];
			for (const auto& Receiver : SessionList)
			{
				// auto 없이 작성하려면 아래처럼 작성해서 만들어서 전달해야 한다고 합니다 .. ?
				// map<SOCKET, Player> Test;
				// pair<SOCKET, Player> One;
				for (const auto& Data : SessionList)
				{
					PacketManager::PlayerData = Data.second;
					PacketManager::Type = EPacketType::S2C_Spawn;
					PacketManager::Size = 12;
					PacketManager::MakePacket(SendData);

					int SendLength = send(Data.first, SendData, PacketManager::Size + 4, 0);
					cout << "Send Spawn Client" << endl;
				}
			}

			break;
		}
	}

}