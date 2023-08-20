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
JavaJniEnvironment::JavaJniEnvironment(JNIEnv *env) :
		jni_env(env ? env : get_jni_env()) {}

class JavaJniObject::JavaJniObjectPrivate {
public:
	jobject _jobject;
	jclass _jclass;
	bool _own_jclass;
	const char *_class_name;
	JavaJniEnvironment _env;

	JavaJniObjectPrivate() :
			_jobject(0), _jclass(0), _own_jclass(true) {}
	JavaJniObjectPrivate(JNIEnv *env) :
			_jobject(0), _jclass(0), _own_jclass(true), _env(env) {}
	JavaJniObjectPrivate(jobject o) {
		_jobject = _env->NewGlobalRef(o);
		jclass cls = _env->GetObjectClass(o);
		_jclass = static_cast<jclass>(_env->NewGlobalRef(cls));
		_env->DeleteLocalRef(cls);
	}
	~JavaJniObjectPrivate() {
		if (_jobject) {
			_env->DeleteGlobalRef(_jobject);
		}
		if (_jclass && _own_jclass) {
			_env->DeleteGlobalRef(_jclass);
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

static jclass getCachedClass(const String &class_name, bool *is_cached) {
#ifdef MULTI_THREAD_ACCESS
	MutexLock locker(cachedClassesLock);
#endif
	auto *it = cachedClasses.find(class_name);
	const bool found = (it != nullptr);

	if (is_cached) {
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

#define JNI_CALL_METHOD_V(T, M)                                                                        \
	template <>                                                                                        \
	T JavaJniObject::callMethodV<T>(const char *method_name, const char *sig, va_list args) const {    \
		T res = 0;                                                                                     \
		jmethodID id = getCachedMethodID(imp->_env, imp->_jclass, imp->_class_name, method_name, sig); \
		if (id) {                                                                                      \
			res = imp->_env->M(imp->_jobject, id, args);                                               \
		}                                                                                              \
		return res;                                                                                    \
	}

template <>
void JavaJniObject::callMethodV<void>(const char *method_name, const char *sig, va_list args) const {
	jmethodID id = getCachedMethodID(imp->_env, imp->_jclass, imp->_class_name, method_name, sig);
	if (id) {
		imp->_env->CallVoidMethodV(imp->_jobject, id, args);
	}
}

JNI_CALL_METHOD_V(jboolean, CallBooleanMethodV)
JNI_CALL_METHOD_V(jbyte, CallByteMethodV)
JNI_CALL_METHOD_V(jchar, CallCharMethodV)
JNI_CALL_METHOD_V(jshort, CallShortMethodV)
JNI_CALL_METHOD_V(jint, CallIntMethodV)
JNI_CALL_METHOD_V(jlong, CallLongMethodV)
JNI_CALL_METHOD_V(jfloat, CallFloatMethodV)
JNI_CALL_METHOD_V(jdouble, CallDoubleMethodV)

JavaJniObject JavaJniObject::callObjectMethodV(const char *method_name, const char *sig, va_list args) const {
	JavaJniEnvironment env;
	jobject res = 0;
	jmethodID id = getCachedMethodID(env, imp->_jclass, imp->_class_name, method_name, sig);
	if (id) {
		res = env->CallObjectMethodV(imp->_jobject, id, args);
		if (res && env->ExceptionCheck()) {
			res = 0;
		}
	}

	JavaJniObject obj(res);
	env->DeleteLocalRef(res);
	return obj;
}

JavaJniObject JavaJniObject::callObjectMethod(const char *method_name, const char *sig, ...) const {
	va_list args;
	va_start(args, sig);
	JavaJniObject res = callObjectMethodV(method_name, sig, args);
	va_end(args);
	return res;
}

#define JNI_CALL_STATIC_METHOD_V(T, M)                                                                                                        \
	template <>                                                                                                                               \
	T JavaJniObject::callStaticMethodV<T>(JNIEnv * jni_env, const char *class_name, const char *method_name, const char *sig, va_list args) { \
		JavaJniEnvironment env(jni_env);                                                                                                      \
		T res = 0;                                                                                                                            \
		jclass clazz = loadClass(class_name, env);                                                                                            \
		if (clazz) {                                                                                                                          \
			jmethodID id = getCachedMethodID(env, clazz, class_name, method_name, sig, true);                                                 \
			if (id) {                                                                                                                         \
				res = env->M(clazz, id, args);                                                                                                \
			}                                                                                                                                 \
		}                                                                                                                                     \
		return res;                                                                                                                           \
	}                                                                                                                                         \
                                                                                                                                              \
	template <>                                                                                                                               \
	T JavaJniObject::callStaticMethodV<T>(JNIEnv * jni_env, jclass clazz, const char *method_name, const char *sig, va_list args) {           \
		JavaJniEnvironment env(jni_env);                                                                                                      \
		T res = 0;                                                                                                                            \
		jmethodID id = getMethodID(env, clazz, method_name, sig, true);                                                                       \
		if (id) {                                                                                                                             \
			res = env->M(clazz, id, args);                                                                                                    \
		}                                                                                                                                     \
		return res;                                                                                                                           \
	}

template <>
void JavaJniObject::callStaticMethodV<void>(JNIEnv *jni_env, const char *class_name, const char *method_name, const char *sig, va_list args) {
	JavaJniEnvironment env(jni_env);
	jclass clazz = loadClass(class_name, env);
	if (clazz) {
		jmethodID id = getCachedMethodID(env, clazz, class_name, method_name, sig, true);
		if (id) {
			env->CallStaticVoidMethodV(clazz, id, args);
		}
	}
}

template <>
void JavaJniObject::callStaticMethodV<void>(JNIEnv *jni_env, jclass clazz, const char *method_name, const char *sig, va_list args) {
	JavaJniEnvironment env(jni_env);
	jmethodID id = getMethodID(env, clazz, method_name, sig, true);
	if (id) {
		env->CallStaticVoidMethodV(clazz, id, args);
	}
}

JNI_CALL_STATIC_METHOD_V(jboolean, CallStaticBooleanMethodV)
JNI_CALL_STATIC_METHOD_V(jbyte, CallStaticByteMethodV)
JNI_CALL_STATIC_METHOD_V(jchar, CallStaticCharMethodV)
JNI_CALL_STATIC_METHOD_V(jshort, CallStaticShortMethodV)
JNI_CALL_STATIC_METHOD_V(jint, CallStaticIntMethodV)
JNI_CALL_STATIC_METHOD_V(jlong, CallStaticLongMethodV)
JNI_CALL_STATIC_METHOD_V(jfloat, CallStaticFloatMethodV)
JNI_CALL_STATIC_METHOD_V(jdouble, CallStaticDoubleMethodV)

#define JNI_CALL_STATIC_METHOD(T)                                                                                               \
	template <>                                                                                                                 \
	T JavaJniObject::callStaticMethod<T>(jclass clazz, const char *method_name, const char *sig, ...) {                         \
		va_list args;                                                                                                           \
		va_start(args, sig);                                                                                                    \
		T res = callStaticMethodV<T>(nullptr, clazz, method_name, sig, args);                                                   \
		va_end(args);                                                                                                           \
		return res;                                                                                                             \
	}                                                                                                                           \
                                                                                                                                \
	template <>                                                                                                                 \
	T JavaJniObject::callStaticMethod<T>(JNIEnv * env, jclass clazz, const char *method_name, const char *sig, ...) {           \
		va_list args;                                                                                                           \
		va_start(args, sig);                                                                                                    \
		T res = callStaticMethodV<T>(env, clazz, method_name, sig, args);                                                       \
		va_end(args);                                                                                                           \
		return res;                                                                                                             \
	}                                                                                                                           \
                                                                                                                                \
	template <>                                                                                                                 \
	T JavaJniObject::callStaticMethod<T>(const char *class_name, const char *method_name, const char *sig, ...) {               \
		va_list args;                                                                                                           \
		va_start(args, sig);                                                                                                    \
		T res = callStaticMethodV<T>(nullptr, class_name, method_name, sig, args);                                              \
		va_end(args);                                                                                                           \
		return res;                                                                                                             \
	}                                                                                                                           \
                                                                                                                                \
	template <>                                                                                                                 \
	T JavaJniObject::callStaticMethod<T>(JNIEnv * env, const char *class_name, const char *method_name, const char *sig, ...) { \
		va_list args;                                                                                                           \
		va_start(args, sig);                                                                                                    \
		T res = callStaticMethodV<T>(env, class_name, method_name, sig, args);                                                  \
		va_end(args);                                                                                                           \
		return res;                                                                                                             \
	}

template <>
void JavaJniObject::callStaticMethod<void>(jclass clazz, const char *method_name, const char *sig, ...) {
	va_list args;
	va_start(args, sig);
	callStaticMethodV<void>(nullptr, clazz, method_name, sig, args);
	va_end(args);
}

template <>
void JavaJniObject::callStaticMethod<void>(JNIEnv *env, jclass clazz, const char *method_name, const char *sig, ...) {
	va_list args;
	va_start(args, sig);
	callStaticMethodV<void>(env, clazz, method_name, sig, args);
	va_end(args);
}

template <>
void JavaJniObject::callStaticMethod<void>(const char *class_name, const char *method_name, const char *sig, ...) {
	va_list args;
	va_start(args, sig);
	callStaticMethodV<void>(nullptr, class_name, method_name, sig, args);
	va_end(args);
}

template <>
void JavaJniObject::callStaticMethod<void>(JNIEnv *env, const char *class_name, const char *method_name, const char *sig, ...) {
	va_list args;
	va_start(args, sig);
	callStaticMethodV<void>(env, class_name, method_name, sig, args);
	va_end(args);
}

JNI_CALL_STATIC_METHOD(jboolean)
JNI_CALL_STATIC_METHOD(jbyte)
JNI_CALL_STATIC_METHOD(jchar)
JNI_CALL_STATIC_METHOD(jshort)
JNI_CALL_STATIC_METHOD(jint)
JNI_CALL_STATIC_METHOD(jlong)
JNI_CALL_STATIC_METHOD(jfloat)
JNI_CALL_STATIC_METHOD(jdouble)

#define JNI_CALL_STATIC_METHOD_SIG(T, S)                                                                                                                                 \
	template <>                                                                                                                                                          \
	T JavaJniObject::callStaticMethod<T>(const char *class_name, const char *method_name) { return callStaticMethod<T>(class_name, method_name, S); }                    \
	template <>                                                                                                                                                          \
	T JavaJniObject::callStaticMethod<T>(jclass clazz, const char *method_name) { return callStaticMethod<T>(clazz, method_name, S); }                                   \
	template <>                                                                                                                                                          \
	T JavaJniObject::callStaticMethod<T>(JNIEnv * env, const char *class_name, const char *method_name) { return callStaticMethod<T>(env, class_name, method_name, S); } \
	template <>                                                                                                                                                          \
	T JavaJniObject::callStaticMethod<T>(JNIEnv * env, jclass clazz, const char *method_name) { return callStaticMethod<T>(env, clazz, method_name, S); }

template <>
void JavaJniObject::callStaticMethod<void>(const char *class_name, const char *method_name) { return callStaticMethod<void>(class_name, method_name, "()V"); }
template <>
void JavaJniObject::callStaticMethod<void>(jclass clazz, const char *method_name) { return callStaticMethod<void>(clazz, method_name, "()V"); }
template <>
void JavaJniObject::callStaticMethod<void>(JNIEnv *env, const char *class_name, const char *method_name) { return callStaticMethod<void>(env, class_name, method_name, "()V"); }
template <>
void JavaJniObject::callStaticMethod<void>(JNIEnv *env, jclass clazz, const char *method_name) { return callStaticMethod<void>(env, clazz, method_name, "()V"); }

JNI_CALL_STATIC_METHOD_SIG(jboolean, "()Z")
JNI_CALL_STATIC_METHOD_SIG(jbyte, "()B")
JNI_CALL_STATIC_METHOD_SIG(jchar, "()C")
JNI_CALL_STATIC_METHOD_SIG(jshort, "()S")
JNI_CALL_STATIC_METHOD_SIG(jint, "()I")
JNI_CALL_STATIC_METHOD_SIG(jlong, "()J")
JNI_CALL_STATIC_METHOD_SIG(jfloat, "()F")
JNI_CALL_STATIC_METHOD_SIG(jdouble, "()D")

bool JavaJniObject::isValid() const {
	return imp->_jobject;
}
jobject JavaJniObject::javaObject() const {
	return imp->_jobject;
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
	return env->IsSameObject(imp->_jobject, obj);
}

bool JavaJniObject::isSameObject(const JavaJniObject &other) const {
	return isSameObject(other.imp->_jobject);
}

JavaJniObject::JavaJniObject() :
		imp(new JavaJniObjectPrivate()) {}

JavaJniObject::JavaJniObject(JNIEnv *jni_env) :
		imp(new JavaJniObjectPrivate(jni_env)) {}

JavaJniObject::JavaJniObject(const char *class_name) :
		imp(new JavaJniObjectPrivate()) {
	imp->_class_name = class_name;
	imp->_jclass = loadClass(imp->_class_name, imp->_env);
	imp->_own_jclass = false;
	if (imp->_jclass) {
		jmethodID constructorId = getCachedMethodID(imp->_env, imp->_jclass, imp->_class_name, "<init>", "()V"); // get default constructor
		if (constructorId) {
			jobject obj = imp->_env->NewObject(imp->_jclass, constructorId);
			if (obj) {
				imp->_jobject = imp->_env->NewGlobalRef(obj);
				imp->_env->DeleteLocalRef(obj);
			}
		}
	}
}

JavaJniObject::JavaJniObject(const char *class_name, const char *sig, ...) :
		imp(new JavaJniObjectPrivate()) {
	JavaJniEnvironment env;
	imp->_class_name = class_name;
	imp->_jclass = loadClass(imp->_class_name, env);
	imp->_own_jclass = false;
	if (imp->_jclass) {
		jmethodID constructorId = getCachedMethodID(env, imp->_jclass, imp->_class_name, "<init>", sig);
		if (constructorId) {
			va_list args;
			va_start(args, sig);
			jobject obj = env->NewObjectV(imp->_jclass, constructorId, args);
			va_end(args);
			if (obj) {
				imp->_jobject = env->NewGlobalRef(obj);
				env->DeleteLocalRef(obj);
			}
		}
	}
}

JavaJniObject::JavaJniObject(jobject global_ref) :
		imp(new JavaJniObjectPrivate()) {
	if (!global_ref) {
		return;
	}
	JavaJniEnvironment env;
	imp->_jobject = env->NewGlobalRef(global_ref);
	jclass cls = env->GetObjectClass(global_ref);
	imp->_jclass = static_cast<jclass>(env->NewGlobalRef(cls));
	env->DeleteLocalRef(cls);
}
