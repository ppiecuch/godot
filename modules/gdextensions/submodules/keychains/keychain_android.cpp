/**************************************************************************/
/*  keychain_android.cpp                                                  */
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

#include "keychain.h"

#ifdef ANDROID_ENABLED

#include "core/os/os.h"
#include "core/print_string.h"
#include "platform/android/thread_jandroid.h"

#include <jni.h>

// Helper: check for JNI exception, clear it, return error message.
static bool _jni_check_exception(JNIEnv *env, String &r_error) {
	if (env->ExceptionCheck()) {
		jthrowable exc = env->ExceptionOccurred();
		env->ExceptionClear();
		if (exc) {
			jclass cls = env->GetObjectClass(exc);
			jmethodID mid = env->GetMethodID(cls, "getMessage", "()Ljava/lang/String;");
			if (mid) {
				jstring jmsg = (jstring)env->CallObjectMethod(exc, mid);
				if (jmsg) {
					const char *cstr = env->GetStringUTFChars(jmsg, nullptr);
					r_error = String::utf8(cstr);
					env->ReleaseStringUTFChars(jmsg, cstr);
					env->DeleteLocalRef(jmsg);
				}
			}
			env->DeleteLocalRef(cls);
			env->DeleteLocalRef(exc);
		}
		if (r_error.empty()) {
			r_error = "Unknown JNI exception";
		}
		return true;
	}
	return false;
}

// Helper: make JNI string from Godot string.
static jstring _to_jstring(JNIEnv *env, const String &p_str) {
	CharString utf8 = p_str.utf8();
	return env->NewStringUTF(utf8.get_data());
}

// Helper: get alias for the key pair = "service/key".
static String _make_alias(const String &p_service, const String &p_key) {
	return p_service + "/" + p_key;
}

// --- KeyStore operations ---

// Check if a key pair exists in AndroidKeyStore for the given alias.
static bool _keystore_has_alias(JNIEnv *env, const String &p_alias, String &r_error) {
	jclass ks_class = env->FindClass("java/security/KeyStore");
	if (!ks_class) {
		r_error = "KeyStore class not found.";
		return false;
	}

	jmethodID get_instance = env->GetStaticMethodID(ks_class, "getInstance", "(Ljava/lang/String;)Ljava/security/KeyStore;");
	jstring aks = _to_jstring(env, "AndroidKeyStore");
	jobject ks = env->CallStaticObjectMethod(ks_class, get_instance, aks);
	env->DeleteLocalRef(aks);
	if (_jni_check_exception(env, r_error)) {
		env->DeleteLocalRef(ks_class);
		return false;
	}

	// ks.load(null)
	jmethodID load = env->GetMethodID(ks_class, "load", "(Ljava/security/KeyStore$LoadStoreParameter;)V");
	env->CallVoidMethod(ks, load, nullptr);
	if (_jni_check_exception(env, r_error)) {
		env->DeleteLocalRef(ks);
		env->DeleteLocalRef(ks_class);
		return false;
	}

	// ks.containsAlias(alias)
	jmethodID contains = env->GetMethodID(ks_class, "containsAlias", "(Ljava/lang/String;)Z");
	jstring jalias = _to_jstring(env, p_alias);
	jboolean result = env->CallBooleanMethod(ks, contains, jalias);
	env->DeleteLocalRef(jalias);

	String exc_msg;
	_jni_check_exception(env, exc_msg);

	env->DeleteLocalRef(ks);
	env->DeleteLocalRef(ks_class);
	return (bool)result;
}

