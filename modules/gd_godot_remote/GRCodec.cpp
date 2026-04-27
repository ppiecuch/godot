/* GRCodec.cpp */

#include "GRCodec.h"
#include "GRUtils.h"
#include "jpge.h"

#include "core/project_settings.h"

#ifdef GODOT_REMOTE_LIBJPEG_TURBO_ENABLED
#include "turbojpeg.h"
#endif

#include "core/math/math_funcs.h"

using namespace GRUtils;

// =========================================================================
// GRCodec base — benchmark and test image generation
// =========================================================================

PoolByteArray GRCodec::_make_test_image(int w, int h) {
	PoolByteArray data;
	data.resize(w * h * 4);
	PoolByteArray::Write wd = data.write();
	uint32_t seed = 0x12345678;
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int idx = (y * w + x) * 4;
			// Simple pseudo-random with gradient (simulates real content)
			seed = seed * 1103515245 + 12345;
			wd[idx + 0] = (x * 255 / w + (seed >> 16)) & 0xFF;
			wd[idx + 1] = (y * 255 / h + (seed >> 8)) & 0xFF;
			wd[idx + 2] = ((x + y) * 128 / (w + h) + seed) & 0xFF;
			wd[idx + 3] = 255;
		}
	}
	return data;
}

GRCodec::BenchmarkResult GRCodec::benchmark(int test_width, int test_height) {
	BenchmarkResult result;
	if (!is_available())
		return result;

	PoolByteArray img = _make_test_image(test_width, test_height);
	CompressParams params;
	params.quality = 75;

	// Encode benchmark — suppress errors for codecs that may not be fully implemented
	PoolByteArray compressed;
	_print_error_enabled = false;
	uint64_t t0 = OS::get_singleton()->get_ticks_usec();
	Error err = compress(img, test_width, test_height, 4, params, compressed);
	uint64_t t1 = OS::get_singleton()->get_ticks_usec();
	_print_error_enabled = true;

	if (err != OK) {
		result.score = 1e9f;
		return result;
	}

	result.encode_time_ms = (t1 - t0) / 1000.0f;
	result.compressed_size = compressed.size();

	// Decode benchmark
	Ref<Image> decoded;
	decoded.instance();
	uint64_t t2 = OS::get_singleton()->get_ticks_usec();
	decompress(compressed, decoded);
	uint64_t t3 = OS::get_singleton()->get_ticks_usec();
	result.decode_time_ms = (t3 - t2) / 1000.0f;

	// Score: encode speed matters most for streaming
	result.score = result.encode_time_ms * 0.7f + (result.compressed_size / 1000.0f) * 0.3f;
	return result;
}

// =========================================================================
// GRCodecManager
// =========================================================================

GRCodecManager *GRCodecManager::singleton = nullptr;

GRCodecManager::GRCodecManager() {
	singleton = this;
}

GRCodecManager::~GRCodecManager() {
	deinit();
	if (singleton == this)
		singleton = nullptr;
}

GRCodecManager *GRCodecManager::get_singleton() {
	return singleton;
}

void GRCodecManager::_register_codec(GRCodec *codec, int compression_type) {
	CodecEntry entry;
	entry.codec = codec;
	entry.benchmarked = false;
	// Ensure vector is large enough
	while (_codecs.size() <= compression_type) {
		CodecEntry empty;
		_codecs.push_back(empty);
	}
	_codecs.write[compression_type] = entry;
}

void GRCodecManager::init() {
	// Register built-in codecs
	_register_codec(memnew(GRCodecJpge), __COMPRESSION_JPG);
	_register_codec(memnew(GRCodecPNG), __COMPRESSION_PNG);

#ifdef GODOT_REMOTE_LIBJPEG_TURBO_ENABLED
	GRCodecJpegTurbo *turbo = memnew(GRCodecJpegTurbo);
	if (turbo->is_available()) {
		// Turbo replaces jpge for the JPG slot
		memdelete(_codecs.write[__COMPRESSION_JPG].codec);
		_codecs.write[__COMPRESSION_JPG].codec = turbo;
		print_verbose("GodotRemote: libjpeg-turbo available — using as JPEG codec");
	} else {
		memdelete(turbo);
	}
#endif

	// H.264 (dynamic loading — may or may not be available)
	GRCodecH264 *h264 = memnew(GRCodecH264);
	if (h264->is_available()) {
		_register_codec(h264, __COMPRESSION_H264);
		print_verbose("GodotRemote: OpenH264 available — H.264 codec enabled");
	} else {
		memdelete(h264);
		print_verbose("GodotRemote: OpenH264 not found — H.264 disabled");
	}

	// Default to JPG
	_active_compression_type = __COMPRESSION_JPG;
	if (__COMPRESSION_JPG < _codecs.size() && _codecs[__COMPRESSION_JPG].codec) {
		_active_codec = _codecs[__COMPRESSION_JPG].codec;
	}
}

void GRCodecManager::deinit() {
	for (int i = 0; i < _codecs.size(); i++) {
		if (_codecs[i].codec) {
			_codecs[i].codec->finalize();
			memdelete(_codecs[i].codec);
			_codecs.write[i].codec = nullptr;
		}
	}
	_codecs.clear();
	_active_codec = nullptr;
}

