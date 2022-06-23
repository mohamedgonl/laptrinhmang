#include "stdafx.h"




#ifndef menu_h
#define menu_h
/*
@function showMenu: show Menu options
*/

void showMenu() {
	printf("------------------------------MENU---------------------------------\n");
	printf("Select option:\n");
	printf("1. Login\n");
	printf("2. Post content\n");
	printf("3. Logout\n");
	printf("0. Show menu\n");
	printf("-------------------------------------------------------------------\n");
}



/*
@function getData: allow user input option of menu and handle request
@param: 
-[OUT] data: a request buff for send to server
@return: 
-true: if request success
-false: if request is show menu or there is no option that user inputed
*/
bool getData(char* data) {
	printf("Select option (press 0 to show menu): ");
	int myChoice;
	char clean;
	scanf("%d", &myChoice);
	// clean cache when user input invalid ( like a char, string, ...)
	while ((clean = getchar()) != '\n' && clean != EOF);

	switch (myChoice) {
		case 1: {
			printf("Enter your username:");
			char username[BUFF_SIZE];
			scanf("\n%[^\n]s", username);
			strcpy(data, "USER ");
			strcat(data, username);
			strcat(data, ENDING_DELIMITER);
			return true;
		}
		case 2: {
			printf("Enter content:");
			char content[BUFF_SIZE];
			scanf("\n%[^\n]s", content);
			strcpy(data, "POST ");
			strcat(data, content);
			strcat(data, ENDING_DELIMITER);
			return true;
		}
		case 3: {
			strcpy(data, "BYE");
			strcat(data, ENDING_DELIMITER);
			return true;
		}
		case 0: {
			showMenu();
			return false;
		}
		default: {
			printf("--> There is no option you chose\n");
			return false;
		}
		}
}



/*
@function resolveRespone: convert respone code to respone message
@param: 
- res: respone code
@return:
- respone message
*/
char *resolveRespone(char* res) {
	if (!strcmp(res, "10")) return "Login success!";
	if (!strcmp(res, "11")) return "Account is locked!";
	if (!strcmp(res, "12")) return "Account doesn't exist!";
	if (!strcmp(res, "13")) return "Account is logging in somewhere!";
	if (!strcmp(res, "14")) return "404 Not found [Cant access database]!";
	if (!strcmp(res, "15")) return "You have logged in! Please logout before logging in another account";
	if (!strcmp(res, "20")) return "Post content success!";
	if (!strcmp(res, "21")) return "You haven't logged in!";
	if (!strcmp(res, "30")) return "Logout success!";
	if (!strcmp(res, "99")) return "Something error, request undefined";
	return "";
}




#endif // !1
