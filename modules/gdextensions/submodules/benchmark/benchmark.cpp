// Reference:
// ----------
// https://github.com/raysan5/raylib/issues/65
// http://www.codinghorror.com/blog/2011/12/fast-approximate-anti-aliasing-fxaa.html

#include "core/image.h"
#include "scene/resources/font.h"
#include "scene/resources/texture.h"

#include "benchmark.h"

static Ref<BitmapFont> make_font_from_grid(const String &p_characters, int p_grid_width, int p_grid_height, const String &p_img) {
	Ref<BitmapFont> font(memnew(BitmapFont));

	Ref<Image> image = memnew(Image);
	if (image->load(p_img) == OK) {
		Ref<ImageTexture> tex = memnew(ImageTexture);
		tex->create_from_image(image);

		font->add_texture(tex);

		const Size2i cell_size = image->get_size() / Size2(p_grid_width, p_grid_height);
		for (int x = 0; x < p_grid_width; x++) {
			for (int y = 0; y < p_grid_height; y++) {
				const int index = x + y * p_grid_width;
				if (index < p_characters.length()) {
					const int chr = p_characters[index];
					Rect2 frect(Point2(x * cell_size.width, y * cell_size.height), cell_size);
					font->add_char(chr, 0, frect, Point2(), cell_size.width);
				} else {
					break;
				}
			}
		}
		font->set_height(cell_size.height);
	}
	return font;
}

void Benchmark::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_READY: {
		} break;
		case NOTIFICATION_PROCESS: {
			float delta = get_process_delta_time();
		} break;
	}
}

#if 0

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

#ifdef PLATFORM_ANDROID
    typedef unsigned short ushort;
#endif

#include "models/cube_model.h"
#include "models/frog_model.h"
#include "models/kid_model.h"
#include "models/robot_model.h"
#include "models/trex_model.h"

#include <raymath.h>
#include <raylib.h>
#include <rlgl.h>

#define RAYGUI_IMPLEMENTATION
#ifndef RAYGUI_SUPPORT_ICONS
# define RAYGUI_SUPPORT_ICONS
#endif
#include <extraygui.h>

#define RL_IMPLEMENTATION_RGRAPH
#include <rgraph.h>

// Postprocess effects
typedef enum {
    FX_GRAYSCALE = 0,
    FX_POSTERIZATION,
    FX_DREAM_VISION,
    FX_PIXELIZER,
    FX_CROSS_HATCHING,
    FX_CROSS_STITCHING,
    FX_PREDATOR_VIEW,
    FX_SCANLINES,
    FX_FISHEYE,
    FX_SOBEL,
    FX_BLOOM,
    FX_BLUR,
    FX_FXAA,
    NUM_FX_TYPES,
    FX_NONE = NUM_FX_TYPES,
} PostproShader;

static const char *postproShaderText[] = {
    "GRAYSCALE",
    "POSTERIZATION",
    "DREAM_VISION",
    "PIXELIZER",
    "CROSS_HATCHING",
    "CROSS_STITCHING",
    "PREDATOR_VIEW",
    "SCANLINES",
    "FISHEYE",
    "SOBEL",
    "BLOOM",
    "BLUR",
    "FXAA",
    "NONE"
};

// Statistics info.
// ----------------
typedef struct _RenderStats
{
    uint drawCalls;
    uint vertices;
    uint triangles;
} RenderStats;

static RenderStats stats;

void ResetRenderStats()
{
    stats.vertices = 0;
    stats.triangles = 0;
    stats.drawCalls = 0;
}

void LogDrawCall(uint vertices, uint triangles)
{
    stats.vertices += vertices;
    stats.triangles += triangles;
    stats.drawCalls++;
}

static uint GetVertexCount() { return stats.vertices; }
static uint GetTriangleCount() { return stats.triangles; }
static uint GetDrawCallCount() { return stats.drawCalls; }

// Shaders info.
// -------------
typedef enum {
    FIRST_SHADER_TYPE = 0,
    PHONG = 0,
    GOURAUD,
    UNTEXTURED_PHONG,
    UNTEXTURED_GOURAUD,
    FLAT,
    NUM_SHADER_TYPES
} ShaderType;

typedef struct _ShaderInfo
{
    Shader shaderProgram;
    int lightLoc;
} ShaderInfo;

const char* shaderNames[NUM_SHADER_TYPES] = {
    "PER-PIXEL LIGHTING",
    "PER-VERTEX LIGHTING",
    "PER-PIXEL UNTEXTURED",
    "PER-VERTEX UNTEXTURED",
    "FLAT COLORED"
};