void GRCodecManager::run_benchmarks() {
	print_verbose("GodotRemote: Running codec benchmarks...");
	for (int i = 0; i < _codecs.size(); i++) {
		if (_codecs[i].codec && _codecs[i].codec->is_available()) {
			_codecs.write[i].bench = _codecs[i].codec->benchmark();
			_codecs.write[i].benchmarked = true;
			print_verbose(vformat("  %s: encode=%.1fms decode=%.1fms size=%d score=%.1f",
					_codecs[i].codec->get_name(),
					_codecs[i].bench.encode_time_ms,
					_codecs[i].bench.decode_time_ms,
					_codecs[i].bench.compressed_size,
					_codecs[i].bench.score));
		}
	}
}

void GRCodecManager::auto_select_best_codec() {
	float best_score = 1e9f;
	int best_type = __COMPRESSION_JPG;

	for (int i = 0; i < _codecs.size(); i++) {
		if (!_codecs[i].codec || !_codecs[i].benchmarked)
			continue;
		float score = _codecs[i].bench.score;
		// H.264 gets a bonus for inter-frame compression
		if (_codecs[i].codec->get_type() == GRCodec::CODEC_H264) {
			score *= 0.4f;
		}
		if (score < best_score) {
			best_score = score;
			best_type = i;
		}
	}

	set_active_codec(best_type);
	if (_active_codec) {
		print_verbose(vformat("GodotRemote: Auto-selected codec: %s (score=%.1f)",
				_active_codec->get_name(), best_score));
	}
}

Error GRCodecManager::set_active_codec(int compression_type) {
	if (compression_type < 0 || compression_type >= _codecs.size()) {
		return ERR_INVALID_PARAMETER;
	}
	if (!_codecs[compression_type].codec || !_codecs[compression_type].codec->is_available()) {
		return ERR_UNAVAILABLE;
	}
	_active_compression_type = compression_type;
	_active_codec = _codecs[compression_type].codec;
	return OK;
}

GRCodec *GRCodecManager::get_active_codec() const { return _active_codec; }
int GRCodecManager::get_active_compression_type() const { return _active_compression_type; }

Error GRCodecManager::compress(PoolByteArray &r_output, const PoolByteArray &img_data,
		int width, int height, int bytes_per_pixel, int quality, int subsampling) {
	ERR_FAIL_COND_V(!_active_codec, ERR_UNCONFIGURED);
	GRCodec::CompressParams params;
	params.quality = quality;
	params.subsampling = subsampling;
	return _active_codec->compress(img_data, width, height, bytes_per_pixel, params, r_output);
}

Error GRCodecManager::decompress(int compression_type, const PoolByteArray &data,
		Ref<Image> &r_output) {
	GRCodec *codec = get_codec_for_type(compression_type);
	ERR_FAIL_COND_V(!codec, ERR_UNAVAILABLE);
	return codec->decompress(data, r_output);
}

Array GRCodecManager::get_available_codecs() const {
	Array result;
	for (int i = 0; i < _codecs.size(); i++) {
		if (_codecs[i].codec && _codecs[i].codec->is_available()) {
			result.push_back(_codecs[i].codec->get_name());
		}
	}
	return result;
}

Dictionary GRCodecManager::get_benchmark_results() const {
	Dictionary result;
	for (int i = 0; i < _codecs.size(); i++) {
		if (_codecs[i].codec && _codecs[i].benchmarked) {
			Dictionary entry;
			entry["encode_ms"] = _codecs[i].bench.encode_time_ms;
			entry["decode_ms"] = _codecs[i].bench.decode_time_ms;
			entry["compressed_size"] = _codecs[i].bench.compressed_size;
			entry["score"] = _codecs[i].bench.score;
			result[_codecs[i].codec->get_name()] = entry;
		}
	}
	return result;
}

GRCodec *GRCodecManager::get_codec_for_type(int compression_type) const {
	if (compression_type < 0 || compression_type >= _codecs.size())
		return nullptr;
	return _codecs[compression_type].codec;
}

// =========================================================================
// GRCodecJpge — built-in lightweight JPEG encoder
// =========================================================================

