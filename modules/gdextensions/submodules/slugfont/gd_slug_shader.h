/**************************************************************************/
/*  gd_slug_shader.h                                                      */
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

#ifndef GD_SLUG_SHADER_H
#define GD_SLUG_SHADER_H

// Godot 3.x canvas_item shader implementing Slug font rendering.
//
// Vertex layout mapping:
//   VERTEX (vec2)  = glyph quad position (screen/em-space)
//   UV (vec2)      = em-space texture coordinate (for curve evaluation)
//   UV2 (vec2)     = packed glyph data (2 floats encoding 4 uint16 indices)
//   COLOR (vec4)   = vertex color (RGBA)
//   Banding (vec4) = band transform (fetched from banding_data texture via VERTEX_ID)
//
// Textures:
//   curve_texture  = float RGBA with Bézier control points
//   band_texture   = float RGBA with band index data (converted from uint16)
//   banding_data   = float RGBA with per-vertex banding transform (indexed by VERTEX_ID)

static const char *slug_shader_code = R"shader(
shader_type canvas_item;
render_mode unshaded, blend_premul_alpha;

// Font textures (from SlugFont)
uniform sampler2D curve_texture;
uniform sampler2D band_texture;

// Per-vertex banding data (updated per text rebuild)
uniform sampler2D banding_data;
uniform int banding_width = 256;

// Varyings
varying vec4 v_color;
varying vec2 v_texcoord;
varying flat vec4 v_banding;
varying flat ivec4 v_glyph;

void vertex() {
	v_color = COLOR;
	v_texcoord = UV;

	// Decode glyph indices from UV2 (2 floats -> 4 uint16)
	uvec2 g = floatBitsToUint(UV2);
	v_glyph = ivec4(int(g.x & 0xFFFFu), int(g.x >> 16u),
	                int(g.y & 0xFFFFu), int(g.y >> 16u));

	// Fetch per-vertex banding from data texture
	int bx = VERTEX_ID % banding_width;
	int by = VERTEX_ID / banding_width;
	v_banding = texelFetch(banding_data, ivec2(bx, by), 0);
}

// --- Root code classification ---

uint _calc_root_code(float y1, float y2, float y3) {
	uint i1 = floatBitsToUint(y1) >> 31u;
	uint i2 = floatBitsToUint(y2) >> 30u;
	uint i3 = floatBitsToUint(y3) >> 29u;
	uint s = (i2 & 2u) | (i1 & ~2u);
	s = (i3 & 4u) | (s & ~4u);
	return (0x2E74u >> s) & 257u;
}

bool _test_curve(uint c) { return c != 0u; }
bool _test_root1(uint c) { return (c & 1u) != 0u; }
bool _test_root2(uint c) { return c > 1u; }

// --- Quadratic Bézier root solvers ---

vec2 _solve_horiz(vec4 p12, vec2 p3) {
	vec2 a = p12.xy - p12.zw * 2.0 + p3;
	vec2 b = p12.xy - p12.zw;
	float ra = 1.0 / a.y;
	float rb = 0.5 / b.y;
	float d = sqrt(max(b.y * b.y - a.y * p12.y, 0.0));
	float t1 = (b.y - d) * ra;
	float t2 = (b.y + d) * ra;
	if (abs(a.y) < 1.220703125e-4) { t1 = p12.y * rb; t2 = t1; }
	return vec2((a.x * t1 - b.x * 2.0) * t1 + p12.x,
	            (a.x * t2 - b.x * 2.0) * t2 + p12.x);
}

vec2 _solve_vert(vec4 p12, vec2 p3) {
	vec2 a = p12.xy - p12.zw * 2.0 + p3;
	vec2 b = p12.xy - p12.zw;
	float ra = 1.0 / a.x;
	float rb = 0.5 / b.x;
	float d = sqrt(max(b.x * b.x - a.x * p12.x, 0.0));
	float t1 = (b.x - d) * ra;
	float t2 = (b.x + d) * ra;
	if (abs(a.x) < 1.220703125e-4) { t1 = p12.x * rb; t2 = t1; }
	return vec2((a.y * t1 - b.y * 2.0) * t1 + p12.y,
	            (a.y * t2 - b.y * 2.0) * t2 + p12.y);
}

