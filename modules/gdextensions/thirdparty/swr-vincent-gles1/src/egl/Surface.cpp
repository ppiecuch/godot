// Surface.cpp — Platform-independent Surface implementation for offscreen rendering.
// Provides ClearColorBuffer, ClearDepthBuffer, ClearStencilBuffer, and
// resource management (constructor/destructor/Dispose).

#include "Surface.h"
#include <stdlib.h>
#include <string.h>

using namespace EGL;

Surface::Surface(const Config &config, NativeDisplayType hdc) {
	m_Config = config;
	m_SurfaceType = 0;
	m_WindowDepth = 16;
	m_Disposed = false;
	m_CurrentContext = nullptr;

	int w = config.GetConfigAttrib(EGL_WIDTH);
	int h = config.GetConfigAttrib(EGL_HEIGHT);
	if (w <= 0) w = 1;
	if (h <= 0) h = 1;

	m_Rect.x = 0;
	m_Rect.y = 0;
	m_Rect.width = (U16)w;
	m_Rect.height = (U16)h;

	int pixels = w * h;
	m_ColorBuffer = (U16 *)malloc(pixels * sizeof(U16));
	m_AlphaBuffer = (U8 *)malloc(pixels * sizeof(U8));
	m_DepthBuffer = (U16 *)malloc(pixels * sizeof(U16));
	m_StencilBuffer = (U32 *)malloc(pixels * sizeof(U32));

	memset(m_ColorBuffer, 0, pixels * sizeof(U16));
	memset(m_AlphaBuffer, 0xFF, pixels * sizeof(U8));
	memset(m_DepthBuffer, 0xFF, pixels * sizeof(U16));
	memset(m_StencilBuffer, 0, pixels * sizeof(U32));
}

Surface::~Surface() {
	Dispose();
}

void Surface::Dispose() {
	if (!m_Disposed) {
		m_Disposed = true;
		free(m_ColorBuffer);
		free(m_AlphaBuffer);
		free(m_DepthBuffer);
		free(m_StencilBuffer);
		m_ColorBuffer = nullptr;
		m_AlphaBuffer = nullptr;
		m_DepthBuffer = nullptr;
		m_StencilBuffer = nullptr;
	}
}

bool Surface::Save(const char *filename) {
	return false; // not implemented
}

void Surface::SetCurrentContext(Context *context) {
	m_CurrentContext = context;
}

void Surface::ClearColorBuffer(const Color &rgba, const Color &mask, const Rect &scissor) {
	if (!m_ColorBuffer) return;

	// Convert Color (U8 components) to RGB565
	U8 r8 = rgba.r;
	U8 g8 = rgba.g;
	U8 b8 = rgba.b;
	U8 a8 = rgba.a;

	U16 color565 = ((r8 >> 3) << 11) | ((g8 >> 2) << 5) | (b8 >> 3);

	int x0 = scissor.x;
	int y0 = scissor.y;
	int x1 = x0 + scissor.width;
	int y1 = y0 + scissor.height;

	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 > m_Rect.width) x1 = m_Rect.width;
	if (y1 > m_Rect.height) y1 = m_Rect.height;

	for (int y = y0; y < y1; y++) {
		for (int x = x0; x < x1; x++) {
			int idx = y * m_Rect.width + x;
			m_ColorBuffer[idx] = color565;
			if (m_AlphaBuffer) {
				m_AlphaBuffer[idx] = a8;
			}
		}
	}
}

void Surface::ClearDepthBuffer(U16 depth, bool mask, const Rect &scissor) {
	if (!m_DepthBuffer || !mask) return;

	int x0 = scissor.x;
	int y0 = scissor.y;
	int x1 = x0 + scissor.width;
	int y1 = y0 + scissor.height;

	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 > m_Rect.width) x1 = m_Rect.width;
	if (y1 > m_Rect.height) y1 = m_Rect.height;

	for (int y = y0; y < y1; y++) {
		for (int x = x0; x < x1; x++) {
			m_DepthBuffer[y * m_Rect.width + x] = depth;
		}
	}
}

void Surface::ClearStencilBuffer(U32 value, U32 mask, const Rect &scissor) {
	if (!m_StencilBuffer) return;

	int x0 = scissor.x;
	int y0 = scissor.y;
	int x1 = x0 + scissor.width;
	int y1 = y0 + scissor.height;

	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 > m_Rect.width) x1 = m_Rect.width;
	if (y1 > m_Rect.height) y1 = m_Rect.height;

	for (int y = y0; y < y1; y++) {
		for (int x = x0; x < x1; x++) {
			int idx = y * m_Rect.width + x;
			m_StencilBuffer[idx] = (m_StencilBuffer[idx] & ~mask) | (value & mask);
		}
	}
}