Error GRCodecJpge::compress(const PoolByteArray &img_data, int width, int height,
		int bytes_per_pixel, const CompressParams &params, PoolByteArray &r_output) {
	ERR_FAIL_COND_V(img_data.size() == 0, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(params.quality < 1 || params.quality > 100, ERR_INVALID_PARAMETER);

	jpge::params jp;
	jp.m_quality = params.quality;
	jp.m_subsampling = (jpge::subsampling_t)params.subsampling;
	ERR_FAIL_COND_V(!jp.check(), ERR_INVALID_PARAMETER);

	// Allocate output buffer (worst case: input size)
	int buf_size = img_data.size();
	PoolByteArray buffer;
	buffer.resize(buf_size);

	auto ri = img_data.read();
	auto wb = buffer.write();

	bool ok = jpge::compress_image_to_jpeg_file_in_memory(
			(void *)wb.ptr(), buf_size, width, height,
			bytes_per_pixel, (const unsigned char *)ri.ptr(), jp);

	ERR_FAIL_COND_V_MSG(!ok, FAILED, "jpge: compression failed");

	r_output.resize(buf_size);
	auto wo = r_output.write();
	memcpy(wo.ptr(), wb.ptr(), buf_size);

	return OK;
}

Error GRCodecJpge::decompress(const PoolByteArray &compressed_data, Ref<Image> &r_output_image) {
	if (r_output_image.is_null())
		r_output_image.instance();
	return r_output_image->load_jpg_from_buffer(compressed_data);
}

// =========================================================================
// GRCodecPNG — Godot built-in PNG codec
// =========================================================================

Error GRCodecPNG::compress(const PoolByteArray &img_data, int width, int height,
		int bytes_per_pixel, const CompressParams &params, PoolByteArray &r_output) {
	Ref<Image> img;
	img.instance();
	Image::Format fmt = (bytes_per_pixel == 3) ? Image::FORMAT_RGB8 : Image::FORMAT_RGBA8;
	img->create(width, height, false, fmt, img_data);
	ERR_FAIL_COND_V(img->empty(), FAILED);
	r_output = img->save_png_to_buffer();
	return r_output.size() > 0 ? OK : FAILED;
}

Error GRCodecPNG::decompress(const PoolByteArray &compressed_data, Ref<Image> &r_output_image) {
	if (r_output_image.is_null())
		r_output_image.instance();
	return r_output_image->load_png_from_buffer(compressed_data);
}

// =========================================================================
// GRCodecJpegTurbo — high-performance JPEG (conditional)
// =========================================================================

#ifdef GODOT_REMOTE_LIBJPEG_TURBO_ENABLED

Error GRCodecJpegTurbo::compress(const PoolByteArray &img_data, int width, int height,
		int bytes_per_pixel, const CompressParams &params, PoolByteArray &r_output) {
	ERR_FAIL_COND_V(img_data.size() == 0, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(params.quality < 1 || params.quality > 100, ERR_INVALID_PARAMETER);

	tjhandle compressor = tjInitCompress();
	ERR_FAIL_COND_V(!compressor, FAILED);

	auto ri = img_data.read();
	unsigned char *out_buf = nullptr;
	unsigned long out_size = 0;

	int pixel_format = (bytes_per_pixel == 3) ? TJPF_RGB : TJPF_RGBA;
	int err = tjCompress2(compressor, ri.ptr(), width, 0, height,
			pixel_format, &out_buf, &out_size,
			TJSAMP_420, params.quality, TJFLAG_FASTDCT);

	if (err) {
		String error_str = tjGetErrorStr2(compressor);
		tjDestroy(compressor);
		ERR_FAIL_V_MSG(FAILED, "jpeg-turbo: " + error_str);
	}

	tjDestroy(compressor);

	r_output.resize(out_size);
	{
		auto wo = r_output.write();
		memcpy(wo.ptr(), out_buf, out_size);
	}
	tjFree(out_buf);

	return OK;
}

Error GRCodecJpegTurbo::decompress(const PoolByteArray &compressed_data, Ref<Image> &r_output_image) {
	ERR_FAIL_COND_V(compressed_data.size() == 0, ERR_INVALID_PARAMETER);

	tjhandle decompressor = tjInitDecompress();
	ERR_FAIL_COND_V(!decompressor, FAILED);

	auto ri = compressed_data.read();
	int width, height, subsamp;

	int err = tjDecompressHeader2(decompressor,
			const_cast<unsigned char *>(ri.ptr()), compressed_data.size(),
			&width, &height, &subsamp);
	if (err) {
		String error_str = tjGetErrorStr2(decompressor);
		tjDestroy(decompressor);
		ERR_FAIL_V_MSG(FAILED, "jpeg-turbo decompress header: " + error_str);
	}

	int out_size = width * height * 3;
	PoolByteArray out_data;
	out_data.resize(out_size);

	{
		auto wo = out_data.write();
		err = tjDecompress2(decompressor,
				const_cast<unsigned char *>(ri.ptr()), compressed_data.size(),
				wo.ptr(), width, 0, height, TJPF_RGB, TJFLAG_FASTDCT);
	}

	tjDestroy(decompressor);

	if (err) {
		return FAILED;
	}

	if (r_output_image.is_null())
		r_output_image.instance();
	r_output_image->create(width, height, false, Image::FORMAT_RGB8, out_data);
	return OK;
}

#endif // GODOT_REMOTE_LIBJPEG_TURBO_ENABLED

// =========================================================================
// GRCodecH264 — OpenH264 with dynamic loading
// =========================================================================

#include "deps/openh264/include/codec_api.h"
#include "deps/openh264/include/codec_app_def.h"
#include "deps/openh264/include/codec_def.h"

// Function pointer typedefs matching the OpenH264 C API
typedef int (*CreateEncoderFunc)(ISVCEncoder **ppEncoder);
typedef void (*DestroyEncoderFunc)(ISVCEncoder *pEncoder);
typedef long (*CreateDecoderFunc)(ISVCDecoder **ppDecoder);
typedef void (*DestroyDecoderFunc)(ISVCDecoder *pDecoder);

// Required symbols from the OpenH264 shared library
static const char *OPENH264_SYMBOL_CREATE_ENCODER = "WelsCreateSVCEncoder";
static const char *OPENH264_SYMBOL_DESTROY_ENCODER = "WelsDestroySVCEncoder";
static const char *OPENH264_SYMBOL_CREATE_DECODER = "WelsCreateDecoder";
static const char *OPENH264_SYMBOL_DESTROY_DECODER = "WelsDestroyDecoder";

// ---------------------------------------------------------------------------
// RGBA ↔ YUV I420 conversion helpers (BT.601)
// ---------------------------------------------------------------------------

static void _rgba_to_yuv420(const uint8_t *rgba, int width, int height, uint8_t *y_out, uint8_t *u_out, uint8_t *v_out) {
	int y_stride = width;
	int uv_stride = width / 2;

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			int idx = (j * width + i) * 4;
			uint8_t r = rgba[idx + 0];
			uint8_t g = rgba[idx + 1];
			uint8_t b = rgba[idx + 2];

			// BT.601 RGB→Y
			int y_val = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
			y_out[j * y_stride + i] = CLAMP(y_val, 0, 255);

			// Subsample U/V at 2x2 blocks (top-left pixel)
			if ((j & 1) == 0 && (i & 1) == 0) {
				int u_val = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
				int v_val = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
				u_out[(j / 2) * uv_stride + (i / 2)] = CLAMP(u_val, 0, 255);
				v_out[(j / 2) * uv_stride + (i / 2)] = CLAMP(v_val, 0, 255);
			}
		}
	}
}

