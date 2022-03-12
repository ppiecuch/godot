#include "polyfonts.h"
#include "polygodot.h"

#include "core/variant.h"
#include "core/error_macros.h"

#include "polyfonts_all.h"
#include "polyfonts.cpp"

void PolyGodot::setWidth(float w) { width = w; };
void PolyGodot::setColor(float R, float G, float B, float A) { color = Color(R, G, B, A); };

void PolyGodot::beginStringDraw() const { }
void PolyGodot::doneStringDraw() const { }

bool PolyGodot::polyDrawElements(int mode, coord_vector &indices) const {
	switch (mode) {
		case POLY_POINTS:
			break;
		case POLY_LINES:
			break;
		case POLY_LINE_STRIP:
			break;
		case POLY_LINE_LOOP:
			break;
		case POLY_TRIANGLES:
			break;
		case POLY_TRIANGLE_STRIP:
			break;
		case POLY_TRIANGLE_FAN:
			break;
		default:
			ERR_PRINT(vformat("polyDrawElements error: INVALID_ENUM - mode %d", mode));
			return false;
	}
	return true;
}

PolyGodot::PolyGodot():PolyFont(getDefaultFont()) { }

PolyGodot::PolyGodot(const pffont *f):PolyFont(f) { }

PolyGodot::~PolyGodot() { }