void fragment() {
	vec2 rc = v_texcoord;
	vec2 epp = fwidth(rc);
	vec2 ppe = 1.0 / epp;
	ivec2 bmax = v_glyph.zw;
	ivec2 bi = clamp(ivec2(rc * v_banding.xy + v_banding.zw), ivec2(0), bmax);
	ivec2 gl = v_glyph.xy;

	// --- Horizontal coverage ---
	float xcov = 0.0;
	float xwgt = 0.0;

	ivec4 hbd = ivec4(texelFetch(band_texture, ivec2(gl.x + bi.y, gl.y), 0));
	ivec2 hl = ivec2(gl.x + hbd.y, gl.y);
	hl.y += hl.x >> 12;
	hl.x &= 0x0FFF;

	for (int ci = 0; ci < hbd.x; ci++) {
		ivec2 cl = ivec2(texelFetch(band_texture, ivec2(hl.x + ci, hl.y), 0).xy);
		vec4 p12 = texelFetch(curve_texture, cl, 0) - vec4(rc, rc);
		vec2 p3 = texelFetch(curve_texture, ivec2(cl.x + 1, cl.y), 0).xy - rc;

		if (max(max(p12.x, p12.z), p3.x) * ppe.x < -0.5) break;

		uint code = _calc_root_code(p12.y, p12.w, p3.y);
		if (_test_curve(code)) {
			vec2 r = _solve_horiz(p12, p3) * ppe.x;
			if (_test_root1(code)) {
				xcov += clamp(r.x + 0.5, 0.0, 1.0);
				xwgt = max(xwgt, clamp(1.0 - abs(r.x) * 2.0, 0.0, 1.0));
			}
			if (_test_root2(code)) {
				xcov -= clamp(r.y + 0.5, 0.0, 1.0);
				xwgt = max(xwgt, clamp(1.0 - abs(r.y) * 2.0, 0.0, 1.0));
			}
		}
	}

	// --- Vertical coverage ---
	float ycov = 0.0;
	float ywgt = 0.0;

	ivec4 vbd = ivec4(texelFetch(band_texture, ivec2(gl.x + bmax.y + 1 + bi.x, gl.y), 0));
	ivec2 vl = ivec2(gl.x + vbd.y, gl.y);
	vl.y += vl.x >> 12;
	vl.x &= 0x0FFF;

	for (int ci = 0; ci < vbd.x; ci++) {
		ivec2 cl = ivec2(texelFetch(band_texture, ivec2(vl.x + ci, vl.y), 0).xy);
		vec4 p12 = texelFetch(curve_texture, cl, 0) - vec4(rc, rc);
		vec2 p3 = texelFetch(curve_texture, ivec2(cl.x + 1, cl.y), 0).xy - rc;

		if (max(max(p12.y, p12.w), p3.y) * ppe.y < -0.5) break;

		uint code = _calc_root_code(p12.x, p12.z, p3.x);
		if (_test_curve(code)) {
			vec2 r = _solve_vert(p12, p3) * ppe.y;
			if (_test_root1(code)) {
				ycov -= clamp(r.x + 0.5, 0.0, 1.0);
				ywgt = max(ywgt, clamp(1.0 - abs(r.x) * 2.0, 0.0, 1.0));
			}
			if (_test_root2(code)) {
				ycov += clamp(r.y + 0.5, 0.0, 1.0);
				ywgt = max(ywgt, clamp(1.0 - abs(r.y) * 2.0, 0.0, 1.0));
			}
		}
	}

	// --- Combine ---
	float cov = clamp(
		max(abs(xcov * xwgt + ycov * ywgt) / max(xwgt + ywgt, 1.220703125e-4),
		    min(abs(xcov), abs(ycov))),
		0.0, 1.0);

	float a = cov * v_color.a;
	COLOR = vec4(v_color.rgb * a, a);
}
)shader";

#endif // GD_SLUG_SHADER_H