static void _yuv420_to_rgb(const uint8_t *y_in, const uint8_t *u_in, const uint8_t *v_in,
		int y_stride, int uv_stride, int width, int height, uint8_t *rgb_out) {
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			int y_val = y_in[j * y_stride + i];
			int u_val = u_in[(j / 2) * uv_stride + (i / 2)];
			int v_val = v_in[(j / 2) * uv_stride + (i / 2)];

			// BT.601 YUV→RGB
			int c = y_val - 16;
			int d = u_val - 128;
			int e = v_val - 128;
			int r = (298 * c + 409 * e + 128) >> 8;
			int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
			int b = (298 * c + 516 * d + 128) >> 8;

			int out_idx = (j * width + i) * 3;
			rgb_out[out_idx + 0] = CLAMP(r, 0, 255);
			rgb_out[out_idx + 1] = CLAMP(g, 0, 255);
			rgb_out[out_idx + 2] = CLAMP(b, 0, 255);
		}
	}
}

GRCodecH264::GRCodecH264() {}

GRCodecH264::~GRCodecH264() {
	finalize();
}

Vector<String> GRCodecH264::_get_search_paths() const {
	Vector<String> paths;

	// Platform-specific library filename
#if defined(__APPLE__)
#if defined(__aarch64__)
	String lib_name = "libopenh264-2.2.0-osx-arm64.6.dylib";
#else
	String lib_name = "libopenh264-2.2.0-osx-x64.6.dylib";
#endif
#elif defined(__linux__)
#if defined(__x86_64__)
	String lib_name = "libopenh264-2.2.0-linux64.6.so";
#else
	String lib_name = "libopenh264-2.2.0-linux32.6.so";
#endif
#elif defined(_WIN32)
#if defined(_WIN64)
	String lib_name = "openh264-2.2.0-win64.dll";
#else
	String lib_name = "openh264-2.2.0-win32.dll";
#endif
#elif defined(__ANDROID__)
#if defined(__aarch64__)
	String lib_name = "libopenh264-2.2.0-android-arm64.6.so";
#elif defined(__arm__)
	String lib_name = "libopenh264-2.2.0-android-arm.6.so";
#elif defined(__x86_64__)
	String lib_name = "libopenh264-2.2.0-android-x64.6.so";
#else
	String lib_name = "libopenh264-2.2.0-android-x86.6.so";
#endif
#else
	String lib_name = "";
#endif

	if (lib_name.empty())
		return paths;

	// 1. Next to the executable
	String exe_dir = OS::get_singleton()->get_executable_path().get_base_dir();
	paths.push_back(exe_dir.plus_file(lib_name));

	// 2. Module deps/ directory (globalized path)
	String deps_dir = ProjectSettings::get_singleton()->globalize_path(
			"res://modules/gd_godot_remote/deps/openh264");
	paths.push_back(deps_dir.plus_file(lib_name));

	// 3. Just the library name (system search paths)
	paths.push_back(lib_name);

	return paths;
}

bool GRCodecH264::_load_library() {
	if (_lib_handle)
		return true;

	Vector<String> paths = _get_search_paths();

	// Suppress Godot's ERR_PRINT from open_dynamic_library failures —
	// these are expected when probing for optional libraries.
	_print_error_enabled = false;
	for (int i = 0; i < paths.size(); i++) {
		Error err = OS::get_singleton()->open_dynamic_library(paths[i], _lib_handle);
		if (err == OK && _lib_handle) {
			_print_error_enabled = true;
			static bool _logged_once = false;
			if (!_logged_once) {
				print_verbose("GodotRemote: OpenH264 loaded from: " + paths[i]);
				_logged_once = true;
			}
			return true;
		}
	}
	_print_error_enabled = true;

	_lib_handle = nullptr;
	return false;
}

bool GRCodecH264::_resolve_symbols() {
	ERR_FAIL_COND_V(!_lib_handle, false);

	Error err;
	err = OS::get_singleton()->get_dynamic_library_symbol_handle(_lib_handle, OPENH264_SYMBOL_CREATE_ENCODER, _fn_create_encoder);
	if (err != OK || !_fn_create_encoder) {
		print_verbose("GodotRemote: OpenH264 missing symbol: " + String(OPENH264_SYMBOL_CREATE_ENCODER));
		return false;
	}

	err = OS::get_singleton()->get_dynamic_library_symbol_handle(_lib_handle, OPENH264_SYMBOL_DESTROY_ENCODER, _fn_destroy_encoder);
	if (err != OK || !_fn_destroy_encoder) {
		print_verbose("GodotRemote: OpenH264 missing symbol: " + String(OPENH264_SYMBOL_DESTROY_ENCODER));
		return false;
	}

	err = OS::get_singleton()->get_dynamic_library_symbol_handle(_lib_handle, OPENH264_SYMBOL_CREATE_DECODER, _fn_create_decoder);
	if (err != OK || !_fn_create_decoder) {
		print_verbose("GodotRemote: OpenH264 missing symbol: " + String(OPENH264_SYMBOL_CREATE_DECODER));
		return false;
	}

	err = OS::get_singleton()->get_dynamic_library_symbol_handle(_lib_handle, OPENH264_SYMBOL_DESTROY_DECODER, _fn_destroy_decoder);
	if (err != OK || !_fn_destroy_decoder) {
		print_verbose("GodotRemote: OpenH264 missing symbol: " + String(OPENH264_SYMBOL_DESTROY_DECODER));
		return false;
	}

	return true;
}

