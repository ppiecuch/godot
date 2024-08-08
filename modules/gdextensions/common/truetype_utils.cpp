#include "truetype_utils.h"

#include "common/gd_core.h"
#include "core/error_macros.h"
#include "core/os/file_access.h"

#include <algorithm>
#include <deque>
#include <numeric>

#include <ft2build.h>

#ifdef X11_ENABLED
#include <fontconfig/fontconfig.h>
#endif

#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_TRIGONOMETRY_H

const UnicodeBlock::range UnicodeBlock::Space{ 32, 32 };
const UnicodeBlock::range UnicodeBlock::IdeographicSpace{ 0x3000, 0x3000 };
const UnicodeBlock::range UnicodeBlock::Latin{ 32, 0x007F };
const UnicodeBlock::range UnicodeBlock::Latin1Supplement{ 32, 0x00FF };
const UnicodeBlock::range UnicodeBlock::LatinA{ 0x0100, 0x017F };
const UnicodeBlock::range UnicodeBlock::Greek{ 0x0370, 0x03FF };
const UnicodeBlock::range UnicodeBlock::Cyrillic{ 0x0400, 0x04FF };
const UnicodeBlock::range UnicodeBlock::Arabic{ 0x0600, 0x077F };
const UnicodeBlock::range UnicodeBlock::ArabicSupplement{ 0x0750, 0x077F };
const UnicodeBlock::range UnicodeBlock::ArabicExtendedA{ 0x08A0, 0x08FF };
const UnicodeBlock::range UnicodeBlock::Devanagari{ 0x0900, 0x097F };
const UnicodeBlock::range UnicodeBlock::HangulJamo{ 0x1100, 0x11FF };
const UnicodeBlock::range UnicodeBlock::VedicExtensions{ 0x1CD0, 0x1CFF };
const UnicodeBlock::range UnicodeBlock::LatinExtendedAdditional{ 0x1E00, 0x1EFF };
const UnicodeBlock::range UnicodeBlock::GreekExtended{ 0x1F00, 0x1FFF };
const UnicodeBlock::range UnicodeBlock::GeneralPunctuation{ 0x2000, 0x206F };
const UnicodeBlock::range UnicodeBlock::SuperAndSubScripts{ 0x2070, 0x209F };
const UnicodeBlock::range UnicodeBlock::CurrencySymbols{ 0x20A0, 0x20CF };
const UnicodeBlock::range UnicodeBlock::LetterLikeSymbols{ 0x2100, 0x214F };
const UnicodeBlock::range UnicodeBlock::NumberForms{ 0x2150, 0x218F };
const UnicodeBlock::range UnicodeBlock::Arrows{ 0x2190, 0x21FF };
const UnicodeBlock::range UnicodeBlock::MathOperators{ 0x2200, 0x22FF };
const UnicodeBlock::range UnicodeBlock::MiscTechnical{ 0x2300, 0x23FF };
const UnicodeBlock::range UnicodeBlock::BoxDrawing{ 0x2500, 0x257F };
const UnicodeBlock::range UnicodeBlock::BlockElement{ 0x2580, 0x259F };
const UnicodeBlock::range UnicodeBlock::GeometricShapes{ 0x25A0, 0x25FF };
const UnicodeBlock::range UnicodeBlock::MiscSymbols{ 0x2600, 0x26FF };
const UnicodeBlock::range UnicodeBlock::Dingbats{ 0x2700, 0x27BF };
const UnicodeBlock::range UnicodeBlock::CJKSymbolAndPunctuation{ 0x3001, 0x303F };
const UnicodeBlock::range UnicodeBlock::Hiragana{ 0x3040, 0x309F };
const UnicodeBlock::range UnicodeBlock::Katakana{ 0x30A0, 0x30FF };
const UnicodeBlock::range UnicodeBlock::HangulCompatJamo{ 0x3130, 0x318F };
const UnicodeBlock::range UnicodeBlock::KatakanaPhoneticExtensions{ 0x31F0, 0x31FF };
const UnicodeBlock::range UnicodeBlock::CJKLettersAndMonths{ 0x3200, 0x32FF };
const UnicodeBlock::range UnicodeBlock::CJKUnified{ 0x4E00, 0x9FD5 };
const UnicodeBlock::range UnicodeBlock::DevanagariExtended{ 0xA8E0, 0xA8FF };
const UnicodeBlock::range UnicodeBlock::HangulExtendedA{ 0xA960, 0xA97F };
const UnicodeBlock::range UnicodeBlock::HangulSyllables{ 0xAC00, 0xD7AF };
const UnicodeBlock::range UnicodeBlock::HangulExtendedB{ 0xD7B0, 0xD7FF };
const UnicodeBlock::range UnicodeBlock::AlphabeticPresentationForms{ 0xFB00, 0xFB4F };
const UnicodeBlock::range UnicodeBlock::ArabicPresFormsA{ 0xFB50, 0xFDFF };
const UnicodeBlock::range UnicodeBlock::ArabicPresFormsB{ 0xFE70, 0xFEFF };
const UnicodeBlock::range UnicodeBlock::KatakanaHalfAndFullwidthForms{ 0xFF00, 0xFFEF };
const UnicodeBlock::range UnicodeBlock::KanaSupplement{ 0x1B000, 0x1B0FF };
const UnicodeBlock::range UnicodeBlock::RumiNumericalSymbols{ 0x10E60, 0x10E7F };
const UnicodeBlock::range UnicodeBlock::ArabicMath{ 0x1EE00, 0x1EEFF };
const UnicodeBlock::range UnicodeBlock::MiscSymbolsAndPictographs{ 0x1F300, 0x1F5FF };
const UnicodeBlock::range UnicodeBlock::Emoticons{ 0x1F601, 0x1F64F };
const UnicodeBlock::range UnicodeBlock::TransportAndMap{ 0x1F680, 0x1F6FF };
const UnicodeBlock::range UnicodeBlock::EnclosedCharacters{ 0x24C2, 0x1F251 };
const UnicodeBlock::range UnicodeBlock::Uncategorized{ 0x00A9, 0x1F5FF };
const UnicodeBlock::range UnicodeBlock::AdditionalEmoticons{ 0x1F600, 0x1F636 };
const UnicodeBlock::range UnicodeBlock::AdditionalTransportAndMap{ 0x1F681, 0x1F6C5 };
const UnicodeBlock::range UnicodeBlock::OtherAdditionalSymbols{ 0x1F30D, 0x1F567 };
const UnicodeBlock::range UnicodeBlock::UppercaseLatin{ 65, 90 };
const UnicodeBlock::range UnicodeBlock::LowercaseLatin{ 97, 122 };
const UnicodeBlock::range UnicodeBlock::Braces{ 123, 127 };
const UnicodeBlock::range UnicodeBlock::Numbers{ 48, 57 };
const UnicodeBlock::range UnicodeBlock::Symbols{ 33, 47 };
const UnicodeBlock::range UnicodeBlock::GenericSymbols{ 58, 64 };

