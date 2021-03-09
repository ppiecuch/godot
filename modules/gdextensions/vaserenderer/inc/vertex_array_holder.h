/*************************************************************************/
/*  vertex_array_holder.h                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

class vertex_array_holder {
public:
	int count; //counter
	int drawmode; //drawing mode
	bool jumping;
	std::vector<real_t> vert; //because it holds 2d vectors
	std::vector<float> color; //RGBA

	vertex_array_holder() {

		count = 0;
		drawmode = Mesh::PRIMITIVE_TRIANGLES;
		jumping = false;
	}

	void set_draw_draw_mode(int mode) {

		drawmode = mode;
	}

	void clear() {

		count = 0;
	}
	void move(int a, int b) { //move b into a

		vert[a * 2] = vert[b * 2];
		vert[a * 2 + 1] = vert[b * 2 + 1];

		color[a * 4] = color[b * 4];
		color[a * 4 + 1] = color[b * 4 + 1];
		color[a * 4 + 2] = color[b * 4 + 2];
		color[a * 4 + 3] = color[b * 4 + 3];
	}
	void replace(int a, Point P, Color C) {

		vert[a * 2] = P.x;
		vert[a * 2 + 1] = P.y;

		color[a * 4] = C.r;
		color[a * 4 + 1] = C.g;
		color[a * 4 + 2] = C.b;
		color[a * 4 + 3] = C.a;
	}

	int push(const Point &P, const Color &cc, bool trans = false) {

		const int cur = count;
		vert.push_back(P.x);
		vert.push_back(P.y);
		color.push_back(cc.r);
		color.push_back(cc.g);
		color.push_back(cc.b);
		color.push_back(trans ? 0 : cc.a);

		count++;
		if (jumping) {
			jumping = false;
			repeat_last_push();
		}
		return cur;
	}

	void push3(const Point &P1, const Point &P2, const Point &P3,
			const Color &C1, const Color &C2, const Color &C3,
			bool trans1 = 0, bool trans2 = 0, bool trans3 = 0) {
		push(P1, C1, trans1);
		push(P2, C2, trans2);
		push(P3, C3, trans3);
	}

	void push(const vertex_array_holder &hold) {

		if (drawmode == hold.drawmode) {

			count += hold.count;
			vert.insert(vert.end(), hold.vert.begin(), hold.vert.end());
			color.insert(color.end(), hold.color.begin(), hold.color.end());
		} else if (drawmode == Mesh::PRIMITIVE_TRIANGLES &&
				   hold.drawmode == Mesh::PRIMITIVE_TRIANGLES_STRIP) {
			int &a = count;
			for (int b = 2; b < hold.count; b++) {
				for (int k = 0; k < 3; k++, a++) {
					const int B = b - 2 + k;
					vert.push_back(hold.vert[B * 2]);
					vert.push_back(hold.vert[B * 2 + 1]);
					color.push_back(hold.color[B * 4]);
					color.push_back(hold.color[B * 4 + 1]);
					color.push_back(hold.color[B * 4 + 2]);
					color.push_back(hold.color[B * 4 + 3]);
				}
			}
		} else {
			DEBUG("vertex_array_holder:push: unknown type\n");
		}
	}

	Point get(int i) {

		Point P;
		P.x = vert[i * 2];
		P.y = vert[i * 2 + 1];
		return P;
	}

	Color get_color(int b) {

		Color C;
		C.r = color[b * 4];
		C.g = color[b * 4 + 1];
		C.b = color[b * 4 + 2];
		C.a = color[b * 4 + 3];
		return C;
	}

	Point get_relative_end(int di = -1) { //di=-1 is the last one
		int i = count + di;
		if (i < 0) i = 0;
		if (i >= count) i = count - 1;
		return get(i);
	}

	void repeat_last_push() {

		Point P;
		Color cc;

		const int i = count - 1;

		P.x = vert[i * 2];
		P.y = vert[i * 2 + 1];
		cc.r = color[i * 4];
		cc.g = color[i * 4 + 1];
		cc.b = color[i * 4 + 2];
		cc.a = color[i * 4 + 3];

		push(P, cc);
	}

	void jump() { //to make a jump in triangle strip by degenerated triangles

		if (drawmode == Mesh::PRIMITIVE_TRIANGLES_STRIP) {

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

			for (int i = 0; i < count; i++) {

				Point P[4];
				P[0] = get(i);
				i++;
				P[1] = get(i);
				i++;
				P[2] = get(i);
				P[3] = P[0];
				polyline((Vec2 *)P, col, 1.0, 4, 0);
			}
		} else if (drawmode == Mesh::PRIMITIVE_TRIANGLES_STRIP) {

			for (int i = 2; i < count; i++) {

				Point P[3];
				P[0] = get(i - 2);
				P[1] = get(i);
				P[2] = get(i - 1);
				polyline((Vec2 *)P, col, 1.0, 3, 0);
			}
		}
	}

	void swap(vertex_array_holder &B) {

		int hold_count = count;
		int hold_drawmode = drawmode;
		bool hold_jumping = jumping;
		count = B.count;
		drawmode = B.drawmode;
		jumping = B.jumping;
		B.count = hold_count;
		B.drawmode = hold_drawmode;
		B.jumping = hold_jumping;
		vert.swap(B.vert);
		color.swap(B.color);
	}
};

#endif // VASER_VERTEX_ARRAY_HOLDER_H
