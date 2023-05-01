/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2008 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <array>
#include <map>
#include <set>

#include <assert.h>

#include "libldr/bfc.h"
#include "libldr/elements.h"
#include "libldr/model.h"
#include "libldr/utils.h"
#include "libldr/reader.h"

#ifdef LDR_ARCHIVE_SUPPORT
# include "ldrawlib/catalog.inl"
#define HANDLE_NOT_FOUND 0xffff
static ldcatalog::handle_t _ldraw_hnd_from_hash(const ldcatalog::hash_t search)
{
    static unsigned int middle = _ldraw_size/2;
    unsigned int first=0, last=_ldraw_size;
    while (first <= last) {
      if (_ldraw_hnd[middle].hash < search)
         first = middle + 1;
      else if (_ldraw_hnd[middle].hash == search) {
         return _ldraw_hnd[middle].handle;
         break;
      } else
         last = middle - 1;
      middle = (first + last)/2;
    }
    if (first > last)
        std::cerr << "Hash " << search << " not found!" << std::endl;
    return HANDLE_NOT_FOUND;
}
#endif

namespace ldraw
{

// cached entries:
std::map<std::string, item_refcount*> *reader::m_global_cache;

reader::reader()
{
}

reader::reader(const std::string &basepath)
{
    m_basepath = basepath;
    if (!basepath.empty() && *(m_basepath.rbegin()) != '/')
        m_basepath += "/";
}

model_multipart* reader::load_from_file(const std::string &path, const std::string &name) const
{
  std::ifstream file;
  file.open((path).c_str(), std::ios::in);
  if (!file.is_open())
    throw exception(__func__, exception::user_error, std::string("Could not open file for reading: ") + name);
  
  model_multipart *model = load_from_stream(file, name);
  
  file.close();
  
  return model;
}

item_refcount* reader::load_with_cache(const std::string &path, const std::string &name, const model::model_type mtype) const
{
  // check the global cache first
  if (m_global_cache) {
    auto it1 = (*m_global_cache).find(name);
    if (it1 != (*m_global_cache).end()) {
      return (*it1).second;
    }
  }

  std::ifstream file;
  file.open(path.c_str(), std::ios::in);
  if (!file.is_open())
    throw exception(__func__, exception::user_error, std::string("Could not open file for reading: ") + name);

#ifdef LDR_ARCHIVE_CREATE
    if (archive *bb = archiver())
        if (mtype==model::part || mtype==model::part)
            (*bb)
                .line_info(0)
                .write_bits(0, format::ID_LINE_TYPE)
                .write_bits(mtype==model::part?format::MetaCmd_Model_Part:format::MetaCmd_Model_Prim, format::ID_META_CMD);
#endif
  model_multipart *model = load_from_stream(file, name);

  file.close();

  item_refcount *n = new item_refcount(model);
  if (m_global_cache) {
    (*m_global_cache)[name] = n;
    (*m_global_cache)[name]->acquire();
  }

  return n;
}

model_multipart* reader::load_from_file(const std::string &name) const
{
  std::ifstream file;
  file.open((m_basepath + name).c_str(), std::ios::in);
  if (!file.is_open())
    throw exception(__func__, exception::user_error, std::string("Could not open file for reading: ") + name);

  model_multipart *model = load_from_stream(file, name);

  file.close();

  return model;
}

item_refcount* reader::load_with_cache(const std::string &name) const
{
  // check the global cache first
  if (m_global_cache) {
    auto it1 = (*m_global_cache).find(name);
    if (it1 != (*m_global_cache).end()) {
      return (*it1).second;
    }
  }

  std::ifstream file;
  file.open((m_basepath + name).c_str(), std::ios::in);
  if (!file.is_open())
    throw exception(__func__, exception::user_error, std::string("Could not open file for reading: ") + name);

  model_multipart *model = load_from_stream(file, name);

  file.close();

  item_refcount *n = new item_refcount(model);
  if (m_global_cache) {
    (*m_global_cache)[name] = n;
    (*m_global_cache)[name]->acquire();
  }

  return n;
}

model_multipart* reader::load_from_stream(std::istream &stream, std::string name)
{
  model_multipart *nm = new model_multipart;

  auto build_dependencies = [](archive *bb, std::string name, model *m){
    if (!bb) return;
    for (int i = 0; i < m->size(); ++i) {
      if (m->at(i)->get_type() == type_ref) {
        element_ref *r = CAST_AS_REF(m->at(i));
        (*bb).catalog(utils::translate_string(name), utils::translate_string(r->filename()));
      }
    }
  };

  // Search for submodel names
  std::set<std::string> m_submodel_names;
  std::string line;
  while (!stream.eof()) {
    getline(stream, line);
    line = utils::trim_string(line);
    if (line.length() > 7 && line.substr(0, 6) == "0 FILE")
      m_submodel_names.insert( utils::translate_string(line.substr(7, line.length() - 7)) );
  }

  std::string keyname;
  
  // Rewind the stream to the beginning
  stream.clear();
  stream.seekg(0, std::ios::beg);
  
  archive *bb = archiver();

  if (bb)
    bb->catalog(utils::translate_string(name), bb->get_bit_pos());
  bool tmp = parse_stream(nm->main_model(), stream, true, &keyname);
  if (nm->main_model()->name().empty()) {
    std::string nfp;
    size_t o = name.find_last_of("/");
    if (o == std::string::npos)
      nfp = name;
    else
      nfp = name.substr(o + 1, name.length() - o);
    
    nm->main_model()->set_name(nfp);
  }

  build_dependencies(bb, name, nm->main_model());

  if (!tmp)
    return nm;
  
  bool loop = true;
  while (loop) {
    std::string fn;
    model *m = new model;
    
    m->set_parent(nm);
    m->set_modeltype(model::submodel);
    loop = parse_stream(m, stream, true, &fn);
    m->set_name(keyname);
    
    if (bb)
        bb->catalog(utils::translate_string(keyname.c_str()), bb->get_bit_pos());

    nm->insert_submodel(m, keyname);

    keyname = fn;
  }
  
  nm->link_submodels();

  if (bb)
      for(auto &sm : nm->submodel_list())
          build_dependencies(bb, sm.first, sm.second);

  if (utils::cyclic_reference_test(nm->main_model()))
    throw exception(__func__, exception::fatal, "Cyclic reference detected. This model file may be corrupted.");
  for(auto &sm : nm->submodel_list())
    if (utils::cyclic_reference_test(sm.second))
      throw exception(__func__, exception::fatal, "Cyclic reference detected. This model file may be corrupted.");
  
  return nm;
}

#ifdef LDR_ARCHIVE_SUPPORT
item_refcount* reader::load_from_archive(const ldcatalog::handle_t &h) const
{
    return load_from_archive(_ldraw_cat[h].file);
}
item_refcount* reader::load_from_archive(const std::string &name) const
{
    // check the global cache first
    if (m_global_cache) {
        auto it1 = (*m_global_cache).find(name);
        if (it1 != (*m_global_cache).end()) {
            return (*it1).second;
        }
    }

    model_multipart *mm = 0;

    if (ldrawlibData == 0 || ldrawlibSize <= 0) return 0;

    static archive_ro bb(ldrawlibData, ldrawlibSize);

    archive::Hash h(utils::translate_string(name).c_str());

    ldcatalog::handle_t hnd = _ldraw_hnd_from_hash(h);
    if (hnd == HANDLE_NOT_FOUND) return 0;

    // found in archive
    off_t offs = _ldraw_cat[hnd].off;
    bb.set_bit_pos(offs);

#ifdef LDR_TESTER
    std::vector<ldcatalog::handle_t> deps = get_archive_deps(name);
#endif

    mm = new model_multipart;
    model *m = mm->main_model();

    using namespace format;

    off_t pop_offs = -1;
    bool done = false; while(!done)
    {
        element_base *base = 0;
        int line_type = bb.read_bits(ID_LINE_TYPE);
        if (line_type == 7) {
            const off_t offs = bb.read_bits(ID_OFFSET);
            pop_offs = bb.get_bit_pos();
            bb.set_bit_pos(offs);
            continue;
        }
        switch(line_type)
        {
            case 0: {
                int cmd = bb.read_bits(ID_META_CMD);
                if (cmd == MetaCmd_Lib_End || cmd == MetaCmd_Model_End)
                    done = true;
                else {
                    int cert = -1, winding = -1;
                    switch(cmd) {
                        case MetaCmd_Bfc_ccw: base = new element_bfc(element_bfc::ccw); break;
                        case MetaCmd_Bfc_cw: base = new element_bfc(element_bfc::cw); break;
                        case MetaCmd_Bfc_clip: base = new element_bfc(element_bfc::clip); break;
                        case MetaCmd_Bfc_clip_cw: base = new element_bfc(element_bfc::clip_cw); break;
                        case MetaCmd_Bfc_clip_ccw: base = new element_bfc(element_bfc::clip_ccw); break;
                        case MetaCmd_Bfc_noclip: base = new element_bfc(element_bfc::noclip); break;
                        case MetaCmd_Bfc_invertnext: base = new element_bfc(element_bfc::invertnext); break;
                        case MetaCmd_Bfc_uncert:
                            cert = bfc_certification::uncertified; break;
                        case MetaCmd_Bfc_cert_cw:
                            cert = bfc_certification::certified, winding = bfc_certification::cw; break;
                        case MetaCmd_Bfc_cert_ccw:
                            cert = bfc_certification::certified, winding = bfc_certification::ccw; break;
                    }
                    if (m && cert != -1) {
                        bfc_certification *c = m->init_custom_data<bfc_certification>();
                        c->set_certification((bfc_certification::cert_status)cert);
                        if (winding != -1)
                            c->set_orientation((bfc_certification::winding)winding);
                    }
                }
            }; break;
            case 1:
            case 6: {
                unsigned int col, hn;
                float x, y, z, a=1, b=0, c=0, d=0, e=1, f=0, g=0, h=0, i=1;

                // identity: 1 0 0 0 1 0 0 0 1
                bb
                    .read_bits(ID_COLOR, col)
                    .read_vec(x, y, z);
                if( line_type == 1) {
                    bb
                        .read_vec(a, b, c) // full matrix
                        .read_vec(d, e, f)
                        .read_vec(g, h, i);
                }
                bb
                    .read_bits(ID_NAME_HASH, hn);
#ifdef DEBUG
                if (hn >= _ldraw_size)
                    std::cerr << "Index beyond size of the array: " << hn << " - " << name << std::endl;
#endif
#ifdef LDR_TESTER
                if(std::find(deps.begin(), deps.end(), hn)==deps.end())
                    std::cerr << "Dependacy mismatch: " << hn << " - " << name << std::endl;
#endif
                base = new element_ref(color::from_index(col), matrix(a, b, c, d, e, f, g, h, i, x, y, z), _ldraw_cat[hn].file);
            }; break;
            case 2: {
                // Line
                unsigned int col;
                float x1, y1, z1, x2, y2, z2;
                bb
                    .read_bits(ID_COLOR, col)
                    .read_vec(x1, y1, z1)
                    .read_vec(x2, y2, z2);
                base = new element_line(color::from_index(col), vector(x1, y1, z1), vector(x2, y2, z2));
            }; break;
            case 3: {
                // Triangle
                unsigned int col;
                float x1, y1, z1, x2, y2, z2, x3, y3, z3;
                bb
                    .read_bits(ID_COLOR, col)
                    .read_vec(x1, y1, z1)
                    .read_vec(x2, y2, z2)
                    .read_vec(x3, y3, z3);
                base = new element_triangle(color::from_index(col), vector(x1, y1, z1), vector(x2, y2, z2), vector(x3, y3, z3));
            }; break;
            case 4: {
                // Quadrilateral
                unsigned int col;
                float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
                bb
                    .read_bits(ID_COLOR, col)
                    .read_vec(x1, y1, z1)
                    .read_vec(x2, y2, z2)
                    .read_vec(x3, y3, z3)
                    .read_vec(x4, y4, z4);
                base = new element_quadrilateral(color::from_index(col), vector(x1, y1, z1), vector(x2, y2, z2), vector(x3, y3, z3), vector(x4, y4, z4));
            }; break;
            case 5: {
                // Conditional line
                unsigned int col;
                float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
                bb
                    .read_bits(ID_COLOR, col)
                    .read_vec(x1, y1, z1)
                    .read_vec(x2, y2, z2)
                    .read_vec(x3, y3, z3)
                    .read_vec(x4, y4, z4);
                base = new element_condline(color::from_index(col), vector(x1, y1, z1), vector(x2, y2, z2), vector(x3, y3, z3), vector(x4, y4, z4));
            }; break;
            default:
                return 0; // wrong data;
        } // switch
        
        if(pop_offs >= 0) {
            bb.set_bit_pos(pop_offs);
            pop_offs = -1;
        }

        if (base)
            m->insert_element(base);
    }
    mm->insert_submodel(m, name);

  item_refcount *n = new item_refcount(mm);
  if (m_global_cache) {
    (*m_global_cache)[name] = n;
    (*m_global_cache)[name]->acquire();
  }

  return n;
}
#endif

bool reader::parse_stream(model *m, std::istream &stream, bool multipart, std::string *keyname)
{
  std::string line;
  int lines = 0;
  int zerocnt = 0;
  bool founddesc = false;
  bool foundheader;
  
  while(!stream.eof()) {
    getline(stream, line);
      line = utils::trim_string(line);
    long llen = line.length();
    ++lines;
    if (llen == 0)
      continue;
    // Returns true when more subpart(s) available. returns false otherwise.
    if (multipart && llen > 7 && line.substr(0, 6) == "0 FILE") {
      if (keyname)
        *keyname = line.substr(7, llen - 7);
      if (lines != 1) {
        stream.seekg((long)stream.tellg() - llen - 1);
        return true;
      }
    }
    
    foundheader = false;
    
    if (line[0] == '0') {
      // parse line type 0
      foundheader = true;
      ++zerocnt;
      
      std::string cont = utils::trim_string(line.substr(1, line.length()-1));
      std::string contlc = utils::translate_string(cont);
      
      // Parse header data
      if (contlc.length() > 4 && contlc.substr(0, 4) == "file"); // Skip
      else if (contlc.length() > 6 && contlc.substr(0, 5) == "name:") { // Filename
        m->set_name(cont.substr(6, cont.length() - 6));
      } else if (contlc.length() > 5 && contlc.substr(0, 4) == "name") { // Filename without ':'
        m->set_name(cont.substr(5, cont.length() - 5));
      } else if (contlc.length() > 8 && contlc.substr(0, 7) == "author:") { // Author
        m->set_author(cont.substr(8, cont.length() - 8));
      } else if (contlc.length() > 7 && contlc.substr(0, 6) == "author") { // Author without ':'
        m->set_author(cont.substr(7, cont.length() - 7));
      } else if (zerocnt < 3 && !founddesc) { // Partname
        m->set_desc(cont);
        founddesc = true;
      } else {
        foundheader = false;
      }
    }
    
    if (!foundheader) {
      element_base *el = parse_line(line, m);
      if (el)
        m->insert_element(el);
    }
  }
#ifdef LDR_ARCHIVE_CREATE
  if(archive *bb = archiver())
    (*bb)
        .line_info(0)
        .write_bits(0, format::ID_LINE_TYPE).profile("ID_LINE_TYPE", 0)
        .write_bits(format::MetaCmd_Model_End, format::ID_META_CMD); // end ot the model
#endif
  
  return false;
}

element_base* reader::parse_line(const std::string &command, model *m)
{
  std::string line = utils::trim_string(command);
  if (line.length() == 0)
    return 0;
  
  if (line[0] != '0') {
      size_t str_index; if((str_index = line.find(" 0x")) != std::string::npos) {
        // convert hex substring with decimal value:
        size_t spc_index = line.find(" ", str_index+1); if(spc_index != std::string::npos) {
            std::string cval = line.substr(str_index+3, spc_index-str_index-3);
            std::string dval = std::to_string(std::stol(cval, nullptr, 16));
            line.erase(str_index+1, spc_index-str_index-1);
            line.insert(str_index+1, dval);
        }
      }
  }

#ifdef LDR_ARCHIVE_CREATE
  archive *bb = archiver();
#endif

#define WriteMeta(cmd)                                              \
    if (bb)                                                         \
    {                                                               \
        using namespace format;                                     \
        (*bb)                                                       \
            .line_info(0)                                     \
            .write_bits(0, ID_LINE_TYPE).profile("ID_LINE_TYPE", 0) \
            .write_bits(cmd, ID_META_CMD);                          \
    }

  if (line[0] == '0') {
    // parse line type 0
    
    std::string cont = utils::trim_string(line.substr(1, line.length()-1));
    std::string contlc = utils::translate_string(cont);
    if (cont.length() == 0)
      return 0;
    
    if (cont[0] == '!') {
      // header data
      size_t pos = cont.find_first_of(" ");
      if (m && pos != std::string::npos)
        m->set_header(cont.substr(1, pos - 1), cont.substr(pos + 1));
    } else if (contlc == "step") {
      return new element_state(element_state::state_step);
    } else if (contlc == "pause") {
      return new element_state(element_state::state_pause);
    } else if (contlc == "clear") {
      return new element_state(element_state::state_clear);
    } else if (contlc == "save") {
      return new element_state(element_state::state_save);
    } else if (contlc.length() > 6 && (contlc.substr(0, 5) == "print" || contlc.substr(0, 5) == "write")) {
      return new element_print(cont.substr(6, line.length()-6));
    } else if (contlc.length() > 3 && contlc.substr(0, 3) == "bfc") {
      // Handle BFC statements
      std::string subs = contlc.substr(4, line.length() - 4);
      int cert = -1, winding = -1;
      
      if (subs == "ccw") {
#ifdef LDR_ARCHIVE_CREATE
        WriteMeta(MetaCmd_Bfc_ccw);
#endif
        return new element_bfc(element_bfc::ccw);
      } else if (subs == "cw") {
#ifdef LDR_ARCHIVE_CREATE
        WriteMeta(MetaCmd_Bfc_cw);
#endif
        return new element_bfc(element_bfc::cw);
      } else if (subs == "clip") {
#ifdef LDR_ARCHIVE_CREATE
        WriteMeta(MetaCmd_Bfc_clip);
#endif
        return new element_bfc(element_bfc::clip);
      } else if (subs == "clip cw" || subs == "cw clip") {
#ifdef LDR_ARCHIVE_CREATE
        WriteMeta(MetaCmd_Bfc_clip_cw);
#endif
        return new element_bfc(element_bfc::clip_cw);
      } else if (subs == "clip ccw" || subs == "ccw clip") {
#ifdef LDR_ARCHIVE_CREATE
        WriteMeta(MetaCmd_Bfc_clip_ccw);
#endif
        return new element_bfc(element_bfc::clip_ccw);
      } else if (subs == "noclip") {
#ifdef LDR_ARCHIVE_CREATE
        WriteMeta(MetaCmd_Bfc_noclip);
#endif
        return new element_bfc(element_bfc::noclip);
      } else if (subs == "invertnext") {
#ifdef LDR_ARCHIVE_CREATE
        WriteMeta(MetaCmd_Bfc_invertnext);
#endif
        return new element_bfc(element_bfc::invertnext);
      } else if (subs == "certify" || subs == "certify ccw") {
#ifdef LDR_ARCHIVE_CREATE
        WriteMeta(MetaCmd_Bfc_cert_ccw);
#endif
        cert = bfc_certification::certified, winding = bfc_certification::ccw;
      } else if (subs == "certify cw") {
#ifdef LDR_ARCHIVE_CREATE
        WriteMeta(MetaCmd_Bfc_cert_cw);
#endif
        cert = bfc_certification::certified, winding = bfc_certification::cw;
      } else if (subs == "nocertify") {
#ifdef LDR_ARCHIVE_CREATE
        WriteMeta(MetaCmd_Bfc_uncert);
#endif
        cert = bfc_certification::uncertified;
      }
      if (m && cert != -1) {
        bfc_certification *c = m->init_custom_data<bfc_certification>();
        c->set_certification((bfc_certification::cert_status)cert);
        if (winding != -1)
          c->set_orientation((bfc_certification::winding)winding);
      }
      return 0L;
    } else if (contlc.length() > 0) {
      return new element_comment(cont);
    }
  } else if (line[0] == '1') {
    unsigned int col;
    float x=0, y=0, z=0, a=0, b=0, c=0, d=0, e=0, f=0, g=0, h=0, i=0;
#if OLD_LINE1_PARSER
    std::istringstream s(line.substr(2, line.length()-2));
    char fnbuf[255];

    s >> col >> x >> y >> z >> a >> b >> c >> d >> e >> f >> g >> h >> i;
    s.getline(fnbuf, 255);
    std::string fn = utils::trim_string(std::string(fnbuf));
#else
   const auto &sp = utils::split_string(line.substr(2, line.length()-2));

   std::string fn = sp.empty()?"":utils::trim_string(std::string(sp.back()));  // last part is model name
   // process remaining part, that might be shorter than expected
   std::istringstream s(line.substr(2, line.length()-2-fn.length()));
   s >> col >> x >> y >> z >> a >> b >> c >> d >> e >> f >> g >> h >> i;
#endif

#ifdef LDR_ARCHIVE_CREATE
    if (bb)
    {
        using namespace format;
        const bool ident = (a==1 && b==0 && c==0 && d==0 && e==1 && f==0 && g==0 && h==0 && i==1);
        // identity: 1 0 0 0 1 0 0 0 1
        color cl(col);
        (*bb)
            .line_info(ident?6:1)
            .write_bits(ident?6:1, ID_LINE_TYPE).profile("ID_LINE_TYPE", ident?6:1)
            .write_bits(cl.get_index(), ID_COLOR).stat("COLOR", cl.get_id())
            .write_vec(x, y, z);
        if (ident)
            (*bb)
                .profile("ID_MATRIX_TYPE", 0); // identity
        else
            (*bb)
                .profile("ID_MATRIX_TYPE", 1) // full matrix
                .write_vec(a, b, c, "FLOAT-MAT")
                .write_vec(d, e, f, "FLOAT-MAT")
                .write_vec(g, h, i, "FLOAT-MAT");
        (*bb)
            .write_bits(bb->catalog(utils::translate_string(fn).c_str()), ID_NAME_HASH);
    }
#endif

    return new element_ref(color(col), matrix(a, b, c, d, e, f, g, h, i, x, y, z), fn);
  } else if (line[0] == '2') {
    // Line
    std::istringstream s(line.substr(2, line.length()-2));
    unsigned int col;
    float x1=0, y1=0, z1=0, x2=0, y2=0, z2=0;
    
    s >> col >> x1 >> y1 >> z1 >> x2 >> y2 >> z2;

#ifdef LDR_ARCHIVE_CREATE
    if (bb)
    {
        using namespace format;
        color cl(col);
        (*bb)
            .line_info(2)
            .write_bits(2, ID_LINE_TYPE).profile("ID_LINE_TYPE", 2)
            .write_bits(cl.get_index(), ID_COLOR).stat("COLOR", cl.get_id())
            .write_vec(x1, y1, z1)
            .write_vec(x2, y2, z2);

        // colinear/coplanar
        if (x1==x2 && y1==y2) (*bb).profile("SAME-XY", 1);
        else if (x1==x2 && z1==z2) (*bb).profile("SAME-XZ", 1);
        else if (y1==y2 && z1==z2) (*bb).profile("SAME-YZ", 1);
        else if (x1==x2) (*bb).profile("SAME-X", 1);
        else if (y1==y2) (*bb).profile("SAME-Y", 1);
        else if (z1==z2) (*bb).profile("SAME-Z", 1);
        else (*bb).profile("SAME-?", 1);
    }
#endif

    return new element_line(color(col), vector(x1, y1, z1), vector(x2, y2, z2));
  } else if (line[0] == '3') {
    // Triangle
    std::istringstream s(line.substr(2, line.length()-2));
    unsigned int col;
    float x1=0, y1=0, z1=0, x2=0, y2=0, z2=0, x3=0, y3=0, z3=0;
    
    s >> col >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> x3 >> y3 >> z3;

#ifdef LDR_ARCHIVE_CREATE
    if (bb)
    {
        using namespace format;
        color cl(col);
        (*bb)
            .line_info(3)
            .write_bits(3, ID_LINE_TYPE).profile("ID_LINE_TYPE", 3)
            .write_bits(cl.get_index(), ID_COLOR).stat("COLOR", cl.get_id())
            .write_vec(x1, y1, z1)
            .write_vec(x2, y2, z2)
            .write_vec(x3, y3, z3);

        // colinear/coplanar
        if (x1==x2 && x2==x3 && y1==y2 && y2==y3) (*bb).profile("SAME-XY", 2);
        else if (x1==x2 && x2==x3 && z1==z2 && z2==z3) (*bb).profile("SAME-XZ", 2);
        else if (y1==y2 && y2==y3 && z1==z2 && z2==z3) (*bb).profile("SAME-YZ", 2);
        else if (x1==x2 && x2==x3) (*bb).profile("SAME-X", 2);
        else if (y1==y2 && y2==y3) (*bb).profile("SAME-Y", 2);
        else if (z1==z2 && z2==z3) (*bb).profile("SAME-Z", 2);
        else (*bb).profile("SAME-?", 2);
    }
#endif

    return new element_triangle(color(col), vector(x1, y1, z1), vector(x2, y2, z2), vector(x3, y3, z3));
  } else if (line[0] == '4') {
    // Quadrilateral
    std::istringstream s(line.substr(2, line.length()-2));
    unsigned int col;
    float x1=0, y1=0, z1=0, x2=0, y2=0, z2=0, x3=0, y3=0, z3=0, x4=0, y4=0, z4=0;
    
    s >> col >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> x3 >> y3 >> z3 >> x4 >> y4 >> z4;
    
#ifdef LDR_ARCHIVE_CREATE
    if (bb)
    {
        using namespace format;
        color cl(col);
        (*bb)
            .line_info(4)
            .write_bits(4, ID_LINE_TYPE).profile("ID_LINE_TYPE", 4)
            .write_bits(cl.get_index(), ID_COLOR).stat("COLOR", cl.get_id())
            .write_vec(x1, y1, z1)
            .write_vec(x2, y2, z2)
            .write_vec(x3, y3, z3)
            .write_vec(x4, y4, z4);

        // colinear/coplanar
        if (x1==x2 && x2==x3 && x3==x4 && y1==y2 && y2==y3 && y3==y4) (*bb).profile("SAME-XY", 3);
        else if (x1==x2 && x2==x3 && x3==x4 && z1==z2 && z2==z3 && z3==z4) (*bb).profile("SAME-XZ", 3);
        else if (y1==y2 && y2==y3 && y3==y4 && z1==z2 && z2==z3 && z3==z4) (*bb).profile("SAME-YZ", 3);
        else if (x1==x2 && x2==x3 && x3==x4) (*bb).profile("SAME-X", 3);
        else if (y1==y2 && y2==y3 && y3==y4) (*bb).profile("SAME-Y", 3);
        else if (z1==z2 && z2==z3 && z3==z4) (*bb).profile("SAME-Z", 3);
        else (*bb).profile("SAME-?", 3);
    }
#endif

    return new element_quadrilateral(color(col), vector(x1, y1, z1), vector(x2, y2, z2), vector(x3, y3, z3), vector(x4, y4, z4));
  } else if (line[0] == '5') {
    // Conditional line
    std::istringstream s(line.substr(2, line.length()-2));
    unsigned int col;
    float x1=0, y1=0, z1=0, x2=0, y2=0, z2=0, x3=0, y3=0, z3=0, x4=0, y4=0, z4=0;
    
    s >> col >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> x3 >> y3 >> z3 >> x4 >> y4 >> z4;
    
#ifdef LDR_ARCHIVE_CREATE
    if (bb)
    {
        using namespace format;
        color cl(col);
        (*bb)
            .line_info(5)
            .write_bits(5, ID_LINE_TYPE).profile("ID_LINE_TYPE", 5)
            .write_bits(cl.get_index(), ID_COLOR).stat("COLOR", cl.get_id())
            .write_vec(x1, y1, z1)
            .write_vec(x2, y2, z2)
            .write_vec(x3, y3, z3)
            .write_vec(x4, y4, z4);
    }
#endif

    return new element_condline(color(col), vector(x1, y1, z1), vector(x2, y2, z2), vector(x3, y3, z3), vector(x4, y4, z4));
  }
  
  return 0L;
}

#ifdef LDR_ARCHIVE_SUPPORT
off_t reader::get_archive_offs(const std::string &n) const
{
    archive::Hash h(n.c_str());
    ldcatalog::handle_t hnd = _ldraw_hnd_from_hash(h);
    if (hnd != HANDLE_NOT_FOUND)
        return get_archive_offs(_ldraw_cat[hnd].off);
    std::cout << "[libLDR] offset not found for: " << n << std::endl;
    return 0;
}
off_t reader::get_archive_offs(const ldcatalog::handle_t &h) const
{
    return _ldraw_cat[h].off;
}

const std::vector<ldcatalog::handle_t> reader::get_archive_deps(const std::string &n) const
{
    static_assert(sizeof(wchar_t)==sizeof(ldcatalog::handle_t), "handle_t != wchar_t");
    static std::vector<ldcatalog::handle_t> _empty;
    for(auto &p : std::vector<std::string>{"","48/","8/"}) {
      archive::Hash h((p+n).c_str());
      ldcatalog::handle_t hnd = _ldraw_hnd_from_hash(h);
      if (hnd == HANDLE_NOT_FOUND) continue;
      if (const wchar_t *ws = _ldraw_cat[hnd].deps)
        return std::vector<ldcatalog::handle_t>(reinterpret_cast<const ldcatalog::handle_t*>(ws), reinterpret_cast<const ldcatalog::handle_t*>(ws+wcslen(ws)));
    }
#ifdef DEBUG
    std::cout << "[libLDR] dependency not found for: " << n << std::endl;
#endif
    return _empty;
}
const std::vector<ldcatalog::handle_t> reader::get_archive_deps(const ldcatalog::handle_t &h) const
{
    return get_archive_deps(_ldraw_cat[h].file);
}
const char *reader::get_archive_model(const ldcatalog::handle_t &h) const
{
    return _ldraw_cat[h].file;
}
ldcatalog::handle_t reader::get_archive_handle(const std::string &n) const
{
    archive::Hash h(n.c_str());
    ldcatalog::handle_t hnd = _ldraw_hnd_from_hash(h);
    if (hnd != HANDLE_NOT_FOUND) return hnd;
    std::cout << "[libLDR] handle not found for: " << n << std::endl;
    return 0;
}
bool reader::get_archive_exists(const std::string &n) const
{
    archive::Hash h(n.c_str());
    ldcatalog::handle_t hnd = _ldraw_hnd_from_hash(h);
    return hnd != HANDLE_NOT_FOUND;
}
#endif

}
