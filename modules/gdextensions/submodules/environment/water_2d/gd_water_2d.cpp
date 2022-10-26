#include "gd_water_2d.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <string>
#include <iostream>


// define some constants
#ifndef M_PI
# define M_PI 3.1415926535897
#endif


// define aSize X aSize columns of fluid
WaterRipples::WaterRipples (void) {
	angle = 0;
	caustic_frame = anim_frame = bumpmap_frame = 0;

	p1 = &water1;  // FRONT MAP
	p2 = &water2;  // BACK MAP

	last_ms_time = cur_ms_time = 0;

	// allocate buffers

	sommet = new Vector3[WaterGrid][WaterGrid];          // vertices vector
	normal = new Vector3[WaterGrid][WaterGrid];          // quads normals
	snormal = new Vector3[WaterGrid][WaterGrid];         // vertices normals (average)
	snormaln = new Vector3[WaterGrid][WaterGrid];        // normalized vertices normals

	uvmap = new Vector2[WaterGrid][WaterGrid];           // background texture coordinates
	maskmap = new Vector2[WaterGrid][WaterGrid];         // masking texture coordinates
	newuvmap = new Vector2[WaterGrid][WaterGrid];        // perturbated background coordinates -> refraction
	newuvanimmap = new Vector2[WaterGrid][WaterGrid];    // perturbated background animation coordinates -> refraction
	newuvcausticmap = new Vector2[WaterGrid][WaterGrid]; // perturbated caustic animation coordinates -> refraction
	newuvbumpmap = new Vector2[WaterGrid][WaterGrid];    // perturbated bump map coordinates -> refraction
	envmap = new Vector2[WaterGrid][WaterGrid];          // envmap coordinates...

	water_index = new uint16_t[WaterGrid * WaterGrid * 6]; // vertex array index
}


//-----------------------------------
// destructor
//-----------------------------------
WaterRipples::~WaterRipples (void) {
	delete[] sommet;          // vertices vector
	delete[] normal;          // quads normals
	delete[] snormal;         // vertices normals (average)
	delete[] snormaln;        // normalized vertices normals
	delete[] uvmap;           // background texture coordinates
	delete[] maskmap;         // masking texture coordinates
	delete[] newuvmap;        // perturbated background coordinates -> refraction
	delete[] newuvanimmap;    // perturbated background animation coordinates -> refraction
	delete[] newuvcausticmap; // perturbated caustic animation coordinates -> refraction
	delete[] newuvbumpmap;    // perturbated bump map coordinates -> refraction
	delete[] envmap;          // envmap coordinates...
	delete[] water_index;     // vertex array index
}

// initial conditions: every heights at zero + load textures
void WaterRipples::init () {
	memset (*p1, 0, sizeof(int) * WaterBufferSize);
	memset (*p2, 0, sizeof(int) * WaterBufferSize);

	prebuild_water(); // prebuild geometric model

	// current clock
	last_ms_time = TS();
	cur_ms_time = last_ms_time;
}

// trace a hole at normalized coordinates
void WaterRipples::set_wave (real_t p_x, real_t p_y, int p_amp) {
	int x = WaterMilX + int(2 * (p_x - 0.5) * WaterGrid);
	int y = WaterMilY + int(2 * (p_y - 0.5) * WaterGrid);

	// check periodicity
	while (x > WaterSize) {
		x -= WaterSize;
	}
	while (y > WaterSize) {
		y -= WaterSize;
	}
	while (x < 0) {
		x += WaterSize;
	}
	while (y < 0) {
		y += WaterSize;
	}

	(*p1)[x][y] -= p_amp;
}

// trace a hole following parametric curves
void WaterRipples::run_wave (real_t p_phase, real_t p_cos, real_t p_sin, int p_amp)
{
	const real_t r = (angle * M_PI) / 1024;

	// [TODO] Ripple types:
	// https://github.com/lequangios/Ripple-Cocos2dx/blob/e7e0a379ed20c2a033c802e0edc3309e7757eea5/CCRippleSprite.cpp#L290

	int x = WaterMilX + int(Math::cos(p_cos * r + p_phase) * WaterGrid);
	int y = WaterMilY + int(Math::sin(p_sin * r + p_phase) * WaterGrid);

	if (x > WaterSize) {
		x = WaterSize;
	}
	if (y > WaterSize) {
		y = WaterSize;
	}
	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		y = 0;
	}

	(*p1)[x][y] -= p_amp;
}

