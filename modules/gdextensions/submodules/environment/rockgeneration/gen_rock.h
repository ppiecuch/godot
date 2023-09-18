#ifndef GEN_ROCK_H
#define GEN_ROCK_H

#include "rock_header.h"

#include "core/local_vector.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"

class GenRock {
	bool m_PostInitialize = true;
	real_t m_Width, m_Height, m_Depth;
	real_t m_MinRandAngle, m_MaxRandAngle, m_MaxOffsetPercent, m_MaxRandShift;
	Vector2 m_PrevAngles = Vector2(0, 0);
	uint32_t m_MaxPlaneVerts, m_MinPlaneVerts, m_MaxPlanes;
	uint32_t m_Steps;

	std::set<uint32_t> m_NorthIdx, m_SouthIdx;

	struct {
		LocalVector<Vector3> Position;
		LocalVector<Vector3> Normal;
		LocalVector<Vector3> Tangent;
		LocalVector<Vector2> TexCoord;

		VertexRock operator[](size_t index) const {
			VertexRock v;
			v.Position = Position[index];
			v.Normal = Normal[index];
			v.Tangent = Tangent[index];
			v.TexCoord = TexCoord[index];
			return v;
		}
		_FORCE_INLINE_ void push_back(const VertexRock &v) {
			Position.push_back(v.Position);
			Normal.push_back(v.Normal);
			Tangent.push_back(v.Tangent);
			TexCoord.push_back(v.TexCoord);
		}
		_FORCE_INLINE_ void clear() {
			Position.clear();
			Normal.clear();
			Tangent.clear();
			TexCoord.clear();
		}
		_FORCE_INLINE_ bool valid(size_t expected) const {
			return Position.size() == expected && Normal.size() == expected && Tangent.size() == expected && TexCoord.size() == expected;
		}
		_FORCE_INLINE_ bool valid() const {
			return valid(Position.size());
		}
	} m_VecGeom;
	LocalVector<int> m_VecIndices;
	size_t m_NumVertices, m_NumIndices;

	Ref<ArrayMesh> mesh;

	void BuildIco();
	void BuildRock();
	void Expand();
	void CorrectUV();
	void BuildNormals();
	void BuildTangents();

	void _update();

public:
	struct Plane {
		Vector3 origin;
		Vector3 normal;
		int size;
	};

	void Reset() { m_PostInitialize = true; }

	void SetRadiusWidth(real_t width) { m_Width = width; }
	void SetRadiusDepth(real_t depth) { m_Depth = depth; }
	void SetRadiusHeight(real_t height) { m_Height = height; }

	void SetRandAngleMax(real_t angle) { m_MaxRandAngle = angle; }
	void SetRandAngleMin(real_t angle) { m_MinRandAngle = angle; }
	void SetRandOffsetPercent(real_t percent) { m_MaxOffsetPercent = percent; }
	void SetRandShift(real_t shift) { m_MaxRandShift = shift; }

	void SetMaxPlaneVerts(uint32_t verts) { m_MaxPlaneVerts = verts; }
	void SetMinPlaneVerts(uint32_t verts) { m_MinPlaneVerts = verts; }
	void SetMaxPlanes(uint32_t planes) { m_MaxPlanes = planes; }

	void SetSteps(uint32_t steps) { m_Steps = steps; }

	GenRock(real_t width, real_t height, real_t depth, int steps);
	~GenRock(void);
};

#endif // GEN_ROCK_H
