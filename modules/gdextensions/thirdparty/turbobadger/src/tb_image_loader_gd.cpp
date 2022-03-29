// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_bitmap_fragment.h"
#include "tb_system.h"
#include "tb_tempbuffer.h"

#include "core/image.h"

namespace tb {

class GD_Loader : public TBImageLoader
{
	Ref<Image> image;

public:
	GD_Loader(Ref<Image> image) : image(image) {}

	virtual int Width() { return image->get_width(); }
	virtual int Height() { return image->get_height(); }
	virtual const uint32 *Data() const { return (uint32*)image->get_data().read().ptr(); }
};

TBImageLoader *TBImageLoader::CreateFromFile(const char *filename)
{
	TBTempBuffer buf;
	if (buf.AppendFile(filename))
	{
		Ref<Image> image = memnew(Image((uint8_t*)buf.GetData(), buf.GetAppendPos()));
		if (image.is_valid()) {
			return new GD_Loader(image);
		}
	}
	return nullptr;
}

} // namespace tb
