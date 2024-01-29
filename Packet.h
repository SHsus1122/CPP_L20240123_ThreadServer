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
		// 자료를 만드는 형태는 아래와 같습니다. [] -> 1바이트 , 우리는 배열에 쪼개서 넣은 것(형태) 처럼 만들었습니다.
		// Size Type ID       X        Y
		// [][] [][] [][][][] [][][][] [][][][]
		// htons 를 사용한 이유는 Size 의 순서가 뒤바뀌면 결과값이 바뀌기 때문
		unsigned short Data = htons(Size);		// Player 정보 Size
		memcpy(&Buffer[0], &Data, 2);			// 2바이트 짜리 만들어서 Buffer 에 넣기

		Data = htons((unsigned short)(Type));	// Player 종류 Type
		memcpy(&Buffer[2], &Data, 2);			// 2바이트 짜리 만들어서 Buffer 에 넣기

		int Data2 = htons(PlayerData.ID);		// Player ID
		memcpy(&Buffer[4], &Data2, 4);			// 4바이트 짜리 만들어서 Buffer 에 넣기

		Data2 = htons(PlayerData.X);			// Player X 좌표
		memcpy(&Buffer[8], &Data2, 4);			// 4바이트 짜리 만들어서 Buffer 에 넣기

		Data2 = htons(PlayerData.Y);			// Player Y 좌표
		memcpy(&Buffer[12], &Data2, 4);			// 4바이트 짜리 만들어서 Buffer 에 넣기
	}
};


// static 사용으로 인한 기본값 초기화 작업
unsigned short PacketManager::Size = 0;
Player PacketManager::PlayerData = { 0, };
EPacketType PacketManager::Type = EPacketType::C2S_Login;


#endif // __PACKET_H__