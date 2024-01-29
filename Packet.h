#pragma once
#ifndef __PACKET_H__
#define __PACKET_H__

#include <cstring>
#include <WinSock2.h>

enum class EPacketType : unsigned short
{
	C2S_Spawn = 100,
	S2C_Spawn,
	C2S_Login,
	S2C_Login,
	C2S_Logout,
	S2C_Logout,
	C2S_Move,
	S2C_Move,

	Max
};


class Player
{
public:
	int X;
	int Y;
	int ID;
};


class PacketManager
{
public:
	static unsigned short Size;
	static EPacketType Type;
	static Player PlayerData;

	void static MakePacket(char* Buffer)
	{
		// �ڷḦ ����� ���´� �Ʒ��� �����ϴ�. [] -> 1����Ʈ , �츮�� �迭�� �ɰ��� ���� ��(����) ó�� ��������ϴ�.
		// Size Type ID       X        Y
		// [][] [][] [][][][] [][][][] [][][][]
		// htons �� ����� ������ Size �� ������ �ڹٲ�� ������� �ٲ�� ����
		unsigned short Data = htons(Size);		// Player ���� Size
		memcpy(&Buffer[0], &Data, 2);			// 2����Ʈ ¥�� ���� Buffer �� �ֱ�

		Data = htons((unsigned short)(Type));	// Player ���� Type
		memcpy(&Buffer[2], &Data, 2);			// 2����Ʈ ¥�� ���� Buffer �� �ֱ�

		int Data2 = htons(PlayerData.ID);		// Player ID
		memcpy(&Buffer[4], &Data2, 4);			// 4����Ʈ ¥�� ���� Buffer �� �ֱ�

		Data2 = htons(PlayerData.X);			// Player X ��ǥ
		memcpy(&Buffer[8], &Data2, 4);			// 4����Ʈ ¥�� ���� Buffer �� �ֱ�

		Data2 = htons(PlayerData.Y);			// Player Y ��ǥ
		memcpy(&Buffer[12], &Data2, 4);			// 4����Ʈ ¥�� ���� Buffer �� �ֱ�
	}
};


// static ������� ���� �⺻�� �ʱ�ȭ �۾�
unsigned short PacketManager::Size = 0;
Player PacketManager::PlayerData = { 0, };
EPacketType PacketManager::Type = EPacketType::C2S_Login;


#endif // __PACKET_H__