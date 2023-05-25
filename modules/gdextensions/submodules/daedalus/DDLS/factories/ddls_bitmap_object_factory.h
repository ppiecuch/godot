#include "core/image.h"

#include "ddls_fwd.h"

class CanvasItem;

namespaces DDLSBitmapObjectFactory {
	DDLSObject build_from_bmp_data(Ref<Image> bmp_data, Ref<Image> p_debug_bmp = Ref<Image>(), CanvasItem *p_debug_shape = nullptr);
}
