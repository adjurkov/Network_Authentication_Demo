#include "ProtocolManager.h"

// Default Constructor
sPacket::sPacket()
{
	header.msgID = 4;
	header.packetLength = 0;
	roomLength = 0;
	roomname = " ";
	usernameLength = 0;
	username = " ";
	msgLength = 0;
	msg = " ";
}

// Serialize a packet given one of the user commands (/name, /join, /leave, or plain message)
void sPacket::SerializeUserCommand(sPacket& packet, std::vector<char> &userMessage, cBuffer& buffer)
{
	int messageStartIndex = 0;
	std::string messageType;

	// User enters a command
	if (userMessage.at(0) == '/')
	{
		// Parsing usermessage to get command name
		for (int i = 0; i < userMessage.size(); i++)
		{
			// Read until we hit whitespace
			while (userMessage.at(i) != ' ')
			{
				// Storing command type (/name, /join, /leave)
				messageType += userMessage.at(i);
				messageStartIndex++;
				i++;
			}

			if (messageType == "/name")
			{
				// Get username
				std::string msg;
				for (int i = messageStartIndex + 1; i < userMessage.size(); i++)
					msg += userMessage.at(i);

				packet.header.packetLength = 4 + 4 + 4 + msg.length(); // 4-int, 4-enum, 4-int, msg length
				packet.header.msgID = SetName;
				packet.usernameLength = msg.length();
				packet.username = msg;

				// Serialize
				buffer.writeIntBE(packet.header.packetLength);
				buffer.writeIntBE(packet.header.msgID);
				buffer.writeIntBE(packet.usernameLength);
				buffer.writeString(packet.username);
				break;
			}

			else if (messageType == "/join")
			{
				// Get room name
				std::string msg;
				for (int i = messageStartIndex + 1; i < userMessage.size(); i++)
					msg += userMessage.at(i);

				packet.header.packetLength = 4 + 4 + 4 + msg.length(); // 4-int, 4-enum, 4-int, msg length
				packet.header.msgID = JoinRoom;
				packet.roomLength = msg.length();
				packet.roomname = msg;

				// Serialize
				buffer.writeIntBE(packet.header.packetLength);
				buffer.writeIntBE(packet.header.msgID);
				buffer.writeIntBE(packet.roomLength);
				buffer.writeString(packet.roomname);
				break;
			}

			else if (messageType == "/leave")
			{
				// Get room name
				std::string msg;
				for (int i = messageStartIndex + 1; i < userMessage.size(); i++)
					msg += userMessage.at(i);

				packet.header.packetLength = 4 + 4 + 4 + msg.length(); // 4-int, 4-enum, 4-int, msg length
				packet.header.msgID = LeaveRoom;
				packet.roomLength = msg.length();
				packet.roomname = msg;

				// Serialize
				buffer.writeIntBE(packet.header.packetLength);
				buffer.writeIntBE(packet.header.msgID);
				buffer.writeIntBE(packet.roomLength);
				buffer.writeString(packet.roomname);
				break;
			}

			else
			{
				printf("Command not recognized.");
				break;
			}
		}
	}

	// User broadcasts message
	else
	{
		// Get message
		std::string msg;
		for (int i = 0; i < userMessage.size(); i++)
			msg += userMessage.at(i);

		packet.header.packetLength = 4 + 4 + 4 + msg.length();
		packet.header.msgID = Broadcast;
		packet.msg = msg;
		packet.msgLength = msg.length();

		// Serialize
		buffer.writeIntBE(packet.header.packetLength);
		buffer.writeIntBE(packet.header.msgID);
		buffer.writeIntBE(packet.msgLength);
		buffer.writeString(packet.msg);
	}
	userMessage.clear();
}