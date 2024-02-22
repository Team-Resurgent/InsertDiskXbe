#include "drawing.h"
#include "context.h"
#include "math.h"
#include "utils.h"
#include "meshUtility.h"
#include "stringUtility.h"
#include "pointerMap.h"

#include <xgraphics.h>

#define SSFN_IMPLEMENTATION
#define SFFN_MAXLINES 8192
#define SSFN_memcmp memcmp
#define SSFN_memset memset
#define SSFN_realloc realloc
#define SSFN_free free
#include "ssfn.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace
{
	ssfn_t* mFontContext = NULL;
}

inline unsigned char lerp(unsigned  char a, unsigned char b, float t)
{
	return (unsigned char)(a + t * (b - a));
}

inline int length(int a, int b)
{
	return (a * a) + (b * b);
}

void drawing::swizzle(const void *src, const uint32_t& depth, const uint32_t& width, const uint32_t& height, void *dest)
{
  for (UINT y = 0; y < height; y++)
  {
    UINT sy = 0;
    if (y < width)
    {
      for (int bit = 0; bit < 16; bit++)
        sy |= ((y >> bit) & 1) << (2*bit);
      sy <<= 1; // y counts twice
    }
    else
    {
      UINT y_mask = y % width;
      for (int bit = 0; bit < 16; bit++)
        sy |= ((y_mask >> bit) & 1) << (2*bit);
      sy <<= 1; // y counts twice
      sy += (y / width) * width * width;
    }
    BYTE *s = (BYTE *)src + y * width * depth;
    for (UINT x = 0; x < width; x++)
    {
      UINT sx = 0;
      if (x < height * 2)
      {
        for (int bit = 0; bit < 16; bit++)
          sx |= ((x >> bit) & 1) << (2*bit);
      }
      else
      {
        int x_mask = x % (2*height);
        for (int bit = 0; bit < 16; bit++)
          sx |= ((x_mask >> bit) & 1) << (2*bit);
        sx += (x / (2 * height)) * 2 * height * height;
      }
      BYTE *d = (BYTE *)dest + (sx + sy)*depth;
      for (unsigned int i = 0; i < depth; ++i)
        *d++ = *s++;
    }
  }
}

image* drawing::createImage(uint8_t* imageData, D3DFORMAT format, int width, int height)
{
	image* imageToAdd = new image();
	imageToAdd->width = width;
	imageToAdd->height = height;

	if (FAILED(D3DXCreateTexture(context::getD3dDevice(), imageToAdd->width, imageToAdd->height, 1, 0, format, D3DPOOL_DEFAULT, &imageToAdd->texture)))
	{
		return false;
	}

	D3DSURFACE_DESC surfaceDesc;
	imageToAdd->texture->GetLevelDesc(0, &surfaceDesc);

	imageToAdd->uvRect = math::rectF(0, 0, imageToAdd->width / (float)surfaceDesc.Width, imageToAdd->height / (float)surfaceDesc.Height);

	D3DLOCKED_RECT lockedRect;
	if (SUCCEEDED(imageToAdd->texture->LockRect(0, &lockedRect, NULL, 0)))
	{
		uint8_t* tempBuffer = (uint8_t*)malloc(surfaceDesc.Size);
		memset(tempBuffer, 0, surfaceDesc.Size);
		uint8_t* src = imageData;
		uint8_t* dst = tempBuffer;
		for (int32_t y = 0; y < imageToAdd->height; y++)
		{
			memcpy(dst, src, imageToAdd->width * 4);
			src += imageToAdd->width * 4;
			dst += surfaceDesc.Width * 4;
		}
		swizzle(tempBuffer, 4, surfaceDesc.Width, surfaceDesc.Height, lockedRect.pBits);
		free(tempBuffer);
		imageToAdd->texture->UnlockRect(0);
	}

	return imageToAdd;
}

void drawing::addImage(const char* key, uint8_t* imageData, D3DFORMAT format, int width, int height)
{
	image* imageToAdd = createImage(imageData, format, width, height);
	context::getImageMap()->add(key, imageToAdd);
}

uint64_t drawing::getImageMemUse(const char* key)
{
	image* imageInfo = (image*)context::getImageMap()->get(key);
	if (imageInfo == NULL)
	{
		return 0;
	}
	return utils::roundUpToNextPowerOf2(imageInfo->width) * utils::roundUpToNextPowerOf2(imageInfo->height) * 4;
}

void drawing::removeImage(const char* key)
{
	image* imageToRemove = (image*)context::getImageMap()->get(key);
	if (imageToRemove != NULL)
	{
		imageToRemove->texture->Release();
		context::getImageMap()->removeKey(key);
	}
}

