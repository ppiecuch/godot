/*
 * libqueue - provides persistent, named data storage queues
 * Copyright (C) 2014 Jens Oliver John <dev@2ion.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * */


#ifndef CQUEUE_H
#define CQUEUE_H


#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#if defined(__GNUC__) && (__GNUC__ >= 4)
#define WARN_UNUSED_RETURN __attribute__ ((warn_unused_result))
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
#define WARN_UNUSED_RETURN _Check_return_
#else
#define WARN_UNUSED_RETURN
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define QUEUE_DATADIR ("libqueue")
#define QUEUE_TUNINGSUFFIX "#type=kct#zcomp=gz#opts=c"

#define QUEUEUTILS_QUEUE (".")
#define SELECTQUEUE(pvar) ((pvar)!=nullptr?(pvar):(QUEUEUTILS_QUEUE))

enum {
  LIBQUEUE_FAILURE = -1,
  LIBQUEUE_SUCCESS = 0,
  LIBQUEUE_MEM_ERROR = -2
};

struct Queue;

struct QueueData {
  void *v;
  size_t vlen ;
};

Queue *queue_open_with_options(const char *path,...) WARN_UNUSED_RETURN;
Queue *queue_open(const char *path) WARN_UNUSED_RETURN;
int queue_is_opened (const Queue * const q) WARN_UNUSED_RETURN;
int queue_push(Queue *q, QueueData *d) WARN_UNUSED_RETURN;
int queue_pop(Queue *q, QueueData *d) WARN_UNUSED_RETURN;
int queue_len(Queue *q, int64_t *len) WARN_UNUSED_RETURN;
int queue_count(Queue *q, int64_t *count) WARN_UNUSED_RETURN;
int queue_compact(Queue *q) WARN_UNUSED_RETURN;
int queue_peek(Queue *q, int64_t s, QueueData *d) WARN_UNUSED_RETURN;
int queue_poke(Queue *q, int64_t s, QueueData *d) WARN_UNUSED_RETURN;
int queue_close(Queue *q) WARN_UNUSED_RETURN;
int queue_opened(Queue *q) WARN_UNUSED_RETURN;
const char *queue_get_last_error(const Queue * const q) WARN_UNUSED_RETURN;

int closequeue(Queue *q);

#ifdef __cplusplus
}
#endif

