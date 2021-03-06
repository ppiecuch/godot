/*************************************************************************/
/*  simple_shared_mutex.h                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

/**
 * shared_mutex.h
 * Alternative implementations of shared mutex for C++11. Use C++14 std::shared_timed_mutex and
 * C++17 std::shared_mutex if available.
 */
#ifndef C11_SHARED_MUTEX_H
#define C11_SHARED_MUTEX_H

#include <condition_variable>
#include <mutex>

namespace c11 {

/**
 * An implementation of shared timed mutex that satisfies C++14 SharedTimedMutex concept and does
 * not starve readers or writers.
 */
class SharedTimedMutex {
public:
	SharedTimedMutex() = default;

	SharedTimedMutex(const SharedTimedMutex &) = delete;
	SharedTimedMutex &operator=(const SharedTimedMutex &) = delete;

	// Execlusive lock
	void lock() {
		std::unique_lock<std::mutex> lock(mtx);
		while (state & WRITER_ENTERED_MASK) {
			rgate.wait(lock);
		}
		state |= WRITER_ENTERED_MASK;
		while (state & NUM_READER_MASK) {
			wgate.wait(lock);
		}
	}

	void unlock() {
		{
			std::lock_guard<std::mutex> lock(mtx);
			state &= ~WRITER_ENTERED_MASK; // should always set to 0
		}
		rgate.notify_all();
	}

	bool try_lock() {
		std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
		if (lock.owns_lock() && state == 0) {
			state = WRITER_ENTERED_MASK;
			return true;
		} else {
			return false;
		}
	}

	template <class Rep, class Period>
	bool try_lock_for(const std::chrono::duration<Rep, Period> &timeout_duration) {
		return try_lock_until(std::chrono::steady_clock::now() + timeout_duration);
	}

	template <class Clock, class Duration>
	bool try_lock_until(const std::chrono::time_point<Clock, Duration> &timeout_time) {
		// Untimed mutex blocking, but mutex normally should not take long to acquire
		std::unique_lock<std::mutex> lock(mtx);
		if (!rgate.wait_until(lock, timeout_time,
					[this]() { return !(state & WRITER_ENTERED_MASK); })) {
			return false;
		}
		state |= WRITER_ENTERED_MASK;
		if (!wgate.wait_until(lock, timeout_time,
					[this]() { return !(state & NUM_READER_MASK); })) {
			state &= ~WRITER_ENTERED_MASK;
			lock.unlock();
			rgate.notify_all();
			return false;
		}
		return true;
	}

	void lock_shared() {
		std::unique_lock<std::mutex> lock(mtx);
		while ((state & WRITER_ENTERED_MASK) || ((state & NUM_READER_MASK) == NUM_READER_MASK)) {
			rgate.wait(lock);
		}
		state += 1;
	}

	void unlock_shared() {
		std::unique_lock<std::mutex> lock(mtx);
		state -= 1;
		uint_fast32_t num_readers_left = state & NUM_READER_MASK;
		if ((state & WRITER_ENTERED_MASK) && (num_readers_left == 0)) {
			lock.unlock();
			wgate.notify_one();
		} else if (num_readers_left == NUM_READER_MASK - 1) {
			lock.unlock();
			rgate.notify_one();
		}
	}

	bool try_lock_shared() {
		std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
		if (lock.owns_lock() && !(state & WRITER_ENTERED_MASK) && ((state & NUM_READER_MASK) != NUM_READER_MASK)) {
			state += 1;
			return true;
		} else {
			return false;
		}
	}

	template <class Rep, class Period>
	bool try_lock_shared_for(const std::chrono::duration<Rep, Period> &timeout_duration) {
		return try_lock_shared_until(std::chrono::steady_clock::now() + timeout_duration);
	}

	template <class Clock, class Duration>
	bool try_lock_shared_until(const std::chrono::time_point<Clock, Duration> &timeout_time) {
		std::unique_lock<std::mutex> lock(mtx);
		if (!rgate.wait_until(lock, timeout_time,
					[this]() { return !(state & WRITER_ENTERED_MASK) &&
									  ((state & NUM_READER_MASK) != NUM_READER_MASK); })) {
			return false;
		}
		state += 1;
		return true;
	}

private:
	static const uint_fast32_t WRITER_ENTERED_MASK = 1U << 31;
	static const uint_fast32_t NUM_READER_MASK = WRITER_ENTERED_MASK - 1;

	// Main lock
	std::mutex mtx;

	// Readers pass this gate has shared ownership. New readers and writers wait at this gate
	// if one writer has passed rgate.
	std::condition_variable rgate;

	// Writer pass this gate has exclusive ownership. Only the one writer passed rgate waits
	// at this gate for all remaining readers to leave.
	std::condition_variable wgate;

	// Combined state: The highest bit indicated weather a writer has entered (i.e. passed rgate),
	// lower 31 bits is number of active readers.
	uint_fast32_t state = 0;
};

/**
 * An alternative implementation of C++14 SharedTimedMutex concept.
 * This implementation has higher read throughput when there are more readers but may
 * starves writers.
 * Use SharedTimedMutex instead if there is a chance that starvation is of concern.
 */
class ReaderPreferringSharedTimedMutex {
public:
	ReaderPreferringSharedTimedMutex() = default;

	ReaderPreferringSharedTimedMutex(const ReaderPreferringSharedTimedMutex &) = delete;
	ReaderPreferringSharedTimedMutex &operator=(const ReaderPreferringSharedTimedMutex &) = delete;

	void lock() {
		std::unique_lock<std::mutex> lock(mtx);
		while (state) {
			cv.wait(lock);
		}
		state = WRITER_ACTIVE_MASK;
	}

