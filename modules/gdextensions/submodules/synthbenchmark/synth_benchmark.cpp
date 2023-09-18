/**************************************************************************/
/*  synth_benchmark.cpp                                                   */
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

#include "synth_benchmark.h"

#include "common/gd_core.h"
#include "core/os/os.h"
#include "core/ustring_preprocessor.h"
#include "core/version.h"
#include "drivers/video_common/video_utils.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"
#include "servers/visual/rasterizer.h"
#include "servers/visual/visual_server_globals.h"

#ifdef GDEXT_HWINFO_ENABLED
#include "hwinfo/hwinfo.h"
#include "hwinfo/utils/stringutils.h"
#endif

#define SYNTH_BENCH_VERSION "1.0"
#define SYNTH_BENCH_RELEASE "beta"

#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

enum {
	BENCH_BANNER,
	BENCH_CPU_TEST1,
	BENCH_CPU_TEST2,
	BENCH_CPU_RESULTS,
	BENCH_GPU_BANNER,
	BENCH_GPU_TEST1,
	BENCH_GPU_TEST2,
	BENCH_GPU_TEST3,
	BENCH_GPU_RESULTS,
	BENCH_SUMMARY,
};

real_t RayIntersectBenchmark();
real_t FractalBenchmark();

// to prevent compiler optimizations
static real_t g_global_state_object = 0;

_FORCE_INLINE_ static void memory_barrier() {
#if defined(__APPLE__) || defined(__linux__)
	__sync_synchronize();
#elif defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
	_mm_sfence();
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM)
	__dmb(_ARM64_BARRIER_SY);
#else
#warning "Missing memory barrier for this platform"
#endif
}

static String get_full_version_string() {
	static String _version;
	if (_version.empty()) {
		String hash = String(VERSION_HASH);
		if (hash.length() != 0) {
			hash = "." + hash.left(9);
		}
		_version = String(VERSION_NAME) + " v" + String(VERSION_FULL_BUILD) + hash + " - " + String(VERSION_WEBSITE);
	}
	return _version;
}

/* Divides two integers and rounds */
template <class T>
static constexpr _FORCE_INLINE_ T divide_and_round_up(T Dividend, T Divisor) { return (Dividend + Divisor - 1) / Divisor; }
template <class T>
static constexpr _FORCE_INLINE_ T divide_and_round_down(T Dividend, T Divisor) { return Dividend / Divisor; }
template <class T>
static constexpr _FORCE_INLINE_ T divide_and_round_nearest(T Dividend, T Divisor) { return (Dividend >= 0) ? (Dividend + Divisor / 2) / Divisor : (Dividend - Divisor / 2 + 1) / Divisor; }

#define CSTR(S) S.operator String().utf8().c_str()
#define INT64(I) I.operator int64_t()

#define string_format2(format, ...) string_format(format "\n", ##__VA_ARGS__)

// 100: avg good CPU, <100:slower, >100:faster
real_t SynthBenchmarkResults::compute_cpu_perf_index(Vector<real_t> *p_individual_results) const {
	const uint32_t num_stats = sizeof(CPUStats) / sizeof(CPUStats[0]);
	if (p_individual_results != nullptr) {
		p_individual_results->clear();
	}

	real_t ret = 0;
	real_t total_weight = 0;
	for (uint32_t i = 0; i < num_stats; ++i) {
		total_weight += CPUStats[i].get_weight();
	}

	for (uint32_t i = 0; i < num_stats; ++i) {
		const real_t perf_index = CPUStats[i].compute_perf_index();
		if (p_individual_results != nullptr) {
			p_individual_results->push_back(perf_index);
		}

		const real_t normalized_weight = (total_weight > 0) ? (CPUStats[i].get_weight() / total_weight) : 0;
		ret += perf_index * normalized_weight;
	}

	return ret;
}

