/**************************************************************************/
/*  cripter.cpp                                                           */
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

#include "thirdparty/mbedtls/include/mbedtls/aes.h"
#include "thirdparty/mbedtls/include/mbedtls/ctr_drbg.h"
#include "thirdparty/mbedtls/include/mbedtls/entropy.h"
#include "thirdparty/mbedtls/include/mbedtls/gcm.h"
#include "thirdparty/mbedtls/include/mbedtls/pk.h"
#include "thirdparty/mbedtls/include/mbedtls/rsa.h"

#include "thirdparty/mbedtls/include/mbedtls/error.h" //  ---  Desenvolver   ---

#include "core/io/marshalls.h"
#include "core/print_string.h"

#include <stdint.h>
#include <vector>

#include "cripter.h"

// FIXME: For some reason mbedtls_strerror is undefined in gcc release builds
// (optimization?) for gcc and non-intel archs. Remove this when builds are fine.
#if defined(__linux__) || defined(__mips__) || defined(__arm__) || defined(__aarch64__)
extern "C" void mbedtls_strerror(int ret, char *buf, size_t buflen) {
	snprintf(buf, buflen, "MBEDTLS ERROR CODE (%04X)", ret);
}
#endif

//--- Do:
//RSA  ---> Check if key file is valid / Maximun input size / Erros

#define KEY_SIZE 32
#define EXT_SIZE 16
#define TAG_SIZE 4

//-------------- Encrypt Vars
PoolByteArray Cripter::encrypt_var_CBC(const Variant p_input, const String p_key) const {
	return encrypt_byte_CBC((encode_var(p_input)), p_key);
}

PoolByteArray Cripter::encrypt_var_GCM(const Variant p_input, const String p_key, const String p_add) const {
	return encrypt_byte_GCM((encode_var(p_input)), p_key, p_add);
}

PoolByteArray Cripter::encrypt_var_RSA(const Variant p_input, const String p_key_path) const {
	return encrypt_byte_RSA((encode_var(p_input)), p_key_path);
}

//-------------- Decrypt Vars
Variant Cripter::decrypt_var_CBC(const PoolByteArray p_input, const String p_key) const {
	return decode_var((decrypt_byte_CBC(p_input, p_key)));
}

Variant Cripter::decrypt_var_GCM(const PoolByteArray p_input, const String p_key, const String p_add) const {
	return decode_var(decrypt_byte_GCM(p_input, p_key, p_add));
}

Variant Cripter::decrypt_var_RSA(const PoolByteArray p_input, const String p_key_path, const String p_password) const {
	return decode_var((decrypt_byte_RSA(p_input, p_key_path, p_password)));
}

//-------------- Simetric - GCM
PoolByteArray Cripter::encrypt_byte_GCM(const PoolByteArray p_input, const String p_key, const String p_add) const {
	int _err = 0;
	//Prepare key & iv **
	String h_key = p_key.md5_text();
	uint8_t key[KEY_SIZE];
	uint8_t iv[EXT_SIZE];
	for (int i = 0; i < KEY_SIZE; i++) {
		key[i] = h_key[i];
	}
	for (int i = 0; i < EXT_SIZE; i++) {
		iv[i] = key[i * 2];
	}
	//Preparing Buffer
	char erro[150];
	std::vector<uint8_t> input(p_input.size());
	std::vector<uint8_t> output(p_input.size());

	PoolVector<uint8_t>::Read r = p_input.read(); //PoolByteArray to CharArray
	for (int i = 0; i < p_input.size(); i++) {
		input[i] = (uint8_t)p_input[i];
	}

	//Prepare Tag
	uint8_t tag[TAG_SIZE];
	//Prepare Addicional Data
	int add_len = p_add.length();
	std::vector<uint8_t> add(add_len);
	for (int i = 0; i < add_len; i++) {
		add[i] = p_add[i];
	}

	//Encryptation
	mbedtls_gcm_context ctx;
	mbedtls_gcm_init(&ctx);
	mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, 256);

	if (add_len == 0) {
		_err = mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT, input.size(), iv, EXT_SIZE, nullptr, 0, input.data(), output.data(), TAG_SIZE, tag);
		if (_err != 0) {
			mbedtls_strerror(_err, erro, sizeof(erro));
			print_error(erro);
		}
	} else {
		_err = mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT, input.size(), iv, EXT_SIZE, add.data(), add_len, input.data(), output.data(), TAG_SIZE, tag);
		if (_err != 0) {
			mbedtls_strerror(_err, erro, sizeof(erro));
			print_error(erro);
		}
	}

	mbedtls_gcm_free(&ctx);
	PoolByteArray ret_output = char2pool(output.data(), output.size());
	PoolByteArray ret_tag = char2pool(tag, sizeof(tag));
	ret_output.append_array(ret_tag);
	return ret_output;
}

