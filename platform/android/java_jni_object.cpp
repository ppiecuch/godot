/**************************************************************************/
/*  java_jni_object.cpp                                                   */
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

#include "core/map.h"
#include "core/os/mutex.h"
#include "core/ustring.h"
#include "platform/android/java_godot_wrapper.h"
#include "platform/android/os_android.h"

#include "java_jni_object.h"

#include <jni.h>

#ifdef __GNUC__
#define G_LIKELY(expr) __builtin_expect(!!(expr), true)
#define G_UNLIKELY(expr) __builtin_expect(!!(expr), false)
#else
#define G_LIKELY(x) (x)
#define G_UNLIKELY(x) (x)
#endif

#ifdef MULTI_THREAD_ACCESS
static Mutex cachedClassesLock;
static Mutex cachedMethodIDLock;
#endif
static Map<String, jclass> cachedClasses;
static Map<String, jmethodID> cachedMethodID;

static String encClassName(const String &class_name);
static bool exceptionCheckAndClear(JNIEnv *env);
static jclass getCachedClass(const String &classBinEnc, bool *isCached = 0);
static jclass loadClass(const String &class_name, JNIEnv *env, bool binEncoded = false);

jclass JavaJniEnvironment::findClass(const char *class_name, JNIEnv *env) {
	const String &class_dot_enc = encClassName(class_name);
	bool is_cached = false;
	jclass clazz = getCachedClass(class_dot_enc, &is_cached);
	const bool found = (clazz != 0) || (clazz == 0 && is_cached);
	if (found) {
		return clazz;
	}
	const String key = class_dot_enc;
	if (env != nullptr) { // We got an env. pointer (We expect this to be the right env. and call FindClass())
#ifdef MULTI_THREAD_ACCESS
		MutexLock locker(cachedClassesLock);
#endif
		if (auto *it = cachedClasses.find(key)) {
			return it->value();
		}
		jclass fclazz = env->FindClass(class_name);
		if (!exceptionCheckAndClear(env)) {
			clazz = static_cast<jclass>(env->NewGlobalRef(fclazz));
			env->DeleteLocalRef(fclazz);
		}
		if (clazz != 0) {
			cachedClasses.insert(key, clazz);
		}
	}
	if (clazz == 0) { // We didn't get an env. pointer or we got one with the WRONG class loader...
		clazz = loadClass(class_dot_enc, JavaJniEnvironment(), true);
	}
	return clazz;
}

JavaJniEnvironment::JavaJniEnvironment() :
		jni_env(get_jni_env()) {}

class JavaJniObject::JavaJniObjectPrivate {
public:
	jobject m_jobject;
	jclass m_jclass;
	bool m_own_jclass;
	CharString m_className;

	JavaJniObjectPrivate() :
			m_jobject(0), m_jclass(0), m_own_jclass(true) {}
	JavaJniObjectPrivate(jobject o) {
		JavaJniEnvironment env;
		m_jobject = env->NewGlobalRef(o);
		jclass cls = env->GetObjectClass(o);
		m_jclass = static_cast<jclass>(env->NewGlobalRef(cls));
		env->DeleteLocalRef(cls);
	}
	~JavaJniObjectPrivate() {
		JavaJniEnvironment env;
		if (m_jobject) {
			env->DeleteGlobalRef(m_jobject);
		}
		if (m_jclass && m_own_jclass) {
			env->DeleteGlobalRef(m_jclass);
		}
	}
};

static _FORCE_INLINE_ jobject _get_gd_activity() {
	if (OS_Android *os = (OS_Android *)OS::get_singleton()) {
		return os->get_godot_java()->get_activity();
	}
	WARN_PRINT("GodotJavaWrapper not found");
	return nullptr;
}

static _FORCE_INLINE_ jobject _get_gd_class_loader() {
	if (OS_Android *os = (OS_Android *)OS::get_singleton()) {
		return os->get_godot_java()->get_class_loader();
	}
	WARN_PRINT("GodotJavaWrapper not found");
	return nullptr;
}

