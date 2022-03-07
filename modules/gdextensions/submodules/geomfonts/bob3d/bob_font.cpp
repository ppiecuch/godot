/*************************************************************************/
/*  bob_font.cpp                                                         */
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

#include "core/rid.h"
#include "core/variant.h"
#include "core/math/math_defs.h"
#include "scene/resources/mesh.h"
#include "servers/visual_server.h"

#define BOBS_X 7
#define BOBS_Y 7

char char_32[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // space
char char_33[] = { 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x08 }; // !
char char_34[] = { 0x14, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00 }; // "
char char_35[] = { 0x22, 0x7f, 0x22, 0x22, 0x22, 0x7f, 0x22 }; // #
char char_36[] = { 0x08, 0x3f, 0x40, 0x3e, 0x01, 0x7e, 0x08 }; // $
char char_37[] = { 0x61, 0x62, 0x04, 0x08, 0x10, 0x23, 0x43 }; // %
char char_38[] = { 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f }; // &
char char_39[] = { 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00 }; // '
char char_40[] = { 0x04, 0x08, 0x08, 0x08, 0x08, 0x08, 0x04 }; // (
char char_41[] = { 0x10, 0x08, 0x08, 0x08, 0x08, 0x08, 0x10 }; // )
char char_42[] = { 0x41, 0x22, 0x14, 0x7f, 0x14, 0x22, 0x41 }; // *
char char_43[] = { 0x00, 0x08, 0x08, 0x3e, 0x08, 0x08, 0x00 }; // +
char char_44[] = { 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30 }; // ,
char char_45[] = { 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00 }; // -
char char_46[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18 }; // .
char char_47[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40 }; // /
char char_48[] = { 0x7f, 0x43, 0x45, 0x49, 0x51, 0x61, 0x7f }; // 0
char char_49[] = { 0x08, 0x18, 0x28, 0x08, 0x08, 0x08, 0x1c }; // 1
char char_50[] = { 0x3e, 0x41, 0x01, 0x3e, 0x40, 0x40, 0x7f }; // 2
char char_51[] = { 0x3e, 0x41, 0x01, 0x1e, 0x01, 0x41, 0x3e }; // 3
char char_52[] = { 0x02, 0x06, 0x0a, 0x12, 0x22, 0x7f, 0x02 }; // 4
char char_53[] = { 0x7f, 0x40, 0x40, 0x7e, 0x01, 0x41, 0x3e }; // 5
char char_54[] = { 0x3f, 0x40, 0x40, 0x7e, 0x41, 0x41, 0x3e }; // 6
char char_55[] = { 0x7f, 0x41, 0x02, 0x04, 0x08, 0x10, 0x20 }; // 7
char char_56[] = { 0x3e, 0x41, 0x41, 0x3e, 0x41, 0x41, 0x3e }; // 8
char char_57[] = { 0x3e, 0x41, 0x41, 0x3f, 0x01, 0x01, 0x7e }; // 9
char char_58[] = { 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00 }; // :
char char_59[] = { 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30 }; // ;
char char_60[] = { 0x06, 0x0c, 0x18, 0x30, 0x18, 0x0c, 0x06 }; // <
char char_61[] = { 0x00, 0x00, 0x3e, 0x00, 0x3e, 0x00, 0x00 }; // =
char char_62[] = { 0x30, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x30 }; // >
char char_63[] = { 0x3e, 0x41, 0x01, 0x06, 0x0c, 0x00, 0x08 }; // ?
char char_64[] = { 0x3e, 0x41, 0x49, 0x55, 0x4f, 0x40, 0x3e }; // @
char char_65[] = { 0x3e, 0x41, 0x41, 0x7f, 0x41, 0x41, 0x41 }; // A
char char_66[] = { 0x7e, 0x41, 0x41, 0x7e, 0x41, 0x41, 0x7e }; // B
char char_67[] = { 0x3e, 0x41, 0x40, 0x40, 0x40, 0x41, 0x3e }; // C
char char_68[] = { 0x7e, 0x41, 0x41, 0x41, 0x41, 0x41, 0x7e }; // D
char char_69[] = { 0x7f, 0x41, 0x40, 0x70, 0x40, 0x41, 0x7f }; // E
char char_70[] = { 0x7f, 0x41, 0x40, 0x70, 0x40, 0x40, 0x40 }; // F
char char_71[] = { 0x3e, 0x41, 0x40, 0x47, 0x41, 0x41, 0x3e }; // G
char char_72[] = { 0x41, 0x41, 0x41, 0x7f, 0x41, 0x41, 0x41 }; // H
char char_73[] = { 0x1c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1c }; // I
char char_74[] = { 0x0e, 0x04, 0x04, 0x04, 0x04, 0x24, 0x18 }; // J
char char_75[] = { 0x41, 0x42, 0x44, 0x48, 0x54, 0x62, 0x41 }; // K
char char_76[] = { 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7f }; // L
char char_77[] = { 0x41, 0x63, 0x55, 0x49, 0x41, 0x41, 0x41 }; // M
char char_78[] = { 0x41, 0x61, 0x51, 0x49, 0x45, 0x43, 0x41 }; // N
char char_79[] = { 0x3e, 0x41, 0x41, 0x41, 0x41, 0x41, 0x3e }; // O
char char_80[] = { 0x7e, 0x41, 0x41, 0x7e, 0x40, 0x40, 0x40 }; // P
char char_81[] = { 0x3e, 0x41, 0x41, 0x41, 0x45, 0x43, 0x3f }; // Q
char char_82[] = { 0x7e, 0x41, 0x41, 0x7e, 0x44, 0x42, 0x41 }; // R
char char_83[] = { 0x3e, 0x41, 0x40, 0x3e, 0x01, 0x41, 0x3e }; // S
char char_84[] = { 0x7f, 0x49, 0x08, 0x08, 0x08, 0x08, 0x08 }; // T
char char_85[] = { 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x3e }; // U
char char_86[] = { 0x41, 0x41, 0x41, 0x41, 0x22, 0x14, 0x08 }; // V
char char_87[] = { 0x41, 0x41, 0x41, 0x41, 0x49, 0x55, 0x22 }; // W
char char_88[] = { 0x41, 0x22, 0x14, 0x08, 0x14, 0x22, 0x41 }; // X
char char_89[] = { 0x41, 0x22, 0x14, 0x08, 0x08, 0x08, 0x08 }; // Y
char char_90[] = { 0x7f, 0x42, 0x04, 0x08, 0x10, 0x21, 0x7f }; // Z

char *chars[] = {
	char_32, char_33, char_34, char_35, char_36, char_37, char_38, char_39, char_40, char_41,
	char_42, char_43, char_44, char_45, char_46, char_47, char_48, char_49, char_50, char_51,
	char_52, char_53, char_54, char_55, char_56, char_57, char_58, char_59, char_60, char_61,
	char_62, char_63, char_64, char_65, char_66, char_67, char_68, char_69, char_70, char_71,
	char_72, char_73, char_74, char_75, char_76, char_77, char_78, char_79, char_80, char_81,
	char_82, char_83, char_84, char_85, char_86, char_87, char_88, char_89, char_90, nullptr
};

static Color default_white(1,1,1,1);

static void DrawBlock(RID canvas, real_t x1, real_t y1, real_t x2, real_t y2, real_t z1, real_t z2, bool wire, const Transform2D &xform = Transform2D(), const Color &modulate = default_white) {
#define aa 0.7, 0.5, 0.2, 1.0
#define bb 0.9, 0.5, 0.0, 1.0
#define cc 1.0, 0.7, 0.0, 1.0
#define dd 1.0, 0.2, 0.2, 1.0
#define ee 1.0, 0.5, 0.8, 1.0

	struct {
		Point3 v[24];
		Color c[24];
	} _vertices = {
		{
			{ x1, y1, z1 },
			{ x2, y1, z1 },
			{ x2, y1, z2 },
			{ x1, y1, z2 },

			{ x1, y2, z1 },
			{ x1, y2, z2 },
			{ x2, y2, z2 },
			{ x2, y2, z1 },

			{ x1, y1, z1 },
			{ x1, y2, z1 },
			{ x1, y2, z2 },
			{ x1, y1, z2 },

			{ x2, y1, z1 },
			{ x2, y1, z2 },
			{ x2, y2, z2 },
			{ x2, y2, z1 },

			{ x1, y1, z1 },
			{ x2, y1, z1 },
			{ x2, y2, z1 },
			{ x1, y2, z1 },

			{ x1, y1, z2 },
			{ x2, y1, z2 },
			{ x2, y2, z2 },
			{ x1, y2, z2 }
		}, {
			{ aa },
			{ aa },
			{ aa },
			{ aa },

			{ aa },
			{ aa },
			{ aa },
			{ aa },

			{ bb },
			{ bb },
			{ bb },
			{ bb },

			{ bb },
			{ bb },
			{ bb },
			{ bb },

			{ cc },
			{ ee },
			{ cc },
			{ dd },

			{ cc },
			{ ee },
			{ cc },
			{ dd }
		}
	};
	uint32_t _faces[6][6] = {
		{  0,  1,  2,  0,  3,  2 },
		{  4,  5,  6,  4,  7,  6 },
		{  8,  9, 10,  8, 10, 11 },
		{ 12, 13, 14, 12, 14, 15 },
		{ 16, 17, 18, 16, 18, 19 },
		{ 20, 21, 22, 20, 22, 23 }
	};

	static PoolIntArray faces_index;
	if (faces_index.empty()) {
		faces_index.resize(36);
		memcpy(faces_index.write().ptr(), _faces, 36 * sizeof(uint32_t));
	}

	static PoolColorArray verts_color;
	if (verts_color.empty()) {
		verts_color.resize(24);
		memcpy(verts_color.write().ptr(), _vertices.c, 24 * sizeof(Color));
	}

	PoolVector3Array verts;
	verts.resize(24);
	memcpy(verts.write().ptr(), _vertices.v, 24 * sizeof(Point3));

	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = verts;
	mesh_array[VS::ARRAY_COLOR] = verts_color;
	mesh_array[VS::ARRAY_INDEX] = faces_index;
	Ref<ArrayMesh> _mesh = memnew(ArrayMesh);

	if (wire) {
		_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINE_LOOP, mesh_array, Array());
	} else {
		_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array, Array());
	}
	VisualServer::get_singleton()->canvas_item_add_mesh(canvas, _mesh->get_rid(), xform, modulate, RID(), RID(), RID());
}

