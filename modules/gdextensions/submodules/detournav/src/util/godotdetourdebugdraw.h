/**************************************************************************/
/*  godotdetourdebugdraw.h                                                */
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

#ifndef GODOTDETOURDEBUGDRAW_H
#define GODOTDETOURDEBUGDRAW_H

#include "scene/resources/material.h"
#include "scene/resources/mesh.h"
#include "scene/resources/surface_tool.h"

#include "DebugDraw.h"

class GodotDetourDebugDraw : public duDebugDraw {
	Ref<SurfaceTool> _surfaceTool;
	Ref<SpatialMaterial> _material;
	Ref<ArrayMesh> _arrayMesh;

public:
	void setMaterial(Ref<Material> material); // Sets the material to use.

	Ref<ArrayMesh> getArrayMesh(); // Returns the ArrayMesh constructed so far.

	void clear(); // Clear all the mesh data constructed so far.

	// Custom box drawing function as Godot does not support PRIMITVE_TYPE_QUAD.
	void debugDrawBox(float minx, float miny, float minz, float maxx, float maxy, float maxz, unsigned int *fcol);

	// Interface overwrites
	virtual unsigned int areaToCol(unsigned int area);
	virtual void depthMask(bool state);
	virtual void texture(bool state);
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0);
	virtual void vertex(const float *pos, unsigned int color);
	virtual void vertex(const float x, const float y, const float z, unsigned int color);
	virtual void vertex(const float *pos, unsigned int color, const float *uv);
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v);
	virtual void end();

	GodotDetourDebugDraw();
	~GodotDetourDebugDraw();
};

inline Ref<ArrayMesh> GodotDetourDebugDraw::getArrayMesh() {
	return _arrayMesh;
}

#endif // GODOTDETOURDEBUGDRAW_H
