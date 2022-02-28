/*************************************************************************/
/*  java_jni_object.h                                                    */
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

/*  java_jni_object.h                                                    */
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

#include "core/ustring.h"
#include "core/variant.h"

#include "jni_utils.h"

#include <jni.h>
#include <stdarg.h>
#include <memory>

class JavaJniEnvironment {
	JNIEnv *jni_env;

public:
	JNIEnv *operator->() { return jni_env; }
	operator JNIEnv *() const { return jni_env; }
	static jclass findClass(const char *class_name, JNIEnv *env = 0);
	JavaJniEnvironment();
};

class JavaJniObject {
	G_DECL_PRIVATE_IMP(JavaJniObject);

private:
	template <typename T>
	T callMethodV(const char *method_name, const char *sig, va_list args) const;
	JavaJniObject callObjectMethodV(const char *method_name, const char *sig, va_list args) const;
	template <typename T>
	static T callStaticMethodV(const char *className, const char *method_name, const char *sig, va_list args);
	template <typename T>
	static T callStaticMethodV(jclass clazz, const char *method_name, const char *sig, va_list args);
	static JavaJniObject callStaticObjectMethodV(const char *className, const char *method_name, const char *sig, va_list args);
	static JavaJniObject callStaticObjectMethodV(jclass clazz, const char *method_name, const char *sig, va_list args);

	jobject javaObject() const;
	void assign(jobject obj);

	bool isSameObject(jobject obj) const;
	bool isSameObject(const JavaJniObject &other) const;

public:
	template <typename T>
	T callMethod(const char *method_name, const char *sig, ...) const;
	template <typename T>
	T callMethod(const char *method_name) const;
	template <typename T>
	JavaJniObject callObjectMethod(const char *method_name) const;
	JavaJniObject callObjectMethod(const char *method_name, const char *sig, ...) const;
	template <typename T>
	static T callStaticMethod(const char *className, const char *method_name, const char *sig, ...);
	template <typename T>
	static T callStaticMethod(const char *className, const char *method_name);
	template <typename T>
	static T callStaticMethod(jclass clazz, const char *method_name, const char *sig, ...);
	template <typename T>
	static T callStaticMethod(jclass clazz, const char *method_name);
	static JavaJniObject callStaticObjectMethod(const char *className, const char *method_name, const char *sig, ...);
	static JavaJniObject callStaticObjectMethod(jclass clazz, const String &method_name, const char *sig, ...);

	bool isValid() const;
	template <typename T>
	T object() const { return static_cast<T>(javaObject()); }
	jobject object() const;

	static JavaJniObject fromString(const String &string);
	String toString() const;

	Variant toVariant() const;
	operator Variant() const { return toVariant(); }

	static JavaJniObject fromGlobalRef(jobject global_ref) { return JavaJniObject(global_ref); }
	static JavaJniObject fromLocalRef(jobject obj);

	template <typename T>
	inline JavaJniObject &operator=(T o) {
		assign(static_cast<jobject>(o));
		return *this;
	}

	JavaJniObject();
	JavaJniObject(const char *class_name);
	JavaJniObject(const char *class_name, const char *sig, ...);
	JavaJniObject(jobject global_ref);
};
