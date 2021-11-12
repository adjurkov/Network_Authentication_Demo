#pragma once
#include <string>
#include <vector>
#include "cBuffer.h"

// Compiled as a dynamic library
#define DLLExport __declspec ( dllexport )


// Message ID Types 
enum
{
	JoinRoom = 1,
	LeaveRoom = 2,
	SetName = 3,
	Broadcast = 4,
	Register = 5,			// also tied to client socket as request id
	Authenticate = 6,		// also tied to client socket as request id
	AcceptedUsername = 7,
};

// Protocol Manager
struct DLLExport sPacket
{
	// Packet Header
	struct sHeader
	{
		int packetLength;
		int msgID;
	};

	// Packet Contents
	sHeader header;
	int roomLength;
	std::string roomname;
	int usernameLength;
	std::string username;
	int msgLength;
	std::string msg;
	int emailLength;
	std::string email;
	int passwordLength;
	std::string password;

	// Default Constructor
	sPacket();

	// Serialize a packet given one of the user commands (/name, /join, /leave, or plain message)
	void SerializeUserCommand(sPacket& packet, std::vector<char> &userMessage, cBuffer& buffer);
};



