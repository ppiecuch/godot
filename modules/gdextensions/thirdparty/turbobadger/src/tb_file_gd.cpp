// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_system.h"

#include <stdio.h>
#include <vector>

#include "core/os/file_access.h"

namespace tb {

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
}

} // namespace tb
