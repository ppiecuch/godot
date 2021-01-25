#include "starfield_2d.h"
#include "starfield_res.h"
#include "starfield.h"
#include "inc/gd_pack.h"


void Starfield2D::_update() {
    update() ;
}


void Starfield2D::_notification(int p_what) {

    switch(p_what) {

        case NOTIFICATION_READY: {
            update() ;
        } break;

        case NOTIFICATION_PHYSICS_PROCESS: {
        } break ;

        case NOTIFICATION_DRAW: {
        } break ;
    }
}

void Starfield2D::_bind_methods() {

    ADD_PROPERTY( PropertyInfo(Variant::VECTOR2, "virtual_size"), "set_virtual_size", "get_virtual_size" );
    ADD_PROPERTY( PropertyInfo(Variant::VECTOR2, "movement"), "set_movement", "get_movement" );
}

Starfield2D::Starfield2D() {
	// build texture atlas from resources
	Vector<Ref<Image>> images;
	Vector<String> names;
	std::vector<EmbedImageItem> embed(embed_starfield_res, embed_starfield_res + embed_starfield_res_count);
	for (const auto &r : embed) {
		ERR_CONTINUE_MSG(r.channels < 3, "Format is not supported, Skipping!");
		Ref<Image> image;
		image.instance();
		PoolByteArray data;
		data.resize(r.size);
		std::memcpy(data.write().ptr(), r.pixels, r.size);
		image->create(r.width, r.height, false, r.channels == 4 ? Image::FORMAT_RGBA8 : Image::FORMAT_RGB8, data);
		images.push_back(image);
		names.push_back(r.image);
	}
	_image_atlas = merge_images(images, names);
	// prepare starfield
	_starfield = Ref<Starfield>(memnew(Starfield));
}

Starfield2D::~Starfield2D() {
}
