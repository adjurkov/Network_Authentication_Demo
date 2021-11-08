#define WIN32_LEAN_AND_MEAN		

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <map>
#include <iostream>
#include "cBuffer.h"
#include "ProtocolManager.h"

#include <gen/authentication.pb.h>

#pragma comment (lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 16
#define DEFAULT_PORT "5150"

//----------------------------------------   Client Structure   ------------------------------------------------
struct ClientInfo 
{
	SOCKET socket;
	std::string username;

	WSABUF dataBuf;
	cBuffer* buffer;
	int bytesRECV;
};
//----------------------------------------   Global Variables   ------------------------------------------------

// Managing client information, and which room they are in
int TotalClients = 0;
std::vector<ClientInfo*> clientSockets;
std::map<std::string, std::vector<ClientInfo*>> clientsInRooms; // key: roomname, value: clientInfo

// Packet Information
sPacket packet;
int packetLength;
int msgID;
int roomLength;
std::string roomname;
int userMsgLength;
std::string userMsgName;
int usernameLength;
std::string username;

int result;
SOCKET listenSocket = INVALID_SOCKET;
SOCKET acceptSocket = INVALID_SOCKET;
FD_SET ReadSet;
int total;
DWORD flags;
DWORD RecvBytes;
DWORD SentBytes;
DWORD NonBlock = 1;

//--------------------------------------------------------------------------------------------------------------

void RemoveClient(int index)
{
	// Look through all room names that this client is a part of
	for (std::map<std::string, std::vector<ClientInfo*>>::iterator iter = clientsInRooms.begin(); iter != clientsInRooms.end(); ++iter)
	{
		std::string roomNameKey = iter->first;

		// Does the client exist in the current room?
		std::vector<ClientInfo*>::iterator it;
		it = std::find(iter->second.begin(), iter->second.end(), clientSockets.at(index));
		if (it != iter->second.end())
		{
			// Found client in room. Remove them from room
			iter->second.erase(std::remove(iter->second.begin(), iter->second.end(), clientSockets.at(index)), iter->second.end());
		}
	}

	closesocket(clientSockets.at(index)->socket);

	printf("Closing socket %d\n", (int)clientSockets.at(index)->socket);

	clientSockets.erase(clientSockets.begin() + index);

	TotalClients--;
}

