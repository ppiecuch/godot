/*************************************************************************/
/*  gd_timelinefx.cpp                                                    */
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

#include "gd_timelinefx.h"

#include "runtime/TLFXPugiXMLLoader.h"

#include "common/gd_pack.h"
#include "core/engine.h"
#include "misc/miniz.h"

static void _image_to_greyscale(Ref<Image> p_image, float p_fact);
static void _image_to_greyscale_trans(Ref<Image> &p_image);

GdTLFXEffectsLibrary::GdTLFXEffectsLibrary() {
	SetUpdateFrequency(Engine::get_singleton()->get_target_fps());
}

GdTLFXEffectsLibrary::~GdTLFXEffectsLibrary() {}

bool GdTLFXEffectsLibrary::LoadLibrary(const String &library, const String &filename, bool compile) {
	String libraryinfo = filename;

	struct zip_archive_t {
		zip_archive_t() { memset(&za, 0, sizeof(mz_zip_archive)); }
		~zip_archive_t() { mz_zip_reader_end(&za); }
		mz_bool init_file(const char *fn) { return mz_zip_reader_init_file_default(&za, fn); }
		mz_zip_archive *operator&() { return &za; }
		mz_zip_archive za;
	} zip_archive;

	// Now try to open the archive.
	mz_bool status = zip_archive.init_file(library.utf8().c_str());
	if (!status) {
		WARN_PRINT(vformat("[GdTLFXEffectsLibrary] Cannot open effects library %s", library));
		return false;
	}

	if (libraryinfo.empty()) {
		// Try to locate effect data file.
		for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zip_archive); i++) {
			mz_zip_archive_file_stat file_stat;
			if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
				WARN_PRINT("[GdTLFXEffectsLibrary] Cannot read effects library!");
				return false;
			}
			if (libraryinfo.empty() && strcasestr(file_stat.m_filename, "data.xml")) {
				libraryinfo = file_stat.m_filename;
				break;
			}
#if DEBUG_ENABLED
			print_verbose(vformat(
					"Archive summary:\n"
					"----------------"
					" filename: \"%s\"\n"
					" comment: \"%s\"\n"
					" uncompressed size: %u\n"
					" compressed size: %u\n"
					"Is dir: %u\n",
					file_stat.m_filename, file_stat.m_comment, file_stat.m_uncomp_size, (uint)file_stat.m_comp_size, mz_zip_reader_is_file_a_directory(&zip_archive, i)));
#endif
		}
	}

	if (libraryinfo.empty()) {
		WARN_PRINT("[GdTLFXEffectsLibrary] Cannot find library description file!");
		return false;
	}

	// Keep library we are using for effects
	_library = library;

	return Load(libraryinfo.utf8().c_str(), compile);
}

TLFX::XMLLoader *GdTLFXEffectsLibrary::CreateLoader() const {
	return new TLFX::PugiXMLLoader(_library.empty() ? 0 : _library.utf8().c_str());
}

TLFX::AnimImage *GdTLFXEffectsLibrary::CreateImage() const {
	return new GdImage();
}

bool GdTLFXEffectsLibrary::ensureTextureSize(int &w, int &h) {
	// for texture bigger then atlas fix the size to textureLimit:
	if (w > _atlas.atlasTextureSize().width || h > _atlas.atlasTextureSize().height) {
		real_t scale = MIN(_atlas.atlasTextureSizeLimit() / real_t(w),
				_atlas.atlasTextureSizeLimit() / real_t(h));
		w *= scale;
		h *= scale;
		return false; // donot scale - keep size
		// if greater than atlas limit, make the size scaled :
	} else if (w > _atlas.atlasTextureSizeLimit() || h > _atlas.atlasTextureSizeLimit()) {
		real_t scale = MIN(_atlas.atlasTextureSizeLimit() / real_t(w),
				_atlas.atlasTextureSizeLimit() / real_t(h));
		w *= scale;
		h *= scale;
	}
	return true; // scale
}

