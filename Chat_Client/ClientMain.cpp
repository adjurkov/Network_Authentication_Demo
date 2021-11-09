#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include "ProtocolManager.h"
#include "cBuffer.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512						// Default buffer length of our buffer in characters
#define DEFAULT_PORT "5150"	                    // Default port
#define SERVER "127.0.0.1"						// The IP of our server  
SOCKET connectSocket = INVALID_SOCKET;		    // Socket used to connect to the server
sPacket packet;                                 // Packet structure and protocol manager
int result;									    // Results of any commands used

bool quit = false;                              // Flag set to true when user presses ESC key
std::vector<char> userMessage;                  // User input stored as a vector
cBuffer buffer(4);                              // Buffer of data
bool nameIsSet = false;

int init()
{
	WSADATA wsaData;							// Contains Winsock data
	struct addrinfo* infoResult = NULL;			// Holds the address information of server
	struct addrinfo* ptr = NULL;
	struct addrinfo hints;

	//----------------------------------------   Winsock Initialization   ------------------------------------------------

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		printf("Winsock initialization failed with error: %d\n", result);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//------------------------------     Resolve Server Address and Port   --------------------------------------------

	result = getaddrinfo(SERVER, DEFAULT_PORT, &hints, &infoResult); // get address info from the server
	if (result != 0)
	{
		printf("getaddrinfo failed with error: %d\n", result);
		WSACleanup();
		return 1;
	}

	//-------------------------------------   Connect to Address Attempt   ---------------------------------------------

	for (ptr = infoResult; ptr != NULL; ptr = ptr->ai_next)
	{
		// Create a Socket for connecting to server
		connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (connectSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Non-blocking
		DWORD NonBlock = 1;
		result = ioctlsocket(connectSocket, FIONBIO, &NonBlock);
		if (result == SOCKET_ERROR)
		{
			printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		}

		// Connect to server
		result = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (result == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				break;
			}
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(infoResult);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	return 0;
}

int shutdown()
{
	// Shutdown the connection since no more data will be sent
	result = shutdown(connectSocket, SD_SEND);
	if (result == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	// Cleanup
	closesocket(connectSocket);
	WSACleanup();
	return 0;
}

void consoleOutput(std::string message)
{
	printf("%s", message.c_str());
	printf("\n");

	buffer._buffer.clear();
	buffer.readIndex = 0;
	buffer.writeIndex = 0;
}

// Receive until the peer closes the connection
int checkRecv()
{
	// Temporarily resizing the buffer 
	buffer._buffer.resize(500);

	result = recv(connectSocket, (buffer._buffer.data()), DEFAULT_BUFLEN, 0);
	if (result == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK){} // ignore 
		else {printf("Recv failed with error: %d\n", WSAGetLastError());}
	}
	else if (result > 0) // echo   
	{
		// Resizing the buffer appropriately 
		buffer._buffer.resize(result);

		// Did we at least get the packet length?
		if (buffer._buffer.size() >= 4)
		{
			int packetLength = buffer.readIntBE();

			// Did we receive the whole buffer?
			if (buffer._buffer.size() == packetLength)
			{
				// Deserialize the message
				int msgID = buffer.readIntBE();
				int msgLength = buffer.readIntBE();
				std::string message = buffer.readString(msgLength);

				// Output message to the screen
				consoleOutput(message);
			}
			else
			{
				printf("Didn't receive full message from server.");
			}
		}
	}
	else if (result == 0)
	{
		printf("Connection closed\n");
	}
	return 0;
}

int main(int argc, char** argv)
{
	// Initialize Winsock, setup connectSocket
	result = init();
	if (result != 0)
	{
		return result;
	}
	
	// Print welcome message
	printf("INFO-6016 Chat App\n");
	printf("Commands: \n");
	printf("	/name insertNameHere \n");
	printf("	/join insertRoomNameHere \n");
	printf("	/leave insertRoomNameHere \n\n");
	printf("	/register insertEmailHere insertPasswordHere \n\n");
	printf("	/authenticate insertEmailHere insertPasswordHere \n\n");
	printf("Once in a room, simply type your message and hit enter! \n");


//----------------------------------------   Main Loop Begins ------------------------------------------------
	while (!quit)
	{
		checkRecv();

		// Key was hit
		if (_kbhit())
		{
			char key = _getch();

			// Escape key pressed
			if (key == 27)
			{
				quit = true;
			}

			// Backspace key pressed
			else if (key == 8)
			{
				// Erase character from screen
				printf("%c", key);
				printf(" ");
				printf("%c", key);

				// Pop character from vector
				userMessage.pop_back();
			}

			// Enter key pressed
			else if (key == 13)
			{
				printf("\r");

				// Parses user input and serializes packet
				packet.SerializeUserCommand(packet, userMessage, buffer);
				
//----------------------------------------   Send Serialized Packet to Server   ------------------------------------------------

				result = send(connectSocket, buffer._buffer.data(), packet.header.packetLength, 0); 
				if (result == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(connectSocket);
					WSACleanup();
					return 1;
				}
			}

			else
			{
				// If any other key pressed, store in userMessage
				printf("%c", key);
				userMessage.push_back(key);
			}
		}
	}
	
	result = shutdown(); 
	if (result != 0)
	{
		return result;
	}

	system("pause");
	return 0;
}