// trace a random hole
void WaterRipples::random_wave (void) {
	(*p1)[irand() % WaterSize + 1][irand() % WaterSize + 1] -= irand() & 127;
}

// measure elapsed time ...
bool WaterRipples::bench (real_t p_rate) {
	cur_ms_time = TS();
	return (cur_ms_time - last_ms_time) >= p_rate; // don't run to quick otherwise it doesn't look like water
}

// update to next state of fluid model
void WaterRipples::update (void) {
	angle = (angle + 2) & 1023; // new angle for parametric curves
	new_water();                // fluid update
	smooth_water();             // smoothing
	bumpmap_frame = (++bumpmap_frame) % tex_bumpmap_frames.size(); // next bumpmap frame
}

// build geometric model
void WaterRipples::build (void) {
	build_water ();
}

// physical calculus for fluid model
void WaterRipples::new_water (void) {
	// discretized differential equation
	for (int x = 1; x <= WaterSize; x++) {
		for (int y = 0; y <= WaterSize; y++) {
			(*p1)[x][y] = (((*p2)[x - 1][y] + (*p2)[x + 1][y] + (*p2)[x][y - 1] + (*p2)[x][y + 1]) >> 1) - (*p1)[x][y];
			(*p1)[x][y] -= (*p1)[x][y] >> 4;
		}
	}
	constexpr int step = WaterSize + 2;

	// copy borders to make the map periodic
	memcpy(&((*p1)[0][0]), &((*p1)[1][0]), sizeof(int) * step);
	memcpy(&((*p1)[WaterSize + 1][0]), &((*p1)[1][0]), sizeof(int) * step);

	for (int x = 0, *ptr = &((*p1)[0][0]); x < WaterSize + 1; x++, ptr += step) {
		ptr[0] = ptr[1];
		ptr[WaterSize + 1] = ptr[1];
	}

	// swap buffers t and t-1, we advance in time
	auto q = p1;
	p1 = p2;
	p2 = q;
}

// filter and smooth producted values
void WaterRipples::smooth_water (void) {
	for (int x = 1; x < WaterSize + 1; x++) {
		for (int y = 1; y < WaterSize + 1; y++) {
			smooth[x][y] = (3 * (*p1)[x][y] + 2 * (*p1)[x+1][y] + 2 * (*p1)[x][y+1] + (*p1)[x+1][y+1]) >> 3;
		}
	}
	for (int i = 1; i < 4; i++) {
		for (int x = 1; x < WaterSize + 1; x++) {
			for (int y = 1; y < WaterSize + 1; y++) {
				smooth[x][y] = (3 * smooth[x][y] + 2 * smooth[x+1][y] + 2 * smooth[x][y+1] + smooth[x+1][y+1]) >> 3;
			}
		}
	}
}

