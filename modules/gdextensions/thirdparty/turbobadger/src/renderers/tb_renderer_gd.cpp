// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_renderer_gd.h"

#include "tb_bitmap_fragment.h"
#include "tb_system.h"
#include <assert.h>
#include <stdio.h>

namespace tb {

#ifdef TB_RUNTIME_DEBUG_INFO
uint32 dbg_bitmap_validations = 0;
#endif // TB_RUNTIME_DEBUG_INFO

// == Utilities ===================================================================================

static void Ortho2D(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top)
{
	glOrthof(left, right, bottom, top, -1.0, 1.0);
}

// == Batching ====================================================================================

Ref<Texture> g_current_texture = nullptr;
TBRendererBatcher::Batch *g_current_batch = nullptr;

void BindBitmap(TBBitmap *bitmap)
{
	Ref<Texture> texture = bitmap ? static_cast<TBBitmapGD*>(bitmap)->m_texture : nullptr;
	if (texture != g_current_texture)
	{
		g_current_texture = texture;
	}
}

// == TBBitmapGD ==================================================================================

TBBitmapGD::TBBitmapGD(TBRendererGD *renderer)
	: m_renderer(renderer), m_w(0), m_h(0), m_texture(0)
{
}

TBBitmapGD::~TBBitmapGD()
{
	// Must flush and unbind before we delete the texture
	m_renderer->FlushBitmap(this);
	if (m_texture == g_current_texture) {
		BindBitmap(nullptr);
	}
	glDeleteTextures(1, &m_texture);
}

bool TBBitmapGD::Init(int width, int height, uint32 *data)
{
	assert(width == TBGetNearestPowerOfTwo(width));
	assert(height == TBGetNearestPowerOfTwo(height));

	m_w = width;
	m_h = height;

	glGenTextures(1, &m_texture);
	BindBitmap(this);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	SetData(data);

	return true;
}

void TBBitmapGD::SetData(uint32 *data)
{
	m_renderer->FlushBitmap(this);
	BindBitmap(this);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_w, m_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	TB_IF_DEBUG_SETTING(RENDER_BATCHES, dbg_bitmap_validations++);
}

// == TBRendererGD ================================================================================

TBRendererGD::TBRendererGD()
{
}

void TBRendererGD::BeginPaint(int render_target_w, int render_target_h)
{
#ifdef TB_RUNTIME_DEBUG_INFO
	dbg_bitmap_validations = 0;
#endif

	TBRendererBatcher::BeginPaint(render_target_w, render_target_h);

	g_current_texture = nullptr;
	g_current_batch = nullptr;
}

void TBRendererGD::EndPaint()
{
	TBRendererBatcher::EndPaint();

#ifdef TB_RUNTIME_DEBUG_INFO
	if (TB_DEBUG_SETTING(RENDER_BATCHES))
		TBDebugPrint("Frame caused %d bitmap validations.\n", dbg_bitmap_validations);
#endif // TB_RUNTIME_DEBUG_INFO
}

TBBitmap *TBRendererGD::CreateBitmap(int width, int height, uint32 *data)
{
	TBBitmapGD *bitmap = new TBBitmapGD(this);
	if (!bitmap || !bitmap->Init(width, height, data))
	{
		delete bitmap;
		return nullptr;
	}
	return bitmap;
}

void TBRendererGD::RenderBatch(Batch *batch)
{
	// Bind texture and array pointers
	BindBitmap(batch->bitmap);
	if (g_current_batch != batch)
	{
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), (void *) &batch->vertex[0].r);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void *) &batch->vertex[0].u);
		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), (void *) &batch->vertex[0].x);
		g_current_batch = batch;
	}

	// Flush
	glDrawArrays(GL_TRIANGLES, 0, batch->vertex_count);
}

void TBRendererGD::SetClipRect(const TBRect &rect)
{
	glScissor(m_clip_rect.x, m_screen_rect.h - (m_clip_rect.y + m_clip_rect.h), m_clip_rect.w, m_clip_rect.h);
}

} // namespace tb
