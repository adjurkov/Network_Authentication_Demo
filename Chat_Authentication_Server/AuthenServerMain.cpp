#define WIN32_LEAN_AND_MEAN	

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <map>
#include <iostream>

#include "DBHelper.h"
#include "cBuffer.h"
#include "gen/authentication.pb.h"
#include "ProtocolManager.h"

DBHelper database;

#pragma comment (lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5050"

SOCKET clientSocket = INVALID_SOCKET;
SOCKET listenSocket = INVALID_SOCKET;

int result;

DWORD RecvBytes;
DWORD SentBytes;
DWORD NonBlock = 1;

cBuffer* buffer;

//--------------------------------------------------------------------------------------------------------------

int init()
{
	database.Connect("127.0.0.1", "root", "root");
	database.CreateAccount("ana_djurkovic@hotmail.com", "password");
	//system("Pause");

	//return 0;

	//----------------------------------------   Initialize Winsock   ------------------------------------------------
	WSADATA wsaData;

	// Initialize Winsock
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		printf("Winsock initialization failed with error: %d\n", result);
		return 1;
	}
	else
	{
		printf("Winsock initialization was successful!\n");
	}

	//----------------------------------------   Create Sockets   ------------------------------------------------


	struct addrinfo* addrResult = NULL;
	struct addrinfo hints;

	// Define our connection address info 
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addrResult);
	if (result != 0)
	{
		printf("getaddrinfo() failed with error %d\n", result);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("getaddrinfo() is successful!\n");
	}

	// Create a SOCKET for connecting to the server
	listenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket() failed with error %d\n", WSAGetLastError());
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("socket() is created!\n");
	}

	//------------------------------------------   Bind Socket   --------------------------------------------------
	result = bind(listenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		printf("bind() failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(addrResult);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("bind() is successful!\n");
	}

	// We don't need this anymore
	freeaddrinfo(addrResult);

	//-------------------------------------------   Listening    ---------------------------------------------------
	result = listen(listenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		printf("listen() failed with error %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("Listening for connections...\n");
	}
	//result = ioctlsocket(listenSocket, FIONBIO, &NonBlock);
	//if (result == SOCKET_ERROR)
	//{
	//	printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
	//	closesocket(listenSocket);
	//	WSACleanup();
	//	return 1;
	//}
	//printf("ioctlsocket() was successful!\n");

	clientSocket = accept(listenSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET)
	{
		printf("accept() failed with error %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("New client connected on socket %d\n", (int)clientSocket);
	}

	// idk if i should be closing this
	closesocket(listenSocket);
	
}

int main(int argc, char** argv)
{
	buffer = new cBuffer();

	// Initialize winsock, create sockets, bind, listen
	init();

	//----------------------------------------   Network Loop   ------------------------------------------------
	printf("Entering accept/recv/send loop...\n");
	do
	{
		result = recv(clientSocket, buffer->_buffer.data(), DEFAULT_BUFLEN, 0);
		if (result > 0)// Result > 0 we receive data, if 0 connection lost
		{
			printf("Bytes received: %d\n", result);
			int packetSize = buffer->readIntBE();
			packetSize -= 4;
			std::string data = buffer->readString(packetSize);

			// Deserializing using protobuf
			authentication::CreateAccountWeb deserializedNewAccount;
			bool success = deserializedNewAccount.ParseFromString(data);
			if (!success){ printf("Failed to parse message\n");}
			printf("Parsing successful\n");

//***************************************  REQUEST ID: REGISTER  ************************************************
			// delete this later
			std::cout << deserializedNewAccount.requestid() << std::endl;
			std::cout << deserializedNewAccount.email() << std::endl;
			std::cout << deserializedNewAccount.plaintextpassword() << std::endl;

			switch (deserializedNewAccount.requestid())
			{
				case 
			}


			// Echo back to sender
			result = send(clientSocket, buffer->_buffer.data(), result, 0);
			if (result == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(clientSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", result);
		}
		else if (result == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
	} while (true);


	// Close
	result = shutdown(clientSocket, SD_SEND);
	if (result == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	// Cleanup
	closesocket(clientSocket);
	WSACleanup();
	delete buffer;

	return 0;
}