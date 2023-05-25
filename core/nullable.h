#ifndef NULLABLE_H
#define NULLABLE_H


#include "Poco/Foundation.h"
#include "Poco/Exception.h"

#include <algorithm>
#include <iostream>

/// Nullable is a simple wrapper class for value types
/// that allows objects or native type variables
/// to have "null" value.
///
/// The class is useful for passing parameters to functions
/// when parameters are optional and no default values
/// should be used or when a non-assigned state is needed,
/// such as in e.g. fetching null values from database.
///
/// A Nullable can be default constructed. In this case,
/// the Nullable will have a Null value and isNull() will
/// return true. Calling value() (without default value) on
/// a Null object will throw a NullValueException.
///
/// A Nullable can also be constructed from a value.
/// It is possible to assign a value to a Nullable, and
/// to reset a Nullable to contain a Null value by calling
/// clear().
///
/// For use with Nullable, the value type should support
/// default construction.

enum NullType {
	NULL_GENERIC = 0
};


template <typename C>
class Nullable {
	C        _value;
	bool     _isNull;
	NullType _null;

public:
	Nullable():
		_value(), _isNull(true), _null() { }

	Nullable(const NullType&): // Creates an empty Nullable.
		_value(), _isNull(true), _null() { }

	Nullable(const C& value): // Creates a Nullable with the given value.
		_value(value), _isNull(false), _null() { }

	Nullable(C&& value): // Creates a Nullable by moving the given value.
		_value(std::forward<C>(value)), _isNull(false), _null() { }

	Nullable(const Nullable& other): // Creates a Nullable by copying another one.
		_value(other._value), _isNull(other._isNull), _null() { }

	Nullable(Nullable&& other) noexcept: // Creates a Nullable by moving another one.
		_value(std::move(other._value)), _isNull(other._isNull), _null() { other._isNull = true; }

	~Nullable() { }

	Nullable& assign(const C& value) {
		_value  = value;
		_isNull = false;
		return *this;
	}

	Nullable& assign(C&& value) {
		_value  = std::move(value);
		_isNull = false;
		return *this;
	}

	Nullable& assign(const Nullable& other) {
		Nullable tmp(other);
		swap(tmp);
		return *this;
	}

	Nullable& assign(NullType) { // Sets value to null.
		_isNull = true;
		return *this;
	}

	Nullable& operator = (const C& value) { // Assigns a value to the Nullable.
		return assign(value);
	}

	Nullable& operator = (C&& value) { // Move-assigns a value to the Nullable.
		return assign(std::move(value));
	}

	Nullable& operator = (const Nullable& other) { // Assigns another Nullable.
		return assign(other);
	}

	Nullable& operator = (Nullable&& other) noexcept { // Moves another Nullable.
		_isNull = other._isNull;
		_value = std::move(other._value);
		other._isNull = true;
		return *this;
	}

	Nullable& operator = (NullType) { // Assigns another Nullable.
		_isNull = true;
		return *this;
	}

	void swap(Nullable& other) noexcept { // Swaps this Nullable with other.
		std::swap(_value, other._value);
		std::swap(_isNull, other._isNull);
	}

	bool operator == (const Nullable<C>& other) const { // Compares two Nullables for equality
		return (_isNull && other._isNull) || (_isNull == other._isNull && _value == other._value);
	}

	bool operator == (const C& value) const { // Compares Nullable with value for equality
		return (!_isNull && _value == value);
	}

	bool operator == (const NullType&) const { // Compares Nullable with NullData for equality
		return _isNull;
	}

	bool operator != (const C& value) const { // Compares Nullable with value for non equality
		return !(*this == value);
	}

	bool operator != (const Nullable<C>& other) const { // Compares two Nullables for non equality
		return !(*this == other);
	}

	bool operator != (const NullType&) const { // Compares with NullData for non equality
		return !_isNull;
	}

	// Compares two Nullable objects. Return true if this object's
	// value is smaler than the other object's value.
	// Null value is smaller than a non-null value.
	bool operator < (const Nullable<C>& other) const {
		if (_isNull && other._isNull) {
			return false;
		}
		if (!_isNull && !other._isNull) {
			return (_value < other._value);
		}
		if (_isNull && !other._isNull) {
			return true;
		}
		return false;
	}

	// Compares two Nullable objects. Return true if this object's
	// value is greater than the other object's value.
	// A non-null value is greater than a null value.
	bool operator > (const Nullable<C>& other) const {
		return !(*this == other) && !(*this < other);
	}

	// Returns the Nullable's value.
	// Throws a NullValueException if the Nullable is empty.
	C& value() {
		if (!_isNull)
			return _value;
		else
			throw NullValueException();
	}

	// Returns the Nullable's value.
	// Throws a NullValueException if the Nullable is empty.
	const C& value() const {
		if (!_isNull) {
			return _value;
		} else {
			throw NullValueException();
		}
	}

	// Returns the Nullable's value, or the
	// given default value if the Nullable is empty.
	const C& value(const C& deflt) const {
		return _isNull ? deflt : _value;
	}

	operator C& () { // Get reference to the value
		return value();
	}

	operator const C& () const { // Get const reference to the value
		return value();
	}

	operator NullType& () { // Get reference to the value
		return _null;
	}

	bool isNull() const { // Returns true if the Nullable is empty.
		return _isNull;
	}

	void clear() { // Clears the Nullable.
		_isNull = true;
	}
};


template <typename C>
inline void swap(Nullable<C>& n1, Nullable<C>& n2) noexcept {
	n1.swap(n2);
}


template <typename C>
std::ostream& operator<<(std::ostream& out, const Nullable<C>& obj) {
	if (!obj.isNull()) out << obj.value();
	return out;
}


template <typename C>
bool operator == (const NullType&, const Nullable<C>& n) {
	return n.isNull(); // Returns true if this Nullable is null.
}


template <typename C>
bool operator != (const C& c, const Nullable<C>& n) {
	return !(n == c); // Compares Nullable with value for non equality
}


template <typename C>
bool operator == (const C& c, const Nullable<C>& n) {
	return (n == c); // Compares Nullable with NullData for equality
}


template <typename C>
bool operator != (const NullType&, const Nullable<C>& n) {
	return !n.isNull(); // Returns true if this Nullable is not null.
}

#endif // NULLABLE_H
