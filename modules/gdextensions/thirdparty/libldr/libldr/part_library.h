/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2008 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#ifndef LIBLDR_PART_LIBRARY_H
#define LIBLDR_PART_LIBRARY_H

#include <map>
#include <set>
#include <string>
#include <utility>

#include "libldr/common.h"
#include "libldr/reader.h"
#include "libldr/utils.h"

namespace ldraw
{

class element_ref;
class model;
class model_multipart;

enum part_lib_quality {
    QualityLow,
    QualityHigh,
    QualityDefault
};

class part_library
{
 public:
  enum path_type { ldraw_path, ldraw_parts_path, ldraw_primitives_path };
  enum unlink_policy { parts = 0x1, primitives = 0x2 };
  
  const part_lib_quality Part_lib_quality = QualityDefault;

  part_library();
  part_library(const std::string &path);
  ~part_library();
  
  const std::map<std::string, std::string>& part_list() const { return m_partlist; }
  const std::map<std::string, std::string>& prim_list() const { return m_primlist; }
  
  int get_unlink_policy() const { return m_unlink_policy; }
  void set_unlink_policy(int u) { m_unlink_policy = u; }
  
  std::string ldrawpath(path_type path_type = ldraw_path) const;
  std::string ldrawpath(const std::string &filename, path_type path_type = ldraw_parts_path) const;
  
  bool find(const std::string &name) const;
  int size() const { return m_data.size(); }
  
  void link(model_multipart *m);
  bool link_element(element_ref *r);
  void unlink(model_multipart *m);
  void unlink_element(element_ref *r);
  
 private:
  bool read_fs(const std::string &path);
  void link_model(model *m);
  
  std::map<std::string, std::string> m_partlist;
  std::map<std::string, std::string> m_primlist;
  std::map<std::string, item_refcount*> m_data;
  std::string m_ldrawpath;
  std::string m_partsdir;
  std::string m_primdir;
  int m_unlink_policy;
};

} // ldraw

#endif // LIBLDR_PART_LIBRARY_H
