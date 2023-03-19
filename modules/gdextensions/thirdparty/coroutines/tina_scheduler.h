#ifndef TINA_SCHEDULER_H
#define TINA_SCHEDULER_H

typedef struct tina_scheduler tina_scheduler;

void scheduler_start_worker_threads(unsigned thread_count, tina_scheduler* sched, unsigned queue_idx);
unsigned scheduler_worker_count(void);
void scheduler_destroy_worker_threads();

#endif // TINA_SCHEDULER_H
