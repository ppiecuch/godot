/*************************************************************************/
/*  gd_timelinefx.h                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

// Godot platform implementation (PugiXML for parsing data)

#ifndef GD_TIMELINEFX_H
#define GD_TIMELINEFX_H

#include "runtime/TLFXAnimImage.h"
#include "runtime/TLFXEffectsLibrary.h"
#include "runtime/TLFXParticle.h"
#include "runtime/TLFXParticleManager.h"

#include "common/gd_core.h"
#include "core/reference.h"
#include "scene/2d/canvas_item.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"

class XMLLoader;

class GdImage : public TLFX::AnimImage {
	String _image;
	Ref<Texture> _texture;

public:
	Ref<Texture> GetTexture() const { return _texture; }
	void SetTexture(Ref<Texture> texture, const String &imageName) {
		_texture = texture;
		_image = imageName;
	}
	void SetTexture(Ref<Texture> texture) { _texture = texture; }
	String GetImageName() const { return _image; }
	Rect2 normalizedTextureSubRect() const {
		if (const AtlasTexture *t = Object::cast_to<AtlasTexture>(*_texture)) {
			return t->get_region();
		}
		return Rect2(0, 0, 1, 1);
	}

	virtual bool Load() { return true; }

	GdImage() {}
};

class GdTLFXEffectsLibrary : public TLFX::EffectsLibrary, public Reference {
	GDCLASS(GdTLFXEffectsLibrary, Reference);

	String _library;
	struct {
		Ref<ImageTexture> texture;
		int sizeLimit = 256;
		Size2 padding{ 2, 2 };

		Ref<Texture> getTexture() const { return texture; }
		Size2 atlasTextureSize() const { return texture->get_size(); }
		int atlasTextureSizeLimit() const { return sizeLimit; }
		void clearAll(int limit = 0) {
			texture = newref(ImageTexture);
			if (limit)
				sizeLimit = limit;
		}
		Ref<Texture> create(const Ref<Image> &image) { return nullptr; }
	} _atlas;

	const Color white = Color::named("white");

	PoolVector2Array verts;
	PoolVector2Array uvs;
	PoolColorArray colors;
	PoolIntArray indexes;

protected:
	static void _bind_methods();

public:
	bool LoadLibrary(const String &library, const String &filename = "", bool compile = true);
	void ClearAll(Size2 reqAtlasSize = Size2()) {
		TLFX::EffectsLibrary::ClearAll();
		_atlas.clearAll();
	}

	virtual TLFX::XMLLoader *CreateLoader() const;
	virtual TLFX::AnimImage *CreateImage() const;

	Ref<Texture> TextureAtlas() const { return _atlas.getTexture(); }
	Size2 TextureAtlasSize() const { return _atlas.atlasTextureSize(); }

	bool ensureTextureSize(int &w, int &h);
	bool UploadTextures();

	void Debug(Ref<ArrayMesh> &mesh);

	GdTLFXEffectsLibrary();
	~GdTLFXEffectsLibrary();
};

class GdTLFXParticleManager : public TLFX::ParticleManager, public Reference {
	GDCLASS(GdTLFXParticleManager, Object);

public:
	enum GlobalBlendModeType {
		FromEffectBlendMode,
		AddBlendMode,
		AlphaBlendMode,
	};

private:
	PoolVector2Array verts;
	PoolVector2Array uvs;
	PoolColorArray colors;
	PoolIntArray indexes;

	Ref<ArrayMesh> _mesh;
	Ref<CanvasItem> *_canvas;
	Ref<CanvasItemMaterial> _mat_add, _mat_mul;

	Ref<Texture> _lastTexture;
	bool _lastAdditive;
	GlobalBlendModeType _globalBlend;

	void reset() {
		verts.reset();
		uvs.reset();
		colors.reset();
		indexes.reset();
	}

	virtual void DrawSprite(TLFX::Particle *p, TLFX::AnimImage *sprite, float px, float py, float frame, float x, float y, float rotation, float scaleX, float scaleY, unsigned char r, unsigned char g, unsigned char b, float a, bool additive);

protected:
	static void _bind_methods();

public:
	void Reset() {
		Destroy();
		_lastTexture = Ref<Texture>(nullptr);
		_lastAdditive = true;
	}
	void Flush();

	GlobalBlendModeType GlobalBlendMode() { return _globalBlend; }
	String GlobalBlendModeInfo() {
		switch (_globalBlend) {
			case FromEffectBlendMode:
				return "effect blend";
			case AddBlendMode:
				return "addictive blend";
			case AlphaBlendMode:
				return "alpha blend";
			default:
				return "<undefined>";
		}
	}
	void SetGlobalBlendMode(GlobalBlendModeType state) { _globalBlend = state; }

	GdTLFXParticleManager(int particles = TLFX::ParticleManager::particleLimit, int layers = 1);
};

#endif // GD_TIMELINEFX_H
