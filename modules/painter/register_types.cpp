#include "register_types.h"

#include "core/class_db.h"
#include "painter_2d.h"
#include "painter_brush.h"
#include "painter_path.h"
#include "painter_pen.h"

void register_painter_types() {
	ClassDB::register_class<PainterPen>();
	ClassDB::register_class<PainterBrush>();
	ClassDB::register_class<PainterPath>();
	ClassDB::register_class<Painter2D>();
}

void unregister_painter_types() {
}
