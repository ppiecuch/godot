#if defined(OSX_ENABLED) || defined(IOS_ENABLED)

#include "autorelease_pool.h"

AutoreleasePool::AutoreleasePool() noexcept:
	pool([[NSAutoreleasePool alloc] init]) {
}

AutoreleasePool::~AutoreleasePool() {
	[pool release];
}

AutoreleasePool::AutoreleasePool(const AutoreleasePool& other) noexcept:
	pool([other.pool retain]) {
}

AutoreleasePool::AutoreleasePool(AutoreleasePool&& other) noexcept:
	pool(other.pool)
{
	other.pool = nil;
}

AutoreleasePool& AutoreleasePool::operator=(const AutoreleasePool& other) noexcept {
	if (&other == this) return *this;
	[pool release];
	pool = [other.pool retain];
	return *this;
}

AutoreleasePool& AutoreleasePool::operator=(AutoreleasePool&& other) noexcept {
	if (&other == this) return *this;
	[pool release];
	pool = other.pool;
	other.pool = nil;
	return *this;
}
