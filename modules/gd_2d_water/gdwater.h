
// https://github.com/elemel/water-shader
// https://github.com/dionyziz/wave-experiment
// Code library/Demo and\ examples/[shaders]/water shader


#ifndef __GD_WATER_H__
#define __GD_WATER_H__

#include "core/int_types.h"
#include "core/math/rect2.h"
#include "core/math/random_pcg.h"
#include "scene/resources/texture.h"

/* grid of 50 x 50 fluid cells */
#define WATER_SIZE  50
#define WATER_MILX ((WATER_SIZE >> 1)+1)
#define WATER_MILY ((WATER_SIZE >> 1)+1)
#define WATER_LARG (WATER_SIZE >> 1)

#define TS() ( ((float (clock ()) * 1000.0f) / CLOCKS_PER_SEC) - 100.0f )

//////////////////////////////////////////////////
// Water object ...
//////////////////////////////////////////////////

class GdWater
{
private:
    RandomPCG m_rng;

    float m_last_msTime;
    float m_cur_msTime;

    bool m_textured;

    // TEX_WIDTH*TEX_HEIGHT*3 or 4
    // static buffers ...
    int m_Water1[WATER_SIZE + 2][WATER_SIZE + 2];
    int m_Water2[WATER_SIZE + 2][WATER_SIZE + 2];

    int m_Smooth[WATER_SIZE + 2][WATER_SIZE + 2];               // buffer used for smoothing operation

    int (*m_p1)[WATER_SIZE + 2][WATER_SIZE + 2];                // pointer FRONT
    int (*m_p2)[WATER_SIZE + 2][WATER_SIZE + 2];                // pointer BACK
    int m_angle;                                                // angle for wave generator

    // geometric construction (static number of vertices)
    float (*m_sommet)[WATER_SIZE * 2][3];                     // vertices vector
    float (*m_normal)[WATER_SIZE * 2][3];                     // quads normals
    float (*m_snormal)[WATER_SIZE * 2][3];                    // vertices normals (average)
    float (*m_snormaln)[WATER_SIZE * 2][3];                   // normalized vertices normals
    float (*m_uvmap)[WATER_SIZE * 2][2];                      // background texture coordinates
    float (*m_maskmap)[WATER_SIZE * 2][2];                    // masking texture coordinates
    float (*m_newuvmap)[WATER_SIZE * 2][2];                   // perturbated background coordinates -> refraction
    float (*m_newuvanimmap)[WATER_SIZE * 2][2];               // perturbated animation coordinates -> refraction
    float (*m_newuvcausticmap)[WATER_SIZE * 2][2];            // perturbated caustics coordinates -> refraction
    float (*m_newuvbumpmap)[WATER_SIZE * 2][2];               // perturbated bump map coordinates -> refraction
    float (*m_envmap)[WATER_SIZE * 2][2];                     // envmap coordinates...
    uint16_t *m_water_index;                                  // vertex array index

    int  m_VBO_sommet_ID;                                    // Vertex VBO Name
    int  m_VBO_newuvmap_ID;
    int  m_VBO_envmap_ID;
    int  m_VBO_normal_ID;

    Ref<Texture> m_TEX_skin, m_TEX_mask;                      // Skin/bg and mask texture
    Ref<Texture> m_TEX_envmap;                                // Texture coordinates
    Array m_TEX_anim_frames;                                  // Subwater animation layer textures
    Array m_TEX_caust_frames;                                 // Water causts textures
    Array m_TEX_bumpmap_frames;                               // Water bumpmap textures
    int m_anim_frame;                                         // current animation frame
    int m_caustic_frame;                                      // current caustic animation frame
    int m_bumpmap_frame;                                      // current bumpmap frame

    bool m_water_index_built;

    // private methods
    unsigned int irand() { return m_rng.rand(); }
    Rect2 get_tex_region(Ref<Texture> tex) {
		Ref<AtlasTexture> p_atlas = tex;
		if ( p_atlas.is_valid() ) {
            return p_atlas->get_region();
        }
        return Rect2(0,0,1,1);
    }

    void new_water (void);        // fluid calculus
    void smooth_water (void);     // smooth filter
    void prebuild_water (void);   // precalculate geometric stuffs
    void build_water (void);      // build geometry

    void build_strip_index(void); // build strip index for vertex arrays
    void build_tri_index(void);   // build triangles index for vertex arrays

public:
    GdWater (void);
    ~GdWater (void);

    void init();

    void set_textured(bool p_textured) { m_textured = p_textured; }
    bool get_textured() const { return m_textured; }

    void set_skin_texture(Ref<Texture> tex);
    void set_mask_texture(Ref<Texture> tex);

    void release (void);                             // release gfx

    void set_wave (float a_x, float a_y, int a_val); // trace a hole in the fluid cells at normalized coords
    void run_wave (float a_phase, float a_cos, float a_sin, int a_val); // some waves using parametric curves
    void random_wave (void);                         // random hole

    void update (void);                              // next step in fluid model
    void build (void);                               // build geometric model
    void display (const float *anim_matrix = NULL);  // display resulting geometry (using animation matrix for animation texture)

    bool bench (float a_rate);                       // measure elapsed time ...
    void nexttime (void) { m_last_msTime = m_cur_msTime; }
    void nextanim(void) { // next animation frame
        m_anim_frame = (++m_anim_frame)%m_TEX_anim_frames.size();
        m_caustic_frame = (++m_caustic_frame)%m_TEX_caust_frames.size();
    }
    
    int curranim() { return m_anim_frame; }
};


#endif /* _GD_WATER_H__ */
