/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2013 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#include "libldr/elements.h"
#include "libldr/utils.h"
#include "libldr/model.h"

#include "libldr/normal_extension.h"
#include "libldr/geometry_exporter.h"

namespace lexporter
{

int geometry_exporter::s_memory_usage = 0;

int geometry_exporter::get_total_memory_usage()
{
	return s_memory_usage;
}

void geometry_exporter::reset_total_memory_usage()
{
	s_memory_usage = 0;
}

void geometry_exporter::clear()
{
	if (!m_isnull) {
		if (m_vertices[0] != 0L) {
			for (int i = 0; i < 4; ++i) {
				delete m_vertices[i];
				delete m_colors[i];
				delete m_colorsindex[i];
			}
			
			for (int i = 0; i < 2; ++i)
				delete m_normals[i];

			delete m_condparams;
		}
		
		for (int i = 0; i < 4; ++i)
			m_elemcnt[i] = 0;

		for (std::map<ldraw::color, float **>::iterator it = m_precolored_buf.begin(); it != m_precolored_buf.end(); ++it) {
			for (int i = 0; i < 4; ++i)
				delete (*it).second[i];
		}

		m_colorfixed = false;
		
		m_isnull = true;
	}
}

void geometry_exporter::update()
{
	clear();

	m_colorfixed = !is_color_ambiguous();

	int nbytes[4];
	int ncolorbytes[4];

	count_elements();

	m_stud = m_params->params.get_stud_rendering_mode();

	if (m_elemcnt[0] + m_elemcnt[1] + m_elemcnt[2] + m_elemcnt[3] == 0)
		return;

	m_isnull = false;

	for (int i = 0; i < 4; ++i) {
		nbytes[i] = 3 * m_elemcnt[i];
		ncolorbytes[i] = 4 * m_elemcnt[i];

		m_vertices[i] = new float[nbytes[i]];
		m_colors[i] = new float[ncolorbytes[i]];
		m_colorsindex[i] = new unsigned int[m_elemcnt[i]];

		s_memory_usage += nbytes[i] * sizeof(float);
		s_memory_usage += ncolorbytes[i] * sizeof(float);
	}

	m_normals[0] = new float[nbytes[1]];
	m_normals[1] = new float[nbytes[2]];

	m_condparams = new float[nbytes[3]];

	s_memory_usage += nbytes[1] * sizeof(float) + nbytes[2] * sizeof(float) + nbytes[3] * sizeof(float);

	fill_elements();
}

void geometry_exporter::update(bool collapse)
{
	m_params->collapse_subfiles = collapse;
	update();
}

bool geometry_exporter::is_null() const
{
	return m_isnull;
}

bool geometry_exporter::is_update_required(bool collapse) const
{
	if (m_params->collapse_subfiles != collapse || m_stud != m_params->params.get_stud_rendering_mode())
		return true;
	else
		return false;
}

int geometry_exporter::count(buffer_type type) const
{
	return m_elemcnt[type];
}

const float* geometry_exporter::get_vertex_array(buffer_type type) const
{
	if (m_isnull)
		return 0L;

	return m_vertices[type];
}

const float* geometry_exporter::get_normal_array(buffer_type type) const
{
	if (m_isnull)
		return 0L;

	if (type == type_triangles)
		return m_normals[0];
	else if (type == type_quads)
		return m_normals[1];

	return 0L;
}

const float* geometry_exporter::get_color_array(buffer_type type) const
{
	if (m_isnull)
		return 0L;

	return m_colors[type];
}

const unsigned int* geometry_exporter::get_color_index(buffer_type type) const
{
	if (m_isnull)
		return 0L;

	return m_colorsindex[type];
}

const float* geometry_exporter::get_condline_direction_array() const
{
	if (m_isnull)
		return 0L;

	return m_condparams;
}

const float* geometry_exporter::get_precolored_array(buffer_type type, const ldraw::color &c)
{
	if (m_isnull)
		return 0L;
	
	if (m_colorfixed)
		return get_color_array(type);

	if (m_precolored_buf.find(c) == m_precolored_buf.end())
		fork_color(c);

	return m_precolored_buf[c][type];
}

bool geometry_exporter::is_color_ambiguous() const
{
	return is_color_ambiguous_recursive(m_model);
}

bool geometry_exporter::is_color_ambiguous_recursive(const ldraw::model *m) const
{
	bool found;
	
	for (auto &it : m->elements()) {
		ldraw::type t = it->get_type();
		ldraw::color c;

		found = false;

		switch (t) {
			case ldraw::type_line:
				c = CAST_AS_CONST_LINE(it)->get_color();
				found = true;
				break;
			case ldraw::type_triangle:
				c = CAST_AS_CONST_TRIANGLE(it)->get_color();
				found = true;
				break;
			case ldraw::type_quadrilateral:
				c = CAST_AS_CONST_QUADRILATERAL(it)->get_color();
				found = true;
				break;
			case ldraw::type_condline:
				c = CAST_AS_CONST_CONDLINE(it)->get_color();
				found = true;
				break;
			case ldraw::type_ref:
				if (m_params->collapse_subfiles) {
					const ldraw::model *mm = CAST_AS_CONST_REF(it)->get_model();

					if (mm) {
						if (is_color_ambiguous_recursive(mm))
							return true;
					}
				}
				break;
			default:
				break;
		}

		if (found) {
			if (c.get_id() == 16 || c.get_id() == 24)
				return true;
		}
	}

	return false;
}

void geometry_exporter::fork_color(const ldraw::color &c)
{
	const ldraw::color_entity *ce = c.get_entity();

	float **colors = new float *[4];

	for (int i = 0; i < 4; ++i) {
		float *ctemp = m_colors[i];

		if (ctemp && m_elemcnt[i] > 0) {
			colors[i] = new float[4 * m_elemcnt[i]];
			float *cval = colors[i];

			s_memory_usage += 4 * m_elemcnt[i] * sizeof(float);

			for (int j = 0; j < m_elemcnt[i]; ++j) {
				if (*ctemp < -1.0f) {
					*(cval++) = ce->complement[0] / 255.0f;
					*(cval++) = ce->complement[1] / 255.0f;
					*(cval++) = ce->complement[2] / 255.0f;
					*(cval++) = ce->complement[3] / 255.0f;
					ctemp += 4;
				} else if (*ctemp < 0.0f) {
					*(cval++) = ce->rgba[0] / 255.0f;
					*(cval++) = ce->rgba[1] / 255.0f;
					*(cval++) = ce->rgba[2] / 255.0f;
					*(cval++) = ce->rgba[3] / 255.0f;
					ctemp += 4;
				} else {
					*(cval++) = *(ctemp++);
					*(cval++) = *(ctemp++);
					*(cval++) = *(ctemp++);
					*(cval++) = *(ctemp++);
				}
			}
		} else {
			colors[i] = 0L;
		}
	}

	if (m_precolored_buf.find(c) != m_precolored_buf.end()) {
		float **b = m_precolored_buf[c];

		for (int i = 0; i < 4; ++i)
			delete b[i];
		delete b;
	}
	m_precolored_buf[c] = colors;
}

void geometry_exporter::count_elements_stud(const ldraw::model *m)
{
	if (m_params->params.get_stud_rendering_mode() == parameters::stud_square)
		m_elemcnt[0] += 8;
	else if (m_params->params.get_stud_rendering_mode() == parameters::stud_line)
		m_elemcnt[0] += 2;
	else
		count_elements_recursive(m);
}

void geometry_exporter::count_elements_recursive(const ldraw::model *m)
{
	for (auto &it : m->elements()) {
		ldraw::type t = it->get_type();
		
		if (t == ldraw::type_line) {
			m_elemcnt[0] += 2;
		} else if (t == ldraw::type_triangle) {
			m_elemcnt[1] += 3;
		} else if (t == ldraw::type_quadrilateral) {
			m_elemcnt[2] += 4;
		} else if (t == ldraw::type_condline) {
			m_elemcnt[3] += 2;
		} else if (t == ldraw::type_ref && m_params->collapse_subfiles) {
			const ldraw::model *mm = CAST_AS_CONST_REF(it)->get_model();

			if (!mm)
				continue;

			if (ldraw::utils::is_stud(mm))
				count_elements_stud(mm);
			else
				count_elements_recursive(mm);
		}
	}
}

void geometry_exporter::count_elements()
{
	for (int i = 0; i < 4; ++i)
		m_elemcnt[i] = 0;

	count_elements_recursive(m_model);
}

void geometry_exporter::fill_element_atomic(const ldraw::vector &v, float *data, int *iterator, bool quadruple)
{
	data[(*iterator)++] = v.x();
	data[(*iterator)++] = v.y();
	data[(*iterator)++] = v.z();

	if (quadruple)
		data[(*iterator)++] = v.w();
}

void geometry_exporter::fill_element_atomic(const unsigned char *color, float *data, int *iterator)
{
	data[(*iterator)++] = color[0] / 255.0f;
	data[(*iterator)++] = color[1] / 255.0f;
	data[(*iterator)++] = color[2] / 255.0f;
	data[(*iterator)++] = color[3] / 255.0f;
}

void geometry_exporter::fill_element_atomic(const unsigned int color, unsigned int *data, int *iterator)
{
    data[(*iterator)++] = color;
}

void geometry_exporter::fill_element_atomic(const float *cflag, float *data, int *iterator)
{
	data[(*iterator)++] = cflag[0];
	data[(*iterator)++] = cflag[1];
}

void geometry_exporter::fill_color(const std::stack<ldraw::color> &colorstack, const ldraw::color &color, int count, buffer_type type)
{
	const char null[] = { -1, -1, -1, -1 };
	const char null_complement[] = { -2, -2, -2, -2 };

	int flag = 0;
	const unsigned char *ce;
	
	if (color.get_id() == 16) {
		if (colorstack.top().get_id() == 16) {
			ce = 0;
			flag = 1;
		} else {
			ce = colorstack.top().get_entity()->rgba;
		}
	} else if (color.get_id() == 24) {
		if (colorstack.top().get_id() == 16 || colorstack.top().get_id() == 24) {
			ce = 0L;
			flag = -1;
		} else {
			ce = colorstack.top().get_entity()->complement;
		}
	} else {
		ce = color.get_entity()->rgba;
	}

	// save one color for whole face
	if (ce)
		fill_element_atomic(*(unsigned int *)ce, m_colorsindex[type], &m_colorindexptr[type]);
	else {
		const char *cf;
		if (flag == 1)
			cf = null;
		else
			cf = null_complement;
		fill_element_atomic(*(unsigned int *)cf, m_colorsindex[type], &m_colorindexptr[type]);
    }

	for (int i = 0; i < count; ++i) {
		if (ce) {
            fill_element_atomic(ce, m_colors[type], &m_colorptr[type]);
		} else {
			const char *cf;

			if (flag == 1)
				cf = null;
			else
				cf = null_complement;
			
            fill_element_atomic(ldraw::vector(cf[0], cf[1], cf[2], cf[3]), m_colors[type], &m_colorptr[type], true);
		}
	}
}

void geometry_exporter::fill_elements_recursive(std::stack<ldraw::color> &colorstack, ldraw::model *m, const ldraw::matrix &transform)
{
	bool invertnext = false;
	ldraw::bfc_certification::winding winding = ldraw::bfc_certification::ccw; // default
	ldraw::bfc_certification::cert_status cert;
	const ldraw::bfc_certification *cext = m->custom_data<ldraw::bfc_certification>();

	if (!cext)
		cert = ldraw::bfc_certification::unknown;
	else {
		cert = cext->certification();
		if (cert == ldraw::bfc_certification::certified) {
			winding = cext->orientation();
			if (m_bfc_tracker.localinverted()) {
				winding = winding == ldraw::bfc_certification::ccw ? ldraw::bfc_certification::cw : ldraw::bfc_certification::ccw;
            }
		}
	}

	if (!m->custom_data<ldraw::normal_extension>())
		m->update_custom_data<ldraw::normal_extension>();

	ldraw::matrix transform_wo_position = transform;
	transform_wo_position.set_translation_vector(ldraw::vector());

	const std::map<int, ldraw::vector> &norms = m->custom_data<ldraw::normal_extension>()->normals();
	
	bool flipped = ldraw::utils::det3(transform) < 0.0f;

	int i = 0;
	for (auto &it : m->elements()) {
		ldraw::type t = it->get_type();
		
		if (t == ldraw::type_line) {
			const ldraw::element_line *l = CAST_AS_CONST_LINE(it);

			fill_element_atomic(transform * l->pos1(), m_vertices[0], &m_vertptr[0]);
			fill_element_atomic(transform * l->pos2(), m_vertices[0], &m_vertptr[0]);

			fill_color(colorstack, l->get_color(), 2, type_lines);
		} else if (t == ldraw::type_triangle) {
			const ldraw::element_triangle *l = CAST_AS_CONST_TRIANGLE(it);

			bool flip_winding =
				(winding == ldraw::bfc_certification::ccw && flipped) ||
				(winding == ldraw::bfc_certification::cw && !flipped);

			if (flip_winding) {
				fill_element_atomic(transform * l->pos3(), m_vertices[1], &m_vertptr[1]);
				fill_element_atomic(transform * l->pos2(), m_vertices[1], &m_vertptr[1]);
				fill_element_atomic(transform * l->pos1(), m_vertices[1], &m_vertptr[1]);
			} else {
				fill_element_atomic(transform * l->pos1(), m_vertices[1], &m_vertptr[1]);
				fill_element_atomic(transform * l->pos2(), m_vertices[1], &m_vertptr[1]);
				fill_element_atomic(transform * l->pos3(), m_vertices[1], &m_vertptr[1]);
			}

			ldraw::vector n = transform_wo_position * (*norms.find(i)).second;
			
			fill_element_atomic(n, m_normals[0], &m_normptr[0]);
			fill_element_atomic(n, m_normals[0], &m_normptr[0]);
			fill_element_atomic(n, m_normals[0], &m_normptr[0]);

			fill_color(colorstack, l->get_color(), 3, type_triangles);
		} else if (t == ldraw::type_quadrilateral) {
			const ldraw::element_quadrilateral *l = CAST_AS_CONST_QUADRILATERAL(it);

			bool flip_winding =
				(winding == ldraw::bfc_certification::ccw && flipped) ||
				(winding == ldraw::bfc_certification::cw && !flipped);

			if (flip_winding) {
				fill_element_atomic(transform * l->pos4(), m_vertices[2], &m_vertptr[2]);
				fill_element_atomic(transform * l->pos3(), m_vertices[2], &m_vertptr[2]);
				fill_element_atomic(transform * l->pos2(), m_vertices[2], &m_vertptr[2]);
				fill_element_atomic(transform * l->pos1(), m_vertices[2], &m_vertptr[2]);
			} else {
				fill_element_atomic(transform * l->pos1(), m_vertices[2], &m_vertptr[2]);
				fill_element_atomic(transform * l->pos2(), m_vertices[2], &m_vertptr[2]);
				fill_element_atomic(transform * l->pos3(), m_vertices[2], &m_vertptr[2]);
				fill_element_atomic(transform * l->pos4(), m_vertices[2], &m_vertptr[2]);
			}

			ldraw::vector n = transform_wo_position * (*norms.find(i)).second;
			
			fill_element_atomic(n, m_normals[1], &m_normptr[1]);
			fill_element_atomic(n, m_normals[1], &m_normptr[1]);
			fill_element_atomic(n, m_normals[1], &m_normptr[1]);
			fill_element_atomic(n, m_normals[1], &m_normptr[1]);

			fill_color(colorstack, l->get_color(), 4, type_quads);
		} else if (t == ldraw::type_condline) {
			const ldraw::element_condline *l = CAST_AS_CONST_CONDLINE(it);

			fill_element_atomic(transform * l->pos1(), m_vertices[3], &m_vertptr[3]);
			fill_element_atomic(transform * l->pos2(), m_vertices[3], &m_vertptr[3]);

			fill_color(colorstack, l->get_color(), 2, type_condlines);			
		} else if (t == ldraw::type_ref && m_params->collapse_subfiles) {
			ldraw::element_ref *l = CAST_AS_REF(it);
			ldraw::model *m = l->get_model();
			if (m) {

				bool reverse = ldraw::utils::det3(transform * l->get_matrix()) < 0.0f;

				const ldraw::color &c = l->get_color();

				if (c.get_id() == 16 || c.get_id() == 24)
					colorstack.push(colorstack.top());
				else
					colorstack.push(c);

				m_bfc_tracker.accumulate_invert(invertnext, reverse);
				if (ldraw::utils::is_stud(m))
					fill_elements_stud(colorstack, m, transform * l->get_matrix());
                else
					fill_elements_recursive(colorstack, m, transform * l->get_matrix());

				m_bfc_tracker.pop_invert();
				colorstack.pop();

				invertnext = false;
			}
		} else if (t == ldraw::type_bfc) {
			// Back Face Culling (BFC)
			const ldraw::element_bfc *l = CAST_AS_CONST_BFC(it);

			if (l->get_command() & ldraw::element_bfc::cw)
				winding = ldraw::bfc_certification::cw;
			else if (l->get_command() & ldraw::element_bfc::ccw)
				winding = ldraw::bfc_certification::ccw;

			if (m_bfc_tracker.inverted())
				winding = winding == ldraw::bfc_certification::cw ? ldraw::bfc_certification::ccw : ldraw::bfc_certification::cw;

			if (l->get_command() == ldraw::element_bfc::invertnext)
				invertnext = true;
		}
		++i;
	}
}

void geometry_exporter::fill_elements_stud(std::stack<ldraw::color> &colorstack, ldraw::model *m, const ldraw::matrix &transform)
{
	if (m_params->params.get_stud_rendering_mode() == parameters::stud_square) {
		ldraw::vector v1(-6.0f, -4.0f, -6.0f);
		ldraw::vector v2(6.0f, -4.0f, -6.0f);
		ldraw::vector v3(6.0f, -4.0f, 6.0f);
		ldraw::vector v4(-6.0f, -4.0f, 6.0f);
		
		v1 = transform * v1;
		v2 = transform * v2;
		v3 = transform * v3;
		v4 = transform * v4;
		
		fill_element_atomic(v1, m_vertices[0], &m_vertptr[0]);
		fill_element_atomic(v2, m_vertices[0], &m_vertptr[0]);
		
		fill_element_atomic(v2, m_vertices[0], &m_vertptr[0]);
		fill_element_atomic(v3, m_vertices[0], &m_vertptr[0]);
		
		fill_element_atomic(v3, m_vertices[0], &m_vertptr[0]);
		fill_element_atomic(v4, m_vertices[0], &m_vertptr[0]);
		
		fill_element_atomic(v4, m_vertices[0], &m_vertptr[0]);
		fill_element_atomic(v1, m_vertices[0], &m_vertptr[0]);

		fill_color(colorstack, ldraw::color(24), 8, type_lines);
	} else if (m_params->params.get_stud_rendering_mode() == parameters::stud_line) {
		fill_element_atomic(transform * ldraw::vector(0.0f, 0.0f, 0.0f), m_vertices[0], &m_vertptr[0]);
		fill_element_atomic(transform * ldraw::vector(0.0f, -4.0f, 0.0f), m_vertices[0], &m_vertptr[0]);

		fill_color(colorstack, ldraw::color(24), 2, type_lines);
	} else if (m_params->params.get_stud_rendering_mode() == parameters::stud_regular) {
		fill_elements_recursive(colorstack, m, transform);
	}
}

void geometry_exporter::fill_elements()
{
	/* initialize pointers */
	for (int i = 0; i < 2; ++i)
		m_normptr[i] = 0;

	for (int i = 0; i < 4; ++i) {
		m_vertptr[i] = 0;
		m_colorptr[i] = 0;
	}

	m_condparamptr = 0;

	ldraw::matrix transform;

	float *m = const_cast<float *>(transform.get_pointer());
	m[5] = -1.0; // verical mirror (https://code-industry.net/masterpdfeditor-help/transformation-matrix/)

	std::stack<ldraw::color> colorstack;

	colorstack.push(ldraw::color(16));

	fill_elements_recursive(colorstack, m_model, transform);
}

geometry_exporter::geometry_exporter(ldraw::model *m, void *arg) : ldraw::extension(m, arg)
{
	m_params = new vbuffer_params;
	std::memcpy(m_params, arg, sizeof(vbuffer_params));

	m_isnull = true;

	for (int i = 0; i < 4; ++i) {
		m_elemcnt[i] = 0;

		m_vertptr[i] = 0;
		m_colorptr[i] = 0;
		m_colorindexptr[i] = 0;

		m_vertices[i] = 0L;
		m_colors[i] = 0L;
		m_colorsindex[i] = 0L;
	}

	for (int i = 0; i < 2; ++i) {
		m_normptr[i] = 0;
		m_normals[i] = 0L;
	}

	m_condparams = 0L;
	m_condparamptr = 0;

	m_colorfixed = false;
}

geometry_exporter::~geometry_exporter()
{
	clear();
	delete m_params;
}

} // ldraw