bool drawing::loadImage(const char* buffer, uint32_t length, const char* key)
{
	int width;
	int height;
	uint8_t* imageData = (uint8_t*)stbi_load_from_memory((const stbi_uc*)buffer, length, &width, &height, NULL, STBI_rgb_alpha);
	if (imageData == NULL)
	{
		return false;
	}
	addImage(key, imageData, D3DFMT_A8B8G8R8, width, height);
	free(imageData);
	return true;
}

bool drawing::loadFont(const uint8_t* data)
{
	if (mFontContext == NULL)
	{
		mFontContext = (ssfn_t*)malloc(sizeof(ssfn_t));                         
		memset(mFontContext, 0, sizeof(ssfn_t));
	}

	int result = ssfn_load(mFontContext, data);
	return result == 0;
}

void drawing::clearBackground()
{
	context::getD3dDevice()->Clear(0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0xff000000, 1.0f, 0L);
}

bool drawing::imageExists(const char* key)
{
	image* result = (image*)context::getImageMap()->get(key);
    return result != NULL;
}

image* drawing::getImage(const char* key)
{
	image* result = (image*)context::getImageMap()->get(key);
    return result;
}

void drawing::setTint(unsigned int color)
{
	context::getD3dDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, color);
}

void drawing::drawImage(image* image, uint32_t tint, int x, int y, int width, int height)
{
	if (image == NULL)
	{
		return;

	}
	context::getD3dDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, tint);
	float newY = (float)context::getBufferHeight() - (y + height);
	context::getD3dDevice()->SetTexture(0, image->texture);
	utils::dataContainer* vertices = meshUtility::createQuadXY(math::vec3F((float)x + 0.5f, newY + 0.5f, 0), math::sizeF((float)width, (float)height), image->uvRect);
	context::getD3dDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, (vertices->size / sizeof(meshUtility::vertex)) / 3, vertices->data, sizeof(meshUtility::vertex));
	delete(vertices);
}

void drawing::drawImage(image* image, uint32_t tint, int x, int y)
{
	if (image == NULL)
	{
		return;

	}
	drawImage(image, tint, x, y, image->width, image->height);
}

void drawing::drawImage(const char* imageKey, uint32_t tint, int x, int y, int width, int height)
{
	image* imageToDraw = getImage(imageKey);
	if (imageToDraw == NULL)
	{
		return;
	}
	drawImage(imageToDraw, tint, x, y, width, height);
}

void drawing::drawImage(const char* imageKey, uint32_t tint, int x, int y)
{
	image* imageToDraw = getImage(imageKey);
	if (imageToDraw == NULL)
	{
		return;
	}
	drawImage(imageToDraw, tint, x, y);
}

bitmapFont* drawing::generateBitmapFont(const char* fontName, int fontStyle, int fontSize, int lineHeight, int spacing, int textureDimension)
{
	bitmapFont* font = new bitmapFont();
	font->charMap = new pointerMap(true);

	ssfn_select(mFontContext, SSFN_FAMILY_ANY, fontName, fontStyle, fontSize);

	int textureWidth = textureDimension;
	int textureHeight = textureDimension; 

	uint32_t* imageData = (uint32_t*)malloc(textureWidth * textureHeight * 4);
	memset(imageData, 0, textureWidth * textureHeight * 4);  

	int x = 2;
	int y = 2;

	char* charsToEncode = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\xC2\xA1\xC2\xA2\xC2\xA3\xC2\xA4\xC2\xA5\xC2\xA6\xC2\xA7\xC2\xA8\xC2\xA9\xC2\xAA\xC2\xAB\xC2\xAC\xC2\xAD\xC2\xAE\xC2\xAF\xC2\xB0\xC2\xB1\xC2\xB2\xC2\xB3";
	char* currentCharPos = charsToEncode;
	while(*currentCharPos)
	{	
		char* nextCharPos = currentCharPos;
		uint32_t unicode = ssfn_utf8(&nextCharPos);

		int32_t length = nextCharPos - currentCharPos;
		char* currentChar = (char*)malloc(length + 1);
		memcpy(currentChar, currentCharPos, length);
		currentChar[length] = 0;

		currentCharPos = nextCharPos;

		Bounds bounds;
		int ret = ssfn_bbox(mFontContext, currentChar, &bounds.width, &bounds.height, &bounds.left, &bounds.top);
		if (ret != 0)
		{
			continue;
		}

		if ((x + bounds.width + 2) > textureWidth)
		{
			x = 2;
			y = y + bounds.height + 2;
		}

		math::rectI* rect = new math::rectI();
		rect->x = x;
		rect->y = y;
		rect->width = bounds.width;
		rect->height = bounds.height;

		char* unicodeString = stringUtility::formatString("%i", unicode);
		font->charMap->add(unicodeString, rect);
		free(unicodeString);

		ssfn_buf_t buffer; 
		memset(&buffer, 0, sizeof(buffer));
		buffer.ptr = (uint8_t*)imageData;       
		buffer.x = x + bounds.left;
		buffer.y = y + bounds.top;
		buffer.w = textureWidth;                        
		buffer.h = textureHeight;                     
		buffer.p = textureWidth * 4;                          
		buffer.bg = 0xffffffff;
		buffer.fg = 0xffffffff;   

		ssfn_render(mFontContext, &buffer, currentChar);

		x = x + bounds.width + 2;   
		free(currentChar);
	}

	font->image = createImage((uint8_t*)imageData, D3DFMT_A8R8G8B8, textureWidth, textureHeight);
	font->lineHeight = lineHeight;
	font->spacing = spacing;
	free(imageData);

	return font;
}