PoolByteArray Cripter::decrypt_byte_GCM(const PoolByteArray p_input, const String p_key, const String p_add) const {
	int _err;
	//Prepare key & iv **
	String h_key = p_key.md5_text();
	uint8_t key[KEY_SIZE];
	uint8_t iv[EXT_SIZE];
	for (int i = 0; i < KEY_SIZE; i++) {
		key[i] = h_key[i];
	}
	for (int i = 0; i < EXT_SIZE; i++) {
		iv[i] = key[i * 2];
	}

	//Preparing Buffer
	char erro[150];
	PoolByteArray ret_output;
	int data_len = p_input.size();
	std::vector<uint8_t> input(data_len - TAG_SIZE);
	std::vector<uint8_t> output(data_len - TAG_SIZE);
	PoolVector<uint8_t>::Read r = p_input.read();
	for (int i = 0; i < (data_len - TAG_SIZE); i++) {
		input[i] = (uint8_t)p_input[i];
	}

	//Extract Tag
	uint8_t tag[TAG_SIZE];
	PoolVector<uint8_t>::Read R = p_input.read();
	for (int i = 0; i < TAG_SIZE; i++) {
		tag[i] = (uint8_t)p_input[(data_len - TAG_SIZE) + i];
	}
	//Prepare Addicional Data
	int add_len = p_add.length();
	std::vector<uint8_t> add(add_len);
	for (int i = 0; i < add_len; i++) {
		add[i] = p_add[i];
	}

	//Decryptation
	mbedtls_gcm_context ctx;
	mbedtls_gcm_init(&ctx);
	mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, 256);

	if (add_len == 0) {
		_err = mbedtls_gcm_auth_decrypt(&ctx, input.size(), iv, EXT_SIZE, nullptr, 0, tag, TAG_SIZE, input.data(), output.data());
		if (_err != 0) {
			mbedtls_strerror(_err, erro, sizeof(erro));
			print_error(erro);
		}
	} else {
		_err = mbedtls_gcm_auth_decrypt(&ctx, input.size(), iv, EXT_SIZE, add.data(), add_len, tag, TAG_SIZE, input.data(), output.data());
		if (_err != 0) {
			mbedtls_strerror(_err, erro, sizeof(erro));
			print_error(erro);
		}
	}

	//Ending
	mbedtls_gcm_free(&ctx);
	return char2pool(output.data(), output.size());
}

//-------------- Simetric - CBC
PoolByteArray Cripter::encrypt_byte_CBC(const PoolByteArray p_input, const String p_key) const {
	int _err;
	//Prepare key & iv **
	String h_key = p_key.md5_text();
	uint8_t key[KEY_SIZE];
	uint8_t iv[EXT_SIZE];
	for (int i = 0; i < KEY_SIZE; i++) {
		key[i] = h_key[i];
	}
	for (int i = 0; i < EXT_SIZE; i++) {
		iv[i] = key[i * 2];
	}

	//Preparing buffer **
	int data_len = p_input.size();
	int extra_len;
	int total_len;
	char erro[150];
	if (data_len % 16) {
		extra_len = (16 - (data_len % 16));
		total_len = data_len + extra_len;
	} else {
		total_len = data_len;
		extra_len = 0;
	}

	uint8_t input[TAG_SIZE];
	uint8_t output[TAG_SIZE];
	for (int g = 0; g < data_len; g++) {
		input[g] = (uint8_t)p_input[g];
	}
	for (int l = data_len; l < total_len; l++) { //fill with zeros couse the input must be multiple of 16
		input[l] = 0;
	}

	//Encryptation **
	mbedtls_aes_context ctx;
	mbedtls_aes_init(&ctx);

	_err = mbedtls_aes_setkey_enc(&ctx, key, 256);
	if (_err != 0) {
		mbedtls_strerror(_err, erro, sizeof(erro));
		print_error(erro);
	}
	_err = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, total_len, iv, input, output);
	if (_err != 0) {
		mbedtls_strerror(_err, erro, sizeof(erro));
		print_error(erro);
	}

	mbedtls_aes_free(&ctx);
	//--- Fit data *
	PoolByteArray ret = char2pool(output, (sizeof(output)));
	ret.push_back(extra_len);
	return ret;
}

