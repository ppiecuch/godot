#ifndef SYNTH_BENCHMARK
#define SYNTH_BENCHMARK

#include "core/math/math_funcs.h"
#include "core/reference.h"
#include "core/ustring.h"
#include "core/vector.h"

// time and amount of work that was measured
struct TimeSample {
	TimeSample(real_t p_total_time, real_t p_normalized_time) :
			total_time(p_total_time), normalized_time(p_normalized_time) {}

	real_t total_time; // in seconds, >=0
	real_t normalized_time; // Time / WorkScale, WorkScale might have been adjusted (e.g. quantized), >0
};

struct SynthBenchmarkStat {
	SynthBenchmarkStat() :
			measured_total_time(-1), measured_normalized_time(-1), index_normalized_time(-1), confidence(0), weight(1) {}

	SynthBenchmarkStat(const String &p_desc, real_t p_index_normalized_time, const String &p_value_type, real_t p_weight) :
			desc(p_desc), measured_total_time(-1), measured_normalized_time(-1), index_normalized_time(p_index_normalized_time), value_type(p_value_type), confidence(0), weight(p_weight) {}

	// Computes the linear performance index (>0), around 100 with good hardware but higher numbers are possible
	real_t compute_perf_index() const { return 100 * index_normalized_time / measured_normalized_time; }

	// p_time_sample seconds and normalized time (e.g. seconds / GPixels)
	// p_in_confidence 0..100
	void set_measured_time(const TimeSample &p_time_sample, real_t p_in_confidence = 90) {
		ERR_FAIL_COND(p_in_confidence < 0 || p_in_confidence > 100);
		measured_total_time = p_time_sample.total_time;
		measured_normalized_time = p_time_sample.normalized_time;
		confidence = p_in_confidence;
	}

	String get_desc() const { return desc; }
	String get_value_type() const { return value_type; }
	real_t get_normalized_time() const { return measured_normalized_time; }
	real_t get_measured_total_time() const { return measured_total_time; }
	real_t get_confidence() const { return confidence; } // return 0=no..100=full
	real_t get_weight() const { return weight; }

private:
	String desc;
	real_t measured_total_time; // -1 if not defined, in seconds, useful to see if a test did run too long (some slower GPUs might timeout)
	real_t measured_normalized_time; // -1 if not defined, depends on the test (e.g. s/GPixels), WorkScale is divided out
	real_t index_normalized_time; // -1 if not defined, timing value expected on a norm GPU (index value 100, here NVidia 670)
	String value_type;
	real_t confidence; // 0..100, 100: fully confident
	real_t weight; // 1 is normal weight, 0 is no weight, > 1 is unbounded additional weight
};

struct SynthBenchmarkResults {
	SynthBenchmarkStat CPUStats[2];
	SynthBenchmarkStat GPUStats[7];

	String report;

	real_t compute_cpu_perf_index(Vector<real_t> *p_individual_results = nullptr) const; // 100: avg good CPU, <100:slower, >100:faster
	real_t compute_gpu_perf_index(Vector<real_t> *p_individual_results = nullptr) const; // 100: avg good GPU, <100:slower, >100:faster

	// return in seconds, useful to check if a benchmark takes too long (very slow hardware, don't make tests with large WorkScale)
	real_t compute_total_gpu_time() const {
		real_t ret = 0;
		for (uint32_t i = 0; i < sizeof(GPUStats) / sizeof(GPUStats[0]); ++i) {
			ret += GPUStats[i].get_measured_total_time();
		}

		return ret;
	}
};

struct TestRender;

class SynthBenchmark : public Reference {
	GDCLASS(SynthBenchmark, Reference)

	TestRender *render;
	SynthBenchmarkResults results;

	// param >0, p_work_scale 10 for normal precision and runtime of less than a second
	String run_benchmark(bool p_gpu_benchmark = true, uint8_t p_work_scale = 10);
	void trigger_gpu_benchmark(uint8_t p_work_scale);

	void _render_done(const Variant &p_udata);

protected:
	static void _bind_methods();

public:
	String get_version_string() const;
	String benchmark();

	SynthBenchmark();
	~SynthBenchmark();
};

#endif // SYNTH_BENCHMARK
