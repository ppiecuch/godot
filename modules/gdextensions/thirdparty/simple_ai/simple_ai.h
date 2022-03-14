/**
 * @mainpage SimpleAI documentation
 *
 * @section purpose Purpose
 *
 * SimpleAI is a small and (hopefully) easy to use C++ library for behaviour
 * tree based @ai{AI}. It's main focus is about games - but of course it can be
 * used for other things, too. The project is released under the MIT license
 * and thus can be used for open source as well as commercial and closed
 * source projects. If you use it, it would be nice to provide a link to the
 * github page.
 *
 * - [GitHub page](http://github.com/mgerhardy/simpleai/)
 * - Small example use case:
 *   - https://github.com/mgerhardy/engine/tree/master/src/modules/backend/entity/ai
 *
 * @section features Features
 *
 * * Header only c++11
 * * Threadsafe (hopefully, checked with hellgrind, tsan and other tools)
 * * LUA and XML script interface to create the trees - but every other data source
 *   is possible, too. The LUA and XML script interfaces are just implemented as an
 *   [example](https://github.com/mgerhardy/simpleai/blob/master/src/run/behaviours.lua)
 *   even though they are of course fully usable to create behaviour trees.
 * * @ref Aggro list implementation
 * * Several standard selectors and conditions (see below)
 * * Group management
 * * Movement implementations for steering
 * * Network based remote @ref debugging with live editing of the behaviour tree
 * * QT5 based remote debugger
 *   * can also be used for other AI implementations. (E.g. there are java protocol classes)
 *   * [example](https://github.com/mgerhardy/simpleai/blob/master/contrib/exampledebugger) on how to extend the debugger to render application specific stuff into the map view.
 * * @ref Zone support (each zone can be debugged separately)
 * * @ref Filter support
 *
 * As a default set of conditions, nodes and filters that SimpleAI already comes with:
 * * Conditions
 *   * @ai{And}
 *   * @ai{False}
 *   * @ai{Filter}
 *   * @ai{HasEnemies}
 *   * @ai{IsCloseToGroup}
 *   * @ai{IsGroupLeader}
 *   * @ai{IsInGroup}
 *   * @ai{Not}
 *   * @ai{Or}
 *   * @ai{True}
 * * Nodes
 *   * @ai{Fail}
 *   * @ai{Idle}
 *   * @ai{Invert}
 *   * @ai{Limit}
 *   * @ai{Parallel}
 *   * @ai{PrioritySelector}
 *   * @ai{ProbabilitySelector}
 *   * @ai{RandomSelector}
 *   * @ai{Sequence}
 *   * @ai{Steer}
 *   * @ai{Succeed}
 * * Filter
 *   * @ai{Complement}
 *   * @ai{Difference}
 *   * @ai{First} - only put the first entry of a filter to the list
 *   * @ai{Intersection} - intersection between several filter results
 *   * @ai{Last} - only put the last entry of a filter to the list
 *   * @ai{Random} - only preserve n random entries of the sub filters
 *   * @ai{SelectAll} - this filter is a nop - it will just use the already filtered entities
 *   * @ai{SelectEmpty} - clears the selection
 *   * @ai{SelectGroupLeader}
 *   * @ai{SelectGroupMembers} - select all the group members of a specified group
 *   * @ai{SelectHighestAggro} - put the highest @ref Aggro @ai{CharacterId} into the selection
 *   * @ai{SelectZone} - select all known entities in the zone
 *   * @ai{Union} - merges several other filter results
 * * Steering
 *   * @movement{GroupFlee}
 *   * @movement{GroupSeek}
 *   * @movement{SelectionFlee}
 *   * @movement{SelectionSeek}
 *   * @movement{TargetFlee}
 *   * @movement{TargetSeek}
 *   * @movement{Wander}
 *
 * @section compilation Compile the lib:
 *
 * * autotools based compilation
 *   * `./autogen.sh`
 *   * `./configure`
 *   * `make`
 *   * `make install`
 * * cmake based compilation (enable tests, remote debugger and so on via options)
 *   * `cmake CMakeLists.txt`
 *   * `make`
 * * Compile the remote debugger
 *   * qmake
 *     * `cd src/debug`
 *     * `qmake`
 *     * `make`
 *   * cmake based compilation
 *     * `cmake CMakeLists.txt -DSIMPLEAI_DEBUGGER=ON`
 *     * `make`
 *
 * @section run Running it:
 *
 * SimpleAI comes with a small tool that is located in src/run. You can
 * execute your own trees with:
 *
 * - `./simpleai-run -file src/run/behaviours.lua`
 *
 * After you ran it, you can connect with the remote @ref debugger and inspect the live
 * state of every spawned @ai{AI} entity.
 *
 * @section using Using it:
 * * Make sure your character extends @ai{ICharacter} or includes it as component.
 * * Implement your behaviour tree loader by extending the class @ai{ITreeloader}.
 * * Extend the @debug{AIDebugger} to deliver your own @debug{MapView} that renders the map of
 *   your application.
 * * Add your own condition, filter and task factories to the @ai{AIRegistry} or via @ai{LUAAIRegistry}.
 * * Assign attributes to your characters that should be shown in the
 *   debuggers live view.
 *
 * As the name states, it should be easy to use and to integrate into your application. You
 * have the ability to create new customized actions, override existing ones with your
 * own implementations and so on.
 *
 * To integrate the @ai{AI} into your application, your entity class should implement or include
 * the @ai{ICharacter} interface. You only have to call the @ai{ICharacter::update()} method
 * to get the @ai{AI} and the character updated. You can view the included `simpleai-run` tool
 * for getting an idea on how to do this.
 *
 * Once this step is done, you can go on with creating new actions for your application. All you
 * have to do for this is to extend @ai{ITask}. The entity instance given to the
 * @ai{ITask::doAction()} method contains the @ai{ICharacter} that you bound to your
 * application entity.
 *
 * After implementing these actions, all you have to do in order to use them with e.g. the
 * existing @ai{LUATreeLoader} is to add them to the registry. Just call
 * @ai{AIRegistry::registerNodeFactory()} on your @ai{AIRegistry} instance and you are ready
 * to write @ai{LUA} scripts with it. Again, as a reference, just check out the example code.
 * You can also create nodes, conditions, filter and steering methods via LUA directly. See
 * @ai{LUAAIRegistry} for more information about this.
 *
 * Note the usage of a few macros that makes your life easier:
 * * @ai{TASK_CLASS}
 * * @ai{NODE_FACTORY}
 * * @ai{CONDITION_CLASS}
 * * @ai{CONDITION_FACTORY}
 *
 * Now you only have to do:
 * @code
 * ai::AIRegistry registry;
 * registry.registerNodeFactory("ExampleTask", ai::example::ExampleTask::FACTORY);
 *
 * ai::LUATreeLoader loader(registry);
 * loader.init(allMyLuaBehaviourTreesInThisString);
 * const ai::TreeNodePtr& root = loader.load("BehaviourNameEgDefensiveBehaviour");
 * @endcode
 *
 * The root node created by the load method should be given to your @ai{ICharacter}
 * implementation which holds an instance of the @ai{AI} class. Each SimpleAI controlled
 * entity in your world will have one of them.
 *
 * @section debugging Remote Debugging
 *
 * @image html aidebugger.png
 *
 * The remote debugger can also render your custom map widget which allows
 * you to show the characters in their "natural" environment instead of on
 * a boring plane. You can choose which entities should be available for
 * remote debugging. The included `Server` class handles the serialization
 * of the entities. You can create one instance of a server per application map
 * you have. Once you add an entity to the server, it gets automatically
 * serialized and is broadcasted to all connected clients. If no client is
 * connected, nothing is serialized and thus the remote debugging has almost
 * no performance influence.
 *
 * The remote debugging can only be active for one `Zone`. So every other
 * zone is also not affected by the debugging overhead. Usually you would
 * split a zone by logical units, like one map is one zone. But you can also
 * split them up into logical units like boss ai, supporter ai, ... and so on.
 *
 * Features so far:
 * * Render the behaviour tree (either in a table, or as a node tree)
 * * Show the @ref Aggro state
 * * Show the attributes that are assigned to an entity (ai::ICharacter::setAttribute())
 * * Pause execution of a zone
 * * Single step in a @ref Zone (after it was paused)
 * * Reset states of the @ai{AI} in a @ref Zone
 * * Live editing of the behaviour tree (update, add, remove)
 *
 * Examples on how to customize the debugger
 * * [some classes](https://github.com/mgerhardy/simpleai/blob/master/contrib/exampledebugger) that provide a custom map view and map item rendering with custom data from attributes.
 *
 * @section legal Legal
 *
 * Copyright (C) 2015-2017 Martin Gerhardy <martin.gerhardy@gmail.com>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/**
 * @file
 *
 * @defgroup AI
 * @{
 *
 * Main header file that you should include when you use SimpleAI.
 *
 * If you also want to use the default loaders for your behaviour trees, you can
 * include the following loader header files in your code:
 * @code{.cpp}
 * #include <tree/loaders/lua/LUATreeLoader.h>
 * #include <tree/loaders/xml/XMLTreeLoader.h>
 * @endcode
 * or define AI_INCLUDE_LUA and/or AI_INCLUDE_XML
 *
 * You can control how to allocate memory by defining your own allocator class via @c AI_ALLOCATOR_CLASS
 *
 * SimpleAI uses a left handed coordinate system with y pointing upwards (meaning, if
 * you are using a 2d application, only handle x and z).
 */
#pragma once

// #include "common/Types.h"
/**
 * @file
 */


// #include "Log.h"
/**
 * @file
 */


#ifndef SIMPLEAI_SKIP_LOG

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

namespace ai {
namespace Log {

#ifdef _MSC_VER
#define __attribute__(x)
#endif

static constexpr int bufSize = 1024;

static inline void trace(const char* msg, ...) __attribute__((format(printf, 1, 2)));
static inline void trace(const char* msg, ...) {
	va_list args;
	va_start(args, msg);
	char buf[bufSize];
	vsnprintf(buf, sizeof(buf), msg, args);
	printf("TRACE: %s\n", buf);
	va_end(args);
}

static inline void debug(const char* msg, ...) __attribute__((format(printf, 1, 2)));
static inline void debug(const char* msg, ...) {
	va_list args;
	va_start(args, msg);
	char buf[bufSize];
	vsnprintf(buf, sizeof(buf), msg, args);
	printf("DEBUG: %s\n", buf);
	va_end(args);
}

static inline void info(const char* msg, ...) __attribute__((format(printf, 1, 2)));
static inline void info(const char* msg, ...) {
	va_list args;
	va_start(args, msg);
	char buf[bufSize];
	vsnprintf(buf, sizeof(buf), msg, args);
	printf("INFO: %s\n", buf);
	va_end(args);
}

static inline void warn(const char* msg, ...) __attribute__((format(printf, 1, 2)));
static inline void warn(const char* msg, ...) {
	va_list args;
	va_start(args, msg);
	char buf[bufSize];
	vsnprintf(buf, sizeof(buf), msg, args);
	printf("WARN: %s\n", buf);
	va_end(args);
}

static inline void error(const char* msg, ...) __attribute__((format(printf, 1, 2)));
static inline void error(const char* msg, ...) {
	va_list args;
	va_start(args, msg);
	char buf[bufSize];
	vsnprintf(buf, sizeof(buf), msg, args);
	printf("ERROR: %s\n", buf);
	va_end(args);
}

}
}
#endif

#include <string>
#include <unordered_map>
#include <cassert>
#include <cstdio>

/**
 * @brief Logging macro to provide your own loggers
 */
#ifndef ai_log
#define ai_log(...) ai::Log::info(__VA_ARGS__)
#endif

/**
 * @brief Logging macro to provide your own loggers
 */
#ifndef ai_log_error
#define ai_log_error(...) ai::Log::error(__VA_ARGS__)
#endif

/**
 * @brief Logging macro to provide your own loggers
 */
#ifndef ai_log_warn
#define ai_log_warn(...) ai::Log::warn(__VA_ARGS__)
#endif

/**
 * @brief Logging macro to provide your own loggers
 */
#ifndef ai_log_debug
#define ai_log_debug(...) ai::Log::debug(__VA_ARGS__)
#endif

/**
 * @brief Logging macro to provide your own loggers
 */
#ifndef ai_log_trace
#define ai_log_trace(...) ai::Log::trace(__VA_ARGS__)
#endif

#if !(__GNUC__ || __GNUC__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

/**
 * @brief Provide your own assert - this is also executed in non DEBUG mode
 */
#ifndef ai_assert_always
	#ifdef __clang_analyzer__
		#define ai_assert_always(condition, ...) assert(condition)
	#else
		#define ai_assert_always(condition, ...) \
			if ( !(condition) ) { \
				ai_log_error(__VA_ARGS__); \
				ai_log_error("%s:%i", __FILE__, __LINE__); \
				assert(condition); \
			}
	#endif
#endif

/**
 * @brief Provide your own assert - this is only executed in DEBUG mode
 * @see ai_assert_always
 */
#ifndef ai_assert
	#ifdef DEBUG
		#define ai_assert ai_assert_always
	#else
		#define ai_assert(condition, ...)
	#endif
#endif

/**
 * @brief If you compile with RTTI activated, you get additional sanity checks when using this
 * macro to perform your @c static_cast calls
 */
template<class T, class S>
inline T ai_assert_cast(const S object) {
#ifdef __cpp_rtti
	ai_assert(dynamic_cast<T>(object) == static_cast<T>(object), "Types don't match");
#endif
	return static_cast<T>(object);
}

#define AI_STRINGIFY_INTERNAL(x) #x
#define AI_STRINGIFY(x) AI_STRINGIFY_INTERNAL(x)

/**
 * @brief If you want to use exceptions in your code and want them to be catched by the library
 * just set this to 1
 */
#ifndef AI_EXCEPTIONS
#define AI_EXCEPTIONS 0
#endif

/**
 * @brief Enable lua sanity checks by default
 */
#ifndef AI_LUA_SANTITY
#define AI_LUA_SANTITY 1
#endif

#ifdef _WIN32
#	ifdef SIMPLEAI_EXPORT
#		define SIMPLEAI_LIB __declspec(dllexport)
#	elif defined(SIMPLEAI_IMPORT)
#		define SIMPLEAI_LIB __declspec(dllimport)
#	else
#		define SIMPLEAI_LIB
#	endif
#else
#	define SIMPLEAI_LIB
#endif

namespace ai {

/**
 * @brief Defines the type of the id to identify an ai controlled entity.
 *
 * @note @c -1 is reserved. You should use ids >= 0
 * @sa NOTHING_SELECTED
 */
typedef int CharacterId;
#define PRIChrId PRId32

/**
 * @brief ICharacter attributes for the remote \ref debugger
 */
typedef std::unordered_map<std::string, std::string> CharacterAttributes;

}

// #include "common/MemoryAllocator.h"
/**
 * @file
 */


#include <memory>

namespace ai {

class _DefaultAllocator {
private:
	_DefaultAllocator() {
	}
public:
	static inline void* allocate(size_t count) {
		void* ptr = new unsigned char[count];
		return ptr;
	}

	static inline void deallocate(void* ptr) {
		delete[] ((unsigned char*) ptr);
	}
};

template<class AllocatorClass>
class _MemObject {
public:
	explicit _MemObject() {
	}

	virtual ~_MemObject() {
	}

	inline void* operator new(size_t size) {
		return AllocatorClass::allocate(size);
	}

	inline void* operator new(size_t, void* ptr) {
		return ptr;
	}

	inline void* operator new[](size_t size) {
		return AllocatorClass::allocate(size);
	}

	inline void operator delete(void* ptr) {
		AllocatorClass::deallocate(ptr);
	}

	inline void operator delete(void* ptr, void*) {
		AllocatorClass::deallocate(ptr);
	}

	inline void operator delete[](void* ptr) {
		AllocatorClass::deallocate(ptr);
	}
};

/**
 * @brief define the macro @c AI_ALLOCATOR_CLASS with your own allocator implementation. just create a class
 * with static functions for @c allocate and @c deallocate.
 */
#ifndef AI_ALLOCATOR_CLASS
#define AI_ALLOCATOR_CLASS _DefaultAllocator
#endif
/**
 * @brief Every object that is derived from @c MemObject is allocated with the @c AI_ALLOCATOR_CLASS allocator.
 */
typedef _MemObject<AI_ALLOCATOR_CLASS> MemObject;

}

// #include "common/String.h"
/**
 * @file
 */


// #include "Math.h"
/**
 * @file
 */


// #include "Types.h"


#define GLM_FORCE_RADIANS
//#define GLM_SWIZZLE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/norm.hpp>
#include <limits>
#include <math.h>

namespace ai {

inline float toRadians (float degree) {
	return glm::radians(degree);
}

inline bool isInfinite (const glm::vec3& vec) {
	return glm::any(glm::isinf(vec));
}

inline float toDegrees (float radians) {
	return glm::degrees(radians);
}

inline glm::vec3 fromRadians(float radians) {
	return glm::vec3(glm::cos(radians), 0.0f, glm::sin(radians));
}

inline float angle(const glm::vec3& v) {
	const float _angle = glm::atan(v.z, v.x);
	return _angle;
}

inline glm::vec3 advance (const glm::vec3& src, const glm::vec3& direction, const float scale) {
	return src + (scale * direction);
}

template<typename T>
inline T clamp(T a, T low, T high) {
	return glm::clamp(a, low, high);
}

static const glm::vec3 ZERO(0.0f);
static const glm::vec3 VEC3_INFINITE(std::numeric_limits<float>::infinity());

inline glm::vec3 parse(const std::string& in) {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

#ifdef _MSC_VER
	if (::sscanf_s(in.c_str(), "%f:%f:%f", &x, &y, &z) != 3) {
#else
	if (::sscanf(in.c_str(), "%f:%f:%f", &x, &y, &z) != 3) {
#endif
		return VEC3_INFINITE;
	}

	return glm::vec3(x, y, z);
}

}

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

namespace ai {
namespace Str {

inline std::string toString(const glm::vec3& pos) {
	char buf[128];
	std::snprintf(buf, sizeof(buf), "%f:%f:%f", pos.x, pos.y, pos.z);
	return buf;
}

inline bool startsWith(const std::string& string, const std::string& token) {
	return !string.compare(0, token.size(), token);
}

inline float strToFloat(const std::string& str) {
	return static_cast<float>(::atof(str.c_str()));
}

inline std::string eraseAllSpaces(const std::string& str) {
	if (str.empty())
		return str;
	std::string tmp = str;
	tmp.erase(std::remove(tmp.begin(), tmp.end(), ' '), tmp.end());
	return tmp;
}

inline void splitString(const std::string& string, std::vector<std::string>& tokens, const std::string& delimiters = "()") {
	// Skip delimiters at beginning.
	std::string::size_type lastPos = string.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos = string.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos) {
		// Found a token, add it to the vector.
		tokens.push_back(string.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = string.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = string.find_first_of(delimiters, lastPos);
	}
}

}

}

// #include "common/Math.h"

// #include "common/Random.h"
/**
 * @file
 */


/**
 * @file
 * @defgroup Random
 * @{
 * Everything that uses randomness in SimpleAI should use these helper methods.
 * You can query random values in a thread safe way and can modify the seed - which
 * really might ease debugging at some points.
 */

#include <chrono>
#include <random>
#include <algorithm>
#include <iterator>
#include <stdlib.h>
// #include "common/Thread.h"
/**
 * @file
 */


// #include "Types.h"

#include <thread>
#include <mutex>
#include <atomic>

namespace ai {

// TODO: not a real read-write-lock - maybe someday
class ReadWriteLock {
private:
	const std::string _name;
	mutable std::atomic_flag _locked = ATOMIC_FLAG_INIT;
public:
	ReadWriteLock(const std::string& name) :
			_name(name) {
	}

	inline void lockRead() const {
		while (_locked.test_and_set(std::memory_order_acquire)) {
			std::this_thread::yield();
		}
	}

	inline void unlockRead() const {
		_locked.clear(std::memory_order_release);
	}

	inline void lockWrite() {
		while (_locked.test_and_set(std::memory_order_acquire)) {
			std::this_thread::yield();
		}
	}

	inline void unlockWrite() {
		_locked.clear(std::memory_order_release);
	}
};

class ScopedReadLock {
private:
	const ReadWriteLock& _lock;
public:
	explicit ScopedReadLock(const ReadWriteLock& lock) : _lock(lock) {
		_lock.lockRead();
	}
	~ScopedReadLock() {
		_lock.unlockRead();
	}
};

class ScopedWriteLock {
private:
	ReadWriteLock& _lock;
public:
	explicit ScopedWriteLock(ReadWriteLock& lock) : _lock(lock) {
		_lock.lockWrite();
	}
	~ScopedWriteLock() {
		_lock.unlockWrite();
	}
};

#ifndef AI_THREAD_LOCAL
#define AI_THREAD_LOCAL thread_local
#endif

}


namespace ai {

inline std::default_random_engine& randomEngine() {
	AI_THREAD_LOCAL std::default_random_engine engine;
	return engine;
}

inline void randomSeed (unsigned int seed) {
	randomEngine().seed(seed);
}

inline float randomf (float max = 1.0f) {
	std::uniform_real_distribution<float> distribution(0.0, max);
	return distribution(randomEngine());
}

inline int random (int min, int max) {
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(randomEngine());
}

inline float randomBinomial (float max = 1.0f) {
	return randomf(max) - randomf(max);
}

template<typename I>
inline I randomElement(I begin, I end) {
	const int n = static_cast<int>(std::distance(begin, end));
	std::uniform_int_distribution<> dis(0, n - 1);
	std::advance(begin, dis(randomEngine()));
	return begin;
}

/**
 * @brief Helper function to cut the input vector down to @c n random elements.
 */
template<typename T>
inline void randomElements(std::vector<T>& vec, int n) {
	if (n >= (int)vec.size()) {
		return;
	}
	std::shuffle(vec.begin(), vec.end(), randomEngine());
	vec.resize(n);
}

template<typename I>
inline void shuffle(I begin, I end) {
	std::shuffle(begin, end, randomEngine());
}

}

/**
 * @}
 */

// #include "common/Log.h"

// #include "common/MoveVector.h"
/**
 * @file
 */


// #include "Math.h"


namespace ai {

class MoveVector {
protected:
	const glm::vec3 _vec3;
	const float _rotation;
public:
	MoveVector(const glm::vec3& vec3, float rotation) :
			_vec3(vec3), _rotation(rotation) {
	}

	MoveVector(const glm::vec3& vec3, double rotation) :
			_vec3(vec3), _rotation(static_cast<float>(rotation)) {
	}

	inline float getOrientation(float duration) const {
		const float pi2 = glm::two_pi<float>();
		const float rotation = _rotation + pi2;
		return fmodf(rotation * duration, pi2);
	}

	inline const glm::vec3& getVector() const {
		return _vec3;
	}

	inline glm::vec3 getVector() {
		return _vec3;
	}

	inline float getRotation() const {
		return _rotation;
	}

	inline operator glm::vec3() const {
		return _vec3;
	}

	inline operator const glm::vec3&() const {
		return _vec3;
	}

	inline operator float() const {
		return _rotation;
	}
};

}

// #include "common/Random.h"

// #include "common/Thread.h"

// #include "common/ThreadPool.h"
/*
 Copyright (c) 2012 Jakob Progsch, Vaclav Zeman

 This software is provided 'as-is', without any express or implied
 warranty. In no event will the authors be held liable for any damages
 arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:

 1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.

 2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original software.

 3. This notice may not be removed or altered from any source
 distribution.
 */



#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <functional>

namespace ai {

class ThreadPool final {
public:
	explicit ThreadPool(size_t);

	/**
	 * Enqueue functors or lambdas into the thread pool
	 */
	template<class F, class ... Args>
	auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

	~ThreadPool();
private:
	// need to keep track of threads so we can join them
	std::vector<std::thread> _workers;
	// the task queue
	std::queue<std::function<void()> > _tasks;

	// synchronization
	std::mutex _queueMutex;
	std::condition_variable _condition;
	std::atomic_bool _stop;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads) :
		_stop(false) {
	_workers.reserve(threads);
	for (size_t i = 0; i < threads; ++i) {
		_workers.emplace_back([this] {
			for (;;) {
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(this->_queueMutex);
					this->_condition.wait(lock, [this] {
						return this->_stop || !this->_tasks.empty();
					});
					if (this->_stop && this->_tasks.empty()) {
						return;
					}
					task = std::move(this->_tasks.front());
					this->_tasks.pop();
				}

				task();
			}
		});
	}
}

// add new work item to the pool
template<class F, class ... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type> {
	using return_type = typename std::result_of<F(Args...)>::type;

	auto task = std::make_shared<std::packaged_task<return_type()> >(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(_queueMutex);
		_tasks.emplace([task]() {(*task)();});
	}
	_condition.notify_one();
	return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool() {
	_stop = true;
	_condition.notify_all();
	for (std::thread &worker : _workers)
		worker.join();
}

}

// #include "common/ExecutionTime.h"
/*
The MIT License (MIT)

Copyright (c) 2015 Hideaki Suzuki

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/



#include <chrono>
#include <iomanip>
#include <string>

#ifdef NO_TIME_ELAPSED_MEASUREMENT
#  define	TIME_ELAPSED_N(n,...)					time_elapsed_impl_noop<n>([&]{__VA_ARGS__;})
#  define	TIME_ELAPSED_MARKER_N(n,marker,...)		time_elapsed_impl_noop<n>([&]{__VA_ARGS__;})
#else
#  define	TIME_ELAPSED_N(n,...)					time_elapsed_impl1<n>([&]{__VA_ARGS__;},__FILE__,__LINE__)
#  define	TIME_ELAPSED_MARKER_N(n,marker,...)		time_elapsed_impl_core<n>([&]{__VA_ARGS__;},marker)
#endif

#define	TIME_ELAPSED(...)							TIME_ELAPSED_N(1,__VA_ARGS__)
#define	TIME_ELAPSED_MARKER(marker,...)				TIME_ELAPSED_MARKER_N(1,marker,__VA_ARGS__)

template<const int count, typename BlockBody>
inline void time_elapsed_impl_core(BlockBody body, const std::string& header) {
	auto t0 = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < count; i++)
		body();

	auto t1 = std::chrono::high_resolution_clock::now();
	const float millis = (float) std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1000.0f;
	ai_log("ElapsedTime: %s, cnt: %i - %0.3fmsec", header.c_str(), count, millis);
}

template<const int count, typename BlockBody>
inline void time_elapsed_impl_noop(BlockBody body) {
	for (int i = 0; i < count; i++)
		body();
}

template<int count, typename BlockBody>
inline void time_elapsed_impl1(BlockBody body, const char * const path, const int lno) {
	std::string basename { path + std::string(path).find_last_of("\\/") + 1 };
	time_elapsed_impl_core<count>(body, basename + " at L." + std::to_string(lno));
}


// #include "AI.h"
/**
 * @file
 */


#include <unordered_map>
#include <memory>

// #include "group/GroupId.h"
/**
 * @file
 */


namespace ai {

typedef int GroupId;

}

// #include "aggro/AggroMgr.h"
/**
 * @file
 *
 * @defgroup Aggro
 * @{
 * The aggro manager can be used to create interactions between AI controlled entities.
 */


#include <vector>
#include <memory>
// #include "ICharacter.h"
/**
 * @file
 */


// #include "common/NonCopyable.h"
/**
 * @file
 */


namespace ai {

class NonCopyable {
public:
	NonCopyable() {
	}
private:
	NonCopyable (const NonCopyable&);
	NonCopyable& operator= (const NonCopyable&);
};

}

// #include "common/Math.h"

// #include "common/Types.h"

#include <atomic>
#include <memory>

namespace ai {

/**
 * @brief Defines some standard names for @c ICharacter attributes. None of these must be used. But if you
 * use them, the remote debugger can make use of known values to render more information into the view.
 */
namespace attributes {
/**
 * @brief Attribute for the name of an entity
 */
const char* const NAME = "Name";
const char* const GROUP = "Group";
const char* const ID = "Id";
const char* const POSITION = "Position";
const char* const SPEED = "Speed";
const char* const ORIENTATION = "Orientation";
}

/**
 * @brief Class that should be extended by the @ai{AI} controlled entity class.
 *
 * It uses a @ai{CharacterId} to identify the character in the game. The @ai{AI} class
 * has a reference to its controlled @c ICharacter instance.
 *
 * @note Update the values of the @c ICharacter class only in in the @ai{ICharacter::update()}
 * method or from within the @ai{Zone} callbacks. Otherwise you will run into race conditions
 * if you run with multiple threads.
 *
 * You often need access to your world your character is living in. You need access to this
 * data to resolve the @ai{CharacterId}'s in the @ai{IFilter} implementations, to interact with
 * other entities that are not SimpleAI controlled and so on. You can use the provided
 * @ai{character_cast} function in your @ai{TreeNode}, @ai{IFilter} or @ai{ICondition} implementations.
 */
class ICharacter : public NonCopyable, public std::enable_shared_from_this<ICharacter> {
protected:
	const CharacterId _id;
	glm::vec3 _position;
	std::atomic<float> _orientation;
	// m/s
	std::atomic<float> _speed;
	CharacterAttributes _attributes;

public:
	explicit ICharacter(CharacterId id) :
			_id(id), _orientation(0.0f), _speed(0.0f) {
	}

	virtual ~ICharacter() {
	}

	bool operator ==(const ICharacter& character) const;
	bool operator !=(const ICharacter& character) const;

	CharacterId getId() const;
	/**
	 * @note This is virtual because you might want to override this in your implementation to
	 * make sure that the new position is also forwarded to your AI controlled entity.
	 */
	virtual void setPosition(const glm::vec3& position);

	const glm::vec3& getPosition() const;
	/**
	 * @note This is virtual because you might want to override this in your implementation to
	 * make sure that the new orientation is also forwarded to your AI controlled entity.
	 *
	 * @see getOrientation()
	 */
	virtual void setOrientation(float orientation);
	/**
	 * @return the radians around the y (up) axis
	 *
	 * @see setOrientation()
	 */
	float getOrientation() const;
	/**
	 * @brief Sets the speed for the character in m/s
	 *
	 * @see getSpeed()
	 */
	virtual void setSpeed(float speed);
	/**
	 * @return The speed for the character in m/s
	 *
	 * @see setSpeed()
	 */
	float getSpeed() const;
	/**
	 * @brief Set an attribute that can be used for debugging
	 * @see AI::isDebuggingActive()
	 */
	virtual void setAttribute(const std::string& key, const std::string& value);
	/**
	 * @brief Get the debugger attributes.
	 */
	const CharacterAttributes& getAttributes() const;
	/**
	 * @brief override this method to let your own @c ICharacter implementation
	 * tick with the @c Zone::update
	 *
	 * @param[in] dt the time delta in millis since the last update was executed
	 * @param[in] debuggingActive @c true if the debugging for this entity is activated. This
	 * can be used to determine whether it's useful to do setAttribute() calls.
	 */
	virtual void update(int64_t dt, bool debuggingActive) {
		(void)dt;
		(void)debuggingActive;
	}

	/**
	 * If the object is currently maintained by a shared_ptr, you can get a shared_ptr from a raw pointer
	 * instance that shares the state with the already existing shared_ptrs around.
	 */
	inline std::shared_ptr<ICharacter> ptr() {
		return shared_from_this();
	}
};

inline void ICharacter::setPosition(const glm::vec3& position) {
	ai_assert(!isInfinite(position), "invalid position");
	_position = position;
}

inline void ICharacter::setOrientation (float orientation) {
	_orientation = orientation;
}

inline float ICharacter::getOrientation () const {
	return _orientation;
}

inline void ICharacter::setAttribute(const std::string& key, const std::string& value) {
	_attributes[key] = value;
}

inline const CharacterAttributes& ICharacter::getAttributes() const {
	return _attributes;
}

inline bool ICharacter::operator ==(const ICharacter& character) const {
	return character._id == _id;
}

inline bool ICharacter::operator !=(const ICharacter& character) const {
	return character._id != _id;
}

inline CharacterId ICharacter::getId() const {
	return _id;
}

inline const glm::vec3& ICharacter::getPosition() const {
	return _position;
}

inline void ICharacter::setSpeed(float speed) {
	_speed = speed;
}

inline float ICharacter::getSpeed() const {
	return _speed;
}

typedef std::shared_ptr<ICharacter> ICharacterPtr;

template <typename CharacterType>
inline const CharacterType& character_cast(const ICharacter& character) {
	return ai_assert_cast<const CharacterType&>(character);
}

template <typename CharacterType>
inline CharacterType& character_cast(ICharacter& character) {
	return ai_assert_cast<CharacterType&>(character);
}

template <typename CharacterType>
inline CharacterType& character_cast(const ICharacterPtr& character) {
	return *ai_assert_cast<CharacterType*>(character.get());
}

}

#include <algorithm>
// #include "aggro/Entry.h"
/**
 * @file
 */


// #include "common/Types.h"


namespace ai {

enum ReductionType {
	DISABLED, RATIO, VALUE
};

/**
 * @brief One entry for the @c AggroMgr
 */
class Entry {
protected:
	float _aggro;
	float _minAggro;
	float _reduceRatioSecond;
	float _reduceValueSecond;
	ReductionType _reduceType;
	CharacterId _id;

	void reduceByRatio(float ratio);
	void reduceByValue(float value);

public:
	Entry(const CharacterId& id, float aggro = 0.0f) :
			_aggro(aggro), _minAggro(0.0f), _reduceRatioSecond(0.0f), _reduceValueSecond(0.0f), _reduceType(DISABLED), _id(id) {
	}

	Entry(const Entry &other) :
			_aggro(other._aggro), _minAggro(other._minAggro), _reduceRatioSecond(other._reduceRatioSecond), _reduceValueSecond(other._reduceValueSecond), _reduceType(
					other._reduceType), _id(other._id) {
	}

	Entry(Entry &&other) :
			_aggro(other._aggro), _minAggro(other._minAggro), _reduceRatioSecond(other._reduceRatioSecond), _reduceValueSecond(other._reduceValueSecond), _reduceType(
					other._reduceType), _id(other._id) {
	}

	float getAggro() const;
	void addAggro(float aggro);
	void setReduceByRatio(float reductionRatioPerSecond, float minimumAggro);
	void setReduceByValue(float reductionValuePerSecond);
	/**
	 * @return @c true if any reduction was done
	 */
	bool reduceByTime(int64_t millis);
	void resetAggro();

	const CharacterId& getCharacterId() const;
	bool operator <(Entry& other) const;
	Entry& operator=(const Entry& other);
};

typedef Entry* EntryPtr;

inline void Entry::addAggro(float aggro) {
	_aggro += aggro;
}

inline void Entry::setReduceByRatio(float reduceRatioSecond, float minAggro) {
	_reduceType = RATIO;
	_reduceRatioSecond = reduceRatioSecond;
	_minAggro = minAggro;
}

inline void Entry::setReduceByValue(float reduceValueSecond) {
	_reduceType = VALUE;
	_reduceValueSecond = reduceValueSecond;
}

inline bool Entry::reduceByTime(int64_t millis) {
	switch (_reduceType) {
	case RATIO: {
		const float f = static_cast<float>(millis) / 1000.0f;
		reduceByRatio(f * _reduceRatioSecond);
		return true;
	}
	case VALUE: {
		const float f = static_cast<float>(millis) / 1000.0f;
		reduceByValue(f * _reduceValueSecond);
		return true;
	}
	case DISABLED:
		break;
	}
	return false;
}

inline void Entry::reduceByRatio(float ratio) {
	_aggro *= (1.0f - ratio);
	if (_aggro < _minAggro) {
		_aggro = 0.0f;
	}
}

inline void Entry::reduceByValue(float value) {
	_aggro -= value;

	if (_aggro < 0.000001f) {
		_aggro = 0.0f;
	}
}

inline float Entry::getAggro() const {
	return _aggro;
}

inline void Entry::resetAggro() {
	_aggro = 0.0f;
}

inline bool Entry::operator <(Entry& other) const {
	return _aggro < other._aggro;
}

inline Entry& Entry::operator=(const Entry& other) {
	_aggro = other._aggro;
	_minAggro = other._minAggro;
	_reduceRatioSecond = other._reduceRatioSecond;
	_reduceValueSecond = other._reduceValueSecond;
	_reduceType = other._reduceType;
	_id = other._id;
	return *this;
}

inline const CharacterId& Entry::getCharacterId() const {
	return _id;
}

}


namespace ai {

/**
 * @brief Manages the aggro values for one @c AI instance. There are several ways to degrade the aggro values.
 */
class AggroMgr {
public:
	typedef std::vector<Entry> Entries;
	typedef Entries::iterator EntriesIter;
protected:
	mutable Entries _entries;

	mutable bool _dirty;

	float _minAggro = 0.0f;
	float _reduceRatioSecond = 0.0f;
	float _reduceValueSecond = 0.0f;
	ReductionType _reduceType = DISABLED;

	class CharacterIdPredicate {
	private:
		const CharacterId& _id;
	public:
		explicit CharacterIdPredicate(const CharacterId& id) :
			_id(id) {
		}

		inline bool operator()(const Entry &n1) {
			return n1.getCharacterId() == _id;
		}
	};

	static bool EntrySorter(const Entry& a, const Entry& b) {
		if (a.getAggro() > b.getAggro()) {
			return false;
		}
		if (::fabs(a.getAggro() - b.getAggro()) < 0.0000001f) {
			return a.getCharacterId() < b.getCharacterId();
		}
		return true;
	}

	/**
	 * @brief Remove the entries from the list that have no aggro left.
	 * This list is ordered, so we will only remove the first X elements.
	 */
	void cleanupList() {
		EntriesIter::difference_type remove = 0;
		for (EntriesIter i = _entries.begin(); i != _entries.end(); ++i) {
			const float aggroValue = i->getAggro();
			if (aggroValue > 0.0f) {
				break;
			}

			++remove;
		}

		if (remove == 0) {
			return;
		}

		const int size = static_cast<int>(_entries.size());
		if (size == remove) {
			_entries.clear();
			return;
		}

		EntriesIter i = _entries.begin();
		std::advance(i, remove);
		_entries.erase(_entries.begin(), i);
	}

	inline void sort() const {
		if (!_dirty) {
			return;
		}
		std::sort(_entries.begin(), _entries.end(), EntrySorter);
		_dirty = false;
	}
public:
	explicit AggroMgr(std::size_t expectedEntrySize = 0u) :
		_dirty(false) {
		if (expectedEntrySize > 0) {
			_entries.reserve(expectedEntrySize);
		}
	}

	virtual ~AggroMgr() {
	}

	inline void setReduceByRatio(float reduceRatioSecond, float minAggro) {
		_reduceType = RATIO;
		_reduceValueSecond = 0.0f;
		_reduceRatioSecond = reduceRatioSecond;
		_minAggro = minAggro;
	}

	inline void setReduceByValue(float reduceValueSecond) {
		_reduceType = VALUE;
		_reduceValueSecond = reduceValueSecond;
		_reduceRatioSecond = 0.0f;
		_minAggro = 0.0f;
	}

	inline void resetReduceValue() {
		_reduceType = DISABLED;
		_reduceValueSecond = 0.0f;
		_reduceRatioSecond = 0.0f;
		_minAggro = 0.0f;
	}

	/**
	 * @brief this will update the aggro list according to the reduction type of an entry.
	 * @param[in] deltaMillis The current milliseconds to use to update the aggro value of the entries.
	 */
	void update(int64_t deltaMillis) {
		for (EntriesIter i = _entries.begin(); i != _entries.end(); ++i) {
			_dirty |= i->reduceByTime(deltaMillis);
		}

		if (_dirty) {
			sort();
			cleanupList();
		}
	}

	/**
	 * @brief will increase the aggro
	 * @param[in] id The entity id to increase the aggro against
	 * @param[in] amount The amount to increase the aggro for
	 * @return The aggro @c Entry that was added or updated. Useful for changing the reduce type or amount.
	 */
	EntryPtr addAggro(CharacterId id, float amount) {
		const CharacterIdPredicate p(id);
		EntriesIter i = std::find_if(_entries.begin(), _entries.end(), p);
		if (i == _entries.end()) {
			Entry newEntry(id, amount);
			switch (_reduceType) {
			case RATIO:
				newEntry.setReduceByRatio(_reduceRatioSecond, _minAggro);
				break;
			case VALUE:
				newEntry.setReduceByValue(_reduceValueSecond);
				break;
			default:
				break;
			}
			_entries.push_back(newEntry);
			_dirty = true;
			return &_entries.back();
		}

		i->addAggro(amount);
		_dirty = true;
		return &*i;
	}

	/**
	 * @return All the aggro entries
	 */
	const Entries& getEntries() const {
		return _entries;
	}

	/**
	 * @brief Get the entry with the highest aggro value.
	 *
	 * @note Might execute a sort on the list if its dirty
	 */
	EntryPtr getHighestEntry() const {
		if (_entries.empty()) {
			return nullptr;
		}

		sort();

		return &_entries.back();
	}
};

}

/**
 * @}
 */

// #include "ICharacter.h"

// #include "tree/TreeNode.h"
/**
 * @file
 */


// #include "common/Types.h"

// #include "common/MemoryAllocator.h"

// #include "conditions/ICondition.h"
/**
 * @file
 * @brief Condition related stuff
 * @defgroup Condition
 * @{
 * @sa ai{ICondition}
 */



#include <string>
#include <vector>
#include <sstream>
#include <memory>

// #include "common/MemoryAllocator.h"

// #include "common/Thread.h"


// #include "AIFactories.h"
/**
 * @file
 */


// #include "IAIFactory.h"
/**
 * @file
 */


#include <memory>
#include <string>
#include <vector>
#include <list>

namespace ai {

class TreeNode;
typedef std::shared_ptr<TreeNode> TreeNodePtr;
typedef std::vector<TreeNodePtr> TreeNodes;

class IFilter;
typedef std::shared_ptr<IFilter> FilterPtr;
typedef std::list<FilterPtr> Filters;

namespace movement {
class ISteering;
}
typedef std::shared_ptr<movement::ISteering> SteeringPtr;
namespace movement {
typedef std::vector<SteeringPtr> Steerings;
}

class ICondition;
typedef std::shared_ptr<ICondition> ConditionPtr;
typedef std::vector<ConditionPtr> Conditions;

struct TreeNodeFactoryContext;
struct ConditionFactoryContext;
struct FilterFactoryContext;
struct SteerNodeFactoryContext;
struct SteeringFactoryContext;

class IAIFactory {
public:
	virtual ~IAIFactory() {
	}

	/**
	 * @brief Allocates a new @c TreeNode for the given @c type. The @c type must be registered in the @c AIRegistry for this to work.
	 */
	virtual TreeNodePtr createNode(const std::string& type, const TreeNodeFactoryContext& ctx) const = 0;
	/**
	 * @brief Allocates a new @c TreeNode for the given @c type. The @c type must be registered in the @c AIRegistry for this to work.
	 */
	virtual TreeNodePtr createSteerNode(const std::string& type, const SteerNodeFactoryContext& ctx) const = 0;
	/**
	 * @brief Allocates a new @c IFilter for the given @c type. The @c type must be registered in the @c AIRegistry for this to work.
	 */
	virtual FilterPtr createFilter(const std::string& type, const FilterFactoryContext& ctx) const = 0;
	/**
	 * @brief Allocates a new @c ICondition for the given @c type. The @c type must be registered in the @c AIRegistry for this to work.
	 */
	virtual ConditionPtr createCondition(const std::string& type, const ConditionFactoryContext& ctx) const = 0;
	/**
	 * @brief Creates a new @c ISteering for the given @c type. The @c type must be registered in the @c AIRegistry for this to work.
	 */
	virtual SteeringPtr createSteering(const std::string& type, const SteeringFactoryContext& ctx) const = 0;
};

}

// #include "common/IFactoryRegistry.h"
/**
 * @file
 */


// #include "common/Types.h"

// #include "common/NonCopyable.h"

#include <memory>
#include <map>

namespace ai {

template<class TYPE, class CTX>
class IFactory {
public:
	virtual ~IFactory() {}
	virtual std::shared_ptr<TYPE> create (const CTX* ctx) const = 0;
};

template<class KEY, class TYPE, class CTX>
class IFactoryRegistry: public NonCopyable {
protected:
	typedef std::map<const KEY, const IFactory<TYPE, CTX>*> FactoryMap;
	typedef typename FactoryMap::const_iterator FactoryMapConstIter;
	typedef typename FactoryMap::iterator FactoryMapIter;
	FactoryMap _factories;
public:
	bool registerFactory (const KEY& type, const IFactory<TYPE, CTX>& factory)
	{
		FactoryMapConstIter i = _factories.find(type);
		if (i != _factories.end()) {
			return false;
		}

		_factories[type] = &factory;
		return true;
	}

	bool unregisterFactory (const KEY& type)
	{
		FactoryMapIter i = _factories.find(type);
		if (i == _factories.end()) {
			return false;
		}

		_factories.erase(i);
		return true;
	}

	std::shared_ptr<TYPE> create (const KEY& type, const CTX* ctx = nullptr) const
	{
		FactoryMapConstIter i = _factories.find(type);
		if (i == _factories.end()) {
			return std::shared_ptr<TYPE>();
		}

		const IFactory<TYPE, CTX>* factory = i->second;

#if AI_EXCEPTIONS
		try {
#endif
			return factory->create(ctx);
#if AI_EXCEPTIONS
		} catch (...) {
			ai_log_error("Exception while trying to create a factory");
		}
		return std::shared_ptr<TYPE>();
#endif
	}
};

}

#include <vector>

namespace ai {

/**
 * @brief Context for ITreeNodeFactory
 */
struct TreeNodeFactoryContext {
	std::string name;
	std::string parameters;
	ConditionPtr condition;
	TreeNodeFactoryContext(const std::string& _name, const std::string& _parameters, const ConditionPtr& _condition) :
			name(_name), parameters(_parameters), condition(_condition) {
	}
};

struct SteerNodeFactoryContext {
	std::string name;
	std::string parameters;
	ConditionPtr condition;
	movement::Steerings steerings;
	SteerNodeFactoryContext(const std::string& _name, const std::string& _parameters, const ConditionPtr& _condition, const movement::Steerings& _steerings) :
			name(_name), parameters(_parameters), condition(_condition), steerings(_steerings) {
	}
};

struct FilterFactoryContext {
	// Parameters for the filter - can get hand over to the ctor in your factory implementation.
	std::string parameters;
	Filters filters;
	explicit FilterFactoryContext(const std::string& _parameters) :
		parameters(_parameters) {
	}
};

struct SteeringFactoryContext {
	// Parameters for the steering class - can get hand over to the ctor in your factory implementation.
	std::string parameters;
	explicit SteeringFactoryContext(const std::string& _parameters) :
		parameters(_parameters) {
	}
};

struct ConditionFactoryContext {
	// Parameters for the condition - can get hand over to the ctor in your factory implementation.
	std::string parameters;
	// Some conditions have child conditions
	Conditions conditions;
	// The filter condition also has filters
	Filters filters;
	bool filter;
	explicit ConditionFactoryContext(const std::string& _parameters) :
		parameters(_parameters), filter(false) {
	}
};

/**
 * @brief This factory will create tree nodes. It uses the @c TreeNodeFactoryContext to
 * collect all the needed data for this action.
 */
class ITreeNodeFactory: public IFactory<TreeNode, TreeNodeFactoryContext> {
public:
	virtual ~ITreeNodeFactory() {
	}
	virtual TreeNodePtr create(const TreeNodeFactoryContext *ctx) const override = 0;
};

class ISteeringFactory: public IFactory<movement::ISteering, SteeringFactoryContext> {
public:
	virtual ~ISteeringFactory() {
	}
	virtual SteeringPtr create(const SteeringFactoryContext* ctx) const override = 0;
};

class ISteerNodeFactory: public IFactory<TreeNode, SteerNodeFactoryContext> {
public:
	virtual ~ISteerNodeFactory() {
	}
	virtual TreeNodePtr create(const SteerNodeFactoryContext *ctx) const override = 0;
};

class IFilterFactory: public IFactory<IFilter, FilterFactoryContext> {
public:
	virtual ~IFilterFactory() {
	}
	virtual FilterPtr create(const FilterFactoryContext *ctx) const override = 0;
};

class IConditionFactory: public IFactory<ICondition, ConditionFactoryContext> {
public:
	virtual ~IConditionFactory() {
	}
	virtual ConditionPtr create(const ConditionFactoryContext *ctx) const override = 0;
};

}


namespace ai {

class AI;
typedef std::shared_ptr<AI> AIPtr;

/**
 * @brief Macro to simplify the condition creation. Just give the class name of the condition as parameter.
 */
#define CONDITION_CLASS(ConditionName) \
	explicit ConditionName(const std::string& parameters = "") : \
		::ai::ICondition(#ConditionName, parameters) { \
	} \
public: \
	virtual ~ConditionName() { \
	}

/**
 * @brief A condition factory macro to ease and unify the registration at AIRegistry.
 * You still have to implement the Factory::create method.
 */
#define CONDITION_FACTORY_NO_IMPL(ConditionName) \
public: \
	class Factory: public ::ai::IConditionFactory { \
	public: \
		::ai::ConditionPtr create (const ::ai::ConditionFactoryContext *ctx) const override; \
	}; \
	static const Factory& getFactory() { \
		static Factory FACTORY; \
		return FACTORY; \
	}

/**
 * @brief A condition factory macro to ease and unify the registration at @ai{AIRegistry}.
 */
#define CONDITION_FACTORY(ConditionName) \
public: \
	class Factory: public ::ai::IConditionFactory { \
	public: \
		::ai::ConditionPtr create (const ::ai::ConditionFactoryContext *ctx) const override { \
			return std::make_shared<ConditionName>(ctx->parameters); \
		} \
	}; \
	static const Factory& getFactory() { \
		static Factory FACTORY; \
		return FACTORY; \
	}

/**
 * @brief A condition factory singleton macro to ease and unify the registration at @ai{AIRegistry}.
 * Nothing from the given context is taken, if you need this, use the instance based factory,
 * not the singleton based.
 */
#define CONDITION_FACTORY_SINGLETON \
public: \
	class Factory: public ::ai::IConditionFactory { \
		::ai::ConditionPtr create (const ::ai::ConditionFactoryContext */*ctx*/) const { \
			return get(); \
		} \
	}; \
	static const Factory& getFactory() { \
		static Factory FACTORY; \
		return FACTORY; \
	}

/**
 * @brief Macro to create a singleton conditions for very easy conditions without a state.
 */
#define CONDITION_CLASS_SINGLETON(ConditionName) \
private: \
	CONDITION_CLASS(ConditionName) \
public: \
	static ConditionPtr& get() { \
		AI_THREAD_LOCAL ConditionName* c = nullptr; \
		if (c == nullptr) { c = new ConditionName; } \
		AI_THREAD_LOCAL ConditionPtr _instance(c); \
		return _instance; \
	} \
	CONDITION_FACTORY_SINGLETON

#define CONDITION_PRINT_SUBCONDITIONS_GETCONDITIONNAMEWITHVALUE \
	void getConditionNameWithValue(std::stringstream& s, const ::ai::AIPtr& entity) override { \
		bool first = true; \
		s << "("; \
		for (::ai::ConditionsConstIter i = _conditions.begin(); i != _conditions.end(); ++i) { \
			if (!first) { \
				s << ","; \
			} \
			s << (*i)->getNameWithConditions(entity); \
			first = false; \
		} \
		s << ")"; \
	}

class ICondition;
typedef std::shared_ptr<ICondition> ConditionPtr;
typedef std::vector<ConditionPtr> Conditions;
typedef Conditions::iterator ConditionsIter;
typedef Conditions::const_iterator ConditionsConstIter;

/**
 * @brief A condition can be placed on a @ai{TreeNode} to decide which node is going to get executed. In general they are stateless.
 * If they are not, it should explicitly get noted.
 */
class ICondition : public MemObject {
protected:
	static int getNextId() {
		static int _nextId = 0;
		const int id = _nextId++;
		return id;
	}

	/**
	 * @brief Every node has an id to identify it. It's unique per type.
	 */
	int _id;
	const std::string _name;
	const std::string _parameters;

	/**
	 * @brief Override this method to get a more detailed result in @c getNameWithConditions()
	 *
	 * @param[out] s The string stream to write your details to
	 * @param[in,out] entity The entity that is used to evaluate a condition
	 * @sa getNameWithConditions()
	 */
	virtual void getConditionNameWithValue(std::stringstream& s, const AIPtr& entity) {
		(void)entity;
		s << "{" << _parameters << "}";
	}
public:
	ICondition(const std::string& name, const std::string& parameters) :
			_id(getNextId()), _name(name), _parameters(parameters) {
	}

	virtual ~ICondition() {
	}

	/**
	 * @brief Checks whether the condition evaluates to @c true for the given @c entity.
	 * @param[in,out] entity The entity that is used to evaluate the condition
	 * @return @c true if the condition is fulfilled, @c false otherwise.
	 */
	virtual bool evaluate(const AIPtr& entity) = 0;

	/**
	 * @brief Returns the short name of the condition - without any related conditions or results.
	 */
	const std::string& getName() const;

	/**
	 * @brief Returns the raw parameters of the condition
	 */
	const std::string& getParameters() const;

	/**
	 * @brief Returns the full condition string with all related conditions and results of the evaluation method
	 * @param[in,out] entity The entity that is used to evaluate the condition
	 * @sa getConditionNameWithValue()
	 */
	inline std::string getNameWithConditions(const AIPtr& entity) {
		std::stringstream s;
		s << getName();
		getConditionNameWithValue(s, entity);
		s << "[";
		s << (evaluate(entity) ? "1" : "0");
		s << "]";
		return s.str();
	}
};

inline const std::string& ICondition::getName() const {
	return _name;
}

inline const std::string& ICondition::getParameters() const {
	return _parameters;
}

}

/**
 * @}
 */

// #include "conditions/True.h"
/**
 * @file
 * @ingroup Condition
 */


// #include "conditions/ICondition.h"


namespace ai {

/**
 * @brief This condition just always evaluates to @c true
 */
class True: public ICondition {
public:
	CONDITION_CLASS_SINGLETON(True)

	bool evaluate(const AIPtr& entity) override;
};

inline bool True::evaluate(const AIPtr& /* entity */) {
	return true;
}

}

#include <vector>
#include <string>
#include <memory>
#include <algorithm>

namespace ai {

class TreeNode;
typedef std::shared_ptr<TreeNode> TreeNodePtr;
typedef std::vector<TreeNodePtr> TreeNodes;

/**
 * @brief Execution states of a TreeNode::execute() call
 */
enum TreeNodeStatus {
	UNKNOWN,
	/**
	 * Not every condition is met in order to run this node
	 * In general this means that the attached condition has to evaluate to @c true
	 */
	CANNOTEXECUTE,
	/**
	 * This behavior is still running and thus can block others
	 */
	RUNNING,
	/**
	 * The behavior ran and terminated without any problems.
	 */
	FINISHED,
	/**
	 * Controlled failure
	 */
	FAILED,
	/**
	 * Unexpected failure while executing the the node's action
	 */
	EXCEPTION,

	MAX_TREENODESTATUS
};

/**
 * @brief A node factory macro to ease and unify the registration at AIRegistry.
 */
#define NODE_FACTORY(NodeName) \
	class Factory: public ::ai::ITreeNodeFactory { \
	public: \
		::ai::TreeNodePtr create (const ::ai::TreeNodeFactoryContext *ctx) const override { \
			return std::make_shared<NodeName>(ctx->name, ctx->parameters, ctx->condition); \
		} \
	}; \
	static const Factory& getFactory() { \
		static Factory FACTORY; \
		return FACTORY; \
	}

/**
 * @brief A node class macro that also defines a factory.
 */
#define NODE_CLASS(NodeName) \
	NodeName(const std::string& name, const std::string& parameters, const ::ai::ConditionPtr& condition) : \
		::ai::TreeNode(name, parameters, condition) { \
		_type = AI_STRINGIFY(NodeName); \
	} \
	virtual ~NodeName() { \
	} \
	\
	NODE_FACTORY(NodeName)

/**
 * @brief The base class for all behaviour tree actions.
 *
 * @c TreeNode::execute is triggered with each @c AI::update.
 * Also the attached @c ICondition is evaluated here. States are stored on the
 * connected @c AI instance. Don't store states on tree nodes, because they can
 * be reused for multiple @c AI instances. Always use the @c AI or @c ICharacter
 * to store your state!
 */
class TreeNode : public MemObject {
protected:
	static int getNextId() {
		static int _nextId;
		const int nextId = _nextId++;
		return nextId;
	}
	/**
	 * @brief Every node has an id to identify it. It's unique per type.
	 */
	int _id;
	TreeNodes _children;
	std::string _name;
	std::string _type;
	std::string _parameters;
	ConditionPtr _condition;

	TreeNodeStatus state(const AIPtr& entity, TreeNodeStatus treeNodeState);
	int getSelectorState(const AIPtr& entity) const;
	void setSelectorState(const AIPtr& entity, int selected);
	int getLimitState(const AIPtr& entity) const;
	void setLimitState(const AIPtr& entity, int amount);
	void setLastExecMillis(const AIPtr& entity);

	TreeNodePtr getParent_r(const TreeNodePtr& parent, int id) const;

public:
	/**
	 * @param name The internal name of the node
	 * @param parameters Each node can be configured with several parameters that are hand in as a string. It's
	 * the responsibility of the node to parse the values in its constructor
	 * @param condition The connected ICondition for this node
	 */
	TreeNode(const std::string& name, const std::string& parameters, const ConditionPtr& condition) :
			_id(getNextId()), _name(name), _parameters(parameters), _condition(condition) {
	}

	virtual ~TreeNode() {}
	/**
	 * @brief Return the unique id for this node.
	 * @return unique id
	 */
	int getId() const;

	/**
	 * @brief Each node can have a user defines name that can be retrieved with this method.
	 */
	const std::string& getName() const;

	/**
	 * @brief Return the raw parameters for this node
	 */
	const std::string& getParameters() const;

	/**
	 * @brief Updates the custom name of this @c TreeNode
	 *
	 * @param[in] name The name to set - empty strings are ignored here
	 */
	void setName(const std::string& name);
	/**
	 * @brief The node type - this usually matches the class name of the @c TreeNode
	 */
	const std::string& getType() const;
	const ConditionPtr& getCondition() const;
	void setCondition(const ConditionPtr& condition);
	const TreeNodes& getChildren() const;
	TreeNodes& getChildren();

	/**
	 * @brief Get the state of all child nodes for the given entity
	 * @param[in] entity The entity to get the child node states for
	 */
	virtual void getRunningChildren(const AIPtr& entity, std::vector<bool>& active) const;
	/**
	 * @brief Returns the time in milliseconds when this node was last run. This is only updated if @c #execute() was called
	 */
	int64_t getLastExecMillis(const AIPtr& ai) const;
	TreeNodeStatus getLastStatus(const AIPtr& ai) const;

	/**
	 * @param entity The entity to execute the TreeNode for
	 * @param deltaMillis The delta since the last execution
	 * @return TreeNodeStatus
	 */
	virtual TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis);

	/**
	 * @brief Reset the states in the node and also in the entity
	 */
	virtual void resetState(const AIPtr& entity);

	virtual bool addChild(const TreeNodePtr& child);
	TreeNodePtr getChild(int id) const;
	/**
	 * @brief Replace the given child node with a new one (or removes it)
	 *
	 * @param[in] id The child node id
	 * @param[in] newNode If this is an empty TreeNodePtr the child will be removed
	 * @return @c true if the removal/replace was successful, @c false otherwise
	 */
	bool replaceChild(int id, const TreeNodePtr& newNode);
	/**
	 * @brief Get the parent node for a given TreeNode id - This should only be called on the root node of the behaviour
	 *
	 * @param[in] self The pointer to the root node that is returned if one of the direct children need their parent
	 * @param[in] id The child node id
	 *
	 * @return An empty TreeNodePtr if not found, or the parent is the root node of the behaviour tree
	 */
	TreeNodePtr getParent(const TreeNodePtr& self, int id) const;
};

}

// #include "tree/loaders/ITreeLoader.h"
/**
 * @file
 */


// #include "common/Thread.h"

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <cstdarg>
#include <cstdio>

namespace ai {

class IAIFactory;
class TreeNode;
typedef std::shared_ptr<TreeNode> TreeNodePtr;

/**
 * @brief This class must be extended to load behaviour trees. The contract here is that the parsing only happens
 * once (of course) and then @c ITreeLoader::getTrees and @c ITreeLoader::load will just access the cached data.
 */
class ITreeLoader {
protected:
	const IAIFactory& _aiFactory;
	typedef std::map<std::string, TreeNodePtr> TreeMap;
	TreeMap _treeMap;
	ReadWriteLock _lock = {"treeloader"};

	inline void resetError() {
		ScopedWriteLock scopedLock(_lock);
		_error = "";
	}
private:
	std::string _error;		/**< make sure to set this member if your own implementation ran into an error. @sa ITreeLoader::getError */
public:
	explicit ITreeLoader(const IAIFactory& aiFactory) :
			_aiFactory(aiFactory) {
	}

	virtual ~ITreeLoader() {
		_error = "";
		_treeMap.clear();
	}

	void shutdown() {
		ScopedWriteLock scopedLock(_lock);
		_error = "";
		_treeMap.clear();
	}

	inline const IAIFactory& getAIFactory() const {
		return _aiFactory;
	}

	/**
	 * @brief Fill the given vector with the loaded behaviour tree names
	 */
	void getTrees(std::vector<std::string>& trees) const {
		ScopedReadLock scopedLock(_lock);
		trees.reserve(_treeMap.size());
		for (TreeMap::const_iterator it = _treeMap.begin(); it != _treeMap.end(); ++it) {
			trees.push_back(it->first);
		}
	}

	/**
	 * @brief Register a new @c TreeNode as behaviour tree with the specified @c name
	 *
	 * @param name The name to register the given root node under
	 * @param root The @c TreeNode that will act as behaviour tree root node
	 *
	 * @return @c true if the registration process went fine, @c false otherwise (there is already
	 * a behaviour tree registered with the same name or the given root node is invalid.
	 */
	bool addTree(const std::string& name, const TreeNodePtr& root) {
		if (!root) {
			return false;
		}
		{
			ScopedReadLock scopedLock(_lock);
			TreeMap::const_iterator i = _treeMap.find(name);
			if (i != _treeMap.end()) {
				return false;
			}
		}
		{
			ScopedWriteLock scopedLock(_lock);
			_treeMap.insert(std::make_pair(name, root));
		}
		return true;
	}

	/**
	 * @brief Loads on particular behaviour tree.
	 */
	TreeNodePtr load(const std::string &name) {
		ScopedReadLock scopedLock(_lock);
		TreeMap::const_iterator i = _treeMap.find(name);
		if (i != _treeMap.end())
			return i->second;
		return TreeNodePtr();
	}

	void setError(const char* msg, ...) __attribute__((format(printf, 2, 3)));

	/**
	 * @brief Gives access to the last error state of the @c ITreeLoader
	 */
	inline std::string getError() const {
		return _error;
	}
};

inline void ITreeLoader::setError(const char* msg, ...) {
	va_list args;
	va_start(args, msg);
	char buf[1024];
	std::vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	if (buf[0] != '\0') {
		ai_log_debug("%s", buf);
	}
	ScopedWriteLock scopedLock(_lock);
	_error = buf;
}

}

// #include "common/Thread.h"

// #include "common/Types.h"

// #include "common/NonCopyable.h"

// #include "common/Math.h"


namespace ai {

class ICharacter;
typedef std::shared_ptr<ICharacter> ICharacterPtr;
class Zone;

typedef std::vector<CharacterId> FilteredEntities;

#ifndef AI_NOTHING_SELECTED
#define AI_NOTHING_SELECTED (-1)
#endif

/**
 * @brief This is the type the library works with. It interacts with it's real world entity by
 * the @ai{ICharacter} interface.
 *
 * Each ai entity has a @ai[AggroMgr} assigned that is updated with each tick (update()).
 *
 * A behaviour can be replaced at runtime with setBehaviour()
 *
 * You can set single @c AI instances to no longer update their state by calling setPause()
 */
class AI : public NonCopyable, public std::enable_shared_from_this<AI> {
	friend class TreeNode;
	friend class LUAAIRegistry;
	friend class IFilter;
	friend class Filter;
	friend class Server;
protected:
	/**
	 * This map is only filled if we are in debugging mode for this entity
	 */
	typedef std::unordered_map<int, TreeNodeStatus> NodeStates;
	NodeStates _lastStatus;
	/**
	 * This map is only filled if we are in debugging mode for this entity
	 */
	typedef std::unordered_map<int, uint64_t> LastExecMap;
	LastExecMap _lastExecMillis;

	/**
	 * @note The filtered entities are kept even over several ticks. The caller should decide
	 * whether he still needs an old/previous filtered selection
	 * @sa @ai{IFilter}
	 */
	mutable FilteredEntities _filteredEntities;

	void setFilteredEntities(const FilteredEntities& filteredEntities);

	void addFilteredEntity(CharacterId id);

	/**
	 * Often @ai{Selector} states must be stored to continue in the next step at a particular
	 * position in the behaviour tree. This map is doing exactly this.
	 */
	typedef std::unordered_map<int, int> SelectorStates;
	SelectorStates _selectorStates;

	/**
	 * This map stores the amount of execution for the @ai{Limit} node. The key is the node id
	 */
	typedef std::unordered_map<int, int> LimitStates;
	LimitStates _limitStates;

	TreeNodePtr _behaviour;
	AggroMgr _aggroMgr;

	ICharacterPtr _character;

	bool _pause;
	bool _debuggingActive;

	int64_t _time;

	Zone* _zone;

	std::atomic_bool _reset;
public:
	/**
	 * @param behaviour The behaviour tree node that is applied to this ai entity
	 */
	explicit AI(const TreeNodePtr& behaviour) :
			_behaviour(behaviour), _pause(false), _debuggingActive(false), _time(0L), _zone(nullptr), _reset(false) {
	}
	virtual ~AI() {
	}

	/**
	 * @brief Update the behaviour and the aggro values if the entity is not on hold.
	 * @param[in] dt The current milliseconds to update the aggro entries and
	 * time based tasks or conditions.
	 */
	virtual void update(int64_t dt, bool debuggingActive);

	/**
	 * @brief Set the new @ai{Zone} this entity is in
	 */
	void setZone(Zone* zone);
	/**
	 * Returns the @ai{Zone} this entity is in.
	 */
	Zone* getZone() const;
	/**
	 * @brief Returns @c true if the entity is already in a @ai{Zone}. This must not be managed manually,
	 * the @c Zone is doing that already.
	 */
	bool hasZone() const;

	/**
	 * @brief don't update the entity as long as it is paused
	 * @sa isPause()
	 */
	void setPause(bool pause);

	/**
	 * @brief don't update the entity as long as it is paused
	 * @sa setPause()
	 */
	bool isPause() const;

	/**
	 * @return @c true if the owning entity is currently under debugging, @c false otherwise
	 */
	bool isDebuggingActive() const;

	/**
	 * @brief Get the current behaviour for this ai
	 */
	TreeNodePtr getBehaviour() const;
	/**
	 * @brief Set a new behaviour
	 * @return the old one if there was any
	 */
	TreeNodePtr setBehaviour(const TreeNodePtr& newBehaviour);
	/**
	 * @return The real world entity reference
	 */
	ICharacterPtr getCharacter() const;
	/**
	 * You might not set a character twice to an @c AI instance.
	 */
	void setCharacter(const ICharacterPtr& character);

	template <typename CharacterType>
	inline CharacterType& getCharacterCast() const {
		return *static_cast<CharacterType*>(_character.get());
	}

	int64_t getTime() const;

	CharacterId getId() const;

	/**
	 * @return the @c AggroMgr for this @c AI instance. Each @c AI instance has its own @c AggroMgr instance.
	 */
	AggroMgr& getAggroMgr();
	/**
	 * @return the @c AggroMgr for this @c AI instance. Each @c AI instance has its own @c AggroMgr instance.
	 */
	const AggroMgr& getAggroMgr() const;

	/**
	 * @brief @c FilteredEntities is holding a list of @c CharacterIds that were selected by the @c Select condition.
	 * @sa @c IFilter interface.
	 * @sa @c Filter condition that executes assigned @c IFilter implementations.
	 * @return A reference to the internal data structure. This should only be used from within @c TreeNode implementations
	 * to access those entities that were filtered by the @c Filter condition.
	 *
	 * @note If you call this from outside of the behaviour tree tick, you will run into race conditions.
	 */
	const FilteredEntities& getFilteredEntities() const;

	/**
	 * If the object is currently maintained by a shared_ptr, you can get a shared_ptr from a raw pointer
	 * instance that shares the state with the already existing shared_ptrs around.
	 */
	inline std::shared_ptr<AI> ptr() {
		return shared_from_this();
	}
};

inline TreeNodePtr AI::getBehaviour() const {
	return _behaviour;
}

inline TreeNodePtr AI::setBehaviour(const TreeNodePtr& newBehaviour) {
	TreeNodePtr current = _behaviour;
	_behaviour = newBehaviour;
	_reset = true;
	return current;
}

inline void AI::setPause(bool pause) {
	_pause = pause;
}

inline bool AI::isPause() const {
	return _pause;
}

inline ICharacterPtr AI::getCharacter() const {
	return _character;
}

inline void AI::setCharacter(const ICharacterPtr& character) {
	ai_assert(!_character, "There is already a character set");
	_character = character;
}

inline AggroMgr& AI::getAggroMgr() {
	return _aggroMgr;
}

inline const AggroMgr& AI::getAggroMgr() const {
	return _aggroMgr;
}

inline const FilteredEntities& AI::getFilteredEntities() const {
	return _filteredEntities;
}

inline void AI::setFilteredEntities(const FilteredEntities& filteredEntities) {
	_filteredEntities = filteredEntities;
}

inline void AI::addFilteredEntity(CharacterId id) {
	_filteredEntities.push_back(id);
}

inline bool AI::isDebuggingActive() const {
	return _debuggingActive;
}

inline void AI::setZone(Zone* zone) {
	_zone = zone;
}

inline Zone* AI::getZone() const {
	return _zone;
}

inline bool AI::hasZone() const {
	return _zone != nullptr;
}

inline int64_t AI::getTime() const {
	return _time;
}

inline CharacterId AI::getId() const {
	if (!_character) {
		return AI_NOTHING_SELECTED;
	}
	return _character->getId();
}

inline void AI::update(int64_t dt, bool debuggingActive) {
	if (isPause()) {
		return;
	}

	if (_character) {
		_character->update(dt, debuggingActive);
	}

	if (_reset) {
		// safe to do it like this, because update is not called from multiple threads
		_reset = false;
		_lastStatus.clear();
		_lastExecMillis.clear();
		_filteredEntities.clear();
		_selectorStates.clear();
	}

	_debuggingActive = debuggingActive;
	_time += dt;
	_aggroMgr.update(dt);
}

typedef std::shared_ptr<AI> AIPtr;

}

// #include "AIFactories.h"

// #include "AIRegistry.h"
/**
 * @file
 */


// #include "IAIFactory.h"

// #include "AIFactories.h"

// #include "common/IFactoryRegistry.h"

// #include "tree/TreeNode.h"

// #include "conditions/ICondition.h"

// #include "tree/Fail.h"
/**
 * @file
 */


// #include "tree/TreeNode.h"


namespace ai {

/**
 * @brief A decorator node with only one child attached. The result of the attached child is only
 * taken into account if it returned @c TreeNodeStatus::RUNNING - in every other case this decorator
 * will return @c TreeNodeStatus::FAILED.
 */
class Fail: public TreeNode {
public:
	NODE_CLASS(Fail)

	TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis) override {
		if (_children.size() != 1) {
			ai_assert(false, "Fail must have exactly one child");
		}

		if (TreeNode::execute(entity, deltaMillis) == CANNOTEXECUTE) {
			return CANNOTEXECUTE;
		}

		const TreeNodePtr& treeNode = *_children.begin();
		const TreeNodeStatus status = treeNode->execute(entity, deltaMillis);
		if (status == RUNNING) {
			return state(entity, RUNNING);
		}
		return state(entity, FAILED);
	}
};

}

// #include "tree/Limit.h"
/**
 * @file
 */


// #include "tree/TreeNode.h"


namespace ai {

/**
 * @brief A decorator node which limits the execution of the attached child to a
 * specified amount of runs.
 *
 * @note If the configured amount is reached, the return @c TreeNodeStatus is
 * @c TreeNodeStatus::FINISHED
 */
class Limit: public TreeNode {
private:
	int _amount;
public:
	NODE_FACTORY(Limit)

	Limit(const std::string& name, const std::string& parameters, const ConditionPtr& condition) :
		TreeNode(name, parameters, condition) {
		_type = "Limit";
		if (!parameters.empty()) {
			_amount = ::atoi(parameters.c_str());
		} else {
			_amount = 1;
		}
	}

	TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis) override {
		ai_assert(_children.size() == 1, "Limit must have exactly one node");

		if (TreeNode::execute(entity, deltaMillis) == CANNOTEXECUTE) {
			return CANNOTEXECUTE;
		}

		const int alreadyExecuted = getLimitState(entity);
		if (alreadyExecuted >= _amount) {
			return state(entity, FINISHED);
		}

		const TreeNodePtr& treeNode = *_children.begin();
		const TreeNodeStatus status = treeNode->execute(entity, deltaMillis);
		setLimitState(entity, alreadyExecuted + 1);
		if (status == RUNNING) {
			return state(entity, RUNNING);
		}
		return state(entity, FAILED);
	}
};

}

// #include "tree/Invert.h"
/**
 * @file
 */


// #include "tree/TreeNode.h"


namespace ai {

/**
 * @brief A node with only one child attached. The result of the attached child is inverted.
 *
 * - If the child returns a TreeNodeStatus::FINISHED, this node will return TreeNodeStatus::FAILED
 * - If the child returns a TreeNodeStatus::FAILED, this node will return TreeNodeStatus::FINISHED
 * - otherwise this node will return a TreeNodeStatus::RUNNING
 */
class Invert: public TreeNode {
public:
	NODE_CLASS(Invert)

	TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis) override {
		if (_children.size() != 1) {
			ai_assert(false, "Invert must have exactly one child");
		}

		if (TreeNode::execute(entity, deltaMillis) == CANNOTEXECUTE) {
			return CANNOTEXECUTE;
		}

		const TreeNodePtr& treeNode = *_children.begin();
		const TreeNodeStatus status = treeNode->execute(entity, deltaMillis);
		if (status == FINISHED) {
			return state(entity, FAILED);
		} else if (status == FAILED) {
			return state(entity, FINISHED);
		} else if (status == EXCEPTION) {
			return state(entity, EXCEPTION);
		} else if (status == CANNOTEXECUTE) {
			return state(entity, FINISHED);
		}
		return state(entity, RUNNING);
	}
};

}

// #include "tree/Idle.h"
/**
 * @file
 */


// #include "tree/ITimedNode.h"
/**
 * @file
 */


// #include "tree/TreeNode.h"

#include <stdlib.h>

namespace ai {

#define NOTSTARTED -1
#define TIMERNODE_CLASS(NodeName) \
	NodeName(const std::string& name, const std::string& parameters, const ConditionPtr& condition) : \
		ITimedNode(name, parameters, condition) { \
		_type = AI_STRINGIFY(NodeName); \
	} \
	virtual ~NodeName() { \
	} \
	\
	NODE_FACTORY(NodeName)

/**
 * @brief A timed node is a @c TreeNode that is executed until a given time (millis) is elapsed.
 */
class ITimedNode : public TreeNode {
protected:
	int64_t _timerMillis;
	int64_t _millis;
public:
	ITimedNode(const std::string& name, const std::string& parameters, const ConditionPtr& condition) :
			TreeNode(name, parameters, condition), _timerMillis(NOTSTARTED) {
		if (!parameters.empty()) {
			_millis = ::atol(parameters.c_str());
		} else {
			_millis = 1000L;
		}
	}
	virtual ~ITimedNode() {}

	TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis) override {
		const TreeNodeStatus result = TreeNode::execute(entity, deltaMillis);
		if (result == CANNOTEXECUTE)
			return CANNOTEXECUTE;

		if (_timerMillis == NOTSTARTED) {
			_timerMillis = _millis;
			const TreeNodeStatus status = executeStart(entity, deltaMillis);
			if (status == FINISHED)
				_timerMillis = NOTSTARTED;
			return state(entity, status);
		}

		if (_timerMillis - deltaMillis > 0) {
			_timerMillis -= deltaMillis;
			const TreeNodeStatus status = executeRunning(entity, deltaMillis);
			if (status == FINISHED)
				_timerMillis = NOTSTARTED;
			return state(entity, status);
		}

		_timerMillis = NOTSTARTED;
		return state(entity, executeExpired(entity, deltaMillis));
	}

	/**
	 * @brief Called whenever the timer is started or restarted
	 * @note The returned @c TreeNodeStatus is recorded automatically
	 */
	virtual TreeNodeStatus executeStart(const AIPtr& /*entity*/, int64_t /*deltaMillis*/) {
		return RUNNING;
	}

	/**
	 * @brief Called whenever the timer is running. Not called in the frame where the timer
	 * is started or in the frame where it expired.
	 * @note If you have a timer started, don't get into the timer callbacks for some time (e.g.
	 * the attached @c ICondition evaluation prevents the action from being executed), you will
	 * not get into @c executeRunning, but directly into @c executeExpired.
	 * @note The returned @c TreeNodeStatus is recorded automatically
	 */
	virtual TreeNodeStatus executeRunning(const AIPtr& /*entity*/, int64_t /*deltaMillis*/) {
		return RUNNING;
	}

	/**
	 * @brief Called in the frame where the timer expired.
	 * @note The returned @c TreeNodeStatus is recorded automatically
	 */
	virtual TreeNodeStatus executeExpired(const AIPtr& /*entity*/, int64_t /*deltaMillis*/) {
		return FINISHED;
	}
};

#undef NOTSTARTED

}


namespace ai {

/**
 * @brief @c ITimedNode that is just idling until the given time is elapsed.
 */
class Idle: public ai::ITimedNode {
public:
	TIMERNODE_CLASS(Idle)
};

}

// #include "tree/Parallel.h"
/**
 * @file
 */


// #include "tree/Selector.h"
/**
 * @file
 */


// #include "tree/TreeNode.h"


namespace ai {

#define SELECTOR_CLASS(NodeName) \
	NodeName(const std::string& name, const std::string& parameters, const ConditionPtr& condition) : \
		Selector(name, parameters, condition) { \
		_type = AI_STRINGIFY(NodeName); \
	} \
	virtual ~NodeName() { \
	} \
	\
	NODE_FACTORY(NodeName)

/**
 * @brief Base class for all type of @c TreeNode selectors.
 *
 * [AiGameDev](http://aigamedev.com/open/article/selector/)
 */
class Selector: public TreeNode {
public:
	NODE_CLASS(Selector)

	/**
	 * @brief Will only deliver valid results if the debugging for the given entity is active
	 */
	virtual void getRunningChildren(const AIPtr& entity, std::vector<bool>& active) const override {
		int n = 0;
		int selectorState = getSelectorState(entity);
		for (TreeNodes::const_iterator i = _children.begin(); i != _children.end(); ++i, ++n) {
			active.push_back(selectorState == n);
		}
	}
};

}

// #include "AI.h"


namespace ai {

/**
 * @brief Executes all the connected children in the order they were added (no matter what
 * the TreeNodeStatus of the previous child was).
 *
 * http://aigamedev.com/open/article/parallel/
 */
class Parallel: public Selector {
public:
	SELECTOR_CLASS(Parallel)

	void getRunningChildren(const AIPtr& entity, std::vector<bool>& active) const override {
		for (TreeNodes::const_iterator i = _children.begin(); i != _children.end(); ++i) {
			active.push_back((*i)->getLastStatus(entity) != RUNNING);
		}
	}
	/**
	 * @brief If one of the children was executed, and is still running, the ::TreeNodeStatus::RUNNING
	 * is returned, otherwise ::TreeNodeStatus::FINISHED is returned.
	 */
	TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis) override {
		if (Selector::execute(entity, deltaMillis) == CANNOTEXECUTE) {
			return CANNOTEXECUTE;
		}

		bool totalStatus = false;
		for (const TreeNodePtr& child : _children) {
			const bool isActive = child->execute(entity, deltaMillis) == RUNNING;
			if (!isActive) {
				child->resetState(entity);
			}
			totalStatus |= isActive;
		}

		if (!totalStatus) {
			resetState(entity);
		}
		return state(entity, totalStatus ? RUNNING : FINISHED);
	}
};

}

// #include "tree/PrioritySelector.h"
/**
 * @file
 */


// #include "tree/Selector.h"

// #include "AI.h"


namespace ai {

/**
 * @brief This node tries to execute all the attached children until one succeeds. This composite only
 * fails if all children failed, too.
 *
 * http://aigamedev.com/open/article/selector/
 */
class PrioritySelector: public Selector {
public:
	SELECTOR_CLASS(PrioritySelector)

	TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis) override {
		if (Selector::execute(entity, deltaMillis) == CANNOTEXECUTE)
			return CANNOTEXECUTE;

		int index = getSelectorState(entity);
		if (index == AI_NOTHING_SELECTED) {
			index = 0;
		}
		const std::size_t size = _children.size();
		TreeNodeStatus overallResult = FINISHED;
		std::size_t i = index;
		for (std::size_t j = 0; j < i; ++j) {
			_children[j]->resetState(entity);
		}
		for (; i < size; ++i) {
			const TreeNodePtr& child = _children[i];
			const TreeNodeStatus result = child->execute(entity, deltaMillis);
			if (result == RUNNING) {
				setSelectorState(entity, static_cast<int>(i));
			} else if (result == CANNOTEXECUTE || result == FAILED) {
				child->resetState(entity);
				setSelectorState(entity, AI_NOTHING_SELECTED);
				continue;
			} else {
				setSelectorState(entity, AI_NOTHING_SELECTED);
			}
			child->resetState(entity);
			overallResult = result;
			break;
		}
		for (++i; i < size; ++i) {
			_children[i]->resetState(entity);
		}
		return state(entity, overallResult);
	}
};

}

// #include "tree/ProbabilitySelector.h"
/**
 * @file
 */


// #include "tree/Selector.h"

#include <vector>
// #include "AI.h"

// #include "common/String.h"

// #include "common/Random.h"


namespace ai {

/**
 * @brief This node executes one of the attached children randomly based on the given weights. The node is
 * executed until it is no longer in the running state
 *
 * http://aigamedev.com/open/article/selector/
 */
class ProbabilitySelector: public Selector {
protected:
	std::vector<float> _weights;
	float _weightSum;
public:
	ProbabilitySelector(const std::string& name, const std::string& parameters, const ConditionPtr& condition) :
			Selector(name, parameters, condition), _weightSum(0.0f) {
		std::vector<std::string> tokens;
		Str::splitString(parameters, tokens, ",");
		const int weightAmount = static_cast<int>(tokens.size());
		for (int i = 0; i < weightAmount; i++) {
			const float weight = Str::strToFloat(tokens[i]);
			_weightSum += weight;
			_weights[i] = weight;
		}
	}

	virtual ~ProbabilitySelector() {
	}

	NODE_FACTORY(ProbabilitySelector)

	TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis) override {
		if (Selector::execute(entity, deltaMillis) == CANNOTEXECUTE)
			return CANNOTEXECUTE;

		int index = getSelectorState(entity);
		if (index == AI_NOTHING_SELECTED) {
			float rndIndex = ai::randomf(_weightSum);
			const int weightAmount = static_cast<int>(_weights.size());
			for (; index < weightAmount; ++index) {
				if (rndIndex < _weights[index])
					break;
				rndIndex -= _weights[index];
			}
		}

		const TreeNodePtr& child = _children[index];
		const TreeNodeStatus result = child->execute(entity, deltaMillis);
		if (result == RUNNING) {
			setSelectorState(entity, index);
		} else {
			setSelectorState(entity, AI_NOTHING_SELECTED);
		}
		child->resetState(entity);

		const int size = static_cast<int>(_children.size());
		for (int i = 0; i < size; ++i) {
			if (i == index)
				continue;
			_children[i]->resetState(entity);
		}
		return state(entity, result);
	}
};

}

// #include "tree/RandomSelector.h"
/**
 * @file
 */


// #include "tree/Selector.h"

// #include "AI.h"


namespace ai {

/**
 * @brief This node executes all the attached children in random order. This composite only
 * fails if all children failed, too. It doesn't continue a node in the state
 * @c TreeNodeStatus::RUNNING. It will always pick a new random node in each tick.
 *
 * http://aigamedev.com/open/article/selector/
 */
class RandomSelector: public Selector {
public:
	SELECTOR_CLASS(RandomSelector)

	TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis) override {
		if (Selector::execute(entity, deltaMillis) == CANNOTEXECUTE) {
			return CANNOTEXECUTE;
		}

		TreeNodes childrenShuffled = _children;
		const std::size_t size = childrenShuffled.size();
		ai::shuffle(childrenShuffled.begin(), childrenShuffled.end());
		TreeNodeStatus overallResult = FINISHED;
		std::size_t i;
		for (i = 0; i < size; ++i) {
			const TreeNodePtr& child = childrenShuffled[i];
			const TreeNodeStatus result = child->execute(entity, deltaMillis);
			if (result == RUNNING) {
				continue;
			} else if (result == CANNOTEXECUTE || result == FAILED) {
				overallResult = result;
			}
			child->resetState(entity);
		}
		for (++i; i < size; ++i) {
			childrenShuffled[i]->resetState(entity);
		}
		return state(entity, overallResult);
	}
};

}

// #include "tree/Sequence.h"
/**
 * @file
 */


// #include "tree/Selector.h"

// #include "AI.h"


namespace ai {

/**
 * @brief The sequence continues to execute their children until one of the children
 * returned a state that is not equal to finished. On the next iteration the execution
 * is continued at the last running children or from the start again if no such
 * children exists.
 *
 * [AiGameDev](http://aigamedev.com/open/article/sequence/)
 */
class Sequence: public Selector {
public:
	SELECTOR_CLASS(Sequence)

	TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis) override {
		if (Selector::execute(entity, deltaMillis) == CANNOTEXECUTE)
			return CANNOTEXECUTE;

		TreeNodeStatus result = FINISHED;
		const int progress = std::max(0, getSelectorState(entity));

		const std::size_t size = _children.size();
		for (std::size_t i = static_cast<std::size_t>(progress); i < size; ++i) {
			TreeNodePtr& child = _children[i];

			result = child->execute(entity, deltaMillis);

			if (result == RUNNING) {
				setSelectorState(entity, static_cast<int>(i));
				break;
			} else if (result == CANNOTEXECUTE || result == FAILED) {
				resetState(entity);
				break;
			} else if (result == EXCEPTION) {
				break;
			}
		}

		if (result != RUNNING) {
			resetState(entity);
		}

		return state(entity, result);
	}

	void resetState(const AIPtr& entity) override {
		setSelectorState(entity, AI_NOTHING_SELECTED);
		TreeNode::resetState(entity);
	}
};

}

// #include "tree/Steer.h"
/**
 * @file
 */


// #include "tree/ITask.h"
/**
 * @file
 */


// #include "tree/TreeNode.h"

// #include "common/Types.h"


namespace ai {

/**
 * @brief Macro for the constructor of a task. Just give the class name as parameter.
 */
#define TASK_CLASS_CTOR(TaskName) \
	TaskName(const std::string& name, const std::string& parameters, const ::ai::ConditionPtr& condition) : \
			::ai::ITask(name, parameters, condition)
/**
 * @brief Macro for the destructor of a task. Just give the class name as parameter.
 */
#define TASK_CLASS_DTOR(TaskName) virtual ~TaskName()

/**
 * @brief Macro to simplify the task creation. Just give the class name as parameter.
 */
#define TASK_CLASS(TaskName) \
	TASK_CLASS_CTOR(TaskName) {}\
	TASK_CLASS_DTOR(TaskName) {}

/**
 * @brief A node for your real actions in the behaviour tree
 * @note This node doesn't support children
 */
class ITask: public TreeNode {
protected:
	TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis) override {
		if (TreeNode::execute(entity, deltaMillis) == CANNOTEXECUTE) {
			return CANNOTEXECUTE;
		}

#if AI_EXCEPTIONS
		try {
#endif
			return state(entity, doAction(entity, deltaMillis));
#if AI_EXCEPTIONS
		} catch (...) {
			ai_log_error("Exception while running task %s of type %s", _name.c_str(), _type.c_str());
		}
		return state(entity, EXCEPTION);
#endif
	}
public:
	ITask(const std::string& name, const std::string& parameters, const ConditionPtr& condition) :
			TreeNode(name, parameters, condition) {
	}

	virtual ~ITask() {
	}

	/**
	 * @note The returned @c TreeNodeStatus is automatically recorded. This method is only
	 * called when the attached @c ICondition evaluated to @c true
	 */
	virtual TreeNodeStatus doAction(const AIPtr& entity, int64_t deltaMillis) = 0;

	bool addChild(const TreeNodePtr& /*child*/) override {
		return false;
	}
};

}

// #include "common/String.h"

// #include "common/Types.h"

// #include "common/Math.h"

// #include "movement/Steering.h"
/**
 * @file
 * @brief Defines some basic movement algorithms like Wandering, Seeking and Fleeing.
 */


// #include "AI.h"

// #include "zone/Zone.h"
/**
 * @file
 *
 * @defgroup Zone
 * @{
 * A zone is a logical unit that groups AI instances.
 *
 * Zones should have unique names - this is for the debug server to print the zone names to
 * debug them properly.
 *
 * Each zone has a dedicated ai::GroupMgr instance. For more detailed information, just check out
 * the [sourcecode](src/ai/zone/Zone.h).
 */


// #include "ICharacter.h"

// #include "group/GroupMgr.h"
/**
 * @file
 */


// #include "common/Thread.h"

// #include "common/Types.h"

// #include "common/Math.h"

// #include "ICharacter.h"

// #include "AI.h"

#include <map>
#include <memory>
#include <numeric>
#include <unordered_map>
#include <unordered_set>

namespace ai {

/**
 * @brief Maintains the groups a @c AI can be in.
 * @note Keep in mind that if you destroy an @c AI somewhere in the game, to also
 * remove it from the groups.
 *
 * Every @ai{Zone} has its own @c GroupMgr instance. It is automatically updated with the zone.
 * The average group position is only updated once per @c update() call.
 */
class GroupMgr {
private:
	struct AveragePositionFunctor {
		glm::vec3 operator()(const glm::vec3& result, const AIPtr& ai) {
			return ai->getCharacter()->getPosition() + result;
		}
	};

	typedef std::unordered_set<AIPtr> GroupMembersSet;
	typedef GroupMembersSet::iterator GroupMembersSetIter;
	typedef GroupMembersSet::const_iterator GroupMembersSetConstIter;

	ReadWriteLock _groupLock = {"groupmgr-group"};
	struct Group {
		AIPtr leader;
		GroupMembersSet members;
		glm::vec3 position;
	};

	typedef std::unordered_multimap<AIPtr, GroupId> GroupMembers;
	typedef std::unordered_map<GroupId, Group> Groups;
	typedef Groups::const_iterator GroupsConstIter;
	typedef Groups::iterator GroupsIter;

	GroupMembersSet _empty;
	ReadWriteLock _lock = {"groupmgr"};
	Groups _groups;
	GroupMembers _groupMembers;

public:
	GroupMgr () {
	}
	virtual ~GroupMgr () {
	}

	/**
	 * @brief Adds a new group member to the given @ai{GroupId}. If the group does not yet
	 * exists, it it created and the given @ai{AI} instance will be the leader of the
	 * group.
	 *
	 * @sa remove()
	 *
	 * @param ai The @ai{AI} to add to the group. Keep
	 * in mind that you have to remove it manually from any group
	 * whenever you destroy the @ai{AI} instance.
	 * @return @c true if the add to the group was successful.
	 *
	 * @note This method performs a write lock on the group manager
	 */
	bool add(GroupId id, const AIPtr& ai);

	void update(int64_t deltaTime);

	/**
	 * @brief Removes a group member from the given @ai{GroupId}. If the member
	 * is the group leader, a new leader will be picked randomly. If after the
	 * removal of the member only one other member is left in the group, the
	 * group is destroyed.
	 *
	 * @param ai The @ai{AI} to remove from this the group.
	 * @return @c true if the given @ai{AI} was removed from the group,
	 * @c false if the removal failed (e.g. the @ai{AI} instance was not part of
	 * the group)
	 *
	 * @note This method performs a write lock on the group manager
	 */
	bool remove(GroupId id, const AIPtr& ai);

	/**
	 * @brief Use this method to remove a @ai{AI} instance from all the group it is
	 * part of. Useful if you e.g. destroy a @ai{AI} instance.
	 *
	 * @note This method performs a write lock on the group manager
	 */
	bool removeFromAllGroups(const AIPtr& ai);

	/**
	 * @brief Returns the average position of the group
	 *
	 * @note If the given group doesn't exist or some other error occurred, this method returns @c glm::vec3::VEC3_INFINITE
	 * @note The position of a group is calculated once per @c update() call.
	 *
	 * @note This method performs a read lock on the group manager
	 */
	glm::vec3 getPosition(GroupId id) const;

	/**
	 * @return The @ai{ICharacter} object of the leader, or @c nullptr if no such group exists.
	 *
	 * @note This method performs a read lock on the group manager
	 */
	AIPtr getLeader(GroupId id) const;

	/**
	 * @brief Visit all the group members of the given group until the functor returns @c false
	 *
	 * @note This methods performs a read lock on the group manager
	 */
	template<typename Func>
	void visit(GroupId id, Func& func) const {
		ScopedReadLock scopedLock(_lock);
		const GroupsConstIter& i = _groups.find(id);
		if (i == _groups.end()) {
			return;
		}
		for (GroupMembersSetConstIter it = i->second.members.begin(); it != i->second.members.end(); ++it) {
			const AIPtr& chr = *it;
			if (!func(chr))
				break;
		}
	}

	/**
	 * @return If the group doesn't exist, this method returns @c 0 - otherwise the amount of members
	 * that must be bigger than @c 1
	 *
	 * @note This method performs a read lock on the group manager
	 */
	int getGroupSize(GroupId id) const;

	/**
	 * @note This method performs a read lock on the group manager
	 */
	bool isInAnyGroup(const AIPtr& ai) const;

	/**
	 * @note This method performs a read lock on the group manager
	 */
	bool isInGroup(GroupId id, const AIPtr& ai) const;

	/**
	 * @note This method performs a read lock on the group manager
	 */
	bool isGroupLeader(GroupId id, const AIPtr& ai) const;
};


inline void GroupMgr::update(int64_t) {
	ScopedReadLock scopedLock(_lock);
	for (auto i = _groups.begin(); i != _groups.end(); ++i) {
		Group& group = i->second;
		glm::vec3 averagePosition(0.0f);
		{
			ScopedReadLock lock(_groupLock);
			averagePosition = std::accumulate(group.members.begin(), group.members.end(), glm::vec3(0.0f), AveragePositionFunctor());
			averagePosition *= 1.0f / (float) group.members.size();
		}
		ScopedWriteLock lock(_groupLock);
		group.position = averagePosition;
	}
}

inline bool GroupMgr::add(GroupId id, const AIPtr& ai) {
	ScopedWriteLock scopedLock(_lock);
	GroupsIter i = _groups.find(id);
	if (i == _groups.end()) {
		Group group;
		group.leader = ai;
		i = _groups.insert(std::pair<GroupId, Group>(id, group)).first;
	}

	Group& group = i->second;
	ScopedWriteLock lock(_groupLock);
	std::pair<GroupMembersSetIter, bool> ret = group.members.insert(ai);
	if (ret.second) {
		_groupMembers.insert(GroupMembers::value_type(ai, id));
		return true;
	}
	return false;
}

inline bool GroupMgr::remove(GroupId id, const AIPtr& ai) {
	ScopedWriteLock scopedLock(_lock);
	const GroupsIter& i = _groups.find(id);
	if (i == _groups.end()) {
		return false;
	}
	Group& group = i->second;
	GroupMembersSetIter si;
	{
		ScopedReadLock lock(_groupLock);
		si = group.members.find(ai);
		if (si == group.members.end()) {
			return false;
		}
	}
	{
		ScopedWriteLock lock(_groupLock);
		group.members.erase(si);
		if (group.members.empty()) {
			_groups.erase(i);
		} else if (group.leader == ai) {
			group.leader = *group.members.begin();
		}
	}

	auto range = _groupMembers.equal_range(ai);
	for (auto it = range.first; it != range.second; ++it) {
		if (it->second == id) {
			_groupMembers.erase(it);
			break;
		}
	}
	return true;
}

inline bool GroupMgr::removeFromAllGroups(const AIPtr& ai) {
	std::list<GroupId> groups;
	{
		ScopedReadLock scopedLock(_lock);
		auto range = _groupMembers.equal_range(ai);
		for (auto it = range.first; it != range.second; ++it) {
			groups.push_back(it->second);
		}
	}
	for (GroupId groupId : groups) {
		remove(groupId, ai);
	}
	return true;
}

inline AIPtr GroupMgr::getLeader(GroupId id) const {
	ScopedReadLock scopedLock(_lock);
	const GroupsConstIter& i = _groups.find(id);
	if (i == _groups.end()) {
		return AIPtr();
	}

	ScopedReadLock lock(_groupLock);
	return i->second.leader;
}

inline glm::vec3 GroupMgr::getPosition(GroupId id) const {
	ScopedReadLock scopedLock(_lock);
	const GroupsConstIter& i = _groups.find(id);
	if (i == _groups.end()) {
		return VEC3_INFINITE;
	}

	ScopedReadLock lock(_groupLock);
	return i->second.position;
}

inline bool GroupMgr::isGroupLeader(GroupId id, const AIPtr& ai) const {
	ScopedReadLock scopedLock(_lock);
	const GroupsConstIter& i = _groups.find(id);
	if (i == _groups.end()) {
		return 0;
	}
	ScopedReadLock lock(_groupLock);
	return i->second.leader == ai;
}

inline int GroupMgr::getGroupSize(GroupId id) const {
	ScopedReadLock scopedLock(_lock);
	const GroupsConstIter& i = _groups.find(id);
	if (i == _groups.end()) {
		return 0;
	}
	ScopedReadLock lock(_groupLock);
	return static_cast<int>(std::distance(i->second.members.begin(), i->second.members.end()));
}

inline bool GroupMgr::isInAnyGroup(const AIPtr& ai) const {
	ScopedReadLock scopedLock(_lock);
	return _groupMembers.find(ai) != _groupMembers.end();
}

inline bool GroupMgr::isInGroup(GroupId id, const AIPtr& ai) const {
	ScopedReadLock scopedLock(_lock);
	auto range = _groupMembers.equal_range(ai);
	for (auto it = range.first; it != range.second; ++it) {
		if (it->second == id) {
			return true;
		}
	}
	return false;
}

}

// #include "common/Thread.h"

// #include "common/ThreadPool.h"

// #include "common/Types.h"

// #include "common/ExecutionTime.h"

#include <unordered_map>
#include <vector>
#include <memory>

namespace ai {

class AI;
typedef std::shared_ptr<AI> AIPtr;

/**
 * @brief A zone represents one logical zone that groups AI instances.
 *
 * You have to update the AI instances in this zone in your tick by calling
 * @c Zone::update.
 *
 * Zones should have unique names - otherwise the @c Server won't be able to
 * select a particular @c Zone to debug it.
 */
class Zone {
public:
	typedef std::unordered_map<CharacterId, AIPtr> AIMap;
	typedef std::vector<AIPtr> AIScheduleList;
	typedef std::vector<CharacterId> CharacterIdList;
	typedef AIMap::const_iterator AIMapConstIter;
	typedef AIMap::iterator AIMapIter;

protected:
	const std::string _name;
	AIMap _ais;
	AIScheduleList _scheduledAdd;
	AIScheduleList _scheduledRemove;
	CharacterIdList _scheduledDestroy;
	bool _debug;
	ReadWriteLock _lock {"zone"};
	ReadWriteLock _scheduleLock {"zone-schedulelock"};
	ai::GroupMgr _groupManager;
	mutable ThreadPool _threadPool;

	/**
	 * @brief called in the zone update to add new @c AI instances.
	 *
	 * @note Make sure to also call @c removeAI whenever you despawn the given @c AI instance
	 * @note This doesn't lock the zone - but because @c Zone::update already does it
	 */
	bool doAddAI(const AIPtr& ai);
	/**
	 * @note This doesn't lock the zone - but because @c Zone::update already does it
	 */
	bool doRemoveAI(const AIPtr& ai);
	/**
	 * @brief @c removeAI will access the character and the @c AI object, this method does not need access to the data anymore.
	 *
	 * @note That means, that this can be called in case the attached @c ICharacter instances or the @c AI instance itself is
	 * already invalid.
	 * @note This doesn't lock the zone - but because @c Zone::update already does it
	 */
	bool doDestroyAI(const CharacterId& id);

public:
	Zone(const std::string& name, int threadCount = std::min(1u, std::thread::hardware_concurrency())) :
			_name(name), _debug(false), _threadPool(threadCount) {
	}

	virtual ~Zone() {}

	/**
	 * @brief Update all the @c ICharacter and @c AI instances in this zone.
	 * @param dt Delta time in millis since the last update call happened
	 * @note You have to call this on your own.
	 */
	void update(int64_t dt);

	/**
	 * @brief If you need to add new @code AI entities to a zone from within the @code AI tick (e.g. spawning via behaviour
	 * tree) - then you need to schedule the spawn. Otherwise you will end up in a deadlock
	 *
	 * @note This does not lock the zone for writing but a dedicated schedule lock
	 */
	bool addAI(const AIPtr& ai);

	/**
	 * @brief Add multiple AIPtr instances but only lock once.
	 * @sa addAI()
	 */
	template<class Collection>
	bool addAIs(const Collection& ais) {
		if (ais.empty()) {
			return false;
		}
		ScopedWriteLock scopedLock(_scheduleLock);
		_scheduledAdd.insert(_scheduledAdd.end(), ais.begin(), ais.end());
		return true;
	}

	/**
	 * @brief Will trigger a removal of the specified @c AI instance in the next @c Zone::update call
	 *
	 * @note This does not lock the zone for writing but a dedicated schedule lock
	 */
	bool removeAI(const AIPtr& ai);

	/**
	 * @brief Remove multiple AIPtr instances but only lock once.
	 * @sa removeAI()
	 */
	template<class Collection>
	bool removeAIs(const Collection& ais) {
		if (ais.empty()) {
			return false;
		}
		ScopedWriteLock scopedLock(_scheduleLock);
		_scheduledRemove.insert(_scheduledRemove.end(), ais.begin(), ais.end());
		return true;
	}

	/**
	 * @brief Will trigger a destroy of the specified @c AI instance in the next @c Zone::update call
	 * @sa destroyAI
	 * @note @c removeAI will access the character and the @c AI object, this method does not need access to the data anymore.
	 * That means, that this can be called in case the attached @c ICharacter instances or the @c AI instance itself is
	 * already invalid.
	 */
	bool destroyAI(const CharacterId& id);

	/**
	 * @brief Every zone has its own name that identifies it
	 */
	const std::string& getName() const;

	/**
	 * @brief Activate the debugging for this particular server instance
	 * @param[in] debug @c true if you want to activate the debugging and send
	 * the npc states of this server to all connected clients, or @c false if
	 * none of the managed entities is broadcasted.
	 */
	void setDebug(bool debug);
	bool isDebug () const;

	GroupMgr& getGroupMgr();

	const GroupMgr& getGroupMgr() const;

	/**
	 * @brief Lookup for a particular @c AI in the zone.
	 *
	 * @return empty @c AIPtr() in the case the given @c CharacterId wasn't found in this zone.
	 *
	 * @note This locks the zone for reading to perform the CharacterId lookup
	 */
	inline AIPtr getAI(CharacterId id) const {
		ScopedReadLock scopedLock(_lock);
		auto i = _ais.find(id);
		if (i == _ais.end()) {
			return AIPtr();
		}
		const AIPtr& ai = i->second;
		return ai;
	}

	/**
	 * @brief Executes a lambda or functor for the given character
	 *
	 * @return @c true if the func is going to get called for the character, @c false if not
	 * e.g. in the case the given @c CharacterId wasn't found in this zone.
	 * @note This is executed in a thread pool - so make sure to synchronize your lambda or functor.
	 * We also don't wait for the functor or lambda here, we are scheduling it in a worker in the
	 * thread pool.
	 *
	 * @note This locks the zone for reading to perform the CharacterId lookup
	 */
	template<typename Func>
	inline bool executeAsync(CharacterId id, const Func& func) const {
		const AIPtr& ai = getAI(id);
		if (!ai) {
			return false;
		}
		executeAsync(ai, func);
		return true;
	}

	/**
	 * @brief Executes a lambda or functor for the given character
	 *
	 * @returns @c std::future with the result of @c func.
	 * @note This is executed in a thread pool - so make sure to synchronize your lambda or functor.
	 * We also don't wait for the functor or lambda here, we are scheduling it in a worker in the
	 * thread pool. If you want to wait - you have to use the returned future.
	 */
	template<typename Func>
	inline auto executeAsync(const AIPtr& ai, const Func& func) const
		-> std::future<typename std::result_of<Func(const AIPtr&)>::type> {
		return _threadPool.enqueue(func, ai);
	}

	template<typename Func>
	inline auto execute(const AIPtr& ai, const Func& func) const
		-> typename std::result_of<Func(const AIPtr&)>::type {
		return func(ai);
	}

	template<typename Func>
	inline auto execute(const AIPtr& ai, Func& func)
		-> typename std::result_of<Func(const AIPtr&)>::type {
		return func(ai);
	}

	/**
	 * @brief The given functor or lambda must be able to deal with invalid @c AIPtr instances
	 * @note It's possible that the given @c CharacterId can't be found in the @c Zone.
	 * @return the return value of your functor or lambda
	 */
	template<typename Func>
	inline auto execute(CharacterId id, const Func& func) const
		-> typename std::result_of<Func(const AIPtr&)>::type {
		return execute(getAI(id), func);
	}

	/**
	 * @brief The given functor or lambda must be able to deal with invalid @c AIPtr instances
	 * @note It's possible that the given @c CharacterId can't be found in the @c Zone.
	 * @return the return value of your functor or lambda
	 */
	template<typename Func>
	inline auto execute(CharacterId id, Func& func)
		-> typename std::result_of<Func(const AIPtr&)>::type {
		return execute(getAI(id), func);
	}

	/**
	 * @brief Executes a lambda or functor for all the @c AI instances in this zone
	 * @note This is executed in a thread pool - so make sure to synchronize your lambda or functor.
	 * We are waiting for the execution of this.
	 *
	 * @note This locks the zone for reading
	 */
	template<typename Func>
	void executeParallel(Func& func) {
		std::vector<std::future<void> > results;
		_lock.lockRead();
		AIMap copy(_ais);
		_lock.unlockRead();
		for (auto i = copy.begin(); i != copy.end(); ++i) {
			const AIPtr& ai = i->second;
			results.emplace_back(executeAsync(ai, func));
		}
		for (auto & result: results) {
			result.wait();
		}
	}

	/**
	 * @brief Executes a lambda or functor for all the @c AI instances in this zone.
	 * @note This is executed in a thread pool - so make sure to synchronize your lambda or functor.
	 * We are waiting for the execution of this.
	 *
	 * @note This locks the zone for reading
	 */
	template<typename Func>
	void executeParallel(const Func& func) const {
		std::vector<std::future<void> > results;
		_lock.lockRead();
		AIMap copy(_ais);
		_lock.unlockRead();
		for (auto i = copy.begin(); i != copy.end(); ++i) {
			const AIPtr& ai = i->second;
			results.emplace_back(executeAsync(ai, func));
		}
		for (auto & result: results) {
			result.wait();
		}
	}

	/**
	 * @brief Executes a lambda or functor for all the @c AI instances in this zone
	 * We are waiting for the execution of this.
	 *
	 * @note This locks the zone for reading
	 */
	template<typename Func>
	void execute(const Func& func) const {
		_lock.lockRead();
		AIMap copy(_ais);
		_lock.unlockRead();
		for (auto i = copy.begin(); i != copy.end(); ++i) {
			const AIPtr& ai = i->second;
			func(ai);
		}
	}

	/**
	 * @brief Executes a lambda or functor for all the @c AI instances in this zone
	 * We are waiting for the execution of this.
	 *
	 * @note This locks the zone for reading
	 */
	template<typename Func>
	void execute(Func& func) {
		_lock.lockRead();
		AIMap copy(_ais);
		_lock.unlockRead();
		for (auto i = copy.begin(); i != copy.end(); ++i) {
			const AIPtr& ai = i->second;
			func(ai);
		}
	}

	inline std::size_t size() const {
		ScopedReadLock scopedLock(_lock);
		return _ais.size();
	}
};

inline void Zone::setDebug (bool debug) {
	_debug = debug;
}

inline bool Zone::isDebug () const {
	return _debug;
}

inline const std::string& Zone::getName() const {
	return _name;
}

inline GroupMgr& Zone::getGroupMgr() {
	return _groupManager;
}

inline const GroupMgr& Zone::getGroupMgr() const {
	return _groupManager;
}

inline bool Zone::doAddAI(const AIPtr& ai) {
	if (ai == nullptr) {
		return false;
	}
	const CharacterId& id = ai->getCharacter()->getId();
	if (_ais.find(id) != _ais.end()) {
		return false;
	}
	_ais.insert(std::make_pair(id, ai));
	ai->setZone(this);
	return true;
}

inline bool Zone::doRemoveAI(const AIPtr& ai) {
	if (!ai) {
		return false;
	}
	const CharacterId& id = ai->getCharacter()->getId();
	AIMapIter i = _ais.find(id);
	if (i == _ais.end()) {
		return false;
	}
	i->second->setZone(nullptr);
	_groupManager.removeFromAllGroups(i->second);
	_ais.erase(i);
	return true;
}

inline bool Zone::doDestroyAI(const CharacterId& id) {
	AIMapIter i = _ais.find(id);
	if (i == _ais.end()) {
		return false;
	}
	_ais.erase(i);
	return true;
}

inline bool Zone::addAI(const AIPtr& ai) {
	if (!ai) {
		return false;
	}
	ScopedWriteLock scopedLock(_scheduleLock);
	_scheduledAdd.push_back(ai);
	return true;
}

inline bool Zone::destroyAI(const CharacterId& id) {
	ScopedWriteLock scopedLock(_scheduleLock);
	_scheduledDestroy.push_back(id);
	return true;
}

inline bool Zone::removeAI(const AIPtr& ai) {
	if (!ai) {
		return false;
	}
	ScopedWriteLock scopedLock(_scheduleLock);
	_scheduledRemove.push_back(ai);
	return true;
}

inline void Zone::update(int64_t dt) {
	{
		AIScheduleList scheduledRemove;
		AIScheduleList scheduledAdd;
		CharacterIdList scheduledDestroy;
		{
			ScopedWriteLock scopedLock(_scheduleLock);
			scheduledAdd.swap(_scheduledAdd);
			scheduledRemove.swap(_scheduledRemove);
			scheduledDestroy.swap(_scheduledDestroy);
		}
		ScopedWriteLock scopedLock(_lock);
		for (const AIPtr& ai : scheduledAdd) {
			doAddAI(ai);
		}
		scheduledAdd.clear();
		for (const AIPtr& ai : scheduledRemove) {
			doRemoveAI(ai);
		}
		scheduledRemove.clear();
		for (auto id : scheduledDestroy) {
			doDestroyAI(id);
		}
		scheduledDestroy.clear();
	}

	auto func = [&] (const AIPtr& ai) {
		if (ai->isPause()) {
			return;
		}
		ai->update(dt, _debug);
		ai->getBehaviour()->execute(ai, dt);
	};
	executeParallel(func);
	_groupManager.update(dt);
}

}

// #include "IAIFactory.h"

// #include "common/Math.h"

// #include "common/MoveVector.h"

// #include "common/String.h"

// #include "common/MemoryAllocator.h"

// #include "ICharacter.h"


namespace ai {
namespace movement {

#define STEERING_FACTORY(SteeringName) \
public: \
	class Factory: public ::ai::ISteeringFactory { \
	public: \
		::ai::SteeringPtr create (const ::ai::SteeringFactoryContext *ctx) const override { \
			return std::make_shared<SteeringName>(ctx->parameters); \
		} \
	}; \
	static const Factory& getFactory() { \
		static Factory FACTORY; \
		return FACTORY; \
	}

#define STEERING_FACTORY_SINGLETON \
public: \
	class Factory: public ::ai::ISteeringFactory { \
		::ai::SteeringPtr create (const ::ai::SteeringFactoryContext */*ctx*/) const { \
			return get(); \
		} \
	}; \
	static const Factory& getFactory() { \
		static Factory FACTORY; \
		return FACTORY; \
	}

/**
 * @brief Steering interface
 */
class ISteering : public MemObject {
public:
	virtual ~ISteering() {}
	/**
	 * @brief Calculates the @c MoveVector
	 *
	 * @return If the @c MoveVector contains @c glm::vec3::VEC3_INFINITE as vector, the result should not be used
	 * because there was an error.
	 */
	virtual MoveVector execute (const AIPtr& ai, float speed) const = 0;
};

/**
 * @brief @c IFilter steering interface
 */
class SelectionSteering : public ISteering {
protected:
	glm::vec3 getSelectionTarget(const AIPtr& entity, std::size_t index) const {
		const FilteredEntities& selection = entity->getFilteredEntities();
		if (selection.empty() || selection.size() <= index) {
			return VEC3_INFINITE;
		}
		const Zone* zone = entity->getZone();
		const CharacterId characterId = selection[index];
		const AIPtr& ai = zone->getAI(characterId);
		const ICharacterPtr character = ai->getCharacter();
		return character->getPosition();
	}

public:
	virtual ~SelectionSteering() {}
};

}
}

// #include "movement/WeightedSteering.h"
/**
 * @file
 */


// #include "Steering.h"


namespace ai {
namespace movement {

/**
 * @brief Steering and weight as input for @c WeightedSteering
 */
struct WeightedData {
	SteeringPtr steering;
	const float weight;

	WeightedData(const SteeringPtr& _steering, float _weight = 1.0f) :
			steering(_steering), weight(_weight) {
		ai_assert(weight > 0.0001f, "Weight is too small");
	}
};
typedef std::vector<WeightedData> WeightedSteerings;
typedef WeightedSteerings::const_iterator WeightedSteeringsIter;

/**
 * @brief This class allows you to weight several steering methods and get a blended @c MoveVector out of it.
 */
class WeightedSteering {
private:
	WeightedSteerings _steerings;
public:
	explicit WeightedSteering(const WeightedSteerings& steerings) :
			_steerings(steerings) {
	}

	MoveVector execute (const AIPtr& ai, float speed) const {
		float totalWeight = 0.0f;
		glm::vec3 vecBlended(0.0f);
		float angularBlended = 0.0f;
		for (const WeightedData& wd : _steerings) {
			const float weight = wd.weight;
			const MoveVector& mv = wd.steering->execute(ai, speed);
			if (isInfinite(mv.getVector())) {
				continue;
			}

			vecBlended += mv.getVector() * weight;
			angularBlended += mv.getRotation() * weight;
			totalWeight += weight;
		}

		if (totalWeight <= 0.0000001f) {
			return MoveVector(VEC3_INFINITE, 0.0f);
		}

		const float scale = 1.0f / totalWeight;
		return MoveVector(vecBlended * scale, fmodf(angularBlended * scale, glm::two_pi<float>()));
	}
};

}
}

// #include "common/Random.h"


namespace ai {

class Steer: public ITask {
protected:
	const movement::WeightedSteering _w;
public:
	Steer(const std::string& name, const std::string& parameters, const ConditionPtr& condition, const movement::WeightedSteering &w) :
			ITask(name, parameters, condition), _w(w) {
		_type = "Steer";
	}
	class Factory: public ISteerNodeFactory {
	public:
		TreeNodePtr create (const SteerNodeFactoryContext *ctx) const override {
			movement::WeightedSteerings weightedSteerings;

			if (ctx->parameters.empty()) {
				for (const SteeringPtr& s : ctx->steerings) {
					weightedSteerings.push_back(movement::WeightedData(s, 1.0f));
				}
			} else {
				std::vector<std::string> tokens;
				Str::splitString(ctx->parameters, tokens, ",");
				ai_assert(tokens.size() == ctx->steerings.size(), "weights doesn't match steerings methods count");
				const int tokenAmount = static_cast<int>(tokens.size());
				for (int i = 0; i < tokenAmount; ++i) {
					weightedSteerings.push_back(movement::WeightedData(ctx->steerings[i], Str::strToFloat(tokens[i])));
				}
			}
			const movement::WeightedSteering w(weightedSteerings);
			return std::make_shared<Steer>(ctx->name, ctx->parameters, ctx->condition, w);
		}
	};
	static const Factory& getFactory() {
		static Factory FACTORY;
		return FACTORY;
	}

	TreeNodeStatus doAction(const AIPtr& entity, int64_t deltaMillis) override {
		const ICharacterPtr& chr = entity->getCharacter();
		const MoveVector& mv = _w.execute(entity, chr->getSpeed());
		if (isInfinite(mv.getVector())) {
			return FAILED;
		}

		const float deltaSeconds = static_cast<float>(deltaMillis) / 1000.0f;
		chr->setPosition(chr->getPosition() + (mv.getVector() * deltaSeconds));
		chr->setOrientation(fmodf(chr->getOrientation() + mv.getRotation() * deltaSeconds, glm::two_pi<float>()));
		return FINISHED;
	}
};

}

// #include "tree/Succeed.h"
/**
 * @file
 */


// #include "tree/TreeNode.h"

// #include "common/Types.h"


namespace ai {

/**
 * @brief A decorator node with only one child attached. The result of the attached child is only
 * taken into account if it returned @c TreeNodeStatus::RUNNING - in every other case this decorator
 * will return TreeNodeStatus::FINISHED.
 */
class Succeed: public TreeNode {
public:
	NODE_CLASS(Succeed)

	TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis) override {
		if (_children.size() != 1) {
			ai_assert(false, "Succeed must have exactly one child");
		}

		if (TreeNode::execute(entity, deltaMillis) == CANNOTEXECUTE) {
			return CANNOTEXECUTE;
		}

		const TreeNodePtr& treeNode = *_children.begin();
		const TreeNodeStatus status = treeNode->execute(entity, deltaMillis);
		if (status == RUNNING) {
			return state(entity, RUNNING);
		}
		return state(entity, FINISHED);
	}
};

}

// #include "conditions/And.h"
/**
 * @file
 * @ingroup Condition
 */


// #include "conditions/ICondition.h"


namespace ai {

/**
 * @brief This condition will logically and all contained conditions
 */
class And: public ICondition {
protected:
	Conditions _conditions;
	CONDITION_PRINT_SUBCONDITIONS_GETCONDITIONNAMEWITHVALUE

public:
	explicit And(const Conditions& conditions) :
			ICondition("And", ""), _conditions(conditions) {
	}
	virtual ~And() {
	}

	CONDITION_FACTORY_NO_IMPL(And)

	bool evaluate(const AIPtr& entity) override {
		for (ConditionsIter i = _conditions.begin(); i != _conditions.end(); ++i) {
			if (!(*i)->evaluate(entity)) {
				return false;
			}
		}

		return true;
	}
};

inline ConditionPtr And::Factory::create(const ConditionFactoryContext *ctx) const {
	if (ctx->conditions.size() < 2) {
		return ConditionPtr();
	}
	return std::make_shared<And>(ctx->conditions);
}

}

// #include "conditions/False.h"
/**
 * @file
 * @ingroup Condition
 */


// #include "conditions/ICondition.h"


namespace ai {

/**
 * @brief This condition just always evaluates to @c false
 */
class False: public ICondition {
public:
	CONDITION_CLASS_SINGLETON(False)

	bool evaluate(const AIPtr& /*entity*/) override {
		return false;
	}
};

}

// #include "conditions/HasEnemies.h"
/**
 * @file
 * @ingroup Condition
 */


// #include "conditions/ICondition.h"

// #include "common/String.h"

// #include "aggro/AggroMgr.h"


namespace ai {

/**
 * @brief This condition checks whether there are enemies.
 *
 * You can either check whether there are enemies at all. Or whether there are more than x enemies.
 * The answer is given with the @c AggroMgr. So keep in mind that this might even return @c true in
 * those cases where the enemy is maybe no longer available. It all depends on how you use the
 * @c AggroMgr or how your aggro @c Entry reduce is configured.
 */
class HasEnemies: public ICondition {
protected:
	int _enemyCount;
public:
	CONDITION_FACTORY(HasEnemies)

	explicit HasEnemies(const std::string& parameters) :
			ICondition("HasEnemies", parameters) {
		if (_parameters.empty()) {
			_enemyCount = -1;
		} else {
			_enemyCount = std::stoi(_parameters);
		}
	}

	bool evaluate(const AIPtr& entity) override {
		const AggroMgr& mgr = entity->getAggroMgr();
		if (_enemyCount == -1) {
			// TODO: check why boolean operator isn't working here
			const bool hasEnemy = mgr.getHighestEntry() != nullptr;
			return hasEnemy;
		}
		const int size = static_cast<int>(mgr.getEntries().size());
		return size >= _enemyCount;
	}
};

}

// #include "conditions/Not.h"
/**
 * @file
 * @ingroup Condition
 */


// #include "conditions/ICondition.h"


namespace ai {

/**
 * @brief This condition will just swap the result of the contained condition
 */
class Not: public ICondition {
protected:
	ConditionPtr _condition;

	void getConditionNameWithValue(std::stringstream& s, const AIPtr& entity) override {
		s << "(" << _condition->getNameWithConditions(entity) << ")";
	}

public:
	CONDITION_FACTORY_NO_IMPL(Not)

	explicit Not(const ConditionPtr& condition) :
			ICondition("Not", ""), _condition(condition) {
	}
	virtual ~Not() {
	}

	bool evaluate(const AIPtr& entity) override {
		return !_condition->evaluate(entity);
	}
};

inline ConditionPtr Not::Factory::create(const ConditionFactoryContext *ctx) const {
	if (ctx->conditions.size() != 1) {
		return ConditionPtr();
	}
	return std::make_shared<Not>(ctx->conditions.front());
}

}

// #include "conditions/Filter.h"
/**
 * @file
 * @ingroup Condition
 * @ingroup Filter
 */


// #include "conditions/ICondition.h"

// #include "filter/IFilter.h"
/**
 * @file
 * @defgroup Filter
 * @{
 * In combination with the `Filter` condition `IFilter` provides a quite flexible way to provide
 * generic behaviour tree tasks. You can just create one @ai{ITask} implementation that deals with
 * e.g. attacking. The target is just picked from the selection. If you encapsulate this with a
 * condition like (lua):
 * @code
 * someNode:addNode("AttackTarget", "attack"):setCondition("Filter(SelectGroupLeader{1})")
 * @endcode
 * You would only attack the group leader of group 1 if it was found. You can provide your own
 * filters like: _SelectAllInRange_, _SelectWithAttribute_ or whatever you like to filter selections
 * and forward them to tasks.
 *
 * There are some filters that accept subfilters - like _Union_, _Intersection_, _Last_, _First_,
 * _Difference_, _Complement_ and _Random_. _Last_, _First_ and _Random_ accept one sub filter as
 * parameter, _Union_ and _Intersection_ accept at least two sub filters.
 * @code
 * someNode:addNode("AttackTarget", "attack"):setCondition("Filter(First(SelectZone))")
 * @endcode
 *
 * _Random_ also accepts a parameter for how many items should be randomly preserved:
 * @code
 * someNode:addNode("AttackTarget", "attack"):setCondition("Filter(Random{1}(SelectZone))")
 * @endcode
 */


#include <list>
#include <vector>
// #include "ICharacter.h"

// #include "AI.h"

// #include "common/MemoryAllocator.h"

// #include "common/Thread.h"


namespace ai {

/**
 * @brief Macro to simplify the condition creation. Just give the class name of the condition as parameter.
 */
#define FILTER_CLASS(FilterName) \
	explicit FilterName(const std::string& parameters = "") : \
		::ai::IFilter(#FilterName, parameters) { \
	} \
public: \
	virtual ~FilterName() { \
	}

#define FILTER_FACTORY(FilterName) \
public: \
	class Factory: public ::ai::IFilterFactory { \
	public: \
		::ai::FilterPtr create (const ::ai::FilterFactoryContext *ctx) const override { \
			return std::make_shared<FilterName>(ctx->parameters); \
		} \
	}; \
	static const Factory& getFactory() { \
		static Factory FACTORY; \
		return FACTORY; \
	}

#define FILTER_ACTION_CLASS(FilterName) \
	FilterName(const std::string& parameters, const ::ai::Filters& filters) : \
		::ai::IFilter(#FilterName, parameters), _filters(filters) { \
		ai_assert(_filters.size() > 1, #FilterName " must contain at least two sub filters"); \
	} \
protected: \
	const ::ai::Filters _filters; \
public: \
	virtual ~FilterName() { \
	}

#define FILTER_ACTION_FACTORY(FilterName) \
public: \
	class Factory: public ::ai::IFilterFactory { \
	public: \
		::ai::FilterPtr create (const ::ai::FilterFactoryContext *ctx) const override { \
			return std::make_shared<FilterName>(ctx->parameters, ctx->filters); \
		} \
	}; \
	static const Factory& getFactory() { \
		static Factory FACTORY; \
		return FACTORY; \
	}

#define FILTER_FACTORY_SINGLETON \
public: \
	class Factory: public ::ai::IFilterFactory { \
		::ai::FilterPtr create (const ::ai::FilterFactoryContext */*ctx*/) const override { \
			return get(); \
		} \
	}; \
	static const Factory& getFactory() { \
		static Factory FACTORY; \
		return FACTORY; \
	}

/**
 * @brief Macro to create a singleton conditions for very easy conditions without a state.
 */
#define FILTER_CLASS_SINGLETON(FilterName) \
private: \
FILTER_CLASS(FilterName) \
public: \
	static ::ai::FilterPtr& get() { \
		AI_THREAD_LOCAL FilterName* c = nullptr; \
		if (c == nullptr) { c = new FilterName; } \
		AI_THREAD_LOCAL ::ai::FilterPtr _instance(c); \
		return _instance; \
	} \
	FILTER_FACTORY_SINGLETON

/**
 * @brief This class is used by the @c Filter condition in order to select entities for a @c TreeNode.
 *
 * To modify the selection, the implementing classes should call @c getFilteredEntities to access
 * the storage to persist the filtering for the @c TreeNode.
 *
 * In combination with the @code Filter condition @code IFilter provides a quite flexible way to provide
 * generic behaviour tree tasks. You can e.g. just create one @code ITask implementation that deals with
 * e.g. attacking. The target is just picked from the selection. If you encapsulate this with a condition
 * like (lua):
 * @code
 * someNode:addNode("AttackTarget", "attack"):setCondition("Filter(SelectGroupLeader{1})")
 * @endcode
 * You would only attack the group leader of group 1 if it was found. You can provide your own filters like:
 * SelectAllInRange, SelectWithAttribute or whatever you like to filter selections and forward them to tasks.
 */
class IFilter : public MemObject {
protected:
	const std::string _name;
	const std::string _parameters;

	/**
	 * @note The filtered entities are kept even over several ticks. The caller should decide
	 * whether he still needs an old/previous filtered selection
	 *
	 * @see @code SelectEmpty to do the clear from within the behaviour tree
	 */
	inline FilteredEntities& getFilteredEntities(const AIPtr& ai) {
		return ai->_filteredEntities;
	}
public:
	IFilter (const std::string& name, const std::string& parameters) :
			_name(name), _parameters(parameters) {
	}

	virtual ~IFilter () {
	}

	inline const std::string& getName() const {
		return _name;
	}

	inline const std::string& getParameters() const {
		return _parameters;
	}

	virtual void filter (const AIPtr& entity) = 0;
};

}

/**
 * @}
 */


#define FILTER_NAME "Filter"

namespace ai {

/**
 * @brief The filter condition executes some selection filters (@c IFilter) and evaluates to @c true if
 * the resulting set of all filter executions is non-empty. Use @c AI::getFilteredEntities to access the
 * result set and work with it in a TreeNode that is executed when this condition evaluated to true
 */
class Filter: public ICondition {
protected:
	Filters _filters;

	void getConditionNameWithValue(std::stringstream& s, const AIPtr& entity) override {
		bool first = true;
		s << "(";
		auto copy = entity->_filteredEntities;
		for (const FilterPtr& filter : _filters) {
			if (!first)
				s << ",";
			s << filter->getName() << "{" << filter->getParameters() << "}[";
			entity->_filteredEntities.clear();
			filter->filter(entity);
			bool firstChr = true;
			int cnt = 0;
			for (CharacterId id : entity->_filteredEntities) {
				if (!firstChr)
					s << ",";
				s << id;
				firstChr = false;
				++cnt;
				if (cnt > 15) {
					s << ",...";
					break;
				}
			}
			s << "]";
			first = false;
		}
		entity->_filteredEntities = copy;
		s << ")";
	}

public:
	CONDITION_FACTORY_NO_IMPL(Filter)

	explicit Filter (const Filters& filters) :
			ICondition(FILTER_NAME, ""), _filters(filters) {
	}

	/**
	 * @brief Executes the attached filters and wiped the last filter results for the given @c AI entity
	 *
	 * @return @c true if the attached filters lead to a non-empty set, @c false otherwise
	 */
	bool evaluate(const AIPtr& entity) override {
		entity->_filteredEntities.clear();
		for (const FilterPtr &filter : _filters) {
			filter->filter(entity);
		}
		return !entity->_filteredEntities.empty();
	}
};

inline ConditionPtr Filter::Factory::create(const ConditionFactoryContext *ctx) const {
	return std::make_shared<Filter>(ctx->filters);
}

}

// #include "conditions/Or.h"
/**
 * @file
 * @ingroup Condition
 */


// #include "conditions/ICondition.h"


namespace ai {

/**
 * @brief This condition will logically or all contained conditions
 */
class Or: public ICondition {
protected:
	Conditions _conditions;
	CONDITION_PRINT_SUBCONDITIONS_GETCONDITIONNAMEWITHVALUE

public:
	explicit Or(const Conditions& conditions) :
			ICondition("Or", ""), _conditions(conditions) {
	}
	virtual ~Or() {
	}

	CONDITION_FACTORY_NO_IMPL(Or)

	bool evaluate(const AIPtr& entity) override {
		for (ConditionsIter i = _conditions.begin(); i != _conditions.end(); ++i) {
			if ((*i)->evaluate(entity)) {
				return true;
			}
		}

		return false;
	}
};

inline ConditionPtr Or::Factory::create(const ConditionFactoryContext *ctx) const {
	if (ctx->conditions.size() < 2) {
		return ConditionPtr();
	}
	return std::make_shared<Or>(ctx->conditions);
}

}

// #include "conditions/True.h"

// #include "conditions/IsInGroup.h"
/**
 * @file
 * @ingroup Condition
 */


// #include "ICondition.h"

// #include "common/String.h"

// #include "group/GroupMgr.h"

// #include "zone/Zone.h"


namespace ai {

/**
 * @brief Checks whether the @c AI is in any or in a particular group
 *
 * If a group id is specified in the parameters, this condition only evaluates to
 * @c true if the @c AI is part of that particular group. If no parameter is
 * specified, it will evaluate to @c true if the @c AI is in any group (even if
 * the group does not contains any other member).
 */
class IsInGroup: public ICondition {
private:
	GroupId _groupId;

public:
	CONDITION_FACTORY(IsInGroup)

	explicit IsInGroup(const std::string& parameters) :
		ICondition("IsInGroup", parameters) {
		if (_parameters.empty()) {
			_groupId = -1;
		} else {
			_groupId = std::stoi(_parameters);
		}
	}

	virtual ~IsInGroup() {
	}

	bool evaluate(const AIPtr& entity) override {
		const GroupMgr& mgr = entity->getZone()->getGroupMgr();
		if (_groupId == -1)
			return mgr.isInAnyGroup(entity);
		return mgr.isInGroup(_groupId, entity);
	}
};

}

// #include "conditions/IsGroupLeader.h"
/**
 * @file
 * @ingroup Condition
 */


// #include "ICondition.h"

// #include "common/String.h"

// #include "group/GroupMgr.h"

// #include "zone/Zone.h"


namespace ai {

/**
 * @brief Evaluates to true if you are the first member in a particular group
 *
 * The parameter that is expected is the group id
 */
class IsGroupLeader: public ICondition {
private:
	GroupId _groupId;
public:
	CONDITION_FACTORY(IsGroupLeader)

	explicit IsGroupLeader(const std::string& parameters) :
		ICondition("IsGroupLeader", parameters) {
		if (_parameters.empty()) {
			_groupId = -1;
		} else {
			_groupId = std::stoi(_parameters);
		}
	}

	virtual ~IsGroupLeader() {
	}

	bool evaluate(const AIPtr& entity) override {
		if (_groupId == -1) {
			return false;
		}
		const GroupMgr& mgr = entity->getZone()->getGroupMgr();
		return mgr.isGroupLeader(_groupId, entity);
	}
};

}

// #include "conditions/IsCloseToGroup.h"
/**
 * @file
 * @ingroup Condition
 */


// #include "ICondition.h"

// #include "common/String.h"

// #include "group/GroupMgr.h"

// #include "zone/Zone.h"


namespace ai {

/**
 * @brief Checks whether the controlled @c AI is close to a particular group.
 *
 * The parameters are given as the group id and the distance to the group that
 * triggers this condition to evaluate to @c true.
 */
class IsCloseToGroup: public ICondition {
private:
	GroupId _groupId;
	float _distance;
public:
	CONDITION_FACTORY(IsCloseToGroup)

	explicit IsCloseToGroup(const std::string& parameters) :
		ICondition("IsCloseToGroup", parameters) {
		std::vector<std::string> tokens;
		Str::splitString(_parameters, tokens, ",");
		if (tokens.size() != 2) {
			_groupId = -1;
			_distance = -1.0f;
		} else {
			_groupId = std::stoi(tokens[0]);
			_distance = std::stof(tokens[1]);
		}
	}

	virtual ~IsCloseToGroup() {
	}

	bool evaluate(const AIPtr& entity) override {
		if (_groupId == -1) {
			return false;
		}

		if (_distance < 0.0f) {
			return false;
		}

		const GroupMgr& mgr = entity->getZone()->getGroupMgr();
		const glm::vec3& pos = mgr.getPosition(_groupId);
		if (isInfinite(pos)) {
			return false;
		}
		return glm::distance(pos, entity->getCharacter()->getPosition()) <= _distance;
	}
};

}

// #include "filter/IFilter.h"

// #include "filter/SelectEmpty.h"
/**
 * @file
 * @ingroup Filter
 */


// #include "filter/IFilter.h"


namespace ai {

/**
 * @brief This filter just clears the selection
 */
class SelectEmpty: public IFilter {
public:
	FILTER_CLASS_SINGLETON(SelectEmpty)

	void filter (const AIPtr& entity) override;
};

inline void SelectEmpty::filter (const AIPtr& entity) {
	getFilteredEntities(entity).clear();
}

}

// #include "filter/SelectHighestAggro.h"
/**
 * @file
 * @ingroup Filter
 */


// #include "filter/IFilter.h"


namespace ai {

/**
 * @brief This filter will pick the entity with the highest aggro value
 */
class SelectHighestAggro: public IFilter {
public:
	FILTER_CLASS_SINGLETON(SelectHighestAggro)

	void filter (const AIPtr& entity) override;
};

inline void SelectHighestAggro::filter (const AIPtr& entity) {
	const EntryPtr entry = entity->getAggroMgr().getHighestEntry();
	if (!entry)
		return;

	const CharacterId id = entry->getCharacterId();
	getFilteredEntities(entity).push_back(id);
}

}

// #include "filter/SelectGroupLeader.h"
/**
 * @file
 * @ingroup Filter
 */


// #include "filter/IFilter.h"

// #include "zone/Zone.h"


namespace ai {

/**
 * @brief This filter will pick the group leader of the specified group
 */
class SelectGroupLeader: public IFilter {
protected:
	GroupId _groupId;
public:
	FILTER_FACTORY(SelectGroupLeader)

	explicit SelectGroupLeader(const std::string& parameters = "") :
		IFilter("SelectGroupLeader", parameters) {
		if (_parameters.empty()) {
			_groupId = -1;
		} else {
			_groupId = std::stoi(_parameters);
		}
	}

	void filter (const AIPtr& entity) override {
		FilteredEntities& entities = getFilteredEntities(entity);
		const Zone* zone = entity->getZone();
		const GroupMgr& groupMgr = zone->getGroupMgr();
		const AIPtr& groupLeader = groupMgr.getLeader(_groupId);
		if (groupLeader) {
			entities.push_back(groupLeader->getId());
		}
	}
};

}

// #include "filter/SelectGroupMembers.h"
/**
 * @file
 * @ingroup Filter
 */


// #include "filter/IFilter.h"

// #include "zone/Zone.h"


namespace ai {

/**
 * @brief This filter will pick the entities from the groups the given @c AI instance is in
 */
class SelectGroupMembers: public IFilter {
protected:
	GroupId _groupId;
public:
	FILTER_FACTORY(SelectGroupMembers)

	explicit SelectGroupMembers(const std::string& parameters = "") :
		IFilter("SelectGroupMembers", parameters) {
		if (_parameters.empty()) {
			_groupId = -1;
		} else {
			_groupId = std::stoi(_parameters);
		}
	}

	void filter (const AIPtr& entity) override {
		FilteredEntities& entities = getFilteredEntities(entity);
		auto func = [&] (const AIPtr& ai) {
			entities.push_back(ai->getId());
			return true;
		};
		Zone* zone = entity->getZone();
		GroupMgr& groupMgr = zone->getGroupMgr();
		groupMgr.visit(_groupId, func);
	}
};

}

// #include "filter/SelectZone.h"
/**
 * @file
 * @ingroup Filter
 */


// #include "filter/IFilter.h"

// #include "zone/Zone.h"


namespace ai {

/**
 * @brief This filter will pick the entities from the zone of the given entity
 */
class SelectZone: public IFilter {
public:
	FILTER_FACTORY(SelectZone)

	explicit SelectZone(const std::string& parameters = "") :
		IFilter("SelectZone", parameters) {
	}

	void filter (const AIPtr& entity) override {
		FilteredEntities& entities = getFilteredEntities(entity);
		auto func = [&] (const AIPtr& ai) {
			entities.push_back(ai->getId());
			return true;
		};
		entity->getZone()->execute(func);
	}
};

}

// #include "filter/Union.h"
/**
 * @file
 * @ingroup Filter
 * @image html union.png
 */


// #include "filter/IFilter.h"

#include <algorithm>

namespace ai {

/**
 * @brief This filter merges several other filter results
 */
class Union: public IFilter {
public:
	FILTER_ACTION_CLASS(Union)
	FILTER_ACTION_FACTORY(Union)

	void filter (const AIPtr& entity) override;
};

inline void Union::filter (const AIPtr& entity) {
	FilteredEntities& filtered = getFilteredEntities(entity);
	// create a copy
	const FilteredEntities alreadyFiltered = filtered;
	// now clear the entity list
	filtered.clear();

	std::vector<FilteredEntities> filteredArray(_filters.size());
	int n = 0;
	size_t max = 0u;
	for (auto& f : _filters) {
		f->filter(entity);
		filteredArray[n++] = filtered;
		max = std::max(filtered.size(), max);
		// safe and clear
		filtered.clear();
	}

	for (size_t i = 0; i < filteredArray.size(); ++i) {
		std::sort(filteredArray[i].begin(), filteredArray[i].end());
	}

	FilteredEntities result(max);
	std::set_union(
			filteredArray[0].begin(), filteredArray[0].end(),
			filteredArray[1].begin(), filteredArray[1].end(),
			std::back_inserter(result));

	if (filteredArray.size() >= 2) {
		FilteredEntities buffer(max);
		for (size_t i = 2; i < filteredArray.size(); ++i) {
			buffer.clear();
			std::sort(result.begin(), result.end());
			std::set_union(
					result.begin(), result.end(),
					filteredArray[i].begin(), filteredArray[i].end(),
					std::back_inserter(buffer));
			std::swap(result, buffer);
		}
	}

	filtered.reserve(alreadyFiltered.size() + max);
	for (auto& e : alreadyFiltered) {
		filtered.push_back(e);
	}
	for (auto& e : result) {
		filtered.push_back(e);
	}
}

}

// #include "filter/Intersection.h"
/**
 * @file
 * @ingroup Filter
 * @image html intersection.png
 */


// #include "filter/IFilter.h"

#include <algorithm>

namespace ai {

/**
 * @brief This filter performs an intersection between several filter results
 */
class Intersection: public IFilter {
public:
	FILTER_ACTION_CLASS(Intersection)
	FILTER_ACTION_FACTORY(Intersection)

	void filter (const AIPtr& entity) override;
};

inline void Intersection::filter (const AIPtr& entity) {
	FilteredEntities& filtered = getFilteredEntities(entity);
	// create a copy
	const FilteredEntities alreadyFiltered = filtered;
	// now clear the entity list
	filtered.clear();

	std::vector<FilteredEntities> filteredArray(_filters.size());
	int n = 0;
	size_t max = 0u;
	for (auto& f : _filters) {
		f->filter(entity);
		filteredArray[n++] = filtered;
		max = std::max(filtered.size(), max);
		// safe and clear
		filtered.clear();
	}

	for (size_t i = 0; i < filteredArray.size(); ++i) {
		std::sort(filteredArray[i].begin(), filteredArray[i].end());
	}

	FilteredEntities result(max);
	std::set_intersection(
			filteredArray[0].begin(), filteredArray[0].end(),
			filteredArray[1].begin(), filteredArray[1].end(),
			std::back_inserter(result));

	if (filteredArray.size() >= 2) {
		FilteredEntities buffer(max);
		for (size_t i = 2; i < filteredArray.size(); ++i) {
			buffer.clear();
			std::sort(result.begin(), result.end());
			std::set_intersection(
					result.begin(), result.end(),
					filteredArray[i].begin(), filteredArray[i].end(),
					std::back_inserter(buffer));
			std::swap(result, buffer);
		}
	}

	filtered.reserve(alreadyFiltered.size() + max);
	for (auto& e : alreadyFiltered) {
		filtered.push_back(e);
	}
	for (auto& e : result) {
		filtered.push_back(e);
	}
}

}

// #include "filter/Last.h"
/**
 * @file
 * @ingroup Filter
 */


// #include "filter/IFilter.h"


namespace ai {

/**
 * @brief This filter will just preserve the last entry of other filters
 */
class Last: public IFilter {
protected:
	Filters _filters;
public:
	Last(const std::string& parameters, const Filters& filters) :
		IFilter("Last", parameters), _filters(filters) {
		ai_assert(filters.size() == 1, "Last must have one child");
	}
	FILTER_ACTION_FACTORY(Last)

	void filter (const AIPtr& entity) override;
};

inline void Last::filter (const AIPtr& entity) {
	FilteredEntities& filtered = getFilteredEntities(entity);
	const FilteredEntities copy = filtered;
	filtered.clear();
	_filters.front()->filter(entity);
	const auto& value = getFilteredEntities(entity).back();
	filtered.clear();
	for (auto& e : copy) {
		filtered.push_back(e);
	}
	filtered.push_back(value);
}

}

// #include "filter/First.h"
/**
 * @file
 * @ingroup Filter
 */


// #include "filter/IFilter.h"


namespace ai {

/**
 * @brief This filter will just preserve the first entry of other filters
 */
class First: public IFilter {
protected:
	Filters _filters;
public:
	First(const std::string& parameters, const Filters& filters) :
		IFilter("First", parameters), _filters(filters) {
		ai_assert(filters.size() == 1, "First must have one child");
	}
	FILTER_ACTION_FACTORY(First)

	void filter (const AIPtr& entity) override;
};

inline void First::filter (const AIPtr& entity) {
	FilteredEntities& filtered = getFilteredEntities(entity);
	const FilteredEntities copy = filtered;
	filtered.clear();
	_filters.front()->filter(entity);
	const auto& value = getFilteredEntities(entity).front();
	filtered.clear();
	for (auto& e : copy) {
		filtered.push_back(e);
	}
	filtered.push_back(value);
}

}

// #include "filter/Random.h"
/**
 * @file
 * @ingroup Filter
 */


// #include "filter/IFilter.h"

// #include "common/Random.h"


namespace ai {

/**
 * @brief This filter will preserve only a few random entries
 */
class Random: public IFilter {
protected:
	Filters _filters;
	int _n;
public:
	Random(const std::string& parameters, const Filters& filters) :
		IFilter("Random", parameters), _filters(filters) {
		ai_assert(filters.size() == 1, "Random must have one child");
		_n = std::stoi(parameters);
	}

	FILTER_ACTION_FACTORY(Random)

	void filter (const AIPtr& entity) override;
};

inline void Random::filter (const AIPtr& entity) {
	FilteredEntities& filtered = getFilteredEntities(entity);
	const FilteredEntities copy = filtered;
	filtered.clear();
	_filters.front()->filter(entity);
	FilteredEntities rndFiltered = getFilteredEntities(entity);
	ai::shuffle(rndFiltered.begin(), rndFiltered.end());
	rndFiltered.resize(_n);
	for (auto& e : copy) {
		filtered.push_back(e);
	}
	for (auto& e : rndFiltered) {
		filtered.push_back(e);
	}
}

}

// #include "filter/Difference.h"
/**
 * @file
 * @ingroup Filter
 * @image html difference.png
 */


// #include "filter/IFilter.h"

#include <algorithm>

namespace ai {

/**
 * @brief This filter performs a difference operation between several filter results. The result
 * consists of elements that are in A and not in B, C, D, ...
 */
class Difference: public IFilter {
public:
	FILTER_ACTION_CLASS(Difference)
	FILTER_ACTION_FACTORY(Difference)

	void filter (const AIPtr& entity) override;
};

inline void Difference::filter (const AIPtr& entity) {
	FilteredEntities& filtered = getFilteredEntities(entity);
	// create a copy
	const FilteredEntities alreadyFiltered = filtered;
	// now clear the entity list
	filtered.clear();

	std::vector<FilteredEntities> filteredArray(_filters.size());
	int n = 0;
	size_t max = 0u;
	for (auto& f : _filters) {
		f->filter(entity);
		filteredArray[n++] = filtered;
		max = std::max(filtered.size(), max);
		// safe and clear
		filtered.clear();
	}

	for (size_t i = 0; i < filteredArray.size(); ++i) {
		std::sort(filteredArray[i].begin(), filteredArray[i].end());
	}

	FilteredEntities result(max);
	std::set_difference(
			filteredArray[0].begin(), filteredArray[0].end(),
			filteredArray[1].begin(), filteredArray[1].end(),
			std::back_inserter(result));

	if (filteredArray.size() >= 2) {
		FilteredEntities buffer(max);
		for (size_t i = 2; i < filteredArray.size(); ++i) {
			buffer.clear();
			std::sort(result.begin(), result.end());
			std::set_difference(
					result.begin(), result.end(),
					filteredArray[i].begin(), filteredArray[i].end(),
					std::back_inserter(buffer));
			std::swap(result, buffer);
		}
	}

	filtered.reserve(alreadyFiltered.size() + max);
	for (auto& e : alreadyFiltered) {
		filtered.push_back(e);
	}
	for (auto& e : result) {
		filtered.push_back(e);
	}
}

}

// #include "filter/Complement.h"
/**
 * @file
 * @ingroup Filter
 * @image html complement.png
 */


// #include "filter/IFilter.h"

#include <algorithm>

namespace ai {

/**
 * @brief This filter performs a complement operation on already filtered entities with the results given by
 * the child filters.
 */
class Complement: public IFilter {
public:
	FILTER_ACTION_CLASS(Complement)
	FILTER_ACTION_FACTORY(Complement)

	void filter (const AIPtr& entity) override;
};

inline void Complement::filter (const AIPtr& entity) {
	FilteredEntities& filtered = getFilteredEntities(entity);
	// create a copy
	const FilteredEntities alreadyFiltered = filtered;
	// now clear the entity list
	filtered.clear();

	std::vector<FilteredEntities> filteredArray(_filters.size());
	int n = 0;
	size_t max = 0u;
	for (auto& f : _filters) {
		f->filter(entity);
		filteredArray[n++] = filtered;
		max = std::max(filtered.size(), max);
		// safe and clear
		filtered.clear();
	}

	for (size_t i = 0; i < filteredArray.size(); ++i) {
		std::sort(filteredArray[i].begin(), filteredArray[i].end());
	}

	FilteredEntities result(max);
	std::set_difference(
			filteredArray[0].begin(), filteredArray[0].end(),
			filteredArray[1].begin(), filteredArray[1].end(),
			std::back_inserter(result));

	if (filteredArray.size() >= 2) {
		FilteredEntities buffer(max);
		for (size_t i = 2; i < filteredArray.size(); ++i) {
			buffer.clear();
			std::sort(result.begin(), result.end());
			std::set_difference(
					result.begin(), result.end(),
					filteredArray[i].begin(), filteredArray[i].end(),
					std::back_inserter(buffer));
			std::swap(result, buffer);
		}
	}

	filtered.reserve(alreadyFiltered.size() + max);
	for (auto& e : alreadyFiltered) {
		filtered.push_back(e);
	}
	for (auto& e : result) {
		filtered.push_back(e);
	}
}

}

// #include "filter/SelectAll.h"
/**
 * @file
 * @ingroup Filter
 */


// #include "filter/IFilter.h"


namespace ai {

/**
 * @brief This filter is a nop - it will just use the already filtered entities.
 */
class SelectAll: public IFilter {
public:
	FILTER_CLASS_SINGLETON(SelectAll)

	void filter (const AIPtr& entity) override;
};

inline void SelectAll::filter (const AIPtr& /*entity*/) {
}

}

// #include "movement/SelectionSeek.h"
/**
 * @file
 */


// #include "Steering.h"


namespace ai {
namespace movement {

/**
 * @brief Seeks the current @c IFilter selection from the given @c ICharacter
 */
class SelectionSeek: public SelectionSteering {
public:
	STEERING_FACTORY(SelectionSeek)

	explicit SelectionSeek(const std::string&) :
			SelectionSteering() {
	}

	virtual MoveVector execute (const AIPtr& ai, float speed) const override {
		const glm::vec3& target = getSelectionTarget(ai, 0);
		if (isInfinite(target)) {
			const MoveVector d(target, 0.0);
		}
		const glm::vec3& v = glm::normalize(target - ai->getCharacter()->getPosition());
		const float orientation = angle(v);
		const MoveVector d(v * speed, orientation);
		return d;
	}
};

}
}

// #include "movement/SelectionFlee.h"
/**
 * @file
 */


// #include "Steering.h"


namespace ai {
namespace movement {

/**
 * @brief Flees the current @c IFilter selection from the given @c ICharacter
 */
class SelectionFlee: public SelectionSteering {
public:
	STEERING_FACTORY(SelectionFlee)

	explicit SelectionFlee(const std::string&) :
			SelectionSteering() {
	}

	virtual MoveVector execute (const AIPtr& ai, float speed) const override {
		const glm::vec3& target = getSelectionTarget(ai, 0);
		if (isInfinite(target)) {
			const MoveVector d(target, 0.0);
		}
		const glm::vec3& v = glm::normalize(ai->getCharacter()->getPosition() - target);
		const float orientation = angle(v);
		const MoveVector d(v * speed, orientation);
		return d;
	}
};

}
}

// #include "movement/GroupFlee.h"
/**
 * @file
 */


// #include "Steering.h"


namespace ai {
namespace movement {

/**
 * @brief Flees from a particular group
 */
class GroupFlee: public ISteering {
protected:
	GroupId _groupId;
public:
	STEERING_FACTORY(GroupFlee)

	explicit GroupFlee(const std::string& parameters) :
			ISteering() {
		_groupId = ::atoi(parameters.c_str());
	}

	inline bool isValid () const {
		return _groupId != -1;
	}

	virtual MoveVector execute (const AIPtr& ai, float speed) const override {
		const Zone* zone = ai->getZone();
		if (zone == nullptr) {
			return MoveVector(VEC3_INFINITE, 0.0f);
		}
		const glm::vec3& target = zone->getGroupMgr().getPosition(_groupId);
		if (isInfinite(target)) {
			return MoveVector(target, 0.0f);
		}
		const glm::vec3& v = glm::normalize(ai->getCharacter()->getPosition() - target);
		const float orientation = angle(v);
		const MoveVector d(v * speed, orientation);
		return d;
	}
};

}
}

// #include "movement/GroupSeek.h"
/**
 * @file
 */


// #include "Steering.h"


namespace ai {
namespace movement {

/**
 * @brief Seeks a particular group
 */
class GroupSeek: public ISteering {
protected:
	GroupId _groupId;
public:
	STEERING_FACTORY(GroupSeek)

	explicit GroupSeek(const std::string& parameters) :
			ISteering() {
		_groupId = ::atoi(parameters.c_str());
	}

	inline bool isValid () const {
		return _groupId != -1;
	}

	virtual MoveVector execute (const AIPtr& ai, float speed) const override {
		const Zone* zone = ai->getZone();
		if (zone == nullptr) {
			return MoveVector(VEC3_INFINITE, 0.0f);
		}
		const glm::vec3& target = zone->getGroupMgr().getPosition(_groupId);
		if (isInfinite(target)) {
			return MoveVector(target, 0.0f);
		}
		const glm::vec3& v = glm::normalize(target - ai->getCharacter()->getPosition());
		const float orientation = angle(v);
		const MoveVector d(v * speed, orientation);
		return d;
	}
};

}
}

// #include "movement/Steering.h"

// #include "movement/TargetFlee.h"
/**
 * @file
 */


// #include "Steering.h"


namespace ai {
namespace movement {

/**
 * @brief Flees from a particular target
 */
class TargetFlee: public ISteering {
protected:
	glm::vec3 _target;
public:
	STEERING_FACTORY(TargetFlee)

	explicit TargetFlee(const std::string& parameters) :
			ISteering() {
		_target = parse(parameters);
	}

	inline bool isValid () const {
		return !isInfinite(_target);
	}

	virtual MoveVector execute (const AIPtr& ai, float speed) const override {
		if (!isValid()) {
			return MoveVector(_target, 0.0f);
		}
		const glm::vec3& v = glm::normalize(ai->getCharacter()->getPosition() - _target);
		const float orientation = angle(v);
		const MoveVector d(v * speed, orientation);
		return d;
	}
};


}
}

// #include "movement/TargetSeek.h"
/**
 * @file
 */


// #include "Steering.h"


namespace ai {
namespace movement {

/**
 * @brief Seeks a particular target
 */
class TargetSeek: public ISteering {
protected:
	glm::vec3 _target;
public:
	STEERING_FACTORY(TargetSeek)

	explicit TargetSeek(const std::string& parameters) :
			ISteering() {
		_target = parse(parameters);
	}

	inline bool isValid () const {
		return !isInfinite(_target);
	}

	virtual MoveVector execute (const AIPtr& ai, float speed) const override {
		if (!isValid()) {
			return MoveVector(_target, 0.0f);
		}
		const glm::vec3& v = glm::normalize(_target - ai->getCharacter()->getPosition());
		const float orientation = angle(v);
		const MoveVector d(v * speed, orientation);
		return d;
	}
};

}
}

// #include "movement/Wander.h"
/**
 * @file
 */


// #include "Steering.h"

// #include "common/Random.h"


namespace ai {
namespace movement {

/**
 * @brief Moves forward in the direction the character is currently facing into.
 *
 * Changes orientation (resp. rotation) in a range of [-rotation,rotation] where more
 * weight is given to keep the current orientation.
 */
class Wander: public ISteering {
protected:
	const float _rotation;
public:
	STEERING_FACTORY(Wander)

	explicit Wander(const std::string& parameter) :
			ISteering(), _rotation(parameter.empty() ? ai::toRadians(10.0f) : Str::strToFloat(parameter)) {
	}

	MoveVector execute (const AIPtr& ai, float speed) const override {
		const float orientation = ai->getCharacter()->getOrientation();
		const glm::vec3& v = fromRadians(orientation);
		const MoveVector d(v * speed, ai::randomBinomial() * _rotation);
		return d;
	}
};

}
}

// #include "movement/WeightedSteering.h"


namespace ai {

#define R_GET(Name) registerFactory(#Name, Name::getFactory());
#define R_MOVE(Name) registerFactory(#Name, movement::Name::getFactory());

/**
 * @brief The place to register your @ai{TreeNode} and @ai{ICondition} factories at
 */
class AIRegistry: public IAIFactory {
protected:
	class TreeNodeFactory: public IFactoryRegistry<std::string, TreeNode, TreeNodeFactoryContext> {
	public:
		TreeNodeFactory() {
			R_GET(Fail);
			R_GET(Limit);
			R_GET(Invert);
			R_GET(Succeed);
			R_GET(Parallel);
			R_GET(PrioritySelector);
			R_GET(ProbabilitySelector);
			R_GET(RandomSelector);
			R_GET(Sequence);
			R_GET(Idle);
		}
	};

	TreeNodeFactory _treeNodeFactory;

	class SteerNodeFactory: public IFactoryRegistry<std::string, TreeNode, SteerNodeFactoryContext> {
	public:
		SteerNodeFactory() {
			R_GET(Steer);
		}
	};

	SteerNodeFactory _steerNodeFactory;

	class SteeringFactory: public IFactoryRegistry<std::string, movement::ISteering, SteeringFactoryContext> {
	public:
		SteeringFactory() {
			R_MOVE(Wander);
			R_MOVE(GroupSeek);
			R_MOVE(GroupFlee);
			R_MOVE(TargetSeek);
			R_MOVE(TargetFlee);
			R_MOVE(SelectionSeek);
			R_MOVE(SelectionFlee);
		}
	};

	SteeringFactory _steeringFactory;

	class FilterFactory: public IFactoryRegistry<std::string, IFilter, FilterFactoryContext> {
	public:
		FilterFactory() {
			R_GET(SelectEmpty);
			R_GET(SelectGroupLeader);
			R_GET(SelectGroupMembers);
			R_GET(SelectHighestAggro);
			R_GET(SelectZone);
			R_GET(Union);
			R_GET(Intersection);
			R_GET(Last);
			R_GET(First);
			R_GET(Random);
			R_GET(Difference);
			R_GET(Complement);
			R_GET(SelectAll);
		}
	};

	FilterFactory _filterFactory;

	class ConditionFactory: public IFactoryRegistry<std::string, ICondition, ConditionFactoryContext> {
	public:
		ConditionFactory() {
			R_GET(And);
			R_GET(False);
			R_GET(HasEnemies);
			R_GET(Not);
			R_GET(Or);
			R_GET(True);
			R_GET(Filter);
			R_GET(IsGroupLeader);
			R_GET(IsInGroup);
			R_GET(IsCloseToGroup);
		}
	};

	ConditionFactory _conditionFactory;
public:
	AIRegistry() :
			IAIFactory() {
	}
	virtual ~AIRegistry() {}

	/**
	 * @brief Registers a tree node factory of the given @c type.
	 * @param[in] type The name that is used in the behaviour tree to identify nodes of the
	 * that are assigned to the given factory
	 * @param[in] factory The factory that will create the real node.
	 * @return @c true if the unregister action was successful, @c false if not (e.g. it wasn't registered at all)
	 */
	bool registerNodeFactory(const std::string& type, const ITreeNodeFactory& factory);
	/**
	 * @brief Unregisters a tree node factory of the given @c type. This can also be used to replace a built-in
	 * type with a user provided type.
	 *
	 * @return @c true if the unregister action was successful, @c false if not (e.g. it wasn't registered at all)
	 */
	bool unregisterNodeFactory(const std::string& type);

	bool registerSteerNodeFactory(const std::string& type, const ISteerNodeFactory& factory);
	/**
	 * @brief Unregisters a tree node factory of the given @c type. This can also be used to replace a built-in
	 * type with a user provided type.
	 *
	 * @return @c true if the unregister action was successful, @c false if not (e.g. it wasn't registered at all)
	 */
	bool unregisterSteerNodeFactory(const std::string& type);

	bool registerSteeringFactory(const std::string& type, const ISteeringFactory& factory);
	bool unregisterSteeringFactory(const std::string& type);

	bool registerFilterFactory(const std::string& type, const IFilterFactory& factory);

	/**
	 * @brief Unregisters a filter factory of the given @c type. This can also be used to replace a built-in
	 * type with a user provided type.
	 *
	 * @return @c true if the unregister action was successful, @c false if not (e.g. it wasn't registered at all)
	 */
	bool unregisterFilterFactory(const std::string& type);

	bool registerConditionFactory(const std::string& type, const IConditionFactory& factory);
	/**
	 * @brief Unregisters a condition factory of the given @c type. This can also be used to replace a built-in
	 * type with a user provided type.
	 *
	 * @return @c true if the unregister action was successful, @c false if not (e.g. it wasn't registered at all)
	 */
	bool unregisterConditionFactory(const std::string& type);

	TreeNodePtr createNode(const std::string& type, const TreeNodeFactoryContext& ctx) const override;
	TreeNodePtr createSteerNode(const std::string& type, const SteerNodeFactoryContext& ctx) const override;
	FilterPtr createFilter(const std::string& type, const FilterFactoryContext& ctx) const override;
	ConditionPtr createCondition(const std::string& type, const ConditionFactoryContext& ctx) const override;
	SteeringPtr createSteering(const std::string& type, const SteeringFactoryContext& ctx) const override;
};

#undef R_GET
#undef R_MOVE

inline TreeNodePtr AIRegistry::createNode(const std::string& nodeType, const TreeNodeFactoryContext& ctx) const {
	return _treeNodeFactory.create(nodeType, &ctx);
}

inline TreeNodePtr AIRegistry::createSteerNode(const std::string& nodeType, const SteerNodeFactoryContext& ctx) const {
	return _steerNodeFactory.create(nodeType, &ctx);
}

inline FilterPtr AIRegistry::createFilter(const std::string& nodeType, const FilterFactoryContext& ctx) const {
	return _filterFactory.create(nodeType, &ctx);
}

inline bool AIRegistry::registerNodeFactory(const std::string& nodeType, const ITreeNodeFactory& factory) {
	return _treeNodeFactory.registerFactory(nodeType, factory);
}

inline bool AIRegistry::registerFilterFactory(const std::string& nodeType, const IFilterFactory& factory) {
	return _filterFactory.registerFactory(nodeType, factory);
}

inline bool AIRegistry::registerConditionFactory(const std::string& nodeType, const IConditionFactory& factory) {
	return _conditionFactory.registerFactory(nodeType, factory);
}

inline bool AIRegistry::unregisterNodeFactory(const std::string& nodeType) {
	return _treeNodeFactory.unregisterFactory(nodeType);
}

inline bool AIRegistry::registerSteerNodeFactory(const std::string& type, const ISteerNodeFactory& factory) {
	return _steerNodeFactory.registerFactory(type, factory);
}

inline bool AIRegistry::unregisterSteerNodeFactory(const std::string& type) {
	return _steerNodeFactory.unregisterFactory(type);
}

inline bool AIRegistry::registerSteeringFactory(const std::string& type, const ISteeringFactory& factory) {
	return _steeringFactory.registerFactory(type, factory);
}

inline bool AIRegistry::unregisterSteeringFactory(const std::string& type) {
	return _steeringFactory.unregisterFactory(type);
}

inline bool AIRegistry::unregisterFilterFactory(const std::string& nodeType) {
	return _filterFactory.unregisterFactory(nodeType);
}

inline bool AIRegistry::unregisterConditionFactory(const std::string& nodeType) {
	return _conditionFactory.unregisterFactory(nodeType);
}

inline ConditionPtr AIRegistry::createCondition(const std::string& nodeType, const ConditionFactoryContext& ctx) const {
	return _conditionFactory.create(nodeType, &ctx);
}

inline SteeringPtr AIRegistry::createSteering(const std::string& type, const SteeringFactoryContext& ctx) const {
	return _steeringFactory.create(type, &ctx);
}

}

// #include "ICharacter.h"


// #include "tree/Fail.h"

// #include "tree/Limit.h"

// #include "tree/Idle.h"

// #include "tree/Invert.h"

// #include "tree/Parallel.h"

// #include "tree/PrioritySelector.h"

// #include "tree/Selector.h"

// #include "tree/Sequence.h"

// #include "tree/Steer.h"

// #include "tree/TreeNode.h"

// #include "tree/TreeNodeImpl.h"
/**
 * @file
 */


// #include "TreeNode.h"

// #include "AI.h"


namespace ai {

inline int TreeNode::getId() const {
	return _id;
}

inline void TreeNode::setName(const std::string& name) {
	if (name.empty()) {
		return;
	}
	_name = name;
}

inline const std::string& TreeNode::getType() const {
	return _type;
}

inline const std::string& TreeNode::getName() const {
	return _name;
}

inline const ConditionPtr& TreeNode::getCondition() const {
	return _condition;
}

inline void TreeNode::setCondition(const ConditionPtr& condition) {
	_condition = condition;
}

inline const std::string& TreeNode::getParameters() const {
	return _parameters;
}

inline const TreeNodes& TreeNode::getChildren() const {
	return _children;
}

inline TreeNodes& TreeNode::getChildren() {
	return _children;
}

inline bool TreeNode::addChild(const TreeNodePtr& child) {
	_children.push_back(child);
	return true;
}

inline void TreeNode::resetState(const AIPtr& entity) {
	for (auto& c : _children) {
		c->resetState(entity);
	}
}

inline void TreeNode::getRunningChildren(const AIPtr& /*entity*/, std::vector<bool>& active) const {
	const int size = (int)_children.size();
	for (int i = 0; i < size; ++i) {
		active.push_back(false);
	}
}

inline void TreeNode::setLastExecMillis(const AIPtr& entity) {
	if (!entity->_debuggingActive) {
		return;
	}
	entity->_lastExecMillis[getId()] = entity->_time;
}

inline int TreeNode::getSelectorState(const AIPtr& entity) const {
	AI::SelectorStates::const_iterator i = entity->_selectorStates.find(getId());
	if (i == entity->_selectorStates.end()) {
		return AI_NOTHING_SELECTED;
	}
	return i->second;
}

inline void TreeNode::setSelectorState(const AIPtr& entity, int selected) {
	entity->_selectorStates[getId()] = selected;
}

inline int TreeNode::getLimitState(const AIPtr& entity) const {
	AI::LimitStates::const_iterator i = entity->_limitStates.find(getId());
	if (i == entity->_limitStates.end()) {
		return 0;
	}
	return i->second;
}

inline void TreeNode::setLimitState(const AIPtr& entity, int amount) {
	entity->_limitStates[getId()] = amount;
}

inline TreeNodeStatus TreeNode::state(const AIPtr& entity, TreeNodeStatus treeNodeState) {
	if (!entity->_debuggingActive) {
		return treeNodeState;
	}
	entity->_lastStatus[getId()] = treeNodeState;
	return treeNodeState;
}

inline int64_t TreeNode::getLastExecMillis(const AIPtr& entity) const {
	if (!entity->_debuggingActive) {
		return -1L;
	}
	AI::LastExecMap::const_iterator i = entity->_lastExecMillis.find(getId());
	if (i == entity->_lastExecMillis.end()) {
		return -1L;
	}
	return i->second;
}

inline TreeNodeStatus TreeNode::getLastStatus(const AIPtr& entity) const {
	if (!entity->_debuggingActive) {
		return UNKNOWN;
	}
	AI::NodeStates::const_iterator i = entity->_lastStatus.find(getId());
	if (i == entity->_lastStatus.end()) {
		return UNKNOWN;
	}
	return i->second;
}

inline TreeNodePtr TreeNode::getChild(int id) const {
	for (auto& child : _children) {
		if (child->getId() == id) {
			return child;
		}
		const TreeNodePtr& node = child->getChild(id);
		if (node) {
			return node;
		}
	}
	return TreeNodePtr();
}

inline bool TreeNode::replaceChild(int id, const TreeNodePtr& newNode) {
	auto i = std::find_if(_children.begin(), _children.end(), [id] (const TreeNodePtr& other) { return other->getId() == id; });
	if (i == _children.end()) {
		return false;
	}

	if (newNode) {
		*i = newNode;
		return true;
	}

	_children.erase(i);
	return true;
}

inline TreeNodePtr TreeNode::getParent_r(const TreeNodePtr& parent, int id) const {
	for (auto& child : _children) {
		if (child->getId() == id) {
			return parent;
		}
		const TreeNodePtr& parentPtr = child->getParent_r(child, id);
		if (parentPtr) {
			return parentPtr;
		}
	}
	return TreeNodePtr();
}

inline TreeNodePtr TreeNode::getParent(const TreeNodePtr& self, int id) const {
	ai_assert(getId() != id, "Root nodes don't have a parent");
	for (auto& child : _children) {
		if (child->getId() == id) {
			return self;
		}
		const TreeNodePtr& parent = child->getParent_r(child, id);
		if (parent) {
			return parent;
		}
	}
	return TreeNodePtr();
}

inline TreeNodeStatus TreeNode::execute(const AIPtr& entity, int64_t /*deltaMillis*/) {
	if (!_condition->evaluate(entity)) {
		return state(entity, CANNOTEXECUTE);
	}

	setLastExecMillis(entity);
	return state(entity, FINISHED);
}

}

// #include "tree/ITask.h"

// #include "tree/ITimedNode.h"

// #include "tree/TreeNodeParser.h"
/**
 * @file
 */


// #include "tree/TreeNode.h"

// #include "common/IParser.h"
/**
 * @file
 */


// #include "Log.h"

#include <vector>
#include <string>
#include <algorithm>
// #include "String.h"


namespace ai {

class IParser {
private:
	std::string _error;

protected:
	void setError(const char* msg, ...) __attribute__((format(printf, 2, 3)));

	inline void resetError() {
		_error = "";
	}

	inline std::string getBetween (const std::string& str, const std::string& tokenStart, const std::string& tokenEnd) {
		const std::size_t start = str.find(tokenStart);
		if (start == std::string::npos) {
			return "";
		}

		const std::size_t end = str.find(tokenEnd);
		if (end == std::string::npos) {
			setError("syntax error - expected %s", tokenEnd.c_str());
			return "";
		}
		const size_t startIndex = start + 1;
		const size_t endIndex = end - startIndex;
		if (endIndex <= 0) {
			return "";
		}
		const std::string& between = str.substr(startIndex, endIndex);
		return between;
	}

public:
	const std::string& getError() const;
};

inline void IParser::setError(const char* msg, ...) {
	va_list args;
	va_start(args, msg);
	char buf[1024];
	std::vsnprintf(buf, sizeof(buf), msg, args);
	va_end(args);
	if (buf[0] != '\0') {
		ai_log_debug("%s", buf);
	}
	_error = buf;
}


inline const std::string& IParser::getError() const {
	return _error;
}

}

// #include "Steer.h"


namespace ai {

class IAIFactory;

/**
 * @brief Transforms the string representation of a @c TreeNode with all its
 * parameters into a @c TreeNode instance.
 *
 * @c #NodeName{Parameters}
 * Parameters are by default optional - but that really depends on the
 * @c TreeNode implementation.
 */
class TreeNodeParser: public IParser {
private:
	const IAIFactory& _aiFactory;
	std::string _taskString;

	void splitTasks(const std::string& string, std::vector<std::string>& tokens) const;
	SteeringPtr getSteering(const std::string& nodeName);
public:
	TreeNodeParser(const IAIFactory& aiFactory, const std::string& taskString) :
			IParser(), _aiFactory(aiFactory) {
		_taskString = ai::Str::eraseAllSpaces(taskString);
	}

	virtual ~TreeNodeParser() {}

	TreeNodePtr getTreeNode(const std::string& name = "");
};

inline void TreeNodeParser::splitTasks(const std::string& string, std::vector<std::string>& tokens) const {
	bool inParameter = false;
	bool inChildren = false;
	std::string token;
	for (std::string::const_iterator i = string.begin(); i != string.end(); ++i) {
		if (*i == '{') {
			inParameter = true;
		} else if (*i == '}') {
			inParameter = false;
		} else if (*i == '(') {
			inChildren = true;
		} else if (*i == ')') {
			inChildren = false;
		}

		if (!inParameter && !inChildren) {
			if (*i == ',') {
				tokens.push_back(token);
				token.clear();
				continue;
			}
		}
		token.push_back(*i);
	}
	tokens.push_back(token);
}

inline SteeringPtr TreeNodeParser::getSteering (const std::string& nodeName) {
	std::string steerType;
	const std::string& parameters = getBetween(nodeName, "{", "}");
	std::size_t n = nodeName.find("{");
	if (n == std::string::npos)
		n = nodeName.find("(");
	if (n != std::string::npos) {
		steerType = nodeName.substr(0, n);
	} else {
		steerType = nodeName;
	}

	const SteeringFactoryContext ctx(parameters);
	return _aiFactory.createSteering(steerType, ctx);
}

inline TreeNodePtr TreeNodeParser::getTreeNode(const std::string& name) {
	resetError();
	std::string nodeType;
	std::string parameters;
	std::size_t n = _taskString.find("(");
	if (n == std::string::npos || _taskString.find("{") < n) {
		parameters = getBetween(_taskString, "{", "}");
		n = _taskString.find("{");
	}
	if (n != std::string::npos) {
		nodeType = _taskString.substr(0, n);
	} else {
		nodeType = _taskString;
	}
	const std::string& subTrees = getBetween(_taskString, "(", ")");
	if (!subTrees.empty()) {
		if (nodeType != "Steer") {
			return TreeNodePtr();
		}
		std::vector<std::string> tokens;
		splitTasks(subTrees, tokens);
		movement::Steerings steerings;
		for (const std::string& nodeName : tokens) {
			const SteeringPtr& steering = getSteering(nodeName);
			if (!steering) {
				return TreeNodePtr();
			}
			steerings.push_back(steering);
		}
		const SteerNodeFactoryContext steerFactoryCtx(name.empty() ? nodeType : name, parameters, True::get(), steerings);
		return _aiFactory.createSteerNode(nodeType, steerFactoryCtx);
	}

	const TreeNodeFactoryContext factoryCtx(name.empty() ? nodeType : name, parameters, True::get());
	return _aiFactory.createNode(nodeType, factoryCtx);
}

}

// #include "tree/loaders/ITreeLoader.h"


// #include "group/GroupId.h"

// #include "group/GroupMgr.h"


// #include "movement/SelectionSeek.h"

// #include "movement/GroupFlee.h"

// #include "movement/GroupSeek.h"

// #include "movement/Steering.h"

// #include "movement/TargetFlee.h"

// #include "movement/TargetSeek.h"

// #include "movement/Wander.h"

// #include "movement/WeightedSteering.h"


// #include "server/Network.h"
/**
 * @file
 */


// #include "IProtocolHandler.h"
/**
 * @file
 */


#include <memory>
// #include "IProtocolMessage.h"
/**
 * @file
 */


#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <string>
#include <deque>
#define AI_LIL_ENDIAN  1234
#define AI_BIG_ENDIAN  4321
#ifdef __linux__
#include <endian.h>
#define AI_BYTEORDER  __BYTE_ORDER
#else
#define AI_BYTEORDER   AI_LIL_ENDIAN
#endif

#if AI_BYTEORDER == AI_LIL_ENDIAN
#define AI_SwapLE16(X) (X)
#define AI_SwapLE32(X) (X)
#define AI_SwapLE64(X) (X)
#define AI_SwapBE16(X) AI_Swap16(X)
#define AI_SwapBE32(X) AI_Swap32(X)
#define AI_SwapBE64(X) AI_Swap64(X)
#else
#define AI_SwapLE16(X) AI_Swap16(X)
#define AI_SwapLE32(X) AI_Swap32(X)
#define AI_SwapLE64(X) AI_Swap64(X)
#define AI_SwapBE16(X) (X)
#define AI_SwapBE32(X) (X)
#define AI_SwapBE64(X) (X)
#endif

namespace ai {

typedef uint8_t ProtocolId;
typedef std::deque<uint8_t> streamContainer;

const ProtocolId PROTO_PING = 0;
const ProtocolId PROTO_STATE = 1;
const ProtocolId PROTO_CHARACTER_STATIC = 2;
const ProtocolId PROTO_CHARACTER_DETAILS = 3;
const ProtocolId PROTO_SELECT = 4;
const ProtocolId PROTO_PAUSE = 5;
const ProtocolId PROTO_CHANGE = 6;
const ProtocolId PROTO_NAMES = 7;
const ProtocolId PROTO_RESET = 8;
const ProtocolId PROTO_STEP = 9;
const ProtocolId PROTO_UPDATENODE = 10;
const ProtocolId PROTO_DELETENODE = 11;
const ProtocolId PROTO_ADDNODE = 12;

/**
 * @brief A protocol message is used for the serialization of the ai states for remote debugging
 *
 * @note Message byte order is big endian
 */
class IProtocolMessage {
private:
#if defined(__GNUC__) && defined(__i386__)
	static inline uint16_t AI_Swap16(uint16_t x) {
		__asm__("xchgb %b0,%h0": "=q"(x):"0"(x));
		return x;
	}
#elif defined(__GNUC__) && defined(__x86_64__)
	static inline uint16_t AI_Swap16(uint16_t x) {
		__asm__("xchgb %b0,%h0": "=Q"(x):"0"(x));
		return x;
	}
#else
	static inline uint16_t AI_Swap16(uint16_t x) {
		return static_cast<uint16_t>((x << 8) | (x >> 8));
	}
#endif

#if defined(__GNUC__) && defined(__i386__)
	static inline uint32_t AI_Swap32(uint32_t x) {
		__asm__("bswap %0": "=r"(x):"0"(x));
		return x;
	}
#elif defined(__GNUC__) && defined(__x86_64__)
	static inline uint32_t AI_Swap32(uint32_t x) {
		__asm__("bswapl %0": "=r"(x):"0"(x));
		return x;
	}
#else
	static inline uint32_t AI_Swap32(uint32_t x) {
		return static_cast<uint32_t>((x << 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x >> 24));
	}
#endif

#if defined(__GNUC__) && defined(__i386__)
	static inline uint64_t AI_Swap64(uint64_t x) {
		union {
			struct {
				uint32_t a, b;
			}s;
			uint64_t u;
		}v;
		v.u = x;
		__asm__("bswapl %0 ; bswapl %1 ; xchgl %0,%1": "=r"(v.s.a), "=r"(v.s.b):"0"(v.s.a), "1"(v.s. b));
		return v.u;
	}
#elif defined(__GNUC__) && defined(__x86_64__)
	static inline uint64_t AI_Swap64(uint64_t x) {
		__asm__("bswapq %0": "=r"(x):"0"(x));
		return x;
	}
#else
	static inline uint64_t AI_Swap64(uint64_t x) {
		/* Separate into high and low 32-bit values and swap them */
		const uint32_t lo = static_cast<uint32_t>(x & 0xFFFFFFFF);
		x >>= 32;
		const uint32_t hi = static_cast<uint32_t>(x & 0xFFFFFFFF);
		x = AI_Swap32(lo);
		x <<= 32;
		x |= AI_Swap32(hi);
		return x;
	}
#endif

protected:
	const ProtocolId _id;

public:
	static void addByte(streamContainer& out, uint8_t byte);
	static void addBool(streamContainer& out, bool value);
	static void addShort(streamContainer& out, int16_t word);
	static void addInt(streamContainer& out, int32_t dword);
	static void addLong(streamContainer& out, int64_t dword);
	static void addFloat(streamContainer& out, float value);
	static void addString(streamContainer& out, const std::string& string);

	static bool readBool(streamContainer& in);
	static uint8_t readByte(streamContainer& in);
	static int16_t readShort(streamContainer& in);
	static int32_t peekInt(const streamContainer& in);
	static int32_t readInt(streamContainer& in);
	static int64_t readLong(streamContainer& in);
	static float readFloat(streamContainer& in);
	static std::string readString(streamContainer& in);

public:
	explicit IProtocolMessage(const ProtocolId& id) :
			_id(id) {
	}

	virtual ~IProtocolMessage() {
	}

	inline const ProtocolId& getId() const {
		return _id;
	}

	virtual void serialize(streamContainer& out) const {
		addByte(out, _id);
	}
};

inline void IProtocolMessage::addByte(streamContainer& out, uint8_t byte) {
	out.push_back(byte);
}

inline void IProtocolMessage::addBool(streamContainer& out, bool value) {
	out.push_back(value);
}

inline bool IProtocolMessage::readBool(streamContainer& in) {
	return readByte(in) == 1;
}

inline uint8_t IProtocolMessage::readByte(streamContainer& in) {
	const uint8_t b = in.front();
	in.pop_front();
	return b;
}

inline void IProtocolMessage::addFloat(streamContainer& out, float value) {
	union toint {
		float f;
		int32_t i;
	} tmp;
	tmp.f = value;
	addInt(out, tmp.i);
}

inline float IProtocolMessage::readFloat(streamContainer& in) {
	union toint {
		float f;
		int32_t i;
	} tmp;
	tmp.i = readInt(in);
	return tmp.f;
}

inline std::string IProtocolMessage::readString(streamContainer& in) {
	std::string strbuff;
	strbuff.reserve(64);
	for (;;) {
		const char chr = static_cast<char>(in.front());
		in.pop_front();
		if (chr == '\0')
			break;
		strbuff += chr;
	}
	return strbuff;
}

inline void IProtocolMessage::addString(streamContainer& out, const std::string& string) {
	const std::size_t length = string.length();
	for (std::size_t i = 0; i < length; ++i) {
		out.push_back(uint8_t(string[i]));
	}
	out.push_back(uint8_t('\0'));
}

inline void IProtocolMessage::addShort(streamContainer& out, int16_t word) {
	const int16_t swappedWord = AI_SwapLE16(word);
	out.push_back(uint8_t(swappedWord));
	out.push_back(uint8_t(swappedWord >> CHAR_BIT));
}

inline void IProtocolMessage::addInt(streamContainer& out, int32_t dword) {
	int32_t swappedDWord = AI_SwapLE32(dword);
	out.push_back(uint8_t(swappedDWord));
	out.push_back(uint8_t(swappedDWord >>= CHAR_BIT));
	out.push_back(uint8_t(swappedDWord >>= CHAR_BIT));
	out.push_back(uint8_t(swappedDWord >> CHAR_BIT));
}

inline void IProtocolMessage::addLong(streamContainer& out, int64_t dword) {
	int64_t swappedDWord = AI_SwapLE64(dword);
	out.push_back(uint8_t(swappedDWord));
	out.push_back(uint8_t(swappedDWord >>= CHAR_BIT));
	out.push_back(uint8_t(swappedDWord >>= CHAR_BIT));
	out.push_back(uint8_t(swappedDWord >>= CHAR_BIT));
	out.push_back(uint8_t(swappedDWord >>= CHAR_BIT));
	out.push_back(uint8_t(swappedDWord >>= CHAR_BIT));
	out.push_back(uint8_t(swappedDWord >>= CHAR_BIT));
	out.push_back(uint8_t(swappedDWord >> CHAR_BIT));
}

inline int16_t IProtocolMessage::readShort(streamContainer& in) {
	uint8_t buf[2];
	const int l = sizeof(buf);
	for (int i = 0; i < l; ++i) {
		buf[i] = in.front();
		in.pop_front();
	}
	const int16_t *word = (const int16_t*) (void*) &buf;
	const int16_t val = AI_SwapLE16(*word);
	return val;
}

inline int32_t IProtocolMessage::readInt(streamContainer& in) {
	uint8_t buf[4];
	const int l = sizeof(buf);
	for (int i = 0; i < l; ++i) {
		buf[i] = in.front();
		in.pop_front();
	}
	const int32_t *word = (const int32_t*) (void*) &buf;
	const int32_t val = AI_SwapLE32(*word);
	return val;
}

inline int32_t IProtocolMessage::peekInt(const streamContainer& in) {
	uint8_t buf[4];
	const int l = sizeof(buf);
	streamContainer::const_iterator it = in.begin();
	for (int i = 0; i < l; ++i) {
		if (it == in.end())
			return -1;
		buf[i] = *it;
		++it;
	}
	const int32_t *word = (const int32_t*) (void*) &buf;
	const int32_t val = AI_SwapLE32(*word);
	return val;
}

inline int64_t IProtocolMessage::readLong(streamContainer& in) {
	uint8_t buf[8];
	const int l = sizeof(buf);
	for (int i = 0; i < l; ++i) {
		buf[i] = in.front();
		in.pop_front();
	}
	const int64_t *word = (const int64_t*) (void*) &buf;
	const int64_t val = AI_SwapLE64(*word);
	return val;
}

#define PROTO_MSG(name, id) class name : public IProtocolMessage { public: name() : IProtocolMessage(id) {} }

/**
 * @brief Reset the behaviour tree states for all ai controlled entities
 */
PROTO_MSG(AIResetMessage, PROTO_RESET);
/**
 * @brief Protocol keep alive message
 */
PROTO_MSG(AIPingMessage, PROTO_PING);
}

// #include "common/Types.h"


namespace ai {

typedef uint8_t ClientId;

/**
 * @brief Interface for the execution of assigned IProtocolMessage
 *
 * @note Register handler implementations at the @c ProtocolHandlerRegistry
 */
class IProtocolHandler {
public:
	virtual ~IProtocolHandler() {
	}

	virtual void execute(const ClientId& clientId, const IProtocolMessage& message) = 0;
};

template<class T>
class ProtocolHandler : public IProtocolHandler {
public:
	virtual ~ProtocolHandler ()
	{
	}

	void execute (const ClientId& clientId, const IProtocolMessage& message) override {
		const T *msg = ai_assert_cast<const T*>(&message);
		execute(clientId, msg);
	}

	virtual void execute (const ClientId& clientId, const T* message) = 0;
};

class NopHandler: public IProtocolHandler {
public:
	void execute(const ClientId& /*clientId*/, const IProtocolMessage& /*message*/) override {
	}
};

typedef std::shared_ptr<IProtocolHandler> ProtocolHandlerPtr;

/**
 * @brief Use this deleter for any handler that should not get freed by @c delete.
 */
struct ProtocolHandlerNopDeleter {
	void operator()(IProtocolHandler* /* ptr */) {
	}
};

}

// #include "common/Thread.h"

#include <string>
#include <stdint.h>
#include <list>
#ifdef WIN32
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2spi.h>
#else
#define SOCKET  int
#include <sys/select.h>
#endif

namespace ai {

class IProtocolMessage;

struct Client {
	explicit Client(SOCKET _socket) :
			socket(_socket), finished(false), in(), out() {
	}
	SOCKET socket;
	bool finished;
	streamContainer in;
	streamContainer out;
};

class INetworkListener {
public:
	virtual ~INetworkListener() {}

	virtual void onConnect(Client*) {}
	virtual void onDisconnect(Client*) {}
};

class Network {
protected:
	uint16_t _port;
	std::string _hostname;
	// the socket file descriptor
	SOCKET _socketFD;
	fd_set _readFDSet;
	fd_set _writeFDSet;
	int64_t _time;

	typedef std::list<Client> ClientSockets;
	typedef ClientSockets::iterator ClientSocketsIter;
	ClientSockets _clientSockets;
	ClientSocketsIter closeClient (ClientSocketsIter& i);

	typedef std::list<INetworkListener*> Listeners;
	Listeners _listeners;

	bool sendMessage(Client& client);
public:
	Network(uint16_t port = 10001, const std::string& hostname = "0.0.0.0");
	virtual ~Network();

	bool start();
	void update(int64_t deltaTime);

	void addListener(INetworkListener* listener);
	void removeListener(INetworkListener* listener);

	int getConnectedClients() const;

	/**
	 * @return @c false if there are no clients
	 */
	bool broadcast(const IProtocolMessage& msg);
	bool sendToClient(Client* client, const IProtocolMessage& msg);
};

inline int Network::getConnectedClients() const {
	return static_cast<int>(_clientSockets.size());
}

inline void Network::addListener(INetworkListener* listener) {
	_listeners.push_back(listener);
}

inline void Network::removeListener(INetworkListener* listener) {
	_listeners.remove(listener);
}

}

// #include "server/NetworkImpl.h"
/**
 * @file
 */


// #include "Network.h"

// #include "IProtocolMessage.h"

// #include "IProtocolHandler.h"

// #include "ProtocolHandlerRegistry.h"
/**
 * @file
 */


#include <unordered_map>
// #include "IProtocolHandler.h"


namespace ai {

class ProtocolHandlerRegistry {
private:
	typedef std::unordered_map<ProtocolId, IProtocolHandler*> ProtocolHandlers;
	ProtocolHandlers _registry;

	ProtocolHandlerRegistry() {
	}

public:
	static ProtocolHandlerRegistry& get() {
		static ProtocolHandlerRegistry _instance;
		return _instance;
	}

	virtual ~ProtocolHandlerRegistry() {
		_registry.clear();
	}

	inline void registerHandler(const ProtocolId& type, IProtocolHandler* handler) {
		_registry.insert(std::make_pair(type, handler));
	}

	inline IProtocolHandler* getHandler(const IProtocolMessage& msg) {
		ProtocolHandlers::iterator i = _registry.find(msg.getId());
		if (i != _registry.end())
			return i->second;

		return nullptr;
	}
};

}

// #include "ProtocolMessageFactory.h"
/**
 * @file
 */


// #include "common/NonCopyable.h"

// #include "IProtocolMessage.h"

// #include "AIStateMessage.h"
/**
 * @file
 */


// #include "IProtocolMessage.h"

// #include "AIStubTypes.h"
/**
 * @file
 */


#include <vector>
#include <string>
// #include "common/Math.h"

// #include "common/Types.h"

// #include "tree/TreeNode.h"


namespace ai {

/**
 * @brief The aggro entry for the @c AIStateAggro
 *
 * Holds a character id and the assigned aggro value
 */
struct AIStateAggroEntry {
	AIStateAggroEntry(ai::CharacterId _id, float _aggro) :
			id(_id), aggro(_aggro) {
	}
	ai::CharacterId id;
	float aggro;
};

/**
 * @brief The list of aggro entry for a character
 */
class AIStateAggro {
private:
	std::vector<AIStateAggroEntry> _aggro;
public:
	inline void reserve(std::size_t size) {
		if (size > 0)
			_aggro.reserve(size);
	}

	inline void addAggro(const AIStateAggroEntry& entry) {
		_aggro.push_back(entry);
	}

	inline const std::vector<AIStateAggroEntry>& getAggro() const {
		return _aggro;
	}
};

class AIStateNodeStatic {
private:
	int32_t _id;
	std::string _name;
	std::string _type;
	std::string _parameters;
	std::string _conditionType;
	std::string _conditionParameters;
public:
	AIStateNodeStatic(const int32_t id, const std::string& name, const std::string& type, const std::string& parameters, const std::string& conditionType, const std::string& conditionParameters) :
			_id(id), _name(name), _type(type), _parameters(parameters), _conditionType(conditionType), _conditionParameters(conditionParameters) {
	}

	AIStateNodeStatic() :
			_id(-1) {
	}

	inline int32_t getId() const {
		return _id;
	}

	inline const std::string& getName() const {
		return _name;
	}

	inline const std::string& getType() const {
		return _type;
	}

	/**
	 * @brief Returns the raw parameters for the task node
	 */
	inline const std::string& getParameters() const {
		return _parameters;
	}

	/**
	 * @brief Returns the raw condition parameters
	 */
	inline const std::string& getConditionParameters() const {
		return _conditionParameters;
	}

	inline std::string getCondition() const {
		return _conditionType + "(" + _conditionParameters + ")";
	}

	/**
	 * @brief Returns the raw condition type string
	 */
	inline const std::string& getConditionType() const {
		return _conditionType;
	}
};

/**
 * @brief This is a representation of a behaviour tree node for the serialization
 */
class AIStateNode {
private:
	int32_t _nodeId;
	std::string _condition;
	typedef std::vector<AIStateNode> NodeVector;
	NodeVector _children;
	int64_t _lastRun;
	TreeNodeStatus _status;
	bool _currentlyRunning;
public:
	AIStateNode(int32_t id, const std::string& condition, int64_t lastRun, TreeNodeStatus status, bool currentlyRunning) :
			_nodeId(id), _condition(condition), _lastRun(lastRun), _status(status), _currentlyRunning(currentlyRunning) {
	}

	AIStateNode() :
			_nodeId(-1), _lastRun(-1L), _status(UNKNOWN), _currentlyRunning(false) {
	}

	void addChildren(const AIStateNode& child) {
		_children.push_back(child);
	}

	void addChildren(AIStateNode&& child) {
		_children.push_back(std::move(child));
	}

	inline const std::vector<AIStateNode>& getChildren() const {
		return _children;
	}

	inline std::vector<AIStateNode>& getChildren() {
		return _children;
	}

	inline int32_t getNodeId() const {
		return _nodeId;
	}

	inline const std::string& getCondition() const {
		return _condition;
	}

	/**
	 * @return The milliseconds since the last execution of this particular node. or @c -1 if it wasn't executed yet
	 */
	inline int64_t getLastRun() const {
		return _lastRun;
	}

	/**
	 * @return The @c TreeNodeStatus of the last execution
	 */
	inline TreeNodeStatus getStatus() const {
		return _status;
	}

	/**
	 * @brief Some nodes have a state that holds which children is currently running
	 * @return Whether this particular node is currently running
	 */
	inline bool isRunning() const {
		return _currentlyRunning;
	}
};

/**
 * @brief This is a representation of a character state for the serialization
 */
class AIStateWorld {
private:
	ai::CharacterId _id;
	glm::vec3 _position;
	float _orientation;
	CharacterAttributes _attributes;
public:
	AIStateWorld() : _id(-1), _position(0.0f, 0.0f, 0.0f), _orientation(0.0f) {}

	AIStateWorld(const ai::CharacterId& id, const glm::vec3& position, float orientation, const CharacterAttributes& attributes) :
			_id(id), _position(position), _orientation(orientation), _attributes(attributes) {
	}

	AIStateWorld(const ai::CharacterId& id, const glm::vec3& position, float orientation, CharacterAttributes&& attributes) :
			_id(id), _position(position), _orientation(orientation), _attributes(std::move(attributes)) {
	}

	AIStateWorld(const ai::CharacterId& id, const glm::vec3& position, float orientation) :
			_id(id), _position(position), _orientation(orientation) {
	}

	inline bool operator==(const AIStateWorld &other) const {
		return _id == other._id;
	}

	inline bool operator<(const AIStateWorld &other) const {
		return _id < other._id;
	}

	/**
	 * @return The unique id that can be used to identify the character in the world
	 */
	inline const ai::CharacterId& getId() const {
		return _id;
	}

	/**
	 * @return The orientation of the character [0, 2*PI]
	 *
	 * @note A negative value means, that the character does not have any orientation
	 */
	inline float getOrientation() const {
		return _orientation;
	}

	/**
	 * @return The position in the world
	 */
	inline const glm::vec3& getPosition() const {
		return _position;
	}

	/**
	 * @return Attributes for the entity
	 */
	inline const CharacterAttributes& getAttributes() const {
		return _attributes;
	}

	/**
	 * @return Attributes for the entity to fill
	 */
	inline CharacterAttributes& getAttributes() {
		return _attributes;
	}
};

}

#include <vector>

namespace ai {

/**
 * @brief Message for the remote debugging interface
 *
 * State of the world. You receive basic information about every watched AI controller entity
 */
class AIStateMessage: public IProtocolMessage {
private:
	typedef std::vector<AIStateWorld> States;
	States _states;

	void readState (streamContainer& in) {
		const ai::CharacterId id = readInt(in);
		const float x = readFloat(in);
		const float y = readFloat(in);
		const float z = readFloat(in);
		const float orientation = readFloat(in);
		const glm::vec3 position(x, y, z);

		AIStateWorld tree(id, position, orientation);
		CharacterAttributes& attributes = tree.getAttributes();
		readAttributes(in, attributes);
		_states.push_back(tree);
	}

	void writeState (streamContainer& out, const AIStateWorld& state) const {
		addInt(out, state.getId());
		const glm::vec3& position = state.getPosition();
		addFloat(out, position.x);
		addFloat(out, position.y);
		addFloat(out, position.z);
		addFloat(out, state.getOrientation());
		writeAttributes(out, state.getAttributes());
	}

	void writeAttributes(streamContainer& out, const CharacterAttributes& attributes) const {
		addShort(out, static_cast<int16_t>(attributes.size()));
		for (CharacterAttributes::const_iterator i = attributes.begin(); i != attributes.end(); ++i) {
			addString(out, i->first);
			addString(out, i->second);
		}
	}

	void readAttributes(streamContainer& in, CharacterAttributes& attributes) const {
		const int size = readShort(in);
		attributes.reserve(size);
		for (int i = 0; i < size; ++i) {
			const std::string& key = readString(in);
			const std::string& value = readString(in);
			attributes.insert(std::make_pair(key, value));
		}
	}

public:
	AIStateMessage() :
			IProtocolMessage(PROTO_STATE) {
	}

	explicit AIStateMessage(streamContainer& in) :
			IProtocolMessage(PROTO_STATE) {
		const int treeSize = readInt(in);
		for (int i = 0; i < treeSize; ++i) {
			readState(in);
		}
	}

	void addState(const AIStateWorld& tree) {
		_states.push_back(tree);
	}

	void addState(AIStateWorld&& tree) {
		_states.push_back(std::move(tree));
	}

	void serialize(streamContainer& out) const override {
		addByte(out, _id);
		addInt(out, static_cast<int>(_states.size()));
		for (States::const_iterator i = _states.begin(); i != _states.end(); ++i) {
			writeState(out, *i);
		}
	}

	inline const std::vector<AIStateWorld>& getStates() const {
		return _states;
	}
};

}

// #include "AICharacterDetailsMessage.h"
/**
 * @file
 */


// #include "IProtocolMessage.h"

// #include "AIStubTypes.h"

#include <vector>

namespace ai {

/**
 * @brief Message for the remote debugging interface
 *
 * If someone selected a character this message gets broadcasted.
 */
class AICharacterDetailsMessage: public IProtocolMessage {
private:
	CharacterId _chrId;
	const AIStateAggro* _aggroPtr;
	const AIStateNode* _rootPtr;
	AIStateAggro _aggro;
	AIStateNode _root;

	AIStateNode readNode (streamContainer& in) {
		const int32_t nodeId = readInt(in);
		const std::string& condition = readString(in);
		const int64_t lastRun = readLong(in);
		const TreeNodeStatus status = static_cast<TreeNodeStatus>(readByte(in));
		const bool running = readBool(in);
		const int16_t childrenCount = readShort(in);
		AIStateNode node(nodeId, condition, lastRun, status, running);
		for (uint8_t i = 0; i < childrenCount; ++i) {
			const AIStateNode& child = readNode(in);
			node.addChildren(child);
		}
		return node;
	}

	void writeNode (streamContainer& out, const AIStateNode& node) const {
		addInt(out, node.getNodeId());
		addString(out, node.getCondition());
		addLong(out, node.getLastRun());
		addByte(out, node.getStatus());
		addBool(out, node.isRunning());
		const std::vector<AIStateNode>& children = node.getChildren();
		addShort(out, static_cast<int16_t>(children.size()));
		for (std::vector<AIStateNode>::const_iterator i = children.begin(); i != children.end(); ++i) {
			writeNode(out, *i);
		}
	}

	void writeAggro(streamContainer& out, const AIStateAggro& aggro) const {
		const std::vector<AIStateAggroEntry>& a = aggro.getAggro();
		addShort(out, static_cast<int16_t>(a.size()));
		for (std::vector<AIStateAggroEntry>::const_iterator i = a.begin(); i != a.end(); ++i) {
			addInt(out, i->id);
			addFloat(out, i->aggro);
		}
	}

	void readAggro(streamContainer& in, AIStateAggro& aggro) const {
		const int size = readShort(in);
		for (int i = 0; i < size; ++i) {
			const CharacterId chrId = readInt(in);
			const float aggroVal = readFloat(in);
			aggro.addAggro(AIStateAggroEntry(chrId, aggroVal));
		}
	}

public:
	/**
	 * Make sure that none of the given references is destroyed, for performance reasons we are only storing the
	 * pointers to those instances in this class. So they need to stay valid until they are serialized.
	 */
	AICharacterDetailsMessage(const CharacterId& id, const AIStateAggro& aggro, const AIStateNode& root) :
			IProtocolMessage(PROTO_CHARACTER_DETAILS), _chrId(id), _aggroPtr(&aggro), _rootPtr(&root) {
	}

	explicit AICharacterDetailsMessage(streamContainer& in) :
			IProtocolMessage(PROTO_CHARACTER_DETAILS), _aggroPtr(nullptr), _rootPtr(nullptr) {
		_chrId = readInt(in);
		readAggro(in, _aggro);
		_root = readNode(in);
	}

	void serialize(streamContainer& out) const override {
		addByte(out, _id);
		addInt(out, _chrId);
		writeAggro(out, *_aggroPtr);
		writeNode(out, *_rootPtr);
	}

	inline const CharacterId& getCharacterId() const {
		return _chrId;
	}

	inline const AIStateAggro& getAggro() const {
		if (_aggroPtr)
			return *_aggroPtr;
		return _aggro;
	}

	inline const AIStateNode& getNode() const {
		if (_rootPtr)
			return *_rootPtr;
		return _root;
	}
};

}

// #include "AICharacterStaticMessage.h"
/**
 * @file
 */


// #include "AIStubTypes.h"

#include <vector>

namespace ai {

class AICharacterStaticMessage : public IProtocolMessage {
private:
	CharacterId _chrId;
	const std::vector<AIStateNodeStatic>* _nodeStaticDataPtr;
	std::vector<AIStateNodeStatic> _nodeStaticData;
public:
	/**
	 * Make sure that none of the given references is destroyed, for performance reasons we are only storing the
	 * pointers to those instances in this class. So they need to stay valid until they are serialized.
	 */
	AICharacterStaticMessage(const CharacterId& id, const std::vector<AIStateNodeStatic>& nodeStaticData) :
			IProtocolMessage(PROTO_CHARACTER_STATIC), _chrId(id), _nodeStaticDataPtr(&nodeStaticData) {
	}

	explicit AICharacterStaticMessage(streamContainer& in) :
			IProtocolMessage(PROTO_CHARACTER_STATIC), _nodeStaticDataPtr(nullptr) {
		_chrId = readInt(in);
		const std::size_t size = readInt(in);
		_nodeStaticData.reserve(size);
		for (std::size_t i = 0; i < size; ++i) {
			const int32_t id = readInt(in);
			const std::string& name = readString(in);
			const std::string& type = readString(in);
			const std::string& parameters = readString(in);
			const std::string& conditionType = readString(in);
			const std::string& conditionParameters = readString(in);
			const AIStateNodeStatic staticData(id, name, type, parameters, conditionType, conditionParameters);
			_nodeStaticData.push_back(staticData);
		}
	}

	void serialize(streamContainer& out) const override {
		addByte(out, _id);
		addInt(out, _chrId);
		const std::size_t size = _nodeStaticDataPtr->size();
		addInt(out, static_cast<int>(size));
		for (std::size_t i = 0; i < size; ++i) {
			addInt(out, (*_nodeStaticDataPtr)[i].getId());
			addString(out, (*_nodeStaticDataPtr)[i].getName());
			addString(out, (*_nodeStaticDataPtr)[i].getType());
			addString(out, (*_nodeStaticDataPtr)[i].getParameters());
			addString(out, (*_nodeStaticDataPtr)[i].getConditionType());
			addString(out, (*_nodeStaticDataPtr)[i].getConditionParameters());
		}
	}

	inline const std::vector<AIStateNodeStatic>& getStaticNodeData() const {
		if (_nodeStaticDataPtr)
			return *_nodeStaticDataPtr;
		return _nodeStaticData;
	}
};

}

// #include "AIPauseMessage.h"
/**
 * @file
 */


// #include "IProtocolMessage.h"


namespace ai {

/**
 * @brief Message for the remote debugging interface
 *
 * If this is received on the server side, it will pause the execution of
 * the behaviour tree for all ai controlled entities. You can then step
 * the execution of all those entities by sending a @c AIStepMessage
 *
 * The server is sending the @AIPauseMessage back to the clients so they know
 * whether it worked or not.
 */
class AIPauseMessage: public IProtocolMessage {
private:
	bool _pause;

public:
	explicit AIPauseMessage(bool pause) :
			IProtocolMessage(PROTO_PAUSE),_pause(pause) {
	}

	explicit AIPauseMessage(streamContainer& in) :
			IProtocolMessage(PROTO_PAUSE) {
		_pause = readBool(in);
	}

	void serialize(streamContainer& out) const override {
		addByte(out, _id);
		addBool(out, _pause);
	}

	inline bool isPause() const {
		return _pause;
	}
};

}

// #include "AISelectMessage.h"
/**
 * @file
 */


// #include "IProtocolMessage.h"

// #include "ICharacter.h"


namespace ai {

/**
 * @brief Message for the remote debugging interface
 *
 * Select a particular character to get detailed information about its state
 */
class AISelectMessage: public IProtocolMessage {
private:
	ai::CharacterId _chrId;

public:
	explicit AISelectMessage(ai::CharacterId id) :
			IProtocolMessage(PROTO_SELECT) {
		_chrId = id;
	}

	explicit AISelectMessage(streamContainer& in) :
			IProtocolMessage(PROTO_SELECT) {
		_chrId = readInt(in);
	}

	void serialize(streamContainer& out) const override {
		addByte(out, _id);
		addInt(out, _chrId);
	}

	inline const CharacterId& getCharacterId() const {
		return _chrId;
	}
};

}

// #include "AINamesMessage.h"
/**
 * @file
 */


// #include "IProtocolMessage.h"


namespace ai {

/**
 * @brief Message for the remote debugging interface
 *
 * Get a list of all potential subsets that can be selected by @c AIChangeMessage
 */
class AINamesMessage: public IProtocolMessage {
private:
	std::vector<std::string> _names;
	const std::vector<std::string>* _namesPtr;

public:
	explicit AINamesMessage(const std::vector<std::string>& names) :
			IProtocolMessage(PROTO_NAMES), _namesPtr(&names) {
	}

	explicit AINamesMessage(streamContainer& in) :
			IProtocolMessage(PROTO_NAMES), _namesPtr(nullptr) {
		const int size = readInt(in);
		for (int i = 0; i < size; ++i) {
			_names.push_back(readString(in));
		}
	}

	void serialize(streamContainer& out) const override {
		addByte(out, _id);
		const std::size_t size = _namesPtr->size();
		addInt(out, static_cast<int>(size));
		for (std::size_t i = 0U; i < size; ++i) {
			addString(out, (*_namesPtr)[i]);
		}
	}

	inline const std::vector<std::string>& getNames() const {
		if (_namesPtr)
			return *_namesPtr;
		return _names;
	}
};

}

// #include "AIChangeMessage.h"
/**
 * @file
 */


// #include "IProtocolMessage.h"


namespace ai {

/**
 * @brief Message for the remote debugging interface
 *
 * Allows to select a particular subset of entities to receive the state for
 * by handling the @c AINamesMessage
 */
class AIChangeMessage: public IProtocolMessage {
private:
	const std::string _name;

public:
	explicit AIChangeMessage(const std::string& name) :
			IProtocolMessage(PROTO_CHANGE), _name(name) {
	}

	explicit AIChangeMessage(streamContainer& in) :
			IProtocolMessage(PROTO_CHANGE), _name(readString(in)) {
	}

	void serialize(streamContainer& out) const override {
		addByte(out, _id);
		addString(out, _name);
	}

	inline const std::string& getName() const {
		return _name;
	}
};

}

// #include "AIStepMessage.h"
/**
 * @file
 */


// #include "IProtocolMessage.h"


namespace ai {

/**
 * @brief Perform one step if the ai controlled entities are in paused mode
 *
 * Also see @c AIPauseMessage
 */
class AIStepMessage: public IProtocolMessage {
private:
	int64_t _millis;

public:
	explicit AIStepMessage(int64_t millis) :
			IProtocolMessage(PROTO_STEP) {
		_millis = millis;
	}

	explicit AIStepMessage(streamContainer& in) :
			IProtocolMessage(PROTO_STEP) {
		_millis = readLong(in);
	}

	void serialize(streamContainer& out) const override {
		addByte(out, _id);
		addLong(out, _millis);
	}

	inline int64_t getStepMillis() const {
		return _millis;
	}
};

}

// #include "AIUpdateNodeMessage.h"
/**
 * @file
 */


// #include "IProtocolMessage.h"

// #include "AIStubTypes.h"


namespace ai {

/**
 * @brief Message for the remote debugging interface
 *
 * Updates a node to some other type, name or condition
 */
class AIUpdateNodeMessage: public IProtocolMessage {
private:
	int32_t _nodeId;
	CharacterId _characterId;
	std::string _name;
	std::string _type;
	std::string _condition;

public:
	AIUpdateNodeMessage(int32_t nodeId, CharacterId characterId, const std::string& name, const std::string& type,
			const std::string& condition) :
			IProtocolMessage(PROTO_UPDATENODE), _nodeId(nodeId), _characterId(characterId), _name(name), _type(type), _condition(condition) {
	}

	explicit AIUpdateNodeMessage(streamContainer& in) :
			IProtocolMessage(PROTO_UPDATENODE) {
		_nodeId = readInt(in);
		_characterId = readInt(in);
		_name = readString(in);
		_type = readString(in);
		_condition = readString(in);
	}

	void serialize(streamContainer& out) const override {
		addByte(out, _id);
		addInt(out, _nodeId);
		addInt(out, _characterId);
		addString(out, _name);
		addString(out, _type);
		addString(out, _condition);
	}

	inline const std::string& getName() const {
		return _name;
	}

	inline const std::string& getType() const {
		return _type;
	}

	inline const std::string& getCondition() const {
		return _condition;
	}

	inline uint32_t getNodeId() const {
		return _nodeId;
	}

	inline CharacterId getCharacterId() const {
		return _characterId;
	}
};

}

// #include "AIAddNodeMessage.h"
/**
 * @file
 */


// #include "IProtocolMessage.h"

// #include "AIStubTypes.h"


namespace ai {

/**
 * @brief Message for the remote debugging interface
 *
 * Adds a new node to some parent node
 */
class AIAddNodeMessage: public IProtocolMessage {
private:
	int32_t _parentNodeId;
	CharacterId _characterId;
	std::string _name;
	std::string _type;
	std::string _condition;

public:
	AIAddNodeMessage(int32_t parentNodeId, CharacterId characterId, const std::string& name, const std::string& type, const std::string& condition) :
			IProtocolMessage(PROTO_ADDNODE), _parentNodeId(parentNodeId), _characterId(characterId), _name(name), _type(type), _condition(condition) {
	}

	explicit AIAddNodeMessage(streamContainer& in) :
			IProtocolMessage(PROTO_ADDNODE) {
		_parentNodeId = readInt(in);
		_characterId = readInt(in);
		_name = readString(in);
		_type = readString(in);
		_condition = readString(in);
	}

	void serialize(streamContainer& out) const override {
		addByte(out, _id);
		addInt(out, _parentNodeId);
		addInt(out, _characterId);
		addString(out, _name);
		addString(out, _type);
		addString(out, _condition);
	}

	inline const std::string& getName() const {
		return _name;
	}

	inline const std::string& getType() const {
		return _type;
	}

	inline const std::string& getCondition() const {
		return _condition;
	}

	inline uint32_t getParentNodeId() const {
		return _parentNodeId;
	}

	inline CharacterId getCharacterId() const {
		return _characterId;
	}
};

}

// #include "AIDeleteNodeMessage.h"
/**
 * @file
 */


// #include "IProtocolMessage.h"

// #include "AIStubTypes.h"


namespace ai {

/**
 * @brief Message for the remote debugging interface
 *
 * Deletes a node
 */
class AIDeleteNodeMessage: public IProtocolMessage {
private:
	int32_t _nodeId;
	CharacterId _characterId;

public:
	AIDeleteNodeMessage(int32_t nodeId, CharacterId characterId) :
			IProtocolMessage(PROTO_DELETENODE), _nodeId(nodeId), _characterId(characterId) {
	}

	explicit AIDeleteNodeMessage(streamContainer& in) :
			IProtocolMessage(PROTO_DELETENODE) {
		_nodeId = readInt(in);
		_characterId = readInt(in);
	}

	void serialize(streamContainer& out) const override {
		addByte(out, _id);
		addInt(out, _nodeId);
		addInt(out, _characterId);
	}

	inline uint32_t getNodeId() const {
		return _nodeId;
	}

	inline CharacterId getCharacterId() const {
		return _characterId;
	}
};

}


namespace ai {

class ProtocolMessageFactory : public NonCopyable {
private:
	uint8_t *_aiState;
	uint8_t *_aiSelect;
	uint8_t *_aiPause;
	uint8_t *_aiNames;
	uint8_t *_aiChange;
	uint8_t *_aiReset;
	uint8_t *_aiStep;
	uint8_t *_aiPing;
	uint8_t *_aiCharacterDetails;
	uint8_t *_aiCharacterStatic;
	uint8_t *_aiUpdateNode;
	uint8_t *_aiAddNode;
	uint8_t *_aiDeleteNode;

	ProtocolMessageFactory() :
		_aiState(new uint8_t[sizeof(AIStateMessage)]),
		_aiSelect(new uint8_t[sizeof(AISelectMessage)]),
		_aiPause(new uint8_t[sizeof(AIPauseMessage)]),
		_aiNames(new uint8_t[sizeof(AINamesMessage)]),
		_aiChange(new uint8_t[sizeof(AIChangeMessage)]),
		_aiReset(new uint8_t[sizeof(AIResetMessage)]),
		_aiStep(new uint8_t[sizeof(AIStepMessage)]),
		_aiPing(new uint8_t[sizeof(AIPingMessage)]),
		_aiCharacterDetails(new uint8_t[sizeof(AICharacterDetailsMessage)]),
		_aiCharacterStatic(new uint8_t[sizeof(AICharacterStaticMessage)]),
		_aiUpdateNode(new uint8_t[sizeof(AIUpdateNodeMessage)]),
		_aiAddNode(new uint8_t[sizeof(AIAddNodeMessage)]),
		_aiDeleteNode(new uint8_t[sizeof(AIDeleteNodeMessage)]) {
	}
public:
	~ProtocolMessageFactory() {
		delete[] _aiState;
		delete[] _aiSelect;
		delete[] _aiPause;
		delete[] _aiNames;
		delete[] _aiChange;
		delete[] _aiReset;
		delete[] _aiStep;
		delete[] _aiPing;
		delete[] _aiCharacterDetails;
		delete[] _aiCharacterStatic;
		delete[] _aiUpdateNode;
		delete[] _aiAddNode;
		delete[] _aiDeleteNode;
	}

	static ProtocolMessageFactory& get() {
		static ProtocolMessageFactory _instance;
		return _instance;
	}

	/**
	 * @brief Checks whether a new message is available in the stream
	 */
	inline bool isNewMessageAvailable(const streamContainer& in) const {
		const int32_t size = IProtocolMessage::peekInt(in);
		if (size == -1) {
			// not enough data yet, wait a little bit more
			return false;
		}
		const int streamSize = static_cast<int>(in.size() - sizeof(int32_t));
		if (size > streamSize) {
			// not enough data yet, wait a little bit more
			return false;
		}
		return true;
	}

	/**
	 * @brief Call this only if @c isNewMessageAvailable returned @c true on the same @c streamContainer before!
	 * @note Don't free this pointer, it reuses memory for each new protocol message
	 */
	IProtocolMessage *create(streamContainer& in) {
		// remove the size from the stream
		in.erase(in.begin(), std::next(in.begin(), sizeof(int32_t)));
		// get the message type
		const uint8_t type = in.front();
		in.pop_front();
		if (type == PROTO_STATE) {
			return new (_aiState) AIStateMessage(in);
		} else if (type == PROTO_SELECT) {
			return new (_aiSelect) AISelectMessage(in);
		} else if (type == PROTO_PAUSE) {
			return new (_aiPause) AIPauseMessage(in);
		} else if (type == PROTO_NAMES) {
			return new (_aiNames) AINamesMessage(in);
		} else if (type == PROTO_CHANGE) {
			return new (_aiChange) AIChangeMessage(in);
		} else if (type == PROTO_RESET) {
			return new (_aiReset) AIResetMessage();
		} else if (type == PROTO_STEP) {
			return new (_aiStep) AIStepMessage(in);
		} else if (type == PROTO_PING) {
			return new (_aiPing) AIPingMessage();
		} else if (type == PROTO_CHARACTER_DETAILS) {
			return new (_aiCharacterDetails) AICharacterDetailsMessage(in);
		} else if (type == PROTO_CHARACTER_STATIC) {
			return new (_aiCharacterStatic) AICharacterStaticMessage(in);
		} else if (type == PROTO_UPDATENODE) {
			return new (_aiUpdateNode) AIUpdateNodeMessage(in);
		} else if (type == PROTO_ADDNODE) {
			return new (_aiAddNode) AIAddNodeMessage(in);
		} else if (type == PROTO_DELETENODE) {
			return new (_aiDeleteNode) AIDeleteNodeMessage(in);
		}

		return nullptr;
	}
};

}

#ifdef WIN32
#define network_cleanup() WSACleanup()
#define network_return int
#else
#define network_return ssize_t
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#include <signal.h>
#define closesocket close
#define INVALID_SOCKET  -1
#define network_cleanup()
#endif
#include <string.h>
#include <deque>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <iterator>
#include <array>

namespace ai {

inline Network::Network(uint16_t port, const std::string& hostname) :
		_port(port), _hostname(hostname), _socketFD(INVALID_SOCKET), _time(0L) {
	FD_ZERO(&_readFDSet);
	FD_ZERO(&_writeFDSet);
}

inline Network::~Network() {
	closesocket(_socketFD);
	network_cleanup();
}

inline bool Network::start() {
#ifdef WIN32
	WSADATA wsaData;
	const int wsaResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (wsaResult != NO_ERROR) {
		return false;
	}
#else
	signal(SIGPIPE, SIG_IGN);
#endif

	FD_ZERO(&_readFDSet);
	FD_ZERO(&_writeFDSet);

	_socketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_socketFD == INVALID_SOCKET) {
		network_cleanup();
		return false;
	}
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(_port);

	int t = 1;
#ifdef _WIN32
	if (setsockopt(_socketFD, SOL_SOCKET, SO_REUSEADDR, (char*) &t, sizeof(t)) != 0) {
#else
	if (setsockopt(_socketFD, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t)) != 0) {
#endif
		closesocket(_socketFD);
		return false;
	}

	if (bind(_socketFD, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
		// Handle the error.
		network_cleanup();
		FD_CLR(_socketFD, &_readFDSet);
		FD_CLR(_socketFD, &_writeFDSet);
		closesocket(_socketFD);
		_socketFD = INVALID_SOCKET;
		return false;
	}

	if (listen(_socketFD, 5) < 0) {
		// Handle the error.
		network_cleanup();
		closesocket(_socketFD);
		_socketFD = INVALID_SOCKET;
		return false;
	}

#ifdef O_NONBLOCK
	fcntl(_socketFD, F_SETFL, O_NONBLOCK);
#endif
#ifdef WIN32
	unsigned long mode = 1;
	ioctlsocket(_socketFD, FIONBIO, &mode);
#endif

	FD_SET(_socketFD, &_readFDSet);

	return true;
}

inline Network::ClientSocketsIter Network::closeClient(ClientSocketsIter& iter) {
	Client& client = *iter;
	const SOCKET clientSocket = client.socket;
	FD_CLR(clientSocket, &_readFDSet);
	FD_CLR(clientSocket, &_writeFDSet);
	closesocket(clientSocket);
	client.socket = INVALID_SOCKET;
	for (INetworkListener* listener : _listeners) {
		listener->onDisconnect(&client);
	}
	return _clientSockets.erase(iter);
}

inline bool Network::sendMessage(Client& client) {
	if (client.out.empty()) {
		return true;
	}

	std::array<uint8_t, 16384> buf;
	while (!client.out.empty()) {
		const size_t len = std::min(buf.size(), client.out.size());
		std::copy_n(client.out.begin(), len, buf.begin());
		const SOCKET clientSocket = client.socket;
		const network_return sent = send(clientSocket, (const char*)&buf[0], len, 0);
		if (sent < 0) {
			return false;
		}
		if (sent == 0) {
			// better luck next time - but don't block others
			return true;
		}
		client.out.erase(client.out.begin(), std::next(client.out.begin(), sent));
	}
	return true;
}

inline void Network::update(int64_t deltaTime) {
	_time += deltaTime;
	if (_time > 5000L) {
		if (!broadcast(AIPingMessage())) {
			_time = 0L;
		}
	}
	fd_set readFDsOut;
	fd_set writeFDsOut;

	memcpy(&readFDsOut, &_readFDSet, sizeof(readFDsOut));
	memcpy(&writeFDsOut, &_writeFDSet, sizeof(writeFDsOut));

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	const int ready = select(FD_SETSIZE, &readFDsOut, &writeFDsOut, nullptr, &tv);
	if (ready < 0) {
		return;
	}
	if (_socketFD != INVALID_SOCKET && FD_ISSET(_socketFD, &readFDsOut)) {
		const SOCKET clientSocket = accept(_socketFD, nullptr, nullptr);
		if (clientSocket != INVALID_SOCKET) {
			FD_SET(clientSocket, &_readFDSet);
			const Client c(clientSocket);
			_clientSockets.push_back(c);
			for (INetworkListener* listener : _listeners) {
				listener->onConnect(&_clientSockets.back());
			}
		}
	}

	ClientId clientId = 0;
	for (ClientSocketsIter i = _clientSockets.begin(); i != _clientSockets.end(); ++clientId) {
		Client& client = *i;
		const SOCKET clientSocket = client.socket;
		if (clientSocket == INVALID_SOCKET) {
			i = closeClient(i);
			continue;
		}

		if (FD_ISSET(clientSocket, &writeFDsOut)) {
			if (!sendMessage(client) || client.finished) {
				i = closeClient(i);
				continue;
			}
		}

		if (FD_ISSET(clientSocket, &readFDsOut)) {
			std::array<uint8_t, 16384> buf;
			const network_return len = recv(clientSocket, (char*)&buf[0], buf.size(), 0);
			if (len < 0) {
				i = closeClient(i);
				continue;
			}
			std::copy_n(buf.begin(), len, std::back_inserter(client.in));
		}

		ProtocolMessageFactory& factory = ProtocolMessageFactory::get();
		if (factory.isNewMessageAvailable(client.in)) {
			IProtocolMessage* msg = factory.create(client.in);
			if (!msg) {
				i = closeClient(i);
				continue;
			}
			IProtocolHandler* handler = ProtocolHandlerRegistry::get().getHandler(*msg);
			if (handler) {
				handler->execute(clientId, *msg);
			}
		}
		++i;
	}
}

inline bool Network::broadcast(const IProtocolMessage& msg) {
	if (_clientSockets.empty()) {
		return false;
	}
	_time = 0L;
	streamContainer out;
	msg.serialize(out);
	for (ClientSocketsIter i = _clientSockets.begin(); i != _clientSockets.end(); ++i) {
		Client& client = *i;
		if (client.socket == INVALID_SOCKET) {
			i = closeClient(i);
			continue;
		}

		IProtocolMessage::addInt(client.out, static_cast<int32_t>(out.size()));
		std::copy(out.begin(), out.end(), std::back_inserter(client.out));
		FD_SET(client.socket, &_writeFDSet);
	}

	return true;
}

inline bool Network::sendToClient(Client* client, const IProtocolMessage& msg) {
	assert(client != nullptr);
	if (client->socket == INVALID_SOCKET) {
		return false;
	}

	streamContainer out;
	msg.serialize(out);

	IProtocolMessage::addInt(client->out, static_cast<int32_t>(out.size()));
	std::copy(out.begin(), out.end(), std::back_inserter(client->out));
	FD_SET(client->socket, &_writeFDSet);
	return true;
}

#undef network_cleanup
#undef INVALID_SOCKET
#ifndef WIN32
#undef closesocket
#endif

}

// #include "server/Server.h"
/**
 * @file
 */


// #include "common/Thread.h"

// #include "tree/TreeNode.h"

#include <unordered_set>
// #include "Network.h"

// #include "zone/Zone.h"

// #include "AIRegistry.h"

// #include "AIStateMessage.h"

// #include "AINamesMessage.h"

// #include "AIStubTypes.h"

// #include "AICharacterDetailsMessage.h"

// #include "AICharacterStaticMessage.h"

// #include "ProtocolHandlerRegistry.h"

// #include "conditions/ConditionParser.h"
/**
 * @file
 * @ingroup Condition
 */


// #include "conditions/ICondition.h"

// #include "common/IParser.h"

// #include "conditions/Filter.h"

// #include "AIRegistry.h"


namespace ai {

class IAIFactory;

/**
 * @brief Transforms the string representation of a condition with all its sub conditions and
 * parameters into a @c ICondition instance.
 *
 * @c #ConditionName{Parameters}(#SubCondition{SubConditionParameters},...)
 * Parameters and subconditions are both optional.
 */
class ConditionParser : public IParser {
private:
	const IAIFactory& _aiFactory;
	std::string _conditionString;

	void splitConditions(const std::string& string, std::vector<std::string>& tokens) const;
	bool fillInnerConditions(ConditionFactoryContext& ctx, const std::string& inner);
	bool fillInnerFilters(FilterFactoryContext& ctx, const std::string& inner);

public:
	ConditionParser(const IAIFactory& aiFactory, const std::string& conditionString) :
			IParser(), _aiFactory(aiFactory) {
		_conditionString = ai::Str::eraseAllSpaces(conditionString);
	}
	virtual ~ConditionParser() {}

	ConditionPtr getCondition();
};

inline void ConditionParser::splitConditions(const std::string& string, std::vector<std::string>& tokens) const {
	int inParameter = 0;
	int inChildren = 0;
	std::string token;
	for (std::string::const_iterator i = string.begin(); i != string.end(); ++i) {
		if (*i == '{') {
			++inParameter;
		} else if (*i == '}') {
			--inParameter;
		} else if (*i == '(') {
			++inChildren;
		} else if (*i == ')') {
			--inChildren;
		}

		if (inParameter == 0 && inChildren == 0) {
			if (*i == ',') {
				tokens.push_back(token);
				token.clear();
				continue;
			}
		}
		token.push_back(*i);
	}
	tokens.push_back(token);
}

inline bool ConditionParser::fillInnerFilters(FilterFactoryContext& ctx, const std::string& filterStr) {
	std::vector<std::string> conditions;
	splitConditions(filterStr, conditions);
	if (conditions.size() > 1) {
		for (std::vector<std::string>::const_iterator i = conditions.begin(); i != conditions.end(); ++i) {
			if (!fillInnerFilters(ctx, *i)) {
				return false;
			}
		}
		return true;
	}

	std::string parameters;
	std::size_t n = filterStr.find("(");
	if (n == std::string::npos || filterStr.find("{") < n) {
		parameters = getBetween(filterStr, "{", "}");
		n = filterStr.find("{");
	}

	std::string name;
	if (n != std::string::npos) {
		name = filterStr.substr(0, n);
	} else {
		name = filterStr;
	}
	FilterFactoryContext ctxInner(parameters);
	n = filterStr.find("(");
	if (n != std::string::npos) {
		const std::size_t r = filterStr.rfind(")");
		if (r == std::string::npos) {
			setError("syntax error, missing closing brace");
			return false;
		}
		const std::string& inner = filterStr.substr(n + 1, r - n - 1);
		if (!fillInnerFilters(ctxInner, inner)) {
			return false;
		}
	}
	const FilterPtr& c = _aiFactory.createFilter(name, ctxInner);
	if (!c) {
		setError("could not create filter for %s", name.c_str());
		return false;
	}
	ctx.filters.push_back(c);
	return true;
}

inline bool ConditionParser::fillInnerConditions(ConditionFactoryContext& ctx, const std::string& conditionStr) {
	std::vector<std::string> conditions;
	splitConditions(conditionStr, conditions);
	if (conditions.size() > 1) {
		for (std::vector<std::string>::const_iterator i = conditions.begin(); i != conditions.end(); ++i) {
			if (!fillInnerConditions(ctx, *i)) {
				return false;
			}
		}
	} else {
		std::string parameters;
		std::size_t n = conditionStr.find("(");
		if (n == std::string::npos || conditionStr.find("{") < n) {
			parameters = getBetween(conditionStr, "{", "}");
			n = conditionStr.find("{");
		}

		std::string name;
		if (n != std::string::npos) {
			name = conditionStr.substr(0, n);
		} else {
			name = conditionStr;
		}
		// filter condition is a little bit special and deserves some extra attention
		if (ctx.filter) {
			FilterFactoryContext ctxInner(parameters);
			n = conditionStr.find("(");
			if (n != std::string::npos) {
				const std::size_t r = conditionStr.rfind(")");
				if (r == std::string::npos) {
					setError("syntax error, missing closing brace");
					return false;
				}
				const std::string& inner = conditionStr.substr(n + 1, r - n - 1);
				if (!fillInnerFilters(ctxInner, inner)) {
					return false;
				}
			}
			const FilterPtr& c = _aiFactory.createFilter(name, ctxInner);
			if (!c) {
				setError("could not create filter for %s", name.c_str());
				return false;
			}
			ctx.filters.push_back(c);
		} else {
			ConditionFactoryContext ctxInner(parameters);
			ctxInner.filter = name == FILTER_NAME;
			n = conditionStr.find("(");
			if (n != std::string::npos) {
				const std::size_t r = conditionStr.rfind(")");
				if (r == std::string::npos) {
					setError("syntax error, missing closing brace");
					return false;
				}
				const std::string& inner = conditionStr.substr(n + 1, r - n - 1);
				if (!fillInnerConditions(ctxInner, inner)) {
					return false;
				}
			}
			const ConditionPtr& c = _aiFactory.createCondition(name, ctxInner);
			if (!c) {
				setError("could not create inner condition for %s", name.c_str());
				return false;
			}
			ctx.conditions.push_back(c);
		}
	}
	return true;
}

inline ConditionPtr ConditionParser::getCondition() {
	resetError();
	std::string parameters;
	std::size_t n = _conditionString.find("(");
	if (n == std::string::npos || _conditionString.find("{") < n) {
		parameters = getBetween(_conditionString, "{", "}");
		n = _conditionString.find("{");
	}
	std::string name;
	if (n != std::string::npos) {
		name = _conditionString.substr(0, n);
	} else {
		name = _conditionString;
	}
	ConditionFactoryContext ctx(parameters);
	ctx.filter = name == FILTER_NAME;
	n = _conditionString.find("(");
	if (n != std::string::npos) {
		const std::size_t r = _conditionString.rfind(")");
		if (r == std::string::npos) {
			setError("syntax error, missing closing brace");
			return ConditionPtr();
		}
		const std::string& inner = _conditionString.substr(n + 1, r - n - 1);
		if (!fillInnerConditions(ctx, inner)) {
			return ConditionPtr();
		}
	} else if (ctx.filter) {
		setError("missing details for Filter condition");
		return ConditionPtr();
	}
	const ConditionPtr& c = _aiFactory.createCondition(name, ctx);
	if (!c) {
		setError("could not create condition for %s", name.c_str());
		return ConditionPtr();
	}
	return c;
}

}

// #include "tree/TreeNodeParser.h"

// #include "tree/TreeNodeImpl.h"


namespace ai {

class AIStateNode;
class AIStateNodeStatic;

class SelectHandler;
class PauseHandler;
class ResetHandler;
class StepHandler;
class ChangeHandler;
class AddNodeHandler;
class DeleteNodeHandler;
class UpdateNodeHandler;
class NopHandler;

/**
 * @brief The server can serialize the state of the @ai{AI} and broadcast it to all connected clients.
 *
 * If you start a server, you can add the @ai{AI} instances to it by calling @ai{addZone()}. If you do so, make
 * sure to remove it when you remove that particular @ai{Zone} instance from your world. You should not do that
 * from different threads. The server should only be managed from one thread.
 *
 * The server will broadcast the world state - that is: It will send out an @ai{AIStateMessage} to all connected
 * clients. If someone selected a particular @ai{AI} instance by sending @ai{AISelectMessage} to the server, it
 * will also broadcast an @ai{AICharacterDetailsMessage} to all connected clients.
 *
 * You can only debug one @ai{Zone} at the same time. The debugging session is shared between all connected clients.
 */
class Server: public INetworkListener {
protected:
	typedef std::unordered_set<Zone*> Zones;
	typedef Zones::const_iterator ZoneConstIter;
	typedef Zones::iterator ZoneIter;
	Zones _zones;
	AIRegistry& _aiRegistry;
	Network _network;
	CharacterId _selectedCharacterId;
	int64_t _time;
	SelectHandler *_selectHandler;
	PauseHandler *_pauseHandler;
	ResetHandler *_resetHandler;
	StepHandler *_stepHandler;
	ChangeHandler *_changeHandler;
	AddNodeHandler *_addNodeHandler;
	DeleteNodeHandler *_deleteNodeHandler;
	UpdateNodeHandler *_updateNodeHandler;
	NopHandler _nopHandler;
	std::atomic_bool _pause;
	// the current active debugging zone
	std::atomic<Zone*> _zone;
	ReadWriteLock _lock = {"server"};
	std::vector<std::string> _names;
	uint32_t _broadcastMask = 0u;

	enum EventType {
		EV_SELECTION,
		EV_STEP,
		EV_UPDATESTATICCHRDETAILS,
		EV_NEWCONNECTION,
		EV_ZONEADD,
		EV_ZONEREMOVE,
		EV_PAUSE,
		EV_RESET,
		EV_SETDEBUG,

		EV_MAX
	};

	struct Event {
		union {
			CharacterId characterId;
			int64_t stepMillis;
			Zone* zone;
			Client* newClient;
			bool pauseState;
		} data;
		std::string strData = "";
		EventType type;
	};
	std::vector<Event> _events;

	void resetSelection();

	void addChildren(const TreeNodePtr& node, std::vector<AIStateNodeStatic>& out) const;
	void addChildren(const TreeNodePtr& node, AIStateNode& parent, const AIPtr& ai) const;

	// only call these from the Server::update method
	void broadcastState(const Zone* zone);
	void broadcastCharacterDetails(const Zone* zone);
	void broadcastStaticCharacterDetails(const Zone* zone);

	void onConnect(Client* client) override;
	void onDisconnect(Client* client) override;

	void handleEvents(Zone* zone, bool pauseState);
	void enqueueEvent(const Event& event);
public:
	Server(AIRegistry& aiRegistry, short port = 10001, const std::string& hostname = "0.0.0.0");
	virtual ~Server();

	/**
	 * @brief Start to listen on the specified port
	 */
	bool start();

	/**
	 * @brief Update the specified node with the given values for the specified @ai{ICharacter} and all the
	 * other characters that are using the same behaviour tree instance
	 *
	 * @param[in] characterId The id of the character where we want to update the specified node
	 * @param[in] nodeId The id of the @ai{TreeNode} to update with the new values
	 * @param[in] name The new name for the node
	 * @param[in] type The new node type (including parameters)
	 * @param[in] condition The new condition (including parameters)
	 *
	 * @see @c TreeNodeParser
	 * @see @c ConditionParser
	 */
	bool updateNode(const CharacterId& characterId, int32_t nodeId, const std::string& name, const std::string& type, const std::string& condition);

	/**
	 * @brief Add a new node with the given values to the specified @ai{ICharacter} and all the
	 * other characters that are using the same behaviour tree instance
	 *
	 * @param[in] characterId The id of the @ai{ICharacter} where we want to add the specified node
	 * @param[in] parentNodeId The id of the @ai{TreeNode} to attach the new @ai{TreeNode} as children
	 * @param[in] name The new name for the node
	 * @param[in] type The new node type (including parameters)
	 * @param[in] condition The new condition (including parameters)
	 *
	 * @see @ai{TreeNodeParser}
	 * @see @ai{ConditionParser}
	 */
	bool addNode(const CharacterId& characterId, int32_t parentNodeId, const std::string& name, const std::string& type, const std::string& condition);

	/**
	 * @brief Delete the specified node from the @ai{ICharacter}'s behaviour tree and all the
	 * other characters that are using the same behaviour tree instance
	 *
	 * @param[in] characterId The id of the @ai{ICharacter} where we want to delete the specified node
	 * @param[in] nodeId The id of the @ai{TreeNode} to delete
	 */
	bool deleteNode(const CharacterId& characterId, int32_t nodeId);

	/**
	 * @brief Adds a new zone to this server instance that can be debugged. The server does not own this pointer
	 * so it also doesn't free it. Every @ai{Zone} that is added here, will be part of the @ai{AINamesMessage}.
	 *
	 * @param zone The @ai{Zone} that should be made available for debugging.
	 *
	 * @note This locks the server instance
	 */
	void addZone(Zone* zone);

	/**
	 * @brief Removes a @ai{Zone} from the server. After this call the given zone is no longer available for debugging
	 * purposes.
	 *
	 * @note This locks the server instance
	 */
	void removeZone(Zone* zone);

	/**
	 * @brief Activate the debugging for this particular zone. And disables the debugging for every other zone
	 *
	 * @note This locks the server instance
	 */
	void setDebug(const std::string& zoneName);

	/**
	 * @brief Resets the @ai{AI} states
	 */
	void reset();

	/**
	 * @brief Select a particular character (resp. @ai{AI} instance) and send detail
	 * information to all the connected clients for this entity.
	 */
	void select(const ClientId& clientId, const CharacterId& id);

	/**
	 * @brief Will pause/unpause the execution of the behaviour trees for all watched @ai{AI} instances.
	 */
	void pause(const ClientId& clientId, bool pause);

	/**
	 * @brief Performs one step of the @ai{AI} in pause mode
	 */
	void step(int64_t stepMillis = 1L);

	/**
	 * @brief call this to update the server - should get called somewhere from your game tick
	 */
	void update(int64_t deltaTime);
};

}

// #include "server/ServerImpl.h"
/**
 * @file
 */


// #include "Server.h"

// #include "SelectHandler.h"
/**
 * @file
 */


// #include "IProtocolHandler.h"

// #include "AISelectMessage.h"

// #include "Server.h"


namespace ai {

class SelectHandler: public ai::IProtocolHandler {
private:
	Server& _server;
public:
	explicit SelectHandler(Server& server) : _server(server) {
	}

	void execute(const ClientId& clientId, const IProtocolMessage& message) override {
		const AISelectMessage& msg = static_cast<const AISelectMessage&>(message);
		_server.select(clientId, msg.getCharacterId());
	}
};

}

// #include "PauseHandler.h"
/**
 * @file
 */


// #include "IProtocolHandler.h"

// #include "AIPauseMessage.h"

// #include "Server.h"


namespace ai {

class Server;

class PauseHandler: public ai::IProtocolHandler {
private:
	Server& _server;
public:
	explicit PauseHandler(Server& server) : _server(server) {
	}

	void execute(const ClientId& clientId, const IProtocolMessage& message) override {
		const AIPauseMessage& msg = static_cast<const AIPauseMessage&>(message);
		_server.pause(clientId, msg.isPause());
	}
};

}

// #include "ResetHandler.h"
/**
 * @file
 */


// #include "IProtocolHandler.h"

// #include "Server.h"


namespace ai {

class ResetHandler: public ai::IProtocolHandler {
private:
	Server& _server;
public:
	explicit ResetHandler(Server& server) : _server(server) {
	}

	void execute(const ClientId& /*clientId*/, const IProtocolMessage& /*message*/) override {
		_server.reset();
	}
};

}

// #include "StepHandler.h"
/**
 * @file
 */


// #include "IProtocolHandler.h"

// #include "Server.h"

// #include "AIStepMessage.h"


namespace ai {

class Server;

class StepHandler: public ai::IProtocolHandler {
private:
	Server& _server;
public:
	explicit StepHandler(Server& server) : _server(server) {
	}

	void execute(const ClientId& /*clientId*/, const IProtocolMessage& message) override {
		const AIStepMessage& msg = static_cast<const AIStepMessage&>(message);
		_server.step(msg.getStepMillis());
	}
};

}

// #include "ChangeHandler.h"
/**
 * @file
 */


// #include "IProtocolHandler.h"

// #include "Server.h"

// #include "AIChangeMessage.h"


namespace ai {

class Server;

class ChangeHandler: public ai::IProtocolHandler {
private:
	Server& _server;
public:
	explicit ChangeHandler(Server& server) : _server(server) {
	}

	void execute(const ClientId& /*clientId*/, const IProtocolMessage& message) override{
		const AIChangeMessage& msg = static_cast<const AIChangeMessage&>(message);
		_server.setDebug(msg.getName());
	}
};

}

// #include "AddNodeHandler.h"
/**
 * @file
 */


// #include "IProtocolHandler.h"

// #include "AIAddNodeMessage.h"


namespace ai {

class Server;

class AddNodeHandler: public ai::IProtocolHandler {
private:
	Server& _server;
public:
	explicit AddNodeHandler(Server& server) : _server(server) {
	}

	void execute(const ClientId& /*clientId*/, const IProtocolMessage& message) override {
		const AIAddNodeMessage& msg = static_cast<const AIAddNodeMessage&>(message);
		if (!_server.addNode(msg.getCharacterId(), msg.getParentNodeId(), msg.getName(), msg.getType(), msg.getCondition())) {
			ai_log_error("Failed to add the new node");
		}
	}
};

}

// #include "DeleteNodeHandler.h"
/**
 * @file
 */


// #include "IProtocolHandler.h"

// #include "AIDeleteNodeMessage.h"

// #include "Server.h"


namespace ai {

class Server;

class DeleteNodeHandler: public ai::IProtocolHandler {
private:
	Server& _server;
public:
	explicit DeleteNodeHandler(Server& server) : _server(server) {
	}

	void execute(const ClientId& /*clientId*/, const IProtocolMessage& message) override {
		const AIDeleteNodeMessage& msg = static_cast<const AIDeleteNodeMessage&>(message);
		if (!_server.deleteNode(msg.getCharacterId(), msg.getNodeId())) {
			ai_log_error("Failed to delete the node %u", msg.getNodeId());
		}
	}
};

}

// #include "UpdateNodeHandler.h"
/**
 * @file
 */


// #include "IProtocolHandler.h"

// #include "AIUpdateNodeMessage.h"

// #include "Server.h"


namespace ai {

class Server;

class UpdateNodeHandler: public ai::IProtocolHandler {
private:
	Server& _server;
public:
	explicit UpdateNodeHandler(Server& server) : _server(server) {
	}

	void execute(const ClientId& /*clientId*/, const IProtocolMessage& message) override {
		const AIUpdateNodeMessage& msg = static_cast<const AIUpdateNodeMessage&>(message);
		if (!_server.updateNode(msg.getCharacterId(), msg.getNodeId(), msg.getName(), msg.getType(), msg.getCondition())) {
			ai_log_error("Failed to update the node %u", msg.getNodeId());
		}
	}
};

}


namespace ai {

namespace {
const int SV_BROADCAST_CHRDETAILS = 1 << 0;
const int SV_BROADCAST_STATE      = 1 << 1;
}

inline Server::Server(AIRegistry& aiRegistry, short port, const std::string& hostname) :
		_aiRegistry(aiRegistry), _network(port, hostname), _selectedCharacterId(AI_NOTHING_SELECTED), _time(0L),
		_selectHandler(new SelectHandler(*this)), _pauseHandler(new PauseHandler(*this)), _resetHandler(new ResetHandler(*this)),
		_stepHandler(new StepHandler(*this)), _changeHandler(new ChangeHandler(*this)), _addNodeHandler(new AddNodeHandler(*this)),
		_deleteNodeHandler(new DeleteNodeHandler(*this)), _updateNodeHandler(new UpdateNodeHandler(*this)), _pause(false), _zone(nullptr) {
	_network.addListener(this);
	ProtocolHandlerRegistry& r = ai::ProtocolHandlerRegistry::get();
	r.registerHandler(ai::PROTO_SELECT, _selectHandler);
	r.registerHandler(ai::PROTO_PAUSE, _pauseHandler);
	r.registerHandler(ai::PROTO_RESET, _resetHandler);
	r.registerHandler(ai::PROTO_STEP, _stepHandler);
	r.registerHandler(ai::PROTO_PING, &_nopHandler);
	r.registerHandler(ai::PROTO_CHANGE, _changeHandler);
	r.registerHandler(ai::PROTO_ADDNODE, _addNodeHandler);
	r.registerHandler(ai::PROTO_DELETENODE, _deleteNodeHandler);
	r.registerHandler(ai::PROTO_UPDATENODE, _updateNodeHandler);
}

inline Server::~Server() {
	delete _selectHandler;
	delete _pauseHandler;
	delete _resetHandler;
	delete _stepHandler;
	delete _changeHandler;
	delete _addNodeHandler;
	delete _deleteNodeHandler;
	delete _updateNodeHandler;
	_network.removeListener(this);
}

inline void Server::enqueueEvent(const Event& event) {
	ScopedWriteLock scopedLock(_lock);
	_events.push_back(event);
}

inline void Server::onConnect(Client* client) {
	Event event;
	event.type = EV_NEWCONNECTION;
	event.data.newClient = client;
	enqueueEvent(event);
}

inline void Server::onDisconnect(Client* /*client*/) {
	ai_log("remote debugger disconnect (%i)", _network.getConnectedClients());
	Zone* zone = _zone;
	if (zone == nullptr) {
		return;
	}

	// if there are still connected clients left, don't disable the debug mode for the zone
	if (_network.getConnectedClients() > 0) {
		return;
	}

	zone->setDebug(false);
	if (_zone.compare_exchange_strong(zone, nullptr)) {
		// restore the zone state if no player is left for debugging
		const bool pauseState = _pause;
		if (pauseState) {
			pause(0, false);
		}

		// only if noone else already started a new debug session
		resetSelection();
	}
}

inline bool Server::start() {
	return _network.start();
}

inline void Server::addChildren(const TreeNodePtr& node, std::vector<AIStateNodeStatic>& out) const {
	for (const TreeNodePtr& childNode : node->getChildren()) {
		const int32_t nodeId = childNode->getId();
		out.push_back(AIStateNodeStatic(nodeId, childNode->getName(), childNode->getType(), childNode->getParameters(), childNode->getCondition()->getName(), childNode->getCondition()->getParameters()));
		addChildren(childNode, out);
	}
}

inline void Server::addChildren(const TreeNodePtr& node, AIStateNode& parent, const AIPtr& ai) const {
	const TreeNodes& children = node->getChildren();
	std::vector<bool> currentlyRunning(children.size());
	node->getRunningChildren(ai, currentlyRunning);
	const int64_t aiTime = ai->_time;
	const std::size_t length = children.size();
	for (std::size_t i = 0; i < length; ++i) {
		const TreeNodePtr& childNode = children[i];
		const int32_t id = childNode->getId();
		const ConditionPtr& condition = childNode->getCondition();
		const std::string conditionStr = condition ? condition->getNameWithConditions(ai) : "";
		const int64_t lastRun = childNode->getLastExecMillis(ai);
		const int64_t delta = lastRun == -1 ? -1 : aiTime - lastRun;
		AIStateNode child(id, conditionStr, delta, childNode->getLastStatus(ai), currentlyRunning[i]);
		addChildren(childNode, child, ai);
		parent.addChildren(child);
	}
}

inline void Server::broadcastState(const Zone* zone) {
	_broadcastMask |= SV_BROADCAST_STATE;
	AIStateMessage msg;
	auto func = [&] (const AIPtr& ai) {
		const ICharacterPtr& chr = ai->getCharacter();
		const AIStateWorld b(chr->getId(), chr->getPosition(), chr->getOrientation(), chr->getAttributes());
		msg.addState(b);
	};
	zone->execute(func);
	_network.broadcast(msg);
}

inline void Server::broadcastStaticCharacterDetails(const Zone* zone) {
	const CharacterId id = _selectedCharacterId;
	if (id == AI_NOTHING_SELECTED) {
		return;
	}

	static const auto func = [&] (const AIPtr& ai) {
		if (!ai) {
			return false;
		}
		std::vector<AIStateNodeStatic> nodeStaticData;
		const TreeNodePtr& node = ai->getBehaviour();
		const int32_t nodeId = node->getId();
		nodeStaticData.push_back(AIStateNodeStatic(nodeId, node->getName(), node->getType(), node->getParameters(), node->getCondition()->getName(), node->getCondition()->getParameters()));
		addChildren(node, nodeStaticData);

		const AICharacterStaticMessage msgStatic(ai->getId(), nodeStaticData);
		_network.broadcast(msgStatic);
		return true;
	};
	if (!zone->execute(id, func)) {
		resetSelection();
	}
}

inline void Server::broadcastCharacterDetails(const Zone* zone) {
	_broadcastMask |= SV_BROADCAST_CHRDETAILS;
	const CharacterId id = _selectedCharacterId;
	if (id == AI_NOTHING_SELECTED) {
		return;
	}

	static const auto func = [&] (const AIPtr& ai) {
		if (!ai) {
			return false;
		}
		const TreeNodePtr& node = ai->getBehaviour();
		const int32_t nodeId = node->getId();
		const ConditionPtr& condition = node->getCondition();
		const std::string conditionStr = condition ? condition->getNameWithConditions(ai) : "";
		AIStateNode root(nodeId, conditionStr, _time - node->getLastExecMillis(ai), node->getLastStatus(ai), true);
		addChildren(node, root, ai);

		AIStateAggro aggro;
		const ai::AggroMgr::Entries& entries = ai->getAggroMgr().getEntries();
		aggro.reserve(entries.size());
		for (const Entry& e : entries) {
			aggro.addAggro(AIStateAggroEntry(e.getCharacterId(), e.getAggro()));
		}

		const AICharacterDetailsMessage msg(ai->getId(), aggro, root);
		_network.broadcast(msg);
		return true;
	};
	if (!zone->execute(id, func)) {
		resetSelection();
	}
}

inline void Server::handleEvents(Zone* zone, bool pauseState) {
	std::vector<Event> events;
	{
		ScopedReadLock scopedLock(_lock);
		events = std::move(_events);
		_events.clear();
	}
	for (Event& event : events) {
		switch (event.type) {
		case EV_SELECTION: {
			if (zone == nullptr || event.data.characterId == AI_NOTHING_SELECTED) {
				resetSelection();
			} else {
				_selectedCharacterId = event.data.characterId;
				broadcastStaticCharacterDetails(zone);
				if (pauseState) {
					broadcastState(zone);
					broadcastCharacterDetails(zone);
				}
			}
			break;
		}
		case EV_STEP: {
			const int64_t queuedStepMillis = event.data.stepMillis;
			auto func = [=] (const AIPtr& ai) {
				if (!ai->isPause())
					return;
				ai->setPause(false);
				ai->update(queuedStepMillis, true);
				ai->getBehaviour()->execute(ai, queuedStepMillis);
				ai->setPause(true);
			};
			zone->executeParallel(func);
			broadcastState(zone);
			broadcastCharacterDetails(zone);
			break;
		}
		case EV_RESET: {
			static auto func = [] (const AIPtr& ai) {
				ai->getBehaviour()->resetState(ai);
			};
			event.data.zone->executeParallel(func);
			break;
		}
		case EV_PAUSE: {
			const bool newPauseState = event.data.pauseState;
			_pause = newPauseState;
			if (zone != nullptr) {
				auto func = [=] (const AIPtr& ai) {
					ai->setPause(newPauseState);
				};
				zone->executeParallel(func);
				_network.broadcast(AIPauseMessage(newPauseState));
				// send the last time the most recent state until we unpause
				if (newPauseState) {
					broadcastState(zone);
					broadcastCharacterDetails(zone);
				}
			}
			break;
		}
		case EV_UPDATESTATICCHRDETAILS: {
			broadcastStaticCharacterDetails(event.data.zone);
			break;
		}
		case EV_NEWCONNECTION: {
			_network.sendToClient(event.data.newClient, AIPauseMessage(pauseState));
			_network.sendToClient(event.data.newClient, AINamesMessage(_names));
			ai_log("new remote debugger connection (%i)", _network.getConnectedClients());
			break;
		}
		case EV_ZONEADD: {
			if (!_zones.insert(event.data.zone).second) {
				return;
			}
			_names.clear();
			for (const Zone* z : _zones) {
				_names.push_back(z->getName());
			}
			_network.broadcast(AINamesMessage(_names));
			break;
		}
		case EV_ZONEREMOVE: {
			_zone.compare_exchange_strong(event.data.zone, nullptr);
			if (_zones.erase(event.data.zone) != 1) {
				return;
			}
			_names.clear();
			for (const Zone* z : _zones) {
				_names.push_back(z->getName());
			}
			_network.broadcast(AINamesMessage(_names));
			break;
		}
		case EV_SETDEBUG: {
			if (_pause) {
				pause(0, false);
			}

			Zone* nullzone = nullptr;
			_zone = nullzone;
			resetSelection();

			for (Zone* z : _zones) {
				const bool debug = z->getName() == event.strData;
				if (!debug) {
					continue;
				}
				if (_zone.compare_exchange_strong(nullzone, z)) {
					z->setDebug(debug);
				}
			}

			break;
		}
		case EV_MAX:
			break;
		}
	}
}

inline void Server::resetSelection() {
	_selectedCharacterId = AI_NOTHING_SELECTED;
}

inline bool Server::updateNode(const CharacterId& characterId, int32_t nodeId, const std::string& name, const std::string& type, const std::string& condition) {
	Zone* zone = _zone;
	if (zone == nullptr) {
		return false;
	}
	const AIPtr& ai = zone->getAI(characterId);
	const TreeNodePtr& node = ai->getBehaviour()->getId() == nodeId ? ai->getBehaviour() : ai->getBehaviour()->getChild(nodeId);
	if (!node) {
		return false;
	}
	ConditionParser conditionParser(_aiRegistry, condition);
	const ConditionPtr& conditionPtr = conditionParser.getCondition();
	if (!conditionPtr) {
		ai_log_error("Failed to parse the condition '%s'", condition.c_str());
		return false;
	}
	TreeNodeParser treeNodeParser(_aiRegistry, type);
	TreeNodePtr newNode = treeNodeParser.getTreeNode(name);
	if (!newNode) {
		ai_log_error("Failed to parse the node '%s'", type.c_str());
		return false;
	}
	newNode->setCondition(conditionPtr);
	for (auto& child : node->getChildren()) {
		newNode->addChild(child);
	}

	const TreeNodePtr& root = ai->getBehaviour();
	if (node == root) {
		ai->setBehaviour(newNode);
	} else {
		const TreeNodePtr& parent = root->getParent(root, nodeId);
		if (!parent) {
			ai_log_error("No parent for non-root node '%i'", nodeId);
			return false;
		}
		parent->replaceChild(nodeId, newNode);
	}

	Event event;
	event.type = EV_UPDATESTATICCHRDETAILS;
	event.data.zone = zone;
	enqueueEvent(event);
	return true;
}

inline bool Server::addNode(const CharacterId& characterId, int32_t parentNodeId, const std::string& name, const std::string& type, const std::string& condition) {
	Zone* zone = _zone;
	if (zone == nullptr) {
		return false;
	}
	const AIPtr& ai = zone->getAI(characterId);
	TreeNodePtr node = ai->getBehaviour();
	if (node->getId() != parentNodeId) {
		node = node->getChild(parentNodeId);
	}
	if (!node) {
		return false;
	}
	ConditionParser conditionParser(_aiRegistry, condition);
	const ConditionPtr& conditionPtr = conditionParser.getCondition();
	if (!conditionPtr) {
		ai_log_error("Failed to parse the condition '%s'", condition.c_str());
		return false;
	}
	TreeNodeParser treeNodeParser(_aiRegistry, type);
	TreeNodePtr newNode = treeNodeParser.getTreeNode(name);
	if (!newNode) {
		ai_log_error("Failed to parse the node '%s'", type.c_str());;
		return false;
	}
	newNode->setCondition(conditionPtr);
	if (!node->addChild(newNode)) {
		return false;
	}

	Event event;
	event.type = EV_UPDATESTATICCHRDETAILS;
	event.data.zone = zone;
	enqueueEvent(event);
	return true;
}

inline bool Server::deleteNode(const CharacterId& characterId, int32_t nodeId) {
	Zone* zone = _zone;
	if (zone == nullptr) {
		return false;
	}
	const AIPtr& ai = zone->getAI(characterId);
	// don't delete the root
	const TreeNodePtr& root = ai->getBehaviour();
	if (root->getId() == nodeId) {
		return false;
	}

	const TreeNodePtr& parent = root->getParent(root, nodeId);
	if (!parent) {
		ai_log_error("No parent for non-root node '%i'", nodeId);
		return false;
	}
	parent->replaceChild(nodeId, TreeNodePtr());
	Event event;
	event.type = EV_UPDATESTATICCHRDETAILS;
	event.data.zone = zone;
	enqueueEvent(event);
	return true;
}

inline void Server::addZone(Zone* zone) {
	Event event;
	event.type = EV_ZONEADD;
	event.data.zone = zone;
	enqueueEvent(event);
}

inline void Server::removeZone(Zone* zone) {
	Event event;
	event.type = EV_ZONEREMOVE;
	event.data.zone = zone;
	enqueueEvent(event);
}

inline void Server::setDebug(const std::string& zoneName) {
	Event event;
	event.type = EV_SETDEBUG;
	event.strData = zoneName;
	enqueueEvent(event);
}

inline void Server::reset() {
	Zone* zone = _zone;
	if (zone == nullptr) {
		return;
	}
	Event event;
	event.type = EV_RESET;
	event.data.zone = zone;
	enqueueEvent(event);
}

inline void Server::select(const ClientId& /*clientId*/, const CharacterId& id) {
	Event event;
	event.type = EV_SELECTION;
	event.data.characterId = id;
	enqueueEvent(event);
}

inline void Server::pause(const ClientId& /*clientId*/, bool state) {
	Event event;
	event.type = EV_PAUSE;
	event.data.pauseState = state;
	enqueueEvent(event);
}

inline void Server::step(int64_t stepMillis) {
	Event event;
	event.type = EV_STEP;
	event.data.stepMillis = stepMillis;
	enqueueEvent(event);
}

inline void Server::update(int64_t deltaTime) {
	_time += deltaTime;
	const int clients = _network.getConnectedClients();
	Zone* zone = _zone;
	bool pauseState = _pause;
	_broadcastMask = 0u;

	handleEvents(zone, pauseState);

	if (clients > 0 && zone != nullptr) {
		if (!pauseState) {
			if ((_broadcastMask & SV_BROADCAST_STATE) == 0) {
				broadcastState(zone);
			}
			if ((_broadcastMask & SV_BROADCAST_CHRDETAILS) == 0) {
				broadcastCharacterDetails(zone);
			}
		}
	} else if (pauseState) {
		pause(1, false);
		resetSelection();
	}
	_network.update(deltaTime);
}

}

// #include "server/IProtocolHandler.h"

// #include "server/ProtocolHandlerRegistry.h"

// #include "server/ProtocolMessageFactory.h"

// #include "server/AICharacterDetailsMessage.h"

// #include "server/AICharacterStaticMessage.h"

// #include "server/AIPauseMessage.h"

// #include "server/AIStepMessage.h"

// #include "server/AISelectMessage.h"

// #include "server/AIStateMessage.h"

// #include "server/AINamesMessage.h"

// #include "server/AIChangeMessage.h"

// #include "server/AIAddNodeMessage.h"

// #include "server/AIDeleteNodeMessage.h"

// #include "server/AIUpdateNodeMessage.h"


// #include "zone/Zone.h"


// #include "conditions/And.h"

// #include "conditions/ICondition.h"

// #include "conditions/ConditionParser.h"

// #include "conditions/False.h"

// #include "conditions/HasEnemies.h"

// #include "conditions/IsGroupLeader.h"

// #include "conditions/IsInGroup.h"

// #include "conditions/Not.h"

// #include "conditions/Or.h"

// #include "conditions/True.h"


// #include "filter/IFilter.h"

// #include "filter/SelectEmpty.h"

// #include "filter/SelectGroupLeader.h"

// #include "filter/SelectGroupMembers.h"

// #include "filter/SelectHighestAggro.h"

// #include "filter/SelectZone.h"

// #include "filter/Union.h"

// #include "filter/Intersection.h"

// #include "filter/Last.h"

// #include "filter/First.h"

// #include "filter/Random.h"

// #include "filter/Difference.h"

// #include "filter/Complement.h"

// #include "filter/SelectAll.h"


#ifdef AI_INCLUDE_LUA
// #include "tree/loaders/lua/LUATreeLoader.h"
/**
 * @file
 * @defgroup LUA
 * @{
 * @code
 * function idle (parentnode)
 * 	local prio = parentnode:addNode("PrioritySelector", "walkuncrowded")
 * 		prio:addNode("Steer(Wander)", "wanderfreely")
 * end
 *
 * function wolf ()
 * 	local name = "ANIMAL_WOLF"
 * 	local rootNode = AI.createTree(name):createRoot("PrioritySelector", name)
 * 	local parallel = rootnode:addNode("Parallel", "hunt")
 * 	parallel:setCondition("Not(IsOnCooldown{HUNT})")
 * 		parallel:addNode("Steer(SelectionSeek)", "follow"):setCondition("Filter(SelectEntitiesOfType{ANIMAL_RABBIT})")
 * 		parallel:addNode("AttackOnSelection", "attack"):setCondition("IsCloseToSelection{1}")
 * 		parallel:addNode("SetPointOfInterest", "setpoi"):setCondition("IsCloseToSelection{1}")
 * 		parallel:addNode("TriggerCooldown{HUNT}", "increasecooldown"):setCondition("Not(IsSelectionAlive)")
 * 	idle(rootNode)
 * end
 *
 * function rabbit ()
 * 	local name = "ANIMAL_RABBIT"
 * 	local rootNode = AI.createTree(name):createRoot("PrioritySelector", name)
 * 	rootnode:addNode("Steer(SelectionFlee)", "fleefromhunter"):setCondition("And(Filter(SelectEntitiesOfTypes{ANIMAL_WOLF}),IsCloseToSelection{10})")
 * 	idle(rootNode)
 * end
 *
 * function init ()
 * 	wolf()
 * 	rabbit()
 * end
 * @endcode
 */


// #include "tree/loaders/ITreeLoader.h"

// #include "tree/TreeNodeImpl.h"

// #include "AIRegistry.h"

// #include "conditions/ConditionParser.h"

// #include "tree/TreeNodeParser.h"

// #include "common/LUA.h"
/**
 * @file
 * @ingroup LUA
 */


// #include "common/Types.h"

// #include "common/NonCopyable.h"

// TODO: redefine these
//lua_writestring
//lua_writeline
//lua_writestringerror
#include <lua.hpp>
#include <string>
#include <map>

namespace ai {

namespace lua {

namespace {
const std::string META_PREFIX = "META_";

int panicCB(lua_State *L) {
	ai_log_error("Lua panic. Error message: %s", (lua_isnil(L, -1) ? "" : lua_tostring(L, -1)));
	return 0;
}

}

class LUAType {
private:
	lua_State* _state;
public:
	LUAType(lua_State* state, const std::string& name) :
			_state(state) {
		const std::string metaTable = META_PREFIX + name;
		luaL_newmetatable(_state, metaTable.c_str());
		lua_pushvalue(_state, -1);
		lua_setfield(_state, -2, "__index");
	}

	// only non-capturing lambdas can be converted to function pointers
	template<class FUNC>
	void addFunction(const std::string& name, FUNC&& func) {
		lua_pushcfunction(_state, func);
		lua_setfield(_state, -2, name.c_str());
	}

	void addValue(const char* name, const char* value) {
		lua_pushstring(_state, value);
		lua_setfield(_state, -2, name);
	}
};

/**
 * @par
 * You can find the reference lua manual at http://www.lua.org/manual/5.3/
 *
 * @par -1 and -2 are pseudo indexes, they count backwards:
 * @li -1 is top
 * @li 1 is bottom
 * @li -2 is under the top
 */
class LUA : public NonCopyable {
private:
	lua_State *_state;
	std::string _error;
	bool _destroy;

public:
	explicit LUA(lua_State *state) :
			_state(state), _destroy(false) {
	}

	explicit LUA(bool debug = false) :
			_state(luaL_newstate()), _destroy(true) {
		luaL_openlibs(_state);
		lua_atpanic(_state, panicCB);
		lua_gc(_state, LUA_GCSTOP, 0);
	}

	~LUA () {
		if (_destroy) {
			//const int bytes = lua_gc(_state, LUA_GCCOUNT, 0) * 1024 + lua_gc(_state, LUA_GCCOUNTB, 0);
			lua_gc(_state, LUA_GCRESTART, 0);
			lua_close(_state);
		}
		_state = nullptr;
	}

	inline lua_State* getState () const {
		return _state;
	}

	template<class T>
	inline T* newGlobalData (const std::string& prefix, T *userData) const {
		lua_pushlightuserdata(_state, userData);
		lua_setglobal(_state, prefix.c_str());
		return userData;
	}

	template<class T>
	static T* getGlobalData(lua_State *L, const std::string& prefix) {
		lua_getglobal(L, prefix.c_str());
		T* data = (T*) lua_touserdata(L, -1);
		lua_pop(L, 1);
		return data;
	}

	template<class T>
	static T* newUserdata(lua_State *L, const std::string& prefix, T* data) {
		T ** udata = (T **) lua_newuserdata(L, sizeof(T *));
		const std::string name = META_PREFIX + prefix;
		luaL_getmetatable(L, name.c_str());
		lua_setmetatable(L, -2);
		*udata = data;
		return data;
	}

	template<class T>
	static T* getUserData (lua_State *L, int n, const std::string& prefix) {
		const std::string name = META_PREFIX + prefix;
		return *(T **) luaL_checkudata(L, n, name.c_str());
	}

	static int returnError (lua_State *L, const std::string& error) {
		ai_log_error("LUA error: '%s'", error.c_str());
		return luaL_error(L, "%s", error.c_str());
	}

	void reg (const std::string& prefix, luaL_Reg* funcs) {
		const std::string metaTableName = META_PREFIX + prefix;
		luaL_newmetatable(_state, metaTableName.c_str());
		luaL_setfuncs(_state, funcs, 0);
		lua_pushvalue(_state, -1);
		lua_setfield(_state, -1, "__index");
		lua_setglobal(_state, prefix.c_str());
	}

	LUAType registerType (const std::string& name) {
		return LUAType(_state, name);
	}

	void setError (const std::string& errorStr) {
		_error = errorStr;
	}

	const std::string& error () const {
		return _error;
	}

	bool load (const std::string &luaString) {
		return load(luaString.c_str(), luaString.length());
	}

	bool load (const char *luaString, size_t len) {
		if (luaL_loadbufferx(_state, luaString, len, "", nullptr) || lua_pcall(_state, 0, LUA_MULTRET, 0)) {
			setError(lua_tostring(_state, -1));
			lua_pop(_state, 1);
			return false;
		}

		return true;
	}

	/**
	 * @param[in] function function to be called
	 */
	bool execute (const std::string &function, int returnValues = 0) {
		lua_getglobal(_state, function.c_str());
		const int ret = lua_pcall(_state, 0, returnValues, 0);
		if (ret != 0) {
			setError(lua_tostring(_state, -1));
			return false;
		}

		return true;
	}
};

}

}


namespace ai {

/**
 * @brief Implementation of @c ITreeLoader that gets its data from a lua script
 */
class LUATreeLoader: public ai::ITreeLoader {
private:
	class LUATree;

	class LUACondition {
	private:
		const ConditionPtr& _condition;
	public:
		LUACondition(const ConditionPtr& condition) :
				_condition(condition) {
		}

		inline const ConditionPtr& getCondition() const {
			return _condition;
		}
	};

	class LUANode {
	private:
		TreeNodePtr _node;
		LUACondition *_condition;
		std::vector<LUANode*> _children;
		LUATree *_tree;
		const IAIFactory& _aiFactory;
	public:
		LUANode(const TreeNodePtr& node, LUATree *tree, const IAIFactory& aiFactory) :
				_node(node), _condition(nullptr), _tree(tree), _aiFactory(aiFactory) {
		}

		~LUANode() {
			delete _condition;
		}

		inline const IAIFactory& getAIFactory() const{
			return _aiFactory;
		}

		inline TreeNodePtr& getTreeNode() {
			return _node;
		}

		inline const TreeNodePtr& getTreeNode() const {
			return _node;
		}

		inline void setCondition(LUACondition *condition) {
			_condition = condition;
			_node->setCondition(condition->getCondition());
		}

		inline const std::vector<LUANode*>& getChildren() const {
			return _children;
		}

		inline LUACondition* getCondition() const {
			return _condition;
		}

		LUANode* addChild (const std::string& nodeType, const TreeNodeFactoryContext& ctx) {
			TreeNodeParser parser(_aiFactory, nodeType);
			const TreeNodePtr& child = parser.getTreeNode(ctx.name);
			if (!child) {
				return nullptr;
			}
			LUANode *node = new LUANode(child, _tree, _aiFactory);
			_children.push_back(node);
			_node->addChild(child);
			return node;
		}
	};

	class LUATree {
	private:
		std::string _name;
		LUATreeLoader* _ctx;
		LUANode* _root;
	public:
		LUATree(const std::string& name, LUATreeLoader* ctx) :
				_name(name), _ctx(ctx), _root(nullptr) {
		}

		inline const IAIFactory& getAIFactory() const{
			return _ctx->getAIFactory();
		}

		inline bool setRoot(LUANode* root) {
			if (_ctx->addTree(_name, root->getTreeNode())) {
				_root = root;
				return true;
			}

			return false;
		}

		inline const std::string& getName() const {
			return _name;
		}

		inline LUANode* getRoot() const {
			return _root;
		}
	};

	static LUATreeLoader* luaGetContext(lua_State * l) {
		return lua::LUA::getGlobalData<LUATreeLoader>(l, "Loader");
	}

	static LUATree* luaGetTreeContext(lua_State * l, int n) {
		return lua::LUA::getUserData<LUATree>(l, n, "Tree");
	}

	static LUANode* luaGetNodeContext(lua_State * l, int n) {
		return lua::LUA::getUserData<LUANode>(l, n, "Node");
	}

	#if 0
	static LUACondition* luaGetConditionContext(lua_State * l, int n) {
		return lua::LUA::getUserData<LUACondition>(l, n, "Condition");
	}
	#endif

	static int luaMain_CreateTree(lua_State * l) {
		LUATreeLoader *ctx = luaGetContext(l);
		const std::string name = luaL_checkstring(l, 1);
		lua::LUA::newUserdata<LUATree>(l, "Tree", new LUATree(name, ctx));
		return 1;
	}

	static int luaTree_GC(lua_State * l) {
		LUATree *tree = luaGetTreeContext(l, 1);
		delete tree;
		return 0;
	}

	static int luaTree_ToString(lua_State * l) {
		const LUATree *tree = luaGetTreeContext(l, 1);
		lua_pushfstring(l, "tree: %s [%s]", tree->getName().c_str(), tree->getRoot() ? "root" : "no root");
		return 1;
	}

	static int luaTree_GetName(lua_State * l) {
		const LUATree *tree = luaGetTreeContext(l, 1);
		lua_pushstring(l, tree->getName().c_str());
		return 1;
	}

	static int luaNode_GC(lua_State * l) {
		LUANode *node = luaGetNodeContext(l, 1);
		delete node;
		return 0;
	}

	static int luaNode_ToString(lua_State * l) {
		const LUANode *node = luaGetNodeContext(l, 1);
		const LUACondition* condition = node->getCondition();
		lua_pushfstring(l, "node: %d children [condition: %s]", (int)node->getChildren().size(),
				condition ? condition->getCondition()->getName().c_str() : "no condition");
		return 1;
	}

	static int luaNode_GetName(lua_State * l) {
		const LUANode *node = luaGetNodeContext(l, 1);
		lua_pushstring(l, node->getTreeNode()->getName().c_str());
		return 1;
	}

	static int luaTree_CreateRoot(lua_State * l) {
		LUATree *ctx = luaGetTreeContext(l, 1);
		const std::string id = luaL_checkstring(l, 2);
		const std::string name = luaL_checkstring(l, 3);

		TreeNodeParser parser(ctx->getAIFactory(), id);
		const TreeNodePtr& node = parser.getTreeNode(name);
		if (!node) {
			return lua::LUA::returnError(l, "Could not create a node for " + id);
		}

		LUANode* udata = lua::LUA::newUserdata<LUANode>(l, "Node", new LUANode(node, ctx, ctx->getAIFactory()));
		if (!ctx->setRoot(udata)) {
			LUATreeLoader *loader = luaGetContext(l);
			return lua::LUA::returnError(l, loader->getError());
		}
		return 1;
	}

	static int luaNode_AddNode(lua_State * l) {
		LUANode *node = luaGetNodeContext(l, 1);
		const std::string id = luaL_checkstring(l, 2);
		const std::string name = luaL_checkstring(l, 3);

		TreeNodeFactoryContext factoryCtx(name, "", True::get());
		LUANode* udata = lua::LUA::newUserdata<LUANode>(l, "Node", node->addChild(id, factoryCtx));
		if (udata == nullptr) {
			return lua::LUA::returnError(l, "Could not create a node for " + id);
		}
		return 1;
	}

	static int luaNode_SetCondition(lua_State * l) {
		LUATreeLoader *ctx = luaGetContext(l);
		LUANode *node = luaGetNodeContext(l, 1);
		const std::string conditionExpression = luaL_checkstring(l, 2);

		ConditionParser parser(ctx->getAIFactory(), conditionExpression);
		const ConditionPtr& condition = parser.getCondition();
		if (!condition) {
			return lua::LUA::returnError(l, "Could not create a condition for " + conditionExpression + ": " + parser.getError());
		}

		LUACondition* udata = lua::LUA::newUserdata<LUACondition>(l, "Condition", new LUACondition(condition));
		node->setCondition(udata);
		return 1;
	}

public:
	LUATreeLoader(const IAIFactory& aiFactory) :
			ITreeLoader(aiFactory) {
	}

	/**
	 * @brief this will initialize the loader once with all the defined behaviours from the given lua string.
	 */
	bool init(const std::string& luaString) {
		shutdown();

		lua::LUA lua;
		luaL_Reg createTree = { "createTree", luaMain_CreateTree };
		luaL_Reg eof = { nullptr, nullptr };
		luaL_Reg funcs[] = { createTree, eof };

		lua::LUAType tree = lua.registerType("Tree");
		tree.addFunction("createRoot", luaTree_CreateRoot);
		tree.addFunction("getName", luaTree_GetName);
		tree.addFunction("__gc", luaTree_GC);
		tree.addFunction("__tostring", luaTree_ToString);

		lua::LUAType node = lua.registerType("Node");
		node.addFunction("addNode", luaNode_AddNode);
		node.addFunction("getName", luaNode_GetName);
		node.addFunction("setCondition", luaNode_SetCondition);
		node.addFunction("__gc", luaNode_GC);
		node.addFunction("__tostring", luaNode_ToString);

		lua.reg("AI", funcs);

		if (!lua.load(luaString)) {
			setError("%s", lua.error().c_str());
			return false;
		}

		// loads all the trees
		lua.newGlobalData<LUATreeLoader>("Loader", this);
		if (!lua.execute("init")) {
			setError("%s", lua.error().c_str());
			return false;
		}

		bool empty;
		{
			ScopedReadLock scopedLock(_lock);
			empty = _treeMap.empty();
		}
		if (empty) {
			setError("No behaviour trees specified");
			return false;
		}
		return true;
	}
};

}

/**
 * @}
 */

// #include "LUAAIRegistry.h"
/***
 * @file LUAAIRegistry.h
 * @ingroup LUA
 */


// #include "AIRegistry.h"

// #include "LUAFunctions.h"
/***
 * @file LUAFunctions.h
 * @ingroup LUA
 */


// #include "common/LUA.h"


namespace ai {

struct luaAI_AI {
	AIPtr ai;
};

struct luaAI_ICharacter {
	ICharacterPtr character;
};

static inline const char *luaAI_metaai() {
	return "__meta_ai";
}

static inline const char *luaAI_metazone() {
	return "__meta_zone";
}

static inline const char* luaAI_metaaggromgr() {
	return "__meta_aggromgr";
}

static inline const char* luaAI_metaregistry() {
	return "__meta_registry";
}

static inline const char* luaAI_metagroupmgr() {
	return "__meta_groupmgr";
}

static inline const char* luaAI_metacharacter() {
	return "__meta_character";
}

static inline const char* luaAI_metavec() {
	return "__meta_vec";
}

static void luaAI_registerfuncs(lua_State* s, const luaL_Reg* funcs, const char *name) {
	luaL_newmetatable(s, name);
	// assign the metatable to __index
	lua_pushvalue(s, -1);
	lua_setfield(s, -2, "__index");
	luaL_setfuncs(s, funcs, 0);
}

static void luaAI_setupmetatable(lua_State* s, const std::string& type, const luaL_Reg *funcs, const std::string& name) {
	const std::string& metaFull = "__meta_" + name + "_" + type;
	// make global
	lua_setfield(s, LUA_REGISTRYINDEX, metaFull.c_str());
	// put back onto stack
	lua_getfield(s, LUA_REGISTRYINDEX, metaFull.c_str());

	// setup meta table - create a new one manually, otherwise we aren't
	// able to override the particular function on a per instance base. Also
	// this 'metatable' must not be in the global registry.
	lua_createtable(s, 0, 2);
	lua_pushvalue(s, -1);
	lua_setfield(s, -2, "__index");
	lua_pushstring(s, name.c_str());
	lua_setfield(s, -2, "__name");
	lua_pushstring(s, type.c_str());
	lua_setfield(s, -2, "type");
	luaL_setfuncs(s, funcs, 0);
	lua_setmetatable(s, -2);
}

static int luaAI_newindex(lua_State* s) {
	// -3 is userdata
	lua_getmetatable(s, -3);
	// -3 is now the field string
	const char *field = luaL_checkstring(s, -3);
	// push -2 to -1 (the value)
	lua_pushvalue(s, -2);
	// set the value into the field
	lua_setfield(s, -2, field);
	lua_pop(s, 1);
	return 0;
}

template<class T>
static inline T luaAI_getudata(lua_State* s, int n, const char *name) {
	void* dataVoid = luaL_checkudata(s, n, name);
	if (dataVoid == nullptr) {
		std::string error(name);
		error.append(" userdata must not be null");
		luaL_argcheck(s, dataVoid != nullptr, n, error.c_str());
	}
	return (T) dataVoid;
}

template<class T>
static inline T* luaAI_newuserdata(lua_State* s, const T& data) {
	T* udata = (T*) lua_newuserdata(s, sizeof(T));
	*udata = data;
	return udata;
}

static void luaAI_globalpointer(lua_State* s, void* pointer, const char *name) {
	lua_pushlightuserdata(s, pointer);
	lua_setglobal(s, name);
}

static int luaAI_assignmetatable(lua_State* s, const char *name) {
	luaL_getmetatable(s, name);
#if AI_LUA_SANTITY
	if (!lua_istable(s, -1)) {
		ai_log_error("LUA: metatable for %s doesn't exist", name);
		return 0;
	}
#endif
	lua_setmetatable(s, -2);
	return 1;
}

template<class T>
static inline int luaAI_pushudata(lua_State* s, const T& data, const char *name) {
	luaAI_newuserdata<T>(s, data);
	return luaAI_assignmetatable(s, name);
}

template<class T>
static T* luaAI_getlightuserdata(lua_State *s, const char *name) {
	lua_getglobal(s, name);
	if (lua_isnil(s, -1)) {
		return nullptr;
	}
	T* data = (T*) lua_touserdata(s, -1);
	lua_pop(s, 1);
	return data;
}

static luaAI_AI* luaAI_toai(lua_State *s, int n) {
	luaAI_AI* ai = luaAI_getudata<luaAI_AI*>(s, n, luaAI_metaai());
	if (!ai->ai) {
		luaL_error(s, "AI is already destroyed");
	}
	return ai;
}

static luaAI_ICharacter* luaAI_tocharacter(lua_State *s, int n) {
	luaAI_ICharacter* chr = luaAI_getudata<luaAI_ICharacter*>(s, n, luaAI_metacharacter());
	if (!chr->character) {
		luaL_error(s, "ICharacter is already destroyed");
	}
	return chr;
}

static Zone* luaAI_tozone(lua_State *s, int n) {
	return *(Zone**)luaAI_getudata<Zone*>(s, n, luaAI_metazone());
}

static AggroMgr* luaAI_toaggromgr(lua_State *s, int n) {
	return *(AggroMgr**)luaAI_getudata<AggroMgr*>(s, n, luaAI_metaaggromgr());
}

static GroupMgr* luaAI_togroupmgr(lua_State *s, int n) {
	return *(GroupMgr**)luaAI_getudata<GroupMgr*>(s, n, luaAI_metagroupmgr());
}

static glm::vec3* luaAI_tovec(lua_State *s, int n) {
	return luaAI_getudata<glm::vec3*>(s, n, luaAI_metavec());
}

static int luaAI_pushzone(lua_State* s, Zone* zone) {
	if (zone == nullptr) {
		lua_pushnil(s);
		return 1;
	}
	return luaAI_pushudata<Zone*>(s, zone, luaAI_metazone());
}

static int luaAI_pushaggromgr(lua_State* s, AggroMgr* aggroMgr) {
	return luaAI_pushudata<AggroMgr*>(s, aggroMgr, luaAI_metaaggromgr());
}

static int luaAI_pushgroupmgr(lua_State* s, GroupMgr* groupMgr) {
	return luaAI_pushudata<GroupMgr*>(s, groupMgr, luaAI_metagroupmgr());
}

static int luaAI_pushcharacter(lua_State* s, const ICharacterPtr& character) {
	luaAI_ICharacter* raw = (luaAI_ICharacter*) lua_newuserdata(s, sizeof(luaAI_ICharacter));
	luaAI_ICharacter* udata = new (raw)luaAI_ICharacter();
	udata->character = character;
	return luaAI_assignmetatable(s, luaAI_metacharacter());
}

static int luaAI_pushai(lua_State* s, const AIPtr& ai) {
	luaAI_AI* raw = (luaAI_AI*) lua_newuserdata(s, sizeof(luaAI_AI));
	luaAI_AI* udata = new (raw)luaAI_AI();
	udata->ai = ai;
	return luaAI_assignmetatable(s, luaAI_metaai());
}

static int luaAI_pushvec(lua_State* s, const glm::vec3& v) {
	return luaAI_pushudata<glm::vec3>(s, v, luaAI_metavec());
}

/***
 * Get the position of the group (average)
 * @tparam integer groupId
 * @treturn vec The average position of the group
 * @function groupMgr:position
 */
static int luaAI_groupmgrposition(lua_State* s) {
	const GroupMgr* groupMgr = luaAI_togroupmgr(s, 1);
	const GroupId groupId = (GroupId)luaL_checkinteger(s, 2);
	return luaAI_pushvec(s, groupMgr->getPosition(groupId));
}

/***
 * Add an entity from the group
 * @tparam integer groupId
 * @tparam ai ai
 * @treturn boolean Boolean to indicate whether the add was successful
 * @function groupMgr:add
 */
static int luaAI_groupmgradd(lua_State* s) {
	GroupMgr* groupMgr = luaAI_togroupmgr(s, 1);
	const GroupId groupId = (GroupId)luaL_checkinteger(s, 2);
	luaAI_AI* ai = luaAI_toai(s, 3);
	const bool state = groupMgr->add(groupId, ai->ai);
	lua_pushboolean(s, state);
	return 1;
}

/***
 * Remove an entity from the group
 * @tparam integer groupId
 * @tparam ai ai
 * @treturn boolean Boolean to indicate whether the removal was successful
 * @function groupMgr:remove
 */
static int luaAI_groupmgrremove(lua_State* s) {
	GroupMgr* groupMgr = luaAI_togroupmgr(s, 1);
	const GroupId groupId = (GroupId)luaL_checkinteger(s, 2);
	luaAI_AI* ai = luaAI_toai(s, 3);
	const bool state = groupMgr->remove(groupId, ai->ai);
	lua_pushboolean(s, state);
	return 1;
}

/***
 * Checks whether a give ai is the group leader of a particular group
 * @tparam integer groupId
 * @tparam ai ai
 * @treturn boolean Boolean to indicate whether the given AI is the group leader of the given group
 * @function groupMgr:isLeader
 */
static int luaAI_groupmgrisleader(lua_State* s) {
	const GroupMgr* groupMgr = luaAI_togroupmgr(s, 1);
	const GroupId groupId = (GroupId)luaL_checkinteger(s, 2);
	luaAI_AI* ai = luaAI_toai(s, 3);
	const bool state = groupMgr->isGroupLeader(groupId, ai->ai);
	lua_pushboolean(s, state);
	return 1;
}

/***
 * Checks whether a give ai is part of the given group
 * @tparam integer groupId
 * @tparam ai ai
 * @treturn boolean Boolean to indicate whether the given AI is part of the given group
 * @function groupMgr:isInGroup
 */
static int luaAI_groupmgrisingroup(lua_State* s) {
	const GroupMgr* groupMgr = luaAI_togroupmgr(s, 1);
	const GroupId groupId = (GroupId)luaL_checkinteger(s, 2);
	luaAI_AI* ai = luaAI_toai(s, 3);
	const bool state = groupMgr->isInGroup(groupId, ai->ai);
	lua_pushboolean(s, state);
	return 1;
}

/***
 * Checks whether a give ai is part of any group
 * @tparam ai ai
 * @treturn boolean Boolean to indicate whether the given AI is part of any group
 * @function groupMgr:isInAnyGroup
 */
static int luaAI_groupmgrisinanygroup(lua_State* s) {
	const GroupMgr* groupMgr = luaAI_togroupmgr(s, 1);
	luaAI_AI* ai = luaAI_toai(s, 2);
	const bool state = groupMgr->isInAnyGroup(ai->ai);
	lua_pushboolean(s, state);
	return 1;
}

/***
 * Get the group size
 * @tparam integer groupId
 * @treturn integer Size of the group (members)
 * @function groupMgr:size
 */
static int luaAI_groupmgrsize(lua_State* s) {
	const GroupMgr* groupMgr = luaAI_togroupmgr(s, 1);
	const GroupId groupId = (GroupId)luaL_checkinteger(s, 2);
	lua_pushinteger(s, groupMgr->getGroupSize(groupId));
	return 1;
}

/***
 * Get the group leader ai of a given group
 * @tparam integer groupId
 * @treturn ai AI of the group leader, or nil, if there is no such group
 * @function groupMgr:leader
 */
static int luaAI_groupmgrleader(lua_State* s) {
	const GroupMgr* groupMgr = luaAI_togroupmgr(s, 1);
	const GroupId groupId = (GroupId)luaL_checkinteger(s, 2);
	const AIPtr& ai = groupMgr->getLeader(groupId);
	if (!ai) {
		lua_pushnil(s);
	} else {
		luaAI_pushai(s, ai);
	}
	return 1;
}

static int luaAI_groupmgrtostring(lua_State* s) {
	const GroupMgr* groupMgr = luaAI_togroupmgr(s, 1);
	lua_pushfstring(s, "groupmgr: %p", groupMgr);
	return 1;
}

/***
 * Execute a function for all entities of the zone
 * @tparam function The function to execute
 * @function zone:execute
 */
static int luaAI_zoneexecute(lua_State* s) {
	Zone* zone = luaAI_tozone(s, 1);
	luaL_checktype(s, 2, LUA_TFUNCTION);
	const int topIndex = lua_gettop(s);
	zone->execute([=] (const AIPtr& ai) {
		if (luaAI_pushai(s, ai) <= 0) {
			return;
		}
		lua_pcall(s, 1, 0, 0);
		const int stackDelta = lua_gettop(s) - topIndex;
		if (stackDelta > 0) {
			lua_pop(s, stackDelta);
		}
	});
	return 0;
}

/***
 * Get the group manager of the zone
 * @treturn groupMgr The groupMgr of the zone
 * @function zone:groupMgr
 */
static int luaAI_zonegroupmgr(lua_State* s) {
	Zone* zone = luaAI_tozone(s, 1);
	return luaAI_pushgroupmgr(s, &zone->getGroupMgr());
}

static int luaAI_zonetostring(lua_State* s) {
	const Zone* zone = luaAI_tozone(s, 1);
	lua_pushfstring(s, "zone: %s", zone->getName().c_str());
	return 1;
}

/***
 * Get the name of the zone
 * @treturn string The name of the zone
 * @function zone:name
 */
static int luaAI_zonename(lua_State* s) {
	const Zone* zone = luaAI_tozone(s, 1);
	lua_pushstring(s, zone->getName().c_str());
	return 1;
}

/***
 * Get the ai instance for some character id in the zone
 * @tparam integer id The character id to get the AI for
 * @treturn ai AI instance for some character id in the zone, or nil if no such character
 * was found in the zone
 * @function zone:ai
 */
static int luaAI_zoneai(lua_State* s) {
	Zone* zone = luaAI_tozone(s, 1);
	const CharacterId id = (CharacterId)luaL_checkinteger(s, 2);
	const AIPtr& ai = zone->getAI(id);
	if (!ai) {
		lua_pushnil(s);
	} else {
		luaAI_pushai(s, ai);
	}
	return 1;
}

/***
 * Get the size of the zone - as in: How many entities are in that particular zone
 * @treturn integer The amount of entities in the zone
 * @function zone:size
 */
static int luaAI_zonesize(lua_State* s) {
	const Zone* zone = luaAI_tozone(s, 1);
	lua_pushinteger(s, zone->size());
	return 1;
}

/***
 * Get the highest aggro entry
 * @treturn integer The current highest aggro entry character id or nil
 * @treturn number The current highest aggro entry value or nil.
 * @function aggroMgr:highestEntry
 */
static int luaAI_aggromgrhighestentry(lua_State* s) {
	AggroMgr* aggroMgr = luaAI_toaggromgr(s, 1);
	EntryPtr entry = aggroMgr->getHighestEntry();
	if (entry == nullptr) {
		lua_pushnil(s);
		lua_pushnil(s);
	} else {
		lua_pushinteger(s, entry->getCharacterId());
		lua_pushnumber(s, entry->getAggro());
	}
	return 2;
}

/***
 * Return the current aggro manager entries
 * @treturn {integer, number, ...} Table with character id and aggro value
 * @function aggroMgr:entries
 */
static int luaAI_aggromgrentries(lua_State* s) {
	AggroMgr* aggroMgr = luaAI_toaggromgr(s, 1);
	const AggroMgr::Entries& entries = aggroMgr->getEntries();
	lua_newtable(s);
	const int top = lua_gettop(s);
	for (auto it = entries.begin(); it != entries.end(); ++it) {
		lua_pushinteger(s, it->getCharacterId());
		lua_pushnumber(s, it->getAggro());
		lua_settable(s, top);
	}
	return 1;
}

/***
 * Set the reduction type to a ratio-per-second style
 * @tparam number reduceRatioSecond
 * @tparam number minAggro
 * @function aggroMgr:reduceByRatio
 */
static int luaAI_aggromgrsetreducebyratio(lua_State* s) {
	AggroMgr* aggroMgr = luaAI_toaggromgr(s, 1);
	const lua_Number reduceRatioSecond = luaL_checknumber(s, 2);
	const lua_Number minAggro = luaL_checknumber(s, 3);
	aggroMgr->setReduceByRatio((float)reduceRatioSecond, (float)minAggro);
	return 0;
}

/***
 * Set the reduction type to a value-by-second style
 * @tparam number reduceValueSecond
 * @function aggroMgr:reduceByValue
 */
static int luaAI_aggromgrsetreducebyvalue(lua_State* s) {
	AggroMgr* aggroMgr = luaAI_toaggromgr(s, 1);
	const lua_Number reduceValueSecond = luaL_checknumber(s, 2);
	aggroMgr->setReduceByValue((float)reduceValueSecond);
	return 0;
}

/***
 * Reset the reduction values of the aggro manager
 * @function aggroMgr:resetReduceValue
 */
static int luaAI_aggromgrresetreducevalue(lua_State* s) {
	AggroMgr* aggroMgr = luaAI_toaggromgr(s, 1);
	aggroMgr->resetReduceValue();
	return 0;
}

/***
 * Apply aggro on some other character
 * @tparam integer id The character id to get aggro on
 * @tparam number amount The amount of aggro to apply
 * @treturn number The amount of aggro you have on the given entity
 * @function aggroMgr:addAggro
 */
static int luaAI_aggromgraddaggro(lua_State* s) {
	AggroMgr* aggroMgr = luaAI_toaggromgr(s, 1);
	const CharacterId chrId = (CharacterId)luaL_checkinteger(s, 2);
	const lua_Number amount = luaL_checknumber(s, 3);
	const EntryPtr& entry = aggroMgr->addAggro(chrId, (float)amount);
	lua_pushnumber(s, entry->getAggro());
	return 1;
}

static int luaAI_aggromgrtostring(lua_State* s) {
	lua_pushliteral(s, "aggroMgr");
	return 1;
}

/***
 * The character id
 * @treturn integer The id of a character
 * @function character:id
 */
static int luaAI_characterid(lua_State* s) {
	const luaAI_ICharacter* chr = luaAI_tocharacter(s, 1);
	lua_pushinteger(s, chr->character->getId());
	return 1;
}

/***
 * Get the position of the character
 * @treturn vec position of the character
 * @function character:position
 */
static int luaAI_characterposition(lua_State* s) {
	const luaAI_ICharacter* chr = luaAI_tocharacter(s, 1);
	return luaAI_pushvec(s, chr->character->getPosition());
}

/***
 * Set the position of the character
 * @tparam vec position
 * @function character:setPosition
 */
static int luaAI_charactersetposition(lua_State* s) {
	luaAI_ICharacter* chr = luaAI_tocharacter(s, 1);
	const glm::vec3* v = luaAI_tovec(s, 2);
	chr->character->setPosition(*v);
	return 0;
}

/***
 * Get the speed of the character
 * @treturn number Speed value of the character
 * @function character:speed
 */
static int luaAI_characterspeed(lua_State* s) {
	const luaAI_ICharacter* chr = luaAI_tocharacter(s, 1);
	lua_pushnumber(s, chr->character->getSpeed());
	return 1;
}

/***
 * Get the orientation of the character
 * @treturn number orientation value of the character
 * @function character:orientation
 */
static int luaAI_characterorientation(lua_State* s) {
	const luaAI_ICharacter* chr = luaAI_tocharacter(s, 1);
	lua_pushnumber(s, chr->character->getOrientation());
	return 1;
}

/***
 * Set the speed for a character
 * @tparam number speed
 * @function character:setSpeed
 */
static int luaAI_charactersetspeed(lua_State* s) {
	luaAI_ICharacter* chr = luaAI_tocharacter(s, 1);
	const lua_Number value = luaL_checknumber(s, 2);
	chr->character->setSpeed((float)value);
	return 0;
}

/***
 * Set the orientation for a character
 * @tparam number orientation
 * @function character:setOrientation
 */
static int luaAI_charactersetorientation(lua_State* s) {
	luaAI_ICharacter* chr = luaAI_tocharacter(s, 1);
	const lua_Number value = luaL_checknumber(s, 2);
	chr->character->setOrientation((float)value);
	return 0;
}

/***
 * Equal check for a character
 * @treturn boolean
 * @function character:__eq
 */
static int luaAI_charactereq(lua_State* s) {
	const luaAI_ICharacter* a = luaAI_tocharacter(s, 1);
	const luaAI_ICharacter* b = luaAI_tocharacter(s, 2);
	const bool e = *a->character == *b->character;
	lua_pushboolean(s, e);
	return 1;
}

/***
 * Garbage collector callback for a character
 * @function character:__gc
 */
static int luaAI_charactergc(lua_State* s) {
	luaAI_ICharacter* chr = luaAI_tocharacter(s, -1);
	chr->character = ICharacterPtr();
	return 0;
}

/***
 * Pushes the table of character (debugger) attributes onto the stack
 * @treturn {string, string, ....} Table with the key/value pair of the character attributes
 * @function character:attributes
 */
static int luaAI_characterattributes(lua_State* s) {
	const luaAI_ICharacter* chr = luaAI_tocharacter(s, 1);
	const CharacterAttributes& attributes = chr->character->getAttributes();
	lua_newtable(s);
	const int top = lua_gettop(s);
	for (auto it = attributes.begin(); it != attributes.end(); ++it) {
		const std::string& key = it->first;
		const std::string& value = it->second;
		lua_pushlstring(s, key.c_str(), key.size());
		lua_pushlstring(s, value.c_str(), value.size());
		lua_settable(s, top);
	}
	return 1;
}

/***
 * Set a debugger attribute to the character
 * @tparam string key The key of the attribute
 * @tparam string value The value of the attribute
 * @function character:setAttribute
 */
static int luaAI_charactersetattribute(lua_State* s) {
	luaAI_ICharacter* chr = luaAI_tocharacter(s, 1);
	const char* key = luaL_checkstring(s, 2);
	const char* value = luaL_checkstring(s, 3);
	chr->character->setAttribute(key, value);
	return 0;
}

static int luaAI_charactertostring(lua_State* s) {
	luaAI_ICharacter* character = luaAI_tocharacter(s, 1);
	lua_pushfstring(s, "Character: %d", (lua_Integer)character->character->getId());
	return 1;
}

/***
 * Get the character id
 * @treturn integer Integer with the character id
 * @function ai:id
 */
static int luaAI_aiid(lua_State* s) {
	const luaAI_AI* ai = luaAI_toai(s, 1);
	lua_pushinteger(s, ai->ai->getId());
	return 1;
}

/***
 * Get the active lifetime of the ai
 * @treturn integer Integer with the active lifetime of the ai
 * @function ai:time
 */
static int luaAI_aitime(lua_State* s) {
	const luaAI_AI* ai = luaAI_toai(s, 1);
	lua_pushinteger(s, ai->ai->getTime());
	return 1;
}

/***
 * Get the filtered entities of the ai
 * @treturn {integer, integer, ...} A table with key/value (index, characterid) pairs of the filtered entities
 * @function ai:filteredEntities
 */
static int luaAI_aifilteredentities(lua_State* s) {
	const luaAI_AI* ai = luaAI_toai(s, 1);
	const FilteredEntities& filteredEntities = ai->ai->getFilteredEntities();
	lua_newtable(s);
	const int top = lua_gettop(s);
	int i = 0;
	for (auto it = filteredEntities.begin(); it != filteredEntities.end(); ++it) {
		lua_pushinteger(s, ++i);
		lua_pushinteger(s, *it);
		lua_settable(s, top);
	}
	return 1;
}

/***
 * Get the zone of the ai
 * @treturn zone The zone where the ai is active in or nil
 * @function ai:zone
 */
static int luaAI_aigetzone(lua_State* s) {
	const luaAI_AI* ai = luaAI_toai(s, 1);
	return luaAI_pushzone(s, ai->ai->getZone());
}

/***
 * Get the aggro mgr of the ai
 * @treturn aggroMgr the aggro manager of the ai
 * @function ai:aggroMgr
 */
static int luaAI_aigetaggromgr(lua_State* s) {
	luaAI_AI* ai = luaAI_toai(s, 1);
	return luaAI_pushaggromgr(s, &ai->ai->getAggroMgr());
}

/***
 * Get the character of the ai
 * @treturn character the character of the ai
 * @function ai:character
 */
static int luaAI_aigetcharacter(lua_State* s) {
	const luaAI_AI* ai = luaAI_toai(s, 1);
	return luaAI_pushcharacter(s, ai->ai->getCharacter());
}

/***
 * Check whether the ai has a zone
 * @treturn boolean true if the ai has a zone, false otherwise
 * @function ai:hasZone
 */
static int luaAI_aihaszone(lua_State* s) {
	const luaAI_AI* ai = luaAI_toai(s, 1);
	lua_pushboolean(s, ai->ai->hasZone() ? 1 : 0);
	return 1;
}

/***
 * Equality for two ai instances
 * @treturn boolean true if the two given ai's are equal
 * @function ai:__eq
 */
static int luaAI_aieq(lua_State* s) {
	const luaAI_AI* a = luaAI_toai(s, 1);
	const luaAI_AI* b = luaAI_toai(s, 2);
	const bool e = a->ai->getId() == b->ai->getId();
	lua_pushboolean(s, e);
	return 1;
}

/***
 * Garbage collector callback for ai instances
 * @function ai:__gc
 */
static int luaAI_aigc(lua_State* s) {
	luaAI_AI* ai = luaAI_toai(s, -1);
	ai->ai = AIPtr();
	return 0;
}

static int luaAI_aitostring(lua_State* s) {
	const luaAI_AI* ai = luaAI_toai(s, 1);
	TreeNodePtr treeNode = ai->ai->getBehaviour();
	if (treeNode) {
		lua_pushfstring(s, "ai: %s", treeNode->getName().c_str());
	} else {
		lua_pushstring(s, "ai: no behaviour tree set");
	}
	return 1;
}

/***
 * Vector addition
 * @tparam vec a
 * @tparam vec b
 * @treturn vec the sum of a + b
 * @function vec:__add
 */
static int luaAI_vecadd(lua_State* s) {
	const glm::vec3* a = luaAI_tovec(s, 1);
	const glm::vec3* b = luaAI_tovec(s, 2);
	const glm::vec3& c = *a + *b;
	return luaAI_pushvec(s, c);
}

/***
 * Vector dot product also as vec:__mul
 * @tparam vec a
 * @tparam vec b
 * @treturn number The dot product of a and b
 * @function vec:dot
 */
static int luaAI_vecdot(lua_State* s) {
	const glm::vec3* a = luaAI_tovec(s, 1);
	const glm::vec3* b = luaAI_tovec(s, 2);
	const float c = glm::dot(*a, *b);
	lua_pushnumber(s, c);
	return 1;
}

/***
 * Vector div function
 * @tparam vec a
 * @tparam vec b
 * @treturn vec a / b
 * @function vec:__div
 */
static int luaAI_vecdiv(lua_State* s) {
	const glm::vec3* a = luaAI_tovec(s, 1);
	const glm::vec3* b = luaAI_tovec(s, 2);
	const glm::vec3& c = *a / *b;
	luaAI_pushvec(s, c);
	return 1;
}

static int luaAI_veclen(lua_State* s) {
	const glm::vec3* a = luaAI_tovec(s, 1);
	const float c = glm::length(*a);
	lua_pushnumber(s, c);
	return 1;
}

static int luaAI_veceq(lua_State* s) {
	const glm::vec3* a = luaAI_tovec(s, 1);
	const glm::vec3* b = luaAI_tovec(s, 2);
	const bool e = glm::all(glm::epsilonEqual(*a, *b, 0.0001f));
	lua_pushboolean(s, e);
	return 1;
}

/***
 * Vector subtraction
 * @tparam vec a
 * @tparam vec b
 * @treturn vec the result of a - b
 * @function vec:__sub
 */
static int luaAI_vecsub(lua_State* s) {
	const glm::vec3* a = luaAI_tovec(s, 1);
	const glm::vec3* b = luaAI_tovec(s, 2);
	const glm::vec3& c = *a - *b;
	luaAI_pushvec(s, c);
	return 1;
}

/***
 * Negates a given vector
 * @tparam vec a
 * @treturn vec The result of -a
 * @function vec:__unm
 */
static int luaAI_vecnegate(lua_State* s) {
	const glm::vec3* a = luaAI_tovec(s, 1);
	luaAI_pushvec(s, -(*a));
	return 1;
}

static int luaAI_vectostring(lua_State* s) {
	const glm::vec3* a = luaAI_tovec(s, 1);
	lua_pushfstring(s, "vec: %f:%f:%f", a->x, a->y, a->z);
	return 1;
}

static int luaAI_vecindex(lua_State *s) {
	const glm::vec3* v = luaAI_tovec(s, 1);
	const char* i = luaL_checkstring(s, 2);

	switch (*i) {
	case '0':
	case 'x':
	case 'r':
		lua_pushnumber(s, v->x);
		break;
	case '1':
	case 'y':
	case 'g':
		lua_pushnumber(s, v->y);
		break;
	case '2':
	case 'z':
	case 'b':
		lua_pushnumber(s, v->z);
		break;
	default:
		lua_pushnil(s);
		break;
	}

	return 1;
}

static int luaAI_vecnewindex(lua_State *s) {
	glm::vec3* v = luaAI_tovec(s, 1);
	const char *i = luaL_checkstring(s, 2);
	const lua_Number t = luaL_checknumber(s, 3);

	switch (*i) {
	case '0':
	case 'x':
	case 'r':
		v->x = (float)t;
		break;
	case '1':
	case 'y':
	case 'g':
		v->y = (float)t;
		break;
	case '2':
	case 'z':
	case 'b':
		v->z = (float)t;
		break;
	default:
		break;
	}

	return 1;
}

}

// #include "tree/LUATreeNode.h"
/**
 * @file
 * @ingroup LUA
 */


// #include "tree/TreeNode.h"

// #include "LUAFunctions.h"


namespace ai {

/**
 * @see @ai{LUAAIRegistry}
 */
class LUATreeNode : public TreeNode {
protected:
	lua_State* _s;

	TreeNodeStatus runLUA(const AIPtr& entity, int64_t deltaMillis) {
		// get userdata of the behaviour tree node
		const std::string name = "__meta_node_" + _type;
		lua_getfield(_s, LUA_REGISTRYINDEX, name.c_str());
#if AI_LUA_SANTITY > 0
		if (lua_isnil(_s, -1)) {
			ai_log_error("LUA node: could not find lua userdata for %s", name.c_str());
			return TreeNodeStatus::EXCEPTION;
		}
#endif
		// get metatable
		lua_getmetatable(_s, -1);
#if AI_LUA_SANTITY > 0
		if (!lua_istable(_s, -1)) {
			ai_log_error("LUA node: userdata for %s doesn't have a metatable assigned", name.c_str());
			return TreeNodeStatus::EXCEPTION;
		}
#endif
		// get execute() method
		lua_getfield(_s, -1, "execute");
		if (!lua_isfunction(_s, -1)) {
			ai_log_error("LUA node: metatable for %s doesn't have the execute() function assigned", name.c_str());
			return TreeNodeStatus::EXCEPTION;
		}

		// push self onto the stack
		lua_getfield(_s, LUA_REGISTRYINDEX, name.c_str());

		// first parameter is ai
		if (luaAI_pushai(_s, entity) == 0) {
			return TreeNodeStatus::EXCEPTION;
		}

		// second parameter is dt
		lua_pushinteger(_s, deltaMillis);

#if AI_LUA_SANTITY > 0
		if (!lua_isfunction(_s, -4)) {
			ai_log_error("LUA node: expected to find a function on stack -4");
			return TreeNodeStatus::EXCEPTION;
		}
		if (!lua_isuserdata(_s, -3)) {
			ai_log_error("LUA node: expected to find the userdata on -3");
			return TreeNodeStatus::EXCEPTION;
		}
		if (!lua_isuserdata(_s, -2)) {
			ai_log_error("LUA node: second parameter should be the ai");
			return TreeNodeStatus::EXCEPTION;
		}
		if (!lua_isinteger(_s, -1)) {
			ai_log_error("LUA node: first parameter should be the delta millis");
			return TreeNodeStatus::EXCEPTION;
		}
#endif
		const int error = lua_pcall(_s, 3, 1, 0);
		if (error) {
			ai_log_error("LUA node script: %s", lua_isstring(_s, -1) ? lua_tostring(_s, -1) : "Unknown Error");
			// reset stack
			lua_pop(_s, lua_gettop(_s));
			return TreeNodeStatus::EXCEPTION;
		}
		const lua_Integer execstate = luaL_checkinteger(_s, -1);
		if (execstate < 0 || execstate >= (lua_Integer)TreeNodeStatus::MAX_TREENODESTATUS) {
			ai_log_error("LUA node: illegal tree node status returned: " LUA_INTEGER_FMT, execstate);
		}

		// reset stack
		lua_pop(_s, lua_gettop(_s));
		return (TreeNodeStatus)execstate;
	}

public:
	class LUATreeNodeFactory : public ITreeNodeFactory {
	private:
		lua_State* _s;
		std::string _type;
	public:
		LUATreeNodeFactory(lua_State* s, const std::string& typeStr) :
				_s(s), _type(typeStr) {
		}

		inline const std::string& type() const {
			return _type;
		}

		TreeNodePtr create(const TreeNodeFactoryContext* ctx) const override {
			return std::make_shared<LUATreeNode>(ctx->name, ctx->parameters, ctx->condition, _s, _type);
		}
	};

	LUATreeNode(const std::string& name, const std::string& parameters, const ConditionPtr& condition, lua_State* s, const std::string& type) :
			TreeNode(name, parameters, condition), _s(s) {
		_type = type;
	}

	~LUATreeNode() {
	}

	TreeNodeStatus execute(const AIPtr& entity, int64_t deltaMillis) override {
		if (TreeNode::execute(entity, deltaMillis) == CANNOTEXECUTE) {
			return CANNOTEXECUTE;
		}

#if AI_EXCEPTIONS
		try {
#endif
			return state(entity, runLUA(entity, deltaMillis));
#if AI_EXCEPTIONS
		} catch (...) {
			ai_log_error("Exception while running lua tree node");
		}
		return state(entity, EXCEPTION);
#endif
	}
};

}

// #include "conditions/LUACondition.h"
/**
 * @file
 * @ingroup Condition
 * @ingroup LUA
 */


// #include "ICondition.h"

// #include "LUAFunctions.h"


namespace ai {

/**
 * @see @ai{LUAAIRegistry}
 */
class LUACondition : public ICondition {
protected:
	lua_State* _s;

	bool evaluateLUA(const AIPtr& entity) {
		// get userdata of the condition
		const std::string name = "__meta_condition_" + _name;
		lua_getfield(_s, LUA_REGISTRYINDEX, name.c_str());
#if AI_LUA_SANTITY > 0
		if (lua_isnil(_s, -1)) {
			ai_log_error("LUA condition: could not find lua userdata for %s", _name.c_str());
			return false;
		}
#endif
		// get metatable
		lua_getmetatable(_s, -1);
#if AI_LUA_SANTITY > 0
		if (!lua_istable(_s, -1)) {
			ai_log_error("LUA condition: userdata for %s doesn't have a metatable assigned", _name.c_str());
			return false;
		}
#endif
		// get evaluate() method
		lua_getfield(_s, -1, "evaluate");
		if (!lua_isfunction(_s, -1)) {
			ai_log_error("LUA condition: metatable for %s doesn't have the evaluate() function assigned", _name.c_str());
			return false;
		}

		// push self onto the stack
		lua_getfield(_s, LUA_REGISTRYINDEX, name.c_str());

		// first parameter is ai
		if (luaAI_pushai(_s, entity) == 0) {
			return false;
		}

#if AI_LUA_SANTITY > 0
		if (!lua_isfunction(_s, -3)) {
			ai_log_error("LUA condition: expected to find a function on stack -3");
			return false;
		}
		if (!lua_isuserdata(_s, -2)) {
			ai_log_error("LUA condition: expected to find the userdata on -2");
			return false;
		}
		if (!lua_isuserdata(_s, -1)) {
			ai_log_error("LUA condition: second parameter should be the ai");
			return false;
		}
#endif
		const int error = lua_pcall(_s, 2, 1, 0);
		if (error) {
			ai_log_error("LUA condition script: %s", lua_isstring(_s, -1) ? lua_tostring(_s, -1) : "Unknown Error");
			// reset stack
			lua_pop(_s, lua_gettop(_s));
			return false;
		}
		const int state = lua_toboolean(_s, -1);
		if (state != 0 && state != 1) {
			ai_log_error("LUA condition: illegal evaluate() value returned: %i", state);
			return false;
		}

		// reset stack
		lua_pop(_s, lua_gettop(_s));
		return state == 1;
	}

public:
	class LUAConditionFactory : public IConditionFactory {
	private:
		lua_State* _s;
		std::string _type;
	public:
		LUAConditionFactory(lua_State* s, const std::string& typeStr) :
				_s(s), _type(typeStr) {
		}

		inline const std::string& type() const {
			return _type;
		}

		ConditionPtr create(const ConditionFactoryContext* ctx) const override {
			return std::make_shared<LUACondition>(_type, ctx->parameters, _s);
		}
	};

	LUACondition(const std::string& name, const std::string& parameters, lua_State* s) :
			ICondition(name, parameters), _s(s) {
	}

	~LUACondition() {
	}

	bool evaluate(const AIPtr& entity) override {
#if AI_EXCEPTIONS
		try {
#endif
			return evaluateLUA(entity);
#if AI_EXCEPTIONS
		} catch (...) {
			ai_log_error("Exception while evaluating lua condition");
		}
		return false;
#endif
	}
};

}

// #include "filter/LUAFilter.h"
/**
 * @file
 * @ingroup LUA
 * @ingroup Filter
 */


// #include "IFilter.h"

// #include "LUAFunctions.h"


namespace ai {

/**
 * @see @ai{LUAAIRegistry}
 */
class LUAFilter : public IFilter {
protected:
	lua_State* _s;

	void filterLUA(const AIPtr& entity) {
		// get userdata of the filter
		const std::string name = "__meta_filter_" + _name;
		lua_getfield(_s, LUA_REGISTRYINDEX, name.c_str());
#if AI_LUA_SANTITY > 0
		if (lua_isnil(_s, -1)) {
			ai_log_error("LUA filter: could not find lua userdata for %s", _name.c_str());
			return;
		}
#endif
		// get metatable
		lua_getmetatable(_s, -1);
#if AI_LUA_SANTITY > 0
		if (!lua_istable(_s, -1)) {
			ai_log_error("LUA filter: userdata for %s doesn't have a metatable assigned", _name.c_str());
			return;
		}
#endif
		// get filter() method
		lua_getfield(_s, -1, "filter");
		if (!lua_isfunction(_s, -1)) {
			ai_log_error("LUA filter: metatable for %s doesn't have the filter() function assigned", _name.c_str());
			return;
		}

		// push self onto the stack
		lua_getfield(_s, LUA_REGISTRYINDEX, name.c_str());

		// first parameter is ai
		if (luaAI_pushai(_s, entity) == 0) {
			return;
		}
#if AI_LUA_SANTITY > 0
		if (!lua_isfunction(_s, -3)) {
			ai_log_error("LUA filter: expected to find a function on stack -3");
			return;
		}
		if (!lua_isuserdata(_s, -2)) {
			ai_log_error("LUA filter: expected to find the userdata on -2");
			return;
		}
		if (!lua_isuserdata(_s, -1)) {
			ai_log_error("LUA filter: second parameter should be the ai");
			return;
		}
#endif
		const int error = lua_pcall(_s, 2, 0, 0);
		if (error) {
			ai_log_error("LUA filter script: %s", lua_isstring(_s, -1) ? lua_tostring(_s, -1) : "Unknown Error");
		}

		// reset stack
		lua_pop(_s, lua_gettop(_s));
	}

public:
	class LUAFilterFactory : public IFilterFactory {
	private:
		lua_State* _s;
		std::string _type;
	public:
		LUAFilterFactory(lua_State* s, const std::string& typeStr) :
				_s(s), _type(typeStr) {
		}

		inline const std::string& type() const {
			return _type;
		}

		FilterPtr create(const FilterFactoryContext* ctx) const override {
			return std::make_shared<LUAFilter>(_type, ctx->parameters, _s);
		}
	};

	LUAFilter(const std::string& name, const std::string& parameters, lua_State* s) :
			IFilter(name, parameters), _s(s) {
	}

	~LUAFilter() {
	}

	void filter(const AIPtr& entity) override {
#if AI_EXCEPTIONS
		try {
#endif
			filterLUA(entity);
#if AI_EXCEPTIONS
		} catch (...) {
			ai_log_error("Exception while evaluating lua filter");
		}
#endif
	}
};

}

// #include "movement/LUASteering.h"
/**
 * @file
 * @ingroup LUA
 */


// #include "Steering.h"

// #include "LUAFunctions.h"


namespace ai {
namespace movement {

/**
 * @see @ai{LUAAIRegistry}
 */
class LUASteering : public ISteering {
protected:
	mutable lua_State* _s;
	std::string _type;

	MoveVector executeLUA(const AIPtr& entity, float speed) const {
		// get userdata of the behaviour tree steering
		const std::string name = "__meta_steering_" + _type;
		lua_getfield(_s, LUA_REGISTRYINDEX, name.c_str());
#if AI_LUA_SANTITY > 0
		if (lua_isnil(_s, -1)) {
			ai_log_error("LUA steering: could not find lua userdata for %s", name.c_str());
			return MoveVector(VEC3_INFINITE, 0.0f);
		}
#endif
		// get metatable
		lua_getmetatable(_s, -1);
#if AI_LUA_SANTITY > 0
		if (!lua_istable(_s, -1)) {
			ai_log_error("LUA steering: userdata for %s doesn't have a metatable assigned", name.c_str());
			return MoveVector(VEC3_INFINITE, 0.0f);
		}
#endif
		// get execute() method
		lua_getfield(_s, -1, "execute");
		if (!lua_isfunction(_s, -1)) {
			ai_log_error("LUA steering: metatable for %s doesn't have the execute() function assigned", name.c_str());
			return MoveVector(VEC3_INFINITE, 0.0f);
		}

		// push self onto the stack
		lua_getfield(_s, LUA_REGISTRYINDEX, name.c_str());

		// first parameter is ai
		if (luaAI_pushai(_s, entity) == 0) {
			return MoveVector(VEC3_INFINITE, 0.0f);
		}

		// second parameter is speed
		lua_pushnumber(_s, speed);

#if AI_LUA_SANTITY > 0
		if (!lua_isfunction(_s, -4)) {
			ai_log_error("LUA steering: expected to find a function on stack -4");
			return MoveVector(VEC3_INFINITE, 0.0f);
		}
		if (!lua_isuserdata(_s, -3)) {
			ai_log_error("LUA steering: expected to find the userdata on -3");
			return MoveVector(VEC3_INFINITE, 0.0f);
		}
		if (!lua_isuserdata(_s, -2)) {
			ai_log_error("LUA steering: second parameter should be the ai");
			return MoveVector(VEC3_INFINITE, 0.0f);
		}
		if (!lua_isnumber(_s, -1)) {
			ai_log_error("LUA steering: first parameter should be the speed");
			return MoveVector(VEC3_INFINITE, 0.0f);
		}
#endif
		const int error = lua_pcall(_s, 3, 4, 0);
		if (error) {
			ai_log_error("LUA steering script: %s", lua_isstring(_s, -1) ? lua_tostring(_s, -1) : "Unknown Error");
			// reset stack
			lua_pop(_s, lua_gettop(_s));
			return MoveVector(VEC3_INFINITE, 0.0f);
		}
		// we get four values back, the direction vector and the
		const lua_Number x = luaL_checknumber(_s, -1);
		const lua_Number y = luaL_checknumber(_s, -2);
		const lua_Number z = luaL_checknumber(_s, -3);
		const lua_Number rotation = luaL_checknumber(_s, -4);

		// reset stack
		lua_pop(_s, lua_gettop(_s));
		return MoveVector(glm::vec3((float)x, (float)y, (float)z), (float)rotation);
	}

public:
	class LUASteeringFactory : public ISteeringFactory {
	private:
		lua_State* _s;
		std::string _type;
	public:
		LUASteeringFactory(lua_State* s, const std::string& typeStr) :
				_s(s), _type(typeStr) {
		}

		inline const std::string& type() const {
			return _type;
		}

		SteeringPtr create(const SteeringFactoryContext* ctx) const override {
			return std::make_shared<LUASteering>(_s, _type);
		}
	};

	LUASteering(lua_State* s, const std::string& type) :
			ISteering(), _s(s) {
		_type = type;
	}

	~LUASteering() {
	}

	MoveVector execute(const AIPtr& entity, float speed) const override {
#if AI_EXCEPTIONS
		try {
#endif
			return executeLUA(entity, speed);
#if AI_EXCEPTIONS
		} catch (...) {
			ai_log_error("Exception while running lua steering");
		}
		return MoveVector(VEC3_INFINITE, 0.0f);
#endif
	}
};

}
}


namespace ai {

/**
 * @brief Allows you to register lua @ai{TreeNode}s, @ai{Conditions}, @ai{Filter}s and @ai{ISteering}s.
 *
 * @see @ai{LUATreeNode}
 *
 * @par TreeNode
 * @code
 * local luanode = REGISTRY.createNode("SomeName")
 * function luanode:execute(ai, deltaMillis)
 *   print("Node execute called with parameters: ai=["..tostring(ai).."], deltaMillis=["..tostring(deltaMillis).."]")
 *   return FINISHED
 * end
 * @encode
 * The @ai{TreeNodeStatus} states are put into the global space. They are: @c UNKNOWN, @c CANNOTEXECUTE,
 * @c RUNNING, @c FINISHED, @c FAILED and @c EXCEPTION
 *
 * Use @c SomeName later on in your behaviour trees to use this @ai{ITreeNode}
 *
 * @par Conditions
 * @code
 * local luacondition = REGISTRY.createCondition("SomeName")
 * function luacondition:evaluate(ai)
 *   --print("Condition evaluate called with parameter: ai=["..tostring(ai).."]")
 *   return true
 * end
 * @encode
 *
 * Use @c SomeName later on in your behaviour trees to use this @ai{ICondition}
 *
 * @par IFilter
 * @code
 * local luafilter = REGISTRY.createFilter("SomeName")
 * function luafilter:filter(ai)
 * end
 * @endcode
 *
 * Use @c SomeName later on in your behaviour trees to use this @ai{ICondition}
 *
 * @par ISteering
 * @code
 * local luasteering = REGISTRY.createSteering("SomeName")
 * function luasteering:execute(ai, speed)
 *   -- return MoveVector
 *   return 0.0, 1.0, 0.0, 0.6
 * end
 * @endcode
 *
 * Use @c SomeName later on in your behaviour trees to use this @ai{ICondition}
 *
 * @par AI metatable
 * There is a metatable that you can modify by calling @ai{LUAAIRegistry::pushAIMetatable()}.
 * This metatable is applied to all @ai{AI} pointers that are forwarded to the lua functions.
 */
class LUAAIRegistry : public AIRegistry {
protected:
	lua_State* _s = nullptr;

	using LuaNodeFactory = LUATreeNode::LUATreeNodeFactory;
	typedef std::shared_ptr<LuaNodeFactory> LUATreeNodeFactoryPtr;
	typedef std::map<std::string, LUATreeNodeFactoryPtr> TreeNodeFactoryMap;

	using LuaConditionFactory = LUACondition::LUAConditionFactory;
	typedef std::shared_ptr<LuaConditionFactory> LUAConditionFactoryPtr;
	typedef std::map<std::string, LUAConditionFactoryPtr> ConditionFactoryMap;

	using LuaFilterFactory = LUAFilter::LUAFilterFactory;
	typedef std::shared_ptr<LuaFilterFactory> LUAFilterFactoryPtr;
	typedef std::map<std::string, LUAFilterFactoryPtr> FilterFactoryMap;

	using LuaSteeringFactory = movement::LUASteering::LUASteeringFactory;
	typedef std::shared_ptr<LuaSteeringFactory> LUASteeringFactoryPtr;
	typedef std::map<std::string, LUASteeringFactoryPtr> SteeringFactoryMap;

	ReadWriteLock _lock{"luaregistry"};
	TreeNodeFactoryMap _treeNodeFactories;
	ConditionFactoryMap _conditionFactories;
	FilterFactoryMap _filterFactories;
	SteeringFactoryMap _steeringFactories;

	/***
	 * Gives you access the the light userdata for the LUAAIRegistry.
	 * @return the registry userdata
	 */
	static LUAAIRegistry* luaAI_toregistry(lua_State * s) {
		return luaAI_getlightuserdata<LUAAIRegistry>(s, luaAI_metaregistry());
	}

	/***
	 * Gives you access the the userdata for the LuaNodeFactory instance you are operating on.
	 * @return the node factory userdata
	 */
	static LuaNodeFactory* luaAI_tonodefactory(lua_State * s, int n) {
		return *(LuaNodeFactory **) lua_touserdata(s, n);
	}

	/***
	 * Gives you access the the userdata for the LuaConditionFactory instance you are operating on.
	 * @return the condition factory userdata
	 */
	static LuaConditionFactory* luaAI_toconditionfactory(lua_State * s, int n) {
		return *(LuaConditionFactory **) lua_touserdata(s, n);
	}

	/***
	 * Gives you access the the userdata for the LuaFilterFactory instance you are operating on.
	 * @return the filter factory userdata
	 */
	static LuaFilterFactory* luaGetFilterFactoryContext(lua_State * s, int n) {
		return *(LuaFilterFactory **) lua_touserdata(s, n);
	}

	/***
	 * Gives you access the the userdata for the LuaSteeringFactory instance you are operating on.
	 * @return the steering factory userdata
	 */
	static LuaSteeringFactory* luaAI_tosteeringfactory(lua_State * s, int n) {
		return *(LuaSteeringFactory **) lua_touserdata(s, n);
	}

	/***
	 * Empty (default) execute() function that just throws an error.
	 * @param ai The ai to execute the tree node on
	 * @param deltaMillis Milliseconds since last execution
	 * @return throws error because execute() function wasn't overridden
	 */
	static int luaAI_nodeemptyexecute(lua_State* s) {
		const LuaNodeFactory* factory = luaAI_tonodefactory(s, 1);
		return luaL_error(s, "There is no execute function set for node: %s", factory->type().c_str());
	}

	static int luaAI_nodetostring(lua_State* s) {
		const LuaNodeFactory* factory = luaAI_tonodefactory(s, 1);
		lua_pushfstring(s, "node: %s", factory->type().c_str());
		return 1;
	}

	/***
	 * @brief Create a new lua @ai{TreeNode}
	 *
	 * @par lua parameters: #1 name of the node
	 * @note you have to specify an @c execute method that accepts two parameters in your lua code. E.g. do it like this:
	 * @code
	 * local luatest = REGISTRY.createNode(\"LuaTest\")
	 " function luatest:execute(ai, deltaMillis)
	 "    return FAILED\n"
	 " end
	 * @endcode
	 */
	static int luaAI_createnode(lua_State* s) {
		LUAAIRegistry* r = luaAI_toregistry(s);
		const std::string type = luaL_checkstring(s, -1);
		const LUATreeNodeFactoryPtr& factory = std::make_shared<LuaNodeFactory>(s, type);
		const bool inserted = r->registerNodeFactory(type, *factory);
		if (!inserted) {
			return luaL_error(s, "tree node %s is already registered", type.c_str());
		}

		luaAI_newuserdata<LuaNodeFactory*>(s, factory.get());
		const luaL_Reg nodes[] = {
			{"execute", luaAI_nodeemptyexecute},
			{"__tostring", luaAI_nodetostring},
			{"__newindex", luaAI_newindex},
			{nullptr, nullptr}
		};
		luaAI_setupmetatable(s, type, nodes, "node");
		ScopedWriteLock scopedLock(r->_lock);
		r->_treeNodeFactories.emplace(type, factory);
		return 1;
	}

	/***
	 * Empty (default) evaluate() function that just throws an error.
	 * @param ai The ai to execute the condition node on
	 * @return throws error because evaluate() function wasn't overridden
	 */
	static int luaAI_conditionemptyevaluate(lua_State* s) {
		const LuaConditionFactory* factory = luaAI_toconditionfactory(s, 1);
		return luaL_error(s, "There is no evaluate function set for condition: %s", factory->type().c_str());
	}

	static int luaAI_conditiontostring(lua_State* s) {
		const LuaConditionFactory* factory = luaAI_toconditionfactory(s, 1);
		lua_pushfstring(s, "condition: %s", factory->type().c_str());
		return 1;
	}

	/***
	 * @param type The string that identifies the name that is used to register the condition under
	 * @return userdata with a metatable for conditions
	 */
	static int luaAI_createcondition(lua_State* s) {
		LUAAIRegistry* r = luaAI_toregistry(s);
		const std::string type = luaL_checkstring(s, -1);
		const LUAConditionFactoryPtr& factory = std::make_shared<LuaConditionFactory>(s, type);
		const bool inserted = r->registerConditionFactory(type, *factory);
		if (!inserted) {
			return luaL_error(s, "condition %s is already registered", type.c_str());
		}

		luaAI_newuserdata<LuaConditionFactory*>(s, factory.get());
		const luaL_Reg nodes[] = {
			{"evaluate", luaAI_conditionemptyevaluate},
			{"__tostring", luaAI_conditiontostring},
			{"__newindex", luaAI_newindex},
			{nullptr, nullptr}
		};
		luaAI_setupmetatable(s, type, nodes, "condition");
		ScopedWriteLock scopedLock(r->_lock);
		r->_conditionFactories.emplace(type, factory);
		return 1;
	}

	/***
	 * Empty (default) filter() function that just throws an error.
	 * @param ai The ai to execute the filter for
	 * @return throws error because filter() function wasn't overridden
	 */
	static int luaAI_filteremptyfilter(lua_State* s) {
		const LuaFilterFactory* factory = luaGetFilterFactoryContext(s, 1);
		return luaL_error(s, "There is no filter function set for filter: %s", factory->type().c_str());
	}

	static int luaAI_filtertostring(lua_State* s) {
		const LuaFilterFactory* factory = luaGetFilterFactoryContext(s, 1);
		lua_pushfstring(s, "filter: %s", factory->type().c_str());
		return 1;
	}

	static int luaAI_createfilter(lua_State* s) {
		LUAAIRegistry* r = luaAI_toregistry(s);
		const std::string type = luaL_checkstring(s, -1);
		const LUAFilterFactoryPtr& factory = std::make_shared<LuaFilterFactory>(s, type);
		const bool inserted = r->registerFilterFactory(type, *factory);
		if (!inserted) {
			return luaL_error(s, "filter %s is already registered", type.c_str());
		}

		luaAI_newuserdata<LuaFilterFactory*>(s, factory.get());
		const luaL_Reg nodes[] = {
			{"filter", luaAI_filteremptyfilter},
			{"__tostring", luaAI_filtertostring},
			{"__newindex", luaAI_newindex},
			{nullptr, nullptr}
		};
		luaAI_setupmetatable(s, type, nodes, "filter");

		ScopedWriteLock scopedLock(r->_lock);
		r->_filterFactories.emplace(type, factory);
		return 1;
	}

	static int luaAI_steeringemptyexecute(lua_State* s) {
		const LuaSteeringFactory* factory = luaAI_tosteeringfactory(s, 1);
		return luaL_error(s, "There is no execute() function set for steering: %s", factory->type().c_str());
	}

	static int luaAI_steeringtostring(lua_State* s) {
		const LuaSteeringFactory* factory = luaAI_tosteeringfactory(s, 1);
		lua_pushfstring(s, "steering: %s", factory->type().c_str());
		return 1;
	}

	static int luaAI_createsteering(lua_State* s) {
		LUAAIRegistry* r = luaAI_toregistry(s);
		const std::string type = luaL_checkstring(s, -1);
		const LUASteeringFactoryPtr& factory = std::make_shared<LuaSteeringFactory>(s, type);
		const bool inserted = r->registerSteeringFactory(type, *factory);
		if (!inserted) {
			return luaL_error(s, "steering %s is already registered", type.c_str());
		}

		luaAI_newuserdata<LuaSteeringFactory*>(s, factory.get());
		const luaL_Reg nodes[] = {
			{"filter", luaAI_steeringemptyexecute},
			{"__tostring", luaAI_steeringtostring},
			{"__newindex", luaAI_newindex},
			{nullptr, nullptr}
		};
		luaAI_setupmetatable(s, type, nodes, "steering");

		ScopedWriteLock scopedLock(r->_lock);
		r->_steeringFactories.emplace(type, factory);
		return 1;
	}

public:
	LUAAIRegistry() {
		init();
	}

	std::vector<luaL_Reg> aiFuncs = {
		{"id", luaAI_aiid},
		{"time", luaAI_aitime},
		{"hasZone", luaAI_aihaszone},
		{"zone", luaAI_aigetzone},
		{"filteredEntities", luaAI_aifilteredentities},
		{"setFilteredEntities", luaAI_aisetfilteredentities},
		{"addFilteredEntity", luaAI_aiaddfilteredentity},
		{"character", luaAI_aigetcharacter},
		{"aggroMgr", luaAI_aigetaggromgr},
		{"__tostring", luaAI_aitostring},
		{"__gc", luaAI_aigc},
		{"__eq", luaAI_aieq},
		{nullptr, nullptr}
	};
	std::vector<luaL_Reg> vecFuncs = {
		{"__add", luaAI_vecadd},
		{"__sub", luaAI_vecsub},
		{"__mul", luaAI_vecdot},
		{"__div", luaAI_vecdiv},
		{"__unm", luaAI_vecnegate},
		{"__len", luaAI_veclen},
		{"__eq", luaAI_veceq},
		{"__tostring", luaAI_vectostring},
		{"__index", luaAI_vecindex},
		{"__newindex", luaAI_vecnewindex},
		{"dot", luaAI_vecdot},
		{nullptr, nullptr}
	};
	std::vector<luaL_Reg> zoneFuncs = {
		{"size", luaAI_zonesize},
		{"name", luaAI_zonename},
		{"ai", luaAI_zoneai},
		{"execute", luaAI_zoneexecute},
		{"groupMgr", luaAI_zonegroupmgr},
		{"__tostring", luaAI_zonetostring},
		{nullptr, nullptr}
	};
	std::vector<luaL_Reg> characterFuncs = {
		{"id", luaAI_characterid},
		{"position", luaAI_characterposition},
		{"setPosition", luaAI_charactersetposition},
		{"speed", luaAI_characterspeed},
		{"setSpeed", luaAI_charactersetspeed},
		{"orientation", luaAI_characterorientation},
		{"setOrientation", luaAI_charactersetorientation},
		{"setAttribute", luaAI_charactersetattribute},
		{"attributes", luaAI_characterattributes},
		{"__eq", luaAI_charactereq},
		{"__gc", luaAI_charactergc},
		{"__tostring", luaAI_charactertostring},
		{nullptr, nullptr}
	};
	std::vector<luaL_Reg> aggroMgrFuncs = {
		{"setReduceByRatio", luaAI_aggromgrsetreducebyratio},
		{"setReduceByValue", luaAI_aggromgrsetreducebyvalue},
		{"resetReduceValue", luaAI_aggromgrresetreducevalue},
		{"addAggro", luaAI_aggromgraddaggro},
		{"highestEntry", luaAI_aggromgrhighestentry},
		{"entries", luaAI_aggromgrentries},
		{"__tostring", luaAI_aggromgrtostring},
		{nullptr, nullptr}
	};
	std::vector<luaL_Reg> groupMgrFuncs = {
		{"add", luaAI_groupmgradd},
		{"remove", luaAI_groupmgrremove},
		{"isLeader", luaAI_groupmgrisleader},
		{"isInGroup", luaAI_groupmgrisingroup},
		{"isInAnyGroup", luaAI_groupmgrisinanygroup},
		{"size", luaAI_groupmgrsize},
		{"position", luaAI_groupmgrposition},
		{"leader", luaAI_groupmgrleader},
		{"__tostring", luaAI_groupmgrtostring},
		{nullptr, nullptr}
	};
	std::vector<luaL_Reg> registryFuncs = {
		{"createNode", luaAI_createnode},
		{"createCondition", luaAI_createcondition},
		{"createFilter", luaAI_createfilter},
		{"createSteering", luaAI_createsteering},
		{nullptr, nullptr}
	};

	static int luaAI_aisetfilteredentities(lua_State* s) {
		luaAI_AI* ai = luaAI_toai(s, 1);
		luaL_checktype(s, 2, LUA_TTABLE);

		const int n = lua_rawlen(s, 2);
		FilteredEntities v(n);
		for (int i = 1; i <= n; ++i) {
			lua_rawgeti(s, 2, i);
			const int top = lua_gettop(s);
			const CharacterId id = (CharacterId)luaL_checknumber(s, top);
			v[i - 1] = id;
			lua_pop(s, 1);
		}
		ai->ai->setFilteredEntities(v);
		return 0;
	}

	static int luaAI_aiaddfilteredentity(lua_State* s) {
		luaAI_AI* ai = luaAI_toai(s, 1);
		const CharacterId id = (CharacterId)luaL_checkinteger(s, 2);
		ai->ai->addFilteredEntity(id);
		return 0;
	}

	/**
	 * @brief Access to the lua state.
	 * @see pushAIMetatable()
	 */
	lua_State* getLuaState() {
		return _s;
	}

	/**
	 * @brief Pushes the AI metatable onto the stack. This allows anyone to modify it
	 * to provide own functions and data that is applied to the @c ai parameters of the
	 * lua functions.
	 * @note lua_ctxai() can be used in your lua c callbacks to get access to the
	 * @ai{AI} pointer: @code const AI* ai = lua_ctxai(s, 1); @endcode
	 */
	int pushAIMetatable() {
		ai_assert(_s != nullptr, "LUA state is not yet initialized");
		return luaL_getmetatable(_s, luaAI_metaai());
	}

	/**
	 * @brief Pushes the character metatable onto the stack. This allows anyone to modify it
	 * to provide own functions and data that is applied to the @c ai:character() value
	 */
	int pushCharacterMetatable() {
		ai_assert(_s != nullptr, "LUA state is not yet initialized");
		return luaL_getmetatable(_s, luaAI_metacharacter());
	}

	/**
	 * @see shutdown()
	 */
	bool init() {
		if (_s != nullptr) {
			return true;
		}
		_s = luaL_newstate();

		lua_atpanic(_s, [] (lua_State* L) {
			ai_log_error("Lua panic. Error message: %s", (lua_isnil(L, -1) ? "" : lua_tostring(L, -1)));
			return 0;
		});
		lua_gc(_s, LUA_GCSTOP, 0);
		luaL_openlibs(_s);

		luaAI_registerfuncs(_s, &registryFuncs.front(), "META_REGISTRY");
		lua_setglobal(_s, "REGISTRY");

		// TODO: random

		luaAI_globalpointer(_s, this, luaAI_metaregistry());

		luaAI_registerfuncs(_s, &aiFuncs.front(), luaAI_metaai());
		luaAI_registerfuncs(_s, &vecFuncs.front(), luaAI_metavec());
		luaAI_registerfuncs(_s, &zoneFuncs.front(), luaAI_metazone());
		luaAI_registerfuncs(_s, &characterFuncs.front(), luaAI_metacharacter());
		luaAI_registerfuncs(_s, &aggroMgrFuncs.front(), luaAI_metaaggromgr());
		luaAI_registerfuncs(_s, &groupMgrFuncs.front(), luaAI_metagroupmgr());

		const char* script = ""
			"UNKNOWN, CANNOTEXECUTE, RUNNING, FINISHED, FAILED, EXCEPTION = 0, 1, 2, 3, 4, 5\n";

		if (luaL_loadbufferx(_s, script, strlen(script), "", nullptr) || lua_pcall(_s, 0, 0, 0)) {
			ai_log_error("%s", lua_tostring(_s, -1));
			lua_pop(_s, 1);
			return false;
		}
		return true;
	}

	/**
	 * @see init()
	 */
	void shutdown() {
		{
			ScopedWriteLock scopedLock(_lock);
			_treeNodeFactories.clear();
			_conditionFactories.clear();
			_filterFactories.clear();
			_steeringFactories.clear();
		}
		if (_s != nullptr) {
			lua_close(_s);
			_s = nullptr;
		}
	}

	~LUAAIRegistry() {
		shutdown();
	}

	inline bool evaluate(const std::string& str) {
		return evaluate(str.c_str(), str.length());
	}

	/**
	 * @brief Load your lua scripts into the lua state of the registry.
	 * This can be called multiple times to e.g. load multiple files.
	 * @return @c true if the lua script was loaded, @c false otherwise
	 * @note you have to call init() before
	 */
	bool evaluate(const char* luaBuffer, size_t size) {
		if (_s == nullptr) {
			ai_log_debug("LUA state is not yet initialized");
			return false;
		}
		if (luaL_loadbufferx(_s, luaBuffer, size, "", nullptr) || lua_pcall(_s, 0, 0, 0)) {
			ai_log_error("%s", lua_tostring(_s, -1));
			lua_pop(_s, 1);
			return false;
		}
		return true;
	}
};

}

#endif

#ifdef AI_INCLUDE_XML
// #include "tree/loaders/xml/XMLTreeLoader.h"
/**
 * @file
 */


// #include "tree/loaders/ITreeLoader.h"

#include <tinyxml2.h>
// #include "conditions/ConditionParser.h"

// #include "tree/TreeNodeParser.h"

// #include "tree/TreeNodeImpl.h"


namespace ai {

/**
 * @brief Implementation of @c ITreeLoader that gets its data from a xml file.
 */
class XMLTreeLoader: public ai::ITreeLoader {
private:
	TreeNodePtr loadSubTreeFromXML (const IAIFactory& aiFactory, tinyxml2::XMLElement* e) {
		if (e == nullptr) {
			return TreeNodePtr();
		}

		const char *name = e->Attribute("name", nullptr);
		if (name == nullptr) {
			setError("No name given");
			return TreeNodePtr();
		}

		const char *type = e->Attribute("type", nullptr);
		if (type == nullptr) {
			setError("No type given for name %s", name);
			return TreeNodePtr();
		}

		TreeNodeParser nodeParser(aiFactory, type);
		const TreeNodePtr& node = nodeParser.getTreeNode(name);
		if (!node) {
			setError("Could not create the tree node for name %s (type: %s)", name, type);
			return TreeNodePtr();
		}

		const char *condition = e->Attribute("condition", nullptr);
		if (condition == nullptr) {
			condition = "True";
		}

		ConditionParser conditionParser(aiFactory, condition);
		const ConditionPtr& conditionPtr = conditionParser.getCondition();
		if (!conditionPtr.get()) {
			setError("Could not create the condition for %s (name %s, type: %s)", condition, name, type);
			return TreeNodePtr();
		}

		node->setCondition(conditionPtr);
		return node;
	}

	TreeNodePtr loadTreeFromXML (const IAIFactory& aiFactory, tinyxml2::XMLElement* rootNode) {
		TreeNodePtr root = loadSubTreeFromXML(aiFactory, rootNode);
		if (!root.get()) {
			return root;
		}
		for (tinyxml2::XMLNode* node = rootNode->FirstChild(); node; node = node->NextSibling()) {
			tinyxml2::XMLElement* e = node->ToElement();
			const TreeNodePtr& child = loadSubTreeFromXML(aiFactory, e);
			if (child.get() == nullptr) {
				continue;
			}
			root->addChild(child);
		}
		return root;
	}

public:
	explicit XMLTreeLoader(const IAIFactory& aiFactory) :
			ITreeLoader(aiFactory) {
	}

	/**
	 * @brief this will initialize the loader once with all the defined behaviours from the given xml data.
	 */
	bool init(const std::string& xmlData, const char* rootNodeName = "trees", const char *treeNodeName = "tree") {
		resetError();
		tinyxml2::XMLDocument doc(false);
		const int status = doc.Parse(xmlData.c_str());
		tinyxml2::XMLElement* rootNode = doc.FirstChildElement(rootNodeName);
		if (rootNode == nullptr) {
			return false;
		}
		for (tinyxml2::XMLNode* node = rootNode->FirstChild(); node; node = node->NextSibling()) {
			tinyxml2::XMLElement* e = node->ToElement();
			if (e == nullptr) {
				setError("unexpected node type");
				continue;
			}
			if (e->Name() == nullptr) {
				setError("expected node name but didn't find one");
				continue;
			}
			if (::strcmp(treeNodeName, e->Name())) {
				setError("unexpected node name - expected 'tree' - got %s", e->Name());
				continue;
			}
			const char *name = e->Attribute("name");
			if (name == nullptr) {
				setError("node 'tree' does not have a 'name' attribute");
				continue;
			}
			tinyxml2::XMLNode* rootXMLNode = e->FirstChild();
			if (rootXMLNode == nullptr) {
				setError("node 'tree' doesn't have a child (which should e.g. be a selector)");
				continue;
			}
			const TreeNodePtr& root = loadTreeFromXML(_aiFactory, rootXMLNode->ToElement());
			if (root.get() == nullptr) {
				setError("could not create the root node");
				continue;
			}
			addTree(name, root);
		}
		if (status != tinyxml2::XML_NO_ERROR) {
			return false;
		}
		return getError().empty();
	}
};

}

#endif

/**
 * @}
 */
