/*************************************************************************/
/*  _sharedpreferences.cpp                                               */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include <jni.h>

#include "core/bind/core_bind.h"
#include "core/io/marshalls.h"
#include "platform/android/java_godot_wrapper.h"
#include "platform/android/jni_utils.h"
#include "platform/android/os_android.h"
#include "platform/android/string_android.h"

#include <string>

static bool isinstanceof(JNIEnv *env, jobject obj, const char *name) {
	ERR_FAIL_COND_V(name == nullptr, false);
	ERR_FAIL_COND_V(obj == nullptr, false);
	return env->IsInstanceOf(obj, env->FindClass(name));
}

static _FORCE_INLINE_ GodotJavaWrapper *_get_gd_java() {
	if (OS_Android *os = (OS_Android *)OS::get_singleton()) {
		return os->get_godot_java();
	}
	return nullptr;
}

static _FORCE_INLINE_ jobject _get_activity() {
	return _get_gd_java()->get_activity();
}

///Example reading values:
///-----------------------
/// SharedPreferences sharedPref(env, context, "pref_telemetry");
/// T_Protocol = sharedPref.getInt(IDT::T_Protocol);
///
///Example writing values:
///-----------------------
/// SharedPreferences_Editor editor = sharedPref.edit();
/// editor.putString("MY_KEY", "HELLO");
/// editor.commit();

