#ifndef AUTORELEASE_POOL_H
#define AUTORELEASE_POOL_H

#ifdef __OBJC__
typedef NSAutoreleasePool* NSAutoreleasePoolPtr;
#else
# include <objc/NSObjCRuntime.h>
using NSAutoreleasePoolPtr = id;
#endif

class AutoreleasePool final
{
public:
	AutoreleasePool() noexcept;
	~AutoreleasePool();
	AutoreleasePool(const AutoreleasePool& other) noexcept;
	AutoreleasePool(AutoreleasePool&& other) noexcept;
	AutoreleasePool& operator=(const AutoreleasePool& other) noexcept;
	AutoreleasePool& operator=(AutoreleasePool&& other) noexcept;

	bool operator==(const AutoreleasePool& other) const noexcept {
		return pool == other.pool;
	}

	bool operator!=(const AutoreleasePool& other) const noexcept {
		return pool != other.pool;
	}

private:
	NSAutoreleasePoolPtr pool = nil;
};

#endif // AUTORELEASE_POOL_H
