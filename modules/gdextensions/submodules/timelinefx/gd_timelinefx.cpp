/**************************************************************************/
/*  gd_timelinefx.cpp                                                     */
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

#include "gd_timelinefx.h"

#include "runtime/TLFXPugiXMLLoader.h"

#include "common/gd_pack.h"
#include "core/engine.h"
#define MINIZ_HEADER_FILE_ONLY
#include "misc/miniz.h"

#ifdef _MSC_VER
#define strcasestr stristr
#endif

#ifndef M_PI
#define M_PI Math_PI
#endif

struct AreaAllocatorNode;

class AreaAllocator {
	AreaAllocatorNode *root;
	Size2 areaSize, padding;

	bool allocateInNode(const Size2 &size, Point2 &result, const Rect2 &currentRect, AreaAllocatorNode *node);
	bool deallocateInNode(const Point2 &pos, AreaAllocatorNode *node);
	void mergeNodeWithNeighbors(AreaAllocatorNode *node);

public:
	Rect2 allocate(const Size2 &size);
	bool allocate(Rect2 &req);
	bool deallocate(const Rect2 &rect);
	bool isEmpty() const { return root == 0; }
	Size2 size() const { return areaSize; }

	AreaAllocator(const Size2 &size, const Size2 &padding = Size2());
	~AreaAllocator();
};

static void _image_to_greyscale(Ref<Image> p_image, float p_fact);
static void _image_to_greyscale_trans(Ref<Image> &p_image);

