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

#ifndef H_SPK_GD_RENDERER
#define H_SPK_GD_RENDERER

#include "SPK_Gd_DEF.h"
#include "Core/SPK_Renderer.h"

#include "scene/2d/canvas_item.h"

namespace SPK { namespace Godot {
	/**
	* @class GLRenderer
	* @brief An abstract Renderer for the Godot renderers
	*/
	class GLRenderer : public Renderer
	{
	public :

		////////////////
		// Destructor //
		////////////////

		/** @brief Destructor of GLRenderer */
		virtual  ~GLRenderer() {}

		/**
		* @brief Enables or disables the blending of this GLRenderer
		* @param blendingEnabled true to enable the blending, false to disable it
		*/
		virtual void enableBlending(bool blendingEnabled);
		virtual void setBlendMode(BlendMode blendMode);

		/**
		* @brief Tells whether blending is enabled for this GLRenderer
		* @return true if blending is enabled, false if it is disabled
		*/
		bool isBlendingEnabled() const;

	protected :
		GLRenderer(bool NEEDS_DATASET);

		/** @brief Inits the rendering hints of this GLRenderer */
		void initRenderingOptions() const;

	private :
		bool blendingEnabled;
		CanvasItemMaterial::BlendMode blendingMode;
	};

	inline GLRenderer::GLRenderer(bool NEEDS_DATASET) :
		Renderer(NEEDS_DATASET),
		blendingEnabled(false),
		blendingMode(CanvasItemMaterial::BLEND_MODE_MIX)
	{}

	inline void GLRenderer::enableBlending(bool blendingEnabled)
	{
		this->blendingEnabled = blendingEnabled;
	}

	inline bool GLRenderer::isBlendingEnabled() const
	{
		return blendingEnabled;
	}

	inline void GLRenderer::initRenderingOptions() const
	{
		// alpha test
		if (isRenderingOptionEnabled(RENDERING_OPTION_ALPHA_TEST))
		{
		}
		// depth write
		if (isRenderingOptionEnabled(RENDERING_OPTION_DEPTH_WRITE))
		{
		}
	}
}} // namespace

#endif // H_SPK_GD_RENDERER
