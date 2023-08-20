/**************************************************************************/
/*  glinfo.c                                                              */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#define GL_SILENCE_DEPRECATION

#if defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#elif defined(_WIN32)
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <GL/gl.h>
#include <windows.h>
#endif

#include <stdio.h>

// clang -o glinfo glinfo.c -framework OpenGL

int main(int argc, char *argv[]) {
#if defined(__APPLE__)
	CGLContextObj ctx;
	CGLPixelFormatObj pix;
	GLint npix;
	CGLPixelFormatAttribute attribs[] = {
		(CGLPixelFormatAttribute)0
	};
	CGLChoosePixelFormat(attribs, &pix, &npix);
	CGLCreateContext(pix, NULL, &ctx);
	CGLSetCurrentContext(ctx);
#elif defined(_WIN32)
	HINSTANCE hInst = GetModuleHandle(0);
	WNDCLASSA windowClass = { 0 };
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = WindowCallbackFn;
	windowClass.hInstance = _inst;
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);
	windowClass.hbrBackground = 0;
	windowClass.lpszClassName = "GLWindow";

	if (!RegisterClassA(&windowClass)) {
		DWORD errCode = GetLastError();
		perror("Failed to register window");
		return 1;
	}

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = _width;
	rect.bottom = _height;

	DWORD windowStyle = WS_OVERLAPPEDWINDOW;
	AdjustWindowRect(&rect, windowStyle, false);

	HWND window = CreateWindowExA(
			0,
			windowClass.lpszClassName,
			"OpenGL",
			windowStyle,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			rect.right - rect.left,
			rect.bottom - rect.top,
			0,
			0,
			_inst,
			0);

	if (!window) {
		perror("Failed to create window");
		return 1;
	}

	HDC winHDC = GetDC(_window);

	PIXELFORMATDESCRIPTOR windowPixelFormatDesc = { 0 };
	windowPixelFormatDesc.nSize = sizeof(windowPixelFormatDesc);
	windowPixelFormatDesc.nVersion = 1;
	windowPixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
	windowPixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	windowPixelFormatDesc.cColorBits = 32;
	windowPixelFormatDesc.cAlphaBits = 8;
	windowPixelFormatDesc.iLayerType = PFD_MAIN_PLANE;
	windowPixelFormatDesc.cDepthBits = 24;
	windowPixelFormatDesc.cStencilBits = 8;

	int pixelFormat = ChoosePixelFormat(winHDC, &windowPixelFormatDesc);
	if (!pixelFormat) {
		perror("Unable to find a suitable pixel format for the requested description");
		return 1;
	}

	if (!SetPixelFormat(winHDC, pixelFormat, &windowPixelFormatDesc)) {
		std::cout << "Unable to set the pixel format.\n";
	}

	HGLRC glContext = wglCreateContext(winHDC);
	if (!glContext) {
		perror("Unable to create OpenGL context");
		return 1;
	}

	if (!wglMakeCurrent(winHDC, glContext)) {
		perror("Unable to apply OpenGL context to window");
		return 1;
	}
#endif

	printf("GL_VERSION: %s\n", (char *)glGetString(GL_VERSION));
	printf("GL_RENDERER: %s\n", (char *)glGetString(GL_RENDERER));
	printf("GL_VENDOR: %s\n", (char *)glGetString(GL_VENDOR));
	printf("GL_EXTENSIONS: %s\n", (char *)glGetString(GL_EXTENSIONS));
}
