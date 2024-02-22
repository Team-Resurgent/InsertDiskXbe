#pragma once

#include "xboxinternals.h"
#include "pointerVector.h"

#include <string>

#define StringNotFouund 0xffffffff

class stringUtility
{
public:
	static char* formatString(const char* format, ...);
	static char* lowerCase(const char* value);
	static char* upperCase(const char* value);
	static bool startsWith(const char* value, const char* startsWith, bool caseInsensitive);
	static bool endsWith(const char* value, const char* endsWith, bool caseInsensitive);
	static bool equals(const char* value1, const char* value2, bool caseInsensitive);
	static char* replace(const char* value, const char* search, const char* with);
	static char* leftTrim(const char* value, const char trimChar);
	static char* rightTrim(const char* value, const char trimChar);
	static char* trim(const char* value, const char trimChar);
	static char* substr(const char* value, uint32_t offset, int32_t length);
	static int32_t find(const char* value, uint32_t length, uint32_t startPos, const char* search, bool caseInsensitive);
	static void copyString(char* dest, char *source, uint32_t maxLength);
	static int toInt(const char* value);
	static int hexCharToInt(char c);
	static char* formatSize(uint32_t size);
	static char* formatIp(uint32_t ip);
};