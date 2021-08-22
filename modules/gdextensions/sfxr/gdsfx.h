
/* Custom importer for .sfx resources */

#ifndef RESOURCE_IMPORTER_SFX
#define RESOURCE_IMPORTER_SFX

#include "core/io/resource_importer.h"
#include "core/io/resource_saver.h"

class SFXData : public Resource {
	GDCLASS(SFXData, Resource);
	Variant data;

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_data", "data"), &SFXData::set_data);
		ClassDB::bind_method(D_METHOD("get_data"), &SFXData::get_data);
		ClassDB::bind_method(D_METHOD("from_file", "file"), &SFXData::from_file);

		ADD_PROPERTY(PropertyInfo(Variant::NIL, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_NIL_IS_VARIANT), "set_data", "get_data");
	}

public:
	Variant get_data() const {
		return data;
	}
	void set_data(Variant p_data) {
		data = p_data;
	}
	void from_file(const String p_file) {
	}
	SFXData() {}
	~SFXData() {}
};

class ResourceImporterSFX : public ResourceImporter {
	GDCLASS(ResourceImporterSFX, ResourceImporter);

public:
	virtual String get_importer_name() const;
	virtual String get_visible_name() const;
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual String get_save_extension() const;
	virtual String get_resource_type() const;

	virtual int get_preset_count() const;
	virtual String get_preset_name(int p_idx) const;

	virtual void get_import_options(List<ImportOption> *r_options, int p_preset = 0) const;
	virtual bool
	get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const;
	virtual Error import(const String &p_source_file, const String &p_save_path,
			const Map<StringName, Variant> &p_options,
			List<String> *r_platform_variants,
			List<String> *r_gen_files = nullptr,
			Variant *r_metadata = nullptr);

	ResourceImporterSFX() {}
	~ResourceImporterSFX() {}
};

#endif // RESOURCE_IMPORTER_SFX
