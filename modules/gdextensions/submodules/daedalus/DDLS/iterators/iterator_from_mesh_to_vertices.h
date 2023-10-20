/**************************************************************************/
/*  iterator_from_mesh_to_vertices.h                                      */
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

#include "data/ddls_mesh.h"
#include "data/ddls_vertex.h"

class IteratorFromMeshToVertices {
	DDLSMesh from_mesh;
	int curr_index;

	DDLSVertex result_vertex;

public:
	// can be chained, eg.: it = ...set_from_mesh()
	IteratorFromMeshToVertices &set_from_mesh(DDLSMesh p_mesh) {
		from_mesh = p_mesh;
		curr_index = 0;
		return *this;
	}

	DDLSVertex next() {
		Vector<DDLSVertex> vertices = from_mesh->get_vertices();
		do {
			if (curr_index < vertices.size()) {
				result_vertex = vertices[curr_index];
				curr_index++;
			} else {
				result_vertex = nullptr;
				break;
			}
		} while (!result_vertex->if_is_real());
		return result_vertex;
	}

	IteratorFromMeshToVertices() {}
};
