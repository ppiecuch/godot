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

		inline void addRenderLayer(Mesh::PrimitiveType primitive, const Array &mesh_array);
		inline void addRenderLayer(Mesh::PrimitiveType primitive, const Array &mesh_array, const GdTexture &texture);

		void flushRender();

		Transform2D getTransform() const { return canvas->get_global_transform(); }

	protected :
		GLRenderer(CanvasItem *canvas,bool NEEDS_DATASET);

		/** @brief Inits the blending of this GLRenderer */
		void initBlending() const;
		/** @brief Inits the rendering hints of this GLRenderer */
		void initRenderingOptions() const;

	private :
		bool blendingEnabled;
		CanvasItemMaterial::BlendMode blendingMode;
		CanvasItem *canvas;
		static GdArrayMesh _mesh;
		static GdTexture _lastTexture;
	};

	inline GLRenderer::GLRenderer(CanvasItem *canvas,bool NEEDS_DATASET) :
		Renderer(NEEDS_DATASET),
		blendingEnabled(false),
		blendingMode(CanvasItemMaterial::BLEND_MODE_MIX),
		canvas(canvas)
	{}

	inline void GLRenderer::enableBlending(bool blendingEnabled)
	{
		this->blendingEnabled = blendingEnabled;
	}

	inline bool GLRenderer::isBlendingEnabled() const
	{
		return blendingEnabled;
	}

	inline void GLRenderer::initBlending() const
	{
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

	inline void GLRenderer::addRenderLayer(Mesh::PrimitiveType primitive, const Array &mesh_array, const GdTexture &texture)
	{
		if (texture != _lastTexture && _mesh->get_surface_count()) {
			flushRender();
			_lastTexture = texture;
		}
		_mesh->add_surface_from_arrays(primitive, mesh_array, Array());
	}

	inline void GLRenderer::addRenderLayer(Mesh::PrimitiveType primitive, const Array &mesh_array)
	{
		if (_lastTexture && _mesh->get_surface_count())
		{
			flushRender();
			_lastTexture = GdTexture(nullptr);
		}
		_mesh->add_surface_from_arrays(primitive, mesh_array, Array());
	}

	inline void GLRenderer::flushRender() {
		if (_mesh->get_surface_count())
		{
			canvas->draw_mesh(_mesh, _lastTexture);
			_mesh->clear_mesh(); // restart
		}
	}

	/** @brief Rendering utilities */
	template <typename PoolOut, typename VecIn>
	PoolOut _from_raw_buffer(const VecIn *in, int inCount) {
		PoolOut r;
		if (inCount)
		{
			memcpy(r.write().ptr(), in, sizeof(VecIn) * inCount);
		}
		return r;
	}
}} // namespace

#endif // H_SPK_GD_RENDERER