// pre-building of a geometric model
void WaterRipples::prebuild_water (void) {
	real_t xmin, ymin;

	// vertices calculus -> we already know X and Y
	// calculus of background texture coordinates
	for (int x = 1; x <= WaterSize; x++) {
		xmin = (x - WaterMilX) / 50.f;

		for (int y = 1; y <= WaterSize; y++) {
			ymin = (y - WaterMilY) / 50.f;

			sommet[(x - 1) * 2][(y - 1) * 2][0] = xmin;
			sommet[(x - 1) * 2][(y - 1) * 2][1] = ymin;
			// --
			maskmap[(x - 1) * 2][(y - 1) * 2][0] = uvmap[(x - 1) * 2][(y - 1) * 2][0] = (x - 1) * (1.0f / (WaterSize - 1));
			maskmap[(x - 1) * 2][(y - 1) * 2][1] = uvmap[(x - 1) * 2][(y - 1) * 2][1] = (y - 1) * (1.0f / (WaterSize - 1));
		}
	}
	// build vertices in-between
	for (int x = 0; x <= WaterGrid - 1; x += 2) { // even rows
		for (int y = 1; y <= WaterGrid - 2; y += 2)  { // odd columns
			sommet[x][y][0] = (sommet[x][y-1][0] + sommet[x][y+1][0]) / 2;
			sommet[x][y][1] = (sommet[x][y-1][1] + sommet[x][y+1][1]) / 2;
			maskmap[x][y][0] = uvmap[x][y][0] = (uvmap[x][y-1][0] + uvmap[x][y+1][0]) / 2;
			maskmap[x][y][1] = uvmap[x][y][1] = (uvmap[x][y-1][1] + uvmap[x][y+1][1]) / 2;
		}
	}

	// build vertices in-between
	for (int x = 1; x <= WaterGrid - 2; x += 2) { // odd rows
		for (int y = 0; y <= WaterGrid - 1; y++) { // every columns
			sommet[x][y][0] = (sommet[x-1][y][0] + sommet[x+1][y][0]) / 2;
			sommet[x][y][1] = (sommet[x-1][y][1] + sommet[x+1][y][1]) / 2;
			maskmap[x][y][0] = uvmap[x][y][0] = (uvmap[x-1][y][0] + uvmap[x+1][y][0]) / 2;
			maskmap[x][y][1] = uvmap[x][y][1] = (uvmap[x-1][y][1] + uvmap[x+1][y][1]) / 2;
		}
	}

	Rect2 region = get_tex_region(tex_mask);

	// normalize uv for mask texture atlas
	for (int x = 0; x < WaterGrid; x++) {
		for (int y = 0; y < WaterGrid; y++) {
			maskmap[x][y][0] = region.position.x + maskmap[x][y][0]*region.size.width;
			maskmap[x][y][1] = region.position.y + maskmap[x][y][1]*region.size.height;
		}
	}

	// normals to faces calculus : Z component is constant
	// -> simplified cross product and optimized knowing that we have
	//    a distance of 1.0 between each fluid cells.
	for (int x = 0; x < (WaterSize << 1) - 1; x++) {
		for (int y = 0; y < (WaterSize << 1) - 1; y++) {
			normal[x][y][2] = 0.01f;
		}
	}

	//..............................................................................................................
	// the following calculus is useless because each cell of:
	// _snormal[x][y][2] = 0.01+0.01+0.01+0.01 = 0.04
	//..............................................................................................................

	constexpr int grid_size = 3 * sizeof(real_t) * WaterGrid;
	constexpr int last_index = WaterGrid - 1;

	/* copy borders of the map (Z component only) for periodicity */
	memcpy ((char *) &m_normal[last_index][0][0], (char *) &m_normal[last_index - 1][0][0], grid_size);

	for (int x = 0; x < WaterSize*2; x++) {
		normal[x][last_index][2] = normal[x][last_index - 1][2];
	}

	/* calculate normals to vertices (Z component only) */
	for (int x = 1; x < last_index; x++) {
		for (int y = 1; y < last_index; y++) {
			snormal[x][y][2] =
			normal[x - 1][y][2] + normal[x + 1][y][2] +
			normal[x][y -1][2] + normal[x][y + 1][2];
		}
	}

	/* copy borders of the map (Z component only) for periodicity */
	for (int x = 0; x < WaterGrid; x++) {
		snormal[x][0][2] = normal[x][0][2];
		snormal[x][last_index][2] = normal[x][last_index][2];
	}

	memcpy ((char *) &snormal[0][0][0], (char *) &normal[0][0][0], grid_size);
	memcpy ((char *) &snormal[last_index][0][0], (char *) &normal[last_index][0][0], grid_size);
}

