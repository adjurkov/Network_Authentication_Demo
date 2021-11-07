#include "cBuffer.h"

cBuffer::cBuffer()
{
	writeIndex = 0;
	readIndex = 0;
	_buffer.resize(500);
}

cBuffer::cBuffer(size_t size)
{
	_buffer.resize(size);
	writeIndex = 0;
	readIndex = 0;
}

cBuffer::cBuffer(std::vector<char> buffer)
{
	_buffer = buffer;
	writeIndex = 0;
	readIndex = 0;
}

void cBuffer::writeShortBE(std::size_t index, int16_t value)
{
	// Growing the buffer when serializing past writeindex
	if (_buffer.size() <= writeIndex)
	{
		int newSize = writeIndex + 2;
		_buffer.resize(newSize);
	}

	_buffer[index + 1] = value;
	_buffer[index] = value >> 8;
	writeIndex += 2;;
}

void cBuffer::writeShortBE(int16_t value)
{
	writeShortBE(writeIndex, value);
}

int16_t cBuffer::readShortBE(std::size_t index)
{
	int16_t value = _buffer[index + 1];
	value |= _buffer[index] << 8;
	readIndex += 2;

	return value;
}

int16_t cBuffer::readShortBE()
{
	return readShortBE(readIndex);
}

void cBuffer::writeIntBE(std::size_t index, int32_t value)
{
	// Growing the buffer when serializing past writeindex
	if (_buffer.size() <= writeIndex)
	{
		int newSize = writeIndex + 4;
		_buffer.resize(newSize);
	}

	_buffer[index + 3] = value;
	_buffer[index + 2] = value >> 8;
	_buffer[index + 1] = value >> 16;
	_buffer[index] = value >> 24;
	writeIndex += 4;
}

void cBuffer::writeIntBE(int32_t value)
{
	writeIntBE(writeIndex, value);
}

uint32_t cBuffer::readIntBE(std::size_t index)
{
	uint32_t value = _buffer[index + 3];
	value |= _buffer[index + 2] << 8;
	value |= _buffer[index + 1] << 16;
	value |= _buffer[index] << 24;
	readIndex += 4;

	return value;
}

uint32_t cBuffer::readIntBE()
{
	return readIntBE(readIndex);
}

void cBuffer::writeString(std::size_t index, std::string value)
{
	// Growing the buffer when serializing past writeindex
	if (_buffer.size() <= writeIndex)
	{
		int newSize = writeIndex + value.length();
		_buffer.resize(newSize);
	}

	for (int i = 0; i < value.length(); i++)
	{
		_buffer[writeIndex] = value[i];
		writeIndex++;
	}
}

void cBuffer::writeString(std::string value)
{
	writeString(writeIndex, value);
}

std::string cBuffer::readString(std::size_t lengthOfString)
{
	std::vector<uint8_t> value;
	std::string valueString;

	for (int i = 0; i < lengthOfString; i++)
	{
		value.push_back(_buffer[readIndex]);
		readIndex++;
	}
	for (char c : value)
	{
		valueString += c;
	}
	return valueString;
}

std::string cBuffer::readString()
{
	return readString(readIndex);
}

