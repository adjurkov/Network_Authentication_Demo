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

	AcceptedUsername = 5,
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

	// Default Constructor
	sPacket();

	// Serialize a packet given one of the user commands (/name, /join, /leave, or plain message)
	void SerializeUserCommand(sPacket& packet, std::vector<char> &userMessage, cBuffer& buffer);
};


//class DLLExport cProtocolManager
//{
//	
//
//	struct sHeader
//	{
//		int packet_length;
//		int message_id;
//	};
//
//	sHeader header;
//	int username_length;
//	std::string username;
//	int room_length;
//	std::string room_name;
//	int user_message_length;
//	std::string user_message;
//
//
//	//struct sJoinRoomMessage
//	//{
//	//	sHeader header;
//	//	int room_length;
//	//	std::string room_name;
//	//};
//
//	//struct sLeaveRoomMessage
//	//{
//	//	sHeader header;
//	//	int room_length;
//	//	std::string room_name;
//	//};
//
//	//struct sSendMessage
//	//{
//	//	sHeader header;
//	//	int room_length;
//	//	std::string room_name;
//	//	int user_message_length;
//	//	std::string user_message;
//	//};
//
//	//struct sReceiveMessage
//	//{
//	//	sHeader header;
//	//	int username_length;
//	//	std::string username;
//	//	int user_message_length;
//	//	std::string user_message;
//	//	int room_length;
//	//	std::string room_name;
//	//};
//};
//



