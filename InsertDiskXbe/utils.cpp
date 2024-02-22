#include "utils.h"
#include "xboxinternals.h"

#include <xtl.h>
#include <stdio.h>
#include <string>

void utils::debugPrint(const char* format, ...)
{
	va_list args;
    va_start(args, format);

	uint32_t length = _vsnprintf(NULL, 0, format, args);

	char* message = (char*)malloc(length + 1);
	_vsnprintf(message, length, format, args);
	message[length] = 0;

    va_end(args);

	OutputDebugStringA(message);
	free(message);
}

void* utils::mallocWithTerminator(uint32_t size)
{
	char* result = (char*)malloc(size + 1);
	result[size] = 0;
	return (void*)result;
}

void* utils::mallocCopyWithTerminator(void* source, uint32_t size, uint32_t copySize)
{
	void* result = mallocWithTerminator(size);
	memcpy(result, source, copySize);
	return result;
}

uint32_t utils::roundUpToNextPowerOf2(uint32_t value) 
{
	value--;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value++;
	return value;
}
