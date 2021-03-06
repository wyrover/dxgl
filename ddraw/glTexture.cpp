// DXGL
// Copyright (C) 2012-2014 William Feely

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "common.h"
#include "BufferObject.h"
#include "glTexture.h"
#include "glUtil.h"
#include <math.h>

extern "C" {

// Use EXACTLY one line per entry.  Don't change layout of the list.
static const int START_TEXFORMATS = __LINE__;
const DDPIXELFORMAT texformats[] = 
{ // Size					Flags							FOURCC	bits	R/Ymask		G/U/Zmask	B/V/STmask	A/Zmask
	{sizeof(DDPIXELFORMAT),	DDPF_PALETTEINDEXED8,			0,		8,		0,			0,			0,			0},
	{sizeof(DDPIXELFORMAT),	DDPF_RGB,						0,		8,		0xE0,		0x1C,		0x3,		0},
	{sizeof(DDPIXELFORMAT),	DDPF_RGB,						0,		16,		0x7C00,		0x3E0,		0x1F,		0},
	{sizeof(DDPIXELFORMAT),	DDPF_RGB,						0,		16,		0xF800,		0x7E0,		0x1F,		0},
	{sizeof(DDPIXELFORMAT),	DDPF_RGB,						0,		24,		0xFF0000,	0xFF00,		0xFF,		0},
	{sizeof(DDPIXELFORMAT),	DDPF_RGB,						0,		32,		0xFF0000,	0xFF00,		0xFF,		0},
	{sizeof(DDPIXELFORMAT),	DDPF_RGB,						0,		32,		0xFF,		0xFF00,		0xFF0000,	0},
	{sizeof(DDPIXELFORMAT),	DDPF_RGB|DDPF_ALPHAPIXELS,		0,		16,		0xE0,		0x1C,		0x3,		0xFF00},
	{sizeof(DDPIXELFORMAT), DDPF_RGB|DDPF_ALPHAPIXELS,		0,		16,		0xF00,		0xF0,		0xF,		0xF000},
	{sizeof(DDPIXELFORMAT), DDPF_RGB|DDPF_ALPHAPIXELS,		0,		16,		0x7c00,		0x3E0,		0x1F,		0x8000},
	{sizeof(DDPIXELFORMAT), DDPF_RGB|DDPF_ALPHAPIXELS,		0,		32,		0xFF0000,	0xFF00,		0xFF,		0xFF000000},
	{sizeof(DDPIXELFORMAT), DDPF_LUMINANCE,					0,		8,		0xFF,		0,			0,			0},
	{sizeof(DDPIXELFORMAT),	DDPF_ALPHA,						0,		8,		0,			0,			0,			0},
	{sizeof(DDPIXELFORMAT),	DDPF_LUMINANCE|DDPF_ALPHAPIXELS,0,		16,		0xFF,		0,			0,			0xFF00},
	{sizeof(DDPIXELFORMAT), DDPF_ZBUFFER,					0,		16,		0,			0xFFFF,		0,			0},
	{sizeof(DDPIXELFORMAT),	DDPF_ZBUFFER,					0,		24,		0,			0xFFFFFF00,	0,			0},
	{sizeof(DDPIXELFORMAT),	DDPF_ZBUFFER,					0,		32,		0,			0xFFFFFF00,	0,			0},
	{sizeof(DDPIXELFORMAT),	DDPF_ZBUFFER,					0,		32,		0,			0xFFFFFFFF,	0,			0},
	{sizeof(DDPIXELFORMAT),	DDPF_ZBUFFER,					0,		32,		8,			0xFFFFFF00,	0xFF,		0},
	{sizeof(DDPIXELFORMAT),	DDPF_ZBUFFER,					0,		32,		8,			0xFF,		0xFFFFFF00,	0}
};
static const int END_TEXFORMATS = __LINE__ - 4;
int numtexformats;

void ClearError()
{
	do
	{
		if (glGetError() == GL_NO_ERROR) break;
	} while (1);
}

DWORD CalculateMipLevels(DWORD width, DWORD height)
{
	DWORD x, y;
	DWORD levels = 1;
	if ((!width) || (!height)) return 0;
	if ((width == 1) || (height == 1)) return 1;
	x = width;
	y = height;
miploop:
	x = max(1,(DWORD)floorf((float)x / 2.0f));
	y = max(1,(DWORD)floorf((float)y / 2.0f));
	levels++;
	if ((x == 1) || (y == 1)) return levels;
	else goto miploop;
}

void ShrinkMip(DWORD *x, DWORD *y)
{
	*x = max(1, (DWORD)floorf((float)*x / 2.0f));
	*y = max(1, (DWORD)floorf((float)*y / 2.0f));
}

TextureManager *TextureManager_Create(glExtensions *glext)
{
	TextureManager *newtex;
	numtexformats = END_TEXFORMATS - START_TEXFORMATS;
	newtex = (TextureManager*)malloc(sizeof(TextureManager));
	if (!newtex) return 0;
	ZeroMemory(newtex, sizeof(TextureManager));
	newtex->ext = glext;
	return newtex;
}

void TextureManager__CreateTexture(TextureManager *This, glTexture *texture, int width, int height, glUtil *util)
{
	TextureManager_CreateTextureClassic(This, texture, width, height, util);
}
void TextureManager__DeleteTexture(TextureManager *This, glTexture *texture)
{
	TextureManager_DeleteTexture(This, texture);
}
void TextureManager__UploadTexture(TextureManager *This, glTexture *texture, int level, const void *data, int width, int height, BOOL checkerror, BOOL realloc, glUtil *util)
{
	TextureManager_UploadTextureClassic(This, texture, level, data, width, height, checkerror, realloc, util);
}
void TextureManager__DownloadTexture(TextureManager *This, glTexture *texture, int level, void *data, glUtil *util)
{
	TextureManager_DownloadTextureClassic(This, texture, level, data, util);
}

void TextureManager_CreateTextureClassic(TextureManager *This, glTexture *texture, int width, int height, glUtil *util)
{
	int texformat = -1;
	int i;
	DWORD x, y;
	GLenum error;
	if (!texture->miplevel) texture->miplevel = 1;
	texture->pixelformat.dwSize = sizeof(DDPIXELFORMAT);
	for(i = 0; i < numtexformats; i++)
	{
		if(!memcmp(&texformats[i],&texture->pixelformat,sizeof(DDPIXELFORMAT)))
		{
			texformat = i;
			break;
		}
	}
	ZeroMemory(texture->internalformats, 8 * sizeof(GLint));
	switch(texformat)
	{
	case -1:
	case 0: // 8-bit palette
		if(This->ext->glver_major >= 3)
		{
			texture->internalformats[0] = GL_R8;
			texture->format = GL_RED;
		}
		else
		{
			texture->internalformats[0] = GL_RGBA8;
			texture->format = GL_LUMINANCE;
		}
		texture->type = GL_UNSIGNED_BYTE;
		texture->colororder = 4;
		texture->colorsizes[0] = 255;
		texture->colorsizes[1] = 255;
		texture->colorsizes[2] = 255;
		texture->colorsizes[3] = 255;
		texture->colorbits[0] = 8;
		texture->colorbits[1] = 0;
		texture->colorbits[2] = 0;
		texture->colorbits[3] = 0;
		break;
	case 1: // 8-bit RGB332
		texture->internalformats[0] = GL_R3_G3_B2;
		texture->internalformats[1] = GL_RGB8;
		texture->internalformats[2] = GL_RGBA8;
		texture->format = GL_RGB;
		texture->type = GL_UNSIGNED_BYTE_3_3_2;
		texture->colororder = 1;
		texture->colorsizes[0] = 7;
		texture->colorsizes[1] = 7;
		texture->colorsizes[2] = 3;
		texture->colorsizes[3] = 1;
		texture->colorbits[0] = 3;
		texture->colorbits[1] = 3;
		texture->colorbits[2] = 2;
		texture->colorbits[3] = 0;
		break;
	case 2: // 16-bit RGB555
		texture->internalformats[0] = GL_RGB5_A1;
		texture->internalformats[1] = GL_RGBA8;
		texture->format = GL_BGRA;
		texture->type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		texture->colororder = 1;
		texture->colorsizes[0] = 31;
		texture->colorsizes[1] = 31;
		texture->colorsizes[2] = 31;
		texture->colorsizes[3] = 1;
		texture->colorbits[0] = 5;
		texture->colorbits[1] = 5;
		texture->colorbits[2] = 5;
		texture->colorbits[3] = 1;
		break;
	case 3: // 16-bit RGB565
		texture->internalformats[0] = GL_RGB565;
		texture->internalformats[1] = GL_RGB8;
		texture->internalformats[2] = GL_RGBA8;
		texture->format = GL_RGB;
		texture->type = GL_UNSIGNED_SHORT_5_6_5;
		texture->colororder = 1;
		texture->colorsizes[0] = 31;
		texture->colorsizes[1] = 63;
		texture->colorsizes[2] = 31;
		texture->colorsizes[3] = 1;
		texture->colorbits[0] = 5;
		texture->colorbits[1] = 6;
		texture->colorbits[2] = 5;
		texture->colorbits[3] = 0;
		break;
	case 4: // 24-bit RGB888
		texture->internalformats[0] = GL_RGB8;
		texture->internalformats[1] = GL_RGBA8;
		texture->format = GL_BGR;
		texture->type = GL_UNSIGNED_BYTE;
		texture->colororder = 1;
		texture->colorsizes[0] = 255;
		texture->colorsizes[1] = 255;
		texture->colorsizes[2] = 255;
		texture->colorsizes[3] = 1;
		texture->colorbits[0] = 8;
		texture->colorbits[1] = 8;
		texture->colorbits[2] = 8;
		texture->colorbits[3] = 0;
		break;
	case 5: // 32-bit RGB888
		texture->internalformats[0] = GL_RGBA8;
		texture->format = GL_BGRA;
		texture->type = GL_UNSIGNED_INT_8_8_8_8_REV;
		texture->colororder = 1;
		texture->colorsizes[0] = 255;
		texture->colorsizes[1] = 255;
		texture->colorsizes[2] = 255;
		texture->colorsizes[3] = 1;
		texture->colorbits[0] = 8;
		texture->colorbits[1] = 8;
		texture->colorbits[2] = 8;
		texture->colorbits[3] = 0;
		break;
	case 6: // 32-bit BGR888
		texture->internalformats[0] = GL_RGBA8;
		texture->format = GL_RGBA;
		texture->type = GL_UNSIGNED_INT_8_8_8_8_REV;
		texture->colororder = 0;
		texture->colorsizes[0] = 255;
		texture->colorsizes[1] = 255;
		texture->colorsizes[2] = 255;
		texture->colorsizes[3] = 1;
		texture->colorbits[0] = 8;
		texture->colorbits[1] = 8;
		texture->colorbits[2] = 8;
		texture->colorbits[3] = 0;
		break;
	case 7: // 16-bit RGBA8332
		FIXME("Unusual texture format RGBA8332 not supported");
		texture->colororder = 1;
		texture->colorsizes[0] = 7;
		texture->colorsizes[1] = 7;
		texture->colorsizes[2] = 3;
		texture->colorsizes[3] = 255;
		texture->colorbits[0] = 3;
		texture->colorbits[1] = 3;
		texture->colorbits[2] = 2;
		texture->colorbits[3] = 8;
		break;
	case 8: // 16-bit RGBA4444
		texture->internalformats[0] = GL_RGBA4;
		texture->internalformats[1] = GL_RGBA8;
		texture->format = GL_BGRA;
		texture->type = GL_UNSIGNED_SHORT_4_4_4_4_REV;
		texture->colororder = 1;
		texture->colorsizes[0] = 15;
		texture->colorsizes[1] = 15;
		texture->colorsizes[2] = 15;
		texture->colorsizes[3] = 15;
		texture->colorbits[0] = 4;
		texture->colorbits[1] = 4;
		texture->colorbits[2] = 4;
		texture->colorbits[3] = 4;
		break;
	case 9: // 16-bit RGBA1555
		texture->internalformats[0] = GL_RGB5_A1;
		texture->internalformats[1] = GL_RGBA8;
		texture->format = GL_BGRA;
		texture->type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		texture->colorbits[0] = 5;
		texture->colorbits[1] = 5;
		texture->colorbits[2] = 5;
		texture->colorbits[3] = 1;
		break;
	case 10: // 32-bit RGBA8888
		texture->internalformats[0] = GL_RGBA8;
		texture->format = GL_BGRA;
		texture->type = GL_UNSIGNED_INT_8_8_8_8_REV;
		texture->colororder = 1;
		texture->colorsizes[0] = 255;
		texture->colorsizes[1] = 255;
		texture->colorsizes[2] = 255;
		texture->colorsizes[3] = 255;
		texture->colorbits[0] = 8;
		texture->colorbits[1] = 8;
		texture->colorbits[2] = 8;
		texture->colorbits[3] = 8;
		break;
	case 11: // 8-bit Luminance
		texture->internalformats[0] = GL_LUMINANCE8;
		texture->internalformats[1] = GL_RGB8;
		texture->internalformats[2] = GL_RGBA8;
		texture->format = GL_LUMINANCE;
		texture->type = GL_UNSIGNED_BYTE;
		texture->colororder = 5;
		texture->colorsizes[0] = 255;
		texture->colorsizes[1] = 255;
		texture->colorsizes[2] = 255;
		texture->colorsizes[3] = 255;
		texture->colorbits[0] = 8;
		texture->colorbits[1] = 0;
		texture->colorbits[2] = 0;
		texture->colorbits[3] = 0;
		break;
	case 12: // 8-bit Alpha
		texture->internalformats[0] = GL_ALPHA8;
		texture->format = GL_ALPHA;
		texture->type = GL_UNSIGNED_BYTE;
		texture->colororder = 6;
		texture->colorsizes[0] = 255;
		texture->colorsizes[1] = 255;
		texture->colorsizes[2] = 255;
		texture->colorsizes[3] = 255;
		texture->colorbits[0] = 0;
		texture->colorbits[1] = 0;
		texture->colorbits[2] = 0;
		texture->colorbits[3] = 8;
		break;
	case 13: // 16-bit Luminance Alpha
		texture->internalformats[0] = GL_LUMINANCE8_ALPHA8;
		texture->internalformats[1] = GL_RGBA8;
		texture->format = GL_LUMINANCE_ALPHA;
		texture->type = GL_UNSIGNED_BYTE;
		texture->colororder = 7;
		texture->colorsizes[0] = 255;
		texture->colorsizes[1] = 255;
		texture->colorsizes[2] = 255;
		texture->colorsizes[3] = 255;
		texture->colorbits[0] = 8;
		texture->colorbits[1] = 0;
		texture->colorbits[2] = 0;
		texture->colorbits[3] = 8;
		break;
	case 14: // 16-bit Z buffer
		texture->internalformats[0] = GL_DEPTH_COMPONENT16;
		texture->format = GL_DEPTH_COMPONENT;
		texture->type = GL_UNSIGNED_SHORT;
		texture->colororder = 4;
		texture->colorsizes[0] = 65535;
		texture->colorsizes[1] = 65535;
		texture->colorsizes[2] = 65535;
		texture->colorsizes[3] = 65535;
		texture->colorbits[0] = 16;
		texture->colorbits[1] = 0;
		texture->colorbits[2] = 0;
		texture->colorbits[3] = 0;
		break;
	case 15: // 24-bit Z buffer
		texture->internalformats[0] = GL_DEPTH_COMPONENT24;
		texture->format = GL_DEPTH_COMPONENT;
		texture->type = GL_UNSIGNED_INT;
		texture->colororder = 4;
		texture->colorsizes[0] = 16777215;
		texture->colorsizes[1] = 16777215;
		texture->colorsizes[2] = 16777215;
		texture->colorsizes[3] = 16777215;
		texture->colorbits[0] = 24;
		texture->colorbits[1] = 0;
		texture->colorbits[2] = 0;
		texture->colorbits[3] = 0;
		break;
	case 16: // 32/24 bit Z buffer
		texture->internalformats[0] = GL_DEPTH_COMPONENT24;
		texture->format = GL_DEPTH_COMPONENT;
		texture->type = GL_UNSIGNED_INT;
		texture->colororder = 4;
		texture->colorsizes[0] = 16777215;
		texture->colorsizes[1] = 16777215;
		texture->colorsizes[2] = 16777215;
		texture->colorsizes[3] = 16777215;
		texture->colorbits[0] = 24;
		texture->colorbits[1] = 0;
		texture->colorbits[2] = 0;
		texture->colorbits[3] = 0;
		break;
	case 17: // 32-bit Z buffer
		texture->internalformats[0] = GL_DEPTH_COMPONENT32;
		texture->format = GL_DEPTH_COMPONENT;
		texture->type = GL_UNSIGNED_INT;
		texture->colororder = 4;
		texture->colorsizes[0] = 4294967295;
		texture->colorsizes[1] = 4294967295;
		texture->colorsizes[2] = 4294967295;
		texture->colorsizes[3] = 4294967295;
		texture->colorbits[0] = 32;
		texture->colorbits[1] = 0;
		texture->colorbits[2] = 0;
		texture->colorbits[3] = 0;
		break;
	case 18: // 32-bit Z/Stencil buffer, depth LSB
		texture->internalformats[0] = GL_DEPTH24_STENCIL8;
		texture->format = GL_DEPTH_STENCIL;
		texture->type = GL_UNSIGNED_INT_24_8;
		texture->colororder = 7;
		texture->colorsizes[0] = 16777215;
		texture->colorsizes[1] = 16777215;
		texture->colorsizes[2] = 16777215;
		texture->colorsizes[3] = 255;
		texture->colorbits[0] = 24;
		texture->colorbits[1] = 0;
		texture->colorbits[2] = 0;
		texture->colorbits[3] = 8;
		break;
	case 19: // 32-bit Z/Stencil buffer, depth MSB
		texture->internalformats[0] = GL_DEPTH24_STENCIL8;
		texture->format = GL_DEPTH_STENCIL;
		texture->type = GL_UNSIGNED_INT_24_8;
		texture->colororder = 7;
		texture->colorsizes[0] = 16777215;
		texture->colorsizes[1] = 16777215;
		texture->colorsizes[2] = 16777215;
		texture->colorsizes[3] = 255;
		texture->colorbits[0] = 24;
		texture->colorbits[1] = 0;
		texture->colorbits[2] = 0;
		texture->colorbits[3] = 8;
		break;
	}
	texture->width = width;
	texture->height = height;
	glGenTextures(1,&texture->id);
	glUtil_SetTexture(util,0,texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture->minfilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture->magfilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture->wraps);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture->wrapt);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, texture->miplevel - 1);
	x = texture->width;
	y = texture->height;
	for (i = 0; i < texture->miplevel; i++)
	{
		do
		{
			ClearError();
			glTexImage2D(GL_TEXTURE_2D, i, texture->internalformats[0], x, y, 0, texture->format, texture->type, NULL);
			ShrinkMip(&x, &y);
			error = glGetError();
			if (error != GL_NO_ERROR)
			{
				if (texture->internalformats[1] == 0)
				{
					FIXME("Failed to create texture, cannot find internal format");
					break;
				}
				memmove(&texture->internalformats[0], &texture->internalformats[1], 7 * sizeof(GLint));
				texture->internalformats[7] = 0;
			}
			else break;
		} while (1);
	}
}

