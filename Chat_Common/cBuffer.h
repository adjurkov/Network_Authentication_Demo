#pragma once

#include <vector>
#include <string>
#include <cstdlib>
#include <stdint.h>


// Compiled as a dynamic library
#define DLLExport __declspec ( dllexport )

class DLLExport cBuffer
{
public:

	std::vector<char> _buffer;
	//char* charBuffer;
	int writeIndex;
	int readIndex;

	cBuffer();
	cBuffer(size_t size);
	cBuffer(std::vector<char> buffer);
	

	void writeIntBE(std::size_t index, int32_t value);
	void writeIntBE(int32_t value);
	uint32_t readIntBE(std::size_t index);
	uint32_t readIntBE();

	void writeShortBE(std::size_t index, int16_t value);
	void writeShortBE(int16_t value);
	int16_t readShortBE(std::size_t index);
	int16_t readShortBE();

	void writeString(std::size_t index, std::string value);
	void writeString(std::string value);
	std::string readString(std::size_t lengthOfString);
	std::string readString();
};


