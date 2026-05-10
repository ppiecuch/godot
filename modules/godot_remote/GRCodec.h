/**************************************************************************/
/*  GRCodec.h                                                             */
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

/* GRCodec.h */
#pragma once

#include "core/image.h"
#include "core/os/os.h"
#include "core/pool_vector.h"
#include "core/reference.h"
#include "core/ustring.h"

#include "GRLiterals.h"

// =========================================================================
// GRCodec — Abstract base for image compression codecs
// =========================================================================

class GRCodec {
public:
	enum Type {
		CODEC_JPGE,
		CODEC_JPEG_TURBO,
		CODEC_PNG,
		CODEC_H264,
	};

	struct CompressParams {
		int quality = 75;
		int subsampling = __SUBSAMPLING_H2V2;
		int bitrate = 500000;
	};

	struct BenchmarkResult {
		float encode_time_ms = 0;
		float decode_time_ms = 0;
		int compressed_size = 0;
		float score = 1e9f; // lower is better
	};

	virtual ~GRCodec() {}

	virtual Error compress(const PoolByteArray &img_data, int width, int height,
			int bytes_per_pixel, const CompressParams &params,
			PoolByteArray &r_output) = 0;

	virtual Error decompress(const PoolByteArray &compressed_data,
			Ref<Image> &r_output_image) = 0;

	virtual String get_name() const = 0;
	virtual Type get_type() const = 0;
	virtual bool is_available() const = 0;

	// Stateful codecs (H.264) override these
	virtual Error initialize(int width, int height) { return OK; }
	virtual void finalize() {}

	// Benchmark: encode a test image, measure time + size
	BenchmarkResult benchmark(int test_width = 256, int test_height = 256);

	static PoolByteArray _make_test_image(int w, int h);
};

// =========================================================================
// GRCodecManager — Registry, benchmark, and auto-selection
// =========================================================================

class GRCodecManager {
	static GRCodecManager *singleton;

	struct CodecEntry {
		GRCodec *codec = nullptr;
		GRCodec::BenchmarkResult bench;
		bool benchmarked = false;
	};

	Vector<CodecEntry> _codecs;
	GRCodec *_active_codec = nullptr;
	int _active_compression_type = __COMPRESSION_JPG;

	void _register_codec(GRCodec *codec, int compression_type);

public:
	static GRCodecManager *get_singleton();

	void init();
	void deinit();

	void run_benchmarks();
	void auto_select_best_codec();

	Error set_active_codec(int compression_type);
	GRCodec *get_active_codec() const;
	int get_active_compression_type() const;

	// Convenience: compress using active codec
	Error compress(PoolByteArray &r_output, const PoolByteArray &img_data,
			int width, int height, int bytes_per_pixel,
			int quality = 75, int subsampling = __SUBSAMPLING_H2V2);

	// Decompress using the type tag from the stream packet
	Error decompress(int compression_type, const PoolByteArray &data,
			Ref<Image> &r_output);

	// Query
	Array get_available_codecs() const;
	Dictionary get_benchmark_results() const;
	GRCodec *get_codec_for_type(int compression_type) const;

	GRCodecManager();
	~GRCodecManager();
};

// =========================================================================
// Built-in codec implementations
// =========================================================================

// jpge — lightweight standalone JPEG encoder (always available)
class GRCodecJpge : public GRCodec {
public:
	Error compress(const PoolByteArray &img_data, int width, int height,
			int bytes_per_pixel, const CompressParams &params,
			PoolByteArray &r_output) override;
	Error decompress(const PoolByteArray &compressed_data,
			Ref<Image> &r_output_image) override;
	String get_name() const override { return "jpge"; }
	Type get_type() const override { return CODEC_JPGE; }
	bool is_available() const override { return true; }
};

// PNG — uses Godot's built-in Image PNG codec (always available)
class GRCodecPNG : public GRCodec {
public:
	Error compress(const PoolByteArray &img_data, int width, int height,
			int bytes_per_pixel, const CompressParams &params,
			PoolByteArray &r_output) override;
	Error decompress(const PoolByteArray &compressed_data,
			Ref<Image> &r_output_image) override;
	String get_name() const override { return "PNG"; }
	Type get_type() const override { return CODEC_PNG; }
	bool is_available() const override { return true; }
};

// libjpeg-turbo — high-performance JPEG (conditional)
#ifdef GODOT_REMOTE_LIBJPEG_TURBO_ENABLED
class GRCodecJpegTurbo : public GRCodec {
public:
	Error compress(const PoolByteArray &img_data, int width, int height,
			int bytes_per_pixel, const CompressParams &params,
			PoolByteArray &r_output) override;
	Error decompress(const PoolByteArray &compressed_data,
			Ref<Image> &r_output_image) override;
	String get_name() const override { return "jpeg-turbo"; }
	Type get_type() const override { return CODEC_JPEG_TURBO; }
	bool is_available() const override { return true; }
};
#endif

// OpenH264 — H.264 video codec (dynamic loading)
class GRCodecH264 : public GRCodec {
	// Dynamic library handle
	void *_lib_handle = nullptr;

	// Resolved function pointers (opaque — cast to proper types in .cpp)
	void *_fn_create_encoder = nullptr;
	void *_fn_destroy_encoder = nullptr;
	void *_fn_create_decoder = nullptr;
	void *_fn_destroy_decoder = nullptr;

	// Encoder/decoder instances (opaque ISVCEncoder*/ISVCDecoder*)
	void *_encoder = nullptr;
	void *_decoder = nullptr;

	int _current_width = 0;
	int _current_height = 0;
	bool _probed = false;
	bool _probe_result = false;
	bool _encoder_initialized = false;
	bool _decoder_initialized = false;
	int64_t _frame_count = 0;

	// YUV I420 conversion buffer (reused across frames to avoid per-frame allocation)
	PoolByteArray _yuv_buffer;

	bool _load_library();
	bool _resolve_symbols();
	void _unload_library();
	Vector<String> _get_search_paths() const;
	bool _ensure_encoder(int width, int height, const CompressParams &params);
	bool _ensure_decoder();

public:
	Error compress(const PoolByteArray &img_data, int width, int height,
			int bytes_per_pixel, const CompressParams &params,
			PoolByteArray &r_output) override;
	Error decompress(const PoolByteArray &compressed_data,
			Ref<Image> &r_output_image) override;
	Error initialize(int width, int height) override;
	void finalize() override;
	String get_name() const override { return "OpenH264"; }
	Type get_type() const override { return CODEC_H264; }
	bool is_available() const override;

	GRCodecH264();
	~GRCodecH264();
};