static void DrawBob(RID canvas, real_t x, real_t y, real_t z, real_t size, bool wire, const Transform2D &xform = Transform2D(), const Color &modulate = default_white) {
	DrawBlock(canvas, x, y, x + size * 0.8, y + size * 0.8, z, z + size * 0.8, wire, xform, modulate);
}

real_t DrawBobChar(RID canvas, char ch, const Point3 &pos, real_t size, bool wire, const Transform2D &xform = Transform2D(), const Color &modulate = default_white) {
	char shft;
	real_t xp, yp = pos.y;

	if (ch < 32 || ch > 90) {
		return 0;
	}
	char *ptr = chars[ch - 32];

	for (int i = 0; i < BOBS_Y; i++) {
		xp = pos.x;
		shft = 0x40;
		for (int j = 0; j < BOBS_X; j++) {
			if ((*ptr) & shft) {
				DrawBob(canvas, xp, yp, pos.z, size, wire);
			}
			shft = shft >> 1;
			xp = xp + size;
		}
		yp = yp - size;
		ptr++;
	}
	return xp + size;
}

real_t DrawBobString(RID canvas, const char *str, const Point3 &pos, real_t size, bool wire, const Transform2D &xform = Transform2D(), const Color &modulate = default_white) {
	char ch = *str;
	Point3 xp(pos.x, 0, 0);
	while ((ch = *str) != 0) {
		DrawBobChar(canvas, ch, pos + xp, size, wire);
		xp.x = xp.x + size * (BOBS_X + 1);
		str++;
	}
	return xp.x;
}

// ---
// gluPerspective(45.0f,(real_t)w/(real_t)h,0.1f,100.0f);
// glTranslatef(0,0,-25);
// ---
// glEnable(GL_DEPTH_TEST);
// ---
// glPushMatrix();
//   glTranslatef(-3.2f,2,18);
//   glRotatef(rotation+45,1,0,0);
//   DrawBobString("GALACTIC",0,0.3f,0,0.1f);
// glPopMatrix();
// ---
// glPushMatrix();
//   glTranslatef(-3.2f,-2,18);
//   glRotatef(-rotation-45,1,0,0);
//   DrawBobString("FORTRESS",0,0.3f,0,0.1f);
// glPopMatrix();
// ---
// glDisable(GL_DEPTH_TEST);
// ---