class SharedPreferences_Editor {
public:
	SharedPreferences_Editor(JNIEnv *env, const jobject joSharedPreferences) :
			env(env), joSharedPreferences(joSharedPreferences) {
		//find the methods for putting values into Shared preferences via the editor
		jclass jcSharedPreferences_Editor = env->GetObjectClass(joSharedPreferences);
		jmPutBoolean = env->GetMethodID(jcSharedPreferences_Editor, "putBoolean", "(Ljava/lang/String;Z)Landroid/content/SharedPreferences$Editor;");
		jmPutInt = env->GetMethodID(jcSharedPreferences_Editor, "putInt", "(Ljava/lang/String;I)Landroid/content/SharedPreferences$Editor;");
		jmPutFloat = env->GetMethodID(jcSharedPreferences_Editor, "putFloat", "(Ljava/lang/String;F)Landroid/content/SharedPreferences$Editor;");
		jmPutString = env->GetMethodID(jcSharedPreferences_Editor, "putString", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/SharedPreferences$Editor;");
		jmCommit = env->GetMethodID(jcSharedPreferences_Editor, "commit", "()Z");
	}
	//return itself for method chaining
	const SharedPreferences_Editor &putBoolean(const char *key, const bool value) const {
		env->CallObjectMethod(joSharedPreferences, jmPutBoolean, env->NewStringUTF(key), (jboolean)value);
		return *this;
	}
	const SharedPreferences_Editor &putInt(const char *key, const int value) const {
		env->CallObjectMethod(joSharedPreferences, jmPutInt, env->NewStringUTF(key), (jint)value);
		return *this;
	}
	const SharedPreferences_Editor &putFloat(const char *key, const float value) const {
		env->CallObjectMethod(joSharedPreferences, jmPutFloat, env->NewStringUTF(key), (jfloat)value);
		return *this;
	}
	const SharedPreferences_Editor &putString(const char *key, const char *value) const {
		env->CallObjectMethod(joSharedPreferences, jmPutString, env->NewStringUTF(key), env->NewStringUTF(value));
		return *this;
	}
	bool commit() const {
		return (bool)env->CallBooleanMethod(joSharedPreferences, jmCommit);
	}

private:
	JNIEnv *env;
	jobject joSharedPreferences;
	jmethodID jmPutBoolean;
	jmethodID jmPutInt;
	jmethodID jmPutFloat;
	jmethodID jmPutString;
	jmethodID jmCommit;
};

class SharedPreferences {
public:
	SharedPreferences(SharedPreferences const &) = delete;
	void operator=(SharedPreferences const &) = delete;

public:
	//Note: Per default, this doesn't keep the reference to the sharedPreferences java object alive
	//longer than the lifetime of the JNIEnv.
	//With keepReference = true the joSharedPreferences is kept 'alive' and you can still use the class after the original JNIEnv* has become invalid -
	//but make sure to refresh the JNIEnv* object with a new valid reference via replaceJNI()
	SharedPreferences(JNIEnv *env, jobject androidContext, const char *name, const bool keepReference = false) {
		this->env = env;
		//Find the 2 java classes we need to make calls with
		jclass jcContext = env->FindClass("android/content/Context");
		jclass jcSharedPreferences = env->FindClass("android/content/SharedPreferences");
		//jclass jcSharedPreferences_Editor=env->FindClass("android/content/SharedPreferences$Editor");
		if (jcContext == nullptr || jcSharedPreferences == nullptr) {
			__android_log_print(ANDROID_LOG_DEBUG, "SharedPreferences", "Cannot find classes");
		}
		//find the 3 functions we need to get values from an SharedPreferences instance
		jmGetBoolean = env->GetMethodID(jcSharedPreferences, "getBoolean", "(Ljava/lang/String;Z)Z");
		jmGetInt = env->GetMethodID(jcSharedPreferences, "getInt", "(Ljava/lang/String;I)I");
		jmGetFloat = env->GetMethodID(jcSharedPreferences, "getFloat", "(Ljava/lang/String;F)F");
		jmGetString = env->GetMethodID(jcSharedPreferences, "getString", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
		jmGetStringSet = env->GetMethodID(jcSharedPreferences, "getStringSet", "(Ljava/lang/String;Ljava/util/Set;)Ljava/util/Set;");
		jmGetAll = env->GetMethodID(jcSharedPreferences, "getAll", "()Ljava/util/Map;");
		//find the 1 function we need to create the SharedPreferences.Editor object
		jmEdit = env->GetMethodID(jcSharedPreferences, "edit", "()Landroid/content/SharedPreferences$Editor;");
		//create a instance of SharedPreferences and store it in @joSharedPreferences
		jmethodID jmGetSharedPreferences = env->GetMethodID(jcContext, "getSharedPreferences", "(Ljava/lang/String;I)Landroid/content/SharedPreferences;");
		joSharedPreferences = env->CallObjectMethod(androidContext, jmGetSharedPreferences, env->NewStringUTF(name), MODE_PRIVATE);
		if (keepReference) {
			joSharedPreferences = env->NewWeakGlobalRef(joSharedPreferences);
		}
		//extra methods
		jclass mapClass = env->FindClass("java/util/HashMap");
		jmethodID jmMapGet = env->GetMethodID(mapClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
	}
	void replaceJNI(JNIEnv *newEnv) {
		env = newEnv;
	}

private:
	JNIEnv *env;
	jobject joSharedPreferences;
	jmethodID jmGetBoolean;
	jmethodID jmGetInt;
	jmethodID jmGetFloat;
	jmethodID jmGetString;
	jmethodID jmGetStringSet;
	jmethodID jmGetAll;
	jmethodID jmMapGet;
	jmethodID jmEdit;

public:
	// https://gist.github.com/theeasiestway/e5f453715cecc55b5ca57d0628b9f12a
	Variant getValue(const char *id) {
		if (jobject map = env->CallObjectMethod(joSharedPreferences, jmGetAll)) {
			if (jobject obj = env->CallObjectMethod(map, jmMapGet, env->NewStringUTF(id))) {
				Variant ret = _jobject_to_variant(env, obj);
				if (ret.get_type() == Variant::STRING) {
					String val = ret;
					Vector<String> split = val.split(";");
					if (split.size() == 3 && split[0] == "V") {
#ifdef DEBUG_ENABLED
						if (split[1] == split[2].md5_text())
#endif
						{
							ret = _Marshalls::get_singleton()->base64_to_variant(split[2]);
						}
#ifdef DEBUG_ENABLED
						else {
							WARN_PRINT("Serialized data are propably corrupted.");
						}
#endif
					}
				}
				return ret;
			}
		}
		return Variant();
	}
	bool getBoolean(const char *id, bool defaultValue = false) const {
		return (bool)(env->CallObjectMethod(joSharedPreferences, jmGetBoolean, env->NewStringUTF(id), (jboolean)defaultValue));
	}
	int getInt(const char *id, int defaultValue = 0) const {
		return (int)(env->CallIntMethod(joSharedPreferences, jmGetInt, env->NewStringUTF(id), (jint)defaultValue));
	}
	float getFloat(const char *id, float defaultValue = 0) const {
		return (float)(env->CallFloatMethod(joSharedPreferences, jmGetFloat, env->NewStringUTF(id), (jfloat)defaultValue));
	}
	std::string getString(const char *id, const char *defaultValue = "") const {
		auto value = (jstring)(env->CallObjectMethod(joSharedPreferences, jmGetString, env->NewStringUTF(id), env->NewStringUTF(defaultValue)));
		const char *valueP = env->GetStringUTFChars(value, nullptr);
		const std::string ret = std::string(valueP);
		env->ReleaseStringUTFChars(value, valueP);
		return ret;
	}
	SharedPreferences_Editor edit() const {
		//create a instance of SharedPreferences.Editor and store it in @joSharedPreferences
		jobject joSharedPreferences = env->CallObjectMethod(joSharedPreferences, jmEdit);
		SharedPreferences_Editor editor(env, joSharedPreferences);
		return editor;
	}

private:
	static constexpr const int MODE_PRIVATE = 0; // taken directly from java, assuming this value stays constant in java
};

class SettingsStorage : public Reference {
private:
	SharedPreferences prefs;
	real_t _last_sync_time;
	void _sync();

public:
	void set(const String &key, const Variant &value);
	Variant get(const String &key);

	SettingsStorage();
	~SettingsStorage();
};

void SettingsStorage::_sync() {
	if (OS::get_singleton()->get_ticks_msec() - _last_sync_time > 1000) {
		prefs.edit().commit();
		_last_sync_time = OS::get_singleton()->get_ticks_msec();
	}
}

void SettingsStorage::set(const String &key, const Variant &value) {
	switch (value.get_type()) {
		case Variant::BOOL: {
			prefs.edit().putBoolean(key.utf8().c_str(), bool(value));
			_sync();
		}
		case Variant::INT: {
			prefs.edit().putInt(key.utf8().c_str(), int(value));
			_sync();
		}
		case Variant::REAL: {
			prefs.edit().putFloat(key.utf8().c_str(), float(value));
			_sync();
		}
		case Variant::STRING: {
			prefs.edit().putString(key.utf8().c_str(), String(value).utf8().c_str());
			_sync();
		}
		default:
			String payload = _Marshalls::get_singleton()->variant_to_base64(value);
			String val = "V;" + payload.md5_text() + ";" + payload;
			prefs.edit().putString(key.utf8().c_str(), String(value).utf8().c_str());
			_sync();
			break;
	}
}

Variant SettingsStorage::get(const String &key) {
	return prefs.getValue(key.utf8().c_str());
}

SettingsStorage::SettingsStorage() :
		prefs(get_jni_env(), _get_activity(), get_app_name().utf8().c_str()) {
	_last_sync_time = OS::get_singleton()->get_ticks_msec();
}

SettingsStorage::~SettingsStorage() {
	prefs.edit().commit();
}
