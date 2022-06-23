// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main(int arg, char *argv[])
{
	// Step 1: Inittiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported.\n");
			printf("Client started!\n");
	//Step 2: Construct socket	
	SOCKET client;
	client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//(optional) Set time-out for receiving
	int tv = 10000; //Time-out interval: 10000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO,
		(const char*)(&tv), sizeof(int));
	//Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	unsigned short port = (unsigned short)strtoul(argv[2], NULL, 0);
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);
	
		//Step 4: Communicate with server
	char buff[BUFF_SIZE];
	int ret, serverAddrLen = sizeof(serverAddr);

	do {
		//Input message
		printf("Send to server: ");
		gets_s(buff, BUFF_SIZE);

		// catch if input is empty string or ctrl+C then close client
		if (strlen(buff) == 0 || signal(SIGINT,NULL)) {
			printf("[END] Got signal for closing client! ");
			break;
		}

		//send message
		ret = sendto(client, buff, strlen(buff), 0,
			(sockaddr *)&serverAddr, serverAddrLen);
		if (ret == SOCKET_ERROR)
			printf("Error! Cannot send mesage.");

		//Receive message from server
			//First, receive length of linked list
		recvfrom(client, buff, BUFF_SIZE, 0, NULL, NULL);
		int listSize = atoi(buff);
		if (listSize != 0) {
			printf("Received %d translated IP address from server: \n", listSize);
		}
		else listSize++; // if listSize = 0, increase variable listSize to 1 for recveive last one message from server
			
			//Then receive all messages from server
		for (int i = 0; i < listSize ; i++) {
			ret = recvfrom(client, buff, BUFF_SIZE, 0,
				(sockaddr *)&serverAddr, &serverAddrLen);
			if (ret == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT)
					printf("Time-out! \n");
				else printf("Error! Cannot receive message.\n");
			}
			else {
				buff[ret] = '\0';
				printf("%s\n", buff);
			}
			_strupr_s(buff, BUFF_SIZE);
		}//End for_loop - Received all messages!!!
	} while (1); 
	//end while
										
	//Step 5: Close socket
	closesocket(client);
	//Step 6: Terminate Winsock
	WSACleanup();
}

