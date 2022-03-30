#pragma once

// Godot platform implementation (PugiXML for parsing data)

#ifndef GD_TIMELINEFX_H
#define GD_TIMELINEFX_H

#include "runtime/TLFXEffectsLibrary.h"
#include "runtime/TLFXParticleManager.h"
#include "runtime/TLFXAnimImage.h"
#include "runtime/TLFXParticle.h"

#include "core/reference.h"
#include "scene/resources/texture.h"

class XMLLoader;

class GdImage : public TLFX::AnimImage {
	String _image;
	Ref<Texture> _texture;

public:
	Ref<Texture> GetTexture() const { return _texture; }
	void SetTexture(Ref<Texture> texture, const String &imageName) { _texture = texture; _image = imageName; }
	void SetTexture(Ref<Texture> texture) { _texture = texture; }
	String GetImageName() const { return _image; }

	virtual bool Load();

	GdImage() : _texture(0) { }
};

class GdTLFXEffectsLibrary : public TLFX::EffectsLibrary, public Reference {
	GDCLASS(GdTLFXEffectsLibrary, Reference);

	String _library;
	Ref<TextureAtlas> _atlas;

protected:
	static void _bind_methods();

public:
	bool LoadLibrary(const char *library, const char *filename = 0, bool compile = true);
	void ClearAll(Size2 reqAtlasSize = Size2()) {
		TLFX::EffectsLibrary::ClearAll();
		_atlas->invalidate(reqAtlasSize);
	}

	virtual TLFX::XMLLoader *CreateLoader() const;
	virtual TLFX::AnimImage *CreateImage() const;

	uint32_t TextureAtlas() const { return _atlas->atlasTextureId(); }
	Size2 TextureAtlasSize() const { return _atlas->atlasTextureSize(); }

	bool ensureTextureSize(int &w, int &h);
	bool UploadTextures();

	void Debug();

	GdTLFXEffectsLibrary();
	~GdTLFXEffectsLibrary();
};

class GdTLFXParticleManager : public TLFX::ParticleManager, public Object {
	GDCLASS(GdTLFXParticleManager, Object);

	struct Batch {
		real_t px, py;
		real_t frame;
		real_t x, y;
		real_t rotation;
		real_t scaleX, scaleY;
		Color color;
	};
	std::list<Batch> _batch;
	Ref<Texture> _lastTexture;
	bool _lastAdditive;
	GlobalBlendModeType _globalBlend;

	virtual void DrawSprite(TLFX::Particle *p, TLFX::AnimImage* sprite, float px, float py, float frame, float x, float y, float rotation, float scaleX, float scaleY, unsigned char r, unsigned char g, unsigned char b, float a, bool additive);

protected:
	static void _bind_methods();

public:
	enum GlobalBlendModeType {
		FromEffectBlendMode,
		AddBlendMode,
		AlphaBlendMode,
		GlobalBlendModesNum
	};

	void Reset() {
		Destroy();
		batch = QGeometryData();
		_lastTexture = 0;
		_lastAdditive = true;
	}
	void Flush();

	GlobalBlendModeType GlobalBlendMode() { return _globalBlend; }
	String GlobalBlendModeInfo() {
		switch(_globalBlend) {
			case FromEffectBlendMode: return "effect blend";
			case AddBlendMode: return "addictive blend";
			case AlphaBlendMode: return "alpha blend";
			default: return "<undefined>";
		}
	}
	void SetGlobalBlendMode(GlobalBlendModeType state) { _globalBlend = state; }
	void ToggleGlobalBlendMode() { _globalBlend = GlobalBlendModeType((int(_globalBlend)+1)%GlobalBlendModesNum); }

	GdTLFXParticleManager(int particles = TLFX::ParticleManager::particleLimit, int layers = 1);
};

#endif // GD_TIMELINEFX_H
