/* libLDR: Portable and easy-to-use LDraw format abstraction & I/O reference library *
* To obtain more information about LDraw, visit http://www.ldraw.org.               *
* Distributed in terms of the GNU Lesser General Public License v3                  *
*                                                                                   *
* Author: (c)2006-2013 Park "segfault" J. K. <mastermind_at_planetmono_dot_org>     *
* Author: (c)2022-2023 Pawel Piecuch <piecuch_dot_pawel_at_gmail_dot_com>           */

#include <core/os/dir_access.h>
#include <core/os/file_access.h>

#include "libldr/part_library.h"
#include "libldr/utils.h"
#include "libldr/bit_buffer.h"
#include "libldr/reader.h"

namespace ldraw
{

static DirAccess *_opendir(const String &path) {
	DirAccess *da;
	if (!(da = DirAccess:: DirAccess::create_for_path(path))) {
		return nullptr;
	}
	if (da->list_dir_begin() != OK) {
		WARN_PRINT("Failed to list dir: " + path);
	}
	return da;
}

static std::string _readdir(DirAccessRef &da) {
	return da->get_next().utf8().c_str();
}
static void _closedir(DirAccessRef &da) {
	da->list_dir_end();
}

bool part_library::read_fs(const std::string &path)
{
	// TODO recursive subdirectory handling
	DirAccessRef de;
	std::string dn1, dn2;
	std::string pdir, partsdir;

	// 1. find subdirectories
	if (!(de = _opendir(path.c_str())))
		return false;

	std::string dn1;
	while ((dn1 = _readdir(de)) != "") {
		if (da->current_is_dir()) {
			dn2 = utils::translate_string(dn1.c_str());

			if (dn2 == "p") pdir = dn1;
			else if (dn2 == "parts") partsdir = dn1;
		}
	}
	_closedir(de);

	if (pdir.empty() || partsdir.empty()) {
		std::cerr << "[libLDR] No p/ or parts/ found." << std::endl;
		return false;
	}

	m_ldrawpath = path;
	m_primdir = pdir;
	m_partsdir = partsdir;

	// 2. look into p/ directory
	std::vector<std::string> merge_paths;
	if (Part_lib_quality == QualityLow) merge_paths.push_back(pdir+DIRECTORY_SEPARATOR+"8");
	else if (Part_lib_quality == QualityHigh) merge_paths.push_back(pdir+DIRECTORY_SEPARATOR+"48");
	merge_paths.push_back(pdir); // Default

	for(auto &p : merge_paths) {
		if (!(de = _opendir((m_ldrawpath + DIRECTORY_SEPARATOR + p).c_str()))) {
			std::cerr << "[libLDR] Couldn't open p/." << std::endl;
			return false;
		}
		std::string dn1;
		while ((dn1 = _readdir(de)) != "") {
			dn2 = utils::translate_string(dn1);

			if (dn2.length() > 4 && dn2.substr(dn1.length()-4, 4) == ".dat")
			if (!m_primlist.count(dn2)) // skip duplicates
				m_primlist[dn2] = dn1;
		}
		_closedir(de);
	}

	// 3. look into p/XX directory
	for(auto &p : std::vector<std::string>{"48","8"}) {
		if (!(de = _opendir((m_ldrawpath + DIRECTORY_SEPARATOR + pdir + DIRECTORY_SEPARATOR + p).c_str())))
			std::cerr << "[libLDR] Couldn't open p/" << p << "/" << std::endl;
		else {
			String dn1;
			while ((dn1 = _readdir(de)) != "") {
				dn1 = p + DIRECTORY_SEPARATOR + dn1;
				dn2 = utils::translate_string(dn1.utf8().c_str());

				if (dn2.length() > 4 && dn2.substr(dn2.length()-4, 4) == ".dat")
					m_primlist[dn2] = dn1;
			}
			_closedir(de);
		}
	}

	// 4. look into parts/ directory
	if (!(de = _opendir((m_ldrawpath + DIRECTORY_SEPARATOR + m_partsdir).c_str()))) {
		std::cerr << "[libLDR] Couldn't open parts/." << std::endl;
		return false;
	}
	std::string dn1;
	while ((dn1 = _readdir(de)) != "") {
		dn2 = utils::translate_string(dn1);
		if (dn2.length() > 4 && dn2.substr(dn2.length() - 4, 4) == ".dat")
		m_partlist[dn2] = dn1;
	}
	_closedir(de);

	// 6. look into parts/s directory
	std::string s;
	de = _opendir((m_ldrawpath + DIRECTORY_SEPARATOR + m_partsdir + DIRECTORY_SEPARATOR + "s").c_str());
	s = "s";

	if(!de) {
		de = _opendir((m_ldrawpath + DIRECTORY_SEPARATOR + m_partsdir + DIRECTORY_SEPARATOR + "S").c_str());
		s = "S";
	}

	if(!de)
		std::cerr << "[libLDR] Couldn't open parts/s/." << std::endl;
	else {
		std::string dn1;
		while ((dn1 = _readdir(de)) != "") {
			dn1 = s + DIRECTORY_SEPARATOR + dn1;
			dn2 = utils::translate_string(dn1);

			if (dn2.length() > 4 && dn2.substr(dn2.length() - 4, 4) == ".dat")
				m_partlist[dn2] = dn1;
		}
		_closedir(de);
	}

	return true;
}

} // ldraw

#endif // _MSC_VER
