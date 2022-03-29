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
public:
	Ref<Image> image;

	GD_Loader() {}

	virtual int Width() { return image->get_width(); }
	virtual int Height() { return image->get_height(); }
	virtual const uint32 *Data() const { return (uint32*)image->get_data().read().ptr(); }
};

TBImageLoader *TBImageLoader::CreateFromFile(const char *filename)
{
	TBTempBuffer buf;
	if (buf.AppendFile(filename))
	{
		int w, h, comp;
		if (unsigned char *img_data = stbi_load_from_memory(
			(unsigned char*) buf.GetData(), buf.GetAppendPos(), &w, &h, &comp, 4))
		{
			if (GD_Loader *img = new GD_Loader())
			{
				img->width = w;
				img->height = h;
				img->data = img_data;
				return img;
			}
			else
				stbi_image_free(img_data);
		}
	}
	return nullptr;
}

} // namespace tb