// Delete a key pair from AndroidKeyStore.
static bool _keystore_delete_alias(JNIEnv *env, const String &p_alias, String &r_error) {
	jclass ks_class = env->FindClass("java/security/KeyStore");
	jmethodID get_instance = env->GetStaticMethodID(ks_class, "getInstance", "(Ljava/lang/String;)Ljava/security/KeyStore;");
	jstring aks = _to_jstring(env, "AndroidKeyStore");
	jobject ks = env->CallStaticObjectMethod(ks_class, get_instance, aks);
	env->DeleteLocalRef(aks);
	if (_jni_check_exception(env, r_error)) {
		env->DeleteLocalRef(ks_class);
		return false;
	}

	jmethodID load = env->GetMethodID(ks_class, "load", "(Ljava/security/KeyStore$LoadStoreParameter;)V");
	env->CallVoidMethod(ks, load, nullptr);

	jmethodID del = env->GetMethodID(ks_class, "deleteEntry", "(Ljava/lang/String;)V");
	jstring jalias = _to_jstring(env, p_alias);
	env->CallVoidMethod(ks, del, jalias);
	env->DeleteLocalRef(jalias);

	bool exc = _jni_check_exception(env, r_error);
	env->DeleteLocalRef(ks);
	env->DeleteLocalRef(ks_class);
	return !exc;
}

// Encrypt data using the RSA public key from AndroidKeyStore.
// Returns encrypted bytes, or empty on failure.
static PoolByteArray _keystore_encrypt(JNIEnv *env, const String &p_alias, const PoolByteArray &p_data, String &r_error) {
	PoolByteArray result;

	// Get KeyStore entry
	jclass ks_class = env->FindClass("java/security/KeyStore");
	jmethodID get_instance = env->GetStaticMethodID(ks_class, "getInstance", "(Ljava/lang/String;)Ljava/security/KeyStore;");
	jstring aks = _to_jstring(env, "AndroidKeyStore");
	jobject ks = env->CallStaticObjectMethod(ks_class, get_instance, aks);
	env->DeleteLocalRef(aks);
	if (_jni_check_exception(env, r_error)) {
		env->DeleteLocalRef(ks_class);
		return result;
	}

	jmethodID load = env->GetMethodID(ks_class, "load", "(Ljava/security/KeyStore$LoadStoreParameter;)V");
	env->CallVoidMethod(ks, load, nullptr);

	// Get certificate's public key
	jmethodID get_cert = env->GetMethodID(ks_class, "getCertificate", "(Ljava/lang/String;)Ljava/security/cert/Certificate;");
	jstring jalias = _to_jstring(env, p_alias);
	jobject cert = env->CallObjectMethod(ks, get_cert, jalias);
	env->DeleteLocalRef(jalias);
	if (!cert || _jni_check_exception(env, r_error)) {
		env->DeleteLocalRef(ks);
		env->DeleteLocalRef(ks_class);
		r_error = "Could not get certificate for alias.";
		return result;
	}

	jclass cert_class = env->GetObjectClass(cert);
	jmethodID get_pub_key = env->GetMethodID(cert_class, "getPublicKey", "()Ljava/security/PublicKey;");
	jobject pub_key = env->CallObjectMethod(cert, get_pub_key);
	env->DeleteLocalRef(cert_class);
	env->DeleteLocalRef(cert);

	// Create Cipher
	jclass cipher_class = env->FindClass("javax/crypto/Cipher");
	jmethodID cipher_get = env->GetStaticMethodID(cipher_class, "getInstance", "(Ljava/lang/String;)Ljavax/crypto/Cipher;");
	jstring transform = env->NewStringUTF("RSA/ECB/PKCS1Padding");
	jobject cipher = env->CallStaticObjectMethod(cipher_class, cipher_get, transform);
	env->DeleteLocalRef(transform);

	// cipher.init(ENCRYPT_MODE, publicKey)
	jmethodID cipher_init = env->GetMethodID(cipher_class, "init", "(ILjava/security/Key;)V");
	env->CallVoidMethod(cipher, cipher_init, 1 /* ENCRYPT_MODE */, pub_key);
	env->DeleteLocalRef(pub_key);

	if (_jni_check_exception(env, r_error)) {
		env->DeleteLocalRef(cipher);
		env->DeleteLocalRef(cipher_class);
		env->DeleteLocalRef(ks);
		env->DeleteLocalRef(ks_class);
		return result;
	}

	// cipher.doFinal(data)
	PoolByteArray::Read r = p_data.read();
	jbyteArray jin = env->NewByteArray(p_data.size());
	env->SetByteArrayRegion(jin, 0, p_data.size(), (const jbyte *)r.ptr());

	jmethodID do_final = env->GetMethodID(cipher_class, "doFinal", "([B)[B");
	jbyteArray jout = (jbyteArray)env->CallObjectMethod(cipher, do_final, jin);
	env->DeleteLocalRef(jin);

	if (_jni_check_exception(env, r_error) || !jout) {
		env->DeleteLocalRef(cipher);
		env->DeleteLocalRef(cipher_class);
		env->DeleteLocalRef(ks);
		env->DeleteLocalRef(ks_class);
		return result;
	}

	int out_len = env->GetArrayLength(jout);
	result.resize(out_len);
	{
		PoolByteArray::Write w = result.write();
		env->GetByteArrayRegion(jout, 0, out_len, (jbyte *)w.ptr());
	}
	env->DeleteLocalRef(jout);
	env->DeleteLocalRef(cipher);
	env->DeleteLocalRef(cipher_class);
	env->DeleteLocalRef(ks);
	env->DeleteLocalRef(ks_class);
	return result;
}

