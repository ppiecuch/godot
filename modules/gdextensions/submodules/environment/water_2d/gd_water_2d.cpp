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


_FORCE_INLINE_ static PoolByteArray create_poolarray(uint8_t *buf_ptr, int buf_size) {
	PoolByteArray data;
	data.resize(buf_size);
	memcpy(data.write().ptr(), buf_ptr, buf_size);
	return data;
}

template<typename T> T create_poolarray(size_t presize) {
	T poolarray;
	poolarray.resize(presize);
	return poolarray;
}


/// Multitexture shader

static const char *_vertex_shader = R"(
	shader canvas_item;
)";


/// Private methods

static Ref<Texture> make_texture_from_data(const uint8_t *p_data, size_t p_data_len, int p_flags, const String &p_name) {
	String fn = "user://__water2d_" + String::num(p_flags) + "_" + p_name; // cached texture
	if (ResourceLoader::exists(fn + ".tex")) {
		Ref<Texture> texture = ResourceLoader::load(fn + ".tex", "Texture");
		return texture;
	} else {
		// build texture atlas from resources
		Vector<Ref<Image>> images;
		Vector<String> names;

		Ref<Image> image = memnew(Image(p_data, p_data_len));
		Ref<ImageTexture> texture = memnew(ImageTexture);
		texture->create_from_image(image, p_flags);

		ResourceSaver::save(fn + ".tex", texture);

		return texture;
	}
}

#define IRAND() unsigned(return Math::rand())
#define TS() (OS::get_singleton()->get_ticks_usec())

static _FORCE_INLINE_  Rect2 get_tex_region(Ref<Texture> tex) {
	Ref<AtlasTexture> p_atlas = tex;
	if ( p_atlas.is_valid() ) {
		return p_atlas->get_region();
	}
	return Rect2(0,0,1,1);
}