const std::initializer_list<UnicodeBlock::range> UnicodeAlphabet::Emoji{
	UnicodeBlock::Space,
	UnicodeBlock::Emoticons,
	UnicodeBlock::Dingbats,
	UnicodeBlock::Uncategorized,
	UnicodeBlock::TransportAndMap,
	UnicodeBlock::EnclosedCharacters,
	UnicodeBlock::OtherAdditionalSymbols,

};

const std::initializer_list<UnicodeBlock::range> UnicodeAlphabet::Japanese{
	UnicodeBlock::Space,
	UnicodeBlock::IdeographicSpace,
	UnicodeBlock::CJKSymbolAndPunctuation,
	UnicodeBlock::Hiragana,
	UnicodeBlock::Katakana,
	UnicodeBlock::KatakanaPhoneticExtensions,
	UnicodeBlock::CJKLettersAndMonths,
	UnicodeBlock::CJKUnified
};

const std::initializer_list<UnicodeBlock::range> UnicodeAlphabet::Chinese{
	UnicodeBlock::Space,
	UnicodeBlock::IdeographicSpace,
	UnicodeBlock::CJKSymbolAndPunctuation,
	UnicodeBlock::CJKLettersAndMonths,
	UnicodeBlock::CJKUnified
};

const std::initializer_list<UnicodeBlock::range> UnicodeAlphabet::Korean{
	UnicodeBlock::Space,
	UnicodeBlock::IdeographicSpace,
	UnicodeBlock::CJKSymbolAndPunctuation,
	UnicodeBlock::HangulJamo,
	UnicodeBlock::HangulCompatJamo,
	UnicodeBlock::HangulExtendedA,
	UnicodeBlock::HangulExtendedB,
	UnicodeBlock::HangulSyllables
};

const std::initializer_list<UnicodeBlock::range> UnicodeAlphabet::Arabic{
	UnicodeBlock::Space,
	UnicodeBlock::Arabic,
	UnicodeBlock::ArabicExtendedA,
	UnicodeBlock::ArabicMath,
	UnicodeBlock::ArabicPresFormsA,
	UnicodeBlock::ArabicPresFormsB
};

const std::initializer_list<UnicodeBlock::range> UnicodeAlphabet::Devanagari{
	UnicodeBlock::Devanagari,
	UnicodeBlock::DevanagariExtended,
	UnicodeBlock::VedicExtensions
};

const std::initializer_list<UnicodeBlock::range> UnicodeAlphabet::Latin{
	UnicodeBlock::Latin1Supplement,
	UnicodeBlock::LatinExtendedAdditional,
	UnicodeBlock::Latin,
	UnicodeBlock::LatinA,
};

const std::initializer_list<UnicodeBlock::range> UnicodeAlphabet::Greek{
	UnicodeBlock::Space,
	UnicodeBlock::Greek,
	UnicodeBlock::GreekExtended
};

const std::initializer_list<UnicodeBlock::range> UnicodeAlphabet::Cyrillic{
	UnicodeBlock::Space,
	UnicodeBlock::Cyrillic
};

const TrueTypeFontUtils::glyph_props TrueTypeFontUtils::InvalidProps{
	0,
	0,
	0.0f, 0.0f, 0.0f, 0.0f
};

static bool libraries_initialized = false;
static FT_Library library;

typedef std::pair<std::vector<Point2>, bool> _polyline;

#define poly_get_points(POLY) POLY.first
#define poly_get_closed(POLY) POLY.second
#define poly_set_close(POLY) POLY.second = true

static int poly_bezier_to(_polyline &poly, const Point2 &cp1, const Point2 &cp2, const Point2 &to, int curve_resolution = 20) {
	// the resolultion with which we computer this bezier is arbitrary, can we possibly make it dynamic?

	std::vector<Point2> &points = poly.first;

	const real_t x0 = points[points.size() - 1].x;
	const real_t y0 = points[points.size() - 1].y;

	// polynomial coefficients
	const real_t cx = 3 * (cp1.x - x0);
	const real_t bx = 3 * (cp2.x - cp1.x) - cx;
	const real_t ax = to.x - x0 - cx - bx;

	const real_t cy = 3 * (cp1.y - y0);
	const real_t by = 3 * (cp2.y - cp1.y) - cy;
	const real_t ay = to.y - y0 - cy - by;

	int count = 0;
	for (int i = 1; i <= curve_resolution; i++) {
		real_t t = real_t(i) / real_t(curve_resolution);
		real_t t2 = t * t;
		real_t t3 = t2 * t;
		real_t x = (ax * t3) + (bx * t2) + (cx * t) + x0;
		real_t y = (ay * t3) + (by * t2) + (cy * t) + y0;
		points.emplace_back(Point2(x, y));
		count++;
	}
	return count;
}

static int poly_quad_bezier_to(_polyline &poly, const Point2 &pt1, const Point2 &pt2, const Point2 &pt3, int curve_resolution = 20) {
	std::vector<Point2> &points = poly_get_points(poly);
	int count = 0;
	for (int i = 0; i <= curve_resolution; i++) {
		real_t t = real_t(i) / real_t(curve_resolution);
		real_t a = (1 - t) * (1 - t);
		real_t b = 2 * t * (1 - t);
		real_t c = t * t;
		real_t x = a * pt1.x + b * pt2.x + c * pt3.x;
		real_t y = a * pt1.y + b * pt2.y + c * pt3.y;
		points.emplace_back(Point2(x, y));
		count++;
	}
	return count;
}

