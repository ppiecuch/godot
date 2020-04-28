#include "water_node_2d.h"

// C libs
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// C++ libs
#include <string>
#include <iostream>


// define some constants
#ifndef M_PI
# define M_PI 3.1415926535897
#endif


// define aSize X aSize columns of fluid
GdWater::GdWater (void)
{
    // initialize some fields
    m_angle = 0;

    // initial conditions
    m_p1 = &m_Water1;  // FRONT MAP
    m_p2 = &m_Water2;  // BACK MAP

    m_last_msTime = m_cur_msTime = 0.0f;

    // allocate buffers

    // geometric construction (static number of vertices)
    m_sommet = new float[WATER_GRID][WATER_GRID][3];          // vertices vector
    m_normal = new float[WATER_GRID][WATER_GRID][3];          // quads normals
    m_snormal = new float[WATER_GRID][WATER_GRID][3];         // vertices normals (average)
    m_snormaln = new float[WATER_GRID][WATER_GRID][3];        // normalized vertices normals

    m_uvmap = new float[WATER_GRID][WATER_GRID][2];           // background texture coordinates
    m_maskmap = new float[WATER_GRID][WATER_GRID][2];         // masking texture coordinates
    m_newuvmap = new float[WATER_GRID][WATER_GRID][2];        // perturbated background coordinates -> refraction
    m_newuvanimmap = new float[WATER_GRID][WATER_GRID][2];    // perturbated background animation coordinates -> refraction
    m_newuvcausticmap = new float[WATER_GRID][WATER_GRID][2]; // perturbated caustic animation coordinates -> refraction
    m_newuvbumpmap = new float[WATER_GRID][WATER_GRID][2];    // perturbated bump map coordinates -> refraction
    m_envmap = new float[WATER_GRID][WATER_GRID][2];          // envmap coordinates...

    m_water_index = new uint16_t[WATER_GRID * WATER_GRID * 6]; // vertex array index

    m_caustic_frame = m_anim_frame = m_bumpmap_frame = 0;
}


//-----------------------------------
// destructor
//-----------------------------------
GdWater::~GdWater (void)
{

    // free buffers
    // geometric construction (static number of vertices)
    delete[] m_sommet;          // vertices vector
    delete[] m_normal;          // quads normals
    delete[] m_snormal;         // vertices normals (average)
    delete[] m_snormaln;        // normalized vertices normals
    delete[] m_uvmap;           // background texture coordinates
    delete[] m_maskmap;         // masking texture coordinates
    delete[] m_newuvmap;        // perturbated background coordinates -> refraction
    delete[] m_newuvanimmap;    // perturbated background animation coordinates -> refraction
    delete[] m_newuvcausticmap; // perturbated caustic animation coordinates -> refraction
    delete[] m_newuvbumpmap;    // perturbated bump map coordinates -> refraction
    delete[] m_envmap;          // envmap coordinates...
    delete[] m_water_index;     // vertex array index
}

// initial conditions: every heights at zero + load textures
void
GdWater::init ()
{
    memset ((unsigned char *) (*m_p1), 0, sizeof (int) * (WATER_SIZE + 2) * (WATER_SIZE + 2));
    memset ((unsigned char *) (*m_p2), 0, sizeof (int) * (WATER_SIZE + 2) * (WATER_SIZE + 2));

    prebuild_water();                    // prebuild geometric model

    // current clock
    m_last_msTime = TS();
    m_cur_msTime = m_last_msTime + 100.0f;
}

// trace a hole at normalized coordinates
void GdWater::set_wave (float a_X, float a_Y, int a_Val)
{
    int x = WATER_MILX + (int) (2.0f * (a_X - 0.5f) * WATER_GRID);
    int y = WATER_MILY + (int) (2.0f * (a_Y - 0.5f) * WATER_GRID);

    // check periodicity
    while (x > WATER_SIZE)
        x -= WATER_SIZE;
    while (y > WATER_SIZE)
        y -= WATER_SIZE;
    while (x < 0)
        x += WATER_SIZE;
    while (y < 0)
        y += WATER_SIZE;

    (*m_p1)[x][y] -= a_Val;
}

// trace a hole following parametric curves
void GdWater::run_wave (float a_Phase, float a_Cos, float a_Sin, int a_Val)
{
    float r = (m_angle * M_PI) / 1024.0f;

    int x = WATER_MILX + ((int) (cosf (a_Cos * r + a_Phase) * WATER_GRID));
    int y = WATER_MILY + ((int) (sinf (a_Sin * r + a_Phase) * WATER_GRID));

    if (x > WATER_SIZE)
        x = WATER_SIZE;
    if (y > WATER_SIZE)
        y = WATER_SIZE;
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;

    (*m_p1)[x][y] -= a_Val;
}

