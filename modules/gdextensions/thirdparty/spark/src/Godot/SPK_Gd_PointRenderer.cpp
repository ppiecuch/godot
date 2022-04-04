//////////////////////////////////////////////////////////////////////////////////
// SPARK particle engine														//
// Copyright (C) 2008-2009 - Julien Fryer - julienfryer@gmail.com				//
//																				//
// This software is provided 'as-is', without any express or implied			//
// warranty.  In no event will the authors be held liable for any damages		//
// arising from the use of this software.										//
//																				//
// Permission is granted to anyone to use this software for any purpose,		//
// including commercial applications, and to alter it and redistribute it		//
// freely, subject to the following restrictions:								//
//																				//
// 1. The origin of this software must not be misrepresented; you must not		//
//    claim that you wrote the original software. If you use this software		//
//    in a product, an acknowledgment in the product documentation would be		//
//    appreciated but is not required.											//
// 2. Altered source versions must be plainly marked as such, and must not be	//
//    misrepresented as being the original software.							//
// 3. This notice may not be removed or altered from any source distribution.	//
//////////////////////////////////////////////////////////////////////////////////

#include "SPARK_Core.h"
#include "SPK_Gd_PointRenderer.h"

namespace SPK { namespace Godot {
	bool GLPointRenderer::setType(PointType type)
	{
		if (type == POINT_TYPE_SPRITE)
		{
			SPK_LOG_WARNING("GLPointRenderer::setType(PointType) - POINT_TYPE_SPRITE is not available on the hardware");
			return false;
		}

		this->type = type;
		return true;
	}

	void GLPointRenderer::render(const Group& group,const DataSet* dataSet,RenderBuffer* renderBuffer) const
	{
		// Sets the different states for rendering
		initBlending();
		initRenderingOptions();

		switch(type)
		{
		case POINT_TYPE_SQUARE :
			break;

		case POINT_TYPE_SPRITE :
			break;

		case POINT_TYPE_CIRCLE :
			break;
		}

		// RenderBuffer is not used as the data are already well organized for rendering

		PoolVector3Array vertexArray = _from_raw_buffer<PoolVector3Array>(static_cast<const Vector3D*>(group.getPositionAddress()), group.getNbParticles());
		PoolColorArray colorArray = _from_raw_buffer<PoolColorArray>(static_cast<const Color*>(group.getColorAddress()), group.getNbParticles());

		Array array;
		array.resize(ArrayMesh::ARRAY_MAX);
		array[ArrayMesh::ARRAY_VERTEX] = vertexArray;
		array[ArrayMesh::ARRAY_COLOR] = colorArray;

		const_cast<GLPointRenderer*>(this)->addRenderLayer(Mesh::PRIMITIVE_POINTS, array);
	}

	void GLPointRenderer::computeAABB(Vector3D& AABBMin,Vector3D& AABBMax,const Group& group,const DataSet* dataSet) const
	{
		for (ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
		{
			AABBMin.setMin(particleIt->position());
			AABBMax.setMax(particleIt->position());
		}
	}
}} // namespace
