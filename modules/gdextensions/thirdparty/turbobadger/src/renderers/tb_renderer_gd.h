// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#ifndef TB_RENDERER_GD_H
#define TB_RENDERER_GD_H

#include "tb_types.h"

#include "core/reference.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"

#include "renderers/tb_renderer_batcher.h"

namespace tb {

class TBRendererGD;

class TBBitmapGD : public TBBitmap
{
public:
	TBBitmapGD(TBRendererGD *renderer);
	~TBBitmapGD();
	bool Init(int width, int height, uint32 *data);
	virtual int Width() { return m_texture->get_width(); }
	virtual int Height() { return m_texture->get_height(); }
	virtual void SetData(uint32 *data);
public:
	TBRendererGD *m_renderer;
	Ref<Texture> m_texture;
};

class TBRendererGD : public Reference, public TBRendererBatcher
{
	Ref<ArrayMesh> mesh;
public:
	TBRendererGD();

	// == TBRenderer ====================================================================

	virtual void BeginPaint(int render_target_w, int render_target_h);
	virtual void EndPaint();

	virtual TBBitmap *CreateBitmap(int width, int height, uint32 *data);

	// == TBRendererBatcher ===============================================================

	virtual void RenderBatch(Batch *batch);
	virtual void SetClipRect(const TBRect &rect);
};

} // namespace tb

#endif // TB_RENDERER_GD_H
