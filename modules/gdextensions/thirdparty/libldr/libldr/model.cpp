/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2008 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#include <algorithm>

#include "libldr/model.h"
#include "libldr/reader.h"
#include "libldr/utils.h"

namespace ldraw
{

model::model(const std::string &desc, const std::string &name, const std::string &author, model_multipart *parent)
    : m_desc(desc), m_name(name), m_author(author), m_null(false), m_parent(parent), m_model_type(general)
{
}

model::~model()
{
  clear();
  
  for (std::map<std::string, extension *>::iterator it = m_data.begin(); it != m_data.end(); ++it)
    delete (*it).second;
}

bool model::is_submodel_of(const model_multipart *m) const
{
  if (!m)
    return false;
  
  return m->contains(this);
}

std::list<std::string> model::header(const std::string &key) const
{
  std::list<std::string> l;
  
  std::multimap<std::string, std::string>::const_iterator it = m_headers.find(key);
  
  if (it == m_headers.end())
    return l;
  
  for (; it != m_headers.upper_bound(key); ++it)
    l.push_back((*it).second);
  
  return l;
}

int model::size() const
{
  return m_elements.size();
}

element_base* model::at(unsigned int index)
{
  if (index >= m_elements.size())
    return 0L;
  
  return m_elements[index];
}

element_base* model::operator[] (unsigned int index)
{
  if (index >= m_elements.size())
    return 0L;
  
  return m_elements[index];
}

void model::insert_element(element_base *e, int pos)
{
  if (e->get_type() == type_ref) {
    element_ref *ref = CAST_AS_REF(e);
    ref->set_parent(this);
    ref->link();
  }
  
  if (pos == -1)
    m_elements.push_back(e);
  else
    m_elements.insert(m_elements.begin() + pos, e);
}

bool model::delete_element(int pos)
{
  if (pos >= (int)m_elements.size())
    return false;
  
  if (pos == -1)
    pos = m_elements.size() - 1;
  
  delete m_elements[pos];
  m_elements.erase(m_elements.begin() + pos);
  
  return true;
}

void model::set_header(const std::string &key, const std::string &value)
{
  m_headers.insert(make_pair(key, value));
}

void model::remove_header(const std::string &key)
{
  m_headers.erase(key);
}

void model::clear()
{
  set_name("");
  set_desc("");
  set_author("");
  
  for (model::iterator it = m_elements.begin(); it != m_elements.end(); ++it)
    delete (*it);
  m_elements.clear();
  
  m_null = true;
}

/*void model::operator=(model &rhs)
{
  clear_all();
  
  m_null = rhs.is_null();
  m_desc = rhs.desc();
  m_name = rhs.name();
  m_author = rhs.author();
  m_parent = &rhs.parent();
  
  for (model::iterator it = rhs.begin(); it != rhs.end(); ++it)
    push_back(*it);
}*/

bool model_multipart::contains(const model *m) const
{
  std::string lowercase = utils::translate_string(m->name());
  
  if (m_submodel_list.find(lowercase) == m_submodel_list.end())
    if (m_external_model_list.find(lowercase) == m_external_model_list.end())
      return false;
  
  return true;
}

void model_multipart::link_submodels()
{
  link_submodel(&m_main_model);
  
  for (submodel_iterator it = m_submodel_list.begin(); it != m_submodel_list.end(); ++it)
    link_submodel((*it).second);
}

void model_multipart::link_submodel(model *m)
{
  for (int i = 0; i < m->size(); ++i) {
    if (m->at(i)->get_type() == type_ref)
      link_submodel_element(CAST_AS_REF(m->at(i)));
  }
}

bool model_multipart::link_submodel_element(element_ref *r)
{
  std::string fn = utils::translate_string(r->filename());
  
  model_multipart *p = 0L;
  
  if (r->parent())
    p = r->parent()->parent();
  
  if (p != this)
    return false;
  
  model *m = find_submodel(fn);
  if (m) {
    r->set_model(m);
    return true;
  }
  
  model_multipart *ext = find_external_model(r->filename());
  if (!ext)
    ext = load_external_model(reader(), r->filename());
  
  if (ext) {
    r->set_model(ext->main_model());
    return true;
  }
  
  // std::cerr << "link model failed: " << r->filename() << std::endl;

  return false;
}

model* model_multipart::find_submodel(const std::string &name)
{
  std::string lowercase = utils::translate_string(name);
  std::map<std::string, model*>::iterator it = m_submodel_list.find(lowercase);

  if(it == m_submodel_list.end())
    return 0L;
  else
    return (*it).second;
}

bool model_multipart::insert_submodel(model *m)
{
  return insert_submodel(m, m->name());
}

bool model_multipart::insert_submodel(model *m, const std::string &key)
{
  std::string fn = utils::translate_string(key);
  
  // Search for duplicate
  if(m_submodel_list.find(fn) != m_submodel_list.end())
    return false;

  m_submodel_list[fn] = m;
  
  return true;
}

bool model_multipart::remove_submodel(const std::string &name)
{
  std::string lowercase = utils::translate_string(name);
  std::map<std::string, model*>::iterator it = m_submodel_list.find(lowercase);
  
  if (it == m_submodel_list.end())
    return false;
  
  model *m = (*it).second;
  
  if (utils::affected_models(this, m).size() > 0)
    return false;
  
  delete m;
  m_submodel_list.erase(it);
  
  return true;
}

bool model_multipart::remove_submodel(ldraw::model *m)
{
  return remove_submodel(m->name());
}

bool model_multipart::rename_submodel(const std::string &name, const std::string &newname)
{
  model *m = find_submodel(name);
  
  if (!m)
    return false;
  
  // rename
  m_submodel_list.erase(utils::translate_string(name));
  m_submodel_list[utils::translate_string(newname)] = m;
  
  // search the main model
  for (int i = 0; i < m_main_model.size(); ++i) {
    if (m_main_model[i]->get_type() == type_ref && CAST_AS_REF(m_main_model[i])->get_model() == m)
      CAST_AS_REF(m_main_model[i])->set_filename(newname);
  }
  
  // search entire submodels
  for (std::map<std::string, model *>::iterator it = m_submodel_list.begin(); it != m_submodel_list.end(); ++it) {
    model *tm = (*it).second;
    for (int i = 0; i < (*it).second->size(); ++i) {
      if (tm->at(i)->get_type() == type_ref && CAST_AS_REF(tm->at(i))->get_model() == m)
        CAST_AS_REF(tm->at(i))->set_filename(newname);
    }
  }
  
  m->set_name(newname);
  
  return true;
}

model_multipart* model_multipart::find_external_model(const std::string &name)
{
  std::map<std::string, model_multipart*>::iterator it = m_external_model_list.find(utils::translate_string(name));
  
  if (it == m_external_model_list.end())
    return 0L;
  else
    return (*it).second;
}

model_multipart* model_multipart::load_external_model(const reader &r, const std::string &name)
{
  model_multipart *m;
  
  try {
    m = r.load_from_file(utils::translate_string(name));
  } catch (const exception &e) {
    return 0L;
  }
  
  remove_external_model(name);
  
  m_external_model_list[utils::translate_string(name)] = m;
  
  return m;
}

bool model_multipart::remove_external_model(const std::string &name)
{
  std::map<std::string, model_multipart*>::iterator it = m_external_model_list.find(utils::translate_string(name));
  
  if (it != m_external_model_list.end()) {
    delete (*it).second;
    m_external_model_list.erase(it);
    
    return true;
  }
  
  return false;
}

void model_multipart::clear()
{
  for (auto &m : m_submodel_list)
    m.second->clear(), delete m.second;
  
  for (auto &m : m_external_model_list)
    delete m.second;
  
  m_submodel_list.clear();
  m_external_model_list.clear();
  m_main_model.clear();
}

}