void TextureManager_DeleteTexture(TextureManager *This, glTexture *texture)
{
	glDeleteTextures(1,&texture->id);
	texture->bordercolor = texture->format = texture->type = texture->width =
		texture->height = texture->magfilter = texture->minfilter =
		texture->miplevel = texture->wraps = texture->wrapt =
		texture->id = 0;
	texture->pboPack = texture->pboUnpack = NULL;
	ZeroMemory(texture->internalformats, 8 * sizeof(GLint));
}

void TextureManager_UploadTextureClassic(TextureManager *This, glTexture *texture, int level, const void *data, int width, int height, BOOL checkerror, BOOL realloc, glUtil *util)
{
	GLenum error;
	texture->width = width;
	texture->height = height;
	if (checkerror)
	{
		do
		{
			ClearError();
			if (This->ext->GLEXT_EXT_direct_state_access)
			{
				if (realloc)This->ext->glTextureImage2DEXT(texture->id, GL_TEXTURE_2D, level, texture->internalformats[0],
					width, height, 0, texture->format, texture->type, data);
				else This->ext->glTextureSubImage2DEXT(texture->id, GL_TEXTURE_2D, level, 0, 0, width, height, texture->format, texture->type, data);
			}
			else
			{
				glUtil_SetActiveTexture(util, 0);
				glUtil_SetTexture(util, 0, texture);
				if (realloc)glTexImage2D(GL_TEXTURE_2D, level, texture->internalformats[0], width, height, 0, texture->format, texture->type, data);
				else glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, width, height, texture->format, texture->type, data);
			}
			error = glGetError();
			if (error != GL_NO_ERROR)
			{
				if (texture->internalformats[1] == 0)
				{
					FIXME("Failed to update texture, cannot find internal format");
					break;
				}
				memmove(&texture->internalformats[0], &texture->internalformats[1], 7 * sizeof(GLint));
				texture->internalformats[7] = 0;
			}
			else break;
		} while (1);
	}
	else
	{
		if (This->ext->GLEXT_EXT_direct_state_access)
		{
			if (realloc)This->ext->glTextureImage2DEXT(texture->id, GL_TEXTURE_2D, level, texture->internalformats[0],
				width, height, 0, texture->format, texture->type, data);
			else This->ext->glTextureSubImage2DEXT(texture->id, GL_TEXTURE_2D, level, 0, 0, width, height, texture->format, texture->type, data);
		}
		else
		{
			glUtil_SetActiveTexture(util, 0);
			glUtil_SetTexture(util, 0, texture);
			if (realloc)glTexImage2D(GL_TEXTURE_2D, level, texture->internalformats[0], width, height, 0, texture->format, texture->type, data);
			else glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, width, height, texture->format, texture->type, data);
		}
	}
}