const char* shaderFilenames[NUM_SHADER_TYPES][2] = {
    { "shaders/phong.vertexShader", "shaders/phong.fragmentShader" },
    { "shaders/gouraud.vertexShader", "shaders/gouraud.fragmentShader" },
    { "shaders/phong.vertexShader", "shaders/untextured-phong.fragmentShader" },
    { "shaders/gouraud.vertexShader", "shaders/untextured-gouraud.fragmentShader" },
    { "shaders/flat.vertexShader", "shaders/flat.fragmentShader" }
};


static const char* GetShaderNameByType(ShaderType shaderType)
{
    return shaderNames[shaderType];
}

// Models info.
// ------------
typedef enum {
    FIRST_MODEL_TYPE = 0,
    CUBE_MODEL = 0,
    FROG_MODEL,
    KID_MODEL,
    TREX_MODEL,
    ROBOT_MODEL,
    NUM_MODEL_TYPES  
} ModelType;

typedef struct _ModelInfo
{
    Mesh mesh;
    Material material;
    int numVerts, numIndices;
} ModelInfo;

typedef struct _RenderState
{
    uint32_t fbWidth;
    uint32_t fbHeight;
    
    uint32_t numObjects;
    float yaw;

    bool showStats;
    bool backFaceCull;
    bool depthTest;
    bool renderLines;
    bool linearFiltering;
    bool mipmapping;
    float cameraDistance;
    float cameraYaw;
    bool cameraLookAway;
    bool multisampling;

    RenderTexture2D target;
    
    uint8_t shaderType;
    ShaderInfo shaderInfo[NUM_SHADER_TYPES];
    uint8_t postType;
    Shader postShader[NUM_FX_TYPES];

    uint8_t modelType;
    ModelInfo modelInfo[NUM_MODEL_TYPES];
} RenderState;

static RenderState rs;

void FreeModel();
void DestroyAll();


void DestroyAll() {
    FreeModel();
}

enum {
    VBO_VERTS = 0,
    VBO_TEXCOORDS01 = 1,
    VBO_NORMALS = 2,
    VBO_COLORS = 3,
    VBO_TANGENTS = 4,
    VBO_TEXCOORDS02 = 5,
    VBO_INDICIES = 6,
    MAX_MESH_VBO = 7 // Maximum number of vbo per mesh
};

Mesh GenMesh(ModelInfo *modelInfo, const float *verts, int num_verts, const unsigned short *indices, int num_indicies)
{
    Mesh mesh = { 0 };

    mesh.memmap = true;
    mesh.vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VBO, sizeof(unsigned int));

    // set vertex data pointers
    const uint posSize = 3;
    const uint normSize = 3;
    const uint uvSize = 2;
    const uint vertBytes = sizeof(float) * (posSize + normSize + uvSize);

    // update buffer interleaves:
    mesh.vboStride = vertBytes;
    mesh.vboOffsets = (intptr_t *)RL_CALLOC(SHADER_NUM_LOC_MAP, sizeof(intptr_t));
    mesh.vboOffsets[SHADER_LOC_VERTEX_POSITION] = 0;
    mesh.vboOffsets[SHADER_LOC_VERTEX_NORMAL] = posSize * sizeof(float);
    mesh.vboOffsets[SHADER_LOC_VERTEX_TEXCOORD01] = (posSize + normSize) * sizeof(float);

    mesh.vertices = (float *)verts;
    mesh.indices = (unsigned short *)indices;
    mesh.vertexCount = num_verts/8;
    mesh.triangleCount = num_indicies/3;

    // Upload vertex data to GPU (static mesh)
    rlLoadMesh(&mesh, false);

    modelInfo->numVerts = num_verts;
    modelInfo->numIndices = num_indicies;
    modelInfo->mesh = mesh;
    modelInfo->material = LoadMaterialDefault();

    return mesh;
}