// trace a random hole
void GdWater::random_wave (void)
{
    (*m_p1)[irand() % WATER_SIZE + 1][irand() % WATER_SIZE + 1] -= (irand()&127);
}

// measure elapsed time ...
bool GdWater::bench (float a_Rate)
{
    m_cur_msTime = TS();
    return (m_cur_msTime - m_last_msTime) >= a_Rate;    // don't run to quick otherwise it doesn't look like water
}

// update to next state of fluid model
void GdWater::update (void)
{
    m_angle = (m_angle + 2) & 1023;   // new angle for parametric curves
    new_water ();                     // fluid update
    smooth_water ();                  // smoothing
    m_bumpmap_frame = (++m_bumpmap_frame)%m_TEX_bumpmap_frames.size(); // next bumpmap frame
}

// build geometric model
void GdWater::build (void)
{
    build_water ();
}

// physical calculus for fluid model
void GdWater::new_water (void)
{
    int x, y, step;
    int *ptr;

    int (*q)[WATER_SIZE + 2][WATER_SIZE + 2];

    // discretized differential equation
    for (x = 1; x <= WATER_SIZE; x++) {
        for (y = 0; y <= WATER_SIZE; y++) {
            (*m_p1)[x][y] = (((*m_p2)[x - 1][y] + (*m_p2)[x + 1][y] + (*m_p2)[x][y - 1] + (*m_p2)[x][y + 1]) >> 1) - (*m_p1)[x][y];
            (*m_p1)[x][y] -= (*m_p1)[x][y] >> 4;
        }
    }

    // copy borders to make the map periodic
    memcpy (&((*m_p1)[0][0]), &((*m_p1)[1][0]), sizeof (int) * (WATER_SIZE + 2));
    memcpy (&((*m_p1)[WATER_SIZE + 1][0]), &((*m_p1)[1][0]), sizeof (int) * (WATER_SIZE + 2));

    step = (WATER_SIZE + 2);

    for (x = 0, ptr = &((*m_p1)[0][0]); x < (WATER_SIZE + 1); x++, ptr += step) {
        ptr[0] = ptr[1];
        ptr[WATER_SIZE + 1] = ptr[1];
    }

    /* swap buffers t and t-1, we advance in time */
    q = m_p1;
    m_p1 = m_p2;
    m_p2 = q;
}

// filter and smooth producted values
void GdWater::smooth_water (void)
{
    int x, y, i;

    for (x = 1; x < WATER_SIZE + 1; x++) {
        for (y = 1; y < WATER_SIZE + 1; y++) {
            m_Smooth[x][y] = (3*(*m_p1)[x][y] + 2*(*m_p1)[x+1][y] + 2*(*m_p1)[x][y+1] + (*m_p1)[x+1][y+1]) >> 3;
        }
    }
    for (i = 1; i < 4; i++) {
        for (x = 1; x < WATER_SIZE + 1; x++) {
            for (y = 1; y < WATER_SIZE + 1; y++) {
                m_Smooth[x][y] = (3*m_Smooth[x][y] + 2*m_Smooth[x+1][y] + 2*m_Smooth[x][y+1] + m_Smooth[x+1][y+1]) >> 3;
            }
        }
    }
}

