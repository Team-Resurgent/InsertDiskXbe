#include "stringUtility.h"
#include "xboxinternals.h"

#include <string.h>
#include <algorithm>
#include <sstream>
#include <cstdarg>


// Tested
char* stringUtility::formatString(const char* format, ...)
{
    va_list args;
    va_start(args, format);

	uint32_t length = _vsnprintf(NULL, 0, format, args);

	char* result = (char*)malloc(length + 1);
	_vsnprintf(result, length, format, args);
	result[length] = 0;

    va_end(args);
    return result;
}

// Tested
char* stringUtility::lowerCase(const char* value)
{
	char* result = strdup(value);
    for (uint32_t i = 0; i < strlen(result); i++) 
	{
        result[i] = tolower(result[i]);
    }
	return result;
}

// Tested
char* stringUtility::upperCase(const char* value)
{
	char* result = strdup(value);
    for (uint32_t i = 0; i < strlen(result); i++) 
	{
        result[i] = toupper(result[i]);
    }
	return result;
}

bool stringUtility::startsWith(const char* value, const char* startsWith, bool caseInsensitive)
{
	if (caseInsensitive) 
	{
		return strnicmp(startsWith, value, strlen(startsWith)) == 0;
	}
	return strncmp(startsWith, value, strlen(startsWith)) == 0;
}

bool stringUtility::endsWith(const char* value, const char* endsWith, bool caseInsensitive)
{
    uint32_t valueLength = strlen(value);
    uint32_t endsWithLength = strlen(endsWith);
    if (endsWithLength > valueLength)
	{
        return 0;
	}
	if (caseInsensitive) 
	{
		return strnicmp(value + valueLength - endsWithLength, endsWith, endsWithLength) == 0;
	}
	return strncmp(value + valueLength - endsWithLength, endsWith, endsWithLength) == 0;
}

bool stringUtility::equals(const char* value1, const char* value2, bool caseInsensitive)
{
	uint32_t valueLength1 = strlen(value1);
	uint32_t valueLength2 = strlen(value2);
	if (valueLength1 != valueLength2)
	{
		return false;
	}
	if (caseInsensitive) 
	{
		return strnicmp(value1, value2, valueLength1) == 0;
	}
	return strncmp(value1, value2, valueLength1) == 0;
}

// Tested
char* stringUtility::replace(const char* value, const char* search, const char* with)
{
	char* valueCopy = strdup(value);
	while (true)
	{
		char* searchResult = strstr(valueCopy, search);
		if (searchResult == NULL)
		{
			return valueCopy;
		}

		char*  result = (char*)malloc(strlen(valueCopy) - strlen(search) + sizeof(with) + 1);
		if (result == NULL)
		{
			free(valueCopy);
			return NULL;
		}

		strncpy(result, valueCopy, searchResult - valueCopy);
		sprintf(result + (searchResult - valueCopy), "%s%s", with, searchResult + strlen(search));
		
		free(valueCopy);
		valueCopy = result;
	}
}

char* stringUtility::leftTrim(const char* value, const char trimChar)
{
    uint32_t offset = 0;
    while(value[offset] == trimChar && value[offset] != 0) 
    {
        offset++;
    }

    uint32_t length = strlen(value) - offset;

    char* result = (char*)malloc(length + 1);
    if (result == NULL)
    {
        return NULL;
    }

    strncpy(result, value + offset, length);
    result[length] = 0;
    return result;
}

char* stringUtility::rightTrim(const char* value, const char trimChar)
{
    if (value == NULL) return NULL;

    int32_t length = strlen(value);

    while(length > 0 && value[length - 1] == trimChar)
    {
        length--;
    }

    char* result = (char*)malloc(length + 1);
    if (result == NULL)
    {
        return NULL;
    }

    strncpy(result, value, length);
    result[length] = 0;
    return result;
}

char* stringUtility::trim(const char* value, const char trimChar)
{
	char* leftTrimmed = leftTrim(value, trimChar);
	char* trimmed = rightTrim(leftTrimmed, trimChar);
	free(leftTrimmed);
	return trimmed;
}

char* stringUtility::substr(const char* value, uint32_t offset, int32_t length)
{
	if (offset >= strlen(value))
	{
		return strdup("");
	}

	if (length < 0)
	{
		length = strlen(value) - offset;
	}

	char* result = (char*)malloc(length + 1);
	if (result == NULL)
	{
		return NULL;
	}

	strncpy(result, value + offset, length);
	result[length] = 0;
	return result;
}

int32_t stringUtility::find(const char* value, uint32_t length, uint32_t startPos, const char* search, bool caseInsensitive)
{
	uint32_t position = -1;
	for (uint32_t i = startPos; i < length - strlen(search); i++) 
	{
        bool match = true;
        for (uint32_t j = 0; j < strlen(search); ++j) 
		{
            if ((caseInsensitive == false && value[i + j] != search[j]) || 
				(caseInsensitive == true && tolower(value[i + j]) != tolower(search[j]))) 
			{
                match = false;
                break;
            }
        }
        if (match) 
		{
			position = i;
            break;
        }
    }
	return position;
}

void stringUtility::copyString(char* dest, char *source, uint32_t maxLength)
{
	uint32_t copyLength = min(strlen(source), maxLength - 1);
	for (uint32_t i = 0; i < copyLength; i++)
	{
		dest[i] = source[i];
	}
	dest[copyLength] = 0;
}

int stringUtility::toInt(const char* value)
{
   int sign = 1;
   if (value[0] == '-') 
   {
        sign = -1;
    }

	int result = 0;
	for (size_t i = (sign == 1 ? 0 : 1); i < strlen(value); i++)
	{
		result = result * 10;
		if (value[i] < 48 || value[i] > 57)
		{
			return 0;
		}
		result += value[i] - 48;
	}
	return result * sign;
}

int stringUtility::hexCharToInt(char c) 
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else {
        return 0;
    }
}

char* stringUtility::formatSize(uint32_t size)
{
    const uint32_t KB = 1024;
    const uint32_t MB = KB * KB;
    if (size < KB) {
		return formatString("%luB", size);
    } else if (size < MB) {
		return formatString("%luKB", size / KB);
    } else {
        return formatString("%luMB", size / MB);
    }
}

char* stringUtility::formatIp(uint32_t ip)
 {
	 return formatString("%i.%i.%i.%i", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff);
 }