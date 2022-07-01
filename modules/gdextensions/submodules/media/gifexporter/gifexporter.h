#ifndef GIFEXPORTER_H
#define GIFEXPORTER_H

#include "core/reference.h"
#include "core/image.h"

#define GIF_FREE
#include <gifanimcplusplus/gifanim.h>

class GifExporter : public Reference {
    GDCLASS(GifExporter, Reference);

    GifAnim ganim;
    GifWriter gwriter;

protected:
    static void _bind_methods();

public:
    String filename;

    void set_filename(const String file);
    void begin_export(const Size2 &size, float frame_delay, int loop_count = 0, int32_t bit_depth = 8, bool dither = false);
    void end_export();
    void write_frame(const Ref<Image> frame, const Color &background_color, float frame_delay, int32_t bit_depth = 8, bool dither = false);

    GifExporter();
    ~GifExporter();
};

#endif //GIFEXPORTER_H