static String encClassName(const String &class_name) {
	return class_name.replace("/", ".");
}

static _FORCE_INLINE_ bool exceptionCheck(JNIEnv *env) {
	if (env->ExceptionCheck()) {
#ifdef DEBUG_ENABLED
		env->ExceptionDescribe();
#endif // DEBUG_ENABLED
		env->ExceptionClear();
		return true;
	}
	return false;
}

static bool exceptionCheckAndClear(JNIEnv *env) {
	if (G_UNLIKELY(env->ExceptionCheck())) {
#ifdef DEBUG_ENABLED
		env->ExceptionDescribe();
#endif // DEBUG_ENABLED
		env->ExceptionClear();
		return true;
	}

	return false;
}

static jobject getClassLoader(JNIEnv *env) {
	static jobject _jClassLoader;

	if (!_jClassLoader) {
		_jClassLoader = env->NewGlobalRef(_get_gd_class_loader());
	}
	return _jClassLoader;
}

static jclass getCachedClass(const String &class_bin_enc, bool *is_cached) {
#ifdef MULTI_THREAD_ACCESS
	MutexLock locker(cachedClassesLock);
#endif
	auto *it = cachedClasses.find(class_bin_enc);
	const bool found = (it != nullptr);

	if (is_cached != 0) {
		*is_cached = found;
	}
	return found ? it->value() : 0;
}

static inline jmethodID getMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig, bool isStatic = false) {
	jmethodID id = isStatic ? env->GetStaticMethodID(clazz, name, sig) : env->GetMethodID(clazz, name, sig);
	if (exceptionCheckAndClear(env)) {
		return 0;
	}
	return id;
}

static jmethodID getCachedMethodID(JNIEnv *env, jclass clazz, const char *class_name, const char *name, const char *sig, bool is_static = false) {
	if (!class_name || !*class_name) {
		return getMethodID(env, clazz, name, sig, is_static);
	}
	const String key = vformat("%s%s:%s", class_name, name, sig);

#ifdef MULTI_THREAD_ACCESS
	MutexLock locker(cachedMethodIDLock);
#endif
	if (auto *it = cachedMethodID.find(key)) {
		return it->value();
	}
	jmethodID id = getMethodID(env, clazz, name, sig, is_static);
	cachedMethodID.insert(key, id);
	return id;
}

static jclass loadClass(const char *class_name, JNIEnv *env, bool bin_encoded) {
	const String &bin_enc_class_name = bin_encoded ? class_name : encClassName(class_name);

	bool is_cached = false;
	jclass clazz = getCachedClass(bin_enc_class_name, &is_cached);
	if (clazz != 0 || is_cached) {
		return clazz;
	}
	JavaJniObject classLoader = JavaJniObject::fromGlobalRef(getClassLoader(env));
	if (!classLoader.isValid()) {
		return 0;
	}
#ifdef MULTI_THREAD_ACCESS
	MutexLock locker(cachedClassesLock);
#endif
	const String key = bin_enc_class_name;
	const auto *it = cachedClasses.find(key);
	if (it != nullptr) {
		return it->value();
	}
	JavaJniObject stringName = JavaJniObject::fromString(key);
	JavaJniObject classObject = classLoader.callObjectMethod("loadClass", "(Ljava/lang/String;)Ljava/lang/Class;", stringName.object());

	if (!exceptionCheckAndClear(env) && classObject.isValid()) {
		clazz = static_cast<jclass>(env->NewGlobalRef(classObject.object()));
	}
	cachedClasses.insert(key, clazz);
	return clazz;
}

template <>
void JavaJniObject::callMethodV<void>(const char *method_name, const char *sig, va_list args) const {
	JavaJniEnvironment env;
	jmethodID id = getCachedMethodID(env, imp->m_jclass, imp->m_className, method_name, sig);
	if (id) {
		env->CallVoidMethodV(imp->m_jobject, id, args);
	}
}

