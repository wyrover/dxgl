// DXGL
// Copyright (C) 2012-2015 William Feely

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

#ifndef _GLRENDERER_H
#define _GLRENDERER_H

typedef struct
{
	float Version;
	float ShaderVer;
	GLint TextureMax;
} GLCAPS;
typedef struct
{
	BOOL enabled;
	int width;
	int height;
	int pitch;
	HDC hdc;
	HBITMAP hbitmap;
	BITMAPINFO info;
	BYTE *pixels;
} DIB;

typedef struct
{
	void *data;
	int stride;
} GLVERTEX;

typedef struct
{
	__int64 stateid;
	__int64 texstageid[8];
} SHADERSTATE;

struct TEXTURESTAGE
{
	D3DTEXTUREOP colorop;
	DWORD colorarg1;
	DWORD colorarg2;
	D3DTEXTUREOP alphaop;
	DWORD alphaarg1;
	DWORD alphaarg2;
	D3DVALUE bumpenv00;
	D3DVALUE bumpenv01;
	D3DVALUE bumpenv10;
	D3DVALUE bumpenv11;
	DWORD texcoordindex;
	D3DTEXTUREADDRESS addressu;
	D3DTEXTUREADDRESS addressv;
	DWORD bordercolor;
	D3DTEXTUREMAGFILTER magfilter;
	D3DTEXTUREMINFILTER minfilter;
	D3DTEXTUREMIPFILTER mipfilter;
	D3DVALUE lodbias;
	DWORD miplevel;
	DWORD anisotropy;
	D3DVALUE bumpenvlscale;
	D3DVALUE bumpenvloffset;
	D3DTEXTURETRANSFORMFLAGS textransform;
	glDirectDrawSurface7 *texture;
	GLint glmagfilter;
	GLint glminfilter;
};

#define OP_NULL						0
#define OP_SETWND					1
#define OP_DELETE					2
#define OP_CREATE					3
#define OP_UPLOAD					4
#define OP_DOWNLOAD					5
#define OP_DELETETEX				6
#define OP_BLT						7
#define OP_DRAWSCREEN				8
#define OP_INITD3D					9
#define OP_CLEAR					10
#define OP_FLUSH					11
#define OP_DRAWPRIMITIVES			12
#define OP_DELETEFBO				13
#define OP_UPDATECLIPPER			14
#define OP_DEPTHFILL				15
#define OP_SETRENDERSTATE			16
#define OP_SETTEXTURE				17
#define OP_SETTEXTURESTAGESTATE		18
#define OP_SETTRANSFORM				19
#define OP_SETMATERIAL				20
#define OP_SETLIGHT					21
#define OP_SETVIEWPORT				22

extern const DWORD renderstate_default[153];
extern const TEXTURESTAGE texstagedefault0;
extern const TEXTURESTAGE texstagedefault1;