bool GdTLFXEffectsLibrary::UploadTextures() {
	// try calculate best fit into current atlas texture:
	int minw = 0, maxw = 0, minh = 0, maxh = 0;
	for (TLFX::AnimImage *shape : _shapeList) {
		const int w = shape->GetWidth();
		const int h = shape->GetHeight();
		if (w < minw)
			minw = w;
		if (h < minh)
			minh = h;
		if (w > maxw)
			maxw = w;
		if (h > maxh)
			maxh = h;
	}
#define SC(x) (sc / (1.1 + (x - minw) / (maxw - minw))) // 1 .. 2
	real_t sc = 1.5;
	bool done = false;
	while (!done && sc > 0) {
		QGL::QAreaAllocator m_allocator(_atlas.atlasTextureSize(), _atlas.padding);
		print_verbose(vformat("[GdTLFXEffectsLibrary] Scaling texture atlas with %f", sc));
		done = true;
		for (TLFX::AnimImage *shape : _shapeList) {
			const int anim_size = Math::pow(2, Math::ceil(Math::log(real_t(shape->GetFramesCount())) / Math_LN2));
			const int anim_square = Math::sqrt(real_t(anim_size));
			int w = shape->GetWidth() * anim_square;
			int h = shape->GetHeight() * anim_square;
			if (ensureTextureSize(w, h)) {
				w *= SC(w);
				h *= SC(h);
			}
			Rect2 rc = m_allocator.allocate(Size2(w, h));
			if (rc.size.width == 0 || rc.size.height == 0) {
				sc -= 0.05;
				done = false;
				break; // next step
			}
		}
	}
	if (!done) {
		WARN_PRINT("[GdTLFXEffectsLibrary] Cannot build texture atlas.");
		return false;
	}
	if (!_library.empty()) {
		struct zip_archive_t {
			zip_archive_t() { memset(&za, 0, sizeof(mz_zip_archive)); }
			~zip_archive_t() { mz_zip_reader_end(&za); }
			mz_bool init_file(const char *fn) { return mz_zip_reader_init_file_default(&za, fn); }
			mz_zip_archive *operator&() { return &za; }
			mz_zip_archive za;
		} zip_archive;

		// Now try to open the archive.
		mz_bool status = zip_archive.init_file(_library.utf8().c_str());
		if (status) {
			for (TLFX::AnimImage *shape : _shapeList) {
				const char *filename = shape->GetFilename();
				const int anim_size = Math::pow(2, Math::ceil(Math::log(real_t(shape->GetFramesCount())) / Math_LN2));
				const int anim_square = Math::sqrt(real_t(anim_size));
				int w = shape->GetWidth() * anim_square;
				int h = shape->GetHeight() * anim_square;
				if (ensureTextureSize(w, h)) {
					w *= SC(w);
					h *= SC(h);
				}

				if (filename == 0 || strlen(filename) == 0) {
					WARN_PRINT("[GdTLFXEffectsLibrary] Empty image filename");
					continue;
				}
				// Try to extract all the files to the heap.
				StringList variants;
				variants
						<< filename
						<< QFileInfo(filename).fileName()
						<< QFileInfo(String(filename).replace("\\", "/")).fileName();
				for (String fn : variants) {
					size_t uncomp_size;
					void *p = mz_zip_reader_extract_file_to_heap(&zip_archive, fn.utf8().c_str(), &uncomp_size, 0);
					if (p == 0) {
						continue; // Try next name
					}

					print_verbose(vformat("[GdTLFXEffectsLibrary] Successfully extracted file %s (%d bytes)", fn, uint64_t(uncomp_size)));

					Ref<Image> img = QImage::fromData((const uchar *)p, uncomp_size);

					if (img.is_null()) {
						WARN_PRINT(vformat("[GdTLFXEffectsLibrary] Failed to create image: %s", filename));
						return false;
					} else {
						switch (shape->GetImportOpt()) {
							case GdImage::impGreyScale:
								_image_to_greyscale_trans(img);
								break;
							case GdImage::impFullColour:
								break;
							case GdImage::impPassThrough:
								break;
							default:
								break;
						}

						// scale images to fit atlas
						QTexture *texture = _atlas.create(img.scaled(Size2(w, h)));
						((GdImage *)shape)->SetTexture(texture, filename);
						if (texture == 0) {
							WARN_PRINT(vformat("[GdTLFXEffectsLibrary] Failed to create texture for image %s (%d / %d frames)", filename, img->get_size(), shape->GetFramesCount()));
							return false;
						} else
							break;
					}
					// We're done.
					mz_free(p);
				}
				if (((GdImage *)shape)->GetTexture().is_null()) {
					WARN_PRINT(vformat("[GdTLFXEffectsLibrary] Failed to extract file %s", filename));
					return false;
				}
			}
			return true;
		} else {
			WARN_PRINT(vformat("[GdTLFXEffectsLibrary] Cannot open library file %s", _library));
			return false;
		}
	} else {
		for (TLFX::AnimImage *shape : _shapeList) {
			const char *filename = shape->GetFilename();
			const int anim_size = powf(2, Math::ceil(Math::log(real_t(shape->GetFramesCount())) / Math_LN2));
			const int anim_square = Math::sqrt(real_t(anim_size));
			int w = shape->GetWidth() * anim_square;
			int h = shape->GetHeight() * anim_square;
			if (ensureTextureSize(w, h)) {
				w *= SC(w);
				h *= SC(h);
			}

			QFile f(filename);
			if (!f.exists())
				f.setFileName(vformat("res:/data/%s", filename));
			if (!f.exists()) {
				WARN_PRINT(vformat("[GdImage] Failed to load image: %s", filename));
				return false;
			}
			Ref<Image> img = newref(Image);
			if (img->load(f.fileName()) != OK) {
				WARN_PRINT(vformat("[GdImage] Failed to load image: %s", filename));
				return false;
			} else {
				switch (shape->GetImportOpt()) {
					case GdImage::impGreyScale:
						_image_to_greyscale_trans(img);
						break;
					case GdImage::impFullColour:
						break;
					case GdImage::impPassThrough:
						break;
					default:
						break;
				}

				// scale images to fit atlas
				Ref<Texture> texture = _atlas.create(img.scaled(Size2(w, h)));
				((GdImage *)shape)->SetTexture(texture, filename);
				if (texture.is_null()) {
					WARN_PRINT(vformat("[GdImage] Failed to create texture for image %s (%d / %d frames)", filename, img->get_size(), shape->GetFramesCount()));
					return false;
				}
			}
		}
	}
	return true;
}

