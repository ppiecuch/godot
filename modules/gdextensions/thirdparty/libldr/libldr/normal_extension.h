/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2013 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#ifndef LIBLDR_NORMAL_EXTENSION_H
#define LIBLDR_NORMAL_EXTENSION_H

#include <map>

#include "libldr/extension.h"
#include "libldr/math.h"

namespace ldraw 
{

class model;

class normal_extension : public ldraw::extension
{
	std::map<int, ldraw::vector> m_normals;

	static ldraw::vector calculate_normal(const ldraw::vector &v1, const ldraw::vector &v2, const ldraw::vector &v3);

  public:
	static const std::string identifier() { return "normal_extension"; }

	void update();

	bool has_normal(int idx) const;
	ldraw::vector normal(int idx) const;
	const std::map<int, ldraw::vector>& normals() const;

	normal_extension(ldraw::model *m, void *arg);
	~normal_extension();
};

}

#endif // LIBLDR_NORMAL_EXTENSION_H
