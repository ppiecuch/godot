/*************************************************************************/
/*  vertex_array_holder.h                                                */
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

#ifndef VASER_VERTEX_ARRAY_HOLDER_H
#define VASER_VERTEX_ARRAY_HOLDER_H

struct vertex_array_holder {
	int drawmode; //drawing mode
	bool jumping;
	PoolVector2Array vert; // because it holds 2d vectors
	PoolColorArray color; // RGBA

	vertex_array_holder() {
		drawmode = Mesh::PRIMITIVE_TRIANGLES;
		jumping = false;
	}

	void set_draw_mode(int mode) { drawmode = mode; }

	void clear() {
		vert = PoolVector2Array();
		color = PoolColorArray();
	}

	void move(int a, int b) { // move b into a
		vert[a] = vert[b];
		color[a] = color[b];
	}

	void replace(int a, Point P, Color C) {
		vert[a] = P;
		color[a] = C;
	}

	int push(const Point &P, const Color &cc, bool trans = false) {
		int cur = count();
		vert.push_back(P);
		color.push_back(Color{ cc.r, cc.g, cc.b, trans ? 0 : cc.a });
		if (jumping) {
			jumping = false;
			repeat_last_push();
		}
		return cur;
	}

	void push3(const Point &P1, const Point &P2, const Point &P3, const Color &C1, const Color &C2, const Color &C3, bool trans1 = false, bool trans2 = false, bool trans3 = false) {
		push(P1, C1, trans1);
		push(P2, C2, trans2);
		push(P3, C3, trans3);
	}

	void push(const vertex_array_holder &hold) {
		if (drawmode == hold.drawmode) {
			vert.append_array(hold.vert);
			color.append_array(hold.color);
		} else if (drawmode == Mesh::PRIMITIVE_TRIANGLES && hold.drawmode == Mesh::PRIMITIVE_TRIANGLE_STRIP) {
			const int count = hold.vert.size();
			for (int b = 2; b < count; b++) {
				for (int k = 0; k < 3; k++) {
					const int B = b - 2 + k;
					vert.push_back(hold.vert[B]);
					color.push_back(hold.color[B]);
				}
			}
		} else {
			DEBUG("vertex_array_holder:push: unknown type");
		}
	}

	_FORCE_INLINE_ int count() const { return vert.size(); }
	_FORCE_INLINE_ Point get(int i) const { return vert[i]; }
	_FORCE_INLINE_ Color get_color(int b) const { return color[b]; }

	Point get_relative_end(int di = -1) { // -1 is the last one
		const int cnt = count();
		int i = cnt + di;
		if (i < 0) {
			i = 0;
		}
		if (i >= cnt) {
			i = cnt - 1;
		}
		return get(i);
	}

	void repeat_last_push() {
		const int i = count() - 1;
		push(vert[i], color[i]);
	}

	void jump() { // to make a jump in triangle strip by degenerated triangles
		if (drawmode == Mesh::PRIMITIVE_TRIANGLE_STRIP) {
			repeat_last_push();
			jumping = true;
		}
	}

	void draw() {
		backend::vah_draw(*this);
	}

	void draw_triangles() {
		Color col = { 1, 0, 0, 0.5 };
		if (drawmode == Mesh::PRIMITIVE_TRIANGLES) {
			for (int i = 0; i < count(); i++) {
				Point P[4];
				P[0] = get(i);
				i++;
				P[1] = get(i);
				i++;
				P[2] = get(i);
				P[3] = P[0];
				polyline((Vector2 *)P, col, 1, 4, 0);
			}
		} else if (drawmode == Mesh::PRIMITIVE_TRIANGLE_STRIP) {
			for (int i = 2; i < count(); i++) {
				Point P[3];
				P[0] = get(i - 2);
				P[1] = get(i);
				P[2] = get(i - 1);
				polyline((Vector2 *)P, col, 1, 3, 0);
			}
		}
	}

	void swap(vertex_array_holder &B) {
		const int hold_drawmode = drawmode;
		const bool hold_jumping = jumping;
		drawmode = B.drawmode;
		jumping = B.jumping;
		B.drawmode = hold_drawmode;
		B.jumping = hold_jumping;
		auto v = vert;
		auto c = color;
		vert = B.vert;
		color = B.color;
		B.vert = v;
		B.color = c;
	}
};

#endif // VASER_VERTEX_ARRAY_HOLDER_H