JavaJniObject JavaJniObject::callObjectMethodV(const char *method_name, const char *sig, va_list args) const {
	JavaJniEnvironment env;
	jobject res = 0;
	jmethodID id = getCachedMethodID(env, imp->m_jclass, imp->m_className, method_name, sig);
	if (id) {
		res = env->CallObjectMethodV(imp->m_jobject, id, args);
		if (res && env->ExceptionCheck())
			res = 0;
	}

	JavaJniObject obj(res);
	env->DeleteLocalRef(res);
	return obj;
}

bool JavaJniObject::isValid() const {
	return imp->m_jobject;
}
jobject JavaJniObject::javaObject() const {
	return imp->m_jobject;
}
jobject JavaJniObject::object() const {
	return javaObject();
}

void JavaJniObject::assign(jobject obj) {
	if (!obj) {
		return;
	}
	if (isSameObject(obj)) {
		return;
	}
	imp = std::make_shared<JavaJniObjectPrivate>(obj);
}

JavaJniObject JavaJniObject::fromString(const String &string) {
	JavaJniEnvironment env;
	jstring res = string_to_jstring(string);
	JavaJniObject obj = JavaJniObject::fromGlobalRef(res);
	env->DeleteLocalRef(res);
	return obj;
}

String JavaJniObject::toString() const {
	if (!isValid()) {
		return String();
	}
	JavaJniObject string = callObjectMethod<jstring>("toString");
	return jstring_to_string(static_cast<jstring>(string.object()));
}

JavaJniObject JavaJniObject::fromLocalRef(jobject lref) {
	JavaJniObject o(lref);
	JavaJniEnvironment()->DeleteLocalRef(lref);
	return o;
}

bool JavaJniObject::isSameObject(jobject obj) const {
	JavaJniEnvironment env;
	return env->IsSameObject(imp->m_jobject, obj);
}

bool JavaJniObject::isSameObject(const JavaJniObject &other) const {
	return isSameObject(other.imp->m_jobject);
}

JavaJniObject::JavaJniObject(jobject global_ref) :
		imp(new JavaJniObjectPrivate()) {
	if (!global_ref) {
		return;
	}
	JavaJniEnvironment env;
	imp->m_jobject = env->NewGlobalRef(global_ref);
	jclass cls = env->GetObjectClass(global_ref);
	imp->m_jclass = static_cast<jclass>(env->NewGlobalRef(cls));
	env->DeleteLocalRef(cls);
}

JavaJniObject::JavaJniObject() :
		imp(new JavaJniObjectPrivate()) {}

JavaJniObject::JavaJniObject(const char *class_name) :
		imp(new JavaJniObjectPrivate()) {
	JavaJniEnvironment env;
	imp->m_className = class_name;
	imp->m_jclass = loadClass(imp->m_className, env, true);
	imp->m_own_jclass = false;
	if (imp->m_jclass) {
		// get default constructor
		jmethodID constructorId = getCachedMethodID(env, imp->m_jclass, imp->m_className, "<init>", "()V");
		if (constructorId) {
			jobject obj = env->NewObject(imp->m_jclass, constructorId);
			if (obj) {
				imp->m_jobject = env->NewGlobalRef(obj);
				env->DeleteLocalRef(obj);
			}
		}
	}
}

JavaJniObject::JavaJniObject(const char *class_name, const char *sig, ...) :
		imp(new JavaJniObjectPrivate()) {
	JavaJniEnvironment env;
	imp->m_className = class_name;
	imp->m_jclass = loadClass(imp->m_className, env, true);
	imp->m_own_jclass = false;
	if (imp->m_jclass) {
		jmethodID constructorId = getCachedMethodID(env, imp->m_jclass, imp->m_className, "<init>", sig);
		if (constructorId) {
			va_list args;
			va_start(args, sig);
			jobject obj = env->NewObjectV(imp->m_jclass, constructorId, args);
			va_end(args);
			if (obj) {
				imp->m_jobject = env->NewGlobalRef(obj);
				env->DeleteLocalRef(obj);
			}
		}
	}
}
