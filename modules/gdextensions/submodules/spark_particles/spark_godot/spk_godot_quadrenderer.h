/*************************************************************************/
/*  spk_godot_quadrenderer.h                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

#ifndef SPK_GODOT_QUADRENDERER_H
#define SPK_GODOT_QUADRENDERER_H

#include "spk_godot_renderer.h"

#include "Spark/Extensions/Renderers/SPK_Oriented3DRenderBehavior.h"
#include "Spark/Extensions/Renderers/SPK_QuadRenderBehavior.h"

namespace SPK {

class GodotQuadRenderer : public IGodotRenderer,
						  public QuadRenderBehavior,
						  public Oriented3DRenderBehavior {
	SPK_IMPLEMENT_OBJECT(GodotQuadRenderer)

	SPK_START_DESCRIPTION
	SPK_PARENT_ATTRIBUTES(Renderer)
	SPK_ATTRIBUTE("material", ATTRIBUTE_TYPE_STRING)
	SPK_ATTRIBUTE("texture", ATTRIBUTE_TYPE_STRING)
	SPK_ATTRIBUTE("scale", ATTRIBUTE_TYPE_FLOATS)
	SPK_ATTRIBUTE("atlasdim", ATTRIBUTE_TYPE_UINT32S)
	SPK_END_DESCRIPTION

public:
	static Ref<GodotQuadRenderer> create(real_t scaleX = 1.0, real_t scaleY = 1.0);

protected:
	virtual void innerImport(const IO::Descriptor &descriptor) override;
	virtual void innerExport(IO::Descriptor &descriptor) const override;

private:
	static const size_t NB_INDICES_PER_PARTICLE = 6;
	static const size_t NB_VERTICES_PER_PARTICLE = 4;
	mutable real_t _u0, _u1, _v0, _v1;
	Godot::PODVector<Godot::VertexElement> _elements;

	virtual RenderBuffer *attachRenderBuffer(const Group &group) const override;
	virtual void render(const Group &group, const DataSet *dataSet, RenderBuffer *renderBuffer) const override;
	virtual void computeAABB(Vector3D &AABBMin, Vector3D &AABBMax, const Group &group, const DataSet *dataSet) const override;

	mutable void (GodotQuadRenderer::*renderParticle)(const Particle &, IGodotBuffer &renderBuffer) const; // pointer to the right render method

	void renderBasic(const Particle &particle, IGodotBuffer &renderBuffer) const; // Rendering for particles with texture or no texture
	void renderRot(const Particle &particle, IGodotBuffer &renderBuffer) const; // Rendering for particles with texture or no texture and rotation
	void renderAtlas(const Particle &particle, IGodotBuffer &renderBuffer) const; // Rendering for particles with texture atlas
	void renderAtlasRot(const Particle &particle, IGodotBuffer &renderBuffer) const; // Rendering for particles with texture atlas and rotation

	GodotQuadRenderer(real_t scaleX = 1.0, real_t scaleY = 1.0);
	GodotQuadRenderer(const GodotQuadRenderer &renderer);
};

inline Ref<GodotQuadRenderer> GodotQuadRenderer::create(Godot::Context *context, real_t scaleX, real_t scaleY) {
	return SPK_NEW(GodotQuadRenderer, context, scaleX, scaleY);
}

} // namespace SPK

#endif // SPK_GODOT_QUADRENDERER_H