// cqueue
// ======
//
// cqueue is a C library that provides persistent, named data queues for
// programs. Arbitrarily many queues can co-exist, given they have distinct
// names.
//
// Arbitrary binary data can be stored. Queues are manipulated using a
// stack-based API:
//
// * <code>push()</code> -- append data to the queue
// * <code>pop()</code> -- pop data from the queue
// * <code>peek()</code> -- retrieve data from a given stack index
//
//
// originally, I port libqueue from another github developer projects to use leveldb, but I found
// that queue_pop was extremely slow. I actually don't need or want the level's  or key value aspects
// of level db so I wrote this as simple file based replacement.
//
// Back ground
// ============
// I needed a simple durable fast queuing solution for one of my projects. I found a simple durable queue api at https://github.com/2ion/libqueue which was using Tokyo Cabinet for storage of which I didn't like the licensing. I ported it to use leveldb and found the performance for deleting (popping) records was extremely slow:
// [:~/libqueue] master+* ± ./bench
// it took 0 to push 10485760(inf) in 1024(inf) blocks
// it took 36 to pop 10485760(291271) in 1024(28.4444) blocks
// https://github.com/acgreek/libqueue
// so replaced leveldb with a simpl file based back-end
//
// [:~/cqueue] master(+1/-1)* ± ./bench
// it took 0 to push 104857600(inf) in 1024(inf) blocks
// it took 1 to pop 104857600(1.04858e+08) in 1024(1024) blocks
// https://github.com/acgreek/cqueue
// note: I ran these tests on my hp chromebook running cruton linux
//
//
//
// API Overview
// ============
//
// ```C
// /*
//   Intransparent data type. Nothing to see here...
// */
// Queue;
//
// /*
//   Data structure used to store data in, and retrieve data from the
//   queue
// */
// QueueData {
//   void *v;
//   size_t vlen;
// };
//
// /*
//   Functions return LIBQUEUE_SUCCESS on success, LIBQUEUE_FAILURE on
//   error or LIBQUEUE_MEM_ERROR as a special failure type for failed
//   memory allocations.
// */
//
// /*
//   Open a named queue. Directory location is provided as string
//   <id>. Queues are persistent and data stored previously will still be
//   available through the name it was written to. Operations on a queue
//   must conclude with a call to queue_close().
//   This function ensures that ${path}/libqueue exist. If it doesn't
//   and cannot be created OR not written to, this function fails.
// */
// Queue *  queue_open(const char *path);
//
//
// /*
//   Same as queue_open but allow one to provde options for various settings
//   The vararg must be terminated by a NULL to indicate end of options
//
//   options are to be provided as strings with some requiring args
//
//   available options:
//   maxBinLogSize     byte size of the binlog, smaller binlogs cost performance in that as the queue grows and strinks, the more files will be created and destroyed. Larger binlog will use up more disk space for when the entries are deleted
//   maxEntries        Max number of entries in the queue. queue_push will return LIBQUEUE_FAILURE if attempt is made to exceed this limit
//   maxSizeInByte     Max number of appropriate disk usage of queue. queue_push will return LIBQUEUE_FAILURE if attempt is made to exceed this limit
//
//  */
// Queue *  queue_open_with_options(const char *path,..., NULL);
//
//
// /*
//   Push a data object onto the queue. QueueData must be filled
//   appropriately: d->v is a void* pointer to the data, d->vlen holds the
//   data size.
//
// */
// int queue_push(Queue *q, QueueData *d);
//
// /*
//   Pop data from the queue. The library will store the data in the passed
//   QueueData struct. d->v must be freed by the user.
// */
// int queue_pop(Queue *q, QueueData *d);
//
// /*
//   returns total size of all files used by the queue unless there are zero elements
//   in which it will return 0
//
// */
// int queue_len(Queue *q, int64_t *len);
//
// /*
//   returns numbers of elements in the queue
// */
// int queue_count(Queue *q, int64_t *countp);
//
// /*
//   compacts the queue on file system to use take up less disk space if possible
// */
// int queue_compact(Queue *q);
// /*
//   Like queue_pop(), but doesn't remove the data object from the queue
//   and returns the data at queue position s. Indexing is not used
// */
// int queue_peek(Queue *q, int64_t s, QueueData *d);
//
// /*
//    Like queue_push(), but over-writes the data at queue position s. This
//    function cannot be used to append new data to the queue. Indexing
//    starts from zero. s must be positive.
//  */
// int queue_poke(Queue *q, int64_t s, QueueData *d);
//
//
// /*
//   Closes the queue. Only AFTER this function has been called one can be
//   sure that the data has been properly stored. Undefined behaviour may
//   occur if the concluding call to this function is omitted.
// */
// int queue_close(Queue *q);
// ```
//
// <code>Queue</code> is an opaque data type the fields of which
// should not be accessed directly.
//
// Building
// ========
//
// Dependencies:
// none
//
// libqueue uses cmake for building:
//
// cmake .
// make
//
// Usage
// =====
//
// It's very simple: Use
//
// ```C
// #include <queue.h>
// ```
//
// in your C program and link it with
//
// ```sh
// -lqueue
// ```
// given that libqueue was installed into the default linker search path.
//
// queueutils
// ==========
//
// The library comes with a number of reference/test/practical utilities
// that all operate on the queue named 'queueutils-stack0' to be used on
// the command line:
//
// qpush
// -----
//
// Pushes all its arguments in onto the queue
// </code>queueutils-stack0</code>. Exits with an error code of 0 if the
// push was successful and 1 if it wasn't.
//
// qpop
// ----
//
// Pops the strings from the queue that were pushed using
// <code>qpush</code>, on element per call to qpush. If there is no element
// to pop, exits with the status code 1. If the pop was successfu, it
// prints the string to stdout and exists with the status code 0.
//
// qlen
// ----
//
// if there are no entries, then zero is returned or else the total size of all file used by the queue is returned
//
// qcount
// ----
// number of elments in the queue
//
//
// qcompact
// ----
// not used
//
// qrepair
// ----
// not implemented
//
//
// qpeek
// -----
//
// Peek at elements in the queueutils' queue.
// ```sh
// qpeek <INDEX> [<index1>[..<indexN>]]</indexN>]]
// ```
// Tries to peek at all indizes listed as arguments. Skips over invalid indizes but fails if the queue cannot be accessed properly.
//
// qpoke
// -----
//
// Replace an existing in the queue with another one.
//
// qpoke <INDEX> <NEW VALUE>
// Fails if we cannot poke the element at the specified index, or if the index is out of bounds.
//
// </NEW></INDEX>
//
// qtest
// -----
// executes some basic tests
//

#endif /* CQUEUE_H */