static int poly_curve_to(_polyline &poly, std::deque<Point2> &curve_vertices, const Point2 &to, int curve_resolution = 20) {
	std::vector<Point2> &points = poly_get_points(poly);
	curve_vertices.push_back(to);
	int pt = 0;
	if (curve_vertices.size() == 4) {
		real_t x0 = curve_vertices[0].x;
		real_t y0 = curve_vertices[0].y;
		real_t x1 = curve_vertices[1].x;
		real_t y1 = curve_vertices[1].y;
		real_t x2 = curve_vertices[2].x;
		real_t y2 = curve_vertices[2].y;
		real_t x3 = curve_vertices[3].x;
		real_t y3 = curve_vertices[3].y;
		for (int i = 1; i <= curve_resolution; i++) {
			real_t t = real_t(i) / real_t(curve_resolution);
			real_t t2 = t * t;
			real_t t3 = t2 * t;
			real_t x = 0.5 * ((2 * x1) + (-x0 + x2) * t + (2 * x0 - 5 * x1 + 4 * x2 - x3) * t2 + (-x0 + 3 * x1 - 3 * x2 + x3) * t3);
			real_t y = 0.5 * ((2 * y1) + (-y0 + y2) * t + (2 * y0 - 5 * y1 + 4 * y2 - y3) * t2 + (-y0 + 3 * y1 - 3 * y2 + y3) * t3);
			points.emplace_back(Point2(x, y));
			pt++;
		}
		curve_vertices.pop_front();
	}
	return pt;
}

static int poly_get_wrapped_index(const _polyline &poly, int index) {
	const std::vector<Point2> &points = poly_get_points(poly);
	const bool closed = poly_get_closed(poly);
	if (points.empty())
		return 0;
	if (index < 0)
		return closed ? (index + points.size()) % points.size() : 0;
	if (index > int(points.size()) - 1)
		return closed ? index % points.size() : points.size() - 1;
	return index;
}

static std::vector<real_t> poly_get_lengths(const _polyline &poly) {
	const std::vector<Point2> &points = poly_get_points(poly);
	std::vector<real_t> lengths;
	if (points.size() > 1) {
		lengths.resize(points.size());
		real_t length = 0;
		for (int i = 0; i < (int)points.size(); i++) {
			lengths[i] = length;
			length += MathExtension::distance(points[i], points[poly_get_wrapped_index(poly, i + 1)]);
		}
	} else {
		lengths.push_back(0); // at least one element
	}
	return lengths;
}

static real_t poly_get_index_at_length(const std::vector<real_t> &lengths, real_t length) {
	ERR_FAIL_COND_V(lengths.size() < 2, 0);

	const real_t total_length = lengths.back();
	length = CLAMP(length, 0, total_length);
	const int last_point_index = lengths.size() - 1;

	int i1 = CLAMP(floor(length / total_length * last_point_index), 0, lengths.size() - 2); // start approximation here
	int left_limit = 0;
	int right_limit = last_point_index;

	real_t dist_at1, dist_at2;
	for (int iterations = 0; iterations < 32; iterations++) { // limit iterations
		i1 = CLAMP(i1, 0, lengths.size() - 1);
		dist_at1 = lengths[i1];
		if (dist_at1 <= length) { // if length at i1 is less than desired length (this is good)
			dist_at2 = lengths[i1 + 1];
			if (dist_at2 >= length) {
				const real_t t = Math::map2(length, dist_at1, dist_at2, 0, 1);
				return i1 + t;
			} else {
				left_limit = i1;
			}
		} else {
			right_limit = i1;
		}
		i1 = (left_limit + right_limit) / 2;
	}
	return 0;
}

static Point2 poly_get_point_at_index_interpolated(const _polyline &poly, real_t findex) {
	const std::vector<Point2> &points = poly_get_points(poly);
	const int i1 = Math::floor(findex);
	const real_t t = findex - i1;

	Point2 left_point = points[poly_get_wrapped_index(poly, i1)];
	Point2 right_point = points[poly_get_wrapped_index(poly, i1 + 1)];
	return MathExtension::mix(left_point, right_point, t);
}

static Point2 poly_get_point_at_length(const _polyline &poly, const std::vector<real_t> lengths, real_t f) {
	return poly_get_point_at_index_interpolated(poly, poly_get_index_at_length(lengths, f));
}

static _polyline poly_get_resampled_by_spacing(const _polyline &poly, const std::vector<real_t> lengths, real_t spacing) {
	const std::vector<Point2> &points = poly_get_points(poly);
	ERR_FAIL_COND_V(spacing <= 0, _polyline());
	ERR_FAIL_COND_V(points.size() == 0, _polyline());
	std::vector<Point2> repoly;
	real_t total_length = lengths.back();
	real_t f = 0;
	for (f = 0; f <= total_length; f += spacing) {
		repoly.push_back(poly_get_point_at_length(poly, lengths, f));
	}
	if (f != total_length) {
		repoly.push_back(points.back()); // closing point
	}
	return _polyline(repoly, poly_get_closed(poly));
}

static _polyline poly_get_resampled_by_count(const _polyline &poly, const std::vector<real_t> lengths, int count) {
	const real_t perimeter = lengths.back();
	ERR_FAIL_COND_V(perimeter <= 0, _polyline());
	ERR_FAIL_COND_V(count < 2, _polyline());
	return poly_get_resampled_by_spacing(poly, lengths, perimeter / (count - 1));
}

struct Segment {
	Point2 P0;
	Point2 P1;
};

