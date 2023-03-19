#include "core/error_macros.h"
#include "core/os/thread.h"

#define TINA_IMPLEMENTATION
#define _TINA_ASSERT(_COND_, _MESSAGE_) DEV_ASSERT_MSG(_COND_, _MESSAGE_)
#include "tine/tina.h"

#define TINA_JOBS_IMPLEMENTATION
#include "tine/tina_jobs.h"

#include "tina_scheduler.h"

#if defined(__unix__) || defined(__APPLE__)
	#include <unistd.h>
	static unsigned scheduler_get_cpu_count(void){ return sysconf(_SC_NPROCESSORS_ONLN); }
#elif defined(_WINDOWS_)
	static unsigned scheduler_get_cpu_count(void) {
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		return sysinfo.dwNumberOfProcessors;
	}
#else
	#warning Unknown system type : default 2 cpu
	static unsigned scheduler_get_cpu_count(void){ return 2; }
#endif

typedef struct {
	thrd_t thread;
	tina_scheduler* sched;
	unsigned queue_idx;
	unsigned thread_id;
} worker_context;

#define MAX_WORKERS 256
static unsigned WORKER_COUNT;
worker_context WORKERS[MAX_WORKERS];

static int scheduler_worker_body(void* data){
	worker_context* ctx = data;
	tina_scheduler_run(ctx->sched, ctx->queue_idx, TINA_RUN_LOOP);
	return 0;
}

void scheduler_start_worker_threads(unsigned thread_count, tina_scheduler* sched, unsigned queue_idx){
	if(thread_count) {
		WORKER_COUNT = thread_count;
	} else {
		WORKER_COUNT = scheduler_get_cpu_count();
		print_verbose(vformat("%d CPUs detected.\n", WORKER_COUNT));
	}
	
	print_verbose("Creating WORKERS.");
	for(unsigned i = 0; i < WORKER_COUNT; i++) {
		worker_context* worker = WORKERS + i;
		(*worker) = (worker_context){.sched = sched, .queue_idx = queue_idx, .thread_id = i};
		thrd_create(&worker->thread, common_worker_body, worker);
	}
}

unsigned scheduler_worker_count(void) { return WORKER_COUNT; }

void common_destroy_worker_threads() {
	for (unsigned i = 0; i < WORKER_COUNT; i++) {
		thrd_join(WORKERS[i].thread, nullptr);
	}
}
