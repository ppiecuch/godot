#pragma once

#ifdef __GNUC__
#define _unused __attribute__((unused))
#else
#define _unused
#endif

#include "common/gd_core.h"

#include "TTFExceptions.h"
#include "TTFTypes.h"
#include "TTFMath.h"

#include "TTFTriangulator2D.h"
#include "TTFTriangulator2DLinear.h"
#include "TTFTriangulator3D.h"
#include "TTFTriangulator3DBezel.h"
#include "TTFFont.h"

namespace TTF {

using TTFCore::FontException;
using TTFCore::FileFailure;
using TTFCore::FileLengthError;
using TTFCore::TableDoesNotExist;
using TTFCore::ChecksumException;
using TTFCore::VersionException;
using TTFCore::InvalidFontException;
using TTFCore::UnsupportedCap;

using TTFCore::vec2f;
using TTFCore::vec3f;
using TTFCore::vec4f;
using TTFCore::vec2t;
using TTFCore::vec3t;
using TTFCore::vec4t;

using TTFCore::TriangulatorFlags;

using TTFCore::Font;
using TTFCore::CodePoint;
using TTFCore::GlyphMetrics;
using TTFCore::FontMetrics;
using TTFCore::VGlyphMetrics;
using TTFCore::VFontMetrics;
using TTFCore::MapFromData;

using TTFCore::TriSmall;
using TTFCore::TriLarge;
typedef TTFCore::Triangulator2D<vec2t, TTFCore::TriSmall> Triangulator2DI;
typedef TTFCore::Triangulator2D<vec2t, TTFCore::TriLarge> Triangulator2DII;
typedef TTFCore::Triangulator2DLinear<vec2t, TTFCore::TriSmall> Triangulator2DLinearI;
typedef TTFCore::Triangulator2DLinear<vec2t, TTFCore::TriLarge> Triangulator2DLinearII;
typedef TTFCore::Triangulator3D<vec3t, TTFCore::TriSmall> Triangulator3DI;
typedef TTFCore::Triangulator3D<vec3t, TTFCore::TriLarge> Triangulator3DII;
typedef TTFCore::Triangulator3DBezel<vec3t, TTFCore::TriSmall> Triangulator3DBezelI;
typedef TTFCore::Triangulator3DBezel<vec3t, TTFCore::TriLarge> Triangulator3DBezelII;

const TTFCore::MapFromData map_from_data{};

} // namespace