static void poly_simplify_dp(real_t tol, const Point2 *v, int j, int k, int *mk) {
	if (k <= j + 1) { // there is nothing to simplify
		return;
	}
	// check for adequate approximation by segment S from v[j] to v[k]
	int maxi = j; // index of vertex farthest from S
	real_t maxd2 = 0; // distance squared of farthest vertex
	real_t tol2 = tol * tol; // tolerance squared
	Segment S = { v[j], v[k] }; // segment from v[j] to v[k]
	Point2 u = S.P1 - S.P0; // segment direction vector
	real_t cu = MathExtension::dot(u, u); // segment length squared

	// test each vertex v[i] for max distance from S
	// compute using the Feb 2001 Algorithm's dist_ofPoint_to_Segment()
	// Note: this works in any dimension (2D, 3D, ...)
	Point2 w;
	Point2 Pb; // base of perpendicular from v[i] to S
	real_t b, cw, dv2; // dv2 = distance v[i] to S squared

	for (int i = j + 1; i < k; i++) {
		// compute distance squared
		w = v[i] - S.P0;
		cw = MathExtension::dot(w, u);
		if (cw <= 0)
			dv2 = (v[i] - S.P0).length_squared();
		else if (cu <= cw)
			dv2 = (v[i] - S.P1).length_squared();
		else {
			b = (real_t)(cw / cu);
			Pb = S.P0 + u * b;
			dv2 = (v[i] - Pb).length_squared();
		}
		// test with current max distance squared
		if (dv2 <= maxd2)
			continue;
		// v[i] is a new max vertex
		maxi = i;
		maxd2 = dv2;
	}
	if (maxd2 > tol2) { // error is worse than the tolerance
		// split the polyline at the farthest vertex from S
		mk[maxi] = 1; // mark v[maxi] for the simplified polyline
		// recursively simplify the two subpolylines at v[maxi]
		poly_simplify_dp(tol, v, j, maxi, mk); // polyline v[j] to v[maxi]
		poly_simplify_dp(tol, v, maxi, k, mk); // polyline v[maxi] to v[k]
	}
	// else the approximation is OK, so ignore intermediate vertices
}

static void poly_simplify(std::vector<Point2> &points, real_t tol) {
	if (points.size() < 2) {
		return;
	}
	const int n = points.size();
	if (n == 0) {
		return;
	}
	std::vector<Point2> sV;
	sV.resize(n);

	int i, k, m, pv; // misc counters
	real_t tol2 = tol * tol; // tolerance squared
	std::vector<Point2> vt;
	std::vector<int> mk;
	vt.resize(n);
	mk.resize(n, 0);

	// STAGE 1. Vertex Reduction within tolerance of prior vertex cluster
	vt[0] = points[0]; // start at the beginning
	for (i = k = 1, pv = 0; i < n; i++) {
		if ((points[i] - points[pv]).length_squared() < tol2)
			continue;
		vt[k++] = points[i];
		pv = i;
	}
	if (pv < n - 1)
		vt[k++] = points[n - 1]; // finish at the end

	// STAGE 2. Douglas-Peucker polyline simplification
	mk[0] = mk[k - 1] = 1; // mark the first and last vertices
	poly_simplify_dp(tol, &vt[0], 0, k - 1, &mk[0]);
	// copy marked vertices to the output simplified polyline
	for (i = m = 0; i < k; i++) {
		if (mk[i])
			sV[m++] = vt[i];
	}
	// get rid of the unused points
	if (m < (int)sV.size()) {
		points.assign(sV.begin(), sV.begin() + m);
	} else {
		points = sV;
	}
}

// Gets a smoothed version of the polyline.
//
// `smoothing_size` is the size of the smoothing window. So if
// `smoothing_size` is 2, then 2 points from the left, 1 in the center,
// and 2 on the right (5 total) will be used for smoothing each point.
// `smoothing_shape` describes whether to use a triangular window (0) or
// box window (1) or something in between (for example, .5).
std::vector<Point2> poly_smoothed(std::vector<Point2> &points, int smoothing_size, real_t smoothing_shape = 0, bool closed = false) {
	int n = points.size();
	smoothing_size = CLAMP(smoothing_size, 0, n);
	smoothing_shape = CLAMP(smoothing_shape, 0, 1);

	// precompute weights and normalization
	std::vector<real_t> weights;
	weights.resize(smoothing_size);
	// side weights
	for (int i = 1; i < smoothing_size; i++) {
		real_t cur_weight = Math::map1(i, 0, smoothing_size, 1, smoothing_shape);
		weights[i] = cur_weight;
	}

	std::vector<Point2> result = points; // make a copy of this polyline

	for (int i = 0; i < n; i++) {
		real_t sum = 1; // center weight
		for (int j = 1; j < smoothing_size; j++) {
			Point2 cur;
			int left_position = i - j;
			int right_position = i + j;
			if (left_position < 0 && closed) {
				left_position += n;
			}
			if (left_position >= 0) {
				cur += points[left_position];
				sum += weights[j];
			}
			if (right_position >= n && closed) {
				right_position -= n;
			}
			if (right_position < n) {
				cur += points[right_position];
				sum += weights[j];
			}
			result[i] += cur * weights[j];
		}
		result[i] /= sum;
	}
	return result;
}

static std::vector<_polyline> generate_polylines_from_commands(const std::vector<TrueTypePath::Command> &commands, int curve_resolution = 20) {
	std::vector<_polyline> polylines;
	std::deque<Point2> curve_vertices;
	polylines.push_back(_polyline()); // first curve
	for (size_t i = 0; i < commands.size(); i++) {
		switch (commands[i].type) {
			case TrueTypePath::Command::MoveTo: {
				polylines.push_back(_polyline());
				poly_get_points(polylines.back()).push_back(commands[i].to);
			} break;
			case TrueTypePath::Command::LineTo: {
				poly_get_points(polylines.back()).push_back(commands[i].to);
			} break;
			case TrueTypePath::Command::CurveTo: {
				poly_curve_to(polylines.back(), curve_vertices, commands[i].to, curve_resolution);
			} break;
			case TrueTypePath::Command::BezierTo: {
				poly_bezier_to(polylines.back(), commands[i].cp1, commands[i].cp2, commands[i].to, curve_resolution);
			} break;
			case TrueTypePath::Command::QuadBezierTo: {
				poly_quad_bezier_to(polylines.back(), commands[i].cp1, commands[i].cp2, commands[i].to, curve_resolution);
			} break;
			case TrueTypePath::Command::Close: {
				poly_set_close(polylines.back());
			} break;
		}
	}
	return polylines;
}

