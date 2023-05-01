/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2008 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#include <sys/types.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>

#include "libldr/model.h"
#include "libldr/reader.h"
#include "libldr/utils.h"
#include "libldr/part_library.h"

namespace ldraw
{

#ifdef WIN32
// For win32
static const std::string predefined_path[] = {
  "c:\\ldraw",
  "c:\\Program Files\\ldraw",
  "c:\\Program Files (x86)\\ldraw",
  ""
};
#else
static const std::string predefined_path[] = {
  "/usr/share/ldraw",
  "/usr/share/LDRAW",
  "/usr/local/share/ldraw",
  "/usr/local/share/LDRAW",
  "/opt/ldraw",
  "/opt/LDRAW",
  "~/.ldraw",
  "~/.LDRAW",
  ""
};
#endif

part_library::part_library()
{
  m_unlink_policy = parts | primitives;
  
  char *tmp = getenv("LDRAWDIR");
  
  if (!tmp) {
    int i = 0;
    while (!predefined_path[i].empty()) {
      if (read_fs(predefined_path[i])) {
        return;
      }
      ++i;
    }
  } else {
    if (read_fs(std::string(tmp))) {
      return;
    }
  }
  
#ifndef LDR_ARCHIVE_SUPPORT
  throw exception(__func__, exception::fatal, "Couldn't find LDraw part library. Please install valid LDraw part library and make sure that \"LDRAWDIR\" environment variable is set!");
#endif
}

part_library::part_library(const std::string &path)
{
  m_unlink_policy = parts | primitives;
  
  if(!read_fs(path))
    throw exception(__func__, exception::fatal, "Couldn't find LDraw part library.");
}

part_library::~part_library()
{
  // FIXME Apparently not working.
#if 0
  for (std::map<std::string, item_refcount*>::iterator it = m_data.begin(); it != m_data.end(); ++it)
    delete (*it).second;
#endif
}

std::string part_library::ldrawpath(path_type path_type) const
{
  switch (path_type) {
    case ldraw_path:
      return m_ldrawpath;
    case ldraw_parts_path:
      return m_ldrawpath + DIRECTORY_SEPARATOR + m_partsdir;
    case ldraw_primitives_path:
      return m_ldrawpath + DIRECTORY_SEPARATOR + m_primdir;
    default:
      return std::string();
  }
}

std::string part_library::ldrawpath(const std::string &filename, path_type path_type) const
{
  return ldrawpath(path_type) + DIRECTORY_SEPARATOR + filename;
}

bool part_library::find(const std::string &name) const
{
  std::string lowercase = utils::translate_string(name);
  
  if (m_partlist.find(lowercase) != m_partlist.end())
    return true;
  else if (m_primlist.find(lowercase) != m_primlist.end())
    return true;
  
  return false;
}	

void part_library::link(model_multipart *m)
{
  link_model(m->main_model());
  
  auto &list = m->submodel_list();
  for(auto &e : list)
    link_model(e.second);
}

bool part_library::link_element(element_ref *r)
{
  if (r->get_model())
    return true;
  
  if (r->linkpoint())
    r->linkpoint()->unlink_element(r);

  // 1. find the submodel if multipart
  if (r->parent() && r->parent()->parent())
    if (r->parent()->parent()->link_submodel_element(r))
      return true;

 reader nil;
 std::string fn = utils::translate_string(r->filename());

  // 2. find the model pool.
  auto it1 = m_data.find(fn);
  if (it1 != m_data.end()) {
    (*it1).second->acquire();
    r->set_model((*it1).second->model()->main_model());
    r->resolve(this);
    return true;
  }

  // 3. find the primitive list
  std::map<std::string, std::string>::iterator it2 = m_primlist.find(fn);
  if (it2 != m_primlist.end()) {
    item_refcount *n = nil.load_with_cache(m_ldrawpath + DIRECTORY_SEPARATOR + m_primdir + DIRECTORY_SEPARATOR + (*it2).second, fn, model::primitive);
    link(n->model());
    r->set_model(n->model()->main_model());
    n->model()->main_model()->set_modeltype(model::primitive);
    m_data[fn] = n;
    m_data[fn]->acquire();
    r->resolve(this);
    return true;
  }
  
  // 4. find the parts list
  std::map<std::string, std::string>::iterator it3 = m_partlist.find(fn);
  if (it3 != m_partlist.end()) {
      item_refcount *n = nil.load_with_cache(m_ldrawpath + DIRECTORY_SEPARATOR + m_partsdir + DIRECTORY_SEPARATOR + (*it3).second, fn, model::part);
    link(n->model());
    r->set_model(n->model()->main_model());
    n->model()->main_model()->set_modeltype(model::part);
    m_data[fn] = n;
    m_data[fn]->acquire();
    r->resolve(this);
    return true;
  }

  // 5. check the archive at the end (might be the only options)
#ifdef LDR_ARCHIVE_SUPPORT
  item_refcount *n = nil.load_from_archive(fn);
  if (n && n->model()) {
    link(n->model());
    r->set_model(n->model()->main_model());
    r->resolve(this);
    return true;
  }
#endif
  if (r->parent())
    std::cerr << "File " << r->parent()->name() << ": Could not open file: " << r->filename() << std::endl;
  else
    std::cerr << "Could not open file: " << r->filename() << std::endl;
  
  return false;
}

void part_library::unlink(model_multipart *m)
{
  for (int i = 0; i < m->main_model()->size(); ++i) {
    if (m->main_model()->at(i)->get_type() == type_ref)
      unlink_element(CAST_AS_REF(m->main_model()->at(i)));
  }
  
  std::map<std::string, model*> &list = m->submodel_list();
  for (std::map<std::string, model*>::iterator it = list.begin(); it != list.end(); ++it) {
    model *tm = (*it).second;
    for (int i = 0; i < tm->size(); ++i) {
      if (tm->at(i)->get_type() == type_ref)
        unlink_element(CAST_AS_REF(tm->at(i)));
    }
  }
}

void part_library::unlink_element(element_ref *r)
{
  if (r->linkpoint() != this)
    return;
  
  std::string fn = utils::translate_string(r->filename());
  
  if (r->get_model() && (r->get_model()->modeltype() == model::submodel || r->get_model()->modeltype() == model::external_file))
    return;
  
  std::map<std::string, item_refcount*>::iterator it = m_data.find(fn);
  if (r->get_model() && it != m_data.end()) {
    (*it).second->release();
    if (!(*it).second->refcount()) {
      if (((*it).second->model()->main_model()->modeltype() == model::part && m_unlink_policy & parts) ||
          ((*it).second->model()->main_model()->modeltype() == model::primitive && m_unlink_policy & primitives)) {
        delete (*it).second;
        m_data.erase(it);
      }
    }
  }
  
  r->set_model(0L);
  r->resolve(0L);
}

void part_library::link_model(model *m)
{
  for (int i = 0; i < m->size(); ++i) {
    if (m->at(i)->get_type() == type_ref) {
      element_ref *r = CAST_AS_REF(m->at(i));
      if (link_element(r)) {
        if(r->get_model())
          link_model(r->get_model());
      }
    }
  }
}

} // ldraw