void GdTLFXEffectsLibrary::Debug(Ref<ArrayMesh> &mesh) {
	for (TLFX::AnimImage *sprite : _shapeList) {
		if (sprite->GetFramesCount() != 64)
			continue;

		// draw texture quad:

		glDisable(GL_DEPTH);
		glEnable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_TEXTURE_2D);
		dynamic_cast<GdImage *>(sprite)->GetTexture()->bind();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		static real_t f = 0;

		Rect2 rc = dynamic_cast<GdImage *>(sprite)->GetTexture()->normalizedTextureSubRect();
		const int anim_size = powf(2, ceilf(log2f(sprite->GetFramesCount())));
		const int anim_square = sqrtf(anim_size);
		const int anim_frame = int(round(f)) % sprite->GetFramesCount();
		f += 0.1;
		const int gr = anim_frame / anim_square, gc = anim_frame % anim_square;
		const float cw = rc.width() / anim_square, ch = rc.height() / anim_square;
		rc = Rect2(rc.x() + gc * cw, rc.y() + gr * ch, cw, ch);

		QGeometryData batch;
		batch.appendVertex(QVector3D(0, 0, 0));
		batch.appendVertex(QVector3D(sprite->GetWidth(), 0, 0));
		batch.appendVertex(QVector3D(sprite->GetWidth(), sprite->GetHeight(), 0));
		batch.appendVertex(QVector3D(0, sprite->GetHeight(), 0));
		batch.appendTexCoord(Vector2(rc.x(), rc.y()));
		batch.appendTexCoord(Vector2(rc.x() + rc.width(), rc.y()));
		batch.appendTexCoord(Vector2(rc.x() + rc.width(), rc.y() + rc.height()));
		batch.appendTexCoord(Vector2(rc.x(), rc.y() + rc.height()));
		batch.appendColor(Qt::white);
		batch.appendColor(Qt::white);
		batch.appendColor(Qt::white);
		batch.appendColor(Qt::white);
		batch.appendIndices(0, 1, 2);
		batch.appendIndices(2, 3, 0);
		batch.draw(p, 0, 6, QGL::Triangles);
		dynamic_cast<GdImage *>(sprite)->GetTexture()->release();

		return;
	}
}

