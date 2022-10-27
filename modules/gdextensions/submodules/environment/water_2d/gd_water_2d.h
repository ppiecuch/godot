#ifndef GD_WATER_2D_H
#define GD_WATER_2D_H

#include "core/int_types.h"
#include "core/vector.h"
#include "core/variant.h"
#include "core/os/os.h"
#include "core/math/rect2.h"
#include "core/math/random_pcg.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/texture.h"

//////////////////////////////////////////////////
// Water object ...
//////////////////////////////////////////////////

class WaterRipples
{
private:
	/* grid of 50 x 50 fluid cells */
	static const int WaterSize  = 50;
	static const int WaterGrid = WaterSize << 1;
	static const int WaterMilX = (WaterSize >> 1) + 1;
	static const int WaterMilY = (WaterSize >> 1) + 1;
	static const int WaterLarg = WaterSize >> 1;

	static const int WaterBufferSize  = (WaterSize + 2) * (WaterSize + 2);

	real_t last_ms_time;
	real_t cur_ms_time;

	bool textured;
	int angle;                                        // angle for wave generator

	int water1[WaterSize + 2][WaterSize + 2];
	int water2[WaterSize + 2][WaterSize + 2];

	int smooth[WaterSize + 2][WaterSize + 2];         // buffer used for smoothing operation

	int (*p1)[WaterSize + 2][WaterSize + 2];          // pointer FRONT
	int (*p2)[WaterSize + 2][WaterSize + 2];          // pointer BACK

	// geometric construction (static number of vertices)
	Vector3 (*sommet)[WaterGrid];                     // vertices vector
	Vector3 (*normal)[WaterGrid];                     // quads normals
	Vector3 (*snormal)[WaterGrid];                    // vertices normals (average)
	Vector3 (*snormaln)[WaterGrid];                   // normalized vertices normals
	Vector2 (*uvmap)[WaterGrid];                      // background texture coordinates
	Vector2 (*newuvmap)[WaterGrid];                   // perturbated background coordinates -> refraction
	Vector2 (*envmap)[WaterGrid];                     // envmap coordinates...
	PoolIntArray water_index;                         // vertex array index

	Ref<Texture> tex_skin, tex_mask;                   // skin/bg and mask texture
	Ref<Texture> tex_envmap;                           // texture coordinates
	Vector<Ref<Texture>> tex_anim_frames;              // subwater animation layer textures
	Vector<Ref<Texture>> tex_caust_frames;             // water causts textures
	Vector<Ref<Texture>> tex_bumpmap_frames;           // water bumpmap textures
	int anim_frame;                                    // current animation frame
	int caustic_frame;                                 // current caustic animation frame
	int bumpmap_frame;                                 // current bumpmap frame

	bool water_index_built;

	void new_water(); // fluid calculus
	void smooth_water(); // smooth filter
	void prebuild_water(); // precalculate geometric stuffs
	void build_water(); // build geometry

	void build_strip_index(); // build strip index for vertex arrays
	void build_tri_index(); // build triangles index for vertex arrays

public:
	void init();

	_FORCE_INLINE_ void set_textured(bool p_textured) { textured = p_textured; }
	_FORCE_INLINE_ bool is_textured() const { return textured; }

	void set_skin_texture(Ref<Texture> tex);
	void set_mask_texture(Ref<Texture> tex);

	void set_wave (real_t p_x, real_t p_y, int p_amp); // trace a hole in the fluid cells at normalized coords
	void run_wave (real_t p_phase, real_t p_cos, real_t p_sin, int p_amp); // some waves using parametric curves
	void random_wave ();  // random hole

	void update (); // next step in fluid model
	void build (); // build geometric model
	void build_mesh(Ref<ArrayMesh> &p_mesh, const Transform *p_anim_matrix = nullptr); // display resulting geometry (using animation matrix for animation texture)

