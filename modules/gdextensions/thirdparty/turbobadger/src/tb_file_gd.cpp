// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_system.h"

#include <string.h>
#include <vector>
#include <map>

#include "core/variant.h"
#include "core/os/file_access.h"

namespace tb {

class TBMemFile : public TBFile
{
	static std::map<const char *, _finfo_t> catalog;
public:
	static void register_file(const _finfo_t &file)
	{
#ifdef DEBUG_ENABLED
		ERR_FAIL_NULL(file.fname);
		ERR_FAIL_NULL(file.data);
		ERR_FAIL_COND(file.len <= 0);
		if (catalog.count(file.fname) > 1) {
			WARN_PRINT(vformat("%s already exists in the catalog", file.fname));
		}
#endif
		catalog[file.fname] = file;
	}

	static const _finfo_t *find(const char *fname) {
		if (catalog.count(fname) == 1) {
			return &catalog.at(fname);
		}
		return nullptr;
	}

	TBMemFile(const unsigned char *data, size_t len) : data(data), len(len), pos(0) {}
	virtual ~TBMemFile() {}

	virtual long Size()
	{
		return len;
	}
	virtual size_t Read(void *buf, size_t elemSize, size_t count)
	{
		if (pos >= len) {
			WARN_PRINT("Read beyond file size");
			return 0;
		}
		const size_t sz = MIN(len - pos, elemSize * count);
		memcpy(buf, data + pos, sz);
		pos += sz;
		return sz;
	}
private:
	const unsigned char *data;
	const size_t len;
	size_t pos;
};

void register_mem_files(const _finfo_t *files, int count)
{
	for (int i = 0; i < count; i++) {
		TBMemFile::register_file(files[i]);
	}
}

class TBGdFile : public TBFile
{
public:
	TBGdFile(FileAccessRef f) : file(f) {}
	virtual ~TBGdFile() { file->close(); }

	virtual long Size()
	{
		return file->get_len();
	}
	virtual size_t Read(void *buf, size_t elemSize, size_t count)
	{
		return file->get_buffer((uint8_t*)buf, elemSize * count);
	}
private:
	FileAccessRef file;
};

// static
TBFile *TBFile::Open(const char *filename, TBFileMode mode)
{
	static const std::vector<String> _search_path = {
		"",
		"res://assets/",
		"res://res/",
	};

	for (const auto &p : _search_path) {
		String path = p + filename;
		if (FileAccess::exists(path)) {
			FileAccessRef f = nullptr;
			switch (mode)
			{
			case MODE_READ:
				f = FileAccess::open(path, FileAccess::READ);
				break;
			default:
				break;
			}
			if (!f)
				return nullptr;
			return new TBGdFile(f);
		}
	}

	return nullptr;
}

} // namespace tb
