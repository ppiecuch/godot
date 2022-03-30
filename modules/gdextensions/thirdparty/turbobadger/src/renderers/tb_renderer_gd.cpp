// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_renderer_gd.h"

#include "tb_bitmap_fragment.h"
#include "tb_system.h"

#include "core/variant.h"
#include "core/image.h"
#include "common/gd_core.h"
#include "scene/resources/texture.h"

namespace tb {

#ifdef TB_RUNTIME_DEBUG_INFO
uint32 dbg_bitmap_validations = 0;
#endif // TB_RUNTIME_DEBUG_INFO

// == TBBitmapGD ==================================================================================

void TBBitmapGD::BuildTexture(int width, int height, uint32 *data)
{
	Ref<Image> image = newref(Image);
	PoolByteArray d;
	const size_t data_size = width * height * 4; // RGBA only
	d.resize(data_size);
	memcpy(d.write().ptr(), data, data_size);
	image->create(width, height, false, Image::FORMAT_RGBA8, d);
	Ref<ImageTexture> texture = newref(ImageTexture);
	texture->create_from_image(image);
	m_texture = texture;
}

TBBitmapGD::TBBitmapGD(TBRendererGD *renderer)
	: m_renderer(renderer), m_texture(0)
{
}

TBBitmapGD::~TBBitmapGD()
{
	// Must flush and unbind before we delete the texture
	m_renderer->FlushBitmap(this);
}

bool TBBitmapGD::Init(int width, int height, uint32 *data)
{
	ERR_FAIL_COND_V(width == TBGetNearestPowerOfTwo(width), false);
	ERR_FAIL_COND_V(height == TBGetNearestPowerOfTwo(height), false);

	BuildTexture(width, height, data);

	return true;
}

void TBBitmapGD::SetData(uint32 *data)
{
	ERR_FAIL_NULL(m_texture);

	m_renderer->FlushBitmap(this);
	BuildTexture(Width(), Height(), data);

	TB_IF_DEBUG_SETTING(RENDER_BATCHES, dbg_bitmap_validations++);
}

// == TBRendererGD ================================================================================

TBRendererGD::TBRendererGD()
{
}

void TBRendererGD::DrawMesh(Ref<Mesh> mesh, TBBitmap *bitmap)
{
	_current_canvas->draw_mesh(mesh, ((TBBitmapGD*)bitmap)->GetTexture());
}

void TBRendererGD::BeginPaint(CanvasItem *canvas, int render_target_w, int render_target_h)
{
	ERR_FAIL_COND_MSG(_current_canvas != nullptr, "Recursive paint call");

#ifdef TB_RUNTIME_DEBUG_INFO
	dbg_bitmap_validations = 0;
#endif

	TBRendererBatcher::BeginPaint(render_target_w, render_target_h);

	_current_canvas = canvas;
	_current_bitmap = nullptr;

	mesh->clear_mesh();
}

void TBRendererGD::EndPaint()
{
	_current_canvas = nullptr;
	_current_bitmap = nullptr;

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
	PoolVector2Array verts;
	PoolColorArray colors;
	PoolVector2Array uvs;

	for (int c = 0; c < batch->vertex_count;  c++)
	{
		verts.push_back({batch->vertex[c].x, batch->vertex[c].y});
		uvs.push_back({batch->vertex[c].u, batch->vertex[c].v});
		colors.push_back({batch->vertex[c].r/255.f, batch->vertex[c].g/255.f, batch->vertex[c].b/255.f, batch->vertex[c].a/255.f});
	}

	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = verts;
	mesh_array[VS::ARRAY_COLOR] = colors;
	mesh_array[VS::ARRAY_TEX_UV] = uvs;
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);

	// Assuming there are no calls without bitmap
	if (!_current_bitmap)
	{
		_current_bitmap = batch->bitmap;
	}

	// Flush
	if (_current_bitmap != batch->bitmap)
	{
		DrawMesh(mesh, batch->bitmap);
		mesh->clear_mesh();
		_current_bitmap = batch->bitmap;
	}
}

void TBRendererGD::SetClipRect(const TBRect &rect)
{
	VisualServer::get_singleton()->canvas_item_set_custom_rect(_current_canvas->get_canvas_item(), true, Rect2(m_clip_rect.x, m_clip_rect.y, m_clip_rect.w, m_clip_rect.h));
}

} // namespace tb
