#include "stdafx.h"

/* @function listLength: find how many elements there are in the addrinfo linked-list pointed by the head pointer
	@param item: A pointer to the header of linked-list
	@return: number of element
	*/
int listLength(struct addrinfo* item){
	struct addrinfo* cur = item;
	int size = 0;
	while (cur != NULL){
		++size;
		cur = cur->ai_next;
	}
	return size;
}
int main(int argc, char *argv[]){
	//Step 1: Init winsock
	WSAData ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws))
		printf("Error %d, Cannot start up winsock\n", GetLastError());
	printf("Start up winsock success \n");
	//Step 2: Init Socket
	SOCKET listener = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	//Step 3:  Bind address to socket
	sockaddr_in listenerInfo;
	listenerInfo.sin_family = AF_INET;
	unsigned short port = (unsigned short)strtoul(argv[1], NULL, 0);
	listenerInfo.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &listenerInfo.sin_addr);
	if (bind(listener, (const sockaddr*)&listenerInfo, sizeof(listenerInfo))) {
		printf("Error %d, cannot bind address to socket\n", GetLastError());
		_getch();
		return 0;
	}
	printf("Bind address to socket success! \n");
	//Step 4: Communicate with client 
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE], clientIP[INET_ADDRSTRLEN];
	int ret, clientAddrLen = sizeof(clientAddr), clientPort;

	while (true) {
		printf("Waiting Client\n");
		ret = recvfrom(listener, buff, BUFF_SIZE, 0, (sockaddr *)&clientAddr, &clientAddrLen);
		if (ret == SOCKET_ERROR)
			printf("Error: %d, Cannot receive from client or data invalid \n", GetLastError());
		else {
			buff[ret] = 0;
			printf("Received data from client: %s \n", buff);
			printf("Send to client: \n");
			printf("-----------------------------START----------------------------\n");

			// handle data
			// Translate received domain name to ip address
			sockaddr_in *address;
			addrinfo hints, *result;
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;
			
			if (getaddrinfo(buff, NULL, &hints, &result)==0) {

				//First, send length of linked list to client
				_itoa_s(listLength(result), buff, 10); // convert int to string
				sendto(listener, buff, sizeof(buff), 0, (SOCKADDR *)&clientAddr, clientAddrLen);

				//Then send all ip address translated in list to client
				do {
					char ipStr[INET_ADDRSTRLEN+1];
					ipStr[0] = '+';
					address = (struct sockaddr_in *) result->ai_addr;
					inet_ntop(AF_INET, &address->sin_addr, ipStr+1, sizeof(ipStr));	
					printf("%s \n", ipStr); 
					ret = sendto(listener, ipStr, INET_ADDRSTRLEN, 0, (SOCKADDR *) &clientAddr, clientAddrLen); // send to client	
					result = result->ai_next;	
					if (ret == SOCKET_ERROR)
						printf("Error: %d, cannot send to client\n", WSAGetLastError());
				} while (result); 
				// end loop browse list

				
			}
			else {
				sendto(listener, "0", sizeof(buff), 0, (SOCKADDR *)&clientAddr, clientAddrLen);
				printf("-Not found information\n");
				sendto(listener, "-Not found information", 23, 0, (SOCKADDR *)&clientAddr, clientAddrLen);
			}

			printf("------------------------------END-----------------------------\n");
		}
	} // end loop
	// Step 5: close socket
	closesocket(listener);
	// Step 6: close winsock
	WSACleanup();

}