// 100: avg good GPU, <100:slower, >100:faster
real_t SynthBenchmarkResults::compute_gpu_perf_index(Vector<real_t> *p_individual_results) const {
	const uint32_t num_stats = sizeof(GPUStats) / sizeof(GPUStats[0]);
	if (p_individual_results != nullptr) {
		p_individual_results->clear();
		p_individual_results->resize(num_stats);
	}

	real_t ret = 0;
	real_t total_weight = 0;
	for (uint32_t i = 0; i < num_stats; ++i) {
		total_weight += GPUStats[i].get_weight();
	}

	for (uint32_t i = 0; i < num_stats; ++i) {
		const real_t perf_index = GPUStats[i].compute_perf_index();
		if (p_individual_results != nullptr) {
			p_individual_results->push_back(perf_index);
		}

		const real_t normalized_weight = (total_weight > 0) ? (GPUStats[i].get_weight() / total_weight) : 0;
		ret += perf_index * normalized_weight;
	}

	return ret;
}

// NOTE: p_work_scale should be around 10 but can be adjusted for precision
// NOTE: p_function should run for about 3 ms
static TimeSample RunBenchmark(uint8_t p_work_scale, real_t (*p_function)()) {
	real_t sum = 0;

	uint32_t run_count = MAX(1, p_work_scale); // this test doesn't support fractional WorkScale

	for (uint32_t i = 0; i < run_count; ++i) {
		memory_barrier();
		const uint64_t start_time = OS::get_singleton()->get_ticks_usec();
		memory_barrier();

		g_global_state_object += p_function();

		memory_barrier();
		sum += OS::get_singleton()->get_ticks_usec() - start_time;
		memory_barrier();
	}
	const real_t secs = sum / 1000000.0;
	return TimeSample(secs, secs / run_count);
}

static const char *gpu_bench_shader = R"(
shader_type spatial;

uniform sampler2D input_texture;

// param xy should be a integer position (e.g. pixel position on the screen), repeats each 128x128 pixels
// similar to a texture lookup but is only ALU
// ~13 ALU operations (3 frac, 6 *, 4 mad)
float pseudo_random(float2 xy) {
	float2 pos = frac(xy / 128.0f) * 128.0f + float2(-64.340622f, -72.465622f);
	return frac(dot(pos.xyx * pos.xyy, float3(20.390625f, 60.703125f, 2.4281209f))); // found by experimentation
}

#if VS_METHOD_0 // Simple Quad Vertex Shader, used for pixel tests

in float4 InPosition : ATTRIBUTE0,
in float2 UV : ATTRIBUTE1,
out float2 OutUV : TEXCOORD0,
out float4 OutPosition : SV_POSITION

void vertex() {
	// DrawRectangle(InPosition, UV, OutPosition, OutUV);
	POSITION = InPosition;
	POSITION.xy = -1.0f + 2.0f * (DrawRectangleParameters.PosScaleBias.zw + (InPosition.xy * DrawRectangleParameters.PosScaleBias.xy)) * DrawRectangleParameters.InvTargetSizeAndTextureSize.xy;
	POSITION.xy *= float2( 1, -1 );
	UV.xy = (DrawRectangleParameters.UVScaleBias.zw + (InTexCoord.xy * DrawRectangleParameters.UVScaleBias.xy)) * DrawRectangleParameters.InvTargetSizeAndTextureSize.zw;
}

#elif VS_METHOD_1 // Vertex Throughput Test

in float4 Arg0 : ATTRIBUTE0,
in float4 Arg1 : ATTRIBUTE1,
in float4 Arg2 : ATTRIBUTE2,
in float4 Arg3 : ATTRIBUTE3,
in float4 Arg4 : ATTRIBUTE4,
out float2 OutUV : TEXCOORD0,
out float4 OutPosition : SV_POSITION

void vertex() {
	POSITION = Arg0 + Arg1 + Arg2 + Arg3 + Arg4;
	UV = 0.0f;
}

#elif VS_METHOD_2

in uint VertexID : SV_VertexID,
out float2 OutUV : TEXCOORD0,
out float4 OutPosition : SV_POSITION

void vertex() {
	POSITION = float4(VertexID, 0, 0, 0);
	UV = 0.0f;
}

#else
#error Invalid VS_METHOD
#endif

float2 InUV : TEXCOORD0, out float4 OutColor : SV_Target0

