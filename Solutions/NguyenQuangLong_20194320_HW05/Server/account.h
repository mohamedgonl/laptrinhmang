#include "stdafx.h"
#ifndef account_h
#define account_h

char directory[9999]; // directory of file account.txt

					  /*
					  @function: inputDirectory: allow user on serverside input directory of account.txt
					  @return:
					  -true: if directory is correct
					  -false: if directory is wrong
					  */
bool inputDirectory() {
	scanf("%s", directory);
	try {
		FILE *fptr;
		fptr = fopen(directory, "r");
		if (!fptr) {
			return false;
		}
		return true;
	}
	catch (exception e) {
		printf("Error: %s\n", e);
		return false;
	}
}

vector<string> accLogged; // vector contain the accounts is logging

						  /*
						  @function isLogged: check if the account is logging at somewhere
						  param:
						  -username: username of the account
						  @return:
						  -true: if the account is logging
						  -false: others
						  */

bool isLogged(char* username) {
	std::vector<string>::iterator it;
	it = find(accLogged.begin(), accLogged.end(), username);
	if (it != accLogged.end()) {
		return true;
	}
	return false;
}


/*
@function getStatus: return value of status of account has the username
@param:
-username: account's username
@return:
- if success, return 1 (account is locked) or 0 (OK)
- if failed, return -1 (cant access the data file) or -2 (there isnt the account in data)
*/
int getStatus(char* username1) {
	char username[MAX_ACCOUNT_LENGTH];
	strcpy(username, username1);
	FILE *fptr;
	fptr = fopen(directory, "r");
	if (!fptr) {
		fclose(fptr);
		return -1;
	}
	char _username[MAX_ACCOUNT_LENGTH];
	int stt;
	while (fscanf(fptr, "%s %d", _username, &stt) != EOF) {
		if (strcmp(_username, username) == 0) {
			fclose(fptr);
			return stt;
		}
	}
	fclose(fptr);
	return -2;
}


/*
@function logout: remove an account from accLogged stack
@param:
-username: a string contain username of account
*/
void logout(char* username) {
	std::vector<string>::iterator it;
	it = find(accLogged.begin(), accLogged.end(), username);
	if (it != accLogged.end()) accLogged.erase(it);
}

/*
@function login: login a account
@param:
-username: a string contain username of account
return:
- true: if login success
- false: if login fail
*/
void login(char* username) {
	//accLogged.push_back(username); if you want the account is logged in by just one client, remove this comment
}
#endif // !account_h
