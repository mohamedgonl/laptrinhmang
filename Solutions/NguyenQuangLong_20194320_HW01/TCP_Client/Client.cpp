// TCP Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main(int argc, char *argv[])
{
	// Step 1: init winsock
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		printf("Init winsock failed \n");
		return 0;
	}
	printf("Init winsock success \n");

	// Step 2: Construct socket 
	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket \n", WSAGetLastError());
		return 0;
	}
	// optional: Set time-out for receiving 
	int tv = 10000; // Time-out interval: 10000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO,(const char*)(&tv),sizeof(int));

	// Step 3: Specify server address 
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	unsigned short port = (unsigned short)strtoul(argv[2], NULL, 0);
	serverAddr.sin_port = htons(port);
	char *addrString = argv[1];
	inet_pton(AF_INET,addrString , &serverAddr.sin_addr);

	// Step 4: Request to connect server
	if (connect(client, (const sockaddr*)&serverAddr, sizeof(serverAddr))) {
		printf("Error %d: cannot connect server.\n", WSAGetLastError());
		return 0;
	}
	printf("Connect server success! \n");

	//Step 5: Communicate with server 
	char buff[BUFF_SIZE];
	int ret, messageLen;
	while (true)
	{
		// send message
		printf("Send to server: ");
		gets_s(buff, BUFF_SIZE);
		messageLen = strlen(buff);
		if (messageLen == 0) {
			printf("\n[END] Got signal close client");
			break;
		}
		ret = send(client, buff, messageLen, 0);
		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot send data.", WSAGetLastError());
		// Receive data from  server
		ret = recv(client, buff, BUFF_SIZE, 0);
		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT)
				printf("Time out!");
			else
			{
				printf("Error %d: Cannot receive data. \n", WSAGetLastError());
			}
		}
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			printf("Receive from server: %s \n", buff);
		}
	}
	// Step 6: close socket
	closesocket(client);
	//Step 7: close winsock
	WSACleanup();
    return 0;
} 