PoolByteArray Cripter::decrypt_byte_CBC(const PoolByteArray p_input, const String p_key) const {
	int _err;
	//Prepare key & iv **
	String h_key = p_key.md5_text();
	uint8_t key[KEY_SIZE];
	uint8_t iv[EXT_SIZE];
	for (int i = 0; i < KEY_SIZE; i++) {
		key[i] = h_key[i];
	}
	for (int i = 0; i < EXT_SIZE; i++) {
		iv[i] = key[i * 2];
	}

	//Preparing buffer **
	int data_len = p_input.size() - 1;
	int zeros = p_input[data_len];
	std::vector<uint8_t> input(data_len);
	std::vector<uint8_t> output(data_len);
	char erro[150];
	for (int g = 0; g < data_len; g++) {
		input[g] = (uint8_t)p_input[g];
	}

	//Decryptation **
	mbedtls_aes_context ctx;
	mbedtls_aes_init(&ctx);
	_err = mbedtls_aes_setkey_dec(&ctx, key, 256);
	if (_err != 0) {
		mbedtls_strerror(_err, erro, sizeof(erro));
		print_error(erro);
	}
	_err = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, data_len, iv, input.data(), output.data());
	if (_err != 0) {
		mbedtls_strerror(_err, erro, sizeof(erro));
		print_error(erro);
	}
	mbedtls_aes_free(&ctx);
	//Fit data **
	return char2pool(output.data(), (output.size() - zeros)); //No more extra zeros here
}

//-------------- Asymmetric - RSA
PoolByteArray Cripter::encrypt_byte_RSA(const PoolByteArray p_input, String p_key_path) const {
	int _err;
	//--- Load key
	std::vector<char> key(p_key_path.length() + 1);
	for (int i = 0; i < p_key_path.length(); i++) {
		key[i] = p_key_path[i];
	}
	key[p_key_path.length()] = 0;
	//---Buffer
	size_t olen = 0;
	const char *pers = "rsa_encrypt";
	char erro[150];
	std::vector<uint8_t> input(p_input.size());
	uint8_t output[512];
	PoolVector<uint8_t>::Read r = p_input.read();
	for (unsigned int i = 0; i < input.size(); i++) {
		input[i] = (uint8_t)p_input[i];
	}

	//---Init
	mbedtls_pk_context pk;
	mbedtls_pk_init(&pk);
	mbedtls_entropy_context entropy;
	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ctr_drbg_init(&ctr_drbg);
	_err = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func,
			&entropy, (const unsigned char *)pers,
			strlen(pers));
	if (_err != 0) {
		mbedtls_strerror(_err, erro, sizeof(erro));
		print_error(erro);
	}

	//---Encryptation
	_err = mbedtls_pk_parse_public_keyfile(&pk, key.data());
	if (_err != 0) {
		mbedtls_strerror(_err, erro, sizeof(erro));
		print_error(erro);
		//	printf( "%c", erro );
	}

	fflush(stdout);
	_err = mbedtls_pk_encrypt(&pk, input.data(), input.size(),
			output, &olen, sizeof(output),
			mbedtls_ctr_drbg_random, &ctr_drbg);
	if (_err != 0) {
		mbedtls_strerror(_err, erro, sizeof(erro));
		print_error(erro);
	}

	//--- Fit data
	mbedtls_pk_free(&pk);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);
	return char2pool(output, olen);
}