// Decrypt data using the RSA private key from AndroidKeyStore.
static PoolByteArray _keystore_decrypt(JNIEnv *env, const String &p_alias, const PoolByteArray &p_data, String &r_error) {
	PoolByteArray result;

	jclass ks_class = env->FindClass("java/security/KeyStore");
	jmethodID get_instance = env->GetStaticMethodID(ks_class, "getInstance", "(Ljava/lang/String;)Ljava/security/KeyStore;");
	jstring aks = _to_jstring(env, "AndroidKeyStore");
	jobject ks = env->CallStaticObjectMethod(ks_class, get_instance, aks);
	env->DeleteLocalRef(aks);
	if (_jni_check_exception(env, r_error)) {
		env->DeleteLocalRef(ks_class);
		return result;
	}

	jmethodID load = env->GetMethodID(ks_class, "load", "(Ljava/security/KeyStore$LoadStoreParameter;)V");
	env->CallVoidMethod(ks, load, nullptr);

	// Get private key
	jmethodID get_key = env->GetMethodID(ks_class, "getKey", "(Ljava/lang/String;[C)Ljava/security/Key;");
	jstring jalias = _to_jstring(env, p_alias);
	jobject priv_key = env->CallObjectMethod(ks, get_key, jalias, nullptr);
	env->DeleteLocalRef(jalias);
	if (!priv_key || _jni_check_exception(env, r_error)) {
		env->DeleteLocalRef(ks);
		env->DeleteLocalRef(ks_class);
		r_error = "Could not get private key for alias.";
		return result;
	}

	// Create Cipher for decryption
	jclass cipher_class = env->FindClass("javax/crypto/Cipher");
	jmethodID cipher_get = env->GetStaticMethodID(cipher_class, "getInstance", "(Ljava/lang/String;)Ljavax/crypto/Cipher;");
	jstring transform = env->NewStringUTF("RSA/ECB/PKCS1Padding");
	jobject cipher = env->CallStaticObjectMethod(cipher_class, cipher_get, transform);
	env->DeleteLocalRef(transform);

	jmethodID cipher_init = env->GetMethodID(cipher_class, "init", "(ILjava/security/Key;)V");
	env->CallVoidMethod(cipher, cipher_init, 2 /* DECRYPT_MODE */, priv_key);
	env->DeleteLocalRef(priv_key);

	if (_jni_check_exception(env, r_error)) {
		env->DeleteLocalRef(cipher);
		env->DeleteLocalRef(cipher_class);
		env->DeleteLocalRef(ks);
		env->DeleteLocalRef(ks_class);
		return result;
	}

	// cipher.doFinal(encrypted)
	PoolByteArray::Read r = p_data.read();
	jbyteArray jin = env->NewByteArray(p_data.size());
	env->SetByteArrayRegion(jin, 0, p_data.size(), (const jbyte *)r.ptr());

	jmethodID do_final = env->GetMethodID(cipher_class, "doFinal", "([B)[B");
	jbyteArray jout = (jbyteArray)env->CallObjectMethod(cipher, do_final, jin);
	env->DeleteLocalRef(jin);

	if (_jni_check_exception(env, r_error) || !jout) {
		env->DeleteLocalRef(cipher);
		env->DeleteLocalRef(cipher_class);
		env->DeleteLocalRef(ks);
		env->DeleteLocalRef(ks_class);
		return result;
	}

	int out_len = env->GetArrayLength(jout);
	result.resize(out_len);
	{
		PoolByteArray::Write w = result.write();
		env->GetByteArrayRegion(jout, 0, out_len, (jbyte *)w.ptr());
	}
	env->DeleteLocalRef(jout);
	env->DeleteLocalRef(cipher);
	env->DeleteLocalRef(cipher_class);
	env->DeleteLocalRef(ks);
	env->DeleteLocalRef(ks_class);
	return result;
}

