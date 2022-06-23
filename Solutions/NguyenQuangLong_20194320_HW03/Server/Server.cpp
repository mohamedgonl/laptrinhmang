// TCP-MultiThreads.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MyProtocol.h"


typedef struct {
	SOCKET connSock;
	char curAccount[MAX_ACCOUNT_LENGTH] = ""; // the account is logging-in in the client session
	char unhandledQueue[BUFF_SIZE] = ""; // cache buff for unhandled request
}ClientInfo;

int Receive(ClientInfo* clientInfo, char *buff, int size, int flags);
int Send(ClientInfo* clientInfo, char *, int, int);

/* handle_send: handle received Buff then send to client
* @param clientInfo: Pointer to client 's info
* @param rBuff: a string received from client
*/
void handle_send(ClientInfo *clientInfo, char*rBuff) {

//---------------------> handle stream trans start
		char **stringArray = (char **)malloc(100 * sizeof(char));
		int index = 0;
		char *p;
		bool end = true; // for checking if rBuff/last msg ending by ENDING_DELIMITER or not

		if (strcmp(rBuff + strlen(rBuff) - 2, ENDING_DELIMITER)) end = false;

		strcat(clientInfo->unhandledQueue, rBuff); // push rBuff to unhandledQueue

		// split unhandledQueue by ENDING_DELIMITER
		p = strtok(clientInfo->unhandledQueue, ENDING_DELIMITER);
		while (p != NULL) {
			stringArray[index] = p;
			index++;
			p = strtok(NULL, ENDING_DELIMITER);
		}

		// if last message not end by ENDING_DELIMITER, push it to unhandledQueue
		if (!end) {
			strcpy(clientInfo->unhandledQueue, stringArray[index - 1]);
			index--;
		}

		// send sub-strings handled to client
		for (int i = 0; i < index; i++) {
			MyMessage sub = getMessage(stringArray[i]);
			char *sBuff = handleRequest(sub, clientInfo->curAccount);
			printf("Send to client: %s\n", sBuff);
			Send(clientInfo, sBuff, strlen(sBuff), 0);
		}
		// if rBuff end by ENDING_DELIMITER, all sub_strings are handled -> unhandledQueue is empty
		if (end) strcpy(clientInfo->unhandledQueue, "");
//-------------------> handle stream trans end
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
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported\n");

	//Step 2: Construct socket	
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	unsigned short port = (unsigned short)strtoul(argv[1], NULL, 0);
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error! Cannot bind this address.");
		return 0;
	}

	//Step 4: Listen request from client
	if (listen(listenSock, 10)) {
		printf("Error! Cannot listen.");
		return 0;
	}

	printf("Server started!\n");


	ClientInfo client[FD_SETSIZE];
	SOCKET connSock;
	fd_set readfds, initfds; //use initfds to initiate readfds at the begining of every loop step
	sockaddr_in clientAddr;
	int ret, nEvents, clientAddrLen;
	char rcvBuff[BUFF_SIZE], sendBuff[BUFF_SIZE];

	for (int i = 0; i < FD_SETSIZE; i++)
		client[i].connSock = 0;	// 0 indicates available entry

	FD_ZERO(&initfds);
	FD_SET(listenSock, &initfds);

	//Step 5: Communicate with clients
	while (1) {
		readfds = initfds;		/* structure assignment */
		nEvents = select(0, &readfds, 0, 0, 0);
		if (nEvents < 0) {
			printf("\nError! Cannot poll sockets: %d", WSAGetLastError());
			break;
		}

		//new client connection
		if (FD_ISSET(listenSock, &readfds)) {
			clientAddrLen = sizeof(clientAddr);
			if ((connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
				printf("\nError! Cannot accept new connection: %d", WSAGetLastError());
				break;
			}
			else {
				printf("You got a connection from %s\n", inet_ntoa(clientAddr.sin_addr)); /* prints client's IP */

				int i;
				for (i = 0; i < FD_SETSIZE; i++)
					if (client[i].connSock == 0) {
						client[i].connSock = connSock;
						FD_SET(client[i].connSock, &initfds);
						break;
					}

				if (i == FD_SETSIZE) {
					printf("\nToo many clients.");
					closesocket(connSock);
				}

				if (--nEvents == 0)
					continue; //no more event
			}
		}

		//receive data from clients
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (client[i].connSock == 0)
				continue;

			if (FD_ISSET(client[i].connSock, &readfds)) {
				ret = Receive(&client[i], rcvBuff, BUFF_SIZE, 0);
				if (ret <= 0) {
					logout(client[i].curAccount);
					// reset ClientInfo[i]
					FD_CLR(client[i].connSock, &initfds);
					closesocket(client[i].connSock);
					client[i].connSock = 0;
					strcpy(client[i].curAccount, "");
					strcpy(client[i].unhandledQueue, "");
				}
				else if (ret > 0) {
					handle_send(&client[i], rcvBuff);	
				}
			}
			if (--nEvents <= 0)
				continue; //no more event
		}

	}

	closesocket(listenSock);
	WSACleanup();
	return 0;
}

/* The recv() wrapper function */
int Receive(ClientInfo* clientInfo, char *buff, int size, int flags) {
	int n;
	n = recv(clientInfo->connSock, buff, size, flags);
	if (n == SOCKET_ERROR) {
		if (WSAGetLastError() == 10054) { // client program suddenlly shut down
			logout(clientInfo->curAccount);
			printf("Client disconnects.\n");
		}
		else {
			printf("Error: %d \n", WSAGetLastError());
		}
	}
	else
	{	
		buff[n] = 0;
		printf("Received from client: %s", buff);
	}
	return n;
}
/* The send() wrapper function*/
int Send(ClientInfo* clientInfo, char *buff, int size, int flags) {
	int n;
	n = send(clientInfo->connSock, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error: %", WSAGetLastError());
	return n;
}