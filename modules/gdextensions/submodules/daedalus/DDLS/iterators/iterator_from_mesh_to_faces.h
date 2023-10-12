/**************************************************************************/
/*  iterator_from_mesh_to_faces.h                                         */
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

#include "data/ddls_face.h"
#include "data/ddls_mesh.h"

class IteratorFromMeshToFaces {
	DDLSMesh from_mesh;
	int curr_index;

	DDLSFace result_face;

public:
	// can be chained, eg.: it = ...set_from_mesh()
	IteratorFromMeshToFaces &set_from_mesh(DDLSMesh value) {
		from_mesh = value;
		curr_index = 0;
		return *this;
	}

	DDLSFace next() {
		const Vector<DDLSFace> faces = from_mesh->get_faces();
		do {
			if (curr_index < faces.size()) {
				result_face = faces[curr_index];
				curr_index++;
			} else {
				result_face = nullptr;
				break;
			}
		} while (!result_face.if_is_real());
		return result_face;
	}

	IteratorFromMeshToFaces() {}
};