PoolByteArray Cripter::decrypt_byte_RSA(const PoolByteArray p_input, const String p_key_path, const String p_password) const {
	int _err;
	//--- Load key
	std::vector<char> key(p_key_path.length() + 1);
	for (int i = 0; i < p_key_path.length(); i++) {
		key[i] = p_key_path[i];
	}
	key[p_key_path.length()] = 0;
	//--- Load Password
	std::vector<char> password(p_password.length() + 1);
	for (int i = 0; i < p_password.length(); i++) {
		password[i] = p_password[i];
	}
	password[p_password.length()] = 0;

	//---Buffer
	uint8_t input[512];
	uint8_t output[512];
	size_t olen = 0;
	const char *pers = "rsa_decrypt";
	char erro[150];
	PoolVector<uint8_t>::Read r = p_input.read();
	for (int i = 0; i < 512; i++) {
		input[i] = (uint8_t)p_input[i];
	}

	//---Init
	mbedtls_pk_context ctx_pk;
	mbedtls_pk_init(&ctx_pk);
	mbedtls_entropy_context entropy;
	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ctr_drbg_init(&ctr_drbg);
	_err = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func,
			&entropy, (const unsigned char *)pers,
			strlen(pers));
	if (_err != 0) {
		mbedtls_strerror(_err, erro, sizeof(erro));
		print_error(erro);
	}
	//---Decryptation **
	_err = mbedtls_pk_parse_keyfile(&ctx_pk, key.data(), password.data());
	if (_err != 0) {
		mbedtls_strerror(_err, erro, sizeof(erro));
		print_error(erro);
	}

	fflush(stdout);
	_err = mbedtls_pk_decrypt(&ctx_pk, input, sizeof(input),
			output, &olen, sizeof(output),
			mbedtls_ctr_drbg_random, &ctr_drbg);
	if (_err != 0) {
		mbedtls_strerror(_err, erro, sizeof(erro));
		print_error(erro);
	}

	//---Turn off the lights **
	mbedtls_pk_free(&ctx_pk);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);
	return char2pool(output, olen);
}

//-------------- Support
PoolByteArray Cripter::char2pool(const uint8_t *p_in, const size_t p_size) const {
	PoolByteArray data;
	data.resize(p_size);
	PoolVector<uint8_t>::Write w = data.write();
	for (unsigned int i = 0; i < p_size; i++) {
		w[i] = (uint8_t)p_in[i];
	}
	w = PoolVector<uint8_t>::Write();
	return data;
}

PoolByteArray Cripter::encode_var(const Variant data) const {
	//Encoder
	PoolByteArray ret;
	int len;
	Error err = encode_variant(data, nullptr, len);
	if (err != OK) {
		print_line("Unexpected error encoding variable to bytes");
		return ret;
	}
	ret.resize(len);
	{
		PoolByteArray::Write w = ret.write();
		encode_variant(data, w.ptr(), len);
	}
	return ret;
}

Variant Cripter::decode_var(const PoolByteArray p_data) const {
	//Decoder
	Variant ret;
	PoolByteArray data = p_data;
	PoolByteArray::Read r = data.read();
	Error err = decode_variant(ret, r.ptr(), data.size(), nullptr);
	if (err != OK) {
		print_line("Unexpected error decoding bytes to variable");
		Variant f;
		return f;
	}
	return ret;
}

void Cripter::_bind_methods() {
	//CBC
	ClassDB::bind_method(D_METHOD("encrypt_byte_CBC", "encrypt_data", "key"), &Cripter::encrypt_byte_CBC);
	ClassDB::bind_method(D_METHOD("decrypt_byte_CBC", "decrypt_data", "key"), &Cripter::decrypt_byte_CBC);
	ClassDB::bind_method(D_METHOD("encrypt_var_CBC", "encrypt_data", "key"), &Cripter::encrypt_var_CBC);
	ClassDB::bind_method(D_METHOD("decrypt_var_CBC", "decrypt_data", "key"), &Cripter::decrypt_var_CBC);
	//GCM
	ClassDB::bind_method(D_METHOD("encrypt_byte_GCM", "encrypt_data", "key", "additional_data"), &Cripter::encrypt_byte_GCM);
	ClassDB::bind_method(D_METHOD("decrypt_byte_GCM", "decrypt_data", "key", "additional_data"), &Cripter::decrypt_byte_GCM);
	ClassDB::bind_method(D_METHOD("encrypt_var_GCM", "encrypt_data", "key", "additional_data"), &Cripter::encrypt_var_GCM);
	ClassDB::bind_method(D_METHOD("decrypt_var_GCM", "decrypt_data", "key", "additional_data"), &Cripter::decrypt_var_GCM);
	//RSA
	ClassDB::bind_method(D_METHOD("encrypt_byte_RSA", "encrypt_data", "key_path"), &Cripter::encrypt_byte_RSA);
	ClassDB::bind_method(D_METHOD("decrypt_byte_RSA", "decrypt_data", "key_path", "password"), &Cripter::decrypt_byte_RSA);
	ClassDB::bind_method(D_METHOD("encrypt_var_RSA", "encrypt_data", "key_path"), &Cripter::encrypt_var_RSA);
	ClassDB::bind_method(D_METHOD("decrypt_var_RSA", "decrypt_data", "key_path", "password"), &Cripter::decrypt_var_RSA);
}

Cripter::Cripter() {
}