void InitModel()
{
    GenMesh(&rs.modelInfo[CUBE_MODEL], cube_verts, sizeof(cube_verts)/(sizeof(float)), cube_indices, sizeof(cube_indices)/sizeof(ushort));
    GenMesh(&rs.modelInfo[FROG_MODEL], frog_verts, sizeof(frog_verts)/(sizeof(float)), frog_indices, sizeof(frog_indices)/sizeof(ushort));
    GenMesh(&rs.modelInfo[KID_MODEL], kid_verts, sizeof(kid_verts)/(sizeof(float)), kid_indices, sizeof(kid_indices)/sizeof(ushort));
    GenMesh(&rs.modelInfo[TREX_MODEL], trex_verts, sizeof(trex_verts)/(sizeof(float)), trex_indices, sizeof(trex_indices)/sizeof(ushort));
    GenMesh(&rs.modelInfo[ROBOT_MODEL], robot_verts, sizeof(robot_verts)/(sizeof(float)), robot_indices, sizeof(robot_indices)/sizeof(ushort));

#ifdef PLATFORM_DESKTOP
# define LOADTEX(m,t) \
   rs.modelInfo[m].material.maps[0].texture = LoadTextureFromMemory(rlFindEmbeddedData(t)); \
   if (rs.modelInfo[m].material.maps[0].texture.id) rlGenerateMipmaps(&rs.modelInfo[m].material.maps[0].texture)
    LOADTEX(CUBE_MODEL, "textures/cube.ktx");
    LOADTEX(CUBE_MODEL, "textures/cube.png");
    LOADTEX(FROG_MODEL, "textures/frog.png");
    LOADTEX(KID_MODEL, "textures/kid.png");
    LOADTEX(TREX_MODEL, "textures/trex.png");
    LOADTEX(ROBOT_MODEL, "textures/robot.png");
#else
# define LOADTEX(m,t) \
   rs.modelInfo[m].material.maps[0].texture = LoadTextureFromMemory(rlFindEmbeddedData(t))
    LOADTEX(CUBE_MODEL, "textures/cube.ktx");
    LOADTEX(FROG_MODEL, "textures/frog.ktx");
    LOADTEX(KID_MODEL, "textures/kid.ktx");
    LOADTEX(TREX_MODEL, "textures/trex.ktx");
    LOADTEX(ROBOT_MODEL, "textures/robot.ktx");
#endif
    if (!rs.modelInfo[CUBE_MODEL].material.maps[0].texture.id ||
        !rs.modelInfo[FROG_MODEL].material.maps[0].texture.id ||
        !rs.modelInfo[KID_MODEL].material.maps[0].texture.id ||
        !rs.modelInfo[TREX_MODEL].material.maps[0].texture.id ||
        !rs.modelInfo[ROBOT_MODEL].material.maps[0].texture.id)
    {
        fprintf(stderr, "*** Failed to load the model textures.\n");
    }

    for (uint t = FIRST_SHADER_TYPE; t < NUM_SHADER_TYPES; t++)
    {
        const ShaderType shaderType = (ShaderType)t;
        Shader po = LoadShader(rlFindEmbeddedFile(shaderFilenames[shaderType][0]), rlFindEmbeddedFile(shaderFilenames[shaderType][1]));

        ShaderInfo *shaderInfo = &rs.shaderInfo[t];
        shaderInfo->shaderProgram = po;
        // save the locations of the program inputs
        shaderInfo->lightLoc = GetShaderLocation( po, "lightPosition" );
    }

    // Load postprocess shaders
    // NOTE: Defining 0 (NULL) for vertex shader forces usage of internal default vertex shader
    rs.postShader[FX_GRAYSCALE] = LoadShader(0, rlFindEmbeddedFile("postprocess/grayscale.fs"));
    rs.postShader[FX_POSTERIZATION] = LoadShader(0, rlFindEmbeddedFile("postprocess/posterization.fs"));
    rs.postShader[FX_DREAM_VISION] = LoadShader(0, rlFindEmbeddedFile("postprocess/dream_vision.fs"));
    rs.postShader[FX_PIXELIZER] = LoadShader(0, rlFindEmbeddedFile("postprocess/pixelizer.fs"));
    rs.postShader[FX_CROSS_HATCHING] = LoadShader(0, rlFindEmbeddedFile("postprocess/cross_hatching.fs"));
    rs.postShader[FX_CROSS_STITCHING] = LoadShader(0, rlFindEmbeddedFile("postprocess/cross_stitching.fs"));
    rs.postShader[FX_PREDATOR_VIEW] = LoadShader(0, rlFindEmbeddedFile("postprocess/predator.fs"));
    rs.postShader[FX_SCANLINES] = LoadShader(0, rlFindEmbeddedFile("postprocess/scanlines.fs"));
    rs.postShader[FX_FISHEYE] = LoadShader(0, rlFindEmbeddedFile("postprocess/fisheye.fs"));
    rs.postShader[FX_SOBEL] = LoadShader(0, rlFindEmbeddedFile("postprocess/sobel.fs"));
    rs.postShader[FX_BLOOM] = LoadShader(0, rlFindEmbeddedFile("postprocess/bloom.fs"));
    rs.postShader[FX_BLUR] = LoadShader(0, rlFindEmbeddedFile("postprocess/blur.fs"));
    rs.postShader[FX_FXAA] = LoadShader(rlFindEmbeddedFile("postprocess/fxaa.vert.fs"), rlFindEmbeddedFile("postprocess/fxaa.frag.fs"));
}

