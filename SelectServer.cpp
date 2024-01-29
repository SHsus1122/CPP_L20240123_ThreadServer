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
		}
		else if (EventSocketCount < 0)
		{
			// error
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
					// 3. ���� ó���� ���� �۾��Դϴ�.
					// ���� �´ٸ� �̰��� ListenSocket �� �Ͱ� ���������� ����ϴ�.
					if (ReadSocketList.fd_array[i] == ListenSocket)
					{
						SOCKADDR_IN ClientSockAddr;
						memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
						int ClientSockAddrLength = sizeof(ClientSockAddr);

						SOCKET NewClientSocket = accept(ReadSocketList.fd_array[i], (SOCKADDR*)&ClientSockAddr, &ClientSockAddrLength);

						// ���� ����Ʈ�� �߰�
						FD_SET(NewClientSocket, &ReadSocketList);
						// �����ϴ� �༮�� ���̵� ����ִ� ��¹�
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

							// �ش� ������ ������ ReadSocketList �� ����Ʈ���� ���� �մϴ�.
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