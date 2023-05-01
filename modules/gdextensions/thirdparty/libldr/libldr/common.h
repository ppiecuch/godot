/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2008 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#ifndef LIBLDR_COMMON_H
#define LIBLDR_COMMON_H

#include <exception>
#include <string>

#ifdef WIN32
#define DIRECTORY_SEPARATOR "\\"
#else
#define DIRECTORY_SEPARATOR "/"
#endif

#ifndef __func__
#define __func__ __FUNCTION__
#endif

namespace ldraw
{

enum error_type { warning, user_error, fatal, fixme };
enum state { state_step, state_pause, state_clear, state_save };
enum cert_status { certified, uncertified, unknown };
enum winding { ccw, cw };
enum command { set_cw = 0x1, set_ccw = 0x2, clip = 0x04, clip_cw = 0x1 | 0x04, clip_ccw = 0x2 | 0x04, no_clip = 0x08, invert_next = 0x10 };
enum buffer_type { type_lines, type_triangles, type_quads, type_condlines };
enum model_type { primitive, part, submodel, general, external_file };

class model;

class filter
{
public:
	virtual ~filter() {}

	virtual bool query(const model *m, int index, int depth) const = 0;
};

class exception : public std::exception
{
	std::string m_location;
	error_type m_type;
	std::string m_details;
	std::string m_what;

	void _update()
	{
		m_what = "[libLDR] ";
		switch(m_type)
		{
			case warning:
				m_what += "warning";
				break;
			case user_error:
				m_what += "user_error";
				break;
			case fatal:
				m_what += "error";
				break;
			case fixme:
				m_what  += "fixme";
				break;
			default:
				m_what  += "(unknown)";
				break;
		}
		m_what += " in " + m_location + "(): " + m_details;
	}

public:
	virtual const char* what() const throw() { return m_what.c_str(); }

	const std::string location() const { return m_location; }
	error_type type() const { return m_type; }
	const std::string details() const { return m_details; }

	explicit exception(const std::string &location, error_type type, const std::string &details)
		: m_location(location), m_type(type), m_details(details) { _update(); }
	virtual ~exception() throw() {}
};

class extension
{
	model *m_model;
	void *m_arg;

public:
	void set_data(void *arg) { m_arg = arg; }
	model *get_model() const { return m_model; }

	virtual void update() {}

	extension(model *m, void *arg = 0) : m_model(m), m_arg(arg) {}
	virtual ~extension() {}
};

} // ldraw

#endif // LIBLDR_COMMON_H