// construction of a geometric model
void WaterRipples::build_water (void) {
	real_t h1, sqroot;

	constexpr int grid_size = 3 * sizeof(real_t) * WaterGrid;
	constexpr int last_index = WaterGrid - 1;

	// calculate vertices : Z component
	for (int x = 1; x <= WaterSize; x++) {
		for (int y = 1; y <= WaterSize; y++) {
			if ((h1 = (smooth[x][y] / 100.0f)) < 0.0f) {
				h1 = 0.0f;
			}
			sommet[(x - 1) << 1][(y - 1) << 1][2] = h1;
		}
	}
	// construct vertices in-between
	for (int x = 0; x <= last_index; x += 2) { // even rows
		for (int y = 1; y <= last_index - 1; y += 2) { // odd columns
			sommet[x][y][2] = (sommet[x][y-1][2] + sommet[x][y+1][2]) / 2.0f;
		}
	}

	// construct vertices in-between
	for (int x = 1; x <= last_index - 1; x += 2) { // even rows
		for (int y = 0; y <= last_index; y++) { // every columns
			sommet[x][y][2] = (sommet[x-1][y][2] + sommet[x+1][y][2]) / 2.0f;
		}
	}


	// calculate normals to faces : components X and Y
	// -> simplified cross product knowing that we have a distance of 1.0 between
	//    each fluid cells.
	for (int x = 0; x < last_index; x++) {
		for (int y = 0; y < WaterGrid - 1; y++) {
			normal[x][y][0] = 0.1f * (sommet[x][y][2] - sommet[x+1][y][2]);
			normal[x][y][1] = 0.1f * (sommet[x][y][2] - sommet[x][y+1][2]);
		}
	}


	/* copy map borders(components X and Y only) for periodicity */

	memcpy ((char *) &m_normal[last_index][0][0], (char *) &m_normal[last_index - 1][0][0], grid_size);

	for (int x = 0; x < WaterGrid; x++) {
		normal[x][last_index][0] = normal[x][last_index - 1][0];
		normal[x][last_index][1] = normal[x][last_index - 1][1];
	}


	// calculate normals to vertices (components X and Y only)
	for (int x = 1; x < last_index; x++) {
		for (int y = 1; y < last_index; y++) {
			snormal[x][y][0] =
			normal[x - 1][y][0] + normal[x + 1][y][0] +
			normal[x][y - 1][0] + normal[x][y + 1][0];

			snormal[x][y][1] =
			normal[x - 1][y][1] + normal[x + 1][y][1] +
			normal[x][y - 1][1] + normal[x][y + 1][1];
		}
	}

	/* copy map borders (components X and Y only) */
	for (int x = 0; x < WaterGrid; x++) {
		snormal[x][0][0] = normal[x][0][0];
		normal[x][0][1] = normal[x][0][1];
		snormal[x][last_index][0] = normal[x][last_index][0];
		snormal[x][last_index][1] = normal[x][last_index][1];
	}

	memcpy ((char *) &snormal[0][0][0], (char *) &normal[0][0][0], grid_size);
	memcpy ((char *) &snormal[last_index][0][0], (char *) &normal[last_index][0][0], grid_size);


	/* calculate ourself normalization */
	for (int x = 0; x < WaterGrid; x++) {
		for (int y = 0; y < WaterGrid; y++) {
			sqroot = Math::sqrt(snormal[x][y][0] * snormal[x][y][0] + snormal[x][y][1] * snormal[x][y][1] + 0.0016);
			snormaln[x][y][0] = snormal[x][y][0] / sqroot;
			snormaln[x][y][1] = snormal[x][y][1] / sqroot;
			snormaln[x][y][2] = 0.04 / sqroot;  // snormal[x][y][2] = 0.04

		}
	}

	Rect2 skin_reg = get_tex_region(tex_skin);
	Rect2 env_reg = get_tex_region(tex_envmap);
	Rect2 bump_reg = get_tex_region(tex_bumpmap_frames[bumpmap_frame]);
	Rect2 anim_reg = get_tex_region(tex_anim_frames[anim_frame]);
	Rect2 caustic_reg = get_tex_region(tex_caust_frames[caustic_frame]);

	// really simple version of a fake envmap generator
	for (int x = 0; x < WaterGrid; x++) {
		for (int y = 0; y < WaterGrid; y++) {
			// perturbate coordinates of background mapping with the components X,Y of normals...
			// simulate refraction
			newuvcausticmap[x][y][0]
				= newuvanimmap[x][y][0]
				= newuvbumpmap[x][y][0]
				= newuvmap[x][y][0]
				= uvmap[x][y][0] + 0.05 * snormaln[x][y][0];
			newuvcausticmap[x][y][1]
				= newuvanimmap[x][y][1]
				= newuvbumpmap[x][y][1]
				= newuvmap[x][y][1]
				= uvmap[x][y][1] + 0.05 * snormaln[x][y][1];
			// normalize uv coordinates for texture atlas
			newuvmap[x][y][0] = skin_reg.position.x + newuvmap[x][y][0]*skin_reg.size.width;
			newuvmap[x][y][1] = skin_reg.position.y + newuvmap[x][y][1]*skin_reg.size.height;
			newuvbumpmap[x][y][0] = bump_reg.position.x + newuvbumpmap[x][y][0]*bump_reg.size.width;
			newuvbumpmap[x][y][1] = bump_reg.position.y + newuvbumpmap[x][y][1]*bump_reg.size.height;
			newuvanimmap[x][y][0] = anim_reg.position.x + newuvanimmap[x][y][0]*anim_reg.size.width;
			newuvanimmap[x][y][1] = anim_reg.position.y + newuvanimmap[x][y][1]*anim_reg.size.height;
			newuvcausticmap[x][y][0] = caustic_reg.position.x + newuvcausticmap[x][y][0]*caustic_reg.size.width;
			newuvcausticmap[x][y][1] = caustic_reg.position.y + newuvcausticmap[x][y][1]*caustic_reg.size.height;

			// trick : xy projection of normals -> assume reflection in direction of the normals
			// looks ok for non-plane surfaces
			envmap[x][y][0] = 0.5f + snormaln[x][y][0] * 0.45f;
			envmap[x][y][1] = 0.5f + snormaln[x][y][1] * 0.45f;
			// normalize st coords for texture atlas
			envmap[x][y][0] = env_reg.position.x + envmap[x][y][0]*env_reg.size.width;
			envmap[x][y][1] = env_reg.position.y + envmap[x][y][1]*env_reg.size.height;
		}
	}
}

