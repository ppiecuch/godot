/**************************************************************************/
/*  gddragonbones.cpp                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "gddragonbones.h"

#include "core/io/resource_loader.h"
#include "core/method_bind_ext.gen.inc"
#include "core/os/file_access.h"
#include "core/os/os.h"

//////////////////////////////////////////////////////////////////
//// Resource

GDDragonBones::GDDragonBonesResource::GDDragonBonesResource() {
	p_data_texture_atlas = nullptr;
	p_data_bones = nullptr;
}

GDDragonBones::GDDragonBonesResource::~GDDragonBonesResource() {
	if (p_data_texture_atlas) {
		memfree(p_data_texture_atlas);
		p_data_texture_atlas = nullptr;
	}
	if (p_data_bones) {
		memfree(p_data_bones);
		p_data_bones = nullptr;
	}
}

static char *load_file(const String &file_path) {
	FileAccess *f = FileAccess::open(file_path, FileAccess::READ);
	ERR_FAIL_COND_V(!f, nullptr);
	ERR_FAIL_COND_V(!f->get_len(), nullptr);

	char *data = (char *)memalloc(f->get_len() + 1);
	ERR_FAIL_COND_V(!data, nullptr);

	f->get_buffer((uint8_t *)data, f->get_len());
	data[f->get_len()] = 0x00;

	memdelete(f);
	return data;
}

void GDDragonBones::GDDragonBonesResource::set_def_texture_path(const String &path) {
	str_default_tex_path = path;
}

bool GDDragonBones::GDDragonBonesResource::load_texture_atlas_data(const String &path) {
	p_data_texture_atlas = load_file(path);
	ERR_FAIL_COND_V(!p_data_texture_atlas, false);
	return true;
}

bool GDDragonBones::GDDragonBonesResource::load_bones_data(const String &path) {
	p_data_bones = load_file(path);
	ERR_FAIL_COND_V(!p_data_bones, false);
	return true;
}

/////////////////////////////////////////////////////////////////
//// Plugin module

GDDragonBones::GDDragonBones() {
	p_factory = memnew(GDFactory(this));

	m_res = RES();
	str_curr_anim = "[none]";
	p_armature = nullptr;
	m_anim_mode = ANIMATION_PROCESS_IDLE;
	f_progress = 0;
	f_speed = 1.f;
	b_processing = false;
	b_active = true;
	b_playing = false;
	b_debug = false;
	c_loop = -1;
	b_inited = false;
	b_try_playing = false;
	b_flip_x = false;
	b_flip_y = false;
	b_inherit_child_material = true;
}

GDDragonBones::~GDDragonBones() {
	_cleanup();
	if (p_factory) {
		memdelete(p_factory);
		p_factory = nullptr;
	}
}

void GDDragonBones::_cleanup() {
	b_inited = false;
	if (p_factory)
		p_factory->clear();
	if (p_armature) {
		if (p_armature->is_inside_tree())
			remove_child(p_armature);
		p_armature = nullptr;
	}
	m_res = RES();
}

void GDDragonBones::dispatch_snd_event(const String &type, const EventObject *value) {
	if (Engine::get_singleton()->is_editor_hint())
		return;
	if (type == EventObject::SOUND_EVENT)
		emit_signal("dragon_anim_snd_event", String(value->animationState->name.c_str()), String(value->name.c_str()));
}

void GDDragonBones::dispatch_event(const String &type, const EventObject *value) {
	if (Engine::get_singleton()->is_editor_hint())
		return;
	if (type == EventObject::START)
		emit_signal("dragon_anim_start", String(value->animationState->name.c_str()));
	else if (type == EventObject::LOOP_COMPLETE)
		emit_signal("dragon_anim_loop_complete", String(value->animationState->name.c_str()));
	else if (type == EventObject::COMPLETE)
		emit_signal("dragon_anim_complete", String(value->animationState->name.c_str()));
	else if (type == EventObject::FRAME_EVENT)
		emit_signal("dragon_anim_event", String(value->animationState->name.c_str()), String(value->name.c_str()));
	else if (type == EventObject::FADE_IN)
		emit_signal("dragon_fade_in", String(value->animationState->name.c_str()));
	else if (type == EventObject::FADE_IN_COMPLETE)
		emit_signal("dragon_fade_in_complete", String(value->animationState->name.c_str()));
	else if (type == EventObject::FADE_OUT)
		emit_signal("dragon_fade_out", String(value->animationState->name.c_str()));
	else if (type == EventObject::FADE_OUT_COMPLETE)
		emit_signal("dragon_fade_out_complete", String(value->animationState->name.c_str()));
}

void GDDragonBones::set_resource(Ref<GDDragonBones::GDDragonBonesResource> data) {
	String old_texture_path = "";
	if (m_res.is_valid())
		old_texture_path = m_res->str_default_tex_path;
	else if (data.is_valid())
		old_texture_path = data->str_default_tex_path;

	if (m_res == data)
		return;

	stop();
	_cleanup();

	m_res = data;
	if (m_res.is_null()) {
		m_texture_atlas = Ref<Texture>();
		ERR_PRINT("Null resources");
		_change_notify();
		return;
	}

	ERR_FAIL_COND(!m_res->p_data_texture_atlas);
	ERR_FAIL_COND(!m_res->p_data_bones);

	TextureAtlasData *p_tad = p_factory->loadTextureAtlasData(m_res->p_data_texture_atlas, nullptr);
	ERR_FAIL_COND(!p_tad);
	DragonBonesData *p_dbd = p_factory->loadDragonBonesData(m_res->p_data_bones);
	ERR_FAIL_COND(!p_dbd);

	const std::vector<std::string> &armature_names = p_dbd->getArmatureNames();
	ERR_FAIL_COND(!armature_names.size());

	p_armature = static_cast<GDArmatureDisplay *>(p_factory->buildArmatureDisplay(armature_names[0].c_str()));
	p_armature->p_owner = this;

	if (!m_texture_atlas.is_valid() || old_texture_path != m_res->str_default_tex_path)
		m_texture_atlas = ResourceLoader::load(m_res->str_default_tex_path);

	if (m_texture_atlas.is_valid()) {
		p_tad->height = m_texture_atlas->get_height();
		p_tad->width = m_texture_atlas->get_width();
	}

	p_armature->add_parent_class(b_debug, m_texture_atlas);
	add_child(p_armature);

	b_inited = true;

	p_armature->update_childs(true, true);
	p_armature->update_material_inheritance(b_inherit_child_material);
	p_armature->getArmature()->setFlipX(b_flip_x);
	p_armature->getArmature()->setFlipY(b_flip_y);
	p_armature->getArmature()->advanceTime(0);

	_change_notify();
	update();
}

Ref<GDDragonBones::GDDragonBonesResource> GDDragonBones::get_resource() {
	return m_res;
}

void GDDragonBones::set_inherit_material(bool enable) {
	b_inherit_child_material = enable;
	if (p_armature)
		p_armature->update_material_inheritance(b_inherit_child_material);
}

bool GDDragonBones::is_material_inherited() const {
	return b_inherit_child_material;
}

void GDDragonBones::fade_in(const String &anim_name, float time, int loop, int layer, const String &group, GDDragonBones::AnimFadeOutMode fade_out_mode) {
	p_factory->set_speed(f_speed);
	if (has_anim(anim_name)) {
		p_armature->getAnimation()->fadeIn(anim_name.ascii().get_data(), time, loop, layer, group.ascii().get_data(), (AnimationFadeOutMode)fade_out_mode);
		if (!b_playing) {
			b_playing = true;
			_set_process(true);
		}
	}
}

void GDDragonBones::fade_out(const String &anim_name) {
	if (!b_inited)
		return;
	if (!p_armature->getAnimation()->isPlaying() ||
			!p_armature->getAnimation()->hasAnimation(anim_name.ascii().get_data()))
		return;

	p_armature->getAnimation()->stop(anim_name.ascii().get_data());
	if (p_armature->getAnimation()->isPlaying())
		return;

	_set_process(false);
	b_playing = false;
	_reset();
}

void GDDragonBones::set_active(bool active) {
	if (b_active == active)
		return;
	b_active = active;
	_set_process(b_processing, true);
}

bool GDDragonBones::is_active() const {
	return b_active;
}

void GDDragonBones::set_debug(bool debug) {
	b_debug = debug;
	if (b_inited)
		p_armature->set_debug(b_debug);
}

bool GDDragonBones::is_debug() const {
	return b_debug;
}

void GDDragonBones::flip_x(bool flip) {
	b_flip_x = flip;
	if (!p_armature)
		return;
	p_armature->getArmature()->setFlipX(flip);
	p_armature->getArmature()->advanceTime(0);
}

bool GDDragonBones::is_fliped_x() const {
	return b_flip_x;
}

void GDDragonBones::flip_y(bool flip) {
	b_flip_y = flip;
	if (!p_armature)
		return;
	p_armature->getArmature()->setFlipY(flip);
	p_armature->getArmature()->advanceTime(0);
}

bool GDDragonBones::is_fliped_y() const {
	return b_flip_y;
}

void GDDragonBones::set_speed(float speed) {
	f_speed = speed;
	if (b_inited)
		p_factory->set_speed(speed);
}

float GDDragonBones::get_speed() const {
	return f_speed;
}

void GDDragonBones::set_animation_process_mode(GDDragonBones::AnimMode mode) {
	if (m_anim_mode == mode)
		return;
	bool pr = b_processing;
	if (pr)
		_set_process(false);
	m_anim_mode = mode;
	if (pr)
		_set_process(true);
}

GDDragonBones::AnimMode GDDragonBones::get_animation_process_mode() const {
	return m_anim_mode;
}

void GDDragonBones::_notification(int what) {
	switch (what) {
		case NOTIFICATION_ENTER_TREE: {
			if (!b_processing) {
				set_process(false);
				set_physics_process(false);
			}
		} break;

		case NOTIFICATION_READY: {
			if (b_playing && b_inited)
				play();
		} break;

		case NOTIFICATION_PROCESS: {
			if (m_anim_mode == ANIMATION_PROCESS_FIXED)
				break;
			if (b_processing)
				p_factory->update(get_process_delta_time());
		} break;

		case NOTIFICATION_PHYSICS_PROCESS: {
			if (m_anim_mode == ANIMATION_PROCESS_IDLE)
				break;
			if (b_processing)
				p_factory->update(get_physics_process_delta_time());
		} break;

		case NOTIFICATION_EXIT_TREE:
			break;
	}
}

void GDDragonBones::_reset() {
	p_armature->getAnimation()->reset();
}

void GDDragonBones::play(bool play) {
	b_playing = play;
	if (!play) {
		stop();
		return;
	}
	p_factory->set_speed(f_speed);
	if (has_anim(str_curr_anim)) {
		p_armature->getAnimation()->play(str_curr_anim.ascii().get_data(), c_loop);
		_set_process(true);
		b_try_playing = false;
	} else {
		b_try_playing = true;
		str_curr_anim = "[none]";
		stop();
	}
}

void GDDragonBones::play_from_time(float time) {
	play();
	if (b_playing)
		p_armature->getAnimation()->gotoAndPlayByTime(str_curr_anim.ascii().get_data(), time, c_loop);
}

void GDDragonBones::play_from_progress(float progress) {
	play();
	if (b_playing)
		p_armature->getAnimation()->gotoAndPlayByProgress(str_curr_anim.ascii().get_data(), CLAMP(progress, 0, 1.f), c_loop);
}

bool GDDragonBones::has_anim(const String &anim) const {
	return p_armature->getAnimation()->hasAnimation(anim.ascii().get_data());
}

void GDDragonBones::stop(bool all) {
	if (!b_inited)
		return;
	_set_process(false);
	b_playing = false;
	if (p_armature->getAnimation()->isPlaying())
		p_armature->getAnimation()->stop(all ? "" : str_curr_anim.ascii().get_data());
	_reset();
}

float GDDragonBones::tell() const {
	if (b_inited && has_anim(str_curr_anim)) {
		AnimationState *p_state = p_armature->getAnimation()->getState(str_curr_anim.ascii().get_data());
		if (p_state)
			return p_state->getCurrentTime() / p_state->_duration;
	}
	return 0;
}

void GDDragonBones::seek(float p) {
	if (b_inited && has_anim(str_curr_anim)) {
		f_progress = p;
		stop();
		auto cp = Math::fmod(p, 1.0f);
		if (cp == 0 && p != 0)
			cp = 1.0f;
		p_armature->getAnimation()->gotoAndStopByProgress(str_curr_anim.ascii().get_data(), cp < 0 ? 1. + cp : cp);
	}
}

float GDDragonBones::get_progress() const {
	return f_progress;
}

bool GDDragonBones::is_playing() const {
	return b_inited && b_playing && p_armature->getAnimation()->isPlaying();
}

String GDDragonBones::get_current_animation() const {
	if (!b_inited || !p_armature->getAnimation())
		return String("");
	return String(p_armature->getAnimation()->getLastAnimationName().c_str());
}

void GDDragonBones::_set_process(bool process, bool force) {
	if (b_processing == process && !force)
		return;
	switch (m_anim_mode) {
		case ANIMATION_PROCESS_FIXED:
			set_physics_process(process && b_active);
			break;
		case ANIMATION_PROCESS_IDLE:
			set_process(process && b_active);
			break;
	}
	b_processing = process;
}

void GDDragonBones::set_texture(const Ref<Texture> &texture) {
	if (texture.is_valid() && m_texture_atlas.is_valid() &&
			(texture == m_texture_atlas ||
					m_texture_atlas->get_height() != texture->get_height() ||
					m_texture_atlas->get_width() != texture->get_width()))
		return;

	m_texture_atlas = texture;

#ifdef DEBUG_ENABLED
	if (m_texture_atlas.is_valid())
		m_texture_atlas->set_flags(m_texture_atlas->get_flags());
#endif

	if (p_armature) {
		p_armature->update_texture_atlas(m_texture_atlas);
		update();
	}
}

Ref<Texture> GDDragonBones::get_texture() const {
	return m_texture_atlas;
}

bool GDDragonBones::_set(const StringName &name, const Variant &value) {
	String n = name;
	if (n == "playback/curr_animation") {
		if (str_curr_anim == value)
			return false;
		str_curr_anim = value;
		if (b_inited) {
			if (str_curr_anim == "[none]")
				stop();
			else if (has_anim(str_curr_anim)) {
				if (b_playing || b_try_playing)
					play();
				else
					p_armature->getAnimation()->gotoAndStopByProgress(str_curr_anim.ascii().get_data());
			}
		}
	} else if (n == "playback/loop") {
		c_loop = value;
		if (b_inited && b_playing) {
			_reset();
			play();
		}
	} else if (n == "playback/progress") {
		seek(value);
	}
	return true;
}

bool GDDragonBones::_get(const StringName &name, Variant &ret) const {
	String n = name;
	if (n == "playback/curr_animation")
		ret = str_curr_anim;
	else if (n == "playback/loop")
		ret = c_loop;
	else if (n == "playback/progress")
		ret = get_progress();
	return true;
}

void GDDragonBones::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &GDDragonBones::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &GDDragonBones::get_texture);

	ClassDB::bind_method(D_METHOD("set_resource", "dragonbones"), &GDDragonBones::set_resource);
	ClassDB::bind_method(D_METHOD("get_resource"), &GDDragonBones::get_resource);

	ClassDB::bind_method(D_METHOD("set_inherit_material"), &GDDragonBones::set_inherit_material);
	ClassDB::bind_method(D_METHOD("is_material_inherited"), &GDDragonBones::is_material_inherited);

	ClassDB::bind_method(D_METHOD("fade_in", "anim_name", "time", "loop", "layer", "group", "fade_out_mode"), &GDDragonBones::fade_in);
	ClassDB::bind_method(D_METHOD("fade_out", "anim_name"), &GDDragonBones::fade_out);

	ClassDB::bind_method(D_METHOD("stop"), &GDDragonBones::stop);
	ClassDB::bind_method(D_METHOD("stop_all"), &GDDragonBones::stop_all);
	ClassDB::bind_method(D_METHOD("reset"), &GDDragonBones::_reset);
	ClassDB::bind_method(D_METHOD("play"), &GDDragonBones::play);
	ClassDB::bind_method(D_METHOD("play_from_time"), &GDDragonBones::play_from_time);
	ClassDB::bind_method(D_METHOD("play_from_progress"), &GDDragonBones::play_from_progress);

	ClassDB::bind_method(D_METHOD("has", "name"), &GDDragonBones::has_anim);
	ClassDB::bind_method(D_METHOD("is_playing"), &GDDragonBones::is_playing);

	ClassDB::bind_method(D_METHOD("get_current_animation"), &GDDragonBones::get_current_animation);

	ClassDB::bind_method(D_METHOD("seek", "pos"), &GDDragonBones::seek);
	ClassDB::bind_method(D_METHOD("tell"), &GDDragonBones::tell);
	ClassDB::bind_method(D_METHOD("get_progress"), &GDDragonBones::get_progress);

	ClassDB::bind_method(D_METHOD("set_active", "active"), &GDDragonBones::set_active);
	ClassDB::bind_method(D_METHOD("is_active"), &GDDragonBones::is_active);

	ClassDB::bind_method(D_METHOD("set_debug", "debug"), &GDDragonBones::set_debug);
	ClassDB::bind_method(D_METHOD("is_debug"), &GDDragonBones::is_debug);

	ClassDB::bind_method(D_METHOD("flip_x", "enable_flip"), &GDDragonBones::flip_x);
	ClassDB::bind_method(D_METHOD("is_fliped_x"), &GDDragonBones::is_fliped_x);
	ClassDB::bind_method(D_METHOD("flip_y", "enable_flip"), &GDDragonBones::flip_y);
	ClassDB::bind_method(D_METHOD("is_fliped_y"), &GDDragonBones::is_fliped_y);

	ClassDB::bind_method(D_METHOD("set_speed", "speed"), &GDDragonBones::set_speed);
	ClassDB::bind_method(D_METHOD("get_speed"), &GDDragonBones::get_speed);

	ClassDB::bind_method(D_METHOD("set_animation_process_mode", "mode"), &GDDragonBones::set_animation_process_mode);
	ClassDB::bind_method(D_METHOD("get_animation_process_mode"), &GDDragonBones::get_animation_process_mode);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug"), "set_debug", "is_debug");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flipX"), "flip_x", "is_fliped_x");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flipY"), "flip_y", "is_fliped_y");

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "resource", PROPERTY_HINT_RESOURCE_TYPE, "GDDragonBonesResource"), "set_resource", "get_resource");

	ADD_PROPERTY(PropertyInfo(Variant::INT, "playback/process_mode", PROPERTY_HINT_ENUM, "Fixed,Idle"), "set_animation_process_mode", "get_animation_process_mode");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "playback/speed", PROPERTY_HINT_RANGE, "0,10,0.01"), "set_speed", "get_speed");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "playback/progress", PROPERTY_HINT_RANGE, "-100,100,0.010"), "seek", "get_progress");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "playback/play"), "play", "is_playing");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "childs use this material"), "set_inherit_material", "is_material_inherited");

	ADD_SIGNAL(MethodInfo("dragon_anim_start", PropertyInfo(Variant::STRING, "anim")));
	ADD_SIGNAL(MethodInfo("dragon_anim_complete", PropertyInfo(Variant::STRING, "anim")));
	ADD_SIGNAL(MethodInfo("dragon_anim_event", PropertyInfo(Variant::STRING, "anim"), PropertyInfo(Variant::STRING, "ev")));
	ADD_SIGNAL(MethodInfo("dragon_anim_loop_complete", PropertyInfo(Variant::STRING, "anim")));
	ADD_SIGNAL(MethodInfo("dragon_anim_snd_event", PropertyInfo(Variant::STRING, "anim"), PropertyInfo(Variant::STRING, "ev")));
	ADD_SIGNAL(MethodInfo("dragon_fade_in", PropertyInfo(Variant::STRING, "anim")));
	ADD_SIGNAL(MethodInfo("dragon_fade_in_complete", PropertyInfo(Variant::STRING, "anim")));
	ADD_SIGNAL(MethodInfo("dragon_fade_out", PropertyInfo(Variant::STRING, "anim")));
	ADD_SIGNAL(MethodInfo("dragon_fade_out_complete", PropertyInfo(Variant::STRING, "anim")));

	BIND_ENUM_CONSTANT(ANIMATION_PROCESS_FIXED);
	BIND_ENUM_CONSTANT(ANIMATION_PROCESS_IDLE);

	BIND_ENUM_CONSTANT(FadeOut_None);
	BIND_ENUM_CONSTANT(FadeOut_SameLayer);
	BIND_ENUM_CONSTANT(FadeOut_SameGroup);
	BIND_ENUM_CONSTANT(FadeOut_SameLayerAndGroup);
	BIND_ENUM_CONSTANT(FadeOut_All);
	BIND_ENUM_CONSTANT(FadeOut_Single);
}

void GDDragonBones::_get_property_list(List<PropertyInfo> *list) const {
	List<String> anim_names;

	if (b_inited && p_armature->getAnimation()) {
		auto names = p_armature->getAnimation()->getAnimationNames();
		for (auto it = names.cbegin(); it != names.cend(); ++it) {
			anim_names.push_back(it->c_str());
		}
	}

	anim_names.sort();
	anim_names.push_front("[none]");
	String hint;
	for (List<String>::Element *E = anim_names.front(); E; E = E->next()) {
		if (E != anim_names.front())
			hint += ",";
		hint += E->get();
	}

	list->push_back(PropertyInfo(Variant::STRING, "playback/curr_animation", PROPERTY_HINT_ENUM, hint));
	list->push_back(PropertyInfo(Variant::INT, "playback/loop", PROPERTY_HINT_RANGE, "-1,100,1"));
}
