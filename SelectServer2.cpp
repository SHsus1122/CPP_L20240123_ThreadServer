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

	fd_set ReadSocketList;		// ������ ��û�� ���� ���� ����Ʈ�� �ִ� ����ü
	FD_ZERO(&ReadSocketList);	// �ʱ�ȭ �۾�

	// �� ��ũ�� �ǹ̴� ã�ƺ���...
	FD_SET(ListenSocket, &ReadSocketList);

	struct timeval TimeOut;		// �󸶳� ��ٸ����� ���� �ð��� ������ ����ü
	TimeOut.tv_sec = 0;			// �� ����
	TimeOut.tv_usec = 100;		// ����ũ�� �� ����

	// ���� �� ���� ����Ʈ�� ��� ���� ����ü ���� �� �ʱ�ȭ
	fd_set CopySocketLists;
	FD_ZERO(&CopySocketLists);

	while (true)
	{
		// �Ʒ� select ���� ���� ����Ʈ�� ���������� ������ ������ �������ֵ��� �մϴ�.
		CopySocketLists = ReadSocketList;

		// �Ʒ��� ���� ����� polling �̶�� �մϴ�.
		// �̸� ������ ������ 0.1�ʸ��� ����ؼ� �°� �Դ��� ������ ���ؼ� ����� �۾��� �մϴ�.
		// �׻� ����ü���� �����ͷ� �����ϵ��� �մϴ�.

		// 1. ���� ���� �̺�Ʈ�� �߻��ߴ���(0.1�� ����) ����ϴ�.
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
			// 2. ���� ���� �̺�Ʈ�� �߻��ߴٸ�..
			// �� ������ OS �� �˷��� �Ͱ� ���� �������� �� �մϴ�.
			for (int i = 0; i < (int)ReadSocketList.fd_count; i++)
			{
				// ù ��° ���� �ȿ� ������ �ι�° ���ڿ� ����ִ��� ����ϴ�. 
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
	int RecvLength = recv(DataSocket, Header, 4, MSG_WAITALL);	// MSG_WAITALL 4 ����Ʈ�� ���� �� ���� ���
	if (RecvLength <= 0)
	{
	}

	unsigned short DataSize = 0;
	EPacketType PacketType = EPacketType::Max;

	// Disassemble Header(��� ����)
	memcpy(&DataSize, &Header[0], 2);
	memcpy(&PacketType, &Header[2], 2);

	// �ڷᰡ �������� �� ���̱� ������ �̸� ������� �ǵ����� �۾�
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
			// ������ ���� ID �� ������ �ϴµ� ���⼭�� ����
			// ����Ͽ����� wifi �� �ٲ�� IP �� �ٲ�� ������ �� �ڵ�δ� ����Ͽ��� ����ϸ� �ȵ˴ϴ�.
			// �׷��� �̸� �����ϱ� ���ؼ��� �����δ� UUID �� ����ϴ� ���Դϴ�.
			Player NewPlayer;
			memcpy(&NewPlayer.ID, &Data[0], 4);
			memcpy(&NewPlayer.X, &Data[4], 4);
			memcpy(&NewPlayer.Y, &Data[8], 4);

			//NewPlayer.ID = ntohl(NewPlayer.ID);
			NewPlayer.ID = (int)DataSocket;			// PlayerID �� ������ �ѹ�
			NewPlayer.X = ntohl(NewPlayer.X);
			NewPlayer.Y = ntohl(NewPlayer.Y);

			SessionList[DataSocket] = NewPlayer;

			cout << "Connect Client : " << SessionList.size() << endl;

			// ������ �α��� �� �� ������ ��������� ���� �̰��� Ŭ���̾�Ʈ���� ����
			// S2C_Login
			PacketManager::PlayerData = NewPlayer;
			PacketManager::Type = EPacketType::S2C_Login;
			PacketManager::Size = 12;
			PacketManager::MakePacket(Data);

			int SendLength = send(DataSocket, Data, PacketManager::Size + 4, 0);

			// S2C_Spawn, ���� �÷��̾��Ʈ�� ����� ��� Ŭ���̾�Ʈ�鿡�� �ݴϴ�.
			char SendData[1024];
			for (const auto& Receiver : SessionList)
			{
				// auto ���� �ۼ��Ϸ��� �Ʒ�ó�� �ۼ��ؼ� ���� �����ؾ� �Ѵٰ� �մϴ� .. ?
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