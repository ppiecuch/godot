/*************************************************************************/
/*  ttftriangulator.cpp                                                  */
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

#include "ttftriangulator/TTF.h"

#include "ttftriangulator/TTFExceptions.cpp"
#include "ttftriangulator/TTFFont.cpp"
#include "ttftriangulator/TTFMath.cpp"
#include "ttftriangulator/TTFTriangulator2D.cpp"
#include "ttftriangulator/TTFTypes.cpp"

#include "core/vector.h"

const char *fragCodeSimple = "                                  \n\
varying vec3 tpos;                                              \n\
float round(float val)                                          \n\
{                                                               \n\
    return sign(val)*floor(abs(val)+0.5);                       \n\
}                                                               \n\
void main()                                                     \n\
{                                                               \n\
    float alpha = round((tpos.x*tpos.x-tpos.y)*tpos.z+0.5);     \n\
    gl_FragColor = alpha *vec4(1.0,1.0,1.0,1.0);                \n\
}                                                               \n\
";

const char *fragCode = "                                        \n\
varying vec3 tpos;                                              \n\
void main()                                                     \n\
{                                                               \n\
    float alpha = 1.0;                                          \n\
    if (tpos.z != 0.0)                                          \n\
    {                                                           \n\
        vec2 p = tpos.xy;                                       \n\
        // Gradients                                            \n\
        vec2 px = dFdx(p);                                      \n\
        vec2 py = dFdy(p);                                      \n\
        // Chain rule                                           \n\
        float fx = ((2.0*p.x)*px.x-px.y);                       \n\
        float fy = ((2.0*p.x)*py.x-py.y);                       \n\
        // Signed distance                                      \n\
        float dist = fx*fx + fy*fy;                             \n\
        float sd = (p.x*p.x - p.y)*tpos.z/sqrt(dist);           \n\
        // Linear alpha                                         \n\
        alpha = 0.5 - sd;                                       \n\
        if (alpha < 0.0) // Outside                             \n\
            discard;                                            \n\
        //alpha = clamp(0.5 - sd, 0.0, 1.0);                    \n\
    }                                                           \n\
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);                    \n\
}                                                               \n\
";

const char *vertCode = "                                        \n\
attribute float t;                                              \n\
attribute float c;                                              \n\
attribute vec2 pos;                                             \n\
varying vec3 tpos;                                              \n\
void main(void)                                                 \n\
{                                                               \n\
    tpos = vec3(t*0.5, max(t-1.0, 0.0), c);                     \n\
    gl_Position = gl_ModelViewProjectionMatrix*vec4(pos, 0.0, 1.0);\n\
}                                                               \n\
";

#if 0
void render_msg(const Font &f, const char *msg) {
	TTF::FontMetrics font_metrics = f.GetFontMetrics(); // will tell you about the font
	// Triangulator2DI, Triangulator2DII, Triangulator2DLinearI, Triangulator2DLinearII
	TTF::Triangulator2DI triangulator;
	for (int i = 0; i < strlen(msg); i++) {
		CodePoint cp(msg[i]);
		f.TriangulateGlyph(cp, triangulator);

		if (i > 0) {
			TTFCore::vec2t kerning = f.GetKerning(CodePoint(msg[i-1]), cp);
			glTranslatef(0.9*kerning.x*0.001, kerning.y*0.001, 0);
		}

		struct vertex_t {
			vec2f pos;
			signed char texCoord; // 0 = (0,0), 1 = (0.5,0), 2 = (1,1)
			signed char coef;     // -1 = CW edge, 0 = inner segment, +1 = CCW segment
		};
		Vector<vertex_t> verts;

		for (auto tri : triangulator) {
			TTF::vec2t v0 = triangulator[tri.i0];
			TTF::vec2t v1 = triangulator[tri.i1];
			TTF::vec2t v2 = triangulator[tri.i2];
			// store in a buffer, or do something with it from here... up to you really
			verts.push_back((vertex_t){{0.001f*v0.x, 0.001f*v0.y}, 0, static_cast<signed char>(tri.coef)});
			verts.push_back((vertex_t){{0.001f*v1.x, 0.001f*v1.y}, 1, static_cast<signed char>(tri.coef)});
			verts.push_back((vertex_t){{0.001f*v2.x, 0.001f*v2.y}, 2, static_cast<signed char>(tri.coef)});
		}

		if (verts.size()) {
			GLint loc = m_shader.attributeLocation("t");
			glEnableVertexAttribArray(loc);
			glVertexAttribPointer(loc, 1, GL_BYTE, GL_FALSE, sizeof(vertex_t), &verts[0].texCoord);
			loc = m_shader.attributeLocation("c");
			glEnableVertexAttribArray(loc);
			glVertexAttribPointer(loc, 1, GL_BYTE, GL_FALSE, sizeof(vertex_t), &verts[0].coef);
			loc = m_shader.attributeLocation("pos");
			glEnableVertexAttribArray(loc);
			glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &verts[0].pos);

			glDrawArrays(GL_TRIANGLES, 0, verts.size());
		}

		printf("%c: %d verts\n", msg[i], verts.size());
		for (int j = 0; j < verts.size(); j++)
		{
			const vertex_t &mv = verts[j];
			printf("%c %d %d, %d: (%f, %f), %d, %d\n", msg[i], j, j/3, j%3, mv.pos.x, mv.pos.y, mv.texCoord, mv.coef);
		}
	}
}
#endif

void render_msg(const TTFCore::TFont &f, const String &msg) {
	TTF::Triangulator2DI triangulator;
	for (int i = 0; i < msg.length(); i++) {
		CodePoint cp(msg.ord_at(i));
		f.TriangulateGlyph(cp, triangulator);

		TTFCore::vec2t kerning;
		if (i > 0) {
			kerning = f.GetKerning(CodePoint(msg[i - 1]), cp) * vec2t(0.9 * kerning.x * 0.001, kerning.y * 0.001);
		}

		struct vertex_t {
			vec2f pos;
			int8_t texCoord; // 0 = (0,0), 1 = (0.5,0), 2 = (1,1)
			int8_t coef; // -1 = CW edge, 0 = inner segment, +1 = CCW segment
		};
		Vector<vertex_t> verts;

		for (auto tri : triangulator) {
			TTF::vec2t v0 = triangulator[tri.i0];
			TTF::vec2t v1 = triangulator[tri.i1];
			TTF::vec2t v2 = triangulator[tri.i2];

			verts.push_back({ { 0.001f * v0.x + kerning.x, 0.001f * v0.y + kerning.y }, 0, static_cast<int8_t>(tri.coef) });
			verts.push_back({ { 0.001f * v1.x + kerning.x, 0.001f * v1.y + kerning.y }, 1, static_cast<int8_t>(tri.coef) });
			verts.push_back({ { 0.001f * v2.x + kerning.x, 0.001f * v2.y + kerning.y }, 2, static_cast<int8_t>(tri.coef) });
		}

		if (verts.size()) {
			// build a mesh
		}

#ifdef DEBUG_ENABLED
		printf("%c: %d verts\n", msg[i], verts.size());
		for (int j = 0; j < verts.size(); j++) {
			const vertex_t &mv = verts[j];
			printf("  %c %d %d, %d: (%f, %f), %d, %d\n", msg[i], j, j / 3, j % 3, mv.pos.x, mv.pos.y, mv.texCoord, mv.coef);
		}
#endif
	}
}