void GRCodecH264::_unload_library() {
	_fn_create_encoder = nullptr;
	_fn_destroy_encoder = nullptr;
	_fn_create_decoder = nullptr;
	_fn_destroy_decoder = nullptr;

	if (_lib_handle) {
		OS::get_singleton()->close_dynamic_library(_lib_handle);
		_lib_handle = nullptr;
	}
}

bool GRCodecH264::is_available() const {
	if (_probed)
		return _probe_result;

	// Probe: load library, resolve symbols, then unload
	GRCodecH264 *self = const_cast<GRCodecH264 *>(this);
	self->_probed = true;
	self->_probe_result = false;

	if (!self->_load_library())
		return false;

	if (!self->_resolve_symbols()) {
		self->_unload_library();
		return false;
	}

	// All symbols resolved — library is usable. Keep loaded to avoid
	// redundant load/unload cycles and duplicate "loaded from" messages.
	self->_probe_result = true;
	return true;
}

bool GRCodecH264::_ensure_encoder(int width, int height, const CompressParams &params) {
	// Re-initialize if resolution changed
	if (_encoder_initialized && (_current_width != width || _current_height != height)) {
		ISVCEncoder *enc = static_cast<ISVCEncoder *>(_encoder);
		enc->Uninitialize();
		_encoder_initialized = false;
	}

	if (_encoder_initialized)
		return true;
	if (!_load_library() || !_resolve_symbols())
		return false;

	// Create encoder instance
	if (!_encoder) {
		CreateEncoderFunc create = reinterpret_cast<CreateEncoderFunc>(_fn_create_encoder);
		ISVCEncoder *enc = nullptr;
		int rv = create(&enc);
		if (rv != 0 || !enc)
			return false;
		_encoder = enc;
	}

	ISVCEncoder *enc = static_cast<ISVCEncoder *>(_encoder);

	// Configure encoder parameters
	SEncParamExt param;
	memset(&param, 0, sizeof(param));
	enc->GetDefaultParams(&param);

	param.iUsageType = SCREEN_CONTENT_REAL_TIME;
	param.iPicWidth = width;
	param.iPicHeight = height;
	param.fMaxFrameRate = 30.0f;
	param.iTargetBitrate = params.bitrate > 0 ? params.bitrate : 500000;
	param.iRCMode = RC_QUALITY_MODE;
	param.iMultipleThreadIdc = 1;
	param.bEnableFrameSkip = true;
	param.bEnableAdaptiveQuant = false;
	param.bEnableBackgroundDetection = false;
	param.iSpatialLayerNum = 1;

	param.sSpatialLayers[0].iVideoWidth = width;
	param.sSpatialLayers[0].iVideoHeight = height;
	param.sSpatialLayers[0].fFrameRate = 30.0f;
	param.sSpatialLayers[0].iSpatialBitrate = param.iTargetBitrate;
	param.sSpatialLayers[0].uiProfileIdc = PRO_BASELINE;
	param.sSpatialLayers[0].uiLevelIdc = LEVEL_5_1;

	int rv = enc->InitializeExt(&param);
	if (rv != 0) {
		print_verbose("GodotRemote: OpenH264 encoder InitializeExt failed: " + itos(rv));
		return false;
	}

	// Suppress OpenH264 library warnings during normal operation
	int trace_level = WELS_LOG_ERROR;
	enc->SetOption(ENCODER_OPTION_TRACE_LEVEL, &trace_level);

	int video_format = videoFormatI420;
	enc->SetOption(ENCODER_OPTION_DATAFORMAT, &video_format);

	_current_width = width;
	_current_height = height;
	_frame_count = 0;
	_encoder_initialized = true;
	return true;
}

bool GRCodecH264::_ensure_decoder() {
	if (_decoder_initialized)
		return true;
	if (!_load_library() || !_resolve_symbols())
		return false;

	if (!_decoder) {
		CreateDecoderFunc create = reinterpret_cast<CreateDecoderFunc>(_fn_create_decoder);
		ISVCDecoder *dec = nullptr;
		long rv = create(&dec);
		if (rv != 0 || !dec)
			return false;
		_decoder = dec;
	}

	ISVCDecoder *dec = static_cast<ISVCDecoder *>(_decoder);

	SDecodingParam dparam;
	memset(&dparam, 0, sizeof(dparam));
	dparam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;

	long rv = dec->Initialize(&dparam);
	if (rv != 0) {
		print_verbose("GodotRemote: OpenH264 decoder Initialize failed: " + itos(rv));
		return false;
	}

	int trace_level = WELS_LOG_ERROR;
	dec->SetOption(DECODER_OPTION_TRACE_LEVEL, &trace_level);

	_decoder_initialized = true;
	return true;
}

