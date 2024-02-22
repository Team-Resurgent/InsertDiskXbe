#pragma once

#include "context.h"
#include "xboxinternals.h"
#include <string>
#include "math.h"
#include "pointerMap.h"
#include "alignment.h"

typedef struct Bounds
{
	int width;
	int height;
	int left;
	int top;
} Bounds;

class drawing
{
public:
	static void swizzle(const void *src, const uint32_t& depth, const uint32_t& width, const uint32_t& height, void *dest);
	static image* createImage(uint8_t* imageData, D3DFORMAT format, int width, int height);
	static void addImage(const char* key, uint8_t* imageData, D3DFORMAT format, int width, int height);
	static uint64_t getImageMemUse(const char* key);
	static void removeImage(const char* key);
	static bool loadImage(const char* buffer, uint32_t length, const char* key);
	static bool loadFont(const uint8_t* data);
	static void clearBackground();
	static bool imageExists(const char* key);
	static image* getImage(const char* key);
	static void setTint(unsigned int color);
	static void drawImage(image* image, uint32_t tint, int x, int y, int width, int height);
	static void drawImage(image* image, uint32_t tint, int x, int y);
	static void drawImage(const char* imageKey, uint32_t tint, int x, int y, int width, int height);
	static void drawImage(const char* imageKey, uint32_t tint, int x, int y);
	static bitmapFont* generateBitmapFont(const char* fontName, int fontStyle, int fontSize, int lineHeight, int spacing, int textureDimension);
	static void measureBitmapString(bitmapFont* font, const char* message, int* width, int* height);
	static void drawBitmapString(bitmapFont* font, const char* message, uint32_t color, int x, int y);
	static void drawBitmapStringAligned(bitmapFont* font, const char*  message, uint32_t color, horizAlignment hAlign, int x, int y, int width);
};
