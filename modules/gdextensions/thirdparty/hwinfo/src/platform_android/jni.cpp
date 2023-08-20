// kate: replace-tabs on; tab-indents on; tab-width 2; indent-width 2; indent-mode cstyle;

#include <jni.h>

#include "core/bind/core_bind.h"
#include "core/io/marshalls.h"
#include "platform/android/java_godot_wrapper.h"
#include "platform/android/java_jni_object.h"
#include "platform/android/jni_utils.h"
#include "platform/android/os_android.h"
#include "platform/android/string_android.h"

#include <string>

struct _local_jstring {
  jstring js;
  JNIEnv *env;
  jstring get() const { return js; }
  _local_jstring(jstring js, JNIEnv *env = nullptr) : js(js), env(env) { }
  ~_local_jstring() {
    if (!env) {
      env = get_jni_env();
    }
    env->ReleaseStringUTF(js);
  }
};

static bool isinstanceof(JNIEnv *env, jobject obj, const char *name) {
  ERR_FAIL_COND_V(name == nullptr, false);
  ERR_FAIL_COND_V(obj == nullptr, false);
  return env->IsInstanceOf(obj, env->FindClass(name));
}

static _FORCE_INLINE_ GodotJavaWrapper *_get_gd_java() {
  if (OS_Android *os = (OS_Android *)OS::get_singleton()) {
    return os->get_godot_java();
  }
  WARN_PRINT("GodotJavaWrapper not found");
  return nullptr;
}

static _FORCE_INLINE_ jobject _get_activity() {
  return _get_gd_java()->get_activity();
}

const char *SystemClass = "java/lang/System";
const char *BuildClass = "android/os/Build";

// android.os.Build.VERSION.SDK_INT
int32_t get_android_sdk_version(JNIEnv *env = nullptr) {
  static jint sdk_version = 0;
  if (!sdk_version) {
    if (!env) {
        env = get_jni_env();
    }
    jclass version_class = env->FindClass("android/os/Build$VERSION");
    ERR_FAIL_NULL_V(version_class, 0);
    jfieldID sdk_int_field = env->GetStaticFieldID(version_class, "SDK_INT", "I");
    ERR_FAIL_NULL_V(sdk_int_field, 0);
    sdk_version = env->GetStaticIntField(version_class, sdk_int_field);
  }
  return sdk_version;
}

String get_system_property(const String& name, JNIEnv *env = nullptr) {
  if (!env) {
    env = get_jni_env();
  }
  return jstring_to_string((jstring) env->CallStaticObjectMethod(SystemClass, SystemClass.getProperty, _local_jstring(string_to_jstring(name), env).get()));
}

String get_android_os_build_value(const String &field_name, JNIEnv *env = nullptr) {
  if (!env) {
    env = get_jni_env();
  }
  return jstring_to_string((jstring)env->GetStaticObjectField(BuildClass, env->GetStaticFieldID(BuildClass, field_name.utf8().c_str(), "Ljava/lang/String;")));
}

bool is_64_bit() {
  return JavaJniObject::callStaticMethod<jboolean>("android.os.Process", "is64Bit", "()Z");
}