void FreeModel()
{
    for (uint t = FIRST_SHADER_TYPE; t < NUM_SHADER_TYPES; t++)
    {
        UnloadShader(rs.shaderInfo[t].shaderProgram);
    }

    for (uint t = FIRST_MODEL_TYPE; t < NUM_MODEL_TYPES; t++)
    {
        rlUnloadMesh(&rs.modelInfo[t].mesh);
        rlUnloadTexture(rs.modelInfo[t].material.maps[0].texture.id);
    }
}

void Render()
{
    Matrix projM = rlGetMatrixProjection();
    Matrix modelViewM = rlGetMatrixModelview();

    Matrix proj = MatrixPerspective(45*DEG2RAD, ((float) rs.fbWidth)/rs.fbHeight, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);

    // calculate the view matrix
    Vector3 lookPos = {0,2.3f,rs.cameraLookAway ? 100 : 0};
    Vector3 eye = Vector3Transform((Vector3){0,0,rs.cameraDistance}, MatrixRotateY(rs.cameraYaw));
    Matrix view = MatrixLookAt(eye, (Vector3){0,2.3f,rs.cameraLookAway ? 100 : 0}, (Vector3){0,1,0});

    // bind the shader program and set the uniform shader parameters
    ShaderInfo currentShader = rs.shaderInfo[rs.shaderType];

    // light position is fixed in world space
    Vector3 lightPosition_worldspace = {-30,10,20};
    Vector3 lightPosition_viewspace = Vector3Transform(lightPosition_worldspace, view);
    SetShaderValue(currentShader.shaderProgram, currentShader.lightLoc, Vector3ToFloat(lightPosition_viewspace), SHADER_UNIFORM_VEC3);

    // setup shader for current model
    ModelInfo currentModel = rs.modelInfo[rs.modelType];
    currentModel.material.shader = currentShader.shaderProgram;

    // set OpenGL states
    if (rs.backFaceCull)
        rlEnableBackfaceCulling();
    else
        rlDisableBackfaceCulling();

    if (rs.depthTest)
        rlEnableDepthTest();
    else
        rlDisableDepthTest();

    rlSetMatrixProjection(proj);

    // calculate how many objects to draw per row
    uint rowSize = 1;
    while (rowSize * rowSize < rs.numObjects)
        rowSize++;
    const float rowScale = 25.0f;
    float rowStep = rs.numObjects == 1 ? rowScale : rowScale / (rowSize - 1);

    // draw the objects in a grid layout
    uint numObjectsDrawn = 0;
    for (int x=0; x<rowSize && numObjectsDrawn < rs.numObjects; x++)
    {
        float objX = (rs.numObjects == 1) ? 0 : (1.5f * (-rowScale/2.0f + x*rowStep));

        for (int y=0; y<rowSize && numObjectsDrawn < rs.numObjects; y++)
        {
            float objXs = objX + (y%2)*rowStep/2.0f;
            float objY = (rs.numObjects == 1) ? 0 : (-rowScale/2.0f + y*rowStep);

            // construct a yaw for this object, by combining the global yaw with the grid position
            // calculate the model matrix from the yaw
            float yaw = rs.yaw + x*40 + y*50;
            Matrix modelRotate = MatrixRotateY(yaw);
            Matrix modelTranslate = MatrixTranslate(objXs, objY, 0);
            Matrix model = MatrixMultiply(modelRotate, modelTranslate);

            // calculate the matrices
            rlSetMatrixModelview(MatrixMultiply(model, view));

            // draw!
            if (rs.renderLines)
                rlDrawMeshWireframe(currentModel.mesh, currentModel.material, MatrixIdentity());
            else
                rlDrawMesh(currentModel.mesh, currentModel.material, MatrixIdentity());

            LogDrawCall(currentModel.numVerts, currentModel.numIndices/3);

            numObjectsDrawn++;
        }
    }

    rlSetMatrixProjection(projM);
    rlSetMatrixModelview(modelViewM);
}

