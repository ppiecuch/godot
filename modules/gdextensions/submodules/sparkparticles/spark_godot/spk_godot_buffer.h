/*************************************************************************/
/*  spk_godot_buffer.h                                                   */
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

#ifndef SPK_GODOT_BUFFER_H
#define SPK_GODOT_BUFFER_H

#include "Spark/SPARK_Core.h"

#include "spk_godot_def.h"

#include "../Graphics/Geometry.h"
#include "../Graphics/IndexBuffer.h"
#include "../Graphics/VertexBuffer.h"

namespace Godot {

class IGodotBuffer : public RenderBuffer {
public:
	IGodotBuffer(Godot::Context *context, size_t nbParticles, size_t nbVerticesPerParticle, size_t nbIndicesPerParticle);
	~IGodotBuffer();

	Godot::SharedPtr<Godot::Geometry> getGeometry();
	Godot::VertexBuffer *getVertexBuffer() { return _vb; }
	Godot::IndexBuffer *getIndexBuffer() { return _ib; }

private:
	Godot::Context *_context;
	Godot::SharedPtr<Godot::Geometry> _geometry;
	Godot::SharedPtr<Godot::VertexBuffer> _vb;
	Godot::SharedPtr<Godot::IndexBuffer> _ib;
};

inline Godot::SharedPtr<Godot::Geometry> IGodotBuffer::getGeometry() {
	return _geometry;
}

} // namespace Godot

#endif // SPK_GODOT_BUFFER_H
