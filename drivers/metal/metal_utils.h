#include <algorithm>
#include <limits>

template <typename T>
inline constexpr bool is_pow2(T x) {
	static_assert(std::is_integral<T>::value, "is_pow2 must be called on an integer type.");
	return (x & (x - 1)) == 0 && (x != 0);
}

template <typename T>
T round_up(const T value, const T alignment) {
	auto temp = value + alignment - static_cast<T>(1);
	return temp - temp % alignment;
}

template <typename T>
constexpr T round_up_pow2(const T value, const T alignment) {
	assert(is_pow2(alignment));
	return (value + alignment - 1) & ~(alignment - 1);
}
