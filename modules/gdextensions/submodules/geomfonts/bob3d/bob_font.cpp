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

#include "core/math/math_defs.h"
#include "core/rid.h"
#include "core/variant.h"
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

struct bob_mesh {
	PoolVector3Array verts;
	PoolColorArray verts_color;
	PoolIntArray faces_index;
	bool wire;

	bob_mesh(int bnum, bool wire) :
			wire(wire) {
		faces_index.resize(bnum * (wire ? 24 : 36));
		verts_color.resize(bnum * 24);
		verts.resize(bnum * 24);
	}
};

static void draw_block(bob_mesh &mesh_info, int array_offset, real_t x1, real_t y1, real_t x2, real_t y2, real_t z1, real_t z2) {
#define aa 0.7, 0.5, 0.2, 1.0
#define bb 0.9, 0.5, 0.0, 1.0
#define cc 1.0, 0.7, 0.0, 1.0
#define dd 1.0, 0.2, 0.2, 1.0
#define ee 1.0, 0.5, 0.8, 1.0

	// clang-format off
	Point3 _vertices[24] = {
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
	};

	static Color _colors[24] = {
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
	};

	const uint32_t vert_offset = array_offset * 24;
	uint32_t _tri_faces[6][6] = {
		{ vert_offset + 0, vert_offset + 1, vert_offset + 2, vert_offset + 0, vert_offset + 3, vert_offset + 2 },
		{ vert_offset + 4, vert_offset + 5, vert_offset + 6, vert_offset + 4, vert_offset + 7, vert_offset + 6 },
		{ vert_offset + 8, vert_offset + 9, vert_offset + 10, vert_offset + 8, vert_offset + 10, vert_offset + 11 },
		{ vert_offset + 12, vert_offset + 13, vert_offset + 14, vert_offset + 12, vert_offset + 14, vert_offset + 15 },
		{ vert_offset + 16, vert_offset + 17, vert_offset + 18, vert_offset + 16, vert_offset + 18, vert_offset + 19 },
		{ vert_offset + 20, vert_offset + 21, vert_offset + 22, vert_offset + 20, vert_offset + 22, vert_offset + 23 }
	};
	uint32_t _wire_faces[14][2] = {
		{ vert_offset + 16, vert_offset + 20 },
		{ vert_offset + 17, vert_offset + 21 },

		{ vert_offset + 18, vert_offset + 22 },
		{ vert_offset + 19, vert_offset + 23 },

		{ vert_offset + 16, vert_offset + 18 },
		{ vert_offset + 21, vert_offset + 23 },

		{ vert_offset + 16, vert_offset + 17 },
		{ vert_offset + 17, vert_offset + 18 },
		{ vert_offset + 18, vert_offset + 19 },
		{ vert_offset + 19, vert_offset + 20 },

		{ vert_offset + 20, vert_offset + 21 },
		{ vert_offset + 21, vert_offset + 22 },
		{ vert_offset + 22, vert_offset + 23 },
		{ vert_offset + 23, vert_offset + 24 }
	};
	// clang-format on

	memcpy(mesh_info.verts.write().ptr() + vert_offset, _vertices, 24 * sizeof(Point3));
	memcpy(mesh_info.verts_color.write().ptr() + vert_offset, _colors, 24 * sizeof(Color));
	if (mesh_info.wire) {
		memcpy(mesh_info.faces_index.write().ptr() + array_offset * 24, _wire_faces, 24 * sizeof(uint32_t));
	} else {
		memcpy(mesh_info.faces_index.write().ptr() + array_offset * 36, _tri_faces, 36 * sizeof(uint32_t));
	}
}

static void draw_bob(bob_mesh &mesh_info, int array_offset, real_t x, real_t y, real_t z, real_t size) {
	draw_block(mesh_info, array_offset, x, y, x + size * 0.8, y + size * 0.8, z, z + size * 0.8);
}

static bool draw_bob_char(bob_mesh &mesh_info, int &array_offset, char ch, const Point3 &pos, real_t size) {
	real_t yp = pos.y;

	if (ch < 32 || ch > 90) {
		return false;
	}
	char *ptr = chars[ch - 32];

	for (int i = 0; i < BOBS_Y; i++) {
		real_t xp = pos.x;
		char shft = 0x40;
		for (int j = 0; j < BOBS_X; j++) {
			if (*ptr & shft) {
				draw_bob(mesh_info, array_offset, xp, yp, pos.z, size);
				array_offset++;
			}
			shft = shft >> 1;
			xp += size;
		}
		yp += size;
		ptr++;
	}
	return true;
}

static int num_of_draw_bobs(const char *str) {
	int bobs = 0;
	char ch = *str;
	while ((ch = *str) != 0) {
		if (ch >= 32 && ch <= 90) {
			char *ptr = chars[ch - 32];
			for (int i = 0; i < BOBS_Y; i++) {
				char shft = 0x40;
				for (int j = 0; j < BOBS_X; j++) {
					if (*ptr & shft) {
						bobs++;
					}
					shft = shft >> 1;
				}
				ptr++;
			}
		}
		str++;
	}
	return bobs;
}

real_t DrawBobString(Ref<ArrayMesh> &mesh, RID canvas, const char *str, const Point3 &pos, real_t size, bool wire) {
	const int bnum = num_of_draw_bobs(str);

	bob_mesh mesh_info(bnum, wire);

	char ch = *str;
	int array_offset = 0;
	Point3 xp(pos.x, 0, 0);
	while ((ch = *str) != 0) {
		if (draw_bob_char(mesh_info, array_offset, ch, pos + xp, size)) {
			xp.x += size * (BOBS_X + 1);
		}
		str++;
	}

	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = mesh_info.verts;
	mesh_array[VS::ARRAY_COLOR] = mesh_info.verts_color;
	mesh_array[VS::ARRAY_INDEX] = mesh_info.faces_index;

	if (wire) {
		mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, mesh_array, Array());
	} else {
		mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array, Array());
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
