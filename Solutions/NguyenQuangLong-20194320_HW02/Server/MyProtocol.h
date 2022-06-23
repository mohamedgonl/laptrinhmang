
#include "account.h"
#include "stdafx.h"
#ifndef MyProtocol_h
#define MyProtocol_h

typedef struct {
	char  method[MAX_METHOD_LENGTH];
	char payload[MAX_PAYLOAD_LENGTH];
}MyMessage;

/*
@function getMessage: convert buff to protocol message
@param buff: a string need convert
@return: a protocol message
*/
MyMessage getMessage(char* buff) {
	char msg[BUFF_SIZE];
	strcpy(msg, buff);
	MyMessage res;
	int i = 0;
	while (msg[i] != ' ' && i<MAX_METHOD_LENGTH) i++; // find first space
	msg[i] = '\0'; // set the first space to 'end string' character

	// the method end at the firt space in message and the following is payload 
	strcpy(res.method, msg);
	if (i <= MAX_METHOD_LENGTH)
		strcpy(res.payload, &msg[i + 1]);
	//	printf("Method: %s  data: %s ", res.method, res.payload);
	return res;
}

/*
@function handleRequest: hanlde request from client
@param:
- msg:  protocol message from client
- curAccount: current account is logging in (if exist)
@return: a response code defined
*/
char* handleRequest(MyMessage msg, char*curAccount) {

	// login
	if (!strcmp(msg.method, "USER")) {

		if (isLogged(msg.payload)) return ACCOUNT_LOGGED_IN; // case 1:the account has logged in somewhere

		else if (strcmp(curAccount, "")) return SESSION_LOGGED_IN; // case 2: the client session is actived

		else if (getStatus(msg.payload) == 1) return ACCOUNT_LOCKED;// case 3: the account has been locked

		else if (getStatus(msg.payload) == -2) return ACCOUNT_NOT_EXIST;// case 4: account doesnt exit

		else if (getStatus(msg.payload) == -1) return CANT_ACCESS_DATA; // case 5: cant access data file


		else {// others: OK
			login(msg.payload);
			strcpy(curAccount, msg.payload);
			return SUCCESS_LOGIN;
		}
	}

	// post
	else if (!strcmp(msg.method, "POST")) {

		if (!strcmp(curAccount, "")) return NOT_LOGGIN_YET; // if the account hasnt logged in
		else {
			return SUCCESS_POST;
		}
	}

	// logout
	else if (!strcmp(msg.method, "BYE")) {
		if (!strcmp(curAccount, "")) return NOT_LOGGIN_YET;
		else {
			logout(curAccount);
			strcpy(curAccount, "");
			return SUCCESS_LOGOUT;
		}
	}

	// others
	else
		return REQUEST_UNDEFINED;

}
#endif // !MyProtocol.h
