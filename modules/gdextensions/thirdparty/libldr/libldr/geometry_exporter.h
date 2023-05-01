/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2013 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#ifndef GEOMETRY_EXPORTER_H
#define GEOMETRY_EXPORTER_H

#include <map>
#include <stack>

#include "libldr/bfc.h"
#include "libldr/color.h"
#include "libldr/common.h"
#include "libldr/math.h"

namespace ldraw
{
	class model;
}

namespace lexporter
{

struct parameters
{
	enum stud_rendering_mode { stud_regular, stud_line, stud_square };
	enum render_method { model_full, model_edges, model_boundingboxes };

	parameters() : m_shading(false), m_culling(false), m_shader(false), m_debug(false) { }

	stud_rendering_mode get_stud_rendering_mode() const { return m_stud_mode; }
	render_method get_rendering_mode() const { return m_mode; }
	bool get_shading() const { return m_shading; }
	bool get_culling() const { return m_culling; }
	bool get_shader() const { return m_shader; }
	bool get_debug() const { return m_debug; }

	void set_stud_rendering_mode(stud_rendering_mode s) { m_stud_mode = s; }
	void set_rendering_mode(render_method m) { m_mode = m; }
	void set_shading(bool b) { m_shading = b; }
	void set_culling(bool b) { m_culling = b; }
	void set_shader(bool b) { m_shader = b; }
	void set_debug(bool b) { m_debug = b; }

	stud_rendering_mode m_stud_mode = stud_regular;
	render_method m_mode = model_full;
	bool m_shading;
	bool m_culling;
	bool m_shader;
	bool m_skip_condline;
	bool m_debug;
};

struct vbuffer_params
{
	bool force_fixed;
	bool collapse_subfiles = true;
	parameters params;
};

class geometry_exporter : public ldraw::extension
{
	static int s_memory_usage;

	ldraw::bfc_state_tracker m_bfc_tracker;
	vbuffer_params *m_params;

	bool m_isnull;
	bool m_colorfixed;
	parameters::stud_rendering_mode m_stud;

	int m_elemcnt[4];

	float *m_vertices[4];
	float *m_normals[2];
	float *m_colors[4];
	unsigned int *m_colorsindex[4];
	float *m_condparams;

	int m_vertptr[4];
	int m_normptr[2];
	int m_colorptr[4], m_colorindexptr[4];
	int m_condparamptr;

	std::map<ldraw::color, float **> m_precolored_buf;

	bool is_color_ambiguous() const;
	bool is_color_ambiguous_recursive(const ldraw::model *m) const;
	void fork_color(const ldraw::color &c);

	void count_elements_stud(const ldraw::model *m);
	void count_elements_recursive(const ldraw::model *m);
	void count_elements();

	void fill_element_atomic(const ldraw::vector &v, float *data, int *iterator, bool quadruple = false);
	void fill_element_atomic(const unsigned char *color, float *data, int *iterator);
	void fill_element_atomic(const float *cflag, float *data, int *iterator);
    void fill_element_atomic(const unsigned int color, unsigned int *data, int *iterator);

	void fill_color(const std::stack<ldraw::color> &colorstack, const ldraw::color &color, int count, ldraw::buffer_type type);
	void fill_elements_recursive(std::stack<ldraw::color> &colorstack, ldraw::model *m, const ldraw::matrix &transform);
	void fill_elements_stud(std::stack<ldraw::color> &colorstack, ldraw::model *m, const ldraw::matrix &transform);
	void fill_elements();

public:
	static int get_total_memory_usage();
	static void reset_total_memory_usage();

	void clear();
	void update();
	void update(bool collapse);

	bool is_null() const;
	bool is_update_required(bool collapse) const;

	int count(ldraw::buffer_type type) const;

	const float* get_vertex_array(ldraw::buffer_type type) const;
	const float* get_normal_array(ldraw::buffer_type type) const;
	const float* get_color_array(ldraw::buffer_type type) const;
	const unsigned int* get_color_index(ldraw::buffer_type type) const;
	const float* get_condline_direction_array() const;
	const float* get_precolored_array(ldraw::buffer_type type, const ldraw::color &c);

	geometry_exporter(ldraw::model *m, void *arg = 0);
	~geometry_exporter();
};

} // lrender

#endif // GEOMETRY_EXPORTER_H