static TrueTypePath make_contours_for_character(FT_Face face) {
	TrueTypePath char_outlines;
	auto move_to = [](const FT_Vector *to, void *user_data) {
		TrueTypePath *char_outlines = static_cast<TrueTypePath *>(user_data);
		char_outlines->move_to(TrueTypeFontUtils::int26p6_to_dbl(to->x, -to->y));
		return 0;
	};
	auto line_to = [](const FT_Vector *to, void *user_data) {
		TrueTypePath *char_outlines = static_cast<TrueTypePath *>(user_data);
		char_outlines->line_to(TrueTypeFontUtils::int26p6_to_dbl(to->x, -to->y));
		return 0;
	};
	auto conic_to = [](const FT_Vector *cp, const FT_Vector *to, void *user_data) {
		TrueTypePath *char_outlines = static_cast<TrueTypePath *>(user_data);
		auto lastP = char_outlines->commands.back().to;
		char_outlines->quad_bezier_to(lastP, { TrueTypeFontUtils::int26p6_to_dbl(cp->x, -cp->y) }, { TrueTypeFontUtils::int26p6_to_dbl(to->x, -to->y) });
		return 0;
	};
	auto cubic_to = [](const FT_Vector *cp1, const FT_Vector *cp2, const FT_Vector *to, void *user_data) {
		TrueTypePath *char_outlines = static_cast<TrueTypePath *>(user_data);
		char_outlines->bezier_to(
				{ TrueTypeFontUtils::int26p6_to_dbl(cp1->x, -cp1->y) },
				{ TrueTypeFontUtils::int26p6_to_dbl(cp2->x, -cp2->y) },
				{ TrueTypeFontUtils::int26p6_to_dbl(to->x, -to->y) });
		return 0;
	};

	FT_Outline_Funcs funcs{
		move_to,
		line_to,
		conic_to,
		cubic_to,
		0,
		0,
	};
	FT_Outline_Decompose(&face->glyph->outline, &funcs, &char_outlines);

	char_outlines.close();

	return char_outlines;
}

#ifdef OSX_ENABLED

#include <CoreFoundation/CoreFoundation.h>
#include <CoreText/CTFontDescriptor.h>

//------------------------------------------------------------------
static String osx_font_path_by_name(const String &fontname) {
	CFStringRef targetName = CFStringCreateWithCString(nullptr, fontname.utf8().c_str(), kCFStringEncodingUTF8);
	CTFontDescriptorRef targetDescriptor = CTFontDescriptorCreateWithNameAndSize(targetName, 0.0);
	CFURLRef targetURL = (CFURLRef)CTFontDescriptorCopyAttribute(targetDescriptor, kCTFontURLAttribute);
	String fontPath = "";

	if (targetURL) {
		UInt8 buffer[PATH_MAX];
		CFURLGetFileSystemRepresentation(targetURL, true, buffer, PATH_MAX);
		fontPath = String((char *)buffer);
		CFRelease(targetURL);
	}

	CFRelease(targetName);
	CFRelease(targetDescriptor);

	return fontPath;
}
#endif

#ifdef WINDOWS_ENABLED
// font font face -> file name name mapping
static std::unordered_map<String, String> fonts_table;
// read font linking information from registry, and store in std::map
//------------------------------------------------------------------
void init_windows() {
	LONG l_ret;

	const wchar_t *Fonts = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";

	HKEY key_ft;
	l_ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, Fonts, 0, KEY_QUERY_VALUE, &key_ft);
	if (l_ret != ERROR_SUCCESS) {
		ERR_PRINT("TrueTypeFontUtils") << "initWindows(): couldn't find fonts registery key";
		return;
	}

	DWORD value_count;
	DWORD max_data_len;
	wchar_t value_name[2048];
	BYTE *value_data;

	// get font_file_name -> font_face mapping from the "Fonts" registry key

	l_ret = RegQueryInfoKeyW(key_ft, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &value_count, nullptr, &max_data_len, nullptr, nullptr);
	if (l_ret != ERROR_SUCCESS) {
		ERR_PRINT("TrueTypeFontUtils") << "initWindows(): couldn't query registery for fonts";
		return;
	}

	// no font installed
	if (value_count == 0) {
		ERR_PRINT("TrueTypeFontUtils") << "initWindows(): couldn't find any fonts in registery";
		return;
	}

	// max_data_len is in BYTE
	value_data = static_cast<BYTE *>(HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, max_data_len));
	if (value_data == nullptr)
		return;

	char value_name_char[2048];
	char value_data_char[2048];
	// char ppidl[2048];
	// char fontsPath[2048];
	// SHGetKnownFolderIDList(FOLDERID_Fonts, 0, nullptr, &ppidl);
	// SHGetPathFromIDList(ppidl,&fontsPath);
	String fontsDir = getenv("windir");
	fontsDir += "\\Fonts\\";
	for (DWORD i = 0; i < value_count; ++i) {
		DWORD name_len = 2048;
		DWORD data_len = max_data_len;

		l_ret = RegEnumValueW(key_ft, i, value_name, &name_len, nullptr, nullptr, value_data, &data_len);
		if (l_ret != ERROR_SUCCESS) {
			ERR_PRINT("TrueTypeFontUtils") << "initWindows(): couldn't read registry key for font type";
			continue;
		}

		wcstombs(value_name_char, value_name, 2048);
		wcstombs(value_data_char, reinterpret_cast<wchar_t *>(value_data), 2048);
		String curr_face = value_name_char;
		String font_file = value_data_char;
		curr_face = curr_face.substr(0, curr_face.find('(') - 1);
		curr_face = ofToLower(curr_face);
		fonts_table[curr_face] = fontsDir + font_file;
	}
	HeapFree(GetProcessHeap(), 0, value_data);
	l_ret = RegCloseKey(key_ft);
}

static string win_font_path_by_name(const String &font_name) {
	return fonts_table[font_name];
}
#endif // WINDOWS_ENABLED

#ifdef X11_ENABLED
//------------------------------------------------------------------
static String linux_font_path_by_name(const String &font_name) {
	String filename;
	FcPattern *pattern = FcNameParse((const FcChar8 *)font_name.c_str());
	FcBool ret = FcConfigSubstitute(0, pattern, FcMatchPattern);
	if (!ret) {
		ERR_PRINT("linux_font_path_by_name(): couldn't find font file or system font with name " + font_name);
		return "";
	}
	FcDefaultSubstitute(pattern);
	FcResult result;
	FcPattern *font_match = nullptr;
	font_match = FcFontMatch(0, pattern, &result);
	if (!font_match) {
		ERR_PRINT("linux_font_path_by_name(): couldn't match font file or system font with name " + font_name);
		FcPatternDestroy(font_match);
		FcPatternDestroy(pattern);
		return "";
	}
	FcChar8 *file;
	if (FcPatternGetString(font_match, FC_FILE, 0, &file) == FcResultMatch) {
		filename = (const char *)file;
	} else {
		ERR_PRINT("linux_font_path_by_name(): couldn't find font match for " + font_name);
		FcPatternDestroy(font_match);
		FcPatternDestroy(pattern);
		return "";
	}
	FcPatternDestroy(font_match);
	FcPatternDestroy(pattern);
	return filename;
}
#endif

