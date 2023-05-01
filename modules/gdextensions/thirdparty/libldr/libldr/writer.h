/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2008 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#ifndef LIBLDR_WRITER_H
#define LIBLDR_WRITER_H

#include <ostream>
#include <string>

#include "libldr/common.h"

namespace ldraw
{

class model;
class model_multipart;
class element_base;
class element_comment;
class element_state;
class element_print;
class element_ref;
class element_line;
class element_triangle;
class element_quadrilateral;
class element_condline;
class element_bfc;
class matrix;
class vector;

class writer
{
	std::ofstream *m_filestream;
	std::ostream &m_stream;

public:
	void write(const model *model);
	void write(const model_multipart *mpmodel);
	void write(const element_base *elem);

	void serialize_comment(const element_comment *e);
	void serialize_state(const element_state *e);
	void serialize_print(const element_print *e);
	void serialize_ref(const element_ref *e);
	void serialize_line(const element_line *e);
	void serialize_triangle(const element_triangle *e);
	void serialize_quadrilateral(const element_quadrilateral *e);
	void serialize_condline(const element_condline *e);
	void serialize_bfc(const element_bfc *e);
	void serialize_matrix(const matrix &m);
	void serialize_vector(const vector &v);
	
	writer(const std::string &filename);
	writer(std::ostream &stream);
	virtual ~writer();
};

} // ldraw

#endif // LIBLDR_WRITER_H