// pixel shader entry point
void fragment() {
	COLOR = 0;

#if PS_METHOD_0 // ALU heavy
	{
		// some dependency to the input texture
		OutColor = Texture2DSample(input_texture, input_textureSampler, InUV) * 0.99f;

		UNROLL for(int i = 0; i < 16; ++i) {
			// todo: use float4 MAD to get raw GPU performance (should be same for scalar and non scalar)
			float4 value = pseudo_random(InUV + float2(i * 0.001f, 0));
			COLOR.r += value.r;
		}
	}
#elif PS_METHOD_1 // TEX heavy
	{
		UNROLL for(int i = 0; i < 16; ++i) {
			float4 value = Texture2DSample(input_texture, input_textureSampler, InUV + float2(i * 0.0001f, 0));
			COLOR.r += value.r;
		}
	}
#elif PS_METHOD_2 // dependent TEX heavy
	{
		UNROLL for(int i = 0; i < 16; ++i) {
			float4 value = Texture2DSample(input_texture, input_textureSampler, InUV + float2(i * 0.001f, COLOR.r * 0.001f));
			COLOR.r += value.r;
		}
	}
#elif PS_METHOD_3
	COLOR = Texture2DSample(input_texture, input_textureSampler, InUV) * 0.99f; // some dependency to the input texture
#elif PS_METHOD_4 // Bandwidth heavy
	{
		float2 PixelPos = frac(InUV * 512.0f / 16.0f) * 16.0f;

		UNROLL for(int y = 0; y < 4; ++y) {
			UNROLL for(int x = 0; x < 4; ++x) {
				// should be bandwidth trashing enough to profile memory bandwidth
				float4 value = Texture2DSample(input_texture, input_textureSampler, (PixelPos + float2(x, y)) * 16 / 512.0f);
				COLOR.r += value.r;
			}
		}
	}
#elif PS_METHOD_5
	// Simple Pixel Shader used when testing vertex throughput.
	// Do Nothing (COLOR is black).
#else
#error Invalid PS_METHOD
#endif

	// TODO: Framebuffer blending test, clear test, vertex performance, draw call performance, constant buffer upload performance
})";

struct TestRender {
	Object *owner;
	RID scenario, viewport, viewport_texture, camera, light, light_instance, mesh_instance;

	void _create();
	void _trigger(const Ref<Mesh> &p_mesh, const Size2 &p_size = Size2(256, 256), const String &p_filepath = "");
	Ref<Image> _get_image();
	void _destroy();

	void done() {
		if (viewport.is_valid()) {
			VS::get_singleton()->viewport_set_active(viewport, false);
		}
	}

	~TestRender() {
		if (viewport.is_valid()) {
			_destroy();
		}
	}
};

void TestRender::_create() {
	scenario = RID_PRIME(VS::get_singleton()->scenario_create());

	viewport = RID_PRIME(VS::get_singleton()->viewport_create());
	VS::get_singleton()->viewport_set_update_mode(viewport, VS::VIEWPORT_UPDATE_DISABLED);
	VS::get_singleton()->viewport_set_vflip(viewport, true);
	VS::get_singleton()->viewport_set_scenario(viewport, scenario);
	VS::get_singleton()->viewport_set_size(viewport, 128, 128);
	VS::get_singleton()->viewport_set_transparent_background(viewport, true);
	VS::get_singleton()->viewport_set_active(viewport, false);

	viewport_texture = VS::get_singleton()->viewport_get_texture(viewport);

	camera = RID_PRIME(VS::get_singleton()->camera_create());
	VS::get_singleton()->viewport_attach_camera(viewport, camera);
	VS::get_singleton()->camera_set_transform(camera, Transform(Basis(), Vector3(0, 0, 3)));
	VS::get_singleton()->camera_set_perspective(camera, 45, 0.1, 10);

	light = RID_PRIME(VS::get_singleton()->directional_light_create());
	light_instance = VS::get_singleton()->instance_create2(light, scenario);
	VS::get_singleton()->instance_set_transform(light_instance, Transform().looking_at(Vector3(-1, -1, -1), Vector3(0, 1, 0)));

	mesh_instance = RID_PRIME(VS::get_singleton()->instance_create());
	VS::get_singleton()->instance_set_scenario(mesh_instance, scenario);
}