//-----------------------------------------------------------
static bool load_font_face(const String &font_name, FT_Face &face, String &filename, int index) {
	String fontname = font_name;
	filename = font_name + ".ttf";
	int fontID = index;
	if (!FileAccess::exists(filename.c_str())) {
#if defined(X11_ENABLED)
		filename = linux_font_path_by_name(font_name);
#elif defined(OSX_ENABLED)
		if (fontname == GD_TTF_SANS) {
			fontname = "Helvetica Neue";
#if MAC_OS_X_VERSION_10_13 && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_13
			fontID = 0;
#else
			fontID = 4;
#endif
		} else if (fontname == GD_TTF_SERIF) {
			fontname = "Times New Roman";
		} else if (fontname == GD_TTF_MONO) {
			fontname = "Menlo Regular";
		}
		filename = osx_font_path_by_name(fontname);
#elif defined(WINDOWS_ENABLED)
		if (fontname == GD_TTF_SANS) {
			fontname = "Arial";
		} else if (fontname == GD_TTF_SERIF) {
			fontname = "Times New Roman";
		} else if (fontname == GD_TTF_MONO) {
			fontname = "Courier New";
		}
		filename = win_font_path_by_name(fontname);
#endif
		if (filename == "") {
			ERR_PRINT("load_font_face(): couldn't find font " + fontname);
			return false;
		}
		WARN_PRINT("load_font_face(): " + fontname + " not a file in data loading system font from " + filename);
	}
	FT_Error err = FT_New_Face(library, filename.utf8().c_str(), fontID, &face);
	if (err) {
		// simple error table in lieu of full table (see fterrors.h)
		String error_string = "unknown freetype";
		if (err == 1)
			error_string = "INVALID FILENAME";
		ERR_PRINT("load_font_face(): couldn't create new face for " + fontname + ". FT_Error " + itos(err) + ", " + error_string);
		return false;
	}

	return true;
}

bool TrueTypeFontUtils::init_libraries() {
	if (!libraries_initialized) {
		FT_Error err = FT_Init_FreeType(&library);
		if (err) {
			ERR_PRINT("init_libraries(): couldn't initialize Freetype lib. FT_Error " + itos(err));
			return false;
		}
#ifdef X11_ENABLED
		FcBool result = FcInit();
		if (!result) {
			return false;
		}
#endif
#ifdef WINDOWS_ENABLED
		init_windows();
#endif
		libraries_initialized = true;
	}
	return true;
}

void TrueTypeFontUtils::set_text_property(TextProperties prop, Variant value) {
	switch (prop) {
		case TEXT_SPACESIZE: {
			space_size = value;
		} break;
		case TEXT_TABWIDTH: {
			tab_width = value;
		}
		case TEXT_DIRECTION: {
			int v = value;
			ERR_FAIL_COND_MSG(v != 1 && v != -1, "Only 1 and -1 are allowed");
			direction = value;
		}
		default:
			WARN_PRINT("Unsupported text property");
	}
}

Variant TrueTypeFontUtils::get_text_property(TextProperties prop) const {
	switch (prop) {
		case TEXT_SPACESIZE: {
			return space_size;
		} break;
		case TEXT_TABWIDTH: {
			return tab_width;
		} break;
		case TEXT_DIRECTION: {
			return direction;
		}
		case TEXT_FONTUNITSCALE: {
			return font_unit_scale;
		} break;
		default:
			WARN_PRINT("Unsupported text property");
	}
	return Variant();
}

TrueTypeFontUtils::TrueTypeFontUtils(const TrueTypeFontUtils &mom) :
		settings(mom.settings) {
	char_outlines = mom.char_outlines;
	char_outlines_non_vflipped = mom.char_outlines_non_vflipped;

	line_height = mom.line_height;
	ascender_height = mom.ascender_height;
	descender_height = mom.descender_height;
	glyph_bbox = mom.glyph_bbox;
	letter_spacing = mom.letter_spacing;
	space_size = mom.space_size;
	font_unit_scale = mom.font_unit_scale;

	cps = mom.cps; // properties for each character
	settings = mom.settings;
	glyph_index_map = mom.glyph_index_map;
	face = mom.face;
}

TrueTypeFontUtils &TrueTypeFontUtils::operator=(const TrueTypeFontUtils &mom) {
	if (this == &mom)
		return *this;
	settings = mom.settings;

	char_outlines = mom.char_outlines;
	char_outlines_non_vflipped = mom.char_outlines_non_vflipped;

	line_height = mom.line_height;
	ascender_height = mom.ascender_height;
	descender_height = mom.descender_height;
	glyph_bbox = mom.glyph_bbox;
	letter_spacing = mom.letter_spacing;
	space_size = mom.space_size;

	cps = mom.cps; // properties for each character
	settings = mom.settings;
	glyph_index_map = mom.glyph_index_map;
	face = mom.face;

	return *this;
}

TrueTypeFontUtils::TrueTypeFontUtils(TrueTypeFontUtils &&mom) :
		settings(std::move(mom.settings)) {
	char_outlines = std::move(mom.char_outlines);
	char_outlines_non_vflipped = std::move(mom.char_outlines_non_vflipped);

	line_height = mom.line_height;
	ascender_height = mom.ascender_height;
	descender_height = mom.descender_height;
	glyph_bbox = mom.glyph_bbox;
	letter_spacing = mom.letter_spacing;
	space_size = mom.space_size;
	font_unit_scale = mom.font_unit_scale;

	cps = mom.cps; // properties for each character
	settings = mom.settings;
	glyph_index_map = std::move(mom.glyph_index_map);
	face = mom.face;
}

