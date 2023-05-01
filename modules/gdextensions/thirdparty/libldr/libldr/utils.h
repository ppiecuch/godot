/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2008 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#ifndef LIBLDR_UTILS_H
#define LIBLDR_UTILS_H

#include <list>
#include <string>
#include <vector>

#include "libldr/common.h"
#include "libldr/math.h"

namespace ldraw
{

class element_ref;
class element_quadrilateral;
class model;
class model_multipart;
class part_library;

class item_refcount : public std::pair<model_multipart *, int>
{
public:
	void set_model(model_multipart *m) { first = m, second = 0; }

	void acquire() { ++second; }
	void release() { --second; }
	int refcount() const { return second; }

	model_multipart* model() { return first; }

	item_refcount() { first = nullptr, second = 0; }
	item_refcount(model_multipart *m) { set_model(m); }
	~item_refcount() { if(first) delete first; }
};

namespace utils
{

// Cyclic reference test
bool cyclic_reference_test(const model *m);
bool cyclic_reference_test(const model *m, const model *insert);

// Returns a list of submodels (including model) which could be affected by a model
std::list<model *> affected_models(model_multipart *base, model *m);

// Name duplicate test
bool name_duplicate_test(const std::string &name, const model_multipart *model);
bool name_duplicate_test(const std::string &name, const part_library &library);

// Fix bowtie quads
void validate_bowtie_quad(element_quadrilateral &quad);
void validate_bowtie_quads(model *model);

// String handling
std::string translate_string(const std::string &str);
std::string trim_string(const std::string &str);
std::vector<std::string> split_string(const std::string &s, char delim = ' ');
bool begins_with(const std::string &t, const std::string &s);
std::string &replaceall(std::string &input, std::string fromstr, std:: string tostr);

// Miscellaneous
bool is_stud(const model *m);
bool is_stud(const element_ref *ref);

// Math
float det3(const matrix &m);
bool is_singular_matrix(const matrix &m);
bool is_orthogonal(const matrix &m);

} // utils

} // ldraw

#endif // LIBLDR_UTILS_H
