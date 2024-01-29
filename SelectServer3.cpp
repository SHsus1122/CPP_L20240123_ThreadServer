#include <iostream>
#include <WinSock2.h>
#include <map>

#include "Packet.h"

using namespace std;

#pragma comment(lib, "ws2_32")

void ProcessPacket(SOCKET DataSocket);

map<SOCKET, Player> SessionList;

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
	char Header[4] = { 0, };

	//Header, 4Byte
	int RecvLength = recv(DataSocket, Header, 4, MSG_WAITALL);
	if (RecvLength <= 0)
	{
	}

	unsigned short DataSize = 0;
	EPacketType PacketType = EPacketType::Max;

	//disassemble header
	memcpy(&DataSize, &Header[0], 2);
	memcpy(&PacketType, &Header[2], 2);

	DataSize = ntohs(DataSize);
	PacketType = (EPacketType)(ntohs((u_short)PacketType));

	char Data[1024] = { 0, };

	//Data
	RecvLength = recv(DataSocket, Data, DataSize, MSG_WAITALL);
	if (RecvLength > 0)
	{
		//Packet Type
		switch (PacketType)
		{
		case EPacketType::C2S_Login:
			Player NewPlayer;
			memcpy(&NewPlayer.ID, &Data[0], 4);
			memcpy(&NewPlayer.X, &Data[4], 4);
			memcpy(&NewPlayer.Y, &Data[8], 4);

			//NewPlayer.ID = ntohl(NewPlayer.ID);
			NewPlayer.ID = (int)DataSocket;
			NewPlayer.X = ntohl(NewPlayer.X);
			NewPlayer.Y = ntohl(NewPlayer.Y);

			SessionList[DataSocket] = NewPlayer;

			cout << "Connect Client : " << SessionList.size() << endl;

			//S2C_Login
			PacketManager::PlayerData = NewPlayer;
			PacketManager::Type = EPacketType::S2C_Login;
			PacketManager::Size = 12;
			PacketManager::MakePacket(Data);

			int SendLength = send(DataSocket, Data, PacketManager::Size + 4, 0);


			//S2C_Spawn, 현재 플레이어리스트 다 준다.
			char SendData[1024];
			for (const auto& Receiver : SessionList)
			{
				for (const auto& Data : SessionList)
				{
					PacketManager::PlayerData = Data.second;
					PacketManager::Type = EPacketType::S2C_Spawn;
					PacketManager::Size = 12;
					PacketManager::MakePacket(SendData);

					int SendLength = send(Receiver.first, SendData, PacketManager::Size + 4, 0);
					cout << "Send Spawn Client" << endl;
				}
			}

			break;
		}
	}



}