// pre-building of a geometric model
void GdWater::prebuild_water (void)
{
    float xmin, ymin;
    int x, y;

    /* vertices calculus -> we already know X and Y */
    /* calculus of background texture coordinates */
    for (x = 1; x <= WATER_SIZE; x++)
    {
        xmin = (x - WATER_MILX) / 50.f;

        for (y = 1; y <= WATER_SIZE; y++)
        {
            ymin = (y - WATER_MILY) / 50.f;

            m_sommet[(x - 1) * 2][(y - 1) * 2][0] = xmin;
            m_sommet[(x - 1) * 2][(y - 1) * 2][1] = ymin;
            // --
            m_maskmap[(x - 1) * 2][(y - 1) * 2][0] = m_uvmap[(x - 1) * 2][(y - 1) * 2][0] = (x - 1) * (1.0f / (WATER_SIZE - 1));
            m_maskmap[(x - 1) * 2][(y - 1) * 2][1] = m_uvmap[(x - 1) * 2][(y - 1) * 2][1] = (y - 1) * (1.0f / (WATER_SIZE - 1));
        }
    }
    // build vertices in-between
    for (x = 0; x <= WATER_GRID - 1; x += 2) // even rows
    {
        for (y = 1; y <= WATER_GRID - 2; y += 2)  // odd columns
        {
            m_sommet[x][y][0] = (m_sommet[x][y-1][0] + m_sommet[x][y+1][0]) / 2.0f;
            m_sommet[x][y][1] = (m_sommet[x][y-1][1] + m_sommet[x][y+1][1]) / 2.0f;
            m_maskmap[x][y][0] = m_uvmap[x][y][0] = (m_uvmap[x][y-1][0] + m_uvmap[x][y+1][0]) / 2.0f;
            m_maskmap[x][y][1] = m_uvmap[x][y][1] = (m_uvmap[x][y-1][1] + m_uvmap[x][y+1][1]) / 2.0f;
        }
    }

    // build vertices in-between
    for (x = 1; x <= WATER_GRID - 2; x += 2) // odd rows
    {
        for (y = 0; y <= WATER_GRID - 1; y++) // every columns
        {
            m_sommet[x][y][0] = (m_sommet[x-1][y][0] + m_sommet[x+1][y][0]) / 2.0f;
            m_sommet[x][y][1] = (m_sommet[x-1][y][1] + m_sommet[x+1][y][1]) / 2.0f;
            m_maskmap[x][y][0] = m_uvmap[x][y][0] = (m_uvmap[x-1][y][0] + m_uvmap[x+1][y][0]) / 2.0f;
            m_maskmap[x][y][1] = m_uvmap[x][y][1] = (m_uvmap[x-1][y][1] + m_uvmap[x+1][y][1]) / 2.0f;
        }
    }

    Rect2 region = get_tex_region(m_TEX_mask);

    // normalize uv for mask texture atlas
    for (x = 0; x < WATER_GRID; x++) {
        for (y = 0; y < WATER_GRID; y++) {
            m_maskmap[x][y][0] = region.position.x + m_maskmap[x][y][0]*region.size.width;
            m_maskmap[x][y][1] = region.position.y + m_maskmap[x][y][1]*region.size.height;
        }
    }

    // normals to faces calculus : Z component is constant
    // -> simplified cross product and optimized knowing that we have
    //    a distance of 1.0 between each fluid cells.
    for (x = 0; x < (WATER_SIZE << 1) - 1; x++) {
        for (y = 0; y < (WATER_SIZE << 1) - 1; y++) {
            m_normal[x][y][2] = 0.01f;
        }
    }

    //..............................................................................................................
    // the following calculus is useless because each cell of:
    // _snormal[x][y][2] = 0.01+0.01+0.01+0.01 = 0.04
    //..............................................................................................................

    constexpr int grid_size = 3 * sizeof(float) * WATER_GRID;
    constexpr int last_index = WATER_GRID - 1;

    /* copy borders of the map (Z component only) for periodicity */
    memcpy ((char *) &m_normal[last_index][0][0], (char *) &m_normal[last_index - 1][0][0], grid_size);

    for (x = 0; x < WATER_SIZE*2; x++) {
        m_normal[x][last_index][2] = m_normal[x][last_index - 1][2];
    }

    /* calculate normals to vertices (Z component only) */
    for (x = 1; x < last_index; x++) {
        for (y = 1; y < last_index; y++) {
            m_snormal[x][y][2] =
            m_normal[x - 1][y][2] + m_normal[x + 1][y][2] + 
            m_normal[x][y -1][2] + m_normal[x][y + 1][2];
        }
    }

    /* copy borders of the map (Z component only) for periodicity */
    for (x = 0; x < WATER_GRID; x++) {
        m_snormal[x][0][2] = m_normal[x][0][2];
        m_snormal[x][last_index][2] = m_normal[x][last_index][2];
    }

    memcpy ((char *) &m_snormal[0][0][0], (char *) &m_normal[0][0][0], grid_size);
    memcpy ((char *) &m_snormal[last_index][0][0], (char *) &m_normal[last_index][0][0], grid_size);
}

