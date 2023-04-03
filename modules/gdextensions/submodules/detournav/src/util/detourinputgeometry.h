/**************************************************************************/
/*  detourinputgeometry.h                                                 */
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

#ifndef DETOURINPUTGEOMETRY_H
#define DETOURINPUTGEOMETRY_H

// This code is mostly taken from recastnavigation's demo project. Just slightly adjusted to fit within godotdetour.
// Most thanks go to Mikko Mononen and maintainers for this.

//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//

#include "core/os/file_access.h"
#include "scene/3d/mesh_instance.h"

#include "chunkytrimesh.h"

class MeshDataAccumulator;

static const int MAX_CONVEXVOL_PTS = 12;

struct ConvexVolume {
	float verts[MAX_CONVEXVOL_PTS * 3];
	float hmin, hmax;
	int nverts;
	int area;

	// 2D Rectangle around the area, not precise but "good enough" to check if it hits tiles
	float front;
	float back;
	float left;
	float right;

	bool isNew = false;
};

class DetourInputGeometry {
	static const int MAX_OFFMESH_CONNECTIONS = 256;
	static const int MAX_VOLUMES = 256;

private:
	rcChunkyTriMesh *m_chunkyMesh;
	MeshDataAccumulator *m_mesh;
	float m_meshBMin[3], m_meshBMax[3];

	// Off-Mesh connections.
	float m_offMeshConVerts[MAX_OFFMESH_CONNECTIONS * 3 * 2];
	float m_offMeshConRads[MAX_OFFMESH_CONNECTIONS];
	unsigned char m_offMeshConDirs[MAX_OFFMESH_CONNECTIONS];
	unsigned char m_offMeshConAreas[MAX_OFFMESH_CONNECTIONS];
	unsigned short m_offMeshConFlags[MAX_OFFMESH_CONNECTIONS];
	unsigned int m_offMeshConId[MAX_OFFMESH_CONNECTIONS];
	bool m_offMeshConNew[MAX_OFFMESH_CONNECTIONS];
	int m_offMeshConCount;

	// Convex Volumes.
	ConvexVolume m_volumes[MAX_VOLUMES];
	int m_volumeCount;

public:
	bool loadMesh(class rcContext *ctx, MeshInstance *inputMesh);
	void clearData();

	bool save(FileAccessRef &targetFile); // Save the input geometry data to the file.
	bool load(FileAccessRef &sourceFile); // Load the input geometry data from the byte array.

	// Method to return static mesh data.
	const MeshDataAccumulator *getMesh() const { return m_mesh; }
	const float *getMeshBoundsMin() const { return m_meshBMin; }
	const float *getMeshBoundsMax() const { return m_meshBMax; }
	const float *getNavMeshBoundsMin() const { return m_meshBMin; }
	const float *getNavMeshBoundsMax() const { return m_meshBMax; }
	const rcChunkyTriMesh *getChunkyMesh() const { return m_chunkyMesh; }
	bool raycastMesh(float *src, float *dst, float &tmin);

	// Off-Mesh connections.
	int getOffMeshConnectionCount() const { return m_offMeshConCount; }
	const float *getOffMeshConnectionVerts() const { return m_offMeshConVerts; }
	const float *getOffMeshConnectionRads() const { return m_offMeshConRads; }
	const unsigned char *getOffMeshConnectionDirs() const { return m_offMeshConDirs; }
	const unsigned char *getOffMeshConnectionAreas() const { return m_offMeshConAreas; }
	const unsigned short *getOffMeshConnectionFlags() const { return m_offMeshConFlags; }
	const unsigned int *getOffMeshConnectionId() const { return m_offMeshConId; }
	bool *getOffMeshConnectionNew() { return m_offMeshConNew; }
	void addOffMeshConnection(const float *spos, const float *epos, const float rad, unsigned char bidir, unsigned char area, unsigned short flags);
	void deleteOffMeshConnection(int i);
	void drawOffMeshConnections(struct duDebugDraw *dd, bool hilight = false);

	// Box Volumes.
	int getConvexVolumeCount() const { return m_volumeCount; }
	ConvexVolume *getConvexVolumes() { return m_volumes; }
	void addConvexVolume(const float *verts, const int nverts, const float minh, const float maxh, unsigned char area);
	void deleteConvexVolume(int i);
	void drawConvexVolumes(struct duDebugDraw *dd, bool hilight = false);

	DetourInputGeometry();
	~DetourInputGeometry();

private:
	// Explicitly disabled copy constructor and copy assignment operator.
	DetourInputGeometry(const DetourInputGeometry &);
	DetourInputGeometry &operator=(const DetourInputGeometry &);
};

#endif // DETOURINPUTGEOMETRY_H
