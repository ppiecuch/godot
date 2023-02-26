/**************************************************************************/
/*  semaphore_posix.cpp                                                   */
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

#if defined(UNIX_ENABLED) || defined(PTHREAD_ENABLED)

#include "semaphore_posix.h"

#include "core/os/memory.h"

#include <errno.h>
#include <stdio.h>

#if defined(OSX_ENABLED) || defined(IOS_ENABLED)
#include <fcntl.h>
#include <unistd.h>

static void cgsem_init(cgsem_t *cgsem) {
	int flags, fd, i;

	pipe(cgsem->pipefd);

	/* Make the pipes FD_CLOEXEC to allow them to close should we call
	 * execv on restart. */
	for (i = 0; i < 2; i++) {
		fd = cgsem->pipefd[i];
		flags = fcntl(fd, F_GETFD, 0);
		flags |= FD_CLOEXEC;
		fcntl(fd, F_SETFD, flags);
	}
}

static Error cgsem_post(cgsem_t *cgsem) {
	const char buf = 1;
	return write(cgsem->pipefd[1], &buf, 1) == -1 ? OK : FAILED;
}

static Error cgsem_wait(cgsem_t *cgsem) {
	char buf;
	return read(cgsem->pipefd[0], &buf, 1) == -1 ? OK : FAILED;
}

static void cgsem_destroy(cgsem_t *cgsem) {
	close(cgsem->pipefd[1]);
	close(cgsem->pipefd[0]);
}
#endif

Error SemaphorePosix::wait() const {
#if defined(OSX_ENABLED) || defined(IOS_ENABLED)
	return cgsem_wait(&sem);
#else
	while (sem_wait(&sem)) {
		if (errno == EINTR) {
			errno = 0;
			continue;
		} else {
			perror("sem waiting");
			return ERR_BUSY;
		}
	}
	return OK;
#endif
}

Error SemaphorePosix::post() const {
#if defined(OSX_ENABLED) || defined(IOS_ENABLED)
	return cgsem_post(&sem);
#else
	return (sem_post(&sem) == 0) ? OK : ERR_BUSY;
#endif
}
int SemaphorePosix::get() const {
#if defined(OSX_ENABLED) || defined(IOS_ENABLED)
	return 0;
#else
	int val;
	sem_getvalue(&sem, &val);

	return val;
#endif
}

SemaphorePosix::SemaphorePosix() {
#if defined(OSX_ENABLED) || defined(IOS_ENABLED)
	cgsem_init(&sem);
#else
	int r = sem_init(&sem, 0, 0);
	if (r != 0)
		perror("sem creating");
#endif
}

SemaphorePosix::~SemaphorePosix() {
#if defined(OSX_ENABLED) || defined(IOS_ENABLED)
	cgsem_destroy(&sem);
#else
	sem_destroy(&sem);
#endif
}

#endif
