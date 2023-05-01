/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
 * To obtain more information about LDraw, visit http://www.ldraw.org.               *
 * Distributed in terms of the GNU Lesser General Public License v3                  *
 *                                                                                   *
 * Author: (c)2006-2008 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     */

#ifndef LIBLDR_READER_H
#define LIBLDR_READER_H

#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <vector>

#include "libldr/common.h"
#include "libldr/bit_buffer.h"
#include "libldr/utils.h"
#include "libldr/model.h"

#include "ldrawlib/ldraw_lib_mem.h"

#define x_ssprintf(...)                                 \
    ({ int _ss_size = snprintf(0, 0, ##__VA_ARGS__);    \
    char *_ss_ret = (char*)alloca(_ss_size+1);          \
    snprintf(_ss_ret, _ss_size+1, ##__VA_ARGS__);       \
    _ss_ret; })

namespace ldraw
{

/* Archive file format: number of bits per each field. */
namespace format {
	const int ID_LINE_TYPE  = 3;
	const int ID_MATRIX_TYPE = 1;
	const int ID_COLOR = 7;
	const int ID_FLOAT = 17;
	const int ID_FLOAT_OPT = 2;
	const int ID_ZERO_POINT = 1;
	const int ID_OFFSET = 30;
	const int ID_NAME_HASH = 14;
	const int ID_META_CMD = 4;

	// float compression:
	const float Nmin = -23973.3, Nmax = 10368;

	const int MetaCmd_Lib_End = 0;
	const int MetaCmd_Model_End = 1;
	const int MetaCmd_Model_Part = 2;
	const int MetaCmd_Model_Prim = 3;

	const int MetaCmd_Bfc_ccw = 6;
	const int MetaCmd_Bfc_cw = 7;
	const int MetaCmd_Bfc_clip = 8;
	const int MetaCmd_Bfc_clip_cw = 9;
	const int MetaCmd_Bfc_clip_ccw = 10;
	const int MetaCmd_Bfc_noclip = 11;
	const int MetaCmd_Bfc_invertnext = 12;
	const int MetaCmd_Bfc_uncert = 13;
	const int MetaCmd_Bfc_cert_cw = 14;
	const int MetaCmd_Bfc_cert_ccw = 15;

	const int Same_X = 0;
	const int Same_Y = 1;
	const int Same_Z = 2;
	const int Same_XY = 3;
	const int Same_XZ = 4;
	const int Same_YZ = 5;
};

// Reference:
// ----------
// https://www.unidata.ucar.edu/blogs/developer/en/entry/compression_by_bit_shaving
// https://www.unidata.ucar.edu/blogs/developer/entry/compression_by_scaling_and_offfset
// https://gist.github.com/rockdoc/ecac8b1c5f3f99079aac
// http://www.ctrl-alt-test.fr/2018/making-floating-point-numbers-smaller/
// http://www.iquilezles.org/www/articles/float4k/float4k.htm

/* roundb(f, 15) => keep 15 bits in the float, set the other bits to zero */
namespace round_bits
{

static inline float float_encode(float f, int bits)
{
		union { int i; float f; } num;
		/* assuming sizeof(int) == sizeof(float) == 4 */
		bits = 32 - bits;
		num.f = f;
		num.i = num.i + (1 << (bits - 1)); // round instead of truncate
		num.i = num.i & (-1 << bits);
		return num.f;
	}

} // round_bits

/* bit-shaving flat compression */
namespace bit_shave
{

static inline uint32_t float_encode(float value)
{
	if (isnan(value)) return value;
	int bitMask = 32 - format::ID_FLOAT;
	if (bitMask >= 23) return 0;

	union { float f; uint32_t u; } cf = { value };
	return cf.u >> bitMask;
}

static inline float float_decode(const uint32_t bits)
{
	int bitMask = 32 - format::ID_FLOAT;
	if (bitMask >= 23) return 0;
	uint32_t value = bits << bitMask; // expand data
	union { uint32_t u; float f; } ci = { value };
	return ci.f;
}

} // bit_shave

// scale-offset float number
namespace bit_scale_offset
{

static inline uint32_t float_encode(float f)
{
	using namespace format;
	static auto clip = [](const float n) { return std::max(Nmin, std::min(n, Nmax)); };
	f = clip(f);
	static const float offset = Nmin;
	static const float scale = (Nmax - Nmin) / (2^format::ID_FLOAT - 1);
	return round((f - offset) / scale);
}

static inline float float_decode(const uint32_t bits)
{
	using namespace format;
	static const float offset = Nmin;
	static const float scale = (Nmax - Nmin) / (2^format::ID_FLOAT - 1);
	return bits * scale + offset;
}

} // bit_scale_offset

class archive : public bit_buffer
{
	friend class archive_ro;

public:
		template <typename T> inline archive &write_bits(const T& data, const size_t num_bits)
		{
			bit_buffer::write_bits_(static_cast<uint32_t>(data), num_bits);
			return *this;
		}

#define compress_method bit_shave
		inline static uint32_t float_to_bits(float f)
		{
			return compress_method::float_encode(f);
		}

		inline static float float_from_bits(const uint32_t bits)
		{
			return compress_method::float_decode(bits);
		}

		archive &write_float(float f)
		{
			using namespace format;
			stat("FLOAT", f);
			const uint32_t r = float_to_bits(f);
			stat("FLOAT-BITS", r);
			return write_bits(r, format::ID_FLOAT);
		}

		archive &write_vec(float x, float y, float z, const std::string &cat = "FLOAT-POS")
		{
			stat(cat, x);
			stat(cat, y);
			stat(cat, z);
			return write_float(x).write_float(y).write_float(z);
		};

		inline float read_float(const size_t bit_index)
		{
			return float_from_bits(read_bits(bit_index, format::ID_FLOAT));
		}

		/* line debug info */
		std::vector<std::pair<uint8_t, size_t> > _line_info;
		archive &line_info(uint8_t line_type)
		{
			if (verbose_)
				std::cout << "line type " << _line_info.size() << ": " << int(line_type) << std::endl;
			_line_info.push_back(std::make_pair(line_type, get_bit_pos()));
			return *this;
		}

		/* simple profiling */
		std::map<std::string, std::map<int, int> > _profile;
		archive &profile(std::string key, int opt)
		{
			_profile[key][opt]++;
			return *this;
		}
		/* dictionary */
		struct key_t
		{
			enum { INT, FLT };
			union
			{
				int i;
				float f;
			} val;
			int dtc;
			key_t() : val({.i=0}), dtc(INT) {}
			key_t(int i) : val({.i=i}), dtc(INT) {}
			key_t(float f) : val({.f=f}), dtc(FLT) {}
			key_t(const key_t &v) : val(v.val), dtc(v.dtc) {}
			key_t& operator=(int i) {
				val.i = i;
				dtc = INT;
				return *this;
			}
			key_t& operator=(float f) {
				val.f = f;
				dtc = FLT;
				return *this;
			}
			bool operator<(key_t f) const {
				switch (dtc) {
					case INT: return val.i<f.val.i;
					case FLT: return val.f<f.val.f;
				}
				return 0;
			}
			operator int() const {
				switch (dtc) {
					case INT: return val.i;
					case FLT: return (int)val.f;
				}
				return 0;
			}
			operator float() const {
				switch (dtc) {
					case INT: return (float)val.i;
					case FLT: return val.f;
				}
				return 0;
			}
		};

		friend std::ostream & operator << (std::ostream &os, key_t k)
		{
			switch (k.dtc)
			{
				case archive::key_t::INT: os << int(k.val.i); break;
				case archive::key_t::FLT: os << float(k.val.f); break;
				default: os << "??"; break;
			}
			return os;
		}

		std::map<std::string, std::map<key_t, int>> _dict;
		inline archive &stat(std::string dict, int k) { _dict[dict][k]++; return *this; }
		inline archive &stat(std::string dict, unsigned int k) { _dict[dict][int(k)]++; return *this; }
		inline archive &stat(std::string dict, float k) { _dict[dict][k]++; return *this; }
		inline archive &dict(std::string dict, int k, int v ) { _dict[dict][k] = v; return *this; }
		inline archive &dict(std::string dict, float k, int v ) { _dict[dict][k] = v; return *this; }
		class Hash
		{
			static const unsigned int FNV_PRIME = 16777619u;
			static const unsigned int OFFSET_BASIS = 2166136261u;
			template <unsigned int N>
			static constexpr unsigned int hashConst(const char (&str)[N], unsigned int I = N)
			{
				return I == 1 ? (OFFSET_BASIS ^ str[0]) * FNV_PRIME : (hashConst(str, I - 1) ^ str[I - 1]) * FNV_PRIME;
			}
			static lcatalog::hash_t fnvHash(const char* str)
			{
				const size_t length = strlen(str) + 1;
				lcatalog::hash_t hash = OFFSET_BASIS;
				for (size_t i = 0; i < length; ++i)
				{
					hash ^= *str++;
					hash *= FNV_PRIME;
				}
				return hash;
			}
			struct Wrapper
			{
				Wrapper(const char* str) : str (str) { }
				const char* str;
			};
			lcatalog::hash_t hash_value;
		public:
			/* calulate in run-time */
			Hash(Wrapper wrapper) : hash_value(fnvHash(wrapper.str)) { }
			/* calulate in compile-time */
			template <unsigned int N> constexpr Hash(const char (&str)[N]) : hash_value(fnvHashConst(str)) { }
			// output result
			constexpr operator lcatalog::hash_t() const { return this->hash_value; }
		};
		/* elements catalog */
		const off_t PLACEHOLDER = 10e8;
		lcatalog::handle_t catalog(std::string nm)
		{
			Hash h(nm.c_str());
			if (!_hnd.count(h))
			{
				/* not found - add placeholder */
				_cat.push_back(PLACEHOLDER);
				return (_hnd[h] = std::make_pair(_cat.size()-1, nm)).first;
			} else
				return _hnd[h].first;
		}

		lcatalog::handle_t catalog(std::string nm, off_t pos)
		{
			Hash h(nm.c_str());
			if (_hnd.count(h) &&  _cat[_hnd[h].first] != PLACEHOLDER && _cat[_hnd[h].first] != pos)
			{
				std::cerr << "*** Duplicate found: " << nm << " (" << h << ")"
					<< " pos: " << _cat[_hnd[h].first] << "," << pos << std::endl;
				int dup = 2;
				while(_hnd.count(h)) {
					nm = nm + "." +  std::to_string(dup);
					h = Hash(nm.c_str());
					dup++;
				}
				std::cerr << "*** Rename to " << nm << std::endl;
			}
			if (_hnd.count(h) && _cat[_hnd[h].first] != PLACEHOLDER)
			{
				std::cerr << "*** Hash conflict: " << nm << " (" << h << ")" << std::endl;
				return 0;
			}
			else if (_hnd.count(h))
			{
				// update position only
				_cat[_hnd[h].first] = pos;
				return _hnd[h].first;
			} else {
				_cat.push_back(pos);
				return (_hnd[h] = std::make_pair(_cat.size()-1, nm)).first;
			}
		}

		void catalog(std::string nm, std::string dep)
		{
			std::vector<std::string> &deps = _deps[nm];
			if( std::find(deps.begin(), deps.end(), dep) == deps.end() )
				deps.push_back(dep);
		}

		std::map<lcatalog::hash_t, std::pair<lcatalog::handle_t, std::string> > _hnd; // hash -> name+handle
		std::vector<lcatalog::hash_t> _cat; // handle -> offset
		std::map<std::string, std::vector<std::string> > _deps;

	/* save at exit */
	~archive()
	{
#ifdef LDR_ARCHIVE_CREATE
		if (_profile.size())
		{
			for(const auto &opt : _profile) {
				std::cout << "Key " << opt.first << " :" << std::endl;
				for(const auto &el : opt.second)
					std::cout << "   opt " << el.first << " : " << el.second << std::endl;
			}
			std::cout << "Done ----" << std::endl;
		}
		std::cout << "Catalog with " << _cat.size() << "/" << _hnd.size() << " entries ----" << std::endl;
		if (_hnd.size())
		{
			std::ofstream fcat("catalog.inl", std::ios::out);
			fcat << "static const unsigned int _ldraw_size = " << _hnd.size() << ";" << std::endl;
			fcat << "static const struct{lcatalog::hash_t hash; lcatalog::handle_t handle;} _ldraw_hnd[] = {" << std::endl;
			unsigned int cnt = 0; for(const auto &el : _hnd)
			{
				fcat << " /* " << cnt << ". " << el.second.second << " */ {" << el.first << ", " << el.second.first << "}";
				if (++cnt == _hnd.size())
					fcat << std::endl;
				else
					fcat << "," << std::endl;
			}
			fcat << "};" << std::endl;
			fcat << "static const struct {off_t off; const char *file; const wchar_t *deps;} _ldraw_cat[] = {" << std::endl;
			cnt = 0; for(const auto &el : _cat)
			{
				fcat << " /* " << cnt << " */ { " << el << "";
				for(const auto &it : _hnd)
					if (it.second.first==cnt) {
						const std::string file = it.second.second;
						fcat << ", \"" << file << "\", L\"";
						for(const auto &dp : _deps[file]) {
							Hash h(dp.c_str());
							if (_hnd.count(h) == 0)
								std::cerr << "** Missing entry " << dp << std::endl;
							std::ios_base::fmtflags f( fcat.flags() );
							fcat << "\\x" << std::setfill('0') << std::setw(2) << std::hex << _hnd[h].first;
							fcat.flags( f );
						}
						fcat << "\" }";
					}
				if (el == PLACEHOLDER)
					std::cerr << "Incomplete catalog at index " << cnt << std::endl;
				if (++cnt == _cat.size())
					fcat << std::endl;
				else
					fcat << "," << std::endl;
			}
			fcat << "};" << std::endl;
			std::ofstream fdb("debug.inl", std::ios::out);
			fdb << "/**" << std::endl;
			cnt = 0; for(const auto &el : _line_info)
			{
				fdb << cnt++ << ": " << int(el.first) << "/" << el.second << std::endl;
			}
			fdb << "**/" << std::endl;
			fdb << "static std::vector<uint8_t> _line_info_type = {" << std::endl;
			cnt = 0; for(const auto &el : _line_info)
			{
				fdb << int(el.first);
				if (++cnt == _line_info.size() || cnt%30 == 0)
					if (cnt == _line_info.size())
						fdb << std::endl;
					else
						fdb << "," << std::endl;
				else
					fdb << ",";
			}
			fdb << "};" << std::endl;
			fdb << "static std::vector<size_t> _line_info_start = {" << std::endl;
			cnt = 0; for(const auto &el : _line_info)
			{
				fdb << el.second;
				if (++cnt == _line_info.size() || cnt%10 == 0)
					if (cnt == _line_info.size())
						fdb << std::endl;
					else
						fdb << "," << std::endl;
				else
					fdb << ",";
			}
			fdb << "};" << std::endl;
		}
		if (_dict.size()) {
			for(const auto &d : _dict) {
				std::vector<std::pair<key_t, int>> pairs;
				std::ofstream fdict("dict-"+d.first+".txt", std::ios::out | std::ofstream::binary);
				key_t kmin, kmax; for(const auto &e : d.second) {
					pairs.push_back(e);
					if (e.first < kmin) kmin = e.first;
					if (kmax < e.first) kmax = e.first;
					fdict << e.first << std::endl;
				}

				std::sort(pairs.begin(), pairs.end(), [=](std::pair<key_t, int>& a, std::pair<key_t, int>& b) {
					return a.second > b.second;
				});

				std::cout << d.first <<" - " << pairs.size() << " entries:" << std::endl;
				int c = 1, o = 0; for(const auto &el : pairs) {
					std::cout << c << ") " << el.first << ": " << el.second << std::endl;
					if (c++ == 5) break;
					o += el.second;
				}
				std::cout << "Done " << "(min: " << kmin << ", max:" << kmax << ", sum: " << o << ") ---- " << std::endl;
			}
		}
#endif
	}
};

class archive_ro : public bit_buffer_gz {
public:
	template<typename T> inline archive_ro &read_bits(const size_t num_bits, T& r)
	{
		r = bit_buffer_gz::read_bits(num_bits);
		return *this;
	}

	inline uint32_t read_bits(const size_t num_bits)
	{
		return bit_buffer_gz::read_bits(num_bits);
	}

	inline float read_float()
	{
		return archive::float_from_bits(read_bits(format::ID_FLOAT));
	}

	archive_ro &read_float(float &f)
	{
		f = read_float();
		return *this;
	}

	archive_ro &read_vec(float &x, float &y, float &z)
	{
		return read_float(x).read_float(y).read_float(z);
	};

	archive_ro(const unsigned char *pre_allocated, const size_t size) : bit_buffer_gz(pre_allocated, size) {}
};

class element_base;
class model;
class model_multipart;

class reader
{
	static bool parse_stream(model *m, std::istream &stream, bool multipart, std::string *keyname = 0L);
	static std::map<std::string, item_refcount*> *m_global_cache;
	std::string m_basepath;

public:
	model_multipart* load_from_file(const std::string &path, const std::string &name) const;
	item_refcount* load_with_cache(const std::string &path, const std::string &name, ldraw::model_type = ldraw::general) const;
	model_multipart* load_from_file(const std::string &name) const;
	item_refcount* load_with_cache(const std::string &name) const;
#ifdef LDR_ARCHIVE_SUPPORT
	item_refcount* load_from_archive(const lcatalog::handle_t &h) const;
	item_refcount* load_from_archive(const std::string &name) const;
#endif
	static model_multipart* load_from_stream(std::istream &stream, std::string name = "");
	static element_base* parse_line(const std::string &command, model *m = 0L);

	const std::string& basepath() const { return m_basepath; }

	/* gzip archive support: */
#ifdef LDR_ARCHIVE_SUPPORT
	off_t get_archive_offs(const std::string &n) const;
	off_t get_archive_offs(const lcatalog::handle_t &h) const;
	const std::vector<lcatalog::handle_t> get_archive_deps(const std::string &n) const;
	const std::vector<lcatalog::handle_t> get_archive_deps(const lcatalog::handle_t &h) const;
	const char *get_archive_model(const lcatalog::handle_t &h) const;
	lcatalog::handle_t get_archive_handle(const std::string &n) const;
	bool get_archive_exists(const std::string &n) const;
#endif

	static archive *archiver(archive *bb = nullptr)
	{
		static archive *s_bb = 0;
		if (bb)
			s_bb = bb;
		return s_bb;
	}

	static void enable_global_cache()
	{
		if (!m_global_cache)
			m_global_cache = new std::map<std::string, item_refcount*>();
	}

	static void disable_global_cache()
	{
		auto data = m_global_cache; m_global_cache = 0;
		if (data)
		{
			for( auto it=(*data).begin();it!=(*data).end();++it)
			{
				(*it).second->release();
				if (!(*it).second->refcount())
				{
					delete (*it).second;
					(*data).erase(it);
				}
			}
		}
	}

	reader();
	reader(const std::string &basepath);
};

} // ldraw

#endif // LIBLDR_READER_H
