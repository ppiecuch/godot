/**************************************************************************/
/*  synth_benchmark.h                                                     */
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

	SynthBenchmarkStat() :
			measured_total_time(-1), measured_normalized_time(-1), index_normalized_time(-1), confidence(0), weight(1) {}

	SynthBenchmarkStat(const String &p_desc, real_t p_index_normalized_time, const String &p_value_type, real_t p_weight) :
			desc(p_desc), measured_total_time(-1), measured_normalized_time(-1), index_normalized_time(p_index_normalized_time), value_type(p_value_type), confidence(0), weight(p_weight) {}

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
	real_t run_time;

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

	SynthBenchmarkResults() :
			run_time(0) {}
};

struct TestRender;

class SynthBenchmark : public Reference {
	GDCLASS(SynthBenchmark, Reference)

	TestRender *render;
	SynthBenchmarkResults results;

	// param >0, p_work_scale 10 for normal precision and runtime of less than a second

	bool progress_benchmark();
	void trigger_gpu_benchmark(uint8_t p_work_scale);

	bool _gpu_benchmark;
	real_t _gpu_time;
	uint8_t _work_scale;
	int _progress;

	void _benchmark(int p_progress);
	void _render_done(const Variant &p_udata);

protected:
	static void _bind_methods();

public:
	void benchmark(bool p_gpu_benchmark, uint8_t p_work_scale);
	bool is_benchmark_in_progress() const;
	String get_benchmark_report() const;
	String get_version_string() const;

	SynthBenchmark();
	~SynthBenchmark();
};

#endif // SYNTH_BENCHMARK