#ifdef __cplusplus
class glDirectDraw7;
class glDirect3DDevice7;
class glDirectDrawSurface7;
class glRenderWindow;
extern "C" {
#else
typedef int glDirectDraw7;
typedef int glDirect3DDevice7;
typedef int glDirectDrawSurface7;
typedef int glRenderWindow;
#endif

/** @brief glRenderer struct
  * OpenGL renderer structure for DXGL.
  */
typedef struct glRenderer
{
	HGLRC hRC;
	GLCAPS gl_caps;
	glExtensions *ext;
	glDirectDraw7 *ddInterface;
	int opcode;
	void* inputs[32];
	void* outputs[32];
	HANDLE hThread;
	HDC hDC;
	HWND hWnd;
	glRenderWindow *RenderWnd;
	DIB dib;
	FBO fbo;
	BufferObject *pbo;
	CRITICAL_SECTION cs;
	HANDLE busy;
	HANDLE start;
	unsigned int frequency;
	DXGLTimer timer;
	glTexture *backbuffer;
	int backx;
	int backy;
	DWORD fogcolor;
	GLfloat fogstart;
	GLfloat fogend;
	GLfloat fogdensity;
	BltVertex bltvertices[4];
	int oldswap;
	TextureManager *texman;
	glUtil *util;
	ShaderManager *shaders;
	DWORD renderstate[153];
	SHADERSTATE shaderstate3d;
	TEXTURESTAGE texstages[8];
	D3DMATERIAL7 material;
	D3DLIGHT7 lights[8];
	D3DMATRIX transform[24];
	D3DVIEWPORT7 viewport;
} glRenderer;

void glRenderer_Init(glRenderer *This, int width, int height, int bpp, bool fullscreen, unsigned int frequency, HWND hwnd, glDirectDraw7 *glDD7, BOOL devwnd);
void glRenderer_Delete(glRenderer *This);
static DWORD WINAPI glRenderer_ThreadEntry(void *entry);
void glRenderer_UploadTexture(glRenderer *This, char *buffer, char *bigbuffer, glTexture *texture, int x, int y, int bigx, int bigy, int pitch, int bigpitch, int bpp, int miplevel);
void glRenderer_DownloadTexture(glRenderer *This, char *buffer, char *bigbuffer, glTexture *texture, int x, int y, int bigx, int bigy, int pitch, int bigpitch, int bpp, int miplevel);
HRESULT glRenderer_Blt(glRenderer *This, LPRECT lpDestRect, glDirectDrawSurface7 *src,
	glDirectDrawSurface7 *dest, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx);
void glRenderer_MakeTexture(glRenderer *This, glTexture *texture, DWORD width, DWORD height);
void glRenderer_DrawScreen(glRenderer *This, glTexture *texture, glTexture *paltex, glDirectDrawSurface7 *dest, glDirectDrawSurface7 *src, GLint vsync);
void glRenderer_DeleteTexture(glRenderer *This, glTexture *texture);
void glRenderer_InitD3D(glRenderer *This, int zbuffer, int x, int y);
void glRenderer_Flush(glRenderer *This);
void glRenderer_SetWnd(glRenderer *This, int width, int height, int bpp, int fullscreen, unsigned int frequency, HWND newwnd, BOOL devwnd);
HRESULT glRenderer_Clear(glRenderer *This, glDirectDrawSurface7 *target, DWORD dwCount, LPD3DRECT lpRects, DWORD dwFlags, DWORD dwColor, D3DVALUE dvZ, DWORD dwStencil);
HRESULT glRenderer_DrawPrimitives(glRenderer *This, glDirect3DDevice7 *device, GLenum mode, GLVERTEX *vertices, int *texformats, DWORD count, LPWORD indices,
	DWORD indexcount, DWORD flags);
void glRenderer_DeleteFBO(glRenderer *This, FBO *fbo);
void glRenderer_UpdateClipper(glRenderer *This, glDirectDrawSurface7 *surface);
unsigned int glRenderer_GetScanLine(glRenderer *This);
HRESULT glRenderer_DepthFill(glRenderer *This, LPRECT lpDestRect, glDirectDrawSurface7 *dest, LPDDBLTFX lpDDBltFx);
void glRenderer_SetRenderState(glRenderer *This, D3DRENDERSTATETYPE dwRendStateType, DWORD dwRenderState);
void glRenderer_SetTexture(glRenderer *This, DWORD dwStage, glDirectDrawSurface7 *Texture);
void glRenderer_SetTextureStageState(glRenderer *This, DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, DWORD dwValue);
void glRenderer_SetTransform(glRenderer *This, D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix);
void glRenderer_SetMaterial(glRenderer *This, LPD3DMATERIAL7 lpMaterial);
void glRenderer_SetLight(glRenderer *This, DWORD index, LPD3DLIGHT7 light, BOOL remove);
void glRenderer_SetViewport(glRenderer *This, LPD3DVIEWPORT7 lpViewport);
// In-thread APIs
DWORD glRenderer__Entry(glRenderer *This);
BOOL glRenderer__InitGL(glRenderer *This, int width, int height, int bpp, int fullscreen, unsigned int frequency, HWND hWnd, glDirectDraw7 *glDD7);
void glRenderer__UploadTexture(glRenderer *This, char *buffer, char *bigbuffer, glTexture *texture, int x, int y, int bigx, int bigy, int pitch, int bigpitch, int bpp, int miplevel);
void glRenderer__DownloadTexture(glRenderer *This, char *buffer, char *bigbuffer, glTexture *texture, int x, int y, int bigx, int bigy, int pitch, int bigpitch, int bpp, int miplevel);
void glRenderer__Blt(glRenderer *This, LPRECT lpDestRect, glDirectDrawSurface7 *src,
	glDirectDrawSurface7 *dest, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx);
void glRenderer__MakeTexture(glRenderer *This, glTexture *texture, DWORD width, DWORD height);
void glRenderer__DrawScreen(glRenderer *This, glTexture *texture, glTexture *paltex, glDirectDrawSurface7 *dest, glDirectDrawSurface7 *src, GLint vsync, bool setsync);
void glRenderer__DeleteTexture(glRenderer *This, glTexture *texture);
void glRenderer__DrawBackbuffer(glRenderer *This, glTexture **texture, int x, int y, int progtype);
void glRenderer__DrawBackbufferRect(glRenderer *This, glTexture *texture, RECT srcrect, int progtype);
void glRenderer__InitD3D(glRenderer *This, int zbuffer, int x, int y);
void glRenderer__Clear(glRenderer *This, glDirectDrawSurface7 *target, DWORD dwCount, LPD3DRECT lpRects, DWORD dwFlags, DWORD dwColor, D3DVALUE dvZ, DWORD dwStencil);
void glRenderer__DrawPrimitives(glRenderer *This, glDirect3DDevice7 *device, GLenum mode, GLVERTEX *vertices, int *texcormats, DWORD count, LPWORD indices,
	DWORD indexcount, DWORD flags);
void glRenderer__Flush(glRenderer *This);
void glRenderer__SetWnd(glRenderer *This, int width, int height, int fullscreen, int bpp, unsigned int frequency, HWND newwnd, BOOL devwnd);
void glRenderer__DeleteFBO(glRenderer *This, FBO *fbo);
void glRenderer__UpdateClipper(glRenderer *This, glDirectDrawSurface7 *surface);
void glRenderer__DepthFill(glRenderer *This, LPRECT lpDestRect, glDirectDrawSurface7 *dest, LPDDBLTFX lpDDBltFx);
void glRenderer__SetFogColor(glRenderer *This, DWORD color);
void glRenderer__SetFogStart(glRenderer *This, GLfloat start);
void glRenderer__SetFogEnd(glRenderer *This, GLfloat end);
void glRenderer__SetFogDensity(glRenderer *This, GLfloat density);
inline void glRenderer__SetSwap(glRenderer *This, int swap);
void glRenderer__SetBlend(glRenderer *This, DWORD src, DWORD dest);
void glRenderer__SetRenderState(glRenderer *This, D3DRENDERSTATETYPE dwRendStateType, DWORD dwRenderState);
void glRenderer__SetTexture(glRenderer *This, DWORD dwStage, glDirectDrawSurface7 *Texture);
void glRenderer__SetTextureStageState(glRenderer *This, DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, DWORD dwValue);
void glRenderer__SetTransform(glRenderer *This, D3DTRANSFORMSTATETYPE dtstTransformStateType, LPD3DMATRIX lpD3DMatrix);
void glRenderer__SetMaterial(glRenderer *This, LPD3DMATERIAL7 lpMaterial);
void glRenderer__SetLight(glRenderer *This, DWORD index, LPD3DLIGHT7 light, BOOL remove);
void glRenderer__SetViewport(glRenderer *This, LPD3DVIEWPORT7 lpViewport);
void glRenderer__SetDepthComp(glRenderer *This);

#ifdef __cplusplus
}
#endif

#endif //_GLRENDERER_H