// define WaterGrid x WaterGrid columns of fluid
WaterRipples::WaterRipples () {
	angle = 0;
	caustic_frame = anim_frame = bumpmap_frame = 0;

	p1 = &water1; // FRONT MAP
	p2 = &water2; // BACK MAP

	last_ms_time = cur_ms_time = 0;

	// allocate buffers

	sommet = new Vector3[WaterGrid][WaterGrid];          // vertices vector
	normal = new Vector3[WaterGrid][WaterGrid];          // quads normals
	snormal = new Vector3[WaterGrid][WaterGrid];         // vertices normals (average)
	snormaln = new Vector3[WaterGrid][WaterGrid];        // normalized vertices normals

	uvmap = new Vector2[WaterGrid][WaterGrid];           // background texture coordinates
	newuvmap = new Vector2[WaterGrid][WaterGrid];        // perturbated background coordinates -> refraction
	envmap = new Vector2[WaterGrid][WaterGrid];          // envmap coordinates...

	water_index.resize(WaterGrid * WaterGrid * 6); // vertex array index
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
	delete[] newuvmap;        // perturbated background coordinates -> refraction
	delete[] envmap;          // envmap coordinates...
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
void WaterRipples::random_wave () {
	(*p1)[IRAND() % WaterSize + 1][IRAND() % WaterSize + 1] -= IRAND() & 127;
}

// measure elapsed time ...
bool WaterRipples::bench (real_t p_rate) {
	cur_ms_time = TS();
	return (cur_ms_time - last_ms_time) >= p_rate; // don't run to quick otherwise it doesn't look like water
}

// update to next state of fluid model
void WaterRipples::update (void) {
	angle = (angle + 2) & 1023; // new angle for parametric curves
	new_water(); // fluid update
	smooth_water(); // smoothing
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
	std::swap(p1, p2);
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
	// vertices calculus -> we already know X and Y
	// calculus of background texture coordinates
	constexpr real_t norm_cell = 1.0 / (WaterSize - 1);
	for (int x = 1; x <= WaterSize; x++) {
		const real_t xmin = (x - WaterMilX) / 50.0;
		for (int y = 1; y <= WaterSize; y++) {
			const real_t ymin = (y - WaterMilY) / 50.0;
			sommet[(x - 1) * 2][(y - 1) * 2].x = xmin;
			sommet[(x - 1) * 2][(y - 1) * 2].y = ymin;
			uvmap[(x - 1) * 2][(y - 1) * 2][0] = (x - 1) * (1.0 / (WaterSize - 1));
			uvmap[(x - 1) * 2][(y - 1) * 2][1] = (y - 1) * (1.0 / (WaterSize - 1));
		}
	}
	// build vertices in-between
	for (int x = 0; x <= WaterGrid - 1; x += 2) { // even rows
		for (int y = 1; y <= WaterGrid - 2; y += 2)  { // odd columns
			sommet[x][y][0] = (sommet[x][y-1].x + sommet[x][y+1][0]) / 2;
			sommet[x][y][1] = (sommet[x][y-1].y + sommet[x][y+1][1]) / 2;
			// --
			maskmap[x][y][0] = uvmap[x][y][0] = (uvmap[x][y-1][0] + uvmap[x][y+1][0]) / 2;
			maskmap[x][y][1] = uvmap[x][y][1] = (uvmap[x][y-1][1] + uvmap[x][y+1][1]) / 2;
		}
	}

	// build vertices in-between
	for (int x = 1; x <= WaterGrid - 2; x += 2) { // odd rows
		for (int y = 0; y <= WaterGrid - 1; y++) { // every columns
			sommet[x][y][0] = (sommet[x-1][y].x + sommet[x+1][y][0]) / 2;
			sommet[x][y][1] = (sommet[x-1][y].y + sommet[x+1][y][1]) / 2;
			// --
			maskmap[x][y][0] = uvmap[x][y][0] = (uvmap[x-1][y][0] + uvmap[x+1][y][0]) / 2;
			maskmap[x][y][1] = uvmap[x][y][1] = (uvmap[x-1][y][1] + uvmap[x+1][y][1]) / 2;
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
	// snormal[x][y][2] = 0.01+0.01+0.01+0.01 = 0.04
	//..............................................................................................................

	constexpr int grid_size = 3 * sizeof(real_t) * WaterGrid;
	constexpr int last_index = WaterGrid - 1;

	// copy borders of the map (Z component only) for periodicity
	memcpy ((void*)&normal[last_index][0][0], (void*)&normal[last_index - 1][0][0], grid_size);

	for (int x = 0; x < WaterSize*2; x++) {
		normal[x][last_index][2] = normal[x][last_index - 1][2];
	}

	// calculate normals to vertices (Z component only)
	for (int x = 1; x < last_index; x++) {
		for (int y = 1; y < last_index; y++) {
			snormal[x][y][2] =
			normal[x - 1][y][2] + normal[x + 1][y][2] +
			normal[x][y -1][2] + normal[x][y + 1][2];
		}
	}

	// copy borders of the map (Z component only) for periodicity
	for (int x = 0; x < WaterGrid; x++) {
		snormal[x][0][2] = normal[x][0][2];
		snormal[x][last_index][2] = normal[x][last_index][2];
	}

	memcpy ((void *)&snormal[0][0][0], (void *)&normal[0][0][0], grid_size);
	memcpy ((void *)&snormal[last_index][0][0], (void *)&normal[last_index][0][0], grid_size);
}

// construction of a geometric model
void WaterRipples::build_water (void) {
	real_t h1, sqroot;

	constexpr int grid_size = sizeof(Vector3) * WaterGrid;
	constexpr int last_index = WaterGrid - 1;

	// calculate vertices: Z component
	for (int x = 1; x <= WaterSize; x++) {
		for (int y = 1; y <= WaterSize; y++) {
			if ((h1 = (smooth[x][y] / 100)) < 0) {
				h1 = 0;
			}
			sommet[(x - 1) << 1][(y - 1) << 1][2] = h1;
		}
	}
	// construct vertices in-between
	for (int x = 0; x <= last_index; x += 2) { // even rows
		for (int y = 1; y <= last_index - 1; y += 2) { // odd columns
			sommet[x][y][2] = (sommet[x][y-1][2] + sommet[x][y+1][2]) / 2;
		}
	}

	// construct vertices in-between
	for (int x = 1; x <= last_index - 1; x += 2) { // even rows
		for (int y = 0; y <= last_index; y++) { // every columns
			sommet[x][y][2] = (sommet[x-1][y][2] + sommet[x+1][y][2]) / 2;
		}
	}

	// calculate normals to faces : components X and Y
	// -> simplified cross product knowing that we have a distance of 1.0 between
	//    each fluid cells.
	for (int x = 0; x < last_index; x++) {
		for (int y = 0; y < WaterGrid - 1; y++) {
			normal[x][y][0] = 0.1 * (sommet[x][y][2] - sommet[x+1][y][2]);
			normal[x][y][1] = 0.1 * (sommet[x][y][2] - sommet[x][y+1][2]);
		}
	}

	// copy map borders(components X and Y only) for periodicity
	memcpy ((void*)&normal[last_index][0][0], (void*)&normal[last_index - 1][0][0], grid_size);

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

	// copy map borders (components X and Y only)
	for (int x = 0; x < WaterGrid; x++) {
		snormal[x][0][0] = normal[x][0][0];
		normal[x][0][1] = normal[x][0][1];
		snormal[x][last_index][0] = normal[x][last_index][0];
		snormal[x][last_index][1] = normal[x][last_index][1];
	}

	memcpy ((void*)&snormal[0][0][0], (void*)&normal[0][0][0], grid_size);
	memcpy ((void*)&snormal[last_index][0][0], (void*)&normal[last_index][0][0], grid_size);

	// calculate ourself normalization
	for (int x = 0; x < WaterGrid; x++) {
		for (int y = 0; y < WaterGrid; y++) {
			sqroot = Math::sqrt(snormal[x][y][0] * snormal[x][y][0] + snormal[x][y][1] * snormal[x][y][1] + 0.0016);
			snormaln[x][y][0] = snormal[x][y][0] / sqroot;
			snormaln[x][y][1] = snormal[x][y][1] / sqroot;
			snormaln[x][y][2] = 0.04 / sqroot;  // snormal[x][y][2] = 0.04

		}
	}

	// really simple version of a fake envmap generator
	for (int x = 0; x < WaterGrid; x++) {
		for (int y = 0; y < WaterGrid; y++) {
			// perturbate coordinates of background mapping with the components X,Y of normals...
			// simulate refraction
			newuvmap[x][y][0] = uvmap[x][y][0] + 0.05 * snormaln[x][y][0];
			newuvmap[x][y][1] = uvmap[x][y][1] + 0.05 * snormaln[x][y][1];
			// trick : xy projection of normals -> assume reflection in direction of the normals
			// looks ok for non-plane surfaces
			envmap[x][y][0] = 0.5 + snormaln[x][y][0] * 0.45;
			envmap[x][y][1] = 0.5 + snormaln[x][y][1] * 0.45;
		}
	}
}

// build strip index for vertex arrays
void WaterRipples::build_strip_index(void) {
	// array is (WaterSize * 2) x (WaterSize * 2)
	int strip_width = WaterGrid - 2; // n points define n-2 triangles in a strip
	int *water_index_ptr = water_index.write().ptr();

	// build index list
	for (int x = 0; x < strip_width; x++) { // vertical index in array
		*water_index_ptr++ = ((x + 1) * WaterGrid) + 1; // strip_width + 1 triangle strips
		for (int y = 1; y < strip_width; y++) { // horizontal index in array
			*water_index_ptr++ = (x * WaterGrid) + y;
			*water_index_ptr++ = ((x + 1) * WaterGrid) + y + 1;
		}
		*water_index_ptr++ = (x * WaterGrid) + strip_width;
	}
}

// build triangles index for vertex arrays
void WaterRipples::build_tri_index() {
	int *water_index_ptr = water_index.write().ptr();

	// build index list

	// for(int y = 0; y < WaterSize * 2 - 1; y += 2) {
	// 	for(int x = 0; x < WaterSize * 2 - 1; x += 2) {
	// 		*water_index_ptr++ = y * (WaterSize * 2) + x;
	// 		*water_index_ptr++ = y * (WaterSize * 2) + x + 2;
	// 		*water_index_ptr++ = (y + 2) * (WaterSize * 2) + x;

	// 		*water_index_ptr++ = (y + 2) * (WaterSize * 2) + x;
	// 		*water_index_ptr++ = y * (WaterSize * 2) + x + 2;
	// 		*water_index_ptr++ = (y + 2) * (WaterSize * 2) + x + 2;
	// 	}
	// }

	int l0 = 0;
	int l1 = WaterSize * 4;
	int l2 = l1;

	for(int y = 0; y < WaterSize * 2; y += 2) {
		for(int x = 0; x < WaterSize * 2; x++) { // n-2 triangles (n points in 2 array lines)
			if ((x & 1) == 1) {
				*water_index_ptr++ = l1++;
				l1++;
			} else {
				*water_index_ptr++ = l0++;
				l0++;
			}
		}

		l0 = l2;
		l1 = l2 + WaterSize * 4;
		l2 = l1;
	}
}

void WaterRipples::build_mesh(Ref<ArrayMesh> &p_mesh, const Transform *p_anim_matrix) {
	PoolVector2Array newuvanimmap = create_poolarray<PoolVector2Array>(WaterGrid);
	PoolVector2Array (*newuvcausticmap)[WaterGrid];
	PoolVector2Array (*newuvbumpmap)[WaterGrid];

	Rect2 skin_reg = get_tex_region(tex_skin);
	Rect2 region = get_tex_region(tex_mask);
	Rect2 env_reg = get_tex_region(tex_envmap);
	Rect2 bump_reg = get_tex_region(tex_bumpmap_frames[bumpmap_frame]);
	Rect2 anim_reg = get_tex_region(tex_anim_frames[anim_frame]);
	Rect2 caustic_reg = get_tex_region(tex_caust_frames[caustic_frame]);

	// normalize uv for mask texture atlas
	for (int x = 0; x < WaterGrid; x++) {
		for (int y = 0; y < WaterGrid; y++) {

		// normalize uv coordinates for texture atlas
		newuvmap[x][y].x = skin_reg.position.x + newuvmap[x][y][0]*skin_reg.size.width;
		newuvmap[x][y].y = skin_reg.position.y + newuvmap[x][y][1]*skin_reg.size.height;
		newuvbumpmap[x][y].x = bump_reg.position.x + newuvbumpmap[x][y][0]*bump_reg.size.width;
		newuvbumpmap[x][y].y = bump_reg.position.y + newuvbumpmap[x][y][1]*bump_reg.size.height;
		newuvanimmap[x][y].x = anim_reg.position.x + newuvanimmap[x][y][0]*anim_reg.size.width;
		newuvanimmap[x][y].y = anim_reg.position.y + newuvanimmap[x][y][1]*anim_reg.size.height;
		newuvcausticmap[x][y].x = caustic_reg.position.x + newuvcausticmap[x][y][0]*caustic_reg.size.width;
		newuvcausticmap[x][y].y = caustic_reg.position.y + newuvcausticmap[x][y][1]*caustic_reg.size.height;
		envmap[x][y].x = env_reg.position.x + envmap[x][y][0]*env_reg.size.width;
		envmap[x][y].y = env_reg.position.y + envmap[x][y][1]*env_reg.size.height;
		maskmap[x][y].x = region.position.x + maskmap[x][y][0]*region.size.width;
		maskmap[x][y].y = region.position.y + maskmap[x][y][1]*region.size.height;
		}
	}

	Array d;
	d.resize(VS::ARRAY_MAX);
	d[VisualServer::ARRAY_VERTEX] = verts;
	d[VisualServer::ARRAY_NORMAL] = norm;
	d[VisualServer::ARRAY_TEX_UV] = uv;
	d[VisualServer::ARRAY_INDEX] = index;

	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, d);
}

void Water2D::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_READY: {
		} break;
		case NOTIFICATION_ENTER_TREE: {
		} break;
		case NOTIFICATION_EXIT_TREE: {
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
		} break;
	}
}

void Water2D::_bind_methods() {
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "env_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
}

Water2D::Water2D() {
}