void TestRender::_trigger(const Ref<Mesh> &p_mesh, const Size2 &p_size, const String &p_filepath) {
	ERR_FAIL_COND(!p_mesh.is_valid());

	if (!viewport.is_valid()) {
		_create();
	}

	ERR_FAIL_COND(!viewport.is_valid());
	ERR_FAIL_COND(!mesh_instance.is_valid());

	// setup mesh
	VS::get_singleton()->instance_set_base(mesh_instance, p_mesh->get_rid());

	AABB aabb = p_mesh->get_aabb();
	Vector3 ofs = aabb.position + aabb.size * 0.5;
	aabb.position -= ofs;
	Transform xform;
	xform.basis = Basis().rotated(Vector3(0, 1, 0), -Math_PI * 0.125);
	xform.basis = Basis().rotated(Vector3(1, 0, 0), Math_PI * 0.125) * xform.basis;
	AABB rot_aabb = xform.xform(aabb);
	real_t m = MAX(rot_aabb.size.x, rot_aabb.size.y) * 0.5;
	if (m == 0) {
		return;
	}
	m = (1.0 / m) * 0.5;
	xform.basis.scale(Vector3(m, m, m));
	xform.origin = -xform.basis.xform(ofs); //-ofs*m;
	xform.origin.z -= rot_aabb.size.z * 2;
	VS::get_singleton()->instance_set_transform(mesh_instance, xform);

	// setup viewport
	VS::get_singleton()->request_frame_drawn_callback(owner, "_render_done", p_filepath);
	VS::get_singleton()->viewport_set_size(viewport, p_size.width, p_size.height);
	VS::get_singleton()->viewport_set_update_mode(viewport, VS::VIEWPORT_UPDATE_ONCE); // once used for capture
	VS::get_singleton()->viewport_set_active(viewport, true);
}

Ref<Image> TestRender::_get_image() {
	ERR_FAIL_COND_V(!viewport_texture.is_valid(), Ref<Image>());
	Ref<Image> img = VS::get_singleton()->texture_get_data(viewport_texture);
	ERR_FAIL_COND_V(img.is_null(), Ref<Image>());
	img->convert(Image::FORMAT_RGBA8);
	return img;
}

#define _FREE_RID(var)                  \
	if (var.is_valid()) {               \
		VS::get_singleton()->free(var); \
		var = RID();                    \
	}

void TestRender::_destroy() {
	_FREE_RID(mesh_instance);
	_FREE_RID(light_instance);
	_FREE_RID(light);
	_FREE_RID(camera);
	_FREE_RID(viewport);
	_FREE_RID(scenario);
}

template <int NumMethods>
String print_gpu_stats(SynthBenchmarkStat (&GPUStats)[NumMethods], const String &end_string) {
	String report;
	for (uint32_t MethodId = 0; MethodId < NumMethods; ++MethodId) {
		report += string_format2("         ... %.3f %s, Confidence=%.0f%% '%s'%s",
				1.0 / GPUStats[MethodId].get_normalized_time(),
				GPUStats[MethodId].get_value_type().c_str(),
				GPUStats[MethodId].get_confidence(),
				GPUStats[MethodId].get_desc().c_str(),
				end_string.c_str());
	}
	return report;
}

void SynthBenchmark::_render_done(const Variant &p_udata) {
	String file = p_udata;
	if (!file.empty()) {
		if (Ref<Image> img = render->_get_image()) {
			if (img.is_valid()) {
				img->save_png(file);
				print_verbose("Snapshot saved to file: " + file);
			}
		}
	}

	render->done();
	String report;
	_gpu_time = results.compute_total_gpu_time();
	if (_gpu_time > 0) {
		switch (_progress) {
			case BENCH_GPU_TEST1:
				report += string_format2("  GPU first test: %.2fs", _gpu_time);
				break;
			case BENCH_GPU_TEST2:
				report += string_format2("  GPU second test: %.2fs", _gpu_time);
				break;
			case BENCH_GPU_TEST3:
				report += string_format2("  GPU third test: %.2fs", _gpu_time);
				break;
		}
		report += print_gpu_stats(results.GPUStats, " (likely to be inaccurate)");
	}
	if (!report.empty()) {
		results.report += report;
	}
	call_deferred("_benchmark", ++_progress); // continue
}