Error GRCodecH264::initialize(int width, int height) {
	CompressParams default_params;
	if (!_ensure_encoder(width, height, default_params)) {
		return ERR_UNAVAILABLE;
	}
	if (!_ensure_decoder()) {
		return ERR_UNAVAILABLE;
	}
	return OK;
}

void GRCodecH264::finalize() {
	if (_encoder) {
		ISVCEncoder *enc = static_cast<ISVCEncoder *>(_encoder);
		enc->Uninitialize();
		DestroyEncoderFunc destroy = reinterpret_cast<DestroyEncoderFunc>(_fn_destroy_encoder);
		if (destroy)
			destroy(enc);
		_encoder = nullptr;
	}
	if (_decoder) {
		ISVCDecoder *dec = static_cast<ISVCDecoder *>(_decoder);
		dec->Uninitialize();
		DestroyDecoderFunc destroy = reinterpret_cast<DestroyDecoderFunc>(_fn_destroy_decoder);
		if (destroy)
			destroy(dec);
		_decoder = nullptr;
	}
	_encoder_initialized = false;
	_decoder_initialized = false;
	_unload_library();
	_current_width = 0;
	_current_height = 0;
	_frame_count = 0;
	_yuv_buffer.resize(0);
}

Error GRCodecH264::compress(const PoolByteArray &img_data, int width, int height,
		int bytes_per_pixel, const CompressParams &params, PoolByteArray &r_output) {
	ERR_FAIL_COND_V(img_data.size() == 0, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(width < 2 || height < 2, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(width % 2 != 0 || height % 2 != 0, ERR_INVALID_PARAMETER);

	if (!_ensure_encoder(width, height, params)) {
		ERR_FAIL_V_MSG(ERR_UNAVAILABLE, "OpenH264 encoder not available");
	}

	ISVCEncoder *enc = static_cast<ISVCEncoder *>(_encoder);

	// Convert RGBA → YUV I420
	int yuv_size = width * height * 3 / 2;
	if (_yuv_buffer.size() != yuv_size) {
		_yuv_buffer.resize(yuv_size);
	}

	{
		auto ri = img_data.read();
		auto yw = _yuv_buffer.write();
		uint8_t *y_plane = yw.ptr();
		uint8_t *u_plane = y_plane + width * height;
		uint8_t *v_plane = u_plane + (width / 2) * (height / 2);
		_rgba_to_yuv420(ri.ptr(), width, height, y_plane, u_plane, v_plane);
	}

	// Set up source picture
	SSourcePicture pic;
	memset(&pic, 0, sizeof(pic));
	pic.iColorFormat = videoFormatI420;
	pic.iPicWidth = width;
	pic.iPicHeight = height;
	pic.iStride[0] = width;
	pic.iStride[1] = width / 2;
	pic.iStride[2] = width / 2;
	{
		auto yr = _yuv_buffer.read();
		pic.pData[0] = const_cast<unsigned char *>(yr.ptr());
		pic.pData[1] = pic.pData[0] + width * height;
		pic.pData[2] = pic.pData[1] + (width / 2) * (height / 2);
		pic.uiTimeStamp = _frame_count * 33; // ~30fps in ms

		// Encode
		SFrameBSInfo info;
		memset(&info, 0, sizeof(info));
		int rv = enc->EncodeFrame(&pic, &info);
		ERR_FAIL_COND_V_MSG(rv != cmResultSuccess, FAILED,
				"OpenH264 EncodeFrame failed: " + itos(rv));

		_frame_count++;

		if (info.eFrameType == videoFrameTypeSkip) {
			r_output.resize(0);
			return OK;
		}

		// Extract bitstream from all layers
		int total_size = 0;
		for (int i = 0; i < info.iLayerNum; i++) {
			SLayerBSInfo &layer = info.sLayerInfo[i];
			for (int n = 0; n < layer.iNalCount; n++) {
				total_size += layer.pNalLengthInByte[n];
			}
		}

		r_output.resize(total_size);
		{
			auto wo = r_output.write();
			int offset = 0;
			for (int i = 0; i < info.iLayerNum; i++) {
				SLayerBSInfo &layer = info.sLayerInfo[i];
				int layer_size = 0;
				for (int n = 0; n < layer.iNalCount; n++) {
					layer_size += layer.pNalLengthInByte[n];
				}
				memcpy(wo.ptr() + offset, layer.pBsBuf, layer_size);
				offset += layer_size;
			}
		}
	}

	return OK;
}

Error GRCodecH264::decompress(const PoolByteArray &compressed_data, Ref<Image> &r_output_image) {
	ERR_FAIL_COND_V(compressed_data.size() == 0, ERR_INVALID_PARAMETER);

	if (!_ensure_decoder()) {
		ERR_FAIL_V_MSG(ERR_UNAVAILABLE, "OpenH264 decoder not available");
	}

	ISVCDecoder *dec = static_cast<ISVCDecoder *>(_decoder);

	auto ri = compressed_data.read();
	unsigned char *dst[3] = { nullptr, nullptr, nullptr };
	SBufferInfo buf_info;
	memset(&buf_info, 0, sizeof(buf_info));

	DECODING_STATE rv = dec->DecodeFrameNoDelay(
			ri.ptr(), compressed_data.size(), dst, &buf_info);

	if (rv != dsErrorFree && rv != dsFramePending) {
		ERR_FAIL_V_MSG(FAILED, "OpenH264 decode failed: " + itos(rv));
	}

	if (buf_info.iBufferStatus != 1 || !dst[0]) {
		// No frame ready yet (e.g., decoder needs more data)
		return ERR_SKIP;
	}

	int width = buf_info.UsrData.sSystemBuffer.iWidth;
	int height = buf_info.UsrData.sSystemBuffer.iHeight;
	int y_stride = buf_info.UsrData.sSystemBuffer.iStride[0];
	int uv_stride = buf_info.UsrData.sSystemBuffer.iStride[1];

	// Convert YUV I420 → RGB24
	PoolByteArray rgb_data;
	rgb_data.resize(width * height * 3);
	{
		auto wo = rgb_data.write();
		_yuv420_to_rgb(dst[0], dst[1], dst[2], y_stride, uv_stride,
				width, height, wo.ptr());
	}

	if (r_output_image.is_null())
		r_output_image.instance();
	r_output_image->create(width, height, false, Image::FORMAT_RGB8, rgb_data);
	return OK;
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"
#include "doctest/doctest_godot.h"

static PoolByteArray _test_img(int w, int h) {
	return GRCodec::_make_test_image(w, h);
}

TEST_SUITE("[[gd_godot_remote]] GRCodec") {
	TEST_CASE("[gr_codec] test image generation") {
		PoolByteArray img = _test_img(16, 16);
		CHECK(img.size() == 16 * 16 * 4);
	}

	TEST_CASE("[gr_codec] jpge compress roundtrip") {
		GRCodecJpge codec;
		CHECK(codec.is_available());
		CHECK(codec.get_name() == "jpge");

		PoolByteArray img = _test_img(64, 64);
		PoolByteArray compressed;
		GRCodec::CompressParams params;
		params.quality = 75;

		Error err = codec.compress(img, 64, 64, 4, params, compressed);
		CHECK(err == OK);
		CHECK(compressed.size() > 0);
		CHECK(compressed.size() < img.size());

		Ref<Image> decoded;
		err = codec.decompress(compressed, decoded);
		CHECK(err == OK);
		REQUIRE(decoded.is_valid());
		CHECK(decoded->get_width() == 64);
		CHECK(decoded->get_height() == 64);
	}

	TEST_CASE("[gr_codec] jpge rejects invalid quality") {
		GRCodecJpge codec;
		PoolByteArray img = _test_img(8, 8);
		PoolByteArray out;
		GRCodec::CompressParams params;

		params.quality = 0;
		EXPECT_ERROR(CHECK(codec.compress(img, 8, 8, 4, params, out) != OK));
		params.quality = 101;
		EXPECT_ERROR(CHECK(codec.compress(img, 8, 8, 4, params, out) != OK));
	}

	TEST_CASE("[gr_codec] jpge rejects empty input") {
		GRCodecJpge codec;
		PoolByteArray empty;
		PoolByteArray out;
		GRCodec::CompressParams params;
		EXPECT_ERROR(CHECK(codec.compress(empty, 0, 0, 4, params, out) != OK));
	}

	TEST_CASE("[gr_codec] PNG compress roundtrip") {
		GRCodecPNG codec;
		CHECK(codec.is_available());
		CHECK(codec.get_name() == "PNG");

		PoolByteArray img = _test_img(32, 32);
		PoolByteArray compressed;
		GRCodec::CompressParams params;

		Error err = codec.compress(img, 32, 32, 4, params, compressed);
		CHECK(err == OK);
		CHECK(compressed.size() > 0);

		Ref<Image> decoded;
		err = codec.decompress(compressed, decoded);
		CHECK(err == OK);
		REQUIRE(decoded.is_valid());
		CHECK(decoded->get_width() == 32);
		CHECK(decoded->get_height() == 32);
	}

	TEST_CASE("[gr_codec] jpge benchmark") {
		GRCodecJpge codec;
		auto result = codec.benchmark(64, 64);
		CHECK(result.encode_time_ms >= 0.0f);
		CHECK(result.compressed_size > 0);
		CHECK(result.score < 1e9f);
	}

	TEST_CASE("[gr_codec] PNG benchmark") {
		GRCodecPNG codec;
		auto result = codec.benchmark(64, 64);
		CHECK(result.encode_time_ms >= 0.0f);
		CHECK(result.compressed_size > 0);
	}

#ifdef GODOT_REMOTE_LIBJPEG_TURBO_ENABLED
	TEST_CASE("[gr_codec] jpeg-turbo compress roundtrip") {
		GRCodecJpegTurbo codec;
		CHECK(codec.is_available());

		PoolByteArray img = _test_img(64, 64);
		PoolByteArray compressed;
		GRCodec::CompressParams params;
		params.quality = 75;

		Error err = codec.compress(img, 64, 64, 4, params, compressed);
		CHECK(err == OK);

		Ref<Image> decoded;
		err = codec.decompress(compressed, decoded);
		CHECK(err == OK);
		REQUIRE(decoded.is_valid());
		CHECK(decoded->get_width() == 64);
	}

	TEST_CASE("[gr_codec] jpeg-turbo is faster than jpge") {
		GRCodecJpge jpge;
		GRCodecJpegTurbo turbo;
		auto b_jpge = jpge.benchmark(256, 256);
		auto b_turbo = turbo.benchmark(256, 256);
		CHECK(b_turbo.encode_time_ms < b_jpge.encode_time_ms);
	}
#endif

	TEST_CASE("[gr_codec] H264 availability check") {
		GRCodecH264 codec;
		// May or may not be available depending on library presence
		bool avail = codec.is_available();
		if (avail) {
			CHECK(codec.get_name() == "OpenH264");
		}
	}

	TEST_CASE("[gr_codec] H264 compress roundtrip") {
		GRCodecH264 codec;
		if (!codec.is_available())
			return; // skip if no library

		PoolByteArray img = _test_img(64, 64);
		PoolByteArray compressed;
		GRCodec::CompressParams params;
		params.bitrate = 500000;

		Error err;
		SUPPRESS_OUTPUT(err = codec.compress(img, 64, 64, 4, params, compressed));
		CHECK(err == OK);
		CHECK(compressed.size() > 0);

		// Decode the compressed frame
		Ref<Image> decoded;
		err = codec.decompress(compressed, decoded);
		// First frame may decode immediately or need more data
		if (err == OK) {
			REQUIRE(decoded.is_valid());
			CHECK(decoded->get_width() == 64);
			CHECK(decoded->get_height() == 64);
		}

		codec.finalize();
	}

	TEST_CASE("[gr_codec] H264 initialize and finalize") {
		GRCodecH264 codec;
		if (!codec.is_available())
			return;

		Error err;
		SUPPRESS_OUTPUT(err = codec.initialize(128, 128));
		CHECK(err == OK);
		codec.finalize();
		// Double finalize should be safe
		codec.finalize();
	}

	TEST_CASE("[gr_codec] H264 multi-frame encode") {
		GRCodecH264 codec;
		if (!codec.is_available())
			return;

		GRCodec::CompressParams params;
		params.bitrate = 500000;

		// Encode multiple frames to test stateful encoder
		for (int f = 0; f < 5; f++) {
			PoolByteArray img = _test_img(64, 64);
			PoolByteArray compressed;
			Error err;
			SUPPRESS_OUTPUT(err = codec.compress(img, 64, 64, 4, params, compressed));
			CHECK(err == OK);
			// After first I-frame, subsequent frames should also produce output
			if (f > 0) {
				CHECK(compressed.size() > 0);
			}
		}

		codec.finalize();
	}

	TEST_CASE("[gr_codec] H264 benchmark") {
		GRCodecH264 codec;
		if (!codec.is_available())
			return;

		GRCodec::BenchmarkResult result;
		SUPPRESS_OUTPUT(result = codec.benchmark(64, 64));
		CHECK(result.encode_time_ms >= 0.0f);
		CHECK(result.compressed_size > 0);
		CHECK(result.score < 1e9f);

		codec.finalize();
	}
}

TEST_SUITE("[[gd_godot_remote]] GRCodecManager") {
	TEST_CASE("[gr_codec] manager init registers codecs") {
		GRCodecManager mgr;
		mgr.init();
		Array codecs = mgr.get_available_codecs();
		CHECK(codecs.size() >= 2); // jpge + PNG always available
		mgr.deinit();
	}

	TEST_CASE("[gr_codec] manager auto-select") {
		GRCodecManager mgr;
		mgr.init();
		mgr.run_benchmarks();
		mgr.auto_select_best_codec();
		CHECK(mgr.get_active_codec() != nullptr);
		CHECK(mgr.get_active_codec()->get_name().length() > 0);
		mgr.deinit();
	}

	TEST_CASE("[gr_codec] manager manual codec override") {
		GRCodecManager mgr;
		mgr.init();
		Error err = mgr.set_active_codec(__COMPRESSION_PNG);
		CHECK(err == OK);
		CHECK(mgr.get_active_codec()->get_type() == GRCodec::CODEC_PNG);
		mgr.deinit();
	}

	TEST_CASE("[gr_codec] manager compress via active codec") {
		GRCodecManager mgr;
		mgr.init();

		PoolByteArray img = _test_img(32, 32);
		PoolByteArray out;
		Error err = mgr.compress(out, img, 32, 32, 4);
		CHECK(err == OK);
		CHECK(out.size() > 0);

		mgr.deinit();
	}

	TEST_CASE("[gr_codec] manager decompress via type tag") {
		GRCodecManager mgr;
		mgr.init();

		// Compress as PNG, decompress using type tag
		PoolByteArray img = _test_img(16, 16);
		PoolByteArray compressed;
		GRCodec::CompressParams params;
		GRCodecPNG png;
		CHECK(png.compress(img, 16, 16, 4, params, compressed) == OK);

		Ref<Image> decoded;
		Error err = mgr.decompress(__COMPRESSION_PNG, compressed, decoded);
		CHECK(err == OK);
		REQUIRE(decoded.is_valid());
		CHECK(decoded->get_width() == 16);

		mgr.deinit();
	}

	TEST_CASE("[gr_codec] manager benchmark results") {
		GRCodecManager mgr;
		mgr.init();
		mgr.run_benchmarks();
		Dictionary results = mgr.get_benchmark_results();
		CHECK(results.size() >= 2);
		mgr.deinit();
	}

	TEST_CASE("[gr_codec] manager invalid codec type") {
		GRCodecManager mgr;
		mgr.init();
		EXPECT_ERROR(CHECK(mgr.set_active_codec(99) != OK));
		mgr.deinit();
	}
}

#endif // DOCTEST
