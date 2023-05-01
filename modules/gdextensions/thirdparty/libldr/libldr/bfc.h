/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2009 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#ifndef LIBLDR_BFC_H
#define LIBLDR_BFC_H

#include <stack>

#include "libldr/elements.h"
#include "libldr/common.h"

#define CAST_AS_BFC(c)       (dynamic_cast<ldraw::element_bfc *>(c))
#define CAST_AS_CONST_BFC(c) (dynamic_cast<const ldraw::element_bfc *>(c))

namespace ldraw
{

class bfc_certification : public extension
{
	cert_status m_cert;
	winding m_ori;

public:
	static const std::string identifier() { return "bfc_certification"; }

	bfc_certification& operator=(const bfc_certification &rhs);
	bfc_certification& operator=(cert_status cert);

	cert_status certification() const { return m_cert; }
	winding orientation() const { return m_ori; }

	void set_certification(cert_status cert) { m_cert = cert; }
	void set_orientation(winding ori) { m_ori = ori; }

	bfc_certification(model *m, void *arg);
	bfc_certification(cert_status cert, winding ori = ccw);
	virtual ~bfc_certification() {}
};

class element_bfc : public element_base
{
	command m_cmd;

public:
	command get_command() const;
	void set_command(command cmd);

	virtual type get_type() const { return type_bfc; }
	virtual int line_type() const { return 0; }

	element_bfc& operator= (const element_bfc &rhs);

	element_bfc(command cmd);
	element_bfc(const element_bfc &b);
	virtual ~element_bfc() {}
};

class bfc_state_tracker
{
	std::stack<int> m_cullstack;
	std::stack<int> m_invertstack;
	std::stack<int> m_localinvertstack;

public:
	bool culling() const;
	bool inverted() const;
	bool localinverted() const;

	void accumulate_culling(bool b);
	void accumulate_invert(bool b, bool r);

	void pop_culling();
	void pop_invert();

	bfc_state_tracker();
	~bfc_state_tracker();
};

} // ldraw

#endif // LIBLDR_BFC_H