void TextureManager_DownloadTextureClassic(TextureManager *This, glTexture *texture, int level, void *data, glUtil *util)
{
	if(This->ext->GLEXT_EXT_direct_state_access) This->ext->glGetTextureImageEXT(texture->id,GL_TEXTURE_2D,level,texture->format,texture->type,data);
	else
	{
		glUtil_SetActiveTexture(util, 0);
		glUtil_SetTexture(util, 0,texture);
		glGetTexImage(GL_TEXTURE_2D,level,texture->format,texture->type,data);
	}
}

BOOL TextureManager_FixTexture(TextureManager *This, glTexture *texture, void *data, DWORD *dirty, GLint level, glUtil *util)
{
	// data should be null to create uninitialized texture or be pointer to top-level
	// buffer to retain texture data
	glTexture newtexture;
	GLenum error;
	memcpy(&newtexture, texture, sizeof(glTexture));
	if (texture->internalformats[1] == 0) return FALSE;
	glGenTextures(1, &newtexture.id);
	glUtil_SetActiveTexture(util, 0);
	if (data)
	{
		glUtil_SetTexture(util, 0, texture);
		glGetTexImage(GL_TEXTURE_2D, level, texture->format, texture->type, data);
		if (dirty) *dirty |= 2;
	}
	glUtil_SetTexture(util, 0, &newtexture);
	do
	{
		memmove(&newtexture.internalformats[0], &newtexture.internalformats[1], 7 * sizeof(GLint));
		newtexture.internalformats[7] = 0;
		ClearError();
		glTexImage2D(GL_TEXTURE_2D, level, newtexture.internalformats[0], newtexture.width, newtexture.height,
			0, newtexture.format, newtexture.type, data);
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			if (newtexture.internalformats[1] == 0)
			{
				FIXME("Failed to repair texture, cannot find internal format");
				break;
			}
		}
		else break;
	} while (1);
	TextureManager__DeleteTexture(This, texture);
	memcpy(texture, &newtexture, sizeof(glTexture));
	return TRUE;
}

}