void SynthBenchmark::trigger_gpu_benchmark(uint8_t p_work_scale) {
	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = parray(
			Vector3(0, 0, 0),
			Vector3(10, 0, 0),
			Vector3(1, 20, 0),
			Vector3(0, 2, 0),
			Vector3(0, 0, 0),
			Vector3(50, 100, 1));
	mesh_array[VS::ARRAY_COLOR] = parray(
			Color::named("red"),
			Color::named("red"),
			Color::named("green"),
			Color::named("green"),
			Color::named("blue"),
			Color::named("blue"));
	Ref<ArrayMesh> mesh = memnew(ArrayMesh);
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, mesh_array, Array());
	Ref<ShaderMaterial> material = memnew(ShaderMaterial);
	Ref<Shader> shader = memnew(Shader);
	shader->set_code(StringProcessor("VS_METHOD_0,UNROLL").process(gpu_bench_shader));
	material->set_shader(shader);
	mesh->surface_set_material(0, material);
	render->_trigger(mesh);
}

bool SynthBenchmark::progress_benchmark() {
	uint8_t work_scale = _work_scale;
	bool gpu_benchmark = _gpu_benchmark;

	if (!gpu_benchmark) {
		// run a very quick GPU benchmark (less confidence but at least we get some numbers)
		// it costs little time and we get some stats
		work_scale = 1;
	}

	String report;
	bool next_step = true;

	const uint64_t start_time = OS::get_singleton()->get_ticks_usec();

	switch (_progress) {
		case BENCH_BANNER: {
			report += string_format2("SynthBenchmark (%s)", get_version_string().utf8().c_str());
			report += string_format2("===============");

#if DEBUG_ENABLED
			report += string_format2("  Note: Values are not trustable because this is a DEBUG build!");
#endif

			report += string_format2("Requested WorkScale: %d", work_scale);
			report += string_format2("Main Processor:");
		} break;

			// developer machine: Intel Xeon E5-2660 2.2GHz
			// divided by the actual value on a developer machine to normalize the results
			// index should be around 100 +-4 on developer machine in a development build (should be the same in shipping)

		case BENCH_CPU_TEST1: {
			results.CPUStats[0] = SynthBenchmarkStat("RayIntersect", 0.02561, "s/Run", 1);
			results.CPUStats[0].set_measured_time(RunBenchmark(work_scale, RayIntersectBenchmark));
		} break;

		case BENCH_CPU_TEST2: {
			results.CPUStats[1] = SynthBenchmarkStat("Fractal", 0.0286, "s/Run", 1.5);
			results.CPUStats[1].set_measured_time(RunBenchmark(work_scale, FractalBenchmark));
		} break;

		case BENCH_CPU_RESULTS: {
			for (uint32_t i = 0; i < ARRAY_COUNT(results.CPUStats); ++i) {
				report += string_format2("         ... %f %s '%s'", results.CPUStats[i].get_normalized_time(), results.CPUStats[i].get_value_type().utf8().c_str(), results.CPUStats[i].get_desc().utf8().c_str());
			}

			report += string_format2("");

			const bool app_is64_bit = (sizeof(void *) == 8);

			report += string_format2("  CompiledTarget Bits: %s", app_is64_bit ? "64" : "32");
			report += string_format2("  BUILD_INFO: %s", get_full_version_string().utf8().c_str());
#ifdef BUILD_DEBUG
			report += string_format2("  BUILD_DEBUG: %d", DEBUG_ENABLED);
#endif
			report += string_format2("  BUILD_VERBOSE: %d", OS::get_singleton()->is_stdout_verbose());
			report += string_format2("  DATA_DIR: %s", OS::get_singleton()->get_data_path().utf8().c_str());
			report += string_format2("  USER_DATA_DIR: %s", OS::get_singleton()->get_user_data_dir().utf8().c_str());

			report += string_format2("  CPU Name: %s", OS::get_singleton()->get_processor_name().utf8().c_str());
			report += string_format2("  NumberOfCores (physical): %d", OS::get_singleton()->get_processor_count());

#ifdef GDEXT_HWINFO_ENABLED
			const hwinfo::System system_info;
			report += string_format2("  System Uptime: %s", hwinfo::utils::time_duration_string(system_info.getUptimeSeconds()).c_str());
			std::vector<float> cpu_load = system_info.getCpuUsagePercent();
			if (cpu_load.size()) {
				String cpu_load_str = string_format("%2.1f%%", cpu_load[0]);
				for (size_t c = 1; c < cpu_load.size(); c++) {
					cpu_load_str += string_format(" | %2.1f%%", cpu_load[c]);
				}
				report += string_format2("  CPU Load: %s", cpu_load_str.utf8().c_str());
			}
			for (uint32_t MethodId = 0; MethodId < ARRAY_COUNT(results.CPUStats); ++MethodId) {
				report += string_format2("  CPU Perf Index %d: %.1f (weight %.2f)", MethodId, results.CPUStats[MethodId].compute_perf_index(), results.CPUStats[MethodId].get_weight());
			}
			report += string_format2("  Processes: %d", system_info.getNumProcesses());
			std::string proc_report = hwinfo::get_string_property(hwinfo::SYS_PROCESS_REPORT);
			if (!proc_report.empty()) {
				report += string_format2("%s", proc_report.c_str());
			}
#else
			report += string_format2("  (System hardware details info not available)";
#endif
		} break;

		case BENCH_GPU_BANNER: {
			report += string_format2(" ");

			const Dictionary &ctx = VisualServerGlobals::rasterizer->get_video_context_info();
			const VideoDriverInfo &info = video_get_driver_info(ctx);

			report += string_format2("Graphics:");
			report += string_format2("  Driver Id: %d", OS::get_singleton()->get_current_video_driver());
			report += string_format2("  Adapter Name: '%s'", CSTR(ctx[VideoContextInfoVideoRenderer]));
			report += string_format2("  Vendor Id: %d", info.vendor);
			report += string_format2("  Device Id: %d", info.renderer);
			report += string_format2("  Device Revision: %s", video_get_video_version_string(info.version).utf8().c_str());

#ifdef GDEXT_HWINFO_ENABLED
			const std::vector<hwinfo::GPU> &gpu = hwinfo::getAllGPUs();
			for (size_t i = 0; i < gpu.size(); i++) {
				const auto &g = gpu[i];
				if (gpu.size() > 1) {
					report += string_format2(" >Device %d:", i);
				}
				int64_t m;
				String gpu_hw_report;
				if ((m = g.totalMemoryMBytes()) > 0) {
					gpu_hw_report += string_format2("  GPU Total Memory: %d MB", m);
				}
				if ((m = hwinfo::get_int_property(hwinfo::GPU_TEXTURE_MEMORY_MB, i)) > 0) {
					gpu_hw_report += string_format2("  GPU Texture Memory: %d MB", m);
				}
				if ((m = hwinfo::get_int_property(hwinfo::GPU_AVAILABLE_MEMORY_MB, i)) > 0) {
					gpu_hw_report += string_format2("  GPU Available Memory: %d MB", m);
				}
				if (gpu_hw_report.empty()) {
					report += "  (Hardware details info not available)";
				} else {
					report += gpu_hw_report;
				}
			}
			std::string gpu_report = hwinfo::get_string_property(hwinfo::GPU_SUMMARY_REPORT);
			if (!gpu_report.empty()) {
				report += string_format2("%s", gpu_report.c_str());
			}
#else
			report += string_format2("  (GPU hardware details info not available)";
#endif // GDEXT_HWINFO_ENABLED
		} break;

		// Not always done - cost some time.
		// First we run a quick test. If that shows very bad performance we don't need another test
		// The hardware is slow, we don't need a long test and risk driver TDR (driver recovery).
		// We have seen this problem on very low end GPUs.
		case BENCH_GPU_TEST1: {
			if (gpu_benchmark) {
				const real_t first_work_scale = 0.01 * work_scale;
				trigger_gpu_benchmark(first_work_scale);
				next_step = false;
			}
		} break;

		case BENCH_GPU_TEST2: {
			if (gpu_benchmark) {
				if (_gpu_time < 0.15) {
					const real_t second_work_scale = 0.1 * work_scale;
					trigger_gpu_benchmark(second_work_scale);
					next_step = false;
				}
			}
		} break;

		case BENCH_GPU_TEST3: {
			if (gpu_benchmark) {
				if (_gpu_time < 0.15) {
					trigger_gpu_benchmark(work_scale);
					next_step = false;
				}
			}
		} break;

		case BENCH_GPU_RESULTS: {
			if (_gpu_time > 0) {
				report += string_format2("  GPU Final Results:");
				print_gpu_stats(results.GPUStats, "");
				report += string_format2("");

				for (uint32_t MethodId = 0; MethodId < ARRAY_COUNT(results.GPUStats); ++MethodId) {
					report += string_format2("  GPU Perf Index %d: %.1f (weight %.2f)", MethodId, results.GPUStats[MethodId].compute_perf_index(), results.GPUStats[MethodId].get_weight());
				}

				report += string_format2("  GPUIndex: %.1f", results.compute_gpu_perf_index());
			}
		} break;

		case BENCH_SUMMARY: {
			report += string_format2("  CPUIndex: %.1f", results.compute_cpu_perf_index());
			report += string_format2("");
			report += string_format2("         ... Total Time: %.2f sec", results.run_time += (OS::get_singleton()->get_ticks_usec() - start_time) / 1000000.);
		} break;
	}

	if (!report.empty()) {
		results.report += report;
	}

	if (_progress != BENCH_SUMMARY) {
		results.run_time += (OS::get_singleton()->get_ticks_usec() - start_time) / 1000000.;
	}

	return next_step;
}

String SynthBenchmark::get_version_string() const {
	String ver = SYNTH_BENCH_VERSION;
#ifdef SYNTH_BENCH_RELEASE
	if (strlen(SYNTH_BENCH_RELEASE)) {
		ver += vformat("-%s", SYNTH_BENCH_RELEASE);
	}
#endif
	return ver;
}

String SynthBenchmark::get_benchmark_report() const {
	return results.report;
}

bool SynthBenchmark::is_benchmark_in_progress() const {
	return _progress > 0;
}

void SynthBenchmark::benchmark(bool p_gpu_benchmark, uint8_t p_work_scale) {
	ERR_FAIL_COND_MSG(_progress > 0, "Benchmark in progress");
	ERR_FAIL_COND(_work_scale == 0);
	_gpu_benchmark = p_gpu_benchmark;
	_work_scale = p_work_scale;
	call_deferred("_benchmark", 0);
}

void SynthBenchmark::_benchmark(int p_progress) {
	ERR_FAIL_COND_MSG(_progress != p_progress, "Wrong benchmark sequence");
	print_verbose("Running benchmark sequence " + itos(_progress) + " ..");
	if (progress_benchmark()) {
		if (_progress == BENCH_SUMMARY) {
			emit_signal("benchmark_ready");
		} else {
			call_deferred("_benchmark", ++_progress); // continue
		}
	}
}

void SynthBenchmark::_bind_methods() {
	ClassDB::bind_method(D_METHOD("benchmark", "gpu_benchmark", "work_scale"), &SynthBenchmark::benchmark, DEFVAL(true), DEFVAL(10));
	ClassDB::bind_method(D_METHOD("is_benchmark_in_progress"), &SynthBenchmark::is_benchmark_in_progress);
	ClassDB::bind_method(D_METHOD("get_benchmark_report"), &SynthBenchmark::get_benchmark_report);
	ClassDB::bind_method(D_METHOD("get_version_string"), &SynthBenchmark::get_version_string);

	ClassDB::bind_method(D_METHOD("_benchmark", "progress"), &SynthBenchmark::_benchmark);
	ClassDB::bind_method(D_METHOD("_render_done"), &SynthBenchmark::_render_done);

	ADD_SIGNAL(MethodInfo("benchmark_ready"));
}

SynthBenchmark::SynthBenchmark() {
	render = memnew(TestRender);
	render->owner = this;
	_gpu_benchmark = true;
	_gpu_time = 0;
	_work_scale = 10;
	_progress = 0;
}

SynthBenchmark::~SynthBenchmark() {
	memdelete(render);
}