TrueTypeFontUtils &TrueTypeFontUtils::operator=(TrueTypeFontUtils &&mom) {
	if (this == &mom)
		return *this;

	char_outlines = std::move(mom.char_outlines);
	char_outlines_non_vflipped = std::move(mom.char_outlines_non_vflipped);

	line_height = mom.line_height;
	ascender_height = mom.ascender_height;
	descender_height = mom.descender_height;
	glyph_bbox = mom.glyph_bbox;
	letter_spacing = mom.letter_spacing;
	space_size = mom.space_size;

	cps = mom.cps; // properties for each character
	settings = mom.settings;
	glyph_index_map = std::move(mom.glyph_index_map);
	face = mom.face;
	return *this;
}

TrueTypeFontUtils::glyph_props TrueTypeFontUtils::load_glyph(uint32_t utf8) const {
	glyph_props props;
	FT_Error err = FT_Load_Glyph(face.get(), FT_Get_Char_Index(face.get(), utf8), FT_LOAD_DEFAULT);
	if (err) {
		ERR_PRINT("load_glyph(): FT_Load_Glyph failed for utf8 code " + itos(utf8) + ". FT_Error " + itos(err));
		return props;
	}

	// info about the character:
	props.glyph = utf8;
	props.height = int26p6_to_dbl(face->glyph->metrics.height);
	props.width = int26p6_to_dbl(face->glyph->metrics.width);
	props.bearing_x = int26p6_to_dbl(face->glyph->metrics.horiBearingX);
	props.bearing_y = int26p6_to_dbl(face->glyph->metrics.horiBearingY);
	props.advance = int26p6_to_dbl(face->glyph->metrics.horiAdvance);

	return props;
}

bool TrueTypeFontUtils::load(const String &filename, int font_size, bool full_character_set, int dpi) {
	TrueTypeFontUtilsSettings settings(filename, font_size);
	settings.dpi = dpi;
	if (full_character_set) {
		settings.ranges = { UnicodeBlock::Latin1Supplement };
	} else {
		settings.ranges = { UnicodeBlock::Latin };
	}
	return load(settings);
}

bool TrueTypeFontUtils::load(const TrueTypeFontUtilsSettings &from_settings) {
	init_libraries();
	settings = from_settings;
	if (settings.dpi <= 0) {
		settings.dpi = 96;
	}

	//--------------- load the library and typeface
	FT_Face load_face;
	if (!load_font_face(settings.font_name, load_face, settings.font_name, settings.index)) {
		return false;
	}
	face = std::shared_ptr<struct FT_FaceRec_>(load_face, FT_Done_Face);

	if (settings.ranges.empty()) {
		settings.ranges.push_back(UnicodeBlock::Latin1Supplement);
	}

	FT_Set_Char_Size(face.get(), dbl_to_int26p6(settings.font_size), dbl_to_int26p6(settings.font_size), settings.dpi, settings.dpi);
	font_unit_scale = real_t(settings.font_size * settings.dpi) / (72 * face->units_per_EM);
	line_height = face->height * font_unit_scale;
	ascender_height = face->ascender * font_unit_scale;
	descender_height = face->descender * font_unit_scale;
	glyph_bbox = Rect2(
			face->bbox.xMin * font_unit_scale,
			face->bbox.yMin * font_unit_scale,
			(face->bbox.xMax - face->bbox.xMin) * font_unit_scale,
			(face->bbox.yMax - face->bbox.yMin) * font_unit_scale);

	//--------------- initialize character info and textures
	auto nglyphs = std::accumulate(settings.ranges.begin(), settings.ranges.end(), 0u,
			[](uint32_t acc, UnicodeBlock::range range) {
				return acc + range.get_num_glyphs();
			});
	cps.resize(nglyphs);

	char_outlines.resize(nglyphs);
	char_outlines_non_vflipped.resize(nglyphs);

	std::vector<TrueTypeFontUtils::glyph_props> all_glyphs;

	//--------------------- load each char -----------------------
	uint32_t i = 0;
	for (auto &range : settings.ranges) {
		for (uint32_t g = range.begin; g <= range.end; g++, i++) {
			all_glyphs.push_back(load_glyph(g));
			all_glyphs[i].character_index = i;
			glyph_index_map[g] = i;
			cps[i] = all_glyphs[i];

			char_outlines[i] = make_contours_for_character(face.get());
			char_outlines_non_vflipped[i] = char_outlines[i];
			char_outlines_non_vflipped[i].translate(Vector2(0, cps[i].height));
			char_outlines_non_vflipped[i].scale(1, -1);
		}
	}

	return true;
}

int TrueTypeFontUtils::get_size() const { return settings.font_size; }
real_t TrueTypeFontUtils::get_ascender_height() const { return ascender_height; }
real_t TrueTypeFontUtils::get_descender_height() const { return descender_height; }
const Rect2 &TrueTypeFontUtils::get_glyph_bbox() const { return glyph_bbox; }

TrueTypePath TrueTypeFontUtils::get_character_as_path(uint32_t character, bool vflip) const {
	if (!is_valid_glyph(character)) {
		return TrueTypePath();
	}
	return vflip ? char_outlines[index_for_glyph(character)] : char_outlines_non_vflipped[index_for_glyph(character)];
}

Vector<Vector<Point2>> TrueTypeFontUtils::get_character_as_points(const TrueTypePath &outlines, bool vflip, real_t simplify_amt, int resample_count) const {
	Vector<Vector<Point2>> result;
	if (!outlines.empty()) {
		std::vector<_polyline> polys = generate_polylines_from_commands(outlines.commands);
		for (auto &poly : polys) {
			if (simplify_amt > 0) {
				poly_simplify(poly_get_points(poly), simplify_amt);
			}
			if (resample_count > 0) {
				std::vector<real_t> lengths = poly_get_lengths(poly);
				const _polyline &reploy = poly_get_resampled_by_count(poly, lengths, resample_count);
				result.push_back(make_vector(poly_get_points(reploy).size(), poly_get_points(reploy).data()));
			} else {
				result.push_back(make_vector(poly_get_points(poly).size(), poly_get_points(poly).data()));
			}
		}
	}
	return result;
}

Vector<Vector<Point2>> TrueTypeFontUtils::get_character_as_points(uint32_t character, bool vflip, real_t simplify_amt, int resample_count) const {
	return get_character_as_points(get_character_as_path(character, vflip), vflip, simplify_amt, resample_count);
}

