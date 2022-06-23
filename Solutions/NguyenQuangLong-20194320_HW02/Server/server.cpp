// TCP-MultiThreads.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MyProtocol.h"

typedef struct {
	SOCKET connSock;
	sockaddr_in clientAddr;
	int clientPort;
	char  clientIP[INET_ADDRSTRLEN];
}ClientInfo;

/* serveThread - Thread to serve client */
unsigned __stdcall serveThread(void *clientInfo) {
	//get client info
	SOCKET connSocket = ((ClientInfo *)clientInfo)->connSock;
	int clientPort = ((ClientInfo *)clientInfo)->clientPort;
	char *clientIP = ((ClientInfo *)clientInfo)->clientIP;

	char curAccount[MAX_ACCOUNT_LENGTH] = ""; // curAccount = empty when there is no account is logging in the client session
	char rBuff[BUFF_SIZE];
	int ret;
	char unhandledQueue[BUFF_SIZE]=""; // cache buff for unhandled request

	while (1) {
		ret = recv(connSocket, rBuff, BUFF_SIZE, 0);
		if (ret == SOCKET_ERROR) { 
			if (WSAGetLastError() == 10054) { // client program suddenlly shut down
				logout(curAccount);
				printf("Client disconnects.\n");
			}
			else printf("Error %d: cannot receive data\n", WSAGetLastError());
			break;
		}
		else if (ret == 0) {
			printf("Client disconnects.\n");
			break;
		}
		rBuff[ret] = 0;
		printf("Receive from client[%s:%d] %s\n", clientIP, clientPort, rBuff);
		
//---------------------> handle stream trans start
			
			char **stringArray = (char **)malloc(100 * sizeof(char));
			int index = 0;
			char *p;
			bool end = true; // for checking if rBuff/last msg ending by ENDING_DELIMITER or not

			if (strcmp(rBuff+strlen(rBuff)-2,ENDING_DELIMITER)) end = false;
			
			strcat(unhandledQueue, rBuff); // push rBuff to unhandledQueue
		
			// split unhandledQueue by ENDING_DELIMITER
			p = strtok(unhandledQueue, ENDING_DELIMITER);
			while (p != NULL) {
				stringArray[index] = p;		
		
				index++;
				p = strtok(NULL, ENDING_DELIMITER);
			}	

			// if last message not end by ENDING_DELIMITER, push it to unhandledQueue
			if (!end) { 
				strcpy(unhandledQueue, stringArray[index-1]);
				index--;
			}
			
			// send sub-strings handled to client
			for (int i = 0; i < index; i++) {
				MyMessage sub = getMessage(stringArray[i]);
				char *sBuff = handleRequest(sub, curAccount);
				printf("Send to client: %s\n", sBuff);
				ret = send(connSocket, sBuff, strlen(sBuff), 0);
				if (ret == SOCKET_ERROR) {
					printf("Error %d: Cannot send data. \n", WSAGetLastError());
				}
			}
			// if rBuff end by ENDING_DELIMITER, all sub_strings are handled -> unhandledQueue is empty
			if (end) strcpy(unhandledQueue, "");
		
//-------------------> handle stream trans end
	}
	closesocket(connSocket);
	return 0;
}

int main(int agrc, char*argv[])
{
	// Step 0: input account.txt directory
	printf("Input account.txt directory: \n");
	do {
		if (inputDirectory()) break;
		else printf("Wrong directory, please input again:\n");
	} while (1);


	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported\n");
		return 0;
	}

	//Step 2: Construct socket	
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket ", WSAGetLastError());
		return 0;
	}

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	unsigned short port = (unsigned short)strtoul(argv[1], NULL, 0);
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot associate a local address with server socket.", WSAGetLastError());
		return 0;
	}

	//Step 4: Listen request from client
	if (listen(listenSock, 10)) {
		printf("Error %d: Cannot place server socket in state LISTEN.", WSAGetLastError());
		return 0;
	}

	printf("Server started!\n");

	//Step 5: Communicate with client

	SOCKET connSock;
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr);

	while (1) {

		SOCKET	connSock = accept(listenSock, (sockaddr *)& clientAddr, &clientAddrLen);
		if (connSock == SOCKET_ERROR)
			printf("Error %d: Cannot permit incoming connection.\n", WSAGetLastError());
		else {
			ClientInfo clientInfo;
			// save client ip address
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientInfo.clientIP, sizeof(clientInfo.clientIP));
			// save client port
			clientInfo.clientPort = ntohs(clientAddr.sin_port);
			// save client socket
			clientInfo.connSock = connSock;
			// save client socket address
			clientInfo.clientAddr = clientAddr;

			printf("Accept incoming connection from %s:%d\n", clientInfo.clientIP, clientInfo.clientPort);
			_beginthreadex(0, 0, serveThread, (void*)&clientInfo, 0, 0); //start thread
		}
	}
	closesocket(connSock);
	closesocket(listenSock);

	WSACleanup();

	return 0;
}