void ApplyTextureFilter()
{
    int minFilter, magFilter;
    if (rs.linearFiltering)
    {
        magFilter = RL_TEXTURE_FILTER_LINEAR;
        if (rs.mipmapping)
            minFilter = RL_TEXTURE_FILTER_LINEAR_MIP_NEAREST;
        else
            minFilter = RL_TEXTURE_FILTER_LINEAR;
    }
    else
    {
        magFilter = RL_TEXTURE_FILTER_NEAREST;
        if (rs.mipmapping)
            minFilter = RL_TEXTURE_FILTER_MIP_NEAREST;
        else
            minFilter = RL_TEXTURE_FILTER_NEAREST;
    }

    rlTextureParameters(rs.modelInfo[rs.modelType].material.maps[0].texture.id, RL_TEXTURE_MIN_FILTER, minFilter);
    rlTextureParameters(rs.modelInfo[rs.modelType].material.maps[0].texture.id, RL_TEXTURE_MAG_FILTER, magFilter);
}

void ResetFramebuffer()
{
    rs.target = LoadRenderTexture(rs.fbWidth, rs.fbHeight);
    SetTextureFilter(rs.target.texture, FILTER_BILINEAR);  // Texture scale filter to use (FILTER_POINT | FILTER_BILINEAR)
}

void SetNextFramebufferResolution()
{
    // shrink the framebuffer to 2/3 its previous size
    if (rs.fbHeight <= 320)
    {
        rs.fbWidth = 0;
        rs.fbHeight = 0;
    }
    else
    {
        rs.fbWidth = rs.fbWidth * 2 / 3;
        rs.fbHeight = rs.fbHeight * 2 / 3;
    }

    ResetFramebuffer();
}