	void unlock() {
		{
			std::lock_guard<std::mutex> lock(mtx);
			state = 0;
		}
		cv.notify_all();
	}

	bool try_lock() {
		std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
		if (lock.owns_lock() && state == 0) {
			state = WRITER_ACTIVE_MASK;
			return true;
		} else {
			return false;
		}
	}

	template <class Rep, class Period>
	bool try_lock_for(const std::chrono::duration<Rep, Period> &timeout_duration) {
		return try_lock_until(std::chrono::steady_clock::now() + timeout_duration);
	}

	template <class Clock, class Duration>
	bool try_lock_until(const std::chrono::time_point<Clock, Duration> &timeout_time) {
		std::unique_lock<std::mutex> lock(mtx);
		if (!cv.wait_until(lock, timeout_time, [this]() { return state == 0; })) {
			return false;
		}
		state = WRITER_ACTIVE_MASK;
		return true;
	}

	void lock_shared() {
		std::unique_lock<std::mutex> lock(mtx);
		while ((state & WRITER_ACTIVE_MASK) || ((state & NUM_READER_MASK) == NUM_READER_MASK)) {
			cv.wait(lock);
		}
		state += 1;
	}

	void unlock_shared() {
		std::unique_lock<std::mutex> lock(mtx);
		state -= 1;
		if (state == 0) {
			lock.unlock();
			cv.notify_all();
		}
	}

	bool try_lock_shared() {
		std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
		if (lock.owns_lock() && !(state & WRITER_ACTIVE_MASK) &&
				((state & NUM_READER_MASK) != NUM_READER_MASK)) {
			state += 1;
			return true;
		} else {
			return false;
		}
	}

	template <class Rep, class Period>
	bool try_lock_shared_for(const std::chrono::duration<Rep, Period> &timeout_duration) {
		return try_lock_shared_until(std::chrono::steady_clock::now() + timeout_duration);
	}

	template <class Clock, class Duration>
	bool try_lock_shared_until(const std::chrono::time_point<Clock, Duration> &timeout_time) {
		std::unique_lock<std::mutex> lock(mtx);
		if (!cv.wait_until(lock, timeout_time,
					[this]() { return !(state & WRITER_ACTIVE_MASK) &&
									  ((state & NUM_READER_MASK) != NUM_READER_MASK); })) {
			return false;
		}
		state += 1;
		return true;
	}

private:
	static const uint_fast32_t WRITER_ACTIVE_MASK = 1U << 31;
	static const uint_fast32_t NUM_READER_MASK = WRITER_ACTIVE_MASK - 1;

	std::mutex mtx;
	std::condition_variable cv;
	uint_fast32_t state = 0;
};

/**
 * An alternative implementation of C++14 shared_lock. Locks underlying shared mutex in shared
 * ownership mode.
 */
template <typename Mutex>
class SharedLock {
public:
	// Shared locking

	SharedLock() noexcept : mtx(nullptr), owns(false) {
	}

	SharedLock(SharedLock const &) = delete;
	SharedLock &operator=(SharedLock const &) = delete;

	SharedLock(SharedLock &&rhs) noexcept : SharedLock() {
		swap(rhs);
	}

	explicit SharedLock(Mutex &mutex) :
			mtx(&mutex), owns(true) {
		mutex.lock_shared();
	}

	SharedLock(Mutex &mutex, std::defer_lock_t) noexcept : mtx(&mutex), owns(false) {
	}

	SharedLock(Mutex &mutex, std::try_to_lock_t) :
			mtx(&mutex), owns(mutex.try_lock_shared()) {
	}

	SharedLock(Mutex &mutex, std::adopt_lock_t) noexcept : mtx(&mutex), owns(true) {
	}

	template <typename Rep, typename Period>
	SharedLock(Mutex &mutex, const std::chrono::duration<Rep, Period> &timeout_duration) :
			mtx(&mutex) {
		owns = mtx->try_lock_shared_for(timeout_duration);
	}

	template <typename Clock, typename Duration>
	SharedLock(Mutex &mutex, const std::chrono::time_point<Clock, Duration> &timeout_time) :
			mtx(&mutex) {
		owns = mtx->try_lock_shared_until(timeout_time);
	}

	~SharedLock() {
		if (owns) {
			mtx->unlock_shared();
		}
	}

	SharedLock &operator=(SharedLock &&rhs) noexcept {
		SharedLock tmp(std::move(rhs));
		tmp.swap(*this);
		return *this;
	}

	void lock() {
		mtx->lock_shared();
		owns = true;
	}

	void unlock() {
		mtx->unlock_shared();
		owns = false;
	}

	bool try_lock() {
		owns = mtx->try_lock_shared();
		return owns;
	}

	template <typename Rep, typename Period>
	bool try_lock_for(const std::chrono::duration<Rep, Period> &timeout_duration) {
		owns = mtx->try_lock_shared_for(timeout_duration);
		return owns;
	}

	template <typename Clock, typename Duration>
	bool try_lock_until(const std::chrono::time_point<Clock, Duration> &timeout_duration) {
		owns = mtx->try_lock_shared_until(timeout_duration);
		return owns;
	}

	void swap(SharedLock &rhs) noexcept {
		std::swap(mtx, rhs.mtx);
		std::swap(owns, rhs.owns);
	}

	Mutex *release() noexcept {
		owns = false;
		Mutex *ret = mtx;
		mtx = nullptr;
		return ret;
	}

	bool owns_lock() const noexcept {
		return owns;
	}

	explicit operator bool() const noexcept {
		return owns;
	}

	Mutex *mutex() const noexcept {
		return mtx;
	}

private:
	Mutex *mtx;
	bool owns;
};

} // namespace c11

#endif /* C11_SHARED_MUTEX_H */