	bool bench(real_t a_rate); // measure elapsed time ...
	_FORCE_INLINE_ void next_time() { last_ms_time = cur_ms_time; }
	_FORCE_INLINE_ void next_anim() { // next animation frame
		anim_frame = (++anim_frame) % tex_anim_frames.size();
		caustic_frame = (++caustic_frame) % tex_caust_frames.size();
	}

	_FORCE_INLINE_ int curr_anim() { return anim_frame; }

	WaterRipples();
	~WaterRipples();
};

class Water2D : public Node2D {
	GDCLASS(Water2D, Node2D);

protected:
	void _notification(int p_notification);
	static void _bind_methods();

public:
	Water2D();
};

#endif /* GD_WATER_2D_H */

// https://www.codeproject.com/Articles/1073/Interactive-water-effect
//
// Introduction
// ------------
// This application demonstrates multitexturing effects and clever math optimizations to produce an interactive water effect.
//
// In the calculations used in the code, we work with the following assumptions:
//
// * Incompressible liquid: a given mass of the liquid occupies the same volume all the time whatever the shape it takes. There is conservation of volume.
// * The surface of the liquid can be represented by grid heights, i.e. we can see the surface of the liquid as being made up
//   of a series of vertical columns of liquid. We will be able to implement that by an N x M matrix giving the height of each point
//   of surface (N being the number of points in ordinate and M the number of points in x-coordinates). This approximation is valid only for not very
//   deep containers and is tolerable as long as the forces applied to the liquid are not too significant.
//   The liquid cannot splash and a wave cannot break (not bearings).
// * The vertical component of the speed of the particles of the liquid can be ignored. The model is not valid any more for stiff slopes or chasms.
// * The horizontal component of the speed of the liquid in a vertical column is roughly constant. The movement is uniform within
//   a vertical column. This model is not valid anymore when there are turbulences.
//
// These assumptions enable us to use a simplified form of the equations of the mechanics of fluids.
//
// To simplify the reasoning, we first of all will work in 2 dimensions (the height h depends on x-coordinate x and time t):
//
// [1] du(x,t)/dt + u(x,t).du(x,t)/dx + g*dh(x,t)/dx = 0
// [2] dp(x,t)/dt + d(u(x,t)p(x,t))/dx = d(h(x,t)-b(x))/dt + p(x,t).du(x,t)/dx + u(x,t)*dp(x,t)/dx = 0
//
// where g is gravitational acceleration, h(x, t) is the height of the surface of the liquid, b(x) is the height of the bottom of the container
// (we suppose that it does not vary according to time t), p(x, t) = h(x, t)-b(x) is the depth of the liquid, and u(x, t) is the horizontal
// speed of a vertical column of liquid. The equation (1) represents the law of Newton (F=ma) which gives the movement according
// to the forces applied, and the equation (2) expresses the conservation of volume.
//
// These two equations are nonlinear (nonconstant coefficients) but if the speed of the liquid is small and if the depth varies slowly,
// we can take the following approximation (we are unaware of the terms multiplied by u(x, t) and p does not depend any more of time t):
//
// [3] du(x,t)/dt + g*dh(x,t)/dx = 0
// [4] dh(x,t)/dt + p(x)*du(x,t)/dx = 0
//
// By differentiating the first equation with respect to x and the second equation with respect to t, we obtain:
//
// [5] d^2u(x,t)/dxdt + g*d^2h(x,t)/dx^2 = 0
// [6] d^2h(x,t)/dt^2 + p(x)*d^2u(x,t)/dtdx = 0
//
// As u is a "suitable" function (its second derivatives are continuous), its second partial derivatives are equals and we can deduce
// the following single equation where u is not present:
//
// [7] d^2h(x,t)/dt^2 = g*p(x)*d2h(x,t)/dx^2
//
// The differential equation of the second member (7) is an equation of wave and expresses the heights' variation as a function of time and the x-coordinate.
//
// In 3 dimensions, the equation (7) has the following form:
//
// [8] d^2h(x,y,t)/dt^2 = g*p(x,y)*(d^2h(x,y,t)/dx^2+d^2h(x,y,t)/dy^2)
//
// To be able to work with our heights' grid, we must have a discrete formula, but this one is continuous.
// Moreover this equation is always nonlinear by the presence of p(x, y). To simplify, we will
// consider p constant, i.e. a speed of constant wave whatever the depth (we will consider a bottom of flat container,
// which will limit the "variability" of the depth). We obtain the equation then (9) to discretize:
//
// [9] d^2h(x,y,t)/dt^2 = g*p(d^2h(x,y,t)/dx^2 + d^2h(x,y,t)/dy^2)
//
// We can discretize the terms of this equation in the following way (using the method of central-differences):
//
// [10] d^2h(x,y,t)/dt^2 => (Dh(x,y,t+1) - Dh(x,y,t))/Dt^2 = (h(x,y,t+1) - h(x,y,t) - h(x,y,t) + h(x,y,t-1)) / Dt^2
//
// [11] d^2h(x,y,t)/dx2 => (Dh(x+1,y,t) - Dh(x,y,t))/Dx2 = (h(x+1,y,t) - 2h(x,y,t) + h(x-1,y,t))/Dx^2
//
// [12] d^2h(x,y,t)/dy2 => (Dh(x,y+1,t) - Dh(x,y,t))/Dy2 = (h(x,y+1,t) - 2h(x,y,t) + h(x,y-1,t))/Dy^2
//
// By posing Dr = Dx = Dy = resolution of the grid, we obtain the discrete analogue of the equation (9):
//
// [13] (h(x,y,t+1) + h(x,y,t-1) - 2h(x,y,t))/Dt^2 = g*p/Dr^2 * (h(x+1,y,t)+h(x-1,y,t)+h(x,y+1,t)+h(x,y-1,t)-4h(x,y,t))
//
// where Dt is the variation of time. We can use a more compact notation for this relation of recurrence:
//
// [14] 1/Dt^2 * [1  -2  1] h(x,y) = g*p/Dr^2 * h(t)
//
// We can then have h(x, y, t+1):
//
// [15] h(x,y,t+1) = g*p*Dt^2/Dr^2 * h(t) -h(x,y,t-1) + 2h(x,y,t)
//
// [16] h(x,y,t+1) = g*p*Dt^2/Dr^2 * h(t) -h(x,y,t-1) + 2h(x,y,t)
//
// [17] h(x,y,t+1) = g*p*Dt^2/Dr^2 * h(t) -h(x,y,t-1) + 2h(x,y,t) - 4gpDt^2/Dr^2 * h(x,y,t)
//
// [18] h(x,y,t+1) = g*p*Dt^2/Dr^2 * h(t) -h(x,y,t-1) + (2 - 4gpDt^2/Dr^2) * h(x,y,t)
//
// While setting g*p*Dt^2/Dr^2 = 1/2, we eliminate the last term, and the relation is simplified while giving us:
//
// [19] h(x,y,t+1) = 1/2 * ( h(x+1,y,t) + h(x-1,y,t) + h(x,y+1,t) + h(x,y-1,t) - h(x,y,t-1) )
//
// This relation of recurrence has a step of 2: the height in t+1 is related to heights in T and in T-1.
// We can implement that using 2 heights' matrices H1 and H2: H2[x, y] = 1/2(H1[x+1, y] + H1[x-1, y] + H1[x, y+1] + H1[x, y-1]) - H2[x, y]
//
// We can then swap the 2 matrices with each step.
//
// To calculate the new height of a point of surface costs only 4 additions, 1 subtraction and a shift-right of 1 (for division by 2).
//
// From the result obtained, we subtract 1/2n of the result (i.e. this same result right shifted of N) to have a certain damping
// (n=4 gives a rather aesthetic result, n < 4 gives more viscosity, n > 4 gives more fluidity).
//
// Let us notice that the heights of these points are signed, 0 being the neutral level of rest.
//
// From the heights' matrix, we can easily build a polygonal representation by considering each box of the grid as a quadrilateral
// (or 2 triangles) of which the heights of the 4 vertices are given by the heights of 4 adjacent boxes of the matrix.
//
// In our example, we tessellate our model with GL_TRIANGLE_STRIP and we use some multipass effects to get it realistic.
//
// First we perturb a first set of texture coordinates proportionally to vertices normals (the logo's texture coordinates).
// It looks like refraction.
//
// Code:
// ---------------------------------------------------------------------------
// -- calculate ourself normalization
//
// for(x=0; x < FLOTSIZE*2; x++)
// {
//    for(y=0; y < FLOTSIZE*2; y++)
//    {
//       sqroot = sqrt(_snormal[x][y][0]*_snormal[x][y][0] + _snormal[x][y][1]*_snormal[x][y][1] + 0.0016);
//
//       _snormaln[x][y][0] = _snormal[x][y][0]/sqroot;
//       _snormaln[x][y][1] = _snormal[x][y][1]/sqroot;
//       _snormaln[x][y][2] = 0.04 / sqroot;
//
//       -- perturbate coordinates of background
//       -- mapping with the components X,Y of normals...
//       -- simulate refraction
//
//       _newuvmap[x][y][0] = _uvmap[x][y][0] + 0.05 * _snormaln[x][y][0];
//       _newuvmap[x][y][1] = _uvmap[x][y][1] + 0.05 * _snormaln[x][y][1];
//
//    }
// }
//
// Then we calculate a second set of texture coordinates using a fake environment mapping formula
// (invariant with observer eye's position, just project normals to the xy plane)
// (the sky's texture coordinates).
//
// Code
// ---------------------------------------------------------------------------
// really simple version of a fake envmap generator
// for(x=0; x < FLOTSIZE; x++)
// {
//     for(y=0; y < FLOTSIZE; y++)
//     {
//         -- trick : xy projection of normals  ->
//         -- assume reflection in direction of the normals
//         -- looks ok for non-plane surfaces
//         _envmap[x][y][0] = 0.5 + _snormaln[x][y][0] * 0.45;
//         _envmap[x][y][1] = 0.5 + _snormaln[x][y][1] * 0.45;
//
//     }
// }
// Then mix the textures together using multitexturing and blending.
//
// Code:
// ---------------------------------------------------------------------------
// glEnable(GL_BLEND);
//
// use texture alpha-channel for blending
//
// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
// glActiveTextureARB(GL_TEXTURE0_ARB);
// glBindTexture(GL_TEXTURE_2D, 2); -- 2nd texture -> background ..
//
// glActiveTextureARB(GL_TEXTURE1_ARB);
// glBindTexture(GL_TEXTURE_2D, 1); -- 2nd texture -> envmap
// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
//
// -- enable texture mapping and specify ourself texcoords
//
// glDisable(GL_TEXTURE_GEN_S);
// glDisable(GL_TEXTURE_GEN_T);
// And to finish the stuff, tessellate using TRIANGLE_STRIPs per matrix scanline.
//
// Code:
// ---------------------------------------------------------------------------
//  for(x=0; x < strip_width; x++)
//  {
//      glBegin(GL_TRIANGLE_STRIP);
//
//      -- WARNING: glTexCoord2fv BEFORE glVertex !!!
//      glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, _newuvmap[x+1][1]);
//      glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, _envmap[x+1][1]);
//      glVertex3fv(_sommet[x+1][1]); // otherwise everything is scrolled !!!
//
//      for(y=1; y < strip_width; y++)
//      {
//          glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, _newuvmap[x][y]);
//          glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, _envmap[x][y]);
//          glVertex3fv(_sommet[x][y]);
//
//          glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, _newuvmap[x+1][y+1]);
//          glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, _envmap[x+1][y+1]);
//          glVertex3fv(_sommet[x+1][y+1]);
//      }
//
//      glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, _newuvmap[x][y]);
//      glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, _envmap[x][y]);
//      glVertex3fv(_sommet[x][y]);
//
//      glEnd();
// }
