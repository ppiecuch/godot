//////////////////////////////////////////////////////////////////////////////////
// SPARK particle engine														//
// Copyright (C) 2008-2011 - Julien Fryer - julienfryer@gmail.com				//
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
#include "SPK_Gd_Renderer.h"

namespace SPK { namespace Godot {
	void GLRenderer::setBlendMode(BlendMode blendMode)
	{
		switch(blendMode)
		{
		case BLEND_MODE_NONE :
			blendingMode = CanvasItemMaterial::BLEND_MODE_DISABLED;
			blendingEnabled = false;
			break;

		case BLEND_MODE_ADD :
			blendingMode = CanvasItemMaterial::BLEND_MODE_ADD;
			blendingEnabled = true;
			break;

		case BLEND_MODE_ALPHA :
			blendingMode = CanvasItemMaterial::BLEND_MODE_MIX;
			blendingEnabled = true;
			break;

		default :
			SPK_LOG_WARNING("GLRenderer::setBlendMode(BlendMode) - Unsupported blending mode. Nothing happens");
			break;
		}
	}
}} // namespace