int init()
{
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
	listenSocket = socket(
		addrResult->ai_family,
		addrResult->ai_socktype,
		addrResult->ai_protocol
	);
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

	
	result = ioctlsocket(listenSocket, FIONBIO, &NonBlock);
	if (result == SOCKET_ERROR)
	{
		printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	printf("ioctlsocket() was successful!\n");


}

int main(int argc, char** argv)
{
	// Initialize winsock, create sockets, bind, listen
	init();

	//----------------------------------------   Network Loop   ------------------------------------------------
	printf("Entering accept/recv/send loop...\n");
	while (true)
	{
		timeval tv = { 0 };
		tv.tv_sec = 2;
		// Initialize our read set
		FD_ZERO(&ReadSet); // everyloop we initialize to zero

		// Always look for connection attempts
		// we need to read from this socket every loop
		FD_SET(listenSocket, &ReadSet);

		// Adding all the client sockets to the readset for each room
		for (std::vector<ClientInfo*>::iterator iter = clientSockets.begin(); iter != clientSockets.end(); ++iter)
			FD_SET((*iter)->socket, &ReadSet);

		// Call our select function to find the sockets that require our attention
		total = select(0, &ReadSet, NULL, NULL, &tv);
		if (total == SOCKET_ERROR)
		{
			printf("select() failed with error: %d\n", WSAGetLastError());
			return 1;
		}

		// Check for arriving connections on the listening socket
		if (FD_ISSET(listenSocket, &ReadSet)) 
		{
			total--; // remove one from the total (total is the size of the readset)
			acceptSocket = accept(listenSocket, NULL, NULL);
			if (acceptSocket == INVALID_SOCKET)
			{
				printf("accept() failed with error %d\n", WSAGetLastError());
				return 1;
			}
			else
			{
				// Set to non-blocking
				result = ioctlsocket(acceptSocket, FIONBIO, &NonBlock); 
				if (result == SOCKET_ERROR)
				{
					printf("ioctsocket() failed with error %d\n", WSAGetLastError());
				}
				else
				{
					ClientInfo* info = new ClientInfo(); // add as client

					info->socket = acceptSocket; // storing new socket as accept socket
					info->bytesRECV = 0; // the write index
					info->buffer = new cBuffer;
					info->buffer->_buffer.resize(500);

					clientSockets.push_back(info);
					TotalClients++;
					printf("New client connected on socket %d\n", (int)acceptSocket);
				}
			}
		}

//----------------------------------------   Recv and Send   ------------------------------------------------
			for (int i = 0; i < clientSockets.size(); i++)
			{
				ClientInfo* client = clientSockets.at(i);

				// If the ReadSet is marked for this socket, then this means data is available to be read on the socket
				if (FD_ISSET(client->socket, &ReadSet))
				{
					// if it is set, we can read the data from that socket
					total--;

					client->dataBuf.buf = client->buffer->_buffer.data();
					client->dataBuf.len = client->buffer->_buffer.size();

					DWORD Flags = 0;
					result = WSARecv(
						client->socket,
						&(client->dataBuf),
						1,
						&RecvBytes,
						&Flags,
						NULL,
						NULL
					);
					// Result > 0 we receive data, if 0 connection lost
					if (result == SOCKET_ERROR)
					{
						if (WSAGetLastError() == WSAEWOULDBLOCK) {}
						else
						{
							printf("WSARecv failed on socket %d with error: %d\n", (int)client->socket, WSAGetLastError());
							RemoveClient(i);
						}
					}
					else // RecvBytes > 0, we got data
					{
						printf("WSARecv() is OK!\n");
						if (RecvBytes == 0)
						{
							RemoveClient(i);
						}
						else if (RecvBytes == SOCKET_ERROR)
						{
							printf("recv: There was an error..%d\n", WSAGetLastError());
							continue;
						}
						else
						{
							memcpy(client->buffer->_buffer.data(), client->dataBuf.buf, client->dataBuf.len);

							// Read in header
							packetLength = client->buffer->readIntBE();
							msgID = client->buffer->readIntBE();

							switch (msgID)
							{

			//***************************************  MESSAGE ID: SET NAME  ************************************************

							case SetName:
							{
								// Read in the rest of the packet, store username
								packet.usernameLength = client->buffer->readIntBE();
								packet.username = client->buffer->readString(packet.usernameLength);
								client->username = packet.username;

								// Preparing msg back to server
								packet.msg = "SERVER: Welcome, " + client->username + "!";
								packet.msgLength = packet.msg.length();
								packetLength = 4 + 4 + 4 + packet.msgLength;
								packet.header.packetLength = packetLength;
								packet.header.msgID = AcceptedUsername;

								// Clear the buffer
								client->buffer->_buffer.clear();
								client->buffer->readIndex = 0;
								client->buffer->writeIndex = 0;

								// Growing the buffer just enough to deal with packet
								int newBufferSize = packet.header.packetLength;
								client->buffer->_buffer.resize(newBufferSize);

								// Serialize
								client->buffer->writeIntBE(packet.header.packetLength);
								client->buffer->writeIntBE(packet.header.msgID);
								client->buffer->writeIntBE(packet.msgLength);
								client->buffer->writeString(packet.msg);

								// SENDING.............................................................. 

								client->dataBuf.buf = client->buffer->_buffer.data();
								client->dataBuf.len = client->buffer->_buffer.size();

								result = WSASend( //sending data right back to client
									client->socket,
									&(client->dataBuf),
									1,
									&SentBytes,
									Flags,
									NULL,
									NULL
								);
								if (SentBytes == SOCKET_ERROR)
									printf("send error %d\n", WSAGetLastError());
								else if (SentBytes == 0)
									printf("Send result is 0\n");
								else
									printf("Successfully sent %d bytes!\n", SentBytes);

								break;
							}

			//***************************************  MESSAGE ID: JOIN ROOM  ************************************************

							case JoinRoom:
							{
								// Read in the rest of the packet, store room name
								packet.roomLength = client->buffer->readIntBE();
								packet.roomname = client->buffer->readString(packet.roomLength);

								// New room
								if (clientsInRooms.count(packet.roomname) == 0)
								{
									clientsInRooms[packet.roomname] = {};
									clientsInRooms[packet.roomname].push_back(client);
								}
								// If room exists
								else
								{
									clientsInRooms[packet.roomname].push_back(client);
								}

								// Preparing msg back to server
								packet.msg = "SERVER: [" + client->username + "] has joined room [" + packet.roomname + "]";
								packet.msgLength = packet.msg.length();
								packetLength = 4 + 4 + 4 + packet.msgLength;
								packet.header.packetLength = packetLength;
								packet.header.msgID = JoinRoom;

								// Clear the buffer
								client->buffer->_buffer.clear();
								client->buffer->readIndex = 0;
								client->buffer->writeIndex = 0;

								// Growing the buffer just enough to deal with packet
								int newBufferSize = packet.header.packetLength;
								client->buffer->_buffer.resize(newBufferSize);

								// Serialize
								client->buffer->writeIntBE(packet.header.packetLength);
								client->buffer->writeIntBE(packet.header.msgID);
								client->buffer->writeIntBE(packet.msgLength);
								client->buffer->writeString(packet.msg);

								// SENDING.............................................................. 
								client->dataBuf.buf = client->buffer->_buffer.data();
								client->dataBuf.len = client->buffer->_buffer.size();

								// Sending everyone in the room that a user has joined that room
								for (int i = 0; i < clientsInRooms[packet.roomname].size(); i++)
								{
									result = WSASend( //sending data right back to client
										clientsInRooms[packet.roomname].at(i)->socket,
										&(client->dataBuf),
										1,
										&SentBytes,
										Flags,
										NULL,
										NULL
									);
									if (SentBytes == SOCKET_ERROR)
										printf("send error %d\n", WSAGetLastError());
									else if (SentBytes == 0)
										printf("Send result is 0\n");
									else
										printf("Successfully sent %d bytes!\n", SentBytes);
								}
								break;
							}
		//***************************************  MESSAGE ID: BROADCAST  ************************************************

							case Broadcast:
							{
								// Read in the rest of the packet, store user message
								packet.msgLength = client->buffer->readIntBE();
								packet.msg = client->buffer->readString(packet.msgLength);

								// Preparing msg back to server
								packet.msg = "SERVER: [" + client->username + "] says: " + packet.msg;
								packet.msgLength = packet.msg.length();
								packetLength = 4 + 4 + 4 + packet.msgLength;
								packet.header.packetLength = packetLength;
								packet.header.msgID = Broadcast;

								// Clear the buffer
								client->buffer->_buffer.clear();
								client->buffer->readIndex = 0;
								client->buffer->writeIndex = 0;

								// Growing the buffer just enough to deal with packet
								int newBufferSize = packet.header.packetLength;
								client->buffer->_buffer.resize(newBufferSize);

								// Serialize
								client->buffer->writeIntBE(packet.header.packetLength);
								client->buffer->writeIntBE(packet.header.msgID);
								client->buffer->writeIntBE(packet.msgLength);
								client->buffer->writeString(packet.msg);

								// SENDING.............................................................. 
								client->dataBuf.buf = client->buffer->_buffer.data();
								client->dataBuf.len = client->buffer->_buffer.size();

								// Look through all room names that this client is a part of
								for (std::map<std::string, std::vector<ClientInfo*>>::iterator iter = clientsInRooms.begin(); iter != clientsInRooms.end(); ++iter)
								{
									std::string roomNameKey = iter->first;

									// Does the client exist in the current room?
									std::vector<ClientInfo*>::iterator it;
									it = std::find(iter->second.begin(), iter->second.end(), client);
									if (it != iter->second.end())
									{
										// Found client in room. Send message to all clients in that room
										for (int i = 0; i < clientsInRooms[roomNameKey].size(); i++)
										{
											result = WSASend( //sending data right back to client
												clientsInRooms[roomNameKey].at(i)->socket,
												&(client->dataBuf),
												1,
												&SentBytes,
												Flags,
												NULL,
												NULL
											);
											if (SentBytes == SOCKET_ERROR)
												printf("send error %d\n", WSAGetLastError());
											else if (SentBytes == 0)
												printf("Send result is 0\n");
											else
												printf("Successfully sent %d bytes!\n", SentBytes);
										}
									}
								}
								
								break;
							}
			//***************************************  MESSAGE ID: LEAVE ROOM  ************************************************
							case LeaveRoom:
							{
								// Read in the rest of the packet, store room name
								packet.roomLength = client->buffer->readIntBE();
								packet.roomname = client->buffer->readString(packet.roomLength);

								// Preparing msg back to server
								packet.msg = "SERVER: [" + client->username + "] has left room [" + packet.roomname + "]";
								packet.msgLength = packet.msg.length();
								packetLength = 4 + 4 + 4 + packet.msgLength;
								packet.header.packetLength = packetLength;
								packet.header.msgID = LeaveRoom;

								// Clear the buffer
								client->buffer->_buffer.clear();
								client->buffer->readIndex = 0;
								client->buffer->writeIndex = 0;

								// Growing the buffer just enough to deal with packet
								int newBufferSize = packet.header.packetLength;
								client->buffer->_buffer.resize(newBufferSize);

								// Serialize
								client->buffer->writeIntBE(packet.header.packetLength);
								client->buffer->writeIntBE(packet.header.msgID);
								client->buffer->writeIntBE(packet.msgLength);
								client->buffer->writeString(packet.msg);

								// SENDING.............................................................. 
								client->dataBuf.buf = client->buffer->_buffer.data();
								client->dataBuf.len = client->buffer->_buffer.size();

								// Sending everyone in the room that a user has left that room
								for (int i = 0; i < clientsInRooms[packet.roomname].size(); i++)
								{
									result = WSASend( //sending data right back to client
										clientsInRooms[packet.roomname].at(i)->socket,
										&(client->dataBuf),
										1,
										&SentBytes,
										Flags,
										NULL,
										NULL
									);
									if (SentBytes == SOCKET_ERROR)
										printf("send error %d\n", WSAGetLastError());
									else if (SentBytes == 0)
										printf("Send result is 0\n");
									else
										printf("Successfully sent %d bytes!\n", SentBytes);
								}

								// Remove client from that room
								clientsInRooms[packet.roomname].erase(remove(clientsInRooms[packet.roomname].begin(), clientsInRooms[packet.roomname].end(), client), clientsInRooms[packet.roomname].end());

								break;
							}

							default:
								break;
							}
						}
					}
				}
			}
	}

	// Close
	result = shutdown(acceptSocket, SD_SEND);
	if (result == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(acceptSocket);
		WSACleanup();
		return 1;
	}

	// Cleanup
	closesocket(acceptSocket);
	WSACleanup();

	return 0;
}