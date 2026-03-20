/**************************************************************************/
/*  detournavigationmesh.h                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef GODOTDETOURNAVIGATIONMESH_H
#define GODOTDETOURNAVIGATIONMESH_H

#include "core/math/vector2.h"
#include "core/os/file_access.h"
#include "core/reference.h"

#include <map>
#include <vector>

#include "detourcrowdagent.h"

class DetourInputGeometry;
class dtTileCache;
class dtNavMesh;
class dtNavMeshQuery;
class dtCrowd;
class RecastContext;
class GodotDetourDebugDraw;
struct MeshProcess;
struct rcConfig;
struct LinearAllocator;
struct FastLZCompressor;
struct TileCacheData;

class MeshInstance;
class DetourObstacle;

// Parameters to initialize a DetourNavigationMesh.
struct DetourNavigationMeshParameters : public Reference {
	GDCLASS(DetourNavigationMeshParameters, Reference)

protected:
	static void _bind_methods();

public:
	// Called when .new() is called in gdscript
	void _init() {}

	// It is important to understand that recast/detour operates on a voxel field internally.
	// The size of a single voxel (often called cell internally) has significant influence on how a navigation mesh is created.
	// A tile is a rectangular region within the navigation mesh. In other words, every navmesh is divided into equal-sized tiles, which are in turn divided into cells.
	// The detail mesh is a mesh used for determining surface height on the polygons of the navigation mesh.
	// Units are usually in world units [wu] (e.g. meters, or whatever you use), but some may be in voxel units [vx] (multiples of cellSize).
	Vector2 cellSize; // x = width & depth of a single cell (only one value as both must be the same) | y = height of a single cell. [wu]
	int maxNumAgents; // How many agents this mesh can manage at once.
	float maxAgentSlope; // How steep an angle can be to still be considered walkable. In degree. Max 90.0.
	float maxAgentHeight; // The maximum height of an agent supported in this navigation mesh. [wu]
	float maxAgentClimb; // How high a single "stair" can be to be considered walkable by an agent. [wu]
	float maxAgentRadius; // The maximum radius of an agent in this navigation mesh. [wu]
	float maxEdgeLength; // The maximum allowed length for contour edges along the border of the mesh. [wu]
	float maxSimplificationError; // The maximum distance a simplified contour's border edges should deviate the original raw contour. [vx]
	int minNumCellsPerIsland; // How many cells an isolated area must at least have to become part of the navmesh.
	int minCellSpanCount; // Any regions with a span count smaller than this value will, if possible, be merged with larger regions.
	int maxVertsPerPoly; // Maximum number of vertices per polygon in the navigation mesh.
	int tileSize; // The width,depth & height of a single tile. [vx]
	int layersPerTile; // How many vertical layers a single tile is expected to have. Should be less for "flat" levels, more for something like tall, multi-floored buildings.
	float detailSampleDistance; // The sampling distance to use when generating the detail mesh. [wu]
	float detailSampleMaxError; // The maximum allowed distance the detail mesh should deviate from the source data. [wu]

	void set_cell_size(const Vector2 &p_value) { cellSize = p_value; }
	Vector2 get_cell_size() const { return cellSize; }
	void set_max_num_agents(int p_value) { maxNumAgents = p_value; }
	int get_max_num_agents() const { return maxNumAgents; }
	void set_max_agent_slope(float p_value) { maxAgentSlope = p_value; }
	float get_max_agent_slope() const { return maxAgentSlope; }
	void set_max_agent_height(float p_value) { maxAgentHeight = p_value; }
	float get_max_agent_height() const { return maxAgentHeight; }
	void set_max_agent_climb(float p_value) { maxAgentClimb = p_value; }
	float get_max_agent_climb() const { return maxAgentClimb; }
	void set_max_agent_radius(float p_value) { maxAgentRadius = p_value; }
	float get_max_agent_radius() const { return maxAgentRadius; }
	void set_max_edge_length(float p_value) { maxEdgeLength = p_value; }
	float get_max_edge_length() const { return maxEdgeLength; }
	void set_max_simplification_error(float p_value) { maxSimplificationError = p_value; }
	float get_max_simplification_error() const { return maxSimplificationError; }
	void set_min_num_cells_per_island(int p_value) { minNumCellsPerIsland = p_value; }
	int get_min_num_cells_per_island() const { return minNumCellsPerIsland; }
	void set_min_cell_span_count(int p_value) { minCellSpanCount = p_value; }
	int get_min_cell_span_count() const { return minCellSpanCount; }
	void set_max_verts_per_poly(int p_value) { maxVertsPerPoly = p_value; }
	int get_max_verts_per_poly() const { return maxVertsPerPoly; }
	void set_tile_size(int p_value) { tileSize = p_value; }
	int get_tile_size() const { return tileSize; }
	void set_layers_per_tile(int p_value) { layersPerTile = p_value; }
	int get_layers_per_tile() const { return layersPerTile; }
	void set_detail_sample_distance(float p_value) { detailSampleDistance = p_value; }
	float get_detail_sample_distance() const { return detailSampleDistance; }
	void set_detail_sample_max_error(float p_value) { detailSampleMaxError = p_value; }
	float get_detail_sample_max_error() const { return detailSampleMaxError; }
};

// Helper struct to store convex volume data
struct ConvexVolumeData {
	Array vertices;
	float height;
	unsigned char areaType;
};

// Helper struct for changed tiles
struct ChangedTileLayerData {
	int ref;
	int layer;
	bool doAll;
};

// Helper struct to store data about changed tile layers
struct ChangedTileLayers {
	std::pair<int, int> tilePos;
	std::vector<ChangedTileLayerData> layers;
};

// Representation of a single TileMesh and Crowd.
class DetourNavigationMesh : public Reference {
	GDCLASS(DetourNavigationMesh, Reference)

	RecastContext *_recastContext;
	rcConfig *_rcConfig;
	dtTileCache *_tileCache;
	dtNavMesh *_navMesh;
	dtNavMeshQuery *_navQuery;
	dtCrowd *_crowd;
	LinearAllocator *_allocator;
	FastLZCompressor *_compressor;
	MeshProcess *_meshProcess;
	DetourInputGeometry *_inputGeom;

	float _maxAgentSlope;
	float _maxAgentHeight;
	float _maxAgentClimb;
	float _maxAgentRadius;

	int _maxAgents;
	int _maxObstacles;
	int _maxLayers;
	int _navQueryMaxNodes;

	Vector2 _cellSize;
	int _tileSize;
	int _layersPerTile;

	int _navMeshIndex;

	std::map<int, ChangedTileLayers> _affectedTilesByVolume;
	std::map<int, ChangedTileLayers> _affectedTilesByConnection;

protected:
	static void _bind_methods();

public:
	void _init() {} // Called when .new() is called in gdscript

	// Initializes the navigation mesh & crowd.
	bool initialize(DetourInputGeometry *inputGeom, Ref<DetourNavigationMeshParameters> params, int maxObstacles, RecastContext *recastContext, int index);

	bool save(FileAccess *targetFile); // Will save this navmesh's current state to the passed file.

	// Loads and initializes the navmesh from the file.
	bool load(DetourInputGeometry *inputGeom, RecastContext *recastContext, FileAccess *sourceFile);

	// Rebuilds all tiles that have changed (by marking areas).
	void rebuildChangedTiles(const std::vector<int> &removedMarkedAreaIDs, const std::vector<int> &removedOffMeshConnections);

	// Adds an agent to the navigation.
	bool addAgent(Ref<DetourCrowdAgent> agent, Ref<DetourCrowdAgentParameters> parameters, bool main = true);

	void removeAgent(dtCrowdAgent *agent); // Remove the passed crowd agent.

	void addObstacle(Ref<DetourObstacle> obstacle); // Adds the passed obstacle to this navmesh.

	void update(float timeDeltaSeconds); // Updates the internal detour classes, agents, etc.

	// Create a debug representation of this navigation mesh and attach it to the MeshInstance as a mesh.
	void createDebugMesh(GodotDetourDebugDraw *debugDrawer, bool drawCacheBounds);

	dtCrowd *getCrowd(); // Get this navigation mesh's crowd.

	// getActorFitFactor Returns how well an actor with the passed stats would fit this navmesh's crowd.
	float getActorFitFactor(float agentRadius, float agentHeight);

	DetourNavigationMesh();
	~DetourNavigationMesh();

private:
	bool initializeCrowd(); // Initializes this mesh's crowd.

	// Rasterize all layers of this tile, preparing them to be in the tile cache.
	int rasterizeTileLayers(const int tileX, const int tileZ, const rcConfig &cfg, TileCacheData *tiles, const int maxTiles);

	void debugDrawTiles(GodotDetourDebugDraw *debugDrawer); // Draws the tiles using the passed debug drawer.

	void debugDrawObstacles(GodotDetourDebugDraw *debugDrawer); // Draws the obstacles using the passed debug drawer.
};

inline dtCrowd *DetourNavigationMesh::getCrowd() {
	return _crowd;
}

#endif // GODOTDETOURNAVIGATIONMESH_H
