#pragma once

#include "xboxinternals.h"
#include <string>

class utils
{
public:

	typedef struct dataContainer 
	{

		char* data;
		uint32_t size;

		dataContainer() : data(NULL), size(0) {}

		dataContainer(uint32_t size)
		{
			this->data = (char*)utils::mallocWithTerminator(size);
			this->size = size; 
		}

		dataContainer(char* data, uint32_t size, uint32_t copySize)
		{
			this->data = (char*)utils::mallocCopyWithTerminator(data, size, copySize);
			this->size = size; 
		}

		~dataContainer()
		{
			free(this->data);
		}

	} dataContainer;

	static void debugPrint(const char* format, ...);
	static void* mallocWithTerminator(uint32_t size);
	static void* mallocCopyWithTerminator(void* source, uint32_t size, uint32_t copySize);
	static uint32_t roundUpToNextPowerOf2(uint32_t value);
};
