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
#include "cProtobuf.h"
#include "cBuffer.h"

DBHelper database;

//#include <gen/authentication.pb.h>

#pragma comment (lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5050"


SOCKET clientSocket = INVALID_SOCKET;
SOCKET listenSocket = INVALID_SOCKET;
//SOCKET acceptSocket = INVALID_SOCKET;

int result;

DWORD RecvBytes;
DWORD SentBytes;
FD_SET ReadSet;
int total;
DWORD NonBlock = 1;

WSABUF dataBuf;
cBuffer* buffer;
cProtobuf* protobuf;

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
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	//----------------------------------------   Network Loop   ------------------------------------------------
	printf("Entering accept/recv/send loop...\n");
	do
	{

		std::string serializedMsg;

		/*	dataBuf.buf = protobuf
			dataBuf.len = client->buffer->_buffer.size();*/

		
		result = recv(clientSocket, recvbuf, recvbuflen, 0);
		// Result > 0 we receive data, if 0 connection lost
		if (result > 0)
		{
			printf("Bytes received: %d\n", result);
			/*	int packetLength = buffer->readIntBE();
				int msgID = buffer->readIntBE();

				std::cout << " packetlength:" << packetLength << std::endl;*/


			// echo TEST buffer back to sender
			result = send(clientSocket, recvbuf, result, 0);
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
		/*	}

		}*/
	} while (true);


	//					memcpy(client->buffer->_buffer.data(), client->dataBuf.buf, client->dataBuf.len);

	//					// Read in header
	//					packetLength = client->buffer->readIntBE();
	//					msgID = client->buffer->readIntBE();

	//					switch (msgID)
	//					{

	//						//***************************************  MESSAGE ID: SET NAME  ************************************************

	//					case SetName:
	//					{
	//						// Read in the rest of the packet, store username
	//						packet.usernameLength = client->buffer->readIntBE();
	//						packet.username = client->buffer->readString(packet.usernameLength);
	//						client->username = packet.username;

	//						// Preparing msg back to server
	//						packet.msg = "SERVER: Welcome, " + client->username + "!";
	//						packet.msgLength = packet.msg.length();
	//						packetLength = 4 + 4 + 4 + packet.msgLength;
	//						packet.header.packetLength = packetLength;
	//						packet.header.msgID = AcceptedUsername;

	//						// Clear the buffer
	//						client->buffer->_buffer.clear();
	//						client->buffer->readIndex = 0;
	//						client->buffer->writeIndex = 0;

	//						// Growing the buffer just enough to deal with packet
	//						int newBufferSize = packet.header.packetLength;
	//						client->buffer->_buffer.resize(newBufferSize);

	//						// Serialize
	//						client->buffer->writeIntBE(packet.header.packetLength);
	//						client->buffer->writeIntBE(packet.header.msgID);
	//						client->buffer->writeIntBE(packet.msgLength);
	//						client->buffer->writeString(packet.msg);

	//						// SENDING.............................................................. 

	//						client->dataBuf.buf = client->buffer->_buffer.data();
	//						client->dataBuf.len = client->buffer->_buffer.size();

	//						result = WSASend( //sending data right back to client
	//							client->socket,
	//							&(client->dataBuf),
	//							1,
	//							&SentBytes,
	//							Flags,
	//							NULL,
	//							NULL
	//						);
	//						if (SentBytes == SOCKET_ERROR)
	//							printf("send error %d\n", WSAGetLastError());
	//						else if (SentBytes == 0)
	//							printf("Send result is 0\n");
	//						else
	//							printf("Successfully sent %d bytes!\n", SentBytes);

	//						break;
	//					}

	//					//***************************************  MESSAGE ID: JOIN ROOM  ************************************************

	//					case JoinRoom:
	//					{
	//						// Read in the rest of the packet, store room name
	//						packet.roomLength = client->buffer->readIntBE();
	//						packet.roomname = client->buffer->readString(packet.roomLength);

	//						// New room
	//						if (clientsInRooms.count(packet.roomname) == 0)
	//						{
	//							clientsInRooms[packet.roomname] = {};
	//							clientsInRooms[packet.roomname].push_back(client);
	//						}
	//						// If room exists
	//						else
	//						{
	//							clientsInRooms[packet.roomname].push_back(client);
	//						}

	//						// Preparing msg back to server
	//						packet.msg = "SERVER: [" + client->username + "] has joined room [" + packet.roomname + "]";
	//						packet.msgLength = packet.msg.length();
	//						packetLength = 4 + 4 + 4 + packet.msgLength;
	//						packet.header.packetLength = packetLength;
	//						packet.header.msgID = JoinRoom;

	//						// Clear the buffer
	//						client->buffer->_buffer.clear();
	//						client->buffer->readIndex = 0;
	//						client->buffer->writeIndex = 0;

	//						// Growing the buffer just enough to deal with packet
	//						int newBufferSize = packet.header.packetLength;
	//						client->buffer->_buffer.resize(newBufferSize);

	//						// Serialize
	//						client->buffer->writeIntBE(packet.header.packetLength);
	//						client->buffer->writeIntBE(packet.header.msgID);
	//						client->buffer->writeIntBE(packet.msgLength);
	//						client->buffer->writeString(packet.msg);

	//						// SENDING.............................................................. 
	//						client->dataBuf.buf = client->buffer->_buffer.data();
	//						client->dataBuf.len = client->buffer->_buffer.size();

	//						// Sending everyone in the room that a user has joined that room
	//						for (int i = 0; i < clientsInRooms[packet.roomname].size(); i++)
	//						{
	//							result = WSASend( //sending data right back to client
	//								clientsInRooms[packet.roomname].at(i)->socket,
	//								&(client->dataBuf),
	//								1,
	//								&SentBytes,
	//								Flags,
	//								NULL,
	//								NULL
	//							);
	//							if (SentBytes == SOCKET_ERROR)
	//								printf("send error %d\n", WSAGetLastError());
	//							else if (SentBytes == 0)
	//								printf("Send result is 0\n");
	//							else
	//								printf("Successfully sent %d bytes!\n", SentBytes);
	//						}
	//						break;
	//					}
	//					//***************************************  MESSAGE ID: BROADCAST  ************************************************

	//					case Broadcast:
	//					{
	//						// Read in the rest of the packet, store user message
	//						packet.msgLength = client->buffer->readIntBE();
	//						packet.msg = client->buffer->readString(packet.msgLength);

	//						// Preparing msg back to server
	//						packet.msg = "SERVER: [" + client->username + "] says: " + packet.msg;
	//						packet.msgLength = packet.msg.length();
	//						packetLength = 4 + 4 + 4 + packet.msgLength;
	//						packet.header.packetLength = packetLength;
	//						packet.header.msgID = Broadcast;

	//						// Clear the buffer
	//						client->buffer->_buffer.clear();
	//						client->buffer->readIndex = 0;
	//						client->buffer->writeIndex = 0;

	//						// Growing the buffer just enough to deal with packet
	//						int newBufferSize = packet.header.packetLength;
	//						client->buffer->_buffer.resize(newBufferSize);

	//						// Serialize
	//						client->buffer->writeIntBE(packet.header.packetLength);
	//						client->buffer->writeIntBE(packet.header.msgID);
	//						client->buffer->writeIntBE(packet.msgLength);
	//						client->buffer->writeString(packet.msg);

	//						// SENDING.............................................................. 
	//						client->dataBuf.buf = client->buffer->_buffer.data();
	//						client->dataBuf.len = client->buffer->_buffer.size();

	//						// Look through all room names that this client is a part of
	//						for (std::map<std::string, std::vector<ClientInfo*>>::iterator iter = clientsInRooms.begin(); iter != clientsInRooms.end(); ++iter)
	//						{
	//							std::string roomNameKey = iter->first;

	//							// Does the client exist in the current room?
	//							std::vector<ClientInfo*>::iterator it;
	//							it = std::find(iter->second.begin(), iter->second.end(), client);
	//							if (it != iter->second.end())
	//							{
	//								// Found client in room. Send message to all clients in that room
	//								for (int i = 0; i < clientsInRooms[roomNameKey].size(); i++)
	//								{
	//									result = WSASend( //sending data right back to client
	//										clientsInRooms[roomNameKey].at(i)->socket,
	//										&(client->dataBuf),
	//										1,
	//										&SentBytes,
	//										Flags,
	//										NULL,
	//										NULL
	//									);
	//									if (SentBytes == SOCKET_ERROR)
	//										printf("send error %d\n", WSAGetLastError());
	//									else if (SentBytes == 0)
	//										printf("Send result is 0\n");
	//									else
	//										printf("Successfully sent %d bytes!\n", SentBytes);
	//								}
	//							}
	//						}

	//						break;
	//					}
	//					//***************************************  MESSAGE ID: LEAVE ROOM  ************************************************
	//					case LeaveRoom:
	//					{
	//						// Read in the rest of the packet, store room name
	//						packet.roomLength = client->buffer->readIntBE();
	//						packet.roomname = client->buffer->readString(packet.roomLength);

	//						// Preparing msg back to server
	//						packet.msg = "SERVER: [" + client->username + "] has left room [" + packet.roomname + "]";
	//						packet.msgLength = packet.msg.length();
	//						packetLength = 4 + 4 + 4 + packet.msgLength;
	//						packet.header.packetLength = packetLength;
	//						packet.header.msgID = LeaveRoom;

	//						// Clear the buffer
	//						client->buffer->_buffer.clear();
	//						client->buffer->readIndex = 0;
	//						client->buffer->writeIndex = 0;

	//						// Growing the buffer just enough to deal with packet
	//						int newBufferSize = packet.header.packetLength;
	//						client->buffer->_buffer.resize(newBufferSize);

	//						// Serialize
	//						client->buffer->writeIntBE(packet.header.packetLength);
	//						client->buffer->writeIntBE(packet.header.msgID);
	//						client->buffer->writeIntBE(packet.msgLength);
	//						client->buffer->writeString(packet.msg);

	//						// SENDING.............................................................. 
	//						client->dataBuf.buf = client->buffer->_buffer.data();
	//						client->dataBuf.len = client->buffer->_buffer.size();

	//						// Sending everyone in the room that a user has left that room
	//						for (int i = 0; i < clientsInRooms[packet.roomname].size(); i++)
	//						{
	//							result = WSASend( //sending data right back to client
	//								clientsInRooms[packet.roomname].at(i)->socket,
	//								&(client->dataBuf),
	//								1,
	//								&SentBytes,
	//								Flags,
	//								NULL,
	//								NULL
	//							);
	//							if (SentBytes == SOCKET_ERROR)
	//								printf("send error %d\n", WSAGetLastError());
	//							else if (SentBytes == 0)
	//								printf("Send result is 0\n");
	//							else
	//								printf("Successfully sent %d bytes!\n", SentBytes);
	//						}

	//						// Remove client from that room
	//						clientsInRooms[packet.roomname].erase(remove(clientsInRooms[packet.roomname].begin(), clientsInRooms[packet.roomname].end(), client), clientsInRooms[packet.roomname].end());

	//						break;
	//					}

	//					default:
	//						break;
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

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
	//delete buffer;

	return 0;
}