int main(int argc, char *argv[])
{
    // Stats view order
    enum {
        OBJECT_COUNT = 2,
        OBJECT_COMPLEXITY,
        FRAMEBUFFER,
        MULTISAMPLING,
        SHADER,
        TEXTURE_FILTERING,
        MIPMAPS,
        BACKFACE_CULLING,
        DEPTH_TEST,
        WIREFRAME,
        CAMERA_DISTANCE,
        CAMERA_YAW,
        CAMERA_LOOK,
        STATS_DISPLAY
    };

    struct PlotterData {
        unsigned int gid, cid;
        size_t bufSize;
        float width, height;
        cvector_vector_type(float) xs;
        cvector_vector_type(float) fps;
    } plotter = { 0 };

    // Initialization
    //--------------------------------------------------------------------------------------
    const Color bg = (Color){ 25, 50, 75, 255 };
    const int defaultLogLevel = GetTraceLogLevel();

    // default settings
    rs.numObjects = 81;
    rs.showStats = true;
    rs.backFaceCull = true;
    rs.depthTest = true;
    rs.renderLines = false;
    rs.linearFiltering = true;
    rs.mipmapping = true;
    rs.cameraDistance = 40.0f;
    rs.cameraYaw = 0.0f;
    rs.shaderType = FIRST_SHADER_TYPE;
    rs.cameraLookAway = false;
    rs.modelType = FIRST_MODEL_TYPE;
    rs.postType = FX_NONE;
    rs.multisampling = false;

    bool restart = false;

    while(!restart)
    {
        if (rs.multisampling)
            SetConfigFlags(FLAG_MSAA_4X_HINT);

#ifdef PLATFORM_DESKTOP
        InitWindow(800, 600, "raylib benchmark");
#else
        InitWindow(0, 0, "raylib benchmark");
#endif
        SetTargetFPS(60);
        //--------------------------------------------------------------------------------------
        // Add some transparency:
        Color c1 = Fade(GetColor(GuiGetStyle(BUTTON, BASE_COLOR_NORMAL)), 0.2);
        GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, ColorToInt(c1));
        Color c2 = Fade(GetColor(GuiGetStyle(BUTTON, BASE_COLOR_FOCUSED)), 0.5);
        GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, ColorToInt(c2));
        GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(RAYWHITE));
        GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, ColorToInt(WHITE));
        GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

        // create a native frambuffer with dimensions equal to the display screen
        rs.fbWidth = GetScreenWidth();
        rs.fbHeight = GetScreenHeight();

        // load the model, texture, and shader
        InitModel();
        ApplyTextureFilter();

        // Graph setup
        plotter.width = 1.0;
        plotter.height = 0.067;
        plotter.bufSize = 250;
        for(int i = 0; i < plotter.bufSize; ++i){
            cvector_push_back(plotter.xs, i/(float)plotter.bufSize);
        }
        cbuffer_resize(plotter.fps, plotter.bufSize);
        const float margins[4] = { 1.0f - plotter.width, 0.0, 1.0f - plotter.height, 0.0};
        const float ratio = GetScreenWidth()/GetScreenHeight();
        plotter.gid = rg_setup(0.0f, 1.0f, 0.0f, 100.0f, ratio, margins, 0.1f, 0.1f, 0.1f);
        rg_add_grid(plotter.gid, 0.02f, 20.0f, 0.0015f, 0.03f, 0.5f, 0.06f, false);
        plotter.cid = rg_add_curve(plotter.gid, plotter.xs, plotter.fps, 0.0018f, 0.18f, 0.39f, 0.99f);
        //rg_add_hist(plotter.id, 25, ys2, 0.0015f, 0.35f, 0.95f, 0.48f);

        int osk_key = -1;
        uint textRowSel = 2; // text row selected

        // Main game loop
        while (!WindowShouldClose() && !restart)
        {
            // Update
            //----------------------------------------------------------------------------------
            float deltaTimeMs = GetFrameTime() * 1000;

            #define EXTKEYNAV(row, key) (textRowSel==row && (IsKeyPressed(key) || IsKeyPressed(KEY_ENTER)))

            if (IsKeyPressed(KEY_ESCAPE))
                break;
            // +/- changes the number of objects displayed
            else if (IsKeyPressed(KEY_MINUS) || osk_key=='-' || EXTKEYNAV(OBJECT_COUNT, KEY_DPAD_X))
                rs.numObjects = (rs.numObjects > 0) ? (rs.numObjects - 1) : 0;
            else if (IsKeyPressed(KEY_EQUAL) || osk_key=='+' || EXTKEYNAV(OBJECT_COUNT, KEY_DPAD_Y))
                rs.numObjects++;
            // S shows/hides the render statistics
            else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DPAD_A) || EXTKEYNAV(STATS_DISPLAY, KEY_DPAD_Y)|| EXTKEYNAV(STATS_DISPLAY, KEY_DPAD_X))
                rs.showStats = !rs.showStats;
            // C turns backface culling on/off
            else if (IsKeyPressed(KEY_C) || EXTKEYNAV(BACKFACE_CULLING, KEY_DPAD_Y)|| EXTKEYNAV(BACKFACE_CULLING, KEY_DPAD_X))
                rs.backFaceCull = !rs.backFaceCull;
            // D turns depth testing on/off
            else if (IsKeyPressed(KEY_D) || EXTKEYNAV(DEPTH_TEST, KEY_DPAD_Y)|| EXTKEYNAV(DEPTH_TEST, KEY_DPAD_X))
                rs.depthTest = !rs.depthTest;
            // W turns wireframe rendering on/off
            else if (IsKeyPressed(KEY_W) || EXTKEYNAV(WIREFRAME, KEY_DPAD_Y)|| EXTKEYNAV(WIREFRAME, KEY_DPAD_X))
                rs.renderLines = !rs.renderLines;
            // F changes the framebuffer size
            else if (IsKeyPressed(KEY_F))
                SetNextFramebufferResolution();
            // I changes the texture filtering mode
            else if (IsKeyPressed(KEY_I) || EXTKEYNAV(TEXTURE_FILTERING, KEY_DPAD_Y)|| EXTKEYNAV(TEXTURE_FILTERING, KEY_DPAD_X))
            {
                rs.linearFiltering = !rs.linearFiltering;
                ApplyTextureFilter();
            }
            // M toggles mipmaps on and off
            else if (IsKeyPressed(KEY_M) || EXTKEYNAV(MIPMAPS, KEY_DPAD_Y)|| EXTKEYNAV(MIPMAPS, KEY_DPAD_X))
            {
                rs.mipmapping = !rs.mipmapping;
                ApplyTextureFilter();
            }
            // Z changes the camera distance
            else if (IsKeyPressed(KEY_Z))
            {
                rs.cameraDistance -= 5.0f;
                if (rs.cameraDistance < 5.0f)
                    rs.cameraDistance = 60.0f;
            }
            // Y changes the camera rotation
            else if (IsKeyPressed(KEY_Y))
            {
                rs.cameraYaw += 30.0f;
                if (rs.cameraYaw > 90.0f)
                    rs.cameraYaw = -90.0f;
            }
            // H changes the shader type
            else if (IsKeyPressed(KEY_H) || EXTKEYNAV(SHADER, KEY_DPAD_Y))
            {
                rs.shaderType++;
                if (rs.shaderType == NUM_SHADER_TYPES)
                    rs.shaderType = FIRST_SHADER_TYPE;
            }
            else if (EXTKEYNAV(SHADER, KEY_DPAD_X))
            {
                if (rs.shaderType == FIRST_SHADER_TYPE)
                    rs.shaderType = NUM_SHADER_TYPES - 1;
                else
                    rs.shaderType--;
            }
            // L makes the camera look towards/away from the scene
            else if (IsKeyPressed(KEY_L))
                rs.cameraLookAway = !rs.cameraLookAway;
            // X changes the model type
            else if (IsKeyPressed(KEY_X) || EXTKEYNAV(OBJECT_COMPLEXITY, KEY_DPAD_Y))
            {
                rs.modelType++;
                if (rs.modelType == NUM_MODEL_TYPES)
                    rs.modelType = FIRST_MODEL_TYPE;
            }
            else if (EXTKEYNAV(OBJECT_COMPLEXITY, KEY_DPAD_X))
            {
                if (rs.modelType == FIRST_MODEL_TYPE)
                    rs.modelType = NUM_MODEL_TYPES - 1;
                else
                    rs.modelType--;
            }
            // P turns 4x multisampling on/off
            else if (IsKeyPressed(KEY_P))
            {
                rs.multisampling = !rs.multisampling;
                restart = true;
            }
            // R saves a screenshot to screenshot.png
            else if (IsKeyPressed(KEY_R))
            {
                TakeScreenshot("screenshot.png");
            }
            // V toggles log level
            else if (IsKeyPressed(KEY_V))
            {
                if (defaultLogLevel == GetTraceLogLevel())
                    SetTraceLogLevel(LOG_DEBUG);
                else
                    SetTraceLogLevel(defaultLogLevel);
            }
            osk_key = -1;

            // Draw
            //----------------------------------------------------------------------------------
            BeginDrawing();
            {
                ClearBackground(bg);

                // update object transforms using the frame time - this ensures animation won't appear
                // to slow down when the frame rate drops
                rs.yaw += 0.0001f * deltaTimeMs;

                if(rs.fbWidth == GetScreenWidth() && rs.fbHeight == GetScreenHeight()) {
                    Render(); // just draw
                } else {
                    // draw with separate lower-res framebuffer:
                    BeginTextureMode(rs.target);
                    {
                        ClearBackground(BLANK);
                        Render();
                    }
                    EndTextureMode();
                    // Draw RenderTexture2D to window
                    DrawTexturePro(rs.target.texture,
                                (Rectangle){ 0.0f, 0.0f, (float)rs.target.texture.width, (float)-rs.target.texture.height },
                                (Rectangle){ 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() },
                                (Vector2){ 0, 0 }, 0.0f, WHITE);
                }

                if (rs.postType != FX_NONE) {
                    // Postproces shader
                    BeginShaderMode(rs.postShader[rs.postType]);
                    EndShaderMode();
                }

                // update and display statistics
                int fps = GetFPS();
                uint draws = GetDrawCallCount(), tris = GetTriangleCount(), verts = GetVertexCount();
                uint trisPerSec = tris * fps;

                const int AvgNumFrames = 5;
                static float avg = 0;
                static int indx = 0;
                avg += GetFrameTime()/AvgNumFrames;
                if (indx++%AvgNumFrames==0 && deltaTimeMs){
                    cbuffer_push_back(plotter.fps, 1/avg);
                    rg_update_curve(plotter.gid, plotter.cid, plotter.xs, plotter.fps);
                    avg = 0;
                }

                ResetRenderStats();
                # define _LineStart 2
                # define _FontSize 20
                # define _LineHeight (_FontSize+3)
                # define _AddTextLine(text, x, row) { \
                    const int curr=row, y=_LineHeight*curr; const int tw = MeasureText(text, _FontSize); \
                    if (curr==textRowSel) { \
                        DrawRectangle(0, y-1, tw+_LineStart*2, _FontSize+1, WHITE); DrawText(text, x, y, _FontSize, BLACK); } \
                    else DrawText(text, x, y, _FontSize, curr < 2 ? SKYBLUE:WHITE); }

                uint textRow = 0;
                _AddTextLine(FormatText("%d FPS %.1fM TRIS/SEC %.1fK TRIS/FRAME", fps, (float)trisPerSec/1000000, (float)tris/1000), _LineStart, textRow++);

                if (rs.showStats)
                {
                    const float ratio = GetScreenWidth()/GetScreenHeight();
                    rg_draw(plotter.gid, ratio);

                    _AddTextLine(FormatText("%d VERTS/FRAME", verts), _LineStart, textRow++);
                    _AddTextLine(FormatText("+/-: OBJECT COUNT [%d]", rs.numObjects), _LineStart, textRow++);
                    _AddTextLine(FormatText("X: OBJECT COMPLEXITY [%s]",
                        rs.modelType == CUBE_MODEL ? "12 TRIS" :
                        rs.modelType == FROG_MODEL ? "496 TRIS" :
                        rs.modelType == KID_MODEL ? "5682 TRIS" :
                        rs.modelType == TREX_MODEL ? "18940 TRIS" :
                        "55593 TRIS"), _LineStart, textRow++);
                    _AddTextLine(FormatText("F: FRAMEBUFFER [%d X %d]", rs.fbWidth, rs.fbHeight), _LineStart, textRow++);
                    _AddTextLine(FormatText("P: MULTISAMPLING [%s]", rs.multisampling ? "ON" : "OFF"), _LineStart, textRow++);
                    _AddTextLine(FormatText("H: SHADER [%s]", GetShaderNameByType((ShaderType)rs.shaderType)), _LineStart, textRow++);
                    _AddTextLine(FormatText("I: TEXTURE FILTER [%s]", rs.linearFiltering ? "LINEAR" : "NEAREST"), _LineStart, textRow++);
                    _AddTextLine(FormatText("M: MIPMAPS [%s]", rs.mipmapping ? "ON" : "OFF"), _LineStart, textRow++);
                    _AddTextLine(FormatText("C: BACKFACE CULLING [%s]", rs.backFaceCull ? "ON" : "OFF"), _LineStart, textRow++);
                    _AddTextLine(FormatText("D: DEPTH TEST [%s]", rs.depthTest ? "ON" : "OFF"), _LineStart, textRow++);
                    _AddTextLine(FormatText("W: WIREFRAME [%s]", rs.renderLines ? "ON" : "OFF"), _LineStart, textRow++);
                    _AddTextLine(FormatText("Z: CAMERA DISTANCE [%d]", (int)rs.cameraDistance), _LineStart, textRow++);
                    _AddTextLine(FormatText("Y: CAMERA YAW [%d]", (int)rs.cameraYaw), _LineStart, textRow++);
                    _AddTextLine(FormatText("L: CAMERA LOOK [%s]", rs.cameraLookAway ? "AWAY" : "TOWARDS"), _LineStart, textRow++);
                    _AddTextLine(FormatText("A: POSTPROCESS [%s]", postproShaderText[rs.postType]), _LineStart, textRow++);
                    _AddTextLine("S: STATS DISPLAY [ON]", _LineStart, textRow++);

                    DrawText("ESC: EXIT", _LineStart, _LineHeight*textRow, _FontSize, BLUE);

                    const char *osk = "+-XFPHIMCDWZYLAS";
                    const size_t osk_sz = strlen(osk);
                    const int bmarg = 2;
                    const float bsize = (GetScreenWidth()-2.0f*_LineStart-osk_sz*bmarg-10)/(osk_sz+1);
                    const int bheight = bsize>80 ? 80 : bsize;
                    for (int k=0; k<osk_sz; ++k) {
                        const char lbl[2] = { osk[k], 0 };
                        if (GuiButton((Rectangle){ _LineStart+k*(bsize+bmarg), GetScreenHeight()-bheight-bmarg, bsize, bheight }, lbl)) { osk_key = osk[k]; }
                    }
                    if (GuiButton((Rectangle){ _LineStart+osk_sz*(bsize+bmarg), GetScreenHeight()-bheight-bmarg, bsize+10, bheight }, "ESC")) { break; }

                    // up/down moves selection after we calculated all rows (skip first and last rows)
                    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DPAD_UP))
                    {
                        if (textRowSel > 2) textRowSel--;
                    }
                    else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_DPAD_DOWN))
                    {
                        if (textRowSel < textRow-1) textRowSel++;
                    }
                }
            }
            EndDrawing();
            //----------------------------------------------------------------------------------
        }

        // De-Initialization
        //--------------------------------------------------------------------------------------
        FreeModel();          // Unload all resources
        CloseWindow();        // Close window and OpenGL context
        //--------------------------------------------------------------------------------------

        // Restart&continue application
        if (restart)
            restart = false;
        else break;
    } // restart

    return 0;
}
#endif
