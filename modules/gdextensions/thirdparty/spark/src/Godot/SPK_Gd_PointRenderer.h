//////////////////////////////////////////////////////////////////////////////////
// SPARK particle engine														//
// Copyright (C) 2008-2013 - Julien Fryer - julienfryer@gmail.com				//
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

#ifndef H_SPK_GD_POINTRENDERER
#define H_SPK_GD_POINTRENDERER

#include "SPK_Gd_Renderer.h"
#include "Extensions/Renderers/SPK_PointRenderBehavior.h"

#include "core/math/math_funcs.h"
#include "scene/2d/canvas_item.h"
#include "scene/resources/texture.h"

namespace SPK { namespace Godot {
	/**
	* @class GLPointRenderer
	* @brief A Renderer drawing drawing particles as OpenGL points
	*
	* OpenGL points can be configured to render them in 3 different ways :
	* <ul>
	* <li>SPK::POINT_SQUARE : standard OpenGL points</li>
	* <li>SPK::POINT_CIRCLE : antialiased OpenGL points</li>
	* <li>SPK::POINT_SPRITE : OpenGL point sprites (must be supported by the hardware)</li>
	* </ul>
	* Moreover, points size can either be defined in screen space (in pixels) or in the universe space (must be supported by the hardware).
	* The advantage of the universe space is that points size on the screen will be dependant to their distance from the camera, whereas in screen space
	* all points will have the same size on the screen no matter what their distance from the camera is.
	* <br>
	* This renderer do not use any parameters of particles.
	*/
	class GLPointRenderer :	public GLRenderer, public PointRenderBehavior
	{
        SPK_IMPLEMENT_OBJECT(GLPointRenderer);

	public :

		/**
		* @brief Creates and registers a new GLPointRenderer
		* @param size : the size of the points
		* @return A new registered GLPointRenderer
		*/
		static  Ref<GLPointRenderer> create(CanvasItem *canvas,float screenSize = 1.0f);

		virtual bool setType(PointType type);

		/**
		* @brief Sets the texture of this GLPointRenderer
		*
		* Note that the texture is only used if point sprites are used (type is set to SPK::POINT_SPRITE)
		*
		* @param textureIndex : the index of the OpenGL texture of this GLPointRenderer
		*/
		void setTexture(GdTexture textureIndex);

		/**
		* @brief Gets the texture of this GLPointRenderer
		* @return the texture of this GLPointRenderer
		*/
		GdTexture getTexture() const;

	private :
		GdTexture textureIndex;

		GLPointRenderer(CanvasItem *canvas = nullptr,float screenSize = 1.0f);
		GLPointRenderer(const GLPointRenderer& renderer);

		virtual void render(const Group& group,const DataSet* dataSet,RenderBuffer* renderBuffer) const;
		virtual void computeAABB(Vector3D& AABBMin,Vector3D& AABBMax,const Group& group,const DataSet* dataSet) const;
	};

	inline GLPointRenderer::GLPointRenderer(CanvasItem *canvas,float screenSize) :
		GLRenderer(canvas,false),
		PointRenderBehavior(POINT_TYPE_SQUARE,screenSize)
	{}

	inline GLPointRenderer::GLPointRenderer(const GLPointRenderer& renderer) :
		GLRenderer(renderer),
		PointRenderBehavior(renderer),
		textureIndex(renderer.textureIndex)
	{}

	inline Ref<GLPointRenderer> GLPointRenderer::create(CanvasItem *canvas,float screenSize)
	{
		return SPK_NEW(GLPointRenderer,canvas,screenSize);
	}
		
	inline void GLPointRenderer::setTexture(GdTexture textureIndex)
	{
		this->textureIndex = textureIndex;
	}

	inline GdTexture GLPointRenderer::getTexture() const
	{
		return textureIndex;
	}
}} // namespace

#endif // H_SPK_GD_POINTRENDERER
