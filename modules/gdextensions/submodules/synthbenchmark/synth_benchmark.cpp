#include "synth_benchmark.h"

#include "common/gd_core.h"
#include "core/os/os.h"
#include "core/version.h"
#include "drivers/video_common/video_utils.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"
#include "servers/visual/rasterizer.h"

#define SYNTH_BENCH_VERSION "1.0"
#define SYNTH_BENCH_RELEASE "beta"

#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

real_t RayIntersectBenchmark();
real_t FractalBenchmark();

// to prevent compiler optimizations
static real_t g_global_state_object = 0;

_FORCE_INLINE_ static void memory_barrier() {
#if defined(__APPLE__)
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

struct TestRender {
	Reference *owner;
	RID scenario, viewport, viewport_texture, camera, light, light_instance, mesh_instance;

	void _create();
	void _trigger(const Ref<Mesh> &p_mesh, const Size2 &p_size = Size2(256, 256), const String &p_filepath = "");
	Ref<Image> _get_image();
	void _destroy();

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
	m = 1.0 / m;
	m *= 0.5;
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
	_FREE_RID(viewport_texture);
	_FREE_RID(viewport);
	_FREE_RID(scenario);
}

template <int NumMethods>
void print_gpu_stats(SynthBenchmarkStat (&GPUStats)[NumMethods], const String &end_string) {
	for (uint32_t MethodId = 0; MethodId < NumMethods; ++MethodId) {
		printf_verbose("         ... %.3f %s, Confidence=%.0f%% '%s'%s",
				1.0f / GPUStats[MethodId].get_normalized_time(),
				GPUStats[MethodId].get_value_type().c_str(),
				GPUStats[MethodId].get_confidence(),
				GPUStats[MethodId].get_desc().c_str(),
				end_string.c_str());
	}
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
	emit_signal("_render_ready", p_udata);
}

void SynthBenchmark::trigger_gpu_benchmark(uint8_t p_work_scale) {
	Ref<ArrayMesh> mesh = memnew(ArrayMesh);
	render->_trigger(mesh);
}

#define string_format2(format, ...) string_format(format "\n", ##__VA_ARGS__)

String SynthBenchmark::run_benchmark(bool p_gpu_benchmark, uint8_t p_work_scale) {
	ERR_FAIL_COND_V(p_work_scale == 0, String());

	String report;

	if (!p_gpu_benchmark) {
		// run a very quick GPU benchmark (less confidence but at least we get some numbers)
		// it costs little time and we get some stats
		p_work_scale = 1;
	}

	const uint32_t start_time = OS::get_singleton()->get_ticks_msec();

	report += string_format2("SynthBenchmark (%s)", get_version_string().utf8().c_str());
	report += string_format2("===============");

#if DEBUG_ENABLED
	report += string_format2("  Note: Values are not trustable because this is a DEBUG build!");
#endif

	report += string_format2("Requested WorkScale: %d", p_work_scale);
	report += string_format2("Main Processor:");

	// developer machine: Intel Xeon E5-2660 2.2GHz
	// divided by the actual value on a developer machine to normalize the results
	// index should be around 100 +-4 on developer machine in a development build (should be the same in shipping)

	results.CPUStats[0] = SynthBenchmarkStat("RayIntersect", 0.02561, "s/Run", 1);
	results.CPUStats[0].set_measured_time(RunBenchmark(p_work_scale, RayIntersectBenchmark));

	results.CPUStats[1] = SynthBenchmarkStat("Fractal", 0.0286, "s/Run", 1.5);
	results.CPUStats[1].set_measured_time(RunBenchmark(p_work_scale, FractalBenchmark));

	for (uint32_t i = 0; i < ARRAY_COUNT(results.CPUStats); ++i) {
		report += string_format2("         ... %f %s '%s'", results.CPUStats[i].get_normalized_time(), results.CPUStats[i].get_value_type().utf8().c_str(), results.CPUStats[i].get_desc().utf8().c_str());
	}

	report += string_format2("");

	const bool app_is64_bit = (sizeof(void *) == 8);

	report += string_format2("  CompiledTarget Bits: %s", app_is64_bit ? "64" : "32");
	report += string_format2("  BUILD_INFO: %s", get_full_version_string().utf8().c_str());
	report += string_format2("  BUILD_DEBUG: %d", DEBUG_ENABLED);
	report += string_format2("  BUILD_VERBOSE: %d", OS::get_singleton()->is_stdout_verbose());
	report += string_format2("  DATA_DIR: %s", OS::get_singleton()->get_data_path().utf8().c_str());
	report += string_format2("  USER_DATA_DIR: %s", OS::get_singleton()->get_user_data_dir().utf8().c_str());

	report += string_format2("  CPU Name: %s", OS::get_singleton()->get_processor_name().utf8().c_str());
	report += string_format2("  NumberOfCores (physical): %d", OS::get_singleton()->get_processor_count());

	for (uint32_t MethodId = 0; MethodId < ARRAY_COUNT(results.CPUStats); ++MethodId) {
		report += string_format2("  CPU Perf Index %d: %.1f (weight %.2f)", MethodId, results.CPUStats[MethodId].compute_perf_index(), results.CPUStats[MethodId].get_weight());
	}

	report += string_format2(" ");

	const Dictionary &ctx = OS::get_singleton()->get_video_system_info(OS::VIDEO_SYSTEM_CONTEXT_INFO);
	const VideoDriverInfo &info = video_get_driver_info(ctx);

	report += string_format2("Graphics:");
	report += string_format2("  Driver Id: %d", OS::get_singleton()->get_current_video_driver());
	report += string_format2("  Adapter Name: '%s'", CSTR(ctx[VideoContextInfoVideoRenderer]));
	report += string_format2("  Vendor Id: %d", info.vendor);
	report += string_format2("  Device Id: %d", info.renderer);
	report += string_format2("  Device Revision: %s", video_get_video_version_string(info.version).utf8().c_str());

	if (int64_t m = INT64(OS::get_singleton()->get_video_system_info(OS::VIDEO_SYSTEM_TOTAL_MEMORY))) {
		report += string_format2("  GPU Total Memory: %d MB", m);
	}
	if (int64_t m = INT64(OS::get_singleton()->get_video_system_info(OS::VIDEO_SYSTEM_TEXTURE_MEMORY))) {
		report += string_format2("  GPU Texture Memory: %d MB", m);
	}
	if (int64_t m = INT64(OS::get_singleton()->get_video_system_info(OS::VIDEO_SYSTEM_AVAILABLE_MEMORY))) {
		report += string_format2("  GPU Available Memory: %d MB", m);
	}

	real_t gpu_time = 0;

	// not always done - cost some time.
	if (p_gpu_benchmark) {
		// First we run a quick test. If that shows very bad performance we don't need another test
		// The hardware is slow, we don't need a long test and risk driver TDR (driver recovery).
		// We have seen this problem on very low end GPUs.
		{
			const real_t first_work_scale = 0.01 * p_work_scale;
			const real_t second_work_scale = 0.1 * p_work_scale;

			trigger_gpu_benchmark(first_work_scale);
			gpu_time = results.compute_total_gpu_time();
			if (gpu_time > 0) {
				report += string_format2("  GPU first test: %.2fs", gpu_time);
				print_gpu_stats(results.GPUStats, " (likely to be very inaccurate)");
			}

			if (gpu_time < 0.15) {
				trigger_gpu_benchmark(second_work_scale);
				gpu_time = results.compute_total_gpu_time();

				if (gpu_time > 0) {
					report += string_format2("  GPU second test: %.2fs", gpu_time);
					print_gpu_stats(results.GPUStats, " (likely to be inaccurate)");
				}

				if (gpu_time < 0.15) {
					trigger_gpu_benchmark(p_work_scale);
					gpu_time = results.compute_total_gpu_time();

					if (gpu_time > 0) {
						report += string_format2("  GPU third test: %.2fs", gpu_time);
						print_gpu_stats(results.GPUStats, "");
					}
				}
			}
		}

		if (gpu_time > 0) {
			report += string_format2("  GPU Final Results:");
			print_gpu_stats(results.GPUStats, "");
			report += string_format2("");

			for (uint32_t MethodId = 0; MethodId < ARRAY_COUNT(results.GPUStats); ++MethodId) {
				report += string_format2("  GPU Perf Index %d: %.1f (weight %.2f)", MethodId, results.GPUStats[MethodId].compute_perf_index(), results.GPUStats[MethodId].get_weight());
			}
		}
	}

	if (gpu_time > 0) {
		report += string_format2("  GPUIndex: %.1f", results.compute_gpu_perf_index());
	}

	report += string_format2("  CPUIndex: %.1f", results.compute_cpu_perf_index());
	report += string_format2("");
	report += string_format2("         ... Total Time: %.2f sec", (OS::get_singleton()->get_ticks_msec() - start_time) / 1000.);

	return results.report = report;
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

String SynthBenchmark::benchmark() {
	run_benchmark();
	emit_signal("benchmark_ready");
	return results.report;
}

void SynthBenchmark::_bind_methods() {
	ClassDB::bind_method(D_METHOD("benchmark"), &SynthBenchmark::benchmark);
	ClassDB::bind_method(D_METHOD("get_version_string"), &SynthBenchmark::get_version_string);

	ClassDB::bind_method(D_METHOD("_render_done"), &SynthBenchmark::_render_done);

	ADD_SIGNAL(MethodInfo("_render_ready"));
	ADD_SIGNAL(MethodInfo("benchmark_ready"));
}

SynthBenchmark::SynthBenchmark() {
	render = memnew(TestRender);
	render->owner = this;
}

SynthBenchmark::~SynthBenchmark() {
	memdelete(render);
}
