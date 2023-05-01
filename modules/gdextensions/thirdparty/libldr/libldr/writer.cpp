/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2008 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#include <fstream>
#include <ios>

#include "libldr/bfc.h"
#include "libldr/elements.h"
#include "libldr/model.h"
#include "libldr/writer.h"

namespace ldraw
{

writer::writer(const std::string &filename)
	: m_filestream(new std::ofstream), m_stream(*m_filestream)
{
	m_filestream->open(filename.c_str(), std::ios::out);
	if (!m_filestream->is_open())
		throw exception(__func__, exception::user_error, std::string("Could not open file for writing: ") + filename);
}

writer::writer(std::ostream &stream)
	: m_stream(stream)
{
	m_filestream = 0L;
}

writer::~writer()
{
	if (m_filestream) {
		m_filestream->close();
		delete m_filestream;
	}
}

void writer::write(const model *model)
{
	m_stream << "0 " << model->desc() << std::endl;
	if (!model->name().empty())
		m_stream << "0 Name: " << model->name() << std::endl;
	if (!model->author().empty())
		m_stream << "0 Author: " << model->author() << std::endl << std::endl;

	if (model->custom_data<bfc_certification>()) {
		bfc_certification *c = model->custom_data<bfc_certification>();
		if (c->certification() == bfc_certification::certified) {
			m_stream << "0 BFC";
			
			if (c->orientation() == bfc_certification::cw)
				m_stream << " CW";

			m_stream << std::endl << std::endl;;
		} else if (c->certification() == bfc_certification::uncertified) {
			m_stream << "0 BFC NOCERTIFY" << std::endl << std::endl;
		}
	}
	
	const std::multimap<std::string, std::string> &headers = model->headers();
	for (std::multimap<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		m_stream << "0 !" << (*it).first << " " << (*it).second << std::endl;
	m_stream << std::endl;
	
	for (model::const_iterator it = model->elements().begin(); it != model->elements().end(); ++it)
		write(*it);
}
	
void writer::write(const model_multipart *mpmodel)
{
	m_stream << "0 FILE " << mpmodel->main_model()->name() << std::endl;

	write(mpmodel->main_model());

	for (std::map<std::string, model *>::const_iterator it = mpmodel->submodel_list().begin(); it != mpmodel->submodel_list().end(); ++it) {
		m_stream << std::endl << "0 FILE " << (*it).second->name() << std::endl;
		write((*it).second);
	}
}

void writer::write(const element_base *elem)
{
	switch (elem->get_type()) {
		case type_comment:
			serialize_comment(CAST_AS_CONST_COMMENT(elem));
			break;
		case type_state:
			serialize_state(CAST_AS_CONST_STATE(elem));
			break;
		case type_print:
			serialize_print(CAST_AS_CONST_PRINT(elem));
			break;
		case type_ref:
			serialize_ref(CAST_AS_CONST_REF(elem));
			break;
		case type_line:
			serialize_line(CAST_AS_CONST_LINE(elem));
			break;
		case type_triangle:
			serialize_triangle(CAST_AS_CONST_TRIANGLE(elem));
			break;
		case type_quadrilateral:
			serialize_quadrilateral(CAST_AS_CONST_QUADRILATERAL(elem));
			break;
		case type_condline:
			serialize_condline(CAST_AS_CONST_CONDLINE(elem));
			break;
		case type_bfc:
			serialize_bfc(CAST_AS_CONST_BFC(elem));
			break;
		default:
			break;
	}
}

void writer::serialize_comment(const element_comment *e)
{
	m_stream << "0 " << e->get_comment() << std::endl;
}

void writer::serialize_state(const element_state *e)
{
	m_stream << "0 ";

	switch (e->get_state()) {
		case element_state::state_step:
			m_stream << "STEP";
			break;
		case element_state::state_pause:
			m_stream << "PAUSE";
			break;
		case element_state::state_clear:
			m_stream << "CLEAR";
			break;
		case element_state::state_save:
			m_stream << "SAVE";
			break;
	}

	m_stream << std::endl;
}

void writer::serialize_print(const element_print *e)
{
	m_stream << "0 PRINT " << e->get_string() << std::endl;
}

void writer::serialize_ref(const element_ref *e)
{
	m_stream << "1 " << e->get_color().get_id() << " ";
	serialize_matrix(e->get_matrix());
	m_stream << " " << e->filename() << std::endl;
}

void writer::serialize_line(const element_line *e)
{
	m_stream << "2 " << e->get_color().get_id() << " ";
	serialize_vector(e->pos1());
	m_stream << " ";
	serialize_vector(e->pos2());
	m_stream << std::endl;
}

void writer::serialize_triangle(const element_triangle *e)
{
	m_stream << "3 " << e->get_color().get_id() << " ";
	serialize_vector(e->pos1());
	m_stream << " ";
	serialize_vector(e->pos2());
	m_stream << " ";
	serialize_vector(e->pos3());
	m_stream << std::endl;
}
	
void writer::serialize_quadrilateral(const element_quadrilateral *e)
{
	m_stream << "4 " << e->get_color().get_id() << " ";
	serialize_vector(e->pos1());
	m_stream << " ";
	serialize_vector(e->pos2());
	m_stream << " ";
	serialize_vector(e->pos3());
	m_stream << " ";
	serialize_vector(e->pos4());
	m_stream << std::endl;
}

void writer::serialize_condline(const element_condline *e)
{
	m_stream << "5 " << e->get_color().get_id() << " ";
	serialize_vector(e->pos1());
	m_stream << " ";
	serialize_vector(e->pos2());
	m_stream << " ";
	serialize_vector(e->pos3());
	m_stream << " ";
	serialize_vector(e->pos4());
	m_stream << std::endl;
}

void writer::serialize_bfc(const element_bfc *e)
{
	m_stream << "0 BFC ";
	
	switch (e->get_command()) {
		case element_bfc::cw:
			m_stream << "CW";
			break;
		case element_bfc::ccw:
			m_stream << "CCW";
			break;
		case element_bfc::clip:
			m_stream << "CLIP";
			break;
		case element_bfc::clip_cw:
			m_stream << "CLIP CW";
			break;
		case element_bfc::clip_ccw:
			m_stream << "CLIP CCW";
			break;
		case element_bfc::noclip:
			m_stream << "NOCLIP";
			break;
		case element_bfc::invertnext:
			m_stream << "INVERTNEXT";
			break;
	}

	m_stream << std::endl;
}

void writer::serialize_matrix(const matrix &m)
{
	m_stream << m.value(0, 3) << " " << m.value(1, 3) << " " << m.value(2, 3) <<
		" " << m.value(0, 0) << " " << m.value(0, 1) << " " << m.value(0, 2) <<
		" " << m.value(1, 0) << " " << m.value(1, 1) << " " << m.value(1, 2) <<
		" " << m.value(2, 0) << " " << m.value(2, 1) << " " << m.value(2, 2);
}

void writer::serialize_vector(const vector &v)
{
	m_stream << v.x() << " " << v.y() << " " << v.z();
}

} // ldraw