// build strip index for vertex arrays
void WaterRipples::build_strip_index(void) {
	// array is (WaterSize * 2) x (WaterSize * 2)

	int strip_width = WaterGrid - 2; // n points define n-2 triangles
	uint16_t *water_index_ptr = nullptr;       // in a strip
	int x, y;

	// init pointers
	water_index_ptr = &water_index[0];

	// build index list
	for (x = 0; x < strip_width; x++) // vertical index in array
	{
		// strip_width+1 triangle strips
		*water_index_ptr++ = ((x + 1) * WaterGrid) + 1;

		for (y = 1; y < strip_width; y++) // horizontal index in array
		{
			*water_index_ptr++ = (x * WaterGrid) + y;
			*water_index_ptr++ = ((x + 1) * WaterGrid) + y + 1;
		}
		*water_index_ptr++ = (x * WaterGrid) + y;
	}
	// end build vertex array
}

// build triangles index for vertex arrays
void WaterRipples::build_tri_index(void) {
	uint16_t *water_index_ptr = &water_index[0];
	int l0, l1, l2;

#if 0
	// build index list
	for(int y = 0; y < WaterSize * 2 - 1; y += 2) {
		for(int x = 0; x < WaterSize * 2 - 1; x += 2) {
			*water_index_ptr++ = (y) * (WaterSize * 2) + x;
			*water_index_ptr++ = (y) * (WaterSize * 2) + x + 2;
			*water_index_ptr++ = (y + 2)*(WaterSize * 2) + x;

			*water_index_ptr++ = (y + 2) * (WaterSize * 2) + x;
			*water_index_ptr++ = (y) * (WaterSize * 2) + x + 2;
			*water_index_ptr++ = (y + 2) * (WaterSize * 2) + x + 2;
		}
	}
#else
	// build index list
	l0 = 0;
	l1 = (WaterSize*2)*2;
	l2 = l1;

	for(int y = 0; y < (WaterSize*2); y += 2) {
		for(int x = 0; x < (WaterSize * 2) * 2 / 2; x++) { // n-2 triangles (n points in 2 array lines)
			if ((x & 1) == 1) {
				*water_index_ptr++ = l1++;
				l1++;
			} else {
				*water_index_ptr++ = l0++;
				l0++;
			}
		}

		l0 = l2;
		l1 = l2 + (WaterSize * 2) * 2;
		l2 = l1;
	}

#endif
	// end build vertex array
}


void Water2D::_bind_methods() {
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "env_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "surface_normal", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
}

