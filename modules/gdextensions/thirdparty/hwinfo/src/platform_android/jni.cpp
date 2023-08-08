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
  WARN_PRINT("GodotJavaWrapper not found");
  return nullptr;
}

static _FORCE_INLINE_ jobject _get_activity() {
  return _get_gd_java()->get_activity();
}

// android.os.Build.VERSION.SDK_INT
int32_t get_android_sdk_version() {
  static jint sdkVersion = 0;
  if (!sdkVersion) {
    JNIEnv *env = get_jni_env();
    jclass versionClass = env->FindClass("android/os/Build$VERSION");
    ERR_FAIL_NULL_V(versionClass, 0);
    jfieldID sdkIntField = env->GetStaticFieldID(versionClass, "SDK_INT", "I");
    ERR_FAIL_NULL_V(sdkIntField, 0);
    sdkVersion = env->GetStaticIntField(versionClass, sdkIntField);
  }
  return sdkVersion;
}