// construction of a geometric model
void GdWater::build_water (void)
{
    float h1, sqroot;
    int x, y;

    constexpr int grid_size = 3 * sizeof(float) * WATER_GRID;
    constexpr int last_index = WATER_GRID - 1;

    /* calculate vertices : Z component */
    for (x = 1; x <= WATER_SIZE; x++) {
        for (y = 1; y <= WATER_SIZE; y++) {
            if ((h1 = (m_Smooth[x][y] / 100.0f)) < 0.0f) {
                h1 = 0.0f;
            }
            m_sommet[(x - 1) << 1][(y - 1) << 1][2] = h1;
        }
    }
    // construct vertices in-between
    for (x = 0; x <= last_index; x += 2) { // even rows
        for (y = 1; y <= last_index - 1; y += 2) { // odd columns
            m_sommet[x][y][2] = (m_sommet[x][y-1][2] + m_sommet[x][y+1][2]) / 2.0f;
        }
    }

    // construct vertices in-between
    for (x = 1; x <= last_index - 1; x += 2) { // even rows
        for (y = 0; y <= last_index; y++) { // every columns
            m_sommet[x][y][2] = (m_sommet[x-1][y][2] + m_sommet[x+1][y][2]) / 2.0f;
        }
    }


    // calculate normals to faces : components X and Y
    // -> simplified cross product knowing that we have a distance of 1.0 between
    //    each fluid cells.
    for (x = 0; x < last_index; x++) {
        for (y = 0; y < WATER_GRID - 1; y++) {
            m_normal[x][y][0] = 0.1f * (m_sommet[x][y][2] - m_sommet[x+1][y][2]);
            m_normal[x][y][1] = 0.1f * (m_sommet[x][y][2] - m_sommet[x][y+1][2]);
        }
    }


    /* copy map borders(components X and Y only) for periodicity */

    memcpy ((char *) &m_normal[last_index][0][0], (char *) &m_normal[last_index - 1][0][0], grid_size);

    for (x = 0; x < WATER_GRID; x++) {
        m_normal[x][last_index][0] = m_normal[x][last_index - 1][0];
        m_normal[x][last_index][1] = m_normal[x][last_index - 1][1];
    }


    // calculate normals to vertices (components X and Y only)
    for (x = 1; x < last_index; x++) {
        for (y = 1; y < last_index; y++) {
            m_snormal[x][y][0] =
            m_normal[x - 1][y][0] + m_normal[x + 1][y][0] + 
            m_normal[x][y - 1][0] + m_normal[x][y + 1][0];

            m_snormal[x][y][1] =
            m_normal[x - 1][y][1] + m_normal[x + 1][y][1] + 
            m_normal[x][y - 1][1] + m_normal[x][y + 1][1];
        }
    }

    /* copy map borders (components X and Y only) */
    for (x = 0; x < WATER_GRID; x++) {
        m_snormal[x][0][0] = m_normal[x][0][0];
        m_snormal[x][0][1] = m_normal[x][0][1];
        m_snormal[x][last_index][0] = m_normal[x][last_index][0];
        m_snormal[x][last_index][1] = m_normal[x][last_index][1];
    }

    memcpy ((char *) &m_snormal[0][0][0], (char *) &m_normal[0][0][0], grid_size);
    memcpy ((char *) &m_snormal[last_index][0][0], (char *) &m_normal[last_index][0][0], grid_size);


    /* calculate ourself normalization */
    for (x = 0; x < WATER_GRID; x++) {
        for (y = 0; y < WATER_GRID; y++) {
            sqroot = sqrtf (m_snormal[x][y][0] * m_snormal[x][y][0] +
                            m_snormal[x][y][1] * m_snormal[x][y][1] + 0.0016f);
            m_snormaln[x][y][0] = m_snormal[x][y][0] / sqroot;
            m_snormaln[x][y][1] = m_snormal[x][y][1] / sqroot;
            m_snormaln[x][y][2] = 0.04f / sqroot;  // m_snormal[x][y][2] = 0.04

        }
    }

    Rect2 skin_reg = get_tex_region(m_TEX_skin);
    Rect2 env_reg = get_tex_region(m_TEX_envmap);
    Rect2 bump_reg = get_tex_region(m_TEX_bumpmap_frames[m_bumpmap_frame]);
    Rect2 anim_reg = get_tex_region(m_TEX_anim_frames[m_anim_frame]);
    Rect2 caustic_reg = get_tex_region(m_TEX_caust_frames[m_caustic_frame]);

    // really simple version of a fake envmap generator
    for (x = 0; x < WATER_GRID; x++) {
        for (y = 0; y < WATER_GRID; y++) {
            // perturbate coordinates of background mapping with the components X,Y of normals...
            // simulate refraction
            m_newuvcausticmap[x][y][0]
                = m_newuvanimmap[x][y][0]
                = m_newuvbumpmap[x][y][0]
                = m_newuvmap[x][y][0]
                = m_uvmap[x][y][0] + 0.05f * m_snormaln[x][y][0];
            m_newuvcausticmap[x][y][1]
                = m_newuvanimmap[x][y][1]
                = m_newuvbumpmap[x][y][1]
                = m_newuvmap[x][y][1]
                = m_uvmap[x][y][1] + 0.05f * m_snormaln[x][y][1];
            // normalize uv coordinates for texture atlas
            m_newuvmap[x][y][0] = skin_reg.position.x + m_newuvmap[x][y][0]*skin_reg.size.width;
            m_newuvmap[x][y][1] = skin_reg.position.y + m_newuvmap[x][y][1]*skin_reg.size.height;
            m_newuvbumpmap[x][y][0] = bump_reg.position.x + m_newuvbumpmap[x][y][0]*bump_reg.size.width;
            m_newuvbumpmap[x][y][1] = bump_reg.position.y + m_newuvbumpmap[x][y][1]*bump_reg.size.height;
            m_newuvanimmap[x][y][0] = anim_reg.position.x + m_newuvanimmap[x][y][0]*anim_reg.size.width;
            m_newuvanimmap[x][y][1] = anim_reg.position.y + m_newuvanimmap[x][y][1]*anim_reg.size.height;
            m_newuvcausticmap[x][y][0] = caustic_reg.position.x + m_newuvcausticmap[x][y][0]*caustic_reg.size.width;
            m_newuvcausticmap[x][y][1] = caustic_reg.position.y + m_newuvcausticmap[x][y][1]*caustic_reg.size.height;

            // trick : xy projection of normals -> assume reflection in direction of the normals
            // looks ok for non-plane surfaces
            m_envmap[x][y][0] = 0.5f + m_snormaln[x][y][0] * 0.45f;
            m_envmap[x][y][1] = 0.5f + m_snormaln[x][y][1] * 0.45f;
            // normalize st coords for texture atlas
            m_envmap[x][y][0] = env_reg.position.x + m_envmap[x][y][0]*env_reg.size.width;
            m_envmap[x][y][1] = env_reg.position.y + m_envmap[x][y][1]*env_reg.size.height;
        }
    }
}

