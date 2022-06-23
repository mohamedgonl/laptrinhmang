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

DWORD		nEvents = 0;
DWORD		index;
ClientInfo	clients[WSA_MAXIMUM_WAIT_EVENTS];
WSAEVENT	events[WSA_MAXIMUM_WAIT_EVENTS];

int main(int agrc, char*argv[])
{
	// Step 0: input account.txt directory
	printf("Input account.txt directory: \n");
	do {
		if (inputDirectory()) break;
		else printf("Wrong directory, please input again:\n");
	} while (1);

	
	
	WSANETWORKEVENTS sockEvent;

	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported\n");
		return 0;
	}

	//Step 2: Construct LISTEN socket	
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	unsigned short port = (unsigned short)strtoul(argv[1], NULL, 0);
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	clients[0].connSock = listenSock;
	events[0] = WSACreateEvent(); //create new events
	nEvents++;

	// Associate event types FD_ACCEPT and FD_CLOSE
	// with the listening socket and newEvent   
	WSAEventSelect(clients[0].connSock, events[0], FD_ACCEPT | FD_CLOSE);


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

	char sendBuff[BUFF_SIZE], recvBuff[BUFF_SIZE];
	SOCKET connSock;
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	int ret, i;

	for (i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; i++) {
		clients[i].connSock = 0;
	}
	while (1) {
		//wait for network events on all socket
		index = WSAWaitForMultipleEvents(nEvents, events, FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED) {
			printf("Error %d: WSAWaitForMultipleEvents() failed\n", WSAGetLastError());
			printf("DEBUG 1\n");
			continue;
		}

		index = index - WSA_WAIT_EVENT_0;
		WSAEnumNetworkEvents(clients[index].connSock, events[index], &sockEvent);

		if (sockEvent.lNetworkEvents & FD_ACCEPT) {
			if (sockEvent.iErrorCode[FD_ACCEPT_BIT] != 0) {
				printf("FD_ACCEPT failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
				printf("DEBUG 2\n");
				continue;
			}

			if ((connSock = accept(clients[index].connSock, (sockaddr *)&clientAddr, &clientAddrLen)) == SOCKET_ERROR) {
				printf("Error %d: Cannot permit incoming connection.\n", WSAGetLastError());
				printf("DEBUG 3\n");
				continue;
			}

			//Add new socket into socks array
			int i;
			if (nEvents == WSA_MAXIMUM_WAIT_EVENTS) {
				printf("Too many clients.\n");
				closesocket(connSock);
				printf("DEBUG 4\n");
			}
			else
				for (i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; i++)
					if (clients[i].connSock == 0) {
						clients[i].connSock = connSock;
						events[i] = WSACreateEvent();
						WSAEventSelect(clients[i].connSock, events[i], FD_READ | FD_CLOSE);
						nEvents++;
						break;
					}

			//reset event
				//WSAResetEvent(events[index]);
		}

		if (sockEvent.lNetworkEvents & FD_READ) {
			//Receive message from client
			if (sockEvent.iErrorCode[FD_READ_BIT] != 0) {
				printf("FD_READ failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
				printf("DEBUG 5\n");
				continue;
			}

			ret = Receive(&clients[index], recvBuff, BUFF_SIZE, 0);

			//Release socket and event if an error occurs
			if (ret <= 0) {
				closesocket(clients[index].connSock);
				clients[index].connSock = 0;
				strcpy(clients[index].curAccount, "");
				strcpy(clients[index].unhandledQueue, "");
				WSACloseEvent(events[index]);
				for (DWORD i = index; i < nEvents; i++) {
					events[i] = events[i + 1];
					clients[i] = clients[i + 1];
				}
				nEvents--;
			}
			else {									//send to client
				handle_send(&clients[index], recvBuff);

				//reset event
				WSAResetEvent(events[index]);
			}
		}

		if (sockEvent.lNetworkEvents & FD_CLOSE) {
			if (sockEvent.iErrorCode[FD_CLOSE_BIT] != 0) {
				if (sockEvent.iErrorCode[FD_CLOSE_BIT] == 10053) {
					printf("Client disconnect!\n");
					closesocket(clients[index].connSock);
					clients[index].connSock = 0;
					strcpy(clients[index].curAccount, "");
					strcpy(clients[index].unhandledQueue, "");
					WSACloseEvent(events[index]);
					for (DWORD i = index; i < nEvents; i++) {
						events[i] = events[i + 1];
						clients[i] = clients[i + 1];
					}
					nEvents--;
					continue;
				}
				printf("FD_CLOSE failed with error %d\n", sockEvent.iErrorCode[FD_CLOSE_BIT]);
				printf("DEBUG 6\n");
				break;
			}
			//Release socket and event
			closesocket(clients[index].connSock);
			clients[index].connSock = 0;
			strcpy(clients[index].curAccount, "");
			strcpy(clients[index].unhandledQueue, "");
			WSACloseEvent(events[index]);
			for (DWORD i = index; i < nEvents; i++) {
				events[i] = events[i + 1];
				clients[i] = clients[i + 1];
			}
			nEvents--;
		}
	}
	return 0;
}

/* The recv() wrapper function */
int Receive(ClientInfo* clientInfo, char *buff, int size, int flags) {
	int n;
	n = recv(clientInfo->connSock, buff, size, flags);
	if (n == SOCKET_ERROR) {
		if (WSAGetLastError() == 10054) { // client program suddenlly shut down
			logout(clientInfo->curAccount);
			printf("Client disconnectsr1.\n");
		}
		else {
			printf("Error: %d \n", WSAGetLastError());
		}
	}
	else if (n == 0) {
		logout(clientInfo->curAccount);
		printf("Client disconnectsr2.\n");
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
	if (n == SOCKET_ERROR) {
		printf("Send Error: %d", WSAGetLastError());
		if(WSAGetLastError() == 10053){
				closesocket(clients[index].connSock);
				clients[index].connSock = 0;
				strcpy(clients[index].curAccount, "");
				strcpy(clients[index].unhandledQueue, "");
				WSACloseEvent(events[index]);
				for (DWORD i = index; i < nEvents; i++) {
					events[i] = events[i + 1];
					clients[i] = clients[i + 1];
				}
				nEvents--;
			}
		
	}
	return n;
}