// Generate an RSA key pair in AndroidKeyStore for the given alias.
static bool _keystore_generate_keypair(JNIEnv *env, const String &p_alias, String &r_error) {
	// KeyPairGenerator.getInstance("RSA", "AndroidKeyStore")
	jclass kpg_class = env->FindClass("java/security/KeyPairGenerator");
	jmethodID kpg_get = env->GetStaticMethodID(kpg_class, "getInstance",
			"(Ljava/lang/String;Ljava/lang/String;)Ljava/security/KeyPairGenerator;");
	jstring jrsa = env->NewStringUTF("RSA");
	jstring jaks = env->NewStringUTF("AndroidKeyStore");
	jobject kpg = env->CallStaticObjectMethod(kpg_class, kpg_get, jrsa, jaks);
	env->DeleteLocalRef(jrsa);
	env->DeleteLocalRef(jaks);
	if (_jni_check_exception(env, r_error)) {
		env->DeleteLocalRef(kpg_class);
		return false;
	}

	// Build KeyGenParameterSpec (API 23+)
	jclass spec_class = env->FindClass("android/security/keystore/KeyGenParameterSpec$Builder");
	if (!spec_class) {
		// Fallback: try legacy KeyPairGeneratorSpec (API 18-22)
		// For simplicity, if KeyGenParameterSpec is not available, fail gracefully
		env->ExceptionClear();
		r_error = "Android API 23+ required for KeyStore key generation.";
		env->DeleteLocalRef(kpg);
		env->DeleteLocalRef(kpg_class);
		return false;
	}

	// new KeyGenParameterSpec.Builder(alias, PURPOSE_ENCRYPT | PURPOSE_DECRYPT)
	jmethodID spec_init = env->GetMethodID(spec_class, "<init>", "(Ljava/lang/String;I)V");
	jstring jalias = _to_jstring(env, p_alias);
	jobject builder = env->NewObject(spec_class, spec_init, jalias, 3 /* ENCRYPT | DECRYPT */);
	env->DeleteLocalRef(jalias);

	// .setEncryptionPaddings("PKCS1Padding")
	jmethodID set_paddings = env->GetMethodID(spec_class, "setEncryptionPaddings",
			"([Ljava/lang/String;)Landroid/security/keystore/KeyGenParameterSpec$Builder;");
	jstring jpadding = env->NewStringUTF("PKCS1Padding");
	jobjectArray paddings = env->NewObjectArray(1, env->FindClass("java/lang/String"), jpadding);
	env->CallObjectMethod(builder, set_paddings, paddings);
	env->DeleteLocalRef(jpadding);
	env->DeleteLocalRef(paddings);

	// .setBlockModes("ECB")
	jmethodID set_blocks = env->GetMethodID(spec_class, "setBlockModes",
			"([Ljava/lang/String;)Landroid/security/keystore/KeyGenParameterSpec$Builder;");
	jstring jecb = env->NewStringUTF("ECB");
	jobjectArray blocks = env->NewObjectArray(1, env->FindClass("java/lang/String"), jecb);
	env->CallObjectMethod(builder, set_blocks, blocks);
	env->DeleteLocalRef(jecb);
	env->DeleteLocalRef(blocks);

	// .build()
	jmethodID build = env->GetMethodID(spec_class, "build",
			"()Landroid/security/keystore/KeyGenParameterSpec;");
	jobject spec = env->CallObjectMethod(builder, build);
	env->DeleteLocalRef(builder);
	env->DeleteLocalRef(spec_class);

	// kpg.initialize(spec)
	jmethodID kpg_init = env->GetMethodID(kpg_class, "initialize",
			"(Ljava/security/spec/AlgorithmParameterSpec;)V");
	env->CallVoidMethod(kpg, kpg_init, spec);
	env->DeleteLocalRef(spec);

	if (_jni_check_exception(env, r_error)) {
		env->DeleteLocalRef(kpg);
		env->DeleteLocalRef(kpg_class);
		return false;
	}

	// kpg.generateKeyPair()
	jmethodID gen = env->GetMethodID(kpg_class, "generateKeyPair", "()Ljava/security/KeyPair;");
	jobject kp = env->CallObjectMethod(kpg, gen);
	env->DeleteLocalRef(kp);
	env->DeleteLocalRef(kpg);
	env->DeleteLocalRef(kpg_class);

	return !_jni_check_exception(env, r_error);
}

