#include "core/class_db.h"
#include "core/io/resource_loader.h"
#include "core/os/file_access.h"
#include "core/os/os.h"
#include "core/project_settings.h"
#include "scene/resources/texture.h"

#include "register_types.h"

#include "gddragonbones.h"

class ResourceFormatLoaderGDDragonBones : public ResourceFormatLoader {
public:
	virtual RES load(const String &p_path, const String &p_original_path = "", Error *p_err = nullptr, bool p_no_subresource_cache = false) {
		uint64_t tm_start = OS::get_singleton()->get_ticks_msec();

		GDDragonBones::GDDragonBonesResource *p_res = memnew(GDDragonBones::GDDragonBonesResource);
		Ref<GDDragonBones::GDDragonBonesResource> p_ref(p_res);

		String str_path_base = p_path.get_basename();
		str_path_base.erase(str_path_base.length() - strlen("_ske"), strlen("_ske"));

		p_ref->set_def_texture_path(str_path_base + "_tex.png");

		bool bret = p_ref->load_texture_atlas_data(String(str_path_base + "_tex.json").ascii().get_data());
		ERR_FAIL_COND_V(!bret, RES());

		bret = p_ref->load_bones_data(p_path.ascii().get_data());
		ERR_FAIL_COND_V(!bret, RES());

#ifdef TOOLS_ENABLED
		p_res->set_path(p_path, true);
#else
		p_res->set_path(p_path);
#endif

		uint64_t tm_finish = OS::get_singleton()->get_ticks_msec();
		print_verbose("DragonBones resource (" + p_path + ") loaded in " + itos(tm_finish - tm_start) + " ms");
		return p_ref;
	}

	virtual void get_recognized_extensions(List<String> *p_extensions) const {
		p_extensions->push_back("dbbin");
		p_extensions->push_back("json");
	}

	virtual bool handles_type(const String &p_type) const {
		return p_type == "GDDragonBonesResource";
	}

	virtual String get_resource_type(const String &p_path) const {
		String el = p_path.get_extension().to_lower();
		if ((el == "json" || el == "dbbin") && p_path.get_basename().to_lower().ends_with("_ske")) {
			return "GDDragonBonesResource";
		}
		return "";
	}
};

static Ref<ResourceFormatLoaderGDDragonBones> resource_loader_dragonbones;

void register_dragonbones_types() {
	ClassDB::register_class<GDDragonBones>();
	ClassDB::register_class<GDDragonBones::GDDragonBonesResource>();

	resource_loader_dragonbones.instance();
	ResourceLoader::add_resource_format_loader(resource_loader_dragonbones);
}

void unregister_dragonbones_types() {
	ResourceLoader::remove_resource_format_loader(resource_loader_dragonbones);
	resource_loader_dragonbones.unref();
}