GdTLFXParticleManager::GdTLFXParticleManager(QGLPainter *p, int particles /*= particleLimit*/, int layers /*= 1*/) :
		TLFX::ParticleManager(particles, layers), _lastTexture(0), _lastAdditive(true), _globalBlend(FromEffectBlendMode), _p(p) {
}

static void _build_tiles(Size2 grid_size, unsigned int total_frames, Point2 tex_origin = Point2(0, 0), Size2 tex_size = Size2(1, 1)) {
	Vector<Rect2> frames;
	for (int fr = 0; fr < grid_size.height() /* rows */; fr++)
		for (int fc = 0; fc < grid_size.width() /* cols */; fc++) {
			const float cw = tex_size.width() / grid_size.width(), ch = tex_size.height() / grid_size.height();
			frames.push_back(Rect2(tex_origin.x() + fc * cw, tex_origin.y() + fr * ch, cw, ch));
			if (frames.size() == total_frames)
				break;
		}
}

void GdTLFXParticleManager::DrawSprite(TLFX::Particle *p, TLFX::AnimImage *sprite, float px, float py, float frame, float x, float y, float rotation, float scaleX, float scaleY, unsigned char r, unsigned char g, unsigned char b, float a, bool additive) {
#define qFF(C) C *(255.999)

	PoolColorArray colors;
	PoolVector2Array verts, uvs;

	uint8_t alpha = qFF(a);
	if (alpha == 0 || scaleX == 0 || scaleY == 0)
		return;

	if ((_lastTexture && dynamic_cast<GdImage *>(sprite)->GetTexture()->textureId() != _lastTexture->textureId()) || (additive != _lastAdditive)) {
		Flush();
	}

	Rect2 rc = dynamic_cast<GdImage *>(sprite)->GetTexture()->normalizedTextureSubRect();
	// calculate frame position in atlas:
	if (sprite && sprite->GetFramesCount() > 1) {
		const int anim_size = powf(2, ceilf(log2f(sprite->GetFramesCount())));
		const int anim_square = sqrtf(anim_size);
		const int anim_frame = floorf(frame);
		if (anim_frame >= sprite->GetFramesCount()) {
			WARN_PRINT(vformat("[GdTLFXParticleManager] Out of range: %d of %d (frames: %d)", frame, anim_frame, sprite->GetFramesCount()));
		}
		const int gr = anim_frame / anim_square, gc = anim_frame % anim_square;
		const float cw = rc.width() / anim_square, ch = rc.height() / anim_square;
		rc = Rect2(rc.x() + gc * cw, rc.y() + gr * ch, cw, ch);
	}

	batch.appendTexCoord(Vector2(rc.x(), rc.y()));
	batch.appendTexCoord(Vector2(rc.x() + rc.width(), rc.y()));
	batch.appendTexCoord(Vector2(rc.x() + rc.width(), rc.y() + rc.height()));
	batch.appendTexCoord(Vector2(rc.x(), rc.y() + rc.height()));

	float x0 = -x * scaleX;
	float y0 = -y * scaleY;
	float x1 = x0;
	float y1 = (-y + sprite->GetHeight()) * scaleY;
	float x2 = (-x + sprite->GetWidth()) * scaleX;
	float y2 = y1;
	float x3 = x2;
	float y3 = y0;

	float cos = Math::cos(rotation / 180.f * M_PI);
	float sin = Math::sin(rotation / 180.f * M_PI);

	batch.appendVertex(Vector2(px + x0 * cos - y0 * sin, py + x0 * sin + y0 * cos));
	batch.appendVertex(Vector2(px + x1 * cos - y1 * sin, py + x1 * sin + y1 * cos));
	batch.appendVertex(Vector2(px + x2 * cos - y2 * sin, py + x2 * sin + y2 * cos));
	batch.appendVertex(Vector2(px + x3 * cos - y3 * sin, py + x3 * sin + y3 * cos));

	for (int i = 0; i < 4; ++i) {
		batch.appendColor(Color(r, g, b, alpha));
	}

	_lastTexture = dynamic_cast<GdImage *>(sprite)->GetTexture();
	switch (_globalBlend) {
		case FromEffectBlendMode:
			_lastAdditive = additive;
			break;
		case AddBlendMode:
			_lastAdditive = true;
			break;
		case AlphaBlendMode:
			_lastAdditive = false;
			break;
	}
}

