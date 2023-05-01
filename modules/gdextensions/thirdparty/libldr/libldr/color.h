/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2008 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#ifndef LIBLDR_COLOR_H
#define LIBLDR_COLOR_H

#include <map>
#include <vector>
#include <string>

#include "libldr/common.h"

namespace ldraw
{

enum material_type {
	material_normal,
	material_transparent,
	material_luminant,
	material_glitter,
	material_pearlescent,
	material_chrome,
	material_metallic,
	material_rubber,
	material_speckle
};

struct material_traits_speckle
{
	unsigned char color[3];
	float fraction;
	int minsize;
	int maxsize;
};

struct material_traits_glitter
{
	unsigned char color[3];
	float fraction;
	float vfraction;
	int size;
};

class color_entity
{
public:
	material_type material;
	union {
		unsigned char rgba[4]; // RGBA Array
		unsigned char rgbacolor; // RGBA Color
	};
	union {
		unsigned char complement[4]; // RGBA Array
		unsigned int complementcolor; // RGBA Color
	};
	char luminance;
	unsigned int id; // LDraw Color ID
	std::string name; // Name String
	const void *traits;
};

// Represents a color.
class color
{
	static bool m_initialized;

	void link();

	bool m_custom_color;
	bool m_valid;
	unsigned int m_id;
	const color_entity *m_entity;

public:
	static const material_traits_glitter material_chart_glitter[];
	static const material_traits_speckle material_chart_speckle[];
	static const color_entity color_chart[];
	static const int color_chart_count;
	static const std::map<unsigned int, const color_entity *> color_map;
	static std::vector<unsigned int> color_index; // all colors table

	static void init();

	void operator=(int cid) { m_id = cid; link(); }
	void operator=(const color &rhs) { m_id = rhs.get_id(); link(); }
	bool operator<(const color &rhs) const { return m_id < rhs.get_id(); }
	bool operator==(const color &rhs) const { return m_id == rhs.get_id(); }

	unsigned int get_id() const { return m_id; }
	void set_id(int i) { m_id = i; link(); }

	unsigned int get_index() const;
	static unsigned int from_index(unsigned int inx);
	static void save_index();

	bool is_valid() { return m_valid; }
	bool is_null() { return m_id == 16 || m_id == 24; }
	const color_entity* get_entity() const { return m_entity; }

	color() : m_valid(true), m_id(0) { link(); }
	color(unsigned int id) : m_id(id) { link(); }
	color(const color &c) : m_id(c.get_id()) { link(); }
	~color();
};

} // ldraw

#endif // LIBLDR_COLOR_H