// --- Platform implementation ---

Keychain::Error Keychain::_platform_read(const String &p_key, PoolByteArray &r_data) {
	JNIEnv *env = get_jni_env();
	ERR_FAIL_COND_V(!env, NO_BACKEND_AVAILABLE);

	String alias = _make_alias(_service, p_key);
	String jni_error;

	if (!_keystore_has_alias(env, alias, jni_error)) {
		if (!jni_error.empty()) {
			_last_error_string = jni_error;
			return OTHER_ERROR;
		}
		_last_error_string = "Entry not found.";
		return ENTRY_NOT_FOUND;
	}

	// Read encrypted blob from fallback storage
	PoolByteArray encrypted;
	Error fb_err = _fallback_read(p_key, encrypted);
	if (fb_err != NO_ERROR) {
		_last_error_string = "Encrypted blob not found in storage.";
		return ENTRY_NOT_FOUND;
	}

	// Decrypt with AndroidKeyStore
	r_data = _keystore_decrypt(env, alias, encrypted, jni_error);
	if (!jni_error.empty()) {
		_last_error_string = "Decryption failed: " + jni_error;
		return OTHER_ERROR;
	}

	_last_error_string = "";
	return NO_ERROR;
}

Keychain::Error Keychain::_platform_write(const String &p_key, const PoolByteArray &p_data) {
	JNIEnv *env = get_jni_env();
	ERR_FAIL_COND_V(!env, NO_BACKEND_AVAILABLE);

	String alias = _make_alias(_service, p_key);
	String jni_error;

	// Generate key pair if not exists
	if (!_keystore_has_alias(env, alias, jni_error)) {
		if (!_keystore_generate_keypair(env, alias, jni_error)) {
			_last_error_string = "Key generation failed: " + jni_error;
			return OTHER_ERROR;
		}
	}

	// Encrypt with public key
	PoolByteArray encrypted = _keystore_encrypt(env, alias, p_data, jni_error);
	if (!jni_error.empty()) {
		_last_error_string = "Encryption failed: " + jni_error;
		return OTHER_ERROR;
	}

	// Store encrypted blob via fallback storage
	Error fb_err = _fallback_write(p_key, encrypted);
	if (fb_err != NO_ERROR) {
		return fb_err;
	}

	_last_error_string = "";
	return NO_ERROR;
}

Keychain::Error Keychain::_platform_delete(const String &p_key) {
	JNIEnv *env = get_jni_env();
	ERR_FAIL_COND_V(!env, NO_BACKEND_AVAILABLE);

	String alias = _make_alias(_service, p_key);
	String jni_error;

	// Delete key pair from AndroidKeyStore
	_keystore_delete_alias(env, alias, jni_error);

	// Also delete encrypted blob from fallback storage
	_fallback_delete(p_key);

	_last_error_string = "";
	return NO_ERROR;
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[keychains]] Android KeyStore backend") {
	TEST_CASE("Alias format") {
		CHECK(_make_alias("my_app", "token") == "my_app/token");
		CHECK(_make_alias("com.example", "secret_key") == "com.example/secret_key");
		CHECK(_make_alias("", "key") == "/key");
	}
}

#endif // DOCTEST

#endif // ANDROID_ENABLED