void GdTLFXParticleManager::Flush() {
	if (batch.count()) {
		DEV_ASSERT(_p);

		glDisable(GL_DEPTH);
		glEnable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		if (_lastTexture) {
			glEnable(GL_TEXTURE_2D);
			_lastTexture->bind();
		} else
			glDisable(GL_TEXTURE_2D);
		if (_lastAdditive) {
			// ALPHA_ADD
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		} else {
			// ALPHA_BLEND
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		QGLBuilder builder;
		builder.addQuads(batch);
		QList<QGeometryData> opt = builder.optimized();
		for (QGeometryData gd : opt) {
			gd.draw(_p, 0, gd.indexCount());
		}
		if (_lastTexture)
			_lastTexture->release();
		batch = QGeometryData(); // clear batch data
	}
}

// Image utilities:

static void _image_to_greyscale(Ref<Image> p_image, float p_fact) {
	if (value == 0.0)
		return;

	p_image->lock();

	for (int y = 0; y < p_image->get_height(); ++y) {
		for (int x = 0; x < p_image->get_width(); ++x) {
			const Color data = p_image->get_pixel(x, y);
			const float gray = (data.r * .34 + data.g * .5 + data.b * .15

			if (p_fact == 1.0) {
				p_image->set_pixel(x, y, { gray, gray, gray, data.a });
			} else{
				p_image->set_pixel(x, y, {(p_fact*gray+(1-p_fact)*data.r, (p_fact*gray+(1-p_fact)*data.g, (p_fact*gray+(1-p_fact)*data.b, data.a});
			}
		}
	}
	p_image->unlock();
}

// For Local loc:Int = 0 Until pixmapcopy.capacity Step 4
//    p = pixmapcopy.pixels + loc
//    c = Min((p[0] *.3) + (p[1] *.59) + (p[2] *.11), p[3])
//    p[0] = 255
//    p[1] = p[0]
//    p[2] = p[0]
//    p[3] = c
// Next
static void _image_to_greyscale_trans(Ref<Image> &p_image) {
	p_image->lock();

	for (int y = 0; y < p_image->get_height(); ++y) {
		for (int x = 0; x < p_image->get_width(); ++x) {
			const Color data = p_image->get_pixel(x, y);
			const float gray = MIN(data.r * .3 + data.g * .59 + data.b * .11, data.a);
			p_image->set_pixel(x, y, { 255, 255, 255, gray });
		}
	}
	p_image->unlock();
}
