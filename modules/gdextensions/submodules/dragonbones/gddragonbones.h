#ifndef GDDRAGONBONES_H
#define GDDRAGONBONES_H

#include "scene/2d/node_2d.h"
#include "scene/resources/texture.h"
#include "src/GDFactory.h"

DRAGONBONES_USING_NAME_SPACE;

class GDDragonBones : public GDOwnerNode {
	GDCLASS(GDDragonBones, GDOwnerNode);

public:
	enum AnimMode {
		ANIMATION_PROCESS_FIXED,
		ANIMATION_PROCESS_IDLE,
	};

	enum AnimFadeOutMode {
		FadeOut_None,
		FadeOut_SameLayer,
		FadeOut_SameGroup,
		FadeOut_SameLayerAndGroup,
		FadeOut_All,
		FadeOut_Single
	};

	// Resource class
	class GDDragonBonesResource : public Resource {
		GDCLASS(GDDragonBonesResource, Resource);

	public:
		GDDragonBonesResource();
		~GDDragonBonesResource();

		void set_def_texture_path(const String &path);
		bool load_texture_atlas_data(const String &path);
		bool load_bones_data(const String &path);

		String str_default_tex_path;
		char *p_data_texture_atlas;
		char *p_data_bones;
	};

private:
	GDFactory *p_factory;
	Ref<Texture> m_texture_atlas;
	Ref<GDDragonBonesResource> m_res;
	String str_curr_anim;
	GDArmatureDisplay *p_armature;
	AnimMode m_anim_mode;
	float f_speed;
	float f_progress;
	int c_loop;
	bool b_processing;
	bool b_active;
	bool b_playing;
	bool b_debug;
	bool b_inited;
	bool b_try_playing;
	bool b_flip_x;
	bool b_flip_y;
	bool b_inherit_child_material;

protected:
	void _notification(int what);
	static void _bind_methods();

	bool _set(const StringName &name, const Variant &value);
	bool _get(const StringName &name, Variant &ret) const;
	void _get_property_list(List<PropertyInfo> *list) const;

public:
	GDDragonBones();
	~GDDragonBones();

	void _cleanup();
	void _reset();
	void _set_process(bool process, bool force = false);

	void dispatch_event(const String &type, const EventObject *value);
	void dispatch_snd_event(const String &type, const EventObject *value);

	void set_resource(Ref<GDDragonBonesResource> data);
	Ref<GDDragonBonesResource> get_resource();

	void set_inherit_material(bool enable);
	bool is_material_inherited() const;

	void fade_in(const String &anim_name, float time, int loop, int layer, const String &group, AnimFadeOutMode fade_out_mode);
	void fade_out(const String &anim_name);

	void set_active(bool active);
	bool is_active() const;

	void set_debug(bool debug);
	bool is_debug() const;

	void set_speed(float speed);
	float get_speed() const;

	void set_texture(const Ref<Texture> &texture);
	Ref<Texture> get_texture() const;

	String get_current_animation() const;

	float tell() const;
	void seek(float p);
	float get_progress() const;

	void set_animation_process_mode(AnimMode mode);
	AnimMode get_animation_process_mode() const;

	void play(bool play = true);
	void play_from_time(float time);
	void play_from_progress(float progress);

	void flip_x(bool flip);
	bool is_fliped_x() const;

	void flip_y(bool flip);
	bool is_fliped_y() const;

	bool is_playing() const;

	bool has_anim(const String &anim) const;
	void stop(bool all = false);
	inline void stop_all() { stop(true); }
};

VARIANT_ENUM_CAST(GDDragonBones::AnimMode);
VARIANT_ENUM_CAST(GDDragonBones::AnimFadeOutMode);

#endif // GDDRAGONBONES_H
