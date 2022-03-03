/* Godot renderer and backend */

namespace VASErin { //VASEr internal namespace

	Mesh _mesh;

	void backend::vah_draw(vertex_array_holder& vah) {

		if ( vah.count > 0) { //save some effort

			Array mesh_array;

			glVertexPointer(2, GL_FLOAT, 0, &vah.vert[0]);
			glColorPointer (4, GL_FLOAT, 0, &vah.color[0]);
			glDrawArrays (vah.glmode, 0, vah.count);
		}
	}

	void backend::polyline(const Vector2* P, Color C, float W, int length, const polyline_opt*) { //constant color and weight

		int type=0;
		if( sizeof (Vector2)==8)
			type = GL_FLOAT;

		assert(type!=0);

		glColor4f(C.r,C.g,C.b,C.a);
		glLineWidth(W);

		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glVertexPointer(2, type, 0, P);
		glDrawArrays(GL_LINE_STRIP, 0, length);
		glEnableClientState(GL_COLOR_ARRAY);

		glLineWidth(1);
	}
} //sub namespace VASErin

void renderer::init() {}

void renderer::before() {}

void renderer::after() {}
