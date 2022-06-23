// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
// --------------------------- library-----------------------------------
#include <string.h>
#include <malloc.h>
using namespace std;
#include "targetver.h"
#include "winsock2.h"
#include "stdlib.h"
#include "WS2tcpip.h"
#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>
#include <tchar.h>
#include "process.h"
#include <vector>
#include <iostream>
#include <algorithm>
//-----------------------------------------define const-------------------------------------
#define MAX_METHOD_LENGTH 5
#define MAX_PAYLOAD_LENGTH 1000
#define ENDING_DELIMITER "\r\n"
#define MAX_ACCOUNT_LENGTH 100
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
// respone code
#define SUCCESS_LOGIN "10"
#define ACCOUNT_LOCKED "11"
#define ACCOUNT_NOT_EXIST "12"
#define ACCOUNT_LOGGED_IN "13"
#define CANT_ACCESS_DATA "14"
#define SESSION_LOGGED_IN "15"
#define SUCCESS_POST "20"
#define NOT_LOGGIN_YET "21"
#define SUCCESS_LOGOUT "30"
#define REQUEST_UNDEFINED "99"




// TODO: reference additional headers your program requires here