real_t TrueTypeFontUtils::get_kerning(uint32_t leftC, uint32_t rightC) const {
	if (FT_HAS_KERNING(face)) {
		FT_Vector kerning;
		FT_Get_Kerning(face.get(), FT_Get_Char_Index(face.get(), leftC), FT_Get_Char_Index(face.get(), rightC), FT_KERNING_UNFITTED, &kerning);
		return int26p6_to_dbl(kerning.x);
	} else {
		return 0;
	}
}

void TrueTypeFontUtils::iterate_string(const String &str, real_t x, real_t y, bool vflipped, std::function<void(uint32_t, Point2)> f) const {
	const real_t new_line_direction = vflipped ? 1 : -1;
	const real_t direction_x = direction;

	if (str.empty()) {
		return;
	}

	Point2 pos(x, y);

	uint32_t prevC = 0;
	for (const CharType c : str) {
		try {
			if (c == '\n') {
				pos.y += line_height * new_line_direction;
				pos.x = x; // reset x pos. back to zero
				prevC = 0;
			} else if (c == '\t') {
				if (direction == 1) {
					f(c, pos);
					pos.x += get_glyph_properties(' ').advance * space_size * tab_width * direction_x;
				} else {
					pos.x += get_glyph_properties(' ').advance * space_size * tab_width * direction_x;
					f(c, pos);
				}
				prevC = c;
			} else if (c == ' ') {
				pos.x += get_glyph_properties(' ').advance * space_size * direction_x;
				f(c, pos);
				prevC = c;
			} else if (is_valid_glyph(c)) {
				const auto &props = get_glyph_properties(c);
				if (prevC > 0) {
					if (direction == 1) {
						pos.x += get_kerning(prevC, c);
					} else {
						pos.x += get_kerning(c, prevC);
					}
				}
				if (direction == 1) {
					f(c, pos);
					pos.x += props.advance * direction_x;
					pos.x += get_glyph_properties(' ').advance * (letter_spacing - 1) * direction_x;
				} else {
					pos.x += props.advance * direction_x;
					pos.x += get_glyph_properties(' ').advance * (letter_spacing - 1) * direction_x;
					f(c, pos);
				}
				prevC = c;
			}
		} catch (...) {
			WARN_PRINT("Failed to process glyph: " + itos(c));
			break;
		}
	}
}

Vector<Vector<Point2>> TrueTypeFontUtils::get_string_as_points(const String &str, bool vflip, real_t simplify_amt) const {
	std::vector<TrueTypePath> shapes;
	iterate_string(str, 0, 0, vflip, [&](uint32_t c, Point2 pos) {
		shapes.push_back(get_character_as_path(c, vflip));
		shapes.back().translate(pos);
	});
	Vector<Vector<Point2>> result;

	return result;
}

bool TrueTypeFontUtils::is_valid_glyph(uint32_t glyph) const {
	return std::any_of(settings.ranges.begin(), settings.ranges.end(),
			[&](UnicodeBlock::range range) {
				return glyph >= range.begin && glyph <= range.end;
			});
}

size_t TrueTypeFontUtils::index_for_glyph(uint32_t glyph) const {
	return glyph_index_map.find(glyph)->second;
}

const TrueTypeFontUtils::glyph_props &TrueTypeFontUtils::get_glyph_properties(uint32_t glyph) const {
	if (is_valid_glyph(glyph)) {
		return cps[index_for_glyph(glyph)];
	} else {
		return InvalidProps;
	}
}

Rect2 TrueTypeFontUtils::get_string_bounding_box(const String &str, real_t x, real_t y, bool vflip) const {
	if (str.empty()) {
		return Rect2(x, y, 0, 0);
	}
	real_t minx = x, miny = y, maxy = y, w = 0;
	// Calculate bounding box by iterating over glyph properties
	// Meaning of props can be deduced from illustration at top of:
	// https://www.freetype.org/freetype2/docs/tutorial/step2.html
	//
	// We deliberately not generate a mesh and iterate over its
	// vertices, as this would not correctly return spacing for
	// blank characters.
	iterate_string(str, x, y, vflip, [&](uint32_t c, Point2 pos) {
		auto props = get_glyph_properties(c);
		if (c == '\t') {
			w += props.advance * space_size * tab_width;
		} else if (c == ' ') {
			w += props.advance * space_size;
		} else {
			w += props.advance;
		}

		minx = MIN(minx, pos.x);
		if (vflip) {
			miny = MIN(miny, pos.y);
			maxy = MAX(maxy, pos.y - (props.bearing_y - props.height));
		} else {
			miny = MIN(miny, pos.y);
			maxy = MAX(maxy, pos.y);
		}
	});
	real_t height = maxy - miny;
	return Rect2(minx, miny, w, height);
}

std::size_t TrueTypeFontUtils::get_num_characters() const { return cps.size(); }

// Example: https://github.com/junkiyoshi/Insta20240103/blob/main/ofApp.cpp
// ========================================================================

#ifdef TOOLS_ENABLED

#include "common/noise_gen.h"

void TrueTypeFontUtils::draw_demo(CanvasItem *p_canvas, int frame_num) {
	String word = "ABCDEFGHIJKLMNOPQRSTUVWXYZ!";
	int sample_count = 180;

	const Color color(1, 1, 0);

	for (int i = 0; i < word.size(); i++) {
		Vector<Vector<Point2>> outline = get_character_as_points(word[i], true, 0, sample_count);
		Vector<Point2> shape;
		for (int outline_index = 0; outline_index < int(outline.size()); outline_index++) {
			for (const Point2 &vertex : outline[outline_index]) {
				Point2 location = Transform2D(Math::deg2rad(i * 13.5), Size2::ONE, Vector2(settings.font_size * -0.5, settings.font_size * 0.5)).xform(vertex);
				const real_t noise_value = Math::map2(NOISE2(location.x * 0.003, frame_num * 0.01), 0, 0, 0.1, 1);
				shape.push_back(Transform2D(0, Size2(noise_value, noise_value), Vector2()).xform(location));
			}
		}
		p_canvas->draw_polyline(shape, color);
	}
}
#endif

TrueTypeFontUtils::TrueTypeFontUtils() :
		settings("", 0) {
	space_size = 1;
	letter_spacing = 1;
	tab_width = 4;
	ascender_height = 0;
	descender_height = 0;
	line_height = 0;
	direction = 1;
	font_unit_scale = 1;
}

TrueTypeFontUtils::~TrueTypeFontUtils() {}