// build strip index for vertex arrays
void GdWater::build_strip_index(void)
{
    // array is (WATER_SIZE * 2) x (WATER_SIZE * 2)

    int strip_width = WATER_GRID - 2; // n points define n-2 triangles
    uint16_t *water_index_ptr = NULL;       // in a strip
    int x, y;

    // init pointers
    water_index_ptr = &m_water_index[0];

    // build index list
    for (x = 0; x < strip_width; x++) // vertical index in array
    {
        // strip_width+1 triangle strips
        *water_index_ptr++ = ((x + 1) * WATER_GRID) + 1;

        for (y = 1; y < strip_width; y++) // horizontal index in array
        {
            *water_index_ptr++ = (x * WATER_GRID) + y;
            *water_index_ptr++ = ((x + 1) * WATER_GRID) + y + 1;
        }
        *water_index_ptr++ = (x * WATER_GRID) + y;
    }
    // end build vertex array
}

// build triangles index for vertex arrays
void GdWater::build_tri_index(void)
{
    uint16_t *water_index_ptr = &m_water_index[0];
    int x, y, l0, l1, l2;

#if 0
    // build index list
    for(y = 0; y < WATER_SIZE*2-1; y+=2)
    {
        for(x = 0; x < WATER_SIZE*2-1; x+=2)
        {
            *water_index_ptr++ = (y)*(WATER_SIZE*2) + x;
            *water_index_ptr++ = (y)*(WATER_SIZE*2) + x+2;
            *water_index_ptr++ = (y+2)*(WATER_SIZE*2) + x;

            *water_index_ptr++ = (y+2)*(WATER_SIZE*2) + x;
            *water_index_ptr++ = (y)*(WATER_SIZE*2) + x+2;
            *water_index_ptr++ = (y+2)*(WATER_SIZE*2) + x+2;
        }
    }
#else
    // build index list
    l0 = 0;
    l1 = (WATER_SIZE*2)*2;
    l2 = l1;

    for(y=0; y < (WATER_SIZE*2); y+=2) {
        for(x=0; x < (WATER_SIZE*2)*2 / 2; x++) { // n-2 triangles (n points in 2 array lines)
            if ((x & 1) == 1) {
                *water_index_ptr++ = l1++;
                l1++;
            } else {
                *water_index_ptr++ = l0++;
                l0++;
            }
        }

        l0 = l2;
        l1 = l2 + (WATER_SIZE*2)*2;
        l2 = l1;
    }

#endif
    // end build vertex array
}