#define _Gd(o) ((GdImage *)o)

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
					" is dir: %u\n",
					file_stat.m_filename, file_stat.m_comment, file_stat.m_uncomp_size, file_stat.m_comp_size, mz_zip_reader_is_file_a_directory(&zip_archive, i)));
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
	return new TLFX::PugiXMLLoader(_library.utf8().c_str());
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
		AreaAllocator allocator(_atlas.atlasTextureSize(), _atlas.padding);
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
			Rect2 rc = allocator.allocate(Size2(w, h));
			if (rc.size.width == 0 || rc.size.height == 0) {
				sc -= 0.05;
				done = false;
				break; // next try
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
				const String filename = shape->GetFilename();
				const int anim_size = Math::pow(2, Math::ceil(Math::log(real_t(shape->GetFramesCount())) / Math_LN2));
				const int anim_square = Math::sqrt(real_t(anim_size));
				int w = shape->GetWidth() * anim_square;
				int h = shape->GetHeight() * anim_square;
				if (ensureTextureSize(w, h)) {
					w *= SC(w);
					h *= SC(h);
				}

				if (filename.empty()) {
					WARN_PRINT("[GdTLFXEffectsLibrary] Empty image filename");
					continue;
				}
				// Try to extract all the files to the heap.
				Vector<String> variants = make_vector(filename, filename.get_file(), filename.replace("\\", "/").get_file());
				for (String fn : variants) {
					size_t uncomp_size;
					void *p = mz_zip_reader_extract_file_to_heap(&zip_archive, fn.utf8().c_str(), &uncomp_size, 0);
					if (p == 0) {
						continue; // Try next name
					}

					print_verbose(vformat("[GdTLFXEffectsLibrary] Successfully extracted file %s (%d bytes)", fn, uint64_t(uncomp_size)));

					Ref<Image> img = memnew(Image((uint8_t *)p, uncomp_size));

					if (img->empty()) {
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

						img->resize(w, h); // scale images to fit atlas
						Ref<Texture> texture = _atlas.create(img);
						_Gd(shape)->SetTexture(texture, filename);
						if (texture.is_null()) {
							WARN_PRINT(vformat("[GdTLFXEffectsLibrary] Failed to create texture for image %s (%d / %d frames)", filename, img->get_size(), shape->GetFramesCount()));
							return false;
						} else {
							break;
						}
					}
					// We're done.
					mz_free(p);
				}
				if (_Gd(shape)->GetTexture().is_null()) {
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
			const int anim_size = Math::pow(2, Math::ceil(Math::log(real_t(shape->GetFramesCount())) / Math_LN2));
			const int anim_square = Math::sqrt(real_t(anim_size));
			int w = shape->GetWidth() * anim_square;
			int h = shape->GetHeight() * anim_square;
			if (ensureTextureSize(w, h)) {
				w *= SC(w);
				h *= SC(h);
			}

			String f = filename;
			if (!FileAccess::exists(f)) {
				f = vformat("res:/data/%s", filename);
			}
			if (!FileAccess::exists(f)) {
				WARN_PRINT(vformat("[GdImage] Failed to load image: %s", filename));
				return false;
			}
			Ref<Image> img = newref(Image);
			if (img->load(filename) != OK) {
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

				img->resize(w, h); // scale images to fit atlas
				Ref<Texture> texture = _atlas.create(img);
				_Gd(shape)->SetTexture(texture, filename);
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

		static real_t f = 0;

		Rect2 rc = _Gd(sprite)->normalizedTextureSubRect();
		const int anim_size = Math::pow(2, Math::ceil(Math::log(real_t(sprite->GetFramesCount())) / Math_LN2));
		const int anim_square = Math::sqrt(real_t(anim_size));
		const int anim_frame = int(Math::round(f)) % sprite->GetFramesCount();
		f += 0.1;
		const int gr = anim_frame / anim_square, gc = anim_frame % anim_square;
		const real_t cw = rc.size.width / anim_square, ch = rc.size.height / anim_square;
		rc = Rect2(rc.position.x + gc * cw, rc.position.y + gr * ch, cw, ch);

		verts.push_back({ 0, 0 });
		verts.push_back({ sprite->GetWidth(), 0 });
		verts.push_back({ sprite->GetWidth(), sprite->GetHeight() });
		verts.push_back({ 0, sprite->GetHeight() });
		uvs.push_back({ rc.position.x, rc.position.y });
		uvs.push_back({ rc.position.x + rc.size.width, rc.position.y });
		uvs.push_back({ rc.position.x + rc.size.width, rc.position.y + rc.size.height });
		uvs.push_back({ rc.position.x, rc.position.y + rc.size.height });
		colors.push_multi(4, white);
		indexes.append_array(parray(0, 1, 2, 2, 3, 0));

		return;
	}
}

GdTLFXParticleManager::GdTLFXParticleManager(int particles, int layers) :
		TLFX::ParticleManager(particles, layers), _lastTexture(0), _lastAdditive(true), _globalBlend(FromEffectBlendMode) {
}

#define gFF(C) (C * (255.999f))

void GdTLFXParticleManager::DrawSprite(TLFX::Particle *p, TLFX::AnimImage *sprite, float px, float py, float frame, float x, float y, float rotation, float scaleX, float scaleY, unsigned char r, unsigned char g, unsigned char b, float a, bool additive) {
	if (a == 0 || scaleX == 0 || scaleY == 0) {
		return;
	}

	if ((_lastTexture && _Gd(sprite)->GetTexture() != _lastTexture) || (additive != _lastAdditive)) {
		Flush();
	}

	Rect2 rc = _Gd(sprite)->normalizedTextureSubRect();
	// calculate frame position in atlas:
	if (sprite && sprite->GetFramesCount() > 1) {
		const int anim_size = Math::pow(2, Math::ceil(Math::log(real_t(sprite->GetFramesCount())) / Math_LN2));
		const int anim_square = Math::sqrt(real_t(anim_size));
		const int anim_frame = Math::floor(frame);
		if (anim_frame >= sprite->GetFramesCount()) {
			WARN_PRINT(vformat("[GdTLFXParticleManager] Out of range: %d of %d (frames: %d)", frame, anim_frame, sprite->GetFramesCount()));
		}
		const int gr = anim_frame / anim_square, gc = anim_frame % anim_square;
		const real_t cw = rc.size.width / anim_square, ch = rc.size.height / anim_square;
		rc = Rect2(rc.position.x + gc * cw, rc.position.y + gr * ch, cw, ch);
	}

	uvs.push_back({ rc.position.x, rc.position.y });
	uvs.push_back({ rc.position.x + rc.size.width, rc.position.y });
	uvs.push_back({ rc.position.x + rc.size.width, rc.position.y + rc.size.height });
	uvs.push_back({ rc.position.x, rc.position.y + rc.size.height });

	real_t x0 = -x * scaleX;
	real_t y0 = -y * scaleY;
	real_t x1 = x0;
	real_t y1 = (-y + sprite->GetHeight()) * scaleY;
	real_t x2 = (-x + sprite->GetWidth()) * scaleX;
	real_t y2 = y1;
	real_t x3 = x2;
	real_t y3 = y0;

	real_t cos = Math::cos(rotation / 180 * M_PI);
	real_t sin = Math::sin(rotation / 180 * M_PI);

	verts.push_back({ px + x0 * cos - y0 * sin, py + x0 * sin + y0 * cos });
	verts.push_back({ px + x1 * cos - y1 * sin, py + x1 * sin + y1 * cos });
	verts.push_back({ px + x2 * cos - y2 * sin, py + x2 * sin + y2 * cos });
	verts.push_back({ px + x3 * cos - y3 * sin, py + x3 * sin + y3 * cos });

	colors.push_multi(4, { gFF(r), gFF(g), gFF(b), a });

	indexes.append_array(parray(0, 1, 2, 2, 3, 0));

	_lastTexture = _Gd(sprite)->GetTexture();
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
	if (verts.size()) {
		if (_lastAdditive) {
			// ALPHA_ADD
		} else {
			// ALPHA_BLEND
		}

		_canvas->draw_mesh(_mesh, _lastTexture);

		reset(); // clear batch data
	}
}

// Atlas utilities:

enum SplitType {
	VerticalSplit,
	HorizontalSplit
};

static const int maxMargin = 2;

struct AreaAllocatorNode {
	inline bool isLeaf();

	AreaAllocatorNode *parent;
	AreaAllocatorNode *left;
	AreaAllocatorNode *right;
	int split; // only valid for inner nodes.
	SplitType splitType;
	bool isOccupied; // only valid for leaf nodes.

	AreaAllocatorNode(AreaAllocatorNode *parent);
	~AreaAllocatorNode();
};

AreaAllocatorNode::AreaAllocatorNode(AreaAllocatorNode *parent) :
		parent(parent), left(0), right(0), isOccupied(false) {
}

AreaAllocatorNode::~AreaAllocatorNode() {
	delete left;
	delete right;
}

bool AreaAllocatorNode::isLeaf() {
	DEV_ASSERT((left != 0) == (right != 0));
	return !left;
}

AreaAllocator::AreaAllocator(const Size2 &size, const Size2 &padding) :
		areaSize(size), padding(padding) {
	root = new AreaAllocatorNode(0);
}

AreaAllocator::~AreaAllocator() {
	delete root;
}

Rect2 AreaAllocator::allocate(const Size2 &size) {
	Point2 point;
	bool result = allocateInNode(size + padding, point, Rect2(Point2(0, 0), areaSize), root);
	return result ? Rect2(point, size + padding) : Rect2();
}

bool AreaAllocator::allocate(Rect2 &req) {
	return allocateInNode(req.size + padding, req.position, Rect2(Point2(0, 0), areaSize), root);
}

bool AreaAllocator::deallocate(const Rect2 &rect) {
	return deallocateInNode(rect.position, root);
}

bool AreaAllocator::allocateInNode(const Size2 &size, Point2 &result, const Rect2 &currentRect, AreaAllocatorNode *node) {
	if (size.width > currentRect.size.width || size.height > currentRect.size.height) {
		return false;
	}
	if (node->isLeaf()) {
		if (node->isOccupied) {
			return false;
		}
		if (size.width + maxMargin >= currentRect.size.width && size.height + maxMargin >= currentRect.size.height) {
			// Snug fit, occupy entire rectangle.
			node->isOccupied = true;
			result = currentRect.position;
			return true;
		}
		// TODO: Reuse nodes.
		// Split node.
		node->left = new AreaAllocatorNode(node);
		node->right = new AreaAllocatorNode(node);
		Rect2 splitRect = currentRect;
		if ((currentRect.size.width - size.width) * currentRect.size.height < (currentRect.size.height - size.height) * currentRect.size.width) {
			node->splitType = HorizontalSplit;
			node->split = currentRect.top() + size.height;
			splitRect.size.height = size.height;
		} else {
			node->splitType = VerticalSplit;
			node->split = currentRect.left() + size.width;
			splitRect.size.width = size.width;
		}
		return allocateInNode(size, result, splitRect, node->left);
	} else {
		// TODO: avoid unnecessary recursion.
		//  has been split.
		Rect2 leftRect = currentRect;
		Rect2 rightRect = currentRect;
		if (node->splitType == HorizontalSplit) {
			leftRect.size.height = node->split - leftRect.top();
			rightRect.position.y = node->split;
		} else {
			leftRect.size.width = node->split - leftRect.left();
			rightRect.position.x = node->split;
		}
		if (allocateInNode(size, result, leftRect, node->left)) {
			return true;
		}
		if (allocateInNode(size, result, rightRect, node->right)) {
			return true;
		}
		return false;
	}
}

bool AreaAllocator::deallocateInNode(const Point2 &pos, AreaAllocatorNode *node) {
	while (!node->isLeaf()) {
		//  has been split.
		int cmp = node->splitType == HorizontalSplit ? pos.y : pos.x;
		node = cmp < node->split ? node->left : node->right;
	}
	if (!node->isOccupied) {
		return false;
	}
	node->isOccupied = false;
	mergeNodeWithNeighbors(node);
	return true;
}

void AreaAllocator::mergeNodeWithNeighbors(AreaAllocatorNode *node) {
	bool done = false;
	AreaAllocatorNode *parent = 0;
	AreaAllocatorNode *current = 0;
	AreaAllocatorNode *sibling;
	while (!done) {
		DEV_ASSERT(node->isLeaf());
		DEV_ASSERT(!node->isOccupied);
		if (node->parent == 0) {
			return; // No neighbours.
		}
		SplitType splitType = SplitType(node->parent->splitType);
		done = true;

		// Special case. Might be faster than going through the general code path.
#if 0
		// Merge with sibling.
		parent = node->parent;
		sibling = (node == parent->left ? parent->right : parent->left);
		if (sibling->isLeaf() && !sibling->isOccupied) {
			DEV_ASSERT(!sibling->right);
			node = parent;
			parent->isOccupied = false;
			delete parent->left;
			delete parent->right;
			parent->left = parent->right = 0;
			done = false;
			continue;
		}
#endif

		// Merge with left neighbour.
		current = node;
		parent = current->parent;
		while (parent && current == parent->left && parent->splitType == splitType) {
			current = parent;
			parent = parent->parent;
		}

		if (parent && parent->splitType == splitType) {
			DEV_ASSERT(current == parent->right);
			DEV_ASSERT(parent->left);

			AreaAllocatorNode *neighbor = parent->left;
			while (neighbor->right && neighbor->splitType == splitType) {
				neighbor = neighbor->right;
			}
			if (neighbor->isLeaf() && neighbor->parent->splitType == splitType && !neighbor->isOccupied) {
				// Left neighbour can be merged.
				parent->split = neighbor->parent->split;

				parent = neighbor->parent;
				sibling = neighbor == parent->left ? parent->right : parent->left;
				AreaAllocatorNode **nodeRef = &root;
				if (parent->parent) {
					if (parent == parent->parent->left) {
						nodeRef = &parent->parent->left;
					} else {
						nodeRef = &parent->parent->right;
					}
				}
				sibling->parent = parent->parent;
				*nodeRef = sibling;
				parent->left = parent->right = 0;
				delete parent;
				delete neighbor;
				done = false;
			}
		}

		// Merge with right neighbour.
		current = node;
		parent = current->parent;
		while (parent && current == parent->right && parent->splitType == splitType) {
			current = parent;
			parent = parent->parent;
		}

		if (parent && parent->splitType == splitType) {
			DEV_ASSERT(current == parent->left);
			DEV_ASSERT(parent->right);

			AreaAllocatorNode *neighbor = parent->right;
			while (neighbor->left && neighbor->splitType == splitType)
				neighbor = neighbor->left;

			if (neighbor->isLeaf() && neighbor->parent->splitType == splitType && !neighbor->isOccupied) {
				// Right neighbour can be merged.
				parent->split = neighbor->parent->split;

				parent = neighbor->parent;
				sibling = neighbor == parent->left ? parent->right : parent->left;
				AreaAllocatorNode **nodeRef = &root;
				if (parent->parent) {
					if (parent == parent->parent->left)
						nodeRef = &parent->parent->left;
					else
						nodeRef = &parent->parent->right;
				}
				sibling->parent = parent->parent;
				*nodeRef = sibling;
				parent->left = parent->right = 0;
				delete parent;
				delete neighbor;
				done = false;
			}
		}
	} // end while(!done)
}

// Image utilities:

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

static void _build_tiles(Size2 grid_size, unsigned int total_frames, Point2 tex_origin = Point2(0, 0), Size2 tex_size = Size2(1, 1)) {
	Vector<Rect2> frames;
	for (int fr = 0; fr < grid_size.height /* rows */; fr++)
		for (int fc = 0; fc < grid_size.width /* cols */; fc++) {
			const real_t cw = tex_size.width / grid_size.width, ch = tex_size.height / grid_size.height;
			frames.push_back(Rect2(tex_origin.x + fc * cw, tex_origin.y + fr * ch, cw, ch));
			if (frames.size() == total_frames) {
				break;
			}
		}
}

static void _image_to_greyscale(Ref<Image> p_image, float p_fact) {
	if (p_fact == 0.0)
		return;

	p_image->lock();

	for (int y = 0; y < p_image->get_height(); ++y) {
		for (int x = 0; x < p_image->get_width(); ++x) {
			const Color data = p_image->get_pixel(x, y);
			const float gray = data.r * .34 + data.g * .5 + data.b * .15;

			if (p_fact == 1.0) {
				p_image->set_pixel(x, y, { gray, gray, gray, data.a });
			} else {
				p_image->set_pixel(x, y, { p_fact * gray + (1 - p_fact) * data.r, p_fact * gray + (1 - p_fact) * data.g, p_fact * gray + (1 - p_fact) * data.b, data.a });
			}
		}
	}
	p_image->unlock();
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

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
