// TCP Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define ENDING_DELIMITER "\r\n"
#pragma warning(disable:4996)

/* @structure ThreadAgruments: contain infomation about a client that pass to CommunicateThread
*/
typedef struct ThreadAgruments {
	SOCKET connSock;
	sockaddr_in clientAddr;
	int clientPort;
	char  clientIP[INET_ADDRSTRLEN];
}ThreadArguments;


/*@function SumInString: find the Sum of all Digits in the String
@param str: string need find sum of all digits
@return: -a string "-Failed: String contains non-number character." if there is any non-number in the input string
-a string contain sum of all digits in the string if success
*/
char* SumInString(char *str) {
	int sum = 0;
	for (int i = 0; i < strlen(str); i++) {
		// if any character is non-number, loop end, else add to sum
		if (!isdigit(str[i])) return "-Failed: String contains non-number character.";
		else {
			sum += (int)str[i] - '0';
		}
	}
	static char res[47];
	itoa(sum, res, 10); // convert int to string
	return res;
}


/* @function CommunicateProcess: a thread communicate with each client
@lpParameter: a pointer point to client's info
*/
DWORD WINAPI CommunicateProcess(LPVOID lpParameter) {
	// get client infomation
	SOCKET      connSock   = ((struct ThreadAgruments *)lpParameter)->connSock;
	sockaddr_in clientAddr = ((struct ThreadAgruments *)lpParameter)->clientAddr;
	int         clientPort = ((struct ThreadAgruments *)lpParameter)->clientPort;
	char        *clientIP  = ((struct ThreadAgruments *)lpParameter)->clientIP;

	printf("Accept incoming connection from [%s : %d] \n", clientIP, clientPort);

	// Communicate with client
	int ret, clientAddrlen = sizeof(clientAddr);
	char buff[BUFF_SIZE];
	while (1) {
		ret = recv(connSock, buff, BUFF_SIZE, 0);
		if (ret == SOCKET_ERROR) {
			printf("Error %d: cannot receive data\n", WSAGetLastError());
			break;
		}
		else if (ret == 0)
		{
			printf("Client  [%s : %d] disconected \n", clientIP, clientPort);
			break;
		}
		else {
			buff[ret] = 0;
			printf("Receive from client [%s: %d]: %s \n", clientIP, clientPort, buff);	
			
		// handle stream transport begin
			// split string by ENDING_DELIMITER	
			char **stringArray = (char **)malloc(100 * sizeof(char));
			int index = 0;
			char *p;
			p = strtok(buff, ENDING_DELIMITER); 
			while (p != NULL)
			{
				stringArray[index] = p;
				index++;
				p = strtok(NULL, ENDING_DELIMITER); \
			}
			// send sub-strings handled to client
			for (int i = 0; i < index; i++) {
				char *res = SumInString(stringArray[i]);
				ret = send(connSock, res, 47, 0);
				if (ret == SOCKET_ERROR) {
					printf("Error %d: Cannot send data. \n", WSAGetLastError());
					break;
				}
				Sleep(100); // wait client received data
				}

			// handle stream transport end		
		}
	}
	closesocket(connSock);
}

int main(int agrc, char*argv[])
{
	// Step 1: init winsock
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		printf("Init winsock failed");
		return 0;
	}
	printf("Init winsock success\n");

	// Step 2: Construct socket 
	SOCKET listentSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listentSocket == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket ", WSAGetLastError());
		return 0;
	}
	// Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	unsigned short port = (unsigned short)strtoul(argv[1], NULL, 0);
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	if (bind(listentSocket, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error %d Cannot associate a local address", WSAGetLastError());
		return 0;
	}
	// Step 4: Listen request from client
	if (listen(listentSocket, 10)) {
		printf("Error %d: cannot listen", WSAGetLastError());
		return 0;
	}
	printf("Server start listening! \n");

	//Step 5: Communicate with client

	SOCKET connSock;
	ThreadAgruments  clientInfo;

	//Server start
	while (true) {
		connSock = SOCKET_ERROR;
		// waiting for client 
		while (connSock == SOCKET_ERROR) {

			sockaddr_in clientAddr;
			int clientAddrlen = sizeof(clientAddr), clientPort;
			connSock = accept(listentSocket, (sockaddr*)&clientAddr, &clientAddrlen);

			//if client was accepted, save client info to struct variable 'clientInfo'
			if (connSock != SOCKET_ERROR) {
				clientInfo.clientAddr = clientAddr;
				clientInfo.connSock = connSock;
				clientInfo.clientPort = ntohs(clientAddr.sin_port);
				inet_ntop(AF_INET, &clientAddr.sin_addr, clientInfo.clientIP, sizeof(clientInfo.clientIP));
			}

		}
		// When a client is accepted, create a thread serve the client
		printf("Client connected! \n");
		DWORD dwThreadId;
		CreateThread(NULL, 0, CommunicateProcess, (LPVOID)&clientInfo, 0, &dwThreadId);
	}
	// //Server end
	//Step 6: close socket
	closesocket(connSock);
	closesocket(listentSocket);
	//Step 7: close winsock
	WSACleanup();
	return 0;
}