void drawing::measureBitmapString(bitmapFont* font, const char* message, int* width, int* height)
{
	image* image = font->image;

	int xPosMax = 0;

	int xPos = 0;
	int yPos = 0;

	char* currentCharPos = (char*)message;
	while(*currentCharPos)
	{	
		char* nextCharPos = currentCharPos;
		uint32_t unicode = ssfn_utf8(&nextCharPos);

		int32_t length = nextCharPos - currentCharPos;
		char* currentChar = (char*)malloc(length + 1);
		memcpy(currentChar, currentCharPos, length);
		currentChar[length] = 0;

		currentCharPos = nextCharPos;

		if (stringUtility::equals(currentChar, "\n", false) == true)
		{
			xPos = 0;
			yPos += font->lineHeight;
			continue;
		}

		char* unicodeString = stringUtility::formatString("%i", unicode);
		math::rectI* rect = (math::rectI*)font->charMap->get(unicodeString);
		free(unicodeString);
		if (rect == NULL)
		{
			continue;
		}

		xPos = xPos + rect->width + font->spacing;
		xPosMax = max(xPosMax, xPos);
		free(currentChar);
	}

	if (width != NULL)
	{
		*width = xPosMax - 2;
	}

	if (height != NULL)
	{
		*height = yPos + font->lineHeight;
	}
}

void drawing::drawBitmapString(bitmapFont* font, const char* message, uint32_t color, int x, int y)
{
	image* image = font->image;

	int xPos = x;
	int yPos = y;

	char* currentCharPos = (char*)message;
	while(*currentCharPos)
	{	
		char* nextCharPos = currentCharPos;
		uint32_t unicode = ssfn_utf8(&nextCharPos);

		int32_t length = nextCharPos - currentCharPos;
		char* currentChar = (char*)malloc(length + 1);
		memcpy(currentChar, currentCharPos, length);
		currentChar[length] = 0;

		currentCharPos = nextCharPos;

		if (stringUtility::equals(currentChar, "\n", false) == true)
		{
			xPos = x;
			yPos += font->lineHeight;
			continue;
		}

		char* unicodeString = stringUtility::formatString("%i", unicode);
		math::rectI* rect = (math::rectI*)font->charMap->get(unicodeString);
		free(unicodeString);
		if (rect == NULL)
		{
			continue;
		}

		math::rectF uvRect;
		uvRect.x = rect->x / (float)image->width;
		uvRect.y = rect->y / (float)image->height;
		uvRect.width = rect->width / (float)image->width;
		uvRect.height = rect->height / (float)image->height;

		context::getD3dDevice()->SetRenderState(D3DRS_TEXTUREFACTOR, color);
		float newY = (float)context::getBufferHeight() - (yPos + rect->height);
		context::getD3dDevice()->SetTexture(0, image->texture);
		utils::dataContainer* vertices = meshUtility::createQuadXY(math::vec3F((float)xPos + 0.5f, newY + 0.5f, 0), math::sizeF((float)rect->width, (float)rect->height), uvRect);
		context::getD3dDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, (vertices->size / sizeof(meshUtility::vertex)) / 3, vertices->data, sizeof(meshUtility::vertex));
		delete(vertices);

		xPos = xPos + rect->width + font->spacing;
		free(currentChar);
	}
}

void drawing::drawBitmapStringAligned(bitmapFont* font, const char*  message, uint32_t color, horizAlignment hAlign, int x, int y, int width)
{
	int textWidth = 0;
	measureBitmapString(font, message, &textWidth, NULL);

	int xPos = x;
	if (hAlign == horizAlignmentCenter)
	{
		xPos = x + ((width - textWidth) / 2);
	}
	else if (hAlign == horizAlignmentRight)
	{
		xPos = x + (width - textWidth);
	}

	drawBitmapString(font, message, color, xPos, y);
}