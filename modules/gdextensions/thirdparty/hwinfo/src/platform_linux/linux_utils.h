// kate: replace-tabs on; tab-indents on; tab-width 2; indent-width 2; indent-mode cstyle;

// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#ifndef LINUX_UTILS_H
#define LINUX_UTILS_H

#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define EINTR_LOOP(var, cmd) do { var = cmd; } while (var == -1 && errno == EINTR)

#define _safe_open64 ::open
#define _safe_read ::read
#define _safe_close ::close

#if defined(HWINFO_X86_32) && defined(__GLIBC__)
# if !__GLIBC_PREREQ(2, 22)
// glibc prior to release 2.22 had a bug that suppresses the third argument to open() / open64() / openat(),
// causing file creation with O_TMPFILE to have the wrong permissions. So we bypass the glibc implementation and go straight
// for the syscall. See https://sourceware.org/git/?p=glibc.git;a=commit;h=65f6f938cd562a614a68e15d0581a34b177ec29d
#undef _safe_open64
static inline int _safe_open64(const char *pathname, int flags, mode_t mode) { return syscall(SYS_open, pathname, flags | O_LARGEFILE, mode); }
# endif
#endif

static inline int io_safe_open(const char *pathname, int flags, mode_t mode = 0777) {
#ifdef O_CLOEXEC
  flags |= O_CLOEXEC;
#endif
  int fd;
  EINTR_LOOP(fd, _safe_open64(pathname, flags, mode));
#ifndef O_CLOEXEC
  if (fd != -1) {
    ::fcntl(fd, F_SETFD, FD_CLOEXEC);
  }
#endif
  return fd;
}

static inline int64_t io_safe_read(int fd, void *data, int64_t maxlen) {
  int64_t ret = 0;
  EINTR_LOOP(ret, _safe_read(fd, data, maxlen));
  return ret;
}

static inline int io_safe_close(int fd) {
  int ret;
  EINTR_LOOP(ret, _safe_close(fd));
  return ret;
}

#endif // LINUX_UTILS_H
